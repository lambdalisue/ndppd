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

bool Rule::_any_aut = false;

bool Rule::_any_iface = false;

bool Rule::_any_static = false;

Rule::Rule()
{
}

std::shared_ptr<Rule>
Rule::create(const std::shared_ptr<Proxy> &pr, const Cidr &cidr, const std::shared_ptr<iface> &ifa)
{
    std::shared_ptr<Rule> rule(new Rule());
    rule->_ptr = rule;
    rule->_pr = pr;
    rule->_daughter = ifa;
    rule->_cidr = cidr;
    rule->_aut = false;
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

    return rule;
}

std::shared_ptr<Rule> Rule::create(const std::shared_ptr<Proxy> &pr, const Cidr &cidr, bool aut)
{
    std::shared_ptr<Rule> rule(new Rule());
    rule->_ptr = rule;
    rule->_pr = pr;
    rule->_cidr = cidr;
    rule->_aut = aut;
    _any_aut = _any_aut || aut;

    if (aut == false)
        _any_static = true;

    Logger::debug()
            << "rule::create() if=" << pr->ifa()->name().c_str() << ", cidr=" << cidr
            << ", auto=" << (aut ? "yes" : "no");

    return rule;
}

const Cidr &Rule::cidr() const
{
    return _cidr;
}

std::shared_ptr<ndppd::iface> Rule::daughter() const
{
    return _daughter;
}

bool Rule::is_auto() const
{
    return _aut;
}

bool Rule::autovia() const
{
    return _autovia;
}

void Rule::autovia(bool val)
{
    _autovia = val;
}

bool Rule::any_auto()
{
    return _any_aut;
}

bool Rule::any_iface()
{
    return _any_iface;
}

bool Rule::any_static()
{
    return _any_static;
}

bool Rule::check(const Address &addr) const
{
    return _cidr % addr;
}

