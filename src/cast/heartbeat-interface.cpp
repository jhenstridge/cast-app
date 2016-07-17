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

#include "heartbeat-interface.h"

namespace cast {

const std::string HeartbeatInterface::URN = "urn:x-cast:com.google.cast.tp.heartbeat";

HeartbeatInterface::HeartbeatInterface(Channel *channel)
    : Interface(channel, URN) {
    connect(&timer_, &QTimer::timeout,
            this, &HeartbeatInterface::onTimeout);
    timer_.setInterval(5000);
    timer_.setSingleShot(false);
    timer_.start();
}

HeartbeatInterface::~HeartbeatInterface() = default;

void HeartbeatInterface::onTimeout()
{
    send(R"({"type": "PING"})");
}

}
