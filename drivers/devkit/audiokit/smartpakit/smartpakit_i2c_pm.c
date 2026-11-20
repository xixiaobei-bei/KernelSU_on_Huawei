/*
 * smartpakit_i2c_pm.c
 *
 * smartpakit s4 pm driver
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
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/irqreturn.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/regulator/consumer.h>
#include <huawei_platform/log/hw_log.h>
#include "smartpakit.h"

#define HWLOG_TAG smartpakit
HWLOG_REGIST();

int smartpakit_i2c_freeze(struct device *dev)
{
	struct smartpakit_i2c_priv *i2c_priv = NULL;
	struct smartpakit_gpio_irq *irq_handler = NULL;

	hwlog_info("%s: begin\n", __func__);
	if (dev == NULL) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}
	i2c_priv = dev_get_drvdata(dev);
	if (i2c_priv == NULL) {
		hwlog_err("%s: invalid i2c_priv\n", __func__);
		return -EINVAL;
	}

	irq_handler = i2c_priv->irq_handler;
	if (irq_handler != NULL) {
		disable_irq(irq_handler->irq);
		hwlog_info("%s: end and disable irq : %d\n",
					__func__, irq_handler->irq);
	}

	return 0;
}

void smartpakit_restore_process(struct work_struct *work)
{
	int ret = 0;
	struct smartpakit_i2c_priv *i2c_priv = NULL;
	struct smartpakit_priv *pakit_priv = NULL;
	struct smartpakit_gpio_irq *irq_handler = NULL;

	if (work == NULL) {
		hwlog_err("%s: work null invalid argument\n", __func__);
		return;
	}

	i2c_priv = container_of(work, struct smartpakit_i2c_priv,
		pm_s4_work);
	if (i2c_priv == NULL || i2c_priv->priv_data == NULL) {
		hwlog_err("%s:i2c_priv or priv_data null.\n", __func__);
		return;
	}
	hwlog_info("%s: enter, chip %u\n", __func__, i2c_priv->chip_id);
	pakit_priv = i2c_priv->priv_data;
	irq_handler = i2c_priv->irq_handler;

	mutex_lock(&pakit_priv->irq_handler_lock);
	mutex_lock(&pakit_priv->hw_reset_lock);
	mutex_lock(&pakit_priv->i2c_ops_lock);
	ret = smartpakit_hw_reset_chips(pakit_priv);
	if (ret != 0) {
		hwlog_err("%s: smartpakit_hw_reset_chips error[%d]\n",
					__func__, ret);
		goto err_out;
	}
	ret = smartpakit_resume_chips(pakit_priv);
	if (ret != 0) {
		hwlog_err("%s: smartpakit_resume_chips error[%d]\n",
					__func__, ret);
		goto err_out;
	}

	if (irq_handler != NULL) {
		enable_irq(irq_handler->irq);
		hwlog_info("%s: end and enable irq : %d\n",
					__func__, irq_handler->irq);
	}

err_out:
	mutex_unlock(&pakit_priv->i2c_ops_lock);
	mutex_unlock(&pakit_priv->hw_reset_lock);
	mutex_unlock(&pakit_priv->irq_handler_lock);
	hwlog_info("%s: enter end\n", __func__);
}

int smartpakit_i2c_restore(struct device *dev)
{
	int ret = 0;
	struct smartpakit_i2c_priv *i2c_priv = NULL;
	struct smartpakit_priv *pakit_priv = NULL;

	if (dev == NULL) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	i2c_priv = dev_get_drvdata(dev);
	if (i2c_priv == NULL) {
		hwlog_err("%s: invalid i2c_priv\n", __func__);
		return -EINVAL;
	}
	hwlog_info("%s: begin\n", __func__);
	if (!work_busy(&i2c_priv->pm_s4_work)) {
		hwlog_info("%s: pm_s4_work schedule_work enter\n", __func__);
		schedule_work(&i2c_priv->pm_s4_work);
	} else {
		hwlog_info("%s: work busy, skip\n", __func__);
	}

	return 0;
}
