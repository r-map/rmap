/// This software is distributed under the terms of the MIT License.
/// Copyright (C) 2021 OpenCyphal <maintainers@opencyphal.org>
/// Author: Pavel Kirienko <pavel@opencyphal.org>

#include "register.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#define PASTE3_IMPL(x, y, z) x##y##z
#define PASTE3(x, y, z) PASTE3_IMPL(x, y, z)

static const char RegistryDirName[] = "registry";

static inline FILE* registerOpen(const char* const register_name, const bool write)
{
  /*
    // An actual implementation on an embedded system may need to perform atomic file transactions via rename().
    char file_path[uavcan_register_Name_1_0_name_ARRAY_CAPACITY_ + sizeof(RegistryDirName) + 2] = {0};
    (void) snprintf(&file_path[0], sizeof(file_path), "%s/%s", RegistryDirName, register_name);
    if (write)
    {
        (void) mkdir(RegistryDirName, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }
    return fopen(&file_path[0], write ? "wb" : "rb");
  */
}

void registerRead(const char* const register_name, uavcan_register_Value_1_0* const inout_value)
{
  /*
    assert(inout_value != NULL);
    bool        init_required = !uavcan_register_Value_1_0_is_empty_(inout_value);
    FILE* const fp            = registerOpen(&register_name[0], false);
    if (fp != NULL)
    {
        uint8_t serialized[uavcan_register_Value_1_0_EXTENT_BYTES_] = {0};
        size_t  sr_size = fread(&serialized[0], 1U, uavcan_register_Value_1_0_EXTENT_BYTES_, fp);
        (void) fclose(fp);
        uavcan_register_Value_1_0 out = {0};
        const int8_t              err = uavcan_register_Value_1_0_deserialize_(&out, serialized, &sr_size);
        if (err >= 0)
        {
            init_required = !registerAssign(inout_value, &out);
        }
    }
    if (init_required)
    {
        printf("Init register: %s\n", register_name);
        registerWrite(register_name, inout_value);
    }
  */
}

void registerWrite(const char* const register_name, const uavcan_register_Value_1_0* const value)
{
  /*
    uint8_t      serialized[uavcan_register_Value_1_0_EXTENT_BYTES_] = {0};
    size_t       sr_size                                             = uavcan_register_Value_1_0_EXTENT_BYTES_;
    const int8_t err = uavcan_register_Value_1_0_serialize_(value, serialized, &sr_size);
    if (err >= 0)
    {
        FILE* const fp = registerOpen(&register_name[0], true);
        if (fp != NULL)
        {
            (void) fwrite(&serialized[0], 1U, sr_size, fp);
            (void) fclose(fp);
        }
    }
  */
}

uavcan_register_Name_1_0 registerGetNameByIndex(const uint16_t index)
{
  /*
    uavcan_register_Name_1_0 out = {0};
    uavcan_register_Name_1_0_initialize_(&out);
    DIR* const dp = opendir(RegistryDirName);
    if (dp != NULL)
    {
        // The service definition requires that the ordering is consistent between calls.
        // We assume here that there will be no new registers added while the listing operation is in progress.
        // If this is not the case, you will need to implement additional logic to uphold the ordering consistency
        // guarantee, such as sorting registers by creation time or adding extra metadata.
        struct dirent* ep = readdir(dp);
        uint16_t       ii = 0;
        while (ep != NULL)
        {
            if (ep->d_type == DT_REG)
            {
                if (ii >= index)
                {
                    break;
                }
                ++ii;
            }
            ep = readdir(dp);
        }
        if (ep != NULL)
        {
            out.name.count = nunavutChooseMin(strlen(ep->d_name), uavcan_register_Name_1_0_name_ARRAY_CAPACITY_);
            memcpy(out.name.elements, ep->d_name, out.name.count);
        }
        (void) closedir(dp);
    }
    return out;
  */
}

bool registerAssign(uavcan_register_Value_1_0* const dst, const uavcan_register_Value_1_0* const src)
{
  /*
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
  */
}

void registerDoFactoryReset(void)
{
  /*
    DIR* const dp = opendir(RegistryDirName);
    if (dp != NULL)
    {
        struct dirent* ep = readdir(dp);
        while (ep != NULL)
        {
            if (ep->d_type == DT_REG)
            {
                char file_path[uavcan_register_Name_1_0_name_ARRAY_CAPACITY_ + sizeof(RegistryDirName) + 2] = {0};
                (void) snprintf(&file_path[0], sizeof(file_path), "%s/%s", RegistryDirName, ep->d_name);
                (void) unlink(&file_path[0]);
            }
            ep = readdir(dp);
        }
        (void) closedir(dp);
    }
  */
}
