borinud
=======

.. contents::

Introduction
------------

**TODO**


Dependencies
------------

- `python-dballe <http://sourceforge.net/projects/wreport/>`_
- `python-bottle <http://bottlepy.org/>`_

Installation
------------

From source code:

::

    python setup.py install

Context url
-----------

::

    /IDENT/COORDINATES/NETWORK/TIMERANGE/LEVEL/BCODE

- ``IDENT``: mobile station identifier or ``-`` for fixed stations
- ``COORDINATES``: coordinates in format ``int(lon*10^5),int(lat*10^5)``
  - e.g.: ``1212345,4467890`` means longitude 12.12345 and latitude 44.67890
- ``NETWORK``: station network (max 16 characters)
- ``TIMERANGE``: time range in format ``type,p1,p2``
- ``LEVEL``: level in format ``type1,l1,type2,l2`` (``-`` means missing)
- ``BCODE``: local B table descriptor if format ``BXXYYY``

Example of context url
^^^^^^^^^^^^^^^^^^^^^^

Instantaneous temperature at 2m above ground measured by fixed station at
longitude 12.12345, latitude 44.67890 in network "agrmet"::

    /-/1212345,4467890/agrmet/254,0,0/103,2000,-,-/B12101

Web interface
-------------

A simple web interface is available at the base dir ``/``.


Web API
-------

The response is in ``GeoJSON`` format (http://geojson.org/)::

    {
      "type": "FeatureCollection",
      "features": [
        {
          "geometry": { "type": "Point", "coordinates": [ 11.95861, 44.36013 ] },
          "properties": {
            "ident": null,
            "lon": 1195861,
            "lat": 4436013,
            "network": "locali",
            "trange_pind": 254,
            "trange_p1": 0,
            "trange_p2": 0,
            "level_t1": 103,
            "level_v1": 2000,
            "level_t2": null,
            "level_v2": null,
            "bcode": "B12101"
          }
        },
        {
          ...
        }
      ]
    }

The services can extend the property list (e.g. ``datetime`` and ``value`` for timeseries services).

The services support the ``JSONP`` format, using the ``GET`` parameter ``callback=NAME``.

Summaries
^^^^^^^^^

::

    /*/*/*/*/*/*/summaries
    /*/*/NETWORK/*/*/*/summaries
    /-/COORDINATES/NETWORK/*/*/*/summaries
    /IDENT/*/NETWORK/*/*/*/summaries
    /*/*/NETWORK/TIMERANGE/LEVEL/BCODE/summaries/YEAR/MONTH
    /*/*/NETWORK/TIMERANGE/LEVEL/BCODE/summaries/YEAR/MONTH/DAY

The features have a ``datetime`` property: if the date extremes are available,
then is an array with min and max date. Otherwise, is ``null``.

Time series
^^^^^^^^^^^

::

    /IDENT/COORDINATES/NETWORK/TIMERANGE/LEVEL/BCODE/timeseries/YEAR
    /IDENT/COORDINATES/NETWORK/TIMERANGE/LEVEL/BCODE/timeseries/YEAR/MONTH
    /IDENT/COORDINATES/NETWORK/TIMERANGE/LEVEL/BCODE/timeseries/YEAR/MONTH/DAY

The features have a ``datetime`` and ``value`` property.

Example of timeseries
"""""""""""""""""""""

::

    $ curl http://host/path/to/-/1200000,4400000/agrmet/254,0,0/103,10000,-,-/B12101/timeseries/2011
    $ curl http://host/path/to/-/1200000,4400000/agrmet/254,0,0/103,10000,-,-/B12101/timeseries/2011/01
    $ curl http://host/path/to/-/1200000,4400000/agrmet/254,0,0/103,10000,-,-/B12101/timeseries/2011/01/01

Spatial series
^^^^^^^^^^^^^^

::

    /*/*/NETWORK/TIMERANGE/LEVEL/BCODE/spatialseries/YEAR/MONTH/DAY/HOUR

The features have a ``datetime`` and ``value`` property.

Station data
^^^^^^^^^^^^

::

    /-/COORDINATES/NETWORK/-,-,-/257,-,-,-/*/stationdata
    /IDENT/*/-,-,-/257,-,-,-/*/stationdata

The features have a ``value`` property.

Simple web server
-----------------

The ``borinud.ws`` module has a simple web server tool::

    python -m borinud.ws --dsn=sqlite://...

Or run ``python -m borinud.ws --help`` for more options.

Deployement under Apache with mod_wsgi
--------------------------------------

The Apache configuration is something like this (see
http://code.google.com/p/modwsgi/ for details)::

    <VirtualHost *:80>
      WSGIDaemonProcess borinud user=www-data group=www-data processes=10 threads=1
      WSGIScriptAlias /borinud /var/www/borinud/app.wsgi
    </VirtualHost>

And you must create the file ``/var/www/borinud/app.wsgi``::

    import borinud.ws
    import borinud.db

    dba_dsn = "sqlite:/dev/shm/borinud.db"

    application = borinud.ws.app
    application.config.db = borinud.db.DballeDB(dba_dsn)

Database
--------

``borinud`` reads data from a ``DB-All.e`` database::

    from borinud.db import DballeDB
    # Create a DB-All.e database
    db = DballeDB("sqlite:mydatabase.sqlite")
    # Load the summary
    summary = db.query_summary(dballe.Record())
    # Load temperature data
    data = db.query_data(dballe.Record(var="B12101"))

Caching the summary
^^^^^^^^^^^^^^^^^^^

If the database has a lot of data, loading the summary could be a bottle neck.
You can dump the summary in a file and use it as a preemptive cache::

    import dballe
    from borinud.db import DballeDB, SummaryCacheDB
    # Create a DB-All.e database with cached summary
    db = SummaryCacheDB(DballeDB("sqlite:mydatabase.sqlite"), "/var/lib/borinud-summary.json")
    # Dump cached summary in /var/lib/borinud-summary.json.
    # NOTE: the cached summary must be written before using it
    db.write_cached_summary()
    # Load the summary
    summary = db.query_summary(dballe.Record())
    # Load temperature data
    data = db.query_data(dballe.Record(var="B12101"))
    # Update the summary cache
    db.write_cached_summary()

The web service can use a summary cache::

    python -m borinud.ws --dsn=sqlite:mydatabase.sqlite --cached-summary=/var/lib/borinud-summary.json

And the cache can be updated by a simple script::

    #!/usr/bin/python
    # cacheupdater.py
    from borinud.db import DballeDB, SummaryCacheDB
    db = SummaryCacheDB(DballeDB("sqlite:mydatabase.sqlite"), "/var/lib/borinud-summary.json")
    db.write_cached_summary()

This script can be executed by ``cron`` to update periodically the summary::

    */10 * * * * python cacheupdater.py

.. note::
    When the web service starts, the summary *must* be already created::

        python cacheupdater.py && python -m borinud.ws --dsn=sqlite:mydatabase.sqlite --cached-summary=/var/lib/borinud-summary.json

Copyright
---------

::

  Copyright (C) 2013 ARPA-SIM <urpsim@smr.arpa.emr.it>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
