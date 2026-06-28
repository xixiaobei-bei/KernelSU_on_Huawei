 


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
static hwsensor_vtbl_t s_hi1333HANDEL_vtbl;
static bool power_on_status = false;//false: power off, true:power on
//lint -save -e846 -e514 -e866 -e30 -e84 -e785 -e64 -e826 -e838 -e715 -e747 -e778 -e774 -e732 -e650 -e31 -e731 -e528 -e753 -e737


int hi1333HANDEL_config(hwsensor_intf_t* si, void  *argp);

struct sensor_power_setting hw_hi1333HANDEL_power_up_setting[] = {

    //disable sec camera reset
    {
        .seq_type     = SENSOR_SUSPEND,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },

    //MCAM1 AFVDD 3V
    {
        .seq_type = SENSOR_VCM_AVDD,
        .data = (void*)"cameravcm-vcc",
        .config_val   = LDO_VOLTAGE_V3PV,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //MCAM1 VCM PD Enable
    {
        .seq_type = SENSOR_VCM_PWDN,
        .config_val = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = SENSOR_REG_SETTING_DELAY_1_MS,
    },

	//MCAM1 IOVDD 1.80V
    {
        .seq_type     = SENSOR_IOVDD,
        .data         = (void*)"main-sensor-iovdd",
        .config_val   = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },
    //MCAM1 AVDD 2.80V
    {
        .seq_type     = SENSOR_AVDD2,
        .data         = (void*)"main-sensor-avdd",
        .config_val   = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },
    //MCAM1 DVDD 1.2V
    {
        .seq_type     = SENSOR_DVDD2,
        .config_val   = LDO_VOLTAGE_1P25V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //SCAM AVDD 2.80V
    {
        .seq_type     = SENSOR_AVDD,
        .data         = (void*)"slave-sensor-avdd",
        .config_val   = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },

    //SCAM DVDD1.2V
    {
        .seq_type     = SENSOR_DVDD,
        .config_val   = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },
    {
        .seq_type     = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    {
        .seq_type     = SENSOR_RST,
        .config_val   = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },
};

struct sensor_power_setting hw_hi1333HANDEL_power_down_setting[] = {
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

    //SCAM DVDD1.2V
    {
        .seq_type     = SENSOR_DVDD,
        .config_val   = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },
    //SCAM AVDD 2.80V
    {
        .seq_type     = SENSOR_AVDD,
        .data         = (void*)"slave-sensor-avdd",
        .config_val   = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },

    //MCAM1 DVDD 1.2V
    {
        .seq_type     = SENSOR_DVDD2,
        .config_val   = LDO_VOLTAGE_1P25V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },

    //MCAM1 AVDD 2.80V
    {
        .seq_type     = SENSOR_AVDD2,
        .data         = (void*)"main-sensor-avdd",
        .config_val   = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },
    //MCAM1 IOVDD 1.80V
    {
        .seq_type     = SENSOR_IOVDD,
        .data         = (void*)"main-sensor-iovdd",
        .config_val   = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //MCAM1 VCM Enable
    {
        .seq_type     = SENSOR_VCM_PWDN,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },
    //MCAM1 AFVDD 3V
    {
        .seq_type     = SENSOR_VCM_AVDD,
        .data         = (void*)"cameravcm-vcc",
        .config_val   = LDO_VOLTAGE_V3PV,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },
    //disable sec camera reset
    {
        .seq_type     = SENSOR_SUSPEND,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },
};

struct mutex hi1333HANDEL_power_lock;
atomic_t volatile hi1333HANDEL_powered = ATOMIC_INIT(0);

static sensor_t s_hi1333HANDEL =
{
    .intf = { .vtbl = &s_hi1333HANDEL_vtbl, },
    .power_setting_array = 
    {
        .size          = ARRAY_SIZE(hw_hi1333HANDEL_power_up_setting),
        .power_setting = hw_hi1333HANDEL_power_up_setting,
    },
    .power_down_setting_array = 
    {
        .size          = ARRAY_SIZE(hw_hi1333HANDEL_power_down_setting),
        .power_setting = hw_hi1333HANDEL_power_down_setting,
    },
    .p_atpowercnt      = &hi1333HANDEL_powered,
};


static const struct of_device_id s_hi1333HANDEL_dt_match[] =
{
    {
        .compatible = "huawei,hi1333HANDEL",
        .data       = &s_hi1333HANDEL.intf,
    },
    {
    },
};

MODULE_DEVICE_TABLE(of, s_hi1333HANDEL_dt_match);

static struct platform_driver s_hi1333HANDEL_driver =
{
    .driver =
    {
        .name           = "huawei,hi1333HANDEL",
        .owner          = THIS_MODULE,
        .of_match_table = s_hi1333HANDEL_dt_match,
    },
};

char const* hi1333HANDEL_get_name(hwsensor_intf_t* si)
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

int hi1333HANDEL_power_up(hwsensor_intf_t* si)
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

    ret = hw_sensor_power_up_config(s_hi1333HANDEL.dev, sensor->board_info);
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

int hi1333HANDEL_power_down(hwsensor_intf_t* si)
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

int hi1333HANDEL_csi_enable(hwsensor_intf_t* si)
{
    (void)si;
    return 0;
}

int hi1333HANDEL_csi_disable(hwsensor_intf_t* si)
{
    (void)si;
    return 0;
}

static int hi1333HANDEL_match_id(hwsensor_intf_t* si, void * data)
{
    sensor_t* sensor = NULL;
    struct sensor_cfg_data *cdata = NULL;

    if (!si || !data){
        cam_err("%s : si or data is null", __func__);
        return -1;
    }

    sensor = I2S(si);
    if (NULL == sensor || NULL == sensor->board_info) {
        cam_err("%s. sensor or board_info is NULL.", __func__);
        return -1;
    }
    cdata = (struct sensor_cfg_data *)data;

    cam_info("%s enter.", __func__);

    cdata->data = sensor->board_info->sensor_index;

    return 0;
}

static hwsensor_vtbl_t s_hi1333HANDEL_vtbl =
{
    .get_name    = hi1333HANDEL_get_name,
    .config      = hi1333HANDEL_config,
    .power_up    = hi1333HANDEL_power_up,
    .power_down  = hi1333HANDEL_power_down,
    .match_id    = hi1333HANDEL_match_id,
    .csi_enable  = hi1333HANDEL_csi_enable,
    .csi_disable = hi1333HANDEL_csi_disable,
};
/*lint -e455 -esym(455,*)*/
int hi1333HANDEL_config(hwsensor_intf_t* si, void  *argp)
{
    int ret =0;
    struct sensor_cfg_data *data = NULL;

    if (NULL == si || NULL == argp){
        cam_err("%s si or argp is null.\n", __func__);
        return -1;
    }

    data = (struct sensor_cfg_data *)argp;
    cam_debug("hi1333HANDEL cfgtype = %d",data->cfgtype);

    switch(data->cfgtype){
        case SEN_CONFIG_POWER_ON:
            mutex_lock(&hi1333HANDEL_power_lock);
            if (false == power_on_status) {
                if(NULL == si->vtbl->power_up){
                    cam_err("%s. si->vtbl->power_up is null.", __func__);
                    mutex_unlock(&hi1333HANDEL_power_lock);
                    return -1;
                }else{
                    ret = si->vtbl->power_up(si);
                    if(0 == ret){
                        power_on_status = true;
                    } else {
                        cam_err("%s. power up fail.", __func__);
                    }
                }
            }
            /*lint -e455 -esym(455,*)*/
            mutex_unlock(&hi1333HANDEL_power_lock);
            /*lint -e455 +esym(455,*)*/
            break;
        case SEN_CONFIG_POWER_OFF:
            mutex_lock(&hi1333HANDEL_power_lock);
            if (true == power_on_status) {
                if(NULL == si->vtbl->power_down){
                    cam_err("%s. si->vtbl->power_down is null.", __func__);
                    mutex_unlock(&hi1333HANDEL_power_lock);
                    return -1;
                }else{
                    ret = si->vtbl->power_down(si);
                    if (0 != ret) {
                        cam_err("%s. power_down fail.", __func__);
                    }
                    power_on_status = false;
                }
            }
            /*lint -e455 -esym(455,*)*/
            mutex_unlock(&hi1333HANDEL_power_lock);
            /*lint -e455 +esym(455,*)*/
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
/*lint -e455 +esym(455,*)*/
static int32_t hi1333HANDEL_platform_probe(struct platform_device* pdev)
{
    int rc = 0;

    if (NULL == pdev){
        cam_err("%s pdev is null.\n", __func__);
        return -1;
    }
    
    cam_info("enter %s",__func__);
    if (pdev->dev.of_node) {
        rc = hw_sensor_get_dt_data(pdev, &s_hi1333HANDEL);
        if (rc < 0) {
            cam_err("%s failed line %d\n", __func__, __LINE__);
            goto hi1333HANDEL_sensor_probe_fail;
        }
    } else {
        cam_err("%s hi1333HANDEL of_node is NULL.\n", __func__);
        goto hi1333HANDEL_sensor_probe_fail;
    }
    mutex_init(&hi1333HANDEL_power_lock);
    s_hi1333HANDEL.dev = &pdev->dev;

    rc = hwsensor_register(pdev, &s_hi1333HANDEL.intf);
    if (0 != rc){
        cam_err("%s hwsensor_register fail.\n", __func__);
        goto hi1333HANDEL_sensor_probe_fail;
    }
    
    rc = rpmsg_sensor_register(pdev, (void *)&s_hi1333HANDEL);
    if (0 != rc){
        cam_err("%s rpmsg_sensor_register fail.\n", __func__);
        hwsensor_unregister(Sensor2Pdev(s_hi1333HANDEL));
        goto hi1333HANDEL_sensor_probe_fail;
    }

hi1333HANDEL_sensor_probe_fail:
    return rc;
}

static int __init hi1333HANDEL_init_module(void)
{
    cam_info("enter %s",__func__);
    return platform_driver_probe(&s_hi1333HANDEL_driver, hi1333HANDEL_platform_probe);
}

static void __exit hi1333HANDEL_exit_module(void)
{
    rpmsg_sensor_unregister((void *)&s_hi1333HANDEL);
    hwsensor_unregister(Sensor2Pdev(s_hi1333HANDEL));
    platform_driver_unregister(&s_hi1333HANDEL_driver);
}
//lint -restore

/*lint -e528 -esym(528,*)*/
module_init(hi1333HANDEL_init_module);
module_exit(hi1333HANDEL_exit_module);
/*lint -e528 +esym(528,*)*/
/*lint -e753 -esym(753,*)*/
MODULE_DESCRIPTION("hi1333HANDEL");
MODULE_LICENSE("GPL v2");
/*lint -e753 +esym(753,*)*/

