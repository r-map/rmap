/**
  ******************************************************************************
  * @file    canard_class_th.cpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   Uavcan Canard Class LibCanard, bxCan, o1Heap
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (C) 2022  Moreno Gasperini <m.gasperini@digiteco.it>
  * All rights reserved.</center></h2>
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

#include "drivers/module_slave_hal.hpp"
#include "canard_class_th.hpp"
#include "canard_config.hpp"
#include "bxcan.h"

// Callback per ISR Routine HAL STM32
/// @brief ISR di sistema CAN1_RX0_IRQHandler HAL_CAN_IRQHandler STM32
/// @param  None
canardClass::CanardRxQueue *__CAN1_RX0_IRQHandler_PTR;
extern "C" void CAN1_RX0_IRQHandler(void) {
    #if (ENABLE_CAN)
    HAL_CAN_IRQHandler(&hcan1);
    #endif
}

// ***************** ISR READ RX CAN_BUS, BUFFER RX SETUP ISR, CALLBACK *****************
// Gestita come coda FIFO (In sostituzione interrupt bxCAN non funzionante correttamente)
extern "C" void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    uint8_t testElement = __CAN1_RX0_IRQHandler_PTR->wr_ptr + 1;
    if(testElement >= CAN_RX_QUEUE_CAPACITY) testElement = 0;
    // Leggo il messaggio già pronto per libreria CANARD (Frame) e Inserisco in Buffer RX
    if (bxCANPop(IFACE_CAN_IDX,
        &__CAN1_RX0_IRQHandler_PTR->msg[testElement].frame.extended_can_id,
        &__CAN1_RX0_IRQHandler_PTR->msg[testElement].frame.payload_size,
        __CAN1_RX0_IRQHandler_PTR->msg[testElement].buf)) {
        if(testElement != __CAN1_RX0_IRQHandler_PTR->rd_ptr) {
            // Non posso registrare il dato (MAX_QUEUE) se (testElement == _canard_rx_queue.rd_ptr)
            // raggiunto MAX Buffer. E' più importante non perdere il primo FIFO payload
            // Quindi non aggiungo il dato ma leggo per svuotare il Buffer FIFO
            // altrimenti rientro sempre in Interrupt RX e mando in stallo la CPU senza RX...
            // READ DATA BUFFER MSG ->
            // Get payload from Buffer (possibilie inizializzazione statica fissa)
            // Il Buffer non cambia indirizzo quindi basterebbe un'init statico di frame[x].payload
            __CAN1_RX0_IRQHandler_PTR->msg[testElement].frame.payload =
            __CAN1_RX0_IRQHandler_PTR->msg[testElement].buf;
            // Push data in queue (Next_WR, Data in testElement + 1 Element from RX)
            __CAN1_RX0_IRQHandler_PTR->wr_ptr = testElement;
        }
    }
}

// ********************************************************************************
//                 Contructor Init Class Canard, O1Heap Mem Ptr ISR
// ********************************************************************************

/// @brief Contruttore della classe Canard
canardClass::canardClass() {

    // CallBack RX Messaggi
    _attach_rx_callback_PTR = NULL;
    _attach_rx_callback = false;
    _attach_param_PTR = NULL;

    // Sottoscrizioni
    _rxSubscriptionIdx = 0;

    // Init Timing
    _lastMicros = micros();
    _currMicros = _lastMicros;
    _syncMicros = _lastMicros;

    // Init O1Heap arena base access ram Canard
    _heap = o1heapInit(_heap_arena, sizeof(_heap_arena));
    LOCAL_ASSERT(NULL != _heap);

    // Setup CanardIstance, SET Memory function Allocate, Free and set Canard Mem reference
    _canard = canardInit(_memAllocate, _memFree);
    _canard.user_reference = this;
    _canard.node_id = CANARD_NODE_ID_UNSET;

    // Interrupt vector CB to Class PTR
    memset(&_canard_rx_queue, 0, sizeof(_canard_rx_queue));
    __CAN1_RX0_IRQHandler_PTR = &_canard_rx_queue;

    // Init memory (0) per le sotto strutture dati che necessitano un INIT_MEM_RESET
    memset(&master, 0, sizeof(master));
    memset(&next_transfer_id, 0, sizeof(next_transfer_id));
    memset(&flag, 0, sizeof(flag));

    // Inizializzazioni di sicurezza
    master.file.download_end();

    // Inizializzazioni locali da module_config.h
    port_id.publisher_module_th = UINT16_MAX;
    port_id.service_module_th = UINT16_MAX;

    publisher_enabled.module_th = DEFAULT_PUBLISH_MODULE_DATA;
    publisher_enabled.port_list = DEFAULT_PUBLISH_PORT_LIST;

    // Configura il trasporto dal registro standard uavcan. Capacità e CANARD_MTU_MAX
    // I parametri sono fissi e configurabili dal file di configurazione
    for (uint8_t ifidx = 0; ifidx < CAN_REDUNDANCY_FACTOR; ifidx++) {
        _canard_tx_queues[ifidx] = canardTxInit(CAN_TX_QUEUE_CAPACITY, CANARD_MTU_MAX);
    }
}

// ***************************************************************
//       Funzioni Timing sincronizzazione e gestione Canard
// ***************************************************************

/// @brief Get MonotonicTime Interno realTime
/// @return Microsecondi correnti realTime di Canard
CanardMicrosecond canardClass::getMicros() {
    // Start Syncro o real_time
    uint32_t ts = micros();
    if(ts > _lastMicros) {
        // Standard micosecond successivo
        _currMicros += (ts - _lastMicros);
    } else if(ts < _lastMicros) {
        // Overflow registro microsecond
        _currMicros += (0xFFFFFFFFul - _lastMicros + 1 + ts);
    }
    // Backup current register micro
    _lastMicros = ts;
    return _currMicros;
}

/// @brief Get MonotonicTime Interno con richiesta del tipo di sincronizzazione Overloading di RealTime
/// @param syncro_type Tipo di funzione richiesta (sincronizzato, inizializza sincronizzazione)
/// @return Microsecondi correnti parametrizzato dalla richiesta
CanardMicrosecond canardClass::getMicros(GetMonotonicTime_Type syncro_type) {
    // Time sincronizzato o da sincrtonizzare
    if(syncro_type == start_syncronization) _syncMicros = getMicros();
    return _syncMicros;
}

/// @brief Ritorna i secondi dall'avvio Canard per UpTime in  (in formato 64 BIT necessario per UAVCAN)
///         Non permette il reset n ei 70 minuti circa previsti per l'overflow della funzione uS a 32 Bit
/// @param  None
/// @return Secondi dall'avvio di Canard (per funzioni Heartbeat o altri scopi)
uint32_t canardClass::getUpTimeSecond(void) {
    return (uint32_t)(_currMicros / MEGA);
}

// ***************************************************************************
//   Gestione Coda messaggi in trasmissione (ciclo di svuotamento messaggi)
// ***************************************************************************

/// @brief Send messaggio Canard con daeadline del tempo sincronizzato
/// @param tx_deadline_usec microsecondi di validità del messaggio
/// @param metadata metadata del messaggio
/// @param payload_size dimensione del messaggio
/// @param payload messaggio da trasmettere
void canardClass::send(const CanardMicrosecond tx_deadline_usec,
                        const CanardTransferMetadata* const metadata,
                        const size_t payload_size,
                        const void* const payload) {
    for (uint8_t ifidx = 0; ifidx < CAN_REDUNDANCY_FACTOR; ifidx++) {
        (void)canardTxPush(&_canard_tx_queues[ifidx],
                            &_canard,
                            _syncMicros + tx_deadline_usec,
                            metadata,
                            payload_size,
                            payload);
    }
}

/// @brief Send risposta al messaggio Canard con daeadline del tempo sincronizzato
/// @param tx_deadline_usec microsecondi di validità del messaggio
/// @param metadata metadata del messaggio
/// @param payload_size dimensione del messaggio
/// @param payload messaggio da trasmettere
void canardClass::sendResponse(const CanardMicrosecond tx_deadline_usec,
                                const CanardTransferMetadata* const request_metadata,
                                const size_t payload_size,
                                const void* const payload) {
    CanardTransferMetadata meta = *request_metadata;
    meta.transfer_kind = CanardTransferKindResponse;
    send(tx_deadline_usec, &meta, payload_size, payload);
}

/// @brief Test coda presenza dati in trasmissione di Canard
/// @param  None
/// @return true se ci sono dati da trasmettere
bool canardClass::transmitQueueDataPresent(void) {
    // Transmit pending frames from the prioritized TX queues managed by libcanard.
    for (uint8_t ifidx = 0; ifidx < CAN_REDUNDANCY_FACTOR; ifidx++)
    {
        CanardTxQueue* const     que = &_canard_tx_queues[ifidx];
        const CanardTxQueueItem* tqi = canardTxPeek(que);  // Find the highest-priority frame.
        if(tqi != NULL) return true;
    }
    return false;
}

/// @brief Trasmette la coda con timeStamp sincronizzato. Per inviare con real_time va aggiornata
///         la sincronizzazione prima della chiamata al relativo metodo di getMonotonicMicrosecond()
/// @param  None
void canardClass::transmitQueue(void) {
    // Transmit pending frames from the prioritized TX queues managed by libcanard.
    for (uint8_t ifidx = 0; ifidx < CAN_REDUNDANCY_FACTOR; ifidx++)
    {
        CanardTxQueue* const     que = &_canard_tx_queues[ifidx];
        const CanardTxQueueItem* tqi = canardTxPeek(que);  // Find the highest-priority frame.
        while (tqi != NULL)
        {
            // Delay Microsecond di sicurezza in Send (Migliora sicurezza RX Pacchetti)
            // Da utilizzare con CPU poco performanti in RX o con controllo Polling gestito Canard
            // N.B. Funziona perfettamente con i Nodi ma utilizzando Yakut è comunque necessario un USB/CAN
            // che utilizzi un'interrupt o molto performanete, altrimenti vengono persi pacchetti in TX
            // e questo comporta uina perdità dei messaggi RX in ricezione dal nodo (master/slave corrente)
            #if (CAN_DELAY_US_SEND > 0)
            delayMicroseconds(CAN_DELAY_US_SEND);
            #endif
            // Attempt transmission only if the frame is not yet timed out while waiting in the TX queue.
            // Otherwise just drop it and move on to the next one.
            if ((tqi->tx_deadline_usec == 0) || (tqi->tx_deadline_usec > _syncMicros)) {
                // Non-blocking write attempt.
                if (bxCANPush(0,
                    _syncMicros,
                    tqi->tx_deadline_usec,
                    tqi->frame.extended_can_id,
                    tqi->frame.payload_size,
                    tqi->frame.payload)) {
                    // Push CAN data
                    _canard.memory_free(&_canard, canardTxPop(que, tqi));
                    tqi = canardTxPeek(que);
                } else  {
                    // Empty Queue
                    break;
                }
            } else {
                // loop continuo per mancato aggiornamento monotonic_time su TIME_OUT
                // grandi quantità di dati trasmesse e raggiunto il TIMEOUT Subscription...
                // Remove frame per blocco in timeout BUG trasmission security !!!
                _canard.memory_free(&_canard, canardTxPop(que, tqi));
                // Test prossimo pacchetto
                tqi = canardTxPeek(que);
            }
        }
    }
}

// ***************************************************************
// Gestione Buffer memorizzazione RX BxCAN per Canard metodo FIFO
// ***************************************************************

/// @brief Azzera il buffer di ricezione dati collegato a BxCAN e ISR Interrupt
/// @param  None
void canardClass::receiveQueueEmpty(void) {
    _canard_rx_queue.wr_ptr=_canard_rx_queue.rd_ptr;
}

/// @brief Ritorna true se ci sono dati utili da gestire dal buffer di RX per BxCAN
/// @param  None
/// @return true se il buffer non è vuoto.
bool canardClass::receiveQueueDataPresent(void) {
    return _canard_rx_queue.wr_ptr!=_canard_rx_queue.rd_ptr;
}

/// @brief Ritorna l'elemento corrente del buffer di BxCAN, pronto ad essere gestito con Canard
/// @param  None
/// @return Indice dell'elemento ricevuto
uint8_t canardClass::receiveQueueElement(void) {
    if(_canard_rx_queue.wr_ptr>=_canard_rx_queue.rd_ptr) {
        return _canard_rx_queue.wr_ptr-_canard_rx_queue.rd_ptr;
    } else {
        return _canard_rx_queue.wr_ptr+(CAN_RX_QUEUE_CAPACITY-_canard_rx_queue.rd_ptr);
    }
}

/// @brief Ritorna il prossimo elemento del buffer
/// @param currElement elemento corrente gestito
/// @return prossimo elemento della coda FIFO
uint8_t canardClass::receiveQueueNextElement(uint8_t currElement) {
    if(currElement + 1 < CAN_RX_QUEUE_CAPACITY) return currElement + 1;
    return 0;
}

/// @brief Gestione metodo ricezione coda messaggi dal buffer FIFO preparato di BxCAN
///         Il buffer gestito nella ISR CAN_Rx viene passato alla libreria Canard e in automatico
///         è gestita la richiamata di callBack per la funzione esterna di gestione su Rx Messaggi conformi
/// @param  None
void canardClass::receiveQueue(void) {
    // Leggo l'elemento disponibile in coda BUFFER RX FiFo CanardFrame + Buffer
    uint8_t getElement = receiveQueueNextElement(_canard_rx_queue.rd_ptr);
    _canard_rx_queue.rd_ptr = getElement;
    // Passaggio CanardFrame Buffered alla RxAccept CANARD
    // DeadLine a partire dal realTime assoluto
    const CanardMicrosecond timestamp_usec = getMicros();
    CanardRxTransfer        transfer;
    const int8_t canard_result = canardRxAccept(&_canard, timestamp_usec, &_canard_rx_queue.msg[getElement].frame, IFACE_CAN_IDX, &transfer, NULL);
    if (canard_result > 0) {
        _attach_rx_callback_PTR(*this, &transfer, _attach_param_PTR);
        _canard.memory_free(&_canard, (void*) transfer.payload);
    } else if ((canard_result == 0) || (canard_result == -CANARD_ERROR_OUT_OF_MEMORY)) {
        (void) 0;  // The frame did not complete a transfer so there is nothing to do.
        // OOM should never occur if the heap is sized correctly. You can track OOM errors via heap API.
    } else {
        LOCAL_ASSERT(false);  // No other error can possibly occur at runtime.
    }
}

/// @brief Gestione metodo ricezione coda messaggi dal buffer FIFO preparato di BxCAN
///         Il buffer gestito nella ISR CAN_Rx viene passato alla libreria Canard e in automatico
///         è gestita la richiamata di callBack per la funzione esterna di gestione su Rx Messaggi conformi
///         Overloading per log dei messaggi
/// @param logMessage Buffer esterno per la gestione del messaggio di log
void canardClass::receiveQueue(char *logMessage) {
    // Leggo l'elemento disponibile in coda BUFFER RX FiFo CanardFrame + Buffer
    uint8_t getElement = receiveQueueNextElement(_canard_rx_queue.rd_ptr);
    _canard_rx_queue.rd_ptr = getElement;
    // *****************************************************************************
    logMessage = itoa(_canard_rx_queue.msg[getElement].frame.payload_size, logMessage, 10);
    logMessage += 1;
    strcpy(logMessage, ",Val:");
    logMessage += 5;
    for(int iIdxPl=0; iIdxPl<_canard_rx_queue.msg[getElement].frame.payload_size; iIdxPl++) {
        strcpy(logMessage, " 0x");
        logMessage += 3;
        if(_canard_rx_queue.msg[getElement].buf[iIdxPl] < 16) {
            strcpy(logMessage, "0");
            logMessage++;
            strcpy(logMessage, itoa(_canard_rx_queue.msg[getElement].buf[iIdxPl], logMessage, HEX));
            logMessage++;
        } else {
            strcpy(logMessage, itoa(_canard_rx_queue.msg[getElement].buf[iIdxPl], logMessage, HEX));
            logMessage+=2;
        }
    }
    *logMessage = 0;
    // *****************************************************************************
    // Passaggio CanardFrame Buffered alla RxAccept CANARD
    // DeadLine a partire dal realTime assoluto
    const CanardMicrosecond timestamp_usec = getMicros();
    CanardRxTransfer        transfer;
    const int8_t canard_result = canardRxAccept(&_canard, timestamp_usec, &_canard_rx_queue.msg[getElement].frame, IFACE_CAN_IDX, &transfer, NULL);
    if (canard_result > 0) {
        _attach_rx_callback_PTR(*this, &transfer, _attach_param_PTR);
        _canard.memory_free(&_canard, (void*) transfer.payload);
    } else if ((canard_result == 0) || (canard_result == -CANARD_ERROR_OUT_OF_MEMORY)) {
        (void) 0;  // The frame did not complete a transfer so there is nothing to do.
        // OOM should never occur if the heap is sized correctly. You can track OOM errors via heap API.
    } else {
        LOCAL_ASSERT(false);  // No other error can possibly occur at runtime.
    }
}

/// @brief Setta il CallBack Function per il processo esterno di interprete su messaggio ricevuto
///         e conforme a Canard. Richiama la funzione esterna su CanardRxAccept Valido. Abilitato su SET
/// @param ptrFunction puntatore alla funzione di callBack(canardClass&, const CanardRxTransfer*)
void canardClass::setReceiveMessage_CB(void (*ptrFunction) (canardClass&, const CanardRxTransfer*, void *param), void *param) {
    _attach_rx_callback_PTR = ptrFunction;
    _attach_rx_callback = true;
    _attach_param_PTR = param;
}

/// @brief Abilita il CallBack Function Canard RX Message CanardRxAccept
/// @param  None
void canardClass::enableReceiveMessage_CB(void) {
    _attach_rx_callback = true;
}

/// @brief Disabilita il CallBack Function Canard RX Message CanardRxAccept
/// @param  None
void canardClass::disableReceiveMessage_CB(void) {
    _attach_rx_callback = false;
}

// ***************************************************************************
//                 Gestione sottoscrizioni messaggi Canard
// ***************************************************************************

/// @brief Sottoscrizione di una funzione Canard
/// @param transfer_kind tipo di messaggio
/// @param port_id porta del messaggio
/// @param extent dimensione del pacchetto dati
/// @param transfer_id_timeout_usec daeadline di validità messaggio in microsecondi
/// @return true se la sottoscrizione è avvenuta
bool canardClass::rxSubscribe(
        const CanardTransferKind    transfer_kind,
        const CanardPortID          port_id,
        const size_t                extent,
        const CanardMicrosecond     transfer_id_timeout_usec) {
    // Controllo limiti di sottoscrizioni
    if(_rxSubscriptionIdx >= MAX_SUBSCRIPTION) return false;
    // Aggiunge rxSubscription con spazio RAM interna alla classe
    const int8_t res = canardRxSubscribe(&_canard,
        transfer_kind,
        port_id,
        extent,
        transfer_id_timeout_usec,
        &_rxSubscription[_rxSubscriptionIdx++]);
    return (res>=0);
}

/// @brief Ritorna il numero di sottoscrizioni ancora disponibili
/// @return Numero di sottoscrizioni libere
uint8_t canardClass::rxSubscriptionAvaiable() {
    return MAX_SUBSCRIPTION - _rxSubscriptionIdx;
}

/// @brief Rimozione di una sottoscrizione di una funzione Canard
/// @param transfer_kind tipo di messaggio
/// @param port_id porta del messaggio
void canardClass::rxUnSubscribe(
        const CanardTransferKind    transfer_kind,
        const CanardPortID          port_id) {
    (void)canardRxUnsubscribe(&_canard, transfer_kind, port_id);
}

// *************************************************************************************
//  GESTIONE TIME OUT E SERVIZI RETRY ED ALTRE UTILITY FUNZIONI DI CLASSE MASTER REMOTA
// *************************************************************************************

/// @brief Avvia la richiesta di un blocco file contiguo al file_server tramite classe e controllo Master
/// @param timeout_us deadline di comando
/// @return true se il metodo è eseguito correttamente
bool canardClass::master_file_read_block_pending(uint32_t timeout_us)
{
    // Accedo alla sezione master con il controllo pending locale
    master.file.start_pending(timeout_us);
    // Invio la richiesta standard
    return send_file_read_block(master.file.get_server_node(), master.file.get_name(),
        master.file.get_offset_rx());
}

/// @brief Avvia la richiesta di un blocco file contiguo al file_server con ID Fisico
/// @param server_id remote file_server
/// @param fileName Nome del file da prelevare
/// @param remoteOffset offset di prelievo del blocco file remoto
/// @return true se il metodo è eseguito correttamente
bool canardClass::send_file_read_block(CanardNodeID server_id, char * fileName, uint64_t remoteOffset)
{
    // FileRead V1.1 Handle Message
    // ***** Ricezione di file generico dalla rete UAVCAN dal nodo chiamante *****
    // Richiamo in continuazione rapida la funzione fino al riempimento del file
    // Alla fine processo il firmware Upload (eventuale) vero e proprio con i relativi check
    uavcan_file_Read_Request_1_1 remotefile = {0};
    remotefile.path.path.count = strlen(fileName);
    memcpy(remotefile.path.path.elements, fileName, remotefile.path.path.count);
    remotefile.offset = remoteOffset;

    uint8_t      serialized[uavcan_file_Read_Request_1_1_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
    size_t       serialized_size                                                           = sizeof(serialized);
    const int8_t err = uavcan_file_Read_Request_1_1_serialize_(&remotefile, &serialized[0], &serialized_size);
    LOCAL_ASSERT(err >= 0);
    if (err >= 0)
    {
        const CanardTransferMetadata meta = {
            .priority       = CanardPriorityHigh,
            .transfer_kind  = CanardTransferKindRequest,
            .port_id        = uavcan_file_Read_1_1_FIXED_PORT_ID_,
            .remote_node_id = server_id,
            .transfer_id    = (CanardTransferID) (next_transfer_id.uavcan_file_read_data()),
        };
        // Messaggio standard ad un secondo dal timeStamp Sincronizzato
        send(MEGA, &meta, serialized_size, &serialized[0]);
        return true;
    }
    return false;
}

// *******************************************************************************
//  GESTIONE TIME OUT E SERVIZI RETRY ED ALTRE UTILITY FUNZIONI DI CLASSE MASTER
// *******************************************************************************

// ******************************** HEART BEAT ***********************************

/// @brief Controlla se il modulo master è online
/// @param  None
/// @return true se il mater remoto è correttamente onLine (ha comunicato) nel limite di tempo valido
bool canardClass::master::heartbeat::is_online(void) {
    return _syncMicros < _timeout_us;
}

/// @brief Imposta il nodo OnLine, richiamato in heartbeat o altre comunicazioni client
/// @param dead_line_us validità di tempo us a partire dal time_stamp sincronizzato interno
void canardClass::master::heartbeat::set_online(uint32_t dead_line_us) {
    _timeout_us = _syncMicros + dead_line_us;
}

// ******************************** TIME STAMP ***********************************

/// @brief Controlla se messaggio valido (coerenza sequenza e validità di time_stamp)
/// @param current_transfer_id Indice di messaggio
/// @param previous_tx_timestamp_us timestamp precedente locale
/// @return true se messaggio è decodificabile con timestamp corretto
bool canardClass::master::timestamp::check_valid_syncronization(uint8_t current_transfer_id,
                                            CanardMicrosecond previous_tx_timestamp_us) {
    // Controllo coerenza messaggio per consentire l'aggiornamento timestamp
    // 1) Sequenza di transfer_id con previous
    // 2) differenza di tempo tra due timestamp remoti inferiore al massimo timeOut impostato di controllo
    if((++_previous_transfer_id == current_transfer_id) &&
        ((previous_tx_timestamp_us - _previous_msg_monotonic_us) < MASTER_MAXSYNCRO_VALID_US))
        // Riferimento locale
        return true;
    // Risincronizzo il transferID Corretto per sicurezza
    _previous_transfer_id = current_transfer_id;
    return false;
}

/// @brief Legge il tempo sincronizzato dal master, utilizzabile dopo controllo di validità del messaggio
/// @param current_rx_timestamp_us time stamp reale del messaggio entrante (al tempo di RX)
/// @param previous_tx_timestamp_us time stamp remoto attuale, riferito al precedente invio remoto di time stamp
/// @return il tempo sincronizzato, regolato ed aggiustato al microsecondo, con l'unità master
CanardMicrosecond canardClass::master::timestamp::get_timestamp_syncronized(CanardMicrosecond current_rx_timestamp_us,
                                            CanardMicrosecond previous_tx_timestamp_us) {
    // Save timestamp variabili per SET e controllo Time nella successiva chiamata time_syncronization
    // Local RealTime RX timestamp monotonic locale al tempo reale di rx_message (transfer->timestamp_usec)
    _syncronized_timestamp_us = previous_tx_timestamp_us + current_rx_timestamp_us - _previous_local_timestamp_us;
    // Aggiusto e sommo la differenza tra il timestamp reale del messaggio ricevuto con il monotonic reale attuale (al tempo di esecuzione syncro real)
    _previous_syncronized_timestamp_us = getMicros();
    _syncronized_timestamp_us += (_previous_syncronized_timestamp_us - current_rx_timestamp_us);
    return _syncronized_timestamp_us;
}

/// @brief Legge il tempo sincronizzato dal master, in Overload, regolato sui microsecondi locali
///         a partire dall'ultima sincronizzazione valida. Utilizzabile come FAKE_RTC() dopo una
///         ricezione di almeno un messagguo valido
/// @param  None
/// @return il tempo sincronizzato, regolato ed aggiustato al microsecondo, con l'unità master
CanardMicrosecond canardClass::master::timestamp::get_timestamp_syncronized(void) {
    CanardMicrosecond realtime_us = getMicros();
    CanardMicrosecond diff_synconized_us = realtime_us - _previous_syncronized_timestamp_us;
    _previous_syncronized_timestamp_us = realtime_us;
    _syncronized_timestamp_us += diff_synconized_us;
    return _syncronized_timestamp_us;
}

/// @brief Aggiorna i time stamp della classe sul messaggio standard UAVCAN per le successive sincronizzazioni
/// @param current_rx_timestamp_us time stamp reale del messaggio entrante (al tempo di RX)
/// @param previous_tx_timestamp_us time stamp remoto attuale, riferito al precedente invio remoto di time stamp
/// @return
CanardMicrosecond canardClass::master::timestamp::update_timestamp_message(CanardMicrosecond current_rx_timestamp_us,
                                CanardMicrosecond previous_tx_timestamp_us) {
    // Differenza tra due messaggi in microsecondi
    CanardMicrosecond difference_timestamp_us = previous_tx_timestamp_us - _previous_msg_monotonic_us;
    // Save timestamp variabili per SET e controllo Time nella successiva chiamata time_syncronization
    // Local RealTime RX timestamp monotonic locale al tempo reale di rx_message (transfer->timestamp_usec)
    _previous_local_timestamp_us = current_rx_timestamp_us;
    // RealTime callBack message RX (TX RTC uSec(Canard type) Master Remoto al tempo di send)
    _previous_msg_monotonic_us = previous_tx_timestamp_us;
    return difference_timestamp_us;
}

// ******************************** FILE CLIENT ***********************************

/// @brief Avvia una richiesta remota di download file ed inizializza le risorse nella classe
/// @param remote_node nodo di provenienza della richiesta, controllo messaggi in coerenza con il nodo
/// @param param_request_name nome del file (parametro request->element entrante)
/// @param param_request_len lunghezza del nome del file (parametro request->count)
/// @param is_firmware imposta il flag che indica se è un file firmware
void canardClass::master::file::start_request(uint8_t remote_node, uint8_t *param_request_name,
                                    uint8_t param_request_len, bool is_firmware) {
    _node_id = remote_node;
    // Copio la stringa nel name file firmware disponibile su state generale (per download successivo)
    memcpy(_filename, param_request_name, param_request_len);
    _filename[param_request_len] = '\0';
    // Init varaiabili di download
    _is_firmware = is_firmware;
    _updating = true;
    _updating_eof = false;
    // Start Offset File
    _offset = 0;
    _updating_retry = 0;
}

/// @brief Nome del file in download
/// @param  None
/// @return puntatore e buffer al nome del file per il dowloading. Non necessità di inizializzazione buffer.
char* canardClass::master::file::get_name(void) {
    return _filename;
}

/// @brief Legge il nodo master file server che richiede il caricamento del file
/// @param  None
/// @return node_id UAVCAN
CanardNodeID canardClass::master::file::get_server_node(void) {
    return _node_id;
}

/// @brief Gestione file, verifica richiesta di download da un nodo remoto
/// @param  None
/// @return true se è in corso una procedura di ricezione file (start comando o download)
bool canardClass::master::file::download_request(void) {
    return _updating;
}

/// @brief Gestione file, fine con successo del download file da un nodo remoto.
///         Il nodo non risponde più alle richieste del comando fino a nuovo restart.
/// @param  None
/// @return true se è finita la procedura di ricezione file con successo
bool canardClass::master::file::is_download_complete(void) {
    return _updating_eof;
}

/// @brief Gestione file, abbandona o termina richiesta di download da un nodo remoto.
///         Il nodo non risponde più alle richieste del comando fino a nuovo restart.
/// @param  None
/// @return true se è in corso una procedura di ricezione file (start comando o download)
void canardClass::master::file::download_end(void) {
    _updating = false;
    _node_id = CANARD_NODE_ID_UNSET;
}

/// @brief Controlla se il file è di tipo firmware o altra tipologia
/// @param  None
/// @return true se il file in download è di tipo firmware
bool canardClass::master::file::is_firmware(void)
{
    // Riferimento locale
    return _is_firmware;
}

/// @brief Legge l'offset corrente
/// @param  None
/// @return l'offste corrente del file attualmente in download
uint64_t canardClass::master::file::get_offset_rx(void) {
    return _offset;
}

/// @brief Imposta l'offset RX per la lettura di un blocco specifico del file in donload remoto
/// @param remote_file_offset indirizzo offset del byte da leggere
void canardClass::master::file::set_offset_rx(uint64_t remote_file_offset) {
    _offset = remote_file_offset;
}

/// @brief Verifica se è il primo blocco di un file. Da utilizzare per rewrite file o init E2Prom Space
/// @param  None
/// @return true se il blocco ricevuto è il primo del file
bool canardClass::master::file::is_first_data_block(void) {
    return _offset == 0;
}

/// @brief Gestione automatica totale retry del comando file all'interno della classe
///         MAX retry è gestito nel file di configurazione module_config.h
/// @param  None
/// @return true se ci sono ancora retry disponibili per il comando
bool canardClass::master::file::next_retry(void) {
    if (++_updating_retry > NODE_GETFILE_MAX_RETRY) _updating_retry = NODE_GETFILE_MAX_RETRY;
    // Reset pending state
    _is_pending = false;
    return _updating_retry < NODE_GETFILE_MAX_RETRY;
}

/// @brief Gestione automatica totale retry del comando file all'interno della classe
///         MAX retry è gestito nel file di configurazione module_config.h
/// @param avaiable_retry retry ancora disponibili
/// @return true se ci sono ancora retry disponibili per il comando
bool canardClass::master::file::next_retry(uint8_t *avaiable_retry) {
    if (++_updating_retry > NODE_GETFILE_MAX_RETRY) _updating_retry = NODE_GETFILE_MAX_RETRY;
    *avaiable_retry = NODE_GETFILE_MAX_RETRY - _updating_retry;
    // Reset pending state
    _is_pending = false;
    return _updating_retry < NODE_GETFILE_MAX_RETRY;
}

/// @brief Avvia un comando per il metodo corrente con timeOut di validità
/// @param timeout_us microsecondi per entrata in timeOut di mancata risposta
void canardClass::master::file::start_pending(uint32_t timeout_us)
{
    // Riferimento locale
    _is_pending = true;
    _timeout_us = _syncMicros + timeout_us;
}

/// @brief Resetta lo stato dei flag pending per il metodo corrente
/// @param None
void canardClass::master::file::reset_pending(void)
{
    _is_pending = false;
    // Azzero contestualmente le retry di controllo x gestione MAX_RETRY -> ABORT
    _updating_retry = 0;
}

/// @brief Resetta lo stato dei flag pending per il metodo corrente
/// @param message_len lunghezza del messaggio ricevuto in risposta
void canardClass::master::file::reset_pending(size_t message_len)
{
    _is_pending = false;
    // Azzero contestualmente le retry di controllo x gestione MAX_RETRY -> ABORT
    _updating_retry = 0;
    // Gestisco internamente l'offset di RX File
    _offset += message_len;
    // Overload per controllo EOF (se lunghezza messaggio != MAX_ARRAY_UAVCAN ... 256 Bytes)
    _updating_eof = (message_len != uavcan_primitive_Unstructured_1_0_value_ARRAY_CAPACITY_);
}

/// @brief Gestione timeout pending file. Controlla il raggiungimento del timeout
/// @param  None
/// @return true se entrata in timeout del comano
bool canardClass::master::file::event_timeout(void)
{
    if(_is_pending) return _syncMicros > _timeout_us;
    return false;
}

/// @brief Verifica se un comando per il relativo modulo è in attesa. Diventerà false o verrà attivato il timeout
/// @param None
/// @return true se un comando è in attesa
bool canardClass::master::file::is_pending(void)
{
    return _is_pending;
}

// ***********************************************************************************
//  GESTIONE TIME OUT E SERVIZI RETRY ED ALTRE UTILITY FUNZIONI DI CLASSE LOCALE SLAVE
// ***********************************************************************************

/// @brief Invia il messaggio di HeartBeat ai nodi remoti
/// @param  None
/// @return true se il metodo è eseguito correttamente
bool canardClass::slave_heartbeat_send_message(void)
{
    // ***** Trasmette alla rete UAVCAN lo stato haeartbeat del modulo *****
    // Heartbeat Fisso anche per modulo Master (Visibile a yakut o altri tools/script gestionali)
    uavcan_node_Heartbeat_1_0 heartbeat = {0};
    heartbeat.uptime = getUpTimeSecond();
    const O1HeapDiagnostics heap_diag = _memGetDiagnostics();
    if (heap_diag.oom_count > 0) {
        heartbeat.health.value = uavcan_node_Health_1_0_CAUTION;
    } else {
        heartbeat.health.value = uavcan_node_Health_1_0_NOMINAL;
    }
    // Stato di heartbeat gestito dalla classe
    heartbeat.vendor_specific_status_code = flag.get_local_value_heartbeat_VSC();
    heartbeat.mode.value = flag.get_local_node_mode();
    // Trasmetto il pacchetto
    uint8_t serialized[uavcan_node_Heartbeat_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
    size_t serialized_size = sizeof(serialized);
    const int8_t err = uavcan_node_Heartbeat_1_0_serialize_(&heartbeat, &serialized[0], &serialized_size);
    LOCAL_ASSERT(err >= 0);
    if (err >= 0) {
        const CanardTransferMetadata meta = {
            .priority = CanardPriorityNominal,
            .transfer_kind = CanardTransferKindMessage,
            .port_id = uavcan_node_Heartbeat_1_0_FIXED_PORT_ID_,
            .remote_node_id = CANARD_NODE_ID_UNSET,
            .transfer_id = (CanardTransferID)(next_transfer_id.uavcan_node_heartbeat()),
        };
        // Messaggio standard ad un secondo dal timeStamp Sincronizzato
        send(MEGA, &meta, serialized_size, &serialized[0]);
        return true;
    }
    return false;
}

/// @brief Invia il messaggio di PNP request (richiesta di node_id valido) al nodo server PNP (master)
/// @param  None
/// @return true se il metodo è eseguito correttamente
bool canardClass::slave_pnp_send_request(uint64_t serial_number) {
    // PnP over Classic CAN, use message v1.0.
    uavcan_pnp_NodeIDAllocationData_1_0 msg = {0};
    // truncated uint48 unique_id_hash
    // msg.allocated_node_id.(count/element) => Solo in response non in request;
    msg.unique_id_hash = serial_number;
    uint8_t serialized[uavcan_pnp_NodeIDAllocationData_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
    size_t serialized_size = sizeof(serialized);
    const int8_t err = uavcan_pnp_NodeIDAllocationData_1_0_serialize_(&msg, &serialized[0], &serialized_size);
    LOCAL_ASSERT(err >= 0);
    if (err >= 0) {
        const CanardTransferMetadata meta = {
            .priority = CanardPrioritySlow,
            .transfer_kind = CanardTransferKindMessage,
            .port_id = uavcan_pnp_NodeIDAllocationData_1_0_FIXED_PORT_ID_,
            .remote_node_id = CANARD_NODE_ID_UNSET,
            .transfer_id = (CanardTransferID) (next_transfer_id.uavcan_pnp_allocation()),
        };
        // Messaggio standard ad un secondo dal timeStamp Sincronizzato
        send(MEGA, &meta, serialized_size, &serialized[0]);
        return true;
    }
    return false;
}

/// @brief Prepara la lista delle sottoscrizioni Canard (privata)
/// @param tree out albero sottoscrizioni
/// @param obj oggetto sottoscrizioni
void canardClass::_fillSubscriptions(const CanardTreeNode* const tree, uavcan_node_port_SubjectIDList_0_1* const obj)
{
    if (NULL != tree) {
        _fillSubscriptions(tree->lr[0], obj);
        const CanardRxSubscription* crs = (const CanardRxSubscription*)tree;
        if (crs->port_id <= CANARD_SUBJECT_ID_MAX) {
            LOCAL_ASSERT(obj->sparse_list.count < uavcan_node_port_SubjectIDList_0_1_sparse_list_ARRAY_CAPACITY_);
            obj->sparse_list.elements[obj->sparse_list.count++].value = crs->port_id;
            _fillSubscriptions(tree->lr[1], obj);
        }
    }
}

/// @brief Prepara la lista dei servizi Canard (privata)
/// @param tree out albero servizi
/// @param obj oggetto servizi
void canardClass::_fillServers(const CanardTreeNode* const tree, uavcan_node_port_ServiceIDList_0_1* const obj)
{
    if (NULL != tree) {
        _fillServers(tree->lr[0], obj);
        const CanardRxSubscription* crs = (const CanardRxSubscription*)tree;
        if (crs->port_id <= CANARD_SERVICE_ID_MAX) {
            (void)nunavutSetBit(&obj->mask_bitpacked_[0], sizeof(obj->mask_bitpacked_), crs->port_id, true);
            _fillServers(tree->lr[1], obj);
        }
    }
}

/// @brief Invia il messaggio dei servizi attivi ai nodi remoti
/// @param  None
/// @return true se il metodo è eseguito correttamente
bool canardClass::slave_servicelist_send_message(void)
{
    // Publish the recommended (not required) port introspection message. No point publishing it if we're anonymous.
    // The message is a bit heavy on the stack (about 2 KiB) but this is not a problem for a modern MCU.
    // L'abilitazione del comando è facoltativa, può essere attivata/disattivata da un comando UAVCAN
    if ((publisher_enabled.port_list) &&
        (_canard.node_id <= CANARD_NODE_ID_MAX))
    {
        uavcan_node_port_List_0_1 m = {0};
        uavcan_node_port_List_0_1_initialize_(&m);
        uavcan_node_port_SubjectIDList_0_1_select_sparse_list_(&m.publishers);
        uavcan_node_port_SubjectIDList_0_1_select_sparse_list_(&m.subscribers);

        // Indicate which subjects we publish to. Don't forget to keep this updated if you add new publications!
        {
            size_t* const cnt = &m.publishers.sparse_list.count;
            m.publishers.sparse_list.elements[(*cnt)++].value = uavcan_node_Heartbeat_1_0_FIXED_PORT_ID_;
            m.publishers.sparse_list.elements[(*cnt)++].value = uavcan_node_port_List_0_1_FIXED_PORT_ID_;
            // Aggiungo i publisher interni validi privati (quando abilitati)
            if ((port_id.publisher_module_th <= CANARD_SUBJECT_ID_MAX)&&
                (publisher_enabled.module_th))
            {
                m.publishers.sparse_list.elements[(*cnt)++].value = port_id.publisher_module_th;
            }
        }

        // Indicate which servers and subscribers we implement.
        // We could construct the list manually but it's easier and more robust to just query libcanard for that.
        _fillSubscriptions(_canard.rx_subscriptions[CanardTransferKindMessage], &m.subscribers);
        _fillServers(_canard.rx_subscriptions[CanardTransferKindRequest], &m.servers);
        _fillServers(_canard.rx_subscriptions[CanardTransferKindResponse], &m.clients);  // For regularity.

        // Serialize and publish the message. Use a small buffer because we know that our message is always small.
        // Verificato massimo utilizzo a 156 bytes. Limitiamo il buffer a 256 Bytes (Come esempio UAVCAN)
        uint8_t serialized[256] = {0};
        size_t  serialized_size = uavcan_node_port_List_0_1_SERIALIZATION_BUFFER_SIZE_BYTES_;
        if (uavcan_node_port_List_0_1_serialize_(&m, &serialized[0], &serialized_size) >= 0)
        {
            const CanardTransferMetadata meta = {
                .priority       = CanardPriorityOptional,  // Mind the priority.
                .transfer_kind  = CanardTransferKindMessage,
                .port_id        = uavcan_node_port_List_0_1_FIXED_PORT_ID_,
                .remote_node_id = CANARD_NODE_ID_UNSET,
                .transfer_id    = (CanardTransferID) (next_transfer_id.uavcan_node_port_list()),
            };
            // Send a 2 secondi
            send(MEGA * 2, &meta, serialized_size, &serialized[0]);
            return true;
        }
    }
    return false;
}

// ***************************************************************
//              ID di Trasferimento UAVCAN Canard
// ***************************************************************

/// @brief Gestione transfer ID UAVCAN per la classe relativa
/// @param  None
/// @return Prossimo transfer_id valido in standard UAVCAN
uint8_t canardClass::next_transfer_id::uavcan_node_heartbeat(void) {
    return next_transfer_id::_uavcan_node_heartbeat++;
}

/// @brief Gestione transfer ID UAVCAN per la classe relativa
/// @param  None
/// @return Prossimo transfer_id valido in standard UAVCAN
uint8_t canardClass::next_transfer_id::uavcan_node_port_list(void) {
    return next_transfer_id::_uavcan_node_port_list++;
}

/// @brief Gestione transfer ID UAVCAN per la classe relativa
/// @param  None
/// @return Prossimo transfer_id valido in standard UAVCAN
uint8_t canardClass::next_transfer_id::uavcan_pnp_allocation(void) {
    return next_transfer_id::_uavcan_pnp_allocation++;
}

/// @brief Gestione transfer ID UAVCAN per la classe relativa
/// @param  None
/// @return Prossimo transfer_id valido in standard UAVCAN
uint8_t canardClass::next_transfer_id::uavcan_file_read_data(void) {
    return next_transfer_id::_uavcan_file_read_data++;
}

/// @brief Gestione transfer ID UAVCAN per la classe relativa
/// @param  None
/// @return Prossimo transfer_id valido in standard UAVCAN
uint8_t canardClass::next_transfer_id::module_th(void) {
    return next_transfer_id::_module_th++;
}

// ***************************************************************
//           Funzioni ed Utility Canard su Local Flag
// ***************************************************************

// ********       COMMAND REMOTI E Stati gestionali       ********

/// @brief Avvia una richiesta standard UAVCAN per il riavvio del sistema
/// @param  None
void canardClass::flag::request_system_restart(void) {
    _restart_required = true;
}

/// @brief Verifica una richiesta di riavvio del sistema standard UAVCAN.
/// @param  None
/// @return true se è stata avanzata una richiesta di riavvio
bool canardClass::flag::is_requested_system_restart(void) {
    return _restart_required;
}

/// @brief Avvia la richiesta di sleep del modulo. Da chiamare prima di attivare il basso consumo generale
/// @param  None
void canardClass::flag::request_sleep(void) {
    // Con inibizione, non permetto lo sleep del modulo
    if(_inibith_sleep) return;
    // TODO:
    // Eseguo la procedura di messa in sleep del modulo
    // Variabili, HW CAN ecc... alla fine setto la var di entrata in sleep
    _enter_sleep = true;
}

/// @brief Verifica se attiva la funzione dello sleep del modulo Canard e hardware relativo
/// @param  None
/// @return true se il modulo è in sleep
bool canardClass::flag::is_module_sleep(void) {
    return _enter_sleep;
}

/// @brief Permetto l'attivazione sleep, funzioni ed hardware, del modulo
/// @param  None
void canardClass::flag::disable_sleep(void) {
    _inibith_sleep = true;
}

/// @brief Inibisce l'attivazione sleep, funzioni ed hardware, del modulo
/// @param  None
void canardClass::flag::enable_sleep(void) {
    _inibith_sleep = false;
}

// ********         HEARTBEAT Vendor Status Code          ********
// Set VendorStatusCode per Heratbeat in RX/TX al modulo Master/Slave

/// @brief Proprietà SET per il valore VendorStatusCode di Heartbeat e per gli utilizzi locali
///         La proprietà è impostata normalmente dal master remoto e viene settata per il locale
/// @param powerMode Modalità power (CanardClass::Power_Mode)
void canardClass::flag::set_local_power_mode(Power_Mode powerMode) {
    _heartLocalVSC.powerMode = powerMode;
}

/// @brief Proprietà SET per il valore VendorStatusCode di Heartbeat e per gli utilizzi locali
/// @param fwUploading true se attivata la funzionalità di firmware uploading
void canardClass::flag::set_local_fw_uploading(bool fwUploading) {
    _heartLocalVSC.fwUploading = fwUploading;
}

/// @brief Proprietà SET per il valore VendorStatusCode di Heartbeat e per gli utilizzi locali
/// @param dataReady true se sono disponibili dati del modulo da presentare al master tramite SensorDrive
void canardClass::flag::set_local_data_ready(bool dataReady) {
    _heartLocalVSC.dataReady = dataReady;
}

/// @brief Proprietà SET per il valore VendorStatusCode di Heartbeat e per gli utilizzi locali
/// @param moduleReady true se il modulo è pronto alle funzioni standard (start complete o fine manutenzione)
void canardClass::flag::set_local_module_ready(bool moduleReady) {
    _heartLocalVSC.moduleReady = moduleReady;
}

/// @brief Proprietà SET per il valore VendorStatusCode di Heartbeat e per gli utilizzi locali
/// @param moduleError true se il modulo è in errore HW
void canardClass::flag::set_local_module_error(bool moduleError) {
    _heartLocalVSC.moduleError = moduleError;
}

/// @brief Proprietà GET per il valore VendorStatusCode di Heartbeat e per gli utilizzi locali
/// @param  None
/// @return Modalità power (CanardClass::Power_Mode)
canardClass::Power_Mode canardClass::flag::get_local_power_mode(void) {
    return _heartLocalVSC.powerMode;
}

/// @brief Proprietà GET per il valore VendorStatusCode di Heartbeat e per gli utilizzi locali
/// @param  None
/// @return true se attivata la funzionalità di firmware uploading
bool canardClass::flag::get_local_fw_uploading(void) {
    return _heartLocalVSC.fwUploading;
}

/// @brief Proprietà GET per il valore VendorStatusCode di Heartbeat e per gli utilizzi locali
/// @param  None
/// @return true se sono disponibili dati del modulo da presentare al master tramite SensorDrive
bool canardClass::flag::get_local_data_ready(void) {
    return _heartLocalVSC.dataReady;
}

/// @brief Proprietà GET per il valore VendorStatusCode di Heartbeat e per gli utilizzi locali
/// @param  None
/// @return true se il modulo è pronto alle funzioni standard (start complete o fine manutenzione)
bool canardClass::flag::get_local_module_ready(void) {
    return _heartLocalVSC.moduleReady;
}

/// @brief Proprietà GET per il valore VendorStatusCode di Heartbeat e per gli utilizzi locali
/// @param  None
/// @return true se il modulo è in errore HW
bool canardClass::flag::get_local_module_error(void) {
    return _heartLocalVSC.moduleError;
}

/// @brief Proprietà GET per il valore VendorStatusCode di Heartbeat e per gli utilizzi locali
/// @param  None
/// @return
uint8_t canardClass::flag::get_local_value_heartbeat_VSC(void) {
    return _heartLocalVSC.uint8_val;
}

// Funzioni Locali per gestione interna NODE MODE x HeartBeat standard UAVCAN

/// @brief Imposta la modalità nodo standard UAVCAN, gestita nelle funzioni heartbeat
/// @param heartLocalMODE Modalità local node standard di UAVCAN
void canardClass::flag::set_local_node_mode(uint8_t heartLocalMODE) {
    _heartLocalMODE = heartLocalMODE;
}

/// @brief Ritorna la modalità node mode locale standard UAVCAN per la gestione heartbeat
/// @param  None
/// @return modalità node mode di UAVCAN
uint8_t canardClass::flag::get_local_node_mode(void) {
    return _heartLocalMODE;
}

// ***************************************************************
//           Funzioni ed Utility di Classe Generali
// ***************************************************************

/// @brief Imposta l'ID Nodo locale per l'istanza Canard privata della classe Canard
/// @param local_id id nodo locale
void canardClass::set_canard_node_id(CanardNodeID local_id) {
    // Istanza del modulo canard
    _canard.node_id = local_id;
}

/// @brief Legge l'ID Nodo locale per l'istanza Canard privata della classe Canard
/// @param None
/// @return id nodo locale
CanardNodeID canardClass::get_canard_node_id(void) {
    // Istanza del modulo canard
    return _canard.node_id;
}

/// @brief Controlla se il nodo è anonimo (senza ID Impostato)
/// @param  None
/// @return true se il nodo è anonimo (ID Non valido o non Settato)
bool canardClass::is_canard_node_anonymous(void) {
    // Istanza del modulo canard
    return _canard.node_id > CANARD_NODE_ID_MAX;
}

// ***************************************************************
//     Wrapper C++ O1Heap memory Access & Function CB Private
// ***************************************************************

/// @brief Gestione O1Heap Memory Canard Allocate
/// @param ins istanza canard
/// @param amount dimensione da allocare
/// @return puntatore alla O1Heap allineata
void* canardClass::_memAllocate(CanardInstance* const ins, const size_t amount) {
    O1HeapInstance* const heap = ((canardClass*)ins->user_reference)->_heap;
    LOCAL_ASSERT(o1heapDoInvariantsHold(heap));
    return o1heapAllocate(heap, amount);
}

/// @brief Gestione O1Heap Memory Canard Free
/// @param ins istanza canard
/// @param pointer puntatore alla posizione da deallocare
void canardClass::_memFree(CanardInstance* const ins, void* const pointer) {
    O1HeapInstance* const heap = ((canardClass*)ins->user_reference)->_heap;
    o1heapFree(heap, pointer);
}

/// @brief Diagnostica Heap Data per Heartbeat e/o azioni interne
/// @param  None
/// @return O1HeapDiagnostics Info
O1HeapDiagnostics canardClass::_memGetDiagnostics(void) {
    return o1heapGetDiagnostics(_heap);
}
