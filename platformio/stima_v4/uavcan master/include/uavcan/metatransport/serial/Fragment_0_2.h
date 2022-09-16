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
// Source file:   C:/Users/Cristiano/Documents/stima-v4/stimav4-test/uavcan master/data_types/uavcan/metatransport/serial/Fragment.0.2.dsdl
// Generated at:  2022-09-16 10:19:43.303439 UTC
// Is deprecated: no
// Fixed port-ID: None
// Full name:     uavcan.metatransport.serial.Fragment
// Version:       0.2
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

#ifndef UAVCAN_METATRANSPORT_SERIAL_FRAGMENT_0_2_INCLUDED_
#define UAVCAN_METATRANSPORT_SERIAL_FRAGMENT_0_2_INCLUDED_

#include <nunavut/support/serialization.h>
#include <stdint.h>
#include <stdlib.h>

static_assert( NUNAVUT_SUPPORT_LANGUAGE_OPTION_TARGET_ENDIANNESS == 434322821,
              "C:/Users/Cristiano/Documents/stima-v4/stimav4-test/uavcan master/data_types/uavcan/metatransport/serial/Fragment.0.2.dsdl is trying to use a serialization library that was compiled with "
              "different language options. This is dangerous and therefore not allowed." );
static_assert( NUNAVUT_SUPPORT_LANGUAGE_OPTION_OMIT_FLOAT_SERIALIZATION_SUPPORT == 0,
              "C:/Users/Cristiano/Documents/stima-v4/stimav4-test/uavcan master/data_types/uavcan/metatransport/serial/Fragment.0.2.dsdl is trying to use a serialization library that was compiled with "
              "different language options. This is dangerous and therefore not allowed." );
static_assert( NUNAVUT_SUPPORT_LANGUAGE_OPTION_ENABLE_SERIALIZATION_ASSERTS == 0,
              "C:/Users/Cristiano/Documents/stima-v4/stimav4-test/uavcan master/data_types/uavcan/metatransport/serial/Fragment.0.2.dsdl is trying to use a serialization library that was compiled with "
              "different language options. This is dangerous and therefore not allowed." );
static_assert( NUNAVUT_SUPPORT_LANGUAGE_OPTION_ENABLE_OVERRIDE_VARIABLE_ARRAY_CAPACITY == 0,
              "C:/Users/Cristiano/Documents/stima-v4/stimav4-test/uavcan master/data_types/uavcan/metatransport/serial/Fragment.0.2.dsdl is trying to use a serialization library that was compiled with "
              "different language options. This is dangerous and therefore not allowed." );
static_assert( NUNAVUT_SUPPORT_LANGUAGE_OPTION_CAST_FORMAT == 2368206204,
              "C:/Users/Cristiano/Documents/stima-v4/stimav4-test/uavcan master/data_types/uavcan/metatransport/serial/Fragment.0.2.dsdl is trying to use a serialization library that was compiled with "
              "different language options. This is dangerous and therefore not allowed." );

#ifdef __cplusplus
extern "C" {
#endif

/// This type does not have a fixed port-ID. See https://forum.opencyphal.org/t/choosing-message-and-service-ids/889
#define uavcan_metatransport_serial_Fragment_0_2_HAS_FIXED_PORT_ID_ false

#define uavcan_metatransport_serial_Fragment_0_2_FULL_NAME_             "uavcan.metatransport.serial.Fragment"
#define uavcan_metatransport_serial_Fragment_0_2_FULL_NAME_AND_VERSION_ "uavcan.metatransport.serial.Fragment.0.2"

/// Extent is the minimum amount of memory required to hold any serialized representation of any compatible
/// version of the data type; or, on other words, it is the the maximum possible size of received objects of this type.
/// The size is specified in bytes (rather than bits) because by definition, extent is an integer number of bytes long.
/// When allocating a deserialization (RX) buffer for this data type, it should be at least extent bytes large.
/// When allocating a serialization (TX) buffer, it is safe to use the size of the largest serialized representation
/// instead of the extent because it provides a tighter bound of the object size; it is safe because the concrete type
/// is always known during serialization (unlike deserialization). If not sure, use extent everywhere.
#define uavcan_metatransport_serial_Fragment_0_2_EXTENT_BYTES_                    2050UL
#define uavcan_metatransport_serial_Fragment_0_2_SERIALIZATION_BUFFER_SIZE_BYTES_ 2050UL
static_assert(uavcan_metatransport_serial_Fragment_0_2_EXTENT_BYTES_ >= uavcan_metatransport_serial_Fragment_0_2_SERIALIZATION_BUFFER_SIZE_BYTES_,
              "Internal constraint violation");

/// saturated uint12 CAPACITY_BYTES = 2048
#define uavcan_metatransport_serial_Fragment_0_2_CAPACITY_BYTES (2048U)

/// Array metadata for: saturated uint8[<=2048] data
#define uavcan_metatransport_serial_Fragment_0_2_data_ARRAY_CAPACITY_           2048U
#define uavcan_metatransport_serial_Fragment_0_2_data_ARRAY_IS_VARIABLE_LENGTH_ true

typedef struct
{
    /// saturated uint8[<=2048] data
    struct  /// Array address equivalence guarantee: &elements[0] == &data
    {
        uint8_t elements[uavcan_metatransport_serial_Fragment_0_2_data_ARRAY_CAPACITY_];
        size_t count;
    } data;
} uavcan_metatransport_serial_Fragment_0_2;

/// Serialize an instance into the provided buffer.
/// The lifetime of the resulting serialized representation is independent of the original instance.
/// This method may be slow for large objects (e.g., images, point clouds, radar samples), so in a later revision
/// we may define a zero-copy alternative that keeps references to the original object where possible.
///
/// @param obj      The object to serialize.
///
/// @param buffer   The destination buffer. There are no alignment requirements.
///                 @see uavcan_metatransport_serial_Fragment_0_2_SERIALIZATION_BUFFER_SIZE_BYTES_
///
/// @param inout_buffer_size_bytes  When calling, this is a pointer to the size of the buffer in bytes.
///                                 Upon return this value will be updated with the size of the constructed serialized
///                                 representation (in bytes); this value is then to be passed over to the transport
///                                 layer. In case of error this value is undefined.
///
/// @returns Negative on error, zero on success.
static inline int8_t uavcan_metatransport_serial_Fragment_0_2_serialize_(
    const uavcan_metatransport_serial_Fragment_0_2* const obj, uint8_t* const buffer,  size_t* const inout_buffer_size_bytes)
{
    if ((obj == NULL) || (buffer == NULL) || (inout_buffer_size_bytes == NULL))
    {
        return -NUNAVUT_ERROR_INVALID_ARGUMENT;
    }


    const size_t capacity_bytes = *inout_buffer_size_bytes;
    if ((8U * (size_t) capacity_bytes) < 16400UL)
    {
        return -NUNAVUT_ERROR_SERIALIZATION_BUFFER_TOO_SMALL;
    }
    // Notice that fields that are not an integer number of bytes long may overrun the space allocated for them
    // in the serialization buffer up to the next byte boundary. This is by design and is guaranteed to be safe.
    size_t offset_bits = 0U;





    {   // saturated uint8[<=2048] data
        if (obj->data.count > 2048)
        {
            return -NUNAVUT_ERROR_REPRESENTATION_BAD_ARRAY_LENGTH;
        }
        // Array length prefix: truncated uint16
        (void) memmove(&buffer[offset_bits / 8U], &obj->data.count, 2U);
        offset_bits += 16U;
        // Optimization prospect: this item is aligned at the byte boundary, so it is possible to use memmove().
        nunavutCopyBits(&buffer[0], offset_bits, obj->data.count * 8U, &obj->data.elements[0], 0U);
        offset_bits += obj->data.count * 8U;
    }


    if (offset_bits % 8U != 0U)  // Pad to 8 bits. TODO: Eliminate redundant padding checks.
    {
        const uint8_t _pad0_ = (uint8_t)(8U - offset_bits % 8U);
        const int8_t _err0_ = nunavutSetUxx(&buffer[0], capacity_bytes, offset_bits, 0U, _pad0_);  // Optimize?
        if (_err0_ < 0)
        {
            return _err0_;
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
static inline int8_t uavcan_metatransport_serial_Fragment_0_2_deserialize_(
    uavcan_metatransport_serial_Fragment_0_2* const out_obj, const uint8_t* buffer, size_t* const inout_buffer_size_bytes)
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





    // saturated uint8[<=2048] data
    // Array length prefix: truncated uint16
    out_obj->data.count = nunavutGetU16(&buffer[0], capacity_bytes, offset_bits, 16);
    offset_bits += 16U;
    if (out_obj->data.count > 2048U)
    {
        return -NUNAVUT_ERROR_REPRESENTATION_BAD_ARRAY_LENGTH;
    }
    nunavutGetBits(&out_obj->data.elements[0], &buffer[0], capacity_bytes, offset_bits, out_obj->data.count * 8U);
    offset_bits += out_obj->data.count * 8U;


    offset_bits = (offset_bits + 7U) & ~(size_t) 7U;  // Align on 8 bits.

    *inout_buffer_size_bytes = (size_t) (nunavutChooseMin(offset_bits, capacity_bits) / 8U);


    return NUNAVUT_SUCCESS;
}

/// Initialize an instance to default values. Does nothing if @param out_obj is NULL.
/// This function intentionally leaves inactive elements uninitialized; for example, members of a variable-length
/// array beyond its length are left uninitialized; aliased union memory that is not used by the first union field
/// is left uninitialized, etc. If full zero-initialization is desired, just use memset(&obj, 0, sizeof(obj)).
static inline void uavcan_metatransport_serial_Fragment_0_2_initialize_(uavcan_metatransport_serial_Fragment_0_2* const out_obj)
{
    if (out_obj != NULL)
    {
        size_t size_bytes = 0;
        const uint8_t buf = 0;
        const int8_t err = uavcan_metatransport_serial_Fragment_0_2_deserialize_(out_obj, &buf, &size_bytes);

        (void) err;
    }
}



#ifdef __cplusplus
}
#endif
#endif // UAVCAN_METATRANSPORT_SERIAL_FRAGMENT_0_2_INCLUDED_
