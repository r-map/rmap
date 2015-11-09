# encoding: utf-8
# borinud/v1/utils - v1 utils for borinud
# Author: Emanuele Di Giacomo <emanueledigiacomo@gmail.com>

def path2query(d):
    return {
        k: d[k] if d[k] != "-" else None
        for k in ["ident", "lon", "lat", "network",
                  "var", "tr", "p1", "p2", "lt1", "lv1", "lt2", "lv2"]
        if k in d and d[k] != "*"
    }
