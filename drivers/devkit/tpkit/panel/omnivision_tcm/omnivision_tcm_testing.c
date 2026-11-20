/*
 * Omnivision TCM touchscreen driver
 *
 * Copyright (C) 2017-2018 Omnivision Incorporated. All rights reserved.

 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * INFORMATION CONTAINED IN THIS DOCUMENT IS PROVIDED "AS-IS," AND Omnivision
 * EXPRESSLY DISCLAIMS ALL EXPRESS AND IMPLIED WARRANTIES, INCLUDING ANY
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE,
 * AND ANY WARRANTIES OF NON-INFRINGEMENT OF ANY INTELLECTUAL PROPERTY RIGHTS.
 * IN NO EVENT SHALL Omnivision BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, PUNITIVE, OR CONSEQUENTIAL DAMAGES ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OF THE INFORMATION CONTAINED IN THIS DOCUMENT, HOWEVER CAUSED
 * AND BASED ON ANY THEORY OF LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, AND EVEN IF Omnivision WAS ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE. IF A TRIBUNAL OF COMPETENT JURISDICTION DOES
 * NOT PERMIT THE DISCLAIMER OF DIRECT DAMAGES OR ANY OTHER DAMAGES, Omnivision'
 * TOTAL CUMULATIVE LIABILITY TO ANY PARTY SHALL NOT EXCEED ONE HUNDRED U.S.
 * DOLLARS.
 */

#include <linux/gpio.h>
#include "omnivision_tcm_core.h"
#include "omnivision_tcm_testing.h"

struct testing_hcd {
	bool result;
	unsigned char report_type;
	unsigned int report_index;
	unsigned int num_of_reports;
	struct kobject *sysfs_dir;
	struct ovt_tcm_buffer out;
	struct ovt_tcm_buffer resp;
	struct ovt_tcm_buffer report;
	struct ovt_tcm_buffer process;
	struct ovt_tcm_buffer output;
	struct ovt_tcm_hcd *tcm_hcd;
	int (*collect_reports)(enum report_type report_type,
			unsigned int num_of_reports);

#ifdef PT1_GET_PIN_ASSIGNMENT
#define MAX_PINS (64)
	unsigned char *satic_cfg_buf;
	short tx_pins[MAX_PINS];
	short tx_assigned;
	short rx_pins[MAX_PINS];
	short rx_assigned;
	short guard_pins[MAX_PINS];
	short guard_assigned;
#endif
};

static struct ovt_tcm_test_params test_params;

static struct testing_hcd *testing_hcd;

#define STR_BUF 15
static int ovt_tcm_get_thr_from_csvfile(void)
{
	int ret = NO_ERR;
	unsigned int rows = 0;
	unsigned int cols = 0;
	struct ovt_tcm_app_info *app_info = NULL;
	struct ovt_tcm_test_threshold *threshold = &(test_params.threshold);
	struct ovt_tcm_hcd *tcm_hcd = testing_hcd->tcm_hcd;

	char file_path[256] = {0};
	strncpy(file_path, "/odm/etc/firmware/ts/ovt_tcm_", sizeof(file_path) - 1);
	if (tcm_hcd->hw_if->bdata->project_id) {
		strncat(file_path, tcm_hcd->hw_if->bdata->project_id, MAX_STR_LEN);
	}
	strncat(file_path, "_cap_limits.csv", STR_BUF);

	app_info = &tcm_hcd->app_info;
	rows = le2_to_uint(app_info->num_of_image_rows);
	cols = le2_to_uint(app_info->num_of_image_cols);

	test_params.cap_thr_is_parsed = false;

	TS_LOG_INFO("rows: %d, cols: %d file_path:%s", rows, cols, file_path);
	ret = ts_kit_parse_csvfile(file_path, CSV_RAW_DATA_MIN_ARRAY,
			threshold->raw_data_min_limits, rows, cols);
	if (ret) {
		TS_LOG_INFO("%s: Failed get %s \n", __func__, CSV_RAW_DATA_MIN_ARRAY);
		return ret;
	}

	ret = ts_kit_parse_csvfile(file_path, CSV_RAW_DATA_MAX_ARRAY,
			threshold->raw_data_max_limits, rows, cols);
	if (ret) {
		TS_LOG_INFO("%s: Failed get %s \n", __func__, CSV_RAW_DATA_MAX_ARRAY);
		return ret;
	}

	ret = ts_kit_parse_csvfile(file_path, CSV_OPEN_SHORT_MIN_ARRAY,
			threshold->open_short_min_limits, rows, cols);
	if (ret) {
		TS_LOG_INFO("%s: Failed get %s \n", __func__, CSV_OPEN_SHORT_MIN_ARRAY);
		return ret;
	}

	ret = ts_kit_parse_csvfile(file_path, CSV_OPEN_SHORT_MAX_ARRAY,
			threshold->open_short_max_limits, rows, cols);
	if (ret) {
		TS_LOG_INFO("%s: Failed get %s \n", __func__, CSV_OPEN_SHORT_MAX_ARRAY);
		return ret;
	}

	ret = ts_kit_parse_csvfile(file_path, CSV_LCD_NOISE_ARRAY,
			threshold->lcd_noise_max_limits, rows, cols);
	if (ret) {
		TS_LOG_INFO("%s: Failed get %s \n", __func__, CSV_LCD_NOISE_ARRAY);
		return ret;
	}

	test_params.cap_thr_is_parsed = true;
	return NO_ERR;
}

int ovt_testing_init(struct ovt_tcm_hcd *tcm_hcd)
{
	int retval = 0;
	int idx;

	testing_hcd = kzalloc(sizeof(*testing_hcd), GFP_KERNEL);
	if (!testing_hcd) {
		OVT_LOG_ERR(
				"Failed to allocate memory for testing_hcd");
		return -ENOMEM;
	}

	testing_hcd->tcm_hcd = tcm_hcd;

	INIT_BUFFER(testing_hcd->out, false);
	INIT_BUFFER(testing_hcd->resp, false);
	INIT_BUFFER(testing_hcd->report, false);
	INIT_BUFFER(testing_hcd->process, false);
	INIT_BUFFER(testing_hcd->output, false);

	return retval;
}


static int ovt_testing_do_test_fw_status(struct ts_rawdata_info_new *info)
{
	struct ovt_tcm_hcd *tcm_hcd = testing_hcd->tcm_hcd;
	struct ovt_tcm_app_info *app_info = NULL;

	struct ts_rawdata_newnodeinfo* pts_node = NULL;

	int retval = 0;

	retval = g_tcm_hcd->identify(g_tcm_hcd, true);
	if (retval < 0) {
		return RESULT_ERR;
	}

	OVT_LOG_INFO("ovt_tcm_testing called");
	if (!test_params.cap_thr_is_parsed) {
		int ret;
		ret = ovt_tcm_get_thr_from_csvfile();
		if (ret) {
			OVT_LOG_INFO("ovt_tcm_testing csv parse fail");
			return RESULT_ERR;
		}
	}
	if (!info) {
		OVT_LOG_ERR("param info null");
		return RESULT_ERR;
	}

	OVT_LOG_INFO("ovt_tcm_testing csv parse success");

	pts_node = (struct ts_rawdata_newnodeinfo *)kzalloc(sizeof(struct ts_rawdata_newnodeinfo), GFP_KERNEL);
	if (!pts_node) {
		OVT_LOG_INFO("malloc failed\n");
		return -ENOMEM;
	}

	app_info = &tcm_hcd->app_info;
	info->rx = le2_to_uint(app_info->num_of_image_rows);
	info->tx = le2_to_uint(app_info->num_of_image_cols);

	if (IS_NOT_FW_MODE(tcm_hcd->id_info.mode) ||
			tcm_hcd->app_status != APP_STATUS_OK) {
		pts_node->typeindex = RAW_DATA_TYPE_IC;
		pts_node->testresult = CAP_TEST_FAIL_CHAR;
		list_add_tail(&pts_node->node, &info->rawdata_head);
		info->listnodenum++;
		return -ENODEV;
	} else {
		pts_node->typeindex = RAW_DATA_TYPE_IC;
		pts_node->testresult = CAP_TEST_PASS_CHAR;
		list_add_tail(&pts_node->node, &info->rawdata_head);
		info->listnodenum++;
		return NO_ERR;
	}
	OVT_LOG_INFO("ovt_testing_do_test_fw_status success");
}
static int ovt_testing_do_test_item(enum test_code test_itme, struct ts_rawdata_info_new *info)
{
	int retval = NO_ERR;
	unsigned int idx;
	signed short data;
	unsigned int data_length = 0;
	unsigned int row;
	unsigned int col;
	unsigned int rows;
	unsigned int cols;

	struct ovt_tcm_app_info *app_info = NULL;
	struct ovt_tcm_hcd *tcm_hcd = testing_hcd->tcm_hcd;

	struct ts_rawdata_newnodeinfo* pts_node = NULL;
	int data_max = 0;
	int data_min = 0;
	int data_avg = 0;
	char failedreason[TS_RAWDATA_FAILED_REASON_LEN] = {0};
	char testresult = CAP_TEST_PASS_CHAR;

	pts_node = (struct ts_rawdata_newnodeinfo *)kzalloc(sizeof(struct ts_rawdata_newnodeinfo), GFP_KERNEL);
	if (!pts_node) {
		TS_LOG_ERR("malloc pts_node failed\n");
		return -ENOMEM;
	}

	app_info = &tcm_hcd->app_info;
	rows = le2_to_uint(app_info->num_of_image_rows);
	cols = le2_to_uint(app_info->num_of_image_cols);

	retval = ovt_tcm_alloc_mem(tcm_hcd, &testing_hcd->out, 1);
	if (retval < 0) {
		OVT_LOG_ERR("Failed to allocate memory for testing_hcd->out.buf");
		goto exit;
	}

	testing_hcd->out.buf[0] = test_itme;

	retval = tcm_hcd->write_message(tcm_hcd,
			CMD_PRODUCTION_TEST,
			testing_hcd->out.buf,
			1,
			&testing_hcd->resp.buf,
			&testing_hcd->resp.buf_size,
			&testing_hcd->resp.data_length,
			NULL,
			100);
	if (retval < 0) {
		OVT_LOG_ERR("Failed to write command %s %s\n",
			STR(CMD_PRODUCTION_TEST), STR(test_itme));
		goto exit;
	}

	data_length = testing_hcd->resp.data_length;
	OVT_LOG_INFO("%d\n", data_length);

	pts_node->values = kzalloc((data_length / 2)*sizeof(int), GFP_KERNEL);
	if (!pts_node->values) {
		TS_LOG_ERR("%s malloc value failed  for values\n", __func__);
		testresult = CAP_TEST_FAIL_CHAR;
		strncpy(failedreason, "malloc node value fail", TS_RAWDATA_FAILED_REASON_LEN - 1);
		goto exit;
	}

	idx = 0;
	testing_hcd->result = true;

	OVT_LOG_ERR("%s data begin", STR(test_itme));
	data_min = (signed short)le2_to_uint(&testing_hcd->resp.buf[idx * 2]);
	data_max = data_min;
	data_avg = 0;
	for (row = 0; row < rows; row++) {
		for (col = 0; col < cols; col++) {
			data = (signed short)le2_to_uint(&testing_hcd->resp.buf[idx * 2]);
			pts_node->values[idx] = (int)data;
			data_max = data > data_max ? data : data_max;
			data_min = data < data_min ? data : data_min;
			switch (test_itme) {
				case TEST_PT7_DYNAMIC_RANGE:
					if ((data > test_params.threshold.raw_data_max_limits[idx]) || (data < test_params.threshold.raw_data_min_limits[idx])) {
						OVT_LOG_ERR("overlow_data = %8d, row = %d, col = %d\n", data, row, col);
						testing_hcd->result = false;
					}
				break;
				case TEST_PT10_DELTA_NOISE:
					if (data > test_params.threshold.lcd_noise_max_limits[idx]) {
						OVT_LOG_ERR("overlow_data = %8d, row = %d, col = %d\n", data, row, col);
						testing_hcd->result = false;
					}
				break;
				case TEST_PT11_OPEN_DETECTION:
					if ((data > test_params.threshold.open_short_max_limits[idx]) || (data < test_params.threshold.open_short_min_limits[idx])) {
						OVT_LOG_ERR("overlow_data = %8d, row = %d, col = %d\n", data, row, col);
						testing_hcd->result = false;
					}
				break;
				default:
				break;
			}

			data_avg += data;
			idx++;
		}
	}
	if (idx == 0) {
		OVT_LOG_ERR("Failed idx \n");
		retval = -EINVAL;
		goto exit;
	}
	data_avg = data_avg / idx;
	OVT_LOG_ERR("%s data end", STR(test_itme));

exit:
	pts_node->size = data_length / 2;
	pts_node->testresult = testresult;
	switch (test_itme) {
		case TEST_PT7_DYNAMIC_RANGE:
			pts_node->typeindex = RAW_DATA_TYPE_CAPRAWDATA;	
			strncpy(pts_node->test_name, "Cap_Rawdata", TS_RAWDATA_TEST_NAME_LEN - 1);
		break;
		case TEST_PT10_DELTA_NOISE:
			pts_node->typeindex = RAW_DATA_TYPE_NOISE;
			strncpy(pts_node->test_name, "Cap_Noisedata", TS_RAWDATA_TEST_NAME_LEN - 1);
		break;
		case TEST_PT11_OPEN_DETECTION:
			pts_node->typeindex = RAW_DATA_TYPE_OPENSHORT;
			strncpy(pts_node->test_name, "Cap_Openshortdata", TS_RAWDATA_TEST_NAME_LEN - 1);
		break;
		default:
		break;
	}
	snprintf(pts_node->statistics_data, TS_RAWDATA_STATISTICS_DATA_LEN,
			"[%d,%d,%d]",
			data_min, data_max, data_avg);
	if (CAP_TEST_FAIL_CHAR == testresult) {
		strncpy(pts_node->tptestfailedreason, failedreason, TS_RAWDATA_FAILED_REASON_LEN - 1);
	}
	list_add_tail(&pts_node->node, &info->rawdata_head);
	info->listnodenum++;
	return retval;
}

static void ovt_tcm_put_device_info(struct ts_rawdata_info_new *info)
{
	char buf_fw_ver[CHIP_INFO_LENGTH] = {0};
	struct ovt_tcm_hcd *tcm_hcd = testing_hcd->tcm_hcd;
	unsigned long len;
	OVT_LOG_INFO("ovt_tcm_put_device_info 1");
	strncpy(info->deviceinfo, "-ovt_tcm-", sizeof(info->deviceinfo)-1);
	if (sizeof(info->deviceinfo) > strlen(info->deviceinfo))
		len = sizeof(info->deviceinfo) -
			strlen(info->deviceinfo) - 1;
	else
		len = 0;
	strncat(info->deviceinfo,
		tcm_hcd->id_info.part_number,
		len);
	if (sizeof(info->deviceinfo) > strlen(info->deviceinfo))
		len = sizeof(info->deviceinfo) - strlen(info->deviceinfo) - 1;
	else
		len = 0;
	strncat(info->deviceinfo, "-", len);

	snprintf(buf_fw_ver, CHIP_INFO_LENGTH, "%d", tcm_hcd->packrat_number);
	TS_LOG_INFO("buf_fw_ver = %s", buf_fw_ver);
	if (sizeof(info->deviceinfo) > strlen(info->deviceinfo))
		len = sizeof(info->deviceinfo) - strlen(info->deviceinfo) - 1;
	else
		len = 0;
	strncat(info->deviceinfo, buf_fw_ver, len);

	if (sizeof(info->deviceinfo) > strlen(info->deviceinfo))
		len = sizeof(info->deviceinfo) - strlen(info->deviceinfo) - 1;
	else
		len = 0;
	strncat(info->deviceinfo, ";", len);
}

int ovt_tcm_testing(struct ts_rawdata_info_new *info)
{
	int retval = NO_ERR;

	retval = ovt_testing_do_test_fw_status(info);
	if (retval < 0) {
		OVT_LOG_ERR("fail to do ovt_testing_do_test_fw_status");
		return RESULT_ERR;
		goto exit;
	}

	ovt_tcm_put_device_info(info);

	retval = ovt_testing_do_test_item(TEST_PT7_DYNAMIC_RANGE, info);
	if (retval < 0) {
		OVT_LOG_ERR("fail to do ovt_testing_pt7_dynamic_range");
		goto exit;
	}

	retval = ovt_testing_do_test_item(TEST_PT10_DELTA_NOISE, info);
	if (retval < 0) {
		OVT_LOG_ERR("fail to do ovt_testing_noise test");
		goto exit;
	}

	retval = ovt_testing_do_test_item(TEST_PT11_OPEN_DETECTION, info);
	if (retval < 0) {
		OVT_LOG_ERR("fail to do ovt_testing_noise test");
		goto exit;
	}

	retval = NO_ERR;
exit:
	return retval;
}
