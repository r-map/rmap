#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <stdint.h>
#include "cocoos.h"

#ifdef __cplusplus ////
extern "C" {
#endif ////

#define ESC 27
#define CR  0xd
#define CLEAR_LINE "[2K"
#define MOVE_DOWN "[1B"
#define MOVE_UP(n) "["#n"A"

// Message signals
#define TEMP_DATA 32
#define GYRO_DATA 33
#define DISPLAY_MSG 34

/**
 * Message type for display task
 * The value of super.signal indicates which type of data,
 * (temp_sensor or gyro data in this case)
 */
typedef struct {
  Msg_t super;
  const char *data;
} DisplayMsg_t;

typedef struct {
  void (*update)(void);
  void (*updateData)(uint8_t id, const char *data);
} Display_t;

Display_t *display_get(void);
void display_init(void);

extern DisplayMsg_t displayMessages[10];

#ifdef __cplusplus ////
}
#endif ////

#endif /* DISPLAY_H_ */
