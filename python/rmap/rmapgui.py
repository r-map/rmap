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
from kivy.uix.settings import SettingOptions #, SettingString, SettingItem
from kivy.uix.settings import Settings
#from kivy.uix.settings import SettingsWithSidebar #, SettingsWithSpinner, SettingsWithTabbedPanel, SettingsWithNoMenu
from kivy.uix.image import Image
from kivy.uix.scrollview import ScrollView
from kivy.metrics import dp
from kivy.uix.togglebutton import ToggleButton
from kivy.uix.settings import SettingSpacer
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
import traceback
from glob import glob
from utils import nint
import rmap.rmap_core
from rmap import exifutils
import rmap.settings

platform = platform()

PHOTOIMAGE="photo.jpg"
QUEUEDIMAGES="queuedphoto_*.jpg"
def queuednewfilename():
    i = 0
    while os.path.exists("queuedphoto_%3.3d.jpg" % i):
        print "search for new filename; found file: queuedphoto_%3.3d.jpg" % i
        i += 1
    print "new filename: queuedphoto_%3.3d.jpg" % i
    return "queuedphoto_%3.3d.jpg" % i

def queuedfilename():
    files = glob(QUEUEDIMAGES)
    try:
        sfiles=sorted(files)
        print "found queued files:", sfiles
        return sfiles[0]
    except Exception as e:
        print e
        return None

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

template_default=rmap.rmap_core.template_choices[0]
#from kivy.uix.camera import Camera

if platform == 'android':
    from plyer import camera #object to read the camera

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
                    on_release: app.register() 

                Button:
                    text: app.str_View_graph
                    on_release: app.view() 


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

            Toolbar:
                Label:
                    id: stationstatus
                    text: "Station status: unknown"
                    color:  1., .5, .5, 1.
                    bold: True


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
                    on_release: app.savelocation()



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
                    text: app.str_Camera

                    BoxLayout:
                        orientation: 'vertical'
                        canvas:
                            Color:
                                rgba: 50/255., 50/255., 100/255., 1
                            Rectangle:
                                pos: self.pos
                                size: self.size

                        BoxLayout:
                            orientation: 'horizontal'
                            size_hint_y: None
                            height: '40dp'
                            #Button:
                            #    text: app.str_start
                            #    on_release: app.camera_start()

                            #Button:
                            #    text: app.str_Stop
                            #    on_release: app.camera_stop()

                            # android only button
                            Button:
                                text: app.str_Take_Photo
                                on_release: app.camera_take_photo()

                        BoxLayout:
                            orientation: 'horizontal'
                            size_hint_y: None
                            height: '40dp'


                            TextInput:
                                id: cameracomment
                                text: app.str_Comment_Photo
                                multiline: False
                                #on_text_validate: self.validatetext()

                        #Camera:
                        #    id: mycamera
                        #    #resolution: 399, 299

                        CameraImage:
                            id: cameraimage
                            #center: self.parent.center
                            #center: mapview.get_window_xy_from(mapview.lat, mapview.lon, mapview.zoom)

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
                    on_release: app.queuedata() 


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
                    on_release: app.configureboard()
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
                ToggleButton:
                    id: relay_0
                    background_normal: ""
                    background_color: .5,1.,.5,1.
                    text: app.str_On_relay+" 1" if self.state == 'normal' else app.str_Off_relay+" 1"
                    on_state: app.togglepin(4,True) if self.state == 'down' else app.togglepin(4,False)

            Toolbar:
                ToggleButton:
                    id: relay_1
                    background_normal: ""
                    background_color: .5,1.,.5,1.
                    text: app.str_On_relay+" 2" if self.state == 'normal' else app.str_Off_relay+" 2"
                    on_state: app.togglepin(5,True) if self.state == 'down' else app.togglepin(5,False)

            Toolbar:
                ToggleButton:
                    id: relay_2
                    background_normal: ""
                    background_color: .5,1.,.5,1.
                    text: app.str_On_relay+" 3" if self.state == 'normal' else app.str_Off_relay+" 3"
                    on_state: app.togglepin(30,True) if self.state == 'down' else app.togglepin(30,False)

            Toolbar:
                ToggleButton:
                    id: relay_3
                    background_normal: ""
                    background_color: .5,1.,.5,1.
                    text: app.str_On_relay+" 4" if self.state == 'normal' else app.str_Off_relay+" 4"
                    on_state: app.togglepin(31,True) if self.state == 'down' else app.togglepin(31,False)

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
                    on_release: app.cleandata() 

                ToggleButton:
                    id: connect
                    text: app.str_Connect if self.state == 'normal' else app.str_Disconnect
                    on_state: app.queueon() if self.state == 'down' else app.queueoff()

#                Button:
#                    text: app.str_Publish
#                    on_release: app.publishmqtt() 

            BoxLayout:
                id: queued
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

            BoxLayout:
                id: queuedimagebox

                #ScrollView:
                #    bar_width: 10
                #    BoxLayout:

#                    QueuedImage:
#                        id: queuedimage

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

'''

#https://github.com/kivy/kivy/wiki/Scollable-Options-in-Settings-panel
class SettingScrollOptions(SettingOptions):

    def _create_popup(self, instance):

        #global oORCA
        # create the popup

        content         = GridLayout(cols=1, spacing='5dp')
        scrollview      = ScrollView( do_scroll_x=False)
        scrollcontent   = GridLayout(cols=1,  spacing='5dp', size_hint=(None, None))
        scrollcontent.bind(minimum_height=scrollcontent.setter('height'))
        self.popup   = popup = Popup(content=content, title=self.title, size_hint=(0.8, 0.9),  auto_dismiss=False)

        #we need to open the popup first to get the metrics 
        popup.open()
        #Add some space on top
        content.add_widget(Widget(size_hint_y=None, height=dp(2)))
        # add all the options
        uid = str(self.uid)
        for option in self.options:
            state = 'down' if option == self.value else 'normal'
            btn = ToggleButton(text=option, state=state, group=uid, size=(popup.width, dp(55)), size_hint=(None, None))
            btn.bind(on_release=self._set_option)
            scrollcontent.add_widget(btn)

        # finally, add a cancel button to return on the previous panel
        scrollview.add_widget(scrollcontent)
        content.add_widget(scrollview)
        content.add_widget(SettingSpacer())
        #btn = Button(text='Cancel', size=((oORCA.iAppWidth/2)-sp(25), dp(50)),size_hint=(None, None))
        btn = Button(text='Cancel', size=(popup.width, dp(50)),size_hint=(0.9, None))
        btn.bind(on_release=popup.dismiss)
        content.add_widget(btn)

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



class CameraImage(Image):

    def __init__(self, **kwargs):
        super(CameraImage, self).__init__(**kwargs)
        if os.path.isfile(PHOTOIMAGE):
            self.source = PHOTOIMAGE
        else:
            self.source = os.path.join(os.path.dirname(__file__), "icons", "noimage.png")


class QueuedImage(Image):

    def __init__(self,queuedimage=None, **kwargs):
        super(QueuedImage, self).__init__(**kwargs)

        if queuedimage is None:
            queuedimage=queuedfilename()
        if (not queuedimage is None):
            #if os.path.isfile(queued):
            self.source = queuedimage
        else:
            self.source = os.path.join(os.path.dirname(__file__), "icons", "noimage.png")

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

#class SettingPassword(SettingString):
#    def add_widget(self, *largs):
#
#        if self.content is None:
#            return super(SettingItem, self).add_widget(*myargs)
#        return self.content.add_widget(*myargs)


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
    #settings_cls=SettingsWithSidebar
    #settings_cls=SettingsWithSpinner
    #settings_cls=SettingsWithTabbedPanel
    #settings_cls=SettingsWithNoMenu

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

        ##########################
        # reset
        ##########################
        #os.remove("rmap.sqlite3")
        ##########################

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

        self.username=self.config.get('rmap','user')
        print "updateusername"
        if rmap.rmap_core.updateusername(newusername=self.username) > 0:
            print "out of sync"
            print "sync data from config to db" # and preserve active status ?
            self.config2db()

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
        self.str_Next=_("Subsequent")
        self.str_On_relay=_("switch ON relay")
        self.str_Off_relay=_("switch OFF relay")
        self.str_Camera=_("Camera")
        self.str_Comment_Photo= _("Comment your photo")
        self.str_Take_Photo=_("Take Photo")
        self.str_Start_GPS=_("Start GPS") 
        self.str_Stop=_("Stop")
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

            print "trip=",self.trip\
                        ,"username=",self.config.get('rmap','user')
            print "station=",self.config.get('sensors','station')\
                        ,"board=",self.config.get('sensors','board')\
                        ,"template=",self.config.get('sensors','template')\
                        ,"remote board=",self.config.get('sensors','remote_board')\
                        ,"template=",self.config.get('sensors','remote_template')


            self.mystation=rmapstation.station(trip=self.trip,gps=self.gps,
                            slug=self.config.get('sensors','station'),
                            username=self.config.get('rmap','user'),
                            boardslug=self.config.get('sensors','board'),
                            logfunc=jsonrpc.log_stdout)

            #self.config2db(activate=self.mystation.active)
            #rmap.rmap_core.activatestation(username=self.config.get('rmap','user'),
            #                     station=self.config.get('rmap','user'),
            #                     activate=self.mystation.active)

            #self.mystation=rmapstation.station(trip=self.trip,gps=self.gps,
            #                slug=self.config.get('sensors','station'),
            #                username=self.config.get('rmap','user'),
            #                boardslug=self.config.get('sensors','board'),
            #                logfunc=jsonrpc.log_stdout)

               
            #except rmap.stations.models.DoesNotExist:
        except:
            #try:
            #    print "retry with default and without username; this happen when DB was modified and is not in sync with config"
            #    self.config2db()
            #    self.mystation=rmapstation.station(trip=self.trip,gps=self.gps,
            #                    slug=self.config.get('sensors','station'),
            #                    boardslug=self.config.get('sensors','board'),
            #                    logfunc=jsonrpc.log_stdout)

            #except:

            print "restart everithings with default"
            self.resettodefault()

        self.stationstatus()

        # add listview widget
        lang=self.config.get('general','language')
        self.present_weather_table = values(os.path.join(os.path.dirname(__file__), "tables","present_weather_"+lang+".txt"))
        self.present_weather_widget=PresentwView(self.present_weather_table)
        self.root.ids["presentwtab"].add_widget(self.present_weather_widget)

        self.root.ids["queue"].text=self.queue2str()

        queuedimage=queuedfilename()
        if (not queuedimage is None):
            #self.root.ids["queuedimage"].source= queuedimage
            for file in sorted(glob(QUEUEDIMAGES)):
                if os.path.isfile(file):
                    image=QueuedImage(queuedimage=file)
                    self.root.ids["queuedimagebox"].add_widget(image)
        else:
            #add noimage in queue
            image=QueuedImage(queuedimage=os.path.join(os.path.dirname(__file__), "icons", "noimage.png"))
            self.root.ids["queuedimagebox"].add_widget(image)

        if self.mystation.active:

            try:
                rmap.rmap_core.sendjson2amqp(
                    station=self.config.get('sensors','station'),
                    user=self.config.get('rmap','user'),
                    password=self.config.get('rmap','password'),
                    host=self.config.get('rmap','server'))
            except Exception as e:
                print e
                print "WARNING: data not synced with server"
                traceback.print_exc()
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
            bottone1.bind(on_release=stopstation)
            bottone2.bind(on_release=to_background)

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



    def close_settings(self,settings):
        """ The settings panel has been closed. """
        print "Setting was closed"
        self.questionactivatestation()
        super(Rmap, self).close_settings(settings)


    def stationstatus(self):
        try:
            if self.mystation.active:
                self.root.ids["stationstatus"].text= _("Station")+": "+_(" active")
            else:
                self.root.ids["stationstatus"].text= _("Station")+": "+_(" disactive")
        except:
                self.root.ids["stationstatus"].text= _("Station")+": "+_(" disactive")
            
    def questionactivatestation(self):
        box = BoxLayout(orientation='vertical')
        label = Label(text=_("Activate Station?"))
        bottone1 = Button(text=_("Yes"))
        bottone2 = Button(text=_("No"))

        box.add_widget(label)
        box.add_widget(bottone1)
        box.add_widget(bottone2)

        self.questionpopup = Popup(title=_("Question"), content=box,size_hint=(.5, .5))
   
        bottone1.bind(on_release=self.activatestation)
        bottone2.bind(on_release=self.disablestation)

        self.questionpopup.open()


    def activatestation(self,*args):
        #activate station
        print "activate station"


        connected=self.mqtt_connected
        if connected:
            print "disconnect MQTT with old parameter"
            self.stopmqtt()

        self.config2db(activate=True)
        #rmap.rmap_core.activatestation(username=self.config.get('rmap','user'),
        #                     station=self.config.get('sensors','station'),
        #                     board=self.config.get('sensors','board'),
        #                     activate=True)

        try:
            self.mystation=rmapstation.station(
                trip=self.trip,gps=self.gps,
                slug=self.config.get('sensors','station'),
                username=self.config.get('rmap','user'),
                boardslug=self.config.get('sensors','board'),
                logfunc=jsonrpc.log_stdout)

            self.stationstatus()

        except Exception as e:
            print e
            print "ERROR recreating rmapstaton.station"

        if connected:
            print "reconnect MQTT with new parameter"
            self.startmqtt()

        self.questionpopup.dismiss()

        if self.mystation.active:
            try:
                rmap.rmap_core.sendjson2amqp(
                    station=self.config.get('sensors','station'),
                    user=self.config.get('rmap','user'),
                    password=self.config.get('rmap','password'),
                    host=self.config.get('rmap','server'))
            except Exception as e:
                print e
                print "WARNING: data not synced with server"
                traceback.print_exc()
                self.popup(_("data not\nsynced with server"))

    def disablestation(self,*args):
        #none station
        print "disable station"
        #self.config2db(activate=False)
        rmap.rmap_core.activatestation(username=self.config.get('rmap','user'),
                             station=self.config.get('sensors','station'),
                             board=self.config.get('sensors','board'),
                             activate=False)

        self.mystation=rmapstation.station(
            trip=self.trip,gps=self.gps,
            slug=self.config.get('sensors','station'),
            username=self.config.get('rmap','user'),
            boardslug=self.config.get('sensors','board'),
            logfunc=jsonrpc.log_stdout)

        self.stationstatus()
        self.questionpopup.dismiss()

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
                print "updateusername"
                rmap.rmap_core.updateusername(oldusername=self.username,newusername=value)
                self.username=value
                rmapchanged = True
            elif token == ('rmap', 'password'):
                print('password have been changed to', value)
                rmap.rmap_core.updateusername(oldusername=self.username,newusername=self.username,newpassword=value)
                rmapchanged = True
            elif token == ('rmap', 'samplerate'):
                print('samplerate have been changed to', value)
                rmapchanged = True
            elif token == ('location', 'name'):
                print('location name have been changed to', value)
                locationchanged = True
            elif token == ('location', 'mobile'):
                print('location mobile have been changed to', value)
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
                mystation=StationMetadata.objects.get(slug=self.config.get('sensors','station'),ident__username=self.config.get('rmap','user'))

                self.stationstatus()
                try:
                    board=mystation.board_set.all()[0]
                    config.set('sensors', 'board', str(board.slug))
                    config.set('sensors', 'remote_board', str(board.slug))
                except:
                    print "No board and remote board for:", mystation
                    config.set('sensors', 'board',None)
                    config.set('sensors', 'remote_board', None)

                sensorschanged = True

            elif token == ('sensors', 'board'):
                print('sensors board have been changed to', value)
                sensorschanged = True

            elif token == ('sensors', 'template'):
                print('sensors template have been changed to', value)
                sensorschanged = True

            elif token == ('sensors', 'remote_template'):
                print('sensors remote_template have been changed to', value)
                #rmap.rmap_core.addsensors_by_template(
                #    station_slug=self.config.get('sensors','station')
                #    ,username=self.config.get('rmap','user')
                #    ,board_slug=self.config.get('sensors','remote_board')
                #    ,template=self.config.get('sensors','remote_template'))
                sensorschanged = True


            if locationchanged:
                print "update location with new parameter"

                ##self.config2db(activate=True)
                #rmap.rmap_core.activatestation(username=self.config.get('rmap','user'),
                #             station=self.config.get('sensors','station'),
                #             board=self.config.get('sensors','board'),
                #                               activate=True,activateboard=True)

                self.location=self.config.get('location','name')
                self.lon=float(self.config.get('location','lon'))
                self.lat=float(self.config.get('location','lat'))
                self.height=float(self.config.get('location','height'))

                self.config2db()
                self.mystation=rmapstation.station(trip=self.trip,gps=self.gps,
                            slug=self.config.get('sensors','station'),
                            username=self.config.get('rmap','user'),
                            boardslug=self.config.get('sensors','board'),
                            logfunc=jsonrpc.log_stdout)


            if rmapchanged or locationchanged or sensorschanged:

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
                    super(Rmap, self).close_settings()
                    self.destroy_settings()
                    self.open_settings()


                #self.config2db()
                #rmap.rmap_core.addsensors_by_template(
                #    station_slug=self.config.get('sensors','station')
                #    ,username=self.config.get('rmap','user')
                #    ,board_slug=self.config.get('sensors','board')
                #    ,template=self.config.get('sensors','template'))


            if languagechanged:
                self.popup(_("Restart APP\nneeded"),exit=True)



    def config2db(self,activate=None):

        try:

            mqttrootpath=rmap.settings.topicsample
            mqttmaintpath=rmap.settings.topicsample

            if (self.config.get('location','mobile') == '0'):
                network="fixed"
            else:
                network="mobile"

            rmap.rmap_core.configdb(
                username=self.config.get('rmap','user'),
                password=self.config.get('rmap','password'),
                station=self.config.get('sensors','station'),
                lat=self.config.get('location','lat'),lon=self.config.get('location','lon'),
                constantdata={"B01019":self.config.get('location','name'),
                              "B07030":self.config.get('location','height')},
                network=network,
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
                mqttrootpath=mqttrootpath,
                mqttmaintpath=mqttmaintpath,
                board=self.config.get('sensors','board'),
                activate=activate)

            rmap.rmap_core.addsensors_by_template(
                station_slug=self.config.get('sensors','station')
                ,username=self.config.get('rmap','user')
                ,board_slug=self.config.get('sensors','board')
                ,template=self.config.get('sensors','template'))

            rmap.rmap_core.addsensors_by_template(
                station_slug=self.config.get('sensors','station')
                ,username=self.config.get('rmap','user')
                ,board_slug=self.config.get('sensors','remote_board')
                ,template=self.config.get('sensors','remote_template'))

        except Exception as e:
            print e
            traceback.print_exc()
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

        config.setdefaults('sensors', {
            'name': 'HC-05',
            'station': station_default,
            'board': board_default,
            'template': template_default,
            'remote_board': "stima_bt",
            'remote_template': template_default
        })

        config.setdefaults('location', {
            'name': 'home',
            'mobile': 0,
            'lat': 0.,
            'lon': 0.,
            'height': 0.
        })


    def build_settings(self, settings):
        '''
        define the setting panel
        '''
        settings.register_type('scrolloptions', SettingScrollOptions)
        #settings.register_type('password', SettingPassword)

        stations=[]
        #for station in StationMetadata.objects.filter(active=True):
        for station in StationMetadata.objects.all():
            stations.append(str(station.slug))
        try:
            mystation=StationMetadata.objects.get(slug=self.config.get('sensors','station'),ident__username=self.config.get('rmap','user'))
        except:
            self.resettodefault()

        mystation=StationMetadata.objects.get(slug=self.config.get('sensors','station'),ident__username=self.config.get('rmap','user'))

        self.stationstatus()

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
    { "type": "scrolloptions",
      "title": "Template",
      "desc": "Sensor template",
      "section": "sensors",
      "key": "template",
      "options": 
        """ + str(rmap.rmap_core.template_choices).replace("'","\"") + """
        },
    { "type": "options",
      "title": "Remote Board",
      "desc": "remote board name",
      "section": "sensors",
      "key": "remote_board",
      "options": 
        """ + str(boards).replace("'","\"") + """
        },
    { "type": "scrolloptions",
      "title": "Remote Template",
      "desc": "Remote Sensor template",
      "section": "sensors",
      "key": "remote_template",
      "options": 
        """ + str(rmap.rmap_core.template_choices).replace("'","\"") + """
        }
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
    { "type": "bool",
      "title": "Mobile",
      "desc": "Station can go traveling",
      "section": "location",
      "key": "mobile"},
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

        settings.add_json_panel('General',
                                self.config, data=jsongeneral)

        settings.add_json_panel('Rmap',
                                self.config, data=jsonrmap)

        settings.add_json_panel('Sensors',
                                self.config, data=jsonsensors)

        settings.add_json_panel('Location',
                                self.config, data=jsonlocation)


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


    def togglepin(self,n,status):

        self.root.ids["transport"].state="down"

        try:
            rpcproxy = jsonrpc.ServerProxy( jsonrpc.JsonRpc20(),self.mystation.transport)
            rpcproxy.togglepin({"n":n,"s":status})
        except:
            self.popup(_("toggle\nrelay\nfailed!"))
            

    def resettodefault(self):

        try:
            print "restart everithings with default"
            self.config.set('sensors', 'station',station_default)
            self.config.set('sensors', 'board',board_default)
            self.config.set('sensors', 'template',template_default)
            self.config.set('sensors', 'remote_template',template_default)
            self.config.write()

            self.config2db()
            rmap.rmap_core.addsensors_by_template(
                station_slug=self.config.get('sensors','station')
                ,username=self.config.get('rmap','user')
                ,board_slug=self.config.get('sensors','board')
                ,template=self.config.get('sensors','template'))
            rmap.rmap_core.addsensors_by_template(
                station_slug=self.config.get('sensors','station')
                ,username=self.config.get('rmap','user')
                ,board_slug=self.config.get('sensors','remote_board')
                ,template=self.config.get('sensors','remote_template'))

            self.mystation=rmapstation.station(trip=self.trip,gps=self.gps,
                                               slug=self.config.get('sensors','station'),
                                               boardslug=self.config.get('sensors','board'),
                                               logfunc=jsonrpc.log_stdout)
            
        except Exception as e:
            print e
            print "ERROR: cannot get a good station from DB !"
            print "WARNING: data not synced with server"
            traceback.print_exc()
            #raise SystemExit(0)
            raise


    def configureboard(self):

        try:
            # this stop transport if active (configure restart transport and stop it at the end)
            self.root.ids["transport"].state="normal"
            self.board_status='Transport Status: OFF'
            try:
                self.stoptransport()
            except:
                pass

            #self.config2db(activate=True,board=self.config.get('sensors','remote_board'))
            rmap.rmap_core.activatestation(username=self.config.get('rmap','user'),
                             station=self.config.get('sensors','station'),
                             board=self.config.get('sensors','board'),
                             activateboard=True)

            self.mystation.configurestation(board_slug=self.config.get('sensors','remote_board')
                                            ,username=self.config.get('rmap','user'))
            self.board_status=_("Transport Status: CONFIG OK")

            #try:
            #    rmap.rmap_core.sendjson2amqp(
            #        station=self.config.get('sensors','station'),
            #        user=self.config.get('rmap','user'),
            #        password=self.config.get('rmap','password'),
            #        host=self.config.get('rmap','server'))
            #except Exception as e:
            #    print e
            #    print "WARNING: data not synced with server"
            #    traceback.print_exc()
            #    self.popup(_("data not\nsynced with server"))

            #self.mystation.stoptransport()
        except Exception as e:
            print e
            print "ERROR configure board"
            traceback.print_exc()
            self.board_status=_("Transport Status: CONFIG ERROR")
            self.popup(_("ERROR configure\nboard"))




    def getdata(self):

        self.root.ids["transport"].state="down"

        if self.mystation.ismobile() and ( not self.trip or not self.gps.gpsfix):
            self.popup(_("travel with\nGPS not fixed!\nretry"))
            self.getdataerror+=1
            self.board_status=_("Transport Status: ERROR")+_(" err: ")+ str(self.getdataerror)
            return
        
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
        
        else:
            if not datavars:
                print "ERROR executing getdata: no data returned"

                self.popup(_("ERROR no data\nreturned"))
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

        if self.mystation.active:
            self.startmqtt()
            self.myqueue_loop=self.publishmqtt_loop
            Clock.schedule_interval(self.myqueue_loop, 5.)

            self.myphoto_loop=self.publishphoto_loop
            Clock.schedule_once(self.myphoto_loop)
            Clock.schedule_interval(self.myphoto_loop, 180.)
        else:
            self.popup(_("Cannot connect.\nStation disabled!"))
            self.root.ids["connect"].state="normal"

    def queueoff(self):
        if self.mystation.active:
            Clock.unschedule(self.myqueue_loop)
            Clock.unschedule(self.myphoto_loop)
            self.stopmqtt()
            #update to last status
            self.mqtt_status = self.mystation.mqtt_status
            try:
                self.mqtt_connected = self.mystation.rmap.connected
            except AttributeError:
                pass
        else:
            self.mqtt_connected = False
            #self.mqtt_status = _('Connect Status: disconnected')
            self.mqtt_status = _("Station")+": "+_(" disactive")



    def rpcin(self, message, *args):
        print "RPC: ",message[2]
        self.rpcin_message=message[2]

    #def rpcout(self, *args):
    #    osc.sendMsg('/rpc', ["testinout",], port=3000)

    def rpcout(self, message):
        osc.sendMsg('/rpc', [message,], port=3000)



    def servicewebserver(self):

        if self.root.ids["webserverbutton"].state == "down":
            if self.service is None:

                if platform == 'linux':
                    import subprocess
                    #self.service=subprocess.Popen(["python", "rmapmanage", "runserver","8888"], 
                    self.service=subprocess.Popen(["./rmapweb"], 
                                    stderr=subprocess.STDOUT)
                    #import os
                    #os.spawnl(os.P_NOWAIT, "python manage.py runserver")


                elif platform == 'win':
                    import subprocess
                    #self.service=subprocess.Popen(["python", "rmapmanage", "runserver","8888"], 
                    self.service=subprocess.Popen(["rmapweb.bat"], 
                                    stderr=subprocess.STDOUT)

                elif platform == 'android':

                    self.start_service("webserver")
                    self.servicename="webserver"

                time.sleep(6)
                webbrowser.open("http://"+rmap.settings.port+"/admin/")

            else:
                self.root.ids["webserverbutton"].state="normal"
                self.popup(_("service\nalready\nactive!"))

        else:

            if platform != 'android':
                self.service.kill()
                #self.popup(_("not supported\non this\nplatform!"))
                self.service=None
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
            bottone.bind(on_release=self.popupcloseandexit)
        else:
            bottone.bind(on_release=self.mypopup.dismiss)

        self.mypopup.open()

    def notify(self,message):

        # open only one notification popup (the last)
        self.notifydismiss()

        box = BoxLayout(orientation='vertical')
        label = Label(text=message)

        box.add_widget(label)

        self.mynotify = Popup(title=_("Info"), content=box,size_hint=(.5, .5))
        self.mynotify.open()

    def notifydismiss(self):
        try:
            self.mynotify.dismiss()
        except:
            pass

    def popupcloseandexit(self,*args):
        self.mypopup.dismiss()
        self.on_stop()
        raise SystemExit(0)

    def register(self):

#        if platform == 'android':
#            browser=Wv()
#            browser.open("http://rmap.cc/registrazione/register/")
#        else:
            webbrowser.open("http://"+self.config.get('rmap','server')+"/registrazione/register/")


    def view(self):

#        if platform == 'android':
#            browser=Wv()
#            browser.open("http://graphite.rmapv.rmap.cc/render?width=800&height=600&from=-1hours&until=now&target=rmap.*.*.*.*.105_2_-_-.*.v")
#        else:
            #webbrowser.open("http://localhost:8000/stations/"+self.config.get('sensors','station'))
            webbrowser.open("http://"+self.config.get('rmap','server')+"/stations/"+self.config.get('rmap','user')+"/"+self.config.get('sensors','station').split("/")[0])

    def starttrip(self):
        print "network: ",self.mystation.network
        if not self.mystation.ismobile():
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
        height=self.height
        if (height is None):
            height=0
        self.root.ids["height"].text= self.str_Height.format(height)


    def movelocation(self,lat=None,lon=None,height=None,location=None):
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


    def savelocation(self):


        if self.trip:
            self.popup(_("travel active\ncannot save\nretry"))

        else:
            print "save new location in config"

            self.movelocation()

            self.config.set('location', 'name',self.location)
            self.config.set('location', 'lat',self.lat)
            self.config.set('location', 'lon',self.lon)
            self.config.set('location', 'height',self.height)
            self.config.write()

            #refresh config tabs
            self.destroy_settings()
            mystation=StationMetadata.objects.get(slug=self.config.get('sensors','station'),ident__username=self.config.get('rmap','user'))

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


            self.questionactivatestation()


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

        if self.mystation.ismobile() and ( not self.trip or not self.gps.gpsfix):
            self.popup(_("travel with\nGPS not fixed!\nretry"))
            return

        try:
            value=int(self.root.ids["fog"].text)/10
            datavar={"B20001":{"t": datetime.utcnow(),"v": str(value)}}
            self.root.ids["fog"].text=""

            self.mystation.datavarlist.append({"coord":{"lat":self.lat,"lon":self.lon},
                                               "timerange":"254,0,0",
                                               "level":"1,-,-,-",
                                               "datavar":datavar,
                                               "prefix":rmap.settings.topicreport})
        except:
            pass
            
        try:
            value=int(self.root.ids["snow"].text)*10
            datavar={"B13013":{"t": datetime.utcnow(),"v": str(value)}}
            self.root.ids["snow"].text=""
            self.mystation.datavarlist.append({"coord":{"lat":self.lat,"lon":self.lon},
                                               "timerange":"254,0,0",
                                               "level":"1,-,-,-",
                                               "datavar":datavar,
                                               "prefix":rmap.settings.topicreport})
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
                                                   "datavar":datavar,
                                                   "prefix":rmap.settings.topicreport})
                                                   
        except:
            pass

        self.present_weather_widget.deselect()
        self.present_weather_table.value=None


        print self.mystation.datavarlist
        self.root.ids["queue"].text=self.queue2str()

        if os.path.isfile(PHOTOIMAGE):
            try:

                #QUEUEDIMAGE=queuedfilename()
                ##if os.path.isfile(QUEUEDIMAGE):
                #if (not QUEUEDIMAGE is None):
                #    os.remove(QUEUEDIMAGE)

                # queue photo for the server

#                import pexif
#                # Add exif in a file
#                img = pexif.JpegFile.fromFile(PHOTOIMAGE)
#                #img = pexif.JpegFile.fromString(body)
#                exif = img.get_exif()
#                if exif:
#                    primary = exif.get_primary()
#                if not exif is None or not primary is None:
#
#                    print ">",self.root.ids["cameracomment"].text,"<"
#                    primary.ImageDescription =str(self.config.get('rmap','user'))
#                    primary.ExtendedEXIF.UserComment = chr(0x55)+chr(0x4E)+chr(0x49)+chr(0x43)+chr(0x4F)+chr(0x44)+chr(0x45)+chr(0x00)+str(self.root.ids["cameracomment"].text)
#                    #embedded = img.exif.primary.__getattr__("ExtendedEXIF")
#                    #embedded["UserComment"] = chr(0x55)+chr(0x4E)+chr(0x49)+chr(0x43)+chr(0x4F)+chr(0x44)+chr(0x45)+chr(0x00)+self.root.ids["cameracomment"].text
#                    img.set_geo(float(self.lat), float(self.lon))
#
#                    try:
#                        print primary.DateTime
#                    except:
#                        print "DateTime not present"
#
#                    #datetime
#                    primary.DateTime=datetime.utcnow().strftime("%Y:%m:%d %H:%M:%S")
#                    print primary.DateTime
#                    #embedded = img.exif.primary.__getattr__("ExtendedEXIF")
#                    #if embedded["DateTime"]:
#                    #    embedded["DateTime"] = datetime.utcnow().strftime("%Y:%m:%d %H:%M:%S")
#                    #try:
#                    #    os.remove(QUEUEDIMAGE)
#                    #except:
#                    #    pass

#                    newfilename=queuednewfilename()
##                    img.writeFile(newfilename)

                ident=str(self.config.get('rmap','user'))
                comment=self.root.ids["cameracomment"].text
                lat=float(self.lat)
                lon=float(self.lon)

                with open(PHOTOIMAGE,"rb") as pi:
                    data=pi.read()

                data=exifutils.setgeoimage(data,lat,lon,imagedescription=ident,usercomment=comment)

                with open(queuednewfilename(),"wb") as pi:
                    pi.write(data)

                os.remove(PHOTOIMAGE)

                self.root.ids["queuedimagebox"].clear_widgets()

                queuedimage=queuedfilename()
                if (not queuedimage is None):
                    #self.root.ids["queuedimage"].source= queuedimage
                    for file in sorted(glob(QUEUEDIMAGES)):
                        if os.path.isfile(file):
                            image=QueuedImage(queuedimage=file)
                            self.root.ids["queuedimagebox"].add_widget(image)

                #self.root.ids["queuedimage"].reload()
                self.root.ids["cameraimage"].source= os.path.join(os.path.dirname(__file__), "icons", "noimage.png")
                self.root.ids["cameraimage"].reload()


            except Exception as e:
                print e
                print "WARNING: photo not queued"
                traceback.print_exc()
                self.popup(_("problems with\nCamera!"))
                #os.rename(PHOTOIMAGE,QUEUEDIMAGE)


    def cleandata(self):
         self.mystation.anavarlist=[]
         self.mystation.datavarlist=[]
         self.root.ids["queue"].text=""

         # remove queued image
         for file in glob(QUEUEDIMAGES):
             if os.path.isfile(file):
                 os.remove(file)

         self.root.ids["queuedimagebox"].clear_widgets()
         image=QueuedImage(queuedimage=os.path.join(os.path.dirname(__file__), "icons", "noimage.png"))
         self.root.ids["queuedimagebox"].add_widget(image)

#         self.root.ids["queuedimage"].source= os.path.join(os.path.dirname(__file__), "icons", "noimage.png")
#         self.root.ids["queuedimage"].reload()

#    def camera_start(self):
#        if platform == 'linux':
#            self.root.ids["mycamera"].resolution=(399, 299)
#            self.root.ids["mycamera"].play= True
#
#    def camera_stop(self):
#        if platform == 'linux':
#            self.root.ids["mycamera"].play= False

    def camera_take_photo(self):
        if platform == 'android':

            self.notify(_("Wait"))

            from jnius import autoclass

            Environment = autoclass('android.os.Environment')
            #dir=Environment.getExternalStorageDirectory().getPath()
            dir=Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES).getPath()

            #PythonActivity = autoclass('org.renpy.android.PythonActivity')
            #activity = PythonActivity.mActivity
            #dir =  activity.getCacheDir().getPath()

            # simple queued file names
            #index=0
            #while True:
            #    index += 1
            #    fn = (Environment.getExternalStorageDirectory().getPath() +
            #          '/takepicture{}.jpg'.format(index))
            #    if not os.path.exists(fn):
            #        break

            try:
                os.makedirs(dir)
            except:
                print "error makedirs:",dir

            fn = dir + '/rmap_picture.jpg'
            if os.path.isfile(fn):
                os.remove(fn)

            #Take a picture and save at this location. After will call on_complete() callback
            camera.take_picture(filename=fn, on_complete=self.photo_done)

        else:
            #self.camera_start()
            #while self.root.ids["mycamera"].texture is None:
            #    print "wait"
            #    time.sleep(1)

            try:
                if self.root.ids["mycamera"].texture is None:
                    self.popup(_("Start Camera!"))
                    return
            except:
                self.popup(_("not supported\non this\nplatform!"))
                return

            self.root.ids["mycamera"].texture.save(filename=PHOTOIMAGE,flipped=False)
            self.camera_stop()
            
        # TODO work more on timestamp


    def photo_done(self, filename): #receive filename as the image location
        print "photo is in: ",filename

        import shutil
        shutil.copyfile(filename, PHOTOIMAGE)
        #os.rename(filename,PHOTOIMAGE)

        ##MediaScanner
        #MediaScannerConnection = autoclass('android.media.MediaScannerConnection')
        #PythonActivity = autoclass('org.renpy.android.PythonActivity')
        #activity = cast('android.app.Activity', PythonActivity.mActivity)
        #context = activity.getApplicationContext()
        #MediaScannerConnection.scanFile(context,filename,None,None)   # filename is a list of absolute paths of files on Android

        # do not ask me why, but without schedule_once the file is not accessible
        Clock.schedule_once(self.photo_show,5)

        # return true unlink the file (not shure if it's a good think)
        return True



    def photo_show(self,*largs):

        #with open(PHOTOIMAGE) as file:
        #    data = file.read()
        #    print "Input metadata:"
        #    exifutils.dumpimage(data)

        exifutils.photo_manage(PHOTOIMAGE)

        #with open(PHOTOIMAGE) as file:
        #    data = file.read()
        #    print "Output metadata:"
        #    exifutils.dumpimage(data)

        self.root.ids["cameraimage"].source= PHOTOIMAGE
        self.root.ids["cameraimage"].reload()
        self.notifydismiss()

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

        if self.mystation.active:
            try:
                self.mystation.publishmqtt_loop()

            except:
                print "error in publishmqtt_loop"

            self.root.ids["queue"].text=self.queue2str()

            try:
                self.mqtt_connected = self.mystation.rmap.connected
            except AttributeError:
                pass
            self.mqtt_status = self.mystation.mqtt_status
        else:
            self.mqtt_connected = False
            #self.mqtt_status = _('Connect Status: disconnected')
            self.mqtt_status = _("Station")+": "+_(" disactive")


    @mainthread
    def publishphoto_loop(self, *args):
        '''
        This function publish photo to amqp broker.
        '''
        print "call in publishphoto_loop"

        try:

            for file in sorted(glob(QUEUEDIMAGES)):
                if os.path.isfile(file):

                    #self.notify(_("Wait"))

                    print "send image: ",file
                    # read image in memory.
                    photo_file = open(file,"r")
                    body = photo_file.read()
                    photo_file.close()

                    rmap.rmap_core.send2amqp(body=body,
                                             user=self.config.get('rmap','user'),
                                             password=self.config.get('rmap','password'),
                                             host=self.config.get('rmap','server'),
                                             exchange="photo",routing_key="photo")

                    os.remove(file)
                    #self.notifydismiss()

            self.root.ids["queuedimagebox"].clear_widgets()
            image=QueuedImage(queuedimage=os.path.join(os.path.dirname(__file__), "icons", "noimage.png"))
            self.root.ids["queuedimagebox"].add_widget(image)

#            self.root.ids["queuedimage"].source= os.path.join(os.path.dirname(__file__), "icons", "noimage.png")
#            self.root.ids["queuedimage"].reload()

            return True

        except:

            #self.notifydismiss()
            self.root.ids["queuedimagebox"].clear_widgets()

            queuedimage=queuedfilename()
            if (not queuedimage is None):
                #self.root.ids["queuedimage"].source= queuedimage
                for file in sorted(glob(QUEUEDIMAGES)):
                    if os.path.isfile(file):
                        image=QueuedImage(queuedimage=file)
                        self.root.ids["queuedimagebox"].add_widget(image)

            else:
                image=QueuedImage(queuedimage=os.path.join(os.path.dirname(__file__), "icons", "noimage.png"))
                self.root.ids["queuedimagebox"].add_widget(image)

                #self.root.ids["queuedimage"].source= os.path.join(os.path.dirname(__file__), "icons", "noimage.png")
                #self.root.ids["queuedimage"].reload()

            self.popup(_("error sending\nimage to server!"))
            # disable until new connect
            return False


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
            self.movelocation(lat=lat,lon=lon,height=height)


    @mainthread
    def on_status(self, status, gpsfix):
        '''
        callback for new GPS status
        '''

        self.gps_status = status
        self.gpsfix=gpsfix

        #mmm take alook at 
        #http://stackoverflow.com/questions/2021176/how-can-i-check-the-current-status-of-the-gps-receiver


