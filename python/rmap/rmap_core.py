#!/usr/bin/python
# GPL. (C) 2014 Paolo Patruno.

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

from django.db import IntegrityError
from django.core.exceptions import ObjectDoesNotExist
from stations.models import StationMetadata
from stations.models import StationConstantData
from stations.models import Board
from stations.models import Sensor, SensorType
from stations.models import TransportMqtt
from stations.models import TransportBluetooth
from stations.models import TransportAmqp
from stations.models import TransportSerial
from stations.models import TransportTcpip
from django.contrib.auth.models import User
from django.core import serializers
from django.utils.translation import ugettext as _
import pika
from rmap.utils import nint
from rmap import jsonrpc
from django.core.files.base import ContentFile
from datetime import datetime
import exifutils
from django.db import connection
#from django.contrib.sites.shortcuts import get_current_site
#from django.contrib.sites.models import Site

#sensortemplates={"stima_t_u":
#'''
#[
#    {
#        "fields": {
#            "node": 1,
#            "name": "Stima T",
#            "level": "105,2000,-,-",
#            "timerange": "254,0,0",
#            "driver": "I2C",
#            "i2cbus": 1,
#            "board": [
#                "base",
#                [
#                    "home",
#                    [
#                        "rmap"
#                    ]
#                ]
#            ],
#            "address": 72,
#            "active": true,
#            "type": "ADT"
#        },
#        "model": "stations.sensor"
#    },
#    {
#        "fields": {
#            "node": 1,
#            "name": "Stima U",
#            "level": "105,2000,-,-",
#            "timerange": "254,0,0",
#            "driver": "I2C",
#            "i2cbus": 1,
#            "board": [
#                "base",
#                [
#                    "home",
#                    [
#                        "rmap"
#                    ]
#                ]
#            ],
#            "address": 39,
#            "active": true,
#            "type": "HIH"
#        },
#        "model": "stations.sensor"
#    }
#]
#'''
#,
#"stima_wind":
#'''
#[
#    {
#        "fields": {
#            "node": 1,
#            "name": "Stima wind",
#            "level": "105,10000,-,-",
#            "timerange": "254,0,0",
#            "driver": "I2C",
#            "i2cbus": 1,
#            "board": [
#                "base",
#                [
#                    "home",
#                    [
#                        "rmap"
#                    ]
#                ]
#            ],
#            "address": 22,
#            "active": true,
#            "type": "win"
#        },
#        "model": "stations.sensor"
#    }
#]
#'''
#,
#"stima_rain":
#'''
#[
#    {
#        "fields": {
#            "node": 1,
#            "name": "Stima rain",
#            "level": "105,2000,-,-",
#            "timerange": "254,0,0",
#            "driver": "I2C",
#            "i2cbus": 1,
#            "board": [
#                "base",
#                [
#                    "home",
#                    [
#                        "rmap"
#                    ]
#                ]
#            ],
#            "address": 22,
#            "active": true,
#            "type": "rai"
#        },
#        "model": "stations.sensor"
#    }
#]
#'''
#}


#def isRainboInstance(request):
#    #name=get_current_site(request).name
#    domain=request.get_host()
#    name=Site.objects.get(domain=domain).name
#    print "MY SITE:", name
#    return name == "rainbo"


def delsensor(station_slug=None,username=None,board_slug=None,name=None):

    Sensor.objects.get(name=name,board__slug=board_slug
                                ,board__stationmetadata__slug=station_slug
                                ,board__stationmetadata__ident__username=username).delete()

def delsensors(station_slug=None,username=None,board_slug=None):

    Sensor.objects.filter(board__slug=board_slug
                                ,board__stationmetadata__slug=station_slug
                                ,board__stationmetadata__ident__username=username).delete()


def addboard(station_slug=None,username=None,board_slug=None,activate=False
              ,serialactivate=False
              ,mqttactivate=False, mqttserver="rmap.cc", mqttusername=None, mqttpassword=None, mqttsamplerate=5
              ,bluetoothactivate=False, bluetoothname="HC-05"
              ,amqpactivate=False, amqpusername="rmap", amqppassword=None, amqpserver="rmap.cc", queue="rmap", exchange="rmap"
              ,tcpipactivate=False, tcpipname="master", tcpipntpserver="ntpserver"
          ):

    print "---------------------------"
    print station_slug,username,board_slug
    print "---------------------------"

    try:
        myboard = Board.objects.get(slug=board_slug
                                    ,stationmetadata__slug=station_slug
                                    ,stationmetadata__ident__username=username)
    except ObjectDoesNotExist :
        mystation=StationMetadata.objects.get(slug=station_slug,ident__username=username)
        myboard=Board(name=board_slug,slug=board_slug,stationmetadata=mystation,active=activate)
        myboard.save()
        myboard = Board.objects.get(slug=board_slug
                                    ,stationmetadata__slug=station_slug
                                    ,stationmetadata__ident__username=username)


    try:
        transportserial=myboard.transportserial
    except ObjectDoesNotExist :
        transportserial=TransportSerial()

    transportserial.active=serialactivate
    myboard.transportserial=transportserial
    print "Serial Transport", myboard.transportserial
    myboard.transportserial.save()

    try:
        transportmqtt=myboard.transportmqtt
    except ObjectDoesNotExist :
        transportmqtt=TransportMqtt()

    transportmqtt.active=mqttactivate
    transportmqtt.mqttserver=mqttserver
    transportmqtt.mqttuser=mqttusername
    transportmqtt.mqttpassword=mqttpassword
    transportmqtt.mqttsampletime=mqttsamplerate
    myboard.transportmqtt=transportmqtt
    print "MQTT Transport", myboard.transportmqtt
    myboard.transportmqtt.save()
                
    try:
        transportbluetooth=myboard.transportbluetooth
    except ObjectDoesNotExist :
        transportbluetooth=TransportBluetooth()
    transportbluetooth.active=bluetoothactivate
    transportbluetooth.name=bluetoothname
    myboard.transportbluetooth=transportbluetooth
    print "bluetooth Transport", myboard.transportbluetooth
    myboard.transportbluetooth.save()


    try:
        transportamqp=myboard.transportamqp
    except ObjectDoesNotExist :
        transportamqp=TransportAmqp()
    transportamqp.active=amqpactivate
    transportamqp.amqpuser=amqpusername
    transportamqp.amqppassword=amqppassword
    transportamqp.amqpserver=amqpserver
    transportamqp.queue=queue
    transportamqp.exchange=exchange
    myboard.transportamqp=transportamqp
    print "AMQP Transport", myboard.transportamqp                
    myboard.transportamqp.save()

    try:
        transporttcpip=myboard.transporttcpip
    except ObjectDoesNotExist :
        transporttcpip=TransportTcpip()
    transporttcpip.active=tcpipactivate
    transporttcpip.name=tcpipname
    transporttcpip.ntpserver=tcpipntpserver
    myboard.transporttcpip=transporttcpip
    print "TCPIP Transport", myboard.transporttcpip                
    myboard.transporttcpip.save()
    

def addsensor(station_slug=None,username=None,board_slug=None,name="my sensor",driver="TMP",type="TMP",i2cbus=1,address=72,node=1
              ,timerange="254,0,0",level="0,1",activate=False
              ,mqttactivate=False, mqttserver="rmap.cc", mqttusername=None, mqttpassword=None, mqttsamplerate=5
              ,bluetoothactivate=False, bluetoothname="hc-05"
              ,amqpactivate=False, amqpusername="rmap", amqppassword=None, amqpserver="rmap.cc", queue="rmap", exchange="rmap"
          ):
    #,sensortemplate=None):

    print "---------------------------"
    print station_slug,username,board_slug
    print "---------------------------"

    try:
        myboard = Board.objects.get(slug=board_slug
                                    ,stationmetadata__slug=station_slug
                                    ,stationmetadata__ident__username=username)
    except ObjectDoesNotExist :
            print "board not present for this station"
            raise

    try:
        mytype = SensorType.objects.get(type=type)
    except ObjectDoesNotExist :
        print "sensor type: ",type,"  not present in DB"
        raise

    #if sensortemplate is None :
    mysensor=Sensor(board=myboard,active=True,name=name,driver=driver,type=mytype
                    ,i2cbus=i2cbus,address=address,node=node
                    ,timerange=timerange,level=level)

    mysensors=[mysensor]

    #else:
    #    mysensorstmp=serializers.deserialize("json",sensortemplates[sensortemplate])
    #    mysensors=[]
    #    for mysensor in mysensorstmp:
    #        #myboard.sensor_set.clear()
    #        #myboard.sensor_set.add(mysensor.object) 
    #
    #        mysensors.append(mysensor)

    for mysensor in mysensors:
        print "try to save:",mysensor
        try:
            mysensor.clean()
            mysensor.save()
        except IntegrityError:
            oldsensor=Sensor.objects.get(board=myboard,name=name)
            oldsensor.delete()
            mysensor.save()

# the first is the default
template_choices = [
    "default",
    "none",
    "test",    "test_indirect",    "test_rf24",    "test_master",    "test_base",
    "stima_base",    "stima_t",    "stima_h",    "stima_w",    "stima_r",    "stima_p",    "stima_s",    "stima_m",
    "stima_sm",    "stima_th",    "stima_y",    "stima_ths",    "stima_thsm",    "stima_thw",    "stima_thp",    "stima_yp",
    "stima_thwr",    "stima_thwrp",
    "stima_rf24_t",    "stima_rf24_h",    "stima_rf24_w",    "stima_rf24_r",    "stima_rf24_p",    "stima_rf24_th",    "stima_rf24_y",
    "stima_rf24_thw",    "stima_rf24_thp",    "stima_rf24_yp",    "stima_rf24_thwr",    "stima_rf24_thwrp",
    "stima_report_thp",
    "stima_indirect_t",    "stima_indirect_h",    "stima_indirect_r",    "stima_indirect_p",    "stima_indirect_s", "stima_indirect_m",
    "stima_indirect_sm", "stima_indirect_th",    "stima_indirect_y",    "stima_indirect_thw",    "stima_indirect_thp",    "stima_indirect_yp",    "stima_indirect_ths",    "stima_indirect_thsm",
    "stima_indirect_thwr",    "stima_indirect_thwrp",    "stima_indirect_report_thp",
]

def addsensors_by_template(station_slug=None,username=None,board_slug=None,template=None):

    if (template == "default"):
        print "setting template:", template," do not change sensors on db"
        pass

    if (template == "none"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)

    if (template == "test"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="stima test",driver="I2C",
                  type="TMP",address=72,timerange="254,0,0",level="0,1,-,-")

    if (template == "test_indirect"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="stima test jrpc",driver="JRPC",
                  type="TMP",address=72,timerange="254,0,0",level="0,1,-,-")

    if (template == "test_rf24"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="stima test rf24",driver="RF24",
                  type="TMP",address=72,timerange="254,0,0",level="0,2,-,-")

    if (template == "test_master"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="stima test",driver="I2C",
                  type="TMP",address=72,timerange="254,0,0",level="0,1,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="stima test rf24",driver="RF24",
                  type="TMP",address=72,timerange="254,0,0",level="0,2,-,-")

    if (template == "test_base"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="test Temperature",driver="I2C",
                  type="TMP",address=72,timerange="254,0,0",level="0,1,-,-")

    if (template == "stima_base"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature",driver="I2C",
                  type="ADT",address=73,timerange="254,0,0",level="103,1000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Humidity",driver="I2C",
                  type="HIH",address=39,timerange="254,0,0",level="103,1000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Pressure",driver="I2C",
                  type="BMP",address=119,timerange="254,0,0",level="1,-,-,-")

    if (template == "stima_t"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature",driver="I2C",
                  type="ADT",address=73,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_h"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Humidity",driver="I2C",
                  type="HIH",address=39,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_w"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Wind",driver="I2C",
                  type="DW1",address=34,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_r"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Precipitation",driver="I2C",
                  type="TBS",address=33,timerange="1,0,0",level="1,-,-,-")

    if (template == "stima_p"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Pressure",driver="I2C",
                  type="BMP",address=119,timerange="254,0,0",level="1,-,-,-")

    if (template == "stima_s"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Dust",driver="I2C",
                  type="SSD",address=36,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_m"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Nitrogen dioxide",driver="I2C",
                  type="SMI",address=36,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_sm"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Dust",driver="I2C",
                  type="SSD",address=36,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Nitrogen dioxide",driver="I2C",
                  type="SMI",address=36,timerange="254,0,0",level="103,2000,-,-")
        
    if (template == "stima_th"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature",driver="I2C",
                  type="ADT",address=73,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Humidity",driver="I2C",
                  type="HIH",address=39,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_y"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature, Humidity",driver="I2C",
                  type="HYT",address=40,timerange="254,0,0",level="103,2000,-,-")
        
    if (template == "stima_ths"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature",driver="I2C",
                  type="ADT",address=73,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Humidity",driver="I2C",
                  type="HIH",address=39,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Dust",driver="I2C",
                  type="SSD",address=36,timerange="254,0,0",level="103,2000,-,-")


    if (template == "stima_thsm"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature",driver="I2C",
                  type="ADT",address=73,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Humidity",driver="I2C",
                  type="HIH",address=39,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Dust",driver="I2C",
                  type="SSD",address=36,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Nitrogen dioxide",driver="I2C",
                  type="SMI",address=36,timerange="254,0,0",level="103,2000,-,-")

        
    if (template == "stima_thw"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature",driver="I2C",
                  type="ADT",address=73,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Humidity",driver="I2C",
                  type="HIH",address=39,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Wind",driver="I2C",
                  type="DW1",address=34,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_thp"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature",driver="I2C",
                  type="ADT",address=73,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Humidity",driver="I2C",
                  type="HIH",address=39,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Precipitation",driver="I2C",
                  type="TBS",address=33,timerange="1,0,0",level="1,-,-,-")


    if (template == "stima_yp"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature, Humidity",driver="I2C",
                  type="HYT",address=40,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="precipitation",driver="I2C",
                  type="TBS",address=33,timerange="1,0,0",level="1,-,-,-")

        
    if (template == "stima_thwr"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature",driver="I2C",
                  type="ADT",address=73,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Humidity",driver="I2C",
                  type="HIH",address=39,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Wind",driver="I2C",
                  type="DW1",address=34,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Precipitation",driver="I2C",
                  type="TBS",address=33,timerange="1,0,0",level="1,-,-,-")

    if (template == "stima_thwrp"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature",driver="I2C",
                  type="ADT",address=73,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Humidity",driver="I2C",
                  type="HIH",address=39,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Wind",driver="I2C",
                  type="DW1",address=34,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Precipitation",driver="I2C",
                  type="TBS",address=33,timerange="1,0,0",level="1,-,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Pressure",driver="I2C",
                  type="BMP",address=119,timerange="254,0,0",level="1,-,-,-")

    if (template == "stima_rf24_t"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="rf24 Temperature",driver="RF24",
                  type="ADT",address=73,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_rf24_h"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="rf24 Humidity",driver="RF24",
                  type="HIH",address=39,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_rf24_w"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="rf24 Wind",driver="RF24",
                  type="DW1",address=34,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_rf24_r"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="rf24 Precipitation",driver="RF24",
                  type="TBS",address=33,timerange="1,0,0",level="1,-,-,-")

    if (template == "stima_rf24_p"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="rf24 Pressure",driver="RF24",
                  type="BMP",address=119,timerange="254,0,0",level="1,-,-,-")

    if (template == "stima_rf24_th"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="rf24 Temperature",driver="RF24",
                  type="ADT",address=73,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="rf24 Humidity",driver="RF24",
                  type="HIH",address=39,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_rf24_y"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="rf24 Temperature, Humidity",driver="RF24",
                  type="HYT",address=40,timerange="254,0,0",level="103,2000,-,-")
        
    if (template == "stima_rf24_thw"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="rf24 Temperature",driver="RF24",
                  type="ADT",address=73,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="rf24 Humidity",driver="RF24",
                  type="HIH",address=39,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="rf24 Humidity",driver="RF24",
                  type="DW1",address=34,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_rf24_thp"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="rf24 Temperature",driver="RF24",
                  type="ADT",address=73,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="rf24 Humidity",driver="RF24",
                  type="HIH",address=39,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="rf24 Humidity",driver="RF24",
                  type="TBS",address=33,timerange="1,0,0",level="1,-,-,-")

    if (template == "stima_rf24_yp"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature, Humidity",driver="RF24",
                  type="HYT",address=40,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="precipitation",driver="RF24",
                  type="TBS",address=33,timerange="1,0,0",level="1,-,-,-")

        
    if (template == "stima_rf24_thwr"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="rf24 Temperature",driver="RF24",
                  type="ADT",address=73,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="rf24 Humidity",driver="RF24",
                  type="HIH",address=39,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="rf24 Wind",driver="RF24",
                  type="DW1",address=34,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="rf24 Precipitation",driver="RF24",
                  type="TBS",address=33,timerange="1,0,0",level="1,-,-,-")

    if (template == "stima_rf24_thwrp"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="rf24 Temperature",driver="RF24",
                  type="ADT",address=73,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="rf24 Humidity",driver="RF24",
                  type="HIH",address=39,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="rf24 Wind",driver="RF24",
                  type="DW1",address=34,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="rf24 Precipitation",driver="RF24",
                  type="TBS",address=33,timerange="1,0,0",level="1,-,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="rf24 Pressure",driver="RF24",
                  type="BMP",address=119,timerange="254,0,0",level="1,-,-,-")

    if (template == "stima_report_thp"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Temperature/Humidity report inst. values",driver="I2C",
                  type="ITH",address=35,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Temperature/Humidity report min values",driver="I2C",
                  type="NTH",address=35,timerange="3,0,900",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Temperature/Humidity report mean values",driver="I2C",
                  type="MTH",address=35,timerange="0,0,900",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Temperature/Humidity report max malues",driver="I2C",
                  type="XTH",address=35,timerange="2,0,900",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Precipitation report",driver="I2C",
                  type="TBR",address=33,timerange="1,0,900",level="1,-,-,-")



    if (template == "stima_indirect_t"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature",driver="JRPC",
                  type="ADT",address=73,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_indirect_h"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Humidity",driver="JRPC",
                  type="HIH",address=39,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_indirect_w"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Wind",driver="JRPC",
                  type="DW1",address=34,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_indirect_r"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Precipitation",driver="JRPC",
                  type="TBs",address=33,timerange="1,0,0",level="1,-,-,-")

    if (template == "stima_indirect_p"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Pressure",driver="JRPC",
                  type="BMP",address=119,timerange="254,0,0",level="1,-,-,-")

    if (template == "stima_indirect_s"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Dust",driver="JRPC",
                  type="SSD",address=36,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_indirect_m"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Nitrogen dioxide",driver="JRPC",
                  type="SMI",address=36,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_indirect_sm"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Dust",driver="JRPC",
                  type="SSD",address=36,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Nitrogen dioxide",driver="JRPC",
                  type="SMI",address=36,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_indirect_th"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature",driver="JRPC",
                  type="ADT",address=73,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Humidity",driver="JRPC",
                  type="HIH",address=39,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_indirect_y"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature, Humidity",driver="JRPC",
                  type="HYT",address=40,timerange="254,0,0",level="103,2000,-,-")
        
    if (template == "stima_indirect_thw"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature",driver="JRPC",
                  type="ADT",address=73,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Humidity",driver="JRPC",
                  type="HIH",address=39,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Humidity",driver="JRPC",
                  type="DW1",address=34,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_indirect_thp"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature",driver="JRPC",
                  type="ADT",address=73,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Humidity",driver="JRPC",
                  type="HIH",address=39,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Humidity",driver="JRPC",
                  type="TBS",address=33,timerange="1,0,0",level="1,-,-,-")

    if (template == "stima_indirect_yp"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature, Humidity",driver="JRPC",
                  type="HYT",address=40,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="precipitation",driver="JRPC",
                  type="TBS",address=33,timerange="1,0,0",level="1,-,-,-")
        
    if (template == "stima_indirect_ths"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature",driver="JRPC",
                  type="ADT",address=73,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Humidity",driver="JRPC",
                  type="HIH",address=39,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Dust",driver="JRPC",
                  type="SSD",address=36,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_indirect_thsm"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature",driver="JRPC",
                  type="ADT",address=73,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Humidity",driver="JRPC",
                  type="HIH",address=39,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Dust",driver="JRPC",
                  type="SSD",address=36,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Nitrogen dioxide",driver="JRPC",
                  type="SMI",address=36,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_indirect_thwr"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature",driver="JRPC",
                  type="ADT",address=73,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Humidity",driver="JRPC",
                  type="HIH",address=39,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Wind",driver="JRPC",
                  type="DW1",address=34,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Precipitation",driver="JRPC",
                  type="TBS",address=33,timerange="1,0,0",level="1,-,-,-")

    if (template == "stima_indirect_thwrp"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature",driver="JRPC",
                  type="ADT",address=73,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Humidity",driver="JRPC",
                  type="HIH",address=39,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Wind",driver="JRPC",
                  type="DW1",address=34,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Precipitation",driver="JRPC",
                  type="TBS",address=33,timerange="1,0,0",level="1,-,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Pressure",driver="JRPC",
                  type="BMP",address=119,timerange="254,0,0",level="1,-,-,-")


    if (template == "stima_indirect_report_thp"):
        print "setting template:", template
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Temperature/Humidity report inst. values",driver="JRPC",
                  type="ITH",address=35,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Temperature/Humidity report min values",driver="JRPC",
                  type="NTH",address=35,timerange="3,0,900",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Temperature/Humidity report mean values",driver="JRPC",
                  type="MTH",address=35,timerange="0,0,900",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Temperature/Humidity report max malues",driver="JRPC",
                  type="XTH",address=35,timerange="2,0,900",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Precipitation report",driver="JRPC",
                  type="TBR",address=33,timerange="1,0,900",level="1,-,-,-")


def configstation(transport_name="serial",station_slug=None,board_slug=None,logfunc=jsonrpc.log_file("rpc.log"),
                  device=None,baudrate=None,host=None,transport=None,username=None):

    if (station_slug is None): return
    if (username is None): return

    mystation=StationMetadata.objects.get(slug=station_slug,ident__username=username)

    if not mystation.active:
        print "disactivated station: do nothing!"
        return


    for board in mystation.board_set.all():

        if board_slug is not None and board.slug != board_slug:
            continue

        if transport_name == "amqp":
            try:
                if ( board.transportamqp.active):
                    print "AMQP Transport", board.transportamqp

                    amqpserver =board.transportamqp.amqpserver
                    amqpuser=board.transportamqp.amqpuser
                    amqppassword=board.transportamqp.amqppassword
                    queue=board.transportamqp.queue
                    exchange=board.transportamqp.exchange

                    sh=rabbitshovel.shovel(srcqueue=queue,destexchange=exchange,destserver=amqpserver)
                    sh.delete()
                    sh.create(destuser=amqpuser,destpassword=amqppassword)

            except ObjectDoesNotExist:
                print "transport AMQP not present for this board"
                return


        if transport is None:

            if transport_name == "serial":
                try:
                    if ( board.transportserial.active):
                        print "Serial Transport", board.transportserial
                        mydevice =board.transportserial.device
                        if device is not None :
                            mydevice=device
                        mybaudrate=board.transportserial.baudrate
                        if baudrate is not None :
                            mybaudrate=baudrate

                        print "mybaudrate:",mybaudrate

                        transport=jsonrpc.TransportSERIAL( logfunc=logfunc,port=mydevice,baudrate=mybaudrate,timeout=5)

                except ObjectDoesNotExist:
                    print "transport serial not present for this board"
                    return


            if transport_name == "tcpip":
                try:
                    if ( board.transporttcpip.active):
                        print "TCP/IP Transport", board.transporttcpip

                        myhost =board.transporttcpip.name
                        if host is not None :
                            myhost=host

                        transport=jsonrpc.TransportTcpIp(logfunc=logfunc,addr=(myhost,1000),timeout=5)

                except ObjectDoesNotExist:
                    print "transport TCPIP not present for this board"
                    return

        rpcproxy = jsonrpc.ServerProxy( jsonrpc.JsonRpc20(),transport)
        if (rpcproxy is None): return

        print ">>>>>>> reset config"
        print "reset",rpcproxy.configure(reset=True )

        print ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> configure board: ", board.name," slug="+board.slug

        try:
            if ( board.transportmqtt.active):
                print "TCP/IP Transport",board.transportmqtt
                print "sampletime and mqttserver:",rpcproxy.configure(mqttsampletime=board.transportmqtt.mqttsampletime,
                                                   mqttserver=board.transportmqtt.mqttserver)
                print "mqtt user and password:",rpcproxy.configure(mqttuser=board.transportmqtt.mqttuser,
                                                   mqttpassword=board.transportmqtt.mqttpassword)
        except ObjectDoesNotExist:
            print "transport mqtt not present"

        try:
            if ( board.transporttcpip.active):
                print "TCP/IP Transport",board.transporttcpip
                mac=board.transporttcpip.mac[board.transporttcpip.name]
                print "ntpserver:",rpcproxy.configure(mac=mac,ntpserver=board.transporttcpip.ntpserver)

        except ObjectDoesNotExist:
            print "transport tcpip not present"

        try:
            if ( board.transportrf24network.active):
                print "RF24Network Transport",board.transportrf24network
                print "thisnode:",rpcproxy.configure(thisnode=board.transportrf24network.node,
                                                 channel=board.transportrf24network.channel)
                if board.transportrf24network.key != "":
                    print "key:",rpcproxy.configure(key=map(int, board.transportrf24network.key.split(',')))
                if board.transportrf24network.iv != "":
                    print "iv:",rpcproxy.configure(iv=map(int, board.transportrf24network.iv.split(',')))

        except ObjectDoesNotExist:
            print "transport rf24network not present"

        print ">>>> sensors:"
        for sensor in board.sensor_set.all():
            if not sensor.active: continue
            print sensor

            print "add driver:",rpcproxy.configure(driver=sensor.driver,
                                type=sensor.type.type,
                                node=sensor.node,address=sensor.address,
                                mqttpath=sensor.timerange+"/"+sensor.level+"/")
            #TODO  check id of status (good only > 0)

        print "mqttrootpath:",rpcproxy.configure(mqttrootpath=mystation.mqttrootpath+"/"+str(mystation.ident)+"/"+\
                                                 "%d,%d" % (nint(mystation.lon*100000),nint(mystation.lat*100000))+\
                                                 "/"+mystation.network+"/")

        print ">>>>>>> save config"
        print "save",rpcproxy.configure(save=True )

        print "----------------------------- board configured ---------------------------------------"
        
        transport.close()


def send2amqp(body="",user=None,password=None,host="rmap.cc",exchange="configuration",routing_key="config"):

    credentials=pika.PlainCredentials(user, password)
    properties=pika.BasicProperties(
        user_id= user,
        delivery_mode = 2, # persistent
    )

    connection = pika.BlockingConnection(pika.ConnectionParameters(
        host=host,credentials=credentials,socket_timeout=5000))
    channel = connection.channel()

    channel.confirm_delivery()
    
    try:
        if channel.basic_publish(exchange=exchange,
                                 routing_key=routing_key,
                                 body=body,
                                 properties=properties):
            print " [x] Message Sent "
            channel.close()
            connection.close()

        else:
            
            print " [x] Error on publish "
            connection.close()
            
    except Exception as e:
        print ("PikaMQ publish really error ", e) 
        connection.close()
        raise

def export2json(objects):

    return serializers.serialize('json', objects, indent=2,
        use_natural_foreign_keys=True, use_natural_primary_keys=True)


def dumpstation(station,user=u"your user"):

    objects=[]

    mystation=StationMetadata.objects.get(slug=station,ident__username=user)
    objects.append(mystation)
    for board in mystation.board_set.all():
        objects.append(board)
        
        for sensor in board.sensor_set.all():
            objects.append(sensor)
        
    return export2json(objects)


def sendjson2amqp(station,user=u"your user",password="your password",host="rmap.cc",exchange="configuration"):

    print "sendjson2amqp"

    body=dumpstation(station,user)
    send2amqp(body,user,password,host,exchange)


def receivegeoimagefromamqp(user=u"your user",password="your password",host="rmap.cc",queue="photo"):

    from geoimage.models import GeorefencedImage

    def callback(ch, method, properties, body):
        print " [x] Received message"

        if properties.user_id is None:
            print "Ignore anonymous message"
            print " [x] Done"
            ch.basic_ack(delivery_tag = method.delivery_tag)
            return
  
        #At this point we can check if we trust this authenticated user... 
        ident=properties.user_id
        print "Received from user: %r" % ident 

        try:
            # store image in DB

            lat,lon,imgident,comment,date=exifutils.getgeoimage(body)


            #img = pexif.JpegFile.fromString(body)

            #exif = img.get_exif()
            #if exif:
            #    primary = exif.get_primary()
            #if exif is None or primary is None:
            #    print "image has no EXIF tag, skipping"
            #    imgident=None
            #else:
            #    imgident=primary.ImageDescription

            #but we check that message content is with the same ident
            #if (imgident == ident):

            #    comment="".join(img.exif.primary.ExtendedEXIF.UserComment[8:])
            #    lat,lon=img.get_geo()

            #    primary = exif.get_primary()
            #    timetag=primary.DateTime
            #    date = datetime.strptime(timetag, '%Y:%m:%d %H:%M:%S')

            print "getted those metadata from exif:"
            print lat,lon
            print comment
            print date
            print imgident

            if (imgident == ident):
                geoimage=GeorefencedImage()
                geoimage.geom = {'type': 'Point', 'coordinates': [lon, lat]}
                geoimage.comment=comment
                geoimage.date=date

                try:
                    geoimage.ident=User.objects.get(username=ident)
                    geoimage.image.save('geoimage.jpg',ContentFile(body))
                    geoimage.save()
                except User.DoesNotExist:
                    print "user does not exist"
            else:
                print "reject:",ident

        except Exception as e:
            print e
            raise

        print " [x] Done"
        ch.basic_ack(delivery_tag = method.delivery_tag)

    credentials=pika.PlainCredentials(user, password)

    connection = pika.BlockingConnection(pika.ConnectionParameters(
        host=host,credentials=credentials))
    channel = connection.channel()
    #channel.queue_declare(queue=queue)

    print ' [*] Waiting for messages. To exit press CTRL+C'


    channel.basic_consume(callback,
                          queue=queue,
                          no_ack=False)

    channel.start_consuming()

    connection.close()
    sendconnection.close()



def receivejsonfromamqp(user=u"your user",password="your password",host="rmap.cc",queue="configuration"):

    def callback(ch, method, properties, body):
        print " [x] Received message"

        if properties.user_id is None:
            print "Ignore anonymous message"
            print " [I] Ignore"
            ch.basic_ack(delivery_tag = method.delivery_tag)
            print " [x] Done"
            return
  
        #At this point we can check if we trust this authenticated user... 
        ident=properties.user_id
        print "Received from user: %r" % ident 
        
        #but we check that message content is with the same ident
        try:
            for deserialized_object in serializers.deserialize("json",body):
                if object_auth(deserialized_object.object,ident):
                    try:
                        print "save:",deserialized_object.object
                        deserialized_object.save()
                    except Exception as e:
                        print (" [E] Error saving in DB",e)
                        #close django connection to DB
                        try:
                            connection.close()
                        except:
                            pass
                        # we have to put message in error queue to recover it later
                        # this is more conservative but we can stall
                        #ch.basic_nack(delivery_tag = method.delivery_tag)
                        ch.basic_ack(delivery_tag = method.delivery_tag)
                        return

                else:
                    print "reject:",deserialized_object.object
                    ch.basic_ack(delivery_tag = method.delivery_tag)
                    print " [R] Rejected"

        except Exception as e:
            print ("error in deserialize object; skip it",e)

        ch.basic_ack(delivery_tag = method.delivery_tag)
        print " [x] Done"

        #close django connection to DB
        try:
            connection.close()
        except Exception as e:
            print ("django connection close error",e)


    credentials=pika.PlainCredentials(user, password)

    amqpconnection = pika.BlockingConnection(pika.ConnectionParameters(
        host=host,credentials=credentials))
    channel = amqpconnection.channel()
    #channel.queue_declare(queue=queue)

    print ' [*] Waiting for messages. To exit press CTRL+C'


    channel.basic_consume(callback,
                          queue=queue,
                          no_ack=False)

    channel.start_consuming()

    amqpconnection.close()


def object_auth(object,user):
    #print object
    #print type(object)

    if isinstance(object,StationMetadata):
        if object.ident.username == user:
            return True

    if isinstance(object,Board):
        if object.stationmetadata.ident.username == user:
            return True

    if isinstance(object,Sensor):
        if object.board.stationmetadata.ident.username == user:
            return True

    return False



def updateusername(oldusername="rmap",newusername="rmap",newpassword=None):
    "returns the number of affected rows"

    row=0
    if (oldusername != newusername):
        row=User.objects.filter(username=oldusername).update(username=newusername)
    if (not newpassword is None):
        u = User.objects.get(username__exact=newusername)
        u.set_password(newpassword)
        u.save()
    return row

def activatestation(username="rmap",station="home",board=None,activate=None,activateboard=None):

    print "elaborate station: ",station

    mystation=StationMetadata.objects.get(slug=station,ident__username=username)

    if not (activate is None):
        mystation.active=activate
        mystation.clean()
        mystation.save()

    if not (activateboard is None) and not (board is None):
        for myboard in mystation.board_set.all():
            print "elaborate board: ",myboard

            if not (myboard.slug == board): continue
            if not (activateboard is None): 
                myboard.active=activateboard
                myboard.save()


def configdb(username="rmap",password="rmap",
             station="home",lat=0,lon=0,constantdata={},network="fixed",
             mqttusername="your user",
             mqttpassword="your password",
             mqttserver="rmap.cc",
             mqttsamplerate=5,
             bluetoothname="hc06",
             amqpusername="your user",
             amqppassword="your password",
             amqpserver="rmap.cc",
             queue="rmap",
             exchange="rmap",
             board=None,
             activate=None,
             stationname=None,
             mqttrootpath=None,
             mqttmaintpath=None):

    try:
        user = User.objects.create_user(username, username+'@rmap.cc', password)            
        #trap IntegrityError for user that already exist
    except IntegrityError:
        pass
    except:
        raise

    updateusername(oldusername=username,newusername=username,newpassword=password)
        
    try:

        print "elaborate station: ",station

        try:
            mystation=StationMetadata.objects.get(slug=station,ident__username=username)
        except ObjectDoesNotExist:
            if (stationname is None):
                stationname=""
            mystation=StationMetadata(slug=station,name=stationname)

        user=User.objects.get(username=username)
            
        mystation.ident=user
        mystation.lat=lat
        mystation.lon=lon
        mystation.network=network
        
        if not (mqttrootpath is None):
            mystation.mqttrootpath=mqttrootpath

        if not (mqttmaintpath is None):
            mystation.mqttmaintpath=mqttmaintpath

        if not (activate is None): mystation.active=activate
        mystation.clean()
        mystation.save()
            
    except:
        raise # "Error\nsetting station"

    # remove all StationConstantData
    try:
        StationConstantData.objects.filter(stationmetadata=mystation).delete()
    except:
        pass

    for btable,value in constantdata.iteritems():

        # remove only StationConstantData in constantdata
        #try:
        #    StationConstantData.objects.filter(stationmetadata=mystation,btable=btable).delete()
        #except:
        #    pass

        try:
            mystation.stationconstantdata_set.create(
                active=True,
                btable=btable,
                value=value
            )

        except:
            pass

    for myboard in mystation.board_set.all():
        print "elaborate board: ",myboard

        if board is None:
            if not myboard.active: continue
        else:
            if not myboard.slug == board: continue
            if not (activate is None): myboard.active=activate
            myboard.save()

        try:
            if ( myboard.transportmqtt.active):
                print "MQTT Transport", myboard.transportmqtt
                
                myboard.transportmqtt.mqttserver=mqttserver
                myboard.transportmqtt.mqttuser=mqttusername
                myboard.transportmqtt.mqttpassword=mqttpassword
                myboard.transportmqtt.mqttsampletime=mqttsamplerate
                myboard.transportmqtt.save()
                
        except ObjectDoesNotExist:
            print "transport MQTT not present for this board"


        try:
            if ( myboard.transportbluetooth.active):
                print "bluetooth Transport", myboard.transportbluetooth

                myboard.transportbluetooth.name=bluetoothname
                myboard.transportbluetooth.save()

        except ObjectDoesNotExist:
            print "transport Bluetooth not present for this board"

        try:
            if ( myboard.transportamqp.active):
                print "AMQP Transport", myboard.transportamqp

                myboard.transportamqp.amqpuser=amqpusername
                myboard.transportamqp.amqppassword=amqppassword
                
                myboard.transportamqp.amqpserver=amqpserver
                myboard.transportamqp.queue=queue
                myboard.transportamqp.exchange=exchange
                
                myboard.transportamqp.save()

        except ObjectDoesNotExist:
            print "transport AMQP not present for this board"


        # TODO Serial TCPIP

