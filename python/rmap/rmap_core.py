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
from stations.models import Sensor
from django.contrib.auth.models import User
from django.core import serializers
import pika
from rmap.utils import nint
from rmap import jsonrpc


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


def delsensor(board_slug=None,name=None):

    Sensor.objects.get(name=name,board__slug=board_slug).delete()

def delsensors(board_slug=None):

    Sensor.objects.filter(board__slug=board_slug).delete()


def addsensor(board_slug=None,name="my sensor",driver="TMP",type="TMP",i2cbus=1,address=72,node=1
              ,timerange="254,0,0",level="0,1"):
    #,sensortemplate=None):

    myboard = Board.objects.get(slug=board_slug)

    #if sensortemplate is None :
    mysensor=Sensor(board=myboard,active=True,name=name,driver=driver,type=type
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
            mysensor.save()
        except IntegrityError:
            oldsensor=Sensor.objects.get(board=myboard,active=True,name=name,driver=driver,type=type
                                         ,i2cbus=i2cbus,address=address,node=node
                                         ,timerange=timerange,level=level)
            oldsensor.delete()
            mysensor.save()

# the first is the default
template_choices = ["default","none","test","test_indirect","test_rf24","test_indirect_rf24","test_master"
                    ,"stima_base","stima_t_u","stima_t"]

def addsensors_by_template(board_slug=None,template=None):

    if (template == "default"):
        print "setting template:", template
        pass

    if (template == "none"):
        print "setting template:", template
        delsensors(board_slug=board_slug)

    if (template == "test"):
        print "setting template:", template
        delsensors(board_slug=board_slug)
        addsensor(board_slug=board_slug,name="stima test",driver="I2C",
                  type="TMP",address=72,timerange="254,0,0",level="0,1,-,-")

    if (template == "test_indirect"):
        print "setting template:", template
        delsensors(board_slug=board_slug)
        addsensor(board_slug=board_slug,name="stima test jrpc",driver="JRPC",
                  type="TMP",address=72,timerange="254,0,0",level="0,1,-,-")


    if (template == "test_rf24"):
        print "setting template:", template
        delsensors(board_slug=board_slug)
        addsensor(board_slug=board_slug,name="stima test rf24",driver="RF24",
                  type="TMP",address=72,timerange="254,0,0",level="0,2,-,-")

    if (template == "test_indirect_rf24"):
        print "setting template:", template
        delsensors(board_slug=board_slug)
        addsensor(board_slug=board_slug,name="stima test jrpc",driver="JRPC",
                  type="TMP",address=72,timerange="254,0,0",level="0,1,-,-")
        addsensor(board_slug=board_slug,name="stima test rf24",driver="RF24",
                  type="TMP",address=72,timerange="254,0,0",level="0,2,-,-")

    if (template == "test_master"):
        print "setting template:", template
        delsensors(board_slug=board_slug)
        addsensor(board_slug=board_slug,name="stima test",driver="i2c",
                  type="TMP",address=72,timerange="254,0,0",level="0,1,-,-")
        addsensor(board_slug=board_slug,name="stima test rf24",driver="RF24",
                  type="TMP",address=72,timerange="254,0,0",level="0,2,-,-")

    if (template == "test_base"):
        print "setting template:", template
        delsensors(board_slug=board_slug)
        addsensor(board_slug=board_slug,name="Temperature",driver="I2C",
                  type="TMP",address=72,timerange="254,0,0",level="0,1,-,-")
        addsensor(board_slug=board_slug,name="Humidity",driver="I2C",
                  type="HIH",address=39,timerange="254,0,0",level="0,1,-,-")
        addsensor(board_slug=board_slug,name="Pressure",driver="I2C",
                  type="BMP",address=119,timerange="254,0,0",level="0,1,-,-")

    if (template == "stima_base"):
        print "setting template:", template
        delsensors(board_slug=board_slug)
        addsensor(board_slug=board_slug,name="Temperature",driver="I2C",
                  type="TMP",address=72,timerange="254,0,0",level="103,1000,-,-")
        addsensor(board_slug=board_slug,name="Humidity",driver="I2C",
                  type="HIH",address=39,timerange="254,0,0",level="103,1000,-,-")
        addsensor(board_slug=board_slug,name="Pressure",driver="I2C",
                  type="BMP",address=119,timerange="254,0,0",level="1,-,-,-")

    if (template == "stima_t_u"):
        print "setting template:", template
        delsensors(board_slug=board_slug)
        addsensor(board_slug=board_slug,name="Temperature",driver="I2C",
                  type="ADT",address=49,timerange="254,0,0",level="103,2000,-,-")
        addsensor(board_slug=board_slug,name="Humidity",driver="I2C",
                  type="HIH",address=39,timerange="254,0,0",level="103,2000,-,-")

    if (template == "stima_t"):
        print "setting template:", template
        delsensors(board_slug=board_slug)
        addsensor(board_slug=board_slug,name="Temperature",driver="I2C",
                  type="ADT",address=49,timerange="254,0,0",level="103,2000,-,-")



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

    print "sendjson2amqp"

    objects=[]

    mystation=StationMetadata.objects.get(slug=station,ident__username=user)
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
             exchange="rmap",
             board=None,
             activate=None):

    try:
        user = User.objects.create_user(username, username+'@rmap.cc', password)            
        #trap IntegrityError for user that already exist
    except IntegrityError:
        pass
    except:
        raise
        
    try:

        print "elaborate station: ",station

        mystation=StationMetadata.objects.filter(slug=station)[0]
        user=User.objects.get(username=username)
            
        mystation.ident=user
        mystation.lat=lat
        mystation.lon=lon
        if not (activate is None): mystation.active=activate
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

