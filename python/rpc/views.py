from django.views.generic import ListView
from django.views.generic.detail import DetailView
from .models import Rpc
from django.shortcuts import render
from django import forms
from django.contrib.auth.decorators import login_required
from django.http import HttpResponse
from rmap import rmap_core
from django.core.paginator import Paginator
from django.shortcuts import get_object_or_404
from django.db.models import Q
from django.views.decorators.cache import never_cache
from django import forms
from rmap.stations.models import StationMetadata
from django.contrib.auth.decorators import login_required
from rpc.models import Rpc
from jsonfield import JSONField
import json
from datetime import datetime,timedelta

@never_cache
def rpc_list(request,user=None,slug=None):

    if (user is None):
        query_user=Q()
    else:
        query_user = Q(stationmetadata__user__username=user) 

    if (slug is None):
        query_slug=Q()
    else:
        query_slug = Q(stationmetadata__slug=slug)


    if 'search' in request.GET:
        search = request.GET['search']
        query =  (Q(stationmetadata__slug__icontains=search))
    else:
        query = Q()
        search = None
        
    if 'status' in request.GET:
        status = request.GET['status']

        if (status == "submitted"):
            query_status =  (Q(active=True) & Q(datecmd__isnull=True) & Q(dateres__isnull=True))
        elif (status == "running"):
            query_status =  (Q(active=False) & Q(datecmd__isnull=False) & Q(dateres__isnull=True))
        elif (status == "completed"):
            query_status =  (Q(active=False) & Q(datecmd__isnull=False) & Q(dateres__isnull=False))
        else:
            query_status = Q()
            
    else:
        query_status = Q()
        status=None
        
    query = query & query_user & query_slug & query_status
                         
    myrpc = Rpc.objects.filter(query).distinct()

    paginator = Paginator(myrpc, 25) # Show 25 contacts per page.
    page_number = request.GET.get('page')
    page_obj = paginator.get_page(page_number)
    return render(request, 'rpc/rpc_list.html',{"page_obj":page_obj,"user":user,"slug":slug,"search":search,"status":status})
    
@never_cache
def rpc_details(request,user=None,slug=None,id=None):

    myrpc = Rpc.objects.get(stationmetadata__user__username=user,stationmetadata__slug=slug,id=id)
    return render(request, 'rpc/rpc_details.html',{"myrpc":myrpc})


class RpcSubmitFormFree(forms.Form):
    method = forms.CharField(required=True)
    params = forms.CharField(required=False)

    def clean_params(self):
         jdata = self.cleaned_data['params']
         try:
             json_data = json.loads(jdata) #loads string as json
             #validate json_data
         except:
             raise forms.ValidationError("Invalid data in params")
         #if json data not valid:
            #raise forms.ValidationError("Invalid data in jsonfield")
         return jdata

    
    
@login_required
def rpc_submit_free(request,user,slug):

    if request.method == 'POST': # If the form has been submitted...

        form = RpcSubmitFormFree(request.POST, request.FILES) # A form bound to the POST data
        if (form.is_valid() and (request.user.get_username()==user)):
            # All validation rules pass
            try:
                mystationmetadata = StationMetadata.objects.get(user__username=user,slug=slug)
                method=form.cleaned_data['method']
                params=form.cleaned_data['params']
                myrpc=Rpc(stationmetadata=mystationmetadata,method=method,params=params)
                myrpc.save()
                
            except:
                form = RpcSubmitFormFree()
                return render(request, 'rpc/rpc_submit.html',{'form': form,'user':user,'slug':slug,"error":True})
            
            
            return render(request, 'rpc/rpc_submit.html',{'form': form,'user':user,'slug':slug,"success":True})
            
        else:
            form = RpcSubmitFormFree()
            return render(request, 'rpc/rpc_submit.html',{'form': form,'user':user,'slug':slug,"invalid":True})
        
    form = RpcSubmitFormFree()
    return render(request, 'rpc/rpc_submit.html',{'form': form,'user':user,'slug':slug})


RPC_CHOICES = (
    ("reboot"                    , "reboot"),
    ("reboot_fupdate"            , "reboot and firmware update"),
    ("recovery_lastday"          , "recovery last day of data"),
    ("admin_fdownload"           , "admin firmware download"),
    ("admin_cdownload"           , "admin configuration download"),
)

class RpcSubmitFormChoice(forms.Form):
    rpc = forms.ChoiceField(choices=RPC_CHOICES,required=True)
    
    
@login_required
def rpc_submit_choice(request,user,slug):


    datetimestart=datetime.now()-timedelta(days=1)
    dts=[datetimestart.year,datetimestart.month,datetimestart.day,
         datetimestart.hour,datetimestart.minute,datetimestart.second]

    RPC_CHOICES_SPLIT = {
        "reboot":{"method":"reboot","params":""},
        "reboot_fupdate":{"method":"reboot","params":'{"fupdate":true}'},
        "recovery_lastday":{"method":"recovery","params":'{"dts":'+json.dumps(dts)+'}'},
        "admin_fdownload":{"method":"admin","params":'{"fdownload":true}'},
        "admin_cdownload":{"method":"admin","params":'{"cdownload":true}'},
    }

    if request.method == 'POST': # If the form has been submitted...

        form = RpcSubmitFormChoice(request.POST) # A form bound to the POST data
        if (form.is_valid() and (request.user.get_username()==user)):
            # All validation rules pass
            try:
                mystationmetadata = StationMetadata.objects.get(user__username=user,slug=slug)

                method=RPC_CHOICES_SPLIT[form.cleaned_data['rpc']]["method"]
                params=json.loads(RPC_CHOICES_SPLIT[form.cleaned_data['rpc']]["params"])

                myrpc=Rpc(stationmetadata=mystationmetadata,method=method,params=params)
                myrpc.save()
                
            except:
                form = RpcSubmitFormChoice()
                raise
                #return render(request, 'rpc/rpc_submit.html',{'form': form,'user':user,'slug':slug,"error":True})
            
            return render(request, 'rpc/rpc_submit.html',{'form': form,'user':user,'slug':slug,"success":True})
            
        else:
            form = RpcSubmitFormChoice()
            return render(request, 'rpc/rpc_submit.html',{'form': form,'user':user,'slug':slug,"invalid":True})
        
    form = RpcSubmitFormChoice()
    return render(request, 'rpc/rpc_submit.html',{'form': form,'user':user,'slug':slug})
