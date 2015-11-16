#define SIZE 2000
uint8_t mem[SIZE];
int i;


void setup() {
  Serial.begin(9600);        // connect to the serial port
  Serial.println("Start");

  for (i=0; i<SIZE; i++){
    mem[i]=0xFF;
    if (mem[i] != 0xFF){
      Serial.print("error: ");
      Serial.print(mem[i]);
      Serial.print(" : ");
      Serial.println(i);
    }
  }

  for (i=0; i<SIZE; i++){
    mem[i]=0x00;
    if (mem[i] != 0x00){
      Serial.print("error: ");
      Serial.print(mem[i]);
      Serial.print(" : ");
      Serial.println(i);
    }
  }

  Serial.println("OK 1");

}

void loop() {
uint8_t memm[SIZE];

  for (i=0; i<SIZE; i++){
    memm[i]=0xFF;
    if (memm[i] != 0xFF){
      Serial.print("error: ");
      Serial.print(memm[i]);
      Serial.print(" : ");
      Serial.println(i);
    }
  }

  for (i=0; i<SIZE; i++){
    memm[i]=0x00;
    if (memm[i] != 0x00){
      Serial.print("error: ");
      Serial.print(memm[i]);
      Serial.print(" : ");
      Serial.println(i);
    }
  }

  Serial.println("OK 2");
  delay(3000);
}  
