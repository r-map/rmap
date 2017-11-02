from django.shortcuts import render
#from django.http import HttpResponse
from django.views.generic import ListView
#from django.views.generic.detail import DetailView
from models import NetworkMetadata
from django.shortcuts import render

#def index(request):
#    return HttpResponse("Work in progress.")

class NetworkList(ListView):
    model = NetworkMetadata

#class NetworkDetail(DetailView):
#    model = NetworkMetadata


def detail(request,name):
    mynetwork=NetworkMetadata.objects.get(name=name)
    return render(request, 'network/networkmetadata_detail.html',{"object":mynetwork})

    
