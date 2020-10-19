/// This is a trivial demonstration of the runtime subject-ID assignment logic via the standard register interface.
/// This node implements a server for uavcan.register.Access.1 and two publishers: the heartbeat publisher using its
/// fixed subject-ID, and a custom subject of type uavcan.primitive.String.1 via a runtime-assigned ID.
/// DSDL auto-generation is not used in this example -- everything is (de-)serialized manually.
///
/// This node is not PnP -- the node-ID has to be assigned manually when starting the application along with the
/// bxcan interface name. A PnP allocator can be implemented trivially in just a few lines of code but I decided
/// that it would distract the reader from the main demonstrated behavior, which is runtime subject-ID assignment.
///
/// This software is released under Creative Commons CC0.
/// Author: Pavel Kirienko <pavel.kirienko@zubax.com>

#include "Arduino.h"
#include "stm32_def.h"
#include "canard.h"
#include "bxcan.h"

static const uint16_t HeartbeatSubjectID      = 7509U;
static const uint16_t RegisterAccessServiceID = 384;
unsigned long int  next;

/// Should match the wildcard "uavcan.pub.*" for publishers. Likewise, "uavcan.sub.*" for subscribers.
static const char MeasurementSubjectIDRegisterName[] = "uavcan.pub.measurement";

static uint16_t g_measurement_subject_id = UINT16_MAX;

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

static void publishMeasurement(CanardInstance* const canard)
{
    // Do not publish messages until the subject-ID is configured.
    // A real node would store the value in a non-volatile memory; we do not emulate this behavior in this demo.
    if (g_measurement_subject_id <= CANARD_SUBJECT_ID_MAX)
    {
        static CanardTransferID transfer_id=0;
        const CanardTransfer    transfer =
	  {
	   .timestamp_usec = micros()+2000000UL,
	   .priority       = CanardPriorityNominal,
	   .transfer_kind  = CanardTransferKindMessage,
	   .port_id        = g_measurement_subject_id,
	   .remote_node_id = CANARD_NODE_ID_UNSET,
	   .transfer_id    = transfer_id,
	   .payload_size   = 13,
            .payload        = "\x0B\x00"
	   "So it goes.",
	  };
        ++transfer_id;
        (void) canardTxPush(canard, &transfer);
    }
}

static void publishHeartbeat(CanardInstance* const canard, const uint32_t uptime)
{
    static CanardTransferID transfer_id=0;
    const uint8_t payload[7] =
      {
       (uint8_t)(uptime >> 0U),
       (uint8_t)(uptime >> 8U),
       (uint8_t)(uptime >> 16U),
       (uint8_t)(uptime >> 24U),
       0,   // health 2 bit  // mode   3 bit // vendor_specific_status_code  19 bit
       0,
       0,
      };
    const CanardTransfer transfer =
      {
       //       .timestamp_usec = micros()+2000000UL,
       .timestamp_usec = 0,
       .priority       = CanardPriorityNominal,
       .transfer_kind  = CanardTransferKindMessage,
       .port_id        = HeartbeatSubjectID,
       .remote_node_id = CANARD_NODE_ID_UNSET,
       .transfer_id    = transfer_id,
       .payload_size   = sizeof(payload),
       .payload        = &payload[0],
      };
    ++transfer_id;
    (void) canardTxPush(canard, &transfer);
}

static void handleRegisterAccess(CanardInstance* const canard, const CanardTransfer* const request_transfer)
{
  // The following few lines is a basic hand-crafted special-purpose deserializer.
  const uint8_t* pld      = (uint8_t*)request_transfer->payload;
  size_t         pld_size = request_transfer->payload_size;
  const uint8_t  name_len = *pld++;
  pld_size--;
  const uint8_t* const name = pld;
  if (name_len >= pld_size)
    {
      return;  // Bad serialization
    }
  pld += name_len;
  pld_size -= name_len;
  const uint8_t value_tag = *pld++;
  pld_size--;
  uint8_t response[313] = {0};  // All-zero response encodes an empty Value, which represents an invalid register.
  size_t  response_size = 9;    // timestamp [7] + flags [1] + value tag [1] + empty value [0]
  
  // Match the name against existing registers.
  // If none match, respond with an empty value, thereby indicating the lack of such register.
  if ((name_len == sizeof(MeasurementSubjectIDRegisterName) - 1) &&
      (0 == memcmp(MeasurementSubjectIDRegisterName, name, name_len)))
    {
      // Write the register (if the type matches).
      if ((value_tag == 10) && (pld_size >= 3) && (*pld == 1))  // Natural16[1]
        {
	  ++pld;
	  uint16_t value = *pld++;
	  value |= *pld++ << 8;
	  pld_size -= 3;
	  g_measurement_subject_id = value;
        }
        // Then read it and return the result.
      response[7]   = 0b00000001;  // Flags: mutable, not persistent (although normally it should be persistent)
      response[8]   = 10;          // Type: Natural16
      response[9]   = 1;           // One element
      response[10]  = (uint8_t)(g_measurement_subject_id);
      response[11]  = (uint8_t)(g_measurement_subject_id >> 8);
      response_size = 12;
    }
  
  // Send the response back. Make sure to re-use the same priority and transfer-ID.
  CanardTransfer response_transfer = *request_transfer;
  response_transfer.transfer_kind  = CanardTransferKindResponse;
  response_transfer.payload_size   = response_size;
  response_transfer.payload        = response;
  
  (void) canardTxPush(canard, &response_transfer);
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


CanardInstance canard;

void setup(void) {

  Serial.begin(115200);
  Serial.print("Initializing...");
  Serial.println(HAL_RCC_GetHCLKFreq());

  // initialize digital pins
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  CAN_HW_Init();
  
  // Initialize the node with a static node-ID as specified in the command-line arguments.
  canard = canardInit(&canardAllocate, &canardFree);
  canard.mtu_bytes      = CANARD_MTU_CAN_CLASSIC;  // Do not use CAN FD to enhance compatibility.
  canard.node_id        = (CanardNodeID) 10;
  
  // Configure the library to listen for register access service requests.
  CanardRxSubscription srv_register_access;
  (void) canardRxSubscribe(&canard,
			   CanardTransferKindRequest,
			   RegisterAccessServiceID,
			   1024U,  // Larger buffers are OK.
			   CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
			   &srv_register_access);
  
  next = millis() + 1000;
}


void loop(void) {  
  // The main loop: publish messages and process service requests.
  if (next < millis()) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    publishMeasurement(&canard);
    publishHeartbeat(&canard, next);
    next = next + 1000;
  }
  
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
      free((void*) txf);
      txf = canardTxPeek(&canard);
    }
  }
  
  // Process received frames, if any.
  uint32_t  out_extended_can_id;
  size_t    out_payload_size;
  uint8_t   out_payload[8];

  while (bxCANPop(0,
		  &out_extended_can_id,
		  &out_payload_size,
		  out_payload)) {
    CanardFrame rxf=
      {
       .extended_can_id=out_extended_can_id,
       .payload_size=out_payload_size,
       .payload=&out_payload[0]
      };

    CanardTransfer transfer;
    if (canardRxAccept(&canard, &rxf, 0, &transfer)) {
      if ((transfer.transfer_kind == CanardTransferKindRequest) &&
	  (transfer.port_id == RegisterAccessServiceID)) {
	handleRegisterAccess(&canard, &transfer);
      }
      free((void*) transfer.payload);
    }
  }
}
