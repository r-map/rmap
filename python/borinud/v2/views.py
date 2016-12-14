# encoding: utf-8
# borinud/v2/views - v2 views for borinud
# Author: Emanuele Di Giacomo <emanueledigiacomo@gmail.com>

from django.http import JsonResponse
import json
from datetime import datetime

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


class jsonlines:

    def __init__(self, q, summary=False):
        self.q = q
        self.summary = summary

    def __iter__(self):
        if (self.summary):
            self.handle = get_db().query_summary(self.q)
        else:
            self.handle = get_db().query_data(self.q)

        return self.next()

    def next(self):
        for s in self.handle:

            # TODO !
            # per summary gestire:
            #    "lon": s.key("lon").enqi(),
            #    "lat": s.key("lat").enqi(),
            #    "date": [s["datemin"].isoformat(), s["datemax"].isoformat()],

            jsonline = json.dumps({
                "ident": s.get("ident"),
                "lon": s.key("lon").enqi(),
                "lat": s.key("lat").enqi(),
                "network": s["rep_memo"],
                "date": s["date"],
                "data": [{
                    "vars": {
                        s["var"]: {
                            "v": s[s["var"]]
                        }
                    },
                    "timerange": s["trange"],
                    "level": s["level"],
                }]
            }, default=json_serial)+"\n"

            yield jsonline


def summaries(request, **kwargs):
    q = params2record(kwargs)
    q['year'] = kwargs.get('year')
    q['month'] = kwargs.get('month')
    q['day'] = kwargs.get('day')

    return JsonResponse([j for j in jsonlines(q, summary=True)], safe=False)
    # return StreamingHttpResponse(jsonlines(q,summary=True))


def timeseries(request, **kwargs):
    q = params2record(kwargs)
    q["year"] = kwargs["year"]
    q["month"] = kwargs.get("month")
    q["day"] = kwargs.get("day")

    # https://codefisher.org/catch/blog/2015/04/22/python-how-group-and-count-dictionaries/
    # from collections import defaultdict
    # d = defaultdict(list)

    # return JsonResponse([j for j in jsonlines(q)],safe=False)
    return StreamingHttpResponse(jsonlines(q))


def spatialseries(request, **kwargs):
    from datetime import datetime, timedelta
    q = params2record(kwargs)
    d = datetime(*(int(kwargs[k]) for k in ("year", "month", "day", "hour")))
    b = d - timedelta(seconds=1799)
    e = d + timedelta(seconds=1799)
    q["datemin"] = b
    q["datemax"] = e

    return StreamingHttpResponse(jsonlines(q))


def stationdata(request, **kwargs):
    q = params2record(kwargs)

    # return JsonResponse([j for j in jsonlines(q)],safe=False)
    return StreamingHttpResponse(jsonlines(q))
