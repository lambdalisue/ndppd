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

#ifndef NDPPD_NETLINK_H
#define NDPPD_NETLINK_H

#include <memory>
#include <set>
#include "ndppd.h"
#include "socket.h"
#include "address.h"
#include "range.h"

NDPPD_NS_BEGIN

class Netlink {
public:
    static void initialize();

    static void finalize();

    static const Range<std::set<Address>::const_iterator> local_addresses();

    static void load_local_ips();

    static bool is_local(const Address& address);

    static void test();
};

NDPPD_NS_END

#endif
