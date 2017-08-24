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


def summary_to_station(summ):
    ident = summ["ident"] if summ["ident"] is not None else "-"
    lon = summ.get("lon").enqd()
    lat = summ.get("lat").enqd()
    rep = summ["rep_memo"]
    return "urn:rmap:station:{ident}/{lon},{lat}/{rep}".format(
        ident=ident, lon=lon, lat=lat, rep=rep,
    )


def procedure_to_record(procedure):
    import re
    reg = re.compile((
        r'urn:rmap:procedure:'
        r'(?P<ident>.*)/'
        r'(?P<lon>[0-9]+),(?P<lat>[0-9]+)/'
        r'(?P<rep>.*)/'
        r'(?P<pind>[0-9]+),(?P<p1>[0-9]+),(?P<p2>[0-9]+)/'
        r'(?P<leveltype1>[0-9]+|-),(?P<l1>[0-9]+|-)/'
        r'(?P<leveltype2>[0-9]+|-),(?P<l2>[0-9]+|-)/'
        r'(?P<var>B[0-9]{5})'
    ))
    res = reg.match(procedure)
    if res is not None:
        return dballe.Record(**res.groupdict())
    else:
        return None


def get_capabilities_1_0(request):
    """GetCapabilities for SOS 1.0.

    This implementation has some limitations:
    1. It reads the default db only.
    2. The stations are considered fixed (mobile stations should be grouped
       by ident and rep_memo and coordinates should be collapsed in a bbox).
    3. Each observation offerings has one procedure only (the sensor) and the
       feature of interest is the station.
    4. The offering description is static (could be the station name).
    """
    db = get_db()
    summaries = list(db.query_summaries(dballe.Record()))
    observed_properties = set(summary_to_observed_property(s) for s in summaries)
    return render(request, "borinud_sos/xml/1.0/GetCapabilities.xml", {
        "sos_full_url": request.build_absolute_uri(),
        "observed_properties": observed_properties,
        "offerings": [{
            "id": summary_to_procedure(s),
            "description": "Sensor offering",
            "bbox": [[s["lon"], s["lat"]], [s["lon"], s["lat"]]],
            "date": s.date_extremes(),
            "observed_property": summary_to_observed_property(s),
            "feature_of_interest": summary_to_station(s),
        } for s in summaries]
    })


def describe_sensor_1_0(request):
    """DescribeSensor for SOS 1.0."""
    db = get_db()
    procedure = request.GET['procedure']
    rec = procedure_to_record(procedure)
    cur = db.query_stations(rec)
    sensor = next(db.query_summaries(rec))
    return render(request, "borinud_sos/xml/1.0/DescribeSensor.xml", {
        "name": procedure,
        "lon": sensor["lon"],
        "lat": sensor["lat"],
    })
