from django.conf.urls import url
from rmap.stations.views import StationList
from rmap.stations.views import StationDetail
from rmap.stations.views import StationsOnMap
from rmap.stations.views import mystationmetadata_list
from rmap.stations.views import mystationmetadata_detail
from rmap.stations.views import mystationmetadata_del

urlpatterns = [

#                       url(r'^stations/$',StationList.as_view()
#                           ,name='stationmetadata-list' ),

#                       url(r'^stations/(?P<slug>[-_\w]+)/$',
#                           StationDetail.as_view(),name='stationmetadata-detail'),


                       url(r'^stations/$',StationList.as_view()
                           ,name='stationmetadata-list' ),

                       url(r'^stations/(?P<ident>[-_\w]+)/$',
                           mystationmetadata_list ,name='mystationmetadata_list'),

                       url(r'^stations/(?P<ident>[-_\w]+)/(?P<slug>[-_\w]+)/$',
                           mystationmetadata_detail ,name='mystationmetadata-detail'),

                       url(r'^delstation/(?P<ident>[-_\w]+)/(?P<slug>[-_\w]+)/$',
                           mystationmetadata_del, name='mystationmetadata-del'),

                       url(r'^stationsonmap/$', StationsOnMap
                           ,name='stationsonmap' ),

                       url(r'^stationsonmap/(?P<ident>[-_\w]+)/$',StationsOnMap
                           ,name='stationsonmap' ),

                       url(r'^stationsonmap/(?P<ident>[-_\w]+)/(?P<slug>[-_\w]+)/$',StationsOnMap
                           ,name='stationsonmap' ),

]
