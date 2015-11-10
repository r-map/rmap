import os
import json
import tempfile
from datetime import datetime

from django.test import TestCase
from django.test import Client, override_settings

import dballe

from .models import Source, Summary
from .utils import sync_source
from .v1.utils import path2query


class TestUtils(TestCase):
    def test_path2query(self):
        d = {
            "network": "*",
            "tr": "-"
        }
        q = path2query(d)
        self.assertEquals(tuple(q.keys()), ("tr",))
        self.assertEquals(q["tr"], None)


class TestViews(TestCase):
    @override_settings(ROOT_URLCONF='borinud.urls')
    def jsonrequest(self, path):
        c = Client()
        r = c.get("/api/v1/" + path)
        self.assertEquals(r.status_code, 200)
        return json.loads(r.content.decode("utf-8"))

    def setUp(self):
        self.dbfile = tempfile.NamedTemporaryFile(
            mode="w", delete=False, suffix=".sqlite3",
        ).name
        self.db = dballe.DB.connect_from_file(self.dbfile)
        self.db.reset()
        self.summ = [{
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
        }, {
            "ident": None,
            "rep_memo": "test_net_2",
            "lon": 1200000,
            "lat": 4200000,
            "datetime": datetime(2015, 1, 2),
            "B12101": 0,
            "trange": (254, 0, 0),
            "level": (103, 2000),
        }]

        for r in self.summ:
            self.db.insert_data(dballe.Record(**r), can_add_stations=True)

        from borinud.utils import sync_source
        s = Source.objects.create(connection_type=Source.DBALLE,
                                  connection="sqlite:{}".format(self.dbfile))
        sync_source(s)


    def tearDown(self):
        os.remove(self.dbfile)

    def test_summaries(self):
        geojson = self.jsonrequest("*/*/*/*/*/*/summaries")
        self.assertEquals(len(geojson["features"]), 3)

    def test_summaries_by_network(self):
        geojson = self.jsonrequest("*/*/test_net/*/*/*/summaries")
        self.assertEquals(len(geojson["features"]), 2)

    def test_summaries_by_ident(self):
        geojson = self.jsonrequest("test_sta/*/*/*/*/*/summaries")
        self.assertEquals(len(geojson["features"]), 1)

    def test_summaries_by_fixed_station(self):
        geojson = self.jsonrequest("-/1200000,4300000/test_net/*/*/*/summaries")
        self.assertEquals(len(geojson["features"]), 1)
