import csv
import json
import codecs
from datetime import datetime,timedelta
import logging
try:
    from urllib.request import urlopen
except ImportError:
    from urllib2 import urlopen
try:
    from urllib.parse import urlsplit, parse_qsl, urlencode, urlunsplit
except ImportError:
    from urllib import urlencode
    from urlparse import urlsplit, urlunsplit, parse_qsl

import dballe


logger = logging.getLogger(__name__)

# Devo convertire gli id in bcode. Alcuni mancano o non sono in grado io di
# stabilire una corrispondenza...
VARIABLE_BCODES = {
    1: "B15197",
    5: "B15195",
    7: "B15194",
    8: "B15193",
    9: None,
    10: "B15196",
    20: "B15236",
    21: None,
    38: "B15192",
    111: "B15198",
    82: None,
}

current=0

STATIONS_URL = "https://docs.google.com/spreadsheets/d/1-4wgZ8JeLeg0bODTSFUrshPY-_y9mERUu0FJtSFr78s/export?format=csv"
VARIABLES_URL = "https://docs.google.com/spreadsheets/d/1K6vRcShjje2CDnvnk39o3jkU4ihgpA7rfsdAV1S-yXU/export?format=csv"
DATASTORE_URL = "https://dati.arpae.it/api/action/datastore_search?resource_id=4dc855a1-6298-4b71-a1ae-d80693d43dcb"

# max step = 32000
def iter_datastore(low=0, step=30000, high=None):
    current = low
    urlparts = list(urlsplit(DATASTORE_URL))
    querydict = dict(parse_qsl(urlparts[3]))
    while True:
        if high is not None and current > high:
            break
        else:
            querydict.update({
                "limit": step,
                "offset": current,
            })
            urlparts[3] = urlencode(querydict)
            nexturl = urlunsplit(urlparts)
            logger.info("Loading data from {}".format(nexturl))
            resp = json.load(codecs.getreader("utf-8")(urlopen(nexturl)))
            records = resp["result"]["records"]
            if len(records) == 0:
                logging.info("No records found, stopping")
                break
            else:
                logger.info("Found {} records".format(len(records)))
                current += len(records)
                for r in records:
                    yield r


def load_stations():
    stations = {}
    logging.info("Loading stations from {}".format(STATIONS_URL))
    resp = urlopen(STATIONS_URL)
    reader = csv.DictReader(codecs.getreader("utf-8")(resp))
    for row in reader:
        a,b,c=row["Cod_staz"].split(".")
        key = int(a+b+c)
        altezza=row["Altezza"].replace(",", ".")
        if altezza =="":
            altezza = None
        else:
            altezza=float(altezza)
        rec = {"B01019":row["Stazione"],
               "B07030":altezza,
               "lon":float(row["LON_GEO"]), "lat":float(row["LAT_GEO"]),
               "rep_memo":"arpae-aq"}
        stations[key] = rec

    return stations


def load_variables():
    variables = {}
    logging.info("Loading variables from {}".format(VARIABLES_URL))
    resp = urlopen(VARIABLES_URL)
    reader = csv.DictReader(codecs.getreader("utf-8")(resp))
    for row in reader:
        key = int(row["IdParametro"])
        bcode = VARIABLE_BCODES.get(key)
        if bcode is None:
            logger.warning("Var for variable {} not found, skipping".format(key))
        else:
            variables[key] = {
                "var": bcode,
                "level": (103, 2000),
                "trange": (0, 0, int(row["Tmed (min)"])*60),
            }

    return variables


def export_data(outfile,low=0,high=None,datetimemin=None):
    db = dballe.DB.connect("mem:")
    db.reset()
    last=low
    stations = load_stations()
    variables = load_variables()
    with db.transaction() as tr:
        for rec in stations.values():
            tr.insert_station_data(rec, can_add_stations=True)

    with db.transaction() as tr:
        for row in iter_datastore(low=low,high=high):
            #last+=1
            last=row["_id"]
            variable = variables.get(int(row["variable_id"]))
            station = stations.get(int(row["station_id"]))
            reftime = datetime.strptime(row["reftime"], "%m/%d/%Y %H:%M")
            value = row["value"]
            if variable is None:
                logger.debug("Unknown variable {}, skipping".format(row["variable_id"]))
                continue
            elif station is None:
                logger.debug("Unknown station {}, skipping".format(row["station_id"]))
                continue
            else:
                rec = {**{
                    k: station.get(k)
                    for k in ("ident", "lon", "lat", "rep_memo")
                }}
                try:
                    rec["year"] = reftime.year
                    rec["month"] = reftime.month
                    rec["day"] = reftime.day
                    rec["hour"] = reftime.hour
                    rec["min"] = reftime.minute
                    rec["sec"] = reftime.second
                    rec[variable["var"]] = float(value) * 10**-9
                    rec["level"] = variable["level"]
                    rec["trange"] = variable["trange"]
                    tr.insert_data(rec)
                except OverflowError:
                    logger.warning("Error encoding/write message: OverflowError {} {}".format(variable["var"],value))
                    
                except:
                    logger.error("Error encoding/write message")


    exporter = dballe.Exporter("BUFR")
    with open(outfile, "wb") as outfile:
        with db.transaction() as tr:
            for row in tr.query_messages({"datetimemin":datetimemin}):
                outfile.write(exporter.to_binary(row.message))
                    
    return last+1


def main():

    """
    example: python arpae_aq_ckan_to_bufr/__init__.py --verbose --low=190388 --yearmin=2017 --monthmin=01 --daymin=01 tmp.bufr
    """
    from argparse import ArgumentParser
    parser = ArgumentParser()
    parser.add_argument("--verbose", action="store_true")
    parser.add_argument("--debug", action="store_true")
    parser.add_argument("outfile")
    parser.add_argument("--low", default=0,type=int,help='start download data from this record')

    parser.add_argument("--daymin",default=1,type=int,help='day min to extract')
    parser.add_argument("--monthmin",default=1,type=int,help='month min to extract')
    parser.add_argument("--yearmin",type=int,help='year min to extract')

    parser.add_argument("--hourmin",default=0,type=int,help='hour min to extract')
    parser.add_argument("--minmin",default=0,type=int,help='min min to extract')

    parser.add_argument("--nlastdays",type=int,help='extract this number of day back in time')

    args = parser.parse_args()

    logformat = '%(levelname)s: %(message)s'
    loglevel = logging.INFO

    if args.verbose:
        loglevel = logging.INFO
    elif args.debug:
        loglevel = logging.DEBUG
    else:
        loglevel = logging.WARN

    logging.basicConfig(level=loglevel, format=logformat)

    if args.yearmin is None:
        datetimemin=None
    else:
        datetimemin=datetime(args.yearmin, args.monthmin, args.daymin, args.hourmin, args.minmin)

    if not args.nlastdays is None:
        datetimemin=(datetime.now()-timedelta(days=args.nlastdays)).replace(hour=0, minute=0, second=0, microsecond=0)

    logging.info("extract data starting from: "+str(datetimemin))


    try:
        last = export_data(args.outfile,low=args.low,datetimemin=datetimemin)
        logging.info("extracted. Last record: {}".format(last))
    except Exception as e:
        logging.exception(e)


if __name__ == '__main__':
    main()
