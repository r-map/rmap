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
from stations.models import Board
from stations.models import Sensor
from django.contrib.auth.models import User
from django.core import serializers
import pika

def send2amqp(body="",user="your user",password="your password",host="rmap.cc",exchange="configuration",routing_key="config"):

    credentials=pika.PlainCredentials(user, password)
    properties=pika.BasicProperties(
        user_id= user,
        delivery_mode = 2, # persistent
    )

    connection = pika.BlockingConnection(pika.ConnectionParameters(
        host=host,credentials=credentials))
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


def sendjson2amqp(station,user=u"your user",password="your password",host="rmap.cc",exchange="configuration"):
    objects=[]

    mystation=StationMetadata.objects.get(slug=station)
    objects.append(mystation)
    for board in mystation.board_set.all():
        objects.append(board)
        
        for sensor in board.sensor_set.all():
            objects.append(sensor)
        
    body = export2json(objects)

    #print body

    send2amqp(body,user,password,host,exchange)



def receivejsonfromamqp(user=u"your user",password="your password",host="rmap.cc",queue="configuration"):

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
        
        #but we check that message content is with the same ident
        for deserialized_object in serializers.deserialize("json",body):
            if object_auth(deserialized_object.object,ident):
                print "save:",deserialized_object.object
                deserialized_object.save()
            else:
                print "reject:",deserialized_object.object

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



def configdb(username="your user",password="your password",
             station="home",lat=0,lon=0,constantdata={},
             mqttusername="your user",
             mqttpassword="your password",
             mqttserver="rmap.cc",
             mqttsamplerate=5,
             bluetoothname="hc05",
             amqpusername="your user",
             amqppassword="your password",
             amqpserver="rmap.cc",
             queue="rmap",
             exchange="rmap"):

    try:
        user = User.objects.create_user(username, username+'@rmap.cc', password)            
        #trap IntegrityError for user that already exist
    except IntegrityError:
        pass
    except:
        raise
        
    try:

        print "elaborate station: ",station

        mystation=StationMetadata.objects.get(slug=station)
        user=User.objects.get(username=username)
            
        mystation.ident=user
        mystation.lat=lat
        mystation.lon=lon
        mystation.active=True
        mystation.save()
            
    except:
        raise # "Error\nsetting station"

    try:
        for btable,value in constantdata.iteritems():
            StationConstantData.objects.filter(stationmetadata=mystation,btable=btable).delete()
    except:
        pass

    try:
        mystation.stationconstantdata_set.create(
            active=True,
            btable=btable,
            value=value
        )

    except:
        pass

    for board in mystation.board_set.all():
        print "elaborate board: ",board
        
        if not board.active: continue
        try:
            if ( board.transportmqtt.active):
                print "MQTT Transport", board.transportmqtt
                
                board.transportmqtt.mqttserver=mqttserver
                board.transportmqtt.mqttuser=mqttusername
                board.transportmqtt.mqttpassword=mqttpassword
                board.transportmqtt.mqttsampletime=mqttsamplerate
                board.transportmqtt.save()
                
        except ObjectDoesNotExist:
            print "transport MQTT not present for this board"


        try:
            if ( board.transportbluetooth.active):
                print "bluetooth Transport", board.transportbluetooth

                board.transportbluetooth.name=bluetoothname
                board.transportbluetooth.save()

        except ObjectDoesNotExist:
            print "transport Bluetooth not present for this board"

        try:
            if ( board.transportamqp.active):
                print "AMQP Transport", board.transportamqp

                board.transportamqp.amqpuser=amqpusername
                board.transportamqp.amqppassword=amqppassword
                
                board.transportamqp.amqpserver=amqpserver
                board.transportamqp.queue=queue
                board.transportamqp.exchange=exchange
                
                board.transportamqp.save()

        except ObjectDoesNotExist:
            print "transport AMQP not present for this board"


        # TODO Serial TCPIP

