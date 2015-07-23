/*
 * bufr2mqtt - Convert BUFR to MQTT messages
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

#include <unistd.h>
#include <iostream>

#include <mosquittopp.h>

#include <jansson.h>

#include <getopt.h>
#include <string.h>

#include <wreport/bulletin.h>
#include <dballe/msg/msg.h>
#include <dballe/msg/msgs.h>
#include <dballe/msg/wr_codec.h>

#include "parser.h"

struct Publisher : mosqpp::mosquittopp {
    std::vector<std::string> topics;
    bool debug;
    int last_ack_mid;
    int last_pub_mid;

    Publisher(const std::vector<std::string>& topics, bool debug=false) : topics(topics), debug(debug), last_ack_mid(-1), last_pub_mid(-1) {}

    virtual void on_log(int level, const char *str) {
      if (debug)
        std::cerr << str << std::endl;
    }

    virtual void on_publish(int mid) {
      last_ack_mid = mid;
    }

    bool all_sent() const {
      return last_ack_mid == last_pub_mid;
    }

    void publish_msgs(const dballe::Msgs& msgs) {
        bufr2mqtt::Parser parser;
        for (dballe::Msgs::const_iterator i = msgs.begin(); i != msgs.end(); ++i) {
            int date[6];
            dballe::Msg* msg = *i;
            msg->parse_date(date);
            const dballe::msg::Context* station_context = msg->find_station_context();
            for (std::vector<dballe::msg::Context*>::const_iterator j = msg->data.begin();
                 j != msg->data.end(); ++j) {
                dballe::msg::Context* ctx = *j;
                for (std::vector<wreport::Var*>::const_iterator k = ctx->data.begin();
                     k != ctx->data.end(); ++k) {
                    const wreport::Var* var = *k;
                    // Skip date from station context
                    if (ctx == station_context && (
                            var->code() == WR_VAR(0, 4,  1) ||
                            var->code() == WR_VAR(0, 4,  2) ||
                            var->code() == WR_VAR(0, 4,  3) ||
                            var->code() == WR_VAR(0, 4,  4) ||
                            var->code() == WR_VAR(0, 4,  5) ||
                            var->code() == WR_VAR(0, 4,  6)
                            ))
                      continue;
                    std::string topic;
                    std::string payload;
                    bool retain = ( ctx->is_station() ? true : false );
                    parser.parse(*var, ctx->level, ctx->trange,
                                 *station_context,
                                 date,
                                 topic, payload);
                    for (std::vector<std::string>::const_iterator t = topics.begin();
                         t != topics.end(); ++t) {
                      // TODO: do something if publish() fails
                      int mosqerr;
                      if ((mosqerr = publish(&last_pub_mid, (*t + topic).c_str(), payload.size(), payload.c_str(), 1, retain)) != MOSQ_ERR_SUCCESS) {
                        std::cerr << "Error while publishing message"
                                  << ": " << mosqpp::strerror(mosqerr)
                                  << std::endl;
                      }
		      if (loop() != MOSQ_ERR_SUCCESS) {
                        std::cerr << "Error while calling mosquitto loop: "
                                  << mosqpp::strerror(mosqerr)
                                  << std::endl;
                      }
                    }
                }
            }
        }
    }
};

void print_help(std::ostream& out)
{
    out << "Usage: bufr2mqtt [OPTIONS]" << std::endl
        << "Publish BUFR messages to MQTT" << std::endl
        << "Options are" << std::endl
        << " --help             show this help and exit" << std::endl
        << " --version          show version and exit" << std::endl
        << " -h,--host NAME     host to connect to (default: localhost)" << std::endl
        << " -k,--keepalive SEC seconds between sending PING commands to the broker (default: 60s)" << std::endl
        << " -p,--port PORT     connect to the port specified (default: 1883)" << std::endl
        << " -t,--topic TOPIC   MQTT topic prefix to publish to (may be repeated multiple times)" << std::endl
        << " -u,--username NAME username for authenticating with the broker" << std::endl
        << " -P,--pw PASSWORD   password for authenticating with the broker" << std::endl
        << " -d,--debug         enable debug messages" << std::endl
        << std::endl
        << "Report bugs to: " << PACKAGE_BUGREPORT << std::endl;
        ;
}

void print_version(std::ostream& out)
{
    out << "bufr2mqtt " << PACKAGE_VERSION << std::endl;
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
    int mosqerr;
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
    Publisher publisher(topics, debug);

    if ((mosqerr = publisher.username_pw_set(username, password)) != 0) {
        std::cerr << "Error while setting username and password"
                  << ": " << mosqpp::strerror(mosqerr)
                  << std::endl;
        return 1;
    }
    if ((mosqerr = publisher.connect(hostname.c_str(), port, keepalive)) != 0) {
        std::cerr << "Error while connecting to " << hostname << ":" << port
                  << ": " << mosqpp::strerror(mosqerr)
                  << std::endl;
        return 1;
    }


    dballe::Rawmsg raw;
    std::auto_ptr<dballe::msg::Importer> importer = dballe::msg::Importer::create(dballe::BUFR);

    while (wreport::BufrBulletin::read(stdin, raw, "(stdin)", &raw.offset)) {
        dballe::Msgs msgs;
        importer->from_rawmsg(raw, msgs);
        publisher.publish_msgs(msgs);
    }
    while (not publisher.all_sent()) {
      usleep(1000000);
      publisher.loop();
    }

    if ((mosqerr = publisher.disconnect()) != 0) {
        std::cerr << "Error while disconnetting from "
                  << hostname << ":" << port
                  << ": " << mosqpp::strerror(mosqerr)
                  << std::endl;
        return 1;
    }

    mosqpp::lib_cleanup();

    if (not publisher.all_sent()) {
      std::cerr << "Ack timeout error." << std::endl;
      return 2;
    }

    return 0;
}
