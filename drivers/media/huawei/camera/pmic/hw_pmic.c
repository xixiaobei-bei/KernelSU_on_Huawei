/* Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "hwcam_intf.h"
#include "cam_log.h"
#include "hw_pmic.h"
#include "hw_pmic_i2c.h"

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif
#define GPIO_IRQ_REGISTER_SUCCESS 1
extern int snprintf_s(char* strDest, size_t destMax, size_t count, const char* format, ...);
struct hisi_pmic_ctrl_t *hisi_pmic_ctrl = NULL;

struct dsm_client *client_pmic = NULL;
int dsm_notify_limit = 0;
static int pmic_vote_status[VOUT_MAX];

struct dsm_dev dsm_cam_pmic = {
	.name = "dsm_cam_pmic",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = 256,
};
int pmic_ctl_otg_onoff(bool on_off)
{
    int pmic_ctl_gpio_otg_switch = 0;
	pmic_ctl_gpio_otg_switch = of_get_named_gpio(of_find_compatible_node(
		NULL, NULL, "huawei,vbus_channel_boost_gpio"),
		"gpio_otg_switch", 0);
    cam_info("pmic_ctl_otg_onoff,ret = %d\n",pmic_ctl_gpio_otg_switch);
    if (pmic_ctl_gpio_otg_switch > 0)
    {
        cam_info("otg_gpio is %d\n",pmic_ctl_gpio_otg_switch);
        gpio_set_value(pmic_ctl_gpio_otg_switch,on_off);
    }
    return pmic_ctl_gpio_otg_switch;
}
int pmic_enable_boost(int value)
{
    struct hisi_pmic_ctrl_t *pmic_ctrl = hisi_get_pmic_ctrl();
    int rc = -1;
    u32 voltage = 0;

    if((pmic_ctrl == NULL) || (pmic_ctrl->func_tbl == NULL) || (pmic_ctrl->func_tbl->pmic_seq_config == NULL)) {
        cam_err("pmic_ctrl is NULL,just return");
        return -1;
    }

    if (value == 0) {
        rc = pmic_ctrl->func_tbl->pmic_seq_config(pmic_ctrl, (pmic_seq_index_t)VOUT_BOOST, (u32)voltage, 0);
    } else {
        rc = pmic_ctrl->func_tbl->pmic_seq_config(pmic_ctrl, (pmic_seq_index_t)VOUT_BOOST, (u32)voltage, 1);
    }

    cam_err("%s rc=%d\n",__func__, rc);
    return rc;
}

EXPORT_SYMBOL(pmic_enable_boost);

void hisi_set_pmic_ctrl(struct hisi_pmic_ctrl_t *pmic_ctrl)
{
	hisi_pmic_ctrl = pmic_ctrl;
}

struct hisi_pmic_ctrl_t * hisi_get_pmic_ctrl(void)
{
	return hisi_pmic_ctrl;
}

static irqreturn_t hisi_pmic_interrupt_handler(int vec, void *info)
{
	struct hisi_pmic_ctrl_t *pmic_ctrl = NULL;
	if (NULL == info) {
		return IRQ_NONE;
	}

	pmic_ctrl = (struct hisi_pmic_ctrl_t *)info;
	if (NULL == pmic_ctrl->func_tbl){
	    return IRQ_NONE;
	}
	if (pmic_ctrl->func_tbl->pmic_check_exception != NULL) {
		pmic_ctrl->func_tbl->pmic_check_exception(pmic_ctrl);
	}
	return IRQ_HANDLED;
}

void irq_err_time_reset(struct irq_err_monitor *irq_err)
{
	if (!irq_err)
		return;
	irq_err->last_time.tv_sec = irq_err->now.tv_sec;
	irq_err->last_time.tv_usec = irq_err->now.tv_usec;
}

void pmic_fault_reset_check(struct hisi_pmic_ctrl_t *pmic_ctrl,
	struct irq_err_monitor *irq_err, unsigned int latch_time,
	const unsigned int sche_work_time)
{
	if (!irq_err || !pmic_ctrl)
		return;
	if (pmic_ctrl->pmic_info.fault_check == 0) {
		cam_info("not support fault err check");
		return;
	}
	do_gettimeofday(&irq_err->now);
	irq_err->irq_time = (irq_err->now.tv_sec - irq_err->last_time.tv_sec) *
		(1000000L) + (irq_err->now.tv_usec - irq_err->last_time.tv_usec);
	cam_err("%s err time = %ld", __func__, irq_err->irq_time);
	if (irq_err->irq_time > latch_time) {
		cam_err("pmic fault err check after %u", latch_time);
		irq_err_time_reset(irq_err);
		schedule_delayed_work(&pmic_ctrl->pmic_err_work,
			msecs_to_jiffies(sche_work_time));
	}
}

static int hisi_pmic_type_ctrl_cfg(struct hisi_pmic_ctrl_t *pmic_ctrl,
	pmic_seq_index_t idx, u32 vol, int state)
{
	int ret;

	if ((pmic_ctrl == NULL) || (pmic_ctrl->func_tbl == NULL) ||
		(pmic_ctrl->func_tbl->pmic_buck_vote_cfg == NULL) ||
		(pmic_ctrl->func_tbl->pmic_ldo_vote_cfg == NULL)) {
		cam_err("pmic_ctrl is NULL, just return");
		return -1;
	}

	if (idx > VOUT_LDO_5)
		ret = pmic_ctrl->func_tbl->pmic_buck_vote_cfg(pmic_ctrl,
			idx, vol, state);
	else
		ret = pmic_ctrl->func_tbl->pmic_ldo_vote_cfg(pmic_ctrl,
			idx, vol, state);
	return ret;
}

int hisi_pmic_vote_cfg(struct hisi_pmic_ctrl_t *pmic_ctrl,
	pmic_seq_index_t idx, u32 vol, int state)
{
	int ret = 0;

	if (idx >= VOUT_MAX)
		return -1;

	if (state == 0) {
		if (pmic_vote_status[idx] > PMIC_ENABLE_CNT) {
			cam_info("%s state_votes = %d, cannot close PMIC-%d",
				__func__, pmic_vote_status[idx], idx);
			pmic_vote_status[idx]--;
		} else if (pmic_vote_status[idx] == PMIC_ENABLE_CNT) {
			ret = hisi_pmic_type_ctrl_cfg(pmic_ctrl, idx, vol, state);
			if (ret == 0) {
				pmic_vote_status[idx] = PMIC_DISABLE_CNT;
				cam_info("%s close PMIC-%d suc", __func__, idx);
			} else {
				cam_err("%s close PMIC-%d err", __func__, idx);
			}
		} else {
			cam_err("%s invalid vote count = %d", __func__,
				pmic_vote_status[idx]);
			ret = -1;
		}
	} else {
		if (pmic_vote_status[idx] > PMIC_DISABLE_CNT) {
			cam_info("%s state_votes = %d, not open PMIC-%d again",
				__func__, pmic_vote_status[idx], idx);
			pmic_vote_status[idx]++;
		} else if (pmic_vote_status[idx] == PMIC_DISABLE_CNT) {
			ret = hisi_pmic_type_ctrl_cfg(pmic_ctrl, idx, vol, state);
			if (ret == 0) {
				pmic_vote_status[idx] = PMIC_ENABLE_CNT;
				cam_info("%s open PMIC-%d suc", __func__, idx);
			} else {
				cam_err("%s open PMIC-%d err", __func__, idx);
			}
		} else {
			cam_err("%s invalid vote count = %d", __func__,
				pmic_vote_status[idx]);
			ret = -1;
		}
	}
	return ret;
}

int hisi_pmic_gpio_boost_enable(struct hisi_pmic_ctrl_t *pmic_ctrl, int state)
{
	int ret = -EPERM;
	struct hisi_pmic_info *pmic_info = NULL;

	cam_info("%s enter\n", __func__);
	if (!pmic_ctrl)
		return ret;
	pmic_info = &pmic_ctrl->pmic_info;
	if (pmic_info->boost_en_pin == 0) {
		cam_info("boost enable pin not controled here\n");
		return ret;
	}

	if (state)
		gpio_set_value(pmic_info->boost_en_pin, 1);
	else
		gpio_set_value(pmic_info->boost_en_pin, 0);
	return 0;
}

int hisi_pmic_setup_intr(struct hisi_pmic_ctrl_t *pmic_ctrl)
{
	struct device_node *dev_node = NULL;
	struct hisi_pmic_info *pmic_info = NULL;
	int rc = -1;

	cam_info("%s enter.\n", __func__);

	if (NULL == pmic_ctrl) {
		cam_err("%s pmic_ctrl is NULL.", __func__);
		return rc;
	}

	dev_node = pmic_ctrl->dev->of_node;

	pmic_info = &pmic_ctrl->pmic_info;

	// intrrupt pin
	rc = of_property_read_u32(dev_node, "hisi,pmic-intr", &pmic_info->intr);
	cam_info("%s huawei,pmic-intr %u", __func__, pmic_info->intr);
	if (rc < 0) {
		cam_err("%s, failed %d\n", __func__, __LINE__);
		rc = 0; //ignore if pmic chip is not support interrupt mode;
		goto fail;
	}

	// setup intrrupt
	rc = gpio_request(pmic_info->intr, "pmic-power-intr");
	if (rc < 0) {
		cam_err("%s failed to request pmic-power-ctrl pin. rc = %d.", __func__, rc);
		goto fail;
	}

	rc = gpio_direction_input(pmic_info->intr);
	if (rc < 0) {
		cam_err("fail to configure intr as input %d", rc);
		goto direction_failed;
	}

	rc = gpio_to_irq(pmic_info->intr);
	if (rc < 0) {
		cam_err("fail to irq");
		goto direction_failed;
	}
	pmic_info->irq = rc;

	rc = request_threaded_irq(pmic_info->irq, NULL,
			hisi_pmic_interrupt_handler,
			IRQF_TRIGGER_FALLING|IRQF_ONESHOT,
			"hisi_pmic_interrupt",
			(void *)pmic_ctrl);
	if (rc < 0) {
		cam_err("allocate rt5112 int fail ! result:%d\n",  rc);
		goto irq_failed;
	}
	pmic_info->flag = GPIO_IRQ_REGISTER_SUCCESS;
	return 0;

irq_failed:
	free_irq(pmic_info->irq,(void *)pmic_ctrl);
direction_failed:
	gpio_free(pmic_info->intr);
fail:
	return rc;
}
EXPORT_SYMBOL(hisi_pmic_setup_intr);

void hisi_pmic_release_intr(struct hisi_pmic_ctrl_t *pmic_ctrl)
{
	struct hisi_pmic_info *pmic_info = NULL;
	cam_info("%s enter.\n", __func__);

	if (NULL == pmic_ctrl) {
		cam_err("%s pmic_ctrl is NULL.", __func__);
		return;
	}
	pmic_info = &pmic_ctrl->pmic_info;
	if(pmic_info->flag != GPIO_IRQ_REGISTER_SUCCESS){
	    cam_err("%s pmic_info->irq failed.",__func__);
	    return;
	}
	free_irq(pmic_info->irq, (void*)pmic_ctrl);
	gpio_free(pmic_info->intr);
}
EXPORT_SYMBOL(hisi_pmic_release_intr);

int hisi_pmic_get_dt_data(struct hisi_pmic_ctrl_t *pmic_ctrl)
{
	struct device_node *dev_node = NULL;
	struct hisi_pmic_info *pmic_info = NULL;
	static bool gpio_req_status = false;
	int rc = -1;

	cam_info("%s enter.\n", __func__);

	if (NULL == pmic_ctrl) {
		cam_err("%s pmic_ctrl is NULL.", __func__);
		return rc;
	}

	dev_node = pmic_ctrl->dev->of_node;

	pmic_info = &pmic_ctrl->pmic_info;

	rc = of_property_read_string(dev_node, "hisi,pmic_name", &pmic_info->name);
	cam_info("%s hisi,pmic_name %s, rc %d\n", __func__, pmic_info->name, rc);
	if (rc < 0) {
		cam_err("%s failed %d\n", __func__, __LINE__);
		goto fail;
	}

	rc = of_property_read_u32(dev_node, "hisi,pmic_index",
		(u32 *)&pmic_info->index);
	cam_info("%s hisi,pmic_index %d, rc %d\n", __func__,
		pmic_info->index, rc);
	if (rc < 0) {
		cam_err("%s failed %d\n", __func__, __LINE__);
		goto fail;
	}

	rc = of_property_read_u32(dev_node, "hisi,slave_address",
		&pmic_info->slave_address);
	cam_info("%s slave_address %d, rc %d\n", __func__,
		pmic_info->slave_address, rc);
	if (rc < 0) {
		cam_err("%s failed %d\n", __func__, __LINE__);
		goto fail;
	}

	rc = of_property_read_u32(dev_node, "hw,boost-pin",
		&pmic_info->boost_en_pin);
	if (rc < 0) {
		cam_warn("%s, get boost enable pin failed\n", __func__);
		pmic_info->boost_en_pin = 0;
		rc = 0;
	} else {
		cam_info("enable boost-pin = %d\n", pmic_info->boost_en_pin);
		if (!gpio_req_status) {
			rc = gpio_request(pmic_info->boost_en_pin,
				"hisi_pmic_boost_en");
			if (rc < 0) {
				cam_err("fail req boost en = %d\n", rc);
				return -EPERM;
			}
			gpio_direction_output(pmic_info->boost_en_pin, 1);
			gpio_req_status = true;
		}
	}

	rc = of_property_read_u32(dev_node, "hisi,fault_check",
		&pmic_info->fault_check);
	if (rc < 0) {
		cam_info("%s failed set default\n", __func__);
		pmic_info->fault_check = 0;
		rc = 0;
	}
	cam_info("%s fault_check = %u", __func__, pmic_info->fault_check);

fail:
	return rc;
}

int hisi_pmic_config(struct hisi_pmic_ctrl_t *pmic_ctrl, void *argp)
{
	return 0;
}

EXPORT_SYMBOL(hisi_pmic_config);

int32_t hisi_pmic_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = NULL;
	struct hisi_pmic_ctrl_t *pmic_ctrl = NULL;
	int32_t rc=0;
	dsm_notify_limit = 0;

	cam_info("%s client name = %s.\n", __func__, client->name);

	adapter = client->adapter;
	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C)) {
		cam_err("%s i2c_check_functionality failed.\n", __func__);
		return -EIO;
	}

	pmic_ctrl = (struct hisi_pmic_ctrl_t *)id->driver_data;
	pmic_ctrl->pmic_i2c_client->client = client;/*[false alarm]:pmic_ctrl is not alarm*/
	pmic_ctrl->dev = &client->dev;
	pmic_ctrl->pmic_i2c_client->i2c_func_tbl = &hisi_pmic_i2c_func_tbl;

	rc = hisi_pmic_get_dt_data(pmic_ctrl);
	if (rc < 0) {
		cam_err("%s hisi_pmic_get_dt_data failed.", __func__);
		return -EFAULT;
	}

	rc = pmic_ctrl->func_tbl->pmic_get_dt_data(pmic_ctrl);
	if (rc < 0) {
		cam_err("%s flash_get_dt_data failed.", __func__);
		return -EFAULT;
	}

	rc = pmic_ctrl->func_tbl->pmic_init(pmic_ctrl);
	if (rc < 0) {
		cam_err("%s pmic init failed.\n", __func__);
		return -EFAULT;
	}

	rc = pmic_ctrl->func_tbl->pmic_match(pmic_ctrl);
	if (rc < 0) {
		cam_err("%s pmic match failed.\n", __func__);
		return -EFAULT;
	}

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	/* detect current device successful, set the flag as present */
#endif
	hisi_set_pmic_ctrl(pmic_ctrl);

	if (NULL == client_pmic) {
		client_pmic = dsm_register_client(&dsm_cam_pmic);
	}
	return rc;
}
