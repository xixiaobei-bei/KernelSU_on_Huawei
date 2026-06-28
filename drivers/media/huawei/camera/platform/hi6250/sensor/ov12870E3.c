


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

//lint -save -e846 -e514 -e866 -e30 -e84 -e785 -e64 -e826 -e838 -e715 -e747 -e778 -e774 -e732 -e650 -e31 -e731 -e528 -e753

static bool s_ov12870E3_power_on = false;
extern struct hw_csi_pad hw_csi_pad;
static hwsensor_vtbl_t s_ov12870E3_vtbl;

int ov12870E3_config(hwsensor_intf_t* si, void  *argp);

struct sensor_power_setting hw_ov12870E3_power_up_setting[] = {

    //disable sec camera reset
    {
        .seq_type = SENSOR_SUSPEND,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //SCAM AVDD 2.80V
    {
        .seq_type = SENSOR_AVDD,
        .data = (void*)"slave-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //SCAM DVDD1.2V
    {
        .seq_type = SENSOR_DVDD,
        .config_val = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },

    //MCAM1 IOVDD 1.80V
    {
        .seq_type = SENSOR_IOVDD,
        .data = (void*)"main-sensor-iovdd",
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //MCAM1 AVDD 2.80V
    {
        .seq_type = SENSOR_AVDD2,
        .data = (void*)"main-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //MCAM1 DVDD 1.2V
    {
        .seq_type = SENSOR_DVDD2,
        .config_val = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },

    //MCAM1 AFVDD 2.85V
    {
        .seq_type = SENSOR_VCM_AVDD,
        .data = (void*)"cameravcm-vcc",
        .config_val = LDO_VOLTAGE_V2P85V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },

    //MCAM1 VCM Enable
    {
        .seq_type = SENSOR_VCM_PWDN,
        .config_val = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 6,
    },
    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
};
struct sensor_power_setting hw_ov12870E3_power_down_setting[] = {
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
    //MCAM1 VCM Enable
    {
        .seq_type = SENSOR_VCM_PWDN,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //MCAM1 AFVDD 2.85V
    {
        .seq_type = SENSOR_VCM_AVDD,
        .data = (void*)"cameravcm-vcc",
        .config_val = LDO_VOLTAGE_V2P85V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //MCAM1 DVDD 1.2V
    {
        .seq_type = SENSOR_DVDD2,
        .config_val = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //MCAM1 AVDD 2.80V
    {
        .seq_type = SENSOR_AVDD2,
        .data = (void*)"main-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //MCAM1 IOVDD 1.80V
    {
        .seq_type = SENSOR_IOVDD,
        .data = (void*)"main-sensor-iovdd",
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //SCAM DVDD1.2V
    {
        .seq_type = SENSOR_DVDD,
        .config_val = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //SCAM AVDD 2.80V
    {
        .seq_type = SENSOR_AVDD,
        .data = (void*)"slave-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //disable sec camera reset
    {
        .seq_type = SENSOR_SUSPEND,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
};

atomic_t volatile ov12870E3_powered = ATOMIC_INIT(0);
struct mutex ov12870E3_power_lock;
static sensor_t s_ov12870E3 =
{
    .intf = { .vtbl = &s_ov12870E3_vtbl, },
    .power_setting_array = {
            .size = ARRAY_SIZE(hw_ov12870E3_power_up_setting),
            .power_setting = hw_ov12870E3_power_up_setting,
     },
    .power_down_setting_array = {
            .size = ARRAY_SIZE(hw_ov12870E3_power_down_setting),
            .power_setting = hw_ov12870E3_power_down_setting,
    },
     .p_atpowercnt = &ov12870E3_powered,
};

static const struct of_device_id
s_ov12870E3_dt_match[] =
{
	{
        .compatible = "huawei,ov12870E3",
        .data = &s_ov12870E3.intf,
    },
	{
    },
};

MODULE_DEVICE_TABLE(of, s_ov12870E3_dt_match);

static struct platform_driver
s_ov12870E3_driver =
{
    .driver =
    {
        .name = "huawei,ov12870E3",
        .owner = THIS_MODULE,
        .of_match_table = s_ov12870E3_dt_match,
    },
};

char const* ov12870E3_get_name(hwsensor_intf_t* si)
{
    sensor_t* sensor = I2S(si);
    return sensor->board_info->name;
}

int ov12870E3_power_up(hwsensor_intf_t* si)
{
    int ret = 0;
    sensor_t* sensor = NULL;
    sensor = I2S(si);
    cam_info("enter %s. index = %d name = %s", __func__, sensor->board_info->sensor_index, sensor->board_info->name);
    ret = hw_sensor_power_up_config(s_ov12870E3.dev, sensor->board_info);
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

int ov12870E3_power_down(hwsensor_intf_t* si)
{
    int ret = 0;
    sensor_t* sensor = NULL;
    sensor = I2S(si);
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
    hw_sensor_power_down_config(sensor->board_info);
    return ret;
}

int ov12870E3_csi_enable(hwsensor_intf_t* si)
{
    return 0;
}

int ov12870E3_csi_disable(hwsensor_intf_t* si)
{
    return 0;
}

static int
ov12870E3_match_id(
        hwsensor_intf_t* si, void * data)
{
    
    sensor_t* sensor = I2S(si);
    struct sensor_cfg_data *cdata = (struct sensor_cfg_data *)data;

    cam_info("%s enter.", __func__);

    cdata->data = sensor->board_info->sensor_index;

    return 0;

}

static hwsensor_vtbl_t
s_ov12870E3_vtbl =
{
	.get_name = ov12870E3_get_name,
	.config = ov12870E3_config,
	.power_up = ov12870E3_power_up,
	.power_down = ov12870E3_power_down,
	.match_id = ov12870E3_match_id,
	.csi_enable = ov12870E3_csi_enable,
	.csi_disable = ov12870E3_csi_disable,
	/*.sensor_register_attribute = ov12870E3_register_attribute,*/
};

int
ov12870E3_config(
        hwsensor_intf_t* si,
        void  *argp)
{
	struct sensor_cfg_data *data;
	int ret =0;

	if (NULL == si || NULL == argp){
		cam_err("%s : si or argp is null", __func__);
		return -1;
	}

	data = (struct sensor_cfg_data *)argp;
	cam_debug("ov12870E3 cfgtype = %d",data->cfgtype);
	switch(data->cfgtype){
		case SEN_CONFIG_POWER_ON:
		mutex_lock(&ov12870E3_power_lock);
		if (!s_ov12870E3_power_on)
		{
			ret = si->vtbl->power_up(si);
            if (ret == 0) {
               s_ov12870E3_power_on = true;
            }
		}
        /*lint -e455 -esym(455,*)*/
        mutex_unlock(&ov12870E3_power_lock);
        /*lint -e455 +esym(455,*)*/
			break;
		case SEN_CONFIG_POWER_OFF:
		mutex_lock(&ov12870E3_power_lock);
		if (s_ov12870E3_power_on)
		{
			ret = si->vtbl->power_down(si);
            if (ret != 0) {
                cam_err("%s. power_down fail.", __func__);
            }
            s_ov12870E3_power_on = false;
		}
		/*lint -e455 -esym(455,*)*/
        mutex_unlock(&ov12870E3_power_lock);
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
			ret = si->vtbl->match_id(si,argp);
			break;
		default:
			cam_err("%s cfgtype(%d) is error", __func__, data->cfgtype);
			break;
	}
	return ret;
}

static int32_t
ov12870E3_platform_probe(
        struct platform_device* pdev)
{
	int rc = 0;
	cam_notice("enter %s",__func__);
	if (pdev->dev.of_node) {
		rc = hw_sensor_get_dt_data(pdev, &s_ov12870E3);
		if (rc < 0) {
			cam_err("%s failed line %d\n", __func__, __LINE__);
			goto ov12870E3_sensor_probe_fail;
		}
	} else {
		cam_err("%s ov12870E3 of_node is NULL.\n", __func__);
		goto ov12870E3_sensor_probe_fail;
	}

    mutex_init(&ov12870E3_power_lock);
    s_ov12870E3.dev = &pdev->dev;

	rc = hwsensor_register(pdev, &s_ov12870E3.intf);
	rc = rpmsg_sensor_register(pdev, (void*)&s_ov12870E3);

ov12870E3_sensor_probe_fail:
	return rc;
}

static int __init
ov12870E3_init_module(void)
{
    cam_notice("enter %s",__func__);
    return platform_driver_probe(&s_ov12870E3_driver,
            ov12870E3_platform_probe);
}

static void __exit
ov12870E3_exit_module(void)
{
    rpmsg_sensor_unregister((void*)&s_ov12870E3);
    hwsensor_unregister(Sensor2Pdev(s_ov12870E3));
    platform_driver_unregister(&s_ov12870E3_driver);
}
//lint -restore

/*lint -e528 -esym(528,*)*/
module_init(ov12870E3_init_module);
module_exit(ov12870E3_exit_module);
/*lint -e528 +esym(528,*)*/
/*lint -e753 -esym(753,*)*/
MODULE_DESCRIPTION("ov12870E3");
MODULE_LICENSE("GPL v2");
/*lint -e753 +esym(753,*)*/
