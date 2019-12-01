import dballe


def params2record(p):
    q = {**{
        k2: p.get(k1) if p.get(k1) != "-" else None
        for k1, k2 in (
            ("ident", "ident"),
            ("network", "report"),
            ("var", "var")
        )
        if k1 in p and p.get(k1) != "*"
    }}
    
    q = {**q,**{
        k2: int(p.get(k1)) if p.get(k1) != "-" else None
        for k1, k2 in (
            ("lon", "lon"),
            ("lat", "lat"),
            ("tr", "pindicator"),
            ("p1", "p1"),
            ("p2", "p2"),
            ("lt1", "leveltype1"),
            ("lv1", "l1"),
            ("lt2", "leveltype2"),
            ("lv2", "l2"),
        )
        if k1 in p and p.get(k1) != "*" and p.get(k1) is not None
    }}


    return q
