from django.conf.urls import url
from rmap.stations.views import StationList
from rmap.stations.views import StationDetail
from rmap.stations.views import StationsOnMap
from rmap.stations.views import mystationmetadata_list
from rmap.stations.views import mystationmetadata_detail

urlpatterns = [

#                       url(r'^stations/$',StationList.as_view()
#                           ,name='stationmetadata-list' ),

#                       url(r'^stations/(?P<slug>[-_\w]+)/$',
#                           StationDetail.as_view(),name='stationmetadata-detail'),


                       url(r'^stations/$',StationList.as_view()
                           ,name='stationmetadata-list' ),

                       url(r'^stations/(?P<ident>[-_\w]+)/$',
                           mystationmetadata_list),

                       url(r'^stations/(?P<ident>[-_\w]+)/(?P<slug>[-_\w]+)/$',
                           mystationmetadata_detail),

                       url(r'^stationsonmap/$', StationsOnMap
                           ,name='stationsonmap' ),

                       url(r'^stationsonmap/(?P<ident>[-_\w]+)/$',StationsOnMap
                           ,name='stationsonmap' ),

                       url(r'^stationsonmap/(?P<ident>[-_\w]+)/(?P<slug>[-_\w]+)/$',StationsOnMap
                           ,name='stationsonmap' ),

]
