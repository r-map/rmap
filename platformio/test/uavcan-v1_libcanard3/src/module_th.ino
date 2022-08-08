///                            ____                   ______            __          __
///                           / __ `____  ___  ____  / ____/_  ______  / /_  ____  / /
///                          / / / / __ `/ _ `/ __ `/ /   / / / / __ `/ __ `/ __ `/ /
///                         / /_/ / /_/ /  __/ / / / /___/ /_/ / /_/ / / / / /_/ / /
///                         `____/ .___/`___/_/ /_/`____/`__, / .___/_/ /_/`__,_/_/
///                             /_/                     /____/_/
///
/// A demo application showcasing the implementation of a simple plug&play node
/// This application is intended to run on STM32 bluepill.
/// Please refer to the enclosed README for details.
///
/**********************************************************************
Copyright (C) 2022  Paolo Paruno <p.patruno@iperbole.bologna.it>
authors: Paolo Paruno <p.patruno@iperbole.bologna.it>
         Pavel Kirienko <pavel@opencyphal.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of 
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/

#include "register.h"
#include <o1heap.h>
#include <canard.h>
#include "canard_dsdl.h"
#include "bxcan.h"

#if (CANARD_MTU_MAX == CANARD_MTU_CAN_CLASSIC)
    #define PNP_NODE_ALLOCATION_V_10
#else
    #define PNP_NODE_ALLOCATION_V_20
#endif

#include <uavcan/node/Heartbeat_1_0.h>
#include <uavcan/node/GetInfo_1_0.h>
#include <uavcan/node/ExecuteCommand_1_1.h>
#include <uavcan/node/port/List_0_1.h>
#include <uavcan/_register/Access_1_0.h>
#include <uavcan/_register/List_1_0.h>

#ifdef PNP_NODE_ALLOCATION_V_10
#include <uavcan/pnp/NodeIDAllocationData_1_0.h>
#endif

#include <uavcan/pnp/NodeIDAllocationData_2_0.h>

// Use /sample/ instead of /unit/ if you need timestamping.
//#include <uavcan/si/unit/pressure/Scalar_1_0.h>
//#include <uavcan/si/unit/temperature/Scalar_1_0.h>

#include <rmap/_module/TH_1_0.h>
#include <rmap/service/_module/TH/GetDataAndMetadata_1_0.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <ArduinoLog.h>

/// By default, this macro resolves to the standard assert(). The user can redefine this if necessary.
/// To disable assertion checks completely, make it expand into `(void)(0)`.
#define RMAP_ASSERT(x) (void)(0)

//// Intentional violation of MISRA: inclusion not at the top of the file to eliminate unnecessary dependency on assert.h.
//#    include <assert.h>  // NOSONAR
//// Intentional violation of MISRA: assertion macro cannot be replaced with a function definition.
//#    define RMAP_ASSERT(x) assert(x)  // NOSONAR


#define KILO 1000L
#define MEGA ((int64_t) KILO * KILO)

#define CAN_REDUNDANCY_FACTOR 1
/// For CAN FD the queue can be smaller.
#define CAN_TX_QUEUE_CAPACITY 100

rmap_module_TH_1_0 module_th_msg = {0};

CanardInstance canard;
//HardwareSerial Serial2(PA3, PA2);  //uart2
unsigned long int  next;
unsigned long int  nextrpc;


/// We keep the state of the application here. Feel free to use static variables instead if desired.
typedef struct State
{
    CanardMicrosecond started_at;

    O1HeapInstance* heap;
    CanardInstance  canard;
    CanardTxQueue   canard_tx_queues[CAN_REDUNDANCY_FACTOR];

    /// These values are read from the registers at startup. You can also implement hot reloading if desired.
    struct
    {
        struct
        {
            CanardPortID module_th;
            CanardPortID service_module_th;
        } pub;
    } port_id;

    /// A transfer-ID is an integer that is incremented whenever a new message is published on a given subject.
    /// It is used by the protocol for deduplication, message loss detection, and other critical things.
    /// For CAN, each value can be of type uint8_t, but we use larger types for genericity and for statistical purposes,
    /// as large values naturally contain the number of times each subject was published to.
    struct
    {
        uint64_t uavcan_node_heartbeat;
        uint64_t uavcan_node_port_list;
        uint64_t uavcan_pnp_allocation;
        // Tip: messages published synchronously can share the same transfer-ID.
        uint64_t module_th;
        //uint64_t service_module_th;  // not required
    } next_transfer_id;
} State;

State state = {0};

/// This flag is raised when the node is requested to restart.
static volatile bool g_restart_required = false;

/// A deeply embedded system should sample a microsecond-resolution non-overflowing 64-bit timer.
/// Here is a simple non-blocking implementation as an example:
/// https://github.com/PX4/sapog/blob/601f4580b71c3c4da65cc52237e62a/firmware/src/motor/realtime/motor_timer.c#L233-L274
/// Mind the difference between monotonic time and wall time. Monotonic time never changes rate or makes leaps,
/// it is therefore impossible to synchronize with an external reference. Wall time can be synchronized and therefore
/// it may change rate or make leap adjustments. The two kinds of time serve completely different purposes.
static CanardMicrosecond getMonotonicMicroseconds()
{

    return (CanardMicrosecond)micros();
}

// Returns the 128-bit unique-ID of the local node. This value is used in uavcan.node.GetInfo.Response and during the
// plug-and-play node-ID allocation by uavcan.pnp.NodeIDAllocationData. The function is infallible.
static void getUniqueID(uint8_t out[uavcan_node_GetInfo_Response_1_0_unique_id_ARRAY_CAPACITY_])
{
    // A real hardware node would read its unique-ID from some hardware-specific source (typically stored in ROM).
    // This example is a software-only node so we store the unique-ID in a (read-only) register instead.
    uavcan_register_Value_1_0 value = {0};
    uavcan_register_Value_1_0_select_unstructured_(&value);
    // Populate the default; it is only used at the first run if there is no such register.
    for (uint8_t i = 0; i < uavcan_node_GetInfo_Response_1_0_unique_id_ARRAY_CAPACITY_; i++)
    {
        value.unstructured.value.elements[value.unstructured.value.count++] = (uint8_t) rand();  // NOLINT
    }
    registerRead("uavcan.node.unique_id", &value);
    RMAP_ASSERT(uavcan_register_Value_1_0_is_unstructured_(&value) &&
           value.unstructured.value.count == uavcan_node_GetInfo_Response_1_0_unique_id_ARRAY_CAPACITY_);
    memcpy(&out[0], &value.unstructured.value, uavcan_node_GetInfo_Response_1_0_unique_id_ARRAY_CAPACITY_);
}

/// Reads the port-ID from the corresponding standard register. The standard register schema is documented in
/// the Cyphal Specification, section for the standard service uavcan.register.Access. You can also find it here:
/// https://github.com/OpenCyphal/public_regulated_data_types/blob/master/uavcan/register/384.Access.1.0.dsdl
/// A very hands-on demo is available in Python: https://pycyphal.readthedocs.io/en/stable/pages/demo.html
static CanardPortID getPublisherSubjectID(const char* const port_name, const char* const type_name)
{
    // Deduce the register name from port name.
    char register_name[uavcan_register_Name_1_0_name_ARRAY_CAPACITY_] = {0};
    snprintf(&register_name[0], sizeof(register_name), "%s.id", port_name);

    // Set up the default value. It will be used to populate the register if it doesn't exist.
    uavcan_register_Value_1_0 val = {0};
    uavcan_register_Value_1_0_select_natural16_(&val);
    val.natural16.value.count       = 1;
    val.natural16.value.elements[0] = UINT16_MAX;  // This means "undefined", per Specification, which is the default.

    // Read the register with defensive self-checks.
    registerRead(&register_name[0], &val);
    RMAP_ASSERT(uavcan_register_Value_1_0_is_natural16_(&val) && (val.natural16.value.count == 1));
    const uint16_t result = val.natural16.value.elements[0];

    // This part is NOT required but recommended by the Specification for enhanced introspection capabilities. It is
    // very cheap to implement so all implementations should do so. This register simply contains the name of the
    // type exposed at this port. It should be immutable but it is not strictly required so in this implementation
    // we take shortcuts by making it mutable since it's behaviorally simpler in this specific case.
    snprintf(&register_name[0], sizeof(register_name), "%s.type", port_name);
    uavcan_register_Value_1_0_select_string_(&val);
    val._string.value.count = nunavutChooseMin(strlen(type_name), uavcan_primitive_String_1_0_value_ARRAY_CAPACITY_);
    memcpy(&val._string.value.elements[0], type_name, val._string.value.count);
    registerWrite(&register_name[0], &val);  // Unconditionally overwrite existing value because it's read-only.

    return result;
}

static void send(State* const                        state,
                 const CanardMicrosecond             tx_deadline_usec,
                 const CanardTransferMetadata* const metadata,
                 const size_t                        payload_size,
                 const void* const                   payload)
{
    for (uint8_t ifidx = 0; ifidx < CAN_REDUNDANCY_FACTOR; ifidx++)
    {
      //Log.verbose(F("send %l-%l %l" CR),(uint32_t)getMonotonicMicroseconds(),(uint32_t)tx_deadline_usec,payload_size);      
        int32_t frames = canardTxPush(&state->canard_tx_queues[ifidx],
                            &state->canard,
                            tx_deadline_usec,
                            metadata,
                            payload_size,
                            payload);
	//Log.verbose(F("frame %l" CR),frames);      	
    }
}

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


static void updateSensorsData(void){

  module_th_msg.temperature.val.value                 = (int32_t) (rand() % 2000 + 27315);  // TODO: sample data from the real sensor.
  module_th_msg.temperature.confidence.value          = (uint8_t) (rand() % 100       );  // TODO: sample data from the real sensor.
  module_th_msg.humidity.val.value                    = (int32_t) (rand() % 100       );  // TODO: sample data from the real sensor.
  module_th_msg.humidity.confidence.value             = (uint8_t) (rand() % 100       );  // TODO: sample data from the real sensor.
}


/// Invoked at the rate of the fastest loop.
static void handleFastLoop(State* const state, const CanardMicrosecond monotonic_time)
{
  //Log.verbose(F("handleFastLoop" CR));
  
    const bool anonymous = state->canard.node_id > CANARD_NODE_ID_MAX;

    // Publish differential pressure reading if the subject is enabled and the node is non-anonymous.
    if (!anonymous && (state->port_id.pub.module_th <= CANARD_SUBJECT_ID_MAX))
    {
      updateSensorsData();
        // Serialize and publish the message:
        uint8_t      serialized[rmap_module_TH_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
        size_t       serialized_size = sizeof(serialized);
        const int8_t err = rmap_module_TH_1_0_serialize_(&module_th_msg, &serialized[0], &serialized_size);
        RMAP_ASSERT(err >= 0);
        if (err >= 0)
        {
            const CanardTransferMetadata meta = {
                .priority       = CanardPriorityHigh,
                .transfer_kind  = CanardTransferKindMessage,
                .port_id        = state->port_id.pub.module_th,
                .remote_node_id = CANARD_NODE_ID_UNSET,
                .transfer_id    = (CanardTransferID) state->next_transfer_id.module_th++,  // Increment!
            };
            send(state, monotonic_time + 10 * KILO, &meta, serialized_size, &serialized[0]);
        }
    }
}

/// Invoked every second.
static void handle1HzLoop(State* const state, const CanardMicrosecond monotonic_time)
{

  //Log.verbose(F("handle1HzLoop" CR));
  
    const bool anonymous = state->canard.node_id > CANARD_NODE_ID_MAX;
    // Publish heartbeat every second unless the local node is anonymous. Anonymous nodes shall not publish heartbeat.
    if (!anonymous)
    {
        uavcan_node_Heartbeat_1_0 heartbeat = {0};
        heartbeat.uptime                    = (uint32_t) ((monotonic_time - state->started_at) / MEGA);
        heartbeat.mode.value                = uavcan_node_Mode_1_0_OPERATIONAL;
        const O1HeapDiagnostics heap_diag   = o1heapGetDiagnostics(state->heap);
        if (heap_diag.oom_count > 0)
        {
            heartbeat.health.value = uavcan_node_Health_1_0_CAUTION;
        }
        else
        {
            heartbeat.health.value = uavcan_node_Health_1_0_NOMINAL;
        }

        uint8_t      serialized[uavcan_node_Heartbeat_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
        size_t       serialized_size                                                        = sizeof(serialized);
        const int8_t err = uavcan_node_Heartbeat_1_0_serialize_(&heartbeat, &serialized[0], &serialized_size);
        RMAP_ASSERT(err >= 0);
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
                 monotonic_time + MEGA,  // Set transmission deadline 1 second, optimal for heartbeat.
                 &meta,
                 serialized_size,
                 &serialized[0]);
        }
    }
    else  // If we don't have a node-ID, obtain one by publishing allocation request messages until we get a response.
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
            // TODO: Creare UniqueId e gestirlo
            uint64_t local_unique_id_hash = 0x2030405060708090;
            // msg.allocated_node_id.count
            // msg.allocated_node_id.(count/element) => Solo in response non in request;
            msg.unique_id_hash = local_unique_id_hash;
            //////local_unique_id_hash = getUniqueID_10(msg2.unique_id);
            uint8_t      serialized[uavcan_pnp_NodeIDAllocationData_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
            size_t       serialized_size = sizeof(serialized);
            const int8_t err = uavcan_pnp_NodeIDAllocationData_1_0_serialize_(&msg, &serialized[0], &serialized_size);
            assert(err >= 0);
            if (err >= 0)
            {
                const CanardTransferMetadata meta = {
                    .priority       = CanardPrioritySlow,
                    .transfer_kind  = CanardTransferKindMessage,
                    .port_id        = uavcan_pnp_NodeIDAllocationData_1_0_FIXED_PORT_ID_,
                    .remote_node_id = CANARD_NODE_ID_UNSET,
                    .transfer_id    = (CanardTransferID) (state->next_transfer_id.uavcan_pnp_allocation++),
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
            // Note that this will only work over CAN FD. If you need to run PnP over Classic CAN, use message v1.0.
            uavcan_pnp_NodeIDAllocationData_2_0 msg = {0};
            msg.node_id.value                       = UINT16_MAX;
            getUniqueID(msg.unique_id);
            uint8_t      serialized[uavcan_pnp_NodeIDAllocationData_2_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
            size_t       serialized_size = sizeof(serialized);
            const int8_t err = uavcan_pnp_NodeIDAllocationData_2_0_serialize_(&msg, &serialized[0], &serialized_size);
            RMAP_ASSERT(err >= 0);
            if (err >= 0)
            {
                const CanardTransferMetadata meta = {
                    .priority       = CanardPrioritySlow,
                    .transfer_kind  = CanardTransferKindMessage,
                    .port_id        = uavcan_pnp_NodeIDAllocationData_2_0_FIXED_PORT_ID_,
                    .remote_node_id = CANARD_NODE_ID_UNSET,
                    .transfer_id    = (CanardTransferID) (state->next_transfer_id.uavcan_pnp_allocation++),
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

    // Publish module_th reading if the subject is enabled and the node is non-anonymous.
    if (!anonymous && state->port_id.pub.module_th <= CANARD_SUBJECT_ID_MAX)
    {

	updateSensorsData();

        // Serialize and publish the message:
        uint8_t      serialized[rmap_module_TH_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
        size_t       serialized_size = sizeof(serialized);
        const int8_t err = rmap_module_TH_1_0_serialize_(&module_th_msg, &serialized[0], &serialized_size);
        RMAP_ASSERT(err >= 0);
	if (err >= 0)
        {
            const CanardTransferMetadata meta = {
                .priority       = CanardPriorityNominal,
                .transfer_kind  = CanardTransferKindMessage,
                .port_id        = state->port_id.pub.module_th,
                .remote_node_id = CANARD_NODE_ID_UNSET,
                .transfer_id    = (CanardTransferID) state->next_transfer_id.module_th++,  // Increment!
            };
            send(state, monotonic_time + MEGA, &meta, serialized_size, &serialized[0]);
        }
    }
}

/// This is needed only for constructing uavcan_node_port_List_0_1.
static void fillSubscriptions(const CanardTreeNode* const tree, uavcan_node_port_SubjectIDList_0_1* const obj)
{
    if (NULL != tree)
    {
        fillSubscriptions(tree->lr[0], obj);
        const CanardRxSubscription* crs = (const CanardRxSubscription*) tree;
        RMAP_ASSERT(crs->port_id <= CANARD_SUBJECT_ID_MAX);
        RMAP_ASSERT(obj->sparse_list.count < uavcan_node_port_SubjectIDList_0_1_sparse_list_ARRAY_CAPACITY_);
        obj->sparse_list.elements[obj->sparse_list.count++].value = crs->port_id;
        fillSubscriptions(tree->lr[1], obj);
    }
}

/// This is needed only for constructing uavcan_node_port_List_0_1.
static void fillServers(const CanardTreeNode* const tree, uavcan_node_port_ServiceIDList_0_1* const obj)
{
    if (NULL != tree)
    {
        fillServers(tree->lr[0], obj);
        const CanardRxSubscription* crs = (const CanardRxSubscription*) tree;
        RMAP_ASSERT(crs->port_id <= CANARD_SERVICE_ID_MAX);
        (void) nunavutSetBit(&obj->mask_bitpacked_[0], sizeof(obj->mask_bitpacked_), crs->port_id, true);
        fillServers(tree->lr[1], obj);
    }
}

/// Invoked every 10 seconds.
static void handle01HzLoop(State* const state, const CanardMicrosecond monotonic_time)
{

  //Log.verbose(F("handle01HzLoop" CR));

    // Publish the recommended (not required) port introspection message. No point publishing it if we're anonymous.
    // The message is a bit heavy on the stack (about 2 KiB) but this is not a problem for a modern MCU.
    if (state->canard.node_id <= CANARD_NODE_ID_MAX)
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
            send(state, monotonic_time + MEGA, &meta, serialized_size, &serialized[0]);
        }
    }
}

#ifdef PNP_NODE_ALLOCATION_V_10
static void processMessagePlugAndPlayNodeIDAllocation(State* const                                     state,
                                                      const uavcan_pnp_NodeIDAllocationData_1_0* const msg)
{
    // TODO: Unificare il GetUniqueID x gestione PnP (con comando e scrittura E2/Rnd...)
    //uint8_t uid[uavcan_node_GetInfo_Response_1_0_unique_id_ARRAY_CAPACITY_] = {0};
    //getUniqueID(uid);
    if (msg->allocated_node_id.elements[0].value <= CANARD_NODE_ID_MAX)
    {
        printf("Got PnP node-ID allocation: %d\n", msg->allocated_node_id.elements[0].value);
        state->canard.node_id = (CanardNodeID) msg->allocated_node_id.elements[0].value;
        // Store the value into the non-volatile storage.
        uavcan_register_Value_1_0 reg = {0};
        uavcan_register_Value_1_0_select_natural16_(&reg);
        reg.natural16.value.elements[0] = msg->allocated_node_id.elements[0].value;
        reg.natural16.value.count       = 1;
        registerWrite("uavcan.node.id", &reg);
        // We no longer need the subscriber, drop it to free up the resources (both memory and CPU time).
        (void) canardRxUnsubscribe(&state->canard,
                                   CanardTransferKindMessage,
                                   uavcan_pnp_NodeIDAllocationData_1_0_FIXED_PORT_ID_);
    }
    // Otherwise, ignore it: either it is a request from another node or it is a response to another node.
}
#endif
#ifdef PNP_NODE_ALLOCATION_V_20
static void processMessagePlugAndPlayNodeIDAllocation(State* const                                     state,
                                                      const uavcan_pnp_NodeIDAllocationData_2_0* const msg)
{
    uint8_t uid[uavcan_node_GetInfo_Response_1_0_unique_id_ARRAY_CAPACITY_] = {0};
    getUniqueID(uid);
    if ((msg->node_id.value <= CANARD_NODE_ID_MAX) && (memcmp(uid, msg->unique_id, sizeof(uid)) == 0))
    {
        printf("Got PnP node-ID allocation: %d\n", msg->node_id.value);
        state->canard.node_id = (CanardNodeID) msg->node_id.value;
        // Store the value into the non-volatile storage.
        uavcan_register_Value_1_0 reg = {0};
        uavcan_register_Value_1_0_select_natural16_(&reg);
        reg.natural16.value.elements[0] = msg->node_id.value;
        reg.natural16.value.count       = 1;
        registerWrite("uavcan.node.id", &reg);
        // We no longer need the subscriber, drop it to free up the resources (both memory and CPU time).
        (void) canardRxUnsubscribe(&state->canard,
                                   CanardTransferKindMessage,
                                   uavcan_pnp_NodeIDAllocationData_2_0_FIXED_PORT_ID_);
    }
    // Otherwise, ignore it: either it is a request from another node or it is a response to another node.
}
#endif

static uavcan_node_ExecuteCommand_Response_1_1 processRequestExecuteCommand(
    const uavcan_node_ExecuteCommand_Request_1_1* req)
{
    uavcan_node_ExecuteCommand_Response_1_1 resp = {0};
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
        // You can add vendor-specific commands here as well.
    default:
    {
        resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_BAD_COMMAND;
        break;
    }
    }
    return resp;
}

/// Performance notice: the register storage may be slow to access depending on its implementation (e.g., if it is
/// backed by an uncached filesystem). If your register storage implementation is slow, this may disrupt real-time
/// activities of the device. To avoid this, you can employ these measures:
/// - Access registers asynchronously.
/// - Run a separate Cyphal processing task for soft-real-time blocking operations (this approach is used in PX4).
/// - Cache register states in RAM and synchronize them with the storage asynchronously.
/// - Document an operational limitation that the register interface should not be accessed while ENGAGED (armed).
static uavcan_register_Access_Response_1_0 processRequestRegisterAccess(const uavcan_register_Access_Request_1_0* req)
{
    char name[uavcan_register_Name_1_0_name_ARRAY_CAPACITY_ + 1] = {0};
    RMAP_ASSERT(req->name.name.count < sizeof(name));
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

/// Constructs a response to uavcan.node.GetInfo which contains the basic information about this node.
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

static void processReceivedTransfer(State* const state, const CanardRxTransfer* const transfer)
{
    if (transfer->metadata.transfer_kind == CanardTransferKindMessage)
    {
      Log.notice(F("message"));
        size_t size = transfer->payload_size;

        #ifdef PNP_NODE_ALLOCATION_V_10
        if (transfer->metadata.port_id == uavcan_pnp_NodeIDAllocationData_1_0_FIXED_PORT_ID_)
        {
            uavcan_pnp_NodeIDAllocationData_1_0 msg = {0};
            if (uavcan_pnp_NodeIDAllocationData_1_0_deserialize_(&msg, static_cast<uint8_t const*>(transfer->payload), &size) >= 0)
            {
                processMessagePlugAndPlayNodeIDAllocation(state, &msg);
            }
        }
        #endif
        #ifdef PNP_NODE_ALLOCATION_V_20
        if (transfer->metadata.port_id == uavcan_pnp_NodeIDAllocationData_2_0_FIXED_PORT_ID_)
        {
            uavcan_pnp_NodeIDAllocationData_2_0 msg = {0};
            if (uavcan_pnp_NodeIDAllocationData_2_0_deserialize_(&msg, static_cast<uint8_t const*>(transfer->payload), &size) >= 0)
            {
                processMessagePlugAndPlayNodeIDAllocation(state, &msg);
            }
        }
	#endif
        else
        {
            RMAP_ASSERT(false);  // Seems like we have set up a port subscription without a handler -- bad implementation.
        }
    }
    else if (transfer->metadata.transfer_kind == CanardTransferKindRequest)
    {

      Log.notice(F("request portid %d" CR),transfer->metadata.port_id);
	if (transfer->metadata.port_id == state->port_id.pub.service_module_th)
	{
            // The request object is empty so we don't bother deserializing it. Just send the response.

	    updateSensorsData();
	    
	    // Serialize and publish the message:
	    uint8_t      serialized[rmap_module_TH_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
	    size_t       serialized_size = sizeof(serialized);
	    const int8_t res = rmap_module_TH_1_0_serialize_(&module_th_msg, &serialized[0], &serialized_size);
	    if (res >= 0)
	      {
	  // Send the response back. Make sure to re-use the same priority and transfer-ID.
		// todo
		//transfer->metadata.transfer_kind  = CanardTransferKindResponse;
	  sendResponse(state,
		       transfer->timestamp_usec + MEGA,
		       &transfer->metadata,
		       serialized_size,
		       &serialized[0]);
	      }
	}
      else if (transfer->metadata.port_id == uavcan_node_GetInfo_1_0_FIXED_PORT_ID_)
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
            else
            {
                RMAP_ASSERT(false);
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
                const uavcan_node_ExecuteCommand_Response_1_1 resp = processRequestExecuteCommand(&req);
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
            RMAP_ASSERT(false);  // Seems like we have set up a port subscription without a handler -- bad implementation.
        }
    }
    else
    {
        RMAP_ASSERT(false);  // Bad implementation -- check your subscriptions.
    }
}

static void* canardAllocate(CanardInstance* const ins, const size_t amount)
{
    O1HeapInstance* const heap = ((State*) ins->user_reference)->heap;
    RMAP_ASSERT(o1heapDoInvariantsHold(heap));
    return o1heapAllocate(heap, amount);
}

static void canardFree(CanardInstance* const ins, void* const pointer)
{
    O1HeapInstance* const heap = ((State*) ins->user_reference)->heap;
    o1heapFree(heap, pointer);
}

extern char** environ;

void CAN_HW_Init(void) {

  GPIO_InitTypeDef GPIO_InitStruct;

  // GPIO Ports Clock Enable
  __HAL_RCC_GPIOA_CLK_ENABLE();

  // CAN1 clock enable
  __HAL_RCC_CAN1_CLK_ENABLE();

  // CAN GPIO Configuration
  // PA11     ------> CAN_RX
  // PA12     ------> CAN_TX

#if defined (STM32F103x6) || defined (STM32F103xB)
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
#elif defined (STM32F303x8) || defined (STM32F303xC) || defined (STM32F303xE)
  GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF9_CAN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
#else
#error "Error untested processor variant"
#endif

  BxCANTimings timings;
  bool result = bxCANComputeTimings(HAL_RCC_GetPCLK1Freq(), 250000, &timings);
  if (!result) {
    Log.fatal(F("timings Error" CR));
    delay(1000);
    NVIC_SystemReset();
  }

  result = bxCANConfigure(0, timings, false);
  if (!result) {
    Log.fatal(F("configure Error" CR));
    delay(1000);
    NVIC_SystemReset();
  }
  
}

void setup(void) {

  Serial.begin(115200);
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);
  Log.notice(F("Initializing..." CR));
  Log.notice(F("clock : %d" CR), HAL_RCC_GetHCLKFreq());

  CAN_HW_Init();

  registerSetup();
    
  // A simple node like this one typically does not require more than 8 KiB of heap and 4 KiB of stack.
  // For the background and related theory refer to the following resources:
  // - https://github.com/OpenCyphal/libcanard/blob/master/README.md
  // - https://github.com/pavel-kirienko/o1heap/blob/master/README.md
  // - https://forum.opencyphal.org/t/uavcanv1-libcanard-nunavut-templates-memory-usage-concerns/1118/4
  _Alignas(O1HEAP_ALIGNMENT) static uint8_t heap_arena[1024 * 8] = {0};
  state.heap                                                      = o1heapInit(heap_arena, sizeof(heap_arena));
  RMAP_ASSERT(NULL != state.heap);

  // The libcanard instance requires the allocator for managing protocol states.
  state.canard                = canardInit(&canardAllocate, &canardFree);
  state.canard.user_reference = &state;  // Make the state reachable from the canard instance.

  // Restore the node-ID from the corresponding standard register. Default to anonymous.
  uavcan_register_Value_1_0 val = {0};
  uavcan_register_Value_1_0_select_natural16_(&val);
  val.natural16.value.count       = 1;
  val.natural16.value.elements[0] = UINT16_MAX;  // This means undefined (anonymous), per Specification/libcanard.
  registerRead("uavcan.node.id", &val);  // The names of the standard registers are regulated by the Specification.
  RMAP_ASSERT(uavcan_register_Value_1_0_is_natural16_(&val) && (val.natural16.value.count == 1));
  state.canard.node_id = (val.natural16.value.elements[0] > CANARD_NODE_ID_MAX)
    ? CANARD_NODE_ID_UNSET
    : (CanardNodeID) val.natural16.value.elements[0];

  // The description register is optional but recommended because it helps constructing/maintaining large networks.
  // It simply keeps a human-readable description of the node that should be empty by default.
  uavcan_register_Value_1_0_select_string_(&val);
  val._string.value.count = 0;
  registerRead("uavcan.node.description", &val);  // We don't need the value, we just need to ensure it exists.

  // The UDRAL cookie is used to mark nodes that are auto-configured by a specific auto-configuration authority.
  // We don't use this value, it is managed by remote nodes; our only responsibility is to persist it across reboots.
  // This register is entirely optional though; if not provided, the node will have to be configured manually.
  uavcan_register_Value_1_0_select_string_(&val);
  val._string.value.count = 0;  // The value should be empty by default, meaning that the node is not configured.
  registerRead("udral.pnp.cookie", &val);

  // Announce which UDRAL network services we support by populating appropriate registers. They are supposed to be
  // immutable (read-only), but in this simplified demo we don't support that, so we make them mutable (do fix this).
  uavcan_register_Value_1_0_select_string_(&val);
  strcpy((char*) val._string.value.elements, "airspeed");  // Prefix in port names like "differential_pressure", etc.
  val._string.value.count = strlen((const char*) val._string.value.elements);
  registerWrite("reg.udral.service.pitot", &val);

  /*
  // Configure the transport by reading the appropriate standard registers.
  uavcan_register_Value_1_0_select_natural16_(&val);
  val.natural16.value.count       = 1;
  val.natural16.value.elements[0] = CANARD_MTU_CAN_FD;
  registerRead("uavcan.can.mtu", &val);
  RMAP_ASSERT(uavcan_register_Value_1_0_is_natural16_(&val) && (val.natural16.value.count == 1));
  // We also need the bitrate configuration register. In this demo we can't really use it but an embedded application
  // should define "uavcan.can.bitrate" of type natural32[2]; the second value is 0/ignored if CAN FD not supported.
  const int sock[CAN_REDUNDANCY_FACTOR] = {
  socketcanOpen("vcan0", val.natural16.value.elements[0] > CANARD_MTU_CAN_CLASSIC)  //
  };
  */
  for (uint8_t ifidx = 0; ifidx < CAN_REDUNDANCY_FACTOR; ifidx++)
  {
    state.canard_tx_queues[ifidx] = canardTxInit(CAN_TX_QUEUE_CAPACITY, CANARD_MTU_CAN_CLASSIC);
  }
 
  // Load the port-IDs from the registers. You can implement hot-reloading at runtime if desired.
  // Publications:
  state.port_id.pub.module_th =
    getPublisherSubjectID("rmap.module.TH.1.0",
  			  rmap_module_TH_1_0_FULL_NAME_AND_VERSION_);
  
  //state.port_id.pub.module_th =
  //  getPublisherSubjectID("uavcan.pub.TH",
  //			  rmap_module_TH_1_0_FULL_NAME_AND_VERSION_);
  
  
  state.port_id.pub.service_module_th =
    getPublisherSubjectID("rmap.service.module.TH.GetDataAndMetadata.1.0",
  			  rmap_service_module_TH_GetDataAndMetadata_1_0_FULL_NAME_AND_VERSION_);
  
  //state.port_id.pub.service_module_th =
  //  getPublisherSubjectID("uavcan.pub.GetDataAndMetadata",
  //			  rmap_service_module_TH_GetDataAndMetadata_1_0_FULL_NAME_AND_VERSION_);
  
  // Set up the default value. It will be used to populate the register if it doesn't exist.
  uavcan_register_Value_1_0_select_natural32_(&val);
  val.natural32.value.count       = 1;
  val.natural32.value.elements[0] = UINT32_MAX;  // This means "undefined", per Specification, which is the default.
  

  registerRead("rmap.module.TH.metadata.Level.L1", &val);  // Unconditionally overwrite existing value because it's read-only.
  module_th_msg.metadata.level.L1.value = val.natural32.value.elements[0];

  registerRead("rmap.module.TH.metadata.Level.L2", &val);  // Unconditionally overwrite existing value because it's read-only.
  module_th_msg.metadata.level.L2.value = val.natural32.value.elements[0];
    
  registerRead("rmap.module.TH.metadata.Timerange.P1", &val);  // Unconditionally overwrite existing value because it's read-only.
  module_th_msg.metadata.timerange.P1.value = val.natural32.value.elements[0];
    
  registerRead("rmap.module.TH.metadata.Timerange.P2", &val);  // Unconditionally overwrite existing value because it's read-only.
  module_th_msg.metadata.timerange.P2.value = val.natural32.value.elements[0];
    

    
  uavcan_register_Value_1_0_select_natural8_(&val);
  val.natural16.value.count       = 1;
  val.natural16.value.elements[0] = UINT8_MAX;  // This means "undefined", per Specification, which is the default.

  registerRead("rmap.module.TH.metadata.Timerange.Pindicator", &val);  // Unconditionally overwrite existing value because it's read-only.
  module_th_msg.metadata.timerange.Pindicator.value = val.natural8.value.elements[0];
    
    
  registerRead("rmap.module.TH.metadata.Level.LevelType1", &val);  // Unconditionally overwrite existing value because it's read-only.
  module_th_msg.metadata.level.LevelType1.value = val.natural8.value.elements[0];
    
  registerRead("rmap.module.TH.metadata.Level.LevelType2", &val);  // Unconditionally overwrite existing value because it's read-only.
  module_th_msg.metadata.level.LevelType2.value = val.natural8.value.elements[0];

  // Subscriptions:
  // (none in this application)

  // Set up subject subscriptions and RPC-service servers.
  // Message subscriptions:
  if (state.canard.node_id > CANARD_NODE_ID_MAX)

    #ifdef PNP_NODE_ALLOCATION_V_10    
    {
        // PnP over Classic CAN, use message v1.0.
        static CanardRxSubscription rx;
        const int8_t                res =  //
            canardRxSubscribe(&state.canard,
                              CanardTransferKindMessage,
                              uavcan_pnp_NodeIDAllocationData_1_0_FIXED_PORT_ID_,
                              uavcan_pnp_NodeIDAllocationData_1_0_EXTENT_BYTES_,
                              CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
                              &rx);
    #endif
    #ifdef PNP_NODE_ALLOCATION_V_20    
    {
      static CanardRxSubscription rx;
      const int8_t                res =  //
	canardRxSubscribe(&state.canard,
			  CanardTransferKindMessage,
			  uavcan_pnp_NodeIDAllocationData_2_0_FIXED_PORT_ID_,
			  uavcan_pnp_NodeIDAllocationData_2_0_EXTENT_BYTES_,
			  CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
			  &rx);
      #endif
      if (res < 0)
        {
	  Log.fatal(F("subscribe message Error" CR));
	  delay(1000);
	  NVIC_SystemReset();
	  //return -res;
        }
    }
  // Service servers:
  {
    static CanardRxSubscription rx;
    const int8_t                res =  //
      canardRxSubscribe(&state.canard,
			CanardTransferKindRequest,
			uavcan_node_GetInfo_1_0_FIXED_PORT_ID_,
			uavcan_node_GetInfo_Request_1_0_EXTENT_BYTES_,
			CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
			&rx);
    if (res < 0)
      {
	Log.fatal(F("subscribe request Error" CR));
	delay(1000);
	NVIC_SystemReset();
	//return -res;
      }
  }
  {
    static CanardRxSubscription rx;
    const int8_t                res =  //
      canardRxSubscribe(&state.canard,
			CanardTransferKindRequest,
			uavcan_node_ExecuteCommand_1_1_FIXED_PORT_ID_,
			uavcan_node_ExecuteCommand_Request_1_1_EXTENT_BYTES_,
			CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
			&rx);
    if (res < 0)
      {
	Log.fatal(F("subscribe command Error" CR));
	delay(1000);
	NVIC_SystemReset();
	//return -res;
      }
  }
  {
    static CanardRxSubscription rx;
    const int8_t                res =  //
      canardRxSubscribe(&state.canard,
			CanardTransferKindRequest,
			uavcan_register_Access_1_0_FIXED_PORT_ID_,
			uavcan_register_Access_Request_1_0_EXTENT_BYTES_,
			CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
			&rx);
    if (res < 0)
      {
	Log.fatal(F("subscribe register Error" CR));
	delay(1000);
	NVIC_SystemReset();
	//return -res;
      }
  }
  {
    static CanardRxSubscription rx;
    const int8_t                res =  //
      canardRxSubscribe(&state.canard,
			CanardTransferKindRequest,
			uavcan_register_List_1_0_FIXED_PORT_ID_,
			uavcan_register_List_Request_1_0_EXTENT_BYTES_,
			CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
			&rx);
    if (res < 0)
      {
	Log.fatal(F("subscribe register list Error" CR));
	delay(1000);
	NVIC_SystemReset();
	//return -res;
      }
  }

  // Configure the library to listen for register access service requests.
  {
 
    static CanardRxSubscription rx;
    const int8_t                res =  //
      canardRxSubscribe(&state.canard,
			CanardTransferKindRequest,
			state.port_id.pub.service_module_th,
			rmap_service_module_TH_GetDataAndMetadata_Request_1_0_EXTENT_BYTES_,
			CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
			&rx);

    if (res < 0)
      {
	Log.fatal(F("subscribe get Error" CR));
	delay(1000);
	NVIC_SystemReset();
	//return -res;
      }
  }  


  // Now the node is initialized and we're ready to roll.
  state.started_at                           = getMonotonicMicroseconds();
  
  // initialize digital pins
  pinMode(PC13, OUTPUT);
  digitalWrite(PC13, LOW);

  //Log.notice(F("end setup" CR));

}


void loop(void)
{

  //Log.notice(F("loop" CR));
  
  const CanardMicrosecond fast_loop_period   = MEGA / 3;
  CanardMicrosecond       next_fast_iter_at  = state.started_at + fast_loop_period;
  CanardMicrosecond       next_1_hz_iter_at  = state.started_at + MEGA;
  CanardMicrosecond       next_01_hz_iter_at = state.started_at + MEGA * 10;
  do
    {

      //Log.notice(F("do" CR));
      
      // Run a trivial scheduler polling the loops that run the business logic.
      CanardMicrosecond monotonic_time = getMonotonicMicroseconds();
      if (monotonic_time >= next_fast_iter_at)
        {
	  next_fast_iter_at += fast_loop_period;
	  handleFastLoop(&state, monotonic_time);
        }
      if (monotonic_time >= next_1_hz_iter_at)
        {
	  next_1_hz_iter_at += MEGA;
	  handle1HzLoop(&state, monotonic_time);
        }
      if (monotonic_time >= next_01_hz_iter_at)
        {
	  next_01_hz_iter_at += MEGA * 10;
	  handle01HzLoop(&state, monotonic_time);
        }

      // Transmit pending frames from the prioritized TX queues managed by libcanard.
      for (uint8_t ifidx = 0; ifidx < CAN_REDUNDANCY_FACTOR; ifidx++)
        {
	  CanardTxQueue* const     que = &state.canard_tx_queues[ifidx];
	  const CanardTxQueueItem* tqi = canardTxPeek(que);  // Find the highest-priority frame.
	  while (tqi != NULL)
            {
	      // Attempt transmission only if the frame is not yet timed out while waiting in the TX queue.
	      // Otherwise just drop it and move on to the next one.
	      if ((tqi->tx_deadline_usec == 0) || (tqi->tx_deadline_usec > monotonic_time))
                {

		  //Log.notice(F("bxCANPush" CR));
		  
		  bool result =bxCANPush(ifidx,
					 micros(),
					 tqi->tx_deadline_usec,
					 tqi->frame.extended_can_id,
					 tqi->frame.payload_size,
					 tqi->frame.payload);
		  
		  if (!result)
                    {
		      //Log.fatal(F("BXCAN interface failure" CR));
		      //delay(1000);
		      //NVIC_SystemReset();

		      //Log.notice(F("The queue is full, we will try again on the next iteration." CR));
		      break;
		    }
                }
	      state.canard.memory_free(&state.canard, canardTxPop(que, tqi));
	      tqi = canardTxPeek(que);
            }
        }

      // Process received frames by feeding them from SocketCAN to libcanard.
      // The order in which we handle the redundant interfaces doesn't matter -- libcanard can accept incoming
      // frames from any of the redundant interface in an arbitrary order. The internal state machine will sort
      // them out and remove duplicates automatically.


      for (uint8_t ifidx = 0; ifidx < CAN_REDUNDANCY_FACTOR; ifidx++)
        {

	  uint32_t extended_can_id = 0;
	  size_t payload_size = CANARD_MTU_CAN_CLASSIC;
	  CanardFrame   frame;
	  uint8_t payload[CANARD_MTU_CAN_CLASSIC] = {0};
	      
	  bool bxcan_result  = bxCANPop(ifidx, &extended_can_id, &payload_size, payload);
	    
	  //const int16_t socketcan_result       = socketcanPop(sock[ifidx], &frame, NULL, sizeof(buf), buf, 0, NULL);
	  if (! bxcan_result)  // The read operation has timed out with no frames, nothing to do here.
            {
	      break;
            }

	  //Log.notice(F("bxCANPop" CR));
	  
	  //payload to frame !
	  frame.extended_can_id = extended_can_id;
	  frame.payload = payload;
	  frame.payload_size = payload_size;
	    
	  // The SocketCAN adapter uses the wall clock for timestamping, but we need monotonic.
	  // Wall clock can only be used for time synchronization.
	  const CanardMicrosecond timestamp_usec = getMonotonicMicroseconds();
	  CanardRxTransfer        transfer ; //      = {0};
	  const int8_t canard_result = canardRxAccept(&state.canard, timestamp_usec, &frame, ifidx, &transfer, NULL);
	  if (canard_result > 0)
            {
	      processReceivedTransfer(&state, &transfer);
	      state.canard.memory_free(&state.canard, (void*) transfer.payload);
            }
	  else if ((canard_result == 0) || (canard_result == -CANARD_ERROR_OUT_OF_MEMORY))
            {
	      //Log.notice(F("nothing to do" CR));
	      (void) 0;  // The frame did not complete a transfer so there is nothing to do.
	      // OOM should never occur if the heap is sized correctly. You can track OOM errors via heap API.
            }
	  else
            {
	      RMAP_ASSERT(false);  // No other error can possibly occur at runtime.
            }
        }
    } while (!g_restart_required);

  // It is recommended to postpone restart until all frames are sent though.
  Log.fatal(F("Reboot" CR));
  delay(1000);
  NVIC_SystemReset();

}
