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
import tempfile
import codecs
from itertools import groupby
import dateutil.parser
try:
    from urllib.parse import quote
    from urllib.request import urlopen
except ImportError:
    from urllib.request import urlopen
    from urllib.parse import quote

from ..settings import BORINUD,BORINUDLAST


def get_db(dsn="report",last=True):
    from django.utils.module_loading import import_string
    dbs = [
        import_string(i["class"])(**{
            k: v for k, v in list(i.items()) if k != "class"
        })
        for i in (BORINUDLAST[dsn]["SOURCES"] if last else BORINUD[dsn]["SOURCES"])
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
        """Query stations. Return a record."""
        raise NotImplementedError()

    def query_summary(self, rec):
        """Query summary. Return a record."""
        raise NotImplementedError()

    def query_data(self, rec):
        """Query data. Return a record."""
        raise NotImplementedError()

    def query_stations_data(self, rec):
        """Query stations. Return a record."""
        raise NotImplementedError()
    
    def fill_data_db(self, memdb):
        """Query data and fill a memdb."""
        raise NotImplementedError()

    def fill_station_data_db(self, memdb):
        """Query stations data and fill a memdb."""
        raise NotImplementedError()


class MergeDBfake(DB):
    """Container for DB."""

    def __init__(self, dbs):
        self.dbs=dbs

    def __open_db(self,rec):
        """Open the database."""
        memdb = dballe.DB.connect("mem:")
        for db in self.dbs:
            db.fill_data_db(rec,memdb)
            db.fill_station_data_db(rec,memdb)
        return memdb

    def query_stations(self, rec):
        db = self.__open_db(rec)
        return db.query_stations(rec)

    def query_summary(self, rec):
        db = self.__open_db(rec)
        rec["query"] = "details"
        return db.query_summary(rec)

    def query_data(self, rec):
        db = self.__open_db(rec)
        return db.query_data(rec)

    def query_station_data(self, rec):
        db = self.__open_db(rec)
        return db.query_station_data(rec)
    
    def fill_data_db(self, rec, memdb):
        for r in self.query_data(rec):
            #TODO del r["ana_id"]
            #TODO del r["data_id"]
            memdb.insert_data(r, True, True)

    def fill_station_data_db(self, rec, memdb):
        for r in self.query_station_data(rec):
            #TODO del r["ana_id"]
            #TODO del r["data_id"]
            memdb.insert_station_data(r, True, True)
            

class MergeDB(DB):
    """Container for DB."""
    def __init__(self, dbs):
        self.dbs = dbs

    def unique_record_key(self, rec):
        """Create a string from a record, based on ident, lon, lat, report,
        pindicator, level and var values. Null values are encoded as "-"."""
        def if_null(value, default="-"):
            return value if value is not None else default

        return (
            "{}/"
            "{},{}/"
            "{}/"
            "{},{},{}/"
            "{},{},{},{}/"
            "{}"
        ).format(*map(if_null, (
            rec.get("ident",None),
            rec["lon"],
            rec["lat"],
            rec["report"],
            rec["pindicator"],
            rec["p1"],
            rec["p2"],
            rec["leveltype1"],
            rec["l1"],
            rec["leveltype2"],
            rec["l2"],
            rec["var"],
        )))


    def unique_record_station_key(self, rec):
        """Create a string from a record, based on ident, lon, lat, report,
        pindicator, level and var values. Null values are encoded as "-"."""
        def if_null(value, default="-"):
            return value if value is not None else default

        return (
            "{}/"
            "{},{}/"
            "{}/"
        ).format(*map(if_null, (
            rec.get("ident",None),
            rec["lon"],
            rec["lat"],
            rec["report"]
        )))

    
    def get_unique_records(self, funcname, rec, reducer):
        for k, g in groupby(sorted([
            r for db in self.dbs for r in getattr(db, funcname)(rec)
        ], key=self.unique_record_key), self.unique_record_key):
            yield reducer(g)

    def get_unique_station_records(self, funcname, rec, reducer):
        for k, g in groupby(sorted([
            r for db in self.dbs for r in getattr(db, funcname)(rec)
        ], key=self.unique_record_station_key), self.unique_record_station_key):
            yield reducer(g)
            
    def query_stations(self, rec):
        for r in self.get_unique_station_records(
            "query_stations", rec, lambda g: next(g)
        ):
            yield r

    def query_summary(self, rec):
        def reducer(g):
            rec = next(g)
            for r in g:
                if r["datemin"] < rec["datemin"]:
                    rec["datemin"] = r["datemin"]
                if r["datemax"] > rec["datemax"]:
                    rec["datemax"] = r["datemax"]

            return rec

        for r in self.get_unique_records(
            "query_summary", rec, reducer
        ):
            yield r

    def query_data(self, rec):
        memdb = dballe.DB.connect_from_url("mem:")
        for db in self.dbs:
            db.fill_data_db(rec,memdb)

        with memdb.transaction() as tr:
            for cur in tr.query_data(rec):
                data={}
                data["ident"]=cur["ident"]
                data["report"]=cur["report"]
                data["lat"]=cur.enqi("lat")
                data["lon"]=cur.enqi("lon")
                data["leveltype1"]= cur["leveltype1"]
                data["l1"]=cur["l1"]
                data["leveltype2"]=cur["leveltype2"]
                data["l2"]=cur["l2"]
                data["pindicator"]=cur["pindicator"]
                data["p1"]=cur["p1"]
                data["p2"]=cur["p2"]
                data["var"]=cur["var"]
                data[cur["var"]]=cur[cur["var"]].get()
                data["date"]=datetime(cur["year"], cur["month"], cur["day"], cur["hour"], cur["min"], cur["sec"])
                #print ("merge query data: ",data)
                yield data

    def query_station_data(self, rec):
        # TODO si devono rendere univoci i risultati raggruppando per stazione,
        # i.e. per ident,lon,lat,report.
        memdb = dballe.DB.connect("mem:")
        with memdb.transaction() as tr:
            for db in self.dbs:
                for r in db.query_station_data(rec):
                    del r["var"]
                    tr.insert_station_data(r, True, True)

        with memdb.transaction() as tr:
            for r in db.query_station_data(rec):
                yield r


class DballeDB(DB):
    """DB-All.e database."""
    def __init__(self, url):
        """Create a DB-All.e database from `url` DSN."""
        self.url = url

    def __open_db(self):
        """Open the database."""
        return dballe.DB.connect(self.url)

    def query_stations(self, rec):
        db = self.__open_db()
        #return db.query_stations(rec)

        with db.transaction() as tr:
            for cur in tr.query_stations(rec):
                data={}
                data["ident"]=cur["ident"]
                data["report"]=cur["report"]
                data["lat"]=cur.enqi("lat")
                data["lon"]=cur.enqi("lon")
                #print ("dballe query station: ",data)
                yield data
    
    def query_summary(self, rec):
        db = self.__open_db()
        rec["query"] = "details"

        with db.transaction() as tr:
            for cur in tr.query_summary(rec):
                data={}
                data["ident"]=cur["ident"]
                data["report"]=cur["report"]
                data["lat"]=cur.enqi("lat")
                data["lon"]=cur.enqi("lon")
                data["datemin"]=cur["datetimemin"]
                data["datemax"]=cur["datetimemax"]
                data["leveltype1"]= cur["leveltype1"]
                data["l1"]=cur["l1"]
                data["leveltype2"]=cur["leveltype2"]
                data["l2"]=cur["l2"]
                data["pindicator"]=cur["pindicator"]
                data["p1"]=cur["p1"]
                data["p2"]=cur["p2"]
                data["var"]=cur["var"]
                #print ("dballe query summary: ",data)
                yield data
    
    def query_data(self, rec):
        db = self.__open_db()

        with db.transaction() as tr:
            for cur in tr.query_data(rec):
                data={}
                data["ident"]=cur["ident"]
                data["report"]=cur["report"]
                data["lat"]=cur.enqi("lat")
                data["lon"]=cur.enqi("lon")
                data["leveltype1"]= cur["leveltype1"]
                data["l1"]=cur["l1"]
                data["leveltype2"]=cur["leveltype2"]
                data["l2"]=cur["l2"]
                data["pindicator"]=cur["pindicator"]
                data["p1"]=cur["p1"]
                data["p2"]=cur["p2"]
                data["var"]=cur["var"]
                data[cur["var"]]=cur[cur["var"]].get()
                data["date"]=datetime(cur["year"], cur["month"], cur["day"], cur["hour"], cur["min"], cur["sec"])
                #print ("dballe query data: ",data)
                yield data

    def query_station_data(self, rec):
        db = self.__open_db()

        with db.transaction() as tr:
            for cur in tr.query_station_data(rec):
                data={}
                data["ident"]=cur["ident"]
                data["report"]=cur["report"]
                data["lat"]=cur.enqi("lat")
                data["lon"]=cur.enqi("lon")
                data["var"]=cur["var"]
                data[cur["var"]]=cur[cur["var"]].get()
                #print ("dballe query station data: ",data)
                yield data

    def fill_data_db(self, rec, memdb):

        db = self.__open_db()

        with db.transaction() as tr:
            for cur in tr.query_data(rec):
                memdb.insert_data(cur.data, True, True)

    def fill_station_data_db(self, rec, memdb):

        db = self.__open_db()

        with db.transaction() as tr:
            for cur in tr.query_station_data(rec):
                memdb.insert_station_data(cur.data, True, True)


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
        
        res = self.db.query_summary({})
         
        summary = [{
            "ident":      o["ident"],
            "lon":        o["lon"],
            "lat":        o["lat"],
            "report":     o["report"],
            "leveltype1": o["leveltype1"],
            "l1":         o["l1"],
            "leveltype2": o["leveltype2"],
            "l2":         o["l2"],
            "pindicator": o["pindicator"],
            "p1":         o["p1"],
            "p2":         o["p2"],
            "var":        o["var"],
            "datemin":    o["datemin"].isoformat(),
            "datemax":    o["datemax"].isoformat()
        } for o in res]
        self.cache.set('borinud-summary-cache-%s' % self.dsn, summary, self.timeout)
        return summary

    def get_cached_summary(self):
        """Get the cached summary."""
        summary = self.cache.get('borinud-summary-cache-%s' % self.dsn)

        if summary is None:
            summary = self.set_cached_summary()

        return tuple({**{
            "ident":    None if i["ident"] is None else i["ident"],
            "lon":        i["lon"],
            "lat":        i["lat"],
            "report":     i["report"],
            "leveltype1": i["leveltype1"],
            "l1":         i["l1"],
            "leveltype2": i["leveltype2"],
            "l2":         i["l2"],
            "pindicator": i["pindicator"],
            "p1":         i["p1"],
            "p2":         i["p2"],
            "var":        i["var"],
            "datemin":    dateutil.parser.parse(i["datemin"]),
            "datemax":    dateutil.parser.parse(i["datemax"])
        }} for i in summary)

    def get_filter_summary(self, rec):
        """Return a filter function based on dballe.Record `rec`.

        The following keys are considered

        - ident
        - lon
        - lat
        - report
        - pindicator.p1,p2
        - leveltype1,l1
        - leveltype2,l2
        - var
        """
        def wrapper(item):
            f = [
                rec.get(k) == item.get(k)

                for k in ["ident", "lon", "lat", "report",
                          'pindicator','p1','p2','leveltype1','l1','leveltype2','l2',
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
        return list(filter(
            self.get_filter_summary(rec),
            self.get_cached_summary()
        ))
        
    def query_data(self, rec):
            return self.db.query_data(rec)

    def query_station_data(self, rec):
            return self.db.query_station_data(rec)        

    def fill_data_db(self, rec, memdb):
        for r in self.db.query_data(rec):
            #TODO del r["ana_id"]
            #TODO del r["data_id"]
            memdb.insert_data(r, True, True)

    def fill_station_data_db(self, rec, memdb):
        for r in self.db.query_station_data(rec):
            #TODO del r["ana_id"]
            #TODO del r["data_id"]
            memdb.insert_station_data(r, True, True)

            

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

        if "report" in rec:
            q["area"]["rep"] = rec["report"]

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
            "{}={}".format(k, v) for k, v in q["area"].items()
        ]))
        q["product"] = "VM2:{}".format(",".join([
            "{}={}".format(k, v) for k, v in q["product"].items()
        ]))

        arkiquery = ";".join("{}:{}".format(k, v) for k, v in q.items())

        return arkiquery

    def query_data(self, rec):
        query = self.record_to_arkiquery(rec)
        url = "{}/query?{}".format(self.dataset, "&".join([
            "{}={}".format(k, quote(v)) for k, v in {
                "style": "postprocess",
                "command": "json",
                "query": query,
            }.items()]))
        r = urlopen(url)
        for f in json.load(r)["features"]:
            p = f["properties"]
            r = {**{
                "lon": p["lon"],
                "lat": p["lat"],
                "report": str(p["network"]),
                "level": tuple(p[k] for k in ["level_t1", "level_v1",
                                              "level_t2", "level_v2"]),
                "trange": tuple(p[k] for k in ["trange_pind",
                                               "trange_p1", "trange_p2"]),
                "date": datetime.strptime(p["datetime"], "%Y-%m-%dT%H:%M:%SZ"),
                str(p["bcode"]): float(p["value"]),
            }}
            yield r

#TO BE DONE!
#TO BE DONE!            
    def query_station_data(self, rec):
        query = self.record_to_arkiquery(rec)
        url = "{}/query?{}".format(self.dataset, "&".join([
            "{}={}".format(k, quote(v)) for k, v in {
                "style": "postprocess",
                "command": "json",
                "query": query,
            }.items()]))
        r = urlopen(url)
        for f in json.load(r)["features"]:
            p = f["properties"]
            r = {**{
                "lon": p["lon"],
                "lat": p["lat"],
                "report": str(p["network"]),
                "level": tuple(p[k] for k in ["level_t1", "level_v1",
                                              "level_t2", "level_v2"]),
                "trange": tuple(p[k] for k in ["trange_pind",
                                               "trange_p1", "trange_p2"]),
                "date": datetime.strptime(p["datetime"], "%Y-%m-%dT%H:%M:%SZ"),
                str(p["bcode"]): float(p["value"]),
            }}
            yield r
            
    def fill_data_db(self,rec,memedb):
        for r in self.query_data(rec):
            memdb.insert_data(r, True, True)

    def fill_station_data_db(self,rec,memedb):
        for r in self.query_station_data(rec):
            memdb.insert_station_data(r, True, True)
            
    def query_summary(self, rec):
        query = self.record_to_arkiquery(rec)
        url = "{}/summary?{}".format(self.dataset, "&".join([
            "{}={}".format(k, quote(v)) for k, v in {
                "style": "json",
                "query": query,
            }.items()]))
        r = urlopen(url)
        for i in json.load(r)["items"]:
            if not "va" in  i["area"] or not "va" in i["product"]:
                continue
            yield {**{
                "ident": i["area"]["va"].get("ident"),
                "lon": i["area"]["va"]["lon"],
                "lat": i["area"]["va"]["lat"],
                "report": i["area"]["va"]["rep"],
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
            }}

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

            Only `ident`, `report`, `lon` and `lat` are returned.
            Loading static data must be implemented.
        """
        dates = set(r["datemax"] for r in self.query_summary({}))
        db = dballe.DB.connect_from_url("mem:")
        for d in dates:
            self.load_arkiquery_to_dbadb({"datetime":d}, db)

        with db.transaction() as tr:
            for cur in tr.query_stations(rec):
                data={}
                data["ident"]=cur["ident"]
                data["report"]=cur["report"]
                data["lat"]=cur.enqi("lat")
                data["lon"]=cur.enqi("lon")
                yield data

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
            }.items()]))

        reader = codecs.getreader("utf-8")
        r = reader(urlopen(url))
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

                    yield {**{
                        "var": m["var"],
                        "leveltype1": m["level"][0],
                        "l1": m["level"][1],
                        "leveltype2": m["level"][2],
                        "l2": m["level"][3],
                        "pindicator": m["trange"][0],
                        "p1": m["trange"][1],
                        "p2": m["trange"][2],
                        "ident": i.get("proddef", {}).get("va", {}).get("id", None),
                        "lon": lon,
                        "lat": lat,
                        "report": i["product"]["va"]["t"],
                        "datemin": datetime(*i["summarystats"]["b"]),
                        "datemax": datetime(*i["summarystats"]["e"]),
                    }}

    #def query_data(self, rec):
    #    db = dballe.DB.connect_from_url("mem:")
    #    self.load_arkiquery_to_dbadb(rec, db)
    #    for r in db.query_data(rec):
    #        yield r


    def get_datastream(self, rec):
        def if_null(value, default="-"):
            # TODO spostarla in utils
            return value if value is not None else default

        query = self.record_to_arkiquery(rec)
        # TODO ho disattivato i parametri che sono None perché dbadb non gestisce più il "-" nelle query.
        filter= [
            "{}={}".format(kk, if_null(rec[kk])) for kk in [
                "ident",
                "report",
                # "lat", "lon",
                "leveltype1", "l1",
                "leveltype2", "l2",
                "pindicator", "p1", "p2"
            ] if kk in rec and rec[kk] is not None
        ] + [
            "{}={:.5f}".format(kk, rec[kk]/10**5)
            for kk in [
                "lon", "lat",
            ] if kk in rec and rec[kk] is not None
        ]
        
        filter = " ".join(filter)
        
        myvar=rec.get("var",None)
        if (not myvar is None):
            filter+= " var={}".format(myvar)
        url = "{}/query?{}".format(self.dataset, "&".join([
            "{}={}".format(k, quote(v)) for k, v in {
                "style": "postprocess",
                "command": "bufr-filter "+filter,
                "query": query }.items()]))

        return urlopen(url)


    def query_data(self, rec):

        fo=self.get_datastream(rec)
        memdb = dballe.DB.connect_from_url("mem:")

        with tempfile.SpooledTemporaryFile(max_size=10000000) as tmpf:
            tmpf.write(fo.read())
            tmpf.seek(0)
            memdb.load(tmpf, "BUFR")

        for r in memdb.query_data(rec):
            #TODO del r["ana_id"]
            #TODO del r["data_id"]
            yield r

    def query_station_data(self, rec):
        # fo=self.get_datastream(rec)
        # memdb = dballe.DB.connect_from_url("mem:")

        # with tempfile.SpooledTemporaryFile(max_size=10000000) as tmpf:
        #     tmpf.write(fo.read())
        #     tmpf.seek(0)
        #     memdb.load(tmpf, "BUFR")

        # for r in memdb.query_station_data(rec):
        #     #TODO del r["ana_id"]
        #     #TODO del r["data_id"]
        #     yield r
        dates = set(r["datemax"] for r in self.query_summary({}))
        db = dballe.DB.connect_from_url("mem:")
        for d in dates:
            self.load_arkiquery_to_dbadb({"datetime":d}, db)
            
        with db.transaction() as tr:
            for cur in tr.query_station_data(rec):
                data={}
                data["ident"]=cur["ident"]
                data["report"]=cur["report"]
                data["lat"]=cur.enqi("lat")
                data["lon"]=cur.enqi("lon")
                data["var"]=cur["var"]
                data[cur["var"]]=cur[cur["var"]].get()
                #print ("dballe query station data: ",data)
                yield data

    def fill_data_db(self, rec,memdb):

        fo=self.get_datastream(rec)
        with tempfile.SpooledTemporaryFile(max_size=10000000) as tmpf:
            tmpf.write(fo.read())
            tmpf.seek(0)
            memdb.load(tmpf, "BUFR")

    def fill_station_data_db(self, rec,memdb):

        fo=self.get_datastream(rec)
        with tempfile.SpooledTemporaryFile(max_size=10000000) as tmpf:
            tmpf.write(fo.read())
            tmpf.seek(0)
            memdb.load(tmpf, "BUFR")
            
    def load_arkiquery_to_dbadb(self, rec, db):
        query = self.record_to_arkiquery(rec)
        url = "{}/query?{}".format(self.dataset, "&".join([
            "{}={}".format(k, quote(v)) for k, v in {
                "style": "data",
                "query": query,
            }.items()]))
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

        try:
            d = datetime(*(rec[k] for k in ("yearmin", "monthmin", "daymin", "hourmin", "minumin", "secmin")))
            q["reftime"].append(">={}".format(d))
        except KeyError:
            # Se sono qui, vuol dire che non si sono
            pass

        try:
            d = datetime(*(rec[k] for k in ("yearmax", "monthmax", "daymax", "hourmax", "minumax", "secmax")))
            q["reftime"].append("<={}".format(d))
        except KeyError:
            # Se sono qui, vuol dire che non si sono
            pass

        try:
            d = rec["datemin"]
            q["reftime"].append(">={}".format(d))
        except KeyError:
            # Se sono qui, vuol dire che non c'è
            pass

        try:
            d = rec["datemax"]
            q["reftime"].append("<={}".format(d))
        except KeyError:
            # Se sono qui, vuol dire che non c'è
            pass

        d = None
        if "sec" in rec:
            d = "{year}-{month}-{day} {hour}:{min}:{sec}".format(**rec)
        elif "min" in rec:
            d = "{year}-{month}-{day} {hour}:{min}".format(**rec)
        elif "hour" in rec:
            d = "{year}-{month}-{day} {hour}".format(**rec)
        elif "day" in rec:
            d = "{year}-{month}-{day}".format(**rec)
        elif "month" in rec:
            d = "{year}-{month}".format(**rec)
        elif "year" in rec:
            d = "{year}".format(**rec)

        if d is not None:
            q["reftime"].append("={}".format(d))

        try:
            d = rec["datetime"]
            q["reftime"].append("={}".format(d))
        except KeyError:
            # Se sono qui, vuol dire che non c'è
            pass

        for k in ["lon", "lat"]:
            if k in rec:
                q["area"]["fixed"][k] = int(rec[k])
                q["area"]["mobile"][{"lon": "x", "lat": "y"}[k]] = math.floor(rec[k])

        if "report" in rec:
            q["product"] = "BUFR:t={}".format(rec["report"])

        if "ident" in rec and rec["ident"] is not None:
            q["proddef"] = "GRIB:id={}".format(rec["ident"])

        q["reftime"] = ",".join(q["reftime"])

        q["area"] = "GRIB:{}".format(",".join([
            "{}={}".format(k, v) for k, v in q["area"]["fixed"].items()
        ])) + " or GRIB:{}".format(",".join([
            "{}={}".format(k, v) for k, v in q["area"]["mobile"].items()
        ]))

        if "lonmin" in rec and "latmin" in rec and "lonmax" in rec and "latmax" in rec:
            q["area"] +="; area:bbox coveredby POLYGON(({} {},{} {},{} {},{} {}))".format(
                rec["lonmin"],rec["latmin"],rec["lonmin"],rec["latmax"],rec["lonmax"],rec["latmax"],rec["lonmin"],rec["latmin"]
            )

        arkiquery = ";".join("{}:{}".format(k, v) for k, v in q.items())
        return arkiquery
