/*
 * multi_btb_temp.c
 *
 * multi btb temperature monitor and mixed driver
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
#include <linux/err.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/thermal.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/device.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/battery/battery_temp.h>
#include <chipset_common/hwpower/common_module/power_common.h>
#include <chipset_common/hwpower/common_module/power_temp.h>
#include <chipset_common/hwpower/common_module/power_algorithm.h>
#include <chipset_common/hwpower/common_module/power_log.h>
#include <chipset_common/hwpower/common_module/power_debug.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG btb_temp
HWLOG_REGIST();

#define INVALID_TEMP                     80000 /* 80 degree */
#define READ_TEMPERATURE_MS              5000 /* 5 sec */

/*
 * define temp compensation value of different ibat
 * support up to 8 parameters list on dts
 */
#define SENSOR_MAX_NUM                   8
#define COMP_PARA_LEVEL                  16
#define COMP_PARA_NODE_LEN_MAX           32

enum comp_para_info {
	COMP_PARA_ICHG = 0,
	COMP_PARA_TEMP,
	COMP_PARA_TOTAL,
};

enum sensor_info {
	SENSOR_INFO_SENSOR_INDEX = 0,
	SENSOR_INFO_COMP_PARA_ID,
	SENSOR_INFO_TOTAL,
};

struct sensor_para {
	char name[THERMAL_NAME_LENGTH];
	char comp_para_index[COMP_PARA_NODE_LEN_MAX];
	struct compensation_para comp_data[COMP_PARA_LEVEL];
	int comp_para_size;
	int temp;
	int raw;
};

struct multi_btb_temp_info {
	struct device *dev;
	int ntc_compensation_is;
	struct sensor_para sensor_tab[SENSOR_MAX_NUM];
	int sensor_num;
	int max_tbat_gap;
	struct delayed_work temp_work;
};

static struct multi_btb_temp_info *g_info;

static int multi_btb_get_temp_with_comp(int index, int temp_without_comp)
{
	struct multi_btb_temp_info *di = g_info;
	struct common_comp_data data = { 0 };
	struct smooth_comp_data smooth_data = { 0 };
	int temp_with_comp;
	int refer;

	if (!di) {
		hwlog_err("di is null\n");
		return temp_without_comp;
	}

	/*
	 * fix v600 hardware issue:
	 * temp is 125 degree when adaptor disconnect
	 */
	if (!di->ntc_compensation_is || (temp_without_comp >= INVALID_TEMP))
		return temp_without_comp;

	refer = power_platform_get_battery_current();
	data.refer = abs(refer);
	data.para = di->sensor_tab[index].comp_data;
	data.para_size = di->sensor_tab[index].comp_para_size;

	temp_with_comp = power_get_compensation_value(temp_without_comp, &data);
	if (di->sensor_tab[index].raw < INVALID_TEMP) {
		smooth_data.current_raw = temp_without_comp;
		smooth_data.current_comp = temp_with_comp;
		smooth_data.last_raw = di->sensor_tab[index].raw;
		smooth_data.last_comp = di->sensor_tab[index].temp;
		smooth_data.max_delta = di->max_tbat_gap;
		temp_with_comp = power_get_smooth_compensation_value(&smooth_data);
	}

	hwlog_info("sensor:%d,raw:%d,temp:%d\n", index, temp_without_comp, temp_with_comp);
	return temp_with_comp;
}

static int multi_btb_get_temp_sync(const int index, const char *sensor_name)
{
	int raw;

	raw = power_temp_get_average_value(sensor_name);
	return multi_btb_get_temp_with_comp(index, raw);
}

static void multi_btb_update_temp(struct multi_btb_temp_info *di)
{
	int i, raw, temp;

	for (i = 0; i < di->sensor_num; i++) {
		raw = power_temp_get_average_value(di->sensor_tab[i].name);
		temp = multi_btb_get_temp_with_comp(i, raw);
		hwlog_info("update temp%d:%s,raw:%d,temp:%d\n", i,
			di->sensor_tab[i].name, raw, temp);
		di->sensor_tab[i].raw = raw;
		di->sensor_tab[i].temp = temp;
	}
}

static void multi_btb_init_temp(struct multi_btb_temp_info *di)
{
	int i;

	for (i = 0; i < di->sensor_num; i++) {
		di->sensor_tab[i].raw = power_temp_get_average_value(di->sensor_tab[i].name);
		di->sensor_tab[i].temp = di->sensor_tab[i].raw;
		hwlog_info("init temp%d:%s,raw:%d\n", i,
			di->sensor_tab[i].name, di->sensor_tab[i].raw);
	}
}

static void multi_btb_temp_work(struct work_struct *work)
{
	struct multi_btb_temp_info *di = container_of(work,
		struct multi_btb_temp_info, temp_work.work);

	multi_btb_update_temp(di);
	schedule_delayed_work(&di->temp_work,
		msecs_to_jiffies(READ_TEMPERATURE_MS));
}

static int multi_btb_temp(enum bat_temp_id id, int *temp)
{
	int batt_temp;
	struct multi_btb_temp_info *di = g_info;

	if (!temp) {
		hwlog_err("temp is null\n");
		return -1;
	}

	if (!di) {
		*temp = power_platform_get_battery_temperature();
		hwlog_err("g_di is null, batt_temp = [%d]\n", *temp);
		return 0;
	}

	if (di->sensor_num <= 0) {
		*temp = power_platform_get_battery_temperature();
		hwlog_err("sensor_num is 0, batt_temp = [%d]\n", *temp);
		return 0;
	}

	if ((int)id >= di->sensor_num)
		id = BTB_TEMP_0;

	batt_temp = di->sensor_tab[id].temp;
	hwlog_info("sensor%d temp:%d\n", id, batt_temp);
	*temp = batt_temp / THOUSAND_UNIT;
	return 0;
}

static int multi_btb_temp_sync(enum bat_temp_id id, int *temp)
{
	struct multi_btb_temp_info *di = g_info;
	int batt_temp;

	if (!temp) {
		hwlog_err("temp is null\n");
		return -1;
	}

	if (!di) {
		*temp = power_platform_get_rt_battery_temperature();
		hwlog_err("g_di is null, batt_temp is [%d]\n", *temp);
		return 0;
	}

	if ((di->sensor_num <= 0) || ((int)id >= di->sensor_num))
		id = BTB_TEMP_0;

	batt_temp = multi_btb_get_temp_sync(id, di->sensor_tab[id].name);

	hwlog_info("temp_sync%d temp:%d\n", id, batt_temp);
	*temp = batt_temp / THOUSAND_UNIT;
	return 0;
}

static int multi_btb_get_register_head(char *buffer, int size, void *dev_data)
{
	if (!buffer)
		return -1;

	snprintf(buffer, size, "btb_temp0 btb_temp1");
	return 0;
}

static int multi_btb_value_dump(char *buffer, int size, void *dev_data)
{
	int btb_temp0 = 0;
	int btb_temp1 = 0;

	if (!buffer)
		return -1;

	multi_btb_temp_sync(BTB_TEMP_0, &btb_temp0);
	multi_btb_temp_sync(BTB_TEMP_1, &btb_temp1);

	snprintf(buffer, size, "%-9d %-9d", btb_temp0, btb_temp1);
	return 0;
}

static struct bat_temp_ops multi_temp_ops = {
	.get_rt_temp = multi_btb_temp_sync,
	.get_temp = multi_btb_temp,
};

static struct power_log_ops multi_btb_log_ops = {
	.dev_name = "multi_btb",
	.dump_log_head = multi_btb_get_register_head,
	.dump_log_content = multi_btb_value_dump,
};

static ssize_t multi_btb_dbg_para_store(void *dev_data,
	const char *buf, size_t size)
{
	int max_tbat_gap = 0;
	int ret;
	struct multi_btb_temp_info *di = dev_data;

	if (!di)
		return -EINVAL;

	ret = kstrtoint(buf, 0, &max_tbat_gap);
	if (ret) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	di->max_tbat_gap = max_tbat_gap;
	hwlog_info("set max_tbat_gap:%d\n", max_tbat_gap);

	return size;
}

static ssize_t multi_btb_dbg_para_show(void *dev_data,
	char *buf, size_t size)
{
	struct multi_btb_temp_info *di = dev_data;

	if (!di)
		return scnprintf(buf, size, "not support\n");

	return snprintf(buf, PAGE_SIZE, "%d\n", di->max_tbat_gap);
}

static int multi_btb_temp_parse_sensor_tab(struct device_node *np,
	struct multi_btb_temp_info *di)
{
	int array_len, i, row, col;
	const char *str = NULL;

	array_len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"sensor-names", SENSOR_MAX_NUM, SENSOR_INFO_TOTAL);
	if (array_len <= 0) {
		hwlog_err("sensor_table read fail\n");
		goto err_out;
	}

	di->sensor_num = array_len / SENSOR_INFO_TOTAL;
	for (i = 0; i < array_len; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG),
			np, "sensor-names", i, &str))
			goto err_out;

		row = i / SENSOR_INFO_TOTAL;
		col = i % SENSOR_INFO_TOTAL;
		switch (col) {
		case SENSOR_INFO_SENSOR_INDEX:
			if (strlen(str) >= THERMAL_NAME_LENGTH) {
				hwlog_err("invalid sensor name\n");
				goto err_out;
			}
			strncpy(di->sensor_tab[row].name,
				str, THERMAL_NAME_LENGTH - 1);
			break;
		case SENSOR_INFO_COMP_PARA_ID:
			strncpy(di->sensor_tab[row].comp_para_index,
				str, COMP_PARA_NODE_LEN_MAX - 1);
			break;
		default:
			break;
		}
	}

	for (i = 0; i < di->sensor_num; i++)
		hwlog_info("sensor_tab[%d] %s %s\n", i, di->sensor_tab[i].name,
			di->sensor_tab[i].comp_para_index);

	return 0;

err_out:
	di->sensor_num = 0;
	return -1;
}

static int multi_btb_temp_parse_comp_para(struct device_node *np,
	struct multi_btb_temp_info *di, const int group)
{
	int array_len, col, row;
	int idata[COMP_PARA_LEVEL * COMP_PARA_TOTAL] = { 0 };
	char *str = NULL;

	di->sensor_tab[group].comp_para_size = 0;
	str = di->sensor_tab[group].comp_para_index;
	array_len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		str, idata, COMP_PARA_LEVEL, COMP_PARA_TOTAL);
	if (array_len < 0) {
		di->ntc_compensation_is = 0;
		return -1;
	}

	di->sensor_tab[group].comp_para_size = array_len / COMP_PARA_TOTAL;
	for (row = 0; row < array_len / COMP_PARA_TOTAL; row++) {
		col = row * COMP_PARA_TOTAL + COMP_PARA_ICHG;
		di->sensor_tab[group].comp_data[row].refer = idata[col];
		col = row * COMP_PARA_TOTAL + COMP_PARA_TEMP;
		di->sensor_tab[group].comp_data[row].comp_value = idata[col];
		hwlog_info("temp_comp_para[%d]=%d %d\n", row,
			di->sensor_tab[group].comp_data[row].refer,
			di->sensor_tab[group].comp_data[row].comp_value);
	}

	return 0;
}

static void multi_btb_temp_parse_group_comp_para(
	struct device_node *np, struct multi_btb_temp_info *di)
{
	int i;

	if (!di || !di->sensor_num || !di->ntc_compensation_is)
		return;

	for (i = 0; i < di->sensor_num; i++)
		if (multi_btb_temp_parse_comp_para(np, di, i))
			return;
}

static int multi_btb_temp_parse_dts(struct device_node *np,
	struct multi_btb_temp_info *di)
{
	power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ntc_compensation_is", &di->ntc_compensation_is, 0);

	power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"max_tbat_gap", &di->max_tbat_gap, 5000); /* 5 degrees */

	if (multi_btb_temp_parse_sensor_tab(np, di))
		return -1;

	multi_btb_temp_parse_group_comp_para(np, di);
	return 0;
}

static int multi_btb_temp_probe(struct platform_device *pdev)
{
	struct multi_btb_temp_info *di = NULL;
	struct device_node *np = NULL;
	int ret;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	np = pdev->dev.of_node;
	ret = multi_btb_temp_parse_dts(np, di);
	if (ret)
		goto err_out;

	platform_set_drvdata(pdev, di);
	g_info = di;

	multi_btb_init_temp(di);

	ret = bat_temp_ops_register("multi_btb_temp", &multi_temp_ops);
	if (ret)
		goto err_out;
	multi_btb_log_ops.dev_data = (void *)di;
	power_log_ops_register(&multi_btb_log_ops);

	power_dbg_ops_register("max_delta", (void *)di,
		(power_dbg_show)multi_btb_dbg_para_show,
		(power_dbg_store)multi_btb_dbg_para_store);

	INIT_DELAYED_WORK(&di->temp_work, multi_btb_temp_work);
	schedule_delayed_work(&di->temp_work,
		msecs_to_jiffies(READ_TEMPERATURE_MS));

	return 0;
err_out:
	kfree(di->sensor_tab);
	devm_kfree(&pdev->dev, di);
	g_info = NULL;
	return ret;
}

static int multi_btb_temp_remove(struct platform_device *pdev)
{
	struct multi_btb_temp_info *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	platform_set_drvdata(pdev, NULL);
	kfree(di->sensor_tab);
	devm_kfree(&pdev->dev, di);
	g_info = NULL;

	return 0;
}

#ifdef CONFIG_PM
static int multi_btb_temp_resume(struct platform_device *pdev)
{
	struct multi_btb_temp_info *di = platform_get_drvdata(pdev);

	if (!di) {
		hwlog_err("di is null\n");
		return 0;
	}

	multi_btb_init_temp(di);
	schedule_delayed_work(&di->temp_work,
		msecs_to_jiffies(READ_TEMPERATURE_MS));
	return 0;
}

static int multi_btb_temp_suspend(struct platform_device *pdev,
	pm_message_t state)
{
	struct multi_btb_temp_info *di = platform_get_drvdata(pdev);

	if (!di) {
		hwlog_err("di is null\n");
		return 0;
	}

	cancel_delayed_work_sync(&di->temp_work);
	return 0;
}
#endif /* CONFIG_PM */

static const struct of_device_id btb_temp_match_table[] = {
	{
		.compatible = "huawei,multi_btb_temp",
		.data = NULL,
	},
	{},
};

static struct platform_driver multi_btb_temp_driver = {
	.probe = multi_btb_temp_probe,
	.remove = multi_btb_temp_remove,
#ifdef CONFIG_PM
	.resume = multi_btb_temp_resume,
	.suspend = multi_btb_temp_suspend,
#endif /* CONFIG_PM */
	.driver = {
		.name = "huawei,multi_btb_temp",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(btb_temp_match_table),
	},
};

static int __init multi_btb_temp_init(void)
{
	return platform_driver_register(&multi_btb_temp_driver);
}

static void __exit multi_btb_temp_exit(void)
{
	platform_driver_unregister(&multi_btb_temp_driver);
}

device_initcall_sync(multi_btb_temp_init);
module_exit(multi_btb_temp_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei multi btb temp module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
