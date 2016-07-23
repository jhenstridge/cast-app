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

#include "interface.h"
#include <QVariantList>

namespace cast {

class MediaInterface : public Interface {
    Q_OBJECT
    Q_PROPERTY(QVariantList status READ status NOTIFY statusChanged)
public:
    MediaInterface(Channel *channel);
    virtual ~MediaInterface();

    static const QString URN;

    bool getStatus();
    Q_INVOKABLE bool play(int media_session_id);
    Q_INVOKABLE bool pause(int media_session_id);
    Q_INVOKABLE bool stop(int media_session_id);

Q_SIGNALS:
    void statusChanged();

private Q_SLOTS:
    void onMessageReceived(const QString& data);

private:
    QVariantList status() const { return status_; }

    int last_request_ = 0;

    QVariantList status_;
};

}
