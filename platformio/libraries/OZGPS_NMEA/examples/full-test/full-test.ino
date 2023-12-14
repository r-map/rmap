#include <Arduino.h>
#include <stdio.h>
#include <stdint.h>
#include "ozgps.h"

//#define uart_only //if has a only device remove // 

OZGPS gps;
uint32_t t = 0;

// _write fonksiyonunu UART gönderme fonksiyonu ile değiştirme
int _write(int file, char* ptr, int len)
{
    int count = len;
    while (count--)
    {
        Serial.print(*ptr++);
    }
    return len;
}

void setup(){
	//init serial
	Serial.begin(115200); // Seri haberleşmeyi başlat
	
    //init gps
    uint8_t gpsflag;
    MGPS mgps;
    gps.init(&mgps);
	
#ifndef uart_only
    const char* console_text = "$GPTXT,01,01,02,ANTSTATUS=INIT*25\r\n$GPRMC,,V,,,,,,,,,,N*53\r\n$GPVTG,,,,,,,,,N*30\r\n$GPGGA,,,,,,0,00,99.99,,,,,,*48\r\n$GPGSA,A,1,,,,,,,,,,,,,99.99,99.99,99.99*30\r\n$GPGLL,,,,,,V,N*64\r\n$GPXTE,A,A,0.67,L,N*6F\r\n$GPXTE,A,A,0.67,L,N*6f\r\n$GPGGA,123204.00,5106.94086,N,01701.51680,E,1,06,3.86,127.9,M,40.5,M,,*51\r\n$GPGSA,A,3,02,08,09,05,04,26,,,,,,,4.92,3.86,3.05*00\r\n$GPGSV,4,1,13,02,28,259,33,04,12,212,27,05,34,305,30,07,79,138,*7F\r\n$GPGSV,4,2,13,08,51,203,30,09,45,215,28,10,69,197,19,13,47,081,*76\r\n$GPGSV,4,3,13,16,20,040,17,26,08,271,30,28,01,168,18,33,24,219,27*74\r\n$GPGSV,4,4,13,39,31,170,27*40\r\n$GPGLL,5106.94086,N,01701.51680,E,123204.00,A,A*63\r\n$GPRMC,123205.00,A,5106.94085,N,01701.51689,E,0.016,,280214,,,A*7B\r\n$GPVTG,,T,,M,0.016,N,0.030,K,A*27\r\n$GPGST,024603.00,3.2,6.6,4.7,47.3,5.8,5.6,22.0*58\r\n$GPZDA,160012.71,11,03,2004,-1,00*7D\r\nGNGBS,170556.00,3.0,2.9,8.3,,,,*5C\r\n";
	
    for (size_t i = 0; i < strlen(console_text); i++)
    {
        gpsflag = gps.encode(console_text[i]);
        if(gps.valid){
            t++;
            printf("parser_ok: %d\n", t);
            printf("latitude: %f\n", mgps.rmc.dms.latitude);
        }
    }

    printf("gps_error: %d", gpsflag);
#endif

}

void loop(){
	delay(100);
	
#ifdef uart_only

  if (Serial.available()) { // Seri portta veri var mı kontrol et
	  char c = Serial.read();
	  gps.encode(c);
  }
  
	if(gps.valid){
		t++;
		printf("parser_ok: %d\n", t);
		printf("latitude: %f\n", mgps.rmc.dms.latitude);
	}else{
		printf("gps_error: %d", gpsflag);
	}
		
	printf();	
#endif
}