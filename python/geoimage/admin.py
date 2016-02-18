from leaflet.admin import LeafletGeoAdmin
from django.contrib import admin
from models import GeorefencedImage

admin.site.register(GeorefencedImage, LeafletGeoAdmin)
