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
#include "hw_pmic.h"
#include "../cam_log.h"
#include "../platform/sensor_commom.h"
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif
//lint -save -e502 -e31
// FAN53880 register define
#define REVERSION_ID (0x01)

// BUCK/LDO ENABLE
#define ENABLE_REG (0x09)
#define LDO1_ENABLE_SHIFT (0x00)
#define LDO2_ENABLE_SHIFT (0x01)
#define LDO3_ENABLE_SHIFT (0x02)
#define LDO4_ENABLE_SHIFT (0x03)
#define BUCK_ENABLE_SHIFT (0x04)
// LDO
#define LDO1_VOUT (0x04) // 0.6V-3.775V 25mV/step
#define LDO2_VOUT (0x05) // 0.6V-3.775V 25mV/step
#define LDO3_VOUT (0x06) // 0.6V-3.775V 25mV/step
#define LDO4_VOUT (0x07) // 0.6V-3.775V 25mV/step
// 600000 -> 3775000
// step 25000 (0.025)
#define FAN53880_LDO_VOLTAGE_MIN (800000)
#define FAN53880_LDO_VOLTAGE_MAX (3300000)
#define FAN53880_LDO_VOLTAGE_STEP (25000)
#define FAN53880_LDO_VOLTAGE_MIN_LEVEL (0x0F)

// Buck
#define BUCK_VOUT (0x02) // 0.6V-3.3V 12.5mV/step

#define FAN53880_BUCK_VOLTAGE_MIN (600000)
#define FAN53880_BUCK_VOLTAGE_MAX (3300000)
#define FAN53880_BUCK_VOLTAGE_STEP (12500)
#define FAN53880_BUCK_VOLTAGE_MIN_LEVEL (0x1F)
// Boost
#define BOOST_CTL (0x03)
#define BOOST_ENABLE (0x0A)
#define BOOST_VOUT (0x11) //3V-5.5V 25mV/step
#define FAN53880_VOUT_4V5 0x40
#define FAN53880_VOUT_5V0 0x54
#define FAN53880_VOUT_5V15 0x5A
#define FAN53880_VOUT_5V5 0x68
#define FAN53880_VOUT_5V3 0x60
#define FAN_BOOST_SUSPEND          0x20
#define FAN_BOOST_UVP_OVP_OCCUR    0xA0
#define FAN_BOOST_ERROR_OCCUR      1
#define FAN_MAX_LATCH_TIME         50000L
#define FAN_LATCH_CHECK_TIME       50

#define FAN53880_BOOST_VOLTAGE_MIN (3000000)
#define FAN53880_BOOST_VOLTAGE_MAX (5700000)
#define FAN53880_BOOST_VOLTAGE_STEP (25000)

// satus reg
#define FAN53880_PMU_STATUS_REG (0x1A)
#define FAN53880_OVP_STATUS_REG (0x11)
#define FAN53880_OCP_STATUS_REG (0x12)
#define FAN53880_UVP_STATUS_REG (0x13)

// PIN
#define FAN53880_PIN_ENABLE (1)
#define FAN53880_PIN_DISABLE (0)
#define FAN53880_POWERON_MASK   (0x80)
#define FAN53880_POWEROFF_MASK (0x7F)
#define FAN53880_MASK_BOOST_5V (0x80)
#define FAN53880_MASK_UVP_INT (0x20)
#define FAN53880_MASK_BST_IPK_INT (0x80)

#define PMIC_DISCHARGE               0x0F
#define CONFIG_OPEN_FAKE_LOAD        0x40
#define CONFIG_CLOSE_FAKE_LOAD       0xBF
#define CONFIG_FORCE_PWM_MODE        0x80
#define CONFIG_CLOSE_PWM_MODE        0x7F
//LDO2
#define FAN53880_LDO2_CTRL 1

// Private data struct
struct fan53880_private_data_t {
    unsigned int pin;
    unsigned int voltage[VOUT_MAX];
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
    {ENABLE_REG, LDO1_VOUT},
    {ENABLE_REG, LDO2_VOUT},
    {ENABLE_REG, LDO3_VOUT},
    {ENABLE_REG, LDO4_VOUT},
    {0, 0,}, //stub not support LDO5
    {ENABLE_REG, BUCK_VOUT},
};

// static var
static struct fan53880_private_data_t fan53880_pdata;
static struct i2c_driver fan53880_i2c_driver;
static struct hisi_pmic_ctrl_t fan53880_ctrl;
extern struct dsm_client *client_pmic;
static void pmic_fault_err_work(struct work_struct *work);
static int boost_enabled;

DEFINE_HISI_PMIC_MUTEX(fan53880);

static int fan53880_set_boost_load_enable(struct hisi_pmic_ctrl_t *pmic_ctrl, int state)
{
	struct hisi_pmic_i2c_client *i2c_client = NULL;
	struct hisi_pmic_i2c_fn_t *i2c_func = NULL;
	u8 boost_load_enable = 0;
	int ret;

	cam_err("%s enter:", __func__);
	if (!pmic_ctrl || !pmic_ctrl->pmic_i2c_client || !pmic_ctrl->pdata ||
		!pmic_ctrl->pmic_i2c_client->i2c_func_tbl) {
		cam_err("%s pmic_ctrl is NULL", __func__);
		return -1;
	}

	if (state != POWER_OFF && state != POWER_ON) {
		cam_err("%s state [%u] error", __func__, state);
		return -1;
	}

	i2c_client = pmic_ctrl->pmic_i2c_client;
	i2c_func = pmic_ctrl->pmic_i2c_client->i2c_func_tbl;

	ret = i2c_func->i2c_read(i2c_client, PMIC_DISCHARGE, &boost_load_enable);
	if (ret < 0)
		return ret;

	if (state == 1)
		boost_load_enable = boost_load_enable | CONFIG_OPEN_FAKE_LOAD;
	else
		boost_load_enable = boost_load_enable & CONFIG_CLOSE_FAKE_LOAD;
	cam_info("%s, boost_load_enable%u\n", __func__, boost_load_enable);
	ret = i2c_func->i2c_write(i2c_client, PMIC_DISCHARGE, boost_load_enable);

	return ret;
}

static int fan53880_force_pwm_mode(struct hisi_pmic_ctrl_t *pmic_ctrl, int state)
{
	struct hisi_pmic_i2c_client *i2c_client = NULL;
	struct hisi_pmic_i2c_fn_t *i2c_func = NULL;
	u8 enable_reg = 0;
	int ret;

	cam_err("%s enter:", __func__);
	if (!pmic_ctrl || !pmic_ctrl->pmic_i2c_client || !pmic_ctrl->pdata ||
		!pmic_ctrl->pmic_i2c_client->i2c_func_tbl) {
		cam_err("%s pmic_ctrl is NULL", __func__);
		return -1;
	}

	if (state != POWER_OFF && state != POWER_ON) {
		cam_err("%s state [%u] error", __func__, state);
		return -1;
	}

	i2c_client = pmic_ctrl->pmic_i2c_client;
	i2c_func = pmic_ctrl->pmic_i2c_client->i2c_func_tbl;

	ret = i2c_func->i2c_read(i2c_client, ENABLE_REG, &enable_reg);
	if (ret < 0)
		return ret;

	if (state == 1)
		enable_reg = enable_reg | CONFIG_FORCE_PWM_MODE;
	else
		enable_reg = enable_reg & CONFIG_CLOSE_PWM_MODE;
	cam_info("%s, enable_reg =%u\n", __func__, enable_reg);
	ret = i2c_func->i2c_write(i2c_client, ENABLE_REG, enable_reg);

	return ret;
}

static u32 calc_voltage_level(u32 voltage, u32 max, u32 min, u32 step)
{
    if (voltage > max || voltage < min)
    {
        return min;
    }

    return (voltage - min) / step;
}

static int fan53880_boost_seq_config(struct hisi_pmic_ctrl_t *pmic_ctrl,
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
        chx_enable = chx_enable | 0x01;
    }
    else
    {
        chx_enable = 0;
    }

    i2c_func->i2c_write(i2c_client, chx_enable_reg, chx_enable);
    cam_err("%s, power state[%d], chx_reg[%u],chx_enable[%u]", __func__,
            state, chx_enable_reg, chx_enable);

    /*vbus need 700us raise to stable 5v*/
    if (state == POWER_ON) {
        msleep(1);
    }
    boost_enabled = state;
    return ret;
}

static int fan53880_buck_seq_config(struct hisi_pmic_ctrl_t *pmic_ctrl,
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

    if (NULL == pmic_ctrl) {
        cam_err("%s pmic_ctrl is NULL.", __func__);
        return -1;
    }

    if (seq_index != VOUT_BUCK_1) {
        cam_err("%s seq_index error, seq_index[%u]", __func__, seq_index);
        return -1;
    }

    if (voltage > FAN53880_BUCK_VOLTAGE_MAX \
        || voltage < FAN53880_BUCK_VOLTAGE_MIN) {
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
                                            FAN53880_BUCK_VOLTAGE_MAX,
                                            FAN53880_BUCK_VOLTAGE_MIN,
                                            FAN53880_BUCK_VOLTAGE_STEP);
        voltage_level += FAN53880_BUCK_VOLTAGE_MIN_LEVEL;
        // set voltage
        i2c_func->i2c_write(i2c_client, voltage_reg, voltage_level);
        chx_enable = chx_enable | (0x1 << BUCK_ENABLE_SHIFT);
    }
    else
    {
        chx_enable = chx_enable & (~(0x1 << BUCK_ENABLE_SHIFT));
    }

    i2c_func->i2c_write(i2c_client, chx_enable_reg, chx_enable);
    cam_info("%s, power state[%d], chx_reg[%u], reg[%u], vol[%u],vol_level[%u]", __func__,
            state, chx_enable_reg, voltage_reg, voltage, voltage_level);

    return ret;
}
// FAN53880 function
static int fan53880_ldo_seq_config(struct hisi_pmic_ctrl_t *pmic_ctrl,
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

    cam_debug("%s enter.", __func__);

    if (NULL == pmic_ctrl) {
        cam_err("%s pmic_ctrl is NULL.", __func__);
        return -1;
    }

    if (seq_index < VOUT_LDO_1 || seq_index > VOUT_LDO_4) {
        cam_err("%s seq_index error, seq_index[%u]", __func__, seq_index);
        return -1;
    }

    if (voltage > FAN53880_LDO_VOLTAGE_MAX \
        || voltage < FAN53880_LDO_VOLTAGE_MIN) {
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
                                            FAN53880_LDO_VOLTAGE_MAX,
                                            FAN53880_LDO_VOLTAGE_MIN,
                                            FAN53880_LDO_VOLTAGE_STEP);
        voltage_level += FAN53880_LDO_VOLTAGE_MIN_LEVEL;
        // set voltage
        i2c_func->i2c_write(i2c_client, voltage_reg, voltage_level);
        chx_enable = chx_enable | (0x01 << seq_index);
    }
    else
    {
        chx_enable = chx_enable & (~(0x01 << seq_index));
    }

    i2c_func->i2c_write(i2c_client, chx_enable_reg, chx_enable);
    cam_info("%s, power state[%d], chx_reg[%u], reg[%u], vol[%u],vol_level[%u]", __func__,
            state, chx_enable_reg, voltage_reg, voltage, voltage_level);

    return ret;
}

static int fan53880_seq_config(struct hisi_pmic_ctrl_t *pmic_ctrl,
                              pmic_seq_index_t seq_index,
                              u32 voltage,
                              int state)
{
	int ret = 0;
	struct fan53880_private_data_t *pdata = NULL;

	pdata = (struct fan53880_private_data_t *)pmic_ctrl->pdata;
	if (pdata == NULL) {
		cam_err("%s pdata error", __func__);
		return -1;
	}

	mutex_lock(&pmic_mut_fan53880);
	if (seq_index == VOUT_BOOST) {
		ret = fan53880_boost_seq_config(pmic_ctrl, seq_index, voltage, state);
	} else if (seq_index == VOUT_BOOST_EN) {
		ret = hisi_pmic_gpio_boost_enable(pmic_ctrl, state);
	} else if (seq_index < VOUT_LDO_5) {
		if (pdata->ldo_vote == 1) {
			cam_debug("%s, vote set ldo", __func__);
			ret = hisi_pmic_vote_cfg(pmic_ctrl, seq_index, voltage,
				state);
		} else if ((pdata->ldo2_ctrl == FAN53880_LDO2_CTRL) &&
			(seq_index == VOUT_LDO_2)) {
			ret = hisi_pmic_vote_cfg(pmic_ctrl, VOUT_LDO_2, voltage,
				state);
		} else {
			ret = fan53880_ldo_seq_config(pmic_ctrl, seq_index, voltage, state);
		}
	} else {
		if (pdata->buck_vote == 1)
			ret = hisi_pmic_vote_cfg(pmic_ctrl, seq_index, voltage,
				state);
		else
			ret = fan53880_buck_seq_config(pmic_ctrl, seq_index,
				voltage, state);
	}

	mutex_unlock(&pmic_mut_fan53880);
	return ret;
}

static int fan53880_match(struct hisi_pmic_ctrl_t *pmic_ctrl)
{
    cam_debug("%s enter.", __func__);
    return 0;
}

static int fan53880_get_dt_data(struct hisi_pmic_ctrl_t *pmic_ctrl)
{
    struct fan53880_private_data_t *pdata = NULL;
    struct device_node *dev_node = NULL;
    int rc = 0;

    if (NULL == pmic_ctrl) {
        cam_err("%s pmic ctrl is NULL", __func__);
        return -1;
    }

    pdata = (struct fan53880_private_data_t *)pmic_ctrl->pdata;
    dev_node = pmic_ctrl->dev->of_node;

    // PIN Enable gpio
    rc = of_property_read_u32(dev_node, "hisi,pmic-pin", &pdata->pin);
    cam_info("%s huawei,pmic-pin %d", __func__, pdata->pin);
    if (rc  < 0) {
        cam_err("%s, failed %d\n", __func__, __LINE__);
        goto fail;
    }

	// ldo2_ctrl
	rc = of_property_read_u32(dev_node, "hisi,pmic_ldo2_ctrl", &pdata->ldo2_ctrl);
	if (rc < 0) {
		pdata->ldo2_ctrl = 0;
		cam_info("%s, cannot get buck config, set to default ldo2_ctrl", __func__);
	}
	cam_info("%s huawei,pmic_ldo2_ctrl %d", __func__, pdata->ldo2_ctrl);

	rc = of_property_read_u32(dev_node, "hisi,shutdown_4V5",
		&pdata->shutdown_4v5);
	if (rc < 0) {
		pdata->shutdown_4v5 = 0;
		cam_info("%s, cannot get shutdown 4v5 set, set 0", __func__);
	}
	cam_info("get shutdown_4V5 val = %d", pdata->shutdown_4v5);
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
	rc = of_property_read_u32(dev_node, "hisi,ctl_otg", &pdata->ctl_otg);
	if (rc < 0) {
		pdata->ctl_otg = 1;
		cam_info("%s, cannot get ctl_otg, set 1", __func__);
	}
	cam_info("%s ctl_otg %u", __func__, pdata->ctl_otg);
	rc = of_property_read_u32(dev_node, "hisi,boost_5v3", &pdata->boost_5v3);
	if (rc < 0) {
		pdata->boost_5v3 = 0;
		cam_info("%s, cannot get ctl_otg, set 1", __func__);
	}
	cam_info("%s boost_5v3 %u", __func__, pdata->boost_5v3);
	rc = of_property_read_u32(dev_node, "hisi,boost_5v15", &pdata->boost_5v15);
	if (rc < 0) {
		pdata->boost_5v15 = 0;
		cam_info("%s, cannot get boost 5v15 set, set 0", __func__);
	}
	cam_info("%s boost_5v15 %u", __func__, pdata->boost_5v15);
    return 0;

fail:
    return -EIO;
}

static int fan53880_on(struct hisi_pmic_ctrl_t *pmic_ctrl, void *data)
{
    // check Error registor
    return 0;
}

static int fan53880_off(struct hisi_pmic_ctrl_t *pmic_ctrl)
{
    // Never power off pmic when SOC is running;
    return 0;
}

static int fan53880_init(struct hisi_pmic_ctrl_t *pmic_ctrl)
{
    struct hisi_pmic_i2c_client *i2c_client = NULL;
    struct hisi_pmic_i2c_fn_t *i2c_func = NULL;
    struct fan53880_private_data_t *pdata = NULL;
    u8 chip_id = 0;
    int ret = 0;

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

    ret = hisi_pmic_setup_intr(pmic_ctrl);
    if (ret < 0) {
        cam_err("%s setup interrupt failed", __func__);
        goto err;
    }

    pdata = (struct fan53880_private_data_t *)pmic_ctrl->pdata;

    cam_debug("%s PMIC_POWER_CTRL = %d", __func__,
            pdata->pin);

    ret = gpio_request(pdata->pin, "pmic-power-ctrl");
    if (ret < 0) {
        cam_err("%s failed to request pmic-power-ctrl pin. ret = %d.", __func__, ret);
        goto req_failed;
    }

    gpio_direction_output(pdata->pin, FAN53880_PIN_ENABLE);

    msleep(1);

    i2c_client = pmic_ctrl->pmic_i2c_client;
    i2c_func = pmic_ctrl->pmic_i2c_client->i2c_func_tbl;

    ret = i2c_func->i2c_read(i2c_client, REVERSION_ID, &chip_id);
    if (ret < 0) {
        cam_err("%s: read CHIP ID failed, ret = %d ", __func__, ret);
        goto err2;
    }
    cam_debug("%s chip id=%d", __func__, chip_id);
	if (pdata->boost_5v3 == 1) // support boost 5v3
		ret = i2c_func->i2c_write(i2c_client, BOOST_CTL,
			FAN53880_VOUT_5V3);
	else if (pdata->boost_5v15 == 1) // support boost 5v15
		ret = i2c_func->i2c_write(i2c_client, BOOST_CTL,
			FAN53880_VOUT_5V15);
	else
		ret = i2c_func->i2c_write(i2c_client, BOOST_CTL,
			FAN53880_VOUT_5V0);
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

static int fan53880_exit(struct hisi_pmic_ctrl_t *pmic_ctrl)
{
    struct fan53880_private_data_t *pdata = NULL;

    cam_debug("%s enter.", __func__);

    if (NULL == pmic_ctrl) {
        cam_err("%s pmic_ctrl is NULL.", __func__);
        return -1;
    }

    pdata = (struct fan53880_private_data_t *)pmic_ctrl->pdata;
    gpio_free(pdata->pin);
    pmic_ctrl->pctrl = devm_pinctrl_get_select(pmic_ctrl->dev,
        PINCTRL_STATE_IDLE);
    hisi_pmic_release_intr(pmic_ctrl);
    return 0;
}

static int fan53880_register_attribute(struct hisi_pmic_ctrl_t *pmic_ctrl, struct device *dev)
{
    return 0;
}

static void pmic_boost_reset(struct hisi_pmic_ctrl_t *pmic_ctrl)
{
	fan53880_boost_seq_config(pmic_ctrl, VOUT_BOOST, 0, POWER_OFF);
	udelay(1000);
	fan53880_boost_seq_config(pmic_ctrl, VOUT_BOOST, 0, POWER_ON);
}

static void fan53880_pmic_reset(struct hisi_pmic_ctrl_t *pmic_ctrl)
{
	struct hisi_pmic_i2c_client *i2c_client = NULL;
	struct hisi_pmic_i2c_fn_t *i2c_func = NULL;
	u8 pmu_status = 0;

	if (!pmic_ctrl || !pmic_ctrl->pmic_i2c_client || !pmic_ctrl->pdata ||
		!pmic_ctrl->pmic_i2c_client->i2c_func_tbl) {
		cam_err("%s pmic_ctrl is NULL", __func__);
		return;
	}

	i2c_client = pmic_ctrl->pmic_i2c_client;
	i2c_func = pmic_ctrl->pmic_i2c_client->i2c_func_tbl;
	// read pmic suspend status
	i2c_func->i2c_read(i2c_client, FAN53880_PMU_STATUS_REG, &pmu_status);
	cam_info("%s pmu-0x%x, boost-%d", __func__, pmu_status, boost_enabled);

	if ((pmu_status & FAN_BOOST_SUSPEND) != FAN_BOOST_SUSPEND) {
		cam_info("%s, boost fault recovery", __func__);
		return;
	}

	if (boost_enabled == POWER_ON) {
		// reset boost en
		pmic_ctl_otg_onoff(FAN53880_PIN_DISABLE);
		cam_info("%s reset boost when boost still exist", __func__);
		fan53880_boost_seq_config(pmic_ctrl, VOUT_BOOST, 0, POWER_OFF);
		udelay(1000);
		fan53880_boost_seq_config(pmic_ctrl, VOUT_BOOST, 0, POWER_ON);
	}
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

	fan53880_pmic_reset(pmic_ctrl);
}

static int pmic_check_state_exception(struct hisi_pmic_ctrl_t *pmic_ctrl)
{
    struct hisi_pmic_i2c_client *i2c_client = NULL;
    struct hisi_pmic_i2c_fn_t *i2c_func = NULL;
    struct fan53880_private_data_t *pdata = NULL;
	static struct irq_err_monitor irq_err;
	u8 reg_value = 0;
	u8 pmu_status = 0;
	u8 boost_err_occur = 0;

    if (NULL == pmic_ctrl) {
        cam_err("%s pmic_ctrl is NULL.", __func__);
        return -1;
    }

    i2c_client = pmic_ctrl->pmic_i2c_client;
    i2c_func = pmic_ctrl->pmic_i2c_client->i2c_func_tbl;
    pdata = (struct fan53880_private_data_t *)pmic_ctrl->pdata;
    cam_info("%s pmic_ctrl->pdata pin = %d.",__func__,pdata->pin);

    // PMU_STAT
    i2c_func->i2c_read(i2c_client, FAN53880_PMU_STATUS_REG, &pmu_status);

    //OVP
    i2c_func->i2c_read(i2c_client, FAN53880_OVP_STATUS_REG, &reg_value);
    if (reg_value != 0) {
        REPORT_DSM_STATUS(client_pmic, FAN53880_OVP_STATUS_REG,
                    reg_value, FAN53880_PMU_STATUS_REG,
                    pmu_status, DSM_CAMPMIC_OVER_CURRENT_ERROR_NO,
                    "FAN53880 OVP");
    }

	// OCP close OTG MOS
    i2c_func->i2c_read(i2c_client, FAN53880_OCP_STATUS_REG, &reg_value);
	if (reg_value != 0) {
		REPORT_DSM_STATUS(client_pmic, FAN53880_OCP_STATUS_REG,
			reg_value, FAN53880_PMU_STATUS_REG, pmu_status,
			DSM_CAMPMIC_OVER_CURRENT_ERROR_NO, "FAN53880 OCP");
		if((reg_value & FAN53880_MASK_UVP_INT) && pdata->ctl_otg)
			pmic_ctl_otg_onoff(FAN53880_PIN_DISABLE);
	}
	if ((reg_value & FAN_BOOST_UVP_OVP_OCCUR) > 0)
		boost_err_occur = FAN_BOOST_ERROR_OCCUR;

	// UVP close OTG MOS
	i2c_func->i2c_read(i2c_client, FAN53880_UVP_STATUS_REG, &reg_value);
	if (reg_value != 0) {
		REPORT_DSM_STATUS(client_pmic, FAN53880_UVP_STATUS_REG,
			reg_value, FAN53880_PMU_STATUS_REG, pmu_status,
			DSM_CAMPMIC_UNDER_VOLTAGE_ERROR_NO, "FAN53880 UVP");
		if ((reg_value & FAN53880_MASK_BST_IPK_INT) && pdata->ctl_otg)
			pmic_ctl_otg_onoff(FAN53880_PIN_DISABLE);
	}
	if (reg_value & FAN53880_MASK_BST_IPK_INT)
		boost_err_occur = FAN_BOOST_ERROR_OCCUR;
	if (boost_err_occur == FAN_BOOST_ERROR_OCCUR)
		pmic_boost_reset(pmic_ctrl);
	pmic_fault_reset_check(pmic_ctrl, &irq_err, FAN_MAX_LATCH_TIME,
		FAN_LATCH_CHECK_TIME);

	// reset fan53889 ENABLE
	if (pdata->reset != 0) {
		gpio_set_value(pdata->pin, FAN53880_PIN_DISABLE);
		udelay(1000);
		gpio_set_value(pdata->pin, FAN53880_PIN_ENABLE);
	} else {
		cam_info("%s pmic need not reset", __func__);
	}
    return 0;
}

static void fan53880_shutdown(struct i2c_client *client)
{
	struct hisi_pmic_i2c_client *fan_i2c_client = NULL;
	struct hisi_pmic_ctrl_t *fan_shut_pmic_ctrl = NULL;
	struct hisi_pmic_i2c_fn_t *i2c_func = NULL;
	struct fan53880_private_data_t *pdata = NULL;

	cam_warn("%s enter", __func__);
	fan_shut_pmic_ctrl = hisi_get_pmic_ctrl();
	if (!fan_shut_pmic_ctrl || !fan_shut_pmic_ctrl->pmic_i2c_client ||
		!fan_shut_pmic_ctrl->pmic_i2c_client->i2c_func_tbl ||
		!fan_shut_pmic_ctrl->pdata)
		return;
	pdata = (struct fan53880_private_data_t *)fan_shut_pmic_ctrl->pdata;
	if (pdata->shutdown_4v5 == 0) {
		cam_warn("%s not support shut down to 4v5", __func__);
		return;
	}

	fan_i2c_client = fan_shut_pmic_ctrl->pmic_i2c_client;
	i2c_func = fan_shut_pmic_ctrl->pmic_i2c_client->i2c_func_tbl;
	i2c_func->i2c_write(fan_i2c_client, BOOST_CTL, FAN53880_VOUT_4V5);
	cam_warn("set to 4V5");
}

static int fan53880_remove(struct i2c_client *client)
{
    cam_debug("%s enter.", __func__);

    client->adapter = NULL;
    return 0;
}

// lint -save -e31
static const struct i2c_device_id fan53880_id[] = {
    {"fan53880", (unsigned long)&fan53880_ctrl},
    {}
};

static const struct of_device_id fan53880_dt_match[] = {
    {.compatible = "hisi,fan53880"},
    {}
};

MODULE_DEVICE_TABLE(of, fan53880_dt_match);

static struct i2c_driver fan53880_i2c_driver = {
    .probe    = hisi_pmic_i2c_probe,
    .remove = fan53880_remove,
	.shutdown = fan53880_shutdown,
    .id_table   = fan53880_id,
    .driver = {
        .name = "fan53880",
        .of_match_table = fan53880_dt_match,
    },
};

static int __init fan53880_module_init(void)
{
    cam_info("%s enter.\n", __func__);
    return i2c_add_driver(&fan53880_i2c_driver);
}

static void __exit fan53880_module_exit(void)
{
    cam_info("%s enter.", __func__);
    i2c_del_driver(&fan53880_i2c_driver);
    return;
}

static struct hisi_pmic_i2c_client fan53880_i2c_client;

static struct hisi_pmic_fn_t fan53880_func_tbl = {
    .pmic_config = hisi_pmic_config,
    .pmic_init = fan53880_init,
    .pmic_exit = fan53880_exit,
    .pmic_on = fan53880_on,
    .pmic_off = fan53880_off,
    .pmic_match = fan53880_match,
    .pmic_get_dt_data = fan53880_get_dt_data,
    .pmic_seq_config = fan53880_seq_config,
    .pmic_register_attribute = fan53880_register_attribute,
    .pmic_check_exception = pmic_check_state_exception,
	.pmic_ldo_vote_cfg = fan53880_ldo_seq_config,
	.pmic_buck_vote_cfg = fan53880_buck_seq_config,
	.pmic_set_boost_load_enable = fan53880_set_boost_load_enable,
	.pmic_force_pwm_mode = fan53880_force_pwm_mode,
};

static struct hisi_pmic_ctrl_t fan53880_ctrl = {
    .pmic_i2c_client = &fan53880_i2c_client,
    .func_tbl = &fan53880_func_tbl,
    .hisi_pmic_mutex = &pmic_mut_fan53880,
    .pdata = (void*)&fan53880_pdata,
};

subsys_initcall(fan53880_module_init);
module_exit(fan53880_module_exit);
MODULE_DESCRIPTION("FAN53880 PMIC");
MODULE_LICENSE("GPL v2");

//lint -restore
