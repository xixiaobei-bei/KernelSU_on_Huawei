 


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

//lint -save -e846 -e866 -e826 -e785 -e838 -e715 -e747 -e774 -e778 -e732 -e731
//lint -save -e514 -e30 -e84 -e64 -e650 -e737 -e31 -e64 -esym(528,*) -esym(753,*)
#define DELAY_0MS    0
#define DELAY_1MS    1
#define DELAY_5MS    5

static hwsensor_vtbl_t s_imx258JKM_vtbl;

#define I2S(i) container_of((i), sensor_t, intf)
#define Sensor2Pdev(s) container_of((s).dev, struct platform_device, dev)

static bool s_imx258JKM_power_on = false; //false for power down, ture for power up
struct mutex imx258JKM_power_lock;
static struct platform_device *s_pdev = NULL;
static sensor_t *s_sensor = NULL;

struct sensor_power_setting imx258JKM_power_up_setting[] = {
    //disable SCAM0 RESET2 [GPIO31]
    {
        .seq_type = SENSOR_RST2,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_0MS,
    },
    //disable MCAM0 reset [GPIO32]
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_5MS,
    },
    //MCAM0 AFVDD 3.0V [LDO25]
    {
        .seq_type = SENSOR_VCM_AVDD,
        .config_val   = LDO_VOLTAGE_V3PV,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_1MS,
    },
    //MIPI Swith to MCAM1 [GPIO94]
    {
        .seq_type = SENSOR_MIPI_SW,
        .config_val = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_1MS,
    },

    //SCAM0 AVDD 2.8V [LDO13]
    {
        .seq_type = SENSOR_AVDD2,
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_0MS,
    },

    //SCAM0 DVDD 1.1V [LDO32]
    {
        .seq_type = SENSOR_DVDD2,
        .config_val = LDO_VOLTAGE_1P1V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_1MS,
    },

    //MCAM0 AVDD 2.80V [LDO19]
    {
        .seq_type = SENSOR_AVDD,
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_0MS,
    },

    //MCAM0 DVDD 1.2V [LDO20]
    {
        .seq_type = SENSOR_DVDD,
        .config_val = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_0MS,
    },
    //MCAM0 IOVDD 1.80V [LDO21]
    {
        .seq_type = SENSOR_IOVDD,
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_1MS,
    },
    //MCAM0 CLK
    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_1MS,
    },
    //MCAM0 reset [GPIO32]
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_1MS,
    },
};

struct sensor_power_setting imx258JKM_power_down_setting[] = {
    //MCAM0 RESET
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_1MS,
    },
    //MCAM0 CLK
    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_1MS,
    },
    //MCAM0 IOVDD 1.80V
    {
        .seq_type = SENSOR_IOVDD,
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_0MS,
    },
    //MCAM0 DVDD 1.2V
    {
        .seq_type = SENSOR_DVDD,
        .config_val = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_0MS,
    },
    //MCAM0 AVDD 2.80V
    {
        .seq_type = SENSOR_AVDD,
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_1MS,
    },
    //SCAM0 DVDD 1.1V
    {
        .seq_type = SENSOR_DVDD2,
        .config_val = LDO_VOLTAGE_1P1V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_1MS,
    },
    //SCAM0 AVDD 2.8V
    {
        .seq_type = SENSOR_AVDD2,
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_0MS,
    },
    //MIPI Swith
    {
        .seq_type     = SENSOR_MIPI_SW,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = DELAY_1MS,
    },
    //MCAM0 AFVDD 3.0V
    {
        .seq_type = SENSOR_VCM_AVDD,
        .config_val   = LDO_VOLTAGE_V3PV,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_1MS,
    },
    //disable SCAM0 reset [GPIO031]
    {
        .seq_type     = SENSOR_RST2,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = DELAY_0MS,
    },
};

atomic_t volatile imx258JKM_powered = ATOMIC_INIT(0);

static sensor_t s_imx258JKM =
{
    .intf = { .vtbl = &s_imx258JKM_vtbl, },
    .power_setting_array = {
        .size = ARRAY_SIZE(imx258JKM_power_up_setting),
        .power_setting = imx258JKM_power_up_setting,
    },
    .power_down_setting_array = {
            .size = ARRAY_SIZE(imx258JKM_power_down_setting),
            .power_setting = imx258JKM_power_down_setting,
    },
    .p_atpowercnt = &imx258JKM_powered,
};

static const struct of_device_id s_imx258JKM_dt_match[] =
{
    {
        .compatible = "huawei,imx258JKM",
        .data = &s_imx258JKM.intf,
    },
    {
    } /* terminate list */
};

MODULE_DEVICE_TABLE(of, s_imx258JKM_dt_match);
/* platform driver struct */
static int32_t imx258JKM_platform_probe(struct platform_device* pdev);
static int32_t imx258JKM_platform_remove(struct platform_device* pdev);
static struct platform_driver s_imx258JKM_driver =
{
    .probe = imx258JKM_platform_probe,
    .remove = imx258JKM_platform_remove,
    .driver =
    {
        .name = "huawei,imx258JKM",
        .owner = THIS_MODULE,
        .of_match_table = s_imx258JKM_dt_match,
    },
};

static char const*imx258JKM_get_name(hwsensor_intf_t* si)
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

static int imx258JKM_power_up(hwsensor_intf_t* si)
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

    ret = hw_sensor_power_up_config(s_imx258JKM.dev, sensor->board_info);
    if (0 == ret ){
        cam_info("%s. power up config success.", __func__);
    }else{
        cam_err("%s. power up config fail.", __func__);
        return ret;
    }

    if (hw_is_fpga_board()) {
        cam_info("%s powerup by isp on FPGA", __func__);
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

static int imx258JKM_power_down(hwsensor_intf_t* si)
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
        cam_info("%s poweroff by isp on FPGA", __func__);
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

    hw_sensor_power_down_config(sensor->board_info);

    return ret;
}

static int imx258JKM_csi_enable(hwsensor_intf_t* si)
{
    (void)si;
    return 0;
}

static int imx258JKM_csi_disable(hwsensor_intf_t* si)
{
    (void)si;
    return 0;
}

static int imx258JKM_match_id(hwsensor_intf_t* si, void * data)
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
        cam_err("%s. sensor or board_info or name is NULL.", __func__);
        return -EINVAL;
    }
    cdata = (struct sensor_cfg_data *)data;

    cdata->data = sensor->board_info->sensor_index;

    return 0;
}

static int imx258JKM_config_power_on(hwsensor_intf_t* si)
{
    int ret = 0;
    mutex_lock(&imx258JKM_power_lock);

    if (NULL == si || NULL == si->vtbl || NULL == si->vtbl->power_up)
    {
        cam_err("%s. si power_up is null", __func__);
        /*lint -e455 -esym(455,*)*/
        mutex_unlock(&imx258JKM_power_lock);
        /*lint -e455 +esym(455,*)*/
        return -EINVAL;
    }

    if (!s_imx258JKM_power_on){
        ret = si->vtbl->power_up(si);
        if (0 == ret) {
            s_imx258JKM_power_on = true;
        } else {
            cam_err("%s. power up fail.", __func__);
        }
    } else {
        cam_err("%s camera has powered on",__func__);
    }

    /*lint -e455 -esym(455,*)*/
    mutex_unlock(&imx258JKM_power_lock);
    /*lint -e455 +esym(455,*)*/
    return ret;
}
static int imx258JKM_config_power_off(hwsensor_intf_t* si)
{
    int ret = 0;
    mutex_lock(&imx258JKM_power_lock);

    if (NULL == si || NULL == si->vtbl || NULL == si->vtbl->power_down)
    {
        cam_err("%s. si power_down is null", __func__);
        /*lint -e455 -esym(455,*)*/
        mutex_unlock(&imx258JKM_power_lock);
        /*lint -e455 +esym(455,*)*/
        return -EINVAL;
    }

    if (s_imx258JKM_power_on){
        ret = si->vtbl->power_down(si);
        if (0 != ret) {
            cam_err("%s. power down fail.", __func__);
        }
        s_imx258JKM_power_on = false;
    } else {
        cam_err("%s camera has powered off",__func__);
    }
    /*lint -e455 -esym(455,*)*/
    mutex_unlock(&imx258JKM_power_lock);
    /*lint -e455 +esym(455,*)*/
    return ret;
}

static int imx258JKM_config_match_id(hwsensor_intf_t* si, void *argp)
{
    int ret = 0;

    if (NULL == si || NULL == si->vtbl || NULL == si->vtbl->match_id)
    {
        cam_err("%s. si power_up is null", __func__);
        ret = -EINVAL;
    }else{
        ret = si->vtbl->match_id(si,argp);
    }
    return ret;
}

static int imx258JKM_config(hwsensor_intf_t* si, void  *argp)
{
    struct sensor_cfg_data *data = NULL;
    int ret = 0;

    if ((NULL == si) || (NULL == argp) || (NULL == si->vtbl)) {
        cam_err("%s : si, argp or si->vtbl is null", __func__);
        return -EINVAL;
    }

    data = (struct sensor_cfg_data *)argp;
    cam_debug("imx258JKM cfgtype = %d",data->cfgtype);
    switch (data->cfgtype) {
        case SEN_CONFIG_POWER_ON:
            ret = imx258JKM_config_power_on(si);
            break;
        case SEN_CONFIG_POWER_OFF:
            ret = imx258JKM_config_power_off(si);
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
            ret = imx258JKM_config_match_id(si,argp);
            break;
        case SEN_CONFIG_RESET_HOLD:
            break;
        case SEN_CONFIG_RESET_RELEASE:
            break;
        default:
            cam_err("%s cfgtype(%d) is error", __func__, data->cfgtype);
            break;
    }
    return ret;
}


static hwsensor_vtbl_t
s_imx258JKM_vtbl =
{
    .get_name = imx258JKM_get_name,
    .config = imx258JKM_config,
    .power_up = imx258JKM_power_up,
    .power_down = imx258JKM_power_down,
    .match_id = imx258JKM_match_id,
    .csi_enable = imx258JKM_csi_enable,
    .csi_disable = imx258JKM_csi_disable,
};
/* individual driver data for each device */




static int32_t imx258JKM_platform_probe(struct platform_device* pdev)
{
    int rc = 0;

    const struct of_device_id *id = NULL;
    hwsensor_intf_t *intf = NULL;
    sensor_t *sensor = NULL;
    struct device_node *np = NULL;
    cam_info("enter %s ",__func__);

    if (NULL == pdev) {
        cam_err("%s pdev is NULL", __func__);
        return -EINVAL;
    }

    mutex_init(&imx258JKM_power_lock);
    np = pdev->dev.of_node;
    if (!np) {
        cam_err("%s of_node is NULL", __func__);
        return -ENODEV;
    }

    id = of_match_node(s_imx258JKM_dt_match, np);
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

    sensor->dev = &pdev->dev;

    rc = hwsensor_register(pdev, intf);
    if (rc < 0) {
        cam_err("%s hwsensor_register failed rc %d\n", __func__, rc);
        return -ENODEV;
    }
    s_pdev = pdev;

    rc = rpmsg_sensor_register(pdev, (void*)sensor);
    if (rc < 0) {
        hwsensor_unregister(s_pdev);
        s_pdev = NULL;
        cam_err("%s rpmsg_sensor_register failed rc %d\n", __func__, rc);
        return -ENODEV;
    }
    s_sensor = sensor;

    return rc;
}

static int32_t imx258JKM_platform_remove(struct platform_device * pdev)
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
    return 0;
}
static int __init imx258JKM_init_module(void)
{
    cam_info("enter %s",__func__);
    return platform_driver_probe(&s_imx258JKM_driver,
            imx258JKM_platform_probe);
}

static void __exit imx258JKM_exit_module(void)
{
    platform_driver_unregister(&s_imx258JKM_driver);
}
//lint -restore

/*lint -e528 -esym(528,*)*/
module_init(imx258JKM_init_module);
module_exit(imx258JKM_exit_module);
/*lint -e528 +esym(528,*)*/
/*lint -e753 -esym(753,*)*/
MODULE_DESCRIPTION("imx258JKM");
MODULE_LICENSE("GPL v2");
/*lint -e753 +esym(753,*)*/

