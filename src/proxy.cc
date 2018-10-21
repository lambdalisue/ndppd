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
#include "iface.h"
#include "rule.h"
#include "session.h"

namespace ndppd
{
    std::list<std::shared_ptr<proxy> > proxy::_list;

    proxy::proxy() :
            _router(true), _ttl(30000), _deadtime(3000), _timeout(500), _autowire(false), _keepalive(true),
            _promiscuous(false), _retries(3)
    {
    }

    std::shared_ptr<proxy> proxy::find_aunt(const std::string &ifname, const Address &taddr)
    {
        for (auto &weak_proxy : _list)
        {
            std::shared_ptr<proxy> proxy = weak_proxy;

            bool has_addr = false;
            for (const auto &rule : proxy->_rules)
            {
                if (rule->cidr() % taddr)
                { // TODO
                    has_addr = true;
                    break;
                }
            }

            if (!has_addr)
            {
                continue;
            }

            if (proxy->ifa() && proxy->ifa()->name() == ifname)
                return proxy;
        }

        return std::shared_ptr<proxy>();
    }

    std::shared_ptr<proxy> proxy::create(const std::shared_ptr<iface> &ifa, bool promiscuous)
    {
        std::shared_ptr<proxy> proxy(new class proxy());
        proxy->_ptr = proxy;
        proxy->_ifa = ifa;
        proxy->_promiscuous = promiscuous;

        _list.push_back(proxy);

        ifa->add_serves(proxy);

        Logger::debug() << "proxy::create() if=" << ifa->name();

        return proxy;
    }

    std::shared_ptr<proxy> proxy::open(const std::string &ifname, bool promiscuous)
    {
        std::shared_ptr<iface> ifa = iface::open_pfd(ifname, promiscuous);

        if (!ifa)
        {
            return std::shared_ptr<proxy>();
        }

        return create(ifa, promiscuous);
    }

    std::shared_ptr<session> proxy::find_or_create_session(const Address &taddr)
    {
        // Let's check this proxy's list of sessions to see if we can
        // find one with the same target address.

        for (auto &_session : _sessions)
        {
            if (_session->taddr() == taddr)
                return _session;
        }

        std::shared_ptr<session> se;

        // Since we couldn't find a session that matched, we'll try to find
        // a matching rule instead, and then set up a new session.

        for (auto &rule : _rules)
        {
            Logger::debug() << "checking " << rule->cidr() << " against " << taddr;

            if (rule->cidr() % taddr)
            {
                if (!se)
                {
                    se = session::create(_ptr.lock(), taddr, _autowire, _keepalive, _retries);
                }

                if (rule->is_auto())
                {
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
                else if (!rule->daughter())
                {
                    // This rule doesn't have an interface, and thus we'll consider
                    // it "static" and immediately send the response.
                    se->handle_advert();
                    return se;

                }
                else
                {

                    std::shared_ptr<iface> ifa = rule->daughter();
                    se->add_iface(ifa);

#ifdef WITH_ND_NETLINK
                    if (if_addr_find(ifa->name(), &taddr.const_addr())) {
                    logger::debug() << "Sending NA out " << ifa->name();
                    se->add_iface(_ifa);
                    se->handle_advert();
                }
#endif
                }
            }
        }

        if (se)
        {
            _sessions.push_back(se);
        }

        return se;
    }

    void proxy::handle_advert(const Address &saddr, const Address &taddr, const std::string &ifname, bool use_via)
    {
        // If a session exists then process the advert in the context of the session
        for (auto &sess : _sessions)
        {
            if ((sess->taddr() == taddr))
            {
                sess->handle_advert(saddr, ifname, use_via);
            }
        }
    }

    void proxy::handle_stateless_advert(const Address &saddr, const Address &taddr,
                                        const std::string &ifname, bool use_via)
    {
        Logger::debug() << "proxy::handle_stateless_advert() proxy=" << (ifa() ? ifa()->name() : "null") << ", taddr="
                        << taddr.to_string() << ", ifname=" << ifname;

        std::shared_ptr<session> se = find_or_create_session(taddr);
        if (!se) return;

        if (_autowire && se->status() == session::WAITING)
        {
            // TODO
            // se->handle_auto_wire(saddr, ifname, use_via);
        }
    }

    void proxy::handle_solicit(const Address &saddr, const Address &taddr, const std::string &ifname)
    {
        Logger::debug() << "proxy::handle_solicit()";

        // Otherwise find or create a session to scan for this address
        std::shared_ptr<session> se = find_or_create_session(taddr);
        if (!se) return;

        // Touching the session will cause an NDP advert to be transmitted to all
        // the daughters
        se->touch();

        // If our session is confirmed then we can respoond with an advert otherwise
        // subscribe so that if it does become active we can notify everyone
        if (saddr != taddr)
        {
            switch (se->status())
            {
                case session::WAITING:
                case session::INVALID:
                    se->add_pending(saddr);
                    break;

                case session::VALID:
                case session::RENEWING:
                    se->send_advert(saddr);
                    break;
            }
        }
    }

    std::shared_ptr<rule> proxy::add_rule(const Cidr &cidr, const std::shared_ptr<iface> &ifa, bool autovia)
    {
        std::shared_ptr<rule> ru(rule::create(_ptr.lock(), cidr, ifa));
        ru->autovia(autovia);
        _rules.push_back(ru);
        return ru;
    }

    std::shared_ptr<rule> proxy::add_rule(const Cidr &cidr, bool aut)
    {
        std::shared_ptr<rule> ru(rule::create(_ptr.lock(), cidr, aut));
        _rules.push_back(ru);
        return ru;
    }

    void proxy::remove_session(const std::shared_ptr<session> &se)
    {
        _sessions.remove(se);
    }

    const std::shared_ptr<iface> &proxy::ifa() const
    {
        return _ifa;
    }

    bool proxy::promiscuous() const
    {
        return _promiscuous;
    }

    bool proxy::router() const
    {
        return _router;
    }

    void proxy::router(bool val)
    {
        _router = val;
    }

    bool proxy::autowire() const
    {
        return _autowire;
    }

    void proxy::autowire(bool val)
    {
        _autowire = val;
    }

    int proxy::retries() const
    {
        return _retries;
    }

    void proxy::retries(int val)
    {
        _retries = val;
    }

    bool proxy::keepalive() const
    {
        return _keepalive;
    }

    void proxy::keepalive(bool val)
    {
        _keepalive = val;
    }

    int proxy::ttl() const
    {
        return _ttl;
    }

    void proxy::ttl(int val)
    {
        _ttl = (val >= 0) ? val : 30000;
    }

    int proxy::deadtime() const
    {
        return _deadtime;
    }

    void proxy::deadtime(int val)
    {
        _deadtime = (val >= 0) ? val : 30000;
    }

    int proxy::timeout() const
    {
        return _timeout;
    }

    void proxy::timeout(int val)
    {
        _timeout = (val >= 0) ? val : 500;
    }

    const Range<std::list<std::shared_ptr<rule> >::const_iterator> proxy::rules() const
    {
        return { _rules.cbegin(), _rules.cend() };
    }
}

