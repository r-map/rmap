#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
from collections import OrderedDict

# one table row
class TableEntry:
    def __init__(self, line=None, code=None, description=None):

        if line is not None:
            self.code=int(line[0:3])
            self.description = line[4:].replace("\n","").replace("\\n","\n")

        if code          is not None: self.code = code
        if description   is not None: self.description = description

    def output(self, fileobj=sys.stdout):
        fileobj.write(" %s %s\n" % 
                      (self.code, self.description))


    def __str__(self):

        return self.description


# table.txt
class Table(OrderedDict):
    def __init__(self, filename=None):

        super(Table,self).__init__()

        if filename is None:
            file = open("table.txt")
        else:
            file = open(filename)

        for line in file.readlines():
            entry=TableEntry(line)
            self[entry.code]=entry

    def output(self, fileobj=sys.stdout):
        for entry in tmp.iterkeys():
            self[entry].output(fileobj)

def main():

    # importo present_weather.txt
    table = Table("tables/present_weather.txt")
    table.output()

    print "------------------------------"

    print  table[171]

if __name__ == '__main__':
    main()  # (this code was run as script)
