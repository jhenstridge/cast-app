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

#include "browser.h"

#include <avahi-common/error.h>

#include <cstdio>
#include <stdexcept>
#include <string>

namespace cast {

Browser::Browser(QObject *parent)
    : QAbstractListModel(parent) {
    poll_.reset(avahi_glib_poll_new(nullptr, G_PRIORITY_DEFAULT));
    if (!poll_) {
        throw std::runtime_error("Could not create AvahiGLibPoll");
    }

    AvahiServerConfig config_backing { nullptr, };
    std::unique_ptr<AvahiServerConfig, decltype(&avahi_server_config_free)>
        config(avahi_server_config_init(&config_backing),
               avahi_server_config_free);
    config->disable_publishing = true;

    int error = 0;
    server_.reset(avahi_server_new(
                      avahi_glib_poll_get(poll_.get()), config.get(),
                      &Browser::serverCallback, this, &error));
    if (!server_) {
        throw std::runtime_error(std::string("Could not create AvahiServer: ")
                                 + avahi_strerror(error));
    }
}

Browser::~Browser() = default;

QString Browser::serviceType() const {
    return service_type_;
}

void Browser::setServiceType(QString type) {
    service_type_ = type;
    startBrowsing();
}

void Browser::startBrowsing() {
    if (service_type_.isEmpty()) return;
    if (!server_ || avahi_server_get_state(server_.get()) != AVAHI_SERVER_RUNNING) return;

    browser_.reset(
        avahi_s_service_browser_new(
            server_.get(), AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC,
            service_type_.toUtf8().constData(),
            "local", AVAHI_LOOKUP_USE_MULTICAST,
            &Browser::browserCallback, this));
}

void Browser::serverCallback(AvahiServer *server,
                             AvahiServerState state,
                             void *userdata) noexcept {
    auto browser = static_cast<Browser*>(userdata);

    switch (state) {
    case AVAHI_SERVER_RUNNING:
        browser->startBrowsing();
        break;
    default:
        break;
    }
}

void Browser::browserCallback(AvahiSServiceBrowser *browser,
                              AvahiIfIndex iface,
                              AvahiProtocol protocol,
                              AvahiBrowserEvent event,
                              const char *name,
                              const char *type,
                              const char *domain,
                              AvahiLookupResultFlags flags,
                              void *userdata)  noexcept {
    printf("Found %s of type %s in domain %s\n", name, type, domain);
}

int Browser::rowCount(const QModelIndex &parent) const {
    return 0;
}

QVariant Browser::data(const QModelIndex &index, int role) const {
    return QVariant();
}

QHash<int,QByteArray> Browser::roleNames() const {
    return QHash<int,QByteArray>();
}

}
