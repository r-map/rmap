# borinud/codec.py - encoding/decoding utilities
#
# Copyright (C) 2013-2015  ARPA-SIM <urpsim@smr.arpa.emr.it>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
# Author: Emanuele Di Giacomo <edigiacomo@arpa.emr.it>
import json
import dballe

class BaseJSONEncoder(json.JSONEncoder):
    """Base JSON encoder."""
    def default(self, o):
        from datetime import datetime
        if isinstance(o, datetime):
            return o.isoformat()
        else:
            return super(BaseJSONEncoder, self).default(o)

class GeoJSONEncoder(BaseJSONEncoder):
    """GeoJSON encoder."""
    def encode(self, o):
        try:
            iterable = iter(o)
        except TypeError:
            return super(GeoJSONEncoder, self).encode(o)
        else:
            return super(GeoJSONEncoder, self).encode(self.toFeatureCollection(o))

    def default(self, o):
        from datetime import datetime
        if isinstance(o, datetime) and o == datetime(1000, 1, 1, 0, 0, 0):
                return None
        else:
            return super(GeoJSONEncoder, self).default(o)

    def toSkip(self, record):
        """True if the record must be skipped, false otherwise."""
        from datetime import datetime
        if record.get("date") == datetime(1000, 1, 1, 0, 0, 0) and record.get("var") in ( "B05001", "B06001", "B01194" ):
            return True
        else:
            return False

    def toFeatureCollection(self, cursor):
        """Convert a collection of items to a feature collection."""
        return {
            "type": "FeatureCollection",
            "features": [ self.toFeature(r) for r in cursor if not self.toSkip(r) ]
        }

    def toFeature(self, rec):
        """Convert a record to a feature."""
        return {
            "type": "Feature",
            "geometry": {
                "type": "Point",
                "coordinates": [ rec["lon"], rec["lat"] ],
            },
            "properties": self.toProperties(rec)
        }

    def toProperties(self, rec):
        """Get feature properties from a record."""
        p = {
            "ident": rec.get("ident"),
            "lon": rec.key("lon").enqi(),
            "lat": rec.key("lat").enqi(),
            "network": rec["rep_memo"],
            "level_t1": rec["level"][0],
            "level_v1": rec["level"][1],
            "level_t2": rec["level"][2],
            "level_v2": rec["level"][3],
            "trange_pind": rec["trange"][0],
            "trange_p1": rec["trange"][1],
            "trange_p2": rec["trange"][2],
            "bcode": rec["var"],
            "datetime": None
        }
        if rec.get("date"):
            p["datetime"] = rec.get("date")
            p["value"] = rec.get(rec["var"])
        elif rec.date_extremes() != (None, None):
            p["datetime"] = rec.date_extremes()
        return p
