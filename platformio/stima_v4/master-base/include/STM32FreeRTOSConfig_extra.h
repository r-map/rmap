#define configUSE_CMSIS_RTOS_V2        1
#define configCHECK_FOR_STACK_OVERFLOW 1

#define _USE_FREERTOS_LOW_POWER        1
#ifdef _USE_FREERTOS_LOW_POWER
    #define _EXIT_SLEEP_FOR_DEBUGGING
    #define configUSE_TICKLESS_IDLE           1
    #define configEXPECTED_IDLE_TIME_BEFORE_SLEEP   100
    #define configPRE_SLEEP_PROCESSING( x ) xTaskSleepPrivate ( &x )
    #define configPOST_SLEEP_PROCESSING( x ) xTaskWakeUpPrivate ( x )
#endif
