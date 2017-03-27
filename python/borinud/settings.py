"""
Settings for Borinud.

Example::
    # project/settings.py
    BORINUD["report"]["SOURCES"] = [{
        "class": "borinud.utils.source.DballeDB",
        "url": "odbc://rmap",
    }, {
        "class": "borinud.utils.source.ArkimetBufrDB",
        "dataset": "http://localhost:8090/dataset/rmap",
        "measurements": [{
            "var": "B13011",
            "level": (1, None, None, None),
            "trange": (0, 0, 3600),
        }, {
            "var": "B12101",
            "level": (103, 2000, None, None),
            "trange": (254, 0, 0),
        }],
    }]
    BORINUD["report"]["CACHED_SUMMARY"] = "default"
    BORINUD["report"]["CACHED_SUMMARY_TIMEOUT"] = 3600
"""
from django.conf import settings

DEFAULTS = {
    "SOURCES": [],
    "CACHED_SUMMARY": None,
    "CACHED_SUMMARY_TIMEOUT": 0,
}

BORINUD = getattr(settings, 'BORINUD', {})

for name, default in DEFAULTS.items():
    for dsn in BORINUD:
        if name not in BORINUD[dsn]:
            BORINUD[dsn][name] = default
