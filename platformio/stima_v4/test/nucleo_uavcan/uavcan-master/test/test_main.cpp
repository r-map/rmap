#include <Arduino.h>
#include <unity.h>

#include "bxcan.h"
#include "canard.h"
#include "canard_dsdl.h"
#include "stm32_def.h"

//********** FUNCTION DECLARATIONS **********
static void* canardAllocate(CanardInstance* const ins, const size_t amount);
static void canardFree(CanardInstance* const ins, void* const pointer);
void test_can_init(void);
void test_can_filter_init(void);
void test_can_start(void);
void test_create_rx_subscription(void);
void test_canard_tx_push(void);
void test_queue_is_empty(void);
void test_canard_rx_accept(void);
void test_rx_data_is_equal_tx_data(void);
//*******************************************

//********** GLOBAL VARIABLES/CONSTANTS **********
// Definition CAN structure variable
CAN_HandleTypeDef CAN_Handle;
// Canard instance initialization
CanardInstance canard = canardInit(&canardAllocate, &canardFree);
// UAVCAN transfer model (either incoming or outgoing)
CanardTransfer transfer;
// Data to transfer
CanardDSDLFloat32 data = 123.45;
// Configure the library to listen for register access service requests
static CanardRxSubscription testSubscription;
// Port ID of subscription
static const CanardPortID testSubId = 1000U;
//************************************************
aaa;

static void* canardAllocate(CanardInstance* const ins, const size_t amount) {
    (void)ins;
    return malloc(amount);
}

static void canardFree(CanardInstance* const ins, void* const pointer) {
    (void)ins;
    free(pointer);
}

void test_can_init() {
    // Definition GPIO structure variable
    GPIO_InitTypeDef GPIO_InitStruct;

    // GPIO Ports clock enable
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // CAN1 clock enable
    __HAL_RCC_CAN1_CLK_ENABLE();

#if defined(STM32L452xx)
    // Mapping GPIO for CAN
    /* Configure CAN pin: RX */
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    /* Configure CAN pin: TX */
    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
#else
#error "Warning untested processor variant"
#endif

    // CAN cell initialization - BAUD RATE = (80M / (SJW + TS1 + TS2)) / Prescaler = 1 Mbit/s
    CAN_Handle.Instance = CAN1;
    CAN_Handle.Init.Prescaler = 5;
    CAN_Handle.Init.Mode = CAN_MODE_LOOPBACK;
    CAN_Handle.Init.SyncJumpWidth = CAN_SJW_1TQ;
    CAN_Handle.Init.TimeSeg1 = CAN_BS1_13TQ;
    CAN_Handle.Init.TimeSeg2 = CAN_BS2_2TQ;
    CAN_Handle.Init.TimeTriggeredMode = DISABLE;
    CAN_Handle.Init.AutoBusOff = DISABLE;
    CAN_Handle.Init.AutoWakeUp = DISABLE;
    CAN_Handle.Init.AutoRetransmission = DISABLE;
    CAN_Handle.Init.ReceiveFifoLocked = DISABLE;
    CAN_Handle.Init.TransmitFifoPriority = DISABLE;

    // Check error initialization CAN
    TEST_ASSERT_EQUAL(HAL_CAN_Init(&CAN_Handle), HAL_OK);
}

void test_can_filter_init() {
    // Definition CAN filter structure variable
    CAN_FilterTypeDef CAN_FilterInitStruct;

    // CAN filter initialization
    CAN_FilterInitStruct.FilterIdHigh = 0x0000;
    CAN_FilterInitStruct.FilterIdLow = 0x0000;
    CAN_FilterInitStruct.FilterMaskIdHigh = 0x0000;
    CAN_FilterInitStruct.FilterMaskIdLow = 0x0000;
    CAN_FilterInitStruct.FilterFIFOAssignment = 0;
    CAN_FilterInitStruct.FilterBank = 0;
    CAN_FilterInitStruct.FilterMode = CAN_FILTERMODE_IDMASK;
    CAN_FilterInitStruct.FilterScale = CAN_FILTERSCALE_32BIT;
    CAN_FilterInitStruct.FilterActivation = ENABLE;

    // Check error initalization CAN filter
    TEST_ASSERT_EQUAL(HAL_CAN_ConfigFilter(&CAN_Handle, &CAN_FilterInitStruct), HAL_OK);
}

void test_can_start() {
    // Check error starting CAN
    TEST_ASSERT_EQUAL(HAL_CAN_Start(&CAN_Handle), HAL_OK);
}

void test_create_rx_subscription() {
    // Assign canard properties
    canard.mtu_bytes = CANARD_MTU_CAN_CLASSIC;
    canard.node_id = (CanardNodeID)10;

    // Create a RX subscription
    TEST_ASSERT_NOT_EQUAL(canardRxSubscribe(&canard,
                                            CanardTransferKindMessage,
                                            testSubId,
                                            12,
                                            CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
                                            &testSubscription),
                          -CANARD_ERROR_INVALID_ARGUMENT);
}

void test_canard_tx_push() {
    // Defines a transmission transfer ID
    static CanardTransferID txTransferId = 0;
    // Payload of TX frame
    uint8_t payloadTx[4] = {0, 0, 0, 0};

    // Serialize the data to frame
    canardDSDLSetF32(payloadTx, 0, data);

    // UAVCAN transfer model (either incoming or outgoing)
    const CanardTransfer transfer = {
        .timestamp_usec = micros() + 2000000UL,
        .priority = CanardPriorityNominal,
        .transfer_kind = CanardTransferKindMessage,
        .port_id = testSubId,
        .remote_node_id = CANARD_NODE_ID_UNSET,
        .transfer_id = txTransferId,
        .payload_size = 4,
        .payload = &payloadTx[0],
    };

    // Increment the transfer ID
    ++txTransferId;

    /*
        Serializes a transfer into a sequence of transport frames and inserts them into the prioritized
        transmission queue at the appropriate position
    */
    TEST_ASSERT_NOT_EQUAL(canardTxPush(&canard, &transfer), -CANARD_ERROR_INVALID_ARGUMENT);
}

void test_queue_is_empty() {
    /*
        The function accesses the top element of the prioritized transmission queue. The queue itself is not modified.
        The application should invoke this function to collect the transport frames of serialized transfers
        pushed into the prioritized transmission queue by canardTxPush().
    */
    const CanardFrame* txf = canardTxPeek(&canard);

    while (txf != NULL) {
        if (bxCANPush(0,
                      micros(),
                      txf->timestamp_usec,
                      txf->extended_can_id,
                      txf->payload_size,
                      txf->payload)) {
            /*
                Transfers the ownership of the top element of the prioritized transmission queue to the application.
                The application should invoke this function to remove the top element from the prioritized transmission queue
            */
            canardTxPop(&canard);

            // Deallocates the block of memory previously allocated
            free((void*)txf);

            // Accesses the top element of the prioritized transmission queue and saves into CAN data frame with an extended 29-bit ID
            txf = canardTxPeek(&canard);
        }
    }

    TEST_ASSERT_NULL(txf);
}

void test_canard_rx_accept() {
    // Extended ID of frame received
    uint32_t rxExtendedCanId;
    // Payload of RX frame
    uint8_t payloadRx[8];
    // Size of payload of TX frame
    size_t payloadRxSize;

    // Extract one frame from the RX FIFOs. FIFO0 checked first
    while (bxCANPop(0,
                    &rxExtendedCanId,
                    &payloadRxSize,
                    payloadRx)) {
        // CAN data frame with an extended 29-bit ID
        CanardFrame rxf = {
            .extended_can_id = rxExtendedCanId,
            .payload_size = payloadRxSize,
            .payload = &payloadRx[0]};

        /*
            This function implements the transfer reassembly logic. It accepts a transport frame, locates the appropriate
            subscription state, and, if found, updates it
        */
        TEST_ASSERT_EQUAL(canardRxAccept(&canard, &rxf, 0, &transfer), 1);
    }
}

void test_rx_data_is_equal_tx_data() {
    // Check if transfer kind and port id of received frame are the same of transmitted frame
    if ((transfer.transfer_kind == CanardTransferKindMessage) &&
        (transfer.port_id == testSubId)) {
        // Deserialize data from frame
        TEST_ASSERT_EQUAL(canardDSDLGetF32((const uint8_t*)transfer.payload, transfer.payload_size, 0), data);
    }

    // Deallocates the block of memory previously allocated
    free((void*)transfer.payload);
}

void setup() {
    UNITY_BEGIN();

    delay(1000);

    RUN_TEST(test_can_init);
    RUN_TEST(test_can_filter_init);
    RUN_TEST(test_can_start);
    RUN_TEST(test_create_rx_subscription);
    RUN_TEST(test_canard_tx_push);
    RUN_TEST(test_queue_is_empty);
    RUN_TEST(test_canard_rx_accept);
    RUN_TEST(test_rx_data_is_equal_tx_data);

    UNITY_END();
}

void loop() {
}