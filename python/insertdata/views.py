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
from  rmap.tables import Table
import os
from django.utils.translation import ugettext as _
from django.core.files import File
from tempfile import NamedTemporaryFile
from django.core.files.base import ContentFile
from rmap.rmapstation import rmapmqtt

lang="it"


class scelta():
    '''
    build choices for build 
    '''

    def __init__(self):
        self.table = Table(os.path.join(os.path.dirname(__file__), "../rmap/tables","present_weather_"+lang+".txt"))

    def __iter__(self):
        self.iter=self.table.__iter__()
        return self

    def next(self):
        entry=self.iter.next()
        return (self.table[entry].code,self.table[entry].description)

class ImageForm(forms.ModelForm):
    #geom = PointField()

    class Meta:
        model = GeorefencedImage
        fields = ('geom','image','comment')
        widgets = {'geom': LeafletWidget()}


class ManualForm(forms.ModelForm):

    #geom = PointField()
    presentweather=forms.ChoiceField(choices=scelta(),required=False,label=_('Present weather'),help_text=_('Present weather'))
    class Meta:
        model = GeorefencedImage
        fields = ('geom','presentweather')
        widgets = {'geom': LeafletWidget()}


from django.contrib.auth.decorators import login_required

@login_required
def insertDataImage(request):

    if request.method == 'POST': # If the form has been submitted...
        form = ImageForm(request.POST, request.FILES) # A form bound to the POST data
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
            return render(request, 'insertdata/form.html',{'form': form})

    else:
            form = ImageForm() # An unbound form
            return render(request, 'insertdata/form.html',{'form': form})



@login_required
def insertDataManualData(request):

    if request.method == 'POST': # If the form has been submitted...
        form = ManualForm(request.POST, request.FILES) # A form bound to the POST data
        if form.is_valid(): # All validation rules pass

            value=form.cleaned_data['presentweather']
            geom=form.cleaned_data['geom']
            print geom
            lon=geom['coordinates'][0]
            lat=geom['coordinates'][1]
            dt=datetime.utcnow().replace(microsecond=0)


            datavar={"B20003":{"t": dt,"v": str(value)}}

            print datavar
            stationslug=None
            boardslug=None
            ident=request.user.username

            mqtt=rmapmqtt(ident=ident,lon=lon,lat=lat,network="rmap",host="rmap.cc",port=1883,prefix="test",maintprefix="test")
            mqtt.loop()
            mqtt.data(timerange="254,0,0",level="1,-,-,-",datavar=datavar)
            mqtt.loop()
            mqtt.diconnect()

            return render(request, 'insertdata/manualdataform.html',{'form': form})

        else:

            form = ManualForm() # An unbound form
            return render(request, 'insertdata/manualdataform.html',{'form': form})

    else:
            form = ManualForm() # An unbound form
            return render(request, 'insertdata/manualdataform.html',{'form': form})
