from django.contrib import admin
from django.utils.translation import ugettext_lazy
import rmap.settings
from .models import Rpc
from django.utils.translation import ugettext_lazy as _

from django.contrib.auth.admin import UserAdmin
from django.contrib.auth.models import User
from .models import UserProfile

class RpcAdmin(admin.ModelAdmin):

    fieldsets = [
        (_('Rpc'), {'fields': ['active', 'stationmetadata','date','method']}),
        (_('Command'), {'classes': ('wide',), 'fields': ['datecmd','params',]}),
        (_('Response'), {'classes': ('wide',), 'fields': ['dateres','result','error',]}),
            ]
    list_editable = ('active',)
    list_display = ('id', 'date','datecmd','dateres', 'active', 'method', 'params')
    list_filter = ['date', 'active', 'method', 'stationmetadata']
    search_fields = ['method','stationmetadata']
    date_hierarchy = 'date'
    list_display_links = ('id', 'date')

admin.site.register(Rpc, RpcAdmin)

