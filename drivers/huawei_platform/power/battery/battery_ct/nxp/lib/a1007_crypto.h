/*
 * a1007_crypto.h
 *
 * a1007 crypto interface
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _A1007_CRYPTO_H_
#define _A1007_CRYPTO_H_

#include "present80.h"
#include "uecc.h"
#include "becc.h"

/*
 * Cryptographic helper functions for A1007.
 * Includes function to decompress and validate A1007 certificates,
 * as well as to generate A1007 authentication challenges and validate
 * responses.
 */

#define A1007_CERT_SIZE 321
#define A1007_CERTZ_SIZE 128
#define A1007_EECODE_SIZE 8

/* Offsets and sizes of fields in A1007 certificate */
#define A1007_CERT_TBS_OFF 0x004
#define A1007_CERT_TBS_SIZE 238
#define A1007_CERT_SERIAL_OFF 0x00E
#define A1007_CERT_SERIAL_SIZE 9
#define A1007_CERT_ISSORG_OFF 0x030
#define A1007_CERT_ISSORG_SIZE 10
#define A1007_CERT_SUBUID_OFF 0x06C
#define A1007_CERT_SUBUID_SIZE 9
#define A1007_CERT_SUBDESC_OFF 0x080
#define A1007_CERT_SUBDESC_SIZE 4
#define A1007_CERT_SUBORG_OFF 0x08F
#define A1007_CERT_SUBORG_SIZE 10
#define A1007_CERT_SUBCN_OFF 0x0A4
#define A1007_CERT_SUBCN_SIZE 12
#define A1007_CERT_PUBKEY_OFF 0x0C8
#define A1007_CERT_PUBKEY_SIZE 42
#define A1007_CERT_SIG_OFF 0x101
#define A1007_CERT_SIG_SIZE 64

#define A1007_CERTZ_SUBORG_OFF 0x000
#define A1007_CERTZ_SUBCN_OFF 0x00A
#define A1007_CERTZ_PUBKEY_OFF 0x016
#define A1007_CERTZ_SIG_OFF 0x040

/* Error return codes */
#define ERROR_ECDSA_DER_SEQ_TAG		1
#define ERROR_ECDSA_DER_SEQ_LEN		2
#define ERROR_ECDSA_DER_R_INT_TAG	3
#define ERROR_ECDSA_DER_R_INT_LEN	4
#define ERROR_ECDSA_DER_S_INT_TAG	5
#define ERROR_ECDSA_DER_S_INT_LEN	6
#define ERROR_ECDSA_VERIFY_FAIL		7

/*
 * Verifies the DER encoded signature with NIST-p224 (secp224r1) curve,
 * with the specified public key, uncompressed (x,y) coordinated
 * with little endian and zero padded, against against the specified data
 * pubkey	Pointer to signer's public key (little-endian format)
 * dersig	Pointer to DER-encoded ECDSA signature
 * digest	SHA224 digest of data to verify
 * return 	0 on verified OK
 *		ERROR_ECDSA_VERIFY_FAIL on verify fail
 */
#define A1007_VERIFY_ECDSA_SIGN(pubkey, dersig, digest)	\
((uecc_verify((pubkey), (digest), 28, (dersig), true, uecc_secp224r1()) ==\
	1) ? 0 : ERROR_ECDSA_VERIFY_FAIL)

/*
 * Verifies the compressed DER-encoded X509v3 certificate.
 *
 * pubkey	Pointer to signer's public key (little-endian format)
 * devid	Device ID as 16-bit integer
 * uid		Pointer to UID
 * certz	Pointer to compressed certificate
 * return	0 on verified OK
 *		ERROR_ECDSA_VERIFY_FAIL on verify fail
 */
uint32_t a1007_verify_certz(const uecc_word_t *pubkey, uint16_t devid,
				const uint8_t *uid, const uint8_t *certz);

/*
 * Calculate CRC8 value of command/data message.
 *
 * command	Command of message
 * buf		Pointer to data of message
 * bufsize	Number of bytes in data
 * return	CRC8 value
 */
uint8_t a1007_crc8_calc(const uint16_t command, const uint8_t *buf,
				const uint32_t bufsize);

/*
 * Generates the A1007 challenge message from random data, stores the ephemeral
 * private key.
 */
uint32_t a1007_generate_challenge(uint8_t *buf, bitstr_t ecc_r,
		const uint8_t *randbuf);

/*
 * Precomputes ECDH secret (affine x-coordinate) by multiplying the
 * host ephemeral private key with the device-unique public key of the A1007.
 *
 * ecc_s	Output to store calculated ECDH secret.
 *			Can be same buffer as \p ECC_r
 * ecc_r	Own ephemeral private key used for keyagreement
 * pubkey	Pointer to the device-unique public key (big-endian format)
 *			of A1007
 */
void a1007_precompute_secret(bitstr_t ecc_s,
		const bitstr_t ecc_r, const uint8_t *pubkey);

/*
 * Parse the A1007 challenge response, using the returned projective
 * Z-coordinate to convert ECDH secret from affine to projective,
 * then run through KDF to obtain shared session MAC key, which is
 * then used to set the MAC context (key schedule) for that MAC key.
 *
 * buf		Pointer to received response
 * bufsize	Size of response in bytes
 * ecc_s	Precalculated ECDH secret
 * maccontext	Pointer to session MAC key context output
 * return	0 if key agreement successful
 *		1 if key agreement failed
 */
uint32_t a1007_compute_key_from_response(const uint8_t *buf,
	const uint32_t bufsize, const bitstr_t ecc_s, present80_t maccontext);

#endif	/* _A1007_CRYPTO_H_ */
