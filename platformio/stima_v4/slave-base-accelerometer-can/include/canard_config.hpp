// Assert Locali
#define LOCAL_ASSERT    assert

#define KILO 1000L
#define MEGA ((int64_t)KILO * KILO)

// CODA, RIDONDANZA, TIMEDELAY TX & RX CANARD
#define CAN_REDUNDANCY_FACTOR 1
#define CAN_TX_QUEUE_CAPACITY 100
#define CAN_MAX_IFACE         1
#define CAN_RX_QUEUE_CAPACITY 100
#define IFACE_CAN_IDX         0
#define CAN_DELAY_US_SEND     0
#define MAX_SUBSCRIPTION      10

// CAN SPEED RATE HZ
#define CAN_BIT_RATE 1000000ul
#define CAN_MTU_BASE 8

// Messaggi di lunghezza TimeOut Extra <> Standard
#define CANARD_REGISTERLIST_TRANSFER_ID_TIMEOUT_USEC 3500000UL
#define CANARD_READFILE_TRANSFER_ID_TIMEOUT_USEC     2500000UL

// A compilazione per semplificazione setup nodo (TEST)
// #define INIT_REGISTER

// Nodo Fisso per Modulo Master
#define NODE_MASTER_ID 100

// Nodo fisso per Modulo Slave
#define NODE_SLAVE_ID 10

// SET Default value per risposte
#define GENERIC_STATE_UNDEFINED 0x0Fu
#define GENERIC_BVAL_UNDEFINED  0xFFu

// Servizi Cypal attivi di default
#define DEFAULT_PUBLISH_PORT_LIST   true
#define DEFAULT_PUBLISH_MODULE_DATA false

// Time Publisher Servizi (secondi)
#define TIME_PUBLISH_MODULE_DATA    0.333
#define TIME_PUBLISH_PNP_REQUEST    2
#define TIME_PUBLISH_HEARTBEAT      1
#define TIME_PUBLISH_PORT_LIST      20

// TimeOUT (millisecondi)
#define MASTER_OFFLINE_TIMEOUT_US 6000000
#define MASTER_MAXSYNCRO_VALID_US 1250000
#define NODE_GETFILE_TIMEOUT_US   1750000
#define NODE_GETFILE_MAX_RETRY    3

// CODICI E STATUS AGGIORNAMENTO FILE REMOTI
#define FILE_NAME_SIZE_MAX 50
