/*
 * wireless_accessory.c
 *
 * wireless accessory driver
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#include <chipset_common/hwpower/wireless_charge/wireless_accessory.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG wireless_accessory
HWLOG_REGIST();

static struct wireless_acc_dev *g_wireless_acc_dev;

void wireless_acc_report_uevent(struct wireless_acc_key_info *key_info,
	u8 info_no, u8 dev_no)
{
	int i;
	char *p_uevent_data[ACC_DEV_INFO_NUM_MAX] = { 0 };
	char *p_data = NULL;
	char *p_data_tmp = NULL;
	struct wireless_acc_dev *di = g_wireless_acc_dev;

	hwlog_info("accessory notify uevent begin\n");

	if (!key_info) {
		hwlog_err("key_info is null\n");
		return;
	}

	if (!di || !di->dev) {
		hwlog_err("di or dev is null\n");
		return;
	}

	if (dev_no >= ACC_DEV_NO_END) {
		hwlog_err("dev_no=%u invalid\n", dev_no);
		return;
	}

	if (info_no > ACC_DEV_INFO_NUM_MAX) {
		hwlog_err("info_no=%u invalid\n", info_no);
		return;
	}

	di->dev_info[dev_no].info_no = info_no;
	di->dev_info[dev_no].key_info = key_info;
	p_data = kzalloc((info_no * ACC_DEV_INFO_LEN) + 1, GFP_KERNEL);
	if (!p_data)
		return;

	p_data_tmp = p_data;
	for (i = 0; i < info_no; i++) {
		snprintf(p_data_tmp, ACC_DEV_INFO_LEN, "%s=%s",
			key_info[i].name, key_info[i].value);
		p_uevent_data[i] = p_data_tmp;
		p_data_tmp += ACC_DEV_INFO_LEN;
		hwlog_info("acc uevent_data[%d]:%s\n", i, p_uevent_data[i]);
	}
	kobject_uevent_env(&di->dev->kobj, KOBJ_CHANGE, p_uevent_data);
	kfree(p_data);

	hwlog_info("accessory notify uevent end\n");
}

static ssize_t wireless_acc_dev_info_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int dev_no;
	int info_no;
	char acc_info[ACC_DEV_INFO_MAX] = { 0 };
	char *p_acc_info = acc_info;
	struct wireless_acc_dev_info *dev_info = NULL;
	struct wireless_acc_key_info *key_info = NULL;
	struct wireless_acc_dev *di = g_wireless_acc_dev;

	if (!di) {
		hwlog_err("di is null\n");
		goto out;
	}

	for (dev_no = ACC_DEV_NO_BEGIN; dev_no < ACC_DEV_NO_END; dev_no++) {
		dev_info = &di->dev_info[dev_no];
		if (!dev_info->key_info) {
			hwlog_info("acc dev[%d] info is null\n", dev_no);
			continue;
		}
		for (info_no = 0; info_no < dev_info->info_no; info_no++) {
			key_info = dev_info->key_info + info_no;
			if (strlen(acc_info) >= (ACC_DEV_INFO_MAX - ACC_DEV_INFO_LEN))
				goto out;

			snprintf(p_acc_info, ACC_DEV_INFO_LEN,
				"%s=%s,", key_info->name, key_info->value);
			/* info contain '=' and ',' two bytes */
			p_acc_info += strlen(key_info->name) + strlen(key_info->value) + 2;
		}
		/* two info separated by ','; two devices separated by ';' */
		if (dev_no == (ACC_DEV_NO_END - 1))
			p_acc_info[strlen(p_acc_info) - 1] = '\0';
		else
			p_acc_info[strlen(p_acc_info) - 1] = ';';
	}

out:
	return scnprintf(buf, PAGE_SIZE, "%s\n", acc_info);
}

static DEVICE_ATTR(dev, 0440, wireless_acc_dev_info_show, NULL);

static struct attribute *wireless_acc_attributes[] = {
	&dev_attr_dev.attr,
	NULL,
};

static const struct attribute_group wireless_acc_attr_group = {
	.attrs = wireless_acc_attributes,
};

static struct device *wireless_acc_create_group(void)
{
	return power_sysfs_create_group("hw_accessory", "monitor",
		&wireless_acc_attr_group);
}

static void wireless_acc_remove_group(struct device *dev)
{
	power_sysfs_remove_group(dev, &wireless_acc_attr_group);
}

static int wireless_acc_probe(struct platform_device *pdev)
{
	struct wireless_acc_dev *di = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_wireless_acc_dev = di;
	di->dev = wireless_acc_create_group();
	platform_set_drvdata(pdev, di);

	return 0;
}

static int wireless_acc_remove(struct platform_device *pdev)
{
	struct wireless_acc_dev *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	wireless_acc_remove_group(di->dev);
	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, di);
	g_wireless_acc_dev = NULL;

	return 0;
}

static const struct of_device_id wireless_acc_match_table[] = {
	{
		.compatible = "huawei,accessory",
		.data = NULL,
	},
	{},
};

static struct platform_driver wireless_acc_driver = {
	.probe = wireless_acc_probe,
	.remove = wireless_acc_remove,
	.driver = {
		.name = "huawei,accessory",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(wireless_acc_match_table),
	},
};

static int __init wireless_acc_init(void)
{
	return platform_driver_register(&wireless_acc_driver);
}

static void __exit wireless_acc_exit(void)
{
	platform_driver_unregister(&wireless_acc_driver);
}

late_initcall(wireless_acc_init);
module_exit(wireless_acc_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("wireless accessory module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
