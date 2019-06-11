from django.contrib import admin
from .models import NetworkMetadata

class NetworkAdmin(admin.ModelAdmin):

    list_display = ('name','provider','licenze')
    list_editable = ('provider','licenze')
    search_fields = ['name','licenze','description']

    list_filter = ('name','licenze','provider')

admin.site.register(NetworkMetadata, NetworkAdmin)

