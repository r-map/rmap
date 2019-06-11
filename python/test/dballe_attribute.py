import dballe

db = dballe.DB.connect_from_url("sqlite:/dev/shm/tmp.sqlite")

for r in db.query_data(dballe.Record()):
    a = db.attr_query_data(r["context_id"])
    try:
        if a.var("B33196").get() == 1:
            print("da rimuovere")
        else:
            print("B33196=",a.var("B33196").get())
    except KeyError:
        print("B33196 assente")

