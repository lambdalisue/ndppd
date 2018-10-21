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

#include <algorithm>
#include <sstream>

#include "ndppd.h"
#include "proxy.h"
#include "interface.h"
#include "session.h"

namespace ndppd
{
    std::list<std::weak_ptr<Session> > Session::_sessions;

    void Session::update_all(int elapsed_time)
    {
        for (auto it = _sessions.begin(); it != _sessions.end(); )
        {
            auto session = it->lock();

            if (!session)
            {
                _sessions.erase(it++);
                continue;
            }

            it++;

            if ((session->_ttl -= elapsed_time) >= 0)
            {
                continue;
            }

            auto pr = session->_pr.lock();

            switch (session->_status)
            {

                case Session::WAITING:
                    if (session->_fails < session->_retries)
                    {
                        Logger::debug() << "session will keep trying [taddr=" << session->_taddr << "]";

                        session->_ttl = session->_pr.lock()->timeout();
                        session->_fails++;

                        // Send another solicit
                        session->send_solicit();
                    }
                    else
                    {

                        Logger::debug() << "session is now invalid [taddr=" << session->_taddr << "]";

                        session->_status = Session::INVALID;
                        session->_ttl = pr->deadtime();
                    }
                    break;

                case Session::RENEWING:
                    Logger::debug() << "session is became invalid [taddr=" << session->_taddr << "]";

                    if (session->_fails < session->_retries)
                    {
                        session->_ttl = pr->timeout();
                        session->_fails++;

                        // Send another solicit
                        session->send_solicit();
                    }
                    else
                    {
                        pr->remove_session(session);
                    }
                    break;

                case Session::VALID:
                    if (session->touched() || session->keepalive())
                    {
                        Logger::debug() << "session is renewing [taddr=" << session->_taddr << "]";
                        session->_status = Session::RENEWING;
                        session->_ttl = pr->timeout();
                        session->_fails = 0;
                        session->_touched = false;

                        // Send another solicit to make sure the route is still valid
                        session->send_solicit();
                    }
                    else
                    {
                        pr->remove_session(session);
                    }
                    break;

                default:
                    pr->remove_session(session);
            }
        }
    }

    Session::~Session()
    {
        Logger::debug() << "session::~session() this=" << Logger::format("%x", this);

        if (_wired)
        {
            for (auto &_iface : _ifaces)
            {
                //handle_auto_unwire(_iface->name());
            }
        }
    }

    std::shared_ptr<Session>
    Session::create(const std::shared_ptr<Proxy> &pr, const Address &taddr, bool auto_wire, bool keepalive, int retries)
    {
        std::shared_ptr<Session> session(new Session());

        session->_ptr = session;
        session->_pr = pr;
        session->_taddr = taddr;
        session->_autowire = auto_wire;
        session->_keepalive = keepalive;
        session->_retries = retries;
        session->_wired = false;
        session->_ttl = pr->ttl();
        session->_touched = false;

        _sessions.push_back(session);

        Logger::debug()
                << "session::create() pr=" << Logger::format("%x", (Proxy *) pr.get()) << ", proxy="
                << ((pr->ifa()) ? pr->ifa()->name() : "null")
                << ", taddr=" << taddr << " =" << Logger::format("%x", (Session *) session.get());

        return session;
    }

    void Session::add_iface(const std::shared_ptr<Interface> &ifa)
    {
        if (std::find(_ifaces.begin(), _ifaces.end(), ifa) != _ifaces.end())
            return;

        _ifaces.push_back(ifa);
    }

    void Session::add_pending(const Address &addr)
    {
        _pending.insert(Address(addr));
    }

    void Session::send_solicit()
    {
        Logger::debug() << "session::send_solicit() (_ifaces.size() = " << _ifaces.size() << ")";

        for (auto &iface : _ifaces)
        {
            Logger::debug() << " - " << iface->name();
            iface->write_solicit(_taddr);
        }
    }

    void Session::touch()
    {
        if (!_touched)
        {
            _touched = true;

            if (status() == Session::WAITING || status() == Session::INVALID)
            {
                _ttl = _pr.lock()->timeout();
                Logger::debug() << "session is now probing [taddr=" << _taddr << "]";
                send_solicit();
            }
        }
    }

    void Session::send_advert(const Address &daddr)
    {
        auto pr = _pr.lock();
        pr->ifa()->write_advert(daddr, _taddr, pr->router());
    }

    void Session::handle_advert(const Address &saddr, const std::string &ifname, bool use_via)
    {
        if (_autowire && _status == WAITING)
        {
            // handle_auto_wire(saddr, ifname, use_via);
        }

        handle_advert();
    }


    void Session::handle_advert()
    {
        auto pr = _pr.lock();

        Logger::debug() << "session::handle_advert() taddr=" << _taddr << ", ttl=" << pr->ttl();

        if (_status != VALID)
        {
            _status = VALID;
            Logger::debug() << "session is active [taddr=" << _taddr << "]";
        }

        _ttl = pr->ttl();
        _fails = 0;

        if (!_pending.empty())
        {
            for (auto &address : _pending)
            {
                Logger::debug() << " - forward to " << address;
                send_advert(address);
            }
            _pending.clear();
        }
    }

    const Address &Session::taddr() const
    {
        return _taddr;
    }

    bool Session::autowire() const
    {
        return _autowire;
    }

    bool Session::keepalive() const
    {
        return _keepalive;
    }

    int Session::retries() const
    {
        return _retries;
    }

    int Session::fails() const
    {
        return _fails;
    }

    bool Session::wired() const
    {
        return _wired;
    }

    bool Session::touched() const
    {
        return _touched;
    }

    int Session::status() const
    {
        return _status;
    }

    void Session::status(int val)
    {
        _status = val;
    }
}
