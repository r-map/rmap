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
from django.views.decorators.csrf import csrf_exempt
from django.core import serializers
from rmap.jsonrpc import TransportHTTPREPONSE


class StationList(ListView):
    paginate_by = 25
    model = StationMetadata
    def get_queryset(self):

        if 'search' in self.request.GET:
            objects = StationMetadata.objects.filter(
                Q(user__username__icontains=self.request.GET['search']) | Q(slug__icontains=self.request.GET['search'])
            )
        else:
            objects = StationMetadata.objects.all()
        
        return objects

    
class StationDetail(DetailView):
    model = StationMetadata


class DelStationForm(forms.Form):
    pass


@login_required
def mystationmetadata_del(request,user,slug):

    if request.method == 'POST': # If the form has been submitted...

        delstationform = DelStationForm(request.POST) # A form bound to the POST data

        if delstationform.is_valid(): # All validation rules pass

            try:
                username=request.user.get_username()
                if (user == username):
                    print("del station:", slug,username)
                    mystation=StationMetadata.objects.get(slug__exact=slug,user__username=username)
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



def mystationmetadata_list(request,user):
    mystations=StationMetadata.objects.filter(user__username=user)
    paginator = Paginator(mystations, 25) # Show 25 contacts per page.
    page_number = request.GET.get('page')
    page_obj = paginator.get_page(page_number)
    return render(request, 'stations/stationmetadata_list.html',{"page_obj":page_obj,"user":user})

def mystationmetadata_detail(request,user,slug):

    now=datetime.utcnow()
    showdate=(now-timedelta(minutes=30))
    year='{:04d}'.format(showdate.year)
    month='{:02d}'.format(showdate.month)
    day='{:02d}'.format(showdate.day)
    hour='{:02d}'.format(showdate.hour)
    
    mystation=get_object_or_404(StationMetadata,user__username=user,slug=slug)
    return render(request, 'stations/stationmetadata_detail.html',{"object":mystation,"year":year,"month":month,"day":day,"hour":hour})

def mystation_localdata(request,user,slug):
    mystation=get_object_or_404(StationMetadata,user__username=user,slug=slug)
    return render(request, 'stations/stationlocaldata.html',{"object":mystation})


@csrf_exempt
def mystationmetadata_upload_json(request):

    if request.user.is_authenticated:
        body=request.POST.get("body")
        #At this point we can check if we trust this authenticated user... 
        user = request.user.username
        print("Received from user: %r" % user) 
        
        #but we check that message content is with the same user
        try:
            for deserialized_object in serializers.deserialize("json",body):
                if rmap_core.object_auth(deserialized_object.object,user):
                    try:
                        StationMetadata.objects.get(slug=deserialized_object.object.slug,user__username=deserialized_object.object.user.username).delete()
                    except:
                        pass
                    print("save:",deserialized_object.object)
                    deserialized_object.save(force_insert=True)
                else:
                    response=HttpResponse("deny")
                    response.status_code=500
                    return response
                    
        except Exception as e:
            print(("error in deserialize object; skip it",e))
            response=HttpResponse("error")
            response.status_code=500
            return response

        response=HttpResponse("OK")
        response.status_code=200
        return response

    response=HttpResponse("deny")
    response.status_code=403
    return response


def mystationmetadata_config(request,user,station_slug):
    
    response = HttpResponse()
    transport=TransportHTTPREPONSE(response=response)

    rmap_core.configstation(transport=transport,station_slug=station_slug,
                            username=user,
                            notification=True,version="4", without_password=True)

    return response
    

def mystationmetadata_json(request,user,station_slug,board_slug=None,dump=False):
    if request.user.is_authenticated:
        if request.user.username == user:
            jsonstation = rmap_core.dumpstation(user,station_slug,board_slug,dump=dump)
        else:
            response=HttpResponse("deny")
            response.status_code=403
            return response
    else:
        jsonstation = rmap_core.dumpstation(user,station_slug,board_slug, without_password=True,dump=dump)

    if jsonstation is None:
        response=HttpResponse("error")
        response.status_code=403
        return response
    else:
        return HttpResponse(jsonstation, content_type="application/json")

    
def StationsOnMap(request,user=None,slug=None):
    if user is None:
        stations=StationMetadata.objects.exclude(lat=None,lon=None)
    else:
        if slug is None:
            stations=StationMetadata.objects.filter(user__username=user).exclude(lat=None,lon=None)
        else:
            stations=StationMetadata.objects.filter(user__username=user,slug=slug).exclude(lat=None,lon=None)

    print(stations,user,slug)
    return render(request, 'stations/stationsonmap.html',{"stations":stations,"user":user,"slug":slug})
