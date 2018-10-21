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
#include <algorithm>
#include <sstream>

#include "ndppd.h"
#include "proxy.h"
#include "iface.h"
#include "session.h"

NDPPD_NS_BEGIN

std::list<std::weak_ptr<session> > session::_sessions;

void session::update_all(int elapsed_time)
{
    for (auto it = _sessions.begin();
            it != _sessions.end(); ) {

        auto se = it->lock();

        if (!se) {
            _sessions.erase(it++);
            continue;
        }

        it++;

        if ((se->_ttl -= elapsed_time) >= 0) {
            continue;
        }

        auto pr = se->_pr.lock();

        switch (se->_status) {
            
        case session::WAITING:
            if (se->_fails < se->_retries) {
                Logger::debug() << "session will keep trying [taddr=" << se->_taddr << "]";
                
                se->_ttl     = se->_pr.lock()->timeout();
                se->_fails++;
                
                // Send another solicit
                se->send_solicit();
            } else {
                
                Logger::debug() << "session is now invalid [taddr=" << se->_taddr << "]";
                
                se->_status = session::INVALID;
                se->_ttl    = pr->deadtime();
            }
            break;
            
        case session::RENEWING:
            Logger::debug() << "session is became invalid [taddr=" << se->_taddr << "]";
            
            if (se->_fails < se->_retries) {
                se->_ttl     = pr->timeout();
                se->_fails++;
                
                // Send another solicit
                se->send_solicit();
            } else {            
                pr->remove_session(se);
            }
            break;
            
        case session::VALID:            
            if (se->touched() == true ||
                se->keepalive() == true)
            {
                Logger::debug() << "session is renewing [taddr=" << se->_taddr << "]";
                se->_status  = session::RENEWING;
                se->_ttl     = pr->timeout();
                se->_fails   = 0;
                se->_touched = false;

                // Send another solicit to make sure the route is still valid
                se->send_solicit();
            } else {
                pr->remove_session(se);
            }            
            break;

        default:
            pr->remove_session(se);
        }
    }
}

session::~session()
{
    Logger::debug() << "session::~session() this=" << Logger::format("%x", this);
    
    if (_wired) {
        for (auto &_iface : _ifaces) {
            //handle_auto_unwire(_iface->name());
        }
    }
}

std::shared_ptr<session> session::create(const std::shared_ptr<proxy>& pr, const Address& taddr, bool auto_wire, bool keepalive, int retries)
{
    std::shared_ptr<session> se(new session());

    se->_ptr       = se;
    se->_pr        = pr;
    se->_taddr     = taddr;
    se->_autowire  = auto_wire;
    se->_keepalive = keepalive;
    se->_retries   = retries;
    se->_wired     = false;
    se->_ttl       = pr->ttl();
    se->_touched   = false;

    _sessions.push_back(se);

    Logger::debug()
        << "session::create() pr=" << Logger::format("%x", (proxy* )pr.get()) << ", proxy=" << ((pr->ifa()) ? pr->ifa()->name() : "null")
        << ", taddr=" << taddr << " =" << Logger::format("%x", (session* )se.get());

    return se;
}

void session::add_iface(const std::shared_ptr<iface>& ifa)
{
    if (std::find(_ifaces.begin(), _ifaces.end(), ifa) != _ifaces.end())
        return;

    _ifaces.push_back(ifa);
}

void session::add_pending(const Address& addr)
{
    _pending.insert(Address(addr));
}

void session::send_solicit()
{
    Logger::debug() << "session::send_solicit() (_ifaces.size() = " << _ifaces.size() << ")";

    for (auto &iface : _ifaces) {
        Logger::debug() << " - " << iface->name();
        iface->write_solicit(_taddr);
    }
}

void session::touch()
{
    if (!_touched)
    {
        _touched = true;
        
        if (status() == session::WAITING || status() == session::INVALID) {
            _ttl = _pr.lock()->timeout();
            
            Logger::debug() << "session is now probing [taddr=" << _taddr << "]";
            
            send_solicit();
        }
    }
}

void session::send_advert(const Address& daddr)
{
    auto pr = _pr.lock();
    pr->ifa()->write_advert(daddr, _taddr, pr->router());
}

void session::handle_advert(const Address& saddr, const std::string& ifname, bool use_via)
{
    if (_autowire && _status == WAITING) {
        // handle_auto_wire(saddr, ifname, use_via);
    }
    
    handle_advert();
}


void session::handle_advert()
{
    auto pr = _pr.lock();

    Logger::debug()
        << "session::handle_advert() taddr=" << _taddr << ", ttl=" << pr->ttl();
    
    if (_status != VALID) {
        _status = VALID;
        
        Logger::debug() << "session is active [taddr=" << _taddr << "]";
    }
    
    _ttl    = pr->ttl();
    _fails  = 0;
    
    if (!_pending.empty()) {
        for (auto &address : _pending) {
            Logger::debug() << " - forward to " << address;
            send_advert(address);
        }
        _pending.clear();
    }
}

const Address& session::taddr() const
{
    return _taddr;
}

bool session::autowire() const
{
    return _autowire;
}

bool session::keepalive() const
{
    return _keepalive;
}

int session::retries() const
{
    return _retries;
}

int session::fails() const
{
    return _fails;
}

bool session::wired() const
{
    return _wired;
}

bool session::touched() const
{
    return _touched;
}

int session::status() const
{
    return _status;
}

void session::status(int val)
{
    _status = val;
}

NDPPD_NS_END
