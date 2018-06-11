import sys, os
sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

os.environ['DJANGO_SETTINGS_MODULE'] = 'rmap.settings'
import django
django.setup()

import rmap.jsonrpc as jsonrpc
from rmap import rmap_core


def compact(mytemplate,mydata):
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

    # start encoding
    template=0
    totbit=0
    #set data template number
    nbit=8
    bit=mytemplate
    template=bitprepend(template,bit,nbit)
    totbit+=nbit

    #insert data
    for bcode,meta in rmap_core.ttntemplate[mytemplate].items():
        print "insert: ", bcode
        if bcode in mydata:
            bit=int(mydata[bcode]*meta["scale"]-meta["offset"])
        else:
            print "missed data"
            bit=(1<<meta["nbit"])-1
        template=bitprepend(template,bit,meta["nbit"])
        totbit+=meta["nbit"]

    print "totbit: ",totbit
    print bin(template)

    #create a list of bytes
    data=bit2bytelist(template,totbit)

    ##convert endian
    #for i in xrange(0,len(data),2):
    #    tmp=data[i]
    #    data[i]=data[i+1]
    #    data[i+1]=tmp
        
    return data

# create JSON-RPC server
server = jsonrpc.ServerProxy(jsonrpc.JsonRpc20(radio=False,notification=False), jsonrpc.TransportSERIAL(port="/dev/ttyUSB0",baudrate=19200, logfunc=jsonrpc.log_file("myrpc.log"),timeout=15))


# call a remote-procedure

# configure parameter for join
#Note: Use lsb notation for deveui var, use lsb/msb switch button on ttn web page for format adjustement
print server.set(deveui=(0x42, 0xEC, 0x5D, 0x6A, 0x74, 0x25, 0x3D, 0x00))
#Note: Use lsb notation for appeui var, use lsb/msb switch button on ttn web page for format adjustement
print server.set(appeui=(0x04, 0x87, 0x00, 0xD0, 0x7E, 0xD5, 0xB3, 0x70))
#Note: Use msb notation for appkey var, use lsb/msb switch button on ttn web page for format adjustement
print server.set(appkey=(0x36, 0xA4, 0x12, 0x5A, 0xBC, 0xBD, 0xEA, 0x66, 0x1B, 0xA4, 0xE9, 0x00, 0xE7, 0x46, 0xD2, 0x21))

# set other parameter
#  "reset" "sampletime" "ack" "mobile" "template" "fs" "template" "deveui" "appeui" "appkey" "netid" "devaddr" "seqnoUp" "seqnoDn" "nwkkey" "artkey" return "ok"
print server.set(sampletime=1800,ack=1,fs=12,mobile=False)

# set or read sf
# parameter "sf" return "sf"
print server.sf(sf=12)
print server.sf()

# save configuration parameter to eeprom; if executed after join save session parameter and use it on next reboot
# parameter "eeprom" return "ok"
#print server.save(eeprom=True)

# (save parameter and sleep waiting for interrupt)
# parameter none return none
print server.shutdown()


# send data
mytemplate=1
mydata={"B12101":278.5,"B13003":55.}
data=compact(mytemplate,mydata)
print data
print [format(value, '02x') for value in data]
# parameter "payload" return "payloadlen" "status" "ack"
print server.send(payload=data)

