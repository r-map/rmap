import unittest

import mqtt2bufr


class TestMqtt2bufr(unittest.TestCase):

    def test_parse_topic(self):
        obj = mqtt2bufr.parse_topic("rootpath/-/1212345,4312345/test/254,0,0/103,2000,-,-/B12101",0)
        self.assertEqual(sorted(obj.keys()),
                         sorted(["user", "ident", "lon", "lat", "rep_memo", "level",
                                 "trange", "var"]))
        self.assertEqual(obj["user"], None)
        self.assertEqual(obj["ident"], None)
        self.assertEqual(obj["lon"], 1212345)
        self.assertEqual(obj["lat"], 4312345)
        self.assertEqual(obj["level"], (103, 2000, None, None))
        self.assertEqual(obj["trange"], (254, 0, 0))
        self.assertEqual(obj["var"], "B12101")


    def test_parse_topic_v1(self):
        obj = mqtt2bufr.parse_topic("1/report/pat1//1254321,4554321/test/254,0,0/103,2000,-,-/B12101",1)
        self.assertEqual(sorted(obj.keys()),
                         sorted(["user", "ident", "lon", "lat", "rep_memo", "level",
                                 "trange", "var"]))
        self.assertEqual(obj["user"], "pat1")
        self.assertEqual(obj["ident"], None)
        self.assertEqual(obj["lon"], 1254321)
        self.assertEqual(obj["lat"], 4554321)
        self.assertEqual(obj["level"], (103, 2000, None, None))
        self.assertEqual(obj["trange"], (254, 0, 0))
        self.assertEqual(obj["var"], "B12101")



        
if __name__ == '__main__':
    unittest.main()
