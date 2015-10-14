from django.conf.urls import patterns, urls

from . import view


urlpatterns = patterns(
    '',
    url(r'^publish/$', views.publish, name='publish'),
)
