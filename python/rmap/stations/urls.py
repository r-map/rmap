from django.conf.urls import url
from rmap.stations.views import StationList
from rmap.stations.views import StationDetail
from rmap.stations.views import StationsOnMap
from rmap.stations.views import mystationmetadata_list
from rmap.stations.views import mystationmetadata_detail
from rmap.stations.views import mystationmetadata_json
from rmap.stations.views import mystationmetadata_del
from rmap.stations.views import mystation_localdata

urlpatterns = [

#                       url(r'^stations/$',StationList.as_view()
#                           ,name='stationmetadata-list' ),

#                       url(r'^stations/(?P<slug>[-_\w]+)/$',
#                           StationDetail.as_view(),name='stationmetadata-detail'),


                       url(r'^stations/$',StationList.as_view()
                           ,name='stationmetadata-list' ),

                       url(r'^stations/(?P<user>[-_\w]+)/$',
                           mystationmetadata_list ,name='mystationmetadata_list'),

                       url(r'^stations/(?P<user>[-_\w]+)/(?P<slug>[-_\w]+)/$',
                           mystationmetadata_detail ,name='mystationmetadata-detail'),

                       url(r'^stations/(?P<user>[-_\w]+)/(?P<station_slug>[-_\w]+)/json/$',
                           mystationmetadata_json ,name='mystationmetadata-json'),

                       url(r'^stations/(?P<user>[-_\w]+)/(?P<station_slug>[-_\w]+)/json_dump/$',
                           mystationmetadata_json, kwargs={'dump':True},name='mystationmetadata-json-dump'),
    
                       url(r'^stations/(?P<user>[-_\w]+)/(?P<station_slug>[-_\w]+)/(?P<board_slug>[-_\w]*)/json/$',
                           mystationmetadata_json ,name='mystationmetadata-board-json'),

                       url(r'^delstation/(?P<user>[-_\w]+)/(?P<slug>[-_\w]+)/$',
                           mystationmetadata_del, name='mystationmetadata-del'),

                       url(r'^stationlocaldata/(?P<user>[-_\w]+)/(?P<slug>[-_\w]+)/$',
                           mystation_localdata, name='mystation-localdata'),
    
                       url(r'^stationsonmap/$', StationsOnMap
                           ,name='stationsonmap' ),

                       url(r'^stationsonmap/(?P<user>[-_\w]+)/$',StationsOnMap
                           ,name='stationsonmap' ),

                       url(r'^stationsonmap/(?P<user>[-_\w]+)/(?P<slug>[-_\w]+)/$',StationsOnMap
                           ,name='stationsonmap' ),

]
