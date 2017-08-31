# encoding: utf-8
# Author: Paolo Patruno <p.patruno@iperbole.bologna.it>

#TODO: make datefrom and dateuntil using GET parametere *min *max

from django.shortcuts import render
import dballe
from datetime import date,datetime,timedelta,time
from rmap.settings import *
from django.core.urlresolvers import reverse
from rmap.stations.models import Bcode
#from rmap.rmap_core import isRainboInstance

defaultdsn="report"
defaulttimedsn="report_fixed"

def filtro(request, **kwargs):

    now=datetime.utcnow()
    showdate=(now-timedelta(minutes=30))

    year='{:04d}'.format(showdate.year)
    month='{:02d}'.format(showdate.month)
    day='{:02d}'.format(showdate.day)
    hour='{:02d}'.format(showdate.hour)

    return render(request, 'showdata/filtro.html',{
        "year":year,"month":month,"day":day,"hour":hour
    })


def menu(request, **kwargs):

    now=datetime.utcnow()
    showdate=(now-timedelta(minutes=30))
    year='{:04d}'.format(showdate.year)
    month='{:02d}'.format(showdate.month)
    day='{:02d}'.format(showdate.day)
    hour='{:02d}'.format(showdate.hour)

####   define what to put on menu #############

    metadata=[]

    for mymeta in report_measurements:

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
            #meta["vartxt"]=varinfo.desc+" "+varinfo.unit
            meta["vartxt"]=varinfo.desc

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


def rainbotimeseries(request, **kwargs):

    return timeseries(request,html_template="showdata/rainbotimeseries.html",**kwargs)


def timeseries(request,html_template="showdata/timeseries.html", **kwargs):

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
                        "hour" :"{:02d}".format(dtprevious.hour)})+"?dsn="+request.GET.get('dsn', defaulttimedsn)
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
                        "hour" :"{:02d}".format(dtnext.hour)})+"?dsn="+request.GET.get('dsn', defaulttimedsn)
                    more=reverse('showdata:timeseriesdaily', kwargs={
                        "ident":kwargs.get("ident"),
                        "coords":kwargs.get("coords"), 
                        "network":kwargs.get("network"), 
                        "trange":kwargs.get("trange"),
                        "level":kwargs.get("level"),
                        "var":kwargs.get("var"),
                        "year":kwargs.get("year"),
                        "month":kwargs.get("month"),
                        "day":kwargs.get("day")})+"?dsn="+request.GET.get('dsn', defaulttimedsn)
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
                        "day"  :"{:02d}".format(dtprevious.day)})+"?dsn="+request.GET.get('dsn', defaulttimedsn)
                    next= reverse('showdata:timeseriesdaily', kwargs={
                        "ident":kwargs.get("ident"),
                        "coords":kwargs.get("coords"), 
                        "network":kwargs.get("network"), 
                        "trange":kwargs.get("trange"),
                        "level":kwargs.get("level"),
                        "var":kwargs.get("var"),
                        "year" :"{:04d}".format(dtnext.year),
                        "month":"{:02d}".format(dtnext.month),
                        "day"  :"{:02d}".format(dtnext.day)})+"?dsn="+request.GET.get('dsn', defaulttimedsn)
                    more=reverse('showdata:timeseriesmonthly', kwargs={
                        "ident":kwargs.get("ident"),
                        "coords":kwargs.get("coords"), 
                        "network":kwargs.get("network"), 
                        "trange":kwargs.get("trange"),
                        "level":kwargs.get("level"),
                        "var":kwargs.get("var"),
                        "year":kwargs.get("year"),
                        "month":kwargs.get("month")})+"?dsn="+request.GET.get('dsn', defaulttimedsn)
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
                        "hour":"12"})+"?dsn="+request.GET.get('dsn', defaulttimedsn)
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
                    "month":"{:02d}".format(dtprevious.month)})+"?dsn="+request.GET.get('dsn', defaulttimedsn)
                next= reverse('showdata:timeseriesmonthly', kwargs={
                    "ident":kwargs.get("ident"),
                    "coords":kwargs.get("coords"), 
                    "network":kwargs.get("network"), 
                    "trange":kwargs.get("trange"),
                    "level":kwargs.get("level"),
                    "var":kwargs.get("var"),
                    "year" :"{:04d}".format(dtnext.year),
                    "month":"{:02d}".format(dtnext.month)})+"?dsn="+request.GET.get('dsn', defaulttimedsn)
                more=reverse('showdata:timeseriesyearly', kwargs={
                    "ident":kwargs.get("ident"),
                    "coords":kwargs.get("coords"), 
                    "network":kwargs.get("network"), 
                    "trange":kwargs.get("trange"),
                    "level":kwargs.get("level"),
                    "var":kwargs.get("var"),
                    "year":kwargs.get("year")})+"?dsn="+request.GET.get('dsn', defaulttimedsn)
                less=reverse('showdata:timeseriesdaily', kwargs={
                    "ident":kwargs.get("ident"),
                    "coords":kwargs.get("coords"), 
                    "network":kwargs.get("network"), 
                    "trange":kwargs.get("trange"),
                    "level":kwargs.get("level"),
                    "var":kwargs.get("var"),
                    "year":kwargs.get("year"),
                    "month":kwargs.get("month"),
                    "day":"15"})+"?dsn="+request.GET.get('dsn', defaulttimedsn)
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
                "year" :"{:04d}".format(dtprevious.year)})+"?dsn="+request.GET.get('dsn', defaulttimedsn)
            next= reverse('showdata:timeseriesyearly', kwargs={
                "ident":kwargs.get("ident"),
                "coords":kwargs.get("coords"), 
                "network":kwargs.get("network"), 
                "trange":kwargs.get("trange"),
                "level":kwargs.get("level"),
                "var":kwargs.get("var"),
                "year" :"{:04d}".format(dtnext.year)})+"?dsn="+request.GET.get('dsn', defaulttimedsn)
            more=None
            less=reverse('showdata:timeseriesmonthly', kwargs={
                "ident":kwargs.get("ident"),
                "coords":kwargs.get("coords"), 
                "network":kwargs.get("network"), 
                "trange":kwargs.get("trange"),
                "level":kwargs.get("level"),
                "var":kwargs.get("var"),
                "year":kwargs.get("year"),
                "month":"06"})+"?dsn="+request.GET.get('dsn', defaulttimedsn)
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
        bcode=Bcode(bcode="B00001",description="Undefined",unit="Undefined",userunit="",scale=1.0,offset=0.0)
    else:
        varinfo=dballe.varinfo(kwargs.get("var"))
        #vartxt=varinfo.desc+" "+varinfo.unit
        vartxt=varinfo.desc
        bcode=Bcode.objects.get(bcode=kwargs.get("var"))

    spatialbox={}
    for k in ('lonmin','latmin','lonmax','latmax'):
        if not request.GET.get(k, None) is None:
            spatialbox[k]=request.GET.get(k)

    timebox={}
    for k in ('yearmin','monthmin','daymin','hourmin','minumin','secmin','yearmax','monthmax','daymax','hourmax','minumax','secmax'):
        if not request.GET.get(k, None) is None:
            timebox[k]=request.GET.get(k)
            
    return render(request, html_template,{
        "ident":kwargs.get("ident"),"coords":kwargs.get("coords"), 
        "undescored_coords":kwargs.get("coords").replace(",","_"), "network":kwargs.get("network"), 
        "trange":kwargs.get("trange"), "undescored_trange":kwargs.get("trange").replace(",","_"), 
        "level":kwargs.get("level"), "undescored_level":kwargs.get("level").replace(",","_"), "var":kwargs.get("var"), 
        "year":kwargs.get("year"), "month":kwargs.get("month"), "day":kwargs.get("day"),
        "hour":kwargs.get("hour"), 
        "datefrom":datefrom,"dateuntil":dateuntil, 
        "vartxt":vartxt, "trangetxt":trangetxt, "leveltxt":leveltxt,
        "previous":previous,"next":next,"less":less,"more":more,"dsn":request.GET.get('dsn', defaulttimedsn),"bcode":bcode,"spatialbox":spatialbox,"timebox":timebox})


def rainbospatialseries(request,html_template="showdata/spatialseries.html",**kwargs):

    return spatialseries(request,html_template="showdata/rainbospatialseries.html",**kwargs)


def spatialseries(request,html_template="showdata/spatialseries.html",**kwargs):

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
                        "hour" :"{:02d}".format(dtprevious.hour)})+"?dsn="+request.GET.get('dsn', defaultdsn)
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
                        "hour" :"{:02d}".format(dtnext.hour)})+"?dsn="+request.GET.get('dsn', defaultdsn)
                    more=reverse('showdata:spatialseriesdaily', kwargs={
                        "ident":kwargs.get("ident"),
                        "coords":kwargs.get("coords"), 
                        "network":kwargs.get("network"), 
                        "trange":kwargs.get("trange"),
                        "level":kwargs.get("level"),
                        "var":kwargs.get("var"),
                        "year":kwargs.get("year"),
                        "month":kwargs.get("month"),
                        "day":kwargs.get("day")})+"?dsn="+request.GET.get('dsn', defaultdsn)
                    less=None
                    datefrom=kwargs.get("hour")+":00_"+kwargs.get("year")+kwargs.get("month")+kwargs.get("day")
                    dateuntil=kwargs.get("hour")+":59_"+kwargs.get("year")+kwargs.get("month")+kwargs.get("day")
                elif not request.GET.get('type') is None:
                    #DAILY RAINBO FILTER
                    timerequested=datetime(year=int(kwargs.get("year")), month=int(kwargs.get("month")), day=int(kwargs.get("day")))
                    delta=timedelta(days=1)
                    dtprevious = timerequested - delta
                    dtnext     = timerequested + delta
                    previous = reverse('spatialseriesdaily',kwargs={
                        "ident":kwargs.get("ident"),
                        "coords":kwargs.get("coords"),
                        "network":kwargs.get("network"),
                        "trange":kwargs.get("trange"),
                        "level":kwargs.get("level"),
                        "var":kwargs.get("var"),
                        "year" :"{:04d}".format(dtprevious.year),
                        "month":"{:02d}".format(dtprevious.month),
                        "day"  :"{:02d}".format(dtprevious.day)})\
                        +"?dsn="+request.GET.get('dsn', defaultdsn)+"&type="+request.GET.get('type')                        
                    next= reverse('spatialseriesdaily', kwargs={
                        "ident":kwargs.get("ident"),
                        "coords":kwargs.get("coords"),
                        "network":kwargs.get("network"),
                        "trange":kwargs.get("trange"),
                        "level":kwargs.get("level"),
                        "var":kwargs.get("var"),
                        "year" :"{:04d}".format(dtnext.year),
                        "month":"{:02d}".format(dtnext.month),
                        "day"  :"{:02d}".format(dtnext.day)})\
                        +"?dsn="+request.GET.get('dsn', defaultdsn)+"&type="+request.GET.get('type')                        
                    datefrom="00:00_"+kwargs.get("year")+kwargs.get("month")+kwargs.get("day")
                    dateuntil="23:59_"+kwargs.get("year")+kwargs.get("month")+kwargs.get("day")
                    less=None
                    more=None
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
                        "day"  :"{:02d}".format(dtprevious.day)})+"?dsn="+request.GET.get('dsn', defaultdsn)
                    next= reverse('showdata:spatialseriesdaily', kwargs={
                        "ident":kwargs.get("ident"),
                        "coords":kwargs.get("coords"), 
                        "network":kwargs.get("network"), 
                        "trange":kwargs.get("trange"),
                        "level":kwargs.get("level"),
                        "var":kwargs.get("var"),
                        "year" :"{:04d}".format(dtnext.year),
                        "month":"{:02d}".format(dtnext.month),
                        "day"  :"{:02d}".format(dtnext.day)})+"?dsn="+request.GET.get('dsn', defaultdsn)
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
                        "hour":"12"})+"?dsn="+request.GET.get('dsn', defaultdsn)
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
        bcode=Bcode(bcode="B00001",description="Undefined",unit="Undefined",userunit="",scale=1.0,offset=0.0)
    else:
        varinfo=dballe.varinfo(kwargs.get("var"))
        #vartxt=varinfo.desc+" "+varinfo.unit
        vartxt=varinfo.desc
        bcode=Bcode.objects.get(bcode=kwargs.get("var"))
        
    spatialbox={}
    for k in ('lonmin','latmin','lonmax','latmax'):
        if not request.GET.get(k, None) is None:
            spatialbox[k]=request.GET.get(k)

    timebox={}
    for k in ('yearmin','monthmin','daymin','hourmin','minumin','secmin','yearmax','monthmax','daymax','hourmax','minumax','secmax'):
        if not request.GET.get(k, None) is None:
            timebox[k]=request.GET.get(k)

    return render(request, html_template,{
        "ident":kwargs.get("ident"), "coords":kwargs.get("coords"), 
        "network":kwargs.get("network"), "trange":kwargs.get("trange"), 
        "level":kwargs.get("level"), "var":kwargs.get("var"), 
        "year":kwargs.get("year"), "month":kwargs.get("month"), "day":kwargs.get("day"), 
        "hour":kwargs.get("hour"), 
        "vartxt":vartxt, "trangetxt":trangetxt, "leveltxt":leveltxt,
        "datefrom":datefrom,"dateuntil":dateuntil,
        "previous":previous,"next":next,"less":less,"more":more,"dsn":request.GET.get('dsn', defaultdsn),"bcode":bcode,"spatialbox":spatialbox,"timebox":timebox})
    
def stations(request, **kwargs):

    spatialbox={}
    for k in ('lonmin','latmin','lonmax','latmax'):
        if not request.GET.get(k, None) is None:
            spatialbox[k]=request.GET.get(k)

    timebox={}
    for k in ('yearmin','monthmin','daymin','hourmin','minumin','secmin','yearmax','monthmax','daymax','hourmax','minumax','secmax'):
        if not request.GET.get(k, None) is None:
            timebox[k]=request.GET.get(k)

    return render(request, 'showdata/stations.html',{"ident":kwargs.get("ident"), "coords":kwargs.get("coords"), "network":kwargs.get("network"), "trange":kwargs.get("trange"), "level":kwargs.get("level"), "var":kwargs.get("var"),"dsn":request.GET.get('dsn', defaultdsn),"spatialbox":spatialbox,"timebox":timebox})

