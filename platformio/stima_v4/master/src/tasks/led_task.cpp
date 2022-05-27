#include "tasks/led_task.h"

using namespace cpp_freertos;

LedTask::LedTask(uint16_t stackSize, uint8_t priority, LedParam_t ledParam) : Thread(stackSize, priority),
LedParam(ledParam)
{
  Start();
};

void LedTask::Run() {
  pinMode(LedParam.led, OUTPUT);
  while (true) {
    digitalWrite(LedParam.led, HIGH);
    Delay(Ticks::MsToTicks(LedParam.onDelayMs));
    digitalWrite(LedParam.led, LOW);
    Delay(Ticks::MsToTicks(LedParam.offDelayMs));
  }
}
