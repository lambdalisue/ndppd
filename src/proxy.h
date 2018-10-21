// ndppd - NDP Proxy Daemon
// Copyright (C) 2011  Daniel Adolfsson <daniel@priv.nu>
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

#include <sys/poll.h>

#include "ndppd.h"
#include "range.h"

NDPPD_NS_BEGIN

class iface;
class rule;

class proxy {
public:    
    static std::shared_ptr<proxy> create(const std::shared_ptr<iface>& ifa, bool promiscuous);
    
    static std::shared_ptr<proxy> find_aunt(const std::string& ifname, const address& taddr);

    static std::shared_ptr<proxy> open(const std::string& ifn, bool promiscuous);
    
    std::shared_ptr<session> find_or_create_session(const address& taddr);
    
    void handle_advert(const address& saddr, const address& taddr, const std::string& ifname, bool use_via);
    
    void handle_stateless_advert(const address& saddr, const address& taddr, const std::string& ifname, bool use_via);
    
    void handle_solicit(const address& saddr, const address& taddr, const std::string& ifname);

    void remove_session(const std::shared_ptr<session>& se);

    std::shared_ptr<rule> add_rule(const address& addr, const std::shared_ptr<iface>& ifa, bool autovia);

    std::shared_ptr<rule> add_rule(const address& addr, bool aut = false);
    
    std::list<std::shared_ptr<rule> >::iterator rules_begin();
    
    std::list<std::shared_ptr<rule> >::iterator rules_end();

    const std::shared_ptr<iface>& ifa() const;
    
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

    Range<std::list<std::shared_ptr<rule> >::const_iterator> rules() const {
        return {_rules.cbegin(), _rules.cend()};
    }


private:
    static std::list<std::shared_ptr<proxy> > _list;

    std::weak_ptr<proxy> _ptr;

    std::shared_ptr<iface> _ifa;

    std::list<std::shared_ptr<rule> > _rules;

    std::list<std::shared_ptr<session> > _sessions;
    
    bool _promiscuous;

    bool _router;
    
    bool _autowire;
    
    int _retries;
    
    bool _keepalive;

    int _ttl, _deadtime, _timeout;

    proxy();
};

NDPPD_NS_END
