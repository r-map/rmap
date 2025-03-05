/**
 ******************************************************************************
 * @file    elaborate_data_task.h
 * @author  Marco Baldinetti <m.baldinetti@digiteco.it>
 * @author  Moreno Gasperini <m.gasperini@digiteco.it>
 * @author  Cristano Souza Paz Fiorio <c.souzapaz@digiteco.it> 
 * @brief   Elaborate data sensor to CAN header file
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

#define TRACE_LEVEL   ELABORATE_DATA_TASK_TRACE_LEVEL
#define LOCAL_TASK_ID ELABORATE_TASK_ID

#include "tasks/elaborate_data_task.h"

using namespace cpp_freertos;

/// @brief Construct the Elaborate Data Task::ElaborateDataTask object
/// @param taskName name of the task
/// @param stackSize size of the stack
/// @param priority priority of the task
/// @param elaborateDataParam parameters for the task
ElaborateDataTask::ElaborateDataTask(const char *taskName, uint16_t stackSize, uint8_t priority, ElaborateDataParam_t elaborateDataParam) : Thread(taskName, stackSize, priority), param(elaborateDataParam) {
    // Start WDT controller and TaskState Flags
    TaskWatchDog(WDT_STARTING_TASK_MS);
    TaskState(ELABORATE_DATA_CREATE, UNUSED_SUB_POSITION, task_flag::normal);

    state = ELABORATE_DATA_INIT;
    Start();
};

#if (ENABLE_STACK_USAGE)
/// @brief local stack Monitor (optional)
void ElaborateDataTask::TaskMonitorStack() {
    uint16_t stackUsage = (uint16_t)uxTaskGetStackHighWaterMark(NULL);
    if ((stackUsage) && (stackUsage < param.system_status->tasks[LOCAL_TASK_ID].stack)) {
        param.systemStatusLock->Take();
        param.system_status->tasks[LOCAL_TASK_ID].stack = stackUsage;
        param.systemStatusLock->Give();
    }
}
#endif

/// @brief local watchDog and Sleep flag Task (optional)
/// @param millis_standby time in ms to perfor check of WDT. If longer than WDT Reset, WDT is temporanly suspend
void ElaborateDataTask::TaskWatchDog(uint32_t millis_standby) {
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
void ElaborateDataTask::TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation) {
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

/// @brief RUN Task
void ElaborateDataTask::Run() {
    // Queue for data
    elaborate_data_t edata;
    request_data_t request_data;
    // System message data queue structured
    system_message_t system_message;

    bufferReset<maintenance_t, uint16_t, bool>(&maintenance_samples, SAMPLES_COUNT_MAX);
    bufferReset<sample_t, uint16_t, rmapdata_t>(&leaf_samples, SAMPLES_COUNT_MAX);

// Start Running Monitor and First WDT normal state
#if (ENABLE_STACK_USAGE)
    TaskMonitorStack();
#endif
    TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

#if defined(USE_SIMULATOR) && defined(INIT_SIMULATOR)
    for (uint16_t iInit = 0; iInit < 900; iInit++) {
        if (iInit < 810)
            edata.value = true;
        else
            edata.value = false;
        TRACE_VERBOSE_F(F("Leaf: %d%%\r\n"), getLeafPercentage(edata.value));
        addValue<maintenance_t, uint16_t, bool>(&maintenance_samples, SAMPLES_COUNT_MAX, false);
        addValue<sample_t, uint16_t, rmapdata_t>(&leaf_samples, SAMPLES_COUNT_MAX, edata.value);
    }
    make_report(request_data.is_init, 900, 60);
#endif

    while (true) {
        // ********* SYSTEM QUEUE MESSAGE ***********
        // enqueud system message from caller task
        if (!param.systemMessageQueue->IsEmpty()) {
            // Read queue in test mode
            if (param.systemMessageQueue->Peek(&system_message, 0)) {
                // Its request addressed into ALL TASK... -> no pull (only SUPERVISOR or exernal gestor)
                if (system_message.task_dest == ALL_TASK_ID) {
                    // Pull && elaborate command,
                    if (system_message.command.do_sleep) {
                        // Enter sleep module OK and update WDT
                        TaskWatchDog(ELABORATE_TASK_SLEEP_DELAY_MS);
                        TaskState(state, UNUSED_SUB_POSITION, task_flag::sleepy);
                        Delay(Ticks::MsToTicks(ELABORATE_TASK_SLEEP_DELAY_MS));
                        TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);
                    }
                }
            }
        }

        // enqueud from leaf sensors task (populate data)
        if (!param.elaborateDataQueue->IsEmpty()) {
            if (param.elaborateDataQueue->Peek(&edata, 0)) {
                param.elaborateDataQueue->Dequeue(&edata);
                switch (edata.index) {
                    case LEAF_INDEX:
// Data Simulator
#ifdef USE_SIMULATOR
                        edata.value = true;
#endif
                        TRACE_VERBOSE_F(F("Leaf: %d%%\r\n"), getLeafPercentage(edata.value));
                        addValue<maintenance_t, uint16_t, bool>(&maintenance_samples, SAMPLES_COUNT_MAX, param.system_status->flags.is_maintenance);
                        addValue<sample_t, uint16_t, rmapdata_t>(&leaf_samples, SAMPLES_COUNT_MAX, edata.value);
                        break;
                }
            }
        }

        // enqueued from can task (get data, start command...)
        if (!param.requestDataQueue->IsEmpty()) {
            if (param.requestDataQueue->Peek(&request_data, 0)) {
                // send request to elaborate task (all data is present verified on elaborate_task)
                param.requestDataQueue->Dequeue(&request_data);
                make_report(request_data.is_init, request_data.report_time_s, request_data.observation_time_s);
                param.reportDataQueue->Enqueue(&report);
            }
        }

#if (ENABLE_STACK_USAGE)
        TaskMonitorStack();
#endif

        // Local TaskWatchDog update;
        TaskWatchDog(ELABORATE_TASK_WAIT_DELAY_MS);

        DelayUntil(Ticks::MsToTicks(ELABORATE_TASK_WAIT_DELAY_MS));
    }
}

uint8_t ElaborateDataTask::getLeafPercentage(bool leaf) {
    if (leaf) return 100;
    else return 0;
}


/// @brief Check data in and perform calculate of Optional Quality value
/// @param leaf real value readed from sensor
/// @return value uint_8 percent data quality value
uint8_t ElaborateDataTask::checkLeaf(rmapdata_t leaf) {
    // Optional check quality data function
    uint8_t quality = 100;
    return quality;
}

/// @brief Create a report from buffered sample
/// @param is_init Bool optional backup ptr_wr calc
/// @param report_time_s time of report
/// @param observation_time_s time to make an observation
void ElaborateDataTask::make_report(bool is_init, uint16_t report_time_s, uint8_t observation_time_s) {
    // Generic and shared var
    bool measures_maintenance = false;  // Maintenance mode?
    float valid_data_calc_perc;         // Shared calculate valid % of measure (Data Valid or not?)
    bool is_observation = false;        // Is an observation (when calculate is requested)
    uint16_t n_sample = 0;              // Sample elaboration number... (incremented on calc development)

    // Elaborate suffix description: _s(sample) _o(observation) _t(total) [Optional for debug]

    // Elaboration leaf
    rmapdata_t leaf_s = 0;
    uint16_t count_leaf_s = 0;
    uint16_t count_leaf_o = 0;
    float avg_leaf_quality_s = 0;
    float avg_leaf_quality_o = 0;
#if TRACE_LEVEL >= TRACE_LEVEL_DEBUG
    uint16_t valid_count_leaf_t = 0;
#endif
    uint16_t valid_count_leaf_s = 0;
    uint16_t total_count_leaf_s = 0;
    uint16_t valid_count_leaf_o = 0;

    // Elaboration timings calculation
    uint16_t report_sample_count = round((report_time_s * 1.0) / (param.configuration->sensor_acquisition_delay_ms / 1000.0));
    uint16_t observation_sample_count = round((observation_time_s * 1.0) / (param.configuration->sensor_acquisition_delay_ms / 1000.0));
    uint16_t report_observations_count = 0;
    if (report_time_s && observation_sample_count) report_observations_count = report_sample_count / observation_sample_count;

    // Request to calculate is correct? Trace request
    if (report_time_s == 0) {
        // Request an direct sample value for istant measure
        TRACE_INFO_F(F("Elaborate: Requested an istant value\r\n"));
    } else {
        TRACE_INFO_F(F("Elaborate: Requested an report on %d seconds\r\n"), report_time_s);
        TRACE_DEBUG_F(F("-> %d samples counts need for report\r\n"), report_sample_count);
        TRACE_DEBUG_F(F("-> %d samples counts need for observation\r\n"), observation_sample_count);
        TRACE_DEBUG_F(F("-> %d observation counts need for report\r\n"), report_observations_count);
        TRACE_DEBUG_F(F("-> %d available leaf samples count\r\n"), leaf_samples.count);
    }

    // Default value to RMAP Limit error value
    report.count = RMAPDATA_MAX;
    report.quality = RMAPDATA_MAX;

    // Ptr for maintenance
    bufferPtrResetBack<maintenance_t, uint16_t>(&maintenance_samples, SAMPLES_COUNT_MAX);
    // Ptr for value sample
    bufferPtrResetBack<sample_t, uint16_t>(&leaf_samples, SAMPLES_COUNT_MAX);

    // align all sensor's data to last common acquired sample
    uint16_t samples_count = leaf_samples.count;

    // it's a simple istant or report request?
    if (report_time_s == 0) {
        // Make last data value to Get Istant show value
        report.count = bufferReadBack<sample_t, uint16_t, rmapdata_t>(&leaf_samples, SAMPLES_COUNT_MAX);
    } else {
        for (uint16_t i = 0; i < samples_count; i++) {
            // End of Sample in calculation (Completed with the request... Exit on Full Buffer ReadBack executed)
            if (n_sample >= report_sample_count) break;
            // Get base operation for any record...
            n_sample++;  // Elaborate next sample... (Initzialize with 0, Sample 1 is first. Exit UP if buffer completed)
            // Check if is an observation
            is_observation = (n_sample % observation_sample_count) == 0;
            // Is Maintenance mode? (Excluding measure from elaboration value)
            // Maintenance is sytemic value for all measure (always pushed into module for excuding value with maintenance)
            measures_maintenance = bufferReadBack<maintenance_t, uint16_t, bool>(&maintenance_samples, SAMPLES_COUNT_MAX);

            // ***************************************************************************************************
            // ************* GET SAMPLE VALUE DATA FROM AND CREATE OBSERVATION VALUES FOR TYPE SENSOR ************
            // ***************************************************************************************************

            leaf_s = bufferReadBack<sample_t, uint16_t, rmapdata_t>(&leaf_samples, SAMPLES_COUNT_MAX);
            total_count_leaf_s++;
            avg_leaf_quality_s += (float)(((float)checkLeaf(leaf_s) - avg_leaf_quality_s) / total_count_leaf_s);
            if ((ISVALID_RMAPDATA(leaf_s)) && !measures_maintenance) {
                valid_count_leaf_s++;
#if TRACE_LEVEL >= TRACE_LEVEL_DEBUG
                valid_count_leaf_t++;
#endif
                if (leaf_s) count_leaf_s++;
            }

            // ***************************************************************************************************
            // ************* ELABORATE OBSERVATION VALUES FOR TYPE SENSOR FOR PREPARE REPORT RESPONSE ************
            // ***************************************************************************************************
            if (is_observation) {
                // leaf, sufficient number of valid samples?
                valid_data_calc_perc = (float)(valid_count_leaf_s) / (float)(total_count_leaf_s) * 100.0;
                if (valid_data_calc_perc >= SAMPLE_ERROR_PERCENTAGE_MIN) {
                    valid_count_leaf_o++;
                    if (count_leaf_s >= (total_count_leaf_s / 2)) count_leaf_o++;
                    avg_leaf_quality_o += (avg_leaf_quality_s - avg_leaf_quality_o) / valid_count_leaf_o;
                }
                // Reset Buffer sample for calculate next observation
                avg_leaf_quality_s = 0;
                count_leaf_s = 0;
                valid_count_leaf_s = 0;
                total_count_leaf_s = 0;
            }
        }

        // ***************************************************************************************************
        // ******* GENERATE REPORT RESPONSE WITH ALL DATA AVAIABLE AND VALID WITH EXPECETD OBSERVATION *******
        // ***************************************************************************************************

        // Elaboration final
        valid_data_calc_perc = (float)(valid_count_leaf_o) / (float)(report_observations_count) * 100.0;
        TRACE_DEBUG_F(F("-> %d leaf sample error (%d%%)\r\n"), (n_sample - valid_count_leaf_t), (uint8_t)(((float)n_sample - (float)valid_count_leaf_t) / (float)n_sample * 100.0));
        TRACE_DEBUG_F(F("-> %d leaf observation available (%d%%)\r\n"), valid_count_leaf_o, (uint8_t)valid_data_calc_perc);
        if (valid_data_calc_perc >= OBSERVATION_ERROR_PERCENTAGE_MIN) {
            report.count = (rmapdata_t)((float)(count_leaf_o) / (float)(report_observations_count) * (float)(report_sample_count) * (float)(param.configuration->sensor_acquisition_delay_ms / 1000.0));
            report.quality = (rmapdata_t)avg_leaf_quality_o;
        }

        // Trace report final
        TRACE_INFO_F(F("--> leaf report\t%d\t%d\r\n"), (rmapdata_t)report.count, (rmapdata_t)report.quality);
    }
}

template <typename buffer_g, typename length_v, typename value_v>
value_v bufferRead(buffer_g *buffer, length_v length) {
    value_v value = *buffer->read_ptr;

    if (buffer->read_ptr == buffer->values + length - 1) {
        buffer->read_ptr = buffer->values;
    } else
        buffer->read_ptr++;

    return value;
}

template <typename buffer_g, typename length_v, typename value_v>
value_v bufferReadBack(buffer_g *buffer, length_v length) {
    value_v value = *buffer->read_ptr;

    if (buffer->read_ptr == buffer->values) {
        buffer->read_ptr = buffer->values + length - 1;
    } else
        buffer->read_ptr--;

    return value;
}

template <typename buffer_g, typename value_v>
void bufferWrite(buffer_g *buffer, value_v value) {
    *buffer->write_ptr = value;
}

template <typename buffer_g>
void bufferPtrReset(buffer_g *buffer) {
    buffer->read_ptr = buffer->values;
}

template <typename buffer_g, typename length_v>
void bufferPtrResetBack(buffer_g *buffer, length_v length) {
    if (buffer->write_ptr == buffer->values) {
        buffer->read_ptr = buffer->values + length - 1;
    } else
        buffer->read_ptr = buffer->write_ptr - 1;
}

template <typename buffer_g, typename length_v>
void incrementBuffer(buffer_g *buffer, length_v length) {
    if (buffer->count < length) {
        buffer->count++;
    }

    if (buffer->write_ptr + 1 < buffer->values + length) {
        buffer->write_ptr++;
    } else
        buffer->write_ptr = buffer->values;
}

template <typename buffer_g, typename length_v, typename value_v>
void bufferReset(buffer_g *buffer, length_v length) {
    memset(buffer->values, 0xFF, length * sizeof(value_v));
    buffer->count = 0;
    buffer->read_ptr = buffer->values;
    buffer->write_ptr = buffer->values;
}

template <typename buffer_g, typename length_v, typename value_v>
void addValue(buffer_g *buffer, length_v length, value_v value) {
    *buffer->write_ptr = (value_v)value;
    incrementBuffer<buffer_g, length_v>(buffer, length);
}
