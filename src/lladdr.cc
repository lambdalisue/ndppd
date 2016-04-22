// ndppd - NDP Proxy Daemon
// Copyright (C) 2011-2016  Daniel Adolfsson <daniel@priv.nu>
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
#include <string>
#include <vector>
#include <map>

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cctype>

#include <netinet/ip6.h>
#include <arpa/inet.h>

#include "lladdr.h"

NDPPD_NS_BEGIN

std::string lladdr::to_string() const
{
    char buf[18];
    sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x",
        _v[0], _v[1], _v[2], _v[3], _v[4], _v[5]);
    return buf;
}

NDPPD_NS_END
