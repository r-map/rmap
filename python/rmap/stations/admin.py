from django.contrib import admin
from models import StationMetadata,Board,Sensor,SensorType,Bcode,TransportSerial,TransportTcpip,TransportRF24Network,TransportMqtt,TransportAmqp,TransportBluetooth,StationConstantData
from django import forms
from django.utils.translation import ugettext_lazy
import rmap.settings


class TransportSerialInline(admin.TabularInline):
    model = TransportSerial

class TransportBluetoothInline(admin.TabularInline):
    model = TransportBluetooth

class TransportTcpipInline(admin.TabularInline):
    model = TransportTcpip

class TransportRF24NetworkInline(admin.TabularInline):
    model = TransportRF24Network

class TransportMqttInline(admin.TabularInline):
    model = TransportMqtt

class TransportAmqpInline(admin.TabularInline):
    model = TransportAmqp

class SensorInline(admin.TabularInline):
    model = Sensor
    extra=0   # this require javascript to add record inline

class BoardInline(admin.StackedInline):
    model = Board
    extra=0   # this require javascript to add record inline

## Board has Certificates inline but rather
## than nesting inlines (not possible), shows a link to
## its own ModelAdmin's change form, for accessing TrainingDates:
#
#class CertificateLinkInline(admin.TabularInline):
#    model = Board
#    # Whichever fields you want: (I usually use only a couple
#    # needed to identify the entry)
#    fields = ('name', 'slug',)
#    readonly_fields = ('changeform_link', )


class SensorAdmin(admin.ModelAdmin):

    list_display = ('name','active','driver','type','address','timerange','level','board')
    list_editable = ('active','timerange','level')
    search_fields = ['name','driver','type','address','timerange','level','board__name']

    list_filter = ('driver','timerange','level','board')

admin.site.register(Sensor, SensorAdmin)


class SensorTypeAdmin(admin.ModelAdmin):

    list_display = ('name','active','datalevel','type')
    list_editable = ('active','datalevel')
    search_fields = ['name','type','bcodes']

    list_filter = ('datalevel','bcodes',)

admin.site.register(SensorType, SensorTypeAdmin)


class BcodeAdmin(admin.ModelAdmin):

    list_display = ('bcode','description','unit','scale','offset','userunit')
    search_fields = ['bcode','description','unit']
    list_editable =('scale','offset','userunit')
    list_filter = ('unit',)

admin.site.register(Bcode, BcodeAdmin)


class BoardAdmin(admin.ModelAdmin):
    prepopulated_fields = {'slug': ("name",)}

#    inlines = [CertificateLinkInline,]

    inlines = [
        TransportSerialInline,
        TransportBluetoothInline,
        TransportTcpipInline,
        TransportRF24NetworkInline,
        TransportAmqpInline,
        TransportMqttInline,
        SensorInline,
    ]

    list_display = ('name','active','slug','category','stationmetadata')
    list_editable = ('active',)
    search_fields = ('name','active','slug','category','stationmetadata__name')

    list_filter = ('active','category','stationmetadata','category')



admin.site.register(Board, BoardAdmin)


#class StationConstantDataAdmin(admin.ModelAdmin):
#    pass

#admin.site.register(StationConstantData, StationConstantDataAdmin)

class StationConstantDataInline(admin.TabularInline):
    model = StationConstantData
    extra=1


class StationMetadataAdmin(admin.ModelAdmin):

    inlines = [BoardInline,StationConstantDataInline]

    prepopulated_fields = {'slug': ("name",)}

#    fieldsets = (
#        (None, {'fields': ('name','slug','active','category')}),
#
#        ('Station position', {
#                'classes': ('collapse',),
#                'fields': ('lat','lon')}),
#        )


    list_display = ('name','active','slug','ident','lat','lon','network','mqttrootpath','mqttmaintpath','category')
    list_display_links = ('name', 'slug')
    list_editable = ('active','ident','lat','lon')
    search_fields = ['name','slug','ident__username','network']

    list_filter = ('ident','network','mqttrootpath','mqttmaintpath','category')


admin.site.register(StationMetadata, StationMetadataAdmin)

from django.contrib.auth.admin import UserAdmin
from django.contrib.auth.models import User
from models import UserProfile

# Define an inline admin descriptor for UserProfile model
# which acts a bit like a singleton
class UserProfileInline(admin.StackedInline):
    model = UserProfile
    can_delete = False
    verbose_name_plural = 'profile'

# Define a new User admin
class UserAdmin(UserAdmin):
    inlines = (UserProfileInline, )

# Re-register UserAdmin
admin.site.unregister(User)
admin.site.register(User, UserAdmin)
