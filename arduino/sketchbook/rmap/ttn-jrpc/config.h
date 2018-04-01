// current configuration version
#define CONFVER "ttn00"

// define DIO Radio pins
#define DIO0 2
#define DIO1 3
#define DIO2 4

// define pin and led that force and display configuration status at boot
#define FORCECONFIGPIN 8

// read sensors and send messages every SAMPLETIME seconds
#define SAMPLETIME 10800UL

// enable for one channel gateway
//#define CHANNEL0

// enable for deep sleep microcontroller and disable jsrpc runtime
#define DEEPSLEEP yes
