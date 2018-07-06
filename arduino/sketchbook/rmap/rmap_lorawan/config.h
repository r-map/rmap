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

//pin that command power up and down
#define POWERPIN D3
//#define POWERPIN_PULL INPUT
#define POWERPIN_PULL INPUT_PULLUP

// define the output pins used for (relays)
#define OUTPUTPINS D7,D8

// on when station is power on
#define POWERLED D9

#define JOINRETRYDELAY 60UL

// this should be removed !
#define TXTIMEOUT 130UL

// enable for one channel gateway
//#define CHANNEL0

// enable for deep sleep microcontroller and disable jsrpc runtime
#define DEEPSLEEP

#define digitalPinToInterrupt(p) ((p) == 2 ? 0 : ((p) == 3 ? 1 : ((p) >= 18 && (p) <= 21 ? 23 - (p) : NOT_AN_INTERRUPT)))
