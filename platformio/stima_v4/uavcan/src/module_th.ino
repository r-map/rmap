/// This software is distributed under the terms of the MIT License.
/// Copyright (C) 2021 OpenCyphal <maintainers@opencyphal.org>
/// Author: Pavel Kirienko <pavel@opencyphal.org>
/// Revis.: Gasperini Moreno <m.gasperini@digiteco.it>

/// TODO: Integrazione logica con sensordrive ( Vedere con Marco e Paolo )
/// TODO: BUG rilevato, loop continuo per TIME_OUT in rxSubscribe txCanard > TIME_OUT impostato!!!!
///         -> SISTEMARE CON DEBUG E TEST !!!!!!
/// TODO: Tutti i comandi da inserire PREPARE,GET altro quello che sereve...
/// TODO: Corretta DSDL Finale + updateSensorsData
/// TODO: bxCanInterrupt + Filters
/// TODO: Inserire disturbatore HW e verifiche restore, reset ecc...
/// TODO: C++/strutture
/// TODO: CanardMicrosecond

// Arduino
#include <Arduino.h>
// Libcanard
#include "register.hpp"
#include <o1heap.h>
#include <canard.h>
#include "bxcan.h"
// Namespace UAVCAN
#include <uavcan/_register/Access_1_0.h>
#include <uavcan/_register/List_1_0.h>
#include <uavcan/node/ExecuteCommand_1_1.h>
#include <uavcan/node/GetInfo_1_0.h>
#include <uavcan/node/Heartbeat_1_0.h>
#include <uavcan/node/port/List_0_1.h>
#include <uavcan/pnp/NodeIDAllocationData_1_0.h>
#include <uavcan/pnp/NodeIDAllocationData_2_0.h>
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

// INIT REGISTER INTERNI x TEST E/O SETUP RAPIDO
// #define INIT_REGISTER

// VARIABILI GLOBALI ISTANZE, TODO: CLASSE c++ ecc...

// Modulo client corrente (Publish)
rmap_module_TH_1_0 module_th_msg = {0};
// Modulo client corrente (Service)
rmap_service_module_TH_Request_1_0 module_th_rqst = {0};
rmap_service_module_TH_Response_1_0 module_th_resp = {0};

/// We keep the state of the application here. Feel free to use static variables instead if desired.
typedef struct State {
    CanardMicrosecond started_at;

    O1HeapInstance* heap;
    CanardInstance canard;
    CanardTxQueue canard_tx_queues[CAN_REDUNDANCY_FACTOR];

    // Subject ID porte e servizi modulo locale
    struct
    {
        struct
        {
            CanardPortID module_th;
        } pub;
        struct
        {
            CanardPortID service_module_th;
        } srv;
    } port_id;

    // Abilitazione delle pubblicazioni falcoltative sulla rete (ON/OFF a richiesta)
    struct
    {
        bool module_th;
        bool port_list;
    } pub_enabled;

    /// A transfer-ID is an integer that is incremented whenever a new message is published on a given subject.
    /// It is used by the protocol for deduplication, message loss detection, and other critical things.
    /// For CAN, each value can be of type uint8_t, but we use larger types for genericity and for statistical purposes,
    /// as large values naturally contain the number of times each subject was published to.
    /// Tip: messages published synchronously can share the same transfer-ID.
    struct
    {
        uint64_t uavcan_node_heartbeat;
        uint64_t uavcan_node_port_list;
        uint64_t uavcan_pnp_allocation;
        uint64_t module_th;
    } next_transfer_id;
} State;

// Flag per Reboot del nodo.
static volatile bool g_restart_required = false;

// ***************************************************************************************************
// **********             Funzioni ed utility generiche per gestione UAVCAN                 **********
// ***************************************************************************************************

/// A deeply embedded system should sample a microsecond-resolution non-overflowing 64-bit timer.
/// Here is a simple non-blocking implementation as an example:
/// https://github.com/PX4/sapog/blob/601f4580b71c3c4da65cc52237e62a/firmware/src/motor/realtime/motor_timer.c#L233-L274
/// Mind the difference between monotonic time and wall time. Monotonic time never changes rate or makes leaps,
/// it is therefore impossible to synchronize with an external reference. Wall time can be synchronized and therefore
/// it may change rate or make leap adjustments. The two kinds of time serve completely different purposes.
// TODO: Microsecond register STM32... Microsecond non va bene perchè si resetta mentre
// Per Cypal non dovrebbe resettarsi uint_64 Bit
static CanardMicrosecond getMonotonicMicroseconds() {
    time_t ts;
    ts = (uint64_t)millis() * KILO;
    return (uint64_t)ts;
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
static void sendResponse(State* const state,
                         const CanardMicrosecond tx_deadline_usec,
                         const CanardTransferMetadata* const request_metadata,
                         const size_t payload_size,
                         const void* const payload) {
    CanardTransferMetadata meta = *request_metadata;
    meta.transfer_kind = CanardTransferKindResponse;
    send(state, tx_deadline_usec, &meta, payload_size, payload);
}

// *******              FUNZIONI INVOCATE HANDLE FAST_LOOP EV. PREPARATORIE              *********

// Prepara il blocco messaggio dati per il modulo corrente
static void updateSensorsData(void) {
    // TODO: Tutto da fare a servizio
    module_th_resp.dataandmetadata.temperature.val.value = (int32_t)(rand() % 2000 + 27315);  // TODO: sample data from the real sensor.
    module_th_resp.dataandmetadata.temperature.confidence.value = (uint8_t)(rand() % 100);    // TODO: sample data from the real sensor.
    module_th_resp.dataandmetadata.humidity.val.value = (int32_t)(rand() % 100);              // TODO: sample data from the real sensor.
    module_th_resp.dataandmetadata.humidity.confidence.value = (uint8_t)(rand() % 100);       // TODO: sample data from the real sensor.
    // TODO: a pubblicazione inserire i dati corretti
    module_th_msg.temperature.val.value = (int32_t)(rand() % 2000 + 27315);  // TODO: sample data from the real sensor.
    module_th_msg.temperature.confidence.value = (uint8_t)(rand() % 100);    // TODO: sample data from the real sensor.
    module_th_msg.humidity.val.value = (int32_t)(rand() % 100);              // TODO: sample data from the real sensor.
    module_th_msg.humidity.confidence.value = (uint8_t)(rand() % 100);       // TODO: sample data from the real sensor.
}

/// Invoked at the rate of the fastest loop.
static void handleFastLoop(State* const state, const CanardMicrosecond monotonic_time) {
    // Controlla corretta assegnazione node_id
    const bool anonymous = state->canard.node_id > CANARD_NODE_ID_MAX;
    // Pubblica i dati del nodo corrente se abilitata la funzione e con il corretto subjectId
    // Ovviamente il nodo non può essere anonimo per la pubblicazione...
    if ((!anonymous) &&
        (state->pub_enabled.module_th) &&
        (state->port_id.pub.module_th <= CANARD_SUBJECT_ID_MAX)) {
        // Aggiorna i sensori TODO:
        updateSensorsData();
        // Serialize and publish the message:
        uint8_t serialized[rmap_module_TH_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
        size_t serialized_size = sizeof(serialized);
        const int8_t err = rmap_module_TH_1_0_serialize_(&module_th_msg, &serialized[0], &serialized_size);
        assert(err >= 0);
        if (err >= 0) {
            const CanardTransferMetadata meta = {
                .priority = CanardPriorityHigh,
                .transfer_kind = CanardTransferKindMessage,
                .port_id = state->port_id.pub.module_th,
                .remote_node_id = CANARD_NODE_ID_UNSET,
                .transfer_id = (CanardTransferID)state->next_transfer_id.module_th++,  // Increment!
            };
            send(state, monotonic_time + 10 * KILO, &meta, serialized_size, &serialized[0]);
        }
    }
}

// *******              FUNZIONI INVOCATE HANDLE 1 SECONDO EV. PREPARATORIE              *********

static void handle1HzLoop(State* const state, const CanardMicrosecond monotonic_time) {
    const bool anonymous = state->canard.node_id > CANARD_NODE_ID_MAX;
    // Heartbeat ogni secondo (la funzione è richiesta e non può essere mascherata in UAVCAN)
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
        // TODO: Inserire quà specifici codici per gestione client/server
        // heartbeat.vendor_specific_status_code -> = XX
        // Esempio acquisizione: dati (tempo acquisizione finito... pronto a prelievo dati)
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
                 monotonic_time + MEGA,  // Set transmission deadline 1 second, optimal for heartbeat.
                 &meta,
                 serialized_size,
                 &serialized[0]);
        }
    } else  // If we don't have a node-ID, obtain one by publishing allocation request messages until we get a response.
    {
        // The Specification says that the allocation request publication interval shall be randomized.
        // We implement randomization by calling rand() at fixed intervals and comparing it against some threshold.
        // There are other ways to do it, of course. See the docs in the Specification or in the DSDL definition here:
        // https://github.com/OpenCyphal/public_regulated_data_types/blob/master/uavcan/pnp/8165.NodeIDAllocationData.2.0.dsdl
        // Note that a high-integrity/safety-certified application is unlikely to be able to rely on this feature.
        #ifdef PNP_NODE_ALLOCATION_V_10
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
        #endif
        #ifdef PNP_NODE_ALLOCATION_V_20
        if (rand() > RAND_MAX / 2)  // NOLINT
        {
            // PnP CAN FD, use message v2.0.
            uavcan_pnp_NodeIDAllocationData_2_0 msg = {0};
            msg.node_id.value = UINT16_MAX;
            getUniqueID(msg.unique_id);
            uint8_t serialized[uavcan_pnp_NodeIDAllocationData_2_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
            size_t serialized_size = sizeof(serialized);
            const int8_t err = uavcan_pnp_NodeIDAllocationData_2_0_serialize_(&msg, &serialized[0], &serialized_size);
            assert(err >= 0);
            if (err >= 0) {
                const CanardTransferMetadata meta = {
                    .priority = CanardPrioritySlow,
                    .transfer_kind = CanardTransferKindMessage,
                    .port_id = uavcan_pnp_NodeIDAllocationData_2_0_FIXED_PORT_ID_,
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
        #endif
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
static void handle01HzLoop(State* const state, const CanardMicrosecond monotonic_time) {
    // Publish the recommended (not required) port introspection message. No point publishing it if we're anonymous.
    // The message is a bit heavy on the stack (about 2 KiB) but this is not a problem for a modern MCU.
    // L'abilitazione del comando è facoltativa, può essere attivata/disattivata da un comando UAVCAN
    if ((state->pub_enabled.port_list) &&
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
            // Aggiungo i publisher interni validi
            if (state->port_id.pub.module_th <= CANARD_SUBJECT_ID_MAX)
            {
                m.publishers.sparse_list.elements[(*cnt)++].value = state->port_id.pub.module_th;
            }            
        }

        // Indicate which servers and subscribers we implement.
        // We could construct the list manually but it's easier and more robust to just query libcanard for that.
        fillSubscriptions(state->canard.rx_subscriptions[CanardTransferKindMessage], &m.subscribers);
        fillServers(state->canard.rx_subscriptions[CanardTransferKindRequest], &m.servers);
        fillServers(state->canard.rx_subscriptions[CanardTransferKindResponse], &m.clients);  // For regularity.

        // Serialize and publish the message. Use a small buffer because we know that our message is always small.
        uint8_t serialized[512] = {0};  // https://github.com/OpenCyphal/nunavut/issues/191
        size_t serialized_size = uavcan_node_port_List_0_1_SERIALIZATION_BUFFER_SIZE_BYTES_;
        if (uavcan_node_port_List_0_1_serialize_(&m, &serialized[0], &serialized_size) >= 0) {
            const CanardTransferMetadata meta = {
                .priority = CanardPriorityOptional,  // Mind the priority.
                .transfer_kind = CanardTransferKindMessage,
                .port_id = uavcan_node_port_List_0_1_FIXED_PORT_ID_,
                .remote_node_id = CANARD_NODE_ID_UNSET,
                .transfer_id = (CanardTransferID)(state->next_transfer_id.uavcan_node_port_list++),
            };
            send(state, monotonic_time + MEGA, &meta, serialized_size, &serialized[0]);
        }
    }
}

// ***************************************************************************************************
//   Funzioni ed utility di ricezione dati dalla rete UAVCAN, richiamati da processReceivedTransfer()
// ***************************************************************************************************

#ifdef PNP_NODE_ALLOCATION_V_10
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
#endif
#ifdef PNP_NODE_ALLOCATION_V_20
// Plug and Play Slave, Versione 2.0 solo CAN_FD MTU 64
static void processMessagePlugAndPlayNodeIDAllocation(State* const state,
                                                      const uavcan_pnp_NodeIDAllocationData_2_0* const msg) {
    uint8_t uid[uavcan_node_GetInfo_Response_1_0_unique_id_ARRAY_CAPACITY_] = {0};
    getUniqueID(uid);
    if ((msg->node_id.value <= CANARD_NODE_ID_MAX) && (memcmp(uid, msg->unique_id, sizeof(uid)) == 0)) {
        printf("Got PnP node-ID allocation: %d\n", msg->node_id.value);
        state->canard.node_id = (CanardNodeID)msg->node_id.value;
        // Store the value into the non-volatile storage.
        uavcan_register_Value_1_0 reg = {0};
        uavcan_register_Value_1_0_select_natural16_(&reg);
        reg.natural16.value.elements[0] = msg->node_id.value;
        reg.natural16.value.count = 1;
        registerWrite("uavcan.node.id", &reg);
        // We no longer need the subscriber, drop it to free up the resources (both memory and CPU time).
        (void)canardRxUnsubscribe(&state->canard,
                                  CanardTransferKindMessage,
                                  uavcan_pnp_NodeIDAllocationData_2_0_FIXED_PORT_ID_);
    }
    // Otherwise, ignore it: either it is a request from another node or it is a response to another node.
}
#endif

// Chiamate gestioni RPC remote da master (yakut o altro servizio di controllo)
static uavcan_node_ExecuteCommand_Response_1_1 processRequestExecuteCommand(State* const state,
                                                                            const uavcan_node_ExecuteCommand_Request_1_1* req) {
    uavcan_node_ExecuteCommand_Response_1_1 resp = {0};
    // req->command (Comando esterno ricevuto 2 BYTES RESERVED FFFF-FFFA)
    // Gli altri sono liberi per utilizzo interno applicativo con #define interne
    // req->parameter (array di byte MAX 255 per i parametri da request)
    // Risposta attuale (resp) 1 Bytes RESERVER (0..6) gli altri #define interne
    switch (req->command) {
        // Case standard UAVCAN
        case uavcan_node_ExecuteCommand_Request_1_1_COMMAND_BEGIN_SOFTWARE_UPDATE: {
            char file_name[uavcan_node_ExecuteCommand_Request_1_1_parameter_ARRAY_CAPACITY_ + 1] = {0};
            memcpy(file_name, req->parameter.elements, req->parameter.count);
            file_name[req->parameter.count] = '\0';
            // TODO: invoke the bootloader with the specified file name. See https://github.com/Zubax/kocherga/
            printf("Firmware update request; filename: '%s' \n", &file_name[0]);
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_BAD_STATE;  // This is a stub.
            break;
        }
        case uavcan_node_ExecuteCommand_Request_1_1_COMMAND_FACTORY_RESET: {
            registerDoFactoryReset();
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        case uavcan_node_ExecuteCommand_Request_1_1_COMMAND_RESTART: {
            g_restart_required = true;
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
        // CASE private comandi locali
        case CMD_ENABLE_PUBLISH_DATA: {
            // Abilita pubblicazione fast_loop data_and_metadata modulo locale (test yakut e user master)
            state->pub_enabled.module_th = true;
            break;
        }
        case CMD_DISABLE_PUBLISH_DATA: {
            // Disabilita pubblicazione fast_loop data_and_metadata modulo locale (test yakut e user master)
            state->pub_enabled.module_th = false;
            break;
        }
        case CMD_ENABLE_PUBLISH_PORT_LIST: {
            // Abilita pubblicazione slow_loop elenco porte (Cypal facoltativo)
            state->pub_enabled.port_list = true;
            break;
        }
        case CMD_DISABLE_PUBLISH_PORT_LIST:{
            // Disabilita pubblicazione slow_loop elenco porte (Cypal facoltativo)
            state->pub_enabled.port_list = false;
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
    if (transfer->metadata.transfer_kind == CanardTransferKindMessage) {
        size_t size = transfer->payload_size;
        #ifdef PNP_NODE_ALLOCATION_V_10
        if (transfer->metadata.port_id == uavcan_pnp_NodeIDAllocationData_1_0_FIXED_PORT_ID_) {
            uavcan_pnp_NodeIDAllocationData_1_0 msg = {0};
            if (uavcan_pnp_NodeIDAllocationData_1_0_deserialize_(&msg, static_cast<uint8_t const*>(transfer->payload), &size) >= 0) {
                processMessagePlugAndPlayNodeIDAllocation(state, &msg);
            }
        }
        #endif
        #ifdef PNP_NODE_ALLOCATION_V_20
        if (transfer->metadata.port_id == uavcan_pnp_NodeIDAllocationData_2_0_FIXED_PORT_ID_) {
            uavcan_pnp_NodeIDAllocationData_2_0 msg = {0};
            if (uavcan_pnp_NodeIDAllocationData_2_0_deserialize_(&msg, static_cast<uint8_t const*>(transfer->payload), &size) >= 0) {
                processMessagePlugAndPlayNodeIDAllocation(state, &msg);
            }
        }
        #endif
        else {
            // Gestione di un messaggio senza sottoscrizione. Se arrivo quà è un errore di sviluppo
            assert(false);
        }
    } else if (transfer->metadata.transfer_kind == CanardTransferKindRequest) {
        if (transfer->metadata.port_id == state->port_id.srv.service_module_th) {
        // if (transfer->metadata.port_id == state->module_th_port_id) {
            // The request object is empty so we don't bother deserializing it. Just send the response.

            updateSensorsData();
            module_th_resp.stato = 1;

            // Serialize and publish the message:
            uint8_t serialized[rmap_service_module_TH_Response_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
            size_t serialized_size = sizeof(serialized);
            const int8_t res = rmap_service_module_TH_Response_1_0_serialize_(&module_th_resp, &serialized[0], &serialized_size);
            //const int8_t res = rmap_service_module_TH_serialize_(&module_th_msg, &serialized[0], &serialized_size);
            if (res >= 0) {
                // Send the response back. Make sure to re-use the same priority and transfer-ID.
                // todo
                // transfer->metadata.transfer_kind  = CanardTransferKindResponse;
                sendResponse(state,
                             transfer->timestamp_usec + MEGA,
                             &transfer->metadata,
                             serialized_size,
                             &serialized[0]);
            }
        } else if (transfer->metadata.port_id == uavcan_node_GetInfo_1_0_FIXED_PORT_ID_) {
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
        } else if (transfer->metadata.port_id == uavcan_node_ExecuteCommand_1_1_FIXED_PORT_ID_) {
            uavcan_node_ExecuteCommand_Request_1_1 req = {0};
            size_t size = transfer->payload_size;
            if (uavcan_node_ExecuteCommand_Request_1_1_deserialize_(&req, static_cast<uint8_t const*>(transfer->payload), &size) >= 0) {
                const uavcan_node_ExecuteCommand_Response_1_1 resp = processRequestExecuteCommand(state, &req);
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
        } else {
            // Gestione di una richiesta senza controllore locale. Se arrivo quà è un errore di sviluppo
            assert(false);
        }
    } else {
        // Se arrivo quà è un errore di sviluppo, controllare setup sottoscrizioni
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

extern char** environ;

// Setup HW (PIN, interface, filter, baud)
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
    Serial.begin(9600);
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
void loop(void) {
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
    state.pub_enabled.port_list = true;
    state.pub_enabled.module_th = true;
     
    // ********************************************************************************
    // FIXED REGISTER_INIT TODO: DA FINIRE, FARE INIT x REGISTRI FISSI ECC. E/O INVAR.
    // ********************************************************************************
    #ifdef INIT_REGISTER

    // CANARD_MTU -> CLASSIC CAN 3xFIFO 8 x SINGLE CAN
    uavcan_register_Value_1_0_select_natural16_(&val);
    val.natural16.value.count = 1;
    val.natural16.value.elements[0] = CANARD_MTU_CAN_CLASSIC;
    registerWrite("uavcan.can.mtu", &val);

    // CANARD_BITRATE -> INIT_VALUE + Extra (NOT USED, CAN_MTU_CLASSIC)
    uavcan_register_Value_1_0_select_natural32_(&val);
    val.natural32.value.count = 2;
    val.natural32.value.elements[0] = CAN_BIT_RATE;
    val.natural32.value.elements[1] = 0ul;  // Ignored for CANARD_MTU_CAN_CLASSIC
    registerWrite("uavcan.can.bitrate", &val);

    // Node Name description (Platformio.ini)
    uavcan_register_Value_1_0_select_string_(&val);
    val._string.value.count = sizeof(NODE_NAME);
    strcpy((char*)val._string.value.elements, NODE_NAME);
    registerWrite("uavcan.node.description", &val);  // We don't need the value, we just need to ensure it exists.

    #ifdef NODE_SLAVE_ID
    // Restore the node-ID from the corresponding standard register. Default to anonymous.
    uavcan_register_Value_1_0_select_natural16_(&val);
    val.natural16.value.count = 1;
    val.natural16.value.elements[0] = NODE_SLAVE_ID;  // Fixed NodeID, per Specification/libcanard.
    registerWrite("uavcan.node.id", &val);            // The names of the standard registers are regulated by the Specification.
    #endif

    #endif

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

    // Carico i/il port-ID/subject-ID del modulo locale dai registri relativi associati nel namespace UAVCAN
    state.port_id.pub.module_th =
        getModeAccessID(PublisherSubjectID, "TH.data_and_metadata",
                              rmap_module_TH_1_0_FULL_NAME_AND_VERSION_);

    state.port_id.srv.service_module_th =
        getModeAccessID(ServicePortID, "TH.service_data_and_metadata",
                              rmap_service_module_TH_1_0_FULL_NAME_AND_VERSION_);

    // Lettura dei registri RMAP al modulo corrente, con impostazione di default x Startup/Init value

    // Lettura dei registri RMAP 32 Bit relativi al modulo corrente
    uavcan_register_Value_1_0_select_natural32_(&val);
    val.natural32.value.count = 1;
    val.natural32.value.elements[0] = UINT32_MAX;
    registerRead("rmap.module.TH.metadata.Level.L1", &val);
    module_th_msg.metadata.level.L1.value = val.natural32.value.elements[0];
    registerRead("rmap.module.TH.metadata.Level.L2", &val);
    module_th_msg.metadata.level.L2.value = val.natural32.value.elements[0];
    registerRead("rmap.module.TH.metadata.Timerange.P1", &val);
    module_th_msg.metadata.timerange.P1.value = val.natural32.value.elements[0];
    registerRead("rmap.module.TH.metadata.Timerange.P2", &val);
    module_th_msg.metadata.timerange.P2.value = val.natural32.value.elements[0];

    // Lettura dei registri RMAP 8 Bit relativi al modulo corrente
    uavcan_register_Value_1_0_select_natural8_(&val);
    val.natural16.value.count = 1;
    val.natural16.value.elements[0] = UINT8_MAX;
    registerRead("rmap.module.TH.metadata.Timerange.Pindicator", &val);
    module_th_msg.metadata.timerange.Pindicator.value = val.natural8.value.elements[0];
    registerRead("rmap.module.TH.metadata.Level.LevelType1", &val);
    module_th_msg.metadata.level.LevelType1.value = val.natural8.value.elements[0];
    registerRead("rmap.module.TH.metadata.Level.LevelType2", &val);
    module_th_msg.metadata.level.LevelType2.value = val.natural8.value.elements[0];

    // ********************************************************************************
    //               AVVIA SOTTOSCRIZIONI ai messaggi per servizi RPC ecc...
    // ********************************************************************************

    // Plug and Play Versione 1_0 CAN_CLASSIC
    #ifdef PNP_NODE_ALLOCATION_V_10
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
    #endif
    // Plug and Play Versione 2_0 CAN_FD
    #ifdef PNP_NODE_ALLOCATION_V_20
    {  // PnP over CAN FD, use message v2.0.
        static CanardRxSubscription rx;
        const int8_t res =  //
            canardRxSubscribe(&state.canard,
                              CanardTransferKindMessage,
                              uavcan_pnp_NodeIDAllocationData_2_0_FIXED_PORT_ID_,
                              uavcan_pnp_NodeIDAllocationData_2_0_EXTENT_BYTES_,
                              CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
                              &rx);
        if (res < 0) {
            NVIC_SystemReset();
            // return -res;
        }
    }
    #endif

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
                              state.port_id.srv.service_module_th,
                              rmap_service_module_TH_Request_1_0_EXTENT_BYTES_,
                              CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
                              &rx);
        if (res < 0) NVIC_SystemReset();
    }

    // ********************************************************************************
    //         AVVIA LOOP CANARD PRINCIPALE gestione TX/RX Code -> Messaggi
    // ********************************************************************************

    // TODO: Eliminare
    digitalWrite(LED_BUILTIN, LOW);
    bool ledShow;
    int bTxAttempt = 0;
    int bRxAttempt = 0;
    long lastMillis;

    // Set START Timetable LOOP RX/TX.
    state.started_at                            = getMonotonicMicroseconds();
    const CanardMicrosecond fast_loop_period    = MEGA / 4;
    CanardMicrosecond       next_250_ms_iter_at = state.started_at + fast_loop_period;
    CanardMicrosecond       next_01_sec_iter_at = state.started_at + MEGA;
    CanardMicrosecond       next_10_sec_iter_at = state.started_at + MEGA * 10;

    do {
        // Run a trivial scheduler polling the loops that run the business logic.
        CanardMicrosecond monotonic_time = getMonotonicMicroseconds();
        // monotonic_time.

        // ***************************************************************************
        // Scheduler temporizzato dei messaggi / comandi da inviare alla rete UAVCAN 
        // ***************************************************************************

        // LOOP HANDLER >> FAST <<
        if (monotonic_time >= next_250_ms_iter_at)
        {
            next_250_ms_iter_at += fast_loop_period;
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
            next_01_sec_iter_at += MEGA;
            handle1HzLoop(&state, monotonic_time);
        }

        // LOOP HANDLER >> 10 SECONDI <<
        if (monotonic_time >= next_10_sec_iter_at) {
            next_10_sec_iter_at += MEGA * 10;
            handle01HzLoop(&state, monotonic_time);
        }

        // ***************************************************************************
        //   Gestione Coda messaggi in trasmissione (ciclo di svuotamento messaggi)
        // ***************************************************************************
        // Transmit pending frames from the prioritized TX queues managed by libcanard.
        for (uint8_t ifidx = 0; ifidx < CAN_REDUNDANCY_FACTOR; ifidx++) {
            CanardTxQueue* const que = &state.canard_tx_queues[ifidx];
            const CanardTxQueueItem* tqi = canardTxPeek(que);  // Find the highest-priority frame.
            while (tqi != NULL) {
                // TODO: Remove test
                bTxAttempt++;
                if ((millis() - lastMillis) > 333) {
                    Serial.print("Attempt -> ");
                    Serial.println(bTxAttempt);
                    lastMillis = millis();
                }

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
                        // Push CAN data
                        state.canard.memory_free(&state.canard, canardTxPop(que, tqi));
                        tqi = canardTxPeek(que);
                    } else {
                        // Empty Queue
                        break;
                    }
                } else {
                    // BUG rilevato, loop continuo per mancato aggiornamento monotonic_time su
                    // grandi quantità di dati trasmesse e raggiunto il TIMEOUT Subscription...
                    // TODO: Sistemare coda peek & clear trasmission security BUG!!!
                    // CANARD_REGISTERLIST_TRANSFER_ID_TIMEOUT_USEC = 500000 E PROVA
                }
            }
        }

        // TODO: Interrupt RX...
        // ***************************************************************************
        //   Gestione Coda messaggi in ricezione (ciclo di caricamento messaggi)
        // ***************************************************************************
        for (uint8_t ifidx = 0; ifidx < CAN_REDUNDANCY_FACTOR; ifidx++) {
            CanardFrame frame;
            uint8_t buf[CANARD_MTU_MAX] = {0};

            frame.extended_can_id = 0;
            frame.payload_size = sizeof(buf);
            // The read operation has timed out with no frames, nothing to do here.
            if (bxCANPop(ifidx, &frame.extended_can_id, &frame.payload_size, buf)) {
                // Get payload from Buffer
                frame.payload = buf;

                // TODO: Remove test
                bRxAttempt++;
                if ((millis() - lastMillis) > 333) {
                    Serial.print("Attempt <- ");
                    Serial.println(bRxAttempt);
                    lastMillis = millis();
                }

                // The SocketCAN adapter uses the wall clock for timestamping, but we need monotonic.
                // Wall clock can only be used for time synchronization.
                const CanardMicrosecond timestamp_usec = getMonotonicMicroseconds();
                CanardRxTransfer transfer;
                const int8_t canard_result = canardRxAccept(&state.canard, timestamp_usec, &frame, ifidx, &transfer, NULL);
                if (canard_result > 0) {
                    processReceivedTransfer(&state, &transfer);
                    state.canard.memory_free(&state.canard, (void*)transfer.payload);
                } else if ((canard_result == 0) || (canard_result == -CANARD_ERROR_OUT_OF_MEMORY)) {
                    // TODO: Remove test
                    if ((millis() - lastMillis) > 333) {
                        Serial.print("Rx Nothing ToDo...");
                        lastMillis = millis();
                    }
                    (void)0;  // The frame did not complete a transfer so there is nothing to do.
                    // OOM should never occur if the heap is sized correctly. You can track OOM errors via heap API.
                } else {
                    assert(false);  // No other error can possibly occur at runtime.
                }
            } else
                break;
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