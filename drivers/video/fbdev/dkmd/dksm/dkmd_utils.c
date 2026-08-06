/**
 * @file
 * Copyright (c) 2021-2021, Huawei Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/regulator/consumer.h>
#include "dkmd_utils.h"

uint32_t g_dkmd_log_level = DPU_LOG_LVL_MAX;
EXPORT_SYMBOL(g_dkmd_log_level);

void set_reg(char __iomem *addr, uint32_t val, uint8_t bw, uint8_t bs)
{
	uint32_t mask;
	uint32_t temp;

	if (bw == 0)
		return;

	mask = GENMASK(bs + bw - 1, bs);
	temp = readl(addr);
	temp &= ~mask;

	dpu_pr_debug("addr:%#x value:%#x ", addr, temp | ((val << bs) & mask));

	writel(temp | ((val << bs) & mask), addr);
}
EXPORT_SYMBOL(set_reg);

uint32_t set_bits32(uint32_t old_val, uint32_t val, uint8_t bw, uint8_t bs)
{
	uint32_t mask = (1UL << bw) - 1UL;
	uint32_t tmp;

	tmp = old_val;
	tmp &= ~(mask << bs);

	return (tmp | ((val & mask) << bs));
}
EXPORT_SYMBOL(set_bits32);

struct peri_handle_data {
	int dtype;
	int (*handle_func)(void *desc);
};

static int gpio_handle_input(void *desc)
{
	struct gpio_desc *cmd = (struct gpio_desc *)desc;

	return gpio_direction_input(*(cmd->gpio));
}

static int gpio_handle_output(void *desc)
{
	struct gpio_desc *cmd = (struct gpio_desc *)desc;

	return gpio_direction_output(*(cmd->gpio), cmd->value);
}

static int gpio_handle_request(void *desc)
{
	struct gpio_desc *cmd = (struct gpio_desc *)desc;

	return gpio_request(*(cmd->gpio), cmd->label);
}

static int gpio_handle_free(void *desc)
{
	struct gpio_desc *cmd = (struct gpio_desc *)desc;

	gpio_free(*(cmd->gpio));

	return 0;
}

static struct peri_handle_data g_gpio_handle_data[] = {
	{ DTYPE_GPIO_REQUEST, gpio_handle_request },
	{ DTYPE_GPIO_INPUT, gpio_handle_input },
	{ DTYPE_GPIO_OUTPUT, gpio_handle_output },
	{ DTYPE_GPIO_FREE, gpio_handle_free },
};

static int gpio_cmds_tx_check_param(struct gpio_desc *cmd, int index)
{
	if (!cmd || !cmd->label) {
		dpu_pr_err("cmd or cmd->label is NULL! index=%d\n", index);
		return -1;
	}

	if (!gpio_is_valid(*(cmd->gpio))) {
		dpu_pr_err("gpio invalid, dtype=%d, lable=%s, gpio=%d!\n",
			cmd->dtype, cmd->label, *(cmd->gpio));
		return -1;
	}

	return 0;
}

static void cmds_tx_delay(int waittype, int wait)
{
	if (wait) {
		if (waittype == WAIT_TYPE_US)
			udelay(wait);
		else if (waittype == WAIT_TYPE_MS)
			mdelay(wait);
		else
			mdelay(wait * 1000);
	}
}

int gpio_cmds_tx(struct gpio_desc *cmds, int cnt)
{
	int ret = 0;
	int i;
	int j;

	struct gpio_desc *cm = cmds;

	for (i = 0; i < cnt; i++) {
		ret = gpio_cmds_tx_check_param(cm, i);
		if (ret)
			return ret;

		ret = -1;
		for (j = 0; j < ARRAY_SIZE(g_gpio_handle_data); j++) {
			if (cm->dtype == g_gpio_handle_data[j].dtype) {
				dpu_pr_info("dtype=%d label: %s, gpio=%d\n", cm->dtype, cm->label, *(cm->gpio));
				ret = g_gpio_handle_data[j].handle_func((void *)cm);
				break;
			}
		}

		if (ret != 0) {
			dpu_pr_err("dtype=%x handle failed, ret=%d, label: %s, gpio=%d\n",
				cm->dtype, ret, cm->label, *(cm->gpio));
			return ret;
		}
		cmds_tx_delay(cm->waittype, cm->wait);
		cm++;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(gpio_cmds_tx);

static int vcc_handle_put(void *desc)
{
	struct vcc_desc *cmd = (struct vcc_desc *)desc;

	devm_regulator_put(*(cmd->regulator));

	return 0;
}

static int vcc_handle_enable(void *desc)
{
	struct vcc_desc *cmd = (struct vcc_desc *)desc;

	return regulator_enable(*(cmd->regulator));
}

static int vcc_handle_disable(void *desc)
{
	struct vcc_desc *cmd = (struct vcc_desc *)desc;

	return regulator_disable(*(cmd->regulator));
}

static int vcc_handle_set_voltage(void *desc)
{
	struct vcc_desc *cmd = (struct vcc_desc *)desc;

	return regulator_set_voltage(*(cmd->regulator), cmd->min_uv, cmd->max_uv);
}

static struct peri_handle_data g_vcc_handle_data[] = {
	{ DTYPE_VCC_PUT, vcc_handle_put },
	{ DTYPE_VCC_ENABLE, vcc_handle_enable },
	{ DTYPE_VCC_DISABLE, vcc_handle_disable },
	{ DTYPE_VCC_SET_VOLTAGE, vcc_handle_set_voltage },
};

int vcc_cmds_tx(struct platform_device *pdev, struct vcc_desc *cmds, int cnt)
{
	int ret = 0;
	int i;
	int j;
	struct vcc_desc *cm = cmds;

	for (i = 0; i < cnt; i++) {
		if (!cm || !cm->label) {
			dpu_pr_err("cmds or cmds->label is NULL! i=%d\n", i);
			return -1;
		}

		ret = -1;
		for (j = 0; j < ARRAY_SIZE(g_vcc_handle_data); j++) {
			if (IS_ERR(*(cm->regulator)))
				return -1;
			if (cm->dtype == g_vcc_handle_data[i].dtype) {
				ret = g_vcc_handle_data[j].handle_func((void *)cm);
				break;
			}
		}

		if (cm->dtype == DTYPE_VCC_GET) {
			if (!pdev)
				return -1;
			*(cm->regulator) = devm_regulator_get(&pdev->dev, cm->label);
			if (IS_ERR(*(cm->regulator))) {
				dpu_pr_err("failed to get %s regulator!\n", cm->label);
				return -1;
			}
		}

		if (ret != 0) {
			dpu_pr_err("dtype=%x handle failed, ret=%d, label: %s\n", cm->dtype, ret, cm->label);
			return ret;
		}
		cmds_tx_delay(cm->wait, cm->waittype);
		cm++;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(vcc_cmds_tx);

static int pinctrl_get_state_by_mode(struct pinctrl_cmd_desc *cmds)
{
	if (cmds->mode == DTYPE_PINCTRL_STATE_DEFAULT) {
		cmds->pctrl_data->pinctrl_def = pinctrl_lookup_state(cmds->pctrl_data->p, PINCTRL_STATE_DEFAULT);
		if (IS_ERR(cmds->pctrl_data->pinctrl_def)) {
			dpu_pr_err("failed to get pinctrl_def!\n");
			return -1;
		}
	} else if (cmds->mode == DTYPE_PINCTRL_STATE_IDLE) {
		cmds->pctrl_data->pinctrl_idle = pinctrl_lookup_state(cmds->pctrl_data->p, PINCTRL_STATE_IDLE);
		if (IS_ERR(cmds->pctrl_data->pinctrl_idle)) {
			dpu_pr_err("failed to get pinctrl_idle!\n");
			return -1;
		}
	}
	return 0;
}

static int pinctrl_set_state_by_mode(struct pinctrl_cmd_desc *cmds)
{
	if (cmds->mode == DTYPE_PINCTRL_STATE_DEFAULT) {
		if (cmds->pctrl_data->p && cmds->pctrl_data->pinctrl_def)
			return pinctrl_select_state(cmds->pctrl_data->p, cmds->pctrl_data->pinctrl_def);

	} else if (cmds->mode == DTYPE_PINCTRL_STATE_IDLE) {
		if (cmds->pctrl_data->p && cmds->pctrl_data->pinctrl_idle)
			return pinctrl_select_state(cmds->pctrl_data->p, cmds->pctrl_data->pinctrl_idle);
	}

	return 0;
}

int pinctrl_cmds_tx(struct platform_device *pdev, struct pinctrl_cmd_desc *cmds, int cnt)
{
	int ret = -1;
	int i;
	struct pinctrl_cmd_desc *cm = NULL;

	cm = cmds;
	for (i = 0; i < cnt; i++) {
		if (!cm) continue;
		if (cm->dtype == DTYPE_PINCTRL_GET) {
			if (!pdev)
				return -EINVAL;
			cm->pctrl_data->p = devm_pinctrl_get(&pdev->dev);
			ret = IS_ERR(cm->pctrl_data->p);
		} else if (cm->dtype == DTYPE_PINCTRL_STATE_GET) {
			ret = pinctrl_get_state_by_mode(cm);
		} else if (cm->dtype == DTYPE_PINCTRL_SET) {
			ret = pinctrl_set_state_by_mode(cm);
		} else if (cm->dtype == DTYPE_PINCTRL_PUT) {
			if (cm->pctrl_data->p)
				pinctrl_put(cm->pctrl_data->p);
		}

		if (ret != 0) {
			dpu_pr_err("ret:%d err handle dtype: %d index: %d!\n", ret, cm->dtype, i);
			return ret;
		}
		cm++;
	}
	return ret;
}
EXPORT_SYMBOL_GPL(pinctrl_cmds_tx);

MODULE_LICENSE("GPL");