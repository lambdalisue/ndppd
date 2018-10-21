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

#include <string>
#include <vector>
#include <map>
#include <list>

#include <sys/poll.h>
#include <memory>

#include "cidr.h"

namespace ndppd
{
    class Interface;

    class Proxy;

    class Rule
    {
    public:
        static std::shared_ptr<Rule>
        create(const std::shared_ptr<Proxy> &pr, const Cidr &cidr, const std::shared_ptr<Interface> &ifa);

        static std::shared_ptr<Rule> create(const std::shared_ptr<Proxy> &pr, const Cidr &cidr, bool stc = true);

        const Cidr &cidr() const;

        std::shared_ptr<Interface> daughter() const;

        bool is_auto() const;

        bool check(const Address &addr) const;

        static bool any_auto();

        static bool any_static();

        static bool any_iface();

        bool autovia() const;

        void autovia(bool val);

    private:
        std::weak_ptr<Rule> _ptr;

        std::weak_ptr<Proxy> _pr;

        std::shared_ptr<Interface> _daughter;

        Cidr _cidr;

        bool _aut;

        static bool _any_aut;

        static bool _any_static;

        static bool _any_iface;

        bool _autovia;

        Rule();
    };
}
