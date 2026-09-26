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
#include <linux/delay.h>
#include <linux/mutex.h>
#include "hw_pmic.h"
#include "../cam_log.h"
#include "../platform/sensor_commom.h"
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif

//lint -save -e31

// RT5112 register define
#define REVERSION_ID (0x2A)

// LDO
#define LDO1_CTL (0x01)
#define LDO2_CTL (0x02)
#define LDO3_CTL (0x03)
#define LDO4_CTL (0x04)
#define LDO1_VOUT (0x0D) // 0.6V-3.775V 25mV/step
#define LDO2_VOUT (0x0E) // 0.6V-3.775V 25mV/step
#define LDO3_VOUT (0x0F) // 0.6V-3.775V 25mV/step
#define LDO4_VOUT (0x10) // 0.6V-3.775V 25mV/step
// 600000 -> 3775000
// step 25000 (0.025)
#define RT5112_LDO_VOLTAGE_MIN (600000)
#define RT5112_LDO_VOLTAGE_MAX (3775000)
#define RT5112_LDO_VOLTAGE_STEP (25000)

// Buck
#define BUCK_CTL (0x00)
#define BUCK2_CTL (0x06)
#define BUCK_VOUT (0x0C) // 0.6V-3.3V 12.5mV/step
#define BUCK2_VOUT (0x12) // 0.6V-3.3V 12.5mV/step

#define RT5112_BUCK_VOLTAGE_MIN (600000)
#define RT5112_BUCK_VOLTAGE_MAX (3300000)
#define RT5112_BUCK_VOLTAGE_STEP (12500)
// Boost
#define BOOST_CTL (0x05)
#define BOOST_ENABLE (0x08)
#define BOOST_VOUT (0x11) //4.5V-5.5V 25mV/step
#define BOOST_VOUT_4V5 0x03
#define BOOST_VOUT_5V0 0x50
#define BOOST_VOUT_5V5 0xA0
#define BOOST_VOUT_5V15 0x68
#define BOOST_VOUT_5V3 0x80
#define BOOST_VOUT_OFF 0x03

#define RT5112_BOOST_VOLTAGE_MIN (4500000)
#define RT5112_BOOST_VOLTAGE_MAX (5500000)
#define RT5112_BOOST_VOLTAGE_STEP (25000)

// OVP && OCP
#define RT5112_PMU_STATUS_REG (0x43)
#define RT5112_UVP_STATUS_REG (0x15)
#define RT5112_OCP_STATUS_REG (0x17)
#define RT5112_OVP_STATUS_REG (0x16)
// interrupt
#define RT5112_INT_STATUS_REG (0x14)
#define RT5112_MASK_INTR_REG (0x33)
#define RT5112_MASK_BOOST_5V (0x04)
#define RT5112_MASK_UV_EVT (0x04)
#define RT5112_MASK_FAULT_EVT (0x01)
#define RT5112_MASK_OC_EVT (0x04)

// PIN
#define RT5112_PIN_ENABLE (1)
#define RT5112_PIN_DISABLE (0)
#define RT5112_POWERON_MASK   (0x80)
#define RT5112_POWEROFF_MASK (0x7F)

// BUCK_PWM_MODE
#define RT5112_BUCK_POWERON_MASK 0x81
#define RT5112_BUCK_POWEROFF_MASK 0x7E
#define RT5112_BUCK_PWM_MODE 1
#define RT5112_LDO2_CTRL 1
#define RT5112_PMIC_LATCH_RESET   1
#define RT5112_MAX_LATCH_TIME     500000L
#define RT5112_LATCH_CHECK_TIME   500
#define RT5112_BOOST_LATCH_OFF    0xC0

// Private data struct
struct rt5112_private_data_t {
    unsigned int pin;
    unsigned int voltage[VOUT_MAX];
	unsigned int buck_pwm_mode;
	unsigned int ldo2_ctrl;
	unsigned int shutdown_4v5;
	unsigned int reset;
	unsigned int ldo_vote;
	unsigned int buck_vote;
	unsigned int ctl_otg;
	unsigned int boost_5v15;
	unsigned int boost_5v3;
};

typedef struct {
    int chx_enable;
    int vout_reg;
}voltage_map_t;

static voltage_map_t voltage_map[VOUT_MAX] =
{
    {LDO1_CTL, LDO1_VOUT},
    {LDO2_CTL, LDO2_VOUT},
    {LDO3_CTL, LDO3_VOUT},
    {LDO4_CTL, LDO4_VOUT},
    {0, 0,}, //stub not support LDO5
    {BUCK_CTL, BUCK_VOUT},
};

// static var
static struct rt5112_private_data_t rt5112_pdata;
static struct i2c_driver rt5112_i2c_driver;
static struct hisi_pmic_ctrl_t rt5112_ctrl;
extern struct dsm_client *client_pmic;
static int boost_enabled;
static int pmic_en_reset;

DEFINE_HISI_PMIC_MUTEX(rt5112);

static int pmic_check_state_exception(struct hisi_pmic_ctrl_t *pmic_ctrl);
static void rt5112_clear_interrupt(struct hisi_pmic_ctrl_t *pmic_ctrl);
static void pmic_fault_err_work(struct work_struct *work);

static u32 calc_voltage_level(u32 voltage, u32 max, u32 min, u32 step)
{
    if (voltage > max || voltage < min)
    {
        return min;
    }

    return (voltage - min) / step;
}

static int rt5112_boost_seq_config(struct hisi_pmic_ctrl_t *pmic_ctrl,
                              pmic_seq_index_t seq_index,
                              u32 voltage,
                              int state)
{
    struct hisi_pmic_i2c_client *i2c_client = NULL;
    struct hisi_pmic_i2c_fn_t *i2c_func = NULL;
    u8 chx_enable = 0;
    u8 chx_enable_reg = 0;
    int ret = 0;

    cam_err("%s enter.", __func__);

    if (NULL == pmic_ctrl) {
        cam_err("%s pmic_ctrl is NULL.", __func__);
        return -1;
    }

    if (seq_index != VOUT_BOOST) {
        cam_err("%s seq_index error, seq_index[%u]", __func__, seq_index);
        return -1;
    }

    if (state != POWER_OFF && state != POWER_ON) {
        cam_err("%s state [%u] error", __func__, state);
        return -1;
    }

    i2c_client = pmic_ctrl->pmic_i2c_client;
    i2c_func = pmic_ctrl->pmic_i2c_client->i2c_func_tbl;

    chx_enable_reg = BOOST_ENABLE;

    i2c_func->i2c_read(i2c_client, chx_enable_reg, &chx_enable);
    if (state == POWER_ON) {
        chx_enable = chx_enable | POWER_ON;
    }
    else
    {
        chx_enable = chx_enable & POWER_OFF;
    }

    i2c_func->i2c_write(i2c_client, chx_enable_reg, chx_enable);
    cam_err("%s, power state[%d], chx_reg[%u],chx_enable[%u]", __func__,
            state, chx_enable_reg, chx_enable);
	boost_enabled = state;
    return ret;
}

static int rt5112_buck_seq_config(struct hisi_pmic_ctrl_t *pmic_ctrl,
                              pmic_seq_index_t seq_index,
                              u32 voltage,
                              int state)
{
    struct rt5112_private_data_t *pdata = NULL;
    struct hisi_pmic_i2c_client *i2c_client = NULL;
    struct hisi_pmic_i2c_fn_t *i2c_func = NULL;
    u8 chx_enable = 0;
    u8 chx_enable_reg = 0;
    u8 voltage_reg = 0;
    u32 voltage_level = 0;
    int ret = 0;

    if (NULL == pmic_ctrl) {
        cam_err("%s pmic_ctrl is NULL.", __func__);
        return -1;
    }

    if (seq_index != VOUT_BUCK_1) {
        cam_err("%s seq_index error, seq_index[%u]", __func__, seq_index);
        return -1;
    }

    if (voltage > RT5112_BUCK_VOLTAGE_MAX \
        || voltage < RT5112_BUCK_VOLTAGE_MIN) {
        cam_err("%s voltage error, vol[%u]", __func__, voltage);
        return -1;
    }

    if (state != POWER_OFF && state != POWER_ON) {
        cam_err("%s state [%u] error", __func__, state);
        return -1;
    }

	pdata = (struct rt5112_private_data_t *)pmic_ctrl->pdata;
	if (pdata == NULL) {
		cam_err("%s pdata error", __func__);
		return -1;
	}
    i2c_client = pmic_ctrl->pmic_i2c_client;
    i2c_func = pmic_ctrl->pmic_i2c_client->i2c_func_tbl;

    chx_enable_reg = voltage_map[seq_index].chx_enable;
    voltage_reg = voltage_map[seq_index].vout_reg;

    i2c_func->i2c_read(i2c_client, chx_enable_reg, &chx_enable);
    if (state == POWER_ON) {
        voltage_level = calc_voltage_level(voltage,
                                            RT5112_BUCK_VOLTAGE_MAX,
                                            RT5112_BUCK_VOLTAGE_MIN,
                                            RT5112_BUCK_VOLTAGE_STEP);
        // set voltage
		i2c_func->i2c_write(i2c_client, voltage_reg, voltage_level);
		if (pdata->buck_pwm_mode == RT5112_BUCK_PWM_MODE) {
			chx_enable = chx_enable | RT5112_BUCK_POWERON_MASK;
		}
		else {
			chx_enable = chx_enable | RT5112_POWERON_MASK;
		}
    }
    else
    {
		if (pdata->buck_pwm_mode == RT5112_BUCK_PWM_MODE) {
			chx_enable = chx_enable & RT5112_BUCK_POWEROFF_MASK;
		}
		else {
			chx_enable = chx_enable & RT5112_POWEROFF_MASK;
		}
    }

    i2c_func->i2c_write(i2c_client, chx_enable_reg, chx_enable);
    cam_info("%s, power state[%d], chx_reg[%u], reg[%u], vol[%u],vol_level[%u]", __func__,
            state, chx_enable_reg, voltage_reg, voltage, voltage_level);

    return ret;
}
// RT5112 function
static int rt5112_ldo_seq_config(struct hisi_pmic_ctrl_t *pmic_ctrl,
                              pmic_seq_index_t seq_index,
                              u32 voltage,
                              int state)
{
    struct hisi_pmic_i2c_client *i2c_client = NULL;
    struct hisi_pmic_i2c_fn_t *i2c_func = NULL;
    u8 chx_enable = 0;
    u8 chx_enable_reg = 0;
    u8 voltage_reg = 0;
    u32 voltage_level = 0;
    int ret = 0;

	cam_info("%s enter, ldo%d, onoff-%d, vol = %d", __func__,
		seq_index, state, voltage);

    if (NULL == pmic_ctrl) {
        cam_err("%s pmic_ctrl is NULL.", __func__);
        return -1;
    }

    if (seq_index < VOUT_LDO_1 || seq_index > VOUT_LDO_4) {
        cam_err("%s seq_index error, seq_index[%u]", __func__, seq_index);
        return -1;
    }

    if (voltage > RT5112_LDO_VOLTAGE_MAX \
        || voltage < RT5112_LDO_VOLTAGE_MIN) {
        cam_err("%s voltage error, vol[%u]", __func__, voltage);
        return -1;
    }

    if (state != POWER_OFF && state != POWER_ON) {
        cam_err("%s state [%u] error", __func__, state);
        return -1;
    }

    i2c_client = pmic_ctrl->pmic_i2c_client;
    i2c_func = pmic_ctrl->pmic_i2c_client->i2c_func_tbl;

    chx_enable_reg = voltage_map[seq_index].chx_enable;
    voltage_reg = voltage_map[seq_index].vout_reg;

    i2c_func->i2c_read(i2c_client, chx_enable_reg, &chx_enable);
    if (state == POWER_ON) {
        voltage_level = calc_voltage_level(voltage,
                                            RT5112_LDO_VOLTAGE_MAX,
                                            RT5112_LDO_VOLTAGE_MIN,
                                            RT5112_LDO_VOLTAGE_STEP);
        voltage_level = voltage_level << 1;
        // set voltage
        i2c_func->i2c_write(i2c_client, voltage_reg, voltage_level);
        chx_enable = chx_enable | RT5112_POWERON_MASK;
    }
    else
    {
        chx_enable = chx_enable & RT5112_POWEROFF_MASK;
    }

    i2c_func->i2c_write(i2c_client, chx_enable_reg, chx_enable);
    cam_info("%s, power state[%d], chx_reg[%u], reg[%u], vol[%u],vol_level[%u]", __func__,
            state, chx_enable_reg, voltage_reg, voltage, voltage_level);

    return ret;
}

static int rt5112_seq_config(struct hisi_pmic_ctrl_t *pmic_ctrl,
                              pmic_seq_index_t seq_index,
                              u32 voltage,
                              int state)
{
	int ret = 0;
	struct rt5112_private_data_t *pdata = NULL;

	pdata = (struct rt5112_private_data_t *)pmic_ctrl->pdata;
	if (pdata == NULL) {
		cam_err("%s pdata error", __func__);
		return -1;
	}

	mutex_lock(&pmic_mut_rt5112);
	if (seq_index == VOUT_BOOST) {
		ret = rt5112_boost_seq_config(pmic_ctrl, seq_index, voltage, state);
	} else if (seq_index == VOUT_BOOST_EN) {
		ret = hisi_pmic_gpio_boost_enable(pmic_ctrl, state);
	} else if (seq_index < VOUT_LDO_5) {
		if (pdata->ldo_vote == 1) {
			cam_debug("%s, vote set ldo", __func__);
			ret = hisi_pmic_vote_cfg(pmic_ctrl, seq_index, voltage,
				state);
		} else if ((pdata->ldo2_ctrl == RT5112_LDO2_CTRL) &&
			(seq_index == VOUT_LDO_2)) {
			ret = hisi_pmic_vote_cfg(pmic_ctrl, VOUT_LDO_2, voltage,
				state);
		} else {
			ret = rt5112_ldo_seq_config(pmic_ctrl, seq_index, voltage, state);
		}
	} else {
		if (pdata->buck_vote == 1)
			ret = hisi_pmic_vote_cfg(pmic_ctrl, seq_index, voltage,
				state);
		else
			ret = rt5112_buck_seq_config(pmic_ctrl, seq_index,
				voltage, state);
	}

	mutex_unlock(&pmic_mut_rt5112);
	return ret;
}

static int rt5112_match(struct hisi_pmic_ctrl_t *pmic_ctrl)
{
    cam_debug("%s enter.", __func__);
    return 0;
}

static int rt5112_get_dt_data(struct hisi_pmic_ctrl_t *pmic_ctrl)
{
    struct rt5112_private_data_t *pdata = NULL;
    struct device_node *dev_node = NULL;
    int rc = 0;

    if (NULL == pmic_ctrl) {
        cam_err("%s pmic ctrl is NULL", __func__);
        return -1;
    }

    pdata = (struct rt5112_private_data_t *)pmic_ctrl->pdata;
    dev_node = pmic_ctrl->dev->of_node;

    // PIN Enable gpio
    rc = of_property_read_u32(dev_node, "hisi,pmic-pin", &pdata->pin);
    cam_info("%s huawei,pmic-pin %d", __func__, pdata->pin);
    if (rc  < 0) {
        cam_err("%s, failed %d\n", __func__, __LINE__);
        goto fail;
    }

	// buck pwm forced mode
	rc = of_property_read_u32(dev_node, "hisi,pmic_buck_pwm_mode", &pdata->buck_pwm_mode);
	if (rc < 0) {
		pdata->buck_pwm_mode = 0;
		cam_info("%s, cannot get buck config, set to default buck mode", __func__);
	}
	cam_info("%s huawei,pmic_buck_pwm_mode %d", __func__, pdata->buck_pwm_mode);

	// ldo2_ctrl
	rc = of_property_read_u32(dev_node, "hisi,pmic_ldo2_ctrl", &pdata->ldo2_ctrl);
	if (rc < 0) {
		pdata->ldo2_ctrl = 0;
		cam_info("%s, cannot get buck config, set to default ldo2_ctrl", __func__);
	}
	rc = of_property_read_u32(dev_node, "hisi,shutdown_4V5",
		&pdata->shutdown_4v5);
	if (rc < 0) {
		pdata->shutdown_4v5 = 0;
		cam_info("%s, cannot get shutdown 4v5 set, set 0", __func__);
	}
	cam_info("%s shutdown 4v5 val = %d", __func__, pdata->shutdown_4v5);
	cam_info("%s huawei,pmic_ldo2_ctrl %d", __func__, pdata->ldo2_ctrl);

	rc = of_property_read_u32(dev_node, "hisi,ldo_vote", &pdata->ldo_vote);
	if (rc < 0) {
		pdata->ldo_vote = 0;
		cam_info("%s, not set ldo_vote", __func__);
	}
	cam_info("%s ldo_vote = %u", __func__, pdata->ldo_vote);
	rc = of_property_read_u32(dev_node, "hisi,buck_vote", &pdata->buck_vote);
	if (rc < 0) {
		pdata->buck_vote = 0;
		cam_info("%s, not set buck_vote", __func__);
	}
	cam_info("%s buck_vote = %u", __func__, pdata->buck_vote);

	// pmic not reset when exception state
	rc = of_property_read_u32(dev_node, "hisi,pmic_reset", &pdata->reset);
	if (rc < 0) {
		pdata->reset = 1;
		cam_info("%s, cannot get pmic_reset value, set 1", __func__);
	}
	cam_info("%s huawei,pmic_reset %d", __func__, pdata->reset);

	// pmic fault error when exception state
	rc = of_property_read_u32(dev_node, "hisi,ctl_otg", &pdata->ctl_otg);
	if (rc < 0) {
		pdata->ctl_otg = 1;
		cam_info("%s, support ctl_otg", __func__);
	}
	cam_info("%s ctl_otg %d", __func__, pdata->ctl_otg);

	rc = of_property_read_u32(dev_node, "hisi,boost_5v3",
		&pdata->boost_5v3);
	if (rc < 0) {
		pdata->boost_5v3 = 0;
		cam_info("%s, cannot get boost 5v5 set, set 0", __func__);
	}
	cam_info("%s boost_5v3 %u", __func__, pdata->boost_5v3);
	rc = of_property_read_u32(dev_node, "hisi,boost_5v15",
		&pdata->boost_5v15);
	if (rc < 0) {
		pdata->boost_5v15 = 0;
		cam_info("%s, cannot get boost 5v15 set, set 0", __func__);
	}
	cam_info("%s boost_5v15 %u", __func__, pdata->boost_5v15);
	return 0;

fail:
    return -EIO;
}

static int rt5112_on(struct hisi_pmic_ctrl_t *pmic_ctrl, void *data)
{
    // check Error registor
    return 0;
}

static int rt5112_off(struct hisi_pmic_ctrl_t *pmic_ctrl)
{
    // Never power off pmic when SOC is running;
    return 0;
}

static int rt5112_init(struct hisi_pmic_ctrl_t *pmic_ctrl)
{
    struct hisi_pmic_i2c_client *i2c_client = NULL;
    struct hisi_pmic_i2c_fn_t *i2c_func = NULL;
    struct rt5112_private_data_t *pdata = NULL;
    u8 chip_id = 0;
    int ret = 0;
    u8 vout_reg = 0;
    u8 boost_volt = 0;

    cam_debug("%s enter.", __func__);

    if (NULL == pmic_ctrl) {
        cam_err("%s pmic_ctrl is NULL.", __func__);
        return -1;
    }

    pmic_ctrl->pctrl = devm_pinctrl_get_select(pmic_ctrl->dev,
        PINCTRL_STATE_DEFAULT);

    if (NULL == pmic_ctrl->pctrl) {
        cam_err("%s failed to set pin.", __func__);
        return -EIO;
    }

    pdata = (struct rt5112_private_data_t *)pmic_ctrl->pdata;

    ret = hisi_pmic_setup_intr(pmic_ctrl);
    if (ret < 0) {
        cam_err("%s setup interrupt failed", __func__);
        goto err;
    }

    cam_debug("%s PMIC_POWER_CTRL = %d", __func__,
            pdata->pin);

    ret = gpio_request(pdata->pin, "pmic-power-ctrl");
    if (ret < 0) {
        cam_err("%s failed to request pmic-power-ctrl pin. ret = %d.", __func__, ret);
        goto req_failed;
    }

    gpio_direction_output(pdata->pin, RT5112_PIN_ENABLE);
    msleep(1);
    i2c_client = pmic_ctrl->pmic_i2c_client;
    i2c_func = pmic_ctrl->pmic_i2c_client->i2c_func_tbl;

    ret = i2c_func->i2c_read(i2c_client, REVERSION_ID, &chip_id);
    if (ret < 0) {
        cam_err("%s: read CHIP ID failed, ret = %d ", __func__, ret);
        goto err2;
    }
    cam_debug("%s chip id=%d", __func__, chip_id);

	i2c_func->i2c_read(i2c_client, BOOST_VOUT, &vout_reg);
	cam_info("get vboost vol reg = %d\n", vout_reg);
    //mask boost interrupt
    i2c_func->i2c_write(i2c_client, RT5112_MASK_INTR_REG, RT5112_MASK_BOOST_5V);
    //clean interrupt
    rt5112_clear_interrupt(pmic_ctrl);

	ret = i2c_func->i2c_read(i2c_client, BOOST_VOUT, &boost_volt);
	if (ret < 0) {
		cam_err("%s, read boost volt err", __func__);
		boost_volt = BOOST_VOUT_OFF;
	}
	if (pdata->boost_5v3 == 1) // set 5v3
		boost_volt = (boost_volt & BOOST_VOUT_OFF) | BOOST_VOUT_5V3;
	else if (pdata->boost_5v15 == 1) // set 5v15
		boost_volt = (boost_volt & BOOST_VOUT_OFF) | BOOST_VOUT_5V15;
	else
		boost_volt = (boost_volt & BOOST_VOUT_OFF) | BOOST_VOUT_5V0;

	ret = i2c_func->i2c_write(i2c_client, BOOST_VOUT, boost_volt);
	if (ret < 0)
		cam_err("%s, set boost volt err", __func__);
	INIT_DELAYED_WORK(&pmic_ctrl->pmic_err_work, pmic_fault_err_work);
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
    set_hw_dev_flag(DEV_I2C_CAMERA_PMIC);
#endif
    return 0;
err2:
    gpio_free(pdata->pin);
req_failed:
    hisi_pmic_release_intr(pmic_ctrl);
err:
    return -EIO;
}

static int rt5112_exit(struct hisi_pmic_ctrl_t *pmic_ctrl)
{
    struct rt5112_private_data_t *pdata = NULL;

    cam_debug("%s enter.", __func__);

    if (NULL == pmic_ctrl) {
        cam_err("%s pmic_ctrl is NULL.", __func__);
        return -1;
    }

    pdata = (struct rt5112_private_data_t *)pmic_ctrl->pdata;
    gpio_free(pdata->pin);
    pmic_ctrl->pctrl = devm_pinctrl_get_select(pmic_ctrl->dev,
        PINCTRL_STATE_IDLE);
    hisi_pmic_release_intr(pmic_ctrl);
    return 0;
}

static int rt5112_register_attribute(struct hisi_pmic_ctrl_t *pmic_ctrl, struct device *dev)
{
    return 0;
}

static void pmic_hw_en_reset(struct rt5112_private_data_t *pdata)
{
	if (!pdata) {
		cam_err("%s null", __func__);
		return;
	}
	gpio_set_value(pdata->pin, RT5112_PIN_DISABLE);
	udelay(1000);
	gpio_set_value(pdata->pin, RT5112_PIN_ENABLE);
	cam_err("pmic fault err, set %s", __func__);
}

static void rt5112_clear_interrupt(struct hisi_pmic_ctrl_t *pmic_ctrl)
{
    struct hisi_pmic_i2c_client *i2c_client = NULL;
    struct hisi_pmic_i2c_fn_t *i2c_func = NULL;
	struct rt5112_private_data_t *pdata = NULL;
	u8 reg_value = 0;
	u8 pmu_status = 0;

    if (NULL == pmic_ctrl) {
        cam_err("%s pmic_ctrl is NULL.", __func__);
        return;
    }

    i2c_client = pmic_ctrl->pmic_i2c_client;
    i2c_func = pmic_ctrl->pmic_i2c_client->i2c_func_tbl;
	pdata = (struct rt5112_private_data_t *)pmic_ctrl->pdata;;
    // clean status
	i2c_func->i2c_read(i2c_client, RT5112_PMU_STATUS_REG, &pmu_status);
    i2c_func->i2c_read(i2c_client, RT5112_OVP_STATUS_REG, &reg_value);
    i2c_func->i2c_read(i2c_client, RT5112_OCP_STATUS_REG, &reg_value);
    i2c_func->i2c_read(i2c_client, RT5112_UVP_STATUS_REG, &reg_value);

    //clear interrupt
    i2c_func->i2c_read(i2c_client, RT5112_INT_STATUS_REG, &reg_value);
    i2c_func->i2c_write(i2c_client, RT5112_INT_STATUS_REG, (reg_value | 0x01));
	cam_err("%s pmu = 0x%x, en = %d",__func__, pmu_status, pmic_en_reset);
	if ((pmu_status > 0) && (pmic_en_reset == RT5112_PMIC_LATCH_RESET)) {
		cam_err("%s pmu_status = 0x%x",__func__, pmu_status);
		if (pmu_status & RT5112_BOOST_LATCH_OFF)
			pmic_ctl_otg_onoff(RT5112_PIN_DISABLE);
		pmic_hw_en_reset(pdata);
		pmic_en_reset = 0;
		if (!boost_enabled)
			return;
		cam_info("%s, pmic err, reset boost power", __func__);
		rt5112_boost_seq_config(pmic_ctrl, VOUT_BOOST, 0, POWER_ON);
	}
	pmic_en_reset = 0;
}

static void pmic_fault_err_work(struct work_struct *work)
{
	struct hisi_pmic_ctrl_t *pmic_ctrl = NULL;

	if (!work) {
		cam_err("work is null\n");
		return;
	}
	pmic_ctrl = container_of(work, struct hisi_pmic_ctrl_t,
		pmic_err_work.work);
	pmic_en_reset = RT5112_PMIC_LATCH_RESET;
	rt5112_clear_interrupt(pmic_ctrl);
}

static int pmic_check_state_exception(struct hisi_pmic_ctrl_t *pmic_ctrl)
{
    struct hisi_pmic_i2c_client *i2c_client = NULL;
    struct hisi_pmic_i2c_fn_t *i2c_func = NULL;
    struct rt5112_private_data_t *pdata = NULL;
    u8 reg_value = 0;
    u8 pmu_status = 0;
	static struct irq_err_monitor irq_err;

    if (NULL == pmic_ctrl) {
        cam_err("%s pmic_ctrl is NULL.", __func__);
        return -1;
    }

    i2c_client = pmic_ctrl->pmic_i2c_client;
    i2c_func = pmic_ctrl->pmic_i2c_client->i2c_func_tbl;
    pdata = (struct rt5112_private_data_t *)pmic_ctrl->pdata;
    cam_info("%s pmic_ctrl->pdata pin is %d.",__func__,pdata->pin);

    // PMU_STAT
    i2c_func->i2c_read(i2c_client, RT5112_PMU_STATUS_REG, &pmu_status);

    //OVP
    i2c_func->i2c_read(i2c_client, RT5112_OVP_STATUS_REG, &reg_value);
    if (reg_value != 0) {
        REPORT_DSM_STATUS(client_pmic, RT5112_OVP_STATUS_REG,
                reg_value, RT5112_PMU_STATUS_REG,
                pmu_status, DSM_CAMPMIC_OVER_CURRENT_ERROR_NO,
                "RT5112 OVP");
    }

    //OCP close OTG MOS
    i2c_func->i2c_read(i2c_client, RT5112_OCP_STATUS_REG, &reg_value);
	if (reg_value != 0) {
		REPORT_DSM_STATUS(client_pmic, RT5112_OCP_STATUS_REG,
			reg_value, RT5112_PMU_STATUS_REG, pmu_status,
			DSM_CAMPMIC_OVER_CURRENT_ERROR_NO, "RT5112 OCP");
		if ((reg_value & RT5112_MASK_OC_EVT) && pdata->ctl_otg)
			pmic_ctl_otg_onoff(RT5112_PIN_DISABLE);
	}

	// UVP close OTG MOS
	i2c_func->i2c_read(i2c_client, RT5112_UVP_STATUS_REG, &reg_value);
	if (reg_value != 0) {
		REPORT_DSM_STATUS(client_pmic, RT5112_UVP_STATUS_REG,
			reg_value, RT5112_PMU_STATUS_REG, pmu_status,
			DSM_CAMPMIC_UNDER_VOLTAGE_ERROR_NO, "RT5112 UVP");
		if (((reg_value & RT5112_MASK_UV_EVT) ||
			(reg_value & RT5112_MASK_FAULT_EVT)) && pdata->ctl_otg)
			pmic_ctl_otg_onoff(RT5112_PIN_DISABLE);
	}

	rt5112_clear_interrupt(pmic_ctrl);
	pmic_fault_reset_check(pmic_ctrl, &irq_err, RT5112_MAX_LATCH_TIME,
		RT5112_LATCH_CHECK_TIME);

	// reset rt5112_ENABLE
	if (pdata->reset != 0) {
		gpio_set_value(pdata->pin, RT5112_PIN_DISABLE);
		udelay(1000);
		gpio_set_value(pdata->pin, RT5112_PIN_ENABLE);
	} else {
		cam_info("%s not reset HWEN if irq occur everytime", __func__);
	}

    //mask boost interrupt
    i2c_func->i2c_write(i2c_client, RT5112_MASK_INTR_REG, RT5112_MASK_BOOST_5V);

	if (boost_enabled) {
		cam_info("reset boost power\n");
		rt5112_boost_seq_config(pmic_ctrl, VOUT_BOOST, 0, boost_enabled);
	}
    return 0;
}

static void rt5112_shutdown(struct i2c_client *client)
{
	struct hisi_pmic_i2c_client *rt_i2c_client = NULL;
	struct hisi_pmic_ctrl_t *rt_shut_pmic_ctrl = NULL;
	struct hisi_pmic_i2c_fn_t *i2c_func = NULL;
	struct rt5112_private_data_t *pdata = NULL;
	u8 boost_vol_value = 0;

	cam_warn("%s enter", __func__);
	rt_shut_pmic_ctrl = hisi_get_pmic_ctrl();
	if (!rt_shut_pmic_ctrl || !rt_shut_pmic_ctrl->pmic_i2c_client ||
		!rt_shut_pmic_ctrl->pmic_i2c_client->i2c_func_tbl ||
		!rt_shut_pmic_ctrl->pdata)
		return;

	pdata = (struct rt5112_private_data_t *)rt_shut_pmic_ctrl->pdata;
	if (pdata->shutdown_4v5 == 0) {
		cam_warn("%s not support shut down to 4v5", __func__);
		return;
	}

	rt_i2c_client = rt_shut_pmic_ctrl->pmic_i2c_client;
	i2c_func = rt_shut_pmic_ctrl->pmic_i2c_client->i2c_func_tbl;
	i2c_func->i2c_read(rt_i2c_client, BOOST_VOUT, &boost_vol_value);
	boost_vol_value = boost_vol_value & BOOST_VOUT_4V5;
	i2c_func->i2c_write(rt_i2c_client, BOOST_VOUT, boost_vol_value);
	cam_warn("set to 4V5");
}

static int rt5112_remove(struct i2c_client *client)
{
    cam_debug("%s enter.", __func__);

    client->adapter = NULL;
    return 0;
}

// lint -save -e31
static const struct i2c_device_id rt5112_id[] = {
    {"rt5112", (unsigned long)&rt5112_ctrl},
    {}
};

static const struct of_device_id rt5112_dt_match[] = {
    {.compatible = "hisi,rt5112"},
    {}
};

MODULE_DEVICE_TABLE(of, rt5112_dt_match);

static struct i2c_driver rt5112_i2c_driver = {
    .probe    = hisi_pmic_i2c_probe,
    .remove = rt5112_remove,
	.shutdown = rt5112_shutdown,
    .id_table   = rt5112_id,
    .driver = {
        .name = "rt5112",
        .of_match_table = rt5112_dt_match,
    },
};

static int __init rt5112_module_init(void)
{
    cam_info("%s enter.\n", __func__);
    return i2c_add_driver(&rt5112_i2c_driver);
}

static void __exit rt5112_module_exit(void)
{
    cam_info("%s enter.", __func__);
    i2c_del_driver(&rt5112_i2c_driver);
    return;
}

static struct hisi_pmic_i2c_client rt5112_i2c_client;

static struct hisi_pmic_fn_t rt5112_func_tbl = {
    .pmic_config = hisi_pmic_config,
    .pmic_init = rt5112_init,
    .pmic_exit = rt5112_exit,
    .pmic_on = rt5112_on,
    .pmic_off = rt5112_off,
    .pmic_match = rt5112_match,
    .pmic_get_dt_data = rt5112_get_dt_data,
    .pmic_seq_config = rt5112_seq_config,
    .pmic_register_attribute = rt5112_register_attribute,
    .pmic_check_exception = pmic_check_state_exception,
	.pmic_ldo_vote_cfg = rt5112_ldo_seq_config,
	.pmic_buck_vote_cfg = rt5112_buck_seq_config,
};

static struct hisi_pmic_ctrl_t rt5112_ctrl = {
    .pmic_i2c_client = &rt5112_i2c_client,
    .func_tbl = &rt5112_func_tbl,
    .hisi_pmic_mutex = &pmic_mut_rt5112,
    .pdata = (void*)&rt5112_pdata,
};

subsys_initcall(rt5112_module_init);
module_exit(rt5112_module_exit);
MODULE_DESCRIPTION("RT5112 PMIC");
MODULE_LICENSE("GPL v2");

//lint -restore
