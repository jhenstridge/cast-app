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

#include "channel.h"

namespace cast {

Channel::Channel(Caster *caster, const QString& source_id,
                 const QString& destination_id,
                 const QString& channel_namespace)
    : QObject(caster), source_id_(source_id),
      destination_id_(destination_id), namespace_(channel_namespace) {
    connect(caster, &Caster::messageReceived,
            this, &Channel::onMessageReceived);
}

Channel::~Channel() = default;

bool Channel::send(const QString& data) {
    Caster::Message message;

    message.set_protocol_version(Caster::Message::CASTV2_1_0);
    message.set_source_id(source_id_.toStdString());
    message.set_destination_id(destination_id_.toStdString());
    message.set_namespace_(namespace_.toStdString());
    message.set_payload_type(Caster::Message::STRING);
    message.set_payload_utf8(data.toStdString());
    return static_cast<Caster*>(parent())->sendMessage(message);
}

bool Channel::sendBinary(const QByteArray& data) {
    Caster::Message message;

    message.set_protocol_version(Caster::Message::CASTV2_1_0);
    message.set_source_id(source_id_.toStdString());
    message.set_destination_id(destination_id_.toStdString());
    message.set_namespace_(namespace_.toStdString());
    message.set_payload_type(Caster::Message::BINARY);
    message.set_payload_binary(data.constData(), data.size());
    return static_cast<Caster*>(parent())->sendMessage(message);
}

void Channel::onMessageReceived(const Caster::Message& message) {
    if (message.protocol_version() != Caster::Message::CASTV2_1_0) return;
    if (message.source_id() != source_id_.toStdString()) return;
    if (message.destination_id() != destination_id_.toStdString()) return;
    if (message.namespace_() != namespace_.toStdString()) return;

    switch (message.payload_type()) {
    case Caster::Message::STRING:
        Q_EMIT messageReceived(QString::fromStdString(message.payload_utf8()));
        break;
    case Caster::Message::BINARY: {
        const auto& data = message.payload_binary();
        Q_EMIT binaryMessageReceived(QByteArray(&data[0], data.size()));
        break;
    }
    default:
        break;
    }
}

}
