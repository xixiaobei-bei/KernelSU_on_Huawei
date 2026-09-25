/*
 * becc.h
 *
 * Binary ECC operations
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


#ifndef _BECC_H_
#define _BECC_H_

#include <linux/types.h>

/* the degree of the field polynomial */
#define DEGREE		163
/* don't touch this */
#define MARGIN		3
/* Size of coordinate point in 32-bit words */
#define NUMWORDS	((DEGREE + MARGIN + 31) / 32)

typedef uint32_t bitstr_t[NUMWORDS];

/*
 * Convert from binary to ECC scalar.
 *
 * e		ECC scalar input
 * buf		Pointer to binary output
 * bufsize	Size of binary output buffer
 */
void becc_bin2elem(bitstr_t e, const uint8_t *buf, const uint32_t bufsize);

/*
 * Convert from ECC scalar to binary.
 *
 * buf		Pointer to binary input
 * bufsize	Size of binary input buffer
 * e		ECC scalar output
 */
void becc_elem2bin(uint8_t *buf, const uint32_t bufsize, const bitstr_t e);

/*
 * Converts point coordinate to EPIF format from normal representation.
 *
 * destination	Destination where to store converted coordinate in EPIF format
 * x_value	Coordinate in standard representation
 */
void becc_to_epif(bitstr_t destination, const bitstr_t x_value);

/*
 * Copy bitstream
 * A Output bitstream
 * B Input bitstream
 */
#define BECC_BITSTR_COPY(x, y)	memcpy((x), (y), sizeof(bitstr_t))

/*
 * Copy point
 * x1 X-coordinate output
 * y1 Y-coordinate output
 * x2 X-coordinate input
 * y2 Y-coordinate input
 */
#define BECC_POINT_COPY(x1, y1, x2, y2)			\
do {							\
	BECC_BITSTR_COPY(x1, x2);			\
	BECC_BITSTR_COPY(y1, y2);			\
} while (0)

/* Point multiplication via double-and-add algorithm */
void becc_point_mult(bitstr_t x, bitstr_t y, const bitstr_t exp);

/*
 * Invert field
 * z Output
 * x Input
 */
void becc_field_invert(bitstr_t z, const bitstr_t x);

/*
 * Multiply two fields.
 * z		Output
 * x		Input1
 * y		Input2
 * isEPIF	Set to true if input/output is in EPIF format,
 *			otherwise set to false
 */
void becc_field_mult(bitstr_t z,
		const bitstr_t x, const bitstr_t y, const bool epif);

/*
 * Add two fields
 * z		Output
 * x		Input1
 * y		Input2
 */
void becc_field_add(bitstr_t z, const bitstr_t x, const bitstr_t y);

/* field square */
void becc_field_square(bitstr_t z, const bitstr_t x);

#endif /* _BECC_H_ */
