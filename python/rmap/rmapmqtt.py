#!/usr/bin/python

# Copyright (c) 2013 Paolo Patruno <p.patruno@iperbole.bologna.it>
#                    Emanuele Di Giacomo <edigiacomo@arpa.emr.it>
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

# N.B. 
# Bisognera provvedere a una ripulitura delle stazioni
# (messaggi retained) dopo un certo lasso di tempo 

import os
import traceback
try:
    from plyer.compat import PY2
    from plyer import notification
except:
    print "plyer not available"

#import threading # https://github.com/kivy/kivy/wiki/Working-with-Python-threads-inside-a-Kivy-application

import settings
import json
from datetime import datetime, timedelta
import time
import codecs
#import mosquitto
import paho.mqtt.client as mqtt
from utils import nint,log_stdout

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

class Rmapdonotexist(Exception):
    pass



class rmapmqtt:

    def __init__(self,ident="-",lon=None,lat=None,network="generic",host="localhost",port=1883,username=None,password=None,timeout=60,logfunc=log_stdout,clientid="",prefix="test",maintprefix="test"):

        self.ident=ident
        self.lonlat="%d,%d" % (nint(lon*100000),nint(lat*100000))
        self.network=network
        self.host=host
        self.port=port
        self.username=username
        self.password=password
        self.timeout=timeout
        self.log    = logfunc
        self.prefix=prefix
        self.maintprefix=maintprefix
        self.connected=False
        self.mid=-1
        self.loop_started=False

        # If you want to use a specific client id, use
        # mqttc = mosquitto.Mosquitto("client-id")
        # but note that the client id must be unique on the broker. Leaving the client
        # id parameter empty will generate a random id for you.
        #self.mqttc = mosquitto.Mosquitto(clientid)
        self.mqttc = mqtt.Client(clientid)

        self.mqttc.on_message = self.on_message
        self.mqttc.on_connect = self.on_connect
        self.mqttc.on_disconnect = self.on_disconnect
        self.mqttc.on_publish = self.on_publish
        self.mqttc.on_subscribe = self.on_subscribe
        # Uncomment to enable debug messages
        self.mqttc.on_log = self.on_log

        # Imposto le credenziali
        if (not self.username is None):
            self.mqttc.username_pw_set(self.username,self.password)

        #self.mqttc.max_inflight_messages_set(1)

        # retained only if the station is fixed
        retain = self.maintprefix != "mobile"

        # mando stato di connessione della stazione con segnalazione di sconnessione gestita male com will
        self.mqttc.will_set(self.maintprefix+"/"+self.ident+"/"+self.lonlat+"/"+self.network+"/-,-,-/-,-,-,-/B01213",
                    payload=dumps({"v": "error01"}),
                            qos=1, retain=retain)

        try:
            print "start connect"
            #self.mqttc.connect_async(self.host,self.port,self.timeout)
            rc=self.mqttc.connect(self.host,self.port,self.timeout)
            print "end connect"
            if rc != mqtt.MQTT_ERR_SUCCESS:
                raise Exception("connect",rc)

            # retained only if the station is fixed
            retain = self.maintprefix != "mobile"

            rc=self.mqttc.publish(self.maintprefix+"/"+self.ident+"/"+self.lonlat+"/"+self.network+"/-,-,-/-,-,-,-/B01213",
                             payload=dumps({ "v": "conn"}),
                                  qos=1,retain=retain)

            if rc[0] != mqtt.MQTT_ERR_SUCCESS:
                raise Exception("publish status",rc)

            self.log("publish maint message mid: "+str(rc[1]))

        except Exception as inst:
            self.error(inst)

    def publish(self,topic,payload,qos=0,retain=False,timeout=3.):
        ''' 
        bloking publish
        with qos > 0 we wait for ack
        '''
        self.puback=False
        rc,self.mid=self.mqttc.publish(topic,payload=payload,qos=qos,retain=retain)
        if rc != mqtt.MQTT_ERR_SUCCESS:
            return rc
        if (qos == 0 ):
            return rc

        self.log("publish ana message mid: "+str(self.mid))

        last=time.time()
        while ((time.time()-last) < timeout and not self.puback):
            time.sleep(.1)
            if (not self.loop_started):
                self.loop()

        if (not self.puback):
            return 50
        else:
            return  mqtt.MQTT_ERR_SUCCESS

    def ana(self,anavar={},lon=None,lat=None):

        try:
            if lon is not None and lat is not None:
                lonlat="%d,%d" % (nint(lon*100000),nint(lat*100000))
            else:
                lonlat=self.lonlat

            # mando dati di anagrafica

            # retained only if the station is fixed
            retain = self.prefix != "mobile"

            for key,val in anavar.iteritems():
                rc=self.publish(self.prefix+"/"+self.ident+"/"+lonlat+"/"+self.network+"/-,-,-/-,-,-,-/"+key,
                                      payload=dumps(val),
                                      qos=1,retain=retain)
                if rc != mqtt.MQTT_ERR_SUCCESS:
                    raise Exception("publish ana",rc)

        except Exception as inst:
            self.error(inst)


    def data(self,timerange=None,level=None,datavar={},lon=None,lat=None,prefix=None):

        try:

            # send data (temperature  for example) non retained

            if lon is not None and lat is not None:
                lonlat="%d,%d" % (nint(lon*100000),nint(lat*100000))
            else:
                lonlat=self.lonlat

            if prefix is None:
                prefix=self.prefix
                
            for key,val in datavar.iteritems():
                rc=self.publish(prefix+"/"+self.ident+"/"+lonlat+"/"+self.network+"/"+
                                      timerange+"/"+level+"/"+key,
                                      payload=dumps(val), 
                                      qos=1,
                                      retain=False
                                  )
            
                if rc != mqtt.MQTT_ERR_SUCCESS:
                    raise Exception("publish data",rc)

            #rc = self.mqttc.loop()
            #if rc != mqtt.MQTT_ERR_SUCCESS:
            #    raise Exception("loop",rc)

        except Exception as inst:
            self.error(inst)

    def loop(self,timeout=1):
        """
        Use this as alternative to loop_start and loop_stop.
        You have to call loop to mantain mqtt protocol.
        This function is the driving force behind the mqtt client. If they
        are not called, incoming network data will not be processed
        and outgoing network data may not be sent in a timely fashion.
        """

        try:
            rc = self.mqttc.loop(timeout)
            #if rc != mqtt.MQTT_ERR_SUCCESS:
                #raise Exception("loop",rc)
        
        except Exception as inst:
            self.error(inst)



    def loop_start(self):
        """
        this start a thead to mantain mqtt protocol
        """
        try:
            rc = self.mqttc.loop_start()
            self.loop_started=True
        except Exception as inst:
            self.loop_started=False
            self.error(inst)

    def loop_stop(self):

        try:
            rc = self.mqttc.loop_stop(force=True)
            self.loop_started=False
        except Exception as inst:
            self.loop_started=False
            self.error(inst)        

    def disconnect(self):

        try:

            #clean disconnect
            # retained only if the station is fixed
            retain = self.maintprefix != "mobile"

            rc=self.mqttc.publish(self.maintprefix+"/"+self.ident+"/"+self.lonlat+"/"+self.network+"/-,-,-/-,-,-,-/B01213",
                             payload=dumps({ "v": "disconn"}),
                                  qos=1,retain=retain)
            if rc[0] != mqtt.MQTT_ERR_SUCCESS:
                raise Exception("publish status",rc)

            self.log("publish maint message mid: "+str(rc[1]))

            #rc = self.mqttc.loop()
            #if rc != mqtt.MQTT_ERR_SUCCESS:
            #    raise Exception("loop",rc)

            rc = self.mqttc.disconnect()
            if rc != mqtt.MQTT_ERR_SUCCESS:
                raise Exception("disconnect",rc)

        except Exception as inst:
            self.error(inst)


    def on_connect(self,mosq, userdata,flags, rc):
        self.log("connect rc: "+str(rc))
        #self.log(mqtt.error_string(rc))

        if not self.connected:
            try:
                # retained only if the station is fixed
                retain = self.maintprefix != "mobile"

                rc=self.mqttc.publish(self.maintprefix+"/"+self.ident+"/"+self.lonlat+"/"+self.network+"/-,-,-/-,-,-,-/B01213",
                             payload=dumps({ "v": "conn"}),
                                      qos=1,retain=retain)

                if rc[0] != mqtt.MQTT_ERR_SUCCESS:
                    raise Exception("publish status",rc)

                self.log("publish maint message mid: "+str(rc[1]))

            except Exception as inst:
                self.error(inst)

        print "--------------------------------> connected"
        self.connected=True


    def on_disconnect(self,mosq, userdata, rc):
        self.log("disconnect rc: "+str(rc))

        print "--------------------------------> disconnected"
        self.connected=False

        #if rc == 1 :
        #    try:
        #        rc=self.mqttc.connect(self.host,self.port,self.timeout)
        #        if rc != mqtt.MQTT_ERR_SUCCESS:
        #            raise Exception("connect",rc)

        #        # stato della connessione
        #        self.connected=True
        #        rc=self.mqttc.publish(self.maintprefix+"/"+self.ident+"/"+self.lonlat+"/"+self.network+"/-,-,-/-,-,-,-/B01213",
        #                              payload=dumps({ "v": "conn"}),
        #                              qos=1,retain=True)

        #        if rc[0] != mqtt.MQTT_ERR_SUCCESS:
        #            raise Exception("publish status",rc)

        #    except Exception as inst:
        #        self.error(inst)


    def on_message(self,mosq, userdata, msg):
        self.log("message: "+msg.topic+" "+str(msg.qos)+" "+str(msg.payload))

    def on_publish(self,mosq, userdata, mid):
        self.log("published mid: "+str(mid))
        if (mid == self.mid):
            self.puback=True

    def on_subscribe(self,mosq, userdata, mid, granted_qos):
        self.log("Subscribed: "+str(mid)+" "+str(granted_qos))

    def on_log(self,mosq, userdata, level, string):
        self.log("log: "+string)


    def error(self,inst=None):

        self.log("Error !!!!!!!!!")
        self.log(str(inst))
        raise


def do_notify(message="",title="Notification"):

    """
    Notify a message with title and message
    """

    if PY2:
        title = title.decode('utf8')
        message = message.decode('utf8')
    kwargs = {'title': title, 'message': message}

    try:
        notification.notify(**kwargs)
    except exception as e:
        print e
        print "error on notify message:",title, message
        traceback.print_exc()

def main():

    ident="test"
    lon=10.0
    lat=44.0
    value=100
    dt=datetime.utcnow().replace(microsecond=0)

    datavar={"B20003":{"t": dt,"v": value}}

    mqtt=rmapmqtt(ident=ident,lon=lon,lat=lat,network="rmap",host="rmap.cc",port=1883,prefix="test",maintprefix="test")
    mqtt.data(timerange="254,0,0",level="1,-,-,-",datavar=datavar)
    mqtt.disconnect()


if __name__ == '__main__':
    main()  # (this code was run as script)
