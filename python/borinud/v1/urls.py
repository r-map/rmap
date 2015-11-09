from django.conf.urls import url

from . import views


urlpatterns = [
    url((
        r'^(?P<ident>\w+|\*)'
        r'/(?P<coords>(?P<lon>\d+),(?P<lat>\d+)|\*)'
        r'/(?P<network>\w+|\*)'
        r'/(?P<trange>(?P<tr>\d+|-),(?P<p1>\d+|-),(?P<p2>\d+|-)|\*)'
        r'/(?P<level>(?P<lt1>\d+|-),(?P<lv1>\d+|-),(?P<lt2>\d+|-),(?P<lv2>\d+|-)|\*)'
        r'/(?P<var>B\d{5}|\*)'
        r'/summaries'
    ), views.summaries),
]
