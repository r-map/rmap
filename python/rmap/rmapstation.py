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
import re

#import threading # https://github.com/kivy/kivy/wiki/Working-with-Python-threads-inside-a-Kivy-application

import settings
import jsonrpc
from sensordriver import SensorDriver
from stations.models import StationMetadata
from gps import *
from bluetooth import *

from plyer import notification
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
import codecs
#import mosquitto
import paho.mqtt.client as mqtt
from utils import nint
import rmap.rmap_core

# Encoder per la data
class JSONEncoder(json.JSONEncoder):
    def default(self, o): 
        if isinstance(o, datetime):
            return o.strftime("%Y-%m-%dT%H:%M:%S")
        else:
            return json.JSONEncoder.default(o)
# Funzione per fare il dump in JSON
def dumps(o):
    return json.dumps(o, cls=JSONEncoder)

class Rmapdonotexist(Exception):
    pass


def log_dummy( message ):
    """dummy-logger: do nothing"""
    pass
def log_stdout( message ):
    """print message to STDOUT"""
    print message

def log_file( filename ):
    """return a logfunc which logs to a file (in utf-8)"""
    def logfile( message ):
        f = codecs.open( filename, 'a', encoding='utf-8' )
        f.write( message+"\n" )
        f.close()
    return logfile

def log_filedate( filename ):
    """return a logfunc which logs date+message to a file (in utf-8)"""
    def logfile( message ):
        f = codecs.open( filename, 'a', encoding='utf-8' )
        f.write( time.strftime("%Y-%m-%d %H:%M:%S ")+message+"\n" )
        f.close()
    return logfile


class rmapmqtt:

    def __init__(self,ident="-",lon=None,lat=None,network="generic",host="localhost",port=1883,username=None,password=None,timeout=60,logfunc=log_stdout,clientid="",prefix="test",maintprefix="test"):

        self.ident=ident
        self.lonlat="%d,%d" % (nint(lon*100000),nint(lat*100000))
        self.network=network
        self.host=host
        self.port=port
        self.username=username
        self.password=password
        self.timeout=timeout
        self.log    = logfunc
        self.prefix=prefix
        self.maintprefix=maintprefix
        self.connected=False
        self.mid=-1

        # If you want to use a specific client id, use
        # mqttc = mosquitto.Mosquitto("client-id")
        # but note that the client id must be unique on the broker. Leaving the client
        # id parameter empty will generate a random id for you.
        #self.mqttc = mosquitto.Mosquitto(clientid)
        self.mqttc = mqtt.Client(clientid)

        self.mqttc.on_message = self.on_message
        self.mqttc.on_connect = self.on_connect
        self.mqttc.on_disconnect = self.on_disconnect
        self.mqttc.on_publish = self.on_publish
        self.mqttc.on_subscribe = self.on_subscribe
        # Uncomment to enable debug messages
        self.mqttc.on_log = self.on_log

        # Imposto le credenziali
        if (not self.username is None):
            self.mqttc.username_pw_set(self.username,self.password)

        #self.mqttc.max_inflight_messages_set(1)

        # mando stato di connessione della stazione con segnalazione di sconnessione gestita male com will
        self.mqttc.will_set(self.maintprefix+"/"+self.ident+"/"+self.lonlat+"/"+self.network+"/-,-,-/-,-,-,-/B01213",
                    payload=dumps({"v": "error01"}),
                       qos=1, retain=True)

        try:
            print "start connect"
            #self.mqttc.connect_async(self.host,self.port,self.timeout)
            rc=self.mqttc.connect(self.host,self.port,self.timeout)
            print "end connect"
            if rc != mqtt.MQTT_ERR_SUCCESS:
                raise Exception("connect",rc)

            rc=self.mqttc.publish(self.maintprefix+"/"+self.ident+"/"+self.lonlat+"/"+self.network+"/-,-,-/-,-,-,-/B01213",
                             payload=dumps({ "v": "conn"}),
                             qos=1,retain=True)

            if rc[0] != mqtt.MQTT_ERR_SUCCESS:
                raise Exception("publish status",rc)

            self.log("publish maint message mid: "+str(rc[1]))

        except Exception as inst:
            self.error(inst)

    def publish(self,topic,payload,qos=0,retain=False,timeout=3.):
        ''' 
        bloking publish
        with qos > 0 we wait for ack
        '''
        self.puback=False
        rc,self.mid=self.mqttc.publish(topic,payload=payload,qos=qos,retain=retain)
        if rc != mqtt.MQTT_ERR_SUCCESS:
            return rc
        if (qos == 0 ):
            return rc

        self.log("publish ana message mid: "+str(self.mid))

        last=time.time()
        while ((time.time()-last) < timeout and not self.puback):
            time.sleep(.1)

        if (not self.puback):
            return 50
        else:
            return  mqtt.MQTT_ERR_SUCCESS

    def ana(self,anavar={},lon=None,lat=None):

        try:
            if lon is not None and lat is not None:
                lonlat="%d,%d" % (nint(lon*100000),nint(lat*100000))
            else:
                lonlat=self.lonlat

            # mando dati di anagrafica retained

            for key,val in anavar.iteritems():
                rc=self.publish(self.prefix+"/"+self.ident+"/"+lonlat+"/"+self.network+"/-,-,-/-,-,-,-/"+key,
                                      payload=dumps(val),
                                      qos=1,retain=True)
                if rc != mqtt.MQTT_ERR_SUCCESS:
                    raise Exception("publish ana",rc)

        except Exception as inst:
            self.error(inst)


    def data(self,timerange=None,level=None,datavar={},lon=None,lat=None):

        try:

            # send data (temperature  for example) non retained

            if lon is not None and lat is not None:
                lonlat="%d,%d" % (nint(lon*100000),nint(lat*100000))
            else:
                lonlat=self.lonlat

            for key,val in datavar.iteritems():
                rc=self.publish(self.prefix+"/"+self.ident+"/"+lonlat+"/"+self.network+"/"+
                                      timerange+"/"+level+"/"+key,
                                      payload=dumps(val), 
                                      qos=1,
                                      retain=False
                                  )
            
                if rc != mqtt.MQTT_ERR_SUCCESS:
                    raise Exception("publish data",rc)

            #rc = self.mqttc.loop()
            #if rc != mqtt.MQTT_ERR_SUCCESS:
            #    raise Exception("loop",rc)

        except Exception as inst:
            self.error(inst)

    def loop(self,timeout=1):
        """
        Use this as alternative to loop_start and loop_stop.
        You have to call loop to mantain mqtt protocol.
        This function is the driving force behind the mqtt client. If they
        are not called, incoming network data will not be processed
        and outgoing network data may not be sent in a timely fashion.
        """

        try:
            rc = self.mqttc.loop(timeout)
            #if rc != mqtt.MQTT_ERR_SUCCESS:
                #raise Exception("loop",rc)
        
        except Exception as inst:
            self.error(inst)



    def loop_start(self):
        """
        this start a thead to mantain mqtt protocol
        """
        try:
            rc = self.mqttc.loop_start()
        except Exception as inst:
            self.error(inst)

    def loop_stop(self):

        try:
            rc = self.mqttc.loop_stop(force=True)
        except Exception as inst:
            self.error(inst)        

    def disconnect(self):

        try:

            #clean disconnect
            rc=self.mqttc.publish(self.maintprefix+"/"+self.ident+"/"+self.lonlat+"/"+self.network+"/-,-,-/-,-,-,-/B01213",
                             payload=dumps({ "v": "disconn"}),
                             qos=1,retain=True)
            if rc[0] != mqtt.MQTT_ERR_SUCCESS:
                raise Exception("publish status",rc)

            self.log("publish maint message mid: "+str(rc[1]))

            #rc = self.mqttc.loop()
            #if rc != mqtt.MQTT_ERR_SUCCESS:
            #    raise Exception("loop",rc)

            rc = self.mqttc.disconnect()
            if rc != mqtt.MQTT_ERR_SUCCESS:
                raise Exception("disconnect",rc)

        except Exception as inst:
            self.error(inst)


    def on_connect(self,mosq, userdata,flags, rc):
        self.log("connect rc: "+str(rc))
        #self.log(mqtt.error_string(rc))

        if not self.connected:
            try:
                rc=self.mqttc.publish(self.maintprefix+"/"+self.ident+"/"+self.lonlat+"/"+self.network+"/-,-,-/-,-,-,-/B01213",
                             payload=dumps({ "v": "conn"}),
                             qos=1,retain=True)

                if rc[0] != mqtt.MQTT_ERR_SUCCESS:
                    raise Exception("publish status",rc)

                self.log("publish maint message mid: "+str(rc[1]))

            except Exception as inst:
                self.error(inst)

        print "--------------------------------> connected"
        self.connected=True


    def on_disconnect(self,mosq, userdata, rc):
        self.log("disconnect rc: "+str(rc))

        print "--------------------------------> disconnected"
        self.connected=False

        #if rc == 1 :
        #    try:
        #        rc=self.mqttc.connect(self.host,self.port,self.timeout)
        #        if rc != mqtt.MQTT_ERR_SUCCESS:
        #            raise Exception("connect",rc)

        #        # stato della connessione
        #        self.connected=True
        #        rc=self.mqttc.publish(self.maintprefix+"/"+self.ident+"/"+self.lonlat+"/"+self.network+"/-,-,-/-,-,-,-/B01213",
        #                              payload=dumps({ "v": "conn"}),
        #                              qos=1,retain=True)

        #        if rc[0] != mqtt.MQTT_ERR_SUCCESS:
        #            raise Exception("publish status",rc)

        #    except Exception as inst:
        #        self.error(inst)


    def on_message(self,mosq, userdata, msg):
        self.log("message: "+msg.topic+" "+str(msg.qos)+" "+str(msg.payload))

    def on_publish(self,mosq, userdata, mid):
        self.log("published mid: "+str(mid))
        if (mid == self.mid):
            self.puback=True

    def on_subscribe(self,mosq, userdata, mid, granted_qos):
        self.log("Subscribed: "+str(mid)+" "+str(granted_qos))

    def on_log(self,mosq, userdata, level, string):
        self.log("log: "+string)


    def error(self,inst=None):

        self.log("Error !!!!!!!!!")
        self.log(str(inst))
        raise


def do_notify(message="",title="Notification"):

    """
    Notify a message with title and message
    """

    if PY2:
        title = title.decode('utf8')
        message = message.decode('utf8')
    kwargs = {'title': title, 'message': message}

    try:
        notification.notify(**kwargs)
    except:
        print "error on notify message:",title, message



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

    def __init__(self,slug=None,boardslug=None, picklefile="saveddata-service.pickle",trip=None,gps=plyergps(),transport_name=None,logfunc=jsonrpc.log_stdout):
        '''
        do all startup operations
        '''

        print "INITIALIZE rmap station"
        self.picklefile=picklefile

        self.anavarlist=[]
        self.datavarlist=[]
        self.bluetooth=None
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
            mystation=StationMetadata.objects.get(slug=self.slug)
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
                                     "type":sensor.type,
                                     "i2cbus":sensor.i2cbus,
                                     "address":sensor.address,
                                     "node":sensor.node,
                                     "timerange":sensor.timerange,
                                     "level":sensor.level})
                

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


    def configurestation(self,board_slug=None):
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
            rmap.rmap_core.configstation(station_slug=self.slug,
                            board_slug=board_slug,
                            transport=self.transport,
                            logfunc=self.log)

        except:
            print "error in configure:"
            raise

        finally:
            self.stoptransport()


    def sensorssetup(self):
        """
        Setup of all sensors.
        This should be done al startup 
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
            except:
                print "ERROR executing prepare rpc"

        print "sleep ms:",dt
        time.sleep(dt/1000.)

        #message=""
        datavars=[]
        for sensor in self.sensors:
            try:
              for btable,value in sensor["driver"].get().iteritems():
                datavar={btable:{"t": now,"v": str(value)}}
                datavars.append(datavar)
                self.datavarlist.append({"coord":{"lat":self.lat,"lon":self.lon},"timerange":sensor["timerange"],\
                                         "level":sensor["level"],"datavar":datavar})
#                stringa =""
#                for btable,data in datavar.iteritems():
#                    stringa += btable+": "+ data["t"].strftime("%d/%m/%y %H:%M:%S")+" -> "+str(data["v"])+"\n"
#                message=stringa

            except:
                print "ERROR executing getJson rpc"

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
#                     "v": str(value),
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
                        self.rmap.data(item["timerange"],item["level"],item["datavar"],lon=item["coord"]["lon"],lat=item["coord"]["lat"])
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
            if self.rmap.connected:
                self.rmap.disconnect()
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

            except:
                print "ERROR in main loop!"

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
            print ">>>>> ----- rpcin background message: ", self.rpcin_message
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
    os.environ['DJANGO_SETTINGS_MODULE'] = 'settings'
    from django.conf import settings
    from django.utils import translation
    from django.core import management
    import django
    django.setup()
    django.utils.translation.activate("it")

    import random

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
                "v": str(value),
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
