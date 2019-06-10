from django.conf.urls import url, include

from . import views
from .v1 import urls as v1_urls


api_patterns = [
    url(r'^v1/', include(v1_urls, namespace='v1')),
]

urlpatterns = [
    url(r'^$', views.get_map),
    url(r'^api/', include(api_patterns, namespace='api')),
]
