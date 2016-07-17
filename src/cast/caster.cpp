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

#include "caster.h"
#include "connection-channel.h"
#include "heartbeat-channel.h"
#include "receiver-channel.h"

#include <QtEndian>
#include <QDebug>

namespace cast {

Caster::Caster(QObject *parent) : QObject(parent) {
    socket_.setPeerVerifyMode(QSslSocket::VerifyNone);
    connect(&socket_, &QSslSocket::encrypted,
            this, &Caster::onEncrypted);
    connect(&socket_, &QIODevice::readyRead,
            this, &Caster::onReadyRead);
    connect(&socket_, &QIODevice::readChannelFinished,
            this, &Caster::onReadChannelFinished);

    connection_ = new ConnectionChannel(this, "sender-0", "receiver-0");
    heartbeat_ = new HeartbeatChannel(this, "sender-0", "receiver-0");
    receiver_ = new ReceiverChannel(this, "sender-0", "receiver-0");
}

Caster::~Caster() = default;

void Caster::connectToHost(const QString &host_name, int port) {
    socket_.connectToHostEncrypted(host_name, port);
}

void Caster::disconnectFromHost() {
    qInfo() << "Disconnecting";
    socket_.disconnectFromHost();
    Q_EMIT disconnected();
}

Channel* Caster::createChannel(const QString& sender_id,
                               const QString& destination_id,
                               const QString& channel_namespace) {
    return new Channel(this, sender_id, destination_id, channel_namespace);
}

void Caster::onEncrypted() {
    qInfo() << "Connected";
    Q_EMIT connected();
    connection_->sendConnect();
    receiver_->launch("YouTube");
}

void Caster::onReadyRead() {
    while (true) {
        switch (read_state_) {
        case State::read_header: {
            if (socket_.bytesAvailable() < 4) {
                /* We haven't read enough information to know the
                 * message size, so wait for more. */
                return;
            }
            char header[4];
            socket_.read(header, sizeof(header));
            message_size_ = qFromBigEndian<uint32_t>(
                reinterpret_cast<const unsigned char *>(header));
            if (message_size_ > 0) {
                read_state_ = State::read_body;
                message_read_ = 0;
                message_data_.resize(message_size_);
            }
            break;
        }
        case State::read_body: {
            auto n_read = socket_.read(message_data_.data() + message_read_,
                                       message_size_ - message_read_);
            if (n_read <= 0) {
                return;
            }
            message_read_ += n_read;
            /* Do we have an entire message? */
            if (message_read_ == message_size_) {
                if (received_message_.ParseFromArray(
                        message_data_.constData(), message_data_.size())) {

    qInfo() << "Received message:";
    qInfo() << "  source:" << received_message_.source_id().c_str();
    qInfo() << "  destination:" << received_message_.destination_id().c_str();
    qInfo() << "  namespace:" << received_message_.namespace_().c_str();
    if (received_message_.payload_type() == Message::STRING) {
        qInfo() << "  payload:" << received_message_.payload_utf8().c_str();
    }

                    Q_EMIT messageReceived(received_message_);
                } else {
                    qWarning() << "Could not parse incoming message";
                }
                read_state_ = State::read_header;
                message_size_ = 0;
                message_read_ = 0;
            }
            break;
        }
        }
    }
}

void Caster::onReadChannelFinished() {
    disconnectFromHost();
}

bool Caster::sendMessage(const Message& message) {
    int msg_size = message.ByteSize();
    QByteArray data;
    data.resize(msg_size + 4);
    qToBigEndian<uint32_t>(msg_size,
                           reinterpret_cast<unsigned char*>(data.data()));
    if (!message.SerializeToArray(data.data() + 4, msg_size)) {
        return false;
    }

    qInfo() << "Sending message:";
    qInfo() << "  source:" << message.source_id().c_str();
    qInfo() << "  destination:" << message.destination_id().c_str();
    qInfo() << "  namespace:" << message.namespace_().c_str();
    if (message.payload_type() == Message::STRING) {
        qInfo() << "  payload:" << message.payload_utf8().c_str();
    }
    return socket_.write(data) == data.size();
}

}
