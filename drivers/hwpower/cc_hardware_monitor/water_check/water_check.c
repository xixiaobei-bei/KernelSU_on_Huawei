/*
 * water_check.c
 *
 * water check monitor driver
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

#include "water_check.h"
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <chipset_common/hwpower/hardware_monitor/water_detect.h>
#include <chipset_common/hwpower/common_module/power_common.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_wakeup.h>

#define HWLOG_TAG water_check
HWLOG_REGIST();

static struct water_check_info *g_info;

static int usb_gpio_is_water_intruded(void *dev_data)
{
	int i, gpio_value;
	struct water_check_info *info = g_info;

	if (!info) {
		hwlog_info("info is null\n");
		return WATER_NULL;
	}

	for (i = 0; i < info->data.total_type; i++) {
		if (strncmp(info->data.para[i].gpio_name,
			"usb", strlen("usb")))
			continue;

		gpio_value = gpio_get_value(info->data.para[i].gpio_no);
		if (!gpio_value) {
			hwlog_info("water is detected in usb\n");
			return WATER_IN;
		}
	}

	return WATER_NULL;
}

static struct water_detect_ops usb_gpio_water_detect_ops = {
	.type_name = "usb_gpio",
	.is_water_intruded = usb_gpio_is_water_intruded,
};

static void water_check_enable_irq(struct water_check_info *info)
{
	int i;

	for (i = 0; i < info->data.total_type; i++) {
		if (info->data.para[i].type == SINGLE_POSITION)
			enable_irq(info->data.para[i].irq_no);
	}

	__pm_relax(info->wakelock);
}

static void water_check_disable_irq(struct water_check_info *info)
{
	int i;

	__pm_stay_awake(info->wakelock);

	for (i = 0; i < info->data.total_type; i++) {
		if (info->data.para[i].type == SINGLE_POSITION)
			disable_irq_nosync(info->data.para[i].irq_no);
	}
}

static void water_check_process_action(struct water_check_info *info, int i)
{
	switch (info->data.para[i].action) {
	case BATFET_DISABLED_ACTION:
		hwlog_info("charge set batfet to disable\n");
		msleep(50); /* delay 50ms */
		power_platform_set_charge_batfet(true);
		break;
	default:
		break;
	}
}

static void water_check_dmd_report(struct water_check_info *info, int i)
{
	char s_buff[DSM_LOG_SIZE] = { 0 };

	if ((i < 0) || (i > info->data.total_type)) {
		hwlog_err("i is invalid\n");
		return;
	}

	if (info->data.para[i].dmd_no_offset == DSM_IGNORE_NUM)
		return;

	if (info->data.para[i].type == SINGLE_POSITION) {
		memset(s_buff, 0, DSM_LOG_SIZE);
		snprintf(s_buff, DSM_LOG_SIZE, "water check is triggered in: %s",
			info->data.para[i].gpio_name);
		hwlog_info("single_position dsm_buff:%s\n", s_buff);

		/* single position dsm report one time */
		power_dsm_report_dmd(POWER_DSM_BATTERY, ERROR_NO_WATER_CHECK_BASE +
			info->data.para[i].dmd_no_offset, s_buff);

		msleep(150); /* delay 150ms */
		info->dsm_report_status[i] = DSM_REPORTED;
	} else {
		hwlog_info("multiple_intruded dsm_buff:%s\n", info->dsm_buff);
		power_dsm_report_dmd(POWER_DSM_BATTERY, ERROR_NO_WATER_CHECK_BASE +
			info->data.para[i].dmd_no_offset, info->dsm_buff);
	}
}

static void water_check_irq_work(struct work_struct *work)
{
	int i;
	u8 gpio_val;
	int water_intruded_num = 0;
	struct water_check_info *info = NULL;

	hwlog_info("detect_work start\n");

	if (!work)
		goto end_enable_irq;

	info = container_of(work, struct water_check_info, irq_work.work);
	if (!info)
		goto end_enable_irq;

	memset(info->dsm_buff, 0, DSM_LOG_SIZE);
	snprintf(info->dsm_buff, DSM_LOG_SIZE, "water check is triggered in: ");

	for (i = 0; i < info->data.total_type; i++) {
		if (info->data.para[i].type != SINGLE_POSITION)
			continue;

		gpio_val = gpio_get_value_cansleep(info->data.para[i].gpio_no);
		if (!gpio_val) {
			if (info->data.para[i].multiple_handle) {
				water_intruded_num++;
				snprintf(info->dsm_buff + strlen(info->dsm_buff),
					DSM_LOG_SIZE, "%s ", info->data.para[i].gpio_name);
			}

			if ((gpio_val ^ info->last_check_status[i]) ||
				info->dsm_report_status[i]) {
				water_check_dmd_report(info, i);
				water_check_process_action(info, i);
			}
		}

		if (gpio_val)
			info->dsm_report_status[i] = DSM_NEED_REPORT;

		info->last_check_status[i] = gpio_val;
	}

	/* multiple intruded */
	if (water_intruded_num > SINGLE_POSITION) {
		for (i = 0; i < info->data.total_type; i++) {
			if (water_intruded_num == info->data.para[i].type) {
				water_check_dmd_report(info, i);
				water_check_process_action(info, i);
			}
		}
	}

end_enable_irq:
	hwlog_info("detect_work end\n");
	water_check_enable_irq(info);
}

static irqreturn_t water_check_irq_handler(int irq, void *p)
{
	struct water_check_info *info = (struct water_check_info *)p;

	if (!info)
		return IRQ_NONE;

	hwlog_info("irq_handler irq_no:%d\n", irq);
	water_check_disable_irq(info);
	schedule_delayed_work(&info->irq_work, msecs_to_jiffies(DEBOUNCE_TIME));

	return IRQ_HANDLED;
}

static void water_check_print_dts(struct water_check_info *info)
{
	int i;

	for (i = 0; i < info->data.total_type; i++)
		hwlog_info("para[%d] type:%d gpio:%d,%s %d %u %d %u %u\n",
			i,
			info->data.para[i].type,
			info->data.para[i].gpio_no,
			info->data.para[i].gpio_name,
			info->data.para[i].irq_no,
			info->data.para[i].multiple_handle,
			info->data.para[i].dmd_no_offset,
			info->data.para[i].prompt,
			info->data.para[i].action);

	for (i = 0; i < info->data.total_type; i++) {
		if (info->data.para[i].type != SINGLE_POSITION)
			continue;

		hwlog_info("gpio_name:%s irq_no:%d gpio_sts:%u dsm_sts:%u\n",
		info->data.para[i].gpio_name, info->data.para[i].irq_no,
		info->last_check_status[i], info->dsm_report_status[i]);
	}
}

static int water_check_config_gpio(struct water_check_info *info)
{
	int i, ret;
	unsigned long irqflags;

	irqflags = (info->irq_trigger_type ? IRQF_TRIGGER_LOW :
		(IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING)) |
		IRQF_NO_SUSPEND | IRQF_ONESHOT;
	/* request gpio & register interrupt handler function */
	for (i = 0; i < info->data.total_type; i++) {
		if (info->data.para[i].type != SINGLE_POSITION)
			continue;

		ret = devm_gpio_request_one(info->dev,
			(unsigned int)info->data.para[i].gpio_no,
			(unsigned long)GPIOF_DIR_IN, info->data.para[i].gpio_name);
		if (ret < 0) {
			hwlog_err("gpio request fail\n");
			return ret;
		}

		info->data.para[i].irq_no =
			gpio_to_irq(info->data.para[i].gpio_no);
		if (info->data.para[i].irq_no < 0) {
			hwlog_err("gpio map to irq fail\n");
			return -EINVAL;
		}

		info->last_check_status[i] =
			gpio_get_value_cansleep(info->data.para[i].gpio_no);
		info->dsm_report_status[i] = DSM_NEED_REPORT;

		if (info->gpio_type) {
			ret = devm_request_threaded_irq(info->dev,
				(unsigned int)info->data.para[i].irq_no,
				NULL, water_check_irq_handler, irqflags,
				info->data.para[i].gpio_name, info);
		} else {
			ret = devm_request_irq(info->dev,
				(unsigned int)info->data.para[i].irq_no,
				water_check_irq_handler, irqflags,
				info->data.para[i].gpio_name, info);
		}
		if (ret < 0) {
			hwlog_err("gpio irq request fail\n");
			return ret;
		}

		disable_irq_nosync(info->data.para[i].irq_no);
	}
	water_check_print_dts(info);

	return 0;
}

static int water_check_parse_pinctrl(struct device_node *np,
	struct water_check_info *info)
{
	int i, ret, array_len;
	const char *tmp_string = NULL;
	struct pinctrl *pinctrl = NULL;
	struct pinctrl_state *state = NULL;

	array_len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"pinctrl-names", WATER_CHECK_PARA_LEVEL, 1);
	if (array_len < 0)
		return -EINVAL;

	pinctrl = devm_pinctrl_get(info->dev);
	if (IS_ERR(pinctrl)) {
		hwlog_err("cannot find pinctrl\n");
		return PTR_ERR(pinctrl);
	}

	for (i = 0; i < array_len; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG),
			np, "pinctrl-names", i, &tmp_string))
			return -EINVAL;

		state = pinctrl_lookup_state(pinctrl, tmp_string);
		if (IS_ERR(state)) {
			hwlog_err("%s cannot find pinctrl\n", tmp_string);
			return PTR_ERR(state);
		}

		ret = pinctrl_select_state(pinctrl, state);
		if (ret) {
			hwlog_err("%s select state failed\n", tmp_string);
			return ret;
		}
	}

	return ret;
}

static int water_check_parse_extra_dts(struct device_node *np,
	struct water_check_info *info, int i)
{
	int j, ret, gpio_no;
	char gpio_name[GPIO_NAME_SIZE] = { 0 };
	const char *tmp_string = NULL;

	for (j = 0; j < WATER_PARA_TOTAL; j++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG),
			np, "water_check_para", i * WATER_PARA_TOTAL + j, &tmp_string))
			return -EINVAL;

		switch (j) {
		case WATER_TYPE:
			ret = kstrtoint(tmp_string, POWER_BASE_DEC,
				&info->data.para[i].type);
			if (ret)
				return -EINVAL;
			break;
		case WATER_GPIO_NAME:
			strncpy(info->data.para[i].gpio_name,
				tmp_string, GPIO_NAME_SIZE - 1);
			if (info->data.para[i].type != SINGLE_POSITION)
				break;

			memset(gpio_name, 0, GPIO_NAME_SIZE);
			snprintf(gpio_name, sizeof(gpio_name), "gpio_%s",
				info->data.para[i].gpio_name);
			gpio_no = of_get_named_gpio(np, gpio_name, 0);
			info->data.para[i].gpio_no = gpio_no;
			if (!gpio_is_valid(gpio_no)) {
				hwlog_err("gpio is not valid\n");
				return -EINVAL;
			}
			break;
		case WATER_IRQ_NO:
			ret = kstrtoint(tmp_string, POWER_BASE_DEC,
				&info->data.para[i].irq_no);
			if (ret)
				return -EINVAL;
			break;
		case WATER_MULTIPLE_HANDLE:
			ret = kstrtou8(tmp_string, POWER_BASE_DEC,
				&info->data.para[i].multiple_handle);
			if (ret)
				return -EINVAL;
			break;
		case WATER_DMD_NO_OFFSET:
			ret = kstrtoint(tmp_string, POWER_BASE_DEC,
				&info->data.para[i].dmd_no_offset);
			if (ret)
				return -EINVAL;
			break;
		case WATER_IS_PROMPT:
			ret = kstrtou8(tmp_string, POWER_BASE_DEC,
				&info->data.para[i].prompt);
			if (ret)
				return -EINVAL;
			break;
		case WATER_ACTION:
			ret = kstrtou8(tmp_string, POWER_BASE_DEC,
				&info->data.para[i].action);
			if (ret)
				return -EINVAL;
			break;
		default:
			break;
		}
	}

	return 0;
}

static int water_check_parse_dts(struct device_node *np,
	struct water_check_info *info)
{
	int i, ret, array_len;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"gpio_type", &info->gpio_type, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"irq_trigger_type", &info->irq_trigger_type, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"pinctrl_enable", &info->pinctrl_enable, 0);

	/* parse water_check para */
	array_len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"water_check_para", WATER_CHECK_PARA_LEVEL, WATER_PARA_TOTAL);
	if (array_len < 0) {
		info->data.total_type = 0;
		return -EINVAL;
	}

	info->data.total_type = array_len / WATER_PARA_TOTAL;
	hwlog_info("total_type=%d\n", info->data.total_type);

	for (i = 0; i < info->data.total_type; i++) {
		ret = water_check_parse_extra_dts(np, info, i);
		if (ret)
			return -EINVAL;
	}

	if (info->pinctrl_enable) {
		ret = water_check_parse_pinctrl(np, info);
		if (ret)
			return -EINVAL;
	}

	return 0;
}

static int water_check_probe(struct platform_device *pdev)
{
	int ret;
	struct water_check_info *info = NULL;
	struct device_node *np = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	info = devm_kzalloc(&pdev->dev, sizeof(*info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	g_info = info;
	info->dev = &pdev->dev;
	np = pdev->dev.of_node;

	info->wakelock = power_wakeup_source_register(info->dev,
		"watercheck_wakelock");
	if (!info->wakelock) {
		ret = -EINVAL;
		goto fail_free_mem;
	}

	ret = water_check_parse_dts(np, info);
	if (ret < 0)
		goto fail_free_wakelock;

	ret = water_check_config_gpio(info);
	if (ret < 0)
		goto fail_free_wakelock;

	INIT_DELAYED_WORK(&info->irq_work, water_check_irq_work);
	/* delay 8000ms to schedule work when power up first */
	schedule_delayed_work(&info->irq_work, msecs_to_jiffies(DELAY_WORK_TIME));

	platform_set_drvdata(pdev, info);
	water_detect_ops_register(&usb_gpio_water_detect_ops);

	return 0;

fail_free_wakelock:
	power_wakeup_source_unregister(info->wakelock);
fail_free_mem:
	devm_kfree(&pdev->dev, info);
	g_info = NULL;

	return ret;
}

static int water_check_remove(struct platform_device *pdev)
{
	struct water_check_info *info = dev_get_drvdata(&pdev->dev);

	if (!info)
		return -ENODEV;

	power_wakeup_source_unregister(info->wakelock);
	cancel_delayed_work(&info->irq_work);
	devm_kfree(&pdev->dev, info);
	platform_set_drvdata(pdev, NULL);
	g_info = NULL;

	return 0;
}

static const struct of_device_id water_check_of_match[] = {
	{
		.compatible = "huawei,water_check",
	},
	{},
};

static struct platform_driver water_check_driver = {
	.probe = water_check_probe,
	.remove = water_check_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = "water_check",
		.of_match_table = water_check_of_match,
	},
};

static int __init water_check_init(void)
{
	return platform_driver_register(&water_check_driver);
}

static void __exit water_check_exit(void)
{
	platform_driver_unregister(&water_check_driver);
}

module_init(water_check_init);
module_exit(water_check_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("water check module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
