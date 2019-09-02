#ifndef GYRO_SENSOR_H_
#define GYRO_SENSOR_H_

#include "sensor.h"

#ifdef __cplusplus ////
extern "C" {
#endif ////

Sensor_t *gyroSensor_get(void);
void gyroSensor_service(void);

#ifdef __cplusplus ////
}
#endif ////

#endif /* GYRO_SENSOR_H_ */
