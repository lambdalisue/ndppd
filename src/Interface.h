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

#ifndef NDPPD_INTERFACE_H
#define NDPPD_INTERFACE_H

#include <string>
#include <list>
#include <vector>
#include <map>
#include <sys/poll.h>
#include <net/ethernet.h>
#include <memory>

#include "ndppd.h"
#include "range.h"
#include "address.h"
#include "socket.h"

NDPPD_NS_BEGIN

class Session;

class Proxy;

class Interface {
public:
    // Destructor.
    ~Interface();

    static std::shared_ptr<Interface> open_ifd(const std::string& name);

    static std::shared_ptr<Interface> open_pfd(const std::string& name, bool promisc);

    static int poll_all();

    // Writes a NB_NEIGHBOR_SOLICIT message to the _ifd socket.
    ssize_t write_solicit(const Address& taddr);

    // Writes a NB_NEIGHBOR_ADVERT message to the _ifd socket;
    ssize_t write_advert(const Address& daddr, const Address& taddr, bool router);

    // Reads a NB_NEIGHBOR_SOLICIT message from the _pfd socket.
    ssize_t read_solicit(Address& saddr, Address& daddr, Address& taddr);

    // Reads a NB_NEIGHBOR_ADVERT message from the _ifd socket;
    ssize_t read_advert(Address& saddr, Address& taddr);

    bool handle_local(const Address& saddr, const Address& taddr);

    bool is_local(const Address& addr);

    void handle_reverse_advert(const Address& saddr, const std::string& ifname);

    // Returns the name of the interface.
    const std::string& name() const;

    void add_serves(const std::shared_ptr<Proxy>& proxy);

    void add_parent(const std::shared_ptr<Proxy>& parent);

    static std::map<std::string, std::weak_ptr<Interface> > _map;

    const Range<std::list<std::weak_ptr<Proxy>>::const_iterator> parents() const;

    const Range<std::list<std::weak_ptr<Proxy>>::const_iterator> serves() const;

private:
    void icmp6_handler();

    void packet_handler();

    // Weak pointer so this object can reference itself.
    std::weak_ptr<Interface> _self;

    // The "generic" ICMPv6 socket for reading/writing NB_NEIGHBOR_ADVERT messages as well
    // as writing NB_NEIGHBOR_SOLICIT messages.
    std::unique_ptr<Socket> _icmp6_socket;

    // This is the PF_PACKET socket we use in order to read NB_NEIGHBOR_SOLICIT messages.
    std::unique_ptr<Socket> _packet_socket;

    // Previous state of ALLMULTI for the interface.
    int _prev_allmulti;

    // Previous state of PROMISC for the interface
    int _prev_promisc;

    // Name of this interface.
    std::string _name;

    std::list<std::weak_ptr<Proxy> > _serves;

    std::list<std::weak_ptr<Proxy> > _parents;

    // The link-layer address of this interface.
    struct ether_addr hwaddr;

    // Turns on/off ALLMULTI for this interface - returns the previous state
    // or -1 if there was an error.
    int allmulti(bool state = true);

    // Turns on/off PROMISC for this interface - returns the previous state
    // or -1 if there was an error
    int promisc(bool state = true);

    // Constructor.
    Interface();


};

NDPPD_NS_END

#endif
