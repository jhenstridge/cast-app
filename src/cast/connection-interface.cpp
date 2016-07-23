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

#include "connection-interface.h"
#include "channel.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

namespace cast {

const QString ConnectionInterface::URN = QStringLiteral("urn:x-cast:com.google.cast.tp.connection");

ConnectionInterface::ConnectionInterface(Channel *channel)
    : Interface(channel, URN) {
    connect(this, &Interface::messageReceived,
            this, &ConnectionInterface::onMessageReceived);
    send(R"({"type": "CONNECT"})");
}

ConnectionInterface::~ConnectionInterface() = default;

void ConnectionInterface::onMessageReceived(const QString& data) {
    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(data.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "Could not parse message on connection channel:"
                   << err.errorString();
        return;
    }
    if (doc.object()["type"].toString() == "CLOSE") {
#if 0
        qWarning() << "Received close message from channel";
#endif
        channel().close();
    }
}

}
