Stima WiFi V3
=============

* Citizen Science // Progetto RMAP // La Stazione STiMA
  
  - Versione pdf :download:`pdf <stimawifi_intro.pdf>`
  - Versione open documet :download:`odp <stimawifi_intro.odp>`

* Stazione Stima // Hardware // Sensori
  
  - Versione pdf :download:`pdf <stimawifi_hardware.pdf>`
  - Versione open documet :download:`odp <stimawifi_hardware.odp>`
    
* Configurazione // Firmware + Software // Python + Json // NodeRed
  
  - Versione pdf :download:`pdf <stimawifi_programming.pdf>`
  - Versione open documet :download:`odp <stimawifi_programming.odp>`


Introduzione
------------

Stima Wi-Fi è una stazione di monitoraggio ambientale, uno strumento
che misura in modo continuativo, a intervalli regolari, alcuni
parametri chimici e fisici che caratterizzano l’inquinamento e le
proprietà dell’ambiente in cui viviamo.

Non è una stazione di rilevamento meteorologico che segue i rigorosi
standard dell’organizzazione meteorologica mondiale. I criteri per la
sua installazione sono diversi e meno stringenti da quelli da seguire
per la messa in opera e l’uso di queste ultime ma bisogna comunque
prestare una certa attenzione nel suo assemblaggio. Non si tratta
solamente di un problema di nomenclatura, perché i parametri
analizzati, lo scopo ed i criteri di messa in opera sono molto
differenti.  Il kit di autocostruzione è progettato per poter essere
assemblato senza richiedere particolari capacità o competenze, giusto
un po’ di attenzione e manualità.

.. image:: stimawifi.png

Messa in opera della stazione
-----------------------------

La messa in opera della stazione può essere affrontata in più fasi,
dopo aver assemblato la scheda elettronica ed averla posizionata nel
proprio guscio bisognerà configurare la stazione, registrarla presso
il sito che raccoglierà i dati per una prima elaborazione ed
installare la stazione in una posizione individuata in precedenza.

In queste pagine tratteremo sommariamente queste operazioni
preoccupandoci di dare delle direttive di massima su cosa fare e su
materiali e strumenti necessari alla corretta esecuzione delle
procedure necessarie alla messa in opera.


Assemblaggio scheda elettronica
--------------------------------

La prima fase della messa in opera presuppone l’assemblaggio del
modulo di sistema, la parte della stazione che si occupa di consultare
periodicamente i sensori installati e di inviare i campionamenti al
server centrale.

A seconda del kit utilizzato, potrebbe essere necessario utilizzare un
saldatore a stagno per installare i connettori a pettine necessari a
collegare tra loro i vari componenti ed assemblare i cavi di
connessione (il progetto prevede che connettori e cavi siano
preassemblati ma nulla vieta, per chi avesse tra gli obiettivi di
migliorare la confidenza con la saldatura di componenti elettronici di
utilizzare il kit senza il servizio di saldatura).

I diversi moduli dovranno essere collegati tra di loro rispettando la
polarità.  Prima di procedere alla connessione dei sensori, bisognerà
controllare che il sistema funzioni attivandolo tramite l’alimentatore
in dotazione oppure collegandolo via usb ad un computer.

L’apparizione sul piccolo schermo oled in dotazione della scritta
Starting Up! seguito dal numero di versione del firmware e, nella
schermata successiva dell’ESSID di configurazione della scheda,
STIMA-Config, e della password indicano che l’assemblaggio è stato
completato con successo.

Una volta completata con successo il primo avvio, visto che la scheda
può essere alimentata in due modi diversi, tramite connettore micro
usb e tramite alimentatore 12v, bisogna togliere l’alimentazione e
provare il metodo ad alimentare la stazione verificando anche la
modalità di alimentazione che non si è ancora utilizzata.


Strumentazione necessaria
^^^^^^^^^^^^^^^^^^^^^^^^^

cavo micro usb
computer/Alimentatore per smartphone
saldatore a stagno*

Dotazione Software
^^^^^^^^^^^^^^^^^^

Nessuna


Caricamento firmware
--------------------

Questa fase della messa in opera è facoltativa. La stazione arriva già
con un firmware installato pronta per la configurazione iniziale ma,
in caso si voglia modificare l’utilizzo della stazione,
personalizzarne le funzionalità o cogliere l’occasione di
impratichirsi con questa operazione fondamentale nel ciclo di vita del
software per microcontrollori,

La stazione Stima è basata sul microcontrollore Esp8266 prodotto da
ExpressIf. Si tratta di una soluzione economica e affidabile che da
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

NOTA: Non tutti i cavi usb sono uguali, in special modo quelli forniti
con gli smartphone. Alcuni sono adatti solo all’alimentazione ed alla
ricarica dei dispositivi e non permettono lo scambio di dati. Se il
computer non sembra riconoscere la stazione provare a sostituire il
cavo di connessione. Se anche questa prova non sortisce effetti, ma la
stazione si accende regolarmente, è probabile che il computer in uso
non riconosca l’interfaccia seriale usb usata dalla stazione. In
questo caso bisognerà caricare l’apposito driver prima di poter
procedere.


Strumentazione necessaria
^^^^^^^^^^^^^^^^^^^^^^^^^

cavo micro usb
computer

Dotazione Software
^^^^^^^^^^^^^^^^^^

Ambiente di sviluppo Python
Platformio (Piattaforma per lo sviluppo embedded)


Collegamento dei sensori
------------------------

.. video:: stimawifi_v3_board_e_pila.mp4
   :width: 100%
	
Prima di procedere con questa fase, disalimentare la stazione di
monitoraggio.

Una volta assemblata e configurata la scheda madre della stazione, è
necessario collegare i sensori alla scheda e verificarne il
funzionamento.

Per farlo bisogna assemblare i cavi di collegamento secondo gli schemi
forniti dal produttore dei sensori facendo in modo che corrispondono
alla piedinatura dei connettori presenti sulla stazione Stima.

.. image:: board.jpg

Dopo aver messo a punto la cavetteria bisogna collegare i sensori
ognuno secondo lo standard facendo attenzione alla polarità ed al
voltaggio (il sensore di polveri sottili ha bisogno di essere
alimentato a 5v mentre gli altri sensori a 3,3v)

La prima installazione ed il collaudo dei sensori è una fase critica,
errori possono rendere un sensore, la scheda o entrambi
inutilizzabili. Prima di alimentare ancora una volta la stazione, è
buona norma controllare la connessione con un multimetro che disponga
della modalità test di continuità.  Dopo le opportune verifiche
bisogna collegare l’alimentazione esterna, usando l’alimentatore
esterno in dotazione, e verificare che la stazione si avvii
regolarmente.

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

Multimetro
Computer, tablet o smartphone con connettività Wi-Fi

Dotazione Software
^^^^^^^^^^^^^^^^^^

Un qualunque browser web
Accesso alla rete Wi-Fi


Censimento stazione
-------------------

Prima di poter operare, a meno che non sia stata dotata di un firmware
specifico, la stazione deve essere censita presso un server di
raccolta dati.

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

Computer, tablet o smartphone

Dotazione Software
^^^^^^^^^^^^^^^^^^

Un qualunque browser web


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
configurazioni effettuando un reset con l'apposito ponticello mentre il ponticello di riconfigurazione è cortocirduitato.
Questo video mostra una modalità per procedere alla riconfigurazione.

.. video:: stimawifi_v3_reset.mp4
   :width: 100%
	   
Strumentazione necessaria
^^^^^^^^^^^^^^^^^^^^^^^^^

Computer, tablet o smartphone con connettività Wi-Fi


Dotazione Software
^^^^^^^^^^^^^^^^^^

Un qualunque browser web
Accesso alla rete Wi-Fi

Preparazione del guscio
-----------------------

Una volta completata l'assemblaggio, la configurazione ed il collaudo
della parte elettronica della stazione, bisognerà procedere ad
installarla, insieme ad alcuni sensori, all’interno del suo guscio
protettivo. Il sensore di temperatura, per non essere influenzato
nelle sue misurazioni dal funzionamento della stazione, viene
installato in un involucro separato denominato schermo solare passivo.

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

Forbici o taglierino
Colla a caldo
Nastro biadesivo
Un foglio di Foam a celle chiuse (schiuma per imballaggi)
Multimetro
Cacciavite
Plexiglas


Dotazione Software
^^^^^^^^^^^^^^^^^^

Nessuna

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

Ancora da scrivere
^^^^^^^^^^^^^^^^^^


Appendice A
-----------

Checklist installazione Stima wifi
Per mettere in opera una stazione stima bisogna tenere da conto tre fattori che permettono di farla operare al meglio: 

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

Accesso WiFi 
------------

Le stazioni Stima, utilizzano una rete WiFi a 2.4Ghz* per collegarsi
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

[*] Anche se più veloce, la rete a 5Ghz non garantisce, nel nostro
caso, migliorie significative rispetto a quella a 2.4Ghz e, di contro,
è molto più sensibile agli ostacoli che dovessero frapporsi tra
l’access point e la stazione.

