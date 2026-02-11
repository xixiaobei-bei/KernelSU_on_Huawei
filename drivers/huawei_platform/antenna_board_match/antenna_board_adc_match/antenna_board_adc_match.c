/*
 * antenna_board_adc_match.c
 *
 * Antenna board match driver,match the antenna board by adc.
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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

#include "antenna_board_adc_match.h"
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/hisi/hisi_adc.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/antenna_board_match/antenna_board_match.h>

/*
 * 2: adc match range array have two member, min and max.
 * 820: is min default range provided by hardware.
 * 975: is max default range provided by hardware.
 */
#define ADC_MATCH_RANGE    2
#define ADC_MATCH_RANGE_MIN    820
#define ADC_MATCH_RANGE_MAX    975
#define HWLOG_TAG antenna_board_adc_match
HWLOG_REGIST();

static int board_adc_channel = DEFAULT_ANTENNA_BOARD_ADC_CHANNEL;

static int antenna_adc_match_range[ADC_MATCH_RANGE] = {
	ADC_MATCH_RANGE_MIN,
	ADC_MATCH_RANGE_MAX,
};

int check_match_by_adc(void)
{
	int ret;
	int rf_voltage;

	rf_voltage = hisi_adc_get_value(board_adc_channel);
	hwlog_info("Antenna board adc voltage = %d\n", rf_voltage);

	if (rf_voltage >= antenna_adc_match_range[0] &&
		rf_voltage <= antenna_adc_match_range[1]) {
		ret = 1;
	} else {
		hwlog_err("adc voltage isn't in range, Antenna_board_match error!\n");
		ret = 0;
	}

	return ret;
}

int get_voltage_by_adc(void)
{
	int voltage;

	voltage = hisi_adc_get_value(board_adc_channel);
	hwlog_info("Antenna board adc voltage = %d\n", voltage);

	return voltage;
}

/* lint -save -e* */
static void parse_dts(struct antenna_adc_match_info *di)
{
	struct device_node *np = NULL;

	np = di->dev->of_node;
	if (np == NULL) {
		hwlog_err("%s np is null!\n", __func__);
		return;
	}

	/* adc channel */
	if (of_property_read_u32(np, "antenna_board_adc_channel",
		&board_adc_channel))
		hwlog_err("dts: can't get adc channel, use default channel: %d!\n",
			board_adc_channel);
	hwlog_info("get antenna board adc channel = %d!\n", board_adc_channel);

	if (of_property_read_u32_array(np, "antenna_board_match_range",
		antenna_adc_match_range, ADC_MATCH_RANGE))
		hwlog_err("%s: match range not exist, use default array!\n",
			__func__);
	hwlog_info("antenna_adc_match_range: min = %d, max = %d\n",
		antenna_adc_match_range[0], antenna_adc_match_range[1]);
}

struct antenna_device_ops adc_match_ops = {
	.get_antenna_match_status = check_match_by_adc,
	.get_antenna_board_voltage = get_voltage_by_adc,
};

static int antenna_board_adc_probe(struct platform_device *pdev)
{
	int ret;
	struct antenna_adc_match_info *di = NULL;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;

	/* get dts data */
	parse_dts(di);

	/* register ops */
	ret = antenna_match_ops_register(&adc_match_ops);
	if (ret) {
		hwlog_err("register antenna_match ops failed!\n");
		goto adc_match_fail0;
	}

	/* if di is used in other function, delete the below two lines */
	kfree(di);
	di = NULL;

	hwlog_info("huawei antenna_board_adc_match probe ok!\n");
	return 0;

adc_match_fail0:
	kfree(di);
	di = NULL;
	return -1;
}

/* probe match table */
static const struct of_device_id antenna_board_adc_table[] = {
	{
		.compatible = "huawei,antenna_board_adc_match",
		.data = NULL,
	},
	{},
};

/* antenna board match adc driver */
static struct platform_driver antenna_board_adc_driver = {
	.probe = antenna_board_adc_probe,
	.driver = {
		.name = "huawei,antenna_board_adc_match",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(antenna_board_adc_table),
	},
};

static int __init antenna_board_adc_init(void)
{
	return platform_driver_register(&antenna_board_adc_driver);
}

static void __exit antenna_board_adc_exit(void)
{
	platform_driver_unregister(&antenna_board_adc_driver);
}

module_init(antenna_board_adc_init);
module_exit(antenna_board_adc_exit);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei antenna board adc driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
