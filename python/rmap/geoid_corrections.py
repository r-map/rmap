#!/usr/bin/python

# Copyright (c) 2013 Paolo Patruno <p.patruno@iperbole.bologna.it>
# This program is free software; you can redistribute it and/or modify 
# it under the terms of the GNU General Public License as published by 
# the Free Software Foundation; either version 2 of the License, or 
# (at your option) any later version. 
# 
# This program is distributed in the hope that it will be useful, 
# but WITHOUT ANY WARRANTY; without even the implied warranty of 
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
# GNU General Public License for more details. 
# 
# You should have received a copy of the GNU General Public License 
# along with this program; if not, write to the Free Software 
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 


import sqlite3
from utils import nint

class geoid():


    def __init__ (self,db='geoid_heights/geoid-corrections.sqlite'):
        self.con = sqlite3.connect(db, check_same_thread=False)
        self.cur = self.con.cursor()
        self.lat=None
        self.lon=None
        self.height=None

    def get(self,lon,lat):

        nlat=nint(lat)
        nlon=nint(lon)

        if (self.lat != nlat or self.lon != nlon): 
            #try:
            #print "execute select"
            self.cur.execute('select geoid_height from geoid_height where longitude=? and latitude=?',(nint(lon),nint(lat)))
            self.height=self.cur.fetchone()

            #except ProgrammingError:
            #    # here we are in a diffrent thread ! new con and cur
            #    # ProgrammingError: SQLite objects created in a thread can only be used in that same thread.
            #    # The object was created in thread id 1100526960 and this is thread id 1074954032
            #
            #    print "ProgrammingError: SQLite objects created in a thread can only be used in that same thread."
            #    print "recover with new connect"
            #    not needed with check_same_thread=False
            #
            #    con = sqlite3.connect(db)
            #    cur = self.con.cursor()
            #    cur.execute('select geoid_height from geoid_height where longitude=? and latitude=?',(nint(lon),nint(lat)))
            #    self.height=self.cur.fetchone()
            #    con.close()

            if self.height is not None:
                self.height=self.height[0]
            self.lat=nlat
            self.lon=nlon

        return self.height

    def close (self):
        self.con.close()

class Closer:
    '''A context manager to automatically close an object with a close method
    in a with statement.'''

    def __init__(self, obj):
        self.obj = obj

    def __enter__(self):
        return self.obj # bound to target

    def __exit__(self, exception_type, exception_val, trace):
        try:
           self.obj.close()
        except AttributeError: # obj isn't closable
           print 'Not closable.'
           return True # exception handled successfully



def main():

    lon=11.36992
    lat=44.48906
        
    with Closer(geoid()) as geo:
        print geo.get(lon,lat)
        print geo.get(lon,lat)
        print geo.get(lon+1.,lat)
        print geo.get(lon,lat+1.)
        print geo.get(90.,60.)


if __name__ == '__main__':
    main()  # (this code was run as script)

