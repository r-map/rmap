#!/usr/bin/env python
# GPL. (C) 2015 Paolo Patruno.

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
# 

import sys, os
sys.path = [os.path.join(os.getcwd(),"..")] + sys.path
os.chdir("..")

os.environ['DJANGO_SETTINGS_MODULE'] = 'rmap.settings'
from django.conf import settings

import django
django.setup()

from rmap.rmapstation import station

if __name__ == '__main__':

    arg = os.getenv('PYTHON_SERVICE_ARGUMENT')
    print "message from father: ",arg

    if arg is None:
        arg="station"

    if arg == "webserver":

        from django.core.servers.basehttp import run, get_internal_wsgi_application

        run("127.0.0.1",8888,get_internal_wsgi_application(),
            ipv6=False, threading=False)

    if arg == "station":

        mystation=station()
        print "background restored queue:",mystation.anavarlist,mystation.datavarlist
        mystation.display()

        mystation.boot()
        mystation.loopforever()
