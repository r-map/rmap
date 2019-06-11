#!/usr/bin/env python
# GPL. (C) 2015 Paolo Patruno.

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
# 

from utils import nint
from geoid_corrections import geoid
from kivy.clock import Clock, mainthread

def null(**kwargs):
    return

class plyergps():
    """
    GPS interface for Android and ...
    use plyer
    """

    def __init__(self,call_on_location=null,call_on_status=null):

        from plyer import gps

        self.call_on_location=call_on_location
        self.call_on_status=call_on_status

        self.gpsfix=False
        self.connected = False
        self.lat=None
        self.lon=None
        self.height=None
        self.status="GPS status unknown"

        self.gps = gps

        try:
            self.gps.configure(on_location=self.on_location,
                    on_status=self.on_status)
        except NotImplementedError:
            self.status="GPS not implemented on this platform"


    def start(self):
        ''' start use GPS'''

        try:
            self.gps.start()
            self.status="Starting GPS: wait ..."
            self.connected = True
            self.geo=geoid()
            return 0

        except NotImplementedError:
            print('GPS not implemented on this platform')
            self.status="GPS not implemented in this platform"
            return 1


    def stop(self):
        ''' stop use of GPS'''
        if self.connected:
            self.gps.stop()
            self.geo.close()
        self.status = 'GPS OFF'
        self.connected = False


    #@mainthread
    def on_location(self, **kwargs):
        '''
        callback for new GPS location
        '''

#        self.gps_location = '\n'.join([
#            '{}={}'.format(k, v) for k, v in kwargs.items()])
        self.lat=nint(kwargs["lat"]*100000)/100000.
        self.lon=nint(kwargs["lon"]*100000)/100000.
        try:
            self.height=int(kwargs["altitude"])-int(self.geo.get(self.lon,self.lat))
        except:
            print "ERROR getting height on msl"
            self.height=None

        self.gpsfix=True

        self.call_on_location(lon=self.lon,lat=self.lat,height=self.height,gpsfix=self.gpsfix)


    #@mainthread
    def on_status(self, stype, status):
        '''
        callback for new GPS status
        '''

        self.status = 'type={}; {}'.format(stype, status)

        #mmm take alook at http://stackoverflow.com/questions/2021176/how-can-i-check-the-current-status-of-the-gps-receiver
        if self.status == "gps: available":
            self.gpsfix=True
        else:
            self.gpsfix=False

        self.call_on_status(status=self.status,gpsfix=self.gpsfix)


