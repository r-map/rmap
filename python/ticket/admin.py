from django.contrib import admin
from django.utils.translation import ugettext_lazy
import rmap.settings
from .models import (Ticket, TicketAction, TicketImage, TicketAttachment)
from django.utils.translation import ugettext_lazy as _

from django.contrib.auth.admin import UserAdmin
from django.contrib.auth.models import User
from .models import UserProfile


class TicketActionInline(admin.TabularInline):
    model = TicketAction
    extra = 1
    max_num = 3

class TicketImageInline(admin.TabularInline):
    model = TicketImage
    extra = 1
    max_num = 3

class TicketAttachmentInline(admin.TabularInline):
    model = TicketAttachment
    extra = 1
    max_num = 3
    
class TicketAdmin(admin.ModelAdmin):

    fieldsets = [
        (_('Ticket'), {'fields': ['active', 'stationmetadata','date', 'priority','assigned_to','subscribed_by']}),
        (_('issue'), {'classes': ('wide',), 'fields': ['abstract', 'description']}),
            ]
    list_editable = ('active', 'priority')
    list_display = ('ticket', 'date', 'active', 'priority', 'abstract', 'stationmetadata')
    list_filter = ['date', 'active', 'priority', 'stationmetadata','assigned_to','subscribed_by']
    search_fields = ['abstract']
    date_hierarchy = 'date'
    inlines = [TicketActionInline, TicketImageInline, TicketAttachmentInline ]
    #inlines = [TicketActionInline, TicketImageInline]
    list_display_links = ('ticket', 'date')

admin.site.register(Ticket, TicketAdmin)

class TicketActionAdmin(admin.ModelAdmin):
    list_display = ('ticket', 'date', 'description')

admin.site.register(TicketAction, TicketActionAdmin)

class TicketImageAdmin(admin.ModelAdmin):
    list_display = ('ticket', 'date', 'description')

admin.site.register(TicketImage, TicketImageAdmin)

class TicketAttachmentAdmin(admin.ModelAdmin):
    list_display = ('ticket', 'date', 'file')

admin.site.register(TicketAttachment, TicketAttachmentAdmin)
