from django.conf.urls import url

from . import views


urlpatterns = [
    url(r'^http2mqtt/$', views.publish, name='publish'),
]
