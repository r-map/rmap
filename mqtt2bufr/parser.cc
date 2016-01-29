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

#include "parser.h"

#include <regex.h>

#include <iostream>

#include <jansson.h>

#include <wibble/grcal/grcal.h>

#define IDENT_RE "([^/]+)"
#define LON_RE   "([0-9]+)"
#define LAT_RE   "([0-9]+)"
#define REP_RE   "([^/]+)"
#define LT1_RE   "([0-9]+|-)"
#define L1_RE    "([0-9]+|-)"
#define LT2_RE   "([0-9]+|-)"
#define L2_RE    "([0-9]+|-)"
#define PIND_RE  "([0-9]+|-)"
#define P1_RE    "([0-9]+|-)"
#define P2_RE    "([0-9]+|-)"
#define VAR_RE   "(B[0-9]{5})"

#define TOPIC_RE "^.*/" IDENT_RE "/" LON_RE "," LAT_RE "/" REP_RE "/" PIND_RE "," P1_RE "," P2_RE "/" LT1_RE "," L1_RE "," LT2_RE "," L2_RE "/" VAR_RE "$"

#define throw_regexception(errcode, preg, errbuf, prefixmsg) do { regerror(errcode, preg, errbuf, sizeof(errbuf)); throw std::runtime_error(std::string(prefixmsg) + std::string(errbuf)); } while(0);

struct RAIIRegexp {
    regex_t* re;
    RAIIRegexp(regex_t* re) : re(re) {}
    ~RAIIRegexp() { regfree(re); }
};

struct RAIIJson {
    json_t* root;
    RAIIJson(json_t* root) : root(root) {}
    ~RAIIJson() { json_decref(root); }
};

namespace mqtt2bufr {

static std::vector<std::string> split_topic(const std::string& topic) {
    int r;
    char errmsg[1024];
    int nmatches = 13;
    regmatch_t matches[nmatches];
    regex_t re;
    std::vector<std::string> items;

    r = regcomp(&re, TOPIC_RE, REG_EXTENDED);
    RAIIRegexp raiiregexp(&re);
    if (r != 0) throw_regexception(r, &re, errmsg, "While compiling topic regexp: ");
    r = regexec(&re, topic.c_str(), nmatches, matches, 0);
    if (r != 0) throw_regexception(r, &re, errmsg, "While parsing topic: ");

    for (int i = 1; i < nmatches; ++i) { 
        items.push_back(topic.substr(matches[i].rm_so, matches[i].rm_eo - matches[i].rm_so));
    }
    return items;
}

static void parse_datetime(const std::string str, int* values)
{
    int r;
    char errmsg[1024];
    int nmatches = 7;
    regex_t re;
    regmatch_t matches[nmatches];

    r = regcomp(&re,
                "^([0-9]{4})-([0-9]{2})-([0-9]{2})[T ]([0-9]{2}):([0-9]{2}):([0-9]{2})$",
                REG_EXTENDED);
    RAIIRegexp raiiregexp(&re);
    if (r != 0) throw_regexception(r, &re, errmsg, "While compiling datetime regexp: ");
    r = regexec(&re, str.c_str(), nmatches, matches, 0);
    if (r != 0) throw_regexception(r, &re, errmsg, "While parsing datetime: ");

    for (int i = 1; i < nmatches; ++i) { 
        values[i-1] = atoi(str.substr(matches[i].rm_so, matches[i].rm_eo - matches[i].rm_so).c_str());
    }
}

void Parser::parse_topic(const std::string& topic) {
    // split topic by "/" delimiter
    std::vector<std::string> items = split_topic(topic);
    // set station ident
    if (items[0] != "-")
        station_rec.set_var(dballe::var(WR_VAR(0, 1, 11), items[0].c_str()));
    // set station coordinates
    station_rec.set_var(dballe::var(WR_VAR(0, 6,  1), items[1].c_str()));
    station_rec.set_var(dballe::var(WR_VAR(0, 5,  1), items[2].c_str()));
    // set station rep_memo
    station_rec.set_var(dballe::var(WR_VAR(0, 1,194), items[3].c_str()));
    // set variable trange
    variable_rec.setf("pindicator", items[4].c_str());
    variable_rec.setf("p1", items[5].c_str());
    variable_rec.setf("p2", items[6].c_str());
    // set variable level
    variable_rec.setf("leveltype1", items[7].c_str());
    variable_rec.setf("l1", items[8].c_str());
    variable_rec.setf("leveltype2", items[9].c_str());
    variable_rec.setf("l2", items[10].c_str());
    // set variable
    variable_rec.setf("var", items[11].c_str());
}

void Parser::parse_payload(const std::string& payload) {
    wreport::Var var(dballe::varinfo(variable_rec["var"].enqs()));
    json_t* root = json_loads(payload.c_str(), 0, NULL);
    RAIIJson raiijson(root);
    if (!json_is_object(root))
        throw std::runtime_error("Payload is not a valid JSON object (document is not a JSON object)");
    // Set the value
    json_t* v = json_object_get(root, "v");
    if (json_is_string(v))
        var.set(json_string_value(v));
    else if (json_is_integer(v))
        var.set((int)json_integer_value(v));
    else if (json_is_real(v))
        var.set(json_real_value(v));
    else
        throw std::runtime_error("Payload is not a valid JSON object (value associated to key \"v\" is not a string, integer or real)");
    // Parse datetime when data are not in station context
    if (variable_rec.get_level() != dballe::Level() &&
        variable_rec.get_trange() != dballe::Trange()) {
        json_t* t = json_object_get(root, "t");
        int date[6];
        // A datetime missing or null means "now"
        if (!t || json_is_null(t))
            wibble::grcal::date::now(date);
        else if (json_is_string(t))
            parse_datetime(json_string_value(t), date);
        else
            throw std::runtime_error("Payload is not a valid JSON object (value associated to key \"t\" is not a string)");

        variable_rec.seti("year", date[0]);
        variable_rec.seti("month", date[1]);
        variable_rec.seti("day", date[2]);
        variable_rec.seti("hour", date[3]);
        variable_rec.seti("min", date[4]);
        variable_rec.seti("sec", date[5]);
    }else{
      variable_rec.set_datetime(Datetime());
    }
    // Parse attributes (if any)
    if (json_object_iter_at(root, "a")) {
        json_t* a = json_object_get(root, "a");
        if (!json_is_object(a))
            throw std::runtime_error("Payload is not a valid JSON object (value associated to key \"a\" is not an object)");
        void* i = json_object_iter(a);
        while (i) {
            const char* k = json_object_iter_key(i);
            json_t* av = json_object_iter_value(i);
            const char* s = json_string_value(av);
            var.seta(dballe::var(k, s));
            i = json_object_iter_next(a, i);
        }
    }
    variable_rec.set(var);
}
dballe::Msg Parser::parse(const std::string& topic, const std::string& payload) {
    // clear records
    station_rec.clear();
    variable_rec.clear();
    attributes_rec.clear();

    // parse topic
    parse_topic(topic);
    // parse payload
    parse_payload(payload);

    // create message
    dballe::Msg msg;
    std::vector<wreport::Var*> vars;
    std::vector<wreport::Var*>::const_iterator it;

    vars = station_rec.vars();
    for (it = vars.begin(); it != vars.end(); ++it) {
        msg.set(**it, (*it)->code(), station_rec.get_level(), station_rec.get_trange());
    }

    vars = variable_rec.vars();
    for (it = vars.begin(); it != vars.end(); ++it) {
        msg.set(**it, (*it)->code(), variable_rec.get_level(), variable_rec.get_trange());
    }

    // set datetime from variable_rec
    msg.set_datetime(variable_rec.get_datetime());

    return msg;
}

}

#include <sstream>

namespace bufr2mqtt {

void Parser::parse(const wreport::Var& var, const dballe::Level& level, const dballe::Trange& trange,
                   const dballe::msg::Context& station_context,
                   const dballe::Datetime& datetime,
                   std::string& topic, std::string& payload) {
    topic.clear();
    payload.clear();
    topic = "/";
    if (const wreport::Var* v = station_context.find(WR_VAR(0, 1, 11)))
        topic += v->enqc();
    else
        topic += "-";
    topic += "/";
    if (const wreport::Var* v = station_context.find(WR_VAR(0, 6,  1)))
        topic += v->enqc();
    else
        topic += "-";
    topic +=",";
    if (const wreport::Var* v = station_context.find(WR_VAR(0, 5,  1)))
        topic += v->enqc();
    else
        topic += "-";
    topic += "/";
    if (const wreport::Var* v = station_context.find(WR_VAR(0, 1,194)))
        topic += v->enqc();
    else
        topic += "-";
    topic += "/";
    {
        std::stringstream ss;
        trange.to_stream(ss);
        topic += ss.str();
    }
    topic += "/";
    {
        std::stringstream ss;
        // NOTE: at this moment, the station info level is not (-,-,-,-), so we
        // have to translate from dballe internal representation.
        level.to_stream(ss);
        topic += ss.str();
    }
    topic += "/";
    topic += wreport::varcode_format(var.code());

    json_t* root = json_object();
    json_t* v;
    RAIIJson raiijson(root);
    v = json_string(var.enqc());
    json_object_set_new(root, "v", v);
    if (level != dballe::Level() && trange != dballe::Trange()) {
        std::stringstream ss;
        datetime.to_stream_iso8601(ss, 'T', "");
        v = json_string(ss.str().c_str());
        json_object_set_new(root, "t", v);
    }
    v = NULL;
    for (const wreport::Var* a = var.next_attr(); a != NULL; a = a->next_attr()) {
        if (!v) {
            v = json_object();
            json_object_set_new(root, "a", v);
        }
        json_t* av = json_string(a->enqc());
        json_object_set_new(v, wreport::varcode_format(a->code()).c_str(), av);
    }
    char* s = json_dumps(root, 0);
    payload = s;
    free(s);
}

}
