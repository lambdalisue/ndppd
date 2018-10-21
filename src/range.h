//
// Created by Daniel on 10/21/2018.
//

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
