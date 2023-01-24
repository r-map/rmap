"""
Settings for Borinud.

Example::
    # project/settings.py
BORINUD =\
{
    "report":{
        "SOURCES": 
        [
            {
                "class": "borinud.utils.source.DballeDB",
                "url": "sqlite:/rmap/dballe/report_fixed.sqlite",
                "explorer": "/rmap/dballe/report.json"
            }, 
 
            {
                "class": "borinud.utils.source.ArkimetBufrDB",
                "dataset": "/rmap/arkimet/report.conf",
                "explorer": "/rmap/arkimet/report.json"
            }
        ]
    }
}

"""
from django.conf import settings

#DEFAULTS = {
#    "SOURCES": [],
#}

BORINUD = getattr(settings, 'BORINUD', {})
BORINUDLAST = getattr(settings, 'BORINUDLAST', {})

#for name, default in list(DEFAULTS.items()):
#    for dsn in BORINUD:
#        if name not in BORINUD[dsn]:
#            BORINUD[dsn][name] = default
