import sys, os
sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

os.environ['DJANGO_SETTINGS_MODULE'] = 'rmap.settings'
import django
django.setup()

import rmap.jsonrpc as jsonrpc
from rmap import rmap_core

def bin(s):
    return str(s) if s<=1 else bin(s>>1) + str(s&1)

def bitprepend(template,bit,nbit):
    return (template<<nbit) | bit

def bitextract(template,start,nbit):
    return (template>>start)&((1 << nbit) - 1)

def bit2bytelist(template,totbit):
    data=[]
    start=totbit-8
    while start >= 0:
        data.append(bitextract(template,start,8))
        start-=8
    if start >=-7:
        data.append(bitextract(template,0,8+start)<< -start )
    return data

# create JSON-RPC server
server = jsonrpc.ServerProxy(jsonrpc.JsonRpc20(radio=False,notification=False), jsonrpc.TransportSERIAL(port="/dev/ttyUSB0",baudrate=19200, logfunc=jsonrpc.log_file("myrpc.log"),timeout=15))


# call a remote-procedure
#print server.set(ack=1)
#Note: Use lsb notation for deveui var, use lsb/msb switch button on ttn web page for format adjustement
#print server.set(deveui=(1,2,3,4,5,6,7,8))
#Note: Use lsb notation for appeui var, use lsb/msb switch button on ttn web page for format adjustement
#print server.set(appeui=(1,2,3,4,5,6,7,8))
#Note: Use msb notation for appkey var, use lsb/msb switch button on ttn web page for format adjustement
#print server.set(appkey=(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16))
#print server.save(eeprom=True)

mytemplate=1
temperature=278.5
humidity=55.

# start encoding
template=0
totbit=0
#set data template number
nbit=8
bit=mytemplate
template=bitprepend(template,bit,nbit)
totbit+=nbit

#insert temperature
nbit=rmap_core.ttntemplate[mytemplate][0]["nbit"]
scale=rmap_core.ttntemplate[mytemplate][0]["scale"]
offset=rmap_core.ttntemplate[mytemplate][0]["offset"]
bit=int(temperature*scale-offset)
template=bitprepend(template,bit,nbit)
totbit+=nbit

#insert humidity
nbit=rmap_core.ttntemplate[mytemplate][1]["nbit"]
scale=rmap_core.ttntemplate[mytemplate][1]["scale"]
offset=rmap_core.ttntemplate[mytemplate][1]["offset"]
bit=int(humidity*scale-offset)
template=bitprepend(template,bit,nbit)
totbit+=nbit

print "totbit: ",totbit
print bin(template)

#create a list of bytes
data=bit2bytelist(template,totbit)
print data
    
print server.send(payload=data)
