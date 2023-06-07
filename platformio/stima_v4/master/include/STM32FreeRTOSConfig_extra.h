#define configUSE_CMSIS_RTOS_V2         1
#define configCHECK_FOR_STACK_OVERFLOW  1

// USB Serial Freertos Need Malloc Free transparent mode
#define configMEMMANG_HEAP_NB 3

// Enable for use LowPower
#define _USE_FREERTOS_LOW_POWER

#define USE_LOWPOWER_IDLE_LOOP      (false)

// Enable For Exit Immediate from LowPower (Using for Debug)
#define _EXIT_SLEEP_FOR_DEBUGGING

#define SLEEP_NONE         0       // Unused LowPOWER
#define SLEEP_IDLE         1       // Power mode main regulator ON, mode 1 debug standard enabled
#define SLEEP_LOWPOWER     2       // Power mode low power regulator ON, debug Low Power
#define SLEEP_STOP2        3       // Stop2 mode all power regulator OFF

// Define For LowPower Method
#ifdef _USE_FREERTOS_LOW_POWER

#ifdef _EXIT_SLEEP_FOR_DEBUGGING    
    #define LOWPOWER_MODE   SLEEP_NONE
#else
    #define LOWPOWER_MODE   SLEEP_STOP2
#endif

#define LOW_POWER_NONE                  0
#define LOW_POWER_DEFAULT               1
#define LOW_POWER_PRIVATE_LPTIMx_TICK   2

// Freertos Tickless Mode (LOW_POWER_PRIVATE Enable LptimTick.c)
#define configUSE_TICKLESS_IDLE         LOW_POWER_PRIVATE_LPTIMx_TICK

// Define for lptimTick.c
#if (configUSE_TICKLESS_IDLE==LOW_POWER_PRIVATE_LPTIMx_TICK)
    #define configTICK_USES_LSE                     // USE LSE CLOCK
    #define configLPTIM_REF_CLOCK_HZ 32768UL        // LSE CLOCK REFERENCE
    #define configLPTIM_ENABLE_PRECISION 1          // ENABLE AUTO PRECISION
    #define configLPTIM_SRC_LPTIM1                  // LPTIM1 OR LPTIM2 CONFIG
    #define configTICK_ENABLE_UWTICK_PRECISION 1    // ENABLE UPDATE SYSTICK
#endif

// Time minimal for start LOW_POWER && SuppressTick...
#ifdef _EXIT_SLEEP_FOR_DEBUGGING
    #define configEXPECTED_IDLE_TIME_BEFORE_SLEEP   60000
#else
    #define configEXPECTED_IDLE_TIME_BEFORE_SLEEP   50
#endif

// Macro Pre && Post Sleep/Wake method
#define configPRE_SLEEP_PROCESSING( x ) xTaskSleepPrivate ( &x )
#define configPOST_SLEEP_PROCESSING( x ) xTaskWakeUpPrivate ( x )

#endif