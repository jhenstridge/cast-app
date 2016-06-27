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

#include <QAbstractListModel>
#include <avahi-glib/glib-watch.h>
#include <avahi-core/core.h>
#include <avahi-core/lookup.h>

#include <memory>

namespace cast {

class Browser : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(QString serviceType READ serviceType WRITE setServiceType)
public:
    explicit Browser(QObject *parent=nullptr);
    virtual ~Browser();

    int rowCount(const QModelIndex &parent=QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    enum Roles {
        Role
    };
    Q_ENUM(Roles);

protected:
    QHash<int,QByteArray> roleNames() const override;

private:
    QString serviceType() const;
    void setServiceType(QString type);

    void startBrowsing();

    static void serverCallback(AvahiServer *server,
                               AvahiServerState state,
                               void *userdata) noexcept;
    static void browserCallback(AvahiSServiceBrowser *browser,
                                AvahiIfIndex iface,
                                AvahiProtocol protocol,
                                AvahiBrowserEvent event,
                                const char *name,
                                const char *type,
                                const char *domain,
                                AvahiLookupResultFlags flags,
                                void *userdata)  noexcept;

    std::unique_ptr<AvahiGLibPoll, decltype(&avahi_glib_poll_free)> poll_
        {nullptr, avahi_glib_poll_free};
    std::unique_ptr<AvahiServer, decltype(&avahi_server_free)> server_
        {nullptr, avahi_server_free};
    std::unique_ptr<AvahiSServiceBrowser, decltype(&avahi_s_service_browser_free)> browser_
        {nullptr, avahi_s_service_browser_free};
    QString service_type_;
};

}
