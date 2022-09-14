#define KILO 1000L
#define MEGA ((int64_t)KILO * KILO)

// CODA, RIDONDANZA, TIMEDELAY TX & RX CANARD
#define CAN_REDUNDANCY_FACTOR 1
#define CAN_TX_QUEUE_CAPACITY 100
#define CAN_MAX_IFACE         1
#define CAN_RX_QUEUE_CAPACITY 100
#define IFACE_CAN_IDX         0
#define CAN_DELAY_US_SEND     0

// CAN SPEED RATE HZ
#define CAN_BIT_RATE 1000000ul
#define CAN_MTU_BASE 8

// CANARD_SUBJECT_ID_DYNAMIC
#define PublisherSubjectID    0
#define SubscriptionSubjectID 1
#define ClientPortID          2
#define ServicePortID         3

// Messaggi di lunghezza TimeOut Extra <> Standard
#define CANARD_REGISTERLIST_TRANSFER_ID_TIMEOUT_USEC 3500000UL
#define CANARD_READFILE_TRANSFER_ID_TIMEOUT_USEC     2500000UL

// A compilazione per semplificazione setup nodo (TEST)
// #define INIT_REGISTER

// Utilizza metodo di sottoscrizione al publisher per acceso ai dati slave remoti
// Opzionale se non utilizzata per il popolamento di dati come ad. esempio display
// Sempre attiva invece sui nodi slave per accesso con tool esterni di debug (Yakut)
#define USE_SUB_PUBLISH_SLAVE_DATA

// Nodo Fisso per Modulo Master
#define NODE_MASTER_ID 100
// Numero di nodi massimo da collegare al MASTER, TODO: DA Verificare se malloc...
#define MAX_NODE_CONNECT 8

// UTILITA' x PLUG_AND PLAY ED INFO_NODE
// Tipologie Nodi/Sensori remoti (NODE_TYPE) TODO: H.EXTERN!!!
#define MODULE_TYPE_MASTER    0xFF
#define MODULE_TYPE_TH        0x01
#define MODULE_TYPE_RAIN      0x02
#define MODULE_TYPE_WIND      0x03
#define MODULE_TYPE_RADIATION 0x04
#define MODULE_TYPE_VWC       0x05
#define MODULE_TYPE_POWER     0x06
// MODULE_TYPE_TH -> SUBMODULES (ecc...)
#define SENS_TYPE_TH_HYT_221 0x01
#define SENS_TYPE_TH_PT100   0x02

// SET Default value per risposte
#define GENERIC_STATE_UNDEFINED 0x0F
#define GENERIC_BVAL_UNDEFINED  0xFF

// Tipologie di comandi interni / esterni USER_DEFINE UAVCAN
#define CMD_DOWNLOAD_FILE             5
#define CMD_ENABLE_PUBLISH_DATA       10
#define CMD_DISABLE_PUBLISH_DATA      11
#define CMD_ENABLE_PUBLISH_PORT_LIST  12
#define CMD_DISABLE_PUBLISH_PORT_LIST 13
#define CMD_TEST                      99
#define CMD_TEST_VALUE                100

// Servizi di default
#define DEFAULT_PUBLISH_PORT_LIST true

// TimeOUT (millisecondi) OFF_LINE Deve essere > Degli altri TimeOUT
// Non succede nulla perchè gestito completamente con Reset TimeOut
// Ma si riesce ad identificare il TimeOut intervenuto per la sua gestione
#define NODE_OFFLINE_TIMEOUT_US  6000000
#define NODE_COMMAND_TIMEOUT_US  1250000
#define NODE_REGISTER_TIMEOUT_US 1500000
#define NODE_GETDATA_TIMEOUT_US  1500000
#define NODE_GETFILE_TIMEOUT_US  1750000
#define NODE_REQFILE_TIMEOUT_US  4000000
#define NODE_GETFILE_MAX_RETRY   3

// VENDOR STATUS CODE LOCALI
#define VSC_SOFTWARE_NORMAL      0x00
#define VSC_SOFTWARE_UPDATE_READ 0x01

// CODICI E STATUS AGGIORNAMENTO FIRMWARE REMOTI
#define FILE_NAME_SIZE_MAX         50
#define FILE_STATE_STANDBY         0x00
#define FILE_STATE_BEGIN_UPDATE    0x01
#define FILE_STATE_COMMAND_SEND    0x02
#define FILE_STATE_COMMAND_WAIT    0x03
#define FILE_STATE_UPLOADING       0x04
#define FILE_STATE_UPLOAD_COMPLETE 0x05