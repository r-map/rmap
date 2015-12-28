from django.conf import settings

DEFAULTS = {
    "SOURCES": [],
    "CACHED_SUMMARY": None,
    "CACHED_SUMMARY_TIMEOUT": 0,
}

BORINUD = getattr(settings, 'BORINUD', {})

for name, default in DEFAULTS.items():
    if name not in BORINUD:
        BORINUD[name] = default
