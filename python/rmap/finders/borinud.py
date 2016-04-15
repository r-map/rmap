from graphite.node import LeafNode, BranchNode
from graphite.intervals import IntervalSet, Interval
import time

class Finder(object):
    def find_nodes(self, query):
        # find some paths matching the query, then yield them
        #for path in matches:
        #    if is_branch(path):
        #        yield BranchNode(path)
        #    if is_leaf(path):
        #        yield LeafNode(path, Reader(path))

        yield LeafNode(path, Reader(path))



class Reader(object):
    __slots__ = ('path',)  # __slots__ is recommended to save memory on readers

    def __init__(self, path):
        self.path = path

    def fetch(self, start_time, end_time):
        # fetch data
        try:
            _from_=start_time
            _to_=end_time
            _step_=(end_time-start_time)/9
            series=[0.,1.,2.,3.,4.,5.,6.,7.,8.,9.]

            time_info = _from_, _to_, _step_
            return time_info, series

        except:
            return (start_time, end_time, end_time - start_time), []

    def get_intervals(self):

        # TODO use borinud summary
        #return IntervalSet([Interval(start, end)])
        return IntervalSet([Interval(0, int(time.time()))])
