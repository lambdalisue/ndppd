#include <utility>

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
#include <list>
#include <poll.h>
#include <algorithm>
#include <iostream>
#include <net/if.h>
#include <cstring>

#include "ndppd.h"
#include "socket.h"
#include "logger.h"

NDPPD_NS_BEGIN

namespace {
bool pollfds_dirty;
std::vector<pollfd> pollfds;
std::list<std::reference_wrapper<Socket>> sockets;
}

bool Socket::poll_all()
{
    if (pollfds_dirty) {
        pollfds.resize(sockets.size());

        std::transform(sockets.cbegin(), sockets.cend(), pollfds.begin(),
                [](const Socket& socket) {
                    return (pollfd) { socket._fd, POLLIN };
                });

        pollfds_dirty = false;
    }

    if (::poll(&pollfds[0], pollfds.size(), 50) < 0)
        return false;

    for (auto pollfd : pollfds) {
        if (pollfd.revents & POLLIN) {
            auto it = std::find_if(sockets.cbegin(), sockets.cend(),
                    [&](Socket& socket) { return socket._fd == pollfd.fd; });

            if (it != sockets.cend()) {
                Socket& socket = *it;
                if (socket._handler)
                    socket._handler(socket);
            }
        }
    }

    return true;
}

Socket::Socket(int domain, int type, int protocol)
        : _handler(nullptr)
{
    if ((_fd = ::socket(domain, type, protocol)) < 0)
        throw std::system_error(errno, std::generic_category(), "Socket::Socket()");
    sockets.push_back(std::ref(*this));
    pollfds_dirty = true;
}

Socket::~Socket()
{
    ::close(_fd);
    sockets.remove_if([&](Socket& socket) { return &socket == this; });
    pollfds_dirty = true;
}

void Socket::handler(SocketHandler handler, void* userdata)
{
    _handler = std::move(handler);
    _userdata = userdata;
}

void Socket::ioctl(unsigned long request, void* data) const
{
    if (::ioctl(_fd, request, data) == -1)
        throw std::system_error(errno, std::generic_category(), "Socket::ioctl()");
}

NDPPD_NS_END
