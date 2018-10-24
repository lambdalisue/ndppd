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

#ifndef NDPPD_RANGE_H
#define NDPPD_RANGE_H

#include <algorithm>
#include "ndppd.h"

NDPPD_NS_BEGIN

template<typename _IIter>
class Range {
private:
    _IIter _begin;
    _IIter _end;

public:
    Range(_IIter begin, _IIter end)
    {
        _begin = begin;
        _end = end;
    }

    _IIter begin() const
    {
        return _begin;
    }

    _IIter end() const
    {
        return _end;
    }

    template<typename _Predicate>
    _IIter find_if(_Predicate predicate) const
    {
        return std::find_if(_begin, _end, predicate);
    }
};

NDPPD_NS_END

#endif
