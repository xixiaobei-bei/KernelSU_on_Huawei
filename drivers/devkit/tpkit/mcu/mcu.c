/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: watch tp support hybrid
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/of_gpio.h>
#include "huawei_ts_kit.h"
#include "huawei_ts_kit_api.h"
#include "keypad.h"
#include "securec.h"
#include "ext_sensorhub_api.h"

#define MCU_VENDER_NAME            "mcu_input"
#define TP_SERVICE_ID              0X01
#define TP_READ_COMMAND_ID         0X97
#define TP_WRITE_COMMAND_ID        0X98
#define MCU_CROWN_COMMAND_ID       0X99
#define MCU_POWER_COMMAND_ID       0X9A
#define MCU_FUNCK_COMMAND_ID       0X9B
#define MCU_ENABLE_COMMAND_ID      0X9C
#define TP_SID_CID_COUNT           6
#define MCU_REPORT_BUFFER_LEN      24
#define MCU_RESPONSE_BUFFER_LEN    24
#define MCU_ABS_MIN                0
#define MCU_ABS_MAX                0xff
#define TRACKING_ID_MAX            2
#define REPORT_TIMEOUT             50
#define TP_EVENT                   0
#define XPOSITION                  4
#define YPOSITION                  8
#define KEY_DATA_LEN               4

static unsigned char g_mcutp_response_info[MCU_RESPONSE_BUFFER_LEN] = {0};
static struct sid_cid g_tp_read_mcu_sid_cid[TP_SID_CID_COUNT] = {
	{ TP_SERVICE_ID, TP_READ_COMMAND_ID },
	{ TP_SERVICE_ID, TP_WRITE_COMMAND_ID },
	{ TP_SERVICE_ID, MCU_CROWN_COMMAND_ID },
	{ TP_SERVICE_ID, MCU_POWER_COMMAND_ID },
	{ TP_SERVICE_ID, MCU_FUNCK_COMMAND_ID },
	{ TP_SERVICE_ID, MCU_ENABLE_COMMAND_ID },
};
static struct subscribe_cmds g_tp_read_mcu_cmds = {
	g_tp_read_mcu_sid_cid, TP_SID_CID_COUNT,
};

struct mcu_tp_info {
	struct device *dev;
	struct work_struct monitor_mcutp_report_work;
	struct work_struct wait_mcutp_response_work;
	struct device_node *node;
	struct input_dev *input_dev;
	struct mutex lock;
	int x_max;
	int y_max;
	int x_max_mt;
	int y_max_mt;
};

struct monitor_mcutp_work_struct {
	struct work_struct monitor_mcutp_report_work;
	unsigned char mcutp_report_info[MCU_REPORT_BUFFER_LEN];
	unsigned char cmd;
};

struct mcu_tp_info g_tp_info;
static int g_funckey_status = 1;

static int mcu_init_chip(void);
static int mcu_input_config(struct input_dev *input_dev);
static int register_mcu_report_callback(void);
static int check_recv_mcu_data(unsigned char command_id,
	unsigned char *data, int data_len);
static int tp_recv_mcu_cb(unsigned char service_id,
	unsigned char command_id, unsigned char *data, int data_len);
static void monitor_mcutp_report_work(struct work_struct *work);

static int register_mcu_report_callback(void)
{
	return register_data_callback(UPGRADE_CHANNEL, &g_tp_read_mcu_cmds,
		tp_recv_mcu_cb);
}

static int check_recv_mcu_data(unsigned char command_id,
	unsigned char *data, int count)
{
	switch (command_id) {
	case MCU_CROWN_COMMAND_ID:
	case MCU_POWER_COMMAND_ID:
	case MCU_FUNCK_COMMAND_ID:
	case MCU_ENABLE_COMMAND_ID:
		if (!data || count != KEY_DATA_LEN) {
			TS_LOG_INFO("%s recv mcu report data error datalen:%d",
				__func__, count);
			return -EINVAL;
		}
		break;
	case TP_READ_COMMAND_ID:
	case TP_WRITE_COMMAND_ID:
		if (!data || count != MCU_RESPONSE_BUFFER_LEN) {
			TS_LOG_INFO("%s recv mcu tp data error datalen:%d",
				__func__, count);
			return -EINVAL;
		}
		break;
	default:
		TS_LOG_INFO("%s recv command_id invalid", __func__);
		return -EINVAL;
	}
	return 0;
}

static int tp_recv_mcu_cb(unsigned char service_id,
	unsigned char command_id, unsigned char *data, int data_len)
{
	int ret;
	struct monitor_mcutp_work_struct *ws = NULL;

	TS_LOG_INFO("%s enter\n", __func__);
	if (service_id != TP_SERVICE_ID) {
		TS_LOG_ERR("%s service_id:%d\n", __func__, service_id);
		return -EINVAL;
	}

	ret = check_recv_mcu_data(command_id, data, data_len);
	switch (command_id) {
	case TP_READ_COMMAND_ID:
	case MCU_CROWN_COMMAND_ID:
	case MCU_POWER_COMMAND_ID:
	case MCU_FUNCK_COMMAND_ID:
	case MCU_ENABLE_COMMAND_ID:
		/* free after callback work has done or schedule failed */
		ws = kmalloc(sizeof(*ws), GFP_KERNEL);
		if (!ws)
			return -EFAULT;
		INIT_WORK(&ws->monitor_mcutp_report_work, monitor_mcutp_report_work);
		ws->cmd = command_id;
		if (memcpy_s(ws->mcutp_report_info, MCU_REPORT_BUFFER_LEN,
			data, data_len) != EOK) {
			TS_LOG_ERR("%s memcpy mcu report data fail", __func__);
			return -ENOMEM;
		}
		if (!schedule_work(&ws->monitor_mcutp_report_work)) {
			TS_LOG_ERR("%s schedule mcutp error", __func__);
			kfree(ws);
		}
		break;
	case TP_WRITE_COMMAND_ID:
		if (memcpy_s(g_mcutp_response_info, MCU_RESPONSE_BUFFER_LEN,
			data, data_len) != EOK) {
			TS_LOG_ERR("%s memcpy response data fail", __func__);
			return -ENOMEM;
		}
		break;
	default:
		break;
	}

	return 0;
}

static int input_validate(int x, int y)
{
	if (x < 0 || y < 0 || x > g_tp_info.x_max_mt || y > g_tp_info.y_max_mt)
		return -1;
	return 0;
}

static void monitor_mcutp_report_work(struct work_struct *work)
{
	int ret;
	int x;
	int y;
	struct monitor_mcutp_work_struct *mcutp_work =
	container_of(work, struct monitor_mcutp_work_struct,
		     monitor_mcutp_report_work);

	TS_LOG_INFO("%s enter\n", __func__);
	if (!mcutp_work) {
		TS_LOG_ERR("%s mcutp_work is null\n", __func__);
		return;
	}

	mutex_lock(&g_tp_info.lock);
	if (mcutp_work->cmd == MCU_CROWN_COMMAND_ID) {
		ret = memcpy_s(&x, sizeof(int),
			&mcutp_work->mcutp_report_info[0],
			sizeof(int));
		if (ret) {
			TS_LOG_ERR("%s cpy crown data failed", __func__);
			goto exit;
		}
		input_report_rel(g_tp_info.input_dev, REL_WHEEL, x);
		input_sync(g_tp_info.input_dev);
		TS_LOG_INFO("%s crown data = %d", __func__, x);
		goto exit;
	}
	if (mcutp_work->cmd == MCU_POWER_COMMAND_ID) {
		ret = memcpy_s(&x, sizeof(int),
			&mcutp_work->mcutp_report_info[0],
			sizeof(int));
		if (ret) {
			TS_LOG_ERR("%s cpy power data failed", __func__);
			goto exit;
		}
		input_report_key(g_tp_info.input_dev, KEY_POWER, x);
		input_sync(g_tp_info.input_dev);
		TS_LOG_INFO("%s power data = %d", __func__, x);
		goto exit;
	}
	if (mcutp_work->cmd == MCU_FUNCK_COMMAND_ID) {
		ret = memcpy_s(&x, sizeof(int),
			&mcutp_work->mcutp_report_info[0],
			sizeof(int));
		if (ret) {
			TS_LOG_ERR("%s cpy funck data failed", __func__);
			goto exit;
		}
		input_report_key(g_tp_info.input_dev, KEY_F26, x);
		input_sync(g_tp_info.input_dev);
		TS_LOG_INFO("%s func data = %d", __func__, x);
		goto exit;
	}
	if (mcutp_work->cmd == MCU_ENABLE_COMMAND_ID) {
		ret = memcpy_s(&x, sizeof(int),
			&mcutp_work->mcutp_report_info[0],
			sizeof(int));
		if (ret) {
			TS_LOG_INFO("%s cpy func data failed", __func__);
			goto exit;
		}
		if (!x && g_funckey_status) {
			TS_LOG_INFO("%s disable the function key", __func__);
			func_key_irq_disable();
			g_funckey_status = 0;
		}
		if (x && !g_funckey_status) {
			TS_LOG_INFO("%s enable the function key", __func__);
			func_key_irq_enable();
			g_funckey_status = 1;
		}
		TS_LOG_INFO("%s data = %d, status = %d", __func__, x, g_funckey_status);
		goto exit;
	}

	if (mcutp_work->mcutp_report_info[TP_EVENT] == 1) {
		input_mt_sync(g_tp_info.input_dev);
		TS_LOG_INFO("%s enter UP\n", __func__);
		input_report_key(g_tp_info.input_dev, BTN_TOUCH, 0);
		input_sync(g_tp_info.input_dev);
	} else {
		ret = memcpy_s(&x, sizeof(int),
			&mcutp_work->mcutp_report_info[XPOSITION],
			sizeof(int));
		if (ret) {
			TS_LOG_ERR("[%s]:cpy x failed!\n", __func__);
			goto exit;
		}
		ret = memcpy_s(&y, sizeof(int),
			&mcutp_work->mcutp_report_info[YPOSITION],
			sizeof(int));
		if (ret) {
			TS_LOG_ERR("[%s]:cpy y failed!\n", __func__);
			goto exit;
		}
		x = mcutp_work->mcutp_report_info[XPOSITION + 1];
		x = (x << 8) | mcutp_work->mcutp_report_info[XPOSITION];
		y = mcutp_work->mcutp_report_info[YPOSITION + 1];
		y = (y << 8) | mcutp_work->mcutp_report_info[YPOSITION];
		TS_LOG_INFO("%s get touch X = %d, Y = %d\n", __func__, x, y);
		if (input_validate(x, y) < 0) {
			TS_LOG_ERR("[%s]:x = %d, y = %d, not validate!\n",
				__func__, x, y);
			goto exit;
		}
		input_report_abs(g_tp_info.input_dev, ABS_MT_PRESSURE, MCU_ABS_MAX);
		input_report_abs(g_tp_info.input_dev, ABS_MT_POSITION_X, x);
		input_report_abs(g_tp_info.input_dev, ABS_MT_POSITION_Y, y);
		input_report_abs(g_tp_info.input_dev, ABS_MT_TRACKING_ID, 0);
		input_report_key(g_tp_info.input_dev, BTN_TOUCH, 1);
		input_mt_sync(g_tp_info.input_dev);
		input_sync(g_tp_info.input_dev);
	}
exit:
	mutex_unlock(&g_tp_info.lock);
	kfree(mcutp_work);
}

static int mcu_init_chip(void)
{
	int rc;

	TS_LOG_INFO("%s enter\n", __func__);
	rc = register_mcu_report_callback();
	if (rc < 0)
		TS_LOG_ERR("%s callback register fail ret=%d\n", __func__, rc);
	return rc;
}

static int mcu_get_dts_value(struct device_node *core_node, char *name)
{
	int rc;
	int value;

	rc = of_property_read_u32(core_node, name, &value);
	if (rc) {
		TS_LOG_DEBUG("%s,[%s] read fail, rc = %d.\n", __func__,
			name, rc);
		return -1;
	}
	return value;
}

static int mcu_input_config(struct input_dev *input_dev)
{
	if (!input_dev) {
		TS_LOG_ERR("%s:info is Null\n", __func__);
		return -EINVAL;
	}
	g_tp_info.x_max = mcu_get_dts_value(g_tp_info.node, "x_max");
	g_tp_info.y_max = mcu_get_dts_value(g_tp_info.node, "y_max");
	g_tp_info.x_max_mt = mcu_get_dts_value(g_tp_info.node, "x_max_mt");
	g_tp_info.y_max_mt = mcu_get_dts_value(g_tp_info.node, "y_max_mt");
	if (g_tp_info.x_max < 0 || g_tp_info.y_max < 0
		|| g_tp_info.x_max_mt < 0 || g_tp_info.y_max_mt < 0) {
		TS_LOG_ERR("%s:get info error!\n", __func__);
		return -1;
	}
	set_bit(EV_KEY, input_dev->evbit);
	set_bit(BTN_TOUCH, input_dev->keybit);
	set_bit(KEY_F26, input_dev->keybit);
	set_bit(KEY_POWER, input_dev->keybit);
	set_bit(EV_ABS, input_dev->evbit);
	set_bit(INPUT_PROP_DIRECT, input_dev->propbit);
	set_bit(ABS_X, input_dev->absbit);
	set_bit(ABS_Y, input_dev->absbit);
	set_bit(ABS_MT_TOUCH_MAJOR, input_dev->absbit);
	set_bit(ABS_MT_WIDTH_MAJOR, input_dev->absbit);
	set_bit(ABS_MT_POSITION_X, input_dev->absbit);
	set_bit(ABS_MT_POSITION_Y, input_dev->absbit);
	set_bit(ABS_MT_TOOL_TYPE, input_dev->absbit);
	set_bit(ABS_MT_BLOB_ID, input_dev->absbit);
	set_bit(ABS_MT_TRACKING_ID, input_dev->absbit);
	input_set_capability(input_dev, EV_KEY, KEY_F26);
	input_set_capability(input_dev, EV_KEY, KEY_POWER);
	input_set_capability(input_dev, EV_KEY, KEY_SLEEP);
	input_set_capability(input_dev, EV_REL, REL_WHEEL);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, MCU_ABS_MIN, MCU_ABS_MAX,
		0, 0);
	input_set_abs_params(input_dev, ABS_MT_WIDTH_MAJOR, MCU_ABS_MIN, MCU_ABS_MAX,
		0, 0);
	input_set_abs_params(input_dev, ABS_MT_TRACKING_ID, 0,
			TRACKING_ID_MAX, 0, 0);
	input_set_abs_params(input_dev, ABS_PRESSURE, MCU_ABS_MIN, MCU_ABS_MAX, 0, 0);

	input_set_abs_params(input_dev, ABS_MT_POSITION_X,
				g_tp_info.x_max, g_tp_info.x_max_mt, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y,
				g_tp_info.y_max, g_tp_info.y_max_mt, 0, 0);
	return 0;
}

static int mcutp_probe(struct platform_device *pdev)
{
	int rc;

	TS_LOG_INFO("%s: called here\n", __func__);
	mutex_init(&g_tp_info.lock);
	rc = mcu_init_chip();
	if (rc < 0)
		mutex_destroy(&g_tp_info.lock);
	TS_LOG_INFO("%s: end\n", __func__);
	return rc;
}

static s32 mcutp_remove(struct platform_device *pdev)
{
	mutex_destroy(&g_tp_info.lock);
	if (g_tp_info.input_dev) {
		input_free_device(g_tp_info.input_dev);
		input_unregister_device(g_tp_info.input_dev);
	}
	return 0;
}

static int mcutp_input_device_register(void)
{
	int error;
	struct mcu_tp_info *data = &g_tp_info;

	data->input_dev = input_allocate_device();
	if (!data->input_dev) {
		TS_LOG_ERR("failed to allocate memory for input tp dev\n");
		error = -ENOMEM;
		goto err_out;
	}

	data->input_dev->name = MCU_VENDER_NAME;
	if (!data->input_dev) {
		TS_LOG_ERR("%s:info is Null\n", __func__);
		goto err_out;
	}

	error = mcu_input_config(data->input_dev);
	if (error < 0) {
		TS_LOG_ERR("%s:input info config failed!\n", __func__);
		goto err_out;
	}

	error = input_register_device(data->input_dev);
	if (error) {
		TS_LOG_ERR("input dev register failed : %d\n", error);
		goto err_free_dev;
	}
	TS_LOG_INFO("%s exit, %d, error is %d\n", __func__, __LINE__, error);

	return error;

err_free_dev:
	input_free_device(data->input_dev);
err_out:
	return error;
}

static const struct of_device_id mcutp_match[] = {
	{
		.compatible = "huawei,mcutp",
		.data = NULL,
	},
	{},
};

MODULE_DEVICE_TABLE(of, mcutp_match);

static struct platform_driver mcutp_driver = {
	.probe = mcutp_probe,
	.remove = mcutp_remove,
	.driver = {
		.name = "mcutp",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(mcutp_match),
	},
};

static int __init mcutp_init(void)
{
	struct device_node *node = NULL;
	int error;

	TS_LOG_INFO("%s: called here\n", __func__);
	node = of_find_compatible_node(NULL, NULL, "huawei,mcutp");
	if (!node) {
		TS_LOG_ERR("find_compatible_node mcutp error\n");
		return -EINVAL;
	}

	g_tp_info.node = node;
	error = mcutp_input_device_register();
	if (error) {
		TS_LOG_ERR(" mcutp chip register fail !\n");
		return -EINVAL;
	}
	TS_LOG_INFO("mcutp chip_register end!\n");
	return platform_driver_register(&mcutp_driver);
}

static void __exit mcutp_exit(void)
{
	TS_LOG_INFO("mcu_module_exit called here\n");
	platform_driver_unregister(&mcutp_driver);
}

module_init(mcutp_init);
module_exit(mcutp_exit);

MODULE_DESCRIPTION("Huawei TouchScreen driver");
MODULE_LICENSE("GPL");
