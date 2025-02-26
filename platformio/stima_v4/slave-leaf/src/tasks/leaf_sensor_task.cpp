/**
 ******************************************************************************
 * @file    leaf_sensor_task.cpp
 * @author  Marco Baldinetti <m.baldinetti@digiteco.it>
 * @author  Moreno Gasperini <m.gasperini@digiteco.it>
 * @author  Cristano Souza Paz Fiorio <c.souzapaz@digiteco.it> 
 * @brief   Module sensor source file
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

#define TRACE_LEVEL   LEAF_SENSOR_TASK_TRACE_LEVEL
#define LOCAL_TASK_ID SENSOR_TASK_ID

#include "tasks/leaf_sensor_task.h"

#if (MODULE_TYPE == STIMA_MODULE_TYPE_LEAF)

using namespace cpp_freertos;

/// @brief Constructor for the sensor Task
/// @param taskName name of the task
/// @param stackSize size of the stack
/// @param priority priority of the task
/// @param leafSensorParam local parameters for the task
LeafSensorTask::LeafSensorTask(const char *taskName, uint16_t stackSize, uint8_t priority, LeafSensorParam_t leafSensorParam) : Thread(taskName, stackSize, priority), param(leafSensorParam) {
    // Start WDT controller and TaskState Flags
    TaskWatchDog(WDT_STARTING_TASK_MS);
    TaskState(SENSOR_STATE_CREATE, UNUSED_SUB_POSITION, task_flag::normal);

    // Set analog resolution
    analogReadResolution(ADC_RESOLUTION);

    state = SENSOR_STATE_WAIT_CFG;
    Start();
};

#if (ENABLE_STACK_USAGE)
/// @brief local stack Monitor (optional)
void LeafSensorTask::TaskMonitorStack() {
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
void LeafSensorTask::TaskWatchDog(uint32_t millis_standby) {
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
void LeafSensorTask::TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation) {
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
void LeafSensorTask::Run() {
    rmapdata_t values_readed_from_sensor[VALUES_TO_READ_FROM_SENSOR_COUNT];
    elaborate_data_t edata;
    // Request response for system queue Task controlled...
    system_message_t system_message;

    bool is_adc_overflow;
    bool is_adc_error;
    uint8_t perc_error_adc;
    uint8_t adc_channel;
    float value;

// Start Running Monitor and First WDT normal state
#if (ENABLE_STACK_USAGE)
    TaskMonitorStack();
#endif
    TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

// Using powered rest mode (comparator and optional sesnsor chanel power)
#if (LEAF_TASK_LOW_POWER_ENABLED)
    powerOff();
#else
    digitalWrite(PIN_EN_5VA, HIGH);  // Fixed powered Analog Comparator (xIAN) Get value
#endif

// Fixed Chanel Powered used define (Sensor do not switching power, always on from define)
#if (LEAF_FIXED_POWER_CHANEL_0)
    digitalWrite(PIN_OUT0, HIGH);  // Enable relative fixed Output Chanel alim
#endif
#if (LEAF_FIXED_POWER_CHANEL_1)
    digitalWrite(PIN_OUT1, HIGH);  // Enable relative fixed Output Chanel alim
#endif
#if (LEAF_FIXED_POWER_CHANEL_2)
    digitalWrite(PIN_OUT2, HIGH);  // Enable relative fixed Output Chanel alim
#endif
#if (LEAF_FIXED_POWER_CHANEL_3)
    digitalWrite(PIN_OUT3, HIGH);  // Enable relative fixed Output Chanel alim
#endif

    while (true) {
        switch (state) {
            case SENSOR_STATE_WAIT_CFG:

                // Check if configuration is done loaded
                if (param.system_status->flags.is_cfg_loaded) {
                    // Select read chanel first from config
                    for (uint8_t idx = 0; idx < MAX_ADC_CHANELS; idx++) {
                        if (param.configuration->sensors[idx].is_active) {
                            // Select first active chanel selected (only one is used on this module)
                            adc_channel = idx;
                            break;
                        }
                    }
                    TRACE_VERBOSE_F(F("WAIT -> INIT\r\n"));
                    state = SENSOR_STATE_INIT;
                }
                // do something else with non-blocking wait ....
                TaskWatchDog(LEAF_TASK_WAIT_DELAY_MS);
                Delay(Ticks::MsToTicks(LEAF_TASK_WAIT_DELAY_MS));
                break;

            case SENSOR_STATE_INIT:

                TRACE_INFO_F(F("Initializing sensors...\r\n"));
#if (LEAF_TASK_LOW_POWER_ENABLED)  // Auto power chanel with switching mode and power time waiting...
                powerOn(adc_channel);
#endif
                resetADCData(adc_channel);
                state = SENSOR_STATE_SET;
                TRACE_INFO_F(F("SENSOR_STATE_INIT --> SENSOR_STATE_SET\r\n"));
                break;

            case SENSOR_STATE_SET:

                // Add sample ADC value to data
                if (addADCData(adc_channel) >= SAMPLES_REPETED_ADC) {
                    // Read real value
                    value = getADCData(adc_channel, &perc_error_adc);
                    // ?Error ADC reading < MIN_PERRCENT_READING_VALUE_OK
                    is_adc_error = (perc_error_adc < ADC_ERROR_PERCENTAGE_MIN);
                    state = SENSOR_STATE_EVALUATE;
                    break;
                }
                break;

            case SENSOR_STATE_EVALUATE:

                // With ADC Error nothing to do...
                if (is_adc_error) {
                    // Error code data
                    TRACE_ERROR_F(F("Sensor analog: Error reading ADC\r\n"));
                    value = (float)UINT16_MAX;
                } else {
                    // Gain - offset to ADC to real value, and connvert to scale used (mV for LEAF for each ADC Type method used)
                    value = getAdcCalibratedValue(value, param.configuration->sensors[adc_channel].adc_offset, param.configuration->sensors[adc_channel].adc_gain);
                    value = getAdcAnalogValue(value, param.configuration->sensors[adc_channel].adc_type);
                    TRACE_DEBUG_F(F("Sensor analog value %d (mV)\r\n"), (uint16_t)round(value * 1000.0));
                    // Read value into U.M. Real Leaf (Sample value)
                    value = getLeaf(value, param.configuration->sensors[adc_channel].analog_min, param.configuration->sensors[adc_channel].analog_max, &is_adc_overflow);
                }

                // Inform system state if ADC error event ( reading measure % error < MIN_VALID_PERCENTAGE )
                if (param.system_status->events.is_adc_unit_error != (perc_error_adc < ADC_ERROR_PERCENTAGE_MIN)) {
                    param.systemStatusLock->Take();
                    param.system_status->events.is_adc_unit_error = (perc_error_adc < ADC_ERROR_PERCENTAGE_MIN);
                    param.systemStatusLock->Give();
                }
                // Inform system state if ADC oveflow analog sensor limits
                if ((param.system_status->events.is_adc_unit_overflow != is_adc_overflow) ||
                    (param.system_status->events.is_adc_unit_error != is_adc_error)) {
                    param.systemStatusLock->Take();
                    param.system_status->events.is_adc_unit_overflow = is_adc_overflow;
                    param.system_status->events.is_adc_unit_error = is_adc_error;
                    param.systemStatusLock->Give();
                }

                state = SENSOR_STATE_READ;
                TRACE_INFO_F(F("SENSOR_STATE_EVALUATE --> SENSOR_STATE_READ\r\n"));
                break;

            case SENSOR_STATE_READ:

                edata.value = getBooleanLeaf(value);
                edata.index = LEAF_INDEX;

                param.elaborateDataQueue->Enqueue(&edata, Ticks::MsToTicks(WAIT_QUEUE_REQUEST_PUSHDATA_MS));
                TRACE_INFO_F(F("SENSOR_STATE_READ --> SENSOR_STATE_END\r\n"));
                state = SENSOR_STATE_END;
                break;

            case SENSOR_STATE_END:

#if (LEAF_TASK_LOW_POWER_ENABLED)
                powerOff();
#endif

#if (ENABLE_STACK_USAGE)
                TaskMonitorStack();
#endif

                // Local TaskWatchDog update and Sleep Activate before Next Read
                TaskWatchDog(param.configuration->sensor_acquisition_delay_ms);
                TaskState(state, UNUSED_SUB_POSITION, task_flag::sleepy);
                DelayUntil(Ticks::MsToTicks(param.configuration->sensor_acquisition_delay_ms));
                TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);
                state = SENSOR_STATE_INIT;
                break;
        }  // End Of Switch
    }
}

/// @brief Power ON Method (rest power mode)
/// @param chanel_out chanel To be powered (Fixed or switching depending from define into header_file)
void LeafSensorTask::powerOn(uint8_t chanel_out) {
    if (!is_power_on) {
        digitalWrite(PIN_EN_5VA, HIGH);  // Enable Analog Comparator (xIAN)
// Switching powered chanel mode
#if (LEAF_TASK_SWITCH_POWER_ENABLED)
        if (chanel_out == 0) digitalWrite(PIN_OUT0, HIGH);  // Enable relative Output Chanel alim
        if (chanel_out == 1) digitalWrite(PIN_OUT1, HIGH);  // Enable relative Output Chanel alim
        if (chanel_out == 2) digitalWrite(PIN_OUT2, HIGH);  // Enable relative Output Chanel alim
        if (chanel_out == 3) digitalWrite(PIN_OUT3, HIGH);  // Enable relative Output Chanel alim
#endif
        // WDT
        TaskWatchDog(LEAF_TASK_POWER_ON_WAIT_DELAY_MS);
        Delay(Ticks::MsToTicks(LEAF_TASK_POWER_ON_WAIT_DELAY_MS));
        is_power_on = true;
    }
}

/// @brief Power OFF Method (rest power mode)
void LeafSensorTask::powerOff() {
    digitalWrite(PIN_EN_5VA, LOW);  // Disable Analog Comparator (xIAN)
#if (LEAF_TASK_SWITCH_POWER_ALIM_SENS)
    digitalWrite(PIN_OUT0, LOW);  // Disable Output Chanel alim
    digitalWrite(PIN_OUT1, LOW);  // Disable Output Chanel alim
    digitalWrite(PIN_OUT2, LOW);  // Disable Output Chanel alim
    digitalWrite(PIN_OUT3, LOW);  // Disable Output Chanel alim
#endif
    is_power_on = false;
}

/// @brief Add Data ADC to counter
/// @param chanel_out Chanel to be getted
uint8_t LeafSensorTask::addADCData(uint8_t chanel_out) {
    int32_t analogReadVal;
    adc_in_count[chanel_out]++;
    switch (chanel_out) {
        case 0:
            analogReadVal = analogRead(PIN_ANALOG_01);
            break;
        case 1:
            analogReadVal = analogRead(PIN_ANALOG_02);
            break;
        case 2:
            analogReadVal = analogRead(PIN_ANALOG_03);
            break;
        case 3:
            analogReadVal = analogRead(PIN_ANALOG_04);
            break;
    }
    // Add data only if not an error (count error as uint 32 FFFFFFFF = -1, < 0)
    if (analogReadVal >= 0) {
        adc_in[chanel_out] += (uint32_t)analogReadVal;
    } else {
        adc_err_count[chanel_out]++;
    }
    return adc_in_count[chanel_out];
}

/// @brief Reset ADC Counter
/// @param chanel_out Chanel to be resetted value
void LeafSensorTask::resetADCData(uint8_t chanel_out) {
    adc_in_count[chanel_out] = 0;
    adc_err_count[chanel_out] = 0;
    adc_in[chanel_out] = 0;
}

/// @brief Get real ADC Data
/// @param chanel_out Chanel to be resetted value
/// @param quality_data Return quality perc of good measure in current cycle reading
/// @return ADC Value (float conversion)
float LeafSensorTask::getADCData(uint8_t chanel_out, uint8_t *quality_data) {
    *quality_data = 0;
    // All measure are error?
    if (adc_in_count[chanel_out] == adc_err_count[chanel_out]) return 0;
    // Get quality of repeted measure
    if (adc_err_count[chanel_out] == 0) {
        *quality_data = 100;
    } else {
        *quality_data = (uint8_t)(100.0 - (((float)adc_err_count[chanel_out] / (float)adc_in_count[chanel_out]) * 100.0));
    }
    // Return measure, eliminating the wrong measurements
    return adc_in[chanel_out] / (adc_in_count[chanel_out] - adc_err_count[chanel_out]);
}

/// @brief Get System VREF internal temperature
/// @return ambient external CPU temperature value
int32_t LeafSensorTask::getVrefTemp(void) {
    int32_t vRefVoltage = (__LL_ADC_CALC_VREFANALOG_VOLTAGE(analogRead(AVREF), LL_ADC_RESOLUTION));
    return (__LL_ADC_CALC_TEMPERATURE(vRefVoltage, analogRead(ATEMP), LL_ADC_RESOLUTION));
}

/// @brief Get ADC value with param optional calibration
/// @param adc_value adc in value
/// @param offset offset adjust value
/// @param gain gain adjust value
/// @return ADC value calibrated and checked with input area validation limit
float LeafSensorTask::getAdcCalibratedValue(float adc_value, float offset, float gain) {
    float value = (float)UINT16_MAX;

    if (!isnan(adc_value) && (adc_value >= ADC_MIN) && (adc_value <= ADC_MAX)) {
        value = adc_value;
        value += offset;
        value *= gain;
        if (value <= ADC_MIN) value = ADC_MIN;
        if (value >= ADC_MAX) value = ADC_MAX;
    }

    return value;
}

/// @brief Get analog value converted from ADC with type input selected
/// @param adc_value value read from adc
/// @param adc_type ADC_Mode as type of input value selected type
/// @return ADC value to real analog value
float LeafSensorTask::getAdcAnalogValue(float adc_value, Adc_Mode adc_type) {
    float min, max;
    float value = (float)UINT16_MAX;

    switch (adc_type) {
        case Adc_Mode::mVolt:
            min = (float)(ADC_VOLTAGE_MIN_MV);
            max = (float)(ADC_VOLTAGE_MAX_MV);
            /* code */
            break;

        case Adc_Mode::Volt:
            min = (float)(ADC_VOLTAGE_MIN_V);
            max = (float)(ADC_VOLTAGE_MAX_V);
            /* code */
            break;

        case Adc_Mode::mA:
            min = (float)(ADC_CURRENT_MIN_MA);
            max = (float)(ADC_CURRENT_MAX_MA);
            /* code */
            break;

        default:
            break;
    }

    if (!isnan(adc_value)) {
        value = adc_value;
        value *= (((max - min) / (float)(ADC_MAX)));
        value += min;
    }

    return value;
}

/// @brief Get real data Leaf
/// @param adc_value in adc value calibrated
/// @param adc_voltage_min VMIN Ref range of sensor
/// @param adc_voltage_max VMAX Ref range of sensor
/// @param adc_overflow bool if ADC get an overflow range value (setted to true)
/// @return Real scaled data Leaf from analog adc calibrated value
float LeafSensorTask::getLeaf(float adc_value, float adc_voltage_min, float adc_voltage_max, bool *adc_overflow) {
    float value = adc_value;
    *adc_overflow = false;

    if ((value < LEAF_ERROR_VOLTAGE_MIN) || (value > LEAF_ERROR_VOLTAGE_MAX)) {
        *adc_overflow = true;
        value = UINT16_MAX;
    } else {
        value = ((value - adc_voltage_min) / (adc_voltage_max - adc_voltage_min) * LEAF_MAX);

        if (value <= LEAF_ERROR_MIN) value = LEAF_MIN;
        if (value >= LEAF_ERROR_MAX) value = LEAF_MAX;
    }

    // If the value is less than sensibility param and the value is greater than zero => set value to zero
    if ((value >= LEAF_MIN) && (value <= LEAF_SENSIBILITY)) {
        value = LEAF_MIN;
    }

    return round(value);
}

boolean LeafSensorTask::getBooleanLeaf(float value) {
    if (value >= LEAF_VOLTAGE_THRESHOLD) return true;
    return false;
}

#endif
