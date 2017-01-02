# documentation at http://graphite.readthedocs.io/en/latest/storage-backends.html

import time
#import requests
import re

from .intervals import Interval, IntervalSet
from .node import LeafNode, BranchNode
from rmap.settings import *

class DballeFinder(object):

#    def is_branch(self,path):
#        return not "@" in path

    def find_nodes(self, query):
        # find some paths matching the query, then yield them

        print "query: ", query

        #r=requests.get("http://rmap.cc/borinud/api/v2/"+query2path(query)+"/summary")
        #r.json()

        #query:  <FindQuery: due/duedue/dueduedue.* from * until *>
        #query.pattern
        #query.startTime
        #query.endTime

        metad=BORINUD ["SOURCES"][0]["measurements"] 

        p = re.compile(query.pattern.replace(".","\.").replace("*",".*"))

        # if query.pattern == "*":
        #     matches=["uno","due","tre"]
        # elif query.pattern == "uno.*":
        #     matches=["menouno","menodue","menotre"]
        # elif query.pattern == "due.*":
        #     matches=["ugualeuno","ugualedue","ugualetre"]
        # elif query.pattern == "tre.*":
        #     matches=["piuuno","piudue","piutre"]
        # else:
        #     matches=["misura1@","misura2@","misura3@"]

        for mymeta in metad:

            trange = "%s_%s_%s" % tuple(("-" if v is None else str(v) for v in mymeta["trange"]))
            level  = "%s_%s_%s_%s" % tuple(("-" if v is None else str(v) for v in mymeta["level"]))
            var=mymeta["var"]
            path=trange+"."+level+"."+var

            position=query.pattern.count(".")
            if position == 0:
                node=trange
            elif position == 1:
                node=level
            elif position == 2:
                node=var
            
            if  not (p.match(path) is None):
                #if   query.pattern[:-2].endswith(var):
                if   node == var:
                    yield LeafNode(path, DballeReader(node))
                else:
                    yield BranchNode(node)


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

class DballeReader(object):
    __slots__ = ('path',)  # __slots__ is recommended to save memory on readers

    def __init__(self, path):
        self.path = path

    def fetch(self, start_time, end_time):
        # fetch data

        print "fetch: ",start_time,end_time
        #time_info = _from_, _to_, _step_
        time_info=(int(time.time()-100), int(time.time()),1)
        series=range(*time_info)
        return time_info, series

    def get_intervals(self):
        print "getintervals"
        #return IntervalSet([Interval(start, end)])
        return IntervalSet([Interval(int(time.time())-100, int(time.time()))])



#STORAGE_FINDERS = (
#    'graphite-dballe.DballeFinder',
#)



# # Cyanite example

# import requests


# def chunk(nodelist, length):
#     chunklist = []
#     linelength = 0
#     for node in nodelist:
#         # the magic number 6 is because the nodes list gets padded
#         # with '&path=' in the resulting request
#         nodelength = len(str(node)) + 6

#         if linelength + nodelength > length:
#             yield chunklist
#             chunklist = [node]
#             linelength = nodelength
#         else:
#             chunklist.append(node)
#             linelength += nodelength
#     yield chunklist


# class CyaniteLeafNode(LeafNode):
#     __fetch_multi__ = 'cyanite'


# class URLs(object):
#     def __init__(self, hosts):
#         self.iterator = itertools.cycle(hosts)

#     @property
#     def host(self):
#         return next(self.iterator)

#     @property
#     def paths(self):
#         return '{0}/paths'.format(self.host)

#     @property
#     def metrics(self):
#         return '{0}/metrics'.format(self.host)
# urls = None
# urllength = 8000


# class CyaniteReader(object):
#     __slots__ = ('path',)

#     def __init__(self, path):
#         self.path = path

#     def fetch(self, start_time, end_time):
#         data = requests.get(urls.metrics, params={'path': self.path,
#                                                   'from': start_time,
#                                                   'to': end_time}).json()
#         if 'error' in data:
#             return (start_time, end_time, end_time - start_time), []
#         if len(data['series']) == 0:
#             return
#         time_info = data['from'], data['to'], data['step']
#         return time_info, data['series'].get(self.path, [])

#     def get_intervals(self):
#         # TODO use cyanite info
#         return IntervalSet([Interval(0, int(time.time()))])


# class CyaniteFinder(object):
#     __fetch_multi__ = 'cyanite'

#     def __init__(self, config=None):
#         global urls
#         global urllength
#         if config is not None:
#             if 'urls' in config['cyanite']:
#                 urls = config['cyanite']['urls']
#             else:
#                 urls = [config['cyanite']['url'].strip('/')]
#             if 'urllength' in config['cyanite']:
#                 urllength = config['cyanite']['urllength']
#         else:
#             from django.conf import settings
#             urls = getattr(settings, 'CYANITE_URLS')
#             if not urls:
#                 urls = [settings.CYANITE_URL]
#             urllength = getattr(settings, 'CYANITE_URL_LENGTH', urllength)
#         urls = URLs(urls)

#     def find_nodes(self, query):
#         paths = requests.get(urls.paths,
#                              params={'query': query.pattern}).json()
#         for path in paths:
#             if path['leaf']:
#                 yield CyaniteLeafNode(path['path'],
#                                       CyaniteReader(path['path']))
#             else:
#                 yield BranchNode(path['path'])

#     def fetch_multi(self, nodes, start_time, end_time):

#         paths = [node.path for node in nodes]
#         data = {}
#         for pathlist in chunk(paths, urllength):
#             tmpdata = requests.get(urls.metrics,
#                                    params={'path': pathlist,
#                                            'from': start_time,
#                                            'to': end_time}).json()
#             if 'error' in tmpdata:
#                 return (start_time, end_time, end_time - start_time), {}

#             if 'series' in data:
#                 data['series'].update(tmpdata['series'])
#             else:
#                 data = tmpdata

#         time_info = data['from'], data['to'], data['step']
#         return time_info, data['series']
