/* Himax Android Driver Sample Code for Himax chipset
 *
 * Copyright (C) 2021 Himax Corporation.
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

bool zf_dsram_flag;
int zf_self_test_flag;
static struct platform_device *hx_dev;

#define MOV_3BIT 3
#define COLUMNS_LEN 16
#define REG_COMMON_LEN 128
#define DIAG_COMMAND_MAX_SIZE 80
#define DIAG_INT_COMMAND_MAX_SIZE 12

#if defined(CONFIG_TOUCHSCREEN_HIMAX_ZF_DEBUG)
#ifdef HX_TP_SYS_DIAG
	static uint8_t x_channel;
	static uint8_t y_channel;
	static uint8_t diag_max_cnt;
	static int g_switch_mode;
	static int16_t *diag_self;
	static int16_t *diag_mutual;
	static int16_t *diag_mutual_new;
	static int16_t *diag_mutual_old;
	uint8_t g_diag_arr_num;
	int g_max_mutual;
	int g_min_mutual = 0xFFFF;
	int g_max_self;
	int g_min_self = 0xFFFF;
	int g_zf_diag_command = 0;

	#define IIR_DUMP_FILE "/sdcard/HX_IIR_Dump.txt"
	#define DC_DUMP_FILE "/sdcard/HX_DC_Dump.txt"
	#define BANK_DUMP_FILE "/sdcard/HX_BANK_Dump.txt"
#endif

#ifdef HX_TP_SYS_REGISTER
	static uint8_t register_command[4] = {0};
	uint8_t cfg_flag;
#endif

#define HIMAX_UPDATE_FW 	"Himax_firmware.bin"
char *himax_update_fw;

#ifdef HX_TP_SYS_DEBUG
	uint8_t zf_cmd_set[4] = {0};
	uint8_t zf_mutual_set_flag = 0;
	static bool fw_update_complete;
	static unsigned char debug_level_cmd;
	static unsigned char upgrade_fw[FW_SIZE_64k] = {0};
#endif

#endif
extern char himax_firmware_name[64];
extern char himax_zf_project_id[];
extern int hx_zf_irq_enable_count;
extern struct himax_ts_data *g_himax_zf_ts_data;

extern int himax_zf_bus_read(uint8_t command, uint8_t *data, uint16_t length, uint16_t limit_len, uint8_t toRetry);
extern void himax_zf_int_enable(int irqnum, int enable);
extern void himax_zf_rst_gpio_set(int pinnum, uint8_t value);
extern void hx_reload_to_active(void);

#ifdef HX_TP_SYS_RESET
static ssize_t hx_reset_set(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	if (buf[0] == '1') {
		himax_zf_rst_gpio_set(g_himax_zf_ts_data->rst_gpio, 0);
		usleep_range(RST_LOW_PERIOD_S, RST_LOW_PERIOD_E);
		himax_zf_rst_gpio_set(g_himax_zf_ts_data->rst_gpio, 1);
		usleep_range(RST_HIGH_PERIOD_S, RST_HIGH_PERIOD_E);
		hx_reload_to_active();
		TS_LOG_INFO("pin reset \n");
	}

	return count;
}
static DEVICE_ATTR(reset, 0600, NULL, hx_reset_set);
#endif

#ifdef HX_TP_SYS_DIAG

int16_t *hx_zf_getMutualBuffer(void)
{
	return diag_mutual;
}

int16_t *hx_zf_getMutualNewBuffer(void)
{
	return diag_mutual_new;
}

int16_t *hx_zf_getMutualOldBuffer(void)
{
	return diag_mutual_old;
}

int16_t *hx_zf_getSelfBuffer(void)
{
	return diag_self;
}

int16_t hx_zf_getXChannel(void)
{
	return x_channel;
}

int16_t hx_zf_getYChannel(void)
{
	return y_channel;
}

int16_t hx_zf_getDiagCommand(void)
{
	return g_zf_diag_command;
}

void hx_zf_setXChannel(uint8_t x)
{
	x_channel = x;
}

void hx_zf_setYChannel(uint8_t y)
{
	y_channel = y;
}

void hx_zf_setMutualBuffer(void)
{
	diag_mutual = kzalloc(x_channel * y_channel * sizeof(int16_t), GFP_KERNEL);
	if (diag_mutual == NULL)
		TS_LOG_ERR("%s: kzalloc error.\n", __func__);
}

void hx_zf_setMutualNewBuffer(void)
{
	diag_mutual_new = kzalloc(x_channel * y_channel * sizeof(int16_t), GFP_KERNEL);
	if (diag_mutual_new == NULL)
		TS_LOG_ERR("%s: kzalloc error.\n", __func__);
}

void hx_zf_setMutualOldBuffer(void)
{
	diag_mutual_old = kzalloc(x_channel * y_channel * sizeof(int16_t), GFP_KERNEL);
	if (diag_mutual_old == NULL)
		TS_LOG_ERR("%s: kzalloc error.\n", __func__);
}

void hx_zf_setSelfBuffer(void)
{
	diag_self = kzalloc((x_channel + y_channel) * sizeof(int16_t), GFP_KERNEL);
	if (diag_self == NULL)
		TS_LOG_ERR("%s: kzalloc error.\n", __func__);
}

void hx_zf_freeMutualBuffer(void)
{
	if (diag_mutual)
		kfree(diag_mutual);
	diag_mutual = NULL;
}

void hx_zf_freeMutualNewBuffer(void)
{
	if (diag_mutual_new)
		kfree(diag_mutual_new);
	diag_mutual_new = NULL;
}

void hx_zf_freeMutualOldBuffer(void)
{
	if (diag_mutual_old)
		kfree(diag_mutual_old);
	diag_mutual_old = NULL;
}

void hx_zf_freeSelfBuffer(void)
{
	if (diag_self)
		kfree(diag_self);
	diag_self = NULL;
}

static int himax_determin_diag_rawdata(int diag_command)
{
	return diag_command%10;
}

static int himax_determin_diag_storage(int diag_command)
{
	return diag_command/10;
}

static int hx_diag_stack_test_arrange_print(int i, int j, int transpose, char *buf, int count)
{
	TS_LOG_INFO("%s, transpose = %d\n", __func__, transpose);
	if (transpose) {
		count += snprintf(buf + count, PAGE_SIZE - count, "%6d", diag_mutual[j + i * HX_ZF_RX_NUM]);
		TS_LOG_INFO("diag_mutual[%d] = %6d", j + i * HX_ZF_RX_NUM, diag_mutual[j + i * HX_ZF_RX_NUM]);
	} else {
		count += snprintf(buf + count, PAGE_SIZE - count, "%6d", diag_mutual[i + j * HX_ZF_RX_NUM]);
		TS_LOG_INFO("diag_mutual[%d] = %6d", i + j * HX_ZF_RX_NUM, diag_mutual[j + i * HX_ZF_RX_NUM]);
	}
	return count;
}

static int hx_diag_stack_test_arrange_inloop(int in_init, int out_init, bool transpose, int j, char *buf, int count)
{
	int x_channel = HX_ZF_RX_NUM;
	int y_channel = HX_ZF_TX_NUM;
	int i;
	int in_max;

	if (transpose)
		in_max = y_channel;
	else
		in_max = x_channel;

	TS_LOG_INFO("%s, in_max = %d, out_init = %d, in_init = %d, transpose = %d\n", __func__, in_max, out_init, in_init, transpose);
	TS_LOG_INFO("%s, x_channel = %d, y_channel = %d\n", __func__, x_channel, y_channel);

	if (in_init > 0) { /* bit0 = 1 */
		for (i = in_init - 1; i >= 0; i--)
			count = hx_diag_stack_test_arrange_print(i, j, transpose, buf, count);
		if (transpose) {
			if (out_init > 0) {
				count += snprintf(buf + count, PAGE_SIZE - count, " %5d\n", diag_self[j]);
				TS_LOG_INFO("%s77, diag_self[%d] = %6d", __func__, j, diag_self[j]);
			} else {
				count += snprintf(buf + count, PAGE_SIZE - count, " %5d\n", diag_self[x_channel - j - 1]);
				TS_LOG_INFO("%s88, diag_self[%d] = %6d", __func__, x_channel - j - 1, diag_self[x_channel - j - 1]);
			}
		}
	} else { /* bit0 = 0 */
		for (i = 0; i < in_max; i++)
			count = hx_diag_stack_test_arrange_print(i, j, transpose, buf, count);
		if (transpose) {
			if (out_init > 0) {
				count += snprintf(buf + count, PAGE_SIZE - count, " %5d\n", diag_self[x_channel - j - 1]);
				TS_LOG_INFO("%s99, diag_self[%d] = %6d", __func__, x_channel - j - 1, diag_self[x_channel - j - 1]);
			} else {
				count += snprintf(buf + count, PAGE_SIZE - count, " %5d\n", diag_self[j]);
				TS_LOG_INFO("%saa, diag_self[%d] = %6d", __func__, j, diag_self[j]);
			}
		}
	}

	return count;
}

static int hx_diag_stack_test_arrange_outloop(int transpose, int out_init, int in_init, char *buf, int count)
{
	int j;
	int x_channel = HX_ZF_RX_NUM;
	int y_channel = HX_ZF_TX_NUM;
	int out_max;
	int self_cnt = 0;

	if (transpose)
		out_max = x_channel;
	else
		out_max = y_channel;

	TS_LOG_INFO("%s, out_init = %d, in_init = %d, transpose = %d, x_channel = %d, y_channel = %d, out_max = %d\n",
		__func__, out_init, in_init, transpose, x_channel, y_channel, out_max);

	if (out_init > 0) { /* bit1 = 1 */
		self_cnt = 1;
		for (j = out_init - 1; j >= 0; j--) {
			count = hx_diag_stack_test_arrange_inloop(in_init, out_init, transpose, j, buf, count);
			if (!transpose) {
				count += snprintf(buf + count, PAGE_SIZE - count, " %5d\n", diag_self[y_channel + x_channel - self_cnt]);
				TS_LOG_INFO("%s55, diag_self[%d] = %6d", __func__, y_channel + x_channel - self_cnt,
					diag_self[y_channel + x_channel - self_cnt]);
				self_cnt++;
			}
		}
	} else { /* bit1 = 0 */
		for (j = 0; j < out_max; j++) {
			count = hx_diag_stack_test_arrange_inloop(in_init, out_init, transpose, j,  buf, count);
			if (!transpose) {
				count += snprintf(buf + count, PAGE_SIZE - count, " %5d\n", diag_self[j + x_channel]);
				TS_LOG_INFO("%s66, diag_self[%d] = %6d", __func__, j + x_channel, diag_self[j + x_channel]);
			}
		}
	}

	return count;
}

static int hx_diag_stack_test_arrange(char *buf, int count)
{
	int x_channel = HX_ZF_RX_NUM;
	int y_channel = HX_ZF_TX_NUM;
	int bit0;
	int bit1;
	int bit2;
	int i;
	/* rotate bit */
	bit2 = g_diag_arr_num >> 2;
	/* reverse Y */
	bit1 = (g_diag_arr_num >> 1) & 0x1;
	/* reverse X */
	bit0 = g_diag_arr_num & 0x1;
	TS_LOG_INFO("%s, bit0 = %d, bit1 = %d, bit2 = %d\n", __func__, bit0, bit1, bit2);
	TS_LOG_INFO("%s, x_channel = %d, y_channel = %d\n", __func__, x_channel, y_channel);

	if (g_diag_arr_num < 4) {
		count += snprintf(buf + count, PAGE_SIZE - count, "\n");
		count = hx_diag_stack_test_arrange_outloop(bit2, bit1 * y_channel, bit0 * x_channel, buf, count);
		if (bit0 == 1) {
			for (i = x_channel - 1; i >= 0; i--) {
				count += snprintf(buf + count, PAGE_SIZE - count, "%6d", diag_self[i]);
				TS_LOG_INFO("%s11, diag_self[%d] = %6d", __func__, i, diag_self[i]);
			}
		} else {
			for (i = 0; i < x_channel; i++) {
				count += snprintf(buf + count, PAGE_SIZE - count, "%6d", diag_self[i]);
				TS_LOG_INFO("%s22, diag_self[%d] = %6d", __func__, i, diag_self[i]);
			}
		}
	} else {
		count += snprintf(buf + count, PAGE_SIZE - count, "\n");
		count = hx_diag_stack_test_arrange_outloop(bit2, bit1 * x_channel, bit0 * y_channel, buf, count);
		if (bit1 == 1) {
			for (i = x_channel + y_channel - 1; i >= x_channel; i--) {
				count += snprintf(buf + count, PAGE_SIZE - count, "%6d", diag_self[i]);
				TS_LOG_INFO("%s33, diag_self[%d] = %6d", __func__, i, diag_self[i]);
			}
		} else {
			for (i = x_channel; i < x_channel + y_channel; i++) {
				count += snprintf(buf + count, PAGE_SIZE - count, "%6d", diag_self[i]);
				TS_LOG_INFO("%s44, diag_self[%d] = %6d", __func__, i, diag_self[i]);
			}
		}
	}

	return count;
}

void himax_get_mutual_edge(void)
{
	int i;
	for (i = 0; i < (HX_ZF_RX_NUM * HX_ZF_TX_NUM); i++) {
		if (diag_mutual[i] > g_max_mutual)
			g_max_mutual = diag_mutual[i];

		if (diag_mutual[i] < g_min_mutual)
			g_min_mutual = diag_mutual[i];
	}
}

void himax_get_self_edge(void)
{
	int i;
	for (i = 0; i < (HX_ZF_RX_NUM + HX_ZF_TX_NUM); i++) {
		if (diag_self[i] > g_max_self)
			g_max_self = diag_self[i];

		if (diag_self[i] < g_min_self)
			g_min_self = diag_self[i];
	}
}

static int print_stack_test_state_info(char *buf, int count)
{
#if defined(HX_NEW_EVENT_STACK_FORMAT)
	count += snprintf(buf + count, PAGE_SIZE - count, "ReCal = %d\t", g_himax_zf_ts_data->hx_state_info[0] & 0x03);
	count += snprintf(buf + count, PAGE_SIZE - count, "Base Line = %d\t", g_himax_zf_ts_data->hx_state_info[0] >> 2 & 0x01);
	count += snprintf(buf + count, PAGE_SIZE - count, "Palm = %d\t", g_himax_zf_ts_data->hx_state_info[0] >> 3 & 0x01);
	count += snprintf(buf + count, PAGE_SIZE - count, "Idle mode = %d\t", g_himax_zf_ts_data->hx_state_info[0] >> 4 & 0x01);
	count += snprintf(buf + count, PAGE_SIZE - count, "Water = %d\n", g_himax_zf_ts_data->hx_state_info[0] >> 5 & 0x01);
	count += snprintf(buf + count, PAGE_SIZE - count, "TX Hop = %d\t", g_himax_zf_ts_data->hx_state_info[0] >> 6 & 0x01);
	count += snprintf(buf + count, PAGE_SIZE - count, "AC mode = %d\t", g_himax_zf_ts_data->hx_state_info[0] >> 7 & 0x01);
	count += snprintf(buf + count, PAGE_SIZE - count, "Glove = %d\t", g_himax_zf_ts_data->hx_state_info[1] & 0x01);
	count += snprintf(buf + count, PAGE_SIZE - count, "Stylus = %d\t", g_himax_zf_ts_data->hx_state_info[1] >> 1 & 0x01);
	count += snprintf(buf + count, PAGE_SIZE - count, "Hovering = %d\t", g_himax_zf_ts_data->hx_state_info[1] >> 2 & 0x01);
	count += snprintf(buf + count, PAGE_SIZE - count, "Proximity = %d\t", g_himax_zf_ts_data->hx_state_info[1] >> 3 & 0x01);
	count += snprintf(buf + count, PAGE_SIZE - count, "KEY = %d\n", g_himax_zf_ts_data->hx_state_info[1] >> 4 & 0x0F);
#else
	count += snprintf(buf + count, PAGE_SIZE - count, "ReCal = %d\t", g_himax_zf_ts_data->hx_state_info[0] & 0x01);
	count += snprintf(buf + count, PAGE_SIZE - count, "Palm = %d\t", g_himax_zf_ts_data->hx_state_info[0] >> 1 & 0x01);
	count += snprintf(buf + count, PAGE_SIZE - count, "AC mode = %d\t", g_himax_zf_ts_data->hx_state_info[0] >> 2 & 0x01);
	count += snprintf(buf + count, PAGE_SIZE - count, "Water = %d\n", g_himax_zf_ts_data->hx_state_info[0] >> 3 & 0x01);
	count += snprintf(buf + count, PAGE_SIZE - count, "Glove = %d\t", g_himax_zf_ts_data->hx_state_info[0] >> 4 & 0x01);
	count += snprintf(buf + count, PAGE_SIZE - count, "TX Hop = %d\t", g_himax_zf_ts_data->hx_state_info[0] >> 5 & 0x01);
	count += snprintf(buf + count, PAGE_SIZE - count, "Base Line = %d\t", g_himax_zf_ts_data->hx_state_info[0] >> 6 & 0x01);
	count += snprintf(buf + count, PAGE_SIZE - count, "OSR Hop = %d\t", g_himax_zf_ts_data->hx_state_info[1] >> 3 & 0x01);
	count += snprintf(buf + count, PAGE_SIZE - count, "KEY = %d\n", g_himax_zf_ts_data->hx_state_info[1] >> 4 & 0x0F);
#endif

	return count;
}

static int hx_diag_stack_test_print(char *buf, int count)
{
	int x_num = HX_ZF_RX_NUM;
	int y_num = HX_ZF_TX_NUM;

	TS_LOG_INFO("%s, x_num = %d, y_num = %d\n", __func__, x_num, y_num);

	count += snprintf(buf + count, PAGE_SIZE - count, "ChannelStart: %4d, %4d\n\n", x_num, y_num);
	/* start to show out the raw data in adb shell */
	count = hx_diag_stack_test_arrange(buf, count);
	count += snprintf(buf + count, PAGE_SIZE - count, "\n");
	count += snprintf(buf + count, PAGE_SIZE - count, "ChannelEnd");
	count += snprintf(buf + count, PAGE_SIZE - count, "\n");
	/* print Mutual/Slef Maximum and Minimum */
	himax_get_mutual_edge();
	himax_get_self_edge();
	count += snprintf(buf + count, PAGE_SIZE - count, "Mutual Max:%3d, Min:%3d\n", g_max_mutual, g_min_mutual);
	count += snprintf(buf + count, PAGE_SIZE - count, "Self Max:%3d, Min:%3d\n", g_max_self, g_min_self);
	/* recovery status after print */
	g_max_mutual = 0;
	g_min_mutual = 0xFFFF;
	g_max_self = 0;
	g_min_self = 0xFFFF;
	/* pring state info */
	count = print_stack_test_state_info(buf, count);

	return count;
}

static ssize_t himax_diag_arr_show(struct device *dev, struct device_attribute *attr, const char *buf)
{
	size_t retval = 0;
	retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "diag value=%d\n", g_diag_arr_num);

	return retval;
}

static ssize_t hx_diag_arr_dump(struct device *dev, struct device_attribute *attr, const char *buff, size_t len)
{
	if (len >= 80) {
		TS_LOG_ERR("%s: no command exceeds 80 chars.\n", __func__);
		return -EFAULT;
	}
	g_diag_arr_num = buff[0] - '0';
	TS_LOG_INFO("%s: g_diag_arr_num = %d\n", __func__, g_diag_arr_num);
	return len;
}

static DEVICE_ATTR(diag_arr, 0600, himax_diag_arr_show, hx_diag_arr_dump); // Debug_diag_done

static int hx_diag_arrange_print(int i, int j, int transpose, char *buf, int count)
{
	int x_channel = HX_ZF_RX_NUM;
	int y_channel = HX_ZF_TX_NUM;
	if (transpose)
		count += snprintf(buf + count, HX_MAX_PRBUF_SIZE - count, "%6d", diag_mutual[j + i * x_channel]);
	else
		count += snprintf(buf + count, HX_MAX_PRBUF_SIZE - count, "%6d", diag_mutual[i + j * y_channel]);

	return count;
}

static int hx_diag_arrange_inloop(int in_init, int out_init, bool transpose, int j, char *buf, int count)
{
	int x_channel = HX_ZF_RX_NUM;
	int y_channel = HX_ZF_TX_NUM;
	int i;
	int in_max;

	if (transpose)
		in_max = y_channel;
	else
		in_max = x_channel;

	if (in_init > 0) { /* bit0 = 1 */
		for (i = in_init - 1; i >= 0; i--)
			count = hx_diag_arrange_print(i, j, transpose, buf, count);
		if (transpose) {
			if (out_init > 0)
				count += snprintf(buf + count, HX_MAX_PRBUF_SIZE - count, " %5d\n", diag_self[j]);
			else
				count += snprintf(buf + count, HX_MAX_PRBUF_SIZE - count, " %5d\n", diag_self[x_channel - j - 1]);
		}
	} else { /* bit0 = 0 */
		for (i = 0; i < in_max; i++)
			count = hx_diag_arrange_print(i, j, transpose, buf, count);
		if (transpose) {
			if (out_init > 0)
				count += snprintf(buf + count, HX_MAX_PRBUF_SIZE - count, " %5d\n", diag_self[x_channel - j - 1]);
			else
				count += snprintf(buf + count, HX_MAX_PRBUF_SIZE - count, " %5d\n", diag_self[j]);
		}
	}

	return count;
}

static int hx_diag_arrange_outloop(int transpose, int out_init, int in_init, char *buf, int count)
{
	int j;
	int x_channel = HX_ZF_RX_NUM;
	int y_channel = HX_ZF_TX_NUM;
	int out_max;
	int self_cnt = 0;

	if (transpose)
		out_max = x_channel;
	else
		out_max = y_channel;

	if (out_init > 0) { /* bit1 = 1 */
		self_cnt = 1;
		for (j = out_init - 1; j >= 0; j--) {
			count = hx_diag_arrange_inloop(in_init, out_init, transpose, j, buf, count);
			if (!transpose) {
				count += snprintf(buf + count, HX_MAX_PRBUF_SIZE - count, " %5d\n",
					diag_self[y_channel + x_channel - self_cnt]);
				self_cnt++;
			}
		}
	} else { /* bit1 = 0 */
		for (j = 0; j < out_max; j++) {
			count = hx_diag_arrange_inloop(in_init, out_init, transpose, j, buf, count);
			if (!transpose) {
				count += snprintf(buf + count, HX_MAX_PRBUF_SIZE - count, " %5d\n",
					diag_self[j + x_channel]);
			}
		}
	}

	return count;
}

static int hx_diag_arrange(char *buf, int count)
{
	int bit0;
	int bit1;
	int bit2;
	int i;
	/* rotate bit */
	bit2 = g_diag_arr_num >> 2;
	/* reverse Y */
	bit1 = (g_diag_arr_num >> 1) & 0x1;
	/* reverse X */
	bit0 = g_diag_arr_num & 0x1;
	TS_LOG_INFO("__func__ is %s\n", __func__);

	if (g_diag_arr_num < 4) {
		count += snprintf(buf + count, HX_MAX_PRBUF_SIZE - count, "\n");
		count = hx_diag_arrange_outloop(bit2, bit1 * y_channel, bit0 * x_channel, buf, count);
		count += snprintf(buf + count, HX_MAX_PRBUF_SIZE - count, "%6c", ' ');
		if (bit0 == 1) {
			for (i = x_channel - 1; i >= 0; i--)
				count += snprintf(buf + count, HX_MAX_PRBUF_SIZE - count, "%6d", diag_self[i]);
		} else {
			for (i = 0; i < x_channel; i++)
				count += snprintf(buf + count, HX_MAX_PRBUF_SIZE - count, "%6d", diag_self[i]);
		}
	} else {
		count += snprintf(buf + count, HX_MAX_PRBUF_SIZE - count, "\n");
		count = hx_diag_arrange_outloop(bit2, bit1 * x_channel, bit0 * y_channel, buf, count);
		count += snprintf(buf + count, HX_MAX_PRBUF_SIZE - count, "%6c", ' ');
		if (bit1 == 1) {
			for (i = x_channel + y_channel - 1; i >= x_channel;  i--)
				count += snprintf(buf + count, HX_MAX_PRBUF_SIZE - count, "%6d", diag_self[i]);
		} else {
			for (i = x_channel; i < x_channel + y_channel; i++)
				count += snprintf(buf + count, HX_MAX_PRBUF_SIZE - count, "%6d", diag_self[i]);
		}
	}
	TS_LOG_INFO("%s, HX_MAX_PRBUF_SIZE = %d, count = %d\n", __func__, HX_MAX_PRBUF_SIZE, count);

	return count;
}

static ssize_t hx_diag_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int count = 0;
	uint32_t loop_i = 0;
	uint32_t loop_j = 0;
	int16_t new_data = 0;
	uint16_t dsram_type = 0;
	uint16_t index = 0;
	uint8_t info_data[MUTUL_NUM_HX83108A * 2] = {0};
	int x_channel = HX_ZF_RX_NUM;
	int y_channel = HX_ZF_TX_NUM;
	dsram_type = g_zf_diag_command / 10;

	TS_LOG_INFO("g_zf_diag_command = %d, dsram_type = %d, x_channel = %d, y_channel = %d\n",
		g_zf_diag_command, dsram_type, x_channel, y_channel);
	if (x_channel == 0) {
		TS_LOG_ERR("diag_show: divided by zero\n");
		return count;
	}

	TS_LOG_INFO("__func__ is %s\n", __func__);
	if ((g_zf_diag_command >= 1 && g_zf_diag_command <= 3) || (g_zf_diag_command == 7)) {
		count = hx_diag_stack_test_print(buf, count);
	} else if (g_zf_diag_command == 8) {
		for (loop_i = 0; loop_i < 128 ; loop_i++) {
			if ((loop_i % 16) == 0)
				count += snprintf(buf, HX_MAX_PRBUF_SIZE - count, "LineStart:");

			count += snprintf(buf, HX_MAX_PRBUF_SIZE - count, "%4x", hx_zf_diag_coor[loop_i]);
			if ((loop_i % 16) == 15)
				count += snprintf(buf, HX_MAX_PRBUF_SIZE-count, "\n");
		}
	} else if (dsram_type > 0 && dsram_type <= 8) {
		hx_zf_burst_enable(1);
		hx_zf_get_dsram_data(info_data);
		for (loop_i = 0, index = 0; loop_i < y_channel; loop_i++) {
			for (loop_j = 0; loop_j < x_channel; loop_j++) {
				new_data = (short)(info_data[index + 1] << 8 | info_data[index]);
				if (dsram_type == 1)
					diag_mutual[loop_i * x_channel + loop_j] = new_data;
				index += 2;
			}
		}
		count = hx_diag_stack_test_print(buf, count);
	}

	return count;
}

static ssize_t hx_diag_dump(struct device *dev, struct device_attribute *attr,  const char *buff,  size_t len)
{
	uint8_t command[2] = {0x00, 0x00};
	int storage_type = 0; // 0: common , other: dsram
	int rawdata_type = 0; // 1:IIR,2:DC
	struct himax_ts_data *ts;
	ts = g_himax_zf_ts_data;

	if (len >= DIAG_COMMAND_MAX_SIZE) {
		TS_LOG_INFO("%s: no command exceeds 80 chars.\n", __func__);
		return -EFAULT;
	}
	TS_LOG_INFO("%s:g_switch_mode = %d,  buff[1] = 0x%x\n", __func__, g_switch_mode, buff[1]);

	if (buff[1] == 0x0A)
		g_zf_diag_command = buff[0] - '0';
	else
		g_zf_diag_command = (buff[0] - '0')*10 + (buff[1] - '0');

	storage_type = himax_determin_diag_storage(g_zf_diag_command);
	rawdata_type = himax_determin_diag_rawdata(g_zf_diag_command);

	if (g_zf_diag_command > 0 && rawdata_type == 0) {
		TS_LOG_INFO("[Himax]g_zf_diag_command=0x%x ,storage_type=%d, rawdata_type=%d! Maybe no support!\n",
			g_zf_diag_command, storage_type, rawdata_type);
		g_zf_diag_command = 0x00;
	} else {
		TS_LOG_INFO("[Himax]g_zf_diag_command=0x%x ,storage_type=%d, rawdata_type=%d\n",
			g_zf_diag_command, storage_type, rawdata_type);
	}
	if (storage_type == 0 && rawdata_type > 0 && rawdata_type < 8) {
		TS_LOG_INFO("%s,common\n", __func__);
		if (zf_dsram_flag) {
			zf_dsram_flag = false; // 1. Clear DSRAM flag
			// 2. Enable ISR
			himax_zf_int_enable(g_himax_zf_ts_data->tskit_himax_data->ts_platform_data->irq_id, 1);
			hx_zf_return_event_stack(); /* 3 FW leave sram and return to event stack */
		}
		if (g_switch_mode == 2) {
			hx_zf_idle_mode(0);
			g_switch_mode = hx_zf_switch_mode(0);
		}
		if (g_zf_diag_command == 0x04) {
			g_zf_diag_command = 0x00;
			command[0] = 0x00;
		} else {
			command[0] = g_zf_diag_command;
		}
		hx_zf_diag_register_set(command[0]);
	} else if (storage_type > 0 && storage_type < 8 && rawdata_type > 0 && rawdata_type < 8) {
		TS_LOG_INFO("%s, dsram\n", __func__);
		diag_max_cnt = 0;
		memset(diag_mutual, 0x00, x_channel * y_channel * sizeof(int16_t)); //Set data 0 everytime
		//0. set diag flag
		if (zf_dsram_flag) {
			//(1) Clear DSRAM flag
			zf_dsram_flag = false;
			//(2) Enable ISR
			himax_zf_int_enable(g_himax_zf_ts_data->tskit_himax_data->ts_platform_data->irq_id, 1);
			/*(3) FW leave sram and return to event stack*/
			hx_zf_return_event_stack();
		}
		/* close sorting if turn on*/
		if (g_switch_mode == 2) {
			hx_zf_idle_mode(0);
			g_switch_mode = hx_zf_switch_mode(0);
		}
		switch (rawdata_type) {
		case 1:
			command[0] = 0x09;  //IIR
			break;
		case 2:
			command[0] = 0x0A;  //RAWDATA
			break;
		case 7:
			command[0] = 0x0B;   //DC
			break;
		default:
			command[0] = 0x00;
			TS_LOG_ERR("%s: Sram no support this type !\n", __func__);
			break;
		}
		hx_zf_diag_register_set(command[0]);
		//1. Disable ISR
		himax_zf_int_enable(g_himax_zf_ts_data->tskit_himax_data->ts_platform_data->irq_id, 0);
		//Open file for save raw data log
		//3. Set DSRAM flag
		zf_dsram_flag = true;
	} else if (storage_type == 8) {
		TS_LOG_INFO("Soritng mode!\n");
		if (zf_dsram_flag) {
			//1. Clear DSRAM flag
			zf_dsram_flag = false;
			//2. Enable ISR
			himax_zf_int_enable(g_himax_zf_ts_data->tskit_himax_data->ts_platform_data->irq_id, 1);
			//3. FW leave sram and return to event stack
			hx_zf_return_event_stack();
		}
		hx_zf_idle_mode(1);
		g_switch_mode = hx_zf_switch_mode(1);
		if (g_switch_mode == 2) {
			if (rawdata_type == 1)
				command[0] = 0x09; //IIR
			else if (rawdata_type == 2)
				command[0] = 0x0A; //DC
			else if (rawdata_type == 7)
				command[0] = 0x08; //BASLINE
			else {
				command[0] = 0x00;
				TS_LOG_ERR("%s: Now Sorting Mode does not support this command=%d\n", __func__, g_zf_diag_command);
			}
			hx_zf_diag_register_set(command[0]);
		}
		zf_dsram_flag = true;
	} else {
		//set diag flag
		if (zf_dsram_flag) {
			TS_LOG_INFO("return and cancel sram thread!\n");
			//1. Clear DSRAM flag
			zf_dsram_flag = false;
			//2. Enable ISR
			himax_zf_int_enable(g_himax_zf_ts_data->tskit_himax_data->ts_platform_data->irq_id, 1);
			//3. FW leave sram and return to event stack
			hx_zf_return_event_stack();
		}
		if (g_switch_mode == 2) {
			hx_zf_idle_mode(0);
			g_switch_mode = hx_zf_switch_mode(0);
		}
		if (g_zf_diag_command != 0x00) {
			TS_LOG_ERR("[Himax]g_zf_diag_command error!diag_command=0x%x so reset\n", g_zf_diag_command);
			command[0] = 0x00;
			if (g_zf_diag_command != 0x08)
				g_zf_diag_command = 0x00;
			hx_zf_diag_register_set(command[0]);
		} else {
			command[0] = 0x00;
			g_zf_diag_command = 0x00;
			hx_zf_diag_register_set(command[0]);
			TS_LOG_INFO("return to normal g_zf_diag_command=0x%x\n", g_zf_diag_command);
		}
	}
	return len;
}

static DEVICE_ATTR(diag, 0600, hx_diag_show, hx_diag_dump); // Debug_diag_done
#endif

#ifdef HX_TP_SYS_REGISTER
static ssize_t hx_register_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int ret = 0;
	uint16_t loop_i = 0;
	uint16_t row_width = 16;
	uint8_t data[REG_COMMON_LEN] = {0};

	TS_LOG_INFO("hx_register_show: %02X,%02X,%02X,%02X\n", register_command[3],
		register_command[2], register_command[1], register_command[0]);

	if (cfg_flag == 1) {
		ret = himax_zf_bus_read(register_command[0], data, REG_COMMON_LEN, MAX_READ_LENTH, DEFAULT_RETRY_CNT);
		if (ret < 0) {
			TS_LOG_ERR("%s: bus access fail!\n", __func__);
			return -EFAULT;
		}
	} else {
		hx_zf_register_read(register_command, REG_COMMON_LEN, data);
	}

	ret += snprintf(buf, HX_MAX_PRBUF_SIZE - ret, "command:  %02X,%02X,%02X,%02X\n", register_command[3],
		register_command[2], register_command[1], register_command[0]);
	for (loop_i = 0; loop_i < REG_COMMON_LEN; loop_i++) {
		ret += snprintf(buf + ret, HX_MAX_PRBUF_SIZE - ret, "0x%2.2X ", data[loop_i]);
		if ((loop_i % row_width) == (row_width - 1))
			ret += snprintf(buf + ret, HX_MAX_PRBUF_SIZE - ret, "\n");
	}
	ret += snprintf(buf + ret, HX_MAX_PRBUF_SIZE - ret, "\n");

	return ret;
}

static ssize_t hx_register_store(struct device *dev, struct device_attribute *attr, const char *buff, size_t count)
{
	char buf_tmp[DIAG_COMMAND_MAX_SIZE] = {0};
	char *data_str = NULL;
	int ret = 0;
	uint16_t base = 2;
	uint8_t buff_size = 0;
	uint8_t length = 0;
	uint8_t loop_i = 0;
	uint8_t flg_cnt = 0;
	uint8_t byte_length = 0;
	uint8_t w_data[DIAG_COMMAND_MAX_SIZE] = {0};
	uint8_t x_pos[DIAG_COMMAND_MAX_SIZE] = {0};
	unsigned long result = 0;

	TS_LOG_INFO("%s:buff = %s, line = %d\n", __func__, buff, __LINE__);
	if (count >= DIAG_COMMAND_MAX_SIZE) {
		TS_LOG_INFO("%s: no command exceeds 80 chars.\n", __func__);
		return -EFAULT;
	}
	TS_LOG_INFO("%s:buff = %s, line = %d, %p\n", __func__, buff, __LINE__, &buff[0]);

	memset(register_command, 0x0, sizeof(register_command));

	if ((buff[0] == 'r' || buff[0] == 'w') && buff[1] == ':' && buff[2] == 'x') {
		length = strlen(buff);
		TS_LOG_INFO("%s: length = %d.\n", __func__, length);
		for (loop_i = 0; loop_i < length; loop_i++) {//find postion of 'x'
			if (buff[loop_i] == 'x') {
				x_pos[flg_cnt] = loop_i;
				flg_cnt++;
			}
		}
		TS_LOG_INFO("%s: flg_cnt = %d.\n", __func__, flg_cnt);
		data_str = strrchr(buff, 'x');
		TS_LOG_INFO("%s: %s.\n", __func__, data_str);
		length = strlen(data_str+1) - 1;
		if (buff[0] == 'r') {
			if (buff[3] == 'F' && buff[4] == 'E' && length == 4) {
				length = length - base;
				cfg_flag = 1;
				memcpy(buf_tmp, data_str + base + 1, length);
				TS_LOG_INFO("buf_tmp1 = %s\n", buf_tmp);
			} else {
				cfg_flag = 0;
				if (length < DIAG_COMMAND_MAX_SIZE)
					memcpy(buf_tmp, data_str + 1, length);
				else
					TS_LOG_INFO("register_store: length more than 80 bytes\n");
			}
			byte_length = length/2;
			if (!kstrtoul(buf_tmp, 16, &result)) {
				for (loop_i = 0; loop_i < byte_length; loop_i++)
					register_command[loop_i] = (uint8_t)(result >> loop_i*8);
			}
			TS_LOG_INFO("%s: buff[0] == 'r'\n", __func__);
		} else if (buff[0] == 'w') {
			if (buff[3] == 'F' && buff[4] == 'E') {
				cfg_flag = 1;
				memcpy(buf_tmp, buff + base + 3, length);
			} else {
				cfg_flag = 0;
				buff_size = (strlen(buff + MOV_3BIT) < DIAG_COMMAND_MAX_SIZE) ? strlen(buff + MOV_3BIT) : DIAG_COMMAND_MAX_SIZE;
				if (length < buff_size)
					memcpy(buf_tmp, buff + MOV_3BIT, length);
				else
					TS_LOG_INFO("register_store: length too long\n");
			}
			if (flg_cnt < 3) {
				byte_length = length/2;
				if (!kstrtoul(buf_tmp, 16, &result)) { //command
					for (loop_i = 0; loop_i < byte_length; loop_i++)
						register_command[loop_i] = (uint8_t)(result >> loop_i*8);
				}
				if (!kstrtoul(data_str + 1, 16, &result)) {//data
					for (loop_i = 0; loop_i < byte_length; loop_i++)
						w_data[loop_i] = (uint8_t)(result >> loop_i*8);
				}
				himax_zf_register_write(register_command, byte_length, w_data);
				TS_LOG_INFO("%s: buff[0] == 'w' && flg_cnt < 3\n", __func__);
			} else {
				for (loop_i = 0; loop_i < flg_cnt; loop_i++) { // parsing addr after 'x'
					if (cfg_flag != 0 && loop_i != 0)
						byte_length = 2;
					else
						byte_length = x_pos[1] - x_pos[0] - 2;
					/* original */
					memcpy(buf_tmp, buff + x_pos[loop_i] + 1, byte_length);
					TS_LOG_INFO("%s: buf_tmp = %s\n", __func__, buf_tmp);
					if (!kstrtoul(buf_tmp, 16, &result)) {
						if (loop_i == 0) {
							register_command[loop_i] = (uint8_t)(result);
							TS_LOG_INFO("%s: register_command = %X\n", __func__, register_command[0]);
						} else {
							w_data[loop_i - 1] = (uint8_t)(result);
							TS_LOG_INFO("%s: w_data[%d] = %2X\n", __func__, loop_i - 1, w_data[loop_i - 1]);
						}
					}
				}
				byte_length = flg_cnt - 1;
				himax_zf_register_write(register_command, byte_length, &w_data[0]);
				TS_LOG_INFO("%s: buff[0] == 'w' && flg_cnt >= 3\n", __func__);
			}
			if (cfg_flag == 1) {
				TS_LOG_INFO("proc_reg_addr[0] = %x, w_data = %s\n", register_command[0], w_data);
				ret = hx_zf_bus_write(register_command[0], w_data, byte_length, sizeof(w_data), DEFAULT_RETRY_CNT);
				if (ret < 0) {
					TS_LOG_ERR("%s: bus access fail!\n", __func__);
					return -EFAULT;
				}
			} else {
				himax_zf_register_write(register_command, byte_length, w_data);
			}
		} else {
			return count;
		}
	}

	return count;
}

static DEVICE_ATTR(register, 0600, hx_register_show, hx_register_store); // Debug_register_done
#endif
#ifdef HX_TP_SYS_DEBUG
#define ROW_LEN 7
#define COL_LEN 7
#define HEADER_LEN 4
#define OFFSET 8

static ssize_t himax_debug_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	size_t retval = 0;
	uint8_t roi_data[54] = {0};
	int i;
	int j;

	if (debug_level_cmd == 't') {
		if (fw_update_complete)
			retval += snprintf(buf, HX_MAX_PRBUF_SIZE-retval, "FW Update Complete ");
		else
			retval += snprintf(buf, HX_MAX_PRBUF_SIZE-retval, "FW Update Fail ");
	} else if (debug_level_cmd == 'v') {
		himax_zf_read_FW_info();
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "FW_VER = ");
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "0x%2.2X \n", g_himax_zf_ts_data->vendor_fw_ver);
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "CONFIG_VER = ");
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "0x%2.2X \n", g_himax_zf_ts_data->vendor_config_ver);
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "PANEL_VER = ");
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "0x%2.2X \n", g_himax_zf_ts_data->vendor_panel_ver);
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "CID_VER = ");
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "0x%2.2X \n",
			g_himax_zf_ts_data->vendor_cid_maj_ver << 8 | g_himax_zf_ts_data->vendor_cid_min_ver);
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "\n");
	} else if (debug_level_cmd == 'd') {
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE-retval, "Himax Touch IC Information :\n");
		if (IC_ZF_TYPE == HX_83108A_SERIES_PWON)
			retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "IC Type : HX83108A\n");
		else
			retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "IC Type error.\n");

		if (IC_ZF_CHECKSUM == HX_TP_BIN_CHECKSUM_SW)
			retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "IC Checksum : SW\n");
		else if (IC_ZF_CHECKSUM == HX_TP_BIN_CHECKSUM_HW)
			retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "IC Checksum : HW\n");
		else if (IC_ZF_CHECKSUM == HX_TP_BIN_CHECKSUM_CRC)
			retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "IC Checksum : CRC\n");
		else
			retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "IC Checksum error.\n");

		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "RX Num : %d\n", HX_ZF_RX_NUM);
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "TX Num : %d\n", HX_ZF_TX_NUM);
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "BT Num : %d\n", HX_ZF_BT_NUM);
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "X Resolution : %d\n", HX_ZF_X_RES);
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "Y Resolution : %d\n", HX_ZF_Y_RES);
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "Max Point : %d\n", HX_ZF_MAX_PT);
	} else if (debug_level_cmd == 'i') {
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "Himax Touch Driver Version:\n");
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "%s \n", HIMAX_DRIVER_VER);
	} else if (debug_level_cmd == 'y') {
		roi_data[0] = g_himax_zf_ts_data->hx_rawdata_buf_roi[5];
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "%6d\t", roi_data[0]);
		roi_data[1] = g_himax_zf_ts_data->hx_rawdata_buf_roi[4];
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "%6d\t", roi_data[1]);
		roi_data[2] = g_himax_zf_ts_data->hx_rawdata_buf_roi[7];
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "%6d\t", roi_data[2]);
		roi_data[3] = g_himax_zf_ts_data->hx_rawdata_buf_roi[6];
		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "%6d\t", roi_data[3]);

		retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "\n");

		for (i = 0; i < HX_ROI_DATA_LENGTH; i++)
			roi_data[i + HEADER_LEN] = (g_himax_zf_ts_data->hx_rawdata_buf_roi[i * 2 + OFFSET + 1] << 8) |
				g_himax_zf_ts_data->hx_rawdata_buf_roi[i * 2 + OFFSET];

		for (j = 0; j < ROW_LEN; j++) {
			for (i = 0; i < COL_LEN; i++)
				retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "%6d\t", roi_data[HEADER_LEN + j + i * ROW_LEN]);

			retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "\n");
		}

		memset(g_himax_zf_ts_data->hx_rawdata_buf_roi, 0, sizeof(uint8_t) * (HX_RECEIVE_ROI_BUF_MAX_SIZE - HX_TOUCH_SIZE));
	} else {
		return -EINVAL;
	}
	return retval;
}

static int hx_node_update(void)
{
	int result;
	char firmware_name[64] = {0};
	const struct firmware *fw = NULL;
	himax_update_fw = HIMAX_UPDATE_FW;

	snprintf(firmware_name, sizeof(firmware_name), "ts/%s", himax_update_fw);
	/* manual upgrade will not use embedded firmware */
	result = request_firmware(&fw, firmware_name, &g_himax_zf_ts_data->tskit_himax_data->ts_platform_data->ts_dev->dev);
	if (result < 0) {
		TS_LOG_ERR("request FW %s failed\n", firmware_name);
		return result;
	}

	firmware_update(fw);
	release_firmware(fw);
	TS_LOG_INFO("%s: end!\n", __func__);
}

static ssize_t hx_debug_dump(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	char fileName[REG_COMMON_LEN] = {0};
	TS_LOG_INFO("%s: enter\n", __func__);

	if (count >= DIAG_COMMAND_MAX_SIZE) {
		TS_LOG_INFO("%s: no command exceeds 80 chars.\n", __func__);
		return -EFAULT;
	}
	if (buf[0] == 'v') { // firmware version
		debug_level_cmd = buf[0];
		return count;
	} else if (buf[0] == 'd') { // test
		debug_level_cmd = buf[0];
		return count;
	} else if (buf[0] == 'i') { // driver version
		debug_level_cmd = buf[0];
		return count;
	} else if (buf[0] == 't') {
		debug_level_cmd = buf[0];
		fw_update_complete	= false;
		TS_LOG_INFO("%s: count = %d, len = %d\n", __func__, (int)count, strlen(buf));
		hx_node_update();
	} else {
		return -EINVAL;
	}

	return count;
}

static DEVICE_ATTR(debug, 0600, himax_debug_show, hx_debug_dump); // Debug_debug_done
#endif

static ssize_t himax_debug_level_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t retval = 0;
	struct himax_ts_data *ts_data;
	ts_data = g_himax_zf_ts_data;
	retval += snprintf(buf, HX_MAX_PRBUF_SIZE-retval, "%d\n", ts_data->debug_log_level);
	return retval;
}

static ssize_t himax_debug_level_dump(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int loop_i = 0;
	struct himax_ts_data *ts = NULL;
	int flg = (int)count;
	ts = g_himax_zf_ts_data;
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
		if (ts->pdata->screenWidth > 0 && ts->pdata->screenHeight > 0 &&
			(ts->pdata->abs_x_max - ts->pdata->abs_x_min) > 0 &&
			(ts->pdata->abs_y_max - ts->pdata->abs_y_min) > 0) {
			ts->widthFactor = (ts->pdata->screenWidth << SHIFTBITS)/(ts->pdata->abs_x_max - ts->pdata->abs_x_min);
			ts->heightFactor = (ts->pdata->screenHeight << SHIFTBITS)/(ts->pdata->abs_y_max - ts->pdata->abs_y_min);
			if (ts->widthFactor > 0 && ts->heightFactor > 0)
				ts->useScreenRes = 1;
			else {
				ts->heightFactor = 0;
				ts->widthFactor = 0;
				ts->useScreenRes = 0;
			}
		} else
			TS_LOG_INFO("Enable finger debug with raw position mode!\n");
	} else {
		ts->useScreenRes = 0;
		ts->widthFactor = 0;
		ts->heightFactor = 0;
	}

	return count;
}

static DEVICE_ATTR(debug_level, 0600, himax_debug_level_show, himax_debug_level_dump);

static ssize_t touch_vendor_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t retval = 0;

	retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "FW_VER = ");
	retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "0x%2.2X \n", g_himax_zf_ts_data->vendor_fw_ver);
	retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "CONFIG_VER = ");
	retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "0x%2.2X \n", g_himax_zf_ts_data->vendor_config_ver);
	retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "PANEL_VER = ");
	retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "0x%2.2X \n", g_himax_zf_ts_data->vendor_panel_ver);
	retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "CID_VER = ");
	retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "0x%2.2X \n",
		g_himax_zf_ts_data->vendor_cid_maj_ver << 8 | g_himax_zf_ts_data->vendor_cid_min_ver);
	retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "\n");
	return retval;
}

static DEVICE_ATTR(vendor, 0600, touch_vendor_show, NULL); // Debug_vendor_done

static ssize_t touch_attn_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t retval = 0;
	struct himax_ts_data *ts = NULL;
	ts = g_himax_zf_ts_data;
	retval += snprintf(buf + retval, HX_MAX_PRBUF_SIZE - retval, "%x",
		gpio_get_value(ts->tskit_himax_data->ts_platform_data->irq_gpio));

	return retval;
}

static DEVICE_ATTR(attn, 0600, touch_attn_show, NULL); // Debug_attn_done

static ssize_t himax_int_status_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	size_t retval = 0;
	retval = snprintf(buf, HX_MAX_PRBUF_SIZE,  "%d ", hx_zf_irq_enable_count);
	return retval;
}

static ssize_t hx_int_status_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int value = 0;
	struct himax_ts_data *ts = g_himax_zf_ts_data;

	if (count >= DIAG_INT_COMMAND_MAX_SIZE) {
		TS_LOG_INFO("%s: no command exceeds 12 chars.\n", __func__);
		return -EFAULT;
	}
	if (buf[0] == '0')
		value = false;
	else if (buf[0] == '1')
		value = true;
	else
		return -EINVAL;

	if (value) {
		himax_zf_int_enable(ts->tskit_himax_data->ts_platform_data->irq_id, 1);
		ts->irq_enabled = 1;
		hx_zf_irq_enable_count = 1;
	} else {
		himax_zf_int_enable(ts->tskit_himax_data->ts_platform_data->irq_id, 0);
		ts->irq_enabled = 0;
		hx_zf_irq_enable_count = 0;
	}

	return count;
}

static ssize_t hx_diag_arr_store(struct device *dev, struct device_attribute *attr, char *buf)
{
	g_diag_arr_num = buf[0] - '0';
	TS_LOG_INFO("%s: g_diag_arr_num = %d\n", __func__, g_diag_arr_num);

	return NO_ERR;
}

// Debug_int_en_done
static DEVICE_ATTR(int_en, 0600, himax_int_status_show, hx_int_status_store);

static ssize_t himax_senseronoff_status_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	if (buf[0] == '0') {
		hx_zf_sense_off();
	} else if (buf[0] == '1') {
		if (buf[1] == 's') {
			hx_zf_sense_on(0x00);
			TS_LOG_INFO("Sense on re-map on, run sram\n");
		} else {
			hx_zf_sense_on(0x01);
			TS_LOG_INFO("Sense on re-map off, run flash\n");
		}
	} else {
		return -EINVAL;
	}

	return count;
}

// Debug_int_en_done
static DEVICE_ATTR(senseronoff, 0600, NULL, himax_senseronoff_status_store);

static struct attribute *hx_attributes[] = {
	&dev_attr_reset.attr,
	&dev_attr_diag_arr.attr,
	&dev_attr_diag.attr,
	&dev_attr_register.attr,
	&dev_attr_debug.attr,
	&dev_attr_debug_level.attr,
	&dev_attr_vendor.attr,
	&dev_attr_attn.attr,
	&dev_attr_int_en.attr,
	&dev_attr_senseronoff.attr,
	NULL
};

const struct attribute_group hx_attr_group = {
	.attrs = hx_attributes,
};

int hx_zf_touch_sysfs_init(void)
{
	int ret = 0;
	TS_LOG_INFO("%s: init hx touch sysfs!", __func__);

	hx_dev = platform_device_alloc("hx_debug", -1);
	if (!hx_dev) {
		TS_LOG_ERR("platform device malloc failed\n");
		ret = -ENOMEM;
		goto out;
	}
	ret = platform_device_add(hx_dev);
	if (ret) {
		TS_LOG_ERR("platform device add failed :%d\n", ret);
		goto err_put_platform_dev;
	}
	ret = sysfs_create_group(&hx_dev->dev.kobj, &hx_attr_group);
	if (ret) {
		TS_LOG_ERR("can't create himax's sysfs\n");
		goto err_del_platform_dev;
	}
	TS_LOG_INFO("sysfs_create_group success\n");
	ret = sysfs_create_link(NULL, &hx_dev->dev.kobj, "hx_debug");
	if (ret) {
		TS_LOG_ERR("%s create link error = %d\n", __func__, ret);
		goto err_free_sysfs;
	}
	goto out;

err_free_sysfs:
	sysfs_remove_group(&hx_dev->dev.kobj, &hx_attr_group);
err_del_platform_dev:
	platform_device_del(hx_dev);
err_put_platform_dev:
	platform_device_put(hx_dev);
out:
	return ret;
}

void himax_zf_touch_sysfs_deinit(void)
{
	sysfs_remove_link(NULL, "hx_debug");
	sysfs_remove_group(&hx_dev->dev.kobj, &hx_attr_group);
	platform_device_unregister(hx_dev);
}
