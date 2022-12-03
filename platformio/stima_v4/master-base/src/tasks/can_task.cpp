/**
  ******************************************************************************
  * @file    can_task.cpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   Uavcan over CanBus cpp_Freertos source file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (C) 2022  Moreno Gasperini <m.gasperini@digiteco.it>
  * All rights reserved.</center></h2>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the GNU General Public License
  * as published by the Free Software Foundation; either version 2
  * of the License, or (at your option) any later version.
  * 
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  * 
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
  * <http://www.gnu.org/licenses/>.
  * 
  ******************************************************************************
*/

#define TRACE_LEVEL CAN_TASK_TRACE_LEVEL

#include "tasks/can_task.h"

using namespace cpp_freertos;

// ***************************************************************************************************
// **********             Funzioni ed utility generiche per gestione UAVCAN                 **********
// ***************************************************************************************************

/// @brief Enable Power CAN_Circuit TJA1443
/// @param ModeCan (Mode TYPE CAN_BUS)
void CanTask::HW_CAN_Power(CAN_ModePower ModeCan) {
    // Normal Mode (TX/RX Full functionally)
    if(ModeCan == CAN_ModePower::CAN_INIT) {
        canPower = ModeCan;
        digitalWrite(PIN_CAN_STB, LOW);
        digitalWrite(PIN_CAN_STB, LOW);
        // Waiting min of 5 uS for Full Operational Setup bxCan && Hal_Can_Init
        delayMicroseconds(10);
        digitalWrite(PIN_CAN_STB, HIGH);
        digitalWrite(PIN_CAN_EN, HIGH);
    }
    // Exit if state is the same
    if (canPower != ModeCan) return;
    // Normal Mode (TX/RX Full functionally)
    if(ModeCan == CAN_ModePower::CAN_NORMAL) {
        digitalWrite(PIN_CAN_STB, HIGH);
        delayMicroseconds(10);
        digitalWrite(PIN_CAN_EN, HIGH);
        // Waiting min of 65 uS for Full Operational External CAN Power Circuit
        // Perform secure WakeUp Timer with 100 uS
        delayMicroseconds(90);
    }
    // Listen Mode (Only RX circuit enabled for first paket data)
    if(ModeCan == CAN_ModePower::CAN_LISTEN_ONLY) {
        digitalWrite(PIN_CAN_EN, LOW);
        delayMicroseconds(10);
        digitalWrite(PIN_CAN_STB, HIGH);
    }
    // Sleep (Turn OFF HW and enter sleep mode TJA1443)
    if(ModeCan == CAN_ModePower::CAN_SLEEP) {
        digitalWrite(PIN_CAN_STB, LOW);
        delayMicroseconds(10);
        digitalWrite(PIN_CAN_EN, HIGH);
    }
    // Saving state
    canPower = ModeCan;
}

// Ritorna unique-ID 128-bit del nodo locale. E' utilizzato in uavcan.node.GetInfo.Response e durante
// plug-and-play node-ID allocation da uavcan.pnp.NodeIDAllocationData. SerialNumber, Produttore..
// Dovrebbe essere verificato in uavcan.node.GetInfo.Response per la verifica non sia cambiato Nodo.
// Al momento vengono inseriti 2 BYTE fissi, altri eventuali, che Identificano il Tipo Modulo
void CanTask::getUniqueID(uint8_t out[uavcan_node_GetInfo_Response_1_0_unique_id_ARRAY_CAPACITY_])
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
    clRegister.read("uavcan.node.unique_id", &value);
    LOCAL_ASSERT(uavcan_register_Value_1_0_is_unstructured_(&value) &&
           value.unstructured.value.count == uavcan_node_GetInfo_Response_1_0_unique_id_ARRAY_CAPACITY_);
    memcpy(&out[0], &value.unstructured.value, uavcan_node_GetInfo_Response_1_0_unique_id_ARRAY_CAPACITY_);
}

// ***************************************************************************************************
//   Funzioni ed utility di ricezione dati dalla rete UAVCAN, richiamati da processReceivedTransfer()
// ***************************************************************************************************

// Chiamate gestioni RPC remote da master (yakut o altro servizio di controllo)
uavcan_node_ExecuteCommand_Response_1_1 CanTask::processRequestExecuteCommand(canardClass &clCanard, const uavcan_node_ExecuteCommand_Request_1_1* req,
                                                                            uint8_t remote_node) {
    uavcan_node_ExecuteCommand_Response_1_1 resp = {0};
    // req->command (Comando esterno ricevuto 2 BYTES RESERVED FFFF-FFFA)
    // Gli altri sono liberi per utilizzo interno applicativo con #define interne
    // req->parameter (array di byte MAX 255 per i parametri da request)
    // Risposta attuale (resp) 1 Bytes RESERVED (0..6) gli altri #define interne
    switch (req->command)
    {
        // **************** Comandi standard UAVCAN GENERIC_SPECIFIC_COMMAND ****************
        // Comando di aggiornamento Firmware compatibile con Yakut e specifice UAVCAN
        case uavcan_node_ExecuteCommand_Request_1_1_COMMAND_BEGIN_SOFTWARE_UPDATE:
        {
            // Nodo Server chiamante (Yakut solo Master, Yakut e Master per Slave)
            clCanard.master.file.start_request(remote_node, (uint8_t*) req->parameter.elements,
                                                req->parameter.count, true);
            clCanard.flag.set_local_fw_uploading(true);
            Serial.print(F("Firmware update request from node id: "));
            Serial.println(clCanard.master.file.get_server_node());
            Serial.print(F("Filename to download: "));
            Serial.println(clCanard.master.file.get_name());
            // Avvio la funzione con OK
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        case uavcan_node_ExecuteCommand_Request_1_1_COMMAND_FACTORY_RESET:
        {
            clRegister.doFactoryReset();
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        case uavcan_node_ExecuteCommand_Request_1_1_COMMAND_RESTART:
        {
            clCanard.flag.request_system_restart();
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
            clCanard.master.file.start_request(remote_node, (uint8_t*) req->parameter.elements,
                                                req->parameter.count, false);
            clCanard.flag.set_local_fw_uploading(true);
            Serial.print(F("File standard update request from node id: "));
            Serial.println(clCanard.master.file.get_server_node());
            Serial.print(F("Filename to download: "));
            Serial.println(clCanard.master.file.get_name());
            // Avvio la funzione con OK
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        case canardClass::Command_Private::enable_publish_port_list:
        {
            // Abilita pubblicazione slow_loop elenco porte (Cypal facoltativo)
            clCanard.publisher_enabled.port_list = true;
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        case canardClass::Command_Private::disable_publish_port_list:
        {
            // Disabilita pubblicazione slow_loop elenco porte (Cypal facoltativo)
            clCanard.publisher_enabled.port_list = false;
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
uavcan_register_Access_Response_1_0 CanTask::processRequestRegisterAccess(const uavcan_register_Access_Request_1_0* req) {
    char name[uavcan_register_Name_1_0_name_ARRAY_CAPACITY_ + 1] = {0};
    LOCAL_ASSERT(req->name.name.count < sizeof(name));
    memcpy(&name[0], req->name.name.elements, req->name.name.count);
    name[req->name.name.count] = '\0';

    uavcan_register_Access_Response_1_0 resp = {0};

    // If we're asked to write a new value, do it now:
    if (!uavcan_register_Value_1_0_is_empty_(&req->value)) {
        uavcan_register_Value_1_0_select_empty_(&resp.value);
        clRegister.read(&name[0], &resp.value);
        // If such register exists and it can be assigned from the request value:
        if (!uavcan_register_Value_1_0_is_empty_(&resp.value) && clRegister.assign(&resp.value, &req->value)) {
            clRegister.write(&name[0], &resp.value);
        }
    }

    // Regardless of whether we've just wrote a value or not, we need to read the current one and return it.
    // The client will determine if the write was successful or not by comparing the request value with response.
    uavcan_register_Value_1_0_select_empty_(&resp.value);
    clRegister.read(&name[0], &resp.value);

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
uavcan_node_GetInfo_Response_1_0 CanTask::processRequestNodeGetInfo() {
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
void CanTask::processReceivedTransfer(canardClass &clCanard, const CanardRxTransfer* const transfer) {
        // Gestione dei Messaggi in ingresso
    if (transfer->metadata.transfer_kind == CanardTransferKindMessage)
    {
        // bool Per assert mancanza handler di eventuale servizio sottoscritto
        bool bKindMessageProcessed = false;
        // Gestione dei messaggi PNP per allocazione nodi di rete (gestisco come master)
        if (transfer->metadata.port_id == uavcan_pnp_NodeIDAllocationData_1_0_FIXED_PORT_ID_)
        {
            // Richiesta di allocazione Nodo dalla rete con messaggio anonimo V1.0 CAN_MTU 8
            bKindMessageProcessed = true;
            uint8_t defaultNodeId = 0;
            size_t size = transfer->payload_size;
            uavcan_pnp_NodeIDAllocationData_1_0 msg = {0};
            if (uavcan_pnp_NodeIDAllocationData_1_0_deserialize_(&msg, static_cast<uint8_t const*>(transfer->payload), &size) >= 0)
            {
                // Cerco nei moduli conosciuti (in HASH_UNIQUE_ID) invio il tipo modulo...
                // Verifico se ho un nodo ancora da configurare come da cfg del master
                // Il nodo deve essere compatibile con il tipo di modulo previsto da allocare
                Serial.print(F("RX PNP Allocation message request from -> "));
                switch(msg.unique_id_hash & 0xFF) {
                    case canardClass::Module_Type::th:
                        Serial.print(F("Anonimous module TH"));
                        defaultNodeId = clCanard.getPNPValidIdFromNodeType(canardClass::Module_Type::th);
                        break;
                    case canardClass::Module_Type::rain:
                        Serial.print(F("Anonimous module RAIN"));
                        defaultNodeId = clCanard.getPNPValidIdFromNodeType(canardClass::Module_Type::rain);
                        break;
                    case canardClass::Module_Type::wind:
                        Serial.print(F("Anonimous module WIND"));
                        defaultNodeId = clCanard.getPNPValidIdFromNodeType(canardClass::Module_Type::wind);
                        break;
                    case canardClass::Module_Type::radiation:
                        Serial.print(F("Anonimous module RADIATION"));
                        defaultNodeId = clCanard.getPNPValidIdFromNodeType(canardClass::Module_Type::radiation);
                        break;
                    case canardClass::Module_Type::vwc:
                        Serial.print(F("Anonimous module VWC"));
                        defaultNodeId = clCanard.getPNPValidIdFromNodeType(canardClass::Module_Type::vwc);
                        break;
                    case canardClass::Module_Type::power:
                        Serial.print(F("Anonimous module POWER"));
                        defaultNodeId = clCanard.getPNPValidIdFromNodeType(canardClass::Module_Type::power);
                        break;
                    default:
                        // PNP Non gestibile
                        Serial.print(F("Anonimous module Unknown"));
                        defaultNodeId = GENERIC_BVAL_UNDEFINED;
                        break;
                }
                Serial.print(F(" [UniqueID:"));
                Serial.print(msg.unique_id_hash, HEX);
                Serial.println(F("]"));

                // Risposta immediata diretta (Se nodo ovviamente è riconosciuto...)
                // Non utilizziamo una Response in quanto l'allocation è sempre un messaggio anonimo
                // I metadati del trasporto sono come quelli riceuti del transferID quindi è un messaggio
                // che si comporta parzialmente come una risposta (per rilevamento remoto hash/transfer_id)
                if(defaultNodeId <= CANARD_NODE_ID_MAX) {
                    Serial.print(F("Try PNP Allocation with Node_ID -> "));
                    Serial.println(defaultNodeId);
                    // Se il nodo proposto viene confermato inizieremo a ricevere heartbeat
                    // da quel nodeId. A questo punto in Heartbeat settiamo il flag pnp.configure()
                    // che conclude la procedura con esito positivo.
                    msg.allocated_node_id.count = 1;
                    msg.allocated_node_id.elements[0].value = defaultNodeId;
                    // The request object is empty so we don't bother deserializing it. Just send the response.
                    uint8_t      serialized[uavcan_pnp_NodeIDAllocationData_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
                    size_t       serialized_size = sizeof(serialized);
                    const int8_t res = uavcan_pnp_NodeIDAllocationData_1_0_serialize_(&msg, &serialized[0], &serialized_size);
                    // Preparo la pubblicazione anonima in risposta alla richiesta anonima
                    const CanardTransferMetadata meta = {
                        .priority       = CanardPriorityNominal,
                        .transfer_kind  = CanardTransferKindMessage,
                        .port_id        = uavcan_pnp_NodeIDAllocationData_1_0_FIXED_PORT_ID_,
                        .remote_node_id = CANARD_NODE_ID_UNSET,
                        .transfer_id    = (CanardTransferID) (transfer->metadata.transfer_id),
                    };
                    clCanard.send(MEGA, &meta, serialized_size, &serialized[0]);
                }
            }
        }
        // Gestione dei messaggi Heartbeat per stato rete (gestisco come master)
        else if (transfer->metadata.port_id == uavcan_node_Heartbeat_1_0_FIXED_PORT_ID_)
        {
            bKindMessageProcessed = true;
            size_t size = transfer->payload_size;
            uavcan_node_Heartbeat_1_0 msg = {0};
            if (uavcan_node_Heartbeat_1_0_deserialize_(&msg, static_cast<uint8_t const*>(transfer->payload), &size) >= 0)
            {
                Serial.print(F("RX HeartBeat from -> "));
                Serial.println(transfer->metadata.remote_node_id);
                // Processo e registro il nodo: stato, OnLine e relativi flag
                uint8_t queueId = clCanard.getSlaveIstanceFromId(transfer->metadata.remote_node_id);
                // Se nodo correttamente allocato e gestito (potrebbe essere Yakut non registrato)
                if (queueId != GENERIC_BVAL_UNDEFINED) {
                    // Primo assegnamento da PNP, gestisco eventuale configurazione remota
                    // e salvataggio del flag di assegnamento in ROM / o Register
                    if(!clCanard.slave[queueId].pnp.is_configured()) {
                        // Configura i metadati...
                        // Configura altri parametri...
                        // Modifico il flag PNP Executed e termino la procedura PNP
                        clCanard.slave[queueId].pnp.disable();
                        // Salvo su registro lo stato
                        uavcan_register_Value_1_0 val = {0};
                        char registerName[24] = "rmap.pnp.allocateID.";
                        uavcan_register_Value_1_0_select_natural8_(&val);
                        val.natural32.value.count       = 1;
                        val.natural32.value.elements[0] = transfer->metadata.remote_node_id;
                        // queueId -> index Val = NodeId
                        itoa(queueId, registerName + strlen(registerName), 10);
                        clRegister.write(registerName, &val);
                    }                    
                    // Accodo i dati letti dal messaggio (Nodo -> OnLine) verso la classe
                    clCanard.slave[queueId].heartbeat.set_online(NODE_OFFLINE_TIMEOUT_US,
                        msg.vendor_specific_status_code, msg.health.value, msg.mode.value, msg.uptime);  
                    // Rientro in OnLINE da OFFLine o Init Gestino può (dovrebbe) essere esterna alla Call
                    // Inizializzo le variabili e gli stati necessari per Reset e corretta gestione
                    if(clCanard.slave[queueId].is_entered_online()) {
                        Serial.println(F("Node Is Now Entered ONLINE !!! "));
                        // Metto i Flag in sicurezza, laddove dove non eventualmente gestito
                        clCanard.slave[queueId].command.reset_pending();
                        clCanard.slave[queueId].register_access.reset_pending();
                        clCanard.slave[queueId].file_server.reset_pending();
                        clCanard.slave[queueId].rmap_service.reset_pending();
                    }
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
            uint8_t queueId = clCanard.getSlaveIstanceFromId(transfer->metadata.remote_node_id);
            // Se nodo correttammente allocato e gestito
            if (queueId != GENERIC_BVAL_UNDEFINED) {
                // Verifico se risposta del servizio corrisponde al chiamante (eventuali + servizi sotto...)
                // Gestione di tutti i servizi possibili allocabili, valido per tutti i nodi
                if (transfer->metadata.port_id == clCanard.slave[queueId].publisher.get_subject_id())
                {                
                    // *************            Service Modulo TH Response            *************
                    if(clCanard.slave[queueId].get_module_type() == canardClass::Module_Type::th) {
                        // Processato il messaggio con il relativo Handler. OK
                        bKindMessageProcessed = true;
                        // Modulo TH, leggo e deserializzo il messaggio in ingresso
                        rmap_module_TH_1_0  msg = {0};
                        size_t             size = transfer->payload_size;
                        if (rmap_module_TH_1_0_deserialize_(&msg, static_cast<uint8_t const*>(transfer->payload), &size) >= 0)
                        {
                            // msg contiene i dati di blocco pubblicati
                            Serial.print(F("Ricevuto dato in publisher modulo_th -> ID: "));
                            Serial.print(transfer->metadata.remote_node_id);
                            Serial.print(F(" , transfer ID: "));
                            Serial.println(transfer->metadata.transfer_id);
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
            LOCAL_ASSERT(false);
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
            if (res >= 0) {
                clCanard.sendResponse(MEGA, &transfer->metadata, serialized_size, &serialized[0]);
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
                if (uavcan_register_Access_Response_1_0_serialize_(&resp, &serialized[0], &serialized_size) >= 0) {
                    clCanard.sendResponse(MEGA, &transfer->metadata, serialized_size, &serialized[0]);
                }
            }
        }
        else if (transfer->metadata.port_id == uavcan_register_List_1_0_FIXED_PORT_ID_)
        {
            uavcan_register_List_Request_1_0 req  = {0};
            size_t                           size = transfer->payload_size;
            if (uavcan_register_List_Request_1_0_deserialize_(&req, static_cast<uint8_t const*>(transfer->payload), &size) >= 0)
            {
                uavcan_register_List_Response_1_0 resp;
                resp.name =  clRegister.getNameByIndex(req.index);
                uint8_t serialized[uavcan_register_List_Response_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
                size_t  serialized_size = sizeof(serialized);
                if (uavcan_register_List_Response_1_0_serialize_(&resp, &serialized[0], &serialized_size) >= 0) {
                    clCanard.sendResponse(MEGA, &transfer->metadata, serialized_size, &serialized[0]);
                }
            }
        }
        else if (transfer->metadata.port_id == uavcan_node_ExecuteCommand_1_1_FIXED_PORT_ID_)
        {
            uavcan_node_ExecuteCommand_Request_1_1 req  = {0};
            size_t                                 size = transfer->payload_size;
            if (uavcan_node_ExecuteCommand_Request_1_1_deserialize_(&req, static_cast<uint8_t const*>(transfer->payload), &size) >= 0)
            {
                const uavcan_node_ExecuteCommand_Response_1_1 resp = processRequestExecuteCommand(clCanard, &req, transfer->metadata.remote_node_id);
                uint8_t serialized[uavcan_node_ExecuteCommand_Response_1_1_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
                size_t  serialized_size = sizeof(serialized);
                if (uavcan_node_ExecuteCommand_Response_1_1_serialize_(&resp, &serialized[0], &serialized_size) >= 0) {
                    clCanard.sendResponse(MEGA, &transfer->metadata, serialized_size, &serialized[0]);
                }
            }
        }
        else if (transfer->metadata.port_id == uavcan_file_Read_1_1_FIXED_PORT_ID_)
        {
            // La funzione viene eseguita solo con UPLOAD Sequenza corretta
            // Serve a fare eseguire un'eventuale TimeOut su procedura non corretta
            // Ed evitare blocchi non coerenti... Viene gestita senza problemi
            // Send multiplo di File e procedure in sequenza ( Gestito TimeOut di Request )
            uint8_t queueId = clCanard.getSlaveIstanceFromId(transfer->metadata.remote_node_id);
            // Devo essere in aggiornamento per sicurezza!!! O al massimo con verifica del comando
            if((clCanard.slave[queueId].file_server.get_state() == clCanard.state_uploading)||
                (clCanard.slave[queueId].file_server.get_state() == clCanard.command_wait)) {
                // Update TimeOut (Comunico request OK al Master, Slave sta scaricando)
                // Se Slave si blocca per TimeOut, esco dalla procedura dove gestita
                clCanard.slave[queueId].file_server.start_pending(NODE_REQFILE_TIMEOUT_US);
                uavcan_file_Read_Request_1_1 req  = {0};
                size_t                       size = transfer->payload_size;
                if (uavcan_file_Read_Request_1_1_deserialize_(&req, static_cast<uint8_t const*>(transfer->payload), &size) >= 0)
                {
                    uavcan_file_Read_Response_1_1 resp = {0};
                    byte dataBuf[uavcan_primitive_Unstructured_1_0_value_ARRAY_CAPACITY_];
                    // Terminatore di sicurezza
                    req.path.path.elements[req.path.path.count] = 0;
                    // Allego il blocco dati se presente
                    size_t dataLen = uavcan_primitive_Unstructured_1_0_value_ARRAY_CAPACITY_;
                    // TODO: Firmware from SD or FLASH
                    // if (getDataFile((char*)&req.path.path.elements[0],
                    //                 clCanard.slave[queueId].file_server.is_firmware(),
                    //                 req.offset, dataBuf, &dataLen)) {
                    //     resp._error.value = uavcan_file_Error_1_0_OK;
                    //     // Controllo se il Master ha finito la trasmissione (specifiche UAVCAN)
                    //     clCanard.slave[queueId].file_server.reset_pending(dataLen);
                    // } else {
                    //     // Ritorno un errore da interpretare a Slave. Non interrompo la procedura
                    //     // E' il client che eventualmente la interrompe. Stop locale solo da TimeOut
                    //     resp._error.value = uavcan_file_Error_1_0_IO_ERROR;
                    //     dataLen = 0;
                    // }
                    // Preparo la risposta corretta
                    resp.data.value.count = dataLen;
                    memcpy(resp.data.value.elements, dataBuf, dataLen);
                    uint8_t serialized[uavcan_file_Read_Response_1_1_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
                    size_t  serialized_size = sizeof(serialized);
                    if (uavcan_file_Read_Response_1_1_serialize_(&resp, &serialized[0], &serialized_size) >= 0) {
                        clCanard.sendResponse(MEGA, &transfer->metadata, serialized_size, &serialized[0]);
                    }
                }
            }
        }
        else
        {
            // Gestione di una richiesta senza controllore locale. Se arrivo quà è un errore di sviluppo
            LOCAL_ASSERT(false);
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
                uint8_t queueId = clCanard.getSlaveIstanceFromId(transfer->metadata.remote_node_id);
                if (queueId != GENERIC_BVAL_UNDEFINED) {
                    // Resetta il pending del comando del nodo verificato
                    clCanard.slave[queueId].command.reset_pending(resp.status);
                }
            }
        }
        else if (transfer->metadata.port_id == uavcan_register_Access_1_0_FIXED_PORT_ID_)
        {
            uavcan_register_Access_Response_1_0 resp = {0};
            size_t                              size = transfer->payload_size;
            if (uavcan_register_Access_Response_1_0_deserialize_(&resp, static_cast<uint8_t const*>(transfer->payload), &size) >= 0)
            {
                // Ricerco idNodo nella coda degli allocati del master
                // Copio la risposta ricevuta nella struttura relativa e resetto il flag pending
                uint8_t queueId = clCanard.getSlaveIstanceFromId(transfer->metadata.remote_node_id);                
                if (queueId != GENERIC_BVAL_UNDEFINED) {
                    // Resetta il pending del comando del nodo verificato con la risposta
                    clCanard.slave[queueId].register_access.reset_pending(resp.value);
                }
            }
        }
        else if (transfer->metadata.port_id == uavcan_file_Read_1_1_FIXED_PORT_ID_)
        {
            // Accetto solo messaggi indirizzati dal node_id che ha fatto la richiesta di upload
            // E' una sicurezza per il controllo dell'upload, ed evità errori di interprete
            // Inoltre non accetta messaggi extra standard UAVCAN, necessarià prima la CALL al comando
            // SEtFirmwareUpload o SetFileUpload, che impostano il node_id, resettato su EOF dalla classe
            if (clCanard.master.file.get_server_node() == transfer->metadata.remote_node_id) {
                uavcan_file_Read_Response_1_1 resp  = {0};
                size_t                         size = transfer->payload_size;
                if (uavcan_file_Read_Response_1_1_deserialize_(&resp, static_cast<uint8_t const*>(transfer->payload), &size) >= 0) {
                    if(clCanard.master.file.is_firmware()) {
                        Serial.print(F("RX FIRMWARE READ BLOCK LEN: "));
                    } else {
                        Serial.print(F("RX FILE READ BLOCK LEN: "));
                    }
                    Serial.println(resp.data.value.count);
                    // TODO: Firmware FILE FROM SD OR FLASH
                    // Save Data in File at Block Position (Init = Rewrite file...)
                    // putDataFile(clCanard.master.file.get_name(), clCanard.master.file.is_firmware(), clCanard.master.file.is_first_data_block(),
                    //             resp.data.value.elements, resp.data.value.count);
                    // Reset pending command (Comunico request/Response Serie di comandi OK!!!)
                    // Uso l'Overload con controllo di EOF (-> EOF se msgLen != UAVCAN_BLOCK_DEFAULT [256 Bytes])
                    // Questo Overload gestisce in automatico l'offset del messaggio, per i successivi blocchi
                    clCanard.master.file.reset_pending(resp.data.value.count);
                }
            } else {
                // Errore Nodo non settato...
                Serial.println(F("RX FILE READ BLOCK REJECT: Node_Id not valid or not set"));
            }
        }
        // Risposta ad un servizio (dati) dinamicamente allocato... ( deve essere ultimo else )
        // Servizio di risposta alla richiesta su modulo slave, verifica della risposta e della coerenza messaggio
        // Per il nodo che risponde verifico i servizi attivi per la corrispondenza dinamica risposta
        else
        {
            // Nodo rispondente (posso avere senza problemi più servizi con stesso port_id su diversi nodi)
            uint8_t queueId = clCanard.getSlaveIstanceFromId(transfer->metadata.remote_node_id);
            // Se nodo correttammente allocato e gestito
            if (queueId != GENERIC_BVAL_UNDEFINED) {
                // Verifico se risposta del servizio corrisponde al chiamante (eventuali + servizi sotto...)
                // Gestione di tutti i servizi possibili allocabili, valido per tutti i nodi
                if (transfer->metadata.port_id == clCanard.slave[queueId].rmap_service.get_port_id())
                {                
                    // *************            Service Modulo TH Response            *************
                    if(clCanard.slave[queueId].get_module_type() == canardClass::Module_Type::th) {
                        // Modulo TH, leggo e deserializzo il messaggio in ingresso
                        rmap_service_module_TH_Response_1_0 resp = {0};
                        size_t                              size = transfer->payload_size;
                        if (rmap_service_module_TH_Response_1_0_deserialize_(&resp, static_cast<uint8_t const*>(transfer->payload), &size) >= 0)
                        {
                            // Resetta il pending del comando del nodo verificato (size_mem preparato in avvio)
                            // Copia la risposta nella variabile di chiamata in state
                            // Oppure possibile gestire qua tutte le occorrenze per stima V4
                            clCanard.slave[queueId].rmap_service.reset_pending(&resp, sizeof(resp));
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
        LOCAL_ASSERT(false);
    }
}


/// *********************************************************************************************
/// @brief Main TASK && INIT TASK --- UAVCAN
/// *********************************************************************************************
CanTask::CanTask(const char *taskName, uint16_t stackSize, uint8_t priority, CanParam_t canParam) : Thread(taskName, stackSize, priority), param(canParam) {
  
  // Setup register mode
  clRegister = EERegister(param.wire, param.wireLock);

  // FullChip Power Mode after Startup
  // Resume from LowPower or reset the controller TJA1443ATK
  // Need FullPower for bxCan Programming (Otherwise Handler_Error()!)
  HW_CAN_Power(CAN_ModePower::CAN_INIT);

  // REGISTER_INIT && Fixed Optional Setup Reset
  #ifdef INIT_REGISTER
  // Inizializzazione fissa dei registri nella modalità utilizzata (prepare SD/FLASH/ROM con default Value)
  clRegister.setup(true);
  #else
  // Default dei registri nella modalità utilizzata (prepare SD/FLASH/ROM con default Value)
  // Creazione dei registri standard base se non esistono
  clRegister.setup(false);
  #endif

  TRACE_INFO_F(F("Starting CAN Configuration"));
  // *******************         CANARD SETUP TIMINGS AND SPEED        *******************
  // CAN BITRATE Dinamico su LoadRegister (CAN_FD 2xREG natural32 0=Speed, 1=0 (Not Used))
  uavcan_register_Value_1_0 val = {0};
  uavcan_register_Value_1_0_select_natural32_(&val);
  val.natural32.value.count       = 2;
  val.natural32.value.elements[0] = CAN_BIT_RATE;
  val.natural32.value.elements[1] = 0ul;          // Ignored for CANARD_MTU_CAN_CLASSIC
  clRegister.read("uavcan.can.bitrate", &val);
  LOCAL_ASSERT(uavcan_register_Value_1_0_is_natural32_(&val) && (val.natural32.value.count == 2));

  // Dynamic BIT RATE Change CAN Speed to CAN_BIT_RATE (register default/defined)
  BxCANTimings timings;
  bool result = bxCANComputeTimings(HAL_RCC_GetPCLK1Freq(), val.natural32.value.elements[0], &timings);
  if (!result) {
      TRACE_INFO_F(F("Error redefinition bxCANComputeTimings, try loading default..."));
      val.natural32.value.count       = 2;
      val.natural32.value.elements[0] = CAN_BIT_RATE;
      val.natural32.value.elements[1] = 0ul;          // Ignored for CANARD_MTU_CAN_CLASSIC
      clRegister.write("uavcan.can.bitrate", &val);
      result = bxCANComputeTimings(HAL_RCC_GetPCLK1Freq(), val.natural32.value.elements[0], &timings);
      if (!result) {
          TRACE_INFO_F(F("Error initialization bxCANComputeTimings"));
          LOCAL_ASSERT(false);
          return;
      }
  }

  // Configurea bxCAN speed && mode
  result = bxCANConfigure(0, timings, false);
  if (!result) {
      TRACE_INFO_F(F("Error initialization bxCANConfigure"));
      LOCAL_ASSERT(false);
      return;
  }
  // *******************     CANARD SETUP TIMINGS AND SPEED COMPLETE   *******************

  // Check error starting CAN
  if (HAL_CAN_Start(&hcan1) != HAL_OK) {
      TRACE_INFO_F(F("CAN startup ERROR!!!"));
      LOCAL_ASSERT(false);
      return;
  }

  // Enable Interrupt RX Standard CallBack -> CAN1_RX0_IRQHandler
  if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
      TRACE_INFO_F(F("Error initialization interrupt CAN base"));
      LOCAL_ASSERT(false);
      return;
  }
  // Setup Priority e CB CAN_IRQ_RX Enable
  HAL_NVIC_SetPriority(CAN1_RX0_IRQn, CAN_NVIC_INT_PREMPT_PRIORITY, 0);
  HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);

  // Setup Complete
  TRACE_INFO_F(F("CAN Configuration complete..."));

  // Run Task Init
  state = INIT;
  Start();
};

/// @brief RUN Task
void CanTask::Run() {

    // Data Local Task (Class + Registro)
    // Avvia l'istanza alla classe State_Canard ed inizializza Ram, Buffer e variabili base
    canardClass clCanard;
    uavcan_register_Value_1_0 val = {0};

    // LoopTimer Publish
    CanardMicrosecond last_pub_heartbeat;
    CanardMicrosecond last_pub_port_list;
    CanardMicrosecond next_timesyncro_msg;

    uint32_t test_millis = millis();
    bool bLowPower = false;

    // TODO: Eliminare
    bool ledShow;
    int bTxAttempt = 0;
    int bRxAttempt = 0;
    long lastMillis = 0;
    long checkTimeout = 0;
    uint8_t stackTimer = 0;
    bool masterOnline = false;

    // Funzioni di modulo TODO: Eliminare -> portare a TRACE
    #define LOG_PUBLISH_RMAP
    #define LOG_HEARTBEAT
    #define LOG_LISTPORT
    #define PUBLISH_HEARTBEAT
    #define PUBLISH_PORTLIST
    #define LOG_RX_PACKET
    #define LOG_STACK_USAGE
    // #define LED_ON_CAN_DATA_TX
    // #define LED_ON_CAN_DATA_RX
    // #define LED_ON_SYNCRO_TIME

    // Main Loop TASK
    while (true) {

        // ********************************************************************************
        //                   SETUP CONFIG CYPAL, CLASS, REGISTER, DATA
        // ********************************************************************************
        switch (state) {
            // Setup Class CB and NodeId
            case INIT:
                // Avvio inizializzazione (Standard UAVCAN MSG). Reset su INIT END OK
                // Segnale al Master necessità di impostazioni ev. parametri, Data/Ora ecc..
                clCanard.flag.set_local_node_mode(uavcan_node_Mode_1_0_INITIALIZATION);

                // Attiva il callBack su RX Messaggio Canard sulla funzione interna processReceivedTransfer
                clCanard.setReceiveMessage_CB(processReceivedTransfer);

                // ********************************************************************************
                //            INIT VALUE, Caricamento default e registri locali MASTER
                // ********************************************************************************

                // Canard Master NODE ID Fixed dal defined value in module_config
                clCanard.set_canard_node_id((CanardNodeID) NODE_MASTER_ID);

                // TODO:
                // Read Config Slave Node x Lettura porte e servizi.
                // Possibilità di utilizzo come sotto (registri) - Fixed Value adesso !!!
                #ifdef USE_SUB_PUBLISH_SLAVE_DATA
                clCanard.slave[0].configure(125, canardClass::Module_Type::th, 100, 0);    
                // clCanard.slave[0].configure(125, canardClass::Module_Type::th, 100, 5678);    
                #else
                clCanard.slave[0].configure(125, canardClass::Module_Type::th, 100);    
                #endif

                // TODO: Register Config ID Node in Array... non N registri
                // **********************************************************************************
                // Lettura registri, parametri per PNP Allocation Verifica locale di assegnamento CFG
                // **********************************************************************************
                for(uint8_t iCnt = 0; iCnt<MAX_NODE_CONNECT; iCnt++) {
                    // Lettura registro di allocazione PNP MASTER Locale avvenuta, da eseguire
                    // Possibilità di salvare tutte le informazioni di NODO qui al suo interno
                    // Per rendere disponibili le configurazioni in esterno (Yakut, altri)
                    // Utilizzando la struttupra allocateID.XX (count = n° registri utili)
                    char registerName[24] = "rmap.pnp.allocateID.";
                    uavcan_register_Value_1_0_select_natural8_(&val);
                    val.natural8.value.count       = 1;
                    val.natural8.value.elements[0] = CANARD_NODE_ID_UNSET;
                    // queueId -> index Val = NodeId
                    itoa(iCnt, registerName + strlen(registerName), 10);
                    clRegister.read(registerName, &val);
                    LOCAL_ASSERT(uavcan_register_Value_1_0_is_natural8_(&val) && (val.natural8.value.count == 1));
                    // Il Node_id deve essere valido e uguale a quello programmato in configurazione
                    if((val.natural8.value.elements[0] != CANARD_NODE_ID_UNSET) &&
                        (val.natural8.value.elements[0] == clCanard.slave[iCnt].get_node_id()))
                    {
                        // Assegnamento PNP per nodeQueueID con clCanard.slave[iCnt].node_id
                        // già avvenuto. Non rispondo a eventuali messaggi PNP del tipo per quel nodo
                        clCanard.slave[iCnt].pnp.disable();
                    }
                }

                // ********************************************************************************
                // *********               Lettura Registri standard UAVCAN               *********
                // ********************************************************************************

                // The description register is optional but recommended because it helps constructing/maintaining large networks.
                // It simply keeps a human-readable description of the node that should be empty by default.
                uavcan_register_Value_1_0_select_string_(&val);
                val._string.value.count = 0;
                clRegister.read("uavcan.node.description", &val);  // We don't need the value, we just need to ensure it exists.

                // Passa alle sottoscrizioni
                state = SETUP;
                break;

            // ********************************************************************************
            //               AVVIA SOTTOSCRIZIONI ai messaggi per servizi RPC ecc...
            // ********************************************************************************
            case SETUP:

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

                // Service servers: -> Risposta per Accesso ai registri richiesta esterna (Yakut, Altri)
                if (!clCanard.rxSubscribe(CanardTransferKindRequest,
                                        uavcan_register_Access_1_0_FIXED_PORT_ID_,
                                        uavcan_register_Access_Request_1_0_EXTENT_BYTES_,
                                        CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                    LOCAL_ASSERT(false);
                }

                // Service servers: -> Risposta per Lista dei registri richiesta esterna (Yakut, Altri)
                if (!clCanard.rxSubscribe(CanardTransferKindRequest,
                                        uavcan_register_List_1_0_FIXED_PORT_ID_,
                                        uavcan_register_List_Request_1_0_EXTENT_BYTES_,
                                        CANARD_REGISTERLIST_TRANSFER_ID_TIMEOUT_USEC)) {
                    LOCAL_ASSERT(false);
                }

                // ******* SOTTOSCRIZIONE MESSAGGI / COMANDI E SERVIZI AD UTILITA' MASTER ********

                // Messaggi PNP_Allocation: -> Allocazione dei nodi standard in PlugAndPlay per i nodi conosciuti
                if (!clCanard.rxSubscribe(CanardTransferKindMessage,
                                        uavcan_pnp_NodeIDAllocationData_1_0_FIXED_PORT_ID_,
                                        uavcan_pnp_NodeIDAllocationData_1_0_EXTENT_BYTES_,
                                        CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                    LOCAL_ASSERT(false);
                }

                // Messaggi HEARTBEAT: -> Verifica della presenza per stato Nodi (Slave) OnLine / OffLine
                if (!clCanard.rxSubscribe(CanardTransferKindMessage,
                                        uavcan_node_Heartbeat_1_0_FIXED_PORT_ID_,
                                        uavcan_node_Heartbeat_1_0_EXTENT_BYTES_,
                                        CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                    LOCAL_ASSERT(false);
                }

                // Service client: -> Risposta per ExecuteCommand richiesta interna (come master)
                if (!clCanard.rxSubscribe(CanardTransferKindResponse,
                                        uavcan_node_ExecuteCommand_1_1_FIXED_PORT_ID_,
                                        uavcan_node_ExecuteCommand_Response_1_1_EXTENT_BYTES_,
                                        CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                    LOCAL_ASSERT(false);
                }

                // Service client: -> Risposta per Accesso ai registri richiesta interna (come master)
                if (!clCanard.rxSubscribe(CanardTransferKindResponse,
                                        uavcan_register_Access_1_0_FIXED_PORT_ID_,
                                        uavcan_register_Access_Response_1_0_EXTENT_BYTES_,
                                        CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                    LOCAL_ASSERT(false);
                }

                // Service client: -> Risposta per Read (Receive) File local richiesta esterna (Yakut, Altri)
                if (!clCanard.rxSubscribe(CanardTransferKindResponse,
                                        uavcan_file_Read_1_1_FIXED_PORT_ID_,
                                        uavcan_file_Read_Response_1_1_EXTENT_BYTES_,
                                        CANARD_READFILE_TRANSFER_ID_TIMEOUT_USEC)) {
                    LOCAL_ASSERT(false);
                }

                // Service server: -> Risposta per Read (Request Slave) File read archivio (come master)
                if (!clCanard.rxSubscribe(CanardTransferKindRequest,
                                        uavcan_file_Read_1_1_FIXED_PORT_ID_,
                                        uavcan_file_Read_Request_1_1_EXTENT_BYTES_,
                                        CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                    LOCAL_ASSERT(false);
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
                    // Se previsto il servizio request/response (!=NULL, quindi allocato) con port_id valido
                    if ((clCanard.slave[queueId].rmap_service.get_response()) &&
                        (clCanard.slave[queueId].rmap_service.get_port_id() <= CANARD_SERVICE_ID_MAX)) {
                        // Controllo le varie tipologie di request/service per il nodo
                        if(clCanard.slave[queueId].get_module_type() == canardClass::Module_Type::th) {     
                            // Alloco la stottoscrizione in funzione del tipo di modulo
                            // Service client: -> Risposta per ServiceDataModuleTH richiesta interna (come master)
                            if (!clCanard.rxSubscribe(CanardTransferKindResponse,
                                                    clCanard.slave[queueId].rmap_service.get_port_id(),
                                                    rmap_service_module_TH_Response_1_0_EXTENT_BYTES_,
                                                    CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                                LOCAL_ASSERT(false);
                            }
                        }
                    }
                    #ifdef USE_SUB_PUBLISH_SLAVE_DATA
                    // *************   PUBLISH    *************
                    // Se previsto il servizio publisher (subject_id valido)
                    // Non alloco niente per il publish (gestione esempio display o altro debug interno da gestire)
                    if (clCanard.slave[queueId].publisher.get_subject_id() <= CANARD_SUBJECT_ID_MAX) {
                        // Controllo le varie tipologie di request/service per il nodo
                        if(clCanard.slave[queueId].get_module_type() == canardClass::Module_Type::th) {            
                            // Alloco la stottoscrizione in funzione del tipo di modulo
                            // Service client: -> Sottoscrizione per ModuleTH (come master)
                            if (!clCanard.rxSubscribe(CanardTransferKindMessage,
                                                    clCanard.slave[queueId].publisher.get_subject_id(),
                                                    rmap_module_TH_1_0_EXTENT_BYTES_,
                                                    CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                                LOCAL_ASSERT(false);
                            }
                        }
                    }
                    #endif
                }

                // Avvio il modulo UAVCAN in modalità operazionale normale
                // Eventuale SET Flag dopo acquisizione di configurazioni e/o parametri da Remoto
                clCanard.flag.set_local_node_mode(uavcan_node_Mode_1_0_OPERATIONAL);

                // Set START Timetable LOOP RX/TX. Set Canard microsecond start, per le sincronizzazioni
                clCanard.getMicros(clCanard.start_syncronization);
                last_pub_heartbeat = clCanard.getMicros(clCanard.syncronized_time) + MEGA * (TIME_PUBLISH_HEARTBEAT + random(100) / 100);
                next_timesyncro_msg = clCanard.getMicros(clCanard.syncronized_time) + MEGA;
                last_pub_port_list = last_pub_heartbeat + MEGA * (0.5);

                // Passo alla gestione Main
                state = STANDBY;
                break;

            // ********************************************************************************
            //         AVVIA LOOP CANARD PRINCIPALE gestione TX/RX Code -> Messaggi
            // ********************************************************************************
            case STANDBY:

                // Set Canard microsecond corrente monotonic, per le attività temporanee di ciclo
                clCanard.getMicros(clCanard.start_syncronization);

                // TEST VERIFICA sincronizzazione time_stamp locale con remoto... (LED sincronizzati)
                #if defined(LED_ON_SYNCRO_TIME) && defined(LED2_PIN)
                // Verifico LED al secondo... su timeSTamp sincronizzato remoto
                if((clCanard.getMicros() / MEGA) % 2) {
                    digitalWrite(LED2_PIN, HIGH);
                } else {
                    digitalWrite(LED2_PIN, LOW);
                }
                #endif

                // ************************************************************************
                // ***************   CHECK OFFLINE/DEADLINE COMMAND/STATE   ***************
                // ************************************************************************

                // **********************************************************
                // Per la coda/istanze allocate valide SLAVE remote_node_flag
                // **********************************************************
                for (byte queueId = 0; queueId<MAX_NODE_CONNECT; queueId++) {
                    if (clCanard.slave[queueId].get_node_id() <= CANARD_NODE_ID_MAX) {
                        // Check eventuale Nodo OFFLINE (Ultimo comando sempre perchè posso)
                        // Effettuare eventuali operazioni di SET,RESET Cmd in sicurezza
                        if (clCanard.slave[queueId].is_entered_offline()) {
                            // Entro in OffLine
                            Serial.print(F("Nodo OFFLINE !!! Alert -> : "));
                            Serial.println(clCanard.slave[queueId].get_node_id());
                            // Metto i Flag in sicurezza, laddove dove non eventualmente gestito
                            // Eventuali altre operazioni quà su Reset Comandi
                            clCanard.slave[queueId].command.reset_pending();
                            clCanard.slave[queueId].register_access.reset_pending();
                            clCanard.slave[queueId].file_server.reset_pending();
                            clCanard.slave[queueId].rmap_service.reset_pending();
                        }
                    }
                }

                // **************************************************************************
                // *********************** FILE UPLOAD PROCESS HANDLER **********************
                // **************************************************************************

                // Verifica file download in corso (entro se in download)
                // Attivato da ricezione comando appropriato rxFile o rxFirmware
                if(clCanard.master.file.download_request()) {
                    if(clCanard.master.file.is_firmware())
                        // Set Flag locale per comunicazione HeartBeat... uploading OK in corso
                        // Utilizzo locale per blocco procedure, sleep ecc. x Uploading
                        clCanard.flag.set_local_fw_uploading(true);
                    // Controllo eventuale timeOut del comando o RxBlocco e gestisco le Retry
                    // Verifica TimeOUT Occurs per File download
                    if(clCanard.master.file.event_timeout()) {
                        Serial.println(F("Time OUT File... event occurs"));
                        // Gestione Retry previste dal comando per poi abbandonare
                        uint8_t retry; // In overload x LOGGING
                        if(clCanard.master.file.next_retry(&retry)) {
                            Serial.print(F("Next Retry File read: "));
                            Serial.println(retry);
                        } else {
                            Serial.println(F("MAX Retry File occurs, download file ABORT !!!"));
                            clCanard.master.file.download_end();
                        }
                    }
                    // Se messaggio in pending non faccio niente è attendo la fine del comando in run
                    // In caso di errore subentrerà il TimeOut e verrà essere gestita la retry
                    if(!clCanard.master.file.is_pending()) {
                        // Fine pending, con messaggio OK. Verifico se EOF o necessario altro blocco
                        if(clCanard.master.file.is_download_complete()) {
                            if(clCanard.master.file.is_firmware()) {
                                Serial.println(F("RX FIRMWARE COMPLETED !!!"));
                            } else {
                                Serial.println(F("RX FILE COMPLETED !!!"));
                            }
                            Serial.println(clCanard.master.file.get_name());
                            Serial.print(F("Size: "));
                            // TODO: FLASH File
                            // Serial.print(getDataFileInfo(clCanard.master.file.get_name(), clCanard.master.file.is_firmware()));
                            Serial.println(F(" (bytes)"));
                            // Nessun altro evento necessario, chiudo File e stati
                            // procedo all'aggiornamento Firmware dopo le verifiche di conformità
                            // Ovviamente se si tratta di un file firmware
                            clCanard.master.file.download_end();
                            // Comunico a HeartBeat (Yakut o Altri) l'avvio dell'aggiornamento (se il file è un firmware...)
                            // Per Yakut Pubblicare un HeartBeat prima dell'Avvio quindi con il flag
                            // clCanard.local_node.file.updating_run = true >> HeartBeat Counica Upgrade...
                            if(clCanard.master.file.is_firmware()) {
                                clCanard.flag.set_local_node_mode(uavcan_node_Mode_1_0_SOFTWARE_UPDATE);
                            }
                            // Il Firmware Upload dovrà partire necessariamente almeno dopo l'invio completo
                            // di HeartBeat (svuotamento coda), quindi attendiamo 2/3 secondi poi via
                            // Counque non rispondo più ai comandi di update con file.updating_run = true
                            // FirmwareUpgrade(*NameFile)... -> Fra 2/3 secondi dopo HeartBeat
                        } else {
                            // Avvio prima request o nuovo blocco (Set Flag e TimeOut)
                            // Prima request (clCanard.local_node.file.offset == 0)
                            // Firmmware Posizione blocco gestito automaticamente in sequenza Request/Read
                            // Gestione retry (incremento su TimeOut/Error) Automatico in Init/Request-Response
                            // Esco se raggiunga un massimo numero di retry x Frame... sopra
                            // Get Data Block per popolare il File
                            // Se il Buffer è pieno = 256 Caratteri è necessario continuare
                            // Altrimenti se inferiore o (0 Compreso) il trasferimento file termina.
                            // Se = 0 il blocco finale non viene considerato ma serve per il protocollo
                            // Se l'ultimo buffer dovesse essere pieno discrimina l'eventualità di MaxBuf==Eof
                            clCanard.master_file_read_block_pending(NODE_GETFILE_TIMEOUT_US);
                        }
                    }
                }
                // **************************************************************************

                // -> Scheduler temporizzato dei messaggi standard da inviare alla rete UAVCAN

                // ************************* HEARTBEAT DATA PUBLISHER ***********************
                if (clCanard.getMicros(clCanard.syncronized_time) >= last_pub_heartbeat) {
                    #ifdef LOG_HEARTBEAT
                    Serial.print(F("Publish MASTER Heartbeat -->> ["));
                    Serial.print(TIME_PUBLISH_HEARTBEAT);
                    Serial.println(F(" sec]"));
                    #endif
                    clCanard.master_heartbeat_send_message();
                    // Update publisher
                    last_pub_heartbeat += MEGA * TIME_PUBLISH_HEARTBEAT;
                }

                // LOOP HANDLER >> 1 SECONDO << TIME SYNCRO (alternato 0.5 sec con Heartbeat)
                if (clCanard.getMicros(clCanard.syncronized_time) >= next_timesyncro_msg)
                {
                    #ifdef LOG_TIMESYNCRO
                    Serial.println(F("Publish MASTER Time Syncronization -->> [1 sec]"));
                    #endif
                    next_timesyncro_msg += MEGA;
                    clCanard.master_timestamp_send_syncronization();
                }

                // ********************** SERVICE PORT LIST PUBLISHER ***********************
                if (clCanard.getMicros(clCanard.syncronized_time) >= last_pub_port_list) {
                    #ifdef LOG_LISTPORT
                    Serial.print(F("Publish Local PORT LIST -->> ["));
                    Serial.print(TIME_PUBLISH_PORT_LIST);
                    Serial.println(F(" sec]"));
                    #endif
                    last_pub_port_list += MEGA * TIME_PUBLISH_PORT_LIST;
                    // Update publisher
                    clCanard.master_servicelist_send_message();
                }

                #ifdef TEST_COMMAND
                // ********************** TEST COMANDO TX-> RX<- *************************
                // LOOP HANDLER >> 0..15 SECONDI x TEST COMANDI <<
                if ((clCanard.getMicros(clCanard.syncronized_time) >= test_cmd_cs_iter_at)) {
                    // TimeOUT variabile in 15 secondi
                    test_cmd_cs_iter_at += MEGA * ((float)(rand() % 60)/4.0);
                    // Invio un comando di test in prova al nodo 125
                    // Possibile accesso da NodeId o direttamente dall'indice in coda conosciuto al master
                    // Abilito disabilito pubblicazione dei dati ogni 3 secondi...
                    uint8_t queueId = clCanard.getSlaveIstanceFromId(125);
                    // Il comando viene inviato solamente se il nodo è ONLine
                    if(clCanard.slave[queueId].is_online()) {
                        // Il comando viene inviato solamente senza altri Pending di Comandi
                        // La verifica andrebbe fatta per singolo servizio, non necessario un blocco di tutto
                        if(!clCanard.slave[queueId].command.is_pending()) {
                            // Eliminare solo per gestione multi comando con file...
                            // Attivo il reset pending standard solo se eseguito il comando quà
                            is_test_command_send = true;
                            // Imposta il pending del comando per verifica sequenza TX-RX e il TimeOut
                            // Imposta la risposta del comadno A UNDEFINED (verrà settato al valore corretto in risposta)
                            // Il comando inverte il servizio di pubblicazione continua dei dati remoti
                            // Semplice TEST di esempio (queueId = IDX di coda, diventerà l'istanza relativa)
                            bEnabService = !bEnabService;
                            if (bEnabService) {
                                Serial.print(F("Inviato comando PUBLISH DISABLE al nodo remoto: "));
                                Serial.println(clCanard.slave[0].get_node_id());
                                clCanard.send_command_pending(queueId, NODE_COMMAND_TIMEOUT_US,
                                    canardClass::Command_Private::disable_publish_rmap, NULL, 0);
                            } else {
                                Serial.print(F("Inviato comando PUBLISH ENABLE al nodo remoto: "));
                                Serial.println(clCanard.slave[0].get_node_id());
                                clCanard.send_command_pending(queueId, NODE_COMMAND_TIMEOUT_US,
                                    canardClass::Command_Private::enable_publish_rmap, NULL, 0);
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
                if (is_test_command_send) {
                    if (clCanard.slave[0].command.event_timeout()) {
                        // Adesso elimino solo gli stati
                        clCanard.slave[0].command.reset_pending();
                        // TimeOUT di un comando in attesa... gestisco il da farsi
                        Serial.print(F("Timeout risposta su invio comando al nodo remoto: "));
                        Serial.print(clCanard.slave[0].get_node_id());
                        Serial.println(F(", Warning [restore pending command]"));
                        is_test_command_send = false;
                    }
                    if (clCanard.slave[0].command.is_executed()) {
                        // Adesso elimino solo gli stati
                        clCanard.slave[0].command.reset_pending();
                        Serial.print(F("Ricevuto conferma di comando dal nodo remoto: "));
                        Serial.print(clCanard.slave[0].get_node_id());
                        Serial.print(F(", cod. risposta ->: "));
                        Serial.println(clCanard.slave[0].command.get_response());
                        is_test_command_send = false;
                    }
                }
                // ***************** FINE TEST COMANDO TX-> RX<- *************************
                #endif

                #ifdef TEST_RMAP_DATA
                // ********************** TEST GETDATA TX-> RX<- *************************
                // LOOP HANDLER >> 0..15 SECONDI x TEST GETDATA <<
                if ((clCanard.getMicros(clCanard.syncronized_time) >= test_cmd_vs_iter_at)) {
                    // TimeOUT variabile in 15 secondi
                    test_cmd_vs_iter_at += MEGA * ((float)(rand() % 60)/4.0);   
                    // Invio un comando di test in prova al nodo 125
                    // Possibile accesso da NodeId o direttamente dall'indice in coda conosciuto al master
                    // Abilito disabilito pubblicazione dei dati ogni 5 secondi...
                    uint8_t queueId = clCanard.getSlaveIstanceFromId(125);
                    // Il comando viene inviato solamente se il nodo è ONLine
                    if(clCanard.slave[queueId].is_online()) {
                        // Il comando viene inviato solamente senza altri Pending di Comandi
                        // La verifica andrebbe fatta per singolo servizio, non necessario un blocco di tutto
                        if(!clCanard.slave[queueId].rmap_service.is_pending()) {
                            Serial.print(F("Inviato richiesta dati al nodo remoto: "));
                            Serial.println(clCanard.slave[0].get_node_id());
                            // parametri.canale = rmap_service_setmode_1_0_CH01 (es-> set CH Analogico...)
                            // parametri.run_for_second = 900; ( inutile in get_istant )
                            rmap_service_setmode_1_0 parmRequest;
                            parmRequest.canale = 0; // Necessario solo per i canali indirizzabili 1..X (Analog/Digital)
                            parmRequest.comando = rmap_service_setmode_1_0_get_istant;
                            parmRequest.run_sectime = 10;
                            // Imposta il pending del comando per verifica sequenza TX-RX e il TimeOut
                            // La risposta al comando è già nel blocco dati, non necessaria ulteriore variabile
                            clCanard.send_rmap_data_pending(queueId, NODE_GETDATA_TIMEOUT_US, parmRequest);
                        }
                    }
                }
                // Test rapido con nodo[0]... SOLO x OUT TEST DI VERIFICA TX/RX SEQUENZA
                // TODO: Eliminare, solo per verifica sequenza... Gestire da Master...
                if (clCanard.slave[0].rmap_service.event_timeout()) {
                    // Adesso elimino solo gli stati per corretta visualizzazione a Serial.println
                    clCanard.slave[0].rmap_service.reset_pending();
                    // TimeOUT di un comando in attesa... gestisco il da farsi
                    Serial.print(F("Timeout risposta su richiesta dati al nodo remoto: "));
                    Serial.print(clCanard.slave[0].get_node_id());
                    Serial.println(F(", Warning [restore pending command]"));
                }
                if(clCanard.slave[0].rmap_service.is_executed()) {
                    // Adesso elimino solo gli stati per corretta visualizzazione a Serial.println
                    clCanard.slave[0].rmap_service.reset_pending();
                    // Interprete del messaggio in casting dal puntatore dinamico
                    // Nell'esempio Il modulo e TH, naturalmente bisogna gestire il tipo
                    // in funzione di clCanard.slave[x].node_type
                    // Esempio ->
                    // switch (clCanard.slave[0].get_module_type()) {
                    //     case canardClass::Module_Type::th:
                    //         rmap_service_module_TH_Response_1_0* retData =
                    //             (rmap_service_module_TH_Response_1_0*) clCanard.slave[0].rmap_service.get_response();
                    //     break;
                    //     ...
                    // }
                    rmap_service_module_TH_Response_1_0* retData =
                        (rmap_service_module_TH_Response_1_0*) clCanard.slave[0].rmap_service.get_response();
                    // Stampo la risposta corretta
                    Serial.print(F("Ricevuto risposta di richiesta dati dal nodo remoto: "));
                    Serial.print(clCanard.slave[0].get_node_id());
                    Serial.print(F(", cod. risposta ->: "));
                    Serial.println(retData->stato);
                    // Test data Received e stampa valori con accesso al puntatore in casting per il modulo
                    Serial.println(F("Value (ITH) L1, TP, UH: "));
                    Serial.print(retData->ITH.metadata.level.L1.value);
                    Serial.print(F(", "));
                    Serial.print(retData->ITH.temperature.val.value);
                    Serial.print(F(", "));
                    Serial.println(retData->ITH.humidity.val.value);
                }
                // ***************** FINE TEST GETDATA TX-> RX<- *************************
                #endif

                #ifdef TEST_REGISTER
                // ********************** TEST REGISTER TX-> RX<- *************************
                // LOOP HANDLER >> 0..15 SECONDI x TEST REGISTER ACCESS <<
                if ((clCanard.getMicros(clCanard.syncronized_time) >= test_cmd_rg_iter_at)) {
                    // TimeOUT variabile in 15 secondi
                    test_cmd_rg_iter_at += MEGA * ((float)(rand() % 60)/4.0);
                    // Invio un comando di set registro in prova al nodo 125
                    // Possibile accesso da NodeId o direttamente dall'indice in coda conosciuto al master
                    uint8_t queueId = clCanard.getSlaveIstanceFromId(125);
                    // Il comando viene inviato solamente se il nodo è ONLine
                    if(clCanard.slave[queueId].is_online())
                    {
                        // Il comando viene inviato solamente senza altri Pending di Comandi
                        // La verifica andrebbe fatta per singolo servizio, non necessario un blocco di tutto
                        if(!clCanard.slave[queueId].register_access.is_pending()) {
                            // Preparo il registro da inviare (configurazione generale => sequenza di request/response)
                            // Semplice TEST di esempio trasmissione di un registro fisso con nome fisso
                            // Uso in Test uavcan_register_Value_1_0 val utilizzato in ReadRegister iniziale
                            // Init Var per confronto memCmp di verifica elementi == x TEST Veloce
                            memset(&val, 0, sizeof(uavcan_register_Value_1_0));
                            // x SPECIFICHE UAVCAN ->
                            // NB Il tipo di registro deve essere == (es. Natural32) e deve esistere sul nodo Remoto
                            // Altrimenti la funzione deve fallire e ritornare NULL
                            // Quindi il Master deve conoscere la tipologia di registro e nome dello SLAVE
                            // Non è possibile creare un registro senza uscire dalle specifiche (es. comando vendor_specific)
                            // Preparo il registro (Inifluente se l'operazione è di sola lettura parametro -> write)
                            uavcan_register_Value_1_0_select_natural16_(&val);
                            val.natural32.value.count       = 1;
                            val.natural32.value.elements[0] = 12345;
                            // Avvia comando e setta il pending relativo di istanza
                            clCanard.send_register_access_pending(queueId, NODE_REGISTER_TIMEOUT_US,
                                        "rmap.module.TH.metadata.Level.L2", val, bIsWriteRegister);
                            if(bIsWriteRegister) {
                                Serial.print(F("Inviato registro WRITE al nodo remoto: "));
                            } else {
                                Serial.print(F("Richiesto registro READ al nodo remoto: "));
                            }
                            Serial.println(clCanard.slave[0].get_node_id());
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
                if (clCanard.slave[0].register_access.event_timeout()) {
                    // Reset del pending comando
                    clCanard.slave[0].register_access.reset_pending();
                    // TimeOUT di un comando in attesa... gestisco il da farsi
                    Serial.print(F("Timeout risposta su invio registro al nodo remoto: "));
                    Serial.print(clCanard.slave[0].get_node_id());
                    Serial.println(F(", Warning [restore pending register]"));
                }
                if (clCanard.slave[0].register_access.is_executed()) {
                    // Reset del pending comando
                    clCanard.slave[0].register_access.reset_pending();
                    Serial.print(F("Ricevuto messaggio register R/W registro dal nodo remoto: "));
                    Serial.println(clCanard.slave[0].get_node_id());
                    Serial.print(F("Totale elementi (Natural16): "));
                    uavcan_register_Value_1_0 registerResp = clCanard.slave[0].register_access.get_response();
                    Serial.println(registerResp.unstructured.value.count);
                    for(byte bElement=0; bElement<registerResp.unstructured.value.count; bElement++) {
                        Serial.print(registerResp.natural16.value.elements[bElement]);
                        Serial.print(" ");                        
                    }
                    Serial.println();
                    // Con TX == RX Allora è una scrittura e se coincide il registro è impostato
                    // Se non coincide il comando è fallito (anche se RX = OK) TEST COMPLETO!!!
                    // Tecnicamente se != da empty in request ed in request il registro è impostato
                    // I valori != 0 in elementi extra register.value.count non sono considerati 
                    if(bIsWriteRegister) {
                        // TEST BYTE A BYTE...
                        if(memcmp(&registerResp, &val, sizeof(uavcan_register_Value_1_0)) == 0) {
                            Serial.println("Registro impostato correttamente");
                        } else {
                            Serial.println("Registro NON impostato correttamente. ALERT!!!");
                        }
                    }
                    // Inverto da Lettura a scrittura e viceversa per il TEST
                    bIsWriteRegister = !bIsWriteRegister;
                }
                // ***************** FINE TEST COMANDO TX-> RX<- *************************
                #endif

                // ********************** TEST UPLOAD FIRMWARE/FILE **********************
                #ifdef TEST_FIRMWARE
                // Se nodo Online, procedo, se passo OffLine interrompo la procedura
                // Oppure posso gestire in un timeOut Limitato (L'OffLine è comunque già temporizzato...)
                // Invio comando e attendo risposta affermativa, poi passo all'aggiornamento
                // NB!!! Rispondo a Request solo se sono nello stato coerente di gestione file upload
                // Questo comporta la sicurezza che il nodo non abbia avuto problematiche di genere
                // Sia come server che client. Se non sono in coerenza e non rispondo, lo slave andrà
                // in TimeOut e la procedura non finisce (senza causare quindi irregolari trasferimenti)
                // Al termine del trasferimento, esco dalla modalità >> FILE_STATE_WAIT_COMMAND <<
                // direttamente nell' invio dell' ultimo pacchetto dati. E mi preparo a nuovo invio...
                if(bEventRealTimeLoop) {
                    // TEST Locale di esempio
                    uint8_t queueId = clCanard.getSlaveIstanceFromId(125);

                    // Se vado OffLine la procedura comunque viene interrotta dall'evento di OffLine
                    switch(clCanard.slave[queueId].file_server.get_state()) {

                        default: // -->> FILE_STATE_ERROR STATE:
                            clCanard.slave[queueId].file_server.end_transmission();
                            break;

                        case canardClass::FileServer_State::standby:
                            // TEST Press Button Nucleo
                            if(digitalRead(USER_BTN) == HIGH) {
                                // Test di esempio con nome fisso e tipologia Firmware
                                clCanard.slave[queueId].file_server.set_file_name(
                                    "stima4.module_th-1.1.app.hex", true);
                            }
                            break;

                        case canardClass::FileServer_State::begin_update:
                            // Avvio comando di aggiornamento con controllo coerenza File (Register)
                            // Controllo Check Firmware... CRC, presenza ed altro
                            if(checkFile(clCanard.slave[queueId].file_server.get_file_name(),
                                        clCanard.slave[queueId].file_server.is_firmware())) {
                                // Avvio il comando nel nodo remoto
                                clCanard.slave[queueId].file_server.next_state();
                            } else {
                                // Annullo la richiesta (Invio Error al Remoto su Request File)
                                // O smetto di rispondere al servizio per quella richiesta (TimeOut)
                                clCanard.slave[queueId].file_server.end_transmission();
                            }
                            break;

                        case canardClass::FileServer_State::command_send:
                            // Invio comando di aggiornamento File (in attesa in coda con switch...)
                            // Il comando viene inviato solamente senza altri Pending di Comandi (come semaforo)
                            if(!clCanard.slave[queueId].command.is_pending()) {
                                // Imposta il pending del comando per verifica sequenza TX-RX e il TimeOut
                                // Imposta la risposta del comadno A UNDEFINED (verrà settato al valore corretto in risposta)
                                // Il comando comunica la presenza del file di download nel remoto e avvia dowload
                                clCanard.send_command_file_server_pending(queueId, NODE_COMMAND_TIMEOUT_US);
                                clCanard.slave[queueId].file_server.next_state();
                            }
                            break;

                        case canardClass::FileServer_State::command_wait:
                            // Attendo la risposta del Nodo Remoto conferma, errore o TimeOut
                            if(clCanard.slave[queueId].command.event_timeout()) {
                                // Counico al server l'errore di timeOut Command Update Start ed esco
                                clCanard.slave[queueId].file_server.end_transmission();
                                Serial.print(F("Node ["));
                                Serial.print(clCanard.slave[queueId].get_node_id());                            
                                Serial.println(F("] TimeOut Command Start Send file "));
                                if(clCanard.slave[queueId].file_server.is_firmware()) {
                                    Serial.println(F("FIRMWARE send update ABORT!!!"));
                                } else {
                                    Serial.println(F("FILE send update ABORT!!!"));
                                }
                                // Se decido di uscire nella procedura di OffLine, la comunicazione
                                // al server di eventuali errori deve essere gestita al momento dell'OffLine
                            }
                            // Comando eseguito,verifico la correttezza della risposta
                            if(clCanard.slave[queueId].command.is_executed()) {
                                // La risposta si trova in command_response fon flag pending azzerrato.
                                if(clCanard.slave[queueId].command.get_response() == 
                                    uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS) {
                                    // Sequenza terminata, avvio il file transfer !!!
                                    // Essendo una procedura server e non avendo nessun controllo sui comandi
                                    // La classe si limita a fornire gli elementi di controllo timeOut per
                                    // la verifica di coerenza e gestione del download dal remoto ed internamente
                                    // gestisco le fasi dell'upload. Non è richiesta nessun altra gestione e per
                                    // comodità gestisco l'aggiornamento del timeOut direttamente nella callBack
                                    // nella sezione request del file dallo slave.
                                    clCanard.slave[queueId].file_server.next_state();
                                    // Stampo lo stato
                                    Serial.print(F("Node ["));
                                    Serial.print(clCanard.slave[queueId].get_node_id());
                                    Serial.println(F("] Upload Start Send file OK"));
                                    // Imposto il timeOUT per controllo Deadline con pending per sequenza di download
                                    clCanard.slave[queueId].file_server.start_pending(NODE_REQFILE_TIMEOUT_US);
                                } else {
                                    // Errore comando eseguito ma risposta non valida. Annullo il trasferimento
                                    clCanard.slave[queueId].file_server.end_transmission();
                                    // ...counico al server (RMAP remoto) l'errore per il mancato aggiornamento ed esco
                                    Serial.print(F("Node ["));
                                    Serial.print(clCanard.slave[queueId].get_node_id());
                                    Serial.println(F("] Response Cmd Error in Send file"));
                                    if(clCanard.slave[queueId].file_server.is_firmware()) {
                                        Serial.println(F("FIRMWARE send update ABORT!!!"));
                                    } else {
                                        Serial.println(F("FILE send update ABORT!!!"));
                                    }
                                }
                            }
                            break;

                        case canardClass::FileServer_State::state_uploading:
                            // Attendo la risposta del Nodo Remoto conferma, errore o TimeOut
                            if(clCanard.slave[queueId].file_server.event_timeout()) {
                                // Counico al server l'errore di timeOut Command Update Start ed esco
                                clCanard.slave[queueId].file_server.end_transmission();
                                Serial.print(F("Node ["));
                                Serial.print(clCanard.slave[queueId].get_node_id());
                                Serial.println(F("] TimeOut Request/Response Send file "));
                                if(clCanard.slave[queueId].file_server.is_firmware()) {
                                    Serial.println(F("FIRMWARE send update ABORT!!!"));
                                } else {
                                    Serial.println(F("FILE send update ABORT!!!"));
                                }
                                // Se decido di uscire nella procedura di OffLine, la comunicazione
                                // al server di eventuali errori deve essere gestita al momento dell'OffLine
                            }
                            // Evento completato (file_server) terminato con successo, vado a step finale
                            if(clCanard.slave[queueId].file_server.is_executed()) {
                                clCanard.slave[queueId].file_server.next_state();
                            }
                            break;

                        case canardClass::FileServer_State::upload_complete:
                            // Counico al server file upload Complete ed esco (nuova procedura ready)
                            // -> EXIT FROM FILE_STATE_STANDBY ( In procedura di SendFileBlock )
                            // Quando invio l'ultimo pacchetto dati valido ( Blocco < 256 Bytes )
                            clCanard.slave[queueId].file_server.end_transmission();
                            Serial.print(F("Node ["));
                            Serial.print(clCanard.slave[queueId].get_node_id());
                            if(clCanard.slave[queueId].file_server.is_firmware()) {
                                Serial.println(F("] FIRMWARE send Complete!!!"));
                            } else {
                                Serial.println(F("] FILE send Complete!!!"));
                            }
                            break;
                    }
                }
                // ******************** FINE TEST UPLOAD FIRWARE/FILE ********************
                #endif

                // Utilizzato per blinking Led (TX/RX)
                #if defined(LED_ON_CAN_DATA_TX) or defined(LED_ON_CAN_DATA_RX)
                digitalWrite(LED2_PIN, LOW);
                #endif

                // ***************************************************************************
                //   Gestione Coda messaggi in trasmissione (ciclo di svuotamento messaggi)
                // ***************************************************************************
                // Transmit pending frames, avvia la trasmissione gestita da canard a priorità.
                // Il metodo può essere chiamato direttamente in preparazione messaggio x coda
                if (clCanard.transmitQueueDataPresent()) {
                    #ifdef LED_ON_CAN_DATA_TX
                    digitalWrite(LED2_PIN, HIGH);
                    #endif
                    clCanard.transmitQueue();
                    test_millis = millis();
                    bLowPower = false;
                }

                // ***************************************************************************
                //   Gestione Coda messaggi in ricezione (ciclo di caricamento messaggi)
                // ***************************************************************************
                // Gestione con Intererupt RX Only esterna (verifica dati in coda gestionale)
                if (clCanard.receiveQueueDataPresent()) {
                    #ifdef LED_ON_CAN_DATA_RX
                    digitalWrite(LED2_PIN, HIGH);
                    #endif
                    // Log Packet
                    #ifdef LOG_RX_PACKET
                    char logMsg[50];
                    clCanard.receiveQueue(logMsg);
                    Serial.println(logMsg);
                    #else
                    clCanard.receiveQueue();
                    #endif
                }

                // Request Reboot
                if (clCanard.flag.is_requested_system_restart()) {
                    // TODO: Save param...
                    NVIC_SystemReset();
                }
                break;

            // TODO: Implementare
            case SLEEP:
                // ...
                // LowPower Mode before general PowerDown or Waiting TimeEvent WakeUP
                HW_CAN_Power(CAN_ModePower::CAN_SLEEP);
                break;
        }

        #ifdef LOG_STACK_USAGE
        if(stackTimer++>100) {
          stackTimer = 0;
          TRACE_VERBOSE_F(F("Stack Usage: %d\r\n"), uxTaskGetStackHighWaterMark( NULL ));
        }
        #endif

        // Esecuzione ongi 10 ms
        DelayUntil(Ticks::MsToTicks(10));

    }
}
