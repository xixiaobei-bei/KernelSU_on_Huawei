/*
 * optiga_nvm.h
 *
 * optiga nvm operation function headfile
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

#ifndef _OPTIGA_NVM_H_
#define _OPTIGA_NVM_H_

#include "optiga_common.h"

#define BYT_P_PG_NVM                         8

/* nvm page offset */
#define OPTIGA_CYC_PG_OFFSET                 1 /* page offset 1 */

/* lifespan counter */
#define OPTIGA_LS_LEN                        4 /* bytes */

/* nvm operation */
#define OPTIGA_NVM_OPR_LEN1                  0x00
#define OPTIGA_NVM_OPR_LEN2                  0x08
#define OPTIGA_NVM_OPR_LEN4                  0x10
#define OPTIGA_NVM_OPR_LEN8                  0x18

/* function predefinations */
int optiga_nvm_read(uint8_t nvm_area_mask, uint8_t pgind, int byte_offset,
	int read_len, uint8_t *data_ret);
int optiga_nvm_write(uint8_t nvm_area_mask, uint8_t pgind, int byte_offset,
	int write_len, uint8_t *data);

#endif /* _OPTIGA_NVM_H_ */
