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

// Utilizza metodo di sottoscrizione al publisher per acceso ai dati slave remoti
// Opzionale se non utilizzata per il popolamento di dati come ad. esempio display
// Sempre attiva invece sui nodi slave per accesso con tool esterni di debug (Yakut)
#define USE_SUB_PUBLISH_SLAVE_DATA

// Nodo Fisso per Modulo Master
#define NODE_MASTER_ID 100
// Numero di nodi massimo da collegare al MASTER, TODO: DA Verificare se malloc...
#define MAX_NODE_CONNECT 8

// Gestione FLAG INTERNO per gestione stati nodi remoti e Locali
// Gestioni Locali semplificata stato Nodi remoti BITS:
// Utilizzo speculare per nodi Locali (solo quelli utilizzati -> File)

#define GENERIC_BVAL_UNDEFINED  0xFF
// Flag e Bit Locali
#define LocalPendingFileRead        0u
#define LocalTimeOutFileRead        1u
// Set/Reset Value Locali
#define IsLocalPendingFile(x)       bitRead(x, LocalPendingFileRead)
#define SetLocalPendingFile(x)      bitSet(x, LocalPendingFileRead)
#define ResetLocalPendingFile(x)    bitClear(x, LocalPendingFileRead)
#define IsLocalTimeOutFile(x)       bitRead(x, LocalTimeOutFileRead)
#define SetLocalTimeOutFile(x)      bitSet(x, LocalTimeOutFileRead)
#define ResetLocalTimeOutFile(x)    bitClear(x, LocalTimeOutFileRead)
// Flag e Bit Remoti
#define RemoteHealtStateLow         0u
#define RemoteHealtStateHigh        1u
#define RemoteNodeOnline            2u
#define RemoteNodePendindCmd        3u
#define RemoteNodeTimeOutCmd        4u
#define RemoteNodePendindData       5u
#define RemoteNodeTimeOutData       6u
#define RemoteNodePendindFile       7u
#define RemoteNodeTimeOutFile       8u
// Set/Reset Value Remoti
#define SetNodeWarningState(x, y)   bitClear(x, 0);bitClear(x, 1);(x|=y)
#define GetNodeWarningState(x)      (x&0x03)
#define IsRemoteOnline(x)           bitRead(x, RemoteNodeOnline)
#define IsRemoteOffline(x)          (!IsRemoteOnline(x))
#define SetRemoteOnline(x)          bitSet(x, RemoteNodeOnline)
#define SetRemoteOffline(x)         bitClear(x, RemoteNodeOnline)
#define IsRemotePendingCmd(x)       bitRead(x, RemoteNodePendindCmd)
#define SetRemotePendingCmd(x)      bitSet(x, RemoteNodePendindCmd)
#define ResetRemotePendingCmd(x)    bitClear(x, RemoteNodePendindCmd)
#define IsRemoteTimeOutCmd(x)       bitRead(x, RemoteNodeTimeOutCmd)
#define SetRemoteTimeOutCmd(x)      bitSet(x, RemoteNodeTimeOutCmd)
#define ResetRemoteTimeOutCmd(x)    bitClear(x, RemoteNodeTimeOutCmd)
#define IsRemotePendingData(x)      bitRead(x, RemoteNodePendindData)
#define SetRemotePendingData(x)     bitSet(x, RemoteNodePendindData)
#define ResetRemotePendingData(x)   bitClear(x, RemoteNodePendindData)
#define IsRemoteTimeOutData(x)      bitRead(x, RemoteNodeTimeOutData)
#define SetRemoteTimeOutData(x)     bitSet(x, RemoteNodeTimeOutData)
#define ResetRemoteTimeOutData(x)   bitClear(x, RemoteNodeTimeOutData)
#define IsRemotePendingFile(x)      bitRead(x, RemoteNodePendindFile)
#define SetRemotePendingFile(x)     bitSet(x, RemoteNodePendindFile)
#define ResetRemotePendingFile(x)   bitClear(x, RemoteNodePendindFile)
#define IsRemoteTimeOutFile(x)      bitRead(x, RemoteNodeTimeOutFile)
#define SetRemoteTimeOutFile(x)     bitSet(x, RemoteNodeTimeOutFile)
#define ResetRemoteTimeOutFile(x)   bitClear(x, RemoteNodeTimeOutFile)

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
#define DEFAULT_PUBLISH_PORT_LIST       true

// TimeOUT (millisecondi) OFF_LINE Deve essere > Degli altri TimeOUT
// Non succede nulla perchè gestito completamente con Reset TimeOut
// Ma si riesce ad identificare il TimeOut intervenuto per la sua gestione
#define NODE_OFFLINE_TIMEOUT_MS     6000
#define NODE_COMMAND_TIMEOUT_MS     1250
#define NODE_GETDATA_TIMEOUT_MS     1500
#define NODE_GETFILE_TIMEOUT_MS     1750
#define NODE_REQFILE_TIMEOUT_MS     4000
#define NODE_GETFILE_MAX_RETRY      3

// VENDOR STATUS CODE LOCALI
#define VSC_SOFTWARE_NORMAL         0x00
#define VSC_SOFTWARE_UPDATE_READ    0x01

// CODICI E STATUS AGGIORNAMENTO FIRMWARE REMOTI
#define FW_NAME_SIZE_MAX            50
#define FW_STATE_STANDBY            0x00
#define FW_STATE_BEGIN_UPDATE       0x01
#define FW_STATE_COMMAND_SEND       0x02
#define FW_STATE_COMMAND_WAIT       0x03
#define FW_STATE_UPLOADING          0x04
#define FW_STATE_UPLOAD_COMPLETE    0x05