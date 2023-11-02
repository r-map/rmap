from django.conf.urls import url
#from rmap.stations.views import StationList
from rmap.stations.views import StationDetail
from rmap.stations.views import StationsOnMap
from rmap.stations.views import mystationmetadata_list
from rmap.stations.views import mystationstatus_list
from rmap.stations.views import mystationmetadata_detail
from rmap.stations.views import mystationstatus_detail
from rmap.stations.views import mystationmetadata_json
from rmap.stations.views import mystationmetadata_upload_json
from rmap.stations.views import mystationmetadata_del
from rmap.stations.views import mystation_localdata
from rmap.stations.views import mystationmetadata_config
from rmap.stations.views import mystationmetadata_configv3

urlpatterns = [

#                       url(r'^stations/$',StationList.as_view()
#                           ,name='stationmetadata-list' ),

#                       url(r'^stations/(?P<slug>[-_\w]+)/$',
#                           StationDetail.as_view(),name='stationmetadata-detail'),


#                       url(r'^stations/$',StationList.as_view()
#                           ,name='stationmetadata-list' ),

                       url(r'^stations/$',
                           mystationmetadata_list ,name='stationmetadata-list' ),

                       url(r'^stations/(?P<user>[-_\w]+)/$',
                           mystationmetadata_list ,name='mystationmetadata-list'),

                       url(r'^stations/(?P<user>[-_\w]+)/(?P<slug>[-_\w]+)/$',
                           mystationmetadata_detail ,name='mystationmetadata-detail'),


                       url(r'^stationstatus/$',
                           mystationstatus_list ,name='stationstatus-list' ),

                       url(r'^stationstatus/(?P<user>[-_\w]+)/$',
                           mystationstatus_list ,name='mystationstatus-list'),

                       url(r'^stationstatus/(?P<user>[-_\w]+)/(?P<slug>[-_\w]+)/$',
                           mystationstatus_detail ,name='mystationstatus-detail'),

    
                       url(r'^stationsupload/json/$',
                           mystationmetadata_upload_json ,name='mystationmetadata-upload-json'),
    
                       url(r'^stationconfig/(?P<user>[-_\w]+)/(?P<station_slug>[-_\w]+)$',
                           mystationmetadata_config ,name='mystationmetadata-config'),

                       url(r'^stations/(?P<user>[-_\w]+)/(?P<station_slug>[-_\w]+)/json/$',
                           mystationmetadata_json ,name='mystationmetadata-json'),

                       url(r'^stations/(?P<user>[-_\w]+)/(?P<station_slug>[-_\w]+)/json_dump/$',
                           mystationmetadata_json, kwargs={'dump':True},name='mystationmetadata-json-dump'),

                       url(r'^stations/(?P<user>[-_\w]+)/(?P<station_slug>[-_\w]+)/configv3/$',
                           mystationmetadata_configv3, name='mystationmetadata-configv3'),
    
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
