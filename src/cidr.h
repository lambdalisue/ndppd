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

#pragma once

#include <netinet/in.h>
#include "address.h"

namespace ndppd
{
    class Cidr
    {
    public:
        Cidr();

        Cidr(const Cidr &cidr);

        explicit Cidr(std::string string);

        explicit Cidr(const in6_addr &addr, int prefix = 128);

        const struct in6_addr &addr() const;

        explicit operator bool() const;

        const std::string to_string() const;

        int prefix() const;

        bool operator%(const Address &address) const;

    private:
        in6_addr _addr;
        int _prefix;
    };

    Logger &operator<<(Logger &logger, const Cidr &cidr);
}
