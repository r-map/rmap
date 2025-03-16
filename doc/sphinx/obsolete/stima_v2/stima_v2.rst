.. _stima_v2-reference:

Stima V2
========

Questa versione della stazione Stima si basa su moduli Microduino.

Si consiglia fortemente la realizzazione di Stima V3 o successive
versioni.


Articoli sulla rivista Elettronica In
"""""""""""""""""""""""""""""""""""""

Parte 1 :download:`pdf <prima_puntata.pdf>`

Parte 2 :download:`pdf <seconda_puntata.pdf>`

Parte 3 :download:`pdf <terza_puntata.pdf>`

.. _how_to_stima_versione_2:

How To Stima Versione 2
"""""""""""""""""""""""

STIMA PCB
~~~~~~~~~

Per il momento è possibile ottenere i circuiti stampati da OSH Park:

* STIMA bluetooth https://oshpark.com/shared_projects/cKkpi2IN
* STIMA I2C https://oshpark.com/shared_projects/DMztH0Vu
* STIMA breakout sensor board:
     https://oshpark.com/shared_projects/6XorC9H2
* STIMA I2C HUB https://oshpark.com/shared_projects/e5HS7gP1
* Stima AirQuality Connector board
     https://oshpark.com/shared_projects/owmle8Pe (Ancora da testare)
* MICS-4514 Breakout board
     https://oshpark.com/shared_projects/ei8rMhO6 (Ancora da testare)

.. _installazione_ambiente_di_sviluppo:

Installazione ambiente di sviluppo
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. _sdk_arduino:

SDK Arduino
^^^^^^^^^^^

-  Installare arduino 1.8.1 da https://www.arduino.cc/en/Main/Software
-  scaricare l'ultima versione del software stima (file
   stimasketchbookbluetooth) da https://github.com/r-map/rmap/releases
-  scompattare il file zip

-  aprire l'ide arduino e in file -> impostazioni -> percorso della
   cartella degli sketch selezionare la cartella sketchbook appena
   scompattata dal file scaricato
-  chiudere e riaprire l'ide

-  sezionare in -> strumenti

   | Scheda: Microduino Core+ (644pa)``
   | Processore: ATmega644pa@16M5V``
   | Porta: (quella disponibile) ``

-  selezionare in -> Sketch -> Verifica e compila

Ora per uplodare il firmware sul microprocessore dovrete collegare tra
loro SOLO

-  MICRODUINO Core+ ATMEGA644PA
-  MICRODUINO Shield USB/TTL

poi selezionate nella IDE di arduino:

-  selezionare in -> Sketch -> Carica

Utilizzando MICRODUINO Core+ con ATMEGA 1284 sostituire 1284 a 644 nei
passi precedenti.

.. _pacchetto_python_rmap:

Pacchetto python RMAP
^^^^^^^^^^^^^^^^^^^^^

Queste istruzioni permettono di installare il pacchetto RMAP scritto in
python utile alla configurazione delle stazioni:

| ``mkdir rmap``
| ``cd rmap``
| ``virtualenv myrmap``
| ``source myrmap/bin/activate``
| ``pip install rmap``

successivamente ogni volta che si vuole eseguire un comando rmap
eseguire preventivamente:

| ``cd rmap``
| ``source myrmap/bin/activate``

.. _modulo_stima_th:

Modulo Stima-th
~~~~~~~~~~~~~~~

Hardware
^^^^^^^^

-  ponticellare con un resistore A1-centrale con A1-
-  ponticellare con un resistore A0-centrale con A0+

Una volta saldati i terminali dei cavi e protetti con nastro
autoagglomerante è importante preservare le connessioni dalla corrosione
degli agenti atmosferici e consigliamo uno strato di silicone protettivo
spray del tipo Electrolube DCR200H; solo una volta asciugato lo strato
di vernice protettiva rimuovere la protezione adesiva del sensore HIH

Configurazione
^^^^^^^^^^^^^^

Bisogna modificare l'indirizzo i2c del sensore hih.

-  Assemblare microduino core+ con microduino FT232RL con Stima-i2c
-  Caricare il firmware sensor_config
-  connettere:

| ``* Stima-th VCC  ->  Stima-i2c rl2``
| ``* Stima-th GND  ->  Stima-i2c GND``
| ``* Stima-th SDA  ->  Stima-i2c SDA``
| ``* Stima-th SCL  ->  Stima-i2c SCL``

alla porta seriale inviare i comandi:

| ``Sensor to config:``
| ``w = i2c-wind``
| ``s = i2c-windsonic``
| ``t = i2c-th``
| ``r = i2c-rain``
| ``h = hih humidity sensor``
| ``? = help - this page``

h

| ``If you want to use Command Mode to setup HIH61xx sensor you MUST use one pin to power the HIH!``
| ``If not this will not work!``
| ``digit old i2c address for HIH sensor (1-127)``

39

``digit the pin number for power HIH sensor (1-127)``

4

| ``started HIH fo command mode``
| ``digit new i2c address for HIH sensor (1-127)``

38

``Done; switch off``

Assemblaggio
^^^^^^^^^^^^

Utilizzare un cavo quadripolare a bassa capacità se possibile schermato
da esterno con da un lato connettore rj45 e dall'altro la connssione a
Stima-th.

Per questi sensori installati all'esterno è importante preservare le
connessioni dalla corrosione degli agenti atmosferici e consigliamo uno
strato di silicone protettivo spray del tipo Electrolube DCR200H.

Inserire Stima-th nell'apposito schermo per le radiazioni.

.. _modulo_stima_i2c_th:

Modulo Stima-I2C-th
~~~~~~~~~~~~~~~~~~~

.. _hardware_1:

Hardware
^^^^^^^^

Assemblare le schede impilabili:

-  Board microduino core+ 644
-  Board microduino SD
-  Board STIMA-I2C
-  Microduino FT232RL

Software
^^^^^^^^

-  Caricare il firmware i2c-th tramite microduino FT232RL

.. _configurazione_1:

Configurazione
^^^^^^^^^^^^^^

Bisogna configurare l'indirizzo i2c del sensore hih.

-  Assemblare microduino core+ con microduino FT232RL con Stima-i2c
-  Caricare il firmware sensor_config
-  connettere:

| ``* Stima-i2c modulo configurazione +5  ->  Stima-i2c modulo stima +5``
| ``* Stima-th modulo configurazione GND  ->  Stima-i2c modulo stima GND``
| ``* Stima-th modulo configurazione SDA  ->  Stima-i2c modulo stima SDA``
| ``* Stima-th modulo configurazione SCL  ->  Stima-i2c modulo stima SCL``

alla porta seriale inviare i comandi:

| ``Terminal ready``
| ``Start sensor config``
| ``     Sensor configuration - 1.0``
| ``scan I2C bus:``
| `` i = scan one time``
| ``Sensor to config:``
| `` w = i2c-wind``
| `` s = i2c-windsonic``
| `` t = i2c-th``
| `` r = i2c-rain``
| `` h = hih humidity sensor``
| ``? = help - this page``

``digit new i2c address for i2c-th (1-127)``

35

``digit new i2c_temperature address for i2c-th (1-127)``

73

``digit new i2c_humidity address for i2c-th (1-127)``

38

``digit 1 for oneshotmode; 0 for continous mode for i2c-th (0/1)``

0

``Done; switch off``

.. _assemblaggio_1:

Assemblaggio
''''''''''''

-  connettere:

| ``* Stima-i2c modulo configurazione +5  ->  Stima-i2c modulo stima +5``
| ``* Stima-th modulo configurazione GND  ->  Stima-i2c modulo stima GND``
| ``* Stima-th modulo configurazione SDA  ->  Stima-i2c modulo stima SDA``
| ``* Stima-th modulo configurazione SCL  ->  Stima-i2c modulo stima SCL``

.. _modulo_stima_i2c_rain:

Modulo Stima-I2C-rain
~~~~~~~~~~~~~~~~~~~~~

.. _hardware_2:

Hardware
^^^^^^^^

-  Assemblare le schede impilabili:
-  Board microduino core+ 644
-  Board microduino SD
-  Board STIMA-I2C
-  Microduino FT232RL

.. _software_1:

Software
^^^^^^^^

-  Caricare il firmware i2c-rain tramite microduino FT232RL

.. _configurazione_2:

Configurazione
^^^^^^^^^^^^^^

Nessuna necessaria.

.. _assemblaggio_2:

Assemblaggio
^^^^^^^^^^^^

Il due poli del contatto della bascula vanno collegati l'uno a massa e
l'altro al pin D2 del microcontrollore. Aggiungere una resistenza di
pullup di qualche migliaio di ohm tra il pin D2 e +5V.

.. _scheda_stima_i2c_hub:

Scheda STIMA-I2C-Hub
~~~~~~~~~~~~~~~~~~~~

.. _hardware_3:

Hardware
^^^^^^^^

-  Segare il circuito stampato seguendo i fori guida dopo il secondo
   connettore RJ45.
-  Ponticellare con saldature per ottenere 4 file di dupoint a 5V e uno
   a 3.3V
-  Ponticellare con saldature per ottenere un rj45 a 5V e un rg45 a
   3.3v.

.. _assemblaggio_3:

Assemblaggio
^^^^^^^^^^^^

Collegare i moduli Stima-th Stima-rain Stima-GSM e il display LCDtramite
cavo quadripolare alle file di dupoint su Stima-i2c-hub impostati a 5V.

Collegare i sensori Stima-th al connettore RJ45 alimentato a 3.3V.

.. _stazione_stima_gsm_thp:

stazione STIMA GSM THP
~~~~~~~~~~~~~~~~~~~~~~

.. _modulo_stima_gsm:

Modulo Stima-GSM
^^^^^^^^^^^^^^^^

.. _hardware_4:

Hardware
''''''''

Apportare queste due modifiche al modulo microduino GPRS/GSM:

-  cortocircuitare con una saldatura i due terminali del pulsante di
   accensione "POWR KEY"
-  connettere il punto "RST" al pin D6
-  saldare i ponticelli per portarli a TX1,RX1 (jumper for tx, jumper
   for rx)

Assemblare le schede impilabili:

-  Board microduino core+ 1284
-  Board microduino GPRS/GSM
-  Board microduino SD
-  Board STIMA-I2C
-  Microduino FT232RL

.. _software_2:

Software
''''''''

In arduino/sketchbook/rmap/rmap copiare il file stima_gsm_report.h in
rmap_config.h

In sketchbook/libraries/PubSubClient/PubSubClient.h modificare come
segue:

::
   
   // if use sim800 client
   #include "sim800Client.h"
   #define TCPCLIENT sim800Client
   // if use arduino_uip or etherclient
   //#include "Client.h"
   //#include "Stream.h"
   //#define TCPCLIENT Client

Caricare il firmware rmap tramite microduino FT232RL.

.. _assemblaggio_4:

Assemblaggio
^^^^^^^^^^^^

Collegare tutti i moduli tramite l'hub i2c rispettando le corrette
tensioni di alimentazione (STIMA-TH a 3.3V). Collegare all'HUB i2c anche
il Display LCD 20x4 con interfaccia I²C alimentandolo a 5V.

Inserire la SIM della TIM senza richiesta di PIN e la scheda SD nel
modulo Stima-GSM.

Alimentare il modulo Stima-GSM tramite il connettore micro USB della
scheda Stima-i2c con un alimentatore a 5V 2A o in alternativa tramite i
pin GND e +5 della scheda Stima-i2c. Collegare i pin denominati "LED"
della scheda del display LCD a un pulsante per l'attivazione della
retroilluminazione.

E' possibile utilizzare il connettore micro USB della scheda Microduino
FT232RL dei vari moduli per ottenere su porta seriale messaggi di debug.

.. _configurazione_3:

Configurazione
''''''''''''''

Per ottenere una username e una password iscriversi al sito
http://rmap.cc/registrazione/register/

Eventualmente (dopo la prima configurazione) ponticellare sulla scheda
Stima-i2c i pin "Set".

eseguire i comandi:

| ``rmapctrl --syncdb``
| ``rmap-configure --wizard --station_slug=``\ \ `` --height=``\ \ `` --stationname=``\ \ `` --username=``\ \ `` --password=``\ \ `` --server=rmap.cc --lat=<xx.xxxxx> --lon=<xx.xxxxx> --mqttrootpath=report --mqttmaintpath=report``
| ``rmap-configure --addboard --station_slug=``\ \ `` --board_slug=``\ \ `` --user=``\ \ `` --serialactivate --mqttactivate --mqttuser=``\ \ `` --mqttpassword=``\ \ `` --mqttsamplerate=900``
| ``rmap-configure --addsensors_by_template=stima_report_thp --station_slug=``\ \ `` --board_slug=``\ \ `` --user=``\ \ `` --password=``\ \ `` --upload_to_server``
| ``rmap-configure --config_station --station_slug=``\ \ `` --board_slug=``\ \ `` --username=``\ \ `` --baudrate 115200``

sostituendo i valori <> con opportuni valori.

Rimuovere il ponticello ai pin "Set".

.. _operazioni_finali:

Operazioni finali
^^^^^^^^^^^^^^^^^

Una volta verificato il corretto funzionamento della stazione è
possibile ricaricare i firmware con l'opzione di debug disabilitata
commentando l'apposita variabile del preprocessore C nei file config.h
presenti nelle cartelle dei firmware; in questo caso sarà anche
possibile rimuovere le schede Microduino FT232RL dai moduli.

.. _stazione_stima_master_thp:

stazione STIMA MASTER THP
~~~~~~~~~~~~~~~~~~~~~~~~~

.. _modulo_stima_master:

Modulo Stima-master
^^^^^^^^^^^^^^^^^^^

.. _hardware_5:

Hardware
''''''''

Assemblare le schede impilabili:

-  Board microduino core+ 1284
-  Board microduino wiz W5500 oppure microduino ENC28j60
-  Board microduino SD
-  Board microduino RJ45-POE
-  Board STIMA-I2C
-  Microduino FT232RL

.. _software_3:

Software
''''''''

In arduino/sketchbook/rmap/rmap copiare il file stima_master_report.h in
rmap_config.h

In sketchbook/libraries/PubSubClient/PubSubClient.h modificare come
segue:

::
   
   // if use sim800 client
   //#include "sim800Client.h"
   //#define TCPCLIENT sim800Client
   // if use arduino_uip or etherclient
   #include "Client.h"
   #include "Stream.h"
   #define TCPCLIENT Client

Caricare il firmware rmap tramite microduino FT232RL.

.. _assemblaggio_5:

Assemblaggio
^^^^^^^^^^^^

Collegare tutti i moduli tramite l'hub i2c rispettando le corrette
tensioni di alimentazione (STIMA-TH a 3.3V). Collegare all'HUB i2c anche
il Display LCD 20x4 con interfaccia I²C e Modulo Tiny RTC I²C Real Time
Clock con DS1307 alimentandoli a 5V

Alimentare il modulo Stima-master tramite il cavo ethernet con power
over ethernet con opportuno injector e alimentazione. Ponticellare i pin
denominati "LED" della scheda del display LCD.

E' possibile utilizzare il connettore micro USB della scheda Microduino
FT232RL dei vari moduli per ottenere su porta seriale messaggi di debug.

.. _configurazione_4:

Configurazione
''''''''''''''

Per ottenere una username e una password iscriversi al sito
http://rmap.cc/registrazione/register/

Eventualmente (dopo la prima configurazione) ponticellare sulla scheda
Stima-i2c i pin "Set".

Eseguire i comandi:

| ``rmapctrl --syncdb``
| ``rmap-configure --wizard --station_slug=``\ \ `` --height=``\ \ `` --stationname=``\ \ `` --username=``\ \ `` --password=``\ \ `` --server=rmap.cc --lat=<xx.xxxxx> --lon=<xx.xxxxx>  --mqttrootpath=report --mqttmaintpath=report``
| ``rmap-configure --addboard --station_slug=``\ \ `` --board_slug=``\ \ `` --user=``\ \ `` --serialactivate --mqttactivate --mqttuser=``\ \ `` --mqttpassword=``\ \ `` --mqttsamplerate=900 --tcpipactivate --tcpipntpserver="it.pool.ntp.org" --tcpipname=stima``
| ``rmap-configure --addsensors_by_template=stima_report_thp --station_slug=``\ \ `` --board_slug=``\ \ `` --user=``\ \ `` --password=``\ \ `` --upload_to_server``
| ``rmap-configure --config_station --station_slug=``\ \ ``  --board_slug=``\ \ `` --username=``\ \ `` --baudrate 115200``

sostituendo i valori tra <> con opportuni valori.

Rimuovere il ponticello ai pin "Set".

.. _operazioni_finali_1:

Operazioni finali
^^^^^^^^^^^^^^^^^

Una volta verificato il corretto funzionamento della stazione è
possibile ricaricare i firmware con l'opzione di debug disabilitata
commentando l'apposita variabile del preprocessore C nei file config.h
presenti nelle cartelle dei firmware; in questo caso sarà anche
possibile rimuovere le schede Microduino FT232RL dai moduli.

.. _howto_in_sintesi_sempre_aggiornato_per_gli_altri_moduli:

HowTo in sintesi sempre aggiornato per gli altri moduli
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. _hardware_6:

Hardware
~~~~~~~~

.. _modulo_stima_bluetooth:

Modulo Stima-bluetooth
^^^^^^^^^^^^^^^^^^^^^^

E' composto dalle seguenti schede:

-  Board microduino core+ 644
-  Board stima-bluetooth
-  Board STIMA-I2C
-  Board microduino nRF24 (opzionale)

.. _modulo_stima_master_1:

Modulo Stima-Master
^^^^^^^^^^^^^^^^^^^

E' composto dalle seguenti schede:

-  Board microduino core+ 1284
-  Board microduino ENC
-  Board STIMA-I2C
-  Board microduino nRF24 (opzionale)

.. _modulo_stima_satellite:

Modulo Stima-Satellite
^^^^^^^^^^^^^^^^^^^^^^

E' composto dalle seguenti schede:

-  Board microduino core+ 644
-  Board microduino nRF24
-  Board STIMA-I2C

.. _modulo_stima_gsmgprs:

Modulo Stima-GSM/GPRS
^^^^^^^^^^^^^^^^^^^^^

E' composto dalle seguenti schede:

-  Board microduino core+ 1284
-  Board microduino nRF24
-  Board microduino sim800
-  Board microduino SD
-  Board STIMA-I2C

.. _modulo_stima_i2c_sdsmics:

Modulo Stima-i2c-sdsmics
^^^^^^^^^^^^^^^^^^^^^^^^

E' composto dalle seguenti schede:

-  Board microduino core+ 644 5V
-  Board STIMA-AirQuality_Connector

-  Board STIMA-NO2-CO

questa la disposizione dei pin dell'hardware versione 2 osservata dal
lato dei connettori:

Board STIMA-NO2-CO

==== ======
GND  PWM
GNDD SCALE1
VREF SCALE2
VDD  NO2
+5   CO
==== ======

Board STIMA-AirQuality_Connector

=== === ==== ==== ===
GND GND VREF NONE +5V
=== === ==== ==== ===

=====  ===  ======
+3.3V  +5V  PWM
GND    GND  SCALE1
SDA    TX   SCALE2
SCL    RX   CO
+5V         NO2
=====  ===  ======

Connettere:

============ === ==========================
STIMA-NO2-CO     STIMA-AirQuality_Connector
============ === ==========================
GNDD         <-> GND
VREF         <-> VREF
VDD          <-> +5V
PWM          <-> PWM
SCALE1       <-> SCALE1
SCALE2       <-> SCALE2
NO2          <-> NO2
CO           <-> CO
============ === ==========================

.. _firmware_stima_bluetooth:

Firmware STIMA-BlueTooth
~~~~~~~~~~~~~~~~~~~~~~~~

installare arduino 1.6.5 da:

https://www.arduino.cc/en/Main/Software

o tramite la propria distribuzione

scaricare l'ultima versione del software stima (file stimasketchbook) da

https://github.com/r-map/rmap/releases

* scompattare il file zip
* aprire l'ide arduino e in file -> impostazioni -> percorso della
  cartella degli sketch
* selezionare la cartella sketchbook appena
  scompattata dal file scaricato
* chiudere e riaprire l'ide

.. _modulo_stima_bluetooth_1:

modulo Stima-bluetooth
^^^^^^^^^^^^^^^^^^^^^^

Se in questo modulo avete montato anche la board microduino nRF24 e
quindi volete utilizzare anche il modulo Stima-Satellite con un editor
modificate il file
sketchbook/libraries/SensorDriver/SensorDriver_config.h scommentando
l'opzione

``#define RADIORF24``

scommentando anche l'opzione

``#define AES``

abiliterete anche la crittografia AES ma consigliamo questa ultima
opzione solo ai più esperti.

In sketchbook/rmap/rmap copiare il file stima_bluetooth.h in
rmap_config.h

* sezionare in -> strumenti

  * Scheda: Microduino Core+ (644pa)``
  * Processore: ATmega644pa@16M5V
  * Porta: (quella disponibile)

* selezionare in -> Sketch -> Verifica e compila

Ora per uplodare il firmware sul microprocessore dovrete collegare tra
loro SOLO

* MICRODUINO Core+ ATMEGA644PA
* MICRODUINO Shield USB/TTL

poi selezionate nella IDE di arduino:

* selezionare in -> Sketch -> Carica

  .. _modulo_stima_master_2:

modulo Stima-master
^^^^^^^^^^^^^^^^^^^

Se in questo modulo avete montato anche la board microduino nRF24 e
quindi volete utilizzare anche il modulo Stima-Satellite con un editor
modificate il file
sketchbook/libraries/SensorDriver/SensorDriver_config.h scommentando
l'opzione "#define RADIORF24" ; scommentando anche l'opzione #define AES
abiliterete anche la crittografia AES ma consigliamo questa ultima
opzione solo ai più esperti.

In sketchbook/rmap/rmap copiare il file stima_master.h in rmap_config.h

sezionare in -> strumenti

* Scheda: Microduino Core+ (1284pa)
* Processore: ATmega1284pa@16M5V
* Porta: (quella disponibile)

sezionare in -> strumenti -> cartella degli sketch -> rmap -> rmap

selezionare in -> Sketch -> Carica

.. _modulo_stima_satellite_1:

modulo Stima-satellite
^^^^^^^^^^^^^^^^^^^^^^

In sketchbook/rmap/rmap copiare il file stima_satellite.h in
rmap_config.h

sezionare in -> strumenti

* Scheda: Microduino Core+ (644pa)
* Processore: ATmega644pa@16M5V
* Porta: (quella disponibile)

sezionare in -> strumenti -> cartella degli sketch -> rmap -> rmap

selezionare in -> Sketch -> Carica

.. _modulo_stima_gsm_1:

modulo Stima-gsm
^^^^^^^^^^^^^^^^

In sketchbook/rmap/rmap copiare il file stima_gsm.h in rmap_config.h Se
non utilizzerete una SIM card della TIM inserite in fondo al file
rmap_config.h:

#. define GSMAPN ""
#. define GSMUSER ""
#. define GSMPASSWORD ""

con gli opportuni valori.

In sketchbook/libraries/PubSubClient/PubSubClient.h modificare come
segue:

::
   
   // if use sim800 client
   #include "sim800Client.h"
   #define TCPCLIENT sim800Client
   
   // if use arduino_uip or etherclient
   //#include "Client.h"
   //#include "Stream.h"
   //#define TCPCLIENT Client

sezionare in -> strumenti

* Scheda: Microduino Core+ (1284pa)
* Processore: ATmega1284pa@16M5V
* Porta: (quella disponibile)

sezionare in -> strumenti -> cartella degli sketch -> rmap -> rmap

selezionare in -> Sketch -> Carica

.. _applicazione_rmap:

Applicazione Rmap
~~~~~~~~~~~~~~~~~

Android
^^^^^^^

L'installazione su android è semplicissima; è sufficiente ricercare tra
le app su google play "rmap" e procedere all'installazione:
https://play.google.com/store/apps/details?id=org.test.rmap.

Linux
^^^^^

L'installazione in ambiente Linux richiede la disponibilità di alcuni
pacchetti e del comando pip. Prima di tutto bisogna installare Kivy
seguendo le istruzioni sul sito di Kivy
http://kivy.org/docs/installation/installation-linux.html. Per il
comando pip nelle distribuzioni Linux più diffure lo si ottiene
installando il pacchetto python-pip. Per installare da utente non
privilegiato l'ambiente rmap si può usare virtualenv e pip; da terminale
eseguire:

::
   
   virtualenv --system-site-packages rmap
   source rmap/bin/activate
   
   pip install --upgrade rmap

Poi attivare l'interfaccia utente grafica:

::
   
   source rmap/bin/activate
   rmapgui

In alternativa si può provare a installare Kivy tramite pip:

::
   
   pip install cython
   pip install kivy

Per aggiornare l'App una volta chiusa la finestra grafica nella finestra
dei comandi al prompt digitare:

::
   
   pip install --upgrade rmap

Windows
^^^^^^^

Seguire le istruzioni a
https://kivy.org/docs/installation/installation-windows.html

poi:

::
   
   python -m pip install rmap

Le istruzioni che seguono sono per una vecchia modalità per un vecchio
pacchetto:

L'installazione in windows è molto semplice in quanto il file da
scaricare è autoscompattante e comprende tutto l'ambiente necessario a
Rmap.  Sacricare quindi il file rmapwindows da:

https://github.com/r-map/rmap/releases

ed eseguirlo per scompattarlo.

Per far partire l'applicazione a questo punto basterà eseguire il file
rmapgui contenuto nella cartella rmap

Per aggiornare l'App una volta chiusa la finestra grafica nella
finestra dei comandi al prompt digitare:
::
   
  pip install --upgrade rmap

.. _mac_osx:

Mac OSX
^^^^^^^

Prima di tutto bisogna installare Kivy su Macosx seguendo le istruzioni
https://kivy.org/docs/installation/installation-osx.html e installere
gettext da http://www.ellert.se/twain-sane/

poi:

``kivi -m pip install --upgrade rmap``

si può attivare il programma:

``/Applications/Kivy.app/Contents/Resources/venv/bin/rmapgui``

Per aggiornare l'App una volta chiusa la finestra grafica nella finestra
dei comandi al prompt digitare:

``pip install --upgrade rmap``

.. _configurazione_moduli:

Configurazione moduli
~~~~~~~~~~~~~~~~~~~~~

Per pubblicare i dati sul server RMAP.cc bisogna registrarsi al sito; il
bottone "Registrazione" dell'app dovrebbe aprire un browser alla url
della registrazione che comunque è
http://rmap.cc/registrazione/register/ Una volta fatta la registrazione
sarete in possesso di uno user e di una password.

A questo punto dovrete trasferire la vostra configurazione sulla eeprom
del microcontrollore; per farlo:

* ponticellate sulla board Stima-I2C i pin contrassegnati con "SET" con un jumper.
* collegate il modulo con la board Microduino FT232RL alla USB del vostro PC.

.. _configurazione_tramite_lapplicazione_grafica:

Configurazione tramite l'applicazione grafica
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Dovrete accedere al menu "Impostazioni" che si aprirà automaticamente al
primo avvio e accedere alle sottosezioni:

Nella sezione "Rmap" dovrete inserire "RMAP user" e "RMAP password"
ottenuti durante la registrazione a rmap.cc.

Dopo aver accoppiato il dispositivo bluetooth (dispositivo HC-05
inserendo come pin "1234") si può attivare il programma. In windows
tramite le apposite interfacce di windows procedere all'accoppiamento
del dispositivo blue-tooth (dispositivo HC-05 inserendo come pin "1234")
e richiedere la creazione della relativa porta seriale COM13; in Linux
per accoppiare il dispositivo stima-bluetooth cosigliamo di utilizzare
blueman-manager contenuto nel pacchetto blueman; seguendo pochi passi
con l'interfaccia grafica risulta molto facile accoppiare il dispositivo
HC-05 inserendo come pin "1234"; il device RFCOM0 viene utilizzato per
la comunicazione seriale.

Ora dal menu Impostazioni selezionate la sezione "Sensors" e impostate:

::
   
      per il modulo Stima-Bluetooth
          Name: HC-05
          Station: BT_fixed
          Board:
              su android: BT_fixed
              su linux: BT_fixed_LINUX
              su windows: BT_fixed_WINDOWS
              su OSX: BT_fixed_OSX 
          Template: test_indirect
          Remote Board: stima_bt
          Remote Template: test
      per il modulo Stima-Master o Stima-gsm
          Station: ETH_fixed
          Board:
              su linux: rmapgui_LINUX
              su windows: rmapgui_WINDOWS
              su OSX: rmapgui_OSX 
          Template: test_indirect
          Remote Board: master_eth_fixed
          Remote Template: test (test_master se avete la board nRF24)
      per il modulo Stima-Satellite
          come per il modulo Stima-Master ma come Remote Board: satellite_eth_fixed
   
Nella sezione "Location" potete inserire manualmente le vostre
coordinate e selezionare "Close" attivando la stazione. Se non conoscete
le vostre coordinate dalla pagina "Posizione" selezionate accuratamente
la vostra posizione e salvatela con il tasto "Salva posizione". La prima
pagina dell'App "Avvia" presenta un manuale che potrà aiutarvi.

Dalla pagina "Dati automatici" premere il bottone "configura" e
verificate che tutto vada a buon fine.

.. _configurazione_a_linea_di_comando:

Configurazione a linea di comando
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

E' possibile fare tutte le funzioni di configurazioni con due comandi da
terminale: nel caso di windows utilizzate la finestra che rimane aperta
dopo aver eseguito rmap-configure.bat; su Linux o OSX attivate il
virtualenv di python come per eseguire l'App grafica. Il primo comando
inizializza il DB. Il secondo con l'opzione --wizard inserisce tutti i
metadati nel DB, --config_station trasferisce la configurazione sulla
eeprom del microcontrollore e --upload_to_server comunica i metadati al
server:

| ``rmapctrl --syncdb``
| ``rmap-configure --station_slug=ETH_fixed --board_slug=master_eth_fixed --height=``\ \ `` --stationname="``\ \ ``" --username=``\ \ `` --password=``\ \ `` --server=rmap.cc --samplerate=5 --lat=``\ \ `` --lon=``\ \ `` --addsensors_by_template=``\ \ `` --wizard --config_station --upload_to_server``

Ovviamente sostituite il contenuto tra <> con i vostri dati; cercate di
avere latitudine e longitudine definita fino alla quinta cifra decimale;
dovrà essere "test_master" se il vostro modulo Stima-master o Stima-gsm
comprende una board microduino nRF24, "test" in tutti gli altri casi.

.. _operazioni_finali_2:

Operazioni finali
~~~~~~~~~~~~~~~~~

Scollegare il modulo dalla USB, rimuovete la board Microduino FT232RL e
attivate l'alimentazione saldando insieme il ponticello della board
Stima-I2C come da figura. attachment:microduino_r_freccia.png
Ricordatevi di rimuovere il jumper dai pin contrassegnati con "SET"
sulla board Stima-I2C. Messa in opera

Ai moduli potete collegare il display LCD e/o i relays. Sul modulo
Stima-gsm inserite una micro SD formattata FAT; dovete inserire anche
una SIM card; tutto è preconfigurato per una sim della TIM. Alimentate i
moduli tramite il connettore micro-USB sulla board Stima-I2C; il modulo
Stima-master potete alimentarlo con l'apposito Injector e un
alimentatore da 12 a 24V (connettore con positivo al centro). Sul modulo
Stima-gsm il sim800 va acceso manualmente tenendo premuto l'apposito
switch. Se tutto funziona regolarmente ogni 5 secondi i dati della
temperatura del modulo verranno inviati a rmap.cc. Se sul server rmap.cc
a questo punto fate login con il vostro utente accederete alla vostra
pagina personale con l'elenco delle vostre stazioni di misura e la
possibilità di visualizzare i grafici dei vostri dati. Ma per ora potete
inviare solo i dati di test (temperatura del modulo); nella prossima
puntata impareremo ad aggiungere sensori e navigare il server per la
visualizzazione dei dati.

.. _messa_in_opera:

Messa in opera
~~~~~~~~~~~~~~

Ai moduli potete collegare il display LCD e/o i relays. Sul modulo
Stima-GSM/GPRS inserite una micro SD formattata FAT; dovete inserire
anche una SIM card; tutto è preconfigurato per una sim della TIM.
Alimentate i moduli tramite il connettore micro-USB sulla board
Stima-I2C; il modulo Stima-master potete alimentarlo con l'apposito
Injector e un alimentatore da 12 a 24V (connettore con positivo al
centro). Sul modulo Stima-GSM/GPRS il sim800 va acceso manualmente
tenendo premuto l'apposito switch. Sulla rete ethernet dovrte avere un
server DHCP in quanto STIMA-Master lo richiede. Se tutto funziona
regolarmente ogni 5 secondi i dati della temperatura di test del modulo
verranno inviati a rmap.cc. Se sul server rmap.cc a questo punto fate
login con il vostro utente accederete alla vostra pagina personale con
l'elenco delle vostre stazioni di misura e la possibilità di
visualizzare i grafici dei vostri dati. Ma per ora potete inviare solo i
dati di test (temperatura del modulo); nella prossima puntata impareremo
ad aggiungere sensori e navigare il server per la visualizzazione dei
dati.

Box
~~~

Abbiamo progettato un interessante box per il modulo Stima-bluetooth e i
sensori di temperatura e umidità (presentati nella prossima puntata); é
composto da alcuni elementi impilabili a seconda delle esigenze. Ora lo
presentiamo nella versione con un comodo attacco a elastico da usare ad
esempio sul manubrio della bici per monitorare il percorso delle nostre
escursioni. Sono disponibili i file stl per stamparlo con una stampante
3D. I file per il box progettato da Mirco Bergamini si scaricano da
https://github.com/r-map/rmap/releases ; è consigliato stamparlo in PLA
bianco per ridurre gli effetti della radiazione solare.
attachment:box.jpg Il box è composto da tre pezzi: un attacco "a
elastico", un contenitore per il modulo stima, uno schermo per le
radiazioni per l'alloggiamento dei sensori. Gli ultimi due pezzi posso
essere montati a due a due permettendo di aumentare lo spazio contenuto
dal box; Sarà poi necessario praticare un foro tra il contenitore del
modulo stima e lo schermo pr i sensori per il passaggio dei 4 fili del
bus I2C; barre filettate e dadi completano l'assemblaggio.

.. _stima_overview:

Stima V2 Overview
"""""""""""""""""

Stazione modulare per la misura di parametri ambientali.

Premesse
~~~~~~~~

-  Aderisce alla Rete di Monitoraggio Ambientale Partecipativo (R-MAP)
-  Open hardware e open software
-  al momento vengono gestiti parametri meteorologici

Funzionalità
~~~~~~~~~~~~

Sensori
^^^^^^^

.. _collegamento_su_bus_i2c:

Collegamento su bus I2C
'''''''''''''''''''''''

I sersori devono essere compatibili con il bus I2C. Quando sensori I2C
non siano disponibili il problema viene risolto con un microcontrollore
che adatta le letture (analogiche o digitali) e le elaborazioni
(contatori, medie etc.) rendendole disponibili su registri interrogabili
tramite I2C.

Il protocollo I2C prevede l’utilizzo di un bus formato da due linee
bidirezionali. Le due linee, chiamate “scl” e “sda” rispettivamente,
trasportano la tempistica di sincronizzazione (chiamata anche “clock”) e
i dati. Abbiamo scelto il bus I2C in quanto:

-  È diventato lo standard di fatto per una serie di integrati tra cui i
   sensori
-  Si possono collegare fino a 127 dispositivi
-  La comunicazione è bidirezionale (read e write) con velocità
   assolutamente sufficienti per i nostri scopi
-  la lunghezza operativa dei cavi è adeguata al nostro utilizzo (anche
   alcune decine di metri)

.. _interrogazione_dei_sensori:

Interrogazione dei sensori
''''''''''''''''''''''''''

I sensori possono venire interrogati a richiesta tramite remote call
procedure oppure ad intervalli regolari. Quando interrogati a intervalli
regolari tutti i sensori vengono interrogati "in parallelo" ossia tutti
i sensori vengono impostati e configurati all'accensione poi
periodicamente vengono attivati e impartita la richiesta di lettura; il
driver del sensore torna il tempo di attesa necessario per avere la
misura disponibile; si attente il tempo necessario per il sensore più
lento; si effettuano tutte le letture. In questo modo si riescono a
campionare tutti i sensori solitamente entro i 3 secondi e considerando
i tempi per la loro pubblicazione sul server generalmente viene
utilizzata una frequenza di campionamneto pari a una ogni 5 secondi.
Tenendo i sensori normalmente in sleep si riducono anche i consumi.

Ogni sensore può restituire volori multipli (ad esempio temperatura e
umidità).

.. _operazioni_di_mantenimento:

Operazioni di mantenimento
''''''''''''''''''''''''''

Il software effettua periodicamente tutte le funzioni di mantenimento
necessarie a un corretto funzionamento quali quelle relative al DHCP o
alla sincronizzazione dell'orologio interno con una sorgente esterna.
Tutti i firmware hanno attivo un watchdog hardware che evita blocchi
permanenti dovuti a malfunzionamenti su eventi improbabili.

.. _orologio_di_riferimento:

Orologio di riferimento
'''''''''''''''''''''''

Una base dei tempi precisa è richiesta nel caso in cui sia necessario
salvare i dati localmente (su SD) nel caso la connessione utilizzata per
pubblicare i dati sul server (broker) non sia considerata stabile. Se
invece la connessione (trasporto) viene considerata stabile (o non sia
necessario recuperare i dati in caso di fault) un preciso orologio di
riferimento non è necessario e il tempo di riferimento verrà aggiunto
automaticamente dal server alla pubblicazione in tempo reale del dato.
Ci sono diversi sistemi per avere un orologio di riferimento preciso sui
moduli Stima.

.. _stazioni_fisse_o_mobili:

Stazioni fisse o mobili
'''''''''''''''''''''''

È possibile installare sia stazioni fisse, la cui posizione non cambia
nel tempo, sia stazioni mobili, sia terrestri che marine. Per aggiornare
la posizioni delle stazioni mobili viene utilizzato un GPS che può
essere o a bordo del modulo Stima o a bordo di un dispositivo android.

.. _attenzione_ai_consumi_energetici:

Attenzione ai consumi energetici
''''''''''''''''''''''''''''''''

Attenzione è stata posta alla limitazione dei consumi. Quando possibile
i microcontrollori e i sensori vengono messi in sleep e sono alcuni
interrupt a risvegliare il sistema. Questo agevola l'utilizzo con
batterie dei sistemi a basso consumo quali il modulo satellite che
funziona con un modulo radio.

.. _differenti_tipologie_di_rete:

Differenti tipologie di rete
''''''''''''''''''''''''''''

La configurazione della rete può essee differente a seconda delle
esigenze; oltre alla classica configurazione a stella (moduli master e
base) con un broker al centro è disponibile la configurazione ad albero
sia via cavo (modulo master + base) che via radio: con la possibilità di
utilizzare moduli radio di maggiore potenza (~1Km in aria libera) è
possibile prevedere coperture di un terriotorio con ampia superficie.

.. _software_utente_multipiattaforma:

Software utente multipiattaforma
''''''''''''''''''''''''''''''''

Il software che l'utente può utlizzare per la pubblicazione e
visualizzazione dei dati è multipiattaforma. Fatti salvi i moduli basati
su microcontrollore e vincolati all'ambiente Atmel e alcune funzioni sul
server di raccolta dei dati sviluppati in ambiente Linux (distribuzioni
CentOS e Fedora) la visulizzazione e il monitoraggio sono
multipiattaforma. Anche l'interfaccia utente grafica che permette la
geolocalizzazione, autenticazione e pubblicazione dei dati sia
automatica che manuale anche di dati rilevati manualmente e a vista è
multipiattaforma (attualmente testata su Linux, Android, ma portabile su
Windows, OS X, iOS

.. _salvataggio_locale_dei_dati:

Salvataggio locale dei dati
'''''''''''''''''''''''''''

I dati possono essere pubblicati in real time e/o salvati localmente. È
previsto un meccanismo di salvataggio dei dati su SD formattata FAT; i
file vengono frammentati a una dimensione prefissata per farne circa uno
al giorno e numerati da 000 a 999; i dati salvati hanno un flag che
indica se i dati sono stati già pubblicati correttamente su MQTT; i file
che devono essere controllati per possibili reinvii hanno postfisso .que
e quelli che hanno tutti i dati già inviati hanno postfisso .don

In questo modo si ottengono queste funzionalità:

-  salvataggio dati su SD almeno per due anni con campionamenti ogni 5s
-  reinvio automatico al server dei dati salvati ma non pubblicati
   correttamente sul server
-  ottimizzazione dei tempi in quanto solo i file che contengono dati da
   inviare vengono letti per selezionare i dati da reinviare
-  i dati possono essere riletti su un normale PC estraendo la SD

.. _messagistica_di_diagnostica:

Messagistica di diagnostica
'''''''''''''''''''''''''''

C'è la possibilità di ottenere una ampia messaggistica di diagnostica
per la soluzione dei problemi

Configurazione
''''''''''''''

Le versioni delle configurazioni vengono verificate e quando il firmware
non è retrocompatibile il modulo resta in attesa di una nuova
configurazione. Le configurazioni vengono subito verificate: non è
possibile configurare un modulo con dei sensori non corretti o non
funzionanti.

.. _modularità_hardware_e_software:

Modularità hardware e software
''''''''''''''''''''''''''''''

Le configurazioni hardware sono molteplici e possono essere utilizzate
differenti board; sono compatibili i moduli hardware maggiormente
diffusi e conosciuti dai makers oltre ad essere generalmente a basso
costo.

Crittografia
''''''''''''

Qualora il trasporto non sia considerato sicuro (via radio) viene
utilizzata la crittografia per garantire riservatezza e autenticità.

.. _integrazione_con_la_domotica:

Integrazione con la domotica
''''''''''''''''''''''''''''

Per quello che è stato possibile si è cercato di integrarsi con gli
standard della domotica (MQTT). Tutti i moduli possono essere utilizzati
anche da attuatori on/off (fino a 4 relay) ma è molto semplice
aggiungere altre funzionalità tramite remote procedure in formato json
su tutti i trasporti o tramite MQTT.

.. _concetti_base:

Concetti base
~~~~~~~~~~~~~

La modularità della stazione è stata ottenuta astraendo alcuni concetti
e funzioni e implementandoli nei differenti moduli hardware e software.

Trasporti
^^^^^^^^^

Il concetto di trasporto in Stima è simile ma non rigidamente aderente
ai concetti del modello ISO-OSI. Nel caso dei trasporti passivi il suo
compito è fornire un canale logico-affidabile di comunicazione
end-to-end per fornire servizi al soprastante livello che in Stima è
JsonRPC. Nel caso dei trasporti attivi corrisponde al protocollo
(Session Layer) per la pubblicazione dei dati su un server (broker).

.. _passivi_o_attivi:

Passivi o attivi
''''''''''''''''

In pratica i trasporti "passivi" permettono di eseguire procedure remote
codificate in formato json specifiche dell'implementazione Stima; quelli
attivi permettono la pubblicazione su server (broker) dei messaggi
aderenti allo standard R-MAP.

Passivi
.......

Seriale
+++++++

Collegamento punto a punto tramite porta seriale.

-  Principalmente per configurazione e debug
-  Piccole distanze via cavo

caratterizzato da:

-  Baud rate
-  Device

TCP/IP
++++++

Trasporto che utilizza il TCP/IP; i supporti fisici supportati sono:

-  ethernet: collegamenti tramite cavo ethernet a breve e media distanza
-  GSM/GPRS: installazioni con problemi per le cablature di
   alimentazione e collegamento di rete

caratterizzato da:

-  Name (Nome risolto dal DNS)
-  NTPserver

Bluetooth
+++++++++

caratterizzato da:

-  Bluetooth Name

NRF24
+++++

-  OSI Network Layer using nRF24L01(+) radios 2.4GHz ISM 50/150m in aria
   libera
-  Host Addressing. Each node has a logical address on the local
   network.
-  Message Forwarding. Messages can be sent from one node to any other,
   and this layer will get them there no matter how many hops it takes.
-  Ad-hoc Joining. A node can join a network without any changes to any
   existing nodes.

RF24Network Addressing and Topology

Each node must be assigned an 15-bit address by the administrator. This
address exactly describes the position of the node within the tree. The
address is an octal number. Each digit in the address represents a
position in the tree further from the base.

-  Node 00 is the base node.
-  Nodes 01-05 are nodes whose parent is the base.
-  Node 021 is the second child of node 01.
-  Node 0321 is the third child of node 021, an so on.
-  The largest node address is 05555, so 3,125 nodes are allowed on a
   single channel.

Alla libreria distributia è stata aggiunta la crittografia e
frammentazione e ricomposizione del payload

caratterizzato da:

-  Node (Node ID for RF24 Network)
-  Channel (Numero canale per RF24)
-  Key (AES key)
-  Iv

Attivi
......

MQTT
++++

MQTT (Message Queue Telemetry Transport) è un protocollo
publish/subscribe particolarmente leggero, adatto per la comunicazione
M2M tra dispositivi con poca memoria o potenza di calcolo e server o
message broker.

caratterizzato da:

-  Mqttsampletime (intervallo in secondi per la pubblicazione)
-  Mqttserver (MQTT server)
-  Mqttuser (MQTT user)
-  Mqttpassword (MQTT password)

AMQP
++++

AMQP (Advanced Message Queuing Protocol) è protocollo per comunicazioni
attraverso code di messaggi. Sono garantite l'interoperabilità, la
sicurezza, l'affidabilità, la persistenza.

caratterizzato da:

-  Amqpserver (Server AMQP)
-  Exchange (Nome dell'exchange remoto AMQP)
-  Queue (Nome della coda locale AMQP )
-  Amqpuser (User AMQP)
-  Amqppassword

JsonRPC
'''''''

La chiamata di procedure remote in formato json è l'unico metodo per
poter eseguire funzioni su un modulo dalla configurazione al
campionamento dei sensori.

La documentazione delle procedure remote è disponibile qui
`Gruppo_Meteo/RemoteProcedure <Gruppo_Meteo/RemoteProcedure>`__

.. _jsonrpc_over_different_transports:

JsonRPC over different transports
.................................

È possibile fare richiesta di una procedura remota che a sua volta
richiede una procedura remota; in questo modo è possibile utilizzare due
trasporti differenti e usare un modulo come gateway. Ad esempio il
modulo base non dispone al momento del trasporto radio RF24 ma puo'
richiedere a un modulo master tramite trasporto seriale o TCP/IP di
eseguire una procedura remota su un modulo satellite raggiungibile
tramite trasporto RF24. Queste funzionalità sono ampiamente da testare.

.. _elementi_hardware:

Elementi hardware
~~~~~~~~~~~~~~~~~

.. _board_microcontroller:

board microcontroller
^^^^^^^^^^^^^^^^^^^^^

.. _atmel_328p:

atmel 328p
''''''''''

Il più piccolo della serie può essere utilizzato per:

-  modulo i2c-gps
-  modulo i2c-wind
-  modulo i2c-rain

implementazioni on board:

-  arduino uno
-  arduino nano
-  microduino core

.. _atmel_644p:

atmel 644p
''''''''''

Il medio della serie può essere utilizzato per:

-  modulo satellite
-  modulo bluetooth
-  tutti i moduli relativi al 328p

implementazioni on board:

-  microduino core+ 644p

.. _altmel_mega_25601284p:

altmel mega 2560/1284p
''''''''''''''''''''''

Il grande della serie può essere utilizzato per:

-  modulo master
-  modulo GPS/GPRS
-  tutti i moduli relativi al 644p

implementazioni on board:

-  arduino mega2560
-  microduino core+ 1284p

.. _board_rtc:

board RTC
^^^^^^^^^

Il real time clock deve utilizzato quando non è possibile avere un'altra
sorgente affidabile per il tempo di riferimento e al tempo stesso la
pubblicazione dei dati può avvenire con tempo differito ad esempio
tramite la memorizzazione su scheda SD.

.. _board_radio_rf24:

board radio RF24
^^^^^^^^^^^^^^^^

È necessaria per supportare il trasporto NRF24

.. _board_ft232:

board ft232
^^^^^^^^^^^

È necessaria per programmare, debuggare, a volte configurare il modulo e
per supportare il trasporto seriale.

.. _board_enc28j60:

board ENC28J60
^^^^^^^^^^^^^^

Ethernet module a basso costo; è necessario lo stack tcp/ip software su
microcontrollore. Serve per supportare il trasporto TCP/IP. Alternativa
alla board wiznet

.. _board_wiznet_w5500:

board wiznet W5500
^^^^^^^^^^^^^^^^^^

Ethernet module completa dello stack tcp/ip. Serve per supportare il
trasporto TCP/IP. Alternativa alla board ENC28j60

.. _board_display_i2c_lcd_4_linee_20_caratteri:

board display I2C LCD 4 linee 20 caratteri
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Utilizzabile per visualizzare messaggistica di diagnostica e alcune
misure quando non è disponibile un PC per debug e altre visualizzazioni.

implementazioni on board:

-  YwRobot Arduino LCM1602 IIC V1

.. _board_5v_relay:

board 5V relay
^^^^^^^^^^^^^^

5V 4/2-Channel Relay interface board; Equipped with high-current relay,
AC 250V 10A / DC 30V 10A Opticalcoupler Protection Utilizzabile per
aggiungere a un modulo la funzionalità di attuatore. Ogni relay può
essere attivato singolarmente.

.. _board_sd:

board SD
^^^^^^^^

Microduino-SD aims to read and write data of a memory card. Utilizzata
per memorizzare i dati in loco; necessaria quando non ci siano trasporti
utili o la stabilità dei trasporti utilizzati è messa in dubbio e i dati
hanno valore anche in tempo differito.

.. _board_gsmgprs_sim800sim900:

board GSM/GPRS sim800/sim900
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Adopt SIM800L module to support four-band GSM/GPRS, whose working band
is：GSM850, EGSM900, DCS1800 and PCS1900MHz. Utilizzabile per avere il
trasporto TCP/IP quando non è disponibile una connessione ethernet.
Questo modulo può funzionare sul trasporto TCP/IP in due modalità: una
con delle get http tramutate dal server in publish MQTT e l'altra in una
vera connessione MQTT. E' possibile utilizzare questo modulo anche cone
Real Time Clock per ottenere una tempo di riferimento stabile. Si può
quindi ottenere dal server rmap il tempo di riferimento e impostarlo
nell'RTC di questo modulo per poi rileggerlo al bisogno in caso di non
disponibilità del trasporto TCP/IP; tutto questo a scapito di stabilità
e continuità di servizio. Nel caso sia importante avere un RTC
affidabile si consiglia l'aggiunta di un modulo RTC o ancora meglio del
modulo GPS.

.. _board_gps_neo_6m:

board GPS Neo 6M
^^^^^^^^^^^^^^^^

Questa board insieme a una board microcontrollore possono creare un
modulo i2c-gps. Il modulo i2c-gps fornisce a richiesta la posizione
(lat, lon, altezza) e il tempo di riferimento. Serve per istallazioni
mobili o che necessitano di un tempo di riferimento particolarmente
stabile.
