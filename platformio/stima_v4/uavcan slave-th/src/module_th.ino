/// This software is distributed under the terms of the MIT License.
/// Copyright (C) 2021 OpenCyphal <maintainers@opencyphal.org>
/// Author: Pavel Kirienko <pavel@opencyphal.org>
/// Revis.: Gasperini Moreno <m.gasperini@digiteco.it>

/// TODO: (ricerca e ricontrolla i TODO vari)
/// STAMPE CORRETTE E A DEFINE_SOFTWARE E -> LEVEL COME ESEMPIO DI PAOLO
/// DSDL Esatta e conforme status, command, Lx, ecc. versione FINALE e tutte le altre slave
/// C++

/// NOTE:
/// msg->vendor_specific_status_code in Heartbeat può indicare (Fine acq, + altri STATI locali)
///     Errori, bassa alimentazione 0/1/2, Ho finito acq. in che modalità sono ecc... GIA OK!!!
///     Va solo gestito in logica master ma funziona correttamente
/// Capire L1/L2 quali parametri sono in master... x SET Misura ecc...
/// Funzionalità gestibili entrata in OffLine, uscita in OnLine (Master e Slave)
/// Funzionalità gestibili tutti i timeOut, OK ciclo STD ma altri possibili (Master e Slave)

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
#include <uavcan/file/Read_1_1.h>
#include <uavcan/time/Synchronization_1_0.h>
#include <uavcan/pnp/NodeIDAllocationData_1_0.h>
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
typedef struct State {
    CanardMicrosecond started_at;
    uint32_t lastMicrosecond;
    uint64_t currentMicrosecond;

    O1HeapInstance* heap;
    CanardInstance canard;
    CanardTxQueue canard_tx_queues[CAN_REDUNDANCY_FACTOR];

    // Subject ID porte e servizi modulo locale
    struct
    {
        struct
        {
            CanardPortID module_th;
        } publisher;
        struct
        {
            CanardPortID module_th;
        } service;
    } port_id;

    // Gestione master/server e funzioni attive in RX messaggi
    // Master Remoto
    struct
    {
        // Master OnLine / OffLine
        bool     is_online;
        // Heartbeat
        struct {
            uint8_t  state;
            uint64_t timeout_us;
        } heartbeat;
        // Time stamp
        struct {
            CanardMicrosecond previous_local_timestamp_usec;
            CanardMicrosecond previous_msg_monotonic_usec;
            CanardMicrosecond synchronized_last_update;
            CanardMicrosecond synchronized;
            uint8_t previous_transfer_id;
        } timestamp;
        // File upload (node_id può essere differente dal master, es. yakut e lo riporto)
        struct
        {
            uint8_t  node_id;
            char     filename[FILE_NAME_SIZE_MAX];
            bool     is_firmware;
            bool     updating;    
            bool     updating_eof;
            bool     updating_run;
            byte     updating_retry;
            uint64_t offset;
            uint64_t timeout_us;            // Time command Remoto x Verifica deadLine Request
            bool     is_pending;            // Funzione in pending (inviato, attesa risposta o timeout)
            bool     is_timeout;            // Funzione in timeout (mancata risposta)
        } file;
    } master;

    // Abilitazione delle pubblicazioni falcoltative sulla rete (ON/OFF a richiesta)
    struct
    {
        bool module_th;
        bool port_list;
    } publisher_enabled;

    // Metadata del modulo locale (valori fissi)
    rmap_metadata_Metadata_1_0 module_metadata;

    // Tranfer ID (CAN Interfaccia ID -> uint8) servizi attivi del modulo locale
    struct
    {
        uint8_t uavcan_node_heartbeat;
        uint8_t uavcan_node_port_list;
        uint8_t uavcan_pnp_allocation;
        uint8_t uavcan_file_read_data;
        uint8_t module_th;
    } next_transfer_id;

    // Flag di state
    struct
    {
        bool g_restart_required;            // Forzatura reboot del nodo
    } flag;
} State;

// ***************************************************************************************************
// **********             Funzioni ed utility generiche per gestione UAVCAN                 **********
// ***************************************************************************************************

// Ritorna i uS dalle funzioni Micros di Arduino (in formato 64 BIT necessario per UAVCAN)
// Non permette il reset n ei 70 minuti circa previsti per l'overflow della funzione uS a 32 Bit
static CanardMicrosecond getMonotonicMicroseconds(State* const state) {
    uint32_t ts = micros();
    if(ts > state->lastMicrosecond) {
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

/// Legge il subjectID per il modulo corrente per la risposta al servizio di gestione dati.
/// The standard register schema is documented in the Cyphal Specification, section for the standard service
/// https://github.com/OpenCyphal/public_regulated_data_types/blob/master/uavcan/register/384.Access.1.0.dsdl
/// A very hands-on demo is available in Python: https://pycyphal.readthedocs.io/en/stable/pages/demo.html
static CanardPortID getModeAccessID(byte modeAccessID, const char* const port_name, const char* const type_name) {
    // Deduce the register name from port name e modeAccess
    char register_name[uavcan_register_Name_1_0_name_ARRAY_CAPACITY_] = {0};
    // In funzione del modo imposta il registro corretto
    switch (modeAccessID) {
        case PublisherSubjectID:
            snprintf(&register_name[0], sizeof(register_name), "uavcan.pub.%s.id", port_name);
            break;
        case SubscriptionSubjectID:
            snprintf(&register_name[0], sizeof(register_name), "uavcan.sub.%s.id", port_name);
            break;
        case ClientPortID:
            snprintf(&register_name[0], sizeof(register_name), "uavcan.cln.%s.id", port_name);
            break;
        case ServicePortID:
            snprintf(&register_name[0], sizeof(register_name), "uavcan.srv.%s.id", port_name);
            break;
    }     

    // Set up the default value. It will be used to populate the register if it doesn't exist.
    uavcan_register_Value_1_0 val = {0};
    uavcan_register_Value_1_0_select_natural16_(&val);
    val.natural16.value.count = 1;
    val.natural16.value.elements[0] = UINT16_MAX;  // This means "undefined", per Specification, which is the default.

    // Read the register with defensive self-checks.
    registerRead(&register_name[0], &val);
    assert(uavcan_register_Value_1_0_is_natural16_(&val) && (val.natural16.value.count == 1));
    const uint16_t result = val.natural16.value.elements[0];

    // This part is NOT required but recommended by the Specification for enhanced introspection capabilities. It is
    // very cheap to implement so all implementations should do so. This register simply contains the name of the
    // type exposed at this port. It should be immutable but it is not strictly required so in this implementation
    // we take shortcuts by making it mutable since it's behaviorally simpler in this specific case.
    // In funzione del modo imposta il registro corretto
    switch (modeAccessID) {
        case PublisherSubjectID:
            snprintf(&register_name[0], sizeof(register_name), "uavcan.pub.%s.type", port_name);
            break;
        case SubscriptionSubjectID:
            snprintf(&register_name[0], sizeof(register_name), "uavcan.sub.%s.type", port_name);
            break;
        case ClientPortID:
            snprintf(&register_name[0], sizeof(register_name), "uavcan.cln.%s.type", port_name);
            break;
        case ServicePortID:
            snprintf(&register_name[0], sizeof(register_name), "uavcan.srv.%s.type", port_name);
            break;
    }

    uavcan_register_Value_1_0_select_string_(&val);
    val._string.value.count = nunavutChooseMin(strlen(type_name), uavcan_primitive_String_1_0_value_ARRAY_CAPACITY_);
    memcpy(&val._string.value.elements[0], type_name, val._string.value.count);
    registerWrite(&register_name[0], &val);  // Unconditionally overwrite existing value because it's read-only.

    return result;
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
static void handleFileReadBlock_1_1(State* const state, const CanardMicrosecond monotonic_time)
{
    // ***** Ricezione di file generico dalla rete UAVCAN dal nodo chiamante *****
    // Richiamo in continuazione rapida la funzione fino al riempimento del file
    // Alla fine processo il firmware Upload (eventuale) vero e proprio con i relativi check
    uavcan_file_Read_Request_1_1 remotefile = {0};
    remotefile.path.path.count = strlen(state->master.file.filename);
    memcpy(remotefile.path.path.elements, state->master.file.filename, remotefile.path.path.count);
    remotefile.offset = state->master.file.offset;

    uint8_t      serialized[uavcan_file_Read_Request_1_1_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
    size_t       serialized_size                                                           = sizeof(serialized);
    const int8_t err = uavcan_file_Read_Request_1_1_serialize_(&remotefile, &serialized[0], &serialized_size);
    assert(err >= 0);
    if (err >= 0)
    {
        const CanardTransferMetadata meta = {
            .priority       = CanardPriorityHigh,
            .transfer_kind  = CanardTransferKindRequest,
            .port_id        = uavcan_file_Read_1_1_FIXED_PORT_ID_,
            .remote_node_id = state->master.file.node_id,
            .transfer_id    = (CanardTransferID) (state->next_transfer_id.uavcan_file_read_data++),
        };
        send(state,
                monotonic_time + MEGA,
                &meta,
                serialized_size,
                &serialized[0]);
    }
}

// *******              FUNZIONI INVOCATE HANDLE FAST_LOOP EV. PREPARATORIE              *********

// Prepara il blocco messaggio dati per il modulo corrente istantaneo (Crea un esempio)
// TODO: Collegare al modulo sensor_drive per il modulo corrente
// NB: Aggiorno solo i dati fisici in questa funzione i metadati sono esterni
rmap_module_TH_1_0 prepareSensorsDataValueExample(void) {
    rmap_module_TH_1_0 local_data;
    // TODO: Inserire i dati, passaggio da Update... altro
    local_data.temperature.val.value = (int32_t)(rand() % 2000 + 27315);
    local_data.temperature.confidence.value = (uint8_t)(rand() % 100);
    local_data.humidity.val.value = (int32_t)(rand() % 100);
    local_data.humidity.confidence.value = (uint8_t)(rand() % 100);
    return local_data;
}

/// Invoked at the rate of the fastest loop.
static void handleFastLoop(State* const state, const CanardMicrosecond monotonic_time) {
    // Controlla corretta assegnazione node_id
    const bool anonymous = state->canard.node_id > CANARD_NODE_ID_MAX;
    // Pubblica i dati del nodo corrente se abilitata la funzione e con il corretto subjectId
    // Ovviamente il nodo non può essere anonimo per la pubblicazione...
    if ((!anonymous) &&
        (state->publisher_enabled.module_th) &&
        (state->port_id.publisher.module_th <= CANARD_SUBJECT_ID_MAX)) {
        rmap_module_TH_1_0 module_th_msg = prepareSensorsDataValueExample();
        // Copio i metadati fissi
        module_th_msg.metadata = state->module_metadata;        
        // Serialize and publish the message:
        uint8_t serialized[rmap_module_TH_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
        size_t serialized_size = sizeof(serialized);
        const int8_t err = rmap_module_TH_1_0_serialize_(&module_th_msg, &serialized[0], &serialized_size);
        assert(err >= 0);
        if (err >= 0) {
            const CanardTransferMetadata meta = {
                .priority = CanardPrioritySlow,
                .transfer_kind = CanardTransferKindMessage,
                .port_id = state->port_id.publisher.module_th,
                .remote_node_id = CANARD_NODE_ID_UNSET,
                .transfer_id = (CanardTransferID)state->next_transfer_id.module_th++,  // Increment!
            };
            send(state,
                 monotonic_time + MEGA / 4,    // 0.25 Secondi per pubblicazione dati MAX
                 &meta, serialized_size,
                 &serialized[0]);
        }
    }
}

// *******              FUNZIONI INVOCATE HANDLE 1 SECONDO EV. PREPARATORIE              *********

static void handleNormalLoop(State* const state, const CanardMicrosecond monotonic_time) {
    const bool anonymous = state->canard.node_id > CANARD_NODE_ID_MAX;
    // Heartbeat ogni secondo (la funzione è richiesta e non può essere mascherata in UAVCAN)
    // In anonimo invece esegue la richiesta di allocazione node_id con il servizio plug_and_play 1.0
    if (!anonymous) {
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
        if(state->master.file.updating) {
            // heartbeat.mode.value = uavcan_node_Mode_1_0_SOFTWARE_UPDATE;
            heartbeat.vendor_specific_status_code = VSC_SOFTWARE_UPDATE_READ;
        }
        // A fine trasferimento completo
        if(state->master.file.updating_run) {
            // Utilizzare questo flag solo in avvio di update (YAKUT Blocca i trasferimenti)
            // Altrimenti ricomincia il trasferimento da capo da inizio file all'infinito...
            heartbeat.mode.value = uavcan_node_Mode_1_0_SOFTWARE_UPDATE;
        }
        uint8_t serialized[uavcan_node_Heartbeat_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
        size_t serialized_size = sizeof(serialized);
        const int8_t err = uavcan_node_Heartbeat_1_0_serialize_(&heartbeat, &serialized[0], &serialized_size);
        assert(err >= 0);
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
    else  // If we don't have a node-ID, obtain one by publishing allocation request messages until we get a response.
    {
        // The Specification says that the allocation request publication interval shall be randomized.
        // We implement randomization by calling rand() at fixed intervals and comparing it against some threshold.
        if (rand() > RAND_MAX / 2)  // NOLINT
        {
            // PnP over Classic CAN, use message v1.0.
            uavcan_pnp_NodeIDAllocationData_1_0 msg = {0};
            /// truncated uint48 unique_id_hash
            // Crea uint_64 con LOW_POWER NODE_TYPE_MAJOR << 8 + NODE_TYPE_MINOR
            uint64_t local_unique_id_hash = 0;
            local_unique_id_hash |= (uint64_t) NODE_TYPE_MINOR;
            local_unique_id_hash |= (uint64_t) ((uint16_t) NODE_TYPE_MAJOR << 8);
            for(byte bRnd=2; bRnd<8; bRnd++) {
                local_unique_id_hash |= ((uint64_t)(rand() & 0xFF)) << 8*bRnd;
            }
            // msg.allocated_node_id.count
            // msg.allocated_node_id.(count/element) => Solo in response non in request;
            msg.unique_id_hash = local_unique_id_hash;
            uint8_t serialized[uavcan_pnp_NodeIDAllocationData_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
            size_t serialized_size = sizeof(serialized);
            const int8_t err = uavcan_pnp_NodeIDAllocationData_1_0_serialize_(&msg, &serialized[0], &serialized_size);
            assert(err >= 0);
            if (err >= 0) {
                const CanardTransferMetadata meta = {
                    .priority = CanardPrioritySlow,
                    .transfer_kind = CanardTransferKindMessage,
                    .port_id = uavcan_pnp_NodeIDAllocationData_1_0_FIXED_PORT_ID_,
                    .remote_node_id = CANARD_NODE_ID_UNSET,
                    .transfer_id = (CanardTransferID)(state->next_transfer_id.uavcan_pnp_allocation++),
                };
                send(state,  // The response will arrive asynchronously eventually.
                     monotonic_time + MEGA,
                     &meta,
                     serialized_size,
                     &serialized[0]);
            }
        }
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
            // Aggiungo i publisher interni validi privati (quando abilitati)
            if ((state->port_id.publisher.module_th <= CANARD_SUBJECT_ID_MAX)&&
                (state->publisher_enabled.module_th))
            {
                m.publishers.sparse_list.elements[(*cnt)++].value = state->port_id.publisher.module_th;
            }            
        }

        // Indicate which servers and subscribers we implement.
        // We could construct the list manually but it's easier and more robust to just query libcanard for that.
        fillSubscriptions(state->canard.rx_subscriptions[CanardTransferKindMessage], &m.subscribers);
        fillServers(state->canard.rx_subscriptions[CanardTransferKindRequest], &m.servers);
        fillServers(state->canard.rx_subscriptions[CanardTransferKindResponse], &m.clients);  // For regularity.

        // Serialize and publish the message. Use a small buffer because we know that our message is always small.
        // Verificato massimo utilizzo a 156 bytes. Limitiamo il buffer a 256 Bytes (Come esempio UAVCAN)
        uint8_t serialized[256] = {0};
        size_t  serialized_size = uavcan_node_port_List_0_1_SERIALIZATION_BUFFER_SIZE_BYTES_;
        if (uavcan_node_port_List_0_1_serialize_(&m, &serialized[0], &serialized_size) >= 0) {
            const CanardTransferMetadata meta = {
                .priority = CanardPriorityOptional,  // Mind the priority.
                .transfer_kind = CanardTransferKindMessage,
                .port_id = uavcan_node_port_List_0_1_FIXED_PORT_ID_,
                .remote_node_id = CANARD_NODE_ID_UNSET,
                .transfer_id = (CanardTransferID)(state->next_transfer_id.uavcan_node_port_list++),
            };
            send(state, monotonic_time + MEGA * 2, &meta, serialized_size, &serialized[0]);
        }
    }
}

// ***************************************************************************************************
//   Funzioni ed utility di ricezione dati dalla rete UAVCAN, richiamati da processReceivedTransfer()
// ***************************************************************************************************

// Plug and Play Slave, Versione 1.0 compatibile con CAN_CLASSIC MTU 8
// Messaggi anonimi CAN non sono consentiti se messaggi > LUNGHEZZA MTU disponibile
static void processMessagePlugAndPlayNodeIDAllocation(State* const state,
                                                      const uavcan_pnp_NodeIDAllocationData_1_0* const msg) {
    // msg->unique_id_hash RX non gestito, è valido GetUniqueID Unificato per entrambe versioni V1 e V2
    if (msg->allocated_node_id.elements[0].value <= CANARD_NODE_ID_MAX) {
        printf("Got PnP node-ID allocation: %d\n", msg->allocated_node_id.elements[0].value);
        state->canard.node_id = (CanardNodeID)msg->allocated_node_id.elements[0].value;
        // Store the value into the non-volatile storage.
        uavcan_register_Value_1_0 reg = {0};
        uavcan_register_Value_1_0_select_natural16_(&reg);
        reg.natural16.value.elements[0] = msg->allocated_node_id.elements[0].value;
        reg.natural16.value.count = 1;
        registerWrite("uavcan.node.id", &reg);
        // We no longer need the subscriber, drop it to free up the resources (both memory and CPU time).
        (void)canardRxUnsubscribe(&state->canard,
                                  CanardTransferKindMessage,
                                  uavcan_pnp_NodeIDAllocationData_1_0_FIXED_PORT_ID_);
    }
    // Otherwise, ignore it: either it is a request from another node or it is a response to another node.
}

// Chiamate gestioni RPC remote da master (yakut o altro servizio di controllo)
static uavcan_node_ExecuteCommand_Response_1_1 processRequestExecuteCommand(State* state, const uavcan_node_ExecuteCommand_Request_1_1* req,
                                                                            uint8_t remote_node) {
    uavcan_node_ExecuteCommand_Response_1_1 resp = {0};
    // req->command (Comando esterno ricevuto 2 BYTES RESERVED FFFF-FFFA)
    // Gli altri sono liberi per utilizzo interno applicativo con #define interne
    // req->parameter (array di byte MAX 255 per i parametri da request)
    // Risposta attuale (resp) 1 Bytes RESERVER (0..6) gli altri #define interne
    switch (req->command)
    {
        // **************** Comandi standard UAVCAN GENERIC_SPECIFIC_COMMAND ****************
        // Comando di aggiornamento Firmware compatibile con Yakut e specifice UAVCAN
        case uavcan_node_ExecuteCommand_Request_1_1_COMMAND_BEGIN_SOFTWARE_UPDATE:
        {
            // Nodo Server chiamante (Yakut solo Master, Yakut e Master per Slave)
            state->master.file.node_id = remote_node;
            // Copio la stringa nel name file firmware disponibile su state generale (per download successivo)
            memcpy(state->master.file.filename, req->parameter.elements, req->parameter.count);
            state->master.file.filename[req->parameter.count] = '\0';
            Serial.print(F("Firmware update request from node id: "));
            Serial.println(state->master.file.node_id);
            Serial.print(F("Filename to download: "));
            Serial.println(state->master.file.filename);
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
        case uavcan_node_ExecuteCommand_Request_1_1_COMMAND_FACTORY_RESET:
        {
            registerDoFactoryReset();
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        case uavcan_node_ExecuteCommand_Request_1_1_COMMAND_RESTART:
        {
            state->flag.g_restart_required = true;
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
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
        // **************** Comandi personalizzati VENDOR_SPECIFIC_COMMAND ****************
        // Comando di download File generico compatibile con specifice UAVCAN, (LOG/CFG altro...)
        case CMD_DOWNLOAD_FILE:
        {
            // Nodo Server chiamante (Yakut solo Master, Yakut e Master per Slave)
            state->master.file.node_id = remote_node;
            // Copio la stringa nel name file generico disponibile su state generale (per download successivo)
            memcpy(state->master.file.filename, req->parameter.elements, req->parameter.count);
            state->master.file.filename[req->parameter.count] = '\0';
            Serial.print(F("File download request from node id: "));
            Serial.println(state->master.file.node_id);
            Serial.print(F("Filename to download: "));
            Serial.println(state->master.file.filename);
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
        case CMD_ENABLE_PUBLISH_DATA:
        {
            // Abilita pubblicazione fast_loop data_and_metadata modulo locale (test yakut e user master)
            state->publisher_enabled.module_th = true;
            Serial.println(F("ATTIVO Trasmissione dati in publish"));
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        case CMD_DISABLE_PUBLISH_DATA:
        {
            // Disabilita pubblicazione fast_loop data_and_metadata modulo locale (test yakut e user master)
            state->publisher_enabled.module_th = false;
            Serial.println(F("DISATTIVO Trasmissione dati in publish"));
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        case CMD_ENABLE_PUBLISH_PORT_LIST:
        {
            // Abilita pubblicazione slow_loop elenco porte (Cypal facoltativo)
            state->publisher_enabled.port_list = true;
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        case CMD_DISABLE_PUBLISH_PORT_LIST:
        {
            // Disabilita pubblicazione slow_loop elenco porte (Cypal facoltativo)
            state->publisher_enabled.port_list = false;
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

// Chiamate gestioni dati remote da master (yakut o altro servizio di controllo)
static rmap_service_module_TH_Response_1_0 processRequestGetModuleData(State* state,
                                                                        rmap_service_module_TH_Request_1_0* req) {
    rmap_service_module_TH_Response_1_0 resp = {0};
    // Richeista parametri univoca a tutti i moduli
    // req->parametri tipo: rmap_service_setmode_1_0
    // req->parametri.comando (Comando esterno ricevuto 3 BIT)
    // req->parametri.run_sectime (Timer to run 13 bit)

    // Copio i metadati fissi
    resp.dataandmetadata.metadata = state->module_metadata;

    // Case comandi RMAP su GetModule Data (Da definire con esattezza quali e quanti altri)
    switch (req->parametri.comando) {

        /// saturated uint3 get_istant = 0
        /// Ritorna il dato istantaneo (o ultima acquisizione dal sensore)
        case rmap_service_setmode_1_0_get_istant:
            // Ritorno lo stato (Copia dal comando...)
            resp.stato = req->parametri.comando;
            // Preparo la risposta di esempio
            resp.dataandmetadata = prepareSensorsDataValueExample();
            break;

        /// saturated uint3 get_current = 1
        /// Ritorna il dato attuale (ciclo finito o no lo stato di acq_vale)
        case rmap_service_setmode_1_0_get_current:
            // resp.dataandmetadata = prepareSensorsDataGetCurrent();
            resp.stato = GENERIC_BVAL_UNDEFINED;
            break;

        /// saturated uint3 get_last = 2
        /// Ritorna l'ultimo valore valido di acquisizione (riferito al ciclo precedente)
        /// Se utilizzato con loop automatico, shifta il valore senza perdite di tempo riavvia il ciclo
        case rmap_service_setmode_1_0_get_last:
            resp.stato = GENERIC_BVAL_UNDEFINED;
            break;

        /// saturated uint3 reset_last = 3
        /// Reset dell'ultimo valore (dopo lettura... potrebbe essere un comando di command standard)
        /// Potremmo collegare lo stato a heartbeat (ciclo di acquisizione finito, dati disponibili...)
        case rmap_service_setmode_1_0_reset_last:
            resp.stato = GENERIC_BVAL_UNDEFINED;
            break;

        /// saturated uint3 start_acq = 4
        /// Avvio ciclo di lettura... una tantum start stop automatico, con tempo parametrizzato
        case rmap_service_setmode_1_0_start_acq:
            resp.stato = GENERIC_BVAL_UNDEFINED;
            break;

        /// saturated uint3 stop_acq = 5
        /// Arresta ciclo di lettura in ogni condizione (standard o loop)
        case rmap_service_setmode_1_0_stop_acq:
            resp.stato = GENERIC_BVAL_UNDEFINED;
            break;

        /// saturated uint3 loop_acq = 6
        /// Avvio ciclo di lettura... in loop automatico continuo, con tempo parametrizzato
        case rmap_service_setmode_1_0_loop_acq:
            resp.stato = GENERIC_BVAL_UNDEFINED;
            break;

        /// saturated uint3 continuos_acq = 7
        /// Avvio ciclo di lettura... in continuo, senza tempo parametrizzato (necessita di stop remoto)
        case rmap_service_setmode_1_0_continuos_acq:
            resp.stato = GENERIC_BVAL_UNDEFINED;
            break;

        /// NON Gestito, risposta error (undefined)
        default:
            resp.stato = GENERIC_BVAL_UNDEFINED;
            break;
    }

    return resp;
}

// Accesso ai registri UAVCAN risposta a richieste
static uavcan_register_Access_Response_1_0 processRequestRegisterAccess(const uavcan_register_Access_Request_1_0* req) {
    char name[uavcan_register_Name_1_0_name_ARRAY_CAPACITY_ + 1] = {0};
    assert(req->name.name.count < sizeof(name));
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
    if (transfer->metadata.transfer_kind == CanardTransferKindMessage)
    {
        if (transfer->metadata.port_id == uavcan_node_Heartbeat_1_0_FIXED_PORT_ID_)
        {
            // Ricevo messaggi Heartbeat per stato rete (Controllo esistenza del MASTER)
            // Solo a scopo precauzionale per attività da gestire alla cieca (SAVE QUEUE LOG, DATA ecc...)
            size_t size = transfer->payload_size;
            uavcan_node_Heartbeat_1_0 msg = {0};
            if (uavcan_node_Heartbeat_1_0_deserialize_(&msg, static_cast<uint8_t const*>(transfer->payload), &size) >= 0) {
                // Controllo e gestisco solo il nodo MASTER
                if(transfer->metadata.remote_node_id == NODE_MASTER_ID) {
                    // Entro in OnLine se precedentemente arrivo dall'OffLine
                    // ed eseguo eventuali operazioni di entrata in attività se necessario
                    if(!state->master.is_online) {
                        // Il master è entrato in modalità ONLine e gestisco
                        Serial.println(F("Master controller ONLINE !!! Starting application..."));
                    }
                    Serial.print(F("RX HeartBeat from master, master state code: -> "));
                    Serial.println(msg.vendor_specific_status_code);
                    // Processo e registro il nodo: stato, OnLine e relativi flag
                    // Set canard_us local per controllo NodoOffline
                    state->master.is_online = true;
                    state->master.heartbeat.state = msg.vendor_specific_status_code;
                    state->master.heartbeat.timeout_us = transfer->timestamp_usec + MASTER_OFFLINE_TIMEOUT_US;
                }
            }
        }
        else if (transfer->metadata.port_id == uavcan_time_Synchronization_1_0_FIXED_PORT_ID_)
        {
            // Ricevo messaggi Heartbeat per syncro_time CanardMicrosec (Base dei tempi locali dal MASTER)
            // Gestione differenze del tempo tra due comunicazioni Canard time_incro del master (local adjust time)
            size_t size = transfer->payload_size;
            uavcan_time_Synchronization_1_0 msg = {0};
            if (uavcan_time_Synchronization_1_0_deserialize_(&msg, static_cast<uint8_t const*>(transfer->payload), &size) >= 0) {
                // Controllo e gestisco solo il nodo MASTER come SyncroTime
                if(transfer->metadata.remote_node_id == NODE_MASTER_ID) {
                    // Controllo coerenza messaggio per consentire l'aggiornamento timestamp
                    // 1) Sequenza di transfer_id con previous
                    // 2) differenza di tempo tra due timestamp remoti inferiore al massimo timeOut impostato di controllo
                    if((++state->master.timestamp.previous_transfer_id == transfer->metadata.transfer_id) &&
                        ((msg.previous_transmission_timestamp_microsecond - state->master.timestamp.previous_msg_monotonic_usec) < MASTER_MAXSYNCRO_VALID_US))
                    {
                        // Il tempo sincronizzato è il time_stamp ricevuto precedente sommato alla differenza reale locale di rx dei due messaggi syncronized_timestamp consecutivi validi
                        state->master.timestamp.synchronized = msg.previous_transmission_timestamp_microsecond + (transfer->timestamp_usec - state->master.timestamp.previous_local_timestamp_usec);
                        // Aggiusto e sommo la differenza tra il timestamp reale del messaggio ricevuto con il monotonic reale attuale (al tempo di esecuzione syncro real)
                        state->master.timestamp.synchronized += (getMonotonicMicroseconds(state) - transfer->timestamp_usec);
                        // TODO: Eliminare solo per FAKE RTC (Usare la procedura sotto...)
                        state->master.timestamp.synchronized_last_update = getMonotonicMicroseconds(state);
                        Serial.print(F("RX TimeSyncro from master, syncronized value is (uSec): "));
                        Serial.print(state->master.timestamp.synchronized);
                        Serial.print(F(" from 2 message difference is (uSec): "));
                        Serial.println(msg.previous_transmission_timestamp_microsecond - state->master.timestamp.previous_msg_monotonic_usec);
                    } else {
                        // Sincronizzo il transferID Corretto solo in init o Reset è utile
                        state->master.timestamp.previous_transfer_id = transfer->metadata.transfer_id;
                        Serial.print(F("RX TimeSyncro from master, reset or invalid Value at local_time_stamp (uSec): "));
                        Serial.println(transfer->timestamp_usec);
                    }
                    // Save timestamp variabili per SET e controllo Time nella successiva chiamata time_syncronization
                    // Local RealTime RX timestamp monotonic locale al tempo reale di rx_message (transfer->timestamp_usec)
                    state->master.timestamp.previous_local_timestamp_usec = transfer->timestamp_usec;
                    // RealTime callBack message RX (TX RTC uSec(Canard type) Master Remoto al tempo di send)
                    state->master.timestamp.previous_msg_monotonic_usec =  msg.previous_transmission_timestamp_microsecond;
                    // Adjust Local RTC Time!!!
                    // TODO: Pseudocode
                    // if abs((timeStamp) - convertCanardUsec(RTC)) > 500000 (0.5 secondi...)
                    //   -> SETDateTime 
                }
            }
        }
        else if (transfer->metadata.port_id == uavcan_pnp_NodeIDAllocationData_1_0_FIXED_PORT_ID_)
        {
            // PLUG AND PLAY (Dovrei ricevere solo in anonimo)
            size_t size = transfer->payload_size;
            uavcan_pnp_NodeIDAllocationData_1_0 msg = {0};
            if (uavcan_pnp_NodeIDAllocationData_1_0_deserialize_(&msg, static_cast<uint8_t const*>(transfer->payload), &size) >= 0) {
                processMessagePlugAndPlayNodeIDAllocation(state, &msg);
            }
        }
        else
        {
            // Gestione di un messaggio senza sottoscrizione. Se arrivo quà è un errore di sviluppo
            assert(false);
        }
    }
    else if (transfer->metadata.transfer_kind == CanardTransferKindRequest)
    {
        // Gestione delle richieste esterne
        if (transfer->metadata.port_id == state->port_id.service.module_th) {
            // Richiesta ai dati e metodi di sensor drive
            rmap_service_module_TH_Request_1_0 req = {0};
            size_t size = transfer->payload_size;
            Serial.println(F("<<-- Ricevuto richiesta dati da master"));
            // The request object is empty so we don't bother deserializing it. Just send the response.
            if (rmap_service_module_TH_Request_1_0_deserialize_(&req, static_cast<uint8_t const*>(transfer->payload), &size) >= 0) {
                // I dati e metadati sono direttamente popolati in processRequestGetModuleData
                rmap_service_module_TH_Response_1_0 module_th_resp = processRequestGetModuleData(state, &req);
                // Serialize and publish the message:
                uint8_t serialized[rmap_service_module_TH_Response_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
                size_t serialized_size = sizeof(serialized);
                const int8_t res = rmap_service_module_TH_Response_1_0_serialize_(&module_th_resp, &serialized[0], &serialized_size);
                if (res >= 0) {
                    sendResponse(state,
                                transfer->timestamp_usec + MEGA,
                                &transfer->metadata,
                                serialized_size,
                                &serialized[0]);
                }
            }
        }
        else if (transfer->metadata.port_id == uavcan_node_GetInfo_1_0_FIXED_PORT_ID_)
        {
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
        }
        else if (transfer->metadata.port_id == uavcan_register_Access_1_0_FIXED_PORT_ID_)
        {
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
        }
        else if (transfer->metadata.port_id == uavcan_register_List_1_0_FIXED_PORT_ID_)
        {
            uavcan_register_List_Request_1_0 req = {0};
            size_t size = transfer->payload_size;
            if (uavcan_register_List_Request_1_0_deserialize_(&req, static_cast<uint8_t const*>(transfer->payload), &size) >= 0) {
                const uavcan_register_List_Response_1_0 resp = {.name = registerGetNameByIndex(req.index)};
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
        }
        else if (transfer->metadata.port_id == uavcan_node_ExecuteCommand_1_1_FIXED_PORT_ID_)
        {
            uavcan_node_ExecuteCommand_Request_1_1 req = {0};
            size_t size = transfer->payload_size;
            Serial.println(F("<<-- Ricevuto comando esterno"));
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
        }
        else
        {
            // Gestione di una richiesta senza controllore locale. Se arrivo quà è un errore di sviluppo
            assert(false);
        }
    }
    else if (transfer->metadata.transfer_kind == CanardTransferKindResponse)
    {
        if (transfer->metadata.port_id == uavcan_file_Read_1_1_FIXED_PORT_ID_) {
            uavcan_file_Read_Response_1_1 resp  = {0};
            size_t                         size = transfer->payload_size;
            if (uavcan_file_Read_Response_1_1_deserialize_(&resp, static_cast<uint8_t const*>(transfer->payload), &size) >= 0) {
                // Reset pending command (Comunico request/Response Serie di comandi OK!!!)
                state->master.file.is_pending = false;
                // Azzero contestualmente le retry di controllo x gestione MAX_RETRY -> ABORT
                state->master.file.updating_retry = 0;
                if(state->master.file.is_firmware) {
                    Serial.print(F("RX FIRMWARE READ BLOCK LEN: "));
                } else {
                    Serial.print(F("RX FILE READ BLOCK LEN: "));
                }
                Serial.println(resp.data.value.count);
                // Save Data in File at Block Position (Init = Rewrite file...)
                putDataFile(state->master.file.filename, state->master.file.is_firmware, state->master.file.offset==0,
                            resp.data.value.elements, resp.data.value.count);
                state->master.file.offset+=resp.data.value.count;
                if(resp.data.value.count != uavcan_primitive_Unstructured_1_0_value_ARRAY_CAPACITY_) {
                    // Blocco con EOF!!! Fine trasferimento, nessun altro blocco disponibile
                    state->master.file.updating_eof = true;
                }
            }
        }
    }
    else
    {
        // Se arrivo quà è un errore di sviluppo, controllare setup sottoscrizioni e Rqst (non gestito slave)
        assert(false);
    }
}

// *********************************************************************************************
//          Inizializzazione generale HW, canard, CAN_BUS, e dispositivi collegati
// *********************************************************************************************

static void* canardAllocate(CanardInstance* const ins, const size_t amount) {
    O1HeapInstance* const heap = ((State*)ins->user_reference)->heap;
    assert(o1heapDoInvariantsHold(heap));
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
#define bxCANRxQueueEmpty()         (canard_rx_queue.wr_ptr=canard_rx_queue.rd_ptr)
#define bxCANRxQueueIsEmpty()       (canard_rx_queue.wr_ptr==canard_rx_queue.rd_ptr)
#define bxCANRxQueueDataPresent()   (canard_rx_queue.wr_ptr!=canard_rx_queue.rd_ptr)
// For Monitor queue Interrupt RX movement
#define bxCANRxQueueElement()       (canard_rx_queue.wr_ptr>=canard_rx_queue.rd_ptr ? canard_rx_queue.wr_ptr-canard_rx_queue.rd_ptr: canard_rx_queue.wr_ptr+(CAN_RX_QUEUE_CAPACITY-canard_rx_queue.rd_ptr))
#define bxCANRxQueueNextElement(x)  (x+1 < CAN_RX_QUEUE_CAPACITY ? x+1 : 0)
typedef struct Canard_rx_queue
{
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
extern "C" void CAN1_RX0_IRQHandler(void)
{
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
        if(testElement != canard_rx_queue.rd_ptr) {
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
    CAN_FilterInitStruct.FilterFIFOAssignment = CAN_RX_FIFO0;
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

    // *******************         CANARD SETUP TIMINGS AND SPEED        *******************
    // CAN BITRATE Dinamico su LoadRegister (CAN_FD 2xREG natural32 0=Speed, 1=0 (Not Used))
    uavcan_register_Value_1_0 val = {0};
    uavcan_register_Value_1_0_select_natural32_(&val);
    val.natural32.value.count       = 2;
    val.natural32.value.elements[0] = CAN_BIT_RATE;
    val.natural32.value.elements[1] = 0ul;          // Ignored for CANARD_MTU_CAN_CLASSIC
    registerRead("uavcan.can.bitrate", &val);
    assert(uavcan_register_Value_1_0_is_natural32_(&val) && (val.natural32.value.count == 2));

    // Dynamic BIT RATE Change CAN Speed to CAN_BIT_RATE (register default/defined)
    BxCANTimings timings;
    bool result = bxCANComputeTimings(HAL_RCC_GetPCLK1Freq(), val.natural32.value.elements[0], &timings);
    if (!result) {
        Serial.println(F("Error redefinition bxCANComputeTimings, try loading default..."));
        val.natural32.value.count       = 2;
        val.natural32.value.elements[0] = CAN_BIT_RATE;
        val.natural32.value.elements[1] = 0ul;          // Ignored for CANARD_MTU_CAN_CLASSIC
        registerWrite("uavcan.can.bitrate", &val);
        result = bxCANComputeTimings(HAL_RCC_GetPCLK1Freq(), val.natural32.value.elements[0], &timings);
        if (!result) {
            Serial.println(F("Error initialization bxCANComputeTimings"));
            assert(false);
            return false;
        }
    }
    // Attivazione bxCAN sulle interfacce richieste, velocità e modalità
    result = bxCANConfigure(0, timings, false);
    if (!result) {
        Serial.println(F("Error initialization bxCANConfigure"));
        assert(false);
        return false;
    }
    // *******************     CANARD SETUP TIMINGS AND SPEED COMPLETE   *******************

    // Check error starting CAN
    if (HAL_CAN_Start(&CAN_Handle) != HAL_OK) {
        Serial.println(F("CAN startup ERROR!!!"));
        assert(false);
        return false;
    }

    // Enable Interrupt RX Standard CallBack -> CAN1_RX0_IRQHandler
    if (HAL_CAN_ActivateNotification(&CAN_Handle, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
        Serial.println(F("Error initialization interrupt CAN base"));
        assert(false);
        return false;
    }
    // Setup Priority e CB CAN_IRQ_RX Enable
    HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);

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
    // Output mode for LED BLINK SW LOOP (High per Setup)
    // Input mode for test button
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(USER_BTN, INPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    // *****************************************************
    //      STARTUP LIBRERIA SD/MEM REGISTER COLLEGATA
    // *****************************************************
    if (!setupSd(PIN_SPI_MOSI, PIN_SPI_MISO, PIN_SPI_SCK, PIN_SPI_SS, 18)) {
        Serial.println(F("Initialization SD card error"));
        assert(false);
    }
    Serial.println(F("Initialization SD card done"));

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

    // *****************************************************
    //            STARTUP CANBUS E CANARD SPEED
    // *****************************************************
    Serial.print(F("Initializing CANBUS..., PCLK1 Clock Freq: "));
    Serial.println(HAL_RCC_GetPCLK1Freq());
    if (!CAN_HW_Init()) {
        Serial.println(F("Initialization CAN BUS error"));
        assert(false);
    }
    Serial.println(F("Initialization CAN BUS done"));

    // Led Low Init Setup OK
    digitalWrite(LED_BUILTIN, LOW);
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
    state.publisher_enabled.port_list = DEFAULT_PUBLISH_PORT_LIST;
    state.publisher_enabled.module_th = DEFAULT_PUBLISH_MODULE_DATA;
     
    // ********************************************************************************
    //                   READING REGISTER FROM E2 MEMORY / FLASH / SDCARD
    // ********************************************************************************

    // *********               Lettura Registri standard UAVCAN               *********

    // Restore the node-ID from the corresponding standard register. Default to anonymous.
    #ifdef NODE_SLAVE_ID
    // Canard Slave NODE ID Fixed dal defined value in module_config
    state.canard.node_id = (CanardNodeID) NODE_SLAVE_ID;
    #else
    uavcan_register_Value_1_0_select_natural16_(&val);
    val.natural16.value.count = 1;
    val.natural16.value.elements[0] = UINT16_MAX; // This means undefined (anonymous), per Specification/libcanard.
    registerRead("uavcan.node.id", &val);         // The names of the standard registers are regulated by the Specification.
    assert(uavcan_register_Value_1_0_is_natural16_(&val) && (val.natural16.value.count == 1));
    state.canard.node_id = (val.natural16.value.elements[0] > CANARD_NODE_ID_MAX)
                               ? CANARD_NODE_ID_UNSET
                               : (CanardNodeID)val.natural16.value.elements[0];
    #endif

    // The description register is optional but recommended because it helps constructing/maintaining large networks.
    // It simply keeps a human-readable description of the node that should be empty by default.
    uavcan_register_Value_1_0_select_string_(&val);
    val._string.value.count = 0;
    registerRead("uavcan.node.description", &val);  // We don't need the value, we just need to ensure it exists.

    // Configura il trasporto dal registro standard uavcan. Default a CANARD_MTU_MAX
    // Inserito per compatibilità, attualmente non gestita la modifica mtu_bytes (FISSA A MTU_CLASSIC)
    uavcan_register_Value_1_0_select_natural16_(&val);
    val.natural16.value.count       = 1;
    val.natural16.value.elements[0] = CANARD_MTU_MAX;
    registerRead("uavcan.can.mtu", &val);
    assert(uavcan_register_Value_1_0_is_natural16_(&val) && (val.natural16.value.count == 1));
    if(val.natural16.value.elements[0] != CANARD_MTU_MAX) {
        Serial.println(F("Error redefinition MTU, CAN_FD it must be 8. Loading default..."));
        val.natural16.value.count       = 1;
        val.natural16.value.elements[0] = CANARD_MTU_MAX;
        registerWrite("uavcan.can.mtu", &val);
    }
    for (uint8_t ifidx = 0; ifidx < CAN_REDUNDANCY_FACTOR; ifidx++)
    {
        state.canard_tx_queues[ifidx] = canardTxInit(CAN_TX_QUEUE_CAPACITY, val.natural16.value.elements[0]);
    }

    // Carico i/il port-ID/subject-ID del modulo locale dai registri relativi associati nel namespace UAVCAN
    state.port_id.publisher.module_th =
        getModeAccessID(PublisherSubjectID, "TH.data_and_metadata",
                              rmap_module_TH_1_0_FULL_NAME_AND_VERSION_);

    state.port_id.service.module_th =
        getModeAccessID(ServicePortID, "TH.service_data_and_metadata",
                              rmap_service_module_TH_1_0_FULL_NAME_AND_VERSION_);

    // Lettura dei registri RMAP al modulo corrente, con impostazione di default x Startup/Init value

    // Lettura dei registri RMAP 32 Bit relativi al modulo corrente
    uavcan_register_Value_1_0_select_natural32_(&val);
    val.natural32.value.count = 1;
    val.natural32.value.elements[0] = UINT32_MAX;
    registerRead("rmap.module.TH.metadata.Level.L1", &val);
    state.module_metadata.level.L1.value = val.natural32.value.elements[0];
    registerRead("rmap.module.TH.metadata.Level.L2", &val);
    state.module_metadata.level.L2.value = val.natural32.value.elements[0];
    registerRead("rmap.module.TH.metadata.Timerange.P1", &val);
    state.module_metadata.timerange.P1.value = val.natural32.value.elements[0];
    registerRead("rmap.module.TH.metadata.Timerange.P2", &val);
    state.module_metadata.timerange.P2.value = val.natural32.value.elements[0];

    // Lettura dei registri RMAP 8 Bit relativi al modulo corrente
    uavcan_register_Value_1_0_select_natural8_(&val);
    val.natural16.value.count = 1;
    val.natural16.value.elements[0] = UINT8_MAX;
    registerRead("rmap.module.TH.metadata.Timerange.Pindicator", &val);
    state.module_metadata.timerange.Pindicator.value = val.natural8.value.elements[0];
    registerRead("rmap.module.TH.metadata.Level.LevelType1", &val);
    state.module_metadata.level.LevelType1.value = val.natural8.value.elements[0];
    registerRead("rmap.module.TH.metadata.Level.LevelType2", &val);
    state.module_metadata.level.LevelType2.value = val.natural8.value.elements[0];

    // ********************************************************************************
    //               AVVIA SOTTOSCRIZIONI ai messaggi per servizi RPC ecc...
    // ********************************************************************************

    // Plug and Play Versione 1_0 CAN_CLASSIC
    if (state.canard.node_id > CANARD_NODE_ID_MAX) {
        // PnP over Classic CAN, use message v1.0.
        static CanardRxSubscription rx;
        const int8_t res =  //
            canardRxSubscribe(&state.canard,
                              CanardTransferKindMessage,
                              uavcan_pnp_NodeIDAllocationData_1_0_FIXED_PORT_ID_,
                              uavcan_pnp_NodeIDAllocationData_1_0_EXTENT_BYTES_,
                              CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
                              &rx);
        if (res < 0) {
            NVIC_SystemReset();
            // return -res;
        }
    }

    // Service Client: -> Verifica della presenza Heartbeat del MASTER [Networks OffLine]
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

    // Service Client: -> Sincronizzazione timestamp Microsecond del MASTER [su base time local]
    {
        static CanardRxSubscription rx;
        const int8_t                res =  //
            canardRxSubscribe(&state.canard,
                                CanardTransferKindMessage,
                                uavcan_time_Synchronization_1_0_FIXED_PORT_ID_,
                                uavcan_time_Synchronization_1_0_EXTENT_BYTES_,
                                CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
                                &rx);
        if (res < 0) NVIC_SystemReset();
    }

    // Service servers: -> Risposta per GetNodeInfo richiesta esterna master (Yakut, Altri)
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
    
    // Service servers: -> Risposta per ExecuteCommand richiesta esterna master (Yakut, Altri)
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

    // Service servers: -> Risposta per Accesso ai registri richiesta esterna master (Yakut, Altri)
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

    // Service servers: -> Risposta per Lista dei registri richiesta esterna master (Yakut, Altri)
    {
        // Time OUT Canard raddoppiato per elenco registri (Con molte Call vado in TimOut)
        // Con raddoppio del tempo Default problema risolto
        static CanardRxSubscription rx;
        const int8_t res =  //
            canardRxSubscribe(&state.canard,
                              CanardTransferKindRequest,
                              uavcan_register_List_1_0_FIXED_PORT_ID_,
                              uavcan_register_List_Request_1_0_EXTENT_BYTES_,
                              CANARD_REGISTERLIST_TRANSFER_ID_TIMEOUT_USEC,
                              &rx);
        if (res < 0) NVIC_SystemReset();
    }

    // Service servers: -> Risposta per dati e metadati sensore modulo corrente da master (Yakut, Altri)
    {
        static CanardRxSubscription rx;
        const int8_t res =  //
            canardRxSubscribe(&state.canard,
                              CanardTransferKindRequest,
                              state.port_id.service.module_th,
                              rmap_service_module_TH_Request_1_0_EXTENT_BYTES_,
                              CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
                              &rx);
        if (res < 0) NVIC_SystemReset();
    }

    // Service client: -> Risposta per Read (Receive) File local richiesta esterna (Yakut, Altri)
    {
        static CanardRxSubscription rx;
        const int8_t                res =  //
            canardRxSubscribe(&state.canard,
                                CanardTransferKindResponse,
                                uavcan_file_Read_1_1_FIXED_PORT_ID_,
                                uavcan_file_Read_Response_1_1_EXTENT_BYTES_,
                                CANARD_READFILE_TRANSFER_ID_TIMEOUT_USEC,
                                &rx);
        if (res < 0) NVIC_SystemReset();
    }

    // ********************************************************************************
    //         AVVIA LOOP CANARD PRINCIPALE gestione TX/RX Code -> Messaggi
    // ********************************************************************************

    // TODO: Eliminare
    bool ledShow;
    int bTxAttempt = 0;
    int bRxAttempt = 0;
    long lastMillis = 0;
    long checkTimeout = 0;
    bool bEventRealTimeLoop = false;

    #define MILLIS_EVENT 10
    #define PUBLISH_HEARTBEAT
    #define PUBLISH_LISTPORT
    // #define LED_ON_CAN_DATA_TX
    // #define LED_ON_CAN_DATA_RX
    #define LED_ON_SYNCRO_TIME
    #define LOG_RX_PACKET

    // Set START Timetable LOOP RX/TX.
    state.started_at                            = getMonotonicMicroseconds(&state);
    const CanardMicrosecond fast_loop_period    = MEGA / 3;
    CanardMicrosecond       next_333_ms_iter_at = state.started_at + fast_loop_period;
    CanardMicrosecond       next_01_sec_iter_at = state.started_at + MEGA;
    CanardMicrosecond       next_20_sec_iter_at = state.started_at + MEGA * 1.5;

    do {
        // Run a trivial scheduler polling the loops that run the business logic.
        CanardMicrosecond monotonic_time = getMonotonicMicroseconds(&state);
        // monotonic_time.

        // Check TimeLine (quasi RealTime...) Simulo Task a Timer
        // Gestione eventi ogni millisEvent mSec -> bEventRealTimeLoop
        if ((millis()-checkTimeout) >= MILLIS_EVENT)
        {
            // Deadline di controllo per eventi di controllo Rapidi (TimeOut, FileHandler ecc...)
            // Mancata risposta, nodo in Errore o Full o CanardHeapError ecc...
            checkTimeout = millis();
            // Utilizzo per eventi quasi continuativi... Es. Send/Receive File queue...
            bEventRealTimeLoop = true;
        }

        // TEST VERIFICA sincronizzazione time_stamp locale con remoto... (LED sincronizzati)
        // Test con reboot e successiva risincronizzazione automatica (FAKE RTC!!!!)
        #ifdef LED_ON_SYNCRO_TIME
        // TODO Eliminare e passare a RTC
        // Simulo incremento di millisEvent in microsecondi per FAKE RTC
        state.master.timestamp.synchronized += monotonic_time - state.master.timestamp.synchronized_last_update;
        state.master.timestamp.synchronized_last_update = monotonic_time;
        // Verifico LED al secondo... su timeSTamp sincronizzato remoto
        if((state.master.timestamp.synchronized / 1000000) % 2) {
            digitalWrite(LED_BUILTIN, HIGH);
        } else {
            digitalWrite(LED_BUILTIN, LOW);
        }
        #endif

        // ************************************************************************
        // ***********   CHECK OFFLINE/DEADLINE TIMEOUT COMMAND/STATE   ***********
        // ************************************************************************
        // TEST Check ogni RTL circa ( SOLO TEST COMANDI DA INSERIRE IN TASK_TIME )
        if (bEventRealTimeLoop)
        {
            // **********************************************************
            //           Per il nodo locale MASTER node_flag
            // **********************************************************
            // Controllo TimeOut Comando file su modulo remoto
            if(state.master.file.is_pending) {
                if(monotonic_time > state.master.file.timeout_us) {
                    // Setto il flag di TimeOUT che il master dovrà gestire (segnalazione BUG al Server?)
                    state.master.file.is_timeout = true;
                }
            }
            // Check eventuale Nodo Master OFFLINE (Ultimo comando sempre perchè posso)
            // Effettuare eventuali operazioni di SET,RESET Cmd in sicurezza
            if (monotonic_time > state.master.heartbeat.timeout_us) {
                // Entro in OffLine ed eseguo eventuali operazioni di entrata
                // N.B. In Heartbeat è stata fatta la cosa contraria, per gestire il ritorno OnLine
                if (state.master.is_online) {
                    // Il master è entrato in modalità OFFLine
                    state.master.is_online = false;
                    // Attività x Reset e/o riavvio Nodo dopo OnLine -> Offline
                    // Solo TEST Locale TODO: da eliminare
                    Serial.println(F("Master controller OFFLINE !!! Alert"));
                }
            }
        }

        // **************************************************************************
        //            STATO MODULO (Decisionale in funzione di stato remoto)
        // Gestione continuativa del modulo di acquisizione master.state (flag remoto)
        // **************************************************************************
        if (state.master.is_online) {
            // TODO: SOLO CODICE DI ESEMPIO DA GESTIRE
            // Il master comunica nell'HeartBeat il proprio stato che viene gestito qui se OnLine
            // Gestione attività (es. risparmio energetico, altro in funzione di codice remoto)
            /*
            switch (state.master.state) {
                case 01:
                    // Risparmio energetico
                    // -> Entro in risparmio energetico
                    break;
                case 02:
                    // Altro...
                    // -> Eseguo altro
                    break;
                default:
                    // Normale gestione, o non definita
            }
            */
        }
        else
        {
            // Gestisco situazione con Master OFFLine...
        }

        // **************************************************************************
        //        CANARD UAVCAN    Gestione procedure, retry e messaggi di rete
        // **************************************************************************

        // -> Scheduler realtime delle attività in corso (file download, altre...)

        // FILE HANDLER ( con controllo continuativo se avviato bEventRealTimeLoop )
        // Procedura di gestione ricezione file
        if(bEventRealTimeLoop)
        {
            // Verifica TimeOUT Occurs per File download
            if(state.master.file.is_timeout) {
                Serial.println(F("Time OUT File... reset state"));
                state.master.file.is_pending = false;
                state.master.file.is_timeout = false;
                // Gestita eventuale Retry N Volte per poi abbandonare
                state.master.file.updating_retry++;
                if(state.master.file.updating_retry < NODE_GETFILE_MAX_RETRY) {
                    Serial.print(F("Next Retry File read: "));
                    Serial.println(state.master.file.updating_retry);
                } else {
                    Serial.println(F("MAX Retry File occurs"));
                    if(state.master.file.is_firmware) {
                        Serial.println(F("Uprgading FIRMWARE ABORT!!!"));
                    } else {
                        Serial.println(F("Uprgading FILE ABORT!!!"));
                    }
                    state.master.file.updating = false;
                }
            }
            // Verifica file download in corso (entro se in download)
            if(state.master.file.updating) {
                // Se messaggio in pending non faccio niente è attendo la conferma del ResetPending
                // In caso di errore subentrerà il TimeOut e verrà essere gestita la retry
                if(!state.master.file.is_pending) {
                    // Fine pending, con messaggio OK. Verifico se EOF o necessario altro blocco
                    if(state.master.file.updating_eof) {
                        if(state.master.file.is_firmware) {
                            Serial.println(F("RX FIRMWARE COMPLETED !!!"));
                        } else {
                            Serial.println(F("RX FILE COMPLETED !!!"));
                        }
                        Serial.println(state.master.file.filename);
                        Serial.print(F("Size: "));
                        Serial.print(getDataFileInfo(state.master.file.filename, state.master.file.is_firmware));
                        Serial.println(F(" (bytes)"));
                        // Nessun altro evento necessario, chiudo File e stati
                        // procedo all'aggiornamento Firmware dopo le verifiche di conformità
                        // Ovviamente se si tratta di un file firmware
                        state.master.file.updating = false;
                        // Comunico a HeartBeat (Yakut o Altri) l'avvio dell'aggiornamento (se il file è un firmware...)
                        // Per Yakut Pubblicare un HeartBeat prima dell'Avvio quindi con il flag
                        // state->local_node.file.updating_run = true >> HeartBeat Counica Upgrade...
                        if(state.master.file.is_firmware) {
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

        // -> Scheduler temporizzato dei messaggi standard da inviare alla rete UAVCAN 

        // LOOP HANDLER >> FAST << 333 msec
        // Led Running + Publish Data se abilitato
        if (monotonic_time >= next_333_ms_iter_at)
        {
            next_333_ms_iter_at += fast_loop_period;
            handleFastLoop(&state, monotonic_time);
        }

        // LOOP HANDLER >> 1 SECONDO << HEARTBEAT
        if (monotonic_time >= next_01_sec_iter_at) {
            #ifdef PUBLISH_HEARTBEAT
            Serial.println(F("Publish SLAVE Heartbeat -->> [1 sec]"));
            #endif
            next_01_sec_iter_at += MEGA;
            handleNormalLoop(&state, monotonic_time);
        }

        // LOOP HANDLER >> 20 SECONDI PUBLISH SERVIZI <<
        if (monotonic_time >= next_20_sec_iter_at) {
            #ifdef PUBLISH_LISTPORT
            Serial.println(F("Publish Local PORT LIST -->> [20 sec]"));
            #endif
            next_20_sec_iter_at += MEGA * 20;
            handleSlowLoop(&state, monotonic_time);
        }

        // Fine handler quasi RealTime...
        // Attendo nuovo evento per rielaborare
        // Utilizzato per blinking Led (TX/RX)
        if(bEventRealTimeLoop) {
            bEventRealTimeLoop = false;
            #if defined(LED_ON_CAN_DATA_TX) or defined(LED_ON_CAN_DATA_RX)
            digitalWrite(LED_BUILTIN, LOW);
            #endif
        };

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
                #ifdef LED_ON_CAN_DATA_TX
                digitalWrite(LED_BUILTIN, HIGH);
                #endif
                // Delay Microsecond di sicurezza in Send (Migliora sicurezza RX Pacchetti)
                // Da utilizzare con CPU poco performanti in RX o con controllo Polling gestito Canard
                #if (CAN_DELAY_MS_SEND > 0)
                delayMicroseconds(CAN_DELAY_US_SEND);
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
            #ifdef LED_ON_CAN_DATA_RX
            digitalWrite(LED_BUILTIN, HIGH);
            #endif
            // Leggo l'elemento disponibile in coda BUFFER RX FiFo CanardFrame + Buffer
            byte getElement = bxCANRxQueueNextElement(canard_rx_queue.rd_ptr);
            canard_rx_queue.rd_ptr = getElement;
            // Log Packet
            #ifdef LOG_RX_PACKET
            Serial.print(canard_rx_queue.msg[getElement].frame.payload_size);
            Serial.print(F(",Val: "));
            for(int iIdxPl=0; iIdxPl<canard_rx_queue.msg[getElement].frame.payload_size; iIdxPl++) {
                Serial.print("0x");
                Serial.print(canard_rx_queue.msg[getElement].buf[iIdxPl] < 16 ? "0" : "");
                Serial.print(canard_rx_queue.msg[getElement].buf[iIdxPl], HEX);
                Serial.print(" ");
            }
            Serial.println("");
            #endif
            // Passaggio CanardFrame Buffered alla RxAccept CANARD
            // DeadLine a partire da getMonotonicMicroseconds() realTime assoluto
            const CanardMicrosecond timestamp_usec = getMonotonicMicroseconds(&state);
            CanardRxTransfer        transfer;
            const int8_t canard_result = canardRxAccept(&state.canard, timestamp_usec, &canard_rx_queue.msg[getElement].frame, IFACE_CAN_IDX, &transfer, NULL);
            if (canard_result > 0)
            {
                processReceivedTransfer(&state, &transfer);
                state.canard.memory_free(&state.canard, (void*) transfer.payload);
            }
            else if ((canard_result == 0) || (canard_result == -CANARD_ERROR_OUT_OF_MEMORY))
            {
                (void) 0;  // The frame did not complete a transfer so there is nothing to do.
                // OOM should never occur if the heap is sized correctly. You can track OOM errors via heap API.
            }
            else
            {
                assert(false);  // No other error can possibly occur at runtime.
            }
        }
    } while (!state.flag.g_restart_required);

    // Reboot
    NVIC_SystemReset();
}