#!/usr/bin/env python
# Copyright (c) 2019 Paolo Patruno <p.patruno@iperbole.bologna.it>
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
__copyright__ = "Copyright (C) 2019 by Paolo Patruno"


import paho.mqtt.client as paho
import os, sys
import logging
import time
import json
import signal
from rmap import rmapmqtt
import traceback
from  rmap.ttntemplate import ttntemplate
from  rmap.dtable import dtable

# Encoder per la data
class JSONEncoder(json.JSONEncoder):
    def default(self, o): 
        if isinstance(o, datetime):
            return o.strftime("%Y-%m-%dT%H:%M:%S")
        else:
            return json.JSONEncoder.default(o)
# Funzione per fare il dump in JSON
def dumps(o):
    return json.dumps(o, cls=JSONEncoder)

class report2observation(object):

  def __init__(self,mqtt_host,mqttuser, mqttpassword, subtopics, topicmaint, terminate):

    self.mqtt_host=mqtt_host
    self.subtopics=subtopics
    self.client_id = "report2observation"
    self.mqttc = paho.Client(self.client_id, clean_session=False)   # with qos 1
    self.terminateevent=terminate
    self.mqttuser=mqttuser
    self.mqttpassword=mqttpassword
    self.mqtttopicmaint=topicmaint
    
    self.mqttc.username_pw_set(self.mqttuser,self.mqttpassword)
    self.mqttc.max_queued_messages_set(0)

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
    sys.exit(0)


  def on_connect(self,mosq, userdata, flags, rc):
    logging.info("Connected to broker at %s as %s" % (self.mqtt_host, self.client_id))

    for topic in self.subtopics:
        logging.debug("Subscribing to topic %s" % topic)
        self.mqttc.subscribe(topic, 1)   # qos 1

  def on_publish(self,mosq, userdata, mid):
    logging.debug("pubblished %s with id %s" % (userdata, mid))


  def publish(self,topic,payload,retain=False,timeout=15.):

      logging.info("publish %s : %s" % (topic,payload)) 
      messageinfo=self.mqttc.publish(topic,payload=payload,qos=1,retain=retain)
      if messageinfo.rc != paho.MQTT_ERR_SUCCESS:
              logging.error("publish rc %d" % messageinfo.rc) 
      return messageinfo.rc

      #if messageinfo.rc != mqtt.MQTT_ERR_SUCCESS:
      #  return messageinfo.rc
      #messageinfo.wait_for_publish(timeout)
      #if (messageinfo.is_published()):
      #  return mqtt.MQTT_ERR_SUCCESS
      #else:
      #  return 1
    

  def on_message(self,mosq, userdata, msg):

    # JSON: try and load the JSON string from payload             
    try:

      # "report/digiteco/1208611,4389056/fixed/0,0,900/103,2000,-,-/B12101 {"v":null,"t":"2019-03-16T08:15:00"}"
      # "report/+/+/+/+/+"

      logging.info("message: %s  %s" % (msg.topic,msg.payload))
                   
      topics=msg.topic.split("/")

      if (topics[0] =="1"):
        version=1
        prefix= topics[1]
        user = topics[2]
        ident = topics[3]
        lonlat=topics[4]
        network=topics[5]
        nexttopic=6
      else:
        version=0    
        prefix= topics[0]
        user = topics[1]
        ident = topics[1]
        lonlat=topics[2]
        network=topics[3]
        nexttopic=4

      st = json.loads(msg.payload.decode())
      dt=st.get("t")

      try:
        logging.debug("try to decode with table d")

        d=st["d"]
        timerange=topics[nexttopic]
        nexttopic += 1
        level=topics[nexttopic]
        bcodes=dtable[str(d)]
        timeranges=[]
        levels=[]
        for bcode in bcodes:
          timeranges.append(timerange)
          levels.append(level)
          
        
      except:
        try:
          logging.debug("Error; try to decode with table e")
          e=st["e"]
          numtemplate=int(e)
          #if numtemplate > 0 and numtemplate < len(ttntemplate):
          mytemplate=ttntemplate[numtemplate]
          bcodes=[]
          timeranges=[]
          levels=[]
          for bcode,param in list(mytemplate.items()):
            bcodes.append(bcode)
            timeranges.append(param["timerange"])
            levels.append(param["level"])
        except:
          logging.error("skip message: %s : %s"% (msg.topic,msg.payload))
          return

      logging.info("user={} ident={} username={} password={} lonlat={} network={} host={} prefix={} maintprefix=maint".format(user,ident,self.mqttuser,"fakepassword",lonlat,network,self.mqtt_host,prefix))
      #mqtt=rmapmqtt.rmapmqtt(user=user,ident=ident,username=self.mqttuser,password=self.mqttpassword,lonlat=lonlat,network=network,host=self.mqtt_host,prefix=prefix,maintprefix=self.mqtttopicmaint,logfunc=logging.info,qos=0,version=version)  # attention qos 0 for fast publish

      try:
        attributearray=st.get("a",{})
        dindex=0
        for val in st["p"]:
          if ( val is not None ):
            bcode=bcodes[dindex]
            timerange=timeranges[dindex]
            level=levels[dindex]
            attributes={}
            for abcode in attributearray.keys():
              attributes[abcode]=attributearray[abcode][dindex]

            datavar={bcode:{"t": dt,"v": val,"a":attributes}}
            
            logging.info("timerange={} level={} bcode={} val={} attr{}".format(timerange,level,bcode,val,attributes))
            #mqtt.data(timerange=timerange,level=level,datavar=datavar)

            for key,val in datavar.items():
              if (version == 0) :
                rc=self.publish(prefix+"/"+user+"/"+lonlat+"/"+network+"/"+
                                timerange+"/"+level+"/"+key,
                                payload=dumps(val)
                                )
              else:
                rc=self.publish("1/"+prefix+"/"+user+"/"+ident+"/"+lonlat+"/"+network+"/"+
                                timerange+"/"+level+"/"+key,
                                payload=dumps(val) 
                                )


              if rc != paho.MQTT_ERR_SUCCESS:
                raise Exception("publish data",rc)
            
          dindex+=1
      except:
        raise
            
          
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
    logging.debug("Subscribed: "+str(mid)+" "+str(granted_qos))

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

    logging.info('Starting up report2observation')

  
    MQTT_HOST = os.environ.get('MQTT_HOST', 'localhost')
    subtopics=["test/+/+/+","test/+/+/+/+/+"]
    mqttuser="user"
    mqttpassword="password"    
    
    r2o=report2observation(MQTT_HOST,mqttuser, mqttpassword ,subtopics)
    r2o.run()


    
