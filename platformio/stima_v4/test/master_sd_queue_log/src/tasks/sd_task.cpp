/**
  ******************************************************************************
  * @file    sd_task.cpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   sd_task source file (SD SPI StimaV4)
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

#define TRACE_LEVEL     SD_TASK_TRACE_LEVEL
#define LOCAL_TASK_ID   SD_TASK_ID

#define MSG_CHECK "Starting Application StimaV4"

#include "tasks/sd_task.h"
#include "date_time.h"

#if (ENABLE_SD)

using namespace cpp_freertos;

SdTask::SdTask(const char *taskName, uint16_t stackSize, uint8_t priority, SdParam_t sdParam) : Thread(taskName, stackSize, priority), param(sdParam)
{
  state = SD_STATE_INIT;
  Start();
};

void SdTask::Run()
{

  // Generic retry
  uint8_t retry;
  bool message_traced = false;
  bool is_getted_rtc;
  // Queue buffer for logging
  char logBuffer[LOG_PUT_DATA_ELEMENT_SIZE];
  char logIntest[23] = {0};
  File logFile;
  uint32_t delay_start;

  // SD-CARD Setup PIN CS SD UPIN27
  pinMode(PIN_SPI_SS, OUTPUT);
  digitalWrite(PIN_SPI_SS, HIGH);

  delay_start = millis();

  while (true)
  {

    switch (state)
    {
    case SD_STATE_INIT:
      // Check SD or Resynch after Error
      if (SD.begin(PIN_SPI_SS, 8)) {
        // ***** TEST SD CARD INIT OK
        Serial.println("// ***** TEST SD CARD INIT OK");
        state = SD_STATE_CHECK_SD;
        message_traced = false;
      } else {
        // Only one TRACE message... SD Not present
        if(millis()-delay_start > 10000) {
          // ***** TEST SD CARD INIT FAIL
          Serial.println("// ***** TEST SD CARD INIT FAIL");
          // END UNITY
        }
      }
      break;

    case SD_STATE_CHECK_SD:
      // Optional Trace Type of CARD... and Size
      // Check or create directory Structure...
      if(!SD.exists("log")) {
        SD.mkdir("log");
      }

      if(SD.exists("log")) {
        // ***** TEST SD CARD CREATE DIR OK
        Serial.println("// ***** TEST SD CARD CREATE DIR OK");
      } else {
        // ***** TEST SD CARD CREATE DIR FAIL
        Serial.println("// ***** TEST SD CARD CREATE DIR FAILED");
      }

      // TEST Only 2° Attempt if not exixtsing file...
      if(SD.exists("log/log.txt")) {
        SD.remove("log/log.txt");
        // ***** TEST SD REMOVE LOG FILE OK
        Serial.println("// ***** TEST SD REMOVE LOG FILE OK");
      }

      // ***************************************************
      // SD Was Ready... for System Structure and Pointer OK
      // ***************************************************
      param.systemStatusLock->Take();
      param.system_status->flags.sd_card_ready = true;
      param.systemStatusLock->Give();

      state = SD_STATE_WAITING_EVENT;
      break;

    case SD_STATE_WAITING_EVENT:

      // *********************************************************
      //             Perform LOG WRITE append message 
      // *********************************************************
      // If element get all element from the queue and Put to SD
      // Typical Put of Logging are Time controlled from TASK (If queue are free into reasonable time LOG is pushed)
      // Log queue element is reasonable sized to avoid problems
      // File are always opened if Append for fast Access Operation
      // File can be opened simultaneously also readonly mode by another function es.Read/Print/Send INFO LOG
      is_getted_rtc = false;
      while(!param.dataLogPutQueue->IsEmpty()) {
        if(!is_getted_rtc) {
          // ***** TEST SD CARD INCOMING QUEUE MESSAGE TO LOG OK
          Serial.println("// ***** TEST SD CARD INCOMING QUEUE MESSAGE TO LOG OK");
          // Get date time to Intest string to PUT (for this message session)
          is_getted_rtc = true;
          if(param.rtcLock->Take(Ticks::MsToTicks(RTC_WAIT_DELAY_MS))) {
            sprintf(logIntest, "%02d/%02d/%02d %02d:%02d:%02d.%03d ",
              rtc.getDay(), rtc.getMonth(), rtc.getYear(), rtc.getHours(), rtc.getMinutes(), rtc.getSeconds(), rtc.getSubSeconds());
            param.rtcLock->Give();
          }
        }
        // Get message from queue
        if(param.dataLogPutQueue->Dequeue(logBuffer)) {
          // Put to SD ( APPEND File Always Opened with Flush Data )
          if(!logFile) logFile = SD.open("log/log.txt", FILE_WRITE | O_APPEND);
          if(logFile) {
            // ***** TEST SD CARD OPEN FILE FOR WRITE LOG OK
            Serial.println("// ***** TEST SD CARD OPEN FILE FOR WRITE LOG OK");
            logFile.print(logIntest);
            logFile.write(logBuffer, strlen(logBuffer) < LOG_PUT_DATA_ELEMENT_SIZE ? strlen(logBuffer) : LOG_PUT_DATA_ELEMENT_SIZE);
            logFile.println();
            logFile.flush();
            logFile.close();
            // TEST FOR READ STRING INTO IS SAME TO
            memcpy(logBuffer, 0, sizeof(logBuffer));
            logFile = SD.open("log/log.txt", FILE_READ);
            uint8_t idx = 0;
            while(logFile.available()) {
              logBuffer[idx++] = logFile.read();
            }
            if(strstr(logBuffer, MSG_CHECK)) {
              // ***** TEST SD CARD READ BACK MESSAGE LOG OK
              Serial.println("// ***** TEST SD CARD READ BACK MESSAGE LOG OK");
            } else {
              // ***** TEST SD CARD READ BACK MESSAGE LOG FAIL
              Serial.println("// ***** TEST SD CARD READ BACK MESSAGE LOG FAIL");
            }
            // ***** TEST END
            Serial.println("// ***** TEST END");
          } else {
            // ***** TEST SD CARD OPEN FILE FOR WRITE LOG FAILED
            Serial.println("// ***** TEST SD CARD OPEN FILE FOR WRITE LOG FAILED");
          }
        }
      }
      // *********************************************************
      //             End OF perform LOG append message
      // *********************************************************
      break;

    case SD_STATE_ERROR:
      // Gest Error... Resynch SD
      TRACE_VERBOSE_F(F("SD_STATE_ERROR -> SD_STATE_INIT\r\n"));
      state = SD_STATE_INIT;
      break;
    }

    Delay(Ticks::MsToTicks(SD_TASK_WAIT_DELAY_MS));

  }
}

#endif