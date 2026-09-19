


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

#define POWER_DELAY_1               1

static hwsensor_vtbl_t s_imx371ANE_vtbl;

struct mutex imx371ANE_power_lock;
//lint -save -e846 -e514 -e866 -e30 -e84 -e785 -e64 -e826 -e838 -e715 -e747 -e778 -e774 -e732 -e650 -e31 -e731 -e528 -e753 -e737
int imx371ANE_config(hwsensor_intf_t* si, void  *argp);

static struct platform_device *s_pdev = NULL;
static sensor_t *s_sensor = NULL;
struct sensor_power_setting hw_imx371ANE_power_up_setting[] = {
    //MCAM0 AFVDD & mipi enable 3V [LDO25]
    {
        .seq_type     = SENSOR_VCM_AVDD,
        .data         = (void*)"cameravcm-vcc",
        .config_val   = LDO_VOLTAGE_V3PV,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_1,
    },

    //disable MCAM0 reset [GPIO18]
    {
        .seq_type     = SENSOR_SUSPEND2,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },

    //MCAM0 DVDD 1.1V [LDO20]
    {
        .seq_type     = SENSOR_DVDD,
        .data         = (void*)"back-main-sensor-dvdd",
        .config_val   = LDO_VOLTAGE_1P1V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_1,
    },

    //SCAM IOVDD 1.80V [LDO33]
    {
        .seq_type     = SENSOR_IOVDD,
        .config_val   = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_1,
    },

    //SCAM AVDD 2.80V [LDO19]
    {
        .seq_type     = SENSOR_AVDD,
        .config_val   = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },

    //SCAM DVDD 1.1V [LDO32]
    {
        .seq_type     = SENSOR_DVDD2,
        .config_val   = LDO_VOLTAGE_1P1V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_1,
    },

    //MIPI Swith to SCAM [GPIO207]
    {
        .seq_type     = SENSOR_MIPI_SW,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_1,
    },

    //SCAM CLK
    {
        .seq_type     = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_1,
    },

    //SCAM RESET [GPIO17]
    {
        .seq_type     = SENSOR_RST,
        .config_val   = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_1,
    },

};
struct sensor_power_setting hw_imx371ANE_power_down_setting[] = {

    {
        .seq_type     = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_1,
    },

    {
        .seq_type     = SENSOR_RST,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_1,
    },

    //MIPI Swith to SCAM [GPIO207]
    {
        .seq_type     = SENSOR_MIPI_SW,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_1,
    },

    //SCAM DVDD 1.1V [LDO32]
    {
        .seq_type     = SENSOR_DVDD2,
        .config_val   = LDO_VOLTAGE_1P1V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_1,
    },
    
    //SCAM AVDD 2.80V [LDO19]
    {
        .seq_type     = SENSOR_AVDD,
        .config_val   = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },
    //SCAM IOVDD 1.80V [LDO33]
    {
        .seq_type     = SENSOR_IOVDD,
        .config_val   = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_1,
    },
    //MCAM0 DVDD 1.1V [LDO20]
    {
        .seq_type     = SENSOR_DVDD,
        .data         = (void*)"back-main-sensor-dvdd",
        .config_val   = LDO_VOLTAGE_1P1V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_1,
    },

    //disable MCAM0 reset [GPIO18]
    {
        .seq_type     = SENSOR_SUSPEND2,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },

    //MCAM0 AFVDD 3V & disable MIPI
    {
        .seq_type     = SENSOR_VCM_AVDD,
        .data         = (void*)"cameravcm-vcc",
        .config_val   = LDO_VOLTAGE_V3PV,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_1,
    },
};

atomic_t volatile imx371ANE_powered = ATOMIC_INIT(0);
static sensor_t s_imx371ANE =
{
    .intf = { .vtbl = &s_imx371ANE_vtbl, },
    .power_setting_array = {
            .size = ARRAY_SIZE(hw_imx371ANE_power_up_setting),
            .power_setting = hw_imx371ANE_power_up_setting,
     },
    .power_down_setting_array = {
            .size = ARRAY_SIZE(hw_imx371ANE_power_down_setting),
            .power_setting = hw_imx371ANE_power_down_setting,
    },
    .p_atpowercnt = &imx371ANE_powered,
};

static const struct of_device_id s_imx371ANE_dt_match[] =
{
    {
        .compatible = "huawei,imx371ANE",
        .data = &s_imx371ANE.intf,
    },
    {
    },
};

MODULE_DEVICE_TABLE(of, s_imx371ANE_dt_match);

static struct platform_driver s_imx371ANE_driver =
{
    .driver =
    {
        .name = "huawei,imx371ANE",
        .owner = THIS_MODULE,
        .of_match_table = s_imx371ANE_dt_match,
    },
};

char const* imx371ANE_get_name(hwsensor_intf_t* si)
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

int imx371ANE_power_up(hwsensor_intf_t* si)
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

int imx371ANE_power_down(hwsensor_intf_t* si)
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

int imx371ANE_csi_enable(hwsensor_intf_t* si)
{
    (void)si;
    return 0;
}

int imx371ANE_csi_disable(hwsensor_intf_t* si)
{
    (void)si;
    return 0;
}

static int imx371ANE_match_id(hwsensor_intf_t* si, void * data)
{
    sensor_t* sensor = NULL;
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

static hwsensor_vtbl_t s_imx371ANE_vtbl =
{
    .get_name    = imx371ANE_get_name,
    .config      = imx371ANE_config,
    .power_up    = imx371ANE_power_up,
    .power_down  = imx371ANE_power_down,
    .match_id    = imx371ANE_match_id,
    .csi_enable  = imx371ANE_csi_enable,
    .csi_disable = imx371ANE_csi_disable,
};

int imx371ANE_config(hwsensor_intf_t* si, void  *argp)
{
    int ret =0;
    struct sensor_cfg_data *data = NULL;
    static bool imx371ANE_power_on = false;

    if (NULL == si || NULL == argp){
        cam_err("%s si or argp is null.\n", __func__);
        return -EINVAL;
    }

    data = (struct sensor_cfg_data *)argp;
    cam_debug("imx371ANE cfgtype = %d",data->cfgtype);

    switch(data->cfgtype){
        case SEN_CONFIG_POWER_ON:
            mutex_lock(&imx371ANE_power_lock);
            if (!imx371ANE_power_on) {
                if(NULL == si->vtbl->power_up){
                    cam_err("%s. si->vtbl->power_up is null.", __func__);
                    ret=-EINVAL;
                }else{
                    ret = si->vtbl->power_up(si);
                    if (0 == ret) {
                        imx371ANE_power_on = true;
                    } else {
                        cam_err("%s. power up fail.", __func__);
                    }
                }
            }
            /*lint -e455 -esym(455,*)*/
            mutex_unlock(&imx371ANE_power_lock);
            /*lint -e455 +esym(455,*)*/
            break;
        case SEN_CONFIG_POWER_OFF:
            mutex_lock(&imx371ANE_power_lock);
            if (imx371ANE_power_on) {
                if(NULL == si->vtbl->power_down){
                    cam_err("%s. si->vtbl->power_down is null.", __func__);
                    ret=-EINVAL;
                }else{
                    ret = si->vtbl->power_down(si);
                    if (0 != ret) {
                        cam_err("%s. power_down fail.", __func__);
                    }
                    imx371ANE_power_on = false;
                }
            }
            /*lint -e455 -esym(455,*)*/
            mutex_unlock(&imx371ANE_power_lock);
            /*lint -e455 +esym(455,*)*/
            break;
        case SEN_CONFIG_WRITE_REG:
            break;
        case SEN_CONFIG_READ_REG:
            break;
        case SEN_CONFIG_WRITE_REG_SETTINGS:
            break;
        case SEN_CONFIG_READ_REG_SETTINGS:
            break;
        case SEN_CONFIG_ENABLE_CSI:
            break;
        case SEN_CONFIG_DISABLE_CSI:
            break;
        case SEN_CONFIG_MATCH_ID:
            if(NULL == si->vtbl->match_id){
                cam_err("%s. si->vtbl->match_id is null.", __func__);
                ret=-EINVAL;
            }else{
                ret = si->vtbl->match_id(si,argp);
            }
            break;
        default:
            cam_err("%s cfgtype(%d) is error", __func__, data->cfgtype);
            break;
    }

    cam_debug("%s exit %d",__func__, ret);
    return ret;
}

static int32_t imx371ANE_platform_probe(struct platform_device* pdev)
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

    id = of_match_node(s_imx371ANE_dt_match, np);
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

    mutex_init(&imx371ANE_power_lock);
    sensor->dev = &pdev->dev;
    rc = hwsensor_register(pdev, intf);
    if (0 != rc){
        cam_err("%s hwsensor_register fail.\n", __func__);
        goto imx371ANE_sensor_probe_fail;
    }
    s_pdev = pdev;
    rc = rpmsg_sensor_register(pdev, (void*)sensor);
    if (0 != rc){
        cam_err("%s rpmsg_sensor_register fail.\n", __func__);
        hwsensor_unregister(s_pdev);
        goto imx371ANE_sensor_probe_fail;
    }
    s_sensor = sensor;

imx371ANE_sensor_probe_fail:
    return rc;
}

static int __init imx371ANE_init_module(void)
{
    cam_info("enter %s",__func__);
    return platform_driver_probe(&s_imx371ANE_driver, imx371ANE_platform_probe);
}

static void __exit imx371ANE_exit_module(void)
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
    platform_driver_unregister(&s_imx371ANE_driver);
}
//lint -restore

/*lint -e528 -esym(528,*)*/
module_init(imx371ANE_init_module);
module_exit(imx371ANE_exit_module);
/*lint -e528 +esym(528,*)*/
/*lint -e753 -esym(753,*)*/
MODULE_DESCRIPTION("imx371ANE");
MODULE_LICENSE("GPL v2");
/*lint -e753 +esym(753,*)*/
