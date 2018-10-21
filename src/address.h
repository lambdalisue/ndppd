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
#include "logger.h"

namespace ndppd {
    class Address {
    public:
        Address();

        explicit Address(const in6_addr &addr);

        explicit Address(const std::string &address);

        const in6_addr &c_addr() const;

        in6_addr &addr();

        bool is_unicast() const;

        bool is_multicast() const;

        std::string to_string() const;

        bool operator==(const Address &address) const;
        bool operator!=(const Address &address) const;
        bool operator<(const Address &address) const;

        explicit operator bool() const;

    private:
        in6_addr _addr;
    };

    Logger &operator<<(Logger &logger, const Address &address);

}
