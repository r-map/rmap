
.. |GITHUBURL| replace:: https://raw.githubusercontent.com/r-map/rmap/master/server

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
  dnf copr enable simc/cosudo
  dnf install python3-django-dynamic-map-borinud
  dnf install mosquitto mosquitto-auth-plug
  dnf install arkimet
  useradd rmap
  
/etc/selinux/config::

  SELINUX=disabled

`/etc/tmpfiles.d/rmap.conf <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/tmpfiles.d/rmap.conf>`_

::

  mkdir /rmap
  chmod go+rx /rmap

`/etc/sysconfig/crond <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/sysconfig/crond>`_


::

   mkdir /var/log/rmap
   chown -R rmap:rmap /var/log/rmap

postgresql
----------
::

   dnf module disable postgresql:10
   dnf module enable postgresql:12
   dnf install postgresql-server postgresql-contrib
   dnf installpython3-psycopg2

::

   postgresql-setup --initdb --unit postgresql


`/var/lib/pgsql/data/pg_hba.conf <https://raw.githubusercontent.com/r-map/rmap/master/server/var/lib/pgsql/data/pg_hba.conf>`_

`/var/lib/pgsql/data/postgresql.conf <https://raw.githubusercontent.com/r-map/rmap/master/server/var/lib/pgsql/data/postgresql.conf>`_

::

   mkdir /etc/systemd/system/postgresql.service.d/

`/etc/systemd/system/postgresql.service.d/rmap.conf <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/systemd/system/postgresql.service.d/rmap.conf>`_
::

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


`/etc/rmap/rmap-site.cfg <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/rmap/rmap-site.cfg>`_

`/etc/rmap/dashboard.conf <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/rmap/dashboard.conf>`_

`/etc/rmap/graphTemplates.conf <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/rmap/graphTemplates.conf>`_

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
   
   mkdir /root/tmp/global_static
   rmapctrl --collectstatic
   rmdir /root/tmp/global_static

   yum install python3-mod_wsgi

   useradd -r rmap
   mkdir /home/rmap
   chown rmap:rmap /home/rmap
   mkdir /rmap/cache
   chown rmap:rmap /rmap/cache

   
`/etc/httpd/conf.modules.d/00-mpm.conf <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/httpd/conf.modules.d/00-mpm.conf>`_

`/etc/httpd/conf.d/rmap.conf <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/httpd/conf.d/rmap.conf>`_

`/etc/httpd/conf.d/rmap.inc <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/httpd/conf.d/rmap.inc>`_

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


`/etc/sysconfig/arkimet <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/sysconfig/arkimet>`_

`/etc/arkimet/scan/bufr_generic_mobile_rmap.py <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/arkimet/scan/bufr_generic_mobile_rmap.py>`_


Replicate structure in:

`/rmap/arkimet  <https://github.com/r-map/rmap/tree/master/server/rmap/arkimet>`_

::

 systemctl daemon-reload
 chkconfig arkimet on
 service arkimet start


Mosquitto
---------

::
   
   mkdir /etc/mosquitto/conf.d
   mkdir /rmap/mosquitto
   chown mosquitto:mosquitto /rmap/mosquitto

   
`/etc/mosquitto/conf.d/rmap.conf <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/mosquitto/conf.d/rmap.conf>`_

`/etc/mosquitto/aclfile <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/mosquitto/aclfile>`_

remove everythings and add in /etc/mosquitto/mosquitto.conf
::
   
   include_dir /etc/mosquitto/conf.d
   pid_file /var/run/mosquitto.pid

::
   
   touch /etc/mosquitto/pwfile
   chkconfig mosquitto on
   service mosquitto start

if the package use systemV create:

`/etc/monit.d/mosquitto <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/monit.d/mosquitto>`_


Rabbitmq
--------

::
   
   curl -s https://packagecloud.io/install/repositories/rabbitmq/rabbitmq-server/script.rpm.sh |bash
   curl -s https://packagecloud.io/install/repositories/rabbitmq/erlang/script.rpm.sh | sudo bash

   dnf install rabbitmq-server

`/etc/rabbitmq/enabled_plugins <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/rabbitmq/enabled_plugins>`_

`/etc/rabbitmq/rabbitmq-env.conf <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/rabbitmq/rabbitmq-env.conf>`_

`/etc/rabbitmq/rabbitmq.config <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/rabbitmq/rabbitmq.config>`_

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

`/etc/monitrc <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/monitrc>`_

`/etc/monit.d/rmap <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/monit.d/rmap>`_

::
   
 chkconfig monit on
 service monit start

Cron
----

`/etc/cron.d/arpae_aq_ckan <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/cron.d/arpae_aq_ckan>`_

`/etc/cron.d/dballe2arkimet <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/cron.d/dballe2arkimet>`_

`/etc/cron.d/luftdaten <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/cron.d/luftdaten>`_

`/etc/cron.d/makeexplorer <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/cron.d/makeexplorer>`_


Sincronizzazione DB da un server
--------------------------------

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
