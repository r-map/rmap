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
// Source file:   C:/Dati/RMAP/stimav4-test/uavcan slave-th/data_types/uavcan/pnp/cluster/Entry.1.0.dsdl
// Generated at:  2022-09-03 16:09:57.309430 UTC
// Is deprecated: no
// Fixed port-ID: None
// Full name:     uavcan.pnp.cluster.Entry
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
//     runtime_platform:  Windows-10-10.0.19043-SP0
//
// Language Options
//     target_endianness:  little
//     omit_float_serialization_support:  False
//     enable_serialization_asserts:  False
//     enable_override_variable_array_capacity:  False
//     cast_format:  (({type}) {value})

#ifndef UAVCAN_PNP_CLUSTER_ENTRY_1_0_INCLUDED_
#define UAVCAN_PNP_CLUSTER_ENTRY_1_0_INCLUDED_

#include <nunavut/support/serialization.h>
#include <uavcan/node/ID_1_0.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static_assert( NUNAVUT_SUPPORT_LANGUAGE_OPTION_TARGET_ENDIANNESS == 434322821,
              "C:/Dati/RMAP/stimav4-test/uavcan slave-th/data_types/uavcan/pnp/cluster/Entry.1.0.dsdl is trying to use a serialization library that was compiled with "
              "different language options. This is dangerous and therefore not allowed." );
static_assert( NUNAVUT_SUPPORT_LANGUAGE_OPTION_OMIT_FLOAT_SERIALIZATION_SUPPORT == 0,
              "C:/Dati/RMAP/stimav4-test/uavcan slave-th/data_types/uavcan/pnp/cluster/Entry.1.0.dsdl is trying to use a serialization library that was compiled with "
              "different language options. This is dangerous and therefore not allowed." );
static_assert( NUNAVUT_SUPPORT_LANGUAGE_OPTION_ENABLE_SERIALIZATION_ASSERTS == 0,
              "C:/Dati/RMAP/stimav4-test/uavcan slave-th/data_types/uavcan/pnp/cluster/Entry.1.0.dsdl is trying to use a serialization library that was compiled with "
              "different language options. This is dangerous and therefore not allowed." );
static_assert( NUNAVUT_SUPPORT_LANGUAGE_OPTION_ENABLE_OVERRIDE_VARIABLE_ARRAY_CAPACITY == 0,
              "C:/Dati/RMAP/stimav4-test/uavcan slave-th/data_types/uavcan/pnp/cluster/Entry.1.0.dsdl is trying to use a serialization library that was compiled with "
              "different language options. This is dangerous and therefore not allowed." );
static_assert( NUNAVUT_SUPPORT_LANGUAGE_OPTION_CAST_FORMAT == 2368206204,
              "C:/Dati/RMAP/stimav4-test/uavcan slave-th/data_types/uavcan/pnp/cluster/Entry.1.0.dsdl is trying to use a serialization library that was compiled with "
              "different language options. This is dangerous and therefore not allowed." );

#ifdef __cplusplus
extern "C" {
#endif

/// This type does not have a fixed port-ID. See https://forum.opencyphal.org/t/choosing-message-and-service-ids/889
#define uavcan_pnp_cluster_Entry_1_0_HAS_FIXED_PORT_ID_ false

#define uavcan_pnp_cluster_Entry_1_0_FULL_NAME_             "uavcan.pnp.cluster.Entry"
#define uavcan_pnp_cluster_Entry_1_0_FULL_NAME_AND_VERSION_ "uavcan.pnp.cluster.Entry.1.0"

/// Extent is the minimum amount of memory required to hold any serialized representation of any compatible
/// version of the data type; or, on other words, it is the the maximum possible size of received objects of this type.
/// The size is specified in bytes (rather than bits) because by definition, extent is an integer number of bytes long.
/// When allocating a deserialization (RX) buffer for this data type, it should be at least extent bytes large.
/// When allocating a serialization (TX) buffer, it is safe to use the size of the largest serialized representation
/// instead of the extent because it provides a tighter bound of the object size; it is safe because the concrete type
/// is always known during serialization (unlike deserialization). If not sure, use extent everywhere.
#define uavcan_pnp_cluster_Entry_1_0_EXTENT_BYTES_                    22UL
#define uavcan_pnp_cluster_Entry_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_ 22UL
static_assert(uavcan_pnp_cluster_Entry_1_0_EXTENT_BYTES_ >= uavcan_pnp_cluster_Entry_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_,
              "Internal constraint violation");

/// Array metadata for: saturated uint8[16] unique_id
#define uavcan_pnp_cluster_Entry_1_0_unique_id_ARRAY_CAPACITY_           16U
#define uavcan_pnp_cluster_Entry_1_0_unique_id_ARRAY_IS_VARIABLE_LENGTH_ false

typedef struct
{
    /// saturated uint32 term
    uint32_t term;

    /// saturated uint8[16] unique_id
    uint8_t unique_id[16];

    /// uavcan.node.ID.1.0 node_id
    uavcan_node_ID_1_0 node_id;
} uavcan_pnp_cluster_Entry_1_0;

/// Serialize an instance into the provided buffer.
/// The lifetime of the resulting serialized representation is independent of the original instance.
/// This method may be slow for large objects (e.g., images, point clouds, radar samples), so in a later revision
/// we may define a zero-copy alternative that keeps references to the original object where possible.
///
/// @param obj      The object to serialize.
///
/// @param buffer   The destination buffer. There are no alignment requirements.
///                 @see uavcan_pnp_cluster_Entry_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_
///
/// @param inout_buffer_size_bytes  When calling, this is a pointer to the size of the buffer in bytes.
///                                 Upon return this value will be updated with the size of the constructed serialized
///                                 representation (in bytes); this value is then to be passed over to the transport
///                                 layer. In case of error this value is undefined.
///
/// @returns Negative on error, zero on success.
static inline int8_t uavcan_pnp_cluster_Entry_1_0_serialize_(
    const uavcan_pnp_cluster_Entry_1_0* const obj, uint8_t* const buffer,  size_t* const inout_buffer_size_bytes)
{
    if ((obj == NULL) || (buffer == NULL) || (inout_buffer_size_bytes == NULL))
    {
        return -NUNAVUT_ERROR_INVALID_ARGUMENT;
    }


    const size_t capacity_bytes = *inout_buffer_size_bytes;
    if ((8U * (size_t) capacity_bytes) < 176UL)
    {
        return -NUNAVUT_ERROR_SERIALIZATION_BUFFER_TOO_SMALL;
    }
    // Notice that fields that are not an integer number of bytes long may overrun the space allocated for them
    // in the serialization buffer up to the next byte boundary. This is by design and is guaranteed to be safe.
    size_t offset_bits = 0U;





    {   // saturated uint32 term
        // Saturation code not emitted -- native representation matches the serialized representation.
        (void) memmove(&buffer[offset_bits / 8U], &obj->term, 4U);
        offset_bits += 32U;
    }




    {   // saturated uint8[16] unique_id
        // Optimization prospect: this item is aligned at the byte boundary, so it is possible to use memmove().
        nunavutCopyBits(&buffer[0], offset_bits, 16UL * 8U, &obj->unique_id[0], 0U);
        offset_bits += 16UL * 8U;
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

    {   // uavcan.node.ID.1.0 node_id
        size_t _size_bytes0_ = 2UL;  // Nested object (max) size, in bytes.
        int8_t _err1_ = uavcan_node_ID_1_0_serialize_(
            &obj->node_id, &buffer[offset_bits / 8U], &_size_bytes0_);
        if (_err1_ < 0)
        {
            return _err1_;
        }
        // It is assumed that we know the exact type of the serialized entity, hence we expect the size to match.
        offset_bits += _size_bytes0_ * 8U;  // Advance by the size of the nested object.
    }


    if (offset_bits % 8U != 0U)  // Pad to 8 bits. TODO: Eliminate redundant padding checks.
    {
        const uint8_t _pad1_ = (uint8_t)(8U - offset_bits % 8U);
        const int8_t _err2_ = nunavutSetUxx(&buffer[0], capacity_bytes, offset_bits, 0U, _pad1_);  // Optimize?
        if (_err2_ < 0)
        {
            return _err2_;
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
static inline int8_t uavcan_pnp_cluster_Entry_1_0_deserialize_(
    uavcan_pnp_cluster_Entry_1_0* const out_obj, const uint8_t* buffer, size_t* const inout_buffer_size_bytes)
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





    // saturated uint32 term
    out_obj->term = nunavutGetU32(&buffer[0], capacity_bytes, offset_bits, 32);
    offset_bits += 32U;




    // saturated uint8[16] unique_id
    nunavutGetBits(&out_obj->unique_id[0], &buffer[0], capacity_bytes, offset_bits, 16UL * 8U);
    offset_bits += 16UL * 8U;


    offset_bits = (offset_bits + 7U) & ~(size_t) 7U;  // Align on 8 bits.

    // uavcan.node.ID.1.0 node_id
    {
        size_t _size_bytes1_ = (size_t)(capacity_bytes - nunavutChooseMin((offset_bits / 8U), capacity_bytes));
        const int8_t _err3_ = uavcan_node_ID_1_0_deserialize_(
            &out_obj->node_id, &buffer[offset_bits / 8U], &_size_bytes1_);
        if (_err3_ < 0)
        {
            return _err3_;
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
static inline void uavcan_pnp_cluster_Entry_1_0_initialize_(uavcan_pnp_cluster_Entry_1_0* const out_obj)
{
    if (out_obj != NULL)
    {
        size_t size_bytes = 0;
        const uint8_t buf = 0;
        const int8_t err = uavcan_pnp_cluster_Entry_1_0_deserialize_(out_obj, &buf, &size_bytes);

        (void) err;
    }
}



#ifdef __cplusplus
}
#endif
#endif // UAVCAN_PNP_CLUSTER_ENTRY_1_0_INCLUDED_