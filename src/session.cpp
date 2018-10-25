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

NDPPD_NS_BEGIN

namespace {
std::list<std::reference_wrapper<Session>> sessions;
}

void Session::update_all(int elapsed_time)
{
    for (auto it = sessions.begin(); it != sessions.end();) {
        Session& session = *it++;

        if ((session._ttl -= elapsed_time) >= 0)
            continue;

        switch (session._status) {

        case Session::WAITING:
        case Session::RENEWING:
            if (session._fails < session._retries) {
                Logger::debug() << "Session is still WAITING (taddr=" << session._taddr << ")";
                session._ttl = session._proxy.timeout;
                session._fails++;
                session.send_solicit();
            }
            else {
                Logger::debug() << "Session became INVALID (taddr=" << session._taddr << ")";
                session._status = Session::INVALID;
                session._ttl = session._proxy.timeout;
            }
            break;

        case Session::VALID:
            if (session.keepalive()) {
                Logger::debug() << "Session is RENEWING (taddr=" << session._taddr << ")";
                session._status = Session::RENEWING;
                session._ttl = session._proxy.timeout;
                session._fails = 0;
                session._touched = false;
                session.send_solicit();
            }
            break;

        default:
            session._proxy.remove_session(session);
            break;
        }
    }
}

Session::Session(Proxy& proxy, const Address& taddr, bool keepalive, int retries)
        : _proxy(proxy), _taddr(taddr), _keepalive(keepalive), _retries(retries),
          _ttl(proxy.ttl), _touched(false)
{
    Logger::debug() << "Session::create() pr=" << Logger::format("%x", &proxy)
                    << ", proxy=" << (proxy.iface() ? proxy.iface()->name() : "null")
                    << ", taddr=" << taddr << " =" << Logger::format("%x", this);

    sessions.push_back(std::ref(*this));
}

Session::~Session()
{
    Logger::debug() << "session::~session() this=" << Logger::format("%x", this);

    sessions.remove(*this);

    for (const auto& iface : _ifaces)
        iface->sessions.remove(std::ref(*this));
}

void Session::add_iface(const std::shared_ptr<Interface>& iface)
{
    if (std::find(_ifaces.begin(), _ifaces.end(), iface) != _ifaces.end())
        return;

    _ifaces.push_back(iface);
    iface->sessions.push_back(std::ref(*this));
}

void Session::add_pending(const Address& addr)
{
    _pending.insert(Address(addr));
}

void Session::send_solicit()
{
    Logger::debug() << "Session::send_solicit() (_ifaces.size() = " << _ifaces.size() << ")";

    for (const auto& iface : _ifaces) {
        Logger::debug() << " - " << iface->name();
        iface->write_solicit(_taddr);
    }
}

void Session::send_advert(const Address& daddr)
{
    _proxy.iface()->write_advert(daddr, _taddr, _proxy.router);
}

void Session::handle_advert()
{
    Logger::debug() << "Session::handle_advert() taddr=" << _taddr << ", ttl=" << _proxy.ttl;

    if (_status != VALID) {
        _status = VALID;
        Logger::debug() << "Session became VALID (taddr=" << _taddr << ")";
    }

    _ttl = _proxy.ttl;
    _fails = 0;

    for (const auto& address : _pending) {
        Logger::debug() << " - Notify " << address;
        send_advert(address);
    }

    _pending.clear();
}

const Address& Session::taddr() const
{
    return _taddr;
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

NDPPD_NS_END
