/*
 * Synaptics TCM touchscreen driver
 *
 * Copyright (C) 2017-2018 Synaptics Incorporated. All rights reserved.
 *
 * Copyright (C) 2017-2018 Scott Lin <scott.lin@tw.synaptics.com>
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
 * INFORMATION CONTAINED IN THIS DOCUMENT IS PROVIDED "AS-IS," AND SYNAPTICS
 * EXPRESSLY DISCLAIMS ALL EXPRESS AND IMPLIED WARRANTIES, INCLUDING ANY
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE,
 * AND ANY WARRANTIES OF NON-INFRINGEMENT OF ANY INTELLECTUAL PROPERTY RIGHTS.
 * IN NO EVENT SHALL SYNAPTICS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, PUNITIVE, OR CONSEQUENTIAL DAMAGES ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OF THE INFORMATION CONTAINED IN THIS DOCUMENT, HOWEVER CAUSED
 * AND BASED ON ANY THEORY OF LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, AND EVEN IF SYNAPTICS WAS ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE. IF A TRIBUNAL OF COMPETENT JURISDICTION DOES
 * NOT PERMIT THE DISCLAIMER OF DIRECT DAMAGES OR ANY OTHER DAMAGES, SYNAPTICS'
 * TOTAL CUMULATIVE LIABILITY TO ANY PARTY SHALL NOT EXCEED ONE HUNDRED U.S.
 * DOLLARS.
 */

#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/slab.h>

#include "synaptics_tcm_core.h"
#include "synaptics_tcm_cap_check.h"

#include "huawei_ts_kit_algo.h"
#include "tpkit_platform_adapter.h"
#include "huawei_ts_kit_api.h"

#if defined (CONFIG_HUAWEI_DSM)
#include <dsm/dsm_pub.h>
#endif
#if defined (CONFIG_TEE_TUI)
#include "tui.h"
#endif
#include "huawei_ts_kit.h"

#define SYSFS_DIR_NAME "testing"
#define SYNA_TCM_LIMITS_CSV_FILE "/odm/etc/firmware/ts/syna_tcm_cap_limits.csv"

#define SYNA_TCM_TEST_BUF_LEN 50
#define RES_TIMEOUT_MS 50
#define REPORT_TIMEOUT_MS 200
#define REPORT_TIMEOUT_MS_1 800
#define RESPONSE_TIMEOUT_BASE 10

#define MAX_FRAME_BUFFER 4096
#define READ_REPORT_RETRY_TIMES 3

#define _TEST_PASS_ 1
#define _TEST_FAIL_ 0

#define SYNA_PIN_32 32
#define SYNA_PIN_33 33
int last_test_mode;
enum touch_screen_cap_test_mode {
	CAP_TEST_NORMAL = 1,
	CAP_TEST_SCREENOFF = 2,
};

enum gear_frequency {
	GEAR_FREQUENCY_0 = 0,
	GEAR_FREQUENCY_1,
	GEAR_FREQUENCY_2,
};

struct syna_tcm_test_params *test_params = NULL;

static int tskit_driver_abs(int value)
{
	return value < 0 ? -value : value;
}

//extern char synaptics_raw_data_limit_flag;
extern struct ts_kit_platform_data g_ts_kit_platform_data;

enum test_code {
	TEST_TRX_TRX_SHORTS = 1,
	TEST_TRX_SENSOR_OPENS = 2,
	TEST_TRX_GROUND_SHORTS = 3,
	TEST_FULL_RAW_CAP = 5,
	TEST_DYNAMIC_RANGE = 7,
	TEST_OPEN_SHORT_DETECTOR = 8,
	TEST_PREDECODE_RAW = 9,
	TEST_NOISE = 10,
	TEST_PT11 = 11,
	TEST_PT12 = 12,
	TEST_PT13 = 13,
	TEST_PT18 = 18, /* abs raw cap */
	TEST_PT18_NEW_RULES = 0x12, /* abs raw cap */
	TEST_PT1D = 0x1D, /* abs noise cap */
	TEST_DISCRETE_PT196 = 196,       /* support discrete production test */
};

/* support discrete production test + */
#define MAX_PINS (64)
#define CFG_IMAGE_TXES_OFFSET (3184)
#define CFG_IMAGE_TXES_LENGTH (256)
#define CFG_IMAGE_RXES_OFFSET (2640)
#define CFG_IMAGE_RXES_LENGTH (544)
#define CFG_NUM_TXGUARD_OFFSET (3568)
#define CFG_NUM_TXGUARD_LENGTH (16)
#define CFG_NUM_RXGUARD_OFFSET (3552)
#define CFG_NUM_RXGUARD_LENGTH (16)
#define CFG_TX_GUARD_PINS_OFFSET (6640)
#define CFG_TX_GUARD_PINS_LENGTH (48)
#define CFG_RX_GUARD_PINS_OFFSET (6576)
#define CFG_RX_GUARD_PINS_LENGTH (64)

#define CFG_IMAGE_CBCS_OFFSET (2032)
#define CFG_IMAGE_CBCS_LENGTH (544)
#define CFG_REF_LO_TRANS_CAP_OFFSET (3664)
#define CFG_REF_LO_TRANS_CAP_LENGTH (16)
#define CFG_REF_LO_XMTR_PL_OFFSET (6560)
#define CFG_REF_LO_XMTR_PL_LENGTH (16)
#define CFG_REF_HI_TRANS_CAP_OFFSET (3648)
#define CFG_REF_HI_TRANS_CAP_LENGTH (16)
#define CFG_REF_HI_XMTR_PL_OFFSET (6528)
#define CFG_REF_HI_XMTR_PL_LENGTH (16)
#define CFG_REF_GAIN_CTRL_OFFSET (3632)
#define CFG_REF_GAIN_CTRL_LENGTH (16)

#define SYNA_TCM_STATUS_IDLE 0x00
#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))
#define READ_TEMP_BUF_SIZE 1900 /* tmp buff length 1900 */
/* support discrete production test - */

struct test_hcd {
	short *mmi_buf;
	bool result;
	unsigned char report_type;
	unsigned int report_index;
	unsigned int num_of_reports;
	struct kobject *sysfs_dir;
	struct syna_tcm_buffer out;
	struct syna_tcm_buffer resp;
	struct syna_tcm_buffer report;
	struct syna_tcm_buffer process;
	struct syna_tcm_buffer output;
	struct syna_tcm_hcd *tcm_hcd;
	/* support discrete production test + */
	unsigned char *satic_cfg_buf;
	short tx_pins[MAX_PINS];
	short tx_assigned;
	short rx_pins[MAX_PINS];
	short rx_assigned;
	short g_pins[MAX_PINS];
	short guard_assigned;
	/* support discrete production test - */

};
extern int reflash_after_fw_update_do_reset(void);
static int tskit_driver_trx_trx_short_test(struct ts_rawdata_info_new *info);
static int touch_driver_cap_test_for_newformat(
	struct ts_rawdata_info_new *info);
static int touch_driver_delta_test(
	struct ts_rawdata_info_new *info, int *rawdata);

static struct test_hcd *test_hcd = NULL;
static unsigned char *read_temp_buf = NULL;
static int tskit_driver_testing_realloc_mem(
	struct syna_tcm_hcd *tcm_hcd, unsigned char *buffer,
	unsigned int *buffer_size, unsigned int request_length)
{
	unsigned char *temp = NULL;

	if (request_length > *buffer_size) {
		temp = buffer;

		buffer = kzalloc(request_length, GFP_KERNEL);
		if (!buffer) {
			TS_LOG_ERR("%s: Failed to allocate memory\n",
				__func__);
			kfree(temp);
			return -ENOMEM;
		}
		memcpy(buffer, temp, *buffer_size);
		*buffer_size = request_length;
		kfree(temp);
	}

	return 0;
}
static unsigned char g_temp_buf[2000]; /* rawdata */

static int tskit_driver_testing_continued_read(
	struct syna_tcm_hcd *tcm_hcd, unsigned char *buf,
	unsigned int buf_size, unsigned int payload_length)
{
	int retval;
	unsigned int idx;
	unsigned int offset;
	unsigned int chunks;
	unsigned int chunk_space;
	unsigned int xfer_length;
	unsigned int total_length;
	unsigned int remaining_length;
	unsigned int temp_length = 0;

	total_length = MESSAGE_HEADER_SIZE + payload_length + 1;

	remaining_length = total_length - MESSAGE_HEADER_SIZE;

	retval = tskit_driver_testing_realloc_mem(tcm_hcd,
		buf, &buf_size, total_length + 1);
	if (retval < 0) {
		TS_LOG_ERR("Failed to reallocate memory for tcm_hcd->in.buf\n");
		return retval;
	}

	/*
	 * available chunk space for payload = total chunk size minus header
	 * marker byte and header code byte
	 */
	if (tcm_hcd->rd_chunk_size == 0)
		chunk_space = READ_TEMP_BUF_SIZE;
	else
		chunk_space = tcm_hcd->rd_chunk_size - 2; /* tail size is 2 */

	chunks = ceil_div(remaining_length, chunk_space);

	chunks = chunks == 0 ? 1 : chunks;

	offset = MESSAGE_HEADER_SIZE;

	for (idx = 0; idx < chunks; idx++) {
		if (remaining_length > chunk_space)
			xfer_length = chunk_space;
		else
			xfer_length = remaining_length;

		if (xfer_length == 1) {
			buf[offset] = MESSAGE_PADDING;
			offset += xfer_length;
			remaining_length -= xfer_length;
			continue;
		}

		retval = tskit_driver_read(tcm_hcd, read_temp_buf,
			xfer_length + 2); /* tail size is 2 */
		if (retval < 0) {
			TS_LOG_ERR("Failed to read from device\n");
			goto exit;
		}

		if ((read_temp_buf[0] != MESSAGE_MARKER) ||
			(read_temp_buf[1] != STATUS_CONTINUED_READ)) {
			TS_LOG_ERR("Incorrect header marker 0x%02x, 0x%02x\n",
				read_temp_buf[0], read_temp_buf[1]);
			retval = -EIO;
			goto exit;
		}

		temp_length = READ_TEMP_BUF_SIZE;
		memcpy(&buf[offset], &read_temp_buf[2], xfer_length);
		offset += xfer_length;
		remaining_length -= xfer_length;
	}
exit:
	return retval;
}


static inline int touch_driver_testing_read(
	struct syna_tcm_hcd *tcm_hcd,
	unsigned char *data_buf, unsigned int buf_length)
{
	int retval;
	int retryInterval = RESPONSE_TIMEOUT_BASE;
	int retry = 0;
	int payload_length = 0;

	if (buf_length < MESSAGE_HEADER_SIZE) {
		TS_LOG_ERR("invalid buffer length\n");
		retval = -EINVAL;
		goto exit;
	}

	retry = 2900 / retryInterval;
	msleep(100);
	while (retry) {
		retval = tskit_driver_read(tcm_hcd, data_buf,
			MESSAGE_HEADER_SIZE);

		if (retval < 0) {
			TS_LOG_ERR("Failed to read the data\n");
			goto exit;
		}

		if (data_buf[0] != MESSAGE_MARKER) {
			TS_LOG_ERR("Incorrect Header Marker!\n");
			retval = -EIO;
			goto exit;
		}

		if ((data_buf[0] == MESSAGE_MARKER) &&
			(data_buf[1] != SYNA_TCM_STATUS_IDLE)) {
			payload_length = data_buf[2] | data_buf[3] << 8;
			if (payload_length > 0) {
				retval = tskit_driver_testing_continued_read(
					tcm_hcd, data_buf,
					buf_length, payload_length);
				if (retval < 0) {
					TS_LOG_ERR("Failed to read the data payload\n");
					goto exit;
				}
			}
			break;
		}
		mdelay(retryInterval);
		retry--;
	}

	if (retry == 0) {
		TS_LOG_ERR("Failed to read the data - timeout\n");
		retval = -EINVAL;
	}

exit:
	return retval;
}

static void touch_driver_read_csvfile(unsigned int rows,
	unsigned int cols, char *file_path)
{
	int ret;
	struct syna_tcm_test_threshold *threshold = &test_params->threshold;

	TS_LOG_INFO("%s:rows: %d, cols: %d\n", __func__, rows, cols);
	ret = ts_kit_parse_csvfile(file_path, CSV_CAP_RAW_DATA_MAX_ARRAY,
			threshold->cap_raw_data_max_limits, rows, cols);
	if (ret)
		TS_LOG_INFO("%s: Failed get %s\n", __func__,
			CSV_CAP_RAW_DATA_MAX_ARRAY);
	ret = ts_kit_parse_csvfile(file_path, CSV_CAP_RAW_DATA_MIN_ARRAY,
			threshold->cap_raw_data_min_limits, rows, cols);
	if (ret)
		TS_LOG_INFO("%s: Failed get %s\n", __func__,
			CSV_CAP_RAW_DATA_MIN_ARRAY);

	ret = ts_kit_parse_csvfile(file_path, CSV_RAW_DATA_MIN_ARRAY,
			threshold->raw_data_min_limits, rows, cols);
	if (ret) {
		TS_LOG_INFO("%s: Failed get %s \n", __func__, CSV_RAW_DATA_MIN_ARRAY);
	}

	ret = ts_kit_parse_csvfile(file_path, CSV_RAW_DATA_MAX_ARRAY,
			threshold->raw_data_max_limits, rows, cols);
	if (ret) {
		TS_LOG_INFO("%s: Failed get %s \n", __func__, CSV_RAW_DATA_MAX_ARRAY);
	}

	ret = ts_kit_parse_csvfile(file_path, CSV_OPEN_SHORT_MIN_ARRAY,
			threshold->open_short_min_limits, rows, cols);
	if (ret) {
		TS_LOG_INFO("%s: Failed get %s \n", __func__, CSV_OPEN_SHORT_MIN_ARRAY);
	}

	ret = ts_kit_parse_csvfile(file_path, CSV_OPEN_SHORT_MAX_ARRAY,
			threshold->open_short_max_limits, rows, cols);
	if (ret) {
		TS_LOG_INFO("%s: Failed get %s \n", __func__, CSV_OPEN_SHORT_MAX_ARRAY);
	}

	ret = ts_kit_parse_csvfile(file_path, CSV_LCD_NOISE_ARRAY,
			threshold->lcd_noise_max_limits, rows, cols);
	if (ret) {
		TS_LOG_INFO("%s: Failed get %s \n", __func__, CSV_LCD_NOISE_ARRAY);
	}

	ret = ts_kit_parse_csvfile(file_path, CSV_RAW_DATA_TRX_DELTA_MAX_ARRAY,
			threshold->raw_data_Trx_delta_max_limits, rows*2, cols);
	if (ret) {
		TS_LOG_INFO("%s: Failed get %s \n", __func__, CSV_RAW_DATA_TRX_DELTA_MAX_ARRAY);
	}

	ret = ts_kit_parse_csvfile(file_path, CSV_RAW_DATA_COL_MIN_ARRAY,
			threshold->raw_data_col_min_limits, 1, cols);
	if (ret) {
		TS_LOG_INFO("%s: Failed get %s \n", __func__, CSV_RAW_DATA_COL_MIN_ARRAY);
	}
	ret = ts_kit_parse_csvfile(file_path, CSV_RAW_DATA_COL_MAX_ARRAY,
			threshold->raw_data_col_max_limits, 1, cols);
	if (ret) {
		TS_LOG_INFO("%s: Failed get %s \n", __func__, CSV_RAW_DATA_COL_MAX_ARRAY);
	}

	ret = ts_kit_parse_csvfile(file_path, CSV_RAW_DATA_ROW_MIN_ARRAY,
			threshold->raw_data_row_min_limits, 1, rows);
	if (ret) {
		TS_LOG_INFO("%s: Failed get %s \n", __func__, CSV_RAW_DATA_COL_MIN_ARRAY);
	}
	ret = ts_kit_parse_csvfile(file_path, CSV_RAW_DATA_ROW_MAX_ARRAY,
			threshold->raw_data_row_max_limits, 1, rows);
	if (ret) {
		TS_LOG_INFO("%s: Failed get %s \n", __func__, CSV_RAW_DATA_COL_MAX_ARRAY);
	}

	ret = ts_kit_parse_csvfile(file_path, CSV_ABS_NOISE_ROW_MAX_ARRAY,
		threshold->abs_noise_row_max_limits, 1, rows);
	if (ret)
		TS_LOG_INFO("Failed get %s\n", CSV_ABS_NOISE_ROW_MAX_ARRAY);

	ret = ts_kit_parse_csvfile(file_path, CSV_ABS_NOISE_COL_MAX_ARRAY,
		threshold->abs_noise_col_max_limits, 1, cols);
	if (ret)
		TS_LOG_INFO("Failed get %s\n", CSV_ABS_NOISE_COL_MAX_ARRAY);

}

static void touch_driver_get_thr_from_csvfile(void)
{
	unsigned int rows;
	unsigned int cols;
	unsigned int current_test_mode = 0;
	struct syna_tcm_app_info *app_info = NULL;
	struct syna_tcm_hcd *tcm_hcd = test_hcd->tcm_hcd;
	char file_path[MAX_STR_LEN*5] = {0};
	char file_name[MAX_STR_LEN*4] = {0};
	struct ts_kit_platform_data *ts_platform_data =
		test_hcd->tcm_hcd->syna_tcm_chip_data->ts_platform_data;
	struct ts_factory_extra_cmd *factory_extra_cmd = NULL;

	factory_extra_cmd =
		&tcm_hcd->syna_tcm_chip_data->ts_platform_data->factory_extra_cmd;

	if (tcm_hcd->bdata && tcm_hcd->bdata->support_vendor_ic_type == S3909) {
		if (factory_extra_cmd->support_factory_mode_extra_cmd == 1 &&
			factory_extra_cmd->always_poweron_in_screenoff == 1) {
			TS_LOG_INFO("%s screen off file\n", __func__);
			snprintf(file_name, (MAX_STR_LEN * 2),
				"%s_%s_limits_screenoff.csv",
				ts_platform_data->product_name,
				tcm_hcd->tcm_mod_info.project_id_string);
			current_test_mode = CAP_TEST_SCREENOFF;
		} else {
			TS_LOG_INFO("%s normal file\n", __func__);
			snprintf(file_name, (MAX_STR_LEN * 2),
				"%s_%s_limits.csv",
				ts_platform_data->product_name,
				tcm_hcd->tcm_mod_info.project_id_string);
			current_test_mode = CAP_TEST_NORMAL;
		}
	} else {
		snprintf(file_name, (MAX_STR_LEN * 4), "%s_%s_%s_limits.csv",
			ts_platform_data->product_name,
			SYNAPTICS_VENDER_NAME,
			tcm_hcd->tcm_mod_info.project_id_string);
	}

#ifdef BOARD_VENDORIMAGE_FILE_SYSTEM_TYPE
	snprintf(file_path, sizeof(file_path), "/product/etc/firmware/ts/%s",
		file_name);
#else
	snprintf(file_path, sizeof(file_path), "/odm/etc/firmware/ts/%s",
		file_name);
#endif

	app_info = &tcm_hcd->app_info;
	rows = le2_to_uint(app_info->num_of_image_rows);
	cols = le2_to_uint(app_info->num_of_image_cols);

	if (tcm_hcd->bdata && tcm_hcd->bdata->support_vendor_ic_type == S3909) {
		TS_LOG_INFO("%s last_test_mode=%d current_test_mode=%d\n",
			__func__, last_test_mode, current_test_mode);
		if (last_test_mode == current_test_mode)
			test_params->cap_thr_is_parsed = true;
		else
			test_params->cap_thr_is_parsed = false;
		if (test_params->cap_thr_is_parsed == false)
			touch_driver_read_csvfile(rows, cols, file_path);
		last_test_mode = current_test_mode;
	} else {
		touch_driver_read_csvfile(rows, cols, file_path);
		test_params->cap_thr_is_parsed = true;
	}
}

static void tskit_driver_get_frame_size_words(
	unsigned int *size, bool image_only)
{
	unsigned int rows = 0;
	unsigned int cols = 0;
	unsigned int hybrid = 0;
	unsigned int buttons = 0;
	struct syna_tcm_app_info *app_info = NULL;
	struct syna_tcm_hcd *tcm_hcd = test_hcd->tcm_hcd;

	app_info = &tcm_hcd->app_info;

	rows = le2_to_uint(app_info->num_of_image_rows);
	cols = le2_to_uint(app_info->num_of_image_cols);
	hybrid = le2_to_uint(app_info->has_hybrid_data);
	buttons = le2_to_uint(app_info->num_of_buttons);

	*size = rows * cols;

	if (!image_only) {
		if (hybrid)
			*size += rows + cols;
		*size += buttons;
	}

	return;
}

static void tskit_driver_put_device_info(struct ts_rawdata_info_new *info)
{
	char buf_fw_ver[CHIP_INFO_LENGTH] = {0};
	struct syna_tcm_hcd *tcm_hcd = test_hcd->tcm_hcd;
	unsigned long len;

	if (tcm_hcd->bdata && tcm_hcd->bdata->support_vendor_ic_type != S3909) {
		strncpy(info->deviceinfo, "-syna_tcm-",
			sizeof(info->deviceinfo) - 1);
		if (sizeof(info->deviceinfo) > strlen(info->deviceinfo))
			len = sizeof(info->deviceinfo) -
				strlen(info->deviceinfo) - 1;
		else
			len = 0;
		strncat(info->deviceinfo,
			tcm_hcd->tcm_mod_info.project_id_string,
			len);
	} else {
		strncpy(info->deviceinfo,
			tcm_hcd->tcm_mod_info.project_id_string,
			strlen(tcm_hcd->tcm_mod_info.project_id_string));
	}
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

	if (tcm_hcd->bdata && tcm_hcd->bdata->support_vendor_ic_type == S3909) {
		if (sizeof(info->deviceinfo) > strlen(info->deviceinfo))
			len = sizeof(info->deviceinfo) -
				strlen(info->deviceinfo) - 1;
		else
			len = 0;
		strncat(info->deviceinfo, "-", len);

		/* store 4 byte config id */
		snprintf(buf_fw_ver, SYNC_TCM_CONFIG_ID_ARRY_SIZE,
			"%02x%02x_%02x%02x",
			tcm_hcd->app_info.customer_config_id[0],
			tcm_hcd->app_info.customer_config_id[1],
			tcm_hcd->app_info.customer_config_id[2],
			tcm_hcd->app_info.customer_config_id[3]);
		if (sizeof(info->deviceinfo) > strlen(info->deviceinfo))
			len = sizeof(info->deviceinfo) -
				strlen(info->deviceinfo) - 1;
		else
			len = 0;
		strncat(info->deviceinfo, buf_fw_ver, len);
	}

	if (sizeof(info->deviceinfo) > strlen(info->deviceinfo))
		len = sizeof(info->deviceinfo) - strlen(info->deviceinfo) - 1;
	else
		len = 0;
	strncat(info->deviceinfo, ";", len);
}

static int tskit_driver_open_short_test(struct ts_rawdata_info_new *info)
{
	int retval = 0;
	unsigned int timeout = 0;
	unsigned int data_length = 0;
	signed short data = 0;
	unsigned int idx = 0;
	unsigned int row = 0;
	unsigned int col = 0;
	unsigned int rows = 0;
	unsigned int cols = 0;
	unsigned int frame_size_words = 0;
	unsigned char *temp_buf = NULL;
	int open_short_data_max = 0;
	int open_short_data_min = 0;
	int open_short_data_avg = 0;
	int open_short_data_size = 0;
	char testresult = CAP_TEST_PASS_CHAR;
	char failedreason[TS_RAWDATA_FAILED_REASON_LEN] = {0};
	int retry = READ_REPORT_RETRY_TIMES;

	struct syna_tcm_app_info *app_info = NULL;
	struct syna_tcm_hcd *tcm_hcd = test_hcd->tcm_hcd;
	struct ts_rawdata_newnodeinfo *pts_node = NULL;
	struct syna_tcm_test_threshold *threshold = &test_params->threshold;

	TS_LOG_INFO("%s called\n", __func__);

	if (tcm_hcd->id_info.mode != MODE_APPLICATION ||
			tcm_hcd->app_status != APP_STATUS_OK) {
		return -ENODEV;
	}

	app_info = &tcm_hcd->app_info;
	rows = le2_to_uint(app_info->num_of_image_rows);
	cols = le2_to_uint(app_info->num_of_image_cols);
	open_short_data_size = rows * cols;

	pts_node = (struct ts_rawdata_newnodeinfo *)kzalloc(sizeof(struct ts_rawdata_newnodeinfo), GFP_KERNEL);
	if (!pts_node) {
		TS_LOG_ERR("malloc pts_node failed\n");
		return -ENOMEM;
	}

	pts_node->values = kzalloc(open_short_data_size*sizeof(int), GFP_KERNEL);
	if (!pts_node->values) {
		TS_LOG_ERR("%s malloc value failed  for values\n", __func__);
		testresult = CAP_TEST_FAIL_CHAR;
		strncpy(failedreason, "-software_reason", TS_RAWDATA_FAILED_REASON_LEN-1);
		retval = -ENOMEM;
		goto exit;
	}

	temp_buf = kzalloc(((rows * cols + MESSAGE_HEADER_SIZE) * 2), GFP_KERNEL);
	if (!temp_buf) {
		TS_LOG_ERR("%s Failed to allocate memory for temp_buf\n", __func__);
		testresult = CAP_TEST_FAIL_CHAR;
		strncpy(failedreason, "-software_reason", TS_RAWDATA_FAILED_REASON_LEN-1);
		retval = -ENOMEM;
		goto exit;
	}

	retval = tskit_driver_alloc_mem(tcm_hcd,
			&test_hcd->out,
			1);
	if (retval < 0) {
		TS_LOG_ERR(
				"Failed to allocate memory for testing_hcd->out.buf\n");
		goto exit;
	}

	test_hcd->out.buf[0] = TEST_PT11;
	retval = tskit_driver_write_hdl_message(tcm_hcd,
			CMD_PRODUCTION_TEST,
			test_hcd->out.buf,
			1,
			&test_hcd->resp.buf,
			&test_hcd->resp.buf_size,
			&test_hcd->resp.data_length,
			NULL,
			0);
	if (retval < 0) {
		TS_LOG_ERR(
				"Failed to write command %s\n",
				STR(CMD_PRODUCTION_TEST));
		goto exit;
	}

	while(retry){
		timeout = REPORT_TIMEOUT_MS_1;
		msleep(timeout);

		retval = tskit_driver_read(tcm_hcd,
				temp_buf,
				((rows * cols + MESSAGE_HEADER_SIZE) * 2));
		if (retval < 0) {
			TS_LOG_ERR("Failed to read raw data\n");
			goto exit;
		}

		if (temp_buf[0] != MESSAGE_MARKER) {
			TS_LOG_ERR("incorrect Header Marker!");
			retval = -EINVAL;
			goto exit;
		}

		if (temp_buf[1] != STATUS_OK) {
			TS_LOG_INFO("status = 0x%02x", temp_buf[1]);
		}else{
			break;
		}
		retry--;
	}

	data_length = temp_buf[2] | temp_buf[3] << 8;
	TS_LOG_INFO("%d\n", data_length);

	tskit_driver_get_frame_size_words(&frame_size_words, false);

	if (frame_size_words != data_length / 2) {
		TS_LOG_ERR("Frame size mismatch\n");
		goto exit;
	}

	idx = 0;
	open_short_data_max = open_short_data_min = le2_to_uint(&temp_buf[MESSAGE_HEADER_SIZE]);
	for (row = 0; row < rows; row++) {
		for (col = 0; col < cols; col++) {
			data = (signed short)le2_to_uint(&temp_buf[idx * 2 + MESSAGE_HEADER_SIZE]);
			pts_node->values[idx] = (int)data;
			open_short_data_max = data > open_short_data_max ? data : open_short_data_max;
			open_short_data_min = data < open_short_data_min ? data : open_short_data_min;
			open_short_data_avg += data;
			TS_LOG_DEBUG("open_short = %d", data);
			idx++;
		}
	}
	if (open_short_data_size)
		open_short_data_avg /= open_short_data_size;

	idx = 0;
	test_hcd->result = true;
	for (row = 0; row < rows; row++) {
		for (col = 0; col < cols; col++) {
			data = (signed short)le2_to_uint(&temp_buf[idx * 2 + MESSAGE_HEADER_SIZE]);
			TS_LOG_DEBUG("open_short data=%d", data);
			if (data > threshold->open_short_max_limits[idx] ||
					data < threshold->open_short_min_limits[idx]) {
				TS_LOG_ERR("%s overlow_data = %d, row = %d, col = %d , limits [%d, %d]\n", __func__, data, row, col, threshold->open_short_min_limits[idx], threshold->open_short_max_limits[idx]);
				test_hcd->result = false;
				testresult = CAP_TEST_FAIL_CHAR;
				strncpy(failedreason, "-panel_reason", TS_RAWDATA_FAILED_REASON_LEN-1);
				goto exit;
			}
			idx++;
		}
	}

exit:
	pts_node->size = open_short_data_size;
	pts_node->testresult = testresult;
	pts_node->typeindex = RAW_DATA_TYPE_OPENSHORT;
	strncpy(pts_node->test_name, "Open_short", TS_RAWDATA_TEST_NAME_LEN-1);
	snprintf(pts_node->statistics_data, TS_RAWDATA_STATISTICS_DATA_LEN,
			"[%d,%d,%d]",
			open_short_data_min, open_short_data_max, open_short_data_avg);
	if (CAP_TEST_FAIL_CHAR == testresult) {
		strncpy(pts_node->tptestfailedreason, failedreason, TS_RAWDATA_FAILED_REASON_LEN-1);
	}
	list_add_tail(&pts_node->node, &info->rawdata_head);

	if(temp_buf) {
		kfree(temp_buf);
		temp_buf = NULL;
	}
	TS_LOG_INFO("%s end retval = %d\n", __func__, retval);

	return retval;
}

static int tskit_driver_noise_test(struct ts_rawdata_info_new *info)
{
	int retval = 0;
	unsigned int idx = 0;
	signed short data = 0;
	unsigned int timeout = 0;
	unsigned int data_length = 0;
	unsigned int row = 0;
	unsigned int col = 0;
	unsigned int rows = 0;
	unsigned int cols = 0;
	unsigned int frame_size_words = 0;
	unsigned char *temp_buf = NULL;
	char testresult = CAP_TEST_PASS_CHAR;
	char failedreason[TS_RAWDATA_FAILED_REASON_LEN] = {0};
	int noise_data_max = 0;
	int noise_data_min = 0;
	int noise_data_avg = 0;
	int noise_data_size = 0;
	int retry = READ_REPORT_RETRY_TIMES;

	struct syna_tcm_app_info *app_info = NULL;
	struct syna_tcm_hcd *tcm_hcd = test_hcd->tcm_hcd;
	struct ts_rawdata_newnodeinfo* pts_node = NULL;
	struct syna_tcm_test_threshold *threshold = &test_params->threshold;

	TS_LOG_INFO("%s start", __func__);

	if (tcm_hcd->id_info.mode != MODE_APPLICATION ||
			tcm_hcd->app_status != APP_STATUS_OK) {
		TS_LOG_ERR("%s Application firmware not running\n", __func__);
		return -ENODEV;
	}

	app_info = &tcm_hcd->app_info;
	rows = le2_to_uint(app_info->num_of_image_rows);
	cols = le2_to_uint(app_info->num_of_image_cols);
	noise_data_size = rows * cols;

	pts_node = (struct ts_rawdata_newnodeinfo *)kzalloc(sizeof(struct ts_rawdata_newnodeinfo), GFP_KERNEL);
	if (!pts_node) {
		TS_LOG_ERR("malloc pts_node failed\n");
		return -ENOMEM;
	}

	pts_node->values = kzalloc(noise_data_size*sizeof(int), GFP_KERNEL);
	if (!pts_node->values) {
		TS_LOG_ERR("%s malloc value failed  for values\n", __func__);
		testresult = CAP_TEST_FAIL_CHAR;
		strncpy(failedreason, "-software_reason", TS_RAWDATA_FAILED_REASON_LEN-1);
		goto exit;
	}

	temp_buf = kzalloc(((rows * cols + MESSAGE_HEADER_SIZE) * 2), GFP_KERNEL);
	if (!temp_buf) {
		TS_LOG_ERR("%s Failed to allocate memory for temp_buf\n", __func__);
		testresult = CAP_TEST_FAIL_CHAR;
		strncpy(failedreason, "-software_reason", TS_RAWDATA_FAILED_REASON_LEN-1);
		retval = -ENOMEM;
		goto exit;
	}

	retval = tskit_driver_alloc_mem(tcm_hcd,
			&test_hcd->out,
			1);
	if (retval < 0) {
		TS_LOG_ERR(
				"Failed to allocate memory for testing_hcd->out.buf\n");
		goto exit;
	}

	test_hcd->out.buf[0] = TEST_NOISE;

	retval = tskit_driver_write_hdl_message(tcm_hcd,
			CMD_PRODUCTION_TEST,
			test_hcd->out.buf,
			1,
			&test_hcd->resp.buf,
			&test_hcd->resp.buf_size,
			&test_hcd->resp.data_length,
			NULL,
			0);
	if (retval < 0) {
		TS_LOG_ERR(
				"Failed to write command %s\n",
				STR(CMD_PRODUCTION_TEST));
		goto exit;
	}

	while(retry) {
		timeout = REPORT_TIMEOUT_MS_1;   //REPORT_TIMEOUT_MS
		msleep(timeout);
		retval = tskit_driver_read(tcm_hcd,
				temp_buf,
				((rows * cols + MESSAGE_HEADER_SIZE) * 2));
		if (retval < 0) {
			TS_LOG_ERR("Failed to read raw data\n");
			goto exit;
		}

		if (temp_buf[0] != MESSAGE_MARKER) {
			TS_LOG_ERR("incorrect Header Marker!");
			retval = -EINVAL;
			goto exit;
		}

		if (temp_buf[1] != STATUS_OK) {
			TS_LOG_INFO("status = 0x%02x", temp_buf[1]);
		}else{
			break;
		}
		retry--;
	}

	data_length = temp_buf[2] | temp_buf[3] << 8;
	TS_LOG_INFO(" data_length =%d\n", data_length);
	if (tcm_hcd->htd == true) {
		tskit_driver_get_frame_size_words(&frame_size_words, false);
	} else {
		tskit_driver_get_frame_size_words(&frame_size_words, true);
	}

	if (frame_size_words != data_length / 2) {
		TS_LOG_ERR(
				"Frame size mismatch\n");
		retval = -EINVAL;
		goto exit;
	}

	idx = 0;
	noise_data_max = noise_data_min = (int)le2_to_uint(&temp_buf[MESSAGE_HEADER_SIZE]);
	for (row = 0; row < rows; row++) {
		for (col = 0; col < cols; col++) {
			data = (signed short)le2_to_uint(&temp_buf[idx * 2 + MESSAGE_HEADER_SIZE]);
			pts_node->values[idx] = (int)data;
			noise_data_max = data > noise_data_max ? data : noise_data_max;
			noise_data_min = data < noise_data_min ? data : noise_data_min;
			noise_data_avg += data;
			TS_LOG_DEBUG("noise_buf = %d", data);
			idx++;
		}
	}
	if (noise_data_size)
		noise_data_avg /= noise_data_size;

	idx = 0;
	test_hcd->result = true;

	for (row = 0; row < rows; row++) {
		for (col = 0; col < cols; col++) {
			data = (signed short)le2_to_uint(&temp_buf[idx * 2 + MESSAGE_HEADER_SIZE]);
			TS_LOG_DEBUG("noise data=%d", data);
			if (data > threshold->lcd_noise_max_limits[idx]) {
				TS_LOG_ERR("%s overlow_data = %d, limits = %d, row = %d, col = %d\n", __func__, data,threshold->lcd_noise_max_limits[idx], row, col);
				test_hcd->result = false;
				testresult = CAP_TEST_FAIL_CHAR;
				strncpy(failedreason, "-panel_reason", TS_RAWDATA_FAILED_REASON_LEN-1);
				goto exit;
			}
			idx++;
		}
	}

exit:
	TS_LOG_INFO("%s end retval = %d", __func__, retval);

	pts_node->size = noise_data_size;
	pts_node->testresult = testresult;
	pts_node->typeindex = RAW_DATA_TYPE_NOISE;
	strncpy(pts_node->test_name, "Noise_delta", TS_RAWDATA_TEST_NAME_LEN-1);
	snprintf(pts_node->statistics_data, TS_RAWDATA_STATISTICS_DATA_LEN,
			"[%d,%d,%d]",
			noise_data_min, noise_data_max, noise_data_avg);
	if (CAP_TEST_FAIL_CHAR == testresult) {
		strncpy(pts_node->tptestfailedreason, failedreason, TS_RAWDATA_FAILED_REASON_LEN-1);
	}
	list_add_tail(&pts_node->node, &info->rawdata_head);

	if(temp_buf) {
		kfree(temp_buf);
		temp_buf = NULL;
	}
	return retval;
}

static int tskit_driver_dynamic_range_test(struct ts_rawdata_info_new *info)
{
	int retval = 0;
	unsigned int idx = 0;
	signed short data = 0;
	unsigned int timeout = 0;
	unsigned int data_length = 0;
	unsigned int row = 0;
	unsigned int col = 0;
	unsigned int rows = 0;
	unsigned int cols = 0;
	unsigned int frame_size_words = 0;
	unsigned char *temp_buf = NULL;
	char testresult = CAP_TEST_PASS_CHAR;
	char failedreason[TS_RAWDATA_FAILED_REASON_LEN] = {0};
	int raw_data_size= 0;
	int raw_data_max = 0;
	int raw_data_min = 0;
	int raw_data_avg = 0;
	int retry = READ_REPORT_RETRY_TIMES;

	struct syna_tcm_app_info *app_info = NULL;
	struct syna_tcm_hcd *tcm_hcd = test_hcd->tcm_hcd;
	struct ts_rawdata_newnodeinfo* pts_node = NULL;
	struct syna_tcm_test_threshold *threshold = &test_params->threshold;

	TS_LOG_INFO("%s start", __func__);

	if (tcm_hcd->id_info.mode != MODE_APPLICATION ||
			tcm_hcd->app_status != APP_STATUS_OK) {
		return -ENODEV;
	}

	app_info = &tcm_hcd->app_info;
	rows = le2_to_uint(app_info->num_of_image_rows);
	cols = le2_to_uint(app_info->num_of_image_cols);
	raw_data_size = rows * cols;

	pts_node = (struct ts_rawdata_newnodeinfo *)kzalloc(sizeof(struct ts_rawdata_newnodeinfo), GFP_KERNEL);
	if (!pts_node) {
		TS_LOG_ERR("malloc pts_node failed\n");
		return -ENOMEM;
	}

	pts_node->values = kzalloc(raw_data_size*sizeof(int), GFP_KERNEL);
	if (!pts_node->values) {
		TS_LOG_ERR("%s malloc value failed  for values\n", __func__);
		testresult = CAP_TEST_FAIL_CHAR;
		strncpy(failedreason, "-software_reason", TS_RAWDATA_FAILED_REASON_LEN-1);
		goto exit;
	}

	temp_buf = kzalloc(((rows * cols + MESSAGE_HEADER_SIZE) * 2), GFP_KERNEL);
	if (!temp_buf) {
		TS_LOG_ERR("%s Failed to allocate memory for temp_buf\n", __func__);
		testresult = CAP_TEST_FAIL_CHAR;
		strncpy(failedreason, "-software_reason", TS_RAWDATA_FAILED_REASON_LEN-1);
		retval = -ENOMEM;
		goto exit;
	}

	retval = tskit_driver_alloc_mem(tcm_hcd,
			&test_hcd->out,
			1);
	if (retval < 0) {
		TS_LOG_ERR(
				"Failed to allocate memory for testing_hcd->out.buf\n");
		goto exit;
	}

	test_hcd->out.buf[0] = TEST_DYNAMIC_RANGE;

	retval = tskit_driver_write_hdl_message(tcm_hcd,
			CMD_PRODUCTION_TEST,
			test_hcd->out.buf,
			1,
			&test_hcd->resp.buf,
			&test_hcd->resp.buf_size,
			&test_hcd->resp.data_length,
			NULL,
			0);
	if (retval < 0) {
		TS_LOG_ERR(
				"Failed to write command %s\n",
				STR(CMD_PRODUCTION_TEST));
		goto exit;
	}

	while(retry) {
		timeout = REPORT_TIMEOUT_MS_1;   //REPORT_TIMEOUT_MS
		msleep(timeout);
		retval = tskit_driver_read(tcm_hcd,
				temp_buf,
				((rows * cols + MESSAGE_HEADER_SIZE) * 2));
		if (retval < 0) {
			TS_LOG_ERR("Failed to read raw data\n");
			goto exit;
		}

		if (temp_buf[0] != MESSAGE_MARKER) {
			TS_LOG_ERR("incorrect Header Marker!");
			retval = -EINVAL;
			goto exit;
		}

		if (temp_buf[1] != STATUS_OK) {
			TS_LOG_INFO("status = 0x%02x", temp_buf[1]);
		}else{
			break;
		}
		retry--;
	}

	data_length = temp_buf[2] | temp_buf[3] << 8;
	TS_LOG_INFO(" data_length =%d\n", data_length);

	tskit_driver_get_frame_size_words(&frame_size_words, false);
	if (frame_size_words != data_length / 2) {
		TS_LOG_ERR(
				"Frame size mismatch\n");
		retval = -EINVAL;
		goto exit;
	}
	idx = 0;
	raw_data_max = raw_data_min = (int)le2_to_uint(&temp_buf[MESSAGE_HEADER_SIZE]);
	for (row = 0; row < rows; row++) {
		for (col = 0; col < cols; col++) {
			data = (short)le2_to_uint(&temp_buf[idx * 2 + MESSAGE_HEADER_SIZE]);
			pts_node->values[idx] = (int)data;
			raw_data_max = data > raw_data_max ? data : raw_data_max;
			raw_data_min = data < raw_data_min ? data : raw_data_min;
			raw_data_avg += data;
			idx++;
		}
	}
	if (raw_data_size)
		raw_data_avg /= raw_data_size;

	idx = 0;
	test_hcd->result = true;

	for (row = 0; row < rows; row++) {
		for (col = 0; col < cols; col++) {
			data = (short)le2_to_uint(&temp_buf[idx * 2 + MESSAGE_HEADER_SIZE]);
			if (data > threshold->raw_data_max_limits[idx] ||
				data < threshold->raw_data_min_limits[idx]) {
				TS_LOG_ERR("%s overlow_data = %d, row = %d, col = %d, limits [%d,%d]\n", __func__, data, row, col, threshold->raw_data_max_limits[idx], threshold->raw_data_min_limits[idx]);
				test_hcd->result = false;
				testresult = CAP_TEST_FAIL_CHAR;
				strncpy(failedreason, "-panel_reason", TS_RAWDATA_FAILED_REASON_LEN-1);
				goto exit;
			}
			idx++;
		}
	}

exit:
	TS_LOG_INFO("%s end retval = %d\n", __func__, retval);
	pts_node->size = raw_data_size;
	pts_node->testresult = testresult;
	pts_node->typeindex = RAW_DATA_TYPE_CAPRAWDATA;
	strncpy(pts_node->test_name, "Cap_Rawdata", TS_RAWDATA_TEST_NAME_LEN-1);
	snprintf(pts_node->statistics_data, TS_RAWDATA_STATISTICS_DATA_LEN,
			"[%d,%d,%d]",
			raw_data_min, raw_data_max, raw_data_avg);
	if (CAP_TEST_FAIL_CHAR == testresult) {
		strncpy(pts_node->tptestfailedreason, failedreason, TS_RAWDATA_FAILED_REASON_LEN-1);
	}
	list_add_tail(&pts_node->node, &info->rawdata_head);

	test_hcd->report_type = 0;
	if(temp_buf) {
		kfree(temp_buf);
		temp_buf = NULL;
	}

	return retval;
}

static int tskit_driver_delta_test(
	struct ts_rawdata_info_new *info, int *rawdata)
{
	int retval = 0;
	unsigned int rows = 0;
	unsigned int cols = 0;
	char testresult = CAP_TEST_PASS_CHAR;
	char failedreason[TS_RAWDATA_FAILED_REASON_LEN] = {0};
	int raw_data_size= 0;
	int delta_data_max = 0;
	int delta_data_min = 0;
	int delta_data_avg = 0;
	int i = 0;
	int j = 0;
	int delta_data_size = 0;
	struct syna_tcm_app_info *app_info = NULL;
	struct syna_tcm_hcd *tcm_hcd = test_hcd->tcm_hcd;
	struct ts_rawdata_newnodeinfo* pts_node = NULL;
	struct syna_tcm_test_threshold *threshold = &test_params->threshold;

	TS_LOG_INFO("%s start", __func__);

	if (tcm_hcd->id_info.mode != MODE_APPLICATION ||
			tcm_hcd->app_status != APP_STATUS_OK) {
		TS_LOG_ERR("%s Application firmware not running\n", __func__);
		return -ENODEV;
	}

	app_info = &tcm_hcd->app_info;
	rows = le2_to_uint(app_info->num_of_image_rows);
	cols = le2_to_uint(app_info->num_of_image_cols);
	raw_data_size = rows * cols;

	pts_node = (struct ts_rawdata_newnodeinfo *)kzalloc(sizeof(struct ts_rawdata_newnodeinfo), GFP_KERNEL);
	if (!pts_node) {
		TS_LOG_ERR("malloc pts_node failed\n");
		return -ENOMEM;
	}

	pts_node->values = kzalloc(raw_data_size*sizeof(int) * 2, GFP_KERNEL);
	if (!pts_node->values) {
		TS_LOG_ERR("%s malloc value failed	for values\n", __func__);
		testresult = CAP_TEST_FAIL_CHAR;
		strncpy(failedreason, "-software_reason", TS_RAWDATA_FAILED_REASON_LEN-1);
		goto exit;
	}
	memset(pts_node->values, 0 ,raw_data_size*sizeof(int) * 2);
	test_hcd->result = true;
	delta_data_max = delta_data_min =
		tskit_driver_abs(rawdata[0]-rawdata[1]);

	for(i = 0, j=0; i < rows * cols ; i++) {
		if (i < cols) {
			/* nothing to do*/
		} else {
			pts_node->values[j] =
				tskit_driver_abs(rawdata[i] -
				rawdata[i - cols]);
			delta_data_max = delta_data_max < pts_node->values[j] ? pts_node->values[j] : delta_data_max;
			delta_data_min = delta_data_min > pts_node->values[j] ? pts_node->values[j] : delta_data_min;
			delta_data_avg += pts_node->values[j];
			j++;
		}
	}
	delta_data_size += j;

	for(i = 0, j=0 ; i < rows * cols ; i++) {
		if (i%cols == 0) {
			/* nothing to do*/
		} else {
			pts_node->values[j + raw_data_size] =
				tskit_driver_abs(rawdata[i] - rawdata[i - 1]);
			delta_data_max = delta_data_max < pts_node->values[j] ? pts_node->values[j] : delta_data_max;
			delta_data_min = delta_data_min > pts_node->values[j] ? pts_node->values[j] : delta_data_min;
			delta_data_avg += pts_node->values[j];
			j++;
		}
	}
	delta_data_size += j;

	TS_LOG_ERR("delta_data_size = %d\n", delta_data_size);

	if(delta_data_size){
		delta_data_avg /= delta_data_size;
	}

	for (i = 0; i < raw_data_size*2; i++) {
		TS_LOG_DEBUG("delta value is[%d, %d] %d > %d\n", i/cols, i%cols,
				pts_node->values[i],
				threshold->raw_data_Trx_delta_max_limits[i]);
		if (pts_node->values[i] > threshold->raw_data_Trx_delta_max_limits[i]) {
			testresult = CAP_TEST_FAIL_CHAR;
			strncpy(failedreason, "-panel_reason",TS_RAWDATA_FAILED_REASON_LEN-1);
			TS_LOG_ERR("delta value is[%d, %d] %d > %d\n", i/cols, i%cols,
				pts_node->values[i],
				threshold->raw_data_Trx_delta_max_limits[i]);
		}
	}

exit:
	TS_LOG_INFO("%s end retval = %d\n", __func__, retval);
	pts_node->size = raw_data_size*2;
	pts_node->testresult = testresult;
	pts_node->typeindex = RAW_DATA_TYPE_TRXDELTA;
	strncpy(pts_node->test_name, "delta_Rawdata", TS_RAWDATA_TEST_NAME_LEN-1);
	snprintf(pts_node->statistics_data, TS_RAWDATA_STATISTICS_DATA_LEN,
			"[%d,%d,%d]",
			delta_data_min, delta_data_max, delta_data_avg);
	if (CAP_TEST_FAIL_CHAR == testresult) {
		strncpy(pts_node->tptestfailedreason, failedreason, TS_RAWDATA_FAILED_REASON_LEN-1);
	}
	list_add_tail(&pts_node->node, &info->rawdata_head);
	info->listnodenum++;

	test_hcd->report_type = 0;

	return retval;
}

static int tskit_driver_full_raw_cap_test(struct ts_rawdata_info_new *info)
{
	int retval = 0;
	unsigned int idx = 0;
	signed short data = 0;
	unsigned int timeout = 0;
	unsigned int data_length = 0;
	unsigned int row = 0;
	unsigned int col = 0;
	unsigned int rows = 0;
	unsigned int cols = 0;
	unsigned int frame_size_words = 0;
	unsigned char *temp_buf = NULL;
	char testresult = CAP_TEST_PASS_CHAR;
	char failedreason[TS_RAWDATA_FAILED_REASON_LEN] = {0};
	int raw_data_size= 0;
	int raw_data_max = 0;
	int raw_data_min = 0;
	int raw_data_avg = 0;
	int retry = READ_REPORT_RETRY_TIMES;

	struct syna_tcm_app_info *app_info = NULL;
	struct syna_tcm_hcd *tcm_hcd = test_hcd->tcm_hcd;
	struct ts_rawdata_newnodeinfo* pts_node = NULL;
	struct syna_tcm_test_threshold *threshold = &test_params->threshold;

	TS_LOG_INFO("%s start", __func__);

	if (tcm_hcd->id_info.mode != MODE_APPLICATION ||
			tcm_hcd->app_status != APP_STATUS_OK) {
		TS_LOG_ERR("%s Application firmware not running\n", __func__);
		return -ENODEV;
	}

	app_info = &tcm_hcd->app_info;
	rows = le2_to_uint(app_info->num_of_image_rows);
	cols = le2_to_uint(app_info->num_of_image_cols);
	raw_data_size = rows * cols;

	pts_node = (struct ts_rawdata_newnodeinfo *)kzalloc(sizeof(struct ts_rawdata_newnodeinfo), GFP_KERNEL);
	if (!pts_node) {
		TS_LOG_ERR("malloc pts_node failed\n");
		return -ENOMEM;
	}

	pts_node->values = kzalloc(raw_data_size*sizeof(int), GFP_KERNEL);
	if (!pts_node->values) {
		TS_LOG_ERR("%s malloc value failed  for values\n", __func__);
		testresult = CAP_TEST_FAIL_CHAR;
		strncpy(failedreason, "-software_reason", TS_RAWDATA_FAILED_REASON_LEN-1);
		goto exit;
	}

	temp_buf = kzalloc(((rows * cols + MESSAGE_HEADER_SIZE) * 2), GFP_KERNEL);
	if (!temp_buf) {
		TS_LOG_ERR("%s Failed to allocate memory for temp_buf\n", __func__);
		testresult = CAP_TEST_FAIL_CHAR;
		strncpy(failedreason, "-software_reason", TS_RAWDATA_FAILED_REASON_LEN-1);
		retval = -ENOMEM;
		goto exit;
	}

	retval = tskit_driver_alloc_mem(tcm_hcd,
			&test_hcd->out,
			1);
	if (retval < 0) {
		TS_LOG_ERR("Failed to allocate memory for testing_hcd->out.buf\n");
		goto exit;
	}

	test_hcd->out.buf[0] = TEST_FULL_RAW_CAP;

	retval = tskit_driver_write_hdl_message(tcm_hcd,
			CMD_PRODUCTION_TEST,
			test_hcd->out.buf,
			1,
			&test_hcd->resp.buf,
			&test_hcd->resp.buf_size,
			&test_hcd->resp.data_length,
			NULL,
			0);
	if (retval < 0) {
		TS_LOG_ERR("Failed to write command %s\n", STR(CMD_PRODUCTION_TEST));
		goto exit;
	}

	while(retry) {
		timeout = REPORT_TIMEOUT_MS_1;   //REPORT_TIMEOUT_MS
		msleep(timeout);
		retval = tskit_driver_read(tcm_hcd,
				temp_buf,
				((rows * cols + MESSAGE_HEADER_SIZE) * 2));
		if (retval < 0) {
			TS_LOG_ERR("Failed to read raw data\n");
			goto exit;
		}

		if (temp_buf[0] != MESSAGE_MARKER) {
			TS_LOG_ERR("incorrect Header Marker!");
			retval = -EINVAL;
			goto exit;
		}

		if (temp_buf[1] != STATUS_OK) {
			TS_LOG_INFO("status = 0x%02x", temp_buf[1]);
		}else{
			break;
		}
		retry--;
	}

	data_length = temp_buf[2] | temp_buf[3] << 8;
	TS_LOG_INFO(" data_length =%d\n", data_length);

	tskit_driver_get_frame_size_words(&frame_size_words, true);
	if (frame_size_words != data_length / 2) {
		TS_LOG_ERR("Frame size mismatch\n");
		retval = -EINVAL;
		goto exit;
	}
	idx = 0;
	raw_data_max = raw_data_min = (int)le2_to_uint(&temp_buf[MESSAGE_HEADER_SIZE]);
	for (row = 0; row < rows; row++) {
		for (col = 0; col < cols; col++) {
			data = (short)le2_to_uint(&temp_buf[idx * 2 + MESSAGE_HEADER_SIZE]);
			pts_node->values[idx] = (int)data;
			raw_data_max = data > raw_data_max ? data : raw_data_max;
			raw_data_min = data < raw_data_min ? data : raw_data_min;
			raw_data_avg += data;
			idx++;
		}
	}
	if (raw_data_size)
		raw_data_avg /= raw_data_size;

	idx = 0;
	test_hcd->result = true;

	for (row = 0; row < rows; row++) {
		for (col = 0; col < cols; col++) {
			data = (short)le2_to_uint(&temp_buf[idx * 2 + MESSAGE_HEADER_SIZE]);
			if (data > threshold->raw_data_max_limits[idx] ||
				data < threshold->raw_data_min_limits[idx]) {
				TS_LOG_ERR("%s overlow_data = %d, row = %d, col = %d, limits [%d,%d]\n", __func__, data, row, col, threshold->raw_data_max_limits[idx], threshold->raw_data_min_limits[idx]);
				test_hcd->result = false;
				testresult = CAP_TEST_FAIL_CHAR;
				strncpy(failedreason, "-panel_reason", TS_RAWDATA_FAILED_REASON_LEN-1);
			}
			idx++;
		}
	}
	retval = tskit_driver_delta_test(info, pts_node->values);
	if (retval)
		TS_LOG_INFO("%s tskit_driver_delta_test FAILED = %d\n",
			__func__, retval);
exit:
	TS_LOG_INFO("%s end retval = %d\n", __func__, retval);
	pts_node->size = raw_data_size;
	pts_node->testresult = testresult;
	pts_node->typeindex = RAW_DATA_TYPE_CAPRAWDATA;
	strncpy(pts_node->test_name, "Cap_Rawdata", TS_RAWDATA_TEST_NAME_LEN-1);
	snprintf(pts_node->statistics_data, TS_RAWDATA_STATISTICS_DATA_LEN,
			"[%d,%d,%d]",
			raw_data_min, raw_data_max, raw_data_avg);
	if (CAP_TEST_FAIL_CHAR == testresult) {
		strncpy(pts_node->tptestfailedreason, failedreason, TS_RAWDATA_FAILED_REASON_LEN-1);
	}
	list_add_tail(&pts_node->node, &info->rawdata_head);

	test_hcd->report_type = 0;
	if(temp_buf) {
		kfree(temp_buf);
		temp_buf = NULL;
	}
	return retval;
}

static int tskit_driver_abs_raw_cap_test(struct ts_rawdata_info_new *info)
{
	int retval = 0;
	unsigned int idx = 0;
	unsigned int timeout = 0;
	unsigned int data_length = 0;
	unsigned int row = 0;
	unsigned int col = 0;
	unsigned int rows = 0;
	unsigned int cols = 0;
	unsigned char *temp_buf = NULL;
	int *temp_buf_row_int_p= NULL;
	int *temp_buf_col_int_p = NULL;
	char testresult = CAP_TEST_PASS_CHAR;
	char failedreason[TS_RAWDATA_FAILED_REASON_LEN] = {0};
	int noise_data_max = 0;
	int noise_data_min = 0;
	int noise_data_avg = 0;
	int abs_raw_data_size = 0;
	int retry = READ_REPORT_RETRY_TIMES;

	struct syna_tcm_app_info *app_info = NULL;
	struct syna_tcm_hcd *tcm_hcd = test_hcd->tcm_hcd;
	struct ts_rawdata_newnodeinfo* pts_node = NULL;
	struct syna_tcm_test_threshold *threshold = &test_params->threshold;

	TS_LOG_INFO("%s start", __func__);

	if (tcm_hcd->id_info.mode != MODE_APPLICATION ||
			tcm_hcd->app_status != APP_STATUS_OK) {
		TS_LOG_ERR("%s Application firmware not running\n", __func__);
		return -ENODEV;
	}

	app_info = &tcm_hcd->app_info;
	rows = le2_to_uint(app_info->num_of_image_rows);
	cols = le2_to_uint(app_info->num_of_image_cols);
	abs_raw_data_size = rows + cols;

	pts_node = (struct ts_rawdata_newnodeinfo *)kzalloc(sizeof(struct ts_rawdata_newnodeinfo), GFP_KERNEL);
	if (!pts_node) {
		TS_LOG_ERR("malloc pts_node failed\n");
		return -ENOMEM;
	}

	pts_node->values = kzalloc(abs_raw_data_size*sizeof(int), GFP_KERNEL);
	if (!pts_node->values) {
		TS_LOG_ERR("%s malloc value failed  for values\n", __func__);
		testresult = CAP_TEST_FAIL_CHAR;
		strncpy(failedreason, "-software_reason", TS_RAWDATA_FAILED_REASON_LEN-1);
		goto exit;
	}
	temp_buf_row_int_p = kzalloc(rows*sizeof(int), GFP_KERNEL);
	if (!temp_buf_row_int_p) {
		TS_LOG_ERR("%s malloc temp_buf_row_int_p failed  for values\n", __func__);
		testresult = CAP_TEST_FAIL_CHAR;
		retval = -ENOMEM;
		strncpy(failedreason, "-software_reason", TS_RAWDATA_FAILED_REASON_LEN-1);
		goto exit;
	}
	temp_buf_col_int_p = kzalloc(cols*sizeof(int), GFP_KERNEL);
	if (!temp_buf_col_int_p) {
		TS_LOG_ERR("%s malloc temp_buf_col_int_p failed  for values\n", __func__);
		testresult = CAP_TEST_FAIL_CHAR;
		retval = -ENOMEM;
		strncpy(failedreason, "-software_reason", TS_RAWDATA_FAILED_REASON_LEN-1);
		goto exit;
	}

	temp_buf = kzalloc(((rows + cols) * 4 + MESSAGE_HEADER_SIZE + 1), GFP_KERNEL);
	if (!temp_buf) {
		TS_LOG_ERR("%s Failed to allocate memory for temp_buf\n", __func__);
		testresult = CAP_TEST_FAIL_CHAR;
		strncpy(failedreason, "-software_reason", TS_RAWDATA_FAILED_REASON_LEN-1);
		retval = -ENOMEM;
		goto exit;
	}

	retval = tskit_driver_alloc_mem(tcm_hcd,
			&test_hcd->out,
			1);
	if (retval < 0) {
		TS_LOG_ERR("Failed to allocate memory for test_hcd->out.buf\n");
		goto exit;
	}

	test_hcd->out.buf[0] = TEST_PT18;

	retval = tskit_driver_write_hdl_message(tcm_hcd,
			CMD_PRODUCTION_TEST,
			test_hcd->out.buf,
			1,
			&test_hcd->resp.buf,
			&test_hcd->resp.buf_size,
			&test_hcd->resp.data_length,
			NULL,
			0);
	if (retval < 0) {
		TS_LOG_ERR(
				"Failed to write command %s\n",
				STR(CMD_PRODUCTION_TEST));
		goto exit;
	}

	while(retry) {
		timeout = REPORT_TIMEOUT_MS_1;   //REPORT_TIMEOUT_MS
		msleep(timeout);
		retval = tskit_driver_read(tcm_hcd,
				temp_buf,
				((rows + cols) * 4 + MESSAGE_HEADER_SIZE + 1));
		if (retval < 0) {
			TS_LOG_ERR("Failed to read raw data\n");
			goto exit;
		}

		if (temp_buf[0] != MESSAGE_MARKER) {
			TS_LOG_ERR("incorrect Header Marker!");
			retval = -EINVAL;
			goto exit;
		}

		if (temp_buf[1] != STATUS_OK) {
			TS_LOG_INFO("status = 0x%02x", temp_buf[1]);
		}else{
			break;
		}
		retry--;
	}

	data_length = temp_buf[2] | temp_buf[3] << 8;
	TS_LOG_INFO(" data_length =%d\n", data_length);

	if ((temp_buf[1] != STATUS_OK) || (data_length != ((rows + cols) * 4))) {
		TS_LOG_ERR("not correct status or data_length status:%x  length:%x\n", temp_buf[1], data_length);
		retval = -EINVAL;
		goto exit;
	}

	idx = MESSAGE_HEADER_SIZE;
	test_hcd->result = true;

	for (col = 0; col < cols; col++) {
		temp_buf_col_int_p[col] = (unsigned int)(temp_buf[idx] & 0xff) | (unsigned int)(temp_buf[idx+1] << 8) |
					(unsigned int)(temp_buf[idx+2] << 16) | (unsigned int)(temp_buf[idx+3] << 24);
			TS_LOG_DEBUG("col=%-2d data = %d , limit = [%d, %d]\n",
					col, temp_buf_col_int_p[col],
					threshold->raw_data_col_min_limits[col],
					threshold->raw_data_col_max_limits[col]);
		if (temp_buf_col_int_p[col] < threshold->raw_data_col_min_limits[col]
				|| temp_buf_col_int_p[col] > threshold->raw_data_col_max_limits[col]) {
			test_hcd->result = false;
			testresult = CAP_TEST_FAIL_CHAR;
			strncpy(failedreason, "-panel_reason", TS_RAWDATA_FAILED_REASON_LEN-1);
			retval = -EINVAL;
			TS_LOG_ERR("fail at col=%-2d data = %d (0x%2x 0x%2x 0x%2x 0x%2x), limit = [%d,%d]\n",
					col, temp_buf_col_int_p[col],
					temp_buf[idx], temp_buf[idx+1], temp_buf[idx+2], temp_buf[idx+3],
					threshold->raw_data_col_min_limits[col],
					threshold->raw_data_col_max_limits[col]);
		}
		idx+=4;
	}

	for (row = 0; row < rows; row++) {
		temp_buf_row_int_p[row] = (unsigned int)(temp_buf[idx] & 0xff) | (unsigned int)(temp_buf[idx+1] << 8) |
					(unsigned int)(temp_buf[idx+2] << 16) | (unsigned int)(temp_buf[idx+3] << 24);
			TS_LOG_DEBUG("row=%-2d data = %d, limit = [%d, %d]\n",
					row, temp_buf_row_int_p[row],
					threshold->raw_data_row_min_limits[row],
					threshold->raw_data_row_max_limits[row]);
		if (temp_buf_row_int_p[row] < threshold->raw_data_row_min_limits[row]
				|| temp_buf_row_int_p[row] > threshold->raw_data_row_max_limits[row]) {
			test_hcd->result = false;
			testresult = CAP_TEST_FAIL_CHAR;
			strncpy(failedreason, "-panel_reason", TS_RAWDATA_FAILED_REASON_LEN-1);
			retval = -EINVAL;
			TS_LOG_ERR("fail at row=%-2d data = %d (0x%2x 0x%2x 0x%2x 0x%2x), limit = [%d, %d]\n",
					row, temp_buf_row_int_p[row],
					temp_buf[idx], temp_buf[idx+1], temp_buf[idx+2], temp_buf[idx+3],
					threshold->raw_data_row_min_limits[row],
					threshold->raw_data_row_max_limits[row]);
		}
		idx+=4;
	}

	memcpy(pts_node->values, temp_buf_row_int_p, rows*sizeof(int));
	memcpy(&(pts_node->values[rows]), temp_buf_col_int_p, cols*sizeof(int));

exit:
	TS_LOG_INFO("%s end retval = %d", __func__, retval);

	pts_node->size = abs_raw_data_size;
	pts_node->testresult = testresult;
	pts_node->typeindex = RAW_DATA_TYPE_SELFCAP;
	strncpy(pts_node->test_name, "abs raw data", TS_RAWDATA_TEST_NAME_LEN-1);
	snprintf(pts_node->statistics_data, TS_RAWDATA_STATISTICS_DATA_LEN,
			"[%d,%d,%d]",
			noise_data_min, noise_data_max, noise_data_avg);
	if (CAP_TEST_FAIL_CHAR == testresult) {
		strncpy(pts_node->tptestfailedreason, failedreason, TS_RAWDATA_FAILED_REASON_LEN-1);
	}
	list_add_tail(&pts_node->node, &info->rawdata_head);

	if(temp_buf) {
		kfree(temp_buf);
		temp_buf = NULL;
	}
	if(temp_buf_row_int_p) {
		kfree(temp_buf_row_int_p);
		temp_buf_row_int_p = NULL;
	}
	if(temp_buf_col_int_p) {
		kfree(temp_buf_col_int_p);
		temp_buf_col_int_p = NULL;
	}
	return retval;
}

int tskit_driver_cap_test(struct ts_rawdata_info_new *info,
			struct ts_cmd_node *out_cmd)
{
	int retval = NO_ERR;
	unsigned int rows = 0;
	unsigned int cols = 0;
	struct syna_tcm_hcd *tcm_hcd = test_hcd->tcm_hcd;
	struct syna_tcm_app_info *app_info = NULL;
	struct ts_rawdata_newnodeinfo* pts_node = NULL;

	TS_LOG_INFO("cap test called\n");

	test_hcd->tcm_hcd = tcm_hcd;
	app_info = &tcm_hcd->app_info;
	rows = le2_to_uint(app_info->num_of_image_rows);
	cols = le2_to_uint(app_info->num_of_image_cols);
	if(0 == rows || 0 == cols) {
		TS_LOG_ERR("rows or cols is NULL\n");
		return -ENOMEM;
	}

	if (tcm_hcd->bdata && tcm_hcd->bdata->support_vendor_ic_type == S3909)
		return touch_driver_cap_test_for_newformat(info);
	if (!test_params->cap_thr_is_parsed)
		touch_driver_get_thr_from_csvfile();

	pts_node = (struct ts_rawdata_newnodeinfo *)kzalloc(sizeof(struct ts_rawdata_newnodeinfo), GFP_KERNEL);
	if (!pts_node) {
		TS_LOG_ERR("malloc failed\n");
		return -ENOMEM;
	}

	if (tcm_hcd->id_info.mode != MODE_APPLICATION ||
			tcm_hcd->app_status != APP_STATUS_OK) {
		pts_node->typeindex = RAW_DATA_TYPE_IC;
		pts_node->testresult = CAP_TEST_FAIL_CHAR;
		list_add_tail(&pts_node->node, &info->rawdata_head);
		return -ENODEV;
	} else {
		pts_node->typeindex = RAW_DATA_TYPE_IC;
		pts_node->testresult = CAP_TEST_PASS_CHAR;
		list_add_tail(&pts_node->node, &info->rawdata_head);
	}

	tskit_driver_put_device_info(info);

	tskit_driver_enable_touch(tcm_hcd, false);

	TS_LOG_INFO("%s: before disable doze\n", __func__);
	retval = test_hcd->tcm_hcd->set_dynamic_config(test_hcd->tcm_hcd,
			DC_NO_DOZE, 1);
	if (retval < 0) {
		TS_LOG_ERR("%s failed to set dc\n", __func__);
	}
	retval = tskit_driver_noise_test(info);

	if (retval < 0) {
		TS_LOG_ERR("%s lcd noise testing failed.\n", __func__);
	}
	info->listnodenum++;
	if (tcm_hcd->htd == true) {
		retval = tskit_driver_dynamic_range_test(info);
	} else {
		retval = tskit_driver_full_raw_cap_test(info);
	}
	if (retval < 0) {
		TS_LOG_ERR("%s raw data testing failed.\n", __func__);
	}
	info->listnodenum++;
	if (tcm_hcd->htd == true) {
		/* do nothing */
	} else {
		retval = tskit_driver_abs_raw_cap_test(info);
		if (retval < 0) {
			TS_LOG_ERR("%s raw data testing failed.\n", __func__);
		}
		info->listnodenum++;
	}

	if (tcm_hcd->htd == true) {
		retval = tskit_driver_open_short_test(info);
		if (retval < 0) {
			TS_LOG_ERR("%s open short testing failed.\n", __func__);
		}
	} else {
		if (tcm_hcd->syna_tcm_chip_data->ts_platform_data->bops->btype == TS_BUS_SPI) {
			retval = tskit_driver_trx_trx_short_test(info);
			if (retval < 0)
				TS_LOG_ERR("%s fail trx_trx_short", __func__);
		}
		/* retval = testing_discrete_ex_trx_short(info); */
		/* if (retval < 0) {
			TS_LOG_ERR("%s fail to do testing_discrete_ex_trx_short", __func__);
		} */
	}
	info->listnodenum++;

	retval = test_hcd->tcm_hcd->set_dynamic_config(test_hcd->tcm_hcd,
			DC_NO_DOZE, 0);
	if (retval < 0) {
		TS_LOG_ERR("%s failed to set dc\n", __func__);
	}
	TS_LOG_INFO("%s: after enable doze\n", __func__);

	tskit_driver_enable_touch(tcm_hcd, true);

	info->tx = cols;
	info->rx = rows;

	if (tcm_hcd->htd == false) {
		gpio_set_value(tcm_hcd->syna_tcm_chip_data->ts_platform_data->reset_gpio, 0);
		msleep(10);
		gpio_set_value(tcm_hcd->syna_tcm_chip_data->ts_platform_data->reset_gpio, 1);
	}

	return 0;
}
static void judge_pin(unsigned char *trx, int py_pin, bool *flag,
		int *pt_flag, short rx, short *rxp)
{
	int i = 0;
	unsigned char *g_trx_result = trx;
	int py_p = py_pin;
	bool *is_rx = flag;
	int *failure_cnt_pt1 = pt_flag;
	short rx_assigned = rx;
	short *rx_pins = rxp;

	if ((py_p == 0) || (py_p == 1) ||
		(py_p == SYNA_PIN_32) || (py_p == SYNA_PIN_33)) {
		for (i = 0; i < rx_assigned; i++) {
			*is_rx = false;
			if (py_p == (int)rx_pins[i]) {
				*is_rx = true;
				break;
			}
		}
		if (*is_rx) {
			TS_LOG_ERR("pin-%2d : is rx\n", py_p);
		} else {
			TS_LOG_ERR("pin-%2d : fail byte %d\n", py_p, i);
			g_trx_result[py_p] = 1;
			*failure_cnt_pt1 += 1;
		}
	} else {
		TS_LOG_ERR("pin-%2d : fail byte %d\n", py_p, i);
		g_trx_result[py_p] = 1;
		*failure_cnt_pt1 += 1;
	}
}

static bool testing_get_pins_assigned(unsigned short pin,
			short *tx_pins, short tx_assigned, short *rx_pins,
		short rx_assigned, short *g_pins, short guard_assigned)
{
	int i;

	for (i = 0; i < tx_assigned; i++) {
		if (pin == tx_pins[i])
			return true;
	}
	for (i = 0; i < rx_assigned; i++) {
		if (pin == rx_pins[i])
			return true;
	}
	for (i = 0; i < guard_assigned; i++) {
		if (pin == g_pins[i])
			return true;
	}

	return false;
}

static void testing_pins_mapping_trgm(unsigned char *cfg_data,
				unsigned int cfg_data_len,
				int nu_tx_g, int nu_rx_g, int j)
{
	int i;
	int idx = 0;
	/* unit should be byte */
	int of_rx_g = CFG_RX_GUARD_PINS_OFFSET / 8;
	int len_rx_g = CFG_RX_GUARD_PINS_LENGTH / 8;
	int of_tx_gd = CFG_TX_GUARD_PINS_OFFSET / 8;
	int len_tx_g = CFG_TX_GUARD_PINS_LENGTH / 8;

	/* get tx guards mapping */
	if ((nu_tx_g > 0) && (nu_tx_g <= cfg_data_len / 2) &&
		(cfg_data_len > of_tx_gd + len_tx_g)) {
		idx = 0;
		for (i = 0; i < nu_tx_g; i++) {
			test_hcd->g_pins[j] = (short)cfg_data[of_tx_gd + idx] |
				(short)(cfg_data[of_tx_gd + idx + 1] << 8);
			TS_LOG_ERR("g_ps-%d = %2d\n", i, test_hcd->g_pins[j]);
			idx += 2;
			j += 1;
		}
	}

	/* get rx guards mapping */
	if ((nu_rx_g > 0) && (nu_rx_g < cfg_data_len / 2) &&
		(cfg_data_len > of_rx_g + len_rx_g)) {
		for (i = 0; i < nu_rx_g; i++) {
			test_hcd->g_pins[j] = (short)cfg_data[of_rx_g + idx] |
				(short)(cfg_data[of_rx_g + idx + 1] << 8);
			TS_LOG_ERR("g_ps-%d = %2d\n", i, test_hcd->g_pins[j]);
			idx += 2;
			j += 1;
		}
	}
}

static void testing_pins_mapping_trg(unsigned char *cfg_data,
				unsigned int cfg_data_len)
{
	int j = 0;
	int nu_rx_g = 0;
	/* unit should be byte */
	int of_nu_rx_g = CFG_NUM_RXGUARD_OFFSET / 8;
	int len_nu_rx_g = CFG_NUM_RXGUARD_LENGTH / 8;
	int nu_tx_g = 0;
	int of_nu_tx_g = CFG_NUM_TXGUARD_OFFSET / 8;
	int len_nu_tx_g = CFG_NUM_TXGUARD_LENGTH / 8;

	test_hcd->guard_assigned = 0;

	/* get number of tx guards */
	if (cfg_data_len > of_nu_tx_g + len_nu_tx_g) {
		nu_tx_g = (short)cfg_data[of_nu_tx_g] |
				(short)(cfg_data[of_nu_tx_g + 1] << 8);

		test_hcd->guard_assigned += nu_tx_g;
	}

	/* get number of rx guards */
	if (cfg_data_len > of_nu_rx_g + len_nu_rx_g) {
		nu_rx_g = (short)cfg_data[of_nu_rx_g] |
				(short)(cfg_data[of_nu_rx_g + 1] << 8);

		test_hcd->guard_assigned += nu_rx_g;
	}
	if (test_hcd->guard_assigned > 0) {
		TS_LOG_ERR("num of guards = %2d, tx: %d, rx: %d\n",
			test_hcd->guard_assigned, nu_tx_g, nu_rx_g);
		j = 0;
	}
	testing_pins_mapping_trgm(cfg_data, cfg_data_len,
			nu_tx_g, nu_rx_g, j);
}

static int testing_pins_mapping(unsigned char *cfg_data,
						unsigned int cfg_data_len)
{
	int i;
	int idx = 0;
	/* unit should be byte */
	int of_rx = CFG_IMAGE_RXES_OFFSET / 8;
	int len_rx = CFG_IMAGE_RXES_LENGTH / 8;
	int of_tx = CFG_IMAGE_TXES_OFFSET / 8;
	int len_tx = CFG_IMAGE_TXES_LENGTH / 8;

	if (!cfg_data) {
		TS_LOG_ERR("invalid parameter\n");
		return -EINVAL;
	}

	test_hcd->tx_assigned = 0;
	test_hcd->rx_assigned = 0;

	/* get tx pins mapping */
	if (cfg_data_len > (of_tx + len_tx)) {
		test_hcd->tx_assigned = (len_tx / 2);
		idx = 0;
		for (i = 0; i < (len_tx / 2); i++) {
			test_hcd->tx_pins[i] = (short)cfg_data[of_tx + idx] |
					(short)(cfg_data[of_tx + idx + 1] << 8);
			idx += 2;
			TS_LOG_INFO("i = %d, tx = %2d", i, test_hcd->tx_pins[i]);
		}
	}

	/* get rx pins mapping */
	if (cfg_data_len > of_rx + len_rx) {
		test_hcd->rx_assigned = (len_rx / 2);
		idx = 0;
		for (i = 0; i < (len_rx / 2); i++) {
			test_hcd->rx_pins[i] = (short)cfg_data[of_rx + idx] |
					(short)(cfg_data[of_rx + idx + 1] << 8);
			idx += 2;
			TS_LOG_INFO("i = %d, rx = %2d", i, test_hcd->rx_pins[i]);
		}
	}
	testing_pins_mapping_trg(cfg_data, cfg_data_len);

	return 0;
}

static int testing_get_static_config(unsigned char *buf, unsigned int buf_len)
{
	int retval = NO_ERR;
	struct syna_tcm_hcd *tcm_hcd = test_hcd->tcm_hcd;

	if (!buf) {
		TS_LOG_ERR("invalid parameter\n");
		return -EINVAL;
	}

	retval = tskit_driver_write_hdl_message(tcm_hcd, CMD_GET_STATIC_CONFIG,
			NULL, 0, &test_hcd->resp.buf, &test_hcd->resp.buf_size,
			&test_hcd->resp.data_length, NULL, 0);
	if (retval < 0) {
		TS_LOG_ERR("Failed to write command %s\n",
			STR(CMD_GET_STATIC_CONFIG));
		goto exit;
	}

	msleep(REPORT_TIMEOUT_MS);
	retval = tskit_driver_read(tcm_hcd, g_temp_buf, sizeof(g_temp_buf));

	if (retval < 0) {
		TS_LOG_ERR("Failed to g_temp_buf data\n");
		return retval;
	}

	if ((g_temp_buf[0] != MESSAGE_MARKER) || (g_temp_buf[1] != STATUS_OK)) {
		TS_LOG_ERR("message error %x, %x %x %x\n",
			g_temp_buf[0], g_temp_buf[1],
			g_temp_buf[2], g_temp_buf[3]);
		retval = -EINVAL;
		goto exit;
	}

	TS_LOG_ERR("data length:%d\n", g_temp_buf[2] | (g_temp_buf[3] << 8));

	memcpy(buf, &g_temp_buf[MESSAGE_HEADER_SIZE], buf_len);

exit:
	return retval;
}
static unsigned char g_trx_result[MAX_PINS] = {0};

static int tskit_driver_trx_trx_short_test(struct ts_rawdata_info_new *info)
{
	int retval = NO_ERR;
	int i;
	int j;
	int py_p;
	bool do_pin_test = false;
	bool is_rx = false;
	unsigned char *pt1_data = NULL;
	unsigned int pt1_data_size = 8;
	unsigned char *satic_cfg_buf = NULL;
	unsigned int satic_cfg_length;
	short *tx_pins = test_hcd->tx_pins;
	short tx_assigned;
	short *rx_pins = test_hcd->rx_pins;
	short rx_assigned;
	short *g_pins = test_hcd->g_pins;
	short guard_assigned;
	struct syna_tcm_app_info *app_info = NULL;
	struct syna_tcm_hcd *tcm_hcd = test_hcd->tcm_hcd;
	int failure_cnt_pt1 = 0;
	int data_length;
	int pt1_data_min;
	int pt1_data_max;
	int pt1_data_avg = 0;
	unsigned int num_kzalloc;
	char testresult = CAP_TEST_PASS_CHAR;
	char failedreason[TS_RAWDATA_FAILED_REASON_LEN] = {0};
	struct ts_rawdata_newnodeinfo *pts_node = NULL;

	pts_node = kzalloc(sizeof(*pts_node), GFP_KERNEL);
	if (!pts_node) {
		TS_LOG_ERR("malloc pts_node failed\n");
		return -ENOMEM;
	}

	pts_node->values = kzalloc(MAX_PINS*sizeof(int), GFP_KERNEL);
	if (!pts_node->values) {
		TS_LOG_ERR("%s malloc value failed for values\n", __func__);
		testresult = CAP_TEST_FAIL_CHAR;
		strncpy(failedreason, "-software_reason",
			TS_RAWDATA_FAILED_REASON_LEN - 1);
		goto exit;
	}

	app_info = &tcm_hcd->app_info;

	memset(g_trx_result, 0, sizeof(g_trx_result));

	if (!test_hcd->satic_cfg_buf) {
		satic_cfg_length = le2_to_uint(app_info->static_config_size);

		satic_cfg_buf = kzalloc(satic_cfg_length, GFP_KERNEL);
		if (!satic_cfg_buf) {
			TS_LOG_ERR("Failed Kzalloc satic_cfg_buf\n");
			goto exit;
		}

		retval = testing_get_static_config(satic_cfg_buf,
				satic_cfg_length);
		if (retval < 0) {
			TS_LOG_ERR("Failed to get static config\n");
			goto exit;
		}
		test_hcd->satic_cfg_buf = satic_cfg_buf;
	}

	/* get pins mapping */
	if ((test_hcd->tx_assigned <= 0) ||
		 (test_hcd->rx_assigned <= 0) ||
		 (test_hcd->guard_assigned <= 0)) {

		if (satic_cfg_buf) {
			retval = testing_pins_mapping(satic_cfg_buf,
					satic_cfg_length);
			if (retval < 0) {
				TS_LOG_ERR("Failed to get pins mapping\n");
				goto exit;
			}

			TS_LOG_ERR("tx = %d, rx = %d, guard = %d",
				test_hcd->tx_assigned,
				test_hcd->rx_assigned,
				test_hcd->guard_assigned);
		}
	}

	tx_assigned = test_hcd->tx_assigned;
	rx_assigned = test_hcd->rx_assigned;
	guard_assigned = test_hcd->guard_assigned;

	retval = tskit_driver_alloc_mem(tcm_hcd, &test_hcd->out, 1);
	if (retval < 0) {
		TS_LOG_ERR("Failed to alloc_mem for test_hcd->out.buf\n");
		goto exit;
	}

	test_hcd->out.buf[0] = TEST_TRX_TRX_SHORTS;

	retval = tskit_driver_write_hdl_message(tcm_hcd, CMD_PRODUCTION_TEST,
			test_hcd->out.buf, 1, &test_hcd->resp.buf,
			&test_hcd->resp.buf_size, &test_hcd->resp.data_length,
			NULL, 0);
	if (retval < 0) {
		TS_LOG_ERR("Failed to write command %s\n",
			STR(CMD_PRODUCTION_TEST));
		goto exit;
	}

	msleep(REPORT_TIMEOUT_MS);
	retval = tskit_driver_read(tcm_hcd, g_temp_buf,
			pt1_data_size + MESSAGE_HEADER_SIZE + 1);

	if (retval < 0) {
		TS_LOG_ERR("Failed to read raw data\n");
		goto exit;
	}

	if (g_temp_buf[0] != MESSAGE_MARKER) {
		TS_LOG_ERR("incorrect Header Marker!\n");
		retval = -EINVAL;
		goto exit;

	}

	data_length = g_temp_buf[2] | g_temp_buf[3] << 8;
	TS_LOG_INFO("%d\n", data_length);

	test_hcd->result = true;
	num_kzalloc = pt1_data_size * sizeof(*pt1_data);
	pt1_data = kzalloc(num_kzalloc, GFP_KERNEL);
	if (!pt1_data) {
		TS_LOG_ERR("Failed to allocate mem to pt1_data\n");
		goto exit;
	}
	memcpy(pt1_data, &g_temp_buf[MESSAGE_HEADER_SIZE],
		pt1_data_size);

	pt1_data_max = g_trx_result[0];
	pt1_data_min = g_trx_result[0];

	for (i = 0; i < pt1_data_size; i++) {
		TS_LOG_ERR("i = %d: %2d\n", i, pt1_data[i]);
		pts_node->values[i] = g_trx_result[i];

		if (pt1_data_max < pts_node->values[i])
			pt1_data_max = pts_node->values[i];
		if (pt1_data_min > pts_node->values[i])
			pt1_data_min = pts_node->values[i];

		pt1_data_avg += pts_node->values[i];

		for (j = 0; j < 8; j++) {
			py_p = (i * 8 + j);
			do_pin_test = testing_get_pins_assigned(py_p,
					tx_pins, tx_assigned,
					rx_pins, rx_assigned,
					g_pins, guard_assigned);
			if (do_pin_test) {
				if (CHECK_BIT(pt1_data[i], j) == 0)
					TS_LOG_DEBUG("pin-%2d : pass\n", py_p);
				else /* check pin-0, 1, 32, 33 */
					judge_pin(g_trx_result, py_p,
						&is_rx, &failure_cnt_pt1,
						rx_assigned, rx_pins);
			} /* end if(do_pin_test) */
		}
	}

	if (pt1_data_size)
		pt1_data_avg /= pt1_data_size;
	test_hcd->result = (failure_cnt_pt1 == 0);
	retval = failure_cnt_pt1;
	pts_node->size = MAX_PINS;
	pts_node->testresult = testresult;
	pts_node->typeindex = RAW_DATA_TYPE_OPENSHORT;
	strncpy(pts_node->test_name, "trx-trx short",
			TS_RAWDATA_TEST_NAME_LEN - 1);
	snprintf(pts_node->statistics_data, TS_RAWDATA_STATISTICS_DATA_LEN,
			"[%d,%d,%d]",
			pt1_data_min, pt1_data_max, pt1_data_avg);
	if (testresult == CAP_TEST_FAIL_CHAR)
		strncpy(pts_node->tptestfailedreason, failedreason,
				TS_RAWDATA_FAILED_REASON_LEN - 1);
	list_add_tail(&pts_node->node, &info->rawdata_head);

exit:
	kfree(pt1_data);
	pt1_data = NULL;
	kfree(satic_cfg_buf);
	satic_cfg_buf = NULL;

	return retval;
}

/* RT199 report */
#define REPORT_199  0xC7
static int touch_driver_cap_raw_data_test(
	struct ts_rawdata_info_new *info)
{
	int retval;
	unsigned int idx = 0;
	signed short data;
	unsigned int data_length;
	unsigned int row;
	unsigned int col;
	unsigned int rows;
	unsigned int cols;
	unsigned int frame_size_words = 0;
	unsigned char *temp_buf = NULL;
	unsigned char *resp_buf = NULL;
	char testresult = CAP_TEST_PASS_CHAR;
	char failedreason[TS_RAWDATA_FAILED_REASON_LEN] = {0};
	int cap_raw_data_size;
	int cap_raw_data_max = 0;
	int cap_raw_data_min = 0;
	int cap_raw_data_avg = 0;
	int i;

	struct syna_tcm_app_info *app_info = NULL;
	struct syna_tcm_hcd *tcm_hcd = test_hcd->tcm_hcd;
	struct ts_rawdata_newnodeinfo *pts_node = NULL;
	struct syna_tcm_test_threshold *threshold = &test_params->threshold;

	TS_LOG_INFO("%s start", __func__);

	if (tcm_hcd->id_info.mode != MODE_APPLICATION ||
			tcm_hcd->app_status != APP_STATUS_OK) {
		TS_LOG_ERR("%s Application firmware not running\n", __func__);
		return -ENODEV;
	}

	app_info = &tcm_hcd->app_info;
	rows = le2_to_uint(app_info->num_of_image_rows);
	cols = le2_to_uint(app_info->num_of_image_cols);
	cap_raw_data_size = rows * cols;

	pts_node = kzalloc(sizeof(struct ts_rawdata_newnodeinfo), GFP_KERNEL);
	if (!pts_node) {
		TS_LOG_ERR("malloc pts_node failed\n");
		goto pts_node_zalloc_fail;
	}

	pts_node->values = kzalloc(cap_raw_data_size * sizeof(int), GFP_KERNEL);
	if (!pts_node->values) {
		TS_LOG_ERR("%s malloc value failed  for values\n", __func__);
		testresult = CAP_TEST_FAIL_CHAR;
		strncpy(failedreason, "-software_reason",
			TS_RAWDATA_FAILED_REASON_LEN - 1);
		goto pts_node_value_zalloc_fail;
	}

	temp_buf = kzalloc(MAX_FRAME_BUFFER, GFP_KERNEL);
	if (!temp_buf) {
		TS_LOG_ERR("%s Failed to allocate memory for temp_buf\n",
			__func__);
		testresult = CAP_TEST_FAIL_CHAR;
		strncpy(failedreason, "-software_reason",
			TS_RAWDATA_FAILED_REASON_LEN - 1);
		goto tmp_buf_zalloc_fail;
	}

	resp_buf = kzalloc(MAX_FRAME_BUFFER, GFP_KERNEL);
	if (!resp_buf) {
		TS_LOG_ERR("%s Failed to allocate memory for resp_buf\n",
			__func__);
		testresult = CAP_TEST_FAIL_CHAR;
		strncpy(failedreason, "-software_reason",
			TS_RAWDATA_FAILED_REASON_LEN - 1);
		goto resp_buf_zalloc_fail;
	}

	retval = tskit_driver_alloc_mem(tcm_hcd, &test_hcd->out, 1);
	if (retval < 0) {
		TS_LOG_ERR("Failed to allocate memory for testing_hcd->out.buf\n");
		goto tcm_buf_zalloc_fail;
	}

	/* enable RT199 report */
	test_hcd->out.buf[0] = REPORT_199;
	retval = tskit_driver_write_hdl_message(tcm_hcd, CMD_ENABLE_REPORT,
			test_hcd->out.buf, 1, &test_hcd->resp.buf,
			&test_hcd->resp.buf_size, &test_hcd->resp.data_length,
			NULL, 0);
	if (retval < 0) {
		TS_LOG_ERR("Failed to write command %s\n",
			STR(CMD_ENABLE_REPORT));
		goto exit;
	}

	retval = touch_driver_testing_read(tcm_hcd, resp_buf, MAX_FRAME_BUFFER);
	if ((retval < 0) || (resp_buf[1] != STATUS_OK)) {
		TS_LOG_ERR("status = 0x%02x", resp_buf[1]);
		TS_LOG_ERR("Failed to enable RT199 report\n");
		testresult = CAP_TEST_FAIL_CHAR;
		strncpy(failedreason, "-software_reason",
			TS_RAWDATA_FAILED_REASON_LEN - 1);
		goto exit;
	} else {
		msleep(150); /* 150:wait for RT199 report 150ms*/
		retval = touch_driver_testing_read(tcm_hcd, temp_buf,
			MAX_FRAME_BUFFER);
		if ((retval < 0) || (temp_buf[1] != REPORT_199)) {
			TS_LOG_INFO("status = 0x%02x", temp_buf[1]);
			TS_LOG_ERR("Failed to read RT199 data\n");
			testresult = CAP_TEST_FAIL_CHAR;
			strncpy(failedreason, "-software_reason",
				TS_RAWDATA_FAILED_REASON_LEN - 1);
			goto exit;
		}

	}

	/* disable RT199 report */
	test_hcd->out.buf[0] = REPORT_199;
	retval = tskit_driver_write_hdl_message(tcm_hcd, CMD_DISABLE_REPORT,
		test_hcd->out.buf, 1, &test_hcd->resp.buf,
		&test_hcd->resp.buf_size, &test_hcd->resp.data_length,
		NULL, 0);
	if (retval < 0) {
		TS_LOG_ERR("Failed to write command %s\n",
			STR(CMD_DISABLE_REPORT));
		goto exit;
	}
	retval = touch_driver_testing_read(tcm_hcd, resp_buf, MAX_FRAME_BUFFER);
	if ((retval < 0) || (resp_buf[1] != STATUS_OK)) {
		if (resp_buf[1] == REPORT_199) {
			for (i = 0; i < 10; i++) { /* 10: testing read retry 10 times max */
				retval = touch_driver_testing_read(tcm_hcd,
					resp_buf, MAX_FRAME_BUFFER);
				if (retval == 0 || resp_buf[1] == STATUS_OK)
					break;
			}
		} else {
			TS_LOG_ERR("status = 0x%02x", resp_buf[1]);
			TS_LOG_ERR("Failed to disable RT199 report\n");
			testresult = CAP_TEST_FAIL_CHAR;
			strncpy(failedreason, "-software_reason",
			TS_RAWDATA_FAILED_REASON_LEN - 1);
			goto exit;
		}
	}

	data_length = temp_buf[2] | (temp_buf[3] << 8);
	TS_LOG_INFO("data_length =%d\n", data_length);

	tskit_driver_get_frame_size_words(&frame_size_words, true);
	/* 108: here is from (16+38)*2 for abs data */
	if (frame_size_words != (data_length - 108) / 2) {
		TS_LOG_ERR("Cap frame size mismatch\n");
		retval = -EINVAL;
		goto exit;
	}
	cap_raw_data_max = cap_raw_data_min =
		(int)le2_to_uint(&temp_buf[MESSAGE_HEADER_SIZE]);
	for (row = 0; row < rows; row++) {
		for (col = 0; col < cols; col++) {
			data = (short)le2_to_uint(&temp_buf[idx * 2 +
				MESSAGE_HEADER_SIZE]);
			pts_node->values[idx] = (int)data;
			cap_raw_data_max = data > cap_raw_data_max ? data :
				cap_raw_data_max;
			cap_raw_data_min = data < cap_raw_data_min ? data :
				cap_raw_data_min;
			cap_raw_data_avg += data;
			idx++;
		}
	}
	if (cap_raw_data_size)
		cap_raw_data_avg /= cap_raw_data_size;

	idx = 0;
	test_hcd->result = true;

	for (row = 0; row < rows; row++) {
		for (col = 0; col < cols; col++) {
			data = (short)le2_to_uint(&temp_buf[idx * 2 +
				MESSAGE_HEADER_SIZE]);
			if (data > threshold->cap_raw_data_max_limits[idx] ||
				data < threshold->cap_raw_data_min_limits[idx]) {
				TS_LOG_ERR("%s overlow_data = %d, row = %d, col = %d, limits [%d,%d]\n",
					__func__, data, row, col,
					threshold->cap_raw_data_max_limits[idx],
					threshold->cap_raw_data_min_limits[idx]);
				test_hcd->result = false;
				testresult = CAP_TEST_FAIL_CHAR;
				strncpy(failedreason, "-panel_reason",
					TS_RAWDATA_FAILED_REASON_LEN-1);
			}
			idx++;
		}
	}
exit:
	TS_LOG_INFO("%s end retval = %d 1%c\n", __func__, retval, testresult);
	pts_node->size = cap_raw_data_size;
	pts_node->testresult = testresult;
	pts_node->typeindex = RAW_DATA_TYPE_CAPRAWDATA;
	strncpy(pts_node->test_name, "Cap_Rawdata",
		TS_RAWDATA_TEST_NAME_LEN - 1);
	snprintf(pts_node->statistics_data, TS_RAWDATA_STATISTICS_DATA_LEN,
		"[%d,%d,%d]",
		cap_raw_data_min, cap_raw_data_max, cap_raw_data_avg);
	if (testresult == CAP_TEST_FAIL_CHAR)
		strncpy(pts_node->tptestfailedreason, failedreason,
			TS_RAWDATA_FAILED_REASON_LEN - 1);
	list_add_tail(&pts_node->node, &info->rawdata_head);

	if (retval >= 0) {
		retval = touch_driver_delta_test(info, pts_node->values);
		if (retval)
			TS_LOG_INFO("delta_test failed = %d\n", retval);
	}
	test_hcd->report_type = 0;
	kfree(temp_buf);
	temp_buf = NULL;
	kfree(resp_buf);
	resp_buf = NULL;
	return retval;
tcm_buf_zalloc_fail:
	kfree(resp_buf);
	resp_buf = NULL;
resp_buf_zalloc_fail:
	kfree(temp_buf);
	temp_buf = NULL;
tmp_buf_zalloc_fail:
	kfree(pts_node->values);
	pts_node->values = NULL;
pts_node_value_zalloc_fail:
	kfree(pts_node);
	pts_node = NULL;
pts_node_zalloc_fail:
	return -ENOMEM;
}


static int touch_driver_full_raw_cap_test(
	struct ts_rawdata_info_new *info)
{
	int retval;
	unsigned int idx;
	signed short data;
	unsigned int data_length;
	unsigned int row;
	unsigned int col;
	unsigned int rows;
	unsigned int cols;
	unsigned int frame_size_words;
	unsigned char *temp_buf = NULL;
	char testresult = CAP_TEST_PASS_CHAR;
	char failedreason[TS_RAWDATA_FAILED_REASON_LEN] = {0};
	int raw_data_size;
	int raw_data_max;
	int raw_data_min;
	int raw_data_avg = 0;
	struct syna_tcm_app_info *app_info = NULL;
	struct syna_tcm_hcd *tcm_hcd = test_hcd->tcm_hcd;
	struct ts_rawdata_newnodeinfo *pts_node = NULL;
	struct syna_tcm_test_threshold *threshold = &test_params->threshold;

	TS_LOG_INFO("%s start", __func__);

	if (tcm_hcd->id_info.mode != MODE_APPLICATION ||
			tcm_hcd->app_status != APP_STATUS_OK) {
		TS_LOG_ERR("%s Application firmware not running\n", __func__);
		return -ENODEV;
	}

	app_info = &tcm_hcd->app_info;
	rows = le2_to_uint(app_info->num_of_image_rows);
	cols = le2_to_uint(app_info->num_of_image_cols);
	raw_data_size = rows * cols;

	pts_node = kzalloc(sizeof(struct ts_rawdata_newnodeinfo), GFP_KERNEL);
	if (!pts_node) {
		TS_LOG_ERR("malloc pts_node failed\n");
		goto pts_node_zalloc_fail;
	}

	pts_node->values = kzalloc(raw_data_size*sizeof(int), GFP_KERNEL);
	if (!pts_node->values) {
		TS_LOG_ERR("%s malloc value failed  for values\n", __func__);
		goto pts_node_value_zalloc_fail;
	}

	temp_buf = kzalloc(MAX_FRAME_BUFFER, GFP_KERNEL);
	if (!temp_buf) {
		TS_LOG_ERR("%s Failed to allocate memory for temp_buf\n",
			__func__);
		goto tmp_buf_zalloc_fail;
	}

	retval = tskit_driver_alloc_mem(tcm_hcd, &test_hcd->out, 1);
	if (retval < 0) {
		TS_LOG_ERR("Failed to allocate memory for testing_hcd->out.buf\n");
		goto tcm_buf_zalloc_fail;
	}

	test_hcd->out.buf[0] = TEST_FULL_RAW_CAP;

	retval = tskit_driver_write_hdl_message(tcm_hcd, CMD_PRODUCTION_TEST,
		test_hcd->out.buf, 1, &test_hcd->resp.buf,
		&test_hcd->resp.buf_size, &test_hcd->resp.data_length, NULL, 0);
	if (retval < 0) {
		TS_LOG_ERR("Failed to write command %s\n",
			STR(CMD_PRODUCTION_TEST));
		testresult = CAP_TEST_FAIL_CHAR;
		strncpy(failedreason, "-software_reason",
			TS_RAWDATA_FAILED_REASON_LEN - 1);
		goto exit;
	}

	retval = touch_driver_testing_read(tcm_hcd, temp_buf, MAX_FRAME_BUFFER);
	if ((retval < 0) || (temp_buf[1] != STATUS_OK)) {
		TS_LOG_ERR("status = 0x%02x", temp_buf[1]);
		TS_LOG_ERR("Failed to read raw data\n");
		testresult = CAP_TEST_FAIL_CHAR;
		strncpy(failedreason, "-software_reason",
			TS_RAWDATA_FAILED_REASON_LEN - 1);
		goto exit;
	}

	data_length = temp_buf[2] | temp_buf[3] << 8;
	TS_LOG_INFO("data_length =%d\n", data_length);

	tskit_driver_get_frame_size_words(&frame_size_words, true);
	if (frame_size_words != data_length / 2) {
		TS_LOG_ERR("Frame size mismatch\n");
		retval = -EINVAL;
		testresult = CAP_TEST_FAIL_CHAR;
		strncpy(failedreason, "-software_reason",
			TS_RAWDATA_FAILED_REASON_LEN - 1);
		goto exit;
	}
	idx = 0;
	raw_data_max = raw_data_min = (int)le2_to_uint(
		&temp_buf[MESSAGE_HEADER_SIZE]);
	for (row = 0; row < rows; row++) {
		for (col = 0; col < cols; col++) {
			data = (short)le2_to_uint(
				&temp_buf[idx * 2 + MESSAGE_HEADER_SIZE]);
			pts_node->values[idx] = (int)data;
			raw_data_max = data > raw_data_max ?
				data : raw_data_max;
			raw_data_min = data < raw_data_min ?
				data : raw_data_min;
			raw_data_avg += data;
			idx++;
		}
	}
	if (raw_data_size)
		raw_data_avg /= raw_data_size;

	idx = 0;
	test_hcd->result = true;

	for (row = 0; row < rows; row++) {
		for (col = 0; col < cols; col++) {
			data = (short)le2_to_uint(
				&temp_buf[idx * 2 + MESSAGE_HEADER_SIZE]);
			if (data > threshold->raw_data_max_limits[idx] ||
				data < threshold->raw_data_min_limits[idx]) {
				TS_LOG_ERR("%s overlow_data = %d, row = %d, col = %d, limits [%d,%d]\n",
					__func__, data, row, col,
					threshold->raw_data_max_limits[idx],
					threshold->raw_data_min_limits[idx]);
				test_hcd->result = false;
				testresult = CAP_TEST_FAIL_CHAR;
				strncpy(failedreason, "-panel_reason",
					TS_RAWDATA_FAILED_REASON_LEN - 1);
			}
			idx++;
		}
	}
exit:
	TS_LOG_INFO("%s end retval = %d 4%c\n", __func__, retval, testresult);
	pts_node->size = raw_data_size;
	pts_node->testresult = testresult;
	pts_node->typeindex = RAW_DATA_TYPE_FULLRAWDATA;
	strncpy(pts_node->test_name, "Full_Rawdata",
		TS_RAWDATA_TEST_NAME_LEN - 1);
	snprintf(pts_node->statistics_data, TS_RAWDATA_STATISTICS_DATA_LEN,
		"[%d,%d,%d]", raw_data_min, raw_data_max, raw_data_avg);
	if (testresult == CAP_TEST_FAIL_CHAR)
		strncpy(pts_node->tptestfailedreason, failedreason,
			TS_RAWDATA_FAILED_REASON_LEN - 1);
	list_add_tail(&pts_node->node, &info->rawdata_head);

	test_hcd->report_type = 0;
	kfree(temp_buf);
	temp_buf = NULL;
	return retval;

tcm_buf_zalloc_fail:
	kfree(temp_buf);
	temp_buf = NULL;
tmp_buf_zalloc_fail:
	kfree(pts_node->values);
	pts_node->values = NULL;
pts_node_value_zalloc_fail:
	kfree(pts_node);
	pts_node = NULL;
pts_node_zalloc_fail:
	return -ENOMEM;
}

static int touch_driver_delta_test(
	struct ts_rawdata_info_new *info, int *rawdata)
{
	int retval = 0;
	unsigned int rows;
	unsigned int cols;
	char testresult = CAP_TEST_PASS_CHAR;
	char failedreason[TS_RAWDATA_FAILED_REASON_LEN] = {0};
	int raw_data_size;
	int delta_data_max;
	int delta_data_min;
	int delta_data_avg = 0;
	int i;
	int j;
	int delta_data_size = 0;
	struct syna_tcm_app_info *app_info = NULL;
	struct syna_tcm_hcd *tcm_hcd = test_hcd->tcm_hcd;
	struct ts_rawdata_newnodeinfo *pts_node = NULL;
	struct syna_tcm_test_threshold *threshold = &test_params->threshold;

	TS_LOG_INFO("%s start", __func__);

	if (tcm_hcd->id_info.mode != MODE_APPLICATION ||
		tcm_hcd->app_status != APP_STATUS_OK) {
		TS_LOG_ERR("%s Application firmware not running\n", __func__);
		return -ENODEV;
	}

	app_info = &tcm_hcd->app_info;
	rows = le2_to_uint(app_info->num_of_image_rows);
	cols = le2_to_uint(app_info->num_of_image_cols);
	raw_data_size = rows * cols;

	pts_node = kzalloc(sizeof(struct ts_rawdata_newnodeinfo), GFP_KERNEL);
	if (!pts_node) {
		TS_LOG_ERR("malloc pts_node failed\n");
		return -ENOMEM;
	}

	pts_node->values = kzalloc(raw_data_size*sizeof(int) * 2, GFP_KERNEL);
	if (!pts_node->values) {
		TS_LOG_ERR("%s malloc value failed for values\n", __func__);
		kfree(pts_node);
		pts_node = NULL;
		return -ENOMEM;
	}
	memset(pts_node->values, 0, raw_data_size*sizeof(int) * 2);
	test_hcd->result = true;
	delta_data_max = delta_data_min =
		tskit_driver_abs(rawdata[0]-rawdata[1]);

	for (i = 0, j = 0; i < rows * cols; i++) {
		if (i >= cols) {
			pts_node->values[j] = tskit_driver_abs(rawdata[i] -
				rawdata[i - cols]);
			delta_data_max = delta_data_max < pts_node->values[j] ?
				pts_node->values[j] : delta_data_max;
			delta_data_min = delta_data_min > pts_node->values[j] ?
				pts_node->values[j] : delta_data_min;
			delta_data_avg += pts_node->values[j];
			j++;
		}
	}
	delta_data_size += j;

	for (i = 0, j = 0; i < rows * cols; i++) {
		if (i % cols != 0) {
			pts_node->values[j + raw_data_size] =
				tskit_driver_abs(rawdata[i] - rawdata[i - 1]);
			delta_data_max = delta_data_max < pts_node->values[j] ?
				pts_node->values[j] : delta_data_max;
			delta_data_min = delta_data_min > pts_node->values[j] ?
				pts_node->values[j] : delta_data_min;
			delta_data_avg += pts_node->values[j];
			j++;
		}
	}
	delta_data_size += j;

	TS_LOG_ERR("delta_data_size = %d\n", delta_data_size);

	if (delta_data_size)
		delta_data_avg /= delta_data_size;

	for (i = 0; i < raw_data_size * 2; i++) {
		TS_LOG_DEBUG("delta value is[%d, %d] %d > %d\n", i/cols, i%cols,
			pts_node->values[i],
			threshold->raw_data_Trx_delta_max_limits[i]);
		if (pts_node->values[i] >
			threshold->raw_data_Trx_delta_max_limits[i]) {
			testresult = CAP_TEST_FAIL_CHAR;
			strncpy(failedreason, "-panel_reason",
				TS_RAWDATA_FAILED_REASON_LEN - 1);
			TS_LOG_ERR("delta value is[%d, %d] %d > %d\n",
				i/cols, i%cols, pts_node->values[i],
				threshold->raw_data_Trx_delta_max_limits[i]);
		}
	}

	TS_LOG_INFO("%s end retval = %d 2%c\n", __func__, retval, testresult);
	pts_node->size = raw_data_size * 2;
	pts_node->testresult = testresult;
	pts_node->typeindex = RAW_DATA_TYPE_TRXDELTA;
	strncpy(pts_node->test_name, "delta_Rawdata",
		TS_RAWDATA_TEST_NAME_LEN - 1);
	snprintf(pts_node->statistics_data, TS_RAWDATA_STATISTICS_DATA_LEN,
		"[%d,%d,%d]", delta_data_min, delta_data_max, delta_data_avg);
	if (testresult == CAP_TEST_FAIL_CHAR)
		strncpy(pts_node->tptestfailedreason, failedreason,
			TS_RAWDATA_FAILED_REASON_LEN - 1);
	list_add_tail(&pts_node->node, &info->rawdata_head);
	info->listnodenum++;

	test_hcd->report_type = 0;

	return retval;
}


#define SYNC_TCM_CAP_TEST_GEAR_COUNT 3
static int touch_driver_noise_different_gear_test(
	struct ts_rawdata_info_new *info)
{
	int retval;
	unsigned int idx;
	unsigned int value_index;
	signed short data;
	unsigned int data_length;
	unsigned int row;
	unsigned int col;
	unsigned int rows;
	unsigned int cols;
	unsigned int frame_size_words;
	unsigned char *temp_buf = NULL;
	char testresult = CAP_TEST_PASS_CHAR;
	char failedreason[TS_RAWDATA_FAILED_REASON_LEN] = {0};
	int noise_data_max;
	int noise_data_min;
	int noise_data_avg = 0;
	int noise_data_size;
	int gear_index;
	int gear;

	struct syna_tcm_app_info *app_info = NULL;
	struct syna_tcm_hcd *tcm_hcd = test_hcd->tcm_hcd;
	struct ts_rawdata_newnodeinfo *pts_node = NULL;
	struct syna_tcm_test_threshold *threshold = &test_params->threshold;

	TS_LOG_INFO("%s start", __func__);

	if (tcm_hcd->id_info.mode != MODE_APPLICATION ||
		tcm_hcd->app_status != APP_STATUS_OK) {
		TS_LOG_ERR("%s Application firmware not running\n", __func__);
		return -ENODEV;
	}

	app_info = &tcm_hcd->app_info;
	rows = le2_to_uint(app_info->num_of_image_rows);
	cols = le2_to_uint(app_info->num_of_image_cols);
	noise_data_size = SYNC_TCM_CAP_TEST_GEAR_COUNT * rows * cols;

	pts_node = kzalloc(sizeof(struct ts_rawdata_newnodeinfo), GFP_KERNEL);
	if (!pts_node) {
		TS_LOG_ERR("malloc pts_node failed\n");
		goto pts_node_zalloc_fail;
	}

	pts_node->values = kzalloc(noise_data_size*sizeof(int), GFP_KERNEL);
	if (!pts_node->values) {
		TS_LOG_ERR("%s malloc value failed  for values\n", __func__);
		goto pts_node_value_zalloc_fail;
	}

	temp_buf = kzalloc(MAX_FRAME_BUFFER, GFP_KERNEL);
	if (!temp_buf) {
		TS_LOG_ERR("%s Failed to allocate memory for temp_buf\n",
			__func__);
		goto tmp_buf_zalloc_fail;
	}

	retval = tskit_driver_alloc_mem(tcm_hcd, &test_hcd->out, 1);
	if (retval < 0) {
		TS_LOG_ERR("Failed to allocate memory for testing_hcd buf\n");
		goto tcm_buf_zalloc_fail;
	}

	/* enable gear switch */
	retval = test_hcd->tcm_hcd->set_dynamic_config(test_hcd->tcm_hcd,
		DC_INHIBIT_FREQUENCY_SHIFT, 1);
	if (retval < 0)
		TS_LOG_ERR("%s failed to set dc\n", __func__);

	for (gear_index = 0; gear_index <
		SYNC_TCM_CAP_TEST_GEAR_COUNT; gear_index++) {

		if (gear_index == 0)
			gear = GEAR_FREQUENCY_0;
		else if (gear_index == 1)
			gear = GEAR_FREQUENCY_1;
		else if (gear_index == 2)
			gear = GEAR_FREQUENCY_2;
		retval = test_hcd->tcm_hcd->set_dynamic_config(
			test_hcd->tcm_hcd, DC_REQUESTED_FREQUENCY, gear);
		if (retval < 0)
			TS_LOG_ERR("%s failed to set dc\n", __func__);

		test_hcd->out.buf[0] = TEST_NOISE;

		retval = tskit_driver_write_hdl_message(tcm_hcd,
			CMD_PRODUCTION_TEST, test_hcd->out.buf, 1,
			&test_hcd->resp.buf, &test_hcd->resp.buf_size,
			&test_hcd->resp.data_length, NULL, 0);
		if (retval < 0) {
			TS_LOG_ERR("Failed to write command %s\n",
				STR(CMD_PRODUCTION_TEST));
			testresult = CAP_TEST_FAIL_CHAR;
			strncpy(failedreason, "-software_reason",
				TS_RAWDATA_FAILED_REASON_LEN-1);
			goto exit;
		}

		retval = touch_driver_testing_read(tcm_hcd, temp_buf,
			MAX_FRAME_BUFFER);
		if ((retval < 0) || (temp_buf[1] != STATUS_OK)) {
			TS_LOG_ERR("status = 0x%02x", temp_buf[1]);
			TS_LOG_ERR("Failed to read raw data\n");
			testresult = CAP_TEST_FAIL_CHAR;
			strncpy(failedreason, "-software_reason",
				TS_RAWDATA_FAILED_REASON_LEN - 1);
			goto exit;
		}

		data_length = temp_buf[2] | temp_buf[3] << 8;
		TS_LOG_INFO(" data_length =%d\n", data_length);
		if (tcm_hcd->htd == true)
			tskit_driver_get_frame_size_words(&frame_size_words,
				false);
		else
			tskit_driver_get_frame_size_words(&frame_size_words,
				true);

		if (frame_size_words != data_length / 2) {
			TS_LOG_ERR("Frame size mismatch\n");
			retval = -EINVAL;
			testresult = CAP_TEST_FAIL_CHAR;
			strncpy(failedreason, "-software_reason",
				TS_RAWDATA_FAILED_REASON_LEN-1);

			goto exit;
		}

		idx = 0;
		value_index = gear_index * rows * cols;
		if (gear_index == 0)
			noise_data_max = noise_data_min =
				(int)le2_to_uint(
				&temp_buf[MESSAGE_HEADER_SIZE]);

		for (row = 0; row < rows; row++) {
			for (col = 0; col < cols; col++) {
				data = (signed short)le2_to_uint(
					&temp_buf[idx * 2 +
					MESSAGE_HEADER_SIZE]);
				pts_node->values[idx + value_index] = (int)data;
				noise_data_max = data > noise_data_max ?
					data : noise_data_max;
				noise_data_min = data < noise_data_min ?
					data : noise_data_min;
				noise_data_avg += data;
				TS_LOG_DEBUG("noise_buf = %d", data);
				idx++;
			}
		}
		if (noise_data_size)
			noise_data_avg /= noise_data_size;

		idx = 0;
		test_hcd->result = true;

		for (row = 0; row < rows; row++) {
			for (col = 0; col < cols; col++) {
				data = (signed short)le2_to_uint(
					&temp_buf[idx * 2 +
					MESSAGE_HEADER_SIZE]);
				TS_LOG_DEBUG("noise data=%d", data);
				if (data >
					threshold->lcd_noise_max_limits[idx]) {
					TS_LOG_ERR("%s overlow_data = %d, limits = %d, row = %d, col = %d\n",
						__func__, data,
						threshold->lcd_noise_max_limits[idx],
						row, col);
					test_hcd->result = false;
					testresult = CAP_TEST_FAIL_CHAR;
					strncpy(failedreason, "-panel_reason",
						(TS_RAWDATA_FAILED_REASON_LEN -
						1));
					goto exit;
				}
				idx++;
			}
		}
	}
exit:
	TS_LOG_INFO("%s end retval = %d 3%c\n", __func__, retval, testresult);

	/* set the gear to 0 */
	retval = test_hcd->tcm_hcd->set_dynamic_config(test_hcd->tcm_hcd,
		DC_REQUESTED_FREQUENCY, 0);
	if (retval < 0)
		TS_LOG_ERR("%s failed to switch the frequency\n", __func__);
	/* set the frequency shift to 0 */
	retval = test_hcd->tcm_hcd->set_dynamic_config(test_hcd->tcm_hcd,
		DC_INHIBIT_FREQUENCY_SHIFT, 0);
	if (retval < 0)
		TS_LOG_ERR("%s failed to set the frequency shift\n", __func__);

	pts_node->size = noise_data_size;
	pts_node->testresult = testresult;
	pts_node->typeindex = RAW_DATA_TYPE_NOISE;
	strncpy(pts_node->test_name, "Noise_delta",
		TS_RAWDATA_TEST_NAME_LEN - 1);
	snprintf(pts_node->statistics_data, TS_RAWDATA_STATISTICS_DATA_LEN,
		"[%d,%d,%d]", noise_data_min, noise_data_max, noise_data_avg);
	if (testresult == CAP_TEST_FAIL_CHAR)
		strncpy(pts_node->tptestfailedreason, failedreason,
			TS_RAWDATA_FAILED_REASON_LEN - 1);
	list_add_tail(&pts_node->node, &info->rawdata_head);

	kfree(temp_buf);
	temp_buf = NULL;
	return retval;

tcm_buf_zalloc_fail:
	kfree(temp_buf);
	temp_buf = NULL;
tmp_buf_zalloc_fail:
	kfree(pts_node->values);
	pts_node->values = NULL;
pts_node_value_zalloc_fail:
	kfree(pts_node);
	pts_node = NULL;
pts_node_zalloc_fail:
	return -ENOMEM;

}

static unsigned int touch_driver_thp_check_short(void *pdata,
	unsigned int rows, unsigned int cols)
{
	unsigned short i;
	unsigned short j;
	unsigned short k;
	unsigned short fail;
	unsigned short *ptr = (unsigned short *)pdata;

	for (i = 0; i < rows - 1; i++) {
		for (j = i+1; j < rows; j++) {
			fail = 1;
			for (k = 0 ; k < cols ; k++) {
				if (ptr[i * cols + k] != ptr[j * cols + k]) {
					fail = 0;
					break;
				}
			}
			if (fail)
				return (i << 16) + j;
		}
	}
	return 0;
}

static int touch_driver_predecode_test(void)
{
	int retval;
	int i;
	unsigned int rows;
	unsigned int cols;
	unsigned int pt9_data_size;
	unsigned int packsize;
	unsigned int short_result[3] = {0, 0, 0};
	struct syna_tcm_hcd *tcm_hcd = test_hcd->tcm_hcd;
	struct syna_tcm_app_info *app_info = NULL;

	app_info = &tcm_hcd->app_info;
	rows = le2_to_uint(app_info->num_of_image_rows);
	cols = le2_to_uint(app_info->num_of_image_cols);
	pt9_data_size = (rows * cols);

	TS_LOG_INFO("%s start cols = %d, rows = %d\n", __func__, cols, rows);

	memset(test_hcd->out.buf, 0x00, test_hcd->out.buf_size);

	for (i = 0 ; i < 3 ; i++) {
		test_hcd->out.buf[0] = TEST_PREDECODE_RAW;

		retval = tskit_driver_write_hdl_message(tcm_hcd,
			CMD_PRODUCTION_TEST, test_hcd->out.buf, 1,
			&test_hcd->resp.buf, &test_hcd->resp.buf_size,
			&test_hcd->resp.data_length, NULL, 0);
		if (retval < 0) {
			TS_LOG_ERR("Failed to write command %s with TEST_PREDECODE_RAW\n",
				STR(CMD_PRODUCTION_TEST));
			goto exit;
		}

		retval = touch_driver_testing_read(tcm_hcd, g_temp_buf,
			sizeof(g_temp_buf));
		if ((retval < 0) || (g_temp_buf[1] != STATUS_OK)) {
			TS_LOG_ERR("status = 0x%02x", g_temp_buf[1]);
			TS_LOG_ERR("Failed to read PT9 data\n");
			retval = -EINVAL;
			goto exit;
		}

		packsize = ((g_temp_buf[3] << 8) | (g_temp_buf[2])) >> 1;
		TS_LOG_INFO("1/2 payload size = %d\n", packsize);

		if (packsize == cols * rows) {
			short_result[i] = touch_driver_thp_check_short(
				&g_temp_buf[4], rows, cols);
			TS_LOG_INFO("short result[%hu] = %08x\n", i,
				short_result[i]);
		}
	}

	if (short_result[0] * short_result[1] * short_result[2]) {
		TS_LOG_ERR("PT9 test failed\n");
		retval = -EINVAL;
	}

exit:
	return retval;
}

static int touch_driver_trx_trx_short_test(
	struct ts_rawdata_info_new *info)
{
	int retval = NO_ERR;
	int i;
	int j;
	int py_p;
	unsigned char *pt1_data = NULL;
	unsigned int pt1_data_size = 8;
	struct syna_tcm_app_info *app_info = NULL;
	struct syna_tcm_hcd *tcm_hcd = test_hcd->tcm_hcd;
	int failure_cnt_pt1 = 0;
	int data_length;
	int pt1_data_min = 0;
	int pt1_data_max = 0;
	int pt1_data_avg = 0;
	unsigned int num_kzalloc;
	char testresult = CAP_TEST_PASS_CHAR;
	char failedreason[TS_RAWDATA_FAILED_REASON_LEN] = {0};
	struct ts_rawdata_newnodeinfo *pts_node = NULL;

	TS_LOG_INFO("%s start", __func__);
	pts_node = kzalloc(sizeof(*pts_node), GFP_KERNEL);
	if (!pts_node) {
		TS_LOG_ERR("malloc pts_node failed\n");
		goto pts_node_zalloc_fail;
	}

	pts_node->values = kzalloc(pt1_data_size*sizeof(int), GFP_KERNEL);
	if (!pts_node->values) {
		TS_LOG_ERR("%s malloc value failed for values\n", __func__);
		goto pts_node_value_zalloc_fail;
	}

	app_info = &tcm_hcd->app_info;

	retval = tskit_driver_alloc_mem(tcm_hcd, &test_hcd->out, 1);
	if (retval < 0) {
		TS_LOG_ERR("Failed to alloc_mem for test_hcd->out.buf\n");
		goto tcm_buf_zalloc_fail;
	}

	test_hcd->out.buf[0] = TEST_TRX_TRX_SHORTS;

	retval = tskit_driver_write_hdl_message(tcm_hcd, CMD_PRODUCTION_TEST,
		test_hcd->out.buf, 1, &test_hcd->resp.buf,
		&test_hcd->resp.buf_size, &test_hcd->resp.data_length,
		NULL, 0);
	if (retval < 0) {
		TS_LOG_ERR("Failed to write command %s\n",
			STR(CMD_PRODUCTION_TEST));
		testresult = CAP_TEST_FAIL_CHAR;
		strncpy(failedreason, "-software_reason",
			TS_RAWDATA_FAILED_REASON_LEN - 1);
		goto exit;
	}

	retval = touch_driver_testing_read(tcm_hcd, g_temp_buf,
		sizeof(g_temp_buf));
	if (retval < 0 || (g_temp_buf[1] != STATUS_OK)) {
		TS_LOG_ERR("status = 0x%02x", g_temp_buf[1]);
		TS_LOG_ERR("Failed to read short data\n");
		testresult = CAP_TEST_FAIL_CHAR;
		strncpy(failedreason, "-software_reason",
		TS_RAWDATA_FAILED_REASON_LEN - 1);
		goto exit;
	}

	data_length = g_temp_buf[2] | g_temp_buf[3] << 8;
	TS_LOG_INFO("data_length = %d\n", data_length);

	test_hcd->result = true;
	num_kzalloc = pt1_data_size * sizeof(*pt1_data);
	pt1_data = kzalloc(num_kzalloc, GFP_KERNEL);
	if (!pt1_data) {
		TS_LOG_ERR("Failed to allocate mem to pt1_data\n");
		goto pt1_data_zalloc_fail;
	}
	memcpy(pt1_data, &g_temp_buf[MESSAGE_HEADER_SIZE],
		pt1_data_size);

	for (i = 0; i < pt1_data_size; i++) {
		pts_node->values[i] = (int)pt1_data[i];
		TS_LOG_ERR("i = %d: %2d\n", i, pt1_data[i]);
		if (i == 0) {
			pt1_data_min = pt1_data[i];
			pt1_data_max = pt1_data[i];
			pt1_data_avg = pt1_data[i];
		} else {
			if (pt1_data_min > pt1_data[i])
				pt1_data_min = pt1_data[i];
			if (pt1_data_max < pt1_data[i])
				pt1_data_max = pt1_data[i];
			pt1_data_avg = pt1_data[i] + pt1_data_avg;
		}
		for (j = 0; j < 8; j++) {
			py_p = (i * 8 + j);
				if (CHECK_BIT(pt1_data[i], j) == 0) {
					TS_LOG_INFO("pin-%2d : pass\n", py_p);
				} else {
					TS_LOG_ERR("pin-%2d : failed\n", py_p);
					failure_cnt_pt1++;
				}
		}
	}
	if (pt1_data_avg)
		pt1_data_avg = (pt1_data_avg/pt1_data_size);
	test_hcd->result = (failure_cnt_pt1 == 0);
	retval = (failure_cnt_pt1 == 0);

	/* PT9 */
	if (retval) {
		retval = touch_driver_predecode_test();
		if (retval < 0) {
			TS_LOG_ERR("Failed to do predecode test\n");
			testresult = CAP_TEST_FAIL_CHAR;
			strncpy(failedreason, "-panel_reason",
				TS_RAWDATA_FAILED_REASON_LEN - 1);
		}
	} else {
		TS_LOG_ERR("Failed to do TRX_TRX_SHORTS test\n");
		testresult = CAP_TEST_FAIL_CHAR;
		strncpy(failedreason, "-panel_reason",
			TS_RAWDATA_FAILED_REASON_LEN - 1);
	}

exit:
	TS_LOG_INFO("%s end retval = %d 5%c\n", __func__, retval, testresult);
	pts_node->size = pt1_data_size;
	pts_node->testresult = testresult;
	pts_node->typeindex = RAW_DATA_TYPE_OPENSHORT;
	strncpy(pts_node->test_name, "trx-trx short",
		TS_RAWDATA_TEST_NAME_LEN - 1);
	snprintf(pts_node->statistics_data, TS_RAWDATA_STATISTICS_DATA_LEN,
		"[%d,%d,%d]", pt1_data_min, pt1_data_max, pt1_data_avg);
	if (testresult == CAP_TEST_FAIL_CHAR)
		strncpy(pts_node->tptestfailedreason, failedreason,
			TS_RAWDATA_FAILED_REASON_LEN - 1);
	list_add_tail(&pts_node->node, &info->rawdata_head);

	kfree(pt1_data);
	pt1_data = NULL;

	return retval;
pt1_data_zalloc_fail:
	kfree(test_hcd->out.buf);
	test_hcd->out.buf = NULL;
tcm_buf_zalloc_fail:
	kfree(pts_node->values);
	pts_node->values = NULL;
pts_node_value_zalloc_fail:
	kfree(pts_node);
	pts_node = NULL;
pts_node_zalloc_fail:
	return -ENOMEM;
}

static int touch_driver_abs_raw_cap_test(
	struct ts_rawdata_info_new *info)
{
	int retval;
	unsigned int idx;
	unsigned int data_length;
	unsigned int row;
	unsigned int col;
	unsigned int rows;
	unsigned int cols;
	unsigned char *temp_buf = NULL;
	int *temp_buf_row_int_p = NULL;
	int *temp_buf_col_int_p = NULL;
	char testresult = CAP_TEST_PASS_CHAR;
	char failedreason[TS_RAWDATA_FAILED_REASON_LEN] = {0};
	int noise_data_max = 0;
	int noise_data_min = 0;
	int noise_data_avg = 0;
	int abs_raw_data_size;

	struct syna_tcm_app_info *app_info = NULL;
	struct syna_tcm_hcd *tcm_hcd = test_hcd->tcm_hcd;
	struct ts_rawdata_newnodeinfo *pts_node = NULL;
	struct syna_tcm_test_threshold *threshold = &test_params->threshold;

	TS_LOG_INFO("%s start", __func__);

	if (tcm_hcd->id_info.mode != MODE_APPLICATION ||
		tcm_hcd->app_status != APP_STATUS_OK) {
		TS_LOG_ERR("%s Application firmware not running\n", __func__);
		return -ENODEV;
	}

	app_info = &tcm_hcd->app_info;
	rows = le2_to_uint(app_info->num_of_image_rows);
	cols = le2_to_uint(app_info->num_of_image_cols);
	abs_raw_data_size = rows + cols;

	pts_node = kzalloc(sizeof(struct ts_rawdata_newnodeinfo), GFP_KERNEL);
	if (!pts_node) {
		TS_LOG_ERR("malloc pts_node failed\n");
		goto pts_node_zalloc_fail;
	}

	pts_node->values = kzalloc(abs_raw_data_size*sizeof(int), GFP_KERNEL);
	if (!pts_node->values) {
		TS_LOG_ERR("%s malloc value failed  for values\n", __func__);
		goto pts_node_value_zalloc_fail;
	}
	temp_buf_row_int_p = kzalloc(rows*sizeof(int), GFP_KERNEL);
	if (!temp_buf_row_int_p) {
		TS_LOG_ERR("%s malloc temp_buf_row_int_p failed  for values\n",
			__func__);
		goto temp_buf_row_int_p_zalloc_fail;
	}
	temp_buf_col_int_p = kzalloc(cols*sizeof(int), GFP_KERNEL);
	if (!temp_buf_col_int_p) {
		TS_LOG_ERR("%s malloc temp_buf_col_int_p failed  for values\n",
			__func__);
		goto temp_buf_col_int_p_zalloc_fail;
	}

	temp_buf = kzalloc(MAX_FRAME_BUFFER, GFP_KERNEL);
	if (!temp_buf) {
		TS_LOG_ERR("%s Failed to allocate memory for temp_buf\n",
			__func__);
		goto temp_buf_zalloc_fail;
	}

	retval = tskit_driver_alloc_mem(tcm_hcd, &test_hcd->out, 1);
	if (retval < 0) {
		TS_LOG_ERR("Failed to allocate memory for test_hcd->out.buf\n");
		goto tcm_buf_zalloc_fail;
	}

	test_hcd->out.buf[0] = TEST_PT18_NEW_RULES;

	retval = tskit_driver_write_hdl_message(tcm_hcd, CMD_PRODUCTION_TEST,
		test_hcd->out.buf, 1, &test_hcd->resp.buf,
		&test_hcd->resp.buf_size, &test_hcd->resp.data_length, NULL, 0);
	if (retval < 0) {
		TS_LOG_ERR("Failed to write command %s\n",
			STR(CMD_PRODUCTION_TEST));
		testresult = CAP_TEST_FAIL_CHAR;
		strncpy(failedreason, "-software_reason",
			TS_RAWDATA_FAILED_REASON_LEN - 1);
		goto exit;
	}

	retval = touch_driver_testing_read(tcm_hcd, temp_buf, MAX_FRAME_BUFFER);
	if ((retval < 0) || (temp_buf[1] != STATUS_OK)) {
		TS_LOG_ERR("status = 0x%02x", temp_buf[1]);
		TS_LOG_ERR("Failed to read raw cap data\n");
		testresult = CAP_TEST_FAIL_CHAR;
		strncpy(failedreason, "-software_reason",
		TS_RAWDATA_FAILED_REASON_LEN - 1);
		goto exit;
	}

	data_length = temp_buf[2] | temp_buf[3] << 8;
	TS_LOG_INFO(" data_length =%d\n", data_length);

	if ((temp_buf[1] != STATUS_OK) ||
		(data_length != ((rows + cols) * 4))) {
		TS_LOG_ERR("not correct status or data_length status:%x  length:%x\n",
			temp_buf[1], data_length);
		retval = -EINVAL;
		testresult = CAP_TEST_FAIL_CHAR;
		strncpy(failedreason, "-software_reason",
			TS_RAWDATA_FAILED_REASON_LEN - 1);
		goto exit;
	}

	idx = MESSAGE_HEADER_SIZE;
	test_hcd->result = true;

	for (col = 0; col < cols; col++) {
		temp_buf_col_int_p[col] = (unsigned int)(temp_buf[idx] & 0xff) |
			(unsigned int)(temp_buf[idx+1] << 8) |
			(unsigned int)(temp_buf[idx+2] << 16) |
			(unsigned int)(temp_buf[idx+3] << 24);
			TS_LOG_DEBUG("col=%-2d data = %d , limit = %d, %d\n",
				col, temp_buf_col_int_p[col],
				threshold->raw_data_col_min_limits[col],
				threshold->raw_data_col_max_limits[col]);
		if ((temp_buf_col_int_p[col] <
			threshold->raw_data_col_min_limits[col]) ||
			(temp_buf_col_int_p[col] >
			threshold->raw_data_col_max_limits[col])) {
			test_hcd->result = false;
			testresult = CAP_TEST_FAIL_CHAR;
			strncpy(failedreason, "-panel_reason",
				TS_RAWDATA_FAILED_REASON_LEN-1);
			retval = -EINVAL;
			TS_LOG_ERR("fail at col=%-2d data = %d 0x%2x 0x%2x 0x%2x 0x%2x, limit = %d,%d\n",
				col, temp_buf_col_int_p[col],
				temp_buf[idx], temp_buf[idx+1],
				temp_buf[idx+2], temp_buf[idx+3],
				threshold->raw_data_col_min_limits[col],
				threshold->raw_data_col_max_limits[col]);
		}
		idx += 4;
	}

	for (row = 0; row < rows; row++) {
		temp_buf_row_int_p[row] = (unsigned int)(temp_buf[idx] & 0xff) |
			(unsigned int)(temp_buf[idx + 1] << 8) |
			(unsigned int)(temp_buf[idx + 2] << 16) |
			(unsigned int)(temp_buf[idx + 3] << 24);
			TS_LOG_DEBUG("row=%-2d data = %d, limit = %d, %d\n",
				row, temp_buf_row_int_p[row],
				threshold->raw_data_row_min_limits[row],
				threshold->raw_data_row_max_limits[row]);
		if ((temp_buf_row_int_p[row] <
			threshold->raw_data_row_min_limits[row]) ||
			(temp_buf_row_int_p[row] >
			threshold->raw_data_row_max_limits[row])) {
			test_hcd->result = false;
			testresult = CAP_TEST_FAIL_CHAR;
			strncpy(failedreason, "-panel_reason",
				TS_RAWDATA_FAILED_REASON_LEN-1);
			retval = -EINVAL;
			TS_LOG_ERR("fail at row=%-2d data = %d 0x%2x 0x%2x 0x%2x 0x%2x, limit = %d, %d\n",
				row, temp_buf_row_int_p[row],
				temp_buf[idx], temp_buf[idx+1],
				temp_buf[idx+2], temp_buf[idx+3],
				threshold->raw_data_row_min_limits[row],
				threshold->raw_data_row_max_limits[row]);
		}
		idx += 4;
	}

	memcpy(pts_node->values, temp_buf_row_int_p, rows*sizeof(int));
	memcpy(&(pts_node->values[rows]), temp_buf_col_int_p, cols*sizeof(int));

exit:
	TS_LOG_INFO("%s end retval = %d 6%c\n", __func__, retval, testresult);

	pts_node->size = abs_raw_data_size;
	pts_node->testresult = testresult;
	pts_node->typeindex = RAW_DATA_TYPE_SELFCAP;
	strncpy(pts_node->test_name, "abs raw data",
		TS_RAWDATA_TEST_NAME_LEN - 1);
	snprintf(pts_node->statistics_data, TS_RAWDATA_STATISTICS_DATA_LEN,
		"[%d,%d,%d]", noise_data_min, noise_data_max, noise_data_avg);
	if (testresult == CAP_TEST_FAIL_CHAR)
		strncpy(pts_node->tptestfailedreason, failedreason,
			TS_RAWDATA_FAILED_REASON_LEN-1);
	list_add_tail(&pts_node->node, &info->rawdata_head);

	kfree(temp_buf);
	temp_buf = NULL;
	kfree(temp_buf_row_int_p);
	temp_buf_row_int_p = NULL;
	kfree(temp_buf_col_int_p);
	temp_buf_col_int_p = NULL;
	return retval;

tcm_buf_zalloc_fail:
	kfree(temp_buf);
	temp_buf = NULL;
temp_buf_zalloc_fail:
	kfree(temp_buf_col_int_p);
	temp_buf_col_int_p = NULL;
temp_buf_col_int_p_zalloc_fail:
	kfree(temp_buf_row_int_p);
	temp_buf_row_int_p = NULL;
temp_buf_row_int_p_zalloc_fail:
	kfree(pts_node->values);
	pts_node->values = NULL;
pts_node_value_zalloc_fail:
	kfree(pts_node);
	pts_node = NULL;
pts_node_zalloc_fail:
	return -ENOMEM;
}

static int touch_driver_abs_noise_cap_test(
	struct ts_rawdata_info_new *info)
{
	int retval;
	unsigned int idx;
	unsigned int data_length;
	unsigned int row;
	unsigned int col;
	unsigned int rows;
	unsigned int cols;
	unsigned char *temp_buf = NULL;
	int *temp_buf_row_int_p = NULL;
	int *temp_buf_col_int_p = NULL;
	char testresult = CAP_TEST_PASS_CHAR;
	char failedreason[TS_RAWDATA_FAILED_REASON_LEN] = {0};
	int noise_data_max = 0;
	int noise_data_min = 0;
	int noise_data_avg = 0;
	int abs_raw_data_size;

	struct syna_tcm_app_info *app_info = NULL;
	struct syna_tcm_hcd *tcm_hcd = test_hcd->tcm_hcd;
	struct ts_rawdata_newnodeinfo *pts_node = NULL;
	struct syna_tcm_test_threshold *threshold = &test_params->threshold;

	TS_LOG_INFO("%s start", __func__);

	if (tcm_hcd->id_info.mode != MODE_APPLICATION ||
			tcm_hcd->app_status != APP_STATUS_OK) {
		TS_LOG_ERR("%s Application firmware not running\n", __func__);
		return -ENODEV;
	}

	app_info = &tcm_hcd->app_info;
	rows = le2_to_uint(app_info->num_of_image_rows);
	cols = le2_to_uint(app_info->num_of_image_cols);
	abs_raw_data_size = rows + cols;

	pts_node = kzalloc(sizeof(struct ts_rawdata_newnodeinfo), GFP_KERNEL);
	if (!pts_node) {
		TS_LOG_ERR("%s malloc pts_node failed\n", __func__);
		goto pts_node_zalloc_fail;
	}

	pts_node->values = kzalloc(abs_raw_data_size*sizeof(int), GFP_KERNEL);
	if (!pts_node->values) {
		TS_LOG_ERR("%s malloc value failed for values\n", __func__);
		goto pts_node_value_zalloc_fail;
	}
	temp_buf_row_int_p = kzalloc(rows*sizeof(int), GFP_KERNEL);
	if (!temp_buf_row_int_p) {
		TS_LOG_ERR("%s malloc temp_buf_row_int_p failed  for values\n",
			__func__);
		goto temp_buf_row_int_p_zalloc_fail;
	}
	temp_buf_col_int_p = kzalloc(cols*sizeof(int), GFP_KERNEL);
	if (!temp_buf_col_int_p) {
		TS_LOG_ERR("%s malloc temp_buf_col_int_p failed  for values\n",
			__func__);
		goto temp_buf_col_int_p_zalloc_fail;
	}

	temp_buf = kzalloc(MAX_FRAME_BUFFER, GFP_KERNEL);
	if (!temp_buf) {
		TS_LOG_ERR("%s Failed to allocate memory for temp_buf\n",
			__func__);
		goto temp_buf_zalloc_fail;
	}

	retval = tskit_driver_alloc_mem(tcm_hcd, &test_hcd->out, 1);
	if (retval < 0) {
		TS_LOG_ERR("%s Failed to allocate memory for test_hcd buf\n",
			__func__);
		goto tcm_buf_zalloc_fail;
	}

	test_hcd->out.buf[0] = TEST_PT1D;

	retval = tskit_driver_write_hdl_message(tcm_hcd, CMD_PRODUCTION_TEST,
		test_hcd->out.buf, 1, &test_hcd->resp.buf,
		&test_hcd->resp.buf_size, &test_hcd->resp.data_length, NULL, 0);
	if (retval < 0) {
		TS_LOG_ERR("%s Failed to write command %s with TEST_PT1D\n",
			__func__, STR(CMD_PRODUCTION_TEST));
		testresult = CAP_TEST_FAIL_CHAR;
		strncpy(failedreason, "-software_reason",
			TS_RAWDATA_FAILED_REASON_LEN - 1);
		goto exit;
	}

	retval = touch_driver_testing_read(tcm_hcd, temp_buf, MAX_FRAME_BUFFER);
	if ((retval < 0) || (temp_buf[1] != STATUS_OK)) {
		TS_LOG_ERR("%s status = 0x%02x", __func__, temp_buf[1]);
		TS_LOG_ERR("%s Failed to read abs nosie data\n", __func__);
		testresult = CAP_TEST_FAIL_CHAR;
		strncpy(failedreason, "-software_reason",
			TS_RAWDATA_FAILED_REASON_LEN - 1);
		goto exit;
	}

	data_length = temp_buf[2] | temp_buf[3] << 8;
	TS_LOG_INFO("%s data_length =%d\n", __func__, data_length);

	if ((temp_buf[1] != STATUS_OK) ||
		(data_length != ((rows + cols) * 2))) {
		TS_LOG_ERR("%s not correct status or data_length status:%x  length:%x\n",
			__func__, temp_buf[1], data_length);
		retval = -EINVAL;
		testresult = CAP_TEST_FAIL_CHAR;
		strncpy(failedreason, "-software_reason",
			TS_RAWDATA_FAILED_REASON_LEN - 1);
		goto exit;
	}

	idx = MESSAGE_HEADER_SIZE;
	test_hcd->result = true;

	for (col = 0; col < cols; col++) {
		temp_buf_col_int_p[col] = (int)((temp_buf[idx] & 0xff) |
			(temp_buf[idx+1] << 8));
			TS_LOG_ERR("abs noise:: col=%-2d data = %d , limit = %d\n",
				col, temp_buf_col_int_p[col],
				threshold->abs_noise_col_max_limits[col]);
		if (col == 0) {
			noise_data_min = temp_buf_col_int_p[col];
			noise_data_max = temp_buf_col_int_p[col];
			noise_data_avg = temp_buf_col_int_p[col];
		} else {
			if (noise_data_min > temp_buf_col_int_p[col])
				noise_data_min = temp_buf_col_int_p[col];
			if (noise_data_max < temp_buf_col_int_p[col])
				noise_data_max = temp_buf_col_int_p[col];
			noise_data_avg = (temp_buf_col_int_p[col] +
				noise_data_avg);
		}
		if (temp_buf_col_int_p[col] >
			threshold->abs_noise_col_max_limits[col]) {
			test_hcd->result = false;
			testresult = CAP_TEST_FAIL_CHAR;
			strncpy(failedreason, "-panel_reason",
				TS_RAWDATA_FAILED_REASON_LEN - 1);
			retval = -EINVAL;
			TS_LOG_ERR("%s fail at col=%-2d data = %d 0x%2x 0x%2x, limit = [%d]\n",
				__func__,
				col, temp_buf_col_int_p[col],
				temp_buf[idx], temp_buf[idx+1],
				threshold->abs_noise_col_max_limits[col]);
			noise_data_avg = (noise_data_avg/col);
		}
		idx += 2;
	}

	for (row = 0; row < rows; row++) {
		temp_buf_row_int_p[row] = (int)((temp_buf[idx] & 0xff) |
			(temp_buf[idx+1] << 8));
		TS_LOG_DEBUG("%s row=%-2d data = %d, limit = %d\n",
			__func__, row, temp_buf_row_int_p[row],
			threshold->abs_noise_row_max_limits[row]);
		if (noise_data_min > temp_buf_col_int_p[row])
			noise_data_min = temp_buf_col_int_p[row];
		if (noise_data_max < temp_buf_col_int_p[row])
			noise_data_max = temp_buf_col_int_p[row];
		noise_data_avg = (temp_buf_col_int_p[row]+noise_data_avg);

		if (temp_buf_row_int_p[row] >
			threshold->abs_noise_row_max_limits[row]) {
			test_hcd->result = false;
			testresult = CAP_TEST_FAIL_CHAR;
			strncpy(failedreason, "-panel_reason",
				TS_RAWDATA_FAILED_REASON_LEN - 1);
			retval = -EINVAL;
			TS_LOG_ERR("%s fail at row=%-2d data = %d 0x%2x 0x%2x, limit = [%d]\n",
				__func__,
				row, temp_buf_row_int_p[row],
				temp_buf[idx], temp_buf[idx+1],
				threshold->abs_noise_row_max_limits[row]);
			noise_data_avg = (noise_data_avg/(row+cols));
		//	goto exit;
		}
		idx += 2;
	}
	noise_data_avg = (noise_data_avg / (rows + cols));
	memcpy(pts_node->values, temp_buf_row_int_p, rows*sizeof(int));
	memcpy(&(pts_node->values[rows]), temp_buf_col_int_p, cols*sizeof(int));

exit:
	TS_LOG_INFO("%s end retval = %d 9%c\n", __func__, retval, testresult);

	pts_node->size = abs_raw_data_size;
	pts_node->testresult = testresult;
	pts_node->typeindex = RAW_DATA_TYPE_SELFNOISETEST;
	strncpy(pts_node->test_name, "abs noise data",
		TS_RAWDATA_TEST_NAME_LEN - 1);
	snprintf(pts_node->statistics_data, TS_RAWDATA_STATISTICS_DATA_LEN,
		"[%d,%d,%d]", noise_data_min, noise_data_max, noise_data_avg);
	if (testresult == CAP_TEST_FAIL_CHAR)
		strncpy(pts_node->tptestfailedreason, failedreason,
			TS_RAWDATA_FAILED_REASON_LEN - 1);
	list_add_tail(&pts_node->node, &info->rawdata_head);

	kfree(temp_buf);
	temp_buf = NULL;
	kfree(temp_buf_row_int_p);
	temp_buf_row_int_p = NULL;
	kfree(temp_buf_col_int_p);
	temp_buf_col_int_p = NULL;
	return retval;
tcm_buf_zalloc_fail:
	kfree(temp_buf);
	temp_buf = NULL;
temp_buf_zalloc_fail:
	kfree(temp_buf_col_int_p);
	temp_buf_col_int_p = NULL;
temp_buf_col_int_p_zalloc_fail:
	kfree(temp_buf_row_int_p);
	temp_buf_row_int_p = NULL;
temp_buf_row_int_p_zalloc_fail:
	kfree(pts_node->values);
	pts_node->values = NULL;
pts_node_value_zalloc_fail:
	kfree(pts_node);
	pts_node = NULL;
pts_node_zalloc_fail:
	return -ENOMEM;

}

static int touch_driver_i2c_communication_test(
	struct ts_rawdata_info_new *info)
{
	struct syna_tcm_hcd *tcm_hcd = test_hcd->tcm_hcd;

	if (tcm_hcd->id_info.mode != MODE_APPLICATION ||
		tcm_hcd->app_status != APP_STATUS_OK) {
		snprintf(info->i2cinfo, TS_RAWDATA_RESULT_CODE_LEN,
			"%d%c", RAW_DATA_TYPE_IC, CAP_TEST_FAIL_CHAR);
		strncat(info->i2cerrinfo, "software reason ",
			sizeof(info->i2cerrinfo));
		return -ENODEV;
	}
	snprintf(info->i2cinfo, TS_RAWDATA_RESULT_CODE_LEN,
		"%d%c", RAW_DATA_TYPE_IC, CAP_TEST_PASS_CHAR);
	return 0;
}

static void touch_driver_cap_test_order(
	struct ts_rawdata_info_new *info)
{
	int retval;

	TS_LOG_INFO("%s: before disable doze\n", __func__);
	retval = test_hcd->tcm_hcd->set_dynamic_config(test_hcd->tcm_hcd,
		DC_NO_DOZE, 1);
	if (retval < 0)
		TS_LOG_ERR("%s failed to set dc\n", __func__);

	/* test item 1 2 */
	retval = touch_driver_cap_raw_data_test(info);
	if (retval < 0)
		TS_LOG_ERR("%s raw data testing failed\n", __func__);
	info->listnodenum++;

	/* test item 3 */
	retval = touch_driver_noise_different_gear_test(info);
	if (retval < 0)
		TS_LOG_ERR("%s noise different gear testing failed\n",
			__func__);
	info->listnodenum++;

	/* test item 4 */
	retval = touch_driver_full_raw_cap_test(info);
	if (retval < 0)
		TS_LOG_ERR("%s raw data testing failed\n", __func__);
	info->listnodenum++;

	/* test item 5 */
	retval = touch_driver_trx_trx_short_test(info);
	if (retval < 0)
		TS_LOG_ERR("%s trx_trx_short failed\n", __func__);
	info->listnodenum++;
	/* test item 6 */
	retval = touch_driver_abs_raw_cap_test(info);
	if (retval < 0)
		TS_LOG_ERR("%s raw data testing failed\n", __func__);
	info->listnodenum++;
	msleep(100); /* sleep 100ms to wait for ic ready */
	/* test item 9 */
	retval = touch_driver_abs_noise_cap_test(info);
	if (retval < 0)
		TS_LOG_ERR("%s abs noise data testing failed\n", __func__);
	info->listnodenum++;

	retval = test_hcd->tcm_hcd->set_dynamic_config(test_hcd->tcm_hcd,
		DC_NO_DOZE, 0);
	if (retval < 0)
		TS_LOG_ERR("%s failed to set dc\n", __func__);
	TS_LOG_INFO("%s: after enable doze\n", __func__);
}

#define SYNC_TCM_RESET_DELAY 10
static int touch_driver_cap_test_for_newformat(
	struct ts_rawdata_info_new *info)
{
	int retval;
	unsigned int rows;
	unsigned int cols;
	struct syna_tcm_hcd *tcm_hcd = test_hcd->tcm_hcd;
	struct syna_tcm_app_info *app_info = NULL;
	struct ts_kit_platform_data *ts_platform_data = NULL;

	TS_LOG_INFO("%s called\n", __func__);

	app_info = &tcm_hcd->app_info;
	ts_platform_data =
		tcm_hcd->syna_tcm_chip_data->ts_platform_data;
	rows = le2_to_uint(app_info->num_of_image_rows);
	cols = le2_to_uint(app_info->num_of_image_cols);
	if (rows == 0 || cols == 0) {
		TS_LOG_ERR("rows or cols is NULL\n");
		return -EINVAL;
	}
	retval = touch_driver_i2c_communication_test(info);
	if (retval < 0) {
		TS_LOG_ERR("%s failed to set dc\n", __func__);
		return  -ENODEV;
	}
	touch_driver_get_thr_from_csvfile();
	tskit_driver_put_device_info(info);
	tskit_driver_enable_touch(tcm_hcd, false);
	touch_driver_cap_test_order(info);
	tskit_driver_enable_touch(tcm_hcd, true);

	info->tx = cols;
	info->rx = rows;

	gpio_set_value(ts_platform_data->reset_gpio, 0);
	msleep(SYNC_TCM_RESET_DELAY);
	gpio_set_value(ts_platform_data->reset_gpio, 1);
	return 0;
}

int tskit_driver_cap_test_init(struct syna_tcm_hcd *tcm_hcd)
{
	int retval = 0;

	if (!test_hcd) {
		test_hcd = kzalloc(sizeof(*test_hcd), GFP_KERNEL);
		if (!test_hcd) {
			TS_LOG_ERR("Failed to allocate memory for test_hcd\n");
			return -ENOMEM;
		}
	}
	if (!read_temp_buf) {
		read_temp_buf = kzalloc(READ_TEMP_BUF_SIZE *
			sizeof(unsigned char), GFP_KERNEL);
		if (!read_temp_buf) {
			TS_LOG_ERR("Failed to allocate memory for read_temp_buf\n");
			return -ENOMEM;
		}
	}
	if (!test_params) {
		test_params = vmalloc(sizeof(struct syna_tcm_test_params));
		if (!test_params) {
			TS_LOG_ERR("%s:alloc mem for params fail\n", __func__);
			retval = -ENOMEM;
			goto err_alloc_testing_hcd;
		}
		memset(test_params, 0, sizeof(struct syna_tcm_test_params));
	}

	test_hcd->tcm_hcd = tcm_hcd;

	return 0;

err_alloc_testing_hcd:
	kfree(test_hcd);
	test_hcd = NULL;

	return retval;
}


int tskit_driver_cap_test_remove(struct syna_tcm_hcd *tcm_hcd)
{
	kfree(test_hcd->satic_cfg_buf);
	test_hcd->satic_cfg_buf = NULL;

	kfree(test_hcd->out.buf);
	test_hcd->out.buf = NULL;
	kfree(test_hcd);
	test_hcd = NULL;
	kfree(read_temp_buf);
	read_temp_buf = NULL;
	kfree(test_params);
	test_params = NULL;
	return 0;
}

MODULE_AUTHOR("Synaptics, Inc.");
MODULE_DESCRIPTION("Synaptics TCM Testing Module");
MODULE_LICENSE("GPL v2");
