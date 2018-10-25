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

#ifndef NDPPD_SOCKET_H
#define NDPPD_SOCKET_H

#include <system_error>
#include <cassert>
#include <memory>
#include <sys/socket.h>
#include <functional>

#include "ndppd.h"
#include "logger.h"

NDPPD_NS_BEGIN

class Socket {
public:
    std::function<void(Socket&)> handler;

    static bool poll_all();

    Socket(int domain, int type, int protocol);

    ~Socket();

    void ioctl(unsigned long request, void* data) const;

    template<typename T>
    void bind(const T& sa) const
    {
        if (::bind(_fd, (const sockaddr*) &sa, sizeof(T)) != 0)
            throw std::system_error(errno, std::generic_category(), "Socket::bind()");
    }

    template<typename T>
    ssize_t recvmsg(T& sa, void* msg, size_t size, bool wait = false) const
    {
        assert(msg != nullptr);
        iovec iov = { msg, size };
        msghdr mhdr = { &sa, sizeof(sa), &iov, 1 };
        auto len = ::recvmsg(_fd, &mhdr, wait ? 0 : MSG_DONTWAIT);
        if (len < 0 && errno != EAGAIN)
            throw std::system_error(errno, std::generic_category(), "Socket::recvmsg()");
        return len;
    }

    template<typename T>
    ssize_t sendmsg(const T& sa, const void* msg, size_t size, bool wait = true) const
    {
        assert(msg != nullptr);
        iovec iov = { (void*) msg, size };
        msghdr mhdr = { (void*) &sa, sizeof(sa), &iov, 1, nullptr, 0, 0 };
        auto len = ::sendmsg(_fd, &mhdr, wait ? 0 : MSG_DONTWAIT);
        if (len < 0 && errno != EAGAIN)
            throw std::system_error(errno, std::generic_category(), "Socket::recvmsg()");
        return len;
    }

    template<typename T>
    void setsockopt(int level, int optname, const T& val) const
    {
        if (::setsockopt(_fd, level, optname, &val, sizeof(val)) != 0)
            throw std::system_error(errno, std::generic_category(), "Socket::setsockopt()");
    }

private:
    int _fd;
};

NDPPD_NS_END

#endif
