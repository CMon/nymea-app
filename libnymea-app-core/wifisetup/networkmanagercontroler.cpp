/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2018 Simon Stuerz <simon.stuerz@guh.io>                  *
 *                                                                         *
 *  This file is part of nymea:app                                         *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "networkmanagercontroler.h"
#include "engine.h"

NetworkManagerControler::NetworkManagerControler(QObject *parent) : QObject(parent)
{

}

QString NetworkManagerControler::name() const
{
    return m_name;
}

void NetworkManagerControler::setName(const QString &name)
{
    m_name = name;
    emit nameChanged();
}

QString NetworkManagerControler::address() const
{
    return m_address;
}

void NetworkManagerControler::setAddress(const QString &address)
{
    m_address = address;
}

WirelessSetupManager *NetworkManagerControler::manager()
{
    return m_wirelessSetupManager;
}

void NetworkManagerControler::connectDevice()
{
    if (m_wirelessSetupManager) {
        delete m_wirelessSetupManager;
        m_wirelessSetupManager = nullptr;
        emit managerChanged();
    }

    // find device info for this address and name
    BluetoothDeviceInfo *deviceInfo = nullptr;
    foreach (BluetoothDeviceInfo *deviceInformation, Engine::instance()->bluetoothDiscovery()->deviceInfos()->deviceInfos()) {
        if (deviceInformation->address() == address() && deviceInformation->name() == name()) {
            deviceInfo = deviceInformation;
        }
    }

    if (!deviceInfo) {
        qDebug() << "Could not connect to device. There is no device info for" << name() << address();
        return;
    }

    m_wirelessSetupManager = new WirelessSetupManager(deviceInfo->getBluetoothDeviceInfo(), this);
    emit managerChanged();

    m_wirelessSetupManager->connectDevice();
}
