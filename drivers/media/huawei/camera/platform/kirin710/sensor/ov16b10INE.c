 


#include <linux/module.h>
#include <linux/printk.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/rpmsg.h>

#include "hwsensor.h"
#include "sensor_commom.h"
#include "hw_csi.h"

#define I2S(i) container_of(i, sensor_t, intf)
#define Sensor2Pdev(s) container_of((s).dev, struct platform_device, dev)
#define POWER_DELAY_0       0
#define POWER_DELAY_1       1
#define POWER_DELAY_5       5

extern struct hw_csi_pad hw_csi_pad;
static hwsensor_vtbl_t s_ov16b10INE_vtbl;
static bool ov16b10INE_power_on = false;

//lint -save -e846 -e514 -e866 -e30 -e84 -e785 -e64 -e826 -e838 -e715 -e747 -e778 -e774 -e732 -e650 -e31 -e731 -e528 -e753 -e737

int ov16b10INE_config(hwsensor_intf_t* si, void  *argp);

static struct platform_device *s_pdev = NULL;
static sensor_t *s_sensor = NULL;
static sensor_t s_ov16b10INE;
struct sensor_power_setting hw_ov16b10INE_power_setting[] = {
    //disable camera S1 reset
    {
        .seq_type = SENSOR_SUSPEND,
        .config_val = SENSOR_GPIO_LOW,//SUSPEND type use real gpio status
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_0,
    },

    //disable camera S1 pwdn[GPIO28]
    {
        .seq_type = SENSOR_PWDN,
        .config_val = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_0,
    },

    //disable camera S0 reset
    {
        .seq_type = SENSOR_SUSPEND2,
        .config_val = SENSOR_GPIO_LOW,//SUSPEND type use real gpio status
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_0,
    },

    //MCAM0 VCM ENABLE [GPIO178]
    {
        .seq_type     = SENSOR_VCM_PWDN,
        .config_val   = SENSOR_GPIO_HIGH, //out high
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_1,
    },

    //MCAM0 AFVDD & mipi enable 2.85V [LDO25]
    {
        .seq_type     = SENSOR_VCM_AVDD,
        .data         = (void*)"cameravcm-vcc",
        .config_val   = LDO_VOLTAGE_V3PV,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_1,
    },

    //MCAM0 IOVDD 1.80V [LDO21]
    {
        .seq_type     = SENSOR_IOVDD,
        .data         = (void*)"back-main-sensor-iovdd",
        .config_val   = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_1,
    },

    //MCAM0 AVDD 2.80V [LDO19]
    {
        .seq_type     = SENSOR_AVDD,
        .data         = (void*)"back-main-sensor-avdd",
        .config_val   = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },

    //MCAM0 DVDD 1.1V [LDO32]
    {
        .seq_type     = SENSOR_DVDD,
        .data         = (void*)"back-main-sensor-dvdd",
        .config_val   = LDO_VOLTAGE_1P1V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_1,
    },

    //MCAM0 CLK
    {
        .seq_type     = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_1,
    },

    //MCAM0 RESET [GPIO32]
    {
        .seq_type     = SENSOR_RST,
        .config_val   = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_5,
    },
};
struct sensor_power_setting hw_ov16b10INE_power_down_setting[] = {
    //MCAM0 RESET [GPIO32]
    {
        .seq_type     = SENSOR_RST,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_5,
    },
    //MCAM0 CLK
    {
        .seq_type     = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_1,
    },
    //MCAM0 DVDD 1.1V [LDO32]
    {
        .seq_type     = SENSOR_DVDD,
        .data         = (void*)"back-main-sensor-dvdd",
        .config_val   = LDO_VOLTAGE_1P1V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_1,
    },
    //MCAM0 AVDD 2.80V [LDO19]
    {
        .seq_type     = SENSOR_AVDD,
        .data         = (void*)"back-main-sensor-avdd",
        .config_val   = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },
    //MCAM0 IOVDD 1.80V [LDO21]
    {
        .seq_type     = SENSOR_IOVDD,
        .data         = (void*)"back-main-sensor-iovdd",
        .config_val   = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_1,
    },
    //MCAM0 AFVDD & mipi enable 2.85V [LDO25]
    {
        .seq_type     = SENSOR_VCM_AVDD,
        .data         = (void*)"cameravcm-vcc",
        .config_val   = LDO_VOLTAGE_V3PV,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_1,
    },
    //MCAM0 VCM ENABLE [GPIO178]
    {
        .seq_type     = SENSOR_VCM_PWDN,
        .config_val   = SENSOR_GPIO_LOW, //out high
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_1,
    },
    //disable camera S0 reset
    {
        .seq_type = SENSOR_SUSPEND2,
        .config_val = SENSOR_GPIO_LOW,//SUSPEND type use real gpio status
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_0,
    },
    //disable camera S1 pwdn[GPIO28]
    {
        .seq_type = SENSOR_PWDN,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_0,
    },
    //disable camera S1 reset
    {
        .seq_type = SENSOR_SUSPEND,
        .config_val = SENSOR_GPIO_LOW,//SUSPEND type use real gpio status
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_0,
    },

};
atomic_t volatile ov16b10INE_powered = ATOMIC_INIT(0);
struct mutex ov16b10INE_power_lock;

static sensor_t s_ov16b10INE =
{
    .intf = { .vtbl = &s_ov16b10INE_vtbl, },
    .power_setting_array =
    {
        .size          = ARRAY_SIZE(hw_ov16b10INE_power_setting),
        .power_setting = hw_ov16b10INE_power_setting,
    },
    .power_down_setting_array = {
        .size = ARRAY_SIZE(hw_ov16b10INE_power_down_setting),
        .power_setting = hw_ov16b10INE_power_down_setting,
    },
    .p_atpowercnt      = &ov16b10INE_powered,
};

static const struct of_device_id s_ov16b10INE_dt_match[] =
{
    {
        .compatible = "huawei,ov16b10INE",
        .data       = &s_ov16b10INE.intf,
    },
    {
    },
};

MODULE_DEVICE_TABLE(of, s_ov16b10INE_dt_match);

static struct platform_driver s_ov16b10INE_driver =
{
    .driver =
    {
        .name           = "huawei,ov16b10INE",
        .owner          = THIS_MODULE,
        .of_match_table = s_ov16b10INE_dt_match,
    },
};

char const* ov16b10INE_get_name(hwsensor_intf_t* si)
{
    sensor_t* sensor = NULL;

    if (!si){
        cam_err("%s. si is null.", __func__);
        return NULL;
    }

    sensor = I2S(si);
    if (NULL == sensor || NULL == sensor->board_info || NULL == sensor->board_info->name) {
        cam_err("%s. sensor or board_info->name is NULL.", __func__);
        return NULL;
    }
    return sensor->board_info->name;
}

int ov16b10INE_power_up(hwsensor_intf_t* si)
{
    int ret          = 0;
    sensor_t* sensor = NULL;

    if (!si){
        cam_err("%s. si is null.", __func__);
        return -EINVAL;
    }

    sensor = I2S(si);
    if (NULL == sensor || NULL == sensor->board_info || NULL == sensor->board_info->name) {
        cam_err("%s. sensor or board_info->name is NULL.", __func__);
        return -EINVAL;
    }
    cam_info("enter %s. index = %d name = %s", __func__, sensor->board_info->sensor_index, sensor->board_info->name);
    ret = hw_sensor_power_up_config(sensor->dev, sensor->board_info);
    if (0 == ret ){
        cam_info("%s. power up config success.", __func__);
    }else{
        cam_err("%s. power up config fail.", __func__);
        return ret;
    }
    if (hw_is_fpga_board()) {
        ret = do_sensor_power_on(sensor->board_info->sensor_index, sensor->board_info->name);
    } else {
        ret = hw_sensor_power_up(sensor);
    }

    if (0 == ret){
        cam_info("%s. power up sensor success.", __func__);
    }
    else{
        cam_err("%s. power up sensor fail.", __func__);
    }

    return ret;
}

int ov16b10INE_power_down(hwsensor_intf_t* si)
{
    int ret          = 0;
    sensor_t* sensor = NULL;

    if (!si){
        cam_err("%s. si is null.", __func__);
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

    if (0 == ret ){
        cam_info("%s. power down sensor success.", __func__);
    }
    else{
        cam_err("%s. power down sensor fail.", __func__);
    }
    hw_sensor_power_down_config(sensor->board_info);
    return ret;
}

int ov16b10INE_csi_enable(hwsensor_intf_t* si)
{
    (void)si;
    return 0;
}

int ov16b10INE_csi_disable(hwsensor_intf_t* si)
{
    (void)si;
    return 0;
}

static int ov16b10INE_match_id(hwsensor_intf_t* si, void * data)
{
    sensor_t* sensor              = NULL;
    struct sensor_cfg_data *cdata = NULL;
    if(NULL == si || NULL == data)
    {
        cam_err("%s. si or data is NULL.", __func__);
        return -EINVAL;
    }

    cam_info("%s enter.", __func__);
    sensor = I2S(si);
    if (NULL == sensor || NULL == sensor->board_info || NULL == sensor->board_info->name) {
        cam_err("%s. sensor or board_info is NULL.", __func__);
        return -EINVAL;
    }
    cdata = (struct sensor_cfg_data *)data;

    cdata->data = sensor->board_info->sensor_index;

    return 0;
}

static hwsensor_vtbl_t s_ov16b10INE_vtbl =
{
    .get_name    = ov16b10INE_get_name,
    .config      = ov16b10INE_config,
    .power_up    = ov16b10INE_power_up,
    .power_down  = ov16b10INE_power_down,
    .match_id    = ov16b10INE_match_id,
    .csi_enable  = ov16b10INE_csi_enable,
    .csi_disable = ov16b10INE_csi_disable,
};

static int ov16b10INE_config_power_on(hwsensor_intf_t* si)
{
    int ret = 0;
    mutex_lock(&ov16b10INE_power_lock);

    if (NULL == si || NULL == si->vtbl || NULL == si->vtbl->power_up)
    {
        cam_err("%s. si power_up is null", __func__);
        /*lint -e455 -esym(455,*)*/
        mutex_unlock(&ov16b10INE_power_lock);
        /*lint -e455 +esym(455,*)*/
        return -EINVAL;
    }

    if (!ov16b10INE_power_on){
        ret = si->vtbl->power_up(si);
        if (0 == ret) {
            ov16b10INE_power_on = true;
        } else {
            cam_err("%s. power up fail.", __func__);
        }
    } else {
        cam_err("%s camera has powered on",__func__);
    }

    /*lint -e455 -esym(455,*)*/
    mutex_unlock(&ov16b10INE_power_lock);
    /*lint -e455 +esym(455,*)*/

    return ret;
}

static int ov16b10INE_config_power_off(hwsensor_intf_t* si)
{
    int ret = 0;
    mutex_lock(&ov16b10INE_power_lock);

    if (NULL == si || NULL == si->vtbl || NULL == si->vtbl->power_down)
    {
        cam_err("%s. si power_down is null", __func__);
        /*lint -e455 -esym(455,*)*/
        mutex_unlock(&ov16b10INE_power_lock);
        /*lint -e455 +esym(455,*)*/
        return -EINVAL;
    }

    if (ov16b10INE_power_on){
        ret = si->vtbl->power_down(si);
        if (0 != ret) {
            cam_err("%s. power down fail.", __func__);
        }
        ov16b10INE_power_on = false;
    } else {
        cam_err("%s camera has powered off",__func__);
    }
    /*lint -e455 -esym(455,*)*/
    mutex_unlock(&ov16b10INE_power_lock);
    /*lint -e455 +esym(455,*)*/

    return ret;
}

static int ov16b10INE_config_match_id(hwsensor_intf_t* si, void *argp)
{
    int ret = 0;

    if (NULL == si || NULL == si->vtbl || NULL == si->vtbl->match_id)
    {
        cam_err("%s. si power_up is null", __func__);
        ret = -EINVAL;
    } else {
        ret = si->vtbl->match_id(si,argp);
    }

    return ret;
}

int ov16b10INE_config(hwsensor_intf_t* si, void  *argp)
{
    int ret =0;
    struct sensor_cfg_data *data = NULL;

    if (NULL == si || NULL == argp){
        cam_err("%s si or argp is null.\n", __func__);
        return -EINVAL;
    }

    data = (struct sensor_cfg_data *)argp;
    cam_debug("ov16b10INE cfgtype = %d",data->cfgtype);

    switch(data->cfgtype){
        case SEN_CONFIG_POWER_ON:
            ret = ov16b10INE_config_power_on(si);
            break;
        case SEN_CONFIG_POWER_OFF:
            ret = ov16b10INE_config_power_off(si);
            break;
        case SEN_CONFIG_WRITE_REG:
        case SEN_CONFIG_READ_REG:
        case SEN_CONFIG_WRITE_REG_SETTINGS:
        case SEN_CONFIG_READ_REG_SETTINGS:
        case SEN_CONFIG_ENABLE_CSI:
        case SEN_CONFIG_DISABLE_CSI:
            break;
        case SEN_CONFIG_MATCH_ID:
            ret = ov16b10INE_config_match_id(si, argp);
            break;
        case SEN_CONFIG_RESET_HOLD:
            break;
        case SEN_CONFIG_RESET_RELEASE:
            break;
        default:
            cam_err("%s cfgtype(%d) is error", __func__, data->cfgtype);
            break;
    }

    cam_debug("%s exit %d",__func__, ret);
    return ret;
}

static int32_t ov16b10INE_platform_probe(struct platform_device* pdev)
{
    int rc = 0;
    const struct of_device_id *id = NULL;
    hwsensor_intf_t *intf = NULL;
    sensor_t *sensor = NULL;
    struct device_node *np = NULL;
    if (NULL == pdev){
        cam_err("%s pdev is null.\n", __func__);
        return -EINVAL;
    }

    cam_info("enter %s",__func__);
    np = pdev->dev.of_node;
    if (NULL == np) {
        cam_err("%s of_node is NULL", __func__);
        return -ENODEV;
    }

    id = of_match_node(s_ov16b10INE_dt_match, np);
    if (!id) {
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

    mutex_init(&ov16b10INE_power_lock);
    sensor->dev = &pdev->dev;
    rc = hwsensor_register(pdev, intf);
    if (0 != rc){
        cam_err("%s hwsensor_register fail.\n", __func__);
        goto ov16b10INE_sensor_probe_fail;
    }
    s_pdev = pdev;
    rc = rpmsg_sensor_register(pdev, (void*)sensor);
    if (0 != rc){
        cam_err("%s rpmsg_sensor_register fail.\n", __func__);
        hwsensor_unregister(s_pdev);
        goto ov16b10INE_sensor_probe_fail;
    }
    s_sensor = sensor;

ov16b10INE_sensor_probe_fail:
    return rc;
}

static int __init ov16b10INE_init_module(void)
{
    cam_info("enter %s",__func__);
    return platform_driver_probe(&s_ov16b10INE_driver, ov16b10INE_platform_probe);
}

static void __exit ov16b10INE_exit_module(void)
{
    if( NULL != s_sensor)
    {
        rpmsg_sensor_unregister((void*)s_sensor);
        s_sensor = NULL;
    }
    if (NULL != s_pdev) {
        hwsensor_unregister(s_pdev);
        s_pdev = NULL;
    }
    platform_driver_unregister(&s_ov16b10INE_driver);
}
//lint -restore

/*lint -e528 -esym(528,*)*/
module_init(ov16b10INE_init_module);
module_exit(ov16b10INE_exit_module);
/*lint -e528 +esym(528,*)*/
/*lint -e753 -esym(753,*)*/
MODULE_DESCRIPTION("ov16b10INE");
MODULE_LICENSE("GPL v2");
/*lint -e753 +esym(753,*)*/

