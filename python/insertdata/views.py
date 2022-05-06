from geoimage.models import GeorefencedImage
from geoimage.models import CATEGORY_CHOICES
from django.shortcuts import render
from django import forms
from datetime import date,datetime,timedelta,time
from django.utils.translation import ugettext_lazy
from django.contrib.auth.models import User
from django.http import HttpResponseRedirect
from leaflet.forms.widgets import LeafletWidget
from leaflet.forms.fields import PointField
from django.urls import reverse
from  rmap.tables import Table,TableEntry
import os
from django.utils.translation import ugettext as _
from django.utils.translation import ugettext_lazy as __
from django.core.files import File
from tempfile import NamedTemporaryFile
from django.core.files.base import ContentFile
from rmap.rmapmqtt import rmapmqtt
from rmap.stations.models import StationMetadata
from django.contrib.gis.geos import Point
import rmap.settings
from nominatim import Nominatim
from django.contrib.sites.shortcuts import get_current_site
from django.utils.text import slugify
#from rmap.rmap_core import isRainboInstance
import rmap.rmap_core
import decimal
from django.core.exceptions import ObjectDoesNotExist
from django.db.utils import IntegrityError

class scelta_present_weather(object):
    '''
    build choices for build 
    '''

    def __init__(self,language_code):

        lang = language_code.split("-")[0]

        try:
            self.table = Table(os.path.join(os.path.dirname(__file__), "../rmap/tables","present_weather_"+lang+".txt"))
        except:
            try:
                #print ("error getting lang: ", lang)
                from django.utils import  translation
                lang=translation.get_language().split("-")[0]
                self.table = Table(os.path.join(os.path.dirname(__file__), "../rmap/tables","present_weather_"+lang+".txt"))
            except:
                #print ("error getting system lang: ", lang)
                lang="en"
                self.table = Table(os.path.join(os.path.dirname(__file__), "../rmap/tables","present_weather_"+lang+".txt"))

        self.table[""]=TableEntry(code="",description="----------------")

    def __iter__(self):
        self.iter=self.table.__iter__()
        return self

    def __next__(self):
        entry=next(self.iter)
        return (self.table[entry].code,self.table[entry].description)


class scelta_stations(object):

    def __init__(self,username):
        self.username=username

    def __iter__(self):
        #self.stations=StationMetadata.objects.filter(active=True,ident__username=self.username).values("slug","lat","lon")
        self.stations=StationMetadata.objects.filter(active=True,ident__username=self.username).iterator()
        self.first=True
        return self

    def __next__(self):

        if self.first:
            self.first=False
            return ("","-------")

        station=next(self.stations)
        #return (station["slug"],str(station["lat"])+str(station["lon"]))
        return (station.slug,station.name)


class ImageForm(forms.ModelForm):
    #geom = PointField()

    class Meta:
        model = GeorefencedImage
        fields = ('geom','image','comment')
        widgets = {'geom': LeafletWidget()}


class StationForm(forms.Form):

    def __init__(self, username, *args, **kwargs):

        super(StationForm, self).__init__(*args, **kwargs)
        self.fields['station_slug'] = forms.ChoiceField(choices=scelta_stations(username),required=False,label=__('Your station'),help_text=__('Select configurated station'),initial="")


class TimeElapsedForm(forms.Form):

    timeelapsedchoices=[(0,__("now")),
                        (-1,__("observed 1 our before")),
                        (-2,__("observed 2 hours before")),
                        (-3,__("observed 3 hours before")),
                        (-4,__("observed 4 hours before")),
                        (-5,__("observed 5 hours before")),
                        (-6,__("observed 6 hours before"))]
    
    timeelapsed = forms.ChoiceField(choices=timeelapsedchoices,
                                    required=True,label=__('Time elapsed'),
                                    help_text=__('Time elapsed from observation time'),
                                    initial="0")

class ManualForm(forms.ModelForm):

    #geom = PointField()

    coordinate_slug= forms.CharField(widget=forms.HiddenInput(),required=False)
    
    visibility=forms.IntegerField(required=False,label=__("Visibility(m.)"),help_text='',min_value=0,max_value=1000000)
    snow_height=forms.IntegerField(required=False,label=__("Snow height(cm.)"),help_text='',min_value=0,max_value=1000)
    
    def __init__(self, *args, **kwargs):
        super().__init__(*args)
        self.language_code = kwargs["language_code"]
        self.fields["presentweather"]=forms.ChoiceField(choices=scelta_present_weather(self.language_code),required=False,label=__('Present weather'),help_text=__('Present weather'),initial="")
    class Meta:
        model = GeorefencedImage
        fields = ('geom',)
        widgets = {'geom': LeafletWidget()}


class RainboWeatherForm(forms.ModelForm):
    coordinate_slug= forms.CharField(widget=forms.HiddenInput(),required=False)
    #fixed standard values from ~/rmap/python/rmap/tables/present_weather.txt  
    not_significant = forms.ChoiceField(widget=forms.RadioSelect(),choices=[(100,__("not significant"))],required=False,label=__("Not_significant"),help_text='')
    visibility_intensity=forms.ChoiceField(widget=forms.RadioSelect(),choices=[(110,__("haze")),(130,__("fog"))],required=False,label=__("Visibility"),help_text='')
    snow_intensity=forms.ChoiceField(widget=forms.RadioSelect(),choices=[(185,__("weak")),(186,__("moderate")),(187,__("intense"))],required=False,label=__("Snow"),help_text='')    
    thunderstorm_intensity=forms.ChoiceField(widget=forms.RadioSelect(),choices=[(192,__("moderate with rain")),(193,__("moderate with hail")),(195,__("intense with rain")),(196,__("intense with hail"))],required=False,label=__("Thunderstorm"),help_text='') 
    rain_intensity=forms.ChoiceField(widget=forms.RadioSelect(),choices=[(150,__("drizzle")),(160,__("rain")),(165,__("freezing on the ground")),(184,'very heavy')],required=False,label=__("Rain"),help_text='')
    tornado=forms.ChoiceField(widget=forms.RadioSelect(),choices=[(199,__("tornado"))],required=False,label=__("Tornado"),help_text='')

    class Meta:
        model = GeorefencedImage
        fields = ('geom',)
        widgets = {'geom': LeafletWidget()}


class RainboImpactForm(forms.ModelForm):
    coordinate_slug= forms.CharField(widget=forms.HiddenInput(),required=False)
    impact_detected=forms.ChoiceField(widget=forms.RadioSelect(),choices=[(10,__("fallen tree")),(20,__("icy road")),(30,__("flooding")),(40,__("pothole"))],required=False,label=__("Impact detected"),help_text='') 

    class Meta:
        model = GeorefencedImage
        fields = ('geom',)
        widgets = {'geom': LeafletWidget()}


class NominatimForm(forms.Form):
    address= forms.CharField(required=False,label=__("Search address"),help_text='')



class NewStationForm(forms.ModelForm):

    #geom = PointField()

    CHOICES = []
    for tem in rmap.rmap_core.template_choices: 
        CHOICES.append((tem,tem))
    
    coordinate_slug= forms.CharField(widget=forms.HiddenInput(),required=False)
    name= forms.CharField(required=True,label=__("New station name"),help_text=__('The name of the station to insert'))
    template=forms.ChoiceField(choices=CHOICES,required=True,label=__("station model"),help_text=__('The model of the station to insert'),initial="none")
    password = forms.CharField(required=True,label=_('Password'),help_text=_('Password for MQTT broker'),widget=forms.PasswordInput)
    passwordrepeat = forms.CharField(required=True,label=_('Repeat password'),help_text=_('Repeat password for MQTT broker'),widget=forms.PasswordInput)
    
    class Meta:
        model = GeorefencedImage
        fields = ('geom',)
        widgets = {'geom': LeafletWidget()}


class NewStationDetailForm(forms.Form):

    CHOICES = []
    for tem in rmap.rmap_core.template_choices: 
        CHOICES.append((tem,tem))
    
    stationname = forms.CharField(required=True,label=_('Station name'),help_text=_('Station name'))
    latitude = forms.DecimalField(required=True,label=_('Latitude'),help_text=_('Latitude'),min_value=decimal.Decimal("0."),max_value=decimal.Decimal("90."),decimal_places=5)
    longitude = forms.DecimalField(required=True,label=_('Longitude'),help_text=_('Longitude'),min_value=decimal.Decimal("0."),max_value=decimal.Decimal("360."),decimal_places=5)
    height = forms.DecimalField(required=True,label=_('Station height (m.)'),help_text=_('Station height (m.)'),min_value=decimal.Decimal("-10."),max_value=decimal.Decimal("10000."),decimal_places=1)
    template = forms.ChoiceField(choices=CHOICES,required=True,label=__("station model"),help_text=__('The model of the station to insert'),initial="none")
    mqttsamplerate=forms.IntegerField(required=True,label=__("report period (secondi)"),help_text='Time elapsed from two reports',min_value=0,max_value=3600*12,initial=900)
    password = forms.CharField(required=True,label=_('Password'),help_text=_('Password for MQTT broker'),widget=forms.PasswordInput)
    passwordrepeat = forms.CharField(required=True,label=_('Repeat password'),help_text=_('Repeat password for MQTT broker'),widget=forms.PasswordInput)

        
from django.contrib.auth.decorators import login_required

@login_required
def insertDataImage(request):

    if request.method == 'POST': # If the form has been submitted...
        stationform = StationForm(request.user.get_username(),request.POST, request.FILES) # A form bound to the POST data
        nominatimform = NominatimForm(request.POST) # A form bound to the POST data
        form = ImageForm(request.POST, request.FILES) # A form bound to the POST data

        if stationform.is_valid(): # All validation rules pass

            slug=stationform.cleaned_data['station_slug']
            if slug:
                station=StationMetadata.objects.get(ident__username=request.user.username,slug=slug)
                #stationlat=station.lat
                #stationlon=station.lon
                POST=request.POST.copy()
                POST['geom']= str(Point(station.lon,station.lat))
                stationform = StationForm(request.user.get_username(),POST, request.FILES) # A form bound to the new data
                form = ImageForm(POST, request.FILES) # A form bound to the new data
                return render(request, 'insertdata/form.html',{'form': form,'stationform':stationform,'nominatimform':nominatimform})
        else:
            stationform = StationForm(request.user.get_username())
            return render(request, 'insertdata/form.html',{'form': form,'stationform':stationform,'nominatimform':nominatimform,"invalid":True})

        if nominatimform.is_valid(): # All validation rules pass

            address=nominatimform.cleaned_data['address']
            if address:
                nom = Nominatim(base_url="http://nominatim.openstreetmap.org",referer= get_current_site(request).domain)
                result=nom.query(address,limit=1,countrycodes="IT")
                if len(result) >= 1:
                    lat= result[0]["lat"]
                    lon= result[0]["lon"]
                    address= result[0]["display_name"]
                    POST=request.POST.copy()
                    POST['geom']= str(Point(float(lon),float(lat)))
                    POST['address']= address
                    stationform = StationForm(request.user.get_username(),POST, request.FILES) # A form bound to the new data
                    nominatimform = NominatimForm(POST) # A form bound to the new data
                    form = ImageForm(POST, request.FILES) # A form bound to the new data
                return render(request, 'insertdata/form.html',{'form': form,'stationform':stationform,'nominatimform':nominatimform})
        else:
            nominatimform = NominatimForm()
            return render(request, 'insertdata/form.html',{'form': form,'stationform':stationform,'nominatimform':nominatimform,"invalid":True})

        if form.is_valid(): # All validation rules pass

            if True:
                from rmap import exifutils
                comment=form.cleaned_data['comment']
                geom=form.cleaned_data['geom']
                image=request.FILES['image']
                dt=datetime.utcnow().replace(microsecond=0)
                lon=geom['coordinates'][0]
                lat=geom['coordinates'][1]
                image=image.read()

                body=exifutils.setgeoimage(image,lat,lon,imagedescription=request.user.username,usercomment=comment)

            else:
                import pexif
                img = pexif.JpegFile.fromString(handle_uploaded_file(image).encode("utf8"))
                exif = img.get_exif()
                if exif:
                    primary = exif.get_primary()
                    if not exif is None or not primary is None:

                        primary.ImageDescription = str(request.user.username)
                        #primary.ExtendedEXIF.UserComment = "UNICODE"+chr(0x00)+str(comment)
                        primary.ExtendedEXIF.UserComment = chr(0x55)+chr(0x4E)+chr(0x49)+chr(0x43)+chr(0x4F)+chr(0x44)+chr(0x45)+chr(0x00)+str(comment)
                        img.set_geo(lat,lon)
                        
                        #        try:
                        #            print primary.DateTime
                        #        except:
                        #            print "DateTime not present"

                        primary.DateTime=datetime.utcnow().strftime("%Y:%m:%d %H:%M:%S")
                        #        print primary.DateTime
                        body=img.writeString()


            #grimages=GeorefencedImage.objects.filter(ident__username=ident)
            #grimages=GeorefencedImage.objects.filter(id=1)


            #f = NamedTemporaryFile(delete=False)
            #image = File(f)
            #image.write(body)
            #f.close()
            #os.unlink(f.name)

            if True:
                #inserimento diretto in DB

                geoimage=GeorefencedImage(active=True,geom = geom,comment=comment,ident=request.user,
                                      date=dt, category = CATEGORY_CHOICES[1])

                geoimage.image.save('geoimage.jpg',ContentFile(body))

                geoimage.save()

            else:
                # invio ad AMQP

                #quale utente usare per AMQP; ho l'utente ma non la password
                #bisognerebbe abilitare tutti gli admin a pubblicare immagini e qui usare amqpuser

                #user=request.user.username,
                user=rmap.settings.amqpuser
                password=rmap.settings.amqppassword

                rmap.rmap_core.send2amqp(body=body,
                                         user=user,
                                         password=password,
                                         host="localhost",
                                         exchange="photo",routing_key="photo")

            #return HttpResponseRedirect(reverse('geoimage-ident-id', args=[request.user.username,geoimage.pk]))
            return HttpResponseRedirect(reverse('geoimage-ident', args=[request.user.username]))

        else:

            form = ImageForm() # An unbound form
            return render(request, 'insertdata/form.html',{'form': form,'stationform':stationform,'nominatimform':nominatimform,"invalid":True})

    else:
            stationform = StationForm(request.user.get_username()) # An unbound form
            nominatimform = NominatimForm() # An unbound form
            form = ImageForm() # An unbound form
            return render(request, 'insertdata/form.html',{'form': form,'stationform':stationform,'nominatimform':nominatimform})

@login_required
def insertDataRainboImpactData(request):
    html_template = 'insertdata/rainbodataform.html'
    form = RainboImpactForm(request.POST) # A form bound to the POST data
    if request.method == 'POST': # If the form has been submitted...
        if form.is_valid(): # All validation rules pass
            geom=form.cleaned_data['geom']
            lon=geom['coordinates'][0]
            lat=geom['coordinates'][1]
            dt=datetime.utcnow().replace(microsecond=0)
            ident=request.user.username
            datavar={}

            value = form.cleaned_data['impact_detected'] if form.cleaned_data['impact_detected'] != "" else ""

            if (value != ""):
                datavar["B20203"]={"t": dt,"v": str(value)}
            if (len(datavar)>0):
                try:
                    # TODO!
                    #here we have to use mqtt_user and mqtt_password from transport_mqtt
                    user=rmap.settings.mqttuser
                    password=rmap.settings.mqttpassword
                    prefix=rmap.settings.topicreport
                    maintprefix=rmap.settings.topicmaint
                    network="mobile"
                    slug=form.cleaned_data['coordinate_slug']

                    print("<",slug,">","prefix:",prefix)

                    mqtt=rmapmqtt(ident=ident,lon=lon,lat=lat,network=network,host="localhost",port=1883,prefix=prefix,maintprefix=maintprefix,username=user,password=password)
                    mqtt.connect()
                    mqtt.data(timerange="254,0,0",level="1,-,-,-",datavar=datavar)
                    mqtt.disconnect()
                    form = RainboImpactForm() # An unbound Rainbo form
                except Exception as e:
                    return render(request, html_template,{'form': form,"error":True})

            return render(request, html_template,{'form': form, "success":True})

        else:
            return render(request, html_template,{'form': form,"invalid":True})
                
    else:
        return render(request, html_template,{'form': form})

@login_required
def insertDataRainboWeatherData(request):

    html_template = 'insertdata/rainbodataform.html'
    form = RainboWeatherForm(request.POST) # A form bound to the POST data
    if request.method == 'POST': # If the form has been submitted...
        if form.is_valid(): # All validation rules pass
            geom=form.cleaned_data['geom']
            lon=geom['coordinates'][0]
            lat=geom['coordinates'][1]
            dt=datetime.utcnow().replace(microsecond=0)
            ident=request.user.username
            datavar={}

            #Ascending importance order
            value = form.cleaned_data['not_significant'] if form.cleaned_data['not_significant'] != "" else ""
            value = form.cleaned_data['visibility_intensity'] if form.cleaned_data['visibility_intensity'] != "" else value
            value = form.cleaned_data['rain_intensity'] if form.cleaned_data['rain_intensity'] != ""  else value
            value = form.cleaned_data['snow_intensity'] if form.cleaned_data['snow_intensity'] != "" else value
            value = form.cleaned_data['thunderstorm_intensity'] if form.cleaned_data['thunderstorm_intensity'] != "" else value
            value = form.cleaned_data['tornado'] if form.cleaned_data['tornado'] != "" else value
            if (value != ""):
                datavar["B20003"]={"t": dt,"v": str(value)}
            if (len(datavar)>0):
                try:
                    # TODO!
                    #here we have to use mqtt_user and mqtt_password from transport_mqtt
                    user=rmap.settings.mqttuser
                    password=rmap.settings.mqttpassword
                    prefix=rmap.settings.topicreport
                    maintprefix=rmap.settings.topicmaint
                    network="mobile"
                    slug=form.cleaned_data['coordinate_slug']
                    print(user,password,network,prefix)
                    print("<",slug,">","prefix:",prefix)
                    mqtt=rmapmqtt(ident=ident,lon=lon,lat=lat,network=network,host="localhost",port=1883,prefix=prefix,maintprefix=maintprefix,username=user,password=password)
                    mqtt.connect()                    
                    mqtt.data(timerange="254,0,0",level="1,-,-,-",datavar=datavar)
                    mqtt.disconnect()
                    form = RainboWeatherForm() # An unbound Rainbo form
                except Exception as e:
                    print(e)
                    return render(request, html_template,{'form': form,"error":True})

            return render(request, html_template,{'form': form ,"success":True})

        else:
            return render(request, html_template,{'form': form,"invalid":True})
                
    else:
        return render(request, html_template,{'form': form})


@login_required
def insertDataManualData(request):

    if request.method == 'POST': # If the form has been submitted...

        #stationlat=None
        #stationlon=None

        stationform = StationForm(request.user.get_username(),request.POST) # A form bound to the POST data
        nominatimform = NominatimForm(request.POST) # A form bound to the POST data
        timeelapsedform=TimeElapsedForm(request.POST) # A form bound to the POST data
        form = ManualForm(request.POST,language_code=request.LANGUAGE_CODE) # A form bound to the POST data

        if stationform.is_valid(): # All validation rules pass

            slug=stationform.cleaned_data['station_slug']
            if slug:
                station=StationMetadata.objects.get(ident__username=request.user.username,slug=slug)
                #stationlat=station.lat
                #stationlon=station.lon
                POST=request.POST.copy()
                POST['geom']= str(Point(station.lon,station.lat))
                POST['coordinate_slug']= slug
                stationform = StationForm(request.user.get_username(),POST) # A form bound to the new data
                form = ManualForm(POST,language_code=request.LANGUAGE_CODE) # A form bound to the new data
                return render(request, 'insertdata/manualdataform.html',{'form': form,'stationform':stationform,'nominatimform':nominatimform,'timeelapsedform':timeelapsedform})
        else:
            stationform = StationForm(request.user.get_username())
            return render(request, 'insertdata/manualdataform.html',{'form': form,'stationform':stationform,'nominatimform':nominatimform,"invalid":True,'timeelapsedform':timeelapsedform})
        
        if nominatimform.is_valid(): # All validation rules pass

            address=nominatimform.cleaned_data['address']
            if address:
                nom = Nominatim(base_url="http://nominatim.openstreetmap.org",referer=get_current_site(request).domain)
                result=nom.query(address,limit=1,countrycodes="IT")
                if result is not None:
                    if len(result) >= 1:
                        lat= result[0]["lat"]
                        lon= result[0]["lon"]
                        address= result[0]["display_name"]
                        POST=request.POST.copy()
                        POST['geom']= str(Point(float(lon),float(lat)))
                        POST['address']= address
                        nominatimform = NominatimForm(POST) # A form bound to the new data
                        stationform = StationForm(request.user.get_username(),POST) # A form bound to the new data
                        form = ManualForm(POST,language_code=request.LANGUAGE_CODE) # A form bound to the new data
                return render(request, 'insertdata/manualdataform.html',{'form': form,'stationform':stationform,'nominatimform':nominatimform,'timeelapsedform':timeelapsedform})
        else:
            nominatimform = NominatimForm()
            return render(request, 'insertdata/manualdataform.html',{'form': form,'stationform':stationform,'nominatimform':nominatimform,"invalid":True,'timeelapsedform':timeelapsedform})

        if form.is_valid() and timeelapsedform.is_valid(): # All validation rules pass

            timeelapsed=timeelapsedform.cleaned_data['timeelapsed']
            geom=form.cleaned_data['geom']
            lon=geom['coordinates'][0]
            lat=geom['coordinates'][1]
            dt=datetime.utcnow().replace(microsecond=0)+timedelta(hours=int(timeelapsed))
            username=request.user.username
            board_slug="default"
            
            #if (not stationlat is None):
            #    if (stationlat != lat):
            #        stationform = StationForm(request.user.get_username())
            #        return render(request, 'insertdata/manualdataform.html',{'form': form,'stationform':stationform,"invalid":True})
            #if (not stationlon is None):
            #    if (stationlon != lon):
            #        stationform = StationForm(request.user.get_username())
            #        return render(request, 'insertdata/manualdataform.html',{'form': form,'stationform':stationform,"invalid":True})

            datavar={}
            value=form.cleaned_data['presentweather']
            if (value != ""):
                datavar["B20003"]={"t": dt,"v": str(value)}

            value=form.cleaned_data['snow_height']
            if (not value is None):
                value=int(value*10)
                datavar["B13013"]={"t": dt,"v": str(value)}

            value=form.cleaned_data['visibility']
            if (not value is None):
                value=int(value/10)
                datavar["B20001"]={"t": dt,"v": str(value)}

            print("datavar:",datavar)

            if (len(datavar)>0):
                try:
                    prefix=rmap.settings.topicreport
                    maintprefix=rmap.settings.topicmaint
                    print(prefix,maintprefix)
                    station_slug=form.cleaned_data['coordinate_slug']
                    if (station_slug):
                        # if we have station slug from other form we get it from DB
                        # fixed station
                        mystation=StationMetadata.objects.get(ident__username=username,slug=station_slug)
                        if mystation is not None:
                            if mystation.active:
                                lat=mystation.lat
                                lon=mystation.lon
                                ident=""
                                network=mystation.network
                                myboard = mystation.board_set.get(slug=board_slug)
                                if myboard is not None:
                                    if ( myboard.active and myboard.transportmqtt.active):
                                        mqttuser = myboard.transportmqtt.mqttuser
                                        mqttpassword = myboard.transportmqtt.mqttpassword
                                        host = myboard.transportmqtt.mqttserver
                                    
                    else:
                        # no station slug so we are a mobile station
                        try:
                            # get default mobile station from DB
                            station_slug="auto_mobile"
                            mystation=StationMetadata.objects.get(ident__username=username,slug=station_slug)
                            network=mystation.network
                            myboard = mystation.board_set.get(slug=board_slug)
                            if myboard is not None:
                                if ( myboard.active and myboard.transportmqtt.active):
                                    mqttuser = myboard.transportmqtt.mqttuser
                                    ident = mqttuser
                                    mqttpassword = myboard.transportmqtt.mqttpassword
                                    host = myboard.transportmqtt.mqttserver

                        except ObjectDoesNotExist:
                            # create new default mobile station in DB
                            network="mobile"
                            host = get_current_site(request).domain.split(":")[0]
                            #host="localhost"
                            mqttuser=username
                            mqttpassword=User.objects.make_random_password()
                            user=User.objects.get(username=username)
                            ident=username
                            mystation=StationMetadata(slug=station_slug,name="Auto mobile",active=True,network=network,ident=user, mqttrootpath="report",lat=None,lon=None)    
                            mystation.clean()
                            mystation.save()

                            rmap.rmap_core.addboard(station_slug=station_slug,username=username,board_slug=board_slug,activate=True
                                                    ,serialactivate=False
                                                    ,mqttactivate=True, mqttserver=host, mqttusername=mqttuser, mqttpassword=mqttpassword, mqttsamplerate=30
                                                    ,bluetoothactivate=False, bluetoothname="HC-05"
                                                    ,amqpactivate=False, amqpusername=username, amqppassword=mqttpassword, amqpserver=host, queue="rmap", exchange="rmap"
                                                    ,tcpipactivate=False, tcpipname="master", tcpipntpserver="pool.ntp.org"
                                                    )

                    print(host, username, ident,lon,lat,network,prefix,prefix, mqttuser+"/"+station_slug+"/"+board_slug,mqttpassword)
                    mqtt=rmapmqtt(ident=ident,lon=lon,lat=lat,network=network,host=host,port=1883,prefix=prefix,maintprefix=maintprefix,
                                  username=mqttuser+"/"+station_slug+"/"+board_slug,password=mqttpassword,version=1,user=username)
                    mqtt.connect()
                    mqtt.data(timerange="254,0,0",level="1,-,-,-",datavar=datavar)
                    mqtt.disconnect()

                    timeelapsedform = TimeElapsedForm()
                    form = ManualForm(language_code=request.LANGUAGE_CODE) # An unbound form
                except:
                    raise
                    #return render(request, 'insertdata/manualdataform.html',{'form': form,'stationform':stationform,'nominatimform':nominatimform,'timeelapsedform':timeelapsedform,"error":True})

            return render(request, 'insertdata/manualdataform.html',{'form': form,'stationform':stationform,'nominatimform':nominatimform,'timeelapsedform':timeelapsedform,"success":True})

        else:

            print("invalid form")
            form = ManualForm(language_code=request.LANGUAGE_CODE) # An unbound form
            timeelapsedform = TimeElapsedForm()
            return render(request, 'insertdata/manualdataform.html',{'form': form,'stationform':stationform,'nominatimform':nominatimform,"invalid":True,'timeelapsedform':timeelapsedform})

    else:
        stationform = StationForm(request.user.get_username()) # An unbound form
        nominatimform = NominatimForm() # An unbound form
        form = ManualForm(language_code=request.LANGUAGE_CODE) # An unbound form
        timeelapsedform = TimeElapsedForm() # A form bound to the POST data
        return render(request, 'insertdata/manualdataform.html',{'form': form,'stationform':stationform,'nominatimform':nominatimform,'timeelapsedform':timeelapsedform})


@login_required
def insertNewStation(request):

    if request.method == 'POST': # If the form has been submitted...

        nominatimform = NominatimForm(request.POST) # A form bound to the POST data
        newstationform = NewStationForm(request.POST) # A form bound to the POST data

        if nominatimform.is_valid(): # All validation rules pass

            address=nominatimform.cleaned_data['address']
            if address:
                nom = Nominatim(base_url="https://nominatim.openstreetmap.org",referer=get_current_site(request).domain)
                result=nom.query(address,limit=1,countrycodes="IT")
                if len(result) >= 1:
                    lat= result[0]["lat"]
                    lon= result[0]["lon"]
                    address= result[0]["display_name"]
                    POST=request.POST.copy()
                    POST['geom']= str(Point(float(lon),float(lat)))
                    POST['address']= address
                    newstationform = NewStationForm(POST) # A form bound to the new data
                    nominatimform = NominatimForm(POST) # A form bound to the new data

                return render(request, 'insertdata/newstationform.html',{'nominatimform':nominatimform,'newstationform':newstationform})
        else:
            nominatimform = NominatimForm()
            return render(request, 'insertdata/newstationform.html',{'nominatimform':nominatimform,'newstationform':newstationform,"invalid":True})


        if newstationform.is_valid(): # All validation rules pass
            
            geom=newstationform.cleaned_data['geom']
            lon=geom['coordinates'][0]
            lat=geom['coordinates'][1]
            ident=request.user.username
            name=newstationform.cleaned_data['name']
            password=newstationform.cleaned_data['password']
            passwordrepeat=newstationform.cleaned_data['passwordrepeat']
            slug=slugify(name)
            board_slug="default"
            template=newstationform.cleaned_data['template']
            host = get_current_site(request).domain.split(":")[0]
            
            if name and (password == passwordrepeat):
                try:
                    try:
                        print("del station:", ident,slug,ident)
                        mystation=StationMetadata.objects.get(slug__exact=slug,ident__username=ident)
                        mystation.delete()
                    except Exception as e:
                        print(e)
                    
                    print("new station:", name,ident,lon,lat)

                    mystation=StationMetadata(slug=slug,name=name)
                    user=User.objects.get(username=ident)
                    mystation.ident=user
                    mystation.lat=rmap.rmap_core.truncate(lat,5)
                    mystation.lon=rmap.rmap_core.truncate(lon,5)
                    mystation.active=True

                    # this in not very good ! we need to specify better in template the type (report/sample)
                    if ("_report_" in template):
                        mystation.mqttrootpath="report"
                    
                    mystation.clean()
                    mystation.save()

                    rmap.rmap_core.addboard(station_slug=slug,username=ident,board_slug=board_slug,activate=True
                                 ,serialactivate=False
                                 ,mqttactivate=True, mqttserver=host, mqttusername=ident, mqttpassword=password, mqttsamplerate=30
                                 ,bluetoothactivate=False, bluetoothname="HC-05"
                                ,amqpactivate=False, amqpusername=ident, amqppassword=password, amqpserver=host, queue="rmap", exchange="rmap"
                                 ,tcpipactivate=False, tcpipname="master", tcpipntpserver="pool.ntp.org"
                    )
                    
                    rmap.rmap_core.addsensors_by_template(
                        station_slug=slug
                        ,username=ident
                        ,board_slug=board_slug
                        ,template=template)

                except IntegrityError as e:
                    print(e)
                    return render(request, 'insertdata/newstationform.html',{'nominatimform':nominatimform,'newstationform':newstationform,"error":True,"duplicated":True})
                    
                except Exception as e:
                    print(e)
                    return render(request, 'insertdata/newstationform.html',{'nominatimform':nominatimform,'newstationform':newstationform,"error":True})

            return render(request, 'insertdata/newstationform.html',{'nominatimform':nominatimform,'newstationform':newstationform,"station":mystation})

        else:

            print("invalid form")
            nominatimform = NominatimForm() # An unbound form
            newstationform = NewStationForm() # An unbound form
            return render(request, 'insertdata/newstationform.html',{'nominatimform':nominatimform,'newstationform':newstationform,"invalid":True})

    else:
        nominatimform = NominatimForm() # An unbound form
        newstationform = NewStationForm() # An unbound form
        return render(request, 'insertdata/newstationform.html',{'nominatimform':nominatimform,'newstationform':newstationform,})


@login_required
def insertNewStationDetail(request,slug=None):

    if request.method == 'POST': # If the form has been submitted...

        newstationdetailform = NewStationDetailForm(request.POST) # A form bound to the POST data
        if newstationdetailform.is_valid(): # All validation rules pass
            
            lat=newstationdetailform.cleaned_data['latitude']
            lon=newstationdetailform.cleaned_data['longitude']
            ident=request.user.username
            mqttsamplerate=newstationdetailform.cleaned_data['mqttsamplerate']
            password=newstationdetailform.cleaned_data['password']
            passwordrepeat=newstationdetailform.cleaned_data['passwordrepeat']
            name=newstationdetailform.cleaned_data['stationname']
            height=str(int(newstationdetailform.cleaned_data['height']*10))
            constantdata={}
            constantdata["B01019"]=name
            constantdata["B07030"]=height

            #slug=newstationdetailform.cleaned_data.get('slug')
            if (not slug): slug=slugify(name)
            
            board_slug="default"
            template=newstationdetailform.cleaned_data['template']
            
            if name and (password == passwordrepeat):
                try:
                    try:
                        print("del station:", ident,slug,ident)
                        mystation=StationMetadata.objects.get(slug__exact=slug,ident__username=ident)
                        mystation.delete()
                    except Exception as e:
                        print(e)
                    
                    print("new station:", name,ident,lon,lat)

                    mystation=StationMetadata(slug=slug,name=name)
                    user=User.objects.get(username=ident)
                    mystation.ident=user
                    mystation.lat=rmap.rmap_core.truncate(lat,5)
                    mystation.lon=rmap.rmap_core.truncate(lon,5)
                    mystation.active=True
                    host = get_current_site(request).domain.split(":")[0]

                    # this in not very good ! we need to specify better in template the type (report/sample)
                    if ("_report_" in template):
                        mystation.mqttrootpath="report"
                    
                    mystation.clean()
                    mystation.save()


                    # remove all StationConstantData
                    try:
                        StationConstantData.objects.filter(stationmetadata=mystation).delete()
                    except:
                        pass

                    for btable,value in constantdata.items():

                        # remove only StationConstantData in constantdata
                        #try:
                        #    StationConstantData.objects.filter(stationmetadata=mystation,btable=btable).delete()
                        #except:
                        #    pass

                        try:
                            mystation.stationconstantdata_set.create(
                                active=True,
                                btable=btable,
                                value=value
                            )

                        except:
                            pass

                    rmap.rmap_core.addboard(station_slug=slug,username=ident,board_slug=board_slug,activate=True
                                ,serialactivate=True
                                ,mqttactivate=True, mqttserver=host, mqttusername=ident, mqttpassword=password, mqttsamplerate=mqttsamplerate
                                ,bluetoothactivate=False, bluetoothname="HC-05"
                                ,amqpactivate=False, amqpusername=ident, amqppassword=password, amqpserver=host, queue="rmap", exchange="rmap"
                                ,tcpipactivate=True, tcpipname="master", tcpipntpserver="it.pool.ntp.org", tcpipgsmapn="ibox.tim.it"
                    )

                    rmap.rmap_core.addsensors_by_template(
                        station_slug=slug
                        ,username=ident
                        ,board_slug=board_slug
                        ,template=template)

                except IntegrityError as e:
                    print(e)
                    return render(request, 'insertdata/newstationdetailform.html',{'newstationdetailform':newstationdetailform,"error":True,"duplicated":True})
                    
                except Exception as e:
                    print(e)
                    return render(request, 'insertdata/newstationdetailform.html',{'newstationdetailform':newstationdetailform,"error":True})

                newstationdetailform = NewStationDetailForm() # An unbound form
                return render(request, 'insertdata/newstationdetailform.html',{'newstationdetailform':newstationdetailform,"station":mystation})

                
            else:
                return render(request, 'insertdata/newstationdetailform.html',{'newstationdetailform':newstationdetailform,"invalid":True})


        else:
            print("invalid form")
            newstationdetailform = NewStationDetailForm() # An unbound form
            return render(request, 'insertdata/newstationdetailform.html',{'newstationdetailform':newstationdetailform,"invalid":True})
            
    newstationdetailform = NewStationDetailForm() # An unbound form            
    return render(request, 'insertdata/newstationdetailform.html',{'newstationdetailform':newstationdetailform})



@login_required
def stationModify(request,slug):

    if request.method == 'POST': # If the form has been submitted...
        return insertNewStationDetail(request,slug=slug)
    
    try:
        mystation=StationMetadata.objects.get(slug__exact=slug,ident__username=request.user.username)

        mystation_dict= {
            "slug":  mystation.slug
            ,"latitude":    mystation.lat
            ,"longitude":  mystation.lon
            ,"stationname":  mystation.name
        }

    except Exception as e:
        print(e)
        newstationdetailform = NewStationDetailForm() # An unbound form            
        return render(request, 'insertdata/newstationdetailform.html',{'newstationdetailform':newstationdetailform,"error":True})

    newstationdetailform=NewStationDetailForm(initial = mystation_dict)    
    return render(request, 'insertdata/newstationdetailform.html',{'newstationdetailform':newstationdetailform})

    
