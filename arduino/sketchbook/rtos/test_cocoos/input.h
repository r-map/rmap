#ifndef INPUT_H_
#define INPUT_H_

#include "sensor.h"

#ifdef __cplusplus ////
extern "C" {
#endif ////

  bool input_available();
  uint8_t input_getcommand(void);

#ifdef __cplusplus ////
}
#endif ////

#endif /* INPUT_H_ */
