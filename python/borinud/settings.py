from django.conf import settings

DEFAULTS = {
    # List of databases (e.g. ["sqlite:/path/to/db.sqlite3"]).
    "SOURCES": [],
    # Name of the cache used for the summary (None if disabled)
    "CACHED_SUMMARY": None,
    # Cache timeout
    "CACHED_SUMMARY_TIMEOUT": 0,
}

BORINUD = getattr(settings, 'BORINUD', {})

for name, default in DEFAULTS.items():
    if name not in BORINUD:
        BORINUD[name] = default
