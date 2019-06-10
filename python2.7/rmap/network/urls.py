from django.conf.urls import url
from .views import NetworkList
#from .views import NetworkDetail

from . import views

urlpatterns = [

    #    url(r'^$', views.index, name='index'),

    url(r'^$',NetworkList.as_view()
        ,name='networkmetadata-list' ),
    url(r'^(?P<name>[-\w]+)/$', views.detail, name='networkmetadata-detail'),
#    url(r'^(?P<slug>[-\w]+)/$', NetworkDetail.as_view(), name='networkmetadata-detail'),
    

]
