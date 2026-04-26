/*
 * sc8545_protocol.c
 *
 * sc8545 protocol driver
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#include "sc8545_protocol.h"
#include "sc8545_i2c.h"
#include <linux/mutex.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/protocol/adapter_protocol.h>
#include <chipset_common/hwpower/protocol/adapter_protocol_scp.h>
#include <chipset_common/hwpower/protocol/adapter_protocol_fcp.h>

#define HWLOG_TAG sc8545
HWLOG_REGIST();

static bool sc8545_scp_check_data(void *dev_data)
{
	int ret;
	int i;
	u8 scp_stat = 0;
	u8 fifo_stat = 0;
	u8 data_num;
	struct sc8545_device_info *di = dev_data;

	ret = sc8545_read_byte(di, SC8545_SCP_STAT_REG, &scp_stat);
	ret += sc8545_read_byte(di, SC8545_SCP_FIFO_STAT_REG, &fifo_stat);
	if (ret)
		return false;

	data_num = fifo_stat & SC8545_RX_FIFO_CNT_STAT_MASK;
	for (i = 0; i < data_num; i++) {
		(void)sc8545_read_byte(di, SC8545_SCP_RX_DATA_REG,
			&di->scp_data[i]);
		hwlog_info("read scp_data=0x%x\n", di->scp_data[i]);
	}
	hwlog_info("scp_stat=0x%x,fifo_stat=0x%x,rx_num=%d\n",
		scp_stat, fifo_stat, data_num);
	hwlog_info("scp_op_num=%d,scp_op=%d\n", di->scp_op_num, di->scp_op);

	/* first scp data should be ack(0x08 or 0x88) */
	if (((di->scp_data[0] & 0x0F) == SC8545_SCP_ACK) &&
		(scp_stat == SC8545_SCP_NO_ERR) &&
		(((di->scp_op == SC8545_SCP_WRITE_OP) &&
		(data_num == SC8545_SCP_ACK_AND_CRC_LEN)) ||
		((di->scp_op == SC8545_SCP_READ_OP) &&
		(data_num == di->scp_op_num + SC8545_SCP_ACK_AND_CRC_LEN))))
		return true;

	return false;
}

static int sc8545_scp_cmd_transfer_check(void *dev_data)
{
	int cnt = 0;
	struct sc8545_device_info *di = dev_data;

	do {
		(void)power_msleep(DT_MSLEEP_50MS, 0, NULL);
		if (di->scp_trans_done) {
			if (sc8545_scp_check_data(di)) {
				hwlog_info("scp_trans success\n");
				return 0;
			}
			hwlog_err("scp_trans_done, but data err\n");
			return -EINVAL;
		}
		cnt++;
	} while (cnt < SC8545_SCP_ACK_RETRY_TIME);

	hwlog_err("scp adapter trans time out\n");
	return -EINVAL;
}

static int sc8545_scp_adapter_reg_read(u8 *val, u8 reg, void *dev_data)
{
	int ret;
	int i;
	struct sc8545_device_info *di = dev_data;

	mutex_lock(&di->accp_adapter_reg_lock);

	/* clear scp data */
	memset(di->scp_data, 0, sizeof(di->scp_data));

	di->scp_op = SC8545_SCP_READ_OP;
	di->scp_op_num = SC8545_SCP_SBRWR_NUM;
	di->scp_trans_done = false;
	for (i = 0; i < SC8545_SCP_RETRY_TIME; i++) {
		/* clear tx/rx fifo */
		ret = sc8545_write_mask(di, SC8545_SCP_CTRL_REG,
			SC8545_CLR_TX_FIFO_MASK, SC8545_CLR_TX_FIFO_SHIFT, 1);
		ret += sc8545_write_mask(di, SC8545_SCP_CTRL_REG,
			SC8545_CLR_RX_FIFO_MASK, SC8545_CLR_RX_FIFO_SHIFT, 1);
		/* write data */
		ret += sc8545_write_byte(di, SC8545_SCP_TX_DATA_REG,
			SC8545_SCP_CMD_SBRRD);
		ret += sc8545_write_byte(di, SC8545_SCP_TX_DATA_REG, reg);
		/* start trans */
		ret += sc8545_write_mask(di, SC8545_SCP_CTRL_REG,
			SC8545_SND_START_TRANS_MASK,
			SC8545_SND_START_TRANS_SHIFT, 1);
		if (ret) {
			mutex_unlock(&di->accp_adapter_reg_lock);
			return -EIO;
		}
		/* check cmd transfer success or fail */
		if (sc8545_scp_cmd_transfer_check(di) == 0) {
			memcpy(val, &di->scp_data[1], di->scp_op_num);
			break;
		}
	}
	if (i >= SC8545_SCP_RETRY_TIME) {
		hwlog_err("ack error, retry\n");
		ret = -EINVAL;
	}

	mutex_unlock(&di->accp_adapter_reg_lock);
	return ret;
}

static int sc8545_scp_adapter_multi_reg_read(u8 reg, u8 *val,
	u8 num, void *dev_data)
{
	int ret;
	int i;
	struct sc8545_device_info *di = dev_data;

	mutex_lock(&di->accp_adapter_reg_lock);

	/* clear scp data */
	memset(di->scp_data, 0, sizeof(di->scp_data));

	di->scp_op = SC8545_SCP_READ_OP;
	di->scp_op_num = num;
	di->scp_trans_done = false;
	for (i = 0; i < SC8545_SCP_RETRY_TIME; i++) {
		/* clear tx/rx fifo */
		ret = sc8545_write_mask(di, SC8545_SCP_CTRL_REG,
			SC8545_CLR_TX_FIFO_MASK, SC8545_CLR_TX_FIFO_SHIFT, 1);
		ret += sc8545_write_mask(di, SC8545_SCP_CTRL_REG,
			SC8545_CLR_RX_FIFO_MASK, SC8545_CLR_RX_FIFO_SHIFT, 1);
		/* write cmd, reg, num */
		ret += sc8545_write_byte(di, SC8545_SCP_TX_DATA_REG,
			SC8545_SCP_CMD_MBRRD);
		ret += sc8545_write_byte(di, SC8545_SCP_TX_DATA_REG, reg);
		ret += sc8545_write_byte(di, SC8545_SCP_TX_DATA_REG, num);
		/* start trans */
		ret += sc8545_write_mask(di, SC8545_SCP_CTRL_REG,
			SC8545_SND_START_TRANS_MASK,
			SC8545_SND_START_TRANS_SHIFT, 1);
		if (ret) {
			mutex_unlock(&di->accp_adapter_reg_lock);
			return -EIO;
		}
		/* check cmd transfer success or fail, ignore ack data */
		if (sc8545_scp_cmd_transfer_check(di) == 0) {
			memcpy(val, &di->scp_data[1], SC8545_SCP_MULTI_READ_LEN);
			break;
		}
	}
	if (i >= SC8545_SCP_RETRY_TIME) {
		hwlog_err("ack error, retry\n");
		ret = -EINVAL;
	}

	mutex_unlock(&di->accp_adapter_reg_lock);
	return ret;
}

static int sc8545_scp_reg_read(u8 *val, u8 reg, void *dev_data)
{
	int ret;
	struct sc8545_device_info *di = dev_data;

	if (di->scp_error_flag) {
		hwlog_err("scp timeout happened, do not read reg=0x%x\n", reg);
		return -EINVAL;
	}

	ret = sc8545_scp_adapter_reg_read(val, reg, di);
	if (ret) {
		if (reg != SCP_PROTOCOL_ADP_TYPE0)
			di->scp_error_flag = SC8545_SCP_IS_ERR;

		return -EINVAL;
	}

	return 0;
}

static int sc8545_scp_adapter_reg_write(u8 val, u8 reg, void *dev_data)
{
	int ret = 0;
	int i;
	struct sc8545_device_info *di = dev_data;

	mutex_lock(&di->accp_adapter_reg_lock);

	/* clear scp data */
	memset(di->scp_data, 0, sizeof(di->scp_data));

	di->scp_op = SC8545_SCP_WRITE_OP;
	di->scp_op_num = SC8545_SCP_SBRWR_NUM;
	di->scp_trans_done = false;
	for (i = 0; i < SC8545_SCP_RETRY_TIME; i++) {
		/* clear tx/rx fifo */
		sc8545_write_mask(di, SC8545_SCP_CTRL_REG,
			SC8545_CLR_TX_FIFO_MASK, SC8545_CLR_TX_FIFO_SHIFT, 1);
		sc8545_write_mask(di, SC8545_SCP_CTRL_REG,
			SC8545_CLR_RX_FIFO_MASK, SC8545_CLR_RX_FIFO_SHIFT, 1);
		/* write data */
		ret += sc8545_write_byte(di, SC8545_SCP_TX_DATA_REG,
			SC8545_SCP_CMD_SBRWR);
		ret += sc8545_write_byte(di, SC8545_SCP_TX_DATA_REG, reg);
		ret += sc8545_write_byte(di, SC8545_SCP_TX_DATA_REG, val);
		/* start trans */
		ret += sc8545_write_mask(di, SC8545_SCP_CTRL_REG,
			SC8545_SND_START_TRANS_MASK,
			SC8545_SND_START_TRANS_SHIFT, 1);
		if (ret) {
			mutex_unlock(&di->accp_adapter_reg_lock);
			return -EIO;
		}
		/* check cmd transfer success or fail */
		if (sc8545_scp_cmd_transfer_check(di) == 0)
			break;
	}
	if (i >= SC8545_SCP_RETRY_TIME) {
		hwlog_err("ack error, retry\n");
		ret = -EINVAL;
	}

	mutex_unlock(&di->accp_adapter_reg_lock);
	return ret;
}

static int sc8545_scp_reg_write(u8 val, u8 reg, void *dev_data)
{
	int ret;
	struct sc8545_device_info *di = dev_data;

	if (di->scp_error_flag) {
		hwlog_err("scp timeout happened, do not write reg=0x%x\n", reg);
		return -EINVAL;
	}

	ret = sc8545_scp_adapter_reg_write(val, reg, di);
	if (ret) {
		di->scp_error_flag = SC8545_SCP_IS_ERR;
		return -EINVAL;
	}

	return 0;
}

static int sc8545_fcp_adapter_vol_check(int adapter_vol, void *dev_data)
{
	int i;
	int ret;
	int adc_vol = 0;
	struct sc8545_device_info *di = dev_data;

	if ((adapter_vol < SC8545_FCP_ADAPTER_MIN_VOL) ||
		(adapter_vol > SC8545_FCP_ADAPTER_MAX_VOL)) {
		hwlog_err("check vol out of range, input vol = %dmV\n", adapter_vol);
		return -EINVAL;
	}

	for (i = 0; i < SC8545_FCP_ADAPTER_VOL_CHECK_TIME; i++) {
		ret = sc8545_get_vbus_mv((unsigned int *)&adc_vol, di);
		if (ret)
			continue;
		if ((adc_vol > (adapter_vol - SC8545_FCP_ADAPTER_ERR_VOL)) &&
			(adc_vol < (adapter_vol + SC8545_FCP_ADAPTER_ERR_VOL)))
			break;
		(void)power_msleep(DT_MSLEEP_100MS, 0, NULL);
	}
	if (i == SC8545_FCP_ADAPTER_VOL_CHECK_TIME) {
		hwlog_err("check vol timeout, input vol = %dmV\n", adapter_vol);
		return -EINVAL;
	}

	hwlog_info("check vol success, input vol = %dmV, spend %dms\n",
		adapter_vol, i * DT_MSLEEP_100MS);
	return 0;
}

static int sc8545_fcp_master_reset(void *dev_data)
{
	int ret;
	struct sc8545_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = sc8545_write_mask(di, SC8545_SCP_CTRL_REG,
		SC8545_SCP_SOFT_RST_MASK, SC8545_SCP_SOFT_RST_SHIFT, 1);
	power_usleep(DT_USLEEP_10MS);

	return ret;
}

static int sc8545_fcp_adapter_reset(void *dev_data)
{
	int ret;
	struct sc8545_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = sc8545_write_mask(di, SC8545_SCP_CTRL_REG,
		SC8545_SCP_SOFT_RST_MASK, SC8545_SCP_SOFT_RST_MASK, 1);
	power_usleep(DT_USLEEP_20MS);
	ret += sc8545_fcp_adapter_vol_check(SC8545_FCP_ADAPTER_RST_VOL, di);
	if (ret)
		return -EINVAL;

	ret = sc8545_config_vac_ovp_th_mv(di, SC8545_DFT_VAC_OVP_TH_INIT);
	ret += sc8545_config_vbus_ovp_th_mv(di, SC8545_DFT_VBUS_OVP_TH_INIT);

	return ret;
}

static int sc8545_fcp_read_switch_status(void *dev_data)
{
	return 0;
}

static int sc8545_is_fcp_charger_type(void *dev_data)
{
	struct sc8545_device_info *di = dev_data;

	if (!di)
		return 0;

	if (di->dts_fcp_support == 0) {
		hwlog_err("not support fcp\n");
		return 0;
	}

	if (di->fcp_support)
		return 1;

	return 0;
}

static void sc8545_fcp_adapter_detect_reset(struct sc8545_device_info *di)
{
	(void)sc8545_write_mask(di, SC8545_DPDM_CTRL1_REG,
		SC8545_DPDM_EN_MASK, SC8545_DPDM_EN_SHIFT, false);
	(void)sc8545_write_mask(di, SC8545_DPDM_CTRL2_REG,
		SC8545_DP_BUFF_EN_MASK, SC8545_DP_BUFF_EN_SHIFT, false);
	(void)sc8545_write_mask(di, SC8545_SCP_CTRL_REG, SC8545_SCP_EN_MASK,
		SC8545_SCP_EN_SHIFT, false);
	(void)sc8545_fcp_adapter_reset(di);
}

static int sc8545_fcp_adapter_detect_enable(struct sc8545_device_info *di)
{
	int ret;

	ret = sc8545_write_mask(di, SC8545_SCP_CTRL_REG, SC8545_SCP_EN_MASK,
		SC8545_SCP_EN_SHIFT, true);
	ret += sc8545_write_mask(di, SC8545_DPDM_CTRL1_REG, SC8545_DPDM_EN_MASK,
		SC8545_DPDM_EN_SHIFT, true);
	ret += sc8545_write_mask(di, SC8545_DPDM_CTRL2_REG,
		SC8545_DP_BUFF_EN_MASK, SC8545_DP_BUFF_EN_SHIFT, true);
	ret += sc8545_write_byte(di, SC8545_SCP_FLAG_MASK_REG,
		SC8545_SCP_FLAG_MASK_REG_INIT);

	return ret;
}

static int sc8545_fcp_adapter_detct_dpdm_stat(struct sc8545_device_info *di)
{
	int cnt;
	int ret;
	u8 stat = 0;

	for (cnt = 0; cnt < SC8545_SCP_DETECT_MAX_CNT; cnt++) {
		ret = sc8545_read_byte(di, SC8545_DPDM_STAT_REG, &stat);
		hwlog_info("scp_dpdm_stat=0x%x\n", stat);
		if (ret) {
			hwlog_err("read dpdm_stat_reg fail\n");
			continue;
		}

		(void)power_msleep(DT_MSLEEP_100MS, 0, NULL);
		/* 0: DM voltage < 0.325v */
		if (((stat & SC8545_VDM_RD_MASK) >> SC8545_VDM_RD_SHIFT) == 0)
			break;
	}
	if (cnt == SC8545_SCP_DETECT_MAX_CNT) {
		hwlog_err("CHG_SCP_ADAPTER_DETECT_OTHER\n");
		return -EINVAL;
	}

	return 0;
}

static int sc8545_fcp_adapter_detect_ping_stat(struct sc8545_device_info *di)
{
	int cnt;
	int ret;
	u8 scp_stat = 0;

	for (cnt = 0; cnt < SC8545_SCP_PING_DETECT_MAX_CNT; cnt++) {
		/* wait 82ms for every 5-ping */
		if ((cnt % 5) == 0)
			power_msleep(82, 0, NULL);

		ret = sc8545_write_mask(di, SC8545_SCP_CTRL_REG,
			SC8545_SND_START_TRANS_MASK,
			SC8545_SND_START_TRANS_SHIFT, true);
		if (ret)
			return -EIO;

		/* wait 10ms for every ping */
		power_usleep(DT_USLEEP_10MS);

		sc8545_read_byte(di, SC8545_SCP_STAT_REG, &scp_stat);
		hwlog_info("scp ping detect,scp_stat:0x%x\n", scp_stat);
		if ((scp_stat & SC8545_NO_FIRST_SLAVE_PING_STAT_MASK) == 0) {
			hwlog_info("scp adapter detect ok\n");
			di->fcp_support = true;
			break;
		}
	}
	if (cnt == SC8545_SCP_PING_DETECT_MAX_CNT) {
		hwlog_err("CHG_SCP_ADAPTER_DETECT_OTHER\n");
		return -EINVAL;
	}

	return 0;
}

static int sc8545_fcp_adapter_detect(struct sc8545_device_info *di)
{
	int ret;

	mutex_lock(&di->scp_detect_lock);
	di->init_finish_flag = true;

	if (di->fcp_support) {
		mutex_unlock(&di->scp_detect_lock);
		return ADAPTER_DETECT_SUCC;
	}

	/* transmit vdp_src_BC1.2 signal */
	ret = sc8545_fcp_adapter_detect_enable(di);
	if (ret) {
		sc8545_fcp_adapter_detect_reset(di);
		mutex_unlock(&di->scp_detect_lock);
		return ADAPTER_DETECT_OTHER;
	}

	/* waiting for hvdcp */
	(void)power_msleep(DT_MSLEEP_1S, 0, NULL);

	/* detect dpdm stat */
	ret = sc8545_fcp_adapter_detct_dpdm_stat(di);
	if (ret) {
		sc8545_fcp_adapter_detect_reset(di);
		mutex_unlock(&di->scp_detect_lock);
		return ADAPTER_DETECT_OTHER;
	}

	/* detect ping stat */
	ret = sc8545_fcp_adapter_detect_ping_stat(di);
	if (ret) {
		sc8545_fcp_adapter_detect_reset(di);
		mutex_unlock(&di->scp_detect_lock);
		return ADAPTER_DETECT_OTHER;
	}

	mutex_unlock(&di->scp_detect_lock);
	return ADAPTER_DETECT_SUCC;
}

static int sc8545_fcp_stop_charge_config(void *dev_data)
{
	int ret;
	struct sc8545_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = sc8545_fcp_master_reset(di);
	ret += sc8545_write_mask(di, SC8545_SCP_CTRL_REG, SC8545_SCP_EN_MASK,
		SC8545_SCP_EN_SHIFT, 0);
	ret += sc8545_write_mask(di, SC8545_DPDM_CTRL1_REG, SC8545_DPDM_EN_MASK,
		SC8545_DPDM_EN_SHIFT, 0);
	if (ret)
		return -EINVAL;

	di->fcp_support = false;

	return ret;
}

static int sc8545_self_check(void *dev_data)
{
	return 0;
}

static int sc8545_scp_chip_reset(void *dev_data)
{
	return sc8545_fcp_master_reset(dev_data);
}

static int sc8545_scp_reg_read_block(int reg, int *val, int num,
	void *dev_data)
{
	int ret;
	int i;
	u8 data = 0;
	struct sc8545_device_info *di = dev_data;

	if (!di || !val)
		return -ENODEV;

	di->scp_error_flag = SC8545_SCP_NO_ERR;

	for (i = 0; i < num; i++) {
		ret = sc8545_scp_reg_read(&data, reg + i, di);
		if (ret) {
			hwlog_err("scp read failed, reg=0x%x\n", reg + i);
			return -EINVAL;
		}
		val[i] = data;
	}

	return 0;
}

static int sc8545_scp_multi_reg_read_block(u8 reg, u8 *val, u8 num,
	void *dev_data)
{
	int ret;
	int i;
	u8 data[SC8545_SCP_MULTI_READ_LEN] = { 0 };
	struct sc8545_device_info *di = dev_data;

	if (!di || !val)
		return -ENODEV;

	di->scp_error_flag = SC8545_SCP_NO_ERR;

	for (i = 0; i < num; i += SC8545_SCP_MULTI_READ_LEN) {
		ret = sc8545_scp_adapter_multi_reg_read(reg + i, data,
			SC8545_SCP_MULTI_READ_LEN, di);
		if (ret) {
			hwlog_err("scp read failed, reg=0x%x\n", reg + i);
			return -EINVAL;
		}
		val[i] = data[0];
		val[i + 1] = data[1];
	}

	return 0;
}

static int sc8545_scp_reg_write_block(int reg, const int *val, int num,
	void *dev_data)
{
	int ret;
	int i;
	struct sc8545_device_info *di = dev_data;

	if (!di || !val)
		return -ENODEV;

	di->scp_error_flag = SC8545_SCP_NO_ERR;

	for (i = 0; i < num; i++) {
		ret = sc8545_scp_reg_write(val[i], reg + i, di);
		if (ret) {
			hwlog_err("scp write failed, reg=0x%x\n", reg + i);
			return -EINVAL;
		}
	}

	return 0;
}

static int sc8545_scp_detect_adapter(void *dev_data)
{
	struct sc8545_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	return sc8545_fcp_adapter_detect(di);
}

static int sc8545_fcp_reg_read_block(int reg, int *val, int num, void *dev_data)
{
	int ret, i;
	u8 data = 0;
	struct sc8545_device_info *di = dev_data;

	if (!di || !val)
		return -ENODEV;

	di->scp_error_flag = SC8545_SCP_NO_ERR;

	for (i = 0; i < num; i++) {
		ret = sc8545_scp_reg_read(&data, reg + i, di);
		if (ret) {
			hwlog_err("fcp read failed, reg=0x%x\n", reg + i);
			return -EINVAL;
		}
		val[i] = data;
	}
	return 0;
}

static int sc8545_fcp_reg_write_block(int reg, const int *val, int num,
	void *dev_data)
{
	int ret, i;
	struct sc8545_device_info *di = dev_data;

	if (!di || !val)
		return -ENODEV;

	di->scp_error_flag = SC8545_SCP_NO_ERR;

	for (i = 0; i < num; i++) {
		ret = sc8545_scp_reg_write(val[i], reg + i, di);
		if (ret) {
			hwlog_err("fcp write failed, reg=0x%x\n", reg + i);
			return -EINVAL;
		}
	}

	return 0;
}

static int sc8545_fcp_detect_adapter(void *dev_data)
{
	struct sc8545_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	return sc8545_fcp_adapter_detect(di);
}

static int sc8545_pre_init(void *dev_data)
{
	struct sc8545_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	return sc8545_self_check(di);
}

static int sc8545_scp_adapter_reset(void *dev_data)
{
	return sc8545_fcp_adapter_reset(dev_data);
}

static struct scp_protocol_ops g_sc8545_scp_protocol_ops = {
	.chip_name = "sc8545",
	.reg_read = sc8545_scp_reg_read_block,
	.reg_write = sc8545_scp_reg_write_block,
	.reg_multi_read = sc8545_scp_multi_reg_read_block,
	.detect_adapter = sc8545_scp_detect_adapter,
	.soft_reset_master = sc8545_scp_chip_reset,
	.soft_reset_slave = sc8545_scp_adapter_reset,
	.pre_init = sc8545_pre_init,
};

static struct fcp_protocol_ops g_sc8545_fcp_protocol_ops = {
	.chip_name = "sc8545",
	.reg_read = sc8545_fcp_reg_read_block,
	.reg_write = sc8545_fcp_reg_write_block,
	.detect_adapter = sc8545_fcp_detect_adapter,
	.soft_reset_master = sc8545_fcp_master_reset,
	.soft_reset_slave = sc8545_fcp_adapter_reset,
	.get_master_status = sc8545_fcp_read_switch_status,
	.stop_charging_config = sc8545_fcp_stop_charge_config,
	.is_accp_charger_type = sc8545_is_fcp_charger_type,
};

int sc8545_protocol_ops_register(struct sc8545_device_info *di)
{
	int ret;

	if (di->dts_scp_support) {
		g_sc8545_scp_protocol_ops.dev_data = (void *)di;
		ret = scp_protocol_ops_register(&g_sc8545_scp_protocol_ops);
		if (ret)
			return -EINVAL;
	}
	if (di->dts_fcp_support) {
		g_sc8545_fcp_protocol_ops.dev_data = (void *)di;
		ret = fcp_protocol_ops_register(&g_sc8545_fcp_protocol_ops);
		if (ret)
			return -EINVAL;
	}

	return 0;
}
