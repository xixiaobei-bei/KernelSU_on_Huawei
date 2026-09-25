/*
 * optiga_nvm.c
 *
 * optiga nvm operation function
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#include "include/optiga_common.h"
#include "include/optiga_swi.h"
#include "include/optiga_reg.h"
#include "include/optiga_nvm.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG battct_nvm
HWLOG_REGIST();

int optiga_nvm_read(uint8_t nvm_area_mask, uint8_t pgind, int byte_offset,
	int read_len, uint8_t *data_ret)
{
	int i;
	int ret;
	int rty;
	uint8_t data;
	uint8_t ctr2mask;
	uint8_t pgdatai;
	uint8_t pgdata[read_len];
	unsigned long start;
	unsigned long timeout = optiga_get_timeout_val(NVM_TIMEOUT);

	for (rty = 0; rty < OPTIGA_NVM_RTY; rty++) {
		optiga_reg_write(REG_NVM_ADDR, OPTIGA_NVM_OPR_LEN8, false);
		ret = optiga_reg_read(REG_UID11, &data, true);
		if (ret)
			continue;

		ctr2mask = OPTIGA_NVM_ACT | OPTIGA_WIP2 | (nvm_area_mask + pgind);
		optiga_reg_write(REG_CTRL2, ctr2mask, false);
		start = get_cycles();
		while ((get_cycles() - start) < timeout) {
			ret = optiga_reg_read(REG_CTRL2, &data, false);
			if (ret)
				break;

			ret = (data & NVM_PROCESS_ON_GOING);
			if (ret)
				continue;

			break;
		}
		if (ret)
			continue;

		ret = (data != (ctr2mask ^ NVM_PROCESS_ON_GOING));
		if (ret) {
			hwlog_err("%s: page addr write fail\n", __func__);
			continue;
		}

		for (i = byte_offset; i < byte_offset + read_len; i++) {
			ret = optiga_reg_read(REG_WIP2 + i, &pgdatai, false);
			if (ret)
				break;
			pgdata[i - byte_offset] = pgdatai;
		}

		if (ret)
			continue;

		break;
	}

	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return ret;
	}

	memcpy(data_ret, pgdata, read_len);

	return 0;
}

int optiga_nvm_write(uint8_t nvm_area_mask, uint8_t pgind, int byte_offset,
	int write_len, uint8_t *data)
{
	int i;
	int ret;
	int retry;
	unsigned long start;
	uint8_t ctr2mask;
	uint8_t data_temp;
	uint8_t opr_len;
	uint8_t nvm_mask;
	uint8_t wip0_data[BYT_P_PG_REG] = { 0 };
	unsigned long timeout = optiga_get_timeout_val(NVM_TIMEOUT);

	if ((byte_offset + write_len) > BYT_P_PG_NVM) {
		hwlog_err("%s: nvm write over page\n", __func__);
		return -EINVAL;
	}

	switch (write_len) {
	case 1:
		opr_len = OPTIGA_NVM_OPR_LEN1;
		break;
	case 2:
		opr_len = OPTIGA_NVM_OPR_LEN2;
		break;
	case 4:
		opr_len = OPTIGA_NVM_OPR_LEN4;
		break;
	case 8:
		opr_len = OPTIGA_NVM_OPR_LEN8;
		break;
	default:
		hwlog_err("%s: invalid len\n", __func__);
		return -EINVAL;
	}

	nvm_mask = ((uint8_t)byte_offset) | opr_len;
	ctr2mask = OPTIGA_NVM_ACT | OPTIGA_NVM_W | OPTIGA_WIP0 | (nvm_area_mask + pgind);

	memcpy(wip0_data + byte_offset, data, write_len);
	/* dump data into wip0 */
	for (i = 0; i < BYT_P_PG_REG; i++)
		optiga_reg_write(REG_WIP0 + i, wip0_data[i], false);

	for (retry = 0; retry < OPTIGA_NVM_RTY; retry++) {
		optiga_reg_write(REG_NVM_ADDR, nvm_mask, false);
		ret = optiga_reg_read(REG_UID11, &data_temp, true);
		if (ret)
			continue;
		optiga_reg_write(REG_CTRL2, ctr2mask, false);
		start = get_cycles();
		while ((get_cycles() - start) < timeout) {
			ret = optiga_reg_read(REG_CTRL2, &data_temp, false);
			if (ret)
				break;

			ret = (data_temp & NVM_PROCESS_ON_GOING);
			if (ret)
				continue;

			break;
		}
		if (ret)
			continue;
	}

	if (ret)
		hwlog_err("%s: fail\n", __func__);

	return ret;
}
