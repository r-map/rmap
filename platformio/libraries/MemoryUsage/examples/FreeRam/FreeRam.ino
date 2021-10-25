#include <MemoryUsage.h>

void setup() 
{
    Serial.begin(115200);
    Serial.println(F("Starting state of the memory:"));
    Serial.println();
    
    MEMORY_PRINT_START
    MEMORY_PRINT_HEAPSTART
    MEMORY_PRINT_HEAPEND
    MEMORY_PRINT_STACKSTART
    MEMORY_PRINT_END
    MEMORY_PRINT_HEAPSIZE

    Serial.println();
    Serial.println();
   
    FREERAM_PRINT;

    byte *p = new byte[3000];    
    
    Serial.println();
    Serial.println();
    
    Serial.println(F("Ending state of the memory:"));
    Serial.println();
    
    MEMORY_PRINT_START
    MEMORY_PRINT_HEAPSTART
    MEMORY_PRINT_HEAPEND
    MEMORY_PRINT_STACKSTART
    MEMORY_PRINT_END
    MEMORY_PRINT_HEAPSIZE
    
    Serial.println();
    Serial.println();
    
    FREERAM_PRINT;
}

void loop() 
{

}