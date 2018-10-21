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

#include <system_error>
#include <cassert>
#include <memory>
#include <sys/socket.h>

namespace ndppd
{
    class Socket
    {
        using SocketHandler = void (*)(Socket &socket);

    private:
        int _fd;
        void *_userdata;
        SocketHandler _handler;

        Socket(int domain, int type, int protocol);

    public:
        static std::unique_ptr<Socket> create(int domain, int type, int protocol);

        static void poll();

        ~Socket();

        bool if_allmulti(const std::string &name, bool state = true) const;

        /*!
         * Enable or disable PROMISC for the specified interface.
         */
        bool if_promisc(const std::string &name, bool state = true) const;

        /*!
         *
         */
        template<typename T>
        void bind(const T &sa) const
        {
            if ((::bind(_fd, (const sockaddr *) &sa, sizeof(T))) < 0)
                throw std::system_error(errno, std::generic_category());
        }

        template<typename T>
        ssize_t recvmsg(T &sa, void *msg, size_t size, bool wait = false) const
        {
            assert(msg != nullptr);
            iovec iov = { msg, size };
            msghdr mhdr = { &sa, sizeof(sa), &iov, 1 };
            return ::recvmsg(_fd, &mhdr, wait ? 0 : MSG_DONTWAIT);
        }

        template<typename T>
        ssize_t sendmsg(const T &sa, const void *msg, size_t size, bool wait = true) const
        {
            assert(msg != nullptr);
            iovec iov = { (void *) msg, size };
            msghdr mhdr = { (void *) &sa, sizeof(sa), &iov, 1, nullptr, 0, 0 };
            return ::sendmsg(_fd, &mhdr, wait ? 0 : MSG_DONTWAIT);
        }
    };
}
