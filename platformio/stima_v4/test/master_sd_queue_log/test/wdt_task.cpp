/**
  ******************************************************************************
  * @file    wdt_task.cpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   wdt_task source file (Wdt && Logging Task for Module Slave)
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

#define TRACE_LEVEL     WDT_TASK_TRACE_LEVEL
#define LOCAL_TASK_ID   WDT_TASK_ID

#include "tasks/wdt_task.h"
#include "unity.h"

#define MSG_CHECK "Starting Application StimaV4"

void test_put_data_into_queue_log_message_success(void);

void test_put_data_into_queue_log_message_success() {
  TEST_ASSERT_TRUE(true);
}

WdtTask::WdtTask(const char *taskName, uint16_t stackSize, uint8_t priority, WdtParam_t wdtParam) : Thread(taskName, stackSize, priority), param(wdtParam)
{
  Start();
};

void WdtTask::Run() {

  bool firtsCheck = true;
  char logMessage[128];

  // ************************************************************************
  // ***************************** TEST BEGIN *******************************
  // ************************************************************************

  UNITY_BEGIN();

  // Necessary delay to start test
  delay(3000);

  while (true) {

    if(firtsCheck) {
      // ***** TEST PUT DATA INTO QUEUE LOG MESSAGE
      Serial.println("// ***** TEST PUT DATA INTO QUEUE LOG MESSAGE");
      RUN_TEST(test_put_data_into_queue_log_message_success);
      // TEST LOG MESSAGE
      strcpy(logMessage, MSG_CHECK);
      param.dataLogPutQueue->Enqueue(logMessage, 0);
      // END TEST
      firtsCheck = false;
    }

    // Not used WatchDog... on TEST
    IWatchdog.reload();
    DelayUntil(Ticks::MsToTicks(WDT_TASK_WAIT_DELAY_MS));

  }

}