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
#include "interface.h"

NDPPD_NS_BEGIN

Rule::Rule(Proxy& proxy, const Cidr& cidr, const std::shared_ptr<Interface>& iface)
        : _proxy(proxy), _cidr(cidr), _iface(iface), _aut(false)
{
    Logger::debug() << "Rule::Rule() if=" << proxy.iface()->name() << ", slave=" << iface->name() << ", cidr=" << cidr;
}

Rule::Rule(Proxy& proxy, const Cidr& cidr, bool aut)
        : _proxy(proxy), _cidr(cidr), _iface {}, _aut(aut)
{
    Logger::debug() << "Rule::Rule() if=" << proxy.iface()->name() << ", cidr=" << cidr
                    << ", auto=" << (aut ? "yes" : "no");
}

const Cidr& Rule::cidr() const
{
    return _cidr;
}

const std::shared_ptr<ndppd::Interface>& Rule::iface() const
{
    return _iface;
}

bool Rule::is_auto() const
{
    return _aut;
}

NDPPD_NS_END
