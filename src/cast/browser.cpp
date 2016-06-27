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
#include <algorithm>
#include <stdexcept>
#include <string>

namespace cast {

struct Browser::Service {
    QString service_name;
    QString host_name;
    QString address;
    uint16_t port = 0;
    QStringList txt;

    std::unique_ptr<AvahiSServiceResolver, decltype(&avahi_s_service_resolver_free)> resolver
        {nullptr, avahi_s_service_resolver_free};

    Service(const QString& name) : service_name(name) {}
    bool operator<(const Browser::Service& other) const {
        return service_name < other.service_name; }
};

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
    config->publish_hinfo = false;
    config->publish_addresses = false;
    config->publish_workstation = false;
    config->publish_domain = false;
    config->disable_publishing = true;

    int error = 0;
    server_.reset(avahi_server_new(
                      avahi_glib_poll_get(poll_.get()), config.get(),
                      &Browser::serverCallback, this, &error));
    if (!server_) {
        throw std::runtime_error(std::string("Could not create AvahiServer: ")
                                 + avahi_strerror(error));
    }

    roles[RoleServiceName] = "serviceName";
    roles[RoleHostName] = "hostName";
    roles[RoleAddress] = "address";
    roles[RolePort] = "port";
    roles[RoleTxt] = "txt";
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
    if (!services_.empty()) {
        beginResetModel();
        services_.clear();
        endResetModel();
    }

    if (service_type_.isEmpty()) return;
    if (!server_ || avahi_server_get_state(server_.get()) != AVAHI_SERVER_RUNNING) return;

    browser_.reset(
        avahi_s_service_browser_new(
            server_.get(), AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC,
            service_type_.toUtf8().constData(),
            "local", AVAHI_LOOKUP_USE_MULTICAST,
            &Browser::browserCallback, this));
}

void Browser::addService(AvahiIfIndex iface,
                         AvahiProtocol protocol,
                         const char *name,
                         const char *type,
                         const char *domain) {
    Browser::Service svc(name);
    if (!std::binary_search(services_.begin(), services_.end(), svc)) {
            // Start resolving the service
            svc.resolver.reset(
                avahi_s_service_resolver_new(
                    server_.get(), iface, protocol,
                    name, type, domain, AVAHI_PROTO_UNSPEC,
                    static_cast<AvahiLookupFlags>(0),
                    &Browser::resolverCallback, this));

            auto it = std::upper_bound(services_.begin(), services_.end(), svc);
            int index = it - services_.begin();
            beginInsertRows(QModelIndex(), index, index);
            services_.emplace(it, std::move(svc));
            endInsertRows();
        }
}

void Browser::removeService(const char *name) {
    Browser::Service svc(name);
    auto it = std::lower_bound(services_.begin(), services_.end(), svc);
    if (it != services_.end() && it->service_name == svc.service_name) {
        int index = it - services_.begin();
        beginRemoveRows(QModelIndex(), index, index);
        services_.erase(it);
        endRemoveRows();
    }
}

void Browser::updateService(const char *name,
                            const char *host_name,
                            const AvahiAddress *a,
                            uint16_t port,
                            AvahiStringList *txt) {
    // Find the service in our list
    Browser::Service s(name);
    auto it = std::lower_bound(services_.begin(), services_.end(), s);
    if (it == services_.end() || it->service_name != name) {
        return;
    }
    Browser::Service &svc = *it;
    auto index = createIndex(it - services_.begin(), 0);

    svc.host_name = host_name;
    char addr[AVAHI_ADDRESS_STR_MAX];
    avahi_address_snprint(addr, sizeof(addr), a);
    svc.address = addr;
    svc.port = port;
    svc.txt.clear();
    for (auto item = txt; item != nullptr; item = avahi_string_list_get_next(item)) {
        // TODO: this probably isn't actually UTF-8
        svc.txt.push_front(
            QString::fromUtf8(
                reinterpret_cast<const char*>(avahi_string_list_get_text(item)),
                avahi_string_list_get_size(item)));
    }
    Q_EMIT dataChanged(index, index);
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

void Browser::browserCallback(AvahiSServiceBrowser *b,
                              AvahiIfIndex iface,
                              AvahiProtocol protocol,
                              AvahiBrowserEvent event,
                              const char *name,
                              const char *type,
                              const char *domain,
                              AvahiLookupResultFlags flags,
                              void *userdata)  noexcept {
    auto browser = static_cast<Browser*>(userdata);

    switch (event) {
    case AVAHI_BROWSER_NEW:
        printf("Found %s of type %s in domain %s\n", name, type, domain);
        browser->addService(iface, protocol, name, type, domain);
        break;
    case AVAHI_BROWSER_REMOVE:
        printf("Removing %s of type %s in domain %s\n", name, type, domain);
        browser->removeService(name);
        break;
    case AVAHI_BROWSER_FAILURE:
        printf("Error: %s\n", avahi_strerror(avahi_server_errno(browser->server_.get())));
        break;
    default:
        break;
    }
}

void Browser::resolverCallback(AvahiSServiceResolver *r,
                               AvahiIfIndex iface,
                               AvahiProtocol protocol,
                               AvahiResolverEvent event,
                               const char *name,
                               const char *type,
                               const char *domain,
                               const char *host_name,
                               const AvahiAddress *a,
                               uint16_t port,
                               AvahiStringList *txt,
                               AvahiLookupResultFlags flags,
                               void *userdata) noexcept {
    auto browser = static_cast<Browser*>(userdata);

    switch (event) {
    case AVAHI_RESOLVER_FOUND:
        printf("Resolved %s to %s\n", name, host_name);
        browser->updateService(name, host_name, a, port, txt);
        break;
    case AVAHI_RESOLVER_FAILURE:
        browser->removeService(name);
        break;
    }
}

int Browser::rowCount(const QModelIndex &parent) const {
    return services_.size();
}

QVariant Browser::data(const QModelIndex &index, int role) const {
    int i = index.row();
    if (i < 0 || i > static_cast<int>(services_.size())) return QVariant();

    const Browser::Service& svc = services_[i];
    switch (role) {
    case RoleServiceName:
        return QVariant(svc.service_name);
    case RoleHostName:
        return QVariant(svc.host_name);
    case RoleAddress:
        return QVariant(svc.address);
    case RolePort:
        return QVariant(svc.port);
    case RoleTxt:
        return QVariant(svc.txt);
    default:
        return QVariant();
    }
}

QHash<int,QByteArray> Browser::roleNames() const {
    return roles;
}

}
