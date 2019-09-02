#ifndef TEMP_SENSOR_H_
#define TEMP_SENSOR_H_

#include "sensor.h"

#ifdef __cplusplus ////
extern "C" {
#endif ////

Sensor_t *tempSensor_get(void);
void tempSensor_service(void);

#ifdef __cplusplus ////
}
#endif ////

#endif /* TEMP_SENSOR_H_ */
