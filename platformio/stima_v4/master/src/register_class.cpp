/**
  ******************************************************************************
  * @file    register_class.cpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   Register class (Uavcan/Other) cpp source
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

#include "register_class.hpp"
#include "canard_config.hpp"

#include "config.h"
#include "stima_utility.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define PASTE3_IMPL(x, y, z) x##y##z
#define PASTE3(x, y, z) PASTE3_IMPL(x, y, z)

// ***************************************************************************************
//                 E2PROM STIMAV4 STM32 ARDUINO REGISTER CLASS ACCESS
// ***************************************************************************************

// Contructor
EERegister::EERegister()
{
}
EERegister::EERegister(TwoWire *wire, BinarySemaphore *wireLock, uint8_t i2c_address)
{
    // Memory controller for ClassRegister
    _Eprom = EEprom(wire, wireLock);
}

/// @brief Wrapper memory_write_block
/// @param address Address to write
/// @param data data to write
/// @param len packet len
void EERegister::_memory_write_block(uint16_t address, uint8_t *data, uint8_t len) {
    _Eprom.Write(address, data, len);
}

/// @brief Wrapper memory_read_block
/// @param address Address to read
/// @param data data readed
/// @param len packet len request
void EERegister::_memory_read_block(uint16_t address, uint8_t *data, uint8_t len) {
    _Eprom.Read(address, data, len);
}

/// @brief Wrapper memory_write_byte
/// @param address Address to write
/// @param data single byte data to write
void EERegister::_memory_write_byte(uint16_t address, uint8_t data) {
    _Eprom.Write(address, data);
}

/// @brief Wrapper memory_read_byte
/// @param address Address to read
/// @param data single byte readed
void EERegister::_memory_read_byte(uint16_t address, uint8_t *data) {
    _Eprom.Read(address, data);
}

/// @brief Inizializza l'area memory (indice) dedicata a REGISTER
/// @param  None
void EERegister::_eeprom_register_factory(void) {
    uint8_t register_index[MEM_UAVCAN_MAX_REG];
    // Scrivo in un unica tornata
    memset(register_index, MEM_UAVCAN_REG_UNDEF, MEM_UAVCAN_MAX_REG);
    _memory_write_block(MEM_UAVCAN_GET_ADDR_FLAG(), register_index, MEM_UAVCAN_MAX_REG);
}

/// @brief Inizializza/Elimina un registro CYPAL/STIMAV4 dalla memoria
/// @param reg_numb numero di registro da eliminare
void EERegister::_eeprom_register_clear(uint8_t reg_numb) {
    // Controllo area register
    if(reg_numb<MEM_UAVCAN_MAX_REG)
        _memory_write_byte(MEM_UAVCAN_GET_ADDR_FLAG_REG(reg_numb), MEM_UAVCAN_REG_UNDEF);
}

/// @brief Legge un registro CYPAL/STIMAV4 dalla memoria (per indice)
///        (fast=senza controlli validità) la procedura chiamante si occupa dei limiti
/// @param reg_numb (IN) Numero di regsitro da leggere
/// @param reg_name (OUT) Nome del resistro UAVCAN/CYPAL
/// @param data (OUT) Valore del registro
/// @return lunghezza del registro
size_t EERegister::_eeprom_register_get_fast(uint8_t reg_numb, uint8_t *reg_name, uint8_t *reg_value) {
    uint8_t read_block[MEM_UAVCAN_LEN_REG];
    uint8_t reg_valid = MEM_UAVCAN_REG_UNDEF;
    uint8_t data_len;
    uint8_t name_len;
    // Leggo il blocco in un unica read
    _memory_read_block(MEM_UAVCAN_GET_ADDR_BASE_REG(reg_numb), read_block, MEM_UAVCAN_LEN_REG);
    // Ritorno i campi name e value corretti
    name_len = read_block[MEM_UAVCAN_POS_LEN_NAME];
    data_len = read_block[MEM_UAVCAN_POS_LEN_DATA];
    memcpy(reg_name, &read_block[MEM_UAVCAN_POS_STR_NAME], name_len);
    memcpy(reg_value, &read_block[MEM_UAVCAN_POS_VALUE_DATA], data_len);
    return (size_t)data_len;
}

/// @brief Legge un registro CYPAL/STIMAV4 dalla memoria (per indice)
///        (fast=senza controlli validità) la procedura chiamante si occupa dei limiti
/// @param reg_numb (IN) Numero di regsitro da leggere
/// @return lunghezza del registro indirizzato
size_t EERegister::_eeprom_register_get_len_intest_fast(uint8_t reg_numb) {
    uint8_t name_len;
    // Registro eeprom valido, ritorno i campi name e value
    _memory_read_byte(MEM_UAVCAN_GET_ADDR_NAME_LEN(reg_numb), &name_len);
    return (size_t)name_len;
}

/// @brief Legge un registro CYPAL/STIMAV4 dalla memoria (per indice)
///        (fast=senza controlli validità) la procedura chiamante si occupa dei limiti
/// @param reg_numb (IN) Numero di regsitro da leggere
/// @param reg_name (OUT) Nome del resistro UAVCAN/CYPAL
/// @param name_len (IN) Lunghezza del messaggio di intestazione (preventivamente letto)
/// @return None
void EERegister::_eeprom_register_get_intest_fast(uint8_t reg_numb, uint8_t *reg_name, uint8_t name_len) {
    // Registro eeprom valido, ritorno i campi name e value
    _memory_read_block(MEM_UAVCAN_GET_ADDR_NAME(reg_numb), reg_name, name_len);
}

/// @brief Legge un registro CYPAL/STIMAV4 dalla EEprom (per indice)
/// @param reg_numb (IN) Numero di regsitro da leggere
/// @param reg_name (OUT) Nome del resistro UAVCAN/CYPAL
/// @return true se registro trovato
bool EERegister::_eeprom_register_get_name_from_index(uint8_t reg_numb, uint8_t *reg_name) {
    uint8_t reg_valid = MEM_UAVCAN_REG_UNDEF;
    uint8_t len_name;
    // Controllo area register
    if(reg_numb>=MEM_UAVCAN_MAX_REG) return false;
    // Leggo l'indice se impostato (registro valido)
    _memory_read_byte(MEM_UAVCAN_GET_ADDR_FLAG_REG(reg_numb), &reg_valid);
    if(reg_valid == reg_numb) {
        // Registro eeprom valido, ritorno i campi name e value
        _memory_read_byte(MEM_UAVCAN_GET_ADDR_NAME_LEN(reg_numb), &len_name);
        _memory_read_block(MEM_UAVCAN_GET_ADDR_NAME(reg_numb), reg_name, len_name);
        return true;
    } 
    // Registro non impostato correttamente
    return false;
}

/// @brief Legge un registro CYPAL/STIMAV4 dalla memoria (per Nome)
/// @param reg_name (IN) Nome del regsitro da leggere
/// @param reg_numb (OUT) Numero del registro
/// @param data (OUT) Valore del registro
/// @return lunghezza del registro se valido, altrimenti 0
size_t EERegister::_eeprom_register_get_from_name(uint8_t const *reg_name, uint8_t *reg_numb, uint8_t *data) {    
    uint8_t _reg_name[MEM_UAVCAN_LEN_NAME_REG];
    uint8_t _reg_data[MEM_UAVCAN_LEN_VALUE_REG];
    uint8_t _len_name = strlen((char*)reg_name);
    size_t _len_data = 0;
    uint8_t register_index[MEM_UAVCAN_MAX_REG];
    // Leggo l'intero status register per velocizzare l'indice di ricerca
    // Controllo preventivo dell'array di validità dei registri indicizzata
    _memory_read_block(MEM_UAVCAN_GET_ADDR_FLAG(), register_index, MEM_UAVCAN_MAX_REG);
    // Controllo area register (Search for name)
    for(uint8_t reg_index = 0; reg_index<MEM_UAVCAN_MAX_REG; reg_index++) {
        // Test Data (Register Valid && Len Intest == for Rapid Check)
        if(register_index[reg_index]!=MEM_UAVCAN_REG_UNDEF) {
            // eeprom_register_get_len_intest_fast -> Rapido senza controllo validità REG
            // La sua chiamata prevede un controllo preventivo della validità del REG
            if (_eeprom_register_get_len_intest_fast(reg_index) == _len_name) {
                // Retrieve all info register value
                _len_data = _eeprom_register_get_fast(reg_index, _reg_name, _reg_data);
                // Compare value name
                if(memcmp(reg_name, _reg_name, _len_name) == 0) {
                    // Data is found
                    memcpy(data, _reg_data, _len_data);
                    *reg_numb = reg_index;
                    return _len_data;
                }
            }
        }
    }
    // End Of Size ROM (Register Not Found)
    return 0;
}

/// @brief Legge un indiced di registro CYPAL/STIMAV4 dalla memoria (per Nome)
/// @param reg_name (IN) Nome del regsitro da leggere
/// @return indice del registro se esiste altrimenti MEM_UAVCAN_REG_UNDEF
uint8_t EERegister::_eeprom_register_get_index_from_name(uint8_t *reg_name) {    
    uint8_t _reg_name[MEM_UAVCAN_LEN_NAME_REG];
    uint8_t _reg_data[MEM_UAVCAN_LEN_VALUE_REG];
    uint8_t _len_name = strlen((char*)reg_name);
    uint8_t register_index[MEM_UAVCAN_MAX_REG];
    // Leggo l'intero status register per velocizzare l'indice di ricerca
    // Controllo preventivo dell'array di validità dei registri indicizzata
    _memory_read_block(MEM_UAVCAN_GET_ADDR_FLAG(), register_index, MEM_UAVCAN_MAX_REG);
    // Controllo area register (Search for name)
    for(uint8_t reg_index = 0; reg_index<MEM_UAVCAN_MAX_REG; reg_index++) {
        // Test Data (Register Valid && Len Intest == for Rapid Check)
        if(register_index[reg_index]!=MEM_UAVCAN_REG_UNDEF) {
            // eeprom_register_get_len_intest_fast -> Rapido senza controllo validità REG
            // La sua chiamata prevede un controllo preventivo della validità del REG
            if (_eeprom_register_get_len_intest_fast(reg_index) == _len_name) {
                // Test Data (Register Valid)
                _eeprom_register_get_intest_fast(reg_index, _reg_name, _len_name);
                // Compare value name
                if(memcmp(reg_name, _reg_name, _len_name) == 0) {
                    // Data is found
                    return reg_index;
                }
            }
        }
    }
    // End Of Size ROM (Register Not Found)
    return MEM_UAVCAN_REG_UNDEF;
}

/// @brief Scrive/edita un registro CYPAL/STIMAV4 sulla memoria
/// @param reg_numb Numero di regsitro da impostare
/// @param reg_name Nome del resistro UAVCAN/CYPAL
/// @param data Valore del registro
void EERegister::_eeprom_register_set(uint8_t reg_numb, uint8_t *reg_name, uint8_t *data, size_t len_data) {
    uint8_t reg_valid;
    uint8_t name_len = strlen((char*)reg_name);
    uint8_t write_block[MEM_UAVCAN_LEN_REG] = {0};
    // Controllo area register
    if(reg_numb>=MEM_UAVCAN_MAX_REG) return;
    // Leggo l'indice se impostato (registro valido)
    _memory_read_byte(MEM_UAVCAN_GET_ADDR_FLAG_REG(reg_numb), &reg_valid);
    if(reg_valid != reg_numb) {
        // Imposto il Numero sul BYTE relativo (Registro inizializzato)
        _memory_write_byte(MEM_UAVCAN_GET_ADDR_FLAG_REG(reg_numb), reg_numb);
    }
    // Registro eeprom valido, imposto i campi name e value
    write_block[MEM_UAVCAN_POS_LEN_NAME] = name_len;
    write_block[MEM_UAVCAN_POS_LEN_DATA] = len_data;
    memcpy(write_block + MEM_UAVCAN_POS_STR_NAME, reg_name, name_len);
    memcpy(write_block + MEM_UAVCAN_POS_VALUE_DATA, data, len_data);
    // Perform in unica scrittura
    _memory_write_block(MEM_UAVCAN_GET_ADDR_BASE_REG(reg_numb), write_block, MEM_UAVCAN_LEN_REG);
}

/// @brief Ritorna il prossimo indice (se esiste) valido nella sezione memoria Cypal dedicata
/// @param [IN/OUT] current_register indirizzo di partenza nel campo di validità [MEM_UAVCAN_MAX_REG]
/// @return None
void EERegister::_eeprom_register_get_next_id(uint8_t *current_register) {
    uint8_t register_index[MEM_UAVCAN_MAX_REG];
    // Controllo se richiesto avvio dall'inizio della coda... get_next(MAX)...
    bool is_first = (*current_register==MEM_UAVCAN_REG_UNDEF);
    // Continuo alla ricerca del prossimo register se esiste
    _memory_read_block(MEM_UAVCAN_GET_ADDR_FLAG(), register_index, MEM_UAVCAN_MAX_REG);
    while((*current_register<MEM_UAVCAN_MAX_REG) || (is_first))
    {
        // Starting attempt        
        if(is_first)
            *current_register = 0;
        else
            (*current_register)++;
        is_first = false;
        if(register_index[*current_register] == *current_register) {
            // Found... exit
            break;
        }
    }
}

/// @brief Aggiunge un registro alla configurazione CYPAL/STIMAV4
/// @param reg_name Nome del resistro UAVCAN/CYPAL
/// @param data Valore del registro
/// @return indice dell'elemento inserito nello stazio EEprom Cypal Dedicato [FAIL = MEM_UAVCAN_REG_UNDEF]
uint8_t EERegister::_eeprom_register_add(uint8_t *reg_name, uint8_t *data, size_t data_len) {
    uint8_t register_index[MEM_UAVCAN_MAX_REG];
    // Vado alla ricerca del prossimo register se esiste (parto dal primo...)
    _memory_read_block(MEM_UAVCAN_GET_ADDR_FLAG(), register_index, MEM_UAVCAN_MAX_REG);
    for(uint8_t register_ptr=0; register_ptr<MEM_UAVCAN_MAX_REG; register_ptr++) {
        if(register_index[register_ptr] == MEM_UAVCAN_REG_UNDEF) {
            _eeprom_register_set(register_ptr, reg_name, data, data_len);
            return register_ptr;
        }
    }
    return MEM_UAVCAN_REG_UNDEF;
}

/// @brief Check if exist or create space register with init default value
/// @param register_init (true, perform an init to default value)
void EERegister::setup(void)
{
    // Init AREA E2PROM
    _eeprom_register_factory();

    // Open Register in Write se non inizializzati correttamente...
    // Populate INIT Default Value
    static uavcan_register_Value_1_0 val = {0};
    uavcan_register_Value_1_0_select_natural16_(&val);
    val.natural16.value.count       = 1;
    val.natural16.value.elements[0] = CAN_MTU_BASE; // CAN_CLASSIC MTU 8
    write(REGISTER_UAVCAN_MTU, &val);

    // We also need the bitrate configuration register. In this demo we can't really use it but an embedded application
    // should define "uavcan.can.bitrate" of type natural32[2]; the second value is 0/ignored if CAN FD not supported.
    // TODO: Default a CAN_BIT_RATE, se CAN_BIT_RATE <> readRegister setup bxCAN con nuovo RATE hot reload
    uavcan_register_Value_1_0_select_natural32_(&val);
    val.natural32.value.count       = 2;
    val.natural32.value.elements[0] = CAN_BIT_RATE;
    val.natural32.value.elements[1] = 0ul;          // Ignored for CANARD_MTU_CAN_CLASSIC
    write(REGISTER_UAVCAN_BITRATE, &val);

    // Node ID
    #ifdef NODE_MASTER_ID
    uavcan_register_Value_1_0_select_natural16_(&val);
    val.natural16.value.count = 1;
    val.natural16.value.elements[0] = NODE_MASTER_ID; // This means undefined (anonymous), per Specification/libcanard.
    write(REGISTER_UAVCAN_NODE_ID, &val);       // The names of the standard registers are regulated by the Specification.
    #endif

    // Service RMAP
    #ifdef PORT_SERVICE_MASTER
    uavcan_register_Value_1_0_select_natural16_(&val);
    val.natural16.value.count       = 1;
    val.natural16.value.elements[0] = PORT_SERVICE_MASTER;
    write(REGISTER_UAVCAN_DATA_SERVICE, &val);
    #endif

    // Publish RMAP
    #ifdef SUBJECTID_PUBLISH_MASTER
    uavcan_register_Value_1_0_select_natural16_(&val);
    val.natural16.value.count       = 1;
    val.natural16.value.elements[0] = SUBJECTID_PUBLISH_MASTER;
    write(REGISTER_UAVCAN_DATA_PUBLISH, &val);
    #endif

    // The description register is optional but recommended because it helps constructing/maintaining large networks.
    // It simply keeps a human-readable description of the node that should be empty by default.
    char stima_description[STIMA_MODULE_DESCRIPTION_LENGTH] = {0};
    getStimaDescriptionByType(stima_description, MODULE_TYPE);
    uavcan_register_Value_1_0_select_string_(&val);
    val._string.value.count = strlen(stima_description);
    memcpy(val._string.value.elements, stima_description, val._string.value.count);
    write(REGISTER_UAVCAN_NODE_DESCR, &val);  // We don't need the value, we just need to ensure it exists.
}

/// @brief Legge un registro Cypal/Uavcan wrapper UAVCAN 
///        (Imposta Default su Set inout_value su value se non esiste)
/// @param register_name nome del registro
/// @param inout_value valore del registro (formato uavcan) -> Set Valore default
void EERegister::read(const char* const register_name, uavcan_register_Value_1_0* const inout_value) {
    LOCAL_ASSERT(inout_value != NULL);

    uint8_t register_number;
    bool init_required = !uavcan_register_Value_1_0_is_empty_(inout_value);
    uint8_t serialized[uavcan_register_Value_1_0_EXTENT_BYTES_] = {0};
    size_t sr_size = _eeprom_register_get_from_name((const uint8_t*)register_name, &register_number, serialized);
    
    static uavcan_register_Value_1_0 out = {0};
    const int8_t err = uavcan_register_Value_1_0_deserialize_(&out, serialized, &sr_size);
    if (err >= 0) init_required = !assign(inout_value, &out);
    if (init_required) write(register_name, inout_value);
}

/// @brief Scrive un registro Cypal/Uavcan wrapper UAVCAN
/// @param register_name nome del registro
/// @param value valore del registro (formato uavcan)
void EERegister::write(const char* const register_name, const uavcan_register_Value_1_0* const value) {
    uint8_t serialized[uavcan_register_Value_1_0_EXTENT_BYTES_] = {0};
    size_t sr_size = uavcan_register_Value_1_0_EXTENT_BYTES_;
    const int8_t err = uavcan_register_Value_1_0_serialize_(value, serialized, &sr_size);
    uint8_t register_index = _eeprom_register_get_index_from_name((uint8_t*) register_name);
    if(register_index == MEM_UAVCAN_REG_UNDEF) {
        printf("Init register: %s\n", register_name);
        _eeprom_register_add((uint8_t*) register_name, serialized, sr_size);
    } else {
        _eeprom_register_set(register_index, (uint8_t*) register_name, serialized, sr_size);
    }
}

/// @brief Scroll degli indici dal primo all'ultimo e return ID UavCAN
///        Nel passaggio di un eventuale INDICE vuoto, non viene incrementato l'ID
///        ID Uavcan è solo l'elenco sequenziale degli ID registarti validi
///        La procedura scorre tutta l'area registri e ritorda IDX+1 alla lettura di un ID Valido 
/// @param index indice di controllo
/// @return UavCan Name Register Formato UAVCAN
uavcan_register_Name_1_0 EERegister::getNameByIndex(const uint16_t index) {
    uavcan_register_Name_1_0 out = {0};
    uavcan_register_Name_1_0_initialize_(&out);
    // N.B. Get starting list from first data Next of MEM_UAVCAN_REG_UNDEF -> = 0
    // Get First...
    if(index > MEM_UAVCAN_LEN_NAME_REG)
        return out;
    uint8_t reg_current = MEM_UAVCAN_REG_UNDEF;
    _eeprom_register_get_next_id(&reg_current);
    uint8_t reg_index = 0;
    while(reg_current<=MEM_UAVCAN_MAX_REG) {
        // Index Exact
        if(reg_index==index) {
            uint8_t _reg_name[MEM_UAVCAN_LEN_NAME_REG] = {0};
            _eeprom_register_get_name_from_index(reg_current, _reg_name);
            out.name.count = nunavutChooseMin(strlen((char*)_reg_name), uavcan_register_Name_1_0_name_ARRAY_CAPACITY_);
            memcpy(out.name.elements, _reg_name, out.name.count);
            break;
        }
        // Get Next register E2prom
        _eeprom_register_get_next_id(&reg_current);
        // Set next Id register list contiguos UAVCAN
        reg_index++;
    }
    return out;
}

/// @brief Set factoryReset Register UAVCAN
/// @param  None
void EERegister::doFactoryReset(void) {
    setup();
}

/// @brief Register type Assign UAVCAN
/// @param dst destination register
/// @param src source register
/// @return Register assign completed OK
bool EERegister::assign(uavcan_register_Value_1_0* const dst, const uavcan_register_Value_1_0* const src) {
    if (uavcan_register_Value_1_0_is_empty_(dst)) {
        *dst = *src;
        return true;
    }
    if ((uavcan_register_Value_1_0_is_string_(dst) && uavcan_register_Value_1_0_is_string_(src)) ||
        (uavcan_register_Value_1_0_is_unstructured_(dst) && uavcan_register_Value_1_0_is_unstructured_(src))) {
        *dst = *src;
        return true;
    }
    if (uavcan_register_Value_1_0_is_bit_(dst) && uavcan_register_Value_1_0_is_bit_(src)) {
        nunavutCopyBits(dst->bit.value.bitpacked,
                        0,
                        nunavutChooseMin(dst->bit.value.count, src->bit.value.count),
                        src->bit.value.bitpacked,
                        0);
        return true;
    }
    // This is a violation of MISRA/AUTOSAR but it is believed to be less error-prone than manually copy-pasted code.
    #define REGISTER_CASE_SAME_TYPE(TYPE)                                                                                 \
    if (PASTE3(uavcan_register_Value_1_0_is_, TYPE, _)(dst) && PASTE3(uavcan_register_Value_1_0_is_, TYPE, _)(src)) { \
        for (size_t i = 0; i < nunavutChooseMin(dst->TYPE.value.count, src->TYPE.value.count); ++i) {                 \
            dst->TYPE.value.elements[i] = src->TYPE.value.elements[i];                                                \
        }                                                                                                             \
        return true;                                                                                                  \
    }
    REGISTER_CASE_SAME_TYPE(integer64)
    REGISTER_CASE_SAME_TYPE(integer32)
    REGISTER_CASE_SAME_TYPE(integer16)
    REGISTER_CASE_SAME_TYPE(integer8)
    REGISTER_CASE_SAME_TYPE(natural64)
    REGISTER_CASE_SAME_TYPE(natural32)
    REGISTER_CASE_SAME_TYPE(natural16)
    REGISTER_CASE_SAME_TYPE(natural8)
    REGISTER_CASE_SAME_TYPE(real64)
    REGISTER_CASE_SAME_TYPE(real32)
    REGISTER_CASE_SAME_TYPE(real16)
    return false;
}