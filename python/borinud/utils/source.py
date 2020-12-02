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
from urllib.parse import quote
from urllib.request import urlopen
import sys,io
from arkimet.cmdline.query import Query
from contextlib import redirect_stdout
    
from ..settings import BORINUD,BORINUDLAST


def get_db(dsn="report",last=True,attr=False):
    from django.utils.module_loading import import_string

    dbs = [
        import_string(i["class"])(**{
            k: v for k, v in list(i.items()) + [("attr", attr)] if k != "class"
        })
        for i in (BORINUDLAST[dsn]["SOURCES"] if last else BORINUD[dsn]["SOURCES"])
    ]

    if len(dbs) == 1:
        db = dbs[0]
    else:
        db = MergeDB(dbs,attr)

    return db


class DB(object):
    """Abstract class.
    A concrete DB must implement the following methods

    - `query_summary`
    - `query_data`
    - `query_stations`
    - `query_stations_data`
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



class MergeDB(DB):
    """Container for DB."""
    def __init__(self, dbs,attr=False):
        self.dbs = dbs
        self.attr = attr

    #def _unique_record_key(self, rec):
    #    """Create a string from a record, based on ident, lon, lat, report,
    #    pindicator, level and var values. Null values are encoded as "-"."""
    #    def if_null(value, default="-"):
    #        return value if value is not None else default
    #
    #    return (
    #        "{}/"
    #        "{},{}/"
    #        "{}/"
    #        "{},{},{}/"
    #        "{},{},{},{}/"
    #        "{}"
    #    ).format(*map(if_null, (
    #        rec.get("ident",None),
    #        rec["lon"],
    #        rec["lat"],
    #        rec["report"],
    #        rec["pindicator"],
    #        rec["p1"],
    #        rec["p2"],
    #        rec["leveltype1"],
    #        rec["l1"],
    #        rec["leveltype2"],
    #        rec["l2"],
    #        rec["var"],
    #    )))


    #def _unique_record_station_key(self, rec):
    #    """Create a string from a record, based on ident, lon, lat, report,
    #    pindicator, level and var values. Null values are encoded as "-"."""
    #    def if_null(value, default="-"):
    #        return value if value is not None else default
    #
    #    return (
    #        "{}/"
    #        "{},{}/"
    #        "{}/"
    #    ).format(*map(if_null, (
    #        rec.get("ident",None),
    #        rec["lon"],
    #        rec["lat"],
    #        rec["report"]
    #    )))

    
    #def _get_unique_records(self, funcname, rec, reducer):
    #    for k, g in groupby(sorted([
    #        r for db in self.dbs for r in getattr(db, funcname)(rec)
    #    ], key=self._unique_record_key), self._unique_record_key):
    #        yield reducer(g)

    #def _get_unique_station_records(self, funcname, rec, reducer):
    #    for k, g in groupby(sorted([
    #        r for db in self.dbs for r in getattr(db, funcname)(rec)
    #    ], key=self._unique_record_station_key), self._unique_record_station_key):
    #        yield reducer(g)


    def query_stations(self, rec):

        with dballe.Explorer() as explorer:
            for db in dbs:
                explorer.add_json(db.get_json_explorer())
                explorer.set_filter(rec)
                for staz in explorer.stations:
                    #print(staz.ident,staz.lat,staz.lon,staz.report) 
                    data={}
                    data["ident"]=staz.ident
                    data["report"]=staz.report
                    data["lat"]=staz.lat
                    data["lon"]=staz.lon
                    #print ("dballe query station: ",data)
                    yield data
            
    #def _query_stations_db(self, rec):
    #    for r in self._get_unique_station_records(
    #        "query_stations", rec, lambda g: next(g)
    #    ):
    #        yield r


    #def query_stations(self, rec):
    #
    #    allexplorer=True
    #
    #    # check if all DB have an explorer
    #    for db in self.dbs:
    #        if (db.explorer is None):
    #            allexplorer=False
    #
    #    if (allexplorer):
    #        for r in self._query_stations_explorer(rec)(rec):
    #            yield r
    #    else:
    #        for r in self._query_stations_db(rec):
    #            yield r
        

    def query_summary(self, rec):

        with dballe.Explorer() as explorer:
            with explorer.update() as updater:
                for db in self.dbs:
                    #jsexp=db.get_json_explorer()
                    #print (jsexp)
                    #updater.add_json(jsexp)
                    updater.add_json(db.get_json_explorer())
        
            explorer.set_filter(rec)
            for cur in explorer.query_summary(rec):
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
            

    #def _query_summary_db(self, rec):
    #    def reducer(g):
    #        rec = next(g)
    #        for r in g:
    #            if r["datemin"] < rec["datemin"]:
    #                rec["datemin"] = r["datemin"]
    #            if r["datemax"] > rec["datemax"]:
    #                rec["datemax"] = r["datemax"]
    #        return rec
    #
    #    for r in self._get_unique_records(
    #        "query_summary", rec, reducer
    #    ):
    #        yield r

    def query_data(self, rec):
        memdb = dballe.DB.connect("mem:")
        for db in self.dbs:
            db.fill_data_db(rec,memdb)

        if (self.attr):
            rec["query"]="attrs"

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
                if (self.attr):
                    attrs =  cur["variable"].get_attrs()
                    data["a"]= {v.code: v.get() for v in attrs}
                
                data["date"]=datetime(cur["year"], cur["month"], cur["day"], cur["hour"], cur["min"], cur["sec"])
                #print ("merge query data: ",data)
                yield data

    def query_station_data(self, rec):
        # TODO si devono rendere univoci i risultati raggruppando per stazione,
        # i.e. per ident,lon,lat,report.
        memdb = dballe.DB.connect("mem:")
        for db in self.dbs:
            db.fill_station_data_db(rec, memdb)

        with memdb.transaction() as tr:
            for cur in tr.query_station_data(rec):
                data={}
                data["ident"]=cur["ident"]
                data["report"]=cur["report"]
                data["lat"]=cur.enqi("lat")
                data["lon"]=cur.enqi("lon")
                data["var"]=cur["var"]
                data[cur["var"]]=cur[cur["var"]].get()
                yield data

class DballeDB(DB):
    """DB-All.e database."""
    def __init__(self, url, explorer=None, attr=False):
        """Create a DB-All.e database from `url` DSN."""
        self.url = url
        self.explorer = explorer
        self.attr=attr
        
    def __open_db(self):
        """Open the database."""
        return dballe.DB.connect(self.url)

    def query_stations(self, rec):
        """Query summary.
        """
        if (self.explorer is None):
            for item in self.query_stations_db(rec):
                yield item
        else:
            for item in self.query_stations_explorer(rec):
                yield item

    def query_stations_explorer(self, rec):

        with dballe.Explorer(self.explorer) as explorer:
            for staz in explorer.stations:
                #print(staz.ident,staz.lat,staz.lon,staz.report)
 
                data={}
                data["ident"]=staz.ident
                data["report"]=staz.report
                data["lat"]=staz.lat
                data["lon"]=staz.lon
                #print ("dballe query station: ",data)
                yield data
                
    def query_stations_db(self, rec):
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
        """Query summary.
        """
        if (self.explorer is None):
            for item in self.query_summary_db(rec):
                yield item
        else:
            for item in self.query_summary_explorer(rec):
                yield item        
                
    def query_summary_explorer(self, rec):
        """Query summary.

            Get data from dballe explorer
        """

        with dballe.Explorer(self.explorer) as explorer:

            for cur in explorer.query_summary(rec):
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


    def query_summary_db(self, rec):
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

        if (self.attr):
            rec["query"]="attrs"
        
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

                if (self.attr):
                    attrs =  cur["variable"].get_attrs()
                    data["a"]= {v.code: v.get() for v in attrs}
                
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

    def get_json_explorer(self):

        if (self.explorer is None):
            db = self.__open_db()

            with dballe.Explorer() as explorer:
                with explorer.rebuild() as updater:
                    with db.transaction() as tr:
                        updater.add_db(tr)
                return explorer.to_json()
        else:
            with dballe.Explorer(self.explorer) as explorer:
                return explorer.to_json()


    def fill_data_db(self, rec, memdb):

        db = self.__open_db()

        if (self.attr):
            rec["query"]="attrs"
        
        with db.transaction() as tr:
            with memdb.transaction() as memtr:
                for cur in tr.query_data(rec):
                    memtr.insert_data(cur.data, True, True)

    def fill_station_data_db(self, rec, memdb):

        db = self.__open_db()

        with db.transaction() as tr:
            with memdb.transaction() as memtr:
                for cur in tr.query_station_data(rec):
                    memtr.insert_station_data(cur.data, True, True)



class ArkimetBufrDB(DB):
    """Arkimet dataset containing generic ``BUFR`` data."""

    def __init__(self, dataset, explorer,attr=False):
        """
        Create a DB from an `HTTP` Arkimet `dataset` containing generic BUFR
        data.

        :param dataset: `URL` of Arkimet dataset
        :param explorer: dballe explorer for summaries
        Example::

            ArkimetBufrDB("http://localhost:8090/dataset/rmap","report_fixed")
        """
        self.dataset = dataset
        self.explorer = explorer
        self.attr = attr
        
    def query_summary(self, rec):
        """Query summary.

        .. warning::

            Get data from dballe explorer
        """

        with dballe.Explorer(self.explorer) as explorer:

            for cur in explorer.query_summary(rec):
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

        memdb = dballe.DB.connect("mem:")
        self.fill_data_db( rec,memdb)
            
        if (self.attr):
            rec["query"]="attrs"
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


                if (self.attr):
                    attrs =  cur["variable"].get_attrs()
                    data["a"]= {v.code: v.get() for v in attrs}
                
                data["date"]=datetime(cur["year"], cur["month"], cur["day"], cur["hour"], cur["min"], cur["sec"])
                #print ("dballe query data: ",data)
                yield data

    def query_stations(self, rec):
        """Query summary.
        """
        if (self.explorer is None):
            for item in self.query_stations_db(rec):
                yield item
        else:
            for item in self.query_stations_explorer(rec):
                yield item
                
    def query_stations_explorer(self, rec):

        with dballe.Explorer(self.explorer) as explorer:
            for staz in explorer.stations:
                #print(staz.ident,staz.lat,staz.lon,staz.report)
 
                data={}
                data["ident"]=staz.ident
                data["report"]=staz.report
                data["lat"]=staz.lat
                data["lon"]=staz.lon
                #print ("dballe query station: ",data)
                yield data

                
    def query_stations_db(self, rec):
        """Query stations.

        .. warning::

            Only `ident`, `report`, `lon` and `lat` are returned.
            Loading static data must be implemented.
        """
        dates = set(r["datemax"] for r in self.query_summary(rec))
        memdb = dballe.DB.connect("mem:")

        for d in dates:
            self.fill_data_db({"datetime":d},memdb)
            
        with memdb.transaction() as tr:
            for cur in tr.query_stations(rec):
                data={}
                data["ident"]=cur["ident"]
                data["report"]=cur["report"]
                data["lat"]=cur.enqi("lat")
                data["lon"]=cur.enqi("lon")
                yield data


    def query_station_data(self, rec):

        memdb = dballe.DB.connect("mem:")
        fill_station_data_db(self, rec,memdb)

        with memdb.transaction() as tr:
            for cur in tr.query_station_data(rec):
                data={}
                data["ident"]=cur["ident"]
                data["report"]=cur["report"]
                data["lat"]=cur.enqi("lat")
                data["lon"]=cur.enqi("lon")
                data["var"]=cur["var"]
                data[cur["var"]]=cur[cur["var"]].get()
                yield data


    def get_json_explorer(self):

        with dballe.Explorer(self.explorer) as explorer:
            return explorer.to_json()


    def fill_data_db(self, rec,memdb):

        query = self.record_to_arkiquery(rec)

        with io.BytesIO() as stdoutbytesio:
            with redirect_stdout(stdoutbytesio):
                sys.argv=["borinud", "--data", "--config", self.dataset, query]
                Query.main()
                stdoutbytesio.seek(0,0)    
                
            importer = dballe.Importer("BUFR")
            with importer.from_file(stdoutbytesio) as f:
                with memdb.transaction() as tr:
                    for msgs in f:
                        for msg in msgs:
                            try:
                                tr.import_messages(msg)
                            except:
                                sys.stderr.write("ERROR {m.report},{m.coords},{m.ident},{m.datetime},{m.type}".format(m=msg))


    def fill_station_data_db(self, rec,memdb):

        dates = set(r["datemax"] for r in self.query_summary(rec))
        memdb = dballe.DB.connect("mem:")

        for d in dates:
            self.fill_data_db({"datetime":d},memdb)


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


#class ArkimetVm2DB(DB):
#    """Arkimet dataset containing ``VM2`` data."""
#    def __init__(self, dataset):
#        self.dataset = dataset
#
#    def record_to_arkiquery(self, rec):
#        """Translate a dballe.Record to arkimet query."""
#        # TODO: less verbose implementation
#        q = {
#            "reftime": [],
#            "area": {},
#            "product": {},
#        }
#
#        d1, d2 = rec.date_extremes()
#        if d1:
#            q["reftime"].append(">={}".format(d1))
#
#        if d2:
#            q["reftime"].append("<={}".format(d2))
#
#        for k in ["lon", "lat"]:
#            if k in rec:
#                q["area"][k] = int(rec[k] * 10**5)
#
#        if "report" in rec:
#            q["area"]["rep"] = rec["report"]
#
#        if "var" in rec:
#            q["product"]["bcode"] = rec["var"]
#
#        if "leveltype1" in rec:
#            q["product"]["lt1"] = rec["leveltype1"]
#
#        if "l1" in rec:
#            q["product"]["l1"] = rec["l1"]
#
#        if "leveltype2" in rec:
#            q["product"]["lt2"] = rec["leveltype2"]
#
#        if "l2" in rec:
#            q["product"]["l2"] = rec["l2"]
#
#        if "pindicator" in rec:
#            q["product"]["tr"] = rec["pindicator"]
#
#        if "p1" in rec:
#            q["product"]["p1"] = rec["p1"]
#
#        if "p2" in rec:
#            q["product"]["p2"] = rec["p2"]
#
#        q["reftime"] = ",".join(q["reftime"])
#        q["area"] = "VM2:{}".format(",".join([
#            "{}={}".format(k, v) for k, v in q["area"].items()
#        ]))
#        q["product"] = "VM2:{}".format(",".join([
#            "{}={}".format(k, v) for k, v in q["product"].items()
#        ]))
#
#        arkiquery = ";".join("{}:{}".format(k, v) for k, v in q.items())
#
#        return arkiquery

#TO BE DONE!
#TO BE DONE!            
#    def query_data(self, rec):   
#        query = self.record_to_arkiquery(rec)
#        url = "{}/query?{}".format(self.dataset, "&".join([
#            "{}={}".format(k, quote(v)) for k, v in {
#                #"style": "postprocess",
#                #"command": "json",
#                "query": query,
#            }.items()]))
#        r = urlopen(url)
#        for f in json.load(r)["features"]:
#            p = f["properties"]
#            r = {**{
#                "lon": p["lon"],
#                "lat": p["lat"],
#                "report": str(p["network"]),
#                "level": tuple(p[k] for k in ["level_t1", "level_v1",
#                                              "level_t2", "level_v2"]),
#                "trange": tuple(p[k] for k in ["trange_pind",
#                                               "trange_p1", "trange_p2"]),
#                "date": datetime.strptime(p["datetime"], "%Y-%m-%dT%H:%M:%SZ"),
#                str(p["bcode"]): float(p["value"]),
#            }}
#            yield r

#TO BE DONE!
#TO BE DONE!
#    def query_station_data(self, rec):
#        query = self.record_to_arkiquery(rec)
#        url = "{}/query?{}".format(self.dataset, "&".join([
#            "{}={}".format(k, quote(v)) for k, v in {
#                #"style": "postprocess",
#                #"command": "json",
#                "query": query,
#            }.items()]))
#        r = urlopen(url)
#        for f in json.load(r)["features"]:
#            p = f["properties"]
#            r = {**{
#                "lon": p["lon"],
#                "lat": p["lat"],
#                "report": str(p["network"]),
#                "level": tuple(p[k] for k in ["level_t1", "level_v1",
#                                              "level_t2", "level_v2"]),
#                "trange": tuple(p[k] for k in ["trange_pind",
#                                               "trange_p1", "trange_p2"]),
#                "date": datetime.strptime(p["datetime"], "%Y-%m-%dT%H:%M:%SZ"),
#                str(p["bcode"]): float(p["value"]),
#            }}
#            yield r
#            
#    def fill_data_db(self,rec,memedb):
#        with memdb.transaction() as tr:
#            for r in self.query_data(rec):
#                tr.insert_data(r, True, True)
#
#    def fill_station_data_db(self,rec,memedb):
#        with memdb.transaction() as tr:
#            for r in self.query_station_data(rec):
#                tr.insert_station_data(r, True, True)
#            
#    def query_summary(self, rec):
#        query = self.record_to_arkiquery(rec)
#        url = "{}/summary?{}".format(self.dataset, "&".join([
#            "{}={}".format(k, quote(v)) for k, v in {
#                "style": "json",
#                "query": query,
#            }.items()]))
#        r = urlopen(url)
#        for i in json.load(r)["items"]:
#            if not "va" in  i["area"] or not "va" in i["product"]:
#                continue
#            yield {**{
#                "ident": i["area"]["va"].get("ident"),
#                "lon": i["area"]["va"]["lon"],
#                "lat": i["area"]["va"]["lat"],
#                "report": i["area"]["va"]["rep"],
#                "var": i["product"]["va"]["bcode"],
#                "level": (i["product"]["va"]["lt1"],
#                          i["product"]["va"].get("l1"),
#                          i["product"]["va"].get("lt2"),
#                          i["product"]["va"].get("l2")),
#                "trange": (i["product"]["va"]["tr"],
#                           i["product"]["va"]["p1"],
#                           i["product"]["va"]["p2"]),
#                "datemin": datetime(*i["summarystats"]["b"]),
#                "datemax": datetime(*i["summarystats"]["e"]),
#            }}
#
#    def query_stations(self, rec):
#        """Not yet implemented."""
#        return DB.query_stations(self, rec)

