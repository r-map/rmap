#!/usr/bin/python3
import dballe

from pathlib import Path

dstypes=["report_fixed","report_mobile","sample_fixed","sample_mobile"]

for dstype in dstypes:
    for path in Path(dstype).rglob('*.bufr'):
        file=path.as_posix()
        print(file)

        # update from file
        with dballe.Explorer(dstype+".json") as explorer:
            with explorer.update() as updater:
                importer = dballe.Importer("BUFR")
                with importer.from_file(file) as message:
                    try:
                        updater.add_messages(message)
                    except Exception as e:
                        print (e)
                
            print ("updated from file")
            #print (explorer.all_reports)
            #print (explorer.all_levels)
            #print (explorer.all_tranges)
            #print (explorer.all_varcodes)
            print (explorer.stats)
