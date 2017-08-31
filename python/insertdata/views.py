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
from django.core.urlresolvers import reverse
from  rmap.tables import Table,TableEntry
import os
from django.utils.translation import ugettext as _
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

lang="it"


class scelta_present_weather(object):
    '''
    build choices for build 
    '''

    def __init__(self):
        self.table = Table(os.path.join(os.path.dirname(__file__), "../rmap/tables","present_weather_"+lang+".txt"))
        self.table[""]=TableEntry(code="",description="----------------")

    def __iter__(self):
        self.iter=self.table.__iter__()
        return self

    def next(self):
        entry=self.iter.next()
        return (self.table[entry].code,self.table[entry].description)


class scelta_stations(object):

    def __init__(self,username):
        self.username=username

    def __iter__(self):
        #self.stations=StationMetadata.objects.filter(active=True,ident__username=self.username).values("slug","lat","lon")
        self.stations=StationMetadata.objects.filter(active=True,ident__username=self.username).iterator()
        self.first=True
        return self

    def next(self):

        if self.first:
            self.first=False
            return ("","-------")

        station=self.stations.next()
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
        self.fields['station_slug'] = forms.ChoiceField(scelta_stations(username),required=False,label=_('Your station'),help_text=_('Select configurated station'),initial="")


class ManualForm(forms.ModelForm):

    #geom = PointField()

    coordinate_slug= forms.CharField(widget=forms.HiddenInput(),required=False)
    presentweather=forms.ChoiceField(scelta_present_weather(),required=False,label=_('Present weather'),help_text=_('Present weather'),initial="")

    visibility=forms.IntegerField(required=False,label=_("Visibility(m.)"),help_text='',min_value=0,max_value=1000000)
    snow_height=forms.IntegerField(required=False,label=_("Snow height(cm.)"),help_text='',min_value=0,max_value=1000)

    class Meta:
        model = GeorefencedImage
        fields = ('geom',)
        widgets = {'geom': LeafletWidget()}


class RainboWeatherForm(forms.ModelForm):
    coordinate_slug= forms.CharField(widget=forms.HiddenInput(),required=False)
    #fixed standard values from ~/rmap/python/rmap/tables/present_weather.txt  
    visibility_intensity=forms.ChoiceField(widget=forms.RadioSelect(),choices=[(110,'110'),(130,'130')],required=False,label=_("Visibility"),help_text='')
    snow_intensity=forms.ChoiceField(widget=forms.RadioSelect(),choices=[(185,'185'),(186,'186'),(187,'187')],required=False,label=_("Snow"),help_text='')    
    thunderstorm_intensity=forms.ChoiceField(widget=forms.RadioSelect(),choices=[(192,'192'),(193,'193'),(195,'195'),(196,'196')],required=False,label=_("Thunderstorm"),help_text='') 
    rain_intensity=forms.ChoiceField(widget=forms.RadioSelect(),choices=[(150,'150'),(160,'160'),(165,'165'),(184,'184')],required=False,label=_("Rain"),help_text='')
    tornado=forms.ChoiceField(widget=forms.RadioSelect(),choices=[(199,'199')],required=False,label=_("Tornado"),help_text='')

    class Meta:
        model = GeorefencedImage
        fields = ('geom',)
        widgets = {'geom': LeafletWidget()}


class RainboImpactForm(forms.ModelForm):
    coordinate_slug= forms.CharField(widget=forms.HiddenInput(),required=False)
    impact_detected=forms.ChoiceField(widget=forms.RadioSelect(),choices=[(1,'1'),(2,'2'),(3,'3')],required=False,label=_("Impact detected"),help_text='') 

    class Meta:
        model = GeorefencedImage
        fields = ('geom',)
        widgets = {'geom': LeafletWidget()}


class NominatimForm(forms.Form):
    address= forms.CharField(required=False,label=_("Search address"),help_text='')



class NewStationForm(forms.ModelForm):

    #geom = PointField()

    coordinate_slug= forms.CharField(widget=forms.HiddenInput(),required=False)
    name= forms.CharField(required=True,label=_("New station name"),help_text=_('The name of the station to insert'))

    class Meta:
        model = GeorefencedImage
        fields = ('geom',)
        widgets = {'geom': LeafletWidget()}


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
                request.POST['geom']= str(Point(station.lon,station.lat))
                return render(request, 'insertdata/form.html',{'form': form,'stationform':stationform,'nominatimform':nominatimform})
        else:
            stationform = StationForm(request.user.get_username())
            return render(request, 'insertdata/form.html',{'form': form,'stationform':stationform,'nominatimform':nominatimform,"invalid":True})

        if nominatimform.is_valid(): # All validation rules pass

            address=nominatimform.cleaned_data['address']
            if address:
                nom = Nominatim(base_url="http://nominatim.openstreetmap.org",referer= get_current_site(request))
                result=nom.query(address,limit=1,countrycodes="IT")
                if len(result) >= 1:
                    lat= result[0]["lat"]
                    lon= result[0]["lon"]
                    address= result[0]["display_name"]
                    request.POST['geom']= str(Point(float(lon),float(lat)))
                    request.POST['address']= address
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

                import rmap.rmap_core
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
                #TODO: code to define 
                datavar["??????"]={"t": dt,"v": str(value)}
            if (len(datavar)>0):
                try:
                    user=rmap.settings.mqttuser
                    password=rmap.settings.mqttpassword
                    prefix=rmap.settings.topicsample
                    network="mobile"
                    slug=form.cleaned_data['coordinate_slug']

                    print "<",slug,">","prefix:",prefix

                    mqtt=rmapmqtt(ident=ident,lon=lon,lat=lat,network=network,host="localhost",port=1883,prefix=prefix,maintprefix=prefix,username=user,password=password)
                    mqtt.data(timerange="254,0,0",level="1,-,-,-",datavar=datavar)
                    mqtt.disconnect()
                    form = RainboImpactForm() # An unbound Rainbo form
                except Exception as e:
                    return render(request, html_template,{'form': form,"error":True})

            return render(request, html_template,{'form': form})

        else:
            return render(request, html_template,{'form': form,"invalid":True})
                
    else:
        return render(request, html_template,{'form': form})


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
            value = form.cleaned_data['visibility_intensity'] if form.cleaned_data['visibility_intensity'] != "" else ""
            value = form.cleaned_data['rain_intensity'] if form.cleaned_data['rain_intensity'] != ""  else value
            value = form.cleaned_data['snow_intensity'] if form.cleaned_data['snow_intensity'] != "" else value
            value = form.cleaned_data['thunderstorm_intensity'] if form.cleaned_data['thunderstorm_intensity'] != "" else value
            value = form.cleaned_data['tornado'] if form.cleaned_data['tornado'] != "" else value

            if (value != ""):
                datavar["B20003"]={"t": dt,"v": str(value)}
            if (len(datavar)>0):
                try:
                    user=rmap.settings.mqttuser
                    password=rmap.settings.mqttpassword
                    prefix=rmap.settings.topicsample
                    network="mobile"
                    slug=form.cleaned_data['coordinate_slug']
                    print user,password,network,prefix
                    print "<",slug,">","prefix:",prefix
                    mqtt=rmapmqtt(ident=ident,lon=lon,lat=lat,network=network,host="rmap.cc",port=1883,prefix=prefix,maintprefix=prefix,username=user,password=password)
                    #mqtt=rmapmqtt(ident=ident,lon=lon,lat=lat,network=network,host="localhost",port=1883,prefix=prefix,maintprefix=prefix,username=user,password=password)
                    mqtt.data(timerange="254,0,0",level="1,-,-,-",datavar=datavar)
                    mqtt.disconnect()
                    form = RainboWeatherForm() # An unbound Rainbo form
                except Exception as e:
                    print e
                    return render(request, html_template,{'form': form,"error":True})

            return render(request, html_template,{'form': form})

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
        form = ManualForm(request.POST) # A form bound to the POST data

        if stationform.is_valid(): # All validation rules pass

            slug=stationform.cleaned_data['station_slug']
            if slug:
                station=StationMetadata.objects.get(ident__username=request.user.username,slug=slug)
                #stationlat=station.lat
                #stationlon=station.lon

                request.POST['geom']= str(Point(station.lon,station.lat))
                request.POST['coordinate_slug']= slug
                return render(request, 'insertdata/manualdataform.html',{'form': form,'stationform':stationform,'nominatimform':nominatimform})
        else:
            stationform = StationForm(request.user.get_username())
            return render(request, 'insertdata/manualdataform.html',{'form': form,'stationform':stationform,'nominatimform':nominatimform,"invalid":True})

        if nominatimform.is_valid(): # All validation rules pass

            address=nominatimform.cleaned_data['address']
            if address:
                nom = Nominatim(base_url="http://nominatim.openstreetmap.org",referer=get_current_site(request))
                result=nom.query(address,limit=1,countrycodes="IT")
                if len(result) >= 1:
                    lat= result[0]["lat"]
                    lon= result[0]["lon"]
                    address= result[0]["display_name"]
                    request.POST['geom']= str(Point(float(lon),float(lat)))
                    request.POST['address']= address
                return render(request, 'insertdata/manualdataform.html',{'form': form,'stationform':stationform,'nominatimform':nominatimform})
        else:
            nominatimform = NominatimForm()
            return render(request, 'insertdata/manualdataform.html',{'form': form,'stationform':stationform,'nominatimform':nominatimform,"invalid":True})

        if form.is_valid(): # All validation rules pass
            
            geom=form.cleaned_data['geom']
            lon=geom['coordinates'][0]
            lat=geom['coordinates'][1]
            dt=datetime.utcnow().replace(microsecond=0)
            ident=request.user.username

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

            print "datavar:",datavar
            if (len(datavar)>0):
                try:

                    user=rmap.settings.mqttuser
                    password=rmap.settings.mqttpassword
                    prefix=rmap.settings.topicsample

                    slug=form.cleaned_data['coordinate_slug']
                    if (slug):
                        network="fixed"
                    else:
                        network="mobile"

                    print "<",slug,">","prefix:",prefix

                    mqtt=rmapmqtt(ident=ident,lon=lon,lat=lat,network=network,host="localhost",port=1883,prefix=prefix,maintprefix=prefix,username=user,password=password)
                    mqtt.data(timerange="254,0,0",level="1,-,-,-",datavar=datavar)
                    mqtt.disconnect()

                    form = ManualForm() # An unbound form
                except:
                    return render(request, 'insertdata/manualdataform.html',{'form': form,'stationform':stationform,'nominatimform':nominatimform,"error":True})

            return render(request, 'insertdata/manualdataform.html',{'form': form,'stationform':stationform,'nominatimform':nominatimform})

        else:

            print "invalid form"
            form = ManualForm() # An unbound form
            return render(request, 'insertdata/manualdataform.html',{'form': form,'stationform':stationform,'nominatimform':nominatimform,"invalid":True})

    else:
        stationform = StationForm(request.user.get_username()) # An unbound form
        nominatimform = NominatimForm() # An unbound form
        form = ManualForm() # An unbound form
        return render(request, 'insertdata/manualdataform.html',{'form': form,'stationform':stationform,'nominatimform':nominatimform})


@login_required
def insertNewStation(request):

    if request.method == 'POST': # If the form has been submitted...

        nominatimform = NominatimForm(request.POST) # A form bound to the POST data
        newstationform = NewStationForm(request.POST) # A form bound to the POST data

        if nominatimform.is_valid(): # All validation rules pass

            address=nominatimform.cleaned_data['address']
            if address:
                nom = Nominatim(base_url="http://nominatim.openstreetmap.org",referer=get_current_site(request))
                result=nom.query(address,limit=1,countrycodes="IT")
                if len(result) >= 1:
                    lat= result[0]["lat"]
                    lon= result[0]["lon"]
                    address= result[0]["display_name"]
                    request.POST['geom']= str(Point(float(lon),float(lat)))
                    request.POST['address']= address
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

            if name:
                try:
                    print "new station:", name,ident,lon,lat

                    mystation=StationMetadata(slug=slugify(name),name=name)
                    user=User.objects.get(username=ident)
                    mystation.ident=user
                    mystation.lat=lat
                    mystation.lon=lon
                    mystation.active=True

                    mystation.save()

                except:
                    return render(request, 'insertdata/newstationform.html',{'nominatimform':nominatimform,'newstationform':newstationform,"error":True})

            return render(request, 'insertdata/newstationform.html',{'nominatimform':nominatimform,'newstationform':newstationform})

        else:

            print "invalid form"
            form = NewStationForm() # An unbound form
            return render(request, 'insertdata/newstationform.html',{'nominatimform':nominatimform,'newstationform':newstationform,"invalid":True})

    else:
        nominatimform = NominatimForm() # An unbound form
        newstationform = NewStationForm() # An unbound form
        return render(request, 'insertdata/newstationform.html',{'nominatimform':nominatimform,'newstationform':newstationform,})


