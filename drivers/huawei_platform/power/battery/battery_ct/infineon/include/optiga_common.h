/*
 * optiga_common.h
 *
 * optiga common param
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

#ifndef _OPTIGA_COMMON_H_
#define _OPTIGA_COMMON_H_

#include "../sle95250.h"

#define SLE95250_ALLPGLK_MASK              0xff
#define SLE95250_NOPGLK_MASK               0x00
#define SLE95250_DFT_FULL_CYC              0xffff
#define SLE95250_SN_HEXASC_MASK            0xfb80

/* gpio operation error code */
#define SWI_GPIO_READ_FAIL                 0xff

/* size defination */
#define BIT_P_BYT_SWI                      13    /* bits */

/* swi time parameter */
#define OPTIGA_T_PWR_OFF                   400   /* 400us */
#define OPTIGA_T_PWR_UP                    15000 /* 15ms */
#define OPTIGA_T_NVM_TIMEOUT               5000  /* 5ms */
#define OPTIGA_T_RESP_TIMEOUT_LONG         32000 /* 32ms */

/* swi read byte error code */
#define SWI_ERR_RESP_TO                    1
#define SWI_ERR_BIT_TO                     2
#define SWI_ERR_TRAINING                   3

#endif /* _OPTIGA_COMMON_H_ */
