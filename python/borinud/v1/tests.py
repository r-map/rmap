import json

from django.test import TestCase, Client

import dballe


class TestUtils(TestCase):
    def test_summaries_all(self):
        c = Client()
        response = c.get("/borinud/api/v1/geojson/*/*/*/*/*/*/summaries")
        geojson = json.loads(response.content.decode("utf-8"))
        self.assertTrue("type" in geojson)
        self.assertEquals(geojson["type"], "FeatureCollection")
        self.assertTrue("features" in geojson)
        self.assertTrue(len(geojson["features"]) > 0)

    def test_summaries_by_network(self):
        c = Client()

        response = c.get("/borinud/api/v1/geojson/*/*/boa/*/*/*/summaries")
        geojson = json.loads(response.content.decode("utf-8"))
        self.assertTrue(all([
            f["properties"]["network"] == "boa"
            for f in geojson["features"]
        ]))

    def test_summaries_by_fixed_station(self):
        c = Client()
        response = c.get("/borinud/api/v1/geojson/-/1251139,4452250/boa/*/*/*/summaries")
        geojson = json.loads(response.content.decode("utf-8"))
        self.assertTrue(all([
            (f["properties"]["ident"],
             f["properties"]["lon"],
             f["properties"]["lat"],
             f["properties"]["network"]) == (None, 1251139, 4452250, "boa")
            for f in geojson["features"]
        ]))
