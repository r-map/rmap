from django.conf.urls import patterns, url

import views


urlpatterns = patterns(
    '',
    url(r'^http2mqtt/$', views.publish, name='publish'),
)
