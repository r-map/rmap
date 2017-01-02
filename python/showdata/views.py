# encoding: utf-8
# Author: Paolo Patruno <p.patruno@iperbole.bologna.it>

from django.shortcuts import render
import dballe
from datetime import date,datetime,timedelta,time
from rmap.settings import *

def menu(request, **kwargs):

    now=datetime.utcnow()
    showdate=(now-timedelta(minutes=30))
    year='{:04d}'.format(showdate.year)
    month='{:02d}'.format(showdate.month)
    day='{:02d}'.format(showdate.day)
    hour='{:02d}'.format(showdate.hour)

####   define what to put on menu #############

    metad=BORINUD ["SOURCES"][0]["measurements"] 

    metadata=[]

    for mymeta in metad:

        meta={}

        if mymeta["level"] == "*":
            meta["leveltxt"]="All levels"
        else:
            meta["leveltxt"]=dballe.describe_level(*mymeta["level"])

        if mymeta["trange"]== "*":
            meta["trangetxt"]="All timeranges"
        else:
            meta["trangetxt"]=dballe.describe_trange(*mymeta["trange"])

        if mymeta["var"]== "*":
            meta["vartxt"]="All vars"
        else:
            varinfo=dballe.varinfo(mymeta["var"])
            meta["vartxt"]=varinfo.desc+" "+varinfo.unit

        meta["var"]=mymeta["var"]
        meta["level"]  = "%s,%s,%s,%s" % tuple(("-" if v is None else str(v) for v in mymeta["level"]))
        meta["trange"] = "%s,%s,%s" % tuple(("-" if v is None else str(v) for v in mymeta["trange"]))

        metadata.append(meta)

    return render(request, 'showdata/menu.html',{
        "ident":"*", "coords":"*", "network":"*",
        #"trange":trange, "level":level, "var":var,
        "metadata":metadata,
        "year":year,"month":month,"day":day,"hour":hour
    })


def timeseries(request, **kwargs):

    if kwargs.get("year"):
        if kwargs.get("month"):
            if kwargs.get("day"):
                if kwargs.get("hour"):
                    datefrom=kwargs.get("hour")+":00_"+kwargs.get("year")+kwargs.get("month")+kwargs.get("day")
                    dateuntil=kwargs.get("hour")+":59_"+kwargs.get("year")+kwargs.get("month")+kwargs.get("day")
                else:
                    datefrom="00:00_"+kwargs.get("year")+kwargs.get("month")+kwargs.get("day")
                    dateuntil="23:59_"+kwargs.get("year")+kwargs.get("month")+kwargs.get("day")
            else:
                datefrom="00:00_"+kwargs.get("year")+kwargs.get("month")+"01"
                dateuntil="23:59_"+kwargs.get("year")+kwargs.get("month")+"31"
        else:
            datefrom="00:00_"+kwargs.get("year")+"0101"
            dateuntil="23:59_"+kwargs.get("year")+"1231"
    else:
        datefrom=""
        dateuntil=""

    if kwargs.get("level")== "*":
        leveltxt="All levels"
    else:
        leveltxt=dballe.describe_level(*[None if v == "-" else int(v) for v in kwargs.get("level").split(",")])

    if kwargs.get("trange")== "*":
        trangetxt="All timeranges"
    else:
        trangetxt=dballe.describe_trange(*[None if v == "-" else int(v) for v in kwargs.get("trange").split(",")])

    if kwargs.get("var")== "*":
        vartxt="All vars"
    else:
        varinfo=dballe.varinfo(kwargs.get("var"))
        vartxt=varinfo.desc+" "+varinfo.unit

    return render(request, 'showdata/timeseries.html',{"ident":kwargs.get("ident"),"coords":kwargs.get("coords"), "undescored_coords":kwargs.get("coords").replace(",","_"), "network":kwargs.get("network"), "trange":kwargs.get("trange"), "undescored_trange":kwargs.get("trange").replace(",","_"), "level":kwargs.get("level"), "undescored_level":kwargs.get("level").replace(",","_"), "var":kwargs.get("var"), "year":kwargs.get("year"), "month":kwargs.get("month"), "day":kwargs.get("day"),"datefrom":datefrom,"dateuntil":dateuntil, "vartxt":vartxt, "trangetxt":trangetxt, "leveltxt":leveltxt})

def spatialseries(request, **kwargs):

    if kwargs.get("year"):
        if kwargs.get("month"):
            if kwargs.get("day"):
                if kwargs.get("hour"):
                    datefrom=kwargs.get("hour")+":00_"+kwargs.get("year")+kwargs.get("month")+kwargs.get("day")
                    dateuntil=kwargs.get("hour")+":59_"+kwargs.get("year")+kwargs.get("month")+kwargs.get("day")
                else:
                    datefrom="00:00_"+kwargs.get("year")+kwargs.get("month")+kwargs.get("day")
                    dateuntil="23:59_"+kwargs.get("year")+kwargs.get("month")+kwargs.get("day")
            else:
                datefrom="00:00_"+kwargs.get("year")+kwargs.get("month")+"01"
                dateuntil="23:59_"+kwargs.get("year")+kwargs.get("month")+"31"
        else:
            datefrom="00:00_"+kwargs.get("year")+"0101"
            dateuntil="23:59_"+kwargs.get("year")+"1231"
    else:
        datefrom=""
        dateuntil=""

    if kwargs.get("level")== "*":
        leveltxt="All levels"
    else:
        leveltxt=dballe.describe_level(*[None if v == "-" else int(v) for v in kwargs.get("level").split(",")])

    if kwargs.get("trange")== "*":
        trangetxt="All timeranges"
    else:
        trangetxt=dballe.describe_trange(*[None if v == "-" else int(v) for v in kwargs.get("trange").split(",")])

    if kwargs.get("var")== "*":
        vartxt="All vars"
    else:
        varinfo=dballe.varinfo(kwargs.get("var"))
        vartxt=varinfo.desc+" "+varinfo.unit

    return render(request, 'showdata/spatialseries.html',{"ident":kwargs.get("ident"), "coords":kwargs.get("coords"), "network":kwargs.get("network"), "trange":kwargs.get("trange"), "level":kwargs.get("level"), "var":kwargs.get("var"), "year":kwargs.get("year"), "month":kwargs.get("month"), "day":kwargs.get("day"), "hour":kwargs.get("hour"), "vartxt":vartxt, "trangetxt":trangetxt, "leveltxt":leveltxt,"datefrom":datefrom,"dateuntil":dateuntil})


def stationdata(request, **kwargs):

    return render(request, 'showdata/stationdata.html',{"ident":kwargs.get("ident"), "coords":kwargs.get("coords"), "network":kwargs.get("network"), "trange":kwargs.get("trange"), "level":kwargs.get("level"), "var":kwargs.get("var")})

