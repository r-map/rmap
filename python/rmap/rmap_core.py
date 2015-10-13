
from django.db import IntegrityError
from django.core.exceptions import ObjectDoesNotExist
from stations.models import StationMetadata
from django.contrib.auth.models import User
from django.core import serializers
import pika

def send2amqp(body="",user="your user",password="your password",host="rmap.cc",exchange="exc",routing_key="routing"):

    credentials=pika.PlainCredentials(user, password)
    properties=pika.BasicProperties(
        #    user_id= user,
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


def sendjson2amqp(station,user="your user",password="your password",host="rmap.cc",exchange="exc"):

    mystation=StationMetadata.objects.get(slug=station)
    body= export2json([mystation])
    body+= export2json(mystation.board_set.all())


    for board in mystation.board_set.all():
        body+= export2json(board.sensor_set.all())


    send2amqp(body,user,password,host,exchange,routing_key="configuration")



def configdb(username="your user",password="your password",
             station="home",lat=0,lon=0,constantdata=(),
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
        for btable,value in constantdata:
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
                print "bluetooth Transport", board.transportblutooth

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

