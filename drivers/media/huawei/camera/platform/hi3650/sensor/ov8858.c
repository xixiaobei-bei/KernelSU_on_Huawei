


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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"

extern struct hw_csi_pad hw_csi_pad;
static hwsensor_vtbl_t s_ov8858_vtbl;



static int ov8858_config(hwsensor_intf_t* si, void  *argp);

static struct sensor_power_setting ov8858_power_setting[] = {

	//disable slave camera reset
	{
		.seq_type = SENSOR_SUSPEND,
		.config_val = SENSOR_GPIO_LOW,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},



	//MCAM1 AFVDD 2.85V
	{
		.seq_type = SENSOR_VCM_AVDD,
		.data = (void*)"cameravcm-vcc",
		.config_val = LDO_VOLTAGE_V2P85V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 1,
	},

	{
		.seq_type = SENSOR_VCM_PWDN,
		.config_val = SENSOR_GPIO_LOW,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 1,
	},

	//SCAM AVDD 2.8V
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

	//MCAM1 AVDD 2.8V
	{
		.seq_type = SENSOR_AVDD2,
		.data = (void*)"main-sensor-avdd",
		.config_val = LDO_VOLTAGE_V2P8V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 1,
	},

	//enable gpio12 output iovdd 1.8v
	{
		.seq_type = SENSOR_LDO_EN,
		.config_val = SENSOR_GPIO_LOW,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 1,
	},

	//MCAM1 DVDD 1.05V
	{
		.seq_type = SENSOR_DVDD2,
		//FIXME: need check  1.05V
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
		.config_val = SENSOR_GPIO_LOW,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 1,
	},
};

static sensor_t s_ov8858 =
{
    .intf = { .vtbl = &s_ov8858_vtbl, },
    .power_setting_array = {
            .size = ARRAY_SIZE(ov8858_power_setting),
            .power_setting = ov8858_power_setting,
     },
};

static const struct of_device_id s_ov8858_dt_match[] =
{
	{
        .compatible = "huawei,ov8858",
        .data = &s_ov8858.intf,
    },
	{
    },
};

MODULE_DEVICE_TABLE(of, s_ov8858_dt_match);

static struct platform_driver s_ov8858_driver =
{
	.driver =
    {
		.name = "huawei,ov8858",
		.owner = THIS_MODULE,
		.of_match_table = s_ov8858_dt_match,
	},
};

static char const* ov8858_get_name(hwsensor_intf_t* si)
{
    sensor_t* sensor = I2S(si);
    return sensor->board_info->name;
}

static int ov8858_power_up(hwsensor_intf_t* si)
{
    int ret = 0;
    sensor_t* sensor = NULL;
    sensor = I2S(si);
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

static int ov8858_power_down(
        hwsensor_intf_t* si)
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
	return ret;
}

static int ov8858_csi_enable(hwsensor_intf_t* si)
{
    return 0;
}

static int ov8858_csi_disable(hwsensor_intf_t* si)
{
    return 0;
}

static int ov8858_match_id(
        hwsensor_intf_t* si, void * data)
{




	sensor_t* sensor = I2S(si);
    struct sensor_cfg_data *cdata = (struct sensor_cfg_data *)data;

    cam_info("%s TODO.", __func__);

    cdata->data = sensor->board_info->sensor_index;

    hwsensor_writefile(sensor->board_info->sensor_index, sensor->board_info->name);
    return 0;
}

static hwsensor_vtbl_t s_ov8858_vtbl =
{
	.get_name = ov8858_get_name,
	.config = ov8858_config,
	.power_up = ov8858_power_up,
	.power_down = ov8858_power_down,
	.match_id = ov8858_match_id,
	.csi_enable = ov8858_csi_enable,
	.csi_disable = ov8858_csi_disable,
#ifdef DEBUG_HISI_CAMERA
	/* .sensor_register_attribute = ov8858_register_attribute, */
#endif
};

static int ov8858_config(hwsensor_intf_t* si, void  *argp)
{

    struct sensor_cfg_data *data;
    int ret =0;
	static bool ov8858_power_on = false;

	if (NULL == si || NULL == argp){
		cam_err("%s : si or argp is null", __func__);
		return -1;
	}
	data = (struct sensor_cfg_data *)argp;

	cam_debug("ov8858 cfgtype = %d",data->cfgtype);
	switch(data->cfgtype){
		case SEN_CONFIG_POWER_ON:
			if (!ov8858_power_on) {
				ret = si->vtbl->power_up(si);
				ov8858_power_on = true;
			}
			break;
		case SEN_CONFIG_POWER_OFF:
			if (ov8858_power_on) {
				ret = si->vtbl->power_down(si);
				ov8858_power_on = false;
			}
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
	cam_debug("%s exit",__func__);
	return ret;
}

static int32_t ov8858_platform_probe(struct platform_device* pdev)
{
    if (NULL == pdev){
        cam_err("%s pdev is null.\n", __func__);
        return -1;
    }
	int rc = 0;
	cam_notice("enter %s",__func__);
	if (pdev->dev.of_node) {
		rc = hw_sensor_get_dt_data(pdev, &s_ov8858);
		if (rc < 0) {
			cam_err("%s failed line %d\n", __func__, __LINE__);
			goto ov8858_sensor_probe_fail;
		}
	} else {
		cam_err("%s ov8858 of_node is NULL.\n", __func__);
		goto ov8858_sensor_probe_fail;
	}

    s_ov8858.dev = &pdev->dev;

	rc = hwsensor_register(pdev, &s_ov8858.intf);
    if (0 != rc){
        cam_err("%s hwsensor_register fail.\n", __func__);
        goto ov8858_sensor_probe_fail;
    }
	rc = rpmsg_sensor_register(pdev, (void *)&s_ov8858);
    if (0 != rc){
        cam_err("%s rpmsg_sensor_register fail.\n", __func__);
        hwsensor_unregister(&s_ov8858.intf);
        goto ov8858_sensor_probe_fail;
    }

ov8858_sensor_probe_fail:
	return rc;
}

static int __init ov8858_init_module(void)
{
    cam_notice("enter %s",__func__);
    return platform_driver_probe(&s_ov8858_driver,
            ov8858_platform_probe);
}

static void __exit ov8858_exit_module(void)
{
    rpmsg_sensor_unregister((void *)&s_ov8858);
    hwsensor_unregister(&s_ov8858.intf);
    platform_driver_unregister(&s_ov8858_driver);
}

module_init(ov8858_init_module);
module_exit(ov8858_exit_module);
MODULE_DESCRIPTION("ov8858");
MODULE_LICENSE("GPL v2");
#pragma GCC diagnostic pop
