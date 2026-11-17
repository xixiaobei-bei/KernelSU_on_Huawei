/*
 * sensor_common.c
 *
 * camera driver source file
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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

#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/thermal.h>
#include <securec.h>
#include "sensor_commom.h"
#include "hw_pmic.h"
#include "huawei_platform/sensor/hw_comm_pmic.h"
#include "../../clt/hisi_clt_flag.h"
#include <huawei_platform/sensor/hw_comm_pmic.h>

#define HIGH_TEMP    70000  /* 70 C  */
#define LOW_TEMP     (-100000) /* -100 C */

#define LPM3_REGISTER_ENABLE  1
#define LPM3_REGISTER_DISABLE 0

static int g_is_fpga; /* default is no fpga */
static unsigned int g_flag;
static atomic_t volatile g_powered = ATOMIC_INIT(0);

extern unsigned int *g_lpm3;

struct power_seq_type_tab {
	const char *seq_name;
	enum sensor_power_seq_type_t seq_type;
};

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
		cam_err("Error getting sensor thermal zone (%s),not yet ready?\n",
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

	if (((temperature < LOW_TEMP) || (temperature > HIGH_TEMP)) &&
		(g_flag == 0)) {
		g_flag = 1;
		cam_err("Abnormal temperature = %d\n", temperature);
	} else {
		g_flag = 0;
	}

	*temp = temperature;
	return rc;
}

int mclk_config(sensor_t *s_ctrl, unsigned int index, unsigned int clk, int on)
{
	int ret;
	/* 2 3 4 sensor index for different clk_isp_snclk */
	bool fsnclk2 = (index == 2 || index == 3 || index == 4);
	struct device *dev = NULL;

	if (!s_ctrl) {
		cam_err("%s invalid parameter\n", __func__);
		return -1;
	}
	dev = s_ctrl->dev;

	cam_info("%s enter.index[%u], clk[%u], on[%d]",
		__func__, index, clk, on);

	/* clk_isp_snclk max value is 48000000 */
	if (clk > 48000000) {
		cam_err("input(index[%d],clk[%d]) is error!\n", index, clk);
		return -1;
	}

	if (on) {
		if (index == 0) {
			s_ctrl->isp_snclk0 =
				devm_clk_get(dev, "clk_isp_snclk0");

			if (IS_ERR_OR_NULL(s_ctrl->isp_snclk0)) {
				dev_err(dev, "could not get snclk0\n");
				ret = PTR_ERR(s_ctrl->isp_snclk0);
				return ret;
			}

			ret = clk_set_rate(s_ctrl->isp_snclk0, clk);
			if (ret < 0) {
				dev_err(dev, "failed set_rate snclk0 rate\n");
				return ret;
			}

			ret = clk_prepare_enable(s_ctrl->isp_snclk0);
			if (ret) {
				dev_err(dev, "cloud not prepare_enalbe clk_isp_snclk0\n");
				return ret;
			}
		} else if (index == 1 || index == 6) {
			s_ctrl->isp_snclk1 =
				devm_clk_get(dev, "clk_isp_snclk1");

			if (IS_ERR_OR_NULL(s_ctrl->isp_snclk1)) {
				dev_err(dev, "could not get snclk1\n");
				ret = PTR_ERR(s_ctrl->isp_snclk1);
				return ret;
			}

			ret = clk_set_rate(s_ctrl->isp_snclk1, clk);
			if (ret < 0) {
				dev_err(dev, "failed set_rate snclk1 rate\n");
				return ret;
			}

			ret = clk_prepare_enable(s_ctrl->isp_snclk1);
			if (ret) {
				dev_err(dev, "cloud not prepare_enalbe clk_isp_snclk1\n");
				return ret;
			}
		} else if (fsnclk2) {
			s_ctrl->isp_snclk2 =
				devm_clk_get(dev, "clk_isp_snclk2");

			if (IS_ERR_OR_NULL(s_ctrl->isp_snclk2)) {
				dev_err(dev, "could not get snclk2\n");
				ret = PTR_ERR(s_ctrl->isp_snclk2);
				return ret;
			}

			ret = clk_set_rate(s_ctrl->isp_snclk2, clk);
			if (ret < 0) {
				dev_err(dev, "failed set_rate snclk2 rate\n");
				return ret;
			}

			ret = clk_prepare_enable(s_ctrl->isp_snclk2);
			if (ret) {
				dev_err(dev, "cloud not prepare_enalbe clk_isp_snclk2\n");
				return ret;
			}
		}
	} else {
		if ((index == 0) && s_ctrl->isp_snclk0) {
			clk_disable_unprepare(s_ctrl->isp_snclk0);
			cam_debug("clk_disable_unprepare snclk0\n");
		} else if (((index == 1) || (index == 6)) &&
			s_ctrl->isp_snclk1) {
			clk_disable_unprepare(s_ctrl->isp_snclk1);
			cam_info("clk_disable_unprepare snclk1\n");
		} else if (fsnclk2 && s_ctrl->isp_snclk2) {
			clk_disable_unprepare(s_ctrl->isp_snclk2);
			cam_info("clk_disable_unprepare snclk2\n");
		}
	}

	return 0;
}

int hw_mclk_config(sensor_t *s_ctrl,
	struct sensor_power_setting *power_setting,
	int state)
{
	int sensor_index;

	cam_debug("%s enter.state:%d!", __func__, state);

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

int hw_sensor_gpio_config(gpio_t pin_type, hwsensor_board_info_t *sensor_info,
	struct sensor_power_setting *power_setting, int state)
{
	int rc;

	if (hisi_is_clt_flag()) {
		cam_debug("%s just return for CLT camera", __func__);
		return 0;
	}

	cam_debug("%s enter, pin_type:%d state:%d delay:%u",
		__func__, pin_type, state, power_setting->delay);

	if (hw_is_fpga_board())
		return 0;

	if (sensor_info->gpios[pin_type].gpio == 0) {
		cam_err("gpio type[%d] is not actived", pin_type);
		return 0; /* skip this */
	}

	rc = gpio_request(sensor_info->gpios[pin_type].gpio, NULL);
	if (rc < 0) {
		cam_err("failed to request gpio[%d]",
			sensor_info->gpios[pin_type].gpio);
		return rc;
	}

	if (pin_type == FSIN) {
		cam_debug("pin_level: %d",
			gpio_get_value(sensor_info->gpios[pin_type].gpio));
		rc = 0;
	} else {
		/* 2 for Val Calculation */
		rc = gpio_direction_output(sensor_info->gpios[pin_type].gpio,
			state ? (power_setting->config_val + 1) % 2 :
			power_setting->config_val);
		if (rc < 0)
			cam_err("failed to control gpio[%d]",
				sensor_info->gpios[pin_type].gpio);
		else
			/* 2 for Val Calculation */
			cam_debug("%s config gpio[%d] output[%d]", __func__,
				sensor_info->gpios[pin_type].gpio,
				(state ? (power_setting->config_val + 1) % 2 :
				power_setting->config_val));
	}

	gpio_free(sensor_info->gpios[pin_type].gpio);

	if (power_setting->delay != 0)
		hw_camdrv_msleep(power_setting->delay);

	return rc;
}

int hw_sensor_ldo_config(ldo_index_t ldo, hwsensor_board_info_t *sensor_info,
	struct sensor_power_setting *power_setting, int state)
{
	int index;
	int rc;
	const char *ldo_names[LDO_MAX] = {
		"dvdd", "dvdd2", "avdd", "avdd2", "vcm", "vcm2",
		"iopw", "misp", "avdd0", "avdd1", "miniisp", "iovdd",
		"oisdrv", "mipiswitch", "afvdd", "drvvdd"};

	cam_debug("%s enter, ldo:%s state:%d", __func__, ldo_names[ldo], state);

	if (hw_is_fpga_board())
		return 0;

	for (index = 0; index < sensor_info->ldo_num; index++) {
		if (!strcmp(sensor_info->ldo[index].supply, ldo_names[ldo]))
			break;
	}

	if (index == sensor_info->ldo_num) {
		cam_err("ldo [%s] is not actived", ldo_names[ldo]);
		return 0; /* skip this */
	}
	if (state == POWER_ON) {
		if (ldo != LDO_IOPW) {
			rc = regulator_bulk_get(sensor_info->dev, 1,
				&sensor_info->ldo[index]);
			if (rc < 0) {
				cam_err("failed to get ldo[%s]",
					ldo_names[ldo]);
				return rc;
			}

			rc = regulator_set_voltage(
				sensor_info->ldo[index].consumer,
				power_setting->config_val,
				power_setting->config_val);
			if (rc < 0) {
				cam_err("failed to set ldo[%s] to %d V",
					ldo_names[ldo],
					power_setting->config_val);
				return rc;
			}
		}
		rc = regulator_bulk_enable(1, &sensor_info->ldo[index]);
		if (rc) {
			cam_err("failed to enable regulators %d\n", rc);
			return rc;
		}
		if (power_setting->delay != 0)
			hw_camdrv_msleep(power_setting->delay);
	} else {
		if (sensor_info->ldo[index].consumer != NULL) {
			rc = regulator_bulk_disable(1,
				&sensor_info->ldo[index]);
			if (rc) {
				cam_err("failed to disable regulators %d\n",
					rc);
				return rc;
			}
			regulator_bulk_free(1, &sensor_info->ldo[index]);
		}
		rc = 0;
	}

	return rc;
}

void hw_sensor_i2c_config(sensor_t *s_ctrl,
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

int hw_sensor_pmic_config(hwsensor_board_info_t *sensor_info,
	struct sensor_power_setting *power_setting, int state)
{
	int rc = 0;
	struct hisi_pmic_ctrl_t *pmic_ctrl = NULL;

	cam_debug("%s enter", __func__);
	cam_debug("%s seq_val=%d, config_val=%d, state=%d",
		__func__, power_setting->seq_val,
		power_setting->config_val, state);

	pmic_ctrl = hisi_get_pmic_ctrl();
	if (pmic_ctrl)
		rc = pmic_ctrl->func_tbl->pmic_seq_config(pmic_ctrl,
			power_setting->seq_val,
			power_setting->config_val,
			state);
	else
		cam_err("hisi_get_pmic_ctrl is null");

	if (power_setting->delay != 0)
		hw_camdrv_msleep(power_setting->delay);

	return rc;
}

int hw_sensor_phy_clk_enable(hwsensor_board_info_t *sensor_info)
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

int hw_sensor_phy_clk_disable(hwsensor_board_info_t *sensor_info)
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

int load_power_config_switch_1(struct sensor_power_setting *power_setting,
	sensor_t *sctrl, int on_off, int *rc)
{
	if (!power_setting || !sctrl)
		return -1;
	switch (power_setting->seq_type) {
	case SENSOR_DVDD:
		cam_debug("%s, seq_type:%u SENSOR_DVDD",
			__func__, power_setting->seq_type);
		*rc = hw_sensor_ldo_config(LDO_DVDD, sctrl->board_info,
			power_setting, on_off);
		break;
	case SENSOR_DVDD2:
		cam_debug("%s, seq_type:%u SENSOR_DVDD2",
			__func__, power_setting->seq_type);
		*rc = hw_sensor_ldo_config(LDO_DVDD2, sctrl->board_info,
			power_setting, on_off);
		if (*rc) {
			cam_err("%s power up procedure error", __func__);
			*rc = 0;
		}
		break;
	case SENSOR_OIS_DRV:
		cam_debug("%s, seq_type:%u SENSOR_OIS_DRV",
			__func__, power_setting->seq_type);
		*rc = hw_sensor_ldo_config(LDO_OISDRV, sctrl->board_info,
			power_setting, on_off);
		break;
	case SENSOR_IOVDD:
		cam_debug("%s, seq_type:%u SENSOR_IOVDD",
			__func__, power_setting->seq_type);
		*rc = hw_sensor_ldo_config(LDO_IOVDD, sctrl->board_info,
			power_setting, on_off);
		break;
	case SENSOR_AVDD:
		cam_debug("%s, seq_type:%u SENSOR_AVDD",
			__func__, power_setting->seq_type);
		*rc = hw_sensor_ldo_config(LDO_AVDD, sctrl->board_info,
			power_setting, on_off);
		break;
	case SENSOR_AVDD2:
		cam_debug("%s, seq_type:%u SENSOR_AVDD2",
			__func__, power_setting->seq_type);
		*rc = hw_sensor_ldo_config(LDO_AVDD2, sctrl->board_info,
			power_setting, on_off);
		if (*rc) {
			cam_err("%s power up procedure error", __func__);
			*rc = 0;
		}
		break;
	case SENSOR_VCM_AVDD:
		cam_debug("%s, seq_type:%u SENSOR_VCM_AVDD",
			__func__, power_setting->seq_type);
		*rc = hw_sensor_ldo_config(LDO_VCM, sctrl->board_info,
			power_setting, on_off);
		break;
	case SENSOR_VCM_AVDD2:
		cam_debug("%s, seq_type:%u SENSOR_VCM_AVDD2",
			__func__, power_setting->seq_type);
		*rc = hw_sensor_ldo_config(LDO_VCM2, sctrl->board_info,
			power_setting, on_off);
		if (*rc) {
			cam_err("%s power up procedure error", __func__);
			*rc = 0;
		}
		break;
	case SENSOR_AVDD0:
		cam_debug("%s, seq_type:%u SENSOR_AVDD0",
			__func__, power_setting->seq_type);
		*rc = hw_sensor_ldo_config(LDO_AVDD0, sctrl->board_info,
			power_setting, on_off);
		break;
	case SENSOR_AVDD1:
		cam_debug("%s, seq_type:%u SENSOR_AVDD1",
			__func__, power_setting->seq_type);
		*rc = hw_sensor_ldo_config(LDO_AVDD1, sctrl->board_info,
			power_setting, on_off);
		break;
	case SENSOR_MISP_VDD:
		cam_debug("%s, seq_type:%u SENSOR_MISP_VDD",
			__func__, power_setting->seq_type);
		*rc = hw_sensor_ldo_config(LDO_MISP, sctrl->board_info,
			power_setting, on_off);
		break;
	case SENSOR_MCLK:
		cam_debug("%s, seq_type:%u SENSOR_MCLK",
			__func__, power_setting->seq_type);
		*rc = hw_mclk_config(sctrl, power_setting, on_off);
		break;
	case SENSOR_I2C:
		if (on_off == POWER_ON) {
			cam_debug("%s, seq_type:%u SENSOR_I2C",
				__func__, power_setting->seq_type);
			hw_sensor_i2c_config(sctrl, power_setting, POWER_ON);
		}
		break;
	case SENSOR_LDO_EN:
		cam_debug("%s, seq_type:%u SENSOR_LDO_EN",
			__func__, power_setting->seq_type);
		*rc = hw_sensor_gpio_config(LDO_EN, sctrl->board_info,
			power_setting, on_off);
		break;
	case SENSOR_AVDD1_EN:
		cam_debug("%s, seq_type:%u SENSOR_AVDD1_EN",
			__func__, power_setting->seq_type);
		*rc = hw_sensor_gpio_config(AVDD1_EN, sctrl->board_info,
			power_setting, on_off);
		break;
	case SENSOR_DVDD0_EN:
		cam_debug("%s, seq_type:%u SENSOR_DVDD0_EN",
			__func__, power_setting->seq_type);
		*rc = hw_sensor_gpio_config(DVDD0_EN, sctrl->board_info,
			power_setting, on_off);
		break;
	case SENSOR_DVDD1_EN:
		cam_debug("%s, seq_type:%u SENSOR_DVDD1_EN",
			__func__, power_setting->seq_type);
		*rc = hw_sensor_gpio_config(DVDD1_EN, sctrl->board_info,
			power_setting, on_off);
		break;
	case SENSOR_IOVDD_EN:
		cam_debug("%s, seq_type:%u SENSOR_IOVDD_EN",
			__func__, power_setting->seq_type);
		*rc = hw_sensor_gpio_config(IOVDD_EN, sctrl->board_info,
			power_setting, on_off);
		break;
	case SENSOR_MISPDCDC_EN:
		cam_debug("%s, seq_type:%u SENSOR_MISPDCDC_EN",
			__func__, power_setting->seq_type);
		*rc = hw_sensor_gpio_config(MISPDCDC_EN, sctrl->board_info,
			power_setting, on_off);
		break;
	default:
		cam_err("%s invalid seq_type %d",
			__func__, power_setting->seq_type);
		return -1;
	}
	return 0;
}

void load_power_config_switch_2(struct sensor_power_setting *power_setting,
	sensor_t *sctrl, int on_off, int *rc)
{
	struct hw_comm_pmic_cfg_t fp_pmic_ldo_set = {0};

	if (!power_setting || !sctrl)
		return;

	switch (power_setting->seq_type) {
	case SENSOR_CHECK_LEVEL:
		if (on_off == POWER_ON) {
			cam_debug("%s, seq_type:%u SENSOR_CHECK_LEVEL",
				__func__, power_setting->seq_type);
			*rc = hw_sensor_gpio_config(FSIN, sctrl->board_info,
				power_setting, POWER_ON);
		}
		break;
	case SENSOR_RST:
		cam_debug("%s, seq_type:%u SENSOR_RST",
			__func__, power_setting->seq_type);
		*rc = hw_sensor_gpio_config(RESETB, sctrl->board_info,
			power_setting, on_off);
		break;
	case SENSOR_PWDN:
		cam_debug("%s, seq_type:%u SENSOR_PWDN",
			__func__, power_setting->seq_type);
		*rc = hw_sensor_gpio_config(PWDN, sctrl->board_info,
			power_setting, on_off);
		break;
	case SENSOR_VCM_PWDN:
		cam_debug("%s, seq_type:%u SENSOR_VCM_PWDN",
			__func__, power_setting->seq_type);
		*rc = hw_sensor_gpio_config(VCM, sctrl->board_info,
			power_setting, on_off);
		break;
	case SENSOR_SUSPEND:
		cam_debug("%s, seq_type:%u SENSOR_SUSPEND",
			__func__, power_setting->seq_type);
		*rc = hw_sensor_gpio_config(SUSPEND, sctrl->board_info,
			power_setting, POWER_OFF);
		break;
	case SENSOR_SUSPEND2:
		cam_debug("%s, seq_type:%u SENSOR_SUSPEND2",
			__func__, power_setting->seq_type);
		*rc = hw_sensor_gpio_config(SUSPEND2, sctrl->board_info,
			power_setting, POWER_OFF);
		break;
	case SENSOR_RST2:
		cam_debug("%s, seq_type:%u SENSOR_RST2",
			__func__, power_setting->seq_type);
		*rc = hw_sensor_gpio_config(RESETB2, sctrl->board_info,
			power_setting, on_off);
		break;
	case SENSOR_OIS:
		cam_debug("%s, seq_type:%u SENSOR_OIS",
			__func__, power_setting->seq_type);
		*rc = hw_sensor_gpio_config(OIS, sctrl->board_info,
			power_setting, on_off);
		break;
	case SENSOR_OIS2:
		cam_debug("%s, seq_type:%u SENSOR_OIS2",
			__func__, power_setting->seq_type);
		*rc = hw_sensor_gpio_config(OIS2, sctrl->board_info,
			power_setting, on_off);
		if (*rc) {
			cam_err("%s power up procedure error", __func__);
			*rc = 0;
		}
		break;
	case SENSOR_PMIC:
		cam_debug("%s, seq_type:%u SENSOR_PMIC",
			__func__, power_setting->seq_type);
		*rc = hw_sensor_pmic_config(sctrl->board_info,
			power_setting, on_off);
		break;
	case SENSOR_PMIC2:
		cam_info("%s, seq_type:%u SENSOR_PMIC2",
			__func__, power_setting->seq_val);
		fp_pmic_ldo_set.pmic_num = 1;
		fp_pmic_ldo_set.pmic_power_type = power_setting->seq_val;
		fp_pmic_ldo_set.pmic_power_voltage = power_setting->config_val;
		fp_pmic_ldo_set.pmic_power_state = on_off;
		*rc = hw_pmic_power_cfg(MAIN_CAM_PMIC_REQ, &fp_pmic_ldo_set);
		if (*rc) {
			cam_err("%s Maybe failed to hw_pmic_power_cfg, rc=%d",
				__func__, *rc);
			*rc = 0;
		};
		mdelay(1);
		cam_info("%s hw_pmic_power_cfg, rc=%d", __func__, *rc);
		break;
	case SENSOR_CS:
		break;
	case SENSOR_AVDD2_EN:
		cam_debug("%s, seq_type:%u SENSOR_AVDD2_EN", __func__,
			power_setting->seq_type);
		*rc = hw_sensor_gpio_config(AVDD2_EN, sctrl->board_info,
			power_setting, on_off);
		break;
	case SENSOR_MIPI_SW:
		cam_info("%s, seq_type:%u SENSOR_MIPI_SW", __func__,
			power_setting->seq_type);
		*rc = hw_sensor_gpio_config(MIPI_SW, sctrl->board_info,
			power_setting, on_off);
		break;
	case SENSOR_LASER_XSHUT:
		cam_info("%s, seq_type:%u SENSOR_LASER_XSHUT", __func__,
			power_setting->seq_type);
		if (sctrl->board_info->gpios[LASER_XSHUT].gpio == 0) {
			cam_err("gpio type[%d] is not actived", LASER_XSHUT);
			*rc = 0;
			break;
		}
		*rc = gpio_direction_output(
			sctrl->board_info->gpios[LASER_XSHUT].gpio,
			on_off);
		if (*rc < 0)
			cam_err("failed to control gpio[%d]",
				sctrl->board_info->gpios[LASER_XSHUT].gpio);
		else
			cam_info("%s config gpio[%d] output[%d]", __func__,
				sctrl->board_info->gpios[LASER_XSHUT].gpio,
				on_off);
		break;
	default:
		cam_err("%s invalid seq_type  %d",
			__func__, power_setting->seq_type);
		break;
	}
}

int hw_sensor_power_config(struct sensor_power_setting *power_setting,
	sensor_t *sctrl, int on_off)
{
	int rc = 0;
	int ret = load_power_config_switch_1(power_setting, sctrl, on_off, &rc);
	if (ret < 0)
		load_power_config_switch_2(power_setting, sctrl, on_off, &rc);
	return rc;
}

int hw_sensor_power_up(sensor_t *s_ctrl)
{
	struct sensor_power_setting_array *power_setting_array = NULL;
	struct sensor_power_setting *power_setting = NULL;
	unsigned int index;
	int rc;
	struct hisi_pmic_ctrl_t *pmic_ctrl = NULL;

	if (hisi_is_clt_flag()) {
		cam_debug("%s just return for CLT camera", __func__);
		return 0;
	}

	cam_debug("%s enter", __func__);
	power_setting_array = &s_ctrl->power_setting_array;

	if (s_ctrl->p_atpowercnt) {
		if (atomic_read(s_ctrl->p_atpowercnt)) {
			cam_debug("%s  %d: sensor has already powered up, p_atpowercnt",
				__func__, __LINE__);
			return 0;
		}
	} else {
		if (atomic_read(&g_powered)) {
			cam_debug("%s  %d: sensor has already powered up",
				__func__, __LINE__);
			return 0;
		}
	}

	/* fpga board compatibility */
	if (hw_is_fpga_board())
		return 0;

	rc = hw_sensor_phy_clk_enable(s_ctrl->board_info);
	if (rc)
		return rc;

	pmic_ctrl = hisi_get_pmic_ctrl();
	if (pmic_ctrl != NULL) {
		cam_debug("pmic power on!");
		pmic_ctrl->func_tbl->pmic_on(pmic_ctrl, 0);
	} else {
		cam_debug("%s pimc ctrl is null", __func__);
	}

	/* lpm3 buck support, do wrtel enable */
	if (s_ctrl->board_info->lpm3_gpu_buck == 1) {
		if (!g_lpm3) {
			cam_info("%s lpm3 is null", __func__);
		} else {
			cam_info("%s need to set LPM3_GPU_BUCK", __func__);
			/* do enable */
			writel(LPM3_REGISTER_ENABLE, g_lpm3);
			cam_info("%s read LPM3_GPU_BUCK is %d",
				__func__, readl(g_lpm3));
		}
	}
	for (index = 0; index < power_setting_array->size; index++) {
		power_setting = &power_setting_array->power_setting[index];

		rc = hw_sensor_power_config(power_setting, s_ctrl, POWER_ON);
		if (rc) {
			cam_err("%s power up procedure error", __func__);
			break;
		}
	}

	if (s_ctrl->p_atpowercnt) {
		atomic_set(s_ctrl->p_atpowercnt, 1);
		cam_debug("%s  %d: sensor powered up finish",
			__func__, __LINE__);
	} else {
		atomic_set(&g_powered, 1);
		cam_debug("%s  %d: sensor powered up finish",
			__func__, __LINE__);
	}

	return rc;
}

int hw_sensor_power_down(sensor_t *s_ctrl)
{
	struct sensor_power_setting_array *power_setting_array = NULL;
	struct sensor_power_setting *power_setting = NULL;
	int index;
	int rc;
	struct hisi_pmic_ctrl_t *pmic_ctrl = NULL;

	if (hisi_is_clt_flag()) {
		cam_debug("%s just return for CLT camera", __func__);
		return 0;
	}

	power_setting_array	= &s_ctrl->power_setting_array;

	cam_debug("%s enter", __func__);

	if (s_ctrl->p_atpowercnt) {
		if (!atomic_read(s_ctrl->p_atpowercnt)) {
			cam_debug("%s %d: sensor hasn't powered up",
				__func__, __LINE__);
			return 0;
		}
	} else {
		if (!atomic_read(&g_powered)) {
			cam_debug("%s  %d: sensor hasn't powered up",
				__func__, __LINE__);
			return 0;
		}
	}

	for (index = (power_setting_array->size - 1); index >= 0; index--) {
		power_setting = &power_setting_array->power_setting[index];
		hw_sensor_power_config(power_setting, s_ctrl, POWER_OFF);
	}

	/* lpm3 buck support, do wrtel disable */
	if (s_ctrl->board_info->lpm3_gpu_buck == 1) {
		if (!g_lpm3) {
			cam_info("%s lpm3 is null", __func__);
		} else {
			cam_info("%s need to clear LPM3_GPU_BUCK", __func__);
			/* do disable */
			writel(LPM3_REGISTER_DISABLE, g_lpm3);
			cam_info("%s read LPM3_GPU_BUCK is %d",
				__func__, readl(g_lpm3));
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

int hw_sensor_power_on(void *psensor)
{
	sensor_t *s_ctrl = NULL;

	if (!psensor) {
		cam_err("%s psensor is NULL!\n", __func__);
		return -1;
	}

	s_ctrl = (sensor_t *)(psensor);
	return hw_sensor_power_up(s_ctrl);
}
EXPORT_SYMBOL(hw_sensor_power_on);

int hw_sensor_power_off(void *psensor)
{
	sensor_t *s_ctrl = NULL;

	if (!psensor) {
		cam_err("%s psensor is NULL!\n", __func__);
		return -1;
	}

	s_ctrl = (sensor_t *)(psensor);
	return hw_sensor_power_down(s_ctrl);
}
EXPORT_SYMBOL(hw_sensor_power_off);

int hw_sensor_get_phy_clk(struct device *pdev,
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

	/* get clk parameters */
	ret = of_property_read_string_array(of_node, "clock-names",
		clk_name, clk_num);
	if (ret < 0) {
		cam_err("[%s] Failed : of_property_read_string_array %d\n",
			__func__, ret);
		return -1;
	}

	for (i = 0; i < clk_num; ++i)
		cam_debug("[%s] clk_name[%d] = %s\n", __func__, i, clk_name[i]);

	for (i = 0; i < clk_num; i++) {
		sensor_info->phy_clk[i] = devm_clk_get(pdev, clk_name[i]);
		if (IS_ERR_OR_NULL(sensor_info->phy_clk[i])) {
			cam_err("[%s] Failed : phyclk.%s.%d.%li\n",
				__func__, clk_name[i], i,
				PTR_ERR(sensor_info->phy_clk[i]));
			return -1;
		}
	}

	sensor_info->phy_clk_num = clk_num;

	return 0;
}

int hw_sensor_get_phy_dt_data(struct device *pdev,
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

int hw_sensor_get_phyinfo_data(struct device_node *of_node,
	hwsensor_board_info_t *sensor_info, int info_count)
{
	int ret;
	int count;

	if (!of_node || !sensor_info) {
		cam_err("%s param is invalid", __func__);
		return -1;
	}
	count = of_property_count_elems_of_size(of_node,
		"huawei,is_master_sensor", sizeof(u32));
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
			(unsigned int *)sensor_info->phyinfo.phy_mode,
			count));

	count = of_property_count_elems_of_size(of_node,
		"huawei,phy_freq_mode", sizeof(u32));
	if (count != info_count) {
		cam_err("%s %d, count = %d", __func__, __LINE__, count);
		return -1;
	}
	ret = (int)((unsigned int)ret |
			(unsigned int)of_property_read_u32_array(of_node,
			"huawei,phy_freq_mode",
			(unsigned int *)sensor_info->phyinfo.phy_freq_mode,
			count));

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

	count = of_property_count_elems_of_size(of_node,
		"huawei,phy_work_mode", sizeof(u32));
	if (count != info_count) {
		cam_err("%s %d, count = %d", __func__, __LINE__, count);
		return -1;
	}
	ret = (int)((unsigned int)ret |
			(unsigned int)of_property_read_u32_array(of_node,
			"huawei,phy_work_mode",
			(unsigned int *)sensor_info->phyinfo.phy_work_mode,
			count));

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

int hw_sensor_get_ext_dt_data_count(struct device_node *of_node,
	hwsensor_board_info_t *sensor_info)
{
	const char *ext_name = NULL;
	unsigned int count = of_property_count_strings(of_node,
		"huawei,ext_name");
	unsigned int i;
	int ret;

	if ((count > 0) && (count <= EXT_NAME_NUM)) {
		for (i = 0; i < count; i++) {
			int rc = of_property_read_string_index(of_node,
				"huawei,ext_name", i, &ext_name);
			if (rc < 0) {
				sensor_info->ext_type = NO_EXT_INFO;
				cam_err("%s get ext_name failed %d\n",
					__func__, __LINE__);
				return -EINVAL;
			}
			ret = strncpy_s(sensor_info->ext_name[i],
				DEVICE_NAME_SIZE - 1,
				ext_name,
				strlen(ext_name));
			if (ret != 0) {
				cam_err("%s strncpy_s return error\n",
					__func__);
				return -EINVAL;
			}
			cam_info("%s ext_name: %s\n", __func__, ext_name);
		}
	} else {
		cam_err("%s ext name num %d out of range\n",
			__func__, count);
		sensor_info->ext_type = NO_EXT_INFO;
	}
	return 0;
}

int hw_sensor_get_ext_dt_data(struct device_node *of_node,
	hwsensor_board_info_t *sensor_info)
{
	int rc;
	const char *ext_name = NULL;
	int ret;

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
	cam_info("%s sensor_info->ext_type %d, rc %d\n",
		__func__, sensor_info->ext_type, rc);

	if (sensor_info->ext_type == EXT_INFO_NO_ADC) {
		rc = of_property_read_string(of_node,
			"huawei,ext_name",
			&ext_name);
		if (rc < 0) {
			cam_err("%s get ext_name failed %d\n",
				__func__, __LINE__);
			sensor_info->ext_type = NO_EXT_INFO;
			return -EINVAL;
		}
		ret = strncpy_s(sensor_info->ext_name[0],
			DEVICE_NAME_SIZE - 1, ext_name, strlen(ext_name));
		if (ret != 0) {
			cam_err("%s strncpy_s return error\n",
				__func__);
			return -EINVAL;
		}
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
		cam_info("%s sensor_info->adc_channel %d, rc %d\n",
			__func__, sensor_info->adc_channel, rc);
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
			cam_err("%s ext threshold num %d out of range\n",
				__func__, sensor_info->ext_num);
			sensor_info->ext_type = NO_EXT_INFO;
		}
		hw_sensor_get_ext_dt_data_count(of_node, sensor_info);
	}

	return 0;
}

int get_dt_data_1(struct device_node *of_node,
	hwsensor_board_info_t *sensor_info,
	int *rc)
{
	int count;

	if (!of_node || !sensor_info)
		return -1;

	*rc = of_property_read_string(of_node, "huawei,sensor_name",
		&sensor_info->name);
	cam_debug("%s huawei,sensor_name %s, rc %d\n", __func__,
		sensor_info->name, *rc);
	if (*rc < 0) {
		cam_err("%s failed %d\n", __func__, __LINE__);
		return -1;
	}

	*rc = of_property_read_u32(of_node, "huawei,is_fpga",
		&g_is_fpga);
	cam_debug("%s huawei,is_fpga: %d, rc %d\n", __func__,
		g_is_fpga, *rc);
	if (*rc < 0) {
		cam_err("%s failed %d\n", __func__, __LINE__);
		return -1;
	}

	*rc = of_property_read_u32(of_node, "huawei,sensor_index",
		(u32 *)(&sensor_info->sensor_index));
	cam_debug("%s huawei,sensor_index %d, rc %d\n", __func__,
		sensor_info->sensor_index, *rc);
	if (*rc < 0) {
		cam_err("%s failed %d\n", __func__, __LINE__);
		return -1;
	}

	*rc = of_property_read_u32(of_node, "huawei,pd_valid",
		&sensor_info->power_conf.pd_valid);
	cam_debug("%s huawei,pd_valid %d, rc %d\n", __func__,
		sensor_info->power_conf.pd_valid, *rc);
	if (*rc < 0) {
		cam_err("%s failed %d\n", __func__, __LINE__);
		return -1;
	}

	*rc = of_property_read_u32(of_node, "huawei,reset_valid",
		&sensor_info->power_conf.reset_valid);
	cam_debug("%s huawei,reset_valid %d, rc %d\n", __func__,
		sensor_info->power_conf.reset_valid, *rc);
	if (*rc < 0) {
		cam_err("%s failed %d\n", __func__, __LINE__);
		return -1;
	}

	*rc = of_property_read_u32(of_node, "huawei,vcmpd_valid",
		&sensor_info->power_conf.vcmpd_valid);
	cam_debug("%s huawei,vcmpd_valid %d, rc %d\n", __func__,
		sensor_info->power_conf.vcmpd_valid, *rc);
	if (*rc < 0) {
		cam_err("%s failed %d\n", __func__, __LINE__);
		return -1;
	}

	/* add csi_index and i2c_index for dual camera. */
	count = of_property_count_elems_of_size(of_node, "huawei,csi_index",
		sizeof(u32));
	if (count > 0) {
		of_property_read_u32_array(of_node, "huawei,csi_index",
			(unsigned int *)&sensor_info->csi_id, count);
	} else {
		sensor_info->csi_id[0] = sensor_info->sensor_index;
		sensor_info->csi_id[1] = -1;
	}
	cam_info("sensor:%s csi_id[0-1]=%d:%d", sensor_info->name,
		sensor_info->csi_id[0], sensor_info->csi_id[1]);

	count = of_property_count_elems_of_size(of_node, "huawei,i2c_index",
		sizeof(u32));
	if (count > 0) {
		of_property_read_u32_array(of_node, "huawei,i2c_index",
			(unsigned int *)&sensor_info->i2c_id, count);
	} else {
		sensor_info->i2c_id[0] = sensor_info->sensor_index;
		sensor_info->i2c_id[1] = -1;
	}
	cam_info("sensor:%s i2c_id[0-1]=%d:%d", sensor_info->name,
		sensor_info->i2c_id[0], sensor_info->i2c_id[1]);

	return 0;
}

int get_dt_data_2(struct platform_device *pdev,
	struct device_node *of_node,
	hwsensor_board_info_t *sensor_info,
	int *rc)
{
	int i;
	char *gpio_tag = NULL;
	int index;
	/* enum gpio_t */
	const char *gpio_ctrl_types[IO_MAX] = {
		"reset", "fsin", "pwdn", "vcm_pwdn", "suspend", "suspend2",
		"reset2", "ldo_en", "ois", "ois2", "dvdd0-en", "dvdd1-en",
		"iovdd-en", "mispdcdc-en", "mipisw", "reset3", "pwdn2",
		"avdd1_en", "avdd2_en", " ", "afvdd_en", "laser_xshut"
	};

	if (!pdev || !of_node || !sensor_info)
		return -1;

	*rc = hw_sensor_get_phy_dt_data(&pdev->dev, of_node, sensor_info);
	if (*rc < 0)
		cam_info("%s phy data not ready %d\n", __func__, __LINE__);

	*rc = of_property_read_u32(of_node, "huawei,mclk",
		&sensor_info->mclk);
	cam_debug("%s huawei,mclk 0x%x, rc %d\n", __func__,
		sensor_info->mclk, *rc);
	if (*rc < 0) {
		cam_err("%s failed %d\n", __func__, __LINE__);
		return -1;
	}

	/* get ldo */
	sensor_info->ldo_num =
		of_property_count_strings(of_node, "huawei,ldo-names");
	if (sensor_info->ldo_num < 0) {
		cam_err("%s failed %d\n", __func__, __LINE__);
	} else {
		cam_debug("ldo num = %d", sensor_info->ldo_num);
		for (i = 0; i < sensor_info->ldo_num; i++) {
			*rc = of_property_read_string_index(of_node,
				"huawei,ldo-names", i,
				&sensor_info->ldo[i].supply);
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
			cam_err("%s failed %d", __func__, __LINE__);
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
	return 0;
}

int get_dt_data_3(struct device_node *of_node,
	hwsensor_board_info_t *sensor_info, int *rc)
{
	if (!of_node || !sensor_info)
		return -1;

	*rc = of_property_read_u32(of_node, "module_type",
		&sensor_info->module_type);
	cam_info("%s module_type 0x%x, rc %d\n", __func__,
		sensor_info->module_type, *rc);
	if (*rc < 0) {
		sensor_info->module_type = 0;
		cam_warn("%s read module_type failed, rc %d, set default value %d\n",
			__func__, *rc, sensor_info->module_type);
		*rc = 0;
	}

	*rc = of_property_read_u32(of_node, "reset_type",
		&sensor_info->reset_type);
	cam_info("%s reset_type 0x%x, rc %d\n", __func__,
		sensor_info->reset_type, *rc);
	if (*rc < 0) {
		sensor_info->reset_type = 0;
		cam_warn("%s read reset_type failed, rc %d, set default value %d\n",
			__func__, *rc, sensor_info->reset_type);
		*rc = 0;
	}

	*rc = of_property_read_u32(of_node, "huawei,topology_type",
		&sensor_info->topology_type);
	cam_info("%s topology_type 0x%x, rc %d\n",
		__func__, sensor_info->topology_type, *rc);
	if (*rc < 0) {
		/* set invalid(-1) as default */
		sensor_info->topology_type = -1;
		cam_warn("%s read topology_type failed, rc %d, set default value %d\n",
			__func__, *rc, sensor_info->topology_type);
		*rc = 0;
	}

	*rc = of_property_read_u32(of_node, "huawei,phyinfo_valid",
		(u32 *)&sensor_info->phyinfo_valid);
	cam_info("%s sensor_info->phyinfo_valid %d, rc %d\n",
		__func__, sensor_info->phyinfo_valid, *rc);
	if (!*rc && (sensor_info->phyinfo_valid == 1 ||
		sensor_info->phyinfo_valid == 2)) /* 1 2 for phyinfo count */
		*rc = hw_sensor_get_phyinfo_data(of_node, sensor_info,
			sensor_info->phyinfo_valid);

	*rc = of_property_read_u32(of_node, "need_rpc",
			&sensor_info->need_rpc);
	cam_info("%s need_rpc 0x%x, rc %d\n", __func__,
		sensor_info->need_rpc, *rc);
	if (*rc < 0) {
		sensor_info->need_rpc = 0;
		cam_warn("%s read need_rpc failed, rc %d, set default value %d\n",
			__func__, *rc, sensor_info->need_rpc);
		*rc = 0;
	}

	sensor_info->lpm3_gpu_buck = 0;
	*rc = of_property_read_u32(of_node, "lpm3_gpu_buck",
		&sensor_info->lpm3_gpu_buck);
	cam_info("%s lpm3_gpu_buck 0x%x, rc %d\n",
		__func__, sensor_info->lpm3_gpu_buck, *rc);
	if (*rc < 0) {
		sensor_info->lpm3_gpu_buck = 0;
		cam_warn("%s read lpm3_gpu_buck failed, rc %d\n",
			__func__, *rc);
		*rc = 0;
	}

	*rc = of_property_read_u32(of_node, "use_power_down_seq",
		&sensor_info->use_power_down_seq);
	cam_info("%s use_power_down_seq 0x%x, rc %d", __func__,
		sensor_info->use_power_down_seq, *rc);
	if (*rc < 0) {
		sensor_info->use_power_down_seq = 0;
		*rc = 0;
	}

	hw_sensor_get_ext_dt_data(of_node, sensor_info);
	return 0;
}

int hw_sensor_get_dt_data(struct platform_device *pdev,
	sensor_t *sensor)
{
	struct device_node *of_node = pdev->dev.of_node;
	hwsensor_board_info_t *sensor_info = NULL;
	int rc = 0;
	int ret;

	cam_debug("enter %s", __func__);
	sensor_info = kzalloc(sizeof(*sensor_info), GFP_KERNEL);
	if (!sensor_info) {
		cam_err("%s failed %d\n", __func__, __LINE__);
		return -ENOMEM;
	}
	sensor->board_info = sensor_info;
	sensor_info->dev = &(pdev->dev);

	ret = get_dt_data_1(of_node, sensor_info, &rc);
	if (ret < 0)
		goto fail;

	/* FPGA IGNORE */
	if (hw_is_fpga_board())
		return rc;

	ret = get_dt_data_2(pdev, of_node, sensor_info, &rc);
	if (ret < 0)
		goto fail;

	ret = get_dt_data_3(of_node, sensor_info, &rc);
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
	const char *seq_name,
	uint32_t *seq_vals,
	uint32_t *cfg_vals,
	uint32_t *sensor_indexs,
	uint32_t *seq_delays)
{
	(*power_settings)[i].seq_val = seq_vals[i];
	(*power_settings)[i].config_val = cfg_vals[i];
	(*power_settings)[i].sensor_index = ((sensor_indexs[i] >= 0xFF) ?
		0xFFFFFFFF : sensor_indexs[i]);
	(*power_settings)[i].delay = seq_delays[i];
	cam_info("%s:%d index[%d] seq_name[%s] seq_type[%d] cfg_vals[%d] seq_delay[%d] sensor_index[0x%x]",
		__func__, __LINE__, i, seq_name,
		(*power_settings)[i].seq_type,
		cfg_vals[i], seq_delays[i],
		sensor_indexs[i]);
	cam_info("%s:%d sensor_index = %d", __func__, __LINE__,
		(*power_settings)[i].sensor_index);
}

static int kzalloc_val(int *rc, int count, int is_power_on,
	struct device_node *dev_node, uint32_t *seq_vals, uint32_t **cfg_vals)
{
	const char *seq_val_name = (is_power_on ?
		"huawei,cam-power-seq-val" :
		"huawei,cam-power-down-seq-val");

	*rc = of_property_read_u32_array(dev_node,
		seq_val_name, seq_vals, count);
	if (*rc < 0)
		cam_warn("%s:%d seq val not to config", __func__, __LINE__);

	*cfg_vals = kzalloc(sizeof(**cfg_vals) * count, GFP_KERNEL);
	if (*cfg_vals == NULL) {
		cam_err("%s:%d failed\n", __func__, __LINE__);
		*rc = -ENOMEM;
		return -1;
	}
	return 0;
}

static int kzalloc_cfg(int *rc, int count,
	int is_power_on,
	struct device_node *dev_node,
	uint32_t *cfg_vals,
	uint32_t **sensor_indexs)
{
	const char *seq_cfg_name = (is_power_on ?
		"huawei,cam-power-seq-cfg-val" :
		"huawei,cam-power-down-seq-cfg-val");
	*rc = of_property_read_u32_array(dev_node,
		seq_cfg_name, cfg_vals, count);
	if (*rc < 0)
		cam_warn("%s:%d seq val not to config", __func__, __LINE__);

	*sensor_indexs = kzalloc(sizeof(**sensor_indexs) * count, GFP_KERNEL);
	if (!(*sensor_indexs)) {
		cam_err("%s failed %d", __func__, __LINE__);
		*rc = -ENOMEM;
		return -1;
	}
	return 0;
}

static int kzalloc_sensor(int *rc, int count,
	int is_power_on,
	struct device_node *dev_node,
	uint32_t *sensor_indexs,
	uint32_t **seq_delays)
{
	const char *seq_sensor_index_name = (is_power_on ?
		"huawei,cam-power-seq-sensor-index" :
		"huawei,cam-power-down-seq-sensor-index");

	*rc = of_property_read_u32_array(dev_node,
		seq_sensor_index_name, sensor_indexs, count);
	if (*rc < 0)
		cam_warn("%s:%d sensor index not to config",
			__func__, __LINE__);

	*seq_delays = kzalloc(sizeof(**seq_delays) * count, GFP_KERNEL);
	if (!(*seq_delays)) {
		cam_err("%s failed %d", __func__, __LINE__);
		*rc = -ENOMEM;
		return -1;
	}
	return 0;
}

static int kzalloc_delay(int *rc, int count,
	int is_power_on,
	struct device_node *dev_node,
	uint32_t *seq_delays,
	struct sensor_power_setting **power_settings)
{
	const char *seq_delay_name = (is_power_on ?
		"huawei,cam-power-seq-delay" :
		"huawei,cam-power-down-seq-delay");

	*rc = of_property_read_u32_array(dev_node,
		seq_delay_name, seq_delays, count);
	if (*rc < 0)
		cam_err("%s:%d seq delay not to config", __func__, __LINE__);

	*power_settings = kzalloc(
		sizeof(**power_settings) * count,
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
	size_t count;
	int seq_size;
	const char *seq_type_name = NULL;
	const char *seq_name = NULL;
	uint32_t *seq_vals = NULL;
	uint32_t *cfg_vals = NULL;
	uint32_t *sensor_indexs = NULL;
	uint32_t *seq_delays = NULL;
	struct sensor_power_setting *power_settings = NULL;
	int tmp;

	cam_info("%s:%d is_power_on = %d",
		__func__, __LINE__, is_power_on);

	seq_type_name = (is_power_on ?
		"huawei,cam-power-seq-type" :
		"huawei,cam-power-down-seq-type");
	tmp = of_property_count_strings(dev_node, seq_type_name);
	if (tmp <= 0) {
		cam_warn("%s:%d power settings not to config",
			__func__, __LINE__);
		return -EINVAL;
	}

	count = tmp;
	seq_vals = kzalloc(sizeof(*seq_vals) * count, GFP_KERNEL);
	if (!seq_vals) {
		cam_err("%s:%d failed\n", __func__, __LINE__);
		return -ENOMEM;
	}

	if (kzalloc_val(&rc, count, is_power_on,
		dev_node, seq_vals, &cfg_vals) == -1)
		goto OUT;

	if (kzalloc_cfg(&rc, count, is_power_on,
		dev_node, cfg_vals, &sensor_indexs) == -1)
		goto OUT;

	if (kzalloc_sensor(&rc, count, is_power_on,
		dev_node, sensor_indexs, &seq_delays) == -1)
		goto OUT;

	if (kzalloc_delay(&rc, count, is_power_on,
		dev_node, seq_delays, &power_settings) == -1)
		goto OUT;

	power_setting_array->power_setting = power_settings;
	power_setting_array->size = (unsigned int)count;

	for (i = 0; i < (int)count; i++) {
		rc = of_property_read_string_index(dev_node,
			seq_type_name, i, &seq_name);
		if (rc < 0) {
			cam_err("%s failed %d\n", __func__, __LINE__);
			goto OUT;
		}

		seq_size = (int)ARRAY_SIZE(g_seq_type_tab);
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

		set_power_settings(i, &power_settings,
			seq_name, seq_vals, cfg_vals,
			sensor_indexs, seq_delays);
	}

OUT:
	if (seq_vals != NULL) {
		kfree(seq_vals);
		seq_vals = NULL;
	}
	if (cfg_vals != NULL) {
		kfree(cfg_vals);
		cfg_vals = NULL;
	}
	if (sensor_indexs != NULL) {
		kfree(sensor_indexs);
		sensor_indexs = NULL;
	}
	if (seq_delays != NULL) {
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
	unsigned long jiffies_now;

	if (ms > 0) {
		now.tv_sec = ms / 1000; /* 1000 for ms */
		now.tv_usec = (ms % 1000) * 1000; /* 1000 for ms */
		jiffies_now = timeval_to_jiffies(&now);
		schedule_timeout_interruptible(jiffies_now);
	}
}
EXPORT_SYMBOL(hw_camdrv_msleep);

int hw_is_fpga_board(void)
{
	cam_debug("%s is_fpga=%d", __func__, g_is_fpga);
	return g_is_fpga;
}
EXPORT_SYMBOL(hw_is_fpga_board);

