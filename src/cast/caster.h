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

#pragma once

#include "cast_channel.pb.h"

#include <QObject>
#include <QSslSocket>

#include <cstdint>
#include <map>
#include <utility>

namespace cast {

class Channel;
class ReceiverInterface;

class Caster : public QObject {
    Q_OBJECT
public:
    typedef extensions::api::cast_channel::CastMessage Message;

    explicit Caster(QObject *parent=nullptr);
    virtual ~Caster();

    Q_INVOKABLE void connectToHost(const QString& host_name, int port);
    Q_INVOKABLE void disconnectFromHost();

    Q_INVOKABLE cast::Channel* createChannel(const QString& source_id,
                                             const QString& destination_id);

    bool sendMessage(const Message& message);

Q_SIGNALS:
    void connected();
    void disconnected();

private Q_SLOTS:
    void onEncrypted();
    void onReadyRead();
    void onReadChannelFinished();

    void onChannelClosed();

private:
    void handleMessage(const Message& message);

    QSslSocket socket_;

    // Manage reading the incoming message
    enum class State { read_header, read_body };
    State read_state_ = State::read_header;
    uint32_t message_size_ = 0;
    uint32_t message_read_ = 0;
    QByteArray message_data_;
    Message received_message_;

    // Manage communication channels
    std::map<std::pair<QString,QString>,Channel*> channels_;
    Channel *platform_channel_ = nullptr;
    ReceiverInterface *receiver_ = nullptr;
};

}
