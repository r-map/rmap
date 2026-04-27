.. _stimawifi_v3-reference:

Stima WiFi V3
=============

Introduzione
------------

Stima WiFi è una stazione di monitoraggio ambientale, uno strumento
che misura in modo continuativo, a intervalli regolari, alcuni
parametri chimici e fisici che caratterizzano l’inquinamento e le
proprietà dell’ambiente in cui viviamo.

La stazione Stima WiFi è un apparato pensato per il monitoraggio
ambientale non troppo complicato da assemblare e di costo
relativamento contenuto.  Il software e, nei limiti del possibile,
l’hardware utilizzato è stato selezionato per permettere la
condivisione di schemi di assemblaggio, firmware e software di
supporto sotto licenze libere.  Anche i dati raccolti sono
rilasciati con una licenza permissiva CC-BY 4.0

La stazione è stata progettata per essere generalmente installata in
ambiente urbano. I criteri per la sua installazione sono diversi e
meno stringenti da quelli da seguire per la messa in opera e l’uso di
stazioni meteorologiche di riferimento a standard dell'Organizzazione
Mondiale della Meteorologia ma bisogna comunque seguire alcuni criteri

.. image:: stimawifi.png

**Pavimenti bassi, soffitti alti e stanze spaziose**

Quando discuteva di tecnologie di supporto alla didattica Seymour
Papert sottolineava spesso l’importanza di "pavimenti bassi" e dei
"soffitti alti". Per essere efficace, affermava, una tecnologia deve
essere facile da apprendere ed esplorare per i principianti (pavimenti
bassi) ma, con il tempo, deve permettere di lavorare su progetti
sempre più sofisticati (soffitti alti).

Per un quadro più completo, bisogna aggiungere una dimensione in più
l’ampiezza dell’ambiente [di apprendimento] (stanze spaziose). Non
basta prevedere un percorso di apprendimento dal pavimento basso al
soffitto alto; dobbiamo offrire la possibilità di esplorare differenti
percorsi significativi.

[libera interpretazione del pensiero di] Mitchel Resnick

.. image:: stimawifi_open1.png
   :width: 50%

.. image:: stimawifi_open2.png
   :width: 50%
	   

Schema a blocchi
----------------

.. image:: stimawifi_blocchi.png

Caratteristiche del progetto:

* Precisione delle misure;
* Economicità dell’hardware
* Facilità di assemblaggio
* Possibilità di personalizzazione

Il progetto RMAP, di cui la stazione Stima WiFi fa parte, è in
continua evoluzione e così la stazione di monitoraggio ha vissuto
diverse incarnazioni, variando la sua conﬁgurazione in base
all’hardware via via disponibile.

BUS I2C
-------

Stima WiFi controlla i sensori via bus I2C.
http://www.i2c-bus.org/
https://www.nxp.com/docs/en/application-note/AN10216.pdf

* Protocollo seriale (sincrono)
* due sole linee comunicazione
  * SCL (Serial CLock) (sincronia)
  * SDA (Serial DAta)
* Single Master ~ slave // Multi-master
* Lento (100/400 kbit/s) 
* Supporta ﬁno a 127 device
* Ogni device ha indirizzo univoco (sul bus)

Semplice interazione basata su messaggi (Master invia richiesta ad
indirizzo device slave, slave device risponde solo se interpellato)


Componenti Hardware
-------------------

Elenco componenti elettroniche e sensori
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. image:: stimawifi_bom_mini.jpg
   :width: 50%

:download:`immagine ad alta risoluzione <stimawifi_bom.jpg>`.

Datalogger
..........

* S3 Mini V1.0.0 - LOLIN WiFi & Bluetooth 5 (LE) boards based ESP32-S3FH4R2
  https://it.aliexpress.com/item/1005006646247867.html?gatewayAdapt=glo2ita
  5.60€
* DC Power Shield V1.1.0 per LOLIN (WEMOS) D1 mini
  https://it.aliexpress.com/item/32790327733.html
  2,63€
* OLED 0.96 Shield V1.0.0 per LOLIN (WEMOS) D1 mini D32 0.96 "pollici 128X64 IIC I2C
  https://it.aliexpress.com/item/1005001804136025.html
  3,89€ 
* Shield connettore TFT I2C V1.1.0 per LOLIN (WEMOS) D1 mini 1x TFT, 2x I2C e 1x connettore IO utente
  https://it.aliexpress.com/item/32846977179.html
  1,69€
* Cavo I2C 100mm 10cm per cavo a doppia testa LOLIN (WEMOS) SH1.0 4P
  https://it.aliexpress.com/item/32867490848.html
  1,45€
* Connettori a pettine per moduli da impilare
  https://ie.farnell.com/samtec/esw-108-14-t-s/connector-rcpt-8pos-1row-2-54mm/dp/3699507
  2,3€ / unità
* Data Logger Shield DS1307 slot microSD Wemos per estensione D1 Mini
  https://www.ebay.it/itm/255283310856
  EUR 5,75
* SanDisk 128 GB Ultra microSDXC UHS-I scheda, con adattatore SD, fino a 140 MB/s, prestazioni dell'app A1, Classe 10, U1. Twin Pack
  https://www.amazon.it/dp/B0B7NVV55M
  16,82€ / unità
* 7X Duracell 1220 (7 Blister Da 1 Batteria) 7 Pile (CR1220)
  https://www.amazon.it/dp/B0C86NRTXV
  2,82€ /unità
* numero 15 PCB doppia faccia con spedizione e tasse
  https://jlcpcb.com
  9,54€
* Funmo 50 morsetti a vite PCB 2,54 mm 0,1" Pitch PCB Mount Screw Terminal Block, 2 pin/3 pin/4 pin, per cavi da 26-18AWG
  https://www.amazon.it/dp/B0CQ243JHC
  12,99€
* Alimentatore switching 12V / 1,5A – 18W
  https://futuranet.it/prodotto/alimentatore-switching-12v-15a-18w/
  8,55€
* Cavo prolunga DC maschio / DC femmina – 3 metri
  https://futuranet.it/prodotto/cavo-prolunga-dc-maschio-dc-femmina-3-metri/
  4,50€
* Scatola di derivazione Gewiss, Plastica Grigio, 150 x 110 x 70mm
  https://it.rs-online.com/web/p/scatole-di-derivazione/3038288
  13,97€
  
Sensori e cavi
..............

Particolato

* SENSIRION SPS30
  https://www.tme.eu/it/details/sps30/sensori-di-gas/sensirion/1-101638-10/
  48,8€
* Connettore ZHR-5 from JST Micro JST 5 pin 1,25 mm connettore 28AWG 20 cm
  https://www.ebay.it/itm/205681297852
  4,55€

CO2

* Sensirion SCD30
  https://www.tme.eu/it/details/scd30/sensori-dumidita/sensirion/1-101625-10/
  55€
* 10 set SH1.0  connettore femmina + maschio 4P spina con cavo 20 cm
  https://it.aliexpress.com/item/1005001649158434.html
  3,41€

Temperatura e Umidità

* Sensirion SHT85
  https://www.tme.eu/it/details/sht85/sensori-dumidita/sensirion/3-000-074/
  31€
* Morsettiera collegabile Harting Femmina a 4 vie, 1 fila, passo 1.27mm, Montaggio su cavo
  https://it.rs-online.com/web/p/morsettiere-innestabili/8164994
  4,65 €
* cavo dati numero di fili 4,  sezione nominale del conduttore 0.14, lunghezza 1m
  https://www.igus.it/product/CF240?artnr=CF240.01.04
  https://it.rs-online.com/web/p/cavi-di-comando/2100551
  2,34-3.0 EUR / m
  oppure
  Prolunga a 4 Pin Ebike,Cavo Spina Femmina e Maschio Connettore Impermeabile a 4 Pin per Luce Bicicletta Elettrica 1,5m
  https://www.amazon.it/dp/B0F4QWS1XJ
  11E una coppia

Schermo radiazioni per temperatura e umidità

* TFA Dostmann Copertura Protettiva Custodia Protettiva, Protegge dalle precipitazioni e dai Raggi solari, buona circolazione, Bianco
  https://www.amazon.it/dp/B017ILZF6C
  14,49€

in alternativa la versione delux
  
* TFA Dostmann Custodia protettiva per sensore esterno, 98.1111.02,
  custodia protettiva, copertura del trasmettitore di stazioni
  meteorologiche/termometro, protegge dalle precipitazioni
  https://www.amazon.it/dp/B0CK2RJ2BS
  35,99€

GPS per stazione mobile
.......................

* Waveshare LC76G Multi-GNSS Module Compatible with Raspberry PI, Supports GPS, BDS, GLONASS, Galileo, QZSS
  https://www.amazon.it/dp/B0C498QPS3
  24,99€

oppure:
  
* Reland Sun Modulo GPS micro USB NEO-6M NEO-7M NEO-8M (8M-A)
  https://www.amazon.it/dp/B09YCBYLBR
  11,63€
  
MCU - Espressif ESP32 S3
^^^^^^^^^^^^^^^^^^^^^^^^

.. image:: s3_mini_v1.0.0_1_16x16.jpg
   :width: 50%

* based ESP32-S3FH4R2
* 2.4 GHz Wi-Fi
* Bluetooth LE
* 4MB Flash
* 2MB PSRAM
* 27x IO
* 1x RGB LED (IO47)
* ADC, DAC, I2C, SPI, UART, USB OTG
* Compatible with MicroPython, Arduino and ESP-IDF

.. image:: s3_mini_v1.0.0_4_16x9.jpg
   :width: 50%
  
Questo lo schema elettrico:

.. image:: sch_s3_mini_v1.0.0.png
   :width: 50%


RTC and Microsd Data Logger Shield 
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Data logger shield per la memorizzazione dei dati tramite scheda
MicroSD e orologio in tempo reale integrato DS1307 con quarzo e
batteria in tampone.

.. image:: rtc-sd-shield.png
   :width: 50%

Devono essere fatte delle modifiche al modulo così come viene fornito
.....................................................................
Questo lo schema elettrico:

.. image:: chematic-Prints.png

Come discusso in questi articoli esiste di fatto un errore che porta
la batteria a scaricarsi in brevissimo tempo

* https://forum.arduino.cc/t/trying-to-understand-a-part-of-a-circuit-for-ds1307-rtc/610934/10
* https://sharedinventions.com/?p=663

Bisogna quindi rimuovere con il saldatore il diodo e la resistenza R5.

.. image:: Screen-Shot-2018-08-26-at-07.31.03a.png

che sono posizionate qui sul modulo:

.. image:: FPONC2XKMT6CU7Y.webp

Alla fine il modulo dovrà presentarsi così

.. image:: IMG_20180819_110150a.jpg
	   
OLED 0.96 Shield
^^^^^^^^^^^^^^^^

E' un display oled con risoluzione 128x64 pixels (0.96”).

.. image:: oled_0.96_v1.0.0_1_16x16.jpg
   :width: 50%

.. image:: oled_0.96_v1.0.0_2_16x16.jpg
   :width: 50%

- Schema elettrico :download:`pdf <sch_oled_0.96_v1.0.0.pdf>`

TFT I2C Connector Shield	   
^^^^^^^^^^^^^^^^^^^^^^^^

Per collegare il display e eventuali altri dispositivi è risultato
utile utilizzare questa board di connessione.

.. image:: tft_i2c_con_v1.1.0_1_16x16.jpg
   :width: 50%
	   
.. image:: tft_i2c_con_v1.1.0_2_16x16.jpg
   :width: 50%
    

Power Shield
^^^^^^^^^^^^

.. image:: power_shield.png
   :width: 50%

La scheda D1 mini può essere alimentata tramite connessione
usb (micro).
Per utilizzare la scheda lontano da un computer si può
utilizzare uno shield di alimentazione
Alla scheda di espansione possibile connettere un
alimentatore esterno ( da 7 a 24 V ).

  - Schema elettrico :download:`pdf <sch_dc_v1.1.0.pdf>`

Base Board
^^^^^^^^^^

Esporta le connessioni al bus I2C.

Può essere popolata con diversi tipi di connettori (passo 2.54mm)

.. image:: base1.jpg
   :width: 50%

Dei sensori in dotazione, quello per il particolato è alimentato a 5V,
gli altri componenti a 3.3v.

La base board permette di selezionare il voltaggio adatto al sensore
tramite un cavallotto.  Se il selettore non è popolato, la periferica
collegata, non riceve alimentazione.

.. image:: base2.jpg
   :width: 50%


Hardware alternativo compatibile (sconsigliato o obsoleto)
----------------------------------------------------------

MCU Shield
^^^^^^^^^^

C3 Mini V2.1.0 - LOLIN WIFI Bluetooth LE BLE Scheda IOT ESP32-C3FH4 ESP32-C3 4MB FLASH
https://it.aliexpress.com/item/1005006753245627.html
5,49€

.. image:: c3_mini_v2.1.0_1_16x16.jpg
   :width: 50%

* based ESP32-C3 WIFI & Bluetooth LE RISC-V Single-Core CPU
* Type-C USB
* 4MB Flash
* 1x WS2812B RGB LED
* Digital I/O Pins 12
* ADC, I2C, SPI, UART
* Compatible with LOLIN D1 mini shields
* Compatible with MicroPython, Arduino, CircuitPython and ESP-IDF
* Operating Voltage 3.3V
* Clock Speed 160MHz
* Flash 4M Bytes
* Size 34.3*25.4mm
* Weight 2.6g

La scheda di sviluppo, basata su esp32, è stata lanciata da Wemos,
come alternativa alle schede Arduino. Nella versione mini misura
35x26mm.

.. image:: c3_mini_v2.webp
   :width: 50%

Come con arduino, il modulo è espandibile con apposite schede di
espansione dette shield.

  - Schema elettrico :download:`pdf <sch_c3_mini_v2.1.0.pdf>`


Oled Shield
^^^^^^^^^^^

Questo è un display con un numero di pixel inferiore a queello
standard non più in commercio.  E' supportato per compatibilità con
vecchi hardware.

.. image:: oled_v2.1.0_1_16x16.jpg
   :width: 50%

Dimensione dello schermo: 64x48 pixel
VCC: 3.3V
Driver IC: SSD1306
Indirizzo I2C: 0x3C or 0x3D
Può essere utilizzato sia impilato sullo stack principale, sia
collegato alla base di espansione (in entrambi i casi via I2C bus)

Micro SD Card Shield
^^^^^^^^^^^^^^^^^^^^

Questo modulo può sostituire RTC and Microsd Data Logger Shield ma non
fornisce le funzionalità relative all'RTC in quanto non è presente.
Necessita anche di uno specifico firmware in quanto richiede una
specifica configurazione a tempo di compilazione.

.. image:: sd_v1.2.0_1_16x16.jpg
   :width: 50%

Micro SD(TF) card per D1 mini


Componenti elettroniche e sensori
---------------------------------

.. image:: stimawifi_out_of_box_mini.jpg
   :width: 50%

:download:`immagine ad alta risoluzione <stimawifi_out_of_box.jpg>`.

Questo lo schema di collegamento elettrico delle varie componenti.
La pila dei moduli Wemos è rappresentata in modo semplificato.

.. image:: ../../../../fritzing/stimawifi/v3/global_bb.png
   :width: 50%

.. image:: ../../../../fritzing/stimawifi/v3/global_schem.png
   :width: 50%


Scatola
-------

.. image:: scatola.png
   :width: 50%

Serve a proteggere l’elettronica ed alloggiare parte dei sensori.


Schermo solare
--------------

.. image:: schermo_solare.png
	   :width: 20%

Alloggiamento esterno alla stazione per sensore
umidità e temperatura

* Protegge il sensore da intemperie
* Protegge il sensore da radiazione solare diretta
* Evita surriscaldamento e migliora precisione
* Può essere autocostruito https://e.pavlin.si/2019/02/04/low-cost-solar-radiation-shield/
* Modelli molto economici già pronti	


Sensori
-------
  
Sensirion SPS30 (Sensore per le polveri sottili)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. image:: sps30.png
	   :width: 50%

* Tecnologia: Scatter Beam OPC (Optical Particulate Counter)
* VCC: 5V
* Indirizzo I2C: 0x69
* Data Sheet: https://sensirion.com/products/catalog/SPS30/

Note: Connettore a 5 poli.

Il quinto ﬁlo del connettore deve essere collegato a GND per
selezionare la modalità I2C (se lasciato non collegato il
sensore comunica con la modalità UART

.. image:: sps30_pinout.png
   :width: 50%


Sensirion SCD30 (Sensore CO 2 )
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. image:: scd30.png

* Tecnologia: NonDispersive InfraRed (NDIR)
* VCC: 3.3V ~ 5.5V
* Indirizzo I2C: 0x61
* Data Sheet: https://sensirion.com/products/catalog/SCD30/

.. image:: scd30_pinout.png
   :width: 30%


Sensirion SHT85 (Sensore Umidità & Temperatura)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. image:: sht85.png
   :width: 50%

* Tecnologia: Scatter Beam OPC (Optical Particulate Counter)
* VCC: 2.5V ~ 3.3V ~ 5V (Typical 3.3V)
* Indirizzo I2C: 0x44
* Data Sheet: https://sensirion.com/products/catalog/SHT85/

.. image:: sht85_pinout.png
   :width: 50%



Software
--------

FreeRtos viene utilizzato attraverso un wrapper C++:
https://github.com/michaelbecker/freertos-addons
https://michaelbecker.github.io/freertos-addons/cppdocs/html/namespacecpp__freertos.html
con qualche adattamento per ESP32.

Ogni thread ha una struttura dati utilizzata per comunicare trutture
dati e dati.  Nessun dato possibilmente è definito globalmente.

Il colore del led indica lo stato di funzionamento:

* spento: sconosciuto
* blu: in elaborazione
* verde : tutto funziona
* Rosso: almeno un errore è presente

Il display è opzionale e visualizza comunicazioni, ogni 3 secondi lo
stato aggiornato riassuntivo di funzionamento e un riassunto delle
ultime misurazioni effettuate. Sono supportati due differenti display
con dimensioni differenti riconosciuti automaticamente.

L'SD card è opzionale; se presente è utilizzata per memorizzare i dati
in sqlite3; la struttura del DB è qui rappresentata

.. image:: db_structure.png

Dopo essere passati dal DB sqlite i file vengono trasferiti in un 
archivio, integrato in modalità append.

Per poter utilizzare la stazione in modalità "mobile" ossia con
posizione continuamente aggiornata ci sono due possibilità:

* connettere un modulo GPS Waveshare LC76G Multi-GNSS Module
* utilizzare l'app android gpsdRelay

La configurazione è gestita sul server e i thread sono attivati
automaticamente. Quando la geolocalizzazione è possibile i dati
vengono generati, in caso contrario no.

E' attivo un web server accessibile quando ci si connette allo stesso Wifi
a cui è connessa la stazione, Sono forniti le seguenti URL/servizi:

* http://<station slug>             Full main page
* http://<station slug>/data.json   Data in formato json
* http://<station slug>/geo         Coordinate della statione
* http://<station slug>/archive.dat Dati dell'archivio da leggere con apposito tools
* http://<station slug>/info.dat    Info file dell'archivio da leggere con apposito tools

I dati sono visualizzabile da browser sempre se connessi allo stesso WiFi
autenticandosi sul server RMAP e accedendo alla propria pagina personale,
selezionando la stazione e poi alla voce "Mostra i dettagli stazione" e poi
"Dati locali in tempo reale".

Il reset delle configurazioni è effettuabile a stazione disalimentata
collegando a massa il pin RESET_PIN  o premendo il pulsante A della
board del display, alimentare la stazione e dopo 10 secondi scollegare il
RESET_PIN o rilasciare il pulsante. Il reset della configurazione effettua:

* rimozione delle configurazioni del wifi
* rimozione delle configurazioni stazione (utente slug password)
* completa formattazione dell'SD card con rimozione definitiva di 
  tutti i dati presenti

Il frusso dei dati nelle code è il seguente:

i dati e metadati sono generati da threadMeasure e accodati nella coda
mqttqueue per la pubblicazione, ricevuti da threadPublish per la 
pubblicazione sul broker MQTT; se non c'è spazio
vanno direttamente nella coda dbqueue per l'archiviazione su SD card.
threadMeasure è attivato periodicamente.

threadPublish prova la pubblicazione MQTT attigendo da due code:
mqttquque e recoveryqueue.  L prima ha priorità rispetto alla seconda,
quindi i messaggi ricevuti da recoveryqueue saranno pubblicati solo
quando non ci saranno messaggi nella coda mqttqueue.

Dopo ogni tentativo di pubblicazione al broker MQTT i dati vengono
accodati per l'archiviazione nella coda dbqueue etichettati
relativamente al risultato della pubblicazione, a meno che i messaggi
non siano già stati etichettati come pubblicati e ci si può
risparmiare il lavoro di riarchiviazione (sono quindi il risultato di
una richiesta di recovery).

Il thread threadDb gestisce due tipi di archiviazione dati.

Il primo (DB) che contiene gli ultimi dati misurati (solitamente 24 ore)
con sovrascrittura nel database e una etichetta a indicare lo stato di 
pubblicazione; fino a quando i dati sono presenti in questo
DB i dati possono essere recuperati per la pubblicazione fino al successo della
pubblicazione.

Quando i dati nel DB invecchiano oltre il limite vengono trasferiti nell'archivio
dove potranno essere riletti solo tramite un PC.

Ogni thread ha una struttura dati che descrive
lo stato di funzionamento. Il thread loop di arduino effettua una
sintesi degli stati di tutti i thread e li visualizza tramite i
colori del LED e tramite il display opzionale.

Per pubblicare e archiviare i dati è necessario avere un corretto timestamp.
Data e ora possono essere impostati tramite:

* NTP
* GPS
* UDP

Se presente un RTC locale (DS1307) data e ora sono impostate sull'RTC
automaticamente con uno dei metodi precedenti e poi se tutti i metodi
precedenti non sono più disponibili riletti dall'RTC.

Senza un corretto timestamp i dati non possono essere gestiti e
vengono subito ignorati.

Threads:

thread loop arduino
^^^^^^^^^^^^^^^^^^^

Questo thread esegue tutte le operazioni iniziali di configurazione e
attivazione degli altri thread. Prima si configura la connessione WiFi
insieme ad alcuni parametri univoci della stazione. Tramite questi
ultimi la configurazione stazione viene scaricata dal server. Il
thread governa la visualizzazione sul display e la colorazione del
LED. Inoltre è possibile visualizzare i dati misurati tramite un
browser indirizzandolo sulla pagina personale sul server RMAP.
La libreria TimeAlarm gestisce l'attivazione dei segnali ai
thread per attivazioni perioche.

threadMeasure
^^^^^^^^^^^^^

Questo thread si occupa di interrogare i sensori, associare i metadati
e accodarli per la pubblicazione e archiviazione. I sensori vengono
interrogati in parallelo tramite delle macchine a stati finiti.
Inoltre viene prodotta una struttura di dati di riassunto delle misure
effettuate. Insieme alla libreria di driver per sensori viene gestita
la loro inizializzazione e il restart in caso di ripetuti errori.

threadPublish
^^^^^^^^^^^^^

Pubblica i dati in MQTT secondo lo standard RMAP.  Se la
configurazione è per una stazione mobile della struttura con la
geolocalizzazione viene controllato il timestamp e se ancora attuale
associate le coordinate ai dati.  Periodicamente viene pubblicato
anche lo stato della diagnostica fatta dalla stazione; il server
provvederà a visualizzarne lo stato e a inviare delle email di
segnalazione al proprietario della stazione e a un apposito gruppo di
utenti di amministrazione.  Il thread una volta provata la
pubblicazione dei record sul broker MQTT provvederà se necessario

threadDb
^^^^^^^^

Archivia i dati su SD card. Il formato del DB è quello portabile di sqlite3 e
possono essere letti tramite la stessa libreria da PC. Più scritture
con gli stessi metadati aggiornano i dati, non creano record
duplicati. L'archivio invece è composto da due file, uno di descrizione
e il secondo con i dati.

Il thread threadDb viene attivato periodicamente
per recuperare l'invio dei dati presenti nel DB e non ancora pubblicati
inviando un piccolo blocco di dati a recoveryqueue fino a quando avanzi
sufficiente spazio nella coda di pubblicazione.

Il thread threadDb esegue a priorità più alta degli altri per garantire
l'archiviazione senza perdita di dati in tempi utili e non riempire le code.

I dati vengo continuamente traferiti dal DB all'archivio eliminando dal 
database i dati più vecchi trasferendoli in un semplice archivio su file
sempre sull'SD card. I dati in archivio possono essere letti e traferiti
sul server RMAP tramite mqtt2bufr, un tool della suite RMAP.

Se all'avvio i dati presenti nel DB risultano essere
tutti vecchi i dati vengono traferiti all'archivio e l'intero DB viene eliminato
e ricreato vuoto per limiti di memoria e performance.

threadUdp
^^^^^^^^^

Legge i dati UDP inviati dalla app gpsdRelay di uno smartphone
riempiendo una struttura dati con la geolocalizzazione e un timestamp.

threadGps
^^^^^^^^^

Legge i dati dal GPS se presente (porta seriale) riempiendo una struttura dati con
la geolocalizzazione e un timestamp.

threadGpsI2c
^^^^^^^^^^^^

E' una alternativa a threadGps in cui la comunicazione con il modulo
GPS avviene tramite BUS I2C.

La scelta tra threadGpsI2c e threadGps avviene al momento della
compilazione tramite il file di configurazione include/stimawifi_config.h


Remote Procedure Call
^^^^^^^^^^^^^^^^^^^^^

La stazione funge da Remote Procedure Call (RPC) server: è quindi in
grado di eseguire operazioni su ordine di un client che in questo caso
può essere il server RMAP istruito tramite interfaccia WEB o il tool
rmap-configure installato su proprio PC ad esempio con le opzioni:

* --rpc_mqtt_reboot  execute reboot RPC over MQTT
* --rpc_mqtt_pinout execute pinout RPC over MQTT
* --rpc_mqtt_pinout_P1 set status of Pin 1
* --rpc_mqtt_pinout_P2 set status of Pin 2
* --rpc_mqtt_recovery execute recovery data RPC over MQTT
* --datetimestart recovery data starting from this date and time in iso format (2011-11-04T12:05:23)
* --datetimeend recovery data ending to this date and time in iso format (2011-11-05T18:06:23)

Fare riferimento alla apposita documentazione per le possibili RPC e i
relativi parametri.
  
La stazione si iscrive con connessione persistente (clean session
false) a un topic MQTT da cui riceve i messaggi del client e pubblica
il risultato della RPC su un altro specifico topic. Al riavvio della
stazione gli eventuali messaggi accodati dal server per la stazione
vengono eliminati in quanto la coda di messaggi potrebbe essere
obsoleta.  Vengono invece gestiti i messaggi accodati dal server per
la stazione se questa è rimasta per un periodo disconnessa (ma
accesa).  I messaggi RPC vengono gestiti dal threadPublish. In alcuni
casi le operazioni richieste dalla RPC sono eseguibili dallo stesso
thread ma ad esempio l'RPC "recovery" deve essere eseguite da un
thread differente (threadDb). In questo ultimo caso le informazioni
per eseguire l'RPC vengono inviate tramite una apposita coda
"rpcrecoveryqueue"; è una coda binaria, ossia in grado mi gestire un solo
messaggio che sarà sempre l'ultimo. L'RPC recovery sarà quindi
eseguita dal threadDb; Il recupero dei dati selezionati per limite di
data e su richiesta tra quelli non ancora inviati viene gestito in
modo differente a seconda che risiedano sul DB di lavoro o nel file di
archivio.  Sul DB di lavoro i dati richiesti vengono reimpostati con
la flag "sent" a false, mentre dall'archivio vengono semplicemente
letti e filtrati i record senza poi comunque modificare lo stato della
flag "sent" presente in archivio.
Essendo la rilettura dell'archivio una operazione in alcuni casi
particolarmente lunga e non volendo creare un apposito thread è stata
creata una piccola macchina a stati finiti che continua ad accodare
quando possibile i record per la pubblicazione senza bloccare
l'esecuzione del thread per l'esecuzione delle operazioni ordinarie.
Il threadPublish analizzerà il messaggio di cui è stato chiesto l'invio:

* se non era già stato pubblicato viene reinviato al threadDb
* se era già stato pubblicato non viene reinviato al threadDb

I dati per la ritrasmissione (vedi RPC recovery) quindi presenti in
archivio vengono sempre inviati al thread per la pubblicazione con la
flag di "inviato" attiva per evitare duplicati in archivio


Assemblaggio datalogger
-----------------------

La prima fase della messa in opera presuppone l’assemblaggio del
data logger, la parte della stazione che si occupa di consultare
periodicamente i sensori installati e di inviare i campionamenti al
server centrale.

E' necessario utilizzare un saldatore a stagno per installare i
connettori a pettine necessari a collegare tra loro i vari componenti
ed assemblare i cavi di connessione.

Sono disponibili due tipi di connettori a pettine per impilare i
moduli: è consigliato l'uso di quelli di migliore qualità per i moduli
MCU ESP32 S3 e RTC and Microsd Data Logger e gli altri connettori per
gli altri moduli.

Una volta saldati i connettori su tutti i moduli facendo molta
attenzione al verso : la parte femmina del connettore va sempre verso
l'alto mantenendo i moduli posizionati in questo modo:

* modulo ESP32 con lato  MCU e connettore in alto
* modulo RTC and Microsd Data Logger con SDcard in alto e batteria in basso
* modulo Power con connettori in alto
* modulo TFT&I2C con connettori in alto

Procedere poi alla saldatura dei connettori sul modulo HUB secondo lo
schema in fotografia:

.. image:: hub_assemblata.jpg
   :width: 50%
  
I diversi moduli dovranno essere collegati impilati tra di loro
rispettando la polarità e rispettando anche l'ordine dal basso verso
l'alto riportato appena qui sopra.

.. image:: pila.jpg
   :width: 50%

Collegare la pila dei moduli all'HUB:

.. image:: pila_hub.jpg
   :width: 50%
	   
Strumentazione necessaria
^^^^^^^^^^^^^^^^^^^^^^^^^

* saldatore a stagno*

Dotazione Software
^^^^^^^^^^^^^^^^^^

* Nessuna

Compilazione firmware
---------------------

La stazione Stima WiFi è basata sul microcontrollore Esp32 prodotto
da ExpressIf. Si tratta di una soluzione economica e affidabile che da
qualche anno sta aumentando esponenzialmente la propria popolarità.
Il produttore mette a disposizione strumenti gratuiti e liberi per lo
sviluppo, e sono diffuse librerie ed ambienti di progettazione per i
maggiori linguaggi di programmazione.

Fatta salva la facoltà di usare la soluzione software con cui si ha
più confidenza, abbiamo selezionato, per la ridotta invasività, la
licenza di distribuzione e la disponibilità su tutti i maggiori
sistemi operativi, dell’ambiente di sviluppo integrato Platformio
disponibile all’Url https://www.platformio.org che è a sua volta
basato su ambiente di sviluppo Python.

Questa scelta, val la pena notarlo, non influisce in alcun modo sulle
dotazioni software adottabili successivamente per lo sviluppo di
programmi che interagiscano con la stazione di monitoraggio dopo la
sua installazione.

TODO clone repository; compilazione

Upload del firmware
^^^^^^^^^^^^^^^^^^^

Prima di procedere bisognerà controllare che tutti i collegamenti
fatti siano corretti perchè da ora i moduli saranno alimentati e
collegamenti errati possono cmportare oltre che a malfunzionamenti
anche la rottura irreversibile dei componenti.

Questa fase della messa in opera è necessaria per il funzionamento
della stazione.  Dopo la prima attivazione può rendersi nuovamente
necessario in caso si voglia modificare l’utilizzo della stazione,
personalizzarne le funzionalità o cogliere l’occasione di
impratichirsi con questa operazione fondamentale nel ciclo di vita del
software per microcontrollori,


TODO


L’apparizione sul piccolo schermo oled in dotazione della scritta
Starting Up! seguito dal numero di versione del firmware e, nella
schermata successiva dell’ESSID di configurazione della scheda,
STIMA-Config, e della password indicano che l’assemblaggio è stato
completato con successo.

Strumentazione necessaria
^^^^^^^^^^^^^^^^^^^^^^^^^

* computer

Dotazione Software
^^^^^^^^^^^^^^^^^^

Uso a linea di comando:

* Ambiente di sviluppo Python versione >= 3.11
* Platformio (Piattaforma per lo sviluppo embedded)

Ambiente grafico con IDE:

* VSCodium  https://vscodium.com/
* Pioarduino https://github.com/pioarduino/platform-espressif32?tab=readme-ov-file


Upload del firmware con esptool
-------------------------------

Questa fase della messa in opera è necessaria per il funzionamento
della stazione.  Dopo la prima attivazione può rendersi nuovamente
necessario in caso si voglia modificare l’utilizzo della stazione,
personalizzarne le funzionalità o cogliere l’occasione di
impratichirsi con questa operazione fondamentale nel ciclo di vita del
software per microcontrollori,

Salvare sul proprio COMPUTER tutti file contenuti in questa cartella
git di github:

https://github.com/r-map/rmap/tree/master/platformio/stima_v3/stimawifi/bin/lolin_s3_mini

dopo aver collegato al PC il datalogger tramite cavo USB impartire il comando:

::
   
   esptool --chip esp32s3 --port "/dev/ttyACM0" --baud 460800 --before default-reset --after hard-reset write-flash -z --flash-mode dio --flash-freq 80m --flash-size detect 0x0000 bootloader.bin 0x8000 partitions.bin 0xe000 boot_app0.bin 0x10000 firmware.bin

eventualmente sostituendo a ttyACM0 la porta seriale effettivamente in uso.

Dotazione Software
^^^^^^^^^^^^^^^^^^

WINDOWS

Provvedere innanzitutto al download del pacchetto di installazione di
Python (scegliere solo tra le versioni superiori alla 3.4) presso
questo link, dopodiché installare il pacchetto.  ATTENZIONE: è
assolutamente necessario che l’installazione di Python includa la voce
“Add Python to enviroment variables“. In caso non l’abbiate
selezionato durante l’installazione, è possibile correggere
rilanciando l’installer, selezionando “Modify” e, avanzando di uno
step, selezionare la voce “Add Python to enviroment variables“. NON
proseguire senza aver provveduto.

Effettuata l’installazione, riavviare il computer. A riavvio
completato, recarsi presso prompt dei comandi ed eseguire il seguente
comando:

::
   
   pip3 install esptool


MAC

Provvedere innanzitutto al download del pacchetto di installazione di
Python presso questo link, dopodiché installare il pacchetto e
riavviare al termine.

Recarsi poi presso terminale ed eseguire il seguente comando:

::
   
   python3 -m pip install esptool

   
Linux (Debian, Ubuntu)

Eseguire, da terminale i seguenti comandi:

::

   sudo apt install python3-setuptools git -y
   git clone https://github.com/themadinventor/esptool.git
   cd esptool
   sudo python3 setup.py install


Linux (Fedora)

Eseguire, da terminale i seguenti comandi:

::

   sudo dnf install python3-setuptools git -y
   git clone https://github.com/themadinventor/esptool.git
   cd esptool
   sudo python3 setup.py install

   
Strumentazione necessaria
^^^^^^^^^^^^^^^^^^^^^^^^^

* computer
* cavo USB C

   
Upload del firmware da android
------------------------------

Questa fase della messa in opera è necessaria per il funzionamento
della stazione. Dopo il primo caricamento i successivi aggiornamenti
potranno essere fatti in automatico dal server RMAP tramite WiFi.

Salvare sul proprio telefono tutti file contenuti in questa cartella
git di github:

https://github.com/r-map/rmap/tree/master/platformio/stima_v3/stimawifi/bin/lolin_s3_mini

Eseguire dopo averla installata l'app ESP32_Flash

e configurarla come indicato qui:

.. image:: esp32_flash.png
   :width: 50%

collegare l'adattatore OTG al telefono e il cavo USB collegandolo al
datalogger (modulo wemos ESP32-S3)

Avviare il microcontrollore ESP32 in modalità programmazione con la seguente procedura:

* premere e tenere premuto il pulsante O
* premere e rilasciare il pulsante RST
* rilasciare il pulsante O

premere il pulsante Flash sull'app  ESP32_Flash sul telefono.
attendere la conferma dell'avvenuta programmazione della MCU.
Scollegare il tutto.


Strumentazione necessaria
^^^^^^^^^^^^^^^^^^^^^^^^^

* smartphone con android
* cavo OTG usb C per smatphone

NOTA: La tecnologia USB OTG (o on-the-go) permette di leggere dati da
chiavette e altre memorie di massa USB anche da smartphone e tablet,
collegandoli direttamente alla porta microUSB / USB di tipo C del
dispositivo (per intenderci, quella che si utilizza per collegare il
caricabatterie)
  
NOTA: Non tutti i cavi usb sono uguali, in special modo quelli forniti
con gli smartphone. Alcuni sono adatti solo all’alimentazione ed alla
ricarica dei dispositivi e non permettono lo scambio di dati. Se il
computer non sembra riconoscere la stazione provare a sostituire il
cavo di connessione. Se anche questa prova non sortisce effetti, ma la
stazione si accende regolarmente, è probabile che il computer in uso
non riconosca l’interfaccia seriale usb usata dalla stazione. In
questo caso bisognerà caricare l’apposito driver prima di poter
procedere.

Dotazione Software
^^^^^^^^^^^^^^^^^^

* android app ESP32_Flash https://play.google.com/store/apps/details?id=com.esp_flash.esp_flash_app
  
  
Collegamento dei sensori e altri dispositivi
--------------------------------------------

Connettere i device necessari è semplice se fatto con attenzione:

- assicurarsi che la stazione non sia alimentata
- selezionare appropriato voltaggio alimentazione per ogni sensore
- assicurarsi che i collegamenti siano corretti (SCl -> SCl, SDA->SDA, GND -> GND, Vcc ->Vcc)
  
NOTE

- Alcuni device hanno più dei quattro pin necessari alla connessione
  al bus I2C
- VCC, Vcc, Vdd e VDD sono denominazioni equivalenti
	   
Prima di procedere con questa fase, disalimentare la stazione di
monitoraggio.

Per collegare i sensori al datalogger tramite l'HUB e verificarne il
funzionamento bisogna assemblare i cavi di collegamento secondo gli schemi
forniti dal produttore dei sensori facendo in modo che corrispondono
alla piedinatura dei connettori presenti sulla stazione Stima WiFi.

Collegamento modulo Display:

.. image:: cavo_display.jpg
   :width: 50%

Collegamento HUB per SPS30:

.. image:: cavo_sps30.jpg
   :width: 50%

Per il sensore SHT85 utilizzare il cavo di prolunga per Ebike
tagliandolo in modo asimmetrico a una lunghezza di 20 cm. dal lato
della femmina collegando sempre la parte più corta all'HUB:
	   
.. image:: cavo_sht85.jpg
   :width: 50%
	   
Complessivo collegamenti:

.. image:: assemblata.jpg
   :width: 50%

	   
Dopo aver messo a punto la cavetteria bisogna collegare i sensori
ognuno secondo lo standard facendo attenzione alla polarità ed al
voltaggio (il sensore di polveri sottili ha bisogno di essere
alimentato a 5v mentre gli altri sensori a 3,3v)

Collegamento SPS30: usare l'apposito cavo con connettore.

Collegamento SCD30: è preferibile non andare a saldare direttamente i
cavi sl PCB, ma utilizzare possibilmente una connessione con dupont
connectors 2.54 mm rispettando ordine e colori.

.. image:: collegamento_scd30.jpg
   :width: 50%

Per il collegamento dell'SHT85 usare l'apposito connettore a crimpare;
pelare il cavo solo per la guaina esterna, aprire il connettore per
l'accesso dei cavi rispettando con attenzione la colorazione e poi
premere con molta attenzione a crimpare:

.. image:: collegamento_sht85.jpg
   :width: 50%

Inserire il sensore nel connettore con il lato sensore come da
fotografia.
	   
La prima installazione ed il collaudo dei sensori è una fase critica,
errori possono rendere un sensore, la scheda o entrambi
inutilizzabili. Prima di alimentare ancora una volta la stazione, è
buona norma controllare la connessione con un multimetro che disponga
della modalità test di continuità.

Dopo le opportune verifiche bisogna collegare l’alimentazione esterna,
usando l’alimentatore esterno in dotazione, e verificare che la
stazione si avvii regolarmente.

Dovrebbe comparire sullo schermo un messaggio che invita a collegarsi
alla rete wireless attivata per le operazioni di configurazione
iniziale. Prima di procedere, però, è necessario censire la stazione
presso il server centrale.

NOTA: Anche se operano in condizioni ideali, i sensori di rilevamento
hanno, al netto di malfunzionamenti, una vita attesa non
illimitata. Si stima che passino circa due anni prima che i sensori,
in special modo quello per il particolato, comincino a perdere di
precisione.

Strumentazione necessaria
^^^^^^^^^^^^^^^^^^^^^^^^^

* Multimetro
* Computer, tablet o smartphone con connettività Wi-Fi

Dotazione Software
^^^^^^^^^^^^^^^^^^

* Un qualunque browser web
* Accesso alla rete Wi-Fi

Censimento stazione
-------------------

Prima di poter operare la stazione deve essere censita presso un
server di raccolta dati.

Esistono due punti di raccolta dati, test.rmap.cc ed rmap.cc. Il primo
viene utilizzato per le procedure di collaudo, per controllare il
funzionamento di prototipi di nuove stazioni e per mettere a punto
l’adozione di nuovi sensori; il secondo viene usato per la raccolta e
l’elaborazione dei dati sul campo.

Durante la fase di collaudo bisognerà registrare la stazione presso il
sito di test, una volta completata l’installazione bisognerà ripetere
la registrazione sul sito principare e censire nuovamente la
stazione. Non è prevista, al momento, una procedura automatizzata per
gestire la migrazione, da effettuarsi una tantum.

Prima di procedere al censimento vero e proprio, il gestore della
stazione deve registrare un nuovo utente, nel caso disponga già di un
profilo.

Una volta effettuato l’accesso al sito con nome utente e password,
sarà possibile censire una o più stazioni.

Censire una stazione consiste nel dichiararne le caratteristiche:  

* Coordinate
* Identificativo di stazione 
* Altezza dal livello del suolo
* Classificazione del sito dal punto di vista qualità dell’aria
* Alcune fotografie (5): una della stazione e 4 con le spalle alla
  stazione verso i 4 punti cardinali

L’identificativo di stazione non è altro che il nome che dovrà essere
usato in fase di configurazione iniziale.

Strumentazione necessaria
^^^^^^^^^^^^^^^^^^^^^^^^^

* Computer, tablet o smartphone

Dotazione Software
^^^^^^^^^^^^^^^^^^

* Un qualunque browser web


Collaudo Stazione
-----------------

Dopo aver censito la stazione è possibile configurarla. A sensori
collegati, si può accendere la stazione di monitoraggio.

Dovrebbe comparire sullo schermo un messaggio che invita a collegarsi
alla rete wireless attivata per le operazioni di configurazione
iniziale:

ssid: STIMA-config password: bellastima

L’access point è dotato di Capture Portal ma, se l’automatismo non
dovesse funzionare, è sempre possibile visitare con un browser l’url
http://192.168.4.1 per iniziare la procedura di configurazione.

Una volta raggiunta la pagina di configurazione, bisognerà inserire i
dati necessari alla connessione al sito rmap, l’url dell’istanza
prescelta e le credenziali per l’accesso alla rete Wi-Fi attraverso la
quale la stazione avrà accesso ad internet.

Se tutto andrà per il meglio e la stazione configurata correttamente,
sullo schermo cominceranno a scorrere le misure dei diversi sensori;
misure che saranno visibili, dopo un lasso di tempo, anche sul sito
preposto alla raccolta dei campionamenti.

NOTA: Non è previsto l’uso di proxy con autenticazione per accedere ad
internet. In caso l’istituto preveda questa modalità di navigazione
sarà necessario derogare in base al mac address della stazione o
creando una sottorete Wi-Fi dedicata.

Reset della configurazione
^^^^^^^^^^^^^^^^^^^^^^^^^^

Quando necessario, ad esempio per un cambio di configurazione
dell'access point wifi, è possibile procedere al reset delle
configurazioni tramite un apposito PIN.

Attenzione che questa procedura cancella tutti i dati presenti su
SDcard e ogni precedente configurazione.

A stazione non alimentata collegare il pin C di J5 dell'HUB a massa
(negativo) ad esempio con il pin di J8. poi alimentare la stazione.
E' possibile anche procedere a stazione alimentata collegando il pin C
di J5 dell'HUB a massa e procedendo a premere il pulsante RST del
modulo MCU EPS32.

Il display dovrebbe indicare le fasi del clear delle configurazione e
la formattazione dei supporti di memoria permanente.

Quando richiesto da display procedere a rimuovere il ponticello e
scollegare l'alimentazioen della stazione.

Strumentazione necessaria
^^^^^^^^^^^^^^^^^^^^^^^^^

* cavetto dupont femmina femmina


Dotazione Software
^^^^^^^^^^^^^^^^^^

* nessuna

Preparazione del guscio
-----------------------

Una volta completata l'assemblaggio, la configurazione ed il collaudo
della parte elettronica della stazione, bisognerà procedere ad
installarla, insieme ad alcuni sensori, all’interno del suo guscio
protettivo. Il sensore di temperatura, per non essere influenzato
nelle sue misurazioni dal funzionamento della stazione, viene
installato in un involucro separato denominato schermo solare passivo.

Deve essere divisa in due sezioni principali, una ospita i componenti
elettronici, l’altra (divisa a sua volta in due camere separate) il
sensore per le polveri sottili e quello per la rilevazione della
concentrazione di CO2

.. image:: scatola_interno.png
   :width: 50%

Nella parte alta della foto si nota l’alloggiamento
delle componenti elettroniche principali.

.. image:: scatola_elettronica.png
   :width: 50%

La parte bassa è divisa in due sezioni e queste sezioni sono aperte
verso l’esterno a differenza di quella superiore

.. image:: scatola_inferiore.png
   :width: 50%

I cavi per i sensori passano attraverso piccole incisioni del
polietilene per mantere il più possibile la camera superiore stagna


La ﬁnestra per il monitor è ricavata incollando un riquadro di
policarbonato con della colla a caldo.

.. image:: scatola_display.png
   :width: 30%


Con delle forbici o un taglierino, bisognerà tagliare da un foglio di
schiuma per imballaggi, che può essere riciclato, dei riquadri che
permettano separare l’interno della scatola di derivazione usata come
guscio della stazione, in tre compartimenti, uno per l’elettronica,
uno per l’ingresso dell’aria da analizzare e un altro alloggiamento
che permetterà a sensore di polveri sottili, che andrà installato a
cavallo delle due sezioni, di emettere l’aria analizzata senza
influenzare il flusso in ingresso.

Il foglio di schiuma andrà fissato alla scatola di derivazione con
nastro biadesivo o colla a caldo, a seconda se la parete debba essere
rimovibile, insieme ai sensori. Utilizzeremo un cacciavite per
praticare dei piccoli tagli nel foglio di schiuma per far passare i
cavi di collegamento dei sensori.  Per poter controllare lo schermo
della stazione dall’esterno, andrà rimosso un passacavi
laterale. L’apertura andrà chiusa con un piccolo, 4x4cm, riquadro in
plexiglas fissato con la colla a caldo all’interno della scatola di
derivazione.

Altri due passacavi laterali dovranno essere intagliati per permettere
l’ingresso di cavo di alimentazione e cavo dati del sensore di
temperatura.

Infine andranno rimossi i passacavi posti sul lato inferiore per permettere il ricircolo d’aria.

Strumentazione necessaria
^^^^^^^^^^^^^^^^^^^^^^^^^

* Forbici o taglierino
* Colla a caldo
* Nastro biadesivo
* Un foglio di Foam a celle chiuse (schiuma per imballaggi)
* Multimetro
* Cacciavite
* Plexiglas

Dotazione Software
^^^^^^^^^^^^^^^^^^

* Nessuna

  
Installazione in loco
---------------------

L’installazione della stazione prevede opere in muratura ed elettriche
che andrebbero eseguite da personale competente.

È possibile usare una staffa per parabola satellitare per ottenere un
economico sostegno che allontani la stazione dalla muratura
dell’edificio che la sosterrà dello spazio necessario ad una corretta
analisi del particolato. Alla staffa andranno fissati sia lo schermo
solare, più in basso, la stazione vera e propria. La stazione vinen
alimentata a non più di 12v e quindi i rischi di incidenti elettrici
sono inesistenti, a patto che l’allaccio alla presa elettrica sia
protetto dalle intemperie secondi gli standard vigenti.


Installazione schermo solare e sensore temperatura
--------------------------------------------------

.. image:: schermo.png

TODO
^^^^

Uso come stazione mobile
------------------------

Per poter utilizzare la stazione in modalità "mobile" ossia con
posizione continuamente aggiornata ci sono due possibilità:

* connettere un modulo GPS Waveshare LC76G Multi-GNSS Module
* utilizzare l'app android gpsdRelay

Durante il funzionamento è possibile monitorare lo stato di tutte le
componenti tramite il LED e il display: se il LED è di colore verde
e/o il diplay riporta "status: OK" allora tutte le funzioni sono
attive, data e ora disponibili, coordinate disponibili, sensori
funzionanti, pubblicazione e archiviazione dati funzionanti.  Il LED
diventa per un breve periodo blu per segnalare che alcune funzioni
periodiche sono attive e lo stato di funzionamento aggiornato.  Se il
LED assume altri colori deve essere affettuata una verifica perchè è
possibile che le misure non vengano effettuate/archiviate/pubblicate.
  
Configurazione
^^^^^^^^^^^^^^

Configurare la stazione come per stazione fissa con le seguenti eccezioni:

* non inserire Latitudine e Longitudine
* non inserire altezza stazione
* selezionare un modello stazione esplicitamente di tipo mobile
  (contenente "mobile" nel modello stazione)

Uso del modulo GPS
^^^^^^^^^^^^^^^^^^

https://www.waveshare.com/lc76g-gnss-module.htm
https://www.waveshare.com/wiki/LC76G_GNSS_Module

LEDs POWER

* Luminoso: Module is powered well.
* Spento: Module is not powered.

LED indicatore dello stato del GPS

* Lampeggiante Frequenza: 1 Hz: Successful position fix.
* Spento: No position fix.

Uso dell'app Android gpsdRelay
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

gpsdRelay effettua un Relay delle informazioni NMEA da Android alla
rete locale tramite pacchetti UDP .
https://f-droid.org/packages/io.github.project_kaat.gpsdrelay/
https://github.com/project-kaat/gpsdRelay

Come installare F-Droid
.......................

Per installare F-Droid, è necessario utilizzare il browser web dello
smartphone o del tablet e recarsi sulla pagina ufficiale
https://f-droid.org/it/ . Da lì, bisogna cliccare su “Scarica
F-Droid”.

Una volta scaricato non sarà possibile installarlo immediatamente, in
quanto Google previene l’installazione di app provenienti da fonti
sconosciute (come sistema di sicurezza), ovvero tutte quelle scaricate
al di fuori del Play Store. Per ovviare a ciò, bisogna:

Dalla home screen, tenere premuto sull’icona del browser web usato per
scaricare F-Droid e poi toccare su “Informazioni App” Scorrere
l’elenco fino all’opzione “Installa da fonti sconosciute” e abilitarla

A questo punto è sufficiente recarsi nella cartella download da un
file manager (oppure scaricare nuovamente F-Droid con il browser
web). Stavolta comparirà un pop-up che invita all’installazione.

Una volta installato, è necessario seguire la stessa procedura di
prima per abilitare F-Droid ad installare le app (nel mentre
consigliamo di disabilitare questa funzione per il browser web).

Da adesso è possibile scaricare qualsiasi app presente e ottenere
aggiornamenti automatici delle stesse quando gli sviluppatori li
rendono disponibili.

Perché F-Droid richiede di abilitare “Origini sconosciute” per
installare le app di default?
Perché una normale app Android non può fungere da gestore di pacchetti
autonomamente. Per farlo, avrebbe bisogno di privilegi di sistema
(vedi sotto), proprio come fa Google Play.

Posso evitare di abilitare “Origini sconosciute” installando F-Droid
come app di sistema privilegiata?
In passato era così, ma ora non lo è più. Ora è l'estensione
Privileged che deve essere inserita nel sistema. Può essere inclusa in
una ROM o installata tramite un file zip.

Come Installare gpsdRelay
.........................

Dall'app F-Droid cercare gpsdRelay e installarlo.

Come Usare gpsdRelay insieme a StimaWiFi
........................................

Settings:

.. image:: gpsdRelay_settings.jpg
   :width: 50%

bottone +   -> UDP server
ipv4     -> select broadcast
port     -> 8888
add

.. image:: gpsdRelay_addudp.jpg
   :width: 50%

attivare la riga configurata

premete il bottone con il triangolino

.. image:: gpsdRelay_principale.jpg
   :width: 50%


Calibrazione sensore CO2
------------------------

Sensori come SCD30 sono delicati sistemi ottici.  Bisogna prestare
particolare attenzione durante gestione dei sensori. Stress meccanici
sulla cavità ottica potrebbero modificare le proprietà fisiche del sensore
causando una precisione ridotta.  SCD30 dispone di due algoritmi di
calibrazione per ripristinare completamente precisione, nei casi in
cui lo stress meccanico vari le proprietà fisiche del sensore. Questa
procedura spiega come utilizzare la calibrazione in campo FRC (forced
re - calibrazione) per ripristinare la piena accuratezza del SCD30
  
Calibrate RPC
^^^^^^^^^^^^^

Portare la stazione in un ambiente con concentrazione nota di CO2 e
attendere che le misure si stabilizzino (almeno 2 minuti).

Dal sito rmap.cc ( o test.rmap.cc) autenticarsi con il proprio utente,
dalla propria pagina personale selezionare la stazione con il sensore
da calibrare e selezionare "Invia Remote Procedure Call"

Nel modulo inserire::

  Method: calibrate
  
  Params: {"type":"sdc30","value":430}

dove 430 dovrà essere sostituito dal valore di calibrazione della CO2 in ppm.

E' necessario monitorare lo stato di esecuzione delle RPC alla voce
"Monitor Remote Procedure Call" della stazione dallla propria pagina
personale sulla piattaforma RMAP.

Se Status risulta "completed" il valore fornito viene utilizzato per
aggiornare la calibrazione e ripristina la massima
accuratezza. L'ultimo valore di riferimento viene salvato in una
memoria non volatile sul sensore e persisterà fino a quando non viene
sovrascritto da un nuovo valore di riferimento che è fornito tramite
FRC.


Configurazione // Firmware + Software // Python + Json // NodeRed
-----------------------------------------------------------------

  - Versione pdf :download:`pdf <stimawifi_programming.pdf>`
  - Versione open documet :download:`odp <stimawifi_programming.odp>`


Appendice A Checklist installazione
-----------------------------------

Checklist installazione Stima WiFi Per mettere in opera una stazione
Stima WiFi bisogna tenere da conto tre fattori che permettono di farla
operare al meglio:

* L’accesso all’alimentazione di rete
* L’accesso ad un access point wifi
* Una corretta installazione per permettere ai diversi sensori di operare al meglio

Essendo un apparato che opera all’esterno, bisogna assicurarsi che la
corrente elettrica raggiunga il sito di installazione seguendo tutto
gli standard del caso e che la connessione possa sopportare le
intemperie. Questa attività esula dal mero assemblaggio della stazione
ed andrebbe demandata ad un tecnico specializzato.

La stazione dovrà essere alimentata continuamente, il consumo
elettrico è trascurabile, qualche decina di mA, ma non è possibile
togliere alimentazione alla stazione, ad esempio nei weekend o in
tarda serata, pena l’impossibilità di registrare e conferire i dati
raccolti.

Appendice B Accesso WiFi
------------------------

Le stazioni Stima WiFi, utilizzano una rete WiFi a 2.4Ghz* per collegarsi
ad internet e non possono utilizzare un proxy.  Prima di procedere con
l’installazione è bene confrontarsi con il personale dell’ufficio
tecnico per assicurarsi che la rete che si vuole utilizzare per la
connessione sia adatta

Annotare l’SSID della rete wifi a cui dovrà connettersi la stazione
Stabilire se l’accesso alla rete wifi è protetto da password.

Assicurarsi che la stazione possa accedere ad internet senza passare
per un proxy Solitamente, si può derogare al filtro dei dati,
indicando all’ufficio tecnico della scuola l’indirizzo MAC della
stazione La connessione WiFi deve essere assicurata 7/24

Una volta soddisfatti i prerequisiti, basta assicurarsi che la rete
WiFi copra con un segnale stabile il punto prescelto per
l’installazione.

Il RSSI (Indicatore della potenza del segnale ricevuto), indica
l'intensità del segnale. A seconda dei risultati delle nostre
misurazioni, conosceremo la qualità della nostra connessione:

* Ottimo segnale (tra -30 dBm e -50 dBm), perfetto per gaming o per
  riprodurre video 4K.
* Buon segnale (tra -50 dBm e -70 dBm): sufficiente per la navigazione
  e lo streaming video.
* Segnale normale (tra -70 dBm e -80 dBm), che potrebbero causare
  interruzioni o rallentamenti della velocità.
* Senale debole (più di -80 dBm): abbiamo un problema.

[*] Anche se più veloce, la rete a 5Ghz non garantisce, nel nostro
caso, migliorie significative rispetto a quella a 2.4Ghz e, di contro,
è molto più sensibile agli ostacoli che dovessero frapporsi tra
l’access point e la stazione.

Appendice C PIN
---------------

**Pin**

+-------------------+------+--------------+------+------------------------------+--------------+-------+------------------------------+--------------+-------+------------------------------+
| Stima use         | Pin  | ESP-8286     |      |                              | ESP32 C3     |       |                              | ESP32 S3     |       |                              |
+-------------------+------+--------------+------+------------------------------+--------------+-------+------------------------------+--------------+-------+------------------------------+
|                   |      | Pin          | name | Function                     | Pin          | name  | Function                     | Pin          | name  | Function                     |
+===================+======+==============+======+==============================+==============+=======+==============================+==============+=======+==============================+
| TXD/encA          | dx8  | TXD          | TX   | TXD                          | TXD  (21)    | TX    | TXD                          | TXD  (43)    | TX    | TXD                          |
+-------------------+------+--------------+------+------------------------------+--------------+-------+------------------------------+--------------+-------+------------------------------+
| RXD/encB          | dx7  | RXD          | RX   | RXD                          | RXD  (20)    | RX    | RXD                          | RXD  (44)    | RX    | RXD                          |
+-------------------+------+--------------+------+------------------------------+--------------+-------+------------------------------+--------------+-------+------------------------------+
| encBtn/CLEAR      | sx7  | A0           | A0   | Analog input, max 3.2V       | GPIO3        | D9/A3 | IO                           | GPIO2        | D9/A3 | IO                           |
+-------------------+------+--------------+------+------------------------------+--------------+-------+------------------------------+--------------+-------+------------------------------+
| RAIN/Analog input | sx6  | GPIO16       | D0   | IO                           | GPIO2        | D0/A2 | IO, Analog input, max 3.2V   | GPIO4        | D0/A2 | IO, Analog input, max 3.2V   |
+-------------------+------+--------------+------+------------------------------+--------------+-------+------------------------------+--------------+-------+------------------------------+
| SCL               | dx6  | GPIO5        | D1   | IO, SCL                      | GPIO10       | D1    | IO, SCL                      | GPIO36       | D1    | IO, SCL                      |
+-------------------+------+--------------+------+------------------------------+--------------+-------+------------------------------+--------------+-------+------------------------------+
| SDA               | dx5  | GPIO4        | D2   | IO, SDA                      | GPIO8        | D2    | IO, SDA                      | GPIO35       | D2    | IO, SDA                      |
+-------------------+------+--------------+------+------------------------------+--------------+-------+------------------------------+--------------+-------+------------------------------+
| RGB LED (V3)      | dx4  | GPIO0        | D3   | IO                           | GPIO7        | D3    | RGB_LED                      | GPIO18       | D3    | RGB_LED                      |
+-------------------+------+--------------+------+------------------------------+--------------+-------+------------------------------+--------------+-------+------------------------------+
| LED (V2) SS (V3)  | dx3  | GPIO2        | D4   | IO, 10k Pull-up, BUILTIN_LED | GPIO6        | D4    | IO                           | GPIO16       | D4    | IO                           |
+-------------------+------+--------------+------+------------------------------+--------------+-------+------------------------------+--------------+-------+------------------------------+
| SCK               | sx5  | GPIO14       | D5   | IO, SCK                      | GPIO1        | D5/A1 | IO, SCK                      | GPIO12       | D5/A1 | IO, SCK                      |
+-------------------+------+--------------+------+------------------------------+--------------+-------+------------------------------+--------------+-------+------------------------------+
| MISO              | sx4  | GPIO12       | D6   | IO, MISO                     | GPIO0        | D6/A0 | IO, MISO                     | GPIO13       | D6/A0 | IO, MISO                     |
+-------------------+------+--------------+------+------------------------------+--------------+-------+------------------------------+--------------+-------+------------------------------+
| MOSI              | sx3  | GPIO13       | D7   | IO, MOSI                     | GPIO4        | D7/A4 | IO, MOSI                     | GPIO11       | D7/A4 | IO, MOSI                     |
+-------------------+------+--------------+------+------------------------------+--------------+-------+------------------------------+--------------+-------+------------------------------+
| SS (V3 with RTC)  | sx2  | GPIO15       | D8   | IO, 10k Pull-down, SS        | GPIO5        | D8/A5 | IO, 10k Pull-down, SS        | GPIO10       | D8/A5 | IO, 10k Pull-down, SS        |
+-------------------+------+--------------+------+------------------------------+--------------+-------+------------------------------+--------------+-------+------------------------------+
| GND               | dx2  | GND          | G    | Ground                       | GND          | G     | Ground                       | GND          | G     | Ground                       |
+-------------------+------+--------------+------+------------------------------+--------------+-------+------------------------------+--------------+-------+------------------------------+
| 5v                | dx1  | \-           | 5V   | 5V                           | \-           | 5V    | 5V                           | \-           | 5V    | 5V                           |
+-------------------+------+--------------+------+------------------------------+--------------+-------+------------------------------+--------------+-------+------------------------------+
| 3.3V              | sx1  | 3.3V         | 3V3  | 3.3V                         | 3.3V         | 3V3   | 3.3V                         | 3.3V         | 3V3   | 3.3V                         |
+-------------------+------+--------------+------+------------------------------+--------------+-------+------------------------------+--------------+-------+------------------------------+
| RST               | sx8  | RST          | RST  | Reset                        | RST          | RST   | Reset                        | RST          | RST   | Reset                        |
+-------------------+------+--------------+------+------------------------------+--------------+-------+------------------------------+--------------+-------+------------------------------+

.. note:: All of the IO pins run at 3.3V.


Appendice D Indirizzi I2C
-------------------------

**Indirizzi I2C**

+------------+------+---------+------+-------+-------+-------+-------+-------+-------+---------+
|	DEC  |	HEX | Device  |	50   |	100  |	200  |	250  |	400  |	500  |  800  |  [KHz]  |
+============+======+=========+======+=======+=======+=======+=======+=======+=======+=========+
|	49   |	0x31| bottone |	V    |	V    |	V    |	V    |	V    |	V    |	V    |         |
+------------+------+---------+------+-------+-------+-------+-------+-------+-------+---------+
|	60   |	0x3C| display |	V    |	V    |	V    |	V    |	V    |	V    |	V    |         |
|	     |	    | 64X48   |	     |	     |	     |	     |	     |	     |	     |         |
+------------+------+---------+------+-------+-------+-------+-------+-------+-------+---------+
|	60   |	0x3D| display |	V    |	V    |	V    |	V    |	V    |	V    |	V    |         |
|	     |	    | 128X64  |	     |	     |	     |	     |	     |	     |	     |         |
+------------+------+---------+------+-------+-------+-------+-------+-------+-------+---------+
|	68   |	0x44|	SHT   |	V    |	V    |	V    |	V    |	V    |	V    |	V    |         |
+------------+------+---------+------+-------+-------+-------+-------+-------+-------+---------+
|	80   |	0x50|	GPS   |	V    |	V    |	V    |	.    |	.    |	.    |	.    |         |
+------------+------+---------+------+-------+-------+-------+-------+-------+-------+---------+
|	80   |	0x54|	GPS   |	V    |	V    |	V    |	.    |	.    |	.    |	.    |         |
+------------+------+---------+------+-------+-------+-------+-------+-------+-------+---------+
|	97   |	0x61|	SCD   |	V    |	V    |	V    |	V    |	V    |	V    |	V    |         |
+------------+------+---------+------+-------+-------+-------+-------+-------+-------+---------+
|	104  |	0x68|	RTC   |	V    |	V    |	V    |	V    |	V    |	.    |	.    |         |
+------------+------+---------+------+-------+-------+-------+-------+-------+-------+---------+
|	105  |	0x69|	SPS   |	V    |	V    |	V    |	V    |	V    |	V    |	V    |         |
+------------+------+---------+------+-------+-------+-------+-------+-------+-------+---------+

.. note:: 8 devices found

Appendice E I2C resistenze di pull up
-------------------------------------

**I2C resistenze di pull up**

+-----------------------+---------+---------+
|    Shield             | SDA     | SCL     |
+=======================+=========+=========+
| Oled Shield           | 4.7K    | 4.7K    |
+-----------------------+---------+---------+
| RTC and Microsd       |         |         |
| data Logger Shield    | 10K     | 10K     |
+-----------------------+---------+---------+
| MCU - Espressif       |         |         |
| ESP32 C3              | 10K     |  ---    |
+-----------------------+---------+---------+
| MCU - Espressif       |         |         |
| ESP32 S3              | ---     |  ---    |
+-----------------------+---------+---------+

+-----------------------+---------+---------+
|    Totale             | SDA     | SCL     |
+=======================+=========+=========+
| con ESP32 C3          | 2.4K    | 3.2K    |
+-----------------------+---------+---------+
| con ESP32 S3          | 3.2K    | 3.2K    |
+-----------------------+---------+---------+
