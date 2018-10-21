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

#pragma once

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

namespace ndppd
{
    class Interface;

    class Rule;

    class Proxy
    {
    public:
        static std::shared_ptr<Proxy> create(const std::shared_ptr<Interface> &ifa, bool promiscuous);

        static std::shared_ptr<Proxy> find_aunt(const std::string &ifname, const Address &taddr);

        static std::shared_ptr<Proxy> open(const std::string &ifn, bool promiscuous);

        std::shared_ptr<Session> find_or_create_session(const Address &taddr);

        void handle_advert(const Address &saddr, const Address &taddr, const std::string &ifname, bool use_via);

        void
        handle_stateless_advert(const Address &saddr, const Address &taddr, const std::string &ifname, bool use_via);

        void handle_solicit(const Address &saddr, const Address &taddr, const std::string &ifname);

        void remove_session(const std::shared_ptr<Session> &session);

        std::shared_ptr<Rule> add_rule(const Cidr &cidr, const std::shared_ptr<Interface> &ifa, bool autovia);

        std::shared_ptr<Rule> add_rule(const Cidr &cidr, bool aut = false);

        const std::shared_ptr<Interface> &ifa() const;

        bool promiscuous() const;

        bool router() const;

        void router(bool val);

        bool autowire() const;

        void autowire(bool val);

        int retries() const;

        void retries(int val);

        bool keepalive() const;

        void keepalive(bool val);

        int timeout() const;

        void timeout(int val);

        int ttl() const;

        void ttl(int val);

        int deadtime() const;

        void deadtime(int val);

        const Range<std::list<std::shared_ptr<Rule> >::const_iterator> rules() const;

    private:
        static std::list<std::shared_ptr<Proxy> > _list;

        std::weak_ptr<Proxy> _ptr;

        std::shared_ptr<Interface> _ifa;

        std::list<std::shared_ptr<Rule> > _rules;

        std::list<std::shared_ptr<Session> > _sessions;

        bool _promiscuous;

        bool _router;

        bool _autowire;

        int _retries;

        bool _keepalive;

        int _ttl, _deadtime, _timeout;

        Proxy();
    };

}
