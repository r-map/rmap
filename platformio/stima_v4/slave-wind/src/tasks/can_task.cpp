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

/// @brief Legge il subjectID per il modulo corrente per la risposta al servizio di gestione dati.
/// @param modeAccessID tipo di accesso
/// @param port_name nome porta uavcan
/// @param type_name tipo nome registro
/// @return Canard Port ID associato
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
    static uavcan_register_Value_1_0 val = {0};
    uavcan_register_Value_1_0_select_natural16_(&val);
    val.natural16.value.count = 1;
    val.natural16.value.elements[0] = UINT16_MAX;  // This means "undefined", per Specification, which is the default.

    // Read the register with defensive self-checks.
    localRegisterAccessLock->Take();
    localRegister->read(&register_name[0], &val);
    localRegisterAccessLock->Give();
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
    localRegisterAccessLock->Take();
    localRegister->read(&register_name[0], &val);
    localRegisterAccessLock->Give();

    return result;
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
            canFlashBlock = canFlashPtr / AT25SF161_BLOCK_SIZE;
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
            if((canFlashPtr / AT25SF161_BLOCK_SIZE) != canFlashBlock) {
                canFlashBlock = canFlashPtr / AT25SF161_BLOCK_SIZE;
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

// ***********************************************************************************************
// ***********************************************************************************************
//      FUNZIONI CHIAMATE DA MAIN_LOOP DI PUBBLICAZIONE E RICHIESTE DATI E SERVIZI
// ***********************************************************************************************
// ***********************************************************************************************

// *******                      FUNZIONI RMAP PUBLISH LOCAL DATA                         *********

/// @brief Prepara il blocco messaggio dati per il modulo corrente istantaneo
///    NB: Aggiorno solo i dati fisici in questa funzione i metadati sono esterni
/// @param sensore tipo di sensore richiesto rmap class_canard di modulo
/// @param rmap_data report data module output value per modulo sensore specifico publish
///                  oppure in overload metodo tramite metodo Response applucapile al servizio request
/// @return None
/// TODO:_TH_RAIN (Sistemare report/connfidence/values...)
/// Controllo DWx .... Corretto sensore...
void CanTask::prepareSensorsDataValue(uint8_t const sensore, const report_t *report, rmap_module_Wind_1_0 *rmap_data) {
    // Inserisco i dati reali
    switch (sensore) {
        case canardClass::Sensor_Type::dwa:
            // Prepara i dati DWA (Sample)
            rmap_data->DWA.speed.val.value = report->vavg10_speed;
            rmap_data->DWA.speed.confidence.value = report->quality;
            rmap_data->DWA.direction.val.value = report->vavg10_direction;
            rmap_data->DWA.direction.confidence.value = report->quality;
            break;
        case canardClass::Sensor_Type::dwb:
            // Prepara i dati DWB (Sample)
            rmap_data->DWB.speed.val.value = report->vavg_speed;
            rmap_data->DWB.speed.confidence.value = report->quality;
            rmap_data->DWB.direction.val.value = report->vavg_direction;
            rmap_data->DWB.direction.confidence.value = report->quality;
            break;
        case canardClass::Sensor_Type::dwc:
            // Prepara i dati DWC (Sample)
            rmap_data->DWC.peak.val.value = report->peak_gust_speed;
            rmap_data->DWC.peak.confidence.value = report->quality;
            rmap_data->DWC._long.val.value = report->long_gust_speed;
            rmap_data->DWC._long.confidence.value = report->quality;
            break;
        case canardClass::Sensor_Type::dwd:
            // Prepara i dati SMP (Sample)
            rmap_data->DWD.speed.val.value = report->avg_speed;
            rmap_data->DWD.speed.confidence.value = report->quality;
            break;
        case canardClass::Sensor_Type::dwe:
            // Prepara i dati SMP (Sample)
            rmap_data->DWE.class1.val.value = report->class_1;
            rmap_data->DWE.class1.confidence.value = report->quality;
            rmap_data->DWE.class2.val.value = report->class_2;
            rmap_data->DWE.class2.confidence.value = report->quality;
            rmap_data->DWE.class3.val.value = report->class_3;
            rmap_data->DWE.class3.confidence.value = report->quality;
            rmap_data->DWE.class4.val.value = report->class_4;
            rmap_data->DWE.class4.confidence.value = report->quality;
            rmap_data->DWE.class5.val.value = report->class_5;
            rmap_data->DWE.class5.confidence.value = report->quality;
            rmap_data->DWE.class6.val.value = report->class_6;
            rmap_data->DWE.class6.confidence.value = report->quality;
            break;
        case canardClass::Sensor_Type::dwf:
            // Prepara i dati SMP (Sample)
            rmap_data->DWF.peak.val.value = report->peak_gust_direction;
            rmap_data->DWF.peak.confidence.value = report->quality;
            rmap_data->DWF._long.val.value = report->long_gust_direction;
            rmap_data->DWF._long.confidence.value = report->quality;
            break;
    }
}
void CanTask::prepareSensorsDataValue(uint8_t const sensore, const report_t *report, rmap_service_module_Wind_Response_1_0 *rmap_data) {
    // Inserisco i dati reali
    switch (sensore) {
        case canardClass::Sensor_Type::dwa:
            // Prepara i dati DWA (Sample)
            rmap_data->DWA.speed.val.value = report->vavg10_speed;
            rmap_data->DWA.speed.confidence.value = report->quality;
            rmap_data->DWA.direction.val.value = report->vavg10_direction;
            rmap_data->DWA.direction.confidence.value = report->quality;
            break;
        case canardClass::Sensor_Type::dwb:
            // Prepara i dati DWB (Sample)
            rmap_data->DWB.speed.val.value = report->vavg_speed;
            rmap_data->DWB.speed.confidence.value = report->quality;
            rmap_data->DWB.direction.val.value = report->vavg_direction;
            rmap_data->DWB.direction.confidence.value = report->quality;
            break;
        case canardClass::Sensor_Type::dwc:
            // Prepara i dati DWC (Sample)
            rmap_data->DWC.peak.val.value = report->peak_gust_speed;
            rmap_data->DWC.peak.confidence.value = report->quality;
            rmap_data->DWC._long.val.value = report->long_gust_speed;
            rmap_data->DWC._long.confidence.value = report->quality;
            break;
        case canardClass::Sensor_Type::dwd:
            // Prepara i dati SMP (Sample)
            rmap_data->DWD.speed.val.value = report->avg_speed;
            rmap_data->DWD.speed.confidence.value = report->quality;
            break;
        case canardClass::Sensor_Type::dwe:
            // Prepara i dati SMP (Sample)
            rmap_data->DWE.class1.val.value = report->class_1;
            rmap_data->DWE.class1.confidence.value = report->quality;
            rmap_data->DWE.class2.val.value = report->class_2;
            rmap_data->DWE.class2.confidence.value = report->quality;
            rmap_data->DWE.class3.val.value = report->class_3;
            rmap_data->DWE.class3.confidence.value = report->quality;
            rmap_data->DWE.class4.val.value = report->class_4;
            rmap_data->DWE.class4.confidence.value = report->quality;
            rmap_data->DWE.class5.val.value = report->class_5;
            rmap_data->DWE.class5.confidence.value = report->quality;
            rmap_data->DWE.class6.val.value = report->class_6;
            rmap_data->DWE.class6.confidence.value = report->quality;
            break;
        case canardClass::Sensor_Type::dwf:
            // Prepara i dati SMP (Sample)
            rmap_data->DWF.peak.val.value = report->peak_gust_direction;
            rmap_data->DWF.peak.confidence.value = report->quality;
            rmap_data->DWF._long.val.value = report->long_gust_direction;
            rmap_data->DWF._long.confidence.value = report->quality;
            break;
    }
}

/// @brief Pubblica i dati RMAP con il metodo publisher se abilitato e configurato
/// @param clCanard classe
/// @param param parametri del modulo UAVCAN
void CanTask::publish_rmap_data(canardClass &clCanard, CanParam_t *param) {
    // Pubblica i dati del nodo corrente se abilitata la funzione e con il corretto subjectId
    // Ovviamente il nodo non può essere anonimo per la pubblicazione...
    if ((!clCanard.is_canard_node_anonymous()) &&
        (clCanard.publisher_enabled.module_wind) &&
        (clCanard.port_id.publisher_module_wind <= CANARD_SUBJECT_ID_MAX)) {
        rmap_module_Wind_1_0 module_wind_msg = {0};

        request_data_t request_data = {0};
        report_t report = {0};

        // preparo la struttura dati per richiedere i dati al task che li elabora
        // in publish non inizializzo coda, pibblico in funzione del'ultima riichiesta di CFG
        // Il dato report_time_s non risiede sullo slave ma è in chimata da master
        request_data.is_init = false;   // utilizza i dati esistenti (continua le elaborazioni precedentemente inizializzate)
        request_data.report_time_s = last_req_rpt_time;         // richiedo i dati in conformità a standard request (report)
        request_data.observation_time_s = last_req_obs_time;    // richiedo i dati in conformità a standard request (observation)

        // SET Dynamic metadata (Request data from master Only Data != Sample)
        clCanard.module_wind.DWA.metadata.timerange.P2 = 0;
        clCanard.module_wind.DWB.metadata.timerange.P2 = request_data.report_time_s;
        clCanard.module_wind.DWC.metadata.timerange.P2 = request_data.report_time_s;
        clCanard.module_wind.DWD.metadata.timerange.P2 = request_data.report_time_s;
        clCanard.module_wind.DWE.metadata.timerange.P2 = request_data.report_time_s;
        clCanard.module_wind.DWF.metadata.timerange.P2 = request_data.report_time_s;

        // coda di richiesta dati (senza attesa)
        param->requestDataQueue->Enqueue(&request_data, Ticks::MsToTicks(WAIT_QUEUE_REQUEST_ELABDATA_MS));

        // coda di attesa dati (attesa rmap_calc_data)
        if (param->reportDataQueue->Dequeue(&report, Ticks::MsToTicks(WAIT_QUEUE_RESPONSE_ELABDATA_MS))) {
        //   TRACE_INFO_F(F("--> CAN wind report\t%d\t%d\t%d\r\n"), (int32_t) report.wind.sample, (int32_t) report.wind.ist, (int32_t) report.wind.quality);
        }

        // Preparo i dati
        prepareSensorsDataValue(canardClass::Sensor_Type::dwa, &report, &module_wind_msg);
        prepareSensorsDataValue(canardClass::Sensor_Type::dwb, &report, &module_wind_msg);
        prepareSensorsDataValue(canardClass::Sensor_Type::dwc, &report, &module_wind_msg);
        prepareSensorsDataValue(canardClass::Sensor_Type::dwd, &report, &module_wind_msg);
        prepareSensorsDataValue(canardClass::Sensor_Type::dwe, &report, &module_wind_msg);
        prepareSensorsDataValue(canardClass::Sensor_Type::dwf, &report, &module_wind_msg);
        // Metadata
        module_wind_msg.DWA.metadata = clCanard.module_wind.DWA.metadata;
        module_wind_msg.DWB.metadata = clCanard.module_wind.DWB.metadata;
        module_wind_msg.DWC.metadata = clCanard.module_wind.DWC.metadata;
        module_wind_msg.DWD.metadata = clCanard.module_wind.DWD.metadata;
        module_wind_msg.DWE.metadata = clCanard.module_wind.DWE.metadata;
        module_wind_msg.DWF.metadata = clCanard.module_wind.DWF.metadata;

        // Serialize and publish the message:
        uint8_t serialized[rmap_module_Wind_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
        size_t serialized_size = sizeof(serialized);
        const int8_t err = rmap_module_Wind_1_0_serialize_(&module_wind_msg, &serialized[0], &serialized_size);
        LOCAL_ASSERT(err >= 0);
        if (err >= 0) {
            const CanardTransferMetadata meta = {
                .priority = CanardPrioritySlow,
                .transfer_kind = CanardTransferKindMessage,
                .port_id = clCanard.port_id.publisher_module_wind,
                .remote_node_id = CANARD_NODE_ID_UNSET,
                .transfer_id = (CanardTransferID)(clCanard.next_transfer_id.module_wind()),  // Increment!
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

/// @brief Plug and Play Slave, Versione 1.0 compatibile con CAN_CLASSIC MTU 8
///        Messaggi anonimi CAN non sono consentiti se messaggi > LUNGHEZZA MTU disponibile
/// @param clCanard classe
/// @param msg messaggio di pubblicazione
void CanTask::processMessagePlugAndPlayNodeIDAllocation(canardClass &clCanard,
                                                      const uavcan_pnp_NodeIDAllocationData_1_0* const msg) {
    // msg->unique_id_hash RX non gestito, è valido GetUniqueID Unificato per entrambe versioni V1 e V2
    if (msg->allocated_node_id.elements[0].value <= CANARD_NODE_ID_MAX) {
        printf("Got PnP node-ID allocation: %d\n", msg->allocated_node_id.elements[0].value);
        clCanard.set_canard_node_id((CanardNodeID)msg->allocated_node_id.elements[0].value);
        // Store the value into the non-volatile storage.
        static uavcan_register_Value_1_0 reg = {0};
        uavcan_register_Value_1_0_select_natural16_(&reg);
        reg.natural16.value.elements[0] = msg->allocated_node_id.elements[0].value;
        reg.natural16.value.count = 1;
        localRegisterAccessLock->Take();
        localRegister->write(REGISTER_UAVCAN_NODE_ID, &reg);
        localRegisterAccessLock->Give();
        // We no longer need the subscriber, drop it to free up the resources (both memory and CPU time).
        clCanard.rxUnSubscribe(CanardTransferKindMessage,
                                uavcan_pnp_NodeIDAllocationData_1_0_FIXED_PORT_ID_);
    }
    // Otherwise, ignore it: either it is a request from another node or it is a response to another node.
}

/// @brief Chiamate gestioni RPC remote da master (yakut o altro servizio di controllo)
/// @param clCanard Classe
/// @param req richiesta
/// @param remote_node nodo remoto
/// @return execute_command standard UAVCAN
uavcan_node_ExecuteCommand_Response_1_1 CanTask::processRequestExecuteCommand(canardClass &clCanard, const uavcan_node_ExecuteCommand_Request_1_1* req,
                                                                            uint8_t remote_node) {
    uavcan_node_ExecuteCommand_Response_1_1 resp = {0};
    // System Queue request Structure data
    // Command-> Accelerometer Calibrate...
    // Send Command directly To Task (Init to Null)
    system_message_t system_message = {0};

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
        case canardClass::Command_Private::calibrate_accelerometer:
        {
            // Avvia calibrazione accelerometro (reset bolla elettroniuca)
            TRACE_INFO_F(F("AVVIA Calibrazione accelerometro e salvataggio parametri"));
            // Send queue command to TASK
            system_message.task_dest = ACCELEROMETER_TASK_ID;
            system_message.command.do_init = true;
            if(localSystemMessageQueue->Enqueue(&system_message, Ticks::MsToTicks(WAIT_QUEUE_REQUEST_COMMAND_MS))) {
                resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            } else {                
                resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_FAILURE;
            }
            break;
        }
        case canardClass::Command_Private::module_maintenance:
        {
            // Avvia/Arresta modalità di manutenzione come comando sul system status
            if(req->parameter.elements[0]) {
                TRACE_INFO_F(F("AVVIA modalità di manutenzione modulo"));
            } else {
                TRACE_INFO_F(F("ARRESTA modalità di manutenzione modulo"));
            }
            // Send queue command to TASK
            system_message.task_dest = SUPERVISOR_TASK_ID;
            system_message.command.do_maint = 1;
            system_message.param = req->parameter.elements[0];
            if(localSystemMessageQueue->Enqueue(&system_message, Ticks::MsToTicks(WAIT_QUEUE_REQUEST_COMMAND_MS))) {
                resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            } else {                
                resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_FAILURE;
            }
            break;
        }
        case canardClass::Command_Private::enable_publish_rmap:
        {
            // Abilita pubblicazione fast_loop data_and_metadata modulo locale (test yakut e user master)
            clCanard.publisher_enabled.module_wind = true;
            TRACE_INFO_F(F("ATTIVO Trasmissione dati in publish"));
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        case canardClass::Command_Private::disable_publish_rmap:
        {
            // Disabilita pubblicazione fast_loop data_and_metadata modulo locale (test yakut e user master)
            clCanard.publisher_enabled.module_wind = false;
            TRACE_INFO_F(F("DISATTIVO Trasmissione dati in publish"));
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

/// @brief Chiamate gestioni dati remote da master (yakut o altro servizio di controllo)
/// @param clCanard Classe
/// @param req richiesta
/// @param param parametri CAN
/// @return dati rmap_servizio di modulo
rmap_service_module_Wind_Response_1_0 CanTask::processRequestGetModuleData(canardClass &clCanard, rmap_service_module_Wind_Request_1_0* req, CanParam_t *param) {
    rmap_service_module_Wind_Response_1_0 resp = {0};
    // Richiesta parametri univoca a tutti i moduli
    // req->parametri tipo: rmap_service_setmode_1_0
    // req->parameter.command (Comando esterno ricevuto 3 BIT)
    // req->parametri.run_sectime (Timer to run 13 bit)

    request_data_t request_data = {0};
    report_t report = {0};

    // Case comandi RMAP su GetModule Data (Da definire con esattezza quali e quanti altri)
    switch (req->parameter.command) {

        /// saturated uint3 get_istant = 0
        /// Ritorna il dato come richiesto dal master
        case rmap_service_setmode_1_0_get_istant:   // saturated uint3 get_istant = 0
        case rmap_service_setmode_1_0_get_current:  // saturated uint3 get_current = 1
        case rmap_service_setmode_1_0_get_last:     // saturated uint3 get_last = 2

          // preparo la struttura dati per richiedere i dati al task che li elabora
          if(req->parameter.command == rmap_service_setmode_1_0_get_istant) {
            // Solo Sample istantaneo            
            request_data.is_init = false;           // No Init, Sample
            request_data.report_time_s = 0;         // richiedo i dati su 0 secondi (Sample)
            request_data.observation_time_s = 0;    // richiedo i dati mediati su 0 secondi (Sample)
          } 
          if((req->parameter.command == rmap_service_setmode_1_0_get_current)||
             (req->parameter.command == rmap_service_setmode_1_0_get_last)) {
            // Dato corrente con o senza inizializzazione (get_last...)
            if(req->parameter.command == rmap_service_setmode_1_0_get_current)
                request_data.is_init = false;   // utilizza i dati esistenti (continua le elaborazioni precedentemente inizializzate)
            else
                request_data.is_init = true;    // Reinit delle elaborazioni
            request_data.report_time_s = req->parameter.run_sectime;        // richiedo i dati su request secondi
            request_data.observation_time_s = req->parameter.obs_sectime;   // richiedo i dati mediati su request secondi
            last_req_rpt_time = req->parameter.run_sectime; // report_time_request_backup;
            last_req_obs_time = req->parameter.obs_sectime; // observation_time_request_backup;
            // SET Dynamic metadata (Request data from master Only Data != Sample)
            clCanard.module_wind.DWA.metadata.timerange.P2 = 0;
            clCanard.module_wind.DWB.metadata.timerange.P2 = request_data.report_time_s;
            clCanard.module_wind.DWC.metadata.timerange.P2 = request_data.report_time_s;
            clCanard.module_wind.DWD.metadata.timerange.P2 = request_data.report_time_s;
            clCanard.module_wind.DWE.metadata.timerange.P2 = request_data.report_time_s;
            clCanard.module_wind.DWF.metadata.timerange.P2 = request_data.report_time_s;
          }

          // coda di richiesta dati
          param->requestDataQueue->Enqueue(&request_data, Ticks::MsToTicks(WAIT_QUEUE_REQUEST_ELABDATA_MS));

          // coda di attesa dati (attesa rmap_calc_data)
          if (param->reportDataQueue->Dequeue(&report, Ticks::MsToTicks(WAIT_QUEUE_RESPONSE_ELABDATA_MS))) {
            // TRACE_INFO_F(F("--> CAN wind report\t%d\t%d\t%d\r\n"), (int32_t) report.wind.sample, (int32_t) report.wind.ist, (int32_t) report.wind.quality);
          }

          // Ritorno lo stato (Copia dal comando... e versione modulo)
          resp.state = req->parameter.command;
          resp.version = MODULE_MAIN_VERSION;
          resp.revision = MODULE_MINOR_VERSION;
          // Preparo la risposta con i dati recuperati dalla coda (come da request CAN)
          // TODO:_TH_RAIN
          if(req->parameter.command == rmap_service_setmode_1_0_get_istant) {
            // Solo Istantaneo (Sample display request)
            prepareSensorsDataValue(canardClass::Sensor_Type::dwa, &report, &resp);
          } else {
            prepareSensorsDataValue(canardClass::Sensor_Type::dwa, &report, &resp);
            prepareSensorsDataValue(canardClass::Sensor_Type::dwb, &report, &resp);
            prepareSensorsDataValue(canardClass::Sensor_Type::dwc, &report, &resp);
            prepareSensorsDataValue(canardClass::Sensor_Type::dwd, &report, &resp);
            prepareSensorsDataValue(canardClass::Sensor_Type::dwe, &report, &resp);
            prepareSensorsDataValue(canardClass::Sensor_Type::dwf, &report, &resp);
          }
          break;

        /// NOT USED
        /// saturated uint3 stop_acq = 4
        /// #define rmap_service_setmode_1_0_stop_acq (4U)
        /// saturated uint3 loop_acq = 5
        /// #define rmap_service_setmode_1_0_loop_acq (5U)
        /// saturated uint3 continuos_acq = 6
        /// #define rmap_service_setmode_1_0_continuos_acq (6U)

        /// saturated uint3 test_acq = 7 (Solo x TEST)
        case rmap_service_setmode_1_0_test_acq:
            resp.state = req->parameter.command;
            break;

        /// NON Gestito, risposta error (undefined)
        default:
            resp.state = GENERIC_STATE_UNDEFINED;
            break;
    }

    // Copio i metadati fissi e mobili
    resp.DWA.metadata = clCanard.module_wind.DWA.metadata;
    resp.DWB.metadata = clCanard.module_wind.DWB.metadata;
    resp.DWC.metadata = clCanard.module_wind.DWC.metadata;
    resp.DWD.metadata = clCanard.module_wind.DWD.metadata;
    resp.DWE.metadata = clCanard.module_wind.DWE.metadata;
    resp.DWF.metadata = clCanard.module_wind.DWF.metadata;

    return resp;
}

/// @brief Accesso ai registri UAVCAN risposta a richieste
/// @param req Richiesta
/// @return register access UAVCAN
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

/// @brief Canard CallBACK Receive message to Node (Solo con sottoscrizioni)
/// @param clCanard Classe
/// @param transfer Transfer CanardID
/// @param param Indirizzo di parametri esterni opzionali
void CanTask::processReceivedTransfer(canardClass &clCanard, const CanardRxTransfer* const transfer, void *param) {

    // System Queue request Structure data
    // Command-> Accelerometer Calibrate...
    // Send Command directly To Task (Init to Null)
    system_message_t system_message = {0};

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
                if(transfer->metadata.remote_node_id == clCanard.get_canard_master_id()) {
                    // Entro in OnLine se precedentemente arrivo dall'OffLine
                    // ed eseguo eventuali operazioni di entrata in attività se necessario
                    // Opzionale Controllo ONLINE direttamente dal messaggio Interno
                    // if (!clCanard.master.heartbeat.is_online()) {
                        // Il master è entrato in modalità ONLine e gestisco
                        // TRACE_VERBOSE(F("Master controller ONLINE !!! Starting application..."));
                    // }
                    // Set PowerMode da comando HeartBeat Remoto remoteVSC.powerMode
                    // Eventuali alri flag gestiti direttamente quà e SET sulla classe
                    canardClass::heartbeat_VSC remoteVSC;
                    // remoteVSC.powerMode
                    remoteVSC.uint8_val = msg.vendor_specific_status_code;
                    TRACE_VERBOSE_F(F("RX HeartBeat from master, master power mode SET: -> %u\r\n"), remoteVSC.powerMode);

                    // Processo e registro il nodo: stato, OnLine e relativi flag
                    // Set canard_us local per controllo NodoOffline (validità area di OnLine)
                    clCanard.master.heartbeat.set_online(MASTER_OFFLINE_TIMEOUT_US);
                    // SET Modalità Power richiesta dal Master (in risposta ad HeartBeat locale...)
                    // SErve come conferma al master e come flag locale di azione Power
                    clCanard.flag.set_local_power_mode(remoteVSC.powerMode);
                    
                    // PowerOn, Non faccio nulla o eventuale altra gestione (Tutto ON)
                    // if(remoteVSC.powerMode == canardClass::Power_Mode::pwr_on) {
                        // Ipotesi All Power Sensor ON...
                    // }

                    // Gestisco lo stato Power come richiesto dal Master immediatamente se != power_on
                    if(remoteVSC.powerMode == Power_Mode::pwr_nominal) {
                        // ENTER STANDARD SLEEP FROM CAN COMMAND
                        #ifndef _EXIT_SLEEP_FOR_DEBUGGING
                        system_message.task_dest = ALL_TASK_ID;
                        system_message.command.do_sleep = true;
                        localSystemMessageQueue->Enqueue(&system_message, Ticks::MsToTicks(WAIT_QUEUE_REQUEST_COMMAND_MS));
                        #endif
                    }

                    // if(remoteVSC.powerMode == canardClass::Power_Mode::pwr_deep_save) {
                        // Ipotesi CAN ON solo al passaggio del minuto 57..60 per syncroTime, Cmd e DataSend
                    // }

                    // if(remoteVSC.powerMode == canardClass::Power_Mode::pwr_critical) {
                        // Ipotesi DeepSleep. Save Param in Flash e PowerDown completo 60 Minuti...
                    // }
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
                if(transfer->metadata.remote_node_id == clCanard.get_canard_master_id()) {
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
                        TRACE_VERBOSE_F(F("RX TimeSyncro from master, reset or invalid Value at local_time_stamp (uSec): %u\r\n"), transfer->timestamp_usec);
                    }
                    // Aggiorna i temporizzatori locali per il prossimo controllo
                    CanardMicrosecond last_message_diff_us = clCanard.master.timestamp.update_timestamp_message(
                        transfer->timestamp_usec, msg.previous_transmission_timestamp_microsecond);
                    // Stampa e/o SETUP RTC con sincronizzazione valida
                    if (isSyncronized) {
                        #if (TRACE_LEVEL >= TRACE_LEVEL_VERBOSE)
                        uint32_t low_ms = timestamp_synchronized_us % 1000u;
                        uint32_t high_ms = timestamp_synchronized_us / 1000u;
                        TRACE_VERBOSE_F(F("RX TimeSyncro from master, syncronized value is (uSec): "));
                        TRACE_VERBOSE_F(F("%lu"), high_ms);
                        TRACE_VERBOSE_F(F("%lu\r\n"), low_ms);
                        TRACE_VERBOSE_F(F(" from 2 message difference is (uSec): %lu\r\n"), (uint32_t)last_message_diff_us);
                        #endif
                        // *********** Adjust Local RTC Time *************
                        uint32_t currentSubsecond;
                        volatile uint64_t canardEpoch_ms;
                        // Get Millisecond from TimeStampSyncronized
                        uint64_t timeStampEpoch_ms = timestamp_synchronized_us / 1000ul;
                        // Second Epoch in Millisecond With Add Subsecond
                        canardEpoch_ms = (uint64_t) rtc.getEpoch(&currentSubsecond);
                        canardEpoch_ms *= 1000ul;
                        canardEpoch_ms += currentSubsecond;
                        // Refer to milliSecond for TimeStamp ( Set Epoch with abs (ull) difference > 250 mSec )
                        if(canardEpoch_ms > timeStampEpoch_ms) {
                            currentSubsecond = canardEpoch_ms - timeStampEpoch_ms;
                        } else {
                            currentSubsecond = timeStampEpoch_ms - canardEpoch_ms;
                        }
                        // currentSubsecond -> millisDifference from RTC and TimeStamp_Syncro_ms
                        if(currentSubsecond > 250) {
                            // Set RTC Epoch DateTime with timeStampEpoch (round mSec + 1)
                            TRACE_VERBOSE_F(F("RTC: Adjust time with TimeSyncro from master (difference %lu mS)\r\n"), currentSubsecond);
                            timeStampEpoch_ms++;
                            rtc.setEpoch(timeStampEpoch_ms / 1000, timeStampEpoch_ms % 1000);
                        }
                    }
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
        if (transfer->metadata.port_id == clCanard.port_id.service_module_wind) {
            // Richiesta ai dati e metodi di sensor drive
            rmap_service_module_Wind_Request_1_0 req = {0};
            size_t size = transfer->payload_size;
            TRACE_INFO_F(F("<<-- Ricevuto richiesta dati da master\r\n"));
            // The request object is empty so we don't bother deserializing it. Just send the response.
            if (rmap_service_module_Wind_Request_1_0_deserialize_(&req, static_cast<uint8_t const*>(transfer->payload), &size) >= 0) {
                // I dati e metadati sono direttamente popolati in processRequestGetModuleData
                rmap_service_module_Wind_Response_1_0 module_wind_resp = processRequestGetModuleData(clCanard, &req, (CanParam_t *) param);
                // Serialize and publish the message:
                uint8_t serialized[rmap_service_module_Wind_Response_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
                size_t serialized_size = sizeof(serialized);
                const int8_t res = rmap_service_module_Wind_Response_1_0_serialize_(&module_wind_resp, &serialized[0], &serialized_size);
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
            TRACE_INFO_F(F("<<-- Ricevuto richiesta accesso ai registri\r\n"));
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
            TRACE_INFO_F(F("<<-- Ricevuto richiesta lettura elenco registri\r\n"));
            if (uavcan_register_List_Request_1_0_deserialize_(&req, static_cast<uint8_t const*>(transfer->payload), &size) >= 0) {
                const uavcan_register_List_Response_1_0 resp = {.name = localRegister->getNameByIndex(req.index)};
                uint8_t serialized[uavcan_register_List_Response_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
                size_t serialized_size = sizeof(serialized);
                if (uavcan_register_List_Response_1_0_serialize_(&resp, &serialized[0], &serialized_size) >= 0) {
                    // Risposta standard ad un secondo dal timeStamp Sincronizzato
                    clCanard.sendResponse(CANARD_REGISTERLIST_TRANSFER_ID_TIMEOUT_USEC, &transfer->metadata, serialized_size, &serialized[0]);
                }
            }
        }
        else if (transfer->metadata.port_id == uavcan_node_ExecuteCommand_1_1_FIXED_PORT_ID_)
        {
            uavcan_node_ExecuteCommand_Request_1_1 req = {0};
            size_t size = transfer->payload_size;
            TRACE_INFO_F(F("<<-- Ricevuto comando esterno\r\n"));
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
                        TRACE_ERROR_F(F("SAVING BLOCK FILE ERROR, ABORT RX !!!"));
                        clCanard.master.file.download_end();
                    }
                }
            } else {
                // Errore Nodo non settato...
                TRACE_ERROR_F(F("RX FILE READ BLOCK REJECT: Node_Id not valid or not set\r\n"));
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
  localQspiLock = param.qspiLock;
  localRegisterAccessLock = param.registerAccessLock;

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
    static uavcan_register_Value_1_0 val = {0};

    // System message data queue structured
    system_message_t system_message;

    // LoopTimer Publish
    CanardMicrosecond last_pub_rmap_data;
    CanardMicrosecond last_pub_heartbeat;
    CanardMicrosecond last_pub_port_list;

    // Set when Firmware Upgrade is required
    bool start_firmware_upgrade = false;

    // OnlineMaster (Set/Reset Application Code Function Here Enter/Exit Function OnLine)
    bool masterOnline = false;

    // Start Running Monitor and First WDT normal state
    #if (ENABLE_STACK_USAGE)
    TaskMonitorStack();
    #endif
    TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

    // Main Loop TASK
    while (true) {

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
                clCanard.setReceiveMessage_CB(processReceivedTransfer, (void *) &param);

                // ********************    Lettura Registri standard UAVCAN    ********************
                // Restore the master-ID from the corresponding standard register -> Default to anonymous.
                #ifdef USE_NODE_MASTER_ID_FIXED
                // Canard Slave NODE ID Fixed dal defined value in module_config
                clCanard.set_canard_master_id((CanardNodeID)NODE_MASTER_ID);
                #else
                uavcan_register_Value_1_0_select_natural16_(&val);
                val.natural16.value.count = 1;
                val.natural16.value.elements[0] = UINT16_MAX; // This means undefined (anonymous), per Specification/libcanard.
                localRegisterAccessLock->Take();
                localRegister->read(REGISTER_RMAP_MASTER_ID, &val);      // The names of the standard registers are regulated by the Specification.
                localRegisterAccessLock->Give();
                LOCAL_ASSERT(uavcan_register_Value_1_0_is_natural16_(&val) && (val.natural16.value.count == 1));
                if (val.natural16.value.elements[0] <= CANARD_NODE_ID_MAX) {
                    clCanard.set_canard_master_id((CanardNodeID)val.natural16.value.elements[0]);
                }
                #endif

                // ********************    Lettura Registri standard UAVCAN    ********************
                // Restore the node-ID from the corresponding standard regioster. Default to anonymous.
                #ifdef USE_NODE_SLAVE_ID_FIXED
                // Canard Slave NODE ID Fixed dal defined value in module_config
                clCanard.set_canard_node_id((CanardNodeID)NODE_SLAVE_ID);
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
                }
                #endif

                // The description register is optional but recommended because it helps constructing/maintaining large networks.
                // It simply keeps a human-readable description of the node that should be empty by default.
                uavcan_register_Value_1_0_select_string_(&val);
                val._string.value.count = 0;
                localRegisterAccessLock->Take();
                localRegister->read(REGISTER_UAVCAN_NODE_DESCR, &val);  // We don't need the value, we just need to ensure it exists.
                localRegisterAccessLock->Give();

                // Carico i/il port-ID/subject-ID del modulo locale dai registri relativi associati nel namespace UAVCAN
                #ifdef USE_PORT_PUBLISH_RMAP_FIXED
                clCanard.port_id.publisher_module_wind = (CanardPortID)SUBJECTID_PUBLISH_RMAP;
                #else
                clCanard.port_id.publisher_module_wind =
                    getModeAccessID(canardClass::Introspection_Port::PublisherSubjectID,
                        REGISTER_DATA_PUBLISH, rmap_module_Wind_1_0_FULL_NAME_AND_VERSION_);
                #endif
                #ifdef USE_PORT_SERVICE_RMAP_FIXED
                clCanard.port_id.service_module_wind = (CanardPortID)PORT_SERVICE_RMAP;
                #else
                clCanard.port_id.service_module_wind =
                    getModeAccessID(canardClass::Introspection_Port::ServicePortID,
                        REGISTER_DATA_SERVICE, rmap_service_module_Wind_1_0_FULL_NAME_AND_VERSION_);
                #endif

                // ************************* LETTURA REGISTRI METADATI RMAP ****************************
                // POSITION ARRAY METADATA CONFIG: (TOT ELEMENTS = SENSOR_METADATA_COUNT)
                // SENSOR_METADATA_DWA                   (0)
                // SENSOR_METADATA_DWB                   (1)
                // SENSOR_METADATA_DWC                   (2)
                // SENSOR_METADATA_DWD                   (3)
                // SENSOR_METADATA_DWE                   (4)
                // SENSOR_METADATA_DWF                   (5)
                // *********************************** L1 *********************************************
                uavcan_register_Value_1_0_select_natural16_(&val);
                val.natural16.value.count = SENSOR_METADATA_COUNT;
                for(uint8_t id=0; id<SENSOR_METADATA_COUNT; id++) {
                    val.natural16.value.elements[id] = SENSOR_METADATA_LEVEL_1;
                }
                localRegisterAccessLock->Take();
                localRegister->read(REGISTER_METADATA_LEVEL_L1, &val);
                localRegisterAccessLock->Give();
                LOCAL_ASSERT(uavcan_register_Value_1_0_is_natural16_(&val) && (val.natural16.value.count == SENSOR_METADATA_COUNT));
                clCanard.module_wind.DWA.metadata.level.L1.value = val.natural16.value.elements[SENSOR_METADATA_DWA];
                clCanard.module_wind.DWB.metadata.level.L1.value = val.natural16.value.elements[SENSOR_METADATA_DWB];
                clCanard.module_wind.DWC.metadata.level.L1.value = val.natural16.value.elements[SENSOR_METADATA_DWC];
                clCanard.module_wind.DWD.metadata.level.L1.value = val.natural16.value.elements[SENSOR_METADATA_DWD];
                clCanard.module_wind.DWE.metadata.level.L1.value = val.natural16.value.elements[SENSOR_METADATA_DWE];
                clCanard.module_wind.DWF.metadata.level.L1.value = val.natural16.value.elements[SENSOR_METADATA_DWF];
                // *********************************** L2 *********************************************
                uavcan_register_Value_1_0_select_natural16_(&val);
                val.natural16.value.count = SENSOR_METADATA_COUNT;
                for(uint8_t id=0; id<SENSOR_METADATA_COUNT; id++) {
                    val.natural16.value.elements[id] = SENSOR_METADATA_LEVEL_2;
                }
                localRegisterAccessLock->Take();
                localRegister->read(REGISTER_METADATA_LEVEL_L2, &val);
                localRegisterAccessLock->Give();
                LOCAL_ASSERT(uavcan_register_Value_1_0_is_natural16_(&val) && (val.natural16.value.count == SENSOR_METADATA_COUNT));
                clCanard.module_wind.DWA.metadata.level.L2.value = val.natural16.value.elements[SENSOR_METADATA_DWA];
                clCanard.module_wind.DWB.metadata.level.L2.value = val.natural16.value.elements[SENSOR_METADATA_DWB];
                clCanard.module_wind.DWC.metadata.level.L2.value = val.natural16.value.elements[SENSOR_METADATA_DWC];
                clCanard.module_wind.DWD.metadata.level.L2.value = val.natural16.value.elements[SENSOR_METADATA_DWD];
                clCanard.module_wind.DWE.metadata.level.L2.value = val.natural16.value.elements[SENSOR_METADATA_DWE];
                clCanard.module_wind.DWF.metadata.level.L2.value = val.natural16.value.elements[SENSOR_METADATA_DWF];
                // ******************************* LevelType1 *****************************************
                uavcan_register_Value_1_0_select_natural16_(&val);
                val.natural16.value.count = SENSOR_METADATA_COUNT;
                for(uint8_t id=0; id<SENSOR_METADATA_COUNT; id++) {
                    val.natural16.value.elements[id] = SENSOR_METADATA_LEVELTYPE_1;
                }
                localRegisterAccessLock->Take();
                localRegister->read(REGISTER_METADATA_LEVEL_TYPE1, &val);
                localRegisterAccessLock->Give();
                LOCAL_ASSERT(uavcan_register_Value_1_0_is_natural16_(&val) && (val.natural16.value.count == SENSOR_METADATA_COUNT));
                clCanard.module_wind.DWA.metadata.level.LevelType1.value = val.natural16.value.elements[SENSOR_METADATA_DWA];
                clCanard.module_wind.DWB.metadata.level.LevelType1.value = val.natural16.value.elements[SENSOR_METADATA_DWB];
                clCanard.module_wind.DWC.metadata.level.LevelType1.value = val.natural16.value.elements[SENSOR_METADATA_DWC];
                clCanard.module_wind.DWD.metadata.level.LevelType1.value = val.natural16.value.elements[SENSOR_METADATA_DWD];
                clCanard.module_wind.DWE.metadata.level.LevelType1.value = val.natural16.value.elements[SENSOR_METADATA_DWE];
                clCanard.module_wind.DWF.metadata.level.LevelType1.value = val.natural16.value.elements[SENSOR_METADATA_DWF];
                // ******************************* LevelType2 *****************************************
                uavcan_register_Value_1_0_select_natural16_(&val);
                val.natural16.value.count = SENSOR_METADATA_COUNT;
                for(uint8_t id=0; id<SENSOR_METADATA_COUNT; id++) {
                    val.natural16.value.elements[id] = SENSOR_METADATA_LEVELTYPE_2;
                }
                localRegisterAccessLock->Take();
                localRegister->read(REGISTER_METADATA_LEVEL_TYPE2, &val);
                localRegisterAccessLock->Give();
                LOCAL_ASSERT(uavcan_register_Value_1_0_is_natural16_(&val) && (val.natural16.value.count == SENSOR_METADATA_COUNT));
                clCanard.module_wind.DWA.metadata.level.LevelType2.value = val.natural16.value.elements[SENSOR_METADATA_DWA];
                clCanard.module_wind.DWB.metadata.level.LevelType2.value = val.natural16.value.elements[SENSOR_METADATA_DWB];
                clCanard.module_wind.DWC.metadata.level.LevelType2.value = val.natural16.value.elements[SENSOR_METADATA_DWC];
                clCanard.module_wind.DWD.metadata.level.LevelType2.value = val.natural16.value.elements[SENSOR_METADATA_DWD];
                clCanard.module_wind.DWE.metadata.level.LevelType2.value = val.natural16.value.elements[SENSOR_METADATA_DWE];
                clCanard.module_wind.DWF.metadata.level.LevelType2.value = val.natural16.value.elements[SENSOR_METADATA_DWF];
                // *********************************** P1 *********************************************
                uavcan_register_Value_1_0_select_natural16_(&val);
                val.natural16.value.count = SENSOR_METADATA_COUNT;
                for(uint8_t id=0; id<SENSOR_METADATA_COUNT; id++) {
                    val.natural16.value.elements[id] = SENSOR_METADATA_LEVEL_P1;
                }
                localRegisterAccessLock->Take();
                localRegister->read(REGISTER_METADATA_TIME_P1, &val);
                localRegisterAccessLock->Give();
                LOCAL_ASSERT(uavcan_register_Value_1_0_is_natural16_(&val) && (val.natural16.value.count == SENSOR_METADATA_COUNT));
                clCanard.module_wind.DWA.metadata.timerange.P1.value = val.natural16.value.elements[SENSOR_METADATA_DWA];
                clCanard.module_wind.DWB.metadata.timerange.P1.value = val.natural16.value.elements[SENSOR_METADATA_DWB];
                clCanard.module_wind.DWC.metadata.timerange.P1.value = val.natural16.value.elements[SENSOR_METADATA_DWC];
                clCanard.module_wind.DWD.metadata.timerange.P1.value = val.natural16.value.elements[SENSOR_METADATA_DWD];
                clCanard.module_wind.DWE.metadata.timerange.P1.value = val.natural16.value.elements[SENSOR_METADATA_DWE];
                clCanard.module_wind.DWF.metadata.timerange.P1.value = val.natural16.value.elements[SENSOR_METADATA_DWF];
                // *********************************** P2 *********************************************
                // P2 Non memorizzato sul modulo, parametro dipendente dall'acquisizione locale
                clCanard.module_wind.DWA.metadata.timerange.P2 = 0;
                clCanard.module_wind.DWB.metadata.timerange.P2 = 0;
                clCanard.module_wind.DWC.metadata.timerange.P2 = 0;
                clCanard.module_wind.DWD.metadata.timerange.P2 = 0;
                clCanard.module_wind.DWE.metadata.timerange.P2 = 0;
                clCanard.module_wind.DWF.metadata.timerange.P2 = 0;
                // *********************************** P2 *********************************************
                uavcan_register_Value_1_0_select_natural8_(&val);
                val.natural8.value.count = SENSOR_METADATA_COUNT;
                // Default are single different value for type sensor
                val.natural8.value.elements[SENSOR_METADATA_DWA] = SENSOR_METADATA_LEVEL_P_IND_DWA;
                val.natural8.value.elements[SENSOR_METADATA_DWB] = SENSOR_METADATA_LEVEL_P_IND_DWB;
                val.natural8.value.elements[SENSOR_METADATA_DWC] = SENSOR_METADATA_LEVEL_P_IND_DWC;
                val.natural8.value.elements[SENSOR_METADATA_DWD] = SENSOR_METADATA_LEVEL_P_IND_DWD;
                val.natural8.value.elements[SENSOR_METADATA_DWE] = SENSOR_METADATA_LEVEL_P_IND_DWE;
                val.natural8.value.elements[SENSOR_METADATA_DWF] = SENSOR_METADATA_LEVEL_P_IND_DWF;
                localRegisterAccessLock->Take();
                localRegister->read(REGISTER_METADATA_TIME_PIND, &val);
                localRegisterAccessLock->Give();
                LOCAL_ASSERT(uavcan_register_Value_1_0_is_natural8_(&val) && (val.natural8.value.count == SENSOR_METADATA_COUNT));
                clCanard.module_wind.DWA.metadata.timerange.Pindicator.value = val.natural16.value.elements[SENSOR_METADATA_DWA];
                clCanard.module_wind.DWB.metadata.timerange.Pindicator.value = val.natural16.value.elements[SENSOR_METADATA_DWB];
                clCanard.module_wind.DWC.metadata.timerange.Pindicator.value = val.natural16.value.elements[SENSOR_METADATA_DWC];
                clCanard.module_wind.DWD.metadata.timerange.Pindicator.value = val.natural16.value.elements[SENSOR_METADATA_DWD];
                clCanard.module_wind.DWE.metadata.timerange.Pindicator.value = val.natural16.value.elements[SENSOR_METADATA_DWE];
                clCanard.module_wind.DWF.metadata.timerange.Pindicator.value = val.natural16.value.elements[SENSOR_METADATA_DWF];

                // Passa alle sottoscrizioni
                state = CAN_STATE_SETUP;
                break;

            // ********************************************************************************
            //               AVVIA SOTTOSCRIZIONI ai messaggi per servizi RPC ecc...
            // ********************************************************************************
            case CAN_STATE_SETUP:

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
                                        CANARD_REGISTERLIST_TRANSFER_ID_TIMEOUT_USEC)) {
                    LOCAL_ASSERT(false);
                }

                // Service servers: -> Risposta per dati e metadati sensore modulo corrente da master (Yakut, Altri)
                if (!clCanard.rxSubscribe(CanardTransferKindRequest,
                                        clCanard.port_id.service_module_wind,
                                        rmap_service_module_Wind_Request_1_0_EXTENT_BYTES_,
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

                // Avvio il modulo UAVCAN in modalità operazionale normale
                // Eventuale SET Flag dopo acquisizione di configurazioni e/o parametri da Remoto
                clCanard.flag.set_local_node_mode(uavcan_node_Mode_1_0_OPERATIONAL);

                // Set START Timetable LOOP RX/TX. Set Canard microsecond start, per le sincronizzazioni
                clCanard.getMicros(clCanard.start_syncronization);
                last_pub_rmap_data = clCanard.getMicros(clCanard.syncronized_time) + MEGA * (TIME_PUBLISH_MODULE_DATA + random(50) / 100);
                last_pub_heartbeat = clCanard.getMicros(clCanard.syncronized_time) + MEGA * (TIME_PUBLISH_HEARTBEAT + random(100) / 100);
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

                // TEST VERIFICA sincronizzazione time_stamp locale con remoto... (LED/OUT sincronizzati)
                // Test con reboot e successiva risincronizzazione automatica (FAKE RTC!!!!)
                #if defined(LED_ON_SYNCRO_TIME) && defined(LED2_PIN)
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
                        TRACE_INFO_F(F("Master controller ONLINE !!! -> OnLineFunction()\r\n"));
                        // .... Codice OnLineFunction()
                    }
                    masterOnline = true;
                    // **************************************************************************
                    //            STATO MODULO (Decisionale in funzione di stato remoto)
                    // Gestione continuativa del modulo di acquisizione master.clCanard (flag remoto)
                    // **************************************************************************
                    // Eventuale codice di gestione MasterOnline OK...
                } else {
                    // Solo quando passo da OffLine ad OnLine
                    if (masterOnline) {
                        TRACE_INFO_F(F("Master controller OFFLINE !!! ALERT -> OffLineFunction()\r\n"));
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
                            }                            // Nessun altro evento necessario, chiudo File e stati
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

                // *************************** RMAP DATA PUBLISHER **************************
                // Pubblica i dati del nodo corrente se configurata e abilitata la funzione
                // Ovviamente il nodo non può essere anonimo per la pubblicazione...
                if ((!clCanard.is_canard_node_anonymous()) &&
                    (clCanard.publisher_enabled.module_wind) &&
                    (clCanard.port_id.publisher_module_wind <= CANARD_SUBJECT_ID_MAX)) {
                    if (clCanard.getMicros(clCanard.syncronized_time) >= last_pub_rmap_data)
                    {
                        // Update publisher
                        last_pub_rmap_data += MEGA * TIME_PUBLISH_MODULE_DATA;
                        // Funzione locale privata
                        publish_rmap_data(clCanard, &param);
                    }
                }

                // ************************* HEARTBEAT DATA PUBLISHER ***********************
                if((start_firmware_upgrade)||
                    (clCanard.getMicros(clCanard.syncronized_time) >= last_pub_heartbeat)) {
                    if(clCanard.is_canard_node_anonymous()) {
                        TRACE_INFO_F(F("Publish SLAVE PNP Request Message -->> [ %u sec + Rnd * 1 sec...]\r\n"), TIME_PUBLISH_PNP_REQUEST);
                        clCanard.slave_pnp_send_request(1);
                        last_pub_heartbeat += MEGA * (TIME_PUBLISH_PNP_REQUEST + random(100) / 100);
                    } else {
                        TRACE_INFO_F(F("Publish SLAVE Heartbeat -->> [ %u sec]\r\n"), TIME_PUBLISH_HEARTBEAT);
                        clCanard.slave_heartbeat_send_message();
                        // Update publisher
                        last_pub_heartbeat += MEGA * TIME_PUBLISH_HEARTBEAT;
                    }
                }

                // ********************** SERVICE PORT LIST PUBLISHER ***********************
                if (clCanard.getMicros(clCanard.syncronized_time) >= last_pub_port_list) {
                    TRACE_INFO_F(F("Publish Local PORT LIST -->> [ %u sec]\r\n"), TIME_PUBLISH_PORT_LIST);
                    last_pub_port_list += MEGA * TIME_PUBLISH_PORT_LIST;
                    // Update publisher
                    clCanard.slave_servicelist_send_message();
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
            DelayUntil(Ticks::MsToTicks(CAN_TASK_WAIT_MAXSPEED_DELAY_MS));
        }
        else
        {
            DelayUntil(Ticks::MsToTicks(CAN_TASK_WAIT_DELAY_MS));
        }

    }
}
