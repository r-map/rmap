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

#define TRACE_LEVEL     CAN_TASK_TRACE_LEVEL
#define LOCAL_TASK_ID   CAN_TASK_ID

#include "tasks/can_task.h"

#if (ENABLE_CAN)

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
    if (canPower == ModeCan) return;
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

/// @brief Ritorna unique-ID 128-bit del nodo locale. E' utilizzato in uavcan.node.GetInfo.Response e durante
///        plug-and-play node-ID allocation da uavcan.pnp.NodeIDAllocationData. SerialNumber, Produttore..
///        Dovrebbe essere verificato in uavcan.node.GetInfo.Response per la verifica non sia cambiato Nodo.
///        Al momento vengono inseriti 2 BYTE fissi, altri eventuali, che Identificano il Tipo Modulo
/// @param out data out UniqueID
/// @param serNumb local Hardware Serial Number (64Bit) Already Send in PNP 1.0 Request (Hash 48Bit)
void CanTask::getUniqueID(uint8_t out[uavcan_node_GetInfo_Response_1_0_unique_id_ARRAY_CAPACITY_], uint64_t serNumb)
{
    // A real hardware node would read its unique-ID from some hardware-specific source (typically stored in ROM).
    // This example is a software-only node so we store the unique-ID in a (read-only) register instead.
    static uavcan_register_Value_1_0 val = {0};
    uavcan_register_Value_1_0_select_unstructured_(&val);

    // Crea default unique_id con 8 BYTES From local_serial Number (N.B. serNumb[0] rappresenta Tipo Nodo )
    uint8_t *ptrData = (uint8_t*)&serNumb;
    for (uint8_t i = 0; i < 8; i++)
    {
        val.unstructured.value.elements[val.unstructured.value.count++] = ptrData[i];
    }
    // Il resto dei 128 vengono impostati RANDOM
    for (uint8_t i = val.unstructured.value.count; i < uavcan_node_GetInfo_Response_1_0_unique_id_ARRAY_CAPACITY_; i++)
    {
        val.unstructured.value.elements[val.unstructured.value.count++] = (uint8_t) rand();  // NOLINT
    }
    localRegisterAccessLock->Take();
    localRegister->read("uavcan.node.unique_id", &val);
    localRegisterAccessLock->Give();
    LOCAL_ASSERT(uavcan_register_Value_1_0_is_unstructured_(&val) &&
           val.unstructured.value.count == uavcan_node_GetInfo_Response_1_0_unique_id_ARRAY_CAPACITY_);
    memcpy(&out[0], &val.unstructured.value, uavcan_node_GetInfo_Response_1_0_unique_id_ARRAY_CAPACITY_);
}

/// @brief Scrive dati in append su Flash per scrittura sequenziale file data remoto
/// @param file_name nome del file UAVCAN
/// @param is_firmware true se il file +-è di tipo firmware
/// @param rewrite true se necessaria la riscrittura del file
/// @param buf blocco dati da scrivere in formato UAVCAN [256 Bytes]
/// @param count numero del blocco da scrivere in formato UAVCAN [Blocco x Buffer]
/// @return true if block saved OK, false on any error
bool CanTask::putFlashFile(const char* const file_name, const bool is_firmware, const bool rewrite, void* buf, size_t count)
{
    #ifdef CHECK_FLASH_WRITE
    // check data (W->R) Verify Flash integrity OK    
    uint8_t check_data[FLASH_BUFFER_SIZE];
    #endif
    // Request New File Init Upload
    if(rewrite) {
        // Qspi Security Semaphore
        if(localQspiLock->Take(Ticks::MsToTicks(FLASH_SEMAPHORE_MAX_WAITING_TIME_MS))) {
            // Init if required (DeInit after if required PowerDown Module)
            if(localFlash->BSP_QSPI_Init() != Flash::QSPI_OK) {
                localQspiLock->Give();
                return false;
            }
            // Check Status Flash OK
            Flash::QSPI_StatusTypeDef sts = localFlash->BSP_QSPI_GetStatus();
            if (sts) {
                localQspiLock->Give();
                return false;
            }
            // Start From PtrFlash 0x100 (Reserve 256 Bytes For InfoFile)
            if (is_firmware) {
                // Firmware Flash
                canFlashPtr = FLASH_FW_POSITION;
            } else {
                // Standard File Data Upload
                canFlashPtr = FLASH_FILE_POSITION;
            }
            // Get Block Current into Flash
            canFlashBlock = canFlashPtr / AT25SF641_BLOCK_SIZE;
            // Erase First Block Block (Block OF 4KBytes)
            TRACE_INFO_F(F("FLASH: Erase block: %d\n\r"), canFlashBlock);
            if (localFlash->BSP_QSPI_Erase_Block(canFlashBlock)) {
                localQspiLock->Give();
                return false;
            }
            // Write Name File (Size at Eof...)
            uint8_t file_flash_name[FLASH_FILE_SIZE_LEN] = {0};
            memcpy(file_flash_name, file_name, strlen(file_name));
            localFlash->BSP_QSPI_Write(file_flash_name, canFlashPtr, FLASH_FILE_SIZE_LEN);
            // Write into Flash
            TRACE_INFO_F(F("FLASH: Write [ %d ] bytes at addr: %d\n\r"), FLASH_FILE_SIZE_LEN, canFlashPtr);
            #ifdef CHECK_FLASH_WRITE
            localFlash->BSP_QSPI_Read(check_data, canFlashPtr, FLASH_FILE_SIZE_LEN);
            if(memcmp(file_flash_name, check_data, FLASH_FILE_SIZE_LEN)==0) {
                TRACE_INFO_F(F("FLASH: Reading check OK\n\r"));
            } else {
                TRACE_ERROR_F(F("FLASH: Reading check ERROR\n\r"));
                localQspiLock->Give();
                return false;
            }
            #endif
            // Start Page...
            canFlashPtr += FLASH_INFO_SIZE_LEN;
            localQspiLock->Give();
        }
    }
    // Write Data Block
    // Qspi Security Semaphore
    if(localQspiLock->Take(Ticks::MsToTicks(FLASH_SEMAPHORE_MAX_WAITING_TIME_MS))) {
        // 0 = Is UavCan Signal EOF for Last Block Exact Len 256 Bytes...
        // If Value Count is 0 no need to Write Flash Data (Only close Fule Info)
        if(count!=0) {
            // Write into Flash
            TRACE_INFO_F(F("FLASH: Write [ %d ] bytes at addr: %d\n\r"), count, canFlashPtr);
            // Starting Write at OFFSET Required... Erase here is Done
            localFlash->BSP_QSPI_Write((uint8_t*)buf, canFlashPtr, count);
            #ifdef CHECK_FLASH_WRITE
            localFlash->BSP_QSPI_Read(check_data, canFlashPtr, count);
            if(memcmp(buf, check_data, count)==0) {
                TRACE_INFO_F(F("FLASH: Reading check OK\n\r"));
            } else {
                TRACE_ERROR_F(F("FLASH: Reading check ERROR\n\r"));
                localQspiLock->Give();
                return false;
            }
            #endif
            canFlashPtr += count;
            // Check if Next Page Addressed (For Erase Next Block)
            if((canFlashPtr / AT25SF641_BLOCK_SIZE) != canFlashBlock) {
                canFlashBlock = canFlashPtr / AT25SF641_BLOCK_SIZE;
                // Erase First Block Block (Block OF 4KBytes)
                TRACE_INFO_F(F("FLASH: Erase block: %d\n\r"), canFlashBlock);
                if (localFlash->BSP_QSPI_Erase_Block(canFlashBlock)) {
                    localQspiLock->Give();
                    return false;
                }
            }
        }
        // Eof if != 256 Bytes Write
        if(count!=0x100) {
            // Write Info File for Closing...
            // Size at 
            uint64_t lenghtFile = canFlashPtr - FLASH_INFO_SIZE_LEN;
            if (is_firmware) {
                // Firmware Flash
                canFlashPtr = FLASH_FW_POSITION;
            } else {
                // Standard File Data Upload
                canFlashPtr = FLASH_FILE_POSITION;
            }
            localFlash->BSP_QSPI_Write((uint8_t*)&lenghtFile, FLASH_SIZE_ADDR(canFlashPtr), FLASH_INFO_SIZE_U64);
            // Write into Flash
            TRACE_INFO_F(F("FLASH: Write [ %d ] bytes at addr: %d\n\r"), FLASH_INFO_SIZE_U64, canFlashPtr);
            #ifdef CHECK_FLASH_WRITE
            localFlash->BSP_QSPI_Read(check_data, FLASH_SIZE_ADDR(canFlashPtr), FLASH_INFO_SIZE_U64);
            if(memcmp(&lenghtFile, check_data, FLASH_INFO_SIZE_U64)==0) {
                TRACE_INFO_F(F("FLASH: Reading check OK\n\r"));
            } else {
                TRACE_INFO_F(F("FLASH: Reading check ERROR\n\r"));
            }
            #endif
        }
        localQspiLock->Give();
    }
    return true;
}


/// @brief GetInfo for Firmware File on Flash
/// @param module_type type module of firmware
/// @param version version firmware
/// @param revision revision firmware
/// @param len length of file in bytes
/// @return true if exixst
bool CanTask::getFlashFwInfoFile(uint8_t *module_type, uint8_t *version, uint8_t *revision, uint64_t *len)
{
    uint8_t block[FLASH_FILE_SIZE_LEN];
    bool fileReady = false;

    // Qspi Security Semaphore
    if(localQspiLock->Take(Ticks::MsToTicks(FLASH_SEMAPHORE_MAX_WAITING_TIME_MS))) {
        // Init if required (DeInit after if required PowerDown Module)
        if(localFlash->BSP_QSPI_Init() != Flash::QSPI_OK) {
            localQspiLock->Give();
            return false;
        }
        // Check Status Flash OK
        if (localFlash->BSP_QSPI_GetStatus()) {
            localQspiLock->Give();
            return false;
        }

        // Read Name file, Version and Info
        localFlash->BSP_QSPI_Read(block, 0, FLASH_FILE_SIZE_LEN);
        char stima_name[STIMA_MODULE_NAME_LENGTH] = {0};
        getStimaNameByType(stima_name, MODULE_TYPE);
        if(checkStimaFirmwareType((char*)block, module_type, version, revision)) {
            localFlash->BSP_QSPI_Read((uint8_t*)len, FLASH_SIZE_ADDR(0), FLASH_INFO_SIZE_U64);
            fileReady = true;
        }
        localQspiLock->Give();
    }
    return fileReady;
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
            TRACE_INFO_F(F("Firmware update request from node id: %u\r\n"), clCanard.master.file.get_server_node());
            TRACE_INFO_F(F("Filename to download: %s\r\n"), clCanard.master.file.get_name());
            // Avvio la funzione con OK
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        case uavcan_node_ExecuteCommand_Request_1_1_COMMAND_FACTORY_RESET:
        {
            localRegisterAccessLock->Take();
            localRegister->doFactoryReset();
            localRegisterAccessLock->Give();
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
            TRACE_INFO_F(F("File standard update request from node id: %u\r\n"), clCanard.master.file.get_server_node());
            TRACE_INFO_F(F("Filename to download: %s\r\n"), clCanard.master.file.get_name());
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
        localRegisterAccessLock->Take();
        localRegister->read(&name[0], &resp.value);
        localRegisterAccessLock->Give();
        // If such register exists and it can be assigned from the request value:
        if (!uavcan_register_Value_1_0_is_empty_(&resp.value) && localRegister->assign(&resp.value, &req->value)) {
            localRegisterAccessLock->Take();
            localRegister->write(&name[0], &resp.value);
            localRegisterAccessLock->Give();
        }
    }

    // Regardless of whether we've just wrote a value or not, we need to read the current one and return it.
    // The client will determine if the write was successful or not by comparing the request value with response.
    uavcan_register_Value_1_0_select_empty_(&resp.value);
    localRegisterAccessLock->Take();
    localRegister->read(&name[0], &resp.value);
    localRegisterAccessLock->Give();

    // Currently, all registers we implement are mutable and persistent. This is an acceptable simplification,
    // but more advanced implementations will need to differentiate between them to support advanced features like
    // exposing internal states via registers, perfcounters, etc.
    resp._mutable = true;
    resp.persistent = true;

    // Our node does not synchronize its time with the network so we can't populate the timestamp.
    resp.timestamp.microsecond = uavcan_time_SynchronizedTimestamp_1_0_UNKNOWN;

    return resp;
}

/// @brief Risposta a uavcan.node.GetInfo which Info Node (nome, versione, uniqueID di verifica ecc...)
/// @return Risposta node Get INFO UAVCAN
uavcan_node_GetInfo_Response_1_0 CanTask::processRequestNodeGetInfo() {
    uavcan_node_GetInfo_Response_1_0 resp = {0};
    resp.protocol_version.major = CANARD_CYPHAL_SPECIFICATION_VERSION_MAJOR;
    resp.protocol_version.minor = CANARD_CYPHAL_SPECIFICATION_VERSION_MINOR;

    // The hardware version is not populated in this demo because it runs on no specific hardware.
    // An embedded node would usually determine the version by querying the hardware.

    resp.software_version.major = MODULE_MAIN_VERSION;
    resp.software_version.minor = MODULE_MINOR_VERSION;
    resp.software_vcs_revision_id = RMAP_PROCOTOL_VERSION;

    getUniqueID(resp.unique_id, StimaV4GetSerialNumber());

    // The node name is the name of the product like a reversed Internet domain name (or like a Java package).
    char stima_name[STIMA_MODULE_NAME_LENGTH] = {0};
    getStimaNameByType(stima_name, MODULE_TYPE);
    resp.name.count = strlen(stima_name);
    memcpy(&resp.name.elements, stima_name, resp.name.count);

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
                TRACE_VERBOSE_F(F("RX PNP Allocation message request from -> "));
                switch(msg.unique_id_hash & 0xFF) {
                    case canardClass::Module_Type::th:
                        TRACE_VERBOSE_F(F("Anonimous module TH"));
                        defaultNodeId = clCanard.getPNPValidIdFromNodeType(canardClass::Module_Type::th);
                        break;
                    case canardClass::Module_Type::rain:
                        TRACE_VERBOSE_F(F("Anonimous module RAIN"));
                        defaultNodeId = clCanard.getPNPValidIdFromNodeType(canardClass::Module_Type::rain);
                        break;
                    case canardClass::Module_Type::wind:
                        TRACE_VERBOSE_F(F("Anonimous module WIND"));
                        defaultNodeId = clCanard.getPNPValidIdFromNodeType(canardClass::Module_Type::wind);
                        break;
                    case canardClass::Module_Type::radiation:
                        TRACE_VERBOSE_F(F("Anonimous module RADIATION"));
                        defaultNodeId = clCanard.getPNPValidIdFromNodeType(canardClass::Module_Type::radiation);
                        break;
                    case canardClass::Module_Type::vwc:
                        TRACE_VERBOSE_F(F("Anonimous module VWC"));
                        defaultNodeId = clCanard.getPNPValidIdFromNodeType(canardClass::Module_Type::vwc);
                        break;
                    case canardClass::Module_Type::power:
                        TRACE_VERBOSE_F(F("Anonimous module POWER"));
                        defaultNodeId = clCanard.getPNPValidIdFromNodeType(canardClass::Module_Type::power);
                        break;
                    default:
                        // PNP Non gestibile
                        TRACE_VERBOSE_F(F("Anonimous module Unknown"));
                        defaultNodeId = GENERIC_BVAL_UNDEFINED;
                        break;
                }
                TRACE_VERBOSE_F(F(" [Serial Number UID: %lu]"), (uint32_t) msg.unique_id_hash);

                // Risposta immediata diretta (Se nodo ovviamente è riconosciuto...)
                // Non utilizziamo una Response in quanto l'allocation è sempre un messaggio anonimo
                // I metadati del trasporto sono come quelli riceuti del transferID quindi è un messaggio
                // che si comporta parzialmente come una risposta (per rilevamento remoto hash/transfer_id)
                if(defaultNodeId <= CANARD_NODE_ID_MAX) {
                    TRACE_VERBOSE_F(F("Try PNP Allocation with Node_ID -> %d"), defaultNodeId);
                    // Se il nodo proposto viene confermato inizieremo a ricevere heartbeat
                    // da quel nodeId. A questo punto in Heartbeat settiamo il flag pnp.configure()
                    // che conclude la procedura con esito positivo.
                    msg.allocated_node_id.count = 1;
                    msg.allocated_node_id.elements[0].value = defaultNodeId;
                    // The request object is empty so we don't bother deserializing it. Just send the response.
                    uint8_t serialized[uavcan_pnp_NodeIDAllocationData_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
                    size_t  serialized_size = sizeof(serialized);
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
                TRACE_VERBOSE_F(F("RX HeartBeat from node %u\r\n"), transfer->metadata.remote_node_id);
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
                        localRegisterAccessLock->Take();
                        localRegister->write(registerName, &val);
                        localRegisterAccessLock->Give();
                        TRACE_VERBOSE_F(F("Node is now configured with PNP allocation with ID: %d\n\r"), transfer->metadata.remote_node_id);
                    }                    
                    // Accodo i dati letti dal messaggio (Nodo -> OnLine) verso la classe
                    clCanard.slave[queueId].heartbeat.set_online(NODE_OFFLINE_TIMEOUT_US,
                        msg.vendor_specific_status_code, msg.health.value, msg.mode.value, msg.uptime);  
                    // Rientro in OnLINE da OFFLine o Init Gestino può (dovrebbe) essere esterna alla Call
                    // Inizializzo le variabili e gli stati necessari per Reset e corretta gestione
                    if(clCanard.slave[queueId].is_entered_online()) {
                        TRACE_VERBOSE_F(F("Node is now entered ONLINE !!!\n\r"));
                        // Metto i Flag in sicurezza, laddove dove non eventualmente gestito
                        clCanard.slave[queueId].command.reset_pending();
                        clCanard.slave[queueId].register_access.reset_pending();
                        clCanard.slave[queueId].file_server.reset_pending();
                        clCanard.slave[queueId].rmap_service.reset_pending();
                        // Set system_status from local to static access (Set Node Online)
                        localSystemStatusLock->Take();
                        localSystemStatus->data_slave[queueId].is_online = true;
                        localSystemStatusLock->Give();
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
                        rmap_module_TH_1_0 msg = {0};
                        size_t size = transfer->payload_size;
                        if (rmap_module_TH_1_0_deserialize_(&msg, static_cast<uint8_t const*>(transfer->payload), &size) >= 0)
                        {
                            // msg contiene i dati di blocco pubblicati
                            TRACE_VERBOSE_F(F("Ricevuto dato in publisher modulo_th -> ID: %d, transfer ID: %d\n\r"),
                                transfer->metadata.remote_node_id, transfer->metadata.transfer_id);
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
            uint8_t serialized[uavcan_node_GetInfo_Response_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
            size_t  serialized_size = sizeof(serialized);
            const int8_t res = uavcan_node_GetInfo_Response_1_0_serialize_(&resp, &serialized[0], &serialized_size);
            if (res >= 0) {
                clCanard.sendResponse(MEGA, &transfer->metadata, serialized_size, &serialized[0]);
            }
        }
        else if (transfer->metadata.port_id == uavcan_register_Access_1_0_FIXED_PORT_ID_)
        {
            uavcan_register_Access_Request_1_0 req = {0};
            size_t size = transfer->payload_size;
            TRACE_INFO_F(F("<<-- Ricevuto richiesta accesso ai registri\r\n"));
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
            uavcan_register_List_Request_1_0 req = {0};
            size_t size = transfer->payload_size;
            TRACE_INFO_F(F("<<-- Ricevuto richiesta lettura elenco registri\r\n"));
            if (uavcan_register_List_Request_1_0_deserialize_(&req, static_cast<uint8_t const*>(transfer->payload), &size) >= 0)
            {
                uavcan_register_List_Response_1_0 resp;
                resp.name =  localRegister->getNameByIndex(req.index);
                uint8_t serialized[uavcan_register_List_Response_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
                size_t  serialized_size = sizeof(serialized);
                if (uavcan_register_List_Response_1_0_serialize_(&resp, &serialized[0], &serialized_size) >= 0) {
                    clCanard.sendResponse(MEGA, &transfer->metadata, serialized_size, &serialized[0]);
                }
            }
        }
        else if (transfer->metadata.port_id == uavcan_node_ExecuteCommand_1_1_FIXED_PORT_ID_)
        {
            uavcan_node_ExecuteCommand_Request_1_1 req = {0};
            size_t size = transfer->payload_size;
            TRACE_INFO_F(F("<<-- Ricevuto comando esterno\r\n"));
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
            if((clCanard.slave[queueId].file_server.get_state() == clCanard.state_uploading) ||
                (clCanard.slave[queueId].file_server.get_state() == clCanard.command_wait)) {
                // Update TimeOut (Comunico request OK al Master, Slave sta scaricando)
                // Se Slave si blocca per TimeOut, esco dalla procedura dove gestita
                clCanard.slave[queueId].file_server.start_pending(NODE_REQFILE_TIMEOUT_US);
                uavcan_file_Read_Request_1_1 req = {0};
                size_t size = transfer->payload_size;
                if (uavcan_file_Read_Request_1_1_deserialize_(&req, static_cast<uint8_t const*>(transfer->payload), &size) >= 0)
                {
                    uavcan_file_Read_Response_1_1 resp = {0};
                    byte dataBuf[uavcan_primitive_Unstructured_1_0_value_ARRAY_CAPACITY_];
                    // Terminatore di sicurezza
                    req.path.path.elements[req.path.path.count] = 0;
                    // Allego il blocco dati se presente
                    size_t dataLen = uavcan_primitive_Unstructured_1_0_value_ARRAY_CAPACITY_;
                    // N.B. uavcan_primitive_Unstructured_1_0_value_ARRAY_CAPACITY_ == FILE_GET_DATA_BLOCK_SIZE (256 BYTES)
                    #if uavcan_primitive_Unstructured_1_0_value_ARRAY_CAPACITY_ != FILE_GET_DATA_BLOCK_SIZE
                        #error Buffer file queue isn't equal to uavcan_primitive_Unstructured_1_0_value_ARRAY_CAPACITY_
                    #endif

                    // ************************************************************************
                    //               GET Data File (Firmware) from SD CARD Queue
                    // ************************************************************************
                    bool file_download_error = false;

                    if(!localSystemStatus->flags.sd_card_ready) {
                        file_download_error = true;
                        TRACE_VERBOSE_F(F("CAN: Reject request upload file (Firmware) SD CARD was not ready [ %s ]\r\n"), ERROR_STRING);
                    } else {
                        // Is first block? Perform an request to open file into task sd card
                        if((req.offset / FILE_GET_DATA_BLOCK_SIZE) == 0) {
                            // First block NAME OF FILE (Prepare queue GET struct with file_name and start position)                            
                            memset(&firmwareDownloadChunck, 0, sizeof(file_get_request_t));
                            // Init name file (only first blcok for any board_id request)
                            firmwareDownloadChunck.file_name = clCanard.slave[queueId].file_server.get_file_name();
                            firmwareDownloadChunck.block_read_next = false;
                            TRACE_VERBOSE_F(F("Starting upload file (Firmware) from local SD Card to remote module CAN [ %s ]\r\n"), firmwareDownloadChunck.file_name);
                        }
                        // Switch state of queue download file from sd card
                        firmwareDownloadChunck.block_id = req.offset / FILE_GET_DATA_BLOCK_SIZE;
                        firmwareDownloadChunck.board_id = queueId; // Set BOARDS Require File (Need for multiserver file request)
                        // Pushing data request to queue task sd card
                        localDataFileGetRequestQueue->Enqueue(&firmwareDownloadChunck, 0);
                        // Waiting response from MMC with TimeOUT
                        memset(&sdcard_task_response, 0, sizeof(file_get_response_t));
                        LocalTaskWatchDog(FILE_GET_DATA_QUEUE_TIMEOUT);
                        file_download_error = !localDataFileGetResponseQueue->Dequeue(&sdcard_task_response, FILE_GET_DATA_QUEUE_TIMEOUT);
                        file_download_error |= !sdcard_task_response.done_operation;
                    }
                    // Block data is avaiable? -> Send to remote node
                    if(file_download_error) {
                        // any error? (optional retry... from remote node. File server wait another request)
                        TRACE_VERBOSE_F(F("Download data file from queue error!!!\r\n"));
                        resp._error.value = uavcan_file_Error_1_0_IO_ERROR;
                        dataLen = 0;
                    } else {
                        // Send block
                        if(sdcard_task_response.block_lenght!=FILE_GET_DATA_BLOCK_SIZE) {
                            // EOF Last Block (bytes read != FILE_GET_DATA_BLOCK_SIZE)
                            // No other response/request are to get
                            TRACE_VERBOSE_F(F("End of data block data, readed [ %lu ] bytes\r\n"), ((uint32_t)(firmwareDownloadChunck.block_id) * (uint32_t)(FILE_GET_DATA_BLOCK_SIZE)) + sdcard_task_response.block_lenght);
                        } else {
                            // Normal block, prepare for next block
                            TRACE_VERBOSE_F(F("Read data block id:%d and increment file pointer\r\n"), firmwareDownloadChunck.block_id++);
                        }
                    }
                    // *********** END GET Data File (Firmware) from SD CARD Queue ************

                    // Preparo la risposta corretta
                    resp.data.value.count = sdcard_task_response.block_lenght;
                    memcpy(resp.data.value.elements, sdcard_task_response.block, sdcard_task_response.block_lenght);
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
            size_t size = transfer->payload_size;
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
            size_t size = transfer->payload_size;
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
                uavcan_file_Read_Response_1_1 resp = {0};
                size_t size = transfer->payload_size;
                if (uavcan_file_Read_Response_1_1_deserialize_(&resp, static_cast<uint8_t const*>(transfer->payload), &size) >= 0) {
                    if(clCanard.master.file.is_firmware()) {
                        TRACE_VERBOSE_F(F("RX FIRMWARE READ BLOCK LEN: "));
                    } else {
                        TRACE_VERBOSE_F(F("RX FILE READ BLOCK LEN: "));
                    }
                    TRACE_VERBOSE_F(F("%d\r\n"), resp.data.value.count);
                    // Save Data in Flash File at Block Position (Init = Rewrite file...)
                    if(putFlashFile(clCanard.master.file.get_name(), clCanard.master.file.is_firmware(), clCanard.master.file.is_first_data_block(),
                                  resp.data.value.elements, resp.data.value.count)) {
                        // Reset pending command (Comunico request/Response Serie di comandi OK!!!)
                        // Uso l'Overload con controllo di EOF (-> EOF se msgLen != UAVCAN_BLOCK_DEFAULT [256 Bytes])
                        // Questo Overload gestisce in automatico l'offset del messaggio, per i successivi blocchi
                        clCanard.master.file.reset_pending(resp.data.value.count);
                    } else {
                        // Error Save... Abort request
                        TRACE_ERROR_F(F("SAVING BLOCK FILE ERROR, ABORT RX !!!\r\n"));
                        clCanard.master.file.download_end();
                    }
                }
            } else {
                // Errore Nodo non settato...
                TRACE_ERROR_F(F("RX FILE READ BLOCK REJECT: Node_Id not valid or not set\r\n"));
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
                            // TODO: READ Struct DATA && SEND TO SD CARD OR TASK ELABORATE_SAVING_DATA!!!
                            // PREFEEIBILE UNA CODA DA XX DATI x XXMAX SIZE E PASSARE IL DATO COPIATO INTERO
                            // CON IL PRIMO BYTE CHE E' IL RIFERIMENTO AL TIPO DI MODULO
                            // COSI' IL DATA SAVE VA CON I SUOI TEMPI ANCHE SE STRUTTURA E GRANDE E PIU' SICURO
                            // IL PASSAGGIO DI UN INDIRIZZO PREVEDE CHE IL DATO RIMANGA DISPONIBILE NELLA
                            // STRUCT CAN GENERICA MA UN SECONDO DATO LA CANCELLEREBBE...
                            typedef struct
                            {
                                uint32_t data_ora;
                                uint8_t buffer[300];
                            } dato_t;
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
CanTask::CanTask(const char *taskName, uint16_t stackSize, uint8_t priority, CanParam_t CanParam) : Thread(taskName, stackSize, priority), param(CanParam)
{
    // Start WDT controller and TaskState Flags
    TaskWatchDog(WDT_STARTING_TASK_MS);
    TaskState(CAN_STATE_CREATE, UNUSED_SUB_POSITION, task_flag::normal);

    // Setup register mode
    localRegister = param.clRegister;

    // Setup Flash Access
    localFlash = param.flash;

    // Local static access to global queue and Semaphore
    localSystemMessageQueue = param.systemMessageQueue;
    localDataFileGetRequestQueue = param.dataFileGetRequestQueue;
    localDataFileGetResponseQueue = param.dataFileGetResponseQueue;
    localQspiLock = param.qspiLock;
    localRegisterAccessLock = param.registerAccessLock;
    localSystemStatusLock = param.systemStatusLock;
    localSystemStatus = param.system_status;

    // FullChip Power Mode after Startup
    // Resume from LowPower or reset the controller TJA1443ATK
    // Need FullPower for bxCan Programming (Otherwise Handler_Error()!)
    HW_CAN_Power(CAN_ModePower::CAN_INIT);

    TRACE_INFO_F(F("Starting CAN Configuration\r\n"));
 
    // *******************    CANARD MTU CLASSIC (FOR UAVCAN REQUIRE)     *******************
    // Open Register in Write se non inizializzati correttamente...
    // Populate INIT Default Value
    static uavcan_register_Value_1_0 val = {0};
    uavcan_register_Value_1_0_select_natural16_(&val);
    val.natural16.value.count       = 1;
    val.natural16.value.elements[0] = CAN_MTU_BASE; // CAN_CLASSIC MTU 8
    localRegisterAccessLock->Take();
    localRegister->read("uavcan.can.mtu", &val);
    localRegisterAccessLock->Give();
    LOCAL_ASSERT(uavcan_register_Value_1_0_is_natural16_(&val) && (val.natural16.value.count == 1));

    // *******************         CANARD SETUP TIMINGS AND SPEED        *******************
    // CAN BITRATE Dinamico su LoadRegister (CAN_FD 2xREG natural32 0=Speed, 1=0 (Not Used))
    uavcan_register_Value_1_0_select_natural32_(&val);
    val.natural32.value.count       = 2;
    val.natural32.value.elements[0] = CAN_BIT_RATE;
    val.natural32.value.elements[1] = 0ul;          // Ignored for CANARD_MTU_CAN_CLASSIC
    localRegisterAccessLock->Take();
    localRegister->read("uavcan.can.bitrate", &val);
    localRegisterAccessLock->Give();
    LOCAL_ASSERT(uavcan_register_Value_1_0_is_natural32_(&val) && (val.natural32.value.count == 2));

    // Dynamic BIT RATE Change CAN Speed to CAN_BIT_RATE (register default/defined)
    BxCANTimings timings;
    bool result = bxCANComputeTimings(HAL_RCC_GetPCLK1Freq(), val.natural32.value.elements[0], &timings);
    if (!result) {
        TRACE_VERBOSE_F(F("Error redefinition bxCANComputeTimings, try loading default...\r\n"));
        val.natural32.value.count       = 2;
        val.natural32.value.elements[0] = CAN_BIT_RATE;
        val.natural32.value.elements[1] = 0ul;          // Ignored for CANARD_MTU_CAN_CLASSIC
        localRegisterAccessLock->Take();
        localRegister->write("uavcan.can.bitrate", &val);
        localRegisterAccessLock->Give();
        result = bxCANComputeTimings(HAL_RCC_GetPCLK1Freq(), val.natural32.value.elements[0], &timings);
        if (!result) {
            TRACE_ERROR_F(F("Error initialization bxCANComputeTimings\r\n"));
            LOCAL_ASSERT(false);
            return;
        }
    }

    // HW Setup solo con modulo CAN Attivo
    #if (ENABLE_CAN)

    // Configurea bxCAN speed && mode
    result = bxCANConfigure(0, timings, false);
    if (!result) {
        TRACE_ERROR_F(F("Error initialization bxCANConfigure\r\n"));
        LOCAL_ASSERT(false);
        return;
    }
    // *******************     CANARD SETUP TIMINGS AND SPEED COMPLETE   *******************

    // Check error starting CAN
    if (HAL_CAN_Start(&hcan1) != HAL_OK) {
        TRACE_ERROR_F(F("CAN startup ERROR!!!\r\n"));
        LOCAL_ASSERT(false);
        return;
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

    #endif

    // Run Task Init
    state = CAN_STATE_INIT;
    Start();
};

#if (ENABLE_STACK_USAGE)
/// @brief local stack Monitor (optional)
void CanTask::TaskMonitorStack()
{
  u_int16_t stackUsage = (u_int16_t)uxTaskGetStackHighWaterMark( NULL );
  if((stackUsage) && (stackUsage < param.system_status->tasks[LOCAL_TASK_ID].stack)) {
    param.systemStatusLock->Take();
    param.system_status->tasks[LOCAL_TASK_ID].stack = stackUsage;
    param.systemStatusLock->Give();
  }
}
#endif

/// @brief local watchDog and Sleep flag Task (optional)
/// @param status system_status_t Status STIMAV4
/// @param lock if used (!=NULL) Semaphore locking system status access
/// @param millis_standby time in ms to perfor check of WDT. If longer than WDT Reset, WDT is temporanly suspend
void CanTask::TaskWatchDog(uint32_t millis_standby)
{
  // Local TaskWatchDog update
  param.systemStatusLock->Take();
  // Update WDT Signal (Direct or Long function Timered)
  if(millis_standby)  
  {
    // Check 1/2 Freq. controller ready to WDT only SET flag
    if((millis_standby) < WDT_CONTROLLER_MS / 2) {
      param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::set;
    } else {
      param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::timer;
      // Add security milimal Freq to check
      param.system_status->tasks[LOCAL_TASK_ID].watch_dog_ms = millis_standby + WDT_CONTROLLER_MS;
    }
  }
  else
    param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::set;
  param.systemStatusLock->Give();
}

/// @brief local watchDog and Sleep flag Task (optional) access from member static
/// @param status system_status_t Status STIMAV4
/// @param lock if used (!=NULL) Semaphore locking system status access
/// @param millis_standby time in ms to perfor check of WDT. If longer than WDT Reset, WDT is temporanly suspend
void CanTask::LocalTaskWatchDog(uint32_t millis_standby)
{
  // Local TaskWatchDog update
  localSystemStatusLock->Take();
  // Update WDT Signal (Direct or Long function Timered)
  if(millis_standby)  
  {
    // Check 1/2 Freq. controller ready to WDT only SET flag
    if((millis_standby) < WDT_CONTROLLER_MS / 2) {
      localSystemStatus->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::set;
    } else {
      localSystemStatus->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::timer;
      // Add security milimal Freq to check
      localSystemStatus->tasks[LOCAL_TASK_ID].watch_dog_ms = millis_standby + WDT_CONTROLLER_MS;
    }
  }
  else
    localSystemStatus->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::set;
  localSystemStatusLock->Give();
}

/// @brief local suspend flag and positor running state Task (optional)
/// @param state_position Sw_Position (Local STATE)
/// @param state_subposition Sw_SubPosition (Optional Local SUB_STATE Position Monitor)
/// @param state_operation operative mode flag status for this task
void CanTask::TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation)
{
  // Local TaskWatchDog update
  param.systemStatusLock->Take();
  // Signal Task sleep/disabled mode from request (Auto SET WDT on Resume)
  if((param.system_status->tasks[LOCAL_TASK_ID].state == task_flag::suspended)&&
     (state_operation==task_flag::normal))
     param.system_status->tasks->watch_dog = wdt_flag::set;
  param.system_status->tasks[LOCAL_TASK_ID].state = state_operation;
  param.system_status->tasks[LOCAL_TASK_ID].running_pos = state_position;
  param.system_status->tasks[LOCAL_TASK_ID].running_sub = state_subposition;
  param.systemStatusLock->Give();
}

/// @brief RUN Task
void CanTask::Run() {

    // Data Local Task (Class + Registro)
    // Avvia l'istanza alla classe State_Canard ed inizializza Ram, Buffer e variabili base
    canardClass clCanard;
    uavcan_register_Value_1_0 val = {0};

    // System message data queue structured
    system_message_t system_message;

    // LoopTimer Publish
    CanardMicrosecond last_pub_heartbeat;
    CanardMicrosecond last_pub_port_list;
    CanardMicrosecond next_timesyncro_msg;

    // Buffer to queue Out to MMC
    uint8_t dataQueue[RMAP_PUT_DATA_ELEMENT_SIZE];

    // Set when Firmware Upgrade is required
    bool start_firmware_upgrade = false;

    // Start Running Monitor and First WDT normal state
    #if (ENABLE_STACK_USAGE)
    TaskMonitorStack();
    #endif
    TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

    // Main Loop TASK
    while (true) {

        // ********************************************************************************
        //                   SETUP CONFIG CYPAL, CLASS, REGISTER, DATA
        // ********************************************************************************
        switch (state) {
            // Setup Class CB and NodeId
            case CAN_STATE_INIT:

                // Avvio inizializzazione (Standard UAVCAN MSG). Reset su INIT END OK
                // Segnale al Master necessità di impostazioni ev. parametri, Data/Ora ecc..
                clCanard.flag.set_local_node_mode(uavcan_node_Mode_1_0_INITIALIZATION);

                // Attiva il callBack su RX Messaggio Canard sulla funzione interna processReceivedTransfer
                clCanard.setReceiveMessage_CB(processReceivedTransfer);

                // Setup INIT Time for syncronized TimeStamp with local RTC
                clCanard.setMicros(rtc.getEpoch(), rtc.getSubSeconds());
                // ********************************************************************************
                //            INIT VALUE, Caricamento default e registri locali MASTER
                // ********************************************************************************

                // ********************    Lettura Registri standard UAVCAN    ********************
                // Restore the node-ID from the corresponding standard Register. Default to anonymous.
                #ifdef USE_NODE_MASTER_ID_FIXED
                // Canard Master NODE ID Fixed dal defined value in module_config
                clCanard.set_canard_node_id((CanardNodeID) NODE_MASTER_ID);
                #else
                uavcan_register_Value_1_0_select_natural16_(&val);
                val.natural16.value.count = 1;
                val.natural16.value.elements[0] = UINT16_MAX; // This means undefined (anonymous), per Specification/libcanard.
                localRegisterAccessLock->Take();
                localRegister->read("uavcan.node.id", &val);         // The names of the standard registers are regulated by the Specification.
                localRegisterAccessLock->Give();
                LOCAL_ASSERT(uavcan_register_Value_1_0_is_natural16_(&val) && (val.natural16.value.count == 1));
                if (val.natural16.value.elements[0] <= CANARD_NODE_ID_MAX) {
                    clCanard.set_canard_node_id((CanardNodeID)val.natural16.value.elements[0]);
                } else {
                    // Master must start with an ID Node (if not registerede start with default value NODE_MASTER_ID)
                    clCanard.set_canard_node_id((CanardNodeID) NODE_MASTER_ID);
                }
                #endif

                // The description register is optional but recommended because it helps constructing/maintaining large networks.
                // It simply keeps a human-readable description of the node that should be empty by default.
                uavcan_register_Value_1_0_select_string_(&val);
                val._string.value.count = 0;
                localRegisterAccessLock->Take();
                localRegister->read("uavcan.node.description", &val);  // We don't need the value, we just need to ensure it exists.
                localRegisterAccessLock->Give();

                // TODO:
                // Read Config Slave Node x Lettura porte e servizi.
                // Possibilità di utilizzo come sotto (registri) - Fixed Value adesso !!!
                #ifdef USE_SUB_PUBLISH_SLAVE_DATA
                clCanard.slave[0].configure(125, canardClass::Module_Type::th, 100, 0);
                param.system_status->data_slave[0].module_type = canardClass::Module_Type::th;
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
                    localRegisterAccessLock->Take();
                    localRegister->read(registerName, &val);
                    localRegisterAccessLock->Give();
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

                // Passa alle sottoscrizioni
                state = CAN_STATE_SETUP;
                break;

            // ********************************************************************************
            //               AVVIA SOTTOSCRIZIONI ai messaggi per servizi RPC ecc...
            // ********************************************************************************
            case CAN_STATE_SETUP:

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
                state = CAN_STATE_CHECK;
                break;

            // ********************************************************************************
            //         AVVIA LOOP CANARD PRINCIPALE gestione TX/RX Code -> Messaggi
            // ********************************************************************************
            case CAN_STATE_CHECK:

                // Set Canard microsecond corrente monotonic, per le attività temporanee di ciclo
                clCanard.getMicros(clCanard.start_syncronization);

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
                            // Set system_status (Set Node Offline)
                            param.systemStatusLock->Take();
                            param.system_status->data_slave[queueId].is_online = false;
                            param.systemStatusLock->Give();
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
                        TRACE_ERROR_F(F("Time OUT File... event occurs\r\n"));
                        // Gestione Retry previste dal comando per poi abbandonare
                        uint8_t retry; // In overload x LOGGING
                        if(clCanard.master.file.next_retry(&retry)) {
                            TRACE_VERBOSE_F(F("Next Retry File read: %u\r\n"), retry);
                        } else {
                            TRACE_VERBOSE_F(F("MAX Retry File occurs, download file ABORT !!!\r\n"));
                            clCanard.master.file.download_end();
                        }
                    }
                    // Se messaggio in pending non faccio niente è attendo la fine del comando in run
                    // In caso di errore subentrerà il TimeOut e verrà essere gestita la retry
                    if(!clCanard.master.file.is_pending()) {
                        // Fine pending, con messaggio OK. Verifico se EOF o necessario altro blocco
                        if(clCanard.master.file.is_download_complete()) {
                            if(clCanard.master.file.is_firmware()) {
                                TRACE_INFO_F(F("RX FIRMWARE COMPLETED !!!\r\n"));
                            } else {
                                TRACE_INFO_F(F("RX FILE COMPLETED !!!\r\n"));
                            }
                            TRACE_VERBOSE_F(F("File name: %s\r\n"), clCanard.master.file.get_name());
                            // GetInfo && Verify Start Updating...
                            if(clCanard.master.file.is_firmware()) {
                                // Module type also checked before startin firmware_upgrade
                                uint8_t module_type;
                                uint8_t version;
                                uint8_t revision;
                                uint64_t fwFileLen = 0;
                                getFlashFwInfoFile(&module_type, &version, &revision, &fwFileLen);
                                TRACE_VERBOSE_F(F("Firmware V%d.%d, Size: %lu bytes is ready for flash updating\r\n"),version, revision, (uint32_t) fwFileLen);
                            }
                            // Nessun altro evento necessario, chiudo File e stati
                            // procedo all'aggiornamento Firmware dopo le verifiche di conformità
                            // Ovviamente se si tratta di un file firmware
                            clCanard.master.file.download_end();
                            // Comunico a HeartBeat (Yakut o Altri) l'avvio dell'aggiornamento (se il file è un firmware...)
                            // Per Yakut Pubblicare un HeartBeat prima dell'Avvio quindi con il flag
                            // clCanard.local_node.file.updating_run = true >> HeartBeat Comunica Upgrade...
                            if(clCanard.master.file.is_firmware()) {
                                clCanard.flag.set_local_node_mode(uavcan_node_Mode_1_0_SOFTWARE_UPDATE);
                                start_firmware_upgrade = true;
                                // Preparo la struttua per informare il Boot Loader
                                if(start_firmware_upgrade) {
                                    bootloader_t boot_request;
                                    boot_request.app_executed_ok = false;
                                    boot_request.backup_executed = false;
                                    boot_request.rollback_executed = false;
                                    boot_request.request_upload = true;
                                    param.eeprom->Write(BOOT_LOADER_STRUCT_ADDR, (uint8_t*) &boot_request, sizeof(boot_request));
                                }
                            }
                            // Il Firmware Upload dovrà partire necessariamente almeno dopo l'invio completo
                            // di HeartBeat (svuotamento coda), quindi via subito in heart_beat send
                            // Counque non rispondo più ai comandi di update con file.updating_run = true
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
                if((start_firmware_upgrade)||
                    (clCanard.getMicros(clCanard.syncronized_time) >= last_pub_heartbeat)) {
                    TRACE_INFO_F(F("Publish MASTER Heartbeat -->> [ %u sec]\r\n"), TIME_PUBLISH_HEARTBEAT);
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
                    TRACE_INFO_F(F("Publish Local PORT LIST -->> [ %u sec]\r\n"), TIME_PUBLISH_PORT_LIST);
                    last_pub_port_list += MEGA * TIME_PUBLISH_PORT_LIST;
                    // Update publisher
                    clCanard.master_servicelist_send_message();
                }

                // *******************************************************************************************
                // ********************* GET SYNCRO ACTIVITY FOR CYPAL FUNCTION ******************************
                // *******************************************************************************************
                uint32_t curEpoch = rtc.getEpoch();
                bool bStartGetIstant = false;
                bool bStartGetData = false;

                // need do acquire istant value for display?
                // Read data only if Display or other Task need to show/get this value
                if(1) {
//////////////                if(param.system_status->flags.display_on) {
                    if ((curEpoch / param.configuration->observation_s) != param.system_status->datetime.ptr_time_for_sensors_get_istant) {
                        param.systemStatusLock->Take();
                        uint16_t obs_istant;
                        // Get minim acquire for get request to remote data for Display
                        obs_istant = param.configuration->observation_s < MIN_ACQUIRE_GET_ISTANT_VALUE_SEC ? MIN_ACQUIRE_GET_ISTANT_VALUE_SEC : param.configuration->observation_s;
                        param.system_status->datetime.ptr_time_for_sensors_get_istant = curEpoch / obs_istant;
                        param.system_status->datetime.epoch_sensors_get_istant = (curEpoch / obs_istant) * obs_istant;
                        param.systemStatusLock->Give();
                        bStartGetIstant = true;
                    }
                }
                // need do acquire data value for RMAP Archive?
                if ((curEpoch / param.configuration->report_s) != param.system_status->datetime.ptr_time_for_sensors_get_value) {      
                    param.systemStatusLock->Take();
                    param.system_status->datetime.ptr_time_for_sensors_get_value = curEpoch / param.configuration->report_s;
                    param.system_status->datetime.epoch_sensors_get_istant = (curEpoch / param.configuration->report_s) * param.configuration->report_s;
                    param.systemStatusLock->Give();
                    bStartGetData = true;
                }
                // ********************* END SYNCRO ACTIVITY FOR CYPAL FUNCTION ******************************

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

                // ***********************************************************************
                // ********************** RMAP GETDATA TX-> RX<- *************************
                // ***********************************************************************

                // START COMMAND REQUEST
                // Get Istant Data or Archive Data Request (Need to Display, Saving Data or other Function with Istant/Archive Data)
                if ((bStartGetIstant)||(bStartGetData)) {
                    // For all node
                    for(uint8_t queueId=0; queueId<MAX_NODE_CONNECT; queueId++) {
                        // For all node onLine
                        if(clCanard.slave[queueId].is_online()) {
                            // Comman are sending without other Pending Command
                            // Service request structure is same for all RMAP object to simplify function
                            if(!clCanard.slave[queueId].rmap_service.is_pending()) {
                                TRACE_INFO_F(F("Inviato richiesta dati al nodo remoto: %d\r\n"), clCanard.slave[queueId].get_node_id());
                                // parametri.canale = rmap_service_setmode_1_0_CH01 (es-> set CH Analogico...)
                                // parametri.run_for_second = 900; ( not used for get_istant )
                                rmap_service_setmode_1_0 paramRequest;
                                paramRequest.chanel = 0; // Request only for chanel analog/digital adressed 1..X
                                if(bStartGetData) {
                                    paramRequest.command = rmap_service_setmode_1_0_get_istant;
                                    paramRequest.obs_sectime = param.configuration->observation_s;
                                    paramRequest.run_sectime = param.configuration->report_s;
                                } else {
                                    paramRequest.command = rmap_service_setmode_1_0_get_last;
                                    paramRequest.obs_sectime = 0;
                                    paramRequest.run_sectime = 0;
                                }
                                // Imposta il pending del comando per verifica sequenza TX-RX e il TimeOut
                                // La risposta al comando è già nel blocco dati, non necessaria ulteriore variabile
                                clCanard.send_rmap_data_pending(queueId, NODE_GETDATA_TIMEOUT_US, paramRequest);
                            }
                        }
                    }
                }
                // EVENT GESTION OF TIMEOUT AT REQUEST
                for(uint8_t queueId=0; queueId<MAX_NODE_CONNECT; queueId++) {
                    if (clCanard.slave[queueId].rmap_service.event_timeout()) {
                        // Adesso elimino solo gli stati per corretta visualizzazione a Serial.println
                        clCanard.slave[queueId].rmap_service.reset_pending();
                        // TimeOUT di un comando in attesa... gestire Retry, altri segnali al Server ecc...
                        TRACE_ERROR_F(F("Timeout risposta su richiesta dati al nodo remoto: %d, Warning [restore pending command]\r\n"),
                            clCanard.slave[queueId].get_node_id());
                    }
                }
                // EVENT GESTION OF RECIVED DATA AT REQUEST
                for(uint8_t queueId=0; queueId<MAX_NODE_CONNECT; queueId++) {
                    if(clCanard.slave[queueId].rmap_service.is_executed()) {
                        // Adesso elimino solo gli stati per corretta visualizzazione a Serial.println
                        clCanard.slave[queueId].rmap_service.reset_pending();
                        // Interprete del messaggio in casting dal puntatore dinamico
                        // Nell'esempio Il modulo e TH, naturalmente bisogna gestire il tipo
                        // in funzione di clCanard.slave[x].node_type
                        switch (clCanard.slave[queueId].get_module_type()) {
                            case canardClass::Module_Type::th:
                                // Cast to th module
                                rmap_service_module_TH_Response_1_0* retData;
                                retData = (rmap_service_module_TH_Response_1_0*) clCanard.slave[queueId].rmap_service.get_response();
                                // data RMAP type is ready to send into queue Archive Data for Saving on MMC Memory
                                // Get parameter data
                                #if TRACE_LEVEL >= TRACE_INFO
                                char stimaName[STIMA_MODULE_NAME_LENGTH];
                                getStimaNameByType(stimaName, clCanard.slave[queueId].get_module_type());
                                #endif
                                // Put data in system_status with module_type and RMAP Ver.Rev at first access (if not readed)
                                if(!param.system_status->data_slave[queueId].module_version) {
                                    param.systemStatusLock->Take();
                                    param.system_status->data_slave[queueId].module_version = retData->version;
                                    param.system_status->data_slave[queueId].module_revision = retData->revision;
                                    // Module type also setted on load config module CAN
                                    // param.system_status->data_slave[queueId].module_type = clCanard.slave[queueId].get_module_type();
                                    // Check if module can be updated
                                    for(uint8_t checkId=0; checkId<STIMA_MODULE_TYPE_MAX_AVAIABLE; checkId++) {
                                        if(clCanard.slave[queueId].get_module_type() == param.system_status->boards_update_avaiable[checkId].module_type) {
                                            if((param.system_status->boards_update_avaiable[checkId].version > retData->version) ||
                                                ((param.system_status->boards_update_avaiable[checkId].version == retData->version) && 
                                                 (param.system_status->boards_update_avaiable[checkId].revision > retData->revision))) {
                                                // Found an upgradable boards
                                                param.system_status->data_slave[queueId].fw_upgrade = true;
                                            }
                                            break;
                                        }
                                    }
                                    param.systemStatusLock->Give();
                                }
                                // TRACE Info data
                                TRACE_INFO_F(F("RMAP recived response data module from [ %s ], node id: %d. Response code: %d\r\n"),
                                    stimaName, clCanard.slave[queueId].get_node_id(), retData->state);
                                TRACE_VERBOSE_F(F("Value (STH) TP %d, UH: %d\r\n"), retData->STH.temperature.val.value, retData->STH.humidity.val.value);
                                // Put data in system_status
                                param.systemStatusLock->Take();
                                // Set data istant value
                                param.system_status->data_slave[queueId].data_value_A = retData->STH.temperature.val.value;
                                param.system_status->data_slave[queueId].data_value_B = retData->STH.humidity.val.value;
                                // Add info RMAP to system
                                if(bStartGetIstant)
                                    param.system_status->data_slave[queueId].last_acquire = param.system_status->datetime.epoch_sensors_get_istant;
                                else
                                    param.system_status->data_slave[queueId].last_acquire = param.system_status->datetime.epoch_sensors_get_value;
                                param.systemStatusLock->Give();
                                // Set data into queue if data value (not for istant observation acquire)
                                if(bStartGetData) {
                                    memset(dataQueue, 0, sizeof(dataQueue));
                                    // Set Module Type, Date Time as Uint32 GetEpoch_Style, and Block Data Cast to RMAP Type
                                    dataQueue[0] = clCanard.slave[queueId].get_module_type();
                                    memcpy(&dataQueue[1], &param.system_status->datetime.epoch_sensors_get_value,
                                        sizeof(param.system_status->datetime.epoch_sensors_get_value));
                                    memcpy((void*)dataQueue[5], retData, sizeof(retData));
                                    // Send queue to MMC/SD for direct archive data
                                    // Queue is dimensioned to accept all Data for one step pushing array data (MAX_BOARDS)
                                    param.dataRmapPutQueue->Enqueue(&dataQueue, CAN_PUT_QUEUE_RMAP_TIMEOUT_MS);
                                }
                                break;

                            default:
                                // data RMAP type is ready to send into queue Archive Data for Saving on MMC Memory
                                TRACE_INFO_F(F("RMAP recived response data module from unknown module node id: %d. Response code: %d\r\n"),
                                    clCanard.slave[queueId].get_node_id(), retData->state);
                                break;
                        }
                    }
                }
                // ********************* END RMAP GETDATA TX-> RX<- **********************

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

                // ***********************************************************************
                // ********************** FILE SERVER (CAN UPLOADER) *********************
                // ***********************************************************************
                // Se nodo Online, procedo, se passo OffLine interrompo la procedura
                // Oppure posso gestire in un timeOut Limitato (L'OffLine è comunque già temporizzato...)
                // Invio comando e attendo risposta affermativa, poi passo all'aggiornamento
                // NB!!! Rispondo a Request solo se sono nello stato coerente di gestione file upload
                // Questo comporta la sicurezza che il nodo non abbia avuto problematiche di genere
                // Sia come server che client. Se non sono in coerenza e non rispondo, lo slave andrà
                // in TimeOut e la procedura non finisce (senza causare quindi irregolari trasferimenti)
                // Al termine del trasferimento, esco dalla modalità >> FILE_STATE_WAIT_COMMAND <<
                // direttamente nell' invio dell' ultimo pacchetto dati. E mi preparo a nuovo invio...
                bool file_server_running = true;
                uint8_t file_server_queueId = 0xFF;
                // Request to update (file_server_running?) on valid node
                if((file_server_running)&&(file_server_queueId<BOARDS_COUNT_MAX)) {

                    // Se vado OffLine la procedura comunque viene interrotta dall'evento di OffLine
                    switch(clCanard.slave[file_server_queueId].file_server.get_state()) {

                        default: // -->> FILE_STATE_ERROR STATE:
                            clCanard.slave[file_server_queueId].file_server.end_transmission();
                            break;

                        case canardClass::FileServer_State::standby:
                            // Starting request remote from any system
                            char file_name[CAN_FILE_NAME_SIZE_MAX];
                            // Set correct name of file from last avaiable (by module)
                            setStimaFirmwareName(file_name,
                                param.system_status->boards_update_avaiable[file_server_queueId].module_type,
                                param.system_status->boards_update_avaiable[file_server_queueId].version,
                                param.system_status->boards_update_avaiable[file_server_queueId].revision);
                            // Start upload file with module_type last version found on SD Card
                            clCanard.slave[file_server_queueId].file_server.set_file_name(file_name, true);
                            break;

                        case canardClass::FileServer_State::begin_update:
                            // Avvio comando di aggiornamento, controllo coerenza ed esistenza file già effettuato
                            // Se non ci sono altre problematiche (aggiornamenti vari, download blocchi ecc.. avvio...)
                            // Avvio il comando nel nodo remoto
                            clCanard.slave[file_server_queueId].file_server.next_state();
                            // else
                                // Annullo la richiesta (Invio Error al Remoto su Request File)
                                // O smetto di rispondere al servizio per quella richiesta (TimeOut)
                                // clCanard.slave[queueId].file_server.end_transmission();
                            break;

                        case canardClass::FileServer_State::command_send:
                            // Invio comando di aggiornamento File (in attesa in coda con switch...)
                            // Il comando viene inviato solamente senza altri Pending di Comandi (come semaforo)
                            if(!clCanard.slave[file_server_queueId].command.is_pending()) {
                                // Imposta il pending del comando per verifica sequenza TX-RX e il TimeOut
                                // Imposta la risposta del comadno A UNDEFINED (verrà settato al valore corretto in risposta)
                                // Il comando comunica la presenza del file di download nel remoto e avvia dowload
                                clCanard.send_command_file_server_pending(file_server_queueId, NODE_COMMAND_TIMEOUT_US);
                                clCanard.slave[file_server_queueId].file_server.next_state();
                            }
                            break;

                        case canardClass::FileServer_State::command_wait:
                            // Attendo la risposta del Nodo Remoto conferma, errore o TimeOut
                            if(clCanard.slave[file_server_queueId].command.event_timeout()) {
                                // Counico al server l'errore di timeOut Command Update Start ed esco
                                clCanard.slave[file_server_queueId].file_server.end_transmission();
                                TRACE_VERBOSE_F(F("Node [ %d ], TimeOut Command Start Send file, uploading %s\r\n"),
                                    clCanard.slave[file_server_queueId].get_node_id(), ABORT_STRING);
                                // Se decido di uscire nella procedura di OffLine, la comunicazione
                                // al server di eventuali errori deve essere gestita al momento dell'OffLine
                            }
                            // Comando eseguito,verifico la correttezza della risposta
                            if(clCanard.slave[file_server_queueId].command.is_executed()) {
                                // La risposta si trova in command_response fon flag pending azzerrato.
                                if(clCanard.slave[file_server_queueId].command.get_response() == 
                                    uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS) {
                                    // Sequenza terminata, avvio il file transfer !!!
                                    // Essendo una procedura server e non avendo nessun controllo sui comandi
                                    // La classe si limita a fornire gli elementi di controllo timeOut per
                                    // la verifica di coerenza e gestione del download dal remoto ed internamente
                                    // gestisco le fasi dell'upload. Non è richiesta nessun altra gestione e per
                                    // comodità gestisco l'aggiornamento del timeOut direttamente nella callBack
                                    // nella sezione request del file dallo slave.
                                    clCanard.slave[file_server_queueId].file_server.next_state();
                                    // Stampo lo stato
                                    TRACE_VERBOSE_F(F("Node [ %d ] upload Start Send file %s\r\n"),
                                        clCanard.slave[file_server_queueId].get_node_id(), OK_STRING);
                                    // Imposto il timeOUT per controllo Deadline con pending per sequenza di download
                                    clCanard.slave[file_server_queueId].file_server.start_pending(NODE_REQFILE_TIMEOUT_US);
                                } else {
                                    // Errore comando eseguito ma risposta non valida. Annullo il trasferimento
                                    clCanard.slave[file_server_queueId].file_server.end_transmission();
                                    // ...counico al server (RMAP remoto) l'errore per il mancato aggiornamento ed esco
                                    TRACE_VERBOSE_F(F("Node [ %d ] response Cmd Error in Send file %s\r\n"),
                                        clCanard.slave[file_server_queueId].get_node_id(), ABORT_STRING);
                                }
                            }
                            break;

                        case canardClass::FileServer_State::state_uploading:
                            // Attendo la risposta del Nodo Remoto conferma, errore o TimeOut
                            if(clCanard.slave[file_server_queueId].file_server.event_timeout()) {
                                // Counico al server l'errore di timeOut Command Update Start ed esco
                                clCanard.slave[file_server_queueId].file_server.end_transmission();
                                TRACE_VERBOSE_F(F("Node [ %d ] TimeOut Request/Response send file %s\r\n"),
                                    clCanard.slave[file_server_queueId].get_node_id(), ABORT_STRING);
                                // Se decido di uscire nella procedura di OffLine, la comunicazione
                                // al server di eventuali errori deve essere gestita al momento dell'OffLine
                            }
                            // Evento completato (file_server) terminato con successo, vado a step finale
                            if(clCanard.slave[file_server_queueId].file_server.is_executed()) {
                                clCanard.slave[file_server_queueId].file_server.next_state();
                            }
                            break;

                        case canardClass::FileServer_State::upload_complete:
                            // Counico al server file upload Complete ed esco (nuova procedura ready)
                            // -> EXIT FROM FILE_STATE_STANDBY ( In procedura di SendFileBlock )
                            // Quando invio l'ultimo pacchetto dati valido ( Blocco < 256 Bytes )
                            clCanard.slave[file_server_queueId].file_server.end_transmission();
                            TRACE_VERBOSE_F(F("Node [ %d ] sending completed %s\r\n"),
                                clCanard.slave[file_server_queueId].get_node_id(), OK_STRING);
                            break;
                    }
                }
                // ********************* END FILE SERVER CAN UPLOADER ********************

                // ***************************************************************************
                //   Gestione Coda messaggi in trasmissione (ciclo di svuotamento messaggi)
                // ***************************************************************************
                // Transmit pending frames, avvia la trasmissione gestita da canard a priorità.
                // Il metodo può essere chiamato direttamente in preparazione messaggio x coda
                if (clCanard.transmitQueueDataPresent()) {
                    // Access driver con semaforo
                    if(param.canLock->Take(Ticks::MsToTicks(CAN_SEMAPHORE_MAX_WAITING_TIME_MS))) {
                        clCanard.transmitQueue();
                        param.canLock->Give();
                    }
                }

                // ***************************************************************************
                //   Gestione Coda messaggi in ricezione (ciclo di caricamento messaggi)
                // ***************************************************************************
                // Gestione con Intererupt RX Only esterna (verifica dati in coda gestionale)
                if (clCanard.receiveQueueDataPresent()) {
                    // Log Packet
                    #ifdef LOG_RX_PACKET
                    char logMsg[50];
                    clCanard.receiveQueue(logMsg);
                    TRACE_DEBUG_F(F("RX [ %s ]\r\n"), logMsg);
                    #else
                    clCanard.receiveQueue();
                    #endif
                }

                // Request Reboot (Firmware upgrade... Or Reset)
                if (clCanard.flag.is_requested_system_restart() || (start_firmware_upgrade)) {
                    TRACE_INFO_F(F("Send signal to system Reset...\r\n"));
                    delay(250); // Waiting security queue empty send HB (Updating start...)
                    NVIC_SystemReset();                    
                }
                break;
        }

        #if (ENABLE_STACK_USAGE)
        TaskMonitorStack();
        #endif

        // Local TaskWatchDog update;
        TaskWatchDog(CAN_TASK_WAIT_DELAY_MS);

        // Run switch TASK CAN one STEP every...
        // If File Uploading MIN TimeOut For Task for Increse Speed Transfer RATE
        if(clCanard.master.file.download_request()) {            
            DelayUntil(Ticks::MsToTicks(CAN_TASK_WAIT_REALTIME_DELAY_MS));
        }
        else
        {
            DelayUntil(Ticks::MsToTicks(CAN_TASK_WAIT_DELAY_MS));
        }
    }
}

#endif