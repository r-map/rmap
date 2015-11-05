from django.http import JsonResponse


def summaries(request, ident=None, lon=None, lat=None, network=None,
              trange_pind=None, trange_p1=None, trange_p2=None,
              level_t1=None, level_v1=None, level_t2=None, level_v2=None):
    return JsonResponse({
        "error": "not yet implemented",
        "params": {
            "ident": ident,
            "lon": lon,
            "lat": lat,
            "network": network,
        },
    }, status=400)
