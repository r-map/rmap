/**
 * @file crypto_config.h
 * @brief CycloneCRYPTO configuration file
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

#ifndef _CRYPTO_CONFIG_H
#define _CRYPTO_CONFIG_H

/// @brief Desired trace level (for debugging purposes)
#define CRYPTO_TRACE_LEVEL TRACE_LEVEL_OFF

/// @brief True random number generator
#define STM32L4XX_CRYPTO_TRNG_SUPPORT ENABLED

/// @brief Multiple precision integer support
#define MPI_SUPPORT ENABLED
/// @brief Assembly optimizations for time-critical routines
#define MPI_ASM_SUPPORT ENABLED

/// @brief Base64 encoding support
#define BASE64_SUPPORT ENABLED
/// @brief Base64url encoding support
#define BASE64URL_SUPPORT ENABLED

/// @brief MD2 hash support
#define MD2_SUPPORT ENABLED
/// @brief MD4 hash support
#define MD4_SUPPORT ENABLED
/// @brief MD5 hash support
#define MD5_SUPPORT ENABLED
/// @brief RIPEMD-128 hash support
#define RIPEMD128_SUPPORT ENABLED
/// @brief RIPEMD-160 hash support
#define RIPEMD160_SUPPORT ENABLED
/// @brief SHA-1 hash support
#define SHA1_SUPPORT ENABLED
/// @brief SHA-224 hash support
#define SHA224_SUPPORT ENABLED
/// @brief SHA-256 hash support
#define SHA256_SUPPORT ENABLED
/// @brief SHA-384 hash support
#define SHA384_SUPPORT ENABLED
/// @brief SHA-512 hash support
#define SHA512_SUPPORT ENABLED
/// @brief SHA-512/224 hash support
#define SHA512_224_SUPPORT ENABLED
/// @brief SHA-512/256 hash support
#define SHA512_256_SUPPORT ENABLED
/// @brief SHA3-224 hash support
#define SHA3_224_SUPPORT ENABLED
/// @brief SHA3-256 hash support
#define SHA3_256_SUPPORT ENABLED
/// @brief SHA3-384 hash support
#define SHA3_384_SUPPORT ENABLED
/// @brief SHA3-512 hash support
#define SHA3_512_SUPPORT ENABLED
/// @brief SHAKE support
#define SHAKE_SUPPORT ENABLED
/// @brief cSHAKE support
#define CSHAKE_SUPPORT ENABLED
/// @brief Keccak support
#define KECCAK_SUPPORT ENABLED
/// @brief BLAKE2b support
#define BLAKE2B_SUPPORT ENABLED
/// @brief BLAKE2b-160 hash support
#define BLAKE2B160_SUPPORT ENABLED
/// @brief BLAKE2b-256 hash support
#define BLAKE2B256_SUPPORT ENABLED
/// @brief BLAKE2b-384 hash support
#define BLAKE2B384_SUPPORT ENABLED
/// @brief BLAKE2b-512 hash support
#define BLAKE2B512_SUPPORT ENABLED
/// @brief BLAKE2s support
#define BLAKE2S_SUPPORT ENABLED
/// @brief BLAKE2s-128 hash support
#define BLAKE2S128_SUPPORT ENABLED
/// @brief BLAKE2s-160 hash support
#define BLAKE2S160_SUPPORT ENABLED
/// @brief BLAKE2s-224 hash support
#define BLAKE2S224_SUPPORT ENABLED
/// @brief BLAKE2s-256 hash support
#define BLAKE2S256_SUPPORT ENABLED
/// @brief Tiger hash support
#define TIGER_SUPPORT ENABLED
/// @brief Whirlpool hash support
#define WHIRLPOOL_SUPPORT ENABLED

/// @brief CMAC support
#define CMAC_SUPPORT ENABLED
/// @brief HMAC support
#define HMAC_SUPPORT ENABLED
/// @brief GMAC support
#define GMAC_SUPPORT ENABLED
/// @brief KMAC support
#define KMAC_SUPPORT ENABLED

/// @brief RC2 support
#define RC2_SUPPORT ENABLED
/// @brief RC4 support
#define RC4_SUPPORT ENABLED
/// @brief RC6 support
#define RC6_SUPPORT ENABLED
/// @brief IDEA support
#define IDEA_SUPPORT ENABLED
/// @brief DES support
#define DES_SUPPORT ENABLED
/// @brief Triple DES support
#define DES3_SUPPORT ENABLED
/// @brief AES support
#define AES_SUPPORT ENABLED
/// @brief Blowfish support
#define BLOWFISH_SUPPORT ENABLED
/// @brief Camellia support
#define CAMELLIA_SUPPORT ENABLED
/// @brief SEED support
#define SEED_SUPPORT ENABLED
/// @brief ARIA support
#define ARIA_SUPPORT ENABLED
/// @brief PRESENT support
#define PRESENT_SUPPORT ENABLED
/// @brief Trivium support
#define TRIVIUM_SUPPORT ENABLED

/// @brief ECB mode support
#define ECB_SUPPORT ENABLED
/// @brief CBC mode support
#define CBC_SUPPORT ENABLED
/// @brief CFB mode support
#define CFB_SUPPORT ENABLED
/// @brief OFB mode support
#define OFB_SUPPORT ENABLED
/// @brief CTR mode support
#define CTR_SUPPORT ENABLED
/// @brief XTS mode support
#define XTS_SUPPORT ENABLED
/// @brief CCM mode support
#define CCM_SUPPORT ENABLED
/// @brief GCM mode support
#define GCM_SUPPORT ENABLED

/// @brief Chacha support
#define CHACHA_SUPPORT ENABLED
/// @brief Poly1305 support
#define POLY1305_SUPPORT ENABLED
/// @brief Chacha20Poly1305 support
#define CHACHA20_POLY1305_SUPPORT ENABLED

/// @brief Diffie-Hellman support
#define DH_SUPPORT ENABLED
/// @brief RSA support
#define RSA_SUPPORT ENABLED
/// @brief DSA support
#define DSA_SUPPORT ENABLED

/// @brief Elliptic curve cryptography support
#define EC_SUPPORT ENABLED
/// @brief ECDH support
#define ECDH_SUPPORT ENABLED
/// @brief ECDSA support
#define ECDSA_SUPPORT ENABLED

/// @brief secp112r1 elliptic curve support
#define SECP112R1_SUPPORT ENABLED
/// @brief secp112r2 elliptic curve support
#define SECP112R2_SUPPORT ENABLED
/// @brief secp128r1 elliptic curve support
#define SECP128R1_SUPPORT ENABLED
/// @brief secp128r2 elliptic curve support
#define SECP128R2_SUPPORT ENABLED
/// @brief secp160k1 elliptic curve support
#define SECP160K1_SUPPORT ENABLED
/// @brief secp160r1 elliptic curve support
#define SECP160R1_SUPPORT ENABLED
/// @brief secp160r2 elliptic curve support
#define SECP160R2_SUPPORT ENABLED
/// @brief secp192k1 elliptic curve support
#define SECP192K1_SUPPORT ENABLED
/// @brief secp192r1 elliptic curve support (NIST P-192)
#define SECP192R1_SUPPORT ENABLED
/// @brief secp224k1 elliptic curve support
#define SECP224K1_SUPPORT ENABLED
/// @brief secp224r1 elliptic curve support (NIST P-224)
#define SECP224R1_SUPPORT ENABLED
/// @brief secp256k1 elliptic curve support
#define SECP256K1_SUPPORT ENABLED
/// @brief secp256r1 elliptic curve support (NIST P-256)
#define SECP256R1_SUPPORT ENABLED
/// @brief secp384r1 elliptic curve support (NIST P-384)
#define SECP384R1_SUPPORT ENABLED
/// @brief secp521r1 elliptic curve support (NIST P-521)
#define SECP521R1_SUPPORT ENABLED
/// @brief brainpoolP160r1 elliptic curve support
#define BRAINPOOLP160R1_SUPPORT ENABLED
/// @brief brainpoolP192r1 elliptic curve support
#define BRAINPOOLP192R1_SUPPORT ENABLED
/// @brief brainpoolP224r1 elliptic curve support
#define BRAINPOOLP224R1_SUPPORT ENABLED
/// @brief brainpoolP256r1 elliptic curve support
#define BRAINPOOLP256R1_SUPPORT ENABLED
/// @brief brainpoolP320r1 elliptic curve support
#define BRAINPOOLP320R1_SUPPORT ENABLED
/// @brief brainpoolP384r1 elliptic curve support
#define BRAINPOOLP384R1_SUPPORT ENABLED
/// @brief brainpoolP512r1 elliptic curve support
#define BRAINPOOLP512R1_SUPPORT ENABLED
/// @brief Curve25519 elliptic curve support
#define X25519_SUPPORT ENABLED
/// @brief Curve448 elliptic curve support
#define X448_SUPPORT ENABLED
/// @brief Ed25519 elliptic curve support
#define ED25519_SUPPORT ENABLED
/// @brief Ed448 elliptic curve support
#define ED448_SUPPORT ENABLED

/// @brief HKDF support
#define HKDF_SUPPORT ENABLED
/// @brief PBKDF support
#define PBKDF_SUPPORT ENABLED
/// @brief bcrypt support
#define BCRYPT_SUPPORT ENABLED
/// @brief scrypt support
#define SCRYPT_SUPPORT ENABLED

/// @brief RSA certificate support
#define X509_RSA_SUPPORT DISABLED
/// @brief RSA-PSS certificate support
#define X509_RSA_PSS_SUPPORT DISABLED
/// @brief DSA certificate support
#define X509_DSA_SUPPORT DISABLED
/// @brief Ed25519 certificate support
#define X509_ED25519_SUPPORT DISABLED
/// @brief Ed448 certificate support
#define X509_ED448_SUPPORT DISABLED

#endif
