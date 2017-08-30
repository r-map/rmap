from django.conf import settings
from django_hosts import patterns, host

host_patterns = patterns('',
    host(r'rmapv', settings.ROOT_URLCONF, name='rmapv'),  # <-- The `name` we used to in the `DEFAULT_HOST` setting
    host(r'rainbo', 'rainbo.urls', name='rainbo'),  
)
