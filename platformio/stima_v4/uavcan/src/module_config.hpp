#define KILO 1000L
#define MEGA ((int64_t) KILO * KILO)

// CODA E RIDONDANZA (Nessun proplema verificato, da effettuare TEST con Disturbatore di linea)
#define CAN_REDUNDANCY_FACTOR 1
#define CAN_TX_QUEUE_CAPACITY 100
// CAN SPEED RATE HZ
#define CAN_BIT_RATE 250000ul

// CANARD_SUBJECT_ID_DYNAMIC
#define PublisherSubjectID      0
#define SubscriptionSubjectID   1
#define ClientPortID            2
#define ServicePortID           3

// Messaggi di lunghezza TimeOut Extra <> Standard
#define CANARD_REGISTERLIST_TRANSFER_ID_TIMEOUT_USEC 4000000UL

// CANARD_MTU_CAN_CLASSIC non supporta PNP_NODE_ALLOCATION_V_10
#if (CANARD_MTU_MAX == CANARD_MTU_CAN_CLASSIC)
    #define PNP_NODE_ALLOCATION_V_10
#else
    #define PNP_NODE_ALLOCATION_V_20
#endif

// Nodo Fisso per Modulo Master
#define NODE_MASTER_ID 100
// Nodo fisso per Modulo Slave
// #define NODE_SLAVE_ID 10

// Numero di nodi massimo da collegare al MASTER, TODO: DA Verificare se malloc...
#define MAX_NODE_CONNECT 20
// Gestioni Locali semplificata stato Nodi remoti
// SET a BIT (BIT 0   Online)
//           (BIT 1,2 Canard HealtState)
#define IsNodeOffline(x)        (!(x&0x01))
#define IsNodeOnline(x)         (x&0x01)
#define SetNodeOffline(x)       (x&=0xFE)
#define SetNodeOnline(x)        (x|=0x01)
#define SetNodeHealtState(x, y) (x|=(y<<1))
#define GetNodeHealtState(x)    (x>>1)

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
