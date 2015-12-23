from django.conf.urls import url

import views


urlpatterns = [
    url(r'^http2mqtt/$', views.publish, name='publish'),
]
