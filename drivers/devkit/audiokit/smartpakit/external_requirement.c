/*
 * external_requirement.c
 *
 * external_requirement driver
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

#include <linux/i2c.h>
#include <linux/regmap.h>
#include <huawei_platform/log/hw_log.h>
#include "smartpakit.h"
#include "securec.h"

#define HWLOG_TAG smartpakit
HWLOG_REGIST();

#define EXTERN_NODE_NUM    1

static struct smartpakit_i2c_priv *get_i2c_priv_by_addr(
	struct smartpakit_priv *kit_priv, int i2c_addr)
{
	struct smartpakit_i2c_priv *i2c_priv = NULL;
	unsigned int i;

	if (i2c_addr >= SMARTPAKIT_I2C_ADDR_ARRAY_MAX)
		return NULL;

	for (i = 0; i < SMARTPAKIT_PA_ID_MAX; i++) {
		i2c_priv = kit_priv->i2c_priv[i];
		if ((i2c_priv == NULL) || (i2c_priv->i2c == NULL))
			continue;
		if (i2c_priv->i2c->addr == i2c_addr)
			return i2c_priv;
	}

	return NULL;
}

static int smartpakit_hisi_regulator_reg_read(int i2c_addr,
	int reg_addr, int *value)
{
	struct smartpakit_i2c_priv *i2c_priv = NULL;
	struct smartpakit_priv *kit_priv = NULL;
	struct smartpakit_get_param reg_info;
	int ret;

	kit_priv = smartpakit_get_misc_priv();
	if ((kit_priv == NULL) || (kit_priv->i2c_num <= 0)) {
		hwlog_err("%s: Invalid smartpakit_priv param", __func__);
		return -EINVAL;
	}

	mutex_lock(&kit_priv->i2c_ops_lock);
	if (memset_s(&reg_info, sizeof(reg_info), 0, sizeof(reg_info)) != EOK)
		hwlog_err("%s: memset_s is failed\n", __func__);
	i2c_priv = get_i2c_priv_by_addr(kit_priv, i2c_addr);
	if ((i2c_priv == NULL) || (value == NULL)) {
		hwlog_err("%s: invalid input arg, i2c_addr0x%x\n", __func__,
			i2c_addr);
		ret = -EINVAL;
		goto err_out;
	}

	if ((kit_priv->ioctl_ops == NULL) ||
		(kit_priv->ioctl_ops->read_regs == NULL)) {
		    hwlog_err("%s: ioctl_ops or i2c_reg_ctl is NULL\n", __func__);
		    ret = -ECHILD;
		    goto err_out;
	}

	reg_info.index = reg_addr;
	ret = kit_priv->ioctl_ops->read_regs(i2c_priv, &reg_info);
	if (ret == 0)
		*value = reg_info.value;

err_out:
	mutex_unlock(&kit_priv->i2c_ops_lock);
	return ret;
}

static int smartpakit_hisi_regulator_reg_write(int i2c_addr, int reg_addr,
	int mask, int value)
{
	struct smartpakit_i2c_priv *i2c_priv = NULL;
	struct smartpakit_priv *kit_priv = NULL;
	struct smartpakit_param_node reg_info;
	int ret;

	kit_priv = smartpakit_get_misc_priv();
	if ((kit_priv == NULL) || (kit_priv->i2c_num <= 0)) {
		hwlog_err("%s: Invalid smartpakit_priv param", __func__);
		return -EINVAL;
	}

	mutex_lock(&kit_priv->i2c_ops_lock);
	if (memset_s(&reg_info, sizeof(reg_info), 0, sizeof(reg_info)) != EOK)
		hwlog_err("%s: memset_s is failed\n", __func__);
	i2c_priv = get_i2c_priv_by_addr(kit_priv, i2c_addr);
	if (i2c_priv == NULL) {
		hwlog_err("%s: invalid i2c addr 0x%x\n", __func__,
			i2c_addr);
		ret = -EINVAL;
		goto err_out;
	}

	if ((kit_priv->ioctl_ops == NULL) ||
		(kit_priv->ioctl_ops->write_regs == NULL)) {
		hwlog_err("%s: ioctl_ops or i2c_reg_ctl is NULL\n", __func__);
		ret = -ECHILD;
		goto err_out;
	}

	reg_info.index = reg_addr;
	reg_info.mask = mask;
	reg_info.node_type = SMARTPAKIT_PARAM_NODE_TYPE_REG_WRITE;
	reg_info.value = value;
	ret = kit_priv->ioctl_ops->write_regs(i2c_priv,
			EXTERN_NODE_NUM, &reg_info);

err_out:
	mutex_unlock(&kit_priv->i2c_ops_lock);
	return ret;
}

struct smartpa_regulator_reg_ops hisi_reg_ops = {
	.spmi_regulator_reg_read = smartpakit_hisi_regulator_reg_read,
	.spmi_regulator_reg_write = smartpakit_hisi_regulator_reg_write,
};
