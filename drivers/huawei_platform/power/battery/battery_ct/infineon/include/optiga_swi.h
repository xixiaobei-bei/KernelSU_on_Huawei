/*
 * optiga_swi.h
 *
 * optiga single wire interface operation function headfile
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

#ifndef _OPTIGA_SWI_H_
#define _OPTIGA_SWI_H_

#include "optiga_common.h"

/* transaction elements */
/* broad cast */
#define SWI_BC              0x08u    /* bus command */
#define SWI_EDA             0x09u    /* extended device address */
#define SWI_SDA             0x0Au    /* slave device address */

/* multi cast */
#define SWI_WD              0x04u    /* write data */
#define SWI_ERA             0x05u    /* extended reg addr */
#define SWI_WRA             0x06u    /* write reg addr */
#define SWI_RRA             0x07u    /* read reg addr */

/* bus command */
#define SWI_RLEN0           0x20u    /* read length 1 byte */
#define SWI_ECFG            0x50u    /* enable cfg Space */
#define SWI_EREG            0x51u    /* enable reg Space */
#define SWI_PWDN            0x02u    /* power off by swi command */

enum timeout_case {
	CASE_BEGIN = 0,
	NVM_TIMEOUT,
	RESP_TIMEOUT,
	RESP_TIMEOUT_LONG,
	CASE_END,
};

/* function predefienations */
void optiga_swi_send_byte(uint8_t code, uint8_t data);
int optiga_swi_read_byte(uint8_t *data);
int optiga_swi_init(struct sle95250_dev *di);
void optiga_swi_unloading(void);
void optiga_swi_pwr_on(void);
void optiga_swi_pwr_off(void);
void optiga_swi_pwr_off_by_cmd(void);
void optiga_swi_send_byte(uint8_t code, uint8_t data);
void opiga_select_chip_addr(uint16_t ic_addr);
void optiga_delay(unsigned long cnt);
void optiga_swi_slk_irqsave(void);
void optiga_swi_slk_irqstore(void);
void optiga_gpio_dir_input(void);
void optiga_gpio_dir_output(uint8_t value);
int optiga_gpio_get_value(void);
void optiga_get_gpio_bank_dir(void);
void sle95250_reg_sec_ic_ops(void);
unsigned long optiga_get_timeout_val(enum timeout_case to_case);

#endif /* _OPTIGA_SWI_H_ */
