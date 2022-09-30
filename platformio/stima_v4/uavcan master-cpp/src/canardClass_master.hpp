// *************************************************************************************************
// **********     Funzioni ed utility generiche per gestione Classe UAVCAN e O1Heap       **********
// *************************************************************************************************

// This software is distributed under the terms of the MIT License.
// Progetto RMAP - STIMA V4
// canardClass Master, Rev.1.00 del 30/09/2022
// Copyright (C) 2022 Digiteco s.r.l.
// Author: Gasperini Moreno <m.gasperini@digiteco.it>

// Arduino
#include <Arduino.h>
// Libcanard
#include "register.hpp"
#include <o1heap.h>
#include <canard.h>
// Namespace RMAP
#include <rmap/_module/TH_1_0.h>
#include <rmap/service/_module/TH_1_0.h>
#include <uavcan/node/Mode_1_0.h>
// Configurazione modulo, definizioni ed utility generiche
#include "module_config.hpp"

// Gestione statica completa
// #define CANARD_CLASS_STATIC
#ifdef CANARD_CLASS_STATIC
#   define CLASS_MODE inline static
#else
#   define CLASS_MODE
#endif

class canardClass {

    // ***************** PUBLIC ACCESS *****************
    public: 

        // ********************   Tipi di Dati    *****************

        // Modalità di accesso a getMonotonicMicroseconds()
        enum GetMonotonicTime_Type : uint8_t {
            syncronized_time,
            start_syncronization
        };

        // Gestione modalità Power ( x Canard e Nodo in generale) 
        enum Power_Mode : uint8_t {
            pwr_on,         // Never (All ON, test o gestione locale)
            pwr_nominal,    // Every Second (Nominale base)
            pwr_sleep_05,   // Every 5 Second (Low power 1)
            pwr_sleep_15,   // Every 15 Second (Low power 2)
            pwr_sleep_30,   // Every 30 Second (Low power 3)
            pwr_sleep_60,   // Every Minute (Low power 4...)
            pwr_deep_save,  // Deep mode (Very Low Power)
            pwr_critical    // Deep mode (Power Critical, Save data, Power->Off)
        };

        // Interprete heartBeat VSC (Vendor Status Code) x Comunicazioni messaggi Master<->Slave
        typedef union {
            // Bit field
            struct {
                Power_Mode  powerMode   : 3;
                bool        fwUploading : 1;
                bool        dataReady   : 1;
                bool        moduleReady : 1;
                bool        moduleError : 1;
            };
            // uint8 value
            uint8_t uint8_val;
        } heartbeat_VSC;

        // Coda di ricezione dati per gestione rapida in Interrupt (STM32 IRQ_CB bxCan)
        // Gestisce l'inserimento dei dati sulla coda locale, prima della gestione
        // Con Canard con il passaggio dalla coda bufferizzata ai frame di Canard_Rx
        typedef struct CanardRxQueue
        {
            // CanardTxQueue (Frame e Buffer x Interrupt gestione Coda FiFo)
            uint8_t wr_ptr;
            uint8_t rd_ptr;
            struct msg {
                CanardFrame frame;
                uint8_t buf[CANARD_MTU_MAX];
            } msg[CAN_RX_QUEUE_CAPACITY];
        } CanardRxQueue;

        // ******************** Dati e Funzioni ******************

        // Constructor di classe
        canardClass();

        // Timings Function e Dati associati
        static CanardMicrosecond getMonotonicMicroseconds();
        static CanardMicrosecond getMonotonicMicroseconds(GetMonotonicTime_Type syncro_type);
        static uint32_t getUpTimeSecond(void);
    
        // *************************************************
        //                  Canard O1HEAP
        // *************************************************
        O1HeapDiagnostics memGetDiagnostics(void);

        // *************************************************
        //                  Canard SendData
        // *************************************************
        // Wrapper per send e sendresponse con Canard 
        void send(CanardMicrosecond tx_deadline_usec,
                        const CanardTransferMetadata* const metadata,
                        const size_t payload_size,
                        const void* const payload);
        // Risposte con inversione meta.transfer_kind alle Request
        void sendResponse(const CanardMicrosecond tx_deadline_usec,
                        const CanardTransferMetadata* const request_metadata,
                        const size_t payload_size,
                        const void* const payload);                        

        // Gestione messaggi e coda di trasmissione
        bool transmitQueueDataPresent(void);
        void transmitQueue(void);

        // *************************************************
        //       Canard Local ISR Buffer ReceiveData
        // *************************************************
        // Gestione messaggi e coda di ricezione in buffer FIFO
        void receiveQueueEmpty(void);
        bool receiveQueueDataPresent(void);
        uint8_t receiveQueueElement(void);
        uint8_t receiveQueueNextElement(uint8_t currElement);

        // Ricezione messaggi per elementi in RX, con logger opzionale
        void receiveQueue(void);
        void receiveQueue(char *logMessage);

        // CallBack esterna per rxCanardAccept dei messaggi dal buffer RX (valid message)
        // Richiamata se canardAccept accetta il pacchetto ricomposto dei frame in RX
        void setReceiveMessage_CB (void (*ptrFunction) (canardClass&, const CanardRxTransfer*));
        void enableReceiveMessage_CB (void);
        void disableReceiveMessage_CB (void);

        // *************************************************
        //        Canard Metodi di sottoscrizione
        // *************************************************

        // Sottoscrizione ai metodi Canard
        bool rxSubscribe(const CanardTransferKind transfer_kind,
                        const CanardPortID port_id,
                        const size_t extent,
                        const CanardMicrosecond transfer_id_timeout_usec);
        uint8_t rxSubscriptionAvaiable(void);
        void rxUnSubscribe(const CanardTransferKind transfer_kind,
                        const CanardPortID port_id);

        // Istanza del modulo canard
        CLASS_MODE CanardInstance canard;

        // *************************************************
        //  Sottoclassi MASTER locale per gestione di Stato
        // *************************************************
        CLASS_MODE class master
        {
            public:

            // Time stamp
            class timestamp {
                
                public:

                void set_previous_tx_real(void);
                CanardMicrosecond get_previous_tx_real(bool enable_request_tx_real);
                bool is_requested_transmit_real(void);

                private:

                CanardMicrosecond _previous_tx_real;
                bool _enable_immediate_tx_real;

            } timestamp;

            // File upload (node_id può essere differente dal master, es. yakut e lo riporto)
            class file
            {

                public:

                void     start_request(uint8_t remote_node, uint8_t *param_request_name,
                                    uint8_t param_request_len, bool is_firmware);
                char    *get_name(void);
                CanardNodeID get_server_node(void);
                bool     download_request(void);
                bool     is_download_complete(void);
                void     download_end(void);
                bool     is_firmware(void);
                uint64_t get_offset_rx(void);
                void     set_offset_rx(uint64_t remote_file_offset);
                bool     is_first_data_block(void);
                bool     next_retry(void);
                bool     next_retry(uint8_t *avaiable_retry);
                void     start_pending(uint32_t timeout_us);
                void     reset_pending(void);
                void     reset_pending(size_t message_len);
                bool     event_timeout(void);
                bool     is_pending(void);

                private:

                uint8_t  _node_id;
                char     _filename[FILE_NAME_SIZE_MAX];
                bool     _is_firmware;
                bool     _updating;
                uint8_t  _updating_retry;
                bool     _updating_eof;
                uint64_t _offset;
                uint64_t _timeout_us;           // Time command Remoto x Verifica deadLine Request
                bool     _is_executed_command;  // Funzione in pending (inviato, attesa risposta o timeout)
                bool     _is_pending;           // Funzione in pending (inviato, attesa risposta o timeout)
                bool     _is_timeout;           // Funzione in timeout (mancata risposta)
 
            } file;
 
        } master;


// TODO: CLASS -> + ObjFunction
// Stato dei nodi slave e servizi gestiti collegati. Possibile lettura dai registri e gestione automatica
struct slave
{
    // Parametri Statici da leggere da registri o altro. Comunque inviati dal Master
    // node_id è l'indirizzo nodo remoto sui cui gestire i servizi
    // node_type identifica il tipo di nodo per sapere che tipologia di gestione viene effettuata
    // service è il port id sul node_id remoto che risponde ai servizi relativi di request
    // publisher è il subject id sul node_id remoto che pubblica i dati quando attivato
    // Gli altri parametri sono dinamicamente gestiti durante il funzionamento del programma
    uint8_t  node_id;
    uint8_t  node_type;
    // Nodo OnLine / OffLine
    bool     is_online;
    struct heartbeat {
        uint8_t  state;                 // Vendor specific code NodeSlave State
        uint8_t  healt;                 // Uavcan healt_state remoto
        uint64_t timeout_us;            // Time heartbeat Remoto x Verifica OffLine
    } heartbeat;
    struct pnp {
        // Flag di assegnamento PNP (se TRUE, da scrivere in ROM NON VOLATILE / REGISTER)
        // Il nodo è stato assegnato e il pnp non deve essere eseguito per quel modulo
        // se false il pnp è eseguito fino a quando non rilevo un modulo compatibile
        // con la richiesta (MODULE_TH, MODULE_RAIN ecc...) appena ne trovo uno
        // Heartbeat pubblicati Rx<-(Stato nodo remoto)
        // Questo permette l'assegmaneto PNP di tutti i moduli in sequenza (default)
        // O l'aggiunta successiva previa modifica della configurazione remota
        bool     is_assigned;           // flag resettato (PNP in funzione)
    } pnp;
    // Comandi inviati da locale Tx->(Comando + Param) Rx<-(Risposta)
    struct command {
        uint8_t  response;              // Stato di risposta ai comandi nodo
        uint64_t timeout_us;            // Time command Remoto x Verifica deadLine Request
        bool     is_pending;            // Funzione in pending (inviato, attesa risposta o timeout)
        bool     is_timeout;            // Funzione in timeout (mancata risposta)
        uint8_t  next_transfer_id;      // Transfer ID associato alla funzione
    } command;
    // Accesso ai registri inviati da locale Tx->(Registro + Value) Rx<-(Risposta)
    struct register_access {
        uavcan_register_Value_1_0 response; // Valore in risposta al registro x Set (R/W)
        uint64_t timeout_us;            // Time command Remoto x Verifica deadLine Request
        bool     is_pending;            // Funzione in pending (inviato, attesa risposta o timeout)
        bool     is_timeout;            // Funzione in timeout (mancata risposta)
        uint8_t  next_transfer_id;      // Transfer ID associato alla funzione
    } register_access;
    // Accesso ai dati dello slave in servizio Tx->(Funzione) Rx<-(Dato + Stato)
    // Puntatore alla struttura dati relativa es. -> rmap_module_TH_1_0 ecc...
    struct rmap_service {
        CanardPortID port_id;           // Porta del servizio dati correlato
        void*    module;                // Dati e stato di risposta ai dati nodo
        uint64_t timeout_us;            // Time getData Remoto x Verifica deadline Request
        bool     is_pending;            // Funzione in pending (inviato, attesa risposta o timeout)
        bool     is_timeout;            // Funzione in timeout (mancata risposta)
        uint8_t  next_transfer_id;      // Transfer ID associato alla funzione
    } rmap_service;
    // Nome file(locale) per aggiornamento file remoto e relativo stato di funzionamento
    struct file {
        char     filename[FILE_NAME_SIZE_MAX];
        bool     is_firmware;           // Comunico se file in TX è un Firmware o altro
        uint8_t  state;                 // Stato del file transfer (gestione switch interno)
        uint64_t timeout_us;            // Time command Remoto x Verifica deadLine Request File
        bool     is_pending;            // Funzione in pending (inviato, attesa risposta o timeout)
        bool     is_timeout;            // Funzione in timeout (mancata risposta)
    } file;
    // Pubblicazione dei dati autonoma (struttura dati come per servizio)
    #ifdef USE_SUB_PUBLISH_SLAVE_DATA
    struct publisher {
        CanardPortID subject_id;        // Suject id associato alla pubblicazione
        uint16_t data_count;            // Conteggio pubblicazioni remote autonome (SOLO x TEST)
    } publisher;
    #endif
} slave[MAX_NODE_CONNECT];

        // Abilitazione delle pubblicazioni falcoltative sulla rete (ON/OFF a richiesta)
        CLASS_MODE class publisher_enabled
        {
            public:

            bool port_list;

        } publisher_enabled;

        // Tranfer ID (CAN Interfaccia ID -> uint8) servizi attivi del modulo locale
        CLASS_MODE class next_transfer_id
        {
            public:

            uint8_t uavcan_node_heartbeat(void);
            uint8_t uavcan_node_port_list(void);
            uint8_t uavcan_time_synchronization(void);
            uint8_t uavcan_file_read_data(void);

            private:

            uint8_t _uavcan_node_heartbeat;
            uint8_t _uavcan_node_port_list;
            uint8_t _uavcan_time_synchronization;
            uint8_t _uavcan_file_read_data;

        } next_transfer_id;

        // Flag di stato TODO: da implementare funzioni sleep/inibith locali
        CLASS_MODE class flag
        {

            public:

            // Funzioni Locali per gestione sistema
            void request_system_restart(void);
            bool is_requested_system_restart(void);
            void request_sleep(void);
            bool is_module_sleep(void);
            void disable_sleep(void);
            void enable_sleep(void);
            // Funzioni Locali per gestione interna e VSC x HeartBeat VendorStatusCode a Bit MASK
            void set_local_power_mode(Power_Mode powerMode);
            void set_local_fw_uploading(bool fwUploading);
            void set_local_data_ready(bool dataReady);
            void set_local_module_ready(bool moduleReady);
            void set_local_module_error(bool moduleError);
            Power_Mode get_local_power_mode(void);
            bool get_local_fw_uploading(void);
            bool get_local_data_ready(void);
            bool get_local_module_ready(void);
            bool get_local_module_error(void);
            uint8_t get_local_value_heartbeat_VSC(void);
            // Funzioni Locali per gestione interna e VSC x HeartBeat VendorStatusCode a Bit MASK
            void set_local_node_mode(uint8_t heartLocalMODE);
            uint8_t get_local_node_mode(void);

            private:

            bool _restart_required;     // Forza reboot
            bool _enter_sleep;          // Avvia la procedura di sleep del modulo
            bool _inibith_sleep;        // Inibisce lo sleep del modulo
            // Var per Status locale da utilizzare, RW in Heart_beat
            heartbeat_VSC _heartLocalVSC;
            uint8_t _heartLocalMODE;

        } flag;

    // ***************** PRIVATE ACCESS *****************
    private:

        // Buffer dati e trasmissione delle code di tx e quella di rx
        CLASS_MODE CanardRxQueue _canard_rx_queue;
        CLASS_MODE CanardTxQueue _canard_tx_queues[CAN_REDUNDANCY_FACTOR];

        // Timings var per getMonotonicMicroseconds();
        inline static uint32_t _lastMicros;
        inline static uint64_t _currMicros;        
        inline static uint64_t _syncMicros;

        // Canard O1HEAP, Gestita RAM e CallBack internamente alla classe
        CLASS_MODE O1HeapInstance* _heap;
        _Alignas(O1HEAP_ALIGNMENT) CLASS_MODE uint8_t _heap_arena[1024 * 16];

        // Gestione O1Heap Static Funzioni x Canard Memory Allocate/Free
        static void* _memAllocate(CanardInstance* const ins, const size_t amount);
        static void  _memFree(CanardInstance* const ins, void* const pointer);        

        // Indirizzo della funzione di CallBack Esterna su Rx Messaggio Canard
        CLASS_MODE bool _attach_rx_callback;
        CLASS_MODE void (*_attach_rx_callback_PTR) (canardClass&, const CanardRxTransfer*);

        // Gestione subscription locali
        CLASS_MODE CanardRxSubscription _rxSubscription[MAX_SUBSCRIPTION];
        CLASS_MODE uint8_t _rxSubscriptionIdx;
};
