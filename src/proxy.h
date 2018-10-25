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

#ifndef NDPPD_PROXY_H
#define NDPPD_PROXY_H

#include <string>
#include <vector>
#include <map>
#include <list>
#include <sys/poll.h>
#include <memory>

#include "ndppd.h"
#include "range.h"
#include "cidr.h"
#include "session.h"

NDPPD_NS_BEGIN

class Interface;

class Rule;

class Proxy {
public:
    bool router;
    bool autowire;
    int retries;
    bool keepalive;
    int ttl;
    int deadtime;
    int timeout;

    static Proxy& create(const std::string& ifname, bool promiscuous);

    static const Range<std::list<std::unique_ptr<Proxy>>::const_iterator> proxies();

    Proxy(const std::shared_ptr<Interface>& iface, bool promisc);

    ~Proxy();

    Session* find_or_create_session(const Address& taddr);

    void handle_solicit(const Address& saddr, const Address& taddr);

    void remove_session(Session& session);

    Rule& add_rule(const Cidr& cidr, const std::shared_ptr<Interface>& iface);

    Rule& add_rule(const Cidr& cidr, bool auto_ = false);

    const std::shared_ptr<Interface>& iface() const;

    bool promiscuous() const;

    const Range<std::list<std::unique_ptr<Rule> >::const_iterator> rules() const;

private:
    std::shared_ptr<Interface> _iface;

    std::list<std::unique_ptr<Rule> > _rules;

    std::list<std::unique_ptr<Session> > _sessions;

    bool _promiscuous;
};

NDPPD_NS_END

#endif
