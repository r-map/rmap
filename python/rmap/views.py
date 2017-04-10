from django.http import HttpResponse
from django.http import HttpResponseRedirect
from django.contrib.auth import authenticate
from django.shortcuts import render
#from django.contrib.sites.shortcuts import get_current_site
from django.contrib.sites.shortcuts import get_current_site
from form import WizardForm,WizardForm2
from django.contrib.auth.models import User
from django.db import IntegrityError
from django.core.exceptions import ObjectDoesNotExist
from stations.models import StationMetadata
import settings
import rabbitshovel
import network
from django.contrib.auth.decorators import login_required
import rmap.rmap_core
from rmap.stations.models import StationMetadata


def home(request):
    current_site = get_current_site(request)

    if current_site.domain in settings.MAINSITES:
        return render(request, 'index.html')
    else:
        return render(request, 'mystation.html')


@login_required
def wizard(request):
    current_site = get_current_site(request)

    if current_site.domain in settings.MAINSITES:
        return render(request, 'index.html')
    else:

        # if this is a POST request we need to process the form data
        if request.method == 'POST':
            # create a form instance and populate it with data from the request:
            form = WizardForm(request.POST)
            # check whether it's valid:
            if form.is_valid():
                # process the data in form.cleaned_data as required

                try:
                    username = form.cleaned_data['username']
                    password=form.cleaned_data['password']
                    station_slug=form.cleaned_data['station']
                    lat=form.cleaned_data['latitude']
                    lon=form.cleaned_data['longitude']

                    height=str(int(form.cleaned_data['height']*10))
                    stationname=form.cleaned_data['stationname']
                    constantdata={}
                    constantdata["B01019"]=stationname
                    constantdata["B07030"]=height

                    rmap.rmap_core.configdb(username=username,password=password,
                                            station=station_slug,lat=lat,lon=lon,constantdata={"B07030":height},
                            mqttusername=username,
                            mqttpassword=password,
                            #mqttserver=args.server,
                            #mqttsamplerate=args.samplerate,
                            #bluetoothname=args.bluetoothname,
                            amqpusername=username,
                            amqppassword=password,
                            #amqpserver=args.server,
                            #queue=args.queue,
                            #exchange=args.exchange
                    )

                except:
                    return HttpResponseRedirect('/wizard_error/')

                return HttpResponseRedirect('/wizard2/')


        # if a GET (or any other method) we'll create a blank form
        else:
            form = WizardForm()
            return render(request, 'wizard.html', {'form': form.as_p})

        return render(request, 'wizard.html', {'form': form.as_p})


@login_required
def wizard2(request):
    current_site = get_current_site(request)

    if current_site.domain in settings.MAINSITES:
        return render(request, 'index.html')
    else:

        # if this is a POST request we need to process the form data
        if request.method == 'POST':
            # create a form instance and populate it with data from the request:
            form = WizardForm2(request.POST)
            # check whether it's valid:
            if form.is_valid():
                # process the data in form.cleaned_data as required

                try:

                    ssid=form.cleaned_data['ssid']
                    password=form.cleaned_data['password']

                    if ssid != "":
                        net=network.wifi()
                        stato=net.create(ssid=ssid ,password=password)
                        print "nmcli stato:",stato
                        print "nmcli stdout=",net.stdout
                        print "nmcli stderr=",net.stderr

                        if stato != 0 :
                            return HttpResponseRedirect('/wizard_error/')

                except:
                    return HttpResponseRedirect('/wizard_error/')

                return HttpResponseRedirect('/wizard_done/')


        # if a GET (or any other method) we'll create a blank form
        else:
            form = WizardForm2()
            return render(request, 'wizard2.html', {'form': form.as_p})

        return render(request, 'wizard2.html', {'form': form.as_p})


def wizard_done(request):
    current_site = get_current_site(request)

    if current_site.domain in settings.MAINSITES:
        return render(request, 'index.html')
    else:
        return render(request, 'wizard_done.html')

def wizard_error(request):
    current_site = get_current_site(request)

    if current_site.domain in settings.MAINSITES:
        return render(request, 'index.html')
    else:
        return render(request, 'wizard_error.html')


#AMQP auth for rabbitmq

def user(request):
    if 'username' in request.GET and 'password' in request.GET:
        username = request.GET['username']
        password = request.GET['password']
        user = authenticate(username=username, password=password)
        if user:
            if user.is_superuser:
                return HttpResponse("allow administrator")
            else:
                return HttpResponse("allow management")
    return HttpResponse("deny")

def vhost(request):
    return HttpResponse("allow")

def resource(request):
    return HttpResponse("allow")


#MQTT auth for mosquitto

from django.views.decorators.csrf import csrf_exempt   

@csrf_exempt  
def auth(request):
    if 'username' in request.POST and 'password' in request.POST:
        username = request.POST['username']
        password = request.POST['password']
        user = authenticate(username=username, password=password)
        #print username,password
        if user:
            response=HttpResponse("allow")
            response.status_code=200
            return response

    response=HttpResponse("deny")
    response.status_code=403
    return response

@csrf_exempt  
def superuser(request):

    if 'username' in request.POST and 'password' in request.POST:
        username = request.POST['username']
        password = request.POST['password']
        #print username,password

        #        user = authenticate(username=username, password=password)
        #        if user:
        #            if user.is_superuser:
        #                response=HttpResponse("allow administrator")
        #                response.status_code=200
        #                return response
        #            else:
        #                response=HttpResponse("allow management")
        #                response.status_code=403
        #                return response
        
        # rmap as superuser
        if username == "rmap":
            response=HttpResponse("allow")
            response.status_code=200
            return response
                
    response=HttpResponse("deny")
    response.status_code=403
    return response

@csrf_exempt  
def acl(request):

    if 'username' in request.POST and 'topic' in request.POST and 'acc' in request.POST :
        username = request.POST['username']
        topic = request.POST['topic']
        acc = request.POST['acc']
        #print username,topic,acc

        #read to all
        if acc == "1":
            response=HttpResponse("allow")
            response.status_code=200
            return response

        #write to all in test/#
        if topic.startswith(("test/")) and acc == "2":
            response=HttpResponse("allow")
            response.status_code=200
            return response

        #write to all in rmap/username/# report/username/# mobile/username/# plus new sample/username/# fixed/username/# and rpc/username/#
        if topic.startswith(("sample/"+username+"/","rmap/"+username+"/","report/"+username+"/","fixed/"+username+"/","mobile/"+username+"/","rpc/"+username+"/")) and acc == "2":
            response=HttpResponse("allow")
            response.status_code=200
            return response

    response=HttpResponse("deny")
    response.status_code=403
    return response


#user profile
from django.contrib.auth.decorators import login_required

@login_required
def profile(request):
    stations=StationMetadata.objects.filter(active=True,ident__username=request.user.get_username())
    return render(request, 'profile.html',{ 'ident' : request.user.get_username(),"stations":stations})

@login_required
def profile_details(request,mystation_slug):

    mystation=StationMetadata.objects.get(ident__username=request.user.get_username(),slug=mystation_slug)
    return render(request, 'profile_details.html',{"ident":request.user.get_username(),"mystation":mystation})

#def profile(request):
#    html = "<html><body>This is your personal page. TODO</body></html>"
#    return HttpResponse(html)


