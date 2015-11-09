# encoding: utf-8
# borinud/v1/utils - v1 utils for borinud
# Author: Emanuele Di Giacomo <emanueledigiacomo@gmail.com>

def path2query(d):
    q = {
        k: d[k] if d[k] != "-" else None
        for k in ["ident", "lon", "lat", "network",
                  "var", "tr", "p1", "p2", "lt1", "lv1", "lt2", "lv2"]
        if k in d and d[k] != "*"
    }
    if "coords" in d and d["coords"] == "*":
        del q["lon"]
        del q["lat"]

    if "trange" in d and d["trange"] == "*":
        del q["tr"]
        del q["p1"]
        del q["p2"]

    if "level" in d and d["level"] == "*":
        del q["lt1"]
        del q["lv1"]
        del q["lt2"]
        del q["lv2"]

    return q
