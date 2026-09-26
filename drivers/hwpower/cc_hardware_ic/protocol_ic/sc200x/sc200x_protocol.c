/*
 * sc200x_protocol.c
 *
 * sc200x protocol driver
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

#include "sc200x_protocol.h"
#include "sc200x_i2c.h"
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/protocol/adapter_protocol.h>
#include <chipset_common/hwpower/protocol/adapter_protocol_scp.h>
#include <chipset_common/hwpower/protocol/adapter_protocol_fcp.h>

#define HWLOG_TAG sc200x
HWLOG_REGIST();

static void sc200x_accp_detect_lock(struct sc200x_device_info *di)
{
	mutex_lock(&di->accp_detect_lock);
	hwlog_info("accp_detect_lock locked\n");
}

static void sc200x_accp_detect_unlock(struct sc200x_device_info *di)
{
	mutex_unlock(&di->accp_detect_lock);
	hwlog_info("accp_detect_lock unlock\n");
}

static void sc200x_accp_adapter_reg_lock(struct sc200x_device_info *di)
{
	mutex_lock(&di->accp_adpt_reg_lock);
	hwlog_info("accp_adpt_reg_lock locked\n");
}

static void sc200x_accp_adapter_reg_unlock(struct sc200x_device_info *di)
{
	mutex_unlock(&di->accp_adpt_reg_lock);
	hwlog_info("accp_adpt_reg_lock unlock\n");
}

static int sc200x_fill_accp_cmd(struct sc200x_device_info *di,
	int cmd, int reg, int val)
{
	int len;
	struct sc200x_accp_msg wr_msg = { 0 };

	len = 0x03; /* fill 3 byte */
	wr_msg.cmd = (unsigned char)cmd;
	wr_msg.reg_addr = (unsigned char)(reg & 0xFF);
	wr_msg.data_len = 0x01;
	if (cmd == SC200X_ACCP_CMD_SBRWR) {
		len += 1;
		wr_msg.data = (unsigned char)(val & 0xFF);
	}

	return sc200x_i2c_write_reg(di, SC200X_REG_ACCP_CMD,
		(const unsigned char *)&wr_msg, len);
}

static inline int sc200x_start_accp_cmd_xfer(struct sc200x_device_info *di)
{
	return sc200x_write_reg_mask(di, SC200X_REG_DEV_CTRL,
		SC200X_ACCP_CMD_MASK, SC200X_ACCP_CMD_MASK);
}

static int sc200x_get_accp_cmd_response_status(struct sc200x_device_info *di)
{
	int status;
	int reg_val;

	reg_val = sc200x_read_reg(di, SC200X_REG_DEV_STATUS);
	if (reg_val < 0)
		return SC200X_FAILED;

	status = (reg_val & SC200X_ACCP_RSP_STA_MASK) >>
		SC200X_ACCP_RSP_STA_SHIFT;

	switch (status) {
	case SC200X_ACCP_ACK:
		return SC200X_SUCCESS;
	case SC200X_ACCP_NACK:
		return SC200X_STA_NACK;
	case SC200X_ACCP_NORSP:
		return SC200X_STA_NORSP;
	default:
		return SC200X_FAILED;
	}
}

static int sc200x_get_accp_cmd_response_value(struct sc200x_device_info *di,
	int *val)
{
	int ret;
	struct sc200x_reg_msg rd_msg = { 0 };

	ret = sc200x_i2c_read_reg(di, SC200X_REG_DEV_STATUS,
		(unsigned char *)&rd_msg, 4); /* read 4 byte */
	if (ret < 0) {
		ret = SC200X_FAILED;
		hwlog_err("get adpt value fail, ret=%d\n", ret);
	} else {
		ret = SC200X_SUCCESS;
		*val = rd_msg.data;
	}

	return ret;
}

static int sc200x_send_accp_cmd(struct sc200x_device_info *di,
	int cmd, int reg, int val)
{
	int ret;
	int timeout;

	reinit_completion(&di->accp_rsp_completion);
	/* fill accp command and start command send */
	ret = sc200x_fill_accp_cmd(di, cmd, reg, val);
	if (ret < 0)
		goto send_accp_cmd_fail;

	ret = sc200x_start_accp_cmd_xfer(di);
	if (ret < 0)
		goto send_accp_cmd_fail;

	/* wait accp adapter response */
	timeout = wait_for_completion_timeout(&di->accp_rsp_completion,
		msecs_to_jiffies(SC200X_ACCP_RSP_TIMEOUT));
	if (!timeout) {
		hwlog_err("get accp response timeout\n");
		return SC200X_FAILED;
	}

	return sc200x_get_accp_cmd_response_status(di);

send_accp_cmd_fail:
	hwlog_err("send_accp_cmd error, ret=%d\n", ret);
	return SC200X_FAILED;
}

static inline int sc200x_accp_write_byte(struct sc200x_device_info *di,
	int reg, int val)
{
	return sc200x_send_accp_cmd(di, SC200X_ACCP_CMD_SBRWR, reg, val);
}

static int sc200x_accp_read_byte(struct sc200x_device_info *di,
	int reg, int *val)
{
	int ret;

	ret = sc200x_send_accp_cmd(di, SC200X_ACCP_CMD_SBRRD, reg, 0x00);
	if (ret == SC200X_SUCCESS)
		ret = sc200x_get_accp_cmd_response_value(di, val);

	return ret;
}

static int sc200x_accp_adapter_reg_write(struct sc200x_device_info *di,
	int val, int reg)
{
	int i;
	int ret = SC200X_FAILED;

	sc200x_accp_adapter_reg_lock(di);
	for (i = 0; i < SC200X_ACCP_RETRY_MAX_TIMES; i++) {
		ret = sc200x_accp_write_byte(di, reg, val);
		if (ret == SC200X_SUCCESS) {
			hwlog_info("write adpt success, reg=0x%x val=0x%x\n",
				reg, val);
			break;
		} else if (ret == SC200X_STA_NACK) {
			hwlog_info("received adpt nack, reg=0x%x val=0x%x\n",
				reg, val);
			ret = SC200X_FAILED;
			break;
		}
		hwlog_err("write adpt fail, reg=0x%x val=0x%x retry=%d\n",
			reg, val, i + 1);
	}

	if (i == SC200X_ACCP_RETRY_MAX_TIMES)
		ret = SC200X_FAILED;
	sc200x_accp_adapter_reg_unlock(di);

	return ret;
}

static int sc200x_accp_adapter_reg_read(struct sc200x_device_info *di,
	int *val, int reg)
{
	int i;
	int ret;

	if (!di)
		return SC200X_FAILED;

	sc200x_accp_adapter_reg_lock(di);
	for (i = 0; i < SC200X_ACCP_RETRY_MAX_TIMES; i++) {
		ret = sc200x_accp_read_byte(di, reg, val);
		if (ret == SC200X_SUCCESS) {
			hwlog_info("read adpt success, reg=0x%x val=0x%x\n",
				reg, *val);
			break;
		} else if (ret == SC200X_STA_NACK) {
			hwlog_info("received adpt nack, reg=0x%x\n", reg);
			continue;
		}
		hwlog_err("read adpt fail, reg=0x%x retry=%d\n", reg, i + 1);
	}

	if (i == SC200X_ACCP_RETRY_MAX_TIMES)
		ret = SC200X_FAILED;
	sc200x_accp_adapter_reg_unlock(di);

	return ret;
}

static int sc200x_read_accp_status(void *dev_data)
{
	int status;
	int reg_val;
	int ret = SC200X_FAILED;
	struct sc200x_device_info *di = dev_data;

	if (!di)
		return SC200X_FAILED;

	reg_val = sc200x_read_reg(di, SC200X_REG_DEV_CTRL);
	if ((reg_val < 0) || !(reg_val & SC200X_ACCP_EN_MASK))
		goto read_accp_status_fail;

	reg_val = sc200x_read_reg(di, SC200X_REG_DEV_STATUS);
	if (reg_val < 0)
		goto read_accp_status_fail;
	status = (reg_val & SC200X_DVC_MASK) >> SC200X_DVC_SHIFT;
	if (status == SC200X_DVC_ACCP_DETECTED)
		ret = SC200X_SUCCESS;

read_accp_status_fail:
	hwlog_info("accp adapter status %s\n",
		ret == SC200X_SUCCESS ? "ok" : "fail");

	return ret;
}

static int sc200x_restart_accp_detect(struct sc200x_device_info *di)
{
	int ret;

	ret = sc200x_write_reg_mask(di, SC200X_REG_DEV_CTRL,
		SC200X_ACCP_EN_MASK, SC200X_ACCP_EN_MASK);
	if (ret < 0) {
		hwlog_err("set accp detect enable fail\n");
		return SC200X_FAILED;
	}

	return SC200X_SUCCESS;
}

static int sc200x_accp_reg_read_block(int reg, int *val, int num,
	void *dev_data)
{
	int i;
	int data = 0;
	struct sc200x_device_info *di = dev_data;

	if (!di)
		return SC200X_FAILED;

	if (!val) {
		hwlog_err("val is null\n");
		return SC200X_FAILED;
	}

	for (i = 0; i < num; i++) {
		if (sc200x_accp_adapter_reg_read(di, &data, reg + i) < 0)
			return SC200X_FAILED;
		val[i] = data;
	}

	return SC200X_SUCCESS;
}

static int sc200x_accp_reg_write_block(int reg, const int *val, int num,
	void *dev_data)
{
	int i;
	struct sc200x_device_info *di = dev_data;

	if (!di)
		return SC200X_FAILED;

	if (!val) {
		hwlog_err("val is null\n");
		return SC200X_FAILED;
	}

	for (i = 0; i < num; i++) {
		if (sc200x_accp_adapter_reg_write(di, val[i], reg + i) < 0)
			return SC200X_FAILED;
	}

	return SC200X_SUCCESS;
}

static int sc200x_accp_adapter_detect(void *dev_data)
{
	int ret;
	int timeout;
	struct sc200x_device_info *di = dev_data;

	if (!di)
		return SC200X_FAILED;

	sc200x_accp_detect_lock(di);
	ret = sc200x_read_accp_status(dev_data);
	if (ret == SC200X_SUCCESS)
		goto accp_adapter_detect_out;

	/*
	 * if the detection has not started or the detection failed,
	 * start again
	 */
	reinit_completion(&di->accp_detect_completion);
	ret = sc200x_restart_accp_detect(di);
	if (ret == SC200X_FAILED)
		goto accp_adapter_detect_out;

	/* wait accp adapter response */
	timeout = wait_for_completion_timeout(&di->accp_detect_completion,
		msecs_to_jiffies(SC200X_ACCP_DETECT_TIMEOUT));
	if (!timeout) {
		hwlog_err("wait accp detect timeout\n");
		ret = SC200X_FAILED;
	}

accp_adapter_detect_out:
	sc200x_accp_detect_unlock(di);
	hwlog_err("accp adapter detect %s\n",
		ret == SC200X_FAILED ? "fail" : "ok");

	return ret;
}

static int sc200x_accp_master_reset(void *dev_data)
{
	int ret;
	int val;
	struct sc200x_device_info *di = dev_data;

	if (!di)
		return SC200X_FAILED;

	val = SC200X_RST_CTRL_ACCP_MSTR << SC200X_RST_CTRL_SHIFT;
	ret = sc200x_write_reg_mask(di, SC200X_REG_DEV_CTRL,
		val, SC200X_RST_CTRL_MASK);
	power_msleep(SC200X_ACCP_RST_MSTR_DELAY, 0, NULL);

	hwlog_info("accp master reset %s\n", ret < 0 ? "fail" : "success");

	return ret;
}

static int sc200x_accp_adapter_reset(void *dev_data)
{
	int ret;
	int val;
	struct sc200x_device_info *di = dev_data;

	if (!di)
		return SC200X_FAILED;

	val = SC200X_RST_CTRL_ACCP_SLAVE << SC200X_RST_CTRL_SHIFT;
	ret = sc200x_write_reg_mask(di, SC200X_REG_DEV_CTRL,
		val, SC200X_RST_CTRL_MASK);
	power_msleep(SC200X_ACCP_RST_ADPT_DELAY, 0, NULL);

	hwlog_info("adapter reset %s\n", ret < 0 ? "fail" : "success");

	return ret;
}

static int sc200x_fcp_stop_charge_config(void *dev_data)
{
	struct sc200x_device_info *di = dev_data;

	if (!di)
		return SC200X_FAILED;

	/* clear accp enable bit */
	sc200x_write_reg_mask(di, SC200X_REG_DEV_CTRL,
		0x00, SC200X_ACCP_EN_MASK);

	return SC200X_SUCCESS;
}

static int sc200x_is_accp_charger_type(void *dev_data)
{
	bool support = false;
	struct sc200x_device_info *di = dev_data;

	if (!di)
		return false;

	if (!di->fcp_support)
		goto end;

	if (sc200x_read_accp_status(dev_data) == SC200X_SUCCESS)
		support = true;

end:
	hwlog_info("adapter %s accp\n", support ? "support" : "not support");
	return support;
}

static struct fcp_protocol_ops g_sc200x_fcp_protocol_ops = {
	.chip_name = "sc200x",
	.reg_read = sc200x_accp_reg_read_block,
	.reg_write = sc200x_accp_reg_write_block,
	.detect_adapter = sc200x_accp_adapter_detect,
	.soft_reset_master = sc200x_accp_master_reset,
	.soft_reset_slave = sc200x_accp_adapter_reset,
	.get_master_status = sc200x_read_accp_status,
	.stop_charging_config = sc200x_fcp_stop_charge_config,
	.is_accp_charger_type = sc200x_is_accp_charger_type,
};

static struct scp_protocol_ops g_sc200x_scp_protocol_ops = {
	.chip_name = "sc200x",
	.reg_read = sc200x_accp_reg_read_block,
	.reg_write = sc200x_accp_reg_write_block,
	.detect_adapter = sc200x_accp_adapter_detect,
	.soft_reset_master = sc200x_accp_master_reset,
	.soft_reset_slave = sc200x_accp_adapter_reset,
};

int sc200x_protocol_init(struct sc200x_device_info *di)
{
	if (!di) {
		hwlog_err("chip not init\n");
		return -ENODEV;
	}

	/* disable accp mask on initialization */
	sc200x_write_reg_mask(di, SC200X_REG_DEV_CTRL, 0, SC200X_ACCP_EN_MASK);

	if (di->scp_support) {
		g_sc200x_scp_protocol_ops.dev_data = (void *)di;
		if (scp_protocol_ops_register(&g_sc200x_scp_protocol_ops))
			return -EINVAL;
	}

	if (di->fcp_support) {
		g_sc200x_fcp_protocol_ops.dev_data = (void *)di;
		if (fcp_protocol_ops_register(&g_sc200x_fcp_protocol_ops))
			return -EINVAL;
	}
}

int sc200x_accp_alert_handler(struct sc200x_device_info *di,
	unsigned int alert)
{
	if (alert & SC200X_ALERT_ACCP_MSG_MASK)
		complete(&di->accp_rsp_completion);
	else
		complete(&di->accp_detect_completion);

	return SC200X_SUCCESS;
}
