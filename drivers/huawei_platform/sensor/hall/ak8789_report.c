/*
 * ak8987_report.c
 *
 * hall_ak8987 report driver
 *
 * Copyright (c) 2016-2019 Huawei Technologies Co., Ltd.
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

#include "ak8789.h"

#if defined(HALL_DATA_REPORT_INPUTHUB) && defined(HALL_INPUTHUB_ROUTE)
extern int ap_hall_report(int value);
#else
static inline int ap_hall_report(int value)
{
	return 0;
}
#endif

#ifdef HALL_DATA_REPORT_INPUTDEV
#include <linux/input.h>

#define HALL_INPUT_DEV "hall"
#endif

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG ak8789
HWLOG_REGIST();

#ifdef HALL_DATA_REPORT_INPUTDEV
int ak8789_input_register(struct ak8789_data *data)
{
	int ret;

	if (data == NULL) {
		hwlog_err("%s:null point\n", __func__);
		return -ENOMEM;
	}

	data->input_dev = input_allocate_device();
	if (IS_ERR(data->input_dev)) {
		hwlog_err("input dev alloc failed\n");
		ret = -ENOMEM;
		return ret;
	}

	data->input_dev->name = HALL_INPUT_DEV;
	set_bit(EV_MSC, data->input_dev->evbit);
	set_bit(MSC_SCAN, data->input_dev->mscbit);

	ret = input_register_device(data->input_dev);
	if (ret != 0) {
		hwlog_err("hw_input_hall regiset error %d\n", ret);
		input_free_device(data->input_dev);
	}
	return ret;
}

void ak8789_input_unregister(struct ak8789_data *data)
{
	if (data != NULL)
		input_unregister_device(data->input_dev);
}

int ak8789_report_data(void *data, packet_data pdata)
{
	struct input_dev *h_input = data;

	if (h_input == NULL) {
		hwlog_err("%s:null point\n", __func__);
		return 0;
	}

	input_event(h_input, EV_MSC, MSC_SCAN, pdata);
	input_sync(h_input);
	return 1;
}
EXPORT_SYMBOL_GPL(ak8789_report_data);
#endif

#ifdef HALL_DATA_REPORT_INPUTHUB
int ak8789_report_data(void *data, packet_data pdata)
{
	return ap_hall_report(pdata);
}
EXPORT_SYMBOL_GPL(ak8789_report_data);
#endif
