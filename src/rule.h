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

#ifndef NDPPD_RULE_H
#define NDPPD_RULE_H

#include <string>
#include <vector>
#include <map>
#include <list>
#include <memory>
#include <sys/poll.h>

#include "ndppd.h"
#include "cidr.h"

NDPPD_NS_BEGIN

class Interface;

class Proxy;

class Rule {
public:
    Rule(Proxy& proxy, const Cidr& cidr, const std::shared_ptr<Interface>& ifa);

    Rule(Proxy& proxy, const Cidr& cidr, bool auto_ = true);

    ~Rule();

    Proxy& proxy() const;

    const Cidr& cidr() const;

    const std::shared_ptr<Interface>& iface() const;

    bool is_auto() const;

private:
    Proxy& _proxy;

    // Interface for querying internal network.
    std::shared_ptr<Interface> _iface;

    Cidr _cidr;

    bool _auto;
};

NDPPD_NS_END

#endif
