Server RMAP 
=====================================

Installazione sistema operativo Centos 8
----------------------------------------

Installare Centos 8.

Aggiunta repository e installazione pacchetti
::

  dnf -y install epel-release
  dnf install yum-plugin-copr
  dnf copr enable simc/stable
  dnf copr enable pat1/rmap
  dnf config-manager --set-enabled PowerTool
  dnf groupinstall rmap
  dnf install mosquitto mosquitto-auth-plug
  dnf install arkimet
  useradd rmap

/etc/selinux/config::

  SELINUX=disabled

/etc/tmpfiles.d/rmap.conf::

  d /run/wsgirmap 0755 rmap rmap -
  d /var/run/rmap 0755 rmap rmap -
  d /var/run/httpd 0755 rmap rmap -

::
   
   mkdir /rmap
   chmod go+rx /rmap

/etc/sysconfig/crond::

  CRONDARGS=-s -m off

::

   mkdir /var/log/rmap
   chown -R rmap:rmap /var/log/rmap
   
postgresql
----------

dnf module disable postgresql:10 dnf module enable postgresql:12 dnf
install postgresql-server postgresql-contrib dnf install
python3-psycopg2
::
   
   postgresql-setup --initdb --unit postgresql

/var/lib/pgsql/data/pg_hba.conf
::
   
   # TYPE  DATABASE        USER            ADDRESS                 METHOD
   # "local" is for Unix domain socket connections only
   # allow postgres user to use "ident" authentication on Unix sockets
   local   all             postgres           ident
   # allow all other users to use "md5" authentication on Unix sockets
   local   all             all                                     md5
   # IPv4 local connections:
   host    all             all             127.0.0.1/32            md5
   # IPv6 local connections:
   host    all             all             ::1/128                 md5

/var/lib/pgsql/data/postgresql.conf
::
   
   max_connections = 100
   shared_buffers = 128MB
   work_mem = 100MB
   maintenance_work_mem = 200MB
   effective_cache_size = 1GB
   
::

   mkdir /etc/systemd/system/postgresql.service.d/

/etc/systemd/system/postgresql.service.d/rmap.conf
::
   
 [Service]
 # Location of database directory
 Environment=PGDATA=/rmap/pgsql/data

 mkdir /rmap/pgsql/
 chown postgres:postgres /rmap/pgsql/
 mv /var/lib/pgsql/data /rmap/pgsql/

 su - postgres
 initdb
 exit
 
 systemctl enable postgresql.service
 systemctl start postgresql.service

 su - postgres
 createuser -P -e rmapadmin
 createdb --owner=rmapadmin rmapadmin
 exit

/etc/rmap/rmap-site.cfg
::
   
   [database]
   DATABASE_ENGINE = 'postgresql_psycopg2' # 'postgresql_psycopg2', 'postgresql', 'mysql', 'sqlite3' or 'ado_mssql'.
   DATABASE_NAME = 'rmapadmin'             # Or path to database file if using sqlite3.
   DATABASE_USER = 'rmapadmin'             # Not used with sqlite3.
   DATABASE_PASSWORD = 'rmapadmin'         # Not used with sqlite3.
   DATABASE_HOST = 'localhost'             # Set to empty string for localhost. Not used with sqlite3.
   DATABASE_PORT = '5432'                  # Set to empty string for default. Not used with sqlite3.

::
   
   rmapctrl --syncdb

::
   
   su - postgres
   createuser -P -e rmap
   createdb --owner=rmap report_fixed
   createdb --owner=rmap report_mobile
   createdb --owner=rmap sample_fixed
   createdb --owner=rmap sample_mobile

   exit

apache
------

Collect static files from django apps:
::
   
   mkdir global_static
   export DJANGO_SETTINGS_MODULE=rmap.settings
   django-admin collectstatic
   rmdir global_static

   yum install python3-mod_wsgi

   useradd -r rmap
   mkdir /home/rmap
   chown rmap:rmap /home/rmap
   mkdir /rmap/cache
   chown rmap:rmap /rmap/cache

   
/etc/httpd/conf.modules.d/00-mpm.conf

::

   LoadModule mpm_worker_module modules/mod_mpm_worker.so``

   #StartServers          2
   #MaxClients          150
   #MinSpareThreads      25
   #MaxSpareThreads      75
   #ThreadsPerChild      25
   #MaxRequestsPerChild   0
   ServerLimit         16
   StartServers         2
   MaxRequestWorkers  150
   MinSpareThreads     10
   MaxSpareThreads     35
   ThreadsPerChild     15
   MaxRequestWorkers      240
   MaxConnectionsPerChild 10000

/etc/httpd/conf.d/rmap.conf

::
   
   ServerName rmap.it
   WSGISocketPrefix /run/wsgirmap/rmap
   WSGIDaemonProcess www.rmap.cc user=rmap group=rmap maximum-requests=100 graceful-timeout=200 processes=10 threads=5 request-timeout=180 socket-timeout=180 header-buffer-  size=65000
   WSGIProcessGroup www.rmap.cc
   WSGIApplicationGroup %{GLOBAL}
   <VirtualHost *:80 >
   ServerName rmap.it
   ServerAlias rmap.cc rmapv.rmap.cc rmapv.rmap.it www.rmap.cc www.rmapv.rmap.cc www.rmap.it www.rmapv.rmap.it localhost localhost.localdomain 127.0.0.1 partecipa.rainbolife.eu
   Include conf.d/rmap.inc

/etc/httpd/conf.d/rmap.inc
::

   Alias /download /var/www/html/download
   Alias /repo     /var/www/html/repo
   Alias /showroom /var/www/html/showroom
   Alias /arkiweb  /var/www/html/arkiweb
 
   #Alias /static/admin /usr/lib/python2.7/site-packages/django/contrib/admin/static/admin
   #<Directory "/usr/lib/python2.7/site-packages/django/contrib/admin/static/admin">
   #	    Require all granted
   #</Directory>
 
   Alias /static   /usr/share/rmap/static
   <Directory /usr/share/rmap/static>
     Require all granted
     SetHandler None
   </Directory>
 
   Alias /media /usr/share/rmap/media
   <Directory /usr/share/rmap/media>
     Require all granted
     SetHandler None
   </Directory>
 
   WSGIScriptAlias / /usr/bin/rmap.wsgi
   #WSGIImportScript /usr/bin/rmap.wsgi process-group=%{GLOBAL} application-group=%{GLOBAL}
   #WSGIPythonPath /path/to/mysite.com
 
   #WSGIDaemonProcess rmap processes=5 threads=5
   #WSGIDaemonProcess rmap
   #WSGIProcessGroup rmap
 
   <Directory /usr/bin>
     <Files rmap.wsgi>
       Require all granted
     </Files>
   </Directory>
 
   <Location /auth>
     Order Deny,Allow
     Deny from all
     Allow from 127.0.0.1
   </Location>
 
   SecRuleEngine On
 
   #The first SecAction initializes the state, in this case by IP address.
   #The second SecAction deprecates the counter by 1 every 60 second.
   #This is setting the base rate of our rate limit
   #Then the Header definition ensures a header is set whenever a request
   #is rate limited, giving a hint to the client that they shouldn’t try
   #again for 10 seconds. This is obviously just a guide and a lot of
   #clients don’t implement it (and it’s really only valid on a 503 status
   #anyway) so it’s a little bit of wishful thinking really.
   #Then we define a neat ErrorDocument for the 509 response to give a better clue to the client about what is happening.
 
 
   <Location /borinud>
 
   # whitelist localhost
   #SecRule REMOTE_ADDR "@contains 127.0.0.1" "id:1,phase:1,nolog,allow,ctl:ruleEngine=Off"
   SecRule REMOTE_ADDR "^127.0.0.1$" nolog,allow,id:1
 
   # initialise the state based on ip address
   SecAction id:2,initcol:IP=%{REMOTE_ADDR}
 
   # set the base rate to one per 15 minutes
   SecAction id:3,deprecatevar:IP.CALLS_LIMIT=1/900
 
   # if greater then burst_rate_limit then pause set RATELIMITED var and then return 503
   SecRule IP:CALLS_LIMIT "@gt 60" "id:4,phase:2,pause:300,deny,status:503,setenv:RATELIMITED,skip:1"
 
   # if above rule doesnt match increment the count
   SecAction id:5,setvar:IP.CALLS_LIMIT=+1
 
   # set a header when ratelimited
   Header always set Retry-After "10" env=RATELIMITED
 
   </Location>
 
   ErrorDocument 503 "Service Unavailable"

::
   
   chkconfig httpd on``
   service httpd start``
   
Arkimet
-------

::
   
   dnf install arkimet arkimet-postprocessor-suite
   useradd  -r arkimet
   mkdir /home/arkimet
   chown arkimet:arkimet /home/arkimet
   mkdir /rmap/arkimet/
   chown -R arkimet:arkimet /rmap/arkimet/
   mkdir /var/log/arkimet
   chown -R arkimet:arkimet /var/log/arkimet


/etc/sysconfig/arkimet
::

   DATASET_CONFIG="/rmap/arkimet/arkimet.conf"


Create file /etc/arkimet/scan/bufr_generic_mobile_rmap.py:
::
   
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

::

 systemctl daemon-reload
 chkconfig arkimet on
 service arkimet start

Sincronizzazione DB
-------------------

Server di origine
~~~~~~~~~~~~~~~~~

::
   
   rmapctrl --dumpdata > dumpdata.json

rimuovere le prime righe che non sono json
::
   
   dbadb export --dsn="mysql:///report_fixed?user=rmap&password=****" > report_fixed.bufr
   dbadb export --dsn="mysql:///report_mobile?user=rmap&password=****" > report_mobile.bufr
   dbadb export --dsn="mysql:///sample_fixed?user=rmap&password=****" > sample_fixed.bufr
   dbadb export --dsn="mysql:///sample_mobile?user=rmap&password=****" > sample_mobile.bufr


Server di destinazione
~~~~~~~~~~~~~~~~~~~~~~

Da interfaccia web admin rimuovere TUTTI gli utenti (compreso rmap)
::
   
   rmapctrl --loaddata=dumpdata.json

::
   
   dbadb import --wipe-first --dsn="postgresql://rmap:***@localhost/report_fixed" report_fixed.bufr
   dbadb import --wipe-first --dsn="postgresql://rmap:***@localhost/report_mobile" report_mobile.bufr
   dbadb import --wipe-first --dsn="postgresql://rmap:***@localhost/sample_mobile" sample_mobile.bufr
   dbadb import --wipe-first --dsn="postgresql://rmap:***@localhost/sample_fixed" sample_fixed.bufr

::
   
   cd /usr/share/rmap/
   rsync -av utente@serverorigine:/usr/share/rmap/media .


Mosquitto
---------

::
   
   mkdir /etc/mosquitto/conf.d
   mkdir /rmap/mosquitto
   chown mosquitto:mosquitto /rmap/mosquitto

   
/etc/mosquitto/conf.d/rmap.conf
::
   
   persistent_client_expiration 1d
   allow_anonymous true
   password_file /etc/mosquitto/pwfile
   acl_file /etc/mosquitto/aclfile
   log_type error
   log_type warning
   auth_plugin /usr/lib64/auth-plug.so
   auth_opt_backends http
   auth_opt_http_hostname localhost
   auth_opt_http_ip 127.0.0.1
   auth_opt_http_port 80
   auth_opt_http_getuser_uri /auth/auth
   auth_opt_http_superuser_uri /auth/superuser
   auth_opt_http_aclcheck_uri /auth/acl
   persistence true
   persistence_location /rmap/mosquitto/

/etc/mosquitto/aclfile
::

   topic read #
   topic write test/#
 
   # This only affects clients with username "rmap".
   user rmap
   topic #
  
   pattern write rmap/%u/#
   pattern write sample/%u/#
   
   pattern write report/%u/#
   pattern write fixed/%u/#
 
   pattern write mobile/%u/#
 
   pattern write maint/%u/#
 
   pattern write rpc/%u/#

remove everythings and add in /etc/mosquitto/mosquitto.conf
::
   
   include_dir /etc/mosquitto/conf.d
   pid_file /var/run/mosquitto.pid

::
   
   touch /etc/mosquitto/pwfile
   chkconfig mosquitto on
   service mosquitto start

if the package use systemd:
   
create /etc/systemd/system/mosquitto.service.d/rmap.conf
::
   
   [Service] 
   Restart=always 
   RestartSec=15

if the package use systemV:

/etc/monit.d/mosquitto
::
   
   check process mosquitto with pidfile /var/run/mosquitto.pid
    start program = "/etc/init.d/mosquitto restart"
    stop program = "/etc/init.d/mosquitto stop"
    if failed host localhost port 1883 timeout 30 seconds retry 3 then restart

Rabbitmq
--------

::
   
   curl -s https://packagecloud.io/install/repositories/rabbitmq/rabbitmq-server/script.rpm.sh |bash
   curl -s https://packagecloud.io/install/repositories/rabbitmq/erlang/script.rpm.sh | sudo bash

   dnf install rabbitmq-server

in /etc/rabbitmq/rabbitmq.config
::

 [
   {rabbit, 
     [
       {auth_backends, [rabbit_auth_backend_internal, rabbit_auth_backend_http]},
       {loopback_users, []}
     ]
   },
   {rabbitmq_auth_backend_http,
     [{user_path,     "http://localhost/auth/user"},
       {vhost_path,    "http://localhost/auth/vhost"},
       {resource_path, "http://localhost/auth/resource"}
     ]
   }
 ].


 rabbitmq-plugins enable rabbitmq_auth_backend_http
 rabbitmq-plugins enable rabbitmq_management
 #rabbitmq-plugins enable rabbitmq_management_visualiser
 rabbitmq-plugins enable rabbitmq_shovel
 rabbitmq-plugins enable rabbitmq_shovel_management

::
 
 chkconfig rabbitmq-server on
 service rabbitmq-server start


login at management interface with user "guest" and password "guest"
on overview page use import definition to configure exchange, queue and users
with the same management interface remove "guest" user and login with a new real user

Per attivare uno showell:
::
   
   rabbitmqctl set_parameter shovel report_mobile '{"src-protocol": "amqp091", "src-uri": "amqp://rmap:<password>@rmap.cc", "src-queue": "report_mobile_saved", "dest-protocol": "amqp091", "dest-uri": "amqp://rmap:<password>@", "dest-queue": "report_mobile"}'

problema non risolto:
se si trasferiscono dati scritti da un utente autenticandosi con un altro utente la security su user_id lo vieta.
https://www.rabbitmq.com/shovel-dynamic.html
bisognerebbe riuscire a settare "user_id" tramite il parametro "dest-publish-properties" nel formato json sopra ma non funziona

Monit
-----

::
   
   yum install monit

comment everithings and add in /etc/monitrc
::

 set daemon  60              # check services at 1-minute intervals
 set log syslog
 set httpd port 5925 and
    allow rmap:<password>        # require user 'admin' with password 'monit'
    allow @monit           # allow users of group 'monit' to connect (rw)
    allow @users readonly  # allow users of group 'users' to connect readonly
 include /etc/monit.d/*

::
   
 cd /etc/monit.d/
 wget https://raw.githubusercontent.com/r-map/rmap/master/server/etc/monit.d/rmap

 chkconfig monit on
 service monit start

Cron
----

::

   cd /etc/cron.d
   wget https://raw.githubusercontent.com/r-map/rmap/master/server/etc/cron.d/arpae_aq_ckan
   wget https://raw.githubusercontent.com/r-map/rmap/master/server/etc/cron.d/dballe2arkimet
   wget https://raw.githubusercontent.com/r-map/rmap/master/server/etc/cron.d/luftdaten
   wget https://raw.githubusercontent.com/r-map/rmap/master/server/etc/cron.d/rmap_generate_summary_cache

Arkiweb
-------
AL MOMENTO NON DISPONIBILE SU CENTOS 8
NOT AVAILABLE ON CENTOS 8

::
   
   dnf install arkiweb

/etc/httpd/conf.d/arkiweb.conf
::
   
 ScriptAlias /services/arkiweb/ /usr/lib64/arkiweb/
 Alias /arkiweb  /var/www/html/arkiweb
 
 <Directory "/usr/lib64/arkiweb">
        AllowOverride None
        Options +ExecCGI
 
        Order allow,deny
        Allow from all
 
        # ARKIWEB_CONFIG is mandatory!
        SetEnv ARKIWEB_CONFIG /rmap/arkimet/arkiweb.config
        
 
        Require all granted
 
        # Authentication (optional)
        #
        # Basic authentication example:
        # SetEnv ARKIWEB_RESTRICT REMOTE_USER
        # AuthType Basic
        # AuthUserFile /etc/arkiweb.passwords
        # require valid-user
 </Directory>
 
 Alias /arkiwebjs/ /usr/share/arkiweb/public/
 <Directory "/usr/share/arkiweb/public">
           #Require all granted
           AllowOverride None
 
           Order allow,deny
           Allow from all
 
           Require all granted
 
 </Directory>

::
   
   mkdir /var/www/html/arkiweb/
   cp /usr/share/doc/arkiweb/html/example/index.html /var/www/html/arkiweb/index.html

/rmap/arkimet/arkiweb.config
::
   
 [arpav]
 bounding = POLYGON ((12.3693200000000001 44.9166299999999978, 11.3025699999999993 45.0306599999999975, 11.0090299999999992 45.2172600000000031, 10.8328900000000008 45.3717499999999987, 10.7659300000000009 45.5176999999999978, 11.8763699999999996 46.4992599999999996, 12.4241600000000005 46.6514799999999994, 12.7082800000000002 46.5699699999999979, 13.0772700000000004 45.6406399999999977, 12.3693200000000001 44.9166299999999978))
 filter = product: BUFR:t=arpav
 index = reftime, area, product, origin, proddef
 name = arpav
 path = http://arkiserver:8090/dataset/arpav
 postprocess = json, bufr, bufr-filter
 replace = yes
 server = http://arkiserver:8090
 step = daily
 type = remote
 unique = reftime, area, product, origin, proddef
 
 [meteonetwork]
 bounding = POLYGON ((12.0994399999999995 43.7931499999999971, 9.8880599999999994 44.5129299999999972, 9.4983599999999999 44.6443500000000029, 9.4705399999999997 44.6982100000000031, 9.4444999999999997 44.9392799999999966, 9.4909800000000004 45.0587200000000010, 11.8647899999999993 46.5125900000000030, 12.1300000000000008 46.5499999999999972, 12.9021600000000003 45.6111099999999965, 12.7495600000000007 43.9628200000000007, 12.6686999999999994 43.8718500000000020, 12.6577099999999998 43.8649699999999996, 12.0994399999999995 43.7931499999999971))
 filter = product: BUFR:t=mnw
 index = reftime, area, product, origin, proddef
 name = meteonetwork
 path = http://arkiserver:8090/dataset/meteonetwork
 postprocess = json, bufr, bufr-filter
 replace = yes
 server = http://arkiserver:8090
 step = daily
 type = remote
 unique = reftime, area, product, origin, proddef
 
 [opendata-er]
 bounding = POLYGON ((1.1372199999999999 4.4391400000000001, 8.4514200000000006 44.2992399999999975, 9.2314900000000009 44.8656700000000015, 9.5297699999999992 45.0566800000000001, 9.7055399999999992 45.0605199999999968, 11.8957999999999995 44.9680000000000035, 12.1221499999999995 44.9429000000000016, 12.2213899999999995 44.8950600000000009, 12.7393999999999998 43.9584699999999984, 1.1372199999999999 4.4391400000000001))
 filter = product: BUFR:t=spdsra or BUFR:t=locali or BUFR:t=agrmet or BUFR:t=profe or BUFR:t=simnpr or BUFR:t=simnbo or BUFR:t=rer or BUFR:t=simc or BUFR:t=urbane or BUFR:t=arpae or BUFR:t=boa or BUFR:t=cer or BUFR:t=provpc or BUFR:t=syrep or BUFR:t=umsuol
 index = reftime, area, product, origin, proddef
 name = opendata-er
 path = http://arkiserver:8090/dataset/opendata-er
 postprocess = json, bufr, bufr-filter
 replace = yes
 server = http://arkiserver:8090
 step = daily
 type = remote
 unique = reftime, area, product, origin, proddef
 
 [opendata-aq-er]
 bounding = POLYGON ((1.1372199999999999 4.4391400000000001, 8.4514200000000006 44.2992399999999975, 9.2314900000000009 44.8656700000000015, 9.5297699999999992 45.0566800000000001, 9.7055399999999992 45.0605199999999968, 11.8957999999999995 44.9680000000000035, 12.1221499999999995 44.9429000000000016, 12.2213899999999995 44.8950600000000009, 12.7393999999999998 43.9584699999999984, 1.1372199999999999 4.4391400000000001))
 filter = product: BUFR:t=arpae-aq
 index = reftime, area, product, origin, proddef
 name = opendata-aq-er
 path = http://arkiserver:8090/dataset/opendata-er
 postprocess = json, bufr, bufr-filter
 replace = yes
 server = http://arkiserver:8090
 step = daily
 type = remote
 unique = reftime, area, product, origin, proddef
 
 [luftdaten]
 bounding = POLYGON ((1.1372199999999999 4.4391400000000001, 8.4514200000000006 44.2992399999999975, 9.2314900000000009 44.8656700000000015, 9.5297699999999992 45.0566800000000001, 9.7055399999999992 45.0605199999999968, 11.8957999999999995 44.9680000000000035, 12.1221499999999995 44.9429000000000016, 12.2213899999999995 44.8950600000000009, 12.7393999999999998 43.9584699999999984, 1.1372199999999999 4.4391400000000001))
 filter = product: BUFR:t=luftdaten
 index = reftime, area, product, origin, proddef
 name = luftdaten
 path = http://arkiserver:8090/dataset/luftdaten
 postprocess = json, bufr, bufr-filter
 replace = yes
 server = http://arkiserver:8090
 step = daily
 type = remote
 unique = reftime, area, product, origin, proddef
 
 [report_fixed]
 bounding = POLYGON ((11.2500000000000000 44.3458499999999987, 11.2939500000000006 44.5252300000000005, 11.5576200000000000 44.8620999999999981, 11.6186500000000006 44.8371899999999997, 11.6233599999999999 44.6534600000000026, 11.6230100000000007 44.6530500000000004, 11.2500000000000000 44.3458499999999987))
 filter = product: BUFR:t=rmap or BUFR:t=fixed
 index = reftime, area, product, origin, proddef
 name = report_fixed
 path = http://arkiserver:8090/dataset/report_fixed
 postprocess = json, bufr, bufr-filter
 replace = yes
 server = http://arkiserver:8090
 step = daily
 type = remote
 unique = reftime, area, product, origin, proddef
 
 [report_mobile]
 filter = product: BUFR:t=rmap or BUFR:t=mobile
 index = reftime, area, product, origin, proddef
 name = report_mobile
 path = http://arkiserver:8090/dataset/report_mobile
 postprocess = json, bufr, bufr-filter
 replace = yes
 server = http://arkiserver:8090
 step = daily
 type = remote
 unique = reftime, area, product, origin, proddef
 
 
 [sample_fixed]
 bounding = POLYGON ((0.0000000000000000 0.0000000000000000, 9.1569500000000001 45.4436499999999981, 9.1570599999999995 45.4440700000000035, 11.6006800000000005 46.3956500000000034,   11.6742399999999993 46.4202500000000029, 11.6745000000000001 46.4202900000000014, 11.6747899999999998 46.4201400000000035, 12.4200400000000002 44.1349099999999979, 12.4458099999999998  43.9353399999999965, 12.5000000000000000 41.8999999999999986, 0.0000000000000000 0.0000000000000000))
 filter = product: BUFR:t=rmap or BUFR:t=fixed or BUFR:t=arpae
 index = reftime, area, product, origin, proddef
 name = sample_fixed
 path = http://arkiserver:8090/dataset/sample_fixed
 postprocess = json, bufr, bufr-filter
 replace = yes
 server = http://arkiserver:8090
 step = daily
 type = remote
 unique = reftime, area, product, origin, proddef
 
 [sample_mobile]
 filter = product: BUFR:t=rmap or BUFR:t=mobile or BUFR:t=arpae
 index = reftime, area, product, origin, proddef
 name = sample_mobile
 path = http://arkiserver:8090/dataset/sample_mobile
 postprocess = json, bufr, bufr-filter
 replace = yes
 server = http://arkiserver:8090
 step = daily
 type = remote
 unique = reftime, area, product, origin, proddef
