from django.conf.urls import url, include

from . import views
from . import api_patterns

urlpatterns = [
    url(r'^$', views.get_map),
    url(r'^api/', include(api_patterns, namespace='api')),
]
