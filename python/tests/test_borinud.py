import os
from datetime import datetime

from django.test import TestCase
import dballe

import borinud.utils.source


class TestDballeDB(TestCase):
    def setUp(self):
        db = dballe.DB.connect("sqlite:/tmp/test.db")
        db.reset()
        with db.transaction() as tr:
            tr.insert_station_data({
                "lon": 1212345,
                "lat": 4312345,
                "report": "test",
                "datetime": datetime(2019, 1, 2, 3, 4),
                "B12101": 0.1
            }, can_add_stations=True)

        self.db = borinud.utils.source.DballeDB("sqlite:/tmp/test.db")

    def tearDown(self):
        os.unlink("/tmp/test.db")

    def test_query_stations(self):
        stations = list(self.db.query_stations({}))
        self.assertEqual(len(stations), 1)
        self.assertEqual(stations[0]["ident"], None)
        self.assertEqual(stations[0]["report"], None)
        self.assertEqual(stations[0]["longitude"], None)
