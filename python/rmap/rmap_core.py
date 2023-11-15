#!/usr/bin/python
# GPL. (C) 2020 Paolo Patruno.

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
from .stations.models import StationMetadata
from .stations.models import StationConstantData
from .stations.models import Board
from .stations.models import Sensor, SensorType
from .stations.models import TransportRF24Network
from .stations.models import TransportCan
from .stations.models import TransportMqtt
from .stations.models import TransportBluetooth
from .stations.models import TransportAmqp
from .stations.models import TransportSerial
from .stations.models import TransportTcpip
from .configuration_stimav3 import *
from django.contrib.auth.models import User
from django.core import serializers
from django.utils.translation import ugettext as _
import pika
from rmap.utils import nint
from rmap import jsonrpc
from django.core.files.base import ContentFile
from datetime import datetime
from . import exifutils
from django.db import connection
import collections
import threading
import dballe,io
import functools
import logging
import time
import requests

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

#https://stackoverflow.com/questions/783897/truncating-floats-in-python
def truncate(f, n):
    '''Truncates/pads a float f to n decimal places without rounding'''
    s = '{}'.format(f)
    if 'e' in s or 'E' in s:
        return '{0:.{1}f}'.format(f, n)
    i, p, d = s.partition('.')
    return '.'.join([i, (d+'0'*n)[:n]])


def delsensor(station_slug=None,username=None,board_slug=None,name=None):

    Sensor.objects.get(name=name,board__slug=board_slug
                                ,board__stationmetadata__slug=station_slug
                                ,board__stationmetadata__user__username=username).delete()

def delsensors(station_slug=None,username=None,board_slug=None):

    Sensor.objects.filter(board__slug=board_slug
                                ,board__stationmetadata__slug=station_slug
                                ,board__stationmetadata__user__username=username).delete()


def addboard(station_slug=None,username=None,board_slug=None,type=0,sn=None,activate=False
              ,serialactivate=False
              ,canactivate=False,cannodeid=100,cansubject="",cansubjectid=100,cansamplerate=60
              ,mqttactivate=False, mqttserver="rmap.cc", mqttusername=None, mqttpassword=None, mqttpskkey=None, mqttsamplerate=5
              ,bluetoothactivate=False, bluetoothname="HC-05"
              ,amqpactivate=False, amqpusername="rmap", amqppassword=None, amqpserver="rmap.cc", queue="..bufr.report_fixed", exchange="..bufr.report_fixed"
              ,tcpipactivate=False, tcpipname="master", tcpipntpserver="pool.ntp.org", tcpipgsmapn="ibox.tim.it",tcpippppnumber="*99#"
          ):

    print("---------------------------")
    print(station_slug,username,board_slug)
    print("---------------------------")

    try:
        myboard = Board.objects.get(slug=board_slug
                                    ,stationmetadata__slug=station_slug
                                    ,stationmetadata__user__username=username)
        myboard.type=type
        myboard.sn=sn
        myboard.active=activate
        myboard.save()

    except ObjectDoesNotExist :
        mystation=StationMetadata.objects.get(slug=station_slug,user__username=username)
        myboard=Board(name=board_slug,slug=board_slug,stationmetadata=mystation,type=type,sn=sn,active=activate)
        myboard.save()

    myboard = Board.objects.get(slug=board_slug
                                ,stationmetadata__slug=station_slug
                                ,stationmetadata__user__username=username)


    try:
        transportserial=myboard.transportserial
    except ObjectDoesNotExist :
        transportserial=TransportSerial()

    transportserial.active=serialactivate
    myboard.transportserial=transportserial
    print("Serial Transport", myboard.transportserial)
    myboard.transportserial.save()


    try:
        transportcan=myboard.transportcan
    except ObjectDoesNotExist :
        transportcan=TransportCan()

    
    transportcan.active=canactivate
    transportcan.node_id=cannodeid
    transportcan.subject=cansubject
    transportcan.subject_id=cansubjectid
    transportcan.cansampletime=cansamplerate
    myboard.transportcan=transportcan
    print("CAN Transport", myboard.transportcan)
    myboard.transportcan.save()


    try:
        transportmqtt=myboard.transportmqtt
    except ObjectDoesNotExist :
        transportmqtt=TransportMqtt()

    transportmqtt.active=mqttactivate
    transportmqtt.mqttserver=mqttserver
    transportmqtt.mqttuser=mqttusername
    transportmqtt.mqttpassword=mqttpassword
    transportmqtt.mqttpskkey=mqttpskkey
    transportmqtt.mqttsampletime=mqttsamplerate
    myboard.transportmqtt=transportmqtt
    print("MQTT Transport", myboard.transportmqtt)
    myboard.transportmqtt.save()
                
    try:
        transportbluetooth=myboard.transportbluetooth
    except ObjectDoesNotExist :
        transportbluetooth=TransportBluetooth()
    transportbluetooth.active=bluetoothactivate
    transportbluetooth.name=bluetoothname
    myboard.transportbluetooth=transportbluetooth
    print("bluetooth Transport", myboard.transportbluetooth)
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
    print("AMQP Transport", myboard.transportamqp)                
    myboard.transportamqp.save()

    try:
        transporttcpip=myboard.transporttcpip
    except ObjectDoesNotExist :
        transporttcpip=TransportTcpip()
    transporttcpip.active=tcpipactivate
    transporttcpip.name=tcpipname
    transporttcpip.ntpserver=tcpipntpserver
    transporttcpip.gsmapn=tcpipgsmapn
    transporttcpip.pppnumber=tcpippppnumber    
    myboard.transporttcpip=transporttcpip
    print("TCPIP Transport", myboard.transporttcpip)                
    myboard.transporttcpip.save()
    

def addsensor(station_slug=None,username=None,board_slug=None,name="my sensor",driver="TMP",type="TMP",i2cbus=None,address=None,node=None
              ,timerange="254,0,0",level="0,1",activate=False):
    #,sensortemplate=None):

    print("---------------------------")
    print(station_slug,username,board_slug)
    print("---------------------------")

    try:
        myboard = Board.objects.get(slug=board_slug
                                    ,stationmetadata__slug=station_slug
                                    ,stationmetadata__user__username=username)
    except ObjectDoesNotExist :
            print("board not present for this station")
            raise

    try:
        mytype = SensorType.objects.get(type=type)
    except ObjectDoesNotExist :
        print("sensor type: ",type,"  not present in DB")
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
        print("try to save:",mysensor)
        try:
            mysensor.clean()
            mysensor.save()
        except IntegrityError:
            oldsensor=Sensor.objects.get(board=myboard,name=name)
            oldsensor.delete()
            mysensor.save()

dtable={"50":["B49198","B49199","B49200","B49201","B49202","B49203","B49204",
              "B49205","B49206","B49207","B49208","B49209","B49210","B49211",
              "B49212","B49213","B49214","B49215","B49216","B49217","B49218",
              "B49219","B49220","B49221"],
        "51":["B11211","B11212","B11213","B11214","B11215","B11216"],
        "52":["B49198","B49199","B49200","B49201","B49202","B49203","B49204",
              "B49205","B49206","B49207","B49208","B49209","B49210"]}

ttntemplate=[]
ttntemplate.append(collections.OrderedDict())  # null template 0
ttntemplate.append(collections.OrderedDict())  # template 1: temperature and humidity
ttntemplate[1]["B12101"]={"nbit":16,"offset":22315,"scale":100,"timerange":"254,0,0","level":"103,2000,-,-"}
ttntemplate[1]["B13003"]={"nbit":7,"offset":0,"scale":1,"timerange":"254,0,0","level":"103,2000,-,-"}

ttntemplate.append(collections.OrderedDict())  # template 2: temperature and humidity
ttntemplate[2]["B12101"]={"nbit":16,"offset":22315,"scale":100,"timerange":"254,0,0","level":"103,2000,-,-"}
ttntemplate[2]["B13003"]={"nbit":7,"offset":0,"scale":1,"timerange":"254,0,0","level":"103,2000,-,-"}
ttntemplate[2]["B15198"]={"nbit":20,"offset":0,"scale":10000000000,"timerange":"254,0,0","level":"103,2000,-,-"}

            
# the first is the default
# attention: here if in the template there is "_report_" this is used by interdata app to change the stationmetadata mqttrootpath that default to "sample"
template_choices = [
    "default",
    "none",
    "test",    "test_indirect",    "test_rf24",    "test_master",    "test_base",
    "stima4_report_th","stima4_report_p","stima4_report_w","stima4_report_r","stima4_report_m","stima4_report_s",
    "stima_base",    "stima_t",    "stima_h",    "stima_w",    "stima_r",    "stima_p",    "stima_s",    "stima_m",
    "stima_sm",    "stima_th",    "stima_y",    "stima_ths",    "stima_thsm",    "stima_thw",    "stima_thp",    "stima_yp",
    "stima_thwr",    "stima_thwrp",
    "stima_rf24_t",    "stima_rf24_h",    "stima_rf24_w",    "stima_rf24_r",    "stima_rf24_p",    "stima_rf24_th",    "stima_rf24_y",
    "stima_rf24_thw",    "stima_rf24_thp",    "stima_rf24_yp",    "stima_rf24_thwr",    "stima_rf24_thwrp",
    "airquality_sds", "airquality_pms", "airquality_hpm", "airquality_sps", "airquality_sps_sht", "airquality_sps_sht_scd",
    "stima_thd", "stima_thdm",
    "stima_report_thp","stima_report_thpb", "stima_report_thpbl", "stima_report_thpbwr", "stima_report_thpwb", "stima_report_p",
    "stima_indirect_t",    "stima_indirect_h",    "stima_indirect_r",    "stima_indirect_p",    "stima_indirect_s", "stima_indirect_m",
    "stima_indirect_sm", "stima_indirect_th",    "stima_indirect_y",    "stima_indirect_thw",    "stima_indirect_thp",    "stima_indirect_yp",    "stima_indirect_ths",    "stima_indirect_thsm",
    "stima_indirect_thwr",    "stima_indirect_thwrp",    "stima_indirect_report_thp",
]

def addsensors_by_template(station_slug=None,username=None,board_slug=None,template=None):

    if (template == "default"):
        print("setting template:", template," do not change sensors on db")
        pass

    if (template == "none"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)

    if (template == "test"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="stima test",driver="I2C",
                  type="TMP",address=72,timerange="254,0,0",level="0,1,-,-")

    if (template == "test_indirect"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="stima test jrpc",driver="JRPC",
                  type="TMP",address=72,timerange="254,0,0",level="0,1,-,-")

    if (template == "test_rf24"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="stima test rf24",driver="RF24",
                  type="TMP",address=72,timerange="254,0,0",level="0,2,-,-")

    if (template == "test_master"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="stima test",driver="I2C",
                  type="TMP",address=72,timerange="254,0,0",level="0,1,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="stima test rf24",driver="RF24",
                  type="TMP",address=72,timerange="254,0,0",level="0,2,-,-")

    if (template == "test_base"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="test Temperature",driver="I2C",
                  type="TMP",address=72,timerange="254,0,0",level="0,1,-,-")

    if (template == "stima4_report_th"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Temperature/Humidity report inst. values",driver="CAN",
                  type="ITH",timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Temperature/Humidity report min values",driver="CAN",
                  type="NTH",timerange="3,0,{P2:d}",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Temperature/Humidity report mean values",driver="CAN",
                  type="MTH",timerange="0,0,{P2:d}",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Temperature/Humidity report max values",driver="CAN",
                  type="XTH",timerange="2,0,{P2:d}",level="103,2000,-,-")

    if (template == "stima4_report_p"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Precipitation report",driver="CAN",
                  type="TBR",timerange="1,0,{P2:d}",level="1,-,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Precipitation rate report",driver="CAN",
                  type="TPR",timerange="2,0,{P2:d}",level="1,-,-,-")

    if (template == "stima4_report_w"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Windsonic wind sensor vectorial average 10'",driver="CAN",
                  type="DWA",timerange="254,0,0",level="103,10000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Windsonic wind vect. average, gust direction",driver="CAN",
                  type="DWB",timerange="200,0,{P2:d}",level="103,10000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Windsonic wind sensor gust speeds",driver="CAN",
                  type="DWC",timerange="2,0,{P2:d}",level="103,10000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Windsonic wind sensor scalar average",driver="CAN",
                  type="DWD",timerange="0,0,{P2:d}",level="103,10000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Windsonic wind sensor frequency",driver="CAN",
                  type="DWE",timerange="9,0,{P2:d}",level="103,10000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Windsonic wind sensor gust directions",driver="CAN",
                  type="DWF",timerange="205,0,{P2:d}",level="103,10000,-,-")

    if (template == "stima4_report_r"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Radiation",driver="CAN",
                  type="DSA",timerange="0,0,{P2:d}",level="1,-,-,-")

    if (template == "stima4_report_m"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Power MPPT",driver="CAN",
                  type="MPP",timerange="0,0,{P2:d}",level="265,1,-,-")

    if (template == "stima4_report_s"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Soil water content 25cm",driver="CAN",
                  type="SVW",timerange="0,0,{P2:d}",level="106,250,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Soil water content 45cm",driver="CAN",
                  type="SVW",timerange="0,0,{P2:d}",level="106,450,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Soil water content 70cm",driver="CAN",
                  type="SVW",timerange="0,0,{P2:d}",level="106,700,-,-")
        
    if (template == "stima_base"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature",driver="I2C",
                  type="ADT",address=73,timerange="254,0,0",level="103,1000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Humidity",driver="I2C",
                  type="HIH",address=39,timerange="254,0,0",level="103,1000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Pressure",driver="I2C",
                  type="BMP",address=119,timerange="254,0,0",level="1,-,-,-")

    if (template == "stima_t"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature",driver="I2C",
                  type="ADT",address=73,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_h"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Humidity",driver="I2C",
                  type="HIH",address=39,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_w"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Wind",driver="I2C",
                  type="DW1",address=34,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_r"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Precipitation",driver="I2C",
                  type="TBS",address=33,timerange="1,0,0",level="1,-,-,-")

    if (template == "stima_p"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Pressure",driver="I2C",
                  type="BMP",address=119,timerange="254,0,0",level="1,-,-,-")

    if (template == "stima_s"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Dust",driver="I2C",
                  type="SSD",address=36,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_m"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Nitrogen dioxide",driver="I2C",
                  type="SMI",address=36,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_sm"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Dust",driver="I2C",
                  type="SSD",address=36,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Nitrogen dioxide",driver="I2C",
                  type="SMI",address=36,timerange="254,0,0",level="103,2000,-,-")
        
    if (template == "stima_th"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature",driver="I2C",
                  type="ADT",address=73,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Humidity",driver="I2C",
                  type="HIH",address=39,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_y"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature, Humidity",driver="I2C",
                  type="HYT",address=40,timerange="254,0,0",level="103,2000,-,-")
        
    if (template == "stima_ths"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature",driver="I2C",
                  type="ADT",address=73,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Humidity",driver="I2C",
                  type="HIH",address=39,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Dust",driver="I2C",
                  type="SSD",address=36,timerange="254,0,0",level="103,2000,-,-")


    if (template == "stima_thsm"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature",driver="I2C",
                  type="ADT",address=73,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Humidity",driver="I2C",
                  type="HIH",address=39,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Dust",driver="I2C",
                  type="SSD",address=36,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Nitrogen dioxide",driver="I2C",
                  type="SMI",address=36,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_thd"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature",driver="I2C",
                  type="ADT",address=73,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Humidity",driver="I2C",
                  type="HIH",address=39,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Dust",driver="SERI",
                  type="HPM",address=36,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_thdm"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature",driver="I2C",
                  type="ADT",address=73,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Humidity",driver="I2C",
                  type="HIH",address=39,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Dust",driver="SERI",
                  type="HPM",address=36,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Nitrogen dioxide",driver="I2C",
                  type="SMI",address=36,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_thw"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature",driver="I2C",
                  type="ADT",address=73,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Humidity",driver="I2C",
                  type="HIH",address=39,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Wind",driver="I2C",
                  type="DW1",address=34,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_thp"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature",driver="I2C",
                  type="ADT",address=73,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Humidity",driver="I2C",
                  type="HIH",address=39,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Precipitation",driver="I2C",
                  type="TBS",address=33,timerange="1,0,0",level="1,-,-,-")


    if (template == "stima_yp"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature, Humidity",driver="I2C",
                  type="HYT",address=40,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="precipitation",driver="I2C",
                  type="TBS",address=33,timerange="1,0,0",level="1,-,-,-")

        
    if (template == "stima_thwr"):
        print("setting template:", template)
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
        print("setting template:", template)
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
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="rf24 Temperature",driver="RF24",
                  type="ADT",address=73,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_rf24_h"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="rf24 Humidity",driver="RF24",
                  type="HIH",address=39,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_rf24_w"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="rf24 Wind",driver="RF24",
                  type="DW1",address=34,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_rf24_r"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="rf24 Precipitation",driver="RF24",
                  type="TBS",address=33,timerange="1,0,0",level="1,-,-,-")

    if (template == "stima_rf24_p"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="rf24 Pressure",driver="RF24",
                  type="BMP",address=119,timerange="254,0,0",level="1,-,-,-")

    if (template == "stima_rf24_th"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="rf24 Temperature",driver="RF24",
                  type="ADT",address=73,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="rf24 Humidity",driver="RF24",
                  type="HIH",address=39,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_rf24_y"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="rf24 Temperature, Humidity",driver="RF24",
                  type="HYT",address=40,timerange="254,0,0",level="103,2000,-,-")
        
    if (template == "stima_rf24_thw"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="rf24 Temperature",driver="RF24",
                  type="ADT",address=73,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="rf24 Humidity",driver="RF24",
                  type="HIH",address=39,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="rf24 Humidity",driver="RF24",
                  type="DW1",address=34,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_rf24_thp"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="rf24 Temperature",driver="RF24",
                  type="ADT",address=73,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="rf24 Humidity",driver="RF24",
                  type="HIH",address=39,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="rf24 Humidity",driver="RF24",
                  type="TBS",address=33,timerange="1,0,0",level="1,-,-,-")

    if (template == "stima_rf24_yp"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature, Humidity",driver="RF24",
                  type="HYT",address=40,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="precipitation",driver="RF24",
                  type="TBS",address=33,timerange="1,0,0",level="1,-,-,-")

        
    if (template == "stima_rf24_thwr"):
        print("setting template:", template)
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
        print("setting template:", template)
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

    if (template == "airquality_sds"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Dust",driver="SERI",
                  type="SSD",address=36,timerange="254,0,0",level="103,2000,-,-")

    if (template == "airquality_pms"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Dust",driver="SERI",
                  type="PMS",address=36,timerange="254,0,0",level="103,2000,-,-")

    if (template == "airquality_hpm"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Dust",driver="SERI",
                  type="hpm",address=36,timerange="254,0,0",level="103,2000,-,-")

    if (template == "airquality_sps"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Dust",driver="I2C",
                  type="SPS",address=105,timerange="254,0,0",level="103,2000,-,-")

    if (template == "airquality_sps_sht"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Dust",driver="I2C",
                  type="SPS",address=105,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature_Humidity",driver="I2C",
                  type="SHT",address=68,timerange="254,0,0",level="103,2000,-,-")

    if (template == "airquality_sps_sht_scd"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Dust",driver="I2C",
                  type="SPS",address=105,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature_Humidity",driver="I2C",
                  type="SHT",address=68,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="CO2",driver="I2C",
                  type="SCD",address=97,timerange="254,0,0",level="103,2000,-,-")
        
    if (template == "airquality_sps_sht"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Dust",driver="I2C",
                  type="SPS",address=105,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature_Humidity",driver="I2C",
                  type="SHT",address=68,timerange="254,0,0",level="103,2000,-,-")

        
    if (template == "stima_report_p"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Precipitation report",driver="I2C",
                  type="TBR",address=33,timerange="1,0,{P2:d}",level="1,-,-,-")

    if (template == "stima_report_thp"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Temperature/Humidity report inst. values",driver="I2C",
                  type="ITH",address=35,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Temperature/Humidity report min values",driver="I2C",
                  type="NTH",address=35,timerange="3,0,{P2:d}",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Temperature/Humidity report mean values",driver="I2C",
                  type="MTH",address=35,timerange="0,0,{P2:d}",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Temperature/Humidity report max values",driver="I2C",
                  type="XTH",address=35,timerange="2,0,{P2:d}",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Precipitation report",driver="I2C",
                  type="TBR",address=33,timerange="1,0,{P2:d}",level="1,-,-,-")


    if (template == "stima_report_thp"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Temperature/Humidity report inst. values",driver="I2C",
                  type="ITH",address=35,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Temperature/Humidity report min values",driver="I2C",
                  type="NTH",address=35,timerange="3,0,{P2:d}",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Temperature/Humidity report mean values",driver="I2C",
                  type="MTH",address=35,timerange="0,0,{P2:d}",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Temperature/Humidity report max values",driver="I2C",
                  type="XTH",address=35,timerange="2,0,{P2:d}",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Precipitation report",driver="I2C",
                  type="TBR",address=33,timerange="1,0,{P2:d}",level="1,-,-,-")
        
    if (template == "stima_report_thpbl"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Temperature/Humidity report inst. values",driver="I2C",
                  type="ITH",address=35,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Temperature/Humidity report min values",driver="I2C",
                  type="NTH",address=35,timerange="3,0,{P2:d}",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Temperature/Humidity report mean values",driver="I2C",
                  type="MTH",address=35,timerange="0,0,{P2:d}",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Temperature/Humidity report max values",driver="I2C",
                  type="XTH",address=35,timerange="2,0,{P2:d}",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Precipitation report",driver="I2C",
                  type="TBR",address=33,timerange="1,0,{P2:d}",level="1,-,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Battery charge monitor",driver="I2C",
                  type="DEP",address=48,timerange="254,0,0",level="265,1,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Leaf wetness duration",driver="I2C",
                  type="LWT",address=101,timerange="1,0,{P2:d}",level="103,2000,-,-")
        

    if (template == "stima_report_thpbwr"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Temperature/Humidity report inst. values",driver="I2C",
                  type="ITH",address=35,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Temperature/Humidity report min values",driver="I2C",
                  type="NTH",address=35,timerange="3,0,{P2:d}",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Temperature/Humidity report mean values",driver="I2C",
                  type="MTH",address=35,timerange="0,0,{P2:d}",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Temperature/Humidity report max values",driver="I2C",
                  type="XTH",address=35,timerange="2,0,{P2:d}",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Precipitation report",driver="I2C",
                  type="TBR",address=33,timerange="1,0,{P2:d}",level="1,-,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Battery charge monitor",driver="I2C",
                  type="DEP",address=48,timerange="254,0,0",level="265,1,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Windsonic wind sensor vectorial average 10'",driver="I2C",
                  type="DWA",address=69,timerange="254,0,0",level="103,10000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Windsonic wind vect. average, gust direction",driver="I2C",
                  type="DWB",address=69,timerange="200,0,{P2:d}",level="103,10000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Windsonic wind sensor gust speeds",driver="I2C",
                  type="DWC",address=69,timerange="2,0,{P2:d}",level="103,10000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Windsonic wind sensor scalar average",driver="I2C",
                  type="DWD",address=69,timerange="0,0,{P2:d}",level="103,10000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Windsonic wind sensor frequency",driver="I2C",
                  type="DWE",address=69,timerange="9,0,{P2:d}",level="103,10000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Windsonic wind sensor gust directions",driver="I2C",
                  type="DWF",address=69,timerange="205,0,{P2:d}",level="103,10000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Radiation",driver="I2C",
                  type="DSA",address=71,timerange="0,0,{P2:d}",level="1,-,-,-")

        
    if (template == "stima_report_thpwb"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Temperature/Humidity report inst. values",driver="I2C",
                  type="ITH",address=35,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Temperature/Humidity report min values",driver="I2C",
                  type="NTH",address=35,timerange="3,0,{P2:d}",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Temperature/Humidity report mean values",driver="I2C",
                  type="MTH",address=35,timerange="0,0,{P2:d}",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Temperature/Humidity report max values",driver="I2C",
                  type="XTH",address=35,timerange="2,0,{P2:d}",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Precipitation report",driver="I2C",
                  type="TBR",address=33,timerange="1,0,{P2:d}",level="1,-,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Wind",driver="I2C",
                  type="DW1",address=34,timerange="254,0,0",level="103,10000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Battery charge monitor",driver="I2C",
                  type="DEP",address=48,timerange="254,0,0",level="265,1,-,-")
        

    if (template == "stima_indirect_t"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature",driver="JRPC",
                  type="ADT",address=73,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_indirect_h"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Humidity",driver="JRPC",
                  type="HIH",address=39,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_indirect_w"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Wind",driver="JRPC",
                  type="DW1",address=34,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_indirect_r"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Precipitation",driver="JRPC",
                  type="TBs",address=33,timerange="1,0,0",level="1,-,-,-")

    if (template == "stima_indirect_p"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Pressure",driver="JRPC",
                  type="BMP",address=119,timerange="254,0,0",level="1,-,-,-")

    if (template == "stima_indirect_s"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Dust",driver="JRPC",
                  type="SSD",address=36,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_indirect_m"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Nitrogen dioxide",driver="JRPC",
                  type="SMI",address=36,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_indirect_sm"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Dust",driver="JRPC",
                  type="SSD",address=36,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Nitrogen dioxide",driver="JRPC",
                  type="SMI",address=36,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_indirect_th"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature",driver="JRPC",
                  type="ADT",address=73,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Humidity",driver="JRPC",
                  type="HIH",address=39,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_indirect_y"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature, Humidity",driver="JRPC",
                  type="HYT",address=40,timerange="254,0,0",level="103,2000,-,-")
        
    if (template == "stima_indirect_thw"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature",driver="JRPC",
                  type="ADT",address=73,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Humidity",driver="JRPC",
                  type="HIH",address=39,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Humidity",driver="JRPC",
                  type="DW1",address=34,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_indirect_thp"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature",driver="JRPC",
                  type="ADT",address=73,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Humidity",driver="JRPC",
                  type="HIH",address=39,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Humidity",driver="JRPC",
                  type="TBS",address=33,timerange="1,0,0",level="1,-,-,-")

    if (template == "stima_indirect_yp"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature, Humidity",driver="JRPC",
                  type="HYT",address=40,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="precipitation",driver="JRPC",
                  type="TBS",address=33,timerange="1,0,0",level="1,-,-,-")
        
    if (template == "stima_indirect_ths"):
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Temperature",driver="JRPC",
                  type="ADT",address=73,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Humidity",driver="JRPC",
                  type="HIH",address=39,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,name="Dust",driver="JRPC",
                  type="SSD",address=36,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_indirect_thsm"):
        print("setting template:", template)
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
        print("setting template:", template)
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
        print("setting template:", template)
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
        print("setting template:", template)
        delsensors(station_slug=station_slug,username=username,board_slug=board_slug)
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Temperature/Humidity report inst. values",driver="JRPC",
                  type="ITH",address=35,timerange="254,0,0",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Temperature/Humidity report min values",driver="JRPC",
                  type="NTH",address=35,timerange="3,0,{P2:d}",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Temperature/Humidity report mean values",driver="JRPC",
                  type="MTH",address=35,timerange="0,0,{P2:d}",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Temperature/Humidity report max values",driver="JRPC",
                  type="XTH",address=35,timerange="2,0,{P2:d}",level="103,2000,-,-")
        addsensor(station_slug=station_slug,username=username,board_slug=board_slug,
                  name="Precipitation report",driver="JRPC",
                  type="TBR",address=33,timerange="1,0,{P2:d}",level="1,-,-,-")



def modifystation(station_slug=None,username=None,lon=None,lat=None):

    if (station_slug is None): return
    if (username is None): return

    mystation=StationMetadata.objects.get(slug=station_slug,user__username=username)

    if not mystation.active:
        print("disactivated station: do nothing!")
        return

    #print ("salvo", lat,lon)
    if (lat is not None):
        mystation.lat=lat
        mystation.save()
    if (lon is not None):
        mystation.lon=lon
        mystation.save()


# example rpcsMQTT(station_slug="test", username="myusername",recovery={"dts":[2022,2,16,12,0,0]})
def rpcMQTT(station_slug=None,board_slug=None,logfunc=jsonrpc.log_file("rpc.log"),
                   username=None,version=3,**kwargs):

    if (station_slug is None): return
    if (username is None): return

    mystation=StationMetadata.objects.get(slug=station_slug,user__username=username)

    if not mystation.active:
        print("disactivated station: do nothing!")
        return

    for board in mystation.board_set.all():

        if board_slug is not None and board.slug != board_slug:
            continue

        try:
            if ( board.transportmqtt.active):
                print("mqtt Transport", board.transporttcpip)

                #####################################
                # TODO define lat,lon path for mobile stations !
                #####################################
                
                ident = mystation.ident
                myhost = board.transportmqtt.mqttserver
                myuser = board.transportmqtt.mqttuser
                mypassword = board.transportmqtt.mqttpassword
                if (version == "3"):
                    mymqttpskkey=None
                else:
                    mymqttpskkey = board.transportmqtt.mqttpskkey
                
                #print (myuser,ident,mystation.lon,mystation.lat,mystation.network)
                #print(myuser,mystation.slug,board.slug)
                
                myrpctopic="1/rpc/"+myuser+"/"+ident+"/"+\
                    "%d,%d" % (nint(mystation.lon*100000),nint(mystation.lat*100000))+\
                    "/"+mystation.network+"/"
                mqttuser=myuser+"/"+mystation.slug+"/"+board.slug
                
                transport=jsonrpc.TransportMQTT( host=myhost, user=mqttuser,password=mypassword,
                                                 rpctopic=myrpctopic,logfunc=logfunc,timeout=board.transportmqtt.mqttsampletime*1.2,
                                                 pskkey=mymqttpskkey)


                rpcproxy = jsonrpc.ServerProxy( jsonrpc.JsonRpc20(),transport)
                if (rpcproxy is None): return

                for myrpc in kwargs.keys():
                    if (isinstance(kwargs[myrpc],dict)):
                        print(myrpc,getattr(rpcproxy, myrpc)(**kwargs[myrpc] ))
                    else:
                        print(myrpc,getattr(rpcproxy, myrpc)(*kwargs[myrpc] ))

                
        except ObjectDoesNotExist:
            print("transport MQTT not present for board:",board.slug)

                

def find_report_time(station):

    report_time=None

    for board in station.board_set.all():        
        # try to get sampletime: from mqtt transport of the board of the station
        if (hasattr(board, 'transportmqtt')):
            if (board.transportmqtt.active) :
                if (not report_time is None):
                    if(report_time != board.transportmqtt.mqttsampletime): raise Exception("Cannot define sample time: more then one board have different transportmqtt")
                report_time = board.transportmqtt.mqttsampletime

    if  (report_time is None) : raise Exception("Cannot define sample time: no board have transportmqtt")
    return report_time
            
def configstation(transport_name="serial",station_slug=None,board_slug=None,logfunc=jsonrpc.log_file("rpc.log"),
                  device=None,baudrate=None,host=None,transport=None,username=None,version="3",notification=False,
                  without_password=False):

    if (station_slug is None): return
    if (username is None): return

    mystation=StationMetadata.objects.get(slug=station_slug,user__username=username)

    if not mystation.active:
        print("disactivated station: do nothing!")
        return


    for board in mystation.board_set.all().order_by("-type"):

        if board_slug is not None and board.slug != board_slug:
            continue

        if not board.active:
            continue

        if transport_name == "amqp":
            try:
                if ( board.transportamqp.active):
                    print("AMQP Transport", board.transportamqp)

                    amqpserver =board.transportamqp.amqpserver
                    amqpuser=board.transportamqp.amqpuser
                    amqppassword=board.transportamqp.amqppassword
                    queue=board.transportamqp.queue
                    exchange=board.transportamqp.exchange

                    sh=rabbitshovel.shovel(srcqueue=queue,destexchange=exchange,destserver=amqpserver)
                    sh.delete()
                    sh.create(destuser=amqpuser,destpassword=amqppassword)

            except ObjectDoesNotExist:
                print("transport AMQP not present for this board")
                return


        if transport is None:

            if transport_name == "dummy":
                transport=jsonrpc.TransportDUMMY()

            
            if transport_name == "serial":
                try:
                    if ( board.transportserial.active):
                        print("Serial Transport", board.transportserial)
                        mydevice =board.transportserial.device
                        if device is not None :
                            mydevice=device
                        mybaudrate=board.transportserial.baudrate
                        if baudrate is not None :
                            mybaudrate=baudrate

                        print("mybaudrate:",mybaudrate)

                        transport=jsonrpc.TransportSERIAL( logfunc=logfunc,port=mydevice,baudrate=mybaudrate,timeout=1,sleep=3)

                except ObjectDoesNotExist:
                    print("transport serial not present for this board")
                    return


            if transport_name == "tcpip":
                try:
                    if ( board.transporttcpip.active):
                        print("TCP/IP Transport", board.transporttcpip)

                        myhost =board.transporttcpip.name
                        if host is not None :
                            myhost=host

                        transport=jsonrpc.TransportTcpIp(logfunc=logfunc,addr=(myhost,1000),timeout=5)

                except ObjectDoesNotExist:
                    print("transport TCPIP not present for this board")
                    return

            if transport_name == "mqtt":
                try:
                    if ( board.transportmqtt.active):
                        print("mqtt Transport", board.transportmqtt)

                        ident = mystation.ident
                        myhost =board.transportmqtt.mqttserver
                        myuser =board.transportmqtt.mqttuser
                        mypassword =board.transportmqtt.mqttpassword

                        myrpctopic="1/rpc/"+myuser+"/"+ident+"/"+\
                            "%d,%d" % (nint(mystation.lon*100000),nint(mystation.lat*100000))+\
                            "/"+mystation.network+"/"
                        mqttuser=myuser+"/"+mystation.slug+"/"+board.slug

                        transport=jsonrpc.TransportMQTT( host=myhost, user=mqttuser,password=mypassword,rpctopic=myrpctopic,logfunc=logfunc,timeout=board.transportmqtt.mqttsampletime*1.2)

                except ObjectDoesNotExist:
                    print("transport MQTT not present for this board")
                    return

                
        if (transport is None): return
        rpcproxy = jsonrpc.ServerProxy( jsonrpc.JsonRpc20(notification=notification),transport)
        if (rpcproxy is None): return
        
        # with MQTT we send a configure command without params 
        # to say to the station to do not disconnect and wait for RPC
        if transport_name == "mqtt" and version == "3":
            print("MQTT prepare to RPC",rpcproxy.configure())
            time.sleep(30)

        print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> configure board: ", board.name," slug="+board.slug)

        if (version != "3"):
            print("board",rpcproxy.configure(board=board.slug ))
            if (board.sn is None):
                print("type",rpcproxy.configure(boardtype=board.type))
            else:
                print("type",rpcproxy.configure(boardtype=board.type,sn=hex(board.sn)))
            
        print(">>>>>>> reset config")
        print("reset",rpcproxy.configure(reset=True ))
            
        try:
            if ( board.transportmqtt.active):
                print("TCP/IP Transport",board.transportmqtt)

                for constantdata in  mystation.stationconstantdata_set.all():
                    if ( constantdata.active):
                        #print ("constantdata:",constantdata.btable,constantdata.value)
                        print("constantdata:",constantdata.btable,rpcproxy.configure(sd={constantdata.btable:constantdata.value}))

                if (version == "3"):
                    print("mqttrootpath:",rpcproxy.configure(mqttrootpath="1/"+mystation.mqttrootpath+"/"+board.transportmqtt.mqttuser\
                                                             +"/"+ mystation.ident +"/"\
                                                             +"%d,%d" % (nint(mystation.lon*100000),nint(mystation.lat*100000))\
                                                             +"/"+mystation.network+"/"))

                    print("mqttmaintpath:",rpcproxy.configure(mqttmaintpath="1/"+mystation.mqttmaintpath+"/"+board.transportmqtt.mqttuser\
                                                              +"/"+ mystation.ident +"/"\
                                                              +"%d,%d" % (nint(mystation.lon*100000),nint(mystation.lat*100000))\
                                                              +"/"+mystation.network+"/"))

                    print("mqttrpcpath:",rpcproxy.configure(mqttrpcpath="1/rpc/"+board.transportmqtt.mqttuser\
                                                            +"/"+ mystation.ident +"/"\
                                                            +"%d,%d" % (nint(mystation.lon*100000),nint(mystation.lat*100000))\
                                                            +"/"+mystation.network+"/"))

                else:

                    print("mqttrootpath:",rpcproxy.configure(mqttrootpath="1/"+mystation.mqttrootpath))
                    print("mqttmaintpath:",rpcproxy.configure(mqttmaintpath="1/"+mystation.mqttmaintpath))
                    print("mqttrpcpath:",rpcproxy.configure(mqttrpcpath="1/rpc"))
                    print("coordinate:",rpcproxy.configure(lon=nint(mystation.lon*100000),lat=nint(mystation.lat*100000)))
                    print("coordinate:",rpcproxy.configure(network=mystation.network))

                print("sampletime and mqttserver:",rpcproxy.configure(mqttsampletime=board.transportmqtt.mqttsampletime,
                                                   mqttserver=board.transportmqtt.mqttserver))

                if (without_password):
                    try:
                        print("board_slug:",rpcproxy.configure(boardslug=board.slug))
                    except:
                        pass
                else:
                    print("mqtt user and password:",rpcproxy.configure(mqttuser=board.transportmqtt.mqttuser,
                                                    mqttpassword=board.transportmqtt.mqttpassword))
                    try:
                        print("mqtt pskkey:",rpcproxy.configure(mqttpskkey="0x"+board.transportmqtt.mqttpskkey))
                    except:
                        pass           # to be removed; here fo legacy boards
                
                    try:
                        print("station_slug and board_slug:",rpcproxy.configure(stationslug=mystation.slug,boardslug=board.slug))
                    except:
                        pass
                    
                    
        except ObjectDoesNotExist:
            print("transport mqtt not present")

        try:
            if ( board.transporttcpip.active):
                print("TCP/IP Transport",board.transporttcpip)
                mac=board.transporttcpip.mac[board.transporttcpip.name]
                #print("ntpserver:",rpcproxy.configure(mac=mac,ntpserver=board.transporttcpip.ntpserver))
                #print("ntpserver:",rpcproxy.configure(gsmapn="ibox.tim.it",ntpserver=board.transporttcpip.ntpserver))
                print("ntpserver:",rpcproxy.configure(ntpserver=board.transporttcpip.ntpserver))
                print("gsmapn:",rpcproxy.configure(gsmapn=board.transporttcpip.gsmapn))
                if (version != "3"):
                    print("pppnumber:",rpcproxy.configure(pppnumber=board.transporttcpip.pppnumber))

        except ObjectDoesNotExist:
            print("transport tcpip not present")

        try:
            if ( board.transportrf24network.active):
                print("RF24Network Transport",board.transportrf24network)
                print("thisnode:",rpcproxy.configure(thisnode=board.transportrf24network.node,
                                                 channel=board.transportrf24network.channel))
                if board.transportrf24network.key != "":
                    print("key:",rpcproxy.configure(key=list(map(int, board.transportrf24network.key.split(',')))))
                if board.transportrf24network.iv != "":
                    print("iv:",rpcproxy.configure(iv=list(map(int, board.transportrf24network.iv.split(',')))))

        except ObjectDoesNotExist:
            print("transport rf24network not present")

        try:
            if ( board.transportcan.active):
                print("CAN Transport",board.transportcan)
                print("cansampletime:",rpcproxy.configure(cansampletime=board.transportcan.cansampletime))
                print("node_id, subject, subject_id:",rpcproxy.configure(node_id=board.transportcan.node_id,
                                                                         subject=board.transportcan.subject,
                                                                         subject_id=board.transportcan.subject_id))

        except ObjectDoesNotExist:
            print("transport can not present")

            
        print(">>>> sensors:")
        for sensor in board.sensor_set.all():
            if not sensor.active: continue

            report_time=find_report_time(mystation)
            timerange=sensor.timerange.format(P2=report_time)
            
            if (version == "3"):            
                print("add driver:",rpcproxy.configure(driver=sensor.driver,
                                type=sensor.type.type,
                                node=sensor.node,address=sensor.address,
                                mqttpath=timerange+"/"+sensor.level+"/"))
            else:

                timerange=timerange.split(",")
                for i in range(len(timerange)):
                    if (timerange[i] == "-"):
                        timerange[i] = None
                    else:
                        timerange[i] = int(timerange[i])
                        
                level=sensor.level.split(",")
                for i in range(len(level)):
                    if (level[i] == "-"):
                        level[i] = None
                    else:
                        level[i] = int(level[i])
                            
                print("add driver:",rpcproxy.configure(driver=sensor.driver,
                                                       type=sensor.type.type,
                                                       timerange=timerange,
                                                       level=level))
                if not sensor.node is None:    print("    driver:",rpcproxy.configure(node=sensor.node))
                if not sensor.address is None: print("    driver:",rpcproxy.configure(address=sensor.address))

        #TODO  check id of status (good only > 0)
            
        print(">>>>>>> save config")
        if (isinstance(transport, jsonrpc.TransportSERIAL)):
            transport.ser.timeout=8    # save on eeprom require time
        print("save",rpcproxy.configure(save=True ))
        if (isinstance(transport, jsonrpc.TransportSERIAL)):
            transport.ser.timeout=1

    print("----------------------------- station configured : REBOOT ---------------------------------")
    print("reboot:",rpcproxy.reboot())
        
    transport.close()

            
def configstation_to_struct_v3(station_slug=None,board_slug=None,username=None):

    if (station_slug is None): return
    if (username is None): return

    myconfiguration=configuration()
    myconfiguration.module_main_version=3
    myconfiguration.module_configuration_version=2
    myconfiguration.module_type=4
    
    mystation=StationMetadata.objects.get(slug=station_slug,user__username=username)

    if not mystation.active:
        print("disactivated station: do nothing!")
        return None

    for board in mystation.board_set.all():

        if board_slug is not None and board.slug != board_slug:
            continue

        if not board.active:
            continue

        print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> configure board: ", board.name," slug="+board.slug)
            
        try:
            if ( board.transportmqtt.active):
                print("MQTT Transport",board.transportmqtt)

                myconfiguration.constantdata_count=0                
                for constantdata in  mystation.stationconstantdata_set.all():
                    if ( constantdata.active):
                        myconfiguration.constantdata[myconfiguration.constantdata_count].btable=bytes(constantdata.btable,encoding='ascii')
                        myconfiguration.constantdata[myconfiguration.constantdata_count].value=bytes(constantdata.value,encoding='ascii')
                        myconfiguration.constantdata_count+=1                        

            myconfiguration.mqtt_port=1883
            myconfiguration.mqtt_server=bytes(board.transportmqtt.mqttserver,encoding='ascii')
            myconfiguration.mqtt_root_topic=bytes("1/"+mystation.mqttrootpath+"/"+board.transportmqtt.mqttuser\
                                                  +"/"+ mystation.ident +"/"\
                                                  +"%d,%d" % (nint(mystation.lon*100000),nint(mystation.lat*100000))\
                                                  +"/"+mystation.network+"/",encoding='ascii')
            myconfiguration.mqtt_maint_topic=bytes("1/"+mystation.mqttmaintpath+"/"+board.transportmqtt.mqttuser\
                +"/"+ mystation.ident +"/"\
                +"%d,%d" % (nint(mystation.lon*100000),nint(mystation.lat*100000))\
                +"/"+mystation.network+"/",encoding='ascii')
            myconfiguration.mqtt_rpc_topic=bytes("1/rpc/"+board.transportmqtt.mqttuser\
                +"/"+ mystation.ident +"/"\
                +"%d,%d" % (nint(mystation.lon*100000),nint(mystation.lat*100000))\
                +"/"+mystation.network+"/",encoding='ascii')
            myconfiguration.mqtt_username=bytes(board.transportmqtt.mqttuser,encoding='ascii')
            myconfiguration.mqtt_password=bytes(board.transportmqtt.mqttpassword,encoding='ascii')

                    
            myconfiguration.report_seconds=board.transportmqtt.mqttsampletime
            myconfiguration.stationslug=bytes(mystation.slug,encoding='ascii')
            myconfiguration.boardslug=bytes(board.slug,encoding='ascii')
                                        
        except ObjectDoesNotExist:
            print("transport mqtt not present")


        try:
            if ( board.transporttcpip.active):
                print("TCP/IP Transport",board.transporttcpip)
                myconfiguration.ntp_server=bytes(board.transporttcpip.ntpserver,encoding='ascii')
                myconfiguration.gsm_apn=bytes(board.transporttcpip.gsmapn,encoding='ascii')
                myconfiguration.gsm_username=bytes("",encoding='ascii')
                myconfiguration.gsm_password=bytes("",encoding='ascii')
                
        except ObjectDoesNotExist:
            print("transport tcpip not present")

                   
        print(">>>> sensors:")
        myconfiguration.sensors_count=0        
        for sensor in board.sensor_set.all():
            if not sensor.active: continue

            report_time=find_report_time(mystation)
            timerange=sensor.timerange.format(P2=report_time)
            
            myconfiguration.sensors[myconfiguration.sensors_count].driver=bytes(sensor.driver,encoding='ascii')
            myconfiguration.sensors[myconfiguration.sensors_count].type=bytes(sensor.type.type,encoding='ascii')
            myconfiguration.sensors[myconfiguration.sensors_count].node= 0 if (sensor.node is None) else sensor.node
            myconfiguration.sensors[myconfiguration.sensors_count].address=sensor.address
            myconfiguration.sensors[myconfiguration.sensors_count].mqtt_topic=bytes(timerange+"/"+sensor.level+"/",encoding='ascii')
            myconfiguration.sensors_count+=1        

        return myconfiguration

    
def send2http(body="",user=None,password=None,url=None,login_url=None):

    print("send data to: {}".format(url))

    with requests.Session() as session:

        session.get(login_url)
        csrftoken = session.cookies['csrftoken']
        r = session.post(login_url, data={"username":user, "password":password,"csrfmiddlewaretoken":csrftoken, "next": "/"}, headers={"Referer":login_url})

        if r.status_code == 200:
            if r.text.find('<input type="password" name="password" required id="id_password">') != -1:
                print("ALERT: Wrong username or password")
                return
        else:
            print ("Error code login {}".format(r.status_code))
            return

        r = session.post(url, data={"body": body})
    
    if r.status_code != 200:
        print ("Error code {}".format(r.status_code))
    print (r.text)
        
def send2amqp(body="",user=None,password=None,host="rmap.cc",exchange=".in.json.configuration",routing_key="config"):

    credentials=pika.PlainCredentials(user, password)
    properties=pika.BasicProperties(
        user_id= user,
        delivery_mode = 2, # persistent
    )

    channel=None
    connection=None
    
    try:
        connection = pika.BlockingConnection(pika.ConnectionParameters(
            host=host,credentials=credentials,socket_timeout=5000))
        channel = connection.channel()

        channel.confirm_delivery()

        channel.basic_publish(exchange=exchange,
                              routing_key=routing_key,
                              body=body,
                              properties=properties,
                              mandatory=True)
        print(" [x] Message Sent ")
            
    except Exception as e:
        print(" [x] Error on publish ")
        print(("PikaMQ publish error ", e)) 

    if (channel is not None): channel.close()
    if (connection is not None): connection.close()

def export2json(objects):

    return serializers.serialize('json', objects, indent=2,
                                 use_natural_foreign_keys=True, use_natural_primary_keys=True
                                 #   ,fields=("name", "active", "slug", "lat", "lon", "network",
                                 #   "mqttrootpath", "mqttmaintpath", "driver", "type", "address"
                                 #   "timerange", "level")
                                 )

    
def dumpstation(user, station_slug, board_slug=None, without_password=False,dump=False):

    def add_board(objects,board):
        if (board.active):
            objects.append(board)
        
            for sensor in board.sensor_set.all():
                if (sensor.active):
                    if not dump: sensor.timerange=sensor.dynamic_timerange()
                    objects.append(sensor)
            try:
                transport=board.transportmqtt
                if without_password:
                    transport.mqttpassword=None   # use make_password to generate sha
                    transport.mqttpskkey=None
                if (transport.active): objects.append(transport)
            except ObjectDoesNotExist:
                pass
            try:
                transport=board.transportbluetooth
                if (transport.active): objects.append(transport)
            except ObjectDoesNotExist:
                pass
            try:
                transport=board.transportamqp
                if without_password:
                    transport.amqppassword=None   # use make_password to generate sha
                if (transport.active): objects.append(transport)
            except ObjectDoesNotExist:
                pass
            try:
                transport=board.transportserial
                if (transport.active): objects.append(transport)
            except ObjectDoesNotExist:
                pass
            try:
                transport=board.transporttcpip
                if (transport.active): objects.append(transport)
            except ObjectDoesNotExist:
                pass
            try:
                transport=board.transportcan
                if (transport.active): objects.append(transport)
            except ObjectDoesNotExist:
                pass
            
    objects=[]

    try:
        mystation=StationMetadata.objects.get(slug=station_slug,user__username=user)
        if mystation.active:
            objects.append(mystation)

            if (board_slug):
                add_board(objects,mystation.board_set.get(slug=board_slug))
            else:
                for board in mystation.board_set.all():
                    add_board(objects,board)
        
            for stationconstantdata in mystation.stationconstantdata_set.all():
                if (stationconstantdata.active): objects.append(stationconstantdata)

    except ObjectDoesNotExist:
        print("Station not found!")
        return None
    
    return export2json(objects)


def sendjson2amqp(station,user="your user",password="your password",host="rmap.cc",exchange=".in.json.configuration"):

    print("sendjson2amqp")

    body=dumpstation(user,station,dump=True)
    if body is not None:
        send2amqp(body,user,password,host,exchange)


def sendjson2http(station,user="your user",password="your password",host="rmap.cc"):

    print("sendjson2http")

    url = "https://"+host+"/stationsupload/json/"
    login_url = "https://"+host+"/registrazione/login/"
                  
    body=dumpstation(user,station,dump=True)
    if body is not None:
        send2http(body,user,password,url,login_url)


def receivegeoimagefromamqp(user="your user",password="your password",host="rmap.cc",queue="..jpg.photo"):

    from geoimage.models import GeorefencedImage

    def callback(ch, method, properties, body):
        print(" [x] Received message")

        if properties.user_id is None:
            print("Ignore anonymous message")
            print(" [x] Done")
            ch.basic_ack(delivery_tag = method.delivery_tag)
            return
  
        #At this point we can check if we trust this authenticated user... 
        user=properties.user_id
        print("Received from user: %r" % user) 

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

            print("getted those metadata from exif:")
            print(lat,lon)
            print(comment)
            print(date)
            print(imgident)

            if (imgident == user):
                geoimage=GeorefencedImage()
                geoimage.geom = {'type': 'Point', 'coordinates': [lon, lat]}
                geoimage.comment=comment
                geoimage.date=date

                try:
                    geoimage.ident=User.objects.get(username=user)
                    geoimage.image.save('geoimage.jpg',ContentFile(body))
                    geoimage.save()
                except User.DoesNotExist:
                    print("user does not exist")
            else:
                print("reject:",user)

        except Exception as e:
            print(e)
            raise

        print(" [x] Done")
        ch.basic_ack(delivery_tag = method.delivery_tag)

    credentials=pika.PlainCredentials(user, password)

    connection = pika.BlockingConnection(pika.ConnectionParameters(
        host=host,credentials=credentials))
    channel = connection.channel()
    #channel.queue_declare(queue=queue)

    print(' [*] Waiting for messages. To exit press CTRL+C')


    channel.basic_consume(on_message_callback=callback,
                          queue=queue,
                          auto_ack=False)

    channel.start_consuming()

    connection.close()
    sendconnection.close()



def receivejsonfromamqp(user="your user",password="your password",host="rmap.cc",queue=""):

    def callback(ch, method, properties, body):
        print(" [x] Received message")

        if properties.user_id is None:
            print("Ignore anonymous message")
            print(" [I] Ignore")
            ch.basic_ack(delivery_tag = method.delivery_tag)
            print(" [x] Done")
            return
  
        #At this point we can check if we trust this authenticated user... 
        user=properties.user_id
        print("Received from user: %r" % user) 
        
        #but we check that message content is with the same user
        try:
            for deserialized_object in serializers.deserialize("json",body):
                if object_auth(deserialized_object.object,user):
                    try:
                        try:
                            StationMetadata.objects.get(slug=deserialized_object.object.slug,user__username=deserialized_object.object.user.username).delete()
                        except:
                            pass
                        print("save:",deserialized_object.object)
                        deserialized_object.save(force_insert=True)
                    except Exception as e:
                        print((" [E] Error saving in DB",e))
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
                    print("reject:",deserialized_object.object)
                    ch.basic_ack(delivery_tag = method.delivery_tag)
                    print(" [R] Rejected")
                    return

        except Exception as e:
            print(("error in deserialize object; skip it",e))

        ch.basic_ack(delivery_tag = method.delivery_tag)
        print(" [x] Done")

        #close django connection to DB
        try:
            connection.close()
        except Exception as e:
            print(("django connection close error",e))


    credentials=pika.PlainCredentials(user, password)

    amqpconnection = pika.BlockingConnection(pika.ConnectionParameters(
        host=host,credentials=credentials))
    channel = amqpconnection.channel()
    #channel.queue_declare(queue=queue)

    print(' [*] Waiting for messages. To exit press CTRL+C')


    channel.basic_consume(on_message_callback=callback,
                          queue=queue,
                          auto_ack=False)

    channel.start_consuming()

    amqpconnection.close()


def object_auth(object,user):
    #print object
    #print type(object)

    if isinstance(object,StationMetadata):
        if object.user.username == user:
            return True

    if isinstance(object,Board):
        if object.stationmetadata.user.username == user:
            return True

    if isinstance(object,Sensor):
        if object.board.stationmetadata.user.username == user:
            return True

    if isinstance(object,TransportRF24Network):
        if object.board.stationmetadata.user.username == user:
            return True

    if isinstance(object,TransportBluetooth):
        if object.board.stationmetadata.user.username == user:
            return True

    if isinstance(object,TransportAmqp):
        if object.board.stationmetadata.user.username == user:
            return True

    if isinstance(object,TransportMqtt):
        if object.board.stationmetadata.user.username == user:
            return True
        
    if isinstance(object,TransportSerial):
        if object.board.stationmetadata.user.username == user:
            return True

    if isinstance(object,TransportTcpip):
        if object.board.stationmetadata.user.username == user:
            return True

    if isinstance(object,TransportCan):
        if object.board.stationmetadata.user.username == user:
            return True
        
    if isinstance(object,StationConstantData):
        if object.stationmetadata.user.username == user:
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

    print("elaborate station: ",station)

    mystation=StationMetadata.objects.get(slug=station,user__username=username)

    if not (activate is None):
        mystation.active=activate
        mystation.clean()
        mystation.save()

    if not (activateboard is None) and not (board is None):
        for myboard in mystation.board_set.all():
            print("elaborate board: ",myboard)

            if not (myboard.slug == board): continue
            if not (activateboard is None): 
                myboard.active=activateboard
                myboard.save()



def configuser(username="rmap",password="rmap"):

    try:
        user = User.objects.create_user(username, username+'@rmap.cc', password)            
        #trap IntegrityError for user that already exist
    except IntegrityError:
        pass
    except:
        raise

    updateusername(oldusername=username,newusername=username,newpassword=password)
        

                
def configdb(username="rmap",password="rmap",
             station="home",lat=0,lon=0,constantdata={},network="fixed",
             mqttusername="your user",
             mqttpassword="your password",
             mqttpskkey="12345678901234567890123456789012",
             mqttserver="rmap.cc",
             mqttsamplerate=5,
             bluetoothname="hc06",
             amqpusername="your user",
             amqppassword="your password",
             amqpserver="rmap.cc",
             queue="rmap",
             exchange=".in.json",
             board=None,
             activate=None,
             stationname=None,
             mqttrootpath=None,
             mqttmaintpath=None):


    configuser(username,password)
        
    try:

        print("elaborate station: ",station)

        try:
            StationMetadata.objects.get(slug=station,user__username=username).delete()
        except ObjectDoesNotExist:
            pass

        if (stationname is None):
            stationname=""
        mystation=StationMetadata(slug=station,name=stationname)
        user=User.objects.get(username=username)
            
        mystation.user=user
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

    ## remove all StationConstantData
    #try:
    #    StationConstantData.objects.filter(stationmetadata=mystation).delete()
    #except:
    #    pass

    for btable,value in constantdata.items():

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
        print("elaborate board: ",myboard)

        if board is None:
            if not myboard.active: continue
        else:
            if not myboard.slug == board: continue
            if not (activate is None): myboard.active=activate
            myboard.save()

        try:
            if ( myboard.transportmqtt.active):
                print("MQTT Transport", myboard.transportmqtt)
                
                myboard.transportmqtt.mqttserver=mqttserver
                myboard.transportmqtt.mqttuser=mqttusername
                myboard.transportmqtt.mqttpassword=mqttpassword
                myboard.transportmqtt.mqttpskkey=mqttpskkey
                myboard.transportmqtt.mqttsampletime=mqttsamplerate
                myboard.transportmqtt.save()
                
        except ObjectDoesNotExist:
            print("transport MQTT not present for this board")


        try:
            if ( myboard.transportbluetooth.active):
                print("bluetooth Transport", myboard.transportbluetooth)

                myboard.transportbluetooth.name=bluetoothname
                myboard.transportbluetooth.save()

        except ObjectDoesNotExist:
            print("transport Bluetooth not present for this board")

        try:
            if ( myboard.transportamqp.active):
                print("AMQP Transport", myboard.transportamqp)

                myboard.transportamqp.amqpuser=amqpusername
                myboard.transportamqp.amqppassword=amqppassword
                
                myboard.transportamqp.amqpserver=amqpserver
                myboard.transportamqp.queue=queue
                myboard.transportamqp.exchange=exchange
                
                myboard.transportamqp.save()

        except ObjectDoesNotExist:
            print("transport AMQP not present for this board")


        # TODO Serial TCPIP

def compact(myrpc,mydata):
    def bin(s):
        return str(s) if s<=1 else bin(s>>1) + str(s&1)

    def bitprepend(template,bit,nbit):
        return (template<<nbit) | bit

    def bitextract(template,start,nbit):
        return (template>>start)&((1 << nbit) - 1)

    def bit2bytelist(template,totbit):
        data=[]
        start=totbit-8
        while start >= 0:
            data.append(bitextract(template,start,8))
            start-=8
        if start >=-7:
            data.append(bitextract(template,0,8+start)<< -start )
        return data

    # start encoding
    template=0
    totbit=0
    #set data template number
    nbit=8
    template=bitprepend(template,myrpc,nbit)
    totbit+=nbit

    #insert data
    if (myrpc == 1):
    
        nbit=1
        template=bitprepend(template,int(mydata["save"]),nbit)
        totbit+=nbit
        
        nbit=16
        template=bitprepend(template,mydata["sampletime"],nbit)
        totbit+=nbit

    elif (myrpc == 2):

        #[{"n":0,"s":True},{"n":3,"s":False}]
        for pin in mydata:
            nbit=4
            template=bitprepend(template,pin["n"],nbit)
            totbit+=nbit
        
            nbit=1
            template=bitprepend(template,pin["s"],nbit)
            totbit+=nbit

        
    print("totbit: ",totbit)
    print(bin(template))

    #create a list of bytes
    data=bit2bytelist(template,totbit)
    
    binstring=""
    for byte in data:
        binstring+=chr(byte)

    
    ##convert endian
    #for i in xrange(0,len(data),2):
    #    tmp=data[i]
    #    data[i]=data[i+1]
    #    data[i+1]=tmp
        
    return binstring


class Message(object):
    """ messaggio ricevuto o di conferma usato nella classe 
    amqpConsumerProducer
    body è il payload del messaggio
    delivery_tag il tag amqp del messaggio
    se il messaggio è usato per conferma/disconferma
    il payload dovrà contenere rispettivamente True for "acked"
    and False for "nacked"
    """

    def __init__(self,body,delivery_tag=None):
        self.delivery_tag=delivery_tag
        self.body=body



class amqpConsumerProducer(threading.Thread):
    """questa classe gestisce la comunicazione con un broker amqp
    Viene eseguita in un thread  attivato con il metodo start()
    e terminato con un segnale inviato dal metodo terminate()
    se definita una coda vengono consumati i messaggi e se definita viene eseguita pipefunction
    a cui viene passata self stessa per gestire l'eventuale ack del messaggio stesso.
    se definito un exchange è possibile pubblicare messaggi inviandoli alla coda del thread send_queue
    I messaggi ricevuti possono essere confermati in modo asincrono inviando un Message alla coda thread
    receive_queue
    """

    def __init__(self,host="localhost",queue=None,exchange=None,user="guest",password="guest",
                 send_queue=None,receive_queue=None,
                 pipefunction=None,prefetch_count=50,
                 logging=logging):
        """Create a new instance of the producer consumer class, passing in the AMQP
        URL used to connect to RabbitMQ.

        :param str amqp_url: The AMQP url to connect with

        """

        threading.Thread.__init__(self)

        self._connection = None
        self._channel = None
        self._stopping = False
        self._host = host
        self._queue=queue
        self._exchange=exchange
        self._user=user
        self._password=password
        self._logging=logging
        self._running = False
        self.send_queue=send_queue
        self.receive_queue=receive_queue

        self._deliveries = {}
        self._acked = None
        self._nacked = None
        self._message_number = None
        self._terminate_event = threading.Event()
        self._prefetch_count = prefetch_count 
        self._pipefunction=pipefunction
        self._last_delivered_msg_tag=0
 
    def terminate(self):
        self._terminate_event.set()

    def have_to_terminate(self):
        return self._terminate_event.is_set()
        
    def publish(self,body,totaltag=None):
        """This method publish message to RabbitMQ exhange
        """

        if (self._exchange is None):
            self._logging.error('Try to Publish not defined exchange')
            return False
        
        self._logging.info('Publish to %s', self._exchange)

        #set user_id property to don't get  "PRECONDITION_FAILED - user_id property set to 'guest' but authenticated user was 'rmap'
        properties=pika.BasicProperties(
            user_id= self._user,
            delivery_mode = 2, # persistent
        )
        routing_key=self._user

        try:
            self._channel.basic_publish(exchange=self._exchange,
                                  routing_key=routing_key,
                                  body=body,
                                  properties=properties)
            self._message_number += 1
            self._deliveries[self._message_number]=totaltag            
            self._logging.debug('Message published # %i',self._message_number)
            return True
        except pika.exceptions.UnroutableError:
            self._logging.error('Message UnroutableError')
        except:
            self._logging.error('Message could not be sended')

        return False
        
        
    def connect(self):
        """This method connects to RabbitMQ, returning the connection handle.
        When the connection is established, the on_connection_open method
        will be invoked by pika.

        :rtype: pika.SelectConnection

        """
        self._logging.info('Connecting to %s', self._host)
        self._logging.info('User %s; Password %s', self._user,self._password)

        credentials=pika.PlainCredentials(self._user, self._password)
        self._connection= pika.SelectConnection(pika.ConnectionParameters(host=self._host,credentials=credentials),

                                                on_open_callback=self.on_connection_open,
                                                on_open_error_callback=self.on_connection_open_error,
                                                on_close_callback=self.on_connection_closed)


    def on_connection_open(self, unused_connection):
        """This method is called by pika once the connection to RabbitMQ has
        been established. It passes the handle to the connection object in
        case we need it, but in this case, we'll just mark it unused.

        :type unused_connection: pika.SelectConnection

        """
        self._logging.info('Connection opened')
        self.open_channel()

    def on_connection_open_error(self, _unused_connection, err):
        """This method is called by pika if the connection to RabbitMQ
        can't be established.
        :param pika.SelectConnection _unused_connection: The connection
        :param Exception err: The error
        """

        self._logging.error('Connection open failed, reopening in 5 seconds: %s', err)
        self._connection.ioloop.call_later(5, self._connection.ioloop.stop)


    def on_connection_closed(self, _unused_connection, reason):
        """This method is invoked by pika when the connection to RabbitMQ is
        closed unexpectedly. Since it is unexpected, we will reconnect to
        RabbitMQ if it disconnects.

        :param pika.connection.Connection connection: The closed connection obj
        :param int reply_code: The server provided reply_code if given
        :param str reply_text: The server provided reply_text if given

        """
        self._channel = None
        if self._stopping:
            self._connection.ioloop.stop()
        else:
            self._logging.warning('Connection closed, reopening in 5 seconds: %s',
                                  reason)
            self._connection.ioloop.call_later(5, self._connection.ioloop.stop)


    def open_channel(self):
        """Open a new channel with RabbitMQ by issuing the Channel.Open RPC
        command. When RabbitMQ responds that the channel is open, the
        on_channel_open callback will be invoked by pika.

        """
        self._logging.info('Creating a new channel')
        self._connection.channel(on_open_callback=self.on_channel_open)
        
    def on_channel_open(self, channel):
        """This method is invoked by pika when the channel has been opened.
        The channel object is passed in so we can make use of it.

        :param pika.channel.Channel channel: The channel object

        """
        self._logging.info('Channel opened')
        self._channel = channel
        self.add_on_channel_close_callback()

        # Turn on delivery confirmations
        self._channel.confirm_delivery(ack_nack_callback=self.on_delivery_confirmation)

        if (self._queue is not None):
            self.set_qos()
        
        
    def on_delivery_confirmation(self,method_frame):

        m = method_frame.method
        confirmation_type = m.NAME.split('.')[1].lower()
        multiple = m.multiple
        delivery_tag = m.delivery_tag

        self._logging.info('Received %s %sfor delivery tag: %i',
                confirmation_type, '*multiple* ' if multiple else '', delivery_tag)

        #compute list of tags
        if (multiple):
            if delivery_tag == 0:
                num_acks = len(self._deliveries)
                tags=self._deliveries.keys()
            else:
                num_acks = delivery_tag - self._last_delivered_msg_tag
                tags=[]
                for tag in range(self._last_delivered_msg_tag + 1, delivery_tag + 1):
                    tags.append(tag)
        else:
            num_acks = 1
            tags=[method_frame.method.delivery_tag,]


        #compute num acks or nacks
        if confirmation_type == 'ack':
            self._acked += num_acks
        elif confirmation_type == 'nack':
            self._nacked += num_acks
            
        #remove tags from deliveries
        totaltag=[]
        for tag in tags:
            totaltag.extend(self._deliveries[tag])
            if confirmation_type == 'ack':
                status=True
                self._logging.info(" [x] Done")
            elif confirmation_type == 'nack':
                status=False
                self._logging.info(" [ ] NOT Done")
            else:
                self._logging.error("Strange condition on delivery_confirmation ")

            self._deliveries.pop(tag)

        #acknowledge or reject to reading queue
        for tag in totaltag:
            if (tag is not None):
                if (self.receive_queue is not None):
                    self.receive_queue.put_nowait(Message(status,tag))
                else:
                    if (status):
                        self.acknowledge_message(tag)
                    else:
                        self.reject_message(tag)

        self._last_delivered_msg_tag = delivery_tag

        self._logging.info(
            'Published %i messages, %i have yet to be confirmed, '
            '%i were acked and %i were nacked', self._message_number,
            len(self._deliveries), self._acked, self._nacked)

        
    def add_on_channel_close_callback(self):
        """This method tells pika to call the on_channel_closed method if
        RabbitMQ unexpectedly closes the channel.

        """
        self._logging.info('Adding channel close callback')
        self._channel.add_on_close_callback(self.on_channel_closed)

    def on_channel_closed(self, channel, reason):
        """Invoked by pika when RabbitMQ unexpectedly closes the channel.
        Channels are usually closed if you attempt to do something that
        violates the protocol, such as re-declare an exchange or queue with
        different parameters. In this case, we'll close the connection
        to shutdown the object.

        :param pika.channel.Channel: The closed channel
        :param int reply_code: The numeric reason the channel was closed
        :param str reply_text: The text reason the channel was closed

        """
        self._logging.warning('Channel %i was closed: (%s)',
                              channel, reason)
        self._channel = None
        if not self._stopping:
            try:
                self._connection.close()
            except:
                pass

    def close_channel(self):
        """Call to close the channel with RabbitMQ cleanly by issuing the
        Channel.Close RPC command.

        """
        self._logging.info('Closing the channel')
        self._channel.close()

    def publish_messages_from_thread_queue(self):
        "get messages from thead queue and publish to AMQP"

        def send_message(totalbody,totaltag):
            "send message"
            
            self._logging.debug("elaborate %s bytes" % len(totalbody))

            if (self.publish(totalbody,totaltag)):
                self._logging.debug("message published ")
                status=True
            else:
                self._logging.error("There were some errors sendin message to AMQP")
                self._logging.debug("skip message:")
                self._logging.debug(totalbody)
                #self._logging.debug("---------------------")
                #self._logging.error('Exception occured: ' + str(exception))
                #self._logging.error(traceback.format_exc())

                # requeue message for retry
                #self.send_queue.put_nowait(message)                    
                status=False
                

        totalbody=b""
        totaltag=[]
        while not self.send_queue.empty():
            # get a message
            message=self.send_queue.get()
            self.send_queue.task_done()                    
            totalbody+=(message.body)
            totaltag.append(message.delivery_tag)
            if (len(totalbody) > 1000000):
                send_message(totalbody,totaltag)
                totalbody=b""
                totaltag=[]

        if len(totalbody) > 0:
            send_message(totalbody,totaltag)

        self._connection.ioloop.call_later(3, self.publish_messages_from_thread_queue)

    def ack_messages_from_thread_queue(self):
        "confirm or not messages from thead queue"

        while not self.receive_queue.empty():
            # get a message
            message=self.receive_queue.get()

            if (message.body):
                self.acknowledge_message(message.delivery_tag)
            else:
                self.reject_message(message.delivery_tag)
            
            self.receive_queue.task_done()

        self._connection.ioloop.call_later(3, self.ack_messages_from_thread_queue)       # confirm/noconfirm messages from theading queue
            
    def set_qos(self):
        """This method sets up the consumer prefetch to only be delivered
        one message at a time. The consumer must acknowledge this message
        before RabbitMQ will deliver another one. You should experiment
        with different prefetch values to achieve desired performance.
        """
        self._channel.basic_qos(
            prefetch_count=self._prefetch_count, callback=self.on_basic_qos_ok)

    def on_basic_qos_ok(self, _unused_frame):
        """Invoked by pika when the Basic.QoS method has completed. At this
        point we will start consuming messages by calling start_consuming
        which will invoke the needed RPC commands to start the process.
        :param pika.frame.Method _unused_frame: The Basic.QosOk response frame
        """
        self._logging.info('QOS set to: %d', self._prefetch_count)
        self.start_consuming()

    def start_consuming(self):
        """This method sets up the consumer by first calling
        add_on_cancel_callback so that the object is notified if RabbitMQ
        cancels the consumer. It then issues the Basic.Consume RPC command
        which returns the consumer tag that is used to uniquely identify the
        consumer with RabbitMQ. We keep the value to use it when we want to
        cancel consuming. The on_message method is passed in as a callback pika
        will invoke when a message is fully received.
        """
        self._logging.info('Issuing consumer related RPC commands')
        self.add_on_cancel_callback()
        self._consumer_tag = self._channel.basic_consume(
            self._queue, self.on_message)

    def add_on_cancel_callback(self):
        """Add a callback that will be invoked if RabbitMQ cancels the consumer
        for some reason. If RabbitMQ does cancel the consumer,
        on_consumer_cancelled will be invoked by pika.
        """
        self._logging.info('Adding consumer cancellation callback')
        self._channel.add_on_cancel_callback(self.on_consumer_cancelled)

    def on_consumer_cancelled(self, method_frame):
        """Invoked by pika when RabbitMQ sends a Basic.Cancel for a consumer
        receiving messages.
        :param pika.frame.Method method_frame: The Basic.Cancel frame
        """
        self._logging.info('Consumer was cancelled remotely, shutting down: %r',
                    method_frame)
        if self._channel:
            self._channel.close()

    def acknowledge_message(self, delivery_tag):
        """Acknowledge the message delivery from RabbitMQ by sending a
        Basic.Ack RPC method for the delivery tag.
        :param int delivery_tag: The delivery tag from the Basic.Deliver frame
        """
        self._logging.info('Acknowledging message %s', delivery_tag)
        self._channel.basic_ack(delivery_tag)

    def reject_message(self, delivery_tag):
        """Do not acknowledge the message delivery from RabbitMQ 
        It can be used to interrupt and cancel large incoming messages, or
        return untreatable messages to their original queue.
        :param int delivery_tag: The delivery tag from the Basic.Deliver frame
        """
        self._logging.info('Reject message %s', delivery_tag)
        self._channel.basic_nack(delivery_tag)
        
    def stop_consuming(self):
        """Tell RabbitMQ that you would like to stop consuming by sending the
        Basic.Cancel RPC command.
        """
        if self._channel:
            self._logging.info('Sending a Basic.Cancel RPC command to RabbitMQ')
            cb = functools.partial(
                self.on_cancelok, userdata=self._consumer_tag)
            self._channel.basic_cancel(self._consumer_tag, cb)

    def on_cancelok(self, _unused_frame, userdata):
        """This method is invoked by pika when RabbitMQ acknowledges the
        cancellation of a consumer. At this point we will close the channel.
        This will invoke the on_channel_closed method once the channel has been
        closed, which will in-turn close the connection.
        :param pika.frame.Method _unused_frame: The Basic.CancelOk frame
        :param str|unicode userdata: Extra user data (consumer tag)
        """
        self._logging.info(
            'RabbitMQ acknowledged the cancellation of the consumer: %s',
            userdata)
        self.close_channel()


    def on_message(self, _unused_channel, basic_deliver, properties, body):
        """Invoked by pika when a message is delivered from RabbitMQ. The
        channel is passed for your convenience. The basic_deliver object that
        is passed in carries the exchange, routing key, delivery tag and
        a redelivered flag for the message. The properties passed in is an
        instance of BasicProperties with the message properties and the body
        is the message that was sent.
        :param pika.channel.Channel _unused_channel: The channel object
        :param pika.Spec.Basic.Deliver: basic_deliver method
        :param pika.Spec.BasicProperties: properties
        :param bytes body: The message body
        """
        self._logging.debug('Received message # %s from %s: %s',
                           basic_deliver.delivery_tag, properties.app_id, body)

        if (self._pipefunction is not None):
            self._pipefunction(self,basic_deliver, properties, body)
        else:
            if (self.send_queue is not None):
                message=Message(body,basic_deliver.delivery_tag)
                self.send_queue.put_nowait(message)                    


    def check_terminate(self):
        if(self.have_to_terminate()):
            self.stop()
        else:
            self._connection.ioloop.call_later(3, self.check_terminate)
                
                
    def begin(self):
        """Run the consumer by connecting to RabbitMQ and then
        starting the IOLoop to block and allow the SelectConnection to operate.

        """
        self._deliveries = {}
        self._acked = 0
        self._nacked = 0
        self._message_number = 0
        
        self.connect()
        if (self.send_queue is not None and self._exchange is not None):
            self._connection.ioloop.call_later(10, self.publish_messages_from_thread_queue)   # get messages from theading queue

        if (self.receive_queue is not None and self._queue is not None):
            self._connection.ioloop.call_later(10, self.ack_messages_from_thread_queue)       # confirm/noconfirm messages from theading queue

        self._connection.ioloop.call_later(10, self.check_terminate) # get messages from amqp queue
        self._connection.ioloop.start()

    def run(self):
        while not self._stopping:
            try:
                self.begin()
                self.check_terminate()
            except KeyboardInterrupt:
                self.stop()
                if (self._connection is not None and
                    not self._connection.is_closed):
                    # Finish closing
                    self._connection.ioloop.start()
        self._logging.info('Terminated')

    def stop(self):
        """Cleanly shutdown the connection to RabbitMQ by stopping the consumer/producer
        with RabbitMQ. When RabbitMQ confirms the cancellation, on_cancelok
        will be invoked by pika, which will then closing the channel and
        connection. The IOLoop is started again because this method is invoked
        when CTRL-C is pressed raising a KeyboardInterrupt exception. This
        exception stops the IOLoop which needs to be running for pika to
        communicate with RabbitMQ. All of the commands issued prior to starting
        the IOLoop will be buffered but not processed.

        """
        self._logging.info('Stopping')
        self._stopping = True
        self.close_connection()
        self._logging.info('Stopped')

    def close_connection(self):
        """This method closes the connection to RabbitMQ."""
        self._logging.info('Closing connection')
        try:
            self._connection.close()
        except pika.exceptions.ConnectionWrongStateError:
            self._logging.info('attempt to close not open AMQP connection')
        
