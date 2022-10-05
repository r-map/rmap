#if defined(TEST_UAVCAN_NODE_ONLINE) || defined(TEST_UAVCAN_NODE_OFFLINE)
#if defined(TEST_UAVCAN_NODE_ONLINE) && defined(TEST_UAVCAN_NODE_OFFLINE)
#error Definire solo una modalità di TEST (ONLINE oppure OFFLINE)
#endif

// Arduino
#include <Arduino.h>
// Unity Test
#include <Unity.h>
// Libcanard
#include <canard.h>
#include <o1heap.h>

#include "bxcan.h"
#include "register.hpp"
// Namespace UAVCAN
#include <uavcan/_register/Access_1_0.h>
#include <uavcan/_register/List_1_0.h>
#include <uavcan/file/Read_1_1.h>
#include <uavcan/node/ExecuteCommand_1_1.h>
#include <uavcan/node/GetInfo_1_0.h>
#include <uavcan/node/Heartbeat_1_0.h>
#include <uavcan/node/port/List_0_1.h>
#include <uavcan/pnp/NodeIDAllocationData_1_0.h>
#include <uavcan/time/Synchronization_1_0.h>
// Namespace RMAP
#include <rmap/_module/TH_1_0.h>
#include <rmap/service/_module/TH_1_0.h>
// Standard Library
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
// Configurazione modulo, definizioni ed utility generiche
#include "module_config.hpp"

//****************************************************************************************************
//******************************** FUNCTION DECLARATIONS *********************************************
//****************************************************************************************************
void test_slave_is_online(void);
void test_slave_is_offline(void);
//****************************************************************************************************

//****************************************************************************************************
// ******************************* ENUM STATE OF TEST ************************************************
//****************************************************************************************************
enum test_status {
    INIT,
    ONLINE,
    OFFLINE
};
//****************************************************************************************************

//****************************************************************************************************
//************************************* STATE STRUCTURE **********************************************
//****************************************************************************************************
typedef struct State {
    CanardMicrosecond started_at;
    uint32_t lastMicrosecond;
    uint64_t currentMicrosecond;

    O1HeapInstance* heap;
    CanardInstance canard;
    CanardTxQueue canard_tx_queues[CAN_REDUNDANCY_FACTOR];

    // Gestione master/server e funzioni attive in RX messaggi
    // Master Locale
    struct {
        // Time stamp
        struct {
            CanardMicrosecond previous_tx_real;
            bool enable_immediate_tx_real;
        } timestamp;
        // File upload (node_id può essere differente dal master, es. yakut e lo riporto)
        // AGgiungo file_server_node_id
        struct
        {
            uint8_t server_node_id;
            char filename[FILE_NAME_SIZE_MAX];
            bool is_firmware;
            bool updating;
            bool updating_eof;
            bool updating_run;
            byte updating_retry;
            uint64_t offset;
            uint64_t timeout_us;  // Time command Remoto x Verifica deadLine Request
            bool is_pending;      // Funzione in pending (inviato, attesa risposta o timeout)
            bool is_timeout;      // Funzione in timeout (mancata risposta)
        } file;
    } master;

    // Stato dei nodi slave e servizi gestiti collegati. Possibile lettura dai registri e gestione automatica
    struct
    {
        // Parametri Statici da leggere da registri o altro. Comunque inviati dal Master
        // node_id è l'indirizzo nodo remoto sui cui gestire i servizi
        // node_type identifica il tipo di nodo per sapere che tipologia di gestione viene effettuata
        // service è il port id sul node_id remoto che risponde ai servizi relativi di request
        // publisher è il subject id sul node_id remoto che pubblica i dati quando attivato
        // Gli altri parametri sono dinamicamente gestiti durante il funzionamento del programma
        uint8_t node_id;
        uint8_t node_type;
        // Nodo OnLine / OffLine
        bool is_online;
        struct {
            uint8_t state;        // Vendor specific code NodeSlave State
            uint8_t healt;        // Uavcan healt_state remoto
            uint64_t timeout_us;  // Time heartbeat Remoto x Verifica OffLine
        } heartbeat;
        struct {
            // Flag di assegnamento PNP (se TRUE, da scrivere in ROM NON VOLATILE / REGISTER)
            // Il nodo è stato assegnato e il pnp non deve essere eseguito per quel modulo
            // se false il pnp è eseguito fino a quando non rilevo un modulo compatibile
            // con la richiesta (MODULE_TH, MODULE_RAIN ecc...) appena ne trovo uno
            // Heartbeat pubblicati Rx<-(Stato nodo remoto)
            // Questo permette l'assegmaneto PNP di tutti i moduli in sequenza (default)
            // O l'aggiunta successiva previa modifica della configurazione remota
            bool is_assigned;  // flag resettato (PNP in funzione)
        } pnp;
        // Comandi inviati da locale Tx->(Comando + Param) Rx<-(Risposta)
        struct {
            uint8_t response;          // Stato di risposta ai comandi nodo
            uint64_t timeout_us;       // Time command Remoto x Verifica deadLine Request
            bool is_pending;           // Funzione in pending (inviato, attesa risposta o timeout)
            bool is_timeout;           // Funzione in timeout (mancata risposta)
            uint8_t next_transfer_id;  // Transfer ID associato alla funzione
        } command;
        // Accesso ai registri inviati da locale Tx->(Registro + Value) Rx<-(Risposta)
        struct {
            uavcan_register_Value_1_0 response;  // Valore in risposta al registro x Set (R/W)
            uint64_t timeout_us;                 // Time command Remoto x Verifica deadLine Request
            bool is_pending;                     // Funzione in pending (inviato, attesa risposta o timeout)
            bool is_timeout;                     // Funzione in timeout (mancata risposta)
            uint8_t next_transfer_id;            // Transfer ID associato alla funzione
        } register_access;
        // Accesso ai dati dello slave in servizio Tx->(Funzione) Rx<-(Dato + Stato)
        // Puntatore alla struttura dati relativa es. -> rmap_module_TH_1_0 ecc...
        struct {
            CanardPortID port_id;      // Porta del servizio dati correlato
            void* module;              // Dati e stato di risposta ai dati nodo
            uint64_t timeout_us;       // Time getData Remoto x Verifica deadline Request
            bool is_pending;           // Funzione in pending (inviato, attesa risposta o timeout)
            bool is_timeout;           // Funzione in timeout (mancata risposta)
            uint8_t next_transfer_id;  // Transfer ID associato alla funzione
        } rmap_service;
        // Nome file(locale) per aggiornamento file remoto e relativo stato di funzionamento
        struct {
            char filename[FILE_NAME_SIZE_MAX];
            bool is_firmware;     // Comunico se file in TX è un Firmware o altro
            uint8_t state;        // Stato del file transfer (gestione switch interno)
            uint64_t timeout_us;  // Time command Remoto x Verifica deadLine Request File
            bool is_pending;      // Funzione in pending (inviato, attesa risposta o timeout)
            bool is_timeout;      // Funzione in timeout (mancata risposta)
        } file;
// Pubblicazione dei dati autonoma (struttura dati come per servizio)
#ifdef USE_SUB_PUBLISH_SLAVE_DATA
        struct {
            CanardPortID subject_id;  // Suject id associato alla pubblicazione
            uint16_t data_count;      // Conteggio pubblicazioni remote autonome (SOLO x TEST)
        } publisher;
#endif
    } slave[MAX_NODE_CONNECT];

    // Abilitazione delle pubblicazioni falcoltative sulla rete (ON/OFF a richiesta)
    struct
    {
        bool port_list;
    } publisher_enabled;

    // Tranfer ID (CAN Interfaccia ID -> uint8) servizi attivi del modulo locale
    struct
    {
        uint8_t uavcan_node_heartbeat;
        uint8_t uavcan_node_port_list;
        uint8_t uavcan_file_read_data;
        uint8_t uavcan_time_synchronization;
    } next_transfer_id;

    // Flag di state
    struct
    {
        bool g_restart_required;  // Forzatura reboot del nodo
    } flag;
} State;
//****************************************************************************************************

//****************************************************************************************************
//********************************** GLOBAL VARIABLES/CONSTANTS **************************************
//****************************************************************************************************
State state = {0};
byte queueId;
test_status test_state = INIT;
//****************************************************************************************************

// ***************************************************************************************************
// **********             Funzioni ed utility generiche per gestione UAVCAN                 **********
// ***************************************************************************************************

// Ritorna l'indice della coda master allocata in state in funzione del nodeId fisico
byte getQueueNodeFromId(State* const state, CanardNodeID nodeId) {
    // Cerco la corrispondenza node_id nella coda allocata master per ritorno queueID Index
    for (byte queueId = 0; queueId < MAX_NODE_CONNECT; queueId++) {
        // Se trovo il nodo che sta rispondeno nella coda degli allocati...
        if (state->slave[queueId].node_id == nodeId) {
            return queueId;
        }
    }
    return GENERIC_BVAL_UNDEFINED;
}

// Ritorna l'indice della coda master allocata in state in funzione del nodeId fisico
byte getPNPValidIdFromQueueNode(State* const state, uint8_t node_type) {
    // Cerco la corrispondenza node_id nella coda allocata master per ritorno queueID Index
    for (byte queueId = 0; queueId < MAX_NODE_CONNECT; queueId++) {
        // Se trovo il nodo che sta pubblicando come node_type
        // nella coda dei nodi configurati ma non ancora allocati...
        if ((state->slave[queueId].node_type == node_type) &&
            (state->slave[queueId].pnp.is_assigned == false)) {
            // Ritorno il NodeID configurato da remoto come default da associare
            return state->slave[queueId].node_id;
        }
    }
    return GENERIC_BVAL_UNDEFINED;
}

// Ritorna i uS dalle funzioni Micros di Arduino (in formato 64 BIT necessario per UAVCAN)
// Non permette il reset n ei 70 minuti circa previsti per l'overflow della funzione uS a 32 Bit
static CanardMicrosecond getMonotonicMicroseconds(State* const state) {
    uint32_t ts = micros();
    if (ts > state->lastMicrosecond) {
        state->currentMicrosecond += (ts - state->lastMicrosecond);
    } else {
        state->currentMicrosecond += ts;
        state->currentMicrosecond += (0xFFFFFFFFu - state->lastMicrosecond);
    }
    state->lastMicrosecond = ts;
    return (uint64_t)state->currentMicrosecond;
}

// Ritorna unique-ID 128-bit del nodo locale. E' utilizzato in uavcan.node.GetInfo.Response e durante
// plug-and-play node-ID allocation da uavcan.pnp.NodeIDAllocationData. SerialNumber, Produttore..
// Dovrebbe essere verificato in uavcan.node.GetInfo.Response per la verifica non sia cambiato Nodo.
// Al momento vengono inseriti 2 BYTE fissi, altri eventuali, che Identificano il Tipo Modulo
static void getUniqueID(uint8_t out[uavcan_node_GetInfo_Response_1_0_unique_id_ARRAY_CAPACITY_]) {
    // A real hardware node would read its unique-ID from some hardware-specific source (typically stored in ROM).
    // This example is a software-only node so we store the unique-ID in a (read-only) register instead.
    uavcan_register_Value_1_0 value = {0};
    uavcan_register_Value_1_0_select_unstructured_(&value);
    // Crea default unique_id con NODE_TYPE_MAJOR (Tipo di nodo), MINOR (Hw relativo)
    // Il resto dei 128 Bit (112) vengono impostati RANDOM (potrebbero portare Manufactor, SerialNumber ecc...)
    // Dovrebbe essere l'ID per la verifica incrociata del corretto Node_Id dopo il PnP
    value.unstructured.value.elements[value.unstructured.value.count++] = (uint8_t)NODE_TYPE_MAJOR;
    value.unstructured.value.elements[value.unstructured.value.count++] = (uint8_t)NODE_TYPE_MINOR;
    for (uint8_t i = value.unstructured.value.count; i < uavcan_node_GetInfo_Response_1_0_unique_id_ARRAY_CAPACITY_; i++) {
        value.unstructured.value.elements[value.unstructured.value.count++] = (uint8_t)rand();  // NOLINT
    }
    registerRead("uavcan.node.unique_id", &value);
    memcpy(&out[0], &value.unstructured.value, uavcan_node_GetInfo_Response_1_0_unique_id_ARRAY_CAPACITY_);
}

// ***********************************************************************************************
// ***********************************************************************************************
//      FUNZIONI CHIAMATE DA MAIN_LOOP DI PUBBLICAZIONE E RICHIESTE DATI E SERVIZI
// ***********************************************************************************************
// ***********************************************************************************************

// *******        Funzioni ed utility di ritrasmissione dati sulla rete UAVCAN           *********

// Wrapper per send e sendresponse con Canard
static void send(State* const state,
                 const CanardMicrosecond tx_deadline_usec,
                 const CanardTransferMetadata* const metadata,
                 const size_t payload_size,
                 const void* const payload) {
    for (uint8_t ifidx = 0; ifidx < CAN_REDUNDANCY_FACTOR; ifidx++) {
        (void)canardTxPush(&state->canard_tx_queues[ifidx],
                           &state->canard,
                           tx_deadline_usec,
                           metadata,
                           payload_size,
                           payload);
    }
}
// Risposte con inversione meta.transfer_kind alle Request
static void sendResponse(State* const state,
                         const CanardMicrosecond tx_deadline_usec,
                         const CanardTransferMetadata* const request_metadata,
                         const size_t payload_size,
                         const void* const payload) {
    CanardTransferMetadata meta = *request_metadata;
    meta.transfer_kind = CanardTransferKindResponse;
    send(state, tx_deadline_usec, &meta, payload_size, payload);
}

// *******              FUNZIONI INVOCATE HANDLE CONT_LOOP EV. PREPARATORIE              *********

// FileRead V1.1
static void handleFileReadBlock_1_1(State* const state, const CanardMicrosecond monotonic_time) {
    // ***** Ricezione di file generico dalla rete UAVCAN dal nodo chiamante *****
    // Richiamo in continuazione rapida la funzione fino al riempimento del file
    // Alla fine processo il firmware Upload (eventuale) vero e proprio con i relativi check
    uavcan_file_Read_Request_1_1 remotefile = {0};
    remotefile.path.path.count = strlen(state->master.file.filename);
    memcpy(remotefile.path.path.elements, state->master.file.filename, remotefile.path.path.count);
    remotefile.offset = state->master.file.offset;

    uint8_t serialized[uavcan_file_Read_Request_1_1_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
    size_t serialized_size = sizeof(serialized);
    const int8_t err = uavcan_file_Read_Request_1_1_serialize_(&remotefile, &serialized[0], &serialized_size);
    if (err >= 0) {
        const CanardTransferMetadata meta = {
            .priority = CanardPriorityHigh,
            .transfer_kind = CanardTransferKindRequest,
            .port_id = uavcan_file_Read_1_1_FIXED_PORT_ID_,
            .remote_node_id = state->master.file.server_node_id,
            .transfer_id = (CanardTransferID)(state->next_transfer_id.uavcan_file_read_data++),
        };
        send(state,
             monotonic_time + MEGA,
             &meta,
             serialized_size,
             &serialized[0]);
    }
}

// *******              FUNZIONI INVOCATE HANDLE 1 SECONDO EV. PREPARATORIE              *********

static void handleSyncroLoop(State* const state, const CanardMicrosecond monotonic_time) {
    // ***** Trasmette alla rete UAVCAN lo stato syncronization_time del modulo *****
    // Da specifica invio il timestamp dell'ultima chiamata in modo che slave sincronizzi il delta
    uavcan_time_Synchronization_1_0 timesyncro;
    timesyncro.previous_transmission_timestamp_microsecond = state->master.timestamp.previous_tx_real;
    state->master.timestamp.enable_immediate_tx_real = true;

    uint8_t serialized[uavcan_time_Synchronization_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
    size_t serialized_size = sizeof(serialized);
    const int8_t err = uavcan_time_Synchronization_1_0_serialize_(&timesyncro, &serialized[0], &serialized_size);
    if (err >= 0) {
        // Traferimento immediato per sincronizzazione migliore con i nodi remoti
        // Aggiorno il time stamp su blocco trasmesso a priorità immediata
        const CanardTransferMetadata meta = {
            .priority = CanardPriorityImmediate,
            .transfer_kind = CanardTransferKindMessage,
            .port_id = uavcan_time_Synchronization_1_0_FIXED_PORT_ID_,
            .remote_node_id = CANARD_NODE_ID_UNSET,
            .transfer_id = (CanardTransferID)(state->next_transfer_id.uavcan_time_synchronization++),
        };
        send(state,
             monotonic_time + MEGA,
             &meta,
             serialized_size,
             &serialized[0]);
    }
}

static void handleNormalLoop(State* const state, const CanardMicrosecond monotonic_time) {
    // ***** Trasmette alla rete UAVCAN lo stato haeartbeat del modulo *****
    // Heartbeat Fisso anche per modulo Master (Visibile a yakut o altri tools/script gestionali)
    uavcan_node_Heartbeat_1_0 heartbeat = {0};
    heartbeat.uptime = (uint32_t)((monotonic_time - state->started_at) / MEGA);
    heartbeat.mode.value = uavcan_node_Mode_1_0_OPERATIONAL;
    const O1HeapDiagnostics heap_diag = o1heapGetDiagnostics(state->heap);
    if (heap_diag.oom_count > 0) {
        heartbeat.health.value = uavcan_node_Health_1_0_CAUTION;
    } else {
        heartbeat.health.value = uavcan_node_Health_1_0_NOMINAL;
    }
    heartbeat.vendor_specific_status_code = VSC_SOFTWARE_NORMAL;
    // Comunicazione dei FLAG di Update ed altri VSC privati opzionali
    if (state->master.file.updating) {
        // heartbeat.mode.value = uavcan_node_Mode_1_0_SOFTWARE_UPDATE;
        heartbeat.vendor_specific_status_code = VSC_SOFTWARE_UPDATE_READ;
    }
    // A fine trasferimento completo
    if (state->master.file.updating_run) {
        // Utilizzare questo flag solo in avvio di update (YAKUT Blocca i trasferimenti)
        // Altrimenti ricomincia il trasferimento da capo da inizio file all'infinito...
        heartbeat.mode.value = uavcan_node_Mode_1_0_SOFTWARE_UPDATE;
    }

    uint8_t serialized[uavcan_node_Heartbeat_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
    size_t serialized_size = sizeof(serialized);
    const int8_t err = uavcan_node_Heartbeat_1_0_serialize_(&heartbeat, &serialized[0], &serialized_size);
    if (err >= 0) {
        const CanardTransferMetadata meta = {
            .priority = CanardPriorityNominal,
            .transfer_kind = CanardTransferKindMessage,
            .port_id = uavcan_node_Heartbeat_1_0_FIXED_PORT_ID_,
            .remote_node_id = CANARD_NODE_ID_UNSET,
            .transfer_id = (CanardTransferID)(state->next_transfer_id.uavcan_node_heartbeat++),
        };
        send(state,
             monotonic_time + MEGA,
             &meta,
             serialized_size,
             &serialized[0]);
    }
}

// *******              FUNZIONI INVOCATE HANDLE 10 SECONDI EV. PREPARATORIE             *********

// Prepara lista sottoscrizioni (solo quelle allocate correttamente <= CANARD_SUBJECT_ID_MAX) uavcan_node_port_List_0_1.
static void fillSubscriptions(const CanardTreeNode* const tree, uavcan_node_port_SubjectIDList_0_1* const obj) {
    if (NULL != tree) {
        fillSubscriptions(tree->lr[0], obj);
        const CanardRxSubscription* crs = (const CanardRxSubscription*)tree;
        if (crs->port_id <= CANARD_SUBJECT_ID_MAX) {
            obj->sparse_list.elements[obj->sparse_list.count++].value = crs->port_id;
            fillSubscriptions(tree->lr[1], obj);
        }
    }
}

/// This is needed only for constructing uavcan_node_port_List_0_1.
static void fillServers(const CanardTreeNode* const tree, uavcan_node_port_ServiceIDList_0_1* const obj) {
    if (NULL != tree) {
        fillServers(tree->lr[0], obj);
        const CanardRxSubscription* crs = (const CanardRxSubscription*)tree;
        if (crs->port_id <= CANARD_SERVICE_ID_MAX) {
            (void)nunavutSetBit(&obj->mask_bitpacked_[0], sizeof(obj->mask_bitpacked_), crs->port_id, true);
            fillServers(tree->lr[1], obj);
        }
    }
}

// **************           Pubblicazione vera e propria a 20 secondi           **************
static void handleSlowLoop(State* const state, const CanardMicrosecond monotonic_time) {
    // Publish the recommended (not required) port introspection message. No point publishing it if we're anonymous.
    // The message is a bit heavy on the stack (about 2 KiB) but this is not a problem for a modern MCU.
    // L'abilitazione del comando è facoltativa, può essere attivata/disattivata da un comando UAVCAN
    if ((state->publisher_enabled.port_list) &&
        (state->canard.node_id <= CANARD_NODE_ID_MAX)) {
        uavcan_node_port_List_0_1 m = {0};
        uavcan_node_port_List_0_1_initialize_(&m);
        uavcan_node_port_SubjectIDList_0_1_select_sparse_list_(&m.publishers);
        uavcan_node_port_SubjectIDList_0_1_select_sparse_list_(&m.subscribers);

        // Indicate which subjects we publish to. Don't forget to keep this updated if you add new publications!
        {
            size_t* const cnt = &m.publishers.sparse_list.count;
            m.publishers.sparse_list.elements[(*cnt)++].value = uavcan_node_Heartbeat_1_0_FIXED_PORT_ID_;
            m.publishers.sparse_list.elements[(*cnt)++].value = uavcan_node_port_List_0_1_FIXED_PORT_ID_;
            // Aggiungo i publisher interni validi privati
        }

        // Indicate which servers and subscribers we implement.
        // We could construct the list manually but it's easier and more robust to just query libcanard for that.
        fillSubscriptions(state->canard.rx_subscriptions[CanardTransferKindMessage], &m.subscribers);
        fillServers(state->canard.rx_subscriptions[CanardTransferKindRequest], &m.servers);
        fillServers(state->canard.rx_subscriptions[CanardTransferKindResponse], &m.clients);  // For regularity.

        // Serialize and publish the message. Use a small buffer because we know that our message is always small.
        // Verificato massimo utilizzo a 156 bytes. Limitiamo il buffer a 256 Bytes (Come esempio UAVCAN)
        uint8_t serialized[256] = {0};
        size_t serialized_size = uavcan_node_port_List_0_1_SERIALIZATION_BUFFER_SIZE_BYTES_;
        if (uavcan_node_port_List_0_1_serialize_(&m, &serialized[0], &serialized_size) >= 0) {
            const CanardTransferMetadata meta = {
                .priority = CanardPriorityOptional,  // Mind the priority.
                .transfer_kind = CanardTransferKindMessage,
                .port_id = uavcan_node_port_List_0_1_FIXED_PORT_ID_,
                .remote_node_id = CANARD_NODE_ID_UNSET,
                .transfer_id = (CanardTransferID)(state->next_transfer_id.uavcan_node_port_list++),
            };
            // Send a 2 secondi
            send(state, monotonic_time + MEGA * 2, &meta, serialized_size, &serialized[0]);
        }
    }
}

// ************** SEZIONE COMANDI E RICHIESTE SPECIFICHE AD UN NODO SULLA RETE  **************

// **************       Invio Comando diretto ad un nodo remoto UAVCAN Cmd      **************
static bool serviceSendCommand(State* const state, const CanardMicrosecond monotonic_time,
                               byte istanza, const uint16_t cmd_request, const void* ext_param, size_t ext_lenght) {
    // Effettua una richiesta specifica ad un nodo della rete in formato UAVCAN
    uavcan_node_ExecuteCommand_Request_1_1 cmdRequest = {0};
    uint8_t serialized[uavcan_node_ExecuteCommand_Request_1_1_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
    size_t serialized_size = sizeof(serialized);

    // istanza -> queueId di State o istanza di nodo

    // Imposta il comando da inviare
    cmdRequest.command = cmd_request;
    // Verifica la presenza di parametri opzionali nel comando
    cmdRequest.parameter.count = ext_lenght;
    // Controllo conformità lunghezza messaggio e ne copio il contenuto
    if (ext_lenght) {
        memcpy(cmdRequest.parameter.elements, ext_param, ext_lenght);
    }

    // Serializzo e verifico la conformità del messaggio
    const int8_t err = uavcan_node_ExecuteCommand_Request_1_1_serialize_(&cmdRequest, &serialized[0], &serialized_size);
    if (err >= 0) {
        // Comando a priorità alta
        const CanardTransferMetadata meta = {
            .priority = CanardPriorityHigh,
            .transfer_kind = CanardTransferKindRequest,
            .port_id = uavcan_node_ExecuteCommand_1_1_FIXED_PORT_ID_,
            .remote_node_id = state->slave[istanza].node_id,
            .transfer_id = (CanardTransferID)(state->slave[istanza].command.next_transfer_id++),
        };
        send(state,
             monotonic_time + MEGA,
             &meta,
             serialized_size,
             &serialized[0]);
        return true;
    }
    return false;
}

// **************   Invio richiesta dati diretto ad un nodo remoto UAVCAN Get   **************
static bool serviceSendRegister(State* const state, const CanardMicrosecond monotonic_time,
                                byte istanza, char* registerName, uavcan_register_Value_1_0 registerValue) {
    // Effettua la richiesta UAVCAN per l'accesso ad un registro remoto di un nodo slave
    // Utile per la configurazione remota completa o la modifica di un parametro dello slave
    uavcan_register_Access_Request_1_0 cmdRequest = {0};
    uint8_t serialized[uavcan_register_Access_Request_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
    size_t serialized_size = sizeof(serialized);

    // Imposta il comando da inviare ed il timer base
    cmdRequest.value = registerValue;
    cmdRequest.name.name.count = strlen(registerName);
    memcpy(&cmdRequest.name.name.elements[0], registerName, cmdRequest.name.name.count);

    // Serializzo e verifico la conformità del messaggio
    const int8_t err = uavcan_register_Access_Request_1_0_serialize_(&cmdRequest, &serialized[0], &serialized_size);
    if (err >= 0) {
        // Comando a priorità alta
        const CanardTransferMetadata meta = {
            .priority = CanardPriorityHigh,
            .transfer_kind = CanardTransferKindRequest,
            .port_id = uavcan_register_Access_1_0_FIXED_PORT_ID_,
            .remote_node_id = state->slave[istanza].node_id,
            .transfer_id = (CanardTransferID)(state->slave[istanza].register_access.next_transfer_id++),
        };
        send(state,
             monotonic_time + MEGA,
             &meta,
             serialized_size,
             &serialized[0]);
        return true;
    }
    return false;
}

// **************   Invio richiesta dati diretto ad un nodo remoto UAVCAN Get   **************
static bool serviceSendRequestData(State* const state, const CanardMicrosecond monotonic_time,
                                   byte istanza, byte comando, uint16_t run_sectime) {
    // Effettua una richiesta specifica ad un nodo della rete in formato UAVCAN
    // La richiesta è generica per tutti i moduli (univoca DSDL), comunque parte integrante di ogni
    // DSDL singola di modulo. Il PORT_ID fisso o dinamico indica il nodo remoto.
    // L'interpretazione è invece tipicizzata dalla risposta (DSDL specifica)
    rmap_service_setmode_1_0 cmdRequest = {0};
    uint8_t serialized[rmap_service_module_TH_Request_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
    size_t serialized_size = sizeof(serialized);

    // Imposta il comando da inviare ed il timer base
    cmdRequest.comando = comando;
    cmdRequest.run_sectime = run_sectime;

    // Serializzo e verifico la conformità del messaggio
    const int8_t err = rmap_service_setmode_1_0_serialize_(&cmdRequest, &serialized[0], &serialized_size);
    if (err >= 0) {
        // Comando a priorità alta
        const CanardTransferMetadata meta = {
            .priority = CanardPriorityHigh,
            .transfer_kind = CanardTransferKindRequest,
            .port_id = state->slave[istanza].rmap_service.port_id,
            .remote_node_id = state->slave[istanza].node_id,
            .transfer_id = (CanardTransferID)(state->slave[istanza].rmap_service.next_transfer_id++),
        };
        send(state,
             monotonic_time + MEGA,
             &meta,
             serialized_size,
             &serialized[0]);
        return true;
    }
    return false;
}

// ***************************************************************************************************
//   Funzioni ed utility di ricezione dati dalla rete UAVCAN, richiamati da processReceivedTransfer()
// ***************************************************************************************************

// Chiamate gestioni RPC remote da master (yakut o altro servizio di controllo)
static uavcan_node_ExecuteCommand_Response_1_1 processRequestExecuteCommand(State* state, const uavcan_node_ExecuteCommand_Request_1_1* req,
                                                                            uint8_t remote_node) {
    uavcan_node_ExecuteCommand_Response_1_1 resp = {0};
    // req->command (Comando esterno ricevuto 2 BYTES RESERVED FFFF-FFFA)
    // Gli altri sono liberi per utilizzo interno applicativo con #define interne
    // req->parameter (array di byte MAX 255 per i parametri da request)
    // Risposta attuale (resp) 1 Bytes RESERVER (0..6) gli altri #define interne
    switch (req->command) {
        // **************** Comandi standard UAVCAN GENERIC_SPECIFIC_COMMAND ****************
        // Comando di aggiornamento Firmware compatibile con Yakut e specifice UAVCAN
        case uavcan_node_ExecuteCommand_Request_1_1_COMMAND_BEGIN_SOFTWARE_UPDATE: {
            // Nodo Server chiamante (Yakut solo Master, Yakut e Master per Slave)
            state->master.file.server_node_id = remote_node;
            // Copio la stringa nel name file firmware disponibile su state generale (per download successivo)
            memcpy(state->master.file.filename, req->parameter.elements, req->parameter.count);
            state->master.file.filename[req->parameter.count] = '\0';
            // Init varaiabili di download
            state->master.file.is_firmware = true;
            state->master.file.updating = true;
            state->master.file.updating_eof = false;
            state->master.file.offset = 0;
            // Controlla retry continue (da mettere OFF a RX Response di FileRead)
            state->master.file.updating_retry = 0;
            // Avvio la funzione con OK
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        case uavcan_node_ExecuteCommand_Request_1_1_COMMAND_FACTORY_RESET: {
            registerDoFactoryReset();
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        case uavcan_node_ExecuteCommand_Request_1_1_COMMAND_RESTART: {
            state->flag.g_restart_required = true;
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        case uavcan_node_ExecuteCommand_Request_1_1_COMMAND_STORE_PERSISTENT_STATES: {
            // If your registers are not automatically synchronized with the non-volatile storage, use this command
            // to commit them to the storage explicitly. Otherwise it is safe to remove it.
            // In this demo, the registers are stored in files, so there is nothing to do.
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        // **************** Comandi personalizzati VENDOR_SPECIFIC_COMMAND ****************
        // Comando di download File generico compatibile con specifice UAVCAN, (LOG/CFG altro...)
        case CMD_DOWNLOAD_FILE: {
            // Nodo Server chiamante (Yakut solo Master, Yakut e Master per Slave)
            state->master.file.server_node_id = remote_node;
            // Copio la stringa nel name file generico disponibile su state generale (per download successivo)
            memcpy(state->master.file.filename, req->parameter.elements, req->parameter.count);
            state->master.file.filename[req->parameter.count] = '\0';
            // Init varaiabili di download
            state->master.file.is_firmware = false;
            state->master.file.updating = true;
            state->master.file.updating_eof = false;
            state->master.file.offset = 0;
            // Controlla retry continue (da mettere OFF a RX Response di FileRead)
            state->master.file.updating_retry = 0;
            // Avvio la funzione con OK
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        case CMD_ENABLE_PUBLISH_PORT_LIST: {
            // Abilita pubblicazione slow_loop elenco porte (Cypal facoltativo)
            state->publisher_enabled.port_list = true;
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        case CMD_DISABLE_PUBLISH_PORT_LIST: {
            // Disabilita pubblicazione slow_loop elenco porte (Cypal facoltativo)
            state->publisher_enabled.port_list = false;
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        default: {
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_BAD_COMMAND;
            break;
        }
    }
    return resp;
}

// Accesso ai registri UAVCAN risposta a richieste
static uavcan_register_Access_Response_1_0 processRequestRegisterAccess(const uavcan_register_Access_Request_1_0* req) {
    char name[uavcan_register_Name_1_0_name_ARRAY_CAPACITY_ + 1] = {0};
    memcpy(&name[0], req->name.name.elements, req->name.name.count);
    name[req->name.name.count] = '\0';

    uavcan_register_Access_Response_1_0 resp = {0};

    // If we're asked to write a new value, do it now:
    if (!uavcan_register_Value_1_0_is_empty_(&req->value)) {
        uavcan_register_Value_1_0_select_empty_(&resp.value);
        registerRead(&name[0], &resp.value);
        // If such register exists and it can be assigned from the request value:
        if (!uavcan_register_Value_1_0_is_empty_(&resp.value) && registerAssign(&resp.value, &req->value)) {
            registerWrite(&name[0], &resp.value);
        }
    }

    // Regardless of whether we've just wrote a value or not, we need to read the current one and return it.
    // The client will determine if the write was successful or not by comparing the request value with response.
    uavcan_register_Value_1_0_select_empty_(&resp.value);
    registerRead(&name[0], &resp.value);

    // Currently, all registers we implement are mutable and persistent. This is an acceptable simplification,
    // but more advanced implementations will need to differentiate between them to support advanced features like
    // exposing internal states via registers, perfcounters, etc.
    resp._mutable = true;
    resp.persistent = true;

    // Our node does not synchronize its time with the network so we can't populate the timestamp.
    resp.timestamp.microsecond = uavcan_time_SynchronizedTimestamp_1_0_UNKNOWN;

    return resp;
}

// Risposta a uavcan.node.GetInfo which Info Node (nome, versione, uniqueID di verifica ecc...)
static uavcan_node_GetInfo_Response_1_0 processRequestNodeGetInfo() {
    uavcan_node_GetInfo_Response_1_0 resp = {0};
    resp.protocol_version.major = CANARD_CYPHAL_SPECIFICATION_VERSION_MAJOR;
    resp.protocol_version.minor = CANARD_CYPHAL_SPECIFICATION_VERSION_MINOR;

    // The hardware version is not populated in this demo because it runs on no specific hardware.
    // An embedded node would usually determine the version by querying the hardware.

    resp.software_version.major = VERSION_MAJOR;
    resp.software_version.minor = VERSION_MINOR;
    resp.software_vcs_revision_id = VCS_REVISION_ID;

    getUniqueID(resp.unique_id);

    // The node name is the name of the product like a reversed Internet domain name (or like a Java package).
    resp.name.count = strlen(NODE_NAME);
    memcpy(&resp.name.elements, NODE_NAME, resp.name.count);

    // The software image CRC and the Certificate of Authenticity are optional so not populated in this demo.
    return resp;
}

// ******************************************************************************************
// Processo multiplo di ricezione messaggi e comandi. Gestione entrata ed uscita dei messaggi
// Chiamata direttamente nel main loop in ricezione dalla coda RX
// Richiama le funzioni qui sopra di preparazione e risposta alle richieste
// ******************************************************************************************
static void processReceivedTransfer(State* const state, const CanardRxTransfer* const transfer) {
    // Gestione dei Messaggi in ingresso
    if (transfer->metadata.transfer_kind == CanardTransferKindMessage) {
        // bool Per assert mancanza handler di eventuale servizio sottoscritto
        bool bKindMessageProcessed = false;
        // Gestione dei messaggi PNP per allocazione nodi di rete (gestisco come master)
        if (transfer->metadata.port_id == uavcan_pnp_NodeIDAllocationData_1_0_FIXED_PORT_ID_) {
            // Richiesta di allocazione Nodo dalla rete con messaggio anonimo V1.0 CAN_MTU 8
            bKindMessageProcessed = true;
            uint8_t defaultNodeId = 0;
            size_t size = transfer->payload_size;
            uavcan_pnp_NodeIDAllocationData_1_0 msg = {0};
            if (uavcan_pnp_NodeIDAllocationData_1_0_deserialize_(&msg, static_cast<uint8_t const*>(transfer->payload), &size) >= 0) {
                // Cerco nei moduli conosciuti (in HASH_UNIQUE_ID) invio il tipo modulo...
                // Verifico se ho un nodo ancora da configurare come da cfg del master
                // Il nodo deve essere compatibile con il tipo di modulo previsto da allocare
                switch (msg.unique_id_hash & 0xFF) {
                    case MODULE_TYPE_TH:
                        defaultNodeId = getPNPValidIdFromQueueNode(state, MODULE_TYPE_TH);
                        break;
                    case MODULE_TYPE_RAIN:
                        defaultNodeId = getPNPValidIdFromQueueNode(state, MODULE_TYPE_RAIN);
                        break;
                    case MODULE_TYPE_WIND:
                        defaultNodeId = getPNPValidIdFromQueueNode(state, MODULE_TYPE_WIND);
                        break;
                    case MODULE_TYPE_RADIATION:
                        defaultNodeId = getPNPValidIdFromQueueNode(state, MODULE_TYPE_RADIATION);
                        break;
                    case MODULE_TYPE_VWC:
                        defaultNodeId = getPNPValidIdFromQueueNode(state, MODULE_TYPE_VWC);
                        break;
                    case MODULE_TYPE_POWER:
                        defaultNodeId = getPNPValidIdFromQueueNode(state, MODULE_TYPE_POWER);
                        break;
                    defualt:
                        // PNP Non gestibile
                        defaultNodeId = GENERIC_BVAL_UNDEFINED;
                        break;
                }

                // Risposta immediata diretta (Se nodo ovviamente è riconosciuto...)
                // Non utilizziamo una Response in quanto l'allocation è sempre un messaggio anonimo
                // I metadati del trasporto sono come quelli riceuti del transferID quindi è un messaggio
                // che si comporta parzialmente come una risposta (per rilevamento remoto hash/transfer_id)
                if (defaultNodeId <= CANARD_NODE_ID_MAX) {
                    // Se il nodo proposto viene confermato inizieremo a ricevere heartbeat
                    // da quel nodeId. A questo punto in Heartbeat settiam il flag pnp.is_assigned
                    // che conclude la procedura con esito positivo.
                    msg.allocated_node_id.count = 1;
                    msg.allocated_node_id.elements[0].value = defaultNodeId;
                    // The request object is empty so we don't bother deserializing it. Just send the response.
                    uint8_t serialized[uavcan_pnp_NodeIDAllocationData_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
                    size_t serialized_size = sizeof(serialized);
                    const int8_t res = uavcan_pnp_NodeIDAllocationData_1_0_serialize_(&msg, &serialized[0], &serialized_size);
                    // Preparo la pubblicazione anonima in risposta alla richiesta anonima
                    const CanardTransferMetadata meta = {
                        .priority = CanardPriorityNominal,
                        .transfer_kind = CanardTransferKindMessage,
                        .port_id = uavcan_pnp_NodeIDAllocationData_1_0_FIXED_PORT_ID_,
                        .remote_node_id = CANARD_NODE_ID_UNSET,
                        .transfer_id = (CanardTransferID)(transfer->metadata.transfer_id),
                    };
                    send(state,
                         getMonotonicMicroseconds(state) + MEGA,
                         &meta,
                         serialized_size,
                         &serialized[0]);
                }
            }
        }
        // Gestione dei messaggi Heartbeat per stato rete (gestisco come master)
        else if (transfer->metadata.port_id == uavcan_node_Heartbeat_1_0_FIXED_PORT_ID_) {
            bKindMessageProcessed = true;
            size_t size = transfer->payload_size;
            uavcan_node_Heartbeat_1_0 msg = {0};
            if (uavcan_node_Heartbeat_1_0_deserialize_(&msg, static_cast<uint8_t const*>(transfer->payload), &size) >= 0) {
                // Processo e registro il nodo: stato, OnLine e relativi flag
                byte queueId = getQueueNodeFromId(state, transfer->metadata.remote_node_id);
                // Se nodo correttamente allocato e gestito (potrebbe essere Yakut non registrato)
                if (queueId != GENERIC_BVAL_UNDEFINED) {
                    // Primo assegnamento da PNP, gestisco eventuale configurazione remota
                    // e salvataggio del flag di assegnamento in ROM / o Register
                    if (!state->slave[queueId].pnp.is_assigned) {
                        // Configura i metadati...
                        // Configura altri parametri...
                        // Modifico il flag PNP Executed e termino la procedura PNP
                        state->slave[queueId].pnp.is_assigned = true;
                        // Salvo su registro lo stato
                        uavcan_register_Value_1_0 val = {0};
                        char registerName[24] = "rmap.pnp.allocateID.";
                        uavcan_register_Value_1_0_select_natural8_(&val);
                        val.natural32.value.count = 1;
                        val.natural32.value.elements[0] = transfer->metadata.remote_node_id;
                        // queueId -> index Val = NodeId
                        itoa(queueId, registerName + strlen(registerName), 10);
                        registerWrite(registerName, &val);
                    }

                    // Rientro in OnLINE da OFFLine o Init
                    // Inizializzo le variabili e gli stati necessari per Reset e corretta gestione
                    if (!state->slave[queueId].is_online) {
                        // Accodo i dati letti dal messaggio (Nodo -> OnLine)
                        state->slave[queueId].is_online = true;
                        // Metto i Flag in sicurezza, laddove dove non eventualmente gestito
                        state->slave[queueId].command.is_pending = false;
                        state->slave[queueId].command.is_timeout = true;
                        state->slave[queueId].register_access.is_pending = false;
                        state->slave[queueId].register_access.is_timeout = true;
                        state->slave[queueId].file.is_pending = false;
                        state->slave[queueId].file.is_timeout = true;
                        state->slave[queueId].file.state = FILE_STATE_STANDBY;
                        state->slave[queueId].rmap_service.is_pending = false;
                        state->slave[queueId].rmap_service.is_timeout = true;

                        // ******************************************************************************
                        // ******************************** START TEST **********************************
                        // ******************************************************************************
#ifdef TEST_UAVCAN_NODE_OFFLINE
                        RUN_TEST(test_slave_is_online);
#endif
                        test_state = ONLINE;
                        // ******************************************************************************
                        // ******************************** FINE TEST ***********************************
                        // ******************************************************************************
                    }
                    state->slave[queueId].heartbeat.healt = msg.health.value;
                    state->slave[queueId].heartbeat.state = msg.vendor_specific_status_code;
                    // Set canard_us local per controllo NodoOffline
                    state->slave[queueId].heartbeat.timeout_us = transfer->timestamp_usec + NODE_OFFLINE_TIMEOUT_US;
                }
            }
        }
#ifdef USE_SUB_PUBLISH_SLAVE_DATA
        else {
            // Gestione messaggi pubblicazione dati dei moduli slave (gestisco come master)
            // Es. popalamento dati se attivato un log specifico o show valori su display
            // Il comando è opzionale perchè in request/response esiste già questa possibilità
            // Nodo rispondente leggo dalla coda la/le pubblicazioni attivate (MAX 1 x tipologia)
            byte queueId = getQueueNodeFromId(state, transfer->metadata.remote_node_id);
            // Se nodo correttammente allocato e gestito
            if (queueId != GENERIC_BVAL_UNDEFINED) {
                // Verifico se risposta del servizio corrisponde al chiamante (eventuali + servizi sotto...)
                // Gestione di tutti i servizi possibili allocabili, valido per tutti i nodi
                if (transfer->metadata.port_id == state->slave[queueId].publisher.subject_id) {
                    // *************            Service Modulo TH Response            *************
                    if (state->slave[queueId].node_type == MODULE_TYPE_TH) {
                        // Processato il messaggio con il relativo Handler. OK
                        bKindMessageProcessed = true;
                        // Modulo TH, leggo e deserializzo il messaggio in ingresso
                        rmap_module_TH_1_0 msg = {0};
                        size_t size = transfer->payload_size;
                        if (rmap_module_TH_1_0_deserialize_(&msg, static_cast<uint8_t const*>(transfer->payload), &size) >= 0) {
                            // TODO: vedere con Marco Pubblica, registra elimina, display... altro
                            // Per ora salvo solo il dato ricevuto dalla struttura di state msg (count)
                            // msg contiene i dati di blocco pubblicati
                            state->slave[queueId].publisher.data_count++;
                        }
                    }
                    // ALTRI MODULI DA INSERIRE QUA... PG, VV, RS, GAS ECC...
                }
            }
        }
#endif
    }
    // Gestione delle richieste esterne
    else if (transfer->metadata.transfer_kind == CanardTransferKindRequest) {
        if (transfer->metadata.port_id == uavcan_node_GetInfo_1_0_FIXED_PORT_ID_) {
            // The request object is empty so we don't bother deserializing it. Just send the response.
            const uavcan_node_GetInfo_Response_1_0 resp = processRequestNodeGetInfo();
            uint8_t serialized[uavcan_node_GetInfo_Response_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
            size_t serialized_size = sizeof(serialized);
            const int8_t res = uavcan_node_GetInfo_Response_1_0_serialize_(&resp, &serialized[0], &serialized_size);
            if (res >= 0) {
                sendResponse(state,
                             transfer->timestamp_usec + MEGA,
                             &transfer->metadata,
                             serialized_size,
                             &serialized[0]);
            }
        } else if (transfer->metadata.port_id == uavcan_register_Access_1_0_FIXED_PORT_ID_) {
            uavcan_register_Access_Request_1_0 req = {0};
            size_t size = transfer->payload_size;
            if (uavcan_register_Access_Request_1_0_deserialize_(&req, static_cast<uint8_t const*>(transfer->payload), &size) >= 0) {
                const uavcan_register_Access_Response_1_0 resp = processRequestRegisterAccess(&req);
                uint8_t serialized[uavcan_register_Access_Response_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
                size_t serialized_size = sizeof(serialized);
                if (uavcan_register_Access_Response_1_0_serialize_(&resp, &serialized[0], &serialized_size) >= 0) {
                    sendResponse(state,
                                 transfer->timestamp_usec + MEGA,
                                 &transfer->metadata,
                                 serialized_size,
                                 &serialized[0]);
                }
            }
        } else if (transfer->metadata.port_id == uavcan_register_List_1_0_FIXED_PORT_ID_) {
            uavcan_register_List_Request_1_0 req = {0};
            size_t size = transfer->payload_size;
            if (uavcan_register_List_Request_1_0_deserialize_(&req, static_cast<uint8_t const*>(transfer->payload), &size) >= 0) {
                uavcan_register_List_Response_1_0 resp;
                resp.name = registerGetNameByIndex(req.index);
                uint8_t serialized[uavcan_register_List_Response_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
                size_t serialized_size = sizeof(serialized);
                if (uavcan_register_List_Response_1_0_serialize_(&resp, &serialized[0], &serialized_size) >= 0) {
                    sendResponse(state,
                                 transfer->timestamp_usec + MEGA,
                                 &transfer->metadata,
                                 serialized_size,
                                 &serialized[0]);
                }
            }
        } else if (transfer->metadata.port_id == uavcan_node_ExecuteCommand_1_1_FIXED_PORT_ID_) {
            uavcan_node_ExecuteCommand_Request_1_1 req = {0};
            size_t size = transfer->payload_size;
            if (uavcan_node_ExecuteCommand_Request_1_1_deserialize_(&req, static_cast<uint8_t const*>(transfer->payload), &size) >= 0) {
                const uavcan_node_ExecuteCommand_Response_1_1 resp = processRequestExecuteCommand(state, &req, transfer->metadata.remote_node_id);
                uint8_t serialized[uavcan_node_ExecuteCommand_Response_1_1_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
                size_t serialized_size = sizeof(serialized);
                if (uavcan_node_ExecuteCommand_Response_1_1_serialize_(&resp, &serialized[0], &serialized_size) >= 0) {
                    sendResponse(state,
                                 transfer->timestamp_usec + MEGA,
                                 &transfer->metadata,
                                 serialized_size,
                                 &serialized[0]);
                }
            }
        } else if (transfer->metadata.port_id == uavcan_file_Read_1_1_FIXED_PORT_ID_) {
            // La funzione viene eseguita solo con UPLOAD Sequenza corretta
            // Serve a fare eseguire un'eventuale TimeOut su procedura non corretta
            // Ed evitare blocchi non coerenti... Viene gestita senza problemi
            // Send multiplo di File e procedure in sequenza ( Gestito TimeOut di Request )
            byte queueId = getQueueNodeFromId(state, transfer->metadata.remote_node_id);
            // Devo essere in aghgiornamento per sicurezza!!!
            if (state->slave[queueId].file.state == FILE_STATE_UPLOADING) {
                // Update TimeOut (Comunico request OK al Master, Slave sta scaricando)
                // Se Slave si blocca per TimeOut, esco dalla procedura dove gestita
                // La gestione dei time OUT è in unica funzione x Tutti i TimeOUT
                state->slave[queueId].file.timeout_us = transfer->timestamp_usec + NODE_REQFILE_TIMEOUT_US;
                uavcan_file_Read_Request_1_1 req = {0};
                size_t size = transfer->payload_size;
                if (uavcan_file_Read_Request_1_1_deserialize_(&req, static_cast<uint8_t const*>(transfer->payload), &size) >= 0) {
                    uavcan_file_Read_Response_1_1 resp = {0};
                    byte dataBuf[uavcan_primitive_Unstructured_1_0_value_ARRAY_CAPACITY_];
                    // Terminatore di sicurezza
                    req.path.path.elements[req.path.path.count] = 0;
                    // Allego il blocco dati se presente
                    size_t dataLen = uavcan_primitive_Unstructured_1_0_value_ARRAY_CAPACITY_;
                    // Il Master ha finito la trasmissione, esco dalla procedura
                    if (getDataFile((char*)&req.path.path.elements[0], state->slave[queueId].file.is_firmware,
                                    req.offset, dataBuf, &dataLen)) {
                        resp._error.value = uavcan_file_Error_1_0_OK;
                        if (dataLen != uavcan_primitive_Unstructured_1_0_value_ARRAY_CAPACITY_) {
                            state->slave[queueId].file.state = FILE_STATE_UPLOAD_COMPLETE;
                        }
                    } else {
                        // Ritorno un errore da interpretare a Slave
                        resp._error.value = uavcan_file_Error_1_0_IO_ERROR;
                        dataLen = 0;
                    }
                    // Preparo la risposta corretta
                    resp.data.value.count = dataLen;
                    memcpy(resp.data.value.elements, dataBuf, dataLen);
                    uint8_t serialized[uavcan_file_Read_Response_1_1_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
                    size_t serialized_size = sizeof(serialized);
                    if (uavcan_file_Read_Response_1_1_serialize_(&resp, &serialized[0], &serialized_size) >= 0) {
                        sendResponse(state,
                                     transfer->timestamp_usec + MEGA,
                                     &transfer->metadata,
                                     serialized_size,
                                     &serialized[0]);
                    }
                }
            }
        }
    }
    // Gestione delle risposte alle richeste inviate alla rete come Master
    else if (transfer->metadata.transfer_kind == CanardTransferKindResponse) {
        // Comando inviato ad un nodo remoto, verifica della risposta e della coerenza messaggio
        if (transfer->metadata.port_id == uavcan_node_ExecuteCommand_1_1_FIXED_PORT_ID_) {
            uavcan_node_ExecuteCommand_Response_1_1 resp = {0};
            size_t size = transfer->payload_size;
            if (uavcan_node_ExecuteCommand_Response_1_1_deserialize_(&resp, static_cast<uint8_t const*>(transfer->payload), &size) >= 0) {
                // Ricerco idNodo nella coda degli allocati del master
                // Copio la risposta ricevuta nella struttura relativa e resetto il flag pending
                byte queueId = getQueueNodeFromId(state, transfer->metadata.remote_node_id);
                if (queueId != GENERIC_BVAL_UNDEFINED) {
                    // Resetta il pending del comando del nodo verificato
                    state->slave[queueId].command.is_pending = false;
                    // Copia la risposta nella variabile di chiamata in state
                    state->slave[queueId].command.response = resp.status;
                }
            }
        } else if (transfer->metadata.port_id == uavcan_register_Access_1_0_FIXED_PORT_ID_) {
            uavcan_register_Access_Response_1_0 resp = {0};
            size_t size = transfer->payload_size;
            if (uavcan_register_Access_Response_1_0_deserialize_(&resp, static_cast<uint8_t const*>(transfer->payload), &size) >= 0) {
                // Ricerco idNodo nella coda degli allocati del master
                // Copio la risposta ricevuta nella struttura relativa e resetto il flag pending
                byte queueId = getQueueNodeFromId(state, transfer->metadata.remote_node_id);
                if (queueId != GENERIC_BVAL_UNDEFINED) {
                    // Resetta il pending del comando del nodo verificato
                    state->slave[queueId].register_access.is_pending = false;
                    // Copia la risposta nella variabile di chiamata in state
                    state->slave[queueId].register_access.response = resp.value;
                }
            }
        } else if (transfer->metadata.port_id == uavcan_file_Read_1_1_FIXED_PORT_ID_) {
            uavcan_file_Read_Response_1_1 resp = {0};
            size_t size = transfer->payload_size;
            if (uavcan_file_Read_Response_1_1_deserialize_(&resp, static_cast<uint8_t const*>(transfer->payload), &size) >= 0) {
                // Reset pending command (Comunico request/Response Serie di comandi OK!!!)
                state->master.file.is_pending = false;
                // Azzero contestualmente le retry di controllo x gestione MAX_RETRY -> ABORT
                state->master.file.updating_retry = 0;
                // Save Data in File at Block Position (Init = Rewrite file...)
                putDataFile(state->master.file.filename, state->master.file.is_firmware, state->master.file.offset == 0,
                            resp.data.value.elements, resp.data.value.count);
                state->master.file.offset += resp.data.value.count;
                if (resp.data.value.count != uavcan_primitive_Unstructured_1_0_value_ARRAY_CAPACITY_) {
                    // Blocco con EOF!!! Fine trasferimento, nessun altro blocco disponibile
                    state->master.file.updating_eof = true;
                }
            }
        }
        // Risposta ad un servizio (dati) dinamicamente allocato... ( deve essere ultimo else )
        // Servizio di risposta alla richiesta su modulo slave, verifica della risposta e della coerenza messaggio
        // Per il nodo che risponde verifico i servizi attivi per la corrispondenza dinamica risposta
        else {
            // Nodo rispondente (posso avere senza problemi più servizi con stesso port_id su diversi nodi)
            byte queueId = getQueueNodeFromId(state, transfer->metadata.remote_node_id);
            // Se nodo correttammente allocato e gestito
            if (queueId != GENERIC_BVAL_UNDEFINED) {
                // Verifico se risposta del servizio corrisponde al chiamante (eventuali + servizi sotto...)
                // Gestione di tutti i servizi possibili allocabili, valido per tutti i nodi
                if (transfer->metadata.port_id == state->slave[queueId].rmap_service.port_id) {
                    // *************            Service Modulo TH Response            *************
                    if (state->slave[queueId].node_type == MODULE_TYPE_TH) {
                        // Modulo TH, leggo e deserializzo il messaggio in ingresso
                        rmap_service_module_TH_Response_1_0 resp = {0};
                        size_t size = transfer->payload_size;
                        if (rmap_service_module_TH_Response_1_0_deserialize_(&resp, static_cast<uint8_t const*>(transfer->payload), &size) >= 0) {
                            // Resetta il pending del comando del nodo verificato
                            state->slave[queueId].rmap_service.is_pending = false;
                            // Copia la risposta nella variabile di chiamata in state
                            // Oppure gestire qua tutte le altre occorrenze per stima V4
                            // TODO: vedere con Marco Pubblica, registra elimina, display... altro
                            // Per ora copio in una struttura di state response
                            memcpy(state->slave[queueId].rmap_service.module, &resp, sizeof(resp));
                        }
                    }
                    // ALTRI MODULI DA INSERIRE QUA... PG, VV, RS, GAS ECC...
                }
            }
        }
    }
}

// *********************************************************************************************
//          Inizializzazione generale HW, canard, CAN_BUS, ISR e dispositivi collegati
// *********************************************************************************************

// Setup SW - Canard memory access (allocate/free)
static void* canardAllocate(CanardInstance* const ins, const size_t amount) {
    O1HeapInstance* const heap = ((State*)ins->user_reference)->heap;
    return o1heapAllocate(heap, amount);
}

static void canardFree(CanardInstance* const ins, void* const pointer) {
    O1HeapInstance* const heap = ((State*)ins->user_reference)->heap;
    o1heapFree(heap, pointer);
}

// ***************** ISR READ RX CAN_BUS, BUFFER RX SETUP ISR, CALLBACK *****************
// Push data into Array of CanardFrame and relative buffer payload
// Gestita come coda FIFO (In sostituzione interrupt bxCAN non funzionante correttamente)
// Puntatori alla coda FiFo (Gestione CODA con define)
#define bxCANRxQueueEmpty()       (canard_rx_queue.wr_ptr = canard_rx_queue.rd_ptr)
#define bxCANRxQueueIsEmpty()     (canard_rx_queue.wr_ptr == canard_rx_queue.rd_ptr)
#define bxCANRxQueueDataPresent() (canard_rx_queue.wr_ptr != canard_rx_queue.rd_ptr)
// For Monitor queue Interrupt RX movement
#define bxCANRxQueueElement()      (canard_rx_queue.wr_ptr >= canard_rx_queue.rd_ptr ? canard_rx_queue.wr_ptr - canard_rx_queue.rd_ptr : canard_rx_queue.wr_ptr + (CAN_RX_QUEUE_CAPACITY - canard_rx_queue.rd_ptr))
#define bxCANRxQueueNextElement(x) (x + 1 < CAN_RX_QUEUE_CAPACITY ? x + 1 : 0)
typedef struct Canard_rx_queue {
    // CanardTxQueue (Frame e Buffer x Interrupt gestione Coda FiFo)
    byte wr_ptr;
    byte rd_ptr;
    struct {
        CanardFrame frame;
        uint8_t buf[CANARD_MTU_MAX];
    } msg[CAN_RX_QUEUE_CAPACITY];
} Canard_rx_queue;
Canard_rx_queue canard_rx_queue;

// Call Back Opzionale RX_FIFO0 CAN_IFACE (hcan), Usare con più servizi di INT per discriminare
// Abilitabile in CAN1_RX0_IRQHandler, chiamando -> HAL_CAN_IRQHandler(&CAN_Handle)
// extern "C" void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
// {
//    CallBack CODE Here... (quello Interno CAN1_RX0_IRQHandler)
// }

// INTERRUPT_HANDLER CAN RX (Non modificare extern "C", in C++ non gestirebbe l'ingresso in ISR)
// Più veloce possibile ISR
extern "C" void CAN1_RX0_IRQHandler(void) {
    // -> Chiamata opzionale di Handler Call_Back CAN_Handle
    // La sua chiamata in CAN1_RX0_IRQHandler abilita il CB succesivo
    // -> HAL_CAN_IRQHandler(&CAN_Handle);
    // <- HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
    // In questo caso è possibile discriminare con *hcan altre opzioni/stati
    // In Stima V4 non è necessario altra CB, bxCANPop gestisce il MSG_IN a suo modo
    // Inserisco il messaggio in un BUFFER Circolare di CanardFrame che vengono gestiti
    // nel software al momento opportuno, senza così incorrere in perdite di dati
    byte testElement = canard_rx_queue.wr_ptr;
    testElement = bxCANRxQueueNextElement(canard_rx_queue.wr_ptr);
    // Leggo il messaggio già pronto per libreria CANARD (Frame)
    if (bxCANPop(IFACE_CAN_IDX, &canard_rx_queue.msg[testElement].frame.extended_can_id,
                 &canard_rx_queue.msg[testElement].frame.payload_size,
                 canard_rx_queue.msg[testElement].buf)) {
        if (testElement != canard_rx_queue.rd_ptr) {
            // Non posso registrare il dato (MAX_QUEUE) se (testElement == canard_rx_queue.rd_ptr)
            // raggiunto MAX Buffer. E' più importante non perdere il primo FIFO payload
            // Quindi non aggiungo il dato ma leggo per svuotare il Buffer FIFO
            // altrimenti rientro sempre in Interrupt RX e mando in stallo la CPU senza RX...
            // READ DATA BUFFER MSG ->
            // Get payload from Buffer (possibilie inizializzazione statica fissa)
            // Il Buffer non cambia indirizzo quindi basterebbe un'init statico di frame[x].payload
            canard_rx_queue.msg[testElement].frame.payload = canard_rx_queue.msg[testElement].buf;
            // Push data in queue (Next_WR, Data in testElement + 1 Element from RX)
            canard_rx_queue.wr_ptr = testElement;
        }
    }
}

//*******************************************************************************************
// *********************************** SETUP CAN BUS IFACE **********************************
//*******************************************************************************************
bool CAN_HW_Init(void) {
    // Definition CAN structure variable
    CAN_HandleTypeDef CAN_Handle;

    // Definition GPIO and CAN filter structure variables
    GPIO_InitTypeDef GPIO_InitStruct;
    CAN_FilterTypeDef CAN_FilterInitStruct;

    // GPIO Ports clock enable
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // CAN1 clock enable
    __HAL_RCC_CAN1_CLK_ENABLE();

#if defined(STM32L496xx)
    // Mapping GPIO for CAN
    /* Configure CAN pin: RX */
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    /* Configure CAN pin: TX */
    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
#else
#error "Warning untested processor variant"
#endif

    // Setup CAN Istance Basic
    CAN_Handle.Instance = CAN1;
    CAN_Handle.Init.Mode = CAN_MODE_NORMAL;
    CAN_Handle.Init.TimeTriggeredMode = DISABLE;
    CAN_Handle.Init.AutoBusOff = DISABLE;
    CAN_Handle.Init.AutoWakeUp = DISABLE;
    CAN_Handle.Init.AutoRetransmission = DISABLE;
    CAN_Handle.Init.ReceiveFifoLocked = DISABLE;
    CAN_Handle.Init.TransmitFifoPriority = DISABLE;
    // Check error initialization CAN
    if (HAL_CAN_Init(&CAN_Handle) != HAL_OK) {
        return false;
    }

    // CAN filter basic initialization
    CAN_FilterInitStruct.FilterIdHigh = 0x0000;
    CAN_FilterInitStruct.FilterIdLow = 0x0000;
    CAN_FilterInitStruct.FilterMaskIdHigh = 0x0000;
    CAN_FilterInitStruct.FilterMaskIdLow = 0x0000;
    CAN_FilterInitStruct.FilterFIFOAssignment = CAN_RX_FIFO0;
    CAN_FilterInitStruct.FilterBank = 0;
    CAN_FilterInitStruct.FilterMode = CAN_FILTERMODE_IDMASK;
    CAN_FilterInitStruct.FilterScale = CAN_FILTERSCALE_32BIT;
    CAN_FilterInitStruct.FilterActivation = ENABLE;

    // Check error initalization CAN filter
    if (HAL_CAN_ConfigFilter(&CAN_Handle, &CAN_FilterInitStruct) != HAL_OK) {
        return false;
    }

    // *******************         CANARD SETUP TIMINGS AND SPEED        *******************
    // CAN BITRATE Dinamico su LoadRegister (CAN_FD 2xREG natural32 0=Speed, 1=0 (Not Used))
    uavcan_register_Value_1_0 val = {0};
    uavcan_register_Value_1_0_select_natural32_(&val);
    val.natural32.value.count = 2;
    val.natural32.value.elements[0] = CAN_BIT_RATE;
    val.natural32.value.elements[1] = 0ul;  // Ignored for CANARD_MTU_CAN_CLASSIC
    registerRead("uavcan.can.bitrate", &val);

    // Dynamic BIT RATE Change CAN Speed to CAN_BIT_RATE (register default/defined)
    BxCANTimings timings;
    bool result = bxCANComputeTimings(HAL_RCC_GetPCLK1Freq(), val.natural32.value.elements[0], &timings);
    if (!result) {
        val.natural32.value.count = 2;
        val.natural32.value.elements[0] = CAN_BIT_RATE;
        val.natural32.value.elements[1] = 0ul;  // Ignored for CANARD_MTU_CAN_CLASSIC
        registerWrite("uavcan.can.bitrate", &val);
        result = bxCANComputeTimings(HAL_RCC_GetPCLK1Freq(), val.natural32.value.elements[0], &timings);
        if (!result) {
            return false;
        }
    }
    // Attivazione bxCAN sulle interfacce richieste, velocità e modalità
    result = bxCANConfigure(0, timings, false);
    if (!result) {
        return false;
    }
    // *******************     CANARD SETUP TIMINGS AND SPEED COMPLETE   *******************

    // Check error starting CAN
    if (HAL_CAN_Start(&CAN_Handle) != HAL_OK) {
        return false;
    }

    // Enable Interrupt RX Standard CallBack -> CAN1_RX0_IRQHandler
    if (HAL_CAN_ActivateNotification(&CAN_Handle, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
        return false;
    }
    // Setup Priority e CB CAN_IRQ_RX Enable
    HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);

    // Setup Completato
    return true;
}

/**
 * @brief Return false when the hardware doesn't work correctly
 *
 */
void exit_false() {
    TEST_ASSERT_TRUE(false);
}

/**
 * @brief Test: slave is offline
 *
 */
void test_slave_is_online() {
    TEST_ASSERT_TRUE_MESSAGE(state.slave[queueId].is_online, "Slave is offline");
}

/**
 * @brief Test: slave is online
 *
 */
void test_slave_is_offline() {
    TEST_ASSERT_TRUE_MESSAGE(!state.slave[queueId].is_online, "Slave is online");
}

void setup() {
    UNITY_BEGIN();

    delay(1000);

    if (!setupSd(PIN_SPI_MOSI, PIN_SPI_MISO, PIN_SPI_SCK, PIN_SPI_SS, 18) || !CAN_HW_Init()) {
        RUN_TEST(exit_false);
        UNITY_END();
    }

// ********************************************************************************
//    FIXED REGISTER_INIT, FARE INIT OPZIONALE x REGISTRI FISSI ECC. E/O INVAR.
// ********************************************************************************
#ifdef INIT_REGISTER
    // Inizializzazione fissa dei registri nella modalità utilizzata (prepare SD/FLASH/ROM con default Value)
    registerSetup(true);
#else
    // Default dei registri nella modalità utilizzata (prepare SD/FLASH/ROM con default Value)
    // Creazione dei registri standard base se non esistono
    registerSetup(false);
#endif

    uavcan_register_Value_1_0 val = {0};

    // A simple node like this one typically does not require more than 8 KiB of heap and 4 KiB of stack.
    // For the background and related theory refer to the following resources:
    // - https://github.com/OpenCyphal/libcanard/blob/master/README.md
    // - https://github.com/pavel-kirienko/o1heap/blob/master/README.md
    // - https://forum.opencyphal.org/t/uavcanv1-libcanard-nunavut-templates-memory-usage-concerns/1118/4
    _Alignas(O1HEAP_ALIGNMENT) static uint8_t heap_arena[1024 * 16] = {0};
    state.heap = o1heapInit(heap_arena, sizeof(heap_arena));

    // The libcanard instance requires the allocator for managing protocol states.
    state.canard = canardInit(&canardAllocate, &canardFree);
    state.canard.user_reference = &state;  // Make the state reachable from the canard instance.

    // Default Setup servizi attivi nel modulo
    state.publisher_enabled.port_list = DEFAULT_PUBLISH_PORT_LIST;

    // ********************************************************************************
    //            INIT VALUE, Caricamento default e registri locali MASTER
    // ********************************************************************************
    // Reset Slave node_id per ogni nodo collegato. Solo i nodi validi verranno gestiti
    for (uint8_t iCnt = 0; iCnt < MAX_NODE_CONNECT; iCnt++) {
        state.slave[iCnt].node_id = CANARD_NODE_ID_UNSET;
        state.slave[iCnt].rmap_service.port_id = UINT16_MAX;
        state.slave[iCnt].rmap_service.module = NULL;
#ifdef USE_SUB_PUBLISH_SLAVE_DATA
        state.slave[iCnt].publisher.subject_id = UINT16_MAX;
#endif
        state.slave[iCnt].file.state = FILE_STATE_STANDBY;
    }

    // Canard Master NODE ID Fixed dal defined value in module_config
    state.canard.node_id = (CanardNodeID)NODE_MASTER_ID;

    // ********************************************************************************
    //                   READING PARAM FROM E2 MEMORY / FLASH / SDCARD
    // ********************************************************************************

    // TODO:
    // Read Config Slave Node x Lettura porte e servizi.
    // Possibilità di utilizzo come sotto (registri) - Fixed Value adesso !!!
    state.slave[0].node_id = 125;
    state.slave[0].node_type = MODULE_TYPE_TH;
    state.slave[0].rmap_service.port_id = 100;
#ifdef USE_SUB_PUBLISH_SLAVE_DATA
// state.slave[0].subject_id = 5678;
#endif
    state.slave[0].rmap_service.module = malloc(sizeof(rmap_service_module_TH_Response_1_0));
    strcpy(state.slave[0].file.filename, "stima4.module_th-1.1.app.hex");

    // **********************************************************************************
    // Lettura registri, parametri per PNP Allocation Verifica locale di assegnamento CFG
    // **********************************************************************************
    for (uint8_t iCnt = 0; iCnt < MAX_NODE_CONNECT; iCnt++) {
        // Lettura registro di allocazione PNP MASTER Locale avvenuta, da eseguire
        // Possibilità di salvare tutte le informazioni di NODO qui al suo interno
        // Per rendere disponibili le configurazioni in esterno (Yakut, altri)
        // Utilizzando la struttupra allocateID.XX (count = n° registri utili)
        char registerName[24] = "rmap.pnp.allocateID.";
        uavcan_register_Value_1_0_select_natural8_(&val);
        val.natural8.value.count = 1;
        val.natural8.value.elements[0] = CANARD_NODE_ID_UNSET;
        // queueId -> index Val = NodeId
        itoa(iCnt, registerName + strlen(registerName), 10);
        registerRead(registerName, &val);
        // Il Node_id deve essere valido e uguale a quello programmato in configurazione
        if ((val.natural8.value.elements[0] != CANARD_NODE_ID_UNSET) &&
            (val.natural8.value.elements[0] == state.slave[iCnt].node_id)) {
            // Assegnamento PNP per nodeQueueID con state.slave[iCnt].node_id
            // già avvenuto. Non rispondo a eventuali messaggi PNP del tipo per quel nodo
            state.slave[iCnt].pnp.is_assigned = true;
        }
    }

    // ********************************************************************************
    // *********               Lettura Registri standard UAVCAN               *********
    // ********************************************************************************

    // The description register is optional but recommended because it helps constructing/maintaining large networks.
    // It simply keeps a human-readable description of the node that should be empty by default.
    uavcan_register_Value_1_0_select_string_(&val);
    val._string.value.count = 0;
    registerRead("uavcan.node.description", &val);  // We don't need the value, we just need to ensure it exists.

    // Configura il trasporto dal registro standard uavcan. Default a CANARD_MTU_MAX
    // Inserito per compatibilità, attualmente non gestita la modifica mtu_bytes (FISSA A MTU_CLASSIC)
    uavcan_register_Value_1_0_select_natural16_(&val);
    val.natural16.value.count = 1;
    val.natural16.value.elements[0] = CANARD_MTU_MAX;
    registerRead("uavcan.can.mtu", &val);
    if (val.natural16.value.elements[0] != CANARD_MTU_MAX) {
        val.natural16.value.count = 1;
        val.natural16.value.elements[0] = CANARD_MTU_MAX;
        registerWrite("uavcan.can.mtu", &val);
    }
    for (uint8_t ifidx = 0; ifidx < CAN_REDUNDANCY_FACTOR; ifidx++) {
        state.canard_tx_queues[ifidx] = canardTxInit(CAN_TX_QUEUE_CAPACITY, val.natural16.value.elements[0]);
    }

    // ********************************************************************************
    //               AVVIA SOTTOSCRIZIONI ai messaggi per servizi RPC ecc...
    // ********************************************************************************

    // Service servers: -> Risposta per GetNodeInfo richiesta esterna (Yakut, Altri)
    {
        static CanardRxSubscription rx;
        const int8_t res =  //
            canardRxSubscribe(&state.canard,
                              CanardTransferKindRequest,
                              uavcan_node_GetInfo_1_0_FIXED_PORT_ID_,
                              uavcan_node_GetInfo_Request_1_0_EXTENT_BYTES_,
                              CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
                              &rx);
        if (res < 0) NVIC_SystemReset();
    }

    // Service servers: -> Chiamata per ExecuteCommand richiesta esterna (Yakut, Altri)
    {
        static CanardRxSubscription rx;
        const int8_t res =  //
            canardRxSubscribe(&state.canard,
                              CanardTransferKindRequest,
                              uavcan_node_ExecuteCommand_1_1_FIXED_PORT_ID_,
                              uavcan_node_ExecuteCommand_Request_1_1_EXTENT_BYTES_,
                              CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
                              &rx);
        if (res < 0) NVIC_SystemReset();
    }

    // Service servers: -> Risposta per Accesso ai registri richiesta esterna (Yakut, Altri)
    {
        static CanardRxSubscription rx;
        const int8_t res =  //
            canardRxSubscribe(&state.canard,
                              CanardTransferKindRequest,
                              uavcan_register_Access_1_0_FIXED_PORT_ID_,
                              uavcan_register_Access_Request_1_0_EXTENT_BYTES_,
                              CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
                              &rx);
        if (res < 0) NVIC_SystemReset();
    }

    // Service servers: -> Risposta per Lista dei registri richiesta esterna (Yakut, Altri)
    {
        static CanardRxSubscription rx;
        const int8_t res =  //
            canardRxSubscribe(&state.canard,
                              CanardTransferKindMessage,
                              uavcan_register_List_1_0_FIXED_PORT_ID_,
                              uavcan_register_List_Request_1_0_EXTENT_BYTES_,
                              CANARD_REGISTERLIST_TRANSFER_ID_TIMEOUT_USEC,
                              &rx);
        if (res < 0) NVIC_SystemReset();
    }

    // ******* SOTTOSCRIZIONE MESSAGGI / COMANDI E SERVIZI AD UTILITA' MASTER ********

    // Messaggi PNP_Allocation: -> Allocazione dei nodi standard in PlugAndPlay per i nodi conosciuti
    {
        static CanardRxSubscription rx;
        const int8_t res =  //
            canardRxSubscribe(&state.canard,
                              CanardTransferKindMessage,
                              uavcan_pnp_NodeIDAllocationData_1_0_FIXED_PORT_ID_,
                              uavcan_pnp_NodeIDAllocationData_1_0_EXTENT_BYTES_,
                              CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
                              &rx);
        if (res < 0) NVIC_SystemReset();
    }

    // Messaggi HEARTBEAT: -> Verifica della presenza per stato Nodi (Slave) OnLine / OffLine
    {
        static CanardRxSubscription rx;
        const int8_t res =  //
            canardRxSubscribe(&state.canard,
                              CanardTransferKindMessage,
                              uavcan_node_Heartbeat_1_0_FIXED_PORT_ID_,
                              uavcan_node_Heartbeat_1_0_EXTENT_BYTES_,
                              CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
                              &rx);
        if (res < 0) NVIC_SystemReset();
    }

    // Service client: -> Risposta per ExecuteCommand richiesta interna (come master)
    {
        static CanardRxSubscription rx;
        const int8_t res =  //
            canardRxSubscribe(&state.canard,
                              CanardTransferKindResponse,
                              uavcan_node_ExecuteCommand_1_1_FIXED_PORT_ID_,
                              uavcan_node_ExecuteCommand_Response_1_1_EXTENT_BYTES_,
                              CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
                              &rx);
        if (res < 0) NVIC_SystemReset();
    }

    // Service client: -> Risposta per Accesso ai registri richiesta interna (come master)
    {
        static CanardRxSubscription rx;
        const int8_t res =  //
            canardRxSubscribe(&state.canard,
                              CanardTransferKindResponse,
                              uavcan_register_Access_1_0_FIXED_PORT_ID_,
                              uavcan_register_Access_Response_1_0_EXTENT_BYTES_,
                              CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
                              &rx);
        if (res < 0) NVIC_SystemReset();
    }

    // Service client: -> Risposta per Read (Receive) File local richiesta esterna (Yakut, Altri)
    {
        static CanardRxSubscription rx;
        const int8_t res =  //
            canardRxSubscribe(&state.canard,
                              CanardTransferKindResponse,
                              uavcan_file_Read_1_1_FIXED_PORT_ID_,
                              uavcan_file_Read_Response_1_1_EXTENT_BYTES_,
                              CANARD_READFILE_TRANSFER_ID_TIMEOUT_USEC,
                              &rx);
        if (res < 0) NVIC_SystemReset();
    }

    // Service server: -> Risposta per Read (Request Slave) File read archivio (come master)
    {
        static CanardRxSubscription rx;
        const int8_t res =  //
            canardRxSubscribe(&state.canard,
                              CanardTransferKindRequest,
                              uavcan_file_Read_1_1_FIXED_PORT_ID_,
                              uavcan_file_Read_Request_1_1_EXTENT_BYTES_,
                              CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
                              &rx);
        if (res < 0) NVIC_SystemReset();
    }

    // Allocazione dinamica delle subscription servizi request/response e pubblicazioni dei nodi remoti
    // in funzione delle registrazioni/istanze utilizzate nel master
    // Ogni subject utuilizzato per ogni nodo slave deve avere una sottoscrizione propria
    // Solo una sottoscrizione è possibile per singolo servizio per relativo port_id (dynamic o fixed)
    // La gestione di servizi con stesso port_id è assolutamente possibile e correttamente gestita
    // nel software. Basta solamente a livello server registrare lo stesso port_id per servizio.
    // La funzionailtà cosi impostata consente uno o più port_id x servizio senza problemi
    // Eventuali errori di allocazione possono eventualmente essere rilevati ma non ci sono problemi
    // sw in quanto una sottoscrizione chiamata in coda elimina una precedente (con stesso port o subjcect)
    for (byte queueId = 0; queueId < MAX_NODE_CONNECT; queueId++) {
        // *************   SERVICE    *************
        // Se previsto il servizio request/response con port_id valido
        if ((state.slave[queueId].rmap_service.module) &&
            (state.slave[queueId].rmap_service.port_id <= CANARD_SERVICE_ID_MAX)) {
            // Controllo le varie tipologie di request/service per il nodo
            if (state.slave[queueId].node_type == MODULE_TYPE_TH) {
                // Alloco la stottoscrizione in funzione del tipo di modulo
                // Service client: -> Risposta per ServiceDataModuleTH richiesta interna (come master)
                static CanardRxSubscription rx;
                const int8_t res =  //
                    canardRxSubscribe(&state.canard,
                                      CanardTransferKindResponse,
                                      state.slave[queueId].rmap_service.port_id,
                                      rmap_service_module_TH_Response_1_0_EXTENT_BYTES_,
                                      CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
                                      &rx);
                if (res < 0) NVIC_SystemReset();
            }
        }
#ifdef USE_SUB_PUBLISH_SLAVE_DATA
        // *************   PUBLISH    *************
        // Se previsto il servizio publisher (subject_id valido)
        // Non alloco niente per il publish (gestione esempio display o altro debug interno da gestire)
        if (state.slave[queueId].publisher.subject_id <= CANARD_SUBJECT_ID_MAX) {
            // Controllo le varie tipologie di request/service per il nodo
            if (state.slave[queueId].node_type == MODULE_TYPE_TH) {
                // Alloco la stottoscrizione in funzione del tipo di modulo
                // Service client: -> Sottoscrizione per ModuleTH (come master)
                static CanardRxSubscription rx;
                const int8_t res =  //
                    canardRxSubscribe(&state.canard,
                                      CanardTransferKindMessage,
                                      state.slave[queueId].publisher.subject_id,
                                      rmap_module_TH_1_0_EXTENT_BYTES_,
                                      CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
                                      &rx);
                if (res < 0) NVIC_SystemReset();
            }
        }
#endif
    }

    // ********************************************************************************
    //         AVVIA LOOP CANARD PRINCIPALE gestione TX/RX Code -> Messaggi
    // ********************************************************************************

    long checkTimeout = 0;
    bool bEventRealTimeLoop = false;

#define MILLIS_EVENT 10

    // Set START Timetable LOOP RX/TX.
    state.started_at = getMonotonicMicroseconds(&state);
    CanardMicrosecond next_01_sec_iter_at = state.started_at + MEGA * 0.25;
    CanardMicrosecond next_timesyncro_msg = state.started_at + MEGA;
    CanardMicrosecond next_20_sec_iter_at = state.started_at + MEGA * 1.5;
    CanardMicrosecond test_cmd_cs_iter_at = state.started_at + MEGA * 2.5;
    CanardMicrosecond test_cmd_vs_iter_at = state.started_at + MEGA * 3.5;
    CanardMicrosecond test_cmd_rg_iter_at = state.started_at + MEGA * 4.5;

// TEST TEMPORIZZATO PER FUNZIONE OK/ERRORE
#ifdef TEST_UAVCAN_NODE_ONLINE
    CanardMicrosecond test_software = state.started_at + MEGA * 10;
#elif TEST_UAVCAN_NODE_OFFLINE
    CanardMicrosecond test_software = state.started_at + MEGA * 20;
#endif

    // Run a trivial scheduler polling the loops that run the business logic.
    CanardMicrosecond monotonic_time;
    // monotonic_time.

    do {
        monotonic_time = getMonotonicMicroseconds(&state);

        // Check TimeLine (quasi RealTime...)
        if ((millis() - checkTimeout) >= MILLIS_EVENT) {
            // Deadline di controllo per eventi di controllo Rapidi (TimeOut, FileHandler ecc...)
            // Mancata risposta, nodo in Errore o Full o CanardHeapError ecc...
            checkTimeout = millis();
            // Utilizzo per eventi quasi continuativi... Es. Send/Receive File queue...
            bEventRealTimeLoop = true;
        }

        // ************************************************************************
        // ***************   CHECK OFFLINE/DEADLINE COMMAND/STATE   ***************
        // ************************************************************************
        // TEST Check ogni RTL circa ( SOLO TEST COMANDI DA INSERIRE IN TASK_TIME )
        // Deadline di controllo (checkTimeOut variabile sopra...)
        if (bEventRealTimeLoop) {
            // **********************************************************
            //          Per il nodo locale SERVER local_node_flag
            // **********************************************************
            // Controllo TimeOut Comando file su modulo remoto
            if (state.master.file.is_pending) {
                if (monotonic_time > state.master.file.timeout_us) {
                    // Setto il flag di TimeOUT che il master dovrà gestire (segnalazione BUG al Server?)
                    state.master.file.is_timeout = true;
                }
            }
            // **********************************************************
            // Per la coda/istanze allocate valide SLAVE remote_node_flag
            // **********************************************************
            for (byte queueId = 0; queueId < MAX_NODE_CONNECT; queueId++) {
                if (state.slave[queueId].node_id <= CANARD_NODE_ID_MAX) {
                    // Solo se nodo OnLine (Automatic OnLine su HeartBeat RX)
                    if (state.slave[queueId].is_online) {
                        // Controllo TimeOUT Comando diretto su modulo remoto
                        // Mancata risposta, nodo in Errore o Full o CanardHeapError ecc...
                        if (state.slave[queueId].command.is_pending) {
                            if (monotonic_time > state.slave[queueId].command.timeout_us) {
                                // Setto il flag di TimeOUT che il master dovrà gestire (segnalazione BUG al Server?)
                                state.slave[queueId].command.is_timeout = true;
                            }
                        }
                        // Controllo TimeOut Comando getData su modulo remoto
                        // Mancata risposta, nodo in Errore o Full o CanardHeapError ecc...
                        if (state.slave[queueId].rmap_service.is_pending) {
                            if (monotonic_time > state.slave[queueId].rmap_service.timeout_us) {
                                // Setto il flag di TimeOUT che il master dovrà gestire (segnalazione BUG al Server?)
                                state.slave[queueId].rmap_service.is_timeout = true;
                            }
                        }
                        // Controllo TimeOut Comando file su modulo remoto
                        // Mancata risposta, nodo in Errore o Full o CanardHeapError ecc...
                        if (state.slave[queueId].file.is_pending) {
                            if (monotonic_time > state.slave[queueId].file.timeout_us) {
                                // Setto il flag di TimeOUT che il master dovrà gestire (segnalazione BUG al Server?)
                                state.slave[queueId].file.is_timeout = true;
                            }
                        }
                        // Check eventuale Nodo OFFLINE (Ultimo comando sempre perchè posso)
                        // Effettuare eventuali operazioni di SET,RESET Cmd in sicurezza
                        if (monotonic_time > state.slave[queueId].heartbeat.timeout_us) {
                            // Entro in OffLine
                            state.slave[queueId].is_online = false;
                            // Metto i Flag in sicurezza, laddove dove non eventualmente gestito
                            // Eventuali altre operazioni quà su Reset Comandi
                            state.slave[queueId].command.is_pending = false;
                            state.slave[queueId].command.is_timeout = true;
                            state.slave[queueId].register_access.is_pending = false;
                            state.slave[queueId].register_access.is_timeout = true;
                            state.slave[queueId].file.is_pending = false;
                            state.slave[queueId].file.is_timeout = true;
                            state.slave[queueId].file.state = FILE_STATE_STANDBY;
                            state.slave[queueId].rmap_service.is_pending = false;
                            state.slave[queueId].rmap_service.is_timeout = true;

                            // ******************************************************************************
                            // ******************************** START TEST **********************************
                            // ******************************************************************************
                            // Per il test devo essere entrato OnLine per entrare in OffLine
                            // Per coerenza della sequenza Init->OnLine->OffLine
                            if (test_state == ONLINE) {
                                test_state = OFFLINE;
                            }
                            // ******************************************************************************
                            // ******************************** END TEST ************************************
                            // ******************************************************************************
                        }
                    }
                }
            }
        }

        // **************************************************************************
        //                                CANARD UAVCAN
        // Scheduler temporizzato dei messaggi / comandi da inviare alla rete UAVCAN
        // **************************************************************************

        // FILE HANDLER ( con controllo continuativo se avviato bEventRealTimeLoop )
        if (bEventRealTimeLoop) {
            // Verifica TimeOUT Occurs per File download
            if (state.master.file.is_timeout) {
                state.master.file.is_pending = false;
                state.master.file.is_timeout = false;
                // Gestione con eventuali Retry N Volte per poi abbandonare
                state.master.file.updating_retry++;
                if (state.master.file.updating_retry >= NODE_GETFILE_MAX_RETRY) {
                    state.master.file.updating = false;
                }
            }
            // Verifica file download in corso (entro se in download)
            if (state.master.file.updating) {
                // Se messaggio in pending non faccio niente è attendo la conferma del ResetPending
                // In caso di errore subentrerà il TimeOut e verrà essere gestita la retry
                if (!state.master.file.is_pending) {
                    // Fine pending, con messaggio OK. Verifico se EOF o necessario altro blocco
                    if (state.master.file.updating_eof) {
                        // Nessun altro evento necessario, chiudo File e stati
                        // procedo all'aggiornamento Firmware dopo le verifiche di conformità
                        // Ovviamente se si tratta di un file firmware
                        state.master.file.updating = false;
                        // Comunico a HeartBeat (Yakut o Altri) l'avvio dell'aggiornamento (se il file è un firmware...)
                        // Per Yakut Pubblicare un HeartBeat prima dell'Avvio quindi con il flag
                        // state->local_node.file.updating_run = true >> HeartBeat Counica Upgrade...
                        if (state.master.file.is_firmware) {
                            state.master.file.updating_run = true;
                        }
                        // Il Firmware Upload dovrà partire necessariamente almeno dopo l'invio completo
                        // di HeartBeat (svuotamento coda), quindi attendiamo 2/3 secondi poi via
                        // Counque non rispondo più ai comandi di update con file.updating_run = true
                        // FirmwareUpgrade(*NameFile)... -> Fra 2/3 secondi dopo HeartBeat
                    } else {
                        // Avvio prima request o nuovo blocco (Set Flag e TimeOut)
                        // Prima request (state.local_node.file.offset == 0)
                        // Firmmware Posizione blocco gestito automaticamente in sequenza Request/Read
                        state.master.file.is_pending = true;
                        state.master.file.is_timeout = false;
                        state.master.file.timeout_us = monotonic_time + NODE_GETFILE_TIMEOUT_US;
                        // Gestione retry (incremento su TimeOut/Error) Automatico in Init/Request-Response
                        // Esco se raggiunga un massimo numero di retry x Frame... sopra
                        // Get Data Block per popolare il File
                        // Se il Buffer è pieno = 256 Caratteri è necessario continuare
                        // Altrimenti se inferiore o (0 Compreso) il trasferimento file termina.
                        // Se = 0 il blocco finale non viene considerato ma serve per il protocollo
                        // Se l'ultimo buffer dovesse essere pieno discrimina l'eventualità di MaxBuf==Eof
                        handleFileReadBlock_1_1(&state, monotonic_time);
                    }
                }
            }
        }

        // LOOP HANDLER >> 1 SECONDO << HEARTBEAT
        if (monotonic_time >= next_01_sec_iter_at) {
            next_01_sec_iter_at += MEGA;
            handleNormalLoop(&state, monotonic_time);
        }

        // LOOP HANDLER >> 1 SECONDO << TIME SYNCRO (alternato 0.5 sec con Heartbeat)
        // Disattivo con Firware Upgrade locale...
        if ((monotonic_time >= next_timesyncro_msg) && (!state.master.file.updating)) {
            next_timesyncro_msg += MEGA;
            handleSyncroLoop(&state, monotonic_time);
        }

        // LOOP HANDLER >> 20 SECONDI PUBLISH SERVIZI <<
        // Disattivo con Firware Upgrade locale...
        if ((monotonic_time >= next_20_sec_iter_at) && (!state.master.file.updating)) {
            next_20_sec_iter_at += MEGA * 20;
            handleSlowLoop(&state, monotonic_time);
        }

        // Fine handler quasi RealTime...
        // Attendo nuovo evento per rielaborare
        // Utilizzato per blinking Led (TX/RX)
        if (bEventRealTimeLoop) {
            bEventRealTimeLoop = false;
        };

        // ***************************************************************************
        //   Gestione Coda messaggi in trasmissione (ciclo di svuotamento messaggi)
        // ***************************************************************************
        // Transmit pending frames from the prioritized TX queues managed by libcanard.
        for (uint8_t ifidx = 0; ifidx < CAN_REDUNDANCY_FACTOR; ifidx++) {
            CanardTxQueue* const que = &state.canard_tx_queues[ifidx];
            const CanardTxQueueItem* tqi = canardTxPeek(que);  // Find the highest-priority frame.
            while (tqi != NULL) {
// Delay Microsecond di sicurezza in Send (Migliora sicurezza RX Pacchetti)
// Da utilizzare con CPU poco performanti in RX o con controllo Polling gestito Canard
#if (CAN_DELAY_US_SEND > 0)
                delayMicroseconds(CAN_DELAY_US_SEND);
#endif
                // Attempt transmission only if the frame is not yet timed out while waiting in the TX queue.
                // Otherwise just drop it and move on to the next one.
                if ((tqi->tx_deadline_usec == 0) || (tqi->tx_deadline_usec > monotonic_time)) {
                    // Non-blocking write attempt.
                    if (bxCANPush(0,
                                  monotonic_time,
                                  tqi->tx_deadline_usec,
                                  tqi->frame.extended_can_id,
                                  tqi->frame.payload_size,
                                  tqi->frame.payload)) {
                        // Imposto il timestamp reale della trasmissione syncronization_time
                        // Essendo a priorità immediata (unico) è il primo pacchetto comunque a partire
                        // Una volta attivata la funzione setto il bool relativo e resetto all'invio
                        if (state.master.timestamp.enable_immediate_tx_real) {
                            // Reset var RealTimeStamp
                            state.master.timestamp.enable_immediate_tx_real = false;
                            // Salvo RealTimeStamp letto dal tempo monotonic
                            // TODO: Inviare il vero timeStamp letto da RTC per syncro remoto
                            state.master.timestamp.previous_tx_real = getMonotonicMicroseconds(&state);
                        }
                        state.canard.memory_free(&state.canard, canardTxPop(que, tqi));
                        tqi = canardTxPeek(que);
                    } else {
                        // Empty Queue
                        break;
                    }
                } else {
                    // loop continuo per mancato aggiornamento monotonic_time su TIME_OUT
                    // grandi quantità di dati trasmesse e raggiunto il TIMEOUT Subscription...
                    // Remove frame per blocco in timeout BUG trasmission security !!!
                    state.canard.memory_free(&state.canard, canardTxPop(que, tqi));
                    // Test prossimo pacchetto
                    tqi = canardTxPeek(que);
                }
            }
        }

        // ***************************************************************************
        //   Gestione Coda messaggi in ricezione (ciclo di caricamento messaggi)
        // ***************************************************************************
        // Gestione con Intererupt RX Only esterean (verifica dati in coda gestionale)
        if (bxCANRxQueueDataPresent()) {
            // Leggo l'elemento disponibile in coda BUFFER RX FiFo CanardFrame + Buffer
            byte getElement = bxCANRxQueueNextElement(canard_rx_queue.rd_ptr);
            canard_rx_queue.rd_ptr = getElement;
            // Passaggio CanardFrame Buffered alla RxAccept CANARD
            // DeadLine a partire da getMonotonicMicroseconds() realTime assoluto
            const CanardMicrosecond timestamp_usec = getMonotonicMicroseconds(&state);
            CanardRxTransfer transfer;
            const int8_t canard_result = canardRxAccept(&state.canard, timestamp_usec, &canard_rx_queue.msg[getElement].frame, IFACE_CAN_IDX, &transfer, NULL);
            if (canard_result > 0) {
                processReceivedTransfer(&state, &transfer);
                state.canard.memory_free(&state.canard, (void*)transfer.payload);
            } else if ((canard_result == 0) || (canard_result == -CANARD_ERROR_OUT_OF_MEMORY)) {
                (void)0;  // The frame did not complete a transfer so there is nothing to do.
                // OOM should never occur if the heap is sized correctly. You can track OOM errors via heap API.
            }
        }
    }
#ifdef TEST_UAVCAN_NODE_ONLINE
    while ((!state.flag.g_restart_required) && (monotonic_time <= test_software) && (test_state != ONLINE));
#elif TEST_UAVCAN_NODE_OFFLINE
    while ((!state.flag.g_restart_required) && (monotonic_time <= test_software) && (test_state != OFFLINE));
#endif

#ifdef TEST_UAVCAN_NODE_ONLINE
    RUN_TEST(test_slave_is_online);
#elif TEST_UAVCAN_NODE_OFFLINE
    RUN_TEST(test_slave_is_offline);
#endif

    UNITY_END();
}

void loop() {
}

#endif