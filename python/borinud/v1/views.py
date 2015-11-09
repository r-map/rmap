from django.http import JsonResponse

from ..models import Summary


def summaries(request, **kwargs):
    q = {
        k: kwargs[k]
        for k in [
            "ident", "lon", "lat", "network", "var", "tr", "p1", "p2", "lt1",
            "lv1", "lt2", "lv2"
        ]
        if k in kwargs
    }
    qs = Summary.objects.filter(**q)

    return JsonResponse({
        "type": "FeatureCollection",
        "features": [{
            "type": "Feature",
            "geometry": {
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
