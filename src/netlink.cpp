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

#include "netlink.h"
#include "ndppd.h"
#include "socket.h"
#include "ndppd.h"
#include "nl_address.h"

NDPPD_NS_BEGIN

namespace {
std::unique_ptr<Socket> sockets;
std::list<std::unique_ptr<NetlinkAddress>> g_local_addresses;
std::list<std::unique_ptr<NetlinkRoute>> g_routes;

void handle_new_address(nlmsghdr* nlh)
{
    auto ifaddr = static_cast<ifaddrmsg*>(NLMSG_DATA(nlh));
    int len = IFA_PAYLOAD(nlh);

    Address address {};

    for (auto rta = IFA_RTA(ifaddr); RTA_OK(rta, len); rta = RTA_NEXT(rta, len)) {
        if (rta->rta_type == IFA_ADDRESS)
            address = Address(*static_cast<in6_addr*>(RTA_DATA(rta)));
    }

    static Address localhost("::1");

    if (!address || address == localhost)
        return;

    auto iface = Interface::get_or_create(ifaddr->ifa_index);
    Logger::info() << "Netlink: Registered new local address " << address << " on " << iface->name();
    g_local_addresses.push_back(std::make_unique<NetlinkAddress>(address, std::move(iface)));
}

void handle_new_route(const nlmsghdr* nlh)
{
    auto rt = static_cast<rtmsg*>(NLMSG_DATA(nlh));
    int len = RTM_PAYLOAD(nlh);

    Cidr cidr {};
    unsigned int oif {};

    for (auto rta = RTM_RTA(rt); RTA_OK(rta, len); rta = RTA_NEXT(rta, len)) {
        if (rta->rta_type == RTA_DST)
            cidr = Cidr(*static_cast<in6_addr*>(RTA_DATA(rta)), rt->rtm_dst_len);
        else if (rta->rta_type == RTA_OIF)
            oif = *static_cast<unsigned int*>(RTA_DATA(rta));
    }

    if (!cidr || oif == 0)
        return;

    auto iface = Interface::get_or_create(oif);
    Logger::info() << "Netlink: Registered new route " << cidr << " via " << iface->name();
    g_routes.push_back(std::make_unique<NetlinkRoute>(cidr, std::move(iface)));
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

const Range<std::list<std::unique_ptr<NetlinkAddress>>::const_iterator> Netlink::local_addresses()
{
    return { g_local_addresses.cbegin(), g_local_addresses.cend() };
}

void Netlink::load_local_ips()
{
    struct {
        nlmsghdr hdr;
        rtgenmsg msg;
    } payload {{ NLMSG_LENGTH(sizeof(rtgenmsg)), RTM_GETADDR, NLM_F_REQUEST | NLM_F_DUMP, 1 },
               { AF_INET6 }};

    sockets->sendmsg((sockaddr_nl) { AF_NETLINK }, &payload, sizeof(payload));

    char buf[4096];
    sockaddr_nl sa {};

    for (;;) {
        auto len = sockets->recvmsg(sa, buf, sizeof(buf));

        Logger::error() << len;

        for (auto nlh = (nlmsghdr*) (void*) buf; NLMSG_OK(nlh, len); nlh = NLMSG_NEXT(nlh, len)) {
            switch (nlh->nlmsg_type) {
            case NLMSG_ERROR:
                return;

            case NLMSG_DONE:
                return;

            case RTM_NEWADDR:
                handle_new_address(nlh);
                continue;

            default:
                Logger::error() << "Unexpected NL message";
            }
        }
    }
}

bool Netlink::is_local(const Address& address)
{
    return std::find_if(g_local_addresses.cbegin(), g_local_addresses.cend(),
            [&](const std::unique_ptr<NetlinkAddress>& nla) {
                return nla->address() == address;
            }) != g_local_addresses.end();
}

void Netlink::load_routes()
{
    g_routes.clear();

    struct {
        nlmsghdr hdr;
        rtmsg gen;
    } payload {{ NLMSG_LENGTH(sizeof(rtmsg)), RTM_GETROUTE, NLM_F_REQUEST | NLM_F_DUMP, 1 },
               { AF_INET6, 0, 0, 0, RT_TABLE_MAIN, RTPROT_UNSPEC }};

    sockets->sendmsg((sockaddr_nl) { AF_NETLINK }, &payload, sizeof(payload));

    char buf[4096];

    sockaddr_nl sa {};

    for (;;) {
        auto len = sockets->recvmsg(sa, buf, sizeof(buf), true);

        for (auto nh = (nlmsghdr*) (void*) buf; NLMSG_OK(nh, len); nh = NLMSG_NEXT(nh, len)) {
            switch (nh->nlmsg_type) {
            case NLMSG_ERROR:
                return;

            case NLMSG_DONE:
                return;

            case RTM_NEWROUTE:
                handle_new_route(nh);
                continue;

            default:
                Logger::debug() << "Unexpected NL message";
            }
        }
    }

}

NDPPD_NS_END
