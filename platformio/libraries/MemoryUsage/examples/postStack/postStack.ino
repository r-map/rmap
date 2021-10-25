#include <MemoryUsage.h>

STACK_DECLARE

// Dummy structure sample, with a complex content...
struct rhaaa
{
    int ival[50];
    double dval[10];
    char text[80];
};

void subFull(rhaaa aSample);
void subPointer(rhaaa *apSample);
void subSmartPointer(rhaaa &aSample);
void subConstSmartPointer(const rhaaa &aSample);

void setup() 
{
    post_StackPaint();
    Serial.begin(115200);
    
    // An instance of the sample is declared, and the string is filled with
    // some string to see how to access to it inside functions !
    rhaaa sample; 
    strcpy(sample.text, "Test string");
    for (int i = 0; i < 50; i++)
        sample.ival[i] = i;
    for (int i = 0; i < 10; i++)
        sample.dval[i] = (double) i;

    Serial.println(F("Starting state of the memory:"));
    Serial.println();
    
    MEMORY_PRINT_START
    MEMORY_PRINT_HEAPSTART
    MEMORY_PRINT_HEAPEND
    MEMORY_PRINT_STACKSTART
    MEMORY_PRINT_END
    MEMORY_PRINT_STACKSIZE

    POST_STACKPAINT_PRINT

    Serial.println();
    Serial.println();
   
    // Here we pass a pointer to the original structure instance.
    // Only this pointer is added to the stack.
    // The content is fully modifiable by the sub function.
    // This is the best way to let a sub funtion modify an argument.
    subPointer(&sample);
    
    // Here also, this is a pointer which is passed, but the sub function see its argument
    // as a normal data, not a pointer. Be careful here because the sub function can modify
    // the structure content and because this is not a pointer syntax, you can believe that
    // you only modify a copy !
    subSmartPointer(sample);
    
    // You have here the best way to pass a structure if you dont want to modify it.
    // Only a pointer is added to the stack, and any try to modify the struture content
    // will be detected as an error by the compiler.
    subConstSmartPointer(sample);
    
    // Just pass the structure. The full content is duplicated onto the stack.
    // If the sub function modifies the content, the original structure from the caller
    // function will not be affected.
    subFull(sample);
   
    // No data as argument, nut a nig array of doubles inside the function...
    subLocalData();
   
    POST_STACKPAINT_PRINT

    Serial.println();
    Serial.println();
    
    Serial.println(F("Ending state of the memory:"));
    Serial.println();
    
    MEMORY_PRINT_START
    MEMORY_PRINT_HEAPSTART
    MEMORY_PRINT_HEAPEND
    MEMORY_PRINT_STACKSTART
    MEMORY_PRINT_END
    MEMORY_PRINT_STACKSIZE

    Serial.println();
    Serial.println();
}

void subFull(rhaaa aSample)
{
    Serial.println("subFull");
    Serial.println(aSample.text);
    MEMORY_PRINT_STACKSTART
    MEMORY_PRINT_END
    MEMORY_PRINT_STACKSIZE
    STACK_PRINT
    Serial.println();
}

void subPointer(rhaaa *apSample)
{
    Serial.println("subPointer");
    Serial.println(apSample->text);
    MEMORY_PRINT_STACKSTART
    MEMORY_PRINT_END
    MEMORY_PRINT_STACKSIZE
    STACK_PRINT
    Serial.println();
}

void subSmartPointer(rhaaa &aSample)
{
    Serial.println("subSmartPointer");
    Serial.println(aSample.text);
    MEMORY_PRINT_STACKSTART
    MEMORY_PRINT_END
    MEMORY_PRINT_STACKSIZE
    STACK_PRINT
    Serial.println();
}

void subConstSmartPointer(const rhaaa &aSample)
{
    Serial.println("subConstSmartPointer");
    Serial.println(aSample.text);
    MEMORY_PRINT_STACKSTART
    MEMORY_PRINT_END
    MEMORY_PRINT_STACKSIZE
    STACK_PRINT
    Serial.println();
}

#define SIZE    200
void subLocalData()
{
    Serial.println("subLocalData");
    double v[SIZE];

    for(int i = 0; i < SIZE; i++)
        v[i] = (double)i;
    
    Serial.println(v[10]);
    MEMORY_PRINT_STACKSTART
    MEMORY_PRINT_END
    MEMORY_PRINT_STACKSIZE
    STACK_PRINT
    Serial.println();
}

void loop() 
{

}
