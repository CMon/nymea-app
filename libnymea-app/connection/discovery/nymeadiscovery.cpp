/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "nymeadiscovery.h"
#include "upnpdiscovery.h"
#include "zeroconfdiscovery.h"
#include "bluetoothservicediscovery.h"
#include "connection/awsclient.h"
#include "../nymeahost.h"

#include <QUuid>
#include <QBluetoothUuid>
#include <QUrlQuery>
#include <QSettings>
#include <QNetworkConfigurationManager>
#include <QNetworkSession>

#include "logging.h"
NYMEA_LOGGING_CATEGORY(dcDiscovery, "Discovery")

NymeaDiscovery::NymeaDiscovery(QObject *parent) : QObject(parent)
{
    m_nymeaHosts = new NymeaHosts(this);

    loadFromDisk();

    m_cloudPollTimer.setInterval(5000);
    connect(&m_cloudPollTimer, &QTimer::timeout, this, [this](){
        if (m_awsClient && m_awsClient->isLoggedIn()) {
            m_awsClient->fetchDevices();
        }
    });

}

NymeaDiscovery::~NymeaDiscovery()
{
}

bool NymeaDiscovery::discovering() const
{
    return m_discovering;
}

void NymeaDiscovery::setDiscovering(bool discovering)
{
    if (m_discovering == discovering)
        return;

    m_discovering = discovering;
    if (discovering) {
        if (m_zeroconfDiscoveryEnabled) {
            if (!m_zeroConf) {
                m_zeroConf = new ZeroconfDiscovery(m_nymeaHosts, this);
            }
        }

        // Start UPnP discovery
        if (m_upnpDiscoveryEnabled) {
            if (!m_upnp) {
                m_upnp = new UpnpDiscovery(m_nymeaHosts, this);
            }
            m_upnp->discover();
        }

        // Start Bluetooth discovery if HW is available
        if (m_bluetoothDiscoveryEnabled) {
            if (!m_bluetooth) {
                m_bluetooth = new BluetoothServiceDiscovery(m_nymeaHosts, this);
            }
            m_bluetooth->discover();
        }

        // start polling cloud
        m_cloudPollTimer.start();
        // If we're logged in, poll right away
        if (m_awsClient && m_awsClient->isLoggedIn()) {
            m_awsClient->fetchDevices();
        }
    } else {

        if (m_upnp) {
            m_upnp->stopDiscovery();
        }

        if (m_bluetooth) {
            m_bluetooth->stopDiscovery();
        }

        m_cloudPollTimer.stop();
    }

    emit discoveringChanged();
}

NymeaHosts *NymeaDiscovery::nymeaHosts() const
{
    return m_nymeaHosts;
}

AWSClient *NymeaDiscovery::awsClient() const
{
    return m_awsClient;
}

void NymeaDiscovery::setAwsClient(AWSClient *awsClient)
{
    if (m_awsClient != awsClient) {
        m_awsClient = awsClient;
        emit awsClientChanged();
    }

    if (m_awsClient) {
        connect(m_awsClient, &AWSClient::devicesFetched, this, &NymeaDiscovery::syncCloudDevices);
    }
}

void NymeaDiscovery::cacheHost(NymeaHost *host)
{
    QSettings settings;
    settings.beginGroup("HostCache");
    settings.remove(host->uuid().toString());
    settings.beginGroup(host->uuid().toString());
    settings.setValue("name", host->name());
    QList<Connection*> connections;
    Connection *remoteConnection = host->connections()->bestMatch(Connection::BearerTypeCloud);
    if (remoteConnection) {
        connections.append(remoteConnection);
    }
    Connection *loopbackConnection = host->connections()->bestMatch(Connection::BearerTypeLoopback);
    if (loopbackConnection) {
        connections.append(loopbackConnection);
    }
    Connection *lanConnection = host->connections()->bestMatch(Connection::BearerTypeLan);
    if (lanConnection) {
        connections.append(lanConnection);
    }
    Connection *wanConnection = host->connections()->bestMatch(Connection::BearerTypeWan);
    if (wanConnection) {
        connections.append(wanConnection);
    }
    Connection *btConnection = host->connections()->bestMatch(Connection::BearerTypeBluetooth);
    if (btConnection) {
        connections.append(btConnection);
    }
    int i = 0;
    foreach (Connection *connection, connections) {
        settings.beginGroup(QString::number(i++));
        settings.setValue("url", connection->url());
        settings.setValue("bearerType", connection->bearerType());
        settings.setValue("secure", connection->secure());
        settings.setValue("displayName", connection->displayName());
        settings.endGroup();
    }
    settings.endGroup();
}

bool NymeaDiscovery::zeroconfDiscoveryEnable() const
{
    return m_zeroconfDiscoveryEnabled;
}

bool NymeaDiscovery::bluetoothDiscoveryEnabled() const
{
    return m_bluetoothDiscoveryEnabled;
}

bool NymeaDiscovery::upnpDiscoveryEnabled() const
{
    return m_upnpDiscoveryEnabled;
}

void NymeaDiscovery::setZeroconfDiscoveryEnabled(bool zeroconfDiscoveryEnabled)
{
    if (m_zeroconfDiscoveryEnabled  != zeroconfDiscoveryEnabled) {
        m_zeroconfDiscoveryEnabled = zeroconfDiscoveryEnabled;
        emit zeroconfDiscoveryEnabledChanged(m_zeroconfDiscoveryEnabled);
    }
}

void NymeaDiscovery::setBluetoothDiscoveryEnabled(bool bluetoothDiscoveryEnabled)
{
    if (m_bluetoothDiscoveryEnabled != bluetoothDiscoveryEnabled) {
        m_bluetoothDiscoveryEnabled = bluetoothDiscoveryEnabled;
        emit bluetoothDiscoveryEnabledChanged(m_bluetoothDiscoveryEnabled);
    }
}

void NymeaDiscovery::setUpnpDiscoveryEnabled(bool upnpDiscoveryEnabled)
{
    if (m_upnpDiscoveryEnabled != upnpDiscoveryEnabled) {
        m_upnpDiscoveryEnabled = upnpDiscoveryEnabled;
        emit upnpDiscoveryEnabledChanged(m_upnpDiscoveryEnabled);
    }
}

void NymeaDiscovery::syncCloudDevices()
{
    for (int i = 0; i < m_awsClient->awsDevices()->rowCount(); i++) {
        AWSDevice *d = m_awsClient->awsDevices()->get(i);
        NymeaHost *host = m_nymeaHosts->find(d->id());
        bool alreayAdded = host != nullptr;
        if (!alreayAdded) {
            host = new NymeaHost();
            host->setUuid(d->id());
        }
        host->setName(d->name());
        QUrl url;
        url.setScheme("cloud");
        url.setHost(d->id());
        Connection *conn = host->connections()->find(url);
        if (!conn) {
            conn = new Connection(url, Connection::BearerTypeCloud, true, d->id());
            qCDebug(dcDiscovery) << "Adding new connection to host:" << host->name() << conn->url().toString();
            host->connections()->addConnection(conn);
        }
        conn->setOnline(d->online());
        if (!alreayAdded) {
            qCDebug(dcDiscovery) << "Adding new host:" << d->name() << d->id();
            m_nymeaHosts->addHost(host);
        }
    }

    QList<NymeaHost*> hostsToRemove;
    for (int i = 0; i < m_nymeaHosts->rowCount(); i++) {
        NymeaHost *host = m_nymeaHosts->get(i);
        for (int j = 0; j < host->connections()->rowCount(); j++) {
            if (host->connections()->get(j)->bearerType() == Connection::BearerTypeCloud) {
                if (m_awsClient->awsDevices()->getDevice(host->uuid().toString()) == nullptr) {
                    host->connections()->removeConnection(j);
                    break;
                }
            }
        }
        if (host->connections()->rowCount() == 0) {
            hostsToRemove.append(host);
        }
    }
    while (!hostsToRemove.isEmpty()) {
        m_nymeaHosts->removeHost(hostsToRemove.takeFirst());
    }
}

void NymeaDiscovery::loadFromDisk()
{
    QSettings settings;
    settings.beginGroup("HostCache");
    foreach (const QString &serverUuid, settings.childGroups()) {
        settings.beginGroup(serverUuid);
        NymeaHost* host = m_nymeaHosts->find(QUuid(serverUuid));
        if (!host) {
            host = new NymeaHost(m_nymeaHosts);
            host->setName(settings.value("name").toString());
            host->setUuid(QUuid(serverUuid));
            m_nymeaHosts->addHost(host);
        }
        qCDebug(dcDiscovery()) << "Loaded Host from cache" << host->name() << host->uuid();
        foreach (const QString &group, settings.childGroups()) {
            settings.beginGroup(group);
            QString url = settings.value("url").toString();
            Connection* connection = host->connections()->find(url);
            if (!connection) {
                Connection::BearerType bearerType = static_cast<Connection::BearerType>(settings.value("bearerType").toInt());
                bool secure = settings.value("secure").toBool();
                QString displayName = settings.value("displayName").toString();
                connection = new Connection(url, bearerType, secure, displayName, host);
                host->connections()->addConnection(connection);
                qCDebug(dcDiscovery()) << "|- Connection:" << group << connection->url() << connection->bearerType() << "secure:" << connection->secure();
            }
            settings.endGroup();
        }
        settings.endGroup();
    }

}

void NymeaDiscovery::updateActiveBearers()
{
}

