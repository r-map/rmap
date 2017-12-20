import sys, os
sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

# create JSON-RPC client
import rmap.jsonrpc as jsonrpc
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

print server.send(payload="ciao bello")
