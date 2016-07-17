/*
 * cast-qml: Chromecast binding for QML
 * Copyright (C) 2016  James Henstridge
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "plugin.h"
#include "browser.h"
#include "caster.h"
#include "channel.h"
#include "interface.h"
#include "receiver-interface.h"

namespace cast {

void CastPlugin::registerTypes(const char *uri) {
    qmlRegisterType<Browser>(uri, 0, 1, "Browser");
    qmlRegisterType<Caster>(uri, 0, 1, "Caster");
    qmlRegisterUncreatableType<Channel>(
        uri, 0, 1, "Channel", "Use a Caster to create channels");
    qmlRegisterUncreatableType<Interface>(
        uri, 0, 1, "Interface", "Use a Channel to create interfaces");
    qmlRegisterUncreatableType<ReceiverInterface>(
        uri, 0, 1, "ReceiverInterface", "Use a Channel to create interfaces");
}

}
