/*
 * sm5450_protocol.c
 *
 * sm5450 protocol driver
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
 *
 */

#include "sm5450_protocol.h"
#include "sm5450_i2c.h"
#include <linux/delay.h>
#include <linux/mutex.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/protocol/adapter_protocol.h>
#include <chipset_common/hwpower/protocol/adapter_protocol_scp.h>
#include <chipset_common/hwpower/protocol/adapter_protocol_fcp.h>

#define HWLOG_TAG sm5450
HWLOG_REGIST();

static u32 g_sm5450_scp_error_flag;

static int sm5450_scp_wdt_reset_by_sw(void *dev_data)
{
	int ret;
	struct sm5450_device_info *di = dev_data;

	ret = sm5450_write_byte(di, SM5450_SCP_CTL_REG, SM5450_SCP_CTL_WDT_RESET);
	ret += sm5450_write_byte(di, SM5450_SCP_STIMER_REG, SM5450_SCP_STIMER_WDT_RESET);
	ret += sm5450_write_byte(di, SM5450_RT_BUFFER_0_REG, SM5450_RT_BUFFER_0_WDT_RESET);
	ret += sm5450_write_byte(di, SM5450_RT_BUFFER_1_REG, SM5450_RT_BUFFER_1_WDT_RESET);
	ret += sm5450_write_byte(di, SM5450_RT_BUFFER_2_REG, SM5450_RT_BUFFER_2_WDT_RESET);
	ret += sm5450_write_byte(di, SM5450_RT_BUFFER_3_REG, SM5450_RT_BUFFER_3_WDT_RESET);
	ret += sm5450_write_byte(di, SM5450_RT_BUFFER_4_REG, SM5450_RT_BUFFER_4_WDT_RESET);
	ret += sm5450_write_byte(di, SM5450_RT_BUFFER_5_REG, SM5450_RT_BUFFER_5_WDT_RESET);
	ret += sm5450_write_byte(di, SM5450_RT_BUFFER_6_REG, SM5450_RT_BUFFER_6_WDT_RESET);
	ret += sm5450_write_byte(di, SM5450_RT_BUFFER_7_REG, SM5450_RT_BUFFER_7_WDT_RESET);
	ret += sm5450_write_byte(di, SM5450_RT_BUFFER_8_REG, SM5450_RT_BUFFER_8_WDT_RESET);
	ret += sm5450_write_byte(di, SM5450_RT_BUFFER_9_REG, SM5450_RT_BUFFER_9_WDT_RESET);
	ret += sm5450_write_byte(di, SM5450_RT_BUFFER_10_REG, SM5450_RT_BUFFER_10_WDT_RESET);

	return ret;
}

static int sm5450_scp_cmd_transfer_check(void *dev_data)
{
	u8 reg_val1 = 0;
	u8 reg_val2 = 0;
	int i = 0;
	int ret0;
	int ret1;
	struct sm5450_device_info *di = dev_data;

	do {
		usleep_range(50000, 51000); /* wait 50ms for each cycle */
		ret0 = sm5450_read_byte(di, SM5450_SCP_ISR1_REG, &reg_val1);
		ret1 = sm5450_read_byte(di, SM5450_SCP_ISR2_REG, &reg_val2);
		if (ret0 || ret1) {
			hwlog_err("reg read failed\n");
			break;
		}
		hwlog_info("reg_val1(0x%x), reg_val2(0x%x), scp_isr_backup[0] = 0x%x, scp_isr_backup[1] = 0x%x\n",
			reg_val1, reg_val2, di->scp_isr_backup[0], di->scp_isr_backup[1]);
		/* interrupt work can hook the interrupt value first. so it is necessily to do backup via isr_backup */
		reg_val1 |= di->scp_isr_backup[0];
		reg_val2 |= di->scp_isr_backup[1];
		if (reg_val1 || reg_val2) {
			if (((reg_val2 & SM5450_SCP_ISR2_ACK_MASK) &&
				(reg_val2 & SM5450_SCP_ISR2_CMD_CPL_MASK)) &&
				!(reg_val1 & (SM5450_SCP_ISR1_ACK_CRCRX_MASK |
				SM5450_SCP_ISR1_ACK_PARRX_MASK |
				SM5450_SCP_ISR1_ERR_ACK_L_MASK))) {
				return 0;
			} else if (reg_val1 & (SM5450_SCP_ISR1_ACK_CRCRX_MASK |
				SM5450_SCP_ISR1_ENABLE_HAND_NO_RESPOND_MASK)) {
				hwlog_err("scp transfer fail, slave status changed: ISR1 = 0x%x, ISR2 = 0x%x\n",
					reg_val1, reg_val2);
				return -1;
			} else if (reg_val2 & SM5450_SCP_ISR2_NACK_MASK) {
				hwlog_err("scp transfer fail, slave nack: ISR1 = 0x%x, ISR2 = 0x%x\n",
					reg_val1, reg_val2);
				return -1;
			} else if (reg_val1 & (SM5450_SCP_ISR1_ACK_CRCRX_MASK |
				SM5450_SCP_ISR1_ACK_PARRX_MASK |
				SM5450_SCP_ISR1_TRANS_HAND_NO_RESPOND_MASK)) {
				hwlog_err("scp transfer fail, CRCRX_PARRX_ERROR: ISR1 = 0x%x, ISR2 = 0x%x\n",
					reg_val1, reg_val2);
				return -1;
			}
			hwlog_err("scp transfer fail, ISR1 = 0x%x, ISR2 = 0x%x, index = %d\n",
				reg_val1, reg_val2, i);
		}
		i++;
		if (di->dc_ibus_ucp_happened)
			i = SM5450_SCP_ACK_RETRY_CYCLE;
	} while (i < SM5450_SCP_ACK_RETRY_CYCLE);

	hwlog_err("scp adapter transfer time out\n");
	return -1;
}

static int sm5450_scp_cmd_transfer_check_1(void *dev_data)
{
	u8 reg_val1 = 0;
	u8 reg_val2 = 0;
	u8 pre_val1 = 0;
	u8 pre_val2 = 0;
	int i = 0;
	int ret0;
	int ret1;
	struct sm5450_device_info *di = dev_data;

	do {
		usleep_range(12000, 13000); /* wait 12ms between each cycle */
		ret0 = sm5450_read_byte(di, SM5450_SCP_ISR1_REG, &pre_val1);
		ret1 = sm5450_read_byte(di, SM5450_SCP_ISR2_REG, &pre_val2);
		if (ret0 || ret1) {
			hwlog_err("reg read failed\n");
			break;
		}
		hwlog_info("pre_val1(0x%x), pre_val2(0x%x), scp_isr_backup[0](0x%x), scp_isr_backup[1](0x%x)\n",
			pre_val1, pre_val2, di->scp_isr_backup[0], di->scp_isr_backup[1]);
		/* save insterrupt value to reg_val1/2 from starting scp cmd to SLV_R_CPL interrupt */
		reg_val1 |= pre_val1;
		reg_val2 |= pre_val2;
		/* interrupt work can hook the interrupt value first. so it is necessily to do backup via isr_backup */
		reg_val1 |= di->scp_isr_backup[0];
		reg_val2 |= di->scp_isr_backup[1];
		if (reg_val1 || reg_val2) {
			if (((reg_val2 & SM5450_SCP_ISR2_ACK_MASK) &&
				(reg_val2 & SM5450_SCP_ISR2_CMD_CPL_MASK) &&
				(reg_val2 & SM5450_SCP_ISR2_SLV_R_CPL_MASK)) &&
				!(reg_val1 & (SM5450_SCP_ISR1_ACK_CRCRX_MASK |
				SM5450_SCP_ISR1_ACK_PARRX_MASK | SM5450_SCP_ISR1_ERR_ACK_L_MASK))) {
				return 0;
			} else if (reg_val1 & (SM5450_SCP_ISR1_ACK_CRCRX_MASK |
				SM5450_SCP_ISR1_ENABLE_HAND_NO_RESPOND_MASK)) {
				hwlog_err("scp transfer fail, slave status changed: ISR1 = 0x%x, ISR2 = 0x%x\n",
					reg_val1, reg_val2);
				return -1;
			} else if (reg_val2 & SM5450_SCP_ISR2_NACK_MASK) {
				hwlog_err("scp transfer fail, slave nack: ISR1 = 0x%x, ISR2 = 0x%x\n",
					reg_val1, reg_val2);
				return -1;
			} else if (reg_val1 & (SM5450_SCP_ISR1_ACK_CRCRX_MASK |
				SM5450_SCP_ISR1_ACK_PARRX_MASK |
				SM5450_SCP_ISR1_TRANS_HAND_NO_RESPOND_MASK)) {
				hwlog_err("scp transfer fail, CRCRX_PARRX_ERROR: ISR1 = 0x%x, ISR2 = 0x%x\n",
					reg_val1, reg_val2);
				return -1;
			}
			hwlog_err("scp transfer fail, ISR1 = 0x%x, ISR2 = 0x%x, index = %d\n",
				reg_val1, reg_val2, i);
		}
		i++;
		if (di->dc_ibus_ucp_happened)
			i = SM5450_SCP_ACK_RETRY_CYCLE_1;
	} while (i < SM5450_SCP_ACK_RETRY_CYCLE_1);

	hwlog_err("scp adapter transfer time out\n");
	return -1;
}

static void sm5450_scp_protocol_restart(void *dev_data)
{
	u8 reg_val = 0;
	int ret;
	int i;
	struct sm5450_device_info *di = dev_data;

	mutex_lock(&di->scp_detect_lock);

	/* detect scp charger, wait for ping succ */
	for (i = 0; i < SM5450_SCP_RESTART_TIME; i++) {
		usleep_range(9000, 10000); /* wait 9ms for each cycle */
		ret = sm5450_read_byte(di, SM5450_SCP_STATUS_REG, &reg_val);
		if (ret) {
			hwlog_err("read det attach err, ret: %d\n", ret);
			continue;
		}

		if (reg_val & SM5450_SCP_STATUS_ENABLE_HAND_SUCCESS_MASK)
			break;
	}

	if (i == SM5450_SCP_RESTART_TIME) {
		hwlog_err("wait for slave fail\n");
		mutex_unlock(&di->scp_detect_lock);
		return;
	}
	mutex_unlock(&di->scp_detect_lock);
	hwlog_info("disable and enable scp protocol accp status is 0x%x\n", reg_val);
}

static int sm5450_scp_adapter_reg_read(u8 *val, u8 reg, void *dev_data)
{
	int ret;
	int i;
	u8 reg_val1 = 0;
	u8 reg_val2 = 0;
	struct sm5450_device_info *di = dev_data;

	mutex_lock(&di->accp_adapter_reg_lock);
	hwlog_info("CMD = 0x%x, REG = 0x%x\n", SM5450_SCP_CMD_SBRRD, reg);
	for (i = 0; i < SM5450_SCP_RETRY_TIME; i++) {
		/* init */
		sm5450_write_mask(di, SM5450_SCP_CTL_REG, SM5450_SCP_CTL_SNDCMD_MASK,
			SM5450_SCP_CTL_SNDCMD_SHIFT, SM5450_SCP_CTL_SNDCMD_RESET);

		/* before send cmd, clear isr interrupt registers */
		ret = sm5450_read_byte(di, SM5450_SCP_ISR1_REG, &reg_val1);
		ret += sm5450_read_byte(di, SM5450_SCP_ISR2_REG, &reg_val2);
		ret += sm5450_write_byte(di, SM5450_RT_BUFFER_0_REG, SM5450_SCP_CMD_SBRRD);
		ret += sm5450_write_byte(di, SM5450_RT_BUFFER_1_REG, reg);
		ret += sm5450_write_byte(di, SM5450_RT_BUFFER_2_REG, 1);
		/* initial scp_isr_backup[0],[1] due to catching the missing isr by interrupt_work */
		di->scp_isr_backup[0] = 0;
		di->scp_isr_backup[1] = 0;
		ret += sm5450_write_mask(di, SM5450_SCP_CTL_REG, SM5450_SCP_CTL_SNDCMD_MASK,
			SM5450_SCP_CTL_SNDCMD_SHIFT, SM5450_SCP_CTL_SNDCMD_START);
		if (ret) {
			hwlog_err("write error, ret is %d\n", ret);
			/* manual init */
			sm5450_write_mask(di, SM5450_SCP_CTL_REG, SM5450_SCP_CTL_SNDCMD_MASK,
				SM5450_SCP_CTL_SNDCMD_SHIFT, SM5450_SCP_CTL_SNDCMD_RESET);
			mutex_unlock(&di->accp_adapter_reg_lock);
			return -1;
		}

		/* check cmd transfer success or fail */
		if (di->rev_id == 0) {
			if (sm5450_scp_cmd_transfer_check(di) == 0) {
				/* recived data from adapter */
				ret = sm5450_read_byte(di, SM5450_RT_BUFFER_12_REG, val);
				break;
			}
		} else {
			if (sm5450_scp_cmd_transfer_check_1(di) == 0) {
				/* recived data from adapter */
				ret = sm5450_read_byte(di, SM5450_RT_BUFFER_12_REG, val);
				break;
			}
		}

		sm5450_scp_protocol_restart(di);
		if (di->dc_ibus_ucp_happened)
			i = SM5450_SCP_RETRY_TIME;
	}
	if (i >= SM5450_SCP_RETRY_TIME) {
		hwlog_err("ack error, retry %d times\n", i);
		ret = -1;
	}
	/* manual init */
	sm5450_write_mask(di, SM5450_SCP_CTL_REG, SM5450_SCP_CTL_SNDCMD_MASK,
		SM5450_SCP_CTL_SNDCMD_SHIFT, SM5450_SCP_CTL_SNDCMD_RESET);
	usleep_range(10000, 11000); /* wait 10ms for operate effective */

	mutex_unlock(&di->accp_adapter_reg_lock);

	return ret;
}

static int sm5450_scp_adapter_reg_read_block(u8 reg, u8 *val, u8 num,
	void *dev_data)
{
	int ret;
	int i;
	u8 reg_val1 = 0;
	u8 reg_val2 = 0;
	u8 *p = val;
	u8 data_len = (num < SM5450_SCP_DATA_LEN) ? num : SM5450_SCP_DATA_LEN;
	struct sm5450_device_info *di = dev_data;

	if (!di) {
		hwlog_err("di is null\n");
		return -ENOMEM;
	}
	mutex_lock(&di->accp_adapter_reg_lock);

	hwlog_info("CMD = 0x%x, REG = 0x%x, Num = 0x%x\n",
		SM5450_SCP_CMD_MBRRD, reg, data_len);

	for (i = 0; i < SM5450_SCP_RETRY_TIME; i++) {
		/* init */
		sm5450_write_mask(di, SM5450_SCP_CTL_REG, SM5450_SCP_CTL_SNDCMD_MASK,
			SM5450_SCP_CTL_SNDCMD_SHIFT, SM5450_SCP_CTL_SNDCMD_RESET);

		/* before sending cmd, clear isr registers */
		ret = sm5450_read_byte(di, SM5450_SCP_ISR1_REG, &reg_val1);
		ret += sm5450_read_byte(di, SM5450_SCP_ISR2_REG, &reg_val2);
		ret += sm5450_write_byte(di, SM5450_RT_BUFFER_0_REG, SM5450_SCP_CMD_MBRRD);
		ret += sm5450_write_byte(di, SM5450_RT_BUFFER_1_REG, reg);
		ret += sm5450_write_byte(di, SM5450_RT_BUFFER_2_REG, data_len);
		/* initial scp_isr_backup[0],[1] */
		di->scp_isr_backup[0] = 0;
		di->scp_isr_backup[1] = 0;
		ret += sm5450_write_mask(di, SM5450_SCP_CTL_REG, SM5450_SCP_CTL_SNDCMD_MASK,
			SM5450_SCP_CTL_SNDCMD_SHIFT, SM5450_SCP_CTL_SNDCMD_START);
		if (ret) {
			hwlog_err("read error ret is %d\n", ret);
			/* manual init */
			sm5450_write_mask(di, SM5450_SCP_CTL_REG, SM5450_SCP_CTL_SNDCMD_MASK,
				SM5450_SCP_CTL_SNDCMD_SHIFT, SM5450_SCP_CTL_SNDCMD_RESET);
			mutex_unlock(&di->accp_adapter_reg_lock);
			return -1;
		}

		/* check cmd transfer success or fail */
		if (di->rev_id == 0) {
			if (sm5450_scp_cmd_transfer_check(di) == 0) {
				/* recived data from adapter */
				ret = sm5450_read_block(di, p, SM5450_RT_BUFFER_12_REG, data_len);
				break;
			}
		} else {
			if (sm5450_scp_cmd_transfer_check_1(di) == 0) {
				/* recived data from adapter */
				ret = sm5450_read_block(di, p, SM5450_RT_BUFFER_12_REG, data_len);
				break;
			}
		}

		sm5450_scp_protocol_restart(di);
		if (di->dc_ibus_ucp_happened)
			i = SM5450_SCP_RETRY_TIME;
	}
	if (i >= SM5450_SCP_RETRY_TIME) {
		hwlog_err("ack error, retry %d times\n", i);
		ret = -1;
	}
	mutex_unlock(&di->accp_adapter_reg_lock);

	if (ret) {
		/* manual init */
		sm5450_write_mask(di, SM5450_SCP_CTL_REG, SM5450_SCP_CTL_SNDCMD_MASK,
			SM5450_SCP_CTL_SNDCMD_SHIFT, SM5450_SCP_CTL_SNDCMD_RESET);
		return ret;
	}

	num -= data_len;
	/* max is SM5450_SCP_DATA_LEN. remaining data is read in below */
	if (num) {
		p += data_len;
		reg += data_len;
		ret = sm5450_scp_adapter_reg_read_block(reg, p, num, di);
		if (ret) {
			/* manual init */
			sm5450_write_mask(di, SM5450_SCP_CTL_REG, SM5450_SCP_CTL_SNDCMD_MASK,
				SM5450_SCP_CTL_SNDCMD_SHIFT, SM5450_SCP_CTL_SNDCMD_RESET);
			hwlog_err("read error, ret is %d\n", ret);
			return -1;
		}
	}
	/* manual init */
	sm5450_write_mask(di, SM5450_SCP_CTL_REG, SM5450_SCP_CTL_SNDCMD_MASK,
		SM5450_SCP_CTL_SNDCMD_SHIFT, SM5450_SCP_CTL_SNDCMD_RESET);
	usleep_range(10000, 11000); /* wait 10ms for operate effective */

	return 0;
}

static int sm5450_scp_adapter_reg_write(u8 val, u8 reg, void *dev_data)
{
	int ret;
	int i;
	u8 reg_val1 = 0;
	u8 reg_val2 = 0;
	struct sm5450_device_info *di = dev_data;

	mutex_lock(&di->accp_adapter_reg_lock);
	hwlog_info("CMD = 0x%x, REG = 0x%x, val = 0x%x\n",
		SM5450_SCP_CMD_SBRWR, reg, val);
	for (i = 0; i < SM5450_SCP_RETRY_TIME; i++) {
		/* init */
		sm5450_write_mask(di, SM5450_SCP_CTL_REG, SM5450_SCP_CTL_SNDCMD_MASK,
			SM5450_SCP_CTL_SNDCMD_SHIFT, SM5450_SCP_CTL_SNDCMD_RESET);

		/* before send cmd, clear accp interrupt registers */
		ret = sm5450_read_byte(di, SM5450_SCP_ISR1_REG, &reg_val1);
		ret += sm5450_read_byte(di, SM5450_SCP_ISR2_REG, &reg_val2);
		ret += sm5450_write_byte(di, SM5450_RT_BUFFER_0_REG, SM5450_SCP_CMD_SBRWR);
		ret += sm5450_write_byte(di, SM5450_RT_BUFFER_1_REG, reg);
		ret += sm5450_write_byte(di, SM5450_RT_BUFFER_2_REG, 1);
		ret += sm5450_write_byte(di, SM5450_RT_BUFFER_3_REG, val);
		/* initial scp_isr_backup[0],[1] */
		di->scp_isr_backup[0] = 0;
		di->scp_isr_backup[1] = 0;
		ret += sm5450_write_mask(di, SM5450_SCP_CTL_REG, SM5450_SCP_CTL_SNDCMD_MASK,
			SM5450_SCP_CTL_SNDCMD_SHIFT, SM5450_SCP_CTL_SNDCMD_START);
		if (ret) {
			hwlog_err("write error, ret is %d\n", ret);
			/* manual init */
			sm5450_write_mask(di, SM5450_SCP_CTL_REG, SM5450_SCP_CTL_SNDCMD_MASK,
				SM5450_SCP_CTL_SNDCMD_SHIFT, SM5450_SCP_CTL_SNDCMD_RESET);
			mutex_unlock(&di->accp_adapter_reg_lock);
			return -1;
		}

		/* check cmd transfer success or fail */
		if (di->rev_id == 0) {
			if (sm5450_scp_cmd_transfer_check(di) == 0)
				break;
		} else {
			if (sm5450_scp_cmd_transfer_check_1(di) == 0)
				break;
		}

		sm5450_scp_protocol_restart(di);
		if (di->dc_ibus_ucp_happened)
			i = SM5450_SCP_RETRY_TIME;
	}
	if (i >= SM5450_SCP_RETRY_TIME) {
		hwlog_err("ack error, retry %d times\n", i);
		ret = -1;
	}
	/* manual init */
	sm5450_write_mask(di, SM5450_SCP_CTL_REG, SM5450_SCP_CTL_SNDCMD_MASK,
		SM5450_SCP_CTL_SNDCMD_SHIFT, SM5450_SCP_CTL_SNDCMD_RESET);
	usleep_range(10000, 11000); /* wait 10ms for operate effective */

	mutex_unlock(&di->accp_adapter_reg_lock);
	return ret;
}

static int sm5450_fcp_adapter_vol_check(int adapter_vol_mv, void *dev_data)
{
	int i;
	int ret;
	int adc_vol = 0;

	if ((adapter_vol_mv < SM5450_FCP_ADAPTER_MIN_VOL) ||
		(adapter_vol_mv > SM5450_FCP_ADAPTER_MAX_VOL)) {
		hwlog_err("check vol out of range, input vol = %dmV\n", adapter_vol_mv);
		return -1;
	}

	for (i = 0; i < SM5450_FCP_ADAPTER_VOL_CHECK_TIMEOUT; i++) {
		ret = sm5450_get_vbus_mv((unsigned int *)&adc_vol, dev_data);
		if (ret)
			continue;
		if ((adc_vol > (adapter_vol_mv - SM5450_FCP_ADAPTER_VOL_CHECK_ERROR)) &&
			(adc_vol < (adapter_vol_mv + SM5450_FCP_ADAPTER_VOL_CHECK_ERROR)))
			break;
		msleep(SM5450_FCP_ADAPTER_VOL_CHECK_POLLTIME);
	}

	if (i == SM5450_FCP_ADAPTER_VOL_CHECK_TIMEOUT) {
		hwlog_err("check vol timeout, input vol = %dmV\n", adapter_vol_mv);
		return -1;
	}
	hwlog_info("check vol success, input vol = %dmV, spend %dms\n",
		adapter_vol_mv, i * SM5450_FCP_ADAPTER_VOL_CHECK_POLLTIME);
	return 0;
}

static int sm5450_fcp_master_reset(void *dev_data)
{
	struct sm5450_device_info *di = dev_data;
	int ret;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	ret = sm5450_scp_wdt_reset_by_sw(di);
	if (ret)
		return -1;

	usleep_range(10000, 11000); /* wait 10ms for operate effective */

	/* clear SCP_CTL, enabled EN_STIMER */
	return sm5450_write_byte(di, SM5450_SCP_CTL_REG, SM5450_SCP_CTL_REG_INIT);
}

static int sm5450_fcp_adapter_reset(void *dev_data)
{
	int ret;
	struct sm5450_device_info *di = dev_data;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	ret = sm5450_write_mask(di, SM5450_SCP_CTL_REG, SM5450_SCP_CTL_MSTR_RST_MASK,
		SM5450_SCP_CTL_MSTR_RST_SHIFT, 1);
	usleep_range(20000, 21000); /* wait 20ms for operate effective */
	ret += sm5450_fcp_adapter_vol_check(SM5450_FCP_ADAPTER_RST_VOL, di); /* set 5V */
	if (ret) {
		hwlog_err("fcp adapter reset error\n");
		return sm5450_scp_wdt_reset_by_sw(di);
	}
	ret = sm5450_config_vbuscon_ovp_ref_mv(SM5450_VBUS_OVP_REF_RESET, di);
	ret += sm5450_config_vbus_ovp_ref_mv(SM5450_VBUSCON_OVP_REF_RESET, di);

	return ret;
}

static int sm5450_fcp_read_switch_status(void *dev_data)
{
	return 0;
}

static int sm5450_is_fcp_charger_type(void *dev_data)
{
	u8 reg_val = 0;
	int ret;
	struct sm5450_device_info *di = dev_data;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	if (sm5450_is_support_fcp(di))
		return 0;

	ret = sm5450_read_byte(di, SM5450_SCP_STATUS_REG, &reg_val);
	if (ret)
		return 0;

	if (reg_val & SM5450_SCP_STATUS_ENABLE_HAND_SUCCESS_MASK)
		return 1;
	return 0;
}

static int sm5450_fcp_adapter_detect(struct sm5450_device_info *di)
{
	u8 reg_val = 0;
	int vbus_uvp;
	int i;
	int ret;
	u8 reg_val1 = 0;

	mutex_lock(&di->scp_detect_lock);
	/* temp */
	ret = sm5450_write_mask(di, SM5450_CONTROL3_REG, SM5450_CONTROL3_EN_COMP_MASK,
		SM5450_CONTROL3_EN_COMP_SHIFT, 1);
	ret += sm5450_read_byte(di, SM5450_SCP_STATUS_REG, &reg_val);
	ret += sm5450_read_byte(di, SM5450_CONTROL3_REG, &reg_val1);

	hwlog_info("SM5450_SCP_STATUS_REG: 0x%x, reg_val1 = 0x%x\n", reg_val, reg_val1);
	if (ret) {
		hwlog_err("read det attach err, ret:%d\n", ret);
		mutex_unlock(&di->scp_detect_lock);
		return -1;
	}

	/* confirm enable hand success status */
	if (reg_val & SM5450_SCP_STATUS_ENABLE_HAND_SUCCESS_MASK) {
		mutex_unlock(&di->scp_detect_lock);
		hwlog_info("scp adapter detect ok\n");
		return ADAPTER_DETECT_SUCC;
	}
	ret = sm5450_write_mask(di, SM5450_SCP_CTL_REG, SM5450_SCP_CTL_EN_SCP_MASK,
		SM5450_SCP_CTL_EN_SCP_SHIFT, 1);
	ret += sm5450_write_mask(di, SM5450_SCP_CTL_REG, SM5450_SCP_CTL_SCP_DET_EN_MASK,
		SM5450_SCP_CTL_SCP_DET_EN_SHIFT, 1);
	if (ret) {
		hwlog_err("SCP enable detect fail, ret is %d\n", ret);
		sm5450_write_mask(di, SM5450_SCP_CTL_REG, SM5450_SCP_CTL_EN_SCP_MASK,
			SM5450_SCP_CTL_EN_SCP_SHIFT, 0);
		sm5450_write_mask(di, SM5450_SCP_CTL_REG, SM5450_SCP_CTL_SCP_DET_EN_MASK,
			SM5450_SCP_CTL_SCP_DET_EN_SHIFT, 0);
		/* reset scp registers when EN_SCP is changed to 0 */
		sm5450_scp_wdt_reset_by_sw(di);
		sm5450_fcp_adapter_reset(di);
		mutex_unlock(&di->scp_detect_lock);
		return -1;
	}
	/* waiting for scp set */
	for (i = 0; i < SM5450_SCP_DETECT_MAX_COUT; i++) {
		ret = sm5450_read_byte(di, SM5450_SCP_STATUS_REG, &reg_val);
		vbus_uvp = 0;
		hwlog_info("SM5450_SCP_STATUS_REG 0x%x\n", reg_val);
		if (ret) {
			hwlog_err("read det attach err, ret:%d\n", ret);
			continue;
		}
		if (vbus_uvp) {
			hwlog_err("0x%x vbus uv happen, adapter plug out\n", vbus_uvp);
			break;
		}
		if (reg_val & SM5450_SCP_STATUS_ENABLE_HAND_SUCCESS_MASK)
			break;
		msleep(SM5450_SCP_POLL_TIME);
	}
	if ((i == SM5450_SCP_DETECT_MAX_COUT) || vbus_uvp) {
		sm5450_write_mask(di, SM5450_SCP_CTL_REG, SM5450_SCP_CTL_EN_SCP_MASK,
			SM5450_SCP_CTL_EN_SCP_SHIFT, 0);
		sm5450_write_mask(di, SM5450_SCP_CTL_REG, SM5450_SCP_CTL_SCP_DET_EN_MASK,
			SM5450_SCP_CTL_SCP_DET_EN_SHIFT, 0);
		/* reset scp registers when EN_SCP is changed to 0 */
		sm5450_scp_wdt_reset_by_sw(di);
		sm5450_fcp_adapter_reset(di);
		hwlog_err("CHG_SCP_ADAPTER_DETECT_OTHER return\n");
		mutex_unlock(&di->scp_detect_lock);
		return ADAPTER_DETECT_OTHER;
	}

	mutex_unlock(&di->scp_detect_lock);
	return ret;
}

static int sm5450_fcp_stop_charge_config(void *dev_data)
{
	struct sm5450_device_info *di = dev_data;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	sm5450_fcp_master_reset(di);
	sm5450_write_mask(di, SM5450_SCP_CTL_REG, SM5450_SCP_CTL_SCP_DET_EN_MASK,
		SM5450_SCP_CTL_SCP_DET_EN_SHIFT, 0);

	return 0;
}

static int sm5450_scp_reg_read(u8 *val, u8 reg, void *dev_data)
{
	int ret;

	if (g_sm5450_scp_error_flag) {
		hwlog_err("scp timeout happened, do not read reg = 0x%x\n", reg);
		return -1;
	}

	ret = sm5450_scp_adapter_reg_read(val, reg, dev_data);
	if (ret) {
		hwlog_err("error reg = 0x%x\n", reg);
		if (reg != SCP_PROTOCOL_ADP_TYPE0)
			g_sm5450_scp_error_flag = SM5450_SCP_IS_ERR;

		return -1;
	}

	return 0;
}

static int sm5450_scp_reg_write(u8 val, u8 reg, void *dev_data)
{
	int ret;

	if (g_sm5450_scp_error_flag) {
		hwlog_err("scp timeout happened, do not write reg = 0x%x\n", reg);
		return -1;
	}

	ret = sm5450_scp_adapter_reg_write(val, reg, dev_data);
	if (ret) {
		hwlog_err("error reg = 0x%x\n", reg);
		g_sm5450_scp_error_flag = SM5450_SCP_IS_ERR;
		return -1;
	}

	return 0;
}

static int sm5450_self_check(void *dev_data)
{
	return 0;
}

static int sm5450_scp_chip_reset(void *dev_data)
{
	return sm5450_fcp_master_reset(dev_data);
}

static int sm5450_scp_reg_read_block(int reg, int *val, int num,
	void *dev_data)
{
	int ret;
	int i;
	u8 data = 0;

	if (!val || !dev_data) {
		hwlog_err("val or dev_data is null\n");
		return -1;
	}

	g_sm5450_scp_error_flag = SM5450_SCP_NO_ERR;

	for (i = 0; i < num; i++) {
		ret = sm5450_scp_reg_read(&data, reg + i, dev_data);
		if (ret) {
			hwlog_err("scp read failed, reg=0x%x\n", reg + i);
			return -1;
		}
		val[i] = data;
	}

	return 0;
}

static int sm5450_scp_reg_write_block(int reg, const int *val, int num,
	void *dev_data)
{
	int ret;
	int i;

	if (!val || !dev_data) {
		hwlog_err("val or dev_data is null\n");
		return -1;
	}

	g_sm5450_scp_error_flag = SM5450_SCP_NO_ERR;

	for (i = 0; i < num; i++) {
		ret = sm5450_scp_reg_write(val[i], reg + i, dev_data);
		if (ret) {
			hwlog_err("scp write failed, reg=0x%x\n", reg + i);
			return -1;
		}
	}

	return 0;
}

static int sm5450_scp_detect_adapter(void *dev_data)
{
	struct sm5450_device_info *di = dev_data;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	return sm5450_fcp_adapter_detect(di);
}

static int sm5450_fcp_reg_read_block(int reg, int *val, int num,
	void *dev_data)
{
	int ret, i;
	u8 data = 0;

	if (!val || !dev_data) {
		hwlog_err("val or dev_data is null\n");
		return -1;
	}

	g_sm5450_scp_error_flag = SM5450_SCP_NO_ERR;

	for (i = 0; i < num; i++) {
		ret = sm5450_scp_reg_read(&data, reg + i, dev_data);
		if (ret) {
			hwlog_err("fcp read failed, reg=0x%x\n", reg + i);
			return -1;
		}
		val[i] = data;
	}
	return 0;
}

static int sm5450_fcp_reg_write_block(int reg, const int *val, int num,
	void *dev_data)
{
	int ret, i;

	if (!val || !dev_data) {
		hwlog_err("val or dev_data is null\n");
		return -1;
	}

	g_sm5450_scp_error_flag = SM5450_SCP_NO_ERR;

	for (i = 0; i < num; i++) {
		ret = sm5450_scp_reg_write(val[i], reg + i, dev_data);
		if (ret) {
			hwlog_err("fcp write failed, reg=0x%x\n", reg + i);
			return -1;
		}
	}

	return 0;
}

static int sm5450_fcp_detect_adapter(void *dev_data)
{
	struct sm5450_device_info *di = dev_data;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	return sm5450_fcp_adapter_detect(di);
}

static int sm5450_pre_init(void *dev_data)
{
	struct sm5450_device_info *di = dev_data;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	return sm5450_self_check(di);
}

static int sm5450_scp_adapter_reset(void *dev_data)
{
	return sm5450_fcp_adapter_reset(dev_data);
}

static struct scp_protocol_ops sm5450_scp_protocol_ops = {
	.chip_name = "sm5450",
	.reg_read = sm5450_scp_reg_read_block,
	.reg_write = sm5450_scp_reg_write_block,
	.reg_multi_read = sm5450_scp_adapter_reg_read_block,
	.detect_adapter = sm5450_scp_detect_adapter,
	.soft_reset_master = sm5450_scp_chip_reset,
	.soft_reset_slave = sm5450_scp_adapter_reset,
	.pre_init = sm5450_pre_init,
};

static struct fcp_protocol_ops sm5450_fcp_protocol_ops = {
	.chip_name = "sm5450",
	.reg_read = sm5450_fcp_reg_read_block,
	.reg_write = sm5450_fcp_reg_write_block,
	.detect_adapter = sm5450_fcp_detect_adapter,
	.soft_reset_master = sm5450_fcp_master_reset,
	.soft_reset_slave = sm5450_fcp_adapter_reset,
	.get_master_status = sm5450_fcp_read_switch_status,
	.stop_charging_config = sm5450_fcp_stop_charge_config,
	.is_accp_charger_type = sm5450_is_fcp_charger_type,
};

int sm5450_scp_protocol_register(struct sm5450_device_info *di)
{
	sm5450_scp_protocol_ops.dev_data = (void *)di;
	return scp_protocol_ops_register(&sm5450_scp_protocol_ops);
}

int sm5450_fcp_protocol_register(struct sm5450_device_info *di)
{
	sm5450_fcp_protocol_ops.dev_data = (void *)di;
	return fcp_protocol_ops_register(&sm5450_fcp_protocol_ops);
}
