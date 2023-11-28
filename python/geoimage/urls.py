from django.conf.urls import url
from .views import geoimagesOnMap,geoimageByIdentId,geoimagesByCoordinate

urlpatterns = [
    url(r'^geoimagesonmap$',
        geoimagesOnMap,name="geoimages-on-map"),

    url(r'^geoimagesonmap/(?P<ident>[-_\w]+)/$',
        geoimagesOnMap,name="geoimages-on-map-by-ident"),
    
    url(r'^geoimagebyidentid/(?P<ident>[-_\w]+)/(?P<id>[-_\w]+)/$',
        geoimageByIdentId,name="geoimage-by-ident-id"),
    
    url(r'^geoimagesbycoordinate/(?P<lon>[-_\w.]+)/(?P<lat>[-_\w.]+)/$',
        geoimagesByCoordinate,name="geoimages-by-coordinate"),
]
