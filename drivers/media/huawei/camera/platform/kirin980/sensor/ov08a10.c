

#include <linux/module.h>
#include <linux/printk.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/rpmsg.h>
#include "hwsensor.h"
#include "sensor_commom.h"
#include "hw_csi.h"
#include "../pmic/hw_pmic.h"

#define I2S(i) container_of(i, sensor_t, intf)
#define POWER_DELAY_0        0//delay 0 ms
#define POWER_DELAY_1        1//delay 1 ms
#define CTL_RESET_HOLD    (0)
#define CTL_RESET_RELEASE (1)

//lint -save -e846 -e514 -e866 -e30 -e84 -e785 -e64
//lint -save -e826 -e838 -e715 -e747 -e778 -e774 -e732
//lint -save -e650 -e31 -e731 -e528 -e753 -e737

//static bool s_ov08a10_power_on = false;
static bool power_on_status = false;//false: power off, true:power on
struct mutex ov08a10_power_lock;

char const* ov08a10_get_name(hwsensor_intf_t* si);
int ov08a10_config(hwsensor_intf_t* si, void  *argp);
int ov08a10_power_up(hwsensor_intf_t* si);
int ov08a10_power_down(hwsensor_intf_t* si);
int ov08a10_match_id(hwsensor_intf_t* si, void * data);
int ov08a10_csi_enable(hwsensor_intf_t* si);
int ov08a10_csi_disable(hwsensor_intf_t* si);
extern int hw_sensor_ldo_config(ldo_index_t ldo, hwsensor_board_info_t *sensor_info, struct sensor_power_setting *power_setting, int state);

static hwsensor_vtbl_t s_ov08a10_vtbl =
{
    .get_name = ov08a10_get_name,
    .config = ov08a10_config,
    .power_up = ov08a10_power_up,
    .power_down = ov08a10_power_down,
    .match_id = ov08a10_match_id,
    .csi_enable = ov08a10_csi_enable,
    .csi_disable = ov08a10_csi_disable,
};

static hwsensor_intf_t *s_intf = NULL;
static sensor_t *s_sensor = NULL;

struct sensor_power_setting buck2_power_up = {
        .seq_type = SENSOR_MISP_VDD,
        .config_val = LDO_VOLTAGE_V1P25V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
};

struct sensor_power_setting hw_ov08a10_power_up_setting[] = {
    {
        .seq_type = SENSOR_RXDPHY_CLK,
        .delay = 1,
    },
    //TCAM DVDD 1.2V
    {
        .seq_type = SENSOR_DVDD,
        .config_val = LDO_VOLTAGE_1P205V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //SENSOR IOVDD 1.8V VOUT21
    {
        .seq_type = SENSOR_IOVDD,
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //AFVDD GPIO 255 ENABLE
    {
        .seq_type = SENSOR_VCM_PWDN,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //MCAM_AVDD1_2V8
    {
        .seq_type = SENSOR_PMIC,
        .seq_val = VOUT_LDO_2,
        .config_val = PMIC_2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //MCAM_DRVVDD_2V8
    {
        .seq_type = SENSOR_PMIC,
        .seq_val = VOUT_LDO_4,
        .config_val = PMIC_2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },

    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },

};

atomic_t volatile ov08a10_powered = ATOMIC_INIT(0);
sensor_t s_ov08a10 =
{
    .intf = { .vtbl = &s_ov08a10_vtbl, },
    .power_setting_array = {
            .size = ARRAY_SIZE(hw_ov08a10_power_up_setting),
            .power_setting = hw_ov08a10_power_up_setting,
     },
     .p_atpowercnt = &ov08a10_powered,
};

const struct of_device_id s_ov08a10_dt_match[] =
{
    {
        .compatible = "huawei,ov08a10",
        .data = &s_ov08a10.intf,
    },

    {
    },
};

MODULE_DEVICE_TABLE(of, s_ov08a10_dt_match);

struct platform_driver
s_ov08a10_driver =
{
    .driver =
    {
        .name = "huawei,ov08a10",
        .owner = THIS_MODULE,
        .of_match_table = s_ov08a10_dt_match,
    },
};

char const* ov08a10_get_name(hwsensor_intf_t* si)
{
    sensor_t* sensor = NULL;

    if (NULL == si) {
        cam_err("%s. si is NULL.", __func__);
        return NULL;
    }

    sensor = I2S(si);
    if (NULL == sensor || NULL == sensor->board_info || NULL == sensor->board_info->name) {
        cam_err("%s. sensor or board_info->name is NULL.", __func__);
        return NULL;
    }
    return sensor->board_info->name;
}

int ov08a10_power_up(hwsensor_intf_t* si)
{
    int ret = 0;
    sensor_t* sensor = NULL;

    if (NULL == si) {
        cam_err("%s. si is NULL.", __func__);
        return -EINVAL;
    }

    sensor = I2S(si);
    if (NULL == sensor || NULL == sensor->board_info || NULL == sensor->board_info->name) {
        cam_err("%s. sensor or board_info->name is NULL.", __func__);
        return -EINVAL;
    }
    cam_info("enter %s. index = %d name = %s", __func__, sensor->board_info->sensor_index, sensor->board_info->name);

    if (hw_is_fpga_board()) {
        ret = do_sensor_power_on(sensor->board_info->sensor_index, sensor->board_info->name);
    } else {
        ret = hw_sensor_power_up(sensor);
    }
    if (0 == ret )
    {
        cam_info("%s. power up sensor success.", __func__);
    }
    else
    {
        cam_err("%s. power up sensor fail.", __func__);
    }
    return ret;
}

int ov08a10_power_down(hwsensor_intf_t* si)
{
    int ret = 0;
    sensor_t* sensor = NULL;

    if (NULL == si) {
        cam_err("%s. si is NULL.", __func__);
        return -EINVAL;
    }

    sensor = I2S(si);
    if (NULL == sensor || NULL == sensor->board_info || NULL == sensor->board_info->name) {
        cam_err("%s. sensor or board_info->name is NULL.", __func__);
        return -EINVAL;
    }
    cam_info("enter %s. index = %d name = %s", __func__, sensor->board_info->sensor_index, sensor->board_info->name);
    if (hw_is_fpga_board()) {
        ret = do_sensor_power_off(sensor->board_info->sensor_index, sensor->board_info->name);
    } else {
        ret = hw_sensor_power_down(sensor);
    }
    if (0 == ret )
    {
        cam_info("%s. power down sensor success.", __func__);
    }
    else
    {
        cam_err("%s. power down sensor fail.", __func__);
    }

    return ret;
}

int ov08a10_csi_enable(hwsensor_intf_t* si)
{
    return 0;
}

int ov08a10_csi_disable(hwsensor_intf_t* si)
{
    return 0;
}

static int ov08a10_do_hw_reset(hwsensor_intf_t* si, int ctl, int id)
{
    char *state = NULL;
    sensor_t* sensor = I2S(si);
    hwsensor_board_info_t *b_info = NULL;
    int ret = 0;

    b_info = sensor->board_info;
    if (NULL == b_info) {
        cam_warn("%s invalid sensor board info", __func__);
        return 0;
    }

    ret  = gpio_request(b_info->gpios[RESETB].gpio, "ov08a10reset-0");
    if (ret) {
        cam_err("%s requeset reset pin failed", __func__);
        return ret;
    }

    if (ctl == CTL_RESET_HOLD) {
        state = "hold";
        ret  = gpio_direction_output(b_info->gpios[RESETB].gpio, 0);
    } else {
        state = "release";
        ret  = gpio_direction_output(b_info->gpios[RESETB].gpio, 1);
        udelay(2000);
    }
    gpio_free(b_info->gpios[RESETB].gpio);

    if (ret) {
        cam_err("%s set reset pin failed", __func__);
    } else {
        cam_info("%s: set reset state=%s", __func__, state);
    }
    return ret;
}

int ov08a10_match_id(hwsensor_intf_t* si, void * data)
{
    sensor_t* sensor = NULL;
    struct sensor_cfg_data *cdata = NULL;

    cam_info("%s enter.", __func__);

    if ((NULL == si) || (NULL == data)) {
        cam_err("%s. si or data is NULL.", __func__);
        return -EINVAL;
    }

    sensor = I2S(si);
    if (NULL == sensor || NULL == sensor->board_info || NULL == sensor->board_info->name) {
        cam_err("%s. sensor or board_info is NULL.", __func__);
        return -EINVAL;
    }
    cdata  = (struct sensor_cfg_data *)data;
    cdata->data = sensor->board_info->sensor_index;
    cam_info("%s name:%s", __func__, sensor->board_info->name);

    return 0;
}

int ov08a10_config(hwsensor_intf_t* si, void  *argp)
{
    struct sensor_cfg_data *data = NULL;
    int ret = 0;

    if (NULL == si || NULL == argp || NULL == si->vtbl) {
        cam_err("%s : si or argp is null", __func__);
        return -1;
    }

    data = (struct sensor_cfg_data *)argp;
    cam_debug("ov08a10 cfgtype = %d",data->cfgtype);
    switch(data->cfgtype){
        case SEN_CONFIG_POWER_ON:
            mutex_lock(&ov08a10_power_lock);
            if (false == power_on_status) {
                ret = si->vtbl->power_up(si);
                if (0 == ret) {
                    power_on_status = true;
                } else {
                    cam_err("%s. power up fail.", __func__);
                }
            }
            /*lint -e455 -esym(455,*)*/
            mutex_unlock(&ov08a10_power_lock);
            /*lint -e455 -esym(455,*)*/
            break;
        case SEN_CONFIG_POWER_OFF:
            mutex_lock(&ov08a10_power_lock);
            if (true == power_on_status) {
                ret = si->vtbl->power_down(si);
                if (0 != ret) {
                    cam_err("%s. power_down fail.", __func__);
                }
                power_on_status = false;
            }
            /*lint -e455 -esym(455,*)*/
            mutex_unlock(&ov08a10_power_lock);
            /*lint -e455 -esym(455,*)*/
            break;
        /* case SEN_CONFIG_WRITE_REG: */
        /*     break; */
        /* case SEN_CONFIG_READ_REG: */
        /*     break; */
        /* case SEN_CONFIG_WRITE_REG_SETTINGS: */
        /*     break; */
        /* case SEN_CONFIG_READ_REG_SETTINGS: */
        /*     break; */
        /* case SEN_CONFIG_ENABLE_CSI: */
        /*     break; */
        /* case SEN_CONFIG_DISABLE_CSI: */
        /*     break; */
        case SEN_CONFIG_MATCH_ID:
            ret = si->vtbl->match_id(si,argp);
            break;
        case SEN_CONFIG_RESET_HOLD:
            ret = ov08a10_do_hw_reset(si, CTL_RESET_HOLD, data->mode);
            break;
        case SEN_CONFIG_RESET_RELEASE:
            ret = ov08a10_do_hw_reset(si, CTL_RESET_RELEASE, data->mode);
            break;
        default:
            cam_debug("%s cfgtype(%d) is error", __func__, data->cfgtype);
            break;
    }
    return ret;
}

int32_t ov08a10_platform_probe(struct platform_device* pdev)
{
    int rc = 0;
    const struct of_device_id *id = NULL;
    hwsensor_intf_t *intf = NULL;
    sensor_t *sensor = NULL;
    struct device_node *np = NULL;
    struct regulator *regulator_buck =NULL;
    cam_notice("enter %s",__func__);

    if (NULL == pdev) {
        cam_err("%s pdev is NULL", __func__);
        return -EINVAL;
    }

    mutex_init(&ov08a10_power_lock);
    np = pdev->dev.of_node;
    if (NULL == np) {
        cam_err("%s of_node is NULL", __func__);
        return -ENODEV;
    }

    id = of_match_node(s_ov08a10_dt_match, np);
    if (NULL == id) {
        cam_err("%s none id matched", __func__);
        return -ENODEV;
    }

    intf = (hwsensor_intf_t*)id->data;
    if (NULL == intf) {
        cam_err("%s intf is NULL", __func__);
        return -ENODEV;
    }

    sensor = I2S(intf);
    if(NULL == sensor){
        cam_err("%s sensor is NULL rc %d", __func__, rc);
        return -ENODEV;
    }
    rc = hw_sensor_get_dt_data(pdev, sensor);
    if (rc < 0) {
        cam_err("%s no dt data rc %d", __func__, rc);
        return -ENODEV;
    }

    sensor->dev = &pdev->dev;
    rc = hwsensor_register(pdev, intf);
    if (rc < 0) {
        cam_err("%s hwsensor_register failed rc %d\n", __func__, rc);
        return -ENODEV;
    }
    s_intf = intf;

    rc = rpmsg_sensor_register(pdev, (void*)sensor);
    if (rc < 0) {
        hwsensor_unregister(intf);
        s_intf = NULL;
        cam_err("%s rpmsg_sensor_register failed rc %d\n", __func__, rc);
        return -ENODEV;
    }
    s_sensor = sensor;
    regulator_buck = devm_regulator_get(&pdev->dev,"buck2");
    if(IS_ERR(regulator_buck)) {
        rc = -ENODEV;
        cam_err("Get buck2 failed,ret:%d",rc);
        return rc;
    }
    hw_sensor_ldo_config(LDO_MISP, sensor->board_info,&buck2_power_up,POWER_ON);

    return rc;
}

int __init
ov08a10_init_module(void)
{
    cam_notice("enter %s",__func__);
    return platform_driver_probe(&s_ov08a10_driver,
            ov08a10_platform_probe);
}

void __exit
ov08a10_exit_module(void)
{
    if( NULL != s_sensor)
    {
        rpmsg_sensor_unregister((void*)s_sensor);
        s_sensor = NULL;
    }
    if (NULL != s_intf) {
        hwsensor_unregister(s_intf);
        s_intf = NULL;
    }
    platform_driver_unregister(&s_ov08a10_driver);
}
//lint -restore

/*lint -e528 -esym(528,*)*/
module_init(ov08a10_init_module);
module_exit(ov08a10_exit_module);
/*lint -e528 +esym(528,*)*/
/*lint -e753 -esym(753,*)*/
MODULE_DESCRIPTION("ov08a10");
MODULE_LICENSE("GPL v2");
/*lint -e753 +esym(753,*)*/
