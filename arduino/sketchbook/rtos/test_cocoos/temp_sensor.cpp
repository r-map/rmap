#include "sensor.h"
#include "temp_sensor.h"
#include <string.h>
#include <cocoos.h>

static uint8_t poll(void);
static uint8_t data(uint8_t *buf, uint8_t size);
static void init_sensor(uint8_t id, Evt_t *event, uint16_t period_ms);
static void next_channel(void);
static void prev_channel(void);

////
static Sensor_Info_t temp_sensor_info = {
  "World Temperature Sensor",
  0,
  0,
  0,
  &poll,
  &data
};

static Sensor_Control_t temp_sensor_control = {
  &init_sensor,
  &next_channel,
  &prev_channel
};

static Sensor_t temp_sensor = {
  temp_sensor_info,
  temp_sensor_control
};
////

////static Sensor_t temp_sensor;
  /* ////
= {
    .info.name = "World Temperature Sensor",
    .info.event = 0,
    .info.id = 0,
    .info.period_ms = 0,
    .info.poll = &poll,
    .info.data = &data,
    .control.init = &init_sensor,
    .control.next_channel = &next_channel,
    .control.prev_channel = &prev_channel
};
    */ ////

static uint8_t channel = 0;
static uint8_t newData = 0;

const char *channels[] = {
                          "New York\t25 degC",
                          "London\t\t18 degC",
                          "Berlin\t\t20 degC",
                          "Moscow\t\t12 degC",
                          "Beijing\t\t28 degC",
                          "Hong Kong\t34 degC",
                          "Sydney\t\t42 degC"
                        };

static uint8_t poll(void) {
  return 1;
}

static uint8_t data(uint8_t *buf, uint8_t size) {

  //// strcpy(buf, channels[channel]);
  strcpy((char *) buf, channels[channel]); ////

  return strlen(channels[channel]);
}

static void init_sensor(uint8_t id, Evt_t *event, uint16_t period_ms) {
  temp_sensor.info.id = id;
  temp_sensor.info.event = event;
  temp_sensor.info.period_ms = period_ms;
}

static void next_channel(void) {
  channel++;
  uint8_t nChannels = sizeof(channels)/sizeof(channels[0]);

  if (channel == nChannels) {
    channel = 0;
  }

  event_ISR_signal(*temp_sensor.info.event);
}

static void prev_channel(void) {
  if (channel == 0) {
    channel = sizeof(channels)/sizeof(channels[0]) -1;
  }
  else {
    channel--;
  }
  event_ISR_signal(*temp_sensor.info.event);
}

Sensor_t *tempSensor_get(void) {
  return &temp_sensor;
}

void tempSensor_service(void) {
  debug("tempSensor_service"); ////
  static uint8_t cnt = 0;
  if (++cnt == 5) {
    newData = 1;
    event_ISR_signal(*temp_sensor.info.event);
    cnt = 0;
  }

}
