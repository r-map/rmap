/*
 * mqtt2bufr - Convert MQTT messages to generic BUFR
 *
 * Copyright (C) 2013  ARPA-SIM <urpsim@smr.arpa.emr.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Authors: Emanuele Di Giacomo <edigiacomo@arpa.emr.it>
 *          Paolo Patruno <p.patruno@iperbole.bologna.it>
 */
#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>

#include <mosquittopp.h>

#include <jansson.h>

#include <getopt.h>
#include <string.h>

#include <dballe/msg/msg.h>
#include <dballe/msg/wr_codec.h>

#include "parser.h"

struct mosq : public mosqpp::mosquittopp {
    mqtt2bufr::Parser parser;
    bool debug;

    mosq(bool debug=false) : debug(debug) {}

    virtual void on_message(const struct mosquitto_message *message) {
        dballe::Msg msg;
        try {
            msg = parser.parse(message->topic,
                               std::string((const char*)message->payload,
                                           message->payloadlen));
            dballe::Messages msgs;
            dballe::msg::BufrExporter exporter;
            msgs.append(msg);
            std::cout << exporter.to_binary(msgs) << std::flush;
        } catch(const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    virtual void on_log(int level, const char *str) {
      if (debug)
        std::cerr << str << std::endl;
    }
};

void print_help(std::ostream& out)
{
    out << "Usage: mqtt2bufr [OPTIONS]" << std::endl
        << "MQTT subscriber for BUFR messages" << std::endl
        << "Options are" << std::endl
        << " --help             show this help and exit" << std::endl
        << " --version          show version and exit" << std::endl
        << " -h,--host NAME     host to connect to (default: localhost)" << std::endl
        << " -k,--keepalive SEC seconds between sending PING commands to the broker (default: 60s)" << std::endl
        << " -p,--port PORT     connect to the port specified (default: 1883)" << std::endl
        << " -t,--topic TOPIC   MQTT topic to subscribe to (my be repeated multiple times)" << std::endl
        << " -u,--username NAME username for authenticating with the broker" << std::endl
        << " -P,--pw PASSWORD   password for authenticating with the broker" << std::endl
        << " -d,--debug         enable debug messages" << std::endl
        << std::endl
        << "Report bugs to: " << PACKAGE_BUGREPORT << std::endl;
        ;
}

void print_version(std::ostream& out)
{
    out << "mqtt2bufr " << PACKAGE_VERSION << std::endl;
}

int main(int argc, char** argv)
{
    static int show_help = 0;
    static int show_version = 0;
    int keepalive = 60;
    int port = 1883;
    std::string hostname = "localhost";
    std::vector<std::string> topics;
    char* username = NULL;
    char* password = NULL;
    bool debug = false;

    while (1) {
        int c;
        int opt_idx = 0;
        static struct option opts[] = {
            { "help", no_argument, &show_help, 1 },
            { "version", no_argument, &show_version, 1 },
            { "host", required_argument, 0, 'h' },
            { "keepalive", required_argument, 0, 'k' },
            { "port", required_argument, 0, 'p' },
            { "topic", required_argument, 0, 't' },
            { "username", required_argument, 0, 'u' },
            { "pw", required_argument, 0, 'P' },
            { "debug", no_argument, 0, 'd' },
            { 0, 0, 0, 0 }
        };

        c = getopt_long(argc, argv,
                        "h:k:p:t:u:P:d",
                        opts, &opt_idx);
        if (c == -1)
            break;

        switch (c) {
            case 0:
                if (show_help) {
                  print_help(std::cout);
                  return 0;
                }
                if (show_version) {
                  print_version(std::cout);
                  return 0;
                }
                break;
            case 'h':
                hostname = optarg;
                break;
            case 'k':
                keepalive = atoi(optarg);
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 't':
                topics.push_back(optarg);
                break;
            case 'u':
                username = strdup(optarg);
                break;
            case 'P':
                password = strdup(optarg);
                break;
            case 'd':
                debug = true;
                break;
            default:
                print_help(std::cerr);
                return 1;
        }
    }

    mosqpp::lib_init();
    mosq m(debug);

    if (m.username_pw_set(username, password) != 0) {
        std::cerr << "Error while setting username and password" << std::endl;
        return 1;
    }
    if (m.connect(hostname.c_str(), port, keepalive) != 0) {
        std::cerr << "Error while connecting to " << hostname << ":" << port << std::endl;
        return 1;
    }
    for (std::vector<std::string>::const_iterator i = topics.begin();
         i != topics.end(); ++i) {
        if (m.subscribe(NULL, i->c_str()) != 0) {
            std::cerr << "Error while subscribing to topic " << *i << std::endl;
            return 1;
        }
    }
    while (m.loop() == 0) {}

    if (m.disconnect() != 0) {
        std::cerr << "Error while disconnetting from " << hostname << ":" << port << std::endl;
        return 1;
    }

    mosqpp::lib_cleanup();
    return 0;
}
