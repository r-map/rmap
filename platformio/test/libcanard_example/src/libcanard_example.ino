//#include "stm32yyxx_ll.h"

#define RC_NUM_CHANNELS_TO_PRINT    4

extern "C" void printInitMsg(void) {
  Serial.begin(115200);
  Serial.print("Initializing...");
  Serial.print(HAL_RCC_GetHCLKFreq());
  Serial.println();
}

extern "C" void printArray(int16_t arr[]) {

  char str[20];
  itoa(arr[0], str, 10);
  Serial.print("ESC Array: ");
  Serial.print(str);
  for (int i = 1; i < RC_NUM_CHANNELS_TO_PRINT; i++) {
    itoa(arr[i], str, 10);
    Serial.print(", ");
    Serial.print(str);
  }
  Serial.println();
}

extern "C" void println(char* str) {
  Serial.print(str);
  Serial.println();
}

extern "C" void printstr(char* str) {
  Serial.print(str);
}

extern "C" void printint(uint32_t num) {
  Serial.print(num, HEX);
}
