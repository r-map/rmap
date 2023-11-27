from .models import GeorefencedImage
from django.shortcuts import render
from django import forms
from datetime import date,datetime,timedelta,time
from django.utils import timezone
from django.forms.widgets import SelectDateWidget
from django.utils.translation import ugettext_lazy
from django.core.paginator import Paginator

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

            datetime_start = datetime.combine(datetime_start.date(),time(00,00))
            datetime_end = datetime.combine(datetime_end.date(),time(23,59,59))

    else:

        now=timezone.now()
        datetime_start=(now-timedelta(days=10))
        datetime_end=now
        form = ExtremeForm() # An unbound form

    if ident is None:
        grimages=GeorefencedImage.objects.filter(date__gte=datetime_start,date__lte=datetime_end).order_by("date")
    else:
        grimages=GeorefencedImage.objects.filter(date__gte=datetime_start,date__lte=datetime_end,user__username=ident).order_by("date")

    return render(request, 'geoimage/geoimages_on_map.html',{'form': form,"grimages":grimages,"ident":ident})


def geoimagesByCoordinate(request,lon,lat):
    geom={'type': 'Point', 'coordinates': [float(lon),float(lat)]}
    grimages=GeorefencedImage.objects.filter(geom=geom).order_by("date")
    paginator = Paginator(grimages, 1) # Show 1 image per page.
    page_number = request.GET.get('page',-1)  # start with last page
    page_obj = paginator.get_page(page_number)
    return render(request, 'geoimage/geoimages_by_coordinate.html',{"page_obj":page_obj})

def geoimageByIdentId(request,ident,id):
    grimage=GeorefencedImage.objects.get(user__username=ident,id=id)
    return render(request, 'geoimage/geoimage_by_ident_id.html',{"grimage":grimage})

