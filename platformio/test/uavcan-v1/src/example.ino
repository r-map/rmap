/*
Copyright (C) 2020  Paolo Paruno <p.patruno@iperbole.bologna.it>
authors:
Paolo Patruno <p.patruno@iperbole.bologna.it>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Freeg Software Foundation; either version 2 of 
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Arduino.h"
#include "stm32_def.h"
#include "canard.h"
#include "canard_dsdl.h"
#include "bxcan.h"

static const uint16_t HeartbeatSubjectID         = 7509U;
static const uint16_t UltrasoundMessageSubjectID = 1610U;
static const uint16_t RegisterAccessServiceID    = 84U;

CanardInstance canard;
HardwareSerial Serial2(PA3, PA2);  //uart2
unsigned long int  next;
unsigned long int  nextrpc;

static void* canardAllocate(CanardInstance* const ins, const size_t amount)
{
    (void) ins;
    return malloc(amount);
}

static void canardFree(CanardInstance* const ins, void* const pointer)
{
    (void) ins;
    free(pointer);
}

static void publishHeartbeat(CanardInstance* const canard, const uint32_t uptime)
{
  static CanardTransferID transfer_id=0;
  uint8_t payload[7] ={0};
  
  canardDSDLSetUxx(payload,  0,  uptime, 32);
  canardDSDLSetUxx(payload, 32,  0,       8);
  canardDSDLSetUxx(payload, 40,  0,       8);
  canardDSDLSetUxx(payload, 48,  9,       8);
  
  const CanardTransfer transfer =
    {
     .timestamp_usec = micros()+2000000UL,
     .priority       = CanardPriorityNominal,
     .transfer_kind  = CanardTransferKindMessage,
     .port_id        = HeartbeatSubjectID,
     .remote_node_id = CANARD_NODE_ID_UNSET,
     .transfer_id    = transfer_id,
     .payload_size   = 7, //sizeof(payload),
     .payload        = &payload[0],
    };
  ++transfer_id;
  if(canardTxPush(canard, &transfer) < 0 )
    {
      // An error has occurred: either an argument is invalid or we've ran out of memory.
      Serial2.println("Error canardTxPush");
    }
}


static void publishUltrasoundMessage(CanardInstance* const canard)
{
  static CanardTransferID transfer_id=0;

  uint8_t payload[4] = {0, 0, 0, 0};
  //Serilize the distance
  canardDSDLSetF32(payload, 0, 123.456);
    
  const CanardTransfer    transfer =
    {
     .timestamp_usec = micros()+2000000UL,
     .priority       = CanardPriorityNominal,
     .transfer_kind  = CanardTransferKindMessage,
     .port_id        = UltrasoundMessageSubjectID,
     .remote_node_id = CANARD_NODE_ID_UNSET,
     .transfer_id    = transfer_id,
     .payload_size   = 4, //sizeof(payload),
     .payload        = &payload[0],
    };
  ++transfer_id;
  if (canardTxPush(canard, &transfer) < 0)
    {
      // An error has occurred: either an argument is invalid or we've ran out of memory.
      Serial2.println("Error canardTxPush");
    }

}


static void requestRpc(CanardInstance* const canard)
{
  static CanardTransferID transfer_id=0;

  #ifdef BOARD1
  CanardNodeID remote_node_id        = (CanardNodeID) 11;
  #endif

  #ifdef BOARD2
  CanardNodeID remote_node_id        = (CanardNodeID) 10;
  #endif

  const CanardTransfer    transfer =
    {
     .timestamp_usec = micros()+2000000UL,
     .priority       = CanardPriorityNominal,
     .transfer_kind  = CanardTransferKindRequest,
     .port_id        = RegisterAccessServiceID,
     .remote_node_id = remote_node_id,
     .transfer_id    = transfer_id,
     .payload_size   = 0,
     .payload        = NULL,
    };
  ++transfer_id;
  if (canardTxPush(canard, &transfer) < 0)
    {
      // An error has occurred: either an argument is invalid or we've ran out of memory.
      Serial2.println("Error canardTxPush");
    }
}



static void handleRequestTransfer(CanardInstance* const canard, const CanardTransfer* const request_transfer)
{
  // Send the response back. Make sure to re-use the same priority and transfer-ID.
  uint8_t payload[4] = {0, 0, 0, 0};
  //Serialize the temperature
  canardDSDLSetF32(payload, 0, 273.15);

  CanardTransfer response_transfer = *request_transfer;
  response_transfer.timestamp_usec = micros()+2000000UL,
  response_transfer.transfer_kind  = CanardTransferKindResponse;
  response_transfer.payload_size = 4; // sizeof(payload)
  response_transfer.payload = &payload[0];   

  if (canardTxPush(canard, &response_transfer) < 0)
    {
      // An error has occurred: either an argument is invalid or we've ran out of memory.
      Serial2.println("Error canardTxPush");
    }
}

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

  BxCANTimings timings;
  bool result = bxCANComputeTimings(HAL_RCC_GetPCLK1Freq(), 250000, &timings);
  if (!result) {
    __ASM volatile("BKPT #01");
  }

  result = bxCANConfigure(0, timings, false);
  if (!result) {
    __ASM volatile("BKPT #01");
  }
  
}


void sendQueued(void){
  // Transmit pending frames.
  const CanardFrame* txf = canardTxPeek(&canard);
  while (txf != NULL) {
    if (bxCANPush(0,
		  micros(),
		  txf->timestamp_usec,
		  txf->extended_can_id,
		  txf->payload_size,
		  txf->payload)){
      
      canardTxPop(&canard);
      //canard.memory_free(&canard,(void*) txf);
      free((void*) txf);
      txf = canardTxPeek(&canard);
    }
  }
}

void processReceived(){
  // Process received frames, if any.
  uint32_t  out_extended_can_id;
  size_t    out_payload_size;
  uint8_t   out_payload[8];

  while (bxCANPop(0,
		  &out_extended_can_id,
		  &out_payload_size,
		  out_payload)) {
    /*
    Serial2.print("packet received: ");
    Serial2.print(out_extended_can_id);
    Serial2.print(" : ");
    Serial2.println(out_payload_size);
    */
    CanardFrame rxf=
      {
       .extended_can_id=out_extended_can_id,
       .payload_size=out_payload_size,
       .payload=&out_payload[0]
      };

    CanardTransfer transfer;
    const int8_t result = canardRxAccept(&canard, &rxf, 0, &transfer);
    if ( result == 1 ){
      /*
      Serial2.print("transfer kind and port_id: ");
      Serial2.print(transfer.transfer_kind);
      Serial2.print(" : ");
      Serial2.println(transfer.port_id);
      */
      if ((transfer.transfer_kind == CanardTransferKindMessage) &&
	  (transfer.port_id == HeartbeatSubjectID)) {
	uint8_t  mode   = canardDSDLGetU8((const uint8_t*)transfer.payload,  transfer.payload_size, 40,  8);
	uint32_t uptime = canardDSDLGetU32((const uint8_t*)transfer.payload, transfer.payload_size,  0, 32);
	uint8_t  vssc   = canardDSDLGetU32((const uint8_t*)transfer.payload, transfer.payload_size, 48,  8);
	uint8_t  health = canardDSDLGetU8((const uint8_t*)transfer.payload,  transfer.payload_size, 32,  8);

	Serial2.print("Heartbeat -> ");
	Serial2.print("uptime: ");
	Serial2.print(uptime);
	Serial2.print(" mode: ");
	Serial2.print(mode);
	Serial2.print(" health: ");
	Serial2.print(health);
	Serial2.print(" vssc: ");
	Serial2.println(vssc);

      } else if ((transfer.transfer_kind == CanardTransferKindMessage) &&
		 (transfer.port_id == UltrasoundMessageSubjectID)) {
	// Deserialize distance from ultrasound message
	CanardDSDLFloat32 distance = canardDSDLGetF32((const uint8_t*)transfer.payload, transfer.payload_size, 0);          
	
	Serial2.print("UltrasoundMessage -> ");
	Serial2.print("distance: ");
	Serial2.println(distance);

      } else if ((transfer.transfer_kind == CanardTransferKindRequest) &&
		 (transfer.port_id == RegisterAccessServiceID)) {
	handleRequestTransfer(&canard, &transfer);
      } else if ((transfer.transfer_kind == CanardTransferKindResponse) &&
		 (transfer.port_id == RegisterAccessServiceID)) {
	// Deserialize distance from rpc response
	CanardDSDLFloat32 temperature = canardDSDLGetF32((const uint8_t*)transfer.payload, transfer.payload_size, 0);          
	Serial2.print("RPC Message -> ");
	Serial2.print("temperature: ");
	Serial2.println(temperature);
      } else {
	Serial2.println("transfer kind and port_id not managed");
      }
      //canard.memory_free(&canard,(void*) transfer.payload);
      free((void*) transfer.payload);
    }else if ( result < 0 ) {
      Serial2.println("An error has occurred: either an argument is invalid or we've ran out of memory");
    } else {
      Serial2.println("The received frame is either invalid or it's a non-last frame of a multi-frame transfer.");
    }
  }
}


void setup(void) {

  Serial2.begin(115200);
  Serial2.print("Initializing...");
  Serial2.println(HAL_RCC_GetHCLKFreq());

  // initialize digital pins
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  CAN_HW_Init();
 
  // Initialize the node with a static node-ID.
  canard = canardInit(&canardAllocate, &canardFree);
  canard.mtu_bytes      = CANARD_MTU_CAN_CLASSIC;  // Do not use CAN FD

  #ifdef BOARD1
  canard.node_id        = (CanardNodeID) 10;
  #endif

  #ifdef BOARD2
  canard.node_id        = (CanardNodeID) 11;
  #endif
 
  // Configure the library to listen for register access service requests.
  static CanardRxSubscription heartbeat_subscription;
  (void) canardRxSubscribe(&canard, // Subscribe to messages uavcan.node.Heartbeat.
			   CanardTransferKindMessage,
			   HeartbeatSubjectID, // The fixed Subject-ID of the Heartbeat message type (see DSDL definition).
			   12,                  // The maximum payload size (max DSDL object size) from the DSDL definition.
			   CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
			   &heartbeat_subscription);

  // Ultrasound subscription
  static CanardRxSubscription ultrasound_subscription;
  (void)canardRxSubscribe(&canard,
			  CanardTransferKindMessage,
			  UltrasoundMessageSubjectID,
			  10,                  
			  CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
			  &ultrasound_subscription);

  // rpc subscriptions
  static CanardRxSubscription rpc_request_subscription;
  (void)canardRxSubscribe(&canard,
			  CanardTransferKindRequest,
			  RegisterAccessServiceID,  
			  10,                  
			  CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
			  &rpc_request_subscription);

  static CanardRxSubscription rpc_response_subscription;
  (void)canardRxSubscribe(&canard,
			  CanardTransferKindResponse,
			  RegisterAccessServiceID,  
			  10,                  
			  CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
			  &rpc_response_subscription);

  next = millis() + 1000;
  nextrpc = millis() + 2500;
}

// The main loop: publish messages and process service requests.
void loop(void) {  
  if (next <= millis()) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    /*
    Serial2.print("It's time to send ");
    Serial2.print(millis());
    Serial2.print(":");
    Serial2.println(next);
    */
    publishUltrasoundMessage(&canard);
    publishHeartbeat(&canard, millis());
    next = next + 1000;
  }

  sendQueued();
  processReceived();
  
  if (nextrpc <= millis()) {
    /*    
    Serial2.print("It's time for rpc ");
    Serial2.print(millis());
    Serial2.print(":");
    Serial2.println(nextrpc);
    */
    requestRpc(&canard);
    nextrpc = nextrpc + 3000;
  }
  
  sendQueued();
  processReceived();
  
}
