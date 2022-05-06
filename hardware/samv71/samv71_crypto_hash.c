/**
 * @file samv71_crypto_hash.c
 * @brief SAMV71 hash hardware accelerator
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
#include "samv71.h"
#include "core/crypto.h"
#include "hardware/samv71/samv71_crypto.h"
#include "hardware/samv71/samv71_crypto_hash.h"
#include "hash/sha1.h"
#include "hash/sha224.h"
#include "hash/sha256.h"
#include "debug.h"

//Check crypto library configuration
#if (SAMV71_CRYPTO_HASH_SUPPORT == ENABLED)

//IAR EWARM compiler?
#if defined(__ICCARM__)

//ICM region descriptor
#pragma data_alignment = 64
#pragma location = SAMV71_ICM_RAM_SECTION
static Same54IcmDesc icmDesc;
//ICM data buffer
#pragma data_alignment = 4
#pragma location = SAMV71_ICM_RAM_SECTION
static uint8_t icmBuffer[SAMV71_ICM_BUFFER_SIZE];
//ICM hash area
#pragma data_alignment = 128
#pragma location = SAMV71_ICM_RAM_SECTION
static uint8_t icmHash[32];

//Keil MDK-ARM or GCC compiler?
#else

//ICM region descriptor
static Same54IcmDesc icmDesc
   __attribute__((aligned(64), __section__(SAMV71_ICM_RAM_SECTION)));
//ICM data buffer
static uint8_t icmBuffer[SAMV71_ICM_BUFFER_SIZE]
   __attribute__((aligned(4), __section__(SAMV71_ICM_RAM_SECTION)));
//ICM hash area
static uint32_t icmHash[8]
   __attribute__((aligned(128), __section__(SAMV71_ICM_RAM_SECTION)));

#endif

//Padding string
static const uint8_t padding[64] =
{
   0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


/**
 * @brief Update hash value
 * @param[in] algo Hash algorithm
 * @param[in] data Pointer to the input buffer
 * @param[in] length Length of the input buffer
 * @param[in,out] h Hash value
 **/

void hashProcessData(uint32_t algo, const uint8_t *data, size_t length,
   uint32_t *h)
{
   size_t n;

   //Acquire exclusive access to the ICM module
   osAcquireMutex(&samv71CryptoMutex);

   //Digest input data
   while(length >= 64)
   {
      //Limit the number of data to process at a time
      n = MIN(length, SAMV71_ICM_BUFFER_SIZE);

      //Copy input data
      osMemcpy(icmBuffer, data, (n + 3) & ~3UL);

      //Perform software reset
      ICM->ICM_CTRL = ICM_CTRL_SWRST;

      //Set ICM region descriptor
      icmDesc.raddr = (uint32_t) icmBuffer;
      icmDesc.rcfg = ((algo << 12) & ICM_RCFG_ALGO) | ICM_RCFG_EOM;
      icmDesc.rctrl = (n / 64) - 1;
      icmDesc.rnext = 0;

      //Data memory barrier
      __DMB();

      //Set configuration register
      ICM->ICM_CFG = ICM_CFG_UALGO(algo) | ICM_CFG_UIHASH | ICM_CFG_SLBDIS;
      //The start address is a multiple of 64 bytes
      ICM->ICM_DSCR = (uint32_t) &icmDesc;
      //The hash memory location must be a multiple of 128 bytes
      ICM->ICM_HASH = (uint32_t) &icmHash;

      //Set initial hash value
      ICM->ICM_UIHVAL[0] = h[0];
      ICM->ICM_UIHVAL[1] = h[1];
      ICM->ICM_UIHVAL[2] = h[2];
      ICM->ICM_UIHVAL[3] = h[3];
      ICM->ICM_UIHVAL[4] = h[4];

      //SHA-224 or SHA-256 algorithm?
      if(algo == ICM_ALGO_SHA224 || algo == ICM_ALGO_SHA256)
      {
         ICM->ICM_UIHVAL[5] = h[5];
         ICM->ICM_UIHVAL[6] = h[6];
         ICM->ICM_UIHVAL[7] = h[7];
      }

      //Enable RHC interrupt (Region Hash Completed)
      ICM->ICM_IER = ICM_IER_RHC(1);
      //Enable ICM module
      ICM->ICM_CTRL = ICM_CTRL_ENABLE;

      //The RHC status flag is set when the ICM has completed the region
      while((ICM->ICM_ISR & ICM_ISR_RHC_Msk) == 0)
      {
      }

      //Disable ICM module
      ICM->ICM_CTRL = ICM_CTRL_DISABLE;

      //Data memory barrier
      __DMB();

      //Read resulting hash value
      h[0] = icmHash[0];
      h[1] = icmHash[1];
      h[2] = icmHash[2];
      h[3] = icmHash[3];
      h[4] = icmHash[4];

      //SHA-224 or SHA-256 algorithm?
      if(algo == ICM_ALGO_SHA224 || algo == ICM_ALGO_SHA256)
      {
         h[5] = icmHash[5];
         h[6] = icmHash[6];
         h[7] = icmHash[7];
      }

      //Advance data pointer
      data += n;
      length -= n;
   }

   //Release exclusive access to the ICM module
   osReleaseMutex(&samv71CryptoMutex);
}


/**
 * @brief Initialize SHA-1 message digest context
 * @param[in] context Pointer to the SHA-1 context to initialize
 **/

void sha1Init(Sha1Context *context)
{
   //Set initial hash value
   context->h[0] = BETOH32(0x67452301);
   context->h[1] = BETOH32(0xEFCDAB89);
   context->h[2] = BETOH32(0x98BADCFE);
   context->h[3] = BETOH32(0x10325476);
   context->h[4] = BETOH32(0xC3D2E1F0);

   //Number of bytes in the buffer
   context->size = 0;
   //Total length of the message
   context->totalSize = 0;
}


/**
 * @brief Update the SHA-1 context with a portion of the message being hashed
 * @param[in] context Pointer to the SHA-1 context
 * @param[in] data Pointer to the buffer being hashed
 * @param[in] length Length of the buffer
 **/

void sha1Update(Sha1Context *context, const void *data, size_t length)
{
   size_t n;

   //Process the incoming data
   while(length > 0)
   {
      //Check whether some data is pending in the buffer
      if(context->size == 0 && length >= 64)
      {
         //The length must be a multiple of 64 bytes
         n = length - (length % 64);

         //Update hash value
         hashProcessData(ICM_ALGO_SHA1, data, n, context->h);

         //Update the SHA-1 context
         context->totalSize += n;
         //Advance the data pointer
         data = (uint8_t *) data + n;
         //Remaining bytes to process
         length -= n;
      }
      else
      {
         //The buffer can hold at most 64 bytes
         n = MIN(length, 64 - context->size);

         //Copy the data to the buffer
         osMemcpy(context->buffer + context->size, data, n);

         //Update the SHA-1 context
         context->size += n;
         context->totalSize += n;
         //Advance the data pointer
         data = (uint8_t *) data + n;
         //Remaining bytes to process
         length -= n;

         //Check whether the buffer is full
         if(context->size == 64)
         {
            //Update hash value
            hashProcessData(ICM_ALGO_SHA1, context->buffer, context->size,
               context->h);

            //Empty the buffer
            context->size = 0;
         }
      }
   }
}


/**
 * @brief Finish the SHA-1 message digest
 * @param[in] context Pointer to the SHA-1 context
 * @param[out] digest Calculated digest (optional parameter)
 **/

void sha1Final(Sha1Context *context, uint8_t *digest)
{
   size_t paddingSize;
   uint64_t totalSize;

   //Length of the original message (before padding)
   totalSize = context->totalSize * 8;

   //Pad the message so that its length is congruent to 56 modulo 64
   if(context->size < 56)
   {
      paddingSize = 56 - context->size;
   }
   else
   {
      paddingSize = 64 + 56 - context->size;
   }

   //Append padding
   sha1Update(context, padding, paddingSize);

   //Append the length of the original message
   context->w[14] = htobe32((uint32_t) (totalSize >> 32));
   context->w[15] = htobe32((uint32_t) totalSize);

   //Calculate the message digest
   hashProcessData(ICM_ALGO_SHA1, context->buffer, 64, context->h);

   //Copy the resulting digest
   if(digest != NULL)
   {
      osMemcpy(digest, context->digest, SHA1_DIGEST_SIZE);
   }
}


/**
 * @brief Finish the SHA-1 message digest (no padding added)
 * @param[in] context Pointer to the SHA-1 context
 * @param[out] digest Calculated digest
 **/

void sha1FinalRaw(Sha1Context *context, uint8_t *digest)
{
   //Copy the resulting digest
   osMemcpy(digest, context->digest, SHA1_DIGEST_SIZE);
}


/**
 * @brief Initialize SHA-224 message digest context
 * @param[in] context Pointer to the SHA-224 context to initialize
 **/

void sha224Init(Sha224Context *context)
{
   //Set initial hash value
   context->h[0] = BETOH32(0xC1059ED8);
   context->h[1] = BETOH32(0x367CD507);
   context->h[2] = BETOH32(0x3070DD17);
   context->h[3] = BETOH32(0xF70E5939);
   context->h[4] = BETOH32(0xFFC00B31);
   context->h[5] = BETOH32(0x68581511);
   context->h[6] = BETOH32(0x64F98FA7);
   context->h[7] = BETOH32(0xBEFA4FA4);

   //Number of bytes in the buffer
   context->size = 0;
   //Total length of the message
   context->totalSize = 0;
}


/**
 * @brief Initialize SHA-256 message digest context
 * @param[in] context Pointer to the SHA-256 context to initialize
 **/

void sha256Init(Sha256Context *context)
{
   //Set initial hash value
   context->h[0] = BETOH32(0x6A09E667);
   context->h[1] = BETOH32(0xBB67AE85);
   context->h[2] = BETOH32(0x3C6EF372);
   context->h[3] = BETOH32(0xA54FF53A);
   context->h[4] = BETOH32(0x510E527F);
   context->h[5] = BETOH32(0x9B05688C);
   context->h[6] = BETOH32(0x1F83D9AB);
   context->h[7] = BETOH32(0x5BE0CD19);

   //Number of bytes in the buffer
   context->size = 0;
   //Total length of the message
   context->totalSize = 0;
}


/**
 * @brief Update the SHA-256 context with a portion of the message being hashed
 * @param[in] context Pointer to the SHA-256 context
 * @param[in] data Pointer to the buffer being hashed
 * @param[in] length Length of the buffer
 **/

void sha256Update(Sha256Context *context, const void *data, size_t length)
{
   size_t n;

   //Process the incoming data
   while(length > 0)
   {
      //Check whether some data is pending in the buffer
      if(context->size == 0 && length >= 64)
      {
         //The length must be a multiple of 64 bytes
         n = length - (length % 64);

         //Update hash value
         hashProcessData(ICM_ALGO_SHA256, data, n, context->h);

         //Update the SHA-256 context
         context->totalSize += n;
         //Advance the data pointer
         data = (uint8_t *) data + n;
         //Remaining bytes to process
         length -= n;
      }
      else
      {
         //The buffer can hold at most 64 bytes
         n = MIN(length, 64 - context->size);

         //Copy the data to the buffer
         osMemcpy(context->buffer + context->size, data, n);

         //Update the SHA-256 context
         context->size += n;
         context->totalSize += n;
         //Advance the data pointer
         data = (uint8_t *) data + n;
         //Remaining bytes to process
         length -= n;

         //Check whether the buffer is full
         if(context->size == 64)
         {
            //Update hash value
            hashProcessData(ICM_ALGO_SHA256, context->buffer, context->size,
               context->h);

            //Empty the buffer
            context->size = 0;
         }
      }
   }
}


/**
 * @brief Finish the SHA-256 message digest
 * @param[in] context Pointer to the SHA-256 context
 * @param[out] digest Calculated digest (optional parameter)
 **/

void sha256Final(Sha256Context *context, uint8_t *digest)
{
   size_t paddingSize;
   uint64_t totalSize;

   //Length of the original message (before padding)
   totalSize = context->totalSize * 8;

   //Pad the message so that its length is congruent to 56 modulo 64
   if(context->size < 56)
   {
      paddingSize = 56 - context->size;
   }
   else
   {
      paddingSize = 64 + 56 - context->size;
   }

   //Append padding
   sha256Update(context, padding, paddingSize);

   //Append the length of the original message
   context->w[14] = htobe32((uint32_t) (totalSize >> 32));
   context->w[15] = htobe32((uint32_t) totalSize);

   //Calculate the message digest
   hashProcessData(ICM_ALGO_SHA256, context->buffer, 64, context->h);

   //Copy the resulting digest
   if(digest != NULL)
   {
      osMemcpy(digest, context->digest, SHA256_DIGEST_SIZE);
   }
}


/**
 * @brief Finish the SHA-256 message digest (no padding added)
 * @param[in] context Pointer to the SHA-256 context
 * @param[out] digest Calculated digest
 **/

void sha256FinalRaw(Sha256Context *context, uint8_t *digest)
{
   //Copy the resulting digest
   osMemcpy(digest, context->digest, SHA256_DIGEST_SIZE);
}

#endif
