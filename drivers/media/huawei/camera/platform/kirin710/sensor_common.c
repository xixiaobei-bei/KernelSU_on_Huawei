/*
 * sensor_common.c
 *
 * common driver code for sensor all.
 *
 * Copyright (c) 2001-2021, Huawei Tech. Co., Ltd. All rights reserved.
 *
 * lixiuhua <aimee.li@hisilicon.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */


#include <linux/delay.h>
#include <linux/clk.h>
#include "sensor_commom.h"
#include "hw_cci.h"
#include "hw_isp_io.h"
#include "hw_pmic.h"
//#include "isp_ops.h"
#include "../../clt/hisi_clt_flag.h"
//lint -save -e529 -e542 -e421 -e501 -e613 -e668
static int is_fpga = 0; //default is no fpga
static atomic_t volatile s_powered = ATOMIC_INIT(0);

extern int snprintf_s(char* strDest, size_t destMax, size_t count, const char* format, ...);

typedef struct __power_seq_type_tab {
    const char* seq_name;
    enum sensor_power_seq_type_t seq_type;
} power_seq_type_tab;


static power_seq_type_tab seq_type_tab[] = {
    {"sensor_suspend", SENSOR_SUSPEND},
    {"sensor_suspend2", SENSOR_SUSPEND2},
    {"sensor_pwdn2", SENSOR_PWDN2},
    {"sensor_pwdn", SENSOR_PWDN},
    {"sensor_rst", SENSOR_RST},
    {"sensor_rst2", SENSOR_RST2},
    {"sensor_rst3", SENSOR_RST3},
    {"sensor_vcm_avdd", SENSOR_VCM_AVDD},
    {"sensor_vcm_avdd2", SENSOR_VCM_AVDD2},
    {"sensor_mipi_sw", SENSOR_MIPI_SW},
    {"sensor_vcm_pwdn", SENSOR_VCM_PWDN},
    {"sensor_avdd", SENSOR_AVDD},
    {"sensor_avdd0", SENSOR_AVDD0},
    {"sensor_avdd1", SENSOR_AVDD1},
    {"sensor_avdd2", SENSOR_AVDD2},
    {"sensor_avdd1_en", SENSOR_AVDD1_EN},
    {"sensor_avdd2_en", SENSOR_AVDD2_EN},
    {"sensor_dvdd", SENSOR_DVDD},
    {"sensor_dvdd2", SENSOR_DVDD2},
    {"sensor_dvdd0_en", SENSOR_DVDD0_EN},
    {"sensor_dvdd1_en", SENSOR_DVDD1_EN},
    {"sensor_iovdd", SENSOR_IOVDD},
    {"sensor_iovdd_en", SENSOR_IOVDD_EN},
    {"sensor_bvdd", SENSOR_BVDD},
    {"sensor_mclk", SENSOR_MCLK},
};
int mclk_config(sensor_t *s_ctrl, unsigned int id, unsigned int clk, int on)
{
    int ret = 0;
    bool fSnclk2 = (2 == id || 3 == id);
    struct device *dev;

    if(NULL == s_ctrl) {
        cam_err("%s invalid parameter.\n", __func__);
        return -1;
    }
    dev = s_ctrl->dev;

    cam_info("%s enter.id(%u), clk(%u), on(%d)", __func__, id, clk, on);



    /* clk_isp_snclk max value is 48000000 */
    if((id > 3) || (clk > 48000000)) {
        cam_err("input(id[%d],clk[%d]) is error!\n", id, clk);
        return -1;
    }

    if(on) {
        if(0 == id) {
            s_ctrl->isp_snclk0 = devm_clk_get(dev, "clk_isp_snclk0");

            if(IS_ERR_OR_NULL(s_ctrl->isp_snclk0)) {
                dev_err(dev, "could not get snclk0.\n");
                ret = PTR_ERR(s_ctrl->isp_snclk0);
                return ret;
            }

            ret = clk_set_rate(s_ctrl->isp_snclk0, clk);
            if(ret < 0) {
                dev_err(dev, "failed set_rate snclk0 rate.\n");
                return ret;
            }

            ret = clk_prepare_enable(s_ctrl->isp_snclk0);
            if(ret) {
                dev_err(dev, "cloud not prepare_enalbe clk_isp_snclk0. \n");
                return ret;
            }
        }else if(1 == id) {
            s_ctrl->isp_snclk1 = devm_clk_get(dev, "clk_isp_snclk1");

            if(IS_ERR_OR_NULL(s_ctrl->isp_snclk1)) {
                dev_err(dev, "could not get snclk1.\n");
                ret = PTR_ERR(s_ctrl->isp_snclk1);
                return ret;
            }

            ret = clk_set_rate(s_ctrl->isp_snclk1, clk);
            if(ret < 0) {
                dev_err(dev, "failed set_rate snclk1 rate.\n");
                return ret;
            }

            ret = clk_prepare_enable(s_ctrl->isp_snclk1);
            if(ret) {
                dev_err(dev, "cloud not prepare_enalbe clk_isp_snclk1. \n");
                return ret;
            }
        }else if(fSnclk2)
        {
            s_ctrl->isp_snclk2 = devm_clk_get(dev, "clk_isp_snclk2");

            if(IS_ERR_OR_NULL(s_ctrl->isp_snclk2)) {
                dev_err(dev, "could not get snclk2.\n");
                ret = PTR_ERR(s_ctrl->isp_snclk2);
                return ret;
            }

            ret = clk_set_rate(s_ctrl->isp_snclk2, clk);
            if(ret < 0) {
                dev_err(dev, "failed set_rate snclk2 rate.\n");
                return ret;
            }

            ret = clk_prepare_enable(s_ctrl->isp_snclk2);
            if(ret) {
                dev_err(dev, "cloud not prepare_enalbe clk_isp_snclk2. \n");
                return ret;
            }
        }
    } else {
        if((0 == id) && (NULL != s_ctrl->isp_snclk0)) {
            clk_disable_unprepare(s_ctrl->isp_snclk0);
            cam_info("clk_disable_unprepare snclk0.\n");
        }else if((1 == id) && (NULL != s_ctrl->isp_snclk1)) {
            clk_disable_unprepare(s_ctrl->isp_snclk1);
            cam_info("clk_disable_unprepare snclk1.\n");
        }else if(fSnclk2 && (NULL != s_ctrl->isp_snclk2)) {
            clk_disable_unprepare(s_ctrl->isp_snclk2);
            cam_info("clk_disable_unprepare snclk2.\n");
        }
    }

    return 0;
}

int hw_mclk_config(sensor_t *s_ctrl,
    struct sensor_power_setting *power_setting, int state)
{
    int sensor_index;

    cam_debug("%s enter.state:%d!", __func__, state);

    if (hw_is_fpga_board()) {
        return 0;
    }

    if (SENSOR_INDEX_INVALID != power_setting->sensor_index) {
        sensor_index = power_setting->sensor_index;
    } else {
        sensor_index = s_ctrl->board_info->sensor_index;
    }

    mclk_config(s_ctrl,sensor_index,
        s_ctrl->board_info->mclk, state);

    if (0 != power_setting->delay) {
        hw_camdrv_msleep(power_setting->delay);
    }

    return 0;
}

int hw_sensor_gpio_config(gpio_t pin_type, hwsensor_board_info_t *sensor_info,
    struct sensor_power_setting *power_setting)
{
    int rc = -1;
    if (hisi_is_clt_flag()) {
        cam_info("%s just return for CLT camera.", __func__);
        return 0;
    }

    cam_debug("%s enter, pin_type:%d  delay:%u", __func__, pin_type, power_setting->delay);

    if (hw_is_fpga_board())
        return 0;

    if(0 == sensor_info->gpios[pin_type].gpio) {
        cam_err("gpio type[%d] is not actived", pin_type);
        return 0;//skip this
    }

    rc = gpio_request(sensor_info->gpios[pin_type].gpio, "CAM");
    if(rc < 0) {
        cam_err("failed to request gpio[%d]", sensor_info->gpios[pin_type].gpio);
        return rc;
    }

    if(pin_type == FSIN) {
        cam_info("pin_level: %d", gpio_get_value(sensor_info->gpios[pin_type].gpio));
        rc = 0;
    } else {
        rc = gpio_direction_output(sensor_info->gpios[pin_type].gpio, power_setting->config_val);
        if(rc < 0) {
            cam_err("failed to control gpio[%d]", sensor_info->gpios[pin_type].gpio);
        } else {
                cam_debug("%s config gpio[%d] output[%d].", __func__,
                 sensor_info->gpios[pin_type].gpio, power_setting->config_val);
        }
    }

    gpio_free(sensor_info->gpios[pin_type].gpio);

    if (0 != power_setting->delay) {
        hw_camdrv_msleep(power_setting->delay);
    }

    return rc;

}

int hw_sensor_ldo_config(ldo_index_t ldo, hwsensor_board_info_t *sensor_info,
    struct sensor_power_setting *power_setting, int state)
{
    int index;
    int rc = -1;
    const char *ldo_names[LDO_MAX]
        = {"dvdd", "dvdd2", "avdd", "avdd2", "vcm", "vcm2", "iopw","misp", "avdd0", "avdd1", "miniisp", "iovdd", "oisdrv", "mipiswitch", "afvdd", "drvvdd", "bvdd"};

    cam_info("%s enter, ldo:%s state:%d", __func__, ldo_names[ldo], state);

    if (hw_is_fpga_board())
        return 0;

    for(index = 0; index < sensor_info->ldo_num; index++) {
        if(!strcmp(sensor_info->ldo[index].supply, ldo_names[ldo])) /*lint !e421 */
            break;
    }

    if(index == sensor_info->ldo_num) {
        cam_err("ldo [%s] is not actived", ldo_names[ldo]);
        return 0;//skip this
    }
    if (POWER_ON == state) {
        if((LDO_IOPW != ldo)) {
            rc = regulator_set_voltage(sensor_info->ldo[index].consumer, power_setting->config_val, power_setting->config_val);
            if(rc < 0) {
                cam_err("failed to set ldo[%s] to %d V", ldo_names[ldo], power_setting->config_val);
                return rc;
            }
        }
        rc = regulator_bulk_enable(1, &sensor_info->ldo[index]);
        if (rc) {
            cam_err("failed to enable regulators %d\n", rc);
            return rc;
        }
        if (0 != power_setting->delay) {
            hw_camdrv_msleep(power_setting->delay);
        }
    } else {
        rc = regulator_bulk_disable(1, &sensor_info->ldo[index]);
        if (rc) {
            cam_err("failed to disable regulators %d\n", rc);
            return rc;
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

    if (POWER_ON == state) {
        hw_isp_config_i2c(&s_ctrl->board_info->i2c_config);

        if (0 != power_setting->delay) {
            hw_camdrv_msleep(power_setting->delay);
        }
    }

    return;
}

extern struct hisi_pmic_ctrl_t ncp6925_ctrl;
int hw_sensor_pmic_config(hwsensor_board_info_t *sensor_info,
    struct sensor_power_setting *power_setting, int state)
{
    int rc = 0;
    cam_debug("%s enter.", __func__);
    cam_debug("%s seq_val=%d, config_val=%d, state=%d",
        __func__, power_setting->seq_val, power_setting->config_val, state);
    //todo ...
    //use wangzhengyong pmic interface
    if (ncp6925_ctrl.func_tbl->pmic_seq_config) {
    rc = ncp6925_ctrl.func_tbl->pmic_seq_config(&ncp6925_ctrl, (pmic_seq_index_t)power_setting->seq_val, power_setting->config_val, state);
    }

    if (0 != power_setting->delay) {
        hw_camdrv_msleep(power_setting->delay);
    }

    return rc;
}

int hw_sensor_power_up(sensor_t *s_ctrl)
{
    int rc = 0;
    unsigned int index = 0;
    struct sensor_power_setting_array *power_setting_array = NULL;
    struct sensor_power_setting *power_setting = NULL;
    struct hisi_pmic_ctrl_t *pmic_ctrl = NULL;
    if (hisi_is_clt_flag()) {
        cam_info("%s just return for CLT camera.", __func__);
        return 0;
    }
    if (NULL == s_ctrl->power_setting_array.power_setting || 0 == s_ctrl->power_setting_array.size){
        cam_err("%s invalid power up setting.", __func__);
        return rc;
    }

    power_setting_array = &s_ctrl->power_setting_array;
    cam_debug("%s enter.", __func__);

    if (s_ctrl->p_atpowercnt){
        if (atomic_read(s_ctrl->p_atpowercnt)) {
            cam_info("%s (%d): sensor has already powered up, p_atpowercnt.", __func__, __LINE__);
            return 0;
        }
    }else{
        if (atomic_read(&s_powered)) {
            cam_info("%s (%d): sensor has already powered up.", __func__, __LINE__);
            return 0;
        }
    }

    /* fpga board compatibility */
    if (hw_is_fpga_board()) {
        return 0;
    }

    pmic_ctrl = hisi_get_pmic_ctrl();
    if(pmic_ctrl != NULL) {
        cam_info("pmic power on!");
        pmic_ctrl->func_tbl->pmic_on(pmic_ctrl, 0);
    } else {
        cam_info("%s pimc ctrl is null.", __func__);
    }

    for (index = 0; index < power_setting_array->size; index++) {
        power_setting = &power_setting_array->power_setting[index];
        switch(power_setting->seq_type) {
        case SENSOR_BVDD:
            cam_info("%s, seq_type:%u SENSOR_BVDD", __func__, power_setting->seq_type);
            rc = hw_sensor_ldo_config(LDO_BVDD, s_ctrl->board_info,
                power_setting, POWER_ON);
            break;
        case SENSOR_DVDD:
            cam_info("%s, seq_type:%u SENSOR_DVDD", __func__, power_setting->seq_type);
            rc = hw_sensor_ldo_config(LDO_DVDD, s_ctrl->board_info,
                power_setting, POWER_ON);
            break;
        case SENSOR_DVDD2:
            cam_info("%s, seq_type:%u SENSOR_DVDD2", __func__, power_setting->seq_type);
            rc = hw_sensor_ldo_config(LDO_DVDD2, s_ctrl->board_info,
                power_setting, POWER_ON);
            if(rc) {
                cam_err("%s power up procedure error.", __func__);
                rc = 0;
            }
            break;
        case SENSOR_OIS_DRV:
            cam_info("%s, seq_type:%u SENSOR_OIS_DRV", __func__, power_setting->seq_type);
            rc = hw_sensor_ldo_config(LDO_OISDRV, s_ctrl->board_info,
                power_setting, POWER_ON);
            break;
        case SENSOR_IOVDD:
            cam_info("%s, seq_type:%u SENSOR_IOVDD", __func__, power_setting->seq_type);
            rc = hw_sensor_ldo_config(LDO_IOVDD, s_ctrl->board_info,
                power_setting, POWER_ON);
            break;
        case SENSOR_AVDD:
            cam_info("%s, seq_type:%u SENSOR_AVDD", __func__, power_setting->seq_type);
            rc = hw_sensor_ldo_config(LDO_AVDD, s_ctrl->board_info,
                power_setting, POWER_ON);
            break;
        case SENSOR_AVDD2:
            cam_info("%s, seq_type:%u SENSOR_AVDD2", __func__, power_setting->seq_type);
            rc = hw_sensor_ldo_config(LDO_AVDD2, s_ctrl->board_info,
                power_setting, POWER_ON);
            if(rc) {
                cam_err("%s power up procedure error.", __func__);
                rc = 0;
            }
            break;
        case SENSOR_VCM_AVDD:
            cam_info("%s, seq_type:%u SENSOR_VCM_AVDD", __func__, power_setting->seq_type);
            rc = hw_sensor_ldo_config(LDO_VCM, s_ctrl->board_info,
                power_setting, POWER_ON);
            break;
        case SENSOR_VCM_AVDD2:
            cam_info("%s, seq_type:%u SENSOR_VCM_AVDD2", __func__, power_setting->seq_type);
            rc = hw_sensor_ldo_config(LDO_VCM2, s_ctrl->board_info,
                power_setting, POWER_ON);
            if(rc) {
                cam_err("%s power up procedure error.", __func__);
                rc = 0;
            }
            break;
        case SENSOR_AVDD0:
            cam_info("%s, seq_type:%u SENSOR_AVDD0", __func__, power_setting->seq_type);
            rc = hw_sensor_ldo_config(LDO_AVDD0, s_ctrl->board_info,
                power_setting, POWER_ON);
            break;
        case SENSOR_AVDD1:
            cam_info("%s, seq_type:%u SENSOR_AVDD1", __func__, power_setting->seq_type);
            rc = hw_sensor_ldo_config(LDO_AVDD1, s_ctrl->board_info,
                power_setting, POWER_ON);
            break;
        case SENSOR_MISP_VDD:
            cam_info("%s, seq_type:%u SENSOR_MISP_VDD", __func__, power_setting->seq_type);
            rc = hw_sensor_ldo_config(LDO_MISP, s_ctrl->board_info,
                power_setting, POWER_ON);
            break;
        case SENSOR_MIPI_SW:
            cam_info("%s, seq_type:%u SENSOR_MIPI_SW", __func__, power_setting->seq_type);
            rc = hw_sensor_gpio_config(MIPI_SW, s_ctrl->board_info, power_setting);
            break;
        case SENSOR_MIPI_LDO_EN:
            cam_info("%s, seq_type:%u SENSOR_MIPI_LDO_EN", __func__, power_setting->seq_type);
            rc = hw_sensor_ldo_config(LDO_MIPI_SW_EN, s_ctrl->board_info,
                power_setting, POWER_ON);
            break;
        case SENSOR_MCLK:
            cam_info("%s, seq_type:%u SENSOR_MCLK", __func__, power_setting->seq_type);
            rc = hw_mclk_config(s_ctrl, power_setting, POWER_ON);
            break;
        case SENSOR_I2C:
            cam_debug("%s, seq_type:%u SENSOR_I2C", __func__, power_setting->seq_type);
            hw_sensor_i2c_config(s_ctrl, power_setting, POWER_ON);
            break;

        case SENSOR_LDO_EN:
            cam_info("%s, seq_type:%u SENSOR_LDO_EN", __func__, power_setting->seq_type);
            rc = hw_sensor_gpio_config(LDO_EN, s_ctrl->board_info, power_setting);
            break;
        case SENSOR_AVDD1_EN:
            cam_info("%s, seq_type:%u SENSOR_AVDD1_EN", __func__, power_setting->seq_type);
            rc = hw_sensor_gpio_config(AVDD1_EN, s_ctrl->board_info, power_setting);
            break;
        case SENSOR_DVDD0_EN:
            cam_info("%s, seq_type:%u SENSOR_DVDD0_EN", __func__, power_setting->seq_type);
            rc = hw_sensor_gpio_config(DVDD0_EN, s_ctrl->board_info, power_setting);
            break;
        case SENSOR_DVDD1_EN:
            cam_info("%s, seq_type:%u SENSOR_DVDD1_EN", __func__, power_setting->seq_type);
            rc = hw_sensor_gpio_config(DVDD1_EN, s_ctrl->board_info, power_setting);
            break;
        case SENSOR_IOVDD_EN:
            cam_info("%s, seq_type:%u SENSOR_IOVDD_EN", __func__, power_setting->seq_type);
            rc = hw_sensor_gpio_config(IOVDD_EN, s_ctrl->board_info, power_setting);
            break;
        case SENSOR_MISPDCDC_EN:
            cam_info("%s, seq_type:%u SENSOR_MISPDCDC_EN", __func__, power_setting->seq_type);
            rc = hw_sensor_gpio_config(MISPDCDC_EN, s_ctrl->board_info, power_setting);
            break;
        case SENSOR_CHECK_LEVEL:
            cam_debug("%s, seq_type:%u SENSOR_CHECK_LEVEL", __func__, power_setting->seq_type);
            rc = hw_sensor_gpio_config(FSIN, s_ctrl->board_info, power_setting);
            break;
        case SENSOR_RST:
            cam_info("%s, seq_type:%u SENSOR_RST", __func__, power_setting->seq_type);
            rc = hw_sensor_gpio_config(RESETB, s_ctrl->board_info, power_setting);
            break;
        case SENSOR_PWDN:
            cam_debug("%s, seq_type:%u SENSOR_PWDN", __func__, power_setting->seq_type);
            rc = hw_sensor_gpio_config(PWDN, s_ctrl->board_info, power_setting);
            break;
        case SENSOR_PWDN2:
            cam_debug("%s, seq_type:%u SENSOR_PWDN2", __func__, power_setting->seq_type);
            rc = hw_sensor_gpio_config(PWDN2, s_ctrl->board_info, power_setting);
            break;
        case SENSOR_VCM_PWDN:
            cam_debug("%s, seq_type:%u SENSOR_VCM_PWDN", __func__, power_setting->seq_type);
            rc = hw_sensor_gpio_config(VCM, s_ctrl->board_info, power_setting);
            break;
        case SENSOR_SUSPEND:
            cam_info("%s, seq_type:%u SENSOR_SUSPEND", __func__, power_setting->seq_type);
            rc = hw_sensor_gpio_config(SUSPEND, s_ctrl->board_info, power_setting);
            break;
        case SENSOR_SUSPEND2:
            cam_info("%s, seq_type:%u SENSOR_SUSPEND2", __func__, power_setting->seq_type);
            rc = hw_sensor_gpio_config(SUSPEND2, s_ctrl->board_info, power_setting);
            break;
        case SENSOR_RST2:
            cam_info("%s, seq_type:%u SENSOR_RST2", __func__, power_setting->seq_type);
            rc = hw_sensor_gpio_config(RESETB2, s_ctrl->board_info, power_setting);
            break;
        case SENSOR_RST3:
            cam_info("%s, seq_type:%u SENSOR_RST3", __func__, power_setting->seq_type);
            rc = hw_sensor_gpio_config(RESETB3, s_ctrl->board_info, power_setting);
            break;
        case SENSOR_OIS:
            cam_info("%s, seq_type:%u SENSOR_OIS", __func__, power_setting->seq_type);
            rc = hw_sensor_gpio_config(OIS, s_ctrl->board_info, power_setting);
            break;
        case SENSOR_OIS2:
            cam_info("%s, seq_type:%u SENSOR_OIS2", __func__, power_setting->seq_type);
            rc = hw_sensor_gpio_config(OIS2, s_ctrl->board_info, power_setting);
            if(rc) {
                cam_err("%s power up procedure error.", __func__);
                rc = 0;
            }
            break;
        case SENSOR_PMIC:
            cam_info("%s, seq_type:%u SENSOR_PMIC", __func__, power_setting->seq_type);
            rc = hw_sensor_pmic_config(s_ctrl->board_info,
                power_setting, POWER_ON);
            break;
        case SENSOR_AFVDD:
            cam_info("%s, seq_type:%u SENSOR_AFVDD", __func__, power_setting->seq_type);
            rc = hw_sensor_ldo_config(LDO_AFVDD, s_ctrl->board_info,
                power_setting, POWER_ON);
            break;
        case SENSOR_DRVVDD:
            cam_info("%s, seq_type:%u SENSOR_DRVVDD", __func__, power_setting->seq_type);
            rc = hw_sensor_ldo_config(LDO_DRVVDD, s_ctrl->board_info,
                power_setting, POWER_ON);
            break;
        case SENSOR_AFVDD_EN:
            cam_info("%s, seq_type:%u SENSOR_AFVDD_EN", __func__, power_setting->seq_type);
            rc = hw_sensor_gpio_config(AFVDD_EN, s_ctrl->board_info, power_setting);
            break;
        case SENSOR_CS:
            break;
        default:
            cam_err("%s invalid seq_type.", __func__);
            break;
        }

        if(rc) {
            cam_err("%s power up procedure error.", __func__);
            break;
        }
    }

    if (s_ctrl->p_atpowercnt){
        atomic_set(s_ctrl->p_atpowercnt, 1);
        cam_info("%s (%d): sensor  powered up finish", __func__, __LINE__);
    }else{
        atomic_set(&s_powered, 1);
        cam_info("%s (%d): sensor  powered up finish", __func__, __LINE__);
    }

    return rc;
}

int hw_sensor_power_down(sensor_t *s_ctrl)
{
    unsigned int index = 0;
    int rc = 0;
    struct sensor_power_setting_array *power_setting_array = NULL;
    struct sensor_power_setting *power_setting = NULL;
    struct hisi_pmic_ctrl_t *pmic_ctrl = NULL;
    if (NULL == s_ctrl->power_down_setting_array.power_setting || 0 == s_ctrl->power_down_setting_array.size){
        cam_err("%s invalid power down setting.", __func__);
        return rc;
    }
    if (hisi_is_clt_flag()) {
        cam_info("%s just return for CLT camera.", __func__);
        return 0;
    }
    power_setting_array = &s_ctrl->power_down_setting_array;

    cam_debug("%s enter.", __func__);

    if (s_ctrl->p_atpowercnt){
        if (!atomic_read(s_ctrl->p_atpowercnt)) {
            cam_info("%s(%d): sensor hasn't powered up.", __func__, __LINE__);
            return 0;
        }
    }else{
        if (!atomic_read(&s_powered)) {
            cam_info("%s (%d): sensor hasn't powered up.", __func__, __LINE__);
            return 0;
        }
    }

    for (index = 0; index < power_setting_array->size; index++) {
        power_setting = &power_setting_array->power_setting[index];
        switch(power_setting->seq_type) {
        case SENSOR_BVDD:
            cam_info("%s, seq_type:%u SENSOR_BVDD", __func__, power_setting->seq_type);
            rc = hw_sensor_ldo_config(LDO_BVDD, s_ctrl->board_info,
                power_setting, POWER_ON);
            break;
        case SENSOR_DVDD:
            cam_info("%s, seq_type:%u SENSOR_DVDD", __func__, power_setting->seq_type);
            rc = hw_sensor_ldo_config(LDO_DVDD, s_ctrl->board_info,
                power_setting, POWER_OFF);
            break;
        case SENSOR_DVDD2:
            cam_info("%s, seq_type:%u SENSOR_DVDD2", __func__, power_setting->seq_type);
            rc = hw_sensor_ldo_config(LDO_DVDD2, s_ctrl->board_info,
                power_setting, POWER_OFF);
            if(rc) {
                cam_err("%s power up procedure error.", __func__);
                rc = 0;
            }
            break;
        case SENSOR_OIS_DRV:
            cam_info("%s, seq_type:%u SENSOR_OIS_DRV", __func__, power_setting->seq_type);
            rc = hw_sensor_ldo_config(LDO_OISDRV, s_ctrl->board_info,
                power_setting, POWER_OFF);
            break;
        case SENSOR_IOVDD:
            cam_info("%s, seq_type:%u SENSOR_IOVDD", __func__, power_setting->seq_type);
            rc = hw_sensor_ldo_config(LDO_IOVDD, s_ctrl->board_info,
                power_setting, POWER_OFF);
            break;
        case SENSOR_AVDD:
            cam_info("%s, seq_type:%u SENSOR_AVDD", __func__, power_setting->seq_type);
            rc = hw_sensor_ldo_config(LDO_AVDD, s_ctrl->board_info,
                power_setting, POWER_OFF);
            break;
        case SENSOR_AVDD2:
            cam_info("%s, seq_type:%u SENSOR_AVDD2", __func__, power_setting->seq_type);
            rc = hw_sensor_ldo_config(LDO_AVDD2, s_ctrl->board_info,
                power_setting, POWER_OFF);
            if(rc) {
                cam_err("%s power up procedure error.", __func__);
                rc = 0;
            }
            break;
        case SENSOR_VCM_AVDD:
            cam_info("%s, seq_type:%u SENSOR_VCM_AVDD", __func__, power_setting->seq_type);
            rc = hw_sensor_ldo_config(LDO_VCM, s_ctrl->board_info,
                power_setting, POWER_OFF);
            break;
        case SENSOR_VCM_AVDD2:
            cam_info("%s, seq_type:%u SENSOR_VCM_AVDD2", __func__, power_setting->seq_type);
            rc = hw_sensor_ldo_config(LDO_VCM2, s_ctrl->board_info,
                power_setting, POWER_OFF);
            if(rc) {
                cam_err("%s power up procedure error.", __func__);
                rc = 0;
            }
            break;
        case SENSOR_MIPI_SW:
            cam_info("%s, seq_type:%u SENSOR_MIPI_SW", __func__, power_setting->seq_type);
            rc = hw_sensor_gpio_config(MIPI_SW, s_ctrl->board_info, power_setting);
            break;
        case SENSOR_MIPI_LDO_EN:
            cam_info("%s, seq_type:%u SENSOR_MIPI_LDO_EN", __func__, power_setting->seq_type);
            rc = hw_sensor_ldo_config(LDO_MIPI_SW_EN, s_ctrl->board_info,
                power_setting, POWER_OFF);
            break;
        case SENSOR_MCLK:
            cam_info("%s, seq_type:%u SENSOR_MCLK", __func__, power_setting->seq_type);
            hw_mclk_config(s_ctrl, power_setting, POWER_OFF);
            break;
        case SENSOR_I2C:
            break;

        case SENSOR_CHECK_LEVEL:
            break;
        case SENSOR_PWDN:
            cam_debug("%s, seq_type:%u SENSOR_PWDN", __func__, power_setting->seq_type);
            rc = hw_sensor_gpio_config(PWDN, s_ctrl->board_info, power_setting);
            break;
        case SENSOR_RST:
            cam_info("%s, seq_type:%u SENSOR_RST", __func__, power_setting->seq_type);
            rc = hw_sensor_gpio_config(RESETB, s_ctrl->board_info, power_setting);
            break;
        case SENSOR_PWDN2:
            cam_debug("%s, seq_type:%u SENSOR_PWDN2", __func__, power_setting->seq_type);
            rc = hw_sensor_gpio_config(PWDN2, s_ctrl->board_info,
                power_setting);
            break;
        case SENSOR_VCM_PWDN:
            cam_debug("%s, seq_type:%u SENSOR_VCM_PWDN", __func__, power_setting->seq_type);
            rc = hw_sensor_gpio_config(VCM, s_ctrl->board_info, power_setting);
            break;
        case SENSOR_SUSPEND:
            cam_info("%s, seq_type:%u SENSOR_SUSPEND", __func__, power_setting->seq_type);
            rc = hw_sensor_gpio_config(SUSPEND, s_ctrl->board_info, power_setting);
            break;
        case SENSOR_SUSPEND2:
            cam_info("%s, seq_type:%u SENSOR_SUSPEND2", __func__, power_setting->seq_type);
            rc = hw_sensor_gpio_config(SUSPEND2, s_ctrl->board_info, power_setting);
            break;
        case SENSOR_RST2:
            cam_info("%s, seq_type:%u SENSOR_RST2", __func__, power_setting->seq_type);
            rc = hw_sensor_gpio_config(RESETB2, s_ctrl->board_info, power_setting);
            break;
        case SENSOR_RST3:
            cam_info("%s, seq_type:%u SENSOR_RST3", __func__, power_setting->seq_type);
            rc = hw_sensor_gpio_config(RESETB3, s_ctrl->board_info, power_setting);
            break;
        case SENSOR_OIS:
            cam_info("%s, seq_type:%u SENSOR_OIS", __func__, power_setting->seq_type);
            rc = hw_sensor_gpio_config(OIS, s_ctrl->board_info, power_setting);
            break;
        case SENSOR_OIS2:
            cam_info("%s, seq_type:%u SENSOR_OIS2", __func__, power_setting->seq_type);
            rc = hw_sensor_gpio_config(OIS2, s_ctrl->board_info, power_setting);
            if(rc) {
                cam_err("%s power up procedure error.", __func__);
                rc = 0;
            }
            break;
        case SENSOR_PMIC:
            cam_info("%s, seq_type:%u SENSOR_PMIC", __func__, power_setting->seq_type);
            rc = hw_sensor_pmic_config(s_ctrl->board_info,
                power_setting, POWER_OFF);
            break;
        case SENSOR_CS:
            break;
        case SENSOR_LDO_EN:
            cam_info("%s, seq_type:%u SENSOR_LDO_EN", __func__, power_setting->seq_type);
            rc = hw_sensor_gpio_config(LDO_EN, s_ctrl->board_info, power_setting);
            break;

        case SENSOR_DVDD0_EN:
            cam_info("%s, seq_type:%u SENSOR_DVDD0_EN", __func__, power_setting->seq_type);
            rc = hw_sensor_gpio_config(DVDD0_EN, s_ctrl->board_info, power_setting);
            break;
        case SENSOR_DVDD1_EN:
            cam_info("%s, seq_type:%u SENSOR_DVDD1_EN", __func__, power_setting->seq_type);
            rc = hw_sensor_gpio_config(DVDD1_EN, s_ctrl->board_info, power_setting);
            break;
        case SENSOR_IOVDD_EN:
            cam_info("%s, seq_type:%u SENSOR_IOVDD_EN", __func__, power_setting->seq_type);
            rc = hw_sensor_gpio_config(IOVDD_EN, s_ctrl->board_info, power_setting);
            break;
        case SENSOR_MISPDCDC_EN:
            cam_info("%s, seq_type:%u SENSOR_MISPDCDC_EN", __func__, power_setting->seq_type);
            rc = hw_sensor_gpio_config(MISPDCDC_EN, s_ctrl->board_info, power_setting);
            break;
        case SENSOR_AVDD0:
            cam_info("%s, seq_type:%u SENSOR_AVDD0", __func__, power_setting->seq_type);
            rc = hw_sensor_ldo_config(LDO_AVDD0, s_ctrl->board_info,
                power_setting, POWER_OFF);
            break;
        case SENSOR_AVDD1:
            cam_info("%s, seq_type:%u SENSOR_AVDD1", __func__, power_setting->seq_type);
            rc = hw_sensor_ldo_config(LDO_AVDD1, s_ctrl->board_info,
                power_setting, POWER_OFF);
            break;
         case SENSOR_MISP_VDD:
            cam_info("%s, seq_type:%u SENSOR_MISP_VDD", __func__, power_setting->seq_type);
            rc = hw_sensor_ldo_config(LDO_MISP, s_ctrl->board_info,
                power_setting, POWER_OFF);
            break;
         case SENSOR_AFVDD:
            cam_info("%s, seq_type:%u SENSOR_AFVDD", __func__, power_setting->seq_type);
            rc = hw_sensor_ldo_config(LDO_AFVDD, s_ctrl->board_info,
                power_setting, POWER_OFF);
            break;
         case SENSOR_DRVVDD:
            cam_info("%s, seq_type:%u SENSOR_DRVVDD", __func__, power_setting->seq_type);
            rc = hw_sensor_ldo_config(LDO_DRVVDD, s_ctrl->board_info,
                power_setting, POWER_OFF);
            break;
         case SENSOR_AFVDD_EN:
            cam_info("%s, seq_type:%u SENSOR_AFVDD_EN", __func__, power_setting->seq_type);
            rc = hw_sensor_gpio_config(AFVDD_EN, s_ctrl->board_info, power_setting);
            break;
         case SENSOR_AVDD1_EN:
            cam_info("%s, seq_type:%u SENSOR_AVDD1_EN", __func__, power_setting->seq_type);
            rc = hw_sensor_gpio_config(AVDD1_EN, s_ctrl->board_info, power_setting);
            break;
        default:
            cam_err("%s invalid seq_type.", __func__);
            break;
        }

    }

    pmic_ctrl = hisi_get_pmic_ctrl();
    if(pmic_ctrl != NULL) {
        pmic_ctrl->func_tbl->pmic_off(pmic_ctrl);
    }

    if (s_ctrl->p_atpowercnt){
        atomic_set(s_ctrl->p_atpowercnt, 0); /*lint !e1058 !e446 */
    }else{
        atomic_set(&s_powered, 0); /*lint !e1058 !e446 */
    }

    return rc;
}

int hw_sensor_power_on(void *psensor)
{
    sensor_t *s_ctrl = NULL;

    if (NULL == psensor) {
        cam_err("%s psensor is NULL!.\n", __func__);
        return -1;
    }

    s_ctrl = (sensor_t*)(psensor);
    return hw_sensor_power_up(s_ctrl);
}
EXPORT_SYMBOL(hw_sensor_power_on);

int hw_sensor_power_off(void *psensor)
{
    sensor_t *s_ctrl = NULL;

    if (NULL == psensor) {
        cam_err("%s psensor is NULL!.\n", __func__);
        return -1;
    }

    s_ctrl = (sensor_t*)(psensor);
    return hw_sensor_power_down(s_ctrl);
}
EXPORT_SYMBOL(hw_sensor_power_off);
int hw_sensor_i2c_read(sensor_t *s_ctrl, void *data)
{
    struct sensor_cfg_data *cdata = (struct sensor_cfg_data *)data;
    long   rc = 0;
    unsigned int reg, *val;

    cam_debug("%s: address=0x%x\n", __func__, cdata->cfg.reg.subaddr);

    /* parse the I2C parameters */
    reg = cdata->cfg.reg.subaddr;
    val = &cdata->cfg.reg.value;

    rc = hw_isp_read_sensor_byte(&s_ctrl->board_info->i2c_config, reg, (u16 *)val);

    return rc;
}

int hw_sensor_i2c_write(sensor_t *s_ctrl, void *data)
{
    struct sensor_cfg_data *cdata = (struct sensor_cfg_data *)data;
    long   rc = 0;
    unsigned int reg, val, mask;

    cam_debug("%s enter.\n", __func__);

    cam_debug("%s: address=0x%x, value=0x%x\n", __func__,
        cdata->cfg.reg.subaddr, cdata->cfg.reg.value);

    /* parse the I2C parameters */
    reg = cdata->cfg.reg.subaddr;
    val = cdata->cfg.reg.value;
    mask = cdata->cfg.reg.mask;

    rc = hw_isp_write_sensor_byte(&s_ctrl->board_info->i2c_config, reg, val, mask);

    return rc;
}


int hwsensor_writefile(int index, const char *sensor_name)
{
    mm_segment_t fs;
    struct file *filp = NULL;
    char file_name[FILE_NAME_LEN]={0};
    char data[FILE_NAME_LEN]={0};
    int rc=0;
    int pos = index;

    if (index == CAMERA_SENSOR_INVALID)
        return -1;
    cam_info("%s index=%d,sensor_name=%s.\n", __func__, index, sensor_name);

    snprintf_s(file_name, FILE_NAME_LEN, FILE_NAME_LEN-1, "/data/vendor/camera/hisi_sensor%d", index);

    filp = filp_open(file_name, O_CREAT|O_WRONLY, 0666);
    if (IS_ERR_OR_NULL(filp)) {
        rc = -EFAULT;
        cam_err("%s, fail to open file.\n", __func__);
        return rc;
    }

    fs = get_fs();
    set_fs(KERNEL_DS); /*lint !e501 */

    if (sizeof(pos) != vfs_write(filp, (char*)&pos, sizeof(pos), &filp->f_pos)) { /*lint !e613 */
        rc = -EFAULT;
        cam_err("%s, fail to write pos into file.\n", __func__);
        goto fail;
    }

    snprintf_s(data, FILE_NAME_LEN, FILE_NAME_LEN-1, ":%s", sensor_name);
    if (sizeof(data) != vfs_write(filp, data, sizeof(data), &filp->f_pos)) { /*lint !e613 */
        rc = -EFAULT;
        cam_err("%s, fail to write sensor name into file.\n", __func__);
    }

fail:
    set_fs(fs);
    filp_close(filp, NULL); /*lint !e668 */

    return rc;
}
int hw_sensor_power_up_config(struct device *dev, hwsensor_board_info_t *sensor_info)
{
    int rc = 0;
    rc = devm_regulator_bulk_get(dev, sensor_info->ldo_num, sensor_info->ldo);
    if (rc < 0) {
        cam_err("%s failed %d\n", __func__, __LINE__);
        return rc;
    }
    cam_info("power up config the ldo end");
    return rc;
}

void hw_sensor_power_down_config(hwsensor_board_info_t *sensor_info)
{
    int i;
    cam_info("power down config the ldo begin");
    for(i = 0; i < sensor_info->ldo_num; i++){
        cam_info("%s %d=%s",__func__,i,sensor_info->ldo[i].supply);
        devm_regulator_put(sensor_info->ldo[i].consumer);
    }
    cam_info("power down config the ldo end");
    return;
}
int hw_sensor_get_dt_data(struct platform_device *pdev,
    sensor_t *sensor)
{
    struct device_node *of_np = pdev->dev.of_node;
    hwsensor_board_info_t *sensor_info = NULL;
    int rc = 0;
    int ret = 0;
    int count = 0;
    u32 i, index = 0;
    char *gpio_tag = NULL;
    /* enum gpio_t */
    const char *gpio_ctrl_types[IO_MAX] =
        {"reset", "fsin", "pwdn", "vcm_pwdn", "suspend", "suspend2", "reset2",\
            "ldo_en", "ois", "ois2", "dvdd0-en", "dvdd1-en",\
         "iovdd-en", "mispdcdc-en", "mipisw", "reset3", "pwdn2", \
         "avdd1_en", "avdd2_en","mipi_en", "afvdd_en"};

    cam_debug("enter %s", __func__);
    sensor_info = kzalloc(sizeof(hwsensor_board_info_t),
                GFP_KERNEL);
    if (!sensor_info) {
        cam_err("%s failed %d\n", __func__, __LINE__);
        return -ENOMEM;
    }
    sensor->board_info= sensor_info;

    rc = of_property_read_string(of_np, "huawei,sensor_name",
        &sensor_info->name);
    cam_debug("%s huawei,sensor_name %s, rc %d\n", __func__,
        sensor_info->name, rc);
    if (rc < 0) {
        cam_err("%s failed %d\n", __func__, __LINE__);
        goto fail;
    }

    rc = of_property_read_u32(of_np, "huawei,is_fpga",
        (u32*)&is_fpga);
    cam_debug("%s huawei,is_fpga: %d, rc %d\n", __func__,
        is_fpga, rc);
    if (rc < 0) {
        cam_err("%s failed %d\n", __func__, __LINE__);
        goto fail;
    }

    rc = of_property_read_u32(of_np, "huawei,sensor_index",
        (u32*)(&sensor_info->sensor_index));
    cam_debug("%s huawei,sensor_index %d, rc %d\n", __func__,
        sensor_info->sensor_index, rc);
    if (rc < 0) {
        cam_err("%s failed %d\n", __func__, __LINE__);
        goto fail;
    }

    rc = of_property_read_u32(of_np, "huawei,pd_valid",
        (u32*)&sensor_info->power_conf.pd_valid);
    cam_debug("%s huawei,pd_valid %d, rc %d\n", __func__,
        sensor_info->power_conf.pd_valid, rc);
    if (rc < 0) {
        cam_err("%s failed %d\n", __func__, __LINE__);
        goto fail;
    }

    rc = of_property_read_u32(of_np, "huawei,reset_valid",
        (u32*)&sensor_info->power_conf.reset_valid);
    cam_debug("%s huawei,reset_valid %d, rc %d\n", __func__,
        sensor_info->power_conf.reset_valid, rc);
    if (rc < 0) {
        cam_err("%s failed %d\n", __func__, __LINE__);
        goto fail;
    }

    rc = of_property_read_u32(of_np, "huawei,vcmpd_valid",
        (u32*)&sensor_info->power_conf.vcmpd_valid);
    cam_debug("%s huawei,vcmpd_valid %d, rc %d\n", __func__,
        sensor_info->power_conf.vcmpd_valid, rc);
    if (rc < 0) {
        cam_err("%s failed %d\n", __func__, __LINE__);
        goto fail;
    }


    //add csi_index and i2c_index for dual camera.
    //
    count = of_property_count_elems_of_size(of_np, "huawei,csi_index",
        sizeof(u32));
    if (count > 0) {
        ret = of_property_read_u32_array(of_np, "huawei,csi_index",
            (u32*)&sensor_info->csi_id, count);
    } else {
        sensor_info->csi_id[0] = sensor_info->sensor_index;
        sensor_info->csi_id[1] = -1;
    }
    cam_info("sensor:%s csi_id[0-1]=%d:%d", sensor_info->name,
        sensor_info->csi_id[0], sensor_info->csi_id[1]);

    count = of_property_count_elems_of_size(of_np, "huawei,i2c_index",
        sizeof(u32));
    if (count > 0) {
        ret = of_property_read_u32_array(of_np, "huawei,i2c_index",
           (u32*)&sensor_info->i2c_id, count);
    } else {
        sensor_info->i2c_id[0] = sensor_info->sensor_index;
        sensor_info->i2c_id[1] = -1;
    }
    cam_info("sensor:%s i2c_id[0-1]=%d:%d", sensor_info->name,
            sensor_info->i2c_id[0], sensor_info->i2c_id[1]);

    //FPGA IGNORE
    if (hw_is_fpga_board())
        return rc;

    rc = of_property_read_u32(of_np, "huawei,mclk",
        &sensor_info->mclk);
    cam_debug("%s huawei,mclk 0x%x, rc %d\n", __func__,
        sensor_info->mclk, rc);
    if (rc < 0) {
        cam_err("%s failed %d\n", __func__, __LINE__);
        goto fail;
    }

    /* get ldo */
    sensor_info->ldo_num = of_property_count_strings(of_np, "huawei,ldo-names");
    if(sensor_info->ldo_num < 0) {
            cam_err("%s failed %d\n", __func__, __LINE__);
            //goto fail;
    } else {
        cam_debug("ldo num = %d", sensor_info->ldo_num);
        for (i = 0; i < (u32)sensor_info->ldo_num; i++) {
            rc = of_property_read_string_index(of_np, "huawei,ldo-names", i, &sensor_info->ldo[i].supply);
            if(rc < 0) {
                cam_err("%s failed %d\n", __func__, __LINE__);
                goto fail;
            }
        }
    }
    sensor_info->gpio_num = of_gpio_count(of_np);
    if(sensor_info->gpio_num < 0 ) {
            cam_err("%s failed %d, ret is %d\n", __func__, __LINE__, sensor_info->gpio_num);
            goto fail;
    }

    cam_debug("gpio num = %d", sensor_info->gpio_num);
    for(i = 0; i < (u32)sensor_info->gpio_num; i++) {
        rc = of_property_read_string_index(of_np, "huawei,gpio-ctrl-types",
            i, (const char **)&gpio_tag);
        if(rc < 0) {
            cam_err("%s failed %d", __func__, __LINE__);
            goto fail;
        }
        for(index = 0; index < IO_MAX; index++) {
            if (gpio_ctrl_types[index]) {
                if(!strcmp(gpio_ctrl_types[index], gpio_tag))
                    sensor_info->gpios[index].gpio = of_get_gpio(of_np, i);
            }
        }
        cam_debug("gpio ctrl types: %s", gpio_tag);
    }

    rc = of_property_read_u32(of_np, "module_type", (u32*)&sensor_info->module_type);
    cam_info("%s module_type 0x%x, rc %d\n", __func__, sensor_info->module_type, rc);
    if (rc < 0) {
        sensor_info->module_type = 0;
        cam_warn("%s read module_type failed, rc %d, set default value %d\n",
        __func__, rc, sensor_info->module_type);
        rc = 0;
    }

    rc = of_property_read_u32(of_np, "huawei,flash_pos_type", (u32 *)&sensor_info->flash_pos_type);
    cam_info("%s flash_pos_type 0x%x, rc %d\n", __func__, sensor_info->flash_pos_type, rc);
    if (rc < 0) {
        sensor_info->flash_pos_type = 0;//default alone
        cam_warn("%s read flash_pos_type failed, rc %d, set default value %d\n",
            __func__, rc, sensor_info->flash_pos_type);
        rc = 0;
    }
    rc = of_property_read_u32(of_np, "reset_type", &sensor_info->reset_type);
    cam_info("%s reset_type 0x%x, rc %d\n", __func__, sensor_info->reset_type, rc);
    if (rc < 0) {
        sensor_info->reset_type = 0;
        cam_warn("%s read reset_type failed, rc %d, set default value %d\n",
        __func__, rc, sensor_info->reset_type);
        rc = 0;
    }
    rc = of_property_read_u32(of_np, "release_value", &sensor_info->release_value);
    cam_info("%s release_value 0x%x, rc %d\n", __func__, sensor_info->release_value, rc);
    if (rc < 0) {
        sensor_info->release_value = 0;
        cam_warn("%s read release_value failed, rc %d, set default value %d\n",
        __func__, rc, sensor_info->release_value);
        rc = 0;
    }
    rc = of_property_read_u32(of_np, "hold_value", &sensor_info->hold_value);
    cam_info("%s hold_value 0x%x, rc %d\n", __func__, sensor_info->hold_value, rc);
    if (rc < 0) {
        sensor_info->hold_value = 0;
        cam_warn("%s read hold_value failed, rc %d, set default value %d\n",
        __func__, rc, sensor_info->hold_value);
        rc = 0;
    }
	rc = of_property_read_u32(of_np, "need_rpc", &sensor_info->need_rpc); /* lint !e64 */
	cam_info("%s need_rpc 0x%x, rc %d\n", __func__, sensor_info->need_rpc, rc);
	if (rc < 0) {
		sensor_info->need_rpc = 0;
		cam_warn("%s read need_rpc failed, rc %d, set default value %d\n",
			__func__, rc, sensor_info->need_rpc);
		rc = 0;
	}

	rc = of_property_read_u32(of_np, "dynamic_mipisw_num", &sensor_info->dynamic_mipisw_num);
	cam_info("%s dynamic_mipisw_num 0x%x, rc %d\n", __func__, sensor_info->dynamic_mipisw_num, rc);
	if (rc < 0) {
		sensor_info->dynamic_mipisw_num = 0;
		cam_warn("%s read dynamic_mipisw_num failed, rc %d, set default value %d\n",
		__func__, rc, sensor_info->dynamic_mipisw_num);
		rc = 0;
	}

	if (sensor_info->dynamic_mipisw_num == 1) {
		rc = of_property_read_u32(of_np, "mipisw_enable_value0", &sensor_info->mipisw_enable_value0);
		cam_info("%s mipisw_enable_value0 0x%x, rc %d\n", __func__, sensor_info->mipisw_enable_value0, rc);
		if (rc < 0) {
			sensor_info->mipisw_enable_value0 = 0;
			cam_warn("%s read mipisw_enable_value0 failed, rc %d, set default value %d\n",
			__func__, rc, sensor_info->mipisw_enable_value0);
			rc = 0;
		}
	}
    return rc;
fail:
    cam_err("%s error exit.\n", __func__);
    kfree(sensor_info);
    sensor_info = NULL;
    sensor->board_info = NULL;
    return rc;
}

static int hw_sensor_get_dt_power_setting(struct device_node *dev_node, 
    struct sensor_power_setting_array *power_setting_array, int is_power_on)
{
    int rc = 0;
    int i = 0;
    int j = 0;
    int count = 0;
    int seq_size = 0;
    const char *seq_type_name = NULL;
    const char *seq_val_name = NULL;
    const char *seq_cfg_name = NULL;
    const char *seq_sensor_index_name = NULL;
    const char *seq_delay_name = NULL;
    const char *seq_name = NULL;
    uint32_t *seq_vals = NULL;
    uint32_t *cfg_vals = NULL;
    uint32_t *sensor_indexs = NULL;
    uint32_t *seq_delays = NULL;
    struct sensor_power_setting *power_settings = NULL;

    cam_info("%s:%d is_power_on = %d.", __func__, __LINE__, is_power_on);
    seq_type_name = (is_power_on ? "huawei,cam-power-seq-type" : "huawei,cam-power-down-seq-type");
    count = of_property_count_strings(dev_node, seq_type_name);
    if (count <= 0) {
        cam_warn("%s:%d power settings not to config.", __func__, __LINE__);
        return -EINVAL;
    }

    seq_vals = kzalloc(sizeof(uint32_t) * count, GFP_KERNEL);
    if (NULL == seq_vals) {
        cam_err("%s:%d failed\n", __func__, __LINE__);
        return -ENOMEM;
    }

    seq_val_name = (is_power_on ? "huawei,cam-power-seq-val" : "huawei,cam-power-down-seq-val");
    rc = of_property_read_u32_array(dev_node, seq_val_name, seq_vals, count);
    if (rc < 0) {
        cam_warn("%s:%d seq val not to config.", __func__, __LINE__);
    }

    cfg_vals = kzalloc(sizeof(uint32_t) * count, GFP_KERNEL);
    if (NULL == cfg_vals) {
        cam_err("%s:%d failed\n", __func__, __LINE__);
        rc = -ENOMEM;
        goto OUT;
    }

    seq_cfg_name = (is_power_on ? "huawei,cam-power-seq-cfg-val" : "huawei,cam-power-down-seq-cfg-val");
    rc = of_property_read_u32_array(dev_node, seq_cfg_name, cfg_vals, count);
    if (rc < 0) {
        cam_warn("%s:%d seq val not to config.", __func__, __LINE__);
    }

    sensor_indexs = kzalloc(sizeof(uint32_t) * count, GFP_KERNEL);
    if (!sensor_indexs) {
        cam_err("%s failed %d", __func__, __LINE__);
        rc = -ENOMEM;
        goto OUT;
    }

    seq_sensor_index_name = (is_power_on ? "huawei,cam-power-seq-sensor-index" : "huawei,cam-power-down-seq-sensor-index");
    rc = of_property_read_u32_array(dev_node, seq_sensor_index_name, sensor_indexs, count);
    if (rc < 0) {
        cam_warn("%s:%d sensor index not to config.", __func__, __LINE__);
    }

    seq_delays = kzalloc(sizeof(uint32_t) * count, GFP_KERNEL);
    if (!seq_delays) {
        cam_err("%s failed %d", __func__, __LINE__);
        rc = -ENOMEM;
        goto OUT;
    }

    seq_delay_name = (is_power_on ? "huawei,cam-power-seq-delay" : "huawei,cam-power-down-seq-delay");
    rc = of_property_read_u32_array(dev_node, seq_delay_name, seq_delays, count);
    if (rc < 0) {
        cam_err("%s:%d seq delay not to config.", __func__, __LINE__);
    }

    power_settings = kzalloc(sizeof(struct sensor_power_setting) * count, GFP_KERNEL);
    if (NULL == power_settings) {
        cam_err("%s failed %d\n", __func__, __LINE__);
        rc = -ENOMEM;
        goto OUT;
    }
    power_setting_array->power_setting = power_settings;
    power_setting_array->size = count;

    for (i = 0; i < count; i++) {
        rc = of_property_read_string_index(dev_node, seq_type_name, i, &seq_name);
        if (rc < 0) {
            cam_err("%s failed %d\n", __func__, __LINE__);
            goto OUT;
        }

        seq_size = (int)sizeof(seq_type_tab)/sizeof(seq_type_tab[0]);
        for (j = 0; j < seq_size; j++) {
            if (!strcmp(seq_name, seq_type_tab[j].seq_name)) {
                power_settings[i].seq_type = seq_type_tab[j].seq_type;
                break;
            }
        }

        if (j >= seq_size) {
            cam_warn("%s: unrecognized seq-type\n", __func__);
            rc = -EINVAL;
            goto OUT;
        }

        power_settings[i].seq_val = seq_vals[i];
        power_settings[i].config_val = cfg_vals[i];
        power_settings[i].sensor_index = ((sensor_indexs[i]>=0xFF) ? 0xFFFFFFFF : sensor_indexs[i]);
        power_settings[i].delay = seq_delays[i];
        cam_info("%s:%d index[%d] seq_name[%s] seq_type[%d] cfg_vals[%d] seq_delay[%d] sensor_index[0x%x]",
                       __func__, __LINE__, i, seq_name, power_settings[i].seq_type, cfg_vals[i], seq_delays[i], sensor_indexs[i]);
        cam_info("%s:%d sensor_index = %d", __func__, __LINE__, power_settings[i].sensor_index);
    }

OUT:
    if (NULL != seq_vals) {
        kfree(seq_vals);
        seq_vals = NULL;
    }
    if (NULL != cfg_vals) {
        kfree(cfg_vals);
        cfg_vals = NULL;
    }
    if (NULL != sensor_indexs) {
        kfree(sensor_indexs);
        sensor_indexs = NULL;
    }
    if (NULL != seq_delays) {
        kfree(seq_delays);
        seq_delays = NULL;
    }

    return rc;
}

int hw_sensor_get_dt_power_setting_data(struct platform_device *pdev,
    sensor_t *sensor)
{
    int rc = 0;
    struct device_node *dev_node = NULL;

    if (NULL == pdev || NULL == pdev->dev.of_node || NULL == sensor) {
        cam_err("%s dev_node is NULL.", __func__);
        return -EINVAL;
    }

    dev_node = pdev->dev.of_node;

    rc = hw_sensor_get_dt_power_setting(dev_node, &sensor->power_setting_array, 1);
    if (rc < 0) {
        cam_err("%s:%d get dt power on setting fail.", __func__, __LINE__);
        return rc;
    }

    rc = hw_sensor_get_dt_power_setting(dev_node, &sensor->power_down_setting_array, 0); // 0 is power down
    if (rc < 0) {
        cam_warn("%s:%d get dt power down setting fail, need not to config", __func__, __LINE__);
        return 0;
    }
    return 0;
}

void hw_camdrv_msleep(unsigned int ms)
{
    struct timeval now;
    unsigned long jiffies_to;

    if (ms > 0) {
        now.tv_sec  = ms / 1000;
        now.tv_usec = (ms % 1000) * 1000;
        jiffies_to = timeval_to_jiffies(&now);
        schedule_timeout_interruptible(jiffies_to);
    }
}
EXPORT_SYMBOL(hw_camdrv_msleep);

int hw_is_fpga_board(void)
{
    cam_debug("%s is_fpga=%d.", __func__, is_fpga);
    return is_fpga;
}
EXPORT_SYMBOL(hw_is_fpga_board);

//lint -restore
