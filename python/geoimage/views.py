from .models import GeorefencedImage
from django.shortcuts import render
from django import forms
from datetime import date,datetime,timedelta,time
import pytz
from django.utils import timezone
from django.forms.widgets import SelectDateWidget
from django.utils.translation import ugettext_lazy
from django.core.paginator import Paginator
from django.http import HttpResponse

class ExtremeForm(forms.Form):

    initial_start=date.today()-timedelta(days=10)
    initial_end=date.today()


    this_year = date.today().year-9
    years = list(range(this_year, this_year+10))

    datetime_start = forms.DateTimeField(required=True,initial=initial_start,widget=SelectDateWidget(years=years),label=ugettext_lazy("Starting date"),help_text=ugettext_lazy("Elaborate starting from this date"))

    datetime_end = forms.DateTimeField(required=True,initial=initial_end,widget=SelectDateWidget(years=years),label=ugettext_lazy("Ending date"),help_text=ugettext_lazy("Elaborate ending to this date"))



def geoimagesOnMap(request,ident=None):

    if request.method == 'POST': # If the form has been submitted...
        form = ExtremeForm(request.POST) # A form bound to the POST data
        if form.is_valid(): # All validation rules pass

            datetime_start=form.cleaned_data['datetime_start']
            datetime_end=form.cleaned_data['datetime_end']

            datetime_start = datetime.combine(datetime_start.date(),time(00,00), tzinfo=pytz.utc)
            datetime_end = datetime.combine(datetime_end.date(),time(23,59,59), tzinfo=pytz.utc)

    else:

        now=timezone.now()
        datetime_start=(now-timedelta(days=10))
        datetime_end=now
        form = ExtremeForm() # An unbound form

    if ident is None:
        grimages=GeorefencedImage.objects.filter(active=True,date__gte=datetime_start,date__lte=datetime_end).order_by("date")
    else:
        grimages=GeorefencedImage.objects.filter(active=True,date__gte=datetime_start,date__lte=datetime_end,user__username=ident).order_by("date")

    return render(request, 'geoimage/geoimages_on_map.html',{'form': form,"grimages":grimages,"ident":ident})


def geoimagesByCoordinate(request,lon,lat):
    # the query here is "text" and not a geo-query
    # json come from an unordered dict!
    # so this do not work #geom={'type': 'Point', 'coordinates': [float(lon),float(lat)]}
    coordinate="[{},{}]".format(float(lon),float(lat))
    grimages=GeorefencedImage.objects.filter(active=True,geom__contains=coordinate).order_by("date")
    paginator = Paginator(grimages, 1) # Show 1 image per page.
    page_number = request.GET.get('page',-1)  # start with last page
    page_obj = paginator.get_page(page_number)
    return render(request, 'geoimage/geoimages_by_coordinate.html',{"page_obj":page_obj})

def geoimageById(request,id):
    grimage=GeorefencedImage.objects.get(id=id)
    return render(request, 'geoimage/geoimage_by_id.html',{"grimage":grimage})

class DelGeoimageForm(forms.Form):
    pass


def geoimageDelete(request,id):

    if request.method == 'POST': # If the form has been submitted...

        delgeoimageform = DelGeoimageForm(request.POST) # A form bound to the POST data
        if delgeoimageform.is_valid(): # All validation rules pass

            try:
                if request.user.is_authenticated:
                    grimage = GeorefencedImage.objects.get(user__username=request.user.get_username(),id=id)
                    grimage.delete()    
                else:
                    grimage = GeorefencedImage.objects.get(id=id)
                    return render(request, 'geoimage/geoimage_delete.html',{"grimage":grimage,'delgeoimageform':delgeoimageform,"notauthorized":True})

            except Exception as e:
                print(e)

                grimage = GeorefencedImage.objects.get(id=id)
                return render(request, 'geoimage/geoimage_delete.html',{"grimage":grimage,'delgeoimageform':delgeoimageform,"error":True})

            return render(request, 'geoimage/geoimage_delete.html',{'delgeoimageform':delgeoimageform,"deleted":True})
        else:
            delgeoimageform = DelGeoimageForm()
            grimage = GeorefencedImage.objects.get(id=id)
            return render(request, 'geoimage/geoimage_delete.html',{"grimage":grimage,'delgeoimageform':delgeoimageform,"invalid":True})

    else:
        delgeoimageform = DelGeoimageForm() # An unbound form
        try:

            if request.user.is_authenticated:
               notauthorized=False
            else:
               notauthorized=True
    
            grimage = GeorefencedImage.objects.get(id=id)
            return render(request, 'geoimage/geoimage_delete.html',{"grimage":grimage,'delgeoimageform':delgeoimageform,"notauthorized":notauthorized})


        except Exception as e:
            print(e)
        
            response=HttpResponse("GeorefencedImage matching query does not exist")
            response.status_code=403
            return response
