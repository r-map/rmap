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

import kivy
kivy.require('1.8.0') # replace with your current kivy version !
from kivy.base import stopTouchApp
from kivy.app import App
from kivy.uix.label import Label
from kivy.uix.button import Button
from kivy.uix.popup import Popup
from kivy.uix.tabbedpanel import TabbedPanel
from kivy.uix.textinput import TextInput
from kivy.uix.boxlayout import BoxLayout
#from kivy.uix.settings import Settings
from kivy.uix.image import Image
from kivy.properties import NumericProperty
from kivy.lang import Builder
from kivy.adapters.dictadapter import DictAdapter
from kivy.uix.gridlayout import GridLayout
from kivy.uix.listview import ListView, ListItemButton
from gps import *
from kivy.properties import StringProperty
from kivy.clock import Clock, mainthread
import re,os
import webbrowser
#from kivy.config import Config
import random
import time
from datetime import datetime, timedelta
import rmapstation
import btable
import tables
import jsonrpc
from sensordriver import SensorDriver
from mapview import MapMarkerPopup
from mapview import MapView
from django.utils.translation import ugettext as _
from django.utils import translation
from django.contrib.auth.models import User
from stations.models import StationMetadata,StationConstantData
from django.db import IntegrityError
from django.core.exceptions import ObjectDoesNotExist
from kivy.lib import osc
from kivy.utils import platform
from kivy.uix.widget import Widget                                                              
from utils import nint
import rmap.rmap_core

platform = platform()

if platform == 'android':
    from jnius import autoclass
    station_default= "BT_fixed"
    board_default= "BT_fixed"
elif platform == 'linux':
    station_default= "BT_fixed"
    board_default= "BT_fixed_LINUX"
elif platform == 'win':
    station_default= "BT_fixed"
    board_default= "BT_fixed_WINDOWS"
elif platform == 'macosx':
    station_default= "BT_fixed"
    board_default= "BT_fixed_OSX"
elif platform == 'ios':
    print "ios platform not tested !!!!!"
    station_default= "BT_fixed"
    board_default= "BT_fixed_IOS"
else:
    print "platform unknown !!!!"
    station_default= "BT_fixed"
    board_default= "BT_fixed"

#    from plyer import camera #object to read the camera

#    from android.runnable import run_on_ui_thread
#    WebView = autoclass('android.webkit.WebView')
#    WebViewClient = autoclass('android.webkit.WebViewClient')
#    activity = autoclass('org.renpy.android.PythonActivity').mActivity
#
#
#    class Wv(Widget):
#        def __init__(self, **kwargs):
#            super(Wv, self).__init__(**kwargs)
#
#        def open(self,uri):
#            self.uri=uri
#            Clock.schedule_once(self.create_webview, 0)
#
#        @run_on_ui_thread
#        def create_webview(self, *args):
#            webview = WebView(activity)
#            webview.getSettings().setJavaScriptEnabled(True)
#            wvc = WebViewClient();
#            webview.setWebViewClient(wvc);
#            activity.setContentView(webview)
#            webview.loadUrl(self.uri)


#Config.adddefaultsection("rmap")
#Config.setdefault("rmap", "user", "rmap")


kv='''
#### _("string") here is not usefull because makemessages do catch it
####:import _ django.utils.translation.ugettext
#:import MapSource mapview.MapSource
#:import RiseInTransition kivy.uix.screenmanager.RiseInTransition
###:import camera plyer.camera

<Toolbar@BoxLayout>:
    size_hint_y: None
    height: '40dp'
    padding: '2dp'
    spacing: '2dp'

    canvas:
        Color:
            rgba: .2, .2, .2, .6
        Rectangle:
            pos: self.pos
            size: self.size


<mytextinput>:
    text: ''
    multiline: False
    on_text_validate: self.validatetext()

<MyMapMarker>:

ScreenManager:
    id: screenmanager
    transition: RiseInTransition()



    Screen:
        name: "Setup"
        id: setupscreen

        canvas:
            Color:
                rgba: 50/255., 0/255., 0/255., 1
            Rectangle:
                pos: self.pos
                size: self.size

        GridLayout:

            orientation: 'vertical'
            cols: 1

            Toolbar:

                Button:
                    text:  app.str_Settings

                    on_release:
                        app.open_settings() 

                Label:
                    text: app.str_start
                    color:  1., .5, .5, 1.
                    bold: True

                Button:
                    text: app.str_Next
                    on_release:
                        screenmanager.current = screenmanager.next()


            Toolbar:


                Button:
                    text: app.str_Register
                    on_press: app.register() 

                Button:
                    text: app.str_View_graph
                    on_press: app.view() 


            #Label:
            #    text: app.message 


            #BoxLayout:
            Carousel:
                ScrollView:
                    bar_width: 10
                    Label:
                        height: self.texture_size[1]
                        text_size: self.width, None

                        id: help_intro
                        text: app.str_help_manual_intro
                        markup: True
                        halign: "left"
                        size_hint_y: None

                ScrollView:
                    bar_width: 10
                    Label:
                        height: self.texture_size[1]
                        text_size: self.width, None

                        id: help_setup
                        text: app.str_help_manual_setup
                        markup: True
                        halign: "left"
                        size_hint_y: None

                ScrollView:
                    bar_width: 10
                    Label:
                        height: self.texture_size[1]
                        text_size: self.width, None

                        id: help1
                        text: app.str_help_manual_page1
                        markup: True
                        halign: "left"
                        size_hint_y: None

                ScrollView:
                    bar_width: 10
                    Label:
                        height: self.texture_size[1]
                        text_size: self.width, None

                        id: help2
                        text: app.str_help_manual_page2
                        markup: True
                        halign: "left"
                        size_hint_y: None

                ScrollView:
                    bar_width: 10
                    Label:
                        height: self.texture_size[1]
                        text_size: self.width, None

                        id: help3
                        text: app.str_help_manual_page3
                        markup: True
                        halign: "left"
                        size_hint_y: None

                ScrollView:
                    bar_width: 10
                    Label:
                        height: self.texture_size[1]
                        text_size: self.width, None

                        id: help4
                        text: app.str_help_manual_page4
                        markup: True
                        halign: "left"
                        size_hint_y: None

                ScrollView:
                    bar_width: 10
                    Label:
                        height: self.texture_size[1]
                        text_size: self.width, None

                        id: help5
                        text: app.str_help_manual_page5
                        markup: True
                        halign: "left"
                        size_hint_y: None


    Screen:
        name: app.str_Location
        id: locationscreen

        MapView:
            #on_map_relocated: app.map_relocated(self.lat,self.lon)
            id: mapview
            lat: 42.
            lon: 13.
            zoom: 5
            #size_hint: .5, .5
            #pos_hint: {"x": .25, "y": .25}

            MirinoImage:
                id: mirino
                center: self.parent.center
                #center: mapview.get_window_xy_from(mapview.lat, mapview.lon, mapview.zoom)

            MyMapMarker:
                id: marker
                lat: app.lat
                lon: app.lon

                popup_size: dp(150), dp(100)

                Bubble:
                    BoxLayout:
                        orientation: "horizontal"
                        padding: "5dp"
                        Label:
                            id: markerlabel
                            text: app.str_lat_lon_height % (app.location,app.lat,app.lon,app.height)
                            markup: True
                            halign: "center"


        GridLayout:
            orientation: 'vertical'
            cols: 1
            row_force_default: True
            row_default_height: "40dp"

            Toolbar:


                Button:
                    text: app.str_Previous
                    on_release:
                        screenmanager.current = screenmanager.previous()

                Label:
                    text:   app.str_Select_Location
                    color:  1., .5, .5, 1.
                    bold: True

                Button:
                    text:   app.str_Next
                    on_release:
                        screenmanager.current = screenmanager.next()

            Toolbar:

                ToggleButton:
                    id: gps
                    text: app.str_Start_GPS if self.state == 'normal' else app.str_Stop_GPS
                    on_state: app.startgps() if self.state == 'down' else app.stopgps()

                ToggleButton:
                    id: trip
                    text: app.str_Start_Trip if self.state == 'normal' else app.str_Stop_Trip
                    on_state: app.starttrip() if self.state == 'down' else app.stoptrip()


                Button:
                    text: app.str_Save_Location
                    on_press: app.savelocation()



        StackLayout:
            orientation: "lr-bt"
            id: locationlayout

            Toolbar:
                height: '20dp'
                padding: '2dp'
                spacing: '2dp'
                Label:
                    id: lon
                    text: "Lon: {:4.5f}".format(mapview.lon)
                Label:
                    id: height
                    text: app.str_Height.format(app.height)
                Label:
                    id: lat
                    text: "Lat: {:4.5f}".format(mapview.lat)

            Toolbar:
                height: '20dp'
                padding: '2dp'
                spacing: '2dp'

                Label:
                    text: app.gps_location
            Toolbar:
                height: '20dp'
                padding: '2dp'
                spacing: '2dp'
                Label:
                    text: app.gps_status 

    Screen:
        name: "Data"
        id: datascreen

        canvas:
            Color:
                rgba: 50/255., 100/255., 50/255., 1
            Rectangle:
                pos: self.pos
                size: self.size


        GridLayout:
            orientation: 'vertical'
            cols: 1
            #row_force_default: True
            #row_default_height: '40dp'

            Toolbar:

                Button:
                    text: app.str_Previous
                    on_release:
                        screenmanager.current = screenmanager.previous()

                Label:
                    text: app.str_Insert_data
                    color:  1., .5, .5, 1.
                    bold: True

                Button:
                    text: app.str_Next
                    on_release:
                        screenmanager.current = screenmanager.next()

            TabbedPanel:
                do_default_tab: False
                tab_width: '100sp'

                TabbedPanelItem:
                    text: app.str_Meteo

                    TabbedPanel:
                        do_default_tab: False
                        tab_width: '150sp'

                        TabbedPanelItem:
                            text: app.str_Manual_measurements

                            GridLayout:
                                orientation: 'vertical'
                                cols: 1
                                row_force_default: True
                                row_default_height: '40dp'

                                canvas:
                                    Color:
                                        rgba: 50/255., 100/255., 50/255., 1
                                    Rectangle:
                                        pos: self.pos
                                        size: self.size

                                BoxLayout:
                                    canvas:
                                        Color:
                                            rgba: 100/255., 100/255., 50/255., 1
                                        Rectangle:
                                            pos: self.pos
                                            size: self.size
                                    Label:
                                        text: app.str_Snow_height
                                    mytextinput:
                                        id: snow
                                BoxLayout:
                                    canvas:
                                        Color:
                                            rgba: 50/255., 100/255., 100/255., 1
                                        Rectangle:
                                            pos: self.pos
                                            size: self.size
                                    Label:
                                        text: app.str_Visibility
                                    mytextinput:
                                        id: fog

                        TabbedPanelItem:
                            id: presentwtab
                            text: app.str_Presentw

                TabbedPanelItem:
                    text: app.str_Air_Quality
                    GridLayout:
                        orientation: 'vertical'
                        cols: 1
                        row_force_default: True
                        row_default_height: '40dp'

                        Label:
                            text: app.str_Air_quality_tab_content_area

                TabbedPanelItem:
                    text: app.str_Water_Quality
                    GridLayout:
                        orientation: 'vertical'
                        cols: 1
                        row_force_default: True
                        row_default_height: '40dp'

                        Label:
                            text: app.str_Water_quality_tab_content_area


                    #GridLayout:
                    #    orientation: 'vertical'
                    #    cols: 1
                    #    row_force_default: True
                    #    row_default_height: '40dp'


                    #    Label:
                    #        text: app.str_Special_tab_content_area

                    #SpecialView:
                    #    id: present_weather

            Toolbar:

                Button:
                    text: app.str_Queue_data_to_be_published
                    on_press: app.queuedata() 


    Screen:
        name: "Board"
        id: board


        GridLayout:
            orientation: 'vertical'
            cols: 1
            #row_force_default: True
            canvas:
                Color:
                    rgba: 50/255., 50/255., 100/255., 1
                Rectangle:
                    pos: self.pos
                    size: self.size

            Toolbar:

                Button:
                    text: app.str_Previous
                    on_release:
                        screenmanager.current = screenmanager.previous()

                Label:
                    text: app.str_Automatic_data
                    color:  1., .5, .5, 1.
                    bold: True

                Button:
                    text: app.str_Next
                    on_release:
                        screenmanager.current = screenmanager.next()


            Toolbar:

                Button:
                    id: configure
                    text: app.str_setup
                    on_release: app.configurestation()
                Button:
                    id: getdata
                    text: app.str_getdata
                    on_release: app.getdata()

            Toolbar:

                ToggleButton:
                    id: transport
                    text: app.str_Start_transport if self.state == 'normal' else app.str_Stop_transport
                    on_state: app.starttransport() if self.state == 'down' else app.stoptransport()

                ToggleButton:
                    text: app.str_Sample_ON if self.state == 'normal' else app.str_Sample_OFF
                    on_state: app.sampleon() if self.state == 'down' else app.sampleoff()

            #Label:
            #    text: app.board_message 

            BoxLayout:
                ScrollView:
                    bar_width: 10
                    Label:

                        height: self.texture_size[1]
                        text_size: self.width, None

                        id: board_message
                        text: app.board_message
                        markup: False
                        halign: "left"
                        size_hint_y: None

            Toolbar:
                height: '40dp'
                padding: '2dp'
                spacing: '2dp'

                Label:
                    text: app.board_status 

    Screen:
        name: app.str_Publish
        id: publishscreen

        canvas:
            Color:
                rgba: 100/255., 50/255., 50/255., 1
            Rectangle:
                pos: self.pos
                size: self.size

        GridLayout:
            orientation: 'vertical'
            cols: 1

            Toolbar:

                Button:
                    text: app.str_Previous
                    on_release:
                        screenmanager.current = screenmanager.previous()

                Label:
                    text: app.str_Publish
                    color:  1., .5, .5, 1.
                    bold: True


                Button:
                    text: app.str_Settings
                    on_release:
                        app.open_settings() 

#                    text: app.str_Next
#                    on_release:
#                        screenmanager.current = screenmanager.next()

            Toolbar:

                ToggleButton:
                    id: webserverbutton
                    text: app.str_configure_board if self.state == 'normal' else app.str_configure_board
                    on_state: app.servicewebserver()

            Toolbar:

#                ToggleButton:
#                    id: webserverbutton
#                    text: app.str_configure_board if self.state == 'normal' else app.str_configure_board
#                    on_state: app.servicewebserver()

                Button:
                    text: app.str_Clean_Queue
                    on_press: app.cleandata() 

                ToggleButton:
                    text: app.str_Connect if self.state == 'normal' else app.str_Disconnect
                    on_state: app.queueon() if self.state == 'down' else app.queueoff()

#                Button:
#                    text: app.str_Publish
#                    on_press: app.publishmqtt() 

            BoxLayout:
                ScrollView:
                    bar_width: 10
                    Label:

                        height: self.texture_size[1]
                        text_size: self.width, None

                        id: queue
                        text: app.str_Queue_status
                        markup: True
                        halign: "left"
                        size_hint_y: None

            Toolbar:

                ToggleButton:
                    id: stationbutton
                    text: app.str_run_background if self.state == 'normal' else app.str_stop_background
                    on_state: app.servicestation()

            Toolbar:
                height: '40dp'
                padding: '2dp'
                spacing: '2dp'
                Label:
                    text: app.mqtt_status 


# android version
#    Screen:
#        name: "Camera"
#        id: camerascreen
#
#        BoxLayout:
#            orientation: 'vertical'
#            canvas:
#                Color:
#                    rgba: 50/255., 50/255., 100/255., 1
#                Rectangle:
#                    pos: self.pos
#                    size: self.size
#
#
#            BoxLayout:
#                orientation: 'horizontal'
#                size_hint_y: None
#                height: '40dp'
#                Button:
#                    text: app.str_Start
#                    on_release: app.take_photo


#linux version
#    Screen:
#        name: "Camera"
#        id: camerascreen
#
#        BoxLayout:
#            orientation: 'vertical'
#            canvas:
#                Color:
#                    rgba: 50/255., 50/255., 100/255., 1
#                Rectangle:
#                    pos: self.pos
#                    size: self.size
#
#            camera:
#                id: mycamera
#                resolution: 399, 299
#
#            BoxLayout:
#                orientation: 'horizontal'
#                size_hint_y: None
#                height: '40dp'
#                Button:
#                    text: app.str_Start
#                    on_release: mycamera.play = True
#
#                Button:
#                    text: app.str_Stop
#                    on_release: mycamera.play = False

'''


class values(tables.Table):

    value=None

    def value_changed(self, list_adapter, *args):

        self.value = None

        if len(list_adapter.selection) > 0:
            for key,item in self.iteritems():
                if str(item) == list_adapter.selection[0].text:
                    self.value = item.code

        print "table values changed to: ",self.value


class PresentwView(GridLayout):
    '''
    Implementation of an master-detail view with a vertical scrollable list
    '''

    def __init__(self,table,**kwargs):
        kwargs['cols'] = 1
        super(PresentwView, self).__init__(**kwargs)


        list_item_args_converter = \
                lambda row_index, rec: {'text': _(str(rec)),
                                        'size_hint_y': None,
                                        'height': '90sp',
                                        'halign': 'left',
                                        'text_size': (None,None)}
                                        #'text_size': (420,None)} mmm do not scale good on different dpi

        self.dict_adapter = DictAdapter(sorted_keys=sorted(table.keys()),
                                        data=table,
                                        args_converter=list_item_args_converter,
                                        selection_mode='single',
                                        allow_empty_selection=True,
                                        cls=ListItemButton,
        )

        master_list_view = ListView(adapter=self.dict_adapter,
                                    size_hint=(.3, 1.0))


        self.dict_adapter.bind(on_selection_change=table.value_changed )
        #self.add_widget(detail_view)

        self.add_widget(master_list_view)

    def deselect(self):
        """ deselect item in listitem view"""

        for item in self.dict_adapter.selection:
            item.deselect()



def to_background(*args):
    from jnius import cast
    from jnius import autoclass
    PythonActivity = autoclass('org.renpy.android.PythonActivity')
    currentActivity = cast('android.app.Activity', PythonActivity.mActivity)
    currentActivity.moveTaskToBack(True)

class MirinoImage(Image):

    def __init__(self, **kwargs):
        super(MirinoImage, self).__init__(**kwargs)
        self.source = os.path.join(os.path.dirname(__file__), "icons", "mirino.png")
        #self.pos_hint= {'center_x': 0.5, 'center_y': 0.5}
        #self.center=(300.,300.)


class MyMapMarker(MapMarkerPopup):

    def location(self,lat,lon):

        #lat = NumericProperty(lat)
        #lon = NumericProperty(lon)
        self.lat = lat
        self.lon = lon


class mytextinput(TextInput):

    pat = re.compile('[^0-9]')

    def validatetext(self):
#        print 'Inserted:', self.text
        try:
            if float(self.text) > 999999. or float(self.text) < 0.:
                self.text=''
        except:
                self.text=''

    def insert_text(self, substring, from_undo=False):

#  disabled for now ... do not work on android!

#        pat = self.pat
#        if '.' in self.text:
#            s = re.sub(pat, '', substring)
#        else:
#            s = '.'.join([re.sub(pat, '', s) for s in substring.split('.', 1)])
#        return super(mytextinput, self).insert_text(s, from_undo=from_undo)

        return super(mytextinput, self).insert_text(substring, from_undo=from_undo)


#class MyTab(TabbedPanel):
#    pass


class Rmap(App):

    title = 'RMAP'
    
    gps_status = StringProperty("")
    gps_location = StringProperty("")
    gps_connected = False
    gps_reconnectonresume=False

    mqtt_status = StringProperty("")
    mqtt_connected = False
    mqtt_reconnectonresume=False

    board_message=StringProperty("")
    board_status = StringProperty("")

    boardmessage=[]

    #disable the section kivy in config panel
    use_kivy_settings = False
    trip=False

    rpcin_message=""

    def get_application_config(self):

        '''
        When you are distributing your application on Desktop, please note than
        if the application is meant to be installed system-wise, then the user
        might not have any write-access to the application directory. You could
        overload this method to change the default behavior, and save the
        configuration file in the user directory by default::
        class TestApp(App):
        def get_application_config(self):
        return super(TestApp, self).get_application_config(
        '~/.%(appname)s.ini')
        Some notes:
        - The tilda '~' will be expanded to the user directory.
        - %(appdir)s will be replaced with the application :data:`directory`
        - %(appname)s will be replaced with the application :data:`name`
        '''

        if platform == 'linux':
            return super(Rmap, self).get_application_config(
                '%(appname)s.ini')
        else:
            return super(Rmap, self).get_application_config()


    def build(self):
        '''
        build application
        '''

        self.translate()
        self.table = btable.Btable()

        self.gps_status = _("Start GPS to get location updates")
        self.gps_location = _("wait for location")

        self.mqtt_status = _("Connect Status: DISCONNECTED")
        self.board_status = _("Board Status: DISCONNECTED")

        self.location=self.config.get('location','name')
        self.lon=float(self.config.get('location','lon'))
        self.lat=float(self.config.get('location','lat'))
        self.height=float(self.config.get('location','height'))
        self.board_status=_("Transport Status: OFF")
        self.service = None
        self.servicename = None
        self.getdataerror=0
        self.getdataer=0

        self.gps = plyergps(call_on_location=self.on_location,call_on_status=self.on_status)

        if  os.path.isfile("servicerunning"):
            try:
                fhandle = open("servicerunning", 'r')
                self.servicename=fhandle.read()
                fhandle.close()
            except:
                print "ERROR reading servicerunning file"
                os.remove("servicerunning")

        #self.start_service()
        osc.init()
        self.oscid = osc.listen(port=3001)
        osc.bind(self.oscid, self.rpcin, '/rpc')
        #this seems do not work in on_resume environment
        #Clock.schedule_interval(lambda *x: osc.readQueue(self.oscid), 0)
        ##Clock.schedule_interval(self.rpcout, 5)

        Clock.schedule_once(self.backorfore, 0)

        root= Builder.load_string(kv)

        #root = ScreenManager()
        #root.add_widget(LocationScreen())
        #root.add_widget(DataScreen())
        #root.add_widget(PublishScreen())

        return root

    def translate(self):

        """
        this enable ugetty to find text strings
        """

        translation.activate(self.config.get('general','language'))

        self.str_start=_("Start")
        self.str_help_manual_intro=_("help manual intro")
        self.str_help_manual_setup=_("help manual setup")
        self.str_help_manual_page1=_("help manual page 1")
        self.str_help_manual_page2=_("help manual page 2")
        self.str_help_manual_page3=_("help manual page 3")
        self.str_help_manual_page4=_("help manual page 4")
        self.str_help_manual_page5=_("help manual page 5")
        self.str_Settings=_("Settings")
        self.str_Select_Location=_("Select Location")
        self.str_Next=_("Next")
        self.str_Start_GPS=_("Start GPS") 
        self.str_Stop_GPS=_("Stop GPS")
        self.str_Start_Trip=_("Start Trip")
        self.str_Stop_Trip=_("Stop Trip")
        self.str_Save_Location=_("Save Location")
        self.str_Height=_("Height: {:4.0f}")
        self.str_Previous=_("Previous")
        self.str_Insert_data=_("Insert data")
        self.str_Queue_data_to_be_published=_("Queue data to be published")
        self.str_Meteo=_("Meteo")
        self.str_Snow_height=_("Snow height(cm.)")
        self.str_Visibility=_("Visibility(m.)")
        self.str_Air_Quality=_("Air Quality")
        self.str_Water_Quality=_("Water Quality")
        self.str_Air_quality_tab_content_area=_("Air Quality tab content area\n to be done")
        self.str_Water_quality_tab_content_area=_("Water Quality tab content area\n to be done")
        self.str_Automatic_data=_("Automatic data")
        self.str_Next=_("Next")
        self.str_setup=_("Setup")
        self.str_configure_board=_("Advanced Management")
        self.str_run_background=_("Run background")
        self.str_stop_background=_("Stop background")
        self.str_getdata=_("Getdata")
        self.str_Start_transport=_("Start transport")
        self.str_Stop_transport=_("Stop transport")
        self.str_Sample_ON=_("Sample ON")
        self.str_Sample_OFF=_("Sample OFF")
        self.str_Publish=_("Publish")
        self.str_Register=_("Register")
        self.str_View_graph=_("View graph")
        self.str_Connect=_("Connect") 
        self.str_Disconnect=_("Disconnect")
        self.str_Clean_Queue=_("Clean Queue")
        self.str_Queue_status=_("Queue status")
        self.str_Presentw=_("Present weather")
        self.str_Manual_measurements=_("Manual measurements")
        self.str_lat_lon_height=_("[b]%s\nLat: %4.5f\nLon: %4.5f\nHeight: %d[/b]")
        self.str_Location=_("Location")
        self.str_Publish=_("Publish")


    def on_start(self):
        '''Event handler for the `on_start` event which is fired after
        initialization (after build() has been called) but before the
        application has started running.
        '''

        try:
            #sync data from config to db and preserve active status
            self.mystation=rmapstation.station(trip=self.trip,gps=self.gps,
                                           slug=self.config.get('sensors','station'),
                                           boardslug=self.config.get('sensors','board'),
                                           logfunc=jsonrpc.log_stdout)
            self.config2db(activate=self.mystation.active)
            self.mystation=rmapstation.station(trip=self.trip,gps=self.gps,
                                           slug=self.config.get('sensors','station'),
                                           boardslug=self.config.get('sensors','board'),
                                           logfunc=jsonrpc.log_stdout)


        except rmapstation.Rmapdonotexist:
            # retry with default; this happen when DB was modified and is not in sync with config
            self.config.set('sensors', 'station',station_default)
            self.config.set('sensors', 'board',board_default)
            self.config.write()
            self.mystation=rmapstation.station(trip=self.trip,gps=self.gps,
                                           slug=self.config.get('sensors','station'),
                                           boardslug=self.config.get('sensors','board'),
                                           logfunc=jsonrpc.log_stdout)

        except:
            print "ERROR: cannot get a good station from DB !"
            #raise SystemExit(0)
            raise


        # add listview widget
        lang=self.config.get('general','language')
        self.present_weather_table = values(os.path.join(os.path.dirname(__file__), "tables","present_weather_"+lang+".txt"))
        self.present_weather_widget=PresentwView(self.present_weather_table)
        self.root.ids["presentwtab"].add_widget(self.present_weather_widget)

        self.root.ids["queue"].text=self.queue2str()


        if self.mystation.active:

            try:
                rmap.rmap_core.sendjson2amqp(
                    station=self.config.get('sensors','station'),
                    user=self.config.get('rmap','user'),
                    password=self.config.get('rmap','password'),
                    host=self.config.get('rmap','server'))
            except:
                print _("WARNING: data not synced with server")
                self.popup(_("data not\nsynced with server"))

        else:
            self.open_settings()

    def on_stop(self):
        '''
        called on appication stop
        Here you can save data if needed
        '''
        print ">>>>>>>>> called on appication stop"

        #self.stop_service()

        self.mystation.on_stop()


    def on_pause(self):
        '''
        called on application pause
        Here you can save data if needed
        '''
        print ">>>>>>>>> called on application pause"

        if self.mqtt_connected:
            self.mqtt_reconnectonresume=True

        if self.gps_connected:
            self.gps_reconnectonresume=True

        self.on_stop()

        if self.servicename == "station":
            self.start_service("station")

        return True


    def backorfore(self,dt):
        if self.servicename == "station" and platform == 'android':
            print self.servicename,"lock screen"

            def stopstation(*args):
                self.stopservicestation()
                popup.dismiss()
                self.root.ids["stationbutton"].state="normal"


            box = BoxLayout(orientation='vertical')
            label = Label(text=_("Station is\nrunning in\nbackground"))
            bottone1 = Button(text=_("Stop background\nstation"))
            bottone2 = Button(text=_("Sorry,\nI don't want\ndisturb"))

            box.add_widget(label)
            box.add_widget(bottone1)
            box.add_widget(bottone2)

            popup = Popup(title=_("Warning"), content=box,size_hint=(.5, .5))
            bottone1.bind(on_press=stopstation)
            bottone2.bind(on_press=to_background)

            popup.open()

        if self.servicename == "webserver" and platform == 'android':
            print self.servicename," stop service"
            self.stop_service()


    def on_resume(self):
        '''
        called on appication resume
        Here you can check if any data needs replacing (usually nothing)
        '''
        print ">>>>>>>>> called on appication resume"

        self.backorfore(0)
        self.mystation.on_resume()


        if self.mqtt_reconnectonresume :
            print "start mqtt"
            self.startmqtt()

        if self.gps_reconnectonresume :
            print "start gps"
            self.gps.start()


        self.root.ids["queue"].text=self.queue2str()



    def on_config_change(self, config, section, key, value):
        ''' called when config is changed '''
                                        
        rmapchanged = False
        locationchanged = False
        languagechanged = False
        boardchanged = False
        sensorschanged = False

        if config is self.config:
            token = (section, key)

            if token == ('general', 'language'):
                print('language have been changed to', value)
                languagechanged = True

            elif token == ('rmap', 'server'):
                print('server have been changed to', value)
                rmapchanged = True
            elif token == ('rmap', 'user'):
                print('user have been changed to', value)
                rmapchanged = True
            elif token == ('rmap', 'password'):
                print('password have been changed to', value)
                rmapchanged = True
            elif token == ('rmap', 'samplerate'):
                print('samplerate have been changed to', value)
                rmapchanged = True

            elif token == ('location', 'name'):
                print('location name have been changed to', value)
                locationchanged = True
            elif token == ('location', 'lat'):
                print('lat have been changed to', value)
                locationchanged = True
            elif token == ('location', 'lon'):
                print('lon have been changed to', value)
                locationchanged = True
            elif token == ('location', 'height'):
                print('height have been changed to', value)
                locationchanged = True

            elif token == ('sensors', 'name'):
                print('sensors name have been changed to', value)
                sensorschanged = True
            elif token == ('sensors', 'station'):
                print('sensors station have been changed to', value)

                mystation=StationMetadata.objects.get(slug=self.config.get('sensors','station'))
                #board=mystation.board_set.filter(active=True)[0]
                #TODO check for no boards
                board=mystation.board_set.all()[0]
                config.set('sensors', 'board', str(board.slug))
                config.set('sensors', 'remote_board', str(board.slug))
                sensorschanged = True

            elif token == ('sensors', 'board'):
                print('sensors board have been changed to', value)
                sensorschanged = True

            if locationchanged:
                print "update location with new parameter"

                self.config2db(activate=True)

                self.location=self.config.get('location','name')
                self.lon=float(self.config.get('location','lon'))
                self.lat=float(self.config.get('location','lat'))
                self.height=float(self.config.get('location','height'))


            if rmapchanged or locationchanged or sensorschanged:
                connected=self.mqtt_connected
                if connected:
                    print "disconnect MQTT with old parameter"
                    self.stopmqtt()

                try:
                    self.mystation=rmapstation.station(trip=self.trip,gps=self.gps,
                                                       slug=self.config.get('sensors','station'),
                                                       boardslug=self.config.get('sensors','board'),
                                                       logfunc=jsonrpc.log_stdout)

                    #TODO: get name and height from DB
                    #self.location=self.config.get('location','name')
                    #self.height=float(self.config.get('location','height'))

                    self.lon=self.mystation.lon
                    self.lat=self.mystation.lat

                    self.config.set('location', 'name',self.location)
                    self.config.set('location', 'lat',self.lat)
                    self.config.set('location', 'lon',self.lon)
                    self.config.set('location', 'height',self.height)
                    self.config.write()

                    self.updatelocation()

                    if token == ('sensors', 'station'):
                        self.config.write()
                        self.close_settings()
                        self.destroy_settings()
                        self.open_settings()


                except:
                    print "ERROR recreating rmapstaton.station"

                if connected:
                    print "reconnect MQTT with new parameter"
                    self.startmqtt()

                self.config2db()


                if self.mystation.active:
                    try:
                        rmap.rmap_core.sendjson2amqp(
                            station=self.config.get('sensors','station'),
                            user=self.config.get('rmap','user'),
                            password=self.config.get('rmap','password'),
                            host=self.config.get('rmap','server'))
                    except:
                        self.popup(_("data not\nsynced with server"))


            if languagechanged:
                self.popup(_("Restart APP\nneeded"),exit=True)



    def config2db(self,activate=None,board=None):

        if board is None:
            myboard=self.config.get('sensors','board')
        else:
            myboard=board

        try:

            rmap.rmap_core.configdb(
                username=self.config.get('rmap','user'),
                password=self.config.get('rmap','password'),
                station=self.config.get('sensors','station'),
                lat=self.config.get('location','lat'),lon=self.config.get('location','lon'),
                constantdata={"B01019":self.config.get('location','name'),
                              "B07030":self.config.get('location','height')},
                mqttusername=self.config.get('rmap','user'),
                mqttpassword=self.config.get('rmap','password'),
                mqttserver=self.config.get('rmap','server'),
                mqttsamplerate=float(self.config.get('rmap','samplerate')),
                bluetoothname=self.config.get('sensors','name'),
                amqpusername=self.config.get('rmap','user'),
                amqppassword=self.config.get('rmap','password'),
                amqpserver=self.config.get('rmap','server'),
                queue="rmap",
                exchange="rmap",
                board=myboard,
                activate=activate)

        except:
            self.popup(_("Error\nsetting station"))

        #except Exception as e:
        #    self.popup(str(e))
                       #_("Error\nsetting user")
                       #_("Error\nsetting station")

    def start_service(self,cmdservice="webserver"):
        if platform == 'android':
            from android import AndroidService
            self.service = AndroidService('rmap background',cmdservice)
            self.service.start(cmdservice) # Argument to pass to a service, through the environment variable PYTHON_SERVICE_ARGUMENT.

    def stop_service(self):
        if self.service:
            self.service.stop()
            self.service = None


    def build_config(self, config):

        config.setdefaults('general', {
            'language': 'it',
        })

        config.setdefaults('rmap', {
            'user': _("your user"),
            'password': _("your password"),
            'server': 'rmap.cc',
            'samplerate': 5.,
        })

        config.setdefaults('location', {
            'name': 'home',
            'lat': 0.,
            'lon': 0.,
            'height': 0.
        })

        config.setdefaults('sensors', {
            'name': 'HC-05',
            'station': station_default,
            'board': board_default,
            'remote_board': "stima_bt"
        })

    def build_settings(self, settings):
        '''
        define the setting panel
        '''

        stations=[]
        #for station in StationMetadata.objects.filter(active=True):
        for station in StationMetadata.objects.all():
            stations.append(str(station.slug))

        mystation=StationMetadata.objects.get(slug=self.config.get('sensors','station'))

        boards=[]
        #for board in mystation.board_set.filter(active=True):
        for board in mystation.board_set.all():
            boards.append(str(board.slug))

        jsongeneral = """
[
    { "type": "title",
      "title": "General configuration" },

    { "type": "options",
      "title": "Language",
      "desc": "Language to use in application",
      "section": "general",
      "key": "language",
      "options": ["it","en"]}
]
        """

        jsonrmap = """
[
    { "type": "title",
      "title": "Rmap configuration" },

    { "type": "string",
      "title": "RMAP user",
      "desc": "RMAP user to connet to rmap server",
      "section": "rmap",
      "key": "user"},

    { "type": "string",
      "title": "RMAP password",
      "desc": "RMAP password to connet to rmap server",
      "section": "rmap",
      "key": "password"},

    { "type": "string",
      "title": "RMAP server",
      "desc": "RMAP server",
      "section": "rmap",
      "key": "server"},

    { "type": "numeric",
      "title": "Sample Time",
      "desc": "Sample Time Frequecy (s.)",
      "section": "rmap",
      "key": "samplerate"}

]
        """

        jsonlocation = """
[
    { "type": "title",
      "title": "Location" },

    { "type": "string",
      "title": "Name",
      "desc": "Location name",
      "section": "location",
      "key": "name"},
    { "type": "numeric",
      "title": "Latitude",
      "desc": "Latitude (decimal)",
      "section": "location",
      "key": "lat"},
    { "type": "numeric",
      "title": "Longitude",
      "desc": "Longitude (decimal)",
      "section": "location",
      "key": "lon"},
    { "type": "numeric",
      "title": "Height",
      "desc": "Ground Height  (m.)",
      "section": "location",
      "key": "height"}
]
        """


        jsonsensors = """
[
    { "type": "title",
      "title": "Sensors" },

    { "type": "string",
      "title": "Name",
      "desc": "BlueTooth name",
      "section": "sensors",
      "key": "name"
        },

    { "type": "options",
      "title": "Station",
      "desc": "station name",
      "section": "sensors",
      "key": "station",
      "options": 
        """ + str(stations).replace("'","\"") + """
        },

    { "type": "options",
      "title": "Board",
      "desc": "board name",
      "section": "sensors",
      "key": "board",
      "options": 
        """ + str(boards).replace("'","\"") + """
        },
    { "type": "options",
      "title": "Remote Board",
      "desc": "remote board name",
      "section": "sensors",
      "key": "remote_board",
      "options": 
        """ + str(boards).replace("'","\"") + """
        }
]
        """


        print jsonsensors

        settings.add_json_panel('General',
                                self.config, data=jsongeneral)

        settings.add_json_panel('Rmap',
                                self.config, data=jsonrmap)

        settings.add_json_panel('Location',
                                self.config, data=jsonlocation)

        settings.add_json_panel('Sensors',
                                self.config, data=jsonsensors)

    def starttransport(self):


        try:
            self.mystation.starttransport()
            self.mystation.sensorssetup()
            self.board_status='Transport Status: OK'
            self.getdataerror=0

        except:
            self.popup("cannot activate\ntransport")
            self.board_status='Transport Status: ERROR'
            self.root.ids["transport"].state="normal"


    def stoptransport(self):

        try:
            self.mystation.stoptransport()
            self.board_status='Transport Status: OFF'
        except:
            print "error in stoptransport"
            self.board_status='Transport Status: ERROR'


    def configurestation(self):

        try:
            # this stop transport if active (configure restart transport and stop it at the end)
            self.root.ids["transport"].state="normal"
            self.board_status='Transport Status: OFF'
            try:
                self.stoptransport()
            except:
                pass

            self.config2db(activate=True,board=self.config.get('sensors','remote_board'))
            self.mystation.configurestation(board_slug=self.config.get('sensors','remote_board'))
            self.board_status=_("Transport Status: CONFIG OK")

            try:
                rmap.rmap_core.sendjson2amqp(
                    station=self.config.get('sensors','station'),
                    user=self.config.get('rmap','user'),
                    password=self.config.get('rmap','password'),
                    host=self.config.get('rmap','server'))
            except:
                self.popup(_("data not\nsynced with server"))

            #self.mystation.stoptransport()
        except:
            self.board_status=_("Transport Status: CONFIG ERROR")
            self.popup(_("ERROR configure\nboard"))




    def getdata(self):

        self.root.ids["transport"].state="down"

        try:
            self.mystation.now=datetime.utcnow()
            datavars=self.mystation.getdata_loop(trip=self.trip)
            message=""
            for datavar in datavars:
                for bcode,data in datavar.iteritems():
                    message += str(self.table[bcode])+": "+ data["t"].strftime("%d/%m/%y %H:%M:%S")+" -> "+str(data["v"])+"\n"

            self.boardmessage.append(message)
            self.boardmessage=self.boardmessage[-20:]

            message=""
            for mes in self.boardmessage:
                message+=mes
            self.board_message=message
            self.board_status=_("Transport Status: OK")+_(" err: ")+ str(self.getdataerror)

        except:
            print "ERROR executing getdata"

            self.popup(_("ERROR getting\ndata"))
            self.getdataerror+=1
            self.board_status=_("Transport Status: ERROR")+_(" err: ")+ str(self.getdataerror)

        self.root.ids["queue"].text=self.queue2str()

    def sampleon(self):

        #self.starttransport()
        self.root.ids["transport"].state="down"

        #update trip inside mystation
        #self.mystation.trip=self.trip
        self.mygetdata_loop=self.getdata_loop
        Clock.schedule_interval(self.mygetdata_loop, float(self.config.get('rmap','samplerate')))

    def sampleoff(self):
        Clock.unschedule(self.mygetdata_loop)


    def queueon(self):

        self.startmqtt()
        self.myqueue_loop=self.publishmqtt_loop
        Clock.schedule_interval(self.myqueue_loop, 5.)

    def queueoff(self):
        Clock.unschedule(self.myqueue_loop)
        self.stopmqtt()
        #update to last status
        self.mqtt_status = self.mystation.mqtt_status
        self.mqtt_connected = self.mystation.rmap.connected

    def rpcin(self, message, *args):
        print "RPC: ",message[2]
        self.rpcin_message=message[2]

    #def rpcout(self, *args):
    #    osc.sendMsg('/rpc', ["testinout",], port=3000)

    def rpcout(self, message):
        osc.sendMsg('/rpc', [message,], port=3000)



    def servicewebserver(self):

        if platform != 'android':
            self.popup(_("not supported\non this\nplatform!"))
            return

        if self.root.ids["webserverbutton"].state == "down":
            if self.service is None:
                self.start_service("webserver")
                self.servicename="webserver"

                time.sleep(6)
                webbrowser.open("http://localhost:8888/admin/")

            else:
                self.root.ids["webserverbutton"].state="normal"
                self.popup(_("service\nalready\nactive!"))

        else:
            if self.servicename=="webserver":
                self.stop_service()
                self.servicename=None


    def servicestation(self):

        if platform != 'android':
            self.popup(_("not supported\non this\nplatform!"))
            return

        if self.root.ids["stationbutton"].state == "down":

            if self.service is None:

                self.servicename="station"
                fhandle = open("servicerunning", 'w')
                fhandle.write(self.servicename)
                fhandle.close()

                to_background()

                #stopTouchApp()
                #app.dispatch('on_pause')
                #raise SystemExit(0)

            else:
                self.root.ids["stationbutton"].state="normal"
                self.popup(_("service\nalready\nactive!"))

        else:

            self.stopservicestation()
            self.on_resume()

    def stopservicestation(self):

        if self.servicename=="station":

            print "send stop message to rpc"
            self.rpcout("stop")
            starttime= datetime.utcnow()            
            osc.readQueue(self.oscid)
            while self.rpcin_message != "stopped":
                print ">>>>> ----- rpcin message: ", self.rpcin_message
                time.sleep(.1)
                osc.readQueue(self.oscid)
                if (datetime.utcnow()-starttime) > timedelta(seconds=15) :
                    print "RPCIN timeout"
                    break
            print "if not timeout received stopped message from rpc"
            self.stop_service()
            self.rpcin_message = ""
            self.servicename=None
            os.remove("servicerunning")


    def popup(self,message,exit=False):

        # open only one notification popup (the last)
        try:
            self.mypopup.dismiss()
        except:
            pass

        box = BoxLayout(orientation='vertical')
        label = Label(text=message)
        bottone = Button(text=_("Close!"))


        box.add_widget(label)
        box.add_widget(bottone)

        self.mypopup = Popup(title=_("Warning"), content=box,size_hint=(.5, .5))
        if exit:
            bottone.bind(on_press=self.popupcloseandexit)
        else:
            bottone.bind(on_press=self.mypopup.dismiss)

        self.mypopup.open()

    def popupcloseandexit(self,*args):
        self.mypopup.dismiss()
        self.on_stop()
        raise SystemExit(0)

    def register(self):

#        if platform == 'android':
#            browser=Wv()
#            browser.open("http://rmap.cc/registrazione/register/")
#        else:
            webbrowser.open("http://rmap.cc/registrazione/register/")


    def view(self):

#        if platform == 'android':
#            browser=Wv()
#            browser.open("http://graphite.rmapv.rmap.cc/render?width=800&height=600&from=-1hours&until=now&target=rmap.*.*.*.*.105_2_-_-.*.v")
#        else:
            webbrowser.open("http://graphite.rmapv.rmap.cc/render?width=400&height=400&from=-1hours&until=now&target=rmap."+
                            self.config.get('rmap','user')+".*.*.*.*.*.v")

    def starttrip(self):
        print self.mystation.prefix
        if self.mystation.prefix != "mobile":
            self.popup(_("the station in\nuse is not of\ntype mobile"))
            self.root.ids["trip"].state="normal"
            return

        self.root.ids["gps"].state="down"

        self.trip=True
        self.mystation.trip=True

    def stoptrip(self):
        self.trip=False
        self.mystation.trip=False

    def startgps(self):
        ''' start use GPS'''

        result=self.gps.start()
        self.gps_status    = self.gps.status
        self.gps_connected = self.gps.connected
        self.gps_location  = ""
        
        if result == 1:
            self.popup(_("GPS not\nimplemented on\nthis platform"))

#        self.root.ids["locationlayout"].add_widget(Label(text=self.gps_status))


    def stopgps(self):
        ''' stop use of GPS'''
        self.gps.stop()
        self.gps_status    = self.gps.status
        self.gps_connected = self.gps.connected
        self.gps_location  = ""



    def startmqtt(self):
        '''
        begin mqtt connection
        '''

        self.mystation.startmqtt()

    def stopmqtt(self):
        '''
        disconnect from mqtt server
        '''

        self.mystation.stopmqtt()


    def updatelocation(self):
        '''
        update a new location
        '''

        self.root.ids["marker"].location(self.lat,self.lon)
        self.root.ids["markerlabel"].text= self.str_lat_lon_height % (self.location,self.lat,self.lon,self.height)

        self.root.ids["mapview"].do_update(10)
        self.root.ids["height"].text= self.str_Height.format(self.height)


    def savelocation(self,lat=None,lon=None,height=None,location=None):
        '''
        set a new location
        '''

        self.root.ids["mapview"].do_update(10)

        if location is None:
            self.location=""
        else:
            self.location=location

        if lat is None:
            self.lat=nint(self.root.ids["mapview"].lat*100000)/100000.
        else:
            self.lat=lat

        if lon is None:
            self.lon=nint(self.root.ids["mapview"].lon*100000)/100000.
        else:
            self.lon=lon

        if height is not None:
            self.height=height
        else:
            if not self.gps.gpsfix or not self.gps_connected:
                self.height=0
            else:
                self.height=self.gps.height

        self.updatelocation()

        if not self.trip:
            print "save new location in config"
            self.config.set('location', 'name',self.location)
            self.config.set('location', 'lat',self.lat)
            self.config.set('location', 'lon',self.lon)
            self.config.set('location', 'height',self.height)
            self.config.write()

            mystation=StationMetadata.objects.get(slug=self.config.get('sensors','station'))
            mystation.lat=self.lat
            mystation.lon=self.lon
            mystation.active=True
            mystation.save()

            #refresh config tabs
            self.destroy_settings()
            self.config2db(activate=True)
            try:
                rmap.rmap_core.sendjson2amqp(
                    station=self.config.get('sensors','station'),
                    user=self.config.get('rmap','user'),
                    password=self.config.get('rmap','password'),
                    host=self.config.get('rmap','server'))
            except:
                self.popup(_("data not\nsynced with server"))

            try:
                StationConstantData.objects.filter(stationmetadata=mystation,btable="B01019").delete()
            except:
                pass

            s = mystation.stationconstantdata_set.create(
                active=True,
                btable="B01019",
                value=self.config.get('location','name')
            )

            try:
                StationConstantData.objects.filter(stationmetadata=mystation,btable="B07030").delete()
            except:
                pass

            s = mystation.stationconstantdata_set.create(
                active=True,
                btable="B07030",
                value=self.config.get('location','height')
            )


        #    def map_relocated(self,lat,lon):
        #       pass


    def queue2str(self):
        maxshowqueue=20
        stringa=">> "+_("CONSTANT STATION DATA QUEUED:")+" "+str(len(self.mystation.anavarlist))+"\n"
        if (len(self.mystation.anavarlist) > maxshowqueue) :
             stringa+=">>"+_("SHOW ONLY LAST")+" "+str(maxshowqueue)+"\n"
        for item in self.mystation.anavarlist[-maxshowqueue:]:
            for bcode,data in item["anavar"].iteritems():
                stringa += str(self.table[bcode])+" {:4.5f}".format(item["coord"]["lat"])+",{:4.5f} ".format(item["coord"]["lon"])+" -> "+str(data["v"])+"\n"

        stringa+=">> "+_("STATION DATA QUEUED:")+" "+str(len(self.mystation.datavarlist))+"\n"
        if (len(self.mystation.datavarlist) > maxshowqueue) :
             stringa+=">>"+_("SHOW ONLY LAST")+" "+str(maxshowqueue)+"\n"
        for item in self.mystation.datavarlist[-maxshowqueue:]:
            for bcode,data in item["datavar"].iteritems():
                stringa += str(self.table[bcode])+" {:4.5f}".format(item["coord"]["lat"])+",{:4.5f} ".format(item["coord"]["lon"])+data["t"].strftime("%d/%m/%y %H:%M:%S")+" -> "+str(data["v"])+"\n"
        return stringa

    def queuedata(self):
        print self.root.ids
        try:
            value=float(self.root.ids["fog"].text)/10
            datavar={"B20001":{"t": datetime.utcnow(),"v": str(value)}}
            self.root.ids["fog"].text=""

            self.mystation.datavarlist.append({"coord":{"lat":self.lat,"lon":self.lon},
                                               "timerange":"254,0,0",
                                               "level":"1,-,-,-",
                                               "datavar":datavar})
        except:
            pass
            
        try:
            value=float(self.root.ids["snow"].text)*10
            datavar={"B13013":{"t": datetime.utcnow(),"v": str(value)}}
            self.root.ids["snow"].text=""
            self.mystation.datavarlist.append({"coord":{"lat":self.lat,"lon":self.lon},
                                               "timerange":"254,0,0",
                                               "level":"1,-,-,-",
                                               "datavar":datavar})
        except:
            pass


        try:
            print self.present_weather_table.value
            if self.present_weather_table.value is not None:
                value=self.present_weather_table.value
                datavar={"B20003":{"t": datetime.utcnow(),"v": str(value)}}
                #self.root.ids["snow"].text=""
                self.mystation.datavarlist.append({"coord":{"lat":self.lat,"lon":self.lon},
                                               "timerange":"254,0,0",
                                               "level":"1,-,-,-",
                                               "datavar":datavar})
        except:
            pass

        self.present_weather_widget.deselect()
        self.present_weather_table.value=None


        print self.mystation.datavarlist
        self.root.ids["queue"].text=self.queue2str()


    def cleandata(self):
         self.mystation.anavarlist=[]
         self.mystation.datavarlist=[]
         self.root.ids["queue"].text=""


#    def take_photo(self):
#        if platform == 'android':
#            camera.take_picture('photo.jpg', self.photo_done) #Take a picture and save at this location. After will call done() callback
#    
#    def photo_done(self, e): #receive e as the image location
#        #self.lblCam.text = e; #update the label to the image location
#        pass

    @mainthread
    def getdata_loop(self, *args):
        '''
        This function manage jsonrpc messages.
        '''
        print "call in getdata_loop"

        self.getdata()

        self.root.ids["queue"].text=self.queue2str()

        return True


    @mainthread
    def publishmqtt_loop(self, *args):
        '''
        This function publish mqtt messages.
        '''
        print "call in publishmqtt_loop"

        try:
            self.mystation.publishmqtt_loop()

        except:
            print "error in publishmqtt_loop"

        self.root.ids["queue"].text=self.queue2str()

        self.mqtt_connected = self.mystation.rmap.connected
        self.mqtt_status = self.mystation.mqtt_status

        return True


    @mainthread
    def on_location(self, **kwargs):
        '''
        callback for new GPS location
        '''

        lat=kwargs["lat"]
        lon=kwargs["lon"]
        height=kwargs["height"]

        self.gps_location = _("GPS: new coordinate acquired")
        self.root.ids["mapview"].center_on(lat,lon)
        self.root.ids["height"].text= self.str_Height.format(height)

        if self.trip and kwargs["gpsfix"]:
            self.savelocation(lat=lat,lon=lon,height=height)


    @mainthread
    def on_status(self, status, gpsfix):
        '''
        callback for new GPS status
        '''

        self.gps_status = status
        self.gpsfix=gpsfix

        #mmm take alook at 
        #http://stackoverflow.com/questions/2021176/how-can-i-check-the-current-status-of-the-gps-receiver


