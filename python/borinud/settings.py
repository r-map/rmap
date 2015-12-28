from django.conf import settings

DEFAULTS = {
    "SOURCES": [],
    "CACHED_SUMMARY": None,
}

BORINUD = getattr(settings, 'BORINUD', {})

for name, default in DEFAULTS.items():
    if name not in BORINUD:
        BORINUD[name] = default
