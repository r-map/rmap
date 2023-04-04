/**@file supervisor_task.cpp */

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

#define TRACE_LEVEL   SUPERVISOR_TASK_TRACE_LEVEL
#define LOCAL_TASK_ID SUPERVISOR_TASK_ID

#include "tasks/supervisor_task.h"

#include "unity.h"

// TODO: Move to MQTT
#include "date_time.h"
// Namespace RMAP
#include <rmap/_module/RAIN_1_0.h>
#include <rmap/_module/TH_1_0.h>
#include <rmap/service/_module/RAIN_1_0.h>
#include <rmap/service/_module/TH_1_0.h>

extern bool starting_test;
extern uint32_t DATE_TIME_PTR_TEST;
extern uint32_t data_field_create;

bool rmap_data_error;
bool rmap_eof;  // Rmap Pointer setted? Get All Data from RMAP Archive
rmap_get_response_t rmap_get_response;
uint32_t rmap_date_time_ptr;
uint32_t countData;  // Tot rec. read ( Starting with 1 because first data is getted UP to check TEST UP)

// ************************************************************************
// **************** TEST SUPERVISOR FUNCTION DECLARATIONS *****************
// ************************************************************************

void test_get_all_queue_data(void);
void test_get_next_queue_data_failed(void);
void test_get_next_queue_data_success(void);
void test_get_response_ptr_with_real_existing_date(void);
void test_num_data_write_is_equal_to_num_data_read(void);
void test_setting_ptr_with_date_before_existing_date_failed(void);
void test_setting_ptr_with_date_before_existing_date_success(void);
void test_setting_ptr_with_real_existing_request_date(void);

// ************************************************************************
// **************** TEST SUPERVISOR FUNCTION IMPLEMENTATIONS **************
// ************************************************************************

/**
 * @brief TEST: Get all queue data
 *
 */
void test_get_all_queue_data() {
    TEST_ASSERT_EQUAL(true, (rmap_eof) && (!rmap_data_error));
}

/**
 * @brief TEST: Get next queue data failed
 *
 */
void test_get_next_queue_data_failed() {
    TEST_ASSERT_TRUE(false);
}

/**
 * @brief TEST: Get next queue data success
 *
 */
void test_get_next_queue_data_success() {
    TEST_ASSERT_TRUE(true);
}

/**
 * @brief TEST: Get response ptr with real existing date
 *
 */
void test_get_response_ptr_with_real_existing_date() {
    TEST_ASSERT_EQUAL(true, rmap_get_response.result.done_synch && rmap_get_response.rmap_data.date_time != 0);
}

/**
 * @brief TEST: Check if the number of data written is equal to the number of data read
 *
 */
void test_num_data_write_is_equal_to_num_data_read() {
    TEST_ASSERT_EQUAL(data_field_create, countData);
}

/**
 * @brief TEST: Setting ptr with date before existing date failed
 *
 */
void test_setting_ptr_with_date_before_existing_date_failed() {
    TEST_ASSERT_TRUE(false);
}

/**
 * @brief TEST: Setting ptr with date before existing date success
 *
 */
void test_setting_ptr_with_date_before_existing_date_success() {
    TEST_ASSERT_TRUE(true);
}

/**
 * @brief TEST: Setting ptr with real existing request date
 *
 */
void test_setting_ptr_with_real_existing_request_date() {
    TEST_ASSERT_EQUAL(rmap_date_time_ptr, rmap_get_response.rmap_data.date_time);
}

using namespace cpp_freertos;

SupervisorTask::SupervisorTask(const char *taskName, uint16_t stackSize, uint8_t priority, SupervisorParam_t supervisorParam) : Thread(taskName, stackSize, priority), param(supervisorParam) {
    // Start WDT controller and TaskState Flags
    TaskWatchDog(WDT_STARTING_TASK_MS);
    TaskState(SUPERVISOR_STATE_CREATE, UNUSED_SUB_POSITION, task_flag::normal);

    state = SUPERVISOR_STATE_INIT;
    Start();
};

#if (ENABLE_STACK_USAGE)
/// @brief local stack Monitor (optional)
void SupervisorTask::TaskMonitorStack() {
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
void SupervisorTask::TaskWatchDog(uint32_t millis_standby) {
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

/// @brief local suspend flag and positor running state Task (optional)
/// @param state_position Sw_Position (Local STATE)
/// @param state_subposition Sw_SubPosition (Optional Local SUB_STATE Position Monitor)
/// @param state_operation operative mode flag status for this task
void SupervisorTask::TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation) {
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

void SupervisorTask::Run() {
    uint8_t retry;
    connection_request_t connection_request;
    connection_response_t connection_response;
    SupervisorConnection_t state_check_connection;  // Local state (operation) when module connected

    bool bExecRepetedTest;

// Start Running Monitor and First WDT normal state
#if (ENABLE_STACK_USAGE)
    TaskMonitorStack();
#endif
    TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

    while (true) {
        bool is_saved = false;
        bool is_loaded = false;

        switch (state) {
            case SUPERVISOR_STATE_INIT:

                // **********************************************************
                //   TEST GET RMAP Data AND SET POINTER With Request Queue
                // **********************************************************
                if (starting_test) {
                    bExecRepetedTest = false;
                    rmap_data_error = false;
                    rmap_get_request_t rmap_get_request;

                    // MMC have to GET Ready before Push DATA
                    // EXIT from function if not MMC Ready or present into system_status
                    if (!param.system_status->flags.sd_card_ready) {
                        TRACE_VERBOSE_F(F("SUPERVISOR: Reject request get rmap data, MMC was not ready [ %s ]\r\n"), ERROR_STRING);
                        break;
                    }

                    // ***** START TEST SET POINTER DATA REQUEST INTO EXISTING DATA *****
                    DateTime date_request;
                    date_request.day = 29;
                    date_request.month = 3;
                    date_request.year = 2023;
                    date_request.hours = 4;
                    date_request.minutes = 22;
                    date_request.seconds = 23;
                    rmap_date_time_ptr = convertDateToUnixTime(&date_request);

                    memset(&rmap_get_request, 0, sizeof(rmap_get_request_t));
                    rmap_get_request.param = rmap_date_time_ptr;
                    rmap_get_request.command.do_synch_ptr = true;
                    // Optional Save Pointer in File (Probabiliy always in SetPtr)
                    rmap_get_request.command.do_save_ptr = true;
                    TRACE_VERBOSE_F(F("Starting request SET Data RMAP PTR to local SD\r\n"));
                    // Push data request to queue MMC
                    param.dataRmapGetRequestQueue->Enqueue(&rmap_get_request, 0);

                    // Non blocking task
                    TaskWatchDog(SUPERVISOR_TASK_WAIT_DELAY_MS);
                    Delay(Ticks::MsToTicks(SUPERVISOR_TASK_WAIT_DELAY_MS));

                    // Waiting response from MMC with TimeOUT
                    memset(&rmap_get_response, 0, sizeof(rmap_get_response));
                    // Seek Operation can Be Long Time Procedure. Queue can be post in waiting state without Time End
                    // Task WDT Are suspended
                    TaskState(state, UNUSED_SUB_POSITION, task_flag::suspended);
                    rmap_data_error = !param.dataRmapGetResponseQueue->Dequeue(&rmap_get_response);
                    TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);
                    rmap_data_error |= rmap_get_response.result.event_error;

                    RUN_TEST(test_get_response_ptr_with_real_existing_date);

                    if (rmap_get_response.result.done_synch) {
                        // If SYNCH WITH EXISTING DATA non need to get data. First data is automatically charged into response
                        // TEST (Sincronizzo sui 15 minuti un puntatore generico date_time)
                        rmap_date_time_ptr /= 900;
                        rmap_date_time_ptr *= 900;
                        rmap_date_time_ptr += 900;

                        RUN_TEST(test_setting_ptr_with_real_existing_request_date);
                    }

                    // ***** START TEST SET POINTER DATA REQUEST BEFORE EXISTING DATA *****
                    date_request.day = 16;
                    date_request.month = 2;
                    date_request.year = 2021;
                    date_request.hours = 4;
                    date_request.minutes = 22;
                    date_request.seconds = 23;
                    rmap_date_time_ptr = convertDateToUnixTime(&date_request);

                    memset(&rmap_get_request, 0, sizeof(rmap_get_request_t));
                    rmap_get_request.param = rmap_date_time_ptr;
                    rmap_get_request.command.do_synch_ptr = true;
                    // Optional Save Pointer in File (Probabiliy always in SetPtr)
                    rmap_get_request.command.do_save_ptr = true;
                    TRACE_VERBOSE_F(F("Starting request SET Data RMAP (BEFORE EXISTING) PTR to local SD\r\n"));
                    // Push data request to queue MMC
                    param.dataRmapGetRequestQueue->Enqueue(&rmap_get_request, 0);

                    // Non blocking task
                    TaskWatchDog(SUPERVISOR_TASK_WAIT_DELAY_MS);
                    Delay(Ticks::MsToTicks(SUPERVISOR_TASK_WAIT_DELAY_MS));

                    // Waiting response from MMC with TimeOUT
                    memset(&rmap_get_response, 0, sizeof(rmap_get_response));
                    // Seek Operation can Be Long Time Procedure. Queue can be post in waiting state without Time End
                    // Task WDT Are suspended
                    TaskState(state, UNUSED_SUB_POSITION, task_flag::suspended);
                    rmap_data_error = !param.dataRmapGetResponseQueue->Dequeue(&rmap_get_response);
                    TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);
                    rmap_data_error |= rmap_get_response.result.event_error;

                    // TEST (Sincronizzo sui 15 minuti un puntatore generico date_time)
                    if (rmap_get_response.result.done_synch) {
                        // No data is autmatic readed because pointer exceded limit date... (rmap_data.date_time==0)
                        // Need to read first data with command get_data... to queue request
                        if (rmap_get_response.rmap_data.date_time == 0) {
                            memset(&rmap_get_request, 0, sizeof(rmap_get_request));
                            // Get Next data... Stop at EOF
                            rmap_get_request.command.do_get_data = true;
                            // Save Pointer? Optional
                            // rmap_get_request.command.do_save_ptr = true;
                            // Push data request to queue MMC
                            param.dataRmapGetRequestQueue->Enqueue(&rmap_get_request, 0);
                            // Waiting response from MMC with TimeOUT
                            memset(&rmap_get_response, 0, sizeof(rmap_get_response));
                            TaskWatchDog(FILE_IO_DATA_QUEUE_TIMEOUT);
                            rmap_data_error = !param.dataRmapGetResponseQueue->Dequeue(&rmap_get_response, FILE_IO_DATA_QUEUE_TIMEOUT);
                            rmap_data_error |= rmap_get_response.result.event_error;

                            if (!rmap_data_error) {
                                // EOF Data? (Save and Exit, after last data process)
                                if (rmap_get_response.result.done_get_data) {
                                    if (rmap_get_response.rmap_data.date_time == DATE_TIME_PTR_TEST) {
                                        RUN_TEST(test_setting_ptr_with_date_before_existing_date_success);
                                    } else {
                                        RUN_TEST(test_setting_ptr_with_date_before_existing_date_failed);
                                    }
                                }
                            } else {
                                RUN_TEST(test_setting_ptr_with_date_before_existing_date_failed);
                            }
                        } else {
                            RUN_TEST(test_setting_ptr_with_date_before_existing_date_failed);
                        }
                    }

                    char outString[256];
                    countData = 1;
                    rmap_eof = false;

                    // Exit on End of data or Error from queue
                    while ((!rmap_data_error) && (!rmap_eof)) {
                        memset(&rmap_get_request, 0, sizeof(rmap_get_request));
                        // Get Next data... Stop at EOF
                        rmap_get_request.command.do_get_data = true;
                        // Save Pointer? Optional
                        // rmap_get_request.command.do_save_ptr = true;
                        // Push data request to queue MMC
                        param.dataRmapGetRequestQueue->Enqueue(&rmap_get_request, 0);
                        // Waiting response from MMC with TimeOUT
                        memset(&rmap_get_response, 0, sizeof(rmap_get_response));
                        TaskWatchDog(FILE_IO_DATA_QUEUE_TIMEOUT);
                        rmap_data_error = !param.dataRmapGetResponseQueue->Dequeue(&rmap_get_response, FILE_IO_DATA_QUEUE_TIMEOUT);
                        rmap_data_error |= rmap_get_response.result.event_error;
                        if (!rmap_data_error) {
                            // EOF Data? (Save and Exit, after last data process)
                            rmap_eof = rmap_get_response.result.end_of_data;
                            // ******************************************************************
                            // Exampe of Current Session Upload CountData and DateTime Block Print
                            countData++;
                            DateTime rmap_date_time_val;
                            convertUnixTimeToDate(rmap_get_response.rmap_data.date_time, &rmap_date_time_val);
                            if (bExecRepetedTest == false) {
                                RUN_TEST(test_get_next_queue_data_success);
                                sprintf(outString, "Data RMAP current date/time (Start at 2) [ %d ] %s\r\n", (uint32_t)countData, formatDate(&rmap_date_time_val, NULL));
                                Serial.println(outString);
                                // Only one check ( ON ERROR ALL CASE IS PRINTED )
                                bExecRepetedTest = true;
                            }

                            // ******************************************************************
                            // Process Data with casting RMAP Module Type
                            switch (rmap_get_response.rmap_data.module_type) {
                                case Module_Type::th:
                                    rmap_module_TH_1_0 *rmapDataTH;
                                    rmapDataTH = (rmap_module_TH_1_0 *)&rmap_get_response.rmap_data;
#if (ENABLE_STACK_USAGE)
                                    TaskMonitorStack();
#endif
                                    // Prepare MQTT String -> rmapDataTH->ITH.humidity.val.value ecc...
                                    // PUT String MQTT in Buffer Memory...
                                    // SEND To MQTT Server
                                    break;
                                case Module_Type::rain:
                                    rmap_module_Rain_1_0 *rmapDataRain;
                                    rmapDataRain = (rmap_module_Rain_1_0 *)&rmap_get_response.rmap_data;
#if (ENABLE_STACK_USAGE)
                                    TaskMonitorStack();
#endif
                                    // Prepare MQTT String -> rmapDataRain->TBR.metadata.level.L1.value ecc...
                                    // PUT String MQTT in Buffer Memory...
                                    // SEND To MQTT Server
                                    break;
                            }
                        } else {
                            RUN_TEST(test_get_next_queue_data_failed);
                            TRACE_VERBOSE_F(F("RMAP Reading Data queue error!!!\r\n"));
                        }
                        // Non blocking task
                        TaskWatchDog(TASK_WAIT_REALTIME_DELAY_MS);
                        Delay(Ticks::MsToTicks(TASK_WAIT_REALTIME_DELAY_MS));
                    }

                    RUN_TEST(test_get_all_queue_data);
                    RUN_TEST(test_num_data_write_is_equal_to_num_data_read);

                    UNITY_END();

                    // ************************************************************************
                    // ***************************** TEST END *********************************
                    // ************************************************************************

                    // Trace END Data response
                    TRACE_VERBOSE_F(F("Uploading data RMAP Archive [ %s ]. Updated %d record\r\n"), rmap_eof ? OK_STRING : ERROR_STRING, countData);
                }

                break;

            case SUPERVISOR_STATE_END:
                TRACE_VERBOSE_F(F("SUPERVISOR_STATE_END -> SUPERVISOR_STATE_WAITING_EVENT\r\n"));
                state = SUPERVISOR_STATE_WAITING_EVENT;
                break;
        }

#if (ENABLE_STACK_USAGE)
        TaskMonitorStack();
#endif

        // One step base non blocking switch
        TaskWatchDog(SUPERVISOR_TASK_WAIT_DELAY_MS);
        Delay(Ticks::MsToTicks(SUPERVISOR_TASK_WAIT_DELAY_MS));
    }
}
