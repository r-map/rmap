from django.conf.urls import urls

from . import view


urlpatterns = [
        url(r'?publish/$', views.publish, name='publish'),
]
