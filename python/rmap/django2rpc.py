#!/usr/bin/env python
# Copyright (c) 2023 Paolo Patruno <p.patruno@iperbole.bologna.it>
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
__copyright__ = "Copyright (C) 2023 by Paolo Patruno"


import paho.mqtt.client as paho
import os, sys
import logging
import datetime,time
import json
import signal
import traceback
from  rmap import rmap_core
from rpc.models import Rpc

#NOT USED!
class django2rpc_cmd(object):

  def __init__(self,mqtt_host,mqttuser, mqttpassword, topicrpc, terminate):

    self.mqtt_host=mqtt_host
    self.client_id = "django2rpc_cmd_%d" % (os.getpid())
    self.mqttc = paho.Client(self.client_id, clean_session=True)
    self.terminateevent=terminate
    self.mqttuser=mqttuser
    self.mqttpassword=mqttpassword
    self.mqtttopicrpc=topicrpc
    
    self.mqttc.username_pw_set(self.mqttuser,self.mqttpassword)

    self.mqttc.on_connect = self.on_connect
    self.mqttc.on_disconnect = self.on_disconnect
    self.mqttc.on_publish = self.on_publish

    # set timezone to GMT
    os.environ['TZ'] = 'GMT'
    time.tzset()


  def cleanup(self,signum, frame):
    '''Disconnect cleanly on SIGTERM or SIGINT'''

#    self.mqttc.publish("clients/" + self.client_id, "Offline")
    self.mqttc.disconnect()
    logging.info("Disconnected from broker; exiting on signal %d", signum)
    sys.exit(signum)

  def terminate(self):
    '''Disconnect cleanly on terminate event'''

#    self.mqttc.publish("clients/" + self.client_id, "Offline")
    self.mqttc.disconnect()
    logging.info("Disconnected from broker; exiting on terminate event")
    sys.exit(0)


  def on_connect(self,mosq, userdata, flags, rc):
    logging.info("Connected to broker at %s as %s" % (self.mqtt_host, self.client_id))

  def on_publish(self,mosq, userdata, mid):
    logging.debug("pubblish %s with id %s" % (userdata, mid))

  def on_disconnect(self,mosq, userdata, rc):

    if rc == 0:
        logging.info("Clean disconnection")
    else:
        logging.info("Unexpected disconnect (rc %s); reconnecting in 5 seconds" % rc)
        time.sleep(5)
        
  def run(self):
    logging.info("Starting %s" % self.client_id)
    logging.info("INFO MODE")
    logging.debug("DEBUG MODE")


    rc=self.mqttc.connect_async(self.mqtt_host, 1883, 60)
        
    self.mqttc.loop_start()

    while not self.terminateevent.kill_now:
        time.sleep(5)

    self.terminate()
    self.mqttc.loop_stop()



class django2rpc_res(object):

  def __init__(self,mqtt_host,mqttuser, mqttpassword, subtopics, topicmaint, terminate):

    self.mqtt_host=mqtt_host
    self.subtopics=subtopics
    self.client_id = "django2rpc_res_%d" % (os.getpid())
    self.mqttc = paho.Client(self.client_id, clean_session=False)
    self.terminateevent=terminate
    self.mqttuser=mqttuser
    self.mqttpassword=mqttpassword
    self.mqtttopicmaint=topicmaint
    
    self.mqttc.username_pw_set(self.mqttuser,self.mqttpassword)

    self.mqttc.on_message = self.on_message
    self.mqttc.on_connect = self.on_connect
    self.mqttc.on_disconnect = self.on_disconnect
    self.mqttc.on_publish = self.on_publish
    self.mqttc.on_subscribe = self.on_subscribe

    # set timezone to GMT
    os.environ['TZ'] = 'GMT'
    time.tzset()


  def cleanup(self,signum, frame):
    '''Disconnect cleanly on SIGTERM or SIGINT'''

#    self.mqttc.publish("clients/" + self.client_id, "Offline")
    self.mqttc.disconnect()
    logging.info("Disconnected from broker; exiting on signal %d", signum)
    sys.exit(signum)

  def terminate(self):
    '''Disconnect cleanly on terminate event'''

#    self.mqttc.publish("clients/" + self.client_id, "Offline")
    self.mqttc.disconnect()
    logging.info("Disconnected from broker; exiting on terminate event")
    #sys.exit(0)


  def on_connect(self,mosq, userdata, flags, rc):
    logging.info("Connected to broker at %s as %s" % (self.mqtt_host, self.client_id))

    for topic in self.subtopics:
        logging.info("Subscribing to topic %s" % topic)
        self.mqttc.subscribe(topic, 0)

  def on_publish(self,mosq, userdata, mid):
    logging.debug("pubblish %s with id %s" % (userdata, mid))


  def on_message(self,mosq, userdata, msg):

    # JSON: try and load the JSON string from payload             
    try:

      topics=msg.topic.split("/")
      version=topics[0]
      prefix= topics[1]
      user = topics[2]
      ident = topics[3]
      if ident=="":
        lon,lat=topics[4].split(",")
        lon=float(lon)/100000.
        lat=float(lat)/100000.
      else:
        lon=None
        lat=None
      network=topics[5]
        
      logging.info("user={} ident={} lon={} lat={} network={} prefix={}".format(user,ident,lon,lat,network,prefix))

      payload = json.loads(msg.payload.decode())
      if payload["jsonrpc"] == "2.0":

        try:
          logging.info("try to save response in django RPC id:{}".format(payload["id"]))
          myrpc = Rpc.objects.get(active=False,id=payload["id"]
                                  ,stationmetadata__lon=lon,stationmetadata__lat=lat
                                  ,stationmetadata__user__username=user
                                  ,stationmetadata__ident=ident
                                  ,stationmetadata__network=network)
          myrpc.dateres=datetime.datetime.now()
          myrpc.result=payload.get("result")
          myrpc.error=payload.get("error")
          myrpc.save()
          
        except:
          logging.error("error skip message: %s : %s"% (msg.topic,msg.payload))
          return
      else:          
        logging.error("malformed response skip message: %s : %s"% (msg.topic,msg.payload))
        return

    except Exception as exception:
      logging.error("Topic %s error decoding or publishing; payload: [%s]" %
                    (msg.topic, msg.payload))
      logging.error('Exception occured: ' + str(exception))
      logging.error(traceback.format_exc())

      # if some exception occour here, ask to terminate
      #self.terminateevent.exit_gracefully()

    finally:
      return

    
  def on_subscribe(self,mosq, userdata, mid, granted_qos):
    logging.info("Subscribed: "+str(mid)+" "+str(granted_qos))

  def on_disconnect(self,mosq, userdata, rc):

    if rc == 0:
        logging.info("Clean disconnection")
    else:
        logging.info("Unexpected disconnect (rc %s); reconnecting in 5 seconds" % rc)
        time.sleep(5)


        
  def run(self):
    logging.info("Starting %s" % self.client_id)
    logging.info("INFO MODE")
    logging.debug("DEBUG MODE")

    rc=self.mqttc.connect_async(self.mqtt_host, 1883, 60)        
    self.mqttc.loop_start()


  def stop(self):
    #while not self.terminateevent.kill_now:
    #    time.sleep(5)

    self.terminate()
    self.mqttc.loop_stop()
        
if __name__ == '__main__':

    import logging,logging.handlers

    #formatter=logging.Formatter("%(asctime)s%(thread)d-%(levelname)s- %(message)s",datefmt="%Y-%m-%d %H:%M:%S")
    #handler = logging.handlers.RotatingFileHandler("report2observation.log", maxBytes=5000000, backupCount=10)
    #handler.setFormatter(formatter)
    
    # Add the log message handler to the root logger
    #logging.getLogger().addHandler(handler)
    #logging.getLogger().setLevel(logging.DEBUG)
    logging.getLogger("report2observation")
    logging.basicConfig(level=logging.DEBUG)

    logging.info('Starting up django2rpc')

  
    MQTT_HOST = os.environ.get('MQTT_HOST', 'localhost')
    subtopics=["test/1/+/+/+","test/+/+/+/+/+"]
    mqttuser="user"
    mqttpassword="password"    
    
    d2rpc=django2rpc(MQTT_HOST,mqttuser, mqttpassword ,subtopics)
    d2rpc.run()


    
