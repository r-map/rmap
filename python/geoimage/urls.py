from django.conf.urls import url
from views import showImage,showOneImage

urlpatterns = [
    url(r'^$',
        showImage,name="geoimage"),

    url(r'^(?P<ident>[-_\w]+)/$',
        showImage,name="geoimage-ident"),
    
    url(r'^(?P<ident>[-_\w]+)/(?P<id>[-_\w]+)/$',
        showOneImage,name="geoimage-ident-id"),

]
