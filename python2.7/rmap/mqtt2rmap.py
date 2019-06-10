#!/usr/bin/env python

__author__ = "Paolo Patruno"
__copyright__ = "Copyright (C) 2015 by Paolo Patruno"

import paho.mqtt.client as paho
import os, sys
import logging
import time
import socket
import json
import signal

LOGFORMAT = '%(asctime)-15s %(message)s'

DEBUG = 1
if DEBUG:
    logging.basicConfig(level=logging.DEBUG, format=LOGFORMAT)
else:
    logging.basicConfig(level=logging.INFO, format=LOGFORMAT)

client_id = "MQTT2rmap_%d-%s" % (os.getpid(), socket.getfqdn())


def is_number(s):
    '''Test whether string contains a number (leading/traling white-space is ok)'''

    try:
        float(s)
        return True
    except ValueError:
        return False


class mqtt2rmap():

  def __init__(self,mqtt_host,prefix="rmap",mqtt_username=None,mqtt_password=None):

    self.mqtt_host=mqtt_host
    self.mqtt_username=mqtt_username
    self.mqtt_password=mqtt_password

    self.prefix=prefix
    self.mqttc = paho.Client(client_id, clean_session=True)
    self.mqttc.on_message = self.on_message
    self.mqttc.on_connect = self.on_connect
    self.mqttc.on_disconnect = self.on_disconnect
    self.mqttc.on_publish = self.on_publish
    self.mqttc.on_subscribe = self.on_subscribe
    self.start = int(time.time())


    # set timezone to GMT
    os.environ['TZ'] = 'GMT'
    time.tzset()

  def cleanup(self,signum, frame):
    '''Disconnect cleanly on SIGTERM or SIGINT'''

    self.mqttc.disconnect()
    logging.info("Disconnected from broker; exiting on signal %d", signum)
    sys.exit(signum)


  def on_connect(self,mosq, userdata, flags, rc):
    logging.info("Connected to broker at %s as %s" % (self.mqtt_host, client_id))

    topic = self.prefix+"/+/+/+/-,-,-/-,-,-,-/B01213"
    logging.debug("Subscribing to topic %s" % topic)
    self.mqttc.subscribe(topic, 0)

  def on_publish(self,mosq, userdata, mid):
    logging.debug("pubblished %s with id %s" % (userdata, mid))


  def on_message(self,mosq, userdata, msg):

    # this remove all retained messages (but produce an infinite loop)
    #logging.debug("Remove Topic [%s]" % (msg.topic,))
    #mosq.publish(msg.topic, payload=None, qos=1, retain=True)

    now = int(time.time())
    logging.debug("deltatime [%d]" % (now-self.start,))
    
    logging.debug("Topic [%s]  payload [%s]" %
                 (msg.topic, msg.payload))
    if msg.payload =="": return

    prefix,user,lonlat,network,void=msg.topic.split("/",4)    
    lon,lat = lonlat.split(",")
    lon = float(int(lon))/100000.
    lat = float(int(lat))/100000.

    try:
        st = json.loads(msg.payload)
        message=st["v"]
    except:
        logging.info("Topic %s contains non-JSON payload [%s]" %
                     (msg.topic, msg.payload))
        mosq.publish(msg.topic, payload=None, qos=0, retain=True)
        return

    logging.info("log message [%s %s %f %f %s] -> %s" %
                 (prefix,user,lon,lat,network,message))

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


    self.mqttc.username_pw_set(self.mqtt_username,self.mqtt_password)


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

    MQTT_HOST = os.environ.get('MQTT_HOST', 'rmap.cc')
    MQTT_USERNAME = os.environ.get('MQTT_USERNAME', 'rmap')
    MQTT_PASSWORD = os.environ.get('MQTT_PASSWORD', 'rmap')

    m2g=mqtt2rmap(MQTT_HOST,prefix="rmap",mqtt_username=MQTT_USERNAME,mqtt_password=MQTT_PASSWORD)
    m2g.run()
