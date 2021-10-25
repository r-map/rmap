/* stepper library StepperLab3 
/*
 * conrol stepper motor with serial commands
 *
 * KHM Lab3 2010
 * Kunsthochschule fuer Medien Koeln
 * Academy of Media Arts Cologne
 * http://interface.khm.de
 
 */



// import stepper library
#include <StepperLab3.h>
// create instance for stepper
StepperLab3 myStepper;

// poti value
int potiValue = 0;

int ledGreen= 5;
int pinLsSens=15;
int motor1_1=12;
int motor1_2= 10;
int motor2_1= 11;
int motor2_2= 9;

int cnt;
int posi=48;
int rock;

// separateString
char buff[50];
char st1[10];
char st2[10];
char st3[10];
int arg1; 
int arg2; 
int arg3;
int ibuf;

boolean f_receive=false;
boolean f_message=false;
boolean f_continu=false;



// this block is executed one time when programm starts
void setup(){
  Serial.begin(115200);
  Serial.println("Serial Command stepperLab3 library");
  Serial.println("Input: CommandChar blank parameter CR");
  
  Serial.println("g <position>  // goto positon");
  Serial.println("n             // find 0 positon");
  Serial.println("s <speed>     // set speed, steps per second");
  Serial.println("g <power>     // set motor power 0..1023");
  Serial.println("t <steps>     // set number ramp steps");
  Serial.println("r             // read positon");
  Serial.println("m <mode>      // stepmode full 1 half 0");
  Serial.println("u <sw>        // rocker on 1 off 0");

  pinMode(ledGreen,OUTPUT);  // LED pin
  pinMode(pinLsSens,INPUT);  // switch

  digitalWrite(pinLsSens,1);

  myStepper.attach(motor1_1,motor1_2,motor2_1,motor2_2);

  myStepper.setPower(900);
  myStepper.setSpeed(700);
}


// this block is executed in a loop after setup is called one time
void loop(){

  // read poti (value between 0 and 1023)
  potiValue = analogRead(1);

  receive_serial_message();   // Receive serial 
  if (f_message){     // if message complete
    f_message=false;
    arg2=atoi(st2); 
    arg3=atoi(st3);  // ascii to integer
    st1[0] = st1[0] & 0x5f;  // first cahr into uppercase

    switch (st1[0] ) {
    case 'G':  
      Serial.print("goto:");  
      Serial.println(arg2);  
      myStepper.absoluteSteps(arg2);
      posi=arg2;
      break;

    case 'N':  // Goto Null Position
      Serial.println("goto null ");
      go_null();
      break;

    case 'S': // Set Stepper Speed
      Serial.print("set speed:");
      Serial.println(arg2);
      myStepper.setSpeed(arg2);
      break;

    case 'P': // Set Stepper Motor Tourque
      Serial.print("set sower:");
      Serial.println(arg2);
      myStepper.setPower(arg2);
      break;

    case 'T': // Set Stepper Ramp Steps
      Serial.print("ramp steps:");
      Serial.println(arg2);
      myStepper.setRampSteps(arg2);
      break;

    case 'R': // read Stepper position
      Serial.print("pos:");
      Serial.println(myStepper.getSteps());
      break;  

    case 'M': // set Mode
      if (arg2) {
        Serial.println("full step");
        myStepper.setFullStep();
      } 
      else 
      {
        Serial.println("half step");
        myStepper.setHalfStep();
      }
      break;   

    case 'U': // read Stepper position
      Serial.print("rocker:");
      Serial.println(arg2);
      rock = arg2;
      break;  
    }

    Serial.print(st1); 
    Serial.print(":"); 
    Serial.print(st2);
    Serial.println();
  }


  delay(1);
  if (cnt % 500 == 0) {
    digitalWrite(ledGreen,!digitalRead(ledGreen));  // blink led
  }
  cnt++;

  if (rock) rocker();


}
//*********************************************************************
void go_null() {  // goto switch position

  myStepper.absoluteSteps(20);
  while( myStepper.stepReady() == 0);
  delay(200);
  myStepper.rotate(1);
  while(digitalRead(pinLsSens)==1);
  myStepper.rotate(0);

}

//*********************************************************************
void receive_serial_message() {

  if (f_receive){
    if (comTimeout.check() == 1) {
      f_receive = false;
      buff[0]='\0';
      ibuf=0;
      Serial.println("timeout");
    }
  }  

  if ( Serial.available() > 0) {     // if there are bytes waiting on the serial port
    comTimeout.reset();             // message must be finished within timeout time
    comTimeout.interval(200);
    f_receive = true;
    char inByte = Serial.read();   // read a byte
    buff[ibuf]=inByte;             // add to buffer  
    ibuf++;
    buff[ibuf]='\0';
    if (ibuf >= 50) ibuf=0;        // limit buffer size
    // Serial.println(buff);
    if ((inByte == '.') ||(inByte == '*') || ( inByte == '\n') || ( inByte == '\r')) { // wait for message termination character
      separateString();       // separate message string into 3 substrings
      f_message=true;        // message avail now flag
      buff[0]='\0';          // clear message receive
      ibuf=0;
      f_receive = false;

    }

  } 
}
//*********************************************************************
void separateString() {
  st1[0]='\0'; 
  st2[0]='\0'; 
  st3[0]='\0';
  int pos=0;
  int p1 =0;
  int cnt=0;
  char stc[2]="";
  for (int ii=0; ii<= strlen(buff); ii++) {
    if ((buff[ii] >= '-')  ) { //  Separator == all chars < ' '
      stc[0] = buff[ii];
      cnt++;
      if (cnt <= 5) {
        switch (pos){
        case 0:
          strcat(st1,stc);
          break;
        case 1:
          strcat(st2,stc);
          break;
        case 2:
          strcat(st3,stc);
          break;
        default:
          break;
        }
      }  // if < 5 len arguments

    } 
    else     {
      cnt=0;
      pos++;
    }
    while ((buff[ii]  <'-') && (buff[ii+1] < '-') && (ii <= strlen(buff))) {
      ii++;
    }
  }
}

//*********************************************************************
// drives stepper left and right
void rocker() {
  static int state;
  static int cwait;

  switch (state) {
  case 0:
    myStepper.absoluteSteps(posi);
    state=1;
    break;
  case 1:
    if (myStepper.stepReady()) {
      if (cwait == 0) {
      myStepper.absoluteSteps(0);
       state=2;
      }
    }
    break;
  case 2:
    if (myStepper.stepReady()) {
      if (cwait == 0) state=0;
    }
    break;
  }
  
  if (myStepper.stepReady() ==0) cwait=500;
  if (cwait > 0 ) cwait--;
  
}







