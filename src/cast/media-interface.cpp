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

#include "media-interface.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace cast {

const QString MediaInterface::URN = QStringLiteral("urn:x-cast:com.google.cast.media");

MediaInterface::MediaInterface(Channel *channel)
    : Interface(channel, URN) {
    connect(this, &Interface::messageReceived,
            this, &MediaInterface::onMessageReceived);
    getStatus();
}

MediaInterface::~MediaInterface() = default;

bool MediaInterface::play(int media_session_id) {
    QJsonObject msg;
    msg["type"] = QStringLiteral("PLAY");
    msg["requestId"] = ++last_request_;
    msg["mediaSessionId"] = media_session_id;
    QJsonDocument doc(msg);
    return send(QString(doc.toJson(QJsonDocument::Compact)));
}

bool MediaInterface::pause(int media_session_id) {
    QJsonObject msg;
    msg["type"] = QStringLiteral("PAUSE");
    msg["requestId"] = ++last_request_;
    msg["mediaSessionId"] = media_session_id;
    QJsonDocument doc(msg);
    return send(QString(doc.toJson(QJsonDocument::Compact)));
}

bool MediaInterface::stop(int media_session_id) {
    QJsonObject msg;
    msg["type"] = QStringLiteral("STOP");
    msg["requestId"] = ++last_request_;
    msg["mediaSessionId"] = media_session_id;
    QJsonDocument doc(msg);
    return send(QString(doc.toJson(QJsonDocument::Compact)));
}

bool MediaInterface::getStatus() {
    QJsonObject msg;
    msg["type"] = QStringLiteral("GET_STATUS");
    msg["requestId"] = ++last_request_;
    QJsonDocument doc(msg);
    return send(QString(doc.toJson(QJsonDocument::Compact)));
}

void MediaInterface::onMessageReceived(const QString& data) {
    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(data.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "Could not parse message on connection channel:"
                   << err.errorString();
        return;
    }
    if (doc.object()["type"].toString() != "MEDIA_STATUS") return;

    status_ = doc.object()["status"].toArray().toVariantList();

    Q_EMIT statusChanged();
}

}
