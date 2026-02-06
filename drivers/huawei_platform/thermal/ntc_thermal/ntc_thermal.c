/*
 * ntc_thermal.c
 *
 * thermal for ntc module
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/thermal.h>
#include <securec.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <huawei_platform/log/hw_log.h>
#include "ext_sensorhub_api.h"

#define POWER_TZ_STR_MAX_LEN      16

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG watch_ntc
HWLOG_REGIST();

#define NTC_SERVICE_ID      0x01
#define NTC_COMMAND_ID      0x82
#define NTC_TEMP_RSP_TYPE   0x02
#define NTC_BYTES_RSP_LEN   6
#define NTC_TEMP_DEFAULT    30
#define NTC_RSP_LEN         4
#define NTC_TEMP_INVALID    (-23)
#define NTC_TEMP_CONST      1000
#define DECIMAL             10

static int g_temp_last = NTC_TEMP_DEFAULT;

struct watch_ntc_tz_sensor {
	char sensor_name[POWER_TZ_STR_MAX_LEN];
	char ops_name[POWER_TZ_STR_MAX_LEN];
	struct thermal_zone_device *tz_dev;
};

struct ntc_thermal_priv {
	struct device *dev;
	struct watch_ntc_tz_sensor sensor;
};

static struct ntc_thermal_priv *g_power_tz_info;
struct hw_thermal_class {
	struct class *thermal_class;
	struct device *temperature_device;
};

struct hw_thermal_class hw_thermal_info;

static int parse_ntc_temp(unsigned char *data)
{
	int ntc_temp;
	int ret;

	ret = memcpy_s(&ntc_temp, NTC_RSP_LEN, data, NTC_RSP_LEN);
	if (ret) {
		hwlog_err("[%s]:memcpy ntc temp failed\n", __func__);
		return NTC_TEMP_INVALID;
	}
	ntc_temp = ntohl(ntc_temp);
	hwlog_info("[%s]:ntc temp is %d\n", __func__, ntc_temp);
	g_temp_last = ntc_temp;
	return ntc_temp;
}

static int watch_query_ntc_info(int *raw_temp)
{
	int ret;
	int payload = 0;
	int payload_len = 0;
	unsigned char send_cmd[] = { 0x01, 0x01, 0x01 };
	struct command cmd = {
		.service_id = NTC_SERVICE_ID,
		.command_id = NTC_COMMAND_ID,
		.send_buffer = &send_cmd[0],
		.send_buffer_len = sizeof(send_cmd),
	};

	struct cmd_resp mcu_rsp;
	mcu_rsp.receive_buffer_len = NTC_BYTES_RSP_LEN;
	mcu_rsp.receive_buffer = (unsigned char *)kmalloc(mcu_rsp.receive_buffer_len, GFP_KERNEL);

	if (!mcu_rsp.receive_buffer)
		return -1;

	ret = send_command(CHARGE_CHANNEL, &cmd, true, &mcu_rsp);
	if (ret < 0) {
		hwlog_err("get ntc temp failed, ret %d\n", ret);
		goto err;
	}

	if (mcu_rsp.receive_buffer[payload++] != NTC_TEMP_RSP_TYPE) {
		hwlog_err("receive buff is %02x\n", mcu_rsp.receive_buffer[payload]);
		ret = -1;
		goto err;
	}

	payload_len = mcu_rsp.receive_buffer[payload++];
	if (payload_len != NTC_RSP_LEN) {
		ret = -1;
		hwlog_err("%s get ntc temp payloadlen:%d\n", __func__, payload_len);
		goto err;
	}

	*raw_temp = parse_ntc_temp(&mcu_rsp.receive_buffer[payload]);
	if (raw_temp == NTC_TEMP_INVALID) {
		ret = -1;
		hwlog_err("parse_ntc_temp failed, raw_temp %d\n", raw_temp);
		goto err;
	}

	kfree(mcu_rsp.receive_buffer);
	return 0;

err:
	*raw_temp = g_temp_last;
	kfree(mcu_rsp.receive_buffer);
	return ret;
}

static int ntc_thermal_get_temp(struct thermal_zone_device *thermal, int *temp)
{
	int raw_temp;

	if (!thermal || !temp)
		return -EINVAL;

	watch_query_ntc_info(&raw_temp);
	*temp = raw_temp * NTC_TEMP_CONST;
	return 0;
}

static struct thermal_zone_device_ops ntc_thermal_ops = {
	.get_temp = ntc_thermal_get_temp,
};

static int ntc_thermal_parse_dts(struct device_node *dev_node,
	struct ntc_thermal_priv *info)
{
	const char *sensor_name = NULL;
	const char *ops_name = NULL;

	if (power_dts_read_string(power_dts_tag(HWLOG_TAG), dev_node,
		"sensor_name", &sensor_name))
		return -EINVAL;

	if (power_dts_read_string(power_dts_tag(HWLOG_TAG), dev_node,
		"ops_name", &ops_name))
		return -EINVAL;

	strncpy_s(info->sensor.sensor_name, sizeof(info->sensor.sensor_name),
		sensor_name, (POWER_TZ_STR_MAX_LEN - 1));
	strncpy_s(info->sensor.ops_name, sizeof(info->sensor.ops_name),
		ops_name, (POWER_TZ_STR_MAX_LEN - 1));

	return 0;
}

static ssize_t
hisi_shell_show_temp(struct device *dev, struct device_attribute *devattr, char *buf)
{
	int temp;
	struct thermal_zone_device thermal;

	if (dev == NULL || devattr == NULL || buf == NULL)
		return 0;

	if (dev->driver_data == NULL)
		return 0;

	ntc_thermal_get_temp(&thermal, &temp);

	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n", temp);
}

static ssize_t
hisi_shell_store_temp(struct device *dev, struct device_attribute *devattr,
	const char *buf, size_t count)
{
	int temp;

	if (dev == NULL || devattr == NULL || buf == NULL)
		return 0;

	if (kstrtoint(buf, DECIMAL, &temp) != 0) {
		pr_err("%s Invalid input para\n", __func__);
		return -EINVAL;
	}

	return (ssize_t)count;
}

static DEVICE_ATTR(temp, S_IWUSR | S_IRUGO,
	hisi_shell_show_temp, hisi_shell_store_temp);

static struct attribute *hisi_shell_attributes[] = {
	&dev_attr_temp.attr,
	NULL
};

static struct attribute_group hisi_shell_attribute_group = {
	.attrs = hisi_shell_attributes,
};

static int
create_file_node(struct platform_device *pdev, struct ntc_thermal_priv *priv,
	struct attribute_group *attr)
{
	struct device *dev = &pdev->dev;
	struct device_node *dev_node = dev->of_node;
	int ret;

	ret = sysfs_create_link(&hw_thermal_info.temperature_device->kobj,
			&pdev->dev.kobj, "shell_frame");
	if (ret != 0) {
		hwlog_err("%s: create hw_thermal device file error: %d\n", dev_node->name, ret);
		return -EINVAL;
	}
	ret = sysfs_create_group(&pdev->dev.kobj, attr);
	if (ret != 0) {
		hwlog_err("%s: create shell file error: %d\n", dev_node->name, ret);
		sysfs_remove_link(&hw_thermal_info.temperature_device->kobj, "shell_frame");
		return -EINVAL;
	}
	return 0;
}
static int ntc_thermal_probe(struct platform_device *pdev)
{
	struct ntc_thermal_priv *priv = NULL;
	int ret;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	priv = devm_kzalloc(&pdev->dev, sizeof(priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	g_power_tz_info = priv;
	priv->dev = &pdev->dev;

	ret = ntc_thermal_parse_dts(priv->dev->of_node, priv);
	if (ret < 0) {
		hwlog_err("ntc_thermal_parse_dts failed\n");
		goto fail_parse_dts;
	}

	priv->sensor.tz_dev = thermal_zone_device_register(
		priv->sensor.sensor_name, 0, 0, priv,
		&ntc_thermal_ops, NULL, 0, 0);
	if (IS_ERR(priv->sensor.tz_dev)) {
		hwlog_err("ntc thermal zone register fail\n");
		ret = -ENODEV;
		goto fail_register_tz;
	}

	platform_set_drvdata(pdev, priv);
	ret = create_file_node(pdev, priv, &hisi_shell_attribute_group);
	if (ret != 0)
		goto fail_register_tz;

	return 0;

fail_register_tz:
	thermal_zone_device_unregister(priv->sensor.tz_dev);

fail_parse_dts:
	of_node_put(priv->dev->of_node);

fail_free_mem:
	devm_kfree(&pdev->dev, priv);
	g_power_tz_info = NULL;

	return ret;
}

static int ntc_thermal_remove(struct platform_device *pdev)
{
	struct ntc_thermal_priv *priv = platform_get_drvdata(pdev);

	if (!priv)
		return -ENODEV;

	thermal_zone_device_unregister(priv->sensor.tz_dev);

	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, priv);
	g_power_tz_info = NULL;

	return 0;
}

static const struct of_device_id ntc_thermal_match[] = {
	{
		.compatible = "hisilicon,ntc_thermal",
		.data = NULL,
	},
	{},
};

static struct platform_driver ntc_thermal_driver = {
	.probe = ntc_thermal_probe,
	.remove = ntc_thermal_remove,
	.driver = {
		.name = "ntc thermal",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(ntc_thermal_match),
	},
};

static int __init ntc_thermal_init(void)
{
	/* create huawei thermal class */
	hw_thermal_info.thermal_class = class_create(THIS_MODULE, "hw_thermal");
	if (IS_ERR(hw_thermal_info.thermal_class)) {
		hwlog_err("Huawei thermal class create error\n");
		return PTR_ERR(hw_thermal_info.thermal_class);
	}

	/* create device "temp" */
	hw_thermal_info.temperature_device =
		device_create(hw_thermal_info.thermal_class, NULL, 0, NULL, "temp");
	if (IS_ERR(hw_thermal_info.temperature_device)) {
		hwlog_err("hw_thermal:temperature device create error\n");
		class_destroy(hw_thermal_info.thermal_class);
		hw_thermal_info.thermal_class = NULL;
		return PTR_ERR(hw_thermal_info.temperature_device);
	}
	return platform_driver_register(&ntc_thermal_driver);
}

static void __exit ntc_thermal_exit(void)
{
	if (hw_thermal_info.thermal_class != NULL) {
		device_destroy(hw_thermal_info.thermal_class, 0);
		class_destroy(hw_thermal_info.thermal_class);
	}
	platform_driver_unregister(&ntc_thermal_driver);
}

module_init(ntc_thermal_init);
module_exit(ntc_thermal_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei ext_ntc temp driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");