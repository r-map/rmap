# encoding: utf-8
# borinud/utils - utilities
# Author: Emanuele Di Giacomo <emanueledigiacomo@gmail.com>

from django.utils import timezone

import dballe

from .models import Source, Summary


def summary_slug(summ):
    if summ["ident"] is None:
        s = "fixed:{lon}:{lat}:{network}".format(**summ)
    else:
        s = "mobile:{ident}:{network}".format(**summ)

    s += ":" + ":".join(
        str(summ[k]) if summ[k] is not None else "-"
        for k in ["tr", "p1", "p2", "lt1", "lv1", "lt2", "lv2", "var"]
    )

    return s


def sync_source(source):
    syncfoo = {
        Source.DBALLE: sync_dballe_summary,
    }.get(source.connection_type)
    if syncfoo:
        for summ in syncfoo(source.connection):
            sync_summary(source, summ)


def sync_summary(source, summ):
    defaults = {
        k: summ[k] for k in [
            "network", "tr", "p1", "p2", "lt1", "lv1", "lt2", "lv2", "var",
            "datemin", "datemax",
        ]
    }
    if summ["ident"] is not None:
        defaults["ident"] = summ["ident"]
    else:
        defaults["lon"] = summ["lon"]
        defaults["lat"] = summ["lat"]

    defaults["lonmin"] = defaults["lonmax"] = summ["lon"]
    defaults["latmin"] = defaults["latmax"] = summ["lat"]

    smodel, created = Summary.objects.get_or_create(
        slug=summary_slug(summ),
        source=source,
        defaults=defaults,
    )

    if not created:
        smodel.datemin = min([smodel.datemin, summ["datemin"]])
        smodel.datemax = max([smodel.datemax, summ["datemax"]])
        smodel.lonmin = min([smodel.lonmin, summ["lon"]])
        smodel.latmin = min([smodel.latmin, summ["lat"]])
        smodel.lonmax = max([smodel.lonmax, summ["lon"]])
        smodel.latmax = max([smodel.latmax, summ["lat"]])

        smodel.save()


def sync_dballe_summary(dsn):
    db = dballe.DB.connect_from_url(dsn)
    for s in db.query_summary(dballe.Record(query="details")):
        yield dict(
            ident=s["ident"] if "ident" in s else None,
            network=s["rep_memo"],
            lon=s.key("lon").enqi(),
            lat=s.key("lat").enqi(),
            tr=s["trange"][0],
            p1=s["trange"][1],
            p2=s["trange"][2],
            lt1=s["level"][0],
            lv1=s["level"][1],
            lt2=s["level"][2],
            lv2=s["level"][3],
            var=s["var"],
            datemin=s["datemin"].replace(tzinfo=timezone.utc),
            datemax=s["datemax"].replace(tzinfo=timezone.utc),
        )
