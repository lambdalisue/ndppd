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

#include <sstream>

namespace ndppd
{
    enum class LogLevel
    {
        Error,
        Warning,
        Notice,
        Info,
        Debug
    };

    class Logger
    {
    public:
        static std::string format(const char *fmt, ...);

#ifndef DISABLE_SYSLOG

        static bool syslog(bool enable);

        static bool syslog();

#endif

        static bool verbosity(const std::string &name);

        static LogLevel verbosity();

        static void verbosity(LogLevel verbosity);

        static Logger &endl(Logger &__l);

        static Logger error();

        static Logger info();

        static Logger warning();

        static Logger debug();

        static Logger notice();

        static std::string err();

        explicit Logger(LogLevel logLevel);

        Logger(const Logger &logger);

        ~Logger();

        Logger &operator<<(const std::string &str);

        Logger &operator<<(Logger &(*pf)(Logger &));

        Logger &operator<<(int n);

        void flush();

    private:
        LogLevel _logLevel;
        std::stringstream _ss;
    };
}

