from django.conf.urls import url
from .views import rpc_list, rpc_details

urlpatterns = [
    
    url(r'^rpcs/$',
        rpc_list ,name='rpcs-list' ),
    
    url(r'^rpcs/(?P<user>[-_\w]+)/$',
        rpc_list ,name='rpcs-list'),
    
    url(r'^rpcs/(?P<user>[-_\w]+)/(?P<slug>[-_\w]+)/$',
        rpc_list ,name='rpcs-list'),

    url(r'^rpc/(?P<user>[-_\w]+)/(?P<slug>[-_\w]+)/(?P<id>[-_\w]+)/$',
        rpc_details ,name='rpc-details'),

]
    