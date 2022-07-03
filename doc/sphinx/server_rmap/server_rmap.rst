
.. |GITHUBURL| replace:: https://raw.githubusercontent.com/r-map/rmap/master/server

Server RMAP 
=====================================


Analisi funzionalità RMAP
-------------------------

Introduzione
............

Le funzionalità offerte dal server RMAP
(`https://rmap.cc <https://rmap.cc/>`__) sono implementate usando i
seguenti software e servizi (sono elencati solo quelli significativi):

-  Broker AMQP (RabbitMQ) per la distribuzione di dati in tempo reale
   attraverso un sistema di code, sia da stazione a server che da server
   a server.
-  Broker MQTT (Mosquitto) per l’invio di dati da stazioni al server.
-  Arkimet per l’archiviazione dei dati storici
-  DB-All.e per l’archiviazione dei dati recenti
-  Una singola applicazione web (Django web framework) che gestisce

   -  Sistema di autenticazione
   -  Visualizzazione dei dati
   -  Registrazione utenti
   -  Configurazione di metadati e firmware delle stazioni
   -  Inserimento manuale dati e immagini georeferenziati
   -  Visualizzazione personalizzata per RainBO

|image0|

Data ingestion
..............
Il data ingestion riguarda principalmente l’accoglimento di dati da
stazioni via MQTT e AMQP. I dati che arrivano via MQTT sono poi
inoltrati al broker AMQP, che è il nucleo della movimentazione dei dati
all’interno del sistema. I client che devono inviare dati via MQTT o
AMQP devono passare da un sistema di autenticazione e autorizzazione,
sostanzialmente per garantire che i dati siano inviati solo da utenti
autenticati e che non vadano a sovrapporsi a dati altrui. Entrambi i
broker interrogano il sistema di autenticazione\ **,** un servizio web
che implementa gli endpoint richiesti da RabbitMQ e Mosquitto. Il
servizio di autorizzazione su AMQP (cioè cosa può pubblicare un utente
autenticato) è invece delegato al demone *identvalidationd*, che prende
i dati dalla coda di ingresso dell’utente e passa alla coda di ingestion
solo i dati per cui il campo ident è uguale al nome utente. I dati che
passano quest’ultimo controllo sono poi inviati:

-  Ad altre code AMQP, ad esempio per forniture esterne o per altri
   processamenti. Su due di queste code sono inoltrati i dati per il
   SIMC (archiviati in Arkimet in formato VM2), una per le stazioni
   delle reti *claster* e *rmap* e l’altra per la rete *profe*.
-  Al database DB-All.e che contiene i dati recenti.

Ci sono inoltre alcuni moduli per l’accoglimento di dati da sorgenti che
non usano AMQP o non usano il formato BUFR (e.g. Luftdaten).

|image1|

Configurazione delle stazioni
.............................

Questa funzionalità permette l’aggiornamento della configurazione e del
firmware delle stazioni STIMA. L’aggiornamento può essere fatto via HTTP
o AMQP, previa autenticazione presso il corrispondente servizio di
autenticazione.
|image2|

Registrazione utenti
....................

Questa funzionalità permette di registrare gli utenti attraverso un
classico processo di iscrizione: l’utente compila una form e riceve una
email per la conferma dell’avvenuta registrazione. Ovviamente, c’è un
dialogo con il servizio di autenticazione.
|image3|

Visualizzazione dei dati
........................

Questo servizio permette di visualizzare i dati archiviati (DB-All.e e
Arkimet) sia su mappa che su grafico. Non c’è servizio di autenticazione
e autorizzazione perchè si presuppone che tutti i dati siano pubblici.
Questo sistema è probabilmente necessario per i dati della rete
amatoriale e quindi è necessario che il servizio corrispondente di data
ingestion sia collegato all’importatore dei dati su DB-All.e e Arkimet.
|image4|

Cosudo
......

Cosudo permette di analizzare i dati osservati per identificare
anomalie, confrontando i dati da stazione con dati radar, satellite e
previsti. Inoltre, permette all’operatore di invalidare dei dati: tali
invalidazioni sono poi inviate ai sistemi del SIMC per applicarle
sull’archivio.

È una applicazione che non necessita di accesso dall’esterno e deve
avere a disposizione i dati dall’archivio del SIMC.

Il sistema di autenticazione è necessario poiché, essendo ospitato sul
server rmap.cc, deve essere reso privato.

Non è ancora operativo e mancano i flussi di alimentazione dei dati.
|image5|

Inserimento manuale dei dati
............................

Questa funzionalità permette l’inserimento manuale, da parte di
operatori, di dati e immagini via HTTP e AMQP. I dati attualmente sono
le osservazioni della neve e del tempo (quest’ultimo all’interno del
progetto RainBO). Si appoggia al sistema di autenticazione.
|image6|

RainBO
......

È l’interfaccia per un progetto, che permette la visualizzazione con
delle viste personalizzate per i seguenti servizi:

-  Visualizzazione dei dati
-  Inserimento manuale dei dati
-  Registrazione utenti

Interfaccia web
...............
L’accesso da browser al sistema per alcune funzionalità, quali

-  Registrazione utente
-  Configurazione manuale delle stazioni
-  Visualizzazione dati
-  Inserimento manuale di dati

Sono offerte da un sistema monolitico, in cui tutti i vari pezzi sono
interconnessi. È possibile separarli, ma è richiesto un intervento non
banale sul frontend che può essere eseguito solo a valle
dell’organizzazione dei vari pezzi di RMAP su diversi host.

Se una funzionalità usa varie app Django e deve quindi “assemblare”
varie interfacce insieme, allora è necessario fare un repository per la
singola funzionalità che dipende dalle app Django richieste.

.. |image0| image:: images/Pictures/100000000000070A0000053E6107640098E107D3.png
   :width: 16.51cm
   :height: 12.312cm
.. |image1| image:: images/Pictures/10000000000006F3000006BFD6FD52DC1D86FE41.png
   :width: 16.51cm
   :height: 16.016cm
.. |image2| image:: images/Pictures/100002010000080000000507B85AB66754A5EF29.png
   :width: 16.51cm
   :height: 10.372cm
.. |image3| image:: images/Pictures/10000000000005B80000043555ABB73FC42635F9.png
   :width: 16.51cm
   :height: 12.136cm
.. |image4| image:: images/Pictures/1000020100000800000003D765E8704309FFEAAE.png
   :width: 16.51cm
   :height: 7.938cm
.. |image5| image:: images/Pictures/1000020100000800000007ABE34B7E3A4D8E30E3.png
   :width: 16.51cm
   :height: 15.84cm
.. |image6| image:: images/Pictures/1000020100000800000004695F2A049FAE8C7BE3.png
   :width: 16.51cm
   :height: 9.102cm



Installazione server completo basato su  Rocky Linux 8
------------------------------------------------------

Installazione sistema operativo
...............................

Installare Rocky Linux 8.

Aggiunta repository e installazione pacchetti
::

  dnf -y install epel-release
  dnf install yum-plugin-copr
  dnf copr enable simc/stable
  dnf copr enable pat1/rmap
  dnf config-manager --set-enabled powertools
  dnf copr enable simc/cosudo
  dnf install python3-rmap
  python3-django-cors-headers
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
..........
::

   dnf module disable postgresql:10
   dnf module enable postgresql:12
   dnf install postgresql-server postgresql-contrib python3-psycopg2

::

   mkdir /etc/systemd/system/postgresql.service.d/

`/etc/systemd/system/postgresql.service.d/rmap.conf <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/systemd/system/postgresql.service.d/rmap.conf>`_
::

   mkdir /rmap/pgsql/
   chown postgres:postgres /rmap/pgsql/
   postgresql-setup --initdb --unit postgresql


`/rmap/pgsql/data/pg_hba.conf <https://raw.githubusercontent.com/r-map/rmap/master/server/rmap/pgsql/data/pg_hba.conf>`_

`/rmap/pgsql/data/postgresql.conf <https://raw.githubusercontent.com/r-map/rmap/master/server/rmap/pgsql/data/postgresql.conf>`_

::
   
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



::
   
   dbadb wipe --dsn="postgresql://rmap:<password>@localhost/report_fixed"
   dbadb wipe --dsn="postgresql://rmap:<password>@localhost/report_mobile"
   dbadb wipe --dsn="postgresql://rmap:<password>@localhost/sample_mobile"
   dbadb wipe --dsn="postgresql://rmap:<password>@localhost/sample_fixed"

   
apache
......

Collect static files from django apps:
::
   
   mkdir /root/global_static
   rmapctrl --collectstatic
   rmdir /root/global_static

   dnf install python3-mod_wsgi
   dnf install mod_security
   
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
.......

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
.........

::
   
   mkdir /etc/mosquitto/conf.d
   mkdir /rmap/mosquitto
   chown mosquitto:mosquitto /rmap/mosquitto

   
`/etc/mosquitto/conf.d/rmap.conf <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/mosquitto/conf.d/rmap.conf>`_

`/etc/mosquitto/aclfile <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/mosquitto/aclfile>`_

remove everythings and add in /etc/mosquitto/mosquitto.conf
::
   
   include_dir /etc/mosquitto/conf.d
   pid_file /var/run/mosquitto/mosquitto.pid

::
   
   touch /etc/mosquitto/pwfile
   chkconfig mosquitto on
   service mosquitto start

if the package use systemV create:

`/etc/monit.d/mosquitto <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/monit.d/mosquitto>`_


Rabbitmq
........

::
   
   curl -s https://packagecloud.io/install/repositories/rabbitmq/rabbitmq-server/script.rpm.sh |bash
   curl -s https://packagecloud.io/install/repositories/rabbitmq/erlang/script.rpm.sh | sudo bash

   dnf install rabbitmq-server

`/etc/rabbitmq/enabled_plugins <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/rabbitmq/enabled_plugins>`_

`/etc/rabbitmq/rabbitmq-env.conf <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/rabbitmq/rabbitmq-env.conf>`_

`/etc/rabbitmq/rabbitmq.config <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/rabbitmq/rabbitmq.config>`_

::

   mkdir -p /rmap/rabbitmq/mnesia/
   chown -R rabbitmq:rabbitmq /rmap/rabbitmq
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
.....

::
   
   yum install monit

`/etc/monitrc <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/monitrc>`_

`/etc/monit.d/rmap <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/monit.d/rmap>`_

::
   
 chkconfig monit on
 service monit start

Cron
....

::

   mkdir /rmap/dballe
   chown -R rmap:rmap /rmap/dballe


`/etc/cron.d/arpae_aq_ckan <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/cron.d/arpae_aq_ckan>`_

`/etc/cron.d/dballe2arkimet <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/cron.d/dballe2arkimet>`_

`/etc/cron.d/luftdaten <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/cron.d/luftdaten>`_

`/etc/cron.d/makeexplorer <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/cron.d/makeexplorer>`_


Sincronizzazione DB da un server
................................

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
.......
AL MOMENTO NON DISPONIBILE !

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




Installazione server solo funzionalità DATA INGESTION basato su  Rocky Linux 8
------------------------------------------------------------------------------

Installazione sistema operativo
...............................

Installare Rocky Linux 8.

Aggiunta repository e installazione pacchetti
::

  dnf -y install epel-release
  dnf install yum-plugin-copr
  dnf copr enable simc/stable
  dnf copr enable pat1/rmap
  dnf config-manager --set-enabled powertools
  dnf install python3-rmap-core
  dnf install mosquitto mosquitto-auth-plug
  useradd rmap
  
modificare il file /etc/selinux/config::

  SELINUX=disabled

scaricare il file `/etc/tmpfiles.d/rmap.conf <https://raw.githubusercontent.com/r-map/rmap/master/server-data-ingestion/etc/tmpfiles.d/rmap.conf>`_

::

  mkdir /rmap
  chmod go+rx /rmap


::

   mkdir /var/log/rmap
   chown -R rmap:rmap /var/log/rmap

scaricare il file `/etc/rmap/rmap-site.cfg <https://raw.githubusercontent.com/r-map/rmap/master/server-data-ingestion/etc/rmap/rmap-site.cfg>`_

cambiare la password dell'utente amministratore.

Mosquitto
.........

::
   
   mkdir /etc/mosquitto/conf.d
   mkdir /rmap/mosquitto
   chown mosquitto:mosquitto /rmap/mosquitto

   
scaricare il file `/etc/mosquitto/conf.d/rmap.conf <https://raw.githubusercontent.com/r-map/rmap/master/server-data-ingestion/etc/mosquitto/conf.d/rmap.conf>`_


cancellare tutto il contenuto del file /etc/mosquitto/mosquitto.conf e sostituirlo con le seguenti righe:
::
   
   include_dir /etc/mosquitto/conf.d
   pid_file /var/run/mosquitto/mosquitto.pid

::
   
   systemctl enable mosquitto
   systemctl start mosquitto


Rabbitmq
........

::
   
   curl -s https://packagecloud.io/install/repositories/rabbitmq/rabbitmq-server/script.rpm.sh |bash
   curl -s https://packagecloud.io/install/repositories/rabbitmq/erlang/script.rpm.sh | sudo bash

   dnf install rabbitmq-server

scaricare il file `/etc/rabbitmq/enabled_plugins <https://raw.githubusercontent.com/r-map/rmap/master/server-data-ingestion/etc/rabbitmq/enabled_plugins>`_

scaricare il file `/etc/rabbitmq/rabbitmq-env.conf <https://raw.githubusercontent.com/r-map/rmap/master/server-data-ingestion/etc/rabbitmq/rabbitmq-env.conf>`_

scaricare il file `/etc/rabbitmq/rabbitmq.config <https://raw.githubusercontent.com/r-map/rmap/master/server-data-ingestion/etc/rabbitmq/rabbitmq.config>`_


Installare il certificato ssl/tls per il dominio del server in:

::
   
   /etc/arpaecert/arpae_it.pem

e impostare gli opportuni privilegi di lettura/scrittura.

::

   mkdir -p /rmap/rabbitmq/mnesia/
   chown -R rabbitmq:rabbitmq /rmap/rabbitmq
   systemctl enable rabbitmq-server
   systemctl start rabbitmq-server

effettuare il login all'interfaccia di management web: <http://server-fqdn:15672/>
con user "guest" e password "guest", quindi utilizzare la funzione "import definition" per caricare exchange, queue e users importando il seguente file:

`rabbit_server_data_ingestion.json <https://raw.githubusercontent.com/r-map/rmap/master/rabbitmq/rabbit_server_data_ingestion.json>`_

dalla stessa interfaccia di management web impostare le password per tutti gli utenti, rimuovere l'utente "guest" e fare login con uno dei nuovi utenti definiti.



Monit
.....

::
   
   yum install monit

scaricare il file `/etc/monitrc <https://raw.githubusercontent.com/r-map/rmap/master/server-data-ingestion/etc/monitrc>`_

scaricare il file `/etc/monit.d/rmap <https://raw.githubusercontent.com/r-map/rmap/master/server-data-ingestion/etc/monit.d/rmap>`_

::

 chmod 0700 /etc/monitrc /etc/monit.d/rmap
 chown root:root /etc/monitrc /etc/monit.d/rmap
 systemctl enable monit
 systemctl start monit


Sincronizzazione file statici per autenticazione e autorizzazione da un server RMAP completo
............................................................................................

Server di origine
~~~~~~~~~~~~~~~~~

::

   rmapctrl --exportsha > file.pwd
   rmapctrl --exportmqttsha >> file.pwd
   rmapctrl --exportmqttacl > aclfile
   rmapctrl --exportmqttpsk > file.psk
   
eventualmente rimuovere le prime righe di messaggistica del logging


Server di destinazione
~~~~~~~~~~~~~~~~~~~~~~

Trasferire i file in /etc/mosquitto/ sul nostro server per la data ingestion.
Imparire il comando:

::

   /bin/systemctl reload mosquitto.service



   
