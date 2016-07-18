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

class ReceiverInterface : public Interface {
    Q_OBJECT
    Q_PROPERTY(QVariantList applications READ applications NOTIFY statusChanged)
    Q_PROPERTY(bool isActiveInput READ isActiveInput NOTIFY statusChanged)
    Q_PROPERTY(double volumeLevel READ volumeLevel NOTIFY statusChanged)
    Q_PROPERTY(bool volumeMuted READ volumeMuted NOTIFY statusChanged)
public:
    ReceiverInterface(Channel *channel);
    virtual ~ReceiverInterface();

    static const QString URN;

    bool launch(const QString& app_id);
    bool stop(const QString& session_id);
    bool getStatus();

Q_SIGNALS:
    void statusChanged();

private Q_SLOTS:
    void onMessageReceived(const QString& data);

private:
    QVariantList applications() const { return applications_; }
    bool isActiveInput() const { return is_active_input_; }
    double volumeLevel() const { return volume_level_; }
    bool volumeMuted() const { return volume_muted_; }

    int last_request_ = 0;

    QVariantList applications_;
    bool is_active_input_ = false;
    double volume_level_ = 1.0;
    bool volume_muted_ = false;
};

}
