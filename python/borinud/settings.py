from django.conf import settings

DEFAULTS = {
    "SOURCES": [],
}

BORINUD = getattr(settings, 'BORINUD', {})

for name, default in DEFAULTS.items():
    if name not in BORINUD:
        BORINUD[name] = default
