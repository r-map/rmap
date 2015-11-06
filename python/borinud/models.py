# encoding: utf-8
# borinud/models - models for borinud
# Author: Emanuele Di Giacomo <emanueledigiacomo@gmail.com>

from django.db import models


class Source(models.Model):
    DBALLE = 1
    ARKIMET_BUFR = 2
    ARKIMET_VM2 = 3

    CONNECTION_TYPE_SOURCES = (
        (DBALLE, "dballe"),
        (ARKIMET_BUFR, "arkimet bufr"),
        (ARKIMET_VM2, "arkimet vm2"),
    )
    connection_type = models.IntegerField(choices=CONNECTION_TYPE_SOURCES)
    connection = models.CharField(max_length=512)


class Summary(models.Model):
    """Data summary."""
    slug = models.CharField(max_length=512, primary_key=True)
    ident = models.CharField(max_length=9, blank=True, null=True)
    network = models.CharField(max_length=16)
    lon = models.IntegerField(null=True)
    lat = models.IntegerField(null=True)
    tr = models.IntegerField(null=True)
    p1 = models.IntegerField(null=True)
    p2 = models.IntegerField(null=True)
    lt1 = models.IntegerField(null=True)
    lv1 = models.IntegerField(null=True)
    lt2 = models.IntegerField(null=True)
    lv2 = models.IntegerField(null=True)
    var = models.CharField(max_length=6, blank=True, null=True)
    lonmin = models.IntegerField(null=True)
    latmin = models.IntegerField(null=True)
    lonmax = models.IntegerField(null=True)
    latmax = models.IntegerField(null=True)
    datemin = models.DateTimeField(blank=True, null=True)
    datemax = models.DateTimeField(blank=True, null=True)
    source = models.ForeignKey(Source)
    timestamp = models.DateTimeField(auto_now=True)
