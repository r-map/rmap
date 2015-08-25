#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
from os.path import join,dirname

# una riga di dballe.txt
class BtableEntry:
    def __init__(self, line=None, bcode=None, name=None,
                 unitb=None, scaleb=None, refb=None, widthb=None, 
                 unitc=None, scalec=None,            widthc=None):
        if line is not None:
            self.bcode=line[1:7]
            self.name = line[8:72].rstrip()
            self.unitb = line[73:97].rstrip()
            self.scaleb = int(line[98:101])
            self.refb = int(line[102:114])
            self.widthb = int(line[115:118])
            self.unitc = line[119:142].rstrip()
            self.scalec = int(line[143:146])
            self.widthc = int(line[147:157])

        if bcode  is not None: self.bcode = bcode
        if name   is not None: self.name = name
        if unitb  is not None: self.unitb = unitb
        if scaleb is not None: self.scaleb = scaleb
        if refb   is not None: self.refb = refb
        if widthb is not None: self.widthb = widthb
        if unitc  is not None: self.unitc = unitc
        if scalec is not None: self.scalec = scalec
        if widthc is not None: self.widthc = widthc


    def output(self, fileobj=sys.stdout):
        fileobj.write(" %s %-64s %-24s %3s %12s %3s %-23s %3s %10s\n" % 
                      (self.bcode, self.name, self.unitb,
                       self.scaleb, self.refb, self.widthb,
                       self.unitc, self.scalec, self.widthc))


    def __str__(self):

        return self.name+" "+self.unitb


# dballe.txt
class Btable(dict):
    def __init__(self, filename=None):


        if filename is None:

            try:
                file = open("dballe.txt")
            except:
                try:
                    file = open(join(dirname(__file__), "tables","dballe.txt"))
                except:
                    file = open("/usr/share/wreport/dballe.txt")
        else:
            file = open(filename)


        for line in file.readlines():
            entry=BtableEntry(line)
            if entry.bcode[0] == "0":
                bcode=entry.bcode.replace("0","B",1)
            else:
                bcode=entry.bcode

            self[bcode]=entry


    def output(self, fileobj=sys.stdout):
        for entry in self.iterkeys():
            self[entry].output(fileobj)



def main():

    # importo dballe.txt
    table = Btable()
    table.output()

    print "------------------------------"

    print  table["B12101"]

if __name__ == '__main__':
    main()  # (this code was run as script)
