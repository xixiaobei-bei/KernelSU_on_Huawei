 


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
#define SENSOR_REG_SETTING_DELAY_1_MS 1
#define GPIO_LOW_STATE       0
#define GPIO_HIGH_STATE      1

extern struct hw_csi_pad hw_csi_pad;
static hwsensor_vtbl_t s_hi1333RNE_vtbl;

//lint -save -e846 -e514 -e866 -e30 -e84 -e785 -e64 -e826 -e838 -e715 -e747 -e778 -e774 -e732 -e650 -e31 -e731 -e528 -e753 -e737


int hi1333RNE_config(hwsensor_intf_t* si, void  *argp);

static struct platform_device *s_pdev = NULL;
static sensor_t *s_sensor = NULL;
struct sensor_power_setting hw_hi1333RNE_power_up_setting[] = {

    //enable MIPI [GPIO-010]
    {
        .seq_type     = SENSOR_MIPI_EN,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //MIPI Swith to SCAM [GPIO-009]
    {
        .seq_type     = SENSOR_MIPI_SW,
        .config_val   = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //disable MCAM2 reset [GPIO179]
    {
        .seq_type     = SENSOR_SUSPEND2,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },

    //disable MCAM2 PWDN  [GPIO58]
    {
        .seq_type     = SENSOR_PWDN,
        .config_val   = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //disable MCAM0 reset [GPIO18]
    {
        .seq_type     = SENSOR_SUSPEND,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },

    //MCAM0 DVDD 1.1V [LDO20]
    {
        .seq_type     = SENSOR_DVDD2,
        .config_val   = LDO_VOLTAGE_1P1V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //SCAM0 IOVDD 1.80V [LDO33]
    {
        .seq_type     = SENSOR_IOVDD,
        .config_val   = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //SCAM0 AVDD 2.80V [LDO19]
    {
        .seq_type     = SENSOR_AVDD,
        .config_val   = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },

    //SCAM0 DVDD 1.2V [LDO32]
    {
        .seq_type     = SENSOR_DVDD,
        .config_val   = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //SCAM0 CLK
    {
        .seq_type     = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //SCAM0 RESET [GPIO17]
    {
        .seq_type     = SENSOR_RST,
        .config_val   = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },

};
struct sensor_power_setting hw_hi1333RNE_power_down_setting[] = {
    {
        .seq_type     = SENSOR_RST,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    {
        .seq_type     = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //SCAM0 DVDD 1.2V [LDO32]
    {
        .seq_type     = SENSOR_DVDD,
        .config_val   = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //SCAM0 AVDD 2.80V [LDO19]
    {
        .seq_type     = SENSOR_AVDD,
        .config_val   = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },

    //SCAM0 IOVDD 1.80V [LDO33]
    {
        .seq_type     = SENSOR_IOVDD,
        .config_val   = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //disable MCAM2 reset [GPIO179]
    {
        .seq_type     = SENSOR_SUSPEND2,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },

    //disable MCAM2 PWDN  [GPIO58]
    {
        .seq_type     = SENSOR_PWDN,
        .config_val   = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //MCAM0 DVDD 1.1V [LDO20]
    {
        .seq_type     = SENSOR_DVDD2,
        .config_val   = LDO_VOLTAGE_1P1V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //enable MCAM0 reset [GPIO-018]
    {
        .seq_type     = SENSOR_SUSPEND,
        .config_val   = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },

    //MIPI Swith to MCAM [GPIO-009]
    {
        .seq_type     = SENSOR_MIPI_SW,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //disable MIPI [GPIO-010]
    {
        .seq_type     = SENSOR_MIPI_EN,
        .config_val   = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },

};

struct sensor_power_setting hw_hi1333RNE_v4_power_up_setting[] = {

    //MCAM0 AFVDD & enable MIPI 3V [LDO25]
    {
        .seq_type     = SENSOR_VCM_AVDD,
        .data         = (void*)"cameravcm-vcc",
        .config_val   = LDO_VOLTAGE_V3PV,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //MIPI Swith to SCAM [GPIO-009]
    {
        .seq_type     = SENSOR_MIPI_SW,
        .config_val   = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //disable MCAM2 reset [GPIO179]
    {
        .seq_type     = SENSOR_SUSPEND2,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },

    //disable MCAM2 PWDN  [GPIO58]
    {
        .seq_type     = SENSOR_PWDN,
        .config_val   = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //disable MCAM0 reset [GPIO18]
    {
        .seq_type     = SENSOR_SUSPEND,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },

    //MCAM0 DVDD 1.1V [LDO20]
    {
        .seq_type     = SENSOR_DVDD2,
        .config_val   = LDO_VOLTAGE_1P1V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //SCAM0 IOVDD 1.80V [LDO33]
    {
        .seq_type     = SENSOR_IOVDD,
        .config_val   = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //SCAM0 AVDD 2.80V [LDO19]
    {
        .seq_type     = SENSOR_AVDD,
        .config_val   = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },

    //SCAM0 DVDD 1.2V [LDO32]
    {
        .seq_type     = SENSOR_DVDD,
        .config_val   = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //SCAM0 CLK
    {
        .seq_type     = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //SCAM0 RESET [GPIO17]
    {
        .seq_type     = SENSOR_RST,
        .config_val   = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },

};
struct sensor_power_setting hw_hi1333RNE_v4_power_down_setting[] = {
    {
        .seq_type     = SENSOR_RST,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    {
        .seq_type     = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //SCAM0 DVDD 1.2V [LDO32]
    {
        .seq_type     = SENSOR_DVDD,
        .config_val   = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //SCAM0 AVDD 2.80V [LDO19]
    {
        .seq_type     = SENSOR_AVDD,
        .config_val   = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },

    //SCAM0 IOVDD 1.80V [LDO33]
    {
        .seq_type     = SENSOR_IOVDD,
        .config_val   = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //disable MCAM2 reset [GPIO179]
    {
        .seq_type     = SENSOR_SUSPEND2,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },

    //disable MCAM2 PWDN  [GPIO58]
    {
        .seq_type     = SENSOR_PWDN,
        .config_val   = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //MCAM0 DVDD 1.1V [LDO20]
    {
        .seq_type     = SENSOR_DVDD2,
        .config_val   = LDO_VOLTAGE_1P1V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //enable MCAM0 reset [GPIO-018]
    {
        .seq_type     = SENSOR_SUSPEND,
        .config_val   = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },

    //MIPI Swith to MCAM [GPIO-009]
    {
        .seq_type     = SENSOR_MIPI_SW,
        .config_val   = SENSOR_GPIO_LOW,//fix me
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //MCAM0 AFVDD & disable MIPI 3V [LDO25]
    {
        .seq_type     = SENSOR_VCM_AVDD,
        .data         = (void*)"cameravcm-vcc",
        .config_val   = LDO_VOLTAGE_V3PV,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },
};

atomic_t volatile hi1333RNE_powered = ATOMIC_INIT(0);
struct mutex hi1333RNE_power_lock;
static sensor_t s_hi1333RNE =
{
    .intf = { .vtbl = &s_hi1333RNE_vtbl, },
    .power_setting_array = 
    {
        .size          = ARRAY_SIZE(hw_hi1333RNE_power_up_setting),
        .power_setting = hw_hi1333RNE_power_up_setting,
    },
    .power_down_setting_array = 
    {
        .size          = ARRAY_SIZE(hw_hi1333RNE_power_down_setting),
        .power_setting = hw_hi1333RNE_power_down_setting,
    },
    .p_atpowercnt      = &hi1333RNE_powered,
};

static sensor_t s_hi1333RNE_v4 =
{
    .intf = { .vtbl = &s_hi1333RNE_vtbl, },
    .power_setting_array =
    {
        .size          = ARRAY_SIZE(hw_hi1333RNE_v4_power_up_setting),
        .power_setting = hw_hi1333RNE_v4_power_up_setting,
    },
    .power_down_setting_array =
    {
        .size          = ARRAY_SIZE(hw_hi1333RNE_v4_power_down_setting),
        .power_setting = hw_hi1333RNE_v4_power_down_setting,
    },
    .p_atpowercnt      = &hi1333RNE_powered,
};

static const struct of_device_id s_hi1333RNE_dt_match[] =
{
    {
        .compatible = "huawei,hi1333RNE",
        .data       = &s_hi1333RNE.intf,
    },
    {
        .compatible = "huawei,hi1333RNE_v4",
        .data       = &s_hi1333RNE_v4.intf,
    },
    {
    },
};

MODULE_DEVICE_TABLE(of, s_hi1333RNE_dt_match);

static struct platform_driver s_hi1333RNE_driver =
{
    .driver =
    {
        .name           = "huawei,hi1333RNE",
        .owner          = THIS_MODULE,
        .of_match_table = s_hi1333RNE_dt_match,
    },
};

char const* hi1333RNE_get_name(hwsensor_intf_t* si)
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

int hi1333RNE_power_up(hwsensor_intf_t* si)
{
    int ret          = 0;
    sensor_t* sensor = NULL;

    if (!si){
        cam_err("%s. si is null.", __func__);
        return -1;
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

int hi1333RNE_power_down(hwsensor_intf_t* si)
{
    int ret          = 0;
    sensor_t* sensor = NULL;

    if (!si){
        cam_err("%s. si is null.", __func__);
        return -1;
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

int hi1333RNE_csi_enable(hwsensor_intf_t* si)
{
    (void)si;
    return 0;
}

int hi1333RNE_csi_disable(hwsensor_intf_t* si)
{
    (void)si;
    return 0;
}

static int hi1333RNE_match_id(hwsensor_intf_t* si, void * data)
{
    sensor_t* sensor = NULL;
    struct sensor_cfg_data *cdata = NULL;
    if(NULL == si || NULL == data)
    {
        cam_err("%s. si or data is NULL.", __func__);
        return -1;
    }

    sensor = I2S(si);
    if (NULL == sensor || NULL == sensor->board_info || NULL == sensor->board_info->name) {
        cam_err("%s. sensor or board_info is NULL.", __func__);
        return -1;
    }
    cdata = (struct sensor_cfg_data *)data;

    cam_info("%s enter.", __func__);

    cdata->data = sensor->board_info->sensor_index;

    return 0;
}

static hwsensor_vtbl_t s_hi1333RNE_vtbl =
{
    .get_name    = hi1333RNE_get_name,
    .config      = hi1333RNE_config,
    .power_up    = hi1333RNE_power_up,
    .power_down  = hi1333RNE_power_down,
    .match_id    = hi1333RNE_match_id,
    .csi_enable  = hi1333RNE_csi_enable,
    .csi_disable = hi1333RNE_csi_disable,
};

int hi1333RNE_config(hwsensor_intf_t* si, void  *argp)
{
    int ret =0;
    struct sensor_cfg_data *data = NULL;
    static bool hi1333RNE_power_on = false;

    if (NULL == si || NULL == argp){
        cam_err("%s si or argp is null.\n", __func__);
        return -1;
    }

    data = (struct sensor_cfg_data *)argp;
    cam_debug("hi1333RNE cfgtype = %d",data->cfgtype);

    switch(data->cfgtype){
        case SEN_CONFIG_POWER_ON:
            mutex_lock(&hi1333RNE_power_lock);
            if (!hi1333RNE_power_on) {
                if(NULL == si->vtbl->power_up){
                    cam_err("%s. si->vtbl->power_up is null.", __func__);
                    ret=-EINVAL;
                }else{
                    ret = si->vtbl->power_up(si);
                    if (0 == ret) {
                        hi1333RNE_power_on = true;
                    } else {
                        cam_err("%s. power up fail.", __func__);
                    }
                }
            }
            /*lint -e455 -esym(455,*)*/
            mutex_unlock(&hi1333RNE_power_lock);
            /*lint -e455 +esym(455,*)*/
            break;
        case SEN_CONFIG_POWER_OFF:
            mutex_lock(&hi1333RNE_power_lock);
            if (hi1333RNE_power_on) {
                if(NULL == si->vtbl->power_down){
                    cam_err("%s. si->vtbl->power_down is null.", __func__);
                    ret=-EINVAL;
                }else{
                    ret = si->vtbl->power_down(si);
                    if (0 != ret) {
                        cam_err("%s. power_down fail.", __func__);
                    }
                    hi1333RNE_power_on = false;
		}
            }
            /*lint -e455 -esym(455,*)*/
            mutex_unlock(&hi1333RNE_power_lock);
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

static int32_t hi1333RNE_platform_probe(struct platform_device* pdev)
{
    int rc = 0;
    const struct of_device_id *id = NULL;
    hwsensor_intf_t *intf = NULL;
    sensor_t *sensor = NULL;
    struct device_node *np = NULL;

    if (NULL == pdev){
        cam_err("%s pdev is null.\n", __func__);
        return -1;
    }
    
    cam_info("enter %s",__func__);
    np = pdev->dev.of_node;
    if (NULL == np) {
        cam_err("%s of_node is NULL", __func__);
        return -ENODEV;
    }

    id = of_match_node(s_hi1333RNE_dt_match, np);
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

    mutex_init(&hi1333RNE_power_lock);
    sensor->dev = &pdev->dev;
    rc = hwsensor_register(pdev, intf);
    if (0 != rc){
        cam_err("%s hwsensor_register fail.\n", __func__);
        goto hi1333RNE_sensor_probe_fail;
    }
    s_pdev = pdev;
    rc = rpmsg_sensor_register(pdev, (void*)sensor);
    if (0 != rc){
        cam_err("%s rpmsg_sensor_register fail.\n", __func__);
        hwsensor_unregister(s_pdev);
        goto hi1333RNE_sensor_probe_fail;
    }
    s_sensor = sensor;

hi1333RNE_sensor_probe_fail:
    return rc;
}

static int __init hi1333RNE_init_module(void)
{
    cam_info("enter %s",__func__);
    return platform_driver_probe(&s_hi1333RNE_driver, hi1333RNE_platform_probe);
}

static void __exit hi1333RNE_exit_module(void)
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
    platform_driver_unregister(&s_hi1333RNE_driver);
}
//lint -restore

/*lint -e528 -esym(528,*)*/
module_init(hi1333RNE_init_module);
module_exit(hi1333RNE_exit_module);
/*lint -e528 +esym(528,*)*/
/*lint -e753 -esym(753,*)*/
MODULE_DESCRIPTION("hi1333RNE");
MODULE_LICENSE("GPL v2");
/*lint -e753 +esym(753,*)*/

