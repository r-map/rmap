#define KILO 1000L
#define MEGA ((int64_t) KILO * KILO)

// CODA, RIDONDANZA, TIMEDELAY TX & RX CANARD
#define CAN_REDUNDANCY_FACTOR 1
#define CAN_TX_QUEUE_CAPACITY 100
#define CAN_MAX_IFACE 1
#define CAN_RX_QUEUE_CAPACITY 100
#define IFACE_CAN_IDX 0
#define CAN_DELAY_US_SEND 0

// CAN SPEED RATE HZ
#define CAN_BIT_RATE 1000000ul
#define CAN_MTU_BASE 8

// CANARD_SUBJECT_ID_DYNAMIC
#define PublisherSubjectID      0
#define SubscriptionSubjectID   1
#define ClientPortID            2
#define ServicePortID           3

// Messaggi di lunghezza TimeOut Extra <> Standard
#define CANARD_REGISTERLIST_TRANSFER_ID_TIMEOUT_USEC 3500000UL
#define CANARD_READFILE_TRANSFER_ID_TIMEOUT_USEC 2500000UL

// A compilazione per semplificazione setup nodo (TEST)
// #define INIT_REGISTER

// Nodo Fisso per Modulo Master
#define NODE_MASTER_ID 100

// Nodo fisso per Modulo Slave
// #define NODE_SLAVE_ID 10

// Tipologie Nodi/Sensori remoti (NODE_TYPE) TODO: H.EXTERN!!!
#define MODULE_TYPE_MASTER      0xFF
#define MODULE_TYPE_TH          0x01
#define MODULE_TYPE_PREC        0x02
#define MODULE_TYPE_WIND        0x03
#define MODULE_TYPE_RS          0x04
#define MODULE_TYPE_PRES        0x05
#define MODULE_TYPE_TS          0x06
#define MODULE_TYPE_US          0x07
#define MODULE_TYPE_WM          0x08
#define MODULE_TYPE_LIV         0x09

// SET Default value per risposte
#define GENERIC_BVAL_UNDEFINED  0xFF

// Tipologie di comandi interni / esterni USER_DEFINE UAVCAN
#define CMD_DOWNLOAD_FILE               5
#define CMD_ENABLE_PUBLISH_DATA         10
#define CMD_DISABLE_PUBLISH_DATA        11
#define CMD_ENABLE_PUBLISH_PORT_LIST    12
#define CMD_DISABLE_PUBLISH_PORT_LIST   13

// Servizi di default
#define DEFAULT_PUBLISH_PORT_LIST       true
#define DEFAULT_PUBLISH_MODULE_DATA     false

// TimeOUT (millisecondi)
#define MASTER_OFFLINE_TIMEOUT_US   6000000
#define MASTER_MAXSYNCRO_VALID_US   1250000
#define NODE_GETFILE_TIMEOUT_US     1750000
#define NODE_GETFILE_MAX_RETRY      3

// VENDOR STATUS CODE LOCALI
#define VSC_SOFTWARE_NORMAL         0x00
#define VSC_SOFTWARE_UPDATE_READ    0x01

// CODICI E STATUS AGGIORNAMENTO FILE REMOTI
#define FILE_NAME_SIZE_MAX          50
