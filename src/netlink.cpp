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

#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <unistd.h>
#include <linux/rtnetlink.h>
#include <iostream>
#include "netlink.h"
#include "socket.h"
#include "ndppd.h"

using namespace ndppd;

static void handler(Socket& socket) {

}

std::unique_ptr<Netlink> Netlink::create() {
    return std::unique_ptr<Netlink>(new Netlink());
}

Netlink::Netlink() {
    _socket = std::move(Socket::create(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE));
    _socket->bind((sockaddr_nl) { AF_NETLINK, 0, 0, RTMGRP_IPV6_ROUTE });
}

void Netlink::test() const {
    struct {
        nlmsghdr hdr;
        rtgenmsg gen;
    } payload{
        { NLMSG_LENGTH(sizeof(rtgenmsg)), RTM_GETROUTE, NLM_F_REQUEST | NLM_F_DUMP, 1 },
        { AF_INET6 }
    };

    if (_socket->sendmsg((sockaddr_nl) {AF_NETLINK}, &payload, sizeof(payload)) < 0)
        throw std::system_error(errno, std::generic_category());

    char buf[1024];

    sockaddr_nl sa{};
    auto len = _socket->recvmsg(sa, buf, sizeof(buf), MSG_WAITALL);
    if (len < 0)
        throw std::system_error(errno, std::generic_category());

    for (auto nh = (nlmsghdr *)(void *)buf; NLMSG_OK(nh, len); NLMSG_NEXT(nh, len)) {
        auto entry = (rtmsg *)NLMSG_DATA(nh);

        // entry->rtm_dst_len

        if (nh->nlmsg_type == NLMSG_ERROR)
            break;

        auto rta = RTM_RTA(entry);
        auto rta_len = RTM_PAYLOAD(nh);

        for (; RTA_OK(rta, rta_len); rta = RTA_NEXT(rta, rta_len)) {


            if (rta->rta_type == RTA_DST) {
                std::cout << address(*(in6_addr *)RTA_DATA(rta)).to_string() << std::endl;
            }
        }
    }
}
