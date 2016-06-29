# encoding: utf-8
# borinud/v1/views - v1 views for borinud
# Author: Emanuele Di Giacomo <emanueledigiacomo@gmail.com>

from django.http import JsonResponse

from ..settings import BORINUD
from .utils import params2record
from ..utils.source import get_db


def summaries(request, **kwargs):
    q = params2record(kwargs)
    q['year'] = kwargs.get('year')
    q['month'] = kwargs.get('month')
    q['day'] = kwargs.get('day')
    return JsonResponse({
        "type": "FeatureCollection",
        "features": [{
            "geometry": {
                "type": "Point",
                "coordinates": [s.get("lon"), s.get("lat")],
            },
            "properties": {
                "ident": s.get("ident"),
                "lon": s.key("lon").enqi(),
                "lat": s.key("lat").enqi(),
                "network": s["rep_memo"],
                "trange": s["trange"],
                "level": s["level"],
                "date": [s["datemin"].isoformat(), s["datemax"].isoformat()],
                "var": s["var"],
            },
        } for s in get_db().query_summary(q)],
    })


def timeseries(request, **kwargs):
    q = params2record(kwargs)
    q["year"] = kwargs["year"]
    q["month"] = kwargs.get("month")
    q["day"] = kwargs.get("day")

    print [{
        "var": s["var"],
        "val": s[s["var"]],
    } for s in get_db().query_data(q)]

    #https://codefisher.org/catch/blog/2015/04/22/python-how-group-and-count-dictionaries/
    #from collections import defaultdict
    #d = defaultdict(list)

    return JsonResponse(
        [
            {
                "ident": s.get("ident"),
                "lon": s.key("lon").enqi(),
                "lat": s.key("lat").enqi(),
                "network": s["rep_memo"],
                "date": s["date"],
                "data":
                {"vars":
                 [
                     {
                         s["var"]:s[s["var"]],
                     }
                 ],
                 "timerange": s["trange"],
                 "level": s["level"],
                }
            } for s in get_db().query_data(q)
        ] 
        ,safe=False)

def spatialseries(request, **kwargs):
    from datetime import datetime, timedelta
    q = params2record(kwargs)
    d = datetime(*(int(kwargs[k]) for k in ("year", "month", "day", "hour")))
    b = d - timedelta(seconds=1799)
    e = d + timedelta(seconds=1799)
    q["datemin"] = b
    q["datemax"] = e
    return JsonResponse({
        "type": "FeatureCollection",
        "features": [{
            "geometry": {
                "type": "Point",
                "coordinates": [s.get("lon"), s.get("lat")],
            },
            "properties": {
                "ident": s.get("ident"),
                "lon": s.key("lon").enqi(),
                "lat": s.key("lat").enqi(),
                "network": s["rep_memo"],
                "trange": s["trange"],
                "level": s["level"],
                "date": s["date"],
                "var": s["var"],
                "val": s[s["var"]],
            },
        } for s in get_db().query_data(q)],
    })


def stationdata(request, **kwargs):
    q = params2record(kwargs)
    return JsonResponse({
        "type": "FeatureCollection",
        "features": [{
            "geometry": {
                "type": "Point",
                "coordinates": [s.get("lon"), s.get("lat")],
            },
            "properties": {
                "ident": s.get("ident"),
                "lon": s.key("lon").enqi(),
                "lat": s.key("lat").enqi(),
                "network": s["rep_memo"],
                "trange": [None, None, None],
                "level": [None, None, None],
                "var": s["var"],
                "val": s[s["var"]],
            },
        } for s in get_db().query_stations(q)],
    })
