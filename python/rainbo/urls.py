from django.conf import settings
from django.conf.urls import url,include
import rainbo.views
import insertdata.views
import showdata.views
from rmap.stations.form import RmapRegistrationForm
from registration.backends.default.views import RegistrationView


from rmap.stations.form import RmapRegistrationForm
#from registration.forms import RegistrationFormTermsOfService
from registration.backends.default.views import RegistrationView

# Uncomment the next two lines to enable the admin:
from django.contrib import admin
admin.autodiscover()

#from rainbo_insertdata import views as r_insert
##from rainbo_showdata import views as r_show

app_name = 'rainbo'

basepattern = (
    r'^showdata/(?P<ident>\w+|\*|-)'
    r'/(?P<coords>(?P<lon>\-?\+?\d+|-),(?P<lat>\-?\+?\d+|-)|\*)'
    r'/(?P<network>[-\w]+|\*)'
    r'/(?P<trange>(?P<tr>\d+|-|\*),(?P<p1>\d+|-|\*),(?P<p2>\d+|-|\*)|\*)'
    r'/(?P<level>(?P<lt1>\d+|-|\*),(?P<lv1>\d+|-|\*),(?P<lt2>\d+|-|\*),(?P<lv2>\d+|-|\*)|\*)'
    r'/(?P<var>B\d{5}|\*)'
)


urlpatterns = [
    url( r'^$',  rainbo.views.home ,name='home' ),
    url(r'^insertdata/impactdata$',
        insertdata.views.insertDataRainboImpactData,name="insertdata-impactdata"),
    url(r'^insertdata/manualdata$',
        insertdata.views.insertDataRainboWeatherData,name="insertdata-manualdata"),

    url(r'^registrazione/register/$', RegistrationView.as_view(form_class= RmapRegistrationForm),name='registration_register'),
    url(r'^registrazione/', include('registration.backends.default.urls')),
    url(r'^accounts/profile/$',
        insertdata.views.insertDataRainboWeatherData,name="insertdata-manualdata"),

#   Uncomment the next line to enable the admin and reset password
    url(r'^admin/', include(admin.site.urls)),

    url(basepattern + r'/timeseries/(?P<year>\d{4})$', showdata.views.rainbotimeseries,name="timeseriesyearly"),
    url(basepattern + r'/timeseries/(?P<year>\d{4})/(?P<month>\d{2})$', showdata.views.rainbotimeseries,name="timeseriesmonthly"),
    url(basepattern + r'/timeseries/(?P<year>\d{4})/(?P<month>\d{2})/(?P<day>\d{2})$', showdata.views.rainbotimeseries,name="timeseriesdaily"),
    url(basepattern + r'/timeseries/(?P<year>\d{4})/(?P<month>\d{2})/(?P<day>\d{2})/(?P<hour>\d{2})$', showdata.views.rainbotimeseries,name="timeserieshourly"),

    url(basepattern + r'/spatialseries/(?P<year>\d{4})/(?P<month>\d{2})/(?P<day>\d{2})/(?P<hour>\d{2})$', showdata.views.rainbospatialseries,name="spatialserieshourly"),
    url(basepattern + r'/spatialseries/(?P<year>\d{4})/(?P<month>\d{2})/(?P<day>\d{2})$', showdata.views.rainbospatialseries,name="spatialseriesdaily"),

    url(r'^borinud/', include('borinud.urls')),

    url( r'^contacts/', include( 'contacts.urls' ) ),
]

