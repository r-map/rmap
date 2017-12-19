1#!/usr/bin/env python
# Copyright (c) 2017 Paolo Patruno <p.patruno@iperbole.bologna.it>
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

__author__ = "Paolo Patruno"
__copyright__ = "Copyright (C) 2017 by Paolo Patruno"

import sys, os
sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

import paho.mqtt.client as paho
import os, sys
import logging
import time
import json
import signal
import base64
from rmap import rmapmqtt
from datetime import datetime
import threading
import traceback

LOGFORMAT = '%(asctime)-15s %(message)s'

DEBUG = 1
if DEBUG:
    logging.basicConfig(level=logging.DEBUG, format=LOGFORMAT)
else:
    logging.basicConfig(level=logging.INFO, format=LOGFORMAT)

client_id = "ttn2dballe_%d" % (os.getpid())

def bitextract(template,start,nbit):
    return (template>>start)&((1 << nbit) - 1)


class Threaded_ttn2dballe(threading.Thread):
    def __init__(self,mqtt_host,mqttuser, mqttpassword , topics, user, slug):
        threading.Thread.__init__(self)
        self.mqtt_host=mqtt_host
        self.mqttuser=mqttuser
        self.mqttpassword=mqttpassword
        self.topics=topics
        self.user=user
        self.slug=slug

    def run(self):

        try:
            myttn2dballe=ttn2dballe(self.mqtt_host,self.mqttuser, self.mqttpassword , self.topics, self.user, self.slug)
            myttn2dballe.run()
            
        except Exception as exception:
            # log and retry on exception 
            logging.error('Exception occured: ' + str(exception))
            logging.error(traceback.format_exc())
            logging.error('Subprocess failed')
            time.sleep(10)

        else:
            logging.info("Task done: thread terminated")

        
class ttn2dballe(object):

  def __init__(self,mqtt_host,mqttuser, mqttpassword , topics, user, slug):

    self.mqtt_host=mqtt_host
    self.mqttc = paho.Client(client_id, clean_session=True)
    self.map = {}

    for topic in topics:
        self.map[topic] = (user, slug)
    self.mqttc.username_pw_set(mqttuser,mqttpassword)

#    try:
#        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
#    except:
#        sys.stderr.write("Can't create UDP socket\n")
#        sys.exit(1)

    self.mqttc.on_message = self.on_message
    self.mqttc.on_connect = self.on_connect
    self.mqttc.on_disconnect = self.on_disconnect
    self.mqttc.on_publish = self.on_publish
    self.mqttc.on_subscribe = self.on_subscribe

    # set timezone to GMT
    os.environ['TZ'] = 'GMT'
    time.tzset()

#    self.mqttc.will_set("clients/" + client_id, payload="Adios!", qos=0, retain=False)


  def cleanup(self,signum, frame):
    '''Disconnect cleanly on SIGTERM or SIGINT'''

#    self.mqttc.publish("clients/" + client_id, "Offline")
    self.mqttc.disconnect()
    logging.info("Disconnected from broker; exiting on signal %d", signum)
    sys.exit(signum)


  def on_connect(self,mosq, userdata, flags, rc):
    logging.info("Connected to broker at %s as %s" % (self.mqtt_host, client_id))

#    self.mqttc.publish("clients/" + client_id, "Online")

    for topic in self.map:
        logging.debug("Subscribing to topic %s" % topic)
        self.mqttc.subscribe(topic, 0)

  def on_publish(self,mosq, userdata, mid):
    logging.debug("pubblish %s with id %s" % (userdata, mid))


  def on_message(self,mosq, userdata, msg):

#    sock = self.sock
#    host = self.carbon_server
#    port = self.carbon_port
    lines = []
    now = int(time.time())

    # Find out how to handle the topic in this message: slurp through
    # our map 
    for t in self.map:
        if paho.topic_matches_sub(t, msg.topic):
            # print "%s matches MAP(%s) => %s" % (msg.topic, t, self.map[t])

            # Must we rename the received msg topic into a different
            # name for Carbon? In any case, replace MQTT slashes (/)
            # by Carbon periods (.)
            (user, slug) = self.map[t]
            #if remap is None:
                #carbonkey = msg.topic.replace('/', '.').replace(',','_')
            #else:
                #carbonkey = remap.replace('/', '.')
            #logging.debug("CARBONKEY is [%s]" % carbonkey)

            # JSON: try and load the JSON string from payload 
            
            try:
                st = json.loads(msg.payload)
                payload=base64.b64decode(st["payload_raw"])
                #print payload
                template=int(payload.encode("hex"),16)
                start=0
                nbit=8
                numtemplate=bitextract(template,start,nbit)
                
                if numtemplate==1:
                    lon=10.0
                    lat=44.0
                    password="password"
                    mqtt=rmapmqtt.rmapmqtt(ident=user,password=password,lon=lon,lat=lat,network="sample",host="rmap.cc",prefix="test",maintprefix="test")
                    dt=datetime.utcnow().replace(microsecond=0)

                    start+=nbit
                    nbit=16
                    temp=bitextract(template,  start, nbit)
                    print temp
                    start+=nbit
                    nbit=7
                    humi=bitextract(template, start, nbit)
                    print humi
                    
                    temp=temp/100.+223.15
                    print temp
                    datavar={"B12101":{"t": dt,"v": temp}}
                    mqtt.data(timerange="254,0,0",level="103,2000,-,-",datavar=datavar)
                    
                    himi=humi/1.+0.
                    print humi
                    datavar={"B13003":{"t": dt,"v": humi}}
                    mqtt.data(timerange="254,0,0",level="103,2000,-,-",datavar=datavar)

                    mqtt.disconnect()
                    
                else:
                    logging.error("Unknown template %d " % numtemplate)

                    
                metadata=st["metadata"]
                print metadata
                ts=time.strptime(st.pop("time",time.strftime("%Y-%m-%dT%H:%M:%S",time.gmtime(now))),"%Y-%m-%dT%H:%M:%S")
                print ts
                #timestamp=int(time.mktime(ts))
                #print timestamp
            except:
                logging.error("Topic %s contains non-JSON payload [%s]" %
                             (msg.topic, msg.payload))
                raise
                #return

        else:
            logging.info("Unknown mapping key [%s]", type)
            return

        message = '\n'.join(lines) + '\n'
        logging.debug("%s", message)

        #            self.sock.sendto(message, (host, port))
  
  def on_subscribe(self,mosq, userdata, mid, granted_qos):
    logging.debug("Subscribed: "+str(mid)+" "+str(granted_qos))

  def on_disconnect(self,mosq, userdata, rc):

    if rc == 0:
        logging.info("Clean disconnection")
    else:
        logging.info("Unexpected disconnect (rc %s); reconnecting in 5 seconds" % rc)
        time.sleep(5)

  def run(self):
    logging.info("Starting %s" % client_id)
    logging.info("INFO MODE")
    logging.debug("DEBUG MODE")


    rc=paho.MQTT_ERR_CONN_REFUSED
    while ( not  (rc == paho.MQTT_ERR_SUCCESS)):
        try:
            rc=self.mqttc.connect(self.mqtt_host, 1883, 60)
        except:
            rc=paho.MQTT_ERR_CONN_REFUSED

        if (not (rc == paho.MQTT_ERR_SUCCESS)):
            logging.info("Cannot connect to MQTT; retry in 5 seconds")
            time.sleep(5)

    signal.signal(signal.SIGTERM, self.cleanup)
    signal.signal(signal.SIGINT, self.cleanup)

    self.mqttc.loop_forever()


if __name__ == '__main__':

    MQTT_HOST = os.environ.get('MQTT_HOST', 'eu.thethings.network')

    mapfile="ttnmap"
    f = open(mapfile)
    for line in f.readlines():
        line = line.rstrip()
        if len(line) == 0 or line[0] == '#':
            continue
        try:
            mqttuser, mqttpassword , topic, user, slug = line.split()
            m2g=ttn2dballe(MQTT_HOST,mqttuser, mqttpassword , (topic,), user, slug)
            m2g.run()

        except ValueError:
            logging.error("Malformed ttnmap line: %s" % line)


    
