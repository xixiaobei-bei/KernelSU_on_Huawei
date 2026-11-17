/*
 * mntn_l3cache_ecc.h
 *
 * L3Cache ECC
 *
 * Copyright (c) 2014-2020 Huawei Technologies Co., Ltd.
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
#ifndef __MNTN_L3CACHE_ECC_H__
#define __MNTN_L3CACHE_ECC_H__

#define L3CACHE_FN_MAIN_ID			0xc500f004u
#define L3CACHE_ECC_READ			0x00
#define L3CACHE_ECC_WRITE			0x01
#define L3CACHE_ECC_OPEN			0x02
#define L3CACHE_ECC_CLOSE			0x03
#define L3CACHE_ECC_ERR_INPUT			0x04
#define L3CACHE_ECC_INT_CLEAR			0x05

#define ECC_OK					0
#define ECC_ERR					(-1)

#define ERR1STATUS_UE_BIT_MASK			0x20000000
#define ERR1STATUS_V_BIT_MASK			0x40000000

struct l3cache_ecc_info {
	int irq_fault;
	int irq_err;
};

enum {
	ERR1MISC0,
	ERR1STATUS,
};

enum serr_type {
	NO_EEROR = 0,
	INTERNAL_DATA_BUFFER,
	CACHE_DATA_RAM,
	CACHE_TAG_DIRTY_RAM,
	BUS_ERROR,
	INVALID_VAL,
};

#endif /* __MNTN_L3CACHE_ECC_H__ */
