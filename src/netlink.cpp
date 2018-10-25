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
#include <set>

#include "ndppd.h"
#include "netlink.h"
#include "socket.h"
#include "ndppd.h"

NDPPD_NS_BEGIN

namespace {
static std::unique_ptr<Socket> sockets;
static std::set<Address> local_addresses;
}


static void handler(Socket& socket)
{

}

static void handle_address(nlmsghdr* nlh)
{
    static Address localhost("::1");

    auto data = (ifaddrmsg*) NLMSG_DATA(nlh);
    int len = IFA_PAYLOAD(nlh);

    for (auto rta = IFA_RTA(data); RTA_OK(rta, len); rta = RTA_NEXT(rta, len)) {
        if (rta->rta_type == IFA_ADDRESS) {
            auto address = Address(*(in6_addr*) RTA_DATA(rta));

            if (address == localhost)
                continue;

            local_addresses.insert(address);
            Logger::info() << "Registered local address " << address.to_string();
        }
    }
}

void Netlink::initialize()
{
    sockets = std::make_unique<Socket>(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    sockets->bind((sockaddr_nl) { AF_NETLINK, 0, 0, RTMGRP_IPV6_ROUTE | RTMGRP_IPV6_IFADDR });
}

void Netlink::finalize()
{
    sockets.reset();
}

const Range<std::set<Address>::const_iterator> Netlink::local_addresses()
{
    return { ndppd::local_addresses.cbegin(), ndppd::local_addresses.cend() };
}

void Netlink::load_local_ips()
{
    struct {
        nlmsghdr hdr;
        rtgenmsg msg;
    } payload {{ NLMSG_LENGTH(sizeof(rtgenmsg)), RTM_GETADDR, NLM_F_REQUEST | NLM_F_DUMP, 1 },
               { AF_INET6 }};

    if (sockets->sendmsg((sockaddr_nl) { AF_NETLINK }, &payload, sizeof(payload)) < 0)
        throw std::system_error(errno, std::generic_category());

    char buf[1024];

    sockaddr_nl sa {};
    auto len = sockets->recvmsg(sa, buf, sizeof(buf), true);
    if (len < 0)
        throw std::system_error(errno, std::generic_category());

    for (auto nlh = (nlmsghdr*) (void*) buf; NLMSG_OK(nlh, len); nlh = NLMSG_NEXT(nlh, len)) {
        switch (nlh->nlmsg_type) {
        case NLMSG_ERROR:
            return;

        case NLMSG_DONE:
            return;

        case RTM_NEWADDR:
            handle_address(nlh);
            continue;

        default:
            Logger::debug() << "Unexpected NL message";
        }
    }

    for (auto a : local_addresses())
        std::cout << a.to_string() << std::endl;

}

bool Netlink::is_local(const Address& address)
{
    return ndppd::local_addresses.find(address) != ndppd::local_addresses.end();
}

void Netlink::test()
{
    struct {
        nlmsghdr hdr;
        rtgenmsg gen;
    } payload {{ NLMSG_LENGTH(sizeof(rtgenmsg)), RTM_GETROUTE, NLM_F_REQUEST | NLM_F_DUMP, 1 },
               { AF_INET6 }};

    if (sockets->sendmsg((sockaddr_nl) { AF_NETLINK }, &payload, sizeof(payload)) < 0)
        throw std::system_error(errno, std::generic_category());

    char buf[1024];

    sockaddr_nl sa {};
    auto len = sockets->recvmsg(sa, buf, sizeof(buf), true);
    if (len < 0)
        throw std::system_error(errno, std::generic_category());

    for (auto nh = (nlmsghdr*) (void*) buf; NLMSG_OK(nh, len); NLMSG_NEXT(nh, len)) {
        auto entry = (rtmsg*) NLMSG_DATA(nh);

        // entry->rtm_dst_len

        if (nh->nlmsg_type == NLMSG_ERROR)
            break;

        auto rta = RTM_RTA(entry);
        auto rta_len = RTM_PAYLOAD(nh);

        for (; RTA_OK(rta, rta_len); rta = RTA_NEXT(rta, rta_len)) {
            if (rta->rta_type == RTA_DST) {

            }
        }
    }
}

NDPPD_NS_END
