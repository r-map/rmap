// *************************************************************************************************
// **********        Funzioni ed utility generiche per gestione Cyphal STM32 RMAP         **********
// *************************************************************************************************

// This software is distributed under the terms of the MIT License.
// Copyright (C) 2022 Digiteco s.r.l.
// Author: Gasperini Moreno <m.gasperini@digiteco.it>

// Arduino
#include <Arduino.h>
// Classe Canard
#include "canardClass_th.hpp"
// Libcanard
#include "register.hpp"
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

// ***************************************************************************************************
// **********             Funzioni ed utility generiche per gestione UAVCAN                 **********
// ***************************************************************************************************

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
    LOCAL_ASSERT(uavcan_register_Value_1_0_is_unstructured_(&value) &&
           value.unstructured.value.count == uavcan_node_GetInfo_Response_1_0_unique_id_ARRAY_CAPACITY_);
    memcpy(&out[0], &value.unstructured.value, uavcan_node_GetInfo_Response_1_0_unique_id_ARRAY_CAPACITY_);
}

/// Legge il subjectID per il modulo corrente per la risposta al servizio di gestione dati.
static CanardPortID getModeAccessID(uint8_t modeAccessID, const char* const port_name, const char* const type_name) {
    // Deduce the register name from port name e modeAccess
    char register_name[uavcan_register_Name_1_0_name_ARRAY_CAPACITY_] = {0};
    // In funzione del modo imposta il registro corretto
    switch (modeAccessID) {
        case canardClass::Introspection_Port::PublisherSubjectID:
            snprintf(&register_name[0], sizeof(register_name), "uavcan.pub.%s.id", port_name);
            break;
        case canardClass::Introspection_Port::SubscriptionSubjectID:
            snprintf(&register_name[0], sizeof(register_name), "uavcan.sub.%s.id", port_name);
            break;
        case canardClass::Introspection_Port::ClientPortID:
            snprintf(&register_name[0], sizeof(register_name), "uavcan.cln.%s.id", port_name);
            break;
        case canardClass::Introspection_Port::ServicePortID:
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
    LOCAL_ASSERT(uavcan_register_Value_1_0_is_natural16_(&val) && (val.natural16.value.count == 1));
    const uint16_t result = val.natural16.value.elements[0];

    // This part is NOT required but recommended by the Specification for enhanced introspection capabilities. It is
    // very cheap to implement so all implementations should do so. This register simply contains the name of the
    // type exposed at this port. It should be immutable but it is not strictly required so in this implementation
    // we take shortcuts by making it mutable since it's behaviorally simpler in this specific case.
    // In funzione del modo imposta il registro corretto
    switch (modeAccessID) {
        case canardClass::Introspection_Port::PublisherSubjectID:
            snprintf(&register_name[0], sizeof(register_name), "uavcan.pub.%s.type", port_name);
            break;
        case canardClass::Introspection_Port::SubscriptionSubjectID:
            snprintf(&register_name[0], sizeof(register_name), "uavcan.sub.%s.type", port_name);
            break;
        case canardClass::Introspection_Port::ClientPortID:
            snprintf(&register_name[0], sizeof(register_name), "uavcan.cln.%s.type", port_name);
            break;
        case canardClass::Introspection_Port::ServicePortID:
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

// *******                      FUNZIONI RMAP PUBLISH LOCAL DATA                         *********

// Prepara il blocco messaggio dati per il modulo corrente istantaneo (Crea un esempio)
// TODO: Collegare al modulo sensor_drive per il modulo corrente
// NB: Aggiorno solo i dati fisici in questa funzione i metadati sono esterni
rmap_sensors_TH_1_0 prepareSensorsDataValueExample(uint8_t const sensore) {
    rmap_sensors_TH_1_0 local_data = {0};
    // TODO: Inserire i dati, passaggio da Update... altro
    switch (sensore) {
        case canardClass::Sensor_Type::ith:
            // Prepara i dati ITH
            local_data.temperature.val.value = (int32_t)(rand() % 2000 + 27315);
            local_data.temperature.confidence.value = (uint8_t)(rand() % 100);
            local_data.humidity.val.value = (int32_t)(rand() % 100);
            local_data.humidity.confidence.value = (uint8_t)(rand() % 100);
        case canardClass::Sensor_Type::mth:
            // Prepara i dati ITH
            local_data.temperature.val.value = (int32_t)(rand() % 2000 + 27315);
            local_data.temperature.confidence.value = (uint8_t)(rand() % 100);
            local_data.humidity.val.value = (int32_t)(rand() % 100);
            local_data.humidity.confidence.value = (uint8_t)(rand() % 100);
        case canardClass::Sensor_Type::nth:
            // Prepara i dati ITH
            local_data.temperature.val.value = (int32_t)(rand() % 2000 + 27315);
            local_data.temperature.confidence.value = (uint8_t)(rand() % 100);
            local_data.humidity.val.value = (int32_t)(rand() % 100);
            local_data.humidity.confidence.value = (uint8_t)(rand() % 100);
        case canardClass::Sensor_Type::xth:
            // Prepara i dati ITH
            local_data.temperature.val.value = (int32_t)(rand() % 2000 + 27315);
            local_data.temperature.confidence.value = (uint8_t)(rand() % 100);
            local_data.humidity.val.value = (int32_t)(rand() % 100);
            local_data.humidity.confidence.value = (uint8_t)(rand() % 100);
    }
    return local_data;
}

/// Pubblica i dati RMAP con il metodo publisher se abilitato e configurato
static void publish_rmap_data(canardClass &clsCanard) {
    // Pubblica i dati del nodo corrente se abilitata la funzione e con il corretto subjectId
    // Ovviamente il nodo non può essere anonimo per la pubblicazione...
    if ((!clsCanard.is_canard_node_anonymous()) &&
        (clsCanard.publisher_enabled.module_th) &&
        (clsCanard.port_id.publisher_module_th <= CANARD_SUBJECT_ID_MAX)) {
        rmap_module_TH_1_0 module_th_msg = {0};
        // Preparo i dati e metadati fissi
        // TODO: Aggiorna i valori mobili
        module_th_msg.ITH = prepareSensorsDataValueExample(canardClass::Sensor_Type::ith);
        module_th_msg.ITH.metadata = clsCanard.module_th.ITH.metadata;
        module_th_msg.MTH = prepareSensorsDataValueExample(canardClass::Sensor_Type::mth);
        module_th_msg.MTH.metadata = clsCanard.module_th.MTH.metadata;
        module_th_msg.NTH = prepareSensorsDataValueExample(canardClass::Sensor_Type::nth);
        module_th_msg.NTH.metadata = clsCanard.module_th.NTH.metadata;
        module_th_msg.XTH = prepareSensorsDataValueExample(canardClass::Sensor_Type::xth);
        module_th_msg.XTH.metadata = clsCanard.module_th.XTH.metadata;
        // Serialize and publish the message:
        uint8_t serialized[rmap_module_TH_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
        size_t serialized_size = sizeof(serialized);
        const int8_t err = rmap_module_TH_1_0_serialize_(&module_th_msg, &serialized[0], &serialized_size);
        LOCAL_ASSERT(err >= 0);
        if (err >= 0) {
            const CanardTransferMetadata meta = {
                .priority = CanardPrioritySlow,
                .transfer_kind = CanardTransferKindMessage,
                .port_id = clsCanard.port_id.publisher_module_th,
                .remote_node_id = CANARD_NODE_ID_UNSET,
                .transfer_id = (CanardTransferID)(clsCanard.next_transfer_id.module_th()),  // Increment!
            };
            // Messaggio rapido 1/4 di secondo dal timeStamp Sincronizzato
            clsCanard.send(MEGA / 4, &meta, serialized_size, &serialized[0]);
        }
    }
}

// ***************************************************************************************************
//   Funzioni ed utility di ricezione dati dalla rete UAVCAN, richiamati da processReceivedTransfer()
// ***************************************************************************************************

// Plug and Play Slave, Versione 1.0 compatibile con CAN_CLASSIC MTU 8
// Messaggi anonimi CAN non sono consentiti se messaggi > LUNGHEZZA MTU disponibile
static void processMessagePlugAndPlayNodeIDAllocation(canardClass &clsCanard,
                                                      const uavcan_pnp_NodeIDAllocationData_1_0* const msg) {
    // msg->unique_id_hash RX non gestito, è valido GetUniqueID Unificato per entrambe versioni V1 e V2
    if (msg->allocated_node_id.elements[0].value <= CANARD_NODE_ID_MAX) {
        printf("Got PnP node-ID allocation: %d\n", msg->allocated_node_id.elements[0].value);
        clsCanard.set_canard_node_id((CanardNodeID)msg->allocated_node_id.elements[0].value);
        // Store the value into the non-volatile storage.
        uavcan_register_Value_1_0 reg = {0};
        uavcan_register_Value_1_0_select_natural16_(&reg);
        reg.natural16.value.elements[0] = msg->allocated_node_id.elements[0].value;
        reg.natural16.value.count = 1;
        registerWrite("uavcan.node.id", &reg);
        // We no longer need the subscriber, drop it to free up the resources (both memory and CPU time).
        clsCanard.rxUnSubscribe(CanardTransferKindMessage,
                                uavcan_pnp_NodeIDAllocationData_1_0_FIXED_PORT_ID_);
    }
    // Otherwise, ignore it: either it is a request from another node or it is a response to another node.
}

// Chiamate gestioni RPC remote da master (yakut o altro servizio di controllo)
static uavcan_node_ExecuteCommand_Response_1_1 processRequestExecuteCommand(canardClass &clsCanard, const uavcan_node_ExecuteCommand_Request_1_1* req,
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
            clsCanard.master.file.start_request(remote_node, (uint8_t*) req->parameter.elements,
                                                req->parameter.count, true);
            clsCanard.flag.set_local_fw_uploading(true);
            Serial.print(F("Firmware update request from node id: "));
            Serial.println(clsCanard.master.file.get_server_node());
            Serial.print(F("Filename to download: "));
            Serial.println(clsCanard.master.file.get_name());
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
            clsCanard.flag.request_system_restart();
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
        case canardClass::Command_Private::download_file:
        {
            // Nodo Server chiamante (Yakut solo Master, Yakut e Master per Slave)
            clsCanard.master.file.start_request(remote_node, (uint8_t*) req->parameter.elements,
                                                req->parameter.count, false);
            clsCanard.flag.set_local_fw_uploading(true);
            Serial.print(F("File standard update request from node id: "));
            Serial.println(clsCanard.master.file.get_server_node());
            Serial.print(F("Filename to download: "));
            Serial.println(clsCanard.master.file.get_name());
            // Avvio la funzione con OK
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        case canardClass::Command_Private::enable_publish_rmap:
        {
            // Abilita pubblicazione fast_loop data_and_metadata modulo locale (test yakut e user master)
            clsCanard.publisher_enabled.module_th = true;
            Serial.println(F("ATTIVO Trasmissione dati in publish"));
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        case canardClass::Command_Private::disable_publish_rmap:
        {
            // Disabilita pubblicazione fast_loop data_and_metadata modulo locale (test yakut e user master)
            clsCanard.publisher_enabled.module_th = false;
            Serial.println(F("DISATTIVO Trasmissione dati in publish"));
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        case canardClass::Command_Private::enable_publish_port_list:
        {
            // Abilita pubblicazione slow_loop elenco porte (Cypal facoltativo)
            clsCanard.publisher_enabled.port_list = true;
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        case canardClass::Command_Private::disable_publish_port_list:
        {
            // Disabilita pubblicazione slow_loop elenco porte (Cypal facoltativo)
            clsCanard.publisher_enabled.port_list = false;
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        case canardClass::Command_Private::remote_test:
        {
            // Test locale / remoto
            resp.status = canardClass::Command_Private::remote_test_value;
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
static rmap_service_module_TH_Response_1_0 processRequestGetModuleData(canardClass &clsCanard,
                                                                        rmap_service_module_TH_Request_1_0* req) {
    rmap_service_module_TH_Response_1_0 resp = {0};
    // Richiesta parametri univoca a tutti i moduli
    // req->parametri tipo: rmap_service_setmode_1_0
    // req->parametri.comando (Comando esterno ricevuto 3 BIT)
    // req->parametri.run_sectime (Timer to run 13 bit)

    // Case comandi RMAP su GetModule Data (Da definire con esattezza quali e quanti altri)
    switch (req->parametri.comando) {

        /// saturated uint3 get_istant = 0
        /// Ritorna il dato istantaneo (o ultima acquisizione dal sensore)
        case rmap_service_setmode_1_0_get_istant:
            // Ritorno lo stato (Copia dal comando...)
            resp.stato = req->parametri.comando;
            // Preparo la risposta di esempio
            // TODO: Aggiorna i valori mobili
            resp.ITH = prepareSensorsDataValueExample(canardClass::Sensor_Type::ith);
            resp.MTH = prepareSensorsDataValueExample(canardClass::Sensor_Type::mth);
            resp.NTH = prepareSensorsDataValueExample(canardClass::Sensor_Type::nth);
            resp.XTH = prepareSensorsDataValueExample(canardClass::Sensor_Type::xth);
            break;

        /// saturated uint3 get_current = 1
        /// Ritorna il dato attuale (ciclo finito o no lo stato di acq_vale)
        case rmap_service_setmode_1_0_get_current:
            // resp.dataandmetadata = prepareSensorsDataGetCurrent();
            resp.stato = GENERIC_STATE_UNDEFINED;
            break;

        /// saturated uint3 get_last = 2
        /// Ritorna l'ultimo valore valido di acquisizione (riferito al ciclo precedente)
        /// Se utilizzato con loop automatico, shifta il valore senza perdite di tempo riavvia il ciclo
        case rmap_service_setmode_1_0_get_last:
            resp.stato = GENERIC_STATE_UNDEFINED;
            break;

        /// saturated uint3 reset_last = 3
        /// Reset dell'ultimo valore (dopo lettura... potrebbe essere un comando di command standard)
        /// Potremmo collegare lo stato a heartbeat (ciclo di acquisizione finito, dati disponibili...)
        case rmap_service_setmode_1_0_reset_last:
            resp.stato = GENERIC_STATE_UNDEFINED;
            break;

        /// saturated uint3 start_acq = 4
        /// Avvio ciclo di lettura... una tantum start stop automatico, con tempo parametrizzato
        case rmap_service_setmode_1_0_start_acq:
            resp.stato = GENERIC_STATE_UNDEFINED;
            break;

        /// saturated uint3 stop_acq = 5
        /// Arresta ciclo di lettura in ogni condizione (standard o loop)
        case rmap_service_setmode_1_0_stop_acq:
            resp.stato = GENERIC_STATE_UNDEFINED;
            break;

        /// saturated uint3 loop_acq = 6
        /// Avvio ciclo di lettura... in loop automatico continuo, con tempo parametrizzato
        case rmap_service_setmode_1_0_loop_acq:
            resp.stato = GENERIC_STATE_UNDEFINED;
            break;

        /// saturated uint3 continuos_acq = 7
        /// Avvio ciclo di lettura... in continuo, senza tempo parametrizzato (necessita di stop remoto)
        case rmap_service_setmode_1_0_continuos_acq:
            resp.stato = GENERIC_STATE_UNDEFINED;
            break;

        case rmap_service_setmode_1_0_test_acq:
            resp.stato = req->parametri.comando;
            break;

        /// NON Gestito, risposta error (undefined)
        default:
            resp.stato = GENERIC_STATE_UNDEFINED;
            break;
    }

    // Copio i metadati fissi
    // TODO: aggiornare i metadati mobili
    resp.ITH.metadata = clsCanard.module_th.ITH.metadata;
    resp.MTH.metadata = clsCanard.module_th.MTH.metadata;;
    resp.NTH.metadata = clsCanard.module_th.NTH.metadata;
    resp.XTH.metadata = clsCanard.module_th.XTH.metadata;

    return resp;
}

// Accesso ai registri UAVCAN risposta a richieste
static uavcan_register_Access_Response_1_0 processRequestRegisterAccess(const uavcan_register_Access_Request_1_0* req) {
    char name[uavcan_register_Name_1_0_name_ARRAY_CAPACITY_ + 1] = {0};
    LOCAL_ASSERT(req->name.name.count < sizeof(name));
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
//          CallBack di classe canardClass ( Gestisce i metodi uavcan sottoscritti )
// Processo multiplo di ricezione messaggi e comandi. Gestione entrata ed uscita dei messaggi
// Chiamata direttamente nel main loop in ricezione dalla coda RX
// Richiama le funzioni qui sopra di preparazione e risposta alle richieste
// ******************************************************************************************
static void processReceivedTransfer(canardClass &clsCanard, const CanardRxTransfer* const transfer) {
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
                    // Opzionale Controllo ONLINE direttamente dal messaggio Interno
                    // if (!clsCanard.master.heartbeat.is_online()) {
                        // Il master è entrato in modalità ONLine e gestisco
                        // Serial.println(F("Master controller ONLINE !!! Starting application..."));
                    // }
                    // Set PowerMode da comando HeartBeat Remoto remoteVSC.powerMode
                    // Eventuali alri flag gestiti direttamente quà e SET sulla classe
                    canardClass::heartbeat_VSC remoteVSC;
                    // remoteVSC.powerMode
                    remoteVSC.uint8_val = msg.vendor_specific_status_code;
                    Serial.print(F("RX HeartBeat from master, master power mode SET: -> "));
                    Serial.println(remoteVSC.powerMode);
                    // Processo e registro il nodo: stato, OnLine e relativi flag
                    // Set canard_us local per controllo NodoOffline (validità area di OnLine)
                    clsCanard.master.heartbeat.set_online(MASTER_OFFLINE_TIMEOUT_US);
                    clsCanard.flag.set_local_power_mode(remoteVSC.powerMode);
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
                // Controllo e gestisco solo il nodo MASTER come SyncroTime (a gestione locale)
                // Non sono previsti multi sincronizzatori ma la selezione è esterna alla classe
                if(transfer->metadata.remote_node_id == NODE_MASTER_ID) {
                    bool isSyncronized = false;
                    CanardMicrosecond timestamp_synchronized_us;
                    // Controllo coerenza messaggio per consentire e verificare l'aggiornamento timestamp
                    if(clsCanard.master.timestamp.check_valid_syncronization(
                            transfer->metadata.transfer_id,
                            msg.previous_transmission_timestamp_microsecond)) {
                        // Leggo il time stamp sincronizzato da gestire per Setup RTC
                        timestamp_synchronized_us = clsCanard.master.timestamp.get_timestamp_syncronized(
                            transfer->timestamp_usec,
                            msg.previous_transmission_timestamp_microsecond);
                        isSyncronized = true;
                    } else {
                        Serial.print(F("RX TimeSyncro from master, reset or invalid Value at local_time_stamp (uSec): "));
                        Serial.println(transfer->timestamp_usec);
                    }
                    // Aggiorna i temporizzatori locali per il prossimo controllo
                    CanardMicrosecond last_message_diff_us = clsCanard.master.timestamp.update_timestamp_message(
                        transfer->timestamp_usec, msg.previous_transmission_timestamp_microsecond);
                    // Stampa e/o SETUP RTC
                    if (isSyncronized) {
                        Serial.print(F("RX TimeSyncro from master, syncronized value is (uSec): "));
                        Serial.print(timestamp_synchronized_us);
                        Serial.print(F(" from 2 message difference is (uSec): "));
                        Serial.println(last_message_diff_us);
                    }
                    // Adjust Local RTC Time!!!
                    // TODO: Pseudocode
                    // if abs((timeStamp) - convertCanardUsec(RTC)) > 500000 (0.5 secondi...)
                    //   -> SETDateTime
                    // Oppure wait (microsecond fino al passaggio del secondo e SET-RTC-> Perfetto)
                    // Procedura di esempio già nel main...
                }
            }
        }
        else if (transfer->metadata.port_id == uavcan_pnp_NodeIDAllocationData_1_0_FIXED_PORT_ID_)
        {
            // PLUG AND PLAY (Dovrei ricevere solo in anonimo)
            size_t size = transfer->payload_size;
            uavcan_pnp_NodeIDAllocationData_1_0 msg = {0};
            if (uavcan_pnp_NodeIDAllocationData_1_0_deserialize_(&msg, static_cast<uint8_t const*>(transfer->payload), &size) >= 0) {
                processMessagePlugAndPlayNodeIDAllocation(clsCanard, &msg);
            }
        }
        else
        {
            // Gestione di un messaggio senza sottoscrizione. Se arrivo quà è un errore di sviluppo
            LOCAL_ASSERT(false);
        }
    }
    else if (transfer->metadata.transfer_kind == CanardTransferKindRequest)
    {
        // Gestione delle richieste esterne
        if (transfer->metadata.port_id == clsCanard.port_id.service_module_th) {
            // Richiesta ai dati e metodi di sensor drive
            rmap_service_module_TH_Request_1_0 req = {0};
            size_t size = transfer->payload_size;
            Serial.println(F("<<-- Ricevuto richiesta dati da master"));
            // The request object is empty so we don't bother deserializing it. Just send the response.
            if (rmap_service_module_TH_Request_1_0_deserialize_(&req, static_cast<uint8_t const*>(transfer->payload), &size) >= 0) {
                // I dati e metadati sono direttamente popolati in processRequestGetModuleData
                rmap_service_module_TH_Response_1_0 module_th_resp = processRequestGetModuleData(clsCanard, &req);
                // Serialize and publish the message:
                uint8_t serialized[rmap_service_module_TH_Response_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
                size_t serialized_size = sizeof(serialized);
                const int8_t res = rmap_service_module_TH_Response_1_0_serialize_(&module_th_resp, &serialized[0], &serialized_size);
                if (res >= 0) {
                    // Risposta standard ad un secondo dal timeStamp Sincronizzato
                    clsCanard.sendResponse(MEGA, &transfer->metadata, serialized_size, &serialized[0]);
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
                // Risposta standard ad un secondo dal timeStamp Sincronizzato
                clsCanard.sendResponse(MEGA, &transfer->metadata, serialized_size, &serialized[0]);
            }
        }
        else if (transfer->metadata.port_id == uavcan_register_Access_1_0_FIXED_PORT_ID_)
        {
            uavcan_register_Access_Request_1_0 req = {0};
            size_t size = transfer->payload_size;
            Serial.println(F("<<-- Ricevuto richiesta accesso ai registri da master"));
            if (uavcan_register_Access_Request_1_0_deserialize_(&req, static_cast<uint8_t const*>(transfer->payload), &size) >= 0) {
                const uavcan_register_Access_Response_1_0 resp = processRequestRegisterAccess(&req);
                uint8_t serialized[uavcan_register_Access_Response_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
                size_t serialized_size = sizeof(serialized);
                if (uavcan_register_Access_Response_1_0_serialize_(&resp, &serialized[0], &serialized_size) >= 0) {
                    // Risposta standard ad un secondo dal timeStamp Sincronizzato
                    clsCanard.sendResponse(MEGA, &transfer->metadata, serialized_size, &serialized[0]);
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
                    // Risposta standard ad un secondo dal timeStamp Sincronizzato
                    clsCanard.sendResponse(MEGA, &transfer->metadata, serialized_size, &serialized[0]);
                }
            }
        }
        else if (transfer->metadata.port_id == uavcan_node_ExecuteCommand_1_1_FIXED_PORT_ID_)
        {
            uavcan_node_ExecuteCommand_Request_1_1 req = {0};
            size_t size = transfer->payload_size;
            Serial.println(F("<<-- Ricevuto comando esterno"));
            if (uavcan_node_ExecuteCommand_Request_1_1_deserialize_(&req, static_cast<uint8_t const*>(transfer->payload), &size) >= 0) {
                const uavcan_node_ExecuteCommand_Response_1_1 resp = processRequestExecuteCommand(clsCanard, &req, transfer->metadata.remote_node_id);
                uint8_t serialized[uavcan_node_ExecuteCommand_Response_1_1_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
                size_t serialized_size = sizeof(serialized);
                if (uavcan_node_ExecuteCommand_Response_1_1_serialize_(&resp, &serialized[0], &serialized_size) >= 0) {
                    // Risposta standard ad un secondo dal timeStamp Sincronizzato
                    clsCanard.sendResponse(MEGA, &transfer->metadata, serialized_size, &serialized[0]);
                }
            }
        }
        else
        {
            // Gestione di una richiesta senza controllore locale. Se arrivo quà è un errore di sviluppo
            LOCAL_ASSERT(false);
        }
    }
    else if (transfer->metadata.transfer_kind == CanardTransferKindResponse)
    {
        if (transfer->metadata.port_id == uavcan_file_Read_1_1_FIXED_PORT_ID_) {
            // Accetto solo messaggi indirizzati dal node_id che ha fatto la richiesta di upload
            // E' una sicurezza per il controllo dell'upload, ed evità errori di interprete
            // Inoltre non accetta messaggi extra standard UAVCAN, necessarià prima la CALL al comando
            // SEtFirmwareUpload o SetFileUpload, che impostano il node_id, resettato su EOF dalla classe
            if (clsCanard.master.file.get_server_node() == transfer->metadata.remote_node_id) {
                uavcan_file_Read_Response_1_1 resp  = {0};
                size_t                         size = transfer->payload_size;
                if (uavcan_file_Read_Response_1_1_deserialize_(&resp, static_cast<uint8_t const*>(transfer->payload), &size) >= 0) {
                    if(clsCanard.master.file.is_firmware()) {
                        Serial.print(F("RX FIRMWARE READ BLOCK LEN: "));
                    } else {
                        Serial.print(F("RX FILE READ BLOCK LEN: "));
                    }
                    Serial.println(resp.data.value.count);
                    // Save Data in File at Block Position (Init = Rewrite file...)
                    putDataFile(clsCanard.master.file.get_name(), clsCanard.master.file.is_firmware(), clsCanard.master.file.is_first_data_block(),
                                resp.data.value.elements, resp.data.value.count);
                    // Reset pending command (Comunico request/Response Serie di comandi OK!!!)
                    // Uso l'Overload con controllo di EOF (-> EOF se msgLen != UAVCAN_BLOCK_DEFAULT [256 Bytes])
                    // Questo Overload gestisce in automatico l'offset del messaggio, per i successivi blocchi
                    clsCanard.master.file.reset_pending(resp.data.value.count);
                }
            } else {
                // Errore Nodo non settato...
                Serial.println(F("RX FILE READ BLOCK REJECT: Node_Id not valid or not set"));
            }
        }
    }
    else
    {
        // Se arrivo quà è un errore di sviluppo, controllare setup sottoscrizioni e Rqst (non gestito slave)
        LOCAL_ASSERT(false);
    }
}

// *********************************************************************************************
//          Inizializzazione generale HW, canard, CAN_BUS, e dispositivi collegati
// *********************************************************************************************

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
        LOCAL_ASSERT(false);
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
        LOCAL_ASSERT(false);
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
    LOCAL_ASSERT(uavcan_register_Value_1_0_is_natural32_(&val) && (val.natural32.value.count == 2));

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
            LOCAL_ASSERT(false);
            return false;
        }
    }
    // Attivazione bxCAN sulle interfacce richieste, velocità e modalità
    result = bxCANConfigure(0, timings, false);
    if (!result) {
        Serial.println(F("Error initialization bxCANConfigure"));
        LOCAL_ASSERT(false);
        return false;
    }
    // *******************     CANARD SETUP TIMINGS AND SPEED COMPLETE   *******************

    // Check error starting CAN
    if (HAL_CAN_Start(&CAN_Handle) != HAL_OK) {
        Serial.println(F("CAN startup ERROR!!!"));
        LOCAL_ASSERT(false);
        return false;
    }

    // Enable Interrupt RX Standard CallBack -> CAN1_RX0_IRQHandler
    if (HAL_CAN_ActivateNotification(&CAN_Handle, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
        Serial.println(F("Error initialization interrupt CAN base"));
        LOCAL_ASSERT(false);
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
    while (!Serial) {}
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
        LOCAL_ASSERT(false);
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
        LOCAL_ASSERT(false);
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
    // Avvia l'istanza alla classe State_Canard ed inizializza Ram, Buffer e variabili base
    canardClass clsCanard;

    // Avvio inizializzazione (Standard UAVCAN MSG). Reset su INIT END OK
    // Segnale al Master necessità di impostazioni ev. parametri, Data/Ora ecc..
    clsCanard.flag.set_local_node_mode(uavcan_node_Mode_1_0_INITIALIZATION);

    // Attiva il callBack su RX Messaggio Canard sulla funzione interna processReceivedTransfer
    clsCanard.setReceiveMessage_CB(processReceivedTransfer);
     
    // ********************************************************************************
    //                   READING REGISTER FROM E2 MEMORY / FLASH / SDCARD
    // ********************************************************************************

    // *********               Lettura Registri standard UAVCAN               *********

    // Restore the node-ID from the corresponding standard register. Default to anonymous.
    #ifdef NODE_SLAVE_ID
    // Canard Slave NODE ID Fixed dal defined value in module_config
    clsCanard.set_canard_node_id((CanardNodeID)NODE_SLAVE_ID);
    #else
    uavcan_register_Value_1_0_select_natural16_(&val);
    val.natural16.value.count = 1;
    val.natural16.value.elements[0] = UINT16_MAX; // This means undefined (anonymous), per Specification/libcanard.
    registerRead("uavcan.node.id", &val);         // The names of the standard registers are regulated by the Specification.
    LOCAL_ASSERT(uavcan_register_Value_1_0_is_natural16_(&val) && (val.natural16.value.count == 1));
    if (val.natural16.value.elements[0] <= CANARD_NODE_ID_MAX) {
        clsCanard.set_canard_node_id((CanardNodeID)val.natural16.value.elements[0]);
    }
    #endif

    // The description register is optional but recommended because it helps constructing/maintaining large networks.
    // It simply keeps a human-readable description of the node that should be empty by default.
    uavcan_register_Value_1_0_select_string_(&val);
    val._string.value.count = 0;
    registerRead("uavcan.node.description", &val);  // We don't need the value, we just need to ensure it exists.

    // Carico i/il port-ID/subject-ID del modulo locale dai registri relativi associati nel namespace UAVCAN
    clsCanard.port_id.publisher_module_th =
        getModeAccessID(canardClass::Introspection_Port::PublisherSubjectID,
            "TH.data_and_metadata", rmap_module_TH_1_0_FULL_NAME_AND_VERSION_);

    clsCanard.port_id.service_module_th =
        getModeAccessID(canardClass::Introspection_Port::ServicePortID,
            "TH.service_data_and_metadata", rmap_service_module_TH_1_0_FULL_NAME_AND_VERSION_);

    // Lettura dei registri RMAP al modulo corrente, con impostazione di default x Startup/Init value

    // TODO: Leggere i registri complessivi ITH MTH ECC. di modulo,
    // Inserire funzione per test registro assert... natural 16 e setup defaul x ogni chiamata
    // Lettura dei registri RMAP 16 Bit relativi al modulo corrente

    // ************************* LETTURA REGISTRI METADATI RMAP ****************************
    // ecc... per tutti i metatdati di tutti i sensori
    // Selezionare dai registri i valori fissi (solo 1 e quelli dinamici...)
    // TODO: Per adesso solìno tutti uguali, sistemare con Marco
    // *********************************** L1 *********************************************    
    uavcan_register_Value_1_0_select_natural16_(&val);
    val.natural16.value.count = 1;
    val.natural16.value.elements[0] = UINT16_MAX;
    registerRead("rmap.module.TH.metadata.Level.L1", &val);
    LOCAL_ASSERT(uavcan_register_Value_1_0_is_natural16_(&val) && (val.natural16.value.count == 1));
    clsCanard.module_th.ITH.metadata.level.L1.value = val.natural16.value.elements[0];
    clsCanard.module_th.MTH.metadata.level.L1.value = val.natural16.value.elements[0];
    clsCanard.module_th.NTH.metadata.level.L1.value = val.natural16.value.elements[0];
    clsCanard.module_th.XTH.metadata.level.L1.value = val.natural16.value.elements[0];    
    // *********************************** L2 *********************************************    
    uavcan_register_Value_1_0_select_natural16_(&val);
    val.natural16.value.count = 1;
    val.natural16.value.elements[0] = UINT16_MAX;
    registerRead("rmap.module.TH.metadata.Level.L2", &val);
    LOCAL_ASSERT(uavcan_register_Value_1_0_is_natural16_(&val) && (val.natural16.value.count == 1));
    clsCanard.module_th.ITH.metadata.level.L2.value = val.natural16.value.elements[0];
    clsCanard.module_th.MTH.metadata.level.L2.value = val.natural16.value.elements[0];
    clsCanard.module_th.NTH.metadata.level.L2.value = val.natural16.value.elements[0];
    clsCanard.module_th.XTH.metadata.level.L2.value = val.natural16.value.elements[0];    
    // ******************************* LevelType1 *****************************************    
    uavcan_register_Value_1_0_select_natural16_(&val);
    val.natural16.value.count = 1;
    val.natural16.value.elements[0] = UINT16_MAX;
    registerRead("rmap.module.TH.metadata.Level.LevelType1", &val);
    LOCAL_ASSERT(uavcan_register_Value_1_0_is_natural16_(&val) && (val.natural16.value.count == 1));
    clsCanard.module_th.ITH.metadata.level.LevelType1.value = val.natural16.value.elements[0];
    clsCanard.module_th.MTH.metadata.level.LevelType1.value = val.natural16.value.elements[0];
    clsCanard.module_th.NTH.metadata.level.LevelType1.value = val.natural16.value.elements[0];
    clsCanard.module_th.XTH.metadata.level.LevelType1.value = val.natural16.value.elements[0];    
    // ******************************* LevelType2 *****************************************    
    uavcan_register_Value_1_0_select_natural16_(&val);
    val.natural16.value.count = 1;
    val.natural16.value.elements[0] = UINT16_MAX;
    registerRead("rmap.module.TH.metadata.Level.LevelType2", &val);
    LOCAL_ASSERT(uavcan_register_Value_1_0_is_natural16_(&val) && (val.natural16.value.count == 1));
    clsCanard.module_th.ITH.metadata.level.LevelType2.value = val.natural16.value.elements[0];
    clsCanard.module_th.MTH.metadata.level.LevelType2.value = val.natural16.value.elements[0];
    clsCanard.module_th.NTH.metadata.level.LevelType2.value = val.natural16.value.elements[0];
    clsCanard.module_th.XTH.metadata.level.LevelType2.value = val.natural16.value.elements[0];    
    // *********************************** P1 *********************************************    
    uavcan_register_Value_1_0_select_natural16_(&val);
    val.natural16.value.count = 1;
    val.natural16.value.elements[0] = UINT16_MAX;
    registerRead("rmap.module.TH.metadata.Timerange.P1", &val);
    LOCAL_ASSERT(uavcan_register_Value_1_0_is_natural16_(&val) && (val.natural16.value.count == 1));
    clsCanard.module_th.ITH.metadata.timerange.P1.value = val.natural16.value.elements[0];
    clsCanard.module_th.MTH.metadata.timerange.P1.value = val.natural16.value.elements[0];
    clsCanard.module_th.NTH.metadata.timerange.P1.value = val.natural16.value.elements[0];
    clsCanard.module_th.XTH.metadata.timerange.P1.value = val.natural16.value.elements[0];    
    // *********************************** P2 *********************************************
    // P2 Non memorizzato sul modulo, parametro dipendente dall'acquisizione locale
    clsCanard.module_th.ITH.metadata.timerange.P2 = 900;
    clsCanard.module_th.MTH.metadata.timerange.P2 = 900;
    clsCanard.module_th.NTH.metadata.timerange.P2 = 900;
    clsCanard.module_th.XTH.metadata.timerange.P2 = 900;
    // *********************************** P2 *********************************************    
    uavcan_register_Value_1_0_select_natural8_(&val);
    val.natural16.value.count = 1;
    val.natural16.value.elements[0] = UINT8_MAX;
    registerRead("rmap.module.TH.metadata.Timerange.Pindicator", &val);
    LOCAL_ASSERT(uavcan_register_Value_1_0_is_natural8_(&val) && (val.natural16.value.count == 1));
    clsCanard.module_th.ITH.metadata.timerange.Pindicator.value = val.natural16.value.elements[0];
    clsCanard.module_th.MTH.metadata.timerange.Pindicator.value = val.natural16.value.elements[0];
    clsCanard.module_th.NTH.metadata.timerange.Pindicator.value = val.natural16.value.elements[0];
    clsCanard.module_th.XTH.metadata.timerange.Pindicator.value = val.natural16.value.elements[0];

    // ********************************************************************************
    //               AVVIA SOTTOSCRIZIONI ai messaggi per servizi RPC ecc...
    // ********************************************************************************

    // Plug and Play Versione 1_0 CAN_CLASSIC senza nodo ID valido
    if (clsCanard.is_canard_node_anonymous()) {
        // PnP over Classic CAN, use message v1.0
        if (!clsCanard.rxSubscribe(CanardTransferKindMessage,
                                uavcan_pnp_NodeIDAllocationData_1_0_FIXED_PORT_ID_,
                                uavcan_pnp_NodeIDAllocationData_1_0_EXTENT_BYTES_,
                                CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
            LOCAL_ASSERT(false);
        }
    }

    // Service Client: -> Verifica della presenza Heartbeat del MASTER [Networks OffLine]
    if (!clsCanard.rxSubscribe(CanardTransferKindMessage,
                            uavcan_node_Heartbeat_1_0_FIXED_PORT_ID_,
                            uavcan_node_Heartbeat_1_0_EXTENT_BYTES_,
                            CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
        LOCAL_ASSERT(false);
    }

    // Service Client: -> Sincronizzazione timestamp Microsecond del MASTER [su base time local]
    if (!clsCanard.rxSubscribe(CanardTransferKindMessage,
                            uavcan_time_Synchronization_1_0_FIXED_PORT_ID_,
                            uavcan_time_Synchronization_1_0_EXTENT_BYTES_,
                            CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
        LOCAL_ASSERT(false);
    }

    // Service servers: -> Risposta per GetNodeInfo richiesta esterna master (Yakut, Altri)
    if (!clsCanard.rxSubscribe(CanardTransferKindRequest,
                            uavcan_node_GetInfo_1_0_FIXED_PORT_ID_,
                            uavcan_node_GetInfo_Request_1_0_EXTENT_BYTES_,
                            CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
        LOCAL_ASSERT(false);
    }
    
    // Service servers: -> Risposta per ExecuteCommand richiesta esterna master (Yakut, Altri)
    if (!clsCanard.rxSubscribe(CanardTransferKindRequest,
                            uavcan_node_ExecuteCommand_1_1_FIXED_PORT_ID_,
                            uavcan_node_ExecuteCommand_Request_1_1_EXTENT_BYTES_,
                            CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
        LOCAL_ASSERT(false);
    }

    // Service servers: -> Risposta per Accesso ai registri richiesta esterna master (Yakut, Altri)
    if (!clsCanard.rxSubscribe(CanardTransferKindRequest,
                            uavcan_register_Access_1_0_FIXED_PORT_ID_,
                            uavcan_register_Access_Request_1_0_EXTENT_BYTES_,
                            CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
        LOCAL_ASSERT(false);
    }

    // Service servers: -> Risposta per Lista dei registri richiesta esterna master (Yakut, Altri)
    // Time OUT Canard raddoppiato per elenco registri (Con molte Call vado in TimOut)
    // Con raddoppio del tempo Default problema risolto
    if (!clsCanard.rxSubscribe(CanardTransferKindRequest,
                            uavcan_register_List_1_0_FIXED_PORT_ID_,
                            uavcan_register_List_Request_1_0_EXTENT_BYTES_,
                            CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
        LOCAL_ASSERT(false);
    }

    // Service servers: -> Risposta per dati e metadati sensore modulo corrente da master (Yakut, Altri)
    if (!clsCanard.rxSubscribe(CanardTransferKindRequest,
                            clsCanard.port_id.service_module_th,
                            rmap_service_module_TH_Request_1_0_EXTENT_BYTES_,
                            CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
        LOCAL_ASSERT(false);
    }

    // Service client: -> Risposta per Read (Receive) File local richiesta esterna (Yakut, Altri)
    if (!clsCanard.rxSubscribe(CanardTransferKindResponse,
                            uavcan_file_Read_1_1_FIXED_PORT_ID_,
                            uavcan_file_Read_Response_1_1_EXTENT_BYTES_,
                            CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
        LOCAL_ASSERT(false);
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
    bool masterOnline = false;

    #define MILLIS_EVENT 10
    #define PUBLISH_HEARTBEAT
    #define PUBLISH_LISTPORT
    // #define LED_ON_CAN_DATA_TX
    // #define LED_ON_CAN_DATA_RX
    #define LED_ON_SYNCRO_TIME
    #define LOG_RX_PACKET

    // Set START Timetable LOOP RX/TX. Set Canard microsecond start, per le sincronizzazioni
    clsCanard.getMicros(clsCanard.start_syncronization);
    const CanardMicrosecond fast_loop_period    = MEGA / 3;
    CanardMicrosecond next_333_ms_iter_at = clsCanard.getMicros(clsCanard.syncronized_time) + fast_loop_period;
    CanardMicrosecond next_01_sec_iter_at = clsCanard.getMicros(clsCanard.syncronized_time) + MEGA;
    CanardMicrosecond next_20_sec_iter_at = clsCanard.getMicros(clsCanard.syncronized_time) + MEGA * 1.5;

    // Avvio il modulo UAVCAN in modalità operazionale normale
    // Eventuale SET Flag dopo acquisizione di configurazioni e/o parametri da Remoto
    clsCanard.flag.set_local_node_mode(uavcan_node_Mode_1_0_OPERATIONAL);

    do {
        // Set Canard microsecond corrente monotonic, per le attività temporanee di ciclo
        clsCanard.getMicros(clsCanard.start_syncronization);

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
        // Utilizzo di RTC o locale o generato dal tickMicros locale a partire dall' ultimo SetTimeStamp
        // E' utilizzabile come RTC_FAKE e/o come Setup Perfetto per regolazione RTC al cambio del secondo
        // RTC Infatti non permette la regolazione dei microsecondi, e questa tecnica lo consente
        // Verifico LED al secondo... su timeSTamp sincronizzato remoto
        if((clsCanard.master.timestamp.get_timestamp_syncronized() / MEGA) % 2) {
            digitalWrite(LED_BUILTIN, HIGH);
        } else {
            digitalWrite(LED_BUILTIN, LOW);
        }
        #endif

        // ************************************************************************
        // ***********               CHECK OFFLINE/ONLINE               ***********
        // ************************************************************************
        // Check eventuale Nodo Master OFFLINE (Ultimo comando sempre perchè posso)
        // Effettuare eventuali operazioni di SET,RESET Cmd in sicurezza
        // Entro in OffLine ed eseguo eventuali operazioni di entrata
        if (clsCanard.master.heartbeat.is_online()) {
            // Solo quando passo da OffLine ad OnLine controllo con VarLocale
            if (masterOnline != true) {
                Serial.println(F("Master controller ONLINE !!! -> OnLineFunction()"));
                // .... Codice OnLineFunction()
            }
            masterOnline = true;
            // **************************************************************************
            //            STATO MODULO (Decisionale in funzione di stato remoto)
            // Gestione continuativa del modulo di acquisizione master.clsCanard (flag remoto)
            // **************************************************************************
            // TODO: SOLO CODICE DI ESEMPIO DA GESTIRE
            // Il master comunica nell'HeartBeat il proprio stato che viene gestito qui se OnLine
            // Gestione attività (es. risparmio energetico, altro in funzione di codice remoto)
            /*
            switch (clsCanard.master.state) {
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
        } else {
            // Solo quando passo da OffLine ad OnLine
            if (masterOnline) {
                Serial.println(F("Master controller OFFLINE !!! ALERT -> OffLineFunction()"));
                // .... Codice OffLineFunction()
            }
            masterOnline = false;
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
            // Verifica file download in corso (entro se in download)
            // Attivato da ricezione comando appropriato rxFile o rxFirmware
            if(clsCanard.master.file.download_request()) {
                if(clsCanard.master.file.is_firmware())
                    // Set Flag locale per comunicazione HeartBeat... uploading OK in corso
                    // Utilizzo locale per blocco procedure, sleep ecc. x Uploading
                    clsCanard.flag.set_local_fw_uploading(true);
                // Controllo eventuale timeOut del comando o RxBlocco e gestisco le Retry
                // Verifica TimeOUT Occurs per File download
                if(clsCanard.master.file.event_timeout()) {
                    Serial.println(F("Time OUT File... event occurs"));
                    // Gestione Retry previste dal comando per poi abbandonare
                    uint8_t retry; // In overload x LOGGING
                    if(clsCanard.master.file.next_retry(&retry)) {
                        Serial.print(F("Next Retry File read: "));
                        Serial.println(retry);
                    } else {
                        Serial.println(F("MAX Retry File occurs, download file ABORT !!!"));
                        clsCanard.master.file.download_end();
                    }
                }
                // Se messaggio in pending non faccio niente è attendo la fine del comando in run
                // In caso di errore subentrerà il TimeOut e verrà essere gestita la retry
                if(!clsCanard.master.file.is_pending()) {
                    // Fine pending, con messaggio OK. Verifico se EOF o necessario altro blocco
                    if(clsCanard.master.file.is_download_complete()) {
                        if(clsCanard.master.file.is_firmware()) {
                            Serial.println(F("RX FIRMWARE COMPLETED !!!"));
                        } else {
                            Serial.println(F("RX FILE COMPLETED !!!"));
                        }
                        Serial.println(clsCanard.master.file.get_name());
                        Serial.print(F("Size: "));
                        Serial.print(getDataFileInfo(clsCanard.master.file.get_name(), clsCanard.master.file.is_firmware()));
                        Serial.println(F(" (bytes)"));
                        // Nessun altro evento necessario, chiudo File e stati
                        // procedo all'aggiornamento Firmware dopo le verifiche di conformità
                        // Ovviamente se si tratta di un file firmware
                        clsCanard.master.file.download_end();
                        // Comunico a HeartBeat (Yakut o Altri) l'avvio dell'aggiornamento (se il file è un firmware...)
                        // Per Yakut Pubblicare un HeartBeat prima dell'Avvio quindi con il flag
                        // clsCanard.local_node.file.updating_run = true >> HeartBeat Counica Upgrade...
                        if(clsCanard.master.file.is_firmware()) {
                            clsCanard.flag.set_local_node_mode(uavcan_node_Mode_1_0_SOFTWARE_UPDATE);
                        }
                        // Il Firmware Upload dovrà partire necessariamente almeno dopo l'invio completo
                        // di HeartBeat (svuotamento coda), quindi attendiamo 2/3 secondi poi via
                        // Counque non rispondo più ai comandi di update con file.updating_run = true
                        // FirmwareUpgrade(*NameFile)... -> Fra 2/3 secondi dopo HeartBeat
                    } else {
                        // Avvio prima request o nuovo blocco (Set Flag e TimeOut)
                        // Prima request (clsCanard.local_node.file.offset == 0)
                        // Firmmware Posizione blocco gestito automaticamente in sequenza Request/Read
                        // Gestione retry (incremento su TimeOut/Error) Automatico in Init/Request-Response
                        // Esco se raggiunga un massimo numero di retry x Frame... sopra
                        // Get Data Block per popolare il File
                        // Se il Buffer è pieno = 256 Caratteri è necessario continuare
                        // Altrimenti se inferiore o (0 Compreso) il trasferimento file termina.
                        // Se = 0 il blocco finale non viene considerato ma serve per il protocollo
                        // Se l'ultimo buffer dovesse essere pieno discrimina l'eventualità di MaxBuf==Eof 
                        clsCanard.master_file_read_block_pending(NODE_GETFILE_TIMEOUT_US);
                    }
                }
            }
        }

        // -> Scheduler temporizzato dei messaggi standard da inviare alla rete UAVCAN 

        // LOOP HANDLER >> FAST << 333 msec >> Publisher Data se abilitato
        if (clsCanard.getMicros(clsCanard.syncronized_time) >= next_333_ms_iter_at)
        {
            next_333_ms_iter_at += fast_loop_period;
            // Funzione locale privata
            publish_rmap_data(clsCanard);
        }

        // LOOP HANDLER >> 1 SECONDO << HEARTBEAT
        if (clsCanard.getMicros(clsCanard.syncronized_time) >= next_01_sec_iter_at) {
            #ifdef PUBLISH_HEARTBEAT
            if(clsCanard.is_canard_node_anonymous()) {
                Serial.println(F("Publish SLAVE PNP Request Message -->> [2 sec]"));
                clsCanard.slave_pnp_send_request();
                next_01_sec_iter_at += MEGA * 2;
            } else {
                Serial.println(F("Publish SLAVE Heartbeat -->> [1 sec]"));
                clsCanard.slave_heartbeat_send_message();
                next_01_sec_iter_at += MEGA;
            }            
            #endif
        }

        // LOOP HANDLER >> 20 SECONDI PUBLISH SERVIZI <<
        if (clsCanard.getMicros(clsCanard.syncronized_time) >= next_20_sec_iter_at) {
            #ifdef PUBLISH_LISTPORT
            Serial.println(F("Publish Local PORT LIST -->> [20 sec]"));
            #endif
            next_20_sec_iter_at += MEGA * 20;
            clsCanard.slave_servicelist_send_message();
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
        // Transmit pending frames, avvia la trasmissione gestita da canard a priorità.
        // Il metodo può essere chiamato direttamente in preparazione messaggio x coda
        if (clsCanard.transmitQueueDataPresent()) {
            #ifdef LED_ON_CAN_DATA_TX
            digitalWrite(LED_BUILTIN, HIGH);
            #endif
            clsCanard.transmitQueue();
        }

        // ***************************************************************************
        //   Gestione Coda messaggi in ricezione (ciclo di caricamento messaggi)
        // ***************************************************************************
        // Gestione con Intererupt RX Only esterna (verifica dati in coda gestionale)
        if (clsCanard.receiveQueueDataPresent()) {
            #ifdef LED_ON_CAN_DATA_RX
            digitalWrite(LED_BUILTIN, HIGH);
            #endif
            // Log Packet
            #ifdef LOG_RX_PACKET
            char logMsg[50];
            clsCanard.receiveQueue(logMsg);
            Serial.println(logMsg);
            #else
            clsCanard.receiveQueue();
            #endif
        }
 
    } while (!clsCanard.flag.is_requested_system_restart());

    // Reboot
    NVIC_SystemReset();
}