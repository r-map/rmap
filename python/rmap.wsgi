import os
import rmap.settings

os.environ.setdefault("DJANGO_SETTINGS_MODULE", "rmap.settings")

from django.core.wsgi import get_wsgi_application
application = get_wsgi_application()
