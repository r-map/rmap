from django.conf.urls import url

from . import views


# TODO: each item must be configured to allow fixed, all or miss

basepattern = (
    r'^(?P<ident>\w+|\*|-)'
    r'/(?P<coords>(?P<lon>\d+|-),(?P<lat>\d+|-)|\*)'
    r'/(?P<network>[-\w]+|\*)'
    r'/(?P<trange>(?P<tr>\d+|-|\*),(?P<p1>\d+|-|\*),(?P<p2>\d+|-|\*)|\*)'
    r'/(?P<level>(?P<lt1>\d+|-|\*),(?P<lv1>\d+|-|\*),(?P<lt2>\d+|-|\*),(?P<lv2>\d+|-|\*)|\*)'
    r'/(?P<var>B\d{5}|\*)'
)

urlpatterns = [
    url(basepattern + r'/summaries$', views.summaries),
    url(basepattern + r'/summaries/(?P<year>\d{4})/(?P<month>\d{2})$', views.summaries),
    url(basepattern + r'/summaries/(?P<year>\d{4})/(?P<month>\d{2})/(?P<day>\d{2})$', views.summaries),
    url(basepattern + r'/timeseries/(?P<year>\d{4})$', views.timeseries),
    url(basepattern + r'/timeseries/(?P<year>\d{4})/(?P<month>\d{2})$', views.timeseries),
    url(basepattern + r'/timeseries/(?P<year>\d{4})/(?P<month>\d{2})/(?P<day>\d{2})$', views.timeseries),
    url(basepattern + r'/spatialseries/(?P<year>\d{4})/(?P<month>\d{2})/(?P<day>\d{2})/(?P<hour>\d{2})$', views.spatialseries),
    url(basepattern + r'/stationdata', views.stationdata),
]
