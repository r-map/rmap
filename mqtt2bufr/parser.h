/*
 * parser - Parser class
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
 * Author: Emanuele Di Giacomo <edigiacomo@arpa.emr.it>
 *         Paolo Patruno <p.patruno@iperbole.bologna.it>
 */
#ifndef MQTT2BUFR_PARSER_H
#define MQTT2BUFR_PARSER_H

#include <string>
#include <dballe/core/record.h>
#include <dballe/msg/msg.h>

namespace mqtt2bufr {
/**
 * This class convert a MQTT message in a dballe::Msg.
 *
 * Format of the MQTT message:
 * - topic: `.../IDENT/LON,LAT/REP_MEMO/PIND,P1,P2/LT1,L1,LT2,L2/VAR`
 * - payload: `{ "v": VALUE, "t": "DATETIME", "a": { "BXXYYY": "...", } }`
 *   - VALUE: value of the variable VAR
 *     - if string or integer: CREX format
 *     - if double (e.g. 12.3, 33.0) : CREX format / scale
 *   - DATETIME: `YYYY-mm-ddTHH:MM:SS` or `YYYY-mm-dd HH:MM:SS`
 */
class Parser {
 protected:
  dballe::core::Record station_rec;
  dballe::core::Record variable_rec;
  dballe::core::Record attributes_rec;

  /**
   * Parse the topic.
   */
  void parse_topic(const std::string& topic);
  /**
   * Parse payload.
   */
  void parse_payload(const std::string& payload);

 public:
  dballe::Msg parse(const std::string& topic, const std::string& payload);
};

}

#include <dballe/msg/context.h>

namespace bufr2mqtt {
/**
 * This class converts a (var, level, trange), station context and date to MQTT
 * topic and payload.
 *
 * @see mqtt2bufr::Parser for a description of the MQTT topic and payload.
 */
class Parser {
 public:
  void parse(const wreport::Var& var, const dballe::Level& level, const dballe::Trange& trange,
             const dballe::msg::Context& station_context,
             const dballe::Datetime& datetime,
             std::string& topic, std::string& payload);
};

}

#endif
