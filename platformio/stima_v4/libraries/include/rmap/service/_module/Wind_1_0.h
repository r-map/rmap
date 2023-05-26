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
// Source file:   C:/Dati/RMAP/stimav4-rmap/rmap/platformio/stima_v4/libraries/data_types/rmap/service/module/Wind.1.0.dsdl
// Generated at:  2023-05-25 22:06:37.409739 UTC
// Is deprecated: no
// Fixed port-ID: None
// Full name:     rmap.service.module.Wind
// Version:       1.0
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

#ifndef RMAP_SERVICE_MODULE_WIND_1_0_INCLUDED_
#define RMAP_SERVICE_MODULE_WIND_1_0_INCLUDED_

#include <nunavut/support/serialization.h>
#include <rmap/sensors/WindAvgSpeed_1_0.h>
#include <rmap/sensors/WindAvgVect10_1_0.h>
#include <rmap/sensors/WindAvgVect_1_0.h>
#include <rmap/sensors/WindClassSpeed_1_0.h>
#include <rmap/sensors/WindGustDirection_1_0.h>
#include <rmap/sensors/WindGustSpeed_1_0.h>
#include <rmap/service/setmode_1_0.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

static_assert( NUNAVUT_SUPPORT_LANGUAGE_OPTION_TARGET_ENDIANNESS == 434322821,
              "C:/Dati/RMAP/stimav4-rmap/rmap/platformio/stima_v4/libraries/data_types/rmap/service/module/Wind.1.0.dsdl is trying to use a serialization library that was compiled with "
              "different language options. This is dangerous and therefore not allowed." );
static_assert( NUNAVUT_SUPPORT_LANGUAGE_OPTION_OMIT_FLOAT_SERIALIZATION_SUPPORT == 0,
              "C:/Dati/RMAP/stimav4-rmap/rmap/platformio/stima_v4/libraries/data_types/rmap/service/module/Wind.1.0.dsdl is trying to use a serialization library that was compiled with "
              "different language options. This is dangerous and therefore not allowed." );
static_assert( NUNAVUT_SUPPORT_LANGUAGE_OPTION_ENABLE_SERIALIZATION_ASSERTS == 0,
              "C:/Dati/RMAP/stimav4-rmap/rmap/platformio/stima_v4/libraries/data_types/rmap/service/module/Wind.1.0.dsdl is trying to use a serialization library that was compiled with "
              "different language options. This is dangerous and therefore not allowed." );
static_assert( NUNAVUT_SUPPORT_LANGUAGE_OPTION_ENABLE_OVERRIDE_VARIABLE_ARRAY_CAPACITY == 0,
              "C:/Dati/RMAP/stimav4-rmap/rmap/platformio/stima_v4/libraries/data_types/rmap/service/module/Wind.1.0.dsdl is trying to use a serialization library that was compiled with "
              "different language options. This is dangerous and therefore not allowed." );
static_assert( NUNAVUT_SUPPORT_LANGUAGE_OPTION_CAST_FORMAT == 2368206204,
              "C:/Dati/RMAP/stimav4-rmap/rmap/platformio/stima_v4/libraries/data_types/rmap/service/module/Wind.1.0.dsdl is trying to use a serialization library that was compiled with "
              "different language options. This is dangerous and therefore not allowed." );

#ifdef __cplusplus
extern "C" {
#endif

/// This type does not have a fixed port-ID. See https://forum.opencyphal.org/t/choosing-message-and-service-ids/889
#define rmap_service_module_Wind_1_0_HAS_FIXED_PORT_ID_ false

#define rmap_service_module_Wind_1_0_FULL_NAME_             "rmap.service.module.Wind"
#define rmap_service_module_Wind_1_0_FULL_NAME_AND_VERSION_ "rmap.service.module.Wind.1.0"

#define rmap_service_module_Wind_Request_1_0_FULL_NAME_             "rmap.service.module.Wind.Request"
#define rmap_service_module_Wind_Request_1_0_FULL_NAME_AND_VERSION_ "rmap.service.module.Wind.Request.1.0"

/// Extent is the minimum amount of memory required to hold any serialized representation of any compatible
/// version of the data type; or, on other words, it is the the maximum possible size of received objects of this type.
/// The size is specified in bytes (rather than bits) because by definition, extent is an integer number of bytes long.
/// When allocating a deserialization (RX) buffer for this data type, it should be at least extent bytes large.
/// When allocating a serialization (TX) buffer, it is safe to use the size of the largest serialized representation
/// instead of the extent because it provides a tighter bound of the object size; it is safe because the concrete type
/// is always known during serialization (unlike deserialization). If not sure, use extent everywhere.
#define rmap_service_module_Wind_Request_1_0_EXTENT_BYTES_                    4UL
#define rmap_service_module_Wind_Request_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_ 4UL
static_assert(rmap_service_module_Wind_Request_1_0_EXTENT_BYTES_ >= rmap_service_module_Wind_Request_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_,
              "Internal constraint violation");

typedef struct
{
    /// rmap.service.setmode.1.0 parameter
    rmap_service_setmode_1_0 parameter;
} rmap_service_module_Wind_Request_1_0;

/// Serialize an instance into the provided buffer.
/// The lifetime of the resulting serialized representation is independent of the original instance.
/// This method may be slow for large objects (e.g., images, point clouds, radar samples), so in a later revision
/// we may define a zero-copy alternative that keeps references to the original object where possible.
///
/// @param obj      The object to serialize.
///
/// @param buffer   The destination buffer. There are no alignment requirements.
///                 @see rmap_service_module_Wind_Request_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_
///
/// @param inout_buffer_size_bytes  When calling, this is a pointer to the size of the buffer in bytes.
///                                 Upon return this value will be updated with the size of the constructed serialized
///                                 representation (in bytes); this value is then to be passed over to the transport
///                                 layer. In case of error this value is undefined.
///
/// @returns Negative on error, zero on success.
static inline int8_t rmap_service_module_Wind_Request_1_0_serialize_(
    const rmap_service_module_Wind_Request_1_0* const obj, uint8_t* const buffer,  size_t* const inout_buffer_size_bytes)
{
    if ((obj == NULL) || (buffer == NULL) || (inout_buffer_size_bytes == NULL))
    {
        return -NUNAVUT_ERROR_INVALID_ARGUMENT;
    }


    const size_t capacity_bytes = *inout_buffer_size_bytes;
    if ((8U * (size_t) capacity_bytes) < 32UL)
    {
        return -NUNAVUT_ERROR_SERIALIZATION_BUFFER_TOO_SMALL;
    }
    // Notice that fields that are not an integer number of bytes long may overrun the space allocated for them
    // in the serialization buffer up to the next byte boundary. This is by design and is guaranteed to be safe.
    size_t offset_bits = 0U;





    {   // rmap.service.setmode.1.0 parameter
        size_t _size_bytes0_ = 4UL;  // Nested object (max) size, in bytes.
        int8_t _err0_ = rmap_service_setmode_1_0_serialize_(
            &obj->parameter, &buffer[offset_bits / 8U], &_size_bytes0_);
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
static inline int8_t rmap_service_module_Wind_Request_1_0_deserialize_(
    rmap_service_module_Wind_Request_1_0* const out_obj, const uint8_t* buffer, size_t* const inout_buffer_size_bytes)
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





    // rmap.service.setmode.1.0 parameter
    {
        size_t _size_bytes1_ = (size_t)(capacity_bytes - nunavutChooseMin((offset_bits / 8U), capacity_bytes));
        const int8_t _err2_ = rmap_service_setmode_1_0_deserialize_(
            &out_obj->parameter, &buffer[offset_bits / 8U], &_size_bytes1_);
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
static inline void rmap_service_module_Wind_Request_1_0_initialize_(rmap_service_module_Wind_Request_1_0* const out_obj)
{
    if (out_obj != NULL)
    {
        size_t size_bytes = 0;
        const uint8_t buf = 0;
        const int8_t err = rmap_service_module_Wind_Request_1_0_deserialize_(out_obj, &buf, &size_bytes);

        (void) err;
    }
}



#define rmap_service_module_Wind_Response_1_0_FULL_NAME_             "rmap.service.module.Wind.Response"
#define rmap_service_module_Wind_Response_1_0_FULL_NAME_AND_VERSION_ "rmap.service.module.Wind.Response.1.0"

/// Extent is the minimum amount of memory required to hold any serialized representation of any compatible
/// version of the data type; or, on other words, it is the the maximum possible size of received objects of this type.
/// The size is specified in bytes (rather than bits) because by definition, extent is an integer number of bytes long.
/// When allocating a deserialization (RX) buffer for this data type, it should be at least extent bytes large.
/// When allocating a serialization (TX) buffer, it is safe to use the size of the largest serialized representation
/// instead of the extent because it provides a tighter bound of the object size; it is safe because the concrete type
/// is always known during serialization (unlike deserialization). If not sure, use extent everywhere.
#define rmap_service_module_Wind_Response_1_0_EXTENT_BYTES_                    123UL
#define rmap_service_module_Wind_Response_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_ 123UL
static_assert(rmap_service_module_Wind_Response_1_0_EXTENT_BYTES_ >= rmap_service_module_Wind_Response_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_,
              "Internal constraint violation");

typedef struct
{
    /// saturated uint4 state
    uint8_t state;

    /// saturated uint8 version
    uint8_t version;

    /// saturated uint8 revision
    uint8_t revision;

    /// saturated bool is_windsonic_unit_error
    bool is_windsonic_unit_error;

    /// saturated bool is_windsonic_hardware_error
    bool is_windsonic_hardware_error;

    /// saturated bool is_windsonic_responding_error
    bool is_windsonic_responding_error;

    /// saturated bool is_windsonic_axis_error
    bool is_windsonic_axis_error;

    /// saturated bool is_windsonic_crc_error
    bool is_windsonic_crc_error;

    /// saturated uint7 perc_rs232_error
    uint8_t perc_rs232_error;

    /// saturated uint8 rbt_event
    uint8_t rbt_event;

    /// saturated uint8 wdt_event
    uint8_t wdt_event;

    /// rmap.sensors.WindAvgVect10.1.0 DWA
    rmap_sensors_WindAvgVect10_1_0 DWA;

    /// rmap.sensors.WindAvgVect.1.0 DWB
    rmap_sensors_WindAvgVect_1_0 DWB;

    /// rmap.sensors.WindGustSpeed.1.0 DWC
    rmap_sensors_WindGustSpeed_1_0 DWC;

    /// rmap.sensors.WindAvgSpeed.1.0 DWD
    rmap_sensors_WindAvgSpeed_1_0 DWD;

    /// rmap.sensors.WindClassSpeed.1.0 DWE
    rmap_sensors_WindClassSpeed_1_0 DWE;

    /// rmap.sensors.WindGustDirection.1.0 DWF
    rmap_sensors_WindGustDirection_1_0 DWF;
} rmap_service_module_Wind_Response_1_0;

/// Serialize an instance into the provided buffer.
/// The lifetime of the resulting serialized representation is independent of the original instance.
/// This method may be slow for large objects (e.g., images, point clouds, radar samples), so in a later revision
/// we may define a zero-copy alternative that keeps references to the original object where possible.
///
/// @param obj      The object to serialize.
///
/// @param buffer   The destination buffer. There are no alignment requirements.
///                 @see rmap_service_module_Wind_Response_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_
///
/// @param inout_buffer_size_bytes  When calling, this is a pointer to the size of the buffer in bytes.
///                                 Upon return this value will be updated with the size of the constructed serialized
///                                 representation (in bytes); this value is then to be passed over to the transport
///                                 layer. In case of error this value is undefined.
///
/// @returns Negative on error, zero on success.
static inline int8_t rmap_service_module_Wind_Response_1_0_serialize_(
    const rmap_service_module_Wind_Response_1_0* const obj, uint8_t* const buffer,  size_t* const inout_buffer_size_bytes)
{
    if ((obj == NULL) || (buffer == NULL) || (inout_buffer_size_bytes == NULL))
    {
        return -NUNAVUT_ERROR_INVALID_ARGUMENT;
    }


    const size_t capacity_bytes = *inout_buffer_size_bytes;
    if ((8U * (size_t) capacity_bytes) < 984UL)
    {
        return -NUNAVUT_ERROR_SERIALIZATION_BUFFER_TOO_SMALL;
    }
    // Notice that fields that are not an integer number of bytes long may overrun the space allocated for them
    // in the serialization buffer up to the next byte boundary. This is by design and is guaranteed to be safe.
    size_t offset_bits = 0U;





    {   // saturated uint4 state
        uint8_t _sat0_ = obj->state;
        if (_sat0_ > 15U)
        {
            _sat0_ = 15U;
        }
        buffer[offset_bits / 8U] = (uint8_t)(_sat0_);  // C std, 6.3.1.3 Signed and unsigned integers
        offset_bits += 4U;
    }




    {   // saturated uint8 version
        // Saturation code not emitted -- native representation matches the serialized representation.
        const int8_t _err3_ = nunavutSetUxx(&buffer[0], capacity_bytes, offset_bits, obj->version, 8U);
        if (_err3_ < 0)
        {
            return _err3_;
        }
        offset_bits += 8U;
    }




    {   // saturated uint8 revision
        // Saturation code not emitted -- native representation matches the serialized representation.
        const int8_t _err4_ = nunavutSetUxx(&buffer[0], capacity_bytes, offset_bits, obj->revision, 8U);
        if (_err4_ < 0)
        {
            return _err4_;
        }
        offset_bits += 8U;
    }




    {   // saturated bool is_windsonic_unit_error
        if (obj->is_windsonic_unit_error)
        {
            buffer[offset_bits / 8U] = (uint8_t)(buffer[offset_bits / 8U] | (1U << (offset_bits % 8U)));
        }
        else
        {
            buffer[offset_bits / 8U] = (uint8_t)(buffer[offset_bits / 8U] & ~(1U << (offset_bits % 8U)));
        }
        offset_bits += 1U;
    }




    {   // saturated bool is_windsonic_hardware_error
        if (obj->is_windsonic_hardware_error)
        {
            buffer[offset_bits / 8U] = (uint8_t)(buffer[offset_bits / 8U] | (1U << (offset_bits % 8U)));
        }
        else
        {
            buffer[offset_bits / 8U] = (uint8_t)(buffer[offset_bits / 8U] & ~(1U << (offset_bits % 8U)));
        }
        offset_bits += 1U;
    }




    {   // saturated bool is_windsonic_responding_error
        if (obj->is_windsonic_responding_error)
        {
            buffer[offset_bits / 8U] = (uint8_t)(buffer[offset_bits / 8U] | (1U << (offset_bits % 8U)));
        }
        else
        {
            buffer[offset_bits / 8U] = (uint8_t)(buffer[offset_bits / 8U] & ~(1U << (offset_bits % 8U)));
        }
        offset_bits += 1U;
    }




    {   // saturated bool is_windsonic_axis_error
        if (obj->is_windsonic_axis_error)
        {
            buffer[offset_bits / 8U] = (uint8_t)(buffer[offset_bits / 8U] | (1U << (offset_bits % 8U)));
        }
        else
        {
            buffer[offset_bits / 8U] = (uint8_t)(buffer[offset_bits / 8U] & ~(1U << (offset_bits % 8U)));
        }
        offset_bits += 1U;
    }




    {   // saturated bool is_windsonic_crc_error
        buffer[offset_bits / 8U] = obj->is_windsonic_crc_error ? 1U : 0U;
        offset_bits += 1U;
    }




    {   // saturated uint7 perc_rs232_error
        uint8_t _sat1_ = obj->perc_rs232_error;
        if (_sat1_ > 127U)
        {
            _sat1_ = 127U;
        }
        const int8_t _err5_ = nunavutSetUxx(&buffer[0], capacity_bytes, offset_bits, _sat1_, 7U);
        if (_err5_ < 0)
        {
            return _err5_;
        }
        offset_bits += 7U;
    }




    {   // saturated uint8 rbt_event
        // Saturation code not emitted -- native representation matches the serialized representation.
        buffer[offset_bits / 8U] = (uint8_t)(obj->rbt_event);  // C std, 6.3.1.3 Signed and unsigned integers
        offset_bits += 8U;
    }




    {   // saturated uint8 wdt_event
        // Saturation code not emitted -- native representation matches the serialized representation.
        buffer[offset_bits / 8U] = (uint8_t)(obj->wdt_event);  // C std, 6.3.1.3 Signed and unsigned integers
        offset_bits += 8U;
    }


    if (offset_bits % 8U != 0U)  // Pad to 8 bits. TODO: Eliminate redundant padding checks.
    {
        const uint8_t _pad1_ = (uint8_t)(8U - offset_bits % 8U);
        const int8_t _err6_ = nunavutSetUxx(&buffer[0], capacity_bytes, offset_bits, 0U, _pad1_);  // Optimize?
        if (_err6_ < 0)
        {
            return _err6_;
        }
        offset_bits += _pad1_;
    }

    {   // rmap.sensors.WindAvgVect10.1.0 DWA
        size_t _size_bytes2_ = 19UL;  // Nested object (max) size, in bytes.
        int8_t _err7_ = rmap_sensors_WindAvgVect10_1_0_serialize_(
            &obj->DWA, &buffer[offset_bits / 8U], &_size_bytes2_);
        if (_err7_ < 0)
        {
            return _err7_;
        }
        // It is assumed that we know the exact type of the serialized entity, hence we expect the size to match.
        offset_bits += _size_bytes2_ * 8U;  // Advance by the size of the nested object.
    }


    if (offset_bits % 8U != 0U)  // Pad to 8 bits. TODO: Eliminate redundant padding checks.
    {
        const uint8_t _pad2_ = (uint8_t)(8U - offset_bits % 8U);
        const int8_t _err8_ = nunavutSetUxx(&buffer[0], capacity_bytes, offset_bits, 0U, _pad2_);  // Optimize?
        if (_err8_ < 0)
        {
            return _err8_;
        }
        offset_bits += _pad2_;
    }

    {   // rmap.sensors.WindAvgVect.1.0 DWB
        size_t _size_bytes3_ = 19UL;  // Nested object (max) size, in bytes.
        int8_t _err9_ = rmap_sensors_WindAvgVect_1_0_serialize_(
            &obj->DWB, &buffer[offset_bits / 8U], &_size_bytes3_);
        if (_err9_ < 0)
        {
            return _err9_;
        }
        // It is assumed that we know the exact type of the serialized entity, hence we expect the size to match.
        offset_bits += _size_bytes3_ * 8U;  // Advance by the size of the nested object.
    }


    if (offset_bits % 8U != 0U)  // Pad to 8 bits. TODO: Eliminate redundant padding checks.
    {
        const uint8_t _pad3_ = (uint8_t)(8U - offset_bits % 8U);
        const int8_t _err10_ = nunavutSetUxx(&buffer[0], capacity_bytes, offset_bits, 0U, _pad3_);  // Optimize?
        if (_err10_ < 0)
        {
            return _err10_;
        }
        offset_bits += _pad3_;
    }

    {   // rmap.sensors.WindGustSpeed.1.0 DWC
        size_t _size_bytes4_ = 19UL;  // Nested object (max) size, in bytes.
        int8_t _err11_ = rmap_sensors_WindGustSpeed_1_0_serialize_(
            &obj->DWC, &buffer[offset_bits / 8U], &_size_bytes4_);
        if (_err11_ < 0)
        {
            return _err11_;
        }
        // It is assumed that we know the exact type of the serialized entity, hence we expect the size to match.
        offset_bits += _size_bytes4_ * 8U;  // Advance by the size of the nested object.
    }


    if (offset_bits % 8U != 0U)  // Pad to 8 bits. TODO: Eliminate redundant padding checks.
    {
        const uint8_t _pad4_ = (uint8_t)(8U - offset_bits % 8U);
        const int8_t _err12_ = nunavutSetUxx(&buffer[0], capacity_bytes, offset_bits, 0U, _pad4_);  // Optimize?
        if (_err12_ < 0)
        {
            return _err12_;
        }
        offset_bits += _pad4_;
    }

    {   // rmap.sensors.WindAvgSpeed.1.0 DWD
        size_t _size_bytes5_ = 16UL;  // Nested object (max) size, in bytes.
        int8_t _err13_ = rmap_sensors_WindAvgSpeed_1_0_serialize_(
            &obj->DWD, &buffer[offset_bits / 8U], &_size_bytes5_);
        if (_err13_ < 0)
        {
            return _err13_;
        }
        // It is assumed that we know the exact type of the serialized entity, hence we expect the size to match.
        offset_bits += _size_bytes5_ * 8U;  // Advance by the size of the nested object.
    }


    if (offset_bits % 8U != 0U)  // Pad to 8 bits. TODO: Eliminate redundant padding checks.
    {
        const uint8_t _pad5_ = (uint8_t)(8U - offset_bits % 8U);
        const int8_t _err14_ = nunavutSetUxx(&buffer[0], capacity_bytes, offset_bits, 0U, _pad5_);  // Optimize?
        if (_err14_ < 0)
        {
            return _err14_;
        }
        offset_bits += _pad5_;
    }

    {   // rmap.sensors.WindClassSpeed.1.0 DWE
        size_t _size_bytes6_ = 25UL;  // Nested object (max) size, in bytes.
        int8_t _err15_ = rmap_sensors_WindClassSpeed_1_0_serialize_(
            &obj->DWE, &buffer[offset_bits / 8U], &_size_bytes6_);
        if (_err15_ < 0)
        {
            return _err15_;
        }
        // It is assumed that we know the exact type of the serialized entity, hence we expect the size to match.
        offset_bits += _size_bytes6_ * 8U;  // Advance by the size of the nested object.
    }


    if (offset_bits % 8U != 0U)  // Pad to 8 bits. TODO: Eliminate redundant padding checks.
    {
        const uint8_t _pad6_ = (uint8_t)(8U - offset_bits % 8U);
        const int8_t _err16_ = nunavutSetUxx(&buffer[0], capacity_bytes, offset_bits, 0U, _pad6_);  // Optimize?
        if (_err16_ < 0)
        {
            return _err16_;
        }
        offset_bits += _pad6_;
    }

    {   // rmap.sensors.WindGustDirection.1.0 DWF
        size_t _size_bytes7_ = 19UL;  // Nested object (max) size, in bytes.
        int8_t _err17_ = rmap_sensors_WindGustDirection_1_0_serialize_(
            &obj->DWF, &buffer[offset_bits / 8U], &_size_bytes7_);
        if (_err17_ < 0)
        {
            return _err17_;
        }
        // It is assumed that we know the exact type of the serialized entity, hence we expect the size to match.
        offset_bits += _size_bytes7_ * 8U;  // Advance by the size of the nested object.
    }


    if (offset_bits % 8U != 0U)  // Pad to 8 bits. TODO: Eliminate redundant padding checks.
    {
        const uint8_t _pad7_ = (uint8_t)(8U - offset_bits % 8U);
        const int8_t _err18_ = nunavutSetUxx(&buffer[0], capacity_bytes, offset_bits, 0U, _pad7_);  // Optimize?
        if (_err18_ < 0)
        {
            return _err18_;
        }
        offset_bits += _pad7_;
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
static inline int8_t rmap_service_module_Wind_Response_1_0_deserialize_(
    rmap_service_module_Wind_Response_1_0* const out_obj, const uint8_t* buffer, size_t* const inout_buffer_size_bytes)
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





    // saturated uint4 state
    if ((offset_bits + 4U) <= capacity_bits)
    {
        out_obj->state = buffer[offset_bits / 8U] & 15U;
    }
    else
    {
        out_obj->state = 0U;
    }
    offset_bits += 4U;




    // saturated uint8 version
    out_obj->version = nunavutGetU8(&buffer[0], capacity_bytes, offset_bits, 8);
    offset_bits += 8U;




    // saturated uint8 revision
    out_obj->revision = nunavutGetU8(&buffer[0], capacity_bytes, offset_bits, 8);
    offset_bits += 8U;




    // saturated bool is_windsonic_unit_error
    if (offset_bits < capacity_bits)
    {
        out_obj->is_windsonic_unit_error = (buffer[offset_bits / 8U] & (1U << (offset_bits % 8U))) != 0U;
    }
    else
    {
        out_obj->is_windsonic_unit_error = false;
    }
    offset_bits += 1U;




    // saturated bool is_windsonic_hardware_error
    if (offset_bits < capacity_bits)
    {
        out_obj->is_windsonic_hardware_error = (buffer[offset_bits / 8U] & (1U << (offset_bits % 8U))) != 0U;
    }
    else
    {
        out_obj->is_windsonic_hardware_error = false;
    }
    offset_bits += 1U;




    // saturated bool is_windsonic_responding_error
    if (offset_bits < capacity_bits)
    {
        out_obj->is_windsonic_responding_error = (buffer[offset_bits / 8U] & (1U << (offset_bits % 8U))) != 0U;
    }
    else
    {
        out_obj->is_windsonic_responding_error = false;
    }
    offset_bits += 1U;




    // saturated bool is_windsonic_axis_error
    if (offset_bits < capacity_bits)
    {
        out_obj->is_windsonic_axis_error = (buffer[offset_bits / 8U] & (1U << (offset_bits % 8U))) != 0U;
    }
    else
    {
        out_obj->is_windsonic_axis_error = false;
    }
    offset_bits += 1U;




    // saturated bool is_windsonic_crc_error
    if (offset_bits < capacity_bits)
    {
        out_obj->is_windsonic_crc_error = (buffer[offset_bits / 8U] & 1U) != 0U;
    }
    else
    {
        out_obj->is_windsonic_crc_error = false;
    }
    offset_bits += 1U;




    // saturated uint7 perc_rs232_error
    out_obj->perc_rs232_error = nunavutGetU8(&buffer[0], capacity_bytes, offset_bits, 7);
    offset_bits += 7U;




    // saturated uint8 rbt_event
    if ((offset_bits + 8U) <= capacity_bits)
    {
        out_obj->rbt_event = buffer[offset_bits / 8U] & 255U;
    }
    else
    {
        out_obj->rbt_event = 0U;
    }
    offset_bits += 8U;




    // saturated uint8 wdt_event
    if ((offset_bits + 8U) <= capacity_bits)
    {
        out_obj->wdt_event = buffer[offset_bits / 8U] & 255U;
    }
    else
    {
        out_obj->wdt_event = 0U;
    }
    offset_bits += 8U;


    offset_bits = (offset_bits + 7U) & ~(size_t) 7U;  // Align on 8 bits.

    // rmap.sensors.WindAvgVect10.1.0 DWA
    {
        size_t _size_bytes8_ = (size_t)(capacity_bytes - nunavutChooseMin((offset_bits / 8U), capacity_bytes));
        const int8_t _err19_ = rmap_sensors_WindAvgVect10_1_0_deserialize_(
            &out_obj->DWA, &buffer[offset_bits / 8U], &_size_bytes8_);
        if (_err19_ < 0)
        {
            return _err19_;
        }
        offset_bits += _size_bytes8_ * 8U;  // Advance by the size of the nested serialized representation.
    }


    offset_bits = (offset_bits + 7U) & ~(size_t) 7U;  // Align on 8 bits.

    // rmap.sensors.WindAvgVect.1.0 DWB
    {
        size_t _size_bytes9_ = (size_t)(capacity_bytes - nunavutChooseMin((offset_bits / 8U), capacity_bytes));
        const int8_t _err20_ = rmap_sensors_WindAvgVect_1_0_deserialize_(
            &out_obj->DWB, &buffer[offset_bits / 8U], &_size_bytes9_);
        if (_err20_ < 0)
        {
            return _err20_;
        }
        offset_bits += _size_bytes9_ * 8U;  // Advance by the size of the nested serialized representation.
    }


    offset_bits = (offset_bits + 7U) & ~(size_t) 7U;  // Align on 8 bits.

    // rmap.sensors.WindGustSpeed.1.0 DWC
    {
        size_t _size_bytes10_ = (size_t)(capacity_bytes - nunavutChooseMin((offset_bits / 8U), capacity_bytes));
        const int8_t _err21_ = rmap_sensors_WindGustSpeed_1_0_deserialize_(
            &out_obj->DWC, &buffer[offset_bits / 8U], &_size_bytes10_);
        if (_err21_ < 0)
        {
            return _err21_;
        }
        offset_bits += _size_bytes10_ * 8U;  // Advance by the size of the nested serialized representation.
    }


    offset_bits = (offset_bits + 7U) & ~(size_t) 7U;  // Align on 8 bits.

    // rmap.sensors.WindAvgSpeed.1.0 DWD
    {
        size_t _size_bytes11_ = (size_t)(capacity_bytes - nunavutChooseMin((offset_bits / 8U), capacity_bytes));
        const int8_t _err22_ = rmap_sensors_WindAvgSpeed_1_0_deserialize_(
            &out_obj->DWD, &buffer[offset_bits / 8U], &_size_bytes11_);
        if (_err22_ < 0)
        {
            return _err22_;
        }
        offset_bits += _size_bytes11_ * 8U;  // Advance by the size of the nested serialized representation.
    }


    offset_bits = (offset_bits + 7U) & ~(size_t) 7U;  // Align on 8 bits.

    // rmap.sensors.WindClassSpeed.1.0 DWE
    {
        size_t _size_bytes12_ = (size_t)(capacity_bytes - nunavutChooseMin((offset_bits / 8U), capacity_bytes));
        const int8_t _err23_ = rmap_sensors_WindClassSpeed_1_0_deserialize_(
            &out_obj->DWE, &buffer[offset_bits / 8U], &_size_bytes12_);
        if (_err23_ < 0)
        {
            return _err23_;
        }
        offset_bits += _size_bytes12_ * 8U;  // Advance by the size of the nested serialized representation.
    }


    offset_bits = (offset_bits + 7U) & ~(size_t) 7U;  // Align on 8 bits.

    // rmap.sensors.WindGustDirection.1.0 DWF
    {
        size_t _size_bytes13_ = (size_t)(capacity_bytes - nunavutChooseMin((offset_bits / 8U), capacity_bytes));
        const int8_t _err24_ = rmap_sensors_WindGustDirection_1_0_deserialize_(
            &out_obj->DWF, &buffer[offset_bits / 8U], &_size_bytes13_);
        if (_err24_ < 0)
        {
            return _err24_;
        }
        offset_bits += _size_bytes13_ * 8U;  // Advance by the size of the nested serialized representation.
    }


    offset_bits = (offset_bits + 7U) & ~(size_t) 7U;  // Align on 8 bits.

    *inout_buffer_size_bytes = (size_t) (nunavutChooseMin(offset_bits, capacity_bits) / 8U);


    return NUNAVUT_SUCCESS;
}

/// Initialize an instance to default values. Does nothing if @param out_obj is NULL.
/// This function intentionally leaves inactive elements uninitialized; for example, members of a variable-length
/// array beyond its length are left uninitialized; aliased union memory that is not used by the first union field
/// is left uninitialized, etc. If full zero-initialization is desired, just use memset(&obj, 0, sizeof(obj)).
static inline void rmap_service_module_Wind_Response_1_0_initialize_(rmap_service_module_Wind_Response_1_0* const out_obj)
{
    if (out_obj != NULL)
    {
        size_t size_bytes = 0;
        const uint8_t buf = 0;
        const int8_t err = rmap_service_module_Wind_Response_1_0_deserialize_(out_obj, &buf, &size_bytes);

        (void) err;
    }
}



#ifdef __cplusplus
}
#endif
#endif // RMAP_SERVICE_MODULE_WIND_1_0_INCLUDED_
