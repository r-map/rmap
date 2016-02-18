from django.conf.urls import url
from views import showImage

urlpatterns = [
    url(r'^test$',
        showImage),

]
