#include <AES.h>
uint8_t key[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
uint8_t my_iv[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
byte iv [16] ;
const uint16_t data_len=128;
byte buffer[128];

AES aes ;

void setup()
{
  Serial.begin(9600);
}

void loop()
{
  byte succ;
  succ = aes.set_key (key, 128) ;
  //Serial.println(succ);
  for (byte i = 0 ; i < 16 ; i++)
    iv[i] = my_iv[i] ;

#ifdef NONE
  char datasingle[] = "012345678901234\n"; //16 chars == 16 bytes

  Serial.print(F("Starting string")); Serial.println(datasingle);
 
  succ = aes.encrypt ((byte*)datasingle, buffer) ;
  Serial.println(succ);
  Serial.print("encrypted:");
  Serial.println((char*)buffer);
  succ = aes.decrypt (buffer, buffer) ;
  Serial.println(succ);
  Serial.print("decrypted:");
  Serial.println((char*)buffer);
#endif

  // encrypt multiple blocks of 128bit data, data_len but be mod 16
  // key and iv are assumed to be both 128bit thus 16 uint8_t's
  
  char data[] = "ciao, how are you? fine and you? fine ciao ciao";
  aes.copy_n_bytes (buffer,(byte*) data, sizeof(data));
  //strncpy((char*)buffer, data, sizeof(buffer));
  //Serial.print("original:"); Serial.println((char*)buffer);
  long start=millis();
  succ = aes.cbc_encrypt (buffer, buffer, 3, iv) ;
  //Serial.println(succ);
  //Serial.print("encrypted:"); Serial.println((char*)buffer);
  // for (byte i = 0 ; i < 128 ; i++)
  // Serial.print(buffer[i]);
  //Serial.println("END");

  for (byte i = 0 ; i < 3 ; i++)
    iv[i] = my_iv[i] ;

  succ = aes.cbc_decrypt (buffer, buffer, 3, iv) ;
  //Serial.println(succ);
  //Serial.print("decrypted:"); Serial.println((char*)buffer);
  Serial.println(millis()-start);

  aes.clean();

  delay(3000);
}
