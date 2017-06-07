# documentation at http://graphite.readthedocs.io/en/latest/storage-backends.html

import time
import requests
import re
import dateutil.parser

from ..intervals import Interval, IntervalSet
from ..node import LeafNode, BranchNode
from rmap.settings import *
from itertools import groupby
from django.contrib.sites.models import Site
from datetime import timedelta, datetime
from rrule import rrule, YEARLY, MONTHLY, DAILY, HOURLY


timeout=180.
simclocalextensions=True

def sortandgroup(rj,key):
    return groupby(sorted(rj, key=lambda staz: staz[key]),key=lambda staz: staz[key])


def path2uri(path):
    #uri="/*/*/*/*/*/*"
    #print "path=",path
    uri = path.replace(".","/").replace("_",",")
    uri+="/*" * (6-uri.count("/"))
    #remove the root path
    uri = "{}/{}/{}/{}/{}/{}".format(*uri.split("/")[1:])

    return uri

class  wssummaries(object):

    def __init__(self,query,datalevel,stationtype):
        self.query=query
        self.datalevel=datalevel
        self.stationtype=stationtype

        #print "query pattern: ",self.query.pattern
        #self.query.startTime
        #self.query.endTime

        self.branch =True

        position=self.query.pattern.count(".")
        if position == 0:
            self.key="root"
        elif position == 1:
            self.key="ident"
        elif position == 2:
            self.key="lonlat"
        elif position == 3:
            self.key="network"
        elif position == 4:
            self.key="timerange"
        elif position == 5:
            self.key="level"
        elif position == 6:
            self.key="var"
            self.branch=False
        else:
            raise "error in graphite query to dballe"

        self.summaries=[]

        if self.key != "root" and self.query.pattern.split(".")[0] == self.datalevel+"_"+self.stationtype:

            #p = re.compile(query.pattern.replace(".","\.").replace("*",".*"))
            uri=path2uri(self.query.pattern)

            r=requests.get("http://"+Site.objects.get(id=SITE_ID).domain+"/borinud/api/v1/dbajson/"+uri+"/summaries?dsn="+self.datalevel+"_"+self.stationtype,timeout=timeout)
            rj=r.json()

            #serialize json in a new json good to build graphite path 
            for station in rj:

                #skip fixed station in mobile
                if self.stationtype == "mobile" and station["ident"] is None:
                    continue

                newstation={}
                newstation["ident"]=station["ident"] if not station["ident"] is None else "-"                 

                if self.stationtype == "mobile" :
                    newstation["lonlat"]="*"
                else:
                    newstation["lonlat"]=str(station["lon"])+"_"+str(station["lat"])

                newstation["network"]=station["network"]

                for data in station["data"]:
                    #print "data: ",data

                    for i,value in enumerate(data["level"]):
                        if data["level"][i] is None:
                            data["level"][i]="-"
                        else:
                            data["level"][i]=str(data["level"][i])

                    for i,value in enumerate(data["timerange"]):
                        if data["timerange"][i] is None:
                            data["timerange"][i]="-"
                        else:
                            data["timerange"][i]=str(data["timerange"][i])
                            
                    newstation["level"]= data["level"][0]+"_"+data["level"][1]+"_"+data["level"][2]+"_"+data["level"][3]
                    newstation["timerange"]=data["timerange"][0]+"_"+data["timerange"][1]+"_"+data["timerange"][2]

                    for key in data["vars"].keys():
                        newstation["var"]=key
                        if self.stationtype == "mobile" :
                            #compat mobile stations same ident ... and different coordinates 
                            if not newstation in self.summaries:
                                self.summaries.append(newstation)
                        else:
                            self.summaries.append(newstation)
                            
        #initialize generator
        self.mygenerator=self.generator()

    def __iter__(self):
        return self

    def next(self):
        #the next of iterator is next of generator
        return self.mygenerator.next()

    def generator(self):

        if self.key == "root":
            #we are in the root branch
            yield True,self.datalevel+"_"+self.stationtype

        elif self.branch:
            for k,g in sortandgroup(self.summaries,self.key):
                #print "---->>",self.key," = ",k
                #for station in g:
                #    print "---->> station = ",station
                
                if k is None:
                    k="-"
                # here we are in branch and this test is not required
                #if self.query.pattern[-1] == "*":
                    # remove the last ".*"
                self.node=self.query.pattern[:self.query.pattern.rfind(".")]+"."+str(k)
                #else:
                #    self.node=self.query.pattern+"."+str(k)

                yield self.branch,self.node

        else:
            for summary in self.summaries:
                #print "summary",summary
                self.node=self.datalevel+"_"+self.stationtype+"."+summary["ident"]+"."+summary["lonlat"]+"."+summary["network"]+"."+summary["timerange"]+"."+summary["level"]+"."+summary["var"]
                yield self.branch,self.node


#LeafNode is created with a reader, which is the class responsible for
#fetching the datapoints for the given path. It is a simple class with
#2 methods: fetch() and get_intervals():

#fetch() must return a list of 2 elements: the time info for the data
#and the datapoints themselves. The time info is a list of 3 items: the
#start time of the datapoints (in unix time), the end time and the time
#step (in seconds) between the datapoints.

#The datapoints is a list of points found in the database for the
#required interval. There must be (end - start) / step points in 
#the dataset even if the database has gaps: gaps can be filled with None values.

#get_intervals() is a method that hints graphite-web about the time range 
#available for this given metric in the database.
#It must return an IntervalSet of one or more Interval objects.

class DballeFinderReportFixed(object):

    def find_nodes(self, query):

        #print "query and class: ", query.pattern, "fixed"
        # find some paths matching the query, then yield them
        for branch,node in wssummaries(query,"report","fixed"):
            if branch:
                yield BranchNode(node)
            else:
                yield LeafNode(node, DballeReader(node,"report","fixed"))
            

class DballeFinderReportMobile(object):

    def find_nodes(self, query):

        #print "query and class: ", query.pattern, "mobile"
        # find some paths matching the query, then yield them
        for branch,node in wssummaries(query,"report","mobile"):
            if branch:
                yield BranchNode(node)
            else:
                yield LeafNode(node, DballeReader(node,"report","mobile"))

class DballeFinderSampleFixed(object):

    def find_nodes(self, query):

        #print "query and class: ", query.pattern, "sample"
        # find some paths matching the query, then yield them
        for branch,node in wssummaries(query,"sample","fixed"):
            if branch:
                yield BranchNode(node)
            else:
                yield LeafNode(node, DballeReader(node,"sample","fixed"))


class DballeFinderSampleMobile(object):

    def find_nodes(self, query):

        #print "query and class: ", query.pattern, "mobile"
        # find some paths matching the query, then yield them
        for branch,node in wssummaries(query,"sample","mobile"):
            if branch:
                yield BranchNode(node)
            else:
                yield LeafNode(node, DballeReader(node,"sample","mobile"))



                
class DballeReader(object):
    __slots__ = ('path','datalevel','stationtype')  # __slots__ is recommended to save memory on readers

    def __init__(self, path, datalevel,stationtype):
        self.path = path
        self.datalevel = datalevel
        self.stationtype = stationtype
        #print "DBALLEREADER datalevel: ", datalevel," stationtype: ",stationtype," path: ", self.path
        
    def fetch(self, start_time, end_time):

        # fetch data
        #print "fetch: ",start_time,end_time
        uri=path2uri(self.path)
        #starttime=time.gmtime(start_time)
        #print starttime
        #endtime=time.gmtime(end_time)
        #print endtime
        startdt=datetime.utcfromtimestamp(start_time)
        enddt  =datetime.utcfromtimestamp(end_time)

        #print "fetch: ", startdt, enddt

        rj=[]


        if self.path.split(".")[0] == self.datalevel+"_"+self.stationtype :
            dt=enddt-startdt
        else:
            #have to return none
            dt=timedelta(hours=0)
            
        #print "deltatime: ", dt

        if simclocalextensions:
        
            if dt > timedelta(hours=0):
                r=requests.get("http://"+Site.objects.get(id=SITE_ID).domain+"/borinud/api/v1/dbajson/"+uri+
                               "/timeseries/"+"{:04d}".format(startdt.year)+"?dsn="+self.datalevel+"_"+self.stationtype+
                               "&yearmin={:04d}".format(startdt.year)+"&monthmin={:02d}".format(startdt.month)+"&daymin={:02d}".format(startdt.day)+"&hourmin={:02d}".format(startdt.hour)+"&minumin={:02d}".format(startdt.minute)+"&secmin={:02d}".format(startdt.second)+
                               "&yearmax={:04d}".format(enddt.year)+"&monthmax={:02d}".format(enddt.month)+"&daymax={:02d}".format(enddt.day)+"&hourmax={:02d}".format(enddt.hour)+"&minumax={:02d}".format(enddt.minute)+"&secmax={:02d}".format(enddt.second)
                               ,timeout=timeout)
                rj=r.json()

        else:
            
            #check which query we have to do
            if dt > timedelta(days=30*6):
                #get  years
                step=3600*24
                startdt=startdt.replace(month=1,day=1,hour=0,minute=0,second=0)
                for dt in rrule(YEARLY , dtstart=startdt, until=enddt):
                    #print "loop: ", dt
                    #print "http://"+Site.objects.get(id=SITE_ID).domain+"/borinud/api/v1/dbajson/"+uri+ \
                    #               "/timeseries/"+"{:04d}".format(dt.year)+"?dsn="+self.datalevel+"_"+self.stationtype
                    r=requests.get("http://"+Site.objects.get(id=SITE_ID).domain+"/borinud/api/v1/dbajson/"+uri+
                                   "/timeseries/"+"{:04d}".format(dt.year)+"?dsn="+self.datalevel+"_"+self.stationtype,timeout=timeout)
                    rj+=r.json()
            elif dt > timedelta(days=10):
                #get  month
                step=3600*6
                startdt=startdt.replace(day=1,hour=0,minute=0,second=0)
                for dt in rrule(MONTHLY, dtstart=startdt, until=enddt):
                    #print "loop: ", dt
                    #print "http://"+Site.objects.get(id=SITE_ID).domain+"/borinud/api/v1/dbajson/"+uri+ \
                    #               "/timeseries/"+"{:04d}".format(dt.year)+"/{:02d}".format(dt.month)+"?dsn="+self.datalevel+"_"+self.stationtype
                    r=requests.get("http://"+Site.objects.get(id=SITE_ID).domain+"/borinud/api/v1/dbajson/"+uri+
                                   "/timeseries/"+"{:04d}".format(dt.year)+"/{:02d}".format(dt.month)+"?dsn="+self.datalevel+"_"+self.stationtype,timeout=timeout)
                    rj+=r.json()
            elif dt > timedelta(hours=8):
                #get days
                step=3600
                startdt=startdt.replace(hour=0,minute=0,second=0)
                for dt in rrule(DAILY, dtstart=startdt, until=enddt):
                    #print "http://"+Site.objects.get(id=SITE_ID).domain+"/borinud/api/v1/dbajson/"+uri+ \
                    #               "/timeseries/"+"{:04d}".format(dt.year)+"/{:02d}".format(dt.month)+"/{:02d}".format(dt.day)+"?dsn="+self.datalevel+"_"+self.stationtype
                    r=requests.get("http://"+Site.objects.get(id=SITE_ID).domain+"/borinud/api/v1/dbajson/"+uri+
                                   "/timeseries/"+"{:04d}".format(dt.year)+"/{:02d}".format(dt.month)+"/{:02d}".format(dt.day)+"?dsn="+self.datalevel+"_"+self.stationtype,timeout=timeout)
                    rj+=r.json()
            elif dt > timedelta(hours=0) :
                #get hours
                step=60
                startdt=startdt.replace(minute=0,second=0)
                for dt in rrule(HOURLY, dtstart=startdt, until=enddt):
                    #print "loop: ", dt
                    #print "http://"+Site.objects.get(id=SITE_ID).domain+"/borinud/api/v1/dbajson/"+uri+ \
                    #               "/timeseries/"+"{:04d}".format(dt.year)+"/{:02d}".format(dt.month)+"/{:02d}".format(dt.day)+ \
                    #               "/{:02d}".format(dt.hour)+"?dsn="+self.datalevel+"_"+self.stationtype
                    r=requests.get("http://"+Site.objects.get(id=SITE_ID).domain+"/borinud/api/v1/dbajson/"+uri+
                                   "/timeseries/"+"{:04d}".format(dt.year)+"/{:02d}".format(dt.month)+"/{:02d}".format(dt.day)+
                                   "/{:02d}".format(dt.hour)+"?dsn="+self.datalevel+"_"+self.stationtype,timeout=timeout)
                    rj+=r.json()

            # if starttime.tm_year != endtime.tm_year:
            #     #span years: get two month minimum
            #     #span years
            #     step=3600
            #     rj=[]
            #     for year in xrange( starttime.tm_year, endtime.tm_year+1):
            #         if starttime.tm_mon < endtime.tm_mon:
            #             for month in xrange( starttime.tm_mon, endtime.tm_mon+1):
            #                 r=requests.get("http://"+Site.objects.get(id=SITE_ID).domain+"/borinud/api/v1/dbajson/"+uri+"/timeseries/"+"{:04d}".format(year)+"/{:02d}".format(month))
            #                 rj+=r.json()
            #         else:
            #             for month in xrange( starttime.tm_mon, 12+1):
            #                 r=requests.get("http://"+Site.objects.get(id=SITE_ID).domain+"/borinud/api/v1/dbajson/"+uri+"/timeseries/"+"{:04d}".format(year)+"/{:02d}".format(month))
            #                 rj+=r.json()
            #             for month in xrange( 1, endtime.tm_mon+1):
            #                 r=requests.get("http://"+Site.objects.get(id=SITE_ID).domain+"/borinud/api/v1/dbajson/"+uri+"/timeseries/"+"{:04d}".format(year)+"/{:02d}".format(month))
            #                 rj+=r.json()

            # elif starttime.tm_mon != endtime.tm_mon:
            #     #span months, same year: get two month minimum
            #     step=3600
            #     rj=[]
            #     for month in xrange( starttime.tm_mon, endtime.tm_mon+1):
            #         r=requests.get("http://"+Site.objects.get(id=SITE_ID).domain+"/borinud/api/v1/dbajson/"+uri+"/timeseries/"+"{:04d}".format(starttime.tm_year)+"/{:02d}".format(month))
            #         rj+=r.json()

            # elif starttime.tm_mday != endtime.tm_mday:
            #     #span day same month: get two month minimum
            #     step=3600
            #     rj=[]
            #     for day in xrange( starttime.tm_mday, endtime.tm_mday+1):
            #         r=requests.get("http://"+Site.objects.get(id=SITE_ID).domain+"/borinud/api/v1/dbajson/"+uri+"/timeseries/"+"{:04d}".format(starttime.tm_year)+"/{:02d}".format(starttime.tm_mon)+"/{:02d}".format(day))
            #         rj+=r.json()

            # elif starttime.tm_hour != endtime.tm_hour:
            #     #span hour same day: get two hour minimum
            #     step=60
            #     rj=[]
            #     for hour in xrange( starttime.tm_hour, endtime.tm_hour+1):
            #         r=requests.get("http://"+Site.objects.get(id=SITE_ID).domain+"/borinud/api/v1/dbajson/"+uri+"/timeseries/"+"{:04d}".format(starttime.tm_year)+"/{:02d}".format(starttime.tm_mon)+"/{:02d}".format(starttime.tm_mday)+"/{:02d}".format(hour))
            #         rj+=r.json()

            # else:
            #     #one hour
            #     step=60
            #     r=requests.get("http://"+Site.objects.get(id=SITE_ID).domain+"/borinud/api/v1/dbajson/"+uri+"/timeseries/"+"{:04d}".format(starttime.tm_year)+"/{:02d}".format(starttime.tm_mon)+"/{:02d}".format(starttime.tm_mday)+"/{:02d}".format(starttime.tm_hour))
            #     step=60*5
            #     rj=r.json()

        if len(rj) > 0:

            if self.stationtype == "mobile" :
                rj=sorted(rj, key=lambda staz: staz["date"])

            # find minimum step in data
            #print "find minimum step in data"
            if len(rj) > 1:
                step=end_time-start_time
                startstep = rj[0]["date"]
                #print "startstep:",startstep
                startdatestep = dateutil.parser.parse(startstep)  
                starttimestep = int(time.mktime(startdatestep.timetuple()))
                for i in xrange(1,len(rj)):
                    endstep   = rj[i]["date"]
                    #print "endstep:",endstep
                    enddatestep   = dateutil.parser.parse(endstep)
                    endtimestep   = int(time.mktime(enddatestep.timetuple()))
                    #print "steps: ",endtimestep-starttimestep
                    step=min([step,endtimestep-starttimestep])
                    starttimestep=endtimestep
            #print "found it: ",step

            start = rj[0]["date"]
            end   = rj[-1]["date"]
            
            startdate = dateutil.parser.parse(start)  
            enddate   = dateutil.parser.parse(end)
        
            starttime = int(time.mktime(startdate.timetuple()))
            endtime   = int(time.mktime(enddate.timetuple()))

            size=int((int(end_time)-int(start_time))/step)+1
            series=[None for i in xrange(size)]

            #print "request time: ",start_time,end_time
            #print "getted  time: ",starttime,endtime
            #print "step: ",step
            #print "size: ",size

            #print "recompute end time to not have spare"
            # recompute end time to not have spare
            end_time=start_time+(step*(size-1))
            time_info=(start_time, end_time, step)

            #print "recomputed end time: ",endtime

            #print "put data in an equaly time spaced array"

            for station in rj:
                #print "station: ", station
                # put data in an equaly time spaced array

                isodate = station["date"]
                #print "isodate: ", isodate
            
                date = dateutil.parser.parse(isodate)  
                mytime = int(time.mktime(date.timetuple()))
                #print "mytime: ",mytime

                i=int(((mytime+(step/2))-start_time)/step)-1
                if i>=0 and i<=(size-1):
                    series[i]=station["data"][0]["vars"][uri.split("/")[-1]]["v"]
            
        else:
            series=[]
            time_info=(start_time, end_time,end_time-start_time)

        #print "time_info: ",time_info
        #print "series: ",series
        #time_info = _from_, _to_, _step_
        #time_info=(int(time.time()-100), int(time.time()),1)
        #series=range(*time_info)

        return time_info, series

    def get_intervals(self):
        #print "getintervals"
        #return IntervalSet([Interval(start, end)])

        uri=path2uri(self.path)

        r=requests.get("http://"+Site.objects.get(id=SITE_ID).domain+"/borinud/api/v1/dbajson/"+uri+"/summaries?dsn="+self.datalevel+"_"+self.stationtype,timeout=timeout)
        rj=r.json()

        if self.stationtype == "mobile" :
            rj=sorted(rj, key=lambda staz: staz["date"])

        start=rj[0]["date"][0]
        end=rj[-1]["date"][1]

        startdate = dateutil.parser.parse(start)  
        enddate   = dateutil.parser.parse(end)
        
        #prevent underflow and overflow
        startdate=startdate.replace(max(startdate.year,1970))
        enddate  =enddate.replace(min(enddate.year,2037))

        return IntervalSet([Interval(int(time.mktime(startdate.timetuple())),int(time.mktime(enddate.timetuple())))])
        
        #return IntervalSet([Interval(int(time.time())-100, int(time.time()))])


