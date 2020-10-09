#include "Arduino.h"
#include "canard.h"
#include "drivers/stm32/canard_stm32.h"
#include "uavcan.h"


#define CANARD_SPIN_PERIOD    500
#define PUBLISHER_PERIOD_mS   25
#define RC_NUM_CHANNELS       6
#define RC_NUM_CHANNELS_TO_PRINT    4
            
static CanardInstance g_canard;                //The library instance
static uint8_t g_canard_memory_pool[1024];     //Arena for memory allocation, used by the library
int16_t rc_pwm[RC_NUM_CHANNELS] = {0};


void CAN_HW_Init(void) {

  GPIO_InitTypeDef GPIO_InitStruct;

  // GPIO Ports Clock Enable
  __HAL_RCC_GPIOA_CLK_ENABLE();

  // CAN1 clock enable
  __HAL_RCC_CAN1_CLK_ENABLE();

  // CAN GPIO Configuration
  // PA11     ------> CAN_RX
  // PA12     ------> CAN_TX

#if defined (STM32F103x6) || defined (STM32F103xB)
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
#elif defined (STM32F303x8) || defined (STM32F303xC) || defined (STM32F303xE)
  GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF9_CAN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
#else
#error "Warning untested processor variant"
#endif

}

/*
https://kb.zubax.com/display/MAINKB/1.+Basic+tutorial

canardSTM32Init and canardSTM32ComputeCANTimings are stm32-specific
driver functions shipped with libcanard intended to simplify the CAN
peripheral setup procedure.

Important note: the libcanard's STM32 CAN driver does not use
interrupts or DMA. Its up to user to decide if CAN interrupts are
needed and to implement them if necessary.

Libcanard is a static library and does not use heap, so it needs some
memory pool for operation which the user must give it manually. That
is uint8_t g_canard_memory_pool[1024]
*/

void uavcanInit(void)
{

  CanardSTM32CANTimings timings;
  int result = canardSTM32ComputeCANTimings(HAL_RCC_GetPCLK1Freq(), 1000000, &timings);
  if (result) {
    __ASM volatile("BKPT #01");
  }
  result = canardSTM32Init(&timings, CanardSTM32IfaceModeNormal);
  if (result) {
    __ASM volatile("BKPT #01");
  }
 
  canardInit(&g_canard,                         // Uninitialized library instance
             g_canard_memory_pool,              // Raw memory chunk used for dynamic allocation
             sizeof(g_canard_memory_pool),      // Size of the above, in bytes
             onTransferReceived,                // Callback, see CanardOnTransferReception
             shouldAcceptTransfer,              // Callback, see CanardShouldAcceptTransfer
             NULL);
 
  canardSetLocalNodeID(&g_canard, 10);

}

/*
https://kb.zubax.com/display/MAINKB/1.+Basic+tutorial
Libcanard also needs two functions that must be implemented by the user:

    shouldAcceptTransfer - this callback is called every time a
    transfer is received to determine if it should be passed further
    to the library or ignored. Here we should filter out all messages
    that are not needed for our particular task.

    onTransferReceived - this callback is called every time a transfer
    is received and accepted in shouldAcceptTransfer. It is a good
    idea to put incoming data handlers here.
*/
    
bool shouldAcceptTransfer(const CanardInstance* ins,
                          uint64_t* out_data_type_signature,
                          uint16_t data_type_id,
                          CanardTransferType transfer_type,
                          uint8_t source_node_id)
{

  if ((transfer_type == CanardTransferTypeRequest) &&(data_type_id == UAVCAN_GET_NODE_INFO_DATA_TYPE_ID)) {
    *out_data_type_signature = UAVCAN_GET_NODE_INFO_DATA_TYPE_SIGNATURE;
    return true;
  }
  if (data_type_id == UAVCAN_EQUIPMENT_ESC_RAWCOMMAND_ID) {
    *out_data_type_signature = UAVCAN_EQUIPMENT_ESC_RAWCOMMAND_SIGNATURE;
    return true;
  }
  if (data_type_id == UAVCAN_PROTOCOL_PARAM_GETSET_ID) {
    *out_data_type_signature = UAVCAN_PROTOCOL_PARAM_GETSET_SIGNATURE;
    return true;
  }
  return false;

}


/*
https://kb.zubax.com/display/MAINKB/1.+Basic+tutorial

onTransferReceived - this callback is called every time a transfer
is received and accepted in shouldAcceptTransfer. It is a good
idea to put incoming data handlers here.

As should be obvious from shouldAcceptTransfer, our node will accept
only one type of transfers:

UAVCAN_GET_NODE_INFO_DATA_TYPE_ID - this is a request that the UAVCAN
GUI Tool sends to all nodes that it discovers to get some data like
name, software version, hardware version and so on from them. In fact,
this is optional, but supporting this type of service is a good idea.

We added support of UAVCAN_EQUIPMENT_ESC_RAWCOMMAND_ID, message that
contains all values needed to generate RCPWM signal from ESC setpoint
messages. The UAVCAN GUI Tool starts to broadcast these messages when
the ESC Management panel is opened. Note that other messages can also
be mapped to RCPWM outputs; for example, the actuator command messages
from the namespace uavcan.equipment.actuator.
*/

void onTransferReceived(CanardInstance* ins, CanardRxTransfer* transfer)
{

  if ((transfer->transfer_type == CanardTransferTypeRequest) && (transfer->data_type_id == UAVCAN_GET_NODE_INFO_DATA_TYPE_ID)) {
    getNodeInfoHandleCanard(transfer);
  } 

  if (transfer->data_type_id == UAVCAN_EQUIPMENT_ESC_RAWCOMMAND_ID) {
    rawcmdHandleCanard(transfer);
  }

  if (transfer->data_type_id == UAVCAN_PROTOCOL_PARAM_GETSET_ID) {
    getsetHandleCanard(transfer);
  }
    
}


void getNodeInfoHandleCanard(CanardRxTransfer* transfer)
{

  uint8_t buffer[UAVCAN_GET_NODE_INFO_RESPONSE_MAX_SIZE];
  memset(buffer,0,UAVCAN_GET_NODE_INFO_RESPONSE_MAX_SIZE);
  uint16_t len = makeNodeInfoMessage(buffer);
  canardRequestOrRespond(&g_canard,
                                      transfer->source_node_id,
                                      UAVCAN_GET_NODE_INFO_DATA_TYPE_SIGNATURE,
                                      UAVCAN_GET_NODE_INFO_DATA_TYPE_ID,
                                      &transfer->transfer_id,
                                      transfer->priority,
                                      CanardResponse,
                                      &buffer[0],
                                      (uint16_t)len);

}


/*
https://kb.zubax.com/display/MAINKB/1.+Basic+tutorial

Now it's the handler's turn, where RCPWM values are passed to the MCU timers:
*/

void rawcmdHandleCanard(CanardRxTransfer* transfer)
{
    
  int offset = 0;
  for (int i = 0; i<6; i++) {
    if (canardDecodeScalar(transfer, offset, 14, true, &rc_pwm[i])<14) { break; }
    offset += 14;
  }
  // rcpwmUpdate(ar);

}

/*
https://kb.zubax.com/display/MAINKB/1.+Basic+tutorial

For the sake of simplicity, in this example we will use only integer
numeric parameters, although UAVCAN allows you to have parameters of
just any type you want.

In this example we will have three integer parameters. It is not
necessary but it can be considered a good practice to specify the
default value and the acceptable value range for numeric parameters.
*/

param_t parameters[] =
{
  {"MGeo parameter 0",  0, 10,  20,  15},
  {"MGeo parameter 1",  1,  0, 100,  25},
  {"MGeo parameter 2",  2,  2,   8,   3},
};


/*
https://kb.zubax.com/display/MAINKB/1.+Basic+tutorial

We will also need a couple of ways to access these parameters: by
index and by name with some safety checks.
*/

inline param_t * getParamByIndex(uint16_t index)
{

  if(index >= ARRAY_SIZE(parameters))  {
    return NULL;
  }
  return &parameters[index];

}


inline param_t * getParamByName(uint8_t * name)
{

  for(uint16_t i = 0; i < ARRAY_SIZE(parameters); i++)
  {
    if(strncmp((char const*)name, (char const*)parameters[i].name,strlen((char const*)parameters[i].name)) == 0) {
      return &parameters[i];
    }
  }      
  return NULL;

}

/*
We also need to create a function that encodes an integer param to the
uavcan.protocol.param.GetSet message. It will be a little nasty, as
bit operations always are, but here it is.  canardEncodeParam

According to the UAVCAN DSDL specification, section "Dynamic arrays",
there should be a bit field (often 8 bits wide) representing the
length of the array prepending the array field. But there is one
important detail, which plays a role in this particular case. DSDL
also defines a tail array optimization, which means that in the case
when the array is the last field in the UAVCAN message, there is no
need to specify its length and it must be skipped. That is why in the
function above we did not specify the length of the parameter name

*/

uint16_t encodeParamCanard(param_t * p, uint8_t * buffer)
{

  uint8_t n     = 0;
  int offset    = 0;
  uint8_t tag   = 1;

  if(p==NULL) {   
    tag = 0;
    canardEncodeScalar(buffer, offset, 5, &n);
    offset += 5;
    canardEncodeScalar(buffer, offset,3, &tag);
    offset += 3;
        
    canardEncodeScalar(buffer, offset, 6, &n);
    offset += 6;
    canardEncodeScalar(buffer, offset,2, &tag);
    offset += 2;

    canardEncodeScalar(buffer, offset, 6, &n);
    offset += 6;
    canardEncodeScalar(buffer, offset, 2, &tag);
    offset += 2;
    buffer[offset / 8] = 0;
    return ( offset / 8 + 1 );

  }

  canardEncodeScalar(buffer, offset, 5,&n);
  offset += 5;
  canardEncodeScalar(buffer, offset, 3, &tag);
  offset += 3;
  canardEncodeScalar(buffer, offset, 64, &p->val);
  offset += 64;

  canardEncodeScalar(buffer, offset, 5, &n);
  offset += 5;
  canardEncodeScalar(buffer, offset, 3, &tag);
  offset += 3;
  canardEncodeScalar(buffer, offset, 64, &p->defval);
  offset += 64;
    
  canardEncodeScalar(buffer, offset, 6, &n);
  offset += 6;
  canardEncodeScalar(buffer, offset, 2, &tag);
  offset += 2;
  canardEncodeScalar(buffer, offset, 64, &p->max);
  offset += 64;
    
  canardEncodeScalar(buffer, offset, 6, &n);
  offset += 6;
  canardEncodeScalar(buffer, offset,2,&tag);
  offset += 2;
  canardEncodeScalar(buffer, offset,64,&p->min);
  offset += 64;
    
  memcpy(&buffer[offset / 8], p->name, strlen((char const*)p->name));
  return  (offset/8 + strlen((char const*)p->name)); 

}

/*
We should also write a handler for UAVCAN_PROTOCOL_PARAM_GETSET_ID. Here it is:
canard_getset_handle
*/

void getsetHandleCanard(CanardRxTransfer* transfer)
{

  uint16_t index = 0xFFFF;
  uint8_t tag    = 0;
  int offset     = 0;
  int64_t val    = 0;

  canardDecodeScalar(transfer, offset,  13, false, &index);
  offset += 13;
  canardDecodeScalar(transfer, offset, 3, false, &tag);
  offset += 3;

  if(tag == 1) {
    canardDecodeScalar(transfer, offset, 64, false, &val);
    offset += 64;
  }

  uint16_t n = transfer->payload_len - offset / 8 ;
  uint8_t name[16]      = "";
  for(int i = 0; i < n; i++) {
    canardDecodeScalar(transfer, offset, 8, false, &name[i]);
    offset += 8;
  }

  param_t * p = NULL;

  if(strlen((char const*)name)) {
    p = getParamByName(name);
  } else {
    p = getParamByIndex(index);
  }

  if((p)&&(tag == 1)) {
    p->val = val;
  }

  uint8_t  buffer[64] = "";
  uint16_t len = encodeParamCanard(p, buffer);
  canardRequestOrRespond(&g_canard,
			 transfer->source_node_id,
			 UAVCAN_PROTOCOL_PARAM_GETSET_SIGNATURE,
			 UAVCAN_PROTOCOL_PARAM_GETSET_ID,
			 &transfer->transfer_id,
			 transfer->priority,
			 CanardResponse,
			 &buffer[0],
			 (uint16_t)len);

}


/*
https://kb.zubax.com/display/MAINKB/1.+Basic+tutorial

As libcanard does not use any interrupts, it is up to the user to
decide when and how to receive and transmit UAVCAN messages. In this
application we will constantly poll if any message was received by the
MCU's CAN peripheral and process it. We will also poll if the library
has any new messages to transmit and manually extract them from the
library and pass them to the CAN transmitter.
*/

void sendCanard(void)
{

  const CanardCANFrame* txf = canardPeekTxQueue(&g_canard); 

  while(txf) {
    const int tx_res = canardSTM32Transmit(txf);
    if (tx_res < 0) {                 // Failure - drop the frame and report
      __ASM volatile("BKPT #01");     // TODO: handle the error properly
    }
    if(tx_res > 0) {
      canardPopTxQueue(&g_canard);
    }
    txf = canardPeekTxQueue(&g_canard); 
  }

}


void receiveCanard(void)
{

  CanardCANFrame rx_frame;
  int res = canardSTM32Receive(&rx_frame);
  if(res) {
    canardHandleRxFrame(&g_canard, &rx_frame, HAL_GetTick() * 1000);
  }    

}

/*
https://kb.zubax.com/display/MAINKB/1.+Basic+tutorial

Besides receiving UAVCAN messages, each node must also broadcast at
least one type of messages periodically - NodeStatus (once in every
100-1000 ms should be fine). So let's make a function for that.
*/  

void spinCanard(void)
{  

  static uint32_t spin_time = 0;

  if(HAL_GetTick() < spin_time + CANARD_SPIN_PERIOD) return;    // rate limiting
  
  spin_time = HAL_GetTick();
  //HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_11);
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    
  uint8_t buffer[UAVCAN_NODE_STATUS_MESSAGE_SIZE];    
  static uint8_t transfer_id = 0;                               // This variable MUST BE STATIC; refer to the libcanard documentation for the background
  makeNodeStatusMessage(buffer);  
  canardBroadcast(&g_canard, 
                  UAVCAN_NODE_STATUS_DATA_TYPE_SIGNATURE,
                  UAVCAN_NODE_STATUS_DATA_TYPE_ID,
                  &transfer_id,
                  CANARD_TRANSFER_PRIORITY_LOW,
                  buffer, 
                  UAVCAN_NODE_STATUS_MESSAGE_SIZE);             //some indication
  
}


/*
https://kb.zubax.com/display/MAINKB/1.+Basic+tutorial

We are going to use the UAVCAN message
uavcan.protocol.debug.KeyValue. The UAVCAN specification says that
float32 values can be broadcasted this way. We can use it to broadcast
some custom sensor data, ADC data, or just any named value. For the
sake of simplicity, in this tutorial we will broadcast sine
values. But, assuming that the MCU resources are quite constrained, we
will take the values from a lookup table. We will also broadcast the
second value – the current table index.

Important note. Integer and float values have very different
bit-structure. As libcanard expects a float data type for
uavcan.protocol.debug.KeyValue, it is important to give it exactly
what it wants - a float value. So, despite the fact that we have an
unsigned integer (even uint8_t) typed sine table, it is important to
provide the canardEncodeScalar function with a float type parameter.

*/

void publishCanard(void)
{  

  static uint32_t publish_time = 0;
  static int step = 0;
  if(HAL_GetTick() < publish_time + PUBLISHER_PERIOD_mS) {return;} // rate limiting
  publish_time = HAL_GetTick();

  uint8_t buffer[UAVCAN_PROTOCOL_DEBUG_KEYVALUE_MESSAGE_SIZE];
  memset(buffer,0x00,UAVCAN_PROTOCOL_DEBUG_KEYVALUE_MESSAGE_SIZE);
  step++;
  if(step == 256) {
    step = 0;
  }
  
  float val = sine_wave[step];
  static uint8_t transfer_id = 0;
  canardEncodeScalar(buffer, 0, 32, &val);
  memcpy(&buffer[4], "sin", 3);    
  canardBroadcast(&g_canard, 
                  UAVCAN_PROTOCOL_DEBUG_KEYVALUE_SIGNATURE,
                  UAVCAN_PROTOCOL_DEBUG_KEYVALUE_ID,
                  &transfer_id,
                  CANARD_TRANSFER_PRIORITY_LOW,
                  &buffer[0], 
                  7);
  memset(buffer,0x00,UAVCAN_PROTOCOL_DEBUG_KEYVALUE_MESSAGE_SIZE);

  val = step;
  canardEncodeScalar(buffer, 0, 32, &val);
  memcpy(&buffer[4], "stp", 3);  
  canardBroadcast(&g_canard, 
                  UAVCAN_PROTOCOL_DEBUG_KEYVALUE_SIGNATURE,
                  UAVCAN_PROTOCOL_DEBUG_KEYVALUE_ID,
                  &transfer_id,
                  CANARD_TRANSFER_PRIORITY_LOW,
                  &buffer[0], 
                  7);

}

/*
https://kb.zubax.com/display/MAINKB/1.+Basic+tutorial

To make a node status message we will have to compose it manually. For that we will need three values:

    Uptime in seconds.
    Node health. Our node will always be 100% healthy.
    Node mode. Our node will always be in the operational mode.

These values have to be encoded according to NodeStatus message description:

*/

void makeNodeStatusMessage(uint8_t buffer[UAVCAN_NODE_STATUS_MESSAGE_SIZE])
{

  uint8_t node_health = UAVCAN_NODE_HEALTH_OK;
  uint8_t node_mode   = UAVCAN_NODE_MODE_OPERATIONAL;
  memset(buffer, 0, UAVCAN_NODE_STATUS_MESSAGE_SIZE);
  uint32_t uptime_sec = (HAL_GetTick() / 1000);
  canardEncodeScalar(buffer,  0, 32, &uptime_sec);
  canardEncodeScalar(buffer, 32,  2, &node_health);
  canardEncodeScalar(buffer, 34,  3, &node_mode);

}


void readUniqueID(uint8_t* out_uid)
{

  for (uint8_t i = 0; i < UNIQUE_ID_LENGTH_BYTES; i++) {
    out_uid[i] = i;
  }

}

/*
https://kb.zubax.com/display/MAINKB/1.+Basic+tutorial

When the UAVCAN GUI Tool receives this message for the first time, it
will attempt to get more info about the new node, so we also have to
implement a handler that will form a GetNodeInfo response and send it
back to the requesting node (client):
*/

uint16_t makeNodeInfoMessage(uint8_t buffer[UAVCAN_GET_NODE_INFO_RESPONSE_MAX_SIZE])
{

  memset(buffer, 0, UAVCAN_GET_NODE_INFO_RESPONSE_MAX_SIZE);
  makeNodeStatusMessage(buffer);
   
  buffer[7] = APP_VERSION_MAJOR;
  buffer[8] = APP_VERSION_MINOR;
  buffer[9] = 1;                            // Optional field flags, VCS commit is set
  uint32_t u32 = GIT_HASH;
  canardEncodeScalar(buffer, 80, 32, &u32); 
    
  readUniqueID(&buffer[24]);
  const size_t name_len = strlen(APP_NODE_NAME);
  memcpy(&buffer[41], APP_NODE_NAME, name_len);
  return 41 + name_len ;

}

void printArray(int16_t arr[]) {

  char str[20];
  itoa(arr[0], str, 10);
  Serial.print("ESC Array: ");
  Serial.print(str);
  for (int i = 1; i < RC_NUM_CHANNELS_TO_PRINT; i++) {
    itoa(arr[i], str, 10);
    Serial.print(", ");
    Serial.print(str);
  }
  Serial.println();
}

void showRcpwmonUart()
{
  printArray(rc_pwm);
}

