#!/usr/bin/env python

import sys
import os
os.environ['DJANGO_SETTINGS_MODULE'] = 'rmap.settings'
import django
django.setup()

from rmap.stations.models import StationMetadata
from rmap.stations.models import StationConstantData
from rmap.stations.models import Board
from rmap.stations.models import Sensor
from django.contrib.auth.models import User
from django.core.exceptions import ObjectDoesNotExist
import rmap.rmap_core

if __name__ == '__main__':

    if len(sys.argv) < 2:
        print "usage: ",sys.argv[0],"station_slug"
        sys.exit(1)

    objects=[]

    for station_slug in sys.argv[1:]:
        mystation=StationMetadata.objects.get(slug=station_slug)
        objects.append(mystation)
        for board in mystation.board_set.all():
            objects.append(board)

            for sensor in board.sensor_set.all():
                objects.append(sensor)

            try:
                if ( board.transportmqtt.active):
                    #print "MQTT Transport", board.transportmqtt
                    objects.append(board.transportmqtt)
            except ObjectDoesNotExist:
                #print "transport mqtt not present"
                pass
            try:
                if ( board.transporttcpip.active):
                    #print "TCPIP Transport", board.transporttcpip
                    objects.append(board.transporttcpip)
            except ObjectDoesNotExist:
                #print "transport tcpip not present"
                pass
            try:
                if ( board.transportserial.active):
                    #print "Serial Transport", board.transportserial
                    objects.append(board.transportserial)
            except ObjectDoesNotExist:
                #print "transport serial not present"
                pass
            try:
                if ( board.transportbluetooth.active):
                    #print "Bluetooth Transport", board.transportbluetooth
                    objects.append(board.transportbluetooth)
            except ObjectDoesNotExist:
                #print "transport bluetooth not present"
                pass
            try:
                if ( board.transportamqp.active):
                    #print "AMQP Transport", board.transportamqp
                    objects.append(board.transportamqp)
            except ObjectDoesNotExist:
                #print "transport amqp not present"
                pass
            try:
                if ( board.transportrf24network.active):
                    #print "RF24 Transport", board.transportrf24
                    objects.append(board.transportrf24network)
            except ObjectDoesNotExist:
                #print "transport rf24network not present"
                pass
        
    body = rmap.rmap_core.export2json(objects)


    print body

