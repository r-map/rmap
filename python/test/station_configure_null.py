#!/usr/bin/python

# Copyright (c) 2013 Paolo Patruno <p.patruno@iperbole.bologna.it>
#                    Emanuele Di Giacomo <edigiacomo@arpa.emr.it>
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 
# 1. Redistributions of source code must retain the above copyright notice,
#   this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
# 3. Neither the name of mosquitto nor the names of its
#   contributors may be used to endorse or promote products derived from
#   this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

import os
os.environ['DJANGO_SETTINGS_MODULE'] = 'rmap.settings'
import django
django.setup()

from rmap import jsonrpc
import time



from django.db import IntegrityError
from django.core.exceptions import ObjectDoesNotExist
from rmap.stations.models import StationMetadata
from rmap.stations.models import StationConstantData
from rmap.stations.models import Board
from rmap.stations.models import Sensor, SensorType
from rmap.stations.models import TransportMqtt
from rmap.stations.models import TransportBluetooth
from rmap.stations.models import TransportAmqp
from rmap.stations.models import TransportSerial
from django.contrib.auth.models import User
from django.core import serializers
from django.utils.translation import ugettext as _
from rmap.utils import nint
from rmap import jsonrpc
from django.core.files.base import ContentFile
from datetime import datetime
from django.db import connection




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
                                type=sensor.type,
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









cfg= configstation(transport_name="serial",station_slug="report_thp",board_slug="report_thp_master",logfunc=jsonrpc.log_file("rpc.log"),
                  device=None,baudrate=None,host=None,transport=None,username=None)













# USB

rpcproxy = jsonrpc.ServerProxy( jsonrpc.JsonRpc20(),\
                               jsonrpc.TransportSERIAL( logfunc=jsonrpc.log_file("logrpc.txt"),\
                                                        port='/dev/ttyUSB0',baudrate=9600,timeout=5) )


time.sleep(1)

print "start configuration"

status=False
while ( not status):
    try:
        status = rpcproxy.configure(reset=True )
    except:
        status = False
    print "reset:",status
    time.sleep(1)


status=False
while ( not status):
    try:
        status = rpcproxy.configure(mqttrootpath="meteo/-/1012345,4412345/rmap/")
    except:
        status = False
    print "mqttrootpath:",status
    time.sleep(1)

status=False
while ( not status):
    try:
        status = rpcproxy.configure(mqttsampletime=10,mqttserver="rmap.cc")
    except:
        status = False
    print "mqttserver:",status
    time.sleep(1)


status=False
while ( not status):
    try:
        status = rpcproxy.configure(mqttuser="",mqttpassword="")
    except:
        status = False
    print "mqtt user and password:",status
    time.sleep(1)


#status=False
#while ( not status):
#    try:
#        status = rpcproxy.configure(driver="TMP",type="TMP",node=0,address=72,mqttpath="254,0,0/105,2000,-,-/")
#    except:
#        status = False
#    print "add driver:", status
#    time.sleep(1)

#status = rpcproxy.configure(driver="ADT",type="ADT",node=0,address=75,mqttpath="254,0,0/105,1000,-,-/")
#print "add driver:", status

status=False
while ( not status):
    try:
        status = rpcproxy.configure(save=True )
    except:
        status = False
    print "save",status
    time.sleep(1)


