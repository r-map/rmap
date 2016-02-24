from models import GeorefencedImage
from django.shortcuts import render

def showImage(request):
    grimages=GeorefencedImage.objects.all()
    return render(request, 'geoimage/georefencedimage_list.html',{"grimages":grimages})

