


#include <linux/module.h>
#include <linux/printk.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/rpmsg.h>
#include <securec.h>

#include "hwsensor.h"
#include "sensor_commom.h"
#include "hw_csi.h"

#define I2S(i) container_of(i, sensor_t, intf)
#define Sensor2Pdev(s) container_of((s).dev, struct platform_device, dev)

//lint -save -e846 -e514 -e866 -e30 -e84 -e785 -e64 -e826 -e838 -e715 -e747 -e778 -e774 -e732 -e650 -e31 -e731 -e528 -e753 -e737

char *sensor_dts_name = "IMX486_VENDOR";

extern struct hw_csi_pad hw_csi_pad;
hwsensor_vtbl_t s_imx486_vtbl;

bool power_on_status = false;//false: power off, true:power on
int imx486_config(hwsensor_intf_t* si, void  *argp);

struct sensor_power_setting hw_imx486_power_up_setting[] = {

    //disable sec camera reset
    {
        .seq_type = SENSOR_SUSPEND,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //MCAM1 AFVDD 3V
    {
        .seq_type = SENSOR_VCM_AVDD,
        .data = (void*)"cameravcm-vcc",
        .config_val = LDO_VOLTAGE_V3PV,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
	//MCAM1 VCM PD Enable
    {
        .seq_type = SENSOR_VCM_PWDN,
        .config_val = SENSOR_GPIO_HIGH,
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
    
    //MCAM1 DVDD 1.2V
    {
        .seq_type = SENSOR_DVDD2,
        .config_val = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },


    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },

    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
};
struct sensor_power_setting hw_imx486_power_down_setting[] = {
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    {
        .seq_type = SENSOR_MCLK,
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
        .delay = 1,
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
    //MCAM1 AFVDD 3V
    {
        .seq_type = SENSOR_VCM_AVDD,
        .data = (void*)"cameravcm-vcc",
        .config_val = LDO_VOLTAGE_V3PV,
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
    //disable sec camera reset
    {
        .seq_type = SENSOR_SUSPEND,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
};

atomic_t volatile imx486_powered = ATOMIC_INIT(0);
struct mutex imx486_power_lock;
sensor_t s_imx486 =
{
    .intf = { .vtbl = &s_imx486_vtbl, },
    .power_setting_array = {
            .size = ARRAY_SIZE(hw_imx486_power_up_setting),
            .power_setting = hw_imx486_power_up_setting,
     },
    .power_down_setting_array = {
            .size = ARRAY_SIZE(hw_imx486_power_down_setting),
            .power_setting = hw_imx486_power_down_setting,
    },
     .p_atpowercnt = &imx486_powered,
};

const struct of_device_id
s_imx486_dt_match[] =
{
    {
        .compatible = "huawei,imx486",
        .data = &s_imx486.intf,
    },
    {
    },
};

MODULE_DEVICE_TABLE(of, s_imx486_dt_match);

struct platform_driver
s_imx486_driver =
{
    .driver =
    {
        .name = "huawei,imx486",
        .owner = THIS_MODULE,
        .of_match_table = s_imx486_dt_match,
    },
};

char const* imx486_get_name(hwsensor_intf_t* si)
{
    sensor_t* sensor = I2S(si);
    return sensor->board_info->name;
}

int imx486_power_up(hwsensor_intf_t* si)
{
    int ret = 0;
    sensor_t* sensor = NULL;
    sensor = I2S(si);
    cam_info("enter %s. index = %d name = %s", __func__, sensor->board_info->sensor_index, sensor->board_info->name);
    ret = hw_sensor_power_up_config(s_imx486.dev, sensor->board_info);
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

int imx486_power_down(hwsensor_intf_t* si)
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

int imx486_csi_enable(hwsensor_intf_t* si)
{
    return 0;
}

int imx486_csi_disable(hwsensor_intf_t* si)
{
    return 0;
}

int imx486_match_id(hwsensor_intf_t* si, void * data)
{
	sensor_t* sensor = I2S(si);
	struct sensor_cfg_data *cdata = (struct sensor_cfg_data *)data;
	int32_t module_id = 0;
	char *sensor_name [] = {"IMX486_OFILM", "IMX486_SUNNY"};
	int rc = 0;

      rc = memset_s(cdata->cfg.name, DEVICE_NAME_SIZE, 0, DEVICE_NAME_SIZE);
      if (rc != EOK) {
          cam_err("%s. failed to memset_s write_data.", __func__);
      }
	if (!strncmp(sensor->board_info->name, sensor_dts_name, strlen(sensor_dts_name))) {
		rc = gpio_request(sensor->board_info->gpios[FSIN].gpio, NULL);
		if(rc < 0) {
			cam_err("%s failed to request gpio[%d]", __func__, sensor->board_info->gpios[FSIN].gpio);
			return rc;
		}

        cam_info("%s gpio[%d].", __func__, sensor->board_info->gpios[FSIN].gpio);

		rc = gpio_direction_input(sensor->board_info->gpios[FSIN].gpio);
		if (rc < 0) {
		    cam_err("%s failed to config gpio(%d) input.\n",__func__, sensor->board_info->gpios[FSIN].gpio);
		}

		module_id = gpio_get_value_cansleep(sensor->board_info->gpios[FSIN].gpio);
		if (module_id < 0) {
		    cam_err("%s failed to get gpio(%d) value(%d).\n",__func__, sensor->board_info->gpios[FSIN].gpio, module_id);
		}

		cam_info("%s module_id = %d", __func__,module_id);

		gpio_free(sensor->board_info->gpios[FSIN].gpio);

		if (0 == module_id) {//ofilm
		    strncpy_s(cdata->cfg.name, sizeof(cdata->cfg.name), sensor_name[0], strlen(sensor_name[0])+1);
		    cdata->data = sensor->board_info->sensor_index;
		} else if(1 == module_id){//sunny
		    strncpy_s(cdata->cfg.name, sizeof(cdata->cfg.name), sensor_name[1], strlen(sensor_name[1])+1);
		    cdata->data = sensor->board_info->sensor_index;
		}else{
            cam_err("%s module =%d unknown", __func__,module_id);
		}
	} else {
	    strncpy_s(cdata->cfg.name, sizeof(cdata->cfg.name), sensor->board_info->name,
	        strlen(sensor->board_info->name)+1);
	    cdata->data = sensor->board_info->sensor_index;
	}

    if (cdata->data != SENSOR_INDEX_INVALID) {
        cam_info("%s, cdata->cfg.name = %s", __func__,cdata->cfg.name );
    }

    return rc;
}

hwsensor_vtbl_t
s_imx486_vtbl =
{
	.get_name = imx486_get_name,
	.config = imx486_config,
	.power_up = imx486_power_up,
	.power_down = imx486_power_down,
	.match_id = imx486_match_id,
	.csi_enable = imx486_csi_enable,
	.csi_disable = imx486_csi_disable,
	/* .sensor_register_attribute = imx486_register_attribute, */
};

int
imx486_config(
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
	cam_debug("imx486 cfgtype = %d",data->cfgtype);
	switch(data->cfgtype){
		case SEN_CONFIG_POWER_ON:
			mutex_lock(&imx486_power_lock);
			if(false == power_on_status){
				ret = si->vtbl->power_up(si);
				if (0 == ret)
				{
					power_on_status = true;
				}
			}
			/*lint -e455 -esym(455,*)*/
			mutex_unlock(&imx486_power_lock);
			/*lint -e455 +esym(455,*)*/
			break;
		case SEN_CONFIG_POWER_OFF:
			mutex_lock(&imx486_power_lock);
			if(true == power_on_status){
				ret = si->vtbl->power_down(si);
				if (0 != ret) {
					cam_err("%s. power_down fail.", __func__);
				}
				power_on_status = false;
			}
			/*lint -e455 -esym(455,*)*/
			mutex_unlock(&imx486_power_lock);
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

int32_t
imx486_platform_probe(
        struct platform_device* pdev)
{
	int rc = 0;
	cam_notice("enter %s",__func__);
	if (pdev->dev.of_node) {
		rc = hw_sensor_get_dt_data(pdev, &s_imx486);
		if (rc < 0) {
			cam_err("%s failed line %d\n", __func__, __LINE__);
			goto imx486_sensor_probe_fail;
		}
	} else {
		cam_err("%s imx486 of_node is NULL.\n", __func__);
		goto imx486_sensor_probe_fail;
	}

    s_imx486.dev = &pdev->dev;
	mutex_init(&imx486_power_lock);
	rc = hwsensor_register(pdev, &s_imx486.intf);
	rc = rpmsg_sensor_register(pdev, (void*)&s_imx486);

imx486_sensor_probe_fail:
	return rc;
}

int __init
imx486_init_module(void)
{
    cam_notice("enter %s",__func__);
    return platform_driver_probe(&s_imx486_driver,
            imx486_platform_probe);
}

void __exit
imx486_exit_module(void)
{
    rpmsg_sensor_unregister((void*)&s_imx486);
    hwsensor_unregister(Sensor2Pdev(s_imx486));
    platform_driver_unregister(&s_imx486_driver);
}
//lint -restore

/*lint -e528 -esym(528,*)*/
module_init(imx486_init_module);
module_exit(imx486_exit_module);
/*lint -e528 +esym(528,*)*/
/*lint -e753 -esym(753,*)*/
MODULE_DESCRIPTION("imx486");
MODULE_LICENSE("GPL v2");
/*lint -e753 +esym(753,*)*/
