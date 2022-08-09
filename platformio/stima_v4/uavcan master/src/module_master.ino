/// This software is distributed under the terms of the MIT License.
/// Copyright (C) 2021 OpenCyphal <maintainers@opencyphal.org>
/// Author: Pavel Kirienko <pavel@opencyphal.org>
/// Revis.: Gasperini Moreno <m.gasperini@digiteco.it>

/// TODO: (ricerca e ricontrolla i TODO vari)
/// DSDL Esatta e conforme status, command, Lx, ecc. versione FINALE e tutte le altre slave
/// Funzioni per TimeStamp e File
/// Inserire disturbatore HW e verifiche restore, reset ecc...
/// STATO AD ARRAY [x] 1 stato per nodo slave su TX Messaggi -> Converitre C++ istanze
/// BXCAN AD Interrupt - SLAVE * MASTER SU RX, Filtri?, probabile gestione a NodeID Master + 1 x Yakut ???
/// C++/strutture
/// CanardMicrosecond
/// CanBitRate Dinamico, Fixed register non mutable...

/// NOTE:
/// evitare pubblicazioni sovrapposte a comandi con high_priority (ci sono perdite di frame possibili)
/// i comandi passano lo stesso senza problemi ma essendo tutto sotto il controllo Micro, gestire
/// msg->vendor_specific_status_code in Heartbeat può indicare (Fine acq, + altri STATI locali)
///     Errori, bassa alimentazione 0/1/2, Ho finito acq. in che modalità sono ecc... GIA OK!!!
///     Va solo gestito in logica master ma funziona correttamente
/// Verificare, serialized_size in 7510 per nulla convinto. Controllare RAM con tutti i servizi master/slave
/// uint8_t serialized[512] = {0};  // https://github.com/OpenCyphal/nunavut/issues/191 !!!!!
/// Capire L1/L2 quali parametri sono in master... x SET Misura ecc...

// Arduino
#include <Arduino.h>
// Libcanard
#include "register.hpp"
#include <o1heap.h>
#include <canard.h>
#include "bxcan.h"
// Namespace UAVCAN
#include <uavcan/node/Heartbeat_1_0.h>
#include <uavcan/node/GetInfo_1_0.h>
#include <uavcan/node/ExecuteCommand_1_1.h>
#include <uavcan/node/port/List_0_1.h>
#include <uavcan/_register/Access_1_0.h>
#include <uavcan/_register/List_1_0.h>
// Namespace RMAP
#include <rmap/_module/TH_1_0.h>
#include <rmap/service/_module/TH_1_0.h>
// Standard Library
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
// Configurazione modulo, definizioni ed utility generiche
#include "module_config.hpp"

/// We keep the state of the application here. Feel free to use static variables instead if desired.
typedef struct State
{
    CanardMicrosecond started_at;

    O1HeapInstance* heap;
    CanardInstance  canard;
    CanardTxQueue   canard_tx_queues[CAN_REDUNDANCY_FACTOR];

    // Stato dei nodi remoti e servizi gestiti collegati. Possibile lettura dai registri e gestione automatica
    // TODO: Vedere se fare elenco dinamico/statico con servizi_struttura statico/dinamico
    // TODO: Da convertire C++ con struttura ADD Istanza in funzione di Numero Nodi presenti
    struct
    {
        // Parametri Statici da leggere da registri o altro. Comunque inviati dal Master
        // node_id è l'indirizzo nodo remoto sui cui gestire i servizi
        // node_type identifica il tipo di nodo per sapere che tipologia di gestione viene effettuata
        // node_port_id è il port id sul node_id remoto che risponde ai servizi relativi di request
        // node_subject_id è il subject id sul node_id remoto che pubblica i dati quando attivato
        uint8_t  node_id;
        uint8_t  node_type;
        CanardPortID port_id;
        #ifdef USE_SUB_PUBLISH_SLAVE_DATA
        CanardPortID subject_id;
        #endif
        // Parametri dinamici popolati durante l'esecuzione run_time (requst_id, time ecc..)
        // I Transfer ID sono relativi al singolo nodo, quindi vanno tenuti separati
        uint8_t  node_flag;                 // Stato del Nodo remoto (Flag interno stati/comandi nodi)
        uint32_t heartbeat_timeout;         // Time heartbeat Remoto x Verifica OffLine
        uint8_t  heartbeat_state;           // Vendor specific code NodeRemote State
        uint64_t service_transfer_id;       // Accesso ai dati
        uint64_t command_transfer_id;       // Comandi x Nodo relativo
        // **** Gestione delle risposte ai comandi / richieste ****
        // Puntatore alla struttura dati relativa es. -> rmap_module_TH_1_0 ecc...
        // Utilizzata per il collegamento con il modulo remoto specifico
        // Allocazione interna dinamica con C/C++ malloc, FreeRTOS pvPortMalloc();
        void*    service_module;            // Dati e stato di risposta ai dati nodo
        uint8_t  command_response;          // Stato di risposta ai comandi nodo
        uint32_t command_timeout;           // Time command Remoto x Verifica deadLine Request
        uint32_t service_timeout;           // Time getData Remoto x Verifica deadline Request
        #ifdef USE_SUB_PUBLISH_SLAVE_DATA
        uint16_t publisher_data_count;      // Conteggio pubblicazioni remote autonome (SOLO TEST)
        #endif
    } remote_node[MAX_NODE_CONNECT];

    // Abilitazione delle pubblicazioni falcoltative sulla rete (ON/OFF a richiesta)
    struct
    {
        bool port_list;
    } pub_enabled;

    /// A transfer-ID is an integer that is incremented whenever a new message is published on a given subject.
    /// It is used by the protocol for deduplication, message loss detection, and other critical things.
    /// For CAN, each value can be of type uint8_t, but we use larger types for genericity and for statistical purposes,
    /// as large values naturally contain the number of times each subject was published to.
    struct
    {
        uint64_t uavcan_node_heartbeat;
        uint64_t uavcan_node_port_list;
    } next_transfer_id;
} State;

// Flag per Reboot del nodo.
static volatile bool g_restart_required = false;

// ***************************************************************************************************
// **********             Funzioni ed utility generiche per gestione UAVCAN                 **********
// ***************************************************************************************************

// Ritorna l'indice della coda master allocata in state in funzione del nodeId fisico
byte getQueueNodeFromId(State* const state, CanardNodeID nodeId) {
    // Cerco la corrispondenza node_id nella coda allocata master per ritorno queueID Index
    for(byte queueId=0; queueId<MAX_NODE_CONNECT; queueId++) {
        // Se trovo il nodo che sta rispondeno nella coda degli allocati...
        if(state->remote_node[queueId].node_id == nodeId) {
            return queueId;
        } 
    }
    return GENERIC_BVAL_UNDEFINED;
}

/// A deeply embedded system should sample a microsecond-resolution non-overflowing 64-bit timer.
/// Here is a simple non-blocking implementation as an example:
/// https://github.com/PX4/sapog/blob/601f4580b71c3c4da65cc52237e62a/firmware/src/motor/realtime/motor_timer.c#L233-L274
/// Mind the difference between monotonic time and wall time. Monotonic time never changes rate or makes leaps,
/// it is therefore impossible to synchronize with an external reference. Wall time can be synchronized and therefore
/// it may change rate or make leap adjustments. The two kinds of time serve completely different purposes.
// TODO: Microsecond register STM32... Microsecond non va bene perchè si resetta mentre
// Per Cypal non dovrebbe resettarsi uint_64 Bit
static CanardMicrosecond getMonotonicMicroseconds()
{
    time_t ts;
    ts=(uint64_t) millis() * KILO;
    return (uint64_t) ts;
}

// Ritorna unique-ID 128-bit del nodo locale. E' utilizzato in uavcan.node.GetInfo.Response e durante
// plug-and-play node-ID allocation da uavcan.pnp.NodeIDAllocationData. SerialNumber, Produttore..
// Dovrebbe essere verificato in uavcan.node.GetInfo.Response per la verifica non sia cambiato Nodo.
// Al momento vengono inseriti 2 BYTE fissi, altri eventuali, che Identificano il Tipo Modulo
static void getUniqueID(uint8_t out[uavcan_node_GetInfo_Response_1_0_unique_id_ARRAY_CAPACITY_])
{
    // A real hardware node would read its unique-ID from some hardware-specific source (typically stored in ROM).
    // This example is a software-only node so we store the unique-ID in a (read-only) register instead.
    uavcan_register_Value_1_0 value = {0};
    uavcan_register_Value_1_0_select_unstructured_(&value);
    // Crea default unique_id con NODE_TYPE_MAJOR (Tipo di nodo), MINOR (Hw relativo)
    // Il resto dei 128 Bit (112) vengono impostati RANDOM (potrebbero portare Manufactor, SerialNumber ecc...)
    // Dovrebbe essere l'ID per la verifica incrociata del corretto Node_Id dopo il PnP
    value.unstructured.value.elements[value.unstructured.value.count++] = (uint8_t) NODE_TYPE_MAJOR;
    value.unstructured.value.elements[value.unstructured.value.count++] = (uint8_t) NODE_TYPE_MINOR;
    for (uint8_t i = value.unstructured.value.count; i < uavcan_node_GetInfo_Response_1_0_unique_id_ARRAY_CAPACITY_; i++)
    {
        value.unstructured.value.elements[value.unstructured.value.count++] = (uint8_t) rand();  // NOLINT
    }
    registerRead("uavcan.node.unique_id", &value);
    assert(uavcan_register_Value_1_0_is_unstructured_(&value) &&
           value.unstructured.value.count == uavcan_node_GetInfo_Response_1_0_unique_id_ARRAY_CAPACITY_);
    memcpy(&out[0], &value.unstructured.value, uavcan_node_GetInfo_Response_1_0_unique_id_ARRAY_CAPACITY_);
}

// ***********************************************************************************************
// ***********************************************************************************************
//      FUNZIONI CHIAMATE DA MAIN_LOOP DI PUBBLICAZIONE E RICHIESTE DATI E SERVIZI
// ***********************************************************************************************
// ***********************************************************************************************

// *******        Funzioni ed utility di ritrasmissione dati sulla rete UAVCAN           *********

// Wrapper per send e sendresponse con Canard 
static void send(State* const                        state,
                 const CanardMicrosecond             tx_deadline_usec,
                 const CanardTransferMetadata* const metadata,
                 const size_t                        payload_size,
                 const void* const                   payload)
{
    for (uint8_t ifidx = 0; ifidx < CAN_REDUNDANCY_FACTOR; ifidx++)
    {
        (void) canardTxPush(&state->canard_tx_queues[ifidx],
                            &state->canard,
                            tx_deadline_usec,
                            metadata,
                            payload_size,
                            payload);
    }
}
// Risposte con inversione meta.transfer_kind alle Request
static void sendResponse(State* const                        state,
                         const CanardMicrosecond             tx_deadline_usec,
                         const CanardTransferMetadata* const request_metadata,
                         const size_t                        payload_size,
                         const void* const                   payload)
{
    CanardTransferMetadata meta = *request_metadata;
    meta.transfer_kind          = CanardTransferKindResponse;
    send(state, tx_deadline_usec, &meta, payload_size, payload);
}

// *******              FUNZIONI INVOCATE HANDLE FAST_LOOP EV. PREPARATORIE              *********

static void handleFastLoop(State* const state, const CanardMicrosecond monotonic_time)
{
    // TODO: FILE ??????
}

// *******              FUNZIONI INVOCATE HANDLE 1 SECONDO EV. PREPARATORIE              *********

static void handle1HzLoop(State* const state, const CanardMicrosecond monotonic_time) {
    
    // ***** Trasmette alla rete UAVCAN lo stato haeartbeat del modulo *****
    // Heartbeat Fisso anche per modulo Master (Visibile a yakut o altri tools/script gestionali)
    uavcan_node_Heartbeat_1_0 heartbeat = {0};
    heartbeat.uptime                    = (uint32_t) ((monotonic_time - state->started_at) / MEGA);
    heartbeat.mode.value                = uavcan_node_Mode_1_0_OPERATIONAL;
    const O1HeapDiagnostics heap_diag   = o1heapGetDiagnostics(state->heap);
    if (heap_diag.oom_count > 0) {
        heartbeat.health.value = uavcan_node_Health_1_0_CAUTION;
    } else {
        heartbeat.health.value = uavcan_node_Health_1_0_NOMINAL;
    }

    uint8_t      serialized[uavcan_node_Heartbeat_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
    size_t       serialized_size                                                        = sizeof(serialized);
    const int8_t err = uavcan_node_Heartbeat_1_0_serialize_(&heartbeat, &serialized[0], &serialized_size);
    assert(err >= 0);
    if (err >= 0)
    {
        const CanardTransferMetadata meta = {
            .priority       = CanardPriorityNominal,
            .transfer_kind  = CanardTransferKindMessage,
            .port_id        = uavcan_node_Heartbeat_1_0_FIXED_PORT_ID_,
            .remote_node_id = CANARD_NODE_ID_UNSET,
            .transfer_id    = (CanardTransferID) (state->next_transfer_id.uavcan_node_heartbeat++),
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
            assert(obj->sparse_list.count < uavcan_node_port_SubjectIDList_0_1_sparse_list_ARRAY_CAPACITY_);
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

// **************           Pubblicazione vera e propria a 10 secondi           **************
static void handle01HzLoop(State* const state, const CanardMicrosecond monotonic_time)
{
    // Publish the recommended (not required) port introspection message. No point publishing it if we're anonymous.
    // The message is a bit heavy on the stack (about 2 KiB) but this is not a problem for a modern MCU.
    // L'abilitazione del comando è facoltativa, può essere attivata/disattivata da un comando UAVCAN
    if ((state->pub_enabled.port_list) &&
        (state->canard.node_id <= CANARD_NODE_ID_MAX))
    {
        uavcan_node_port_List_0_1 m = {0};
        uavcan_node_port_List_0_1_initialize_(&m);
        uavcan_node_port_SubjectIDList_0_1_select_sparse_list_(&m.publishers);
        uavcan_node_port_SubjectIDList_0_1_select_sparse_list_(&m.subscribers);

        // Indicate which subjects we publish to. Don't forget to keep this updated if you add new publications!
        {
            size_t* const cnt                                 = &m.publishers.sparse_list.count;
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
        // TODO: Verificare, per nulla convinto. Controllare RAM con tutti i servizi master/slave
        uint8_t serialized[512] = {0};  // https://github.com/OpenCyphal/nunavut/issues/191
        size_t  serialized_size = uavcan_node_port_List_0_1_SERIALIZATION_BUFFER_SIZE_BYTES_;
        if (uavcan_node_port_List_0_1_serialize_(&m, &serialized[0], &serialized_size) >= 0)
        {
            const CanardTransferMetadata meta = {
                .priority       = CanardPriorityOptional,  // Mind the priority.
                .transfer_kind  = CanardTransferKindMessage,
                .port_id        = uavcan_node_port_List_0_1_FIXED_PORT_ID_,
                .remote_node_id = CANARD_NODE_ID_UNSET,
                .transfer_id    = (CanardTransferID) (state->next_transfer_id.uavcan_node_port_list++),
            };
            // Send a 2 secondi
            send(state, monotonic_time + MEGA * 2, &meta, serialized_size, &serialized[0]);
        }
    }
}

// ************** SEZIONE COMANDI E RICHIESTE SPECIFICHE AD UN NODO SULLA RETE  **************

// **************       Invio Comando diretto ad un nodo remoto UAVCAN Cmd      **************
static bool serviceSendCommand(State* const state, const CanardMicrosecond monotonic_time,
                                byte istanza, const uint16_t cmd_request, const void* ext_param, size_t ext_lenght)
{
    // Effettua una richiesta specifica ad un nodo della rete in formato UAVCAN
    uavcan_node_ExecuteCommand_Request_1_1 cmdRequest = {0};
    uint8_t      serialized[uavcan_node_ExecuteCommand_Request_1_1_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
    size_t       serialized_size = sizeof(serialized);

    // istanza -> queueId di State o istanza di nodo

    // Imposta il comando da inviare
    cmdRequest.command = cmd_request;
    // Verifica la presenza di parametri opzionali nel comando
    cmdRequest.parameter.count = ext_lenght;
    // Controllo conformità lunghezza messaggio e ne copio il contenuto
    assert(cmdRequest.parameter.count<=uavcan_node_ExecuteCommand_Request_1_1_parameter_ARRAY_CAPACITY_);
    if (ext_lenght) {
        memcpy(cmdRequest.parameter.elements, ext_param, ext_lenght);
    }

    // Serializzo e verifico la conformità del messaggio
    const int8_t err = uavcan_node_ExecuteCommand_Request_1_1_serialize_(&cmdRequest, &serialized[0], &serialized_size);
    assert(err >= 0);
    if (err >= 0)
    {
        // Comando a priorità Immediata
        const CanardTransferMetadata meta = {
            .priority       = CanardPriorityHigh,
            .transfer_kind  = CanardTransferKindRequest,
            .port_id        = uavcan_node_ExecuteCommand_1_1_FIXED_PORT_ID_,
            .remote_node_id = state->remote_node[istanza].node_id,
            .transfer_id    = (CanardTransferID) (state->remote_node[istanza].command_transfer_id++),
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
                                    byte istanza, byte comando, uint16_t run_sectime)
{
    // Effettua una richiesta specifica ad un nodo della rete in formato UAVCAN
    // La richiesta è generica per tutti i moduli (univoca DSDL), comunque parte integrante di ogni
    // DSDL singola di modulo. Il PORT_ID fisso o dinamico indica il nodo remoto.
    // L'interpretazione è invece tipicizzata dalla risposta (DSDL specifica)
    rmap_service_setmode_1_0 cmdRequest = {0};
    uint8_t      serialized[rmap_service_module_TH_Request_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
    size_t       serialized_size = sizeof(serialized);

    // Imposta il comando da inviare ed il timer base
    cmdRequest.comando = comando;
    cmdRequest.run_sectime = run_sectime;

    // Serializzo e verifico la conformità del messaggio
    const int8_t err = rmap_service_setmode_1_0_serialize_(&cmdRequest, &serialized[0], &serialized_size);
    assert(err >= 0);
    if (err >= 0)
    {
        // Comando a priorità Immediata
        const CanardTransferMetadata meta = {
            .priority       = CanardPriorityHigh,
            .transfer_kind  = CanardTransferKindRequest,
            .port_id        = state->remote_node[istanza].port_id,
            .remote_node_id = state->remote_node[istanza].node_id,
            .transfer_id    = (CanardTransferID) (state->remote_node[istanza].command_transfer_id++),
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
static uavcan_node_ExecuteCommand_Response_1_1 processRequestExecuteCommand(State* const state,
                                                                            const uavcan_node_ExecuteCommand_Request_1_1* req)
{
    uavcan_node_ExecuteCommand_Response_1_1 resp = {0};
    // req->command (Comando esterno ricevuto 2 BYTES RESERVED FFFF-FFFA)
    // Gli altri sono liberi per utilizzo interno applicativo con #define interne
    // req->parameter (array di byte MAX 255 per i parametri da request)
    // Risposta attuale (resp) 1 Bytes RESERVER (0..6) gli altri #define interne
    switch (req->command)
    {
        case uavcan_node_ExecuteCommand_Request_1_1_COMMAND_BEGIN_SOFTWARE_UPDATE:
        {
            char file_name[uavcan_node_ExecuteCommand_Request_1_1_parameter_ARRAY_CAPACITY_ + 1] = {0};
            memcpy(file_name, req->parameter.elements, req->parameter.count);
            file_name[req->parameter.count] = '\0';
            // TODO: invoke the bootloader with the specified file name. See https://github.com/Zubax/kocherga/
            printf("Firmware update request; filename: '%s' \n", &file_name[0]);
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_BAD_STATE;  // This is a stub.
            break;
        }
        case uavcan_node_ExecuteCommand_Request_1_1_COMMAND_FACTORY_RESET:
        {
            registerDoFactoryReset();
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        case uavcan_node_ExecuteCommand_Request_1_1_COMMAND_RESTART:
        {
            g_restart_required = true;
            resp.status        = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        case uavcan_node_ExecuteCommand_Request_1_1_COMMAND_STORE_PERSISTENT_STATES:
        {
            // If your registers are not automatically synchronized with the non-volatile storage, use this command
            // to commit them to the storage explicitly. Otherwise it is safe to remove it.
            // In this demo, the registers are stored in files, so there is nothing to do.
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        case CMD_ENABLE_PUBLISH_PORT_LIST:
        {
            // Abilita pubblicazione slow_loop elenco porte (Cypal facoltativo)
            state->pub_enabled.port_list = true;
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        case CMD_DISABLE_PUBLISH_PORT_LIST:
        {
            // Disabilita pubblicazione slow_loop elenco porte (Cypal facoltativo)
            state->pub_enabled.port_list = false;
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }        
        default:
        {
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_BAD_COMMAND;
            break;
        }
    }
    return resp;
}

// Accesso ai registri UAVCAN risposta a richieste
static uavcan_register_Access_Response_1_0 processRequestRegisterAccess(const uavcan_register_Access_Request_1_0* req)
{
    char name[uavcan_register_Name_1_0_name_ARRAY_CAPACITY_ + 1] = {0};
    assert(req->name.name.count < sizeof(name));
    memcpy(&name[0], req->name.name.elements, req->name.name.count);
    name[req->name.name.count] = '\0';

    uavcan_register_Access_Response_1_0 resp = {0};

    // If we're asked to write a new value, do it now:
    if (!uavcan_register_Value_1_0_is_empty_(&req->value))
    {
        uavcan_register_Value_1_0_select_empty_(&resp.value);
        registerRead(&name[0], &resp.value);
        // If such register exists and it can be assigned from the request value:
        if (!uavcan_register_Value_1_0_is_empty_(&resp.value) && registerAssign(&resp.value, &req->value))
        {
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
    resp._mutable   = true;
    resp.persistent = true;

    // Our node does not synchronize its time with the network so we can't populate the timestamp.
    resp.timestamp.microsecond = uavcan_time_SynchronizedTimestamp_1_0_UNKNOWN;

    return resp;
}

// Risposta a uavcan.node.GetInfo which Info Node (nome, versione, uniqueID di verifica ecc...)
static uavcan_node_GetInfo_Response_1_0 processRequestNodeGetInfo()
{
    uavcan_node_GetInfo_Response_1_0 resp = {0};
    resp.protocol_version.major           = CANARD_CYPHAL_SPECIFICATION_VERSION_MAJOR;
    resp.protocol_version.minor           = CANARD_CYPHAL_SPECIFICATION_VERSION_MINOR;

    // The hardware version is not populated in this demo because it runs on no specific hardware.
    // An embedded node would usually determine the version by querying the hardware.

    resp.software_version.major   = VERSION_MAJOR;
    resp.software_version.minor   = VERSION_MINOR;
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
static void processReceivedTransfer(State* const state, const CanardRxTransfer* const transfer)
{
    // Gestione dei Messaggi in ingresso
    if (transfer->metadata.transfer_kind == CanardTransferKindMessage)
    {
        // bool Per assert mancanza handler di eventuale servizio sottoscritto
        bool bKindMessageProcessed = false;
        // Gestione dei messaggi Heartbeat per stato rete (gestisco come master)
        if (transfer->metadata.port_id == uavcan_node_Heartbeat_1_0_FIXED_PORT_ID_)
        {
            bKindMessageProcessed = true;
            size_t size = transfer->payload_size;
            uavcan_node_Heartbeat_1_0 msg = {0};
            if (uavcan_node_Heartbeat_1_0_deserialize_(&msg, static_cast<uint8_t const*>(transfer->payload), &size) >= 0)
            {
                Serial.print(F("RX HeartBeat from -> "));
                Serial.println(transfer->metadata.remote_node_id);
                // Processo e registro il nodo: stato, OnLine e relativi flag
                byte queueId = getQueueNodeFromId(state, transfer->metadata.remote_node_id);
                // Se nodo correttamente allocato e gestito (potrebbe esser Yakut non registrato)
                if (queueId != GENERIC_BVAL_UNDEFINED) {
                    // Accodo i dati letti dal messaggio
                    SetNodeOnline(state->remote_node[queueId].node_flag);
                    SetNodeHealtState(state->remote_node[queueId].node_flag, msg.health.value);
                    state->remote_node[queueId].heartbeat_state = msg.vendor_specific_status_code;
                    // Set internal local millis per controllo NodoOffline
                    state->remote_node[queueId].heartbeat_timeout = millis();
                }
            }
        }
        #ifdef USE_SUB_PUBLISH_SLAVE_DATA
        else
        {
            // Gestione messaggi pubblicazione dati dei moduli slave (gestisco come master)
            // Es. popalamento dati se attivato un log specifico o show valori su display
            // Il comando è opzionale perchè in request/response esiste già questa possibilità
            // Nodo rispondente leggo dalla coda la/le pubblicazioni attivate (MAX 1 x tipologia)
            byte queueId = getQueueNodeFromId(state, transfer->metadata.remote_node_id);
            // Se nodo correttammente allocato e gestito
            if (queueId != GENERIC_BVAL_UNDEFINED) {
                // Verifico se risposta del servizio corrisponde al chiamante (eventuali + servizi sotto...)
                // Gestione di tutti i servizi possibili allocabili, valido per tutti i nodi
                if (transfer->metadata.port_id == state->remote_node[queueId].subject_id)
                {                
                    // *************            Service Modulo TH Response            *************
                    if(state->remote_node[queueId].node_type == MODULE_TYPE_TH) {
                        // Processato il messaggio con il relativo Handler. OK
                        bKindMessageProcessed = true;
                        // Modulo TH, leggo e deserializzo il messaggio in ingresso
                        rmap_module_TH_1_0  msg = {0};
                        size_t             size = transfer->payload_size;
                        if (rmap_module_TH_1_0_deserialize_(&msg, static_cast<uint8_t const*>(transfer->payload), &size) >= 0)
                        {
                            // TODO: vedere con Marco Pubblica, registra elimina, display... altro
                            // Per ora salvo solo il dato ricevuto dalla struttura di state msg (count)
                            // msg contiene i dati di blocco pubblicati
                            state->remote_node[queueId].publisher_data_count++;
                        }
                    }
                    // ALTRI MODULI DA INSERIRE QUA... PG, VV, RS, GAS ECC...
                }
            }
        }
        #endif
        // Mancanza di handler per un servizio sottoscritto
        if (!bKindMessageProcessed)
        {
            // Gestione di un messaggio sottoscritto senza gestione. Se arrivo quà è un errore di sviluppo
            assert(false);
        }
    }
    // Gestione delle richieste esterne
    else if (transfer->metadata.transfer_kind == CanardTransferKindRequest)
    {
        if (transfer->metadata.port_id == uavcan_node_GetInfo_1_0_FIXED_PORT_ID_)
        {
            // The request object is empty so we don't bother deserializing it. Just send the response.
            const uavcan_node_GetInfo_Response_1_0 resp = processRequestNodeGetInfo();
            uint8_t      serialized[uavcan_node_GetInfo_Response_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
            size_t       serialized_size = sizeof(serialized);
            const int8_t res = uavcan_node_GetInfo_Response_1_0_serialize_(&resp, &serialized[0], &serialized_size);
            if (res >= 0)
            {
                sendResponse(state,
                             transfer->timestamp_usec + MEGA,
                             &transfer->metadata,
                             serialized_size,
                             &serialized[0]);
            }
        }
        else if (transfer->metadata.port_id == uavcan_register_Access_1_0_FIXED_PORT_ID_)
        {
            uavcan_register_Access_Request_1_0 req  = {0};
            size_t                             size = transfer->payload_size;
            if (uavcan_register_Access_Request_1_0_deserialize_(&req, static_cast<uint8_t const*>(transfer->payload), &size) >= 0)
            {
                const uavcan_register_Access_Response_1_0 resp = processRequestRegisterAccess(&req);
                uint8_t serialized[uavcan_register_Access_Response_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
                size_t  serialized_size = sizeof(serialized);
                if (uavcan_register_Access_Response_1_0_serialize_(&resp, &serialized[0], &serialized_size) >= 0)
                {
                    sendResponse(state,
                                 transfer->timestamp_usec + MEGA,
                                 &transfer->metadata,
                                 serialized_size,
                                 &serialized[0]);
                }
            }
        }
        else if (transfer->metadata.port_id == uavcan_register_List_1_0_FIXED_PORT_ID_)
        {
            uavcan_register_List_Request_1_0 req  = {0};
            size_t                           size = transfer->payload_size;
            if (uavcan_register_List_Request_1_0_deserialize_(&req, static_cast<uint8_t const*>(transfer->payload), &size) >= 0)
            {
                const uavcan_register_List_Response_1_0 resp = {.name = registerGetNameByIndex(req.index)};
                uint8_t serialized[uavcan_register_List_Response_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
                size_t  serialized_size = sizeof(serialized);
                if (uavcan_register_List_Response_1_0_serialize_(&resp, &serialized[0], &serialized_size) >= 0)
                {
                    sendResponse(state,
                                 transfer->timestamp_usec + MEGA,
                                 &transfer->metadata,
                                 serialized_size,
                                 &serialized[0]);
                }
            }
        }
        else if (transfer->metadata.port_id == uavcan_node_ExecuteCommand_1_1_FIXED_PORT_ID_)
        {
            uavcan_node_ExecuteCommand_Request_1_1 req  = {0};
            size_t                                 size = transfer->payload_size;
            if (uavcan_node_ExecuteCommand_Request_1_1_deserialize_(&req, static_cast<uint8_t const*>(transfer->payload), &size) >= 0)
            {
                const uavcan_node_ExecuteCommand_Response_1_1 resp = processRequestExecuteCommand(state, &req);
                uint8_t serialized[uavcan_node_ExecuteCommand_Response_1_1_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
                size_t  serialized_size = sizeof(serialized);
                if (uavcan_node_ExecuteCommand_Response_1_1_serialize_(&resp, &serialized[0], &serialized_size) >= 0)
                {
                    sendResponse(state,
                                 transfer->timestamp_usec + MEGA,
                                 &transfer->metadata,
                                 serialized_size,
                                 &serialized[0]);
                }
            }
        }
        else
        {
            // Gestione di una richiesta senza controllore locale. Se arrivo quà è un errore di sviluppo
            assert(false);
        }
    }
    // Gestione delle risposte alle richeste inviate alla rete come Master
    else if (transfer->metadata.transfer_kind == CanardTransferKindResponse)
    {
        // Comando inviato ad un nodo remoto, verifica della risposta e della coerenza messaggio
        if (transfer->metadata.port_id == uavcan_node_ExecuteCommand_1_1_FIXED_PORT_ID_)
        {
            uavcan_node_ExecuteCommand_Response_1_1 resp = {0};
            size_t                                  size = transfer->payload_size;
            if (uavcan_node_ExecuteCommand_Response_1_1_deserialize_(&resp, static_cast<uint8_t const*>(transfer->payload), &size) >= 0)
            {
                // Ricerco idNodo nella coda degli allocati del master
                // Copio la risposta ricevuta nella struttura relativa e resetto il flag pending                
                byte queueId = getQueueNodeFromId(state, transfer->metadata.remote_node_id);
                if (queueId != GENERIC_BVAL_UNDEFINED) {
                    // Resetta il pending del comando del nodo verificato
                    ResetNodePendingCmd(state->remote_node[queueId].node_flag);
                    // Copia la risposta nella variabile di chiamata in state
                    state->remote_node[queueId].command_response = resp.status;
                }
            }
        }
        // else if -> servizi opzionali di risposta vanno inseriti qui
        // Risposta ad un servizio (dati) dinamicamente allocato... ( deve essere ultimo else )
        // Servizio di risposta alla richiesta su modulo slave, verifica della risposta e della coerenza messaggio
        // Per il nodo che risponde verifico i servizi attivi per la corrispondenza dinamica risposta
        else
        {
            // Nodo rispondente (posso avere senza problemi più servizi con stesso port_id su diversi nodi)
            byte queueId = getQueueNodeFromId(state, transfer->metadata.remote_node_id);
            // Se nodo correttammente allocato e gestito
            if (queueId != GENERIC_BVAL_UNDEFINED) {
                // Verifico se risposta del servizio corrisponde al chiamante (eventuali + servizi sotto...)
                // Gestione di tutti i servizi possibili allocabili, valido per tutti i nodi
                if (transfer->metadata.port_id == state->remote_node[queueId].port_id)
                {                
                    // *************            Service Modulo TH Response            *************
                    if(state->remote_node[queueId].node_type == MODULE_TYPE_TH) {
                        // Modulo TH, leggo e deserializzo il messaggio in ingresso
                        rmap_service_module_TH_Response_1_0 resp = {0};
                        size_t                              size = transfer->payload_size;
                        if (rmap_service_module_TH_Response_1_0_deserialize_(&resp, static_cast<uint8_t const*>(transfer->payload), &size) >= 0)
                        {
                            // Resetta il pending del comando del nodo verificato
                            ResetNodePendingData(state->remote_node[queueId].node_flag);
                            // Copia la risposta nella variabile di chiamata in state
                            // Oppure gestire qua tutte le altre occorrenze per stima V4
                            // TODO: vedere con Marco Pubblica, registra elimina, display... altro
                            // Per ora copio in una struttura di state response
                            memcpy(state->remote_node[queueId].service_module, &resp, sizeof(resp));
                        }
                    }
                    // ALTRI MODULI DA INSERIRE QUA... PG, VV, RS, GAS ECC...
                }
            }
        }
    }
    else
    {
        // Se arrivo quà è un errore di sviluppo, controllare setup sottoscrizioni, risposte e messaggi
        assert(false);
    }
}

// *********************************************************************************************
//          Inizializzazione generale HW, canard, CAN_BUS, e dispositivi collegati
// *********************************************************************************************

// Setup SW - Canard memory access (allocate/free)
static void* canardAllocate(CanardInstance* const ins, const size_t amount)
{
    O1HeapInstance* const heap = ((State*) ins->user_reference)->heap;
    assert(o1heapDoInvariantsHold(heap));
    return o1heapAllocate(heap, amount);
}

static void canardFree(CanardInstance* const ins, void* const pointer)
{
    O1HeapInstance* const heap = ((State*) ins->user_reference)->heap;
    o1heapFree(heap, pointer);
}

// Setup HW (PIN, interface, filter, baud)
bool CAN_HW_Init(void)
{
    // Definition CAN structure variable
    CAN_HandleTypeDef CAN_Handle;

    // Definition GPIO and CAN filter structure variables
    GPIO_InitTypeDef GPIO_InitStruct;
    CAN_FilterTypeDef CAN_FilterInitStruct;

    // GPIO Ports clock enable
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // CAN1 clock enable
    __HAL_RCC_CAN1_CLK_ENABLE();

    #if defined(STM32L452xx)
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
        Serial.println(F("Error initialization HW CAN base"));
        assert(false);
        return false;
    }

    // CAN filter basic initialization
    CAN_FilterInitStruct.FilterIdHigh = 0x0000;
    CAN_FilterInitStruct.FilterIdLow = 0x0000;
    CAN_FilterInitStruct.FilterMaskIdHigh = 0x0000;
    CAN_FilterInitStruct.FilterMaskIdLow = 0x0000;
    CAN_FilterInitStruct.FilterFIFOAssignment = 0;
    CAN_FilterInitStruct.FilterBank = 0;
    CAN_FilterInitStruct.FilterMode = CAN_FILTERMODE_IDMASK;
    CAN_FilterInitStruct.FilterScale = CAN_FILTERSCALE_32BIT;
    CAN_FilterInitStruct.FilterActivation = ENABLE;
    // Check error initalization CAN filter
    if (HAL_CAN_ConfigFilter(&CAN_Handle, &CAN_FilterInitStruct) != HAL_OK) {
        Serial.println(F("Error initialization filter CAN base"));
        assert(false);
        return false;
    }

    // SetUp Speed to CAN_BIT_RATE (defined)
    BxCANTimings timings;
    bool result = bxCANComputeTimings(HAL_RCC_GetPCLK1Freq(), CAN_BIT_RATE, &timings);
    if (!result) {
        Serial.println(F("Error initialization bxCANComputeTimings"));
        assert(false);
        return false;
    }

    // Attivazione bxCAN sulle interfacce richieste, velocità e modalità
    result = bxCANConfigure(0, timings, false);
    if (!result) {
        Serial.println(F("Error initialization bxCANConfigure"));
        assert(false);
        return false;
    }

    // Check error starting CAN
    if (HAL_CAN_Start(&CAN_Handle) != HAL_OK) {
        Serial.println(F("CAN startup ERROR!!!"));
        assert(false);
        return false;
    }

    // Setup Completato
    return true;
}

// *********************************************************************************************
//                                       SETUP AMBIENTE
// *********************************************************************************************
void setup(void) {

    // *****************************************************
    //            STARTUP SERIAL-COMMUNICATION 
    // *****************************************************
    Serial.begin(115200);
    // Wait for serial port to connect
    while (!Serial) {
    }
    Serial.println(F("Start RS232 Monitor"));

    // *****************************************************
    //            STARTUP LED E PIN DIAGNOSTICI
    // *****************************************************
    // Output mode for LED BLINK SW LOOP
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    // *****************************************************
    //      STARTUP LIBRERIA SD/MEM REGISTER COLLEGATA
    // *****************************************************
    if (!setupSd(PIN_SPI_MOSI, PIN_SPI_MISO, PIN_SPI_SCK, PIN_SPI_SS, 18)) {
        Serial.println(F("Initialization SD card error"));
        assert(false);
    }
    Serial.println(F("Initialization SD card done"));

    // *****************************************************
    //                STARTUP CANBUS 
    // *****************************************************
    Serial.print(F("Initializing CANBUS..., PCLK1 Clock Freq: "));
    Serial.println(HAL_RCC_GetPCLK1Freq());
    if (!CAN_HW_Init()) {
        Serial.println(F("Initialization CAN BUS error"));
        assert(false);
    }
    Serial.println(F("Initialization CAN BUS done"));
}

// *************************************************************************************************
//                                          MAIN LOOP
// *************************************************************************************************
void loop(void)
{
    uavcan_register_Value_1_0 val = {0};
    State state = {0};

    // A simple node like this one typically does not require more than 8 KiB of heap and 4 KiB of stack.
    // For the background and related theory refer to the following resources:
    // - https://github.com/OpenCyphal/libcanard/blob/master/README.md
    // - https://github.com/pavel-kirienko/o1heap/blob/master/README.md
    // - https://forum.opencyphal.org/t/uavcanv1-libcanard-nunavut-templates-memory-usage-concerns/1118/4
    _Alignas(O1HEAP_ALIGNMENT) static uint8_t heap_arena[1024 * 16] = {0};
    state.heap = o1heapInit(heap_arena, sizeof(heap_arena));
    assert(NULL != state.heap);

    // The libcanard instance requires the allocator for managing protocol states.
    state.canard = canardInit(&canardAllocate, &canardFree);
    state.canard.user_reference = &state;  // Make the state reachable from the canard instance.

    // Default Setup servizi attivi nel modulo
    state.pub_enabled.port_list = DEFAULT_PUBLISH_PORT_LIST;

    // ********************************************************************************
    //                                   INIT VALUE
    // ********************************************************************************
    // Reset Remote node_id per ogni nodo collegato. Solo i nodi validi verranno gestiti
    for(uint8_t iCnt = 0; iCnt<MAX_NODE_CONNECT; iCnt++) {
        state.remote_node[iCnt].node_id = CANARD_NODE_ID_UNSET;
        state.remote_node[iCnt].node_flag = 0;
        state.remote_node[iCnt].heartbeat_timeout = 0;
        state.remote_node[iCnt].command_timeout = 0;
        state.remote_node[iCnt].service_timeout = 0;
        state.remote_node[iCnt].service_transfer_id = 0;
        state.remote_node[iCnt].command_transfer_id = 0;
        state.remote_node[iCnt].port_id = UINT16_MAX;
        state.remote_node[iCnt].service_module = NULL;
        #ifdef USE_SUB_PUBLISH_SLAVE_DATA
        state.remote_node[iCnt].subject_id = UINT16_MAX;
        state.remote_node[iCnt].publisher_data_count = 0;
        #endif
    }

    // Canard Master NODE ID Fixed dal defined value in module_config
    state.canard.node_id = (CanardNodeID) NODE_MASTER_ID;

    // ********************************************************************************
    //                   READING PARAM FROM E2 MEMORY / FLASH / SDCARD
    // ********************************************************************************

    // TODO:
    // Read Config Remote Node x Lettura porte e servizi. Non utilizzo registri ma CFG Generale
    // Vedere poi se meglio utilizzare i registri o anche solo per porzioni di configurazione
    // Fixed Value adesso !!!
    state.remote_node[0].node_id = 125;
    state.remote_node[0].node_type = MODULE_TYPE_TH;    
    state.remote_node[0].port_id = 100;
    #ifdef USE_SUB_PUBLISH_SLAVE_DATA
    // state.remote_node[0].node_subject_id = 5678;
    #endif
    state.remote_node[0].service_module = malloc(sizeof(rmap_service_module_TH_Response_1_0));
    
    // ********************************************************************************
    //    FIXED REGISTER_INIT, FARE INIT OPZIONALE x REGISTRI FISSI ECC. E/O INVAR.
    // ********************************************************************************
    #ifdef INIT_REGISTER
    // Opzionale Salvataggio veloce dei registri utili Stima V4
    #endif

    // Inizializza i registri nella modalità utilizzata (prepare SD/FLASH/ROM con default Value)
    registerInit();

    // *********               Lettura Registri standard UAVCAN               *********

    // The description register is optional but recommended because it helps constructing/maintaining large networks.
    // It simply keeps a human-readable description of the node that should be empty by default.
    uavcan_register_Value_1_0_select_string_(&val);
    val._string.value.count = 0;
    registerRead("uavcan.node.description", &val);  // We don't need the value, we just need to ensure it exists.

    // Configure il trasporto dal registro standard uavcan. Default a CANARD_MTU_MAX
    // Inserito per compatibilità, attualmente non gestita la modifica mtu_bytes (FISSA A MTU_CLASSIC)
    uavcan_register_Value_1_0_select_natural16_(&val);
    val.natural16.value.count       = 1;
    val.natural16.value.elements[0] = CANARD_MTU_MAX;
    registerRead("uavcan.can.mtu", &val);
    assert(uavcan_register_Value_1_0_is_natural16_(&val) && (val.natural16.value.count == 1));
    for (uint8_t ifidx = 0; ifidx < CAN_REDUNDANCY_FACTOR; ifidx++)
    {
        state.canard_tx_queues[ifidx] = canardTxInit(CAN_TX_QUEUE_CAPACITY, val.natural16.value.elements[0]);
    }

    // We also need the bitrate configuration register. In this demo we can't really use it but an embedded application
    // should define "uavcan.can.bitrate" of type natural32[2]; the second value is 0/ignored if CAN FD not supported.
    // TODO: Default a CAN_BIT_RATE, se CAN_BIT_RATE <> readRegister setup bxCAN con nuovo RATE hot reload
    uavcan_register_Value_1_0_select_natural32_(&val);
    val.natural32.value.count       = 2;
    val.natural32.value.elements[0] = CAN_BIT_RATE;
    val.natural32.value.elements[1] = 0ul;          // Ignored for CANARD_MTU_CAN_CLASSIC
    registerRead("uavcan.can.bitrate", &val);
    assert(uavcan_register_Value_1_0_is_natural32_(&val) && (val.natural32.value.count == 2));

    // TODO: (Controlla per MTU, CAN_FD non supportato, gestire solo il BitRate, mantenere per compatibilità futura)
    // Se BIT_RATE o MTU <> da valori standard, devo reinizializzare bxCanTimings e

    // ********************************************************************************
    //               AVVIA SOTTOSCRIZIONI ai messaggi per servizi RPC ecc...
    // ********************************************************************************
    
    // Service servers: -> Risposta per GetNodeInfo richiesta esterna (Yakut, Altri)
    {
        static CanardRxSubscription rx;
        const int8_t                res =  //
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
        const int8_t                res =  //
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
        const int8_t                res =  //
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
        const int8_t                res =  //
            canardRxSubscribe(&state.canard,
                                CanardTransferKindRequest,
                                uavcan_register_List_1_0_FIXED_PORT_ID_,
                                uavcan_register_List_Request_1_0_EXTENT_BYTES_,
                                CANARD_REGISTERLIST_TRANSFER_ID_TIMEOUT_USEC,
                                &rx);
        if (res < 0) NVIC_SystemReset();
    }

    // ******* SOTTOSCRIZIONE MESSAGGI / COMANDI E SERVIZI AD UTILITA' MASTER ********

    // Messaggi HEARTBEAT: -> Verifica della presenza per stato Nodi (Slave) OnLine / OffLine
    {
        static CanardRxSubscription rx;
        const int8_t                res =  //
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
        const int8_t                res =  //
            canardRxSubscribe(&state.canard,
                                CanardTransferKindResponse,
                                uavcan_node_ExecuteCommand_1_1_FIXED_PORT_ID_,
                                uavcan_node_ExecuteCommand_Response_1_1_EXTENT_BYTES_,
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
    for(byte queueId=0; queueId<MAX_NODE_CONNECT; queueId++) {
        // *************   SERVICE    *************
        // Se previsto il servizio request/response con port_id valido
        if ((state.remote_node[queueId].service_module) &&
            (state.remote_node[queueId].port_id <= CANARD_SERVICE_ID_MAX)) {
            // Controllo le varie tipologie di request/service per il nodo
            if(state.remote_node[queueId].node_type == MODULE_TYPE_TH) {            
                // Alloco la stottoscrizione in funzione del tipo di modulo
                // Service client: -> Risposta per ServiceDataModuleTH richiesta interna (come master)
                static CanardRxSubscription rx;
                const int8_t                res =  //
                    canardRxSubscribe(&state.canard,
                                        CanardTransferKindResponse,
                                        state.remote_node[queueId].port_id,
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
         if (state.remote_node[queueId].subject_id <= CANARD_SUBJECT_ID_MAX) {
            // Controllo le varie tipologie di request/service per il nodo
            if(state.remote_node[queueId].node_type == MODULE_TYPE_TH) {            
                // Alloco la stottoscrizione in funzione del tipo di modulo
                // Service client: -> Sottoscrizione per ModuleTH (come master)
                static CanardRxSubscription rx;
                const int8_t                res =  //
                    canardRxSubscribe(&state.canard,
                                        CanardTransferKindMessage,
                                        state.remote_node[queueId].subject_id,
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

    // TODO: Eliminare
    digitalWrite(LED_BUILTIN, LOW);
    bool ledShow;
    int  bTxAttempt = 0;
    int  bRxAttempt = 0;
    long checkTimeout = 0;
    long lastMillis;
    bool bEnabService = false;
    bool bLastPendingCmd = false;
    bool bIsPendingCmd = false;
    bool bLastPendingData = false;
    bool bIsPendingData = false;
    bool bIsResetFaultCmd = false;

    // Stampe locali TODO: utilizzare il metodo di Paolo
    // #define LOG_TX_RX_ATTEMPT
    #define PUBLISH_HEARTBEAT
    #define PUBLISH_PORTLIST
    #define TEST_COMMAND
    #define TEST_DATA

    // Set START Timetable LOOP RX/TX.
    state.started_at                            = getMonotonicMicroseconds();
    const CanardMicrosecond fast_loop_period    = MEGA / 3;
    CanardMicrosecond       next_333_ms_iter_at = state.started_at + fast_loop_period;
    CanardMicrosecond       next_01_sec_iter_at = state.started_at + MEGA;
    CanardMicrosecond       next_20_sec_iter_at = state.started_at + MEGA * 1.5;
    CanardMicrosecond       test_cmd_cs_iter_at = state.started_at + MEGA * 2.5;
    CanardMicrosecond       test_cmd_vs_iter_at = state.started_at + MEGA * 3.5;

    do 
    {
        // Run a trivial scheduler polling the loops that run the business logic.
        CanardMicrosecond monotonic_time = getMonotonicMicroseconds();
        // monotonic_time.

        // ************************************************************************
        // ***************   CHECK OFFLINE/DEADLINE COMMAND/STATE   ***************
        // ************************************************************************
        // TEST Check ogni 10 mSec circa ( SOLO TEST COMANDI DA INSERIRE IN TASK )
        if ((millis()-checkTimeout)>=10)
        {
            // Deadline di controllo
            checkTimeout = millis();
            // Per la coda/istanze allocate valide
            for (byte queueId = 0; queueId<MAX_NODE_CONNECT; queueId++) {
                if (state.remote_node[queueId].node_id <= CANARD_NODE_ID_MAX) {
                    // Solo se nodo OnLine (Automatic OnLine su HeartBeat RX)
                    if(IsNodeOnline(state.remote_node[queueId].node_flag)) {
                        // Controllo TimeOUT Comando diretto su modulo remoto
                        // Mancata risposta, nodo in Errore o Full o CanardHeapError ecc...
                        if(IsNodePendingCmd(state.remote_node[queueId].node_flag)) {
                            if((checkTimeout - state.remote_node[queueId].command_timeout) > NODE_COMMAND_TIMEOUT_MS) {
                                // Setto il flag di TimeOUT che il master dovrà gestire (segnalazione BUG al Server?)
                                SetNodeTimeOutCmd(state.remote_node[queueId].node_flag);
                            }
                        }
                        // Controllo TimeOut Comando getData su modulo remoto
                        // Mancata risposta, nodo in Errore o Full o CanardHeapError ecc...
                        if(IsNodePendingData(state.remote_node[queueId].node_flag)) {
                            if((checkTimeout - state.remote_node[queueId].service_timeout) > NODE_GETDATA_TIMEOUT_MS) {
                                // Setto il flag di TimeOUT che il master dovrà gestire (segnalazione BUG al Server?)
                                SetNodeTimeOutData(state.remote_node[queueId].node_flag);
                            }
                        }
                        // Check eventuale Nodo OFFLINE (Ultimo comando sempre perchè posso)
                        // Effettuare eventuali operazioni di SET,RESET Cmd in sicurezza
                        if ((checkTimeout - state.remote_node[queueId].heartbeat_timeout) > NODE_OFFLINE_TIMEOUT_MS) {
                            SetNodeOffline(state.remote_node[queueId].node_flag);
                            Serial.print(F("Nodo OFFLINE !!! Alert -> : "));
                            Serial.println(state.remote_node[queueId].node_id);
                            // Elimina gli eventuali stati pending e TimeOut
                            // Eventuali altre operazioni quà su Reset Comandi
                            ResetNodePendingCmd(state.remote_node[queueId].node_flag);
                            ResetNodePendingData(state.remote_node[queueId].node_flag);
                            ResetNodeTimeOutCmd(state.remote_node[queueId].node_flag);
                            ResetNodeTimeOutData(state.remote_node[queueId].node_flag);
                            // Attività x Reset e/o riavvio Nodo dopo OnLine -> Offline
                            // Solo TEST Locale TODO: da eliminare
                            bIsResetFaultCmd = true;
                        }
                    }
                }
            }
        }

        // **************************************************************************
        //                                CANARD UAVCAN
        // Scheduler temporizzato dei messaggi / comandi da inviare alla rete UAVCAN 
        // **************************************************************************

        // LOOP HANDLER >> FAST << 333 mSec
        if (monotonic_time >= next_333_ms_iter_at)
        {
            next_333_ms_iter_at += fast_loop_period;
            handleFastLoop(&state, monotonic_time);
            // Test Led Running 
            ledShow = !ledShow;
            if (ledShow)
                digitalWrite(LED_BUILTIN, HIGH);
            else
                digitalWrite(LED_BUILTIN, LOW);
        }

        // LOOP HANDLER >> 1 SECONDO <<
        if (monotonic_time >= next_01_sec_iter_at) {
            #ifdef PUBLISH_HEARTBEAT
            Serial.println(F("Publish MASTER Heartbeat -->> [1 sec]"));
            #endif
            next_01_sec_iter_at += MEGA;
            handle1HzLoop(&state, monotonic_time);
        }

        // LOOP HANDLER >> 20 SECONDI PUBLISH SERVIZI <<
        if (monotonic_time >= next_20_sec_iter_at) {
            next_20_sec_iter_at += MEGA * 20;
            #ifdef PUBLISH_PORTLIST
            Serial.println(F("Publish local PORT List -- [20 sec]"));
            #endif
            handle01HzLoop(&state, monotonic_time);
        }

        #ifdef TEST_COMMAND
        // ********************** TEST COMANDO TX-> RX<- *************************
        // LOOP HANDLER >> 0..15 SECONDI x TEST COMANDI <<
        if (monotonic_time >= test_cmd_cs_iter_at) {
            // TimeOUT variabile in 15 secondi
            test_cmd_cs_iter_at += MEGA * ((float)(rand() % 60)/4.0);
            // Invio un comando di test in prova al nodo 125
            // Possibile accesso da NodeId o direttamente dall'indice in coda conosciuto al master
            // Abilito disabilito pubblicazione dei dati ogni 3 secondi...
            byte queueId = getQueueNodeFromId(&state, 125);
            // Il comando viene inviato solamente se il nodo è ONLine
            if(IsNodeOnline(state.remote_node[queueId].node_flag)) {
                // Il comando viene inviato solamente senza altri Pending di Comandi
                // La verifica andrebbe fatta per singolo servizio, non necessario un blocco di tutto
                if(!IsNodePendingCmd(state.remote_node[queueId].node_flag)) {
                    // Imposta il pending del comando per verifica sequenza TX-RX e il TimeOut
                    // Imposta la risposta del comadno A UNDEFINED (verrà settato al valore corretto in risposta)
                    SetNodePendingCmd(state.remote_node[queueId].node_flag);
                    state.remote_node[queueId].command_timeout = millis();
                    state.remote_node[queueId].command_response = GENERIC_BVAL_UNDEFINED;
                    // Il comando inverte il servizio di pubblicazione continua dei dati remoti
                    // Semplice TEST di esempio (queueId = IDX di coda, diventerà l'istanza relativa)
                    bEnabService = !bEnabService;
                    if (bEnabService) {
                        serviceSendCommand(&state, monotonic_time, queueId,
                                            CMD_DISABLE_PUBLISH_DATA, NULL, 0);
                    } else {
                        serviceSendCommand(&state, monotonic_time, queueId,
                                            CMD_ENABLE_PUBLISH_DATA, NULL, 0);
                    }
                }
            }
            // La verifica verrà fatta con il Flag Pending resettato e la risposta viene
            // popolata nel'apposito registro di state service_module del il servizio relativo
            // Il set data avviene in processReciveTranser alle sezioni CanardTransferKindResponse
            // Eventuale Flag TimeOut indica un'avvenuta mancata risposta al comando
            // Il master una volta inviato il comando deve attendere ResetPending o TimeOutCommand
        }

        // Test rapido con nodo[0]... SOLO x OUT TEST DI VERIFICA TX/RX SEQUENZA
        // TODO: Eliminare, solo per verifica sequenza... Gestire da Master...
        if (IsNodeTimeOutCmd(state.remote_node[0].node_flag)) {
            // TimeOUT di un comando in attesa... gestisco il da farsi
            Serial.print(F("Timeout risposta su invio comando al nodo remoto: "));
            Serial.print(state.remote_node[0].node_id);
            Serial.println(F(", Warning [restore pending command]"));
            // Adesso elimino solo gli stati
            ResetNodePendingCmd(state.remote_node[0].node_flag);
            ResetNodeTimeOutCmd(state.remote_node[0].node_flag);
            // Adesso elimino solo gli stati per corretta visualizzazione a Serial.println
            bIsPendingCmd = false;
            bLastPendingCmd = false;
        } else {
            bIsPendingCmd = IsNodePendingCmd(state.remote_node[0].node_flag);
            // Gestione esempio di fault (Entrata in Offline... non faccio nulla solo reset var locale test)
            if(bIsResetFaultCmd) {
                bIsPendingCmd = false;
                bLastPendingCmd = false;
            }
            if(bLastPendingCmd != bIsPendingCmd) {
                bLastPendingCmd = bIsPendingCmd;
                if(bLastPendingCmd) {
                    Serial.print(F("Inviato comando al nodo remoto: "));
                    Serial.println(state.remote_node[0].node_id);
                } else {
                    Serial.print(F("Ricevuto conferma di comando dal nodo remoto: "));
                    Serial.print(state.remote_node[0].node_id);
                    Serial.print(F(", cod. risposta ->: "));
                    Serial.println(state.remote_node[0].command_response);
                }
            }
        }
        // ***************** FINE TEST COMANDO TX-> RX<- *************************
        #endif

        #ifdef TEST_DATA
        // ********************** TEST GETDATA TX-> RX<- *************************
        // LOOP HANDLER >> 0..15 SECONDI x TEST GETDATA <<
        if (monotonic_time >= test_cmd_vs_iter_at) {
            // TimeOUT variabile in 15 secondi
            test_cmd_vs_iter_at += MEGA * ((float)(rand() % 60)/4.0);   
            // Invio un comando di test in prova al nodo 125
            // Possibile accesso da NodeId o direttamente dall'indice in coda conosciuto al master
            // Abilito disabilito pubblicazione dei dati ogni 5 secondi...
            byte queueId = getQueueNodeFromId(&state, 125);
            // Il comando viene inviato solamente se il nodo è ONLine
            if(IsNodeOnline(state.remote_node[queueId].node_flag)) {
                // Il comando viene inviato solamente senza altri Pending di Comandi
                // La verifica andrebbe fatta per singolo servizio, non necessario un blocco di tutto
                if(!IsNodePendingData(state.remote_node[queueId].node_flag)) {
                    // Imposta il pending del comando per verifica sequenza TX-RX e il TimeOut
                    // La risposta al comando è già nel blocco dati, non necessaria ulteriore variabile
                    SetNodePendingData(state.remote_node[queueId].node_flag);
                    state.remote_node[queueId].service_timeout = millis();
                    // Il comando richiede il dato istantaneo ad un nodo remoto
                    // Semplice TEST di esempio con i parametri in richiesta
                    rmap_service_setmode_1_0 parametri;
                    parametri.comando = rmap_service_setmode_1_0_get_istant;
                    // parametri.run_for_second = 900; ( inutile in get_istant )
                    serviceSendRequestData(&state, monotonic_time, queueId,
                                            rmap_service_setmode_1_0_get_istant, 10);
                }
            }
            // La verifica verrà fatta con il Flag Pending resettato e la risposta viene
            // popolata nel'apposito registro di state service_module del il servizio relativo
            // Il set data avviene in processReciveTranser alle sezioni CanardTransferKindResponse
            // Eventuale Flag TimeOut indica un'avvenuta mancata risposta al comando
            // Il master una volta inviato il comando deve attendere ResetPending o TimeOutCommand
        }

        // Test rapido con nodo[0]... SOLO x OUT TEST DI VERIFICA TX/RX SEQUENZA
        // TODO: Eliminare, solo per verifica sequenza... Gestire da Master...
        if (IsNodeTimeOutData(state.remote_node[0].node_flag)) {
            // TimeOUT di un comando in attesa... gestisco il da farsi
            Serial.print(F("Timeout risposta su richiesta dati al nodo remoto: "));
            Serial.print(state.remote_node[0].node_id);
            Serial.println(F(", Warning [restore pending command]"));
            // Adesso elimino solo gli stati per corretta visualizzazione a Serial.println
            ResetNodePendingData(state.remote_node[0].node_flag);
            ResetNodeTimeOutData(state.remote_node[0].node_flag);
            bIsPendingData = false;
            bLastPendingData = false;
        } else {
            bIsPendingData = IsNodePendingData(state.remote_node[0].node_flag);
            // Gestione esempio di fault (Entrata in Offline... non faccio nulla solo reset var locale test)
            if(bIsResetFaultCmd) {
                bIsPendingData = false;
                bLastPendingData = false;
            }
            if(bLastPendingData != bIsPendingData) {
                bLastPendingData = bIsPendingData;
                if(bLastPendingData) {
                    Serial.print(F("Inviato richiesta dati al nodo remoto: "));
                    Serial.println(state.remote_node[0].node_id);
                } else {
                    // Interprete del messaggio in casting dal puntatore dinamico
                    // Nell'esempio Il modulo e TH, naturalmente bisogna gestire il tipo
                    // in funzione di state.remote_node[x].node_type
                    rmap_service_module_TH_Response_1_0* retData =
                        (rmap_service_module_TH_Response_1_0*) state.remote_node[0].service_module;
                    // Stampo la risposta corretta
                    Serial.print(F("Ricevuto risposta di richiesta dati dal nodo remoto: "));
                    Serial.print(state.remote_node[0].node_id);
                    Serial.print(F(", cod. risposta ->: "));
                    Serial.println(retData->stato);
                    // Test data Received e stampa valori con accesso al puntatore in casting per il modulo
                    Serial.println(F("Value L1, TP, UH: "));
                    Serial.print(retData->dataandmetadata.metadata.level.L1.value);
                    Serial.print(F(", "));
                    Serial.print(retData->dataandmetadata.temperature.val.value);
                    Serial.print(F(", "));
                    Serial.println(retData->dataandmetadata.humidity.val.value);
                }
            }
        }
        // ***************** FINE TEST GETDATA TX-> RX<- *************************
        #endif

        // RESET Gestione Fault di NODO TODO: Eliminare solo per verifica test
        bIsResetFaultCmd = false;

        // ***************************************************************************
        //   Gestione Coda messaggi in trasmissione (ciclo di svuotamento messaggi)
        // ***************************************************************************
        // Transmit pending frames from the prioritized TX queues managed by libcanard.
        for (uint8_t ifidx = 0; ifidx < CAN_REDUNDANCY_FACTOR; ifidx++)
        {
            CanardTxQueue* const     que = &state.canard_tx_queues[ifidx];
            const CanardTxQueueItem* tqi = canardTxPeek(que);  // Find the highest-priority frame.
            while (tqi != NULL)
            {
                // TODO: Remove test
                #ifdef LOG_TX_RX_ATTEMPT
                bTxAttempt++;
                if ((millis() - lastMillis) > 333) {
                    Serial.print("TX Attempt -> ");
                    Serial.println(bTxAttempt);
                    lastMillis = millis();
                }
                #endif
                // Attempt transmission only if the frame is not yet timed out while waiting in the TX queue.
                // Otherwise just drop it and move on to the next one.
                if ((tqi->tx_deadline_usec == 0) || (tqi->tx_deadline_usec > monotonic_time))
                {
                    // Non-blocking write attempt.
                    if (bxCANPush(0,
                        monotonic_time,
                        tqi->tx_deadline_usec,
                        tqi->frame.extended_can_id,
                        tqi->frame.payload_size,
                        tqi->frame.payload)) {
                        // Push CAN data
                        state.canard.memory_free(&state.canard, canardTxPop(que, tqi));
                        tqi = canardTxPeek(que);
                    } else  {
                        // Empty Queue
                        break;
                    }
                }
                else
                {
                    // loop continuo per mancato aggiornamento monotonic_time su TIME_OUT
                    // grandi quantità di dati trasmesse e raggiunto il TIMEOUT Subscription...
                    // TODO: Verificare non blocco come BUG trasmission security BUG!!!
                    // CANARD_REGISTERLIST_TRANSFER_ID_TIMEOUT_USEC = 500000 E PROVA
                    // Free CANARD data queue (packet TimeOUT...). Primi TEST = OK
                    state.canard.memory_free(&state.canard, canardTxPop(que, tqi));
                    // Test prossimo pacchetto
                    tqi = canardTxPeek(que);
                }
            }
        }

        // TODO: Interrupt RX...
        // ***************************************************************************
        //   Gestione Coda messaggi in ricezione (ciclo di caricamento messaggi)
        // ***************************************************************************
        for (uint8_t ifidx = 0; ifidx < CAN_REDUNDANCY_FACTOR; ifidx++)
        {
            CanardFrame frame;
            uint8_t     buf[CANARD_MTU_MAX] = {0};

            frame.extended_can_id = 0;
            frame.payload_size = sizeof(buf);
            // The read operation has timed out with no frames, nothing to do here.
            if (bxCANPop(ifidx, &frame.extended_can_id, &frame.payload_size, buf)) {
                // Get payload from Buffer
                frame.payload = buf;
                // TODO: Remove test
                #ifdef LOG_TX_RX_ATTEMPT
                bRxAttempt++;
                if ((millis() - lastMillis) > 333) {
                    Serial.print("RX Attempt <- ");
                    Serial.println(bRxAttempt);
                    lastMillis = millis();
                }
                #endif
                // The SocketCAN adapter uses the wall clock for timestamping, but we need monotonic.
                // Wall clock can only be used for time synchronization.
                const CanardMicrosecond timestamp_usec = getMonotonicMicroseconds();
                CanardRxTransfer        transfer; //      = {0};
                const int8_t canard_result = canardRxAccept(&state.canard, timestamp_usec, &frame, ifidx, &transfer, NULL);
                if (canard_result > 0)
                {
                    processReceivedTransfer(&state, &transfer);
                    state.canard.memory_free(&state.canard, (void*) transfer.payload);
                }
                else if ((canard_result == 0) || (canard_result == -CANARD_ERROR_OUT_OF_MEMORY))
                {
                    // TODO: Remove test
                    #ifdef LOG_TX_RX_ATTEMPT
                    if ((millis() - lastMillis) > 333) {
                        Serial.print("Rx Nothing ToDo...");
                        lastMillis = millis();
                    }
                    #endif
                    (void) 0;  // The frame did not complete a transfer so there is nothing to do.
                    // OOM should never occur if the heap is sized correctly. You can track OOM errors via heap API.
                }
                else
                {
                    assert(false);  // No other error can possibly occur at runtime.
                }
            }
            else break;
        }
    } while (!g_restart_required);

    // Reboot
    NVIC_SystemReset();
}

/* TIME STAMP

MASTER ---->>>>>
    // State variables
    56 # transfer_id := 0;
    57 # previous_tx_timestamp_per_iface[NUM_IFACES] := {0};
    58 #
    59 # // This function publishes a message with a specified transfer-ID using only one transport interface.
    60 # function publishMessage(transfer_id, iface_index, msg);
    61 #
    62 # // This callback is invoked when the transport layer completes the transmission of a time sync message.
    63 # // Observe that the time sync message is always a single-frame message by virtue of its small size.
    64 # // The tx_timestamp argument contains the exact timestamp when the transport frame was delivered to the bus.
    65 # function messageTxTimestampCallback(iface_index, tx_timestamp)
    66 # {
    67 # previous_tx_timestamp_per_iface[iface_index] := tx_timestamp;
    68 # }
    69 #
    70 # // Publishes messages of type uavcan.time.Synchronization to each available transport interface.
    71 # // It is assumed that this function is invoked with a fixed frequency not lower than 1 hertz.
    72 # function publishTimeSync()
    73 # {
    74 # for (i := 0; i < NUM_IFACES; i++)
    75 # {
    76 # message := uavcan.time.Synchronization();
    77 # message.previous_transmission_timestamp_usec := previous_tx_timestamp_per_iface[i];
    78 # previous_tx_timestamp_per_iface[i] := 0;
    79 # publishMessage(transfer_id, i, message);
    80 # }
    81 # transfer_id++; // Overflow shall be handled correctly
    82 # }

*/