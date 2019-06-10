from django.conf.urls import url

from . import views

app_name = 'showdata'

#urlpatterns = [
#    url(r'showdata$', views.showdata),
#]

# TODO: each item must be configured to allow fixed, all or miss

basepattern = (
    r'^(?P<ident>\w+|\*|-)'
    r'/(?P<coords>(?P<lon>\-?\+?\d+|-),(?P<lat>\-?\+?\d+|-)|\*)'
    r'/(?P<network>[-\w]+|\*)'
    r'/(?P<trange>(?P<tr>\d+|-|\*),(?P<p1>\d+|-|\*),(?P<p2>\d+|-|\*)|\*)'
    r'/(?P<level>(?P<lt1>\d+|-|\*),(?P<lv1>\d+|-|\*),(?P<lt2>\d+|-|\*),(?P<lv2>\d+|-|\*)|\*)'
    r'/(?P<var>B\d{5}|\*)'
)

urlpatterns = [

    url(r'^$', views.menu,name="menu"),
    url(r'^filtro$', views.filtro,name="filtro"),

#    url(basepattern + r'/summaries$', views.summaries),
#    url(basepattern + r'/summaries/(?P<year>\d{4})/(?P<month>\d{2})$', views.summaries),
#    url(basepattern + r'/summaries/(?P<year>\d{4})/(?P<month>\d{2})/(?P<day>\d{2})$', views.summaries),

    url(basepattern + r'/timeseries/(?P<year>\d{4})$', views.timeseries,name="timeseriesyearly"),
    url(basepattern + r'/timeseries/(?P<year>\d{4})/(?P<month>\d{2})$', views.timeseries,name="timeseriesmonthly"),
    url(basepattern + r'/timeseries/(?P<year>\d{4})/(?P<month>\d{2})/(?P<day>\d{2})$', views.timeseries,name="timeseriesdaily"),
    url(basepattern + r'/timeseries/(?P<year>\d{4})/(?P<month>\d{2})/(?P<day>\d{2})/(?P<hour>\d{2})$', views.timeseries,name="timeserieshourly"),

    url(basepattern + r'/spatialseries/(?P<year>\d{4})/(?P<month>\d{2})/(?P<day>\d{2})/(?P<hour>\d{2})$', views.spatialseries,name="spatialserieshourly"),
    url(basepattern + r'/spatialseries/(?P<year>\d{4})/(?P<month>\d{2})/(?P<day>\d{2})$', views.spatialseries,name="spatialseriesdaily"),

    url(basepattern + r'/stations', views.stations,name="stations"),
]
