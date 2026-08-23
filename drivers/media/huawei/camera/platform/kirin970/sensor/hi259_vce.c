

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
#define SENSOR_INDEX_0       (0)
#define DELAY_0MS            (0)
#define DELAY_1MS            (1)
#define DELAY_5MS            (5)

//lint -save -e846 -e514 -e866 -e30 -e84 -e785 -e64
//lint -save -e826 -e838 -e715 -e747 -e778 -e774 -e732
//lint -save -e650 -e31 -e731 -e528 -e753 -e737

static bool s_hi259VCE_power_on = false;

struct mutex hi259_vce_power_lock;

char const* hi259VCE_get_name(hwsensor_intf_t* si);
int hi259VCE_config(hwsensor_intf_t* si, void  *argp);
int hi259VCE_power_up(hwsensor_intf_t* si);
int hi259VCE_power_down(hwsensor_intf_t* si);
int hi259VCE_match_id(hwsensor_intf_t* si, void * data);
int hi259VCE_csi_enable(hwsensor_intf_t* si);
int hi259VCE_csi_disable(hwsensor_intf_t* si);

static hwsensor_vtbl_t s_hi259VCE_vtbl =
{
    .get_name = hi259VCE_get_name,
    .config = hi259VCE_config,
    .power_up = hi259VCE_power_up,
    .power_down = hi259VCE_power_down,
    .match_id = hi259VCE_match_id,
    .csi_enable = hi259VCE_csi_enable,
    .csi_disable = hi259VCE_csi_disable,
};

static hwsensor_intf_t *s_intf = NULL;
static sensor_t *s_sensor = NULL;

struct sensor_power_setting hw_hi259VCE_power_up_setting[] = {

      //SEL=0 SWITCH MIPI to S1 [GPIO-220]
    {
        .seq_type = SENSOR_MIPI_SW,
        .config_val = SENSOR_GPIO_HIGH, //pull down, for power down
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_0MS,
    },
    //MCAM TELE PWDN STANDBY [GPIO-227]
    {
        .seq_type = SENSOR_PWDN,
        .config_val = SENSOR_GPIO_LOW, //out high
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_1MS,
    },
    //MCAM TELE IOVDD & DVDD 1.80V [PMU-LDO021]
    {
        .seq_type = SENSOR_IOVDD,
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_1MS,
    },
    //MCAM TELE AVDD 2.8V [PMU-LDO033]
    {
        .seq_type = SENSOR_AVDD,
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_0MS,
    },
	//MCAM TELE AFVDD & MIPISW_LDO 3.0V [PMU-LDO25]
    {
        .seq_type = SENSOR_VCM_AVDD,
        .config_val = LDO_VOLTAGE_V3PV,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_0MS,
    },
    //SEL=0 SWITCH MIPI to S1 [GPIO-220]
    {
        .seq_type = SENSOR_MIPI_SW,
        .config_val = SENSOR_GPIO_LOW, //pull high
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_0MS,
    },
    //MCAM TELE MCLK [ISP-CLK0]
    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = 0,
        .delay = DELAY_1MS,
    },
    //MCAM TELE PWDN NORMAL WORK [GPIO-227]
    {
        .seq_type = SENSOR_PWDN,
        .config_val = SENSOR_GPIO_HIGH, //pull down
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_5MS,
    },
    //MCAM TELE RESET [GPIO_136]
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_1MS,
    },
};

struct sensor_power_setting hi259VCE_power_down_setting[] = {

    //MCAM TELE PWDN STANDBY [GPIO-227]
    {
        .seq_type = SENSOR_PWDN,
        .config_val = SENSOR_GPIO_LOW, //out low
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_1MS,
    },
    //SEL=0 SWITCH MIPI to S1 [GPIO-220]
    {
        .seq_type = SENSOR_MIPI_SW,
        .config_val = SENSOR_GPIO_HIGH, //pull high
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_0MS,
    },

    //MCAM TELE IOVDD & DVDD 1.80V [PMU-LDO021]
    {
        .seq_type = SENSOR_IOVDD,
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_1MS,
    },
    //MCAM TELE AVDD 2.8V [PMU-LDO033]
    {
        .seq_type = SENSOR_AVDD,
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_0MS,
    },
	//MCAM TELE AFVDD & MIPISW_LDO 3.0V [PMU-LDO25]
    {
        .seq_type = SENSOR_VCM_AVDD,
        .config_val = LDO_VOLTAGE_V3PV,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_0MS,
    },
    //SEL=0 SWITCH MIPI to S1 [GPIO-220]
    {
        .seq_type = SENSOR_MIPI_SW,
        .config_val = SENSOR_GPIO_LOW, //pull low
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_0MS,
    },
    //MCAM TELE MCLK [ISP-CLK0]
    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = 0,
        .delay = DELAY_1MS,
    },
    //MCAM TELE RESET [GPIO_136]
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_1MS,
    },
    //MCAM TELE PWDN NORMAL WORK [GPIO-227]
    {
        .seq_type = SENSOR_PWDN,
        .config_val = SENSOR_GPIO_HIGH, //pull high
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_5MS,
    },
};

atomic_t volatile hi259VCE_powered = ATOMIC_INIT(0);
sensor_t s_hi259VCE =
{
    .intf = { .vtbl = &s_hi259VCE_vtbl, },
    .power_setting_array = {
            .size = ARRAY_SIZE(hw_hi259VCE_power_up_setting),
            .power_setting = hw_hi259VCE_power_up_setting,
     },
    .power_down_setting_array = {
            .size = ARRAY_SIZE(hi259VCE_power_down_setting),
            .power_setting = hi259VCE_power_down_setting,
     },

    .p_atpowercnt = &hi259VCE_powered,
};

const struct of_device_id s_hi259VCE_dt_match[] =
{
    {
        .compatible = "huawei,hi259_vce",
        .data = &s_hi259VCE.intf,
    },

    {
    },
};

MODULE_DEVICE_TABLE(of, s_hi259VCE_dt_match);

struct platform_driver
s_hi259VCE_driver =
{
    .driver =
    {
        .name = "huawei,hi259",
        .owner = THIS_MODULE,
        .of_match_table = s_hi259VCE_dt_match,
    },
};

char const* hi259VCE_get_name(hwsensor_intf_t* si)
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

int hi259VCE_power_up(hwsensor_intf_t* si)
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

int hi259VCE_power_down(hwsensor_intf_t* si)
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

int hi259VCE_csi_enable(hwsensor_intf_t* si)
{
    return 0;
}

int hi259VCE_csi_disable(hwsensor_intf_t* si)
{
    return 0;
}

int hi259VCE_match_id(hwsensor_intf_t* si, void * data)
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

int hi259VCE_config(hwsensor_intf_t* si, void  *argp)
{
    struct sensor_cfg_data *data = NULL;
    int ret = 0;

    if (NULL == si || NULL == argp || NULL == si->vtbl) {
        cam_err("%s : si or argp or si->vtbl is null", __func__);
        return -EINVAL;
    }

    data = (struct sensor_cfg_data *)argp;
    cam_debug("hi259VCE cfgtype = %d",data->cfgtype);
    switch (data->cfgtype) {
        case SEN_CONFIG_POWER_ON:
            mutex_lock(&hi259_vce_power_lock);
            if (NULL == si->vtbl->power_up)
            {
                cam_err("%s. si power_up is null", __func__);
                /*lint -e455 -esym(455,*)*/
                mutex_unlock(&hi259_vce_power_lock);
                /*lint -e455 +esym(455,*)*/
                return -EINVAL;
            }
            if (!s_hi259VCE_power_on){
                ret = si->vtbl->power_up(si);
                if (0 == ret) {
                    s_hi259VCE_power_on = true;
                } else {
                    cam_err("%s. power up fail.", __func__);
                }
            }
            /*lint -e455 -esym(455,*)*/
            mutex_unlock(&hi259_vce_power_lock);
            /*lint -e455 +esym(455,*)*/
            break;
        case SEN_CONFIG_POWER_OFF:
            mutex_lock(&hi259_vce_power_lock);
            if (NULL == si->vtbl->power_down)
            {
                cam_err("%s. si power_up is null", __func__);
                /*lint -e455 -esym(455,*)*/
                mutex_unlock(&hi259_vce_power_lock);
                /*lint -e455 +esym(455,*)*/
                return -EINVAL;
            }
            if (s_hi259VCE_power_on)
            {
                ret = si->vtbl->power_down(si);
                if (0 == ret)
                {
                    s_hi259VCE_power_on = false;
                }else{
                    cam_err("%s. power down fail.", __func__);
                }
            }
            /*lint -e455 -esym(455,*)*/
            mutex_unlock(&hi259_vce_power_lock);
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
    return ret;
}

int32_t hi259VCE_platform_probe(struct platform_device* pdev)
{
    int rc = 0;
    const struct of_device_id *id = NULL;
    hwsensor_intf_t *intf = NULL;
    sensor_t *sensor = NULL;
    struct device_node *np = NULL;
    cam_notice("enter %s",__func__);

    if (NULL == pdev) {
        cam_err("%s pdev is NULL", __func__);
        return -EINVAL;
    }

    mutex_init(&hi259_vce_power_lock);
    np = pdev->dev.of_node;
    if (NULL == np) {
        cam_err("%s of_node is NULL", __func__);
        return -ENODEV;
    }

    id = of_match_node(s_hi259VCE_dt_match, np);
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

    return rc;
}

int __init
hi259VCE_init_module(void)
{
    cam_info("enter %s",__func__);
    return platform_driver_probe(&s_hi259VCE_driver,
            hi259VCE_platform_probe);
}

void __exit
hi259VCE_exit_module(void)
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
    platform_driver_unregister(&s_hi259VCE_driver);
}
//lint -restore

/*lint -e528 -esym(528,*)*/
module_init(hi259VCE_init_module);
module_exit(hi259VCE_exit_module);
/*lint -e528 +esym(528,*)*/
/*lint -e753 -esym(753,*)*/
MODULE_DESCRIPTION("hi259VCE");
MODULE_LICENSE("GPL v2");
/*lint -e753 +esym(753,*)*/
