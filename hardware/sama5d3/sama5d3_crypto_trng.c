/**
 * @file sama5d3_crypto_trng.c
 * @brief SAMA5D3 true random number generator
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
#include "sama5d3x.h"
#include "core/crypto.h"
#include "hardware/sama5d3/sama5d3_crypto.h"
#include "hardware/sama5d3/sama5d3_crypto_trng.h"
#include "debug.h"

//Check crypto library configuration
#if (SAMA5D3_CRYPTO_TRNG_SUPPORT == ENABLED)


/**
 * @brief TRNG module initialization
 * @return Error code
 **/

error_t trngInit(void)
{
   //Enable TRNG peripheral clock
   PMC->PMC_PCER1 = (1 << (ID_TRNG - 32));
   //Enable TRNG
   TRNG->TRNG_CR = TRNG_CR_KEY(0x524E47) | TRNG_CR_ENABLE;

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief Get random data from the TRNG module
 * @param[out] data Buffer where to store random data
 * @param[in] length Number of random bytes to generate
 **/

error_t trngGetRandomData(uint8_t *data, size_t length)
{
   size_t i;
   uint32_t value;

   //Acquire exclusive access to the TRNG module
   osAcquireMutex(&sama5d3CryptoMutex);

   //Generate random data
   for(i = 0; i < length; i++)
   {
      //Generate a new 32-bit random value when necessary
      if((i % 4) == 0)
      {
         //Wait for the TRNG to contain a valid data
         while((TRNG->TRNG_ISR & TRNG_ISR_DATRDY) == 0)
         {
         }

         //Get the 32-bit random value
         value = TRNG->TRNG_ODATA;
      }

      //Copy random byte
      data[i] = value & 0xFF;
      //Shift the 32-bit random value
      value >>= 8;
   }

   //Release exclusive access to the TRNG module
   osReleaseMutex(&sama5d3CryptoMutex);

   //Successful processing
   return NO_ERROR;
}

#endif
