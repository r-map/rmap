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

    def __init__(self,q,summary=False,stations=False,format="jsonlines",dsn="report",seg="last"):
        self.q=q
        self.summary=summary
        self.stations=stations
        self.format=format
        if self.stations:
            self.jsondict=self.jsondictstation
        else:
            self.jsondict=self.jsondictdata
        self.dsn=dsn
        self.last= seg == "last"
        #print ("++++++++++++++++++++++++++++++++++++++++++++++++++++")
        #print ("summary=",self.summary)
        #print ("stations=",self.stations)
        #print ("last=",self.last)
        #print (q)
        #print ("++++++++++++++++++++++++++++++++++++++++++++++++++++")


    def __iter__(self):
        if self.summary:
            self.handle = get_db(dsn=self.dsn,last=self.last).query_summary(self.q)
        elif self.stations:
            self.handle = get_db(dsn=self.dsn,last=self.last).query_stations(self.q)
        else:
            self.handle = get_db(dsn=self.dsn,last=self.last).query_data(self.q)

        return next(self)

    def __next__(self):

        if self.format == "geojson" :
            features=[]

        if self.format == "dbajson" :
            jsondicts=[]

        for self.s in self.handle:
            if self.format == "geojson" :

                if self.stations:
                    properties= {
                        "ident": self.s["ident"],
                        "lon": self.s.enqd("lon"),
                        "lat": self.s.enqd("lat"),
                        "network": self.s["rep_memo"],
                        "var": self.s["var"],
                        "val": self.s.enqd(self.s["var"])
                    }

                else:
                    properties= {
                        "ident": self.s["ident"],
                        "lon": self.s.enqi("lon"),
                        "lat": self.s.enqi("lat"),
                        "network": self.s["rep_memo"],
                        "trange": (self.s["trange"].pind,self.s["trange"].p1,self.s["trange"].p2),
                        "level": (self.s["level"].ltype1,self.s["level"].l1,self.s["level"].ltype2,self.s["level"].l2),
                        "date": list( d for d in self.s.date_extremes()) if self.summary else list( d for d in (self.s["year"],self.s["month"],self.s["day"],self.s["hour"],self.s["min"],self.s["sec"])),
                        "var": self.s["var"],
                        "val": self.s.enqd(self.s["var"]),
                    }

                features.append({
                    "type": "Feature",
                        "geometry": {
                            "type": "Point",
                            "coordinates": [self.s.enqd("lon"), self.s.enqd("lat")],
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

        if (self.summary):
            #print (self.s["datemin"],self.s["datemax"]) 
            return {
                "ident": self.s["ident"],
                "lon": self.s["lon"],
                "lat": self.s["lat"],
                "network": self.s["rep_memo"],
                "date": (self.s["datemin"],self.s["datemax"]) if self.summary else self.s["datetime"],
                "data": [{
                    "vars": {
                        self.s["var"]: {
                            "v": None if self.summary else self.s.enqd(self.s["var"])
                        }
                    },
                    "timerange": (self.s["trange"][0],self.s["trange"][1],self.s["trange"][2]),
                    "level": (self.s["level"][0],self.s["level"][1],self.s["level"][2],self.s["level"][3]),
                }]
            }
        else:

            return {
                "ident": self.s["ident"],
                "lon": self.s["lon"],
                "lat": self.s["lat"],
                "network": self.s["rep_memo"],
                "date": (self.s["datemin"],self.s["datemax"]) if self.summary else self.s["datetime"],
                "data": [{
                    "vars": {
                        self.s["var"]: {
                            "v": None if self.summary else self.s.enqd(self.s["var"])
                        }
                    },
                    "timerange": (self.s["trange"].pind,self.s["trange"].p1,self.s["trange"].p2),
                    "level": (self.s["level"].ltype1,self.s["level"].l1,self.s["level"].ltype2,self.s["level"].l2),
                }]
            }


    
    def jsondictstation (self):

        return {
            "ident": self.s.get("ident"),
            "lon": self.s.enqi("lon"),
            "lat": self.s.enqi("lat"),
            "network": self.s["rep_memo"],
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
        return JsonResponse(next(itertools.islice(dbajson(q,format=format,dsn=request.GET.get('dsn', 'report'),seg=request.GET.get('seg', seg)),0,None)),safe=False)

    if format == "jsonline" :
        return StreamingHttpResponse(dbajson(q,format=format,dsn=request.GET.get('dsn', 'report'),seg=request.GET.get('seg', seg)))


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

    q["latmin"] = request.GET.get("latmin")
    q["latmax"] = request.GET.get("latmax")
    q["lonmin"] = request.GET.get("lonmin")
    q["lonmax"] = request.GET.get("lonmax")

    if b <  (datetime.utcnow()-timedelta(days=lastdays)):
        seg="historical"
    else:
        seg="last"

    format=kwargs.get('format')

    if format == "geojson" or format == "dbajson" :
        return JsonResponse(next(itertools.islice(dbajson(q,format=format,dsn=request.GET.get('dsn', 'report'),seg=request.GET.get('seg', seg)),0,None)),safe=False)

    if format == "jsonline" :
        return StreamingHttpResponse(dbajson(q,format=format,dsn=request.GET.get('dsn', 'report'),seg=request.GET.get('seg', seg)))


def stationdata(request, **kwargs):
    q = params2record(kwargs)

    format=kwargs.get('format')

    if format == "geojson" or format == "dbajson" :
        return JsonResponse(next(itertools.islice(dbajson(q,format=format,dsn=request.GET.get('dsn', 'report'),seg=request.GET.get('seg', 'historical')),0,None)),safe=False)

    if format == "jsonline" :
        return StreamingHttpResponse(dbajson(q,format=format,dsn=request.GET.get('dsn', 'report'),seg=request.GET.get('seg', 'historical')))


def stations(request, **kwargs):
    q = params2record(kwargs)

    format=kwargs.get('format')

    if format == "geojson" or format == "dbajson" :
        return JsonResponse(next(itertools.islice(dbajson(q,stations=True,format=format,dsn=request.GET.get('dsn', 'report'),seg=request.GET.get('seg', 'hostorical')),0,None)),safe=False)

    if format == "jsonline" :
        return StreamingHttpResponse(dbajson(q,stations=True,format=format,dsn=request.GET.get('dsn', 'report'),seg=request.GET.get('seg', 'historical')))
