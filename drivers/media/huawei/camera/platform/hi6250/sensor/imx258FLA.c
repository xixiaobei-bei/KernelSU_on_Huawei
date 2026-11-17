 


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

static hwsensor_vtbl_t s_imx258FLA_vtbl;
static bool power_on_status = false;//false: power off, true:power on
//lint -save -e846 -e514 -e866 -e30 -e84 -e785 -e64 -e826 -e838 -e715 -e747 -e778 -e774 -e732 -e650 -e31 -e731 -e528 -e753 -e737


int imx258FLA_config(hwsensor_intf_t* si, void  *argp);

struct sensor_power_setting hw_imx258FLA_power_up_setting[] = {

    //disable SCAM0 reset [GPIO17]
    {
        .seq_type = SENSOR_SUSPEND,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //disable SCAM1 reset [GPIO060]
    {
        .seq_type     = SENSOR_SUSPEND2,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },

    //disable SCAM1 PWDN [GPIO068]
    {
        .seq_type     = SENSOR_PWDN2,
        .config_val   = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },
    //disable MCAM1 reset [GPIO076]
    {
        .seq_type     = SENSOR_RST2,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },

    //disable MCAM1 PWDN [GPIO075]
    {
        .seq_type     = SENSOR_PWDN,
        .config_val   = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },

    //MCAM0 AFVDD 3.0V [LDO25]
    {
        .seq_type = SENSOR_VCM_AVDD,
        .config_val   = LDO_VOLTAGE_V3PV,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = SENSOR_REG_SETTING_DELAY_1_MS,
    },
    //MIPI Swith to MCAM [GPIO78]
    {
        .seq_type = SENSOR_MIPI_SW,
        .config_val = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = SENSOR_REG_SETTING_DELAY_1_MS,
    },
    //MCAM0 VCM PD Enable [GPIO19]
    {
        .seq_type = SENSOR_VCM_PWDN,
        .config_val = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = SENSOR_REG_SETTING_DELAY_1_MS,
    },
    //MCAM0 IOVDD 1.80V [LDO33]
    {
        .seq_type = SENSOR_IOVDD,
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //SCAM0 AVDD 2.8V [LDO13]
    {
        .seq_type = SENSOR_AVDD2,
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //SCAM0 DVDD 1.2V [LDO32]
    {
        .seq_type = SENSOR_DVDD2,
        .config_val = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = SENSOR_REG_SETTING_DELAY_1_MS,
    },
    //MCAM0 AVDD 2.80V [LDO19]
    {
        .seq_type = SENSOR_AVDD,
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //MCAM0 DVDD 1.2V [LDO20]
    {
        .seq_type = SENSOR_DVDD,
        .config_val = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //MCAM0 CLK
    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //MCAM0 RESET [GPIO18]
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = SENSOR_REG_SETTING_DELAY_1_MS,
    },
};

struct sensor_power_setting hw_imx258FLA_power_down_setting[] = {
    //MCAM0 RESET
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //MCAM0 CLK
    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //MCAM0 DVDD 1.2V
    {
        .seq_type = SENSOR_DVDD,
        .config_val = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //MCAM0 AVDD 2.80V
    {
        .seq_type = SENSOR_AVDD,
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //SCAM0 AVDD 2.8V
    {
        .seq_type = SENSOR_AVDD2,
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //MCAM0 IOVDD 1.80V
    {
        .seq_type = SENSOR_IOVDD,
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = SENSOR_REG_SETTING_DELAY_1_MS,
    },
    //SCAM0 DVDD 1.2V
    {
        .seq_type = SENSOR_DVDD2,
        .config_val = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = SENSOR_REG_SETTING_DELAY_1_MS,
    },
    //MCAM0 VCM PD Enable
    {
        .seq_type = SENSOR_VCM_PWDN,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = SENSOR_REG_SETTING_DELAY_1_MS,
    },
    //MIPI Swith
    {
        .seq_type     = SENSOR_MIPI_SW,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },
    //MCAM0 AFVDD 3.0V
    {
        .seq_type = SENSOR_VCM_AVDD,
        .config_val   = LDO_VOLTAGE_V3PV,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = SENSOR_REG_SETTING_DELAY_1_MS,
    },
    //disable MCAM1 PWDN [GPIO075]
    {
        .seq_type     = SENSOR_PWDN,
        .config_val   = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },
    //disable MCAM1 reset [GPIO076]
    {
        .seq_type     = SENSOR_RST2,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },
    //disable SCAM1 PWDN
    {
        .seq_type     = SENSOR_PWDN2,
        .config_val   = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },
    //disable SCAM1 reset
    {
        .seq_type     = SENSOR_SUSPEND2,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },
    //disable SCAM0 reset
    {
        .seq_type = SENSOR_SUSPEND,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
};


atomic_t volatile imx258FLA_powered = ATOMIC_INIT(0);
struct mutex imx258FLA_power_lock;
static sensor_t s_imx258FLA =
{
    .intf = { .vtbl = &s_imx258FLA_vtbl, },
    .power_setting_array = 
    {
        .size          = ARRAY_SIZE(hw_imx258FLA_power_up_setting),
        .power_setting = hw_imx258FLA_power_up_setting,
    },
    .power_down_setting_array = 
    {
        .size          = ARRAY_SIZE(hw_imx258FLA_power_down_setting),
        .power_setting = hw_imx258FLA_power_down_setting,
    },
    .p_atpowercnt      = &imx258FLA_powered,
};


static const struct of_device_id s_imx258FLA_dt_match[] =
{
    {
        .compatible = "huawei,imx258FLA",
        .data       = &s_imx258FLA.intf,
    },
    {
    },
};

MODULE_DEVICE_TABLE(of, s_imx258FLA_dt_match);

static struct platform_driver s_imx258FLA_driver =
{
    .driver =
    {
        .name           = "huawei,imx258FLA",
        .owner          = THIS_MODULE,
        .of_match_table = s_imx258FLA_dt_match,
    },
};

char const* imx258FLA_get_name(hwsensor_intf_t* si)
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

int imx258FLA_power_up(hwsensor_intf_t* si)
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

    ret = hw_sensor_power_up_config(s_imx258FLA.dev, sensor->board_info);
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

int imx258FLA_power_down(hwsensor_intf_t* si)
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

int imx258FLA_csi_enable(hwsensor_intf_t* si)
{
    (void)si;
    return 0;
}

int imx258FLA_csi_disable(hwsensor_intf_t* si)
{
    (void)si;
    return 0;
}

static int imx258FLA_match_id(hwsensor_intf_t* si, void * data)
{
    sensor_t* sensor = NULL;
    struct sensor_cfg_data *cdata = NULL;
    if(NULL == si || NULL == data)
    {
        cam_err("%s : si or data is null", __func__);
        return -1;
    }

    cam_info("%s enter.", __func__);
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

static hwsensor_vtbl_t s_imx258FLA_vtbl =
{
    .get_name    = imx258FLA_get_name,
    .config      = imx258FLA_config,
    .power_up    = imx258FLA_power_up,
    .power_down  = imx258FLA_power_down,
    .match_id    = imx258FLA_match_id,
    .csi_enable  = imx258FLA_csi_enable,
    .csi_disable = imx258FLA_csi_disable,
};
/*lint -e455 -esym(455,*)*/
int imx258FLA_config(hwsensor_intf_t* si, void  *argp)
{
    int ret =0;
    struct sensor_cfg_data *data = NULL;

    if (NULL == si || NULL == argp){
        cam_err("%s si or argp is null.\n", __func__);
        return -1;
    }

    data = (struct sensor_cfg_data *)argp;
    cam_debug("imx258FLA cfgtype = %d",data->cfgtype);

    switch(data->cfgtype){
        case SEN_CONFIG_POWER_ON:
            mutex_lock(&imx258FLA_power_lock);
            if (false == power_on_status) {
                if(NULL == si->vtbl->power_up){
                    cam_err("%s. si->vtbl->power_up is null.", __func__);
                    mutex_unlock(&imx258FLA_power_lock);
                    return -1;
                }else{
                    ret = si->vtbl->power_up(si);
                    if(0 == ret){
                        power_on_status = true;
                    }
                }
            }
            /*lint -e455 -esym(455,*)*/
            mutex_unlock(&imx258FLA_power_lock);
            /*lint -e455 +esym(455,*)*/
            break;
        case SEN_CONFIG_POWER_OFF:
            mutex_lock(&imx258FLA_power_lock);
            if (true == power_on_status) {
                if(NULL == si->vtbl->power_down){
                    cam_err("%s. si->vtbl->power_down is null.", __func__);
                    mutex_unlock(&imx258FLA_power_lock);
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
            mutex_unlock(&imx258FLA_power_lock);
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
/*lint -e455 +esym(455,*)*/
static int32_t imx258FLA_platform_probe(struct platform_device* pdev)
{
    int rc = 0;

    if (NULL == pdev){
        cam_err("%s pdev is null.\n", __func__);
        return -1;
    }
    
    cam_info("enter %s",__func__);
    if (pdev->dev.of_node) {
        rc = hw_sensor_get_dt_data(pdev, &s_imx258FLA);
        if (rc < 0) {
            cam_err("%s failed line %d\n", __func__, __LINE__);
            goto imx258FLA_sensor_probe_fail;
        }
    } else {
        cam_err("%s imx258FLA of_node is NULL.\n", __func__);
        goto imx258FLA_sensor_probe_fail;
    }
    mutex_init(&imx258FLA_power_lock);
    s_imx258FLA.dev = &pdev->dev;

    rc = hwsensor_register(pdev, &s_imx258FLA.intf);
    if (0 != rc){
        cam_err("%s hwsensor_register fail.\n", __func__);
        goto imx258FLA_sensor_probe_fail;
    }
    
    rc = rpmsg_sensor_register(pdev, (void *)&s_imx258FLA);
    if (0 != rc){
        cam_err("%s rpmsg_sensor_register fail.\n", __func__);
        hwsensor_unregister(Sensor2Pdev(s_imx258FLA));
        goto imx258FLA_sensor_probe_fail;
    }

imx258FLA_sensor_probe_fail:
    return rc;
}

static int __init imx258FLA_init_module(void)
{
    cam_info("enter %s",__func__);
    return platform_driver_probe(&s_imx258FLA_driver, imx258FLA_platform_probe);
}

static void __exit imx258FLA_exit_module(void)
{
    rpmsg_sensor_unregister((void *)&s_imx258FLA);
    hwsensor_unregister(Sensor2Pdev(s_imx258FLA));
    platform_driver_unregister(&s_imx258FLA_driver);
}
//lint -restore

/*lint -e528 -esym(528,*)*/
module_init(imx258FLA_init_module);
module_exit(imx258FLA_exit_module);
/*lint -e528 +esym(528,*)*/
/*lint -e753 -esym(753,*)*/
MODULE_DESCRIPTION("imx258FLA");
MODULE_LICENSE("GPL v2");
/*lint -e753 +esym(753,*)*/

