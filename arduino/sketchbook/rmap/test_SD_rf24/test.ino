#include <SPI.h>
#include <SdFat.h>
#include <RF24Network.h>
#include <RF24.h>

SdFat SD;

// On the Ethernet Shield, CS is pin 4. Note that even if it's not
// used as the CS pin, the hardware CS pin (10 on most Arduino boards,
// 53 on the Mega) must be left as an output or the SD library
// functions will not work.
const int chipSelect = 7;

File dataFile;

uint8_t key[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
uint8_t  iv[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

// nRF24L01(+) radio attached using Getting Started board 
RF24 radio(9,10);

// Network uses that radio
RF24Network network(radio);

// Address of our node
const uint16_t this_node = 0;

// Address of the other node
const uint16_t other_node = 1;

// How often to send 'hello world to the other unit
const unsigned long interval = 5000; //ms

// When did we last send?
unsigned long last_sent;

// buffer
char buffer[200];
char bufferr[200];


void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  SPI.begin();

  // workaround to use SDcard and RF24 together
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);

  Serial.print("Initializing SD card...");
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1) ;
  }
  Serial.println("card initialized.");

  // Open up the file we're going to log to!
  dataFile = SD.open("datalog.txt", FILE_WRITE);
  if (! dataFile) {
    Serial.println("error opening datalog.txt");
    // Wait forever since we cant write data
    while (1) ;
  }


  Serial.print("Initializing radio card...");
  radio.begin();
  Serial.println("card initialized.");

  Serial.print("Initializing network...");
  network.begin(/*channel*/ 93, /*node address*/ this_node, key, iv);

  radio.setRetries(1,15);
  network.txTimeout=500;
  Serial.println("network initialized.");

}

void loop()
{

  // make a string for assembling the data to log:
  String dataString = "";

  // read three sensors and append to the string:
  for (int analogPin = 0; analogPin < 3; analogPin++) {
    int sensor = analogRead(analogPin);
    dataString += String(sensor);
    if (analogPin < 2) {
      dataString += ","; 
    }
  }

  dataFile.println(dataString);

  // print to the serial port too:
  Serial.println("write on SD.");
  Serial.println(dataString);
  
  // The following line will 'save' the file to the SD card after every
  // line of data - this will use more power and slow down how much data
  // you can read but it's safer! 
  // If you want to speed up the system, remove the call to flush() and it
  // will save the file only every 512 bytes - every time a sector on the 
  // SD card is filled with data.
  dataFile.flush();


  Serial.println("network update.");
  // Pump the network regularly
  network.update();

  strcpy (buffer,"abcdefghilmnopqrstuvzABCDEFGHILMNOPQRSTUVZ0123456789");

  Serial.println("Sending...");
  RF24NetworkHeader header( other_node,0);

  bool ok = network.writemulti(header,buffer,strlen(buffer)+1);

  if (ok){
    Serial.println("ok.");
    Serial.println("Receiving...");
    size_t size = network.readmulti(header,bufferr,sizeof(bufferr));
    if (size >0){
      Serial.print("message: "); Serial.println(bufferr);
    }else 
      Serial.println("Error");
  } else
    Serial.println("failed.");
 
  // Take 1 measurement every 500 milliseconds
  delay(500);
}
