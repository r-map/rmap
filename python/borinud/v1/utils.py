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

    # https://github.com/ARPA-SIMC/dballe/issues/201
    #Viene aggiunta la possibilità di specificare
    #leveltype2=MISSING_INT-1 o leveltype2="-", per significare "il
    #valore nel database deve essere "mancante" invece di "qualunque"

    # solo per sistemi a 64 bit 
    REQUIRED_MISSING_INT = 2147483646
    
    for tl2 in ("leveltype2","l2"):
        if (tl2 in q):
            if (q[tl2] is None):
                q[tl2]= REQUIRED_MISSING_INT

    return q
