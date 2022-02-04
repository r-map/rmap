Manuale Utente
==============

Hardware
--------
Descrizione dell'hardware e delle sue principali funzioni.

Il datalogger è composto da moduli a loro volta composti board.
Una descrizione con alcune foto si trova a https://doc.rmap.cc/stima_v3/howto_stima_v3/howto_stima_v3.html e https://doc.rmap.cc/stima_v3/doxygen/index.html
a cui fare riferimento per maggiori dettagli.
I moduli si interconnettono tra loro tramite bus I2C.
Le board si interconnettono tramite interfaccia basata sullo standard UPIN-27.


funzioni svolte dalle Board
...........................

RTC
^^^
* determina la base dei tempi per il modulo master
* conserva data e ora anche in mancanza di alimentazione tramite la
  carica accumulata in un supercondensatore per non più di ~24h

Core+
^^^^^
Contiene il microcontrollore in queste due versioni:
* 8 MHz 3.3V
* 16 MHz 5V

USB
^^^
E' una interfaccia USB con la porta seriale del microcontrollore.
Permette di:
* caricare firmware
* effettuare debug

SDcard
^^^^^^
Permette di utilizzare una SDcard per effettuare:
* aggiornamento firmware
* salvataggio dati
* debug

I2C
^^^
Espone una 
* connessione e reset

GSM
^^^
* SIM
* antenna

HUB
^^^
* interconnessione
* alimentazione
* separazione BUS I2C e adattamento tensioni

Display
^^^^^^^
* Diagnostica


Funzioni software
-----------------

Accensione e supervisione (supervisor task)
...........................................

RTC task
........

Time task
.........

GSM task
........

Sensors reading task
....................

Data saving task
................

MQTT task
.........


Messa in opera
--------------

Connessione e disconnessione
............................

Impilamento board
^^^^^^^^^^^^^^^^^

Connessione moduli
^^^^^^^^^^^^^^^^^^

Regolazione Display
...................

Assemblaggio scatola stazione
.............................

SDcard
......

Formattazione
^^^^^^^^^^^^^

Aggiornamento Firmware
^^^^^^^^^^^^^^^^^^^^^^

Logging
^^^^^^^
(vedi sotto in diagnostica)

Recupero dati
^^^^^^^^^^^^^
(vedi sotto)

Configurazione
--------------

Recupero dati
-------------

Diagnostica
-----------

tramite DISPLAY
...............

Temporizzazioni
^^^^^^^^^^^^^^^

Segnale Radio GSM/GPRS
^^^^^^^^^^^^^^^^^^^^^^

Dati sensori
^^^^^^^^^^^^

Salvataggio e invio dati
^^^^^^^^^^^^^^^^^^^^^^^^

Messaggi di errore
^^^^^^^^^^^^^^^^^^

Tramite porta seriale
.....................

Tramite SDcard
..............
