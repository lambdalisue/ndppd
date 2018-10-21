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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <net/if.h>

#include "ndppd.h"
#include "rule.h"
#include "proxy.h"
#include "iface.h"

using namespace ndppd;

bool rule::_any_aut = false;

bool rule::_any_iface = false;

bool rule::_any_static = false;

rule::rule()
{
}

std::shared_ptr<rule>
rule::create(const std::shared_ptr<proxy> &pr, const Cidr &cidr, const std::shared_ptr<iface> &ifa)
{
    std::shared_ptr<rule> ru(new rule());
    ru->_ptr = ru;
    ru->_pr = pr;
    ru->_daughter = ifa;
    ru->_cidr = cidr;
    ru->_aut = false;
    _any_iface = true;
    unsigned int ifindex;

    ifindex = if_nametoindex(pr->ifa()->name().c_str());
#ifdef WITH_ND_NETLINK
    if_add_to_list(ifindex, pr->ifa());
#endif
    ifindex = if_nametoindex(ifa->name().c_str());
#ifdef WITH_ND_NETLINK
    if_add_to_list(ifindex, ifa);
#endif

    Logger::debug() << "rule::create() if=" << pr->ifa()->name() << ", slave=" << ifa->name() << ", cidr=" << cidr;

    return ru;
}

std::shared_ptr<rule> rule::create(const std::shared_ptr<proxy> &pr, const Cidr &cidr, bool aut)
{
    std::shared_ptr<rule> ru(new rule());
    ru->_ptr = ru;
    ru->_pr = pr;
    ru->_cidr = cidr;
    ru->_aut = aut;
    _any_aut = _any_aut || aut;

    if (aut == false)
        _any_static = true;

    Logger::debug()
            << "rule::create() if=" << pr->ifa()->name().c_str() << ", cidr=" << cidr
            << ", auto=" << (aut ? "yes" : "no");

    return ru;
}

const Cidr &rule::cidr() const
{
    return _cidr;
}

std::shared_ptr<ndppd::iface> rule::daughter() const
{
    return _daughter;
}

bool rule::is_auto() const
{
    return _aut;
}

bool rule::autovia() const
{
    return _autovia;
}

void rule::autovia(bool val)
{
    _autovia = val;
}

bool rule::any_auto()
{
    return _any_aut;
}

bool rule::any_iface()
{
    return _any_iface;
}

bool rule::any_static()
{
    return _any_static;
}

bool rule::check(const Address &addr) const
{
    return _cidr % addr;
}

