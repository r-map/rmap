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

/// Legge il subjectID per il modulo corrente per la risposta al servizio di gestione dati.
CanardPortID CanTask::getModeAccessID(uint8_t modeAccessID, const char* const port_name, const char* const type_name) {
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
    clRegister.read(&register_name[0], &val);
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
    clRegister.write(&register_name[0], &val);  // Unconditionally overwrite existing value because it's read-only.

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
rmap_sensors_TH_1_0 CanTask::prepareSensorsDataValueExample(uint8_t const sensore, const report_t *report) {
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
void CanTask::publish_rmap_data(canardClass &clCanard, CanParam_t *param) {
    // Pubblica i dati del nodo corrente se abilitata la funzione e con il corretto subjectId
    // Ovviamente il nodo non può essere anonimo per la pubblicazione...
    if ((!clCanard.is_canard_node_anonymous()) &&
        (clCanard.publisher_enabled.module_th) &&
        (clCanard.port_id.publisher_module_th <= CANARD_SUBJECT_ID_MAX)) {
        rmap_module_TH_1_0 module_th_msg = {0};

        request_data_t request_data;
        report_t report;

        // preparo la struttura dati per richiedere i dati al task che li elabora
        request_data.is_init = false;   // utilizza i dati esistenti (continua le elaborazioni precedentemente inizializzate)
        request_data.report_time_s = 900;   // richiedo i dati su 900 secondi
        request_data.observation_time_s = 60;   // richiedo i dati mediati su 60 secondi

        // coda di richiesta dati (senza attesa)
        param->requestDataQueue->Enqueue(&request_data, 0);

        // coda di attesa dati (attesa infinita fino alla ricezione degli stessi)
        // if (param->reportDataQueue->Dequeue(&report, portMAX_DELAY)) {
        //   TRACE_INFO(F("--> CAN temperature report\t%d\t%d\t%d\t%d\t%d\t%d\r\n"), (int32_t) report.temperature.sample, (int32_t) report.temperature.ist, (int32_t) report.temperature.min, (int32_t) report.temperature.avg, (int32_t) report.temperature.max, (int32_t) report.temperature.quality);
        //   TRACE_INFO(F("--> CAN humidity report\t%d\t%d\t%d\t%d\t%d\t%d\r\n"), (int32_t) report.humidity.sample, (int32_t) report.humidity.ist, (int32_t) report.humidity.min, (int32_t) report.humidity.avg, (int32_t) report.humidity.max, (int32_t) report.humidity.quality);
        // }

        // Preparo i dati e metadati fissi
        // TODO: Aggiorna i valori mobili
        module_th_msg.ITH = prepareSensorsDataValueExample(canardClass::Sensor_Type::ith, &report);
        module_th_msg.ITH.metadata = clCanard.module_th.ITH.metadata;
        module_th_msg.MTH = prepareSensorsDataValueExample(canardClass::Sensor_Type::mth, &report);
        module_th_msg.MTH.metadata = clCanard.module_th.MTH.metadata;
        module_th_msg.NTH = prepareSensorsDataValueExample(canardClass::Sensor_Type::nth, &report);
        module_th_msg.NTH.metadata = clCanard.module_th.NTH.metadata;
        module_th_msg.XTH = prepareSensorsDataValueExample(canardClass::Sensor_Type::xth, &report);
        module_th_msg.XTH.metadata = clCanard.module_th.XTH.metadata;
        // Serialize and publish the message:
        uint8_t serialized[rmap_module_TH_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
        size_t serialized_size = sizeof(serialized);
        const int8_t err = rmap_module_TH_1_0_serialize_(&module_th_msg, &serialized[0], &serialized_size);
        LOCAL_ASSERT(err >= 0);
        if (err >= 0) {
            const CanardTransferMetadata meta = {
                .priority = CanardPrioritySlow,
                .transfer_kind = CanardTransferKindMessage,
                .port_id = clCanard.port_id.publisher_module_th,
                .remote_node_id = CANARD_NODE_ID_UNSET,
                .transfer_id = (CanardTransferID)(clCanard.next_transfer_id.module_th()),  // Increment!
            };
            // Messaggio rapido 1/4 di secondo dal timeStamp Sincronizzato
            clCanard.send(MEGA / 4, &meta, serialized_size, &serialized[0]);
        }
    }
}

// ***************************************************************************************************
//   Funzioni ed utility di ricezione dati dalla rete UAVCAN, richiamati da processReceivedTransfer()
// ***************************************************************************************************

// Plug and Play Slave, Versione 1.0 compatibile con CAN_CLASSIC MTU 8
// Messaggi anonimi CAN non sono consentiti se messaggi > LUNGHEZZA MTU disponibile
void CanTask::processMessagePlugAndPlayNodeIDAllocation(canardClass &clCanard,
                                                      const uavcan_pnp_NodeIDAllocationData_1_0* const msg) {
    // msg->unique_id_hash RX non gestito, è valido GetUniqueID Unificato per entrambe versioni V1 e V2
    if (msg->allocated_node_id.elements[0].value <= CANARD_NODE_ID_MAX) {
        printf("Got PnP node-ID allocation: %d\n", msg->allocated_node_id.elements[0].value);
        clCanard.set_canard_node_id((CanardNodeID)msg->allocated_node_id.elements[0].value);
        // Store the value into the non-volatile storage.
        uavcan_register_Value_1_0 reg = {0};
        uavcan_register_Value_1_0_select_natural16_(&reg);
        reg.natural16.value.elements[0] = msg->allocated_node_id.elements[0].value;
        reg.natural16.value.count = 1;
        clRegister.write("uavcan.node.id", &reg);
        // We no longer need the subscriber, drop it to free up the resources (both memory and CPU time).
        clCanard.rxUnSubscribe(CanardTransferKindMessage,
                                uavcan_pnp_NodeIDAllocationData_1_0_FIXED_PORT_ID_);
    }
    // Otherwise, ignore it: either it is a request from another node or it is a response to another node.
}

// Chiamate gestioni RPC remote da master (yakut o altro servizio di controllo)
uavcan_node_ExecuteCommand_Response_1_1 CanTask::processRequestExecuteCommand(canardClass &clCanard, const uavcan_node_ExecuteCommand_Request_1_1* req,
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
        case canardClass::Command_Private::enable_publish_rmap:
        {
            // Abilita pubblicazione fast_loop data_and_metadata modulo locale (test yakut e user master)
            clCanard.publisher_enabled.module_th = true;
            Serial.println(F("ATTIVO Trasmissione dati in publish"));
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        case canardClass::Command_Private::disable_publish_rmap:
        {
            // Disabilita pubblicazione fast_loop data_and_metadata modulo locale (test yakut e user master)
            clCanard.publisher_enabled.module_th = false;
            Serial.println(F("DISATTIVO Trasmissione dati in publish"));
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
rmap_service_module_TH_Response_1_0 CanTask::processRequestGetModuleData(canardClass &clCanard, rmap_service_module_TH_Request_1_0* req, CanParam_t *param) {
    rmap_service_module_TH_Response_1_0 resp = {0};
    // Richiesta parametri univoca a tutti i moduli
    // req->parametri tipo: rmap_service_setmode_1_0
    // req->parametri.comando (Comando esterno ricevuto 3 BIT)
    // req->parametri.run_sectime (Timer to run 13 bit)

    request_data_t request_data;
    report_t report;

    // Case comandi RMAP su GetModule Data (Da definire con esattezza quali e quanti altri)
    switch (req->parametri.comando) {

        /// saturated uint3 get_istant = 0
        /// Ritorna il dato istantaneo (o ultima acquisizione dal sensore)
        case rmap_service_setmode_1_0_get_istant:
          // preparo la struttura dati per richiedere i dati al task che li elabora
          request_data.is_init = false;   // utilizza i dati esistenti (continua le elaborazioni precedentemente inizializzate)
          request_data.report_time_s = 900;   // richiedo i dati su 900 secondi
          request_data.observation_time_s = 60;   // richiedo i dati mediati su 60 secondi

          // coda di richiesta dati (senza attesa)
          param->requestDataQueue->Enqueue(&request_data, 0);

          // coda di attesa dati (attesa infinita fino alla ricezione degli stessi)
        //   if (param->reportDataQueue->Dequeue(&report, portMAX_DELAY)) {
        //     TRACE_INFO(F("--> CAN temperature report\t%d\t%d\t%d\t%d\t%d\t%d\r\n"), (int32_t) report.temperature.sample, (int32_t) report.temperature.ist, (int32_t) report.temperature.min, (int32_t) report.temperature.avg, (int32_t) report.temperature.max, (int32_t) report.temperature.quality);
        //     TRACE_INFO(F("--> CAN humidity report\t%d\t%d\t%d\t%d\t%d\t%d\r\n"), (int32_t) report.humidity.sample, (int32_t) report.humidity.ist, (int32_t) report.humidity.min, (int32_t) report.humidity.avg, (int32_t) report.humidity.max, (int32_t) report.humidity.quality);
        //   }

          // Ritorno lo stato (Copia dal comando...)
          resp.stato = req->parametri.comando;
          // Preparo la risposta di esempio
          // TODO: Aggiorna i valori mobili
          resp.ITH = prepareSensorsDataValueExample(canardClass::Sensor_Type::ith, &report);
          resp.MTH = prepareSensorsDataValueExample(canardClass::Sensor_Type::mth, &report);
          resp.NTH = prepareSensorsDataValueExample(canardClass::Sensor_Type::nth, &report);
          resp.XTH = prepareSensorsDataValueExample(canardClass::Sensor_Type::xth, &report);
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
    resp.ITH.metadata = clCanard.module_th.ITH.metadata;
    resp.MTH.metadata = clCanard.module_th.MTH.metadata;;
    resp.NTH.metadata = clCanard.module_th.NTH.metadata;
    resp.XTH.metadata = clCanard.module_th.XTH.metadata;

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
void CanTask::processReceivedTransfer(canardClass &clCanard, const CanardRxTransfer* const transfer, void *param) {
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
                    // if (!clCanard.master.heartbeat.is_online()) {
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
                    clCanard.master.heartbeat.set_online(MASTER_OFFLINE_TIMEOUT_US);
                    clCanard.flag.set_local_power_mode(remoteVSC.powerMode);
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
                    if(clCanard.master.timestamp.check_valid_syncronization(
                            transfer->metadata.transfer_id,
                            msg.previous_transmission_timestamp_microsecond)) {
                        // Leggo il time stamp sincronizzato da gestire per Setup RTC
                        timestamp_synchronized_us = clCanard.master.timestamp.get_timestamp_syncronized(
                            transfer->timestamp_usec,
                            msg.previous_transmission_timestamp_microsecond);
                        isSyncronized = true;
                    } else {
                        Serial.print(F("RX TimeSyncro from master, reset or invalid Value at local_time_stamp (uSec): "));
                        Serial.println(transfer->timestamp_usec);
                    }
                    // Aggiorna i temporizzatori locali per il prossimo controllo
                    CanardMicrosecond last_message_diff_us = clCanard.master.timestamp.update_timestamp_message(
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
                processMessagePlugAndPlayNodeIDAllocation(clCanard, &msg);
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
        if (transfer->metadata.port_id == clCanard.port_id.service_module_th) {
            // Richiesta ai dati e metodi di sensor drive
            rmap_service_module_TH_Request_1_0 req = {0};
            size_t size = transfer->payload_size;
            Serial.println(F("<<-- Ricevuto richiesta dati da master"));
            // The request object is empty so we don't bother deserializing it. Just send the response.
            if (rmap_service_module_TH_Request_1_0_deserialize_(&req, static_cast<uint8_t const*>(transfer->payload), &size) >= 0) {
                // I dati e metadati sono direttamente popolati in processRequestGetModuleData
                rmap_service_module_TH_Response_1_0 module_th_resp = processRequestGetModuleData(clCanard, &req, (CanParam_t *) param);
                // Serialize and publish the message:
                uint8_t serialized[rmap_service_module_TH_Response_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
                size_t serialized_size = sizeof(serialized);
                const int8_t res = rmap_service_module_TH_Response_1_0_serialize_(&module_th_resp, &serialized[0], &serialized_size);
                if (res >= 0) {
                    // Risposta standard ad un secondo dal timeStamp Sincronizzato
                    clCanard.sendResponse(MEGA, &transfer->metadata, serialized_size, &serialized[0]);
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
                clCanard.sendResponse(MEGA, &transfer->metadata, serialized_size, &serialized[0]);
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
                    clCanard.sendResponse(MEGA, &transfer->metadata, serialized_size, &serialized[0]);
                }
            }
        }
        else if (transfer->metadata.port_id == uavcan_register_List_1_0_FIXED_PORT_ID_)
        {
            uavcan_register_List_Request_1_0 req = {0};
            size_t size = transfer->payload_size;
            if (uavcan_register_List_Request_1_0_deserialize_(&req, static_cast<uint8_t const*>(transfer->payload), &size) >= 0) {
                const uavcan_register_List_Response_1_0 resp = {.name = clRegister.getNameByIndex(req.index)};
                uint8_t serialized[uavcan_register_List_Response_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
                size_t serialized_size = sizeof(serialized);
                if (uavcan_register_List_Response_1_0_serialize_(&resp, &serialized[0], &serialized_size) >= 0) {
                    // Risposta standard ad un secondo dal timeStamp Sincronizzato
                    clCanard.sendResponse(MEGA, &transfer->metadata, serialized_size, &serialized[0]);
                }
            }
        }
        else if (transfer->metadata.port_id == uavcan_node_ExecuteCommand_1_1_FIXED_PORT_ID_)
        {
            uavcan_node_ExecuteCommand_Request_1_1 req = {0};
            size_t size = transfer->payload_size;
            Serial.println(F("<<-- Ricevuto comando esterno"));
            if (uavcan_node_ExecuteCommand_Request_1_1_deserialize_(&req, static_cast<uint8_t const*>(transfer->payload), &size) >= 0) {
                const uavcan_node_ExecuteCommand_Response_1_1 resp = processRequestExecuteCommand(clCanard, &req, transfer->metadata.remote_node_id);
                uint8_t serialized[uavcan_node_ExecuteCommand_Response_1_1_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
                size_t serialized_size = sizeof(serialized);
                if (uavcan_node_ExecuteCommand_Response_1_1_serialize_(&resp, &serialized[0], &serialized_size) >= 0) {
                    // Risposta standard ad un secondo dal timeStamp Sincronizzato
                    clCanard.sendResponse(MEGA, &transfer->metadata, serialized_size, &serialized[0]);
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
                    // Save Data in File at Block Position (Init = Rewrite file...)
                    // TODO: Flash
                    // putDataFile(clCanard.master.file.get_name(), clCanard.master.file.is_firmware(), clCanard.master.file.is_first_data_block(),
                    //              resp.data.value.elements, resp.data.value.count);
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
    }
    else
    {
        // Se arrivo quà è un errore di sviluppo, controllare setup sottoscrizioni e Rqst (non gestito slave)
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
    CanardMicrosecond last_pub_rmap_data;
    CanardMicrosecond last_pub_heartbeat;
    CanardMicrosecond last_pub_port_list;

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
    #define LOG_RX_PACKET
    #define LOG_STACK_USAGE
    // #define LED_ON_CAN_DATA_TX
    // #define LED_ON_CAN_DATA_RX
    #define LED_ON_SYNCRO_TIME

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
                //clCanard.setReceiveMessage_CB(&this->processReceivedTransfer(), (void *) &param);
                clCanard.setReceiveMessage_CB(processReceivedTransfer, (void *) &param);

                // ********************    Lettura Registri standard UAVCAN    ********************
                // Restore the node-ID from the corresponding standard clRegister. Default to anonymous.
                #ifdef NODE_SLAVE_ID
                // Canard Slave NODE ID Fixed dal defined value in module_config
                clCanard.set_canard_node_id((CanardNodeID)NODE_SLAVE_ID);
                #else
                uavcan_register_Value_1_0_select_natural16_(&val);
                val.natural16.value.count = 1;
                val.natural16.value.elements[0] = UINT16_MAX; // This means undefined (anonymous), per Specification/libcanard.
                registerRead("uavcan.node.id", &val);         // The names of the standard registers are regulated by the Specification.
                LOCAL_ASSERT(uavcan_register_Value_1_0_is_natural16_(&val) && (val.natural16.value.count == 1));
                if (val.natural16.value.elements[0] <= CANARD_NODE_ID_MAX) {
                    clCanard.set_canard_node_id((CanardNodeID)val.natural16.value.elements[0]);
                }
                #endif

                // The description register is optional but recommended because it helps constructing/maintaining large networks.
                // It simply keeps a human-readable description of the node that should be empty by default.
                uavcan_register_Value_1_0_select_string_(&val);
                val._string.value.count = 0;
                clRegister.read("uavcan.node.description", &val);  // We don't need the value, we just need to ensure it exists.

                // Carico i/il port-ID/subject-ID del modulo locale dai registri relativi associati nel namespace UAVCAN
                clCanard.port_id.publisher_module_th =
                    getModeAccessID(canardClass::Introspection_Port::PublisherSubjectID,
                        "TH.data_and_metadata", rmap_module_TH_1_0_FULL_NAME_AND_VERSION_);

                clCanard.port_id.service_module_th =
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
                clRegister.read("rmap.module.TH.metadata.Level.L1", &val);
                LOCAL_ASSERT(uavcan_register_Value_1_0_is_natural16_(&val) && (val.natural16.value.count == 1));
                clCanard.module_th.ITH.metadata.level.L1.value = val.natural16.value.elements[0];
                clCanard.module_th.MTH.metadata.level.L1.value = val.natural16.value.elements[0];
                clCanard.module_th.NTH.metadata.level.L1.value = val.natural16.value.elements[0];
                clCanard.module_th.XTH.metadata.level.L1.value = val.natural16.value.elements[0];
                // *********************************** L2 *********************************************
                uavcan_register_Value_1_0_select_natural16_(&val);
                val.natural16.value.count = 1;
                val.natural16.value.elements[0] = UINT16_MAX;
                clRegister.read("rmap.module.TH.metadata.Level.L2", &val);
                LOCAL_ASSERT(uavcan_register_Value_1_0_is_natural16_(&val) && (val.natural16.value.count == 1));
                clCanard.module_th.ITH.metadata.level.L2.value = val.natural16.value.elements[0];
                clCanard.module_th.MTH.metadata.level.L2.value = val.natural16.value.elements[0];
                clCanard.module_th.NTH.metadata.level.L2.value = val.natural16.value.elements[0];
                clCanard.module_th.XTH.metadata.level.L2.value = val.natural16.value.elements[0];
                // ******************************* LevelType1 *****************************************
                uavcan_register_Value_1_0_select_natural16_(&val);
                val.natural16.value.count = 1;
                val.natural16.value.elements[0] = UINT16_MAX;
                clRegister.read("rmap.module.TH.metadata.Level.LevelType1", &val);
                LOCAL_ASSERT(uavcan_register_Value_1_0_is_natural16_(&val) && (val.natural16.value.count == 1));
                clCanard.module_th.ITH.metadata.level.LevelType1.value = val.natural16.value.elements[0];
                clCanard.module_th.MTH.metadata.level.LevelType1.value = val.natural16.value.elements[0];
                clCanard.module_th.NTH.metadata.level.LevelType1.value = val.natural16.value.elements[0];
                clCanard.module_th.XTH.metadata.level.LevelType1.value = val.natural16.value.elements[0];
                // ******************************* LevelType2 *****************************************
                uavcan_register_Value_1_0_select_natural16_(&val);
                val.natural16.value.count = 1;
                val.natural16.value.elements[0] = UINT16_MAX;
                clRegister.read("rmap.module.TH.metadata.Level.LevelType2", &val);
                LOCAL_ASSERT(uavcan_register_Value_1_0_is_natural16_(&val) && (val.natural16.value.count == 1));
                clCanard.module_th.ITH.metadata.level.LevelType2.value = val.natural16.value.elements[0];
                clCanard.module_th.MTH.metadata.level.LevelType2.value = val.natural16.value.elements[0];
                clCanard.module_th.NTH.metadata.level.LevelType2.value = val.natural16.value.elements[0];
                clCanard.module_th.XTH.metadata.level.LevelType2.value = val.natural16.value.elements[0];
                // *********************************** P1 *********************************************
                uavcan_register_Value_1_0_select_natural16_(&val);
                val.natural16.value.count = 1;
                val.natural16.value.elements[0] = UINT16_MAX;
                clRegister.read("rmap.module.TH.metadata.Timerange.P1", &val);
                LOCAL_ASSERT(uavcan_register_Value_1_0_is_natural16_(&val) && (val.natural16.value.count == 1));
                clCanard.module_th.ITH.metadata.timerange.P1.value = val.natural16.value.elements[0];
                clCanard.module_th.MTH.metadata.timerange.P1.value = val.natural16.value.elements[0];
                clCanard.module_th.NTH.metadata.timerange.P1.value = val.natural16.value.elements[0];
                clCanard.module_th.XTH.metadata.timerange.P1.value = val.natural16.value.elements[0];
                // *********************************** P2 *********************************************
                // P2 Non memorizzato sul modulo, parametro dipendente dall'acquisizione locale
                clCanard.module_th.ITH.metadata.timerange.P2 = 900;
                clCanard.module_th.MTH.metadata.timerange.P2 = 900;
                clCanard.module_th.NTH.metadata.timerange.P2 = 900;
                clCanard.module_th.XTH.metadata.timerange.P2 = 900;
                // *********************************** P2 *********************************************
                uavcan_register_Value_1_0_select_natural8_(&val);
                val.natural16.value.count = 1;
                val.natural16.value.elements[0] = UINT8_MAX;
                clRegister.read("rmap.module.TH.metadata.Timerange.Pindicator", &val);
                LOCAL_ASSERT(uavcan_register_Value_1_0_is_natural8_(&val) && (val.natural16.value.count == 1));
                clCanard.module_th.ITH.metadata.timerange.Pindicator.value = val.natural16.value.elements[0];
                clCanard.module_th.MTH.metadata.timerange.Pindicator.value = val.natural16.value.elements[0];
                clCanard.module_th.NTH.metadata.timerange.Pindicator.value = val.natural16.value.elements[0];
                clCanard.module_th.XTH.metadata.timerange.Pindicator.value = val.natural16.value.elements[0];

                // Passa alle sottoscrizioni
                state = SETUP;
                break;

            // ********************************************************************************
            //               AVVIA SOTTOSCRIZIONI ai messaggi per servizi RPC ecc...
            // ********************************************************************************
            case SETUP:

                // Plug and Play Versione 1_0 CAN_CLASSIC senza nodo ID valido
                if (clCanard.is_canard_node_anonymous()) {
                    // PnP over Classic CAN, use message v1.0
                    if (!clCanard.rxSubscribe(CanardTransferKindMessage,
                                            uavcan_pnp_NodeIDAllocationData_1_0_FIXED_PORT_ID_,
                                            uavcan_pnp_NodeIDAllocationData_1_0_EXTENT_BYTES_,
                                            CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                        LOCAL_ASSERT(false);
                    }
                }

                // Service Client: -> Verifica della presenza Heartbeat del MASTER [Networks OffLine]
                if (!clCanard.rxSubscribe(CanardTransferKindMessage,
                                        uavcan_node_Heartbeat_1_0_FIXED_PORT_ID_,
                                        uavcan_node_Heartbeat_1_0_EXTENT_BYTES_,
                                        CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                    LOCAL_ASSERT(false);
                }

                // Service Client: -> Sincronizzazione timestamp Microsecond del MASTER [su base time local]
                if (!clCanard.rxSubscribe(CanardTransferKindMessage,
                                        uavcan_time_Synchronization_1_0_FIXED_PORT_ID_,
                                        uavcan_time_Synchronization_1_0_EXTENT_BYTES_,
                                        CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                    LOCAL_ASSERT(false);
                }

                // Service servers: -> Risposta per GetNodeInfo richiesta esterna master (Yakut, Altri)
                if (!clCanard.rxSubscribe(CanardTransferKindRequest,
                                        uavcan_node_GetInfo_1_0_FIXED_PORT_ID_,
                                        uavcan_node_GetInfo_Request_1_0_EXTENT_BYTES_,
                                        CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                    LOCAL_ASSERT(false);
                }

                // Service servers: -> Risposta per ExecuteCommand richiesta esterna master (Yakut, Altri)
                if (!clCanard.rxSubscribe(CanardTransferKindRequest,
                                        uavcan_node_ExecuteCommand_1_1_FIXED_PORT_ID_,
                                        uavcan_node_ExecuteCommand_Request_1_1_EXTENT_BYTES_,
                                        CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                    LOCAL_ASSERT(false);
                }

                // Service servers: -> Risposta per Accesso ai registri richiesta esterna master (Yakut, Altri)
                if (!clCanard.rxSubscribe(CanardTransferKindRequest,
                                        uavcan_register_Access_1_0_FIXED_PORT_ID_,
                                        uavcan_register_Access_Request_1_0_EXTENT_BYTES_,
                                        CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                    LOCAL_ASSERT(false);
                }

                // Service servers: -> Risposta per Lista dei registri richiesta esterna master (Yakut, Altri)
                // Time OUT Canard raddoppiato per elenco registri (Con molte Call vado in TimOut)
                // Con raddoppio del tempo Default problema risolto
                if (!clCanard.rxSubscribe(CanardTransferKindRequest,
                                        uavcan_register_List_1_0_FIXED_PORT_ID_,
                                        uavcan_register_List_Request_1_0_EXTENT_BYTES_,
                                        CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                    LOCAL_ASSERT(false);
                }

                // Service servers: -> Risposta per dati e metadati sensore modulo corrente da master (Yakut, Altri)
                if (!clCanard.rxSubscribe(CanardTransferKindRequest,
                                        clCanard.port_id.service_module_th,
                                        rmap_service_module_TH_Request_1_0_EXTENT_BYTES_,
                                        CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                    LOCAL_ASSERT(false);
                }

                // Service client: -> Risposta per Read (Receive) File local richiesta esterna (Yakut, Altri)
                if (!clCanard.rxSubscribe(CanardTransferKindResponse,
                                        uavcan_file_Read_1_1_FIXED_PORT_ID_,
                                        uavcan_file_Read_Response_1_1_EXTENT_BYTES_,
                                        CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                    LOCAL_ASSERT(false);
                }

                // Avvio il modulo UAVCAN in modalità operazionale normale
                // Eventuale SET Flag dopo acquisizione di configurazioni e/o parametri da Remoto
                clCanard.flag.set_local_node_mode(uavcan_node_Mode_1_0_OPERATIONAL);

                // Set START Timetable LOOP RX/TX. Set Canard microsecond start, per le sincronizzazioni
                clCanard.getMicros(clCanard.start_syncronization);
                last_pub_rmap_data = clCanard.getMicros(clCanard.syncronized_time) + MEGA * (TIME_PUBLISH_MODULE_DATA + random(50) / 100);
                last_pub_heartbeat = clCanard.getMicros(clCanard.syncronized_time) + MEGA * (TIME_PUBLISH_HEARTBEAT + random(100) / 100);
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
                // Test con reboot e successiva risincronizzazione automatica (FAKE RTC!!!!)
                #ifdef LED_ON_SYNCRO_TIME
                // Utilizzo di RTC o locale o generato dal tickMicros locale a partire dall' ultimo SetTimeStamp
                // E' utilizzabile come RTC_FAKE e/o come Setup Perfetto per regolazione RTC al cambio del secondo
                // RTC Infatti non permette la regolazione dei microsecondi, e questa tecnica lo consente
                // Verifico LED al secondo... su timeSTamp sincronizzato remoto
                if((clCanard.master.timestamp.get_timestamp_syncronized() / MEGA) % 2) {
                    digitalWrite(LED2_PIN, HIGH);
                } else {
                    digitalWrite(LED2_PIN, LOW);
                }
                #endif

                // ************************************************************************
                // ***********               CHECK OFFLINE/ONLINE               ***********
                // ************************************************************************

                // Check eventuale Nodo Master OFFLINE (Ultimo comando sempre perchè posso)
                // Effettuare eventuali operazioni di SET,RESET Cmd in sicurezza
                // Entro in OffLine ed eseguo eventuali operazioni di entrata
                if (clCanard.master.heartbeat.is_online()) {
                    // Solo quando passo da OffLine ad OnLine controllo con VarLocale
                    if (masterOnline != true) {
                        Serial.println(F("Master controller ONLINE !!! -> OnLineFunction()"));
                        // .... Codice OnLineFunction()
                    }
                    masterOnline = true;
                    // **************************************************************************
                    //            STATO MODULO (Decisionale in funzione di stato remoto)
                    // Gestione continuativa del modulo di acquisizione master.clCanard (flag remoto)
                    // **************************************************************************
                    // TODO: SOLO CODICE DI ESEMPIO DA GESTIRE
                    // Il master comunica nell'HeartBeat il proprio stato che viene gestito qui se OnLine
                    // Gestione attività (es. risparmio energetico, altro in funzione di codice remoto)
                    /*
                    switch (clCanard.master.state) {
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

                // *************************** RMAP DATA PUBLISHER **************************
                // Pubblica i dati del nodo corrente se configurata e abilitata la funzione
                // Ovviamente il nodo non può essere anonimo per la pubblicazione...
                if ((!clCanard.is_canard_node_anonymous()) &&
                    (clCanard.publisher_enabled.module_th) &&
                    (clCanard.port_id.publisher_module_th <= CANARD_SUBJECT_ID_MAX)) {
                    if (clCanard.getMicros(clCanard.syncronized_time) >= last_pub_rmap_data)
                    {
                        // Update publisher
                        last_pub_rmap_data += MEGA * TIME_PUBLISH_MODULE_DATA;
                        // Funzione locale privata
                        publish_rmap_data(clCanard, &param);
                    }
                }

                // ************************* HEARTBEAT DATA PUBLISHER ***********************
                if (clCanard.getMicros(clCanard.syncronized_time) >= last_pub_heartbeat) {
                    if(clCanard.is_canard_node_anonymous()) {
                        #ifdef LOG_HEARTBEAT
                        Serial.print(F("Publish SLAVE PNP Request Message -->> ["));
                        Serial.print(TIME_PUBLISH_PNP_REQUEST);
                        Serial.println(F(" sec + Rnd * 1 sec...]"));
                        #endif
                        clCanard.slave_pnp_send_request();
                        last_pub_heartbeat += MEGA * (TIME_PUBLISH_PNP_REQUEST + random(100) / 100);
                    } else {
                        #ifdef LOG_HEARTBEAT
                        Serial.print(F("Publish SLAVE Heartbeat -->> ["));
                        Serial.print(TIME_PUBLISH_HEARTBEAT);
                        Serial.println(F(" sec]"));
                        #endif
                        clCanard.slave_heartbeat_send_message();
                        // Update publisher
                        last_pub_heartbeat += MEGA * TIME_PUBLISH_HEARTBEAT;
                    }
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
                    clCanard.slave_servicelist_send_message();
                }

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
