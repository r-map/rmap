from django.conf.urls import patterns, url
from rmap.stations.views import StationList
from rmap.stations.views import StationDetail

urlpatterns = patterns('',
                       url(r'^stations/$',StationList.as_view() ,name='stationmetadata-list' ),
                       url(r'^stations/(?P<slug>[-_\w]+)/$', StationDetail.as_view(), name='stationmetadata-detail'),
)
