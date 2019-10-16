import dballe


def params2record(p):
    q = {
        k2: p.get(k1) if p.get(k1) != "-" else None
        for k1, k2 in (
            ("ident", "ident"),
            ("lon", "lon"),
            ("lat", "lat"),
            ("network", "rep_memo"),
            ("tr", "pindicator"),
            ("p1", "p1"),
            ("p2", "p2"),
            ("lt1", "leveltype1"),
            ("lv1", "l1"),
            ("lt2", "leveltype2"),
            ("lv2", "l2"),
            ("var", "var")
        )
        if k1 in p and p.get(k1) != "*"
    }

    for k,v in q.items():
        if k in ("ident","lon","lat","pindicator","p1", "p2","leveltype1", "l1","leveltype2","l2"):
            try:
                q[k]=int(v)
            except:
                pass

    return q
