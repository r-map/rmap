#define KILO 1000L
#define MEGA ((int64_t) KILO * KILO)

// CODA E RIDONDANZA (Nessun proplema verificato, da effettuare TEST con Disturbatore di linea)
#define CAN_REDUNDANCY_FACTOR 1
#define CAN_TX_QUEUE_CAPACITY 100
// CAN SPEED RATE HZ
#define CAN_BIT_RATE 1000000ul
#define CAN_MTU_BASE 8

// CANARD_SUBJECT_ID_DYNAMIC
#define PublisherSubjectID      0
#define SubscriptionSubjectID   1
#define ClientPortID            2
#define ServicePortID           3

// Messaggi di lunghezza TimeOut Extra <> Standard
#define CANARD_REGISTERLIST_TRANSFER_ID_TIMEOUT_USEC 4000000UL

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

// Gestione FLAG INTERNO per gestione stati nodi remoti
// Gestioni Locali semplificata stato Nodi remoti BITS:
// (BIT 7   Online)
// (BIT 0,1 Heartbeat HealtState)
// (BIT 2   Pending Command request)
// (BIT 3   TimeOut Command occurs)
// (BIT 4   Pending Data request)
// (BIT 5   TimeOut Data occurs)
// (BIT 6   Free)

#define GENERIC_BVAL_UNDEFINED  0xFF
#define IsNodeOnline(x)         (x&0x80)
#define IsNodeOffline(x)        (!(x&0x80))
#define SetNodeOffline(x)       (x&=0x7F)
#define SetNodeOnline(x)        (x|=0x80)
#define SetNodeHealtState(x, y) (x|=(x&0xFC|y))
#define GetNodeHealtState(x)    (x&0x03)
#define SetNodePendingCmd(x)    (x|=0x04)
#define ResetNodePendingCmd(x)  (x&=0xFB)
#define IsNodePendingCmd(x)     (x&0x04)
#define SetNodeTimeOutCmd(x)    (x|=0x08)
#define ResetNodeTimeOutCmd(x)  (x&=0xF7)
#define IsNodeTimeOutCmd(x)     (x&0x08)
#define SetNodePendingData(x)   (x|=0x10)
#define ResetNodePendingData(x) (x&=0xEF)
#define IsNodePendingData(x)    (x&0x10)
#define SetNodeTimeOutData(x)   (x|=0x20)
#define ResetNodeTimeOutData(x) (x&=0xD7)
#define IsNodeTimeOutData(x)    (x&0x20)

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

// Tipologie di comandi interni / esterni USER_DEFINE UAVCAN
#define CMD_ENABLE_PUBLISH_DATA         10
#define CMD_DISABLE_PUBLISH_DATA        11
#define CMD_ENABLE_PUBLISH_PORT_LIST    12
#define CMD_DISABLE_PUBLISH_PORT_LIST   13

// Servizi di default
#define DEFAULT_PUBLISH_PORT_LIST       false

// TimeOUT (millisecondi)
#define NODE_OFFLINE_TIMEOUT_MS     3000
#define NODE_COMMAND_TIMEOUT_MS     1500
#define NODE_GETDATA_TIMEOUT_MS     1500