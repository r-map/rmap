#!/usr/bin/python

# Copyright (c) 2015 Paolo Patruno <p.patruno@iperbole.bologna.it>
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

import sys,os
sys.path.insert(0, "..")

from rmap import jsonrpc
import time

#mac="860719026700066"
#mac="860719020206417"

MQTT_HOST = os.environ.get('MQTT_HOST', 'test.rmap.cc')
MQTT_RPCTOPIC = os.environ.get('MQTT_RPCTOPIC', 'rpc/pat1/1165625,4485892/fixed/')
MQTT_USERNAME = os.environ.get('MQTT_USERNAME', 'pat1')
MQTT_PASSWORD = os.environ.get('MQTT_PASSWORD', '1password')


with jsonrpc.ServerProxy( jsonrpc.JsonRpc20(),\
                          jsonrpc.TransportMQTT(
                              host=MQTT_HOST, user=MQTT_USERNAME,password=MQTT_PASSWORD,
                              rpctopic=MQTT_RPCTOPIC,
                              logfunc=jsonrpc.log_stdout,timeout=1000)) as rpcproxy :
    
    rpcproxy.recovery(dts=[2021,12,22,12,0,0])


