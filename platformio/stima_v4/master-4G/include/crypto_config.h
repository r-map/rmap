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

//Desired trace level (for debugging purposes)
#define CRYPTO_TRACE_LEVEL TRACE_LEVEL_INFO

// True random number generator
#define STM32L4XX_CRYPTO_TRNG_SUPPORT ENABLED

// Multiple precision integer support
#define MPI_SUPPORT ENABLED
// Assembly optimizations for time-critical routines
#define MPI_ASM_SUPPORT ENABLED

// Base64 encoding support
#define BASE64_SUPPORT ENABLED
// Base64url encoding support
#define BASE64URL_SUPPORT ENABLED

// MD2 hash support
#define MD2_SUPPORT ENABLED
// MD4 hash support
#define MD4_SUPPORT ENABLED
// MD5 hash support
#define MD5_SUPPORT ENABLED
// RIPEMD-128 hash support
#define RIPEMD128_SUPPORT ENABLED
// RIPEMD-160 hash support
#define RIPEMD160_SUPPORT ENABLED
// SHA-1 hash support
#define SHA1_SUPPORT ENABLED
// SHA-224 hash support
#define SHA224_SUPPORT ENABLED
// SHA-256 hash support
#define SHA256_SUPPORT ENABLED
// SHA-384 hash support
#define SHA384_SUPPORT ENABLED
// SHA-512 hash support
#define SHA512_SUPPORT ENABLED
// SHA-512/224 hash support
#define SHA512_224_SUPPORT ENABLED
// SHA-512/256 hash support
#define SHA512_256_SUPPORT ENABLED
// SHA3-224 hash support
#define SHA3_224_SUPPORT ENABLED
// SHA3-256 hash support
#define SHA3_256_SUPPORT ENABLED
// SHA3-384 hash support
#define SHA3_384_SUPPORT ENABLED
// SHA3-512 hash support
#define SHA3_512_SUPPORT ENABLED
// SHAKE support
#define SHAKE_SUPPORT ENABLED
// cSHAKE support
#define CSHAKE_SUPPORT ENABLED
// Keccak support
#define KECCAK_SUPPORT ENABLED
// BLAKE2b support
#define BLAKE2B_SUPPORT ENABLED
// BLAKE2b-160 hash support
#define BLAKE2B160_SUPPORT ENABLED
// BLAKE2b-256 hash support
#define BLAKE2B256_SUPPORT ENABLED
// BLAKE2b-384 hash support
#define BLAKE2B384_SUPPORT ENABLED
// BLAKE2b-512 hash support
#define BLAKE2B512_SUPPORT ENABLED
// BLAKE2s support
#define BLAKE2S_SUPPORT ENABLED
// BLAKE2s-128 hash support
#define BLAKE2S128_SUPPORT ENABLED
// BLAKE2s-160 hash support
#define BLAKE2S160_SUPPORT ENABLED
// BLAKE2s-224 hash support
#define BLAKE2S224_SUPPORT ENABLED
// BLAKE2s-256 hash support
#define BLAKE2S256_SUPPORT ENABLED
// Tiger hash support
#define TIGER_SUPPORT ENABLED
// Whirlpool hash support
#define WHIRLPOOL_SUPPORT ENABLED

// CMAC support
#define CMAC_SUPPORT ENABLED
// HMAC support
#define HMAC_SUPPORT ENABLED
// GMAC support
#define GMAC_SUPPORT ENABLED
// KMAC support
#define KMAC_SUPPORT ENABLED

// RC2 support
#define RC2_SUPPORT ENABLED
// RC4 support
#define RC4_SUPPORT ENABLED
// RC6 support
#define RC6_SUPPORT ENABLED
// IDEA support
#define IDEA_SUPPORT ENABLED
// DES support
#define DES_SUPPORT ENABLED
// Triple DES support
#define DES3_SUPPORT ENABLED
// AES support
#define AES_SUPPORT ENABLED
// Blowfish support
#define BLOWFISH_SUPPORT ENABLED
// Camellia support
#define CAMELLIA_SUPPORT ENABLED
// SEED support
#define SEED_SUPPORT ENABLED
// ARIA support
#define ARIA_SUPPORT ENABLED
// PRESENT support
#define PRESENT_SUPPORT ENABLED
// Trivium support
#define TRIVIUM_SUPPORT ENABLED

// ECB mode support
#define ECB_SUPPORT ENABLED
// CBC mode support
#define CBC_SUPPORT ENABLED
// CFB mode support
#define CFB_SUPPORT ENABLED
// OFB mode support
#define OFB_SUPPORT ENABLED
// CTR mode support
#define CTR_SUPPORT ENABLED
// XTS mode support
#define XTS_SUPPORT ENABLED
// CCM mode support
#define CCM_SUPPORT ENABLED
// GCM mode support
#define GCM_SUPPORT ENABLED

// Chacha support
#define CHACHA_SUPPORT ENABLED
// Poly1305 support
#define POLY1305_SUPPORT ENABLED
// Chacha20Poly1305 support
#define CHACHA20_POLY1305_SUPPORT ENABLED

// Diffie-Hellman support
#define DH_SUPPORT ENABLED
// RSA support
#define RSA_SUPPORT ENABLED
// DSA support
#define DSA_SUPPORT ENABLED

// Elliptic curve cryptography support
#define EC_SUPPORT ENABLED
// ECDH support
#define ECDH_SUPPORT ENABLED
// ECDSA support
#define ECDSA_SUPPORT ENABLED

// secp112r1 elliptic curve support
#define SECP112R1_SUPPORT ENABLED
// secp112r2 elliptic curve support
#define SECP112R2_SUPPORT ENABLED
// secp128r1 elliptic curve support
#define SECP128R1_SUPPORT ENABLED
// secp128r2 elliptic curve support
#define SECP128R2_SUPPORT ENABLED
// secp160k1 elliptic curve support
#define SECP160K1_SUPPORT ENABLED
// secp160r1 elliptic curve support
#define SECP160R1_SUPPORT ENABLED
// secp160r2 elliptic curve support
#define SECP160R2_SUPPORT ENABLED
// secp192k1 elliptic curve support
#define SECP192K1_SUPPORT ENABLED
// secp192r1 elliptic curve support (NIST P-192)
#define SECP192R1_SUPPORT ENABLED
// secp224k1 elliptic curve support
#define SECP224K1_SUPPORT ENABLED
// secp224r1 elliptic curve support (NIST P-224)
#define SECP224R1_SUPPORT ENABLED
// secp256k1 elliptic curve support
#define SECP256K1_SUPPORT ENABLED
// secp256r1 elliptic curve support (NIST P-256)
#define SECP256R1_SUPPORT ENABLED
// secp384r1 elliptic curve support (NIST P-384)
#define SECP384R1_SUPPORT ENABLED
// secp521r1 elliptic curve support (NIST P-521)
#define SECP521R1_SUPPORT ENABLED
// brainpoolP160r1 elliptic curve support
#define BRAINPOOLP160R1_SUPPORT ENABLED
// brainpoolP192r1 elliptic curve support
#define BRAINPOOLP192R1_SUPPORT ENABLED
// brainpoolP224r1 elliptic curve support
#define BRAINPOOLP224R1_SUPPORT ENABLED
// brainpoolP256r1 elliptic curve support
#define BRAINPOOLP256R1_SUPPORT ENABLED
// brainpoolP320r1 elliptic curve support
#define BRAINPOOLP320R1_SUPPORT ENABLED
// brainpoolP384r1 elliptic curve support
#define BRAINPOOLP384R1_SUPPORT ENABLED
// brainpoolP512r1 elliptic curve support
#define BRAINPOOLP512R1_SUPPORT ENABLED
// Curve25519 elliptic curve support
#define X25519_SUPPORT ENABLED
// Curve448 elliptic curve support
#define X448_SUPPORT ENABLED
// Ed25519 elliptic curve support
#define ED25519_SUPPORT ENABLED
// Ed448 elliptic curve support
#define ED448_SUPPORT ENABLED

// HKDF support
#define HKDF_SUPPORT ENABLED
// PBKDF support
#define PBKDF_SUPPORT ENABLED
// bcrypt support
#define BCRYPT_SUPPORT ENABLED
// scrypt support
#define SCRYPT_SUPPORT ENABLED

// RSA certificate support
#define X509_RSA_SUPPORT DISABLED
// RSA-PSS certificate support
#define X509_RSA_PSS_SUPPORT DISABLED
// DSA certificate support
#define X509_DSA_SUPPORT DISABLED
// Ed25519 certificate support
#define X509_ED25519_SUPPORT DISABLED
// Ed448 certificate support
#define X509_ED448_SUPPORT DISABLED

#endif
