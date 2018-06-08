// current configuration version
#define CONFVER "ttn00"

// define DIO Radio pins
// old
//#define DIO0 2
//#define DIO1 3
//#define DIO2 4
//new
#define DIO0 D2
#define DIO1 A7
#define DIO2 A6

// define pin and led that force and display configuration status at boot
#define FORCECONFIGPIN 8

// read sensors and send messages every SAMPLETIME seconds
#define SAMPLETIME 900UL

#define JOINRETRYDELAY 60UL

#define TXTIMEOUT 130UL

// enable for one channel gateway
//#define CHANNEL0

// enable for deep sleep microcontroller and disable jsrpc runtime
#define DEEPSLEEP
