from django.http import HttpResponse
from django.http import HttpResponseRedirect
from django.contrib.auth import authenticate
from django.shortcuts import render
#from django.contrib.sites.shortcuts import get_current_site
from django.contrib.sites.shortcuts import get_current_site
from .form import WizardForm,WizardForm2,StationImageForm
from django.contrib.auth.models import User
from django.db import IntegrityError
from django.core.exceptions import ObjectDoesNotExist
from . import settings
from . import rabbitshovel
from . import network
from django.contrib.auth.decorators import login_required
import rmap.rmap_core
from rmap.stations.models import StationMetadata
from rmap.stations.models import StationImage,PHOTO_CATEGORY_CHOICES
from django.views.decorators.csrf import csrf_exempt
from django.views.decorators.cache import never_cache
import re
from django.contrib.auth.hashers import make_password
from rmap.utils import nint

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
                            station=station_slug,lat=lat,lon=lon,constantdata=constantdata,
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
                        print("nmcli stato:",stato)
                        print("nmcli stdout=",net.stdout)
                        print("nmcli stderr=",net.stderr)

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

@csrf_exempt
def user(request):
    if 'username' in request.GET and 'password' in request.GET:
        username = request.GET['username']
        password = request.GET['password']
        user = authenticate(username=username, password=password)
        if user:
            if user.is_superuser:
                return HttpResponse("allow administrator")
            else:
                return HttpResponse("allow")
    return HttpResponse("deny")

@csrf_exempt
def vhost(request):
    return HttpResponse("allow")

@csrf_exempt
def resource(request):

    # username - the name of the user
    # vhost - the name of the virtual host containing the resource
    # resource - the type of resource (exchange, queue, topic)
    # name - the name of the resource
    # permission - the access level to the resource (configure, write, read) - see the Access Control guide for their meaning


    if 'name' in request.GET and 'permission' in request.GET:
        name = request.GET['name']
        permission = request.GET['permission']

        if (permission == "configure"):
            return HttpResponse("deny")

        if (len(name)>0):
            if (name[0] == "."):
                return HttpResponse("allow")

            # TO BE REMOVED !!!!!!
            # we use new stantard for resources
            # "." is separator in resource name and the first word is username
            # no username is for all users
            if (name == "configuration"):
                return HttpResponse("allow")
            if (name == "photo"):
                return HttpResponse("allow")

    return HttpResponse("deny")


#MQTT auth for mosquitto

from django.views.decorators.csrf import csrf_exempt   

@csrf_exempt  
def auth(request):
    '''
    used for per user authentication
    we check here and do not demand to mosquitto plugin

    '''
    
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
def auth_sha(request):
    '''
    used for per user authentication

    doble check:
    * first django auth
    * second send sha to mosquitto auth plugin for recheck
    
    we need this becouse mosquitto pluin use the same method for pskkey
    to return to mosquitto for TLS
    
    '''
    if 'username' in request.POST and 'password' in request.POST:
        username = request.POST['username']
        password = request.POST['password']
        user = authenticate(username=username, password=password)
        #print username,password
        if ( user and password is not None and password != ""):

#    if 'username' in request.POST:
#        username = request.POST['username']
            u = User.objects.get(username__exact=username)        
            response=HttpResponse(u.password)
            response.status_code=200
            return response

    response=HttpResponse("deny")
    response.status_code=403
    return response



@csrf_exempt
def sha(request):
    '''
    used for per station authentication

    Slug only Contains:
    Letters : a-z,A-Z
    Numbers : 0-9
    Underscores : _
    Hyphens : -
    '''

    payload ="deny"
    p = re.compile('^[a-zA-Z0-9_-]{1,30}$')

    if 'username' in request.POST:
        username_station_board=request.POST['username']
        usb=username_station_board.split("/")

        if(len(usb) == 3):
            username=p.match(usb[0])
            if username:
                username = username.string
                
            station_slug = p.match(usb[1])
            if station_slug:
                station_slug = station_slug.string
            
            board_slug = p.match(usb[2])
            if board_slug:
                board_slug = board_slug.string
 
            if (username and station_slug and board_slug):
                try:
                    mystation=StationMetadata.objects.get(user__username=username,slug=station_slug)
                    if mystation is not None:
                        if mystation.active:
                            myboard = mystation.board_set.get(slug=board_slug)
                            if myboard is not None:
                                if ( myboard.active and myboard.transportmqtt.active):
                                    myuser = myboard.transportmqtt.mqttuser
                                    mypassword = myboard.transportmqtt.mqttpassword
                                    
                                    if (mypassword):
                                        response=HttpResponse(make_password(mypassword))
                                        response.status_code=200
                                        return response
                        
                except ObjectDoesNotExist:
                    payload="MQTT PSKKEY not present for this user/station/board"
        else:
            # fallback to legacy per user authentication  //   will be removed
            return auth_sha(request)

    response=HttpResponse(payload)
    response.status_code=403
    return response


@csrf_exempt
def pskkey(request):
    '''
    used for per station authentication
    PSK TLS on mosquitto

    Slug only Contains:
    Letters : a-z,A-Z
    Numbers : 0-9
    Underscores : _
    Hyphens : -
    '''

    payload ="deny"
    p = re.compile('^[a-zA-Z0-9_-]{1,30}$')

    if 'username' in request.POST:
        username_station_board=request.POST['username']
        usb=username_station_board.split("/")

        if(len(usb) == 3):
            username=p.match(usb[0])
            if username:
                username = username.string
                
            station_slug = p.match(usb[1])
            if station_slug:
                station_slug = station_slug.string
            
            board_slug = p.match(usb[2])
            if board_slug:
                board_slug = board_slug.string
 
            if (username and station_slug and board_slug):
                try:
                    mystation=StationMetadata.objects.get(user__username=username,slug=station_slug)
                    if mystation is not None:
                        if mystation.active:
                            myboard = mystation.board_set.get(slug=board_slug)
                            if myboard is not None:
                                if ( myboard.active and myboard.transportmqtt.active):
                                    myuser = myboard.transportmqtt.mqttuser
                                    mypskkey = myboard.transportmqtt.mqttpskkey
                                    
                                    if (mypskkey):
                                        response=HttpResponse(mypskkey)
                                        response.status_code=200
                                        return response
                        
                except ObjectDoesNotExist:
                    payload="MQTT PSKKEY not present for this user/station/board"

    response=HttpResponse(payload)
    response.status_code=403
    return response


@csrf_exempt  
def superuser(request):

    if 'username' in request.POST:
        username = request.POST['username']
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
    '''ACC:
    1 - read access
    2 - write access
    3 - read and write access
    4 - subscribe access
    8 - unsubscribe access

    #define MOSQ_ACL_NONE 0x00
    #define MOSQ_ACL_READ 0x01
    #define MOSQ_ACL_WRITE 0x02
    #define MOSQ_ACL_SUBSCRIBE 0x04
    #define MOSQ_ACL_UNSUBSCRIBE 0x08

    access will be one of: MOSQ_ACL_SUBSCRIBE when a client is asking
    to subscribe to a topic string.  This differs from MOSQ_ACL_READ
    in that it allows you to deny access to topic strings rather than
    by pattern.  For example, you may use MOSQ_ACL_SUBSCRIBE to deny
    subscriptions to ‘#’, but allow all topics in MOSQ_ACL_READ.  This
    allows clients to subscribe to any topic they want, but not
    discover what topics are in use on the server.  MOSQ_ACL_READ when
    a message is about to be sent to a client (i.e. whether it can
    read that topic or not).  MOSQ_ACL_WRITE when a message has been
    received from a client (i.e. whether it can write to that topic or
    not).

    '''

    if 'topic' in request.POST and 'acc' in request.POST :
        topic = request.POST['topic']
        acc = request.POST['acc']
        #print (topic,acc)
        
        ##read and subscribe to all
        #if acc == "1" or acc == "4":
        #    response=HttpResponse("allow")
        #    response.status_code=200
        #    return response

        ##read and write to all in test/#
        #if topic.startswith(("test/")) and (acc == "2" or acc == "3"):
        #    response=HttpResponse("allow")
        #    response.status_code=200
        #    return response

    if 'username' in request.POST:
        p = re.compile('^[a-zA-Z0-9_-]{1,30}$')
        username_station_board=request.POST['username']
        usb=username_station_board.split("/")

        if(len(usb) == 3):
            # new acl for user/station_slug/board_slug auth
            username=p.match(usb[0])
            if username:
                username = username.string
                
            station_slug = p.match(usb[1])
            if station_slug:
                station_slug = station_slug.string
            
            board_slug = p.match(usb[2])
            if board_slug:
                board_slug = board_slug.string
 
            if (username and station_slug and board_slug):
                try:
                    mystation=StationMetadata.objects.get(user__username=username,slug=station_slug)
                    if mystation is not None:
                        if mystation.active:
                            lat=mystation.lat
                            lon=mystation.lon
                            myboard = mystation.board_set.get(slug=board_slug)
                            if myboard is not None:
                                if ( myboard.active and myboard.transportmqtt.active):
                                    username = myboard.transportmqtt.mqttuser
                                    mynetwork=mystation.network
                                    if lat is None:
                                        mytopic="/"
                                    else:
                                        mytopic="/%d,%d/" % (nint(mystation.lon*100000),nint(mystation.lat*100000))

                                    #read and write and subscribe to all in 1/report/username/ident/# 1/sample/username/ident/# 1/maint/username/ident/# 1/rpc/username/ident/#
                                    if topic.startswith(
                                            (
                                                "1/sample/"+username+"/"+mytopic,
                                                "1/report/"+username+"/"+mytopic,
                                                "1/maint/"+username+"/"+mytopic,
                                                "1/rpc/"+username+"/"+mytopic
                                            )
                                    ):
                                        if (topic.split("/")[5] == mynetwork):
                                            response=HttpResponse("allow")
                                            response.status_code=200
                                            return response
                                        
                except ObjectDoesNotExist:
                    pass

        else:

            #read and un/subscribe to all user's stations
            if acc == "1" or acc == "4" or acc == "8":
                
                username=p.match(request.POST['username'])
                if username:
                    username = username.string
                    mytopic="/"

                    #read and subscribe to all in report/username/# sample/username/# maint/username/# rpc/username/#
                    if topic.startswith(
                            (
                                "1/sample/"+username+mytopic,
                                "1/report/"+username+mytopic,
                                "1/maint/"+username+mytopic,
                                "1/rpc/"+username+mytopic
                            )
                    ):
                        
                        response=HttpResponse("allow")
                        response.status_code=200
                        return response


            # legacy acl for user auth
            username=p.match(request.POST['username'])
            if username:
                username = username.string
                mytopic="/"

                #read and write and subscribe to all in report/username/# sample/username/# maint/username/# rpc/username/#
                if topic.startswith(
                        (
                            "sample/"+username+mytopic,
                            "report/"+username+mytopic,
                            "maint/"+username+mytopic,
                            "rpc/"+username+mytopic
                        )
                ):
                    # check possible network
                    if (topic.split("/")[3] == "fixed" or topic.split("/")[3] == "mobile"):
                        response=HttpResponse("allow")
                        response.status_code=200
                        return response
                
    response=HttpResponse("deny")
    response.status_code=403
    return response


#user profile
from django.contrib.auth.decorators import login_required

@login_required
@never_cache
def profile(request):
    stations=StationMetadata.objects.filter(active=True,user__username=request.user.get_username())
    return render(request, 'profile.html',{ 'user' : request.user.get_username(),"stations":stations})

@login_required
@never_cache
def profile_details(request,mystation_slug):

    if request.method == 'POST': # If the form has been submitted...
        
        if (request.POST.get('stationimageid') is not None and request.POST.get('stationslug') is not None):
            try:
                stationimageid=request.POST['stationimageid']
                mystationslug=request.POST['stationslug']
                
                mystation=StationMetadata.objects.get(user__username=request.user.get_username(),slug=mystation_slug)
                stationimage=StationImage.objects.get(stationmetadata=mystation,id=stationimageid)
                stationimage.delete()
                invalid=False
            except:
                invalid=True
            form = StationImageForm() # An unbound form
            return render(request, 'profile_details.html',{"user":request.user.get_username(),"mystation":mystation,'form': form,"invalid":invalid})
            

        form = StationImageForm(request.POST, request.FILES) # A form bound to the POST data
        if form.is_valid(): # All validation rules pass

                #from rmap import exifutils
                comment=form.cleaned_data['comment']
                #geom=form.cleaned_data['geom']
                image=request.FILES['image']
                #dt=datetime.utcnow().replace(microsecond=0)
                #lon=geom['coordinates'][0]
                #lat=geom['coordinates'][1]
                #image=image.read()
                #body=exifutils.setgeoimage(image,lat,lon,imagedescription=request.user.username,usercomment=comment)

                mystation=StationMetadata.objects.get(user__username=request.user.get_username(),slug=mystation_slug)
                stationimage=StationImage(active=True,comment=comment,stationmetadata=mystation,
                                          #date=dt,
                                          category = PHOTO_CATEGORY_CHOICES[0][0],image=image)

                #stationimage.image.save('stationimage.jpg',ContentFile(body))

                stationimage.save()

        else:

            form = StationImageForm() # An unbound form
            mystation=StationMetadata.objects.get(user__username=request.user.get_username(),slug=mystation_slug)
            return render(request, 'profile_details.html',{"user":request.user.get_username(),"mystation":mystation,'form': form,"invalid":True})

    form = StationImageForm() # An unbound form
    mystation=StationMetadata.objects.get(user__username=request.user.get_username(),slug=mystation_slug)
    return render(request, 'profile_details.html',{"user":request.user.get_username(),"mystation":mystation,'form': form})


@login_required
@never_cache
def profile_details_stationimage(request,mystation_slug,stationimage_id):
    stationimage=StationImage.objects.get(stationmetadata__user__username=request.user.get_username(),stationmetadata__slug=mystation_slug,id=stationimage_id)
    return render(request, 'profile_details_stationimage.html',{"stationimage":stationimage})

#def profile(request):
#    html = "<html><body>This is your personal page. TODO</body></html>"
#    return HttpResponse(html)


