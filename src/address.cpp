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

#include <netinet/in.h>
#include <string>
#include <arpa/inet.h>
#include <cassert>
#include <stdexcept>
#include <cstring>

#include "ndppd.h"
#include "address.h"

NDPPD_NS_BEGIN

Address::Address()
        : addr {}
{
}

Address::Address(const in6_addr& addr)
        : addr(addr)
{

}

Address::Address(const std::string& address)
{
    if (::inet_pton(AF_INET6, address.c_str(), &addr) <= 0)
        throw std::runtime_error("Failed to parse IPv6 address");
}

bool Address::is_multicast() const
{
    return addr.s6_addr[0] == 0xff;
}

bool Address::is_unicast() const
{
    return (addr.s6_addr32[2] != 0 || addr.s6_addr32[3] != 0) && addr.s6_addr[0] != 0xff;
}

std::string Address::to_string() const
{
    char buf[INET6_ADDRSTRLEN];
    assert(::inet_ntop(AF_INET6, &addr, buf, INET6_ADDRSTRLEN) != nullptr);
    return buf;
}

bool Address::operator==(const Address& address) const
{
    return addr.s6_addr32[0] == address.addr.s6_addr32[0] &&
           addr.s6_addr32[1] == address.addr.s6_addr32[1] &&
           addr.s6_addr32[2] == address.addr.s6_addr32[2] &&
           addr.s6_addr32[3] == address.addr.s6_addr32[3];
}

bool Address::operator!=(const Address& address) const
{
    return !(*this == address);
}

bool Address::operator<(const ndppd::Address& address) const
{
    return std::memcmp(&addr, &address.addr, sizeof(in6_addr)) < 0;
}

Address::operator bool() const
{
    return addr.s6_addr32[0] != 0 || addr.s6_addr32[1] != 0 || addr.s6_addr32[2] != 0 || addr.s6_addr32[3] != 0;
}

Logger& operator<<(Logger& logger, const Address& address)
{
    logger << address.to_string();
    return logger;
}

NDPPD_NS_END
