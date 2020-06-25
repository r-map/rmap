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
import time

logger = logging.getLogger(__name__)

# Devo convertire gli id in bcode. Alcuni mancano o non sono in grado io di
# stabilire una corrispondenza...
VARIABLE_BCODES = {
    "temperature": {"bcode":"B12101","a":1.,"b":273.15,"level":(265, 1),"trange":(254, 0, 0)},
    "humidity": {"bcode":"B13003","a":1.,"b":0.,"level":(265, 1),"trange":(254, 0, 0)},
    "P1":{"bcode":"B15195","a":0.000000001,"b":0.,"level":(103, 2000),"trange":(254, 0, 0)},
    "P2": {"bcode":"B15198","a":0.000000001,"b":0.,"level":(103, 2000),"trange":(254, 0, 0)},
    "others": {"bcode":None,"a":0.000000001,"b":0.,"level":(103, 2000),"trange":(254, 0, 0)}
}

current=0

DATASTORE_URL = "http://api.luftdaten.info/static/v1/data.json"

def iter_datastore(url):
    logger.info("Loading data from {}".format(url))
    records = json.load(urlopen(url))
    if len(records) == 0:
        logging.debug("No records found, stopping")
    else:
        logger.debug("Found {} records".format(len(records)))
        for r in records:
            yield r

def export_data(outfile,datetimemin=None,lonmin=None,latmin=None,lonmax=None,latmax=None):
    db = dballe.DB.connect("mem:")
    #db.reset()

    for data in iter_datastore(DATASTORE_URL):

        try:
            lon=float(data["location"]["longitude"])
        except Exception as e:
            logging.warning(str(data["location"]))
            #logging.exception(e)
            continue
        try:
            lat=float(data["location"]["latitude"])
        except Exception as e:
            logging.warning(str(data["location"]))
            #logging.exception(e)
            continue

        try:
            with db.transaction() as tr:
                tr.insert_station_data(
                    {"lon":lon,
                     "lat":lat,
                     "report":"luftdaten",
                     "B01019":str(data["location"]["id"])},
                    can_add_stations=True, can_replace=True)
        except Exception as e:
            logging.exception(e)

        rec={}
        havetowrite=False
        for sensordatavalues in data["sensordatavalues"]:
            key=sensordatavalues["value_type"]
            var=VARIABLE_BCODES.get(key)
            if var is None:
                logger.info("Var for variable {} not found, skipping".format(key))
            else:
                try:
                    rec["lon"]=lon
                    rec["lat"]=lat
                    rec["report"]="luftdaten"
                    bcode =var["bcode"]
                    rec[bcode] = float(sensordatavalues["value"])*var["a"]+var["b"]
                    rec["level"] = dballe.Level(*var["level"])
                    rec["trange"] = dballe.Trange(*var["trange"])
                    rec["datetime"] = datetime.strptime(data["timestamp"], "%Y-%m-%d %H:%M:%S")
                    havetowrite=True
                    
                except Exception as e:
                    logging.exception(e)
                    #rec[bcode]=None

            if havetowrite:
                try:
                    tr.insert_data(rec, can_replace=True)
                except Exception as e:
                    logging.exception(e)
                    print (rec)

    exporter = dballe.Exporter("BUFR")
    with open(outfile, "wb") as outfile:
        for row in tr.query_messages(
                {"datetimemin":datetimemin,
                 "lonmin":lonmin,
                 "latmin":latmin,
                 "lonmax":lonmax,
                 "latmax":latmax}):
            outfile.write(exporter.to_binary(row.message))
        
    #db.export_to_file(dballe.Record(datemin=datetimemin,lonmin=lonmin,latmin=latmin,lonmax=lonmax,latmax=latmax), filename=outfile,
    #                  format="BUFR", generic=True)


def main():

    """
    example: python arpae_aq_ckan_to_bufr/__init__.py --verbose --low=190388 --yearmin=2017 --monthmin=01 --daymin=01 tmp.bufr
    """
    from argparse import ArgumentParser
    parser = ArgumentParser()
    parser.add_argument("--verbose", action="store_true")
    parser.add_argument("--debug", action="store_true")
    parser.add_argument("outfile")

    parser.add_argument("--daymin",default=1,type=int,help='day min to extract')
    parser.add_argument("--monthmin",default=1,type=int,help='month min to extract')
    parser.add_argument("--yearmin",type=int,help='year min to extract')

    parser.add_argument("--hourmin",default=0,type=int,help='hour min to extract')
    parser.add_argument("--minmin",default=0,type=int,help='min min to extract')


    parser.add_argument("--lonmin",default=6.,type=int,help='lon min for data to extract')
    parser.add_argument("--latmin",default=43.,type=int,help='lat min for data to extract')
    parser.add_argument("--lonmax",default=14.,type=int,help='lon max for data to extract')
    parser.add_argument("--latmax",default=47.,type=int,help='lat max for data to extract')

    
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

    # in crontab we execute this every 3 minutes no we do not want to excede this
    # retry is due to poor web API of luftdaten; a lot of time the downloaded file is corrupted
    retry=0
    while retry<6:
        try:
            export_data(args.outfile,datetimemin,args.lonmin,args.latmin,args.lonmax,args.latmax)
            break
        except Exception as e:
            logging.exception(e)
            time.sleep(30)
            retry+=1
            
if __name__ == '__main__':
    main()
