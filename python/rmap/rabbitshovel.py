#!/usr/bin/python

# Copyright (c) 2014 Paolo Patruno <p.patruno@iperbole.bologna.it>
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

import subprocess

def quote(stringa):
   return stringa.replace( "$", "\$" )

class shovel():

   def  __init__(self,name="rmap",srcserver="localhost",srcqueue="rmap",
                 destserver="rmap.cc",destexchange="rmap",reconnectdelay=60):
      self.name=name
      self.srcserver=srcserver
      self.srcqueue=srcqueue
      self.destserver=destserver
      self.destexchange=destexchange
      self.reconnectdelay=reconnectdelay
      

   def delete(self):
      subprocess.call(('sudo','rabbitmqctl','clear_parameter','shovel',self.name))

   def create(self,srcuser="rmap",srcpassword="rmap",destuser="rmap",destpassword="rmap"):

      stringa='{'+ \
      '"src-uri": "amqp://'+quote(srcuser)+':'+quote(srcpassword)+'@'+self.srcserver+'"'+ \
      ',"src-queue": "'+self.srcqueue+ '"'+ \
      ',"dest-uri": "amqp://'+quote(destuser)+':'+quote(destpassword)+'@'+self.destserver+'"'+ \
      ',"dest-exchange": "'+self.destexchange+'"'+ \
      ',"prefetch-count":1'+ \
      ',"reconnect-delay":'+str(int(self.reconnectdelay))+ \
      ',"publish-properties": { "user_id": "'+destuser+'" }'+ \
      '}'
      
      subprocess.call(('sudo','rabbitmqctl','set_parameter','shovel',self.name,stringa))


def main():
   sh=shovel()
   sh.delete()
   sh.create(destuser="myuser",destpassword="my$pas%20.sword")

if __name__ == '__main__':
   main()  # (this code was run as script)
