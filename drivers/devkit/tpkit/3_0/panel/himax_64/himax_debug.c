/* Himax Android Driver Sample Code for Himax chipset
*
* Copyright (C) 2017 Himax Corporation.
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/

#include "himax_ic.h"
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/kernel.h>

bool dsram_flag = false;
int flash_size = FW_SIZE_64K;
int self_test_nc_flag = 0;

#define COLUMNS_LEN 16
#define REG_COMMON_LEN 128
#define DIAG_COMMAND_MAX_SIZE 80
#define DIAG_INT_COMMAND_MAX_SIZE 12

#if defined(CONFIG_TOUCHSCREEN_HIMAX_DEBUG)
#ifdef HX_TP_SYS_DIAG
	static uint8_t x_channel = 0;
	static uint8_t y_channel = 0;
	static uint8_t diag_max_cnt = 0;
	static int g_switch_mode = 0;
	static int16_t *diag_self = NULL;
	static int16_t *diag_mutual = NULL;
	static int16_t *diag_mutual_new = NULL;
	static int16_t *diag_mutual_old = NULL;

	int selftest_flag = 0;
	int g_diag_command = 0;
	uint8_t write_counter = 0;
	uint8_t write_max_count = 30;
	struct file *diag_sram_fn;
	#define IIR_DUMP_FILE "/sdcard/HX_IIR_Dump.txt"
	#define DC_DUMP_FILE "/sdcard/HX_DC_Dump.txt"
	#define BANK_DUMP_FILE "/sdcard/HX_BANK_Dump.txt"
#endif

#ifdef HX_TP_SYS_REGISTER
	static uint8_t register_command[4] = {0};
#endif

#ifdef HX_TP_SYS_DEBUG
	uint8_t cmd_set[4] = {0};
	uint8_t mutual_set_flag = 0;
	static int handshaking_result = 0;
	static bool fw_update_complete = false;
	static unsigned char debug_level_cmd = 0;
	static unsigned char upgrade_fw[FW_SIZE_64K] = {0};
#endif

#ifdef HX_TP_SYS_FLASH_DUMP
	static uint8_t *flash_buffer = NULL;
	static uint8_t flash_command = 0;
	static uint8_t flash_read_step = 0;
	static uint8_t flash_progress = 0;
	static uint8_t flash_dump_complete = 0;
	static uint8_t flash_dump_fail = 0;
	static uint8_t sys_operation = 0;
	static bool flash_dump_going = false;

	static uint8_t get_flash_command(void);
	static uint8_t get_flash_dump_complete(void);
	static uint8_t get_flash_dump_fail(void);
	static uint8_t get_flash_dump_progress(void);
	static uint8_t get_flash_read_step(void);
	static uint8_t get_sys_operation(void);

	static void set_flash_command(uint8_t command);
	static void set_flash_read_step(uint8_t step);
	static void set_flash_dump_complete(uint8_t complete);
	static void set_flash_dump_fail(uint8_t fail);
	static void set_flash_dump_progress(uint8_t progress);
	static void set_flash_dump_going(bool going);
#endif
#endif

extern char himax_nc_project_id[];
extern int hx_nc_irq_enable_count;
extern struct himax_ts_data *g_himax_nc_ts_data;

extern int himax_nc_input_config(struct input_dev * input_dev);
extern int i2c_himax_nc_read(uint8_t command, uint8_t *data,
	uint16_t length, uint16_t limit_len, uint8_t toRetry);
extern int i2c_himax_nc_write(uint8_t command, uint8_t *data,
	uint16_t length, uint16_t limit_len, uint8_t toRetry);
extern void himax_nc_int_enable(int irqnum, int enable);
extern void himax_nc_register_read(uint8_t *read_addr,
	int read_length, uint8_t *read_data);
extern void himax_burst_enable(uint8_t auto_add_4_byte);
extern void himax_register_write(uint8_t *write_addr,
	int write_length, uint8_t *write_data);
extern void himax_flash_write_burst(uint8_t *reg_byte,
	uint8_t *write_data, size_t cnta, size_t cntb);
extern void himax_nc_rst_gpio_set(int pinnum, uint8_t value);
extern void himax_nc_reload_disable(int on);

#ifdef HX_TP_SYS_RESET
static ssize_t himax_reset_set(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int error = 0;

	if (buf[0] == '1') {
#ifndef CONFIG_HUAWEI_DEVKIT_MTK_3_0
		himax_nc_rst_gpio_set(g_himax_nc_ts_data->rst_gpio, 0);
		msleep(RESET_LOW_TIME);
		himax_nc_rst_gpio_set(g_himax_nc_ts_data->rst_gpio, 1);
		msleep(RESET_HIGH_TIME);
#else
		error = pinctrl_select_state(g_himax_nc_ts_data->pctrl,
			g_himax_nc_ts_data->pinctrl_state_reset_low);
		if (error < 0)
			TS_LOG_ERR("reset_set: Set reset pin low state error:%d\n",
				error);
		msleep(RESET_LOW_TIME);
		error = pinctrl_select_state(g_himax_nc_ts_data->pctrl,
			g_himax_nc_ts_data->pinctrl_state_reset_high);
		if (error < 0)
			TS_LOG_ERR("reset_set: Set reset pin high state error:%d\n",
				error);
		msleep(RESET_HIGH_TIME);
#endif
	}
	return count;
}
static DEVICE_ATTR(reset, (S_IWUSR|S_IRUGO), NULL, himax_reset_set);
#endif

#ifdef HX_TP_SYS_DIAG
int16_t *hx_nc_get_mutual_buffer(void)
{
	return diag_mutual;
}
int16_t *hx_nc_get_mutual_new_buffer(void)
{
	return diag_mutual_new;
}
int16_t *hx_nc_get_mutual_old_buffer(void)
{
	return diag_mutual_old;
}
int16_t *hx_nc_get_self_buffer(void)
{
	return diag_self;
}
int16_t hx_nc_get_x_channel(void)
{
	return x_channel;
}
int16_t hx_nc_get_y_channel(void)
{
	return y_channel;
}
int16_t hx_nc_get_diag_command(void)
{
	return g_diag_command;
}
void hx_nc_set_x_channel(uint8_t x)
{
	x_channel = x;
}
void hx_nc_set_y_channel(uint8_t y)
{
	y_channel = y;
}
void hx_nc_set_mutual_buffer(void)
{
	diag_mutual = kzalloc(x_channel * y_channel *
		sizeof(int16_t), GFP_KERNEL);
	if (diag_mutual == NULL)
		TS_LOG_ERR("set_mutual_buffer: kzalloc error\n");
}
void hx_nc_set_mutual_new_buffer(void)
{
	diag_mutual_new = kzalloc(x_channel * y_channel *
		sizeof(int16_t), GFP_KERNEL);
	if (diag_mutual_new == NULL)
		TS_LOG_ERR("set_mutual_new_buffer: kzalloc error\n");
}
void hx_nc_set_mutual_old_buffer(void)
{
	diag_mutual_old = kzalloc(x_channel * y_channel *
		sizeof(int16_t), GFP_KERNEL);
	if (diag_mutual_old == NULL)
		TS_LOG_ERR("set_mutual_old_buffer: kzalloc error\n");
}

void hx_nc_set_self_buffer(void)
{
	diag_self = kzalloc((x_channel + y_channel) *
		sizeof(int16_t), GFP_KERNEL);
	if (diag_self == NULL)
		TS_LOG_ERR("set_self_buffer: kzalloc error\n");
}
void hx_nc_free_mutual_buffer(void)
{
	if (diag_mutual)
		kfree(diag_mutual);
	diag_mutual = NULL;
}

void hx_nc_free_mutual_new_buffer(void)
{
	if (diag_mutual_new)
		kfree(diag_mutual_new);

	diag_mutual_new = NULL;
}

void hx_nc_free_mutual_old_buffer(void)
{
	if (diag_mutual_old)
		kfree(diag_mutual_old);

	diag_mutual_old = NULL;
}

void hx_nc_free_self_buffer(void)
{
	if (diag_self)
		kfree(diag_self);

	diag_self = NULL;
}
static int himax_determin_diag_rawdata(int diag_command)
{
	return diag_command % 10;
}
static int himax_determin_diag_storage(int diag_command)
{
	return diag_command / 10;
}
static ssize_t himax_diag_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	size_t count = 0;
	uint32_t loop_i;
	uint16_t flg;
	uint16_t width;
	uint16_t self_num;
	uint16_t mutual_num;
	int dsram_type;
	int loop_j;
	unsigned int index;
	uint8_t info_data_hx102b[MUTUL_NUM_HX83102 * 2] = {0};
	uint8_t info_data_hx112a[MUTUL_NUM_HX83112 * 2] = {0};
	int16_t new_data = 0;

	dsram_type = g_diag_command / 10;

	// check if devided by zero
	if (x_channel == 0) {
		TS_LOG_ERR("diag_show: devided by zero\n");
		return count;
	}

	mutual_num = x_channel * y_channel;
	self_num = x_channel + y_channel; // don't add KEY_COUNT
	width = x_channel;
	count += snprintf(buf, HX_MAX_PRBUF_SIZE - count,
		"ChannelStart: %4d, %4d\n\n", x_channel, y_channel);

	// start to show out the raw data in adb shell
	if ((g_diag_command >= 1 && g_diag_command <= 3) ||
		(g_diag_command == 7)) {
		for (loop_i = 0; loop_i < mutual_num; loop_i++) {
			count += snprintf(buf + count, HX_MAX_PRBUF_SIZE - count,
				"%6d", diag_mutual[loop_i]);
			if ((loop_i % width) == (width - 1)) {
				flg = width + loop_i / width;
				count += snprintf(buf + count, HX_MAX_PRBUF_SIZE - count,
					" %6d\n", diag_self[flg]);
			}
		}

		count += snprintf(buf + count, HX_MAX_PRBUF_SIZE - count, "\n");
		for (loop_i = 0; loop_i < width; loop_i++) {
			count += snprintf(buf + count, HX_MAX_PRBUF_SIZE - count,
				"%6d", diag_self[loop_i]);
			if (((loop_i) % width) == (width - 1))
				count += snprintf(buf + count, HX_MAX_PRBUF_SIZE - count, "\n");
		}
#ifdef HX_EN_SEL_BUTTON
		count += snprintf(buf, HX_MAX_PRBUF_SIZE - count, "\n");
		for (loop_i = 0; loop_i < hx_nc_bt_num; loop_i++)
			count += snprintf(buf, HX_MAX_PRBUF_SIZE - count, "%6d",
				diag_self[hx_nc_rx_num + hx_nc_tx_num + loop_i]);
#endif
		count += snprintf(buf, HX_MAX_PRBUF_SIZE - count, "ChannelEnd");
		count += snprintf(buf, HX_MAX_PRBUF_SIZE - count, "\n");
	} else if (g_diag_command == 8) {
		for (loop_i = 0; loop_i < 128; loop_i++) {
			if ((loop_i % 16) == 0)
				count += snprintf(buf, HX_MAX_PRBUF_SIZE - count, "LineStart:");
			count += snprintf(buf, HX_MAX_PRBUF_SIZE - count,
				"%4x", hx_nc_diag_coor[loop_i]);
			if ((loop_i % 16) == 15)
				count += snprintf(buf, HX_MAX_PRBUF_SIZE - count, "\n");
		}
	} else if (dsram_type > 0 && dsram_type <= 8) {
		himax_burst_enable(1);
		if (ic_nc_type == HX_83102B_SERIES_PWON)
			himax_nc_get_dsram_data(info_data_hx102b,
				sizeof(info_data_hx102b));
		else if (ic_nc_type == HX_83112A_SERIES_PWON)
			himax_nc_get_dsram_data(info_data_hx112a,
				sizeof(info_data_hx112a));
		for (loop_i = 0, index = 0; loop_i < hx_nc_tx_num; loop_i++) {
			for (loop_j = 0; loop_j < hx_nc_rx_num; loop_j++) {
				if (ic_nc_type == HX_83102B_SERIES_PWON)
					new_data = (short)((info_data_hx102b[index + 1] << 8) |
						info_data_hx102b[index]);
				else if (ic_nc_type == HX_83112A_SERIES_PWON)
					new_data = (short)((info_data_hx112a[index + 1] << 8) |
						info_data_hx112a[index]);
				if (dsram_type == 1)
					diag_mutual[loop_i * hx_nc_rx_num + loop_j] = new_data;
				index += 2;
			}
		}
		for (loop_i = 0; loop_i < mutual_num; loop_i++) {
			count += snprintf(buf + count, HX_MAX_PRBUF_SIZE - count,
				"%6d", diag_mutual[loop_i]);
			if ((loop_i % width) == (width - 1)) {
				flg = width + loop_i / width;
				count += snprintf(buf + count, HX_MAX_PRBUF_SIZE - count,
					" %6d\n", diag_self[flg]);
			}
		}

		count += snprintf(buf + count, HX_MAX_PRBUF_SIZE - count, "\n");
		for (loop_i = 0; loop_i < width; loop_i++) {
			count += snprintf(buf + count, HX_MAX_PRBUF_SIZE - count,
				"%6d", diag_self[loop_i]);
			if (((loop_i) % width) == (width - 1))
				count += snprintf(buf + count, HX_MAX_PRBUF_SIZE - count, "\n");
		}
		count += snprintf(buf, HX_MAX_PRBUF_SIZE - count, "ChannelEnd");
		count += snprintf(buf, HX_MAX_PRBUF_SIZE - count, "\n");
	}

	return count;
}

static ssize_t himax_diag_dump(struct device *dev,
	struct device_attribute *attr, const char *buff, size_t len)
{
	uint8_t command[2] = {0x00, 0x00};
	int storage_type; // 0: common , other: dsram
	int rawdata_type; // 1:IIR,2:DC
	struct himax_ts_data *ts;

	ts = g_himax_nc_ts_data;

	if (len >= DIAG_COMMAND_MAX_SIZE) {
		TS_LOG_INFO("diag_dump: no command exceeds 80 chars\n");
		return -EFAULT;
	}

	TS_LOG_INFO("diag_dump: g_switch_mode = %d\n", g_switch_mode);
	if (buff[1] == 0x0A) {
		g_diag_command = buff[0] - '0';
	} else {
		g_diag_command = (buff[0] - '0') * 10 + (buff[1] - '0');
	}

	storage_type = himax_determin_diag_storage(g_diag_command);
	rawdata_type = himax_determin_diag_rawdata(g_diag_command);

	if (g_diag_command > 0 && rawdata_type == 0) {
		TS_LOG_INFO("g_diag_command=0x%x,storage_type=%d\n",
			g_diag_command, storage_type);
		TS_LOG_INFO("rawdata_type=%d Maybe no support\n",
			rawdata_type);
		g_diag_command = 0x00;
	} else {
		TS_LOG_INFO("g_diag_command=0x%x,storage_type=%d,rawdata_type=%d\n",
			g_diag_command, storage_type, rawdata_type);
	}
	if (storage_type == 0 && rawdata_type > 0 && rawdata_type < 8) {
		TS_LOG_INFO("diag_dump: common\n");
		if (dsram_flag) {
			// 1. Clear DSRAM flag
			dsram_flag = false;
			// 2. Enable ISR
			himax_nc_int_enable(g_himax_nc_ts_data->tskit_himax_data->ts_platform_data->irq_id, 1);
			/* 3 FW leave sram and return to event stack */
			himax_nc_return_event_stack();
		}

		if (g_switch_mode == 2) {
			himax_nc_idle_mode(0);
			g_switch_mode = himax_nc_switch_mode(0);
		}

		if (g_diag_command == 0x04) {
			g_diag_command = 0x00;
			command[0] = 0x00;
		} else {
			command[0] = g_diag_command;
		}
		himax_nc_diag_register_set(command[0]);
	} else if (storage_type > 0 && storage_type < 8 &&
		rawdata_type > 0 && rawdata_type < 8) {
		TS_LOG_INFO("diag_dump: dsram\n");
		diag_max_cnt = 0;
		memset(diag_mutual, 0x00, x_channel * y_channel * sizeof(int16_t));

		// 0. set diag flag
		if (dsram_flag) {
			// (1) Clear DSRAM flag
			dsram_flag = false;
			//(2) Enable ISR
			himax_nc_int_enable(g_himax_nc_ts_data->tskit_himax_data->ts_platform_data->irq_id, 1);
			/* (3) FW leave sram and return to event stack */
			himax_nc_return_event_stack();
		}
		/* close sorting if turn on */
		if (g_switch_mode == 2) {
			himax_nc_idle_mode(0);
			g_switch_mode = himax_nc_switch_mode(0);
		}

		switch(rawdata_type) {
		case 1:
			command[0] = 0x09;  // IIR
			break;
		case 2:
			command[0] = 0x0A;  // RAWDATA
			break;
		case 7:
			command[0] = 0x0B;   // DC
			break;
		default:
			command[0] = 0x00;
			TS_LOG_ERR("diag_dump: Sram no support this type\n");
			break;
		}
		himax_nc_diag_register_set(command[0]);
		// 1. Disable ISR
		himax_nc_int_enable(g_himax_nc_ts_data->tskit_himax_data->ts_platform_data->irq_id, 0);
		// Open file for save raw data log

		// 3. Set DSRAM flag
		dsram_flag = true;
	} else if (storage_type == 8) {
		TS_LOG_INFO("Soritng mode\n");
		if (dsram_flag) {
			// 1. Clear DSRAM flag
			dsram_flag = false;
			// 2. Enable ISR
			himax_nc_int_enable(g_himax_nc_ts_data->tskit_himax_data->ts_platform_data->irq_id, 1);
			// 3. FW leave sram and return to event stack
			himax_nc_return_event_stack();
		}

		himax_nc_idle_mode(1);
		g_switch_mode = himax_nc_switch_mode(1);
		if (g_switch_mode == 2) {
			if (rawdata_type == 1) {
				command[0] = 0x09; // IIR
			} else if (rawdata_type == 2) {
				command[0] = 0x0A; // DC
			} else if (rawdata_type == 7) {
				command[0] = 0x08; // BASLINE
			} else {
				command[0] = 0x00;
				TS_LOG_ERR("diag_dump: Now Sorting Mode does not support this command=%d\n",
					g_diag_command);
			}
			himax_nc_diag_register_set(command[0]);
		}
		dsram_flag = true;
	} else {
		// set diag flag
		if (dsram_flag) {
			TS_LOG_INFO("return and cancel sram thread\n");
			// 1. Clear DSRAM flag
			dsram_flag = false;
			// 2. Enable ISR
			himax_nc_int_enable(g_himax_nc_ts_data->tskit_himax_data->ts_platform_data->irq_id, 1);
			// 3. FW leave sram and return to event stack
			himax_nc_return_event_stack();
		}

		if (g_switch_mode == 2) {
			himax_nc_idle_mode(0);
			g_switch_mode = himax_nc_switch_mode(0);
		}

		if (g_diag_command != 0x00) {
			TS_LOG_ERR("g_diag_command error!diag_command=0x%x so reset\n",
				g_diag_command);
			command[0] = 0x00;
			if (g_diag_command != 0x08)
				g_diag_command = 0x00;
			himax_nc_diag_register_set(command[0]);
		} else {
			command[0] = 0x00;
			g_diag_command = 0x00;
			himax_nc_diag_register_set(command[0]);
			TS_LOG_INFO("return to normal g_diag_command=0x%x\n", g_diag_command);
		}
	}
	return len;
}

static DEVICE_ATTR(diag, (S_IWUSR|S_IRUGO), himax_diag_show, himax_diag_dump); //Debug_diag_done

#endif

#ifdef HX_TP_SYS_REGISTER
static ssize_t himax_register_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = 0;
	uint16_t loop_i;
	uint16_t row_width = 16;
	uint8_t data[REG_COMMON_LEN] = {0};

	TS_LOG_INFO("himax_register_show: %02X,%02X,%02X,%02X\n",
		register_command[3], register_command[2], register_command[1],
		register_command[0]);
	himax_nc_register_read(register_command, REG_COMMON_LEN, data);
	ret += snprintf(buf, HX_MAX_PRBUF_SIZE - ret,
		"command:%02X,%02X,%02X,%02X\n", register_command[3],
		register_command[2], register_command[1], register_command[0]);

	for (loop_i = 0; loop_i < REG_COMMON_LEN; loop_i++) {
		ret += snprintf(buf + ret, HX_MAX_PRBUF_SIZE - ret,
			"0x%2.2X ", data[loop_i]);
		if ((loop_i % row_width) == (row_width - 1))
			ret += snprintf(buf + ret, HX_MAX_PRBUF_SIZE - ret, "\n");
	}
	ret += snprintf(buf + ret, HX_MAX_PRBUF_SIZE - ret, "\n");

	return ret;
}

static ssize_t himax_register_store(struct device *dev,
	struct device_attribute *attr, const char *buff, size_t count)
{
	char buf_tmp[DIAG_COMMAND_MAX_SIZE] = {0};
	char *data_str = NULL;
	uint8_t length = 0;
	uint8_t loop_i;
	uint8_t flg_cnt = 0;
	uint8_t byte_length = 0;
	uint8_t w_data[DIAG_COMMAND_MAX_SIZE] = {0};
	uint8_t x_pos[DIAG_COMMAND_MAX_SIZE] = {0};
	unsigned long result = 0;


	TS_LOG_INFO("register_store: buff = %s, line = %d\n",
		buff, __LINE__);
	if (count >= DIAG_COMMAND_MAX_SIZE) {
		TS_LOG_INFO("register_store: no command exceeds 80 chars.\n");
		return -EFAULT;
	}
	TS_LOG_INFO("register_store: buff = %s, line = %d, %p\n",
		buff, __LINE__, &buff[0]);

	if ((buff[0] == 'r' || buff[0] == 'w') &&
		buff[1] == ':' && buff[2] == 'x') {
		length = strlen(buff);
		TS_LOG_INFO("register_store: length = %d\n", length);
		for (loop_i = 0; loop_i < length; loop_i++) {
			if (buff[loop_i] == 'x') {
				x_pos[flg_cnt] = loop_i;
				flg_cnt++;
			}
		}
		TS_LOG_INFO("register_store: flg_cnt = %d\n", flg_cnt);

		data_str = strrchr(buff, 'x');
		TS_LOG_INFO("register_store: %s\n", data_str);
		length = strlen(data_str + 1) - 1;

		if (buff[0] == 'r') {
			if (length <= DIAG_COMMAND_MAX_SIZE)
				memcpy(buf_tmp, data_str + 1, length);
			else
				TS_LOG_ERR("%s: length more than 80 byte\n",
					__func__);
			byte_length = length / 2;
			if (!kstrtoul(buf_tmp, 16, &result)) {
				for (loop_i = 0 ; loop_i < byte_length ; loop_i++)
					register_command[loop_i] = (uint8_t)(result >> loop_i * 8);
			}
			TS_LOG_INFO("register_store: buff[0] == 'r'\n");
		} else if (buff[0] == 'w') {
			if (length <= DIAG_COMMAND_MAX_SIZE)
				memcpy(buf_tmp, buff + 3, length);
			else
				TS_LOG_ERR("length biger than buf_tmp size\n");
			if (flg_cnt < 3) {
				byte_length = length / 2;
				if (!kstrtoul(buf_tmp, 16, &result)) {
					for (loop_i = 0 ; loop_i < byte_length ; loop_i++)
						register_command[loop_i] =
							(uint8_t)(result >> loop_i * 8);
				}
				if (!kstrtoul(data_str + 1, 16, &result)) {
					for (loop_i = 0 ; loop_i < byte_length ; loop_i++)
						w_data[loop_i] = (uint8_t)(result >> loop_i * 8);
				}
				himax_register_write(register_command, byte_length, w_data);
			TS_LOG_INFO("register_store: buff[0] == 'w' && flg_cnt < 3\n");
			} else {
				byte_length = x_pos[1] - x_pos[0] - 2;
				for (loop_i = 0; loop_i < flg_cnt; loop_i++) {
					memcpy(buf_tmp, buff + x_pos[loop_i] + 1, byte_length);
					if (!kstrtoul(buf_tmp, 16, &result)) {
						if (loop_i == 0) {
							register_command[loop_i] = (uint8_t)(result);
							TS_LOG_INFO("register_store: register_command = %X\n",
								register_command[0]);
						} else {
							w_data[loop_i - 1] = (uint8_t)(result);
							TS_LOG_INFO("register_store: w_data[%d] = %2X\n",
								loop_i - 1, w_data[loop_i - 1]);
						}
					}
				}
				byte_length = flg_cnt - 1;
				himax_register_write(register_command, byte_length, &w_data[0]);
			TS_LOG_INFO("register_store: buff[0] == 'w' && flg_cnt >= 3\n");
			}
		} else {
			return count;
		}
	}

	return count;
}

static DEVICE_ATTR(register, (S_IWUSR|S_IRUGO),
	himax_register_show, himax_register_store); // Debug_register_done

#endif
#ifdef HX_TP_SYS_DEBUG

static ssize_t himax_debug_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	size_t retval = 0;

	if (debug_level_cmd == 't') {
		if (fw_update_complete) {
			retval += snprintf(buf, HX_MAX_PRBUF_SIZE - retval,
				"FW Update Complete ");
		} else {
			retval += snprintf(buf, HX_MAX_PRBUF_SIZE - retval,
				"FW Update Fail ");
		}
	} else if (debug_level_cmd == 'v') {
		himax_nc_read_tp_info();
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "FW_VER = ");
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval,
			"0x%2.2X \n", g_himax_nc_ts_data->vendor_fw_ver);
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval,
			"CONFIG_VER = ");
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval,
			"0x%2.2X \n", g_himax_nc_ts_data->vendor_config_ver);
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval,
			"CID_VER = ");
		retval += snprintf(buf+retval, HX_MAX_PRBUF_SIZE - retval,
			"0x%2.2X\n", ((g_himax_nc_ts_data->vendor_cid_maj_ver
			<< LEFT_MOV_8BIT) | g_himax_nc_ts_data->vendor_cid_min_ver));
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "\n");
	} else if (debug_level_cmd == 'd') {
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval,
			"Himax Touch IC Information :\n");
		if (ic_nc_type == HX_85XX_D_SERIES_PWON) {
			retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval,
				"IC Type : D\n");
		} else if (ic_nc_type == HX_85XX_E_SERIES_PWON) {
			retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval,
				"IC Type : E\n");
		} else if (ic_nc_type == HX_85XX_ES_SERIES_PWON) {
			retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval,
				"IC Type : ES\n");
		} else if (ic_nc_type == HX_85XX_F_SERIES_PWON) {
			retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval,
				"IC Type : F\n");
		} else if (ic_nc_type == HX_83102B_SERIES_PWON) {
			retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval,
				"IC Type : HX83102B\n");
		} else if (ic_nc_type == HX_83102E_SERIES_PWON) {
			retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval,
				"IC Type : HX83102E\n");
		} else {
			retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval,
				"IC Type error.\n");
		}

		if (ic_nc_checksum == HX_TP_BIN_CHECKSUM_SW) {
			retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval,
				"IC Checksum : SW\n");
		} else if (ic_nc_checksum == HX_TP_BIN_CHECKSUM_HW) {
			retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval,
				"IC Checksum : HW\n");
		} else if (ic_nc_checksum == HX_TP_BIN_CHECKSUM_CRC) {
			retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval,
				"IC Checksum : CRC\n");
		} else {
			retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval,
				"IC Checksum error.\n");
		}

		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval,
			"RX Num : %d\n", hx_nc_rx_num);
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval,
			"TX Num : %d\n", hx_nc_tx_num);
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval,
			"BT Num : %d\n", hx_nc_bt_num);
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval,
			"X Resolution : %d\n", hx_nc_x_res);
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval,
			"Y Resolution : %d\n", hx_nc_y_res);
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval,
			"Max Point : %d\n", hx_nc_max_pt);
	} else if (debug_level_cmd == 'i') {
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval,
			"Himax Touch Driver Version:\n");
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval,
			"%s \n", HIMAX_DRIVER_VER);
	}
	return retval;
}

static ssize_t himax_debug_dump(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int result = 0;
	char filename[REG_COMMON_LEN] = {0};
	mm_segment_t oldfs;
	struct file* hx_filp = NULL;

	TS_LOG_INFO("debug_dump: enter\n");
	if (count >= DIAG_COMMAND_MAX_SIZE) {
		TS_LOG_INFO("debug_dump: no command exceeds 80 chars\n");
		return -EFAULT;
	}

	if (buf[0] == 'v') {
		debug_level_cmd = buf[0];
		return count;
	} else if (buf[0] == 'd') {
		debug_level_cmd = buf[0];
		return count;
	} else if (buf[0] == 'i') {
		debug_level_cmd = buf[0];
		return count;
	} else if (buf[0] == 't') {
		himax_nc_int_enable(g_himax_nc_ts_data->tskit_himax_data->ts_platform_data->irq_id, 0);
		debug_level_cmd = buf[0];
		fw_update_complete = false;
		memset(filename, 0, sizeof(filename));
		// parse the file name
		snprintf(filename, count - 2, "%s", &buf[2]);
		TS_LOG_INFO("debug_dump: upgrade from file(%s) start\n",
			filename);
		// open file
		hx_filp = filp_open(filename, O_RDONLY, 0);
		if (IS_ERR(hx_filp)) {
			TS_LOG_ERR("debug_dump: open firmware file failed\n");
			goto firmware_upgrade_done;
		}
		oldfs = get_fs();
		/* lint -save -e* */
		set_fs(KERNEL_DS);
		/* lint -restore */
		// read the latest firmware binary file
		result = hx_filp->f_op->read(hx_filp, upgrade_fw,
			sizeof(upgrade_fw), &hx_filp->f_pos);
		if (result < 0) {
			TS_LOG_ERR("debug_dump: read firmware file failed\n");
			set_fs(oldfs);
			goto firmware_upgrade_done;
		}
		set_fs(oldfs);
		filp_close(hx_filp, NULL);
		TS_LOG_INFO("debug_dump: upgrade start,len %d: %02X, %02X, %02X, %02X\n",
			result, upgrade_fw[0], upgrade_fw[1], upgrade_fw[2],
			upgrade_fw[3]);

		if (result > 0) {
			// start to upgrade
			himax_nc_hw_reset(HX_LOADCONFIG_DISABLE, HX_INT_EN);;
			if (hx_nc_fts_ctpm_fw_upgrade_with_fs(upgrade_fw, result, true) == 0) {
				TS_LOG_ERR("debug_dump: TP upgrade error, line: %d\n", __LINE__);
				fw_update_complete = false;
			} else {
				TS_LOG_INFO("debug_dump: TP upgrade OK, line: %d\n", __LINE__);
				himax_nc_reload_disable(0);
				fw_update_complete = true;
				g_himax_nc_ts_data->vendor_fw_ver =
					((upgrade_fw[nc_fw_ver_maj_flash_addr] << 8) |
					upgrade_fw[nc_fw_ver_min_flash_addr]);
				g_himax_nc_ts_data->vendor_config_ver =
					(upgrade_fw[nc_cfg_ver_maj_flash_addr] << 8) |
					upgrade_fw[nc_cfg_ver_min_flash_addr];
			}
			himax_nc_hw_reset(HX_LOADCONFIG_EN, HX_INT_EN);
			goto firmware_upgrade_done;
		}
	}

firmware_upgrade_done:
	himax_nc_int_enable(g_himax_nc_ts_data->tskit_himax_data->ts_platform_data->irq_id, 1);

	return count;
}

static DEVICE_ATTR(debug, (S_IWUSR|S_IRUGO),
	himax_debug_show, himax_debug_dump); // Debug_debug_done
#endif

#ifdef HX_TP_SYS_FLASH_DUMP

static uint8_t get_flash_command(void)
{
	return flash_command;
}

static uint8_t get_flash_dump_progress(void)
{
	return flash_progress;
}

static uint8_t get_flash_dump_complete(void)
{
	return flash_dump_complete;
}

static uint8_t get_flash_dump_fail(void)
{
	return flash_dump_fail;
}

static uint8_t get_sys_operation(void)
{
	return sys_operation;
}

static uint8_t get_flash_read_step(void)
{
	return flash_read_step;
}

bool hx_nc_get_flash_dump_going(void)
{
	return flash_dump_going;
}

void hx_nc_set_flash_buffer(void)
{
	flash_buffer = kzalloc(FLASH_SIZE * sizeof(uint8_t), GFP_KERNEL);
	memset(flash_buffer, 0x00, FLASH_SIZE);
}

void hx_nc_free_flash_buffer(void)
{
	if (flash_buffer)
		kfree(flash_buffer);
	flash_buffer = NULL;
}

void hx_nc_set_sys_operation(uint8_t operation)
{
	sys_operation = operation;
}

static void set_flash_dump_progress(uint8_t progress)
{
	flash_progress = progress;
}

static void set_flash_dump_complete(uint8_t status)
{
	flash_dump_complete = status;
}

static void set_flash_dump_fail(uint8_t fail)
{
	flash_dump_fail = fail;
}

static void set_flash_command(uint8_t command)
{
	flash_command = command;
}

static void set_flash_read_step(uint8_t step)
{
	flash_read_step = step;
}

static void set_flash_dump_going(bool going)
{
	flash_dump_going = going;
}

static void himax_ts_flash_print_func(void)
{
	int i;
	int cloum = COLUMNS_LEN;

	for (i = 0; i < flash_size; i++) {
		if (i % cloum == 0) {
			if (flash_buffer[i] < COLUMNS_LEN)
				printk("hx_flash_dump: 0x0%X,", flash_buffer[i]);
			else
				printk("hx_flash_dump: 0x%2X,", flash_buffer[i]);
		} else {
			if (flash_buffer[i] < COLUMNS_LEN)
				printk("0x0%X,", flash_buffer[i]);
			else
				printk("0x%2X,", flash_buffer[i]);
			if (i % cloum == (cloum - 1))
				printk("\n");
		}
	}
}

static void himax_ts_flash_func(void)
{
	uint8_t local_flash_command;
	mm_segment_t old_fs;

	himax_nc_int_enable(g_himax_nc_ts_data->tskit_himax_data->ts_platform_data->irq_id, 0);
	set_flash_dump_going(true);
	local_flash_command = get_flash_command();
	msleep(HX_SLEEP_100MS);
	TS_LOG_INFO("flash_func: local_flash_command = %d enter\n",
		local_flash_command);
	if ((local_flash_command == 1 || local_flash_command == 2) ||
		(local_flash_command == 0x0F))
		himax_nc_flash_dump_func(local_flash_command, flash_size,
			flash_buffer);

	if (local_flash_command == 1)
		himax_ts_flash_print_func();
	TS_LOG_INFO("Complete~~~~~~~~~~~~~~~~~~~~~~~\n");

	himax_nc_int_enable(g_himax_nc_ts_data->tskit_himax_data->ts_platform_data->irq_id, 1);
	set_flash_dump_going(false);
	set_flash_dump_complete(1);
	hx_nc_set_sys_operation(0);

	return;
}

void himax_nc_ts_flash_work_func(struct work_struct *work)
{
	himax_ts_flash_func();
}

static ssize_t himax_flash_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int flg = 0;
	int retval = 0;
	int loop_i;
	uint8_t local_flash_fail;
	uint8_t local_flash_progress;
	uint8_t local_flash_complete;
	uint8_t local_flash_command;
	uint8_t local_flash_read_step;

	local_flash_complete = get_flash_dump_complete();
	local_flash_progress = get_flash_dump_progress();
	local_flash_command = get_flash_command();
	local_flash_fail = get_flash_dump_fail();
	TS_LOG_INFO("flash_progress = %d \n", local_flash_progress);
	if (local_flash_fail) {
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval,
			"FlashStart:Fail\n");
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval,
			"FlashEnd");
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "\n");
		return retval;
	}

	if (!local_flash_complete) {
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval,
			"FlashStart:Ongoing:0x%2.2x \n", flash_progress);
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval,
			"FlashEnd");
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "\n");
		return retval;
	}

	if (local_flash_command == 1 && local_flash_complete) {
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval,
			"FlashStart:Complete \n");
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval,
			"FlashEnd");
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "\n");
		return retval;
	}

	if (local_flash_command == 3 && local_flash_complete) {
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval,
			"FlashStart:\n");
		for (loop_i = 0; loop_i < 128; loop_i++) {
			retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval,
				"x%2.2x", flash_buffer[loop_i]);
			if ((loop_i % 16) == 15)
				retval += snprintf(buf + retval,
					HX_MAX_PRBUF_SIZE - retval, "\n");
		}
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval,
			"FlashEnd");
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "\n");
		return retval;
	}

	// flash command == 0 , report the data
	local_flash_read_step = get_flash_read_step();
	retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval,
		"FlashStart:%2.2x \n", local_flash_read_step);
	for (loop_i = 0; loop_i < 1024; loop_i++) {
		flg = local_flash_read_step * 1024 + loop_i;
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval,
			"x%2.2X", flash_buffer[flg]);
		if ((loop_i % 16) == 15)
			retval += snprintf(buf + retval,
				HX_MAX_PRBUF_SIZE - retval, "\n");
	}

	retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "FlashEnd");
	retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "\n");
	return retval;
}

static ssize_t himax_flash_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	char buf_tmp[6] = {0};
	unsigned long result = 0;

	if (count >= DIAG_COMMAND_MAX_SIZE) {
		TS_LOG_INFO("flash_store: no command exceeds 80 chars\n");
		return -EFAULT;
	}
	TS_LOG_INFO("flash_store: buf = %s\n", buf);
	if (get_sys_operation() == 1) {
		TS_LOG_ERR("flash_store: PROC is busy , return\n");
		return count;
	}

	if (buf[0] == '0') {
		set_flash_command(0);
		if (buf[1] == ':' && buf[2] == 'x') {
			memcpy(buf_tmp, buf + 3, 2);
			TS_LOG_INFO("flash_store: read_Step = %s\n", buf_tmp);
			if (!kstrtoul(buf_tmp, 16, &result)) {
				TS_LOG_INFO("flash_store: read_Step = %lu\n", result);
				set_flash_read_step(result);
			}
		}
	} else if (buf[0] == '1') {
		// 1_32,1_60,1_64,1_24,1_28 for flash size 32k,60k,64k,124k,128k
		hx_nc_set_sys_operation(1);
		set_flash_command(1);
		set_flash_dump_progress(0);
		set_flash_dump_complete(0);
		set_flash_dump_fail(0);
		if ((buf[1] == '_') && (buf[2] == '3') && (buf[3] == '2')) {
			flash_size = FW_SIZE_32K;
		} else if ((buf[1] == '_') && (buf[2] == '6')) {
			if (buf[3] == '0') {
				flash_size = FW_SIZE_60K;
			} else if (buf[3] == '4') {
				flash_size = FW_SIZE_64K;
			}
		} else if ((buf[1] == '_') && (buf[2] == '2')) {
			if (buf[3] == '4') {
				flash_size = FW_SIZE_124K;
			} else if (buf[3] == '8') {
				flash_size = FW_SIZE_128K;
			}
		}
		TS_LOG_INFO("flash_store: command = 1, flash_size = %d\n",
			flash_size);
		queue_work(g_himax_nc_ts_data->flash_wq,
			&g_himax_nc_ts_data->flash_work);
	} else if (buf[0] == '2') {
		// 2_32,2_60,2_64,2_24,2_28 for flash size 32k,60k,64k,124k,128k
		hx_nc_set_sys_operation(1);
		set_flash_command(2);
		set_flash_dump_progress(0);
		set_flash_dump_complete(0);
		set_flash_dump_fail(0);
		if ((buf[1] == '_') && (buf[2] == '3') && (buf[3] == '2')) {
			flash_size = FW_SIZE_32K;
		} else if ((buf[1] == '_') && (buf[2] == '6')) {
			if (buf[3] == '0') {
				flash_size = FW_SIZE_60K;
			} else if (buf[3] == '4') {
				flash_size = FW_SIZE_64K;
			}
		} else if ((buf[1] == '_') && (buf[2] == '2')) {
			if (buf[3] == '4') {
				flash_size = FW_SIZE_124K;
			} else if (buf[3] == '8') {
				flash_size = FW_SIZE_128K;
			}
		}
		TS_LOG_INFO("flash_store: command = 2, flash_size = %d\n",
			flash_size);
		queue_work(g_himax_nc_ts_data->flash_wq,
			&g_himax_nc_ts_data->flash_work);
	}
	return count;
}
static DEVICE_ATTR(flash_dump, (S_IWUSR|S_IRUGO),
	himax_flash_show, himax_flash_store); // Debug_flash_dump_done
#endif

#if defined(CONFIG_TOUCHSCREEN_HIMAX_DEBUG)
static struct kobject *android_touch_kobj;
static struct dentry *himax_debugfs_dir;
struct dentry *diag_seq_dentry;
#endif
static ssize_t himax_debug_level_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	ssize_t retval = 0;
	struct himax_ts_data *ts_data;
	ts_data = g_himax_nc_ts_data;
	retval += snprintf(buf, HX_MAX_PRBUF_SIZE - retval,
		"%d\n", ts_data->debug_log_level);
	return retval;
}

static ssize_t himax_debug_level_dump(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int loop_i;
	struct himax_ts_data *ts = NULL;
	int flg = (int)count;
	ts = g_himax_nc_ts_data;
	ts->debug_log_level = 0;
	for (loop_i = 0; loop_i < flg - 1; loop_i++) {
		if (buf[loop_i] >= '0' && buf[loop_i] <= '9')
			ts->debug_log_level |= (buf[loop_i] - '0');
		else if (buf[loop_i] >= 'A' && buf[loop_i] <= 'F')
			ts->debug_log_level |= (buf[loop_i] - 'A' + 10);
		else if (buf[loop_i] >= 'a' && buf[loop_i] <= 'f')
			ts->debug_log_level |= (buf[loop_i] - 'a' + 10);
		if (loop_i != flg - 2)
			ts->debug_log_level <<= 4;
	}

	if (ts->debug_log_level & BIT(3)) {
		if (ts->pdata->screen_width > 0 && ts->pdata->screen_height > 0 &&
		 (ts->pdata->abs_x_max - ts->pdata->abs_x_min) > 0 &&
		 (ts->pdata->abs_y_max - ts->pdata->abs_y_min) > 0) {
			ts->width_factor = (ts->pdata->screen_width << SHIFTBITS) /
				(ts->pdata->abs_x_max - ts->pdata->abs_x_min);
			ts->height_factor = (ts->pdata->screen_height << SHIFTBITS) /
				(ts->pdata->abs_y_max - ts->pdata->abs_y_min);
			if (ts->width_factor > 0 && ts->height_factor > 0) {
				ts->use_screen_res = 1;
			} else {
				ts->height_factor = 0;
				ts->width_factor = 0;
				ts->use_screen_res = 0;
			}
		} else {
			TS_LOG_INFO("Enable finger debug with raw position mode!\n");
		}
	} else {
		ts->use_screen_res = 0;
		ts->width_factor = 0;
		ts->height_factor = 0;
	}
	return count;
}

static DEVICE_ATTR(debug_level, (S_IWUSR|S_IRUGO),
	himax_debug_level_show, himax_debug_level_dump);


static ssize_t touch_vendor_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t retval = 0;
	struct himax_ts_data *ts_data;
	ts_data = g_himax_nc_ts_data;
	himax_nc_read_tp_info();
	retval += snprintf(buf, HX_MAX_PRBUF_SIZE - retval,
		"%s_FW:%#x_CFG:%#x_ProjectId:%s\n",
		ts_data->tskit_himax_data->ts_platform_data->chip_data->chip_name,
		ts_data->vendor_fw_ver, ts_data->vendor_config_ver,
		himax_nc_project_id);
	return retval;
}

static DEVICE_ATTR(vendor, (S_IRUGO), touch_vendor_show, NULL); // Debug_vendor_done

static ssize_t touch_attn_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t retval = 0;
	return retval;
}

static DEVICE_ATTR(attn, (S_IRUGO), touch_attn_show, NULL); // Debug_attn_done
static ssize_t himax_int_status_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	size_t retval = 0;
	retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval,
		"%d ", hx_nc_irq_enable_count);
	retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "\n");
	return retval;
}

static ssize_t himax_int_status_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int value = 0;
	struct himax_ts_data *ts = g_himax_nc_ts_data;

	if (count >= DIAG_INT_COMMAND_MAX_SIZE) {
		TS_LOG_INFO("int_status_store: no command exceeds 12 chars\n");
		return -EFAULT;
	}
	if (buf[0] == '0')
		value = false;
	else if (buf[0] == '1')
		value = true;
	else
		return -EINVAL;
	if (value) {
		himax_nc_int_enable(ts->tskit_himax_data->ts_platform_data->irq_id, 1);
		ts->irq_enabled = 1;
		hx_nc_irq_enable_count = 1;
	} else {
		himax_nc_int_enable(ts->tskit_himax_data->ts_platform_data->irq_id, 0);
		ts->irq_enabled = 0;
		hx_nc_irq_enable_count = 0;
	}
	return count;
}

static DEVICE_ATTR(int_en, (S_IWUSR|S_IRUGO), // Debug_int_en_done
	himax_int_status_show, himax_int_status_store);

static ssize_t himax_sns_onoff_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	size_t retval = 0;

	TS_LOG_INFO("sns_onoff_show: enter\n");
	return retval;
}

static ssize_t himax_sns_onoff_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int value = 0;

	if (count >= DIAG_INT_COMMAND_MAX_SIZE) {
		TS_LOG_INFO("sns_onoff_store: no command exceeds 12 chars.\n");
		return -EFAULT;
	}
	if (buf[DATA_0] == '0')
		value = false;
	else if (buf[DATA_0] == '1')
		value = true;
	else
		return -EINVAL;
	if (value)
		himax_nc_sense_on(SET_OFF);
	else
		himax_nc_sense_off();
	return count;
}

static DEVICE_ATTR(sns_onoff, 0644,
	himax_sns_onoff_show, himax_sns_onoff_store);

static void himax_storage0_diag0_8_func(void)
{
	uint8_t command[DATA_2];
	struct ts_kit_device_data *ts;

	ts = g_himax_nc_ts_data->tskit_himax_data;
	TS_LOG_INFO("storage0_diag0_8_func: common\n");
	if (dsram_flag) {
		// 1. Clear DSRAM flag
		dsram_flag = false;
		// 2. Enable ISR
		himax_nc_int_enable(ts->ts_platform_data->irq_id, SET_ON);
		/* 3 FW leave sram and return to event stack */
		himax_nc_return_event_stack();
	}
	if (g_switch_mode == SORTING_MODE) {
		himax_nc_idle_mode(SET_OFF);
		g_switch_mode = himax_nc_switch_mode(SET_OFF);
	}
	if (g_diag_command == RAWDATA_DIAG_ADC_CALIBRATION) {
		g_diag_command = RAWDATA_DIAG_CMD_NONE;
		command[DATA_0] = RAWDATA_TPYE_NONE;
	} else {
		command[DATA_0] = g_diag_command;
	}
	himax_nc_diag_register_set(command[DATA_0]);
}

static void himax_storage0_8_diag0_8_func(int rawdata_type)
{
	uint8_t command[DATA_2];
	struct ts_kit_device_data *ts;

	ts = g_himax_nc_ts_data->tskit_himax_data;
	diag_max_cnt = DATA_0;
	TS_LOG_INFO("storage0_8_diag0_8_func: dsram\n");
	memset(diag_mutual, 0x00, (x_channel * y_channel * sizeof(int16_t)));
	// 0. set diag flag
	if (dsram_flag) {
		// (1) Clear DSRAM flag
		dsram_flag = false;
		// (2) Enable ISR
		himax_nc_int_enable(ts->ts_platform_data->irq_id, SET_ON);
		/* (3) FW leave sram and return to event stack */
		himax_nc_return_event_stack();
	}
	/* close sorting if turn on */
	if (g_switch_mode == SORTING_MODE) {
		himax_nc_idle_mode(SET_OFF);
		g_switch_mode = himax_nc_switch_mode(SET_OFF);
	}
	// 1. Disable ISR
	himax_nc_int_enable(ts->ts_platform_data->irq_id, SET_OFF);
	/* 2. Determin the diag cmd */
	switch (rawdata_type) {
	case USER_RAWDATA_CMD1:
		command[DATA_0] = RAWDATA_TPYE_IIR_DIFF;  // IIR
		break;
	case USER_RAWDATA_CMD2:
		command[DATA_0] = RAWDATA_TPYE_DC;  // RAWDATA
		break;
	case USER_RAWDATA_CMD3:
		command[DATA_0] = RAWDATA_TPYE_PEN_IIR;   // pen iir
		break;
	case USER_RAWDATA_CMD4:
		command[DATA_0] = RAWDATA_TPYE_PEN_DC;   // pen DC
		break;
	case USER_RAWDATA_CMD7:
		command[DATA_0] = RAWDATA_TPYE_BASELINE2;   // DC
		break;
	default:
		command[DATA_0] = RAWDATA_TPYE_NONE;
		TS_LOG_ERR("storage0_8_diag0_8_func: Sram no support this type\n");
		break;
	}
	himax_nc_diag_register_set(command[DATA_0]);
	// 3. Set DSRAM flag
	dsram_flag = true;
}

static void himax_storage8_diag_func(int rawdata_type)
{
	uint8_t command[DATA_2];
	struct ts_kit_device_data *ts;

	ts = g_himax_nc_ts_data->tskit_himax_data;
	TS_LOG_INFO("Soritng mode!\n");
	if (dsram_flag) {
		// 1. Clear DSRAM flag
		dsram_flag = false;
		// 2. Enable ISR
		himax_nc_int_enable(ts->ts_platform_data->irq_id, SET_ON);
		// 3. FW leave sram and return to event stack
		himax_nc_return_event_stack();
	}
	himax_nc_idle_mode(SET_ON);
	g_switch_mode = himax_nc_switch_mode(SET_ON);
	if (g_switch_mode == SORTING_MODE) {
		if (rawdata_type == USER_RAWDATA_CMD1) {
			command[DATA_0] = RAWDATA_TPYE_IIR_DIFF; // IIR
		} else if (rawdata_type == USER_RAWDATA_CMD2) {
			command[DATA_0] = RAWDATA_TPYE_DC; // DC
		} else if (rawdata_type == USER_RAWDATA_CMD7) {
			command[DATA_0] = RAWDATA_TPYE_BASELINE; // BASLINE
		} else {
			command[DATA_0] = RAWDATA_TPYE_NONE;
			TS_LOG_ERR("storage8_diag_func: Sorting Mode no support command=%d\n",
				g_diag_command);
		}
		himax_nc_diag_register_set(command[DATA_0]);
	}
	dsram_flag = true;
}

static void himax_diag_other_func(void)
{
	uint8_t command[DATA_2];
	struct ts_kit_device_data *ts;

	ts = g_himax_nc_ts_data->tskit_himax_data;
	// set diag flag
	if (dsram_flag) {
		TS_LOG_INFO("return and cancel sram thread!\n");
		// 1. Clear DSRAM flag
		dsram_flag = false;
		// 2. Enable ISR
		himax_nc_int_enable(ts->ts_platform_data->irq_id, 1);
		// 3. FW leave sram and return to event stack
		himax_nc_return_event_stack();
	}
	if (g_switch_mode == SORTING_MODE) {
		himax_nc_idle_mode(SET_OFF);
		g_switch_mode = himax_nc_switch_mode(SET_OFF);
	}
	if (g_diag_command != RAWDATA_DIAG_CMD_NONE) {
		TS_LOG_ERR("diag_other_func: diag_command=0x%x error so reset\n",
			g_diag_command);
		command[DATA_0] = RAWDATA_TPYE_NONE;
		if (g_diag_command != RAWDATA_DIAG_CMD_BASELINE)
			g_diag_command = RAWDATA_DIAG_CMD_NONE;
		himax_nc_diag_register_set(command[DATA_0]);
	} else {
		command[DATA_0] = RAWDATA_TPYE_NONE;
		g_diag_command = RAWDATA_DIAG_CMD_NONE;
		himax_nc_diag_register_set(command[DATA_0]);
		TS_LOG_INFO("diag_other_func: g_diag_command=0x%x\n",
			g_diag_command);
	}
}

static ssize_t himax_diag_seq_write(struct file *filp,
			const char __user *buff, size_t len, loff_t *data)
{
	char buf[DIAG_COMMAND_MAX_SIZE] = {0};
	int storage_type; // 0: common , other: dsram
	int rawdata_type; // 1:IIR,2:DC
	struct himax_ts_data *ts;

	ts = g_himax_nc_ts_data;
	if (len >= DIAG_COMMAND_MAX_SIZE) {
		TS_LOG_INFO("diag_seq_write: no command exceeds 80 chars\n");
		return -EFAULT;
	}
	TS_LOG_INFO("diag_seq_write: g_switch_mode = %d\n", g_switch_mode);
	if (copy_from_user(buf, buff, len))
		return -EFAULT;

	if (buf[DATA_1] == USER_SEPARATION_CHAR)
		g_diag_command = buf[DATA_0] - '0';
	else
		g_diag_command = (buf[DATA_0] - '0') *
			BUFF_NUM_TEN + (buf[DATA_1] - '0');
	storage_type = himax_determin_diag_storage(g_diag_command);
	rawdata_type = himax_determin_diag_rawdata(g_diag_command);
	if ((g_diag_command > RAWDATA_DIAG_CMD_NONE) &&
		(rawdata_type == USER_RAWDATA_CMD_MIN)) {
		TS_LOG_INFO("[Hx]g_diag_command=0x%x,\n", g_diag_command);
		TS_LOG_INFO("[Hx]storage_type=%d, rawdata_type=%d\n",
			storage_type, rawdata_type);
		g_diag_command = RAWDATA_DIAG_CMD_NONE;
	} else {
		TS_LOG_INFO("[Hx]g_diag_command=0x%x,\n", g_diag_command);
		TS_LOG_INFO("[Hx]storage_type=%d, rawdata_type=%d\n",
			storage_type, rawdata_type);
	}
	if ((storage_type == USER_STORAGE_CMD_MIN) &&
		(rawdata_type > USER_RAWDATA_CMD_MIN) &&
		(rawdata_type < USER_RAWDATA_CMD_MAX)) {
		himax_storage0_diag0_8_func();
	} else if ((storage_type > USER_STORAGE_CMD_MIN) &&
		(storage_type < USER_STORAGE_CMD_MAX) &&
		(rawdata_type > USER_RAWDATA_CMD_MIN) &&
		(rawdata_type < USER_RAWDATA_CMD_MAX)) {
		himax_storage0_8_diag0_8_func(rawdata_type);
	} else if (storage_type == USER_STORAGE_CMD_MAX) {
		himax_storage8_diag_func(rawdata_type);
	} else {
		himax_diag_other_func();
	}
	return len;
}

static void diag_seq_com1_3_func(struct seq_file *s, uint16_t mutual_num)
{
	uint32_t loop_i;
	uint16_t flg;
	uint16_t width;

	width = x_channel;
	for (loop_i = DATA_0; loop_i < mutual_num; loop_i++) {
		seq_printf(s, "%6d", diag_mutual[loop_i]);
		if ((loop_i % width) == (width - DATA_1)) {
			flg = width + loop_i / width;
			seq_printf(s, " %6d\n", diag_self[flg]);
		}
	}
	seq_puts(s, "\n");
	for (loop_i = DATA_0; loop_i < width; loop_i++) {
		seq_printf(s, "%6d", diag_self[loop_i]);
		if (((loop_i) % width) == (width - DATA_1))
			seq_puts(s, "\n");
	}
#ifdef HX_EN_SEL_BUTTON
	seq_puts(s, "\n");
	for (loop_i = DATA_0; loop_i < hx_nc_bt_num; loop_i++)
		seq_printf(s, "%6d",
			diag_self[hx_nc_rx_num + hx_nc_tx_num + loop_i]);
#endif
	seq_puts(s, "ChannelEnd");
	seq_puts(s, "\n");
}

static void diag_seq_com8_func(struct seq_file *s)
{
	uint32_t loop_i;

	for (loop_i = DATA_0; loop_i < SEQ_RETRY_TIMES; loop_i++) {
		if ((loop_i % DIAG_NUM_16) == DATA_0)
			seq_puts(s, "LineStart:");
		seq_printf(s, "%4x", hx_nc_diag_coor[loop_i]);
		if ((loop_i % DIAG_NUM_16) == DIAG_NUM_15)
			seq_puts(s, "\n");
	}
}

static void diag_seq_dsram0_8_func(struct seq_file *s, uint8_t *info_data,
	int dsram_type, uint16_t mutual_num)
{
	uint32_t loop_i;
	uint32_t loop_j;
	unsigned int index;
	int16_t new_data;
	uint16_t flg;
	uint16_t width;

	width = x_channel;
	himax_burst_enable(SET_ON);
	himax_nc_get_dsram_data(info_data, mutual_num * DATA_2);
	for (loop_i = DATA_0, index = DATA_0;
		loop_i < hx_nc_tx_num; loop_i++) {
		for (loop_j = DATA_0; loop_j < hx_nc_rx_num; loop_j++) {
			new_data = (short)(info_data[index + DATA_1] <<
			LEFT_MOV_8BIT | info_data[index]);
			if (dsram_type == DATA_1)
				diag_mutual[loop_i * hx_nc_rx_num + loop_j]
					= new_data;
			index += DATA_2;
		}
	}
	for (loop_i = DATA_0; loop_i < mutual_num; loop_i++) {
		seq_printf(s, "%6d", diag_mutual[loop_i]);
		if ((loop_i % width) == (width - DATA_1)) {
			flg = width + loop_i / width;
			seq_printf(s, " %6d\n", diag_self[flg]);
		}
	}
	seq_puts(s, "\n");
	for (loop_i = DATA_0; loop_i < width; loop_i++) {
		seq_printf(s, "%6d", diag_self[loop_i]);
		if (((loop_i) % width) == (width - DATA_1))
			seq_puts(s, "\n");
	}
	seq_puts(s, "ChannelEnd");
	seq_puts(s, "\n");
}

static int himax_diag_seq_show(struct seq_file *s, void *v)
{
	uint16_t self_num;
	uint16_t mutual_num;
	int dsram_type;
	uint8_t *info_data = NULL;

	dsram_type = g_diag_command / BUFF_NUM_TEN;
	// check if divided by zero
	if (x_channel == DATA_0) {
		TS_LOG_ERR("diag_seq_show: divided by zero\n");
		return 0;
	}
	mutual_num = x_channel * y_channel;
	self_num = x_channel + y_channel;
	info_data = kcalloc(mutual_num * DATA_2, sizeof(uint8_t), GFP_KERNEL);
	seq_printf(s, "ChannelStart: %4d, %4d\n\n", x_channel, y_channel);
	if (((g_diag_command >= USER_DIAG_CMD1) &&
		(g_diag_command <= USER_DIAG_CMD3)) ||
		(g_diag_command == USER_DIAG_CMD7)) {
		TS_LOG_INFO("EVENT STACK\n");
		diag_seq_com1_3_func(s, mutual_num);
	} else if (g_diag_command == USER_DIAG_CMD8) {
		diag_seq_com8_func(s);
	} else if ((dsram_type > DSRAM_TYPE0) &&
		(dsram_type <= DSRAM_TYPE8)) {
		TS_LOG_INFO("DSRAM\n");
		diag_seq_dsram0_8_func(s, info_data, dsram_type, mutual_num);
	}
	kfree(info_data);
	return 0;
}

static void *himax_diag_seq_start(struct seq_file *s, loff_t *pos)
{
	uintptr_t p = *pos + DATA_1;

	if (*pos >= DATA_1)
		return NULL;

	return (void *)p;
}

static void *himax_diag_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	return NULL;
}

static void himax_diag_seq_stop(struct seq_file *s, void *v)
{
	kfree(v);
}

static const struct seq_operations himax_diag_seq_ops = {
	.start = himax_diag_seq_start,
	.next = himax_diag_seq_next,
	.stop = himax_diag_seq_stop,
	.show = himax_diag_seq_show,
};

static int himax_diag_seq_open(struct inode *inode, struct file *f)
{
	return seq_open(f, &himax_diag_seq_ops);
}

static const struct file_operations hx_diag_seq_fop = {
	.owner = THIS_MODULE,
	.open = himax_diag_seq_open,
	.write = himax_diag_seq_write,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

static ssize_t himax_attr_show(struct kobject *kobj,
	struct attribute *attr, char *buf)
{
	struct device_attribute *dev_attribute = NULL;
	struct device *dev = NULL;

	dev_attribute = container_of(attr, struct device_attribute, attr);
	if (!dev_attribute->show)
		return -EIO;
	return dev_attribute->show(dev, dev_attribute, buf);
}

static ssize_t himax_attr_store(struct kobject *kobj,
	struct attribute *attr, const char *buf, size_t len)
{
	struct device_attribute *dev_attribute = NULL;
	struct device *dev = NULL;

	dev_attribute = container_of(attr, struct device_attribute, attr);
	if (!dev_attribute->store)
		return -EIO;
	return dev_attribute->store(dev, dev_attribute, buf, len);
}

static const struct sysfs_ops himax_sys_ops = {
	.show = himax_attr_show,
	.store = himax_attr_store,
};

static struct kobj_type himax_type = {
	.sysfs_ops = &himax_sys_ops,
};

int himax_nc_touch_sysfs_init(void)
{
	int ret = 0;

	himax_debugfs_dir = debugfs_create_dir("himax_debug_seq", NULL);
	if (himax_debugfs_dir == NULL) {
		TS_LOG_ERR("touch_sysfs_init: subsystem_register failed\n");
		ret = -ENOMEM;
		return ret;
	}

	diag_seq_dentry = debugfs_create_file("diag", 0644,
		himax_debugfs_dir, NULL, &hx_diag_seq_fop);
	if (diag_seq_dentry == NULL) {
		TS_LOG_ERR("touch_sysfs_init: debugfs for diag is fail\n");
		ret = -ENOMEM;
		return ret;
	}

	android_touch_kobj = kzalloc(sizeof(*android_touch_kobj), GFP_KERNEL);
	if (!android_touch_kobj) {
		TS_LOG_ERR("touch_sysfs_init: alloc android_touch_kobj failed\n");
		ret = -ENOMEM;
		return ret;
	}

	ret = kobject_init_and_add(android_touch_kobj, &himax_type, NULL, "himax_debug");
	if (ret) {
		TS_LOG_ERR("touch_sysfs_init: subsystem_register failed\n");
		ret = -ENOMEM;
		return ret;
	}

	ret = sysfs_create_file(android_touch_kobj, &dev_attr_debug_level.attr);
	if (ret) {
		TS_LOG_ERR("touch_sysfs_init: create_file dev_attr_debug_level failed\n");
		return ret;
	}

	ret = sysfs_create_file(android_touch_kobj, &dev_attr_vendor.attr);
	if (ret) {
		TS_LOG_ERR("touch_sysfs_init: sysfs_create_file dev_attr_vendor failed\n");
		return ret;
	}

	ret = sysfs_create_file(android_touch_kobj, &dev_attr_reset.attr);
	if (ret) {
		TS_LOG_ERR("touch_sysfs_init: sysfs_create_file dev_attr_reset failed\n");
		return ret;
	}

	ret = sysfs_create_file(android_touch_kobj, &dev_attr_attn.attr);
	if (ret) {
		TS_LOG_ERR("touch_sysfs_init: sysfs_create_file dev_attr_attn failed\n");
		return ret;
	}

	ret = sysfs_create_file(android_touch_kobj, &dev_attr_int_en.attr);
	if (ret) {
		TS_LOG_ERR("touch_sysfs_init: sysfs_create_file dev_attr_enabled failed\n");
		return ret;
	}

#ifdef HX_TP_SYS_REGISTER
	ret = sysfs_create_file(android_touch_kobj, &dev_attr_register.attr);
	if (ret) {
		TS_LOG_ERR("touch_sysfs_init: create_file dev_attr_register failed\n");
		return ret;
	}
#endif

#ifdef HX_TP_SYS_DIAG
	ret = sysfs_create_file(android_touch_kobj, &dev_attr_diag.attr);
	if (ret) {
		TS_LOG_ERR("touch_sysfs_init: sysfs_create_file dev_attr_diag failed\n");
		return ret;
	}
#endif

#ifdef HX_TP_SYS_DEBUG
	ret = sysfs_create_file(android_touch_kobj, &dev_attr_debug.attr);
	if (ret) {
		TS_LOG_ERR("touch_sysfs_init: create_file dev_attr_debug failed\n");
		return ret;
	}
#endif

#ifdef HX_TP_SYS_FLASH_DUMP
	ret = sysfs_create_file(android_touch_kobj, &dev_attr_flash_dump.attr);
	if (ret) {
		TS_LOG_ERR("touch_sysfs_init: sysfs_create_file dev_attr_flash_dump failed\n");
		return ret;
	}
#endif

	ret = sysfs_create_file(android_touch_kobj, &dev_attr_sns_onoff.attr);
	if (ret) {
		TS_LOG_ERR("touch_sysfs_init: create_file sns_onoff failed\n");
		return ret;
	}

	return NO_ERR;
}

void himax_nc_touch_sysfs_deinit(void)
{
	sysfs_remove_file(android_touch_kobj, &dev_attr_debug_level.attr);
	sysfs_remove_file(android_touch_kobj, &dev_attr_vendor.attr);
	sysfs_remove_file(android_touch_kobj, &dev_attr_reset.attr);
	sysfs_remove_file(android_touch_kobj, &dev_attr_attn.attr);
	sysfs_remove_file(android_touch_kobj, &dev_attr_int_en.attr);

#ifdef HX_TP_SYS_REGISTER
	sysfs_remove_file(android_touch_kobj, &dev_attr_register.attr);
#endif

#ifdef HX_TP_SYS_DIAG
	sysfs_remove_file(android_touch_kobj, &dev_attr_diag.attr);
#endif

#ifdef HX_TP_SYS_DEBUG
	sysfs_remove_file(android_touch_kobj, &dev_attr_debug.attr);
#endif

#ifdef HX_TP_SYS_FLASH_DUMP
	sysfs_remove_file(android_touch_kobj, &dev_attr_flash_dump.attr);
#endif

	sysfs_remove_file(android_touch_kobj, &dev_attr_sns_onoff.attr);
	debugfs_remove(diag_seq_dentry);
	debugfs_remove(himax_debugfs_dir);
	kobject_del(android_touch_kobj);
}
