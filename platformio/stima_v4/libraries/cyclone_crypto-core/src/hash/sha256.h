/**
 * @file sha256.h
 * @brief SHA-256 (Secure Hash Algorithm 256)
 *
 * @section License
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2010-2022 Oryx Embedded SARL. All rights reserved.
 *
 * This file is part of CycloneCRYPTO Open.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 2.1.4
 **/

#ifndef _SHA256_H
#define _SHA256_H

//Dependencies
#include "core/crypto.h"

//SHA-256 block size
#define SHA256_BLOCK_SIZE 64
//SHA-256 digest size
#define SHA256_DIGEST_SIZE 32
//Minimum length of the padding string
#define SHA256_MIN_PAD_SIZE 9
//SHA-256 algorithm object identifier
#define SHA256_OID sha256Oid
//Common interface for hash algorithms
#define SHA256_HASH_ALGO (&sha256HashAlgo)

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief SHA-256 algorithm context
 **/

typedef struct
{
   union
   {
      uint32_t h[8];
      uint8_t digest[32];
   };
   union
   {
      uint32_t w[16];
      uint8_t buffer[64];
   };
   size_t size;
   uint64_t totalSize;
#if ((defined(MIMXRT1050_CRYPTO_HASH_SUPPORT) && MIMXRT1050_CRYPTO_HASH_SUPPORT == ENABLED) || \
   (defined(MIMXRT1060_CRYPTO_HASH_SUPPORT) && MIMXRT1060_CRYPTO_HASH_SUPPORT == ENABLED))
   uint32_t dcpHandle[11];
   uint32_t dcpContext[64];
#elif (defined(MIMXRT1170_CRYPTO_HASH_SUPPORT) && MIMXRT1170_CRYPTO_HASH_SUPPORT == ENABLED)
   uint32_t caamHandle[3];
   uint32_t caamContext[58];
#elif ((defined(STM32F2XX_CRYPTO_HASH_SUPPORT) && STM32F2XX_CRYPTO_HASH_SUPPORT == ENABLED) || \
   (defined(STM32F4XX_CRYPTO_HASH_SUPPORT) && STM32F4XX_CRYPTO_HASH_SUPPORT == ENABLED) || \
   (defined(STM32F7XX_CRYPTO_HASH_SUPPORT) && STM32F7XX_CRYPTO_HASH_SUPPORT == ENABLED) || \
   (defined(STM32H7XX_CRYPTO_HASH_SUPPORT) && STM32H7XX_CRYPTO_HASH_SUPPORT == ENABLED) || \
   (defined(STM32L4XX_CRYPTO_HASH_SUPPORT) && STM32L4XX_CRYPTO_HASH_SUPPORT == ENABLED) || \
   (defined(STM32L5XX_CRYPTO_HASH_SUPPORT) && STM32L5XX_CRYPTO_HASH_SUPPORT == ENABLED) || \
   (defined(STM32MP1XX_CRYPTO_HASH_SUPPORT) && STM32MP1XX_CRYPTO_HASH_SUPPORT == ENABLED))
   uint32_t hwContext[58];
#endif
} Sha256Context;


//SHA-256 related constants
extern const uint8_t sha256Oid[9];
extern const HashAlgo sha256HashAlgo;

//SHA-256 related functions
error_t sha256Compute(const void *data, size_t length, uint8_t *digest);
void sha256Init(Sha256Context *context);
void sha256Update(Sha256Context *context, const void *data, size_t length);
void sha256Final(Sha256Context *context, uint8_t *digest);
void sha256FinalRaw(Sha256Context *context, uint8_t *digest);
void sha256ProcessBlock(Sha256Context *context);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
