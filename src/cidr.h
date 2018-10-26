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

#ifndef NDPPD_CIDR_H
#define NDPPD_CIDR_H

#include <netinet/in.h>

#include "ndppd.h"
#include "address.h"

NDPPD_NS_BEGIN

class Cidr {
public:
    in6_addr addr;

    Cidr();

    explicit Cidr(std::string string);

    explicit Cidr(const in6_addr& addr, int prefix = 128);

    explicit operator bool() const;

    const std::string to_string() const;

    void prefix(int prefix);

    int prefix() const;

    bool operator%(const Address& address) const;

private:
    int _prefix;
};

Logger& operator<<(Logger& logger, const Cidr& cidr);

NDPPD_NS_END

#endif
