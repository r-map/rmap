#!/usr/bin/env python

__author__ = "Jan-Piet Mens"
__copyright__ = "Copyright (C) 2013 by Jan-Piet Mens"

import paho.mqtt.client as paho
import os, sys
import logging
import time
import socket
import json
import signal

LOGFORMAT = '%(asctime)-15s %(message)s'

DEBUG = 0
if DEBUG:
    logging.basicConfig(level=logging.DEBUG, format=LOGFORMAT)
else:
    logging.basicConfig(level=logging.INFO, format=LOGFORMAT)

client_id = "MQTT2Graphite_%d-%s" % (os.getpid(), socket.getfqdn())


def is_number(s):
    '''Test whether string contains a number (leading/traling white-space is ok)'''

    try:
        float(s)
        return True
    except ValueError:
        return False
    except TypeError:
	return False

class mqtt2graphite():

  def __init__(self,mqtt_host,carbon_server,carbon_port,mapfile):

    self.mqtt_host=mqtt_host
    self.carbon_server=carbon_server
    self.carbon_port=carbon_port
    self.mqttc = paho.Client(client_id, clean_session=True)
    self.map = {}

    f = open(mapfile)
    for line in f.readlines():
        line = line.rstrip()
        if len(line) == 0 or line[0] == '#':
            continue
        remap = None
        try:
            type, topic, remap = line.split()
        except ValueError:
            type, topic = line.split()

        self.map[topic] = (type, remap)

    try:
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    except:
        sys.stderr.write("Can't create UDP socket\n")
        sys.exit(1)

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

    sock = self.sock
    host = self.carbon_server
    port = self.carbon_port
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
            (type, remap) = self.map[t]
            if remap is None:
                carbonkey = msg.topic.replace('/', '.').replace(',','_')
            else:
                carbonkey = remap.replace('/', '.')
            logging.debug("CARBONKEY is [%s]" % carbonkey)

            if type == 'n':
                '''Number: obtain a float from the payload'''
                try:
                    number = float(msg.payload)
                    lines.append("%s %f %d" % (carbonkey, number, now))
                except ValueError:
                    logging.info("Topic %s contains non-numeric payload [%s]" % 
                            (msg.topic, msg.payload))
                    return

            elif type == 'j':
                '''JSON: try and load the JSON string from payload and use
                   subkeys to pass to Carbon'''
                try:
                    st = json.loads(msg.payload)
                    # get timestamp fron json or set it to now
                    # todo : optional seconds and milliseconds
                    try:
                        ts=time.strptime(st.pop("t",time.strftime("%Y-%m-%dT%H:%M:%S",time.gmtime(now))),"%Y-%m-%dT%H:%M:%S")
                        timestamp=int(time.mktime(ts))
                    except:
                        #timestamp=now
                        raise

                    for k in st:
                        if is_number(st[k]):
                            lines.append("%s.%s %f %d" % (carbonkey, k, float(st[k]), timestamp))

                except:
                    logging.info("Topic %s contains non-JSON payload [%s]" %
                            (msg.topic, msg.payload))
                    return

            else:
                logging.info("Unknown mapping key [%s]", type)
                return

            message = '\n'.join(lines) + '\n'
            logging.debug("%s", message)

            self.sock.sendto(message, (host, port))
  
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

    MQTT_HOST = os.environ.get('MQTT_HOST', 'rmap.cc')
    CARBON_SERVER = '127.0.0.1'
    CARBON_PORT = 2003

    m2g=mqtt2graphite(MQTT_HOST, CARBON_SERVER, CARBON_PORT,"map")
    m2g.run()
