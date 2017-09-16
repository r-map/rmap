from django.conf import settings
from django_hosts import patterns, host


def set_base_template_rmapv(request):
    request.base_template = "base.html"

def set_base_template_rainbo(request):
    request.base_template = "rainbo/base_service/base_service_content.html"


host_patterns = patterns('',
    host(r'rmapv', settings.ROOT_URLCONF, callback='rmap.hosts.set_base_template_rmapv', name='rmapv'),  # <-- The `name` we used to in the `DEFAULT_HOST` setting
    host(r'partecipa', 'rainbo.urls', callback='rmap.hosts.set_base_template_rainbo', name='rainbo'),  
)
