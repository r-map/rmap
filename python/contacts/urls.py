from django.conf.urls import url

from . import views

urlpatterns = [
    url( r'contact.json$', views.contacts, name="contact" )
]
