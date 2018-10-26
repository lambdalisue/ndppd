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

#ifndef NDPPD_NL_ROUTE_H
#define NDPPD_NL_ROUTE_H

#include "ndppd.h"
#include "cidr.h"
#include "interface.h"

NDPPD_NS_BEGIN

class NetlinkRoute {
public:
    NetlinkRoute(const Cidr& cidr, const std::shared_ptr<Interface>& iface);

    const Cidr& cidr() const;

    const std::shared_ptr<Interface>& iface() const;

private:
    Cidr _cidr;
    std::shared_ptr<Interface> _iface;
};

NDPPD_NS_END

#endif // NDPPD_NL_ROUTE_H
