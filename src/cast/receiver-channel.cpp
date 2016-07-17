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

#include "receiver-channel.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

namespace cast {

const QString ReceiverChannel::URN = QStringLiteral("urn:x-cast:com.google.cast.receiver");

ReceiverChannel::ReceiverChannel(Caster *caster, const QString& source_id,
                                 const QString& destination_id)
    : Channel(caster, source_id, destination_id, URN) {
    connect(this, &Channel::messageReceived,
            this, &ReceiverChannel::onMessageReceived);
}

ReceiverChannel::~ReceiverChannel() = default;

bool ReceiverChannel::launch(const QString& app_id) {
    QJsonObject msg;
    msg["type"] = QStringLiteral("LAUNCH");
    msg["requestId"] = 1;
    msg["appId"] = app_id;
    QJsonDocument doc(msg);
    return send(QString(doc.toJson(QJsonDocument::Compact)));
}

bool ReceiverChannel::stop(const QString& session_id) {
    QJsonObject msg;
    msg["type"] = QStringLiteral("STOP");
    msg["requestId"] = 1;
    msg["sessionId"] = session_id;
    QJsonDocument doc(msg);
    return send(QString(doc.toJson(QJsonDocument::Compact)));
}

bool ReceiverChannel::getStatus() {
    QJsonObject msg;
    msg["type"] = QStringLiteral("GET_STATUS");
    msg["requestId"] = 1;
    QJsonDocument doc(msg);
    return send(QString(doc.toJson(QJsonDocument::Compact)));
}

void ReceiverChannel::onMessageReceived(const QString& data) {
    qInfo() << "Received message:" << data;
}

}
