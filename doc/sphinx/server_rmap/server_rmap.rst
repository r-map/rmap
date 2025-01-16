
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

.. _dataingestion-reference:
     
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

.. _configurazionestazioni-reference:

Configurazione delle stazioni
.............................

Questa funzionalità permette l’aggiornamento della configurazione e del
firmware delle stazioni STIMA. L’aggiornamento può essere fatto via HTTP
o AMQP, previa autenticazione presso il corrispondente servizio di
autenticazione.
|image2|

.. _registrazioneutenti-reference:

Registrazione utenti
....................

Questa funzionalità permette di registrare gli utenti attraverso un
classico processo di iscrizione: l’utente compila una form e riceve una
email per la conferma dell’avvenuta registrazione. Ovviamente, c’è un
dialogo con il servizio di autenticazione.
|image3|

.. _visualizzazionedati-reference:

Visualizzazione dei dati
........................

Questo servizio permette di visualizzare i dati archiviati (DB-All.e e
Arkimet) sia su mappa che su grafico. Non c’è servizio di autenticazione
e autorizzazione perchè si presuppone che tutti i dati siano pubblici.
Questo sistema è probabilmente necessario per i dati della rete
amatoriale e quindi è necessario che il servizio corrispondente di data
ingestion sia collegato all’importatore dei dati su DB-All.e e Arkimet.
|image4|

.. _cosudo-reference:

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

.. _inserimentomanuale-reference:

Inserimento manuale dei dati
............................

Questa funzionalità permette l’inserimento manuale, da parte di
operatori, di dati e immagini via HTTP e AMQP. I dati attualmente sono
le osservazioni della neve e del tempo (quest’ultimo all’interno del
progetto RainBO). Si appoggia al sistema di autenticazione.
|image6|

.. _raindo-reference:

RainBO
......

È l’interfaccia per un progetto, che permette la visualizzazione con
delle viste personalizzate per i seguenti servizi:

-  Visualizzazione dei dati
-  Inserimento manuale dei dati
-  Registrazione utenti

.. _interfacciaweb-reference:

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

.. |image0| image:: flussodati.png
   :width: 16.51cm
   :height: 12.312cm
.. |image1| image:: dataingestion.png
   :width: 16.51cm
   :height: 16.016cm
.. |image2| image:: stationmanager.png
   :width: 16.51cm
   :height: 10.372cm
.. |image3| image:: registrazione_utenti.png
   :width: 16.51cm
   :height: 12.136cm
.. |image4| image:: borinud_showdata.png
   :width: 16.51cm
   :height: 7.938cm
.. |image5| image:: cosudo.png
   :width: 16.51cm
   :height: 15.84cm
.. |image6| image:: insertdata_geoimage.png
   :width: 16.51cm
   :height: 9.102cm


Porte utilizzate
................

+---------+-------------------------------+-----------+----------------------------------------+------------+---------------------+--------------+
| porta   | protocollo                    | server    | servizio                               |RMAP server | RMAP data ingestion | RMAP backend |
+=========+===============================+===========+========================================+============+=====================+==============+
| 80      | HTTP                          | apache    | download conf e firmware (Stima V3)    |     X      |                     |      X       |
+---------+-------------------------------+-----------+----------------------------------------+------------+---------------------+--------------+
| 442     | HTTPS TLS con pre shared Key  | stunnel   | download conf e firmware (Stima v4)    |     X      |                     |      X       |
+---------+-------------------------------+-----------+----------------------------------------+------------+---------------------+--------------+
| 443     | HTTPS  (SSL/TLS)              | apache    | gestione backend e visualizzazione dati|     X      |                     |      X       |
+---------+-------------------------------+-----------+----------------------------------------+------------+---------------------+--------------+
| 5925    | HTTP                          | monit     | monitoraggio daemoni RMAP              |     X      |                     |      X       |
+---------+-------------------------------+-----------+----------------------------------------+------------+---------------------+--------------+
| 1883    | MQTT                          | mosquitto | pubblicazioni dati stazione (Stima V3) |     X      |         X           |              |
+---------+-------------------------------+-----------+----------------------------------------+------------+---------------------+--------------+
| 5671    | AMQPS  (SSL/TLS)              | rabbit-mq | pubblicazione e distribuzione dati     |     X      |         X           |              |
+---------+-------------------------------+-----------+----------------------------------------+------------+---------------------+--------------+
| 5672    | AMQP                          | rabbit-mq | pubblicazione e distribuzione dati     |     X      |         X           |              |
+---------+-------------------------------+-----------+----------------------------------------+------------+---------------------+--------------+
| 8883    | MQTTS TLS con pre shared Key  | mosquitto | pubblicazione stazioni (Stima V4)      |     X      |         X           |              |
+---------+-------------------------------+-----------+----------------------------------------+------------+---------------------+--------------+
| 8884    | MQTTS (SSL/TLS) WEBSOCKET     | mosquitto | monitoraggio MQTT da web               |     X      |         X           |              |
+---------+-------------------------------+-----------+----------------------------------------+------------+---------------------+--------------+
| 15672   | HTTP                          | rabbit-mq | Management Plugin                      |     X      |         X           |      X       |
+---------+-------------------------------+-----------+----------------------------------------+------------+---------------------+--------------+



Documentazione implementazione
------------------------------

Il funzionamento del back-end e del front-end è basato su due broker,
una suite di applicazioni Django, alcuni tools e da una suite di daemon.

I dati seguono flussi e archivi differenti secondo questa classificazione:

+-------------------------+----------------+------------------+
|                         | stazione fissa |  Stazione mobile |
+-------------------------+----------------+------------------+
| livello dati sample     | sample_fixed   | sample_mobile    |
+-------------------------+----------------+------------------+
| livello dei dati report | report_fixed   | report_mobile    |
+-------------------------+----------------+------------------+

Le quattro categorie corrispondono quindi a differenti root path MQTT,
differenti exchange e queue AMQP, differenti daemon.
Corrispondono anche a quattro differenti DataBase di DB.all.e per i dati recenti e a quattro
macro dataset Arkimet poi eventualmente sottosezionati.
Ecco un esempio di struttura dataset di Arkimet:

::

 ├── report_fixed
 │   ├── agrmet
 │   ├── boa
 │   ├── claster
 │   ├── dpcn-basilicata
 │   ├── dpcn-bolzanoboze
 │   ├── dpcn-calabria
 │   ├── dpcn-campania
 │   ├── dpcn-lazio
 │   ├── dpcn-liguria
 │   ├── dpcn-lombardia
 │   ├── dpcn-marche
 │   ├── dpcn-molise
 │   ├── dpcn-piemonte
 │   ├── dpcn-puglia
 │   ├── dpcn-sardegna
 │   ├── dpcn-sicilia
 │   ├── dpcn-umbria
 │   ├── dpcn-veneto
 │   ├── fixed
 │   ├── locali
 │   ├── mnw
 │   ├── report_fixed_duplicates
 │   ├── report_fixed_error
 │   ├── simnbo
 │   ├── simnpr
 │   ├── spdsra
 │   └── urbane
 ├── report_mobile
 │   ├── report_mobile
 │   ├── report_mobile_duplicates
 │   └── report_mobile_error
 ├── sample_fixed
 │   ├── sample_fixed
 │   │   └── sample_fixed
 │   ├── sample_fixed_duplicates
 │   └── sample_fixed_error
 └── sample_mobile
     ├── sample_mobile
     ├── sample_mobile_duplicates
     └── sample_mobile_error




Configurazione stazioni
.......................

Le configurazioni risiedono sul server. Possono essere scaricate e uplodate sul server.
Le configurazioni devono essere trasferite alle stazioni e devono sempre essere allineate.
Le stazioni Stima si possono configurare con i seguenti formati e trasporti:

+-------------------------+---------------+------------+------------+-------+-----------------+
|                         | trasporto     | trasporto  | tasporto   | SD    | download/upload |
|                         | Seriale       | HTTP       | http PSK   | card  | server          |
+-------------------------+---------------+------------+------------+-------+-----------------+
| JSON-RPC                |       #       |            |            |       |                 |
+-------------------------+---------------+------------+------------+-------+-----------------+
| notification JSON-RPC   |               |     #      |     #      |       |                 |
+-------------------------+---------------+------------+------------+-------+-----------------+
| dump DB in json         |               |     #      |            |       |       #         |
+-------------------------+---------------+------------+------------+-------+-----------------+
| file binario            |               |            |            |   #   |                 |
+-------------------------+---------------+------------+------------+-------+-----------------+

queste configurazioni sono utilizzate differentemente a seconda del
modello di stazione:

+-----------+----------+-----------------------+-----------------+--------------+
|           | JSON-RPC | notification JSON-RPC | dump DB in json | file binario |
+-----------+----------+-----------------------+-----------------+--------------+
| StimaWifi |          |                       |       #         |              |
+-----------+----------+-----------------------+-----------------+--------------+
| Stima V3  |    #     |                       |                 |      #       |
+-----------+----------+-----------------------+-----------------+--------------+
| Stima V4  |    #     |          #            |                 |              |
+-----------+----------+-----------------------+-----------------+--------------+

  
Specifiche per immagini georeferenziate
.......................................

Le immagini devono essere in formato jpeg e usare exif per i metadati.

Vengono utilizzati i seguenti metadati:

* Latitude/Longitude (GPSIFD.GPSLatitude/GPSIFD.GPSLongitude)
* reftime            (ImageIFD.DateTime)
* imagedescription   (ImageIFD.ImageDescription)
* usercomment        (ExifIFD.UserComment)

  
Specifiche per file di configurazione stazione
..............................................

Il file è un dump in json del DB usato da Django relativo solo alle
tabelle di interesse per la configurazione. Questo un esempio:

::
   
    [
    {
      "model": "stations.stationmetadata",
      "fields": {
	"name": "stimawifi",
	"active": true,
	"slug": "stimawifi",
	"user": [
	  "paperino"
	],
	"ident": "",
	"lat": 44.48858,
	"lon": 11.37099,
	"network": "fixed",
	"mqttrootpath": "sample",
	"mqttmaintpath": "maint",
	"category": "good",
	"type": "stimav2"
      }
    },
    {
      "model": "stations.board",
      "fields": {
	"name": "default",
	"active": true,
	"slug": "default",
	"category": "template",
	"type": 0,
	"sn": null,
	"stationmetadata": [
	  "stimawifi",
	  [
	    "paperino"
	  ]
	]
      }
    },
    {
      "model": "stations.sensor",
      "fields": {
	"active": true,
	"name": "Dust",
	"driver": "I2C",
	"type": [
	  "SPS"
	],
	"i2cbus": null,
	"address": 105,
	"node": null,
	"timerange": "254,0,0",
	"level": "103,2000,-,-",
	"board": [
	  "default",
	  [
	    "stimawifi",
	    [
	      "paperino"
	    ]
	  ]
	]
      }
    },
    {
      "model": "stations.sensor",
      "fields": {
	"active": true,
	"name": "Temperature_Humidity",
	"driver": "I2C",
	"type": [
	  "SHT"
	],
	"i2cbus": null,
	"address": 68,
	"node": null,
	"timerange": "254,0,0",
	"level": "103,2000,-,-",
	"board": [
	  "default",
	  [
	    "stimawifi",
	    [
	      "paperino"
	    ]
	  ]
	]
      }
    },
    {
      "model": "stations.sensor",
      "fields": {
	"active": true,
	"name": "CO2",
	"driver": "I2C",
	"type": [
	  "SCD"
	],
	"i2cbus": null,
	"address": 97,
	"node": null,
	"timerange": "254,0,0",
	"level": "103,2000,-,-",
	"board": [
	  "default",
	  [
	    "stimawifi",
	    [
	      "paperino"
	    ]
	  ]
	]
      }
    },
    {
      "model": "stations.transportmqtt",
      "fields": {
	"active": true,
	"mqttsampletime": 30,
	"mqttserver": "test.rmap.it",
	"mqttuser": "paperino",
	"mqttpassword": "wFCeWf4ZqY",
	"mqttpskkey": "648A67186FBF033E8A508282EEA74767",
	"board": [
	  "default",
	  [
	    "stimawifi",
	    [
	      "paperino"
	    ]
	  ]
	]
      }
    },
    {
      "model": "stations.transporttcpip",
      "fields": {
	"active": true,
	"name": "stima",
	"ntpserver": "it.pool.ntp.org",
	"gsmapn": "ibox.tim.it",
	"pppnumber": "*99#",
	"board": [
	  "default",
	  [
	    "stimawifi",
	    [
	      "paperino"
	    ]
	  ]
	]
      }
    },
    {
      "model": "stations.stationconstantdata",
      "fields": {
	"active": true,
	"btable": "B01019",
	"value": "stimawifi",
	"stationmetadata": [
	  "stimawifi",
	  [
	    "paperino"
	  ]
	]
      }
    },
    {
      "model": "stations.stationconstantdata",
      "fields": {
	"active": true,
	"btable": "B07030",
	"value": "400",
	"stationmetadata": [
	  "stimawifi",
	  [
	    "paperino"
	  ]
	]
      }
    }
    ]

Esempio di notification JSON-RPC
................................

::

    {"jsonrpc": "2.0", "method": "configure", "params": {"reset": true}}
    {"jsonrpc":"2.0","result":{},"id":0}
    {"jsonrpc": "2.0", "method": "configure", "params": {"sd": {"B01019": "stimawifi"}}}
    {"jsonrpc":"2.0","result":{},"id":0}
    {"jsonrpc": "2.0", "method": "configure", "params": {"sd": {"B07030": "400"}}}
    {"jsonrpc":"2.0","result":{},"id":0}
    {"jsonrpc": "2.0", "method": "configure", "params": {"mqttrootpath": "1/sample/paperino//1137099,4448858/fixed/"}}
    {"jsonrpc":"2.0","result":{},"id":0}
    {"jsonrpc": "2.0", "method": "configure", "params": {"mqttmaintpath": "1/maint/paperino//1137099,4448858/fixed/"}}
    {"jsonrpc":"2.0","result":{},"id":0}
    {"jsonrpc": "2.0", "method": "configure", "params": {"mqttrpcpath": "1/rpc/paperino//1137099,4448858/fixed/"}}
    {"jsonrpc":"2.0","result":{},"id":0}
    {"jsonrpc": "2.0", "method": "configure", "params": {"mqttsampletime": 30, "mqttserver": "test.rmap.it"}}
    {"jsonrpc":"2.0","result":{},"id":0}
    {"jsonrpc": "2.0", "method": "configure", "params": {"mqttuser": "paperino", "mqttpassword": "77CeWf4ZqY"}}
    {"jsonrpc":"2.0","result":{},"id":0}
    {"jsonrpc": "2.0", "method": "configure", "params": {"mqttpskkey": "0x328467156FBF033E8A508282EEA74767"}}
    {"jsonrpc":"2.0","result":{},"id":0}
    {"jsonrpc": "2.0", "method": "configure", "params": {"stationslug": "stimawifi", "boardslug": "default"}}
    {"jsonrpc":"2.0","result":{},"id":0}
    {"jsonrpc": "2.0", "method": "configure", "params": {"ntpserver": "it.pool.ntp.org"}}
    {"jsonrpc":"2.0","result":{},"id":0}
    {"jsonrpc": "2.0", "method": "configure", "params": {"gsmapn": "ibox.tim.it"}}
    {"jsonrpc":"2.0","result":{},"id":0}
    {"jsonrpc": "2.0", "method": "configure", "params": {"driver": "I2C", "type": "SPS", "node": null, "address": 105, "mqttpath": "254,0,0/103,2000,-,-/"}}
    {"jsonrpc":"2.0","result":{},"id":0}
    {"jsonrpc": "2.0", "method": "configure", "params": {"driver": "I2C", "type": "SHT", "node": null, "address": 68, "mqttpath": "254,0,0/103,2000,-,-/"}}
    {"jsonrpc":"2.0","result":{},"id":0}
    {"jsonrpc": "2.0", "method": "configure", "params": {"driver": "I2C", "type": "SCD", "node": null, "address": 97, "mqttpath": "254,0,0/103,2000,-,-/"}}
    {"jsonrpc":"2.0","result":{},"id":0}
    {"jsonrpc": "2.0", "method": "configure", "params": {"save": true}}
    {"jsonrpc":"2.0","result":{},"id":0}
    {"jsonrpc": "2.0", "method": "reboot"}
    {"jsonrpc":"2.0","result":{},"id":0}



Broker MQTT
...........

Le stazioni Stima comunicano i dati al broker differentemente a
seconda della versione:

-  **stazioni StimaWiFi**: comunicano in WiFi in chiaro, tramite
   protocollo MQTT
-  **stazioni Stima V3**: comunicano in GSM in chiaro, tramite
   protocollo MQTT
-  **stazioni Stima V4**: comunicano in TLS con PSK (Pre-Shared Key),
   tramite protocollo MQTTS (PSK)

Il broker MQTT (mosquitto) authentica e autorizza tramite un apposito
plugin.  Utilizza l'hash delle password utente, le chiavi PSK, le ACL;
queste vengono ottenute da file o da un server di autenticazione
tramite API http.  La generazione del file se necessario piò essere
fatta con il tool rmapctrl.

Mosquitto comunica tramite un terzo protocollo, MQTT con SSL/TLS su
Websocket utilizzato per funzionalità di monitoraggio delle stazioni
meteo; tramite javascript nel web browser.  Gli operatori che
gestiscono le varie stazioni meteo possono attivare il monitoraggio su
websocket per verificare i messaggi pubblicati dalla stazione sul
broker evitando la necessità di avere un apposito client MQTT.

Ogni stazione può pubblicare solamente sul topic mqtt per cui è stata
configurata e quindi autorizzata, in base a utente, rete, latitudine e
longitudine. Ogni autenticazione e autorizzazione corrisponde a una
richiesta http, ma con apposita cache del plugin di mosquitto.

Il plugin di mosquitto è una versione appositamente modificata:
https://github.com/r-map/mosquitto-auth-plug

Se configurato a tale scopo il broker alle richieste di
autenticazione/autorizzazione, nel caso in cui non sia disponibile il
servizio http ricade su un file statico. Quando la configurazione
hardware lo indichi come opportuno (separazione fisica della data
ingestion) il file statico, utilizzato come sistema di autenticazione
di backup, deve essere sincronizzato tramite procedura schedulata dal
server di backend verso il server di data ingestion in modo da
consentire comunque l’autenticazione delle stazioni anche in caso di
failure del backend (il sistema non consentirà per esempio l’aggiunta
o lo spostamento di stazioni ma non si bloccherà la pubblicazione dei
dati).

I dati, una volta autenticate le stazioni sul backend e pubblicati
tramite il broker MQTT sono disponibili ai subscriber, principalmente
ai daemoni per la conversione di formato e l'invio come messaggi AMQP.


Tools
~~~~~

Generazione dei file di autenticazione/autorizzazione

Per mantenere allineato il processo di autenticazione sul broker MQTT
e consentire l’autenticazione delle stazioni, anche in caso di failure
del server di backend, è utilizzabile il tool **rmapctrl** che deve
generere i seguenti file:

-  file.pwd
-  aclfile
-  file.psk

Una volta copiati i file in **/etc/mosquitto/** sul server di data
ingestion, è necessario effettuare un reload del servizio
**mosquitto.service** in modo che i file aggiornati vengano recepiti
correttamente; questo normalmente viene fatto solo in caso di modifica
degli stessi file.


.. _MQTT_URL-reference:

Daemon
~~~~~~

report2observationd

Il demone (**report2observationd**) effettua una trasformazione dei
messaggi che arrivano inizialmente compressi (vedi :ref:`estensioni
<rmaprfc_estensioni-reference>` protocollo RMAP over MQTT), li
decomprime e li pubblica nuovamente, per cui arrivano sul broker MQTT
in forma decompressa.


URL
~~~

Se le URLs configurate tornano HTTP status code == 2xx, allora la
authentication / authorization ha successo. Se lo status code == 4xx,
authentication / authorization fallisce. Per uno status code == 5xx o
server Unreachable, la richiesta HTTP sarà ritentata fino al raggiungimento di
http_retry_count. Se tutti i tentativi falliscono e nessun altro backend ha successo,
allora torna un errore e il client è disconnesso.

Configurazioni Mosquitto per il back-end http:

differenti modi di autenticare l'utente:

* auth_opt_http_getuser_uri /auth/auth
* auth_opt_http_getuser_uri /auth/auth_sha
* auth_opt_http_getuser_uri /auth/sha
* auth_opt_http_getuser_uri /auth/pskkey

verifica autorizzazioni:

* auth_opt_http_superuser_uri /auth/superuser
* auth_opt_http_aclcheck_uri /auth/acl


Broker AMQP
...........

Il brocker AMQP (rabbitmq) svolge queste funzioni principali:

* fornisce AMQP come protocollo per lo scambio dati
* gestisce lo scambio interno dei messaggi sostanzialmente come
  sistema di buffering persistente
* gestisce accoglienza e autorizzazione relativamente a:
  - dati in formato json (secondo specifiche RMAP)
  - immagini georeferenziate
  - configurazioni stazioni
  mettendo poi a disposizione questi dati ad appositi archiviatori
* mette a disposizione un sistema per lo scambio dati tra fornitori e
  consumatori di dati (grosse moli di dati)


.. |fanoutexchange| image:: ./Pictures/10000000000002BA000001CF10D58739AA10CEF3.png
   :width: 11.18cm
   :height: 7.334cm
.. |overview| image:: ./Pictures/10000001000007600000037539E6A3FD1D30171B.png
   :width: 14.986cm
   :height: 7.022cm
.. |exchanges| image:: ./Pictures/10000001000002F6000001694EE7486645E83327.png
   :width: 10.589cm
   :height: 5.313cm
.. |exchange| image:: ./Pictures/100000010000034B00000253E938A43FC98A0D1A.png
   :width: 10.292cm
   :height: 7.417cm
.. |queue| image:: ./Pictures/10000001000006600000033B3F9646A30A7792D1.png
   :width: 11.566cm
   :height: 6.343cm

Viene utilizzato un solo tipo di exchange: fanout
	    
|fanoutexchange|

I producer inviano dati agli **Exchange** e i consumer li prelevano
dalle **Queue**.
La gestione e il monitoraggio del broker avviene tramite l’accesso
all’interfaccia web di RabbitMQ.
All’accesso viene visualizzata la dashboard con statistiche varie,
messaggi accodati, accodati, connessioni, ecc…

|overview|

Il numero di messaggi in stato *Ready* può essere un buon indicatore
in caso di problemi, se il numero cresce in maniera importante e i
messaggi non vengono scodati.

Nella sezione **Exchanges** è possibile
gestire quelli esistenti oppure configurarne di nuovi.

|exchanges|

Cliccando sul nome del Exchange si viene rimandati al dettaglio della
sua configurazione e alla definizione dei **Bindings**, ovvero dove
vengono indirizzati i messaggi pubblicati: la **Queue** di
destinazione

Exchanges → Queue

ad esempio:
rmap.mqtt.bufr.report_fixed → arpaer.out.bufr.report_fixed&validated

|exchange|

Nella sezione **Queues & Streams** vengono gestite e definite le code
di destinazione dei messaggi collezionati dal **Exchange**.

Qui posso controllare il numero di messaggi disponibili nella coda,
non ancora confermati, ecc…  Solitamente se tutto funziona
correttamente la coda dovrebbe risultare sempre vuota, in quanto il
**consumer** ha recuperato correttamente tutti i messaggi pubblicati.

|queue|

In questa sezione è possibile spostare i messaggi da una coda ad
un’altra (**Move messages**), cancellare definitivamente la coda e
tutti i suoi messaggi (**Delete Queue**) oppure cancellare tutti i
messaggi all'interno della coda (**Purge Messages**).

Nella sezione **Admins** vengono definiti gli utenti necessari solo al
broker AMQP e non utilizzati dal resto del sistema di
authenticazione. Questi saranno quindi solo fornitori e consumatori di
dati esterni.  Qui vengono definite le autorizzazioni dell'utente
secondo una precisa definizione di regexp. A esempio per l’utente
**paperino** sono le seguenti:

-  virtual host utilizzato è il default, ovvero “ / ”
-  non ha permessi di configurazione (campo vuoto in fase di
   definizione)
-  ha permessi di lettura e scrittura in tutto ciò (exchange e queue)
   che inizia con il suo nome utente (**^paperino\..\***), in cui potrà
   leggere e pubblicare dati

Definizione nuovo fornitore o consumatore di dati
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

La definizione di un nuovo fornitore di dati o consumatore di dati è
ottenuta da:

-  creazione utente con privilegi write e read con regular expression
   **^utente\..\***
-  creazione queue con eventuale *Message TTL* (se vuoto è indefinito,
   altrimenti imposta il valore massimo in millisecondi di permanenza dei
   messaggi nella queue, una volta superato quel valore i messaggi
   vengono rimossi)
-  creazione exchange con binding alla queue creata al punto precedente

Nomenclatura convenzionale
~~~~~~~~~~~~~~~~~~~~~~~~~~

La sintassi utilizzata nella definizione di exchange e code è
autoesplicativa e basata sul punto come separatore tra campi

Fornitore:
-  **exchange** → *utente.in.formato.descrizione*

Consumatore:
-  **queue** → *utente.out.formato.descrizione*

*utente* = nome utente definito in fase di creazione
*in/out* = direzione operazione (*in* per fornitore, *out* per consumatore)
*formato* = tipologia di output (es. json, xml, bufr)
*descrizione* = label descrittiva dei dati in transito

Ogni utente ha accesso in lettura e scrittura a exchange e code che
iniziano con il **Name** del proprio utente.

Nelle coda è definito il parametro **Message TTL** con un valore ad
esempio di 604800000 (millisecondi) pari a 7 giorni, per cui tutti i
messaggi pubblicati al suo interno rimarranno disponibili nella coda
per 7 giorni, i messaggi più vecchi rimossi automaticamente.

Sistema di autenticazione e autorizzazione di RMAP
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Il broker utilizza inoltre il sistema di autenticazione e
autorizzazione di RMAP per autenticare la ricezione di messaggi:

* fotografie: formato jpeg
* configurazione stazioni: formato json

Questo metodo di uplodare foto e configurazioni eè per il momento da
considerarsi obsoleto.

.. _AMQP_URL-reference:
  
URL
~~~

Autentica utente o amministratore:

* user_path     "http://localhost/auth/user"

Sempre autorizzato tutti vhost:

* vhost_path    "http://localhost/auth/vhost"

Autorizzate risorse che iniziano con un punto "." e "configuration" o "photo":

* resource_path "http://localhost/auth/resource"

Le successive verifiche di autenticazione vengono fatte dai daemon di
importazione dati che verificano che l'utente utilizzato per
l'autenticazione AMQP siano corrispondenti ai metadati dei dati
inviati.

Per fotografie e configurazioni questa attualmente è una metodologia
disattivata a vantaggio della metodologia HTTP.
  
Daemon
~~~~~~

* amqp2amqp_identvalidationd : valida gli ident per la pubblicazione dati da coda AMQP a coda AMQP
* amqp2amqp_jsonline2bufrd : conversione da formato jsonline a bufr da coda AMQP a coda AMQP
* amqp2arkimetd : archiviazione diretta da coda AMQP a arkimet (obsoleto)
* amqp2dballed : archiviazione dati da coda AMQP a DBall.e; utilizza
  il lock per l'accesso in scrittura a DBall.e e le opzioni
  - -d DATALEVEL, --datalevel=DATALEVEL
  sample or report
  - -s STATIONTYPE, --stationtype=STATIONTYPE
  fixed or mobile
  quali delle 4 istanze eseguire
* amqp2djangod : importazioni delle configurazioni stazione da coda AMQP a framework Django
* amqp2geoimaged : importazione delle immagini georeferenziate da coda AMQP
* amqp2mqttd : pubblica i dati da una coda AMQP al broker MQTT


Log
...

Tutti i log relativi ai daemon coinvolti in RMAP si trovano nella directory **/var/log/rmap/**.


Descrizione applicazioni Django
...............................

Il funzionamento del back-end e del front-end è garantito da una suite
di applicazioni Django


stations app
~~~~~~~~~~~~

Descrizione
'''''''''''
Questa applicazione sovranintende alla gestione delle stazioni
gestendone la crezione, configurazione e aggiornamento dei metadati,
mantenedo aggiornati lo stato di funzionamento e autodiagnostica, le
versioni del firmware utilizzate, e le informazioni di autenticazione
e autorizzazione.

Espone l'interfaccia di amministrazione di Django tramite la quale è
possibile fare operazioni particolari di amministrazione non ancora
gestite da apposite funzioni da web.

Le stazioni si possono creare e modificare in vari modi:

* tramite interfaccia web e template: ci sono un certo numero di
  configurazioni predefinite per stazioni standard che possono essere
  utilizzate modificando solo alcuni parametri specifici quali le
  coordinate, nme etc.
* tramite linea di comando, tramite il tool rmap-config con una serie
  di passaggi è possibile creare da un template o definendo tutti i
  dettagli
* tramite l'interfaccia admin di Django con una sequenza di operazioni
  è possibile creare e modificare una stazione; consigliato solo per
  modifiche

I template non sono altro che stazioni definite come tutte le altre
con l'unica caratteristica nella Category:template ("used to generate
new station") oltre a essere disattivate per non entrare negli elenchi
delle stazioni "reali".

All'installazione vengono precaricate alcuni template tramite le fixture di Django; ad esempio: https://github.com/r-map/rmap/blob/master/python/rmap/stations/fixtures/template_stations_02.json

Le definizioni stazioni possono essere trasferite tra differenti
istanze del server (server remoto e DB Django locale).


DataBase
''''''''

.. image:: ../../../python/doc/model_stations.png
	   :width: 100% 

table UserProfile
#################
relazione one to one con l'oggetto User del Django authentication system
utilizzato per estendere i metadati di un utente.

table StationMetadata
#####################
Metadati principali di una stazione.
Identificativi che compongono la chieve univoca per il DataBase dati:

* ident: identificativo per stazioni mobili
* lat/lon: coordinate
* network: rete di stazioni omogenee a cui appartiene

Identificativi univoci per il DataBase dati:

* user: utente
* slug: identificativo sintetico della stazione

table StationConstantdata
#########################
Coppia btable,value che compongono i dati costanti di stazione.

table Board
###########
Board (moduli) che fanno parte di una stazione. Una Board prevede uno o più transporti di comunicazione, 
uno o più sensori, uno stato di funzionamento e uno stato del firmware.

table Transport*
################
Definiscono i paramtri di comunicazione per ogni trasporto.

table Sensor
############
Sensori connessi a una board.
Identificativi che compongono la chiave univoca per il DataBase dati:

* level: livello rappresentativo delle misure
* timernage: elaborazione statistica

Definisce il driver utilizzato per il bus che bisogna utilizzare per
comunicare con il sensore insieme ad alcuni parametri per
identificarlo e per comunicare con lui.

table SensorType
################

Definisce un tipo di sensore: il livello dei dati (sample o report) e
il tipo che determina il driver da utilizzare per comunicare con il
sensore. Ogni sensore può riportare più misure definite dalla tabelle btable.

table Btable
############

Elenco dei Bcode della tabella B definita in modo univoco dalle
specifiche RMAP (tabella B WMO).  Fa parte delle chiavi univoche del
DB dati.  Sono abbinati descrizione e unità di misura prima e dopo un
eventuale applicazione dei fattori di scala.

table BoardMaintStatus
######################

Definisce lo stato di una Board così come comunicati dalla stazione (Stima V4).
Prevede valori booleani e numerici.

table BoardFirmwareMetadata
###########################

Mantiene uno stato aggiornato dei firmware delle board presenti in stazione.
Il campo mac viene utilizzato:

* valorizzato al primo tentativo di aggiornamento
* verificato i successivi aggiornamenti
* i restanti campi sono aggiornati solo se la verifica ha successo

questo serve come semplice metodo di autenticazione della richiesta
http di aggiornamento del firmware delle board. Se l'hardware viene
cambiato a una stazione preesistente il mac deve essere resettato.

table StationMaintStatus
########################

Questa tabella viene aggiornata tramite la messaggistica a standard
RMAP nel root topic "maint" dei dati pubblicati MQTT. Mantiene la
versione del firmware di stazione (versione globale, solitamente della
board master), l'utimo aggiornamento della connessione e lo stato
della connessione.

table StationImage
##################
Associa ad ogni stazione delle fotografie con commenti e metadati
utili per dettagliare la sua collocazione e caratteristiche.


URL
'''
Sono riportate solo le versioni delle URL con tutti i parametri, anche omittibili

* **'^$'** home page
* **'^wizard/$'**         wizard inizio configurazione stazione BASE (solo su stazione BASE)
* **'^wizard2/$'**        wizard seconda pagina configurazione stazione BASE (solo su stazione BASE) 
* **'^wizard_done/$'**    wizard fine configurazione stazione BASE (solo su stazione BASE)
* **'^wizard_error/$'**   wizard errore configurazione stazione BASE (solo su stazione BASE)
* **'^admin/doc/'**       documentatione admin di Django
* **^admin/'**            admin di Django
  
* **'^auth/user'**        autenticazione/autorizzazione rabbit-mq; guarda :ref:`amqp <AMQP_URL-reference>`
* **'^auth/vhost'**       autenticazione/autorizzazione rabbit-mq; guarda :ref:`amqp <AMQP_URL-reference>`
* **'^auth/resource'**    autenticazione/autorizzazione rabbit-mq; guarda :ref:`amqp <AMQP_URL-reference>`

* **'^auth/auth$'**       autenticazione/autorizzazione mosqtto; guarda :ref:`mqtt <MQTT_URL-reference>`
* **'^auth/auth_sha$'**   autenticazione/autorizzazione mosquitto; guarda :ref:`mqtt <MQTT_URL-reference>`
* **'^auth/sha$'**        autenticazione/autorizzazione mosquitto; guarda :ref:`mqtt <MQTT_URL-reference>`
* **'^auth/pskkey$'**     autenticazione/autorizzazione mosquitto; guarda :ref:`mqtt <MQTT_URL-reference>`
* **'^auth/superuser$'**  autenticazione/autorizzazione mosquitto; guarda :ref:`mqtt <MQTT_URL-reference>`
* **'^auth/acl$'**        autenticazione/autorizzazione mosquitto; guarda :ref:`mqtt <MQTT_URL-reference>`

* **'^accounts/profile/(?P<mystation_slug>[-\w]+)/(?P<stationimage_id>[-\w]+)/$'**
  pagina di amministrazione dell'utente/della stazione utente
* **'^robots.txt$'**      robots.txt per i motori di ricerca
  
* **'^stations/(?P<user>[-_\w]+)/(?P<slug>[-_\w]+)/$'** elenco
  stazioni con metadati riassuntivi
* **'^stationstatus/(?P<user>[-_\w]+)/(?P<slug>[-_\w]+)/$'** elenco
  stazioni con stato e autodiagnostica
* **'^stationmqttmonitor/(?P<user>[-_\w]+)/(?P<slug>[-_\w]+)/$'**
  monitoraggio web della comunicazione MQTT

* **'^stationsupload/json/$'** upload della configurazione stazione se
  l'utente è autenticato e se la stazione uplodata corrisponde
  all'utente stesso (è il metodo attualmente utilizzato in sostituzione di AMQP)
  
* **'^stationconfig/(?P<user>[-_\w]+)/(?P<station_slug>[-_\w]+)$'** serie
  di RPC per configurare una stazione in modalità "notification"

* **'^stations/(?P<user>[-_\w]+)/(?P<station_slug>[-_\w]+)/json/$'**
  configurazione stazione in formato json; se l'utente è autenticato
  il json è completo di password, nel caso contrario sono assenti; il
  timerange viene completato con P2=mqttsampletime; da utilizzare per
  la configurazione stazione
    
* **'^stations/(?P<user>[-_\w]+)/(?P<station_slug>[-_\w]+)/json_dump/$'**
  dump della configurazione stazione in formato json; se l'utente è autenticato
  il json è completo di password, nel caso contrario sono assenti; il
  timerange non viene completato lasciando P2 per essere completato successivamente.
  
* **'^stations/(?P<user>[-_\w]+)/(?P<station_slug>[-_\w]+)/configv3/$'**
  restituisce un file binario da utilizzare su SD card per la
  configurazione stazioni Stima V3

* **'^stations/(?P<user>[-_\w]+)/(?P<station_slug>[-_\w]+)/(?P<board_slug>[-_\w]*)/json/$'**
  configurazione di una board di una stazione in formato json; se
  l'utente è autenticato il json è completo di password, nel caso
  contrario sono assenti; il timerange viene completato con
  P2=mqttsampletime; da utilizzare per la configurazione stazione

* **'^delstation/(?P<user>[-_\w]+)/(?P<slug>[-_\w]+)/$'** cancellazione di una stazione
  
* **'^stationlocaldata/(?P<user>[-_\w]+)/(?P<slug>[-_\w]+)/$'** visualizza
  i dati misurati dalla stazione StimaWiFi se risulta collegata nella
  stessa rete locale con avahi/bonjour
  
* **'^stationsonmap/(?P<user>[-_\w]+)/(?P<slug>[-_\w]+)/$'** visualizza le
  stazioni su mappa

Tools
'''''

**rmap-configure**

Esempio di configurazione di una stazione Stima V4

::

   rmap-configure --wizard --station_slug teststation --stationname Stima V4 test station --username myuser --password password --server test.rmap.cc --lat 45.54321 --lon 12.54321 --height 234 --mqttrootpath report --mqttmaintpath maint --network test
   rmap-configure --addboard --boardtype 11 --serialactivate --mqttactivate --tcpipactivate --canactivate --tcpipntpserver it.pool.ntp.org --tcpipname stima --tcpipgsmapn internet.wind --tcpippppnumber *99***1# --station_slug teststation --board_slug masterv4 --username myuser --server test.rmap.cc --mqttuser myuser --mqttpassword h23jhjhjh --mqttpskkey 1BF19DC005FFCE9277B4CFC696410426 --mqttsamplerate 900 --cannodeid 100 --cansubject node.master --cansubjectid 100 --cansamplerate 60
   rmap-configure --addsensors_by_template none --station_slug teststation --board_slug masterv4 --username myuser
   rmap-configure --addboard --boardtype 21 --serialnumber 0xABCDEF1 --serialactivate --canactivate --station_slug teststation --board_slug module_th --username myuser --cannodeid 60 --cansubject node.th --cansubjectid 60 --cansamplerate 60
   rmap-configure --addsensors_by_template stima4_report_th --station_slug teststation --board_slug module_th --username myuser
   rmap-configure --addboard --boardtype 20 --serialnumber 0xABCDEF2 --serialactivate --canactivate --station_slug teststation --board_slug module_rain --username myuser --cannodeid 61 --cansubject node.p --cansubjectid 61 --cansamplerate 60
   rmap-configure --addsensors_by_template stima4_report_p --station_slug teststation --board_slug module_rain --username myuser
   rmap-configure --addboard --boardtype 25 --serialnumber 0xABCDEF3 --serialactivate --canactivate --station_slug teststation --board_slug module_wind --username myuser --cannodeid 62 --cansubject node.wind --cansubjectid 62 --cansamplerate 60
   rmap-configure --addsensors_by_template stima4_report_w --station_slug teststation --board_slug module_wind --username myuser
   rmap-configure --addboard --boardtype 26 --serialnumber 0xABCDEF4 --serialactivate --canactivate --station_slug teststation --board_slug module_radiation --username myuser --cannodeid 63 --cansubject node.rad --cansubjectid 63 --cansamplerate 60
   rmap-configure --addsensors_by_template stima4_report_r --station_slug teststation --board_slug module_radiation --username myuser
   rmap-configure --addboard --boardtype 28 --serialnumber 0xABCDEF5 --serialactivate --canactivate --station_slug teststation --board_slug module_mppt --username myuser --cannodeid 64 --cansubject node.mppt --cansubjectid 64 --cansamplerate 60
   rmap-configure --addsensors_by_template stima4_report_m --station_slug teststation --board_slug module_mppt --username myuser
   rmap-configure --addboard --boardtype 29 --serialnumber 0xABCDEF6 --serialactivate --canactivate --station_slug teststation --board_slug module_svwc --username myuser --cannodeid 65 --cansubject node.svwc --cansubjectid 65 --cansamplerate 60
   rmap-configure --addsensors_by_template stima4_report_s --station_slug teststation --board_slug module_svwc --username myuser

   # configurazione tramite porta seriale
   rmap-configure --config_station --station_slug teststation --username myuser --transport serial --device /dev/ttyACM0 --stima_version 4

   # upload configurazione al server
   #rmap-configure --upload_to_server --station_slug teststation --username myuser --password password --server test.rmap.cc


registration app
~~~~~~~~~~~~~~~~

Descrizione
'''''''''''

Se la funzione è abilitata da configurazione questa app gestisce la
possibilità che gli utenti si registrino in modo sutonomo dovendo
fornire come minimi una email.

DataBase
''''''''

.. image:: ../../../python/doc/model_registration.png
	   
URL
'''
  
* **'^registrazione/register/$'**   pagina di registrazione utenti
* **'^registrazione/'**  radice di tutti i path necessari per la registrazione utenti

  * **'^registrazione/activate/complete/$'**
  * **'^registrazione/activate/resend/$'**
  * **'^registrazione/activate/(?P<activation_key>\w+)/$'**
  * **'^registrazione/register/complete/$'**
  * **'^registrazione/register/closed/$'**
  * **'^registrazione/register/$'**
  * **'^registrazione/login/$'**
  * **'^registrazione/logout/$'**
  * **'^registrazione/password/change/$'**
  * **'^registrazione/password/change/done/$'**
  * **'^registrazione/password/reset/$'**
  * **'^registrazione/password/reset/complete/$'**
  * **'^registrazione/password/reset/done/$'**
  * **'^registrazione/password/reset/confirm/(?P<uidb64>[0-9A-Za-z_\-]+)/(?P<token>.+)/$'**

  
network app
~~~~~~~~~~~

Descrizione
'''''''''''

Ogni stazione è associata a un network. In questa app viene
visualizzata la lista dei network e i metadati di ogni network.  Non
esiste un link automatico tra i network pubblicati dalle stazioni e i
network presenti in questa app.  Se lo stesso network è presente sia
nei dati che in questa app, i metadati presenti in questa app vengono
visualizzati quando richiesto.


DataBase
''''''''

.. image:: ../../../python/doc/model_network.png
	   
URL
'''

* **'^network/(?P<name>[-\w]+)/$'**    navigazione e visualizzazione network


http2mqtt app
~~~~~~~~~~~~~

Descrizione
'''''''''''

Questa app permette di pubblicare i dati tramite http.  Le specifiche
seguono le RMAP RFC per http con le specifiche a questo link
:ref:`http2mqtt <rmaprfc_http-reference>`


URL
'''
* **'^http2mqtt/$'**


  
firmware_updater e firmware_updater_stima app
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Descrizione
'''''''''''

**firmware_updater: Versione per StimaWifi**

Ogni firmware è caratterizzato da un nome e da una versione
il nome è a piacere, la versione è una data parsabile da questo modulo python:
https://dateutil.readthedocs.io/en/stable/parser.html

la richiesta può essere http o https, ma normalmente http.

Viene verificata la presenza di alcuni header nella richiesta http specifici per ESP8266/ESP32:

* HTTP_USER_AGENT deve essere uguale a ESP8266-http-Update/ESP32-http-Update
  Se user-agent divers torna 403
* HTTP_X_ESP8266_SKETCH_SIZE/HTTP_X_ESP32_SKETCH_SIZE deve essere
  impostato (ma non è utilizzato) altrimenti torna 403
* X_ESP8266_AP_MAC/HTTP_X_ESP32_AP_MAC deve essere
  impostato (ma non è utilizzato) altrimenti torna 403
* HTTP_X_ESP8266_FREE_SPACE/HTTP_X_ESP32_FREE_SPACE deve essere
  impostato (ma non è utilizzato) altrimenti torna 403
* HTTP_X_ESP8266_SKETCH_MD5/HTTP_X_ESP32_SKETCH_MD5deve essere
  impostato (ma non è utilizzato) altrimenti torna 403
* HTTP_X_ESP8266_CHIP_SIZE/HTTP_X_ESP32_CHIP_SIZEdeve essere
  impostato (ma non è utilizzato) altrimenti torna 403
* HTTP_X_ESP8266_SDK_VERSION/HTTP_X_ESP32_SDK_VERSIONdeve essere
  impostato (ma non è utilizzato) altrimenti torna 403
* X_ESP8266_STA_MAC/HTTP_X_ESP32_STA_MAC Essendo la richiesta
  d'aggiornamento del firmware non autenticata, la board dispone di un
  MAC che viene inviato nell'header X_ESP8266_STA_MAC il cui hash
  viene salvato la prima volta sul server e tutte le volte successive
  deve fare match per poter aggiornare le info sul server.

* X-ESP8266-VERSION: l'header VERSION è codificata in json ad esempio:
  X-ESP8266-VERSION: {"ver":"2022-11-01T12:00:00"}
  - ver : versione attuale del firmware nel formato datetime ISO
  Nello stesso json possono essere inviati anche:
  - user   (utente)
  - slug   (station slug)
  - bslug  (board slug)

  nel qual caso sul server vengono aggiornate le informazioni relative
  alla board con versione inviata alla board per l'aggiornamento e la
  data e ora dell'invio (eventualmente utile per effettuare un commit
  dell'aggiornamento).
 
risposta:

* Se il firmware non esiste torna 500
* Se la versione non è corretta torna 300
* Se non c'è un nuovo firmware torna 304
* Se l'md5 del firmware attualmente running è uguale a quello dell'ultimo firmware disponibile torna 304

altrimenti risponde con il firmware con header 'x-MD5' pari all'md5 del firmware che segue

Il tutto si riassume con questa richiesta tramite curl:

::

  curl --user-agent "ESP8266-http-Update" -H "X-ESP8266-STA-MAC:
  1234" -H "X-ESP8266-AP-MAC: 1234" -H "X-ESP8266-FREE-SPACE: 1234" -H
  "X-ESP8266-SKETCH-SIZE: 1234" -H "X-ESP8266-CHIP-SIZE: 1234" -H
  "X-ESP8266-SDK-VERSION: 1234" -H "X-ESP8266-SKETCH-MD5: 1234" -H
  'X-ESP8266-VERSION: {"ver":"2022-11-01T12:00:00"}'
  "http://test.rmap.cc/firmware/update/stima4/" -v

URL
'''

* '^firmware/$'
* '^firmware/update/(?P<name>[-_\w]+)/$'

**firmware_updater_stima: versione per Stima V4**

* HTTP_USER_AGENT deve essere uguale a STIMA4-http-Update
  Se user-agent divers torna 403
* HTTP_X_STIMA4_BOARD_MAC Essendo la richiesta d'aggiornamento del
  firmware non autenticata, la board dispone di un MAC che viene
  inviato nell'header HTTP_X_STIMA4_BOARD_MAC il cui hash viene salvato la
  prima volta sul server e tutte le volte successive deve fare match
  per poter aggiornare le info sul server.

* HTTP_X_STIMA4_VERSION l'header VERSION è codificata in json ad esempio:
  X_STIMA4_VERSION: {"version":4,"revision":5,"user":"userv4","slug":"stimav4","bslug":"stimacan1"}
  - version: versione attuale del firmware
  - revision: revisione attuale del firmware
  Nello stesso json possono essere inviati anche:
  - user:    utente
  - slug:    station slug
  - bslug:   board slug

  nel qual caso sul server vengono aggiornate le informazioni relative
  alla board con versione inviata alla board per l'aggiornamento e la
  data e ora dell'invio (eventualmente utile per effettuare un commit
  dell'aggiornamento).

risposta:

* Se il firmware non esiste torna 500
* Se la versione non è corretta torna 300
* Se non c'è un nuovo firmware torna 304

altrimenti risponde con il firmware con header 'x-MD5' pari all'md5 del firmware che segue

URL
'''

* '^firmware/stima/v4/$'
* '^firmware/stima/v4/update/(?P<name>[-_\w]+)/$'

DataBase
''''''''

.. image:: ../../../python/doc/model_firmware.png

Le due tabelle differiscono per la versionatura del firmware in un
caso utilizzando un campo date e nell'altro revisione e versione.

  
geoimage app
~~~~~~~~~~~~

Descrizione
'''''''''''

Tramite interfaccia web è possibile inserire immagini georeferenziate, visualizzare 
anteprime sulla mappa, navigare le immagini su un sito nel tempo e cancellarle.


DataBase
''''''''

.. image:: ../../../python/doc/model_geoimage.png

URL
'''

* '^geoimage/geoimagesonmap/(?P<ident>[-_\w]+)/$'
* '^geoimage/geoimagebyid/(?P<id>[-_\w]+)/$'
* '^geoimage/geoimagedelete/(?P<id>[-_\w]+)/$'
* '^geoimage/geoimagesbycoordinate/(?P<lon>[-_\w.]+)/(?P<lat>[-_\w.]+)/$'

  
rpc app
~~~~~~~

Descrizione
'''''''''''

Le stazioni possono eseguire Remote Procedure Call secondo le spscifiche;
https://www.jsonrpc.org/specification
e l'implementazione su MQTT in :ref:`rmap rfc <rmaprfc_rpc-reference>`.

Ogni RPC è caratterizzata da un id incrementato in seuqenza per ogni
stazione.  Una RPC è attiva quando è solo stata richiesta, ma non
ancora inviata tramite MQTT.

* date corrisponde  a quando è stata richiesta una RPC (stato sumbitted)
* datecmd corrisponde a quando l'RPC è stata inviata tramite MQTT (stato running)
* dateres corrisponde a quando è stata ricevuta una risposta (stato completed)

* method : metodo della RPC
* params : parametri della RPC
* result : risultato della RPC
* error : messaggio di errore

Una RPC assume quindi differenti stati:
* submitted : è stata richiesta una RPC
* running : l'RPC è stata inviata tramite MQTT
* completed : è stata ricevuta una risposta

Lo stato submitted può essere attivato tramite interfaccia web o tool
a linea di comando.

Da questo momento la gestione della RPC passa al daemon rpcd che
periodicamente richiede al DB le RPC da gestire (stato submitted) e le
invia alle stazioni tramite MQTT. Rimane poi sottoscritto al topic
MQTT per la risposta che una volta ricevuta viene inserita in DB.

I tempi di esecuzione delle RPC possono essere molto lunghi vista
caratteristica della connessione delle stazioni che non è permanente.

Se le stazioni non ricevono la richiesta di RPC o non inviano la
riposta lo stato dell'RPC rimane "running".

L'interfaccia WEB che permette l'attivazione di una RPC ha due modalità:
* modalità semplice che utilizza RPC predefinite:

  - reboot  : riavvio stazione
  - reboot e firmware update : riavvio con eventuale firmware update
    del firmware precedentemente scaricato  (solo Stima V4)
  - recovery last day of data : richiede reinvio dei dati delle ultime 24 ore
  - admin firmware download : richiede il download di un eventuale firmware più recente  (solo Stima V4)
  - admin configuration download : richiede il download della
    configurazione stazione eventualmente aggiornata  (solo Stima V4)

* modalità a formato libero dove vengono richiesti:

  - metodo
  - parametri

DataBase
''''''''

.. image:: ../../../python/doc/model_rpc.png

URL
'''

* '^rpcs/(?P<user>[-_\w]+)/(?P<slug>[-_\w]+)/$'
* '^rpc/(?P<user>[-_\w]+)/(?P<slug>[-_\w]+)/(?P<id>[-_\w]+)/$'
* '^rpc-submit-free/(?P<user>[-_\w]+)/(?P<slug>[-_\w]+)/$'
* '^rpc-submit/(?P<user>[-_\w]+)/(?P<slug>[-_\w]+)/$'


Daemon
''''''

* rpcd : legge dal DB Django le eventuali RPC da eseguire inviandole
  tramite MQTT alle stazioni; Riceve le risposte e le inserisce nel DB
  Django

Tools
'''''
  
* rmapctrl
  
  - --rpc_mqtt_admin_fdownload_v4 : invia richiesta RPC per un firmware download
    over MQTT per stazioni Stima V4
    - --username=USERNAME : work on station managed by this username
    - --station_slug=STATION_SLUG : work on station defined by this slug

  - --purge_rpc : remove all non active RPC (submitted, running and
    completed)
  - --purge_rpc_completed : remove non active RPC and completed


ticket app
~~~~~~~~~~

Descrizione
'''''''''''

L'app gestisce i ticket relativi alle stazioni: 
* è relativo a una stazione
* può essere attivo o no
* può essere assegnato a più utenti (chi deve risolvere il problema)
* più utenti possono essere abbonati (notifica delle modifiche)
* possono essere associate immagini
* possono essere associati allegati
* a ogni ticket si susseguono azioni per risolverlo e poi disattivarlo

L'app non è completa e i ticket si possono inserire e gestire solo dall'interfaccia admin di Django.
E' necessario aggiungere anche le notifiche tramite email

DataBase
''''''''

.. image:: ../../../python/doc/model_ticket.png

URL
'''

* '^tickets/(?P<user>[-_\w]+)/(?P<slug>[-_\w]+)/$'
* '^ticket/(?P<ticket>[-_\w]+)/$'
* '^ticket_image/(?P<image_id>[-_\w]+)/$'
* '^ticket_attachment/(?P<attachment_id>[-_\w]+)/$'
* '^tickets_assigned/(?P<user_ass>[-_\w]+)/$'
* '^tickets_subscribed/(?P<user_sub>[-_\w]+)/$'

	      
borinud app
~~~~~~~~~~~

Descrizione
'''''''''''

Borinud mette a disposizioni delle API WEB per il download dei dati.
Le fonti dei dati possono essere differenti e multiple: DBall.e e
Arkimet.  Oltre ai database utilizza gli explorer per mantenere
aggiornati e veloci i cataloghi dei dati presenti.

URL
'''

::
   
  basepattern = ('^borinud/api/v1/(?P<format>\w+)'
  '/(?P<ident>\w+|\*|-)'
  '/(?P<coords>(?P<lon>\-?\+?\d+|-),(?P<lat>\-?\+?\d+|-)|\*)'
  '/(?P<network>[-\w]+|\*)'
  '/(?P<trange>(?P<tr>\d+|-|\*),(?P<p1>\d+|-|\*),(?P<p2>\d+|-|\*)|\*)'
  '/(?P<level>(?P<lt1>\d+|-|\*),(?P<lv1>\d+|-|\*),(?P<lt2>\d+|-|\*),(?P<lv2>\d+|-|\*)|\*)'
  '/(?P<var>B\d{5}|\*)')

* '^borinud/$'
* basepattern + '/summaries/(?P<year>\d{4})/(?P<month>\d{2})/(?P<day>\d{2})$'
* basepattern + '/timeseries/(?P<year>\d{4})/(?P<month>\d{2})/(?P<day>\d{2})/(?P<hour>\d{2})$'
* basepattern + '/spatialseries/(?P<year>\d{4})/(?P<month>\d{2})/(?P<day>\d{2})/(?P<hour>\d{2})$'
* basepattern + '/stationdata$'
* basepattern + '/stations$'

Tools
'''''

**rmap-explorer**: viene eseguito in crontab per aggiornare gli
explorer di DBall.e in **/rmap/dballe**

**dballe2arkimet** : oltre a trasferire i dati da DBall.e a arkimet
aggiorna gli explorer di arkimet in **/rmap/arkimet**.  L'aggiornamento
degli explorer di arkimet avviene in modo incrementale ogni volta che
si aggiunge un blocco dati a Arkimet.  Se dovesse avvenire un
disallineamento o un intervento manuale sul DB è possibile ricreare
gli explorer di Arkimet tramite questo script:

::

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

  
graphite-dballe  app
~~~~~~~~~~~~~~~~~~~~

Descrizione
'''''''''''

Questa app è una versione embedded di `graphite <https://graphite.readthedocs.io/en/stable/index.html>`_
che utilizza borinud come `storage-backends <https://graphite.readthedocs.io/en/stable/storage-backends.html>`_

URL
'''

Consultare la `documentazione <https://graphite.readthedocs.io/en/stable/render_api.html>`_.


insertdata app
~~~~~~~~~~~~~~

Descrizione
'''''''''''

TODO

URL
'''

* '^insertdata/image$'
* '^insertdata/manualdata$'
* '^insertdata/newstation$'
* '^insertdata/stationmodify/(?P<slug>[-_\w]+)/(?P<bslug>[-_\w]+)/$'

showdata app
~~~~~~~~~~~~

Descrizione
'''''''''''

TODO

URL
'''
::
   
  basepattern = (
  '^showdata/(?P<ident>\w+|\*|-)'
  '/(?P<coords>(?P<lon>\-?\+?\d+|-),(?P<lat>\-?\+?\d+|-)|\*)'
  '/(?P<network>[-\w]+|\*)'
  '/(?P<trange>(?P<tr>\d+|-|\*),(?P<p1>\d+|-|\*),(?P<p2>\d+|-|\*)|\*)'
  '/(?P<level>(?P<lt1>\d+|-|\*),(?P<lv1>\d+|-|\*),(?P<lt2>\d+|-|\*),(?P<lv2>\d+|-|\*)|\*)'
  '/(?P<var>B\d{5}|\*)')

* '^showdata/$'
* basepattern + '/timeseries/(?P<year>\d{4})/(?P<month>\d{2})/(?P<day>\d{2})/(?P<hour>\d{2})$'
* basepattern + '/spatialseries/(?P<year>\d{4})/(?P<month>\d{2})/(?P<day>\d{2})$'
* basepattern + '/stationdata$'
* basepattern + '/stations$'

Daemon
......

Il daemon hanno i seguento comandi:
* run : esegue nel terminale
* start : esegue come daemon
* stop : termina l'esecuzione del daemon

le altre opzioni sono documentate con l'opzione --help.
Qui una sintesi di funzionalità e opzioni:

I daemon specifici per una app sono stati descritti precedentemente
nella apposizta sezione della app.

* mqtt2amqpd :
  
  - -d DATALEVEL, --datalevel=DATALEVEL :  sample or report
  - -s STATIONTYPE, --stationtype=STATIONTYPE :  fixed or mobile
  - -u --nouserasident  : do not use user as ident for fixed station too
    (default use ident)

  conversione dei dati da RMAP over MQTT a formato bufr e messaggio
  AMQP; sono necessarie 4 istanze per differenti datalevel e
  stationtype. ; l'opzione --nouserasident decide il tipo di
  conversione da effettuare: se lo user è utilizzato come ident i dati
  prodotti saranno utili per la visualizzazione dentro il server RMAP,
  ma formalmente scorretti; se l'opzione è attivata i dati prodotti
  saranno formalmente corretti e scambiali internazionalmente.  E'
  possibile attivare 8 istanze: 4 per l'archiviazione locale e 4 per
  la distribuzione oppure le prima 4 nella data ingestion e le altre 4
  nel backend.

* mqtt2stationmaintd : preleva i messaggi diagnostici pubblicati dalle
  stazioni in MQTT e li inserisce in archivio Django
* composereportd : effettua le elaborazioni necessarie per calcolare i
  report a partire dai sample; ad esempio esegue le medie ogni 15',
  calcola valori minimi e massimi, dopo aver calcolato le osservazioni
  da campionamenti. Legge e scrive da DBall.e sample_fixed a DBall.e
  report_fixed
    
* mqtt2dballed : conversione diretta da RMAP over MQTT a DBall.e (obsoleta)
* mqtt2graphited : conversione da RMAP over MQTT agli archivi specifici di graphite (obsoleta)
* stationd: daemon che effettua le misure e le elabora, le invia da una stazione di misura basata su Linux/Python (obsoleta)
* ttn2dballed : daemon che effettua la traslazione da dati pubblicati tramite LoraWan TTN a MQTT RMAP (obsoleta)



Tools
.....

I tools specifici per una app sono stati descritti precedentemente
nella apposizta sezione della app.

Altri tools disponibili sono:

* rmapgui : esegue la app interattiva tramite libreria Kivy per dispositivi touch
* rmapweb  : esegue un server http senza necessità di apache
* rmapctrl : esegue comandi inizializzazione e mantenimento del sistema RMAP
  - --syncdb : initialize rmap DB (default False)
  - --collectstatic :  Collect static files from multiple apps into a single
  path (default False)
  - --dumpdata          :  dump Data Base (default False)
  - --loaddata=LOADDATA :  restore Data Base (default none)
  
* rmap-explorer per generare gli explorer dei DB DBall.e e accelerare i summary
  - rmap-explorer --write --type report_fixed
  - rmap-explorer --write --type report_mobile
  - rmap-explorer --write --type sample_fixed
  - rmap-explorer --write --type sample_mobile
* dballe2arkimet : permette la migrazione dei dati ormai
  "stabilizzati" da DBall.e a Arkimet: utilizza il lock per l'accesso
  in scrittura a DBall.e

* rmap-manage : comandi per il sistema Django


Configurazione
..............

TODO
  
Operazioni per l'aggiornamento
..............................

TODO

Operazioni periodiche di manutenzione
.....................................

TODO


Struttura cartelle
..................

.. include:: ../../../README.directory
		   
		   
Installazione server completo basato su  Rocky Linux 8
------------------------------------------------------

Installazione sistema operativo
...............................

Installare Rocky Linux 8.

Aggiunta repository e installazione pacchetti
::

  dnf -y install epel-release
  dnf -y install yum-plugin-copr
  dnf -y copr enable simc/stable
  dnf -y copr enable pat1/rmap
  dnf -y config-manager --set-enabled powertools
  dnf -y copr enable simc/cosudo
  dnf -y install python3-rmap
  dnf -y install python3-django-dynamic-map-borinud
  dnf -y install mosquitto mosquitto-auth-plug
  dnf -y install arkimet dballe
  dnf -y install stunnel
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


Generate ssl auto signed certificate
::

   mkdir certs
   cd certs
   
   1 – Since we don’t have a certificate authority, we’ll create a local CA using SSL:
   openssl genrsa -des3 -out ca.key 2048
   enter a pass phrase with at least 4 characters. Remember it as it will be used several times.
   
   2 – Then we’ll create a signing request for our local CA
   openssl req -new -key ca.key -out ca.csr -sha256
   Enter the same passphrase chosen in the previous step. It’s safe to leave all other fields empty, for the sake of our setup’s simplicity.

   3 – Then we can create our CA root certificate, valid for 180 days:
   openssl x509 -req -in ca.csr -signkey ca.key -out ca-root.crt -days 180 -sha256
   Again, enter the preciously chosen passphrase when requested to do so

   4 – We’ll then create our certificate key
   openssl genrsa -out localhost.key 2048

   5 – And using the key, we’ll create a certificate request:
   openssl req -new -key localhost.key -out localhost.csr -sha256
   Note: when being prompted for the “Common Name”, use “localhost”, as this will be the way we’ll reference the MQTT broker locally.
   If you have a DNS entry pointing to the host server, you can use that too. All remaining fields can be left empty.

   6 – Lastly, we’ll create the certificate, using the certificate request created in step 5 and the CA root certificate created in step 5, also valid for 180 days
   openssl x509 -req -in mosquitto.csr -CA ca-root.crt -CAkey ca.key -CAcreateserial -out localhost.crt -days 180

   When you’re done you should have the following files in the directory:

    ca-root.crt – Certificate authority root certificate
    ca.csr – Certificate authority certificate signing request
    ca.key – Certificate Authority certificate Key
    localhost.crt – Mosquitto certificate
    localhost.csr – Mosquitto certificate signing request
    localhost.key – Mosquitto certificate key

   

Firewall
........
::

   dnf -y install firwalld
   systemctrl enable firewalld
   systemctrl start firewalld
   firewall-cmd --permanent --zone=public --add-port=80/tcp
   firewall-cmd --permanent --zone=public --add-port=442/tcp
   firewall-cmd --permanent --zone=public --add-port=443/tcp
   firewall-cmd --permanent --zone=public --add-port=5925/tcp
   firewall-cmd --permanent --zone=public --add-port=1883/tcp
   firewall-cmd --permanent --zone=public --add-port=5671/tcp
   firewall-cmd --permanent --zone=public --add-port=5672/tcp
   firewall-cmd --permanent --zone=public --add-port=8883/tcp
   firewall-cmd --permanent --zone=public --add-port=8884/tcp
   firewall-cmd --permanent --zone=public --add-port=15672/tcp

   firewall-cmd --reload

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
   < insert rmapadmin as password >
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
   < insert rmap as password >

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

::
   dnf install python3-mod_wsgi
   dnf install mod_security mod_security_crs
   dnf install mod_ssl

   
::
   useradd -r rmap
   mkdir /home/rmap
   chown rmap:rmap /home/rmap
   mkdir /rmap/cache
   chown rmap:rmap /rmap/cache
   mkdir  /usr/share/rmap/media
   chown rmap:rmap  /usr/share/rmap/media

   cp -r ~/certs /etc/httpd
   chown -R apache /etc/httpd/certs
   
`/etc/httpd/conf.modules.d/00-mpm.conf <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/httpd/conf.modules.d/00-mpm.conf>`_

`/etc/httpd/conf.d/rmap.conf <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/httpd/conf.d/rmap.conf>`_

`/etc/httpd/conf.d/rmap.inc <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/httpd/conf.d/rmap.inc>`_

`/etc/httpd/conf.d/ssl.conf <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/httpd/conf.d/ssl.conf>`_

`/etc/httpd/modsecurity.d/crs-setup.conf <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/httpd/modsecurity.d/crs-setup.conf>`_

`/etc/httpd/modsecurity.d/local_rules/modsecurity_localrules.conf <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/httpd/modsecurity.d/local_rules/modsecurity_localrules.conf>`_

::
   
   chkconfig httpd on
   service httpd start


Stunnel
.......

Create file with psk keys:
::
   
   rmapctrl --exportmqttpsk > /etc/stunnel/file.psk

   
`/etc/stunnel/stunnel.conf <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/stunnel/stunnel.conf>`_

`/etc/cron.d/stunnel_presharedkey  <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/cron.d/stunnel_presharedkey>`_


::
   
   chkconfig stunnel on
   service stunnel start


Arkimet
.......

::
   
   dnf install arkimet arkimet-postprocessor-suite
   useradd  -r arkimet
   mkdir /home/arkimet
   chown arkimet:arkimet /home/arkimet
   mkdir /rmap/arkimet/

::
   
   chown -R arkimet:rmap /rmap/arkimet/
   chmod -R g+w  /rmap/arkimet/

oppure:

::
   
   chown -R rmap:rmap /rmap/arkimet/

::
   
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

   cp -r ~/certs /etc/mosquitto/
   chown -R mosquitto /etc/mosquitto/certs

   touch /etc/mosquitto/pwfile
   chkconfig mosquitto on
   service mosquitto start



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

   cp -r ~/certs /etc/rabbitmq/
   chown -R rabbitmq /etc/rabbitmq/certs

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
   
   dnf -y install monit

`/etc/monitrc <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/monitrc>`_

`/etc/monit.d/rmap <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/monit.d/rmap>`_

`/usr/local/bin/check_mosquitto <https://raw.githubusercontent.com/r-map/rmap/master/server/usr/local/bin/check_mosquitto>`_
::
   
 chkconfig monit on
 service monit start

Cron
....

::

   mkdir /rmap/dballe
   chown -R rmap:rmap /rmap/dballe


`/etc/cron.d/dballe2arkimet <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/cron.d/dballe2arkimet>`_

`/etc/cron.d/luftdaten <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/cron.d/luftdaten>`_


Opzionali per provider esterni con appositi convertitori:

`/etc/cron.d/arpae_aq_ckan <https://raw.githubusercontent.com/r-map/rmap/master/server/etc/cron.d/arpae_aq_ckan>`_

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

scaricare il file `/etc/tmpfiles.d/rmap.conf <https://raw.githubusercontent.com/r-map/rmap/master/server-data-ingestion/etc/tmpfiles.d/rmap.conf>`__

::

  mkdir /rmap
  chmod go+rx /rmap


::

   mkdir /var/log/rmap
   chown -R rmap:rmap /var/log/rmap

scaricare il file `/etc/rmap/rmap-site.cfg <https://raw.githubusercontent.com/r-map/rmap/master/server-data-ingestion/etc/rmap/rmap-site.cfg>`__

cambiare la password dell'utente amministratore.

Mosquitto
.........

::
   
   mkdir /etc/mosquitto/conf.d
   mkdir /rmap/mosquitto
   chown mosquitto:mosquitto /rmap/mosquitto

   
scaricare il file `/etc/mosquitto/conf.d/rmap.conf <https://raw.githubusercontent.com/r-map/rmap/master/server-data-ingestion/etc/mosquitto/conf.d/rmap.conf>`__


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

scaricare il file `/etc/rabbitmq/enabled_plugins <https://raw.githubusercontent.com/r-map/rmap/master/server-data-ingestion/etc/rabbitmq/enabled_plugins>`__

scaricare il file `/etc/rabbitmq/rabbitmq-env.conf <https://raw.githubusercontent.com/r-map/rmap/master/server-data-ingestion/etc/rabbitmq/rabbitmq-env.conf>`__

scaricare il file `/etc/rabbitmq/rabbitmq.config <https://raw.githubusercontent.com/r-map/rmap/master/server-data-ingestion/etc/rabbitmq/rabbitmq.config>`__


Installare il certificato ssl/tls per il dominio del server in:

::
   
   /etc/arpaecert/arpae_it-rabbitmq.pem
   /etc/arpaecert/arpae_it-mosquitto.pem

e impostare gli opportuni privilegi di lettura/scrittura.

::
   
   -rw------- 1 mosquitto mosquitto 6849  1 dic 14.23 arpae_it-mosquitto.pem
   -rw------- 1 rabbitmq  rabbitmq  6849 20 nov 10.49 arpae_it-rabbitmq.pem

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

scaricare il file `/etc/monitrc <https://raw.githubusercontent.com/r-map/rmap/master/server-data-ingestion/etc/monitrc>`__

scaricare il file `/etc/monit.d/rmap <https://raw.githubusercontent.com/r-map/rmap/master/server-data-ingestion/etc/monit.d/rmap>`__

::

 chmod 0700 /etc/monitrc /etc/monit.d/rmap
 chown root:root /etc/monitrc /etc/monit.d/rmap
 systemctl enable monit
 systemctl start monit
 

Sincronizzazione file statici per autenticazione e autorizzazione da un server RMAP backend
...........................................................................................

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


Installazione server RMAP solo funzionalità BACKEND basato su Rocky Linux 8 (a seervizio per la data ingestion su un'altra macchina)
------------------------------------------------------------------------------------------------------------------------------------

Questi file sono specializzati per un server di solo backend:

`/etc/httpd/conf.d/rmap.inc <https://raw.githubusercontent.com/r-map/rmap/master/server-backend/etc/httpd/conf.d/rmap.inc>`__

`/etc/httpd/modsecurity.d/crs-setup.conf <https://raw.githubusercontent.com/r-map/rmap/master/server-backend/etc/httpd/modsecurity.d/crs-setup.conf>`__

In questi due file sostituire la stringa <insert IP of data-ingestion machine> con quanto indicato.

`/etc/monit.d/rmap <https://raw.githubusercontent.com/r-map/rmap/master/server-backend/etc/monit.d/rmap>`__

Servizi
.......

Disabilitare mosquitto.
