Manuale Utente
==============


Installazione del software per la configurazione
------------------------------------------------

Installare Rocky Linux 8.

Aggiunta repository e installazione pacchetti da utente amministratore
::

  dnf -y install epel-release
  dnf install yum-plugin-copr
  dnf copr enable simc/stable
  dnf copr enable pat1/rmap
  dnf config-manager --set-enabled powertools
  dnf groupinstall rmap


Uso dei tools a linea di comando
--------------------------------

Quando si utizzano i tool a linea di comando bisogna sempre
considerare che saranno presenti due database:

* database sul server RMAP (persistente)
* database locale (volatile)

Mentre il database sul server RMAP è per definizione persistente
quello locale se non già presente va creato utilizzando il comando::

  rmapctrl --syncdb
  
Successivamente sono disponibili due comandi per mantenere i due
database sincronizzati:

* upload configuration to server::

     rmap-configure --upload_to_server --station_slug="myslug" --user="myuser" --password="mypassword" --server=rmap.cc
  
* download station configuration from server::

     rmap-configure --download_from_server --station_slug="myslug" --user="myuser" --password="mypassword" --server=rmap.cc
  

Configurazione
--------------

Qui vengono descritte le fasi per effettuare la configurazione della stazione necessaria al suo funzionamento.

Creare un nuovo utente RMAP
...........................

Tramite interfaccia WEB
^^^^^^^^^^^^^^^^^^^^^^^

Per iscriversi alla piattarforma RMAP bisogna collegarsi al sito:
http://rmap.cc/

andare con il mouse sul menù "Il mio RMAP", sulla destra della barra
nera, e clickare su "Entra".

Apparirà una maschera che chiede utente e password.  Nella seconda
riga sotto a questa maschera clickare sul bottone blu "Registrazione"
e si verrà inoltrati automaticamente alla maschera di registrazione.

Per registrarsi bisognerà scegliere ed inviare le seguenti informazioni:

* username (una stringa lunga al massimo 9 caratteri che possono essere sia lettere che numeri);
* la propria e-mail
* la password (da inserire due volte per sicurezza). 

Quindi bisogna clickare sul quadratino per dichiarare di aver letto le
Condizioni di Servizio (descritte nel quadrato sotto riportato).
Completate queste operazioni si può procedere a clickare su "Invia".
Fatto questo il server RMAP invierà una mail di conferma all'indirizzo
indicato nella maschera di registrazione.  La registrazione verrà
conclusa aprendo il mail e confermando la propria intenzione di
iscriversi seguendo il link indicato.


Configurare una nuova stazione
..............................

A linea di comando
^^^^^^^^^^^^^^^^^^
Il tool a linea di comando da utilizzare per configurare le stazioni è::
  
  rmap-configure


Con modello (tipo e template sensori) predefinito
"""""""""""""""""""""""""""""""""""""""""""""""""

Eseguire l'apposito script a corredo per creare una nuova stazione.


Con modello (tipo e template sensori) non predefinito
"""""""""""""""""""""""""""""""""""""""""""""""""""""

In questa modalità bisogna avere piena padronanza del data model, dei sensori connessi e dei relativi metadati.

Qui un esempio di configurazione::

--> da scrivere


Trasferire la configurazione al datalogger
..........................................

* download station configuration from server::

    rmap-configure --download_from_server --station_slug="myslug" --user="myuser" --password="mypassword" --server=rmap.cc
  
Ecco il comando da impartire per trasferire e salvare la configurazione nel datalogger::

  rmap-configure --config_station --station_slug="myslug"  --board_slug=default --username="myuser"  --baudrate=115200 --device=/dev/ttyUSB0 --stima_version 4


Aggiornamento Firmware
----------------------

Tramite porta USB?
..................

Per l'aggiornamento del firmware è necessario avere a disposizione un file:

* FIRMWARE.BIN

che dorà risiedere nella cartella corrente da dove si eseguiranno i comandi.

Collegare il modulo tramite cavo USB e dovrà essere l'unico
dispositivo USB collegato in modalità seriale.

Per il modulo master e impartire il comando::

  --> da scrivere 


Tramite HTTPS dal server
........................

Caricare il firmware tramite il back end del server alla seguente URL::
  https://test.rmap.cc/admin/firmware_updater_stima/firmware/

Una volta caricato il firmware con successo sul server eseguire le RPC
over MQTT per impartire i comandi alle stazioni che necessitano
l'update del firmware descritte in questo manuale.
  
Recupero dati
-------------

I dati vengono salvati sul modulo master sulla SD card che deve
essere asportata dopo aver scollegato l'alimentazione.

Una volta recuperata la scheda Sdcard dal datalogger i dati possono
essere letti con apposito tool linea di comando.

Sono necessari almeno due file:

* AAAA_MM_GG.txt : AAAA = anno ; MM = mese ; GG = giorno
* info.dat  : metadati

Eseguire il seguente comando dalla stessa cartella contenete i file con i dati::

  mqtt2bufr -i -f AAAA_MM_GG.txt -a info.dat | bufr2mqtt -h rmap.cc -u "myusername" -P "mypassword"

dove:

* myuser: nome utente RMAP
* mypassword: password utente RMAP
  

Remote Procedure Call over MQTT
-------------------------------

Le remote procedure call permettono di far eseguire delle operazioni
dal datalogger da remoto.

Prima di eseguire qualsiasi RPC è necessario scaricare dal server la configurazione della stazione su cui si vuole agire::

  rmap-configure --download_from_server --station_slug="myslug" --user="myuser" --password="mypassword" --server=rmap.cc


dove:

* myuser: nome utente RMAP
* mypassword: password utente RMAP
* muslug: nome sintetico stazione
* mybslug: nome sintetico board
  

configure
.........

La configurazione da remoto è possibile tramite tool a linea di comando::

  rmap-configure --config_station --username="myuser" --station_slug="myslug" --board_slug="mybslug" --transport=mqtt --stima_version 4


admin
.....
  
Per impartire alla stazione dil comando per scaricare un nuovo firmware dal server eseguire il comando::
  
  rmap-configure --rpc_mqtt_admin --rpc_mqtt_admin_fdownload --username="myuser" --station_slug="myslug" --board_slug="mybslug" --stima_version 4

			  
reboot
......

Il reboot del modulo master è possibile tramite tool a linea di comando::

  rmap-configure --rpc_mqtt_reboot --username="myuser" --station_slug="myslug"  --board_slug="mybslug" --stima_version 4

Se si vuole che al reboot venga installato un nuovo firmware se disponibile::

  rmap-configure --rpc_mqtt_reboot -rpc_mqtt_reboot_fupdate --username="myuser" --station_slug="myslug"  --board_slug="mybslug" --stima_version 4
  

recovery
........

Il recupero dei dati salvati su SDcard è possibile tramite tool a linea di comando
specificando la data iniziale dei dati da recuperare (fino a data e ora corrente)::

  rmap-configure --rpc_mqtt_recovery --username="myuser" --station_slug="myslug"  --board_slug="mybslug" --datetimestart="2022-02-16T12:00" --stima_version 4


oppure specificando la data iniziale e finale dei dati da recuperare::

  rmap-configure --rpc_mqtt_recovery --datetimestart "2022-02-13T10:00" --datetimeend "2022-02-16T12:00" --username="myuser" --station_slug="myslug"  --board_slug="mybslug"  --stima_version 4
  

