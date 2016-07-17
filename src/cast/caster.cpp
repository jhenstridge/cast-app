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
#include "channel.h"
#include "heartbeat-interface.h"
#include "receiver-interface.h"

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
}

Caster::~Caster() = default;

void Caster::connectToHost(const QString &host_name, int port) {
    socket_.connectToHostEncrypted(host_name, port);
}

void Caster::onEncrypted() {
    qInfo() << "Connected";
    platform_channel_ = createChannel(QStringLiteral("sender-0"),
                                      QStringLiteral("receiver-0"));
    platform_channel_->addInterface(HeartbeatInterface::URN);
    receiver_ = static_cast<ReceiverInterface*>(
        platform_channel_->addInterface(ReceiverInterface::URN));
    Q_EMIT connected();
    receiver_->getStatus();
}

void Caster::disconnectFromHost() {
    qInfo() << "Disconnecting";
    // Kill off all the channels
    platform_channel_ = nullptr;
    receiver_ = nullptr;
    for (auto it = channels_.begin(); it != channels_.end(); ++it) {
        it->second->deleteLater();
    }
    channels_.clear();
    // And disconnect the socket
    socket_.disconnectFromHost();
    Q_EMIT disconnected();
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
                    handleMessage(received_message_);
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

Channel* Caster::createChannel(const QString& source_id,
                               const QString& destination_id) {
    Channel *channel;
    try {
        channel = channels_.at({destination_id, source_id});
    } catch (const std::out_of_range &) {
        channel = new Channel(this, source_id, destination_id);
        channels_.emplace(std::make_pair(destination_id, source_id), channel);
        connect(channel, &Channel::closed,
                this, &Caster::onChannelClosed, Qt::QueuedConnection);
    }
    return channel;
}

void Caster::onChannelClosed() {
    Channel *c = static_cast<Channel*>(sender());

    channels_.erase({c->destination_id_, c->source_id_});
    if (c == platform_channel_) {
        disconnectFromHost();
    }
    c->deleteLater();
}

void Caster::handleMessage(const Message& message) {
    if (message.protocol_version() != Caster::Message::CASTV2_1_0) {
        qWarning() << "Unsupported protocol version:" << message.protocol_version();
        return;
    }

    const QString source = QString::fromStdString(message.source_id());
    const QString destination = QString::fromStdString(message.destination_id());
    if (destination == "*") {
        // Broadcast message to all channels connected to the sender
        for (auto it = channels_.lower_bound({source, QString()}); it != channels_.end(); ++it) {
            if (it->first.first != destination) break;
            it->second->handleMessage(message);
        }
    } else {
        try {
            channels_.at({source, destination})->handleMessage(message);
        } catch (const std::out_of_range &) {
            qWarning() << "Message received for unknown channel:"
                       << source << "->" << destination;
        }
    }
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
    return socket_.write(data) == data.size();
}

}
