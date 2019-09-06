#ifndef SENSOR_H_
#define SENSOR_H_

#ifdef __cplusplus ////
extern "C" {
#endif ////

/*
 * Sensor interface
 * Used by tasks to access a sensor
 * An example could be an i2c connected sensor. The sensor driver
 * could signal the event in the tx interrupt when new data is available,
 * or return 1 in the poll() function.
 */
 
void sensor_setup(); ////
void sensor_prepare(); ////
void sensor_get(); ////
  
#ifdef __cplusplus ////
}
#endif ////

#endif /* SENSOR_H_ */
