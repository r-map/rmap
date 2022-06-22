#define TRACE_LEVEL LED_TASK_TRACE_LEVEL

#include "tasks/led_task.h"

using namespace cpp_freertos;

LedTask::LedTask(const char *taskName, uint16_t stackSize, uint8_t priority, LedParam_t ledParam) : Thread(taskName, stackSize, priority), LedParam(ledParam) {
  Start();
};

void LedTask::Run() {
  pinMode(LedParam.led, OUTPUT);
  while (true) {
    digitalWrite(LedParam.led, HIGH);
    DelayUntil(Ticks::MsToTicks(LedParam.onDelayMs));
    digitalWrite(LedParam.led, LOW);
    DelayUntil(Ticks::MsToTicks(LedParam.offDelayMs));
  }
}
