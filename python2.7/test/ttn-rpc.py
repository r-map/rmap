appid="ttn2dballe"
devid="prova"
password="ttn-account-v2.your ttn password"
confirmed=False
port=1
schedule="replace"

import sys, os
sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

os.environ['DJANGO_SETTINGS_MODULE'] = 'rmap.settings'
import django
django.setup()

import rmap.jsonrpc as jsonrpc
from rmap import rmap_core
import base64

# create JSON-RPC server

ttntransport=jsonrpc.TransportTTN(appid=appid,devid=devid,password=password,confirmed=confirmed,port=port,schedule=schedule,logfunc=jsonrpc.log_stdout)
    
with  jsonrpc.ServerProxy( jsonrpc.JsonRpc20(),ttntransport) as rpcproxy:
    if (rpcproxy is None):
        print ">>>>>>> Error building ttn transport"
        raise SystemExit(1)
    else:

        print ">>>>>>> execute ttn JSRPC"


        mydata=[{"n":7,"s":True},{"n":5,"s":False},{"n":4,"s":True},{"n":8,"s":False}]
        mydata=[{"n":7,"s":True},{"n":5,"s":False}]
        data=rmap_core.compact(2,mydata)
        print data
        
        payload_raw=base64.encodestring(data)
        
        print "pinonoff",rpcproxy.pinonoff(payload_raw=payload_raw )
    
        print "END of ttn JSRPC"


