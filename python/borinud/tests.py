import os
import tempfile
from datetime import datetime

from django.test import TestCase
from django.utils.timezone import utc

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

    def tearDown(self):
        os.remove(self.dbfile)

    def test_fixed_sync(self):
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
        }]:
            self.db.insert_data(dballe.Record(**r), can_add_stations=True)

        source = Source.objects.create(
            connection_type=Source.DBALLE,
            connection="sqlite:{}".format(self.dbfile)
        )
        sync_source(source)
        self.assertEquals(Summary.objects.count(), 1)
        self.assertEquals(
            Summary.objects.first().slug,
            "fixed:1200000:4300000:test_net:254:0:0:103:2000:-:-:B12101"
        )
        self.assertEquals(Summary.objects.first().ident, None)
        self.assertEquals(Summary.objects.first().lon, 1200000)
        self.assertEquals(Summary.objects.first().lat, 4300000)
        self.assertEquals(Summary.objects.first().lonmin, 1200000)
        self.assertEquals(Summary.objects.first().lonmax, 1200000)
        self.assertEquals(Summary.objects.first().latmin, 4300000)
        self.assertEquals(Summary.objects.first().latmax, 4300000)
        self.assertEquals(Summary.objects.first().datemin,
                          datetime(2015, 1, 1, tzinfo=utc))
        self.assertEquals(Summary.objects.first().datemax,
                          datetime(2015, 1, 2, tzinfo=utc))

    def test_mobile_sync(self):
        for r in [{
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

        source = Source.objects.create(
            connection_type=Source.DBALLE,
            connection="sqlite:{}".format(self.dbfile)
        )
        sync_source(source)
        self.assertEquals(Summary.objects.count(), 1)
        self.assertEquals(
            Summary.objects.first().slug,
            "mobile:test_sta:test_net:254:0:0:103:2000:-:-:B12101"
        )
        self.assertEquals(Summary.objects.first().ident, "test_sta")
        self.assertEquals(Summary.objects.first().lon, None)
        self.assertEquals(Summary.objects.first().lat, None)
        self.assertEquals(Summary.objects.first().lonmin, 1200000)
        self.assertEquals(Summary.objects.first().lonmax, 1300000)
        self.assertEquals(Summary.objects.first().latmin, 4200000)
        self.assertEquals(Summary.objects.first().latmax, 4300000)
        self.assertEquals(Summary.objects.first().datemin,
                          datetime(2015, 1, 1, tzinfo=utc))
        self.assertEquals(Summary.objects.first().datemax,
                          datetime(2015, 1, 2, tzinfo=utc))
