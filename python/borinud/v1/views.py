# encoding: utf-8
# borinud/v2/views - v2 views for borinud
# Author: Emanuele Di Giacomo <emanueledigiacomo@gmail.com>

from django.http import JsonResponse
from django.http import StreamingHttpResponse
import json
from datetime import datetime
import itertools

from ..settings import BORINUD
from .utils import params2record
from ..utils.source import get_db

def json_serial(obj):
    """JSON serializer for objects not serializable by default json code"""

    if isinstance(obj, datetime):
        serial = obj.isoformat()
        return serial
    else:
        raise TypeError("Type not serializable")



class dbajson:

    def __init__(self,q,summary=False,stations=False,format="jsonlines",dsn="report"):
        self.q=q
        self.summary=summary
        self.stations=stations
        self.format=format
        if self.stations:
            self.jsondict=self.jsondictstation
        else:
            self.jsondict=self.jsondictdata
        self.dsn=dsn


    def __iter__(self):
        if self.summary:
            self.handle = get_db(dsn=self.dsn).query_summary(self.q)
        elif self.stations:
            self.handle = get_db(dsn=self.dsn).query_stations(self.q)
        else:
            self.handle = get_db(dsn=self.dsn).query_data(self.q)

        return self.next()

    def next(self):

        if self.format == "geojson" :
            features=[]

        if self.format == "dbajson" :
            jsondicts=[]

        for self.s in self.handle:

            if self.format == "geojson" :

                if self.stations:
                    properties= {
                        "ident": self.s.get("ident"),
                        "lon": self.s.key("lon").enqi(),
                        "lat": self.s.key("lat").enqi(),
                        "network": self.s["rep_memo"],
                        "var": self.s["var"],
                        "val": self.s[self.s["var"]]
                    }

                else:
                    properties= {
                        "ident": self.s.get("ident"),
                        "lon": self.s.key("lon").enqi(),
                        "lat": self.s.key("lat").enqi(),
                        "network": self.s["rep_memo"],
                        "trange": self.s["trange"],
                        "level": self.s["level"],
                        "date": list( d for d in self.s.date_extremes()) if self.summary else self.s["date"],
                        "var": self.s["var"],
                        "val": self.s[self.s["var"]],
                    }

                features.append({
                    "type": "Feature",
                        "geometry": {
                            "type": "Point",
                            "coordinates": [self.s.get("lon"), self.s.get("lat")],
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

        return {
            "ident": self.s.get("ident"),
            "lon": self.s.key("lon").enqi(),
            "lat": self.s.key("lat").enqi(),
            "network": self.s["rep_memo"],
            "date": list( d for d in self.s.date_extremes()) if self.summary else self.s["date"],
            "data": [{
                "vars": {
                    self.s["var"]: {
                        "v": self.s[self.s["var"]]
                    }
                },
                "timerange": self.s["trange"],
                "level": self.s["level"],
            }]
        }

    def jsondictstation (self):

        return {
            "ident": self.s.get("ident"),
            "lon": self.s.key("lon").enqi(),
            "lat": self.s.key("lat").enqi(),
            "network": self.s["rep_memo"],
        }


def summaries(request, **kwargs):
    q = params2record(kwargs)
    q['year'] = kwargs.get('year')
    q['month'] = kwargs.get('month')
    q['day'] = kwargs.get('day')
    q["hour"] = kwargs.get("hour")

    q["yearmin"] = request.GET.get("yearmin")
    q["yearmax"] = request.GET.get("yearmax")
    q["monthmin"] = request.GET.get("monthmin")
    q["monthmax"] = request.GET.get("monthmax")
    q["daymin"] = request.GET.get("daymin")
    q["daymax"] = request.GET.get("daymax")
    q["hourmin"] = request.GET.get("hourmin")
    q["hourmax"] = request.GET.get("hourmax")
    q["minumin"] = request.GET.get("minumin")
    q["minumax"] = request.GET.get("minumax")
    q["secmin"] = request.GET.get("secmin")
    q["secmax"] = request.GET.get("secmax")


    format=kwargs.get('format')

    if format == "geojson" or format == "dbajson" :
        return JsonResponse(next(itertools.islice(dbajson(q,summary=True,format=format,dsn=request.GET.get('dsn', 'report')),0,None)),safe=False)

    if format == "jsonline" :
        return StreamingHttpResponse(dbajson(q,summary=True,format=format,dsn=request.GET.get('dsn', 'report')))

def timeseries(request, **kwargs):
    q = params2record(kwargs)
    q["year"] = kwargs["year"]
    q["month"] = kwargs.get("month")
    q["day"] = kwargs.get("day")
    q["hour"] = kwargs.get("hour")

    q["yearmin"] = request.GET.get("yearmin")
    q["yearmax"] = request.GET.get("yearmax")
    q["monthmin"] = request.GET.get("monthmin")
    q["monthmax"] = request.GET.get("monthmax")
    q["daymin"] = request.GET.get("daymin")
    q["daymax"] = request.GET.get("daymax")
    q["hourmin"] = request.GET.get("hourmin")
    q["hourmax"] = request.GET.get("hourmax")
    q["minumin"] = request.GET.get("minumin")
    q["minumax"] = request.GET.get("minumax")
    q["secmin"] = request.GET.get("secmin")
    q["secmax"] = request.GET.get("secmax")


    #https://codefisher.org/catch/blog/2015/04/22/python-how-group-and-count-dictionaries/
    #from collections import defaultdict
    #d = defaultdict(list)

    format=kwargs.get('format')

    if format == "geojson" or format == "dbajson" :
        return JsonResponse(next(itertools.islice(dbajson(q,format=format,dsn=request.GET.get('dsn', 'report')),0,None)),safe=False)

    if format == "jsonline" :
        return StreamingHttpResponse(dbajson(q,format=format,dsn=request.GET.get('dsn', 'report')))


def spatialseries(request, **kwargs):
    from datetime import datetime, timedelta
    q = params2record(kwargs)

    if kwargs.get("hour") is None:
        b = datetime(*(int(kwargs[k]) for k in ("year", "month", "day")))
        e = datetime(*(int(kwargs[k]) for k in ("year", "month", "day")),hour=23,minute=59,second=59)
    else:
        d = datetime(*(int(kwargs[k]) for k in ("year", "month", "day", "hour")))
        b = d - timedelta(seconds=1800)
        e = d + timedelta(seconds=1799)

    q["datemin"] = b
    q["datemax"] = e

    q["latmin"] = request.GET.get("latmin")
    q["latmax"] = request.GET.get("latmax")
    q["lonmin"] = request.GET.get("lonmin")
    q["lonmax"] = request.GET.get("lonmax")


    format=kwargs.get('format')

    if format == "geojson" or format == "dbajson" :
        return JsonResponse(next(itertools.islice(dbajson(q,format=format,dsn=request.GET.get('dsn', 'report')),0,None)),safe=False)

    if format == "jsonline" :
        return StreamingHttpResponse(dbajson(q,format=format,dsn=request.GET.get('dsn', 'report')))


def stationdata(request, **kwargs):
    q = params2record(kwargs)

    format=kwargs.get('format')

    if format == "geojson" or format == "dbajson" :
        return JsonResponse(next(itertools.islice(dbajson(q,format=format,dsn=request.GET.get('dsn', 'report')),0,None)),safe=False)

    if format == "jsonline" :
        return StreamingHttpResponse(dbajson(q,format=format,dsn=request.GET.get('dsn', 'report')))


def stations(request, **kwargs):
    q = params2record(kwargs)

    format=kwargs.get('format')

    if format == "geojson" or format == "dbajson" :
        return JsonResponse(next(itertools.islice(dbajson(q,stations=True,format=format,dsn=request.GET.get('dsn', 'report')),0,None)),safe=False)

    if format == "jsonline" :
        return StreamingHttpResponse(dbajson(q,stations=True,format=format,dsn=request.GET.get('dsn', 'report')))
