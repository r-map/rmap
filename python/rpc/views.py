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
        search_status = request.GET['status']

        if (search_status == "active"):
            query_status =  (Q(active=True))
        elif (search_status == "completed"):
            query_status =  (Q(active=False))
        else:
            query_status = Q()
            
    else:
        query_status = Q()

    query = query & query_user & query_slug & query_status
                         
    myrpc = Rpc.objects.filter(query).distinct()

    paginator = Paginator(myrpc, 25) # Show 25 contacts per page.
    page_number = request.GET.get('page')
    page_obj = paginator.get_page(page_number)
    return render(request, 'rpc/rpc_list.html',{"page_obj":page_obj,"user":user,"slug":slug,"search":search})
    

def rpc_details(request,user=None,slug=None,id=None):

    myrpc = Rpc.objects.get(stationmetadata__user__username=user,stationmetadata__slug=slug,id=id)
    return render(request, 'rpc/rpc_details.html',{"myrpc":myrpc})

