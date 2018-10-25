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

#ifndef NDPPD_NL_ADDRESS_H
#define NDPPD_NL_ADDRESS_H

#include <memory>

#include "ndppd.h"
#include "address.h"
#include "interface.h"

NDPPD_NS_BEGIN

class NetlinkAddress {
public:
    NetlinkAddress(const Address& address, const std::shared_ptr<Interface>& iface);

    const Address& address() const;

    const std::shared_ptr<Interface>& iface() const;

    bool operator<(const NetlinkAddress& rval) const;

private:
    Address _address;
    std::shared_ptr<Interface> _iface;
};

NDPPD_NS_END

#endif // NDPPD_NL_ADDRESS_H
