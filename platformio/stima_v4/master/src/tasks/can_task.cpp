/**
  ******************************************************************************
  * @file    can_task.cpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   Uavcan over CanBus cpp_Freertos source file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Stimav4 is Copyright (C) 2023 ARPAE-SIMC urpsim@arpae.it</center></h2>
  * <h2><center>All rights reserved.</center></h2>
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
    localRegister->read(REGISTER_UAVCAN_UNIQUE_ID, &val);
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
            TRACE_INFO_F(F("FLASH: Erase block: %d\r\n"), canFlashBlock);
            if (localFlash->BSP_QSPI_Erase_Block(canFlashBlock)) {
                localQspiLock->Give();
                return false;
            }
            // Write Name File (Size at Eof...)
            uint8_t file_flash_name[FLASH_FILE_SIZE_LEN] = {0};
            memcpy(file_flash_name, file_name, strlen(file_name));
            localFlash->BSP_QSPI_Write(file_flash_name, canFlashPtr, FLASH_FILE_SIZE_LEN);
            // Write into Flash
            TRACE_INFO_F(F("FLASH: Write [ %d ] bytes at addr: %d\r\n"), FLASH_FILE_SIZE_LEN, canFlashPtr);
            #ifdef CHECK_FLASH_WRITE
            localFlash->BSP_QSPI_Read(check_data, canFlashPtr, FLASH_FILE_SIZE_LEN);
            if(memcmp(file_flash_name, check_data, FLASH_FILE_SIZE_LEN)==0) {
                TRACE_INFO_F(F("FLASH: Reading check OK\r\n"));
            } else {
                TRACE_ERROR_F(F("FLASH: Reading check ERROR\r\n"));
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
            TRACE_INFO_F(F("FLASH: Write [ %d ] bytes at addr: %d\r\n"), count, canFlashPtr);
            // Starting Write at OFFSET Required... Erase here is Done
            localFlash->BSP_QSPI_Write((uint8_t*)buf, canFlashPtr, count);
            #ifdef CHECK_FLASH_WRITE
            localFlash->BSP_QSPI_Read(check_data, canFlashPtr, count);
            if(memcmp(buf, check_data, count)==0) {
                TRACE_INFO_F(F("FLASH: Reading check OK\r\n"));
            } else {
                TRACE_ERROR_F(F("FLASH: Reading check ERROR\r\n"));
                localQspiLock->Give();
                return false;
            }
            #endif
            canFlashPtr += count;
            // Check if Next Page Addressed (For Erase Next Block)
            if((canFlashPtr / AT25SF641_BLOCK_SIZE) != canFlashBlock) {
                canFlashBlock = canFlashPtr / AT25SF641_BLOCK_SIZE;
                // Erase First Block Block (Block OF 4KBytes)
                TRACE_INFO_F(F("FLASH: Erase block: %d\r\n"), canFlashBlock);
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
            TRACE_INFO_F(F("FLASH: Write [ %d ] bytes at addr: %d\r\n"), FLASH_INFO_SIZE_U64, canFlashPtr);
            #ifdef CHECK_FLASH_WRITE
            localFlash->BSP_QSPI_Read(check_data, FLASH_SIZE_ADDR(canFlashPtr), FLASH_INFO_SIZE_U64);
            if(memcmp(&lenghtFile, check_data, FLASH_INFO_SIZE_U64)==0) {
                TRACE_INFO_F(F("FLASH: Reading check OK\r\n"));
            } else {
                TRACE_INFO_F(F("FLASH: Reading check ERROR\r\n"));
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

/// @brief Chiamate gestioni RPC remote da master (yakut o altro servizio di controllo)
/// @param clCanard canard class 
/// @param req uavcan node execute command request
/// @param remote_node node of remote server
/// @return uavcan_node_ExecuteCommand_Response_1_1 
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
            // Istant Reboot for next Register base Setup
            NVIC_SystemReset();
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
        // Local CAN Transport to RPC Call
        case canardClass::Command_Private::set_full_power:
        {
            // Abilita modalità full power (per manutenzione e/o test)
            localSystemStatusLock->Take();
            localSystemStatus->flags.full_wakeup_forced = true;
            localSystemStatusLock->Give();
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        case canardClass::Command_Private::set_nominal_power:
        {
            // Disabilita modalità full power (per manutenzione e/o test)
            localSystemStatusLock->Take();
            localSystemStatus->flags.full_wakeup_forced = false;
            localSystemStatusLock->Give();
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        case canardClass::Command_Private::execute_rpc:
        {
            // Security lock task_flag for External Local TASK RPC (Need for risk of WDT Reset)
            // Return to previous state on END of RPC Call execution
            bool is_event_rpc = true;
            localStreamRpc->init();

            // Put RPC for configuration mode
            if (localRpcLock->Take(Ticks::MsToTicks(RPC_WAIT_DELAY_MS)))
            {
                while (is_event_rpc)
                {
                    // Security lock task_flag for External Local TASK RPC (Need for risk of WDT Reset)
                    localSystemStatus->tasks[LOCAL_TASK_ID].state = task_flag::suspended;
                    localStreamRpc->parseCharpointer(&is_event_rpc, (char *)req->parameter.elements, req->parameter.count, NULL, 0, RPC_TYPE_HTTPS);
                    localSystemStatus->tasks[LOCAL_TASK_ID].state = task_flag::normal;
                    localSystemStatus->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::set;
                }
                localRpcLock->Give();
                if(!is_event_rpc)
                    resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
                else
                    resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_BAD_COMMAND;
                break;
            } else {
                resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_BAD_COMMAND;
            }
            break;
        }
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

/// @brief Accesso ai registri UAVCAN risposta a richieste
/// @param req uavcan register access request
/// @return uavcan_register_Access_Response_1_0 
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

///@brief Processing messages and commands received
///@param clCanard class of Canard
///@param transfer Reassembled incoming transfer
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
                // Segnalo a LCD PnP request da un nodo della rete UAVCAN
                localSystemStatusLock->Take();
                localSystemStatus->flags.pnp_request = msg.unique_id_hash & 0xFF;
                localSystemStatusLock->Give();
                // Cerco nei moduli conosciuti (in HASH_UNIQUE_ID) invio il tipo modulo...
                // Verifico se ho un nodo ancora da configurare come da cfg del master
                // Il nodo deve essere compatibile con il tipo di modulo previsto da allocare
                TRACE_VERBOSE_F(F("RX PNP Allocation message request from -> "));
                switch(msg.unique_id_hash & 0xFF) {
                    case Module_Type::th:
                        TRACE_VERBOSE_F(F("Anonimous module TH"));
                        defaultNodeId = clCanard.getPNPValidIdFromNodeType(Module_Type::th, msg.unique_id_hash);
                        break;
                    case Module_Type::rain:
                        TRACE_VERBOSE_F(F("Anonimous module RAIN"));
                        defaultNodeId = clCanard.getPNPValidIdFromNodeType(Module_Type::rain, msg.unique_id_hash);
                        break;
                    case Module_Type::wind:
                        TRACE_VERBOSE_F(F("Anonimous module WIND"));
                        defaultNodeId = clCanard.getPNPValidIdFromNodeType(Module_Type::wind, msg.unique_id_hash);
                        break;
                    case Module_Type::radiation:
                        TRACE_VERBOSE_F(F("Anonimous module RADIATION"));
                        defaultNodeId = clCanard.getPNPValidIdFromNodeType(Module_Type::radiation, msg.unique_id_hash);
                        break;
                    case Module_Type::leaf:
                        TRACE_VERBOSE_F(F("Anonimous module LEAF"));
                        defaultNodeId = clCanard.getPNPValidIdFromNodeType(Module_Type::leaf, msg.unique_id_hash);
                        break;
                    case Module_Type::level:
                        TRACE_VERBOSE_F(F("Anonimous module LEVEL"));
                        defaultNodeId = clCanard.getPNPValidIdFromNodeType(Module_Type::level, msg.unique_id_hash);
                        break;
                    case Module_Type::vwc:
                        TRACE_VERBOSE_F(F("Anonimous module VWC"));
                        defaultNodeId = clCanard.getPNPValidIdFromNodeType(Module_Type::vwc, msg.unique_id_hash);
                        break;
                    case Module_Type::power:
                        TRACE_VERBOSE_F(F("Anonimous module POWER"));
                        defaultNodeId = clCanard.getPNPValidIdFromNodeType(Module_Type::power, msg.unique_id_hash);
                        break;
                    default:
                        // PNP Non gestibile
                        TRACE_VERBOSE_F(F("Anonimous module Unknown"));
                        defaultNodeId = GENERIC_BVAL_UNDEFINED;
                        break;
                }
                TRACE_VERBOSE_F(F(" [Serial Number HASH ID: %lu]\r\n"), (uint32_t) msg.unique_id_hash);
                // Risposta immediata diretta (Se nodo ovviamente è riconosciuto...)
                // Non utilizziamo una Response in quanto l'allocation è sempre un messaggio anonimo
                // I metadati del trasporto sono come quelli riceuti del transferID quindi è un messaggio
                // che si comporta parzialmente come una risposta (per rilevamento remoto hash/transfer_id)
                if(defaultNodeId == GENERIC_BVAL_UNCOERENT) {
                    TRACE_VERBOSE_F(F("PNP Allocation incoerent with Master configuration for reques [ %s ]\r\n"), ABORT_STRING);
                }
                else if(defaultNodeId <= CANARD_NODE_ID_MAX) {
                    TRACE_VERBOSE_F(F("Try PNP Allocation with Node_ID -> %d\r\n"), defaultNodeId);
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
                    // Accodo i dati letti dal messaggio (Nodo -> OnLine) verso la classe
                    clCanard.slave[queueId].heartbeat.set_online(NODE_OFFLINE_TIMEOUT_US,
                        msg.vendor_specific_status_code, msg.health.value, msg.mode.value, msg.uptime);
                    localSystemStatusLock->Take();
                    // Controllo sequenza heartbeat con il rolling a 0
                    if((++localSystemStatus->data_slave[queueId].heartbeat_transf_id != transfer->metadata.transfer_id) && (transfer->metadata.transfer_id)) {
                        localSystemStatus->data_slave[queueId].heartbeat_rx_err++;
                    }
                    localSystemStatus->data_slave[queueId].heartbeat_rx++;
                    localSystemStatus->data_slave[queueId].heartbeat_transf_id = transfer->metadata.transfer_id;                        
                    localSystemStatusLock->Give();
                    // Controlla se il modulo è ready (configurato) Altrimenti avvio la configurazione...
                    if(!clCanard.slave[queueId].heartbeat.get_module_ready()) {
                        TRACE_VERBOSE_F(F("Module slave [ %u ] is not (ready) configured\r\n"), transfer->metadata.remote_node_id);
                        localSystemStatusLock->Take();
                        localSystemStatus->flags.run_module_configure = true;
                        localSystemStatusLock->Give();
                    }
                    TRACE_VERBOSE_F(F("Power mode status node %u [ %s ]\r\n"), transfer->metadata.remote_node_id,
                        clCanard.slave[queueId].heartbeat.get_power_mode() == Power_Mode::pwr_on ? "full power" : "deep sleep");
                    // Rientro in OnLINE da OFFLine o Init Gestino può (dovrebbe) essere esterna alla Call
                    // Inizializzo le variabili e gli stati necessari per Reset e corretta gestione
                    if(clCanard.slave[queueId].is_entered_online()) {
                        TRACE_INFO_F(F("Node is now entered ONLINE !!!\r\n"));
                        // Metto i Flag in sicurezza, laddove dove non eventualmente gestito
                        clCanard.slave[queueId].command.reset_pending();
                        clCanard.slave[queueId].register_access.reset_pending();
                        clCanard.slave[queueId].file_server.reset_pending();
                        clCanard.slave[queueId].rmap_service.reset_pending();
                        // Set system_status from local to static access (Set Node Online)
                        localSystemStatusLock->Take();
                        localSystemStatus->data_slave[queueId].is_online = true;
                        localSystemStatus->datetime.ptr_time_for_sensors_get_istant = 0; // Force get istant data for display
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
                    if(clCanard.slave[queueId].get_module_type() == Module_Type::th) {
                        // Processato il messaggio con il relativo Handler. OK
                        bKindMessageProcessed = true;
                        // Modulo TH, leggo e deserializzo il messaggio in ingresso
                        rmap_module_TH_1_0 msg = {0};
                        size_t size = transfer->payload_size;
                        if (rmap_module_TH_1_0_deserialize_(&msg, static_cast<uint8_t const*>(transfer->payload), &size) >= 0)
                        {
                            // msg contiene i dati di blocco pubblicati
                            TRACE_VERBOSE_F(F("Ricevuto dato in publisher modulo_th -> ID: %d, transfer ID: %d\r\n"),
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
                    clCanard.sendResponse(CANARD_REGISTERLIST_TRANSFER_ID_TIMEOUT_USEC, &transfer->metadata, serialized_size, &serialized[0]);
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
                        TRACE_ERROR_F(F("File server: Reject request uploading file. SD CARD not ready [ %s ]\r\n"), ERROR_STRING);
                    } else {
                        // Is first block? Perform an request to open file with name into task sd card
                        if((req.offset / FILE_GET_DATA_BLOCK_SIZE) == 0) {
                            // First block NAME OF FILE (Prepare queue GET struct with file_name and start position)                            
                            memset(&firmwareDownloadChunck, 0, sizeof(file_get_request_t));
                            // Init name file (only first blcok for any board_id request)
                            firmwareDownloadChunck.file_name = clCanard.slave[queueId].file_server.get_file_name();                            
                            TRACE_INFO_F(F("File server: Starting upload file from local SD Card to remote module CAN [ %s ]\r\n"), firmwareDownloadChunck.file_name);
                        }
                        // Switch state of queue download file from sd card
                        // Method Reading with block request, disable read_next block in request
                        firmwareDownloadChunck.block_read_next = false;
                        firmwareDownloadChunck.block_id = req.offset / FILE_GET_DATA_BLOCK_SIZE;
                        firmwareDownloadChunck.board_id = queueId; // Set BOARDS Require File (Need for multiserver file request)
                        // Pushing data request to queue task sd card
                        localDataFileGetRequestQueue->Enqueue(&firmwareDownloadChunck);
                        // Waiting response from SD with TimeOUT
                        memset(&sdcard_task_response, 0, sizeof(file_get_response_t));
                        LocalTaskWatchDog(FILE_IO_DATA_QUEUE_TIMEOUT);
                        file_download_error = !localDataFileGetResponseQueue->Dequeue(&sdcard_task_response, FILE_IO_DATA_QUEUE_TIMEOUT);
                        file_download_error |= !sdcard_task_response.done_operation;
                    }

                    // Block data is avaiable? -> Send to remote node
                    if(file_download_error) {
                        // any error? (optional retry... from remote node. File server wait another request)
                        TRACE_ERROR_F(F("File server: Download data file from queue error!!!\r\n"));
                        resp._error.value = uavcan_file_Error_1_0_IO_ERROR;
                        dataLen = 0;
                    } else {
                        // Send block
                        if(sdcard_task_response.block_lenght!=FILE_GET_DATA_BLOCK_SIZE) {
                            // EOF Last Block (bytes read != FILE_GET_DATA_BLOCK_SIZE)
                            // No other response/request are to get
                            TRACE_INFO_F(F("File server: End of data block data, readed [ %lu ] bytes\r\n"), ((uint32_t)(firmwareDownloadChunck.block_id) * (uint32_t)(FILE_GET_DATA_BLOCK_SIZE)) + sdcard_task_response.block_lenght);
                        } else {
                            // Normal block, prepare for next block
                            TRACE_VERBOSE_F(F("File server: Readed data block id: [ %d ]\r\n"), firmwareDownloadChunck.block_id);
                        }
                    }
                    // *********** END GET Data File (Firmware) from SD CARD Queue ************
                    // Preparo la risposta corretta
                    resp.data.value.count = (size_t)sdcard_task_response.block_lenght;
                    memcpy(resp.data.value.elements, sdcard_task_response.block, resp.data.value.count);
                    uint8_t serialized[uavcan_file_Read_Response_1_1_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
                    size_t  serialized_size = sizeof(serialized);
                    if (uavcan_file_Read_Response_1_1_serialize_(&resp, &serialized[0], &serialized_size) >= 0) {
                        clCanard.slave[queueId].file_server.reset_pending(resp.data.value.count);
                        clCanard.sendResponse(CANARD_READFILE_TRANSFER_ID_TIMEOUT_USEC, &transfer->metadata, serialized_size, &serialized[0]);
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
                        // Segalo a LCD aggiornamento fw to flash in corso da CAN BUS
                        localSystemStatusLock->Take();
                        localSystemStatus->flags.fw_updating = true;
                        localSystemStatusLock->Give();
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
                    // Utilizzo un buffer di allocazione per RX Messaggio in Casting da postare sul meteodo Slave Canard
                    uint8_t castLocalBuffer[RMAP_DATA_MAX_ELEMENT_SIZE];
                    memset(castLocalBuffer, 0, sizeof(castLocalBuffer));
                    // Resetta il pending del comando del nodo verificato (size_mem preparato in avvio)
                    // Copia la risposta nella variabile di chiamata in state
                    // N.B. possibile gestire qua tutte le occorrenze per stima V4
                    if(clCanard.slave[queueId].get_module_type() == Module_Type::th) {
                        rmap_service_module_TH_Response_1_0 *respCastTH = (rmap_service_module_TH_Response_1_0 *)castLocalBuffer;
                        size_t size = transfer->payload_size;
                        if (rmap_service_module_TH_Response_1_0_deserialize_(respCastTH, static_cast<uint8_t const*>(transfer->payload), &size) >= 0)
                        {
                            clCanard.slave[queueId].rmap_service.reset_pending(respCastTH, sizeof(*respCastTH));
                        }
                    }
                    if(clCanard.slave[queueId].get_module_type() == Module_Type::rain) {
                        rmap_service_module_Rain_Response_1_0 *respCastRain = (rmap_service_module_Rain_Response_1_0 *)castLocalBuffer;
                        size_t size = transfer->payload_size;
                        if (rmap_service_module_Rain_Response_1_0_deserialize_(respCastRain, static_cast<uint8_t const*>(transfer->payload), &size) >= 0)
                        {
                            clCanard.slave[queueId].rmap_service.reset_pending(respCastRain, sizeof(*respCastRain));
                        }
                    }
                    if(clCanard.slave[queueId].get_module_type() == Module_Type::wind) {
                        rmap_service_module_Wind_Response_1_0 *respCastWind = (rmap_service_module_Wind_Response_1_0 *)castLocalBuffer;
                        size_t size = transfer->payload_size;
                        if (rmap_service_module_Wind_Response_1_0_deserialize_(respCastWind, static_cast<uint8_t const*>(transfer->payload), &size) >= 0)
                        {
                            clCanard.slave[queueId].rmap_service.reset_pending(respCastWind, sizeof(*respCastWind));
                        }
                    }
                    if(clCanard.slave[queueId].get_module_type() == Module_Type::radiation) {
                        rmap_service_module_Radiation_Response_1_0 *respCastRadiation = (rmap_service_module_Radiation_Response_1_0 *)castLocalBuffer;
                        size_t size = transfer->payload_size;
                        if (rmap_service_module_Radiation_Response_1_0_deserialize_(respCastRadiation, static_cast<uint8_t const*>(transfer->payload), &size) >= 0)
                        {
                            clCanard.slave[queueId].rmap_service.reset_pending(respCastRadiation, sizeof(*respCastRadiation));
                        }
                    }
                    if(clCanard.slave[queueId].get_module_type() == Module_Type::leaf) {
                        rmap_service_module_Leaf_Response_1_0 *respCastLeaf = (rmap_service_module_Leaf_Response_1_0 *)castLocalBuffer;
                        size_t size = transfer->payload_size;
                        if (rmap_service_module_Leaf_Response_1_0_deserialize_(respCastLeaf, static_cast<uint8_t const*>(transfer->payload), &size) >= 0)
                        {
                            clCanard.slave[queueId].rmap_service.reset_pending(respCastLeaf, sizeof(*respCastLeaf));
                        }
                    }
                    if(clCanard.slave[queueId].get_module_type() == Module_Type::level) {
                        rmap_service_module_RiverLevel_Response_1_0 *respCastLevel = (rmap_service_module_RiverLevel_Response_1_0 *)castLocalBuffer;
                        size_t size = transfer->payload_size;
                        if (rmap_service_module_RiverLevel_Response_1_0_deserialize_(respCastLevel, static_cast<uint8_t const*>(transfer->payload), &size) >= 0)
                        {
                            clCanard.slave[queueId].rmap_service.reset_pending(respCastLevel, sizeof(*respCastLevel));
                        }
                    }
                    if(clCanard.slave[queueId].get_module_type() == Module_Type::vwc) {
                        rmap_service_module_VWC_Response_1_0 *respCastVWC = (rmap_service_module_VWC_Response_1_0 *)castLocalBuffer;
                        size_t size = transfer->payload_size;
                        if (rmap_service_module_VWC_Response_1_0_deserialize_(respCastVWC, static_cast<uint8_t const*>(transfer->payload), &size) >= 0)
                        {
                            clCanard.slave[queueId].rmap_service.reset_pending(respCastVWC, sizeof(*respCastVWC));
                        }
                    }
                    if(clCanard.slave[queueId].get_module_type() == Module_Type::power) {
                        rmap_service_module_Power_Response_1_0 *respCastPower = (rmap_service_module_Power_Response_1_0 *)castLocalBuffer;
                        size_t size = transfer->payload_size;
                        if (rmap_service_module_Power_Response_1_0_deserialize_(respCastPower, static_cast<uint8_t const*>(transfer->payload), &size) >= 0)
                        {
                            clCanard.slave[queueId].rmap_service.reset_pending(respCastPower, sizeof(*respCastPower));
                        }
                    }
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

/// @brief Construct the Can Task::CanTask object
/// @param taskName name of the task
/// @param stackSize size of the stack
/// @param priority priority of the task
/// @param canParam parameters for the task
CanTask::CanTask(const char *taskName, uint16_t stackSize, uint8_t priority, CanParam_t canParam) : Thread(taskName, stackSize, priority), param(canParam)
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
    localRpcLock = param.rpcLock;
    localStreamRpc = param.streamRpc;

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
    localRegister->read(REGISTER_UAVCAN_MTU, &val);
    localRegisterAccessLock->Give();
    LOCAL_ASSERT(uavcan_register_Value_1_0_is_natural16_(&val) && (val.natural16.value.count == 1));

    // *******************         CANARD SETUP TIMINGS AND SPEED        *******************
    // CAN BITRATE Dinamico su LoadRegister (CAN_FD 2xREG natural32 0=Speed, 1=0 (Not Used))
    uavcan_register_Value_1_0_select_natural32_(&val);
    val.natural32.value.count       = 2;
    val.natural32.value.elements[0] = CAN_BIT_RATE;
    val.natural32.value.elements[1] = 0ul;          // Ignored for CANARD_MTU_CAN_CLASSIC
    localRegisterAccessLock->Take();
    localRegister->read(REGISTER_UAVCAN_BITRATE, &val);
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
        localRegister->write(REGISTER_UAVCAN_BITRATE, &val);
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
  uint16_t stackUsage = (uint16_t)uxTaskGetStackHighWaterMark( NULL );
  if((stackUsage) && (stackUsage < param.system_status->tasks[LOCAL_TASK_ID].stack)) {
    param.systemStatusLock->Take();
    param.system_status->tasks[LOCAL_TASK_ID].stack = stackUsage;
    param.systemStatusLock->Give();
  }
}
#endif

/// @brief local watchDog and Sleep flag Task (optional)
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
     param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::set;
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

    // RMAP Queue data Put to memory SD Card
    rmap_archive_data_t rmap_archive_data;
    rmap_archive_data_t rmap_archive_empty;     // Set for clear queue if is full (used normally without SD_CARD queue in RAM)

    bool start_firmware_upgrade = false;        // Set when Firmware Upgrade is required
    bool is_running_update_system = false;      // Multi update firmware running proc from RPC
    bool is_running_update_send_cmd = false;    // Command queue started for one fw upgrading (Waiting end event for next)
    uint8_t index_running_update_boards;        // Index of board actual running in file server upload event

    // Starting acquire IST and value control var
    uint32_t getUpTimeSecondCurr;
    uint32_t curEpoch;
    bool bStartGetIstant = false;       // Get Istant value from node
    bool bStartupGetIstant = true;      // Get Istant value from node first time at startup
    bool bStartGetData = false;         // Get archive value from node
    bool bStartSetFullPower = false;    // Set remote node full power for get data
    // Get Istant value from node first time at startup
    uint8_t bStartupCount = CONFIGURATION_DEFAULT_UPDATE_IST_S + 1;
    // Register access register remote configuration array
    uint8_t remote_configure[MAX_NODE_CONNECT] = {0};
    bool remote_configure_reboot[MAX_NODE_CONNECT] = {0};
    uint32_t remote_configure_end_ms;   // Time wait after end configure one node (from 2 different Node Not Restart Config)
    uint8_t remote_configure_retry[MAX_NODE_CONNECT] = {0};
    // Configure waiting Online after set Parameter (remove request from queue for leak error if external request for configure)
    // If Node not get onLine in expected time, command removed from queue in security mode
    uint32_t remote_configure_wait_online_ms[MAX_NODE_CONNECT] = {0};

    // Response data PTR for type module direct data access
    rmap_service_module_TH_Response_1_0* retTHData;
    rmap_service_module_Rain_Response_1_0* retRainData;
    rmap_service_module_Wind_Response_1_0* retWindData;
    rmap_service_module_Radiation_Response_1_0* retRadiationData;
    rmap_service_module_Leaf_Response_1_0* retLeafData;
    rmap_service_module_RiverLevel_Response_1_0* retLevelData;
    rmap_service_module_VWC_Response_1_0* retVwcData;
    rmap_service_module_Power_Response_1_0* retPwrData;
    uint8_t bit8Flag; // Compose Error Status Flag for each Module Type
    // Trace of Module_type on Direct data Access
    #if TRACE_LEVEL >= TRACE_INFO
    char stimaName[STIMA_MODULE_NAME_LENGTH];
    #endif

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

            // ********* SYSTEM QUEUE MESSAGE ***********
            // enqueud system message from caller task
            if (!param.systemMessageQueue->IsEmpty()) {
                // Read queue in test mode
                if (param.systemMessageQueue->Peek(&system_message, 0))
                {
                    // Its request addressed into ALL TASK... -> no pull (only SUPERVISOR or exernal gestor)
                    if(system_message.task_dest == ALL_TASK_ID)
                    {
                        // Pull && elaborate command, 
                        if(system_message.command.do_sleep)
                        {
                            // Enter module sleep procedure (OFF Module)
                            HW_CAN_Power(CAN_ModePower::CAN_SLEEP);
                            
                            // Enter sleep module OK and update WDT
                            TaskWatchDog(CAN_TASK_SLEEP_DELAY_MS);
                            TaskState(state, UNUSED_SUB_POSITION, task_flag::sleepy);
                            Delay(Ticks::MsToTicks(CAN_TASK_SLEEP_DELAY_MS));
                            TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

                            // Restore module from Sleep
                            HW_CAN_Power(CAN_ModePower::CAN_NORMAL);
                        }
                    }
                }
            }

            // Init, waiting loading configuration (traceing message)
            case CAN_STATE_INIT:
                TRACE_INFO_F(F("Can task: Init, waiting loading configuration before START\r\n"));
                state = CAN_STATE_LOAD_CONFIG;

            // Setup Class waiting configuration and load and set CB and NodeId
            case CAN_STATE_LOAD_CONFIG:

                // Waiting configuration complete loaded before start application
                if (!param.system_status->configuration.is_loaded) break;

                TRACE_INFO_F(F("Can task: STARTING Configuration\r\n"));
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
                localRegister->read(REGISTER_UAVCAN_NODE_ID, &val);         // The names of the standard registers are regulated by the Specification.
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
                localRegister->read(REGISTER_UAVCAN_NODE_DESCR, &val);  // We don't need the value, we just need to ensure it exists.
                localRegisterAccessLock->Give();

                // ***********************************************************************************
                // Setup configuration module node and start canard class slave istance with loaded ID
                // ***********************************************************************************
                #if (FIXED_CONFIGURATION)
                // **** Used for Test module and check CAN data exchange ****
                // TH
                int8_t idxFixed;
                idxFixed = -1;
                #ifdef USE_MODULE_FIXED_TH
                idxFixed++;
                param.configuration->board_slave[idxFixed].can_address = 60;
                param.configuration->board_slave[idxFixed].module_type = Module_Type::th;
                param.configuration->board_slave[idxFixed].can_port_id = PORT_RMAP_TH;
                param.configuration->board_slave[idxFixed].can_publish_id = PORT_RMAP_TH;
                param.configuration->board_slave[idxFixed].serial_number = 0;
                for(uint8_t isCfg=0; isCfg<CAN_SENSOR_COUNT_MAX; isCfg++)
                    param.configuration->board_slave[idxFixed].is_configured[isCfg] = true;
                #endif
                #ifdef USE_MODULE_FIXED_RAIN
                idxFixed++;
                param.configuration->board_slave[idxFixed].can_address = 61;
                param.configuration->board_slave[idxFixed].module_type = Module_Type::rain;
                param.configuration->board_slave[idxFixed].can_port_id = PORT_RMAP_RAIN;
                param.configuration->board_slave[idxFixed].can_publish_id = PORT_RMAP_RAIN;
                param.configuration->board_slave[idxFixed].serial_number = 0;
                for(uint8_t isCfg=0; isCfg<CAN_SENSOR_COUNT_MAX; isCfg++)
                    param.configuration->board_slave[idxFixed].is_configured[isCfg] = true;
                #endif
                #ifdef USE_MODULE_FIXED_WIND
                idxFixed++;
                param.configuration->board_slave[idxFixed].can_address = 62;
                param.configuration->board_slave[idxFixed].module_type = Module_Type::wind;
                param.configuration->board_slave[idxFixed].can_port_id = PORT_RMAP_WIND;
                param.configuration->board_slave[idxFixed].can_publish_id = PORT_RMAP_WIND;
                param.configuration->board_slave[idxFixed].serial_number = 0;
                for(uint8_t isCfg=0; isCfg<CAN_SENSOR_COUNT_MAX; isCfg++)
                    param.configuration->board_slave[idxFixed].is_configured[isCfg] = true;
                #endif
                #ifdef USE_MODULE_FIXED_RADIATION
                idxFixed++;
                param.configuration->board_slave[idxFixed].can_address = 63;
                param.configuration->board_slave[idxFixed].module_type = Module_Type::radiation;
                param.configuration->board_slave[idxFixed].can_port_id = PORT_RMAP_RADIATION;
                param.configuration->board_slave[idxFixed].can_publish_id = PORT_RMAP_RADIATION;
                param.configuration->board_slave[idxFixed].serial_number = 0;
                for(uint8_t isCfg=0; isCfg<CAN_SENSOR_COUNT_MAX; isCfg++)
                    param.configuration->board_slave[idxFixed].is_configured[isCfg] = true;
                #endif
                #ifdef USE_MODULE_FIXED_POWER
                idxFixed++;
                param.configuration->board_slave[idxFixed].can_address = 64;
                param.configuration->board_slave[idxFixed].module_type = Module_Type::power;
                param.configuration->board_slave[idxFixed].can_port_id = PORT_RMAP_MPPT;
                param.configuration->board_slave[idxFixed].can_publish_id = PORT_RMAP_MPPT;
                param.configuration->board_slave[idxFixed].serial_number = 0;
                for(uint8_t isCfg=0; isCfg<CAN_SENSOR_COUNT_MAX; isCfg++)
                    param.configuration->board_slave[idxFixed].is_configured[isCfg] = true;
                #endif
                #ifdef USE_MODULE_FIXED_VWC
                idxFixed++;
                param.configuration->board_slave[idxFixed].can_address = 65;
                param.configuration->board_slave[idxFixed].module_type = Module_Type::vwc;
                param.configuration->board_slave[idxFixed].can_port_id = PORT_RMAP_VWC;
                param.configuration->board_slave[idxFixed].can_publish_id = PORT_RMAP_VWC;
                param.configuration->board_slave[idxFixed].serial_number = 0;
                for(uint8_t isCfg=0; isCfg<CAN_SENSOR_COUNT_MAX; isCfg++)
                    param.configuration->board_slave[idxFixed].is_configured[isCfg] = true;
                #endif
                #ifdef USE_MODULE_FIXED_LEVEL
                idxFixed++;
                param.configuration->board_slave[idxFixed].can_address = 66;
                param.configuration->board_slave[idxFixed].module_type = Module_Type::level;
                param.configuration->board_slave[idxFixed].can_port_id = PORT_RMAP_LEVEL;
                param.configuration->board_slave[idxFixed].can_publish_id = PORT_RMAP_LEVEL;
                param.configuration->board_slave[idxFixed].serial_number = 0;
                for(uint8_t isCfg=0; isCfg<CAN_SENSOR_COUNT_MAX; isCfg++)
                    param.configuration->board_slave[idxFixed].is_configured[isCfg] = true;
                #endif
                #ifdef USE_MODULE_FIXED_LEAF
                idxFixed++;
                param.configuration->board_slave[idxFixed].can_address = 67;
                param.configuration->board_slave[idxFixed].module_type = Module_Type::leaf;
                param.configuration->board_slave[idxFixed].can_port_id = PORT_RMAP_LEAF;
                param.configuration->board_slave[idxFixed].can_publish_id = PORT_RMAP_LEAF;
                param.configuration->board_slave[idxFixed].serial_number = 0;
                for(uint8_t isCfg=0; isCfg<CAN_SENSOR_COUNT_MAX; isCfg++)
                    param.configuration->board_slave[idxFixed].is_configured[isCfg] = true;
                #endif
                // RESET Next Board
                for(uint8_t iIdxRst = idxFixed + 1; iIdxRst < MAX_NODE_CONNECT; iIdxRst++) {
                    param.configuration->board_slave[iIdxRst].can_address = NODE_VALUE_UNSET;
                    param.configuration->board_slave[iIdxRst].module_type = Module_Type::undefined;
                    param.configuration->board_slave[iIdxRst].can_port_id = 0xFFFFu;
                    param.configuration->board_slave[iIdxRst].can_publish_id = 0xFFFFu;
                    param.configuration->board_slave[iIdxRst].serial_number = 0;
                    for(uint8_t isCfg=0; isCfg<CAN_SENSOR_COUNT_MAX; isCfg++)
                        param.configuration->board_slave[iIdxRst].is_configured[isCfg] = false;
                }
                #endif

                // INIT istance class for UAVCAN Network
                for(uint8_t iCnt = 0; iCnt<MAX_NODE_CONNECT; iCnt++) {
                    #ifdef USE_SUB_PUBLISH_SLAVE_DATA
                    // If valid address, configure node
                    if((param.configuration->board_slave[iCnt].module_type)&&(param.configuration->board_slave[iCnt].can_address <= CANARD_NODE_ID_MAX)) {
                        // Configure istance in a class
                        char description_board[STIMA_MODULE_DESCRIPTION_LENGTH];
                        getStimaDescriptionByType(description_board, param.configuration->board_slave[iCnt].module_type);
                        TRACE_INFO_F(F("Configure module slave: [ %s ], Address: %u, Port Id: %u, Publish Id: %u\r\n"),
                            description_board, param.configuration->board_slave[iCnt].can_address,
                            param.configuration->board_slave[iCnt].can_port_id, param.configuration->board_slave[iCnt].can_publish_id);
                        // Configure istance in a class
                        clCanard.slave[iCnt].configure(
                            param.configuration->board_slave[iCnt].can_address,
                            param.configuration->board_slave[iCnt].module_type,
                            param.configuration->board_slave[iCnt].can_port_id,
                            param.configuration->board_slave[iCnt].can_publish_id,
                            param.configuration->board_slave[iCnt].serial_number);
                    }
                    #else
                        // Configure istance in a class
                        clCanard.slave[iCnt].configure(
                            param.configuration->board_slave[iCnt].can_address,
                            param.configuration->board_slave[iCnt].module_type,
                            param.configuration->board_slave[iCnt].can_port_id);
                            param.configuration->board_slave[iCnt].serial_number);
                    #endif
                }

                // Passa alle sottoscrizioni
                state = CAN_STATE_SETUP;
                break;

            // ********************************************************************************
            //               AVVIA SOTTOSCRIZIONI ai messaggi per servizi RPC ecc...
            // ********************************************************************************
            case CAN_STATE_SETUP:

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
                        if(clCanard.slave[queueId].get_module_type() == Module_Type::th) {     
                            // Alloco la stottoscrizione in funzione del tipo di modulo
                            // Service client: -> Risposta per ServiceDataModuleTH richiesta interna (come master)
                            if (!clCanard.rxSubscribe(CanardTransferKindResponse,
                                                    clCanard.slave[queueId].rmap_service.get_port_id(),
                                                    rmap_service_module_TH_Response_1_0_EXTENT_BYTES_,
                                                    CANARD_RMAPDATA_TRANSFER_ID_TIMEOUT_USEC)) {
                                LOCAL_ASSERT(false);
                            }
                        }
                        // Controllo le varie tipologie di request/service per il nodo
                        if(clCanard.slave[queueId].get_module_type() == Module_Type::rain) {     
                            // Alloco la stottoscrizione in funzione del tipo di modulo
                            // Service client: -> Risposta per ServiceDataModuleTH richiesta interna (come master)
                            if (!clCanard.rxSubscribe(CanardTransferKindResponse,
                                                    clCanard.slave[queueId].rmap_service.get_port_id(),
                                                    rmap_service_module_Rain_Response_1_0_EXTENT_BYTES_,
                                                    CANARD_RMAPDATA_TRANSFER_ID_TIMEOUT_USEC)) {
                                LOCAL_ASSERT(false);
                            }
                        }
                        // Controllo le varie tipologie di request/service per il nodo
                        if(clCanard.slave[queueId].get_module_type() == Module_Type::wind) {     
                            // Alloco la stottoscrizione in funzione del tipo di modulo
                            // Service client: -> Risposta per ServiceDataModuleTH richiesta interna (come master)
                            if (!clCanard.rxSubscribe(CanardTransferKindResponse,
                                                    clCanard.slave[queueId].rmap_service.get_port_id(),
                                                    rmap_service_module_Wind_Response_1_0_EXTENT_BYTES_,
                                                    CANARD_RMAPDATA_TRANSFER_ID_TIMEOUT_USEC)) {
                                LOCAL_ASSERT(false);
                            }
                        }
                        // Controllo le varie tipologie di request/service per il nodo
                        if(clCanard.slave[queueId].get_module_type() == Module_Type::radiation) {     
                            // Alloco la stottoscrizione in funzione del tipo di modulo
                            // Service client: -> Risposta per ServiceDataModuleTH richiesta interna (come master)
                            if (!clCanard.rxSubscribe(CanardTransferKindResponse,
                                                    clCanard.slave[queueId].rmap_service.get_port_id(),
                                                    rmap_service_module_Radiation_Response_1_0_EXTENT_BYTES_,
                                                    CANARD_RMAPDATA_TRANSFER_ID_TIMEOUT_USEC)) {
                                LOCAL_ASSERT(false);
                            }
                        }
                        // Controllo le varie tipologie di request/service per il nodo
                        if(clCanard.slave[queueId].get_module_type() == Module_Type::leaf) {     
                            // Alloco la stottoscrizione in funzione del tipo di modulo
                            // Service client: -> Risposta per ServiceDataModuleTH richiesta interna (come master)
                            if (!clCanard.rxSubscribe(CanardTransferKindResponse,
                                                    clCanard.slave[queueId].rmap_service.get_port_id(),
                                                    rmap_service_module_Leaf_Response_1_0_EXTENT_BYTES_,
                                                    CANARD_RMAPDATA_TRANSFER_ID_TIMEOUT_USEC)) {
                                LOCAL_ASSERT(false);
                            }
                        }
                        // Controllo le varie tipologie di request/service per il nodo
                        if(clCanard.slave[queueId].get_module_type() == Module_Type::level) {     
                            // Alloco la stottoscrizione in funzione del tipo di modulo
                            // Service client: -> Risposta per ServiceDataModuleTH richiesta interna (come master)
                            if (!clCanard.rxSubscribe(CanardTransferKindResponse,
                                                    clCanard.slave[queueId].rmap_service.get_port_id(),
                                                    rmap_service_module_RiverLevel_Response_1_0_EXTENT_BYTES_,
                                                    CANARD_RMAPDATA_TRANSFER_ID_TIMEOUT_USEC)) {
                                LOCAL_ASSERT(false);
                            }
                        }
                        // Controllo le varie tipologie di request/service per il nodo
                        if(clCanard.slave[queueId].get_module_type() == Module_Type::vwc) {     
                            // Alloco la stottoscrizione in funzione del tipo di modulo
                            // Service client: -> Risposta per ServiceDataModuleTH richiesta interna (come master)
                            if (!clCanard.rxSubscribe(CanardTransferKindResponse,
                                                    clCanard.slave[queueId].rmap_service.get_port_id(),
                                                    rmap_service_module_VWC_Response_1_0_EXTENT_BYTES_,
                                                    CANARD_RMAPDATA_TRANSFER_ID_TIMEOUT_USEC)) {
                                LOCAL_ASSERT(false);
                            }
                        }
                        // Controllo le varie tipologie di request/service per il nodo
                        if(clCanard.slave[queueId].get_module_type() == Module_Type::power) {     
                            // Alloco la stottoscrizione in funzione del tipo di modulo
                            // Service client: -> Risposta per ServiceDataModuleTH richiesta interna (come master)
                            if (!clCanard.rxSubscribe(CanardTransferKindResponse,
                                                    clCanard.slave[queueId].rmap_service.get_port_id(),
                                                    rmap_service_module_Power_Response_1_0_EXTENT_BYTES_,
                                                    CANARD_RMAPDATA_TRANSFER_ID_TIMEOUT_USEC)) {
                                LOCAL_ASSERT(false);
                            }
                        }
                    }
                    #if defined(USE_SUB_PUBLISH_SLAVE_DATA) && defined(SUBSCRIBE_PUBLISH_SLAVE_DATA)
                    // *************   PUBLISH    *************
                    // Se previsto il servizio publisher (subject_id valido)
                    // Non alloco niente per il publish (gestione esempio display o altro debug interno da gestire)
                    if (clCanard.slave[queueId].publisher.get_subject_id() <= CANARD_SUBJECT_ID_MAX) {
                        // Controllo le varie tipologie di request/service per il nodo
                        if(clCanard.slave[queueId].get_module_type() == Module_Type::th) {            
                            // Alloco la stottoscrizione in funzione del tipo di modulo
                            // Service client: -> Sottoscrizione per ModuleTH (come master)
                            if (!clCanard.rxSubscribe(CanardTransferKindMessage,
                                                    clCanard.slave[queueId].publisher.get_subject_id(),
                                                    rmap_module_TH_1_0_EXTENT_BYTES_,
                                                    CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                                LOCAL_ASSERT(false);
                            }
                        }
                        // Controllo le varie tipologie di request/service per il nodo
                        if(clCanard.slave[queueId].get_module_type() == Module_Type::rain) {            
                            // Alloco la stottoscrizione in funzione del tipo di modulo
                            // Service client: -> Sottoscrizione per ModuleTH (come master)
                            if (!clCanard.rxSubscribe(CanardTransferKindMessage,
                                                    clCanard.slave[queueId].publisher.get_subject_id(),
                                                    rmap_module_Rain_1_0_EXTENT_BYTES_,
                                                    CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                                LOCAL_ASSERT(false);
                            }
                        }
                        // Controllo le varie tipologie di request/service per il nodo
                        if(clCanard.slave[queueId].get_module_type() == Module_Type::wind) {            
                            // Alloco la stottoscrizione in funzione del tipo di modulo
                            // Service client: -> Sottoscrizione per ModuleTH (come master)
                            if (!clCanard.rxSubscribe(CanardTransferKindMessage,
                                                    clCanard.slave[queueId].publisher.get_subject_id(),
                                                    rmap_module_Wind_1_0_EXTENT_BYTES_,
                                                    CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                                LOCAL_ASSERT(false);
                            }
                        }
                        // Controllo le varie tipologie di request/service per il nodo
                        if(clCanard.slave[queueId].get_module_type() == Module_Type::radiation) {            
                            // Alloco la stottoscrizione in funzione del tipo di modulo
                            // Service client: -> Sottoscrizione per ModuleTH (come master)
                            if (!clCanard.rxSubscribe(CanardTransferKindMessage,
                                                    clCanard.slave[queueId].publisher.get_subject_id(),
                                                    rmap_module_Radiation_1_0_EXTENT_BYTES_,
                                                    CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                                LOCAL_ASSERT(false);
                            }
                        }
                        // Controllo le varie tipologie di request/service per il nodo
                        if(clCanard.slave[queueId].get_module_type() == Module_Type::leaf) {            
                            // Alloco la stottoscrizione in funzione del tipo di modulo
                            // Service client: -> Sottoscrizione per ModuleTH (come master)
                            if (!clCanard.rxSubscribe(CanardTransferKindMessage,
                                                    clCanard.slave[queueId].publisher.get_subject_id(),
                                                    rmap_module_Leaf_1_0_EXTENT_BYTES_,
                                                    CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                                LOCAL_ASSERT(false);
                            }
                        }
                        // Controllo le varie tipologie di request/service per il nodo
                        if(clCanard.slave[queueId].get_module_type() == Module_Type::level) {            
                            // Alloco la stottoscrizione in funzione del tipo di modulo
                            // Service client: -> Sottoscrizione per ModuleTH (come master)
                            if (!clCanard.rxSubscribe(CanardTransferKindMessage,
                                                    clCanard.slave[queueId].publisher.get_subject_id(),
                                                    rmap_module_RiverLevel_1_0_EXTENT_BYTES_,
                                                    CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                                LOCAL_ASSERT(false);
                            }
                        }
                        // Controllo le varie tipologie di request/service per il nodo
                        if(clCanard.slave[queueId].get_module_type() == Module_Type::vwc) {            
                            // Alloco la stottoscrizione in funzione del tipo di modulo
                            // Service client: -> Sottoscrizione per ModuleTH (come master)
                            if (!clCanard.rxSubscribe(CanardTransferKindMessage,
                                                    clCanard.slave[queueId].publisher.get_subject_id(),
                                                    rmap_module_VWC_1_0_EXTENT_BYTES_,
                                                    CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                                LOCAL_ASSERT(false);
                            }
                        }
                        // Controllo le varie tipologie di request/service per il nodo
                        if(clCanard.slave[queueId].get_module_type() == Module_Type::power) {            
                            // Alloco la stottoscrizione in funzione del tipo di modulo
                            // Service client: -> Sottoscrizione per ModuleTH (come master)
                            if (!clCanard.rxSubscribe(CanardTransferKindMessage,
                                                    clCanard.slave[queueId].publisher.get_subject_id(),
                                                    rmap_module_Power_1_0_EXTENT_BYTES_,
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
                            TRACE_INFO_F(F("Nodo OFFLINE !!! Alert -> : %d\r\n"), clCanard.slave[queueId].get_node_id());
                            // Metto i Flag in sicurezza, laddove dove non eventualmente gestito
                            // Eventuali altre operazioni quà su Reset Comandi
                            clCanard.slave[queueId].command.reset_pending();
                            clCanard.slave[queueId].register_access.reset_pending();
                            clCanard.slave[queueId].file_server.reset_pending();
                            clCanard.slave[queueId].rmap_service.reset_pending();
                            // Set system_status (Set Node Offline)
                            // Set module_version to unknown version to force get reload version and revision
                            // If new firmware are updated (local or remote command) node go to offline mode...
                            param.systemStatusLock->Take();
                            param.system_status->data_slave[queueId].is_online = false;
                            param.system_status->data_slave[queueId].module_version = 0;
                            param.system_status->data_slave[queueId].module_revision = 0;
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
                                param.boot_request->app_executed_ok = false;
                                param.boot_request->backup_executed = false;
                                param.boot_request->app_forcing_start = false;
                                param.boot_request->rollback_executed = false;
                                param.boot_request->request_upload = true;
                                param.eeprom->Write(BOOT_LOADER_STRUCT_ADDR, (uint8_t*) param.boot_request, sizeof(bootloader_t));
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

                // ***********************************************************************
                // **********  GET SYNCRO ACTIVITY FOR CYPAL RMAP DATA FUNCTION  *********
                // ***********************************************************************
                // Test if data are to acquire (one time for second)
                if(clCanard.getUpTimeSecond()!=getUpTimeSecondCurr)
                {
                    // Update check next acquire test
                    getUpTimeSecondCurr = clCanard.getUpTimeSecond();
                    if (param.rtcLock->Take(Ticks::MsToTicks(RTC_WAIT_DELAY_MS))) {
                        curEpoch = rtc.getEpoch();
                        param.rtcLock->Give();
                    }
                    // need do acquire istant value for display?
                    // N.B. param.system_status->datetime.ptr_time_for_sensors_get_istant automatic resetted when node entering onLine from offLine
                    // Read data only if Display or other Task need to show/get this value
                    if((param.system_status->flags.display_on)||(bStartupGetIstant)) {
                        if ((curEpoch / CONFIGURATION_DEFAULT_UPDATE_IST_S) != param.system_status->datetime.ptr_time_for_sensors_get_istant) {
                            param.systemStatusLock->Take();
                            param.system_status->datetime.ptr_time_for_sensors_get_istant = curEpoch / CONFIGURATION_DEFAULT_UPDATE_IST_S;
                            param.system_status->datetime.epoch_sensors_get_istant = (curEpoch / CONFIGURATION_DEFAULT_UPDATE_IST_S) * CONFIGURATION_DEFAULT_UPDATE_IST_S;
                            param.systemStatusLock->Give();
                            bStartGetIstant = true;
                        }
                    }
                    // need do acquire data value for RMAP Archive?
                    // Perform an Full Power request Method SEC_WAKE_UP_MODULE_FOR_QUERY (5) second before Starting aquire data
                    // In this time we can regulate syncro_time method and perform Full Wake UP of remote Module
                    if (((curEpoch + SEC_WAKE_UP_MODULE_FOR_QUERY) / param.configuration->report_s) != param.system_status->datetime.ptr_time_for_sensors_get_value) {      
                        // WakeUP Network for reading sensor and Synncronize date_time
                        TRACE_VERBOSE_F(F("Rmap data server: Start full power for sending request and syncronize time\r\n"));
                        // Only for RMAP Get Data is need to Forced power ON on Starting time before procedure GET DATA
                        // RMAP Get data dont' start with command request automatic but some second before (also syncro_time)
                        // This is need to fix problem of request ON/OFF continuative for module and simulate waiting command...
                        bStartSetFullPower = true;
                        // Starting server get data (dont stop when bStartSetFullPower is setted. Only for Set Full POWER)
                        param.systemStatusLock->Take();
                        param.system_status->flags.rmap_server_running = true;
                        param.systemStatusLock->Give();
                    } else {
                        // Normal mode (Not WakeUP)
                        bStartSetFullPower = false;
                    }
                    if ((curEpoch / param.configuration->report_s) != param.system_status->datetime.ptr_time_for_sensors_get_value) {
                        // WakeUP Network for reading sensor and Synncronize date_time
                        TRACE_VERBOSE_F(F("Rmap data server: Start acquire request to sensor network\r\n"));
                        param.systemStatusLock->Take();
                        param.system_status->datetime.ptr_time_for_sensors_get_value = curEpoch / param.configuration->report_s;
                        param.system_status->datetime.epoch_sensors_get_value = (curEpoch / param.configuration->report_s) * param.configuration->report_s;
                        // Calculate expected/recived HeartBeat sequence... and reset counter
                        // Only At first data % can be < 100%, depending of acquire time but isn't a real error
                        for(uint8_t iSlave = 0; iSlave < BOARDS_COUNT_MAX; iSlave++) {
                            // Calculate % from correct Heartbeat sequence TX-RX Transaction Completed (if one is executed)
                            if(param.system_status->data_slave[iSlave].heartbeat_rx > MIN_VALID_HEARTBEAT_RX) {
                                if(param.system_status->data_slave[iSlave].heartbeat_rx > MIN_TRANSACTION_HEARTBEAT_RX) {
                                    param.system_status->data_slave[iSlave].perc_can_comm_err = (uint8_t)((float)(param.system_status->data_slave[iSlave].heartbeat_rx_err - MIN_TRANSACTION_HEARTBEAT_RX) / (float)(param.system_status->data_slave[iSlave].heartbeat_rx) * 100.0);
                                } else {
                                    param.system_status->data_slave[iSlave].perc_can_comm_err = 0;
                                }
                            } else {
                                param.system_status->data_slave[iSlave].perc_can_comm_err = 100;
                            }
                            // Reset Next Counter for next acquire data
                            param.system_status->data_slave[iSlave].heartbeat_rx = 0;
                            param.system_status->data_slave[iSlave].heartbeat_rx_err = 0;                            
                        }
                        // Require new connection at time (report time is passed)
                        param.system_status->flags.new_start_connect = true;
                        param.systemStatusLock->Give();
                        bStartGetData = true;
                    }
                }
                // ************* END GET SYNCRO ACTIVITY FOR CYPAL FUNCTION **************

                // ***********************************************************************
                // ********************** RMAP GETDATA TX-> RX<- *************************
                // ***********************************************************************

                // IS START COMMAND DATA RMAP AUTOMATIC REQUEST (From Local Syncro Activity UP...)?
                // Get Istant Data or Archive Data Request (Need to Display, Saving Data or other Function with Istant/Archive Data)
                if ((bStartGetIstant)||(bStartGetData)) {
                    // For all node
                    // bStartGetData are priority Command if both requested
                    for(uint8_t queueId=0; queueId<MAX_NODE_CONNECT; queueId++) {
                        // For all node onLine
                        if(clCanard.slave[queueId].is_online()) {
                            // Command are sending without other Pending Command
                            // Service request structure is same for all RMAP object to simplify function
                            if(!clCanard.slave[queueId].rmap_service.is_pending()) {
                                TRACE_INFO_F(F("Inviato richiesta dati al nodo remoto: %d\r\n"), clCanard.slave[queueId].get_node_id());
                                // parametri.canale = rmap_service_setmode_1_0_CH01 (es-> set CH Analogico...)
                                // parametri.run_for_second = 900; ( not used for get_istant )
                                rmap_service_setmode_1_0 paramRequest;
                                paramRequest.chanel = 0; // Request only for chanel analog/digital adressed 1..X
                                // higher priority guaranteed
                                if(bStartGetData) {
                                    paramRequest.command = rmap_service_setmode_1_0_get_last;
                                    paramRequest.obs_sectime = param.configuration->observation_s;
                                    paramRequest.run_sectime = param.configuration->report_s;
                                } else {
                                    paramRequest.command = rmap_service_setmode_1_0_get_istant;
                                    paramRequest.obs_sectime = 0;
                                    paramRequest.run_sectime = 0;
                                }
                                // Imposta il pending del comando per verifica sequenza TX-RX e il TimeOut e in num retry.
                                // La risposta al comando è già nel blocco dati, non necessaria ulteriore variabile
                                clCanard.send_rmap_data_pending(queueId, NODE_GETDATA_TIMEOUT_US, paramRequest, NODE_GETDATA_MAX_RETRY);
                                // Avvio il server RMAP Request
                                param.systemStatusLock->Take();
                                param.system_status->flags.rmap_server_running = true;
                                param.systemStatusLock->Give();
                            }
                        }
                    }
                    // Remove start command request when command are sent
                    bStartGetIstant = false;
                    bStartGetData = false;
                }

                // EVENT GESTION OF TIMEOUT AT REQUEST
                if(param.system_status->flags.rmap_server_running) {

                    // Check if file server are current in running state
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
                                TRACE_ERROR_F(F("Timeout risposta su richiesta dati al nodo remoto: %d, Warning [restore pending command]\r\n"),
                                    clCanard.slave[queueId].get_node_id());
                            } else {
                                rmapServerEnd = false;
                                // TimeOUT di un comando in attesa... gestione Retry
                                TRACE_ERROR_F(F("Timeout risposta su richiesta dati al nodo remoto: %d, Warning [retry command: %d]\r\n"),
                                    clCanard.slave[queueId].get_node_id(), clCanard.slave[queueId].rmap_service.retry + 1);
                            }
                        }
                    }
                    // EVENT GESTION OF RECIVED DATA AT REQUEST
                    for(uint8_t queueId=0; queueId<MAX_NODE_CONNECT; queueId++) {
                        if(clCanard.slave[queueId].rmap_service.is_executed()) {
                            clCanard.slave[queueId].rmap_service.reset_pending();
                            // Interprete del messaggio in casting dal puntatore dinamico
                            // Nell'esempio Il modulo e TH, naturalmente bisogna gestire il tipo
                            // in funzione di clCanard.slave[x].node_type
                            switch (clCanard.slave[queueId].get_module_type()) {
                                case Module_Type::th:
                                    // Cast to th module
                                    retTHData = (rmap_service_module_TH_Response_1_0*) clCanard.slave[queueId].rmap_service.get_response();
                                    // data RMAP type is ready to send into queue Archive Data for Saving on SD Memory
                                    // Get parameter data
                                    #if TRACE_LEVEL >= TRACE_INFO
                                    getStimaNameByType(stimaName, clCanard.slave[queueId].get_module_type());
                                    #endif
                                    // Put data in system_status with module_type and RMAP Ver.Rev if not equal (reload or updated)
                                    if((param.system_status->data_slave[queueId].module_version!=retTHData->version) ||
                                       (param.system_status->data_slave[queueId].module_revision!=retTHData->revision)) {
                                        param.systemStatusLock->Take();
                                        param.system_status->data_slave[queueId].module_version = retTHData->version;
                                        param.system_status->data_slave[queueId].module_revision = retTHData->revision;
                                        // Module type also setted on load config module CAN
                                        param.system_status->data_slave[queueId].module_type = clCanard.slave[queueId].get_module_type();
                                        // Reset flag before recheck exixting firmware available
                                        param.system_status->data_slave[queueId].fw_upgradable = false;
                                        // Check if module can be updated
                                        for(uint8_t checkId=0; checkId<STIMA_MODULE_TYPE_MAX_AVAIABLE; checkId++) {
                                            if(clCanard.slave[queueId].get_module_type() == param.system_status->boards_update_avaiable[checkId].module_type) {
                                                if((param.system_status->boards_update_avaiable[checkId].version > retTHData->version) ||
                                                    ((param.system_status->boards_update_avaiable[checkId].version == retTHData->version) && 
                                                    (param.system_status->boards_update_avaiable[checkId].revision > retTHData->revision))) {
                                                    // Found an upgradable boards
                                                    param.system_status->data_slave[queueId].fw_upgradable = true;
                                                }
                                                break;
                                            }
                                        }
                                        param.systemStatusLock->Give();
                                    }
                                    // TRACE Info data
                                    TRACE_INFO_F(F("RMAP recived response data module from [ %s ], node id: %d. Response code: %d\r\n"),
                                        stimaName, clCanard.slave[queueId].get_node_id(), retTHData->state);
                                    TRACE_VERBOSE_F(F("Value (ITH) TP %d, UH: %d\r\n"), retTHData->ITH.temperature.val.value, retTHData->ITH.humidity.val.value);
                                    // Get security remote state on maintenance mode from relative state flags
                                    param.system_status->data_slave[queueId].maintenance_mode = (retTHData->state & CAN_FLAG_IS_MAINTENANCE_MODE);
                                    retTHData->state &= CAN_FLAG_MASK_MAINTENANCE_MODE;
                                    if(param.system_status->data_slave[queueId].maintenance_mode) {
                                        TRACE_INFO_F(F("Warning this module is in maintenance mode!!!\r\n"));
                                    }
                                    // Put istant data in system_status
                                    if(retTHData->state == rmap_service_setmode_1_0_get_istant) {
                                        // Only istant request LCD or other device
                                        param.systemStatusLock->Take();
                                        param.system_status->data_slave[queueId].data_value[0] = retTHData->ITH.temperature.val.value;
                                        param.system_status->data_slave[queueId].data_value[1] = retTHData->ITH.humidity.val.value;
                                        param.system_status->data_slave[queueId].is_new_ist_data_ready = true;
                                        param.systemStatusLock->Give();
                                    } else if(retTHData->state == rmap_service_setmode_1_0_get_last) {
                                        // data value id rmap_service_setmode_1_0_get_last into queue SD
                                        // Copy Flag State
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
                                        // Copy Data
                                        memset(&rmap_archive_data, 0, sizeof(rmap_archive_data_t));
                                        // Set Module Type, Date Time as Uint32 GetEpoch_Style, and Block Data Cast to RMAP Type
                                        rmap_archive_data.module_type = clCanard.slave[queueId].get_module_type();
                                        rmap_archive_data.date_time = param.system_status->datetime.epoch_sensors_get_value;
                                        memcpy(rmap_archive_data.block, retTHData, sizeof(*retTHData));
                                        // Send queue to SD for direct archive data
                                        // Queue is dimensioned to accept all Data for one step pushing array data (MAX_BOARDS)
                                        // Clean queue if is full to send alwayl the last data on getted value
                                        if(param.dataRmapPutQueue->IsFull()) param.dataLogPutQueue->Dequeue(&rmap_archive_empty);
                                        param.dataRmapPutQueue->Enqueue(&rmap_archive_data, Ticks::MsToTicks(CAN_PUT_QUEUE_RMAP_TIMEOUT_MS));
                                    }
                                    break;

                                case Module_Type::rain:
                                    // Cast to th module
                                    retRainData = (rmap_service_module_Rain_Response_1_0*) clCanard.slave[queueId].rmap_service.get_response();
                                    // data RMAP type is ready to send into queue Archive Data for Saving on SD Memory
                                    // Get parameter data
                                    #if TRACE_LEVEL >= TRACE_INFO
                                    getStimaNameByType(stimaName, clCanard.slave[queueId].get_module_type());
                                    #endif
                                    // Put data in system_status with module_type and RMAP Ver.Rev if not equal (reload or updated)
                                    if((param.system_status->data_slave[queueId].module_version!=retRainData->version) ||
                                       (param.system_status->data_slave[queueId].module_revision!=retRainData->revision)) {
                                        param.systemStatusLock->Take();
                                        param.system_status->data_slave[queueId].module_version = retRainData->version;
                                        param.system_status->data_slave[queueId].module_revision = retRainData->revision;
                                        // Module type also setted on load config module CAN
                                        param.system_status->data_slave[queueId].module_type = clCanard.slave[queueId].get_module_type();
                                        // Reset flag before recheck exixting firmware available
                                        param.system_status->data_slave[queueId].fw_upgradable = false;
                                        // Check if module can be updated
                                        for(uint8_t checkId=0; checkId<STIMA_MODULE_TYPE_MAX_AVAIABLE; checkId++) {
                                            if(clCanard.slave[queueId].get_module_type() == param.system_status->boards_update_avaiable[checkId].module_type) {
                                                if((param.system_status->boards_update_avaiable[checkId].version > retRainData->version) ||
                                                    ((param.system_status->boards_update_avaiable[checkId].version == retRainData->version) && 
                                                    (param.system_status->boards_update_avaiable[checkId].revision > retRainData->revision))) {
                                                    // Found an upgradable boards
                                                    param.system_status->data_slave[queueId].fw_upgradable = true;
                                                }
                                                break;
                                            }
                                        }
                                        param.systemStatusLock->Give();
                                    }
                                    // TRACE Info data
                                    TRACE_INFO_F(F("RMAP recived response data module from [ %s ], node id: %d. Response code: %d\r\n"),
                                        stimaName, clCanard.slave[queueId].get_node_id(), retRainData->state);
                                    TRACE_VERBOSE_F(F("Value (TBR) Rain %d\r\n"), retRainData->TBR.rain.val.value);
                                    // Get security remote state on maintenance mode from relative state flags
                                    param.system_status->data_slave[queueId].maintenance_mode = (retRainData->state & CAN_FLAG_IS_MAINTENANCE_MODE);
                                    retRainData->state &= CAN_FLAG_MASK_MAINTENANCE_MODE;
                                    if(param.system_status->data_slave[queueId].maintenance_mode) {
                                        TRACE_INFO_F(F("Warning this module is in maintenance mode!!!\r\n"));
                                    }
                                    // Put istant data in system_status
                                    if(retRainData->state == rmap_service_setmode_1_0_get_istant) {
                                        // Only istant request LCD or other device
                                        param.systemStatusLock->Take();
                                        // Rain value istant depending from request type. If request is get_istant (Rain is FullRain)
                                        // Full Rain is Real Rain + Maintenance Rain Value, Otherwise if request is Get_Data, Rain is only
                                        // Real Rain data (without maintenece value). Master can set via CAN (...LCD Command) Maintenance Mode
                                        param.system_status->data_slave[queueId].data_value[0] = retRainData->TBR.rain.val.value;
                                        param.system_status->data_slave[queueId].is_new_ist_data_ready = true;
                                        param.systemStatusLock->Give();
                                    } else if(retRainData->state == rmap_service_setmode_1_0_get_last) {
                                        // data value id rmap_service_setmode_1_0_get_last into queue SD
                                        // Copy Flag State
                                        bit8Flag = 0;
                                        if(retRainData->is_main_error) bit8Flag|=0x01;
                                        if(retRainData->is_redundant_error) bit8Flag|=0x02;
                                        if(retRainData->is_tipping_error) bit8Flag|=0x04;
                                        if(retRainData->is_clogged_up) bit8Flag|=0x08;
                                        if(retRainData->is_bubble_level_error) bit8Flag|=0x10;
                                        if(retRainData->is_accelerometer_error) bit8Flag|=0x20;
                                        param.systemStatusLock->Take();
                                        param.system_status->flags.new_data_to_send = true;
                                        param.system_status->data_slave[queueId].bit8StateFlag = bit8Flag;
                                        param.system_status->data_slave[queueId].byteStateFlag[0] = retRainData->rbt_event;
                                        param.system_status->data_slave[queueId].byteStateFlag[1] = retRainData->wdt_event;
                                        param.system_status->data_slave[queueId].byteStateFlag[2] = 0;
                                        param.systemStatusLock->Give();
                                        // Copy Data
                                        memset(&rmap_archive_data, 0, sizeof(rmap_archive_data_t));
                                        // Set Module Type, Date Time as Uint32 GetEpoch_Style, and Block Data Cast to RMAP Type
                                        rmap_archive_data.module_type = clCanard.slave[queueId].get_module_type();
                                        rmap_archive_data.date_time = param.system_status->datetime.epoch_sensors_get_value;
                                        memcpy(rmap_archive_data.block, retRainData, sizeof(*retRainData));
                                        // Send queue to SD for direct archive data
                                        // Queue is dimensioned to accept all Data for one step pushing array data (MAX_BOARDS)
                                        // Clean queue if is full to send alwayl the last data on getted value
                                        if(param.dataRmapPutQueue->IsFull()) param.dataLogPutQueue->Dequeue(&rmap_archive_empty);
                                        param.dataRmapPutQueue->Enqueue(&rmap_archive_data, Ticks::MsToTicks(CAN_PUT_QUEUE_RMAP_TIMEOUT_MS));
                                    }
                                    break;

                                case Module_Type::wind:
                                    // Cast to th module
                                    retWindData = (rmap_service_module_Wind_Response_1_0*) clCanard.slave[queueId].rmap_service.get_response();
                                    // data RMAP type is ready to send into queue Archive Data for Saving on SD Memory
                                    // Get parameter data
                                    #if TRACE_LEVEL >= TRACE_INFO
                                    getStimaNameByType(stimaName, clCanard.slave[queueId].get_module_type());
                                    #endif
                                    // Put data in system_status with module_type and RMAP Ver.Rev if not equal (reload or updated)
                                    if((param.system_status->data_slave[queueId].module_version!=retWindData->version) ||
                                       (param.system_status->data_slave[queueId].module_revision!=retWindData->revision)) {
                                        param.systemStatusLock->Take();
                                        param.system_status->data_slave[queueId].module_version = retWindData->version;
                                        param.system_status->data_slave[queueId].module_revision = retWindData->revision;
                                        // Module type also setted on load config module CAN
                                        param.system_status->data_slave[queueId].module_type = clCanard.slave[queueId].get_module_type();
                                        // Reset flag before recheck exixting firmware available
                                        param.system_status->data_slave[queueId].fw_upgradable = false;
                                        // Check if module can be updated
                                        for(uint8_t checkId=0; checkId<STIMA_MODULE_TYPE_MAX_AVAIABLE; checkId++) {
                                            if(clCanard.slave[queueId].get_module_type() == param.system_status->boards_update_avaiable[checkId].module_type) {
                                                if((param.system_status->boards_update_avaiable[checkId].version > retWindData->version) ||
                                                    ((param.system_status->boards_update_avaiable[checkId].version == retWindData->version) && 
                                                    (param.system_status->boards_update_avaiable[checkId].revision > retWindData->revision))) {
                                                    // Found an upgradable boards
                                                    param.system_status->data_slave[queueId].fw_upgradable = true;
                                                }
                                                break;
                                            }
                                        }
                                        param.systemStatusLock->Give();
                                    }
                                    // TRACE Info data
                                    TRACE_INFO_F(F("RMAP recived response data module from [ %s ], node id: %d. Response code: %d\r\n"),
                                        stimaName, clCanard.slave[queueId].get_node_id(), retWindData->state);
                                    TRACE_VERBOSE_F(F("Value (DWA) Speed %d, Dir: %d\r\n"), retWindData->DWA.speed.val.value, retWindData->DWA.direction.val.value);
                                    // Get security remote state on maintenance mode from relative state flags
                                    param.system_status->data_slave[queueId].maintenance_mode = (retWindData->state & CAN_FLAG_IS_MAINTENANCE_MODE);
                                    retWindData->state &= CAN_FLAG_MASK_MAINTENANCE_MODE;
                                    if(param.system_status->data_slave[queueId].maintenance_mode) {
                                        TRACE_INFO_F(F("Warning this module is in maintenance mode!!!\r\n"));
                                    }
                                    // Put istant data in system_status
                                    if(retWindData->state == rmap_service_setmode_1_0_get_istant) {
                                        // Only istant request LCD or other device
                                        param.systemStatusLock->Take();
                                        param.system_status->data_slave[queueId].data_value[0] = retWindData->DWA.speed.val.value;
                                        param.system_status->data_slave[queueId].data_value[1] = retWindData->DWA.direction.val.value;
                                        param.system_status->data_slave[queueId].is_new_ist_data_ready = true;
                                        param.systemStatusLock->Give();
                                    } else if(retWindData->state == rmap_service_setmode_1_0_get_last) {
                                        // data value id rmap_service_setmode_1_0_get_last into queue SD
                                        // Copy Flag State
                                        bit8Flag = 0;
                                        if(retWindData->is_windsonic_responding_error) bit8Flag|=0x01;
                                        if(retWindData->is_windsonic_hardware_error) bit8Flag|=0x02;
                                        if(retWindData->is_windsonic_unit_error) bit8Flag|=0x04;
                                        if(retWindData->is_windsonic_axis_error) bit8Flag|=0x08;
                                        if(retWindData->is_windsonic_crc_error) bit8Flag|=0x10;
                                        param.systemStatusLock->Take();
                                        param.system_status->flags.new_data_to_send = true;
                                        param.system_status->data_slave[queueId].bit8StateFlag = bit8Flag;
                                        param.system_status->data_slave[queueId].byteStateFlag[0] = retWindData->rbt_event;
                                        param.system_status->data_slave[queueId].byteStateFlag[1] = retWindData->wdt_event;
                                        param.system_status->data_slave[queueId].byteStateFlag[2] = retWindData->perc_rs232_error;
                                        param.systemStatusLock->Give();
                                        // Copy Data
                                        memset(&rmap_archive_data, 0, sizeof(rmap_archive_data_t));
                                        // Set Module Type, Date Time as Uint32 GetEpoch_Style, and Block Data Cast to RMAP Type
                                        rmap_archive_data.module_type = clCanard.slave[queueId].get_module_type();
                                        rmap_archive_data.date_time = param.system_status->datetime.epoch_sensors_get_value;
                                        memcpy(rmap_archive_data.block, retWindData, sizeof(*retWindData));
                                        // Send queue to SD for direct archive data
                                        // Queue is dimensioned to accept all Data for one step pushing array data (MAX_BOARDS)
                                        // Clean queue if is full to send alwayl the last data on getted value
                                        if(param.dataRmapPutQueue->IsFull()) param.dataLogPutQueue->Dequeue(&rmap_archive_empty);
                                        param.dataRmapPutQueue->Enqueue(&rmap_archive_data, Ticks::MsToTicks(CAN_PUT_QUEUE_RMAP_TIMEOUT_MS));
                                    }
                                    break;

                                case Module_Type::radiation:
                                    // Cast to th module
                                    retRadiationData = (rmap_service_module_Radiation_Response_1_0*) clCanard.slave[queueId].rmap_service.get_response();
                                    // data RMAP type is ready to send into queue Archive Data for Saving on SD Memory
                                    // Get parameter data
                                    #if TRACE_LEVEL >= TRACE_INFO
                                    getStimaNameByType(stimaName, clCanard.slave[queueId].get_module_type());
                                    #endif
                                    // Put data in system_status with module_type and RMAP Ver.Rev if not equal (reload or updated)
                                    if((param.system_status->data_slave[queueId].module_version!=retRadiationData->version) ||
                                       (param.system_status->data_slave[queueId].module_revision!=retRadiationData->revision)) {
                                        param.systemStatusLock->Take();
                                        param.system_status->data_slave[queueId].module_version = retRadiationData->version;
                                        param.system_status->data_slave[queueId].module_revision = retRadiationData->revision;
                                        // Module type also setted on load config module CAN
                                        param.system_status->data_slave[queueId].module_type = clCanard.slave[queueId].get_module_type();
                                        // Reset flag before recheck exixting firmware available
                                        param.system_status->data_slave[queueId].fw_upgradable = false;
                                        // Check if module can be updated
                                        for(uint8_t checkId=0; checkId<STIMA_MODULE_TYPE_MAX_AVAIABLE; checkId++) {
                                            if(clCanard.slave[queueId].get_module_type() == param.system_status->boards_update_avaiable[checkId].module_type) {
                                                if((param.system_status->boards_update_avaiable[checkId].version > retRadiationData->version) ||
                                                    ((param.system_status->boards_update_avaiable[checkId].version == retRadiationData->version) && 
                                                    (param.system_status->boards_update_avaiable[checkId].revision > retRadiationData->revision))) {
                                                    // Found an upgradable boards
                                                    param.system_status->data_slave[queueId].fw_upgradable = true;
                                                }
                                                break;
                                            }
                                        }
                                        param.systemStatusLock->Give();
                                    }
                                    // TRACE Info data
                                    TRACE_INFO_F(F("RMAP recived response data module from [ %s ], node id: %d. Response code: %d\r\n"),
                                        stimaName, clCanard.slave[queueId].get_node_id(), retRadiationData->state);
                                    TRACE_VERBOSE_F(F("Value (DSA) Radiation %d\r\n"), retRadiationData->DSA.radiation.val.value);
                                    // Get security remote state on maintenance mode from relative state flags
                                    param.system_status->data_slave[queueId].maintenance_mode = (retRadiationData->state & CAN_FLAG_IS_MAINTENANCE_MODE);
                                    retRadiationData->state &= CAN_FLAG_MASK_MAINTENANCE_MODE;
                                    if(param.system_status->data_slave[queueId].maintenance_mode) {
                                        TRACE_INFO_F(F("Warning this module is in maintenance mode!!!\r\n"));
                                    }
                                    // Put istant data in system_status
                                    if(retRadiationData->state == rmap_service_setmode_1_0_get_istant) {
                                        // Only istant request LCD or other device
                                        param.systemStatusLock->Take();
                                        // Set data istant value (switch depends from request, istant = sample, Data = Avg.)
                                        param.system_status->data_slave[queueId].data_value[0] = retRadiationData->DSA.radiation.val.value;
                                        param.system_status->data_slave[queueId].is_new_ist_data_ready = true;
                                        param.systemStatusLock->Give();
                                    } else if(retRadiationData->state == rmap_service_setmode_1_0_get_last) {
                                        // data value id rmap_service_setmode_1_0_get_last into queue SD
                                        // Copy Flag State
                                        bit8Flag = 0;
                                        if(retRadiationData->is_adc_unit_error) bit8Flag|=0x01;
                                        if(retRadiationData->is_adc_unit_overflow) bit8Flag|=0x02;
                                        param.systemStatusLock->Take();
                                        param.system_status->flags.new_data_to_send = true;
                                        param.system_status->data_slave[queueId].bit8StateFlag = bit8Flag;
                                        param.system_status->data_slave[queueId].byteStateFlag[0] = retRadiationData->rbt_event;
                                        param.system_status->data_slave[queueId].byteStateFlag[1] = retRadiationData->wdt_event;
                                        param.system_status->data_slave[queueId].byteStateFlag[2] = 0;
                                        param.systemStatusLock->Give();
                                        // Copy Data
                                        memset(&rmap_archive_data, 0, sizeof(rmap_archive_data_t));
                                        // Set Module Type, Date Time as Uint32 GetEpoch_Style, and Block Data Cast to RMAP Type
                                        rmap_archive_data.module_type = clCanard.slave[queueId].get_module_type();
                                        rmap_archive_data.date_time = param.system_status->datetime.epoch_sensors_get_value;
                                        memcpy(rmap_archive_data.block, retRadiationData, sizeof(*retRadiationData));
                                        // Send queue to SD for direct archive data
                                        // Queue is dimensioned to accept all Data for one step pushing array data (MAX_BOARDS)
                                        // Clean queue if is full to send alwayl the last data on getted value
                                        if(param.dataRmapPutQueue->IsFull()) param.dataLogPutQueue->Dequeue(&rmap_archive_empty);
                                        param.dataRmapPutQueue->Enqueue(&rmap_archive_data, Ticks::MsToTicks(CAN_PUT_QUEUE_RMAP_TIMEOUT_MS));
                                    }
                                    break;

                                case Module_Type::leaf:
                                    // Cast to th module
                                    retLeafData = (rmap_service_module_Leaf_Response_1_0*) clCanard.slave[queueId].rmap_service.get_response();
                                    // data RMAP type is ready to send into queue Archive Data for Saving on SD Memory
                                    // Get parameter data
                                    #if TRACE_LEVEL >= TRACE_INFO
                                    getStimaNameByType(stimaName, clCanard.slave[queueId].get_module_type());
                                    #endif
                                    // Put data in system_status with module_type and RMAP Ver.Rev if not equal (reload or updated)
                                    if((param.system_status->data_slave[queueId].module_version!=retLeafData->version) ||
                                       (param.system_status->data_slave[queueId].module_revision!=retLeafData->revision)) {
                                        param.systemStatusLock->Take();
                                        param.system_status->data_slave[queueId].module_version = retLeafData->version;
                                        param.system_status->data_slave[queueId].module_revision = retLeafData->revision;
                                        // Module type also setted on load config module CAN
                                        param.system_status->data_slave[queueId].module_type = clCanard.slave[queueId].get_module_type();
                                        // Reset flag before recheck exixting firmware available
                                        param.system_status->data_slave[queueId].fw_upgradable = false;
                                        // Check if module can be updated
                                        for(uint8_t checkId=0; checkId<STIMA_MODULE_TYPE_MAX_AVAIABLE; checkId++) {
                                            if(clCanard.slave[queueId].get_module_type() == param.system_status->boards_update_avaiable[checkId].module_type) {
                                                if((param.system_status->boards_update_avaiable[checkId].version > retLeafData->version) ||
                                                    ((param.system_status->boards_update_avaiable[checkId].version == retLeafData->version) && 
                                                    (param.system_status->boards_update_avaiable[checkId].revision > retLeafData->revision))) {
                                                    // Found an upgradable boards
                                                    param.system_status->data_slave[queueId].fw_upgradable = true;
                                                }
                                                break;
                                            }
                                        }
                                        param.systemStatusLock->Give();
                                    }
                                    // TRACE Info data
                                    TRACE_INFO_F(F("RMAP recived response data module from [ %s ], node id: %d. Response code: %d\r\n"),
                                        stimaName, clCanard.slave[queueId].get_node_id(), retLeafData->state);
                                    TRACE_VERBOSE_F(F("Value (BFT) Leaf %d\r\n"), retLeafData->BFT.leaf.val.value);
                                    // Get security remote state on maintenance mode from relative state flags
                                    param.system_status->data_slave[queueId].maintenance_mode = (retLeafData->state & CAN_FLAG_IS_MAINTENANCE_MODE);
                                    retLeafData->state &= CAN_FLAG_MASK_MAINTENANCE_MODE;
                                    if(param.system_status->data_slave[queueId].maintenance_mode) {
                                        TRACE_INFO_F(F("Warning this module is in maintenance mode!!!\r\n"));
                                    }
                                    // Put istant data in system_status
                                    if(retLeafData->state == rmap_service_setmode_1_0_get_istant) {
                                        // Only istant request LCD or other device
                                        param.systemStatusLock->Take();
                                        // Set data istant value (switch depends from request, istant = sample, Data = Avg.)
                                        param.system_status->data_slave[queueId].data_value[0] = retLeafData->BFT.leaf.val.value;
                                        param.system_status->data_slave[queueId].is_new_ist_data_ready = true;
                                        param.systemStatusLock->Give();
                                    } else if(retLeafData->state == rmap_service_setmode_1_0_get_last) {
                                        // data value id rmap_service_setmode_1_0_get_last into queue SD
                                        // Copy Flag State
                                        bit8Flag = 0;
                                        if(retLeafData->is_adc_unit_error) bit8Flag|=0x01;
                                        if(retLeafData->is_adc_unit_overflow) bit8Flag|=0x02;
                                        param.systemStatusLock->Take();
                                        param.system_status->flags.new_data_to_send = true;
                                        param.system_status->data_slave[queueId].bit8StateFlag = bit8Flag;
                                        param.system_status->data_slave[queueId].byteStateFlag[0] = retLeafData->rbt_event;
                                        param.system_status->data_slave[queueId].byteStateFlag[1] = retLeafData->wdt_event;
                                        param.system_status->data_slave[queueId].byteStateFlag[2] = 0;
                                        param.systemStatusLock->Give();
                                        // Copy Data
                                        memset(&rmap_archive_data, 0, sizeof(rmap_archive_data_t));
                                        // Set Module Type, Date Time as Uint32 GetEpoch_Style, and Block Data Cast to RMAP Type
                                        rmap_archive_data.module_type = clCanard.slave[queueId].get_module_type();
                                        rmap_archive_data.date_time = param.system_status->datetime.epoch_sensors_get_value;
                                        memcpy(rmap_archive_data.block, retLeafData, sizeof(*retLeafData));
                                        // Send queue to SD for direct archive data
                                        // Queue is dimensioned to accept all Data for one step pushing array data (MAX_BOARDS)
                                        // Clean queue if is full to send alwayl the last data on getted value
                                        if(param.dataRmapPutQueue->IsFull()) param.dataLogPutQueue->Dequeue(&rmap_archive_empty);
                                        param.dataRmapPutQueue->Enqueue(&rmap_archive_data, Ticks::MsToTicks(CAN_PUT_QUEUE_RMAP_TIMEOUT_MS));
                                    }
                                    break;

                                case Module_Type::level:
                                    // Cast to th module
                                    retLevelData = (rmap_service_module_RiverLevel_Response_1_0*) clCanard.slave[queueId].rmap_service.get_response();
                                    // data RMAP type is ready to send into queue Archive Data for Saving on SD Memory
                                    // Get parameter data
                                    #if TRACE_LEVEL >= TRACE_INFO
                                    getStimaNameByType(stimaName, clCanard.slave[queueId].get_module_type());
                                    #endif
                                    // Put data in system_status with module_type and RMAP Ver.Rev if not equal (reload or updated)
                                    if((param.system_status->data_slave[queueId].module_version!=retLevelData->version) ||
                                       (param.system_status->data_slave[queueId].module_revision!=retLevelData->revision)) {
                                        param.systemStatusLock->Take();
                                        param.system_status->data_slave[queueId].module_version = retLevelData->version;
                                        param.system_status->data_slave[queueId].module_revision = retLevelData->revision;
                                        // Module type also setted on load config module CAN
                                        param.system_status->data_slave[queueId].module_type = clCanard.slave[queueId].get_module_type();
                                        // Reset flag before recheck exixting firmware available
                                        param.system_status->data_slave[queueId].fw_upgradable = false;
                                        // Check if module can be updated
                                        for(uint8_t checkId=0; checkId<STIMA_MODULE_TYPE_MAX_AVAIABLE; checkId++) {
                                            if(clCanard.slave[queueId].get_module_type() == param.system_status->boards_update_avaiable[checkId].module_type) {
                                                if((param.system_status->boards_update_avaiable[checkId].version > retLevelData->version) ||
                                                    ((param.system_status->boards_update_avaiable[checkId].version == retLevelData->version) && 
                                                    (param.system_status->boards_update_avaiable[checkId].revision > retLevelData->revision))) {
                                                    // Found an upgradable boards
                                                    param.system_status->data_slave[queueId].fw_upgradable = true;
                                                }
                                                break;
                                            }
                                        }
                                        param.systemStatusLock->Give();
                                    }
                                    // TRACE Info data
                                    TRACE_INFO_F(F("RMAP recived response data module from [ %s ], node id: %d. Response code: %d\r\n"),
                                        stimaName, clCanard.slave[queueId].get_node_id(), retLevelData->state);
                                    TRACE_VERBOSE_F(F("Value (LVM) Level %d\r\n"), retLevelData->LVM.river_level.val.value);
                                    // Get security remote state on maintenance mode from relative state flags
                                    param.system_status->data_slave[queueId].maintenance_mode = (retLevelData->state & CAN_FLAG_IS_MAINTENANCE_MODE);
                                    retLevelData->state &= CAN_FLAG_MASK_MAINTENANCE_MODE;
                                    if(param.system_status->data_slave[queueId].maintenance_mode) {
                                        TRACE_INFO_F(F("Warning this module is in maintenance mode!!!\r\n"));
                                    }
                                    // Put istant data in system_status
                                    if(retLevelData->state == rmap_service_setmode_1_0_get_istant) {
                                        // Only istant request LCD or other device
                                        param.systemStatusLock->Take();
                                        // Set data istant value (switch depends from request, istant = sample, Data = Avg.)
                                        param.system_status->data_slave[queueId].data_value[0] = retLevelData->LVM.river_level.val.value;
                                        param.system_status->data_slave[queueId].is_new_ist_data_ready = true;
                                        param.systemStatusLock->Give();
                                    } else if(retLevelData->state == rmap_service_setmode_1_0_get_last) {
                                        // data value id rmap_service_setmode_1_0_get_last into queue SD
                                        // Copy Flag State
                                        bit8Flag = 0;
                                        if(retLevelData->is_adc_unit_error) bit8Flag|=0x01;
                                        if(retLevelData->is_adc_unit_overflow) bit8Flag|=0x02;
                                        param.systemStatusLock->Take();
                                        param.system_status->flags.new_data_to_send = true;
                                        param.system_status->data_slave[queueId].bit8StateFlag = bit8Flag;
                                        param.system_status->data_slave[queueId].byteStateFlag[0] = retLevelData->rbt_event;
                                        param.system_status->data_slave[queueId].byteStateFlag[1] = retLevelData->wdt_event;
                                        param.system_status->data_slave[queueId].byteStateFlag[2] = 0;
                                        param.systemStatusLock->Give();
                                        // Copy Data
                                        memset(&rmap_archive_data, 0, sizeof(rmap_archive_data_t));
                                        // Set Module Type, Date Time as Uint32 GetEpoch_Style, and Block Data Cast to RMAP Type
                                        rmap_archive_data.module_type = clCanard.slave[queueId].get_module_type();
                                        rmap_archive_data.date_time = param.system_status->datetime.epoch_sensors_get_value;
                                        memcpy(rmap_archive_data.block, retLevelData, sizeof(*retLevelData));
                                        // Send queue to SD for direct archive data
                                        // Queue is dimensioned to accept all Data for one step pushing array data (MAX_BOARDS)
                                        // Clean queue if is full to send alwayl the last data on getted value
                                        if(param.dataRmapPutQueue->IsFull()) param.dataLogPutQueue->Dequeue(&rmap_archive_empty);
                                        param.dataRmapPutQueue->Enqueue(&rmap_archive_data, Ticks::MsToTicks(CAN_PUT_QUEUE_RMAP_TIMEOUT_MS));
                                    }
                                    break;

                                case Module_Type::power:
                                    // Cast to th module
                                    retPwrData = (rmap_service_module_Power_Response_1_0*) clCanard.slave[queueId].rmap_service.get_response();
                                    // data RMAP type is ready to send into queue Archive Data for Saving on SD Memory
                                    // Get parameter data
                                    #if TRACE_LEVEL >= TRACE_INFO
                                    getStimaNameByType(stimaName, clCanard.slave[queueId].get_module_type());
                                    #endif
                                    // Put data in system_status with module_type and RMAP Ver.Rev if not equal (reload or updated)
                                    if((param.system_status->data_slave[queueId].module_version!=retPwrData->version) ||
                                       (param.system_status->data_slave[queueId].module_revision!=retPwrData->revision)) {
                                        param.systemStatusLock->Take();
                                        param.system_status->data_slave[queueId].module_version = retPwrData->version;
                                        param.system_status->data_slave[queueId].module_revision = retPwrData->revision;
                                        // Module type also setted on load config module CAN
                                        param.system_status->data_slave[queueId].module_type = clCanard.slave[queueId].get_module_type();
                                        // Reset flag before recheck exixting firmware available
                                        param.system_status->data_slave[queueId].fw_upgradable = false;
                                        // Check if module can be updated
                                        for(uint8_t checkId=0; checkId<STIMA_MODULE_TYPE_MAX_AVAIABLE; checkId++) {
                                            if(clCanard.slave[queueId].get_module_type() == param.system_status->boards_update_avaiable[checkId].module_type) {
                                                if((param.system_status->boards_update_avaiable[checkId].version > retPwrData->version) ||
                                                    ((param.system_status->boards_update_avaiable[checkId].version == retPwrData->version) && 
                                                    (param.system_status->boards_update_avaiable[checkId].revision > retPwrData->revision))) {
                                                    // Found an upgradable boards
                                                    param.system_status->data_slave[queueId].fw_upgradable = true;
                                                }
                                                break;
                                            }
                                        }
                                        param.systemStatusLock->Give();
                                    }
                                    // TRACE Info data
                                    TRACE_INFO_F(F("RMAP recived response data module from [ %s ], node id: %d. Response code: %d\r\n"),
                                        stimaName, clCanard.slave[queueId].get_node_id(), retPwrData->state);
                                    TRACE_VERBOSE_F(F("Value (MPP) Batt Chg. %d, In V. %d, Batt Curr. %d\r\n"),
                                        retPwrData->MPP.battery_charge.val.value, retPwrData->MPP.input_voltage.val.value, retPwrData->MPP.battery_current.val.value);
                                    // Get security remote state on maintenance mode from relative state flags
                                    param.system_status->data_slave[queueId].maintenance_mode = (retPwrData->state & CAN_FLAG_IS_MAINTENANCE_MODE);
                                    retPwrData->state &= CAN_FLAG_MASK_MAINTENANCE_MODE;
                                    if(param.system_status->data_slave[queueId].maintenance_mode) {
                                        TRACE_INFO_F(F("Warning this module is in maintenance mode!!!\r\n"));
                                    }
                                    // Put istant data in system_status
                                    if(retPwrData->state == rmap_service_setmode_1_0_get_istant) {
                                        // Only istant request LCD or other device
                                        param.systemStatusLock->Take();
                                        param.system_status->data_slave[queueId].data_value[0] = retPwrData->MPP.battery_charge.val.value;
                                        param.system_status->data_slave[queueId].data_value[1] = retPwrData->MPP.input_voltage.val.value;
                                        param.system_status->data_slave[queueId].data_value[2] = retPwrData->MPP.battery_current.val.value;
                                        param.system_status->data_slave[queueId].is_new_ist_data_ready = true;
                                        param.systemStatusLock->Give();
                                    } else if(retPwrData->state == rmap_service_setmode_1_0_get_last) {
                                        // data value id rmap_service_setmode_1_0_get_last into queue SD
                                        // Copy Flag State ( Set only Warning Power to Send at Server RMAP )
                                        bit8Flag = 0;
                                        if(retPwrData->is_ltc_unit_error) bit8Flag|=0x0001;
                                        if(retPwrData->is_power_warning) bit8Flag|=0x0002;
                                        param.systemStatusLock->Take();
                                        param.system_status->flags.new_data_to_send = true;
                                        // Copy critical and warning power flag (Use only critical for internal purpose)
                                        param.system_status->flags.power_warning = retPwrData->is_power_warning;
                                        param.system_status->flags.power_critical = retPwrData->is_power_critical;
                                        // Remove critical power flag if error measure occurs: prevent error sending data (disable modem when critical power)
                                        // This command don't remove flags sending to RMAP Server
                                        if((retPwrData->MPP.battery_charge.val.value == 0) || (retPwrData->MPP.battery_charge.val.value > rmap_tableb_B25192_1_0_MAX)) {
                                            param.system_status->flags.power_critical = false;
                                        }
                                        param.system_status->data_slave[queueId].bit8StateFlag = bit8Flag;
                                        param.system_status->data_slave[queueId].byteStateFlag[0] = retPwrData->rbt_event;
                                        param.system_status->data_slave[queueId].byteStateFlag[1] = retPwrData->wdt_event;
                                        param.system_status->data_slave[queueId].byteStateFlag[2] = 0;
                                        param.systemStatusLock->Give();
                                        // Copy Data
                                        memset(&rmap_archive_data, 0, sizeof(rmap_archive_data_t));
                                        // Set Module Type, Date Time as Uint32 GetEpoch_Style, and Block Data Cast to RMAP Type
                                        rmap_archive_data.module_type = clCanard.slave[queueId].get_module_type();
                                        rmap_archive_data.date_time = param.system_status->datetime.epoch_sensors_get_value;
                                        memcpy(rmap_archive_data.block, retPwrData, sizeof(*retPwrData));
                                        // Send queue to SD for direct archive data
                                        // Queue is dimensioned to accept all Data for one step pushing array data (MAX_BOARDS)
                                        // Clean queue if is full to send alwayl the last data on getted value
                                        if(param.dataRmapPutQueue->IsFull()) param.dataLogPutQueue->Dequeue(&rmap_archive_empty);
                                        param.dataRmapPutQueue->Enqueue(&rmap_archive_data, Ticks::MsToTicks(CAN_PUT_QUEUE_RMAP_TIMEOUT_MS));
                                    }
                                    break;

                                case Module_Type::vwc:
                                    // Cast to th module
                                    retVwcData = (rmap_service_module_VWC_Response_1_0*) clCanard.slave[queueId].rmap_service.get_response();
                                    // data RMAP type is ready to send into queue Archive Data for Saving on SD Memory
                                    // Get parameter data
                                    #if TRACE_LEVEL >= TRACE_INFO
                                    getStimaNameByType(stimaName, clCanard.slave[queueId].get_module_type());
                                    #endif
                                    // Put data in system_status with module_type and RMAP Ver.Rev if not equal (reload or updated)
                                    if((param.system_status->data_slave[queueId].module_version!=retVwcData->version) ||
                                       (param.system_status->data_slave[queueId].module_revision!=retVwcData->revision)) {
                                        param.systemStatusLock->Take();
                                        param.system_status->data_slave[queueId].module_version = retVwcData->version;
                                        param.system_status->data_slave[queueId].module_revision = retVwcData->revision;
                                        // Module type also setted on load config module CAN
                                        param.system_status->data_slave[queueId].module_type = clCanard.slave[queueId].get_module_type();
                                        // Reset flag before recheck exixting firmware available
                                        param.system_status->data_slave[queueId].fw_upgradable = false;
                                        // Check if module can be updated
                                        for(uint8_t checkId=0; checkId<STIMA_MODULE_TYPE_MAX_AVAIABLE; checkId++) {
                                            if(clCanard.slave[queueId].get_module_type() == param.system_status->boards_update_avaiable[checkId].module_type) {
                                                if((param.system_status->boards_update_avaiable[checkId].version > retVwcData->version) ||
                                                    ((param.system_status->boards_update_avaiable[checkId].version == retVwcData->version) && 
                                                    (param.system_status->boards_update_avaiable[checkId].revision > retVwcData->revision))) {
                                                    // Found an upgradable boards
                                                    param.system_status->data_slave[queueId].fw_upgradable = true;
                                                }
                                                break;
                                            }
                                        }
                                        param.systemStatusLock->Give();
                                    }
                                    // TRACE Info data
                                    TRACE_INFO_F(F("RMAP recived response data module from [ %s ], node id: %d. Response code: %d\r\n"),
                                        stimaName, clCanard.slave[queueId].get_node_id(), retVwcData->state);
                                    TRACE_VERBOSE_F(F("Value (VWC) Soil moisture 1,2,3 [ %d, %d, %d ]\r\n"), retVwcData->VWC1.vwc.val.value, retVwcData->VWC2.vwc.val.value, retVwcData->VWC3.vwc.val.value);
                                    // Get security remote state on maintenance mode from relative state flags
                                    param.system_status->data_slave[queueId].maintenance_mode = (retVwcData->state & CAN_FLAG_IS_MAINTENANCE_MODE);
                                    retVwcData->state &= CAN_FLAG_MASK_MAINTENANCE_MODE;
                                    if(param.system_status->data_slave[queueId].maintenance_mode) {
                                        TRACE_INFO_F(F("Warning this module is in maintenance mode!!!\r\n"));
                                    }
                                    // Put istant data in system_status
                                    if(retVwcData->state == rmap_service_setmode_1_0_get_istant) {
                                        // Only istant request LCD or other device
                                        param.systemStatusLock->Take();
                                        // Set data istant value (switch depends from request, istant = sample, Data = Avg.)
                                        param.system_status->data_slave[queueId].data_value[0] = retVwcData->VWC1.vwc.val.value;
                                        param.system_status->data_slave[queueId].data_value[1] = retVwcData->VWC2.vwc.val.value;
                                        param.system_status->data_slave[queueId].data_value[2] = retVwcData->VWC3.vwc.val.value;
                                        param.system_status->data_slave[queueId].is_new_ist_data_ready = true;
                                        param.systemStatusLock->Give();
                                    } else if(retVwcData->state == rmap_service_setmode_1_0_get_last) {
                                        // data value id rmap_service_setmode_1_0_get_last into queue SD
                                        // Copy Flag State
                                        bit8Flag = 0;
                                        if(retVwcData->is_adc_unit_error) bit8Flag|=0x01;
                                        if(retVwcData->is_adc_unit_overflow) bit8Flag|=0x02;
                                        param.systemStatusLock->Take();
                                        param.system_status->flags.new_data_to_send = true;
                                        param.system_status->data_slave[queueId].bit8StateFlag = bit8Flag;
                                        param.system_status->data_slave[queueId].byteStateFlag[0] = retVwcData->rbt_event;
                                        param.system_status->data_slave[queueId].byteStateFlag[1] = retVwcData->wdt_event;
                                        param.system_status->data_slave[queueId].byteStateFlag[2] = 0;
                                        param.systemStatusLock->Give();
                                        // Copy Data
                                        memset(&rmap_archive_data, 0, sizeof(rmap_archive_data_t));
                                        // Set Module Type, Date Time as Uint32 GetEpoch_Style, and Block Data Cast to RMAP Type
                                        rmap_archive_data.module_type = clCanard.slave[queueId].get_module_type();
                                        rmap_archive_data.date_time = param.system_status->datetime.epoch_sensors_get_value;
                                        memcpy(rmap_archive_data.block, retVwcData, sizeof(*retVwcData));
                                        // Send queue to SD for direct archive data
                                        // Queue is dimensioned to accept all Data for one step pushing array data (MAX_BOARDS)
                                        // Clean queue if is full to send alwayl the last data on getted value
                                        if(param.dataRmapPutQueue->IsFull()) param.dataLogPutQueue->Dequeue(&rmap_archive_empty);
                                        param.dataRmapPutQueue->Enqueue(&rmap_archive_data, Ticks::MsToTicks(CAN_PUT_QUEUE_RMAP_TIMEOUT_MS));
                                    }
                                    break;

                                default:
                                    // data RMAP type is ready to send into queue Archive Data for Saving on SD Memory
                                    TRACE_INFO_F(F("RMAP recived response service data module from unknown module node id: [ %d ]\r\n"),
                                        clCanard.slave[queueId].get_node_id());
                                    break;
                            }
                        }
                    }

                    // STOP Server request data RMAP
                    if (rmapServerEnd) {
                        param.systemStatusLock->Take();
                        param.system_status->flags.rmap_server_running = false;
                        param.systemStatusLock->Give();
                    }
                } else {
                    // Security off pending status
                    for(uint8_t rmap_server_queueId=0; rmap_server_queueId<MAX_NODE_CONNECT; rmap_server_queueId++) {
                        clCanard.slave[rmap_server_queueId].rmap_service.reset_pending();
                    }
                }
                // ********************* END RMAP GETDATA TX-> RX<- **********************

                // ***********************************************************************
                // ************* REMOTE RPC UAVCAN REQUEST FUNCTION SERVER ***************
                // ***********************************************************************
                // Get coda comandi da system_message... se richiesto comando da LCD o RPC Remota
                // Da inoltrare al nodo selezionato in coda, parametro
                if(!param.systemMessageQueue->IsEmpty()) {
                    // Message queue is for CAN (If FW Upgrade local Master, Message is for SD...)
                    if(param.systemMessageQueue->Peek(&system_message, 0)) {
                        // Only local task post message queue can be processed here
                        // Messages checked here can only be addressed to remote slaves. So the addresses must be in the valid area
                        if((system_message.task_dest == LOCAL_TASK_ID) && (system_message.node_id < MAX_NODE_CONNECT)) {
                            // ***************************** Command *****************************
                            // Procedure can continue only without other command in pending state
                            // If command not starting, time_out remove the command and queue can be free from item
                            if(!clCanard.slave[system_message.node_id].command.is_pending()) {
                                // ENTER MAINTENANCE
                                if(system_message.command.do_maint) {
                                    // Start Flag Event Start when request configuration is request
                                    // When remote node recive VSC from Master Heartbeat Remote slave FullPower is performed
                                    // Then new state for slave (fullpower) are resend to master. If Ok procedure can start 
                                    if(clCanard.slave[system_message.node_id].heartbeat.get_power_mode() == Power_Mode::pwr_on) {                                
                                        // Remove message from the queue
                                        param.systemMessageQueue->Dequeue(&system_message);
                                        TRACE_INFO_F(F("Command server: Send request maintenance mode at Node: [ %d ]\r\n"), clCanard.slave[system_message.node_id].get_node_id());
                                        // Request start module maintenance from LCD or Remote RPC with Param 0/1
                                        char do_maint = 1;
                                        clCanard.send_command_pending(system_message.node_id, NODE_COMMAND_TIMEOUT_US,                            
                                            canardClass::Command_Private::module_maintenance, &do_maint, sizeof(do_maint));                            
                                        // Starting message server
                                        param.systemStatusLock->Take();
                                        param.system_status->flags.cmd_server_running = true;
                                        param.systemStatusLock->Give();
                                    } else {
                                        // IS NEED to Request FullPower Mode for type of command
                                        if(!param.system_status->flags.full_wakeup_request) {
                                            TRACE_VERBOSE_F(F("Command server: Start full power for sending command at node: [ %d ]"), clCanard.slave[system_message.node_id].get_node_id());
                                            param.systemStatusLock->Take();
                                            param.system_status->flags.full_wakeup_request = true;
                                            param.systemStatusLock->Give();
                                        }
                                    }
                                }
                                // UNDO MAINTENANCE
                                if(system_message.command.undo_maint) {
                                    // Start Flag Event Start when request configuration is request
                                    // When remote node recive VSC from Master Heartbeat Remote slave FullPower is performed
                                    // Then new state for slave (fullpower) are resend to master. If Ok procedure can start 
                                    if(clCanard.slave[system_message.node_id].heartbeat.get_power_mode() == Power_Mode::pwr_on) {
                                        // Remove message from the queue
                                        param.systemMessageQueue->Dequeue(&system_message);
                                        TRACE_INFO_F(F("Command server: Send remove maintenance mode at Node: [ %d ]"), clCanard.slave[system_message.node_id].get_node_id());
                                        // Request start module maintenance from LCD or Remote RPC with Param 0/1
                                        char undo_maint = 0;
                                        clCanard.send_command_pending(system_message.node_id, NODE_COMMAND_TIMEOUT_US,                            
                                            canardClass::Command_Private::module_maintenance, &undo_maint, sizeof(undo_maint));                            
                                        // Starting message server
                                        param.systemStatusLock->Take();
                                        param.system_status->flags.cmd_server_running = true;
                                        param.systemStatusLock->Give();
                                    } else {
                                        // IS NEED to Request FullPower Mode for type of command
                                        if(!param.system_status->flags.full_wakeup_request) {
                                            TRACE_VERBOSE_F(F("Command server: Start full power for sending command at node: [ %d ]"), clCanard.slave[system_message.node_id].get_node_id());
                                            param.systemStatusLock->Take();
                                            param.system_status->flags.full_wakeup_request = true;
                                            param.systemStatusLock->Give();
                                        }
                                    }
                                }
                                // STARTING CALIBRATION (Accelerometer)
                                if(system_message.command.do_calib_acc) {
                                    // Start Flag Event Start when request configuration is request
                                    // When remote node recive VSC from Master Heartbeat Remote slave FullPower is performed
                                    // Then new state for slave (fullpower) are resend to master. If Ok procedure can start 
                                    if(clCanard.slave[system_message.node_id].heartbeat.get_power_mode() == Power_Mode::pwr_on) {                                // Remove message from the queue
                                        // Remove message from the queue
                                        param.systemMessageQueue->Dequeue(&system_message);
                                        TRACE_INFO_F(F("Command server: Send request calibration accelerometer at Node: [ %d ]\r\n"), clCanard.slave[system_message.node_id].get_node_id());
                                        // Requestcalibration accellerometer from LCD or Remote RPC without param
                                        clCanard.send_command_pending(system_message.node_id, NODE_COMMAND_TIMEOUT_US,                            
                                            canardClass::Command_Private::calibrate_accelerometer, NULL, 0);
                                        // Starting message server
                                        param.systemStatusLock->Take();
                                        param.system_status->flags.cmd_server_running = true;
                                        param.systemStatusLock->Give();
                                    } else {
                                        // IS NEED to Request FullPower Mode for type of command
                                        if(!param.system_status->flags.full_wakeup_request) {
                                            TRACE_VERBOSE_F(F("Command server: Start full power for sending command at node: [ %d ]"), clCanard.slave[system_message.node_id].get_node_id());
                                            param.systemStatusLock->Take();
                                            param.system_status->flags.full_wakeup_request = true;
                                            param.systemStatusLock->Give();
                                        }
                                    }
                                }
                                // DO FACTORY (Remote register clear -> init register and node restart from PnP request...)
                                if(system_message.command.do_factory) {
                                    // Start Flag Event Start when request configuration is request
                                    // When remote node recive VSC from Master Heartbeat Remote slave FullPower is performed
                                    // Then new state for slave (fullpower) are resend to master. If Ok procedure can start 
                                    if(clCanard.slave[system_message.node_id].heartbeat.get_power_mode() == Power_Mode::pwr_on) {                                // Remove message from the queue
                                        // Remove message from the queue
                                        param.systemMessageQueue->Dequeue(&system_message);
                                        TRACE_INFO_F(F("Command server: Send request calibration accelerometer at Node: [ %d ]\r\n"), clCanard.slave[system_message.node_id].get_node_id());
                                        // Requestcalibration accellerometer from LCD or Remote RPC without param
                                        clCanard.send_command_pending(system_message.node_id, NODE_COMMAND_TIMEOUT_US,                            
                                            uavcan_node_ExecuteCommand_Request_1_1_COMMAND_FACTORY_RESET, NULL, 0);
                                        // Starting message server
                                        param.systemStatusLock->Take();
                                        param.system_status->flags.cmd_server_running = true;
                                        param.systemStatusLock->Give();
                                    } else {
                                        // IS NEED to Request FullPower Mode for type of command
                                        if(!param.system_status->flags.full_wakeup_request) {
                                            TRACE_VERBOSE_F(F("Command server: Start full power for sending command at node: [ %d ]"), clCanard.slave[system_message.node_id].get_node_id());
                                            param.systemStatusLock->Take();
                                            param.system_status->flags.full_wakeup_request = true;
                                            param.systemStatusLock->Give();
                                        }
                                    }
                                }
                                // RESET FLAGS (All module slave)
                                if(system_message.command.do_reset_flags) {
                                    // Start Flag Event Start when request configuration is request
                                    // When remote node recive VSC from Master Heartbeat Remote slave FullPower is performed
                                    // Then new state for slave (fullpower) are resend to master. If Ok procedure can start 
                                    if(clCanard.slave[system_message.node_id].heartbeat.get_power_mode() == Power_Mode::pwr_on) {                                
                                        // Remove message from the queue
                                        param.systemMessageQueue->Dequeue(&system_message);
                                        TRACE_INFO_F(F("Command server: Send request init flags at Node: [ %d ]\r\n"), clCanard.slave[system_message.node_id].get_node_id());
                                        // Request calibration accellerometer from LCD or Remote RPC without param
                                        clCanard.send_command_pending(system_message.node_id, NODE_COMMAND_TIMEOUT_US,                            
                                            canardClass::Command_Private::reset_flags, NULL, 0);
                                        // Starting message server
                                        param.systemStatusLock->Take();
                                        param.system_status->flags.cmd_server_running = true;
                                        param.systemStatusLock->Give();
                                    } else {
                                        // IS NEED to Request FullPower Mode for type of command
                                        if(!param.system_status->flags.full_wakeup_request) {
                                            TRACE_VERBOSE_F(F("Command server: Start full power for sending command at node: [ %d ]"), clCanard.slave[system_message.node_id].get_node_id());
                                            param.systemStatusLock->Take();
                                            param.system_status->flags.full_wakeup_request = true;
                                            param.systemStatusLock->Give();
                                        }
                                    }
                                }
                                // RESET FLAGS (All module slave)
                                if(system_message.command.do_reboot_node) {
                                    // Start Flag Event Start when request configuration is request
                                    // When remote node recive VSC from Master Heartbeat Remote slave FullPower is performed
                                    // Then new state for slave (fullpower) are resend to master. If Ok procedure can start 
                                    if(clCanard.slave[system_message.node_id].heartbeat.get_power_mode() == Power_Mode::pwr_on) {                                
                                        // Remove message from the queue
                                        param.systemMessageQueue->Dequeue(&system_message);
                                        TRACE_INFO_F(F("Command server: Send direct reboot at Node: [ %d ]\r\n"), clCanard.slave[system_message.node_id].get_node_id());
                                        // Direct reboot from LCD or Remote RPC without param
                                        clCanard.send_command(clCanard.slave[system_message.node_id].get_node_id(), NODE_COMMAND_TIMEOUT_US,                            
                                            uavcan_node_ExecuteCommand_Request_1_1_COMMAND_RESTART, NULL, 0);       
                                        // No required Starting message server on Reboot Command
                                    } else {
                                        // IS NEED to Request FullPower Mode for type of command
                                        if(!param.system_status->flags.full_wakeup_request) {
                                            TRACE_VERBOSE_F(F("Command server: Start full power for sending command at node: [ %d ]"), clCanard.slave[system_message.node_id].get_node_id());
                                            param.systemStatusLock->Take();
                                            param.system_status->flags.full_wakeup_request = true;
                                            param.systemStatusLock->Give();
                                        }
                                    }
                                }
                            }
                            // ************************* Register access *************************
                            // Procedure can continue only without other register in pending state
                            // If register not starting, time_out remove the set register and queue can be free from item
                            if(!clCanard.slave[system_message.node_id].register_access.is_pending()) {
                                // REMOTE REGISTER Modify (All module slave)
                                if(system_message.command.do_remote_reg) {
                                    // Start Flag Event Start when request configuration is request
                                    // When remote node recive VSC from Master Heartbeat Remote slave FullPower is performed
                                    // Then new state for slave (fullpower) are resend to master. If Ok procedure can start 
                                    if(clCanard.slave[system_message.node_id].heartbeat.get_power_mode() == Power_Mode::pwr_on) {                                
                                        // Remove message from the queue
                                        param.systemMessageQueue->Dequeue(&system_message);
                                        TRACE_INFO_F(F("Register server: Send register modify at Node: [ %d ]\r\n"), clCanard.slave[system_message.node_id].get_node_id());
                                        // Send register value (Set type from Uavcan Type register selection)
                                        // uavcan_register select and set value (default is empty)
                                        uavcan_register_Value_1_0 rpcRegisterValue = {0};
                                        uavcan_register_Value_1_0_select_empty_(&rpcRegisterValue);
                                        switch (system_message.param) {
                                            case RVS_TYPE_BIT:
                                                uavcan_register_Value_1_0_select_bit_(&rpcRegisterValue);
                                                rpcRegisterValue.bit.value.count = 1;
                                                rpcRegisterValue.bit.value.bitpacked[0] = system_message.value.bool_val;
                                                break;
                                            case RVS_TYPE_INTEGER_8:
                                                uavcan_register_Value_1_0_select_integer8_(&rpcRegisterValue);
                                                rpcRegisterValue.integer8.value.count = 1;
                                                rpcRegisterValue.integer8.value.elements[0] = system_message.value.int8_val;
                                                break;
                                            case RVS_TYPE_INTEGER_16:
                                                uavcan_register_Value_1_0_select_integer16_(&rpcRegisterValue);
                                                rpcRegisterValue.integer16.value.count = 1;
                                                rpcRegisterValue.integer16.value.elements[0] = system_message.value.int16_val;
                                                break;
                                            case RVS_TYPE_INTEGER_32:
                                                uavcan_register_Value_1_0_select_integer32_(&rpcRegisterValue);
                                                rpcRegisterValue.integer32.value.count = 1;
                                                rpcRegisterValue.integer32.value.elements[0] = system_message.value.int32_val;
                                                break;
                                            case RVS_TYPE_INTEGER_64:
                                                uavcan_register_Value_1_0_select_integer64_(&rpcRegisterValue);
                                                rpcRegisterValue.integer64.value.count = 1;
                                                rpcRegisterValue.integer64.value.elements[0] = (int64_t)system_message.value.int32_val;
                                                break;
                                            case RVS_TYPE_NATURAL_8:
                                                uavcan_register_Value_1_0_select_natural8_(&rpcRegisterValue);
                                                rpcRegisterValue.natural8.value.count = 1;
                                                rpcRegisterValue.natural8.value.elements[0] = system_message.value.uint8_val;
                                                break;
                                            case RVS_TYPE_NATURAL_16:
                                                uavcan_register_Value_1_0_select_natural16_(&rpcRegisterValue);
                                                rpcRegisterValue.natural16.value.count = 1;
                                                rpcRegisterValue.natural16.value.elements[0] = system_message.value.uint16_val;
                                                break;
                                            case RVS_TYPE_NATURAL_32:
                                                uavcan_register_Value_1_0_select_natural32_(&rpcRegisterValue);
                                                rpcRegisterValue.natural32.value.count = 1;
                                                rpcRegisterValue.natural32.value.elements[0] = system_message.value.uint32_val;
                                                break;
                                            case RVS_TYPE_NATURAL_64:
                                                uavcan_register_Value_1_0_select_natural64_(&rpcRegisterValue);
                                                rpcRegisterValue.natural64.value.count = 1;
                                                rpcRegisterValue.natural64.value.elements[0] = (uint64_t)system_message.value.uint32_val;
                                                break;
                                            case RVS_TYPE_REAL_16:
                                                uavcan_register_Value_1_0_select_real16_(&rpcRegisterValue);
                                                rpcRegisterValue.real16.value.count = 1;
                                                rpcRegisterValue.real16.value.elements[0] = system_message.value.float_val;
                                                break;
                                            case RVS_TYPE_REAL_32:
                                                uavcan_register_Value_1_0_select_real32_(&rpcRegisterValue);
                                                rpcRegisterValue.real32.value.count = 1;
                                                rpcRegisterValue.real32.value.elements[0] = system_message.value.float_val;
                                                break;
                                            case RVS_TYPE_REAL_64:
                                                uavcan_register_Value_1_0_select_real64_(&rpcRegisterValue);
                                                rpcRegisterValue.real64.value.count = 1;
                                                rpcRegisterValue.real64.value.elements[0] = (double)system_message.value.float_val;
                                                break;
                                        }
                                        clCanard.send_register_access_pending(system_message.node_id, NODE_REGISTER_TIMEOUT_US,
                                            system_message.message, rpcRegisterValue, true);
                                        // Starting message server
                                        param.systemStatusLock->Take();
                                        param.system_status->flags.reg_server_running = true;
                                        param.systemStatusLock->Give();
                                    } else {
                                        // IS NEED to Request FullPower Mode for setting register
                                        if(!param.system_status->flags.full_wakeup_request) {
                                            TRACE_VERBOSE_F(F("Register server: Start full power for sending register at node: [ %d ]"), clCanard.slave[system_message.node_id].get_node_id());
                                            param.systemStatusLock->Take();
                                            param.system_status->flags.full_wakeup_request = true;
                                            param.systemStatusLock->Give();
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                // La verifica verrà fatta con il Flag Pending resettato e la risposta viene
                // popolata nel'apposito registro di state service_module del il servizio relativo
                // Il set data avviene in processReciveTranser alle sezioni CanardTransferKindResponse
                // Eventuale Flag TimeOut indica un'avvenuta mancata risposta al comando
                // Il master una volta inviato il comando deve attendere ResetPending o TimeOutCommand
                // Are command server running? Process request or verify END procedure
                if(param.system_status->flags.cmd_server_running) {

                    // Check if file server are current in running state
                    bool cmdServerEnd = true;

                    for(uint8_t cmd_server_queueId=0; cmd_server_queueId<MAX_NODE_CONNECT; cmd_server_queueId++) {

                        // state != standby? Running current procedure...
                        if(clCanard.slave[cmd_server_queueId].command.is_pending())
                            cmdServerEnd = false;

                        if (clCanard.slave[cmd_server_queueId].command.event_timeout()) {
                            // Rimuovo gli stati
                            clCanard.slave[cmd_server_queueId].command.reset_pending();
                            // TimeOUT di un comando in attesa... gestisco il da farsi Retry? Abort? Signal?
                            TRACE_ERROR_F(F("Command server: time out command at Node: [ %d ], Warning [restore pending command]\r\n"),
                                clCanard.slave[system_message.node_id].get_node_id());
                        }
                        if (clCanard.slave[cmd_server_queueId].command.is_executed()) {
                            // Rimuovo gli stati
                            clCanard.slave[cmd_server_queueId].command.reset_pending();
                            // Command OK. Signal?
                            TRACE_INFO_F(F("Command server: confirmed command at Node: [ %d ], response code value: [ %d ]\r\n"),
                                clCanard.slave[system_message.node_id].get_node_id(), clCanard.slave[cmd_server_queueId].command.get_response());
                        }
                    }

                    // End server distribution command
                    if(cmdServerEnd) {
                        param.systemStatusLock->Take();
                        param.system_status->flags.cmd_server_running = false;
                        param.systemStatusLock->Give();
                    }
                } else {
                    // Security off pending status
                    for(uint8_t cmd_server_queueId=0; cmd_server_queueId<MAX_NODE_CONNECT; cmd_server_queueId++) {
                        clCanard.slave[cmd_server_queueId].command.reset_pending();
                    }
                }
                // ************************* END COMMAND SERVER **************************

                // La verifica verrà fatta con il Flag Pending resettato e la risposta viene
                // popolata nel'apposito registro di state service_module del il servizio relativo
                // Il set data avviene in processReciveTranser alle sezioni CanardTransferKindResponse
                // Eventuale Flag TimeOut indica un'avvenuta mancata risposta al comando
                // Il master una volta inviato il registro deve attendere ResetPending o TimeOutCommand
                // Are register server running? Process request or verify END procedure
                // Configure method (list_register_sequence) are prioritary. If running configuration server
                // register server it shouldn't work at all. Only at the end can be completed...
                if((param.system_status->flags.reg_server_running)&&
                    (!param.system_status->flags.cfg_server_running)) {

                    // Check if file server are current in running state
                    bool regServerEnd = true;

                    for(uint8_t reg_server_queueId=0; reg_server_queueId<MAX_NODE_CONNECT; reg_server_queueId++) {

                        // state != standby? Running current procedure...
                        if(clCanard.slave[reg_server_queueId].register_access.is_pending())
                            regServerEnd = false;

                        if (clCanard.slave[reg_server_queueId].register_access.event_timeout()) {
                            // Rimuovo gli stati
                            clCanard.slave[reg_server_queueId].register_access.reset_pending();
                            // TimeOUT di un comando in attesa... gestisco il da farsi Retry? Abort? Signal?
                            TRACE_ERROR_F(F("Register server: time out command at Node: [ %d ], Warning [restore pending command]\r\n"),
                                clCanard.slave[system_message.node_id].get_node_id());
                        }
                        if (clCanard.slave[reg_server_queueId].register_access.is_executed()) {
                            // Continuo la programmazione
                            regServerEnd = false;
                            // Rimuovo gli stati
                            clCanard.slave[reg_server_queueId].register_access.reset_pending();
                            // Command OK.
                            TRACE_INFO_F(F("Register server: confirmed modify value at Node: [ %d ]\r\n"), clCanard.slave[system_message.node_id].get_node_id());
                        }
                    }

                    // End server distribution command
                    if(regServerEnd) {
                        param.systemStatusLock->Take();
                        param.system_status->flags.reg_server_running = false;
                        param.systemStatusLock->Give();
                    }
                } else {
                    // Security off pending status
                    if(!param.system_status->flags.cfg_server_running) {
                        for(uint8_t reg_server_queueId=0; reg_server_queueId<MAX_NODE_CONNECT; reg_server_queueId++) {
                            clCanard.slave[reg_server_queueId].register_access.reset_pending();
                        }
                    }
                }
                // ************************* END REGISTER SERVER *************************


                // ***********************************************************************
                // ****** REMOTE REGISTER GET/SET SERVER AND REMOTE CONFIGURATION  *******
                // ***********************************************************************

                // Avvio configurazione remota automatica su richiesta slave (Appena CONFIGURATO da PNP)
                // La procedura parte solo senza register_server in funzione (un solo avvio ammesso)
                // Si avvia generalmente dopo il PnP o su un modulo parzialmente configurato con solo il node_id
                if((param.system_status->flags.run_module_configure)&&
                    (!param.system_status->flags.cfg_server_running)&&
                    (!param.system_status->flags.do_remote_cfg_exec)) {
                    for(uint8_t cfg_remote_queueId=0; cfg_remote_queueId<MAX_NODE_CONNECT; cfg_remote_queueId++) {
                        // Check Configurazione non attiva su un nodo configurato e online
                        // FALSE se il nodo non è configurato nei reglistri Metadati e Porte RMAP
                        // Determino se avviare la configfurazione di un nodo apprena programmato con PnP...
                        // O comunque di un nodo preventivamente programmato con node_id ma senza metadati
                        if(clCanard.slave[cfg_remote_queueId].is_online()) {
                            // N.B: Se ho appena terminato la procedura HeratBeat potrebbe essere arrivato con Status Update request,
                            // Quindi non eseguo la configurazione, e attendo l'eleimnazione del flag e la rilettura dello stato remoto cfg
                            // Anche nel caso di configurazione già in corso non processo il comando. Esco e attendo la fine del PNP/CFG
                            if((!clCanard.slave[cfg_remote_queueId].heartbeat.get_module_ready()) &&
                                (!remote_configure_reboot[cfg_remote_queueId]) &&
                                (!remote_configure[cfg_remote_queueId])) {
                                // Module not configured.
                                // Starting configuration procedure with push command to queue
                                // Queue gestion power UP, starting command ecc... is performed
                                // After terminate procedure and reboot remote Node. Configuration is ready
                                system_message_t system_message = {0};
                                system_message.task_dest = CAN_TASK_ID;
                                system_message.command.do_remote_cfg = true;
                                system_message.node_id = cfg_remote_queueId;
                                param.systemMessageQueue->Enqueue(&system_message, 0);
                                param.systemStatusLock->Take();
                                param.system_status->flags.do_remote_cfg_exec = true;
                                param.systemStatusLock->Give();
                            }
                        }
                    }
                }

                // Get coda config register da system_message... se richiesto comando da LCD, RPC Remota, PNP
                // Avvia la configurazione remota con la sequenza dei registri da programmare
                // LA configurazione può essere avviato solo con nodo già configurato almeno con port_id Valido
                // Evidentemente un nodo non ancora assegnato può essere configurato solo dopo assegnamento PNP del port_id
                // Al termine del PnP port_id, deve essere avviata sempre la procedura di configurazione del nodo remoto
                if(!param.systemMessageQueue->IsEmpty()) {
                    // Message queue is for CAN (If FW Upgrade local Master, Message is for SD...)
                    if(param.systemMessageQueue->Peek(&system_message, 0)) {
                        // Only local task post message queue can be processed here
                        // Messages checked here can only be addressed to remote slaves. So the addresses must be in the valid area
                        if((system_message.task_dest == LOCAL_TASK_ID) && (system_message.node_id < MAX_NODE_CONNECT)) {
                            // ENTER PROCEDURE CONFIG (Only Full POWERED Module!!!)
                            if(system_message.command.do_remote_cfg) {
                                // Try to configure and Waiting OnLine ( If already On Line nothing todo )
                                clCanard.slave[system_message.node_id].configure(
                                    param.configuration->board_slave[system_message.node_id].can_address,
                                    param.configuration->board_slave[system_message.node_id].module_type,
                                    param.configuration->board_slave[system_message.node_id].can_port_id,
                                    param.configuration->board_slave[system_message.node_id].can_publish_id,
                                    param.configuration->board_slave[system_message.node_id].serial_number);
                                // If node is not online, remove command from the queue... Configuration is impossible
                                // Queue must to be free !!!
                                if(!clCanard.slave[system_message.node_id].is_online()) {
                                    if(!remote_configure_wait_online_ms[system_message.node_id]) {
                                        remote_configure_wait_online_ms[system_message.node_id] = millis();
                                    } else {
                                        // Configuration impossible Module not Found OnLine after 4 sec from New Configuration
                                        if(millis() - remote_configure_wait_online_ms[system_message.node_id] > 4000) {
                                            // Remove message from the queue (No more action possible here NOT Online)
                                            param.systemMessageQueue->Dequeue(&system_message);
                                        }
                                    }
                                } else {
                                    // Start Flag Event Start when request configuration is request
                                    // When remote node recive VSC from Master Heartbeat Remote slave FullPower is performed
                                    // Then new state for slave (fullpower) are resend to master. If Ok procedure can start 
                                    if(clCanard.slave[system_message.node_id].heartbeat.get_power_mode() == Power_Mode::pwr_on) {
                                        // Remove message from the queue (ONLY IF REMOTE NODE IS FULL POWERED!!!)
                                        remote_configure_wait_online_ms[system_message.node_id] = 0;
                                        param.systemMessageQueue->Dequeue(&system_message);
                                        if(clCanard.slave[system_message.node_id].get_node_id() <= CANARD_NODE_ID_MAX) {
                                            TRACE_INFO_F(F("Register server: Modify configuration at already configured module stimacan: [ %d ], current node id [ %d ]\r\n"), system_message.node_id + 1, clCanard.slave[system_message.node_id].get_node_id());
                                        } else {
                                            TRACE_INFO_F(F("Register server: Start configuration at new module stimacan: [ %d ]\r\n"), system_message.node_id + 1);
                                        }
                                        if(clCanard.slave[system_message.node_id].is_online()) {
                                            // START Remote configuration of Node -> system_message.node_id
                                            remote_configure[system_message.node_id] = REGISTER_STARTING;
                                            remote_configure_retry[system_message.node_id] = NODE_REGISTER_MAX_RETRY;
                                            param.systemStatusLock->Take();
                                            param.system_status->flags.cfg_server_running = true;
                                            param.systemStatusLock->Give();
                                        } else {
                                            if(clCanard.slave[system_message.node_id].get_node_id() <= CANARD_NODE_ID_MAX) {
                                                // Off line ... Not configure?
                                                TRACE_INFO_F(F("Register server: ALERT stimacan: [ %d ], node id [ %d ] is OFF LINE. Node cannot be configured [ %s ]\r\n"), system_message.node_id + 1, clCanard.slave[system_message.node_id].get_node_id(), ABORT_STRING);
                                            } else {
                                                // not configured yet (waitinq request PNP) ?
                                                TRACE_INFO_F(F("Register server: PNP save parameter for module stimacan: [ %d ]. Configuration is ready for remote PNP request.\r\n"),system_message.node_id + 1);
                                            }
                                        }
                                    } else {
                                        // IS NEED to Request FullPower Mode for type of command (if not yet request full power)
                                        if(!param.system_status->flags.full_wakeup_request) {
                                            TRACE_VERBOSE_F(F("Configuration module: Start full power for sending queue of command configuration to slave, old power state: [ %d ]\r\n"), clCanard.slave[system_message.node_id].heartbeat.get_power_mode());
                                            param.systemStatusLock->Take();
                                            param.system_status->flags.full_wakeup_request = true;
                                            param.systemStatusLock->Give();
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                // Are configuration remote node in execution?
                if(param.system_status->flags.cfg_server_running) {
                    // Check end of configure remote module
                    bool cfgConfigureEnd = true;
                    uint8_t sensorCount = 0;
                    bool reCheckEndEvent = false; // True if end of one programming (recheck back end of server register running)
                    // loop for all Node and switching from list command sequence.
                    // Create and send register command and wait progression in server command procedure
                    for(uint8_t cfg_remote_queueId=0; cfg_remote_queueId<MAX_NODE_CONNECT; cfg_remote_queueId++) {
                        // Are configuration in progress for the node. Check type and sensor count for all parameter
                        // Node request to be online
                        if(remote_configure[cfg_remote_queueId]) {
                            // Configure module is in running mode
                            cfgConfigureEnd = false;
                            if(clCanard.slave[cfg_remote_queueId].is_online()) {
                                // Start comand only start without old pending comand                            
                                if(clCanard.slave[cfg_remote_queueId].register_access.is_pending()) {
                                    // Pending command is active, Waiting Server Register elaborate Response or TimeOUT
                                    break;
                                } else {
                                    // Starting or continue procedure configuration STEP by STEP
                                    switch(param.configuration->board_slave[cfg_remote_queueId].module_type) {
                                        // Prepare sensor count to send limit metadata to remote register for remote module type
                                    // starting "type"... verified with starting sensorId = SETUP_ID for correct sequence command
                                    case Module_Type::th:
                                        sensorCount = SENSOR_METADATA_TH_COUNT;
                                        break;
                                    case Module_Type::rain:
                                        sensorCount = SENSOR_METADATA_RAIN_COUNT;
                                        break;
                                    case Module_Type::wind:
                                        sensorCount = SENSOR_METADATA_WIND_COUNT;
                                        break;
                                    case Module_Type::radiation:
                                        sensorCount = SENSOR_METADATA_RADIATION_COUNT;
                                        break;
                                    case Module_Type::leaf:
                                        sensorCount = SENSOR_METADATA_LEAF_COUNT;
                                        break;
                                    case Module_Type::level:
                                        sensorCount = SENSOR_METADATA_LEVEL_COUNT;
                                        break;
                                    case Module_Type::power:
                                        sensorCount = SENSOR_METADATA_POWER_COUNT;
                                        break;
                                    case Module_Type::vwc:
                                        sensorCount = SENSOR_METADATA_VWC_COUNT;
                                        break;
                                    }
                                    // *******************************************
                                    // **** STEP -> CONFIGURATION PROGRESSION ****
                                    // *******************************************
                                    // RETURN NEXT STEP OR END FROM INTERPRET COMMAND EXECUTING
                                    // x SPECIFICHE UAVCAN ->
                                    // NB Il tipo di registro deve essere == (es. Natural32) e deve esistere sul nodo Remoto !!!
                                    // Altrimenti la funzione deve fallire e ritornare NULL
                                    // Quindi il Master deve conoscere la tipologia di registro e nome dello SLAVE
                                    // Non è possibile creare un registro senza uscire dalle specifiche (es. comando vendor_specific)
                                    // Preparo il registro (Inifluente se l'operazione è di sola lettura parametro -> write)
                                    // RESPONSE -> AFTER REQUEST AND VERIFY:
                                    // La verifica verrà fatta con il Flag Pending resettato e la risposta viene
                                    // popolata nel'apposito registro di state service_module del il servizio relativo
                                    // Il set data avviene in processReciveTranser alle sezioni CanardTransferKindResponse
                                    // Eventuale Flag TimeOut indica un'avvenuta mancata risposta al comando
                                    // Il master una volta inviato il comando deve attendere ResetPending o TimeOutCommand
                                    // Reset register do void (security check TX/RX Value )
                                    memset(&val, 0, sizeof(uavcan_register_Value_1_0));
                                    switch (remote_configure[cfg_remote_queueId])
                                    {
                                    case REGISTER_01_SEND:
                                        // Register-01 ( Configure Metadata register LEVEL.L1 )
                                        uavcan_register_Value_1_0_select_natural16_(&val);
                                        val.natural16.value.count = sensorCount;
                                        for(uint8_t id=0; id<sensorCount; id++) {
                                            val.natural16.value.elements[id] = param.configuration->board_slave[cfg_remote_queueId].metadata[id].level1;
                                        }
                                        // Send register value to Slave Remote with parameter to store
                                        clCanard.send_register_access_pending(cfg_remote_queueId, NODE_REGISTER_TIMEOUT_US,
                                            REGISTER_METADATA_LEVEL_L1, val, NODE_REGISTER_WRITING);
                                        TRACE_VERBOSE_F(F("Register server: Send %s at Node: [ %d ]\r\n"), REGISTER_METADATA_LEVEL_L1, clCanard.slave[cfg_remote_queueId].get_node_id());
                                        // Prepare verify RESPONSE Method OK.
                                        remote_configure[cfg_remote_queueId]++;
                                        break;
                                    case REGISTER_02_SEND:
                                        // Register-02 ( Configure Metadata register LEVEL.L2 )
                                        uavcan_register_Value_1_0_select_natural16_(&val);
                                        val.natural16.value.count = sensorCount;
                                        for(uint8_t id=0; id<sensorCount; id++) {
                                            val.natural16.value.elements[id] = param.configuration->board_slave[cfg_remote_queueId].metadata[id].level2;
                                        }
                                        // Send register value to Slave Remote with parameter to store
                                        clCanard.send_register_access_pending(cfg_remote_queueId, NODE_REGISTER_TIMEOUT_US,
                                            REGISTER_METADATA_LEVEL_L2, val, NODE_REGISTER_MAX_RETRY);
                                        TRACE_VERBOSE_F(F("Register server: Send %s at Node: [ %d ]\r\n"), REGISTER_METADATA_LEVEL_L2, clCanard.slave[cfg_remote_queueId].get_node_id());
                                        // Prepare verify RESPONSE Method OK.
                                        remote_configure[cfg_remote_queueId]++;
                                        break;
                                    case REGISTER_03_SEND:
                                        // Register-03 ( Configure Metadata register LEVEL_TYPE1 )
                                        uavcan_register_Value_1_0_select_natural16_(&val);
                                        val.natural16.value.count = sensorCount;
                                        for(uint8_t id=0; id<sensorCount; id++) {
                                            val.natural16.value.elements[id] = param.configuration->board_slave[cfg_remote_queueId].metadata[id].levelType1;
                                        }
                                        // Send register value to Slave Remote with parameter to store
                                        clCanard.send_register_access_pending(cfg_remote_queueId, NODE_REGISTER_TIMEOUT_US,
                                            REGISTER_METADATA_LEVEL_TYPE1, val, NODE_REGISTER_MAX_RETRY);
                                        TRACE_VERBOSE_F(F("Register server: Send %s at Node: [ %d ]\r\n"), REGISTER_METADATA_LEVEL_TYPE1, clCanard.slave[cfg_remote_queueId].get_node_id());
                                        // Prepare verify RESPONSE Method OK.
                                        remote_configure[cfg_remote_queueId]++;
                                        break;
                                    case REGISTER_04_SEND:
                                        // Register-04 ( Configure Metadata register LEVEL_TYPE2 )
                                        uavcan_register_Value_1_0_select_natural16_(&val);
                                        val.natural16.value.count = sensorCount;
                                        for(uint8_t id=0; id<sensorCount; id++) {
                                            val.natural16.value.elements[id] = param.configuration->board_slave[cfg_remote_queueId].metadata[id].levelType2;
                                        }
                                        // Send register value to Slave Remote with parameter to store
                                        clCanard.send_register_access_pending(cfg_remote_queueId, NODE_REGISTER_TIMEOUT_US,
                                            REGISTER_METADATA_LEVEL_TYPE2, val, NODE_REGISTER_MAX_RETRY);
                                        TRACE_VERBOSE_F(F("Register server: Send %s at Node: [ %d ]\r\n"), REGISTER_METADATA_LEVEL_TYPE2, clCanard.slave[cfg_remote_queueId].get_node_id());
                                        // Prepare verify RESPONSE Method OK.
                                        remote_configure[cfg_remote_queueId]++;
                                        break;
                                    case REGISTER_05_SEND:
                                        // Register-05 ( Configure Metadata register TIME P1 )
                                        uavcan_register_Value_1_0_select_natural16_(&val);
                                        val.natural16.value.count = sensorCount;
                                        for(uint8_t id=0; id<sensorCount; id++) {
                                            val.natural16.value.elements[id] = param.configuration->board_slave[cfg_remote_queueId].metadata[id].timerangeP1;
                                        }
                                        // Send register value to Slave Remote with parameter to store
                                        clCanard.send_register_access_pending(cfg_remote_queueId, NODE_REGISTER_TIMEOUT_US,
                                            REGISTER_METADATA_TIME_P1, val, NODE_REGISTER_MAX_RETRY);
                                        TRACE_VERBOSE_F(F("Register server: Send %s at Node: [ %d ]\r\n"), REGISTER_METADATA_TIME_P1, clCanard.slave[cfg_remote_queueId].get_node_id());
                                        // Prepare verify RESPONSE Method OK.
                                        remote_configure[cfg_remote_queueId]++;
                                        break;
                                    case REGISTER_06_SEND:
                                        // Register-06 ( Configure Metadata TIME PIndicator )
                                        uavcan_register_Value_1_0_select_natural8_(&val);
                                        val.natural8.value.count = sensorCount;
                                        for(uint8_t id=0; id<sensorCount; id++) {
                                            val.natural8.value.elements[id] = param.configuration->board_slave[cfg_remote_queueId].metadata[id].timerangePindicator;
                                        }
                                        // Send register value to Slave Remote with parameter to store
                                        clCanard.send_register_access_pending(cfg_remote_queueId, NODE_REGISTER_TIMEOUT_US,
                                            REGISTER_METADATA_TIME_PIND, val, NODE_REGISTER_MAX_RETRY);
                                        TRACE_VERBOSE_F(F("Register server: Send %s at Node: [ %d ]\r\n"), REGISTER_METADATA_TIME_PIND, clCanard.slave[cfg_remote_queueId].get_node_id());
                                        // Prepare verify RESPONSE Method OK.
                                        remote_configure[cfg_remote_queueId]++;
                                        break;
                                    case REGISTER_07_SEND:
                                        // Register-07 ( Configure Can SUBJECT ID Rmap Publish )
                                        uavcan_register_Value_1_0_select_natural16_(&val);
                                        val.natural16.value.count = 1;
                                        val.natural16.value.elements[0] = param.configuration->board_slave[cfg_remote_queueId].can_publish_id;
                                        // Send register value to Slave Remote with parameter to store
                                        clCanard.send_register_access_pending(cfg_remote_queueId, NODE_REGISTER_TIMEOUT_US,
                                            REGISTER_UAVCAN_DATA_PUBLISH, val, NODE_REGISTER_MAX_RETRY);
                                        TRACE_VERBOSE_F(F("Register server: Send %s at Node: [ %d ]\r\n"), REGISTER_UAVCAN_DATA_PUBLISH, clCanard.slave[cfg_remote_queueId].get_node_id());
                                        // Prepare verify RESPONSE Method OK.
                                        remote_configure[cfg_remote_queueId]++;
                                        break;
                                    case REGISTER_08_SEND:
                                        // Register-08 ( Configure Can Address if rquired != old value )
                                        if(clCanard.slave[cfg_remote_queueId].get_node_id() != param.configuration->board_slave[cfg_remote_queueId].can_address) {
                                            uavcan_register_Value_1_0_select_natural16_(&val);
                                            val.natural16.value.count = 1;
                                            val.natural16.value.elements[0] = param.configuration->board_slave[cfg_remote_queueId].can_address;
                                            // Send register value to Slave Remote with parameter to store
                                            clCanard.send_register_access_pending(cfg_remote_queueId, NODE_REGISTER_TIMEOUT_US,
                                                REGISTER_UAVCAN_NODE_ID, val, NODE_REGISTER_MAX_RETRY);
                                            TRACE_VERBOSE_F(F("Register server: Send %s at Node: [ %d ]\r\n"), REGISTER_UAVCAN_NODE_ID, clCanard.slave[cfg_remote_queueId].get_node_id());
                                            // Prepare verify RESPONSE Method OK.
                                            remote_configure[cfg_remote_queueId]++;
                                        } else {
                                            // End of programming
                                            remote_configure[cfg_remote_queueId] = REGISTER_COMPLETE;
                                        }
                                        break;
                                    case REGISTER_COMPLETE:
                                        // NEXT COMMAND LINE HERE....
                                        // L2..LType ecc... Node, Service, PortId, ServernodeId NodeId (LAST!!!) END!!!
                                        // END PROGRAMMING REGISTER REMOTE LIST OK !!!!
                                        remote_configure[cfg_remote_queueId] = 0;
                                        // Ricordo di avere riavviato. Non riconfiguro fino a rx nuovo HeartBeat con stato valido
                                        remote_configure_reboot[cfg_remote_queueId] = true;
                                        // Attendo fino a 10 secondi per nuvo controllo stato da OFF Server Register End
                                        remote_configure_end_ms = millis() + 10000;
                                        // Try end of all event recheck control
                                        reCheckEndEvent = true;
                                        TRACE_INFO_F(F("Register server: Send register configuration completed for Node: [ %d ]. Send reboot method to slave\r\n"), clCanard.slave[cfg_remote_queueId].get_node_id());
                                        // *******************************************************************************
                                        // Sending Reboot command to slave node remote CFG COMPLETE WITHOUT PENDING METHOD
                                        // *******************************************************************************
                                        // Direct command (without pending) must send with NodeId not with istance !!!
                                        clCanard.send_command(clCanard.slave[cfg_remote_queueId].get_node_id(), NODE_COMMAND_TIMEOUT_US,                            
                                            uavcan_node_ExecuteCommand_Request_1_1_COMMAND_RESTART, NULL, 0);       
                                    default:
                                        // Riparto dal comando precedente precedente se nell'area di validità
                                        // Il comando in stato di attesa non ha avuto esito positivo
                                        if(clCanard.slave[cfg_remote_queueId].register_access.event_timeout()) {
                                            clCanard.slave[cfg_remote_queueId].register_access.reset_pending();
                                            if((remote_configure[cfg_remote_queueId]>REGISTER_01_SEND)&&
                                                (remote_configure[cfg_remote_queueId]<REGISTER_08_SEND)) {
                                                remote_configure[cfg_remote_queueId]--;
                                            }
                                            else {
                                                remote_configure[cfg_remote_queueId] = 0;
                                            }
                                        }
                                        break;
                                    }
                                }
                            } else {
                                // Node is OFF_LINE Procedure ERROR
                                remote_configure[cfg_remote_queueId] = 0;
                                // Try end of all event recheck control
                                reCheckEndEvent = true;
                                TRACE_ERROR_F(F("Register server: ALERT Node: [ %d ] is OFF LINE. Remote configuration [ %s ]\r\n"), clCanard.slave[cfg_remote_queueId].get_node_id(), ABORT_STRING);
                            }
                        }
                    }
                    // Next reckeck immediatly end event server register running
                    if(reCheckEndEvent) {
                        cfgConfigureEnd = true;
                        for(uint8_t cfg_remote_queueId=0; cfg_remote_queueId<MAX_NODE_CONNECT; cfg_remote_queueId++) {
                            if(remote_configure[cfg_remote_queueId]) {
                                cfgConfigureEnd = false;
                                break;
                            }
                        }
                    }
                    // End of configure procedure complete request
                    if(cfgConfigureEnd) {
                        TRACE_VERBOSE_F(F("Module slave programming completed configured, remove flags\r\n"));
                        param.systemStatusLock->Take();
                        param.system_status->flags.cfg_server_running = false;
                        localSystemStatus->flags.run_module_configure = false;
                        param.system_status->flags.do_remote_cfg_exec = false;
                        param.systemStatusLock->Give();
                    }
                }

                // Register SERVER Gestion Pending, Response and TimeOut
                // NB. To Set parameter Register with value remote create a register and call send method
                // To read a register create register and set to unstructured and call send method. (Rx is performed)
                if(param.system_status->flags.cfg_server_running) {
                    // loop for all Node and switching from list command sequence.
                    // Create and send register command and wait progression in server command procedure
                    for(uint8_t register_server_queueId=0; register_server_queueId<MAX_NODE_CONNECT; register_server_queueId++) {
                        if (clCanard.slave[register_server_queueId].register_access.event_timeout()) {
                            // Reset del pending comando
                            clCanard.slave[register_server_queueId].register_access.reset_pending();
                            // TimeOUT di un comando in attesa... gestisco il da farsi
                            if(remote_configure[register_server_queueId]) {
                                // Abort configuration (after retry...)
                                if(remote_configure_retry[register_server_queueId]) {
                                    remote_configure_retry[register_server_queueId]--;
                                    // Retry sending last command register (state proc - 1)
                                    remote_configure[register_server_queueId]--;
                                    TRACE_ERROR_F(F("Register server: Command Node: [ %d ] not responding to param request. Remaining retry send command: [ %d ]\r\n"), clCanard.slave[register_server_queueId].get_node_id(), remote_configure_retry[register_server_queueId]);
                                } else {
                                    TRACE_ERROR_F(F("Register server: ALERT Node: [ %d ] not responding to param request. Remote configuration [ %s ]\r\n"), clCanard.slave[register_server_queueId].get_node_id(), ABORT_STRING);
                                    remote_configure[register_server_queueId] = 0;
                                }
                            } else {
                                TRACE_ERROR_F(F("Register server: ALERT Node: [ %d ] not responding to param request. Command [ %s ]\r\n"), clCanard.slave[register_server_queueId].get_node_id(), ABORT_STRING);
                            }
                        }
                        if (clCanard.slave[register_server_queueId].register_access.is_executed()) {
                            // Reset del pending comando
                            clCanard.slave[register_server_queueId].register_access.reset_pending();
                            if(remote_configure[register_server_queueId]) {
                                // Pass to NEXT REGISTER increment switch position end to REGISTER_COMPLETE value
                                remote_configure[register_server_queueId]++;
                                TRACE_VERBOSE_F(F("Register server: Recive register R/W response from node in configure sequence: [ %d ]. Register access [ %s ]\r\n"), clCanard.slave[register_server_queueId].get_node_id(), OK_STRING);
                            } else {
                                TRACE_VERBOSE_F(F("Register server: Recive register R/W response from node: [ %d ]. Register access [ %s ]\r\n"), clCanard.slave[register_server_queueId].get_node_id(), OK_STRING);
                            }
                            val = clCanard.slave[register_server_queueId].register_access.get_response();
                            // se risposta registr != da empty in request il registro è impostato o letto (se empty = non esiste)
                            if(val._tag_) // !=0 (!= empty)
                            {
                                TRACE_VERBOSE_F(F("Register server: check response from node: [ %d ]. Register setted [ %s ]\r\n"), clCanard.slave[register_server_queueId].get_node_id(), OK_STRING);
                            } else {
                                TRACE_ERROR_F(F("Register server: check response from node: [ %d ]. Register setted [ %s ]\r\n"), clCanard.slave[register_server_queueId].get_node_id(), ERROR_STRING);
                                // Abort configuration without Retry (Command refused)
                                remote_configure[register_server_queueId] = 0;
                            }
                        }
                    }
                } else {
                    // Mantengo off il segnale di reboot inviato (sempre pronto ad una nuova configurazione...)
                    for(uint8_t register_server_queueId=0; register_server_queueId<MAX_NODE_CONNECT; register_server_queueId++) {
                        if(remote_configure_reboot[register_server_queueId]) {
                            // Se passo il timeOut di fine configurazione e riavvio Nodo posso eliminare il segnale di WaitReboot
                            if(millis() > remote_configure_end_ms) {
                                remote_configure_reboot[register_server_queueId] = false;
                            }
                        }
                    }
                }
                // ***************** FINE TEST COMANDO TX-> RX<- *************************


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

                // Get coda comandi da system_message... se richiesto aggiornamenti dei firmware completi (tutta la stazione)
                if(!param.systemMessageQueue->IsEmpty()) {
                    // Message queue is for CAN (If FW Upgrade local Master, Message is for SD...)
                    if(param.systemMessageQueue->Peek(&system_message, 0)) {
                        // No need to check ID Node (global command...)
                        if((system_message.task_dest == LOCAL_TASK_ID) && (system_message.command.do_update_all)) {
                            // Remove message from the queue
                            param.systemMessageQueue->Dequeue(&system_message);
                            // Request start update firmware from LCD or Remote RPC
                            // Start flags and state for file_server start
                            is_running_update_system = true;
                            // Start from last boards to first (When 0xFF -> CMD_PARAM_MASTER_ADDRESS) is Master Request and END Procedure upload
                            index_running_update_boards = MAX_NODE_CONNECT - 1;
                        }
                    }
                }

                // True if Request all system update?! (UP)
                // Next start if is avaiable, not running another file server (File Server can be run simultaneously)
                // But we update one card at a time and Master for last
                if((is_running_update_system)&&(!is_running_update_send_cmd)&&
                    (!param.system_status->flags.file_server_running)) {
                    // Have reached the last boards (Master)?
                    if(index_running_update_boards == CMD_PARAM_MASTER_ADDRESS) {
                        // Master Boards (Update start from SD Task but mode to request is same with queue)
                        if(param.system_status->data_master.fw_upgradable) {
                            memset(&system_message, 0, sizeof(system_message));
                            // Starting sequence as queue command same LCD/RPC ecc...
                            system_message.task_dest = SD_TASK_ID;
                            system_message.command.do_update_fw = true;
                            system_message.node_id = index_running_update_boards;
                            param.systemMessageQueue->Enqueue(&system_message);
                        }
                        // End of procedure. Master not required updating
                        is_running_update_system = false;
                    } else {
                        // Slave Boards
                        // Are avaiable one firmware most recent?
                        if(param.system_status->data_slave[index_running_update_boards].fw_upgradable) {
                            // Is firmware_updating stopped (waiting end of last operation if started...)
                            memset(&system_message, 0, sizeof(system_message));
                            // Inibith procedure uploading while end_event update occurs
                            // Next check board only without server running and command waiting...
                            is_running_update_send_cmd = true;
                            // Starting sequence as queue command same LCD/RPC ecc...
                            system_message.task_dest = CAN_TASK_ID;
                            system_message.command.do_update_fw = true;
                            system_message.node_id = index_running_update_boards;
                            param.systemMessageQueue->Enqueue(&system_message);
                        }
                        // Set check to Next Boards, Update event can start
                        index_running_update_boards--;
                    }
                }

                // Get coda comandi da system_message... se richiesto aggiornamento del firmware
                if(!param.systemMessageQueue->IsEmpty()) {
                    // Message queue is for CAN (If FW Upgrade local Master, Message is only directet to TASK SD...)
                    if(param.systemMessageQueue->Peek(&system_message, 0)) {
                        // Only local task post message queue can be processed here
                        // Messages checked here can only be addressed to remote slaves. So the addresses must be in the valid area
                        if((system_message.task_dest == LOCAL_TASK_ID) && (system_message.node_id < MAX_NODE_CONNECT)) {
                            if(system_message.command.do_update_fw) {
                                // Start Flag Event Start when request configuration is request
                                // When remote node recive VSC from Master Heartbeat Remote slave FullPower is performed
                                // Then new state for slave (fullpower) are resend to master. If Ok procedure can start 
                                if(clCanard.slave[system_message.node_id].heartbeat.get_power_mode() == Power_Mode::pwr_on) {
                                    // Remove message from the queue
                                    param.systemMessageQueue->Dequeue(&system_message);
                                    // Request start update firmware from LCD or Remote RPC
                                    // Start flags and state for file_server start
                                    param.systemStatusLock->Take();
                                    param.system_status->flags.file_server_running = true;
                                    param.systemStatusLock->Give();
                                    // Set STATE for boards request in firmware upgrade
                                    clCanard.slave[(uint8_t)system_message.node_id].file_server.start_state();
                                } else {
                                    // IS NEED to Request FullPower Mode for type of command
                                    if(!param.system_status->flags.full_wakeup_request) {
                                        TRACE_VERBOSE_F(F("Command server: Start full power for sending firmware at node: [ %d ]"), clCanard.slave[system_message.node_id].get_node_id());
                                        param.systemStatusLock->Take();
                                        param.system_status->flags.full_wakeup_request = true;
                                        param.systemStatusLock->Give();
                                    }
                                }
                            }
                        }
                    }
                }

                // Are file server running? Process request or verify END procedure
                if(param.system_status->flags.file_server_running) {

                    // Check if file server are current in running state
                    bool fileServerEnd = true;

                    for(uint8_t file_server_queueId=0; file_server_queueId<MAX_NODE_CONNECT; file_server_queueId++) {

                        // state != standby? Running current procedure...
                        if(clCanard.slave[file_server_queueId].file_server.get_state() != canardClass::FileServer_State::standby)
                            fileServerEnd = false;

                        // Se vado OffLine la procedura comunque viene interrotta dall'evento di OffLine
                        switch(clCanard.slave[file_server_queueId].file_server.get_state()) {
                            default: // -->> FILE_STATE_ERROR STATE:
                                clCanard.slave[file_server_queueId].file_server.end_transmission();
                                break;
                            case canardClass::FileServer_State::standby:
                                // Nothing to do. Start Case is external from this state
                                // The current boards are in normal state. Check next boards...
                                break;
                            case canardClass::FileServer_State::start_request:
                                // Starting request remote from any system
                                char file_name[CAN_FILE_NAME_SIZE_MAX];
                                // Set correct name of file from last avaiable (by module)
                                for(uint8_t iAvaiableFw=0; iAvaiableFw<STIMA_MODULE_TYPE_MAX_AVAIABLE; iAvaiableFw++) {
                                    // Found corrispondence from avaiable firmware and module type installed
                                    // Retrive name firmware to get up
                                    if(param.system_status->boards_update_avaiable[iAvaiableFw].module_type == 
                                        param.system_status->data_slave[file_server_queueId].module_type) {
                                        setStimaFirmwareName(file_name,
                                            param.system_status->boards_update_avaiable[iAvaiableFw].module_type,
                                            param.system_status->boards_update_avaiable[iAvaiableFw].version,
                                            param.system_status->boards_update_avaiable[iAvaiableFw].revision);
                                        break;
                                        TRACE_INFO_F(F("File server: Prepare trasmission file firmware at node: [ %d ]. File name [ %s ]\r\n"), clCanard.slave[file_server_queueId].get_node_id(), file_name);
                                    }
                                }
                                // Start upload file with module_type last version found on SD Card
                                clCanard.slave[file_server_queueId].file_server.set_file_name(file_name, true);
                                break;
                            case canardClass::FileServer_State::begin_update:
                                // Avvio comando di aggiornamento, controllo coerenza ed esistenza file già effettuato
                                // Se non ci sono altre problematiche (aggiornamenti vari, download blocchi ecc.. avvio...)
                                // Avvio il comando nel nodo remoto
                                clCanard.slave[file_server_queueId].file_server.next_state();
                                TRACE_INFO_F(F("File server: Starting trasmission command download firmware at node: [ %d ]\r\n"), clCanard.slave[file_server_queueId].get_node_id());
                                // altrimenti Annullo la richiesta (Invio Error al Remoto su Request File)
                                // O smetto di rispondere al servizio per quella richiesta (TimeOut)
                                // clCanard.slave[queueId].file_server.end_transmission(); -> ANNULLA!!!
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
                                    TRACE_ERROR_F(F("File server: Node [ %d ], TimeOut Command Start Send file, uploading %s\r\n"),
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
                                        // Avvio il flag di aggiornamento corrente in corso... a Display
                                        param.systemStatusLock->Take();
                                        param.system_status->data_slave[file_server_queueId].is_fw_upgrading = true;
                                        param.systemStatusLock->Give();
                                        clCanard.slave[file_server_queueId].file_server.next_state();
                                        // Stampo lo stato
                                        TRACE_INFO_F(F("File server: Node [ %d ] upload Start Send file %s\r\n"),
                                            clCanard.slave[file_server_queueId].get_node_id(), OK_STRING);
                                        // Imposto il timeOUT per controllo Deadline con pending per sequenza di download
                                        clCanard.slave[file_server_queueId].file_server.start_pending(NODE_REQFILE_TIMEOUT_US);
                                    } else {
                                        // Errore comando eseguito ma risposta non valida. Annullo il trasferimento
                                        clCanard.slave[file_server_queueId].file_server.end_transmission();
                                        // ...counico al server (RMAP remoto) l'errore per il mancato aggiornamento ed esco
                                        TRACE_ERROR_F(F("File server: Node [ %d ] response Cmd Error in Send file %s\r\n"),
                                            clCanard.slave[file_server_queueId].get_node_id(), ABORT_STRING);
                                    }
                                }
                                break;
                            case canardClass::FileServer_State::state_uploading:
                                // Attendo la risposta del Nodo Remoto conferma, errore o TimeOut
                                if(clCanard.slave[file_server_queueId].file_server.event_timeout()) {
                                    // Counico al server l'errore di timeOut Command Update Start ed esco
                                    clCanard.slave[file_server_queueId].file_server.end_transmission();
                                    TRACE_ERROR_F(F("File server: Node [ %d ] TimeOut Request/Response send file %s\r\n"),
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
                                TRACE_INFO_F(F("File server: Node [ %d ] uploading completed %s\r\n"),
                                    clCanard.slave[file_server_queueId].get_node_id(), OK_STRING);
                                break;
                        }
                    }

                    // End of Running file server...
                    if(fileServerEnd) {
                        param.systemStatusLock->Take();
                        param.system_status->flags.file_server_running = false;
                        for(uint8_t file_server_queueId=0; file_server_queueId<MAX_NODE_CONNECT; file_server_queueId++) {
                            param.system_status->data_slave[file_server_queueId].is_fw_upgrading = false;
                        }
                        param.systemStatusLock->Give();
                        // End command if started for update_all system RPC. Irrelevant in other cases.
                        is_running_update_send_cmd = false;
                    }
                }
                // ********************* END FILE SERVER CAN UPLOADER ********************

                // *********************************************************************
                // Inibith Reboot Module and GET Full Power for all Module in this case
                // *********************************************************************
                if((param.system_status->flags.full_wakeup_request)&&
                    ((param.system_status->flags.cfg_server_running)||
                    (param.system_status->flags.cmd_server_running)||
                    (param.system_status->flags.reg_server_running)||
                    (param.system_status->flags.file_server_running))) {
                    // Reset request full_wakeup_request is needed and valid while any server mode is started
                    // Flag is setted at start of server if mode power is not full_power and must be reset
                    // when activity_server programmed is terminated. When a server start, flag must to be resetted
                    param.systemStatusLock->Take();
                    param.system_status->flags.full_wakeup_request = false;
                    param.systemStatusLock->Give();
                }
                // *** INIBITH REBOOT LOCAL AND REMOTE RPC ***
                if(((param.system_status->flags.cfg_server_running)||
                    (param.system_status->flags.cmd_server_running)||
                    (param.system_status->flags.reg_server_running)||
                    (param.system_status->flags.file_server_running)||
                    (param.system_status->flags.full_wakeup_request)) &&
                    (!param.system_status->flags.inibith_reboot)) {
                    param.systemStatusLock->Take();
                    param.system_status->flags.inibith_reboot = true;
                    param.systemStatusLock->Give();
                }
                // *** RESTORE REBOOT LOCAL AND REMOTE RPC ***
                if(((!param.system_status->flags.cfg_server_running)&&
                    (!param.system_status->flags.cmd_server_running)&&
                    (!param.system_status->flags.reg_server_running)&&
                    (!param.system_status->flags.file_server_running) &&
                    (!param.system_status->flags.full_wakeup_request)) &&
                    (param.system_status->flags.inibith_reboot)) {
                    param.systemStatusLock->Take();
                    param.system_status->flags.inibith_reboot = false;
                    param.systemStatusLock->Give();
                }
                // *** FULL POWER Operation Request (RPC, Remote config, command, display) ***
                if(((param.system_status->flags.full_wakeup_forced)||
                    (param.system_status->flags.full_wakeup_request)||
                    (param.system_status->flags.cfg_server_running)||
                    (param.system_status->flags.cmd_server_running)||
                    (param.system_status->flags.reg_server_running)||
                    (param.system_status->flags.file_server_running)||
                    (param.system_status->flags.rmap_server_running)||
                    (bStartupGetIstant)||
                    (param.system_status->flags.display_on)) && 
                    (param.system_status->flags.power_state != Power_Mode::pwr_on)) {
                    // Disable module sleep if need operation into Class
                    // Comunicate to the network CAN FullPower request. All Node exit DeepSleep and enter FullPower
                    clCanard.flag.disable_sleep();
                    param.systemStatusLock->Take();
                    // Report Full POWER request for operation specific Task
                    param.system_status->flags.power_state = Power_Mode::pwr_on;
                    param.systemStatusLock->Give();
                    // Applicate command for All Network UAVCAN
                    clCanard.flag.set_local_power_mode(param.system_status->flags.power_state);
                }
                // *** NORMAL POWER Operation Request (End of RPC, Remote config, command, display) ***
                if(((!param.system_status->flags.full_wakeup_forced)&&
                    (!param.system_status->flags.full_wakeup_request)&&
                    (!param.system_status->flags.cfg_server_running)&&
                    (!param.system_status->flags.cmd_server_running)&&
                    (!param.system_status->flags.reg_server_running)&&
                    (!param.system_status->flags.file_server_running)&&
                    (!param.system_status->flags.rmap_server_running)&&
                    (!bStartupGetIstant)&&
                    (!param.system_status->flags.display_on)) && 
                    (param.system_status->flags.power_state == Power_Mode::pwr_on)) {
                    // Enable module sleep if need operation into Class
                    // Comunicate to the network CAN nominal power request. All Node enable DeepSleep and exit FullPower
                    clCanard.flag.enable_sleep();
                    param.systemStatusLock->Take();
                    // Normal POWER request for operation specific. If critical Power... Power Critical mode is selected
                    // Depending from MPPT Level status Critical for Other Method
                    if(param.system_status->flags.power_critical) {
                        param.system_status->flags.power_state = Power_Mode::pwr_critical;
                    } else {
                        param.system_status->flags.power_state = Power_Mode::pwr_nominal;
                    }
                    param.systemStatusLock->Give();
                    // Applicate command for All Network UAVCAN
                    clCanard.flag.set_local_power_mode(param.system_status->flags.power_state);
                }
                // ***********************************************************************


                // **************************************************************************
                // -> Scheduler temporizzato dei messaggi standard da inviare alla rete UAVCAN
                // **************************************************************************
                // ************************* HEARTBEAT DATA PUBLISHER ***********************
                // Abilita pubblicazione HB con upgrade_fw per segnalare lo start. Solo senza inibizione Reboot
                // Altrimenti entrerebbe di continuo nel Publish fino al reboot
                if(((start_firmware_upgrade) && (!param.system_status->flags.inibith_reboot)) ||
                    (clCanard.getMicros(clCanard.syncronized_time) >= last_pub_heartbeat)) {
                    // Startup FullPower to get istant value counter OFF (Synch to Numb Pub of HeartBeat)
                    if(bStartupCount) {
                        bStartupCount--;
                        if(!bStartupCount) bStartupGetIstant = false;
                    }
                    // Publish HeartBeat
                    TRACE_INFO_F(F("Publish MASTER Heartbeat -->> [ %u sec]\r\n"), TIME_PUBLISH_HEARTBEAT);
                    clCanard.master_heartbeat_send_message();
                    // Update next publisher
                    last_pub_heartbeat = clCanard.getMicros(clCanard.syncronized_time) + MEGA * TIME_PUBLISH_HEARTBEAT;
                }
                // LOOP HANDLER >> 1 SECONDO << TIME SYNCRO (alternato 0.5 sec con Heartbeat)
                if (clCanard.getMicros(clCanard.syncronized_time) >= next_timesyncro_msg)
                {
                    next_timesyncro_msg = clCanard.getMicros(clCanard.syncronized_time) + MEGA;
                    // Time syncro are sended only when full power is enabled
                    if(param.system_status->flags.power_state == Power_Mode::pwr_on) {
                        TRACE_INFO_F(F("Publish MASTER Time Syncronization -->> [1 sec]\r\n"));
                        clCanard.master_timestamp_send_syncronization();
                    }
                }
                // ********************** SERVICE PORT LIST PUBLISHER ***********************
                if (clCanard.flag.get_local_power_mode() == Power_Mode::pwr_on) {
                    if (clCanard.getMicros(clCanard.syncronized_time) >= last_pub_port_list) {
                        // Publish list service only if full power mode are selected
                        TRACE_INFO_F(F("Publish Local PORT LIST -->> [ %u sec]\r\n"), TIME_PUBLISH_PORT_LIST);
                        last_pub_port_list = clCanard.getMicros(clCanard.syncronized_time) + MEGA * TIME_PUBLISH_PORT_LIST;
                        // Update publisher
                        clCanard.master_servicelist_send_message();
                    }
                } else {
                    // Mantengo il publish Port avanti nel tempo e Avvio solo a MAX Power e tempo
                    // di pubblicazione interamente trascorso. Evita Publish inutile in Request PowerUP
                    // da Master, per semplici operazioni di lettura e/o configurazione ( In Power::Sleep )
                    last_pub_port_list = clCanard.getMicros(clCanard.syncronized_time);
                }

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

                // Request Reboot from RPC in security mode
                if(!param.systemMessageQueue->IsEmpty()) {
                    // Message queue is for CAN (If FW Upgrade local Master, Message is for SD...)
                    if(param.systemMessageQueue->Peek(&system_message, 0)) {
                        // ENTER PROCEDURE CONFIG (Only Full POWERED Module!!!)
                        // Global message no required check nodeId area validation
                        if((system_message.task_dest == LOCAL_TASK_ID) && (system_message.command.do_reboot)) {
                            param.systemMessageQueue->Dequeue(&system_message);
                            // Start Reboot check state
                            clCanard.flag.request_system_restart();
                        }
                    }
                }
                // Request Reboot (Firmware upgrade direct... Or Request Reset with inibit control)
                if (!param.system_status->flags.inibith_reboot) {
                    if (clCanard.flag.is_requested_system_restart() || (start_firmware_upgrade)) {
                        TRACE_INFO_F(F("Send signal to system Reset...\r\n"));
                        delay(250); // Waiting security queue empty send HB (Updating start...)
                        NVIC_SystemReset();                    
                    }
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
        if(clCanard.master.file.download_request() || param.system_status->flags.file_server_running) {            
            Delay(Ticks::MsToTicks(TASK_WAIT_REALTIME_DELAY_MS));
        }
        else
        {
            Delay(Ticks::MsToTicks(CAN_TASK_WAIT_DELAY_MS));
        }
    }
}

#endif
