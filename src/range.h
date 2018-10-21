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

#include <list>

namespace ndppd {

    template <typename _Tp>
    class Range {
    private:
        _Tp _begin;
        _Tp _end;

    public:
        Range(_Tp begin, _Tp end) {
            _begin = begin;
            _end = end;
        }

        _Tp begin() const {
            return _begin;
        }

        _Tp end() const {
            return _end;
        }
    };
}
