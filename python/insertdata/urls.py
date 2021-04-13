from django.conf.urls import url
from . import views

urlpatterns = [
    url(r'^image$',
        views.insertDataImage,name="insertdata-image"),
    url(r'^manualdata$',
        views.insertDataManualData,name="insertdata-manualdata"),
    url(r'^newstation$',
        views.insertNewStation,name="insertdata-newstation"),
    url(r'^newstationdetail$',
        views.insertNewStationDetail,name="insertdata-newstationdetail"),
    url(r'^stationmodify/(?P<slug>[-_\w]+)/$',
        views.stationModify,name="insertdata-stationmodify"),
]
