#!/usr/bin/env python3
# mqtt2bufr - Convert MQTT messages to generic BUFR
#
# Copyright (C) 2018  ARPA-SIM <urpsim@smr.arpa.emr.it>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
# Author: Emanuele Di Giacomo <edigiacomo@arpae.it>

import sys
import argparse
import re
import json
from datetime import datetime

import dballe
import paho.mqtt.client as mqtt


PACKAGE_BUGREPORT = "@PACKAGE_BUGREPORT@"
PACKAGE_VERSION = "@PACKAGE_VERSION@"

TOPIC_RE_V0 = re.compile((
    r'^.*/'
    r'(?P<ident>[^/]+)/'
    r'(?P<lon>[0-9-]+),'
    r'(?P<lat>[0-9-]+)/'
    r'(?P<rep>[^/]+)/'
    r'(?P<pind>[0-9]+|-),'
    r'(?P<p1>[0-9]+|-),'
    r'(?P<p2>[0-9]+|-)/'
    r'(?P<lt1>[0-9]+|-),'
    r'(?P<lv1>[0-9]+|-),'
    r'(?P<lt2>[0-9]+|-),'
    r'(?P<lv2>[0-9]+|-)/'
    r'(?P<var>B[0-9]{5})$'
))

TOPIC_RE_V1 = re.compile((
    r'^1/[^/]+/'
    r'(?P<user>[^/]+)/'
    r'(?P<ident>[^/]*)/'
    r'(?P<lon>[0-9-]+),'
    r'(?P<lat>[0-9-]+)/'
    r'(?P<rep>[^/]+)/'
    r'(?P<pind>[0-9]+|-),'
    r'(?P<p1>[0-9]+|-),'
    r'(?P<p2>[0-9]+|-)/'
    r'(?P<lt1>[0-9]+|-),'
    r'(?P<lv1>[0-9]+|-),'
    r'(?P<lt2>[0-9]+|-),'
    r'(?P<lv2>[0-9]+|-)/'
    r'(?P<var>B[0-9]{5})$'
))


def parse_topic(topic,version):
    if (version == 1):
        match = TOPIC_RE_V1.match(topic)
    else:
        match = TOPIC_RE_V0.match(topic)
        
    if match is None:
        return None
    else:
        g = match.groupdict()
        return {
            "ident": None if g["ident"] == "-" else g["ident"],
            "lon": int(g["lon"]),
            "lat": int(g["lat"]),
            "rep_memo": g["rep"],
            "level": (
                None if g["lt1"] == "-" else int(g["lt1"]),
                None if g["lv1"] == "-" else int(g["lv1"]),
                None if g["lt2"] == "-" else int(g["lt2"]),
                None if g["lv2"] == "-" else int(g["lv2"]),
            ),
            "trange": (
                None if g["pind"] == "-" else int(g["pind"]),
                None if g["p1"] == "-" else int(g["p1"]),
                None if g["p2"] == "-" else int(g["p2"]),
            ),
            "var": g["var"],
        }


def parse_payload(payload):
    return json.loads(payload)


def parse_message(topic, payload,version):
    t = parse_topic(topic,version)
    if t is None:
        return None

    m = parse_payload(payload)
    msg = t.copy()
    msg["value"] = m["v"]
    if all([
        t["level"] != (None, None, None, None),
        t["trange"] != (None, None, None),
    ]):
        if "t" in m:
            msg["datetime"] = datetime.strptime(m["t"], "%Y-%m-%dT%H:%M:%S")
        else:
            msg["datetime"] = datetime.now()
    else:
        msg["datetime"] = None

    if "a" in m:
        msg["attributes"] = m["a"]
    else:
        msg["attributes"] = {}

    return msg


def on_log(client, userdata, level, buf):
    if userdata["debug"]:
        sys.stderr.write("{}\n".format(buf))


def on_connect(client, userdata, flags, rc):
    for topic in userdata["topics"]:
        client.subscribe(topic)


def on_message(client, userdata, message):
    try:
        m = parse_message(message.topic, message.payload.decode("utf-8"),userdata["version"])
        if m is None:
            return
        msg = dballe.Message("generic")
        if m["ident"] is not None:
            msg.set_named("ident", dballe.var("B01011", m["ident"]))

        msg.set_named("longitude", dballe.var("B06001", m["lon"]))
        msg.set_named("latitude", dballe.var("B05001", m["lat"]))
        msg.set_named("rep_memo", dballe.var("B01194", m["rep_memo"]))

        if all([
            m["level"] != (None, None, None, None),
            m["trange"] != (None, None, None),
            userdata["overwrite_date"],
        ]):
            m["datetime"] = datetime.utcnow()

        if m["datetime"] is not None:
            msg.set_named("year", dballe.var("B04001", m["datetime"].year))
            msg.set_named("month", dballe.var("B04002", m["datetime"].month))
            msg.set_named("day", dballe.var("B04003", m["datetime"].day))
            msg.set_named("hour", dballe.var("B04004", m["datetime"].hour))
            msg.set_named("minute", dballe.var("B04005", m["datetime"].minute))
            msg.set_named("second", dballe.var("B04006", m["datetime"].second))

        var = dballe.var(m["var"], m["value"])

        for b, v in m["attributes"].items():
            var.seta(dballe.var(b, v))

        msg.set(m["level"], m["trange"], var)

        exporter = dballe.Exporter(encoding="BUFR")
        userdata["outfile"].write(exporter.to_binary(msg))
        userdata["outfile"].flush()
    except Exception:
        import traceback
        traceback.print_exc()



def write_message(topic,payload,outfile,version):
    try:
        m = parse_message(topic,payload,version)
        if m is None:
            return
        msg = dballe.Message("generic")
        if m["ident"] is not None:
            msg.set_named("ident", dballe.var("B01011", m["ident"]))
        msg.set_named("longitude", dballe.var("B06001", m["lon"]))
        msg.set_named("latitude", dballe.var("B05001", m["lat"]))
        msg.set_named("rep_memo", dballe.var("B01194", m["rep_memo"]))
        msg.set_named("year", dballe.var("B04001", m["datetime"].year))
        msg.set_named("month", dballe.var("B04002", m["datetime"].month))
        msg.set_named("day", dballe.var("B04003", m["datetime"].day))
        msg.set_named("hour", dballe.var("B04004", m["datetime"].hour))
        msg.set_named("minute", dballe.var("B04005", m["datetime"].minute))
        msg.set_named("second", dballe.var("B04006", m["datetime"].second))

        var = dballe.var(m["var"], m["value"])

        for b, v in m["attributes"].items():
            var.seta(dballe.var(b, v))

        msg.set(m["level"], m["trange"], var)

        exporter = dballe.Exporter(encoding="BUFR")
        outfile.write(exporter.to_binary(msg))
        outfile.flush()
    except Exception:
        import traceback
        traceback.print_exc()


        

def handle_signals(mqttclient):
    import signal

    def cleanup_on_signal(signum, frame):
        sys.stderr.write("Received signal {}\n".format(signum))
        if signum == signal.SIGHUP:
            mqttclient.reconnect()
        else:
            mqttclient.disconnect()
            sys.exit(255)

    signal.signal(signal.SIGHUP, cleanup_on_signal)
    signal.signal(signal.SIGTERM, cleanup_on_signal)
    signal.signal(signal.SIGINT, cleanup_on_signal)


def main(host, keepalive, port, topics, username, password, debug,
         overwrite_date, outfile,
         readfromfile,fileinput,roottopic,fileinfo,version):
    
    if (readfromfile):

        try:
            with fileinfo as i:
                model=i.readline()
                majorversion=i.readline()
                minorversion=i.readline()
                roottopic=i.readline()[:-1]
                MQTT_SENSOR_TOPIC_LENGTH=int(i.readline())
                MQTT_MESSAGE_LENGTH=int(i.readline())
        except:
            MQTT_SENSOR_TOPIC_LENGTH=38
            MQTT_MESSAGE_LENGTH=44
        
        reclen=MQTT_SENSOR_TOPIC_LENGTH+MQTT_MESSAGE_LENGTH
        with fileinput as f:
            while True:
                topic=f.read(MQTT_SENSOR_TOPIC_LENGTH).decode("utf-8").strip('\x00')
                if not topic:
                    break

                payload=f.read(MQTT_MESSAGE_LENGTH).decode("utf-8").strip('\x00')
                if not topic:
                    break

                write_message(topic,payload,outfile,version)

    else:
        mqttclient = mqtt.Client(userdata={
            "topics": topics,
            "debug": debug,
            "overwrite_date": overwrite_date,
            "outfile": outfile,
            "version": version
        })
        
        if username:
            mqttclient.username_pw_set(username, password)
            
        mqttclient.on_log = on_log
        mqttclient.on_connect = on_connect
        mqttclient.on_message = on_message
            
        mqttclient.connect(host, port=port, keepalive=keepalive)
        
        handle_signals(mqttclient)
            
        mqttclient.loop_forever()
            

if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog="mqtt2bufr", add_help=False,
                                     epilog="Report bugs to {}".format(
                                         PACKAGE_BUGREPORT
                                     ))
    parser.add_argument("--help", action="help",
                        help="show this help and exit")
    parser.add_argument("--version", action="version",
                        version="%(prog)s " + PACKAGE_VERSION,
                        help="show version and exit")
    group = parser.add_mutually_exclusive_group()

    group.add_argument("-i", action="store_true",dest="readfromfile",
                        help="read data from file (default false)")
    parser.add_argument("-f", "--file",dest="fileinput",
                        help="file to read; require -i (default: standard input)",
                        default="-",type=argparse.FileType('rb'))
    parser.add_argument("-a", "--fileinfo",dest="fileinfo",
                        help="info file to read; require -i (default: default info)",
                        default="info.dat",type=argparse.FileType('r'))
    parser.add_argument("-r", "--roottopic",
                        help="root topic used when reading from file (default %(default)s)",
                        default="test/myuser/1212345,4512345/fixed/")
    group.add_argument("-h", "--host",
                        help="host to connect to (default: localhost)",
                        default="localhost")
    parser.add_argument("-k", "--keepalive", metavar="SECONDS",
                        help=("seconds between sending PING commands to the "
                              "broker (default %(default)s)"),
                        type=int, default=60)
    parser.add_argument("-p", "--port", metavar="PORT",
                        help=("connect to the port specified "
                              "(default: %(default)s)"),
                        type=int, default=1883)
    parser.add_argument("-t", "--topic", metavar="TOPIC",
                        action="append", default=[],
                        help=("MQTT topic to subscribe to (may be repeated "
                              "multiple times"))
    parser.add_argument("-u", "--username", metavar="NAME",
                        help="username for authenticating with the broker")
    parser.add_argument("-P", "--pw", metavar="PASSWORD",
                        help="password for authenticating with the broker")
    parser.add_argument("-d", "--debug", action="store_true",
                        help="enable debug messages")
    parser.add_argument("--overwrite-date", action="store_true",
                        help=("date is ignored and is oveerwritten with "
                              "current date"))
    parser.add_argument("-m", "--rmap_version",
                        help=("RMAP version (Stimav3 0 / Stimav4 1)"
                              "(default: %(default)s)"),
                        type=int, default=0)

    args = parser.parse_args()

    main(
        host=args.host, keepalive=args.keepalive, port=args.port,
        topics=args.topic, username=args.username, password=args.pw,
        debug=args.debug, overwrite_date=args.overwrite_date,
        outfile=sys.stdout.buffer,
        readfromfile=args.readfromfile,fileinput=args.fileinput,
        roottopic=args.roottopic,fileinfo=args.fileinfo,version=args.rmap_version
    )
