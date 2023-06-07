/**@file ntp_task.cpp */

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

#define TRACE_LEVEL   NTP_TASK_TRACE_LEVEL
#define LOCAL_TASK_ID NTP_TASK_ID

#include "tasks/ntp_task.h"

#include "unity.h"

#if (USE_NTP)

using namespace cpp_freertos;

error_t error;
uint32_t test_epoch;
time_t unixTime;

// ************************************************************************
// ******************* TEST NTP FUNCTION DECLARATIONS *********************
// ************************************************************************

void test_init_ntp_cyclone(void);
void test_init_ntp_retrieve_time_stamp(void);
void test_requested_queue_do_ntp_start(void);
void test_requested_resolve_name_server_ntp(void);
void test_requested_resolve_name_server_ntp_timeout_error(void);
void test_rtc_set_date_time(void);

// ************************************************************************
// ******************** TEST NTP FUNCTION IMPLEMENTATIONS ******************
// ************************************************************************

void test_init_ntp_cyclone() {
    TEST_ASSERT_EQUAL(false, error);
}

void test_init_ntp_retrieve_time_stamp() {
    TEST_ASSERT_EQUAL(true, !error);
}

void test_requested_queue_do_ntp_start() {
    TEST_ASSERT_TRUE(true);
}

void test_requested_resolve_name_server_ntp() {
    TEST_ASSERT_EQUAL(false, error);
}

void test_requested_resolve_name_server_ntp_timeout_error() {
    TEST_ASSERT_EQUAL(false, error);
}

void test_rtc_set_date_time() {
    TEST_ASSERT_EQUAL(true, (test_epoch == unixTime) || (test_epoch == unixTime + 1));
}

NtpTask::NtpTask(const char *taskName, uint16_t stackSize, uint8_t priority, NtpParam_t ntpParam) : Thread(taskName, stackSize, priority), param(ntpParam) {
    // Start WDT controller and TaskState Flags
    TaskWatchDog(WDT_STARTING_TASK_MS);
    TaskState(NTP_STATE_CREATE, UNUSED_SUB_POSITION, task_flag::normal);

    state = NTP_STATE_INIT;
    Start();
};

#if (ENABLE_STACK_USAGE)
/// @brief local stack Monitor (optional)
void NtpTask::TaskMonitorStack() {
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
void NtpTask::TaskWatchDog(uint32_t millis_standby) {
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
void NtpTask::TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation) {
    // Local TaskWatchDog update
    param.systemStatusLock->Take();
    // Signal Task sleep/disabled mode from request (Auto SET WDT on Resume)
    if ((param.system_status->tasks[LOCAL_TASK_ID].state == task_flag::suspended) &&
        (state_operation == task_flag::normal))
        param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::set;
    param.system_status->tasks[LOCAL_TASK_ID].state = state_operation;
    param.system_status->tasks[LOCAL_TASK_ID].running_pos = state_position;
    param.system_status->tasks[LOCAL_TASK_ID].running_sub = state_subposition;
    param.systemStatusLock->Give();
}

void NtpTask::Run() {
    uint8_t retry;
    bool is_error;
    uint32_t kissCode;
    DateTime date;
    IpAddr ipAddr;
    NtpTimestamp timestamp;

    connection_request_t connection_request;
    connection_response_t connection_response;

// Start Running Monitor and First WDT normal state
#if (ENABLE_STACK_USAGE)
    TaskMonitorStack();
#endif
    TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

    while (true) {
        switch (state) {
            case NTP_STATE_INIT:
                TRACE_VERBOSE_F(F("NTP_STATE_INIT -> NTP_STATE_WAIT_NET_EVENT\r\n"));
                state = NTP_STATE_WAIT_NET_EVENT;
                break;

            case NTP_STATE_WAIT_NET_EVENT:
                is_error = false;
                retry = 0;

                // ************************************************************************
                // ***************************** TEST BEGIN *******************************
                // ************************************************************************

                UNITY_BEGIN();

                // Necessary delay for test
                delay(3000);

                // Wait connection request
                // Suspend TASK Controller for queue waiting portMAX_DELAY
                TaskState(state, UNUSED_SUB_POSITION, task_flag::suspended);
                if (param.connectionRequestQueue->Peek(&connection_request, portMAX_DELAY)) {
                    TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);
                    // do ntp sync
                    if (connection_request.do_ntp_sync) {
                        RUN_TEST(test_requested_queue_do_ntp_start);

                        param.connectionRequestQueue->Dequeue(&connection_request, 0);
                        TRACE_VERBOSE_F(F("NTP_STATE_WAIT_NET_EVENT -> NTP_STATE_DO_NTP_SYNC\r\n"));
                        state = NTP_STATE_DO_NTP_SYNC;
                    }
                }
                break;

            case NTP_STATE_DO_NTP_SYNC:
                error = sntpClientInit(&sntpClientContext);
                RUN_TEST(test_init_ntp_cyclone);

                param.systemStatusLock->Take();
                param.system_status->connection.is_ntp_synchronizing = true;
                param.systemStatusLock->Give();

                TRACE_INFO_F(F("%s Resolving ntp server name of %s \r\n"), Thread::GetName().c_str(), param.configuration->ntp_server);

                // Resolve NTP server name
                TaskState(state, 1, task_flag::suspended);  // Or SET Long WDT > 120 sec.
                error = getHostByName(NULL, param.configuration->ntp_server, &ipAddr, 0);
                TaskState(state, 1, task_flag::normal);  // Resume

                RUN_TEST(test_requested_resolve_name_server_ntp);
                // Any error to report?
                if (error) {
                    is_error = true;
                    state = NTP_STATE_END;
                    TRACE_VERBOSE_F(F("NTP_STATE_DO_NTP_SYNC -> NTP_STATE_END\r\n"));

                    TRACE_ERROR_F(F("%s Failed to resolve ntp server name of %s\r\n"), Thread::GetName().c_str(), param.configuration->ntp_server);
                    break;
                }

                TaskWatchDog(SNTP_CLIENT_TIMEOUT_MS);
                error = sntpClientSetTimeout(&sntpClientContext, SNTP_CLIENT_TIMEOUT_MS);
                RUN_TEST(test_requested_resolve_name_server_ntp_timeout_error);

                // Any error to report?
                if (error) {
                    is_error = true;
                    state = NTP_STATE_END;
                    TRACE_VERBOSE_F(F("NTP_STATE_DO_NTP_SYNC -> NTP_STATE_END\r\n"));
                    break;
                }

                // Specify the IP address of the NTP server
                TaskWatchDog(SNTP_CLIENT_TIMEOUT_MS);
                error = sntpClientSetServerAddr(&sntpClientContext, &ipAddr, NTP_PORT);
                // Any error to report?
                if (error) {
                    is_error = true;
                    state = NTP_STATE_END;
                    TRACE_VERBOSE_F(F("NTP_STATE_DO_NTP_SYNC -> NTP_STATE_END\r\n"));
                    break;
                }

                // Retrieve current time from NTP server
                TaskWatchDog(SNTP_CLIENT_TIMEOUT_MS);
                error = sntpClientGetTimestamp(&sntpClientContext, &timestamp);
                RUN_TEST(test_init_ntp_retrieve_time_stamp);
                // Check status code
                if (!error) {
                    // Unix time starts on January 1st, 1970
                    unixTime = timestamp.seconds - NTP_UNIX_EPOCH;

                    // Convert Unix timestamp to date
                    convertUnixTimeToDate(unixTime, &date);

                    // Set DateTime RTC With Semaphore Locked access
                    if (param.rtcLock->Take()) {
                        rtc.setEpoch((uint32_t)unixTime);
                        param.rtcLock->Give();
                    }

                    test_epoch = rtc.getEpoch();

                    RUN_TEST(test_rtc_set_date_time);

                    // Debug message
                    TRACE_INFO_F(F("%s ntp current date/time [ %d ] %s\r\n"), Thread::GetName().c_str(), (uint32_t)unixTime, formatDate(&date, NULL));

                    state = NTP_STATE_END;
                    TRACE_VERBOSE_F(F("NTP_STATE_DO_NTP_SYNC -> NTP_STATE_END\r\n"));
                } else if (error == ERROR_REQUEST_REJECTED) {
                    // Retrieve kiss code
                    TaskWatchDog(SNTP_CLIENT_TIMEOUT_MS);
                    kissCode = sntpClientGetKissCode(&sntpClientContext);

                    TRACE_ERROR_F(F("%s ntp received kiss code: '%c%c%c%c' [ %s ]\r\n"), Thread::GetName().c_str(), (kissCode >> 24) & 0xFF, (kissCode >> 16) & 0xFF, (kissCode >> 8) & 0xFF, kissCode & 0xFF, ERROR_STRING);

                    is_error = true;
                    state = NTP_STATE_END;
                    TRACE_VERBOSE_F(F("NTP_STATE_DO_NTP_SYNC -> NTP_STATE_END\r\n"));
                } else {
                    TRACE_ERROR_F(F("%s Failed to retrieve ntp timestamp [ %s ]\r\n"), Thread::GetName().c_str(), ERROR_STRING);

                    is_error = true;
                    state = NTP_STATE_END;
                    TRACE_VERBOSE_F(F("NTP_STATE_DO_NTP_SYNC -> NTP_STATE_END\r\n"));
                }

                UNITY_END();

                // ************************************************************************
                // ***************************** TEST END *********************************
                // ************************************************************************

                break;

            case NTP_STATE_END:
                // ok
                if (!is_error) {
                    param.systemStatusLock->Take();
                    param.system_status->connection.is_ntp_synchronizing = false;
                    param.system_status->connection.is_ntp_synchronized = true;
                    param.systemStatusLock->Give();

                    sntpClientDeinit(&sntpClientContext);

                    memset(&connection_response, 0, sizeof(connection_response_t));
                    connection_response.done_ntp_synchronized = true;
                    param.connectionResponseQueue->Enqueue(&connection_response, 0);

                    state = NTP_STATE_INIT;
                    TRACE_VERBOSE_F(F("NTP_STATE_END -> NTP_STATE_INIT\r\n"));
                }
                // retry
                else if ((++retry) < NTP_TASK_GENERIC_RETRY) {
                    TaskWatchDog(NTP_TASK_GENERIC_RETRY_DELAY_MS);
                    Delay(Ticks::MsToTicks(NTP_TASK_GENERIC_RETRY_DELAY_MS));
                    TRACE_VERBOSE_F(F("NTP_STATE_END -> NTP_STATE_DO_NTP_SYNC\r\n"));
                    state = NTP_STATE_DO_NTP_SYNC;
                }
                // error
                else {
                    param.systemStatusLock->Take();
                    param.system_status->connection.is_ntp_synchronizing = false;
                    param.system_status->connection.is_ntp_synchronized = false;
                    param.systemStatusLock->Give();

                    sntpClientDeinit(&sntpClientContext);

                    memset(&connection_response, 0, sizeof(connection_response_t));
                    connection_response.error_ntp_synchronized = true;
                    param.connectionResponseQueue->Enqueue(&connection_response, 0);

                    state = NTP_STATE_INIT;
                    TRACE_VERBOSE_F(F("NTP_STATE_END -> NTP_STATE_INIT\r\n"));
                }
                break;
        }

#if (ENABLE_STACK_USAGE)
        TaskMonitorStack();
#endif

        // One step base non blocking switch
        TaskWatchDog(NTP_TASK_WAIT_DELAY_MS);
        Delay(Ticks::MsToTicks(NTP_TASK_WAIT_DELAY_MS));
    }
}

#endif