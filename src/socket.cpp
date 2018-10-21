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

#include <system_error>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/poll.h>

#include <unistd.h>
#include <cassert>
#include <vector>
#include <set>
#include <poll.h>
#include <algorithm>
#include <iostream>
#include <net/if.h>
#include <cstring>
#include "socket.h"
#include "ndppd.h"

using namespace ndppd;

namespace {
    bool pollfds_dirty;
    std::vector<pollfd> pollfds;
    std::set<Socket *> sockets;
}

std::unique_ptr<Socket> Socket::create(int domain, int type, int protocol) {
    return std::unique_ptr<Socket>(new Socket(domain, type, protocol));
}

void Socket::poll() {
    int len;

    if (pollfds_dirty) {
        pollfds.resize(sockets.size());

        std::transform(sockets.cbegin(), sockets.cend(), pollfds.begin(),
                       [](const Socket *socket) {
                           return (pollfd) {socket->_fd, POLLIN};
                       });

        pollfds_dirty = false;
    }

    if ((len = ::poll(&pollfds[0], pollfds.size(), 50)) < 0)
        throw std::system_error(errno, std::generic_category());

    for (auto it : pollfds) {
        if (it.revents & POLLIN) {
            auto s_it = std::find_if(sockets.cbegin(), sockets.cend(),
                                     [it](Socket *socket) { return socket->_fd == it.fd; });
            if (s_it != sockets.cend() && (*s_it)->_handler)
                (*s_it)->_handler(**s_it);
        }
    }
}

Socket::Socket(int domain, int type, int protocol) : _handler(nullptr) {
    // Create the socket.
    if ((_fd = ::socket(domain, type, protocol)) < 0)
        throw std::system_error(errno, std::generic_category());
    sockets.insert(this);
    pollfds_dirty = true;
}

Socket::~Socket() {
    ::close(_fd);
    sockets.erase(this);
    pollfds_dirty = true;
}

bool Socket::if_allmulti(const std::string &name, bool state) const {
    ifreq ifr{};

    Logger::debug() << "Socket::if_allmulti() state="<< state << ", _name=\"" << name << "\"";

    ::strncpy(ifr.ifr_name, name.c_str(), IFNAMSIZ);

    if (ioctl(_fd, SIOCGIFFLAGS, &ifr) < 0)
        throw std::system_error(errno, std::generic_category(), "Socket::if_allmulti()");

    bool old_state = (ifr.ifr_flags & IFF_ALLMULTI) != 0;

    if (state == old_state)
        return old_state;

    ifr.ifr_flags = state ? (ifr.ifr_flags | IFF_ALLMULTI) : (ifr.ifr_flags & ~IFF_ALLMULTI);

    if (ioctl(_fd, SIOCSIFFLAGS, &ifr) < 0)
        throw std::system_error(errno, std::generic_category(), "Socket::if_allmulti()");

    return old_state;
}

bool Socket::if_promisc(const std::string &name, bool state) const {
    ifreq ifr{};

    Logger::debug() << "Socket::if_promisc() state="<< state << ", _name=\"" << name << "\"";

    ::strncpy(ifr.ifr_name, name.c_str(), IFNAMSIZ);

    if (ioctl(_fd, SIOCGIFFLAGS, &ifr) < 0)
        throw std::system_error(errno, std::generic_category(), "Socket::if_promisc()");

    bool old_state = (ifr.ifr_flags & IFF_PROMISC) != 0;

    if (state == old_state)
        return old_state;

    ifr.ifr_flags = state ? (ifr.ifr_flags | IFF_PROMISC) : (ifr.ifr_flags & ~IFF_PROMISC);

    if (ioctl(_fd, SIOCSIFFLAGS, &ifr) < 0)
        throw std::system_error(errno, std::generic_category(), "Socket::if_promisc()");

    return old_state;
}
