/*

 * Pin mapping table for different platforms
 *
 * Platform     IR input    IR output   Tone
 * -----------------------------------------
 * DEFAULT/AVR  3           4           5
 * ATtinyX5     0           4           3
 * ATtin167     9           8           5 // Digispark pro number schema
 * ATtin167     3           2           7
 * SAMD21       3           4           5
 * ESP8266      14 // D5    12 // D6    %
 * ESP32        15          4           %
 * BluePill     PA6         PA7         PA3
 * APOLLO3      11          12          5
 */

#define IRMP_PROTOCOL_NAMES 1 // Enable protocol number mapping to protocol strings - requires some FLASH.
#define IRMP_SUPPORT_NEC_PROTOCOL     1
#define IRSND_SUPPORT_NEC_PROTOCOL    1
#define BUTTON PA2

#define USE_ONE_TIMER_FOR_IRMP_AND_IRSND // otherwise we get an error: redefinition of 'void __vector_8()
#include <irmp.c.h>
#include <irsnd.c.h>

IRMP_DATA irmp_data;
IRMP_DATA irsnd_data;

int abbonamentoscaduto = 0;
bool abbonamentoattivo = false;
bool abbonamentofinito = true;


void IRSend(uint16_t aCommand) {
  irsnd_data.command = aCommand;
  irsnd_send_data(&irsnd_data, true); // true = wait for frame to end. This stores timer state and restores it after sending
}  

void setup() {

    pinMode(BUTTON, INPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(115200);
    delay(2000); // To be able to connect Serial monitor after reset and before first printout
 
    // Just to know which program is running
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRMP));

    digitalWrite(LED_BUILTIN, HIGH);
    delay(400);
    digitalWrite(LED_BUILTIN, LOW);

    irmp_init();
    irmp_LEDFeedback(true); // Enable receive signal feedback at LED_BUILTIN for receive

    irsnd_init();
    irsnd_LEDFeedback(true); // Enable LED feedback for send

    Serial.print(F("Ready to receive IR signals at pin "));
    Serial.println(IRMP_INPUT_PIN);
    Serial.print(F("Ready to send IR signals at pin "));
    Serial.println(IRSND_OUTPUT_PIN);

    irsnd_data.protocol = IRMP_NEC_PROTOCOL;
    irsnd_data.flags = 1; // repeat frame 1 time

}

 void loop() {
       
        if (irmp_get_data(&irmp_data)) {
            if (abbonamentoattivo == true) {
         // irmp_result_print(&irmp_data);
            irsnd_send_data(&irmp_data, true); 
            }else if ((((abbonamentoattivo == false) != (irmp_data.command == 0x04)) != (irmp_data.command == 0x1A)) != (irmp_data.command == 0x1B)){
         //     irmp_result_print(&irmp_data);
         //     Serial.print(F("Abbonamento non attivo. \n"));
                }
            if ((irmp_data.command == 0x04) || (irmp_data.command == 0x1A) || (irmp_data.command == 0x1B)){
               irsnd_send_data(&irmp_data, true);
            } 
         }    
         int ButtonState = digitalRead(BUTTON);
         if (ButtonState == HIGH){
         //  digitalWrite(LED_BUILTIN, HIGH);
             abbonamentoattivo = true;
             abbonamentoscaduto = 0;
             }else if (ButtonState == LOW){
         //      digitalWrite(LED_BUILTIN, LOW);  
                 abbonamentoattivo = false;
                 abbonamentoscaduto++;
              }
              if(abbonamentoscaduto == 1){
                 irsnd_data.address = 0xAF40; // Hospimatic Remote 1
                 IRSend(0x63);
                 IRSend(0x63);
                 IRSend(0x63);
                 IRSend(0x67);
                 irsnd_data.address = 0xAF30; // Hospimatic Remote 2
                 IRSend(0x63);
                 IRSend(0x63);
                 IRSend(0x63);
                 IRSend(0x67);
                 irsnd_data.address = 0xAF50; // Hospimatic Remote 3
                 IRSend(0x63);
                 IRSend(0x63);
                 IRSend(0x63);
                 IRSend(0x67);
                 }                  
}

        
