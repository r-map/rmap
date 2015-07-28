# borinud/ws.py - web services
#
# Copyright (C) 2013 ARPA-SIM <urpsim@smr.arpa.emr.it>
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
# Author: Emanuele Di Giacomo <edigiacomo@arpa.emr.it>
import bottle
import dballe
import json
from borinud import __version__
from borinud.db import DB

__apiversion__ = "0.1"


class SerializerPlugin(object):
    name = "borinud-serialize"
    api = 2

    def setup(self, app):
        app.uninstall('json')

    def togeojson(self, response):
        from borinud.codec import GeoJSONEncoder
        response = json.dumps(response, cls=GeoJSONEncoder)
        jsonp = bottle.request.query.get("callback")
        if jsonp:
            bottle.response.content_type = "application/javascript"
            response = jsonp + "(" + response + ")"
        else:
            bottle.response.content_type = "application/json"
        return response

    def apply(self, callback, context):
        def wrapper(*args, **kwargs):
            response = callback(*args, **kwargs)
            if any([
                isinstance(response, list),
                isinstance(response, tuple),
                isinstance(response, dballe.Cursor)
            ]):
                fmt = bottle.request.query.get("format", "geojson")
                if fmt == "geojson":
                    return self.togeojson(response)
                else:
                    raise Exception("Unsupported format {0}".format(fmt))
            else:
                return response
        return wrapper

app = bottle.Bottle(autojson=False)
app.install(SerializerPlugin())


@app.hook("after_request")
def enable_cors():
    bottle.response.headers[
        "Access-Control-Allow-Origin"
    ] = app.config.get("cors_site")


@app.get("")
@app.get("/")
def get_index():
    return get_static_file("index.html")


@app.get("/static/<filename:path>")
def get_static_file(filename):
    from pkg_resources import Requirement, resource_filename
    rootdir = resource_filename(Requirement.parse("borinud"), "borinud/_static")
    return bottle.static_file(filename, root=rootdir)


@app.get("/apiversion")
def get_api_version():
    bottle.response.content_type = "text/plain"
    return __apiversion__


# Return the summary
@app.get("/*/*/*/*/*/*/summaries")
@app.get("/*/*/<network>/*/*/*/summaries")
@app.get("/-/<lon:int>,<lat:int>/<network>/*/*/*/summaries")
@app.get("/<ident:re:[^*-]+>/*/<network>/*/*/*/summaries")
@app.get(("/*/*/<network>/"
          "<pind:int>,<p1:int>,<p2:int>/<lt1:int>,<l1>,<lt2>,<l2>/"
          "<bcode>/summaries/<year:int>/<month:int>"))
@app.get(("/*/*/<network>/"
          "<pind:int>,<p1:int>,<p2:int>/<lt1:int>,<l1>,<lt2>,<l2>/"
          "<bcode>/summaries/<year:int>/<month:int>/<day:int>"))
def get_summaries(ident=None, network=None, lon=None, lat=None,
                  pind=None, p1=None, p2=None,
                  lt1=None, l1=None, lt2=None, l2=None, bcode=None,
                  year=None, month=None, day=None):
    query = dballe.Record()
    query["ident"] = ident
    query["lon"] = lon
    query["lat"] = lat
    query["rep_memo"] = network
    query["trange"] = (pind, p1, p2)

    for n, v in (
        ("leveltype1", lt1),
        ("l1", l1),
        ("leveltype2", lt2),
        ("l2", l2),
    ):
        if v is not None:
            query.set_from_string("%s=%s" % (n, v))

    query["var"] = bcode
    query["year"] = year
    query["month"] = month
    query["day"] = day
    return list(app.config.db.query_summary(query))


# Timeseries
@app.get(("/<ident>/<lon:int>,<lat:int>/<network>/"
          "<pind:int>,<p1:int>,<p2:int>/<lt1:int>,<l1>,<lt2>,<l2>/"
          "<bcode>/timeseries/<year:int>"))
@app.get(("/<ident>/<lon:int>,<lat:int>/<network>/"
          "<pind:int>,<p1:int>,<p2:int>/<lt1:int>,<l1>,<lt2>,<l2>/"
          "<bcode>/timeseries/<year:int>/<month:int>"))
@app.get(("/<ident>/<lon:int>,<lat:int>/<network>/<pind:int>,<p1:int>,<p2:int>/"
          "<lt1:int>,<l1>,<lt2>,<l2>/<bcode>/"
          "timeseries/<year:int>/<month:int>/<day:int>"))
@app.get(("/<ident>/*/<network>/"
          "<pind:int>,<p1:int>,<p2:int>/<lt1:int>,<l1>,<lt2>,<l2>/"
          "<bcode>/timeseries/<year:int>"))
@app.get(("/<ident>/*/<network>/"
          "<pind:int>,<p1:int>,<p2:int>/<lt1:int>,<l1>,<lt2>,<l2>/"
          "<bcode>/timeseries/<year:int>/<month:int>"))
@app.get(("/<ident>/*/<network>/<pind:int>,<p1:int>,<p2:int>/"
          "<lt1:int>,<l1>,<lt2>,<l2>/<bcode>/"
          "timeseries/<year:int>/<month:int>/<day:int>"))
def get_resource_timeseries(ident,
                            network,
                            pind, p1, p2,
                            lt1, l1, lt2, l2,
                            bcode,
                            year, lon=None, lat=None, month=None, day=None):
    query = dballe.Record()
    if ident != "-":
        query["ident"] = ident
    query["lon"] = lon
    query["lat"] = lat
    query["rep_memo"] = network
    query["trange"] = (pind, p1, p2)
    query.set_from_string("leveltype1=%s" % (lt1,))
    query.set_from_string("l1=%s" % (l1,))
    query.set_from_string("leveltype2=%s" % (lt2,))
    query.set_from_string("l2=%s" % (l2,))
    query["var"] = bcode
    query["year"] = year
    query["month"] = month
    query["day"] = day

    return list(app.config.db.query_data(query))


# Spatialseries
@app.get(("/*/*/<network>/<pind:int>,<p1:int>,<p2:int>/"
          "<lt1:int>,<l1>,<lt2>,<l2>/<bcode>/"
          "spatialseries/<year:int>/<month:int>/<day:int>/<hour:int>"))
def get_network_spatialseries(network,
                              pind, p1, p2,
                              lt1, l1, lt2, l2,
                              bcode,
                              year, month, day, hour):
    from datetime import datetime, timedelta
    query = dballe.Record()
    query["rep_memo"] = network
    query["trange"] = (pind, p1, p2)
    query.set_from_string("leveltype1=%s" % (lt1,))
    query.set_from_string("l1=%s" % (l1,))
    query.set_from_string("leveltype2=%s" % (lt2,))
    query.set_from_string("l2=%s" % (l2,))
    query["var"] = bcode
    d = datetime(year, month, day, hour)
    query["datemin"] = d - timedelta(seconds=1800)
    query["datemax"] = d + timedelta(seconds=1799)

    return list(app.config.db.query_data(query))


# Station data
@app.get("/-/<lon:int>,<lat:int>/<network>/-,-,-/257,-,-,-/*/stationdata")
@app.get("/<ident>/*/<network>/-,-,-/257,-,-,-/*/stationdata")
def get_station_data(network, ident=None, lon=None, lat=None):
    query = dballe.Record()
    query.set_station_context()
    query["rep_memo"] = network
    query["ident"] = ident
    query["lon"] = lon
    query["lat"] = lat

    return list(app.config.db.query_stations(query))


# Get local B table
@app.get("/descriptions/btable")
def get_btable():
    vartable = dballe.Vartable.get("dballe")

    return dict((b.var, {
        "description": b.desc,
        "unit": b.unit
    }) for b in vartable)

versionedapp = bottle.Bottle()


@versionedapp.get("/apiversion")
def get_versioned_api_version():
    bottle.response.content_type = "text/plain"
    return __apiversion__

versionedapp.mount(__apiversion__, app)

if __name__ == '__main__':
    from argparse import ArgumentParser
    parser = ArgumentParser(description="ObsWs Web Server")
    parser.add_argument("--version",
                        action="version",
                        version="%(prog)s " + __version__)
    parser.add_argument("--apiversion",
                        action="version",
                        help="show web api version number and exit",
                        version="%(prog)s WS API " + __apiversion__)
    parser.add_argument("--dsn", metavar="URL", action="append",
                        help="url for dballe db",
                        required=True)
    parser.add_argument("--host", metavar="HOST",
                        help="host name",
                        default="localhost")
    parser.add_argument("--port", metavar="PORT",
                        help="port",
                        default="8080")
    parser.add_argument("--server", metavar="SERVER",
                        help="server name",
                        default='wsgiref')
    parser.add_argument("--allow-cors", metavar="SITE",
                        help="allow CORS requests from site")
    parser.add_argument("--versioned", action="store_true",
                        help="Use versioned API",
                        default=False)
    parser.add_argument("--cached-summary", metavar="FILE",
                        help="cached summary")
    parser.add_argument("--debug-mode", action="store_true",
                        help="debug mode")

    opts = parser.parse_args()

    app.config.db = DB.get(
        opts.dsn,
        cached_summary=opts.cached_summary
    )
    app.config.cors_site = opts.allow_cors

    if opts.versioned:
        apptorun = versionedapp
    else:
        apptorun = app

    bottle.run(app=apptorun,
               host=opts.host,
               port=opts.port,
               server=opts.server,
               debug=opts.debug_mode,
               reloader=opts.debug_mode)
