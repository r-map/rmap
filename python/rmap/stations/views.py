from django.views.generic import ListView
from django.views.generic.detail import DetailView
from .models import StationMetadata
from django.shortcuts import render
from django import forms
from django.contrib.auth.decorators import login_required
from django.http import HttpResponse
from rmap import rmap_core
from datetime import datetime,timedelta
from django.core.paginator import Paginator
from django.db.models import Q
from django.shortcuts import get_object_or_404


class StationList(ListView):
    paginate_by = 25
    model = StationMetadata
    def get_queryset(self):

        if 'search' in self.request.GET:
            objects = StationMetadata.objects.filter(
                Q(ident__username__icontains=self.request.GET['search']) | Q(slug__icontains=self.request.GET['search'])
            )
        else:
            objects = StationMetadata.objects.all()
        
        return objects

    
class StationDetail(DetailView):
    model = StationMetadata


class DelStationForm(forms.Form):
    pass


@login_required
def mystationmetadata_del(request,ident,slug):

    if request.method == 'POST': # If the form has been submitted...

        delstationform = DelStationForm(request.POST) # A form bound to the POST data

        if delstationform.is_valid(): # All validation rules pass

            try:
                username=request.user.get_username()
                if (ident == username):
                    print("del station:", ident,slug,username)
                    mystation=StationMetadata.objects.get(slug__exact=slug,ident__username=username)
                    mystation.delete()
                else:
                    print("notautorized")
                    return render(request, 'insertdata/delstationform.html',{'delstationform':delstationform,"invalid":True,"notauthorized":True})

            except Exception as e:
                print(e)
                return render(request, 'insertdata/delstationform.html',{'delstationform':delstationform,"error":True})

            return render(request, 'insertdata/delstationform.html',{'delstationform':delstationform,"deleted":True})
        else:
            delstationform = DelStationForm()
            return render(request, 'insertdata/delstationform.html',{'delstationform':delstationform,"invalid":True})

    else:
        delstationform = DelStationForm() # An unbound form
        return render(request, 'insertdata/delstationform.html',{'delstationform':delstationform})



def mystationmetadata_list(request,ident):
    mystations=StationMetadata.objects.filter(ident__username=ident)
    paginator = Paginator(mystations, 25) # Show 25 contacts per page.
    page_number = request.GET.get('page')
    page_obj = paginator.get_page(page_number)
    return render(request, 'stations/stationmetadata_list.html',{"page_obj":page_obj,"ident":ident})

def mystationmetadata_detail(request,ident,slug):

    now=datetime.utcnow()
    showdate=(now-timedelta(minutes=30))
    year='{:04d}'.format(showdate.year)
    month='{:02d}'.format(showdate.month)
    day='{:02d}'.format(showdate.day)
    hour='{:02d}'.format(showdate.hour)
    
    mystation=get_object_or_404(StationMetadata,ident__username=ident,slug=slug)
    return render(request, 'stations/stationmetadata_detail.html',{"object":mystation,"year":year,"month":month,"day":day,"hour":hour})

def mystation_localdata(request,ident,slug):
    mystation=get_object_or_404(StationMetadata,ident__username=ident,slug=slug)
    return render(request, 'stations/stationlocaldata.html',{"object":mystation})


def mystationmetadata_json(request,ident,station_slug,board_slug=None):
    return HttpResponse(rmap_core.dumpstation(ident,station_slug,board_slug), content_type="application/json")

def StationsOnMap(request,ident=None,slug=None):
    if ident is None:
        stations=StationMetadata.objects.exclude(lat=0,lon=0)
    else:
        if slug is None:
            stations=StationMetadata.objects.filter(ident__username=ident).exclude(lat=0,lon=0)
        else:
            stations=StationMetadata.objects.filter(ident__username=ident,slug=slug).exclude(lat=0,lon=0)

    return render(request, 'stations/stationsonmap.html',{"stations":stations,"ident":ident,"slug":slug})
