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
#include <cstdlib>
#include <csignal>

#include <iostream>
#include <fstream>
#include <string>
#include <memory>

#include <getopt.h>
#include <sys/time.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "ndppd.h"

#include "netlink.h"
#include "socket.h"

using namespace ndppd;

static int daemonize() {
    pid_t pid = fork();
    if (pid < 0) {
        Logger::error() << "Failed to fork during daemonize: " << Logger::err();
        return -1;
    }

    if (pid > 0)
        exit(0);

    umask(0);

    pid_t sid = setsid();
    if (sid < 0) {
        Logger::error() << "Failed to setsid during daemonize: " << Logger::err();
        return -1;
    }

    if (chdir("/") < 0) {
        Logger::error() << "Failed to change path during daemonize: " << Logger::err();
        return -1;
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    return 0;
}

static std::shared_ptr<conf> load_config(const std::string &path) {
    std::shared_ptr<conf> cf, x_cf;

    if (!(cf = conf::load(path)))
        return {};

    std::vector<std::shared_ptr<conf> >::const_iterator p_it;

    std::vector<std::shared_ptr<conf> > proxies(cf->find_all("proxy"));

    for (p_it = proxies.begin(); p_it != proxies.end(); p_it++) {
        std::shared_ptr<conf> pr_cf = *p_it;

        if (pr_cf->empty()) {
            Logger::error() << "'proxy' section is missing interface name";
            return {};
        }

        std::vector<std::shared_ptr<conf> >::const_iterator r_it;

        std::vector<std::shared_ptr<conf> > rules(pr_cf->find_all("rule"));

        for (r_it = rules.begin(); r_it != rules.end(); r_it++) {
            std::shared_ptr<conf> ru_cf = *r_it;

            if (ru_cf->empty()) {
                Logger::error() << "'rule' is missing an IPv6 address/net";
                return {};
            }

            Cidr addr(*ru_cf);

            if (x_cf = ru_cf->find("iface")) {
                if (ru_cf->find("static") || ru_cf->find("auto")) {
                    Logger::error()
                            << "Only one of 'iface', 'auto' and 'static' may "
                            << "be specified.";
                    return {};
                }
                if ((const std::string &) *x_cf == "") {
                    Logger::error() << "'iface' expected an interface name";
                    return {};
                }
            } else if (ru_cf->find("static")) {
                if (ru_cf->find("auto")) {
                    Logger::error()
                            << "Only one of 'iface', 'auto' and 'static' may "
                            << "be specified.";
                    return {};
                }
                if (addr.prefix() <= 120) {
                    Logger::warning()
                            << "Low prefix length (" << addr.prefix()
                            << " <= 120) when using 'static' method";
                }
            } else if (!ru_cf->find("auto")) {
                Logger::error()
                        << "You must specify either 'iface', 'auto' or "
                        << "'static'";
                return {};

            }
        }
    }

    return cf;
}

static bool configure(std::shared_ptr<conf> &cf) {
    std::shared_ptr<conf> x_cf;

    std::list<std::shared_ptr<rule> > myrules;

    std::vector<std::shared_ptr<conf> >::const_iterator p_it;

    std::vector<std::shared_ptr<conf> > proxies(cf->find_all("proxy"));

    for (p_it = proxies.begin(); p_it != proxies.end(); p_it++) {
        std::shared_ptr<conf> pr_cf = *p_it;

        if (pr_cf->empty()) {
            return false;
        }

        bool promiscuous = false;
        if (!(x_cf = pr_cf->find("promiscuous")))
            promiscuous = false;
        else
            promiscuous = *x_cf;

        std::shared_ptr<proxy> pr = proxy::open(*pr_cf, promiscuous);
        if (!pr) {
            return false;
        }

        if (!(x_cf = pr_cf->find("router")))
            pr->router(true);
        else
            pr->router(*x_cf);

        if (!(x_cf = pr_cf->find("autowire")))
            pr->autowire(false);
        else
            pr->autowire(*x_cf);

        if (!(x_cf = pr_cf->find("keepalive")))
            pr->keepalive(true);
        else
            pr->keepalive(*x_cf);

        if (!(x_cf = pr_cf->find("retries")))
            pr->retries(3);
        else
            pr->retries(*x_cf);

        if (!(x_cf = pr_cf->find("ttl")))
            pr->ttl(30000);
        else
            pr->ttl(*x_cf);

        if (!(x_cf = pr_cf->find("deadtime")))
            pr->deadtime(pr->ttl());
        else
            pr->deadtime(*x_cf);

        if (!(x_cf = pr_cf->find("timeout")))
            pr->timeout(500);
        else
            pr->timeout(*x_cf);

        std::vector<std::shared_ptr<conf> >::const_iterator r_it;

        std::vector<std::shared_ptr<conf> > rules(pr_cf->find_all("rule"));

        for (r_it = rules.begin(); r_it != rules.end(); r_it++) {
            std::shared_ptr<conf> ru_cf = *r_it;

            Cidr addr(*ru_cf);

            bool autovia = false;
            if (!(x_cf = ru_cf->find("autovia")))
                autovia = false;
            else
                autovia = *x_cf;

            if (x_cf = ru_cf->find("iface")) {
                std::shared_ptr<iface> ifa = iface::open_ifd(*x_cf);
                if (!ifa) {
                    return false;
                }

                ifa->add_parent(pr);

                myrules.push_back(pr->add_rule(addr, ifa, autovia));
            } else if (ru_cf->find("auto")) {
                myrules.push_back(pr->add_rule(addr, true));
            } else {
                myrules.push_back(pr->add_rule(addr, false));
            }
        }
    }

    // Print out all the topology    
    for (auto i_it = iface::_map.begin(); i_it != iface::_map.end(); i_it++) {
        std::shared_ptr<iface> ifa = i_it->second.lock();

        Logger::debug() << "iface " << ifa->name() << " {";

        for (auto &weak_proxy : ifa->serves()) {
            std::shared_ptr<proxy> proxy = weak_proxy.lock();
            if (!proxy) continue;

            Logger::debug() << "  " << "proxy " << Logger::format("%x", proxy.get()) << " {";

            for (auto &rule : proxy->rules()) {
                Logger::debug() << "    " << "rule " << Logger::format("%x", rule.get()) << " {";
                Logger::debug() << "      " << "taddr " << rule->cidr() << ";";
                if (rule->is_auto())
                    Logger::debug() << "      " << "auto;";
                else if (!rule->daughter())
                    Logger::debug() << "      " << "static;";
                else
                    Logger::debug() << "      " << "iface " << rule->daughter()->name() << ";";
                Logger::debug() << "    }";
            }

            Logger::debug() << "  }";
        }

        Logger::debug() << "  " << "parents {";
        for (auto &weak_proxy : ifa->parents()) {
            std::shared_ptr<proxy> proxy = weak_proxy.lock();
            Logger::debug() << "    " << "parent " << Logger::format("%x", proxy.get()) << ";";
        }

        Logger::debug() << "  }";
        Logger::debug() << "}";
    }

    return true;
}

static bool running = true;

static void exit_ndppd(int sig) {
    Logger::error() << "Shutting down...";
    running = 0;
}

int main(int argc, char *argv[], char *env[]) {
    signal(SIGINT, exit_ndppd);
    signal(SIGTERM, exit_ndppd);

    std::string config_path("/etc/ndppd.conf");
    std::string pidfile;
    std::string verbosity;
    bool daemon = false;

    while (1) {
        int c, opt;

        static struct option long_options[] =
                {
                        {"config",  1, 0, 'c'},
                        {"daemon",  0, 0, 'd'},
                        {"verbose", 1, 0, 'v'},
                        {0,         0, 0, 0}
                };

        c = getopt_long(argc, argv, "c:dp:v", long_options, &opt);

        if (c == -1)
            break;

        switch (c) {
            case 'c':
                config_path = optarg;
                break;

            case 'd':
                daemon = true;
                break;

            case 'p':
                pidfile = optarg;
                break;

            case 'v':
                Logger::verbosity((LogLevel) ((int) Logger::verbosity() + 1));
                /*if (optarg) {
                    if (!logger::verbosity(optarg))
                        logger::error() << "Unknown verbosity level '" << optarg << "'";
                } else {
                    logger::max_pri(LOG_INFO);
                }*/
                break;
        }
    }

    Logger::notice()
            << "ndppd (NDP Proxy Daemon) version " NDPPD_VERSION << Logger::endl
            << "Using configuration file '" << config_path << "'";

    // Load configuration.

    std::shared_ptr<conf> cf = load_config(config_path);
    if (!cf)
        return -1;

    if (!configure(cf))
        return -1;

    if (daemon) {
        Logger::syslog(true);

        if (daemonize() < 0)
            return 1;
    }

    if (!pidfile.empty()) {
        std::ofstream pf;
        pf.open(pidfile.c_str(), std::ios::out | std::ios::trunc);
        pf << getpid() << std::endl;
        pf.close();
    }

    // Time stuff.

    struct timeval t1, t2;

    gettimeofday(&t1, 0);

#ifdef WITH_ND_NETLINK
    netlink_setup();
#endif
    Netlink::initialize();
    Netlink::load_local_ips();

    while (running) {
        Socket::poll();

        if (iface::poll_all() < 0) {
            if (running) {
                Logger::error() << "iface::poll_all() failed";
            }
            break;
        }

        int elapsed_time;
        gettimeofday(&t2, 0);

        elapsed_time =
                ((t2.tv_sec - t1.tv_sec) * 1000) +
                ((t2.tv_usec - t1.tv_usec) / 1000);

        t1.tv_sec = t2.tv_sec;
        t1.tv_usec = t2.tv_usec;

        session::update_all(elapsed_time);
    }

#ifdef WITH_ND_NETLINK
    netlink_teardown();
#endif


    Logger::notice() << "Bye";

    return 0;
}

