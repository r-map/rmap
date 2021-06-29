from arkimet.scan.bufr import Scanner, read_area_mobile, read_proddef


def scan(msg, md):
    if msg.report == "mobile":
        area = read_area_mobile(msg)
        proddef = read_proddef(msg)
        if area:
            md["area"] = {"style": "GRIB", "value": area}
        if proddef:
            md["proddef"] = {"style": "GRIB", "value": proddef}
    else:
        return False


Scanner.register("generic", scan, priority=1)
