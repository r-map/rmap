
Cosa si intende per Data Model?
===============================

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

* Tuttavia, i sistemi e le interfacce spesso costano più del dovuto
  per la costruzione, il funzionamento e la manutenzione. Inoltre,
  possono limitare l'attività aziendale anziché supportarla. Una delle
  cause principali è la scarsa qualità dei modelli di dati
  implementati nei sistemi e nelle interfacce".



Conceptual data model
---------------------

Questi modelli, talvolta chiamati modelli di dominio, sono tipicamente
utilizzati per esplorare i concetti di dominio con gli stakeholder del
progetto.

DB-All.e Conceptual data model
..............................

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

DB-All.e LDM
............

* **METADATI**:

  * Datetime: tempo di fine misurazione
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

