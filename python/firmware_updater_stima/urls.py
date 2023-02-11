from django.conf.urls import url

from . import views

urlpatterns = [
    url(r'^v4/$', views.index, name='firmware_index'),
    url(r'^v4/update/(?P<name>[-_\w]+)/$', views.update, name='firmware_update'),
]
