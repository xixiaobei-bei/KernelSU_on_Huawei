/*
 * multi_bat_temp.c
 *
 * multi battery temperature monitor and mixed driver
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
#include <linux/hisi/hisi_adc.h>
#include <huawei_platform/power/huawei_charger.h>
#include <huawei_platform/power/battery_voltage.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/battery/battery_temp.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG multi_batt_temp
HWLOG_REGIST();

#define UTEMP_PER_MTEMP                      1000
#define TEMP_LOW                             15000
#define TEMP_HIGH                            40000
#define COMP_PARA_ARRAYSIZE                  6
#define READ_TEMPERATURE_MS                  5000 /* 5 sec */
#define TEMPERATURE_CHANGE_LIMIT             5000 /* x degrees */
#define HIGH_LEVEL                           1
#define SENSOR_NAME_BATT_ID_0                POWER_TEMP_SERIAL_BAT_0

#define DEFAULT_SENSOR_NAME           "default"
#define DEV_NAME                      "multi_battery_temp"

enum batt_temp_comp_para_info {
	BATT_TEMP_COMP_PARA_ICHG = 0,
	BATT_TEMP_COMP_PARA_VALUE,
	BATT_TEMP_COMP_PARA_TOTAL,
};

struct batt_temp_comp_para_data {
	int batt_temp_comp_ichg;
	int batt_temp_comp_value;
};

struct sensor_info {
	char name[THERMAL_NAME_LENGTH];
	int temp;
	int raw;
};

struct multi_batt_temp_info {
	struct device *dev;
	struct batt_temp_comp_para_data comp_para[COMP_PARA_ARRAYSIZE];
	struct delayed_work temp_work;
	struct thermal_zone_device *batt0_thermal;
	int ntc_compensation_is;
	int batt_temp_low;
	int batt_temp_high;
	int gpio_ntc_switch;
	bool is_init;
	int temp_mixed;
	int sensor_num;
	struct sensor_info *sensor_tab;
};

static struct multi_batt_temp_info *g_info;
static const char *g_default_sensor_name;

static int get_batt_temp_with_comp(int temp_without_compensation)
{
	struct multi_batt_temp_info *di = g_info;
	int temp_with_compensation = temp_without_compensation;
	int ichg;
	int i;

	if (!di) {
		hwlog_err("di is null\n");
		return temp_without_compensation;
	}

	if (!di->ntc_compensation_is) {
		hwlog_err("temp_without_compensation is 0\n");
		return temp_without_compensation;
	}

	ichg = power_platform_get_battery_current();
	ichg = abs(ichg);
	for (i = 0; i < COMP_PARA_ARRAYSIZE; i++) {
		if (ichg >= di->comp_para[i].batt_temp_comp_ichg) {
			temp_with_compensation = temp_without_compensation -
				di->comp_para[i].batt_temp_comp_value;
			break;
		}
	}

	hwlog_info("ichg is %dma, old temp is %d, temp with comp is %d\n",
		ichg, temp_without_compensation, temp_with_compensation);
	return temp_with_compensation;
}

static int get_batt_mixed_temp(int bat0_temp, int bat1_temp)
{
	int temp_mixed;
	int high = g_info->batt_temp_high;
	int low = g_info->batt_temp_low;
	int tbat_high = bat0_temp > bat1_temp ? bat0_temp : bat1_temp;
	int tbat_low = bat0_temp > bat1_temp ? bat1_temp : bat0_temp;

	hwlog_info("bat0_temp is %d, bat1_temp is %d\n",
		bat0_temp, bat1_temp);

	if ((tbat_low < low) && (tbat_high > high))
		temp_mixed = ((low - tbat_low) > (tbat_high - high)) ? tbat_low : tbat_high;
	else if ((tbat_low > low) && (tbat_high < high))
		temp_mixed = (tbat_high + tbat_low) / 2;
	else if (tbat_low > low)
		temp_mixed = tbat_high;
	else
		temp_mixed = tbat_low;

	hwlog_info("mixed batt temp is %d\n", temp_mixed);
	return temp_mixed;
}

static int calculate_mixed_temp(struct multi_batt_temp_info *di)
{
	int i;
	int mixed;

	if (di->sensor_num <= 0)
		return power_platform_get_battery_temperature();

	mixed = di->sensor_tab[0].temp;
	/* i start with 1, mixed tab[0], tab[1] ...... */
	for (i = 1; i < di->sensor_num; i++)
		mixed = get_batt_mixed_temp(mixed, di->sensor_tab[i].temp);

	return mixed;
}

static int smooth_batt_temp(int temp_curr, int temp_pre)
{
	hwlog_info("temp_pre:%d, temp_curr:%d\n", temp_pre, temp_curr);

	if (temp_curr - temp_pre > TEMPERATURE_CHANGE_LIMIT)
		temp_curr = temp_pre + TEMPERATURE_CHANGE_LIMIT;
	else if (temp_pre - temp_curr > TEMPERATURE_CHANGE_LIMIT)
		temp_curr = temp_pre - TEMPERATURE_CHANGE_LIMIT;

	return temp_curr;
}

static int get_batt_temp_sync(const char *sensor_name)
{
	int temperature;
	int batt_temp;

	batt_temp = power_temp_get_average_value(sensor_name);
	temperature = get_batt_temp_with_comp(batt_temp);

	return temperature;
}

static int get_mixed_temp_sync(struct multi_batt_temp_info *di)
{
	int i;
	int mixed, temp;

	if (di->sensor_num <= 0)
		return power_platform_get_rt_battery_temperature();

	mixed = get_batt_temp_sync(di->sensor_tab[0].name);
	/* i start with 1, mixed tab[0], tab[1] ...... */
	for (i = 1; i < di->sensor_num; i++) {
		temp = get_batt_temp_sync(di->sensor_tab[i].name);
		mixed = get_batt_mixed_temp(mixed, temp);
	}
	return mixed;
}

static bool init_battery_temp(struct multi_batt_temp_info *di)
{
	int i;
	int raw;
	int temp;

	if (di->gpio_ntc_switch)
		gpio_set_value(di->gpio_ntc_switch, HIGH_LEVEL);

	for (i = 0; i < di->sensor_num; i++) {
		raw = power_temp_get_average_value(di->sensor_tab[i].name);
		if (raw == POWER_TEMP_INVALID_TEMP) {
			hwlog_err("init temp%d:%s,ntc:%d fail\n",
				i, di->sensor_tab[i].name, raw);
			return false;
		}
		temp = get_batt_temp_with_comp(raw);

		di->sensor_tab[i].raw = raw;
		di->sensor_tab[i].temp = temp;
		hwlog_info("init temp i:%d,ntc:%d,temp:%d\n", i, raw, temp);
	}
	di->temp_mixed = calculate_mixed_temp(di);

	hwlog_info("init temp_mixed:%d\n", di->temp_mixed);
	return true;
}

static void update_one_batt_temp(struct multi_batt_temp_info *di, int index)
{
	s64 delta_comp, delta_raw;
	int temp, raw;
	int i = index;

	if (i >= di->sensor_num || di->sensor_num <= 0)
		return;

	/* temperature debounce logic */
	raw = power_temp_get_average_value(di->sensor_tab[i].name);
	temp = get_batt_temp_with_comp(raw);
	hwlog_info("temp%d:%s,ntc:%d,temp:%d\n", i, di->sensor_tab[i].name,
		raw, temp);
	delta_comp = temp - di->sensor_tab[i].temp;
	delta_raw = raw - di->sensor_tab[i].raw;
	if (delta_comp * delta_raw < 0) {
		if (delta_raw < 0)
			temp = di->sensor_tab[i].temp;
	} else {
		if (abs(delta_comp) > abs(delta_raw))
			temp = di->sensor_tab[i].temp + (int)delta_raw;
	}
	di->sensor_tab[i].temp = smooth_batt_temp(temp,
		di->sensor_tab[i].temp);
	di->sensor_tab[i].raw = raw;
}

static void update_battery_temp(struct multi_batt_temp_info *di)
{
	int temp_mixed;
	int i;

	if (!di->is_init) {
		if (init_battery_temp(di))
			di->is_init = true;

		return;
	}

	for (i = 0; i < di->sensor_num; i++)
		update_one_batt_temp(di, i);

	temp_mixed = calculate_mixed_temp(di);
	di->temp_mixed = smooth_batt_temp(temp_mixed,
		di->temp_mixed);
}

static void read_temperature_work(struct work_struct *work)
{
	struct multi_batt_temp_info *di = container_of(work,
		struct multi_batt_temp_info, temp_work.work);

	update_battery_temp(di);
	schedule_delayed_work(&di->temp_work,
		msecs_to_jiffies(READ_TEMPERATURE_MS));
}

static int multi_battery_temp(enum bat_temp_id id, int *temp)
{
	int batt_temp;
	struct multi_batt_temp_info *di = g_info;

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

	if (!di->is_init) {
		if (init_battery_temp(di))
			di->is_init = true;
	}

	if (id == BAT_TEMP_MIXED) {
		batt_temp = di->temp_mixed;
		*temp = batt_temp / UTEMP_PER_MTEMP;
		hwlog_info("sensor%d:temp:%d\n", id, batt_temp);
		return 0;
	}

	if ((int)id >= di->sensor_num) {
		batt_temp = di->sensor_tab[0].temp;
		hwlog_err("invalid bat_temp_id: %d, %d\n", id, batt_temp);
	} else {
		batt_temp = di->sensor_tab[id].temp;
	}

	hwlog_info("sensor%d:temp:%d\n", id, batt_temp);
	*temp = batt_temp / UTEMP_PER_MTEMP;
	return 0;
}

static int multi_battery_temp_sync(enum bat_temp_id id, int *temp)
{
	struct multi_batt_temp_info *di = g_info;
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

	if (di->sensor_num <= 0) {
		*temp = power_platform_get_rt_battery_temperature();
		hwlog_err("sensor_num is 0, batt_temp = [%d]\n", *temp);
		return 0;
	}

	if (id == BAT_TEMP_MIXED) {
		batt_temp = get_mixed_temp_sync(di);
		*temp = batt_temp / UTEMP_PER_MTEMP;
		hwlog_info("temp_sync%d:temp:%d\n", id, batt_temp);
		return 0;
	}

	if ((int)id >= di->sensor_num) {
		batt_temp = get_batt_temp_sync(di->sensor_tab[0].name);
		hwlog_err("invalid bat_temp_id sync: %d, %d\n",
			id, batt_temp);
	} else {
		batt_temp = get_batt_temp_sync(di->sensor_tab[id].name);
	}

	hwlog_info("temp_sync%d:temp:%d\n", id, batt_temp);
	*temp = batt_temp / UTEMP_PER_MTEMP;
	return 0;
}

static int get_batt0_temp(struct thermal_zone_device *thermal, int *temp)
{
	if (!thermal || !temp || !g_default_sensor_name)
		return -EINVAL;

	*temp = power_temp_get_average_value(g_default_sensor_name);
	return 0;
}

static struct thermal_zone_device_ops tz_dev_ops = {
	.get_temp = get_batt0_temp,
};

static struct bat_temp_ops multi_temp_ops = {
	.get_rt_temp = multi_battery_temp_sync,
	.get_temp = multi_battery_temp,
};

static void batt_temp_gpio_init(struct multi_batt_temp_info *di,
	struct device_node *np)
{
	if (power_gpio_config_output(np,
		"gpio_ntc_switch", "gpio_ntc_switch", &di->gpio_ntc_switch, 0))
		di->gpio_ntc_switch = 0;
}

static int rework_batt0_sensor(struct multi_batt_temp_info *di,
	struct sensor_info *ptr, const char *str)
{
	/* sensor0 name is special, do not need rework */
	if (strncmp(str, DEFAULT_SENSOR_NAME, strlen(DEFAULT_SENSOR_NAME))) {
		strlcpy(ptr[0].name, str, sizeof(ptr[0].name));
		return 0;
	}

	hwlog_info("sensor0 need rename\n");
	/* sensor0 need rename and register default thermal */
	di->batt0_thermal = thermal_zone_device_register(
		SENSOR_NAME_BATT_ID_0, 0, 0, NULL,
		&tz_dev_ops, NULL, 0, 0);
	if (IS_ERR(di->batt0_thermal)) {
		hwlog_err("thermal zone register fail\n");
		return -ENODEV;
	}
	strlcpy(ptr[0].name, SENSOR_NAME_BATT_ID_0, sizeof(ptr[0].name));
	return 0;
}

static void batt_temp_parse_sensor_tab(struct device_node *np,
	struct multi_batt_temp_info *di)
{
	int array_len, i;
	struct sensor_info *ptr = NULL;
	const char *str = NULL;

	array_len = of_property_count_strings(np, "sensor-names");
	if (array_len <= 0) {
		hwlog_err("sensor_table read fail\n");
		return;
	}

	ptr = kzalloc(sizeof(*ptr) * array_len, GFP_KERNEL);
	if (!ptr)
		return;
	for (i = 0; i < array_len; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG),
			np, "sensor-names", i, &str))
			goto err_out;
		if (strlen(str) >= THERMAL_NAME_LENGTH) {
			hwlog_err("invalid sensor name\n");
			goto err_out;
		}

		hwlog_err("sensor_name %d %s\n", i, str);
		if (i == 0) {
			if (rework_batt0_sensor(di, ptr, str))
				goto err_out;
		} else {
			strlcpy(ptr[i].name, str, sizeof(ptr[i].name));
		}
	}
	di->sensor_num = array_len;
	di->sensor_tab = ptr;
	return;
err_out:
	di->sensor_num = 0;
	kfree(ptr);
}

static void batt_temp_parse_comp_tab(struct device_node *np,
	struct multi_batt_temp_info *di)
{
	const char *tmp_string = NULL;
	int array_len;
	int i;
	int idata = 0;
	int col;
	int row;

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ntc_compensation_is", &di->ntc_compensation_is, 0))
		return;

	array_len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"ntc_temp_compensation_para", COMP_PARA_ARRAYSIZE,
		BATT_TEMP_COMP_PARA_TOTAL);
	if (array_len < 0) {
		di->ntc_compensation_is = 0;
		return;
	}

	for (i = 0; i < array_len; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG),
			np, "ntc_temp_compensation_para", i, &tmp_string)) {
			di->ntc_compensation_is = 0;
			return;
		}

		if (kstrtoint(tmp_string, POWER_BASE_DEC, &idata)) {
			di->ntc_compensation_is = 0;
			return;
		}

		col = i % BATT_TEMP_COMP_PARA_TOTAL;
		row = i / BATT_TEMP_COMP_PARA_TOTAL;

		switch (col) {
		case BATT_TEMP_COMP_PARA_ICHG:
			di->comp_para[row].batt_temp_comp_ichg = idata;
			break;
		case BATT_TEMP_COMP_PARA_VALUE:
			di->comp_para[row].batt_temp_comp_value = idata;
			break;
		default:
			hwlog_err("ntc_temp_compensation_para get failed\n");
			break;
		}

		hwlog_info("di->comp_para[%d][%d] is %d\n", row, col, idata);
	}
}

static void batt_temp_parse_dts(struct device_node *np,
	struct multi_batt_temp_info *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"battery_temp_high", &di->batt_temp_high, TEMP_HIGH);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"battery_temp_low", &di->batt_temp_low, TEMP_LOW);
	(void)power_dts_read_string(power_dts_tag(HWLOG_TAG), np,
		"default_sensor_name", &g_default_sensor_name);

	batt_temp_parse_comp_tab(np, di);
	batt_temp_parse_sensor_tab(np, di);
}

static int batt_temp_probe(struct platform_device *pdev)
{
	struct multi_batt_temp_info *di = NULL;
	struct device_node *np = NULL;
	int ret;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	np = pdev->dev.of_node;
	batt_temp_parse_dts(np, di);
	if (di->sensor_num <= 0)
		return -EINVAL;
	platform_set_drvdata(pdev, di);
	g_info = di;
	batt_temp_gpio_init(di, np);
	INIT_DELAYED_WORK(&di->temp_work, read_temperature_work);
	di->is_init = false;
	ret = bat_temp_ops_register(DEV_NAME, &multi_temp_ops);
	if (ret)
		goto err_out;
	schedule_delayed_work(&di->temp_work,
		msecs_to_jiffies(READ_TEMPERATURE_MS));

	return 0;
err_out:
	if (di->gpio_ntc_switch)
		gpio_free(di->gpio_ntc_switch);
	kfree(di->sensor_tab);
	g_info = NULL;
	return ret;
}

static int batt_temp_remove(struct platform_device *pdev)
{
	struct multi_batt_temp_info *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	if (di->gpio_ntc_switch)
		gpio_free(di->gpio_ntc_switch);

	platform_set_drvdata(pdev, NULL);
	kfree(di->sensor_tab);
	devm_kfree(&pdev->dev, di);
	g_info = NULL;

	return 0;
}

#ifdef CONFIG_PM
static int batt_temp_resume(struct platform_device *pdev)
{
	struct multi_batt_temp_info *di = NULL;

	di = platform_get_drvdata(pdev);
	if (!di) {
		hwlog_err("di is null\n");
		return 0;
	}

	init_battery_temp(di);
	schedule_delayed_work(&di->temp_work,
		msecs_to_jiffies(READ_TEMPERATURE_MS));
	return 0;
}

static int batt_temp_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct multi_batt_temp_info *di = platform_get_drvdata(pdev);

	if (!di) {
		hwlog_err("di is null\n");
		return 0;
	}

	cancel_delayed_work_sync(&di->temp_work);
	return 0;
}
#endif /* CONFIG_PM */

static const struct of_device_id batt_temp_match_table[] = {
	{
		.compatible = "huawei,multi_battery_temp",
		.data = NULL,
	},
	{},
};

static struct platform_driver multi_batt_temp_driver = {
	.probe = batt_temp_probe,
	.remove = batt_temp_remove,
#ifdef CONFIG_PM
	.resume = batt_temp_resume,
	.suspend = batt_temp_suspend,
#endif /* CONFIG_PM */
	.driver = {
		.name = "huawei,multi_battery_temp",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(batt_temp_match_table),
	},
};

int __init multi_batt_temp_init(void)
{
	return platform_driver_register(&multi_batt_temp_driver);
}

void __exit multi_batt_temp_exit(void)
{
	platform_driver_unregister(&multi_batt_temp_driver);
}

fs_initcall_sync(multi_batt_temp_init);
module_exit(multi_batt_temp_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei multi battery temp module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
