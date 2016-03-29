from django.views.generic import ListView
from django.views.generic.detail import DetailView
from models import StationMetadata
from django.shortcuts import render

class StationList(ListView):
    model = StationMetadata

class StationDetail(DetailView):
    model = StationMetadata


def mystationmetadata_list(request,ident):
    mystations=StationMetadata.objects.filter(ident__username=ident)
    return render(request, 'stations/stationmetadata_list.html',{"object_list":mystations})

def mystationmetadata_detail(request,ident,slug):
    mystation=StationMetadata.objects.get(ident__username=ident,slug=slug)
    return render(request, 'stations/stationmetadata_detail.html',{"object":mystation})


def StationsOnMap(request,ident=None,slug=None):
    if ident is None:
        stations=StationMetadata.objects.exclude(lat=0,lon=0)
    else:
        if slug is None:
            stations=StationMetadata.objects.filter(ident__username=ident).exclude(lat=0,lon=0)
        else:
            stations=StationMetadata.objects.filter(ident__username=ident,slug=slug).exclude(lat=0,lon=0)

    print stations
    return render(request, 'stations/stationsonmap.html',{"stations":stations,"ident":ident,"slug":slug})
