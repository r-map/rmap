Manuale software
****************

StimaV4 logica di funzionamento software
========================================

Introduzione
------------

La realizzazione degli applicativi Master e Slave (open source) di stima
V4, è stata effettuata utilizzato il framework ARDUINO sulla piattaforma
HW STM32 (STM32Duino). Il sistema utilizzato si basa su sistema
operativo in tempo reale FreeRTOS particolarmente diffuso sui
microcontrollori e piccoli microprocessori, anch’esso distribuito sotto
la licenza open source MIT. In generale tutte le librerie utilizzate nel
progetto sono utilizzate e distribuite sotto questa tipologia di
licenza.

La struttura del progetto è organizzata in directory (driver) e (task),
facilmente comprensibili per suddividere le funzionalità di base. Nella
sottodirectory driver saranno presenti i moduli che consentono l’accesso
ai dispositivi HW dell’architettura StimaV4, mentre in task sono
contenuti i veri e propri task di funzionamento sviluppati in C++ usando
il wrapper sviluppato da Michael Becker
https://michaelbecker.github.io/freertos-addons/ in grado di integrare
in una classe C++ le funzionalità FreeRTOS nel singolo modulo e dare
un’organizzazione chiara al software sviluppato.

Per quanto riguarda lo sviluppo in generale anche dove non sono
utilizzate le classi C++ del wrapper di Michael Becker tutte le
funzionalità sono state comunque organizzate in classi. L’accesso alle
librerie come ad. esempio la libreria Canard che consente l’utilizzo
della comunicazione CANBus (metodo di collegamento utilizzato tra i
moduli master e slave del progetto) disponibili nelle sole versioni C,
sono state interfacciate come classi C++ mediante la realizzazione di
una classe pilota appoggiata alle funzioni C della libreria.

Particolare attenzione è stata fatta all’utilizzo delle variabili
nell’applicativo. Per rendere il tutto più leggibile e avere quindi
anche un sistema più semplice da manutenere nel tempo sono state
eliminate tutte le variabili globali (ad eccezione di quelle utilizzate
ad Arduino su HW ed istanze dei suoi moduli). Tutte le variabili di
StimaV4 devono essere dichiarate nel main program e passate come
argomenti ai vari task di utilizzo, con i metodi disponibili.

Il presente documento ha lo scopo di descrivere il funzionamento logico
e la struttura del software, all’interno dei moduli sono presenti
abbondanti commenti per comprendere le metodologie di sviluppo e
l’utilizzo delle singole funzioni e/o variabili di definizione. Non
verranno documentate le librerie utilizzate, come ad.es. la libreria
Canard sopracitata dove è presente in rete documentazione specifica,
consultabile su https://opencyphal.org/ che descrive ampiamente
funzionalità e configurazione delle stesse. Lo stesso vale per le altre
librerie presenti ArduinoJsonRPC, Cyclone ecc…

Drivers
-------

All’interno della cartella drivers sarà sempre presente un file
module_master_hal.cpp o module_slave_hal.cpp (master/slave) che
sostanzialmente è la configurazione dell’interfaccia HW verso le
librerie arduino. In questo modulo sono inizializzate le periferiche HW,
e/o l’assegnamento dei PIN/Funzionalità che ci permettono di utilizzare
le funzioni arduino come ad. Es SPI, I2C, UART ecc… Nel modulo sono
redirezionati i PIN come da esempio sottostante, concedendo alle
periferiche di base (se abilitate) con la struttura ARDUINO la reale
configurazione hardware.

::

   #ifdef HAL_UART_MODULE_ENABLED

   constPinMapPinMap_UART_TX[] = {

       {PA_0, UART4, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF8_UART4)},

       {PB_6, USART1, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF7_USART1)},

       {PD_5, USART2, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF7_USART2)},

       {NC, NP, 0}

   };

Sono in oltre presenti chiamate di inizializzazione di basso livello per
la configurazione dei clock di sistema e dei vari dispositivi utilizzati
e relative configurazioni per i clock sorgenti. Ogni dispositivo ha un
clock sorgente specifico opportunamente dimensionato. La modifica di
questi parametri seppur possibile può rendere il sistema instabile o
inibire l’utilizzo di qualche periferica. La riduzione del clock di
sistema ad intervalli diversi da quelli impostati necessità il ricalcolo
e la modifica delle selezioni dei corretti divisori/moltiplicatori per
mantenere i clock dei dispositivi nei range di validità.

::

   extern "C" void SystemClock_Config(void)

   {

       // Configure the main internal regulator output voltage

       if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
       {
           Error_Handler();
       }

       // Configure LSE Drive Capability

       HAL_PWR_EnableBkUpAccess();
       __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

       // Initializes the RCC Oscillators according to the specified parameters
       RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_LSE
                                 |RCC_OSCILLATORTYPE_MSI;

       RCC_OscInitStruct.LSEState = RCC_LSE_ON;

       RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;

       RCC_OscInitStruct.MSIState = RCC_MSI_ON;

       RCC_OscInitStruct.MSICalibrationValue = 0;

       RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;

       RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;

       RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;

       RCC_OscInitStruct.PLL.PLLM = 1;

       RCC_OscInitStruct.PLL.PLLN = 40;

       RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;

       RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;

       RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;

       if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
       {
           Error_Handler();
       }
     ...

   }

Queste invece le chiamate per la configurazione dei dispositivi HW di
basso livello, inizializzazione pin e altre attività richiamate dal
framework verso le librerie STM di basso livello per la configurazione
HW.

::


   void SetupSystemPeripheral(void)    // INIT delle periferiche basso livello ed interrupt Error_Handler

   void MX_GPIO_Init(void)             // INIT dei PIN di sistema

   void MX_CAN1_Init(void)             // INIT CanBus (richiamato dalle funzioni HAL_CAN_Init di libreria)

   void MX_QUADSPI_Init(void)          // INIT QSPI (richiamato dalle funzioni HAL_QSPI_Init di libreria)

   ...

Questo modulo piuttosto specifico viene richiamato da STM32Duino quando
viene inizializzata e/o utilizzata una periferica tra quelle disponibili
HW. In linea generale e possibile modificare i clock di funzionamento
per ottenere un risparmio energetico o per avere efficienza al massimo
su un dispositivo e/o modificare la funzionalità di un PIN. La modifica
di questo modulo può comunque portare ad un’instabilità del sistema in
quanto il tutto è stato già progettato per ottenere una perfetta
sinergia tra l’HW di StimaV4, STM32Duino e sistema operativo RTOS.
Nell’eventualità di uno sviluppo di una nuova scheda HW, con la sola
modifica di questo file adattato alla nuova interfaccia HW si dovrebbe
ottenere un sistema ancora funzionante.

Altri file sempre presenti come eeprom e flash, rendono disponibile allo
sviluppatore le funzionalità base che permettono l’accesso in lettura e
scrittura a tali dispositivi. Nel main program sono definite le
variabili di classe che istanziano i suddetti dispositivo HW e li
rendono disponibili a tutti i moduli che ne hanno necessita
semplicemente con il passaggio dell’indirizzo della classe nel wrapper
del task.

::

   // Init access Flash istance object

   staticFlashmemFlash(&hqspi);

   ...

   #if (ENABLE_CAN)

        // TASK CAN PARAM CONFIG

        static CanParam_t canParam = {0};

        canParam.configuration = &configuration;

        ...

        canParam.flash = &memFlash;

   #endif

   ...

   #if (ENABLE_CAN)

        static CanTask can_task("CanTask", 7300, OS_TASK_PRIORITY_02, canParam);

   #endif

Nell’esempio sopra nel main program è definita la variabile memFlash
come istanza della classe Flash (presente in drivers), viene inserita
nella struttura canParam (parametri Can) e successivamente passata al
Task can_task che si occuperà della gestione della comunicazione Can BUS
nel sistema.

La classe **Flash** , come la classe **Eeprom** sono a questo punto
disponibili ai vari moduli, ma essendo in un contesto RTOS che può
condividere i dispositivi tra i vari task, per ogni periferica non
esclusiva è definito un semaforo di utilizzo per rendere le operazioni
indivisibili (disponibilità, utilizzo e rilascio) tra loro e garantire
il perfetto funzionamento dei dispositivi.

::

   // Hardware Semaphore

   #if (ENABLE_I2C1)

        wireLock = newBinarySemaphore(true);

   #endif

   #if (ENABLE_I2C2)

        wire2Lock = newBinarySemaphore(true);

   #endif

   ......

   #if (ENABLE_QSPI)

        qspiLock = newBinarySemaphore(true);

   #endif

        rtcLock = newBinarySemaphore(true);

Nei moduli slave sono presenti ulteriori classi relativamente al loro
utilizzo relative all’accelerometro e al modulo MPPT (LTC4015). Come per
i moduli Eeprom e Flash si tratta di classi C++ che danno l’accesso ai
dispositivi HW nel main program e/o nel relativo task di utilizzo.

**Freertos_CallBack.c** è una raccolta di chiamate HW di call_back
relativi ad eccezioni e/o chiamate RTOS HW di sistema. All’interno del
modulo sono state posizionate le chiamate per lo sleep power_down di
sistema e tutte le eccezioni quali bus_fault, error_handler,
memManageecc… che vengono richiamate a basso livello nella gestione di
un eccezione STM32. Inoltre sono presenti le funzionalità di call_back
del FreeRTOS per permettere di gestire il comportamento del sistema ed
eventuale ripristino da un errore.

Ogni funzione base del freeRTOS è stata reedirezionata all’interno di
questo modulo per semplificare la gestione dell’applicativo. Per quanto
riguarda il LowPower è stata collegata nella funzione xTaskSleepPrivate,
automaticamente richiamata dal FreeRTOS quando tutti i task sono in fase
di sospensione per le modalità è il tempo minimo configurato in
STM32Freertosconfig.h e STM32Freertosconfig_extra.h. Per uleriori
approfondimenti sulla configurazione del sistema operativo riferirsi
alle guide freertos disponibili su https://freertos.org. Per quanto
riguarda StimaV4 la configurazione impostata permette il powerDown sia
in modalità normale che in modalità Tickless, definita nel prossimo
paragrafo.

::

   // Prepara il sistema allo Sleep ( OFF Circuirterie ed entrata in PowerDown, utilizzando libreria LowPower di STM32Duino )

   extern "C" void xTaskSleepPrivate(TickType_t *xExpectedIdleTime) {

     #if (LOWPOWER_MODE==SLEEP_IDLE)

       LowPower.idle(*xExpectedIdleTime);

     #elif (LOWPOWER_MODE==SLEEP_LOWPOWER)

       LowPower.sleep(*xExpectedIdleTime - 10);

     #elif (LOWPOWER_MODE==SLEEP_STOP2)

       LowPower.deepSleep(*xExpectedIdleTime - 10);

     #else

     *xExpectedIdleTime = 0;

     #endif

   }

   extern "C" void xTaskWakeUpPrivate(TickType_t *xExpectedIdleTime) {
     ... eventuale codice di WakeUP ...
   }

   ...

   // Hard fault con segnale acustico

   extern "C" void hard_fault_isr() {

     #if(DEBUG_MODE)

     faultStimaV4(4);      // Buzzer di StimaV4 se abilitato il DEBUG_MODE

     #else

     NVIC_SystemReset();  // Reboot in condizioni normali

     #endif
   }

   ...

**Freertos_LPTim.c** invece permette l’utilizzo dei timer LowPower STM32
(qua è necessaria una conoscenza approfondita del sistema STM32 e della
gestione LowPower). In sostanza questo modulo si preoccupa di impostare
come gestione del Tick di sistema (temporizzatore delle funzioni RTOS)
al timer LowPower in modalità **TickLess**.

I timer LowPower a differenza dei timer “normali” opportunamente
programmati proseguono la loro operatività anche se la CPU è posta in
stato di STOP (bassissimo consumo). Con questa metodologia è stato
programmato il timer LowPower per gestire le attività di sistema ed
aggiornare il “Tick” anche quando i task possono essere sospesi per
permettere un risparmio energetico sostanziale. In questo modo tutte le
sorgenti di clock vengono fermate (risparmio energetico) ma la
temporizzazione RTOS rimane sempre sincronizzata anche dopo la
sospensione del sistema.

Il modulo lptim.c è stato opportunamente modificato ed adattato al
sistema RTOS ed arduino in modo da permettere alle sue funzioni base
come per esempio la classica chiamata millis() che restituisce i
millisecondi trascorsi dall’avvio del programma fino al momento della
sua chiamata e renderla perfettamente disponibile con valori reali anche
dopo le chiamate Sleep che ne interromperebbero l’incremento. In
sostanza si è agito sui contatori LPTim per controllare il tempo reale
di standBy ed aggiornare in proporzione il timer di sistema reale.
Questo ha permesso l’utilizzo trasparente al sistema RTOS e alle
funzioni disponibili dalle librerie STM32Duino LowPower e RTC.

Nel file di definizioni STM32FreertosCoinfig_Extra.h (che definisce le
opzioni EXTRA se utilizzate nel FreeRTOS) è possibile utilizzare o meno
questa funzionalità in maniera del tutto automatica
configUSE_TICKLESS_IDLE.

::

   #define_USE_FREERTOS_LOW_POWER 1

   // FreertosTickless Mode (LOW_POWER_PRIVATE EnableLptimTick.c)

   #defineconfigUSE_TICKLESS_IDLE LOW_POWER_PRIVATE_LPTIMx_TICK

All’interno di Freertos_lptimTick.c in automatico il modulo ridefinisce
le funzionalità vPortSuppressTicksAndSleep(TickType_t xExpectedIdleTime)
che attivano la richiamata tramite macro valla funzione
xTaskWakeUpPrivate che fisicamente fa entrare la CPU in modalità di
basso consumo (quella selezionata in configurazione)

::


   // Inclusione del modulo se abilitata la modalità TickLess del FreeRTOS

   #if ( !defined(configUSE_TICKLESS_IDLE) || configUSE_TICKLESS_IDLE != 2 )

   #warning Please edit FreeRTOSConfig.h to define configUSE_TICKLESS_IDLE as 2 *or* exclude this file.

   #else

   #ifdef xPortSysTickHandler

   #warning Please edit FreeRTOSConfig.h to eliminate the preprocessor definition for xPortSysTickHandler.

   #endif

Se il modulo è attivato sostanzialmente viene attivato l’interrupt
relativo al timer LPTim utilizzato (uno tra i disponibili anch’esso
selezionabile nel file di configurazione) programmandone l’intervallo
sulla base dei tempi configurata e lo rende il tick di sistema primario,
inibendo la richiamata ad **osSystickHandler** che nel FreeRTOS di
STM32Duino incrementa il tick autonomamente. Inoltre la funzione è stata
adattata per registrare nel momento dell’attivazione delle modalità
powerDown e del successivo WakeUp il conteggio dei tick reali di
powerDown, verificando il contenuto dei registri LPTim che come sappiamo
continuano la loro attività anche se la CPU è in modalità STOP. Al
momento del risveglio i tick di sistema vengono aggiornati in
proporzione al tempo di powerDown consentendo a tutte le funzioni di
come Arduino millis() micros() ecc… la piena funzionalità in tutte le
modalità di powerdown.

Task
----

Nella cartella TASK, sono inseriti i singoli task in conformità alle
modalità descritte in premessa, usando il wrapper sviluppato da Michael
Becker. Ogni Task si occupa nello specifico della gestione di
un’apparato e/o di una funzionalità (come ad.es. la comunicazione
remota). Oltre ai parametri passati alla funzione del task, di
particolare importanza il dimensionamento dello stack che deve essere
sufficiente al contenimento delle variabili dinamiche e alle chiamate
locali di altre funzioni. Dal task WatchDog è possibile monitorare in
maniera dinamica l’andamento di ogni singolo stack in modo da utilizzare
il giusto dimensionamento in completa sicurezza.

::

   static SupervisorTask supervisor_task("SupervisorTask", 600, OS_TASK_PRIORITY_02, supervisorParam);

   static SdTask sd_task("SdTask", 1750, OS_TASK_PRIORITY_01, sdParam);

   static UsbSerialTask usbSerial_task("UsbSerialTask", 1100, OS_TASK_PRIORITY_01, usbSerialParam);

   static LCDTask lcd_task("LcdTask", 550, OS_TASK_PRIORITY_03, lcdParam);

   static CanTask can_task("CanTask", 13000, OS_TASK_PRIORITY_02, canParam);

   static ModemTask modem_task("ModemTask", 800, OS_TASK_PRIORITY_02, modemParam);

   static NtpTask ntp_task("NtpTask", 550, OS_TASK_PRIORITY_02, ntpParam);

   static HttpTask http_task("HttpTask", 1400, OS_TASK_PRIORITY_02, httpParam);

   static MqttTask mqtt_task("MqttTask", 1900, OS_TASK_PRIORITY_02, mqttParam);

   static WdtTask wdt_task("WdtTask", 400, OS_TASK_PRIORITY_04, wdtParam);

   // Startup Schedulher

   Thread::StartScheduler();

Se abilitati, i task disponibili (nel Master) sono quelli sopra
elencati. Di seguito le principali funzionalità degli stessi:

- Supervisor
    - Supervisione del sistema
    - Caricamento e gestione della configurazione di sistema
    - Gestione degli stati della comunicazione remota

- SdTask
    - Gestione della SD Card
    - Code per lettura archiviazione dati
    - Code per gestione log
    - Code per caricamento lettura firmware

- UsbSerial
    - Gestione porta USB
    - Gestione RPC locali

- LCD
    - Gestione display
    - Gestione encoder

- Can
    - Gestione porta CAN
    - Classe Canard interrogazione ai moduli Slave Remoti (Cypahl)
    - Sincronizzazione data/ora con gli slave remoti
    - Interfacciamento tra CAN (Moduli remoti) e altri dispositivi locali

- Modem
    - Gestione del modulo SIM7600E
    - Avvio connessione PPP remota
    - interfaccia NET CycloneTCP

- NTP
    - Gestione connessione NTP
    - sincronizzazione data/ora con server remoto

- HTTP
    - Gestione delle connessioni http(s)
    - Gestione RPC Remote (tipicamente configurazione)
    - Dowload firmware

- MQTT
    - Gestione della connessione mqtt(s)
    - pubblicazione dati
    - Gestione RPC Remote
- WDT
    - Gestione WatchDog di Sistema
    - Controllo operatività dei Task
    - Controllo bootLoader

--------------

Per quanto riguarda i moduli slave, i task utilizzati sono relativi alla
gestione del modulo di acquisizione dei sensori (periferia) verso il
modulo master. Alcuni task sono concettualmente simili al modulo Master.
Identica rimane invece la logica di integrazione tra i vari task.

L’esempio sottostante si riferisce al modulo TH, ma i vari moduli sono
pressoché identici. Differiscono solamente nel task di interfaccia verso
la sensoristica controllata (TemperatureHumidity_SensorTask) piuttosto
che (Rain_SensorTask) nel caso del modulo di precipitazione.

::

   static SupervisorTask supervisor_task("SupervisorTask", 250, OS_TASK_PRIORITY_04, supervisorParam);

   static TemperatureHumidtySensorTask th_sensor_task("THTask", 400, OS_TASK_PRIORITY_03, thSensorParam);

   static ElaborateDataTask elaborate_data_task("ElaborateDataTask", 400, OS_TASK_PRIORITY_02,elabParam);

   static AccelerometerTask accelerometer_task("AccelerometerTask", 350, OS_TASK_PRIORITY_01, accelParam);

   static CanTask can_task("CanTask", 7300, OS_TASK_PRIORITY_02, canParam);

   static WdtTask wdt_task("WdtTask", 350, OS_TASK_PRIORITY_01, wdtParam);

   // RunSchedulher

   Thread::StartScheduler();

In StimaV4 ogni dispositivo slave è visto come un’unità indipendente che
si occupa di interfacciarsi con la singola sensoristica in campo e
trasformare il semplice modulo di lettura in un sistema intelligente in
grado di gestirne direttamente le relative acquisizioni,
temporizzazioni, eventuali accensioni e spegnimento (risparmio
energetico) memorizzazioni ed elaborazioni, per fornire al master un
risultato completo con un unico protocollo di comunicazione orientato ai
dati (Cyphal) su CanBus particolarmente efficace in questo tipo di
sistemi.

- Supervisor
    - Supervisione del sistema
    - Caricamento e gestione della configurazione di sistema

- Can
    - Gestione porta CAN
    - Classe Canard comunicazione con modulo Master (Cypahl)
    - Sincronizzazione data/ora con il master
    - Avviamento delle funzioni LowPower dai flags remoti

- Accelerometer
    - Gestione dell’accelerometro (inclinometro solo per pluviometro)

- Sensor
    - Acquisizione locale dei valori istantanei
    - Gestione ON/OFF periferia dove previsto
    - Inserimento dati nei buffer per elaborazioni

- Elaborate
    - Gestione delle elaborazioni dati
    - Gestione code per attesa comandi
    - Presentazione report dati

- WDT
    - Gestione WatchDog di Sistema
    - Controllo operatività dei Task
    - Controllo bootLoader

Task di WatchDog ( Master e Slave )
-----------------------------------

Particolare attenzione è stata posta al TASK WDT watchDog. Il Task
WatchDog si occupa della verifica del corretto funzionamento di tutti i
task di sistema. Il WatchDog HW una volta programmato necessita di una
chiamata di refresh che azzera il contatore WDT. Se il contatore WDT
raggiunge un valore senza essere azzerato il sistema si riavvia.

Per integrare il WatchDog in un sistema RTOS è necessario che tutti i
Task siano in funzione e rispondano al sistema di controllo, per questo
motivo in tutti i task sono state inserite queste 3 funzioni che
agiscono a livello locale. Tramite queste 3 funzioni sono possibili il
monitoraggio dello stack utilizzato, a prevenzione degli eventuali
overflow, la vera e propria chiama di watchDog che comunica al Task WDT
il corretto funzionamento del task e la funzione di TaskState che
comunica al Task WDT lo stato del task (attivo, sospeso, in pausa per un
determinato numero di millisecondi ecc…) Il Task WDT attenderà da tutti
i task attivi e funzionanti il flag di conferma di funzionamento, prima
di azzerare il contatore HW che fisicamente agisce sul reset. Nel task
WDT è possibile verificare stack e stato dei task ed eventualmente
visualizzare e/o registrare su log gli errori.

::

   void TaskMonitorStack();

   void TaskWatchDog(uint32_tmillis_standby);

   void TaskState(uint8_tstate_position, uint8_tstate_subposition, task_flagstate_operation);

Nello sviluppo dei task, il programmatore deve tener conto delle
operazioni di WatchDog e monitor stack, gestendo il posizionamento
corretto delle chiamate a queste funzioni.

Il task WDT si occupa inoltre del corretto controllo del WatchDog HW e
dell’integrazione con il sistema di Boot. E’ stato inserito un sistema
di controllo con flag su Eeprom che consentono al sistema di controllare
e registrare se si sono verificati problemi di avvio. Nel caso
particolare di aggiornamento remoto del firmware questo task prevede
controllo e comunicazione del corretto avvio al bootloader, che in caso
di non avvio del sistema dopo un aggiornamento firmware, ripristina la
memoria flash all’ultimo stato funzionante (rollback).

Questa la struttura bootLoader presente sia sul master che sugli slave
ed interagisce con l’applicazione di avvio. Il sistema tramite la
configurazione degli script LD presenti nel codice si avvia da una
locazione di memoria prefissata, mentre il bootLoader parte
dall’indirizzo di default. Il bootLoader, in condizioni normali, si
preoccupa di avviare correttamente il programma spostando il
programCounter e i registri dei vettori all’indirizzo di memorizzazione
dell’applicativo sulla memoria Flash. Se richiesto un’aggiornamento
firmware (segnalato con gli appositi flag) il sistema è in grado di
riprogrammare la memoria Flash con la nuova versione di programma e
avviare al termine il nuovo applicativo. L’operazione prevede il
salvataggio sulla memoria flash esterna dell’attuale versione in modo
che se una volta riprogrammato il dispositivo interviene il WatchDog
prima dell’avvio del programma, il bootLoader provvederà in un
operazione di rollBack al ripristino della versione precedente. I flag
cosi come descritti sotto nella struttura segnalano tutte le possibili
eventualità

::

   typedef struct
   {
       bool request_upload;    // Request an upload of firmware

       bool backup_executed;   // Firmware backup is executed

       bool upload_executed;   // An upload of firmware was executed

       bool rollback_executed; // An rollback of firmware was executed

       bool app_executed_ok;   // Flag running APP (setted after new firmware, prevert a rollback operation)

       bool app_forcing_start; // Force starting APP from Flash RUN APP Memory Position

       uint8_t upload_error;   // Error in upload firmware (ID of Error)

       uint8_t tot_reset;      // Number of module reset

       uint8_t wdt_reset;      // Number of WatchDog

   } bootloader_t;

Comunicazione tra i task
------------------------

La comunicazione tra i task, comandi risposte e scambio dati, avviene
tramite il meccanismo delle code RTOS. Il passaggio dei dati per esempio
tra i dati acquisiti dal master CAN (come detto precedentemente il task
CAN si occupa dell’interrogazione dei moduli slave per il recupero dei
dati remoti) e SD Card (memorizzazione delle elaborazioni dati su SD
Card) avviene con una coda specifica.

Analogamente il Task MQTT tramite una specifica coda fa richiesta dei
dati al task SD Card (precedentemente memorizzati) prima di pubblicarli
al server remoto.

Con questo sistema si è riusciti per esempio a separare i moduli
concedendo al solo task SD Card, l’accesso al dispositivo HW e poter
gestire in autonomia le proprie priorità e criticità.

Alcune code (inerenti alla comunicazione) sono di gestione dei task.
Queste code sono utilizzate per dare comandi di avvio e sospensione ai
task e gestirne l’operatività e sincronizzazione. Nel modulo master il
task di supervisione, si occupa della gestione della comunicazione.
Quando richiesta una connessione remota il task di supervisione attiva
in sequenza il task di gestione del GSM, che si occupa di stabilire una
connessione PPP remota, per poi passare in sequenza l’avvio dei task
NTP, http, MQTT a seconda delle esigenze. Il task di supervisione una
volta attivato un comando/task attenderà la relativa risposta dalla coda
di gestione e così potrà decidere la sequenza di operazioni da compiere.

Una particolare coda systemMessage si occupa del passaggio di comandi
con eventuali parametrizzazioni ai vari task. Questa coda viene
utilizzata per il passaggio di un comando ad uno specifico TASK
(inserendo l’ID del task) o a un particolare ID (ALL) che indica che il
messaggio e valido per tutti i task (per esempio il comando di entrata
in sleep del sistema). Lo sleep è comunicato a tutti i task, ma solo
quando tutti i task hanno finito le relative operazioni (e confermato
opportunamente) in totale sicurezza, può essere concesso il power down
al sistema. Il systemMessage è utilizzato anche per l’invio di un
comando tra un task e l’altro come ad esempio il comando tramite
LCD-Encoder di calibrazione accelerometro, reset flag di sistema ecc… In
questo caso deve essere selezionato il TASK di destinazione CAN (non
importa il chiamante, può essere una RPC Remota SerialUSB o http o MQTT
o per finire da comando LCD). Ogni task avrà al suo interno un sistema
di gestione delle letture delle code in grado di determinare la presenza
messaggio e se il messaggio è indirizzato al task locale. Nel caso il
messaggio verrà prelevato dalla coda e processato. Se il messaggio è per
tutti solo un task avrà la possibilità di eliminare il messaggio,
tipicamente il Supervisor o il task CAN a seconda delle esigenze.

Esempio fasi di sleep per un Task:
    - verifica messaggi in coda
        - messaggio per tutti
            - messaggio di sleep
            - messa in stato di sospensione del WatchDog per un tempo pari al tempo configurato di sleep del task (tempo massimo che non pregiudica il funzionamento del sistema senza quel task attivo)
            - Entrata di task in sleep spegnimento periferiche locali
            - Attivazione del Delay (lungo) che pone il task dormiente.
            - Risveglio e riattivazione periferiche locali

Il resto è gestito dall’RTOS (quando tutti i task sono in sleep per un
tempo superiore al tempo configurato minimo di attivazione del basso
consumo il sistema entra in Sleep, cioè chiama la funzione xTaskSleep
che è definita nel modulo drivers frertosCallback)

Al rientro dallo sleep il Task dovrà riaccendere le proprie perifieriche
ed attendere gli eventuali timer di stabilizzazione degli stessi prima
di rientrare in modalità operativa.

::

   if(!param.systemMessageQueue->IsEmpty()) {

       // Read queue in test mode

       if (param.systemMessageQueue->Peek(&system_message, 0))

       {

           // Its request addressed into ALL TASK... -> no pull (only SUPERVISOR or exernal gestor)

           if(system_message.task_dest == ALL_TASK_ID)

           {

               // Pull && elaborate command,

               if(system_message.command.do_sleep)

               {

                   // Enter sleepmodule OK and update WDT

                   TaskWatchDog(SD_TASK_SLEEP_DELAY_MS);

                   TaskState(state, UNUSED_SUB_POSITION, task_flag::sleepy);

                   Delay(Ticks::MsToTicks(SD_TASK_SLEEP_DELAY_MS));

                   TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

               }

           }

       }

   }

Queste le code definite nel main del modulo Master che permettono la
comunicazione tra i vari task e le classi previste.

::

   staticQueue systemMessageQueue;         // Gestione invio messaggi e coamdandi tra i task

   staticQueue connectionRequestQueue;     // Invio richieste di avvio connessione GSM e rete (http...)

   staticQueue connectionResponseQueue;    // Risposte alle richieste di connessione (stato ed errori)

   staticQueue dataRmapPutQueue;           // Utilizzata come push dei dati acquisiti verso SD Card

   staticQueue dataRmapGetRequestQueue;    // Richiesta Lettura dati RMAP (Cyphal) da SD Card per MQTT

   staticQueue dataRmapGetResponseQueue;   // Risposta e stato dati RMAP (Chphal) da SD per invio MQTT

   staticQueue dataRmapPutBackupQueue;     // Coda per push dati RMAP di backup (formato nativo)

   staticQueue dataFilePutRequestQueue;    // Coda per trasmissione file verso SD (es. push firmware)

   staticQueue dataFilePutResponseQueue;   // Coda in risposta alle richieste push file su SD

   staticQueue dataFileGetRequestQueue;    // Coda per lettura file da SD (es. get firmware)

   staticQueue dataFileGetResponseQueue;   // Coda in risposta alle richieste get file da SD

   staticQueue dataLogPutQueue;            // Coda per invio stringe LOG da salvare su SD Card

   staticQueue displayEventWakeUp;         // Coda per gestione comandi e sleep per task LCD

Questi invece i semafori che vengono utilizzati per la condivisione di
risorse hw e/o per l’accesso ai parametri di sistema

::

   staticBinarySemaphore wireLock;             // Access I2C external interface UPIN_27

   staticBinarySemaphore wire2Lock;            // Access I2C internal EEprom, Display

   staticBinarySemaphore canLock;              // Can BUS

   staticBinarySemaphore qspiLock;             // Qspi (Flash Memory)

   staticBinarySemaphore rtcLock;              // RTC (Access lock)

   staticBinarySemaphore rpcLock;              // RPC (Access lock)

   staticBinarySemaphore configurationLock;    // Access Configuration (parameter)

   staticBinarySemaphore systemStatusLock;     // Access System status (parameter)

   staticBinarySemaphore registerAccessLock;   // Access Register Cyphal Specifications (parameter EEprom)

Configurazione di un task
-------------------------

Ogni task lanciato dal main ha parametri di gestione che vengono passati
al relativo task. Questi parametri sono definiti nel main e sono
relativi ai dispositivi hw, semafori e/o code tra quelle viste in
precedenza. Analizzando per esempio la configurazione del TASK CAN,
vediamo i parametri che vengono passati. Analogamente tutti i task hanno
una struttura similare a quella descritta sotto.

::

   #if (ENABLE_CAN)

   // TASK CAN PARAM CONFIG

   static CanParam_t canParam = {0};                   // Parametri del CAN

   canParam.configuration = &configuration;            // puntatore alla configurazione di sistema

   canParam.system_status = &system_status;            // puntatore allo stato di sistema

   canParam.boot_request = &boot_check;                // puntatore alla struttura dei flag di bootLoader

   canParam.configurationLock = configurationLock;     // semaforo per l'accesso alla configurazione

   canParam.systemStatusLock = systemStatusLock;       // semaforo per l'accesso allo stato di sistema

   canParam.registerAccessLock = registerAccessLock;   // semaforo per accesso ai registri Cyphal

   canParam.systemMessageQueue = systemMessageQueue;   // coda dei messaggi di sistema

   canParam.requestDataQueue = requestDataQueue;       // coda di richiesta dati

   canParam.reportDataQueue = reportDataQueue;         // coda per le risposte report dei dati

   canParam.eeprom = &memEprom;                        // puntatore alla classe EEprom

   canParam.clRegister = &clRegister;                  // puntatore alla classe Registri

   canParam.flash = &memFlash;                         // puntatore alla classe Flash memory

   canParam.canLock = canLock;                         // semaforo di accesso HW al CAN Bus

   canParam.qspiLock = qspiLock;                       // semaforo di accesso alla porta QSPI per Flash memory

   canParam.rtcLock = rtcLock;                         // semaforo di accsso al real time clock

   #endif

Il tipo CanParam_t (ogni task ne ha uno specifico) è definito
all’interno del header del relativo task e appunto ne specifica i
dispositivi utilizzati. Questi vengono passati per indirizzo alla classe
che potrà quindi disporre delle risorse necessarie al suo funzionamento.
Il task avrà all’interno della classe questa struttura generale per
l’accesso alle risorse e ovviamente al suo interno altre variabili
specifiche locali visibili solo all’interno della specifica classe.

I task sono organizzati a stati state_t, normalmente una fase di
inizializzazione, avvio, gestione e sleep, dipendente dall’HW e o dalle
operazioni richieste. Con questa gestione è possibile identificare le
varie sezioni e al proprio interno poter gestire le relative operazioni
e lo switch tra i task definendo bene le tempistiche di accesso a
dispositivi o parametri. All’interno dell’header sono presenti le
definizioni delle tempistiche di gestione dei task:

::

   // Main TASK Switch Delay

   #define CAN_TASK_WAIT_DELAY_MS          (20)

   #define CAN_TASK_WAIT_MAXSPEED_DELAY_MS (1)

   #define CAN_TASK_SLEEP_DELAY_MS         (850)

Sono i tempi di gestione del task in modalità normale ogni (20 mSec),
modalità real_time (quando il task deve essere sempre eseguito per
specifici controlli (1 mSec), e quando il task può andare in sleep (tempo
massimo di attesa prima dello switch nel contesto 850 mS). Nel CAN
essendo necessario un heartBeat definito dalle specifiche Cypal di 1
secondo, si è scelto un tempo massimo all’interno del massimo rate dell
heartBeat. Ogni task ha queste specifiche ma i tempi sono differenti a
seconda delle esigenze di funzionamento.

All’avvio del TASK è necessaria la configurazione dell’interfaccia HW/SW
per il setup bxCAN e delle velocità di collegamento. Queste operazioni
sono effettuate a partire dalla lettura dei registri Cyphal CAN, tramite
la classe di accesso (descritta più avanti). In avvio vengono inoltre
attivate le funzioni Interrupt di ricezione CAN per l’utilizzo di bxCAN
con i flag di interrupt necessari ad un funzionamento corretto del
driver.

I dati di configurazione sono letti dai registri Cyphal. La loro
modifica con qualsiasi programma come il tool Yakut in grado di
modificare i parametri dei registri ne altera le funzionalità. E’
possibile per esempio ridurre la velocità dei moduli CAN se si
presentano errori di comunicazione o se le distanze di collegamento
prevedono velocità ridotte.

::

   // CANARD MTU CLASSIC (FOR UAVCAN REQUIRE)

   // Open Register in Write se non inizializzati correttamente...

   // Populate INIT Default Value

   static uavcan_register_Value_1_0 val = {0};

   uavcan_register_Value_1_0_select_natural16_(&val);

   val.natural16.value.count = 1;

   val.natural16.value.elements[0] = CAN_MTU_BASE; // CAN_CLASSIC MTU 8

   localRegisterAccessLock->Take();

   localRegister->read(REGISTER_UAVCAN_MTU, &val);

   localRegisterAccessLock->Give();

   LOCAL_ASSERT(uavcan_register_Value_1_0_is_natural16_(&val) && (val.natural16.value.count == 1));

   // CANARD SETUP TIMINGS AND SPEED

   // CAN BITRATE Dinamico su LoadRegister (CAN_FD 2xREG natural32 0=Speed, 1=0 (Not Used))

   uavcan_register_Value_1_0_select_natural32_(&val);

   val.natural32.value.counT = 2;

   val.natural32.value.elements[0] = CAN_BIT_RATE;

   val.natural32.value.elements[1] = 0ul; // Ignored for CANARD_MTU_CAN_CLASSIC

   localRegisterAccessLock->Take();

   localRegister->read(REGISTER_UAVCAN_BITRATE, &val);

   localRegisterAccessLock->Give();

   LOCAL_ASSERT(uavcan_register_Value_1_0_is_natural32_(&val) && (val.natural32.value.count == 2));

   // Dynamic BIT RATE Change CAN Speed to CAN_BIT_RATE (register default/defined)

   BxCANTimings timings;

   bool result = bxCANComputeTimings(HAL_RCC_GetPCLK1Freq(), val.natural32.value.elements[0], &timings);

   if (!result) {
       ...
       return;
   }

   // Configurea bxCAN speed && mode

   result = bxCANConfigure(0, timings, false);

   if (!result) {
       ...
       return;
   }

   // CANARD SETUP TIMINGS AND SPEED COMPLETE

   // Check error starting CAN

   if (HAL_CAN_Start(&hcan1) != HAL_OK)
   {
       ...
       TRACE_ERROR_F(F("CAN startup ERROR!!!\r\n"));
   }

   // Enable Interrupt RX Standard CallBack -> CAN1_RX0_IRQHandler

   if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {

       TRACE_ERROR_F(F("Error initialization interrupt CAN base\r\n"));

       LOCAL_ASSERT(false);

       return;

   }

   // Setup Priority e CB CAN_IRQ_RX Enable

   HAL_NVIC_SetPriority(CAN1_RX0_IRQn, CAN_NVIC_INT_PREMPT_PRIORITY, 0);

   HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);

   // Setup Complete

   TRACE_VERBOSE_F(F("CAN Configuration complete...\r\n"));

Nel CAN Task dopo l’inizializzazione nell’avvio RUN in specifico si
hanno in sequenza le operazioni di configurazione dell’HW e della
libreria Canard che agisce sul CAN Bus con il protocollo Cyphal, per poi
passare all’avvio della configurazione delle sue funzionalità.

Nella fase di INIT si hanno la lettura dei registri Cyphal e la
registrazione delle sottoscrizione ai messaggi Cyphal, la procedura di
callback dei messaggi in ingresso ricevuti e l’impostazione degli ID dei
nodi di rete.

::

   TRACE_INFO_F(F("Can task: STARTING Configuration\r\n"));

   // Avvio inizializzazione (Standard UAVCAN MSG). Reset su INIT END OK

   // Segnale al Master necessità di impostazioni ev. parametri, Data/Ora ecc..

   clCanard.flag.set_local_node_mode(uavcan_node_Mode_1_0_INITIALIZATION);

   // Attiva il callBack su RX Messaggio Canard sulla funzione interna processReceivedTransfer

   clCanard.setReceiveMessage_CB(processReceivedTransfer);

   // Setup INIT Time for syncronized TimeStamp with local RTC

   clCanard.setMicros(rtc.getEpoch(), rtc.getSubSeconds());

   // INIT VALUE, Caricamento default e registri locali MASTER e lettura Registri standard UAVCAN

   clCanard.set_canard_node_id((CanardNodeID) NODE_MASTER_ID);

Questa invece una tipica chiamata per la sottoscrizione dei servizi. La
sottoscrizione registra un modulo ad un particolare evento, in questo
caso il messaggio esterno di richiesta Info GetInfo e Comandi.

::

   TRACE_INFO_F(F("Can task: STARTING UAVCAV Subscrition and Service\r\n"));

   // Service servers: -> Risposta per GetNodeInfo richiesta esterna (Yakut, Altri)

   if (!clCanard.rxSubscribe(CanardTransferKindRequest,

       uavcan_node_GetInfo_1_0_FIXED_PORT_ID_,

       uavcan_node_GetInfo_Request_1_0_EXTENT_BYTES_,

       CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {

       LOCAL_ASSERT(false);

   }

   // Service servers: -> Chiamata per ExecuteCommand richiesta esterna (Yakut, Altri)

   if (!clCanard.rxSubscribe(CanardTransferKindRequest,

       uavcan_node_ExecuteCommand_1_1_FIXED_PORT_ID_,

       uavcan_node_ExecuteCommand_Request_1_1_EXTENT_BYTES_,

       CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {

       LOCAL_ASSERT(false);

   }

Se nella rete Cyphal un messaggio tra quelli registrati è indirizzato al
nodo (me stesso) che ha effettuato la registrazione, al momento del
trasferimento completo del messaggio verrà attivata la procedura di
callback registrata in avvio e nella funzione chiamata sarà trasferito
il messaggio in ingresso e la porta fissa o dinamica (che rappresenta il
comando entrante).

Successivamente si passa alla gestione temporizzata della rete con
attesa e processo dei messaggi remoti e la pubblicazione dei propri e
all’invio dei comandi ai nodi remoti.

Passato il tempo di acquisizione dati impostato in configurazione una
serie di comandi risvegliano i nodi remoti dallo stato di basso consumo
ed inviano la richiesta dei dati. Una volta acquisiti i valori questi
vengono passati alla coda di push dei dati su SD card per
l’archiviazione e il successivo prelievo da parte del supervisor per la
pubblicazione al server Remoto. Il CAN tak si occupa inoltre di
attendere tramite code gli eventuali comandi remoti provenienti da RPC o
comandi locali LCD per inviarli ai destinatari tramite il CAN Bus, quali
ad esempio l’aggiornamento del firmware con file transfer Cyphal.

Avvio della richiesta dati

::

   // Get Istant Data or Archive Data Request (Need to Display, Saving Data or other Function)

   if ((bStartGetIstant)||(bStartGetData)) {

       // P er tutti i nodi avvio la funzionalità di lettura dati RMAP

       // bStartGetData prioritario rispetto bGetIstData

       for(uint8_t queueId=0; queueId<MAX_NODE_CONNECT; queueId++) {

       // Solo per i nodi onLine

       if(clCanard.slave[queueId].is_online()) {

           // Se il servizio di getRMAPData non è impegnato

           if(!clCanard.slave[queueId].rmap_service.is_pending()) {

               // parametri.canale = rmap_service_setmode_1_0_CH01 (es-> set CH Analogico...)

               // parametri.run_for_second = 900; ( not used for get_istant )

               rmap_service_setmode_1_0 paramRequest;

               paramRequest.chanel = 0; // Imposto il canale fisico, se necessario

         // Preparo la richiesta (dati archivio o istantanei?)

               if(bStartGetData) {

                   paramRequest.command = rmap_service_setmode_1_0_get_last;

                   paramRequest.obs_sectime = param.configuration->observation_s;

                   paramRequest.run_sectime = param.configuration->report_s;

               } else {

                   paramRequest.command = rmap_service_setmode_1_0_get_istant;

                   paramRequest.obs_sectime = 0;

                   paramRequest.run_sectime = 0;

Attesa della risposta dati (se in attività, cioè dopo l’avvio di una
richiesta)

::

   if(param.system_status->flags.rmap_server_running) {

       // Controllo il file server se non in running state

       bool rmapServerEnd = true;

       // Waiting WARM_UP (GetSyncroTime UP Procedure before end server)

       if(bStartSetFullPower) rmapServerEnd = false;

       for(uint8_t queueId=0; queueId<MAX_NODE_CONNECT; queueId++) {

       // Check if is request pending... (NONE... flag remaining true END Server)

       if (clCanard.slave[queueId].rmap_service.is_pending()) {

           rmapServerEnd = false;

       }

       if (clCanard.slave[queueId].rmap_service.event_timeout()) {

           // Next retry if is possible Stop and estart pending

           clCanard.slave[queueId].rmap_service.reset_pending();

           if(!clCanard.send_rmap_data_pending_retry(queueId, NODE_GETDATA_TIMEOUT_US)) {

               // TimeOUT di un comando in attesa... end Retry

           } else {

               rmapServerEnd = false;

               // TimeOUT di un comando in attesa... gestione Retry

               clCanard.slave[queueId].get_node_id(), clCanard.slave[queueId].rmap_service.retry + 1);

           }

       }

   }

   ...

   // EVENTO DI GESTIONE RICEZIONE DATI

   switch (clCanard.slave[queueId].get_module_type()) {

       case Module_Type::th:

           // Cast to th module

           retTHData = (rmap_service_module_TH_Response_1_0) clCanard.slave[queueId].rmap_service.get_response();

           // data RMAP type is ready to send into queue Archive Data for Saving on SD Memory

           // Get parameter data

           #if TRACE_LEVEL >= TRACE_INFO

           getStimaNameByType(stimaName, clCanard.slave[queueId].get_module_type());

           #endif

Interprete del dato e del metodo di richiesta e trasmissione a coda SD
Card per archiviazione dati

::

   // Inserisce i dati nel system_status

   if(retTHData->state == rmap_service_setmode_1_0_get_istant) {

       // Solo istantanei per visualizzazione LCD o  altre attività locali

       param.systemStatusLock->Take();

       param.system_status->data_slave[queueId].data_value[0] = retTHData->ITH.temperature.val.value;

       param.system_status->data_slave[queueId].data_value[1] = retTHData->ITH.humidity.val.value;

       param.system_status->data_slave[queueId].is_new_ist_data_ready = true;

       param.systemStatusLock->Give();

   } else if(retTHData->state == rmap_service_setmode_1_0_get_last) {

       // Dati e stati elaborati (da inviare al sistema di archiviazione)

       bit8Flag = 0;

       if(retTHData->is_main_error) bit8Flag|=0x01;

       if(retTHData->is_redundant_error) bit8Flag|=0x02;

       param.systemStatusLock->Take();

       param.system_status->flags.new_data_to_send = true;

       param.system_status->data_slave[queueId].bit8StateFlag = bit8Flag;

       param.system_status->data_slave[queueId].byteStateFlag[0] = retTHData->rbt_event;

       param.system_status->data_slave[queueId].byteStateFlag[1] = retTHData->wdt_event;

       param.system_status->data_slave[queueId].byteStateFlag[2] = retTHData->perc_i2c_error;

       param.systemStatusLock->Give();

       // Copia i dati dal report alla coda di pubblicazione

       memset(&rmap_archive_data, 0, sizeof(rmap_archive_data_t));

       // Set Module Type, Date Time as Uint32 GetEpoch_Style, and Block Data Cast to RMAP Type

       rmap_archive_data.module_type = clCanard.slave[queueId].get_module_type();

       rmap_archive_data.date_time = param.system_status->datetime.epoch_sensors_get_value;

       memcpy(rmap_archive_data.block, retTHData, sizeof(retTHData));

       // Trasmestto a SD Card nella relativa coda con i relativi limiti di controllo

       if(param.dataRmapPutQueue->IsFull()) param.dataLogPutQueue->Dequeue(&rmap_archive_empty);

       param.dataRmapPutQueue->Enqueue(&rmap_archive_data, Ticks::MsToTicks(CAN_PUT_QUEUE_RMAP_TIMEOUT_MS));

   }

   break;

Ogni funzionalità, comando, file transfer ecc. della rete Cyphal su CAN
così come inserito nel progetto stimaV4 segue lo stesso iter, se il
canale per quel nodo e per quel determinato comando è libero (non in
pending) si può avviare e questo passa in uno stato di waiting con il
timeout specifico. Successivamente l’ambiente conosce che è in corso un
comando che potrà passare o allo stato di executed o di time_out
librando il canale ad un altro eventuale comando o alla gestione delle
retry. I flag VSC visti in precedenza indicano al master lo stato di un
nodo remoto. Prima di inviare un comando al nodo remoto il flag di
full_power viene abilitato in modo che solo quando il nodo remoto
comunica di essere in full_power pronto quindi a ricevere messaggi senza
rischio di perdite dati, il master invia la propria trasmissione e
attende la risposta remota. Al termine quando tutti i relativi flags del
modulo slave sono off come ad.es file_server, command_server,
rmap_data_server il master potrà nuovamente indicare allo slave relativo
di tornare in modalità normale.

Classe register
---------------

La classe register è una particolare classe che è derivata da una
modifica del file register.c dell’applicativo Cyphal. Questa classe si
occupa di inizializzare, leggere e salvare particolari registri Cyphal
ex “Uavcan”. La classe si integra con il protocollo descritto in
precedenza e fornisce parametrizzazioni del protocollo e degli
applicativi sviluppati, quindi con registri di scopo per il
funzionamento del protocollo e delle sue sottoscrizioni e degli
applicativi utente, come ad. esempio nei moduli TH l’intervallo di tempo
di acquisizione dei sensori.

Tutti gli applicativi Master e Slave hanno particolari registri il cui
accesso e garantito con le funzioni di protocollo ReadRegister,
WriteRegister, ListRegister di Cypal, in modo da rendere configurabile
il modulo senza modificare il software e/o le sue definizioni.
Semplicemente accedendo ai relativi registri (di modulo o generali)
anche con applicativi esterni che integrano il protocollo, è possibile
leggere e alterarne i valori, modificando di fatto il suo funzionamento.

L’accesso ai registri è stato modificato per l’integrazioni con il
modulo EEprom degli applicativi a partire dai sorgenti originali della
libreria LibCanard che utilizzavano la SD Card (gestione a file) dei
registri. Si è così proceduto ad interfacciare la memoria EEprom
permanente dei moduli StimaV4.

La classe è stata scritta in conformità agli altri moduli Task per
rendere omogeneo l’ambiente di sviluppo. La sua parametrizzazione segue
lo stesso concetto utilizzati per i TASK, con una struttura locale a cui
sono passate le istanze degli oggetti utilizzate (eeprom, semafori ecc…)

Questi i registri tipici del master con i relativi nomi accessibili
sulla rete Cyphal

::

   #define REGISTER_UAVCAN_MTU             "uavcan.can.mtu"

   #define REGISTER_UAVCAN_BITRATE         "uavcan.can.bitrate"

   #define REGISTER_UAVCAN_NODE_ID         "uavcan.node.id"

   #define REGISTER_UAVCAN_UNIQUE_ID       "uavcan.node.unique_id"

   #define REGISTER_UAVCAN_NODE_DESCR      "uavcan.node.description"

   #define REGISTER_UAVCAN_DATA_PUBLISH    "uavcan.pub.rmap.publish.id"

   #define REGISTER_UAVCAN_DATA_SERVICE    "uavcan.srv.rmap.service.id"

   #define REGISTER_METADATA_LEVEL_L1      "rmap.metadata.Level.L1"

   #define REGISTER_METADATA_LEVEL_L2      "rmap.metadata.Level.L2"

   #define REGISTER_METADATA_LEVEL_TYPE1   "rmap.metadata.Level.LevelType1"

   #define REGISTER_METADATA_LEVEL_TYPE2   "rmap.metadata.Level.LevelType2"

   #define REGISTER_METADATA_TIME_P1       "rmap.metadata.Timerange.P1"

   #define REGISTER_METADATA_TIME_PIND     "rmap.metadata.Timerange.Pindicator"

   #define REGISTER_DATA_PUBLISH           "rmap.publish"

   #define REGISTER_DATA_SERVICE           "rmap.service"

   #define REGISTER_RMAP_MASTER_ID         "rmap.master.id"

Questi invece i semplici metodi della classe per accedere ai registri di
sistema. Nel costruttore vengono passate le risorse necessarie alla
classe per la gestione accesso alla memoria.

::


   // Costruttore
   EERegister(TwoWire *wire, BinarySemaphore *wireLock, uint8_t i2c_address = EEPROM_AT24C64_DEFAULT_ADDRESS);

   // Inizializza lo spazio RAM/ROM/FLASH/SD dei registri, ai valori di default
   void setup(void);

   // Legge uno specifico registro
   void read(const char* const register_name, uavcan_register_Value_1_0* const inout_value);

   // Scrive uno specifico registro
   void write(const char* const register_name, const uavcan_register_Value_1_0* const value);

   // Recupera il nome di un registro dall' elenco dei disponibili
   uavcan_register_Name_1_0 getNameByIndex(const uint16_t index);

   // Inizializza i registri ai valori di Reset
   void doFactoryReset(void);

Classe RPC
----------

Anche in questo caso ci troviamo di fronte ad una particolare classe
scritta per mantenere omogeneità con l’ambiente di sviluppo.

Questa classe è il tramite tra le RPC di sistema e la libreria Arduino
JSON, per gestire le RPC remote che interagiscono con il sistema tramite
comandi JSON.

Ogni modulo che ne ha necessità avrà un’istanza della classe e al suo
interno i vari metodi di gestione dei comandi JSON (in trasmissione e
risposta) diventano operazioni trasparenti.

La classe è utilizzata per esempio sui Task USB Serial (comandi locali
da USB Seriale), CAN Bus (comandi locali su CAN BUS), http (comandi
remoti da connessione http come il download della configurazione), MQTT
(comandi remoti da connessione MQTT come ad esempio reboot, download
firmware ecc..)

Trovandoci all’interno di un sistema RTOS, la chiamata alla classe da
parte di un TASK deve sapere come i vari comandi interagiscono tra loro.
Se un particolare comando deve attendere risposta da un particolare task
per avere la certezza dell’esecuzione dello stesso ma proviene da un
diverso Task, il sistema deve poterlo gestire in sicurezza. Nel codice
sotto è visibile l’attesa non bloccante del task che ripassa il
controllo al sistema operativo ma attende comunque la conferma
dell’esecuzione del comando dal task interessato.

Prendiamo in esame ad. esempio un metodo JSON per l’inizializzazione
remota della SD Card. Il task chiamante non è importante ma prima di
dare risposta al task chiamante ci dobbiamo assicurare che il comando
eseguito (in questo caso dal task SD Card) abbia terminato la sua
esecuzione.

Per effettuare il tutto in sicurezza, viene gestito lo switch dei
contesti internamente e tramite le code di messaggi di sistema viste in
precedenza si attiva la funzione e si attende la risposta. Al termine si
può rientrare nel Task chiamante che può portare a termine le altre
operazioni in corretta sequenza.

Il programmatore deve essere a conoscenza del tempo necessario per la
gestione di questa operazione e per non incorre all’intervento del
WatchDog, dovrà mettere lo stato del task in sospensione e/o abilitarlo
per un lasso di tempo minimo necessario all’esecuzione dell’operazione,
come spiegato in precedenza.

La classe, come quella register descritta in precedenza, è stata
realizzata in conformità agli altri moduli Task per rendere omogeneo
l’ambiente di sviluppo. La sua parametrizzazione segue lo stesso
concetto utilizzato per i TASK, con una struttura locale a cui sono
passate le istanze degli oggetti utilizzati (eeprom, semafori ecc…)

::

   else if (strcmp(it.key().c_str(), "sdinit") == 0)

   {

       error_command = false;

       // RPC Command for reinit SD Card

       if (it.value().as<bool>() == true)

       {

           // Starting queue request truncate structure data on SD Card (Remote request)

           system_message_t system_message = {0};

           system_message.task_dest = SD_TASK_ID;

           system_message.command.do_trunc_sd = true;

           system_message.param = CMD_PARAM_REQUIRE_RESPONSE;

           param.systemMessageQueue->Enqueue(&system_message);

           // Waiting a response done before continue (reinit SD Data OK!!!)

           while(true) {

               // Continuos Switching context non blocking

               // Need Waiting Task for start command on All used TASK

               taskYIELD();

               vTaskDelay(100);

               // Check response done

               if(!param.systemMessageQueue->IsEmpty()) {

                   param.systemMessageQueue->Peek(&system_message);

                   if(system_message.command.done_trunc_sd) {

                       // Remove message (Reinit Done is OK)

                       param.systemMessageQueue->Dequeue(&system_message);

                       break;

                   }

               }

           }

           TRACE_INFO_F(F("RPC: DO INIT SD CARD DATA\r\n"));

       }

   }

   else if (strcmp(it.key().c_str(), "reginit") == 0)
   ...

Debug e LOG
-----------

Per utilizzare le funzioni di Debug e LOG all’interno del file
debug_config.h sono definiti i livelli di TRACE dei messaggi, cioè il
livello per ogni TASK di LOG. Ogni livello attiva più o meno messaggi a
seconda di come sono stati inseriti nell’applicativo. Di seguito un
esempio di definzione.

::

   #define STIMA_TRACE_LEVEL               TRACE_LEVEL_INFO

   #define ETHERNET_TASK_TRACE_LEVEL       TRACE_LEVEL_OFF

   #define MODEM_TASK_TRACE_LEVEL          TRACE_LEVEL_VERBOSE

   #define NTP_TASK_TRACE_LEVEL            TRACE_LEVEL_INFO

   #define MQTT_TASK_TRACE_LEVEL           TRACE_LEVEL_VERBOSE

   #define HTTP_TASK_TRACE_LEVEL           TRACE_LEVEL_INFO

   #define SUPERVISOR_TASK_TRACE_LEVEL     TRACE_LEVEL_INFO

   #define CAN_TASK_TRACE_LEVEL            TRACE_LEVEL_VERBOSE

   #define SD_TASK_TRACE_LEVEL             TRACE_LEVEL_INFO

   #define LCD_TASK_TRACE_LEVEL            TRACE_LEVEL_INFO

   #define USBSERIAL_TASK_TRACE_LEVEL      TRACE_LEVEL_INFO

   #define WDT_TASK_TRACE_LEVEL            TRACE_LEVEL_OFF

   #define SIM7600_TRACE_LEVEL             TRACE_LEVEL_VERBOSE

I livelli possibili da debug.h

::

   //Trace level definitions

   #define TRACE_LEVEL_OFF       0

   #define TRACE_LEVEL_FATAL     1

   #define TRACE_LEVEL_ERROR     2

   #define TRACE_LEVEL_WARNING   3

   #define TRACE_LEVEL_INFO      4

   #define TRACE_LEVEL_DEBUG     5

   #define TRACE_LEVEL_VERBOSE   6

NTP_TASK, come configurato sopra avrà per esempio solo i level INFO,
quindi solo le stampe INFO o con indice minore potranno essere
visualizzate.

::

   TRACE_INFO_F(F("RPC: DO DOWNLOAD FIRMWARE\r\n"));

Analogamente TRACE_LOG avrà la stessa funzionalità ma il messaggio non è
inviato ad un TRACE Seriale per il monitor di sistema ma da una
particolare coda (LOG) per il push dei messaggi su SD Card. Il livello
trace e la modalità di utilizzo è identica tra i due metodi.

Connessione Modem, ntp, http, mqtt
----------------------------------

I task di gestione della connessione come specificato in precedenza sono
gestiti dalle code di connessione. In particolare quando necessario
l’avvio della comunicazione, sia esso per la temporizzazione di
configurazione sia per una richiesta estemporanea per esempio da comando
apposito tramite LCD, viene attivato il meccanismo di start dei task di
comunicazione dal task supervisor.

Il Task di supervisione parte leggendo la configurazione di sistema per
poi inizializzare le variabili locali di interesse. A questo punto il
task si mette in attesa delle operazioni sopradescritte in attesa
dell’avvio della comunicazione.

::

   // Start only modulePower Full OK (no energy rest) Exit on Deep Power Save or Critical mode...

   if(param.system_status->flags.power_state >= Power_Mode::pwr_deep_save) {

       // Sleep continuos TASK if notingh to do

       TaskWatchDog(SUPERVISOR_TASK_SLEEP_DELAY_MS);

       TaskState(state, UNUSED_SUB_POSITION, task_flag::sleepy);

       Delay(Ticks::MsToTicks(SUPERVISOR_TASK_DEEP_POWER_DELAY_MS));

       break;

   } else {

       // Standard Waiting Sleeping mode

       TaskWatchDog(SUPERVISOR_TASK_SLEEP_DELAY_MS);

       TaskState(state, UNUSED_SUB_POSITION, task_flag::sleepy);

       Delay(Ticks::MsToTicks(SUPERVISOR_TASK_SLEEP_DELAY_MS));

       TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

   }

In una prima fase, cosi come generalmente prevista nei vari task a
seconda dell’operatività del TASK si provvede a determinare il tempo di
funzionamento nel contesto. Questo rende possibile al modulo l’entrata
in basso consumo se e quando tutti i task non hanno attività da
effettuare.

::

   // START REQUEST function LIST...

   param.systemStatusLock->Take();

   param.system_status->connection.is_ntp_synchronized = !param.system_status->command.do_ntp_synchronization;

   param.system_status->connection.is_http_configuration_updated = !param.system_status->command.do_http_configuration_update;

   param.system_status->connection.is_http_firmware_upgraded = !param.system_status->command.do_http_firmware_download;

Per avviare una comunicazione, vengono specificati le operazioni da
effettuare (automatiche e/o manuali) tramite le parametrizzazioni dei
flag do_mqtt, do_ntp, do_http_configuration ecc..

Questi flag se abilitati informano il task della necessità di avviare
quel tipo di connessione (e quindi di quello specifico TASK). Si passa
adesso alla gestione dello stato di connessione.

::

   // SUB Case of sequence of check (connection / operation) state

   switch(state_check_connection) {

       case CONNECTION_INIT: // STARTING CONNECTION

           TRACE_VERBOSE_F(F("SUPERVISOR_STATE_CONNECTION_OPERATION -> SUPERVISOR_STATE_REQUEST_CONNECTION\r\n"));

           state = SUPERVISOR_STATE_REQUEST_CONNECTION;

           state_check_connection = CONNECTION_CHECK;

           break;

       case CONNECTION_CHECK: // CONNECTION VERIFY

           if (!param.system_status->connection.is_connected) // Ready Connected ?

           {

               TRACE_VERBOSE_F(F("SUPERVISOR: Connection not ready\r\n"));

               TRACE_VERBOSE_F(F("SUPERVISOR_STATE_CONNECTION_OPERATION -> SUPERVISOR_STATE_END\r\n"));

               // Exit from the switch (no more action)

               state = SUPERVISOR_STATE_END;

               break;

           }

           // Prepare next state controller

           state_check_connection = CONNECTION_CHECK_NTP;

           break;

A seconda dello stato di connessione si effettuano le operazioni
programmate. Si avvia inizialmente la connessione CONNECTION_INIT e
successivamente a connessione correttamente stabilita si procede con
l’operazione ad esempio di sincronizzazione NTP. In sequenza vengono
effettuate NTP, http e MQTT. NTP è automatico alla prima connessione e
successivamente viene richiamata 1 volta al giorno. http è su richiesta,
normalmente e bypassata ma su richiesta di una RPC locale o remota viene
inserita la richiesta per il controllo/download firmware e/o
configurazione. MQTT è praticamente sempre presente perché almeno lo
stato delle stazioni, anche in assenza di dati, viene trasmesso al
server remoto.

::

   case SUPERVISOR_STATE_CHECK_CONNECTION:

       // wait connection

       // Suspend TASK Controller for queue waiting portMAX_DELAY

       TaskState(state, UNUSED_SUB_POSITION, task_flag::suspended);

       if (param.connectionResponseQueue->Peek(&connection_response, portMAX_DELAY))

       {

           TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

           // ok connected

           if (connection_response.done_connected)

           {

               param.connectionResponseQueue->Dequeue(&connection_response);

               param.systemStatusLock->Take();

               param.system_status->connection.is_connected = true;

               param.system_status->connection.is_connecting = false;

               param.system_status->connection.is_disconnecting = false;

               param.system_status->connection.is_disconnected = false;

               param.systemStatusLock->Give();

               TRACE_INFO_F(F("%s Connection [ %s ]\r\n"), Thread::GetName().c_str(), OK_STRING);

               TRACE_VERBOSE_F(F("SUPERVISOR_STATE_CHECK_CONNECTION -> STATE_CONNECTION_OPERATION\r\n"));

               state = SUPERVISOR_STATE_CONNECTION_OPERATION;

           }

           // Error connection?

           else if (connection_response.error_connected) {

               retry++; // Add error retry

               param.connectionResponseQueue->Dequeue(&connection_response);

               param.systemStatusLock->Take();

Questa la fase di avvio connessione tramite coda e attesa risposta sul
supervisor in sospensione del task. Una volta che il task avviato (modem
in questo caso) avrà effettuato o meno la connessione, risponderà al
supervisore informandolo della riuscita o meno dell’operazione. Se la
connessione è andata a buon fine con lo stesso meccanismo verranno
innescati in sequenza e con le stesse identiche modalità i vari task di
rete necessari, ntp, http, mqtt. Al termine verrà inviato analogo
comando per il processo di disconnessione (questo solo per il task
modem) in modo da riposizionarci ad inizio in attesa di nuovo avvio
comunicazione.

Per la gestione della comunicazione fisica, ci si appoggia alla classe
sim7600 che gestisce ad alto livello le funzionalità del modulo SIM7600E
utilizzato nel progetto. La classe comprende tutti i metodi per la
gestione completa del modulo, a partire dall’alimentazione del
dispositivo passando dalla gestione dei dispositivi HW per la
comunicazione, tutti i comandi AT fino alla creazione di una connessione
PPP che viene passata al contesto CycloneTCP per i successivi task di
comunicazione. Nell’istanza della classe vengono passati i pin
utilizzati per l’accesso HW al modulo e le velocita di Baud RATE (di
avvio e di operatività). Il modulo infatti si avvia ad una velocità
della porta RS232 per poi passare ad un rate superiore che consente
l’utilizzo a pieno delle velocità offerte dai nuovi standard 4G. Il
tutto, gestione della porta RS232 e dei temporizzatori e clock sorgenti
necessari è come sempre del tutto trasparente al programma principale.

Task NTP HTTP MQTT
------------------

Prendiamo in esmpio un task tra quelli di comunicazione (NTP),
ricordando che tutti i task di questo ambito agiscono nello stesso modo,
ovviamente differenziandosi nei relativi metodi per la tipologia di
connessione da effettuare. Per quanto riguarda metodi e relative
configurazioni fare riferimento a https://www.oryx-embedded.com/doc/
dove l’ampia documentazione e guida permetterà di approfondire le
istruzioni e metodologie utilizzabili.

Come descritto precedentemente, nel progetto StimaV4, viene stabilita
una connessione PPP trasparente, tramite libreria Cyclone parzialmente
adattata al funzionamento sull’architettura STM32 con modem e relativi
comandi per SIM7600E SimCom. Una volta stabilità la connessione il tutto
diventa trasparente, la variabile yarrowContext del main contiene una
struttura dati accessibile ai moduli Cyclone e rendono disponibili tutte
le funzionalità della libreria, come ad. Es. http, Udp, Smpt ecc…

All’interno della directory delle librerie sono inserite le varie
funzionalità di CycloneTCP utilizzabili.

Tornando alla gestione dei comandi sul Task di StimaV4 (NTP), la
struttura che andiamo ad analizzare

::

   case NTP_STATE_INIT:

       TRACE_VERBOSE_F(F("NTP_STATE_INIT -> NTP_STATE_WAIT_NET_EVENT\r\n"));

       state = NTP_STATE_WAIT_NET_EVENT;

       break;

   case NTP_STATE_WAIT_NET_EVENT:

       is_error = false;

       retry = 0;

       // wait connection request

       // Suspend TASK Controller for queue waiting portMAX_DELAY

       TaskState(state, UNUSED_SUB_POSITION, task_flag::suspended);

       if (param.connectionRequestQueue->Peek(&connection_request, portMAX_DELAY))

       {

           TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

           // do ntp sync

           if (connection_request.do_ntp_sync)

           {

               param.connectionRequestQueue->Dequeue(&connection_request);

               TRACE_VERBOSE_F(F("NTP_STATE_WAIT_NET_EVENT -> NTP_STATE_DO_NTP_SYNC\r\n"));

               state = NTP_STATE_DO_NTP_SYNC;

           }

       }

       break;

   case NTP_STATE_DO_NTP_SYNC:

       sntpClientInit(&sntpClientContext);

       param.systemStatusLock->Take();

       param.system_status->connection.is_ntp_synchronizing = true;

       param.systemStatusLock->Give();

Successivamente alla fase di init, si entra in uno stato dormiente del
Task, in attesa del risveglio (comando del supervisor). Se richiesta una
sincronizzazione, viene avviato il relativo processo (is_syncronizing)
che terminerà dopo la corretta esecuzione di tutte le istruzioni
necessarie

::

       // Retrieve current time from NTP server

       TaskWatchDog(SNTP_CLIENT_TIMEOUT_MS);

       error = sntpClientGetTimestamp(&sntpClientContext, &timestamp);

       // Check status code

       if (!error)

       {

           // Unix time starts on January 1st, 1970

           unixTime = timestamp.seconds - NTP_UNIX_EPOCH;

           // Convert Unix timestamp to date

           convertUnixTimeToDate(unixTime, &date);


           // Set DateTime RTC With Semaphore Locked access

           if(param.rtcLock->Take()) {

               rtc.setEpoch((uint32_t) unixTime);

               param.rtcLock->Give();

           }

           state = NTP_STATE_END;

           TRACE_VERBOSE_F(F("NTP_STATE_DO_NTP_SYNC -> NTP_STATE_END\r\n"));

       }

       else if (error == ERROR_REQUEST_REJECTED)

       {

           // Retrieve kiss code

         ...

   case NTP_STATE_END:

       // ok

       if (!is_error)

       {

           param.systemStatusLock->Take();

           param.system_status->connection.is_ntp_synchronizing = false;

           param.system_status->connection.is_ntp_synchronized = true;

           param.system_status->flags.ntp_error = false;

           param.systemStatusLock->Give();

           sntpClientDeinit(&sntpClientContext);

           memset(&connection_response, 0, sizeof(connection_response_t));

           connection_response.done_ntp_synchronized = true;

           param.connectionResponseQueue->Enqueue(&connection_response);

           state = NTP_STATE_INIT;

           TRACE_VERBOSE_F(F("NTP_STATE_END -> NTP_STATE_INIT\r\n"));

       }

Il codice sopra è il termine corretto della sequenza comandi (con
impostazione data e ora e gestione del relativo semaforo di accesso),
infine viene inviata la risposta al task di supervisione dello stato
effettuato o meno del comando per le successive elaborazioni. Al termine
dell’invio della risposta il task ritorna dormiente in attesa di nuovo
avvio.

La stessa struttura di avvio, comando e sospensione è applicata agli
altri task http e mqtt.

I task MQTT e http, più complessi rispetto al task NTP, hanno al loro
interno le funzionalità di lettura e scrittura dati sulle code previste
verso il task SD (lettura dati e pubblicazione per MQTT), (scrittura
dati firmware http) e condividono l’accesso alle RPC, con la specifica
classe vista in precedenza per configurazione e comandi tramite comandi
JSON.

Analizzando per esempio http ci troviamo di fronte alla solita attesa
dell’avvio del task tramite coda, successivamente a seconda della
tipologia di comando richiesta (se soddisfatte le varie condizioni), si
passa allo stato di invio richiesta remota per le connessioni da
effettuare (firmware, configurazione) possibili. Altre eventuali
richieste potrebbero essere facilmente inserite in questo contesto.

::

   case HTTP_STATE_WAIT_NET_EVENT:

       is_get_configuration = false;

       is_get_firmware = false;

       is_error = false;

       retry = 0;

       // wait connection request

       // Suspend TASK Controller for queue waiting portMAX_DELAY

       TaskState(state, UNUSED_SUB_POSITION, task_flag::suspended);

       if (param.connectionRequestQueue->Peek(&connection_request, portMAX_DELAY))

       {

           TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

           HttpServer = param.configuration->mqtt_server;

           // do http get configuration (prioritary)

           if (connection_request.do_http_get_configuration)

           {

               is_get_configuration = true;

               param.connectionRequestQueue->Dequeue(&connection_request);

               state = HTTP_STATE_SEND_REQUEST;

               TRACE_VERBOSE_F(F("HTTP_STATE_WAIT_NET_EVENT -> HTTP_STATE_SEND_REQUEST (get configuration)\r\n"));

           }

           // do http get firmware

           else if (connection_request.do_http_get_firmware)

           {

               // SD have to GET Ready before Push DATA (Firmware download?! Exit immediatly)

               // EXIT from function if not SD Ready or present into system_status

               if(!param.system_status->flags.sd_card_ready) {

                   TRACE_VERBOSE_F(F("HTTP: Reject request upload file (Firmware) SD was not ready [ %s ]\r\n"), ERROR_STRING);

                   state = HTTP_STATE_END;

               } else {

                   is_get_firmware = true;

                   module_download = 0xFF; // Starting from Master

                   param.connectionRequestQueue->Dequeue(&connection_request);

                   state = HTTP_STATE_SEND_REQUEST;

                   TRACE_VERBOSE_F(F("HTTP_STATE_WAIT_NET_EVENT -> HTTP_STATE_SEND_REQUEST (get firmware)\r\n"));

               }

           }

       }

       break;

L’organizzazione a stati, così come proposta, contribuisce alla
manutenibilità del software, differenziando in modo semplice le varie
fasi di attività/connessione e le tempistiche di delay dei vari
contesti, tra operazioni semplici e quelle più complesse, che richiedono
al task maggiori tempi di funzionamento.

Dopo la fase di composizione della richiesta (http_state_send_request)
con tutte le gestioni interne ed eventuali anomalie, si entra
nell’interprete (http_state_get_response) dove le risposte ottenute dal
server vengono analizzate e processate

::

   if (is_get_configuration)

   {

       is_event_rpc = true;

       param.streamRpc->init();

       error = httpClientReadBody(&httpClientContext, http_buffer, sizeof(http_buffer) - 1, &http_buffer_length, SOCKET_FLAG_BREAK_CRLF);

       #if (ENABLE_STACK_USAGE)

       TaskMonitorStack();

       #endif

       if (!error)

       {

           // Security Remove flag config wait... Start success download

           if(param.system_status->flags.http_wait_cfg) {

               param.systemStatusLock->Take();

               param.system_status->flags.http_wait_cfg = false;

               param.systemStatusLock->Give();

           }

           http_buffer[http_buffer_length] = '\0';

           TRACE_INFO_F(F("%s"), http_buffer);

       }

       // Put RPC for configuration mode

       if (param.rpcLock->Take(Ticks::MsToTicks(RPC_WAIT_DELAY_MS)))

       {

           while (is_event_rpc)

           {

               #if (ENABLE_STACK_USAGE)

               TaskMonitorStack();

               #endif

               // Security lock task_flag for External Local TASK RPC (Need for risk of WDT Reset)

               param.system_status->tasks[LOCAL_TASK_ID].state = task_flag::suspended;

               param.streamRpc->parseCharpointer(&is_event_rpc, (char )http_buffer, http_buffer_length, NULL, 0, RPC_TYPE_HTTPS);

               param.system_status->tasks[LOCAL_TASK_ID].state = task_flag::normal;

               param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::set;

           }

           param.rpcLock->Give();

       }

       // Delay for command accept...

       TaskWatchDog(HTTP_TASK_RPC_WAIT_DELAY_MS);

       Delay(Ticks::MsToTicks(HTTP_TASK_RPC_WAIT_DELAY_MS));

   }

Per esempio, durante la fase di ricezione della configurazione (più
genericamente definibile in ricezione RPC tramite http) si hanno la
lettura del corpo messaggio http letto (le linee di comando RPC), il
passaggio tramite accesso semaforico alla classe di gestione delle RPC,
e la loro esecuzione con sospensione momentanea del controllo sul task
locale per operazioni che potrebbero essere di lunga durata.

Nel caso dell’aggiornamento firmware il blocco dati letto, viene inviato
ad una funzione specifica do_firmware_add_block invece che alla gestione
RPC, che si occupa del passaggio dei dati verso la memoria SD con il
sistema prescelto (Code). Si noti che la coda di attesa risposta blocca
il processo per un lasso di tempo FILE_IO_DATA_QUEUE_TIMEOUT, che
impedisce al sistema il blocco del programma sulla risposta per evitare
WatchDog o altre situazioni anomale, ma consente la verifica del
corretto scambio di dati. Questa modalità utilizzata in tutto il
progetto deve essere opportunamente programmata per essere compatibile
con i tempi generali di WatchDog ed eventuali tempi di sospensione.

::

   bool HttpTask::do_firmware_add_block(uint8_t block_addr, uint16_t block_len) {

       bool file_upload_error = false;

       // SD have to GET Ready before Push DATA

       // EXIT from function if not SD Ready or present into system_status

       if(!param.system_status->flags.sd_card_ready) {

           TRACE_VERBOSE_F(F("HTTP: Reject request upload file (Firmware) SD was not ready [ %s ]\r\n"), ERROR_STRING);

           return true;

       }

       // Add Data Chunck...

       // Next block is data_chunk + Lenght to SET (in this all 512 bytes)

       firmwareDownloadChunck.block_type = file_block_type::data_chunck;

       memcpy((char)firmwareDownloadChunck.block, (char)block_addr, block_len);

       firmwareDownloadChunck.block_lenght = block_len;

       // Push data request to queue SD

       param.dataFilePutRequestQueue->Enqueue(&firmwareDownloadChunck);

       // Waiting response from SD with TimeOUT

       memset(&sdcard_task_response, 0, sizeof(file_put_response_t));

       TaskWatchDog(FILE_IO_DATA_QUEUE_TIMEOUT);

       file_upload_error = !param.dataFilePutResponseQueue->Dequeue(&sdcard_task_response, FILE_IO_DATA_QUEUE_TIMEOUT);

       file_upload_error |= !sdcard_task_response.done_operation;

       return(file_upload_error);

   }

Per quanto riguarda Mqtt, analogamente agli altri task di connessione si
hanno in sequenza gli stati di inizializzazione mqtt, connessione al
server, pubblicazione degli stati dei flag, pubblicazione dei dati
disconnessione.

Per quanto riguarda la pubblicazione dei dati, questi vengono letti dai
moduli remoti nel formato nativo del messaggio Cyphal trasmesso e
salvati direttamente su SD dal task CanBus tramite l’apposita coda.

::

   // RMAP Casting value to Uavcan Structure

   rmap_service_module_TH_Response_1_0 rmapDataTH;

L’apposito casting in lettura della coda permette di avere la struttura
completa, come definita nei file header RMAP dsdl di Cyphal, completo di
dati e metadati sensore nel task mqtt, per le elaborazioni locali. Sotto
la gestione di un tipo dati radiation in esempio per la radiazione
solare. Tutti i moduli hanno una propria struttura con flags e misure
differenti ma il concetto di funzionamento è univoco in tutti i moduli.

::

   case Module_Type::radiation:

       rmapDataRadiation = (rmap_service_module_Radiation_Response_1_0 ) rmap_get_response.rmap_data.block;

       #if (ENABLE_STACK_USAGE)

       TaskMonitorStack();

       #endif

       // check if the sensor was configured or not

       for (uint8_t slaveId = 0; slaveId < BOARDS_COUNT_MAX; slaveId++)

       {

           if (param.configuration->board_slave[slaveId].module_type == Module_Type::radiation)

           {

               if (!error && param.configuration->board_slave[slaveId].is_configured[SENSOR_METADATA_DSA])

               {

                   error = publishSensorRadiation(&mqttClientContext, qos, rmapDataRadiation->DSA......

               }

               if (error) { ... }

La funzione publishSensor_(name_sensor) si occupa fisicamente di
preparare il messaggio nel formato RMAP e di postare il valore al server
remoto, verificandone corretta esecuzione o eventuali errori di
pubblicazione che sono gestiti nello stato principale della
pubblicazione dei record Mqtt.

Classe Canard
-------------

La classe canard, presente in ogni modulo (master e slave), contiene al
suo interno le funzionalità di gestione della libreria Canard, da quelle
più a “basso livello” (gestione memoria e frame di comunicazione) nei
metodi privati, a quella dei singoli comandi di “alto livello” nei
metodi pubblici.

Lo scopo della classe è di rendere più semplice l’accesso alle
funzionalità della comunicazione Cyphal su CanBus senza doversi occupare
delle inizializzazioni software dei moduli specifici bxCan, interrupt di
sistema, O1Heap per la gestione della memoria della libreria. Nel task
Can si accede a questa specifica classe e con qualche semplice chiamata
si hanno a disposizione tutti i comandi di gestione della connessione
Cyphal.

Internamente alla classe sono stati inseriti ulteriori metodi tramite
flag di verifica per l’invio di specifici messaggi e il monitoraggio
delle tempistiche di risposta (OK, in corso, timeOut) gestibili dal
programma principale (in questo caso dal Task CAN). Ogni comando di ogni
funzionalità prevista (trasmissione file, sincronizzazione data ora,
acesso ai registri ecc…) oltre agli specifici parametri della
particolare richiesta hanno i relativi metodi pending e timeout per
determinare lo stato e la disponibilità di una funzionalità.

A differenza della classe master che contiene tutti i name space delle
tipologie di sensori/dsdl definite per RMAP, le singole classi slave
avranno solamente le relative classi per il singolo modulo.

La classe ha inoltre al suo interno alcuni tipi definiti per la gestione
degli stati dei moduli. Di particolare interesse il VSC, Vendor Status
Code, definito da Cyphal come utilizzo privato nella comunicazione del
metodo Heartbeat. Questo codice di stato, è utilizzato in StimaV4 anche
per la comunicazione tra i moduli dell’attivazione o meno dello sleep
per il basso consumo. Quando il master richiede il full power ai moduli
remoti attiva il relativo flag che comunica agli slave l’impossibilità
di entrare in power down (es. quando viene aggiornato il firmware),
viceversa il flag remoto conferma che un modulo è in basso consumo e non
potrà interagire in comunicazione con comandi diretti. Se necessario
trasmettere un comando diretto, lo slave dovrà prima essere risvegliato
e alla conferma del flag rimosso potrà essere inviato il comando.

Al termine di tutto, se non necessaria una comunicazione particolare o
privilegiata, il master comunicherà agli slave di rientrare in power
down.

Questi messaggi di stato, tramite il flag VSC, vengono inviati nel
messaggio heartbeat di Cyphal, che come specificato dal protocollo dovrà
essere inviato entro due secondi prima di considerare un modulo OffLine.

::

   // Namespace RMAP

   #include <rmap/_module/TH_1_0.h>

   #include <rmap/service/_module/TH_1_0.h>

   #include <rmap/_module/Rain_1_0.h>

   #include <rmap/service/_module/Rain_1_0.h>

   #include <rmap/_module/Power_1_0.h>

   #include <rmap/service/_module/Power_1_0.h>

   #include <rmap/_module/Radiation_1_0.h>

   #include <rmap/service/_module/Radiation_1_0.h>

   #include <rmap/_module/VWC_1_0.h>

   #include <rmap/service/_module/VWC_1_0.h>

   #include <rmap/_module/Wind_1_0.h>

   #include <rmap/service/_module/Wind_1_0.h>

Le definizioni dei tipi se di carattere globale sono sempre posti
all’interno di local_typedef.h, altrimenti sono presenti nella
definizione dell’ header del task o dellla relativa classe o modulo.

::

   // Power mode (Canard and general Node)

   enum Power_Mode : uint8_t {

       pwr_on,        // Never (All ON, test o gestione locale)

       pwr_nominal,   // Every Second (Nominale base)

       pwr_deep_save, // Deep mode (Very Low Power)

       pwr_critical   // Deep mode (Power Critical, Save data, Power->Off)

   };

Moduli SLAVE
------------

Come già detto in precedenza, i moduli slave si differenziano dal master
perché sono l’interfaccia verso il sensore e verso il master. I concetti
espressi in precedenza riguardanti l’architettura software utilizzata
rimangono gli stessi del Master, vedi FreeRTOS, LPTim, sleep,
comunicazioni Task, code, semafori, ecc…

La parte di interfacciamento verso il sensore ha ovviamente un task
specifico differente tra i vari moduli (rain, th, mppt, wind ecc.) che
fisicamente si occupa di acquisire i valori real time dal sensore in
campo e rendere questo dato disponibile al task che effettua le
elaborazioni.

I dati acquisiti sono poi inseriti in un buffer circolare che è letto
dal task delle elaborazioni per calcolare le elaborazioni specifiche del
sensore. Anche il task elaborazione è in parte differente tra i moduli
per consentire il calcolo di elaborazioni specifiche come ad.es quelle
relative al vento velocità e direzione, ma il concetto di buffer valori,
code di richiesta valore istantaneo, di inserimento è richiesta dati è
identico tra tutti gli slave. Identico rimane inolte il task CAN che
pubblica le sue info e attende dal master i vari comandi per interagire
con le elaborazioni su comandi del master.

Anche in questo caso abbiamo il concetto di configurazione locale del
modulo (tramite i registri Cyphal e le classi già approfondite) che
contiene le informazioni rispetto al modulo utilizzato e ai parametri di
gestione che viene caricato e reso disponibile all’intero modulo tramite
configuration. SystemStatus si occupa sempre dello stato del sistema
“modulo”.

Per aggiungere un modulo slave al sistema, bisognerà partendo da un
modulo esistente, preoccuparsi di sviluppare la parte di acquisizione
del sensore

::

   #if ((MODULE_TYPE == STIMA_MODULE_TYPE_THR) || (MODULE_TYPE == STIMA_MODULE_TYPE_TH))

        static TemperatureHumidtySensorTask th_sensor_task("THTask", 400, OS_TASK_PRIORITY_03, thSensorParam);

   #endif

Creando ed aggiungendo nel file di configurazione config.h il nuovo
modulo e tutte le sue definizioni poi abilitandolo al posto di quello di
Temperatura in questo caso.

::

   #if (MODULE_TYPE == STIMA_MODULE_NUOVO)

        static ModuloNuovoSensorTask th_sensor_task("ModuloNuovoTask", (SIZE_TASK), OS_TASK_PRIORITY_03, NewParam);

   #endif

“Modulo nuovo” dovrà preoccuparsi di adattare le funzionalità gestionale
come ad esempio le funzioni PowerOn e PowerOff per spegnere il sensore
ed entrare in basso consumo se possibile per quella tipologia di
sensore, passare i parametri corretti al task di gestione agendo sulla
struttura locale moduloConfigParam adattata alle esigenze (vedi sotto),
passare i semafori e i dispositivi hw necessari. Nel caso del sensore TH
oltre a confiurazione e stato di sistema, TwoWire chè è l’accesso a I2C
e i relativi semafori di utilizzo della risorsa.

Successivamente è necessario aggiornare gli stati possibili di
funzionamento per creare le varie fasi di attività del task (nel caso in
esame il sensore TH con interfacciamento SensorDriver prevede la fase di
CreazioneTask, attesa della configurazione disponibile, inizializzazione
dispositivi, setup sensori, prepare e read specifici di SensorDriver,
End ed eventuali stati di errore e loro risoluzione) con quelli
necessari.

::

   using namespace cpp_freertos;


   // Parametri del task

   typedef struct {

        configuration_t configuration;

        system_status_t system_status;

        TwoWire wire;

        cpp_freertos::BinarySemaphore wireLock;

        cpp_freertos::BinarySemaphore configurationLock;

        cpp_freertos::BinarySemaphore systemStatusLock;

        cpp_freertos::Queue systemMessageQueue;

        cpp_freertos::Queue elaborateDataQueue;

   } TemperatureHumidtySensorParam_t;

   class TemperatureHumidtySensorTask : public cpp_freertos::Thread {


   // Stati associati allo switch generale del TASK (fase software)

   typedef enum

   {

        SENSOR_STATE_CREATE,

        SENSOR_STATE_WAIT_CFG,

        SENSOR_STATE_INIT,

        SENSOR_STATE_SETUP,

        SENSOR_STATE_PREPARE,

        SENSOR_STATE_READ,

        SENSOR_STATE_END,

        SENSOR_STATE_CHECK_ERROR

   } State_t;

   public:

        TemperatureHumidtySensorTask(const char taskName, uint16_t stackSize, uint8_t priority, TemperatureHumidtySensorParam_t temperatureHumidtySensorParam);

   protected:

        virtual void Run();

   private:

   #if (ENABLE_STACK_USAGE)

        void TaskMonitorStack();

   #endif

        void TaskWatchDog(uint32_t millis_standby);

        void TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation);

        void powerOn();

        void powerOff();

        bool is_power_on;

        State_t state;

        TemperatureHumidtySensorParam_t param;

        SensorDriver sensors[SENSORS_COUNT_MAX];

   };

Infine dopo aver creato la sequenza di lettura e acquisizione,
interagire con la coda buffer dati per le successive elaborazioni
(prendendo in esame sempre il modulo TH la parte che si occupa di
inserire il dato nel buffer a scorrimento)

::

   edata.value = values_readed_from_sensor[1];

   edata.index = param.configuration->sensors[i].is_redundant ? TEMPERATURE_REDUNDANT_INDEX : TEMPERATURE_MAIN_INDEX;

   param.elaborateDataQueue->Enqueue(&edata, Ticks::MsToTicks(WAIT_QUEUE_REQUEST_PUSHDATA_MS));

   is_temperature_redundant = param.configuration->sensors[i].is_redundant;

L’altro passaggio da effettuare è quello di creare il tipo di dato
Cyphal relativo (come detto in precedenza questo protocollo è
particolarmente orientato ad un’oggetto quindi ad un sensore con tutt i
i suoi stati). Per questo bisogna creare e compilare una DSDL che
contiene dati, metadati flag e metodi richiesti da quel sensore. Come
creare ed utilizzare una DSDL è spiegato sempre su openCyphal.org, ma a
titolo illustrativo inseriamo la DSDL che dà origine alla struttura
temperatura RMAP Cyphal. Una volta compilata la DSDL ed importato nel
programma il relativo file .h vengono messe a disposizione del programma
le funzioni di interpretazione compressione e decompressione dei dati
per il trasporto su CAN (o altri metodi).

DSDL Modulo di TH (Temperatura + umidità + metadati)

::

   rmap.metadata.Metadata.1.0 metadata

   rmap.measures.Temperature.1.0 temperature

   rmap.measures.Humidity.1.0 humidity

   @sealed

DSDL Misura di Temperatura (Tabella Temperatura + tabella confidence per
attendibilità della misura)

::

   rmap.tableb.B12101.1.0 val

   rmap.tableb.B33199.1.0 confidence

   @sealed

DSDL Tabella B12101 da RMAP (definizione di temperatura bit utilizzati
ed eventuali limiti)

::

   # Temperatura aria (°K)

   # fattore scala x100 + 27315

   # MAX 250.0 °C

   uint16 MAX = 52315

   uint16 value

   @sealed

Al termine della stesura e compilazione della DSDL segue l’importazione
degli header per il nuovo modulo, tabella RMAP e misura e si rende
necessario l’adattamento della classe_canard_nuovo_modulo. In sostanza
lasciando inalterati tutti i metodi classici di Cyphal già inseriti
(partendo dall’esempio di canard_class_th ci si deve solamente
preoccupare di sostituire il modulo_th con il nuovo creato, per la
memorizzazione dati e l’inserimento dei metodi/misure per l’avvio delle
regolari sottoscrizioni Cyphal.

A questo punto tutti gli strumenti sono pronti e disponibili per creare
un nuovo modulo su StimaV4 ed abilitarlo nel master.

Sensor TASK
-----------

Analizzando più nel dettaglio un singolo modulo (prendiamo in esame
quello della temperatura) possiamo vedere dal file config.h la
configurazione del modulo dove vengono definiti i dispositivi HW da
abilitare, gli indirizzi EEprom per la registrazione/lettura della
configurazione, i dimensionamenti dei buffer e le temporizzazioni per i
task di elaborazione, le temporizzazioni per i metodi di basso consumo e
i limiti fisici del sensore collegato. Tutte le variabili sono
ampiamente commentate e laddove il commento non è presente è perché il
nome della variabile indica già in modo esaustivo la sua funzionalità.

Sensor_config.h invece entra nel dettaglio della tipologia di sensore e
si occupa di definire la tipologia del sensore utilizzato e l’eventuale
modalità ridondante. Stima V4 ha questa definizione REDUNDANT per
aggiungere un sensore ridondante al principale dove previsto. Dalla
configurazione è necessario abilitare il modulo utilizzato per
l’integrazione con SensorDriver di StimaV4.

La configurazione di default inizializza i metodi per la libreria che
vengono salvati sui registri Cyphal “SHT” per esempio per quanto
riguarda il tipo di sensore, “I2C” per il tipo di driver. Questi vengono
letti all’avvio del modulo e la loro modifica prevede l’abilitazione
mediante la libreria per il tipo di sensore, modificando per esempio
nell’ apposito registro della tipologia del sensore in “HYT” si
modificherà il protocollo e la classe di riferimento per abilitare lo
specifico sensore.

Per quanto riguarda il sensore SHT utilizzato nel progetto, è stata
redatta una nuova classe che supporta tutti i metodi di utilizzati da
SensorDriver (prepare, get ecc..) per renderli disponibili alla classe
SensorDriver nei metodi privati. Il tipo di trasporto differente
rispetto alla gestione di StimaV3, le modalità di configurazione e di
collegamento tra i moduli ha fatto perdere in parte di significato
all’indirizzo I2C dei moduli (che erano il bus di comunicazione e quindi
il motore del sistema) e anche dei metodi utilizzati, relegando
l’indirizzo I2C a semplice accesso al sensore e non al modulo in sè.

All’interno della libreria SHT, come in quella HYT, nelle relative
definizioni sht.h hyt.h è possibile modificare gli indirizzi I2C fisici
di accoppiamento per i sensori base ed eventuale ridondante per il loro
accesso. L’utilizzo di altri sensori tipo ad.es. come SENSOR_STH,
SENSOR_ITH, SENSOR_MTH, SENSOR_NTH, SENSOR_XTH utilizzati nei moduli di
test sviluppati ad hoc per la verifica delle elaborazioni sono stati
importati nella classe adattandone la chiamata dei metodi. Nella
libreria RMAP sono disponibili nei file registers, registers-th,
registers-th_v2 indirizzi comandi e metodi per la configurazione dei
metodi/sensori sopradescritti.

Analizziamo le varie fasi in sequenza per SensorDriver nel task del
sensore th_sensor_task

::

   Fase di inizializzazione del driver I2C e passaggio a SETUP

   case SENSOR_STATE_INIT:

   TRACE_INFO_F(F("Initializing sensors...\r\n"));

   for (uint8_t i = 0; i < param.configuration->sensors_count; i++)

   {

        if (strlen(param.configuration->sensors[i].type) == 3)

        {

             SensorDriver::createSensor(SENSOR_DRIVER_I2C, param.configuration->sensors[i].type, param.configuration->sensors[i].i2c_address, 1, sensors, param.wire);

        }

   }

   state = SENSOR_STATE_SETUP;

Fase di Setup (con eventuale powerON se utilizzabile), reset stati I2C e
passaggio a PREPARE

::

   case SENSOR_STATE_SETUP:

        // Turn ON ( On Power ON/OFF if Start or Reset after Error Or Always if not use LOW_POWER Method )

        if(!is_power_on) powerOn();

        param.wireLock->Take();

        param.wire->end();

        param.wire->begin();

        param.wireLock->Give();

        is_test = false;

        memset((void )values_readed_from_sensor, RMAPDATA_MAX, (size_t)(VALUES_TO_READ_FROM_SENSOR_COUNT  sizeof(rmapdata_t)));

        for (uint8_t i = 0; i < SensorDriver::getSensorsCount(); i++)

        {

             if (!sensors[i]->isSetted())

             {

                  param.wireLock->Take();

                  sensors[i]->setup();

                  param.wireLock->Give();

             }

        }

        state = SENSOR_STATE_PREPARE;

        break;

Fase di Prepare (con gestione errori), attesa prevista dal Driver e
passagio a lettura

::

   case SENSOR_STATE_PREPARE:

        delay_ms = 0;

        for (uint8_t i = 0; i < SensorDriver::getSensorsCount(); i++)

        {

             sensors[i]->resetPrepared();

             param.wireLock->Take();

             sensors[i]->prepare(is_test);

             param.wireLock->Give();

             // wait the most slowest

             if (sensors[i]->getDelay() > delay_ms) { delay_ms = sensors[i]->getDelay(); }

             // end with error prepare

             param.systemStatusLock->Take();

             param.system_status->events.measure_count++;

             if (!sensors[i]->isSuccess())

             {

                 error_count++;

                  param.system_status->events.perc_i2c_error = (uint8_t)...

             }

             // Local WatchDog update;

             TaskWatchDog(delay_ms);

             Delay(Ticks::MsToTicks(delay_ms));

             state = SENSOR_STATE_READ;

        break;

Fase di Lettura (con gestione errori), inserimento dati in buffer,
attesa prevista dalla configurazione e riavvio

::

   case SENSOR_STATE_READ:

        is_temperature_redundant = false;

        is_humidity_redundant = false;

        for (uint8_t i=0; i<SensorDriver::getSensorsCount(); i++) {

            do {

                  param.wireLock->Take();

                  sensors[i]->get(&values_readed_from_sensor[0], VALUES_TO_READ_FROM_SENSOR_COUNT, is_test);

                  param.wireLock->Give();

                  // Secure WDT

                  TaskWatchDog(sensors[i]->getDelay());

                  Delay(Ticks::MsToTicks(sensors[i]->getDelay()));

             } while (!sensors[i]->isEnd() && !sensors[i]->isReaded());

             // end with error measure

             param.systemStatusLock->Take();

             param.system_status->events.measure_count++;

             if (!sensors[i]->isSuccess())

             {

                  error_count++;

                  ...
            }

             param.system_status->events.perc_i2c_error = (uint8_t)

             if (false) {...}

             #if (USE_SENSOR_ITH)||(USE_SENSOR_ITH_V2)

             #if (USE_SENSOR_ADT)

             ...

             #if (USE_SENSOR_SHT)

             else if (strcmp(sensors[i]->getType(), SENSOR_TYPE_SHT) == 0) {

                  edata.value = values_readed_from_sensor[1];

                  edata.index = param.configuration->sensors[i].is_redundant ? TEMPERATURE_REDUNDANT_INDEX : TEMPERATURE_MAIN_INDEX;

                  param.elaborateDataQueue->Enqueue(&edata, Ticks::MsToTicks(WAIT_QUEUE_REQUEST_PUSHDATA_MS));

                  is_temperature_redundant = param.configuration->sensors[i].is_redundant;

                  ...

             #endif

Inserimento in coda buffer dati e passaggio a END (fine acquisizione)

::

        // If module fail fill void error data

        if (!is_temperature_redundant) {

             edata.value = RMAPDATA_MAX;

             edata.index = TEMPERATURE_REDUNDANT_INDEX;

             param.elaborateDataQueue->Enqueue(&edata, Ticks::MsToTicks(WAIT_QUEUE_REQUEST_PUSHDATA_MS));

        }

        // If module fail fill void error data

        if (!is_humidity_redundant) {

             edata.value = RMAPDATA_MAX;

             edata.index = HUMIDITY_REDUNDANT_INDEX;

             param.elaborateDataQueue->Enqueue(&edata, Ticks::MsToTicks(WAIT_QUEUE_REQUEST_PUSHDATA_MS));

        }

        state = SENSOR_STATE_END;

   break;

END e riavvio procedura (gestione OFF sensore se prevista), messa in
standBy per il tempo previsto dal driver e reinit procedura (nel caso di
nuovo PowerON, si rieffettua anche tutta la fase di Setup sensore)

::

   case SENSOR_STATE_END:

        #if (TH_TASK_LOW_POWER_ENABLED) && (!USE_SENSOR_ITH_V2)

        powerOff();

        #endif

        #if (ENABLE_STACK_USAGE)

        TaskMonitorStack();

        #endif

        // Local TaskWatchDog update and Sleep Activate before Next Read

        TaskWatchDog(param.configuration->sensor_acquisition_delay_ms);

        TaskState(state, UNUSED_SUB_POSITION, task_flag::sleepy);

        DelayUntil(Ticks::MsToTicks(param.configuration->sensor_acquisition_delay_ms));

        TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

        // Check next state if need to Restart Power and new call setup method

        if(is_power_on) {

             state = SENSOR_STATE_PREPARE;

        } else {

             state = SENSOR_STATE_SETUP;

        }

   break;

Task Elaborate
--------------

Per ultimo andiamo ad approfondire il task di elaborazione. Come già
descritto questo task effettua elaborazioni su richiesta esterna tramite
coda comandi e si occupa dell’archiviazione dei dati sulle strutture di
buffer. Nel main del task si attendono appunto queste due code che ne
gestiscono il funzionamento.

Coda di attesa per riempimento buffer a scorrimento dei dati

::

   // enqueud from th sensors task (populate data)

   if (!param.elaborateDataQueue->IsEmpty()) {

        if (param.elaborateDataQueue->Peek(&edata, 0))

        {

             param.elaborateDataQueue->Dequeue(&edata);

             switch (edata.index)

             {

                  case TEMPERATURE_MAIN_INDEX:

                       TRACE_VERBOSE_F(F("Temperature [ %s ]: %d\r\n"), MAIN_STRING, edata.value);

                       addValue<maintenance_t, uint16_t, bool>(&maintenance_samples, SAMPLES_COUNT_MAX, param.system_status->flags.is_maintenance);

                       addValue<sample_t, uint16_t, rmapdata_t>(&temperature_main_samples, SAMPLES_COUNT_MAX, edata.value);

                       break;

                  case TEMPERATURE_REDUNDANT_INDEX:

                       TRACE_VERBOSE_F(F("Temperature [ %s ]: %d\r\n"), REDUNDANT_STRING, edata.value);

                       addValue<sample_t, uint16_t, rmapdata_t>(&temperature_redundant_samples, SAMPLES_COUNT_MAX, edata.value);

                       break;

                  case HUMIDITY_MAIN_INDEX:

                       TRACE_VERBOSE_F(F("Humidity [ %s ]: %d\r\n"), MAIN_STRING, edata.value);

                       addValue<sample_t, uint16_t, rmapdata_t>(&humidity_main_samples, SAMPLES_COUNT_MAX, edata.value);

                       ...

             }

        }

   }

Nel primo indice si può vedere un buffer particolare maintenance_sampe
chè è un buffer circolare di dati bool per segnalare se il dato indice
del buffer dei sensori (Temperatura e Umidità primari e ridondanti) sono
validi oppure in fase di manutenzione. Se correlati ad un’attività di
manutenzione temporale, attivabile dall’operatore locale tramite
apposito comando su display, questi dati non saranno ritenuti idonei nel
calcolo delle elaborazioni (vale ovviamente per tutti i moduli RAIN,
WIND ecc…).

Attivazione eleaborazione tramite coda.

::

   // enqueued from can task (get data, start command...)

   if (!param.requestDataQueue->IsEmpty()) {

        if (param.requestDataQueue->Peek(&request_data))

        {

             // send request to elaborate task (all data is present verified on elaborate_task)

             param.requestDataQueue->Dequeue(&request_data);

             make_report(request_data.is_init, request_data.report_time_s, request_data.observation_time_s);

             param.reportDataQueue->Enqueue(&report);

        }

   }

Qualsiasi task può avviare una richiesta di elaborazione sulla coda
request_data ed attendere sulla relativa coda la risposta dati nella
struttura report. La definizione delle report sono ovviamente dipendenti
dalla tipologia di modulo e racchiudono tutte le elaborazioni del
relativo sensore. La struttura di request è invece identica tra i moduli
e tramite i parametri dei tempi di acquisizione e osservazione è in
grado in qualsiasi momento di dare una risposta dinamica al chiamante in
merito alle elaborazioni anche sulla base di tempi diversi (la
sincronizzazione del tempo è garantita dal metodo timeSyncronization di
Cyphal). In questo modo è possibile oltre al calcolo su tempi fissi di
acquisizione dati della configurazione es. 900 secondi, una estemporanea
richiesta remota proveniente da altra funzione per il calcolo su 1 ora
completa di dati con osservazioni al minuto o altro intervallo a scelta,
senza compromettere nulla sui dati in memoria.

All’inizio vengono prelevati i dati dalla coda a ritroso partendo
dall’ultimo ed effettuare una corretta sequenza di accesso ai dati per
il calcolo delle elaborazioni sulla base dei tempi richiesta, procedendo
da un sample all’altro fino alla generazione di un’osservazione con una
media progressiva. Il prelievo è effettuato con verifica dato e
manutenzione, conteggio sample, recupero dato “mediato” in caso di
ridondanza sensori

::

   // GET SAMPLE VALUE DATA FROM AND CREATE OBSERVATION VALUES FOR TYPE SENSOR

   main_temperature_s = bufferReadBack<sample_t, uint16_t, rmapdata_t>(&temperature_main_samples, SAMPLES_COUNT_MAX);

   #if (USE_REDUNDANT_SENSOR)

   redundant_temperature_s = bufferReadBack<sample_t, uint16_t, rmapdata_t>(&temperature_redundant_samples, SAMPLES_COUNT_MAX);

   #endif

   total_count_main_temperature_s++;

   avg_main_temperature_quality_s += (float)(((float)checkTemperature(main_temperature_s, redundant_temperature_s) - avg_main_temperature_quality_s) / total_count_main_temperature_s);

   #if (USE_REDUNDANT_SENSOR)

   // Only after calculate quality... Reget data for better choiche from main and redundant value (avg if is ok...)

   main_temperature_s = getBetterTemperature(main_temperature_s, redundant_temperature_s);

   #endif

   if ((ISVALID_RMAPDATA(main_temperature_s)) && !measures_maintenance)

   {

        valid_count_main_temperature_s++;

        valid_count_main_temperature_t++;

        ist_main_temperature_setted = true;

        avg_main_temperature_s += (float)(((float)main_temperature_s - avg_main_temperature_s) / valid_count_main_temperature_s);

Quando generata una osservazione viene aggiunta e registrata per
arrivare al calcolo del report complessivo. Le variabili vengono
reinizializzate per determinare il valore della successiva osservazioone
fino al raggiungimento del totale delle osservazioni previste per il
calcolo del report

::

   // ELABORATE OBSERVATION VALUES FOR TYPE SENSOR FOR PREPARE REPORT RESPONSE

   if(is_observation) {

        n_observation++;

        // Elaboration IST for observation (first observation in bufferBack)

        if (n_observation == 1) report.temperature.ist = avg_main_temperature_s; // Elaboration sample -> Last data

        // main_temperature, sufficient number of valid samples?

        valid_data_calc_perc = (float)(valid_count_main_temperature_s) / (float)(total_count_main_temperature_s)  100.0;

        if (valid_data_calc_perc >= SAMPLE_ERROR_PERCENTAGE_MIN)

        {

             valid_count_main_temperature_o++;

             avg_main_temperature_o += (avg_main_temperature_s - avg_main_temperature_o) / valid_count_main_temperature_o;

             avg_main_temperature_quality_o += (avg_main_temperature_quality_s - avg_main_temperature_quality_o) / valid_count_main_temperature_o;

             // Elaboration MIN and MAX for observation

             if(avg_main_temperature_s < min_main_temperature_o) min_main_temperature_o = avg_main_temperature_s;

             if(avg_main_temperature_s > max_main_temperature_o) max_main_temperature_o = avg_main_temperature_s;

        }

        // Reset Buffer sample for calculate next observation

        avg_main_temperature_quality_s = 0;

        avg_main_temperature_s = 0;

        valid_count_main_temperature_s = 0;

        total_count_main_temperature_s = 0;

   ...

Al termine se le percentuali di dati che compongo l’osservazione e la
relativa percentuale che compongono il report soddisfano i parametri
minimi configurati in config.h del sensore, il report è valido e viene
passato in risposta

::

   #define SAMPLE_ERROR_PERCENTAGE_MIN      (90.0)

   #define OBSERVATION_ERROR_PERCENTAGE_MIN (90.0)

Altrimenti il report sarà composto da una struttura di valori nulli per
indicare l’assenza di elaborazione.

Documentazione del codice
*************************

The C++ API documentation for MASTER is available as `doxygen documentation </stima_v4/doxygen_v4/master/html/index.html>`__.

The C++ API documentation for slave TH is available as `doxygen documentation </stima_v4/doxygen_v4/slave-th/html/index.html>`__. 

The C++ API documentation for slave RAIN is available as `doxygen documentation </stima_v4/doxygen_v4/slave-rain/html/index.html>`__. 

The C++ API documentation for slave RADIATION is available as `doxygen documentation </stima_v4/doxygen_v4/slave-radiation/html/index.html>`__. 

The C++ API documentation for slave WIND is available as `doxygen documentation </stima_v4/doxygen_v4/slave-wind/html/index.html>`__. 

The C++ API documentation for slave VWC is available as `doxygen documentation </stima_v4/doxygen_v4/slave-vwc/html/index.html>`__. 

The C++ API documentation for slave MPPT is  available as `doxygen documentation </stima_v4/doxygen_v4/slave-mppt/html/index.html>`__. 
