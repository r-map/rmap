from django.conf.urls import url
from .views import geoimagesOnMap,geoimageById,geoimagesByCoordinate,geoimageDelete

urlpatterns = [
    url(r'^geoimagesonmap$',
        geoimagesOnMap,name="geoimages-on-map"),

    url(r'^geoimagesonmap/(?P<ident>[-_\w]+)/$',
        geoimagesOnMap,name="geoimages-on-map-by-ident"),
    
    url(r'^geoimagebyid/(?P<id>[-_\w]+)/$',
        geoimageById,name="geoimage-by-id"),

    url(r'^geoimagedelete/(?P<id>[-_\w]+)/$',
        geoimageDelete,name="geoimage-delete"),
    
    url(r'^geoimagesbycoordinate/(?P<lon>[-_\w.]+)/(?P<lat>[-_\w.]+)/$',
        geoimagesByCoordinate,name="geoimages-by-coordinate"),
]
