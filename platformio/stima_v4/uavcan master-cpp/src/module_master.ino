/// This software is distributed under the terms of the MIT License.
/// Copyright (C) 2021 OpenCyphal <maintainers@opencyphal.org>
/// Author: Pavel Kirienko <pavel@opencyphal.org>
/// Revis.: Gasperini Moreno <m.gasperini@digiteco.it>

/// TODO: (ricerca e ricontrolla i TODO vari)
/// STAMPE CORRETTE E A DEFINE_SOFTWARE E -> LEVEL COME ESEMPIO DI PAOLO
/// DSDL Esatta e conforme status, command, Lx, ecc. versione FINALE e tutte le altre slave
/// C++

/// NOTE:
/// msg->vendor_specific_status_code in Heartbeat può indicare (Fine acq, + altri STATI locali)
///     Errori, bassa alimentazione 0/1/2, Ho finito acq. in che modalità sono ecc... GIA OK!!!
///     Va solo gestito in logica master ma funziona correttamente
/// Capire L1/L2 quali parametri sono in master... x SET Misura ecc...
/// Funzionalità gestibili entrata in OffLine, uscita in OnLine (Master e Slave)
/// Funzionalità gestibili tutti i timeOut, OK ciclo STD ma altri possibili (Master e Slave)

// Arduino
#include <Arduino.h>
// Classe Canard
#include "canardClass_master.hpp"
// Libcanard
#include "register.hpp"
#include <o1heap.h>
#include <canard.h>
#include "bxcan.h"
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
// Standard Library
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
// Configurazione modulo, definizioni ed utility generiche
#include "module_config.hpp"

// ***************************************************************************************************
// **********             Funzioni ed utility generiche per gestione UAVCAN                 **********
// ***************************************************************************************************

// Ritorna l'indice della coda master allocata in state in funzione del nodeId fisico
byte getQueueNodeFromId(canardClass &clsCanard, CanardNodeID nodeId) {
    // Cerco la corrispondenza node_id nella coda allocata master per ritorno queueID Index
    for(byte queueId=0; queueId<MAX_NODE_CONNECT; queueId++) {
        // Se trovo il nodo che sta rispondeno nella coda degli allocati...
        if(clsCanard.slave[queueId].node_id == nodeId) {
            return queueId;
        } 
    }
    return GENERIC_BVAL_UNDEFINED;
}

// Ritorna l'indice della coda master allocata in state in funzione del nodeId fisico
byte getPNPValidIdFromQueueNode(canardClass &clsCanard, uint8_t node_type) {
    // Cerco la corrispondenza node_id nella coda allocata master per ritorno queueID Index
    for(byte queueId=0; queueId<MAX_NODE_CONNECT; queueId++) {
        // Se trovo il nodo che sta pubblicando come node_type
        // nella coda dei nodi configurati ma non ancora allocati...
        if((clsCanard.slave[queueId].node_type == node_type)&&
            (clsCanard.slave[queueId].pnp.is_assigned == false)) {
            // Ritorno il NodeID configurato da remoto come default da associare
            return clsCanard.slave[queueId].node_id;
        } 
    }
    return GENERIC_BVAL_UNDEFINED;
}

// Ritorna unique-ID 128-bit del nodo locale. E' utilizzato in uavcan.node.GetInfo.Response e durante
// plug-and-play node-ID allocation da uavcan.pnp.NodeIDAllocationData. SerialNumber, Produttore..
// Dovrebbe essere verificato in uavcan.node.GetInfo.Response per la verifica non sia cambiato Nodo.
// Al momento vengono inseriti 2 BYTE fissi, altri eventuali, che Identificano il Tipo Modulo
static void getUniqueID(uint8_t out[uavcan_node_GetInfo_Response_1_0_unique_id_ARRAY_CAPACITY_])
{
    // A real hardware node would read its unique-ID from some hardware-specific source (typically stored in ROM).
    // This example is a software-only node so we store the unique-ID in a (read-only) register instead.
    uavcan_register_Value_1_0 value = {0};
    uavcan_register_Value_1_0_select_unstructured_(&value);
    // Crea default unique_id con NODE_TYPE_MAJOR (Tipo di nodo), MINOR (Hw relativo)
    // Il resto dei 128 Bit (112) vengono impostati RANDOM (potrebbero portare Manufactor, SerialNumber ecc...)
    // Dovrebbe essere l'ID per la verifica incrociata del corretto Node_Id dopo il PnP
    value.unstructured.value.elements[value.unstructured.value.count++] = (uint8_t) NODE_TYPE_MAJOR;
    value.unstructured.value.elements[value.unstructured.value.count++] = (uint8_t) NODE_TYPE_MINOR;
    for (uint8_t i = value.unstructured.value.count; i < uavcan_node_GetInfo_Response_1_0_unique_id_ARRAY_CAPACITY_; i++)
    {
        value.unstructured.value.elements[value.unstructured.value.count++] = (uint8_t) rand();  // NOLINT
    }
    registerRead("uavcan.node.unique_id", &value);
    assert(uavcan_register_Value_1_0_is_unstructured_(&value) &&
           value.unstructured.value.count == uavcan_node_GetInfo_Response_1_0_unique_id_ARRAY_CAPACITY_);
    memcpy(&out[0], &value.unstructured.value, uavcan_node_GetInfo_Response_1_0_unique_id_ARRAY_CAPACITY_);
}

// ***********************************************************************************************
// ***********************************************************************************************
//      FUNZIONI CHIAMATE DA MAIN_LOOP DI PUBBLICAZIONE E RICHIESTE DATI E SERVIZI
// ***********************************************************************************************
// ***********************************************************************************************

// *******              FUNZIONI INVOCATE HANDLE CONT_LOOP EV. PREPARATORIE              *********

// FileRead V1.1
static void handleFileReadBlock_1_1(canardClass &clsCanard)
{
    // ***** Ricezione di file generico dalla rete UAVCAN dal nodo chiamante *****
    // Richiamo in continuazione rapida la funzione fino al riempimento del file
    // Alla fine processo il firmware Upload (eventuale) vero e proprio con i relativi check
    uavcan_file_Read_Request_1_1 remotefile = {0};
    remotefile.path.path.count = strlen(clsCanard.master.file.get_name());
    memcpy(remotefile.path.path.elements, clsCanard.master.file.get_name(), remotefile.path.path.count);
    remotefile.offset = clsCanard.master.file.get_offset_rx();

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
            .remote_node_id = clsCanard.master.file.get_server_node(),
            .transfer_id    = (CanardTransferID) (clsCanard.next_transfer_id.uavcan_file_read_data()),
        };
        // Messaggio standard ad un secondo dal timeStamp Sincronizzato
        clsCanard.send(MEGA, &meta, serialized_size, &serialized[0]);
    }
}

// *******              FUNZIONI INVOCATE HANDLE 1 SECONDO EV. PREPARATORIE              *********

static void handleSyncroLoop(canardClass &clsCanard)
{    
    // ***** Trasmette alla rete UAVCAN lo stato syncronization_time del modulo *****
    // Da specifica invio il timestamp dell'ultima chiamata in modo che slave sincronizzi il delta
    uavcan_time_Synchronization_1_0 timesyncro;
    timesyncro.previous_transmission_timestamp_microsecond = clsCanard.master.timestamp.get_previous_tx_real(true);
    uint8_t      serialized[uavcan_time_Synchronization_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
    size_t       serialized_size                                                              = sizeof(serialized);
    const int8_t err = uavcan_time_Synchronization_1_0_serialize_(&timesyncro, &serialized[0], &serialized_size);
    assert(err >= 0);
    if (err >= 0)
    {
        // Traferimento immediato per sincronizzazione migliore con i nodi remoti
        // Aggiorno il time stamp su blocco trasmesso a priorità immediata
        const CanardTransferMetadata meta = {
            .priority       = CanardPriorityImmediate,
            .transfer_kind  = CanardTransferKindMessage,
            .port_id        = uavcan_time_Synchronization_1_0_FIXED_PORT_ID_,
            .remote_node_id = CANARD_NODE_ID_UNSET,
            .transfer_id    = (CanardTransferID) (clsCanard.next_transfer_id.uavcan_time_synchronization()),
        };
        clsCanard.send(MEGA, &meta, serialized_size, &serialized[0]);
    }
}

static void handleNormalLoop(canardClass &clsCanard)
{    
    // ***** Trasmette alla rete UAVCAN lo stato haeartbeat del modulo *****
    // Heartbeat Fisso anche per modulo Master (Visibile a yakut o altri tools/script gestionali)
    uavcan_node_Heartbeat_1_0 heartbeat = {0};
    heartbeat.uptime = clsCanard.getUpTimeSecond();
    const O1HeapDiagnostics heap_diag = clsCanard.memGetDiagnostics();
    if (heap_diag.oom_count > 0) {
        heartbeat.health.value = uavcan_node_Health_1_0_CAUTION;
    } else {
        heartbeat.health.value = uavcan_node_Health_1_0_NOMINAL;
    }
    // Stato di heartbeat gestito dalla classe
    heartbeat.vendor_specific_status_code = clsCanard.flag.get_local_value_heartbeat_VSC();
    heartbeat.mode.value = clsCanard.flag.get_local_node_mode();
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
            .transfer_id = (CanardTransferID)(clsCanard.next_transfer_id.uavcan_node_heartbeat()),
        };
        // Messaggio standard ad un secondo dal timeStamp Sincronizzato
        clsCanard.send(MEGA, &meta, serialized_size, &serialized[0]);
    }
}

// *******              FUNZIONI INVOCATE HANDLE 10 SECONDI EV. PREPARATORIE             *********

// Prepara lista sottoscrizioni (solo quelle allocate correttamente <= CANARD_SUBJECT_ID_MAX) uavcan_node_port_List_0_1.
static void fillSubscriptions(const CanardTreeNode* const tree, uavcan_node_port_SubjectIDList_0_1* const obj)
{
    if (NULL != tree) {
        fillSubscriptions(tree->lr[0], obj);
        const CanardRxSubscription* crs = (const CanardRxSubscription*)tree;
        if (crs->port_id <= CANARD_SUBJECT_ID_MAX) {
            assert(obj->sparse_list.count < uavcan_node_port_SubjectIDList_0_1_sparse_list_ARRAY_CAPACITY_);
            obj->sparse_list.elements[obj->sparse_list.count++].value = crs->port_id;
            fillSubscriptions(tree->lr[1], obj);
        }
    }
}

/// This is needed only for constructing uavcan_node_port_List_0_1.
static void fillServers(const CanardTreeNode* const tree, uavcan_node_port_ServiceIDList_0_1* const obj)
{
    if (NULL != tree) {
        fillServers(tree->lr[0], obj);
        const CanardRxSubscription* crs = (const CanardRxSubscription*)tree;
        if (crs->port_id <= CANARD_SERVICE_ID_MAX) {
            (void)nunavutSetBit(&obj->mask_bitpacked_[0], sizeof(obj->mask_bitpacked_), crs->port_id, true);
            fillServers(tree->lr[1], obj);
        }
    }
}

// **************           Pubblicazione vera e propria a 20 secondi           **************
static void handleSlowLoop(canardClass &clsCanard)
{
    // Publish the recommended (not required) port introspection message. No point publishing it if we're anonymous.
    // The message is a bit heavy on the stack (about 2 KiB) but this is not a problem for a modern MCU.
    // L'abilitazione del comando è facoltativa, può essere attivata/disattivata da un comando UAVCAN
    if ((clsCanard.publisher_enabled.port_list) &&
        (clsCanard.canard.node_id <= CANARD_NODE_ID_MAX))
    {
        uavcan_node_port_List_0_1 m = {0};
        uavcan_node_port_List_0_1_initialize_(&m);
        uavcan_node_port_SubjectIDList_0_1_select_sparse_list_(&m.publishers);
        uavcan_node_port_SubjectIDList_0_1_select_sparse_list_(&m.subscribers);

        // Indicate which subjects we publish to. Don't forget to keep this updated if you add new publications!
        {
            size_t* const cnt                                 = &m.publishers.sparse_list.count;
            m.publishers.sparse_list.elements[(*cnt)++].value = uavcan_node_Heartbeat_1_0_FIXED_PORT_ID_;
            m.publishers.sparse_list.elements[(*cnt)++].value = uavcan_node_port_List_0_1_FIXED_PORT_ID_;
            // Aggiungo i publisher interni validi privati
        }

        // Indicate which servers and subscribers we implement.
        // We could construct the list manually but it's easier and more robust to just query libcanard for that.
        fillSubscriptions(clsCanard.canard.rx_subscriptions[CanardTransferKindMessage], &m.subscribers);
        fillServers(clsCanard.canard.rx_subscriptions[CanardTransferKindRequest], &m.servers);
        fillServers(clsCanard.canard.rx_subscriptions[CanardTransferKindResponse], &m.clients);  // For regularity.

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
                .transfer_id    = (CanardTransferID) (clsCanard.next_transfer_id.uavcan_node_port_list()),
            };
            // Send a 2 secondi
            clsCanard.send(MEGA * 2, &meta, serialized_size, &serialized[0]);
        }
    }
}

// ************** SEZIONE COMANDI E RICHIESTE SPECIFICHE AD UN NODO SULLA RETE  **************

// **************       Invio Comando diretto ad un nodo remoto UAVCAN Cmd      **************

static bool serviceSendCommand(canardClass &clsCanard, byte istanza, const uint16_t cmd_request,
                                const void* ext_param, size_t ext_lenght)
{
    // Effettua una richiesta specifica ad un nodo della rete in formato UAVCAN
    uavcan_node_ExecuteCommand_Request_1_1 cmdRequest = {0};
    uint8_t      serialized[uavcan_node_ExecuteCommand_Request_1_1_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
    size_t       serialized_size = sizeof(serialized);

    // istanza -> queueId di State o istanza di nodo

    // Imposta il comando da inviare
    cmdRequest.command = cmd_request;
    // Verifica la presenza di parametri opzionali nel comando
    cmdRequest.parameter.count = ext_lenght;
    // Controllo conformità lunghezza messaggio e ne copio il contenuto
    assert(cmdRequest.parameter.count<=uavcan_node_ExecuteCommand_Request_1_1_parameter_ARRAY_CAPACITY_);
    if (ext_lenght) {
        memcpy(cmdRequest.parameter.elements, ext_param, ext_lenght);
    }

    // Serializzo e verifico la conformità del messaggio
    const int8_t err = uavcan_node_ExecuteCommand_Request_1_1_serialize_(&cmdRequest, &serialized[0], &serialized_size);
    assert(err >= 0);
    if (err >= 0)
    {
        // Comando a priorità alta
        const CanardTransferMetadata meta = {
            .priority       = CanardPriorityHigh,
            .transfer_kind  = CanardTransferKindRequest,
            .port_id        = uavcan_node_ExecuteCommand_1_1_FIXED_PORT_ID_,
            .remote_node_id = clsCanard.slave[istanza].node_id,
            .transfer_id    = (CanardTransferID) (clsCanard.slave[istanza].command.next_transfer_id++),
        };
        clsCanard.send(MEGA, &meta, serialized_size, &serialized[0]);
        return true;
    }
    return false;
}

// **************   Invio richiesta dati diretto ad un nodo remoto UAVCAN Get   **************
static bool serviceSendRegister(canardClass &clsCanard, byte istanza, char *registerName,
                                uavcan_register_Value_1_0 registerValue)
{
    // Effettua la richiesta UAVCAN per l'accesso ad un registro remoto di un nodo slave
    // Utile per la configurazione remota completa o la modifica di un parametro dello slave
    uavcan_register_Access_Request_1_0 cmdRequest = {0};
    uint8_t      serialized[uavcan_register_Access_Request_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
    size_t       serialized_size = sizeof(serialized);

    // Imposta il comando da inviare ed il timer base
    cmdRequest.value = registerValue;
    cmdRequest.name.name.count = strlen(registerName);
    memcpy(&cmdRequest.name.name.elements[0], registerName, cmdRequest.name.name.count);

    // Serializzo e verifico la conformità del messaggio
    const int8_t err = uavcan_register_Access_Request_1_0_serialize_(&cmdRequest, &serialized[0], &serialized_size);
    assert(err >= 0);
    if (err >= 0)
    {
        // Comando a priorità alta
        const CanardTransferMetadata meta = {
            .priority       = CanardPriorityHigh,
            .transfer_kind  = CanardTransferKindRequest,
            .port_id        = uavcan_register_Access_1_0_FIXED_PORT_ID_,
            .remote_node_id = clsCanard.slave[istanza].node_id,
            .transfer_id    = (CanardTransferID) (clsCanard.slave[istanza].register_access.next_transfer_id++),
        };
        clsCanard.send(MEGA, &meta, serialized_size, &serialized[0]);
        return true;
    }
    return false;
}

// **************   Invio richiesta dati diretto ad un nodo remoto UAVCAN Get   **************
static bool serviceSendRequestData(canardClass &clsCanard, byte istanza, byte comando, uint16_t run_sectime)
{
    // Effettua una richiesta specifica ad un nodo della rete in formato UAVCAN
    // La richiesta è generica per tutti i moduli (univoca DSDL), comunque parte integrante di ogni
    // DSDL singola di modulo. Il PORT_ID fisso o dinamico indica il nodo remoto.
    // L'interpretazione è invece tipicizzata dalla risposta (DSDL specifica)
    rmap_service_setmode_1_0 cmdRequest = {0};
    uint8_t      serialized[rmap_service_module_TH_Request_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
    size_t       serialized_size = sizeof(serialized);

    // Imposta il comando da inviare ed il timer base
    cmdRequest.comando = comando;
    cmdRequest.run_sectime = run_sectime;

    // Serializzo e verifico la conformità del messaggio
    const int8_t err = rmap_service_setmode_1_0_serialize_(&cmdRequest, &serialized[0], &serialized_size);
    assert(err >= 0);
    if (err >= 0)
    {
        // Comando a priorità alta
        const CanardTransferMetadata meta = {
            .priority       = CanardPriorityHigh,
            .transfer_kind  = CanardTransferKindRequest,
            .port_id        = clsCanard.slave[istanza].rmap_service.port_id,
            .remote_node_id = clsCanard.slave[istanza].node_id,
            .transfer_id    = (CanardTransferID) (clsCanard.slave[istanza].rmap_service.next_transfer_id++),
        };
        clsCanard.send(MEGA, &meta, serialized_size, &serialized[0]);
        return true;
    }
    return false;
}

// ***************************************************************************************************
//   Funzioni ed utility di ricezione dati dalla rete UAVCAN, richiamati da processReceivedTransfer()
// ***************************************************************************************************

// Chiamate gestioni RPC remote da master (yakut o altro servizio di controllo)
static uavcan_node_ExecuteCommand_Response_1_1 processRequestExecuteCommand(canardClass &clsCanard,
    const uavcan_node_ExecuteCommand_Request_1_1* req, uint8_t remote_node)
{
    uavcan_node_ExecuteCommand_Response_1_1 resp = {0};
    // req->command (Comando esterno ricevuto 2 BYTES RESERVED FFFF-FFFA)
    // Gli altri sono liberi per utilizzo interno applicativo con #define interne
    // req->parameter (array di byte MAX 255 per i parametri da request)
    // Risposta attuale (resp) 1 Bytes RESERVER (0..6) gli altri #define interne
    switch (req->command)
    {
        // **************** Comandi standard UAVCAN GENERIC_SPECIFIC_COMMAND ****************
        // Comando di aggiornamento Firmware compatibile con Yakut e specifice UAVCAN
        case uavcan_node_ExecuteCommand_Request_1_1_COMMAND_BEGIN_SOFTWARE_UPDATE:
        {
            // Nodo Server chiamante (Yakut solo Master, Yakut e Master per Slave)
            clsCanard.master.file.start_request(remote_node, (uint8_t*) req->parameter.elements,
                                                req->parameter.count, true);
            clsCanard.flag.set_local_fw_uploading(true);
            Serial.print(F("Firmware update request from node id: "));
            Serial.println(clsCanard.master.file.get_server_node());
            Serial.print(F("Filename to download: "));
            Serial.println(clsCanard.master.file.get_name());
            // Avvio la funzione con OK
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        case uavcan_node_ExecuteCommand_Request_1_1_COMMAND_FACTORY_RESET:
        {
            registerDoFactoryReset();
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        case uavcan_node_ExecuteCommand_Request_1_1_COMMAND_RESTART:
        {
            clsCanard.flag.request_system_restart();
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        case uavcan_node_ExecuteCommand_Request_1_1_COMMAND_STORE_PERSISTENT_STATES:
        {
            // If your registers are not automatically synchronized with the non-volatile storage, use this command
            // to commit them to the storage explicitly. Otherwise it is safe to remove it.
            // In this demo, the registers are stored in files, so there is nothing to do.
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        // **************** Comandi personalizzati VENDOR_SPECIFIC_COMMAND ****************
        // Comando di download File generico compatibile con specifice UAVCAN, (LOG/CFG altro...)
        case CMD_DOWNLOAD_FILE:
        {
            // Nodo Server chiamante (Yakut solo Master, Yakut e Master per Slave)
            clsCanard.master.file.start_request(remote_node, (uint8_t*) req->parameter.elements,
                                                req->parameter.count, false);
            clsCanard.flag.set_local_fw_uploading(true);
            Serial.print(F("File standard update request from node id: "));
            Serial.println(clsCanard.master.file.get_server_node());
            Serial.print(F("Filename to download: "));
            Serial.println(clsCanard.master.file.get_name());
            // Avvio la funzione con OK
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        case CMD_ENABLE_PUBLISH_PORT_LIST:
        {
            // Abilita pubblicazione slow_loop elenco porte (Cypal facoltativo)
            clsCanard.publisher_enabled.port_list = true;
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }
        case CMD_DISABLE_PUBLISH_PORT_LIST:
        {
            // Disabilita pubblicazione slow_loop elenco porte (Cypal facoltativo)
            clsCanard.publisher_enabled.port_list = false;
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS;
            break;
        }          
        default:
        {
            resp.status = uavcan_node_ExecuteCommand_Response_1_1_STATUS_BAD_COMMAND;
            break;
        }
    }
    return resp;
}

// Accesso ai registri UAVCAN risposta a richieste
static uavcan_register_Access_Response_1_0 processRequestRegisterAccess(const uavcan_register_Access_Request_1_0* req)
{
    char name[uavcan_register_Name_1_0_name_ARRAY_CAPACITY_ + 1] = {0};
    assert(req->name.name.count < sizeof(name));
    memcpy(&name[0], req->name.name.elements, req->name.name.count);
    name[req->name.name.count] = '\0';

    uavcan_register_Access_Response_1_0 resp = {0};

    // If we're asked to write a new value, do it now:
    if (!uavcan_register_Value_1_0_is_empty_(&req->value))
    {
        uavcan_register_Value_1_0_select_empty_(&resp.value);
        registerRead(&name[0], &resp.value);
        // If such register exists and it can be assigned from the request value:
        if (!uavcan_register_Value_1_0_is_empty_(&resp.value) && registerAssign(&resp.value, &req->value))
        {
            registerWrite(&name[0], &resp.value);
        }
    }

    // Regardless of whether we've just wrote a value or not, we need to read the current one and return it.
    // The client will determine if the write was successful or not by comparing the request value with response.
    uavcan_register_Value_1_0_select_empty_(&resp.value);
    registerRead(&name[0], &resp.value);

    // Currently, all registers we implement are mutable and persistent. This is an acceptable simplification,
    // but more advanced implementations will need to differentiate between them to support advanced features like
    // exposing internal states via registers, perfcounters, etc.
    resp._mutable   = true;
    resp.persistent = true;

    // Our node does not synchronize its time with the network so we can't populate the timestamp.
    resp.timestamp.microsecond = uavcan_time_SynchronizedTimestamp_1_0_UNKNOWN;

    return resp;
}

// Risposta a uavcan.node.GetInfo which Info Node (nome, versione, uniqueID di verifica ecc...)
static uavcan_node_GetInfo_Response_1_0 processRequestNodeGetInfo()
{
    uavcan_node_GetInfo_Response_1_0 resp = {0};
    resp.protocol_version.major           = CANARD_CYPHAL_SPECIFICATION_VERSION_MAJOR;
    resp.protocol_version.minor           = CANARD_CYPHAL_SPECIFICATION_VERSION_MINOR;

    // The hardware version is not populated in this demo because it runs on no specific hardware.
    // An embedded node would usually determine the version by querying the hardware.

    resp.software_version.major   = VERSION_MAJOR;
    resp.software_version.minor   = VERSION_MINOR;
    resp.software_vcs_revision_id = VCS_REVISION_ID;

    getUniqueID(resp.unique_id);

    // The node name is the name of the product like a reversed Internet domain name (or like a Java package).
    resp.name.count = strlen(NODE_NAME);
    memcpy(&resp.name.elements, NODE_NAME, resp.name.count);

    // The software image CRC and the Certificate of Authenticity are optional so not populated in this demo.
    return resp;
}

// ******************************************************************************************
//          CallBack di classe canardClass ( Gestisce i metodi uavcan sottoscritti )
// Processo multiplo di ricezione messaggi e comandi. Gestione entrata ed uscita dei messaggi
// Chiamata direttamente nel main loop in ricezione dalla coda RX
// Richiama le funzioni qui sopra di preparazione e risposta alle richieste
// ******************************************************************************************
static void processReceivedTransfer(canardClass &clsCanard, const CanardRxTransfer* const transfer)
{
    // Gestione dei Messaggi in ingresso
    if (transfer->metadata.transfer_kind == CanardTransferKindMessage)
    {
        // bool Per assert mancanza handler di eventuale servizio sottoscritto
        bool bKindMessageProcessed = false;
        // Gestione dei messaggi PNP per allocazione nodi di rete (gestisco come master)
        if (transfer->metadata.port_id == uavcan_pnp_NodeIDAllocationData_1_0_FIXED_PORT_ID_)
        {
            // Richiesta di allocazione Nodo dalla rete con messaggio anonimo V1.0 CAN_MTU 8
            bKindMessageProcessed = true;
            uint8_t defaultNodeId = 0;
            size_t size = transfer->payload_size;
            uavcan_pnp_NodeIDAllocationData_1_0 msg = {0};
            if (uavcan_pnp_NodeIDAllocationData_1_0_deserialize_(&msg, static_cast<uint8_t const*>(transfer->payload), &size) >= 0)
            {
                // Cerco nei moduli conosciuti (in HASH_UNIQUE_ID) invio il tipo modulo...
                // Verifico se ho un nodo ancora da configurare come da cfg del master
                // Il nodo deve essere compatibile con il tipo di modulo previsto da allocare
                Serial.print(F("RX PNP Allocation message request from -> "));
                switch(msg.unique_id_hash & 0xFF) {
                    case MODULE_TYPE_TH:
                        Serial.print(F("Anonimous module TH"));
                        defaultNodeId = getPNPValidIdFromQueueNode(clsCanard, MODULE_TYPE_TH);
                        break;
                    case MODULE_TYPE_RAIN:
                        Serial.print(F("Anonimous module RAIN"));
                        defaultNodeId = getPNPValidIdFromQueueNode(clsCanard, MODULE_TYPE_RAIN);
                        break;
                    case MODULE_TYPE_WIND:
                        Serial.print(F("Anonimous module WIND"));
                        defaultNodeId = getPNPValidIdFromQueueNode(clsCanard, MODULE_TYPE_WIND);
                        break;
                    case MODULE_TYPE_RADIATION:
                        Serial.print(F("Anonimous module RADIATION"));
                        defaultNodeId = getPNPValidIdFromQueueNode(clsCanard, MODULE_TYPE_RADIATION);
                        break;
                    case MODULE_TYPE_VWC:
                        Serial.print(F("Anonimous module VWC"));
                        defaultNodeId = getPNPValidIdFromQueueNode(clsCanard, MODULE_TYPE_VWC);
                        break;
                    case MODULE_TYPE_POWER:
                        Serial.print(F("Anonimous module POWER"));
                        defaultNodeId = getPNPValidIdFromQueueNode(clsCanard, MODULE_TYPE_POWER);
                        break;
                    defualt:
                        // PNP Non gestibile
                        Serial.print(F("Anonimous module Unknown"));
                        defaultNodeId = GENERIC_BVAL_UNDEFINED;
                        break;
                }
                Serial.print(F(" [UniqueID:"));
                Serial.print(msg.unique_id_hash, HEX);
                Serial.println(F("]"));

                // Risposta immediata diretta (Se nodo ovviamente è riconosciuto...)
                // Non utilizziamo una Response in quanto l'allocation è sempre un messaggio anonimo
                // I metadati del trasporto sono come quelli riceuti del transferID quindi è un messaggio
                // che si comporta parzialmente come una risposta (per rilevamento remoto hash/transfer_id)
                if(defaultNodeId <= CANARD_NODE_ID_MAX) {
                    Serial.print(F("Try PNP Allocation with Node_ID -> "));
                    Serial.println(defaultNodeId);
                    // Se il nodo proposto viene confermato inizieremo a ricevere heartbeat
                    // da quel nodeId. A questo punto in Heartbeat settiam il flag pnp.is_assigned
                    // che conclude la procedura con esito positivo.
                    msg.allocated_node_id.count = 1;
                    msg.allocated_node_id.elements[0].value = defaultNodeId;
                    // The request object is empty so we don't bother deserializing it. Just send the response.
                    uint8_t      serialized[uavcan_pnp_NodeIDAllocationData_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
                    size_t       serialized_size = sizeof(serialized);
                    const int8_t res = uavcan_pnp_NodeIDAllocationData_1_0_serialize_(&msg, &serialized[0], &serialized_size);
                    // Preparo la pubblicazione anonima in risposta alla richiesta anonima
                    const CanardTransferMetadata meta = {
                        .priority       = CanardPriorityNominal,
                        .transfer_kind  = CanardTransferKindMessage,
                        .port_id        = uavcan_pnp_NodeIDAllocationData_1_0_FIXED_PORT_ID_,
                        .remote_node_id = CANARD_NODE_ID_UNSET,
                        .transfer_id    = (CanardTransferID) (transfer->metadata.transfer_id),
                    };
                    clsCanard.send(MEGA, &meta, serialized_size, &serialized[0]);
                }
            }
        }
        // Gestione dei messaggi Heartbeat per stato rete (gestisco come master)
        else if (transfer->metadata.port_id == uavcan_node_Heartbeat_1_0_FIXED_PORT_ID_)
        {
            bKindMessageProcessed = true;
            size_t size = transfer->payload_size;
            uavcan_node_Heartbeat_1_0 msg = {0};
            if (uavcan_node_Heartbeat_1_0_deserialize_(&msg, static_cast<uint8_t const*>(transfer->payload), &size) >= 0)
            {
                Serial.print(F("RX HeartBeat from -> "));
                Serial.println(transfer->metadata.remote_node_id);
                // Processo e registro il nodo: stato, OnLine e relativi flag
                byte queueId = getQueueNodeFromId(clsCanard, transfer->metadata.remote_node_id);
                // Se nodo correttamente allocato e gestito (potrebbe essere Yakut non registrato)
                if (queueId != GENERIC_BVAL_UNDEFINED) {
                    // Primo assegnamento da PNP, gestisco eventuale configurazione remota
                    // e salvataggio del flag di assegnamento in ROM / o Register
                    if(!clsCanard.slave[queueId].pnp.is_assigned) {
                        // Configura i metadati...
                        // Configura altri parametri...
                        // Modifico il flag PNP Executed e termino la procedura PNP
                        clsCanard.slave[queueId].pnp.is_assigned = true;
                        // Salvo su registro lo stato
                        uavcan_register_Value_1_0 val = {0};
                        char registerName[24] = "rmap.pnp.allocateID.";
                        uavcan_register_Value_1_0_select_natural8_(&val);
                        val.natural32.value.count       = 1;
                        val.natural32.value.elements[0] = transfer->metadata.remote_node_id;
                        // queueId -> index Val = NodeId
                        itoa(queueId, registerName + strlen(registerName), 10);
                        registerWrite(registerName, &val);
                    }
                    
                    // Rientro in OnLINE da OFFLine o Init
                    // Inizializzo le variabili e gli stati necessari per Reset e corretta gestione
                    if(!clsCanard.slave[queueId].is_online) {
                        // Metto i Flag in sicurezza, laddove dove non eventualmente gestito
                        clsCanard.slave[queueId].command.is_pending = false;
                        clsCanard.slave[queueId].command.is_timeout = true;
                        clsCanard.slave[queueId].register_access.is_pending = false;
                        clsCanard.slave[queueId].register_access.is_timeout = true;
                        clsCanard.slave[queueId].file.is_pending = false;
                        clsCanard.slave[queueId].file.is_timeout = true;
                        clsCanard.slave[queueId].file.state = FILE_STATE_STANDBY;
                        clsCanard.slave[queueId].rmap_service.is_pending = false;
                        clsCanard.slave[queueId].rmap_service.is_timeout = true;
                    }
                    // Accodo i dati letti dal messaggio (Nodo -> OnLine)
                    clsCanard.slave[queueId].is_online = true;  
                    clsCanard.slave[queueId].heartbeat.healt = msg.health.value;
                    clsCanard.slave[queueId].heartbeat.state = msg.vendor_specific_status_code;
                    // Set canard_us local per controllo NodoOffline
                    clsCanard.slave[queueId].heartbeat.timeout_us = transfer->timestamp_usec + NODE_OFFLINE_TIMEOUT_US;
                }
            }
        }
        #ifdef USE_SUB_PUBLISH_SLAVE_DATA
        else
        {
            // Gestione messaggi pubblicazione dati dei moduli slave (gestisco come master)
            // Es. popalamento dati se attivato un log specifico o show valori su display
            // Il comando è opzionale perchè in request/response esiste già questa possibilità
            // Nodo rispondente leggo dalla coda la/le pubblicazioni attivate (MAX 1 x tipologia)
            byte queueId = getQueueNodeFromId(clsCanard, transfer->metadata.remote_node_id);
            // Se nodo correttammente allocato e gestito
            if (queueId != GENERIC_BVAL_UNDEFINED) {
                // Verifico se risposta del servizio corrisponde al chiamante (eventuali + servizi sotto...)
                // Gestione di tutti i servizi possibili allocabili, valido per tutti i nodi
                if (transfer->metadata.port_id == clsCanard.slave[queueId].publisher.subject_id)
                {                
                    // *************            Service Modulo TH Response            *************
                    if(clsCanard.slave[queueId].node_type == MODULE_TYPE_TH) {
                        // Processato il messaggio con il relativo Handler. OK
                        bKindMessageProcessed = true;
                        // Modulo TH, leggo e deserializzo il messaggio in ingresso
                        rmap_module_TH_1_0  msg = {0};
                        size_t             size = transfer->payload_size;
                        if (rmap_module_TH_1_0_deserialize_(&msg, static_cast<uint8_t const*>(transfer->payload), &size) >= 0)
                        {
                            // TODO: vedere con Marco Pubblica, registra elimina, display... altro
                            // Per ora salvo solo il dato ricevuto dalla struttura di state msg (count)
                            // msg contiene i dati di blocco pubblicati
                            clsCanard.slave[queueId].publisher.data_count++;
                        }
                    }
                    // ALTRI MODULI DA INSERIRE QUA... PG, VV, RS, GAS ECC...
                }
            }
        }
        #endif
        // Mancanza di handler per un servizio sottoscritto
        if (!bKindMessageProcessed)
        {
            // Gestione di un messaggio sottoscritto senza gestione. Se arrivo quà è un errore di sviluppo
            assert(false);
        }
    }
    // Gestione delle richieste esterne
    else if (transfer->metadata.transfer_kind == CanardTransferKindRequest)
    {
        if (transfer->metadata.port_id == uavcan_node_GetInfo_1_0_FIXED_PORT_ID_)
        {
            // The request object is empty so we don't bother deserializing it. Just send the response.
            const uavcan_node_GetInfo_Response_1_0 resp = processRequestNodeGetInfo();
            uint8_t      serialized[uavcan_node_GetInfo_Response_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
            size_t       serialized_size = sizeof(serialized);
            const int8_t res = uavcan_node_GetInfo_Response_1_0_serialize_(&resp, &serialized[0], &serialized_size);
            if (res >= 0) {
                clsCanard.sendResponse(MEGA, &transfer->metadata, serialized_size, &serialized[0]);
            }
        }
        else if (transfer->metadata.port_id == uavcan_register_Access_1_0_FIXED_PORT_ID_)
        {
            uavcan_register_Access_Request_1_0 req  = {0};
            size_t                             size = transfer->payload_size;
            if (uavcan_register_Access_Request_1_0_deserialize_(&req, static_cast<uint8_t const*>(transfer->payload), &size) >= 0)
            {
                const uavcan_register_Access_Response_1_0 resp = processRequestRegisterAccess(&req);
                uint8_t serialized[uavcan_register_Access_Response_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
                size_t  serialized_size = sizeof(serialized);
                if (uavcan_register_Access_Response_1_0_serialize_(&resp, &serialized[0], &serialized_size) >= 0) {
                    clsCanard.sendResponse(MEGA, &transfer->metadata, serialized_size, &serialized[0]);
                }
            }
        }
        else if (transfer->metadata.port_id == uavcan_register_List_1_0_FIXED_PORT_ID_)
        {
            uavcan_register_List_Request_1_0 req  = {0};
            size_t                           size = transfer->payload_size;
            if (uavcan_register_List_Request_1_0_deserialize_(&req, static_cast<uint8_t const*>(transfer->payload), &size) >= 0)
            {
                uavcan_register_List_Response_1_0 resp;
                resp.name = registerGetNameByIndex(req.index);
                uint8_t serialized[uavcan_register_List_Response_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
                size_t  serialized_size = sizeof(serialized);
                if (uavcan_register_List_Response_1_0_serialize_(&resp, &serialized[0], &serialized_size) >= 0) {
                    clsCanard.sendResponse(MEGA, &transfer->metadata, serialized_size, &serialized[0]);
                }
            }
        }
        else if (transfer->metadata.port_id == uavcan_node_ExecuteCommand_1_1_FIXED_PORT_ID_)
        {
            uavcan_node_ExecuteCommand_Request_1_1 req  = {0};
            size_t                                 size = transfer->payload_size;
            if (uavcan_node_ExecuteCommand_Request_1_1_deserialize_(&req, static_cast<uint8_t const*>(transfer->payload), &size) >= 0)
            {
                const uavcan_node_ExecuteCommand_Response_1_1 resp = processRequestExecuteCommand(clsCanard, &req, transfer->metadata.remote_node_id);
                uint8_t serialized[uavcan_node_ExecuteCommand_Response_1_1_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
                size_t  serialized_size = sizeof(serialized);
                if (uavcan_node_ExecuteCommand_Response_1_1_serialize_(&resp, &serialized[0], &serialized_size) >= 0) {
                    clsCanard.sendResponse(MEGA, &transfer->metadata, serialized_size, &serialized[0]);
                }
            }
        }
        else if (transfer->metadata.port_id == uavcan_file_Read_1_1_FIXED_PORT_ID_)
        {
            // La funzione viene eseguita solo con UPLOAD Sequenza corretta
            // Serve a fare eseguire un'eventuale TimeOut su procedura non corretta
            // Ed evitare blocchi non coerenti... Viene gestita senza problemi
            // Send multiplo di File e procedure in sequenza ( Gestito TimeOut di Request )
            byte queueId = getQueueNodeFromId(clsCanard, transfer->metadata.remote_node_id);
            // Devo essere in aghgiornamento per sicurezza!!!
            if(clsCanard.slave[queueId].file.state == FILE_STATE_UPLOADING) {
                // Update TimeOut (Comunico request OK al Master, Slave sta scaricando)
                // Se Slave si blocca per TimeOut, esco dalla procedura dove gestita
                // La gestione dei time OUT è in unica funzione x Tutti i TimeOUT
                clsCanard.slave[queueId].file.timeout_us = transfer->timestamp_usec + NODE_REQFILE_TIMEOUT_US;
                uavcan_file_Read_Request_1_1 req  = {0};
                size_t                       size = transfer->payload_size;
                if (uavcan_file_Read_Request_1_1_deserialize_(&req, static_cast<uint8_t const*>(transfer->payload), &size) >= 0)
                {
                    uavcan_file_Read_Response_1_1 resp = {0};
                    byte dataBuf[uavcan_primitive_Unstructured_1_0_value_ARRAY_CAPACITY_];
                    // Terminatore di sicurezza
                    req.path.path.elements[req.path.path.count] = 0;
                    // Allego il blocco dati se presente
                    size_t dataLen = uavcan_primitive_Unstructured_1_0_value_ARRAY_CAPACITY_;
                    // Il Master ha finito la trasmissione, esco dalla procedura
                    if (getDataFile((char*)&req.path.path.elements[0], clsCanard.slave[queueId].file.is_firmware,
                                    req.offset, dataBuf, &dataLen)) {
                        resp._error.value = uavcan_file_Error_1_0_OK;
                        if(dataLen != uavcan_primitive_Unstructured_1_0_value_ARRAY_CAPACITY_) {
                            clsCanard.slave[queueId].file.state = FILE_STATE_UPLOAD_COMPLETE;
                        }
                    } else {
                        // Ritorno un errore da interpretare a Slave
                        resp._error.value = uavcan_file_Error_1_0_IO_ERROR;
                        dataLen = 0;
                    }
                    // Preparo la risposta corretta
                    resp.data.value.count = dataLen;
                    memcpy(resp.data.value.elements, dataBuf, dataLen);
                    uint8_t serialized[uavcan_file_Read_Response_1_1_SERIALIZATION_BUFFER_SIZE_BYTES_] = {0};
                    size_t  serialized_size = sizeof(serialized);
                    if (uavcan_file_Read_Response_1_1_serialize_(&resp, &serialized[0], &serialized_size) >= 0) {
                        clsCanard.sendResponse(MEGA, &transfer->metadata, serialized_size, &serialized[0]);
                    }
                }
            }
        }
        else
        {
            // Gestione di una richiesta senza controllore locale. Se arrivo quà è un errore di sviluppo
            assert(false);
        }
    }
    // Gestione delle risposte alle richeste inviate alla rete come Master
    else if (transfer->metadata.transfer_kind == CanardTransferKindResponse)
    {
        // Comando inviato ad un nodo remoto, verifica della risposta e della coerenza messaggio
        if (transfer->metadata.port_id == uavcan_node_ExecuteCommand_1_1_FIXED_PORT_ID_)
        {
            uavcan_node_ExecuteCommand_Response_1_1 resp = {0};
            size_t                                  size = transfer->payload_size;
            if (uavcan_node_ExecuteCommand_Response_1_1_deserialize_(&resp, static_cast<uint8_t const*>(transfer->payload), &size) >= 0)
            {
                // Ricerco idNodo nella coda degli allocati del master
                // Copio la risposta ricevuta nella struttura relativa e resetto il flag pending                
                byte queueId = getQueueNodeFromId(clsCanard, transfer->metadata.remote_node_id);
                if (queueId != GENERIC_BVAL_UNDEFINED) {
                    // Resetta il pending del comando del nodo verificato
                    clsCanard.slave[queueId].command.is_pending = false;
                    // Copia la risposta nella variabile di chiamata in state
                    clsCanard.slave[queueId].command.response = resp.status;
                }
            }
        }
        else if (transfer->metadata.port_id == uavcan_register_Access_1_0_FIXED_PORT_ID_)
        {
            uavcan_register_Access_Response_1_0 resp = {0};
            size_t                              size = transfer->payload_size;
            if (uavcan_register_Access_Response_1_0_deserialize_(&resp, static_cast<uint8_t const*>(transfer->payload), &size) >= 0)
            {
                // Ricerco idNodo nella coda degli allocati del master
                // Copio la risposta ricevuta nella struttura relativa e resetto il flag pending                
                byte queueId = getQueueNodeFromId(clsCanard, transfer->metadata.remote_node_id);
                if (queueId != GENERIC_BVAL_UNDEFINED) {
                    // Resetta il pending del comando del nodo verificato
                    clsCanard.slave[queueId].register_access.is_pending = false;
                    // Copia la risposta nella variabile di chiamata in state
                    clsCanard.slave[queueId].register_access.response = resp.value;
                }
            }
        }
        else if (transfer->metadata.port_id == uavcan_file_Read_1_1_FIXED_PORT_ID_)
        {
            // Accetto solo messaggi indirizzati dal node_id che ha fatto la richiesta di upload
            // E' una sicurezza per il controllo dell'upload, ed evità errori di interprete
            // Inoltre non accetta messaggi extra standard UAVCAN, necessarià prima la CALL al comando
            // SEtFirmwareUpload o SetFileUpload, che impostano il node_id, resettato su EOF dalla classe
            if (clsCanard.master.file.get_server_node() == transfer->metadata.remote_node_id) {
                uavcan_file_Read_Response_1_1 resp  = {0};
                size_t                         size = transfer->payload_size;
                if (uavcan_file_Read_Response_1_1_deserialize_(&resp, static_cast<uint8_t const*>(transfer->payload), &size) >= 0) {
                    if(clsCanard.master.file.is_firmware()) {
                        Serial.print(F("RX FIRMWARE READ BLOCK LEN: "));
                    } else {
                        Serial.print(F("RX FILE READ BLOCK LEN: "));
                    }
                    Serial.println(resp.data.value.count);
                    // Save Data in File at Block Position (Init = Rewrite file...)
                    putDataFile(clsCanard.master.file.get_name(), clsCanard.master.file.is_firmware(), clsCanard.master.file.is_first_data_block(),
                                resp.data.value.elements, resp.data.value.count);
                    // Reset pending command (Comunico request/Response Serie di comandi OK!!!)
                    // Uso l'Overload con controllo di EOF (-> EOF se msgLen != UAVCAN_BLOCK_DEFAULT [256 Bytes])
                    // Questo Overload gestisce in automatico l'offset del messaggio, per i successivi blocchi
                    clsCanard.master.file.reset_pending(resp.data.value.count);
                }
            } else {
                // Errore Nodo non settato...
                Serial.println(F("RX FILE READ BLOCK REJECT: Node_Id not valid or not set"));
            }
        }
        // Risposta ad un servizio (dati) dinamicamente allocato... ( deve essere ultimo else )
        // Servizio di risposta alla richiesta su modulo slave, verifica della risposta e della coerenza messaggio
        // Per il nodo che risponde verifico i servizi attivi per la corrispondenza dinamica risposta
        else
        {
            // Nodo rispondente (posso avere senza problemi più servizi con stesso port_id su diversi nodi)
            byte queueId = getQueueNodeFromId(clsCanard, transfer->metadata.remote_node_id);
            // Se nodo correttammente allocato e gestito
            if (queueId != GENERIC_BVAL_UNDEFINED) {
                // Verifico se risposta del servizio corrisponde al chiamante (eventuali + servizi sotto...)
                // Gestione di tutti i servizi possibili allocabili, valido per tutti i nodi
                if (transfer->metadata.port_id == clsCanard.slave[queueId].rmap_service.port_id)
                {                
                    // *************            Service Modulo TH Response            *************
                    if(clsCanard.slave[queueId].node_type == MODULE_TYPE_TH) {
                        // Modulo TH, leggo e deserializzo il messaggio in ingresso
                        rmap_service_module_TH_Response_1_0 resp = {0};
                        size_t                              size = transfer->payload_size;
                        if (rmap_service_module_TH_Response_1_0_deserialize_(&resp, static_cast<uint8_t const*>(transfer->payload), &size) >= 0)
                        {
                            // Resetta il pending del comando del nodo verificato
                            clsCanard.slave[queueId].rmap_service.is_pending = false;
                            // Copia la risposta nella variabile di chiamata in state
                            // Oppure gestire qua tutte le altre occorrenze per stima V4
                            // TODO: vedere con Marco Pubblica, registra elimina, display... altro
                            // Per ora copio in una struttura di state response
                            memcpy(clsCanard.slave[queueId].rmap_service.module, &resp, sizeof(resp));
                        }
                    }
                    // ALTRI MODULI DA INSERIRE QUA... PG, VV, RS, GAS ECC...
                }
            }
        }
    }
    else
    {
        // Se arrivo quà è un errore di sviluppo, controllare setup sottoscrizioni, risposte e messaggi
        assert(false);
    }
}

// *********************************** SETUP CAN BUS IFACE **********************************
bool CAN_HW_Init(void)
{
    // Definition CAN structure variable
    CAN_HandleTypeDef CAN_Handle;

    // Definition GPIO and CAN filter structure variables
    GPIO_InitTypeDef GPIO_InitStruct;
    CAN_FilterTypeDef CAN_FilterInitStruct;

    // GPIO Ports clock enable
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // CAN1 clock enable
    __HAL_RCC_CAN1_CLK_ENABLE();

    #if defined(STM32L496xx)
    // Mapping GPIO for CAN
    /* Configure CAN pin: RX */
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    /* Configure CAN pin: TX */
    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    #else
    #error "Warning untested processor variant"
    #endif

    // Setup CAN Istance Basic
    CAN_Handle.Instance = CAN1;
    CAN_Handle.Init.Mode = CAN_MODE_NORMAL;
    CAN_Handle.Init.TimeTriggeredMode = DISABLE;
    CAN_Handle.Init.AutoBusOff = DISABLE;
    CAN_Handle.Init.AutoWakeUp = DISABLE;
    CAN_Handle.Init.AutoRetransmission = DISABLE;
    CAN_Handle.Init.ReceiveFifoLocked = DISABLE;
    CAN_Handle.Init.TransmitFifoPriority = DISABLE;
    // Check error initialization CAN
    if (HAL_CAN_Init(&CAN_Handle) != HAL_OK) {
        Serial.println(F("Error initialization HW CAN base"));
        assert(false);
        return false;
    }

    // CAN filter basic initialization
    CAN_FilterInitStruct.FilterIdHigh = 0x0000;
    CAN_FilterInitStruct.FilterIdLow = 0x0000;
    CAN_FilterInitStruct.FilterMaskIdHigh = 0x0000;
    CAN_FilterInitStruct.FilterMaskIdLow = 0x0000;
    CAN_FilterInitStruct.FilterFIFOAssignment = CAN_RX_FIFO0;
    CAN_FilterInitStruct.FilterBank = 0;
    CAN_FilterInitStruct.FilterMode = CAN_FILTERMODE_IDMASK;
    CAN_FilterInitStruct.FilterScale = CAN_FILTERSCALE_32BIT;
    CAN_FilterInitStruct.FilterActivation = ENABLE;

    // Check error initalization CAN filter
    if (HAL_CAN_ConfigFilter(&CAN_Handle, &CAN_FilterInitStruct) != HAL_OK) {
        Serial.println(F("Error initialization filter CAN base"));
        assert(false);
        return false;
    }

    // *******************         CANARD SETUP TIMINGS AND SPEED        *******************
    // CAN BITRATE Dinamico su LoadRegister (CAN_FD 2xREG natural32 0=Speed, 1=0 (Not Used))
    uavcan_register_Value_1_0 val = {0};
    uavcan_register_Value_1_0_select_natural32_(&val);
    val.natural32.value.count       = 2;
    val.natural32.value.elements[0] = CAN_BIT_RATE;
    val.natural32.value.elements[1] = 0ul;          // Ignored for CANARD_MTU_CAN_CLASSIC
    registerRead("uavcan.can.bitrate", &val);
    assert(uavcan_register_Value_1_0_is_natural32_(&val) && (val.natural32.value.count == 2));

    // Dynamic BIT RATE Change CAN Speed to CAN_BIT_RATE (register default/defined)
    BxCANTimings timings;
    bool result = bxCANComputeTimings(HAL_RCC_GetPCLK1Freq(), val.natural32.value.elements[0], &timings);
    if (!result) {
        Serial.println(F("Error redefinition bxCANComputeTimings, try loading default..."));
        val.natural32.value.count       = 2;
        val.natural32.value.elements[0] = CAN_BIT_RATE;
        val.natural32.value.elements[1] = 0ul;          // Ignored for CANARD_MTU_CAN_CLASSIC
        registerWrite("uavcan.can.bitrate", &val);
        result = bxCANComputeTimings(HAL_RCC_GetPCLK1Freq(), val.natural32.value.elements[0], &timings);
        if (!result) {
            Serial.println(F("Error initialization bxCANComputeTimings"));
            assert(false);
            return false;
        }
    }
    // Attivazione bxCAN sulle interfacce richieste, velocità e modalità
    result = bxCANConfigure(0, timings, false);
    if (!result) {
        Serial.println(F("Error initialization bxCANConfigure"));
        assert(false);
        return false;
    }
    // *******************     CANARD SETUP TIMINGS AND SPEED COMPLETE   *******************

    // Check error starting CAN
    if (HAL_CAN_Start(&CAN_Handle) != HAL_OK) {
        Serial.println(F("CAN startup ERROR!!!"));
        assert(false);
        return false;
    }

    // Enable Interrupt RX Standard CallBack -> CAN1_RX0_IRQHandler
    if (HAL_CAN_ActivateNotification(&CAN_Handle, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
        Serial.println(F("Error initialization interrupt CAN base"));
        assert(false);
        return false;
    }
    // Setup Priority e CB CAN_IRQ_RX Enable
    HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);

    // Setup Completato
    return true;
}

// *********************************************************************************************
//                                       SETUP AMBIENTE
// *********************************************************************************************
void setup(void) {

    // *****************************************************
    //            STARTUP SERIAL-COMMUNICATION 
    // *****************************************************
    Serial.begin(115200);
    // Wait for serial port to connect
    while (!Serial) {
    }
    Serial.println(F("Start RS232 Monitor"));

    // *****************************************************
    //            STARTUP LED E PIN DIAGNOSTICI
    // *****************************************************
    // Output mode for LED BLINK SW LOOP (High per Setup)
    // Input mode for test button
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(USER_BTN, INPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    
    // *****************************************************
    //      STARTUP LIBRERIA SD/MEM REGISTER COLLEGATA
    // *****************************************************
    if (!setupSd(PIN_SPI_MOSI, PIN_SPI_MISO, PIN_SPI_SCK, PIN_SPI_SS, 18)) {
        Serial.println(F("Initialization SD card error"));
        assert(false);
    }
    Serial.println(F("Initialization SD card done"));

    // ********************************************************************************
    //    FIXED REGISTER_INIT, FARE INIT OPZIONALE x REGISTRI FISSI ECC. E/O INVAR.
    // ********************************************************************************
    #ifdef INIT_REGISTER
    // Inizializzazione fissa dei registri nella modalità utilizzata (prepare SD/FLASH/ROM con default Value)
    registerSetup(true);
    #else
    // Default dei registri nella modalità utilizzata (prepare SD/FLASH/ROM con default Value)
    // Creazione dei registri standard base se non esistono
    registerSetup(false);
    #endif

    // *****************************************************
    //            STARTUP CANBUS E CANARD SPEED
    // *****************************************************
    Serial.print(F("Initializing CANBUS..., PCLK1 Clock Freq: "));
    Serial.println(HAL_RCC_GetPCLK1Freq());
    if (!CAN_HW_Init()) {
        Serial.println(F("Initialization CAN BUS error"));
        assert(false);
    }
    Serial.println(F("Initialization CAN BUS done"));
    
    // Led Low Init Setup OK
    digitalWrite(LED_BUILTIN, LOW);
}

// *************************************************************************************************
//                                          MAIN LOOP
// *************************************************************************************************
void loop(void)
{
    uavcan_register_Value_1_0 val = {0};
    // Avvia l'istanza alla classe State_Canard ed inizializza Ram, Buffer e variabili base
    canardClass clsCanard;

    // Avvio inizializzazione (Standard UAVCAN MSG). Reset su INIT END OK
    // Segnale al Master necessità di impostazioni ev. parametri, Data/Ora ecc..
    clsCanard.flag.set_local_node_mode(uavcan_node_Mode_1_0_INITIALIZATION);

    // Attiva il callBack su RX Messaggio Canard sulla funzione interna processReceivedTransfer
    clsCanard.setReceiveMessage_CB(processReceivedTransfer);

    // ********************************************************************************
    //            INIT VALUE, Caricamento default e registri locali MASTER
    // ********************************************************************************
    // Reset Slave node_id per ogni nodo collegato. Solo i nodi validi verranno gestiti
    for(uint8_t iCnt = 0; iCnt<MAX_NODE_CONNECT; iCnt++) {
        clsCanard.slave[iCnt].node_id = CANARD_NODE_ID_UNSET;
        clsCanard.slave[iCnt].rmap_service.port_id = UINT16_MAX;
        clsCanard.slave[iCnt].rmap_service.module = NULL;
        #ifdef USE_SUB_PUBLISH_SLAVE_DATA
        clsCanard.slave[iCnt].publisher.subject_id = UINT16_MAX;
        #endif
        clsCanard.slave[iCnt].file.state = FILE_STATE_STANDBY;
    }

    // Canard Master NODE ID Fixed dal defined value in module_config
    clsCanard.canard.node_id = (CanardNodeID) NODE_MASTER_ID;

    // ********************************************************************************
    //                   READING PARAM FROM E2 MEMORY / FLASH / SDCARD
    // ********************************************************************************

    // TODO:
    // Read Config Slave Node x Lettura porte e servizi.
    // Possibilità di utilizzo come sotto (registri) - Fixed Value adesso !!!
    clsCanard.slave[0].node_id = 125;
    clsCanard.slave[0].node_type = MODULE_TYPE_TH;    
    clsCanard.slave[0].rmap_service.port_id = 100;
    #ifdef USE_SUB_PUBLISH_SLAVE_DATA
    // clsCanard.slave[0].subject_id = 5678;
    #endif
    clsCanard.slave[0].rmap_service.module = malloc(sizeof(rmap_service_module_TH_Response_1_0));
    strcpy(clsCanard.slave[0].file.filename, "stima4.module_th-1.1.app.hex");

    // **********************************************************************************
    // Lettura registri, parametri per PNP Allocation Verifica locale di assegnamento CFG
    // **********************************************************************************
    for(uint8_t iCnt = 0; iCnt<MAX_NODE_CONNECT; iCnt++) {
        // Lettura registro di allocazione PNP MASTER Locale avvenuta, da eseguire
        // Possibilità di salvare tutte le informazioni di NODO qui al suo interno
        // Per rendere disponibili le configurazioni in esterno (Yakut, altri)
        // Utilizzando la struttupra allocateID.XX (count = n° registri utili)
        char registerName[24] = "rmap.pnp.allocateID.";
        uavcan_register_Value_1_0_select_natural8_(&val);
        val.natural8.value.count       = 1;
        val.natural8.value.elements[0] = CANARD_NODE_ID_UNSET;
        // queueId -> index Val = NodeId
        itoa(iCnt, registerName + strlen(registerName), 10);
        registerRead(registerName, &val);
        assert(uavcan_register_Value_1_0_is_natural8_(&val) && (val.natural8.value.count == 1));
        // Il Node_id deve essere valido e uguale a quello programmato in configurazione
        if((val.natural8.value.elements[0] != CANARD_NODE_ID_UNSET) &&
            (val.natural8.value.elements[0] == clsCanard.slave[iCnt].node_id))
         {
            // Assegnamento PNP per nodeQueueID con clsCanard.slave[iCnt].node_id
            // già avvenuto. Non rispondo a eventuali messaggi PNP del tipo per quel nodo
            clsCanard.slave[iCnt].pnp.is_assigned = true;
        }
    }

    // ********************************************************************************
    // *********               Lettura Registri standard UAVCAN               *********
    // ********************************************************************************

    // The description register is optional but recommended because it helps constructing/maintaining large networks.
    // It simply keeps a human-readable description of the node that should be empty by default.
    uavcan_register_Value_1_0_select_string_(&val);
    val._string.value.count = 0;
    registerRead("uavcan.node.description", &val);  // We don't need the value, we just need to ensure it exists.

    // ********************************************************************************
    //               AVVIA SOTTOSCRIZIONI ai messaggi per servizi RPC ecc...
    // ********************************************************************************
    
    // Service servers: -> Risposta per GetNodeInfo richiesta esterna (Yakut, Altri)
    if (!clsCanard.rxSubscribe(CanardTransferKindRequest,
                            uavcan_node_GetInfo_1_0_FIXED_PORT_ID_,
                            uavcan_node_GetInfo_Request_1_0_EXTENT_BYTES_,
                            CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
        LOCAL_ASSERT(false);
    }

    // Service servers: -> Chiamata per ExecuteCommand richiesta esterna (Yakut, Altri)
    if (!clsCanard.rxSubscribe(CanardTransferKindRequest,
                            uavcan_node_ExecuteCommand_1_1_FIXED_PORT_ID_,
                            uavcan_node_ExecuteCommand_Request_1_1_EXTENT_BYTES_,
                            CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
        LOCAL_ASSERT(false);
    }

    // Service servers: -> Risposta per Accesso ai registri richiesta esterna (Yakut, Altri)
    if (!clsCanard.rxSubscribe(CanardTransferKindRequest,
                            uavcan_register_Access_1_0_FIXED_PORT_ID_,
                            uavcan_register_Access_Request_1_0_EXTENT_BYTES_,
                            CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
        LOCAL_ASSERT(false);
    }

    // Service servers: -> Risposta per Lista dei registri richiesta esterna (Yakut, Altri)
    if (!clsCanard.rxSubscribe(CanardTransferKindRequest,
                            uavcan_register_List_1_0_FIXED_PORT_ID_,
                            uavcan_register_List_Request_1_0_EXTENT_BYTES_,
                            CANARD_REGISTERLIST_TRANSFER_ID_TIMEOUT_USEC)) {
        LOCAL_ASSERT(false);
    }

    // ******* SOTTOSCRIZIONE MESSAGGI / COMANDI E SERVIZI AD UTILITA' MASTER ********

    // Messaggi PNP_Allocation: -> Allocazione dei nodi standard in PlugAndPlay per i nodi conosciuti
    if (!clsCanard.rxSubscribe(CanardTransferKindMessage,
                            uavcan_pnp_NodeIDAllocationData_1_0_FIXED_PORT_ID_,
                            uavcan_pnp_NodeIDAllocationData_1_0_EXTENT_BYTES_,
                            CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
        LOCAL_ASSERT(false);
    }

    // Messaggi HEARTBEAT: -> Verifica della presenza per stato Nodi (Slave) OnLine / OffLine
    if (!clsCanard.rxSubscribe(CanardTransferKindMessage,
                            uavcan_node_Heartbeat_1_0_FIXED_PORT_ID_,
                            uavcan_node_Heartbeat_1_0_EXTENT_BYTES_,
                            CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
        LOCAL_ASSERT(false);
    }

    // Service client: -> Risposta per ExecuteCommand richiesta interna (come master)
    if (!clsCanard.rxSubscribe(CanardTransferKindResponse,
                            uavcan_node_ExecuteCommand_1_1_FIXED_PORT_ID_,
                            uavcan_node_ExecuteCommand_Response_1_1_EXTENT_BYTES_,
                            CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
        LOCAL_ASSERT(false);
    }

    // Service client: -> Risposta per Accesso ai registri richiesta interna (come master)
    if (!clsCanard.rxSubscribe(CanardTransferKindResponse,
                            uavcan_register_Access_1_0_FIXED_PORT_ID_,
                            uavcan_register_Access_Response_1_0_EXTENT_BYTES_,
                            CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
        LOCAL_ASSERT(false);
    }

    // Service client: -> Risposta per Read (Receive) File local richiesta esterna (Yakut, Altri)
    if (!clsCanard.rxSubscribe(CanardTransferKindResponse,
                            uavcan_file_Read_1_1_FIXED_PORT_ID_,
                            uavcan_file_Read_Response_1_1_EXTENT_BYTES_,
                            CANARD_READFILE_TRANSFER_ID_TIMEOUT_USEC)) {
        LOCAL_ASSERT(false);
    }

    // Service server: -> Risposta per Read (Request Slave) File read archivio (come master)
    if (!clsCanard.rxSubscribe(CanardTransferKindRequest,
                            uavcan_file_Read_1_1_FIXED_PORT_ID_,
                            uavcan_file_Read_Request_1_1_EXTENT_BYTES_,
                            CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
        LOCAL_ASSERT(false);
    }

    // Allocazione dinamica delle subscription servizi request/response e pubblicazioni dei nodi remoti
    // in funzione delle registrazioni/istanze utilizzate nel master
    // Ogni subject utuilizzato per ogni nodo slave deve avere una sottoscrizione propria
    // Solo una sottoscrizione è possibile per singolo servizio per relativo port_id (dynamic o fixed)
    // La gestione di servizi con stesso port_id è assolutamente possibile e correttamente gestita
    // nel software. Basta solamente a livello server registrare lo stesso port_id per servizio.
    // La funzionailtà cosi impostata consente uno o più port_id x servizio senza problemi
    // Eventuali errori di allocazione possono eventualmente essere rilevati ma non ci sono problemi
    // sw in quanto una sottoscrizione chiamata in coda elimina una precedente (con stesso port o subjcect)
    for(byte queueId=0; queueId<MAX_NODE_CONNECT; queueId++) {
        // *************   SERVICE    *************
        // Se previsto il servizio request/response con port_id valido
        if ((clsCanard.slave[queueId].rmap_service.module) &&
            (clsCanard.slave[queueId].rmap_service.port_id <= CANARD_SERVICE_ID_MAX)) {
            // Controllo le varie tipologie di request/service per il nodo
            if(clsCanard.slave[queueId].node_type == MODULE_TYPE_TH) {     
                // Alloco la stottoscrizione in funzione del tipo di modulo
                // Service client: -> Risposta per ServiceDataModuleTH richiesta interna (come master)
                if (!clsCanard.rxSubscribe(CanardTransferKindResponse,
                                        clsCanard.slave[queueId].rmap_service.port_id,
                                        rmap_service_module_TH_Response_1_0_EXTENT_BYTES_,
                                        CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                    LOCAL_ASSERT(false);
                }
            }
        }
        #ifdef USE_SUB_PUBLISH_SLAVE_DATA
        // *************   PUBLISH    *************
        // Se previsto il servizio publisher (subject_id valido)
        // Non alloco niente per il publish (gestione esempio display o altro debug interno da gestire)
         if (clsCanard.slave[queueId].publisher.subject_id <= CANARD_SUBJECT_ID_MAX) {
            // Controllo le varie tipologie di request/service per il nodo
            if(clsCanard.slave[queueId].node_type == MODULE_TYPE_TH) {            
                // Alloco la stottoscrizione in funzione del tipo di modulo
                // Service client: -> Sottoscrizione per ModuleTH (come master)
                if (!clsCanard.rxSubscribe(CanardTransferKindMessage,
                                        clsCanard.slave[queueId].publisher.subject_id,
                                        rmap_module_TH_1_0_EXTENT_BYTES_,
                                        CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC)) {
                    LOCAL_ASSERT(false);
                }
            }
        }
        #endif
    }

    // ********************************************************************************
    //         AVVIA LOOP CANARD PRINCIPALE gestione TX/RX Code -> Messaggi
    // ********************************************************************************

    // TODO: Eliminare
    bool ledShow;
    int  bTxAttempt = 0;
    int  bRxAttempt = 0;
    long checkTimeout = 0;
    long lastMillis;
    bool bEnabService = false;
    bool bLastPendingCmd = false;
    bool bIsPendingCmd = false;
    bool bLastPendingData = false;
    bool bIsPendingData = false;
    bool bIsResetFaultCmd = false;
    bool bEventRealTimeLoop = false;
    bool bFileUpload = false;
    bool bFileMessage = false;
    bool bLastPendingRegister = false;
    bool bIsPendingRegister = false;
    bool bIsWriteRegister = false;

    #define MILLIS_EVENT 10
    #define PUBLISH_HEARTBEAT
    #define PUBLISH_PORTLIST
    #define LOG_RX_PACKET
    #define LED_ON_CAN_DATA_TX
    #define LED_ON_CAN_DATA_RX
    // #define LED_ON_SYNCRO_TIME
    // #define TEST_COMMAND
    // #define TEST_RMAP_DATA
    // #define TEST_REGISTER

    // Set START Timetable LOOP RX/TX. Set Canard microsecond start, per le sincronizzazioni
    clsCanard.getMonotonicMicroseconds(clsCanard.start_syncronization);
    CanardMicrosecond       next_01_sec_iter_at = clsCanard.getMonotonicMicroseconds(clsCanard.syncronized_time) + MEGA * 0.25;
    CanardMicrosecond       next_timesyncro_msg = clsCanard.getMonotonicMicroseconds(clsCanard.syncronized_time) + MEGA;
    CanardMicrosecond       next_20_sec_iter_at = clsCanard.getMonotonicMicroseconds(clsCanard.syncronized_time) + MEGA * 1.5;
    CanardMicrosecond       test_cmd_cs_iter_at = clsCanard.getMonotonicMicroseconds(clsCanard.syncronized_time) + MEGA * 2.5;
    CanardMicrosecond       test_cmd_vs_iter_at = clsCanard.getMonotonicMicroseconds(clsCanard.syncronized_time) + MEGA * 3.5;
    CanardMicrosecond       test_cmd_rg_iter_at = clsCanard.getMonotonicMicroseconds(clsCanard.syncronized_time) + MEGA * 4.5;

    // Avvio il modulo UAVCAN in modalità operazionale normale
    // Eventuale SET Flag dopo acquisizione di configurazioni e/o parametri da Remoto
    clsCanard.flag.set_local_node_mode(uavcan_node_Mode_1_0_OPERATIONAL);

    do {
        // Set Canard microsecond corrente monotonic, per le attività temporanee di ciclo
        clsCanard.getMonotonicMicroseconds(clsCanard.start_syncronization);

        // Check TimeLine (quasi RealTime...)
        if ((millis()-checkTimeout) >= MILLIS_EVENT)
        {
            // Deadline di controllo per eventi di controllo Rapidi (TimeOut, FileHandler ecc...)
            // Mancata risposta, nodo in Errore o Full o CanardHeapError ecc...
            checkTimeout = millis();
            // Utilizzo per eventi quasi continuativi... Es. Send/Receive File queue...
            bEventRealTimeLoop = true;
        }

        // TEST VERIFICA sincronizzazione time_stamp locale con remoto... (LED sincronizzati)
        #ifdef LED_ON_SYNCRO_TIME
        // Verifico LED al secondo... su timeSTamp sincronizzato remoto
        if((monotonic_time / 1000000) % 2) {
            digitalWrite(LED_BUILTIN, HIGH);
        } else {
            digitalWrite(LED_BUILTIN, LOW);
        }
        #endif

        // ************************************************************************
        // ***************   CHECK OFFLINE/DEADLINE COMMAND/STATE   ***************
        // ************************************************************************
        // TEST Check ogni RTL circa ( SOLO TEST COMANDI DA INSERIRE IN TASK_TIME )
        // Deadline di controllo (checkTimeOut variabile sopra...)
        if (bEventRealTimeLoop)
        {
            // **********************************************************
            // Per la coda/istanze allocate valide SLAVE remote_node_flag
            // **********************************************************
            for (byte queueId = 0; queueId<MAX_NODE_CONNECT; queueId++) {
                if (clsCanard.slave[queueId].node_id <= CANARD_NODE_ID_MAX) {
                    // Solo se nodo OnLine (Automatic OnLine su HeartBeat RX)
                    if(clsCanard.slave[queueId].is_online) {
                        // Controllo TimeOUT Comando diretto su modulo remoto
                        // Mancata risposta, nodo in Errore o Full o CanardHeapError ecc...
                        if(clsCanard.slave[queueId].command.is_pending) {
                            if(clsCanard.getMonotonicMicroseconds(clsCanard.syncronized_time) > clsCanard.slave[queueId].command.timeout_us) {
                                // Setto il flag di TimeOUT che il master dovrà gestire (segnalazione BUG al Server?)
                                clsCanard.slave[queueId].command.is_timeout = true;
                            }
                        }
                        // Controllo TimeOut Comando getData su modulo remoto
                        // Mancata risposta, nodo in Errore o Full o CanardHeapError ecc...
                        if(clsCanard.slave[queueId].rmap_service.is_pending) {
                            if(clsCanard.getMonotonicMicroseconds(clsCanard.syncronized_time) > clsCanard.slave[queueId].rmap_service.timeout_us) {
                                // Setto il flag di TimeOUT che il master dovrà gestire (segnalazione BUG al Server?)
                                clsCanard.slave[queueId].rmap_service.is_timeout = true;
                            }
                        }
                        // Controllo TimeOut Comando file su modulo remoto
                        // Mancata risposta, nodo in Errore o Full o CanardHeapError ecc...
                        if(clsCanard.slave[queueId].file.is_pending) {
                            if(clsCanard.getMonotonicMicroseconds(clsCanard.syncronized_time) > clsCanard.slave[queueId].file.timeout_us) {
                                // Setto il flag di TimeOUT che il master dovrà gestire (segnalazione BUG al Server?)
                                clsCanard.slave[queueId].file.is_timeout = true;
                            }
                        }
                        // Check eventuale Nodo OFFLINE (Ultimo comando sempre perchè posso)
                        // Effettuare eventuali operazioni di SET,RESET Cmd in sicurezza
                        if (clsCanard.getMonotonicMicroseconds(clsCanard.syncronized_time) > clsCanard.slave[queueId].heartbeat.timeout_us) {
                            // Entro in OffLine
                            clsCanard.slave[queueId].is_online = false;
                            Serial.print(F("Nodo OFFLINE !!! Alert -> : "));
                            Serial.println(clsCanard.slave[queueId].node_id);
                            // Metto i Flag in sicurezza, laddove dove non eventualmente gestito
                            // Eventuali altre operazioni quà su Reset Comandi
                            clsCanard.slave[queueId].command.is_pending = false;
                            clsCanard.slave[queueId].command.is_timeout = true;
                            clsCanard.slave[queueId].register_access.is_pending = false;
                            clsCanard.slave[queueId].register_access.is_timeout = true;
                            clsCanard.slave[queueId].file.is_pending = false;
                            clsCanard.slave[queueId].file.is_timeout = true;
                            clsCanard.slave[queueId].file.state = FILE_STATE_STANDBY;
                            clsCanard.slave[queueId].rmap_service.is_pending = false;
                            clsCanard.slave[queueId].rmap_service.is_timeout = true;
                            // Attività x Reset e/o riavvio Nodo dopo OnLine -> Offline
                            // Solo TEST Locale TODO: da eliminare
                            bIsResetFaultCmd = true;
                        }
                    }
                }
            }
        }

        // **************************************************************************
        //                                CANARD UAVCAN
        // Scheduler temporizzato dei messaggi / comandi da inviare alla rete UAVCAN 
        // **************************************************************************

        // FILE HANDLER ( con controllo continuativo se avviato bEventRealTimeLoop )
        // Procedura di gestione ricezione file
        if(bEventRealTimeLoop)
        {
            // Verifica file download in corso (entro se in download)
            // Attivato da ricezione comando appropriato rxFile o rxFirmware
            if(clsCanard.master.file.download_request()) {
                if(clsCanard.master.file.is_firmware())
                    // Set Flag locale per comunicazione HeartBeat... uploading OK in corso
                    // Utilizzo locale per blocco procedure, sleep ecc. x Uploading
                    clsCanard.flag.set_local_fw_uploading(true);
                // Controllo eventuale timeOut del comando o RxBlocco e gestisco le Retry
                // Verifica TimeOUT Occurs per File download
                if(clsCanard.master.file.event_timeout()) {
                    Serial.println(F("Time OUT File... event occurs"));
                    // Gestione Retry previste dal comando per poi abbandonare
                    uint8_t retry; // In overload x LOGGING
                    if(clsCanard.master.file.next_retry(&retry)) {
                        Serial.print(F("Next Retry File read: "));
                        Serial.println(retry);
                    } else {
                        Serial.println(F("MAX Retry File occurs, download file ABORT !!!"));
                        clsCanard.master.file.download_end();
                    }
                }
                // Se messaggio in pending non faccio niente è attendo la fine del comando in run
                // In caso di errore subentrerà il TimeOut e verrà essere gestita la retry
                if(!clsCanard.master.file.is_pending()) {
                    // Fine pending, con messaggio OK. Verifico se EOF o necessario altro blocco
                    if(clsCanard.master.file.is_download_complete()) {
                        if(clsCanard.master.file.is_firmware()) {
                            Serial.println(F("RX FIRMWARE COMPLETED !!!"));
                        } else {
                            Serial.println(F("RX FILE COMPLETED !!!"));
                        }
                        Serial.println(clsCanard.master.file.get_name());
                        Serial.print(F("Size: "));
                        Serial.print(getDataFileInfo(clsCanard.master.file.get_name(), clsCanard.master.file.is_firmware()));
                        Serial.println(F(" (bytes)"));
                        // Nessun altro evento necessario, chiudo File e stati
                        // procedo all'aggiornamento Firmware dopo le verifiche di conformità
                        // Ovviamente se si tratta di un file firmware
                        clsCanard.master.file.download_end();
                        // Comunico a HeartBeat (Yakut o Altri) l'avvio dell'aggiornamento (se il file è un firmware...)
                        // Per Yakut Pubblicare un HeartBeat prima dell'Avvio quindi con il flag
                        // clsCanard.local_node.file.updating_run = true >> HeartBeat Counica Upgrade...
                        if(clsCanard.master.file.is_firmware()) {
                            clsCanard.flag.set_local_node_mode(uavcan_node_Mode_1_0_SOFTWARE_UPDATE);
                        }
                        // Il Firmware Upload dovrà partire necessariamente almeno dopo l'invio completo
                        // di HeartBeat (svuotamento coda), quindi attendiamo 2/3 secondi poi via
                        // Counque non rispondo più ai comandi di update con file.updating_run = true
                        // FirmwareUpgrade(*NameFile)... -> Fra 2/3 secondi dopo HeartBeat
                    } else {
                        // Avvio prima request o nuovo blocco (Set Flag e TimeOut)
                        // Prima request (clsCanard.local_node.file.offset == 0)
                        // Firmmware Posizione blocco gestito automaticamente in sequenza Request/Read
                        clsCanard.master.file.start_pending(NODE_GETFILE_TIMEOUT_US);
                        // Gestione retry (incremento su TimeOut/Error) Automatico in Init/Request-Response
                        // Esco se raggiunga un massimo numero di retry x Frame... sopra
                        // Get Data Block per popolare il File
                        // Se il Buffer è pieno = 256 Caratteri è necessario continuare
                        // Altrimenti se inferiore o (0 Compreso) il trasferimento file termina.
                        // Se = 0 il blocco finale non viene considerato ma serve per il protocollo
                        // Se l'ultimo buffer dovesse essere pieno discrimina l'eventualità di MaxBuf==Eof 
                        handleFileReadBlock_1_1(clsCanard);
                    }
                }
            }
        }

        // LOOP HANDLER >> 1 SECONDO << HEARTBEAT
        if (clsCanard.getMonotonicMicroseconds(clsCanard.syncronized_time) >= next_01_sec_iter_at) {
            #ifdef PUBLISH_HEARTBEAT
            Serial.println(F("Publish MASTER Heartbeat -->> [1 sec]"));
            #endif
            next_01_sec_iter_at += MEGA;
            handleNormalLoop(clsCanard);
        }

        // LOOP HANDLER >> 1 SECONDO << TIME SYNCRO (alternato 0.5 sec con Heartbeat)
        if (clsCanard.getMonotonicMicroseconds(clsCanard.syncronized_time) >= next_timesyncro_msg)
        {
            #ifdef PUBLISH_TIMESYNCRO
            Serial.println(F("Publish MASTER Time Syncronization -->> [1 sec]"));
            #endif
            next_timesyncro_msg += MEGA;
            handleSyncroLoop(clsCanard);
        }

        // LOOP HANDLER >> 20 SECONDI PUBLISH SERVIZI <<
        if (clsCanard.getMonotonicMicroseconds(clsCanard.syncronized_time) >= next_20_sec_iter_at) {
            next_20_sec_iter_at += MEGA * 20;
            #ifdef PUBLISH_PORTLIST
            Serial.println(F("Publish local PORT List -- [20 sec]"));
            #endif
            handleSlowLoop(clsCanard);
        }

        #ifdef TEST_COMMAND
        // ********************** TEST COMANDO TX-> RX<- *************************
        // LOOP HANDLER >> 0..15 SECONDI x TEST COMANDI <<
        if ((monotonic_time >= test_cmd_cs_iter_at)&&(!clsCanard.master.file.updating)) {
            // TimeOUT variabile in 15 secondi
            test_cmd_cs_iter_at += MEGA * ((float)(rand() % 60)/4.0);
            // Invio un comando di test in prova al nodo 125
            // Possibile accesso da NodeId o direttamente dall'indice in coda conosciuto al master
            // Abilito disabilito pubblicazione dei dati ogni 3 secondi...
            byte queueId = getQueueNodeFromId(&state, 125);
            // Il comando viene inviato solamente se il nodo è ONLine
            if(clsCanard.slave[queueId].is_online) {
                // Il comando viene inviato solamente senza altri Pending di Comandi
                // La verifica andrebbe fatta per singolo servizio, non necessario un blocco di tutto
                if(!clsCanard.slave[queueId].command.is_pending) {
                    // Imposta il pending del comando per verifica sequenza TX-RX e il TimeOut
                    // Imposta la risposta del comadno A UNDEFINED (verrà settato al valore corretto in risposta)
                    clsCanard.slave[queueId].command.is_pending = true;
                    clsCanard.slave[queueId].command.is_timeout = false;
                    clsCanard.slave[queueId].command.timeout_us = monotonic_time + NODE_COMMAND_TIMEOUT_US;
                    clsCanard.slave[queueId].command.response = GENERIC_BVAL_UNDEFINED;
                    // Il comando inverte il servizio di pubblicazione continua dei dati remoti
                    // Semplice TEST di esempio (queueId = IDX di coda, diventerà l'istanza relativa)
                    bEnabService = !bEnabService;
                    if (bEnabService) {
                        serviceSendCommand(&state, monotonic_time, queueId,
                                            CMD_DISABLE_PUBLISH_DATA, NULL, 0);
                    } else {
                        serviceSendCommand(&state, monotonic_time, queueId,
                                            CMD_ENABLE_PUBLISH_DATA, NULL, 0);
                    }
                }
            }
            // La verifica verrà fatta con il Flag Pending resettato e la risposta viene
            // popolata nel'apposito registro di state service_module del il servizio relativo
            // Il set data avviene in processReciveTranser alle sezioni CanardTransferKindResponse
            // Eventuale Flag TimeOut indica un'avvenuta mancata risposta al comando
            // Il master una volta inviato il comando deve attendere ResetPending o TimeOutCommand
        }

        // Test rapido con nodo[0]... SOLO x OUT TEST DI VERIFICA TX/RX SEQUENZA
        // TODO: Eliminare, solo per verifica sequenza... Gestire da Master...
        if (clsCanard.slave[0].command.is_timeout) {
            // TimeOUT di un comando in attesa... gestisco il da farsi
            Serial.print(F("Timeout risposta su invio comando al nodo remoto: "));
            Serial.print(clsCanard.slave[0].node_id);
            Serial.println(F(", Warning [restore pending command]"));
            // Adesso elimino solo gli stati
            clsCanard.slave[0].command.is_pending = false;
            clsCanard.slave[0].command.is_timeout = false;
            // Adesso elimino solo gli stati per corretta visualizzazione a Serial.println
            bIsPendingCmd = false;
            bLastPendingCmd = false;
        } else {
            bIsPendingCmd = clsCanard.slave[0].command.is_pending;
            // Gestione esempio di fault (Entrata in Offline... non faccio nulla solo reset var locale test)
            if(bIsResetFaultCmd) {
                bIsPendingCmd = false;
                bLastPendingCmd = false;
            }
            if(bLastPendingCmd != bIsPendingCmd) {
                bLastPendingCmd = bIsPendingCmd;
                if(bLastPendingCmd) {
                    Serial.print(F("Inviato comando al nodo remoto: "));
                    Serial.println(clsCanard.slave[0].node_id);
                } else {
                    Serial.print(F("Ricevuto conferma di comando dal nodo remoto: "));
                    Serial.print(clsCanard.slave[0].node_id);
                    Serial.print(F(", cod. risposta ->: "));
                    Serial.println(clsCanard.slave[0].command.response);
                }
            }
        }
        // ***************** FINE TEST COMANDO TX-> RX<- *************************
        #endif

        #ifdef TEST_RMAP_DATA
        // ********************** TEST GETDATA TX-> RX<- *************************
        // LOOP HANDLER >> 0..15 SECONDI x TEST GETDATA <<
        if ((monotonic_time >= test_cmd_vs_iter_at)&&(!clsCanard.master.file.updating)) {
            // TimeOUT variabile in 15 secondi
            test_cmd_vs_iter_at += MEGA * ((float)(rand() % 60)/4.0);   
            // Invio un comando di test in prova al nodo 125
            // Possibile accesso da NodeId o direttamente dall'indice in coda conosciuto al master
            // Abilito disabilito pubblicazione dei dati ogni 5 secondi...
            byte queueId = getQueueNodeFromId(&state, 125);
            // Il comando viene inviato solamente se il nodo è ONLine
            if(clsCanard.slave[queueId].is_online) {
                // Il comando viene inviato solamente senza altri Pending di Comandi
                // La verifica andrebbe fatta per singolo servizio, non necessario un blocco di tutto
                if(!clsCanard.slave[queueId].rmap_service.is_pending) {
                    // Imposta il pending del comando per verifica sequenza TX-RX e il TimeOut
                    // La risposta al comando è già nel blocco dati, non necessaria ulteriore variabile
                    clsCanard.slave[queueId].rmap_service.is_pending = true;
                    clsCanard.slave[queueId].rmap_service.is_timeout = false;
                    clsCanard.slave[queueId].rmap_service.timeout_us = monotonic_time + NODE_GETDATA_TIMEOUT_US;
                    // Il comando richiede il dato istantaneo ad un nodo remoto
                    // Semplice TEST di esempio con i parametri in richiesta
                    rmap_service_setmode_1_0 parametri;
                    parametri.comando = rmap_service_setmode_1_0_get_istant;
                    // parametri.canale = rmap_service_setmode_1_0_CH01 (es-> set CH Analogico...)
                    // parametri.run_for_second = 900; ( inutile in get_istant )
                    serviceSendRequestData(&state, monotonic_time, queueId,
                                            rmap_service_setmode_1_0_get_istant, 10);
                }
            }
            // La verifica verrà fatta con il Flag Pending resettato e la risposta viene
            // popolata nel'apposito registro di state service_module del il servizio relativo
            // Il set data avviene in processReciveTranser alle sezioni CanardTransferKindResponse
            // Eventuale Flag TimeOut indica un'avvenuta mancata risposta al comando
            // Il master una volta inviato il comando deve attendere ResetPending o TimeOutCommand
        }

        // Test rapido con nodo[0]... SOLO x OUT TEST DI VERIFICA TX/RX SEQUENZA
        // TODO: Eliminare, solo per verifica sequenza... Gestire da Master...
        if (clsCanard.slave[0].rmap_service.is_timeout) {
            // TimeOUT di un comando in attesa... gestisco il da farsi
            Serial.print(F("Timeout risposta su richiesta dati al nodo remoto: "));
            Serial.print(clsCanard.slave[0].node_id);
            Serial.println(F(", Warning [restore pending command]"));
            // Adesso elimino solo gli stati per corretta visualizzazione a Serial.println
            clsCanard.slave[0].rmap_service.is_pending = false;
            clsCanard.slave[0].rmap_service.is_timeout = false;
            bIsPendingData = false;
            bLastPendingData = false;
        } else {
            bIsPendingData = clsCanard.slave[0].rmap_service.is_pending;
            // Gestione esempio di fault (Entrata in Offline... non faccio nulla solo reset var locale test)
            if(bIsResetFaultCmd) {
                bIsPendingData = false;
                bLastPendingData = false;
            }
            if(bLastPendingData != bIsPendingData) {
                bLastPendingData = bIsPendingData;
                if(bLastPendingData) {
                    Serial.print(F("Inviato richiesta dati al nodo remoto: "));
                    Serial.println(clsCanard.slave[0].node_id);
                } else {
                    // Interprete del messaggio in casting dal puntatore dinamico
                    // Nell'esempio Il modulo e TH, naturalmente bisogna gestire il tipo
                    // in funzione di clsCanard.slave[x].node_type
                    rmap_service_module_TH_Response_1_0* retData =
                        (rmap_service_module_TH_Response_1_0*) clsCanard.slave[0].rmap_service.module;
                    // Stampo la risposta corretta
                    Serial.print(F("Ricevuto risposta di richiesta dati dal nodo remoto: "));
                    Serial.print(clsCanard.slave[0].node_id);
                    Serial.print(F(", cod. risposta ->: "));
                    Serial.println(retData->stato);
                    // Test data Received e stampa valori con accesso al puntatore in casting per il modulo
                    Serial.println(F("Value (ITH) L1, TP, UH: "));
                    Serial.print(retData->ITH.metadata.level.L1.value);
                    Serial.print(F(", "));
                    Serial.print(retData->ITH.temperature.val.value);
                    Serial.print(F(", "));
                    Serial.println(retData->ITH.humidity.val.value);
                }
            }
        }
        // ***************** FINE TEST GETDATA TX-> RX<- *************************
        #endif

        #ifdef TEST_REGISTER
        // ********************** TEST REGISTER TX-> RX<- *************************
        // LOOP HANDLER >> 0..15 SECONDI x TEST REGISTER ACCESS <<
        if ((monotonic_time >= test_cmd_rg_iter_at)&&(!clsCanard.master.file.updating)) {
            // TimeOUT variabile in 15 secondi
            test_cmd_rg_iter_at += MEGA * ((float)(rand() % 60)/4.0);
            // Invio un comando di set registro in prova al nodo 125
            // Possibile accesso da NodeId o direttamente dall'indice in coda conosciuto al master
            byte queueId = getQueueNodeFromId(&state, 125);
            // Il comando viene inviato solamente se il nodo è ONLine
            if(clsCanard.slave[queueId].is_online)
            {
                // Il comando viene inviato solamente senza altri Pending di Comandi
                // La verifica andrebbe fatta per singolo servizio, non necessario un blocco di tutto
                if(!clsCanard.slave[queueId].register_access.is_pending) {
                    // Imposta il pending del registro per verifica sequenza TX-RX e il TimeOut
                    // Imposta la risposta del registro A UNDEFINED (verrà settato al valore corretto in risposta)
                    clsCanard.slave[queueId].register_access.is_pending = true;
                    clsCanard.slave[queueId].register_access.is_timeout = false;
                    clsCanard.slave[queueId].register_access.timeout_us = monotonic_time + NODE_REGISTER_TIMEOUT_US;
                    // Preparo il registro da inviare (configurazione generale => sequenza di request/response)
                    // Semplice TEST di esempio trasmissione di un registro fisso con nome fisso
                    // Uso in Test uavcan_register_Value_1_0 val utilizzato in ReadRegister iniziale
                    // Init Var per confronto memCmp di verifica elementi == x TEST Veloce
                    memset(&val, 0, sizeof(uavcan_register_Value_1_0));
                    // x SPECIFICHE UAVCAN ->
                    // NB Il tipo di registro deve essere == (es. Natural32) e deve esistere sul nodo Remoto
                    // Altrimenti la funzione deve fallire e ritornare NULL
                    // Quindi il Master deve conoscere la tipologia di registro e nome dello SLAVE
                    // Non è possibile creare un registro senza uscire dalle specifiche (es. comando vendor_specific)
                    if(bIsWriteRegister) {
                        // Invio il registro al nodo slave in scrittura
                        uavcan_register_Value_1_0_select_natural16_(&val);
                        val.natural32.value.count       = 1;
                        val.natural32.value.elements[0] = 12345;
                    } else {
                        // Richiedo al nodo slave la lettura (Specifiche UAVCAN _empty x Lettura)
                        uavcan_register_Value_1_0_select_empty_(&val);
                    }
                    serviceSendRegister(&state, monotonic_time, queueId,
                                        "rmap.module.TH.metadata.Level.L2", val);
                }
            }
            // La verifica verrà fatta con il Flag Pending resettato e la risposta viene
            // popolata nel'apposito registro di state service_module del il servizio relativo
            // Il set data avviene in processReciveTranser alle sezioni CanardTransferKindResponse
            // Eventuale Flag TimeOut indica un'avvenuta mancata risposta al comando
            // Il master una volta inviato il comando deve attendere ResetPending o TimeOutCommand
        }

        // Test rapido con nodo[0]... SOLO x OUT TEST DI VERIFICA TX/RX SEQUENZA
        // TODO: Eliminare, solo per verifica sequenza... Gestire da Master...
        if (clsCanard.slave[0].register_access.is_timeout) {
            // TimeOUT di un comando in attesa... gestisco il da farsi
            Serial.print(F("Timeout risposta su invio registro al nodo remoto: "));
            Serial.print(clsCanard.slave[0].node_id);
            Serial.println(F(", Warning [restore pending register]"));
            // Adesso elimino solo gli stati
            clsCanard.slave[0].register_access.is_pending = false;
            clsCanard.slave[0].register_access.is_timeout = false;
            // Adesso elimino solo gli stati per corretta visualizzazione a Serial.println
            bIsPendingRegister = false;
            bLastPendingRegister = false;
        } else {
            bIsPendingRegister = clsCanard.slave[0].register_access.is_pending;
            // Gestione esempio di fault (Entrata in Offline... non faccio nulla solo reset var locale test)
            if(bIsResetFaultCmd) {
                bIsPendingRegister = false;
                bLastPendingRegister = false;
            }
            if(bLastPendingRegister != bIsPendingRegister) {
                bLastPendingRegister = bIsPendingRegister;
                if(bLastPendingRegister) {
                    Serial.print(F("Inviato registro R/W al nodo remoto: "));
                    Serial.println(clsCanard.slave[0].node_id);
                } else {
                    Serial.print(F("Ricevuto conferma R/W registro dal nodo remoto: "));
                    Serial.println(clsCanard.slave[0].node_id);
                    Serial.print(F("Totale elementi (Natural16): "));
                    Serial.println(clsCanard.slave[0].register_access.response.unstructured.value.count);
                    for(byte bElement=0; bElement<clsCanard.slave[0].register_access.response.unstructured.value.count; bElement++) {
                        Serial.print(clsCanard.slave[0].register_access.response.natural16.value.elements[bElement]);
                        Serial.print(" ");                        
                    }
                    Serial.println();
                    // Con TX == RX Allora è una scrittura e se coincide il registro è impostato
                    // Se non coincide il comando è fallito (anche se RX = OK) TEST COMPLETO!!!
                    // Tecnicamente se != da empty in request ed in request il registro è impostato
                    // I valori != 0 in elementi extra register.value.count non sono considerati 
                    if(bIsWriteRegister) {
                        // TEST BYTE A BYTE...
                        if(memcmp(&val, &clsCanard.slave[0].register_access.response, sizeof(uavcan_register_Value_1_0)) == 0) {
                            Serial.println("Registro impostato correttamente");
                        } else {
                            Serial.println("Registro NON impostato correttamente. ALERT!!!");
                        }
                    }
                    // Inverto da Lettura a scrittura e viceversa per il TEST
                    bIsWriteRegister = !bIsWriteRegister;
                }
            }
        }
        // ***************** FINE TEST COMANDO TX-> RX<- *************************
        #endif

        // ********************** TEST UPLOAD FIRMWARE/FILE **********************
        #ifdef TEST_FIRMWARE
        // Se nodo Online, procedo, se passo OffLine interrompo la procedura
        // Oppure posso gestire in un timeOut Limitato (L'OffLine è comunque già temporizzato...)
        // Invio comando e attendo risposta affermativa, poi passo all'aggiornamento
        // NB!!! Rispondo a Request solo se sono nello stato coerente di gestione file upload
        // Questo comporta la sicurezza che il nodo non abbia avuto problematiche di genere
        // Sia come server che client. Se non sono in coerenza e non rispondo, lo slave andrà
        // in TimeOut e la procedura non finisce (senza causare quindi irregolari trasferimenti)
        // Al termine del trasferimento, esco dalla modalità >> FILE_STATE_WAIT_COMMAND <<
        // direttamente nell' invio dell' ultimo pacchetto dati. E mi preparo a nuovo invio...
        if(bEventRealTimeLoop) {
            // Avvio con il bottone di TEST il trasferimento (se non già attivo)
            // TODO: Eliminare solo x gestione messaggio non ripetuto
            if(digitalRead(USER_BTN) == LOW) {
                bFileMessage = false;
            }
            byte queueId = getQueueNodeFromId(&state, 125);
            if ((clsCanard.slave[queueId].file.state==FILE_STATE_STANDBY) &&
                (digitalRead(USER_BTN) == HIGH)) {
                if(clsCanard.slave[queueId].is_online) {
                    bFileUpload = true;
                    // N.B.!!! Si tratta di un file di firmware... nel TEST
                    clsCanard.slave[queueId].file.is_firmware = true;
                    clsCanard.slave[queueId].file.state = FILE_STATE_BEGIN_UPDATE;
                    Serial.print(F("Node ["));
                    Serial.print(clsCanard.slave[queueId].node_id);
                    if(clsCanard.slave[queueId].file.is_firmware) {
                        Serial.println(F("] FIRMWARE start function !!!"));
                    } else {
                        Serial.println(F("] FILE start function !!!"));
                    }
                } else {
                    if(!bFileMessage) {
                        bFileMessage = true;
                        Serial.print(F("Node ["));
                        Serial.print(clsCanard.slave[queueId].node_id);
                        Serial.println(F("] Is OFFLine "));
                        if(clsCanard.slave[queueId].file.is_firmware) {
                            Serial.println(F("FIRMWARE send update ABORT!!!"));
                        } else {
                            Serial.println(F("FILE send update ABORT!!!"));
                        }
                    }
                }
            }
            if(bFileUpload) {
                // Se vado OffLine la procedura comunque viene interrotta dall'evento di OffLine
                switch(clsCanard.slave[queueId].file.state) {
                    default: // -->> FILE_STATE_STANDBY:
                        // Init, exit ecc...
                        // TODO: Eliminare, usato come test EXIT
                        bFileUpload = false;
                        break;
                    case FILE_STATE_BEGIN_UPDATE:
                        // Avvio comando di aggiornamento Controllo coerenza Firmware se Firmware!!!
                        // Verifico il nome File locale (che RMAP Server ha già inviato il file in HTTP...)
                        if(clsCanard.slave[queueId].file.is_firmware) {
                            if(ccFirwmareFile(clsCanard.slave[queueId].file.filename)) {
                                // Avvio il comando nel nodo remoto
                                clsCanard.slave[queueId].file.state = FILE_STATE_COMMAND_SEND;
                            } else {
                                // Gestisco l'errore di coerenza verso il server
                                // Comunico il problema nel file
                                clsCanard.slave[queueId].file.state = FILE_STATE_STANDBY;
                            }
                        } else {
                            // N.B. Eventuale altro controllo di coerenza...
                            // Al momento non gestisco in quanto un eventuale LOG non è problematico
                            // Avvio il comando nel nodo remoto
                            clsCanard.slave[queueId].file.state = FILE_STATE_COMMAND_SEND;
                        }
                        break;
                    case FILE_STATE_COMMAND_SEND:
                        // Invio comando di aggiornamento File (in attesa in coda con switch...)
                        // Il comando viene inviato solamente senza altri Pending di Comandi (come semaforo)
                        if(!clsCanard.slave[queueId].command.is_pending) {
                            // Imposta il pending del comando per verifica sequenza TX-RX e il TimeOut
                            // Imposta la risposta del comadno A UNDEFINED (verrà settato al valore corretto in risposta)
                            clsCanard.slave[queueId].command.is_pending = true;
                            clsCanard.slave[queueId].command.is_timeout = false;
                            clsCanard.slave[queueId].command.timeout_us = monotonic_time + NODE_COMMAND_TIMEOUT_US;
                            clsCanard.slave[queueId].command.response = GENERIC_BVAL_UNDEFINED;
                            // Il comando comunica la presenza del file di download nel remoto e avvia dowload
                            if(clsCanard.slave[queueId].file.is_firmware) {
                                // Uavcan Firmware
                                serviceSendCommand(&state, monotonic_time, queueId,
                                                    uavcan_node_ExecuteCommand_Request_1_1_COMMAND_BEGIN_SOFTWARE_UPDATE,
                                                    clsCanard.slave[queueId].file.filename, strlen(clsCanard.slave[queueId].file.filename));
                            } else {
                                // Uavcan File con comando locale
                                serviceSendCommand(&state, monotonic_time, queueId,
                                                    CMD_DOWNLOAD_FILE,
                                                    clsCanard.slave[queueId].file.filename, strlen(clsCanard.slave[queueId].file.filename));
                            }
                            clsCanard.slave[queueId].file.state = FILE_STATE_COMMAND_WAIT;
                        }
                        break;
                    case FILE_STATE_COMMAND_WAIT:
                        // Attendo la risposta del Nodo Remoto conferma, errore o TimeOut
                        if(clsCanard.slave[queueId].command.is_timeout) {
                            // Counico al server l'errore di timeOut Command Update Start ed esco
                            clsCanard.slave[queueId].file.state = FILE_STATE_STANDBY;
                            Serial.print(F("Node ["));
                            Serial.print(clsCanard.slave[queueId].node_id);                            
                            Serial.println(F("] TimeOut Command Start Send file "));
                            if(clsCanard.slave[queueId].file.is_firmware) {
                                Serial.println(F("FIRMWARE send update ABORT!!!"));
                            } else {
                                Serial.println(F("FILE send update ABORT!!!"));
                            }
                            // Se decido di uscire nella procedura di OffLine, la comunicazione
                            // al server di eventuali errori deve essere gestita al momento dell'OffLine
                        } else {
                            // Attendo esecuzione del comando/risposta senza sovrapposizioni
                            // di comandi. Come gestione semaforo. Chi invia deve gestire
                            if(!clsCanard.slave[queueId].command.is_pending) {
                                // La risposta si trova in command_response fon flag pending azzerrato.
                                if(clsCanard.slave[queueId].command.response == 
                                    uavcan_node_ExecuteCommand_Response_1_1_STATUS_SUCCESS) {
                                    // Sequenza terminata, avvio il file transfer !!!
                                    clsCanard.slave[queueId].file.state = FILE_STATE_UPLOADING;
                                    Serial.print(F("Node ["));
                                    Serial.print(clsCanard.slave[queueId].node_id);
                                    Serial.println(F("] Upload Start Send file OK"));
                                    // Imposto il timeOUT per controllo Deadline sequenza di download
                                    // SE il client si ferma per troppo tempo incorro in TimeOut ed esco
                                    clsCanard.slave[queueId].file.timeout_us = monotonic_time + NODE_REQFILE_TIMEOUT_US;
                                    // Avvio la funzione (Slave attiva le request)
                                    // Localmente sono in gestione continua file fino a EOF o TimeOUT
                                    clsCanard.slave[queueId].file.is_pending = true;
                                    clsCanard.slave[queueId].file.is_timeout = false;
                                    // Il TimeOut deadLine è aggiornato in RX Request in automatico
                                } else {
                                    // Counico al server l'errore per il mancato aggiornamento ed esco
                                    clsCanard.slave[queueId].file.state = FILE_STATE_STANDBY;
                                    Serial.print(F("Node ["));
                                    Serial.print(clsCanard.slave[queueId].node_id);
                                    Serial.println(F("] Response Cmd Error in Send file"));
                                    if(clsCanard.slave[queueId].file.is_firmware) {
                                        Serial.println(F("FIRMWARE send update ABORT!!!"));
                                    } else {
                                        Serial.println(F("FILE send update ABORT!!!"));
                                    }
                                }
                            }
                        }
                        break;
                    case FILE_STATE_UPLOADING:
                        // Attendo la risposta del Nodo Remoto conferma, errore o TimeOut
                        if(clsCanard.slave[queueId].file.is_timeout) {
                            // Counico al server l'errore di timeOut Command Update Start ed esco
                            clsCanard.slave[queueId].file.state = FILE_STATE_STANDBY;
                            Serial.print(F("Node ["));
                            Serial.print(clsCanard.slave[queueId].node_id);
                            Serial.println(F("] TimeOut Request/Response Send file "));
                            if(clsCanard.slave[queueId].file.is_firmware) {
                                Serial.println(F("FIRMWARE send update ABORT!!!"));
                            } else {
                                Serial.println(F("FILE send update ABORT!!!"));
                            }
                            // Se decido di uscire nella procedura di OffLine, la comunicazione
                            // al server di eventuali errori deve essere gestita al momento dell'OffLine
                        }
                        break;
                    case FILE_STATE_UPLOAD_COMPLETE:
                        // Counico al server file upload Complete ed esco (nuova procedura ready)
                        // -> EXIT FROM FILE_STATE_STANDBY ( In procedura di SendFileBlock )
                        // Quando invio l'ultimo pacchetto dati valido ( Blocco < 256 Bytes )
                        clsCanard.slave[queueId].file.state = FILE_STATE_STANDBY;
                        Serial.print(F("Node ["));
                        Serial.print(clsCanard.slave[queueId].node_id);
                        if(clsCanard.slave[queueId].file.is_firmware) {
                            Serial.println(F("FIRMWARE send Complete!!!"));
                        } else {
                            Serial.println(F("FILE send Complete!!!"));
                        }
                        break;
                }
            }
        }
        // ******************** FINE TEST UPLOAD FIRWARE/FILE ********************
        #endif

        // Fine handler quasi RealTime...
        // Attendo nuovo evento per rielaborare
        // Utilizzato per blinking Led (TX/RX)
        if(bEventRealTimeLoop) {
            bEventRealTimeLoop = false; 
            #if defined(LED_ON_CAN_DATA_TX) or defined(LED_ON_CAN_DATA_RX)
            digitalWrite(LED_BUILTIN, LOW);
            #endif
        };

        // RESET Gestione Fault di NODO TODO: Eliminare solo per verifica test
        bIsResetFaultCmd = false;

        // ***************************************************************************
        //   Gestione Coda messaggi in trasmissione (ciclo di svuotamento messaggi)
        // ***************************************************************************
        // Transmit pending frames, avvia la trasmissione gestita da canard a priorità.
        // Il metodo può essere chiamato direttamente in preparazione messaggio x coda
        if (clsCanard.transmitQueueDataPresent()) {
            #ifdef LED_ON_CAN_DATA_TX
            digitalWrite(LED_BUILTIN, HIGH);
            #endif
            clsCanard.transmitQueue();
        }

        // ***************************************************************************
        //   Gestione Coda messaggi in ricezione (ciclo di caricamento messaggi)
        // ***************************************************************************
        // Gestione con Intererupt RX Only esterna (verifica dati in coda gestionale)
        if (clsCanard.receiveQueueDataPresent()) {
            #ifdef LED_ON_CAN_DATA_RX
            digitalWrite(LED_BUILTIN, HIGH);
            #endif
            // Log Packet
            #ifdef LOG_RX_PACKET
            char logMsg[50];
            clsCanard.receiveQueue(logMsg);
            Serial.println(logMsg);
            #else
            clsCanard.receiveQueue();
            #endif
        }
 
    } while (!clsCanard.flag.is_requested_system_restart());

    // Reboot
    NVIC_SystemReset();
}