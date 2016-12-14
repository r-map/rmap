# encoding: utf-8
# Author: Paolo Patruno <p.patruno@iperbole.bologna.it>

from django.shortcuts import render

def spatialseries(request, **kwargs):

    return render(request, 'showdata/spatialseries.html',{"ident":kwargs.get("ident"), "coords":kwargs.get("coords"), "network":kwargs.get("network"), "trange":kwargs.get("trange"), "level":kwargs.get("level"), "var":kwargs.get("var"), "year":kwargs.get("year"), "month":kwargs.get("month"), "day":kwargs.get("day"), "hour":kwargs.get("hour")})


def stationdata(request, **kwargs):

    return render(request, 'showdata/stationdata.html',{"ident":kwargs.get("ident"), "coords":kwargs.get("coords"), "network":kwargs.get("network"), "trange":kwargs.get("trange"), "level":kwargs.get("level"), "var":kwargs.get("var")})

