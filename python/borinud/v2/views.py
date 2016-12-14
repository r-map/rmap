# encoding: utf-8
# borinud/v2/views - v2 views for borinud
# Author: Emanuele Di Giacomo <emanueledigiacomo@gmail.com>

from datetime import datetime

from django.http import JsonResponse
from django.http import StreamingHttpResponse

from .utils import params2record
from ..utils.source import get_db


def json_serial(obj):
    """JSON serializer for objects not serializable by default json code"""
    if isinstance(obj, datetime):
        serial = obj.isoformat()
        return serial
    else:
        raise TypeError("Type not serializable")


class ResultIter(object):
    def __init__(self, q, summary=False):
        self.q = q
        self.summary = summary

    def __iter__(self):
        if self.summary:
            self.handle = get_db().query_summary(self.q)
        else:
            self.handle = get_db().query_data(self.q)

        return self.next()

    def next(self):
        from django.utils.timezone import utc
        for s in self.handle:
            yield {
                "ident": s.get("ident"),
                "lon": s.key("lon").enqi(),
                "lat": s.key("lat").enqi(),
                "network": s["rep_memo"],
                "date": list(utc.localize(d) for d in s.date_extremes())
                if self.summary else utc.localize(s["date"]),
                "data": [{
                    "vars": {
                        s["var"]: {
                            "v": s[s["var"]]
                        }
                    },
                    "timerange": s["trange"],
                    "level": s["level"],
                }]
            }


def summaries(request, **kwargs):
    q = params2record(kwargs)
    q['year'] = kwargs.get('year')
    q['month'] = kwargs.get('month')
    q['day'] = kwargs.get('day')

    return JsonResponse([j for j in ResultIter(q, summary=True)], safe=False)
    # return StreamingHttpResponse(ResultIter(q,summary=True))


def timeseries(request, **kwargs):
    q = params2record(kwargs)
    q["year"] = kwargs["year"]
    q["month"] = kwargs.get("month")
    q["day"] = kwargs.get("day")

    return JsonResponse([j for j in ResultIter(q)], safe=False)
    # return StreamingHttpResponse(ResultIter(q))


def spatialseries(request, **kwargs):
    from datetime import datetime, timedelta
    q = params2record(kwargs)
    d = datetime(*(int(kwargs[k]) for k in ("year", "month", "day", "hour")))
    b = d - timedelta(seconds=1799)
    e = d + timedelta(seconds=1799)
    q["datemin"] = b
    q["datemax"] = e

    return JsonResponse([j for j in ResultIter(q)], safe=False)
    # return StreamingHttpResponse(ResultIter(q))


def stationdata(request, **kwargs):
    q = params2record(kwargs)

    return JsonResponse([j for j in ResultIter(q)], safe=False)
    # return StreamingHttpResponse(ResultIter(q))
