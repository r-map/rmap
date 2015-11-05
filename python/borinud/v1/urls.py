from django.conf.urls import url

from . import views


urlpatterns = [
    url(r'^\*/\*/\*/\*/\*/\*/summaries$', views.summaries),
    url(r'^\*/\*/(?P<network>\w+)/\*/\*/\*/summaries$', views.summaries),
]
