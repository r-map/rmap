from django.http import HttpResponse
from django.http import HttpResponseRedirect
from django.contrib.auth import authenticate
from django.shortcuts import render
#from django.contrib.sites.shortcuts import get_current_site
from django.contrib.sites.models import get_current_site
from form import WizardForm,WizardForm2
from django.contrib.auth.models import User
from django.db import IntegrityError
from django.core.exceptions import ObjectDoesNotExist
from stations.models import StationMetadata
import settings
import rabbitshovel
import network
from django.contrib.auth.decorators import login_required

MAINSITE="rmapv.rmap.cc"

def home(request):
    current_site = get_current_site(request)

    if current_site.domain == MAINSITE:
        return render(request, 'index.html')
    else:
        return render(request, 'mystation.html')


@login_required
def wizard(request):
    current_site = get_current_site(request)

    if current_site.domain == MAINSITE:
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

                    user = User.objects.create_user(form.cleaned_data['username'], form.cleaned_data['username']+'@casa.it', form.cleaned_data['password'])
                    
                #trap IntegrityError for user that already exist
                except IntegrityError:
                    pass
                except:
                    return HttpResponseRedirect('/wizard_error/')


                try:
                    mystation=StationMetadata.objects.get(slug=form.cleaned_data['station'])
                    user=User.objects.get(username=form.cleaned_data['username'])

                    mystation.ident=user
                    mystation.lat=float(form.cleaned_data['latitude'])
                    mystation.lon=float(form.cleaned_data['longitude'])
                    mystation.active=True
                    mystation.save()

                    for board in mystation.board_set.all():

                        if not board.active: continue
                        try:
                            if ( board.transportamqp.active):
                                print "AMQP Transport", board.transportamqp

                                board.transportamqp.amqpuser=form.cleaned_data['username']
                                board.transportamqp.amqppassword=form.cleaned_data['password']
                                board.transportamqp.save()

                                amqpserver =board.transportamqp.amqpserver
                                amqpuser=board.transportamqp.amqpuser
                                amqppassword=board.transportamqp.amqppassword
                                queue=board.transportamqp.queue
                                exchange=board.transportamqp.exchange

                                sh=rabbitshovel.shovel(srcqueue=queue,destexchange=exchange,destserver=amqpserver)
                                sh.delete()
                                sh.create(destuser=amqpuser,destpassword=amqppassword)

                        except ObjectDoesNotExist:
                            print "transport AMQP not present for this board"

                        try:
                            if ( board.transportmqtt.active):
                                print "MQTT Transport", board.transportmqtt

                                board.transportmqtt.mqttuser=form.cleaned_data['username']
                                board.transportmqtt.mqttpassword=form.cleaned_data['password']
                                board.transportmqtt.save()

                        except ObjectDoesNotExist:
                            print "transport MQTT not present for this board"

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

    if current_site.domain == MAINSITE:
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

    if current_site.domain == MAINSITE:
        return render(request, 'index.html')
    else:
        return render(request, 'wizard_done.html')

def wizard_error(request):
    current_site = get_current_site(request)

    if current_site.domain == MAINSITE:
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

        #write to all in rmap/username/# report/username/#
        if topic.startswith(("rmap/"+username+"/","report/"+username+"/","mobile/"+username+"/")) and acc == "2":
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
    # View code here...
    return render(request, 'profile.html')

#def profile(request):
#    html = "<html><body>This is your personal page. TODO</body></html>"
#    return HttpResponse(html)


