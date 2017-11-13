With this firmware is possible to publish data on an RMAP server.
Only german and italian translation are available.
For now only SDS011 data are published (other data are not required on RMAP platform).
Short instructions:

    register on RMAP to get username and password:
    http://rmap.cc/registrazione/register/

    if you do not know you coordinates make a new station on server:
    http://rmapv.rmap.cc/insertdata/newstation
    insert your address or use "draw a marker" instrument on map and insert the station name.
    You can use the default name "luftdaten"
    Select "luftdaten" as station model type

Follow the standard procedure as for luftdaten.info
http://luftdaten.info/it/traduzione-delle-istruzioni-per-il-montaggio-del-sensore-di-polveri-sottili/
but in the configuration page activate RMAP send (the default) and insert RMAP specifications:

    username
    password
    station name (default luftdaten)
    Save and all is done.

The coordinate are getted from RMAP server, but you can set it manually in configuration page.
To refresh coordinate from server you can delete the coordinate field in configuration page and reboot the station.
