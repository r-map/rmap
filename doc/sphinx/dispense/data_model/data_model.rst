
Data Model
==========

I modelli di dati sono più che progetti di database, definiscono
esplicitamente la struttura dei dati e aiutano nella progettazione di
sistemi informativi e scambi di dati.

* Lo scopo principale dei modelli di dati è quello di supportare lo
  sviluppo di sistemi informativi fornendo la definizione e il formato
  dei dati.

* Se questo viene fatto in modo coerente tra i vari sistemi, è
  possibile ottenere la compatibilità dei dati. Se si utilizzano le
  stesse strutture di dati per memorizzare e accedere ai dati, le
  diverse applicazioni possono condividere i dati.

* Purtroppo i sistemi e le interfacce spesso costano più del dovuto
  per la costruzione, il funzionamento e la manutenzione. Inoltre,
  possono limitare l'attività aziendale anziché supportarla. Una delle
  cause principali è la scarsa qualità dei modelli di dati
  implementati nei sistemi e nelle interfacce".

Conceptual data model
---------------------

Questi modelli sono tipicamente utilizzati per esplorare i concetti
di dominio con gli stakeholder del progetto.

Il Conceptual Data Model (Modello Concettuale dei Dati) è una
rappresentazione astratta e di alto livello dei dati di un sistema
informativo. Ha lo scopo di descrivere i concetti principali e le
relazioni tra di essi, senza entrare nei dettagli tecnici della loro
implementazione.

Caratteristiche del Conceptual Data Model:

* Alto livello di astrazione – Si concentra sulla struttura logica dei
  dati piuttosto che sugli aspetti fisici.
* Indipendenza dalla tecnologia – Non è legato a database specifici o
  a dettagli di implementazione.
* Focus su entità e relazioni – Rappresenta oggetti del mondo reale
  (entità), le loro caratteristiche (attributi) e i legami tra di loro
  (relazioni).
* Facile comprensione – Viene utilizzato per comunicare con
  stakeholder non tecnici, come analisti aziendali o clienti.

RMAP Conceptual data model
..........................

* Il modello è orientato all'applicazione (bisogna capire cosa sono i
  dati, normalizzarli e ricondurli a metadati stardard in fase di
  accoglienza), quindi si lavora pre e non post
* I dati sono legati ai metadati in modo univoco
* Una osservazione è univoca nello spazio dei suoi metadati
* L'unica possibilità di far coesistere due osservazioni dello stesso
  parametro nello stesso punto è atraverso il metadato “network”
  associabile alla classe dello strumento
* La tracciabilità di un sensore, una stazione, un osservatore nello
  spazio, nel tempo etc. Avviene attraverso il metadato “ident”
* Alcuni metadati sono table driven (level,timerange,network)
* Ogni dato può essere associato a un certo numero di attributi
* Nessuna dimensione è vincolata (intervalli temporali tra dati,
  numero attributi....)
* E' contemplata la gestione di previsioni; il datetime è sempre
  quello di verifica
* Misure e metadati hanno troncamenti sulle cifre significative
  dettati dalla loro possibilità reale di misura e stabiliti a priori
* Esistono due categorie di dato: una che varia tutti i metadati
  (osservazioni classiche) e l'altra che non prevede l'uso di alcuni
  metadati e che quindi sono da considerarsi come ulteriori metadati
  di quella singola stazione (constant station data: es. Nome
  stazione)
* Nessuno vieta di espandere i metadati estendendo esternamente questo
  data model

  
Logical data model (LDM)
------------------------

I LDM vengono utilizzati per esplorare i concetti del dominio e le
loro relazioni nel dominio del problema.

Il Logical Data Model (LDM) è una rappresentazione strutturata dei
dati di un'organizzazione o di un sistema, indipendente
dall'implementazione fisica su un database specifico. È un modello
concettuale dettagliato che descrive le entità, gli attributi e le
relazioni tra di esse.

Caratteristiche principali del Logical Data Model:

* Indipendenza dalla tecnologia – Non è legato a un particolare
  sistema di database (SQL, NoSQL, ecc.).
* Struttura dettagliata dei dati – Definisce chiaramente le entità,
  gli attributi e le relazioni.

Quando si usa un Logical Data Model?

* Nella fase di analisi e progettazione del database.
* Per comunicare la struttura dei dati tra analisti, sviluppatori e
  stakeholder.
* Per assicurarsi che i dati siano organizzati in modo logico e
  coerente prima della fase di implementazione.


RMAP Logical data model
.......................

* **METADATI**:

  * **Datetime**: tempo di fine misurazione
  * Ana: **Longitudine**, **latitudine** ed un **identificativo**
  * **network**: definisce stazioni con caratteristiche omogenee
    (classe degli strumenti)
  * **Time range**: Tr,P1,P2 indica osservazione o tempo previsione ed
    eventuale elaborazione “statistica”
  * **Level**: TL1,L1,TL2,L2 le coordinate verticali (eventualmente
    strato)
  * **Variable**: Btable parametro fisico

* **DATI**:

  * Valori rappresentabili come interi, reali, doppia precisione,
    stringhe

    * **Attributi** (alla stregua di dati)

Physical Data Model (Modello Fisico dei Dati)
---------------------------------------------

Scopo: Descrive come i dati vengono effettivamente archiviati nel
database (SQL, NoSQL, Cloud, ecc.).

Componenti: Strutture fisiche come indici, partizionamento, tipi di
dati specifici del database.

RMAP Physical Data Model
........................

Il livello fisico viene gestito tramite una libreria software con
delle API e tools: https://github.com/ARPA-SIMC/dballe

Viene anche gestito tramite formati di scambio dati descritti dalle
:ref:`RMAP RFC<formati-reference>`
