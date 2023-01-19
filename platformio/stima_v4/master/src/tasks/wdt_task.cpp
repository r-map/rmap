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

#define TRACE_LEVEL WDT_TASK_TRACE_LEVEL

#include "tasks/wdt_task.h"
#include "drivers/module_master_hal.hpp"
#include "drivers/eeprom.h"
#include <STM32RTC.h>
#include <IWatchdog.h>

#if (ENABLE_WDT)

WdtTask::WdtTask(const char *taskName, uint16_t stackSize, uint8_t priority, WdtParam_t wdtParam) : Thread(taskName, stackSize, priority), param(wdtParam)
{
  Start();
};

void WdtTask::Run() {
  bool firsCheck = true;
  STM32RTC &rtc = STM32RTC::getInstance();
  EEprom  memEprom(param.wire, param.wireLock);

  TRACE_INFO_F(F("WDT: Starting WDT and Info Stack TASK..."));

  while (true) {

    // Check WDT Ready to reload (reset)
    bool resetWdt = true;

    TRACE_INFO_F(F("%s: "), Thread::GetName().c_str());
    // Trace DateTime
    TRACE_INFO_F(F("%02d/%02d/%02d "), rtc.getDay(), rtc.getMonth(), rtc.getYear());
    TRACE_INFO_F(F("%02d:%02d:%02d.%03d\r\n"), rtc.getHours(), rtc.getMinutes(), rtc.getSeconds(), rtc.getSubSeconds());

    // Check All Module Setting Flag WDT before reset Global WDT
    for(uint8_t id = 0; id < TOTAL_INFO_TASK; id++)
    {
      // For all Running Task (not suspend) and (real started)
      if((!param.system_status->tasks[id].is_suspend)&&
        (param.system_status->tasks[id].running_pos)) {
        // *********************************************************
        // N.B. about uint8_t RUNNING_POS:
        // RUNNING_START 1 Start Task
        // RUNNING_EXEC  2 After Start Task Task Load HW CFG OK
        // OTHER_VALUE  XX User define for LOG Task Info Error Lock Before WDT
        // *********************************************************
        // If task not performed local signal WDT
        // If Task are Longer Sleep Value of WDT is wdt_flag::rest
        // This value is automatic when TimeSleep Task > WDT_RESET_MAX
        if (param.system_status->tasks[id].watch_dog == wdt_flag::clear)
          // Wdt is not applicable (All Working TASK Need to be operative!!!)
          resetWdt = false;

        // Only at first step check EEProm Structure BootLoader
        // All is OK, all task started (no checked position software running_pos)
        // No Need to check WDT from All Task (if config parametr error some task)
        // Can not perform Complete LOAD and configuration BUT Application is OK
        // If All Task Start and enter in WDT Task, The application is Ready to RUN
        if(firsCheck) {
          firsCheck = false;
          bootloader_t boot_check;
          memEprom.Read(BOOT_LOADER_STRUCT_ADDR, (uint8_t*) &boot_check, sizeof(boot_check));
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
            boot_check.version = MODULE_MAIN_VERSION;
            boot_check.revision = MODULE_MINOR_VERSION;
            // No modify SerialNumber
            memEprom.Write(BOOT_LOADER_STRUCT_ADDR, (uint8_t*) &boot_check, sizeof(boot_check));
          }
        }
      }
    }
    // Update WatchDog (if all enabledModule setting local Flag)
    // else... Can Flash Saving INFO on Task Not Responding after (XXX mS)
    // For check TASK debugging Info
    if(resetWdt)
    {
      TRACE_INFO_F(F("WDT: Reset WDT OK\r\n"));
      IWatchdog.reload();
      // Reset WDT Variables for TASK
      param.systemStatusLock->Take();
      // Check All Module Setting Flag WDT before reset Global WDT
      // WDT Are called from Task into TASK While(1) Generic...
      // 
      for(uint8_t id = 0; id < TOTAL_INFO_TASK; id++)
      {
        // Reset only set (not suspend) WDT Flags to Reset State
        if(param.system_status->tasks[id].watch_dog == wdt_flag::set)
          param.system_status->tasks[id].watch_dog = wdt_flag::clear;
      }
      param.systemStatusLock->Give();
    }

    // Logging Stack (Only ready module task controlled)
    #if (ENABLE_STACK_USAGE)
    // Update This Task
    static u_int16_t stackUsage = (u_int16_t)uxTaskGetStackHighWaterMark( NULL );
    if((stackUsage) && (stackUsage < param.system_status->tasks[WDT_TASK_ID].stack)) {
      param.systemStatusLock->Take();
      param.system_status->tasks[WDT_TASK_ID].stack = stackUsage;
      param.systemStatusLock->Give();
    }
    TRACE_INFO_F(F("WDT: Stack Free monitor\r\n"));
    if(param.system_status->tasks[CAN_TASK_ID].stack != 0xFFFFu)
      TRACE_INFO_F(F("Can Bus        : %d\r\n"), param.system_status->tasks[CAN_TASK_ID].stack);
    if(param.system_status->tasks[LCD_TASK_ID].stack != 0xFFFFu)
    TRACE_INFO_F(F("Display LCD    : %d\r\n"), param.system_status->tasks[LCD_TASK_ID].stack);
    // if(param.system_status->tasks[ELABORATE_TASK_ID].stack != 0xFFFFu)
      // TRACE_INFO_F(F("Elaborate data : %d\r\n"), param.system_status->tasks[ELABORATE_TASK_ID].stack);
    if(param.system_status->tasks[MODEM_TASK_ID].stack != 0xFFFFu)
      TRACE_INFO_F(F("Modem 2G/4G    : %d\r\n"), param.system_status->tasks[MODEM_TASK_ID].stack);
    if(param.system_status->tasks[HTTP_TASK_ID].stack != 0xFFFFu)
      TRACE_INFO_F(F("NET-HTTP(S)    : %d\r\n"), param.system_status->tasks[HTTP_TASK_ID].stack);
    if(param.system_status->tasks[MQTT_TASK_ID].stack != 0xFFFFu)
      TRACE_INFO_F(F("NET-MQTT(S)    : %d\r\n"), param.system_status->tasks[MQTT_TASK_ID].stack);
    if(param.system_status->tasks[NTP_TASK_ID].stack != 0xFFFFu)
      TRACE_INFO_F(F("NET-NTP        : %d\r\n"), param.system_status->tasks[NTP_TASK_ID].stack);
    if(param.system_status->tasks[SUPERVISOR_TASK_ID].stack != 0xFFFFu)
      TRACE_INFO_F(F("Supervisor     : %d\r\n"), param.system_status->tasks[SUPERVISOR_TASK_ID].stack);
    TRACE_INFO_F(F("WatchDog Info  : %d\r\n"), param.system_status->tasks[WDT_TASK_ID].stack);
    #endif

    // Exit WDT
    DelayUntil(Ticks::MsToTicks(WDT_TASK_WAIT_DELAY_MS));
  }
}

#endif