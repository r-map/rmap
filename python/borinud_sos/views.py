from django.shortcuts import render

import dballe
import iso8601

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


def sos(request):
    service = request.GET["service"].lower()
    version = request.GET["version"].lower()
    request = request.GET["request"].lower()

    view = {
        ("sos", "1.0.0", "getcapabilities"): get_capabilities_1_0_0,
        ("sos", "1.0.0", "describesensor"): describe_sensor_1_0_0,
        ("sos", "1.0.0", "getobservation"): get_observation_1_0_0,
    }.get((service, version, request))

    return view(request)


def get_capabilities_1_0_0(request):
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


def describe_sensor_1_0_0(request):
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


def get_observation_1_0_0(request):
    """GetObservation for SOS 1.0.

    This implementation has some limitations:
    - responseFormat text/xml;subtype="om/1.0.0" only
    - resultModel om:Observation only
    - responseMode inline only
    - SRS is ignored
    - It supports the specific case of a single sensor offering (i.e. the
      procedure) for a fixed station (the observed property). Then, the
      offering has the same name of the procedure
    """
    db = get_db()
    # text/xml;subtype="om/1.0.0" only
    response_format = request.GET['responseFormat']
    # om:Observation only
    result_model = request.GET.get('resultModel')
    # inline response mode only
    response_mode = request.GET.get('responseMode')
    # SRS is ignored
    srs_name = request.GET.get("srsName")
    offering = request.GET['offering']
    # Procedure is ignored because the offering is the single sensor, i.e. the
    # procedure itself (so we overwrite it)
    procedure = request.GET.get("procedure")
    procedure = offering

    # observedProperty is mandatory, but it is ignored because there's only one
    # property for each offering/procedure
    observed_property = request.GET['observedProperty']

    # featureOfInterest is optional, but it is ignored because there's only
    # one feature for each offering/procedure, i.e. the station
    feature_of_interest = request.GET.get("featureOfInterest")

    # TODO: eventTime is optional and can be and instant (e.g.
    # 2009-06-26T10:00:00+01) or a period (e.g.
    # 2009-06-26T10:00:00+01/2009-06-26T11:00:00+01).
    event_time = request.GET.get("eventTime", None)

    rec = procedure_to_record(offering)

    if event_time is not None:
        if "/" in event_time:
            rec["datemin"] = iso8601.parse_date(event_time.split("/")[0])
            rec["datemax"] = iso8601.parse_date(event_time.split("/")[1])
        else:
            rec["date"] = iso8601.parse_date(event_time)

    feature_of_interest = summary_to_station(rec)

    cur = db.query_data(rec)

    values = [{
        "date": rec["date"].isoformat(),
        "value": rec[rec["var"]]],
    } for rec in cur]

    return render(request, "borinud_sos/xml/1.0/GetObservation.xml", {
        "lon": rec["lon"],
        "lat": rec["lat"],
        "procedure": procedure,
        "observed_property": observed_property,
        "feature_of_interest": feature_of_interest,
        "datemin": values[0]["date"] or "" if len(values) == 0,
        "datemax": values[-1]["date"] or "" len(values) == 0,
    })
