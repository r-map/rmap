from django.conf import settings
from django_hosts import patterns, host


def set_base_template_rmap(request):
    request.base_template = "base.html"

def set_base_template_rainbo(request):
    request.base_template = "rainbo/base_service/base_service_content.html"


host_patterns = patterns('',
    host(r'rmap', settings.ROOT_URLCONF, callback='rmap.hosts.set_base_template_rmap', name='rmap'),  # <-- The `name` we used to in the `DEFAULT_HOST` setting
    host(r'test', settings.ROOT_URLCONF, callback='rmap.hosts.set_base_template_rmap', name='test'),  # <-- The `name` we used to in the `DEFAULT_HOST` setting
    host(r'partecipa', 'rainbo.urls', callback='rmap.hosts.set_base_template_rainbo', name='rainbo'),  
)
