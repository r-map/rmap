///                            ____                   ______            __          __
///                           / __ `____  ___  ____  / ____/_  ______  / /_  ____  / /
///                          / / / / __ `/ _ `/ __ `/ /   / / / / __ `/ __ `/ __ `/ /
///                         / /_/ / /_/ /  __/ / / / /___/ /_/ / /_/ / / / / /_/ / /
///                         `____/ .___/`___/_/ /_/`____/`__, / .___/_/ /_/`__,_/_/
///                             /_/                     /____/_/
///
/// Registers are named values that keep various configuration parameters of the local Cyphal node (application).
/// Some of these parameters are used by the business logic of the application (e.g., PID gains, perfcounters);
/// others are used by the Cyphal stack (e.g., port-IDs, node-ID, transport configuration, introspection, and so on).
/// Registers of the latter category are all named with the same prefix "uavcan.", and their names and semantics
/// are regulated by the Cyphal Specification to ensure consistency across the ecosystem.
///
/// The Specification doesn't define how the registers are to be stored since this part does not affect network
/// interoperability. In this demo we use a very simple and portable approach where each register is stored as
/// a separate file in the local filesystem; the name of the file matches the name of the register, and the register
/// values are serialized in the DSDL format (i.e., same format that is used for network exchange).
/// Deeply embedded systems may either use the same approach with the help of some compact fault-tolerant filesystem
/// (such as, for example, LittleFS: https://github.com/littlefs-project/littlefs), or they can resort to a low-level
/// specialized approach using on-chip EEPROM or similar (like PX4, Sapog, etc).
///
/// This software is distributed under the terms of the MIT License.
/// Copyright (C) 2021 OpenCyphal <maintainers@opencyphal.org>
/// Author: Pavel Kirienko <pavel@opencyphal.org>

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <uavcan/_register/Access_1_0.h>
#include <uavcan/_register/List_1_0.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef USE_STIMA4_E2P
// SETUP SD Mode SdFat Library Mode
bool setupSd(const uint32_t bMOSI, const uint32_t bMISO, const uint32_t bSCLK, const uint32_t sCS, const int speedMHZ);
#endif

// #define TEST_PERFORM_E2
#ifdef USE_STIMA4_E2P
    #ifdef TEST_PERFORM_E2
    void perform_init(void);
    void perform_write(void);
    #else
    #define perform_init()      void(0)
    #define perform_start(x)    void(0)
    #define perform_add(x)      void(0)
    #define perform_write()     void(0)
    #endif
#endif

// SETUP SD Mode SdFat Library Mode
bool setupSd(const uint32_t bMOSI, const uint32_t bMISO, const uint32_t bSCLK, const uint32_t sCS, const int speedMHZ);

// Scrive dati in append per scrittura sequenziale file firmware
void putDataFile(const char* const file_name, const bool is_firmware, const bool rewrite, void* buf, size_t count);

// Legge dati in seek per lettura sequenziale file firmware
bool getDataFile(const char* const file_name, const bool is_firmware, uint64_t position, void* buf, size_t *count);

// Restituisce le info per file firmware e controlli vari
uint64_t getDataFileInfo(const char* const file_name, const bool is_firmware);

// Ritorna vero se il file (firmware) esiste ed è coerente (Controllo coerenza, esiste..CRC..altro...)
bool checkFile(const char* const file_name, bool isFirmware);

// NOTE: this implementation currently does not differentiate between mutable/immutable registers and does not support
// volatile registers. It is trivial to extend though.

// (Re)Inizializza lo spazio RAM/ROM/FLASH/SD dei registri se non già inizializzato, ai valori di default
void registerSetup(const bool register_init);

/// Reads the specified register from the persistent storage into `inout_value`.
/// If the register does not exist or it cannot be automatically converted to the type of the provided argument,
/// the value will be stored in the persistent storage using @ref registerWrite(), overriding existing value.
/// The default will not be initialized if the argument is empty.
void registerRead(const char* const register_name, uavcan_register_Value_1_0* const inout_value);

/// Store the given register value into the persistent storage.
void registerWrite(const char* const register_name, const uavcan_register_Value_1_0* const value);

/// This function is mostly intended for implementing the standard RPC-service uavcan.register.List.
/// It returns the name of the register at the specified index (where the ordering is undefined but guaranteed
/// to be short-term stable), or empty name if the index is out of bounds.
uavcan_register_Name_1_0 registerGetNameByIndex(const uint16_t index);

/// Erase all registers such that the defaults are used at the next launch.
void registerDoFactoryReset(void);

/// Copy one value to the other if their types and dimensionality are the same or automatic conversion is possible.
/// If the destination is empty, it is simply replaced with the source (assignment always succeeds).
/// The return value is true if the assignment has been performed, false if it is not possible
/// (in the latter case the destination is NOT modified).
bool registerAssign(uavcan_register_Value_1_0* const dst, const uavcan_register_Value_1_0* const src);

#ifdef __cplusplus
}
#endif
