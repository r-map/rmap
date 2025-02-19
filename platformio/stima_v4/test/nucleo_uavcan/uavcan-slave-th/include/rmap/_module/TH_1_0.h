// This is an AUTO-GENERATED UAVCAN DSDL data type implementation. Curious? See https://opencyphal.org.
// You shouldn't attempt to edit this file.
//
// Checking this file under version control is not recommended unless it is used as part of a high-SIL
// safety-critical codebase. The typical usage scenario is to generate it as part of the build process.
//
// To avoid conflicts with definitions given in the source DSDL file, all entities created by the code generator
// are named with an underscore at the end, like foo_bar_().
//
// Generator:     nunavut-1.8.2 (serialization was enabled)
// Source file:   C:/Users/Cristiano/Documents/stima-v4/stimav4-test/uavcan slave-th/data_types/rmap/module/TH.1.0.dsdl
// Generated at:  2022-09-16 10:13:42.459906 UTC
// Is deprecated: no
// Fixed port-ID: None
// Full name:     rmap.module.TH
// Version:       1.0
//
// Platform
//     python_implementation:  CPython
//     python_version:  3.10.2
//     python_release_level:  final
//     python_build:  ('tags/v3.10.2:a58ebcc', 'Jan 17 2022 14:12:15')
//     python_compiler:  MSC v.1929 64 bit (AMD64)
//     python_revision:  a58ebcc
//     python_xoptions:  {}
//     runtime_platform:  Windows-10-10.0.19044-SP0
//
// Language Options
//     target_endianness:  little
//     omit_float_serialization_support:  False
//     enable_serialization_asserts:  False
//     enable_override_variable_array_capacity:  False
//     cast_format:  (({type}) {value})

#ifndef RMAP_MODULE_TH_1_0_INCLUDED_
#define RMAP_MODULE_TH_1_0_INCLUDED_

#include <nunavut/support/serialization.h>
#include <rmap/sensors/TH_1_0.h>
#include <stdlib.h>

static_assert( NUNAVUT_SUPPORT_LANGUAGE_OPTION_TARGET_ENDIANNESS == 434322821,
              "C:/Users/Cristiano/Documents/stima-v4/stimav4-test/uavcan slave-th/data_types/rmap/module/TH.1.0.dsdl is trying to use a serialization library that was compiled with "
              "different language options. This is dangerous and therefore not allowed." );
static_assert( NUNAVUT_SUPPORT_LANGUAGE_OPTION_OMIT_FLOAT_SERIALIZATION_SUPPORT == 0,
              "C:/Users/Cristiano/Documents/stima-v4/stimav4-test/uavcan slave-th/data_types/rmap/module/TH.1.0.dsdl is trying to use a serialization library that was compiled with "
              "different language options. This is dangerous and therefore not allowed." );
static_assert( NUNAVUT_SUPPORT_LANGUAGE_OPTION_ENABLE_SERIALIZATION_ASSERTS == 0,
              "C:/Users/Cristiano/Documents/stima-v4/stimav4-test/uavcan slave-th/data_types/rmap/module/TH.1.0.dsdl is trying to use a serialization library that was compiled with "
              "different language options. This is dangerous and therefore not allowed." );
static_assert( NUNAVUT_SUPPORT_LANGUAGE_OPTION_ENABLE_OVERRIDE_VARIABLE_ARRAY_CAPACITY == 0,
              "C:/Users/Cristiano/Documents/stima-v4/stimav4-test/uavcan slave-th/data_types/rmap/module/TH.1.0.dsdl is trying to use a serialization library that was compiled with "
              "different language options. This is dangerous and therefore not allowed." );
static_assert( NUNAVUT_SUPPORT_LANGUAGE_OPTION_CAST_FORMAT == 2368206204,
              "C:/Users/Cristiano/Documents/stima-v4/stimav4-test/uavcan slave-th/data_types/rmap/module/TH.1.0.dsdl is trying to use a serialization library that was compiled with "
              "different language options. This is dangerous and therefore not allowed." );

#ifdef __cplusplus
extern "C" {
#endif

/// This type does not have a fixed port-ID. See https://forum.opencyphal.org/t/choosing-message-and-service-ids/889
#define rmap_module_TH_1_0_HAS_FIXED_PORT_ID_ false

#define rmap_module_TH_1_0_FULL_NAME_             "rmap.module.TH"
#define rmap_module_TH_1_0_FULL_NAME_AND_VERSION_ "rmap.module.TH.1.0"

/// Extent is the minimum amount of memory required to hold any serialized representation of any compatible
/// version of the data type; or, on other words, it is the the maximum possible size of received objects of this type.
/// The size is specified in bytes (rather than bits) because by definition, extent is an integer number of bytes long.
/// When allocating a deserialization (RX) buffer for this data type, it should be at least extent bytes large.
/// When allocating a serialization (TX) buffer, it is safe to use the size of the largest serialized representation
/// instead of the extent because it provides a tighter bound of the object size; it is safe because the concrete type
/// is always known during serialization (unlike deserialization). If not sure, use extent everywhere.
#define rmap_module_TH_1_0_EXTENT_BYTES_                    76UL
#define rmap_module_TH_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_ 76UL
static_assert(rmap_module_TH_1_0_EXTENT_BYTES_ >= rmap_module_TH_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_,
              "Internal constraint violation");

typedef struct
{
    /// rmap.sensors.TH.1.0 ITH
    rmap_sensors_TH_1_0 ITH;

    /// rmap.sensors.TH.1.0 NTH
    rmap_sensors_TH_1_0 NTH;

    /// rmap.sensors.TH.1.0 MTH
    rmap_sensors_TH_1_0 MTH;

    /// rmap.sensors.TH.1.0 XTH
    rmap_sensors_TH_1_0 XTH;
} rmap_module_TH_1_0;

/// Serialize an instance into the provided buffer.
/// The lifetime of the resulting serialized representation is independent of the original instance.
/// This method may be slow for large objects (e.g., images, point clouds, radar samples), so in a later revision
/// we may define a zero-copy alternative that keeps references to the original object where possible.
///
/// @param obj      The object to serialize.
///
/// @param buffer   The destination buffer. There are no alignment requirements.
///                 @see rmap_module_TH_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_
///
/// @param inout_buffer_size_bytes  When calling, this is a pointer to the size of the buffer in bytes.
///                                 Upon return this value will be updated with the size of the constructed serialized
///                                 representation (in bytes); this value is then to be passed over to the transport
///                                 layer. In case of error this value is undefined.
///
/// @returns Negative on error, zero on success.
static inline int8_t rmap_module_TH_1_0_serialize_(
    const rmap_module_TH_1_0* const obj, uint8_t* const buffer,  size_t* const inout_buffer_size_bytes)
{
    if ((obj == NULL) || (buffer == NULL) || (inout_buffer_size_bytes == NULL))
    {
        return -NUNAVUT_ERROR_INVALID_ARGUMENT;
    }


    const size_t capacity_bytes = *inout_buffer_size_bytes;
    if ((8U * (size_t) capacity_bytes) < 608UL)
    {
        return -NUNAVUT_ERROR_SERIALIZATION_BUFFER_TOO_SMALL;
    }
    // Notice that fields that are not an integer number of bytes long may overrun the space allocated for them
    // in the serialization buffer up to the next byte boundary. This is by design and is guaranteed to be safe.
    size_t offset_bits = 0U;





    {   // rmap.sensors.TH.1.0 ITH
        size_t _size_bytes0_ = 19UL;  // Nested object (max) size, in bytes.
        int8_t _err0_ = rmap_sensors_TH_1_0_serialize_(
            &obj->ITH, &buffer[offset_bits / 8U], &_size_bytes0_);
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

    {   // rmap.sensors.TH.1.0 NTH
        size_t _size_bytes1_ = 19UL;  // Nested object (max) size, in bytes.
        int8_t _err2_ = rmap_sensors_TH_1_0_serialize_(
            &obj->NTH, &buffer[offset_bits / 8U], &_size_bytes1_);
        if (_err2_ < 0)
        {
            return _err2_;
        }
        // It is assumed that we know the exact type of the serialized entity, hence we expect the size to match.
        offset_bits += _size_bytes1_ * 8U;  // Advance by the size of the nested object.
    }


    if (offset_bits % 8U != 0U)  // Pad to 8 bits. TODO: Eliminate redundant padding checks.
    {
        const uint8_t _pad1_ = (uint8_t)(8U - offset_bits % 8U);
        const int8_t _err3_ = nunavutSetUxx(&buffer[0], capacity_bytes, offset_bits, 0U, _pad1_);  // Optimize?
        if (_err3_ < 0)
        {
            return _err3_;
        }
        offset_bits += _pad1_;
    }

    {   // rmap.sensors.TH.1.0 MTH
        size_t _size_bytes2_ = 19UL;  // Nested object (max) size, in bytes.
        int8_t _err4_ = rmap_sensors_TH_1_0_serialize_(
            &obj->MTH, &buffer[offset_bits / 8U], &_size_bytes2_);
        if (_err4_ < 0)
        {
            return _err4_;
        }
        // It is assumed that we know the exact type of the serialized entity, hence we expect the size to match.
        offset_bits += _size_bytes2_ * 8U;  // Advance by the size of the nested object.
    }


    if (offset_bits % 8U != 0U)  // Pad to 8 bits. TODO: Eliminate redundant padding checks.
    {
        const uint8_t _pad2_ = (uint8_t)(8U - offset_bits % 8U);
        const int8_t _err5_ = nunavutSetUxx(&buffer[0], capacity_bytes, offset_bits, 0U, _pad2_);  // Optimize?
        if (_err5_ < 0)
        {
            return _err5_;
        }
        offset_bits += _pad2_;
    }

    {   // rmap.sensors.TH.1.0 XTH
        size_t _size_bytes3_ = 19UL;  // Nested object (max) size, in bytes.
        int8_t _err6_ = rmap_sensors_TH_1_0_serialize_(
            &obj->XTH, &buffer[offset_bits / 8U], &_size_bytes3_);
        if (_err6_ < 0)
        {
            return _err6_;
        }
        // It is assumed that we know the exact type of the serialized entity, hence we expect the size to match.
        offset_bits += _size_bytes3_ * 8U;  // Advance by the size of the nested object.
    }


    if (offset_bits % 8U != 0U)  // Pad to 8 bits. TODO: Eliminate redundant padding checks.
    {
        const uint8_t _pad3_ = (uint8_t)(8U - offset_bits % 8U);
        const int8_t _err7_ = nunavutSetUxx(&buffer[0], capacity_bytes, offset_bits, 0U, _pad3_);  // Optimize?
        if (_err7_ < 0)
        {
            return _err7_;
        }
        offset_bits += _pad3_;
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
static inline int8_t rmap_module_TH_1_0_deserialize_(
    rmap_module_TH_1_0* const out_obj, const uint8_t* buffer, size_t* const inout_buffer_size_bytes)
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





    // rmap.sensors.TH.1.0 ITH
    {
        size_t _size_bytes4_ = (size_t)(capacity_bytes - nunavutChooseMin((offset_bits / 8U), capacity_bytes));
        const int8_t _err8_ = rmap_sensors_TH_1_0_deserialize_(
            &out_obj->ITH, &buffer[offset_bits / 8U], &_size_bytes4_);
        if (_err8_ < 0)
        {
            return _err8_;
        }
        offset_bits += _size_bytes4_ * 8U;  // Advance by the size of the nested serialized representation.
    }


    offset_bits = (offset_bits + 7U) & ~(size_t) 7U;  // Align on 8 bits.

    // rmap.sensors.TH.1.0 NTH
    {
        size_t _size_bytes5_ = (size_t)(capacity_bytes - nunavutChooseMin((offset_bits / 8U), capacity_bytes));
        const int8_t _err9_ = rmap_sensors_TH_1_0_deserialize_(
            &out_obj->NTH, &buffer[offset_bits / 8U], &_size_bytes5_);
        if (_err9_ < 0)
        {
            return _err9_;
        }
        offset_bits += _size_bytes5_ * 8U;  // Advance by the size of the nested serialized representation.
    }


    offset_bits = (offset_bits + 7U) & ~(size_t) 7U;  // Align on 8 bits.

    // rmap.sensors.TH.1.0 MTH
    {
        size_t _size_bytes6_ = (size_t)(capacity_bytes - nunavutChooseMin((offset_bits / 8U), capacity_bytes));
        const int8_t _err10_ = rmap_sensors_TH_1_0_deserialize_(
            &out_obj->MTH, &buffer[offset_bits / 8U], &_size_bytes6_);
        if (_err10_ < 0)
        {
            return _err10_;
        }
        offset_bits += _size_bytes6_ * 8U;  // Advance by the size of the nested serialized representation.
    }


    offset_bits = (offset_bits + 7U) & ~(size_t) 7U;  // Align on 8 bits.

    // rmap.sensors.TH.1.0 XTH
    {
        size_t _size_bytes7_ = (size_t)(capacity_bytes - nunavutChooseMin((offset_bits / 8U), capacity_bytes));
        const int8_t _err11_ = rmap_sensors_TH_1_0_deserialize_(
            &out_obj->XTH, &buffer[offset_bits / 8U], &_size_bytes7_);
        if (_err11_ < 0)
        {
            return _err11_;
        }
        offset_bits += _size_bytes7_ * 8U;  // Advance by the size of the nested serialized representation.
    }


    offset_bits = (offset_bits + 7U) & ~(size_t) 7U;  // Align on 8 bits.

    *inout_buffer_size_bytes = (size_t) (nunavutChooseMin(offset_bits, capacity_bits) / 8U);


    return NUNAVUT_SUCCESS;
}

/// Initialize an instance to default values. Does nothing if @param out_obj is NULL.
/// This function intentionally leaves inactive elements uninitialized; for example, members of a variable-length
/// array beyond its length are left uninitialized; aliased union memory that is not used by the first union field
/// is left uninitialized, etc. If full zero-initialization is desired, just use memset(&obj, 0, sizeof(obj)).
static inline void rmap_module_TH_1_0_initialize_(rmap_module_TH_1_0* const out_obj)
{
    if (out_obj != NULL)
    {
        size_t size_bytes = 0;
        const uint8_t buf = 0;
        const int8_t err = rmap_module_TH_1_0_deserialize_(out_obj, &buf, &size_bytes);

        (void) err;
    }
}



#ifdef __cplusplus
}
#endif
#endif // RMAP_MODULE_TH_1_0_INCLUDED_
