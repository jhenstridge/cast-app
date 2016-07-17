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

#include "caster.h"

#include <QByteArray>
#include <QObject>
#include <QString>

namespace cast {

class Channel : public QObject {
    Q_OBJECT
public:
    Channel(Caster *caster, const QString& source_id,
            const QString& destination_id, const QString& channel_namespace);
    virtual ~Channel();

    Q_INVOKABLE bool send(const QString& data);
    Q_INVOKABLE bool sendBinary(const QByteArray& data);

Q_SIGNALS:
    void messageReceived(const QString& data);
    void binaryMessageReceived(const QByteArray& data);

private Q_SLOTS:
    void onMessageReceived(const Caster::Message& message);

private:
    const QString source_id_;
    const QString destination_id_;
    const QString namespace_;
};

}
