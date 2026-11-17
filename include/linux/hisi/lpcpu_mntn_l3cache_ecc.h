/*
 * lpcpu_mntn_l3cache_ecc.h
 *
 * L3Cache ECC(Error Correcting Code)
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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
#ifndef __LPCPU_MNTN_L3CACHE_ECC_H__
#define __LPCPU_MNTN_L3CACHE_ECC_H__

#include "../../../drivers/hisi/mntn/mntn_l3cache_ecc.h"

#ifdef CONFIG_HISI_L3CACHE_ECC
enum serr_type l3cache_ecc_get_status(u64 *err1_status, u64 *err1_misc0);
#else
static inline enum serr_type l3cache_ecc_get_status(u64 *err1_status,
						    u64 *err1_misc0)
{
	return NO_EEROR;
}
#endif
#endif /* __LPCPU_MNTN_L3CACHE_ECC_H__ */
