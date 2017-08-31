from django.conf import settings
from django.conf.urls import url
import rainbo.views
import insertdata.views
import showdata.views

#from rainbo_insertdata import views as r_insert
##from rainbo_showdata import views as r_show

app_name = 'rainbo'


urlpatterns = [
    url( r'^$',  rainbo.views.home ,name='home' ),
    url(r'^insertdata/impactdata$',
        insertdata.views.insertDataRainboImpactData,name="insertdata-impactdata"),
    url(r'^insertdata/manualdata$',
        insertdata.views.insertDataRainboWeatherData,name="insertdata-manualdata"),


    url("showdata/"+showdata.views.basepattern + r'/timeseries/(?P<year>\d{4})$', views.rainbotimeseries,name="timeseriesyearly"),
    url("showdata/"+showdata.views.basepattern + r'/timeseries/(?P<year>\d{4})/(?P<month>\d{2})$', views.rainbotimeseries,name="timeseriesmonthly"),
    url("showdata/"+showdata.views.basepattern + r'/timeseries/(?P<year>\d{4})/(?P<month>\d{2})/(?P<day>\d{2})$', views.rainbotimeseries,name="timeseriesdaily"),
    url("showdata/"+showdata.views.basepattern + r'/timeseries/(?P<year>\d{4})/(?P<month>\d{2})/(?P<day>\d{2})/(?P<hour>\d{2})$', views.rainbotimeseries,name="timeserieshourly"),

    url("showdata/"+showdata.views.basepattern + r'/spatialseries/(?P<year>\d{4})/(?P<month>\d{2})/(?P<day>\d{2})/(?P<hour>\d{2})$', views.rainbospatialseries,name="spatialserieshourly"),
    url("showdata/"+showdata.views.basepattern + r'/spatialseries/(?P<year>\d{4})/(?P<month>\d{2})/(?P<day>\d{2})$', views.rainbospatialseries,name="spatialseriesdaily"),
]

