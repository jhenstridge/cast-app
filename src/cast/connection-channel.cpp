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

#include "connection-channel.h"
#include "channel.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

namespace cast {

const QString ConnectionChannel::URN = QStringLiteral("urn:x-cast:com.google.cast.tp.connection");

ConnectionChannel::ConnectionChannel(Caster *caster, const QString& source_id,
                                   const QString& destination_id)
    : Channel(caster, source_id, destination_id, URN) {
    connect(this, &Channel::messageReceived,
            this, &ConnectionChannel::onMessageReceived);
}

ConnectionChannel::~ConnectionChannel() = default;

bool ConnectionChannel::sendConnect() {
    return send(R"({"type": "CONNECT"})");
}

void ConnectionChannel::onMessageReceived(const QString& data) {
    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(data.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "Could not parse message on connection channel:"
                   << err.errorString();
        return;
    }
    if (doc.object()["type"].toString() == "CLOSE") {
        static_cast<Caster*>(parent())->disconnectFromHost();
    }
}

}
