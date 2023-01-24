from django.contrib import admin
from .models import Firmware
from .models import Name

class FirmwareInline(admin.TabularInline):
    model = Firmware

@admin.register(Name)
class NameAdmin(admin.ModelAdmin):
    fields = ('name','description'),
    list_display = ('name',)
    list_filter = ['name',]
    search_fields = ['name']
    inlines = [FirmwareInline,]


@admin.register(Firmware)
class FirmwareAdmin(admin.ModelAdmin):
    fields = ('firmware','file','version','revision','active'),
    list_display = ('firmware','file','version','revision','active')
    list_filter = ['version','revision','active']
    #date_hierarchy = 'date'
    search_fields = ['firmware']

