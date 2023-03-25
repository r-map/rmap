/**@file main.cpp */

/*********************************************************************
Copyright (C) 2022  Marco Baldinetti <m.baldinetti@digiteco.it>
authors:
Marco Baldinetti <m.baldinetti@digiteco.it>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
<http://www.gnu.org/licenses/>.
**********************************************************************/

#define TRACE_LEVEL STIMA_TRACE_LEVEL

#include "main.h"
#include "Arduino.h"

HardwareSerial Serial2(PD_6, PD_5);

void setup() {
 
  // Semaphore, Queue && Param Config for TASK
#if (ENABLE_I2C1)
  static BinarySemaphore *wireLock;       // Access I2C external interface UPIN_27
#endif

#if (ENABLE_I2C2)
  static BinarySemaphore *wire2Lock;      // Access I2C internal EEprom, Display
#endif

#if (ENABLE_CAN)
  static BinarySemaphore *canLock;        // Can BUS
#endif

#if (ENABLE_QSPI)
  static BinarySemaphore *qspiLock;       // Qspi (Flash Memory)
#endif

  static BinarySemaphore *rtcLock;        // RTC (Access lock)

  static BinarySemaphore *rpcLock;        // RPC (Access lock)

  // System semaphore
  static BinarySemaphore *configurationLock;  // Access Configuration
  static BinarySemaphore *systemStatusLock;   // Access System status
  static BinarySemaphore *registerAccessLock; // Access Register Cyphal Specifications

  // System Queue (Generic Message from/to Task)
  static Queue *systemMessageQueue;
  // Data queue (Request / exchange data from Data Task)
  static Queue *connectionRequestQueue;
  static Queue *connectionResponseQueue;
  //Data MMC/SD WR/RD
  static Queue *dataRmapPutQueue;
  static Queue *dataRmapGetRequestQueue;
  static Queue *dataRmapGetResponseQueue;
  static Queue *dataFilePutRequestQueue;
  static Queue *dataFilePutResponseQueue;
  static Queue *dataFileGetRequestQueue;
  static Queue *dataFileGetResponseQueue;
  static Queue *dataLogPutQueue;

  // System and status configuration struct
  static configuration_t configuration = {0};
  static system_status_t system_status = {0};

  // Initializing basic hardware's configuration, variables and function
  SetupSystemPeripheral();
  init_debug(SERIAL_DEBUG_BAUD_RATE);
  init_wire();
  init_rtc(INIT_PARAMETER);

  TRACE_INFO_F(F("Initialization HW Base done\r\n\r\n"));

  // Get Serial Number and Print Fixed to Serial logger default
  configuration.board_master.serial_number = StimaV4GetSerialNumber();
  Serial.println();
  Serial.println(F("*****************************"));
  Serial.println(F("* Stima V4 MASTER - SER.NUM *"));
  Serial.println(F("*****************************"));
  Serial.print(F("COD: "));
  for(int8_t id=7; id>=0; id--) {
    if((uint8_t)((configuration.board_master.serial_number >> (8*id)) & 0xFF) < 16) Serial.print(F("0"));
    Serial.print((uint8_t)((configuration.board_master.serial_number >> (8*id)) & 0xFF), 16);
    if(id) Serial.print(F("-"));
  }
  Serial.println("\r\n");

  // ***************************************************************
  //           Setup parameter for Task and local Class
  // ***************************************************************

#if (ENABLE_USBSERIAL)
 // TASK SUPERVISOR PARAM CONFIG
  static UsbSerialParam_t usbSerialParam = {0};
  usbSerialParam.configuration = &configuration;
  usbSerialParam.system_status = &system_status;
  usbSerialParam.systemMessageQueue = systemMessageQueue;
  usbSerialParam.qspiLock = qspiLock;  
  usbSerialParam.rtcLock = rtcLock;
  usbSerialParam.configurationLock = configurationLock;
  usbSerialParam.systemStatusLock = systemStatusLock;
  usbSerialParam.rpcLock = rpcLock;
#endif

  // *****************************************************************************
  // Startup Task, Supervisor as first for Loading parameter generic configuration
  // *****************************************************************************

#if (ENABLE_USBSERIAL)
  static UsbSerialTask usbSerial_task("UsbSerialTask", 1100, OS_TASK_PRIORITY_01, usbSerialParam);
#endif

  // Startup Schedulher
  Thread::StartScheduler();
}

// FreeRTOS idleHook callBack to loop
void loop() {
}

// Setup Wire I2C SPI Interface
void init_wire()
{
  // Setup I2C
#if (ENABLE_I2C1)
  Wire.begin();
  Wire.setClock(I2C1_BUS_CLOCK_HZ);
#endif

#if (ENABLE_I2C2)
  Wire2.begin();
  Wire2.setClock(I2C2_BUS_CLOCK_HZ);
#endif
  
  // Start EN Pow GSM ready to SET
  digitalWrite(PIN_GSM_EN_POW, HIGH);
}

// Setup RTC HW && LowPower Class STM32
void init_rtc(bool init)
{
  // Init istance to STM RTC object
  STM32RTC& rtc = STM32RTC::getInstance();
  // Select RTC clock source: LSE_CLOCK (First Istance)
  rtc.setClockSource(STM32RTC::LSE_CLOCK);
  rtc.begin(); // initialize RTC 24H format
  // Set the time if requireq to Reset value
  if(init) {
    // Set the date && Time Init Value
    rtc.setHours(0);rtc.setMinutes(0);rtc.setSeconds(0);
    rtc.setWeekDay(0);rtc.setDay(1);rtc.setMonth(1);rtc.setYear(23);
  }
  // Start LowPower configuration
  LowPower.begin();
}

