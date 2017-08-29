# -*- coding: utf-8 -*-
from __future__ import absolute_import, print_function, unicode_literals

from django.conf import settings
from django.conf.urls import *  # NOQA
from django.conf.urls.static import static
from django.contrib import admin
from django.contrib import sitemaps
from django.contrib.sitemaps.views import sitemap
from . import views
#from rainbo_insertdata import views as r_insert
##from rainbo_showdata import views as r_show

admin.autodiscover()

urlpatterns = [
    url( r'^admin/', admin.site.urls ),  # NOQA
    #url( r'^manualdata', r_insert.insertDataManualData,name="insertdata-manualdata"),
    #url( r'^showdata', r_show.insertDataManualData,name="insertdata-manualdata"),


    #include user management urls
    #url( r'^accounts/', include( 'accounts.urls' ) ),

    url( r'^',  views.home )
]
