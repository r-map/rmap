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

import jsonrpc

class androbluetooth():
    """
    Establish a blutooth interface (pair and unpair device)
    """

    def __init__(self,name="mybluetooth",logfunc=jsonrpc.log_stdout):
        
        self.bluetooth_name=name
        self.bluetooth=None
        self.logfunc=logfunc
        self.bluetooth_status='BlueTooth Status: DISCONNECTED'

    def connect(self):
        """
        Connect (pair) a bluetooth device with SSP profile and configure it as transport for jsonrpc
        """
        try:
            self.bluetooth=jsonrpc.TransportBLUETOOTH(name=self.bluetooth_name,timeout=3, logfunc=self.logfunc)
            self.bluetooth_status='BlueTooth Status: CONNECTED'

            if self.bluetooth.recv_stream is None or self.bluetooth.send_stream is None:
                self.bluetooth=None
                self.bluetooth_status='BlueTooth Status: DISABLED'
                print("cannot activate\nbluetooth")

        except:
            print("cannot activate\nbluetooth")
            try:
                self.bluetooth.close()
            except:
                pass

            self.bluetooth=None
            self.bluetooth_status='BlueTooth Status: ERROR'

        return self.bluetooth


    def close(self):
        """
        Disconnect a bluetooth device (unpair)
        """

        if self.bluetooth is None:
            print("bluetooth disabled")
            return

        try:
            self.bluetooth.close()
            self.bluetooth_status='BlueTooth Status: DISCONNECTED'
        except:
            self.bluetooth_status='BlueTooth Status: ERROR'

        self.bluetooth=None

