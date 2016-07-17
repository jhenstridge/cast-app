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

#include "channel.h"
#include "interface.h"
#include "connection-interface.h"
#include "heartbeat-interface.h"
#include "receiver-interface.h"

namespace cast {

Channel::Channel(Caster *caster, const QString& source_id,
                 const QString& destination_id)
    : QObject(caster), source_id_(source_id),
      destination_id_(destination_id) {
    addInterface(ConnectionInterface::URN);
}

Channel::~Channel() = default;

cast::Interface* Channel::addInterface(const QString& ns) {
    Interface *iface;

    try {
        iface = interfaces_.at(ns);
    } catch (const std::out_of_range &) {
        // TODO: add some kind of registration system
        if (ns == ConnectionInterface::URN) {
            iface = new ConnectionInterface(this);
        } else if (ns == HeartbeatInterface::URN) {
            iface = new HeartbeatInterface(this);
        } else if (ns == ReceiverInterface::URN) {
            iface = new ReceiverInterface(this);
        } else {
            iface = new Interface(this, ns);
        }
        interfaces_.emplace(ns, iface);
    }
    return iface;
}

void Channel::close() {
    if (closed_) return;
    closed_ = true;

    for (auto it = interfaces_.begin(); it != interfaces_.end(); ++it) {
        it->second->deleteLater();
    }
    interfaces_.clear();

    Q_EMIT closed();
}

Caster& Channel::caster() {
    return *static_cast<Caster*>(parent());
}

const Caster& Channel::caster() const {
    return *static_cast<const Caster*>(parent());
}

void Channel::handleMessage(const Caster::Message& message) {
    const QString ns = QString::fromStdString(message.namespace_());
    try {
        interfaces_.at(ns)->handleMessage(message);
    } catch (const std::out_of_range &) {
        qWarning() << "Message received for unknown namespace:"
                   << QString::fromStdString(message.namespace_());
    }
}

}
