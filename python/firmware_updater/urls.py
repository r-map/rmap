from django.conf.urls import url

from . import views

urlpatterns = [
    url(r'^$', views.index, name='firmware_index'),
    url(r'^update/(?P<name>[-_\w]+)/$', views.update, name='firmware_update'),
]
