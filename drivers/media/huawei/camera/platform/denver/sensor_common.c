/*
 * sensor_commom.c
 * Description:  common driver code for sensor all
 * Copyright (c) Huawei Technologies Co., Ltd. 2011-2020. All rights reserved.
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
#include <linux/clk.h>
#include <linux/thermal.h>
#include "sensor_commom.h"
#include "hw_pmic.h"
#ifdef CONFIG_HUAWEI_CAMERA_BUCK
#include "hw_buck.h"
#endif
#include <chipset_common/hwpower/hardware_ic/boost_5v.h>
#include "../../clt/hisi_clt_flag.h"
#include <huawei_platform/sensor/hw_comm_pmic.h>
#include <securec.h>

static int g_is_fpga; // default is no fpga
static atomic_t volatile g_powered = ATOMIC_INIT(0);
extern unsigned int *lpm3;
static unsigned int g_flag;
#define HIGH_TEMP 70000 /* 70 C */
#define LOW_TEMP (-100000) /* -100 C */
struct power_seq_type_tab {
	const char *seq_name;
	enum sensor_power_seq_type_t seq_type;
};

static gpio_vote_t g_gpio_vote;

#define LPM3_REGISTER_ENABLE 1
#define LPM3_REGISTER_DISABLE 0

static struct power_seq_type_tab g_seq_type_tab[] = {
	{ "sensor_suspend", SENSOR_SUSPEND },
	{ "sensor_suspend2", SENSOR_SUSPEND2 },
	{ "sensor_pwdn", SENSOR_PWDN },
	{ "sensor_rst", SENSOR_RST },
	{ "sensor_rst2", SENSOR_RST2 },
	{ "sensor_vcm_avdd", SENSOR_VCM_AVDD },
	{ "sensor_vcm_avdd2", SENSOR_VCM_AVDD2 },
	{ "sensor_vcm_pwdn", SENSOR_VCM_PWDN },
	{ "sensor_avdd", SENSOR_AVDD },
	{ "sensor_avdd0", SENSOR_AVDD0 },
	{ "sensor_avdd1", SENSOR_AVDD1 },
	{ "sensor_avdd2", SENSOR_AVDD2 },
	{ "sensor_misp_vdd", SENSOR_MISP_VDD },
	{ "sensor_avdd1_en", SENSOR_AVDD1_EN },
	{ "sensor_avdd2_en", SENSOR_AVDD2_EN },
	{ "sensor_dvdd", SENSOR_DVDD },
	{ "sensor_dvdd2", SENSOR_DVDD2 },
	{ "sensor_ois_drv", SENSOR_OIS_DRV },
	{ "sensor_dvdd0_en", SENSOR_DVDD0_EN },
	{ "sensor_dvdd1_en", SENSOR_DVDD1_EN },
	{ "sensor_iovdd", SENSOR_IOVDD },
	{ "sensor_iovdd_en", SENSOR_IOVDD_EN },
	{ "sensor_mclk", SENSOR_MCLK },
	{ "sensor_i2c", SENSOR_I2C },
	{ "sensor_ldo_en", SENSOR_LDO_EN },
	{ "sensor_mispdcdc_en", SENSOR_MISPDCDC_EN },
	{ "sensor_check_level", SENSOR_CHECK_LEVEL },
	{ "sensor_ois", SENSOR_OIS },
	{ "sensor_ois2", SENSOR_OIS2 },
	{ "sensor_pmic", SENSOR_PMIC },
	{ "sensor_pmic2", SENSOR_PMIC2 },
	{ "sensor_rxdphy_clk", SENSOR_RXDPHY_CLK },
	{ "sensor_cs", SENSOR_CS },
	{ "sensor_mipi_sw", SENSOR_MIPI_SW },
	{ "laser_xshut", SENSOR_LASER_XSHUT },
	{ "sensor_buck", SENSOR_BUCK },
	{ "sensor_boot_5v", SENSOR_BOOT_5V },
};

struct ldo_config_tab {
	enum sensor_power_seq_type_t seq_type;
	ldo_index_t ldo_index;
	int ldo_ignore_exceptions;
};

static struct ldo_config_tab g_ldo_config_type_tab[] = {
	{ SENSOR_DVDD, LDO_DVDD, 0 },
	{ SENSOR_DVDD2, LDO_DVDD2, 1 },
	{ SENSOR_OIS_DRV, LDO_OISDRV, 0 },
	{ SENSOR_IOVDD, LDO_IOVDD, 0 },
	{ SENSOR_AVDD, LDO_AVDD, 0 },
	{ SENSOR_AVDD2, LDO_AVDD2, 1 },
	{ SENSOR_VCM_AVDD, LDO_VCM, 0 },
	{ SENSOR_VCM_AVDD2, LDO_VCM2, 1 },
	{ SENSOR_AVDD0, LDO_AVDD0, 0 },
	{ SENSOR_AVDD1, LDO_AVDD1, 0 },
	{ SENSOR_MISP_VDD, LDO_MISP, 0 }
};

struct gpio_config_tab {
	enum sensor_power_seq_type_t seq_type;
	gpio_t gpio_index;
	int gpio_ignore_exceptions;
};

static struct gpio_config_tab g_gpio_config_type_tab[] = {
	{ SENSOR_LDO_EN, LDO_EN, 0 },
	{ SENSOR_AVDD1_EN, AVDD1_EN, 0 },
	{ SENSOR_DVDD0_EN, DVDD0_EN, 0 },
	{ SENSOR_DVDD1_EN, DVDD1_EN, 0 },
	{ SENSOR_AFVDD_EN, AFVDD_EN, 0 },
	{ SENSOR_IOVDD_EN, IOVDD_EN, 0 },
	{ SENSOR_MISPDCDC_EN, MISPDCDC_EN, 0 },
	{ SENSOR_CHECK_LEVEL, FSIN, 0 },
	{ SENSOR_RST, RESETB, 0 },
	{ SENSOR_PWDN, PWDN, 0 },
	{ SENSOR_VCM_PWDN, VCM, 0 },
	{ SENSOR_SUSPEND, SUSPEND, 0 },
	{ SENSOR_SUSPEND2, SUSPEND2, 0 },
	{ SENSOR_RST2, RESETB2, 0 },
	{ SENSOR_OIS, OIS, 0 },
	{ SENSOR_OIS2, OIS2, 1 },
	{ SENSOR_AVDD2_EN, AVDD2_EN, 0 },
	{ SENSOR_MIPI_SW, MIPI_SW, 0 }
};

int hw_sensor_get_thermal(const char *name, int *temp)
{
	struct thermal_zone_device *tz = NULL;
	int temperature = 0;
	int rc = 0;

	if (!temp)
		return -1;

	tz = thermal_zone_get_zone_by_name(name);
	if (IS_ERR(tz)) {
		cam_err("Error getting sensor thermal zone %s, not yet ready\n",
			name);
		rc = -1;
	} else {
		int ret;

		ret = tz->ops->get_temp(tz, &temperature);
		if (ret) {
			cam_err("Error reading temperature for gpu thermal zone: %d\n",
				ret);
			rc = -1;
		}
	}

	if (((temperature < LOW_TEMP) ||
		(temperature > HIGH_TEMP)) &&
		(g_flag == 0)) {
		g_flag = 1;
		cam_err("Abnormal temperature = %d\n", temperature);
	} else {
		g_flag = 0;
	}
	*temp = temperature;
	return rc;
}

static int deal_fsnclk(struct device *dev, struct clk *s_ctrl_isp_snclk,
	unsigned int clk)
{
	int ret;

	if (IS_ERR_OR_NULL(s_ctrl_isp_snclk)) {
		dev_err(dev, "could not get snclk\n");
		ret = PTR_ERR(s_ctrl_isp_snclk);
		return ret;
	}

	ret = clk_set_rate(s_ctrl_isp_snclk, clk);
	if (ret < 0) {
		dev_err(dev, "failed set_rate snclk rate\n");
		return ret;
	}

	ret = clk_prepare_enable(s_ctrl_isp_snclk);
	if (ret) {
		dev_err(dev, "cloud not enalbe clk_isp_snclk\n");
		return ret;
	}

	return ret;
}

static int mclk_config(sensor_t *s_ctrl, unsigned int id,
	unsigned int clk, int on)
{
	int ret = 0;
	struct device *dev = NULL;
	bool fsnclk0 = (id == HWSENSOR_POSITION_REAR ||
		id == HWSENSOR_POSITION_FORE_SECOND);
	bool fsnclk1 = (id == HWSENSOR_POSITION_FORE ||
		id == HWSENSOR_POSITION_REAR_FORTH);
	bool fSnclk2 = (id == HWSENSOR_POSITION_SUBREAR ||
		id == HWSENSOR_POSITION_SUBFORE ||
		id == HWSENSOR_POSITION_REAR_THIRD);

	if (!s_ctrl) {
		cam_err("%s invalid parameter\n", __func__);
		return -1;
	}
	dev = s_ctrl->dev;

	cam_info("%s enter.id = %u, clk = %u, on = %d", __func__, id, clk, on);

	/* clk_isp_snclk max value is 48000000 */
	if (clk > 48000000) {
		cam_err("input: id[%d], clk[%d] is error\n", id, clk);
		return -1;
	}

	if (!on) {
		if (fsnclk0 && s_ctrl->isp_snclk0) {
			clk_disable_unprepare(s_ctrl->isp_snclk0);
			cam_info("clk_disable_unprepare snclk0\n");
		} else if (fsnclk1 && s_ctrl->isp_snclk1) {
			clk_disable_unprepare(s_ctrl->isp_snclk1);
			cam_info("clk_disable_unprepare snclk1\n");
		} else if ((fSnclk2) && s_ctrl->isp_snclk2) {
			clk_disable_unprepare(s_ctrl->isp_snclk2);
			cam_info("clk_disable_unprepare snclk2\n");
		}
		return 0;
	}

	if (fsnclk0) {
		s_ctrl->isp_snclk0 = devm_clk_get(dev, "clk_isp_snclk0");
		ret = deal_fsnclk(dev, s_ctrl->isp_snclk0, clk);
	} else if (fsnclk1) {
		s_ctrl->isp_snclk1 = devm_clk_get(dev, "clk_isp_snclk1");
		ret = deal_fsnclk(dev, s_ctrl->isp_snclk1, clk);
	} else if (fSnclk2) {
		s_ctrl->isp_snclk2 = devm_clk_get(dev, "clk_isp_snclk2");
		ret = deal_fsnclk(dev, s_ctrl->isp_snclk2, clk);
	}
	return ret;
}

static int hw_mclk_config(sensor_t *s_ctrl,
	struct sensor_power_setting *power_setting, int state)
{
	int sensor_index;

	cam_debug("%s enter state:%d", __func__, state);

	if (hw_is_fpga_board())
		return 0;

	if (power_setting->sensor_index != SENSOR_INDEX_INVALID)
		sensor_index = power_setting->sensor_index;
	else
		sensor_index = s_ctrl->board_info->sensor_index;

	mclk_config(s_ctrl, sensor_index,
		s_ctrl->board_info->mclk, state);

	if (power_setting->delay != 0)
		hw_camdrv_msleep(power_setting->delay);

	return 0;
}

static int hw_sensor_gpio_operate(unsigned int gpio, int output)
{
	int rc = gpio_request(gpio, NULL);
	if (rc < 0) {
		cam_err("%s failed to request gpio[%u], rc %d",
			__func__, gpio, rc);
		return rc;
	}

	rc = gpio_direction_output(gpio, output);
	if (rc < 0)
		cam_err("%s failed to control gpio %u, rc %d",
		__func__, gpio, rc);
	else
		cam_info("%s config gpio %u output %d", __func__, gpio, output);

	gpio_free(gpio);

	return rc;
}

static int hw_sensor_gpio_vote(int idx, int output)
{
	int rc = -1;

	if (output) {
		rc = hw_sensor_gpio_operate(g_gpio_vote.gpio_pin[idx], output);
		if (rc < 0) {
			cam_err("%s idx %d gpio %u count %d, vote high failed",
				__func__, idx, g_gpio_vote.gpio_pin[idx],
				g_gpio_vote.ref_count[idx]);
			return rc;
		}

		g_gpio_vote.ref_count[idx]++;
		cam_info("%s idx %d gpio %u vote count++ %d, vote high success",
			__func__, idx, g_gpio_vote.gpio_pin[idx],
			g_gpio_vote.ref_count[idx]);
		return 0;
	}

	if (g_gpio_vote.ref_count[idx] > 0) {
		/* gpio output low when reference count is 1 */
		if (g_gpio_vote.ref_count[idx] == 1) {
			rc = hw_sensor_gpio_operate(g_gpio_vote.gpio_pin[idx],
				output);
			if (rc < 0) {
				cam_err("%s idx %d gpio %u count %d, vote low failed",
					__func__, idx,
					g_gpio_vote.gpio_pin[idx],
					g_gpio_vote.ref_count[idx]);
				return rc;
			}
		}

		g_gpio_vote.ref_count[idx]--;
		cam_info("%s idx %d gpio %u vote count-- %d, vote low success",
			__func__, idx, g_gpio_vote.gpio_pin[idx],
			g_gpio_vote.ref_count[idx]);
	}

	return 0;
}

int hw_sensor_gpio_config(gpio_t pin_type,
	hwsensor_board_info_t *sensor_info,
	struct sensor_power_setting *power_setting,
	int state)
{
	int rc;
	int output;
	int config_value;
	int i;
	bool vote_flag = false;

	if (hisi_is_clt_flag()) {
		cam_err("%s just return for CLT camera", __func__);
		return 0;
	}

	cam_info("%s enter, gpio[%d]:%d state:%d delay:%u",
		__func__, pin_type, sensor_info->gpios[pin_type].gpio,
		state, power_setting->delay);

	if (hw_is_fpga_board())
		return 0;

	if (sensor_info->gpios[pin_type].gpio == 0) {
		cam_err("gpio type[%d] is not actived", pin_type);
		return 0;
	}

	for (i = 0; i < g_gpio_vote.gpio_num; i++) {
		if (g_gpio_vote.gpio_pin[i] ==
			sensor_info->gpios[pin_type].gpio) {
			cam_info("%s gpio %u need vote",
				__func__, g_gpio_vote.gpio_pin[i]);
			vote_flag = true;
			break;
		}
	}

	config_value = power_setting->config_val;
	/*
	 * GPIO output state according to the power on-off state
	 * as same as before. when power on, config_value 0 for output high,
	 * config_value 1 for output low. Formula as below.
	 */
	output = state ? (config_value + 1) % 2 : config_value;
	if (vote_flag) {
		rc = hw_sensor_gpio_vote(i, output);
		if (power_setting->delay != 0)
			hw_camdrv_msleep(power_setting->delay);
		return rc;
	}

	rc = gpio_request(sensor_info->gpios[pin_type].gpio, NULL);
	if (rc < 0) {
		cam_err("failed to request gpio[%d]",
			sensor_info->gpios[pin_type].gpio);
		return rc;
	}

	if (pin_type == FSIN) {
		cam_info("pin_level: %d",
			gpio_get_value(sensor_info->gpios[pin_type].gpio));
		rc = 0;
	} else {
		rc = gpio_direction_output(sensor_info->gpios[pin_type].gpio,
			output);
		if (rc < 0)
			cam_err("failed to control gpio %d",
				sensor_info->gpios[pin_type].gpio);
		else
			cam_info("%s config gpio %d output %d",
				__func__,
				sensor_info->gpios[pin_type].gpio,
				output);
	}

	gpio_free(sensor_info->gpios[pin_type].gpio);

	if (power_setting->delay != 0)
		hw_camdrv_msleep(power_setting->delay);

	return rc;
}

static int hw_sensor_ldo_not_iopw(ldo_index_t ldo,
	hwsensor_board_info_t *sensor_info,
	struct sensor_power_setting *power_setting,
	const char *ldo_names, int index)
{
	int rc;

	rc = regulator_bulk_get(sensor_info->dev, 1,
		&sensor_info->ldo[index]);
	cam_info("%s regulator_bulk_get supply = %s index = %d, rc = %d\n",
		__func__, sensor_info->ldo[index].supply, index, rc);
	if (rc < 0) {
		cam_err("%s failed to get ldo[%s]",
			__func__, ldo_names[ldo]);
		return rc;
	}

	rc = regulator_set_voltage(sensor_info->ldo[index].consumer,
		power_setting->config_val, power_setting->config_val);
	if (rc < 0) {
		cam_err("%s failed to set ldo[%s] to %d V",
			__func__, ldo_names[ldo],
			power_setting->config_val);
		return rc;
	}

	return rc;
}

static int hw_sensor_ldo_config(ldo_index_t ldo,
	hwsensor_board_info_t *sensor_info,
	struct sensor_power_setting *power_setting, int state)
{
	int index;
	int rc = -1;
	const char *ldo_names[LDO_MAX] = {
		"dvdd", "dvdd2", "avdd", "avdd2", "vcm", "vcm2", "iopw", "misp",
		"avdd0", "avdd1", "miniisp", "iovdd", "oisdrv", "mipiswitch",
		"afvdd", "drvvdd"
	};

	if (!sensor_info) {
		cam_err("argument sensor_info is NULL");
		return -1;
	}
	if (!power_setting) {
		cam_err("argument power_setting is NULL");
		return -1;
	}
	if (ldo >= LDO_MAX) {
		cam_err("argument ldo is over max value");
		return -1;
	}

	cam_info("%s enter, ldo:%s voltage:%d state:%d",
		__func__, ldo_names[ldo], power_setting->config_val, state);

	if (hw_is_fpga_board())
		return 0;

	for (index = 0; index < sensor_info->ldo_num; index++) {
		if (!strncmp(sensor_info->ldo[index].supply,
			ldo_names[ldo], strlen(ldo_names[ldo])))
			break;
	}

	if (index == sensor_info->ldo_num) {
		cam_err("ldo [%s] is not actived", ldo_names[ldo]);
		return 0;
	}

	if (state != POWER_ON && state != POWER_OFF) {
		cam_err("state is wrong value, %d", state);
		return -1;
	}

	if (state == POWER_OFF) {
		if (sensor_info->ldo[index].consumer) {
			rc = regulator_bulk_disable(1,
				&sensor_info->ldo[index]);
			cam_info("%s disable regulators index = %d, rc = %d\n",
				__func__, index, rc);
			if (rc) {
				cam_err("failed to disable regulators %d\n",
					rc);
				return rc;
			}
			regulator_bulk_free(1, &sensor_info->ldo[index]);
		}
		return 0;
	}

	/* POWERON */
	if (ldo != LDO_IOPW) {
		rc = hw_sensor_ldo_not_iopw(ldo, sensor_info,
			power_setting, ldo_names, index);
		if (rc < 0)
			return rc;
	}

	rc = regulator_bulk_enable(1, &sensor_info->ldo[index]);
	if (rc) {
		cam_err("%s failed to enable regulators %d\n", __func__, rc);
		return rc;
	}
	if (power_setting->delay != 0)
		hw_camdrv_msleep(power_setting->delay);

	return rc;
}

static void hw_sensor_i2c_config(sensor_t *s_ctrl,
	struct sensor_power_setting *power_setting, int state)
{
	cam_debug("enter %s, state:%d", __func__, state);

	if (hw_is_fpga_board())
		return;

	if (state == POWER_ON) {
		if (power_setting->delay != 0)
			hw_camdrv_msleep(power_setting->delay);
	}
}

static int hw_sensor_pmic_config(hwsensor_board_info_t *sensor_info,
	struct sensor_power_setting *power_setting,
	int state)
{
	int rc = -1;
	struct hisi_pmic_ctrl_t *pmic_ctrl = NULL;

	cam_debug("%s enter", __func__);
	cam_debug("%s seq_val = %d, config_val = %d, state = %d",
		__func__, power_setting->seq_val,
		power_setting->config_val, state);

	pmic_ctrl = hisi_get_pmic_ctrl();
	if (pmic_ctrl)
		rc = pmic_ctrl->func_tbl->pmic_seq_config(pmic_ctrl,
			power_setting->seq_val,
			power_setting->config_val, state);
	else
		cam_err("hisi_get_pmic_ctrl is null");

	if (power_setting->delay != 0)
		hw_camdrv_msleep(power_setting->delay);

	return rc;
}


static int hw_sensor_buck_config(hwsensor_board_info_t *sensor_info,
	struct sensor_power_setting *power_setting,
	int state)
{
	int rc = -1;
#ifdef CONFIG_HUAWEI_CAMERA_BUCK
	struct hw_buck_ctrl_t *buck_ctrl = NULL;

	cam_info("%s enter seq_val = %d, config_val = %d, state = %d",
		__func__, power_setting->seq_val,
		power_setting->config_val, state);

	buck_ctrl = hw_get_buck_ctrl();
	if (buck_ctrl && buck_ctrl->func_tbl
		&& buck_ctrl->func_tbl->buck_power_config)
		rc = buck_ctrl->func_tbl->buck_power_config(buck_ctrl,
			power_setting->config_val,
			state);
	else
		cam_err("faid to get buck ctrl");

	if (power_setting->delay != 0)
		hw_camdrv_msleep(power_setting->delay);
#endif
	return rc;
}

static int hw_sensor_phy_clk_enable(hwsensor_board_info_t *sensor_info)
{
	unsigned int i;
	unsigned int clk_num = sensor_info->phy_clk_num;
	int ret;

	if (clk_num == 0)
		return 0;

	if (clk_num > CSI_INDEX_MAX) {
		cam_err("clock-num is not invaild %d\n", clk_num);
		return -1;
	}

	for (i = 0; i < clk_num; i++) {
		if (IS_ERR_OR_NULL(sensor_info->phy_clk[i])) {
			cam_err("phy clk err %d\n", i);
			return -1;
		}

		ret = clk_set_rate(sensor_info->phy_clk[i],
			sensor_info->phy_clk_freq);
		if (ret < 0) {
			cam_err("set rate error %d\n",
				sensor_info->phy_clk_freq);
			return -1;
		}

		ret = clk_prepare_enable(sensor_info->phy_clk[i]);
		if (ret < 0) {
			cam_err("clk_prepare_enable err %d\n", i);
			return -1;
		}
	}

	return 0;
}

static int hw_sensor_phy_clk_disable(hwsensor_board_info_t *sensor_info)
{
	unsigned int i;
	unsigned int clk_num = sensor_info->phy_clk_num;

	if (clk_num == 0)
		return 0;

	if (clk_num > CSI_INDEX_MAX) {
		cam_err("clock-num is not invaild %d\n", clk_num);
		return -1;
	}

	for (i = 0; i < clk_num; i++) {
		if (IS_ERR_OR_NULL(sensor_info->phy_clk[i])) {
			cam_err("phy clk err %d\n", i);
			return -1;
		}

		clk_disable_unprepare(sensor_info->phy_clk[i]);
	}

	return 0;
}

static int hw_sensor_pmic2_config(struct sensor_power_setting *power_setting,
	int on_off)
{
	struct hw_comm_pmic_cfg_t fp_pmic_ldo_set = {0};

	fp_pmic_ldo_set.pmic_num = 1;
	fp_pmic_ldo_set.pmic_power_type = power_setting->seq_val;
	fp_pmic_ldo_set.pmic_power_voltage = power_setting->config_val;
	fp_pmic_ldo_set.pmic_power_state = on_off;
	cam_debug("%s, seq_type:%u SENSOR_PMIC2",
		__func__, power_setting->seq_type);
	hw_pmic_power_cfg(MAIN_CAM_PMIC_REQ, &fp_pmic_ldo_set);

	return 0;
}

static int hw_sensor_laser_config(sensor_t *s_ctrl,
	struct sensor_power_setting *power_setting, int on_off)
{
	int rc;
	unsigned int gpio_no = s_ctrl->board_info->gpios[LASER_XSHUT].gpio;

	if (gpio_no == 0) {
		cam_err("gpio type[%d] is not activated", LASER_XSHUT);
		rc = 0;
		return rc;
	}
	rc = gpio_direction_output(gpio_no, on_off);
	if (rc < 0)
		cam_err("failed to control gpio %d", gpio_no);
	else
		cam_info("%s config gpio %d output %d",
			__func__, gpio_no, on_off);

	return rc;
}

static int hw_sensor_boot_5v_config(int on_off)
{
	return (on_off == POWER_ON) ?
		boost_5v_enable(BOOST_5V_ENABLE, BOOST_CTRL_CAMERA) :
		boost_5v_enable(BOOST_5V_DISABLE, BOOST_CTRL_CAMERA);
}

static int hw_sensor_gpio_check_level_config(hwsensor_board_info_t *sensor_info,
	struct sensor_power_setting *power_setting,
	int on_off)
{
	return (on_off == POWER_ON) ?
		hw_sensor_gpio_config(FSIN, sensor_info, power_setting, on_off) :
		0;
}

static int hisi_sensor_power_other_config(sensor_t *s_ctrl,
	struct sensor_power_setting *power_setting, int on_off)
{
	int rc = -1;

	switch (power_setting->seq_type) {
	case SENSOR_MCLK:
		cam_info("%s, seq_type:%u SENSOR_MCLK",
			__func__, power_setting->seq_type);
		rc = hw_mclk_config(s_ctrl, power_setting, on_off);
		break;
	case SENSOR_I2C:
		cam_info("%s, seq_type:%u SENSOR_I2C",
			__func__, power_setting->seq_type);
		hw_sensor_i2c_config(s_ctrl, power_setting, on_off);
		break;
	case SENSOR_CHECK_LEVEL:
		cam_info("%s, seq_type:%u SENSOR_CHECK_LEVEL",
			__func__, power_setting->seq_type);
		rc = hw_sensor_gpio_check_level_config(s_ctrl->board_info,
			power_setting, on_off);
		break;
	case SENSOR_PMIC:
		cam_info("%s, seq_type:%u SENSOR_PMIC",
			__func__, power_setting->seq_type);
		rc = hw_sensor_pmic_config(s_ctrl->board_info,
			power_setting, on_off);
		break;
	case SENSOR_PMIC2:
		cam_info("%s, seq_type:%u SENSOR_PMIC2",
			__func__, power_setting->seq_type);
		rc = hw_sensor_pmic2_config(power_setting, on_off);
		break;
	case SENSOR_LASER_XSHUT:
		cam_info("%s, seq_type:%u SENSOR_LASER_XSHUT",
			__func__, power_setting->seq_type);
		rc = hw_sensor_laser_config(s_ctrl, power_setting,
			on_off);
		break;
	case SENSOR_BUCK:
		cam_info("%s, seq_type:%u SENSOR_BUCK",
			__func__, power_setting->seq_type);
		rc = hw_sensor_buck_config(s_ctrl->board_info,
			power_setting,
			on_off);
		break;
	case SENSOR_BOOT_5V:
		cam_info("%s, seq_type:%u SENSOR_BOOT_5V",
			__func__, power_setting->seq_type);
		rc = hw_sensor_boot_5v_config(on_off);
		break;
	case SENSOR_CS:
		break;
	default:
		cam_err("%s invalid seq_type %d",
			__func__, power_setting->seq_type);
		break;
	}
	return rc;
}


static int hisi_sensor_power_config(sensor_t *s_ctrl,
	struct sensor_power_setting *power_setting, int on_off)
{
	int rc = -1;
	int ldo_config_size = (int)sizeof(g_ldo_config_type_tab) /
		sizeof(g_ldo_config_type_tab[0]);
	for (int i = 0; i < ldo_config_size; i++) {
		if (power_setting->seq_type ==
			g_ldo_config_type_tab[i].seq_type) {
			cam_info("%s, seq_type: %u ", __func__,
				power_setting->seq_type);
			rc = hw_sensor_ldo_config(g_ldo_config_type_tab[i].ldo_index,
				s_ctrl->board_info, power_setting, on_off);
			if (g_ldo_config_type_tab[i].ldo_ignore_exceptions)
				rc = 0;

			return rc;
		}
	}

	int gpio_config_size = (int)sizeof(g_gpio_config_type_tab) /
		sizeof(g_gpio_config_type_tab[0]);
	for (i = 0; i < gpio_config_size; i++) {
		if (power_setting->seq_type ==
			g_gpio_config_type_tab[i].seq_type) {
			cam_info("%s, seq_type:%u ", __func__,
				power_setting->seq_type);
			rc = hw_sensor_gpio_config(g_gpio_config_type_tab[i].gpio_index,
				s_ctrl->board_info, power_setting, on_off);
			if (g_gpio_config_type_tab[i].gpio_ignore_exceptions)
				rc = 0;

			return rc;
		}
	}
	rc = hisi_sensor_power_other_config(s_ctrl, power_setting, on_off);

	return rc;
}

int hw_sensor_power_up(sensor_t *s_ctrl)
{
	struct sensor_power_setting_array *power_setting_array = NULL;
	struct sensor_power_setting *power_setting = NULL;
	struct hisi_pmic_ctrl_t *pmic_ctrl = NULL;
	int index;
	int rc;

	if (hisi_is_clt_flag()) {
		cam_debug("%s just return for CLT camera", __func__);
		return 0;
	}

	cam_info("%s enter", __func__);
	power_setting_array = &s_ctrl->power_setting_array;

	if (s_ctrl->p_atpowercnt) {
		if (atomic_read(s_ctrl->p_atpowercnt)) {
			cam_info("%s %d: sensor already powerup, p_atpowercnt",
				__func__, __LINE__);
			return 0;
		}
	} else {
		if (atomic_read(&g_powered)) {
			cam_info("%s %d: sensor has already powered up",
				__func__, __LINE__);
			return 0;
		}
	}

	/* fpga board compatibility */
	if (hw_is_fpga_board())
		return 0;

	rc = hw_sensor_phy_clk_enable(s_ctrl->board_info);
	if (rc) {
		cam_err("%s %d: hw_sensor_phy_clk_enable fail rc = %d",
			__func__, __LINE__, rc);
		return rc;
	}

	pmic_ctrl = hisi_get_pmic_ctrl();
	if (pmic_ctrl) {
		cam_debug("pmic power on");
		pmic_ctrl->func_tbl->pmic_on(pmic_ctrl, 0);
	} else {
		cam_debug("%s pimc ctrl is null", __func__);
	}

	/* lpm3 buck support, do wrtel enable */
	if (s_ctrl->board_info->lpm3_gpu_buck == 1) {
		if (!lpm3) {
			cam_info("%s lpm3 is null", __func__);
		} else {
			cam_info("%s need to set LPM3_GPU_BUCK", __func__);
			/* do enable */
			writel(LPM3_REGISTER_ENABLE, lpm3);
			cam_info("%s read LPM3_GPU_BUCK is %d",
				__func__, readl(lpm3));
		}
	}
	for (index = 0; index < power_setting_array->size; index++) {
		power_setting = &power_setting_array->power_setting[index];

		rc = hisi_sensor_power_config(s_ctrl, power_setting, POWER_ON);
		if (rc) {
			cam_err("%s power up procedure error", __func__);
			break;
		}
	}

	if (s_ctrl->p_atpowercnt) {
		atomic_set(s_ctrl->p_atpowercnt, 1);
		cam_info("%s %d: sensor powered up finish",
			__func__, __LINE__);
	} else {
		atomic_set(&g_powered, 1);
		cam_info("%s %d: sensor powered up finish",
			__func__, __LINE__);
	}

	return rc;
}

int hw_sensor_power_down(sensor_t *s_ctrl)
{
	struct sensor_power_setting_array *power_setting_array = NULL;
	struct sensor_power_setting *power_setting = NULL;
	struct hisi_pmic_ctrl_t *pmic_ctrl = NULL;
	int index;
	int rc = 0;

	if (hisi_is_clt_flag()) {
		cam_info("%s just return for CLT camera", __func__);
		return 0;
	}

	if (!s_ctrl) {
		cam_err("%s s_ctrl is null", __func__);
		return -1;
	}

	cam_info("%s enter", __func__);

	if (s_ctrl->p_atpowercnt) {
		if (!atomic_read(s_ctrl->p_atpowercnt)) {
			cam_info("%s [%d]: sensor hasn't powered up",
				__func__, __LINE__);
			return 0;
		}
	} else {
		if (!atomic_read(&g_powered)) {
			cam_info("%s [%d]: sensor hasn't powered up",
				__func__, __LINE__);
			return 0;
		}
	}

	if (!s_ctrl->board_info) {
		cam_err("%s s_ctrl->board_info is null", __func__);
		return -1;
	}

	if (s_ctrl->board_info->use_power_down_seq == 1) {
		int size = s_ctrl->power_down_setting_array.size;

		if (!s_ctrl->power_down_setting_array.power_setting ||
			size <= 0) {
			cam_err("%s invalid power down setting, size %d",
				__func__, size);
			return -1;
		}
		power_setting_array = &s_ctrl->power_down_setting_array;

		for (index = 0; index < size; index++) {
			power_setting =
				&power_setting_array->power_setting[index];
			rc = hisi_sensor_power_config(s_ctrl, power_setting,
				POWER_OFF);
		}
	} else {
		int size = s_ctrl->power_setting_array.size;

		if (!s_ctrl->power_setting_array.power_setting || size <= 0) {
			cam_err("%s invalid power down setting, size %d",
				__func__, size);
			return -1;
		}
		power_setting_array = &s_ctrl->power_setting_array;

		for (index = size - 1; index >= 0; index--) {
			power_setting =
				&power_setting_array->power_setting[index];
			rc = hisi_sensor_power_config(s_ctrl, power_setting,
				POWER_OFF);
		}
	}

	/* lpm3 buck support, do wrtel disable */
	if (s_ctrl->board_info->lpm3_gpu_buck == 1) {
		if (!lpm3) {
			cam_info("%s lpm3 is null", __func__);
		} else {
			cam_info("%s need to clear LPM3_GPU_BUCK", __func__);
			/* do disable */
			writel(LPM3_REGISTER_DISABLE, lpm3);
			cam_info("%s read LPM3_GPU_BUCK is %d", __func__,
				readl(lpm3));
		}
	}

	pmic_ctrl = hisi_get_pmic_ctrl();
	if (pmic_ctrl)
		pmic_ctrl->func_tbl->pmic_off(pmic_ctrl);

	rc = hw_sensor_phy_clk_disable(s_ctrl->board_info);

	if (s_ctrl->p_atpowercnt)
		atomic_set(s_ctrl->p_atpowercnt, 0);
	else
		atomic_set(&g_powered, 0);

	return rc;
}

static int hw_sensor_get_phy_clk(struct device *pdev,
	struct device_node *of_node,
	hwsensor_board_info_t *sensor_info)
{
	unsigned int i;
	const char *clk_name[CSI_INDEX_MAX] = {""};
	unsigned int clk_num = 0;
	int ret;

	ret = of_property_read_u32(of_node, "phy-clock-num", &clk_num);
	if (ret) {
		cam_info("invalid phy-clock-num\n");
		return -1;
	}

	if (clk_num > CSI_INDEX_MAX) {
		cam_err("phy-clock-num is not invaild %d\n", clk_num);
		return -1;
	}

	// get clk parameters
	ret = of_property_read_string_array(of_node, "clock-names",
		clk_name, clk_num);
	if (ret < 0) {
		cam_err("[%s] Failed : of_property_read_string_array %d\n",
			__func__, ret);
		return -1;
	}

	for (i = 0; i < clk_num; ++i)
		cam_info("[%s] clk_name[%d] = %s\n", __func__, i, clk_name[i]);

	for (i = 0; i < clk_num; i++) {
		sensor_info->phy_clk[i] = devm_clk_get(pdev, clk_name[i]);
		if (IS_ERR_OR_NULL(sensor_info->phy_clk[i])) {
			cam_err("[%s] Failed : phyclk %s %d %li\n",
				__func__, clk_name[i], i,
				PTR_ERR(sensor_info->phy_clk[i]));
			return -1;
		}
	}

	sensor_info->phy_clk_num = clk_num;

	return 0;
}

static int hw_sensor_get_phy_dt_data(struct device *pdev,
	struct device_node *of_node,
	hwsensor_board_info_t *sensor_info)
{
	int ret;

	ret = of_property_read_u32(of_node, "huawei,phyclk",
		&sensor_info->phy_clk_freq);
	if (ret) {
		cam_info("invalid huawei,phyclk\n");
		return -1;
	}

	ret = hw_sensor_get_phy_clk(pdev, of_node, sensor_info);
	if (ret)
		return -1;

	return 0;
}

static int hw_sensor_get_phyinfo_data(struct device_node *of_node,
	hwsensor_board_info_t *sensor_info, int info_count)
{
	int ret;
	int count;

	if (!of_node || !sensor_info) {
		cam_err("%s param is invalid", __func__);
		return -1;
	}
	count = of_property_count_elems_of_size(of_node,
		"huawei,is_master_sensor",
		sizeof(u32));
	if (count != info_count) {
		cam_err("%s %d, count = %d", __func__, __LINE__, count);
		return -1;
	}
	ret = of_property_read_u32_array(of_node, "huawei,is_master_sensor",
		(unsigned int *)sensor_info->phyinfo.is_master_sensor, count);

	count = of_property_count_elems_of_size(of_node, "huawei,phy_id",
		sizeof(u32));
	if (count != info_count) {
		cam_err("%s %d, count = %d", __func__, __LINE__, count);
		return -1;
	}
	ret = (int)((unsigned int)ret |
		(unsigned int)of_property_read_u32_array(of_node,
		"huawei,phy_id",
		(unsigned int *)sensor_info->phyinfo.phy_id, count));

	count = of_property_count_elems_of_size(of_node, "huawei,phy_mode",
		sizeof(u32));
	if (count != info_count) {
		cam_err("%s %d, count = %d", __func__, __LINE__, count);
		return -1;
	}
	ret = (int)((unsigned int)ret |
		(unsigned int)of_property_read_u32_array(of_node,
		"huawei,phy_mode",
		(unsigned int *)sensor_info->phyinfo.phy_mode, count));

	count = of_property_count_elems_of_size(of_node, "huawei,phy_freq_mode",
		sizeof(u32));
	if (count != info_count) {
		cam_err("%s %d, count = %d", __func__, __LINE__, count);
		return -1;
	}
	ret = (int)((unsigned int)ret |
		(unsigned int) of_property_read_u32_array(of_node,
		"huawei,phy_freq_mode",
		(unsigned int *)sensor_info->phyinfo.phy_freq_mode, count));

	count = of_property_count_elems_of_size(of_node, "huawei,phy_freq",
		sizeof(u32));
	if (count != info_count) {
		cam_err("%s %d, count = %d", __func__, __LINE__, count);
		return -1;
	}
	ret = (int)((unsigned int)ret |
		(unsigned int)of_property_read_u32_array(of_node,
		"huawei,phy_freq",
		(unsigned int *)sensor_info->phyinfo.phy_freq, count));

	count = of_property_count_elems_of_size(of_node, "huawei,phy_work_mode",
		sizeof(u32));
	if (count != info_count) {
		cam_err("%s %d, count = %d", __func__, __LINE__, count);
		return -1;
	}
	ret = (int)((unsigned int)ret |
		(unsigned int)of_property_read_u32_array(of_node,
		"huawei,phy_work_mode",
		(unsigned int *)sensor_info->phyinfo.phy_work_mode, count));

	cam_info("%s, info_count = %d\n"
		"is_master_sensor[0] = %d, is_master_sensor[1] = %d\n"
		"phy_id[0] = %d, phy_id[1] = %d\n"
		"phy_mode[0] = %d, phy_mode[1] = %d\n"
		"phy_freq_mode[0] = %d, phy_freq_mode[1] = %d\n"
		"phy_freq[0] = %d, phy_freq[1] = %d\n"
		"phy_work_mode[0] = %d, phy_work_mode[1] = %d",
		__func__, info_count,
		sensor_info->phyinfo.is_master_sensor[0],
		sensor_info->phyinfo.is_master_sensor[1],
		sensor_info->phyinfo.phy_id[0],
		sensor_info->phyinfo.phy_id[1],
		sensor_info->phyinfo.phy_mode[0],
		sensor_info->phyinfo.phy_mode[1],
		sensor_info->phyinfo.phy_freq_mode[0],
		sensor_info->phyinfo.phy_freq_mode[1],
		sensor_info->phyinfo.phy_freq[0],
		sensor_info->phyinfo.phy_freq[1],
		sensor_info->phyinfo.phy_work_mode[0],
		sensor_info->phyinfo.phy_work_mode[1]
	);

	return ret;
}

static int hw_sensor_get_ext_dt_data(struct device_node *of_node,
	hwsensor_board_info_t *sensor_info)
{
	int rc;
	unsigned int i;
	unsigned int count;
	const char *ext_name = NULL;

	if (!of_node || !sensor_info) {
		cam_err("%s of_node or sensor_info is NULL", __func__);
		return -EINVAL;
	}
	sensor_info->ext_type = 0;
	rc = of_property_read_u32(of_node, "huawei,ext_type",
		(u32 *)&sensor_info->ext_type);
	if (rc < 0) {
		cam_warn("%s no ext name %d\n", __func__, __LINE__);
		sensor_info->ext_type = 0;
		return 0;
	}
	cam_info("%s sensor_info->ext_type %d, rc %d\n", __func__,
		sensor_info->ext_type, rc);

	if (sensor_info->ext_type == EXT_INFO_NO_ADC) {
		rc = of_property_read_string(of_node,
			"huawei,ext_name", &ext_name);
		if (rc < 0) {
			cam_err("%s get ext_name failed %d\n",
				__func__, __LINE__);
			sensor_info->ext_type = NO_EXT_INFO;
			return -EINVAL;
		}

		strncpy_s(sensor_info->ext_name[0],
			DEVICE_NAME_SIZE - 1,
			ext_name, strlen(ext_name) + 1);
		cam_info("%s huawei,ext_name %s, rc %d\n",
			__func__, ext_name, rc);
	}

	if (sensor_info->ext_type == EXT_INFO_ADC) {
		sensor_info->adc_channel = -1;
		rc = of_property_read_u32(of_node, "huawei,adc_channel",
			(u32 *)&sensor_info->adc_channel);
		if (rc < 0) {
			cam_err("%s get adc_channel failed %d\n",
				__func__, __LINE__);
			sensor_info->ext_type = NO_EXT_INFO;
			return -EINVAL;
		}
		cam_info("%s sensor_info->adc_channel %d, rc %d\n", __func__,
			sensor_info->adc_channel, rc);
		sensor_info->ext_num = 0;
		sensor_info->ext_num = of_property_count_elems_of_size(of_node,
				"huawei,adc_threshold", sizeof(u32));
		if ((sensor_info->ext_num > 0) &&
			(sensor_info->ext_num <= EXT_THRESHOLD_NUM)) {
			rc = of_property_read_u32_array(of_node,
				"huawei,adc_threshold",
				(u32 *)sensor_info->adc_threshold,
				sensor_info->ext_num);
			if (rc < 0) {
				sensor_info->ext_type = NO_EXT_INFO;
				cam_err("%s get huawei,adc_threshold failed %d\n",
					__func__, __LINE__);
				return -EINVAL;
			}
		} else {
			cam_err("%s ext threshold num = %d out of range\n",
				__func__, sensor_info->ext_num);
			sensor_info->ext_type = NO_EXT_INFO;
		}
		count = of_property_count_strings(of_node, "huawei,ext_name");
		if ((count > 0) && (count <= EXT_NAME_NUM)) {
			for (i = 0; i < count; i++) {
				rc = of_property_read_string_index(of_node,
					"huawei,ext_name",
					i, &ext_name);
				if (rc < 0) {
					sensor_info->ext_type = NO_EXT_INFO;
					cam_err("%s get ext_name failed %d\n",
						__func__, __LINE__);
					return -EINVAL;
				}
				strncpy_s(sensor_info->ext_name[i],
					DEVICE_NAME_SIZE - 1,
					ext_name, strlen(ext_name) + 1);
				cam_info("%s ext_name: %s\n",
					__func__, ext_name);
			}
		} else {
			cam_err("%s ext name num = %d out of range\n",
				__func__, count);
			sensor_info->ext_type = NO_EXT_INFO;
		}
	}

	return 0;
}

int read_basic_sensor_info_stage1(struct device_node *of_node,
	hwsensor_board_info_t *sensor_info)
{
	int rc;
	if (!of_node || !sensor_info)
		return -1;

	rc = of_property_read_string(of_node, "huawei,sensor_name",
		&sensor_info->name);
	cam_debug("%s name %s, rc %d\n", __func__, sensor_info->name, rc);
	if (rc < 0) {
		cam_err("%s failed %d\n", __func__, __LINE__);
		return rc;
	}

	rc = of_property_read_u32(of_node, "huawei,is_fpga",
		&g_is_fpga);
	cam_debug("%s huawei,is_fpga: %d, rc %d\n", __func__,
		g_is_fpga, rc);
	if (rc < 0) {
		cam_err("%s failed %d\n", __func__, __LINE__);
		return rc;
	}

	rc = of_property_read_u32(of_node, "huawei,sensor_index",
		(u32 *)(&sensor_info->sensor_index));
	cam_debug("%s huawei,sensor_index %d, rc %d\n", __func__,
		sensor_info->sensor_index, rc);
	if (rc < 0) {
		cam_err("%s failed %d\n", __func__, __LINE__);
		return rc;
	}

	rc = of_property_read_u32(of_node, "huawei,pd_valid",
		&sensor_info->power_conf.pd_valid);
	cam_debug("%s huawei,pd_valid %d, rc %d\n", __func__,
		sensor_info->power_conf.pd_valid, rc);
	if (rc < 0) {
		cam_err("%s failed %d\n", __func__, __LINE__);
		return rc;
	}

	rc = of_property_read_u32(of_node, "huawei,reset_valid",
		&sensor_info->power_conf.reset_valid);
	cam_debug("%s huawei,reset_valid %d, rc %d\n", __func__,
		sensor_info->power_conf.reset_valid, rc);
	if (rc < 0) {
		cam_err("%s failed %d\n", __func__, __LINE__);
		return rc;
	}

	rc = of_property_read_u32(of_node, "huawei,vcmpd_valid",
		&sensor_info->power_conf.vcmpd_valid);
	cam_debug("%s huawei,vcmpd_valid %d, rc %d\n", __func__,
		sensor_info->power_conf.vcmpd_valid, rc);
	if (rc < 0) {
		cam_err("%s failed %d\n", __func__, __LINE__);
		return rc;
	}

	return rc;
}

int read_basic_sensor_info_stage2(struct device_node *of_node,
	hwsensor_board_info_t *sensor_info, struct platform_device *pdev)
{
	int rc;
	int count;

	if (!of_node || !sensor_info || !pdev)
		return -1;

	// add csi_index and i2c_index for dual camera.
	count = of_property_count_elems_of_size(of_node, "huawei,csi_index",
		sizeof(u32));
	if (count > 0) {
		of_property_read_u32_array(of_node, "huawei,csi_index",
			(unsigned int *)&sensor_info->csi_id, count);
	} else {
		sensor_info->csi_id[0] = sensor_info->sensor_index;
		sensor_info->csi_id[1] = -1;
	}
	cam_info("sensor:%s csi_id[0-1] = %d: %d", sensor_info->name,
		sensor_info->csi_id[0], sensor_info->csi_id[1]);

	sensor_info->bus_type = "";
	rc = of_property_read_string(of_node,
		"huawei,bus_type", &sensor_info->bus_type);
	if (rc < 0)
		cam_err("%s failed %d\n", __func__, __LINE__);

	count = of_property_count_elems_of_size(of_node, "huawei,i2c_index",
		sizeof(u32));
	if (count > 0) {
		of_property_read_u32_array(of_node, "huawei,i2c_index",
			(unsigned int *)&sensor_info->i2c_id, count);
	} else {
		sensor_info->i2c_id[0] = sensor_info->sensor_index;
		sensor_info->i2c_id[1] = -1;
	}
	cam_info("sensor:%s i2c_id[0-1] = %d: %d", sensor_info->name,
		sensor_info->i2c_id[0], sensor_info->i2c_id[1]);

	count = of_property_count_elems_of_size(of_node,
		"huawei,ao_i2c_index", sizeof(u32));
	if (count > 0) {
		int ret = of_property_read_u32_array(of_node,
			"huawei,ao_i2c_index",
			(unsigned int *)&sensor_info->ao_i2c_id,
			(unsigned long)(long)count);
		if (ret < 0)
			cam_info("read ao_i2c_index failed %d\n",  __LINE__);
	}
	rc = hw_sensor_get_phy_dt_data(&pdev->dev, of_node, sensor_info);
	if (rc < 0)
		cam_info("%s phy data not ready %d\n", __func__, __LINE__);

	rc = of_property_read_u32(of_node, "huawei,phyinfo_valid",
		(u32 *)&sensor_info->phyinfo_valid);
	cam_info("%s sensor_info->phyinfo_valid %d, rc %d\n", __func__,
		sensor_info->phyinfo_valid, rc);
	if (!rc && (sensor_info->phyinfo_valid == 1 ||
		sensor_info->phyinfo_valid == 2))
		rc = hw_sensor_get_phyinfo_data(of_node, sensor_info,
			sensor_info->phyinfo_valid);

	return 0;
}

static int read_basic_sensor_info(struct platform_device *pdev,
	struct device_node *of_node, hwsensor_board_info_t *sensor_info,
	int *rc)
{
	if (!of_node || !sensor_info || !pdev)
		return -1;

	*rc = read_basic_sensor_info_stage1(of_node, sensor_info);
	if (*rc < 0) {
		cam_err("%s failed %d\n", __func__, __LINE__);
		return -1;
	}

	*rc = read_basic_sensor_info_stage2(of_node, sensor_info, pdev);
	if (*rc < 0) {
		cam_err("%s failed %d\n", __func__, __LINE__);
		return -1;
	}

	return 0;
}

static int read_sensor_gpio_vote_info(struct device_node *of_node, int *rc)
{
	int vote_gpio_size = 0;
	unsigned int vote_gpio_pin[MAX_SHARED_GPIO_NUM] = {0};
	int i;
	int j;

	vote_gpio_size = of_property_count_elems_of_size(of_node,
		"gpio_vote_pin", sizeof(u32));
	if (vote_gpio_size <= 0) {
		cam_info("%s no config gpio vote, size is %d",
			__func__, vote_gpio_size);
		return 0;
	}

	vote_gpio_size = vote_gpio_size > MAX_SHARED_GPIO_NUM ?
		MAX_SHARED_GPIO_NUM : vote_gpio_size;

	*rc = of_property_read_u32_array(of_node, "gpio_vote_pin",
		vote_gpio_pin, vote_gpio_size);
	if (*rc < 0) {
		cam_err("%s failed %d\n", __func__, __LINE__);
		return -1;
	}

	for (i = 0; i < vote_gpio_size; i++) {
		if (vote_gpio_pin[i] == 0)
			continue;

		for (j = 0; j < g_gpio_vote.gpio_num; j++)
			if (g_gpio_vote.gpio_pin[j] == vote_gpio_pin[i])
				break;

		if ((j == g_gpio_vote.gpio_num) &&
			(g_gpio_vote.gpio_num < MAX_SHARED_GPIO_NUM)) {
			g_gpio_vote.gpio_pin[g_gpio_vote.gpio_num] =
				vote_gpio_pin[i];
			g_gpio_vote.ref_count[g_gpio_vote.gpio_num] = 0;
			g_gpio_vote.gpio_num++;
		}
	}

	return 0;
}

static int read_sensor_power_info(struct platform_device *pdev,
	struct device_node *of_node,
	hwsensor_board_info_t *sensor_info,
	int *rc)
{
	int i;
	char *gpio_tag = NULL;
	int index;
	/* enum gpio_t */
	const char *gpio_ctrl_types[IO_MAX] = {
		"reset", "fsin", "pwdn", "vcm_pwdn", "suspend", "suspend2", \
		"reset2", "ldo_en", "ois", "ois2", "dvdd0-en", "dvdd1-en", \
		"iovdd-en", "mispdcdc-en", "mipisw", "reset3", "pwdn2", \
		"avdd1_en", "avdd2_en", " ", "afvdd_en", "laser_xshut"
	};

	if (!pdev || !of_node || !sensor_info)
		return -1;

	*rc = of_property_read_u32(of_node, "use_power_down_seq",
		&sensor_info->use_power_down_seq);
	cam_info("%s use_power_down_seq 0x%x, rc %d", __func__,
		sensor_info->use_power_down_seq, *rc);
	if (*rc < 0)
		sensor_info->use_power_down_seq = 0;

	*rc = of_property_read_u32(of_node, "huawei,mclk",
		&sensor_info->mclk);
	cam_debug("%s huawei,mclk 0x%x, rc %d\n", __func__,
		sensor_info->mclk, *rc);
	if (*rc < 0) {
		cam_err("%s failed %d\n", __func__, __LINE__);
		return -1;
	}

	/* get ldo */
	sensor_info->ldo_num = of_property_count_strings(of_node,
		"huawei,ldo-names");
	if (sensor_info->ldo_num < 0) {
		cam_err("%s failed %d\n", __func__, __LINE__);
	} else {
		cam_debug("ldo num = %d", sensor_info->ldo_num);
		for (i = 0; i < sensor_info->ldo_num; i++) {
			*rc = of_property_read_string_index(of_node,
				"huawei,ldo-names",
				i, &sensor_info->ldo[i].supply);
			if (*rc < 0) {
				cam_err("%s failed %d\n", __func__, __LINE__);
				return -1;
			}
		}
	}

	sensor_info->gpio_num = of_gpio_count(of_node);
	if (sensor_info->gpio_num < 0) {
		cam_err("%s failed %d, ret is %d\n", __func__, __LINE__,
			sensor_info->gpio_num);
		return -1;
	}

	cam_debug("gpio num = %d", sensor_info->gpio_num);
	for (i = 0; i < sensor_info->gpio_num; i++) {
		*rc = of_property_read_string_index(of_node,
			"huawei,gpio-ctrl-types",
			i, (const char **)&gpio_tag);
		if (*rc < 0) {
			cam_err("%s failed %d\n", __func__, __LINE__);
			return -1;
		}
		for (index = 0; index < IO_MAX; index++) {
			if (gpio_ctrl_types[index]) {
				if (!strcmp(gpio_ctrl_types[index], gpio_tag))
					sensor_info->gpios[index].gpio =
						of_get_gpio(of_node, i);
			}
		}
		cam_debug("gpio ctrl types: %s", gpio_tag);
	}

	return read_sensor_gpio_vote_info(of_node, rc);
}

static void read_sensor_info_or_set_u32(struct device_node *of_node,
	hwsensor_board_info_t *sensor_info, int *rc, char *type_name,
	int *sensor_info_type, int default_value)
{
	*rc = of_property_read_u32(of_node, type_name, sensor_info_type);
	cam_info("%s %s 0x%x, rc %d\n", __func__, type_name,
		*sensor_info_type, *rc);
	if (*rc < 0) {
		*sensor_info_type = default_value;
		cam_warn("%s read %s failed, rc %d, set default value %d\n",
		__func__, type_name, *rc, *sensor_info_type);
		*rc = 0;
	}
}

static int read_sensor_type_info(struct device_node *of_node,
	hwsensor_board_info_t *sensor_info, int *rc)
{
	if (!of_node || !sensor_info)
		return -1;

	read_sensor_info_or_set_u32(of_node, sensor_info, rc,
		"module_type", &sensor_info->module_type, 0);
	read_sensor_info_or_set_u32(of_node, sensor_info, rc,
		"reset_type", &sensor_info->reset_type, 0);
	read_sensor_info_or_set_u32(of_node, sensor_info, rc,
		"huawei,topology_type", &sensor_info->topology_type, -1);
	read_sensor_info_or_set_u32(of_node, sensor_info, rc,
		"need_rpc", &sensor_info->need_rpc, 0);

	sensor_info->lpm3_gpu_buck = 0;
	read_sensor_info_or_set_u32(of_node, sensor_info, rc,
		"lpm3_gpu_buck", &sensor_info->lpm3_gpu_buck, 0);

	return 0;
}

static int read_sensor_pwdn_info(struct device_node *of_node,
	hwsensor_board_info_t *sensor_info)
{
	if (!of_node || !sensor_info)
		return -EINVAL;

	int rc = of_property_read_u32(of_node, "high_impedance_pwdn",
		&sensor_info->high_impedance_pwdn);
	if (rc < 0) {
		sensor_info->high_impedance_pwdn = 0;
		cam_warn("%s read pwdn_info failed, rc %d, set default value 0\n",
		__func__, rc);
		return 0;
	}
	cam_info("%s high_impedance_pwdn 0x%x\n", __func__,
		sensor_info->high_impedance_pwdn);
	int count = of_property_count_u32_elems(of_node, "high_impedance_pwdn_val");
	count = count > PWDN_VALUE_TYPE_MAX ? PWDN_VALUE_TYPE_MAX : count;
	rc = of_property_read_u32_array(of_node, "high_impedance_pwdn_val",
		&sensor_info->high_impedance_pwdn_val, count);
	if (rc < 0) {
		cam_err("%s:high_impedance_pwdn_val not to config", __func__);
		return -EINVAL;
	}
	return 0;
}

int hw_sensor_get_dt_data(struct platform_device *pdev, sensor_t *sensor)
{
	struct device_node *of_node = pdev->dev.of_node;
	hwsensor_board_info_t *sensor_info = NULL;
	int rc = 0;
	int ret;

	cam_debug("enter %s", __func__);
	sensor_info = kzalloc(sizeof(hwsensor_board_info_t), GFP_KERNEL);
	if (!sensor_info) {
		cam_err("%s failed %d\n", __func__, __LINE__);
		return -ENOMEM;
	}
	sensor->board_info = sensor_info;
	sensor_info->dev = &(pdev->dev);

	ret = read_basic_sensor_info(pdev, of_node, sensor_info, &rc);
	if (ret < 0)
		goto fail;

	// FPGA IGNORE
	if (hw_is_fpga_board()) {
		rc = 0;
		return rc;
	}

	ret = read_sensor_power_info(pdev, of_node, sensor_info, &rc);
	if (ret < 0)
		goto fail;

	ret = read_sensor_type_info(of_node, sensor_info, &rc);
	if (ret < 0)
		goto fail;

	hw_sensor_get_ext_dt_data(of_node, sensor_info);
	ret = read_sensor_pwdn_info(of_node, sensor_info);
	if (ret < 0)
		goto fail;
	return rc;

fail:
	cam_err("%s error exit\n", __func__);
	kfree(sensor_info);
	sensor_info = NULL;
	sensor->board_info = NULL;
	return rc;
}

static void set_power_settings(int i,
	struct sensor_power_setting **power_settings,
	const char *seq_name, uint32_t *seq_vals, uint32_t *cfg_vals,
	uint32_t *sensor_indexs, uint32_t *seq_delays)
{
	(*power_settings)[i].seq_val = seq_vals[i];
	(*power_settings)[i].config_val = cfg_vals[i];
	(*power_settings)[i].sensor_index =
		((sensor_indexs[i] >= 0xFF) ? 0xFFFFFFFF : sensor_indexs[i]);
	(*power_settings)[i].delay = seq_delays[i];
	cam_info("%s:%d index[%d] seq_name[%s] seq_type[%d] cfg_vals[%d] seq_delay[%d] sensor_index[0x%x]",
		__func__, __LINE__, i, seq_name, (*power_settings)[i].seq_type,
		cfg_vals[i], seq_delays[i], sensor_indexs[i]);
	cam_info("%s:%d sensor_index = %d", __func__, __LINE__,
		(*power_settings)[i].sensor_index);
}

static int kzalloc_val(int *rc, int count, int is_power_on,
	struct device_node *dev_node, uint32_t *seq_vals, uint32_t **cfg_vals)
{
	const char *seq_val_name = (is_power_on ?
		"huawei,cam-power-seq-val" :
		"huawei,cam-power-down-seq-val");
	*rc = of_property_read_u32_array(dev_node, seq_val_name,
		seq_vals, count);
	if (*rc < 0)
		cam_warn("%s:%d seq val not to config", __func__, __LINE__);

	*cfg_vals = kzalloc(sizeof(uint32_t) * count, GFP_KERNEL);
	if (*cfg_vals == NULL) {
		cam_err("%s:%d failed\n", __func__, __LINE__);
		*rc = -ENOMEM;
		return -1;
	}
	return 0;
}

static int kzalloc_cfg(int *rc, int count, int is_power_on,
	struct device_node *dev_node, uint32_t *cfg_vals,
	uint32_t **sensor_indexs)
{
	const char *seq_cfg_name = (is_power_on ?
		"huawei,cam-power-seq-cfg-val" :
		"huawei,cam-power-down-seq-cfg-val");
	*rc = of_property_read_u32_array(dev_node, seq_cfg_name,
		cfg_vals, count);
	if (*rc < 0)
		cam_warn("%s:%d seq val not to config", __func__, __LINE__);

	*sensor_indexs = kzalloc(sizeof(uint32_t) * count, GFP_KERNEL);
	if (!(*sensor_indexs)) {
		cam_err("%s failed %d", __func__, __LINE__);
		*rc = -ENOMEM;
		return -1;
	}

	return 0;
}

static int kzalloc_sensor(int *rc, int count, int is_power_on,
	struct device_node *dev_node, uint32_t *sensor_indexs,
	uint32_t **seq_delays)
{
	const char *seq_sensor_index_name = (is_power_on ?
		"huawei,cam-power-seq-sensor-index" :
		"huawei,cam-power-down-seq-sensor-index");
	*rc = of_property_read_u32_array(dev_node, seq_sensor_index_name,
		sensor_indexs, count);
	if (*rc < 0)
		cam_warn("%s:%d sensor index not to config",
		__func__, __LINE__);

	*seq_delays = kzalloc(sizeof(uint32_t) * count, GFP_KERNEL);
	if (!(*seq_delays)) {
		cam_err("%s failed %d", __func__, __LINE__);
		*rc = -ENOMEM;
		return -1;
	}

	return 0;
}

static int kzalloc_delay(int *rc, int count, int is_power_on,
	struct device_node *dev_node, uint32_t *seq_delays,
	struct sensor_power_setting **power_settings)
{
	const char *seq_delay_name = (is_power_on ?
		"huawei,cam-power-seq-delay" :
		"huawei,cam-power-down-seq-delay");
	*rc = of_property_read_u32_array(dev_node, seq_delay_name,
		seq_delays, count);
	if (*rc < 0)
		cam_err("%s:%d seq delay not to config", __func__, __LINE__);

	*power_settings = kzalloc(sizeof(struct sensor_power_setting) * count,
		GFP_KERNEL);
	if (*power_settings == NULL) {
		cam_err("%s failed %d\n", __func__, __LINE__);
		*rc = -ENOMEM;
		return -1;
	}

	return 0;
}

static int hw_sensor_get_dt_power_setting(struct device_node *dev_node,
	struct sensor_power_setting_array *power_setting_array, int is_power_on)
{
	int rc = 0;
	int i;
	int j;
	int count;
	int seq_size;
	const char *seq_type_name = NULL;
	const char *seq_name = NULL;
	uint32_t *seq_vals = NULL;
	uint32_t *cfg_vals = NULL;
	uint32_t *sensor_indexs = NULL;
	uint32_t *seq_delays = NULL;
	struct sensor_power_setting *power_settings = NULL;

	cam_info("%s:%d is_power_on = %d", __func__, __LINE__, is_power_on);
	seq_type_name = (is_power_on ?
		"huawei,cam-power-seq-type" :
		"huawei,cam-power-down-seq-type");
	count = of_property_count_strings(dev_node, seq_type_name);
	if (count <= 0) {
		cam_warn("%s:%d power settings not to config",
			__func__, __LINE__);
		return -EINVAL;
	}

	seq_vals = kzalloc(sizeof(uint32_t) * count, GFP_KERNEL);
	if (!seq_vals) {
		cam_err("%s:%d failed\n", __func__, __LINE__);
		return -ENOMEM;
	}

	if (kzalloc_val(&rc, count, is_power_on, dev_node,
		seq_vals, &cfg_vals) == -1)
		goto OUT;

	if (kzalloc_cfg(&rc, count, is_power_on, dev_node,
		cfg_vals, &sensor_indexs) == -1)
		goto OUT;

	if (kzalloc_sensor(&rc, count, is_power_on, dev_node,
		sensor_indexs, &seq_delays) == -1)
		goto OUT;

	if (kzalloc_delay(&rc, count, is_power_on, dev_node,
		seq_delays, &power_settings) == -1)
		goto OUT;

	power_setting_array->power_setting = power_settings;
	power_setting_array->size = (unsigned int)count;

	for (i = 0; i < (int)count; i++) {
		rc = of_property_read_string_index(dev_node, seq_type_name,
			i, &seq_name);
		if (rc < 0) {
			cam_err("%s failed %d\n", __func__, __LINE__);
			goto OUT;
		}

		seq_size = (int)sizeof(g_seq_type_tab) / sizeof(g_seq_type_tab[0]);
		for (j = 0; j < seq_size; j++) {
			if (!strcmp(seq_name, g_seq_type_tab[j].seq_name)) {
				power_settings[i].seq_type =
					g_seq_type_tab[j].seq_type;
				break;
			}
		}

		if (j >= seq_size) {
			cam_warn("%s: unrecognized seq-type\n", __func__);
			rc = -EINVAL;
			goto OUT;
		}

		set_power_settings(i, &power_settings, seq_name, seq_vals,
			cfg_vals, sensor_indexs, seq_delays);
	}

OUT:
	if (seq_vals) {
		kfree(seq_vals);
		seq_vals = NULL;
	}
	if (cfg_vals) {
		kfree(cfg_vals);
		cfg_vals = NULL;
	}
	if (sensor_indexs) {
		kfree(sensor_indexs);
		sensor_indexs = NULL;
	}
	if (seq_delays) {
		kfree(seq_delays);
		seq_delays = NULL;
	}

	return rc;
}

int hw_sensor_get_dt_power_setting_data(struct platform_device *pdev,
	sensor_t *sensor)
{
	int rc;
	struct device_node *dev_node = NULL;

	if (!pdev || !pdev->dev.of_node || !sensor) {
		cam_err("%s dev_node is NULL", __func__);
		return -EINVAL;
	}

	dev_node = pdev->dev.of_node;

	rc = hw_sensor_get_dt_power_setting(dev_node,
		&sensor->power_setting_array, 1);
	if (rc < 0) {
		cam_err("%s:%d get dt power on setting fail",
			__func__, __LINE__);
		return rc;
	}

	// 0 is power down
	rc = hw_sensor_get_dt_power_setting(dev_node,
		&sensor->power_down_setting_array, 0);
	if (rc < 0) {
		cam_warn("%s:%d get dt power down setting fail, need not to config",
			__func__, __LINE__);
		return 0;
	}

	return 0;
}

void hw_camdrv_msleep(unsigned int ms)
{
	struct timeval now;
	unsigned long jiffies;

	if (ms > 0) {
		now.tv_sec = ms / 1000;
		now.tv_usec = (ms % 1000) * 1000;
		jiffies = timeval_to_jiffies(&now);
		schedule_timeout_interruptible(jiffies);
	}
}
EXPORT_SYMBOL(hw_camdrv_msleep);

int hw_is_fpga_board(void)
{
	cam_debug("%s is_fpga = %d", __func__, g_is_fpga);
	return g_is_fpga;
}
EXPORT_SYMBOL(hw_is_fpga_board);

