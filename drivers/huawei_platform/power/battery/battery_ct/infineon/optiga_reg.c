/*
 * optiga_reg.c
 *
 * optiga register operation function
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

#include <linux/timex.h>
#include <huawei_platform/log/hw_log.h>

#include "include/optiga_swi.h"
#include "include/optiga_reg.h"
#include "include/optiga_nvm.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG battct_reg
HWLOG_REGIST();

static void optiga_chip_reset(void)
{
	optiga_swi_pwr_off();
	optiga_swi_pwr_on();
	opiga_select_chip_addr(OPTIGA_SINGLE_CHIP_ADDR);
}

void optiga_reg_write(uint16_t reg_addr, uint8_t data, bool cfg_spc)
{
	if (cfg_spc)
		optiga_swi_send_byte(SWI_BC, SWI_ECFG);
	else
		optiga_swi_send_byte(SWI_BC, SWI_EREG);

	optiga_swi_send_byte(SWI_ERA, (reg_addr >> BIT_P_BYT) & 0xff);
	optiga_swi_send_byte(SWI_WRA, reg_addr & 0xff);

	optiga_swi_send_byte(SWI_WD, data);
}

int optiga_reg_read(uint16_t reg_addr, uint8_t *data, bool cfg_spc)
{
	int ret;
	int retry;
	uint8_t data_loc;

	for (retry = 0; retry < REG_RD_RTY; retry++) {
		optiga_swi_send_byte(SWI_BC, SWI_RLEN0);
		if (cfg_spc)
			optiga_swi_send_byte(SWI_BC, SWI_ECFG);
		else
			optiga_swi_send_byte(SWI_BC, SWI_EREG);

		optiga_swi_send_byte(SWI_ERA, (reg_addr >> BIT_P_BYT) & 0xff);
		optiga_swi_send_byte(SWI_RRA, reg_addr & 0xff);
		ret = optiga_swi_read_byte(&data_loc);
		if (ret) {
			if (ret == -SWI_ERR_RESP_TO)
				optiga_chip_reset();
			continue;
		}

		*data = data_loc;

		optiga_swi_send_byte(SWI_ERA, (reg_addr >> BIT_P_BYT) & 0xff);
		optiga_swi_send_byte(SWI_RRA, reg_addr & 0xff);
		ret = optiga_swi_read_byte(&data_loc);
		if (ret) {
			if (ret == -SWI_ERR_RESP_TO)
				optiga_chip_reset();
			continue;
		}

		ret = (*data != data_loc);
		if (ret)
			continue;

		break;
	}

	return ret;
}

int optiga_reg_uid_read(uint8_t *uid)
{
	int ret;
	int retry;
	int byt_ind;
	int byt_ind_last;
	int len2read;
	uint16_t reg_addr;
	uint8_t uid0[SLE95250_UID_LEN];
	uint8_t uid1[SLE95250_UID_LEN];

	byt_ind_last = 0;

	for (retry = 0; retry < OPTIGA_REG_UIDR_RTY; retry++) {
		len2read = SLE95250_UID_LEN;
		optiga_swi_send_byte(SWI_BC, SWI_RLEN0);
		optiga_swi_send_byte(SWI_BC, SWI_ECFG);
		for (byt_ind = byt_ind_last; byt_ind < len2read; byt_ind++) {
			reg_addr = REG_UID + byt_ind;
			optiga_swi_send_byte(SWI_ERA, (reg_addr >> BIT_P_BYT) & 0xff);
			optiga_swi_send_byte(SWI_RRA, reg_addr & 0xff);
			ret = optiga_swi_read_byte(&uid0[byt_ind]);
			if (ret) {
				if (ret == -SWI_ERR_RESP_TO)
					optiga_chip_reset();
				byt_ind_last = byt_ind;
				break;
			}

			optiga_swi_send_byte(SWI_ERA, (reg_addr >> BIT_P_BYT) & 0xff);
			optiga_swi_send_byte(SWI_RRA, reg_addr & 0xff);
			ret = optiga_swi_read_byte(&uid1[byt_ind]);
			if (ret) {
				if (ret == -SWI_ERR_RESP_TO)
					optiga_chip_reset();
				byt_ind_last = byt_ind;
				break;
			}

			ret = (uid0[byt_ind] != uid1[byt_ind]);
			if (ret) {
				byt_ind_last = byt_ind;
				break;
			}
		}
		if (ret)
			continue;

		break;
	}

	if (!ret)
		memcpy(uid, uid0, len2read);

	return 0;
}

int optiga_reg_pglk_read(uint8_t *pglk)
{
	return optiga_reg_read(REG_NVM_LOCK_ST, pglk, false);
}

int optiga_reg_pglk_set(uint8_t pglk_mask)
{
	int ret;
	int retry;
	uint8_t pglk;

	for (retry = 0; retry < SLE95250_PGLK_RTY; retry++) {
		optiga_reg_write(REG_NVM_LOCK, pglk_mask, false);

		optiga_delay(REG_WAIT_MOTION_DONE);

		ret = optiga_reg_read(REG_NVM_LOCK_ST, &pglk, false);
		if (ret)
			return ret;

		ret = (pglk != pglk_mask);
		if (ret)
			continue;

		break;
	}

	return ret;
}

int optiga_reg_life_span_lock_read(uint8_t *cyclk_status)
{
	return optiga_reg_read(REG_PRO_CTR, cyclk_status, false);
}

int optiga_reg_life_span_lock_set(void)
{
	int ret;
	int retry;
	uint8_t cyclk_status;

	for (retry = 0; retry < OPTIGA_REG_CYCLK_RTY; retry++) {
		optiga_reg_write(REG_PRO_CTR, PRO_CTR_CYCLK, false);
		optiga_delay(REG_WAIT_MOTION_DONE);
		ret = optiga_reg_life_span_lock_read(&cyclk_status);
		if (ret)
			return ret;

		ret = !(cyclk_status & PRO_CTR_CYCLK);
		if (ret)
			continue;

		break;
	}

	return ret;
}
