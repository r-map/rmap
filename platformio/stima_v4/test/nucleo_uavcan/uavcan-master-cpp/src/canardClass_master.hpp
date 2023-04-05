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
// Namespace UAVCAN
#include <uavcan/node/Heartbeat_1_0.h>
#include <uavcan/node/GetInfo_1_0.h>
#include <uavcan/node/ExecuteCommand_1_1.h>
#include <uavcan/node/port/List_0_1.h>
#include <uavcan/_register/Access_1_0.h>
#include <uavcan/_register/List_1_0.h>
#include <uavcan/file/Read_1_1.h>
#include <uavcan/time/Synchronization_1_0.h>
#include <uavcan/pnp/NodeIDAllocationData_1_0.h>
// Namespace RMAP
#include <rmap/_module/TH_1_0.h>
#include <rmap/service/_module/TH_1_0.h>
// Configurazione modulo, definizioni ed utility generiche
#include "module_config.hpp"

class canardClass {

    // ***************** PUBLIC ACCESS *****************
    public: 

        // ********************   Tipi di Dati    *****************

        // Tipologia di moduli disponibili Stima V4 x PNP e InfoNode
        enum Module_Type : uint8_t {
            undefined   = 0x00,
            th          = 0x01,
            rain        = 0x02,
            wind        = 0x03,
            radiation   = 0x04,
            vwc         = 0x05,
            power       = 0x06,
            server      = 0xFF
        };

        // Modalità di accesso a getMicros()
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

        // Gestione modalità file server Upload
        enum FileServer_State : uint8_t {
            standby,
            begin_update,
            command_send,
            command_wait,
            state_uploading,
            upload_complete
        };

        // Gestione comandi privati di Canard / Rmap
        enum Command_Private : uint8_t {
            download_file             =  5,
            enable_publish_rmap       = 10,
            disable_publish_rmap      = 11,
            enable_publish_port_list  = 12,
            disable_publish_port_list = 13
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
        static CanardMicrosecond getMicros();
        static CanardMicrosecond getMicros(GetMonotonicTime_Type syncro_type);
        static uint32_t getUpTimeSecond(void);
    
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

        // ****************************************************************************************
        //  Comandi Server per gestione del modulo locale tramite classe Master e pending o Diretti
        // ****************************************************************************************
        bool send_file_read_block(CanardNodeID server_id, char * fileName, uint64_t remoteOffset);
        bool master_file_read_block_pending(uint32_t timeout_us);
        bool master_timestamp_send_syncronization(void);
        bool master_heartbeat_send_message(void);
        bool master_servicelist_send_message(void);

        // *************************************************
        //  Sottoclassi MASTER locale per gestione di Stato
        // *************************************************
        class master
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
                bool     _is_pending;           // Funzione in pending (inviato, attesa risposta o timeout)
 
            } file;
 
        } master;

        // ****************************************************************************************
        //  Comandi Server per gestione dei moduli Remoti tramite classe Slave e pending o Diretti
        // ****************************************************************************************

        bool send_register_access_pending(uint8_t slaveIstance, uint32_t timeout_us, char *registerName,
                        uavcan_register_Value_1_0 registerValue, bool write);
        bool send_register_access(CanardNodeID node_id, uint8_t transfer_id, char *registerName,
                        uavcan_register_Value_1_0 registerValue, bool write);
        bool send_command_pending(uint8_t slaveIstance, uint32_t timeout_us, uint16_t cmd_request,
                                void* ext_param, size_t ext_lenght);
        bool send_command(CanardNodeID node_id, uint8_t transfer_id, uint16_t cmd_request,
                                void* ext_param, size_t ext_lenght);
        bool send_rmap_data_pending(uint8_t slaveIstance, uint32_t timeout_us,
                                rmap_service_setmode_1_0 paramRequest);
        bool send_rmap_data(CanardNodeID node_id, uint8_t transfer_id, CanardPortID port_id,
                                rmap_service_setmode_1_0 paramRequest);
        bool send_command_file_server_pending(uint8_t slaveIstance, uint32_t timeout_us);

        // ****************************************************************************************
        //  Sottoclassi SLAVE remoti per gestione di variabili di Stato, pending, retry e il resto
        // ****************************************************************************************
        class slave        
        {

            public:

            bool is_online(void);
            bool is_entered_online(void);
            bool is_entered_offline(void);
            #ifdef USE_SUB_PUBLISH_SLAVE_DATA
            void configure(CanardNodeID node_id, Module_Type module_type,
                        CanardPortID rmap_port_id, CanardPortID rmap_subject_id);
            #else
            void configure(CanardNodeID node_id, Module_Type module_type, CanardPortID rmap_port_id);
            #endif
            CanardNodeID get_node_id(void);
            Module_Type get_module_type(void);

            class heartbeat {

                public:

                bool is_online(void);
                bool is_entered_online(void);
                bool is_entered_offline(void);
                void set_online(uint32_t dead_line_us, uint8_t status, uint8_t healt,
                    uint8_t mode, uint32_t up_time);
                uint8_t get_healt(void);
                uint8_t get_mode(void);
                Power_Mode get_power_mode(void);
                uint32_t get_up_time(void);
                bool get_fw_uploading(void);
                bool get_data_ready(void);
                bool get_module_ready(void);
                bool get_module_error(void);

                private:

                bool _is_online_evt;            // OnLine event Flag, utilizzato per entrata in Online/Offline
                heartbeat_VSC _heartLocalVSC;   // Vendor specific code NodeSlave State (Flag Srtima V4)
                uint8_t _healt;                 // Uavcan healt_state remoto (Ok,warning,critical HeapRAM)
                uint8_t _mode;                  // Uavcan mode remoto (Init,Nominal,Update...)
                uint32_t _up_time;              // Time in secondi module Start (controlla reboot)
                uint64_t _timeout_us;           // timeout comando

            } heartbeat;

            class pnp {

                public:

                bool is_configured(void);
                void disable(void);
                void enable(void);

                private:

                bool _is_configured;          // flag di PNP in funzione

            } pnp;

            // Comandi inviati da locale Tx->(Comando + Param) Rx<-(Risposta)
            class command {

                public:

                uint8_t  get_response(void);
                void     start_pending(uint32_t timeout_us);
                void     reset_pending(void);
                void     reset_pending(uint8_t response);
                bool     event_timeout(void);
                bool     is_pending(void);
                bool     is_executed(void);
                uint8_t  next_transfer_id(void);

                private:

                uint8_t  _response;              // Stato di risposta ai comandi nodo
                uint64_t _timeout_us;            // Time command Remoto x Verifica deadLine Request
                bool     _is_pending;            // Funzione in pending (inviato, attesa risposta o timeout)
                bool     _is_executed;           // Comando eseguito
                uint8_t  _next_transfer_id;      // Transfer ID associato alla funzione

            } command;

            // Accesso ai registri inviati da locale Tx->(Registro + Value) Rx<-(Risposta)
            class register_access {

                public:

                uavcan_register_Value_1_0 get_response(void);
                void     start_pending(uint32_t timeout_us);
                void     reset_pending(void);
                void     reset_pending(uavcan_register_Value_1_0 response);
                bool     event_timeout(void);
                bool     is_executed(void);
                bool     is_pending(void);
                uint8_t  next_transfer_id(void);

                private:

                uavcan_register_Value_1_0 _response; // Valore in risposta al registro x Set (R/W)
                uint64_t _timeout_us;            // Time command Remoto x Verifica deadLine Request
                bool     _is_pending;            // Funzione in pending (inviato, attesa risposta o timeout)
                bool     _is_executed;           // Comando eseguito
                uint8_t  _next_transfer_id;      // Transfer ID associato alla funzione

            } register_access;

            // Accesso ai dati dello slave in servizio Tx->(Funzione) Rx<-(Dato + Stato)
            // Puntatore alla struttura dati relativa es. -> rmap_module_TH_1_0 ecc...
            class rmap_service {

                public:

                void     set_port_id(CanardPortID);
                CanardPortID get_port_id(void);
                void     set_module_type(Module_Type module_type);
                Module_Type get_module_type(void);
                void*    get_response(void);
                void     start_pending(uint32_t timeout_us);
                void     reset_pending(void);
                void     reset_pending(void *response, size_t len_response);
                bool     event_timeout(void);
                bool     is_executed(void);
                bool     is_pending(void);
                uint8_t  next_transfer_id(void);

                private:

                void*    _response;             // Dati e stato di risposta ai dati nodo (dati di modulo)
                CanardPortID _port_id;          // Porta del servizio dati correlato
                Module_Type _module_type;       // Tipologia del modulo di servizio dati correlato
                uint64_t _timeout_us;           // Time command Remoto x Verifica deadLine Request
                bool     _is_pending;           // Funzione in pending (inviato, attesa risposta o timeout)
                bool     _is_executed;          // Comando eseguito
                uint8_t  _next_transfer_id;     // Transfer ID associato alla funzione

            } rmap_service;

            // Nome file(locale) per aggiornamento file remoto e relativo stato di funzionamento
            class file_server {

                public:

                void     set_file_name(char* file_name, bool is_firmware);
                char*    get_file_name(void);
                FileServer_State get_state(void);
                FileServer_State next_state(void);
                bool     is_firmware(void);
                void     end_transmission(void);
                void     start_pending(uint32_t timeout_us);
                void     reset_pending(void);
                void     reset_pending(size_t len_response);
                bool     event_timeout(void);
                bool     is_executed(void);
                bool     is_pending(void);

                private:

                FileServer_State _state;         // Stato del file server
                char     _filename[FILE_NAME_SIZE_MAX]; // Nome del file presentato allo slave
                bool     _is_firmware;           // Comunico se file in TX è un Firmware o altro
                uint64_t _timeout_us;            // Time command Remoto x Verifica deadLine Request
                bool     _is_pending;            // Funzione in pending (inviato, attesa risposta o timeout)
                bool     _is_executed;           // Comando eseguito

            } file_server;

            // Pubblicazione dei dati autonoma (struttura dati come per servizio)
            #ifdef USE_SUB_PUBLISH_SLAVE_DATA
            class publisher {

                public:

                void set_subject_id(CanardPortID subject_id);
                CanardPortID get_subject_id(void);

                private:

                CanardPortID _subject_id;        // Suject id associato alla pubblicazione

            } publisher;
            #endif

            private:

            CanardNodeID _node_id;

        } slave[MAX_NODE_CONNECT];

        // Abilitazione delle pubblicazioni falcoltative sulla rete (ON/OFF a richiesta)
        class publisher_enabled
        {
            public:

            bool port_list;

        } publisher_enabled;

        // Tranfer ID (CAN Interfaccia ID -> uint8) servizi attivi del modulo locale
        class next_transfer_id
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
        class flag
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

        uint8_t getSlaveIstanceFromId(CanardNodeID nodeId);
        uint8_t getPNPValidIdFromNodeType(Module_Type node_type);

        void set_canard_node_id(CanardNodeID local_id);

    // ***************** PRIVATE ACCESS *****************
    private:

        // Istanza del modulo canard
        CanardInstance _canard;

        // Buffer dati e trasmissione delle code di tx e quella di rx
        CanardRxQueue _canard_rx_queue;
        CanardTxQueue _canard_tx_queues[CAN_REDUNDANCY_FACTOR];

        // Timings var per getMicros();
        inline static uint32_t _lastMicros;
        inline static uint64_t _currMicros;        
        inline static uint64_t _syncMicros;

        // Funzioni di utility private (sezione publish list_message)
        void _fillSubscriptions(const CanardTreeNode* const tree, uavcan_node_port_SubjectIDList_0_1* const obj);
        void _fillServers(const CanardTreeNode* const tree, uavcan_node_port_ServiceIDList_0_1* const obj);

        // Canard O1HEAP, Gestita RAM e CallBack internamente alla classe
        O1HeapInstance* _heap;
        _Alignas(O1HEAP_ALIGNMENT) uint8_t _heap_arena[1024 * 16];

        // Gestione O1Heap Static Funzioni x Canard Memory Allocate/Free
        static void* _memAllocate(CanardInstance* const ins, const size_t amount);
        static void  _memFree(CanardInstance* const ins, void* const pointer);        

        O1HeapDiagnostics _memGetDiagnostics(void);

        // Indirizzo della funzione di CallBack Esterna su Rx Messaggio Canard
        bool _attach_rx_callback;
        void (*_attach_rx_callback_PTR) (canardClass&, const CanardRxTransfer*);

        // Gestione subscription locali
        CanardRxSubscription _rxSubscription[MAX_SUBSCRIPTION];
        uint8_t _rxSubscriptionIdx;
};
