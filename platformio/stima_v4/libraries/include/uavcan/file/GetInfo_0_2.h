// This is an AUTO-GENERATED UAVCAN DSDL data type implementation. Curious? See https://opencyphal.org.
// You shouldn't attempt to edit this file.
//
// Checking this file under version control is not recommended unless it is used as part of a high-SIL
// safety-critical codebase. The typical usage scenario is to generate it as part of the build process.
//
// To avoid conflicts with definitions given in the source DSDL file, all entities created by the code generator
// are named with an underscore at the end, like foo_bar_().
//
// Generator:     nunavut-1.8.3 (serialization was enabled)
// Source file:   C:/Dati/RMAP/stimav4-rmap/rmap/platformio/stima_v4/libraries/data_types/uavcan/file/405.GetInfo.0.2.dsdl
// Generated at:  2023-06-03 00:09:38.590576 UTC
// Is deprecated: no
// Fixed port-ID: 405
// Full name:     uavcan.file.GetInfo
// Version:       0.2
//
// Platform
//     python_implementation:  CPython
//     python_version:  3.10.5
//     python_release_level:  final
//     python_build:  ('tags/v3.10.5:f377153', 'Jun  6 2022 16:14:13')
//     python_compiler:  MSC v.1929 64 bit (AMD64)
//     python_revision:  f377153
//     python_xoptions:  {}
//     runtime_platform:  Windows-10-10.0.22621-SP0
//
// Language Options
//     target_endianness:  little
//     omit_float_serialization_support:  False
//     enable_serialization_asserts:  False
//     enable_override_variable_array_capacity:  False
//     cast_format:  (({type}) {value})

#ifndef UAVCAN_FILE_GET_INFO_0_2_INCLUDED_
#define UAVCAN_FILE_GET_INFO_0_2_INCLUDED_

#include <nunavut/support/serialization.h>
#include <uavcan/file/Error_1_0.h>
#include <uavcan/file/Path_2_0.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

static_assert( NUNAVUT_SUPPORT_LANGUAGE_OPTION_TARGET_ENDIANNESS == 434322821,
              "C:/Dati/RMAP/stimav4-rmap/rmap/platformio/stima_v4/libraries/data_types/uavcan/file/405.GetInfo.0.2.dsdl is trying to use a serialization library that was compiled with "
              "different language options. This is dangerous and therefore not allowed." );
static_assert( NUNAVUT_SUPPORT_LANGUAGE_OPTION_OMIT_FLOAT_SERIALIZATION_SUPPORT == 0,
              "C:/Dati/RMAP/stimav4-rmap/rmap/platformio/stima_v4/libraries/data_types/uavcan/file/405.GetInfo.0.2.dsdl is trying to use a serialization library that was compiled with "
              "different language options. This is dangerous and therefore not allowed." );
static_assert( NUNAVUT_SUPPORT_LANGUAGE_OPTION_ENABLE_SERIALIZATION_ASSERTS == 0,
              "C:/Dati/RMAP/stimav4-rmap/rmap/platformio/stima_v4/libraries/data_types/uavcan/file/405.GetInfo.0.2.dsdl is trying to use a serialization library that was compiled with "
              "different language options. This is dangerous and therefore not allowed." );
static_assert( NUNAVUT_SUPPORT_LANGUAGE_OPTION_ENABLE_OVERRIDE_VARIABLE_ARRAY_CAPACITY == 0,
              "C:/Dati/RMAP/stimav4-rmap/rmap/platformio/stima_v4/libraries/data_types/uavcan/file/405.GetInfo.0.2.dsdl is trying to use a serialization library that was compiled with "
              "different language options. This is dangerous and therefore not allowed." );
static_assert( NUNAVUT_SUPPORT_LANGUAGE_OPTION_CAST_FORMAT == 2368206204,
              "C:/Dati/RMAP/stimav4-rmap/rmap/platformio/stima_v4/libraries/data_types/uavcan/file/405.GetInfo.0.2.dsdl is trying to use a serialization library that was compiled with "
              "different language options. This is dangerous and therefore not allowed." );

#ifdef __cplusplus
extern "C" {
#endif

#define uavcan_file_GetInfo_0_2_HAS_FIXED_PORT_ID_ true
#define uavcan_file_GetInfo_0_2_FIXED_PORT_ID_     405U

#define uavcan_file_GetInfo_0_2_FULL_NAME_             "uavcan.file.GetInfo"
#define uavcan_file_GetInfo_0_2_FULL_NAME_AND_VERSION_ "uavcan.file.GetInfo.0.2"

#define uavcan_file_GetInfo_Request_0_2_FULL_NAME_             "uavcan.file.GetInfo.Request"
#define uavcan_file_GetInfo_Request_0_2_FULL_NAME_AND_VERSION_ "uavcan.file.GetInfo.Request.0.2"

/// Extent is the minimum amount of memory required to hold any serialized representation of any compatible
/// version of the data type; or, on other words, it is the the maximum possible size of received objects of this type.
/// The size is specified in bytes (rather than bits) because by definition, extent is an integer number of bytes long.
/// When allocating a deserialization (RX) buffer for this data type, it should be at least extent bytes large.
/// When allocating a serialization (TX) buffer, it is safe to use the size of the largest serialized representation
/// instead of the extent because it provides a tighter bound of the object size; it is safe because the concrete type
/// is always known during serialization (unlike deserialization). If not sure, use extent everywhere.
#define uavcan_file_GetInfo_Request_0_2_EXTENT_BYTES_                    300UL
#define uavcan_file_GetInfo_Request_0_2_SERIALIZATION_BUFFER_SIZE_BYTES_ 256UL
static_assert(uavcan_file_GetInfo_Request_0_2_EXTENT_BYTES_ >= uavcan_file_GetInfo_Request_0_2_SERIALIZATION_BUFFER_SIZE_BYTES_,
              "Internal constraint violation");

typedef struct
{
    /// uavcan.file.Path.2.0 path
    uavcan_file_Path_2_0 path;
} uavcan_file_GetInfo_Request_0_2;

/// Serialize an instance into the provided buffer.
/// The lifetime of the resulting serialized representation is independent of the original instance.
/// This method may be slow for large objects (e.g., images, point clouds, radar samples), so in a later revision
/// we may define a zero-copy alternative that keeps references to the original object where possible.
///
/// @param obj      The object to serialize.
///
/// @param buffer   The destination buffer. There are no alignment requirements.
///                 @see uavcan_file_GetInfo_Request_0_2_SERIALIZATION_BUFFER_SIZE_BYTES_
///
/// @param inout_buffer_size_bytes  When calling, this is a pointer to the size of the buffer in bytes.
///                                 Upon return this value will be updated with the size of the constructed serialized
///                                 representation (in bytes); this value is then to be passed over to the transport
///                                 layer. In case of error this value is undefined.
///
/// @returns Negative on error, zero on success.
static inline int8_t uavcan_file_GetInfo_Request_0_2_serialize_(
    const uavcan_file_GetInfo_Request_0_2* const obj, uint8_t* const buffer,  size_t* const inout_buffer_size_bytes)
{
    if ((obj == NULL) || (buffer == NULL) || (inout_buffer_size_bytes == NULL))
    {
        return -NUNAVUT_ERROR_INVALID_ARGUMENT;
    }


    const size_t capacity_bytes = *inout_buffer_size_bytes;
    if ((8U * (size_t) capacity_bytes) < 2048UL)
    {
        return -NUNAVUT_ERROR_SERIALIZATION_BUFFER_TOO_SMALL;
    }
    // Notice that fields that are not an integer number of bytes long may overrun the space allocated for them
    // in the serialization buffer up to the next byte boundary. This is by design and is guaranteed to be safe.
    size_t offset_bits = 0U;





    {   // uavcan.file.Path.2.0 path
        size_t _size_bytes0_ = 256UL;  // Nested object (max) size, in bytes.
        int8_t _err0_ = uavcan_file_Path_2_0_serialize_(
            &obj->path, &buffer[offset_bits / 8U], &_size_bytes0_);
        if (_err0_ < 0)
        {
            return _err0_;
        }
        // It is assumed that we know the exact type of the serialized entity, hence we expect the size to match.
        offset_bits += _size_bytes0_ * 8U;  // Advance by the size of the nested object.
    }


    if (offset_bits % 8U != 0U)  // Pad to 8 bits. TODO: Eliminate redundant padding checks.
    {
        const uint8_t _pad0_ = (uint8_t)(8U - offset_bits % 8U);
        const int8_t _err1_ = nunavutSetUxx(&buffer[0], capacity_bytes, offset_bits, 0U, _pad0_);  // Optimize?
        if (_err1_ < 0)
        {
            return _err1_;
        }
        offset_bits += _pad0_;
    }
    // It is assumed that we know the exact type of the serialized entity, hence we expect the size to match.





    *inout_buffer_size_bytes = (size_t) (offset_bits / 8U);

    return NUNAVUT_SUCCESS;
}

/// Deserialize an instance from the provided buffer.
/// The lifetime of the resulting object is independent of the original buffer.
/// This method may be slow for large objects (e.g., images, point clouds, radar samples), so in a later revision
/// we may define a zero-copy alternative that keeps references to the original buffer where possible.
///
/// @param obj      The object to update from the provided serialized representation.
///
/// @param buffer   The source buffer containing the serialized representation. There are no alignment requirements.
///                 If the buffer is shorter or longer than expected, it will be implicitly zero-extended or truncated,
///                 respectively; see Specification for "implicit zero extension" and "implicit truncation" rules.
///
/// @param inout_buffer_size_bytes  When calling, this is a pointer to the size of the supplied serialized
///                                 representation, in bytes. Upon return this value will be updated with the
///                                 size of the consumed fragment of the serialized representation (in bytes),
///                                 which may be smaller due to the implicit truncation rule, but it is guaranteed
///                                 to never exceed the original buffer size even if the implicit zero extension rule
///                                 was activated. In case of error this value is undefined.
///
/// @returns Negative on error, zero on success.
static inline int8_t uavcan_file_GetInfo_Request_0_2_deserialize_(
    uavcan_file_GetInfo_Request_0_2* const out_obj, const uint8_t* buffer, size_t* const inout_buffer_size_bytes)
{
    if ((out_obj == NULL) || (inout_buffer_size_bytes == NULL) || ((buffer == NULL) && (0 != *inout_buffer_size_bytes)))
    {
        return -NUNAVUT_ERROR_INVALID_ARGUMENT;
    }
    if (buffer == NULL)
    {
        buffer = (const uint8_t*)"";
    }


    const size_t capacity_bytes = *inout_buffer_size_bytes;
    const size_t capacity_bits = capacity_bytes * (size_t) 8U;
    size_t offset_bits = 0U;





    // uavcan.file.Path.2.0 path
    {
        size_t _size_bytes1_ = (size_t)(capacity_bytes - nunavutChooseMin((offset_bits / 8U), capacity_bytes));
        const int8_t _err2_ = uavcan_file_Path_2_0_deserialize_(
            &out_obj->path, &buffer[offset_bits / 8U], &_size_bytes1_);
        if (_err2_ < 0)
        {
            return _err2_;
        }
        offset_bits += _size_bytes1_ * 8U;  // Advance by the size of the nested serialized representation.
    }


    offset_bits = (offset_bits + 7U) & ~(size_t) 7U;  // Align on 8 bits.

    *inout_buffer_size_bytes = (size_t) (nunavutChooseMin(offset_bits, capacity_bits) / 8U);


    return NUNAVUT_SUCCESS;
}

/// Initialize an instance to default values. Does nothing if @param out_obj is NULL.
/// This function intentionally leaves inactive elements uninitialized; for example, members of a variable-length
/// array beyond its length are left uninitialized; aliased union memory that is not used by the first union field
/// is left uninitialized, etc. If full zero-initialization is desired, just use memset(&obj, 0, sizeof(obj)).
static inline void uavcan_file_GetInfo_Request_0_2_initialize_(uavcan_file_GetInfo_Request_0_2* const out_obj)
{
    if (out_obj != NULL)
    {
        size_t size_bytes = 0;
        const uint8_t buf = 0;
        const int8_t err = uavcan_file_GetInfo_Request_0_2_deserialize_(out_obj, &buf, &size_bytes);

        (void) err;
    }
}



#define uavcan_file_GetInfo_Response_0_2_FULL_NAME_             "uavcan.file.GetInfo.Response"
#define uavcan_file_GetInfo_Response_0_2_FULL_NAME_AND_VERSION_ "uavcan.file.GetInfo.Response.0.2"

/// Extent is the minimum amount of memory required to hold any serialized representation of any compatible
/// version of the data type; or, on other words, it is the the maximum possible size of received objects of this type.
/// The size is specified in bytes (rather than bits) because by definition, extent is an integer number of bytes long.
/// When allocating a deserialization (RX) buffer for this data type, it should be at least extent bytes large.
/// When allocating a serialization (TX) buffer, it is safe to use the size of the largest serialized representation
/// instead of the extent because it provides a tighter bound of the object size; it is safe because the concrete type
/// is always known during serialization (unlike deserialization). If not sure, use extent everywhere.
#define uavcan_file_GetInfo_Response_0_2_EXTENT_BYTES_                    48UL
#define uavcan_file_GetInfo_Response_0_2_SERIALIZATION_BUFFER_SIZE_BYTES_ 13UL
static_assert(uavcan_file_GetInfo_Response_0_2_EXTENT_BYTES_ >= uavcan_file_GetInfo_Response_0_2_SERIALIZATION_BUFFER_SIZE_BYTES_,
              "Internal constraint violation");

typedef struct
{
    /// uavcan.file.Error.1.0 error
    uavcan_file_Error_1_0 _error;

    /// truncated uint40 size
    uint64_t size;

    /// truncated uint40 unix_timestamp_of_last_modification
    uint64_t unix_timestamp_of_last_modification;

    /// saturated bool is_file_not_directory
    bool is_file_not_directory;

    /// saturated bool is_link
    bool is_link;

    /// saturated bool is_readable
    bool is_readable;

    /// saturated bool is_writeable
    bool is_writeable;
} uavcan_file_GetInfo_Response_0_2;

/// Serialize an instance into the provided buffer.
/// The lifetime of the resulting serialized representation is independent of the original instance.
/// This method may be slow for large objects (e.g., images, point clouds, radar samples), so in a later revision
/// we may define a zero-copy alternative that keeps references to the original object where possible.
///
/// @param obj      The object to serialize.
///
/// @param buffer   The destination buffer. There are no alignment requirements.
///                 @see uavcan_file_GetInfo_Response_0_2_SERIALIZATION_BUFFER_SIZE_BYTES_
///
/// @param inout_buffer_size_bytes  When calling, this is a pointer to the size of the buffer in bytes.
///                                 Upon return this value will be updated with the size of the constructed serialized
///                                 representation (in bytes); this value is then to be passed over to the transport
///                                 layer. In case of error this value is undefined.
///
/// @returns Negative on error, zero on success.
static inline int8_t uavcan_file_GetInfo_Response_0_2_serialize_(
    const uavcan_file_GetInfo_Response_0_2* const obj, uint8_t* const buffer,  size_t* const inout_buffer_size_bytes)
{
    if ((obj == NULL) || (buffer == NULL) || (inout_buffer_size_bytes == NULL))
    {
        return -NUNAVUT_ERROR_INVALID_ARGUMENT;
    }


    const size_t capacity_bytes = *inout_buffer_size_bytes;
    if ((8U * (size_t) capacity_bytes) < 104UL)
    {
        return -NUNAVUT_ERROR_SERIALIZATION_BUFFER_TOO_SMALL;
    }
    // Notice that fields that are not an integer number of bytes long may overrun the space allocated for them
    // in the serialization buffer up to the next byte boundary. This is by design and is guaranteed to be safe.
    size_t offset_bits = 0U;





    {   // uavcan.file.Error.1.0 error
        size_t _size_bytes2_ = 2UL;  // Nested object (max) size, in bytes.
        int8_t _err3_ = uavcan_file_Error_1_0_serialize_(
            &obj->_error, &buffer[offset_bits / 8U], &_size_bytes2_);
        if (_err3_ < 0)
        {
            return _err3_;
        }
        // It is assumed that we know the exact type of the serialized entity, hence we expect the size to match.
        offset_bits += _size_bytes2_ * 8U;  // Advance by the size of the nested object.
    }




    {   // truncated uint40 size
        (void) memmove(&buffer[offset_bits / 8U], &obj->size, 5U);
        offset_bits += 40U;
    }




    {   // truncated uint40 unix_timestamp_of_last_modification
        (void) memmove(&buffer[offset_bits / 8U], &obj->unix_timestamp_of_last_modification, 5U);
        offset_bits += 40U;
    }




    {   // saturated bool is_file_not_directory
        buffer[offset_bits / 8U] = obj->is_file_not_directory ? 1U : 0U;
        offset_bits += 1U;
    }




    {   // saturated bool is_link
        if (obj->is_link)
        {
            buffer[offset_bits / 8U] = (uint8_t)(buffer[offset_bits / 8U] | (1U << (offset_bits % 8U)));
        }
        else
        {
            buffer[offset_bits / 8U] = (uint8_t)(buffer[offset_bits / 8U] & ~(1U << (offset_bits % 8U)));
        }
        offset_bits += 1U;
    }




    {   // saturated bool is_readable
        if (obj->is_readable)
        {
            buffer[offset_bits / 8U] = (uint8_t)(buffer[offset_bits / 8U] | (1U << (offset_bits % 8U)));
        }
        else
        {
            buffer[offset_bits / 8U] = (uint8_t)(buffer[offset_bits / 8U] & ~(1U << (offset_bits % 8U)));
        }
        offset_bits += 1U;
    }




    {   // saturated bool is_writeable
        if (obj->is_writeable)
        {
            buffer[offset_bits / 8U] = (uint8_t)(buffer[offset_bits / 8U] | (1U << (offset_bits % 8U)));
        }
        else
        {
            buffer[offset_bits / 8U] = (uint8_t)(buffer[offset_bits / 8U] & ~(1U << (offset_bits % 8U)));
        }
        offset_bits += 1U;
    }




    {   // void4
        const int8_t _err4_ = nunavutSetUxx(&buffer[0], capacity_bytes, offset_bits, 0U, 4U);  // Optimize?
        if (_err4_ < 0)
        {
            return _err4_;
        }
        offset_bits += 4UL;
    }


    if (offset_bits % 8U != 0U)  // Pad to 8 bits. TODO: Eliminate redundant padding checks.
    {
        const uint8_t _pad1_ = (uint8_t)(8U - offset_bits % 8U);
        const int8_t _err5_ = nunavutSetUxx(&buffer[0], capacity_bytes, offset_bits, 0U, _pad1_);  // Optimize?
        if (_err5_ < 0)
        {
            return _err5_;
        }
        offset_bits += _pad1_;
    }
    // It is assumed that we know the exact type of the serialized entity, hence we expect the size to match.




    *inout_buffer_size_bytes = (size_t) (offset_bits / 8U);

    return NUNAVUT_SUCCESS;
}

/// Deserialize an instance from the provided buffer.
/// The lifetime of the resulting object is independent of the original buffer.
/// This method may be slow for large objects (e.g., images, point clouds, radar samples), so in a later revision
/// we may define a zero-copy alternative that keeps references to the original buffer where possible.
///
/// @param obj      The object to update from the provided serialized representation.
///
/// @param buffer   The source buffer containing the serialized representation. There are no alignment requirements.
///                 If the buffer is shorter or longer than expected, it will be implicitly zero-extended or truncated,
///                 respectively; see Specification for "implicit zero extension" and "implicit truncation" rules.
///
/// @param inout_buffer_size_bytes  When calling, this is a pointer to the size of the supplied serialized
///                                 representation, in bytes. Upon return this value will be updated with the
///                                 size of the consumed fragment of the serialized representation (in bytes),
///                                 which may be smaller due to the implicit truncation rule, but it is guaranteed
///                                 to never exceed the original buffer size even if the implicit zero extension rule
///                                 was activated. In case of error this value is undefined.
///
/// @returns Negative on error, zero on success.
static inline int8_t uavcan_file_GetInfo_Response_0_2_deserialize_(
    uavcan_file_GetInfo_Response_0_2* const out_obj, const uint8_t* buffer, size_t* const inout_buffer_size_bytes)
{
    if ((out_obj == NULL) || (inout_buffer_size_bytes == NULL) || ((buffer == NULL) && (0 != *inout_buffer_size_bytes)))
    {
        return -NUNAVUT_ERROR_INVALID_ARGUMENT;
    }
    if (buffer == NULL)
    {
        buffer = (const uint8_t*)"";
    }


    const size_t capacity_bytes = *inout_buffer_size_bytes;
    const size_t capacity_bits = capacity_bytes * (size_t) 8U;
    size_t offset_bits = 0U;





    // uavcan.file.Error.1.0 error
    {
        size_t _size_bytes3_ = (size_t)(capacity_bytes - nunavutChooseMin((offset_bits / 8U), capacity_bytes));
        const int8_t _err6_ = uavcan_file_Error_1_0_deserialize_(
            &out_obj->_error, &buffer[offset_bits / 8U], &_size_bytes3_);
        if (_err6_ < 0)
        {
            return _err6_;
        }
        offset_bits += _size_bytes3_ * 8U;  // Advance by the size of the nested serialized representation.
    }




    // truncated uint40 size
    out_obj->size = nunavutGetU64(&buffer[0], capacity_bytes, offset_bits, 40);
    offset_bits += 40U;




    // truncated uint40 unix_timestamp_of_last_modification
    out_obj->unix_timestamp_of_last_modification = nunavutGetU64(&buffer[0], capacity_bytes, offset_bits, 40);
    offset_bits += 40U;




    // saturated bool is_file_not_directory
    if (offset_bits < capacity_bits)
    {
        out_obj->is_file_not_directory = (buffer[offset_bits / 8U] & 1U) != 0U;
    }
    else
    {
        out_obj->is_file_not_directory = false;
    }
    offset_bits += 1U;




    // saturated bool is_link
    if (offset_bits < capacity_bits)
    {
        out_obj->is_link = (buffer[offset_bits / 8U] & (1U << (offset_bits % 8U))) != 0U;
    }
    else
    {
        out_obj->is_link = false;
    }
    offset_bits += 1U;




    // saturated bool is_readable
    if (offset_bits < capacity_bits)
    {
        out_obj->is_readable = (buffer[offset_bits / 8U] & (1U << (offset_bits % 8U))) != 0U;
    }
    else
    {
        out_obj->is_readable = false;
    }
    offset_bits += 1U;




    // saturated bool is_writeable
    if (offset_bits < capacity_bits)
    {
        out_obj->is_writeable = (buffer[offset_bits / 8U] & (1U << (offset_bits % 8U))) != 0U;
    }
    else
    {
        out_obj->is_writeable = false;
    }
    offset_bits += 1U;




    // void4
    offset_bits += 4;


    offset_bits = (offset_bits + 7U) & ~(size_t) 7U;  // Align on 8 bits.

    *inout_buffer_size_bytes = (size_t) (nunavutChooseMin(offset_bits, capacity_bits) / 8U);


    return NUNAVUT_SUCCESS;
}

/// Initialize an instance to default values. Does nothing if @param out_obj is NULL.
/// This function intentionally leaves inactive elements uninitialized; for example, members of a variable-length
/// array beyond its length are left uninitialized; aliased union memory that is not used by the first union field
/// is left uninitialized, etc. If full zero-initialization is desired, just use memset(&obj, 0, sizeof(obj)).
static inline void uavcan_file_GetInfo_Response_0_2_initialize_(uavcan_file_GetInfo_Response_0_2* const out_obj)
{
    if (out_obj != NULL)
    {
        size_t size_bytes = 0;
        const uint8_t buf = 0;
        const int8_t err = uavcan_file_GetInfo_Response_0_2_deserialize_(out_obj, &buf, &size_bytes);

        (void) err;
    }
}



#ifdef __cplusplus
}
#endif
#endif // UAVCAN_FILE_GET_INFO_0_2_INCLUDED_
