from django.shortcuts import render

import dballe

from borinud.utils.source import get_db


def summary_to_procedure(summ):
    ident = summ["ident"] if summ["ident"] is not None else "-"
    lon = summ.get("lon").enqd()
    lat = summ.get("lat").enqd()
    rep = summ["rep_memo"]
    trange = [v if v is not None else "-" for v in summ["trange"]]
    level = [v if v is not None else "-" for v in summ["level"]]
    var = summ["var"]
    return "urn:rmap:procedure:{ident}/{lon},{lat}/{rep}/{trange}/{level}/{var}".format(
        ident=ident, lon=lon, lat=lat, rep=rep,
        trange=",".join(trange), level=",".join(level), var=var
    )


def summary_to_observed_property(summ):
    trange = [v if v is not None else "-" for v in summ["trange"]]
    level = [v if v is not None else "-" for v in summ["level"]]
    var = summ["var"]
    return "urn:rmap:property:{trange}/{level}/{var}".format(
        trange = [v if v is not None else "-" for v in summ["trange"]]
    )


def get_capabilities_1_0(request):
    """GetCapabilities for SOS 1.0"""
    # TODO: all dbs should merged in one
    db = get_db()
    summaries = list(db.query_summaries(dballe.Record()))
    observed_properties = set(summary_to_observed_property(s) for s in summaries)
    return render(request, "borinud_sos/xml/1.0/GetCapabilities.xml", {
        "sos_full_url": request.build_absolute_uri(),
        "observed_properties": observed_properties,
        "offerings": [{
            "id": summary_to_procedure(s),
            "description": "",
            # TODO: mobile stations should be merged and bbox should be
            # calculated.
            "bbox": [[s["lon"], s["lat"]], [s["lon"], s["lat"]]],
            "date": s.date_extremes(),
            "observed_property": summary_to_observed_property(s),
        } for s in summaries]
    })
