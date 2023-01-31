/**
  ******************************************************************************
  * @file    mmc_task.cpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   mmc_task source file (SDMMC StimaV4)
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

#define TRACE_LEVEL     MMC_TASK_TRACE_LEVEL
#define LOCAL_TASK_ID   MMC_TASK_ID

#include "tasks/mmc_task.h"

using namespace cpp_freertos;

MmcTask::MmcTask(const char *taskName, uint16_t stackSize, uint8_t priority, MmcParam_t mmcParam) : Thread(taskName, stackSize, priority), param(mmcParam)
{
  // Start WDT controller and TaskState Flags
  TaskWatchDog(WDT_STARTING_TASK_MS);
  TaskState(MMC_STATE_CREATE, UNUSED_SUB_POSITION, task_flag::normal);

  eeprom = EEprom(param.wire, param.wireLock);
  state = MMC_STATE_INIT;
  Start();
};

#if (ENABLE_STACK_USAGE)
/// @brief local stack Monitor (optional)
void MmcTask::TaskMonitorStack()
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
void MmcTask::TaskWatchDog(uint32_t millis_standby)
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
void MmcTask::TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation)
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

void MmcTask::Run()
{
  uint8_t retry;
  bool is_get_rtc;
  STM32RTC &rtc = STM32RTC::getInstance();
  system_request_t request;
  system_response_t response;
  char logMessage[LOG_PUT_DATA_ELEMENT_SIZE] = {0};
  char logIntest[23] = {0};
  // Firmware check and update
  char stima_name[STIMA_MODULE_NAME_LENGTH];
  char file_name[128];
  uint16_t idxList;
  uint8_t module_type, fw_version, fw_revision;
  bool fw_found;

  File dir;

  // Start Running Monitor and First WDT normal state
  #if (ENABLE_STACK_USAGE)
  TaskMonitorStack();
  #endif
  TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

/*

//  delay(100);
  Serial.print("Initializing SD card...");
  uint32_t millist;
  while (!SD.begin(PIN_MMC1_DTC))
  {
  }
  Serial.println("initialization done.");

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  if(SD.exists("test.txt")) {
    Serial.println("Remove test");
    SD.remove("test.txt");
    // Note make a delay after remove
    delay(1);
  }

  //delay(40);
bool done = false;

while(!done) {
  myFile = SD.open("test.txt", FILE_WRITE);
  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to test.txt...");
    for(uint8_t a=0; a < 200; a++){
      myFile.println("testing 1, 2, 3.");
    }
    myFile.close();
    Serial.println("done.");
    done = true;
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
    // Note Resync SD with new init
    delay(500);
    SD.begin();
  }
}

uint8_t dato[50] = {0};
uint32_t index=0;
  if(SD.exists("test.txt")) {
    Serial.println("EXISTTTTTTTTT !!!!!!!! test.txt");
    // re-open the file for reading:
    myFile = SD.open("test.txt");
    if (myFile) {
      Serial.println("test.txt:");
      // read from the file until there's nothing else in it:
      index = myFile.available();
      // close the file:
      myFile.close();
    }
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

  Serial.print("Tot Char read: ");
  Serial.println(index);
  if(index!=3600) {
    delay(10000);    
  }

  delay(10);
  NVIC_SystemReset();

*/

  while (true)
  {

    switch (state)
    {
    case MMC_STATE_INIT:
      // Check SD or Resynch after Error
      if (MMCCard_Present()) {
        TRACE_VERBOSE_F(F("MMC Card slot ready -> MMC_STATE_CHECK_SD\r\n"));
        state = MMC_STATE_CHECK_SD;
      } else {
        TRACE_VERBOSE_F(F("MMC Card slot empty, MMC function disabled\r\n"));
      }
      break;

    case MMC_STATE_CHECK_SD:
      // Setup SD Card
      if (SD.begin()) {
        TRACE_VERBOSE_F(F("MMC Card init complete -> MMC_STATE_WAITING_EVENT\r\n"));
        // Optional Trace Type of CARD... and Size
        // Check or create directory Structure...
        if(!SD.exists("firmware")) SD.mkdir("firmware");
        if(!SD.exists("log")) SD.mkdir("log");
        if(!SD.exists("data")) SD.mkdir("data");

        #if (TRACE_LEVEL > TRACE_LEVEL_OFF)
        // Check firmware file present Type, model and version
        idxList = 0;
        // Check list Firmware File (Added Function to SD Class)
        while(SD.listIndex("/firmware", idxList++, file_name)) {
          // Found firmware file?
          if(checkStimaFirmwareType(file_name, &module_type, &fw_version, &fw_revision)) {
            getStimaNameByType(stima_name, module_type);
            TRACE_INFO_F(F("MMC: found firmware type: %s Ver %u.%u\r\n"), stima_name, fw_version, fw_revision);
          }
        }
        #endif

        // Push firmware to Flash
        state = MMC_STATE_WAITING_EVENT;
      }
      break;

    case MMC_STATE_WAITING_EVENT:

      // If System SLEEP...  Long WAIT
      // Else check all queue input
      // Go to response/action

      // Test Firmware Upload local (Remote RPC / or local display request)
      // if request_update
      {
        Serial.println();
        Serial.print(xPortGetFreeHeapSize());
        Serial.println();
        // Check firmware file present Type, model and version
        fw_found = false;
        // Name of module
        getStimaNameByType(stima_name, param.configuration->module_type);
        // Check list Firmware File
        while(SD.listIndex("/firmware", idxList++, file_name)) {
          // Found firmware file?
          if(checkStimaFirmwareType(file_name, &module_type, &fw_version, &fw_revision)) {
            // Is this module ?
            if(module_type == param.configuration->module_type) {
              fw_found = true;
              if((fw_version > param.configuration->module_main_version) ||
                ((fw_version == param.configuration->module_main_version) && (fw_revision > param.configuration->module_minor_version)))
              {
                TRACE_INFO_F(F("MMC: found firmware upgradable type: %s Ver %u.%u\r\n"), stima_name, fw_version, fw_revision);
                TRACE_INFO_F(F("MMC: starting firmware upgrade...\n\r"));
                // TODO: Put file in Flash (same of UAVCAN push Flash)
                // Rename firmware in .old
                // NB Get VErsion and Revision Remote???
                // Non necessario con send 
                // and receive uavcan_node_ExecuteCommand_Response_1_1_STATUS_NOT_AUTHORIZED
                // rename file in .old e download solo dei file app.hex ma se trovi .old no!
                // creare lista di file version max
                // NVIC Reboot
              }
              else
              {
                TRACE_INFO_F(F("MMC: found firmware obsolete: %s Ver %u.%u\r\n"), stima_name, fw_version, fw_revision);
                TRACE_INFO_F(F("MMC: firmware upgrade abort!!!\n\r"));
              }
              break;
            }
          }
        }
        // Firmware not Found
        if(!fw_found) {
          TRACE_INFO_F(F("MMC: module firmware for module %s not found\r\n"), stima_name);
        }
        Serial.println();
        Serial.print(xPortGetFreeHeapSize());
        Serial.println();
      }

      // *********************************************************
      //             Perform LOG WRITE append message
      // *********************************************************
      // If element get all element from the queue and Put to MMC
      is_get_rtc = false;
      while(!param.dataLogPutQueue->IsEmpty()) {
        if(!is_get_rtc) {
          // Get date time to Intest string to PUT (for this message session)
          is_get_rtc = true;
          if(param.rtcLock->Take(Ticks::MsToTicks(RTC_WAIT_DELAY_MS))) {
            sprintf(logIntest, "%02d/%02d/%02d %02d:%02d:%02d.%03d ",
              rtc.getDay(), rtc.getMonth(), rtc.getYear(), rtc.getHours(), rtc.getMinutes(), rtc.getSeconds(), rtc.getSubSeconds());
            param.rtcLock->Give();
          }
        }
        // Get message from queue
        if(param.dataLogPutQueue->Dequeue(logMessage)) {
          // Put to MMC ( APPEND File )
          localFile = SD.open("log/log.txt", FA_OPEN_APPEND | FA_WRITE);
          if(localFile) {          
            localFile.print(logIntest);
            localFile.write(logMessage, strlen(logMessage) < LOG_PUT_DATA_ELEMENT_SIZE ? strlen(logMessage) : LOG_PUT_DATA_ELEMENT_SIZE);
            localFile.println();
            localFile.close();
          }
        }
      }
      // *********************************************************
      //             End OF perform LOG append message
      // *********************************************************

      break;

    // case MMC_REQUEST_BLOCK_DATA:
    // case MMC_RECEIVE_BLOCK_DATA:
    // case MMC_REQUEST_BLOCK_FIRMWARE:
    // case MMC_RECEIVE_BLOCK_FIRMWARE:
    // case MMC_UPLOAD_FIRMWARE_TO_FLASH:

    case MMC_STATE_ERROR:
      // Gest Error... Resynch SD
      TRACE_VERBOSE_F(F("MMC_STATE_ERROR -> MMC_STATE_INIT\r\n"));
      state = MMC_STATE_INIT;
      break;
    }

    #if (ENABLE_STACK_USAGE)
    TaskMonitorStack();
    #endif

    // One step base non blocking switch
    TaskWatchDog(MMC_TASK_WAIT_DELAY_MS);
    Delay(Ticks::MsToTicks(MMC_TASK_WAIT_DELAY_MS));

  }
}
