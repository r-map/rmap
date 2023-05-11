Platformio unit tests
*********************

Prerequisiti per l'esecuzione dei test
======================================

I moduli Master e Slave necessitano di un'alimentazione da 9-30V.

**Tutti i test elencati sono stati eseguiti nella data del 05/04/2023
presso la sede Digiteco con esito positivo.**

N.B. Per la replica dei test è necessario inserire i jumper delle schede
master e slave in modalità default come di seguito descritta.

Scheda Master
=============

master-sheet

+-----------------------------------+-----------------------------------+
| Identificatore                    | Descrizione                       |
+===================================+===================================+
| J1                                | Connettore Encoder                |
+-----------------------------------+-----------------------------------+
| J2                                | Connettore Programmatore MCU      |
+-----------------------------------+-----------------------------------+
| J3                                | Connettore Micro SD-Card          |
+-----------------------------------+-----------------------------------+
| J4                                | Connettore CAN BUS                |
+-----------------------------------+-----------------------------------+
| J5                                | Connettore EIA RS-232             |
+-----------------------------------+-----------------------------------+
| J6                                | Connettore USB-C                  |
+-----------------------------------+-----------------------------------+
| J7                                | Connettore Alimentazione board    |
+-----------------------------------+-----------------------------------+
| JP1 e JP2                         | Abilitazione terminatori di linea |
|                                   | CAN BUS - Default jumper inserito |
+-----------------------------------+-----------------------------------+
| JP3                               | Abilitazione UPIN27 o connettore  |
|                                   | EIA RS232 - Default jumper        |
|                                   | inserito sui pin 1-2 (UPIN27)     |
+-----------------------------------+-----------------------------------+
| JP4                               | Abilitazione LED DL1 di colore    |
|                                   | verde - Default jumper inserito   |
+-----------------------------------+-----------------------------------+
| BOOT1                             | Abilitazione BOOT                 |
+-----------------------------------+-----------------------------------+
| BATT1                             | Abilitazione batteria tampone -   |
|                                   | Default jumper inserito           |
+-----------------------------------+-----------------------------------+

Scheda Slave
============

slave-sheet

+-----------------------------------+-----------------------------------+
| Identificatore                    | Descrizione                       |
+===================================+===================================+
| J1                                | Connettore Programmatore MCU      |
+-----------------------------------+-----------------------------------+
| J2                                | Connettore I2C 12V                |
+-----------------------------------+-----------------------------------+
| J3                                | Connettore CAN BUS                |
+-----------------------------------+-----------------------------------+
| J4                                | Connettore EIA RS-232 3V3         |
+-----------------------------------+-----------------------------------+
| J5                                | Connettore EIA RS-232             |
+-----------------------------------+-----------------------------------+
| J6                                | Connettore SPI                    |
+-----------------------------------+-----------------------------------+
| J7                                | Connettore Ingressi digitali      |
+-----------------------------------+-----------------------------------+
| J8                                | Connettore Ingressi analogici     |
+-----------------------------------+-----------------------------------+
| J9                                | Connettore Alimentazione sensori  |
|                                   | 5V e 3V3                          |
+-----------------------------------+-----------------------------------+
| J10                               | Connettore Alimentazione sensori  |
|                                   | 12V                               |
+-----------------------------------+-----------------------------------+
| J12                               | Connettore Alimentazione board    |
+-----------------------------------+-----------------------------------+
| JP1                               | Impostazione della programmazione |
|                                   | - Default jumper inserito sui pin |
|                                   | 1-2                               |
+-----------------------------------+-----------------------------------+
| JP2                               | Abilitazione BOOT                 |
+-----------------------------------+-----------------------------------+
| JP3                               | Abilitazione batteria tampone -   |
|                                   | Default jumper inserito           |
+-----------------------------------+-----------------------------------+
| JP4 e JP5                         | Abilitazione terminatori di linea |
|                                   | CAN BUS - Default jumper inserito |
+-----------------------------------+-----------------------------------+
| JP6                               | Impostazione dell'alimentazione   |
|                                   | dell'I2C esteso e bufferizzato    |
|                                   | dal PCA9517D (5V o 3V3)           |
+-----------------------------------+-----------------------------------+
| JP7                               | Impostazione doppia alimentazione |
|                                   | del traslatore di livello         |
|                                   | TXU0204-Q1 (5V o 3V3)             |
+-----------------------------------+-----------------------------------+
| da JP8 a JP31                     | Impostazione del tipo di ingresso |
|                                   | del segnale analogico             |
+-----------------------------------+-----------------------------------+
| JP32                              | Abilitazione LED DL1 di colore    |
|                                   | verde - Default jumper inserito   |
+-----------------------------------+-----------------------------------+
| da JP33 a JP35                    | Abilitazione resistori da 1K5 in  |
|                                   | ingresso agli optoisolatori per   |
|                                   | utilizzare gli ingressi digitali  |
|                                   | a 5V                              |
+-----------------------------------+-----------------------------------+

Can hardware setup
==================

Percorso test: *rmap/platformio/stima_v4/test/master_can_setup*

Procedimento
------------

Eseguire tramite linea di comando: **pio run test**

Requisiti hardware
------------------

Modulo Master

Descrizione software
--------------------

Test sull'inizializzazione e avvio hardware del CAN BUS: 1. Impostazione
bit-rate CAN 2. Configurazione velocità e modalità BXCan 3. Avvio modulo
CAN 4. Abilitazione interrupts 5. Verifica delle sottoscrizioni messaggi
Canard

Il test effettua la programmazione delle apparecchiature hardware HAL
STM32 e il chip di alimentazione della linea CAN StimaV4. Viene inoltre
tramite l'utilizzo dei registri UAVCAN memorizzati su EEprom configurata
e programmata la velocità di linea CAN. Se la velocità non è corretta
viene impostata la velocità di Default di 1 Mhz e nuovamente
riprogrammata la sequenza.

Display LCD
===========

Percorso test: *rmap/platformio/stima_v4/test/master_lcd*

.. _procedimento-1:

Procedimento
------------

1. Eseguire tramite linea di comando: **pio run test**
2. Effettuare una pressione sull'encoder rotativo per attivare il
   display

.. _requisiti-hardware-1:

Requisiti hardware
------------------

Modulo Master

.. _descrizione-software-1:

Descrizione software
--------------------

Test sull'inizializzazione e avvio del task LCD con scrittura “Hello
World” sul display: effettua il test della libreria utilizzata con
semplici comandi e scritta HelloWorld. Viene utilizzata la libreria
U8gl2 opportunamente modifica per renderla funzionante. A partire dalla
libreria che utilizzava solamente l'I2C principale si è modificata per
renderla programmabile anche ad altri canali I2C e come parametro
nel'istanza principale è stato aggiunto il passagio del semaforo per la
gestione a basso livello del canale I2C nel sistema RTOS per rendere il
tutto più rapido possibile.

HTTP
====

Percorso test: *rmap/platformio/stima_v4/test/master_http*

.. _procedimento-2:

Procedimento
------------

Eseguire tramite linea di comando: **pio run test**

.. _requisiti-hardware-2:

Requisiti hardware
------------------

Modulo Master con il modulo GSM

.. _descrizione-software-2:

Descrizione software
--------------------

Richiede una fase precedente di connessione del modem GSM. 1.
Inizializzazione task e libreria Cyclone 2. Connessione http 3. Verifica
stringa avvio board 4. Ricezione rpc configurazione ricevuta 5.
Ricezione rpc richiesta reboot

Il test effettua la connessione https con parametri di default
preimpostati e avvia lo scaricamento della configurazione StimaV4.
Termina al raggiungimento corretto della prima linea di configurazione.
Viene utilizzata la libreria CycloneTCP opportunamente configurata nel
sistema. Il test è in conseguenza della connessione GSM che attiva il
collegamento http.

Master queue data
=================

Percorso test: *rmap/platformio/stima_v4/test/master_queue_data*

.. _procedimento-3:

Procedimento
------------

Eseguire tramite linea di comando: **pio run test**

.. _requisiti-hardware-3:

Requisiti hardware
------------------

Modulo Master con il modulo SDCard UPIN27 (necessario collegarlo
attraverso il modulo GSM)

.. _descrizione-software-3:

Descrizione software
--------------------

1. Inizializzazione SD card , creazione di file dati fittizzio e
   impostazione del puntatore ad un dato esistente/non esistente
2. Passaggio richiesta e risposta tramite coda gestita dal Supervisor
   task

Master o Slave memory
=====================

Percorso test Master: *rmap/platformio/stima_v4/test/master_memory*
Percorso test Slave: *rmap/platformio/stima_v4/test/slave_memory*

.. _procedimento-4:

Procedimento
------------

Eseguire tramite linea di comando: **pio run test**

.. _requisiti-hardware-4:

Requisiti hardware
------------------

Modulo Master o Slave

.. _descrizione-software-4:

Descrizione software
--------------------

Test sui task e librerie EEPROM e Flash con scrittura e lettura
automatica. 1. Scrittura di dati fittizi 2. Lettura e verifica dei dati
scritti in memoria

Modem GSM hardware setup
========================

Percorso test: *rmap/platformio/stima_v4/test/master_gsm*

.. _procedimento-5:

Procedimento
------------

Eseguire tramite linea di comando: **pio run test**

.. _requisiti-hardware-5:

Requisiti hardware
------------------

Modulo Master con il modulo GSM

.. _descrizione-software-5:

Descrizione software
--------------------

Test sul task con inizializzazione, accensione, connessione e
spegnimento modulo SIM7600E: effettua la connessione modem con parametri
di default preimpostati e avvia a partire dal task supervisor(quello che
gestisce la comunicazione) il collegamento al driver C++ SIMCOM7600. La
libreria avvia la programmazione del modulo e la gestione delle
alimenazioni. Power ON/OFF e programmazione tramite comandi AT. Al
termine della programmazione standard viene attivata la modalità rapida
a 960Kbaud ed effettuato il collegamento PPP. Successivamente viene
colegata tramite buffer e struttura la libreria CycloneTCP che rende
disponibili tutte le sue funzioni al collegamento trasparente PPP.

MQTT
====

Percorso test: *rmap/platformio/stima_v4/test/master_mqtt*

.. _procedimento-6:

Procedimento
------------

Eseguire tramite linea di comando: **pio run test**

.. _requisiti-hardware-6:

Requisiti hardware
------------------

Modulo Master con il modulo GSM

.. _descrizione-software-6:

Descrizione software
--------------------

Richiede una fase precedente di connessione del modem GSM. 1.
Inizializzazione task e libreria Cyclone 2. Connessione al server mqtt
3. Connessione al topic 4. Pubblicazione di una stringa fittizia

Il test effettua la connessione mqtt con parametri di default
preimpostati e avvia la pubblicazione e sottoscrizioneal server Mqtt
utilizzato per StimaV4. Termina alla pubblicazione di una linea di test.
Viene utilizzata la libreria CycloneTCP opportunamente configurata nel
sistema. Il test è in conseguenza della connessione GSM che attiva il
collegamento mqtt.

NTP
===

Percorso test: *rmap/platformio/stima_v4/test/master_ntp*

.. _procedimento-7:

Procedimento
------------

Eseguire tramite linea di comando: **pio run test**

.. _requisiti-hardware-7:

Requisiti hardware
------------------

Modulo Master con il modulo GSM

.. _descrizione-software-7:

Descrizione software
--------------------

Richiede una fase precedente di connessione del modem GSM.

Test sul task con inizializzazione, connessione e ricezione data e ora:
effettua la connessione ntp con parametri di default preimpostati e
avvia la sincronizzazione RTC e successiva programmazione dell'orologio
tramite libreria STM32Duino. Termina alla sincronizzazione RTC e
corretta programmazione e rilettura dell'orologio. Viene utilizzata la
libreria CycloneTCP opportunamente configurata nel sistema. Il test è in
conseguenza della connessione GSM che attiva il collegamento http.

RPC Test and Reboot
===================

Percorso test: *rmap/platformio/stima_v4/test/master_rpc_test_reboot*

.. _procedimento-8:

Procedimento
------------

1. Alimentare la board tramite le disposizioni elencate sopra
2. Connettere la board al PC mediante un cavo USB type-C e aprire il
   programma dedicato.
3. Eseguire tramite linea di comando: **pio run test**

.. _requisiti-hardware-8:

Requisiti hardware
------------------

Modulo Master

.. _descrizione-software-8:

Descrizione software
--------------------

Test della classe RPC con test e reboot rpc inviati da USB mediante un
programma dedicato: effettua la verifica di linee di comando usb-rs232
tramite connettore usb-c presente nel master. Il test attende tramite
USB che venga eseguia una RPC di esempio chiamata “RpcTest” o una
“Reboot” nelle modalità StimaV4. Termina alla ricezione di “RpcTest”. Un
segnale buzzer viene eseguito al “Reboot” in modo da verificare
l'effettivo riavvio del sistema sulla chiamata della RPC “Reboot”. Il
test verifica inoltre la catena di funzionamento USB_Serial e relativo
task di controllo e inoltro all'oggetto C++ RPC di StimaV4.

Slave low power
===============

Percorso test: *rmap/platformio/stima_v4/test/slave_lowpower*

.. _procedimento-9:

Procedimento
------------

1. Alimentare la board Master e caricare il firmware che si trova nel
   percorso: rmap/platformio/stima_v4/master
2. Alimentare la board Slave e caricare il firmware che si trova nel
   percorso: rmap/platformio/stima_v4/slave_th
3. Collegare le boards tramite CAN bus: connettere CANH e CANL del
   Master rispettivamente al CANH e CANL dello Slave. Una volta
   terminato il collegamento, lo Slave entrerà in modalità Tickless.
4. Eseguire tramite linea di comando: **pio run test**

.. _requisiti-hardware-9:

Requisiti hardware
------------------

Modulo Slave

.. _descrizione-software-9:

Descrizione software
--------------------

-  Entrata in modalità IDLE a FreeRTOS attivo senza soppressione tick
   automatico
-  Entrata in modalità Tickless 2 e risveglio dal LowPower Stop 2

Il test effettua la verifica della modalità LowPower di un modulo Slave.
Per il corretto funzionamento è necessario che sia presente il master
che invii allo slave il segnale di LowPower. Il modulo slave infatti è
concepito per entrare in modalità LowPower solo con comando da Master.
Il test evidenzia l'utilizzo delle funzionalità STM32Duino LowPower e la
sua integrazione con Tickless su LPTim1. Dopo la programmazione dei
timer e delle modalità LowPower, alla richiesta di LowPower dal Master
viene attivata la relativa modalità STOP2 e il test termina al risveglio
corretto della piena funzionalità.

Slave register
==============

Percorso test: *rmap/platformio/stima_v4/test/slave_register*

.. _procedimento-10:

Procedimento
------------

Eseguire tramite linea di comando: **pio run test**

.. _requisiti-hardware-10:

Requisiti hardware
------------------

Modulo Slave

.. _descrizione-software-10:

Descrizione software
--------------------

Test sull'inizializzazione, scrittura e lettura di un registro UAVCAN
memorizzato nella EEPROM: effettua la verifica della libreria UAVCAN
Register che permette la conservazione di registri UAVCAN nel modulo. La
gestione dei registri è stata integrata nella EEProm interna e le
chiamate di libreria leggono, scrivono inizializzano i registri in
modalità trasparente appogiandosi alla memoria EEProm di StimaV4.

Stack Fault Exception
=====================

Percorso test:
*rmap/platformio/stima_v4/test/master_stack_overflow_beep*

.. _procedimento-11:

Procedimento
------------

1. Il seguente test viene eseguito con il caricamento del firmware
   mediante il comando: pio run -e stimav4_master -t upload
2. Quando si raggiunge l'overflow dello stack, il cicalino emetterà un
   treno di impulsi che segnalano l'attivazione di un errore. Per una
   controverifica modificare in src/main.cppcommentandola riga di
   definizione del Supervisor task

.. code:: cpp

   static SupervisorTasksupervisor_task("SupervisorTask", 200, OS_TASK_PRIORITY_02, supervisorParam);

e decommentando la linea superiore che presenta la stessa definizione ma
con una dimensione maggiore di stack assegnata. Ciò permetterà di non
raggiungere l'overflow.

.. _requisiti-hardware-11:

Requisiti hardware
------------------

Modulo Master

.. _descrizione-software-11:

Descrizione software
--------------------

Test che simula un overlflow da parte dello stack: effettua la
dimostrazione del funzionamento delle funzioni di callBack inserite nel
modulo freertos_callback, che contengono tutte le chiamate agli errori
di sistema. In particolare viene ridotto lo stack di un task per
verificare e simulare un errore di memoria e verificarne tramite
attivazione del buzzere dell'avvenuta call_back nelle funzioni di
sistema.

Queue log with SD Card
======================

Percorso test: *rmap/platformio/stima_v4/test/master_sd_queue_log*

.. _procedimento-12:

Procedimento
------------

Eseguire tramite linea di comando: **pio run test**

.. _requisiti-hardware-12:

Requisiti hardware
------------------

Modulo Master con il modulo SDCard UPIN27 (necessario collegarlo
attraverso il modulo GSM)

.. _descrizione-software-12:

Descrizione software
--------------------

Test che simula un comando su coda per effettuare un logging generico
con scrittura dati log su SD card: effettua un semplice push tramitre
coda al task SD Card che effettua un LOG al posto del TRACE su RS232 su
SD Card per un logging, conforme al TRACE su RS232. Dopo il push, viene
riverificata l'effettiva scrittura su SD del comando di logging e così
viene verificata SD, sua programmazione, coda con passaggio dati.

Uavcan
======

Percorso test Master:
*rmap/platformio/stima_v4/test/nucleo_uavcan/uavcan_master_cpp* Percorso
test Slave:
*rmap/platformio/stima_v4/test/nucleo_uavcan/uavcan_slave_cpp*

.. _requisiti-hardware-13:

Requisiti hardware
------------------

-  STM32L496ZG (Master)
-  STM32L452RE (Slave)
-  2x MicroSD Card Adapter
-  2x SD-Card

Collegamenti necessari
----------------------

Connessione moduli con MicroSD Card Adapter
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

============== ====================
Master e Slave MicroSD Card Adapter
============== ====================
5V             VCC
GND            GND
PB6            CS
PB13           SCK
PB14           MISO
PB15           MOSI
============== ====================

Connessione moduli tramite CAN BUS
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

=========== ==========
Master      Slave
=========== ==========
CAN TX PA12 CAN RX PA7
CAN RX PA11 CAN TX PA6
=========== ==========

.. _procedimento-13:

Procedimento
------------

1. Caricare il firmware nello Slave tramite linea di comando pio run -e
   nucleo_l452re -t upload
2. Nel progetto del Master, decommentare il test da eseguire nel
   platformio.ini (Selezionare solo un tipo di test alla volta)
3. Eseguire tramite linea di comando: **pio run test**

Watchdog
========

Percorso test: *rmap/platformio/stima_v4/test/master_watchdog*

.. _procedimento-14:

Procedimento
------------

Eseguire tramite linea di comando: **pio run test**

.. _requisiti-hardware-14:

Requisiti hardware
------------------

Modulo Master

.. _descrizione-software-13:

Descrizione software
--------------------

Test del task Watchdog. Verifica refresh con funzione presente in ogni
task e blocco con esecuzione Wathdog: effettua la verifica del WatchDog
per stima V4. In tutti i task master e slave sono stati inseriti delle
funzioni per effettuare il WatchDog a livello locale. Un Task principale
di watchdog effettua un continuo controllo dei task che devono azzerare
il relativo flag di controllo o porlo in uno stato di Sleep o Suspend
nel caso il task sia necessariamente soppresso per un tempo piuttosto
lungo. Il WatchDog Task effettua il reset fisico del WatchDog Hardware
solo se tutti i flag sono azzerati. In questo modo è abbastanza semplice
capire se un task è in stallo e agire di conseguenza sul software per
correggere potenziali problemi. Il test avvia il tutto in modalità
normale, succesivamente passati 10 secondi blocca volutamente un task e
il WatchDog interviene segnalando il task non rispondente.
