#include <AESLib.h>
uint8_t key[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
uint8_t iv[]  = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
const uint16_t data_len=128;
char buffer[128];


int freeRam ()
{
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}


void setup()
{
  Serial.begin(9600);
}

void loop()
{
  Serial.print(F("free:")); Serial.println(freeRam());

#ifdef NONE
  char datasingle[] = "0123456789012345"; //16 chars == 16 bytes
  aes128_enc_single(key, datasingle);
  Serial.print("encrypted:");
  Serial.println(datasingle);
  aes128_dec_single(key, datasingle);
  Serial.print("decrypted:");
  Serial.println(datasingle);
#endif

  // encrypt multiple blocks of 128bit data, data_len but be mod 16
  // key and iv are assumed to be both 128bit thus 16 uint8_t's

  char data[] = "ciao, how are you? fine and you? fine ciao ciao";
  
  strncpy(buffer, data, sizeof(buffer));
  //Serial.println(buffer);
  long start=millis();
  aes128_cbc_enc(key, iv, buffer, data_len);
  //Serial.print("encrypted:");
  //Serial.println(buffer);
  aes128_cbc_dec(key,iv, buffer, data_len);
  Serial.println(millis()-start);
  //Serial.print("decrypted:");
  //Serial.println(buffer);

  delay(3000);
}
