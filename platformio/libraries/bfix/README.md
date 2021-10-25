```
A Generic API for Bit Manipulation in C/C++
    - insert(bfi()) or extract(bfx()) bit fields from an unsigned char array of arbitrary length
    - compiles with clang/gcc/clang++/g++
    - offset bit numbers are from 1(start of array) to unlimited
    - can handle 32 or 64 bit machines
    - big endian, little endian or run time checking
    - bit fields inserted/extracted can be from <=32/64 to 25/57 bits in length depending
      on the offset bit number from the beginning of the array(see bfix.cpp Note 2 for details)
    - tested on Fedora 25 Linux with:
          clang   - clang version 5.0.0 (trunk 297778)
          gcc     - gcc (GCC) 6.3.1 20161221 (Red Hat 6.3.1-1)
          clang++ - clang version 5.0.0 (trunk 297778)
          g++     - g++ (GCC) 6.3.1 20161221 (Red Hat 6.3.1-1)
    - use LLVM 4.0.0 scan-build for static analysis
    - see article in Embedded Systems Programming, Jul. 1999,
      "A Generic API for Bit Manipulation in C"(included)
    - caveat: always make the unsigned char array 3(32 bit machines) or 7(64 bit machines) bytes
      longer to prevent read/write of bits beyond the logical end of array
      (see bfix.cpp Note 3 for details)

API:

/*
 *==================================================================================================
 *
 * File Name:
 *     bfix.cpp
 *
 *==================================================================================================
 *
 * Functions:
 *
 *         int
 *     bfi
 *         (
 *             unsigned char *cptr,
 *             unsigned long bit_offset,
 *             unsigned long bit_len,
 *             long value,
 *             unsigned int endian
 *         )
 *         e.g. int return = bfi(cptr, bit_offset, bit_len, value, endian);
 *
 *
 *         long
 *     bfx
 *         (
 *             const unsigned char *cptr,
 *             unsigned long bit_offset,
 *             unsigned long bit_len,
 *             unsigned int endian
 *         )
 *         e.g. long value = bfx(cptr, bit_offset, bit_len, endian);
 *
 *==================================================================================================
 *
 * Description:
 *
 *     bfi()/bfx() are used to insert and extract bit fields from an arbitrary length array
 *     of unsigned chars pointed to by an unsigned char* pointer.
 *
 *==================================================================================================
 *
 * Error Handling
 *
 *     1. Exceptions - None
 *     2. Debugging  - const int DEBUG = true for debugging output
 *     3. Returns
 *
 *            bfi():
 *                bit_offset < 1     - error, return -1
 *                bit_len < 1        - error, return -2
 *                bit_len > too long - error, return -3
 *                endian not 0-2     - error, return -4
 *                return 0           - success
 *
 *            bfx():
 *                bit_offset < 1     - error, return -1
 *                bit_len < 1        - error, return -2
 *                bit_len > too long - error, return -3
 *                endian not 0-2     - error, return -4
 *                return value       - success
 *
 *==================================================================================================
 *
 * Notes:
 *     1. in the following notes any annotation of the form n/m means n for 32 bit systems and
 *        m for 64 bit systems.  operation on 32 or 64 bit systems should be transparent.
 *
 *     2. bit_len should be <=32/64 to 25/57. it depends on the value of bit_offset.  the method
 *        always uses a memmove() of 4/8 bytes to a long temporary storage in which logical
 *        operations can be performed.  this means that the bit_len can be at most 4/8 bytes,
 *        but in a case in which the start bit is not at the beginning of a byte, then the
 *        bit_len can only extend until the end of the 4/8'th byte.  if the start bit is the
 *        last bit of a byte this will limit bit_len to 25/57 bits - the last bit of the first
 *        byte plus the next 3/7 bytes.
 *
 *     3. 4(32 bit machines)/8(64 bit machines) bytes are always read from the unsigned char
 *        array, modified and then written back.  this means that if you set the last bit of
 *        the array, then the next 3/7 bytes will be read and written back, thus seemingly
 *        overrunning the array.  if the 4/8 bytes does not overrun the array then no bits
 *        beyond the end of the array will be changed.  if the 4/8 bytes does overrun the
 *        array some provision must be made to deal with this possibility.  the array could
 *        be padded by 3/7 extra bytes.
 *
 *     4. bit_offset+bit_len should not overrun the array.
 *
 *     5. value should not be too long to fit into the bit field.  if it is, the high order bits
 *        in front of the low order bit_len bits will be truncated.
 *
 *     6. all bit_len bits will be set and no bit outside the bit field will be changed.
 *
 *     7. value may be negative and prefix 2's complement sign bits are truncated to fit into
 *        bit_len bits.
 *
 *     8. use the lscpu cmd to determine 32/64 bit and endianness:
 *        $ lscpu
 *        Architecture:          x86_64
 *        CPU op-mode(s):        32-bit, 64-bit
 *        Byte Order:            Little Endian
 *
 *==================================================================================================
 *
 * Author: Richard Hogaboom
 *         richard.hogaboom@gmail.com
 *
 *==================================================================================================
 */
```
