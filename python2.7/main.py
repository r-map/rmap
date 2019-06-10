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

import os
os.environ['DJANGO_SETTINGS_MODULE'] = 'rmap.settings'
from django.conf import settings
from django.utils import translation
from django.core import management

if __name__ == '__main__':

    import django
    django.setup()
    translation.activate("it")

    #management.call_command("showmigrations")
    try:
        management.call_command("migrate",no_initial_data=True )
    except:
        print "error on django command migrate on boot"

    from rmap.rmapgui import Rmap
    Rmap().run()
