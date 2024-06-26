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

#include "interface.h"

#include "eventtypes.h"
#include "statetypes.h"
#include "actiontypes.h"
#include "thingclass.h"

Interface::Interface(const QString &name, const QString &displayName, QObject *parent) :
    QObject(parent),
    m_name(name),
    m_displayName(displayName),
    m_eventTypes(new EventTypes(this)),
    m_stateTypes(new StateTypes(this)),
    m_actionTypes(new ActionTypes(this))
{

}

QString Interface::name() const
{
    return m_name;
}

QString Interface::displayName() const
{
    return m_displayName;
}

EventTypes* Interface::eventTypes() const
{
    return m_eventTypes;
}

StateTypes* Interface::stateTypes() const
{
    return m_stateTypes;
}

ActionTypes* Interface::actionTypes() const
{
    return m_actionTypes;
}

ThingClass *Interface::createThingClass()
{
    ThingClass* dc = new ThingClass();
    dc->setName(m_name);
    dc->setParamTypes(new ParamTypes(dc));
    dc->setSettingsTypes(new ParamTypes(dc));
    dc->setDisplayName(m_displayName);
    dc->setEventTypes(m_eventTypes);
    dc->setStateTypes(m_stateTypes);
    dc->setActionTypes(m_actionTypes);
    return dc;
}
