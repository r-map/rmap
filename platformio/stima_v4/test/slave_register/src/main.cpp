#define TRACE_LEVEL STIMA_TRACE_LEVEL

#include "main.h"

void setup() {

  // Semaphore, Queue && Param Config for TASK
#if (ENABLE_I2C1)
  static BinarySemaphore *wireLock;       // Access I2C internal EEprom, Accelerometer
#endif

  static BinarySemaphore *rtcLock;        // RTC (Access lock)

  // System semaphore
  static BinarySemaphore *systemStatusLock;   // Access System status
  static BinarySemaphore *registerAccessLock; // Access Register Cyphal Specifications

  // System and status configuration struct
  static system_status_t system_status = {0};

  // Initializing basic hardware's configuration
  SetupSystemPeripheral();
  init_debug(SERIAL_DEBUG_BAUD_RATE);
  init_wire();

  // Init SystemStatus Parameter !=0 ... For Check control Value
  // Task check init data (Wdt = True, TaskStack Max, TaskReady = False)
  // TOTAL_INFO_TASK Number of Task checked
#if (ENABLE_STACK_USAGE)
  for(uint8_t id = 0; id < TOTAL_INFO_TASK; id++)
  {
    system_status.tasks[id].stack = 0xFFFFu;
  }
#endif
  // Disable all Task before INIT
  for(uint8_t id = 0; id < TOTAL_INFO_TASK; id++)
  {
    system_status.tasks[id].state = task_flag::suspended;
  }

#if (ENABLE_WDT)
  // Init the watchdog timer WDT_TIMEOUT_BASE_US mseconds timeout (only control system)
  // Wdt Task Reset the value after All Task reset property single Flag
  if(IWatchdog.isReset()) {
    delay(50);
    TRACE_INFO_F(F("\r\n\r\nALERT: Verified an WDT Reset !!!\r\n\r\n"));
    IWatchdog.clearReset();
  }
  IWatchdog.begin(WDT_TIMEOUT_BASE_US);
#endif

  // Hardware Semaphore
#if (ENABLE_I2C1)
  wireLock = new BinarySemaphore(true);
#endif
  // Software Semaphore
  systemStatusLock = new BinarySemaphore(true);
  registerAccessLock = new BinarySemaphore(true);

  TRACE_INFO_F(F("Initialization HW Base done\r\n"));

  // ***************************************************************
  //                  Setup parameter for Task
  // ***************************************************************

#if (ENABLE_I2C1)
  // Load Info from E2 boot_check flag and send to Config
  static EEprom  memEprom(&Wire, wireLock);
  bootloader_t boot_check = {0};
  #if INIT_PARAMETER
  boot_check.app_executed_ok = true;
  memEprom.Write(BOOT_LOADER_STRUCT_ADDR, (uint8_t*) &boot_check, sizeof(boot_check));
  #else
  memEprom.Read(BOOT_LOADER_STRUCT_ADDR, (uint8_t*) &boot_check, sizeof(boot_check));
  #endif
  // Optional send other InfoParm Boot (Uploaded, rollback, error fail ecc.. to config)
#endif

  // Register
  static uavcan_register_Value_1_0 val = {0};

  // Reset Factory register value
  static EERegister clRegister(&Wire, wireLock);
  clRegister.doFactoryReset();

  uavcan_register_Value_1_0_select_natural16_(&val);
  val.natural16.value.count       = 1;
  val.natural16.value.elements[0] = 0xFFFFu; // Setup to Undefined value
  clRegister.read(REGISTER_UAVCAN_MTU, &val);
  
  if(val.natural16.value.elements[0] == CAN_MTU_BASE) {
    // ***** TEST ***** Done Reset OK  
  }

  // READ NO EXIXTING ARRAY REGISTER ( ... -> AUTO INIT VALUE IF INTERNAL REQUEST )
  uavcan_register_Value_1_0_select_natural16_(&val);
  val.natural16.value.count = 2;
  val.natural16.value.elements[0] = 100;        // Setting Value
  val.natural16.value.elements[1] = 2000;       // Setting Value
  val.natural16.value.elements[2] = 3000;       // Setting Value
  clRegister.read("rmap.test", &val);           // READ UNSETTED REGISTER = INIT!!!

  // RE READ REGISTER WITH ANOTHER SETUP ( SETUP MAY BE FIRST INIT )
  uavcan_register_Value_1_0_select_natural16_(&val);
  val.natural16.value.count = 2;
  val.natural16.value.elements[0] = 0xFFFF;     // Setting Value
  val.natural16.value.elements[1] = 0xFFFF;     // Setting Value
  val.natural16.value.elements[2] = 0xFFFF;     // Setting Value
  clRegister.read("rmap.test", &val);           // READ SETTED REGISTER = NO INIT!!!

  if((val.natural16.value.elements[0] == 100) &&
    (val.natural16.value.elements[1] == 2000) &&
    (val.natural16.value.elements[0] == 3000)) {
    // ***** TEST ***** Done Setup New Register OK  
  }

  // RE READ REGISTER WITH ANOTHER SETUP ( SETUP MAY BE FIRST INIT )
  uavcan_register_Value_1_0_select_natural16_(&val);
  val.natural16.value.count = 2;
  val.natural16.value.elements[0] = 0x500;       // Setting Value
  val.natural16.value.elements[1] = 0x600;       // Setting Value
  val.natural16.value.elements[2] = 0x800;       // Setting Value
  clRegister.write("rmap.test", &val);           // REWRITE EXISTING REGISTER

  // RE READ REGISTER WITH MODIFIED VALUE ( SETUP MAY BE FIRST INIT )
  uavcan_register_Value_1_0_select_natural16_(&val);
  val.natural16.value.count = 2;
  val.natural16.value.elements[0] = 0xFFFF;     // Setting Value
  val.natural16.value.elements[1] = 0xFFFF;     // Setting Value
  val.natural16.value.elements[2] = 0xFFFF;     // Setting Value
  clRegister.read("rmap.test", &val);           // READ SETTED REGISTER = NO INIT!!!

  if((val.natural16.value.elements[0] == 0x500) &&
    (val.natural16.value.elements[1] == 0x600) &&
    (val.natural16.value.elements[0] == 0x800)) {
    // ***** TEST ***** Done Modify Register OK  
    // ***** TEST END
  }

  // ***************** SET PARAMETER TO TASK *********************

  // TASK WDT, INFO STACK PARAM CONFIG AND CHECK BOOTLOADER STATUS
  static WdtParam_t wdtParam = {0};
  wdtParam.system_status = &system_status;
  wdtParam.systemStatusLock = systemStatusLock;
  wdtParam.rtcLock = rtcLock;
  wdtParam.eeprom = &memEprom;

  static WdtTask wdt_task("WdtTask", 350, OS_TASK_PRIORITY_01, wdtParam);

  // Run Schedulher
  Thread::StartScheduler();

}

// FreeRTOS idleHook callBack to loop
void loop() {
}

// Setup Wire I2C Interface
void init_wire()
{
#if (ENABLE_I2C1)
  Wire.begin();
  Wire.setClock(I2C1_BUS_CLOCK_HZ);
#endif

#if (ENABLE_I2C2)
  Wire2.begin();
  Wire2.setClock(I2C2_BUS_CLOCK_HZ);
#endif
}
