from django.conf.urls import url
from .views import functionList, functionDetails

urlpatterns = [
  url(r'^(.+)$', functionDetails, name='functionDetails'),
  url(r'^$', functionList, name='functionList'),
]
