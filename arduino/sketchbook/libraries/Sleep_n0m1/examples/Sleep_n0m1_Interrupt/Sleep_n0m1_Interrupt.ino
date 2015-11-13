#include <Sleep_n0m1.h>

Sleep sleep;
const int mode = FALLING;
const int pin = 18;

void setup()
{
   Serial.begin(9600);
   if(mode == FALLING || mode == LOW)
     {
       //int pin = interrupt + 2; //will fail on the mega	
       pinMode (pin, INPUT);
       digitalWrite (pin, HIGH);
     }

  Serial.println("start");

}

void loop()
{
  
  Serial.println("execute your code here");
  delay(5000);
  Serial.println("Sleeping Till Interrupt");
  delay(100); //delay to allow serial to fully print before sleep
    
  sleep.pwrDownMode(); //set sleep mode
  
  //Sleep till interrupt pin equals a particular state. 
  //In this case "falling" change from 1 to 0.
  sleep.sleepInterrupt(0,FALLING); //(interrupt Number, interrupt State)
 
 
}
