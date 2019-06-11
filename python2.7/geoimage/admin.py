from leaflet.admin import LeafletGeoAdmin
from django.contrib import admin
from models import GeorefencedImage

#from imagekit.admin import AdminThumbnail
#
#class ImageAdmin(admin.ModelAdmin):
#    list_display = ('__str__', 'image_thumbnail')
#    admin_thumbnail = ImageThumbnail(image_field='image_thumbnail')
#
#admin.site.register(GeorefencedImage, ImageAdmin)

admin.site.register(GeorefencedImage, LeafletGeoAdmin)
