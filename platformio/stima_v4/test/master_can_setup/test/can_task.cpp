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

#define TRACE_LEVEL   CAN_TASK_TRACE_LEVEL
#define LOCAL_TASK_ID CAN_TASK_ID

#include "tasks/can_task.h"

#include "unity.h"

#if (ENABLE_CAN)

using namespace cpp_freertos;

bool result;
bool is_init_subs_ok;

// ************************************************************************
// ******************* TEST CAN FUNCTION DECLARATIONS *********************
// ************************************************************************

void test_init_bxcancomputetimings_on_chip(void);
void test_init_bxcanconfigure_on_chip(void);
void test_init_hal_can_interrupt(void);
void test_init_hal_can_stm(void);
void test_init_subscriptions_uavcan(void);

// ************************************************************************
// ******************* TEST CAN FUNCTION IMPLEMENTATIONS ******************
// ************************************************************************

/**
 * @brief TEST: Initialization of bxcancomputetimings on chip
 *
 */
void test_init_bxcancomputetimings_on_chip() {
    TEST_ASSERT_EQUAL(true, result);
}

/**
 * @brief Initialization of bxcanconfigure on chip
 *
 */
void test_init_bxcanconfigure_on_chip() {
    TEST_ASSERT_EQUAL(true, result);
}

/**
 * @brief Initalization of HAL_CAN interrupt
 *
 */
void test_init_hal_can_interrupt() {
    TEST_ASSERT_EQUAL(HAL_OK, HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING));
}

/**
 * @brief TEST: Initialization of HAL_CAN STM
 *
 */
void test_init_hal_can_stm() {
    TEST_ASSERT_EQUAL(HAL_OK, HAL_CAN_Start(&hcan1));
}

/**
 * @brief TEST: Initialization subscriptions UAVCAN
 *
 */
void test_init_subscriptions_uavcan() {
    TEST_ASSERT_EQUAL(true, is_init_subs_ok);
}

// ***************************************************************************************************
// **********             Funzioni ed utility generiche per gestione UAVCAN                 **********
// ***************************************************************************************************

/// @brief Enable Power CAN_Circuit TJA1443
/// @param ModeCan (Mode TYPE CAN_BUS)
void CanTask::HW_CAN_Power(CAN_ModePower ModeCan) {
    // Normal Mode (TX/RX Full functionally)
    if (ModeCan == CAN_ModePower::CAN_INIT) {
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
    if (ModeCan == CAN_ModePower::CAN_NORMAL) {
        digitalWrite(PIN_CAN_STB, HIGH);
        delayMicroseconds(10);
        digitalWrite(PIN_CAN_EN, HIGH);
        // Waiting min of 65 uS for Full Operational External CAN Power Circuit
        // Perform secure WakeUp Timer with 100 uS
        delayMicroseconds(90);
    }
    // Listen Mode (Only RX circuit enabled for first paket data)
    if (ModeCan == CAN_ModePower::CAN_LISTEN_ONLY) {
        digitalWrite(PIN_CAN_EN, LOW);
        delayMicroseconds(10);
        digitalWrite(PIN_CAN_STB, HIGH);
    }
    // Sleep (Turn OFF HW and enter sleep mode TJA1443)
    if (ModeCan == CAN_ModePower::CAN_SLEEP) {
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
void CanTask::getUniqueID(uint8_t out[uavcan_node_GetInfo_Response_1_0_unique_id_ARRAY_CAPACITY_], uint64_t serNumb) {
    // A real hardware node would read its unique-ID from some hardware-specific source (typically stored in ROM).
    // This example is a software-only node so we store the unique-ID in a (read-only) register instead.
    static uavcan_register_Value_1_0 val = {0};
    uavcan_register_Value_1_0_select_unstructured_(&val);

    // Crea default unique_id con 8 BYTES From local_serial Number (N.B. serNumb[0] rappresenta Tipo Nodo )
    uint8_t *ptrData = (uint8_t *)&serNumb;
    for (uint8_t i = 0; i < 8; i++) {
        val.unstructured.value.elements[val.unstructured.value.count++] = ptrData[i];
    }
    // Il resto dei 128 vengono impostati RANDOM
    for (uint8_t i = val.unstructured.value.count; i < uavcan_node_GetInfo_Response_1_0_unique_id_ARRAY_CAPACITY_; i++) {
        val.unstructured.value.elements[val.unstructured.value.count++] = (uint8_t)rand();  // NOLINT
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
bool CanTask::putFlashFile(const char *const file_name, const bool is_firmware, const bool rewrite, void *buf, size_t count) {
#ifdef CHECK_FLASH_WRITE
    // check data (W->R) Verify Flash integrity OK
    uint8_t check_data[FLASH_BUFFER_SIZE];
#endif
    // Request New File Init Upload
    if (rewrite) {
        // Qspi Security Semaphore
        if (localQspiLock->Take(Ticks::MsToTicks(FLASH_SEMAPHORE_MAX_WAITING_TIME_MS))) {
            // Init if required (DeInit after if required PowerDown Module)
            if (localFlash->BSP_QSPI_Init() != Flash::QSPI_OK) {
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
            if (memcmp(file_flash_name, check_data, FLASH_FILE_SIZE_LEN) == 0) {
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
    if (localQspiLock->Take(Ticks::MsToTicks(FLASH_SEMAPHORE_MAX_WAITING_TIME_MS))) {
        // 0 = Is UavCan Signal EOF for Last Block Exact Len 256 Bytes...
        // If Value Count is 0 no need to Write Flash Data (Only close Fule Info)
        if (count != 0) {
            // Write into Flash
            TRACE_INFO_F(F("FLASH: Write [ %d ] bytes at addr: %d\n\r"), count, canFlashPtr);
            // Starting Write at OFFSET Required... Erase here is Done
            localFlash->BSP_QSPI_Write((uint8_t *)buf, canFlashPtr, count);
#ifdef CHECK_FLASH_WRITE
            localFlash->BSP_QSPI_Read(check_data, canFlashPtr, count);
            if (memcmp(buf, check_data, count) == 0) {
                TRACE_INFO_F(F("FLASH: Reading check OK\n\r"));
            } else {
                TRACE_ERROR_F(F("FLASH: Reading check ERROR\n\r"));
                localQspiLock->Give();
                return false;
            }
#endif
            canFlashPtr += count;
            // Check if Next Page Addressed (For Erase Next Block)
            if ((canFlashPtr / AT25SF641_BLOCK_SIZE) != canFlashBlock) {
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
        if (count != 0x100) {
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
            localFlash->BSP_QSPI_Write((uint8_t *)&lenghtFile, FLASH_SIZE_ADDR(canFlashPtr), FLASH_INFO_SIZE_U64);
            // Write into Flash
            TRACE_INFO_F(F("FLASH: Write [ %d ] bytes at addr: %d\n\r"), FLASH_INFO_SIZE_U64, canFlashPtr);
#ifdef CHECK_FLASH_WRITE
            localFlash->BSP_QSPI_Read(check_data, FLASH_SIZE_ADDR(canFlashPtr), FLASH_INFO_SIZE_U64);
            if (memcmp(&lenghtFile, check_data, FLASH_INFO_SIZE_U64) == 0) {
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
bool CanTask::getFlashFwInfoFile(uint8_t *module_type, uint8_t *version, uint8_t *revision, uint64_t *len) {
    uint8_t block[FLASH_FILE_SIZE_LEN];
    bool fileReady = false;

    // Qspi Security Semaphore
    if (localQspiLock->Take(Ticks::MsToTicks(FLASH_SEMAPHORE_MAX_WAITING_TIME_MS))) {
        // Init if required (DeInit after if required PowerDown Module)
        if (localFlash->BSP_QSPI_Init() != Flash::QSPI_OK) {
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
        if (checkStimaFirmwareType((char *)block, module_type, version, revision)) {
            localFlash->BSP_QSPI_Read((uint8_t *)len, FLASH_SIZE_ADDR(0), FLASH_INFO_SIZE_U64);
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
uavcan_node_ExecuteCommand_Response_1_1 CanTask::processRequestExecuteCommand(canardClass &clCanard, const uavcan_node_ExecuteCommand_Request_1_1 *req,
                                                                              uint8_t remote_node) {
    uavcan_node_ExecuteCommand_Response_1_1 resp = {0};
    // req->command (Comando esterno ricevuto 2 BYTES RESERVED FFFF-FFFA)
    // Gli altri sono liberi per utilizzo interno applicativo con #define interne
    // req->parameter (array di byte MAX 255 per i parametri da request)
    // Risposta attuale (resp) 1 Bytes RESERVED (0..6) gli altri #define interne
    switch (req->command) {
        // **************** Comandi standard UAVCAN GENERIC_SPECIFIC_COMMAND ****************
        // Comando di aggiornamento Firmware compatibile con Yakut e specifice UAVCAN
        case uavcan_node_ExecuteCommand_Request_1_1_COMMAND_BEGIN_SOFTWARE_UPDATE: {
            // Nodo Server chiamante (Yakut solo Master, Yakut e Master per Slave)
            clCanard.master.file.start_request(remote_node, (uint8_t *)req->parameter.elements,
                                               req->parameter.count, true);
            clCanard.flag.set_local_fw_uploading(true);
            TRACE_INFO_F(F("Firmware update request from node id: %u\r\n"), clCanard.master.file.get_server_node());
            TRACE_INFO_F(F("Filename to download: %s\r\n"), clCanard.master.file.get_name());
            // Avvio la funzione con OK
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        case uavcan_node_ExecuteCommand_Request_1_1_COMMAND_FACTORY_RESET: {
            localRegisterAccessLock->Take();
            localRegister->doFactoryReset();
            localRegisterAccessLock->Give();
            // Istant Reboot for next Register base Setup
            NVIC_SystemReset();
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        case uavcan_node_ExecuteCommand_Request_1_1_COMMAND_RESTART: {
            clCanard.flag.request_system_restart();
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
        // Local CAN Transport to RPC Call
        case canardClass::Command_Private::set_full_power: {
            // Abilita modalità full power (per manutenzione e/o test)
            localSystemStatusLock->Take();
            localSystemStatus->flags.full_wakeup_forced = true;
            localSystemStatusLock->Give();
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        case canardClass::Command_Private::set_nominal_power: {
            // Disabilita modalità full power (per manutenzione e/o test)
            localSystemStatusLock->Take();
            localSystemStatus->flags.full_wakeup_forced = false;
            localSystemStatusLock->Give();
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        case canardClass::Command_Private::execute_rpc: {
            // Abilita pubblicazione slow_loop elenco porte (Cypal facoltativo)
            bool is_event_rpc = true;
            localStreamRpc->init();
            localRpcLock->Take();
            localStreamRpc->parseCharpointer(&is_event_rpc, (char *)req->parameter.elements, req->parameter.count, NULL, 0, RPC_TYPE_CAN);
            localRpcLock->Give();
            if (!is_event_rpc)
                resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            else
                resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_BAD_COMMAND;
            break;
        }
        // Comando di download File generico compatibile con specifice UAVCAN, (LOG/CFG altro...)
        case canardClass::Command_Private::download_file: {
            // Nodo Server chiamante (Yakut solo Master, Yakut e Master per Slave)
            clCanard.master.file.start_request(remote_node, (uint8_t *)req->parameter.elements,
                                               req->parameter.count, false);
            clCanard.flag.set_local_fw_uploading(true);
            TRACE_INFO_F(F("File standard update request from node id: %u\r\n"), clCanard.master.file.get_server_node());
            TRACE_INFO_F(F("Filename to download: %s\r\n"), clCanard.master.file.get_name());
            // Avvio la funzione con OK
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        case canardClass::Command_Private::enable_publish_port_list: {
            // Abilita pubblicazione slow_loop elenco porte (Cypal facoltativo)
            clCanard.publisher_enabled.port_list = true;
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        case canardClass::Command_Private::disable_publish_port_list: {
            // Disabilita pubblicazione slow_loop elenco porte (Cypal facoltativo)
            clCanard.publisher_enabled.port_list = false;
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
uavcan_register_Access_Response_1_0 CanTask::processRequestRegisterAccess(const uavcan_register_Access_Request_1_0 *req) {
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
void CanTask::processReceivedTransfer(canardClass &clCanard, const CanardRxTransfer *const transfer) {
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
            if (uavcan_pnp_NodeIDAllocationData_1_0_deserialize_(&msg, static_cast<uint8_t const *>(transfer->payload), &size) >= 0) {
                // Cerco nei moduli conosciuti (in HASH_UNIQUE_ID) invio il tipo modulo...
                // Verifico se ho un nodo ancora da configurare come da cfg del master
                // Il nodo deve essere compatibile con il tipo di modulo previsto da allocare
                TRACE_VERBOSE_F(F("RX PNP Allocation message request from -> "));
                switch (msg.unique_id_hash & 0xFF) {
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
                TRACE_VERBOSE_F(F(" [Serial Number HASH ID: %lu]\r\n"), (uint32_t)msg.unique_id_hash);
                // Risposta immediata diretta (Se nodo ovviamente è riconosciuto...)
                // Non utilizziamo una Response in quanto l'allocation è sempre un messaggio anonimo
                // I metadati del trasporto sono come quelli riceuti del transferID quindi è un messaggio
                // che si comporta parzialmente come una risposta (per rilevamento remoto hash/transfer_id)
                if (defaultNodeId == GENERIC_BVAL_UNCOERENT) {
                    TRACE_VERBOSE_F(F("PNP Allocation incoerent with Master configuration for reques [ %s ]\r\n"), ABORT_STRING);
                } else if (defaultNodeId <= CANARD_NODE_ID_MAX) {
                    TRACE_VERBOSE_F(F("Try PNP Allocation with Node_ID -> %d\r\n"), defaultNodeId);
                    // Se il nodo proposto viene confermato inizieremo a ricevere heartbeat
                    // da quel nodeId. A questo punto in Heartbeat settiamo il flag pnp.configure()
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
                    clCanard.send(MEGA, &meta, serialized_size, &serialized[0]);
                }
            }
        }
        // Gestione dei messaggi Heartbeat per stato rete (gestisco come master)
        else if (transfer->metadata.port_id == uavcan_node_Heartbeat_1_0_FIXED_PORT_ID_) {
            bKindMessageProcessed = true;
            size_t size = transfer->payload_size;
            uavcan_node_Heartbeat_1_0 msg = {0};
            if (uavcan_node_Heartbeat_1_0_deserialize_(&msg, static_cast<uint8_t const *>(transfer->payload), &size) >= 0) {
                TRACE_VERBOSE_F(F("RX HeartBeat from node %u\r\n"), transfer->metadata.remote_node_id);
                // Processo e registro il nodo: stato, OnLine e relativi flag
                uint8_t queueId = clCanard.getSlaveIstanceFromId(transfer->metadata.remote_node_id);
                // Se nodo correttamente allocato e gestito (potrebbe essere Yakut non registrato)
                if (queueId != GENERIC_BVAL_UNDEFINED) {
                    // Accodo i dati letti dal messaggio (Nodo -> OnLine) verso la classe
                    clCanard.slave[queueId].heartbeat.set_online(NODE_OFFLINE_TIMEOUT_US,
                                                                 msg.vendor_specific_status_code, msg.health.value, msg.mode.value, msg.uptime);
                    // Controlla se il modulo è ready (configurato) Altrimenti avvio la configurazione...
                    if (!clCanard.slave[queueId].heartbeat.get_module_ready()) {
                        TRACE_VERBOSE_F(F("Module slave [ %u ] is not (ready) configured\r\n"), transfer->metadata.remote_node_id);
                        localSystemStatusLock->Take();
                        localSystemStatus->flags.run_module_configure = true;
                        localSystemStatusLock->Give();
                    }
                    TRACE_VERBOSE_F(F("Power mode status node %u [ %s ]\r\n"), transfer->metadata.remote_node_id,
                                    clCanard.slave[queueId].heartbeat.get_power_mode() == Power_Mode::pwr_on ? "full power" : "deep sleep");
                    // Rientro in OnLINE da OFFLine o Init Gestino può (dovrebbe) essere esterna alla Call
                    // Inizializzo le variabili e gli stati necessari per Reset e corretta gestione
                    if (clCanard.slave[queueId].is_entered_online()) {
                        TRACE_INFO_F(F("Node is now entered ONLINE !!!\n\r"));
                        // Metto i Flag in sicurezza, laddove dove non eventualmente gestito
                        clCanard.slave[queueId].command.reset_pending();
                        clCanard.slave[queueId].register_access.reset_pending();
                        clCanard.slave[queueId].file_server.reset_pending();
                        clCanard.slave[queueId].rmap_service.reset_pending();
                        // Set system_status from local to static access (Set Node Online)
                        localSystemStatusLock->Take();
                        localSystemStatus->data_slave[queueId].is_online = true;
                        localSystemStatus->datetime.ptr_time_for_sensors_get_istant = 0;  // Force get istant data for display
                        localSystemStatusLock->Give();
                    }
                }
            }
        }
#ifdef USE_SUB_PUBLISH_SLAVE_DATA
        else {
            // Gestione messaggi pubblicazione dati dei moduli slave (gestisco come master)
            // Es. popalamento dati se attivato un log specifico o show valori su display
            // Il comando è opzionale perchè in request/response esiste già questa possibilità
            // Nodo rispondente leggo dalla coda la/le pubblicazioni attivate (MAX 1 x tipologia)
            uint8_t queueId = clCanard.getSlaveIstanceFromId(transfer->metadata.remote_node_id);
            // Se nodo correttammente allocato e gestito
            if (queueId != GENERIC_BVAL_UNDEFINED) {
                // Verifico se risposta del servizio corrisponde al chiamante (eventuali + servizi sotto...)
                // Gestione di tutti i servizi possibili allocabili, valido per tutti i nodi
                if (transfer->metadata.port_id == clCanard.slave[queueId].publisher.get_subject_id()) {
                    // *************            Service Modulo TH Response            *************
                    if (clCanard.slave[queueId].get_module_type() == Module_Type::th) {
                        // Processato il messaggio con il relativo Handler. OK
                        bKindMessageProcessed = true;
                        // Modulo TH, leggo e deserializzo il messaggio in ingresso
                        rmap_module_TH_1_0 msg = {0};
                        size_t size = transfer->payload_size;
                        if (rmap_module_TH_1_0_deserialize_(&msg, static_cast<uint8_t const *>(transfer->payload), &size) >= 0) {
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
        if (!bKindMessageProcessed) {
            // Gestione di un messaggio sottoscritto senza gestione. Se arrivo quà è un errore di sviluppo
            LOCAL_ASSERT(false);
        }
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
                clCanard.sendResponse(MEGA, &transfer->metadata, serialized_size, &serialized[0]);
            }
        } else if (transfer->metadata.port_id == uavcan_register_Access_1_0_FIXED_PORT_ID_) {
            uavcan_register_Access_Request_1_0 req = {0};
            size_t size = transfer->payload_size;
            TRACE_INFO_F(F("<<-- Ricevuto richiesta accesso ai registri\r\n"));
            if (uavcan_register_Access_Request_1_0_deserialize_(&req, static_cast<uint8_t const *>(transfer->payload), &size) >= 0) {
                const uavcan_register_Access_Response_1_0 resp = processRequestRegisterAccess(&req);
                uint8_t serialized[uavcan_register_Access_Response_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
                size_t serialized_size = sizeof(serialized);
                if (uavcan_register_Access_Response_1_0_serialize_(&resp, &serialized[0], &serialized_size) >= 0) {
                    clCanard.sendResponse(MEGA, &transfer->metadata, serialized_size, &serialized[0]);
                }
            }
        } else if (transfer->metadata.port_id == uavcan_register_List_1_0_FIXED_PORT_ID_) {
            uavcan_register_List_Request_1_0 req = {0};
            size_t size = transfer->payload_size;
            TRACE_INFO_F(F("<<-- Ricevuto richiesta lettura elenco registri\r\n"));
            if (uavcan_register_List_Request_1_0_deserialize_(&req, static_cast<uint8_t const *>(transfer->payload), &size) >= 0) {
                uavcan_register_List_Response_1_0 resp;
                resp.name = localRegister->getNameByIndex(req.index);
                uint8_t serialized[uavcan_register_List_Response_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
                size_t serialized_size = sizeof(serialized);
                if (uavcan_register_List_Response_1_0_serialize_(&resp, &serialized[0], &serialized_size) >= 0) {
                    clCanard.sendResponse(CANARD_REGISTERLIST_TRANSFER_ID_TIMEOUT_USEC, &transfer->metadata, serialized_size, &serialized[0]);
                }
            }
        } else if (transfer->metadata.port_id == uavcan_node_ExecuteCommand_1_1_FIXED_PORT_ID_) {
            uavcan_node_ExecuteCommand_Request_1_1 req = {0};
            size_t size = transfer->payload_size;
            TRACE_INFO_F(F("<<-- Ricevuto comando esterno\r\n"));
            if (uavcan_node_ExecuteCommand_Request_1_1_deserialize_(&req, static_cast<uint8_t const *>(transfer->payload), &size) >= 0) {
                const uavcan_node_ExecuteCommand_Response_1_1 resp = processRequestExecuteCommand(clCanard, &req, transfer->metadata.remote_node_id);
                uint8_t serialized[uavcan_node_ExecuteCommand_Response_1_1_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
                size_t serialized_size = sizeof(serialized);
                if (uavcan_node_ExecuteCommand_Response_1_1_serialize_(&resp, &serialized[0], &serialized_size) >= 0) {
                    clCanard.sendResponse(MEGA, &transfer->metadata, serialized_size, &serialized[0]);
                }
            }
        } else if (transfer->metadata.port_id == uavcan_file_Read_1_1_FIXED_PORT_ID_) {
            // La funzione viene eseguita solo con UPLOAD Sequenza corretta
            // Serve a fare eseguire un'eventuale TimeOut su procedura non corretta
            // Ed evitare blocchi non coerenti... Viene gestita senza problemi
            // Send multiplo di File e procedure in sequenza ( Gestito TimeOut di Request )
            uint8_t queueId = clCanard.getSlaveIstanceFromId(transfer->metadata.remote_node_id);
            // Devo essere in aggiornamento per sicurezza!!! O al massimo con verifica del comando
            if ((clCanard.slave[queueId].file_server.get_state() == clCanard.state_uploading) ||
                (clCanard.slave[queueId].file_server.get_state() == clCanard.command_wait)) {
                // Update TimeOut (Comunico request OK al Master, Slave sta scaricando)
                // Se Slave si blocca per TimeOut, esco dalla procedura dove gestita
                clCanard.slave[queueId].file_server.start_pending(NODE_REQFILE_TIMEOUT_US);
                uavcan_file_Read_Request_1_1 req = {0};
                size_t size = transfer->payload_size;
                if (uavcan_file_Read_Request_1_1_deserialize_(&req, static_cast<uint8_t const *>(transfer->payload), &size) >= 0) {
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

                    if (!localSystemStatus->flags.sd_card_ready) {
                        file_download_error = true;
                        TRACE_ERROR_F(F("File server: Reject request uploading file. SD CARD not ready [ %s ]\r\n"), ERROR_STRING);
                    } else {
                        // Is first block? Perform an request to open file with name into task sd card
                        if ((req.offset / FILE_GET_DATA_BLOCK_SIZE) == 0) {
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
                        firmwareDownloadChunck.board_id = queueId;  // Set BOARDS Require File (Need for multiserver file request)
                        // Pushing data request to queue task sd card
                        localDataFileGetRequestQueue->Enqueue(&firmwareDownloadChunck, 0);
                        // Waiting response from MMC with TimeOUT
                        memset(&sdcard_task_response, 0, sizeof(file_get_response_t));
                        LocalTaskWatchDog(FILE_IO_DATA_QUEUE_TIMEOUT);
                        file_download_error = !localDataFileGetResponseQueue->Dequeue(&sdcard_task_response, FILE_IO_DATA_QUEUE_TIMEOUT);
                        file_download_error |= !sdcard_task_response.done_operation;
                    }

                    // Block data is avaiable? -> Send to remote node
                    if (file_download_error) {
                        // any error? (optional retry... from remote node. File server wait another request)
                        TRACE_ERROR_F(F("File server: Download data file from queue error!!!\r\n"));
                        resp._error.value = uavcan_file_Error_1_0_IO_ERROR;
                        dataLen = 0;
                    } else {
                        // Send block
                        if (sdcard_task_response.block_lenght != FILE_GET_DATA_BLOCK_SIZE) {
                            // EOF Last Block (bytes read != FILE_GET_DATA_BLOCK_SIZE)
                            // No other response/request are to get
                            TRACE_INFO_F(F("File server: End of data block data, readed [ %lu ] bytes\r\n"), ((uint32_t)(firmwareDownloadChunck.block_id) * (uint32_t)(FILE_GET_DATA_BLOCK_SIZE)) + sdcard_task_response.block_lenght);
                        } else {
                            // Normal block, prepare for next block
                            TRACE_VERBOSE_F(F("File serve: Readed data block id: [ %d ]\r\n"), firmwareDownloadChunck.block_id);
                        }
                    }
                    // *********** END GET Data File (Firmware) from SD CARD Queue ************
                    // Preparo la risposta corretta
                    resp.data.value.count = (size_t)sdcard_task_response.block_lenght;
                    memcpy(resp.data.value.elements, sdcard_task_response.block, resp.data.value.count);
                    uint8_t serialized[uavcan_file_Read_Response_1_1_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
                    size_t serialized_size = sizeof(serialized);
                    if (uavcan_file_Read_Response_1_1_serialize_(&resp, &serialized[0], &serialized_size) >= 0) {
                        clCanard.slave[queueId].file_server.reset_pending(resp.data.value.count);
                        clCanard.sendResponse(CANARD_READFILE_TRANSFER_ID_TIMEOUT_USEC, &transfer->metadata, serialized_size, &serialized[0]);
                    }
                }
            }
        } else {
            // Gestione di una richiesta senza controllore locale. Se arrivo quà è un errore di sviluppo
            LOCAL_ASSERT(false);
        }
    }
    // Gestione delle risposte alle richeste inviate alla rete come Master
    else if (transfer->metadata.transfer_kind == CanardTransferKindResponse) {
        // Comando inviato ad un nodo remoto, verifica della risposta e della coerenza messaggio
        if (transfer->metadata.port_id == uavcan_node_ExecuteCommand_1_1_FIXED_PORT_ID_) {
            uavcan_node_ExecuteCommand_Response_1_1 resp = {0};
            size_t size = transfer->payload_size;
            if (uavcan_node_ExecuteCommand_Response_1_1_deserialize_(&resp, static_cast<uint8_t const *>(transfer->payload), &size) >= 0) {
                // Ricerco idNodo nella coda degli allocati del master
                // Copio la risposta ricevuta nella struttura relativa e resetto il flag pending
                uint8_t queueId = clCanard.getSlaveIstanceFromId(transfer->metadata.remote_node_id);
                if (queueId != GENERIC_BVAL_UNDEFINED) {
                    // Resetta il pending del comando del nodo verificato
                    clCanard.slave[queueId].command.reset_pending(resp.status);
                }
            }
        } else if (transfer->metadata.port_id == uavcan_register_Access_1_0_FIXED_PORT_ID_) {
            uavcan_register_Access_Response_1_0 resp = {0};
            size_t size = transfer->payload_size;
            if (uavcan_register_Access_Response_1_0_deserialize_(&resp, static_cast<uint8_t const *>(transfer->payload), &size) >= 0) {
                // Ricerco idNodo nella coda degli allocati del master
                // Copio la risposta ricevuta nella struttura relativa e resetto il flag pending
                uint8_t queueId = clCanard.getSlaveIstanceFromId(transfer->metadata.remote_node_id);
                if (queueId != GENERIC_BVAL_UNDEFINED) {
                    // Resetta il pending del comando del nodo verificato con la risposta
                    clCanard.slave[queueId].register_access.reset_pending(resp.value);
                }
            }
        } else if (transfer->metadata.port_id == uavcan_file_Read_1_1_FIXED_PORT_ID_) {
            // Accetto solo messaggi indirizzati dal node_id che ha fatto la richiesta di upload
            // E' una sicurezza per il controllo dell'upload, ed evità errori di interprete
            // Inoltre non accetta messaggi extra standard UAVCAN, necessarià prima la CALL al comando
            // SEtFirmwareUpload o SetFileUpload, che impostano il node_id, resettato su EOF dalla classe
            if (clCanard.master.file.get_server_node() == transfer->metadata.remote_node_id) {
                uavcan_file_Read_Response_1_1 resp = {0};
                size_t size = transfer->payload_size;
                if (uavcan_file_Read_Response_1_1_deserialize_(&resp, static_cast<uint8_t const *>(transfer->payload), &size) >= 0) {
                    if (clCanard.master.file.is_firmware()) {
                        TRACE_VERBOSE_F(F("RX FIRMWARE READ BLOCK LEN: "));
                    } else {
                        TRACE_VERBOSE_F(F("RX FILE READ BLOCK LEN: "));
                    }
                    TRACE_VERBOSE_F(F("%d\r\n"), resp.data.value.count);
                    // Save Data in Flash File at Block Position (Init = Rewrite file...)
                    if (putFlashFile(clCanard.master.file.get_name(), clCanard.master.file.is_firmware(), clCanard.master.file.is_first_data_block(),
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
        else {
            // Nodo rispondente (posso avere senza problemi più servizi con stesso port_id su diversi nodi)
            uint8_t queueId = clCanard.getSlaveIstanceFromId(transfer->metadata.remote_node_id);
            // Se nodo correttammente allocato e gestito
            if (queueId != GENERIC_BVAL_UNDEFINED) {
                // Verifico se risposta del servizio corrisponde al chiamante (eventuali + servizi sotto...)
                // Gestione di tutti i servizi possibili allocabili, valido per tutti i nodi
                if (transfer->metadata.port_id == clCanard.slave[queueId].rmap_service.get_port_id()) {
                    // Utilizzo un buffer di allocazione per RX Messaggio in Casting da postare sul meteodo Slave Canard
                    uint8_t castLocalBuffer[RMAP_DATA_MAX_ELEMENT_SIZE];
                    memset(castLocalBuffer, 0, sizeof(castLocalBuffer));
                    // Resetta il pending del comando del nodo verificato (size_mem preparato in avvio)
                    // Copia la risposta nella variabile di chiamata in state
                    // N.B. possibile gestire qua tutte le occorrenze per stima V4
                    if (clCanard.slave[queueId].get_module_type() == Module_Type::th) {
                        rmap_service_module_TH_Response_1_0 *respCastTH = (rmap_service_module_TH_Response_1_0 *)castLocalBuffer;
                        size_t size = transfer->payload_size;
                        if (rmap_service_module_TH_Response_1_0_deserialize_(respCastTH, static_cast<uint8_t const *>(transfer->payload), &size) >= 0) {
                            clCanard.slave[queueId].rmap_service.reset_pending(respCastTH, sizeof(*respCastTH));
                        }
                    }
                    if (clCanard.slave[queueId].get_module_type() == Module_Type::rain) {
                        rmap_service_module_Rain_Response_1_0 *respCastRain = (rmap_service_module_Rain_Response_1_0 *)castLocalBuffer;
                        size_t size = transfer->payload_size;
                        if (rmap_service_module_Rain_Response_1_0_deserialize_(respCastRain, static_cast<uint8_t const *>(transfer->payload), &size) >= 0) {
                            clCanard.slave[queueId].rmap_service.reset_pending(respCastRain, sizeof(*respCastRain));
                        }
                    }
                    if (clCanard.slave[queueId].get_module_type() == Module_Type::wind) {
                        rmap_service_module_Wind_Response_1_0 *respCastWind = (rmap_service_module_Wind_Response_1_0 *)castLocalBuffer;
                        size_t size = transfer->payload_size;
                        if (rmap_service_module_Wind_Response_1_0_deserialize_(respCastWind, static_cast<uint8_t const *>(transfer->payload), &size) >= 0) {
                            clCanard.slave[queueId].rmap_service.reset_pending(respCastWind, sizeof(*respCastWind));
                        }
                    }
                    if (clCanard.slave[queueId].get_module_type() == Module_Type::radiation) {
                        rmap_service_module_Radiation_Response_1_0 *respCastRadiation = (rmap_service_module_Radiation_Response_1_0 *)castLocalBuffer;
                        size_t size = transfer->payload_size;
                        if (rmap_service_module_Radiation_Response_1_0_deserialize_(respCastRadiation, static_cast<uint8_t const *>(transfer->payload), &size) >= 0) {
                            clCanard.slave[queueId].rmap_service.reset_pending(respCastRadiation, sizeof(*respCastRadiation));
                        }
                    }
                    if (clCanard.slave[queueId].get_module_type() == Module_Type::vwc) {
                        rmap_service_module_VWC_Response_1_0 *respCastVWC = (rmap_service_module_VWC_Response_1_0 *)castLocalBuffer;
                        size_t size = transfer->payload_size;
                        if (rmap_service_module_VWC_Response_1_0_deserialize_(respCastVWC, static_cast<uint8_t const *>(transfer->payload), &size) >= 0) {
                            clCanard.slave[queueId].rmap_service.reset_pending(respCastVWC, sizeof(*respCastVWC));
                        }
                    }
                    if (clCanard.slave[queueId].get_module_type() == Module_Type::power) {
                        rmap_service_module_Power_Response_1_0 *respCastPower = (rmap_service_module_Power_Response_1_0 *)castLocalBuffer;
                        size_t size = transfer->payload_size;
                        if (rmap_service_module_Power_Response_1_0_deserialize_(respCastPower, static_cast<uint8_t const *>(transfer->payload), &size) >= 0) {
                            clCanard.slave[queueId].rmap_service.reset_pending(respCastPower, sizeof(*respCastPower));
                        }
                    }
                }
            }
        }
    } else {
        // Se arrivo quà è un errore di sviluppo, controllare setup sottoscrizioni, risposte e messaggi
        LOCAL_ASSERT(false);
    }
}

/// *********************************************************************************************
/// @brief Main TASK && INIT TASK --- UAVCAN
/// *********************************************************************************************
CanTask::CanTask(const char *taskName, uint16_t stackSize, uint8_t priority, CanParam_t CanParam) : Thread(taskName, stackSize, priority), param(CanParam) {
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
    val.natural16.value.count = 1;
    val.natural16.value.elements[0] = CAN_MTU_BASE;  // CAN_CLASSIC MTU 8
    localRegisterAccessLock->Take();
    localRegister->read(REGISTER_UAVCAN_MTU, &val);
    localRegisterAccessLock->Give();
    LOCAL_ASSERT(uavcan_register_Value_1_0_is_natural16_(&val) && (val.natural16.value.count == 1));

    // *******************         CANARD SETUP TIMINGS AND SPEED        *******************
    // CAN BITRATE Dinamico su LoadRegister (CAN_FD 2xREG natural32 0=Speed, 1=0 (Not Used))
    uavcan_register_Value_1_0_select_natural32_(&val);
    val.natural32.value.count = 2;
    val.natural32.value.elements[0] = CAN_BIT_RATE;
    val.natural32.value.elements[1] = 0ul;  // Ignored for CANARD_MTU_CAN_CLASSIC
    localRegisterAccessLock->Take();
    localRegister->read(REGISTER_UAVCAN_BITRATE, &val);
    localRegisterAccessLock->Give();
    LOCAL_ASSERT(uavcan_register_Value_1_0_is_natural32_(&val) && (val.natural32.value.count == 2));

    // ************************************************************************
    // ***************************** TEST BEGIN *******************************
    // ************************************************************************

    UNITY_BEGIN();

    // Necessary delay for test
    delay(3000);

    // Dynamic BIT RATE Change CAN Speed to CAN_BIT_RATE (register default/defined)
    BxCANTimings timings;
    result = bxCANComputeTimings(HAL_RCC_GetPCLK1Freq(), val.natural32.value.elements[0], &timings);
    RUN_TEST(test_init_bxcancomputetimings_on_chip);
    if (!result) {
        TRACE_VERBOSE_F(F("Error redefinition bxCANComputeTimings, try loading default...\r\n"));
        val.natural32.value.count = 2;
        val.natural32.value.elements[0] = CAN_BIT_RATE;
        val.natural32.value.elements[1] = 0ul;  // Ignored for CANARD_MTU_CAN_CLASSIC
        localRegisterAccessLock->Take();
        localRegister->write(REGISTER_UAVCAN_BITRATE, &val);
        localRegisterAccessLock->Give();
        result = bxCANComputeTimings(HAL_RCC_GetPCLK1Freq(), val.natural32.value.elements[0], &timings);
        RUN_TEST(test_init_bxcancomputetimings_on_chip);
        if (!result) {
            UNITY_END();
            return;
        }
    }

// HW Setup solo con modulo CAN Attivo
#if (ENABLE_CAN)

    // Configurea bxCAN speed && mode
    result = bxCANConfigure(0, timings, false);
    RUN_TEST(test_init_bxcanconfigure_on_chip);
    if (!result) {
        UNITY_END();
        return;
    }

    // *******************     CANARD SETUP TIMINGS AND SPEED COMPLETE   *******************

    // Check error starting CAN
    RUN_TEST(test_init_hal_can_stm);

    // Enable Interrupt RX Standard CallBack -> CAN1_RX0_IRQHandler
    RUN_TEST(test_init_hal_can_interrupt);
     
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
void CanTask::TaskMonitorStack() {
    u_int16_t stackUsage = (u_int16_t)uxTaskGetStackHighWaterMark(NULL);
    if ((stackUsage) && (stackUsage < param.system_status->tasks[LOCAL_TASK_ID].stack)) {
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
void CanTask::TaskWatchDog(uint32_t millis_standby) {
    // Local TaskWatchDog update
    param.systemStatusLock->Take();
    // Update WDT Signal (Direct or Long function Timered)
    if (millis_standby) {
        // Check 1/2 Freq. controller ready to WDT only SET flag
        if ((millis_standby) < WDT_CONTROLLER_MS / 2) {
            param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::set;
        } else {
            param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::timer;
            // Add security milimal Freq to check
            param.system_status->tasks[LOCAL_TASK_ID].watch_dog_ms = millis_standby + WDT_CONTROLLER_MS;
        }
    } else
        param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::set;
    param.systemStatusLock->Give();
}

/// @brief local watchDog and Sleep flag Task (optional) access from member static
/// @param status system_status_t Status STIMAV4
/// @param lock if used (!=NULL) Semaphore locking system status access
/// @param millis_standby time in ms to perfor check of WDT. If longer than WDT Reset, WDT is temporanly suspend
void CanTask::LocalTaskWatchDog(uint32_t millis_standby) {
    // Local TaskWatchDog update
    localSystemStatusLock->Take();
    // Update WDT Signal (Direct or Long function Timered)
    if (millis_standby) {
        // Check 1/2 Freq. controller ready to WDT only SET flag
        if ((millis_standby) < WDT_CONTROLLER_MS / 2) {
            localSystemStatus->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::set;
        } else {
            localSystemStatus->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::timer;
            // Add security milimal Freq to check
            localSystemStatus->tasks[LOCAL_TASK_ID].watch_dog_ms = millis_standby + WDT_CONTROLLER_MS;
        }
    } else
        localSystemStatus->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::set;
    localSystemStatusLock->Give();
}

/// @brief local suspend flag and positor running state Task (optional)
/// @param state_position Sw_Position (Local STATE)
/// @param state_subposition Sw_SubPosition (Optional Local SUB_STATE Position Monitor)
/// @param state_operation operative mode flag status for this task
void CanTask::TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation) {
    // Local TaskWatchDog update
    param.systemStatusLock->Take();
    // Signal Task sleep/disabled mode from request (Auto SET WDT on Resume)
    if ((param.system_status->tasks[LOCAL_TASK_ID].state == task_flag::suspended) &&
        (state_operation == task_flag::normal))
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

    // RMAP Queue data Put to memory MMC/SD Card
    rmap_archive_data_t rmap_archive_data;

    // Set when Firmware Upgrade is required
    bool start_firmware_upgrade = false;

    // Starting message trace
    bool message_traced = false;

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

                // Waiting loading configuration complete before start application
                if (param.system_status->configuration.is_loaded) {
                    state = CAN_STATE_INIT;
                } else {
                    if (!message_traced) {
                        TRACE_INFO_F(F("Can task: Waiting configuration before START\r\n"));
                        message_traced = true;
                    }
                    break;
                }

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
                clCanard.set_canard_node_id((CanardNodeID)NODE_MASTER_ID);
#else
                uavcan_register_Value_1_0_select_natural16_(&val);
                val.natural16.value.count = 1;
                val.natural16.value.elements[0] = UINT16_MAX;  // This means undefined (anonymous), per Specification/libcanard.
                localRegisterAccessLock->Take();
                localRegister->read(REGISTER_UAVCAN_NODE_ID, &val);  // The names of the standard registers are regulated by the Specification.
                localRegisterAccessLock->Give();
                LOCAL_ASSERT(uavcan_register_Value_1_0_is_natural16_(&val) && (val.natural16.value.count == 1));
                if (val.natural16.value.elements[0] <= CANARD_NODE_ID_MAX) {
                    clCanard.set_canard_node_id((CanardNodeID)val.natural16.value.elements[0]);
                } else {
                    // Master must start with an ID Node (if not registerede start with default value NODE_MASTER_ID)
                    clCanard.set_canard_node_id((CanardNodeID)NODE_MASTER_ID);
                }
#endif

                // The description register is optional but recommended because it helps constructing/maintaining large networks.
                // It simply keeps a human-readable description of the node that should be empty by default.
                uavcan_register_Value_1_0_select_string_(&val);
                val._string.value.count = 0;
                localRegisterAccessLock->Take();
                localRegister->read(REGISTER_UAVCAN_NODE_DESCR, &val);  // We don't need the value, we just need to ensure it exists.
                localRegisterAccessLock->Give();

                // **********************************************************************************
                // Setup configuration module node and start canard class slave istance with loaded ID
                // **********************************************************************************
                for (uint8_t iCnt = 0; iCnt < MAX_NODE_CONNECT; iCnt++) {
#ifdef USE_SUB_PUBLISH_SLAVE_DATA
                    // If valid address, configure node
                    if (param.configuration->board_slave[iCnt].can_address <= CANARD_NODE_ID_MAX) {
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

                is_init_subs_ok = true;

                TRACE_INFO_F(F("Can task: STARTING UAVCAV Subscrition and Service\r\n"));

                // Service servers: -> Risposta per GetNodeInfo richiesta esterna (Yakut, Altri)
                if (!clCanard.rxSubscribe(CanardTransferKindRequest,
                                          uavcan_node_GetInfo_1_0_FIXED_PORT_ID_,
                                          uavcan_node_GetInfo_Request_1_0_EXTENT_BYTES_,
                                          CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                    is_init_subs_ok = false;
                }

                // Service servers: -> Chiamata per ExecuteCommand richiesta esterna (Yakut, Altri)
                if (!clCanard.rxSubscribe(CanardTransferKindRequest,
                                          uavcan_node_ExecuteCommand_1_1_FIXED_PORT_ID_,
                                          uavcan_node_ExecuteCommand_Request_1_1_EXTENT_BYTES_,
                                          CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                    is_init_subs_ok = false;
                }

                // Service servers: -> Risposta per Accesso ai registri richiesta esterna (Yakut, Altri)
                if (!clCanard.rxSubscribe(CanardTransferKindRequest,
                                          uavcan_register_Access_1_0_FIXED_PORT_ID_,
                                          uavcan_register_Access_Request_1_0_EXTENT_BYTES_,
                                          CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                    is_init_subs_ok = false;
                }

                // Service servers: -> Risposta per Lista dei registri richiesta esterna (Yakut, Altri)
                if (!clCanard.rxSubscribe(CanardTransferKindRequest,
                                          uavcan_register_List_1_0_FIXED_PORT_ID_,
                                          uavcan_register_List_Request_1_0_EXTENT_BYTES_,
                                          CANARD_REGISTERLIST_TRANSFER_ID_TIMEOUT_USEC)) {
                    is_init_subs_ok = false;
                }

                // ******* SOTTOSCRIZIONE MESSAGGI / COMANDI E SERVIZI AD UTILITA' MASTER ********

                // Messaggi PNP_Allocation: -> Allocazione dei nodi standard in PlugAndPlay per i nodi conosciuti
                if (!clCanard.rxSubscribe(CanardTransferKindMessage,
                                          uavcan_pnp_NodeIDAllocationData_1_0_FIXED_PORT_ID_,
                                          uavcan_pnp_NodeIDAllocationData_1_0_EXTENT_BYTES_,
                                          CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                    is_init_subs_ok = false;
                }

                // Messaggi HEARTBEAT: -> Verifica della presenza per stato Nodi (Slave) OnLine / OffLine
                if (!clCanard.rxSubscribe(CanardTransferKindMessage,
                                          uavcan_node_Heartbeat_1_0_FIXED_PORT_ID_,
                                          uavcan_node_Heartbeat_1_0_EXTENT_BYTES_,
                                          CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                    is_init_subs_ok = false;
                }

                // Service client: -> Risposta per ExecuteCommand richiesta interna (come master)
                if (!clCanard.rxSubscribe(CanardTransferKindResponse,
                                          uavcan_node_ExecuteCommand_1_1_FIXED_PORT_ID_,
                                          uavcan_node_ExecuteCommand_Response_1_1_EXTENT_BYTES_,
                                          CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                    is_init_subs_ok = false;
                }

                // Service client: -> Risposta per Accesso ai registri richiesta interna (come master)
                if (!clCanard.rxSubscribe(CanardTransferKindResponse,
                                          uavcan_register_Access_1_0_FIXED_PORT_ID_,
                                          uavcan_register_Access_Response_1_0_EXTENT_BYTES_,
                                          CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                    is_init_subs_ok = false;
                }

                // Service client: -> Risposta per Read (Receive) File local richiesta esterna (Yakut, Altri)
                if (!clCanard.rxSubscribe(CanardTransferKindResponse,
                                          uavcan_file_Read_1_1_FIXED_PORT_ID_,
                                          uavcan_file_Read_Response_1_1_EXTENT_BYTES_,
                                          CANARD_READFILE_TRANSFER_ID_TIMEOUT_USEC)) {
                    is_init_subs_ok = false;
                }

                // Service server: -> Risposta per Read (Request Slave) File read archivio (come master)
                if (!clCanard.rxSubscribe(CanardTransferKindRequest,
                                          uavcan_file_Read_1_1_FIXED_PORT_ID_,
                                          uavcan_file_Read_Request_1_1_EXTENT_BYTES_,
                                          CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                    is_init_subs_ok = false;
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
                    // Se previsto il servizio request/response (!=NULL, quindi allocato) con port_id valido
                    if ((clCanard.slave[queueId].rmap_service.get_response()) &&
                        (clCanard.slave[queueId].rmap_service.get_port_id() <= CANARD_SERVICE_ID_MAX)) {
                        // Controllo le varie tipologie di request/service per il nodo
                        if (clCanard.slave[queueId].get_module_type() == Module_Type::th) {
                            // Alloco la stottoscrizione in funzione del tipo di modulo
                            // Service client: -> Risposta per ServiceDataModuleTH richiesta interna (come master)
                            if (!clCanard.rxSubscribe(CanardTransferKindResponse,
                                                      clCanard.slave[queueId].rmap_service.get_port_id(),
                                                      rmap_service_module_TH_Response_1_0_EXTENT_BYTES_,
                                                      CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                                is_init_subs_ok = false;
                            }
                        }
                        // Controllo le varie tipologie di request/service per il nodo
                        if (clCanard.slave[queueId].get_module_type() == Module_Type::rain) {
                            // Alloco la stottoscrizione in funzione del tipo di modulo
                            // Service client: -> Risposta per ServiceDataModuleTH richiesta interna (come master)
                            if (!clCanard.rxSubscribe(CanardTransferKindResponse,
                                                      clCanard.slave[queueId].rmap_service.get_port_id(),
                                                      rmap_service_module_Rain_Response_1_0_EXTENT_BYTES_,
                                                      CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                                is_init_subs_ok = false;
                            }
                        }
                        // Controllo le varie tipologie di request/service per il nodo
                        if (clCanard.slave[queueId].get_module_type() == Module_Type::wind) {
                            // Alloco la stottoscrizione in funzione del tipo di modulo
                            // Service client: -> Risposta per ServiceDataModuleTH richiesta interna (come master)
                            if (!clCanard.rxSubscribe(CanardTransferKindResponse,
                                                      clCanard.slave[queueId].rmap_service.get_port_id(),
                                                      rmap_service_module_Wind_Response_1_0_EXTENT_BYTES_,
                                                      CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                                is_init_subs_ok = false;
                            }
                        }
                        // Controllo le varie tipologie di request/service per il nodo
                        if (clCanard.slave[queueId].get_module_type() == Module_Type::radiation) {
                            // Alloco la stottoscrizione in funzione del tipo di modulo
                            // Service client: -> Risposta per ServiceDataModuleTH richiesta interna (come master)
                            if (!clCanard.rxSubscribe(CanardTransferKindResponse,
                                                      clCanard.slave[queueId].rmap_service.get_port_id(),
                                                      rmap_service_module_Radiation_Response_1_0_EXTENT_BYTES_,
                                                      CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                                is_init_subs_ok = false;
                            }
                        }
                        // Controllo le varie tipologie di request/service per il nodo
                        if (clCanard.slave[queueId].get_module_type() == Module_Type::vwc) {
                            // Alloco la stottoscrizione in funzione del tipo di modulo
                            // Service client: -> Risposta per ServiceDataModuleTH richiesta interna (come master)
                            if (!clCanard.rxSubscribe(CanardTransferKindResponse,
                                                      clCanard.slave[queueId].rmap_service.get_port_id(),
                                                      rmap_service_module_VWC_Response_1_0_EXTENT_BYTES_,
                                                      CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                                is_init_subs_ok = false;
                            }
                        }
                        // Controllo le varie tipologie di request/service per il nodo
                        if (clCanard.slave[queueId].get_module_type() == Module_Type::power) {
                            // Alloco la stottoscrizione in funzione del tipo di modulo
                            // Service client: -> Risposta per ServiceDataModuleTH richiesta interna (come master)
                            if (!clCanard.rxSubscribe(CanardTransferKindResponse,
                                                      clCanard.slave[queueId].rmap_service.get_port_id(),
                                                      rmap_service_module_Power_Response_1_0_EXTENT_BYTES_,
                                                      CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                                is_init_subs_ok = false;
                            }
                        }
                    }
#ifdef USE_SUB_PUBLISH_SLAVE_DATA
                    // *************   PUBLISH    *************
                    // Se previsto il servizio publisher (subject_id valido)
                    // Non alloco niente per il publish (gestione esempio display o altro debug interno da gestire)
                    if (clCanard.slave[queueId].publisher.get_subject_id() <= CANARD_SUBJECT_ID_MAX) {
                        // Controllo le varie tipologie di request/service per il nodo
                        if (clCanard.slave[queueId].get_module_type() == Module_Type::th) {
                            // Alloco la stottoscrizione in funzione del tipo di modulo
                            // Service client: -> Sottoscrizione per ModuleTH (come master)
                            if (!clCanard.rxSubscribe(CanardTransferKindMessage,
                                                      clCanard.slave[queueId].publisher.get_subject_id(),
                                                      rmap_module_TH_1_0_EXTENT_BYTES_,
                                                      CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                                is_init_subs_ok = false;
                            }
                        }
                    }
#endif
                }

                RUN_TEST(test_init_subscriptions_uavcan);

                // Passo alla gestione Main
                state = CAN_STATE_CHECK;
                message_traced = false;

                break;

            // ********************************************************************************
            //         AVVIA LOOP CANARD PRINCIPALE gestione TX/RX Code -> Messaggi
            // ********************************************************************************
            case CAN_STATE_CHECK:
                if (!message_traced) {
                    message_traced = true;

                    UNITY_END();

                    // ************************************************************************
                    // ***************************** TEST END *********************************
                    // ************************************************************************
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
        if (clCanard.master.file.download_request() || param.system_status->flags.file_server_running) {
            DelayUntil(Ticks::MsToTicks(TASK_WAIT_REALTIME_DELAY_MS));
        } else {
            DelayUntil(Ticks::MsToTicks(CAN_TASK_WAIT_DELAY_MS));
        }
    }
}

#endif