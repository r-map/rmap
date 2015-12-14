# encoding: utf-8
# borinud/v1/views - v1 views for borinud
# Author: Emanuele Di Giacomo <emanueledigiacomo@gmail.com>

from django.http import JsonResponse

from ..models import Summary
from .utils import path2query


def summaries(request, **kwargs):
    q = path2query(kwargs)
    qs = Summary.objects.filter(**q)

    return JsonResponse({
        "type": "FeatureCollection",
        "features": [{
            "type": "Feature",
            "geometry": {
                "type": "Point",
                "coordinates": [s.lon, s.lat]
            }
            if s.lonmin == s.lonmax and s.latmin == s.latmax
            else {
                "type": "Polygon",
                "coordinates": [[s.lonmin, s.latmin], [s.lonmax, s.latmin],
                                [s.lonmax, s.latmax], [s.lonmin, s.latmax],
                                [s.lonmin, s.latmin]]
            },
            "properties": {
                "ident": s.ident,
                "lon": s.lon,
                "lat": s.lat,
                "network": s.network,
                "trange_pind": s.tr,
                "trange_p1": s.p1,
                "trange_p2": s.p2,
                "level_t1": s.lt1,
                "level_v1": s.lv1,
                "level_t2": s.lt2,
                "level_v2": s.lv2,
                "bcode": s.var,
                "datetime": [s.datemin, s.datemax],
            }
        } for s in qs]
    })
