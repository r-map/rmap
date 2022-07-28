/// This software is distributed under the terms of the MIT License.
/// Copyright (C) 2021 OpenCyphal <maintainers@opencyphal.org>
/// Author: Pavel Kirienko <pavel@opencyphal.org>

#include "register.h"
#include <stdio.h>
#include <stdlib.h>
//#include <assert.h>
#include <stdbool.h>
#include <sys/stat.h>
//#include <dirent.h>
#include <unistd.h>

#define PASTE3_IMPL(x, y, z) x##y##z
#define PASTE3(x, y, z) PASTE3_IMPL(x, y, z)
#define SDCARD_CHIP_SELECT_PIN PA4
#define SPI_SPEED SD_SCK_MHZ(4)

static const char RegistryDirName[] = "registry";

#if SD_FAT_TYPE == 0
static SdFat SD;
static File file;
#elif SD_FAT_TYPE == 1
static SdFat32 SD;
static File32 file;
#elif SD_FAT_TYPE == 2
static SdExFat SD;
static ExFile file;
#elif SD_FAT_TYPE == 3
static SdFs SD;
static FsFile file;
#endif  // SD_FAT_TYPE
static FatFile filedir;


void registerSetup() {
  SPI.begin();
  pinMode(SDCARD_CHIP_SELECT_PIN, OUTPUT);
  digitalWrite(SDCARD_CHIP_SELECT_PIN, HIGH);
  Serial.println("Initializing SD card...");
  if (SD.begin(SDCARD_CHIP_SELECT_PIN,SPI_SPEED)){
    Serial.print("The FAT type of the volume: ");
    Serial.println(SD.vol()->fatType());
    
    if (!SD.exists(RegistryDirName)) {
      if (!SD.mkdir(RegistryDirName)) {
	Serial.println("Create Folder failed");
	}
    }

    filedir.open(RegistryDirName, O_RDONLY);
    SD.chdir(RegistryDirName);
  }
}


static bool registerOpen(const char* const register_name, const bool write)
{
  // An actual implementation on an embedded system may need to perform atomic file transactions via rename().
  
  //  char file_path[uavcan_register_Name_1_0_name_ARRAY_CAPACITY_ + sizeof(RegistryDirName) + 2] = {0};
  //(void) snprintf(&file_path[0], sizeof(file_path), "%s/%s", RegistryDirName, register_name);

  Serial.print(register_name);
  if (write)
    {
      //if (!file.open(file_path, O_WRONLY | O_CREAT)) {
      if (!file.open(register_name, O_WRONLY | O_CREAT)) {
	Serial.println("->create file failed");
	return false;
      }
    } else {
    if (!file.open(register_name, O_RDONLY)) {
      Serial.println("->open file failed");
      return false;
    }
  }
  
  Serial.println("->opened");
  return true;
	
}

void registerRead(const char* const register_name, uavcan_register_Value_1_0* const inout_value)
{

  assert(inout_value != NULL);
  bool        init_required = !uavcan_register_Value_1_0_is_empty_(inout_value);
  bool status  = registerOpen(&register_name[0], false);
  if (status) {
    uint8_t serialized[uavcan_register_Value_1_0_EXTENT_BYTES_] = {0};
    int  size = file.read(&serialized[0], uavcan_register_Value_1_0_EXTENT_BYTES_);
    file.close();
    if (size > 0){

      Serial.print("Read register: ");
      Serial.println(register_name);

      size_t sr_size = (size_t) size;
      uavcan_register_Value_1_0 out = {0};
      const int8_t              err = uavcan_register_Value_1_0_deserialize_(&out, serialized, &sr_size);
      if (err >= 0){
	init_required = !registerAssign(inout_value, &out);
      }
    } else {
      init_required = true;
    }
  }
  if (init_required) {
    Serial.println("Init register");
    registerWrite(register_name, inout_value);
  }
}
void registerWrite(const char* const register_name, const uavcan_register_Value_1_0* const value)
{ 

  uint8_t      serialized[uavcan_register_Value_1_0_EXTENT_BYTES_] = {0};
  size_t       sr_size                                             = uavcan_register_Value_1_0_EXTENT_BYTES_;

  const int8_t err = uavcan_register_Value_1_0_serialize_(value, serialized, &sr_size);
  if (err >= 0){
    bool status = registerOpen(&register_name[0], true);
    if (status) {
      printf("write register: %s\n", register_name);
      file.write(&serialized[0], sr_size);
      file.close();
    }
  }
}

uavcan_register_Name_1_0 registerGetNameByIndex(const uint16_t index)
{
  // The service definition requires that the ordering is consistent between calls.
  // We assume here that there will be no new registers added while the listing operation is in progress.
  // If this is not the case, you will need to implement additional logic to uphold the ordering consistency
  // guarantee, such as sorting registers by creation time or adding extra metadata.

  uavcan_register_Name_1_0 out = {0};
  uavcan_register_Name_1_0_initialize_(&out);
  
  uint16_t       ii = 0;
  bool found = false;
  
  filedir.rewind();
  
  while (file.openNext(&filedir, O_READ)) {
    if (file.isHidden() || file.isDir()) {
      file.close();
      continue;
    }
      
    if (ii >= index) {
      found = true;
      break;
    }
    ++ii;
    file.close();
  }

  if (found){
    char FileName[120];
    file.getName(FileName, sizeof(FileName));    
    Serial.print("register index:");
    Serial.print(index);
    Serial.print(" name: ");
    Serial.println(FileName);

    out.name.count = nunavutChooseMin(strlen(FileName), uavcan_register_Name_1_0_name_ARRAY_CAPACITY_);
    memcpy(out.name.elements, FileName, out.name.count);
    file.close();
  }
  return out;
}

bool registerAssign(uavcan_register_Value_1_0* const dst, const uavcan_register_Value_1_0* const src)
{
 
    if (uavcan_register_Value_1_0_is_empty_(dst))
    {
        *dst = *src;
        return true;
    }
    if ((uavcan_register_Value_1_0_is_string_(dst) && uavcan_register_Value_1_0_is_string_(src)) ||
        (uavcan_register_Value_1_0_is_unstructured_(dst) && uavcan_register_Value_1_0_is_unstructured_(src)))
    {
        *dst = *src;
        return true;
    }
    if (uavcan_register_Value_1_0_is_bit_(dst) && uavcan_register_Value_1_0_is_bit_(src))
    {
        nunavutCopyBits(dst->bit.value.bitpacked,
                        0,
                        nunavutChooseMin(dst->bit.value.count, src->bit.value.count),
                        src->bit.value.bitpacked,
                        0);
        return true;
    }
    // This is a violation of MISRA/AUTOSAR but it is believed to be less error-prone than manually copy-pasted code.
#define REGISTER_CASE_SAME_TYPE(TYPE)                                                                               \
    if (PASTE3(uavcan_register_Value_1_0_is_, TYPE, _)(dst) && PASTE3(uavcan_register_Value_1_0_is_, TYPE, _)(src)) \
    {                                                                                                               \
        for (size_t i = 0; i < nunavutChooseMin(dst->TYPE.value.count, src->TYPE.value.count); ++i)                 \
        {                                                                                                           \
            dst->TYPE.value.elements[i] = src->TYPE.value.elements[i];                                              \
        }                                                                                                           \
        return true;                                                                                                \
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

void registerDoFactoryReset(void)
{

  Serial.print("factory reset");
  
  filedir.rmRfStar();
  if (!SD.exists(RegistryDirName)) {
    if (!SD.mkdir(RegistryDirName)) {
      Serial.println("Create Folder failed");
    }
  }
  filedir.close();
  
  filedir.open(RegistryDirName, O_RDONLY);
  SD.chdir(RegistryDirName);

}
