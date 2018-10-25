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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ndppd.h"
#include "proxy.h"
#include "interface.h"
#include "rule.h"
#include "session.h"

NDPPD_NS_BEGIN

namespace {
std::list<std::unique_ptr<Proxy>> l_proxies;
}

Proxy::Proxy(const std::shared_ptr<Interface>& iface, bool promiscuous)
        : _iface(iface), router(true), ttl(30000), deadtime(3000), timeout(500), autowire(false),
          keepalive(true), _promiscuous(promiscuous), retries(3)
{
    Logger::debug() << "Proxy::Proxy() if=" << iface->name();
}

Proxy& Proxy::create(const std::string& ifname, bool promiscuous)
{
    auto iface = Interface::get_or_create(ifname);
    iface->ensure_packet_socket(promiscuous);
    auto proxy = std::make_unique<Proxy>(iface, promiscuous);
    auto& ref = *proxy;
    l_proxies.push_back(std::move(proxy));
    return ref;
}

Session* Proxy::find_or_create_session(const Address& taddr)
{
    // Let's check this proxy's list of sessions to see if we can find one with the same target address.
    for (const auto& session : _sessions) {
        if (session->taddr() == taddr)
            return session.get();
    }

    std::unique_ptr<Session> session;

    // Since we couldn't find a session that matched, we'll try to find a matching rule instead,
    // and then set up a new session.

    for (const auto& rule : _rules) {
        Logger::debug() << "checking " << rule->cidr() << " against " << taddr;

        if (!(rule->cidr() % taddr))
            continue;

        if (!session)
            session = std::make_unique<Session>(*this, taddr, autowire, keepalive, retries);

        if (rule->is_auto()) {
            // TODO: Implement route support again
            /*
            std::shared_ptr<route> rt = route::find(taddr);

            if (rt->ifname() == _ifa->name()) {
                Logger::debug() << "skipping route since it's using interface " << rt->ifname();
            } else {
                std::shared_ptr<iface> ifa = rt->ifa();

                if (ifa && (ifa != rule->daughter())) {
                    se->add_iface(ifa);
                }
            }*/
        }
        else if (!rule->iface())
            // No interface; assume "static" and respond.
            session->handle_advert();
        else
            session->add_iface(rule->iface());
    }

    if (session)
        _sessions.push_back(std::move(session));

    return session.get();
}

void Proxy::handle_advert(const Address& saddr, const Address& taddr, const std::string& ifname, bool use_via)
{
    // If a session exists then process the advert in the context of the session
    for (auto& session : _sessions) {
        if ((session->taddr() == taddr))
            session->handle_advert(saddr, ifname, use_via);
    }
}

void Proxy::handle_stateless_advert(const Address& saddr, const Address& taddr,
        const std::string& ifname, bool use_via)
{
    Logger::debug() << "Proxy::handle_stateless_advert() proxy=" << (iface() ? iface()->name() : "null") << ", taddr="
                    << taddr.to_string() << ", ifname=" << ifname;

    auto session = find_or_create_session(taddr);

    if (!session)
        return;

    if (autowire && session->status() == Session::WAITING) {
        // TODO
        // se->handle_auto_wire(saddr, ifname, use_via);
    }
}

void Proxy::handle_solicit(const Address& saddr, const Address& taddr, const std::string& ifname)
{
    Logger::debug() << "Proxy::handle_solicit()";

    // Otherwise find or create a session to scan for this address
    auto session = find_or_create_session(taddr);

    if (!session)
        return;

    // Touching the session will cause an NDP advert to be transmitted to all
    // the daughters
    session->touch();

    // If our session is confirmed then we can respoond with an advert otherwise
    // subscribe so that if it does become active we can notify everyone
    if (saddr != taddr) {
        switch (session->status()) {
        case Session::WAITING:
        case Session::INVALID:
            session->add_pending(saddr);
            break;

        case Session::VALID:
        case Session::RENEWING:
            session->send_advert(saddr);
            break;
        }
    }
}

Rule& Proxy::add_rule(const Cidr& cidr, const std::shared_ptr<Interface>& iface, bool autovia)
{
    auto rule = std::make_unique<Rule>(*this, cidr, iface);
    rule->autovia = autovia;
    auto& ref = *rule;
    _rules.push_back(std::move(rule));
    return ref;
}

Rule& Proxy::add_rule(const Cidr& cidr, bool _auto)
{
    auto rule = std::make_unique<Rule>(*this, cidr, _auto);
    auto& ref = *rule;
    _rules.push_back(std::move(rule));
    return ref;
}

void Proxy::remove_session(Session& session)
{
    _sessions.remove_if([session](const std::unique_ptr<Session>& ref) { return &session == ref.get(); });
}

const std::shared_ptr<Interface>& Proxy::iface() const
{
    return _iface;
}

bool Proxy::promiscuous() const
{
    return _promiscuous;
}

const Range<std::list<std::unique_ptr<Rule> >::const_iterator> Proxy::rules() const
{
    return { _rules.cbegin(), _rules.cend() };
}

const Range<std::list<std::unique_ptr<Proxy>>::const_iterator> Proxy::proxies()
{
    return { l_proxies.cbegin(), l_proxies.cend() };
}

NDPPD_NS_END
