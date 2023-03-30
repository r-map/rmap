#include "local_typedef.h"

// ************************************************************************************
// ****************************** ELEMENTS STYLES *************************************
// ************************************************************************************
#define HEIGHT_RECT_SERIAL_NUMBER 12
#define LINE_BREAK                10
#define WIDTH_RECT_HEADER         15
#define X_PEAK_TRIANGLE           13
#define X_RECT                    0
#define X_RECT_SERIAL_NUMBER      3
#define X_TEXT_FROM_RECT          5
#define Y_PEAK_TRIANGLE           24
#define Y_RECT                    0
#define Y_RECT_SERIAL_NUMBER      113

#define X_RECT_HEADER_MARGIN                 (X_RECT + 2)
#define X_RECT_SERIAL_NUMBER_MARGIN          (X_RECT_SERIAL_NUMBER + 5)
#define X_TEXT_FROM_RECT_DESCRIPTION_COMMAND (X_TEXT_FROM_RECT + 13)
#define X_TEXT_SYSTEM_MESSAGE                (X_TEXT_FROM_RECT + 20)
#define Y_RECT_HEADER                        (WIDTH_RECT_HEADER + Y_RECT)
#define Y_RECT_HEADER_MARGIN                 (Y_RECT + 0)
#define Y_TEXT_FIRST_LINE                    (Y_RECT_HEADER + 13)
#define Y_TEXT_FROM_RECT                     (Y_RECT_HEADER - 3)
#define Y_TOP_TRIANGLE                       (WIDTH_RECT_HEADER + 5)
// ************************************************************************************
// ****************************** TIMINGS *********************************************
// ************************************************************************************
#define DEBOUNCE_TIMEOUT    50
#define DISPLAY_OFF_TIMEOUT 60000
// ************************************************************************************
// ****************************** CONFIGURATIONS **************************************
// ************************************************************************************
#define ALPHABET_LENGTH             39
#define FIRMWARE_VERSION_LCD_LENGTH 30
#define STATION_LCD_LENGTH          (BOARDSLUG_LENGTH + 10)
// ************************************************************************************
// ****************************** ROTARY **********************************************
// ************************************************************************************
#define DIR_CLOCK_WISE         1
#define DIR_COUNTER_CLOCK_WISE 2
#define DIR_NONE               0
// ************************************************************************************
// ****************************** SYMBOLS *********************************************
// ************************************************************************************
#define U8G2_SYMBOL_APPLY       66
#define U8G2_SYMBOL_DOWNLOAD    84
#define U8G2_SYMBOL_EXIT        65
#define U8G2_SYMBOL_MAINTENANCE 72
#define U8G2_SYMBOL_UNDO        67