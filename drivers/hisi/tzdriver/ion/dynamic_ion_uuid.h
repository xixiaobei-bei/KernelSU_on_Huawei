/*
 * dynamic_ion_uuid.h
 *
 * dynamic ion uuid declaration.
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#ifndef DYNAMIC_ION_UUID_H
#define DYNAMIC_ION_UUID_H

#ifdef DEF_ENG
#define TEE_SERVICE_UT \
{ \
	0x03030303, \
	0x0303, \
	0x0303, \
	{ \
		0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03 \
	} \
}

#define TEE_SERVICE_TEST_DYNION \
{ \
	0x7f313b2a, \
	0x68b9, \
	0x4e92, \
	{ \
		0xac, 0xf9, 0x13, 0x3e, 0xbb, 0x54, 0xeb, 0x56 \
	} \
}
#endif

#define TEE_SECIDENTIFICATION1 \
{ \
	0x8780dda1, \
	0xa49e, \
	0x45f4, \
	{ \
		0x96, 0x97, 0xc7, 0xed, 0x9e, 0x38, 0x5e, 0x83 \
	} \
}

#define TEE_SECIDENTIFICATION3 \
{ \
	0x335129cd, \
	0x41fa, \
	0x4b53, \
	{ \
		0x97, 0x97, 0x5c, 0xcb, 0x20, 0x2a, 0x52, 0xd4 \
	} \
}

#define TEE_SERVICE_HIAI \
{ \
	0xf4a8816d, \
	0xb6fb, \
	0x4d4f, \
	{ \
		0xa2, 0xb9, 0x7d, 0xae, 0x57, 0x33, 0x13, 0xc0 \
	} \
}

#define TEE_SERVICE_HIAI_TINY \
{ \
	0xc123c643, \
	0x5b5b, \
	0x4c9f, \
	{ \
		0x90, 0x98, 0xbb, 0x09, 0x56, 0x4d, 0x6e, 0xda \
	} \
}
#endif
