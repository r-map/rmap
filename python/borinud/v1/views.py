# encoding: utf-8
# borinud/v2/views - v2 views for borinud
# Author: Emanuele Di Giacomo <emanueledigiacomo@gmail.com>

from django.http import JsonResponse
from django.http import StreamingHttpResponse
import json
from datetime import datetime,timedelta
import itertools

from ..settings import BORINUD
from .utils import params2record
from ..utils.source import get_db

lastdays=7

def json_serial(obj):
    """JSON serializer for objects not serializable by default json code"""

    if isinstance(obj, datetime):
        serial = obj.isoformat()
        return serial
    else:
        raise TypeError("Type not serializable")



class dbajson:

    def __init__(self,q,summary=False,stations=False,stationdata=False,format="jsonlines",dsn="report",seg="last",attr=False):
        self.q=q
        self.summary=summary
        self.stations=stations
        self.stationdata=stationdata
        self.format=format
        if self.stations:
            self.jsondict=self.jsondictstation
        elif self.stationdata:
            self.jsondict=self.jsondictstationdata
        else:
            self.jsondict=self.jsondictdata
        self.dsn=dsn
        self.last= seg == "last"
        self.attr = attr
        #print ("++++++++++++++++++++++++++++++++++++++++++++++++++++")
        #print ("summary=",self.summary)
        #print ("format=",self.format)
        #print ("stations=",self.stations)
        #print ("stationdata=",self.stationdata)
        #print ("last=",self.last)
        #print ("attr=",self.attr)
        #print (q)
        #print ("++++++++++++++++++++++++++++++++++++++++++++++++++++")


    def __iter__(self):
        if self.summary:
            self.handle = get_db(dsn=self.dsn,last=self.last).query_summary(self.q)
        elif self.stations:
            self.handle = get_db(dsn=self.dsn,last=self.last).query_stations(self.q)
        elif self.stationdata:
            self.handle = get_db(dsn=self.dsn,last=self.last).query_station_data(self.q)
        else:
            self.handle = get_db(dsn=self.dsn,last=self.last).query_data(self.q,self.attr)

        return next(self)
        #yield from self.handle
        
    def __next__(self):

        if self.format == "geojson" :
            features=[]

        if self.format == "dbajson" :
            jsondicts=[]

        for self.s in self.handle:
            if self.format == "geojson" :

                if self.stations:
                    properties= {
                        "ident": self.s.get("ident",None),
                        "lon": self.s["lon"],
                        "lat": self.s["lat"],
                        "network": self.s["report"],
                    }

                elif self.stationdata:
                    properties= {
                        "ident": self.s.get("ident",None),
                        "lon": self.s["lon"],
                        "lat": self.s["lat"],
                        "network": self.s["report"],
                        "var": self.s["var"],
                        "val": self.s[self.s["var"]]
                    }
                    
                else:
                    properties= {
                        "ident": self.s["ident"],
                        "lon": self.s["lon"],
                        "lat": self.s["lat"],
                        "network": self.s["report"],
                        "trange": (self.s["pindicator"],self.s["p1"],self.s["p2"]),
                        "level": (self.s["leveltype1"],self.s["l1"],self.s["leveltype2"],self.s["l2"]),
                        "date": (self.s["datemin"],self.s["datemax"]) if self.summary else  self.s["date"],
                        "var": self.s["var"],
                        "val": self.s[self.s["var"]]
                }

                features.append({
                    "type": "Feature",
                        "geometry": {
                            "type": "Point",
                            "coordinates": [float(self.s["lon"])/100000., float(self.s["lat"])/100000.],
                        },
                        "properties": properties
                    })


            if self.format == "jsonline" :
                yield json.dumps(self.jsondict(),default=json_serial)+"\n"

            if self.format == "dbajson" :
                jsondicts.append(self.jsondict())


        if self.format == "dbajson" :
            yield jsondicts

        if self.format == "geojson" :
            yield {"type": "FeatureCollection", "features": features}

    def jsondictdata (self):

        dictdata= {
            "ident": self.s["ident"],
            "lon": self.s["lon"],
            "lat": self.s["lat"],
            "network": self.s["report"],
            "date": (self.s["datemin"],self.s["datemax"]) if self.summary else self.s["date"]
        }

        if (self.attr and not self.summary):
            dictadata["data"]= [{
                "vars": {
                    self.s["var"]: {
                        "v": self.s[self.s["var"]],
                        "a": self.s["a"]
                    }
                },
                "timerange": (self.s["pindicator"],self.s["p1"],self.s["p2"]),
                "level": (self.s["leveltype1"],self.s["l1"],self.s["leveltype2"],self.s["l2"]),
            }]
        
        else:
            dictadata["data"]= [{
                "vars": {
                    self.s["var"]: {
                        "v": None if self.summary else self.s[self.s["var"]]
                    }
                },
                "timerange": (self.s["pindicator"],self.s["p1"],self.s["p2"]),
                "level": (self.s["leveltype1"],self.s["l1"],self.s["leveltype2"],self.s["l2"]),
            }]
        

    def jsondictstationdata (self):
        return {
            "ident": self.s["ident"],
            "lon": self.s["lon"],
            "lat": self.s["lat"],
            "network": self.s["report"],
            "data": [{
                "vars": {
                    self.s["var"]: {
                        "v": None if self.summary else self.s[self.s["var"]]
                    }
                }
            }]
        }
    
    
    def jsondictstation (self):

        return {
            "ident": self.s.get("ident",None),
            "lon": self.s["lon"],
            "lat": self.s["lat"],
            "network": self.s["report"],
        }


def summaries(request, **kwargs):
    q = params2record(kwargs)

    if ( request.GET.get("yearmin") is not None):
        q["yearmin"] = int(request.GET.get("yearmin"))
    if ( request.GET.get("yearmax") is not None):
        q["yearmax"] = int(request.GET.get("yearmax"))
    if ( request.GET.get("monthmin") is not None):
        q["monthmin"] = int(request.GET.get("monthmin"))
    if ( request.GET.get("monthmax") is not None):
        q["monthmax"] = int(request.GET.get("monthmax"))
    if ( request.GET.get("daymin")  is not None):
        q["daymin"] = int(request.GET.get("daymin"))
    if ( request.GET.get("daymax")  is not None):
        q["daymax"] = int(request.GET.get("daymax"))
    if ( request.GET.get("hourmin") is not None):
        q["hourmin"] = int(request.GET.get("hourmin"))
    if ( request.GET.get("hourmax") is not None):
        q["hourmax"] = int(request.GET.get("hourmax"))
    if ( request.GET.get("minumin") is not None):
        q["minumin"] = int(request.GET.get("minumin"))
    if ( request.GET.get("minumax") is not None):
        q["minumax"] = int(request.GET.get("minumax"))
    if ( request.GET.get("secmin")  is not None):
        q["secmin"] = int(request.GET.get("secmin"))
    if ( request.GET.get("secmax")  is not None):
        q["secmax"] = int(request.GET.get("secmax"))

    if ( request.GET.get("latmin") is not None):
        q["latmin"] = int(request.GET.get("latmin"))
    if ( request.GET.get("latmax") is not None):
        q["latmax"] = int(request.GET.get("latmax"))
    if ( request.GET.get("lonmin") is not None):
        q["lonmin"] = int(request.GET.get("lonmin"))
    if ( request.GET.get("lonmax") is not None):
        q["lonmax"] = int(request.GET.get("lonmax"))

    if (not 'yearmin' in q) or (not 'yearmax' in q):
        if (kwargs.get('year') is not None):
            q['year'] = int(kwargs.get('year'))
        if (kwargs.get('month') is not None):
            q['month'] = int(kwargs.get('month'))
        if (kwargs.get('day') is not None):
            q['day'] = int(kwargs.get('day'))
        if (kwargs.get('hour') is not None):
            q["hour"] = int(kwargs.get("hour"))


    bd={}
    bd['year']  = int(kwargs.get('year',"1"))
    bd['month'] = int(kwargs.get('month',"1"))
    bd['day']   = int(kwargs.get('day',"1"))
    bd["year"]  = int(bd["year"]) if request.GET.get("yearmin") is None else int(request.GET.get("yearmin")) 
    bd["month"] = int(bd["month"]) if request.GET.get("monthmin") is None else int(request.GET.get("monthmin"))
    bd["day"]   = int(bd["day"]) if request.GET.get("daymin") is None else int(request.GET.get("daymin"))
    b = datetime(int(bd["year"]), int(bd["month"]), int(bd["day"]))
    if b <  (datetime.utcnow()-timedelta(days=lastdays)):
        seg="historical"
    else:
        seg="last"

    format=kwargs.get('format')

    if format == "geojson" or format == "dbajson" :
        return JsonResponse(next(itertools.islice(dbajson(q,summary=True,format=format,dsn=request.GET.get('dsn', 'report'),seg=request.GET.get('seg', seg)),0,None)),safe=False)

    if format == "jsonline" :
        return StreamingHttpResponse(dbajson(q,summary=True,format=format,dsn=request.GET.get('dsn', 'report'),seg=request.GET.get('seg', seg)))

def timeseries(request, **kwargs):
    q = params2record(kwargs)

    if (request.GET.get("yearmin") is not None ):
        q["yearmin"] = int(request.GET.get("yearmin"))
    if (request.GET.get("yearmax") is not None ):
        q["yearmax"] = int(request.GET.get("yearmax"))
    if (request.GET.get("monthmin") is not None ):
        q["monthmin"] = int(request.GET.get("monthmin"))
    if (request.GET.get("monthmax") is not None ):
        q["monthmax"] = int(request.GET.get("monthmax"))
    if (request.GET.get("daymin") is not None ):
        q["daymin"] = int(request.GET.get("daymin"))
    if (request.GET.get("daymax") is not None ):
        q["daymax"] = int(request.GET.get("daymax"))
    if (request.GET.get("hourmin") is not None ):
        q["hourmin"] = int(request.GET.get("hourmin"))
    if (request.GET.get("hourmax") is not None ):
        q["hourmax"] = int(request.GET.get("hourmax"))
    if (request.GET.get("minumin") is not None ):
        q["minumin"] = int(request.GET.get("minumin"))
    if (request.GET.get("minumax") is not None ):
        q["minumax"] = int(request.GET.get("minumax"))
    if (request.GET.get("secmin") is not None ):
        q["secmin"] = int(request.GET.get("secmin"))
    if (request.GET.get("secmax") is not None ):
        q["secmax"] = int(request.GET.get("secmax"))

    if (not 'yearmin' in q) or (not 'yearmax' in q):
        if (kwargs.get('year') is not None):
            q['year'] = int(kwargs.get('year'))
        if (kwargs.get('month') is not None):
            q['month'] = int(kwargs.get('month'))
        if (kwargs.get('day') is not None):
            q['day'] = int(kwargs.get('day'))
        if (kwargs.get("hour") is not None):
            q["hour"] = int(kwargs.get("hour"))


    bd={}
    bd['year']  = int(kwargs.get('year',"1"))
    bd['month'] = int(kwargs.get('month',"1"))
    bd['day']   = int(kwargs.get('day',"1"))
    bd["year"]  = int(bd["year"]) if request.GET.get("yearmin") is None else int(request.GET.get("yearmin"))
    bd["month"] = int(bd["month"]) if request.GET.get("monthmin") is None else int(request.GET.get("monthmin"))
    bd["day"]   = int(bd["day"]) if request.GET.get("daymin") is None else int(request.GET.get("daymin")) 
    b = datetime(int(bd["year"]), int(bd["month"]), int(bd["day"]))

    if b <  (datetime.utcnow()-timedelta(days=lastdays)):
        seg="historical"
    else:
        seg="last"

    #https://codefisher.org/catch/blog/2015/04/22/python-how-group-and-count-dictionaries/
    #from collections import defaultdict
    #d = defaultdict(list)

    format=kwargs.get('format')

    if format == "geojson" or format == "dbajson" :
        return JsonResponse(next(itertools.islice(dbajson(q,format=format,dsn=request.GET.get('dsn', 'report'),seg=request.GET.get('seg', seg),attr=request.GET.get("attr")),0,None)),safe=False)

    if format == "jsonline" :
        return StreamingHttpResponse(dbajson(q,format=format,dsn=request.GET.get('dsn', 'report'),seg=request.GET.get('seg', seg),attr=request.GET.get("attr")))


def spatialseries(request, **kwargs):
    q = params2record(kwargs)

    if kwargs.get("hour") is None:
        b = datetime(*(int(kwargs[k]) for k in ("year", "month", "day")))
        e = datetime(*(int(kwargs[k]) for k in ("year", "month", "day")),hour=23,minute=59,second=59)
    else:
        d = datetime(*(int(kwargs[k]) for k in ("year", "month", "day", "hour")))
        b = d - timedelta(seconds=1800)
        e = d + timedelta(seconds=1799)

#    q["datemin"] = b
#    q["datemax"] = e

    q["yearmin"] = b.year
    q["monthmin"] = b.month
    q["daymin"] = b.day
    q["hourmin"] = b.hour
    q["minumin"] = b.minute
    q["secmin"] = b.second

    q["yearmax"] = e.year
    q["monthmax"] = e.month
    q["daymax"] = e.day
    q["hourmax"] = e.hour
    q["minumax"] = e.minute
    q["secmax"] = e.second

    if ( request.GET.get("latmin") is not None):
        q["latmin"] = int(request.GET.get("latmin"))
    if ( request.GET.get("latmax") is not None):
        q["latmax"] = int(request.GET.get("latmax"))
    if ( request.GET.get("lonmin") is not None):
        q["lonmin"] = int(request.GET.get("lonmin"))
    if ( request.GET.get("lonmax") is not None):
        q["lonmax"] = int(request.GET.get("lonmax"))

    if b <  (datetime.utcnow()-timedelta(days=lastdays)):
        seg="historical"
    else:
        seg="last"

    format=kwargs.get('format')

    if format == "geojson" or format == "dbajson" :
        return JsonResponse(next(itertools.islice(dbajson(q,format=format,dsn=request.GET.get('dsn', 'report'),seg=request.GET.get('seg', seg),attr=request.GET.get("attr")),0,None)),safe=False)

    if format == "jsonline" :
        return StreamingHttpResponse(dbajson(q,format=format,dsn=request.GET.get('dsn', 'report'),seg=request.GET.get('seg', seg),attr=request.GET.get("attr")))


def stationdata(request, **kwargs):
    q = params2record(kwargs)

    format=kwargs.get('format')

    if format == "geojson" or format == "dbajson" :
        return JsonResponse(next(itertools.islice(dbajson(q,stationdata=True,format=format,dsn=request.GET.get('dsn', 'report'),seg=request.GET.get('seg', 'historical')),0,None)),safe=False)

    if format == "jsonline" :
        return StreamingHttpResponse(dbajson(q,stationdata=True,format=format,dsn=request.GET.get('dsn', 'report'),seg=request.GET.get('seg', 'historical')))


def stations(request, **kwargs):
    q = params2record(kwargs)

    format=kwargs.get('format')

    if format == "geojson" or format == "dbajson" :
        return JsonResponse(next(itertools.islice(dbajson(q,stations=True,format=format,dsn=request.GET.get('dsn', 'report'),seg=request.GET.get('seg', 'historical')),0,None)),safe=False)

    if format == "jsonline" :
        return StreamingHttpResponse(dbajson(q,stations=True,format=format,dsn=request.GET.get('dsn', 'report'),seg=request.GET.get('seg', 'historical')))
