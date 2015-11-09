import os
import tempfile
from datetime import datetime

from django.test import TestCase
from django.test import Client

import dballe

from .models import Source, Summary
from .utils import sync_source


class TestUtils(TestCase):
    def setUp(self):
        self.dbfile = tempfile.NamedTemporaryFile(
            mode="w", delete=False, suffix=".sqlite3",
        ).name
        self.db = dballe.DB.connect_from_file(self.dbfile)
        self.db.reset()
        for r in [{
            "ident": None,
            "rep_memo": "test_net",
            "lon": 1200000,
            "lat": 4300000,
            "datetime": datetime(2015, 1, 1),
            "B12101": 0,
            "trange": (254, 0, 0),
            "level": (103, 2000),
        }, {
            "ident": None,
            "rep_memo": "test_net",
            "lon": 1200000,
            "lat": 4300000,
            "datetime": datetime(2015, 1, 2),
            "B12101": 0,
            "trange": (254, 0, 0),
            "level": (103, 2000),
        }, {
            "ident": "test_sta",
            "rep_memo": "test_net",
            "lon": 1200000,
            "lat": 4300000,
            "datetime": datetime(2015, 1, 1),
            "B12101": 0,
            "trange": (254, 0, 0),
            "level": (103, 2000),
        }, {
            "ident": "test_sta",
            "rep_memo": "test_net",
            "lon": 1300000,
            "lat": 4200000,
            "datetime": datetime(2015, 1, 2),
            "B12101": 0,
            "trange": (254, 0, 0),
            "level": (103, 2000),
        }]:
            self.db.insert_data(dballe.Record(**r), can_add_stations=True)

        from borinud.utils import sync_source
        s = Source.objects.create(connection_type=Source.DBALLE,
                                  connection="sqlite:{}".format(self.dbfile))
        sync_source(s)


    def tearDown(self):
        os.remove(self.dbfile)

    def test_summaries(self):
        import json
        c = Client()
        response = c.get("/borinud/api/v1/*/*/*/*/*/*/summaries")
        geojson = json.loads(response.content.decode("utf-8"))
        self.assertEquals(len(geojson["features"]), 2)
