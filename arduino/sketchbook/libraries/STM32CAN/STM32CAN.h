/* -------------------------------------------------------------------------------------------
 simpel CAN library for stm32 devices using ST's official HAL layer, this include the ST supported Arduino
 
 it only support CAN1 and CAN2 as I don't have any devices with 3 CAN interfaces

 Before you can use this driver you need to enable the CAN module in the HAL configuration file, 
	create a hal_conf_extra.h file in the root folder of the project and adding the following three lines:
 
#if !defined(HAL_CAN_MODULE_ENABLED)
#define HAL_CAN_MODULE_ENABLED
#endif
 
This library can be used in a C++ projects by replacing the
#include <Arduino.h> with the include for the used HAL for example;
#include <stm32f1xx_hal.h>

Inspired by the following giving me the input needed to make this work:
https://github.com/jiauka/stm32Can
https://gist.github.com/Arman92/154e2540847b32c44c29
https://github.com/collin80
ST's CAN examples and documentation

For CAN1 standard pins are ( PB_8 and PB_9) if set to false it use (PA_11 and PA_12)
For CAN2 standard pins are ( PB_5 and PB_6) if set to false it use (PB_13 and PB_13)

CAN message filtering: 
This library use 32 bit IDMASK filtering

This is a good explanation of how mask and ID are handled in STM32 devices
https://schulz-m.github.io/2017/03/23/stm32-can-id-filter/
https://community.st.com/s/question/0D50X00009XkfSlSAJ/can-filters
More around standard and extended ID
http://www.copperhilltechnologies.com/can-bus-guide-extended-can-protocol/

Standard ID have a value between 0 and 0x7FF
Extended ID have a value between 0 and 0x1FFFFFFF
 
A CANBUS B frame (extended) consists of a four byte header (containing a 29-bit identifier), followed by up to 8 data bytes.
A receiving node would examine the identifier to decide if it was relevant (e.g. waiting for a frame with ID 00001567 
which contains data to switch on or off a motor). 
It could do this via software (using a C if or case statement); in practice the Canbus interface contains firmware to 
carry out this task using the acceptance filter and mask value to filter out unwanted messages.
The filter mask is used to determine which bits in the identifier of the received frame are compared with the filter
If a mask bit is set to a zero, the corresponding ID bit will automatically be accepted, regardless of the value of the filter bit.
If a mask bit is set to a one, the corresponding ID bit will be compare with the value of the filter bit; if they 
match it is accepted otherwise the frame is rejected.

Default this laibrary accept any frame e.g. no filters are applied
set filter to 0
set mask to 0

Example 1. we wish to accept only frames with ID of 00001567 (hexadecimal values)
set filter to 00001567
set mask to 1FFFFFFF
when a frame arrives its ID is compared with the filter and all bits must match; any frame that does not match ID 00001567 is rejected

Example 2. we wish to accept only frames with IDs of 00001560 thru to 0000156F
set filter to 00001560
set mask to 1FFFFFF0
when a frame arrives its ID is compared with the filter and all bits except bits 0 to 3 must match; any frame other frame is rejected

Example 3. we wish to accept only frames with IDs of 00001560 thru to 00001567
set filter to 00001560
set mask to 1FFFFFF8

when a frame arrives its ID is compared with the filter and all bits except bits 0 to 2 must match; any frame other frame is rejected

Library filter function:
bool setFilter( uint32_t FilterID, uint32_t FilterMask, uint8_t FilterBank, bool IDStdOrExt );

Please read the links to figure out FilterID and FilterMask
FilterBank have to be defined pr. CAN interface, 0 to 13 handle Can1 message filters and 14 to 27 handle Can1 message filters
You alway have to start with the default filter e.g. 0 for Can1 and 14 for Can2 as they by default is set to allow all messages
StdOrExt define ID type, default is standard

Example:
We would like to recive all CAN1 messages for std ID within range 0x400 thru to 0x40f
Can1.setFilter( 0x400, 0x7f0, 0, IDStd );

---------------------------------------------------------------------------------------------*/


#ifndef __stm32Can_H__
#define __stm32Can_H__

#include <Arduino.h>

#if !defined(SIZE_RX_BUFFER)
#define SIZE_RX_BUFFER  16 // receive incoming ring buffer default size
#endif

#if !defined(SIZE_TX_BUFFER)
#define SIZE_TX_BUFFER  16 // transmit ring buffer default size
#endif
#define NUM_BUFFERED_MBOXES 2

/** Define the typical baudrate for CAN communication. */
#ifdef CAN_BPS_500K
#undef CAN_BPS_1000K 
#undef CAN_BPS_800K                 
#undef CAN_BPS_500K                 
#undef CAN_BPS_250K                 
#undef CAN_BPS_125K                  
#undef CAN_BPS_50K                   
#undef CAN_BPS_33333          
#undef CAN_BPS_25K                   
#undef CAN_BPS_10K                   
#undef CAN_BPS_5K                   
#endif

#define CAN_BPS_1000K 1000000
#define CAN_BPS_800K  800000
#define CAN_BPS_500K  500000
#define CAN_BPS_250K  250000
#define CAN_BPS_125K  125000
#define CAN_BPS_50K   50000
#define CAN_BPS_33333 33333
#define CAN_BPS_25K   25000
#define CAN_BPS_10K   10000
#define CAN_BPS_5K    5000

#define CAN_DEFAULT_BAUD  CAN_BPS_500K

#define canLoopBack true

#define IDStd 0
#define IDExt 1


// #define DEBUG(x) x // Enable this to print debug messages, require serial connection
#define DEBUG(x) "" // Debug disabled

//This structure presupposes little endian mode. If you use it on a big endian processor you're going to have a bad time.
typedef union {
  uint64_t value;
  struct {
    uint32_t low;
    uint32_t high;
  };
  struct {
    uint16_t s0;
    uint16_t s1;
    uint16_t s2;
    uint16_t s3;
    };
  uint8_t bytes[8];
} BytesUnion;

/* CAN frame structure */
typedef struct
{
  uint32_t id;       // Standard ID if ide = 0, Extended ID otherwise
  uint8_t  ide;      // Specifies the type of identifier for the message that will be transmitted, 0 for standard and 1 for extended
  uint8_t  rtr;      // Remote Transmission Request
  uint8_t  priority; // Priority but only important for TX frames and then only for special uses.
  uint16_t timestamp;// CAN timer value when mailbox message was received.
  uint8_t  dlc;      // Number of data bytes
  BytesUnion  data;  // 64 bits - lots of ways to access it
} CAN_message_t;

class stm32Can {
  public:
    /* Constructor */
    stm32Can( CAN_HandleTypeDef* pCanHandle, int portNumber );

    /* user interface */    
    void begin( void ) { begin( CAN_DEFAULT_BAUD, false, true ); }; // begin with default baudrate
    void begin( int baudrate ) { begin( baudrate, false, true ); }; // begin with user defined baudrate
    void begin( int baudrate, bool loopBack ) { begin( baudrate, loopBack, true ); }; // begin with option to enable loopback
    void begin( int baudrate, bool loopBack, bool UseAltPins); // begin with user defined baudrate and option for using loopback and alternative pins
    
    bool write( CAN_message_t &msg) { return write( msg, true ); };
    bool write( CAN_message_t &msg, bool wait_sent );
	
    bool read( CAN_message_t &msg );
	
    uint32_t available( void );

    bool readdebug(CAN_message_t &msg); // For debug purpose, read CAN message if interupt not is working
	
	bool setFilter( uint32_t FilterID, uint32_t FilterMask, uint8_t FilterBank, bool IDStdOrExt );
  
    // Before begin, you can define rx buffer size. Default is SIZE_RX_BUFFER. This does not have effect after begin.
    void setRxBufferSize(uint16_t size) {if (!isInitialized()) sizeRxBuffer = size;}

    // Before begin, you can define global tx buffer size. Default is SIZE_TX_BUFFER. This does not have effect after begin.
    void setTxBufferSize(uint16_t size) {if (!isInitialized() ) sizeTxBuffer=size;}
    
    typedef struct RingbufferTypeDef {
      volatile uint16_t head;
      volatile uint16_t tail;
      uint16_t size;
      volatile CAN_message_t *buffer;
    } RingbufferTypeDef;
  
    RingbufferTypeDef rxRing;
    RingbufferTypeDef txRing;

    /* This needs to be public as its caled from interupt */
    bool addToRingBuffer(RingbufferTypeDef &ring, const CAN_message_t &msg);

  
  protected:
    uint16_t sizeRxBuffer;
    uint16_t sizeTxBuffer;
  
  private:
    /* functions */
    bool      removeFromRingBuffer( RingbufferTypeDef &ring, CAN_message_t &msg );
    bool      isInitialized() { return rx_buffer != 0; }
    void      initRingBuffer( RingbufferTypeDef &ring, volatile CAN_message_t *buffer, uint32_t size );
    void      initializeBuffers( void );
    bool      isRingBufferEmpty( RingbufferTypeDef &ring );
    uint32_t  ringBufferCount( RingbufferTypeDef &ring );
    void      init( CAN_HandleTypeDef* CanHandle, int baudrate, bool UseAltPins, bool loopBack );
    void      calculateBaudrate( CAN_HandleTypeDef *CanHandle, int Baudrate );
    uint32_t  getAPB1Clock( void );

    volatile CAN_message_t *rx_buffer;
    volatile CAN_message_t *tx_buffer;
    
    bool     _canIsActive = false;
    int      _portNumber = 0;
    
    /* set by constructor */
    CAN_HandleTypeDef *n_pCanHandle;

};

extern stm32Can Can1;

#ifdef CAN2
extern stm32Can Can2;
#endif

#endif

