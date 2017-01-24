# encoding: utf-8
# Author: Paolo Patruno <p.patruno@iperbole.bologna.it>

from django.shortcuts import render
import dballe
from datetime import date,datetime,timedelta,time
from rmap.settings import *
from django.core.urlresolvers import reverse

def menu(request, **kwargs):

    now=datetime.utcnow()
    showdate=(now-timedelta(minutes=30))
    year='{:04d}'.format(showdate.year)
    month='{:02d}'.format(showdate.month)
    day='{:02d}'.format(showdate.day)
    hour='{:02d}'.format(showdate.hour)

####   define what to put on menu #############

    metadata=[]

    for mymeta in measurements:

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
                    #HOURLY
                    timerequested=datetime(year=int(kwargs.get("year")), month=int(kwargs.get("month")), day=int(kwargs.get("day")), hour=int(kwargs.get("hour")))
                    delta=timedelta(hours=1)
                    dtprevious = timerequested - delta
                    dtnext     = timerequested + delta
                    previous = reverse('showdata:timeserieshourly', kwargs={
                        "ident":kwargs.get("ident"),
                        "coords":kwargs.get("coords"), 
                        "network":kwargs.get("network"), 
                        "trange":kwargs.get("trange"),
                        "level":kwargs.get("level"),
                        "var":kwargs.get("var"),
                        "year" :"{:04d}".format(dtprevious.year),
                        "month":"{:02d}".format(dtprevious.month),
                        "day"  :"{:02d}".format(dtprevious.day),
                        "hour" :"{:02d}".format(dtprevious.hour)})
                    next= reverse('showdata:timeserieshourly', kwargs={
                        "ident":kwargs.get("ident"),
                        "coords":kwargs.get("coords"), 
                        "network":kwargs.get("network"), 
                        "trange":kwargs.get("trange"),
                        "level":kwargs.get("level"),
                        "var":kwargs.get("var"),
                        "year" :"{:04d}".format(dtnext.year),
                        "month":"{:02d}".format(dtnext.month),
                        "day"  :"{:02d}".format(dtnext.day),
                        "hour" :"{:02d}".format(dtnext.hour)})
                    more=reverse('showdata:timeseriesdaily', kwargs={
                        "ident":kwargs.get("ident"),
                        "coords":kwargs.get("coords"), 
                        "network":kwargs.get("network"), 
                        "trange":kwargs.get("trange"),
                        "level":kwargs.get("level"),
                        "var":kwargs.get("var"),
                        "year":kwargs.get("year"),
                        "month":kwargs.get("month"),
                        "day":kwargs.get("day")})
                    less=None
                    datefrom=kwargs.get("hour")+":00_"+kwargs.get("year")+kwargs.get("month")+kwargs.get("day")
                    dateuntil=kwargs.get("hour")+":59_"+kwargs.get("year")+kwargs.get("month")+kwargs.get("day")
                else:
                    #DAILY
                    timerequested=datetime(year=int(kwargs.get("year")), month=int(kwargs.get("month")), day=int(kwargs.get("day")))
                    delta=timedelta(days=1)
                    dtprevious = timerequested - delta
                    dtnext     = timerequested + delta
                    previous = reverse('showdata:timeseriesdaily', kwargs={
                        "ident":kwargs.get("ident"),
                        "coords":kwargs.get("coords"), 
                        "network":kwargs.get("network"), 
                        "trange":kwargs.get("trange"),
                        "level":kwargs.get("level"),
                        "var":kwargs.get("var"),
                        "year" :"{:04d}".format(dtprevious.year),
                        "month":"{:02d}".format(dtprevious.month),
                        "day"  :"{:02d}".format(dtprevious.day)})
                    next= reverse('showdata:timeseriesdaily', kwargs={
                        "ident":kwargs.get("ident"),
                        "coords":kwargs.get("coords"), 
                        "network":kwargs.get("network"), 
                        "trange":kwargs.get("trange"),
                        "level":kwargs.get("level"),
                        "var":kwargs.get("var"),
                        "year" :"{:04d}".format(dtnext.year),
                        "month":"{:02d}".format(dtnext.month),
                        "day"  :"{:02d}".format(dtnext.day)})
                    more=reverse('showdata:timeseriesmonthly', kwargs={
                        "ident":kwargs.get("ident"),
                        "coords":kwargs.get("coords"), 
                        "network":kwargs.get("network"), 
                        "trange":kwargs.get("trange"),
                        "level":kwargs.get("level"),
                        "var":kwargs.get("var"),
                        "year":kwargs.get("year"),
                        "month":kwargs.get("month")})
                    less=reverse('showdata:timeserieshourly', kwargs={
                        "ident":kwargs.get("ident"),
                        "coords":kwargs.get("coords"), 
                        "network":kwargs.get("network"), 
                        "trange":kwargs.get("trange"),
                        "level":kwargs.get("level"),
                        "var":kwargs.get("var"),
                        "year":kwargs.get("year"),
                        "month":kwargs.get("month"),
                        "day":kwargs.get("day"),
                        "hour":"12"})
                    datefrom="00:00_"+kwargs.get("year")+kwargs.get("month")+kwargs.get("day")
                    dateuntil="23:59_"+kwargs.get("year")+kwargs.get("month")+kwargs.get("day")
            else:
                #MONTHLY
                timerequested=datetime(year=int(kwargs.get("year")), month=int(kwargs.get("month")),day=15)
                delta=timedelta(days=30)
                dtprevious = timerequested - delta
                dtnext     = timerequested + delta
                previous = reverse('showdata:timeseriesmonthly', kwargs={
                    "ident":kwargs.get("ident"),
                    "coords":kwargs.get("coords"), 
                    "network":kwargs.get("network"), 
                    "trange":kwargs.get("trange"),
                    "level":kwargs.get("level"),
                    "var":kwargs.get("var"),
                    "year" :"{:04d}".format(dtprevious.year),
                    "month":"{:02d}".format(dtprevious.month)})
                next= reverse('showdata:timeseriesmonthly', kwargs={
                    "ident":kwargs.get("ident"),
                    "coords":kwargs.get("coords"), 
                    "network":kwargs.get("network"), 
                    "trange":kwargs.get("trange"),
                    "level":kwargs.get("level"),
                    "var":kwargs.get("var"),
                    "year" :"{:04d}".format(dtnext.year),
                    "month":"{:02d}".format(dtnext.month)})
                more=reverse('showdata:timeseriesyearly', kwargs={
                    "ident":kwargs.get("ident"),
                    "coords":kwargs.get("coords"), 
                    "network":kwargs.get("network"), 
                    "trange":kwargs.get("trange"),
                    "level":kwargs.get("level"),
                    "var":kwargs.get("var"),
                    "year":kwargs.get("year")})
                less=reverse('showdata:timeseriesdaily', kwargs={
                    "ident":kwargs.get("ident"),
                    "coords":kwargs.get("coords"), 
                    "network":kwargs.get("network"), 
                    "trange":kwargs.get("trange"),
                    "level":kwargs.get("level"),
                    "var":kwargs.get("var"),
                    "year":kwargs.get("year"),
                    "month":kwargs.get("month"),
                    "day":"15"})
                datefrom="00:00_"+kwargs.get("year")+kwargs.get("month")+"01"
                lastdayinmonth="{:02d}".format((dtnext.replace(day=1)-timedelta(days=1)).day)
                dateuntil="23:59_"+kwargs.get("year")+kwargs.get("month")+lastdayinmonth
        else:
            #YEARLY
            timerequested=datetime(year=int(kwargs.get("year")),month=6,day=15)
            delta=timedelta(days=30*12)
            dtprevious = timerequested - delta
            dtnext     = timerequested + delta
            previous = reverse('showdata:timeseriesyearly', kwargs={
                "ident":kwargs.get("ident"),
                "coords":kwargs.get("coords"), 
                "network":kwargs.get("network"), 
                "trange":kwargs.get("trange"),
                "level":kwargs.get("level"),
                "var":kwargs.get("var"),
                "year" :"{:04d}".format(dtprevious.year)})
            next= reverse('showdata:timeseriesyearly', kwargs={
                "ident":kwargs.get("ident"),
                "coords":kwargs.get("coords"), 
                "network":kwargs.get("network"), 
                "trange":kwargs.get("trange"),
                "level":kwargs.get("level"),
                "var":kwargs.get("var"),
                "year" :"{:04d}".format(dtnext.year)})
            more=None
            less=reverse('showdata:timeseriesmonthly', kwargs={
                "ident":kwargs.get("ident"),
                "coords":kwargs.get("coords"), 
                "network":kwargs.get("network"), 
                "trange":kwargs.get("trange"),
                "level":kwargs.get("level"),
                "var":kwargs.get("var"),
                "year":kwargs.get("year"),
                "month":"06"})
            datefrom="00:00_"+kwargs.get("year")+"0101"
            dateuntil="23:59_"+kwargs.get("year")+"1231"
    else:
        #WRONG
        previous=None
        next=None
        less=None
        more=None
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

    return render(request, 'showdata/timeseries.html',{
        "ident":kwargs.get("ident"),"coords":kwargs.get("coords"), 
        "undescored_coords":kwargs.get("coords").replace(",","_"), "network":kwargs.get("network"), 
        "trange":kwargs.get("trange"), "undescored_trange":kwargs.get("trange").replace(",","_"), 
        "level":kwargs.get("level"), "undescored_level":kwargs.get("level").replace(",","_"), "var":kwargs.get("var"), 
        "year":kwargs.get("year"), "month":kwargs.get("month"), "day":kwargs.get("day"),
        "datefrom":datefrom,"dateuntil":dateuntil, 
        "vartxt":vartxt, "trangetxt":trangetxt, "leveltxt":leveltxt,
        "previous":previous,"next":next,"less":less,"more":more,"type":request.GET.get('type', 'fixed')})

def spatialseries(request, **kwargs):

    if kwargs.get("year"):
        if kwargs.get("month"):
            if kwargs.get("day"):
                if kwargs.get("hour"):
                    #HOURLY
                    timerequested=datetime(year=int(kwargs.get("year")), month=int(kwargs.get("month")), day=int(kwargs.get("day")), hour=int(kwargs.get("hour")))
                    delta=timedelta(hours=1)
                    dtprevious = timerequested - delta
                    dtnext     = timerequested + delta
                    previous = reverse('showdata:spatialserieshourly', kwargs={
                        "ident":kwargs.get("ident"),
                        "coords":kwargs.get("coords"), 
                        "network":kwargs.get("network"), 
                        "trange":kwargs.get("trange"),
                        "level":kwargs.get("level"),
                        "var":kwargs.get("var"),
                        "year" :"{:04d}".format(dtprevious.year),
                        "month":"{:02d}".format(dtprevious.month),
                        "day"  :"{:02d}".format(dtprevious.day),
                        "hour" :"{:02d}".format(dtprevious.hour)})
                    next= reverse('showdata:spatialserieshourly', kwargs={
                        "ident":kwargs.get("ident"),
                        "coords":kwargs.get("coords"), 
                        "network":kwargs.get("network"), 
                        "trange":kwargs.get("trange"),
                        "level":kwargs.get("level"),
                        "var":kwargs.get("var"),
                        "year" :"{:04d}".format(dtnext.year),
                        "month":"{:02d}".format(dtnext.month),
                        "day"  :"{:02d}".format(dtnext.day),
                        "hour" :"{:02d}".format(dtnext.hour)})
                    more=reverse('showdata:spatialseriesdaily', kwargs={
                        "ident":kwargs.get("ident"),
                        "coords":kwargs.get("coords"), 
                        "network":kwargs.get("network"), 
                        "trange":kwargs.get("trange"),
                        "level":kwargs.get("level"),
                        "var":kwargs.get("var"),
                        "year":kwargs.get("year"),
                        "month":kwargs.get("month"),
                        "day":kwargs.get("day")})
                    less=None
                    datefrom=kwargs.get("hour")+":00_"+kwargs.get("year")+kwargs.get("month")+kwargs.get("day")
                    dateuntil=kwargs.get("hour")+":59_"+kwargs.get("year")+kwargs.get("month")+kwargs.get("day")
                else:
                    #DAILY
                    timerequested=datetime(year=int(kwargs.get("year")), month=int(kwargs.get("month")), day=int(kwargs.get("day")))
                    delta=timedelta(days=1)
                    dtprevious = timerequested - delta
                    dtnext     = timerequested + delta
                    previous = reverse('showdata:spatialseriesdaily', kwargs={
                        "ident":kwargs.get("ident"),
                        "coords":kwargs.get("coords"), 
                        "network":kwargs.get("network"), 
                        "trange":kwargs.get("trange"),
                        "level":kwargs.get("level"),
                        "var":kwargs.get("var"),
                        "year" :"{:04d}".format(dtprevious.year),
                        "month":"{:02d}".format(dtprevious.month),
                        "day"  :"{:02d}".format(dtprevious.day)})
                    next= reverse('showdata:spatialseriesdaily', kwargs={
                        "ident":kwargs.get("ident"),
                        "coords":kwargs.get("coords"), 
                        "network":kwargs.get("network"), 
                        "trange":kwargs.get("trange"),
                        "level":kwargs.get("level"),
                        "var":kwargs.get("var"),
                        "year" :"{:04d}".format(dtnext.year),
                        "month":"{:02d}".format(dtnext.month),
                        "day"  :"{:02d}".format(dtnext.day)})
                    more=None
                    less=reverse('showdata:spatialserieshourly', kwargs={
                        "ident":kwargs.get("ident"),
                        "coords":kwargs.get("coords"), 
                        "network":kwargs.get("network"), 
                        "trange":kwargs.get("trange"),
                        "level":kwargs.get("level"),
                        "var":kwargs.get("var"),
                        "year":kwargs.get("year"),
                        "month":kwargs.get("month"),
                        "day":kwargs.get("day"),
                        "hour":"12"})
                    datefrom="00:00_"+kwargs.get("year")+kwargs.get("month")+kwargs.get("day")
                    dateuntil="23:59_"+kwargs.get("year")+kwargs.get("month")+kwargs.get("day")
            else:
                #MONTHLY
                #WRONG
                previous=None
                next=None
                less=None
                more=None
                datefrom=""
                dateuntil=""
        else:
            #YEARLY
            #WRONG
            previous=None
            next=None
            less=None
            more=None
            datefrom=""
            dateuntil=""
    else:
        #WRONG
        previous=None
        next=None
        less=None
        more=None
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
    
    return render(request, 'showdata/spatialseries.html',{
        "ident":kwargs.get("ident"), "coords":kwargs.get("coords"), 
        "network":kwargs.get("network"), "trange":kwargs.get("trange"), 
        "level":kwargs.get("level"), "var":kwargs.get("var"), 
        "year":kwargs.get("year"), "month":kwargs.get("month"), "day":kwargs.get("day"), 
        "hour":kwargs.get("hour"), 
        "vartxt":vartxt, "trangetxt":trangetxt, "leveltxt":leveltxt,
        "datefrom":datefrom,"dateuntil":dateuntil,
        "previous":previous,"next":next,"less":less,"more":more})
    
def stations(request, **kwargs):

    return render(request, 'showdata/stations.html',{"ident":kwargs.get("ident"), "coords":kwargs.get("coords"), "network":kwargs.get("network"), "trange":kwargs.get("trange"), "level":kwargs.get("level"), "var":kwargs.get("var")})

