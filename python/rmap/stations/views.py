
from django.views.generic import ListView
from django.views.generic.detail import DetailView
from models import StationMetadata

class StationList(ListView):
    model = StationMetadata

class StationDetail(DetailView):
    model = StationMetadata
