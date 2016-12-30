"""Copyright 2008 Orbitz WorldWide

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License"""
from django.conf import settings
from django.conf.urls import include, url
from django.contrib import admin
from url_shortener.views import shorten, follow
from browser.views import browser

from .render import urls as render_urls
from .composer import urls as composer_urls
from .metrics import urls as metrics_urls
from .browser import urls as browser_urls
from .account import urls as account_urls
from .dashboard import urls as dashboard_urls
from .whitelist import urls as whitelist_urls
from .version import urls as version_urls
from .events import urls as events_urls



graphite_urls = [
    url('^admin/', include(admin.site.urls)),
    url('^render/?', include(render_urls)),
    url('^composer/?', include(composer_urls)),
    url('^metrics/?', include(metrics_urls)),
    url('^browser/?', include(browser_urls)),
    url('^account/', include(account_urls)),
    url('^dashboard/?', include(dashboard_urls)),
    url('^whitelist/?', include(whitelist_urls)),
    url('^version/', include(version_urls)),
    url('^events/', include(events_urls)),
    url('^s/(?P<path>.*)', shorten, name='shorten'),
    url('^S/(?P<link_id>[a-zA-Z0-9]+)/?$', follow, name='follow'),
    url('^$', browser, name='browser'),
]

url_prefix = ''
if settings.URL_PREFIX.strip('/'):
    url_prefix = '{0}/'.format(settings.URL_PREFIX.strip('/'))

urlpatterns = [
    url(r'^{0}'.format(url_prefix), include(graphite_urls)),
]

handler500 = 'views.server_error'
