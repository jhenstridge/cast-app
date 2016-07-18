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

class Channel;

class Interface : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString namespace READ getNamespace CONSTANT)
public:
    Interface(Channel *channel, const QString& ns);
    virtual ~Interface();

    Q_INVOKABLE bool send(const QString& data);
    Q_INVOKABLE bool sendBinary(const QByteArray& data);

Q_SIGNALS:
    void messageReceived(const QString& data);
    void binaryMessageReceived(const QByteArray& data);

protected:
    Channel& channel();
    const Channel& channel() const;

private:
    void handleMessage(const Caster::Message& message);
    const QString& getNamespace() const { return namespace_; }

    const QString namespace_;

    friend class Channel;
};

}
