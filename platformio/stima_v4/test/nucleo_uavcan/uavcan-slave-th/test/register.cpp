/// This software is distributed under the terms of the MIT License.
/// Copyright (C) 2021 OpenCyphal <maintainers@opencyphal.org>
/// Author: Pavel Kirienko <pavel@opencyphal.org>
/// Revis.: Gasperini Moreno <m.gasperini@digiteco.it>

// ***************************************************************************************************
// Preselzione del metodo di accesso ai registri OPENCYPHAL ( SET del FLAG in platform.ini )
// ***************************************************************************************************
// USE_FS_SYSSTAT   -> Gestione file system tipo Linux
// USE_LIB_SD       -> Gestione SD nomi in formato 8.3 con indice (Utilizza meno ROM -7K e RAM -1K)
// USE_LIB_SDFAT    -> Gestione SD nomi lunghi (registerFile => registerName)
// USE_FLASH_SPI    -> Gestione con FLASH_SPI da implementare (FS / Accesso BYTE Flash)
// USE_STIMA4_E2P   -> Gestione con Eeprom in Stima V4 (Da implementare)
// ***************************************************************************************************

#include "register.hpp"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef USE_FS_SYSSTAT
// File system Classic (Linux)
#   include <sys/stat.h>
#   include <unistd.h>
#endif

#ifdef USE_LIB_SD
// Classic ARDUINO SD 8.3 small naming convention
#   include <SD.h>
#   include <SPI.h>
#endif

#ifdef USE_LIB_SDFAT
// Advanced ARDUINO SD long naming convention
#   include <SPI.h>
#   include "SdFat.h"
#endif

#define PASTE3_IMPL(x, y, z) x##y##z
#define PASTE3(x, y, z) PASTE3_IMPL(x, y, z)

#ifdef __cplusplus
extern "C" {
#endif

// ***************************************************************************************
//                       SD CARD SD ARDUINO STANDARD 8.3 NAMING FILE
// ***************************************************************************************
#ifdef USE_LIB_SD

// MAX XX - 0..99 Registri
#define REGISTER_F_SIZE 2
#define REGISTER_F_SFMT "%02d"

static const char registryDirName[] = "regs/";
static const char registryListName[] = "regs/list.txt";

// TODO: TEST su Yakut, 2) Corretto INIT (anche per sdFAT) 3) Portabilità su Flash/E2Prom con stesse chimate
// SD_CARD FAT STANDARD ARDUINO 8.3 NAMING UTILITY
void getRegisterIndexName(byte registerIndex, char* strData);
void getRegisterFileName(byte registerIndex, char* strData);
void registerInit(void);
bool registerListFind(const char* const register_name, byte* registerNumber);
void registerListAppend(const char* const register_name, byte registerIndex);
void registerPut(byte registerIndex, byte* registerValue, byte registerLength);
byte registerGet(byte registerIndex, byte* registerValue, byte registerLength);
void registerDel(byte registerIndex);
void registerSave(const char* const register_name, byte* registerValue, byte registerLength);

// ****************************** IMPLEMENTATION ******************************

// Setup della Libreria SD e SPIClass utilizzate (Return TRUE if OK)
bool setupSd(const uint32_t bMOSI, const uint32_t bMISO, const uint32_t bSCLK, const uint32_t sCS, const int speedMHZ) {
    // Setup SPI (PORT B) -> SD Also setup the Library SPI
    // SPIClass spiPort(bMOSI, bMISO, bSCLK);
    // Setup SD_Lib initialize the istance card
    if (!SD.begin(sCS)) {
        return false;
    }
    return true;
}

// Return FileName Register Index (Buffer memory extern)
void getRegisterIndexName(byte registerIndex, char* strData) {
    sprintf(strData, REGISTER_F_SFMT, registerIndex);
}

// Return FileName And Directory Register Index (Buffer memory extern)
void getRegisterFileName(byte registerIndex, char* strData) {
    char strIndexName[REGISTER_F_SIZE + 1];
    getRegisterIndexName(registerIndex, strIndexName);
    strcpy(strData, registryDirName);
    strcat(strData, strIndexName);
}

// Check if exist or create space register with init default value
void registerInit(void) {
    // Check Exixst Space DIR Register
    if (!SD.exists(registryDirName)) {
        // Create Registry DIR
        SD.mkdir(registryDirName);
    }
    // Check Exist List Register File
    if (!SD.exists(registryListName)) {
        // Populate INIT Default Value
        // Save Default List Register INIT Name and Value (Create File List) In order 0,1,2..N
        registerListAppend("uavcan.node.unique_id", 0);
        uavcan_register_Value_1_0 val = {0};
        uavcan_register_Value_1_0_select_natural16_(&val);
        val.natural16.value.count = 1;
        val.natural16.value.elements[0] = 55;
        registerWrite("uavcan.node.unique_id", &val);
    }
}

// IN  Register name  (UAVCAN REGISTER NAME)
// OUT Register Found (return Function bool)
// OUT registerNumber (registerId UAVCAN if found, LastValidRegister ID if not found
//       -> for append using LastValidRegister + 1 (NEW REGISTER)
bool registerListFind(const char* const register_name, byte* registerNumber) {
    // Search register_name into listRegister file (RETURN 0 = NO EXIST, 1..255 NUMBER REGISTER ID)
    byte registerFound = 0;
    byte registerPtr;
    // char register_read[uavcan_register_Name_1_0_name_ARRAY_CAPACITY_];
    char register_read[uavcan_register_Value_1_0_EXTENT_BYTES_];
    char strIndexName[REGISTER_F_SIZE + 1];
    File registryFileList;

    registerPtr = 0;
    // Scroll File register data while avaiable (exit found or eof)
    registryFileList = SD.open(registryListName, FILE_READ);
    while (registryFileList.available()) {
        // populate name register
        register_read[registerPtr] = registryFileList.read();
        // Separator register
        if (register_read[registerPtr++] == ',') {
            register_read[registerPtr - 1] = 0;
            // Reinit index
            registerPtr = 0;
            registryFileList.read(strIndexName, REGISTER_F_SIZE);
            strIndexName[2] = 0;
            *registerNumber = atoi(strIndexName);
            // CR+LF
            registryFileList.read(strIndexName, 2);
            // Verify Name Register
            registerFound = strcmp(register_name, register_read) == 0;
        }
        // Break for found
        if (registerFound) break;
    }
    registryFileList.close();
    // Find register ID
    return registerFound;
}

// Append register into register fileList (with new registerIndex value)
void registerListAppend(const char* const register_name, byte registerIndex) {
    char strIndexName[REGISTER_F_SIZE + 1];
    File registryFileList;

    registryFileList = SD.open(registryListName, FILE_WRITE);
    // Append NewLine -> 'name_register,index_register[cr_lf]'
    registryFileList.write(register_name);
    registryFileList.write(",");
    getRegisterIndexName(registerIndex, strIndexName);
    registryFileList.write(strIndexName, REGISTER_F_SIZE);
    registryFileList.write("\r\n");
    registryFileList.close();
}

// Put/rewrite data into register file, by registerIndex Value
void registerPut(byte registerIndex, byte* registerValue, byte registerLength) {
    char registryFileReg[sizeof(registryDirName) + REGISTER_F_SIZE + 1];
    File registryFileData;

    // Open registerFile data
    getRegisterFileName(registerIndex, registryFileReg);
    registryFileData = SD.open(registryFileReg, O_WRITE | O_CREAT);
    // Write serialized registerValue Data into registerFile
    registryFileData.write(registerValue, registerLength);
    registryFileData.close();
}

// Get/read data from register file, by registerIndex Value
// Return Byte Read from File and registerValue populated with DataRead
byte registerGet(byte registerIndex, byte* registerValue, byte registerLength) {
    char registryFileReg[sizeof(registryDirName) + REGISTER_F_SIZE];
    byte registerPtr = 0;
    File registryFileData;

    // Open registerFile data
    getRegisterFileName(registerIndex, registryFileReg);
    if (SD.exists(registryFileReg)) {
        registryFileData = SD.open(registryFileReg, FILE_READ);
        // Read serialized registerValue Data from registerFile
        while (registryFileData.available()) {
            registerValue[registerPtr++] = registryFileData.read();
        }
        registryFileData.close();
    }
    return (registerPtr);
}

// Delete register data file
void registerDel(byte registerIndex) {
    char registryFileReg[sizeof(registryDirName) + REGISTER_F_SIZE + 1];

    // Get registerFile Name and delete File
    getRegisterFileName(registerIndex, registryFileReg);
    SD.remove(registryFileReg);
}

// Save register into register fileList and create registerData Container with value
// If registerList not found registerList append new ListLine
void registerSave(const char* const register_name, byte* registerValue, byte registerLength) {
    byte registerIndex;
    // Check if register exist (and modify value)
    if (!registerListFind(register_name, &registerIndex)) {
        // Append next index register available
        registerListAppend(register_name, ++registerIndex);
    }
    // Put Data into Register
    registerPut(registerIndex, registerValue, registerLength);
}

// Read Register Wrapper for registerRead Uavcan
void registerRead(const char* const register_name, uavcan_register_Value_1_0* const inout_value) {
    assert(inout_value != NULL);
    byte registerIndex;
    bool init_required = !uavcan_register_Value_1_0_is_empty_(inout_value);
    bool registerFound = registerListFind(&register_name[0], &registerIndex);
    // Register is found
    if (registerFound) {
        uint8_t serialized[uavcan_register_Value_1_0_EXTENT_BYTES_] = {0};
        size_t sr_size = registerGet(registerIndex, &serialized[0], uavcan_register_Value_1_0_EXTENT_BYTES_);
        uavcan_register_Value_1_0 out = {0};
        const int8_t err = uavcan_register_Value_1_0_deserialize_(&out, serialized, &sr_size);
        if (err >= 0) {
            init_required = !registerAssign(inout_value, &out);
        }
    }
    if (init_required) {
        printf("Init register: %s\n", register_name);
        registerWrite(register_name, inout_value);
    }
}

// Write Register Wrapper for regiterWrite Uavcan
void registerWrite(const char* const register_name, const uavcan_register_Value_1_0* const value) {
    uint8_t serialized[uavcan_register_Value_1_0_EXTENT_BYTES_] = {0};
    size_t sr_size = uavcan_register_Value_1_0_EXTENT_BYTES_;
    const int8_t err = uavcan_register_Value_1_0_serialize_(value, serialized, &sr_size);
    if (err >= 0) {
        registerSave(register_name, &serialized[0], sr_size);
    }
}

// Get Register Name By Index Wrapper for registerGetNameByIndex Uavcan
uavcan_register_Name_1_0 registerGetNameByIndex(const uint16_t index) {
    byte registerNumber;
    bool registerFound = false;
    byte registerIndex = 0;
    char register_name[uavcan_register_Name_1_0_name_ARRAY_CAPACITY_];
    char register_file[REGISTER_F_SIZE];
    uavcan_register_Name_1_0 out = {0};
    uavcan_register_Name_1_0_initialize_(&out);

    File registryFileList = SD.open(registryListName, FILE_READ);
    if (registryFileList != NULL) {
        // The service definition requires that the ordering is consistent between calls.
        // We assume here that there will be no new registers added while the listing operation is in progress.
        // If this is not the case, you will need to implement additional logic to uphold the ordering consistency
        // guarantee, such as sorting registers by creation time or adding extra metadata.
        while (registryFileList.available()) {
            // TODO EOF CHECK ANCHE SOPRA + registerIndex++... ecc...
            register_name[registerIndex++] = registryFileList.read();
            // Separator register
            if (register_name[registerIndex] == ',') {
                register_name[registerIndex] = 0;
                // Reinit index (next register index)
                registerIndex = 0;
                registryFileList.read(register_file, REGISTER_F_SIZE);
                registerNumber = atoi(register_file);
                // CR,LF
                registryFileList.read(register_file, 2);
                // Test Name Register
                registerFound = (index == registerNumber);
            }
            if (registerFound) break;
        }
        // Se registerIdx is Found, retrieve Name
        if (registerFound) {
            out.name.count = nunavutChooseMin(strlen(register_name), uavcan_register_Name_1_0_name_ARRAY_CAPACITY_);
            memcpy(out.name.elements, register_name, out.name.count);
        }
        registryFileList.close();
    }
    return out;
}

// Clear Register Wrapper for registerDoFactoryReset Uavcan
void registerDoFactoryReset(void) {
    byte registerNumber;
    byte registerIndex = 0;
    char register_name[uavcan_register_Name_1_0_name_ARRAY_CAPACITY_];
    char register_file[REGISTER_F_SIZE];
    uavcan_register_Name_1_0 out = {0};
    uavcan_register_Name_1_0_initialize_(&out);

    File registryFileList = SD.open(registryListName, FILE_READ);
    if (registryFileList != NULL) {
        // The service definition requires that the ordering is consistent between calls.
        // We assume here that there will be no new registers added while the listing operation is in progress.
        // If this is not the case, you will need to implement additional logic to uphold the ordering consistency
        // guarantee, such as sorting registers by creation time or adding extra metadata.
        while (registryFileList.available()) {
            // TODO EOF CHECK ANCHE SOPRA + registerIndex++... ecc...
            register_name[registerIndex] = registryFileList.read();
            // Separator register
            if (register_name[registerIndex++] == ',') {
                register_name[registerIndex] = 0;
                // Reinit index (next register index)
                registerIndex = 0;
                registryFileList.read(register_file, REGISTER_F_SIZE);
                registerNumber = atoi(register_file);
                // CR,LF
                registryFileList.read(register_file, 2);
                // Delete Register
                registerDel(registerNumber);
            }
        }
        registryFileList.close();
        // Destroy List
        SD.remove(registryListName);
    }
}
#endif

// ***************************************************************************************
//               SD CARD SDFAT ARDUINO ENANCHED LONG NAMING CONVENTION FILE
// ***************************************************************************************
#ifdef USE_LIB_SDFAT
#define errorExit(msg) errorHalt(F(msg))
#define initError(msg) initErrorHalt(F(msg))
static const char RegistryDirName[] = "registry";

// Variabili SD FAT -> ToDo: Istanza C++
SdFat sdLoc;  // Sd Istanza

// Setup della Lib SD Utilizzata e SPIClass (Return TRUE if OK)
bool setupSd(const uint32_t bMOSI, const uint32_t bMISO, const uint32_t bSCLK, const uint32_t sCS, const int speedMHZ) {
    // Setup SPI (PORT B/2)
    SPIClass spiPort(bMOSI, bMISO, bSCLK);
    // Setup SD_Lib initialize the istance card
    if (!sdLoc.begin(sCS, SD_SCK_MHZ(speedMHZ))) {
        sdLoc.initError("sdLoc.remove():");
        return false;
    }
    return true;
}

static inline void registerOpen(const char* const register_name, const bool write, SdFile& registerFile) {
    char file_path[uavcan_register_Name_1_0_name_ARRAY_CAPACITY_ + sizeof(RegistryDirName) + 2] = {0};
    (void)snprintf(&file_path[0], sizeof(file_path), "%s/%s", RegistryDirName, register_name);
    if (write) {
        sdLoc.mkdir(RegistryDirName);
    }
    registerFile.open(&file_path[0], write ? O_RDWR | O_CREAT | O_TRUNC : O_READ);
}

void registerRead(const char* const register_name, uavcan_register_Value_1_0* const inout_value) {
    assert(inout_value != NULL);
    bool init_required = !uavcan_register_Value_1_0_is_empty_(inout_value);
    SdFile fp;
    registerOpen(&register_name[0], false, fp);
    if (fp != NULL) {
        uint8_t serialized[uavcan_register_Value_1_0_EXTENT_BYTES_] = {0};
        size_t sr_size = fp.read(&serialized[0], uavcan_register_Value_1_0_EXTENT_BYTES_);
        fp.close();
        uavcan_register_Value_1_0 out = {0};
        const int8_t err = uavcan_register_Value_1_0_deserialize_(&out, serialized, &sr_size);
        if (err >= 0) {
            init_required = !registerAssign(inout_value, &out);
        }
    }
    if (init_required) {
        printf("Init register: %s\n", register_name);
        registerWrite(register_name, inout_value);
    }
}

void registerWrite(const char* const register_name, const uavcan_register_Value_1_0* const value) {
    uint8_t serialized[uavcan_register_Value_1_0_EXTENT_BYTES_] = {0};
    size_t sr_size = uavcan_register_Value_1_0_EXTENT_BYTES_;
    const int8_t err = uavcan_register_Value_1_0_serialize_(value, serialized, &sr_size);
    if (err >= 0) {
        SdFile fp;
        registerOpen(&register_name[0], true, fp);
        if (fp != NULL) {
            fp.write(&serialized[0], sr_size);
            fp.close();
        }
    }
}

uavcan_register_Name_1_0 registerGetNameByIndex(const uint16_t index) {
    uavcan_register_Name_1_0 out = {0};
    uavcan_register_Name_1_0_initialize_(&out);
    char register_name[uavcan_register_Name_1_0_name_ARRAY_CAPACITY_] = {0};
    SdFile dirReg;
    // Directory Reg is Found
    if (dirReg.open(RegistryDirName, O_READ)) {
        SdFile fileReg;
        uint16_t ii = 0;
        bool isFound = false;
        // Scroll file into Directory Register
        while (fileReg.openNext(&dirReg, O_READ)) {
            if (!fileReg.isDir()) {
                if (ii >= index) {
                    isFound = true;
                    break;
                }
                ++ii;
            }
            fileReg.close();
        }
        // Registro Indice valido corrispondente trovato
        if (isFound) {
            // Get FullName (Register Name)
            fileReg.getName(register_name, sizeof(register_name));
            fileReg.close();
            out.name.count = nunavutChooseMin(strlen(register_name), uavcan_register_Name_1_0_name_ARRAY_CAPACITY_);
            memcpy(out.name.elements, register_name, out.name.count);
        } else {
            fileReg.close();
        }
    }
    return out;
}

void registerDoFactoryReset(void) {
    char file_path[uavcan_register_Name_1_0_name_ARRAY_CAPACITY_ + sizeof(RegistryDirName) + 2] = {0};
    SdFile dirReg;
    // Directory Reg is Found
    if (dirReg.open(RegistryDirName, O_READ)) {
        SdFile fileReg;
        // Scroll file into Directory Register
        while (fileReg.openNext(&dirReg, O_READ)) {
            if (!fileReg.isDir()) {
                // Get FullName and Path (Register)
                strcpy(file_path, RegistryDirName);
                fileReg.getName(&file_path[strlen(file_path) + 1], sizeof(file_path));
                file_path[strlen(file_path)] = '/';
                fileReg.close();
                // Remove file
                sdLoc.remove(file_path);
            } else {
                fileReg.close();
            }
        }
        // Remove Dir Register
        sdLoc.rmdir(RegistryDirName);
    }
}

#endif
// ***************************************************************************************
//                 FILE SYTEM STANDARD CLASSIC (Linux) ORIGINAL CYPAL
// ***************************************************************************************
#ifdef USE_FS_SYSSTAT

static const char RegistryDirName[] = "registry";

static inline FILE* registerOpen(const char* const register_name, const bool write) {
    // An actual implementation on an embedded system may need to perform atomic file transactions via rename().
    char file_path[uavcan_register_Name_1_0_name_ARRAY_CAPACITY_ + sizeof(RegistryDirName) + 2] = {0};
    (void)snprintf(&file_path[0], sizeof(file_path), "%s/%s", RegistryDirName, register_name);
    if (write) {
        (void)mkdir(RegistryDirName, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }
    return fopen(&file_path[0], write ? "wb" : "rb");
}

void registerRead(const char* const register_name, uavcan_register_Value_1_0* const inout_value) {
    assert(inout_value != NULL);
    bool init_required = !uavcan_register_Value_1_0_is_empty_(inout_value);
    FILE* const fp = registerOpen(&register_name[0], false);
    if (fp != NULL) {
        uint8_t serialized[uavcan_register_Value_1_0_EXTENT_BYTES_] = {0};
        size_t sr_size = fread(&serialized[0], 1U, uavcan_register_Value_1_0_EXTENT_BYTES_, fp);
        (void)fclose(fp);
        uavcan_register_Value_1_0 out = {0};
        const int8_t err = uavcan_register_Value_1_0_deserialize_(&out, serialized, &sr_size);
        if (err >= 0) {
            init_required = !registerAssign(inout_value, &out);
        }
    }
    if (init_required) {
        printf("Init register: %s\n", register_name);
        registerWrite(register_name, inout_value);
    }
}

void registerWrite(const char* const register_name, const uavcan_register_Value_1_0* const value) {
    uint8_t serialized[uavcan_register_Value_1_0_EXTENT_BYTES_] = {0};
    size_t sr_size = uavcan_register_Value_1_0_EXTENT_BYTES_;
    const int8_t err = uavcan_register_Value_1_0_serialize_(value, serialized, &sr_size);
    if (err >= 0) {
        FILE* const fp = registerOpen(&register_name[0], true);
        if (fp != NULL) {
            (void)fwrite(&serialized[0], 1U, sr_size, fp);
            (void)fclose(fp);
        }
    }
}

uavcan_register_Name_1_0 registerGetNameByIndex(const uint16_t index) {
    uavcan_register_Name_1_0 out = {0};
    uavcan_register_Name_1_0_initialize_(&out);

    DIR* const dp = opendir(RegistryDirName);
    if (dp != NULL) {
        // The service definition requires that the ordering is consistent between calls.
        // We assume here that there will be no new registers added while the listing operation is in progress.
        // If this is not the case, you will need to implement additional logic to uphold the ordering consistency
        // guarantee, such as sorting registers by creation time or adding extra metadata.
        struct dirent* ep = readdir(dp);
        uint16_t ii = 0;
        while (ep != NULL) {
            if (ep->d_type == DT_REG) {
                if (ii >= index) {
                    break;
                }
                ++ii;
            }
            ep = readdir(dp);
        }
        if (ep != NULL) {
            out.name.count = nunavutChooseMin(strlen(ep->d_name), uavcan_register_Name_1_0_name_ARRAY_CAPACITY_);
            memcpy(out.name.elements, ep->d_name, out.name.count);
        }
        (void)closedir(dp);
    }
    return out;
}

void registerDoFactoryReset(void) {
    DIR* const dp = opendir(RegistryDirName);
    if (dp != NULL) {
        struct dirent* ep = readdir(dp);
        while (ep != NULL) {
            if (ep->d_type == DT_REG) {
                char file_path[uavcan_register_Name_1_0_name_ARRAY_CAPACITY_ + sizeof(RegistryDirName) + 2] = {0};
                (void)snprintf(&file_path[0], sizeof(file_path), "%s/%s", RegistryDirName, ep->d_name);
                (void)unlink(&file_path[0]);
            }
            ep = readdir(dp);
        }
        (void)closedir(dp);
    }
}
#endif

// ***************************************************************************************
//                 UAVCAN RegisterAssign Standard (Comune a tutti i sistemi)
// ***************************************************************************************
bool registerAssign(uavcan_register_Value_1_0* const dst, const uavcan_register_Value_1_0* const src) {
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

#ifdef __cplusplus
}
#endif
