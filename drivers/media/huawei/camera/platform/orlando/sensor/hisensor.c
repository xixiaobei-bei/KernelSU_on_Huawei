

#include <linux/module.h>
#include <linux/printk.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/rpmsg.h>
#include <linux/hisi/hisi_adc.h>

#include "hwsensor.h"
#include "sensor_commom.h"
#include "hw_csi.h"

#define I2S(i) container_of(i, sensor_t, intf)
#define Sensor2Pdev(s) container_of((s).dev, struct platform_device, dev)
#define SENSOR_REG_SETTING_DELAY_1_MS (1)
#define GPIO_LOW_STATE    (0)
#define GPIO_HIGH_STATE   (1)
#define CTL_RESET_HOLD    (0)
#define CTL_RESET_RELEASE (1)

#define RESET_TYPE_NONE    (0)
#define RESET_TYPE_PRIMARY (1)
#define RESET_TYPE_SECOND  (2)
#define RESET_TYPE_BOTH    (3)
#define RESET_TYPE_CHANGEABLE 4
#define MAX_SENSOR_INDEX 7
#define CAMERA_DRIVER_SLEEP_TIME    (2)
#define CTL_RESET_HOLD_DELAY_TIME    (2000)

//lint -save -e846 -e514 -e866 -e429 -e605 -e30 -e84 -e785 -e64 -e826 -e838
//lint -save -e715 -e747 -e778 -e774 -e732 -e650 -e31 -e731 -e528 -e753 -e737

static int g_rpc_camera_sensor_status[MAX_SENSOR_INDEX];

extern int rpc_status_change_for_camera(unsigned int status);

struct mutex hisensor_power_lock;
enum camera_metadata_enum_android_hw_dual_primary_mode {
	ANDROID_HW_DUAL_PRIMARY_FIRST  = 0,
	ANDROID_HW_DUAL_PRIMARY_SECOND = 2,
	ANDROID_HW_DUAL_PRIMARY_BOTH   = 3,
};

char const *hisensor_get_name(hwsensor_intf_t *si)
{
	sensor_t *sensor = NULL;

	if (!si) {
		cam_err("%s. si is null.", __func__);
		return NULL;
	}

	sensor = I2S(si);
	if (sensor == NULL || sensor->board_info == NULL) {
		cam_err("%s. sensor or board_info->name is NULL.", __func__);
		return NULL;
	}

	return sensor->board_info->name;
}

int hisensor_power_up(hwsensor_intf_t *si)
{
	int ret          = 0;
	sensor_t *sensor = NULL;

	if (!si) {
		cam_err("%s. si is null.", __func__);
		return -EINVAL;
	}

	sensor = I2S(si);
	if (sensor == NULL || sensor->board_info == NULL || sensor->board_info->name == NULL) {
		cam_err("%s. sensor or board_info->name is NULL.", __func__);
		return -EINVAL;
	}

	cam_info("enter %s. index = %d", __func__, sensor->board_info->sensor_index);

	if (hw_is_fpga_board()) {
		ret = do_sensor_power_on(sensor->board_info->sensor_index, sensor->board_info->name);
	} else {
		ret = hw_sensor_power_up(sensor);
	}

	if (ret == 0) {
		cam_info("%s. power up sensor success.", __func__);
	} else {
		cam_err("%s. power up sensor fail.", __func__);
	}

	return ret;
}

int hisensor_power_down(hwsensor_intf_t *si)
{
	int ret          = 0;
	sensor_t *sensor = NULL;

	if (!si) {
		cam_err("%s. si is null.", __func__);
		return -EINVAL;
	}

	sensor = I2S(si);
	if (sensor == NULL || sensor->board_info == NULL || sensor->board_info->name == NULL) {
		cam_err("%s. sensor or board_info->name is NULL.", __func__);
		return -EINVAL;
	}

	cam_info("enter %s. index = %d", __func__, sensor->board_info->sensor_index);

	if (hw_is_fpga_board()) {
		ret = do_sensor_power_off(sensor->board_info->sensor_index, sensor->board_info->name);
	} else {
		ret = hw_sensor_power_down(sensor);
	}

	if (ret == 0) {
		cam_info("%s. power down sensor success.", __func__);
	} else{
		cam_err("%s. power down sensor fail.", __func__);
	}

	return ret;
}

int hisensor_csi_enable(hwsensor_intf_t *si)
{
	(void)si;
	return 0;
}

int hisensor_csi_disable(hwsensor_intf_t *si)
{
	(void)si;
	return 0;
}

static int hisensor_match_id(hwsensor_intf_t *si, void *data)
{
	sensor_t *sensor = NULL;
	struct sensor_cfg_data *cdata = NULL;

	if (si == NULL || data == NULL) {
		cam_err("%s. si or data is NULL.", __func__);
		return -EINVAL;
	}

	cam_info("%s enter.", __func__);
	sensor = I2S(si);
	if (sensor == NULL || sensor->board_info == NULL) {
		cam_err("%s. sensor or board_info is NULL.", __func__);
		return -EINVAL;
	}

	cdata = (struct sensor_cfg_data *)data;
	cdata->data = sensor->board_info->sensor_index;

	return 0;
}

static int hisensor_do_hw_mipisw(hwsensor_board_info_t *b_info, int ctl)
{
	//lint -save -e826 -e778 -e774 -e747
	int ret = 0;
	if (b_info == NULL) {
		cam_warn("%s invalid sensor board info", __func__);
		return 0;
	}

	if (b_info->dynamic_mipisw_num == 1) {
		if (ctl == CTL_RESET_RELEASE) {
			ret  = gpio_request(b_info->gpios[MIPI_SW].gpio, "mipisw");
			if (ret) {
				cam_err("%s requeset mipisw pin failed", __func__);
				return ret;
			}
			ret = gpio_direction_output(b_info->gpios[MIPI_SW].gpio, b_info->mipisw_enable_value0);
			cam_info("enable mipisw, mipisw_enable_value0 = %d", b_info->mipisw_enable_value0);
			gpio_free(b_info->gpios[MIPI_SW].gpio);
		}
	} else if (b_info->dynamic_mipisw_num == 2) { // 2 for dynamic_camera
		if (ctl == CTL_RESET_RELEASE) {
			ret  = gpio_request(b_info->gpios[MIPI_SW].gpio, "mipisw");
			if (ret) {
				cam_err("%s requeset mipisw pin failed", __func__);
				return ret;
			}
			ret = gpio_request(b_info->gpios[MIPI_SW2].gpio, "mipisw2");
			if (ret) {
				cam_err("%s:%d requeset mipisw2 pin failed", __func__, __LINE__);
				gpio_free(b_info->gpios[MIPI_SW].gpio);
				return ret;
			}
			ret |= gpio_direction_output(b_info->gpios[MIPI_SW].gpio, b_info->mipisw_enable_value0);
			ret |= gpio_direction_output(b_info->gpios[MIPI_SW2].gpio, b_info->mipisw_enable_value1);
			cam_info("enable mipisw, mipisw_enable_value0 = %d", b_info->mipisw_enable_value0);
			cam_info("enable mipisw2, mipisw_enable_value1 = %d", b_info->mipisw_enable_value1);
			gpio_free(b_info->gpios[MIPI_SW].gpio);
			gpio_free(b_info->gpios[MIPI_SW2].gpio);
		}
	}
	return ret;
}

static void hisensor_deliver_camera_status(struct work_struct *work)
{
	if (work == NULL) {
		cam_err("%s work is null", __func__);
		return;
	}

	rpc_info_t *rpc_info = container_of(work, rpc_info_t, rpc_work);

	if (rpc_info == NULL) {
		cam_err("%s rpc_info is null", __func__);
		return;
	}

	if (rpc_status_change_for_camera(rpc_info->camera_status)) {
		cam_err("%s set_rpc %d fail", __func__, rpc_info->camera_status);
	} else {
		cam_info("%s set_rpc %d success", __func__, rpc_info->camera_status);
	}
}

static int hisensor_do_hw_reset(hwsensor_intf_t *si, int ctl, int id)
{
	//lint -save -e826 -e778 -e774 -e747
	sensor_t *sensor = I2S(si);
	hwsensor_board_info_t *b_info = NULL;
	int ret = 0;
	int sensorIndex;
	if (sensor == NULL) {
		cam_err("%s. sensor is NULL.", __func__);
		return -EINVAL;
	}
	b_info = sensor->board_info;
	if (b_info == NULL) {
		cam_warn("%s invalid sensor board info", __func__);
		return 0;
	}
	cam_info("reset_type = %d ctl %d id %d need_rpc %d", b_info->reset_type, ctl, id, b_info->need_rpc);
	if (b_info->need_rpc == 1) {
		if (ctl == CTL_RESET_RELEASE)
			g_rpc_camera_sensor_status[b_info->sensor_index] = 1;
		else
			g_rpc_camera_sensor_status[b_info->sensor_index] = 0;
		for (sensorIndex = 0; sensorIndex < MAX_SENSOR_INDEX; sensorIndex++) {
			if (g_rpc_camera_sensor_status[sensorIndex] == 1) {
				b_info->rpc_info.camera_status = 1;
				break;
			} else {
				b_info->rpc_info.camera_status = 0;
			}
		}
		if (b_info->rpc_info.rpc_work_queue != NULL)
			queue_work(b_info->rpc_info.rpc_work_queue, &(b_info->rpc_info.rpc_work));
		else
			cam_err("%s rpc_work_queue is null", __func__);
	}
	if (b_info->dynamic_mipisw_num > 0) {
		ret = hisensor_do_hw_mipisw(b_info, ctl);
		if (ret) {
			cam_err("%s hisensor_do_hw_mipisw failed", __func__);
		}
	}

	if (b_info->reset_type == RESET_TYPE_PRIMARY) {
		ret  = gpio_request(b_info->gpios[RESETB].gpio, "reset-0");
		if (ret) {
			cam_err("%s requeset reset pin failed", __func__);
			return ret;
		}

		if (ctl == CTL_RESET_HOLD) {
			ret  = gpio_direction_output(b_info->gpios[RESETB].gpio, CTL_RESET_HOLD);
		} else {
			ret  = gpio_direction_output(b_info->gpios[RESETB].gpio, CTL_RESET_RELEASE);
			hw_camdrv_msleep(CAMERA_DRIVER_SLEEP_TIME);
		}
		gpio_free(b_info->gpios[RESETB].gpio);
	} else if (b_info->reset_type == RESET_TYPE_SECOND) {
		ret = gpio_request(b_info->gpios[RESETB2].gpio, "reset-1");
		if (ret) {
			cam_err("%s requeset reset2 pin failed", __func__);
			return ret;
		}

		if (ctl == CTL_RESET_HOLD) {
			if ((id == ANDROID_HW_DUAL_PRIMARY_SECOND) || (id == ANDROID_HW_DUAL_PRIMARY_BOTH)) {
				ret = gpio_direction_output(b_info->gpios[RESETB2].gpio, CTL_RESET_HOLD);
				cam_info("RESETB2 = CTL_RESET_HOLD");
			}
		} else if (ctl == CTL_RESET_RELEASE) {
			if ((id == ANDROID_HW_DUAL_PRIMARY_SECOND) || (id == ANDROID_HW_DUAL_PRIMARY_BOTH)) {
				ret = gpio_direction_output(b_info->gpios[RESETB2].gpio, CTL_RESET_RELEASE);
				cam_info("RESETB2 = CTL_RESET_RELEASE");
				hw_camdrv_msleep(CAMERA_DRIVER_SLEEP_TIME);
			}
		}

		gpio_free(b_info->gpios[RESETB2].gpio);
	} else if (b_info->reset_type == RESET_TYPE_BOTH) {
		ret  = gpio_request(b_info->gpios[RESETB].gpio, "reset-0");
		if (ret) {
			cam_err("%s:%d requeset reset pin failed", __func__, __LINE__);
			return ret;
		}

		ret = gpio_request(b_info->gpios[RESETB2].gpio, "reset-1");
		if (ret) {
			cam_err("%s:%d requeset reset pin failed", __func__, __LINE__);
			gpio_free(b_info->gpios[RESETB].gpio);
			return ret;
		}

		if (ctl == CTL_RESET_HOLD) {
			ret |= gpio_direction_output(b_info->gpios[RESETB].gpio, CTL_RESET_HOLD);
			ret |= gpio_direction_output(b_info->gpios[RESETB2].gpio, CTL_RESET_HOLD);
			cam_info("RESETB = CTL_RESET_HOLD, RESETB2 = CTL_RESET_HOLD");
			udelay(CTL_RESET_HOLD_DELAY_TIME);
		} else if (ctl == CTL_RESET_RELEASE) {
			if (id == ANDROID_HW_DUAL_PRIMARY_FIRST) {
				ret |= gpio_direction_output(b_info->gpios[RESETB].gpio, CTL_RESET_RELEASE);
				ret |= gpio_direction_output(b_info->gpios[RESETB2].gpio, CTL_RESET_HOLD);
				cam_info("RESETB = CTL_RESET_RELEASE, RESETB2 = CTL_RESET_HOLD");
			} else if (id == ANDROID_HW_DUAL_PRIMARY_BOTH) {
				ret |= gpio_direction_output(b_info->gpios[RESETB].gpio, CTL_RESET_RELEASE);
				ret |= gpio_direction_output(b_info->gpios[RESETB2].gpio, CTL_RESET_RELEASE);
				cam_info("RESETB = CTL_RESET_RELEASE, RESETB2 = CTL_RESET_RELEASE");
			} else if (id == ANDROID_HW_DUAL_PRIMARY_SECOND) {
				ret |= gpio_direction_output(b_info->gpios[RESETB2].gpio, CTL_RESET_RELEASE);
				ret |= gpio_direction_output(b_info->gpios[RESETB].gpio, CTL_RESET_HOLD);
				cam_info("RESETB = CTL_RESET_HOLD, RESETB2 = CTL_RESET_RELEASE");
			}
		}

		gpio_free(b_info->gpios[RESETB].gpio);
		gpio_free(b_info->gpios[RESETB2].gpio);
	} else if (b_info->reset_type == RESET_TYPE_CHANGEABLE) {
		ret  = gpio_request(b_info->gpios[RESETB].gpio, "reset-0");
		if (ret) {
			cam_err("%s requeset reset pin failed", __func__);
			return ret;
		}

		if (ctl == CTL_RESET_HOLD) {
			ret = gpio_direction_output(b_info->gpios[RESETB].gpio, b_info->hold_value);
			cam_info("PRIMARY CTL_RESET_HOLD, gpio = %d, hold_value = %d",
			b_info->gpios[RESETB].gpio, b_info->hold_value);
		} else {
			ret = gpio_direction_output(b_info->gpios[RESETB].gpio, b_info->release_value);
			cam_info("PRIMARY CTL_RESET_RELEASE, gpio = %d, release_value = %d",
			b_info->gpios[RESETB].gpio, b_info->release_value);
			hw_camdrv_msleep(CAMERA_DRIVER_SLEEP_TIME);
		}
		gpio_free(b_info->gpios[RESETB].gpio);
	} else {
		return 0;
	}

	if (ret) {
		cam_err("%s set reset pin failed", __func__);
	} else {
		cam_info("%s: set reset state=%d, mode=%d", __func__, ctl, id);
	}

	//lint -restore
	return ret;
}

int hisensor_config(hwsensor_intf_t *si, void *argp)
{
	int ret = 0;
	struct sensor_cfg_data *data = NULL;
	sensor_t *sensor = NULL;

	if (si == NULL || argp == NULL|| si->vtbl == NULL) {
		cam_err("%s si or argp is null.\n", __func__);
		return -EINVAL;
	}

	sensor = I2S(si);
	if (sensor == NULL) {
		cam_err("%s sensor is null.\n", __func__);
		return -EINVAL;
	}

	data = (struct sensor_cfg_data *)argp;
	cam_info("hisensor cfgtype = %d", data->cfgtype);

	switch (data->cfgtype) {
	case SEN_CONFIG_POWER_ON:
		mutex_lock(&hisensor_power_lock);
		if (sensor->state == HWSENSRO_POWER_DOWN) {
			if (si->vtbl->power_up == NULL) {
				cam_err("%s. si->vtbl->power_up is null.", __func__);
				mutex_unlock(&hisensor_power_lock);
				return -EINVAL;
			}
			ret = si->vtbl->power_up(si);
			if (ret == 0) {
				sensor->state = HWSENSOR_POWER_UP;
			} else {
				cam_err("%s. power up fail.", __func__);
			}
		}
		/*lint -e455 -esym(455,*)*/
		mutex_unlock(&hisensor_power_lock);
		/*lint -e455 +esym(455,*)*/
	break;

	case SEN_CONFIG_POWER_OFF:
		mutex_lock(&hisensor_power_lock);
		if (sensor->state == HWSENSOR_POWER_UP) {
			if (si->vtbl->power_down == NULL) {
				cam_err("%s. si->vtbl->power_down is null.", __func__);
				mutex_unlock(&hisensor_power_lock);
				return -EINVAL;
			}
			ret = si->vtbl->power_down(si);
			if (ret != 0) {
				cam_err("%s. power_down fail.", __func__);
			}
			sensor->state = HWSENSRO_POWER_DOWN;
		}
		/*lint -e455 -esym(455,*)*/
		mutex_unlock(&hisensor_power_lock);
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
		if (si->vtbl->match_id == NULL) {
			cam_err("%s. si->vtbl->match_id is null.", __func__);
			ret = -EINVAL;
		} else {
			ret = si->vtbl->match_id(si, argp);
		}
	break;

	case SEN_CONFIG_RESET_HOLD:
		ret = hisensor_do_hw_reset(si, CTL_RESET_HOLD, data->mode);
	break;

	case SEN_CONFIG_RESET_RELEASE:
		ret = hisensor_do_hw_reset(si, CTL_RESET_RELEASE, data->mode);
	break;

	default:
		cam_err("%s cfgtype %d is error", __func__, data->cfgtype);
	break;
	}

	cam_debug("%s exit %d", __func__, ret);
	return ret;
}

static hwsensor_vtbl_t hisensor_vtbl = {
	.get_name    = hisensor_get_name,
	.config      = hisensor_config,
	.power_up    = hisensor_power_up,
	.power_down  = hisensor_power_down,
	.match_id    = hisensor_match_id,
	.csi_enable  = hisensor_csi_enable,
	.csi_disable = hisensor_csi_disable,
};

static int32_t hisensor_platform_probe(struct platform_device *pdev)
{
	int rc = 0;

	sensor_t *s_ctrl = NULL;
	atomic_t *hisensor_powered = NULL;
	hwsensor_board_info_t *b_info = NULL;

	cam_info("%s enter", __func__);
	if (pdev == NULL) {
		cam_err("%s:%d pdev is null.\n", __func__, __LINE__);
		return -EINVAL;
	}

	s_ctrl = kzalloc(sizeof(sensor_t), GFP_KERNEL);
	if (!s_ctrl) {
		return -ENOMEM;
	}

	rc = hw_sensor_get_dt_data(pdev, s_ctrl);
	if (rc < 0) {
		cam_err("%s:%d no dt data rc %d", __func__, __LINE__, rc);
		rc = -ENODEV;
		goto hisensor_probe_fail;
	}

	rc = hw_sensor_get_dt_power_setting_data(pdev, s_ctrl);
	if (rc < 0) {
		cam_err("%s:%d no dt power setting data rc %d", __func__, __LINE__, rc);
		rc = -ENODEV;
		goto hisensor_probe_fail;
	}

	mutex_init(&hisensor_power_lock);
	hisensor_powered = kzalloc(sizeof(atomic_t), GFP_KERNEL);

	if (!hisensor_powered) {
		goto hisensor_probe_fail;
	}

	s_ctrl->intf.vtbl = &hisensor_vtbl;
	s_ctrl->p_atpowercnt = hisensor_powered;
	s_ctrl->dev = &pdev->dev;
	rc = hwsensor_register(pdev, &(s_ctrl->intf));
	if (rc != 0) {
		cam_err("%s:%d hwsensor_register fail.\n", __func__, __LINE__);
		goto hisensor_probe_fail;
	}

	rc = rpmsg_sensor_register(pdev, (void *)s_ctrl);
	if (rc != 0) {
		cam_err("%s:%d rpmsg_sensor_register fail.\n", __func__, __LINE__);
		hwsensor_unregister(pdev);
		goto hisensor_probe_fail;
	}

	b_info = s_ctrl->board_info;
	if ((b_info != NULL) && (b_info->need_rpc == 1)) {
		b_info->rpc_info.rpc_work_queue = create_singlethread_workqueue("camera_radio_power_ctl");
		if (b_info->rpc_info.rpc_work_queue == NULL) {
			cam_err("%s - create workqueue error", __func__);
		} else {
			INIT_WORK(&(b_info->rpc_info.rpc_work), hisensor_deliver_camera_status);
			cam_info("%s - create workqueue success", __func__);
		}
	}

	return rc;

hisensor_probe_fail:
	if (s_ctrl->power_setting_array.power_setting != NULL) {
		kfree(s_ctrl->power_setting_array.power_setting);
	}
	if (s_ctrl->power_down_setting_array.power_setting != NULL) {
		kfree(s_ctrl->power_down_setting_array.power_setting);
	}
	if (s_ctrl->board_info != NULL) {
		kfree(s_ctrl->board_info);
		s_ctrl->board_info = NULL;
	}
	if (s_ctrl->p_atpowercnt != NULL) {
		kfree((atomic_t *)s_ctrl->p_atpowercnt);
		s_ctrl->p_atpowercnt = NULL;
	}
	if (s_ctrl != NULL) {
		kfree(s_ctrl);
		s_ctrl = NULL;
	}

	return rc;
}

static int32_t hisensor_platform_remove(struct platform_device *pdev)
{
	struct v4l2_subdev *sd = NULL;
	sensor_t *s_ctrl = NULL;
	hwsensor_board_info_t *b_info = NULL;

	if (pdev == NULL) {
		cam_err("%s:%d pdev is null.\n", __func__, __LINE__);
		return -EINVAL;
	}

	sd = platform_get_drvdata(pdev);
	if (!sd) {
		cam_err("%s: Subdevice is NULL\n", __func__);
		return 0;
	}

	s_ctrl = (sensor_t *)v4l2_get_subdevdata(sd);
	if (!s_ctrl) {
		cam_err("%s: eeprom device is NULL\n", __func__);
		return 0;
	}

	b_info = s_ctrl->board_info;
	if ((b_info != NULL) && (b_info->need_rpc == 1) &&
		(b_info->rpc_info.rpc_work_queue != NULL)) {
		destroy_workqueue(b_info->rpc_info.rpc_work_queue);
		b_info->rpc_info.rpc_work_queue = NULL;
		cam_info("%s - destroy workqueue success", __func__);
	}

	rpmsg_sensor_unregister((void *)s_ctrl);

	hwsensor_unregister(pdev);

	if (s_ctrl->power_setting_array.power_setting != NULL) {
		kfree(s_ctrl->power_setting_array.power_setting);
	}
	if (s_ctrl->power_down_setting_array.power_setting != NULL) {
		kfree(s_ctrl->power_down_setting_array.power_setting);
	}
	if (s_ctrl->p_atpowercnt != NULL) {
		kfree((atomic_t *)s_ctrl->p_atpowercnt);
		s_ctrl->p_atpowercnt = NULL;
	}
	if (s_ctrl->board_info != NULL) {
		kfree(s_ctrl->board_info);
	}

	kfree(s_ctrl);

	return 0;
}

static const struct of_device_id hisensor_dt_match[] = {
	{
		.compatible = "huawei,sensor",
	}, {
	},
};

MODULE_DEVICE_TABLE(of, hisensor_dt_match);

static struct platform_driver hisensor_platform_driver = {
	.driver = {
		.name           = "huawei,sensor",
		.owner          = THIS_MODULE,
		.of_match_table = hisensor_dt_match,
	},

	.probe = hisensor_platform_probe,
	.remove = hisensor_platform_remove,
};

static int __init hisensor_init_module(void)
{
	cam_info("enter %s", __func__);
	return platform_driver_register(&hisensor_platform_driver);
}

static void __exit hisensor_exit_module(void)
{
	platform_driver_unregister(&hisensor_platform_driver);
}
//lint -restore

/*lint -e528 -esym(528,*)*/
module_init(hisensor_init_module);
module_exit(hisensor_exit_module);
/*lint -e528 +esym(528,*)*/
/*lint -e753 -esym(753,*)*/
MODULE_DESCRIPTION("hisensor");
MODULE_LICENSE("GPL v2");
/*lint -e753 +esym(753,*)*/
