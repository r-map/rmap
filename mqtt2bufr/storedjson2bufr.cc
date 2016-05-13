/*
 * storedjson2bufr - Convert stored JSON to generic BUFR
 *
 * Copyright (C) 2015  ARPA-SIM <urpsim@smr.arpa.emr.it>
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
 */
#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>
#include <iostream>
#include <stdexcept>
#include <vector>

#include <getopt.h>
#include <string.h>

#include <dballe/msg/msg.h>
#include <dballe/msg/wr_codec.h>
#include <dballe/core/record.h>

#include "parser.h"


struct FpRAII {
    FILE* fp = nullptr;
    bool close_on_exit = false;

    FpRAII(const std::string& name) {
        if (name == "-") {
            fp = stdin;
            close_on_exit = false;
        } else {
            fp = fopen(name.c_str(), "r");
            close_on_exit = true;
            if (fp == nullptr)
                throw std::runtime_error("Cannot open file " + name);
        }
    }
    ~FpRAII() {
        if (close_on_exit)
            fclose(fp);
    }
};

void print_help(std::ostream& out)
{
    out << "Usage: storedjson2bufr [OPTIONS] [FILE...]" << std::endl
        << "Convert stored JSON to generic BUFR. "
        << "With no FILE, or when FILE is -, read standard input." << std::endl
        << "Options are" << std::endl
        << " --help             show this help and exit" << std::endl
        << " --version          show version and exit" << std::endl
        << " --exclude-sent     exclude records already sent" << std::endl
        << std::endl
        << "Report bugs to: " << PACKAGE_BUGREPORT << std::endl;
        ;
}

void print_version(std::ostream& out)
{
    out << "storedjson2bufr " << PACKAGE_VERSION << std::endl;
}

int main(int argc, char** argv)
{
    static int show_help = 0;
    static int show_version = 0;
    static int exclude_sent = 0;
    std::vector<std::string> files;

    while (1) {
        int c;
        int opt_idx = 0;
        static struct option opts[] = {
            { "help", no_argument, &show_help, 1 },
            { "version", no_argument, &show_version, 1 },
            { "exclude-sent", no_argument, &exclude_sent, 1},
            { 0, 0, 0, 0 }
        };

        c = getopt_long(argc, argv, "", opts, &opt_idx);
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
            default:
                print_help(std::cerr);
                return 1;
        }
    }

    for (int i = optind; i < argc; ++i)
        files.push_back(argv[i]);

    if (files.empty())
        files.push_back("-");

    int return_value = 0;
    for (auto file: files) {
        FpRAII f(file);
        char buf[128];
        while (fread(buf, sizeof(buf), 1, f.fp)) {
            bool already_sent = buf[0];
            if (exclude_sent and already_sent)
                continue;
            dballe::Messages msgs;
            dballe::msg::BufrExporter exporter;
            mqtt2bufr::Parser parser;
            std::string topic(buf + 1, buf + 63);
            std::string payload(buf + 65, buf + 128);
            try {
                dballe::Msg msg = parser.parse(topic, payload);
                msgs.append(msg);
                std::cout << exporter.to_binary(msgs);
            } catch (const std::exception& e) {
                return_value = 1;
                std::cerr << "Error while parsing "
                    << file << "[" << topic << " " << payload << "]"
                    << ": " << e.what() << std::endl;
            }
        }
    }

    return return_value;
}
