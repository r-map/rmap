#!/usr/bin/python

# Copyright (c) 2013 Paolo Patruno <p.patruno@iperbole.bologna.it>
#                    Emanuele Di Giacomo <edigiacomo@arpa.emr.it>
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

# N.B. 
# Bisognera provvedere a una ripulitura delle stazioni
# (messaggi retained) dopo un certo lasso di tempo 

import os
import pickle
import traceback

#import threading # https://github.com/kivy/kivy/wiki/Working-with-Python-threads-inside-a-Kivy-application

import settings
import jsonrpc
from sensordriver import SensorDriver
from gps import *
from bluetooth import *

from plyer.compat import PY2
from kivy.lib import osc    ####   osc IPC  ####
from kivy.utils import platform
from django.core.exceptions import ObjectDoesNotExist
from django.utils.translation import ugettext as _

#platform = platform()

#if platform == "android":
#     from android.broadcast import BroadcastReceiver
#     from jnius import autoclass

import json
from datetime import datetime, timedelta
import time
#import mosquitto
from rmapmqtt import rmapmqtt,do_notify
from utils import log_stdout
from stations.models import StationMetadata
import rmap_core


class station():

    """
    Main object to manage a station
    All configuration parameter are token from DB
    For now is possible to manage one board a time and only one transport for each
    You have to specify the station and the board to use.
    If not specified the passive transport will be selected by hard coded predefined priority:
    * bluetooth (hight)
    * serial (medium)
    * tcpip (low)
    For active transport only MQTT will be taken in account for publish (no AMQP)
    """

    def __init__(self,slug=None,username=None,boardslug=None, picklefile="saveddata-service.pickle",trip=None,gps=plyergps(),transport_name=None,logfunc=jsonrpc.log_stdout):
        '''
        do all startup operations
        '''

        print "INITIALIZE rmap station"
        self.picklefile=picklefile

        self.anavarlist=[]
        self.datavarlist=[]
        self.bluetooth=None
        self.mqtt_status = _('Connect Status: disconnected')
        self.rpcin_message = ""
        self.log = logfunc
        self.now=None

        self.anavarlist=[]
        self.datavarlist=[]
        self.trip=False
        self.slug= "BT_fixed"
        self.boardslug= "BT_fixed"

        # Exception: The intent ACTION_ACL_DISCONNECTED doesnt exist
        #self.br=BroadcastReceiver(self.on_broadcast,actions=["acl_disconnected"])
        #self.br.start()

        self.gps=gps

        try:
            if os.path.isfile(self.picklefile):
                with open( self.picklefile, "rb" ) as file:
                    self.anavarlist= pickle.load( file )
                    self.datavarlist= pickle.load( file )
                    self.trip= pickle.load( file )
                    self.slug= pickle.load( file )
                    self.boardslug= pickle.load( file )
            else:
                print "file ",self.picklefile," do not exist"
        except:
            print "ERROR loading saved data"
            self.anavarlist=[]
            self.datavarlist=[]
            self.trip=False
            self.slug= "BT_fixed"
            self.boardslug= "BT_fixed"

        if trip is not None:
            self.trip=trip

        if slug is not None:
            self.slug=slug

        if boardslug is not None:
            self.boardslug=boardslug

        try:
            print "get information for station:", self.slug
            if username is None:
                mystation=StationMetadata.objects.filter(slug=self.slug)[0]
            else:
                mystation=StationMetadata.objects.get(slug=self.slug,ident__username=username)
        except ObjectDoesNotExist:
            print "not existent station in db: do nothing!"
            #raise SystemExit(0)
            raise Rmapdonotexist("not existent station in db")

        if not mystation.active:
            print "Warning: disactivated station!"

        self.lon=mystation.lon
        self.lat=mystation.lat
        self.mqtt_ident=str(mystation.ident)
        self.prefix=mystation.mqttrootpath
        self.maintprefix=mystation.mqttmaintpath
        self.network=mystation.network
        self.transport_name=None
        self.active=mystation.active

        for cdata in mystation.stationconstantdata_set.all():
            print ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> constant data: ", cdata.btable
            if not cdata.active: continue
            print "found a good constant data"
            self.anavarlist.append({"coord":{"lat":self.lat,"lon":self.lon},"anavar":{cdata.btable:{"v": cdata.value}}})

        self.drivers=[]


        print "get info for BOARD:", self.boardslug
        for board in mystation.board_set.all().filter(slug=self.boardslug):
            print ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> configure board: ", board.name
            if not board.active: continue
            print "found a good base board"

            try:
                if ( board.transportmqtt.active):
                    print "MQTT Transport", board.transportmqtt
                    
                    self.mqtt_sampletime=board.transportmqtt.mqttsampletime
                    self.mqtt_host=board.transportmqtt.mqttserver
                    self.mqtt_user=board.transportmqtt.mqttuser
                    self.mqtt_password=board.transportmqtt.mqttpassword

            except ObjectDoesNotExist:
                print "transport mqtt not present"
                self.mqtt_sampletime=None
                self.mqtt_host=None
                self.mqtt_user=None
                self.mqtt_password=None
                #raise SystemExit(0)


            try:
                if ( board.transporttcpip.active):
                    print "TCPIP Transport", board.transporttcpip
                    
                    self.tcpip_name=board.transporttcpip.name
                    self.transport_name="tcpip"

            except ObjectDoesNotExist:
                print "transport tcpip not present"
                self.tcpip_name=None
            #    raise SystemExit(0)


            try:
                if ( board.transportserial.active):
                    print "Serial Transport", board.transportserial
                    
                    self.serial_device=board.transportserial.device
                    self.serial_baudrate=board.transportserial.baudrate
                    self.transport_name="serial"

            except ObjectDoesNotExist:
                print "transport serial not present"
                self.serial_device=None
                self.serial_baudrate=None
            #    raise SystemExit(0)


            try:
                if ( board.transportbluetooth.active):
                    print "Bluetooth Transport", board.transportbluetooth
                    self.bluetooth_name=board.transportbluetooth.name
                    self.transport_name="bluetooth"

            except ObjectDoesNotExist:
                print "transport bluetooth not present"
                self.bluetooth_name=None
                #raise SystemExit(0)

            if not transport_name is None:
                self.transport_name=transport_name

            for sensor in board.sensor_set.all():
                if not sensor.active: continue
                #name,i2cbus,address,timerange,level
                #TODO sensor.type ??
                self.drivers.append({"driver":sensor.driver,
                                     "type":sensor.type.type,
                                     "i2cbus":sensor.i2cbus,
                                     "address":sensor.address,
                                     "node":sensor.node,
                                     "timerange":sensor.timerange,
                                     "level":sensor.level})
    def ismobile(self):
        return self.network == "mobile"

    def display(self):

            print "station: >>>>>>>>"

            print "lon:",self.lon
            print "lat:",self.lat
            print "mqtt ident:",self.mqtt_ident
            print "prefix:",self.prefix
            print "maintprefix",self.maintprefix
            print "network:",self.network


            print "board: >>>>>>>>"
            try:
                print "sampletime:",self.mqtt_sampletime
            except:
                pass
            try:
                print "host:",self.mqtt_host
            except:
                pass
            try:
                print "user:",self.mqtt_user
            except:
                pass
            try:
                print "password:",self.mqtt_password
            except:
                pass
            try:
                print ""
                print "transport:",self.transport_name
            except:
                pass

            if self.transport_name == "bluetooth":
                print "bluetooth_name:",self.bluetooth_name

            if self.transport_name == "serial":
                print "serial_device:",self.serial_device
                print "serial_baudrate:",self.serial_baudrate

            if self.transport_name == "tcpip":
                print "tcpip_name:",self.tcpip_name

            print ">>>> sensors:"
            print self.drivers


    def rpcin(self, message, *args):
        """
        Get a message from osc channel
        """
        print "RPC: ",message[2]
        self.rpcin_message=message[2]

    def rpcout(self,message,*args):
        """
        Send a message to osc channel
        """
        osc.sendMsg('/rpc',[message, ],port=3001)

    def on_stop(self):
        '''
        called on application stop
        Here you can save data if needed
        '''
        print ">>>>>>>>> called on application stop"

        try:
            self.stoptransport()
            print "transport stopped"
        except:
            print "stop transport failed"

        # this seems required by android >= 5
        if self.bluetooth:
            self.bluetooth.close()

        self.stopmqtt()
        print "mqtt stopped"

        self.gps.stop()
        print "gps stopped"

        #self.br.stop()

        print "start save common parameters"
        with open( self.picklefile, "wb" ) as file:
            pickle.dump( self.anavarlist, file )
            pickle.dump( self.datavarlist, file )
            pickle.dump( self.trip, file )
            pickle.dump( self.slug, file )
            pickle.dump( self.boardslug, file )
        print "end save common parameters"

    def on_pause(self):
        '''
        called on application pause
        Here you can save data if needed
        '''
        print ">>>>>>>>> called on application pause"

        self.on_stop()

        return True

    def on_resume(self):
        '''
        called on appication resume
        Here you can check if any data needs replacing (usually nothing)
        '''
        print ">>>>>>>>> called on appication resume"

        #self.br.start()
        try:
            if os.path.isfile(self.picklefile):
                with open( self.picklefile, "rb" ) as file:
                    self.anavarlist= pickle.load( file )
                    self.datavarlist= pickle.load( file )
                    self.trip= pickle.load( file )
                    self.slug= pickle.load( file )
                    self.boardslug= pickle.load( file )
            else:
                print "file ",self.picklefile," do not exist"
        except:
            print "ERROR loading saved data"
            self.anavarlist=[]
            self.datavarlist=[]
            self.trip=False
            self.slug= "BT_fixed"
            self.boardslug= "BT_fixed"


    def configurestation(self,board_slug=None,username=None):
        """
        configure the board with those steps:
        * reset configuration
        * configure board
        * save configuration
        """

        try:
            self.starttransport()
            if board_slug is None:
                board_slug=self.boardslug
            if username is None:
                username=self.username

            print "configstation:",self.slug,board_slug,board_slug,username
            rmap_core.configstation(station_slug=self.slug,
                                    board_slug=board_slug,
                                    transport=self.transport,
                                    logfunc=self.log,username=username)

        except Exception as e:
            print "error in configure:"
            print e
            raise

        finally:
            self.stoptransport()


    def sensorssetup(self):
        """
        Setup of all sensors.
        This should be done at startup 
        """

        self.sensors=[]
        for driver in self.drivers:
            try:
                print "driver: ",driver

                if driver["driver"] == "JRPC":
                    print "found JRPC driver; setup for bridged RPC"
                    sd =SensorDriver.factory(driver["driver"],transport=self.transport)
                    # change transport !
                    sd.setup(driver="I2C",node=driver["node"],type=driver["type"],address=driver["address"])

                elif driver["driver"] == "RF24":
                    print "found RF24 driver; setup for bridged RPC"
                    sd =SensorDriver.factory("JRPC",transport=self.transport)
                    # change transport !
                    sd.setup(driver=driver["driver"],node=driver["node"],type=driver["type"],address=driver["address"])

                else:
                    sd =SensorDriver.factory(driver["driver"],type=driver["type"])
                    sd.setup(i2cbus=driver["i2cbus"],address=driver["address"])

                self.sensors.append({"driver":sd,"timerange":driver["timerange"],"level":driver["level"]})

            except:
                print "error in setup; sensor disabled:",\
                " driver=",driver["driver"],\
                " node=",driver["node"],\
                " type=",driver["type"],\
                " address=",driver["address"]
                raise Exception("sensors setup",1)


    def getdata(self,trip=None,now=None):
        """
        get data for all sensors with those steps:
      
        * prepare sensors
        * wait for samples are ready
        * get samples

        return False if the transport never works (used to reconnect bluetooth) 
        """

        print ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>getdata"

        if trip is None:
            trip=self.trip

        if trip and not self.ismobile():
            print "trip with fix station: do nothing"
            return True,{}

        if not trip and self.ismobile():
            print "not on trip with mobile station: do nothing"
            return True,{}

        if trip and self.gps.gpsfix:
            self.lat=self.gps.lat
            self.lon=self.gps.lon
            #self.anavar["B07030"]={"v": self.gps.height}   # add
            #self.anavar={"B07030":{"v": self.gps.height}}   # overwrite
            self.anavarlist.append({"coord":{"lat":self.lat,"lon":self.lon},"anavar":{"B07030":{"v": self.gps.height}}})

        elif trip:
            print "we have lost gps during a trip"
            return True,{}

        dt=0
        connected=False
        if now is None:
            now=datetime.utcnow().replace(microsecond=0)

        for sensor in self.sensors:
            print "prepare: ",sensor
            try:
                dt=max(sensor["driver"].prepare(),dt)
                connected=True

            except Exception as e:
                print e
                print "ERROR executing prepare rpc"
                traceback.print_exc()

        print "sleep ms:",dt
        time.sleep(dt/1000.)

        #message=""
        datavars=[]
        for sensor in self.sensors:
            try:
              for btable,value in sensor["driver"].get().iteritems():
                datavar={btable:{"t": now,"v": value}}
                datavars.append(datavar)
                self.datavarlist.append({"coord":{"lat":self.lat,"lon":self.lon},"timerange":sensor["timerange"],\
                                         "level":sensor["level"],"datavar":datavar})
#                stringa =""
#                for btable,data in datavar.iteritems():
#                    stringa += btable+": "+ data["t"].strftime("%d/%m/%y %H:%M:%S")+" -> "+str(data["v"])+"\n"
#                message=stringa

            except Exception as e:
                print e
                print "ERROR executing getJson rpc"
                traceback.print_exc()

        return connected,datavars


#    def on_broadcast(self,context,intent):
#
#        print "BLUETOOTH disconnected"
#
#        extras=intent.getExtras()
#        device= extras.get("device")
#
#        print "BLUETOOTH received:",device


    def startmqtt(self):
        '''
        begin mqtt connection
        and publish constat station data
        '''
        print ">>>>>>> startmqtt"

        #config = self.config

        if self.lat is None or self.lon is None:
            print "you have to set LAT and LON"
            self.mqtt_status = _('Connect Status: ERROR, you have to define a location !')
            return

        self.stopmqtt()

        self.mqtt_status = _('Connect Status: connecting')

        try:
            self.rmap=rmapmqtt(ident=self.mqtt_ident,lon=self.lon,lat=self.lat,network=self.network,
                                           host=self.mqtt_host,username=self.mqtt_user,password=self.mqtt_password,
                                           prefix=self.prefix,maintprefix=self.maintprefix,logfunc=self.log)

            self.rmap.loop_start()

        except:
            pass

    def publishmqtt(self):
        '''
        publish data on mqtt server
        '''

        # here we generate randon data to test only 
        #-----------------------------------------------------------
#        timerange="254,0,0"               # dati istantanei
#        level="105,2,-,-"                 # livello fittizio 2
#
#        value=random.randint(25315,30000) # tempertaure in cent K
#        datavar={"B12101":
#                 {
#                     "t": datetime.utcnow(),
#                     "v": value,
#                     "a": {
#                         "B33194": "90",           # attributi di qualita' del dato
#                         "B33195": "85"
#                     }   
#                 }}

        #-----------------------------------------------------------


        #print "queue ana :",self.anavarlist
        #print "queue data:",self.datavarlist

        try:
            if self.rmap.connected:

                newanavarlist=[]
                for item in self.anavarlist:
                    print "try to publish",item
                    try:
                        self.rmap.ana(item["anavar"],lon=item["coord"]["lon"],lat=item["coord"]["lat"])

                    except:
                        newdatavarlist.append(item)
                        self.mqtt_status =_('Connect Status: ERROR on Publish')

                self.anavarlist=newanavarlist


                newdatavarlist=[]
                for item in self.datavarlist:
                    print "try to publish",item
                    try:
                        self.rmap.data(item["timerange"],item["level"],item["datavar"],lon=item["coord"]["lon"],lat=item["coord"]["lat"],prefix=item.get("prefix",None))
                        self.mqtt_status = _('Connect Status: Published')
                        print "pubblicato", item["datavar"]
                    except:
                        newdatavarlist.append(item)
                        self.mqtt_status =_('Connect Status: ERROR on Publish')

                self.datavarlist=newdatavarlist
            else:
                self.mqtt_status = _('Connect Status: ERROR on Publish, not connected')

        except:
            self.mqtt_status = _('Connect Status: ERROR on Publish')


    def stopmqtt(self):
        '''
        disconnect from mqtt server
        '''

        try:
            #if self.rmap.connected:
            self.rmap.disconnect()
        except:
            pass

        try:
            self.rmap.loop_stop()
        except:
            pass

        #force status and messages because we do not wait ACK
        self.connected=False
        self.mqtt_status = _('Connect Status: disconnected')


    #@mainthread
    def publishmqtt_loop(self, *args):
        '''
        call this to publish data 
        '''

        if self.rmap.connected:
            print "mqtt connected"
        else:
            #print "mqtt reconnect"
            #try:
            #    self.rmap.mqttc.reconnect()
            #except:
            #    print "error on reconnect"

            print "try to restart mqtt"
            self.startmqtt()

        if self.rmap.connected:
            self.mqtt_status = _('Connect Status: OK connected')
        else:
            self.mqtt_status = _('Connect Status: disconnected')

        self.publishmqtt()

        return True


    def getdata_loop(self, trip=None):
        '''
        This function manage jsonrpc.
        With bluetooth transport manage reconnection
        '''

        if self.transport_name == "bluetooth":
            if self.bluetooth.bluetooth is None:
                print "Bluetooth try to reconnect"
                self.transport=self.bluetooth.connect()
                if self.transport is None:
                    print("bluetooth disabled")
                    return True
                else:
                    try:
                        self.sensorssetup()
                    except:
                        print "sensorssetup failed"

        connected,datavars = self.getdata(trip,self.now)

        if not connected and self.transport_name == "bluetooth":
            self.bluetooth.close()
            self.transport=self.bluetooth.connect()
            try:
                self.sensorssetup()
            except:
                print "sensorssetup failed"

        return datavars

    def loop(self, *args):
        '''
        This function manage jsonrpc and mqtt messages.
        '''
        print "call in loop"

        self.getdata_loop()
        self.publishmqtt_loop()

        return True


    def starttransport(self):

        """
        start transport
        """
        print ">>>>>>> start transport ",self.transport_name

        if self.transport_name == "bluetooth":
            print "start bluetooth"
            self.bluetooth=androbluetooth(name=self.bluetooth_name, logfunc=self.log)
            self.transport=self.bluetooth.connect()

        if self.transport_name == "tcpip":
            print "start tcpip"
            self.transport=jsonrpc.TransportTcpIp(addr=(self.tcpip_name,1000),timeout=3, logfunc=self.log)

        if self.transport_name == "serial":
            print "start serial"
            self.transport=jsonrpc.TransportSERIAL(port=self.serial_device,baudrate=self.serial_baudrate, logfunc=self.log)

        if self.transport_name == "mqtt":
            print "start mqtt"
            self.transport=jsonrpc.TransportMQTT(user=self.mqtt_user,
                                                 password=self.mqtt_password,
                                                 host=self.mqtt_host,
                                                 logfunc=self.log)

        if self.transport is None:
            raise Exception("start transport",1)


    def stoptransport(self):
        """
        stop transport
        """

        try:
            self.transport.close()
        except:
            print "ERROR closing transport"
            raise Exception("stop transport",1)


    def boot(self,configurestation=False):

        print "background boot station"

        ####   osc IPC   ####
        osc.init()
        self.oscid = osc.listen(ipAddr='0.0.0.0', port=3000)
        osc.bind(self.oscid, self.rpcin, '/rpc')

        #force trip for mobile station in background
        self.trip=self.ismobile()

        try:
            self.starttransport()
        except:
            print "start transport failed"

        notok=True
        while notok:
            try:
                if configurestation:
                    self.configurestation()
                try:
                    self.sensorssetup()
                except:
                    print "sensorssetup failed"

                if self.trip:
                    self.gps.start()
                self.startmqtt()
                notok=False
            except:
                print "Error booting station"
                time.sleep(5)

                osc.readQueue(self.oscid)
                if self.rpcin_message == "stop":
                    print "received stop message from rpc"
                    self.on_stop()
                    print "send stopped message to rpc"
                    self.rpcout("stopped")
                    raise SystemExit(0)
                    #time.sleep(60) # wait for kill from father

                if self.transport_name == "bluetooth":
                    self.bluetooth.close()
                    self.transport=self.bluetooth.connect()
                    try:
                        self.sensorssetup()
                    except:
                        print "sensorssetup failed"

        print "background end boot"

    def loopforever(self):

        # wait until a "even" datetime and set nexttime      
        now=datetime.utcnow()
        print "now:",now
        nexttime=now+timedelta(seconds=self.mqtt_sampletime)
        nextsec=int(nexttime.second/self.mqtt_sampletime)*self.mqtt_sampletime
        nexttime=nexttime.replace(second=nextsec,microsecond=0)
        print "nexttime:",nexttime
        waitsec=(max((nexttime - datetime.utcnow()),timedelta())).total_seconds()
        print "wait for:",waitsec
        time.sleep(waitsec)
        print "now:",datetime.utcnow()

        self.now=nexttime

        #self.rmap.loop(timeout=waitsec)

        # TODO
        # enable STOP SERVICE DURING SLEEP

        while True:
            try:
    
                #override "even" datetime with more precise but "odd" datetime
                #self.now=datetime.utcnow()
                #print "now:",self.now

                self.loop()

                print "backgroud loop"
                message=self.mqtt_status

                title="Rmap last status"
                if self.transport_name == "bluetooth":
                    title= self.bluetooth.bluetooth_status

                print "notification title:"
                print title
                print "notification message:"
                print message

                do_notify(message,title)

            except Exception as e:
                print e
                print "ERROR in main loop!"
                traceback.print_exc()

            print "now:",datetime.utcnow()
            nexttime=nexttime+timedelta(seconds=self.mqtt_sampletime)

            self.sleep_and_check_stop(nexttime)
            self.now=nexttime


    def sleep_and_check_stop(self,nexttime):
        waitsec=(max((nexttime - datetime.utcnow()),timedelta())).total_seconds()
        print "wait for:",waitsec
        #time.sleep(waitsec)
        stop=False
        while (datetime.utcnow() < nexttime):
            osc.readQueue(self.oscid)
            if self.rpcin_message == "stop":
                stop=True
                break
            time.sleep(.5)

        osc.readQueue(self.oscid)
        if self.rpcin_message == "stop":
            stop=True

        if (not stop):
            print "continue on loop"
            return

        print "start shutdown background process"
        try:
            self.on_stop()
        except:
            print "error on_stop"
        print "retuned from on_stop"

        print "RPC send stoppped"
        self.rpcout("stopped")
        print "background exit"
        raise SystemExit(0)
        #time.sleep(s30)

    def exit(self, *args):
        try:
            self.on_stop()
        except:
            print "error on_stop"
        raise SystemExit(0)


#class RequestHandler(WSGIRequestHandler):
#    pass


def main():

    import os
    import random

    from django.conf import settings
    from django.utils import translation
    from django.core import management

    os.environ['DJANGO_SETTINGS_MODULE'] = 'settings'
    import django
    django.setup()

    django.utils.translation.activate("it")

    lon=11.36992
    lat=44.48906
    host="rmap.cc"

    anavar={
        "B07030":{"v": "400"}
    }

    try:
        rmap=rmapmqtt(lon=lon,lat=lat,host=host)
        rmap.loop_start()

        rmap.ana(anavar)
        time.sleep(5)
        #rmap.loop()

        reptime=datetime.now()
        endtime=reptime+timedelta(days=1)
    
        while reptime <= endtime:

            print "connect status: ",rmap.connected
            timerange="254,0,0"               # dati istantanei
            level="103,2000,-,-"              # 2m dal suolo
            value=random.randint(25315,30000) # tempertaure in cent K
            datavar={"B12101":
            {
                "t": reptime,
                "v": value,
                "a": {
                    "B33194": "90",           # attributi di qualita' del dato
                    "B33195": "85"
                }   
            }}

            rmap.data(timerange,level,datavar)
            time.sleep(5)
            #rmap.loop()
            reptime=datetime.now()

        rmap.disconnect()
        rmap.loop_stop()
        print "work is done OK"

    except:
        print "terminated with error"
        raise

if __name__ == '__main__':
    main()  # (this code was run as script)
