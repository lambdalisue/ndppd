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

#include "nl_route.h"
#include "ndppd.h"

NDPPD_NS_BEGIN

NetlinkRoute::NetlinkRoute(const ndppd::Cidr& cidr, const std::shared_ptr<ndppd::Interface>& iface)
        : _cidr(cidr), _iface(iface)
{
}

const Cidr& NetlinkRoute::cidr() const
{
    return _cidr;
}

const std::shared_ptr<Interface>& NetlinkRoute::iface() const
{
    return _iface;
}

NDPPD_NS_END
