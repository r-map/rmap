/**
 ******************************************************************************
 * @file    sd_task.cpp
 * @author  Moreno Gasperini <m.gasperini@digiteco.it>
 * @brief   sd_task source file (SD SPI StimaV4)
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

#define TRACE_LEVEL   SD_TASK_TRACE_LEVEL
#define LOCAL_TASK_ID SD_TASK_ID

#define MSG_CHECK "Starting Application StimaV4"

#include "tasks/sd_task.h"

#include "date_time.h"
#include "unity.h"

// SD Istance
SdFs SD;
// Queue buffer for logging
bool is_getted_rtc;
char logBuffer[LOG_PUT_DATA_ELEMENT_SIZE];
File logFile;

#if (ENABLE_SD)

using namespace cpp_freertos;

// ************************************************************************
// ******************** TEST SD FUNCTION DECLARATIONS *********************
// ************************************************************************

void test_sd_card_created_log_directory(void);
void test_sd_card_exists_log_directory(void);
void test_sd_card_incoming_queue_message_to_log(void);
void test_sd_card_init(void);
void test_sd_card_open_file_for_write_log(void);
void test_sd_card_read_back_message_log(void);
void test_sd_card_remove_log_file(void);

// ************************************************************************
// ******************** TEST SD FUNCTION IMPLEMENTATIONS ******************
// ************************************************************************

/**
 * @brief TEST: Create a log directory
 *
 */
void test_sd_card_created_log_directory() {
    TEST_ASSERT_EQUAL(true, SD.mkdir("log"));
}

/**
 * @brief TEST: Check if the log directory exists
 *
 */
void test_sd_card_exists_log_directory() {
    TEST_ASSERT_EQUAL(true, SD.exists("log"));
}

/**
 * @brief TEST: Check incoming queue message to log
 *
 */
void test_sd_card_incoming_queue_message_to_log() {
    TEST_ASSERT_EQUAL(true, !is_getted_rtc);
}

/**
 * @brief TEST: Initialization SD card
 *
 */
void test_sd_card_init() {
    TEST_ASSERT_EQUAL(true, SD.begin(PIN_SPI_SS, 8));
}

/**
 * @brief TEST: Check if the file is opened correctly for writing
 *
 */
void test_sd_card_open_file_for_write_log() {
    TEST_ASSERT_EQUAL(true, logFile);
}

/**
 * @brief TEST: Reading back message log
 *
 */
void test_sd_card_read_back_message_log() {
    TEST_ASSERT_NOT_EQUAL(false, strstr(logBuffer, MSG_CHECK));
}

/**
 * @brief TEST: Remove a file
 *
 */
void test_sd_card_remove_log_file() {
    TEST_ASSERT_EQUAL(true, SD.remove("log/log.txt"));
}

SdTask::SdTask(const char *taskName, uint16_t stackSize, uint8_t priority, SdParam_t sdParam) : Thread(taskName, stackSize, priority), param(sdParam) {
    state = SD_STATE_INIT;
    Start();
};

void SdTask::Run() {
    // Generic retry
    uint8_t retry;
    bool message_traced = false;
    char logIntest[23] = {0};
    uint32_t delay_start;

    // SD-CARD Setup PIN CS SD UPIN27
    pinMode(PIN_SPI_SS, OUTPUT);
    digitalWrite(PIN_SPI_SS, HIGH);

    delay_start = millis();

    while (true) {
        switch (state) {
            case SD_STATE_INIT:

                RUN_TEST(test_sd_card_init);

                // Check SD or Resynch after Error
                if (SD.begin(PIN_SPI_SS, 8)) {
                    state = SD_STATE_CHECK_SD;
                    message_traced = false;
                }
                break;

            case SD_STATE_CHECK_SD:
                // Optional Trace Type of CARD... and Size
                // Check or create directory Structure...
                if (!SD.exists("log")) {
                    RUN_TEST(test_sd_card_created_log_directory);
                }

                RUN_TEST(test_sd_card_exists_log_directory);

                // TEST Only 2° Attempt if not exixtsing file...
                if (SD.exists("log/log.txt")) {
                    RUN_TEST(test_sd_card_remove_log_file);
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
                while (!param.dataLogPutQueue->IsEmpty()) {
                    RUN_TEST(test_sd_card_incoming_queue_message_to_log);
                    if (!is_getted_rtc) {
                        // Get date time to Intest string to PUT (for this message session)
                        is_getted_rtc = true;
                        if (param.rtcLock->Take(Ticks::MsToTicks(RTC_WAIT_DELAY_MS))) {
                            sprintf(logIntest, "%02d/%02d/%02d %02d:%02d:%02d.%03d ",
                                    rtc.getDay(), rtc.getMonth(), rtc.getYear(), rtc.getHours(), rtc.getMinutes(), rtc.getSeconds(), rtc.getSubSeconds());
                            param.rtcLock->Give();
                        }
                    }
                    // Get message from queue
                    if (param.dataLogPutQueue->Dequeue(logBuffer)) {
                        // Put to SD ( APPEND File Always Opened with Flush Data )
                        if (!logFile) logFile = SD.open("log/log.txt", FILE_WRITE | O_APPEND);
                        RUN_TEST(test_sd_card_open_file_for_write_log);
                        if (logFile) {
                            logFile.print(logIntest);
                            logFile.write(logBuffer, strlen(logBuffer) < LOG_PUT_DATA_ELEMENT_SIZE ? strlen(logBuffer) : LOG_PUT_DATA_ELEMENT_SIZE);
                            logFile.println();
                            logFile.flush();
                            logFile.close();
                            // TEST FOR READ STRING INTO IS SAME TO
                            memcpy(logBuffer, 0, sizeof(logBuffer));
                            logFile = SD.open("log/log.txt", FILE_READ);
                            uint8_t idx = 0;
                            while (logFile.available()) {
                                logBuffer[idx++] = logFile.read();
                            }
                            RUN_TEST(test_sd_card_read_back_message_log);
                        }
                    }

                    UNITY_END();

                    // ************************************************************************
                    // ***************************** TEST END *********************************
                    // ************************************************************************
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