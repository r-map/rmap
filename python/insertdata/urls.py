from django.conf.urls import url
import views

urlpatterns = [
    url(r'^image$',
        views.insertDataImage,name="insertdata-image"),
    url(r'^manualdata$',
        views.insertDataManualData,name="insertdata-manualdata"),
    url(r'^newstation$',
        views.insertNewStation,name="insertdata-newstation"),

]
