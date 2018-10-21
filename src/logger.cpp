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

#include <cstdio>
#include <cstdarg>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <sstream>

#include "ndppd.h"
#include "logger.h"

#ifndef DISABLE_SYSLOG
#include <syslog.h>
#endif

using namespace ndppd;

namespace {
    const char *logLevel_names[] = {
            "Error",
            "Warning",
            "Notice",
            "Info",
            "Debug",
            nullptr
    };

    bool use_syslog;

    LogLevel verbosity = LogLevel::Info;
}

Logger::Logger(LogLevel logLevel) :
        _logLevel(logLevel) {
}

Logger::Logger(const Logger &logger) :
        _logLevel(logger._logLevel), _ss(logger._ss.str()) {
}

Logger::~Logger() {
    flush();
}

std::string Logger::format(const char *fmt, ...) {
    char buf[2048];
    va_list va;
    va_start(va, fmt);
    vsnprintf(buf, sizeof(buf), fmt, va);
    va_end(va);
    return buf;
}

std::string Logger::err() {
    char buf[2048];

#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && !_GNU_SOURCE
    if (strerror_r(errno, buf, sizeof(buf))
        return "Unknown error";
    return buf;
#else
    return strerror_r(errno, buf, sizeof(buf));
#endif
}

Logger Logger::error() {
    return Logger(LogLevel::Error);
}

Logger Logger::info() {
    return Logger(LogLevel::Info);
}

Logger Logger::warning() {
    return Logger(LogLevel::Warning);
}

Logger Logger::debug() {
    return Logger(LogLevel::Debug);
}

Logger Logger::notice() {
    return Logger(LogLevel::Notice);
}

Logger &Logger::operator<<(const std::string &str) {
    _ss << str;
    return *this;
}

Logger &Logger::operator<<(int n) {
    _ss << n;
    return *this;
}

Logger &Logger::operator<<(Logger &(*pf)(Logger &)) {
    pf(*this);
    return *this;
}

Logger &Logger::endl(Logger &__l) {
    __l.flush();
    return __l;
}

void Logger::flush() {
    if (!_ss.rdbuf()->in_avail())
        return;

    if (_logLevel > ::verbosity)
        return;

#ifndef DISABLE_SYSLOG
    if (use_syslog) {
        int pri;
        switch (_logLevel) {
            case LogLevel::Error:
                pri = LOG_ERR;
                break;
            case LogLevel::Warning:
                pri = LOG_WARNING;
                break;
            case LogLevel::Notice:
                pri = LOG_NOTICE;
                break;
            case LogLevel::Info:
                pri = LOG_INFO;
                break;
            case LogLevel::Debug:
                pri = LOG_DEBUG;
                break;
            default:
                return;
        }

        ::syslog(pri, "(%s) %s", logLevel_names[(int) _logLevel], _ss.str().c_str());
        return;
    }
#endif

    std::cout << "(" << logLevel_names[(int) _logLevel] << ") " << _ss.str() << std::endl;
    _ss.str("");
}

#ifndef DISABLE_SYSLOG

bool Logger::syslog(bool enable) {
    if (enable == use_syslog)
        return use_syslog;

    if ((use_syslog = enable)) {
        setlogmask(LOG_UPTO(LOG_DEBUG));
        openlog("ndppd", LOG_CONS | LOG_NDELAY | LOG_PERROR | LOG_PID, LOG_USER);
    } else {
        closelog();
    }

    return use_syslog;
}

bool Logger::syslog() {
    return use_syslog;
}

#endif

LogLevel Logger::verbosity() {
    return ::verbosity;
}

void Logger::verbosity(LogLevel verbosity) {
    ::verbosity = verbosity;
}

bool Logger::verbosity(const std::string &name) {
    const char *c_name = name.c_str();

    if (!*c_name) {
        return false;
    }

    if (isdigit(*c_name)) {
        ::verbosity = (LogLevel) atoi(c_name);
        return true;
    }

    for (int i = 0; logLevel_names[i]; i++) {
        if (!strcmp(c_name, logLevel_names[i])) {
            ::verbosity = (LogLevel) i;
            return true;
        }
    }

    return false;
}
