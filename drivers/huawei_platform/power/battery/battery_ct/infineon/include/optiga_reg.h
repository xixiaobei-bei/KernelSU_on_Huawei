/*
 * optiga_reg.h
 *
 * optiga register operation function headfile
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

#ifndef _OPTIGA_REG_H_
#define _OPTIGA_REG_H_

#include "optiga_common.h"

#define BYT_P_PG_REG                        8

/* register addr */
/* register space reg */
#define REG_WIP2                            0x0010 /* wip2 page init addr */
#define REG_WIP0                            0x0020 /* wip0 page init addr */
#define REG_PRO_CTR                         0x026f /* pro_ctr reg addr */
#define REG_CTRL2                           0x0272 /* ctrl2 reg addr */
#define REG_NVM_ADDR                        0x0274 /* nvm addr reg addr */
#define REG_NVM_LOCK                        0x0275 /* nvm lock reg addr */
#define REG_NVM_LOCK_ST                     0x0276 /* nvm lock st reg addr */

/* config space reg */
#define REG_UID                             0x0040 /* uid reg start addr */
#define REG_UID11                           0x004b /* uid byte 11 addr */

/* protection control reg masks */
#define PRO_CTR_CYCLK                       0x04

/* nvm addr reg masks */
#define LIFE_SPAN_DRCREASE                  0x20

/* ctrl2 reg masks */
#define NVM_PROCESS_ON_GOING                0x80
#define OPTIGA_WIP0                         0x00
#define OPTIGA_WIP2                         0x00
#define OPTIGA_NVM_W                        0x40
#define OPTIGA_NVM_ACT                      0x80
#define SLE95250_USER_AREA                  0x00
#define SLE95250_RO_AREA                    0x08
#define BYT_OFFSET0                         0
#define BYT_OFFSET4                         4

/* retry */
#define REG_RD_RTY                          4

/* time param */
#define REG_WAIT_MOTION_DONE                10000   /* appromiate 5ms */

/* function predefination */
int optiga_reg_read(uint16_t reg_addr, uint8_t *data, bool cfg_spc);
void optiga_reg_write(uint16_t reg_addr, uint8_t data, bool cfg_spc);
int optiga_reg_pglk_set(uint8_t pglk_mask);
int optiga_reg_pglk_read(uint8_t *pglk);
int optiga_reg_life_span_lock_read(uint8_t *cyclk_status);
int optiga_reg_life_span_lock_set(void);
int optiga_reg_uid_read(uint8_t *uid);

#endif /* _OPTIGA_REG_H_ */
