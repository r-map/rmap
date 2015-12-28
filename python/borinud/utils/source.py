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


def get_db():
    return DB.get(BORINUD["SOURCES"],
                  cached_summary=BORINUD["CACHED_SUMMARY"])


class DB(object):
    """Abstract class.
    A concrete DB must implement the following methods

    - `query_stations`
    - `query_summary`
    - `query_data`
    """

    @classmethod
    def get(cls, urls, *args, **kw):
        """Factory method.

        The supported `url` are:

        - ``sqlite:FILENAME`` (DB-All.e).
        - ``odbc:DSN`` (DB-All.e).
        - ``http://...`` (Arkimet dataset).
        - A ``list`` of the previous

        Keyword arguments:

        - `cached_summary`: name of the cache for the cached summary
        """
        dbs = []

        for url in urls:
            if url.startswith("sqlite:") or url.startswith("odbc:"):
                db = DballeDB(url)
            elif url.startswith("http:"):
                db = ArkimetVm2DB(url)
            else:
                raise Exception("Invalid db url: " + url)

            dbs.append(db)

        db = MergeDB(dbs)

        if kw.get("cached_summary"):
            db = SummaryCacheDB(db, kw.get("cached_summary"))

        return db

    def query_stations(self, rec):
        """Query stations. Return a dballe.Record."""
        raise NotImplementedError()

    def query_summary(self, rec):
        """Query summary. Return a dballe.Record."""
        raise NotImplementedError()

    def query_data(self, rec):
        """Query data. Return a dballe.Record."""
        raise NotImplementedError()


class MergeDB(DB):
    """Container for DB."""
    def __init__(self, dbs):
        self.dbs = dbs

    def query_stations(self, rec):
        for db in self.dbs:
            for r in db.query_stations(rec):
                yield r.copy()

    def query_summary(self, rec):
        for db in self.dbs:
            for r in db.query_summary(rec):
                yield r.copy()

    def query_data(self, rec):
        for db in self.dbs:
            for r in db.query_data(rec):
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
        return db.query_data(rec)

    def query_summary(self, rec):
        db = self.__open_db()
        rec["query"] = "details"
        return db.query_summary(rec)

    def query_data(self, rec):
        db = self.__open_db()
        return db.query_data(rec)


class SummaryCacheDB(DB):
    def __init__(self, db, cachename, timeout=None):
        """Creates a summary cache for the database `db`

        The summary cache can be loaded in memory setting the parameter `ttl`
        (number of seconds the memory cache lives).
        """
        from django.core.cache import caches
        self.db = db
        self.cache = caches[cachename]
        self.timeout = timeout

    def set_cached_summary(self):
        from .codec import SummaryJSONEncoder

        res = self.db.query_summary(dballe.Record())
        summary = json.dumps(res, cls=SummaryJSONEncoder)
        self.cache.set('borinud-summary-cache', summary, self.timeout)
        return summary

    def get_cached_summary(self):
        """Get the cached summary."""
        from .codec import SummaryJSONDecoder

        summary = self.cache.get('borinud-summary-cache')

        if summary is None:
            summary = self.set_cached_summary()

        return json.loads(summary, cls=SummaryJSONDecoder)

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
            return all([
                rec.get(k) == item.get(k)
                for k in ["ident", "lon", "lat", "rep_memo", "trange", "level",
                          "var"]
                if k in rec
            ])
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
            yield dballe.Record(**{
                "lon": p["lon"],
                "lat": p["lat"],
                "rep_memo": str(p["network"]),
                "level": [p[k] for k in ["level_t1", "level_v1",
                                         "level_t2", "level_v2"]],
                "trange": [p[k] for k in ["trange_pind",
                                          "trange_p1", "trange_p2"]],
                "date": datetime.strptime(p["datetime"], "%Y-%m-%dT%H:%M:%SZ"),
                str(p["bcode"]): float(p["value"]),
            })

    def query_summary(self, rec):
        query = self.record_to_arkiquery(rec)
        url = "{}/summary?{}".format(self.dataset, "&".join([
            "{}={}".format(k, quote(v)) for k, v in {
                "style": "json",
                "query": query,
            }.iteritems()]))
        r = urlopen(url)
        for i in json.load(r)["items"]:
            yield dballe.Record(**{
                "ident": i["area"]["va"].get("ident"),
                "lon": i["area"]["va"]["lon"],
                "lat": i["area"]["va"]["lat"],
                "rep_memo": i["area"]["va"]["rep"],
                "var": i["product"]["va"]["bcode"],
                "level": [i["product"]["va"]["lt1"],
                          i["product"]["va"].get("l1"),
                          i["product"]["va"].get("lt2"),
                          i["product"]["va"].get("l2")],
                "trange": [i["product"]["va"]["tr"],
                           i["product"]["va"]["p1"],
                           i["product"]["va"]["p2"]],
                "datemin": datetime(*i["summarystats"]["b"]),
                "datemax": datetime(*i["summarystats"]["e"]),
            })

    def query_stations(self, rec):
        """Not yet implemented."""
        return DB.query_stations(self, rec)
