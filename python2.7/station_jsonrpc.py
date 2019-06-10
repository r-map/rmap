#!/usr/bin/env python

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
import rmap
import datetime

#rpcaddress=0x44
#rpcaddress=0x4f
rpcaddress=0x49
vartable="B12101"

# bluetooth
#rpcproxy = rmap.jsonrpc.ServerProxy( rmap.jsonrpc.JsonRpc20(), rmap.jsonrpc.TransportSERIAL( logfunc=rmap.jsonrpc.log_file("logrpc.txt"),port='/dev/rfcomm0',baudrate=115200,sleep=0) )

# USB
rpcproxy = rmap.jsonrpc.ServerProxy( rmap.jsonrpc.JsonRpc20(), rmap.jsonrpc.TransportSERIAL( logfunc=rmap.jsonrpc.log_file("logrpc.txt"),port='/dev/ttyACM0',baudrate=9600,sleep=2) )

valuerpc = rpcproxy.gettmpvalues(address=rpcaddress )[vartable]
reptime  = datetime.datetime.now()

print valuerpc
print (valuerpc-27315)/100.,reptime

rpcaddress=0x49
valuerpc = rpcproxy.getadt7420values(address=rpcaddress )[vartable]
reptime  = datetime.datetime.now()

print valuerpc
print (valuerpc-27315)/100.,reptime
