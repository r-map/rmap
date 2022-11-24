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
// USE_STIMA4_E2P   -> Gestione con memoria (RAM) e Wrapper per tipologia (E2/FLASH)
// ***************************************************************************************************

#include "register.hpp"
#include "canard_config.hpp"

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

#ifdef USE_STIMA4_E2P
// E2PROM/FLASH/RAM STIMA V4
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
        uavcan_register_Value_1_0 val = {0};
        // Save Default List Register INIT Name and Value (Create File List) In order 0,1,2..N
        registerListAppend("uavcan.can.bitrate", 0);
        uavcan_register_Value_1_0_select_natural16_(&val);
        val.natural16.value.count       = 1;
        val.natural16.value.elements[0] = CAN_MTU_BASE; // CAN_CLASSIC MTU 8
        // We also need the bitrate configuration register. In this demo we can't really use it but an embedded application
        // should define "uavcan.can.bitrate" of type natural32[2]; the second value is 0/ignored if CAN FD not supported.
        // TODO: Default a CAN_BIT_RATE, se CAN_BIT_RATE <> readRegister setup bxCAN con nuovo RATE hot reload
        uavcan_register_Value_1_0_select_natural32_(&val);
        val.natural32.value.count       = 2;
        val.natural32.value.elements[0] = CAN_BIT_RATE;
        val.natural32.value.elements[1] = 0ul;          // Ignored for CANARD_MTU_CAN_CLASSIC
        registerWrite("uavcan.can.bitrate", &val);

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
    LOCAL_ASSERT(inout_value != NULL);
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
            if (register_name[registerIndex-1] == ',') {
                register_name[registerIndex-1] = 0;
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
static const char FirmwareDirName[] = "firmware";
static const char FileDirName[] = "file";
#define fileArchiveDirLen() (sizeof(FirmwareDirName) > sizeof(FileDirName) ? sizeof(FirmwareDirName) : sizeof(FileDirName))

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

// Scrive dati in append per scrittura sequenziale file data remoto
void putDataFile(const char* const file_name, const bool is_firmware, const bool rewrite, void* buf, size_t count)
{
    SdFile fw;
    char file_path[FILE_NAME_SIZE_MAX + fileArchiveDirLen()] = {0};
    if (is_firmware) {
        (void)snprintf(&file_path[0], sizeof(file_path), "%s/%s", FirmwareDirName, file_name);
        sdLoc.mkdir(FirmwareDirName);
    } else {
        (void)snprintf(&file_path[0], sizeof(file_path), "%s/%s", FileDirName, file_name);
        sdLoc.mkdir(FileDirName);
    }
    fw.open(&file_path[0], rewrite ? O_RDWR | O_CREAT | O_TRUNC : O_RDWR | O_APPEND);
    if (fw != NULL) {
        fw.write(buf, count);
    }
    fw.close();
}

// legge dati in append per trasmissione sequenziale file firmware
bool getDataFile(const char* const file_name, const bool is_firmware, uint64_t position, void* buf, size_t *count)
{
    SdFile fw;

    char file_path[FILE_NAME_SIZE_MAX + fileArchiveDirLen()] = {0};
    if (is_firmware) {
        (void)snprintf(&file_path[0], sizeof(file_path), "%s/%s", FirmwareDirName, file_name);
        sdLoc.mkdir(FirmwareDirName);
    } else {
        (void)snprintf(&file_path[0], sizeof(file_path), "%s/%s", FileDirName, file_name);
        sdLoc.mkdir(FileDirName);
    }
    fw.open(&file_path[0], O_RDONLY);
    if (fw != NULL) {
        if(fw.seek(position)) {
            int retVal = fw.read(buf, *count);            
            // RetVal OK >=0
            // 0 = OK Senza Bytes Read...
            if(retVal>=0) {
                *count = (size_t)retVal;
                return true;
            }
        }
    }
    // Error file read or open or FS...
    return false;
}

// Restituisce le info per file firmware e controlli vari
uint64_t getDataFileInfo(const char* const file_name, const bool is_firmware)
{
    SdFile fw;
    uint64_t lof = 0;

    char file_path[FILE_NAME_SIZE_MAX + fileArchiveDirLen()] = {0};
    if (is_firmware) {
        (void)snprintf(&file_path[0], sizeof(file_path), "%s/%s", FirmwareDirName, file_name);
    } else {
        (void)snprintf(&file_path[0], sizeof(file_path), "%s/%s", FileDirName, file_name);
    }
    fw.open(&file_path[0], O_RDONLY);
    if (fw != NULL) {
        lof = fw.fileSize();
        fw.close();
    }
    return lof;
}

// Ritorna vero se il file firmware esiste ed è coerente (Controllo coerenza, esiste..CRC..altro...)
bool ccFirwmareFile(const char* const file_name)
{
    char register_name[60];

    char file_path[FILE_NAME_SIZE_MAX + sizeof(FirmwareDirName)] = {0};
    (void)snprintf(&file_path[0], sizeof(file_path), "%s/%s", FirmwareDirName, file_name);
    return sdLoc.exists(&file_path[0]);
}

// Check if exist or create space register with init default value
void registerSetup(const bool register_init) {
    // Check Exixst Space DIR Register
    if (!sdLoc.exists(RegistryDirName)) {
        // Create Registry DIR
        sdLoc.mkdir(RegistryDirName);
    }
    // Open Register in Write se non inizializzati correttamente...
    // Populate INIT Default Value
    uavcan_register_Value_1_0 val = {0};
    uavcan_register_Value_1_0_select_natural16_(&val);
    val.natural16.value.count       = 1;
    val.natural16.value.elements[0] = CAN_MTU_BASE; // CAN_CLASSIC MTU 8
    if(register_init) {
        registerWrite("uavcan.can.mtu", &val);
    } else {
        registerRead("uavcan.can.mtu", &val);
    }
    // We also need the bitrate configuration register. In this demo we can't really use it but an embedded application
    // should define "uavcan.can.bitrate" of type natural32[2]; the second value is 0/ignored if CAN FD not supported.
    // TODO: Default a CAN_BIT_RATE, se CAN_BIT_RATE <> readRegister setup bxCAN con nuovo RATE hot reload
    uavcan_register_Value_1_0_select_natural32_(&val);
    val.natural32.value.count       = 2;
    val.natural32.value.elements[0] = CAN_BIT_RATE;
    val.natural32.value.elements[1] = 0ul;          // Ignored for CANARD_MTU_CAN_CLASSIC
    if(register_init) {
        registerWrite("uavcan.can.bitrate", &val);
    } else {
        registerRead("uavcan.can.bitrate", &val);
    }

    // N.B. Inserire quà la personalizzazione dei registri in SETUP Fisso o di compilazione di modulo
    if(register_init) {
        uavcan_register_Value_1_0_select_natural16_(&val);
        val.natural16.value.count       = 1;
        val.natural16.value.elements[0] = 100;
        registerWrite("uavcan.srv.TH.service_data_and_metadata.id", &val);
    }
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
    LOCAL_ASSERT(inout_value != NULL);
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
    }
}

#endif

// ***************************************************************************************
//                     E2PROM STIMAV4 STM32 ARDUINO REGISTER ACCESS
// ***************************************************************************************
#ifdef USE_STIMA4_E2P

// Start Address BASE UAVCAN/CYPAL Register
#define MEM_UAVCAN_LEN                      8192u
#define MEM_UAVCAN_ADDR_START               100u
#define MEM_UAVCAN_MAX_REG                  50u
#define MEM_UAVCAN_LEN_SIZE_T_REG           1u
#define MEM_UAVCAN_LEN_INTEST_REG           60u
#define MEM_UAVCAN_LEN_VALUE_REG            66u
#define MEM_UAVCAN_POS_LEN_NAME             0u
#define MEM_UAVCAN_POS_STR_NAME             MEM_UAVCAN_POS_LEN_NAME + MEM_UAVCAN_LEN_SIZE_T_REG
#define MEM_UAVCAN_POS_LEN_DATA             MEM_UAVCAN_POS_STR_NAME + MEM_UAVCAN_LEN_INTEST_REG
#define MEM_UAVCAN_POS_VALUE_DATA           MEM_UAVCAN_POS_LEN_DATA + MEM_UAVCAN_LEN_SIZE_T_REG
#define MEM_UAVCAN_LEN_NAME_REG             (MEM_UAVCAN_LEN_SIZE_T_REG + MEM_UAVCAN_LEN_INTEST_REG)
#define MEM_UAVCAN_LEN_DATA_REG             (MEM_UAVCAN_LEN_SIZE_T_REG + MEM_UAVCAN_LEN_VALUE_REG)
#define MEM_UAVCAN_LEN_REG                  (MEM_UAVCAN_LEN_NAME_REG + MEM_UAVCAN_LEN_DATA_REG)
#define MEM_UAVCAN_START_AREA_REG           (MEM_UAVCAN_ADDR_START + MEM_UAVCAN_MAX_REG)
#define MEM_UAVCAN_REG_UNDEF                0xFF
#define MEM_UAVCAN_GET_ADDR_FLAG()          (MEM_UAVCAN_ADDR_START)
#define MEM_UAVCAN_GET_ADDR_FLAG_REG(X)     (MEM_UAVCAN_ADDR_START + X)
#define MEM_UAVCAN_GET_ADDR_NAME_LEN(X)     (MEM_UAVCAN_START_AREA_REG + (MEM_UAVCAN_LEN_REG * X))
#define MEM_UAVCAN_GET_ADDR_NAME(X)         (MEM_UAVCAN_START_AREA_REG + (MEM_UAVCAN_LEN_REG * X) + MEM_UAVCAN_LEN_SIZE_T_REG)
#define MEM_UAVCAN_GET_ADDR_VALUE_LEN(X)    (MEM_UAVCAN_START_AREA_REG + (MEM_UAVCAN_LEN_REG * X) + MEM_UAVCAN_LEN_NAME_REG)
#define MEM_UAVCAN_GET_ADDR_VALUE(X)        (MEM_UAVCAN_START_AREA_REG + (MEM_UAVCAN_LEN_REG * X) + MEM_UAVCAN_LEN_NAME_REG + MEM_UAVCAN_LEN_SIZE_T_REG)
#define MEM_UAVCAN_GET_ADDR_BASE_REG(X)     MEM_UAVCAN_GET_ADDR_NAME_LEN(X)
// Start Address Eeprom Application Free usage
#define MEM_UAVCAN_ADDR_END                 (MEM_UAVCAN_START_AREA_REG + (MEM_UAVCAN_LEN_REG * MEM_UAVCAN_MAX_REG))

// E2Prom/Flash (Size Simulator) -> RAM Wrapper
uint8_t memRegUAVCAN[MEM_UAVCAN_LEN] = {0};

#ifdef TEST_PERFORM_E2
uint32_t proc_count[20] = {0};
uint32_t proc_mean[20] = {0};
uint32_t proc_max[20] = {0};
uint32_t proc_start[20] = {0};
uint32_t proc_all;

void perform_init(void) {
    proc_all = micros();
    for(uint8_t id=0; id<20; id++) {
        proc_count[id] = 0;
        proc_mean[id] = 0;
        proc_max[id] = 0;
    }
}
void perform_start(uint8_t id) {
    proc_start[id] = micros();
}

void perform_add(uint8_t id) {
    proc_count[id]++;
    proc_mean[id]+=proc_start[id];
    if(proc_start[id]>proc_max[id]) proc_max[id] = proc_start[id];
}

void perform_write(void) {
    Serial.print("Time complete: ");
    Serial.println(micros()-proc_all);
    for (uint8_t i=0;i<20;i++) {
        Serial.print(i);
        Serial.print(", ");
        Serial.print(proc_count[i]);
        Serial.print(", ");
        Serial.print(proc_max[i]);
        Serial.print(", ");
        float fVal= proc_mean[i]/proc_count[i];
        Serial.println(fVal);
        delay(5);
    }
}
#else
    #define perform_init()      void(0)
    #define perform_start(x)    void(0)
    #define perform_add(x)      void(0)
    #define perform_write()     void(0)
#endif

/// @brief Wrapper memory_write_block
/// @param address Address to write
/// @param data data to write
/// @param len packet len
void memory_write_block(uint16_t address, uint8_t *data, uint8_t len) {
   perform_start(1);
   memcpy(&memRegUAVCAN[address], data, len);   
   perform_add(1);
}

/// @brief Wrapper memory_read_block
/// @param address Address to read
/// @param data data readed
/// @param len packet len request
void memory_read_block(uint16_t address, uint8_t *data, uint8_t len) {
    perform_start(2);
    memcpy(data, &memRegUAVCAN[address], len);
    perform_add(2);
}

/// @brief Wrapper memory_write_byte
/// @param address Address to write
/// @param data single byte data to write
void memory_write_byte(uint16_t address, uint8_t data) {
    perform_start(3);
    memRegUAVCAN[address] = data;
    perform_add(3);
}

/// @brief Wrapper memory_read_byte
/// @param address Address to read
/// @param data single byte readed
void memory_read_byte(uint16_t address, uint8_t *data) {
    perform_start(4);
    *data = memRegUAVCAN[address];
    perform_add(4);
}

/// @brief Inizializza l'area memory (indice) dedicata a REGISTER
/// @param  None
void eeprom_register_factory(void) {
    perform_start(5);
    uint8_t register_index[MEM_UAVCAN_MAX_REG];
    // Scrivo in un unica tornata
    memset(register_index, MEM_UAVCAN_REG_UNDEF, MEM_UAVCAN_MAX_REG);
    memory_write_block(MEM_UAVCAN_GET_ADDR_FLAG(), register_index, MEM_UAVCAN_MAX_REG);
    perform_add(5);
}

/// @brief Inizializza/Elimina un registro CYPAL/STIMAV4 dalla memoria
/// @param reg_numb numero di registro da eliminare
void eeprom_register_clear(uint8_t reg_numb) {
    perform_start(6);
    // Controllo area register
    if(reg_numb<MEM_UAVCAN_MAX_REG)
        memory_write_byte(MEM_UAVCAN_GET_ADDR_FLAG_REG(reg_numb), MEM_UAVCAN_REG_UNDEF);
    perform_add(6);
}

/// @brief Legge un registro CYPAL/STIMAV4 dalla memoria (per indice)
///        (fast=senza controlli validità) la procedura chiamante si occupa dei limiti
/// @param reg_numb (IN) Numero di regsitro da leggere
/// @param reg_name (OUT) Nome del resistro UAVCAN/CYPAL
/// @param data (OUT) Valore del registro
/// @return lunghezza del registro
size_t eeprom_register_get_fast(uint8_t reg_numb, uint8_t *reg_name, uint8_t *reg_value) {
    perform_start(7);
    uint8_t read_block[MEM_UAVCAN_LEN_REG];
    uint8_t reg_valid = MEM_UAVCAN_REG_UNDEF;
    uint8_t data_len;
    uint8_t name_len;
    // Leggo il blocco in un unica read
    memory_read_block(MEM_UAVCAN_GET_ADDR_BASE_REG(reg_numb), read_block, MEM_UAVCAN_LEN_REG);
    // Ritorno i campi name e value corretti
    name_len = read_block[MEM_UAVCAN_POS_LEN_NAME];
    data_len = read_block[MEM_UAVCAN_POS_LEN_DATA];
    memcpy(reg_name, &read_block[MEM_UAVCAN_POS_STR_NAME], name_len);
    memcpy(reg_value, &read_block[MEM_UAVCAN_POS_VALUE_DATA], data_len);
    perform_add(7);
    return (size_t)data_len;
}

/// @brief Legge un registro CYPAL/STIMAV4 dalla memoria (per indice)
///        (fast=senza controlli validità) la procedura chiamante si occupa dei limiti
/// @param reg_numb (IN) Numero di regsitro da leggere
/// @return lunghezza del registro indirizzato
size_t eeprom_register_get_len_intest_fast(uint8_t reg_numb) {
    perform_start(8);
    uint8_t name_len;
    // Registro eeprom valido, ritorno i campi name e value
    memory_read_byte(MEM_UAVCAN_GET_ADDR_NAME_LEN(reg_numb), &name_len);
    perform_add(8);
    return (size_t)name_len;
}

/// @brief Legge un registro CYPAL/STIMAV4 dalla memoria (per indice)
///        (fast=senza controlli validità) la procedura chiamante si occupa dei limiti
/// @param reg_numb (IN) Numero di regsitro da leggere
/// @param reg_name (OUT) Nome del resistro UAVCAN/CYPAL
/// @param name_len (IN) Lunghezza del messaggio di intestazione (preventivamente letto)
/// @return None
void eeprom_register_get_intest_fast(uint8_t reg_numb, uint8_t *reg_name, uint8_t name_len) {
    perform_start(9);
    // Registro eeprom valido, ritorno i campi name e value
    memory_read_block(MEM_UAVCAN_GET_ADDR_NAME(reg_numb), reg_name, name_len);
    perform_add(9);
}

/// @brief Legge un registro CYPAL/STIMAV4 dalla EEprom (per indice)
/// @param reg_numb (IN) Numero di regsitro da leggere
/// @param reg_name (OUT) Nome del resistro UAVCAN/CYPAL
/// @return true se registro trovato
bool eeprom_register_get_name_from_index(uint8_t reg_numb, uint8_t *reg_name) {
    perform_start(10);
    uint8_t reg_valid = MEM_UAVCAN_REG_UNDEF;
    uint8_t len_name;
    // Controllo area register
    if(reg_numb>=MEM_UAVCAN_MAX_REG) {
        perform_add(10);
        return false;
    }
    // Leggo l'indice se impostato (registro valido)
    memory_read_byte(MEM_UAVCAN_GET_ADDR_FLAG_REG(reg_numb), &reg_valid);
    if(reg_valid == reg_numb) {
        // Registro eeprom valido, ritorno i campi name e value
        memory_read_byte(MEM_UAVCAN_GET_ADDR_NAME_LEN(reg_numb), &len_name);
        memory_read_block(MEM_UAVCAN_GET_ADDR_NAME(reg_numb), reg_name, len_name);
        perform_add(10);
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
size_t eeprom_register_get_from_name(uint8_t const *reg_name, uint8_t *reg_numb, uint8_t *data) {    
    perform_start(11);
    uint8_t _reg_name[MEM_UAVCAN_LEN_NAME_REG];
    uint8_t _reg_data[MEM_UAVCAN_LEN_VALUE_REG];
    uint8_t _len_name = strlen((char*)reg_name);
    size_t _len_data = 0;
    uint8_t register_index[MEM_UAVCAN_MAX_REG];
    // Leggo l'intero status register per velocizzare l'indice di ricerca
    // Controllo preventivo dell'array di validità dei registri indicizzata
    memory_read_block(MEM_UAVCAN_GET_ADDR_FLAG(), register_index, MEM_UAVCAN_MAX_REG);
    // Controllo area register (Search for name)
    for(uint8_t reg_index = 0; reg_index<MEM_UAVCAN_MAX_REG; reg_index++) {
        // Test Data (Register Valid && Len Intest == for Rapid Check)
        if(register_index[reg_index]!=MEM_UAVCAN_REG_UNDEF) {
            // eeprom_register_get_len_intest_fast -> Rapido senza controllo validità REG
            // La sua chiamata prevede un controllo preventivo della validità del REG
            if (eeprom_register_get_len_intest_fast(reg_index) == _len_name) {
                // Retrieve all info register value
                _len_data = eeprom_register_get_fast(reg_index, _reg_name, _reg_data);
                // Compare value name
                if(memcmp(reg_name, _reg_name, _len_name) == 0) {
                    // Data is found
                    memcpy(data, _reg_data, _len_data);
                    *reg_numb = reg_index;
                    perform_add(11);
                    return _len_data;
                }
            }
        }
    }
    // End Of Size ROM (Register Not Found)
    perform_add(11);
    return 0;
}

/// @brief Legge un indiced di registro CYPAL/STIMAV4 dalla memoria (per Nome)
/// @param reg_name (IN) Nome del regsitro da leggere
/// @return indice del registro se esiste altrimenti MEM_UAVCAN_REG_UNDEF
uint8_t eeprom_register_get_index_from_name(uint8_t *reg_name) {    
    perform_start(12);
    uint8_t _reg_name[MEM_UAVCAN_LEN_NAME_REG];
    uint8_t _reg_data[MEM_UAVCAN_LEN_VALUE_REG];
    uint8_t _len_name = strlen((char*)reg_name);
    uint8_t register_index[MEM_UAVCAN_MAX_REG];
    // Leggo l'intero status register per velocizzare l'indice di ricerca
    // Controllo preventivo dell'array di validità dei registri indicizzata
    memory_read_block(MEM_UAVCAN_GET_ADDR_FLAG(), register_index, MEM_UAVCAN_MAX_REG);
    // Controllo area register (Search for name)
    for(uint8_t reg_index = 0; reg_index<MEM_UAVCAN_MAX_REG; reg_index++) {
        // Test Data (Register Valid && Len Intest == for Rapid Check)
        if(register_index[reg_index]!=MEM_UAVCAN_REG_UNDEF) {
            // eeprom_register_get_len_intest_fast -> Rapido senza controllo validità REG
            // La sua chiamata prevede un controllo preventivo della validità del REG
            if (eeprom_register_get_len_intest_fast(reg_index) == _len_name) {
                // Test Data (Register Valid)
                eeprom_register_get_intest_fast(reg_index, _reg_name, _len_name);
                // Compare value name
                if(memcmp(reg_name, _reg_name, _len_name) == 0) {
                    // Data is found
                    perform_add(12);
                    return reg_index;
                }
            }
        }
    }
    // End Of Size ROM (Register Not Found)
    perform_add(12);
    return MEM_UAVCAN_REG_UNDEF;
}

/// @brief Scrive/edita un registro CYPAL/STIMAV4 sulla memoria
/// @param reg_numb Numero di regsitro da impostare
/// @param reg_name Nome del resistro UAVCAN/CYPAL
/// @param data Valore del registro
void eeprom_register_set(uint8_t reg_numb, uint8_t *reg_name, uint8_t *data, size_t len_data) {
    perform_start(13);
    uint8_t reg_valid;
    uint8_t name_len = strlen((char*)reg_name);
    uint8_t write_block[MEM_UAVCAN_LEN_REG] = {0};
    // Controllo area register
    if(reg_numb>=MEM_UAVCAN_MAX_REG) {
        perform_add(13);
        return;
    }
    // Leggo l'indice se impostato (registro valido)
    memory_read_byte(MEM_UAVCAN_GET_ADDR_FLAG_REG(reg_numb), &reg_valid);
    if(reg_valid != reg_numb) {
        // Imposto il Numero sul BYTE relativo (Registro inizializzato)
        memory_write_byte(MEM_UAVCAN_GET_ADDR_FLAG_REG(reg_numb), reg_numb);
    }
    // Registro eeprom valido, imposto i campi name e value
    write_block[MEM_UAVCAN_POS_LEN_NAME] = name_len;
    write_block[MEM_UAVCAN_POS_LEN_DATA] = len_data;
    memcpy(write_block + MEM_UAVCAN_POS_STR_NAME, reg_name, name_len);
    memcpy(write_block + MEM_UAVCAN_POS_VALUE_DATA, data, len_data);
    // Perform in unica scrittura
    memory_write_block(MEM_UAVCAN_GET_ADDR_BASE_REG(reg_numb), write_block, MEM_UAVCAN_LEN_REG);
    perform_add(13);
}

/// @brief Ritorna il prossimo indice (se esiste) valido nella sezione memoria Cypal dedicata
/// @param start_register indirizzo di partenza nel campo di validità [MEM_UAVCAN_MAX_REG]
/// @return next_register address nella sezione EEprom Cypal Dedicata
uint8_t eeprom_register_get_next_id(uint8_t start_register) {
    perform_start(14);
    uint8_t register_index[MEM_UAVCAN_MAX_REG];
    // Nessun registro oltre soglia
    if(start_register>=MEM_UAVCAN_MAX_REG) {
        return MEM_UAVCAN_REG_UNDEF;
    }
    // Continuo alla ricerca del prossimo register se esiste
    memory_read_block(MEM_UAVCAN_GET_ADDR_FLAG(), register_index, MEM_UAVCAN_MAX_REG);
    for(; start_register<MEM_UAVCAN_MAX_REG; start_register++) {
        if(register_index[start_register] == start_register) {
            perform_add(14);
            return start_register;
        }
    }
    perform_add(14);
    return false;
}

/// @brief Aggiunge un registro alla configurazione CYPAL/STIMAV4
/// @param reg_name Nome del resistro UAVCAN/CYPAL
/// @param data Valore del registro
/// @return indice dell'elemento inserito nello stazio EEprom Cypal Dedicato [FAIL = MEM_UAVCAN_REG_UNDEF]
uint8_t eeprom_register_add(uint8_t *reg_name, uint8_t *data, size_t data_len) {
    perform_start(15);
    uint8_t register_index[MEM_UAVCAN_MAX_REG];
    // Vado alla ricerca del prossimo register se esiste (parto dal primo...)
    memory_read_block(MEM_UAVCAN_GET_ADDR_FLAG(), register_index, MEM_UAVCAN_MAX_REG);
    for(uint8_t register_ptr=0; register_ptr<MEM_UAVCAN_MAX_REG; register_ptr++) {
        if(register_index[register_ptr] == MEM_UAVCAN_REG_UNDEF) {
            eeprom_register_set(register_ptr, reg_name, data, data_len);
            perform_add(15);
            return register_ptr;
        }
    }
    return MEM_UAVCAN_REG_UNDEF;
    perform_add(15);
}

// Scrive dati in append per scrittura sequenziale file data remoto
void putDataFile(const char* const file_name, const bool is_firmware, const bool rewrite, void* buf, size_t count)
{
    // PUT INTO FLASH
}

// legge dati in append per trasmissione sequenziale file firmware
bool getDataFile(const char* const file_name, const bool is_firmware, uint64_t position, void* buf, size_t *count)
{
    // GET FROM FLASH
    return true;
}

// Restituisce le info per file firmware e controlli vari
uint64_t getDataFileInfo(const char* const file_name, const bool is_firmware)
{
    // GET FROM FLASH INFO
    //return lof;
    return 100000;
}

// Ritorna vero se il file firmware esiste ed è coerente (Controllo coerenza, esiste..CRC..altro...)
bool ccFirwmareFile(const char* const file_name)
{
    // CHECK FROM FLASH
    return true;
}

// Check if exist or create space register with init default value
void registerSetup(const bool register_init)
{
    perform_start(16);
    if(register_init) {
        // Init AREA E2PROM
        eeprom_register_factory();
    }
    // Open Register in Write se non inizializzati correttamente...
    // Populate INIT Default Value
    uavcan_register_Value_1_0 val = {0};
    uavcan_register_Value_1_0_select_natural16_(&val);
    val.natural16.value.count       = 1;
    val.natural16.value.elements[0] = CAN_MTU_BASE; // CAN_CLASSIC MTU 8
    if(register_init) {
        registerWrite("uavcan.can.mtu", &val);
    } else {
        registerRead("uavcan.can.mtu", &val);
    }
    // We also need the bitrate configuration register. In this demo we can't really use it but an embedded application
    // should define "uavcan.can.bitrate" of type natural32[2]; the second value is 0/ignored if CAN FD not supported.
    // TODO: Default a CAN_BIT_RATE, se CAN_BIT_RATE <> readRegister setup bxCAN con nuovo RATE hot reload
    uavcan_register_Value_1_0_select_natural32_(&val);
    val.natural32.value.count       = 2;
    val.natural32.value.elements[0] = CAN_BIT_RATE;
    val.natural32.value.elements[1] = 0ul;          // Ignored for CANARD_MTU_CAN_CLASSIC
    if(register_init) {
        registerWrite("uavcan.can.bitrate", &val);
    } else {
        registerRead("uavcan.can.bitrate", &val);
    }

    // N.B. Inserire quà la personalizzazione dei registri in SETUP Fisso o di compilazione di modulo
    if(register_init) {
        uavcan_register_Value_1_0_select_natural16_(&val);
        val.natural16.value.count       = 1;
        val.natural16.value.elements[0] = 100;
        registerWrite("uavcan.srv.TH.service_data_and_metadata.id", &val);
    }
    perform_add(16);
}

/// @brief Legge un registro Cypal/Uavcan wrapper UAVCAN 
///        (Imposta Default su Set inout_value su value se non esiste)
/// @param register_name nome del registro
/// @param inout_value valore del registro (formato uavcan) -> Set Valore default
void registerRead(const char* const register_name, uavcan_register_Value_1_0* const inout_value) {
    perform_start(17);
    LOCAL_ASSERT(inout_value != NULL);

    uint8_t register_number;
    bool init_required = !uavcan_register_Value_1_0_is_empty_(inout_value);
    uint8_t serialized[uavcan_register_Value_1_0_EXTENT_BYTES_] = {0};
    size_t sr_size = eeprom_register_get_from_name((const uint8_t*)register_name, &register_number, serialized);
    
    uavcan_register_Value_1_0 out = {0};
    const int8_t err = uavcan_register_Value_1_0_deserialize_(&out, serialized, &sr_size);
    if (err >= 0) {
        init_required = !registerAssign(inout_value, &out);
    }
    if (init_required) {
        printf("Init register: %s\n", register_name);
        registerWrite(register_name, inout_value);
    }
    perform_add(17);
}

/// @brief Scrive un registro Cypal/Uavcan wrapper UAVCAN
/// @param register_name nome del registro
/// @param value valore del registro (formato uavcan)
void registerWrite(const char* const register_name, const uavcan_register_Value_1_0* const value) {
    perform_start(18);
    uint8_t serialized[uavcan_register_Value_1_0_EXTENT_BYTES_] = {0};
    size_t sr_size = uavcan_register_Value_1_0_EXTENT_BYTES_;
    const int8_t err = uavcan_register_Value_1_0_serialize_(value, serialized, &sr_size);
    uint8_t register_index = eeprom_register_get_index_from_name((uint8_t*) register_name);
    if(register_index == MEM_UAVCAN_REG_UNDEF) {
        eeprom_register_add((uint8_t*) register_name, serialized, sr_size);
    } else {
        eeprom_register_set(register_index, (uint8_t*) register_name, serialized, sr_size);
    }
    perform_add(18);
}

/// @brief Scroll degli indici dal primo all'ultimo e return ID UavCAN
///        Nel passaggio di un eventuale INDICE vuoto, non viene incrementato l'ID
///        ID Uavcan è solo l'elenco sequenziale degli ID registarti validi
///        La procedura scorre tutta l'area registri e ritorda IDX+1 alla lettura di un ID Valido 
/// @param index indice di controllo
/// @return UavCan Name Register Formato UAVCAN
uavcan_register_Name_1_0 registerGetNameByIndex(const uint16_t index) {
    perform_start(19);
    uavcan_register_Name_1_0 out = {0};
    uavcan_register_Name_1_0_initialize_(&out);
    char register_name[uavcan_register_Name_1_0_name_ARRAY_CAPACITY_] = {0};
    uint8_t reg_number = 0;
    uint8_t reg_index = 0;
    while(reg_number<=MEM_UAVCAN_MAX_REG) {
        reg_number = eeprom_register_get_next_id(reg_number);
        if(reg_number!=MEM_UAVCAN_REG_UNDEF) {
            reg_index++;
            // Index Exact
            if(reg_index==index) {
                uint8_t _reg_name[MEM_UAVCAN_LEN_NAME_REG];
                eeprom_register_get_name_from_index(reg_number, _reg_name);
                out.name.count = nunavutChooseMin(strlen((char*)_reg_name), uavcan_register_Name_1_0_name_ARRAY_CAPACITY_);
                memcpy(out.name.elements, register_name, out.name.count);
            }
        }
    }
    perform_add(19);
    return out;
}

/// @brief Set factoryReset Register UAVCAN
/// @param  None
void registerDoFactoryReset(void) {
    eeprom_register_factory();
    registerSetup(true);
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
    LOCAL_ASSERT(inout_value != NULL);
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
