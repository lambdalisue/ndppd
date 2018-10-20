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
#include <unistd.h>
#include <cassert>
#include <vector>
#include <set>
#include <poll.h>
#include <algorithm>
#include <iostream>
#include "socket.h"

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






