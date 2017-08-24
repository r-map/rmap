# borinud/source.py - source utilities.
#
# Copyright (C) 2013-2015 ARPA-SIM <urpsim@smr.arpa.emr.it>
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

import math
import dballe
import json
from datetime import datetime

try:
    from urllib import quote
    from urllib2 import urlopen
except ImportError:
    from urllib.request import urlopen
    from urllib.parse import quote

from ..settings import BORINUD


def get_db(dsn="report"):
    from django.utils.module_loading import import_string
    dbs = [
        import_string(i["class"])(**{
            k: v for k, v in i.items() if k != "class"
        })
        for i in BORINUD[dsn]["SOURCES"]
    ]
    if len(dbs) == 1:
        db = dbs[0]
    else:
        db = MergeDB(dbs)

    if BORINUD[dsn]["CACHED_SUMMARY"]:
        db = SummaryCacheDB(
            db, BORINUD[dsn]["CACHED_SUMMARY"],
            BORINUD[dsn]["CACHED_SUMMARY_TIMEOUT"],dsn,
        )

    return db


class DB(object):
    """Abstract class.
    A concrete DB must implement the following methods

    - `query_stations`
    - `query_summary`
    - `query_data`
    """
    def query_stations(self, rec):
        """Query stations. Return a dballe.Record."""
        raise NotImplementedError()

    def query_summary(self, rec):
        """Query summary. Return a dballe.Record."""
        raise NotImplementedError()

    def query_data(self, rec):
        """Query data. Return a dballe.Record."""
        raise NotImplementedError()

    def fill_db(self, memdb):
        """Query data and fill a memdb."""
        raise NotImplementedError()


class MergeDB(DB):
    """Container for DB."""
    def __init__(self, dbs):
        self.dbs = dbs

    def unique_record_key(self, rec):
        return tuple(map(rec.get, (
            "ident", "lon", "lat", "rep_memo", "var", "level", "trange",
        )))

    def get_unique_records(self, funcname, rec, reducer):
        from itertools import groupby
        for k, g in groupby(sorted([
            r.copy() for db in self.dbs for r in getattr(db, funcname)(rec)
        ], key=self.unique_record_key), self.unique_record_key):
            yield reducer(g)

    def query_stations(self, rec):
        for r in self.get_unique_records(
            "query_stations", rec, lambda g: g.next()
        ):
            yield r.copy()

    def query_summary(self, rec):
        def reducer(g):
            rec = g.next()
            for r in g:
                if r["datemin"] < rec["datemin"]:
                    rec["datemin"] = r["datemin"]
                if r["datemax"] > rec["datemax"]:
                    rec["datemax"] = r["datemax"]

            return rec

        for r in self.get_unique_records(
            "query_summary", rec, reducer
        ):
            yield r.copy()

    def query_data(self, rec):
        memdb = dballe.DB.connect_from_url("mem:")
        for db in self.dbs:
            db.fill_db(rec,memdb)

        for r in memdb.query_data(rec):
            yield r.copy()


class DballeDB(DB):
    """DB-All.e database."""
    def __init__(self, url):
        """Create a DB-All.e database from `url` DSN."""
        self.url = url

    def __open_db(self):
        """Open the database."""
        return dballe.DB.connect_from_url(self.url)

    def query_stations(self, rec):
        db = self.__open_db()
        rec.set_station_context()
        return db.query_station_data(rec)

    def query_summary(self, rec):
        db = self.__open_db()
        rec["query"] = "details"
        return db.query_summary(rec)

    def query_data(self, rec):
        db = self.__open_db()
        return db.query_data(rec)

    def fill_db(self, rec, memdb):
        for r in self.query_data(rec):
            del r["ana_id"]
            del r["data_id"]
            memdb.insert_data(r, True, True)

class SummaryCacheDB(DB):
    def __init__(self, db, cachename, timeout=None,dsn="report"):
        """Creates a summary cache for the database `db`

        The summary cache can be loaded in memory setting the parameter `ttl`
        (number of seconds the memory cache lives).
        """
        from django.core.cache import caches
        self.db = db
        self.cache = caches[cachename]
        self.timeout = timeout
        self.dsn=dsn

    def set_cached_summary(self):
        res = self.db.query_summary(dballe.Record())
        summary = [{
            "ident": o.get("ident"),
            "lon": o.key("lon").enqi(),
            "lat": o.key("lat").enqi(),
            "rep_memo": o.get("rep_memo"),
            "level": o.get("level"),
            "trange": o.get("trange"),
            "bcode": o.get("var"),
            "date": o.date_extremes(),
        } for o in res]
        self.cache.set('borinud-summary-cache-%s' % self.dsn, summary, self.timeout)
        return summary

    def get_cached_summary(self):
        """Get the cached summary."""
        summary = self.cache.get('borinud-summary-cache-%s' % self.dsn)

        if summary is None:
            summary = self.set_cached_summary()

        return tuple(dballe.Record(**{
            "ident": None if i["ident"] is None else i["ident"],
            "lon": i["lon"],
            "lat": i["lat"],
            "rep_memo": i["rep_memo"],
            "level": tuple(i["level"]),
            "trange": tuple(i["trange"]),
            "var": i["bcode"],
            "datemin": i["date"][0],
            "datemax": i["date"][1],
        }) for i in summary)

    def get_filter_summary(self, rec):
        """Return a filter function based on dballe.Record `rec`.

        The following keys are considered

        - ident
        - lon
        - lat
        - rep_memo
        - trange
        - level
        - var
        """
        def wrapper(item):
            f = [
                rec.get(k) == item.get(k)
                for k in ["ident", "lon", "lat", "rep_memo", "trange", "level",
                          "var"]
                if k in rec
            ]

            if rec.get("datemin"):
                f.append(any([
                    rec.get("datemin") <= item.get("datemax"),
                    item.get("datemax") is None,
                ]))

            if rec.get("datemax"):
                f.append(any([
                    rec.get("datemax") >= item.get("datemin"),
                    item.get("datemin") is None,
                ]))

            return(all(f))

        return wrapper

    def query_stations(self, rec):
        return self.db.query_stations(rec)

    def query_summary(self, rec):
        return filter(
            self.get_filter_summary(rec),
            self.get_cached_summary()
        )

    def query_data(self, rec):
            return self.db.query_data(rec)


    def fill_db(self, rec, memdb):
        for r in self.db.query_data(rec):
            del r["ana_id"]
            del r["data_id"]
            memdb.insert_data(r, True, True)


class ArkimetVm2DB(DB):
    """Arkimet dataset containing ``VM2`` data."""
    def __init__(self, dataset):
        self.dataset = dataset

    def record_to_arkiquery(self, rec):
        """Translate a dballe.Record to arkimet query."""
        # TODO: less verbose implementation
        q = {
            "reftime": [],
            "area": {},
            "product": {},
        }

        d1, d2 = rec.date_extremes()
        if d1:
            q["reftime"].append(">={}".format(d1))

        if d2:
            q["reftime"].append("<={}".format(d2))

        for k in ["lon", "lat"]:
            if k in rec:
                q["area"][k] = int(rec[k] * 10**5)

        if "rep_memo" in rec:
            q["area"]["rep"] = rec["rep_memo"]

        if "var" in rec:
            q["product"]["bcode"] = rec["var"]

        if "leveltype1" in rec:
            q["product"]["lt1"] = rec["leveltype1"]

        if "l1" in rec:
            q["product"]["l1"] = rec["l1"]

        if "leveltype2" in rec:
            q["product"]["lt2"] = rec["leveltype2"]

        if "l2" in rec:
            q["product"]["l2"] = rec["l2"]

        if "pindicator" in rec:
            q["product"]["tr"] = rec["pindicator"]

        if "p1" in rec:
            q["product"]["p1"] = rec["p1"]

        if "p2" in rec:
            q["product"]["p2"] = rec["p2"]

        q["reftime"] = ",".join(q["reftime"])
        q["area"] = "VM2:{}".format(",".join([
            "{}={}".format(k, v) for k, v in q["area"].iteritems()
        ]))
        q["product"] = "VM2:{}".format(",".join([
            "{}={}".format(k, v) for k, v in q["product"].iteritems()
        ]))

        arkiquery = ";".join("{}:{}".format(k, v) for k, v in q.iteritems())

        return arkiquery

    def query_data(self, rec):
        query = self.record_to_arkiquery(rec)
        url = "{}/query?{}".format(self.dataset, "&".join([
            "{}={}".format(k, quote(v)) for k, v in {
                "style": "postprocess",
                "command": "json",
                "query": query,
            }.iteritems()]))
        r = urlopen(url)
        for f in json.load(r)["features"]:
            p = f["properties"]
            r = dballe.Record(**{
                "lon": p["lon"],
                "lat": p["lat"],
                "rep_memo": str(p["network"]),
                "level": tuple(p[k] for k in ["level_t1", "level_v1",
                                              "level_t2", "level_v2"]),
                "trange": tuple(p[k] for k in ["trange_pind",
                                               "trange_p1", "trange_p2"]),
                "date": datetime.strptime(p["datetime"], "%Y-%m-%dT%H:%M:%SZ"),
                str(p["bcode"]): float(p["value"]),
            })
            yield r

    def fill_db(self,rec,memedb):
        for r in self.query_data(rec):
            memdb.insert_data(r, True, True)

    def query_summary(self, rec):
        query = self.record_to_arkiquery(rec)
        url = "{}/summary?{}".format(self.dataset, "&".join([
            "{}={}".format(k, quote(v)) for k, v in {
                "style": "json",
                "query": query,
            }.iteritems()]))
        r = urlopen(url)
        for i in json.load(r)["items"]:
            if not "va" in  i["area"] or not "va" in i["product"]:
                continue
            yield dballe.Record(**{
                "ident": i["area"]["va"].get("ident"),
                "lon": i["area"]["va"]["lon"],
                "lat": i["area"]["va"]["lat"],
                "rep_memo": i["area"]["va"]["rep"],
                "var": i["product"]["va"]["bcode"],
                "level": (i["product"]["va"]["lt1"],
                          i["product"]["va"].get("l1"),
                          i["product"]["va"].get("lt2"),
                          i["product"]["va"].get("l2")),
                "trange": (i["product"]["va"]["tr"],
                           i["product"]["va"]["p1"],
                           i["product"]["va"]["p2"]),
                "datemin": datetime(*i["summarystats"]["b"]),
                "datemax": datetime(*i["summarystats"]["e"]),
            })

    def query_stations(self, rec):
        """Not yet implemented."""
        return DB.query_stations(self, rec)


class ArkimetBufrDB(DB):
    """Arkimet dataset containing generic ``BUFR`` data."""
    def __init__(self, dataset, measurements):
        """
        Create a DB from an `HTTP` Arkimet `dataset` containing generic BUFR
        data.

        :param dataset: `URL` of Arkimet dataset
        :param measurements: array of dict with `var`, `level` and `trange`

        Example::

            ArkimetBufrDB(
                "http://localhost:8090/dataset/rmap", [{
                    "var": "B13011",
                    "level": (1, None, None, None),
                    "trange": (0, 0, 3600),
                }, {
                    "var": "B12101",
                    "level": (103, 2000, None, None),
                }],
            )
        """
        self.dataset = dataset
        self.measurements = measurements

    def query_stations(self, rec):
        """Query stations.

        .. warning::

            Only `ident`, `rep_memo`, `lon` and `lat` are returned.
            Loading static data must be implemented.
        """
        dates = set(r["datemax"] for r in self.query_summary(dballe.Record()))
        db = dballe.DB.connect_from_url("mem:")
        for d in dates:
            self.load_arkiquery_to_dbadb(dballe.Record(date=d), db)

        for s in db.query_station_data(rec):
            yield s

    def query_summary(self, rec):
        """Query summary.

        .. warning::

            Every station is supposed to measure all the `self.measurements`
        """
        query = self.record_to_arkiquery(rec)
        url = "{}/summary?{}".format(self.dataset, "&".join([
            "{}={}".format(k, quote(v)) for k, v in {
                "style": "json",
                "query": query,
            }.iteritems()]))
        r = urlopen(url)
        for i in json.load(r)["items"]:
            for m in self.measurements:
                if all([
                        rec.get(k) == i.get(k)
                        for k in ["var", "level", "trange"]
                        if k in rec
                ]):
                    if "lon" in i["area"]["va"]:
                        lon=i["area"]["va"]["lon"]  # fixed station
                    else:
                        lon=i["area"]["va"]["x"]    # mobile

                    if "lat" in i["area"]["va"]:
                        lat=i["area"]["va"]["lat"]  # fixed station
                    else:
                        lat=i["area"]["va"]["y"]    # mobile

                    yield dballe.Record(**{
                        "var": m["var"],
                        "level": m["level"],
                        "trange": m["trange"],
                        "ident": i.get("proddef", {}).get("va", {}).get("id", None),
                        "lon": lon,
                        "lat": lat,
                        "rep_memo": i["product"]["va"]["t"],
                        "datemin": datetime(*i["summarystats"]["b"]),
                        "datemax": datetime(*i["summarystats"]["e"]),
                    })

    #def query_data(self, rec):
    #    db = dballe.DB.connect_from_url("mem:")
    #    self.load_arkiquery_to_dbadb(rec, db)
    #    for r in db.query_data(rec):
    #        yield r.copy()


    def get_datastream(self, rec):

        query = self.record_to_arkiquery(rec)
        url = "{}/query?{}".format(self.dataset, "&".join([
            "{}={}".format(k, quote(v)) for k, v in {
                "style": "postprocess",
                "command": "bufr-filter "+" ".join([
                    "{}={}".format(kk, rec.get(kk,"-")) for kk in ["leveltype1", "l1",
                                                          "leveltype2", "l2",
                                                          "pindicator", "p1", "p2",
                                                          "var"]]),
                "query": query,
            }.iteritems()]))

        return urlopen(url)


    def query_data(self, rec):

        fo=self.get_datastream(rec)
        memdb = dballe.DB.connect_from_url("mem:")
        memdb.load(fo, "BUFR")
        for r in memdb.query_data(rec):
            del r["ana_id"]
            del r["data_id"]
            yield r.copy()


    def fill_db(self, rec,memdb):

        fo=self.get_datastream(rec)
        memdb.load(fo, "BUFR")


    def load_arkiquery_to_dbadb(self, rec, db):
        query = self.record_to_arkiquery(rec)
        url = "{}/query?{}".format(self.dataset, "&".join([
            "{}={}".format(k, quote(v)) for k, v in {
                "style": "data",
                "query": query,
            }.iteritems()]))
        r = urlopen(url)
        db.load(r, "BUFR")

    def record_to_arkiquery(self, rec):
        """Translate a dballe.Record to arkimet query."""
        # TODO: less verbose implementation
        q = {
            "reftime": [],
            "area": {
                "fixed": {},
                "mobile": {
                    "type": "mob",
                },
            }
        }

        d1, d2 = rec.date_extremes()
        if d1:
            q["reftime"].append(">={}".format(d1))

        if d2:
            q["reftime"].append("<={}".format(d2))

        for k in ["lon", "lat"]:
            if k in rec:
                q["area"]["fixed"][k] = int(rec[k] * 10**5)
                q["area"]["mobile"][{"lon": "x", "lat": "y"}[k]] = math.floor(rec[k])

        if "rep_memo" in rec:
            q["product"] = "BUFR:t={}".format(rec["rep_memo"])

        if "ident" in rec:
            q["proddef"] = "GRIB:id={}".format(rec["ident"])

        q["reftime"] = ",".join(q["reftime"])

        q["area"] = "GRIB:{}".format(",".join([
            "{}={}".format(k, v) for k, v in q["area"]["fixed"].iteritems()
        ])) + " or GRIB:{}".format(",".join([
            "{}={}".format(k, v) for k, v in q["area"]["mobile"].iteritems()
        ]))

        if "lonmin" in rec and "latmin" in rec and "lonmax" in rec and "latmax" in rec:
            q["area"] +="; area:bbox coveredby POLYGON(({} {},{} {},{} {},{} {}))".format(
                rec["lonmin"],rec["latmin"],rec["lonmin"],rec["latmax"],rec["lonmax"],rec["latmax"],rec["lonmin"],rec["latmin"]
            )

        arkiquery = ";".join("{}:{}".format(k, v) for k, v in q.iteritems())
        return arkiquery
