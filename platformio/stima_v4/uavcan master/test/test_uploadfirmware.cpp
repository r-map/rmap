#include <Arduino.h>
#include <Unity.h>
#include <canard.h>
#include <o1heap.h>
#include <rmap/_module/TH_1_0.h>
#include <rmap/service/_module/TH_1_0.h>
#include <stdio.h>
#include <stdlib.h>

#include "module_config.hpp"
#include "register.hpp"

#define TEST_NODE_ID 125
#define SLAVE_IS_ONLINE

//********** FUNCTION DECLARATIONS **********
void reset_slave_nodeid_for_each_node(void);
void test_slave_is_online(void);
void test_file_firmware_is_exists(void);
//*******************************************

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
        } timestamp;
        // File upload (node_id può essere differente dal master, es. yakut e lo riporto)
        struct
        {
            uint8_t node_id;
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
    // TODO: Da convertire C++ con struttura ADD Istanza in funzione di Numero Nodi presenti
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
        // Heartbeat pubblicati Rx<-(Stato nodo remoto)
        struct {
            uint8_t state;        // Vendor specific code NodeSlave State
            uint8_t healt;        // Uavcan healt_state remoto
            uint64_t timeout_us;  // Time heartbeat Remoto x Verifica OffLine
        } heartbeat;
        // Comandi inviati da locale Tx->(Comando + Param) Rx<-(Risposta)
        struct {
            uint8_t response;          // Stato di risposta ai comandi nodo
            uint64_t timeout_us;       // Time command Remoto x Verifica deadLine Request
            bool is_pending;           // Funzione in pending (inviato, attesa risposta o timeout)
            bool is_timeout;           // Funzione in timeout (mancata risposta)
            uint8_t next_transfer_id;  // Transfer id relativo
        } command;
        // Accesso ai dati dello slave in servizio Tx->(Funzione) Rx<-(Dato + Stato)
        // Puntatore alla struttura dati relativa es. -> rmap_module_TH_1_0 ecc...
        struct {
            CanardPortID port_id;      // Porta del servizio dati correlato
            void* module;              // Dati e stato di risposta ai dati nodo
            uint64_t timeout_us;       // Time getData Remoto x Verifica deadline Request
            bool is_pending;           // Funzione in pending (inviato, attesa risposta o timeout)
            bool is_timeout;           // Funzione in timeout (mancata risposta)
            uint8_t next_transfer_id;  // Accesso ai dati
        } service;
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

/**
 * @brief Ritorna l'indice della coda master allocata in state in funzione del nodeId fisico
 *
 * @param state
 * @param nodeId
 * @return byte
 */
// Ritorna l'indice della coda master allocata in state in funzione del nodeId fisico
byte getQueueNodeFromId(State* const state, CanardNodeID nodeId) {
    // Cerco la corrispondenza node_id nella coda allocata master per ritorno queueID Index
    for (byte queueId = 0; queueId < MAX_NODE_CONNECT; queueId++) {
        // Se trovo il nodo che sta rispondeno nella coda degli allocati...
        if (state->slave[queueId].node_id == nodeId) {
            return queueId;
        }
    }
    return GENERIC_BVAL_UNDEFINED;
}

//********** GLOBAL VARIABLES/CONSTANTS **********
State state = {0};
byte queueId;
bool bFileUpload = false;
//************************************************

/**
 * @brief Reset Remote node_id per ogni nodo collegato. Solo i nodi validi verranno gestiti
 *
 */
void reset_slave_nodeid_for_each_node() {
    for (uint8_t iCnt = 0; iCnt < MAX_NODE_CONNECT; iCnt++) {
        state.slave[iCnt].node_id = CANARD_NODE_ID_UNSET;
        state.slave[iCnt].heartbeat.timeout_us = 0;
        state.slave[iCnt].command.timeout_us = 0;
        state.slave[iCnt].service.timeout_us = 0;
        state.slave[iCnt].service.next_transfer_id = 0;
        state.slave[iCnt].command.next_transfer_id = 0;
        state.slave[iCnt].service.port_id = UINT16_MAX;
        state.slave[iCnt].service.module = NULL;
#ifdef USE_SUB_PUBLISH_SLAVE_DATA
        state.slave[iCnt].publisher.subject_id = UINT16_MAX;
        state.slave[iCnt].publisher.data_count = 0;
#endif
        state.slave[iCnt].file.filename[0] = 0;
        state.slave[iCnt].file.state = FILE_STATE_STANDBY;
    }

    state.slave[0].node_id = TEST_NODE_ID;
    state.slave[0].node_type = MODULE_TYPE_TH;
#ifdef SLAVE_IS_ONLINE
    state.slave[0].is_online = true;
#endif
    state.slave[0].service.port_id = 100;
#ifdef USE_SUB_PUBLISH_SLAVE_DATA
// state.slave[0].subject_id = 5678;
#endif
    state.slave[0].service.module = malloc(sizeof(rmap_service_module_TH_Response_1_0));
    strcpy(state.slave[0].file.filename, "stima4.module_th-1.1.app.hex");
}

/**
 * @brief Test: check if the slave is online
 *
 */
void test_slave_is_online() {
    queueId = getQueueNodeFromId(&state, TEST_NODE_ID);
    if (state.slave[queueId].is_online) {
        bFileUpload = true;
        state.slave[queueId].file.is_firmware = true;
        state.slave[queueId].file.state = FILE_STATE_BEGIN_UPDATE;
    }
    TEST_ASSERT_TRUE(state.slave[queueId].is_online);
}

/**
 * @brief Test: controllo che il nome del file firmware esiste e che sia coerente
 *
 */
void test_file_firmware_is_exists() {
    // Avvio comando di aggiornamento Controllo coerenza Firmware se Firmware!!!
    // Verifico il nome File locale (che RMAP Server ha già inviato il file in HTTP...)
    if (state.slave[queueId].file.is_firmware) {
        if (ccFirwmareFile(state.slave[queueId].file.filename)) {
            // Avvio il comando nel nodo remoto
            state.slave[queueId].file.state = FILE_STATE_COMMAND_SEND;
        } else {
            // Gestisco l'errore di coerenza verso il server
            // Comunico il problema nel file
            state.slave[queueId].file.state = FILE_STATE_STANDBY;
        }
    } else {
        // N.B. Eventuale altro controllo di coerenza...
        // Al momento non gestisco in quanto un eventuale LOG non è problematico
        // Avvio il comando nel nodo remoto
        state.slave[queueId].file.state = FILE_STATE_COMMAND_SEND;
    }
    TEST_ASSERT_TRUE(ccFirwmareFile(state.slave[queueId].file.filename));
}

void setup() {
    UNITY_BEGIN();

    delay(1000);

    reset_slave_nodeid_for_each_node();
    RUN_TEST(test_slave_is_online);

    while (bFileUpload) {
        switch (state.slave[queueId].file.state) {
            // FW_STATE_STANDBY
            default:
                bFileUpload = false;
                break;
            case FILE_STATE_BEGIN_UPDATE:
                RUN_TEST(test_file_firmware_is_exists);
                bFileUpload = false;
                break;
            case FILE_STATE_COMMAND_SEND:
                break;
            case FILE_STATE_COMMAND_WAIT:
                break;
            case FILE_STATE_UPLOADING:
                break;
            case FILE_STATE_UPLOAD_COMPLETE:
                break;
        }
    }

    UNITY_END();
}

void loop() {
}