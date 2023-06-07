#ifdef TEST_UAVCAN

#include <Arduino.h>
#include <Unity.h>
#include <assert.h>
#include <canard.h>
#include <o1heap.h>
#include <uavcan/_register/Access_1_0.h>
#include <uavcan/node/Heartbeat_1_0.h>

#include "bxcan.h"
#include "module_config.hpp"

//****************************************************************************************************
//******************************** FUNCTION DECLARATIONS *********************************************
//****************************************************************************************************
void test_can_hw_init(void);
void test_can_filter_init(void);
void test_setup_speed_can_init(void);
void test_bxcan_init(void);
void test_can_start(void);
void test_create_rx_subscription(void);
void test_canard_tx_push(void);
//****************************************************************************************************

//****************************************************************************************************
//************************************* STATE STRUCTURE **********************************************
//****************************************************************************************************
typedef struct State {
    CanardMicrosecond started_at;
    uint32_t lastMicrosecond;
    uint64_t currentMicrosecond;

    O1HeapInstance* heap;
    CanardInstance canard;
    CanardTxQueue canard_tx_queues[CAN_REDUNDANCY_FACTOR];

    // Gestione master/server e funzioni attive in RX messaggi
    // Master Locale
    struct {
        // Time stamp
        struct {
            CanardMicrosecond previous_tx_real;
            bool enable_immediate_tx_real;
        } timestamp;
        // File upload (node_id può essere differente dal master, es. yakut e lo riporto)
        // AGgiungo file_server_node_id
        struct
        {
            uint8_t server_node_id;
            char filename[FILE_NAME_SIZE_MAX];
            bool is_firmware;
            bool updating;
            bool updating_eof;
            bool updating_run;
            byte updating_retry;
            uint64_t offset;
            uint64_t timeout_us;  // Time command Remoto x Verifica deadLine Request
            bool is_pending;      // Funzione in pending (inviato, attesa risposta o timeout)
            bool is_timeout;      // Funzione in timeout (mancata risposta)
        } file;
    } master;

    // Stato dei nodi slave e servizi gestiti collegati. Possibile lettura dai registri e gestione automatica
    struct
    {
        // Parametri Statici da leggere da registri o altro. Comunque inviati dal Master
        // node_id è l'indirizzo nodo remoto sui cui gestire i servizi
        // node_type identifica il tipo di nodo per sapere che tipologia di gestione viene effettuata
        // service è il port id sul node_id remoto che risponde ai servizi relativi di request
        // publisher è il subject id sul node_id remoto che pubblica i dati quando attivato
        // Gli altri parametri sono dinamicamente gestiti durante il funzionamento del programma
        uint8_t node_id;
        uint8_t node_type;
        // Nodo OnLine / OffLine
        bool is_online;
        struct {
            uint8_t state;        // Vendor specific code NodeSlave State
            uint8_t healt;        // Uavcan healt_state remoto
            uint64_t timeout_us;  // Time heartbeat Remoto x Verifica OffLine
        } heartbeat;
        struct {
            // Flag di assegnamento PNP (se TRUE, da scrivere in ROM NON VOLATILE / REGISTER)
            // Il nodo è stato assegnato e il pnp non deve essere eseguito per quel modulo
            // se false il pnp è eseguito fino a quando non rilevo un modulo compatibile
            // con la richiesta (MODULE_TH, MODULE_RAIN ecc...) appena ne trovo uno
            // Heartbeat pubblicati Rx<-(Stato nodo remoto)
            // Questo permette l'assegmaneto PNP di tutti i moduli in sequenza (default)
            // O l'aggiunta successiva previa modifica della configurazione remota
            bool is_assigned;  // flag resettato (PNP in funzione)
        } pnp;
        // Comandi inviati da locale Tx->(Comando + Param) Rx<-(Risposta)
        struct {
            uint8_t response;          // Stato di risposta ai comandi nodo
            uint64_t timeout_us;       // Time command Remoto x Verifica deadLine Request
            bool is_pending;           // Funzione in pending (inviato, attesa risposta o timeout)
            bool is_timeout;           // Funzione in timeout (mancata risposta)
            uint8_t next_transfer_id;  // Transfer ID associato alla funzione
        } command;
        // Accesso ai registri inviati da locale Tx->(Registro + Value) Rx<-(Risposta)
        struct {
            uavcan_register_Value_1_0 response;  // Valore in risposta al registro x Set (R/W)
            uint64_t timeout_us;                 // Time command Remoto x Verifica deadLine Request
            bool is_pending;                     // Funzione in pending (inviato, attesa risposta o timeout)
            bool is_timeout;                     // Funzione in timeout (mancata risposta)
            uint8_t next_transfer_id;            // Transfer ID associato alla funzione
        } register_access;
        // Accesso ai dati dello slave in servizio Tx->(Funzione) Rx<-(Dato + Stato)
        // Puntatore alla struttura dati relativa es. -> rmap_module_TH_1_0 ecc...
        struct {
            CanardPortID port_id;      // Porta del servizio dati correlato
            void* module;              // Dati e stato di risposta ai dati nodo
            uint64_t timeout_us;       // Time getData Remoto x Verifica deadline Request
            bool is_pending;           // Funzione in pending (inviato, attesa risposta o timeout)
            bool is_timeout;           // Funzione in timeout (mancata risposta)
            uint8_t next_transfer_id;  // Transfer ID associato alla funzione
        } rmap_service;
        // Nome file(locale) per aggiornamento file remoto e relativo stato di funzionamento
        struct {
            char filename[FILE_NAME_SIZE_MAX];
            bool is_firmware;     // Comunico se file in TX è un Firmware o altro
            uint8_t state;        // Stato del file transfer (gestione switch interno)
            uint64_t timeout_us;  // Time command Remoto x Verifica deadLine Request File
            bool is_pending;      // Funzione in pending (inviato, attesa risposta o timeout)
            bool is_timeout;      // Funzione in timeout (mancata risposta)
        } file;
// Pubblicazione dei dati autonoma (struttura dati come per servizio)
#ifdef USE_SUB_PUBLISH_SLAVE_DATA
        struct {
            CanardPortID subject_id;  // Suject id associato alla pubblicazione
            uint16_t data_count;      // Conteggio pubblicazioni remote autonome (SOLO x TEST)
        } publisher;
#endif
    } slave[MAX_NODE_CONNECT];

    // Abilitazione delle pubblicazioni falcoltative sulla rete (ON/OFF a richiesta)
    struct
    {
        bool port_list;
    } publisher_enabled;

    // Tranfer ID (CAN Interfaccia ID -> uint8) servizi attivi del modulo locale
    struct
    {
        uint8_t uavcan_node_heartbeat;
        uint8_t uavcan_node_port_list;
        uint8_t uavcan_file_read_data;
        uint8_t uavcan_time_synchronization;
    } next_transfer_id;

    // Flag di state
    struct
    {
        bool g_restart_required;  // Forzatura reboot del nodo
    } flag;
} State;
//****************************************************************************************************

//****************************************************************************************************
//********************************** GLOBAL VARIABLES/CONSTANTS **************************************
//****************************************************************************************************
// Definition CAN structure variable
CAN_HandleTypeDef CAN_Handle;
BxCANTimings timings;
State state = {0};
//****************************************************************************************************

static void* canardAllocate(CanardInstance* const ins, const size_t amount) {
    O1HeapInstance* const heap = ((State*)ins->user_reference)->heap;
    assert(o1heapDoInvariantsHold(heap));
    return o1heapAllocate(heap, amount);
}

static void canardFree(CanardInstance* const ins, void* const pointer) {
    O1HeapInstance* const heap = ((State*)ins->user_reference)->heap;
    o1heapFree(heap, pointer);
}

void memoryAllocation(void) {
    // A simple node like this one typically does not require more than 8 KiB of heap and 4 KiB of stack.
    _Alignas(O1HEAP_ALIGNMENT) static uint8_t heap_arena[1024 * 16] = {0};
    state.heap = o1heapInit(heap_arena, sizeof(heap_arena));
    // The libcanard instance requires the allocator for managing protocol states.
    state.canard = canardInit(&canardAllocate, &canardFree);
    // Make the state reachable from the canard instance.
    state.canard.user_reference = &state;
}

/**
 * @brief Test: check if the CAN hardware has been configured correctly
 *
 */
void test_can_hw_init() {
    // Definition GPIO structure variable
    GPIO_InitTypeDef GPIO_InitStruct;

    // GPIO Ports clock enable
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // CAN1 clock enable
    __HAL_RCC_CAN1_CLK_ENABLE();

#if defined(STM32L496xx)
    // Mapping GPIO for CAN
    /* Configure CAN pin: RX */
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
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

    // Setup CAN Istance Basic
    CAN_Handle.Instance = CAN1;
    CAN_Handle.Init.Mode = CAN_MODE_NORMAL;
    CAN_Handle.Init.TimeTriggeredMode = DISABLE;
    CAN_Handle.Init.AutoBusOff = DISABLE;
    CAN_Handle.Init.AutoWakeUp = DISABLE;
    CAN_Handle.Init.AutoRetransmission = DISABLE;
    CAN_Handle.Init.ReceiveFifoLocked = DISABLE;
    CAN_Handle.Init.TransmitFifoPriority = DISABLE;

    TEST_ASSERT_EQUAL(HAL_OK, HAL_CAN_Init(&CAN_Handle));
}

/**
 * @brief Test: check if the CAN filter has been configured correctly
 *
 */
void test_can_filter_init() {
    // Definition CAN filter structure variable
    CAN_FilterTypeDef CAN_FilterInitStruct;

    // CAN filter basic initialization
    CAN_FilterInitStruct.FilterIdHigh = 0x0000;
    CAN_FilterInitStruct.FilterIdLow = 0x0000;
    CAN_FilterInitStruct.FilterMaskIdHigh = 0x0000;
    CAN_FilterInitStruct.FilterMaskIdLow = 0x0000;
    CAN_FilterInitStruct.FilterFIFOAssignment = 0;
    CAN_FilterInitStruct.FilterBank = 0;
    CAN_FilterInitStruct.FilterMode = CAN_FILTERMODE_IDMASK;
    CAN_FilterInitStruct.FilterScale = CAN_FILTERSCALE_32BIT;
    CAN_FilterInitStruct.FilterActivation = ENABLE;

    TEST_ASSERT_EQUAL(HAL_OK, HAL_CAN_ConfigFilter(&CAN_Handle, &CAN_FilterInitStruct));
}

/**
 * @brief Test: check if the speed CAN has been configured correctly
 *
 */
void test_setup_speed_can_init() {
    bxCANComputeTimings(HAL_RCC_GetPCLK1Freq(), CAN_BIT_RATE, &timings);
    // BAUD RATE = (80M / (SJW + TS1 + TS2)) / Prescaler
    uint32_t baudRate = (HAL_RCC_GetPCLK1Freq() /
                         (timings.max_resync_jump_width + timings.bit_segment_1 + timings.bit_segment_2)) /
                        timings.bit_rate_prescaler;
    TEST_ASSERT_EQUAL(CAN_BIT_RATE, baudRate);
}

/**
 * @brief Test: check if the bxCAN has been configured correctly
 *
 */
void test_bxcan_init() {
    TEST_ASSERT_TRUE(bxCANConfigure(0, timings, false));
}

/**
 * @brief Test: check if the CAN has been started correctly
 *
 */
void test_can_start() {
    TEST_ASSERT_EQUAL(HAL_OK, HAL_CAN_Start(&CAN_Handle));
}

/**
 * @brief Test: if the HEARTBEAT RX subscription has been created correctly
 *
 */
void test_create_rx_subscription() {
    CanardRxSubscription rx;

    // Messaggi HEARTBEAT: -> Verifica della presenza per stato Nodi (Slave) OnLine / OffLine
    TEST_ASSERT_EQUAL(1, canardRxSubscribe(&state.canard,
                                           CanardTransferKindMessage,
                                           uavcan_node_Heartbeat_1_0_FIXED_PORT_ID_,
                                           uavcan_node_Heartbeat_1_0_EXTENT_BYTES_,
                                           CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
                                           &rx));
}

/**
 * @brief Test: if the transfer payload has been pushed to the queue
 *
 */
void test_canard_tx_push() {
    for (uint8_t ifidx = 0; ifidx < CAN_REDUNDANCY_FACTOR; ifidx++) {
        CanardTxQueue* const que = &state.canard_tx_queues[ifidx];
        CanardMicrosecond monotonic_time = micros();

        // Find the highest-priority frame
        const CanardTxQueueItem* tqi = canardTxPeek(que);

        while (tqi != NULL) {
            TEST_ASSERT_TRUE(bxCANPush(0,
                                       monotonic_time,
                                       tqi->tx_deadline_usec,
                                       tqi->frame.extended_can_id,
                                       tqi->frame.payload_size,
                                       tqi->frame.payload));
            if (bxCANPush(0,
                          monotonic_time,
                          tqi->tx_deadline_usec,
                          tqi->frame.extended_can_id,
                          tqi->frame.payload_size,
                          tqi->frame.payload)) {
                // Push CAN data
                state.canard.memory_free(&state.canard, canardTxPop(que, tqi));
                tqi = canardTxPeek(que);
            } else {
                // Empty Queue
                break;
            }
        }
    }
}

void setup() {
    UNITY_BEGIN();

    delay(1000);

    // ******************************************************************************
    // ******************************** START TEST **********************************
    // ******************************************************************************
    RUN_TEST(test_can_hw_init);
    RUN_TEST(test_can_filter_init);
    RUN_TEST(test_setup_speed_can_init);
    RUN_TEST(test_bxcan_init);
    RUN_TEST(test_can_start);
    memoryAllocation();
    RUN_TEST(test_create_rx_subscription);
    RUN_TEST(test_canard_tx_push);
    // ******************************************************************************
    // ******************************** FINE TEST ***********************************
    // ******************************************************************************

    UNITY_END();
}

void loop() {
}

#endif