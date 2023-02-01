/**
  ******************************************************************************
  * @file    wdt_task.cpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   wdt_task source file (Wdt && Logging Task for Module Slave)
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

#define TRACE_LEVEL     WDT_TASK_TRACE_LEVEL
#define LOCAL_TASK_ID   WDT_TASK_ID

#include "tasks/wdt_task.h"

WdtTask::WdtTask(const char *taskName, uint16_t stackSize, uint8_t priority, WdtParam_t wdtParam) : Thread(taskName, stackSize, priority), param(wdtParam)
{
  Start();
};

void WdtTask::Run() {
  bool firsCheck = true;
  STM32RTC &rtc = STM32RTC::getInstance();
  u_int16_t stackUsage;
  char strTask[12] = {0};
  bootloader_t boot_check;
  char logMessage[LOG_PUT_DATA_ELEMENT_SIZE] = {0};

  // WDT Start to Normal...
  param.systemStatusLock->Take();
  param.system_status->tasks[WDT_TASK_ID].state = task_flag::normal;
  param.systemStatusLock->Give();

  TRACE_INFO_F(F("WDT: Starting WDT and Info Stack TASK..."));

  while (true) {

    // Check WDT Ready to reload (reset)
    bool resetWdt = true;
    TRACE_INFO_F(F("%s: "), Thread::GetName().c_str());
    // Trace DateTime with Semaphore
    if(param.rtcLock->Take(Ticks::MsToTicks(RTC_WAIT_DELAY_MS))) {
      TRACE_INFO_F(F("%02d/%02d/%02d "), rtc.getDay(), rtc.getMonth(), rtc.getYear());
      TRACE_INFO_F(F("%02d:%02d:%02d.%03d\r\n"), rtc.getHours(), rtc.getMinutes(), rtc.getSeconds(), rtc.getSubSeconds());
      param.rtcLock->Give();
    }

    param.systemStatusLock->Take();
    // WDT Always Set
    param.system_status->tasks[WDT_TASK_ID].watch_dog = wdt_flag::set;
    // Check All Module Setting Flag WDT before reset Global WDT
    for(uint8_t id = 0; id < TOTAL_INFO_TASK; id++)
    {
      // For all Running Task (not suspend) and (real started Running_POS != 0)
      // Sleep is noting Suspended. WatchDog (xxx) Longer Time is required !!!
      if((param.system_status->tasks[id].state != task_flag::suspended)&&
        (param.system_status->tasks[id].running_pos)) {
        // Checkink First WDT Timer correct
        if (param.system_status->tasks[id].watch_dog == wdt_flag::timer) {
          param.system_status->tasks[id].watch_dog_ms -= WDT_TASK_WAIT_DELAY_MS;
          if(param.system_status->tasks[id].watch_dog_ms < 0)
          {
            // Exit Time validity check WDT. Remove Flag and reset TimeUp
            param.system_status->tasks[id].watch_dog_ms = 0;
            param.system_status->tasks[id].watch_dog == wdt_flag::clear;
          }          
        }
        // If single task not performed local signal WDT...
        if (param.system_status->tasks[id].watch_dog == wdt_flag::clear)
          // Wdt is not applicable (All Working TASK Need to be operative!!!)
          resetWdt = false;
      }
    }
    param.systemStatusLock->Give();

    // Logging Stack (Only ready module task controlled)
    #if (ENABLE_STACK_USAGE)
    // Update This Task
    stackUsage = (u_int16_t)uxTaskGetStackHighWaterMark( NULL );
    if((stackUsage) && (stackUsage < param.system_status->tasks[WDT_TASK_ID].stack)) {
      param.systemStatusLock->Take();
      param.system_status->tasks[WDT_TASK_ID].stack = stackUsage;
      param.systemStatusLock->Give();
    }
    TRACE_INFO_F(F("WDT: Stack Free monitor, Heap free: %lu\r\n"), (uint32_t)xPortGetFreeHeapSize());
    for(uint8_t id = 0; id < TOTAL_INFO_TASK; id++) {
      if(param.system_status->tasks[id].stack != 0xFFFFu) {
        switch(id) {
          case SUPERVISOR_TASK_ID:
            strcpy (strTask, "Supervisor  "); break;
          case MMC_TASK_ID:
            strcpy (strTask, "MMC Card    "); break;
          case LCD_TASK_ID:
            strcpy (strTask, "Display LCD "); break;
          case CAN_TASK_ID:
            strcpy (strTask, "Can Bus     "); break;
          case MODEM_TASK_ID:
            strcpy (strTask, "Modem 2G/4G "); break;
          case NTP_TASK_ID:
            strcpy (strTask, "Net-NTP     "); break;
          case HTTP_TASK_ID:
            strcpy (strTask, "Net-HTTP(S) "); break;
          case MQTT_TASK_ID:
            strcpy (strTask, "NET MQTT(S) "); break;
          case WDT_TASK_ID:
            strcpy (strTask, "WDT Info    "); break;
        }
        TRACE_INFO_F(F("%s%s : %d\r\n"), strTask,
          (param.system_status->tasks[id].state == task_flag::suspended) ? SUSPEND_STRING :
          ((param.system_status->tasks[id].watch_dog == wdt_flag::clear) ? SPACE_STRING : FLAG_STRING),
          param.system_status->tasks[id].stack);
      }
    }
    #endif

    // Update TaskWatchDog (if all enabledModule setting local Flag)
    // else... Can Flash Saving INFO on Task Not Responding after (XXX mS)
    // For check TASK debugging Info
    if(resetWdt)
    {
      #if (ENABLE_WDT)
      TRACE_INFO_F(F("WDT: Reset WDT OK\r\n"));
      IWatchdog.reload();
      #endif
      // Reset WDT Variables for TASK
      param.systemStatusLock->Take();
      // Check All Module Setting Flag WDT before reset Global WDT
      // WDT Are called from Task into TASK While(1) Generic...
      // 
      for(uint8_t id = 0; id < TOTAL_INFO_TASK; id++) {
        // Reset only setted WDT Flags to Reset State (Not ::rest)
        if(param.system_status->tasks[id].watch_dog == wdt_flag::set)
          param.system_status->tasks[id].watch_dog = wdt_flag::clear;
      }
      param.systemStatusLock->Give();

      // Only at first step check EEProm Structure BootLoader
      // All is OK, all task started (no checked position software running_pos)
      // No Need to check WDT from All Task (if config parametr error some task)
      // Can not perform Complete LOAD and configuration BUT Application is OK
      // If All Task Start and enter in WDT Task, The application is Ready to RUN
      if(firsCheck) {
        // TEST LOG MESSAGE
        strcpy(logMessage, "Starting OK");
        param.dataLogPutQueue->Enqueue(logMessage, 0);
        // END TEST
        firsCheck = false;
        param.eeprom->Read(BOOT_LOADER_STRUCT_ADDR, (uint8_t*) &boot_check, sizeof(boot_check));
        // Flag OK Start APP Eexcuted
        if (!boot_check.app_executed_ok) {
          if(boot_check.rollback_executed) {
            TRACE_INFO_F(F("WDT: Flashing firmware with roll_back executed."));
          } else {
            TRACE_INFO_F(F("WDT: Flashing firmware with new version ready, clear flags."));
          }            
          // Remove request_upload and roll_back (firmware started OK)
          boot_check.app_executed_ok = true;
          boot_check.request_upload = false;          
          boot_check.backup_executed = false;          
          boot_check.rollback_executed = false;          
          boot_check.upload_error = 0;          
          boot_check.upload_executed = false;
          param.eeprom->Write(BOOT_LOADER_STRUCT_ADDR, (uint8_t*) &boot_check, sizeof(boot_check));
        }
      }

    }

    // Exit WDT
    DelayUntil(Ticks::MsToTicks(WDT_TASK_WAIT_DELAY_MS));
  }
}