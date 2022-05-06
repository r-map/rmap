/**
 * @file lpc54xxx_crypto_hash.c
 * @brief LPC54000 hash hardware accelerator
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

//Switch to the appropriate trace level
#define TRACE_LEVEL CRYPTO_TRACE_LEVEL

//Dependencies
#include "fsl_device_registers.h"
#include "fsl_sha.h"
#include "core/crypto.h"
#include "hardware/lpc54xxx/lpc54xxx_crypto.h"
#include "hardware/lpc54xxx/lpc54xxx_crypto_hash.h"
#include "hash/sha1.h"
#include "hash/sha224.h"
#include "hash/sha256.h"
#include "debug.h"

//Check crypto library configuration
#if (LPC54XXX_CRYPTO_HASH_SUPPORT == ENABLED)


/**
 * @brief Digest a message using SHA-1
 * @param[in] data Pointer to the message being hashed
 * @param[in] length Length of the message
 * @param[out] digest Pointer to the calculated digest
 * @return Error code
 **/

error_t sha1Compute(const void *data, size_t length, uint8_t *digest)
{
   size_t n;
   status_t status;
   sha_ctx_t context;

   //Size of the digest value
   n = SHA1_DIGEST_SIZE;

   //Acquire exclusive access to the SHA module
   osAcquireMutex(&lpc54xxxCryptoMutex);

   //Initialize hash computation
   status = SHA_Init(SHA0, &context, kSHA_Sha1);

   //Check status code
   if(status == kStatus_Success)
   {
      //Digest the message
      SHA_Update(SHA0, &context, data, length, true);
   }

   //Check status code
   if(status == kStatus_Success)
   {
      //Get the resulting digest value
      SHA_Finish(SHA0, &context, digest, &n);
   }

   //Release exclusive access to the SHA module
   osReleaseMutex(&lpc54xxxCryptoMutex);

   //Return status code
   return (status == kStatus_Success) ? NO_ERROR : ERROR_FAILURE;
}


/**
 * @brief Digest a message using SHA-256
 * @param[in] data Pointer to the message being hashed
 * @param[in] length Length of the message
 * @param[out] digest Pointer to the calculated digest
 * @return Error code
 **/

error_t sha256Compute(const void *data, size_t length, uint8_t *digest)
{
   size_t n;
   status_t status;
   sha_ctx_t context;

   //Size of the digest value
   n = SHA256_DIGEST_SIZE;

   //Acquire exclusive access to the SHA module
   osAcquireMutex(&lpc54xxxCryptoMutex);

   //Initialize hash computation
   status = SHA_Init(SHA0, &context, kSHA_Sha256);

   //Check status code
   if(status == kStatus_Success)
   {
      //Digest the message
      SHA_Update(SHA0, &context, data, length, true);
   }

   //Check status code
   if(status == kStatus_Success)
   {
      //Get the resulting digest value
      SHA_Finish(SHA0, &context, digest, &n);
   }

   //Release exclusive access to the SHA module
   osReleaseMutex(&lpc54xxxCryptoMutex);

   //Return status code
   return (status == kStatus_Success) ? NO_ERROR : ERROR_FAILURE;
}

#endif
