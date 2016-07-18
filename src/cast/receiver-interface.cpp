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

#include "receiver-interface.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace cast {

const QString ReceiverInterface::URN = QStringLiteral("urn:x-cast:com.google.cast.receiver");

ReceiverInterface::ReceiverInterface(Channel *channel)
    : Interface(channel, URN) {
    connect(this, &Interface::messageReceived,
            this, &ReceiverInterface::onMessageReceived);
    getStatus();
}

ReceiverInterface::~ReceiverInterface() = default;

bool ReceiverInterface::launch(const QString& app_id) {
    QJsonObject msg;
    msg["type"] = QStringLiteral("LAUNCH");
    msg["requestId"] = ++last_request_;
    msg["appId"] = app_id;
    QJsonDocument doc(msg);
    return send(QString(doc.toJson(QJsonDocument::Compact)));
}

bool ReceiverInterface::stop(const QString& session_id) {
    QJsonObject msg;
    msg["type"] = QStringLiteral("STOP");
    msg["requestId"] = ++last_request_;
    msg["sessionId"] = session_id;
    QJsonDocument doc(msg);
    return send(QString(doc.toJson(QJsonDocument::Compact)));
}

bool ReceiverInterface::getStatus() {
    QJsonObject msg;
    msg["type"] = QStringLiteral("GET_STATUS");
    msg["requestId"] = ++last_request_;
    QJsonDocument doc(msg);
    return send(QString(doc.toJson(QJsonDocument::Compact)));
}

void ReceiverInterface::onMessageReceived(const QString& data) {
    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(data.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "Could not parse message on connection channel:"
                   << err.errorString();
        return;
    }
    if (doc.object()["type"].toString() != "RECEIVER_STATUS") return;

    const auto status = doc.object()["status"].toObject();
    applications_.clear();
    for (const auto& item : status["applications"].toArray()) {
        const auto obj = item.toObject();
        QVariantMap app;
        app["appId"] = obj["appId"].toString();
        app["displayName"] = obj["displayName"].toString();
        QStringList namespaces;
        for (const auto& n : obj["namespaces"].toArray()) {
            namespaces.push_back(n.toString());
        }
        app["namespaces"] = namespaces;
        app["sessionId"] = obj["sessionId"].toString();
        app["statusText"] = obj["statusText"].toString();
        app["transportId"] = obj["transportId"].toString();
        app["isIdleScreen"] = obj["isIdleScreen"].toBool();
        applications_.push_back(std::move(app));
    }
    is_active_input_ = status["isActiveInput"].toBool();
    const auto volume = status["volume"].toObject();
    volume_level_ = volume["level"].toDouble();
    volume_muted_ = volume["muted"].toBool();

    Q_EMIT statusChanged();
}

}
