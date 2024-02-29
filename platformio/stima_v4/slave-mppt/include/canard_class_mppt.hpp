/**
  ******************************************************************************
  * @file    canard_class_mppt.hpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   Uavcan Canard Class LibCanard, bxCan, o1Heap
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Stimav4 is Copyright (C) 2023 ARPAE-SIMC urpsim@arpae.it</center></h2>
  * <h2><center>All rights reserved.</center></h2>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the GNU General Public License
  * as published by the Free Software Foundation; either version 2
  * of the License, or (at your option) any later version.
  * 
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  * 
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
  * <http://www.gnu.org/licenses/>.
  * 
  ******************************************************************************
*/

// Configurazione modulo, definizioni ed utility generiche
#include "canard_config.hpp"
#include "local_typedef.h"
// Arduino
#include <Arduino.h>
// Libcanard
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
#include <rmap/_module/Power_1_0.h>
#include <rmap/service/_module/Power_1_0.h>

#ifndef _CANARD_CLASS_H
#define _CANARD_CLASS_H

class canardClass {

    // ***************** PUBLIC ACCESS *****************
    public:

        // ********************   Tipi di Dati    *****************

        // Tipologie elaborazioni/sensori modulo(i)
        enum Sensor_Type : uint8_t {
            dep
        };

        // Modalità di accesso a getMicros()
        enum GetMonotonicTime_Type : uint8_t {
            syncronized_time,
            start_syncronization
        };

        // Gestione porte e subject di Canard
        enum Introspection_Port : uint8_t {
            PublisherSubjectID,
            SubscriptionSubjectID,
            ClientPortID,
            ServicePortID
        };

        // Gestione comandi privati di Canard / Rmap
        enum Command_Private : uint8_t {
            download_file             =  5,
            calibrate_accelerometer   =  6,
            module_maintenance        =  7,
            reset_flags               =  8,
            enable_publish_rmap       = 10,
            disable_publish_rmap      = 11,
            enable_publish_port_list  = 12,
            disable_publish_port_list = 13,
            remote_test               = 99,
            remote_test_value         = 100
        };

        // Interprete heartBeat VSC (Vendor Status Code) x Comunicazioni messaggi Master<->Slave
        typedef union {
            // Bit field
            struct {
                Power_Mode  powerMode   : 2;
                uint8_t     traceLog    : 1;
                uint8_t     fwUploading : 1;
                uint8_t     dataReady   : 1;
                uint8_t     moduleReady : 1;
                uint8_t     moduleError : 1;
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
        void setReceiveMessage_CB (void (*ptrFunction) (canardClass&, const CanardRxTransfer*, void *), void *param);
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

        // *************************************************
        //  Sottoclassi MASTER locale per gestione di Stato
        // *************************************************
        class master
        {
            public:

            // Heartbeat Online e Stato
            class heartbeat {

                public:

                bool is_online(bool is_heart_syncronized);
                void set_online(uint32_t dead_line_us);
                CanardMicrosecond last_online(void);

                private:

                uint64_t _timeout_us;

            } heartbeat;

            // Time stamp
            class timestamp {

                public:

                bool check_valid_syncronization(uint8_t current_transfer_id,
                                        CanardMicrosecond previous_tx_timestamp_us);
                CanardMicrosecond get_timestamp_syncronized(CanardMicrosecond current_rx_timestamp_us,
                                            CanardMicrosecond previous_tx_timestamp_us);
                CanardMicrosecond get_timestamp_syncronized();
                CanardMicrosecond update_timestamp_message(CanardMicrosecond current_rx_timestamp_us,
                                        CanardMicrosecond previous_tx_timestamp_us);
                private:

                CanardMicrosecond _syncronized_timestamp_us;
                CanardMicrosecond _previous_syncronized_timestamp_us;
                CanardMicrosecond _previous_local_timestamp_us;
                CanardMicrosecond _previous_msg_monotonic_us;
                uint8_t _previous_transfer_id;

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
                char     _filename[CAN_FILE_NAME_SIZE_MAX];
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
        //  Comandi Server per gestione del modulo locale tramite classe locale e pending o Diretti
        // ****************************************************************************************
        bool slave_heartbeat_send_message(void);
        bool slave_pnp_send_request(uint64_t serial_number);
        bool slave_servicelist_send_message(void);

        // *************************************************
        //  Sottoclassi e dati locali per gestione di Stato
        // *************************************************

        // Dati e Metadati del modulo locale
        rmap_module_Power_1_0 module_mppt;

        // Subject ID porte e servizi modulo locale
        class port_id
        {
            public:

            CanardPortID publisher_module_mppt;
            CanardPortID service_module_mppt;

        } port_id;

        // Abilitazione delle pubblicazioni falcoltative sulla rete (ON/OFF a richiesta)
        class publisher_enabled
        {
            public:

            bool module_mppt;
            bool port_list;

        } publisher_enabled;

        // Tranfer ID (CAN Interfaccia ID -> uint8) servizi attivi del modulo locale
        class next_transfer_id
        {
            public:

            uint8_t uavcan_node_heartbeat(void);
            uint8_t uavcan_node_port_list(void);
            uint8_t uavcan_pnp_allocation(void);
            uint8_t uavcan_file_read_data(void);
            uint8_t module_mppt(void);

            private:

            uint8_t _uavcan_node_heartbeat;
            uint8_t _uavcan_node_port_list;
            uint8_t _uavcan_pnp_allocation;
            uint8_t _uavcan_file_read_data;
            uint8_t _module_mppt;

        } next_transfer_id;

        // Flag di stato
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

        // Local address
        void set_canard_node_id(CanardNodeID local_id);
        CanardNodeID get_canard_node_id(void);
        bool is_canard_node_anonymous(void);
        // Master address
        void set_canard_master_id(CanardNodeID remote_id);
        CanardNodeID get_canard_master_id(void);

    // ***************** PRIVATE ACCESS *****************
    private:

        // Istanza del modulo canard
        CanardInstance _canard;

        // Node ID del MASTER remoto utilizzato per riferimento Set Flag e Comandi locali
        // automatici come gestione flag modalità power, Sleep, errori ecc.
        CanardNodeID _master_id;

        // Buffer dati e trasmissione delle code di tx e quella di rx
        CanardRxQueue _canard_rx_queue;
        CanardTxQueue _canard_tx_queues[CAN_REDUNDANCY_FACTOR];

        // Timings var per getMicros();
        inline static uint32_t _lastMicros;
        inline static uint64_t _currMicros;
        inline static uint64_t _syncMicros;
        inline static uint64_t _mastMicros;

        // Funzioni di utility private (sezione publish list_message)
        void _fillSubscriptions(const CanardTreeNode* const tree, uavcan_node_port_SubjectIDList_0_1* const obj);
        void _fillServers(const CanardTreeNode* const tree, uavcan_node_port_ServiceIDList_0_1* const obj);

        // Canard O1HEAP, Gestita RAM e CallBack internamente alla classe
        O1HeapInstance* _heap;
        _Alignas(O1HEAP_ALIGNMENT) uint8_t _heap_arena[HEAP_ARENA_SIZE];

        // Gestione O1Heap Static Funzioni x Canard Memory Allocate/Free
        static void* _memAllocate(CanardInstance* const ins, const size_t amount);
        static void  _memFree(CanardInstance* const ins, void* const pointer);

        O1HeapDiagnostics _memGetDiagnostics(void);

        // Indirizzo della funzione di CallBack Esterna su Rx Messaggio Canard
        bool _attach_rx_callback;
        void (*_attach_rx_callback_PTR) (canardClass&, const CanardRxTransfer*, void *);
        void *_attach_param_PTR;

        // Gestione subscription locali
        CanardRxSubscription _rxSubscription[MAX_SUBSCRIPTION];
        uint8_t _rxSubscriptionIdx;
};

#endif
