#!/usr/bin/python

# Copyright (c) 2013 Paolo Patruno <p.patruno@iperbole.bologna.it>
#                    Emanuele Di Giacomo <edigiacomo@arpa.emr.it>
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 
# 1. Redistributions of source code must retain the above copyright notice,
#   this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
# 3. Neither the name of mosquitto nor the names of its
#   contributors may be used to endorse or promote products derived from
#   this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

from rmap import jsonrpc
import time

# USB

rpcproxy = jsonrpc.ServerProxy( jsonrpc.JsonRpc20(),\
                               jsonrpc.TransportSERIAL( logfunc=jsonrpc.log_file("logrpc.txt"),\
                                                        port='/dev/ttyUSB0',baudrate=9600,timeout=5) )


time.sleep(1)

print "start configuration"

status=False
while ( not status):
    try:
        status = rpcproxy.configure(reset=True )
    except:
        status = False
    print "reset:",status
    time.sleep(1)


status=False
while ( not status):
    try:
        status = rpcproxy.configure(mqttrootpath="meteo/-/1012345,4412345/rmap/")
    except:
        status = False
    print "mqttrootpath:",status
    time.sleep(1)

status=False
while ( not status):
    try:
        status = rpcproxy.configure(mqttsampletime=10,mqttserver="rmap.cc")
    except:
        status = False
    print "mqttserver:",status
    time.sleep(1)


status=False
while ( not status):
    try:
        status = rpcproxy.configure(mqttuser="",mqttpassword="")
    except:
        status = False
    print "mqtt user and password:",status
    time.sleep(1)


#status=False
#while ( not status):
#    try:
#        status = rpcproxy.configure(driver="TMP",type="TMP",node=0,address=72,mqttpath="254,0,0/105,2000,-,-/")
#    except:
#        status = False
#    print "add driver:", status
#    time.sleep(1)

#status = rpcproxy.configure(driver="ADT",type="ADT",node=0,address=75,mqttpath="254,0,0/105,1000,-,-/")
#print "add driver:", status

status=False
while ( not status):
    try:
        status = rpcproxy.configure(save=True )
    except:
        status = False
    print "save",status
    time.sleep(1)


