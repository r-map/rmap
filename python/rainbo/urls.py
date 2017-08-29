from django.conf import settings
from django.conf.urls import url
from . import views
#from rainbo_insertdata import views as r_insert
##from rainbo_showdata import views as r_show


urlpatterns = [
    url( r'^$',  views.home ,name='home' )
]

