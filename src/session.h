// ndppd - NDP Proxy Daemon
// Copyright (C) 2011-2018  Daniel Adolfsson <daniel@priv.nu>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef NDPPD_SESSION_H
#define NDPPD_SESSION_H

#include <vector>
#include <string>
#include <set>

#include "ndppd.h"

NDPPD_NS_BEGIN

class Proxy;

class Interface;

class Session {
private:
    std::weak_ptr<Session> _ptr;

    std::weak_ptr<Proxy> _pr;

    Address _saddr, _daddr, _taddr;

    bool _autowire;

    bool _keepalive;

    bool _wired;

    Address _wired_via;

    bool _touched;

    // An array of interfaces this session is monitoring for
    // ND_NEIGHBOR_ADVERT on.
    std::list<std::shared_ptr<Interface> > _ifaces;

    std::set<Address> _pending;

    // The remaining time in miliseconds the object will stay in the
    // interface's session array or cache.
    int _ttl;

    int _fails;

    int _retries;

    int _status;

    static std::list<std::weak_ptr<Session> > _sessions;

public:
    enum {
        WAITING,  // Waiting for an advert response.
        RENEWING, // Renewing;
        VALID,    // Valid;
        INVALID   // Invalid;
    };

    static void update_all(int elapsed_time);

    // Destructor.
    ~Session();

    static std::shared_ptr<Session>
    create(const std::shared_ptr<Proxy>& pr, const Address& taddr, bool autowire, bool keepalive, int retries);

    void add_iface(const std::shared_ptr<Interface>& ifa);

    void add_pending(const Address& addr);

    const Address& taddr() const;

    const Address& daddr() const;

    const Address& saddr() const;

    bool autowire() const;

    int retries() const;

    int fails() const;

    bool keepalive() const;

    bool wired() const;

    bool touched() const;

    int status() const;

    void status(int val);

    void handle_advert();

    void handle_advert(const Address& saddr, const std::string& ifname, bool use_via);

    void touch();

    void send_advert(const Address& daddr);

    void send_solicit();

    void refesh();
};

NDPPD_NS_END

#endif
