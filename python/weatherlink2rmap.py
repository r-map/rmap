WTLUSER=""
WTLTOKEN=""
RMAPUSER=""
RMAPPASSWORD=""

host="rmap.cc"

import requests
import os, time, sys
import datetime
import email.utils
import signal

from django.conf import settings
from django.utils import translation
from django.core import management
from rmap.rmapmqtt import rmapmqtt



def weatherlink_status():
    r = requests.get("https://api.weatherlink.com/v1/StationStatus.json?user="+WTLUSER+"&pass=giugiu2012&apiToken="+WTLTOKEN)
    data=r.json()
    return data

def weatherlink_get():
    r = requests.get("https://api.weatherlink.com/v1/NoaaExt.json?user="+WTLUSER+"&pass=giugiu2012&apiToken="+WTLTOKEN)
    data=r.json()
    
    print (data["location"])
    print (data["latitude"])
    print (data["longitude"])
    print (data["observation_time_rfc822"])
    print (data["temp_c"])
    print (data["relative_humidity"])
    print (data["pressure_mb"])
    print (data["wind_kt"])
    print (data["wind_degrees"])
    return data

def cleanup(signum, frame):
    #rmap.disconnect()
    #rmap.loop_stop()
    print ("interrupted")
    sys.exit(0)

def main():

    signal.signal(signal.SIGTERM, cleanup)
    signal.signal(signal.SIGINT, cleanup)

    
    os.environ['DJANGO_SETTINGS_MODULE'] = 'rmap.settings'
    import django
    django.setup()

    django.utils.translation.activate("it")

    data=weatherlink_status()

    lon=float(data["station_longitude"])
    lat=float(data["station_latitude"])


    rmap=rmapmqtt(ident=RMAPUSER,lon=lon,lat=lat,host=host,network="fixed",username=RMAPUSER,password=RMAPPASSWORD,prefix="sample",maintprefix="maint")
    rmap.loop_start()
    rmap.connect()

    anavar={
        "B01019":{"v": data["user_city"]},
#        data["user_state"]
#        data["user_country"]
        "B07030":{"v": float(data["station_elevation_m"])}
    }
    
    rmap.ana(anavar)
    time.sleep(5)
    #rmap.loop()
    
    while True:
            
        try:
            data=weatherlink_get()
            reptime=datetime.datetime.utcfromtimestamp(email.utils.mktime_tz(email.utils.parsedate_tz(data["observation_time_rfc822"])))
            print("connect status: ",rmap.connected)
            timerange="254,0,0"               # dati istantanei
            level="103,2000,-,-"              # 2m dal suolo
            datavar={"B12101":
                     {
                         "t": reptime,
                             "v": float(data["temp_c"])+273.15
                     },
                     "B13003":
                     {
                         "t": reptime,
                         "v": float(data["relative_humidity"])
                     }
            }
                
            rmap.data(timerange,level,datavar)
            
            level="1,0,-,-"              # surface
            datavar={"B07004":
                     {
                         "t": reptime,
                         "v": float(data["pressure_mb"])*100.
                     }
            }
                
            rmap.data(timerange,level,datavar)
            
            level="103,10000,-,-"              # 2m dal suolo
            datavar={"B11002":
                     {
                         "t": reptime,
                         "v": float(data["wind_kt"])/1.94
                     },
                     "B11001":
                     {
                         "t": reptime,
                         "v": float(data["wind_degrees"])
                     }
            }
            rmap.data(timerange,level,datavar)
            
            time.sleep(20)
            #rmap.loop()
                
        except:

            while True:

                try:
                    print("terminated with error")
                    rmap.disconnect()
                    rmap.loop_stop()
                    #raise
                    
                    rmap=rmapmqtt(ident=RMAPUSER,lon=lon,lat=lat,host=host,network="fixed",username=RMAPUSER,password=RMAPPASSWORD,prefix="sample",maintprefix="maint")
                    rmap.loop_start()
                    rmap.connect()
                    
                    rmap.ana(anavar)
                    time.sleep(5)
                    break
                
                except:
                    print ("error reconneting")
                    time.sleep(20)

        
if __name__ == '__main__':
    main()  # (this code was run as script)
