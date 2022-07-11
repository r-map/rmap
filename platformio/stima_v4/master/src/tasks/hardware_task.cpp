// #define TRACE_LEVEL HARDWARE_TASK_TRACE_LEVEL
//
// #include "tasks/hardware_task.h"
//
// using namespace cpp_freertos;
//
// HardwareTask::HardwareTask(const char *taskName, uint16_t stackSize, uint8_t priority, HardwareParam_t hardwareParam) : Thread(taskName, stackSize, priority), HardwareParam(hardwareParam) {
//   Start();
// };
//
// void HardwareTask::Run() {
//   while (true) {
//     enc28j60IrqHandler(HardwareParam.interface);
//     Delay(Ticks::MsToTicks(HardwareParam.tickHandlerMs));
//   }
// }
