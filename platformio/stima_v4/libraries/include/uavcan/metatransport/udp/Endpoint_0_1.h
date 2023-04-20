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
// Source file:   C:/Dati/RMAP/stimav4-rmap/rmap/platformio/stima_v4/libraries/data_types/uavcan/metatransport/udp/Endpoint.0.1.dsdl
// Generated at:  2023-04-20 17:13:35.520481 UTC
// Is deprecated: yes
// Fixed port-ID: None
// Full name:     uavcan.metatransport.udp.Endpoint
// Version:       0.1
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

//           _____  ______ _____  _____  ______ _____       _______ ______ _____
//          |  __ `|  ____|  __ `|  __ `|  ____/ ____|   /`|__   __|  ____|  __ `
//          | |  | | |__  | |__) | |__) | |__ | |       /  `  | |  | |__  | |  | |
//          | |  | |  __| |  ___/|  _  /|  __|| |      / /` ` | |  |  __| | |  | |
//          | |__| | |____| |    | | ` `| |___| |____ / ____ `| |  | |____| |__| |
//          |_____/|______|_|    |_|  `_`______`_____/_/    `_`_|  |______|_____/
//
// WARNING: this data type is deprecated and is nearing the end of its life cycle. Seek replacement.

#ifndef UAVCAN_METATRANSPORT_UDP_ENDPOINT_0_1_INCLUDED_
#define UAVCAN_METATRANSPORT_UDP_ENDPOINT_0_1_INCLUDED_

#include <nunavut/support/serialization.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static_assert( NUNAVUT_SUPPORT_LANGUAGE_OPTION_TARGET_ENDIANNESS == 434322821,
              "C:/Dati/RMAP/stimav4-rmap/rmap/platformio/stima_v4/libraries/data_types/uavcan/metatransport/udp/Endpoint.0.1.dsdl is trying to use a serialization library that was compiled with "
              "different language options. This is dangerous and therefore not allowed." );
static_assert( NUNAVUT_SUPPORT_LANGUAGE_OPTION_OMIT_FLOAT_SERIALIZATION_SUPPORT == 0,
              "C:/Dati/RMAP/stimav4-rmap/rmap/platformio/stima_v4/libraries/data_types/uavcan/metatransport/udp/Endpoint.0.1.dsdl is trying to use a serialization library that was compiled with "
              "different language options. This is dangerous and therefore not allowed." );
static_assert( NUNAVUT_SUPPORT_LANGUAGE_OPTION_ENABLE_SERIALIZATION_ASSERTS == 0,
              "C:/Dati/RMAP/stimav4-rmap/rmap/platformio/stima_v4/libraries/data_types/uavcan/metatransport/udp/Endpoint.0.1.dsdl is trying to use a serialization library that was compiled with "
              "different language options. This is dangerous and therefore not allowed." );
static_assert( NUNAVUT_SUPPORT_LANGUAGE_OPTION_ENABLE_OVERRIDE_VARIABLE_ARRAY_CAPACITY == 0,
              "C:/Dati/RMAP/stimav4-rmap/rmap/platformio/stima_v4/libraries/data_types/uavcan/metatransport/udp/Endpoint.0.1.dsdl is trying to use a serialization library that was compiled with "
              "different language options. This is dangerous and therefore not allowed." );
static_assert( NUNAVUT_SUPPORT_LANGUAGE_OPTION_CAST_FORMAT == 2368206204,
              "C:/Dati/RMAP/stimav4-rmap/rmap/platformio/stima_v4/libraries/data_types/uavcan/metatransport/udp/Endpoint.0.1.dsdl is trying to use a serialization library that was compiled with "
              "different language options. This is dangerous and therefore not allowed." );

#ifdef __cplusplus
extern "C" {
#endif

/// This type does not have a fixed port-ID. See https://forum.opencyphal.org/t/choosing-message-and-service-ids/889
#define uavcan_metatransport_udp_Endpoint_0_1_HAS_FIXED_PORT_ID_ false

#define uavcan_metatransport_udp_Endpoint_0_1_FULL_NAME_             "uavcan.metatransport.udp.Endpoint"
#define uavcan_metatransport_udp_Endpoint_0_1_FULL_NAME_AND_VERSION_ "uavcan.metatransport.udp.Endpoint.0.1"

/// Extent is the minimum amount of memory required to hold any serialized representation of any compatible
/// version of the data type; or, on other words, it is the the maximum possible size of received objects of this type.
/// The size is specified in bytes (rather than bits) because by definition, extent is an integer number of bytes long.
/// When allocating a deserialization (RX) buffer for this data type, it should be at least extent bytes large.
/// When allocating a serialization (TX) buffer, it is safe to use the size of the largest serialized representation
/// instead of the extent because it provides a tighter bound of the object size; it is safe because the concrete type
/// is always known during serialization (unlike deserialization). If not sure, use extent everywhere.
#define uavcan_metatransport_udp_Endpoint_0_1_EXTENT_BYTES_                    32UL
#define uavcan_metatransport_udp_Endpoint_0_1_SERIALIZATION_BUFFER_SIZE_BYTES_ 32UL
static_assert(uavcan_metatransport_udp_Endpoint_0_1_EXTENT_BYTES_ >= uavcan_metatransport_udp_Endpoint_0_1_SERIALIZATION_BUFFER_SIZE_BYTES_,
              "Internal constraint violation");

/// Array metadata for: saturated uint8[16] ip_address
#define uavcan_metatransport_udp_Endpoint_0_1_ip_address_ARRAY_CAPACITY_           16U
#define uavcan_metatransport_udp_Endpoint_0_1_ip_address_ARRAY_IS_VARIABLE_LENGTH_ false
/// Array metadata for: saturated uint8[6] mac_address
#define uavcan_metatransport_udp_Endpoint_0_1_mac_address_ARRAY_CAPACITY_           6U
#define uavcan_metatransport_udp_Endpoint_0_1_mac_address_ARRAY_IS_VARIABLE_LENGTH_ false

typedef struct
{
    /// saturated uint8[16] ip_address
    uint8_t ip_address[16];

    /// saturated uint8[6] mac_address
    uint8_t mac_address[6];

    /// saturated uint16 port
    uint16_t port;
} uavcan_metatransport_udp_Endpoint_0_1;

/// Serialize an instance into the provided buffer.
/// The lifetime of the resulting serialized representation is independent of the original instance.
/// This method may be slow for large objects (e.g., images, point clouds, radar samples), so in a later revision
/// we may define a zero-copy alternative that keeps references to the original object where possible.
///
/// @param obj      The object to serialize.
///
/// @param buffer   The destination buffer. There are no alignment requirements.
///                 @see uavcan_metatransport_udp_Endpoint_0_1_SERIALIZATION_BUFFER_SIZE_BYTES_
///
/// @param inout_buffer_size_bytes  When calling, this is a pointer to the size of the buffer in bytes.
///                                 Upon return this value will be updated with the size of the constructed serialized
///                                 representation (in bytes); this value is then to be passed over to the transport
///                                 layer. In case of error this value is undefined.
///
/// @returns Negative on error, zero on success.
static inline int8_t uavcan_metatransport_udp_Endpoint_0_1_serialize_(
    const uavcan_metatransport_udp_Endpoint_0_1* const obj, uint8_t* const buffer,  size_t* const inout_buffer_size_bytes)
{
    if ((obj == NULL) || (buffer == NULL) || (inout_buffer_size_bytes == NULL))
    {
        return -NUNAVUT_ERROR_INVALID_ARGUMENT;
    }


    const size_t capacity_bytes = *inout_buffer_size_bytes;
    if ((8U * (size_t) capacity_bytes) < 256UL)
    {
        return -NUNAVUT_ERROR_SERIALIZATION_BUFFER_TOO_SMALL;
    }
    // Notice that fields that are not an integer number of bytes long may overrun the space allocated for them
    // in the serialization buffer up to the next byte boundary. This is by design and is guaranteed to be safe.
    size_t offset_bits = 0U;





    {   // saturated uint8[16] ip_address
        // Optimization prospect: this item is aligned at the byte boundary, so it is possible to use memmove().
        nunavutCopyBits(&buffer[0], offset_bits, 16UL * 8U, &obj->ip_address[0], 0U);
        offset_bits += 16UL * 8U;
    }




    {   // saturated uint8[6] mac_address
        // Optimization prospect: this item is aligned at the byte boundary, so it is possible to use memmove().
        nunavutCopyBits(&buffer[0], offset_bits, 6UL * 8U, &obj->mac_address[0], 0U);
        offset_bits += 6UL * 8U;
    }




    {   // saturated uint16 port
        // Saturation code not emitted -- native representation matches the serialized representation.
        (void) memmove(&buffer[offset_bits / 8U], &obj->port, 2U);
        offset_bits += 16U;
    }




    {   // void64
        (void) memset(&buffer[offset_bits / 8U], 0, 8);
        offset_bits += 64UL;
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
static inline int8_t uavcan_metatransport_udp_Endpoint_0_1_deserialize_(
    uavcan_metatransport_udp_Endpoint_0_1* const out_obj, const uint8_t* buffer, size_t* const inout_buffer_size_bytes)
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





    // saturated uint8[16] ip_address
    nunavutGetBits(&out_obj->ip_address[0], &buffer[0], capacity_bytes, offset_bits, 16UL * 8U);
    offset_bits += 16UL * 8U;




    // saturated uint8[6] mac_address
    nunavutGetBits(&out_obj->mac_address[0], &buffer[0], capacity_bytes, offset_bits, 6UL * 8U);
    offset_bits += 6UL * 8U;




    // saturated uint16 port
    out_obj->port = nunavutGetU16(&buffer[0], capacity_bytes, offset_bits, 16);
    offset_bits += 16U;




    // void64
    offset_bits += 64;


    offset_bits = (offset_bits + 7U) & ~(size_t) 7U;  // Align on 8 bits.

    *inout_buffer_size_bytes = (size_t) (nunavutChooseMin(offset_bits, capacity_bits) / 8U);


    return NUNAVUT_SUCCESS;
}

/// Initialize an instance to default values. Does nothing if @param out_obj is NULL.
/// This function intentionally leaves inactive elements uninitialized; for example, members of a variable-length
/// array beyond its length are left uninitialized; aliased union memory that is not used by the first union field
/// is left uninitialized, etc. If full zero-initialization is desired, just use memset(&obj, 0, sizeof(obj)).
static inline void uavcan_metatransport_udp_Endpoint_0_1_initialize_(uavcan_metatransport_udp_Endpoint_0_1* const out_obj)
{
    if (out_obj != NULL)
    {
        size_t size_bytes = 0;
        const uint8_t buf = 0;
        const int8_t err = uavcan_metatransport_udp_Endpoint_0_1_deserialize_(out_obj, &buf, &size_bytes);

        (void) err;
    }
}



#ifdef __cplusplus
}
#endif
#endif // UAVCAN_METATRANSPORT_UDP_ENDPOINT_0_1_INCLUDED_
