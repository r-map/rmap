#define configUSE_CMSIS_RTOS_V2         1
#define configCHECK_FOR_STACK_OVERFLOW  1

// Enable for use LowPower
#define _USE_FREERTOS_LOW_POWER
// Enable For Exit Immediate from LowPower (Using for Debug)
#define _EXIT_SLEEP_FOR_DEBUGGING

// Define For LowPower Method
#ifdef _USE_FREERTOS_LOW_POWER

#define LOW_POWER_NONE                  0
#define LOW_POWER_DEFAULT               1
#define LOW_POWER_PRIVATE_LPTIMx_TICK   _USE_FREERTOS_LOW_POWER

// Freertos Tickless Mode (LOW_POWER_PRIVATE Enable LptimTick.c)
#define configUSE_TICKLESS_IDLE         LOW_POWER_PRIVATE_LPTIMx_TICK

// Define for lptimTick.c
#if (configUSE_TICKLESS_IDLE==LOW_POWER_PRIVATE_LPTIMx_TICK)
    #define configTICK_USES_LSE                 // USE LSE CLOCK
    #define configLPTIM_REF_CLOCK_HZ 32768UL    // LSE CLOCK REFERENCE
    #define configLPTIM_ENABLE_PRECISION 1      // ENABLE AUTO PRECISION
    #define configLPTIM_SRC_LPTIM1              // LPTIM1 OR LPTIM2 CONFIG
#endif

// Time minimal for start LOW_POWER && SuppressTick...
#define configEXPECTED_IDLE_TIME_BEFORE_SLEEP   100

// Macro Pre && Post Sleep/Wake method
#define configPRE_SLEEP_PROCESSING( x ) xTaskSleepPrivate ( &x )
#define configPOST_SLEEP_PROCESSING( x ) xTaskWakeUpPrivate ( x )

#endif