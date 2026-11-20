/*
 * ILITEK Touch IC driver
 *
 * Copyright (C) 2011 ILI Technology Corporation.
 *
 * Author: Dicky Chiang <dicky_chiang@ilitek.com>
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "ilitek.h"

#define VALUE 0
#define RETRY_COUNT 3
#define INT_CHECK 0
#define POLL_CHECK 1

#define BENCHMARK 1
#define NODETYPE 1

#define TYPE_BENCHMARK 0
#define TYPE_NO_JUGE 1
#define TYPE_JUGE 2

#define NORMAL_CSV_PASS_NAME "mp_pass"
#define NORMAL_CSV_FAIL_NAME "mp_fail"
#define NORMAL_CSV_WARNING_NAME "mp_warning"

#define CSV_FILE_SIZE (1 * M)

#define PARSER_MAX_CFG_BUF (512 * 3)
#define PARSER_MAX_KEY_NUM (600 * 3)
#define PARSER_MAX_KEY_NAME_LEN 100
#define PARSER_MAX_KEY_VALUE_LEN 2000
#define MAX_SECTION_NUM 100
#define BENCHMARK_KEY_NAME "benchmark_data"
#define NODE_TYPE_KEY_NAME "node type"
#define INI_ERR_OUT_OF_LINE -1

#define CMD_MUTUAL_DAC 0x1
#define CMD_MUTUAL_BG 0x2
#define CMD_MUTUAL_SIGNAL 0x3
#define CMD_MUTUAL_NO_BK 0x5
#define CMD_MUTUAL_HAVE_BK 0x8
#define CMD_MUTUAL_BK_DAC 0x10
#define CMD_SELF_DAC 0xC
#define CMD_SELF_BG 0xF
#define CMD_SELF_SIGNAL 0xD
#define CMD_SELF_NO_BK 0xE
#define CMD_SELF_HAVE_BK 0xB
#define CMD_SELF_BK_DAC 0x11
#define CMD_KEY_DAC 0x14
#define CMD_KEY_BG 0x16
#define CMD_KEY_NO_BK 0x7
#define CMD_KEY_HAVE_BK 0x15
#define CMD_KEY_OPEN 0x12
#define CMD_KEY_SHORT 0x13
#define CMD_ST_DAC 0x1A
#define CMD_ST_BG 0x1C
#define CMD_ST_NO_BK 0x17
#define CMD_ST_HAVE_BK 0x1B
#define CMD_ST_OPEN 0x18
#define CMD_TX_SHORT 0x19
#define CMD_RX_SHORT 0x4
#define CMD_RX_OPEN 0x6
#define CMD_TX_RX_DELTA 0x1E
#define CMD_CM_DATA 0x9
#define CMD_CS_DATA 0xA
#define CMD_TRCRQ_PIN 0x20
#define CMD_RESX2_PIN 0x21
#define CMD_MUTUAL_INTEGRA_TIME 0x22
#define CMD_SELF_INTEGRA_TIME 0x23
#define CMD_KEY_INTERGRA_TIME 0x24
#define CMD_ST_INTERGRA_TIME 0x25
#define CMD_PEAK_TO_PEAK 0x1D
#define CMD_GET_TIMING_INFO 0x30
#define CMD_DOZE_P2P 0x32
#define CMD_DOZE_RAW 0x33
#define CMD_PIN_TEST 0x61
#define TIME_INFO_LEN 16
#define PEAK_TD_LEN 28

#define MP_DATA_PASS 0
#define MP_DATA_FAIL -1

#define mathabs(x) ({					\
		long ret;				\
		long __x = (x);				\
		ret = (__x < 0) ? -__x : __x;		\
		ret;					\
	})

#define dump(fmt, arg...)		\
	do {				\
		if (debug_en)	\
		pr_cont(fmt, ##arg);	\
	} while (0)

static struct ini_file_data {
	char section_name[PARSER_MAX_KEY_NAME_LEN];
	char key_name[PARSER_MAX_KEY_NAME_LEN];
	char key_value[PARSER_MAX_KEY_VALUE_LEN];
	int section_len;
	int key_name_len;
	int key_val_len;
} *ini_info;

enum open_test_node_type {
	NO_COMPARE = 0x00,	/* Not A Area, No Compare */
	AA_AREA = 0x01,		/* AA Area, Compare using charge_aa */
	BORDER_AREA = 0x02, 	/* Border Area, Compare using charge_boarder */
	NOTCH = 0x04,		/* NOTCH Area, Compare using charge_notch */
	ROUND_CORNER = 0x08,	/* Round Corner, No Compare */
	SKIP_MICRO = 0x10	/* SKIP_MICRO, No Compare */
};

enum mp_test_catalog {
	MUTUAL_TEST = 0,
	SELF_TEST = 1,
	KEY_TEST = 2,
	ST_TEST = 3,
	TX_RX_DELTA = 4,
	UNTOUCH_P2P = 5,
	PIXEL = 6,
	OPEN_TEST = 7,
	PEAK_TO_PEAK_TEST = 8,
	SHORT_TEST = 9,
	PIN_TEST = 10,
};

struct mp_test_p540_open {
	s32 *tdf_700;
	s32 *tdf_250;
	s32 *tdf_200;
	s32 *cbk_700;
	s32 *cbk_250;
	s32 *cbk_200;
	s32 *charg_rate;
	s32 *full_open;
	s32 *dac;
};

struct mp_test_open_c {
	s32 *cap_dac;
	s32 *cap_raw;
	s32 *dcl_cap;
};

static struct open_test_spec {
	int tvch;
	int tvcl;
	int gain;
	int cbk_step;
	int cint;
} open_spec;

static struct shor_test_para {
	int tvch;
	int tvcl;
	int variation;
	int rinternal;
	int cint;
} short_para;

static struct core_mp_test_data {
	u32 chip_pid;
	u16 chip_id;
	u8 chip_type;
	u8 chip_ver;
	u32 fw_ver;
	u32 protocol_ver;
	u32 core_ver;
	char ini_date[128];
	char ini_ver[64];
	int no_bk_shift;
	bool retry;
	bool m_signal;
	bool m_dac;
	bool s_signal;
	bool s_dac;
	bool key_dac;
	bool st_dac;
	bool p_no_bk;
	bool p_has_bk;
	bool open_integ;
	bool open_cap;
	bool is_longv;
	bool td_retry;

	int cdc_len;
	int xch_len;
	int ych_len;
	int stx_len;
	int srx_len;
	int key_len;
	int st_len;
	int frame_len;
	int mp_items;
	int final_result;
	int short_varia;

	u32 overlay_start_addr;
	u32 overlay_end_addr;
	u32 mp_flash_addr;
	u32 mp_size;
	u8 dma_trigger_enable;

	/* Tx/Rx threshold & buffer */
	int tx_delta_max;
	int tx_delta_min;
	int rx_delta_max;
	int rx_delta_min;
	s32 *tx_delta_buf;
	s32 *rx_delta_buf;
	s32 *tx_max_buf;
	s32 *tx_min_buf;
	s32 *rx_max_buf;
	s32 *rx_min_buf;

	int tdf;
	int busy_cdc;
	bool ctrl_lcm;
	bool lost_benchmark;
	bool lost_parameter;
} core_mp = {0};

struct mp_test_items {
	/* The description must be the same as ini's section name */
	char *desp;
	char *result;
	int catalog;
	u8 cmd;
	u8 spec_option;
	u8 type_option;
	bool run;
	bool lcm;
	int bch_mrk_multi;
	int max;
	int max_res;
	int item_result;
	int min;
	int min_res;
	int frame_count;
	int trimmed_mean;
	int lowest_percentage;
	int highest_percentage;
	int v_tdf_1;
	int v_tdf_2;
	int h_tdf_1;
	int h_tdf_2;
	int goldenmode;
	int max_min_mode;
	int bch_mrk_frm_num;
	int retry_cnt;
	u8  delay_time;
	u8  test_int_pin;
	u8  int_pulse_test;
	s32 *result_buf;
	s32 *buf;
	s32 *max_buf;
	s32 *min_buf;
	s32 *bench_mark_max;
	s32 *bench_mark_min;
	s32 **bch_mrk_max;
	s32 **bch_mrk_min;
	s32 *node_type;
	int (*do_test)(int index);
};

#define MP_TEST_ITEM	50
static struct mp_test_items titems[MP_TEST_ITEM] = {
	{.desp = "baseline data(bg)", .catalog = MUTUAL_TEST, .cmd = CMD_MUTUAL_BG, .lcm = ON},
	{.desp = "untouch signal data(bg-raw-4096) - mutual", .catalog = MUTUAL_TEST, .cmd = CMD_MUTUAL_SIGNAL, .lcm = ON},
	{.desp = "manual bk data(mutual)", .catalog = MUTUAL_TEST, .cmd = CMD_MUTUAL_BK_DAC, .lcm = ON},
	{.desp = "calibration data(dac) - self", .catalog = SELF_TEST, .cmd = CMD_SELF_DAC, .lcm = ON},
	{.desp = "baselin data(bg,self_tx,self_r)", .catalog = SELF_TEST, .cmd = CMD_SELF_BG, .lcm = ON},
	{.desp = "untouch signal data(bg-raw-4096) - self", .catalog = SELF_TEST, .cmd = CMD_SELF_SIGNAL, .lcm = ON},
	{.desp = "raw data(no bk) - self", .catalog = SELF_TEST, .cmd = CMD_SELF_NO_BK, .lcm = ON},
	{.desp = "raw data(have bk) - self", .catalog = SELF_TEST, .cmd = CMD_SELF_HAVE_BK, .lcm = ON},
	{.desp = "manual bk dac data(self_tx,self_rx)", .catalog = SELF_TEST, .cmd = CMD_SELF_BK_DAC, .lcm = ON},
	{.desp = "calibration data(dac/icon)", .catalog = KEY_TEST, .cmd = CMD_KEY_DAC, .lcm = ON},
	{.desp = "key baseline data", .catalog = KEY_TEST, .cmd = CMD_KEY_BG, .lcm = ON},
	{.desp = "key raw data", .catalog = KEY_TEST, .cmd = CMD_KEY_NO_BK, .lcm = ON},
	{.desp = "key raw bk dac", .catalog = KEY_TEST, .cmd = CMD_KEY_HAVE_BK, .lcm = ON},
	{.desp = "key raw open test", .catalog = KEY_TEST, .cmd = CMD_KEY_OPEN, .lcm = ON},
	{.desp = "key raw short test", .catalog = KEY_TEST, .cmd = CMD_KEY_SHORT, .lcm = ON},
	{.desp = "st calibration data(dac)", .catalog = ST_TEST, .cmd = CMD_ST_DAC, .lcm = ON},
	{.desp = "st baseline data(bg)", .catalog = ST_TEST, .cmd = CMD_ST_BG, .lcm = ON},
	{.desp = "st raw data(no bk)", .catalog = ST_TEST, .cmd = CMD_ST_NO_BK, .lcm = ON},
	{.desp = "st raw(have bk)", .catalog = ST_TEST, .cmd = CMD_ST_HAVE_BK, .lcm = ON},
	{.desp = "st open data", .catalog = ST_TEST, .cmd = CMD_ST_OPEN, .lcm = ON},
	{.desp = "tx short test", .catalog = MUTUAL_TEST, .cmd = CMD_TX_SHORT, .lcm = ON},
	{.desp = "rx open", .catalog = MUTUAL_TEST, .cmd = CMD_RX_OPEN, .lcm = ON},
	{.desp = "untouch cm data", .catalog = MUTUAL_TEST, .cmd = CMD_CM_DATA, .lcm = ON},
	{.desp = "untouch cs data", .catalog = MUTUAL_TEST, .cmd = CMD_CS_DATA, .lcm = ON},
	{.desp = "tx/rx delta", .catalog = TX_RX_DELTA, .cmd = CMD_TX_RX_DELTA, .lcm = ON},
	{.desp = "untouch peak to peak", .catalog = UNTOUCH_P2P, .cmd = CMD_MUTUAL_SIGNAL, .lcm = ON},
	{.desp = "pixel raw (no bk)", .catalog = PIXEL, .cmd = CMD_MUTUAL_NO_BK, .lcm = ON},
	{.desp = "pixel raw (have bk)", .catalog = PIXEL, .cmd = CMD_MUTUAL_HAVE_BK, .lcm = ON},
	{.desp = "noise peak to peak(cut panel)", .catalog = PEAK_TO_PEAK_TEST, .lcm = ON},
	{.desp = "open test(integration)", .catalog = OPEN_TEST, .cmd = CMD_RX_SHORT, .lcm = ON},
	{.desp = "open test(cap)", .catalog = OPEN_TEST, .cmd = CMD_RX_SHORT, .lcm = ON},
	/* Following is the new test items for protocol 5.4.0 above */
	{.desp = "pin test ( int and rst )", .catalog = PIN_TEST, .cmd = CMD_PIN_TEST, .lcm = ON},
	{.desp = "noise peak to peak(with panel)", .catalog = PEAK_TO_PEAK_TEST, .lcm = ON},
	{.desp = "noise peak to peak(ic only)", .catalog = PEAK_TO_PEAK_TEST, .cmd = CMD_PEAK_TO_PEAK, .lcm = ON},
	{.desp = "open test(integration)_sp", .catalog = OPEN_TEST, .lcm = ON},
	{.desp = "raw data(no bk)", .catalog = MUTUAL_TEST, .cmd = CMD_MUTUAL_NO_BK, .lcm = ON},
	{.desp = "raw data(have bk)", .catalog = MUTUAL_TEST, .cmd = CMD_MUTUAL_HAVE_BK, .lcm = ON},
	{.desp = "calibration data(dac)", .catalog = MUTUAL_TEST, .cmd = CMD_MUTUAL_DAC, .lcm = ON},
	{.desp = "short test -ili9881", .catalog = SHORT_TEST, .cmd = CMD_RX_SHORT, .lcm = ON},
	{.desp = "short test", .catalog = SHORT_TEST, .lcm = ON},
	{.desp = "doze raw data", .catalog = MUTUAL_TEST, .lcm = ON},
	{.desp = "doze peak to peak", .catalog = PEAK_TO_PEAK_TEST, .lcm = ON},
	{.desp = "open test_c", .catalog = OPEN_TEST, .lcm = ON},
	{.desp = "touch deltac", .catalog = MUTUAL_TEST, .lcm = ON},
	/* LCM OFF TEST */
	{.desp = "raw data(have bk) (lcm off)", .catalog = MUTUAL_TEST, .lcm = OFF},
	{.desp = "raw data(no bk) (lcm off)", .catalog = MUTUAL_TEST, .lcm = OFF},
	{.desp = "noise peak to peak(with panel) (lcm off)", .catalog = PEAK_TO_PEAK_TEST, .lcm = OFF},
	{.desp = "noise peak to peak(ic only) (lcm off)", .catalog = PEAK_TO_PEAK_TEST, .lcm = OFF},
	{.desp = "raw data_td (lcm off)", .catalog = MUTUAL_TEST, .lcm = OFF},
	{.desp = "peak to peak_td (lcm off)", .catalog = PEAK_TO_PEAK_TEST, .lcm = OFF},
};

static struct run_index {
	int count;
	int index[MP_TEST_ITEM];
} g_run_index;

static s32 *frame_buf;
static s32 **frm_buf;
static s32 *key_buf;
static s32 *frame1_cbk700, *frame1_cbk250, *frame1_cbk200;
static s32 *cap_dac, *cap_raw;
static int g_ini_items;
static int g_katoi_len;
static char csv_name[128] = {0};
static char seq_item[MAX_SECTION_NUM][PARSER_MAX_KEY_NAME_LEN] = {{0}};

char ts_test_failed_reason[ILITEK_TEST_FAILED_REASON_LEN] = {0};

static void ilitek_show_rawdata(int index,struct ts_rawdata_info *raw_info);

static int ilitek_test_get_ini_path(void)
{
	char file_name[ILITEK_FILE_NAME_LEN] = {0};
	struct ts_kit_device_data *p_dev_data = ilits->ts_dev_data;

	if (!strnlen(p_dev_data->ts_platform_data->product_name, MAX_STR_LEN - 1)
		|| !strnlen(p_dev_data->chip_name, MAX_STR_LEN - 1)
		|| !strnlen(ilits->project_id, ILITEK_PROJECT_ID_LEN)
		|| !strnlen(p_dev_data->module_name, MAX_STR_LEN - 1)) {
		TS_LOG_ERR("MP INI name is not detected\n");
		return -EINVAL;
	}
	if (p_dev_data->hide_fw_name)
		snprintf(file_name, ILITEK_FILE_NAME_LEN,
			"%s_raw.ini", ilits->project_id);
	else
		snprintf(file_name, ILITEK_FILE_NAME_LEN, "%s_%s_%s_%s_raw.ini",
			p_dev_data->ts_platform_data->product_name,
			p_dev_data->chip_name, ilits->project_id,
			p_dev_data->module_name);

	snprintf(ilits->ini_path, ILITEK_FILE_PATH_LEN, "%s%s",
		ILITEK_INI_PATH_PERFIX,
		file_name);

	TS_LOG_INFO("ini file path = %s\n", ilits->ini_path);

	return 0;
}

int ilitek_tddi_rawdata_print(struct seq_file *m, struct ts_rawdata_info *info,
	int range_size, int row_size)
{
	int i = 0, index = 0, offset = 0;
	int x_ch = 0, y_ch = 0, data_nums = 0;
	int row = 0, column = 0;

	x_ch = core_mp.xch_len;
	y_ch = core_mp.ych_len;
	data_nums = core_mp.frame_len;

	for (offset = 2, i = 0; i < g_run_index.count; i++) {
		index = g_run_index.index[i];
		seq_printf(m, "%s\n", titems[index].desp);
		seq_printf(m, "frame count: %d\n", titems[index].frame_count);
		for (row = 0; row < y_ch; row++) {
			for (column = 0; column < x_ch; column++)
				seq_printf(m, "%5d,", info->buff[offset + row * x_ch + column]);
			seq_printf(m, "\n");
		}
		offset += data_nums;
		seq_printf(m, "\n");
	}

	return 0;
}

void ilitek_result_init(struct ts_rawdata_info *info,u8 *result_str)
{
	u8 module_info[ILITEK_TEST_MODULE_LEN] = {0};

	if (!info || !result_str) {
		TS_LOG_ERR("info or result_str is null\n");
		return;
	}

	strncat(result_str, "0F-1F-3F-5F-xF-", strlen("0F-1F-3F-5F-xF-"));
	strncat(result_str,
		ts_test_failed_reason, ILITEK_TEST_FAILED_REASON_LEN - 1);
	sprintf(module_info, "%s%x-%s",
		TDDI_DEV_ID,
		ilits->chip->pid,
		ilits->project_id);

	strncat(result_str, module_info, strlen(module_info));

	/* avoid pull result file error */
	strncat(result_str, ";", strlen(";"));
	if (strlen(result_str) >= TS_RAWDATA_RESULT_MAX)
		strncpy(info->result, result_str, TS_RAWDATA_RESULT_MAX - 1);
	else
		strncpy(info->result, result_str, strlen(result_str));
}
static int isspace_t(int x)
{
	if (x == ' ' || x == '\t' || x == '\n' ||
			x == '\f' || x == '\b' || x == '\r')
		return 1;
	else
		return 0;
}

static void dump_benchmark_data(s32 *max_ptr, s32 *min_ptr)
{
	ili_dump_data(max_ptr, 32, core_mp.frame_len, 0, "Dump Benchmark Max : ");
	ili_dump_data(min_ptr, 32, core_mp.frame_len, 0, "Dump Benchmark Min : ");
}

static void dump_node_type_buffer(s32 *node_ptr, u8 *name)
{
	ili_dump_data(node_ptr, 32, core_mp.frame_len, 0, "Dump NodeType : ");
}

static int parser_get_ini_key_value(const char *section, char *key, char *value, int length)
{
	int i = 0;
	int ret = -2;

	for (i = 0; i < g_ini_items; i++) {
		if (ipio_strcmp(section, ini_info[i].section_name) != 0)
			continue;

		if (ipio_strcmp(key, ini_info[i].key_name) == 0) {
			if (ini_info[i].key_val_len > length)
				memcpy(value, ini_info[i].key_value, length);
			else
				memcpy(value, ini_info[i].key_value, ini_info[i].key_val_len);
			ret = 0;
			break;
		}
	}
	return ret;
}

static void parser_ini_nodetype(s32 *type_ptr, char *desp, int frame_len)
{
	int i = 0, j = 0, index1 = 0, temp, count = 0;
	char str[512] = {0}, record = ',';

	for (i = 0; i < g_ini_items; i++) {
		if (!strstr(ini_info[i].section_name, desp) ||
			ipio_strcmp(ini_info[i].key_name, NODE_TYPE_KEY_NAME) != 0) {
			continue;
		}

		record = ',';
		for (j = 0, index1 = 0; j <= ini_info[i].key_val_len; j++) {
			if (ini_info[i].key_value[j] == ';' || j == ini_info[i].key_val_len) {
				if (record != '.') {
					memset(str, 0, sizeof(str));
					if (j - index1 > sizeof(str))
						memcpy(str, &ini_info[i].key_value[index1], sizeof(str));
					else
						memcpy(str, &ini_info[i].key_value[index1], (j - index1));
					temp = ili_katoi(str, g_katoi_len);

					/* Over boundary, end to calculate. */
					if (count >= frame_len) {
						TS_LOG_ERR("count%d is larger than frame length, break\n", count);
						break;
					}
					type_ptr[count] = temp;
					count++;
				}
				record = ini_info[i].key_value[j];
				index1 = j + 1;
			}
		}
	}
}

static void parser_ini_benchmark(s32 *max_ptr,
	s32 *min_ptr, int8_t type, const char *desp, int frame_len, char *bchmrk_name)
{
	int i = 0, j = 0, index1 = 0, temp, count = 0;
	char str[512] = {0}, record = ',';
	s32 data[4];
	char benchmark_str[256] = {0};

	/* format complete string from the name of section "_Benchmark_Data". */
	snprintf(benchmark_str,sizeof(benchmark_str), "%s%s%s", desp, "_", bchmrk_name);
	TS_LOG_DEBUG("benchmark_str = %s\n", benchmark_str);

	for (i = 0; i < g_ini_items; i++) {
		if ((ipio_strcmp(ini_info[i].section_name, benchmark_str) != 0))
			continue;
		record = ',';
		for (j = 0, index1 = 0; j <= ini_info[i].key_val_len; j++) {
			if (ini_info[i].key_value[j] == ',' || ini_info[i].key_value[j] == ';' ||
				ini_info[i].key_value[j] == '.' || j == ini_info[i].key_val_len) {

				if (record != '.') {
					memset(str, 0, sizeof(str));
					if (j - index1 > sizeof(str))
						memcpy(str, &ini_info[i].key_value[index1], sizeof(str));
					else
						memcpy(str, &ini_info[i].key_value[index1], (j - index1));
					temp = ili_katoi(str, g_katoi_len);
					data[(count % 4)] = temp;

					/* Over boundary, end to calculate. */
					if ((count / 4) >= frame_len) {
						TS_LOG_ERR("count %d is larger than frame length, break\n", (count / 4));
						break;
					}

					if ((count % 4) == 3) {
						if (data[0] == 1) {
							if (type == VALUE) {
								max_ptr[count/4] = data[1] + data[2];
								min_ptr[count/4] = data[1] - data[3];
							} else {
								max_ptr[count/4] = data[1] + (data[1] * data[2]) / 100;
								min_ptr[count/4] = data[1] - (data[1] * data[3]) / 100;
							}
						} else {
							max_ptr[count/4] = INT_MAX;
							min_ptr[count/4] = INT_MIN;
						}
					}
					count++;
				}
				record = ini_info[i].key_value[j];
				index1 = j + 1;
			}
		}
	}
}

static int parser_get_tdf_value(const char *str, int catalog)
{
	u32 i, ans, index = 0, flag = 0, count = 0, size = 0;
	char s[10] = {0};

	if (!str) {
		TS_LOG_ERR("String is null\n");
		return -1;
	}

	size = strlen(str);
	for (i = 0, count = 0; i < size; i++) {
		if (str[i] == '.') {
			flag = 1;
			continue;
		}
		s[index++] = str[i];
		if (flag)
			count++;
	}
	ans = ili_katoi(s, g_katoi_len);

	/* Multiply by 100 to shift out of decimal point */
	if (catalog == SHORT_TEST) {
		if (count == 0)
			ans = ans * 100;
		else if (count == 1)
			ans = ans * 10;
	}

	return ans;
}

static int parser_get_u8_array(char *key, u8 *buf, u16 base, int len)
{
	char *s = key;
	char *ptoken = NULL;
	int ret, conut = 0;
	long s_to_long = 0;

	if (strlen(s) == 0 || len <= 0) {
		TS_LOG_ERR("Can't find any characters inside buffer\n");
		return -1;
	}

	/*
	 *	@base: The number base to use. The maximum supported base is 16. If base is
	 *	given as 0, then the base of the string is automatically detected with the
	 *	conventional semantics - If it begins with 0x the number will be parsed as a
	 *	hexadecimal (case insensitive), if it otherwise begins with 0, it will be
	 *	parsed as an octal number. Otherwise it will be parsed as a decimal.
	 */
	if (isspace_t((int)(unsigned char)*s) == 0) {
		while ((ptoken = strsep(&s, ",")) != NULL) {
			ret = kstrtol(ptoken, base, &s_to_long);
			if (ret == 0)
				buf[conut] = s_to_long;
			else
				TS_LOG_INFO("convert string too long, ret = %d\n", ret);
			conut++;

			if (conut >= len)
				break;
		}
	}

	return conut;
}

static int parser_get_int_data(const char *section,
	char *keyname, char *rv, int rv_len)
{
	int len = 0;
#ifdef CONFIG_HUAWEI_DEVKIT_QCOM_3_0
	char value[512] = { 0 };
#else
	char value[PARSER_MAX_KEY_VALUE_LEN] = { 0 };
#endif

	if (rv == NULL || section == NULL || keyname == NULL) {
		TS_LOG_ERR("Parameters are invalid\n");
		return -EINVAL;
	}

	/* return a white-space string if get nothing */
	if (parser_get_ini_key_value((const char *)section,
		keyname, value, sizeof(value)) < 0) {
		snprintf(rv, rv_len, "%s", value);
		return 0;
	}

	len = snprintf(rv, rv_len, "%s", value);
	return len;
}

/* Count the number of each line and assign the content to tmp buffer */
static int parser_get_ini_phy_line(const char *data, char *buffer, int maxlen)
{
	int i = 0;
	int j = 0;
	int ret_num = -1;
	char ch1 = '\0';

	for (i = 0, j = 0; i < maxlen; j++) {
		ch1 = data[j];
		ret_num = j + 1;
		if (ch1 == '\n' || ch1 == '\r') {	/* line end */
			ch1 = data[j + 1];
			if (ch1 == '\n' || ch1 == '\r')
				ret_num++;
			break;
		} else if (ch1 == 0x00) {
			break;	/* file end */
		}

		buffer[i++] = ch1;
	}

	buffer[i] = '\0';
	return ret_num;
}

static char *parser_ini_str_trim_r(char *buf)
{
	int len, i;
	char *tmp = NULL;
	char x[512] = {0};
	char *y = NULL;
	char *empty = "";

	len = strlen(buf);

	if (len < sizeof(x)) {
		tmp = x;
		goto copy;
	}

	y = kzalloc(len, GFP_KERNEL);
	if (err_alloc_mem(y)) {
		TS_LOG_ERR("Failed to allocate tmp buf\n");
		return empty;
	}
	tmp = y;

copy:
	for (i = 0; i < len; i++) {
		if (buf[i] != ' ')
			break;
	}

	if (i < len)
		strncpy(tmp, (buf + i), (len - i));

	strncpy(buf, tmp, len);
	ipio_kfree((void **)&y);
	return buf;
}

static int parser_get_ini_phy_data(char *data, int fsize)
{
	int i, n = 0, ret = 0, banchmark_flag = 0, empty_section, nodetype_flag = 0;
	int offset = 0, equal_sign = 0, scount = 0;
	char *ini_buf = NULL, *tmp_sec_name = NULL;
	char m_cfg_ssl = '[';
	char m_cfg_ssr = ']';
	char m_cfg_nts = '#';
	char m_cfg_eqs = '=';

	if (data == NULL) {
		TS_LOG_ERR("INI data is NULL\n");
		ret = -EINVAL;
		goto out;
	}

	ini_buf = kzalloc((PARSER_MAX_CFG_BUF + 1) * sizeof(char), GFP_KERNEL);
	if (err_alloc_mem(ini_buf)) {
		TS_LOG_ERR("Failed to allocate ini_buf memory, %ld\n", PTR_ERR(ini_buf));
		ret = -ENOMEM;
		goto out;
	}

	tmp_sec_name = kzalloc((PARSER_MAX_CFG_BUF + 1) * sizeof(char), GFP_KERNEL);
	if (err_alloc_mem(tmp_sec_name)) {
		TS_LOG_ERR("Failed to allocate tmp_sec_name memory, %ld\n", PTR_ERR(tmp_sec_name));
		ret = -ENOMEM;
		goto out;
	}

	memset(seq_item, 0, MP_TEST_ITEM * PARSER_MAX_KEY_NAME_LEN * sizeof(char));

	while (true) {
		banchmark_flag = 0;
		empty_section = 0;
		nodetype_flag = 0;
		if (g_ini_items > PARSER_MAX_KEY_NUM) {
			TS_LOG_ERR("MAX_KEY_NUM: Out of length\n");
			goto out;
		}

		if (offset >= fsize)
			goto out;/*over size*/

		n = parser_get_ini_phy_line((const char *)(data + offset),
			ini_buf, PARSER_MAX_CFG_BUF);
		if (n < 0) {
			TS_LOG_ERR("End of Line\n");
			goto out;
		}

		offset += n;

		n = strlen(parser_ini_str_trim_r(ini_buf));

		if (n == 0 || ini_buf[0] == m_cfg_nts)
			continue;

		/* Get section names */
		if (n > 2 && ((ini_buf[0] == m_cfg_ssl && ini_buf[n - 1] != m_cfg_ssr))) {
			TS_LOG_ERR("Bad Section: %s\n", ini_buf);
			ret = -EINVAL;
			goto out;
		} else {
			if (ini_buf[0] == m_cfg_ssl) {
				ini_info[g_ini_items].section_len = n - 2;
				if (ini_info[g_ini_items].section_len > PARSER_MAX_KEY_NAME_LEN) {
					TS_LOG_ERR("MAX_KEY_NAME_LEN: Out Of Length\n");
					ret = INI_ERR_OUT_OF_LINE;
					goto out;
				}

				if (scount > MAX_SECTION_NUM) {
					TS_LOG_ERR("seq_item is over than its define %d, abort\n", scount);
					ret = INI_ERR_OUT_OF_LINE;
					goto out;
				}

				ini_buf[n - 1] = 0x00;
				strncpy((char *)tmp_sec_name, ini_buf + 1, (PARSER_MAX_CFG_BUF + 1) * sizeof(char));
				banchmark_flag = 0;
				nodetype_flag = 0;
				strncpy(seq_item[scount], tmp_sec_name, PARSER_MAX_KEY_NAME_LEN);
				scount++;
				TS_LOG_DEBUG("Section Name: %s, Len: %d, offset = %d\n", seq_item[scount], n - 2, offset);
				continue;
			}
		}

		/* copy section's name without square brackets to its real buffer */
		strncpy(ini_info[g_ini_items].section_name,
			tmp_sec_name, ((PARSER_MAX_KEY_NAME_LEN - 1) * sizeof(char)));
		ini_info[g_ini_items].section_len = strlen(tmp_sec_name);

		equal_sign = 0;
		for (i = 0; i < n; i++) {
			if (ini_buf[i] == m_cfg_eqs) {
				equal_sign = i;
				break;
			}
			if (ini_buf[i] == m_cfg_ssl || ini_buf[i] == m_cfg_ssr) {
				empty_section = 1;
				break;
			}
		}

		if (equal_sign == 0) {
			if (empty_section)
				continue;

			if (strstr(ini_info[g_ini_items].section_name, BENCHMARK_KEY_NAME) != NULL) {
				equal_sign = -1;
				ini_info[g_ini_items].key_name_len = strlen(ini_info[g_ini_items].section_name);
				strncpy(ini_info[g_ini_items].key_name,
					ini_info[g_ini_items].section_name, ((PARSER_MAX_KEY_NAME_LEN - 1) * sizeof(char)));
				ini_info[g_ini_items].key_val_len = n;
			} else if (strstr(ini_info[g_ini_items].section_name, NODE_TYPE_KEY_NAME) != NULL) {
				equal_sign = -1;
				ini_info[g_ini_items].key_name_len = strlen(NODE_TYPE_KEY_NAME);
				strncpy(ini_info[g_ini_items].key_name,
					NODE_TYPE_KEY_NAME, ((PARSER_MAX_KEY_NAME_LEN - 1) * sizeof(char)));
				ini_info[g_ini_items].key_val_len = n;
			} else {
				continue;
			}
		} else {
			ini_info[g_ini_items].key_name_len = equal_sign;
			if (ini_info[g_ini_items].key_name_len > PARSER_MAX_KEY_NAME_LEN) {
				/* ret = CFG_ERR_OUT_OF_LEN; */
				TS_LOG_ERR("MAX_KEY_NAME_LEN: Out Of Length\n");
				ret = INI_ERR_OUT_OF_LINE;
				goto out;
			}
			memcpy(ini_info[g_ini_items].key_name, ini_buf,
				ini_info[g_ini_items].key_name_len);
			ini_info[g_ini_items].key_val_len = n - equal_sign - 1;
		}

		if (ini_info[g_ini_items].key_val_len > PARSER_MAX_KEY_VALUE_LEN) {
			TS_LOG_ERR("MAX_KEY_VALUE_LEN: Out Of Length\n");
			ret = INI_ERR_OUT_OF_LINE;
			goto out;
		}
		memcpy(ini_info[g_ini_items].key_value,
			   ini_buf + equal_sign + 1, ini_info[g_ini_items].key_val_len);

		TS_LOG_DEBUG("%s = %s\n", ini_info[g_ini_items].key_name,
			ini_info[g_ini_items].key_value);

		g_ini_items++;
	}
out:
	ipio_kfree((void **)&ini_buf);
	ipio_kfree((void **)&tmp_sec_name);
	return ret;
}

static int ilitek_tddi_mp_ini_parser(const char *path)
{
	int i, ret = 0, fsize = 0;
	char *tmp = NULL;
	const struct firmware *ini = NULL;
	struct file *f = NULL;
	mm_segment_t old_fs;
	loff_t pos = 0;

	TS_LOG_INFO("ini file path = %s\n", path);

	f = filp_open(path, O_RDONLY, 644);
	if (err_alloc_mem(f)) {
		TS_LOG_ERR("Failed to open ini file at %ld, try to request_firmware\n", PTR_ERR(f));
		f = NULL;
		path = ilits->md_ini_rq_path;
		TS_LOG_INFO("request path = %s\n", path);
		if (request_firmware(&ini, path, ilits->dev) < 0) {
			TS_LOG_ERR("Request ini file failed\n");
			return -EINVAL;
		}
		if (ini)
			fsize = ini->size;
	} else {
		fsize = f->f_inode->i_size;
	}

	TS_LOG_INFO("ini file size = %d\n", fsize);
	if (fsize <= 0) {
		TS_LOG_ERR("The size of file is invaild\n");
		ret = -EINVAL;
		goto out;
	}

	tmp = vmalloc(fsize+1);
	if (err_alloc_mem(tmp)) {
		TS_LOG_ERR("Failed to allocate tmp memory, %ld\n", PTR_ERR(tmp));
		ret = -ENOMEM;
		goto out;
	}

	if (f != NULL) {
		old_fs = get_fs();
		set_fs(get_ds());
		vfs_read(f, tmp, fsize, &pos);
		set_fs(old_fs);
		tmp[fsize] = 0x0;
	} else {
		memcpy(tmp, ini->data, fsize);
	}

	g_ini_items = 0;

	/* Initialise ini strcture */
	for (i = 0; i < PARSER_MAX_KEY_NUM; i++) {
		memset(ini_info[i].section_name, 0, PARSER_MAX_KEY_NAME_LEN);
		memset(ini_info[i].key_name, 0, PARSER_MAX_KEY_NAME_LEN);
		memset(ini_info[i].key_value, 0, PARSER_MAX_KEY_VALUE_LEN);
		ini_info[i].section_len = 0;
		ini_info[i].key_name_len = 0;
		ini_info[i].key_val_len = 0;
	}

	/* change all characters to lower case */
	for (i = 0; i < fsize; i++)
		tmp[i] = tolower(tmp[i]);

	ret = parser_get_ini_phy_data(tmp, fsize);
	if (ret < 0) {
		TS_LOG_ERR("Failed to get physical ini data, ret = %d\n", ret);
		goto out;
	}

	TS_LOG_INFO("Parsed ini file done\n");
out:
	ipio_vfree((void **)&tmp);

	if (f != NULL)
		filp_close(f, NULL);
	else
		release_firmware(ini);

	return ret;
}

static void run_pixel_test(int index)
{
	int i, x, y;
	s32 *p_comb = frame_buf;

	for (y = 0; y < core_mp.ych_len; y++) {
		for (x = 0; x < core_mp.xch_len; x++) {
			int tmp[4] = { 0 }, max = 0;
			int shift = y * core_mp.xch_len;
			int centre = p_comb[shift + x];

			/*
			 * if its position is in corner, the number of point
			 * we have to minus is around 2 to 3.
			 */
			if (y == 0 && x == 0) {
				tmp[0] = mathabs(centre - p_comb[(shift + 1) + x]);	/* down */
				tmp[1] = mathabs(centre - p_comb[shift + (x + 1)]);	/* right */
			} else if (y == (core_mp.ych_len - 1) && x == 0) {
				tmp[0] = mathabs(centre - p_comb[(shift - 1) + x]);	/* up */
				tmp[1] = mathabs(centre - p_comb[shift + (x + 1)]);	/* right */
			} else if (y == 0 && x == (core_mp.xch_len - 1)) {
				tmp[0] = mathabs(centre - p_comb[(shift + 1) + x]);	/* down */
				tmp[1] = mathabs(centre - p_comb[shift + (x - 1)]);	/* left */
			} else if (y == (core_mp.ych_len - 1) && x == (core_mp.xch_len - 1)) {
				tmp[0] = mathabs(centre - p_comb[(shift - 1) + x]);	/* up */
				tmp[1] = mathabs(centre - p_comb[shift + (x - 1)]);	/* left */
			} else if (y == 0 && x != 0) {
				tmp[0] = mathabs(centre - p_comb[(shift + 1) + x]);	/* down */
				tmp[1] = mathabs(centre - p_comb[shift + (x - 1)]);	/* left */
				tmp[2] = mathabs(centre - p_comb[shift + (x + 1)]);	/* right */
			} else if (y != 0 && x == 0) {
				tmp[0] = mathabs(centre - p_comb[(shift - 1) + x]);	/* up */
				tmp[1] = mathabs(centre - p_comb[shift + (x + 1)]);	/* right */
				tmp[2] = mathabs(centre - p_comb[(shift + 1) + x]);	/* down */

			} else if (y == (core_mp.ych_len - 1) && x != 0) {
				tmp[0] = mathabs(centre - p_comb[(shift - 1) + x]);	/* up */
				tmp[1] = mathabs(centre - p_comb[shift + (x - 1)]);	/* left */
				tmp[2] = mathabs(centre - p_comb[shift + (x + 1)]);	/* right */
			} else if (y != 0 && x == (core_mp.xch_len - 1)) {
				tmp[0] = mathabs(centre - p_comb[(shift - 1) + x]);	/* up */
				tmp[1] = mathabs(centre - p_comb[shift + (x - 1)]);	/* left */
				tmp[2] = mathabs(centre - p_comb[(shift + 1) + x]);	/* down */
			} else {
				/* middle minus four directions */
				tmp[0] = mathabs(centre - p_comb[(shift - 1) + x]);	/* up */
				tmp[1] = mathabs(centre - p_comb[(shift + 1) + x]);	/* down */
				tmp[2] = mathabs(centre - p_comb[shift + (x - 1)]);	/* left */
				tmp[3] = mathabs(centre - p_comb[shift + (x + 1)]);	/* right */
			}

			max = tmp[0];

			for (i = 0; i < 4; i++) {
				if (tmp[i] > max)
					max = tmp[i];
			}

			titems[index].buf[shift + x] = max;
		}
	}
}

static void run_untouch_p2p_test(int index)
{
	int x, y;
	s32 *p_comb = frame_buf;

	for (y = 0; y < core_mp.ych_len; y++) {
		for (x = 0; x < core_mp.xch_len; x++) {
			int shift = y * core_mp.xch_len;

			if (p_comb[shift + x] > titems[index].max_buf[shift + x])
				titems[index].max_buf[shift + x] = p_comb[shift + x];

			if (p_comb[shift + x] < titems[index].min_buf[shift + x])
				titems[index].min_buf[shift + x] = p_comb[shift + x];

			titems[index].buf[shift + x] =
				titems[index].max_buf[shift + x] - titems[index].min_buf[shift + x];
		}
	}
}

static int run_open_test(int index)
{
	int i, x, y, k, ret = 0;
	int border_x[] = {-1, 0, 1, 1, 1, 0, -1, -1};
	int border_y[] = {-1, -1, -1, 0, 1, 1, 1, 0};
	s32 *p_comb = frame_buf;

	if (ipio_strcmp(titems[index].desp, "open test(integration)") == 0) {
		for (i = 0; i < core_mp.frame_len; i++)
			titems[index].buf[i] = p_comb[i];
	} else if (ipio_strcmp(titems[index].desp, "open test(cap)") == 0) {
		/*
		 * Each result is getting from a 3 by 3 grid depending on where the centre location is.
		 * So if the centre is at corner, the number of node grabbed from a grid will be different.
		 */
		for (y = 0; y < core_mp.ych_len; y++) {
			for (x = 0; x < core_mp.xch_len; x++) {
				int sum = 0, avg = 0, count = 0;
				int shift = y * core_mp.xch_len;
				int centre = p_comb[shift + x];

				for (k = 0; k < 8; k++) {
					if (((y + border_y[k] >= 0) && (y + border_y[k] < core_mp.ych_len)) &&
								((x + border_x[k] >= 0) && (x + border_x[k] < core_mp.xch_len))) {
						count++;
						sum += p_comb[(y + border_y[k]) * core_mp.xch_len + (x + border_x[k])];
					}
				}

				avg = (sum + centre) / (count + 1);	/* plus 1 because of centre */
				titems[index].buf[shift + x] = (centre * 100) / avg;
			}
		}
	}
	return ret;
}

static void run_tx_rx_delta_test(int index)
{
	int x, y;
	s32 *p_comb = frame_buf;

	for (y = 0; y < core_mp.ych_len; y++) {
		for (x = 0; x < core_mp.xch_len; x++) {
			int shift = y * core_mp.xch_len;

			/* Tx Delta */
			if (y != (core_mp.ych_len - 1))
				core_mp.tx_delta_buf[shift + x] = mathabs(p_comb[shift + x] - p_comb[(shift + 1) + x]);

			/* Rx Delta */
			if (x != (core_mp.xch_len - 1))
				core_mp.rx_delta_buf[shift + x] = mathabs(p_comb[shift + x] - p_comb[shift + (x + 1)]);
		}
	}
}

static char *get_date_time_str(void)
{
	struct timespec now_time;
	struct rtc_time rtc_now_time;
	static char time_data_buf[128] = { 0 };

	now_time.tv_sec = 0;
	getnstimeofday(&now_time);
	rtc_time_to_tm(now_time.tv_sec, &rtc_now_time);
	snprintf(time_data_buf, sizeof(time_data_buf), "%04d%02d%02d-%02d%02d%02d",
		(rtc_now_time.tm_year + 1900), rtc_now_time.tm_mon + 1,
		rtc_now_time.tm_mday, rtc_now_time.tm_hour, rtc_now_time.tm_min,
		rtc_now_time.tm_sec);

	return time_data_buf;
}

static void mp_print_csv_header(char *csv, int *csv_len, int *csv_line, int file_size)
{
	int i, seq, tmp_len = *csv_len, tmp_line = *csv_line;

	/* header must has 19 line*/
	tmp_len += snprintf(csv + tmp_len, (file_size - tmp_len), "==============================================================================\n");
	tmp_line++;
	tmp_len += snprintf(csv + tmp_len, (file_size - tmp_len), "ILITek C-TP Utility V%s	%x : Driver Sensor Test\n", DRIVER_VERSION, core_mp.chip_pid);
	tmp_line++;
	tmp_len += snprintf(csv + tmp_len, (file_size - tmp_len), "Confidentiality Notice:\n");
	tmp_line++;
	tmp_len += snprintf(csv + tmp_len, (file_size - tmp_len), "Any information of this tool is confidential and privileged.\n");
	tmp_line++;
	tmp_len += snprintf(csv + tmp_len, (file_size - tmp_len), "@ ILI TECHNOLOGY CORP. All Rights Reserved.\n");
	tmp_line++;
	tmp_len += snprintf(csv + tmp_len, (file_size - tmp_len), "==============================================================================\n");
	tmp_line++;
	tmp_len += snprintf(csv + tmp_len, (file_size - tmp_len), "Firmware Version ,0x%x\n", core_mp.fw_ver);
	tmp_line++;
	tmp_len += snprintf(csv + tmp_len, (file_size - tmp_len), "Panel information ,XCH=%d, YCH=%d\n", core_mp.xch_len, core_mp.ych_len);
	tmp_len += snprintf(csv + tmp_len, (file_size - tmp_len), "INI Release Version ,%s\n", core_mp.ini_ver);
	tmp_line++;
	tmp_len += snprintf(csv + tmp_len, (file_size - tmp_len), "INI Release Date ,%s\n", core_mp.ini_date);
	tmp_line++;
	tmp_line++;
	tmp_len += snprintf(csv + tmp_len, (file_size - tmp_len), "Test Item:\n");
	tmp_line++;

	for (seq = 0; seq < g_run_index.count; seq++) {
		i = g_run_index.index[seq];
		tmp_len += snprintf(csv + tmp_len, (file_size - tmp_len), "	  ---%s\n", titems[i].desp);
		tmp_line++;
	}

	while (tmp_line < 19) {
		tmp_len += snprintf(csv + tmp_len, (file_size - tmp_len), "\n");
		tmp_line++;
	}

	tmp_len += snprintf(csv + tmp_len, (file_size - tmp_len), "==============================================================================\n");

	*csv_len = tmp_len;
	*csv_line = tmp_line;
}

static void mp_print_csv_tail(char *csv, int *csv_len, int file_size)
{
	int i, seq, tmp_len = *csv_len;

	tmp_len += snprintf(csv + tmp_len, (file_size - tmp_len), "==============================================================================\n");
	tmp_len += snprintf(csv + tmp_len, (file_size - tmp_len), "Result_Summary\n");

	for (seq = 0; seq < g_run_index.count; seq++) {
		i = g_run_index.index[seq];
		if (titems[i].item_result == MP_DATA_PASS)
			tmp_len += snprintf(csv + tmp_len, (file_size - tmp_len), "	  {%s}	   ,OK\n", titems[i].desp);
		else
			tmp_len += snprintf(csv + tmp_len, (file_size - tmp_len), "	  {%s}	   ,NG\n", titems[i].desp);
	}

	*csv_len = tmp_len;
}

static void mp_print_csv_cdc_cmd(char *csv, int *csv_len, int index, int file_size)
{
	int i, slen = 0, tmp_len = *csv_len, size;
	char str[128] = {0};
	char *open_sp_cmd[] = {"open dac", "open raw1", "open raw2", "open raw3"};
	char *open_c_cmd[] = {"open cap1 dac", "open cap1 raw"};
	char *name = titems[index].desp;

	if (ipio_strcmp(name, "open test(integration)_sp") == 0) {
		size = ARRAY_SIZE(open_sp_cmd);
		for (i = 0; i < size; i++) {
			slen = parser_get_int_data("pv5_4 command", open_sp_cmd[i], str, sizeof(str));
			if (slen < 0)
				TS_LOG_ERR("Failed to get CDC command %s from ini\n", open_sp_cmd[i]);
			else
				tmp_len += snprintf(csv + tmp_len, (file_size - tmp_len), "%s = ,%s\n", open_sp_cmd[i], str);
		}
	} else if (ipio_strcmp(name, "open test_c") == 0) {
		size = ARRAY_SIZE(open_c_cmd);
		for (i = 0; i < size; i++) {
			slen = parser_get_int_data("pv5_4 command", open_c_cmd[i], str, sizeof(str));
			if (slen < 0)
				TS_LOG_ERR("Failed to get CDC command %s from ini\n", open_sp_cmd[i]);
			else
				tmp_len += snprintf(csv + tmp_len, (file_size - tmp_len), "%s = ,%s\n", open_c_cmd[i], str);
		}
	} else {
		slen = parser_get_int_data("pv5_4 command", name, str, sizeof(str));
		if (slen < 0)
			TS_LOG_ERR("Failed to get CDC command %s from ini\n", name);
		else
			tmp_len += snprintf(csv + tmp_len, (file_size - tmp_len), "CDC command = ,%s\n", str);

		/* Print short parameters */
		if (ipio_strcmp(name, "short test") == 0) {
			if (ilits->production_test_formula_compatible)
				tmp_len += snprintf(csv + tmp_len, (file_size - tmp_len),
					"Variation = ,%d\n", short_para.variation);
			else
				tmp_len += snprintf(csv + tmp_len, (file_size - tmp_len),
					"Variation = ,%d\n", core_mp.short_varia);
		}

		/* Print td second command */
		if (core_mp.td_retry && (ipio_strcmp(name, "peak to peak_td (lcm off)") == 0)) {
			name = "peak to peak_td (lcm off)_2";
			parser_get_int_data("pv5_4 command", name, str, sizeof(str));
			tmp_len += snprintf(csv + tmp_len, (file_size - tmp_len), "CDC command 2 = ,%s\n", str);
		}
	}
	*csv_len = tmp_len;
}

static void mp_compare_cdc_show_result(int index, s32 *tmp, char *csv,
				int *csv_len, int type, s32 *max_ts,
				s32 *min_ts, const char *desp, int file_zise)
{
	int x, y, tmp_len = *csv_len;
	int mp_result = MP_DATA_PASS;

	if (err_alloc_mem(tmp)) {
		TS_LOG_ERR("The data of test item is null (%p)\n", tmp);
		mp_result = -EMP_INVAL;
		goto out;
	}

	/* print X raw only */
	for (x = 0; x < core_mp.xch_len; x++) {
		if (x == 0) {
			dump("\n %s ", desp);
			tmp_len += snprintf(csv + tmp_len, (file_zise - tmp_len), "\n	   %s ,", desp);
		}
		dump("  X_%d	,", (x+1));
		tmp_len += snprintf(csv + tmp_len, (file_zise - tmp_len), "	 X_%d  ,", (x+1));
	}

	dump("\n");
	tmp_len += snprintf(csv + tmp_len, (file_zise - tmp_len), "\n");

	for (y = 0; y < core_mp.ych_len; y++) {
		dump("  Y_%d	,", (y+1));
		tmp_len += snprintf(csv + tmp_len, (file_zise - tmp_len), "	 Y_%d  ,", (y+1));

		for (x = 0; x < core_mp.xch_len; x++) {
			int shift = y * core_mp.xch_len + x;

			/* In Short teset, we only identify if its value is low than min threshold. */
			if (titems[index].catalog == SHORT_TEST) {
				if (tmp[shift] < min_ts[shift]) {
					dump(" #%7d ", tmp[shift]);
					tmp_len += snprintf(csv + tmp_len, (file_zise - tmp_len), "#%7d,", tmp[shift]);
					mp_result = MP_DATA_FAIL;
				} else {
					dump(" %7d ", tmp[shift]);
					tmp_len += snprintf(csv + tmp_len, (file_zise - tmp_len), " %7d, ", tmp[shift]);
				}
				continue;
			}

			if ((tmp[shift] <= max_ts[shift] && tmp[shift] >= min_ts[shift]) || (type != TYPE_JUGE)) {
				if ((tmp[shift] == INT_MAX || tmp[shift] == INT_MIN) && (type == TYPE_BENCHMARK)) {
					dump("%s", "BYPASS,");
					tmp_len += snprintf(csv + tmp_len, (file_zise - tmp_len), "BYPASS,");
				} else {
					dump(" %7d ", tmp[shift]);
					tmp_len += snprintf(csv + tmp_len, (file_zise - tmp_len), " %7d, ", tmp[shift]);
				}
			} else {
				if (tmp[shift] > max_ts[shift]) {
					dump(" *%7d ", tmp[shift]);
					tmp_len += snprintf(csv + tmp_len, (file_zise - tmp_len), "*%7d,", tmp[shift]);
				} else {
					dump(" #%7d ", tmp[shift]);
					tmp_len += snprintf(csv + tmp_len, (file_zise - tmp_len), "#%7d,", tmp[shift]);
				}
				mp_result = MP_DATA_FAIL;
			}
		}
		dump("\n");
		tmp_len += snprintf(csv + tmp_len, (file_zise - tmp_len), "\n");
	}

out:
	if (type == TYPE_JUGE) {
		if (mp_result == MP_DATA_PASS) {
			TS_LOG_INFO("\n Result : PASS\n");
			tmp_len += snprintf(csv + tmp_len, (file_zise - tmp_len), "Result : PASS\n");
		} else {
			TS_LOG_INFO("\n Result : FAIL\n");
			tmp_len += snprintf(csv + tmp_len, (file_zise - tmp_len), "Result : FAIL\n");
		}
	}
	*csv_len = tmp_len;
}

#define addr(x, y) ((y * core_mp.xch_len) + (x))

static int compare_charge(s32 *charge_rate, int x, int y, s32 *innodetype,
		int charge_aa, int charge_boarder, int charge_notch)
{
	int openthreadhold, tempy, tempx, ret, k;
	int sx[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
	int sy[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
	u32 intype;

	ret = charge_rate[addr(x, y)];

	/*Setting Threadhold from node type	 */
	if (charge_rate[addr(x, y)] == 0)
		return ret;
	else if ((innodetype[addr(x, y)] & AA_AREA) == AA_AREA)
		openthreadhold = charge_aa;
	else if ((innodetype[addr(x, y)] & BORDER_AREA) == BORDER_AREA)
		openthreadhold = charge_boarder;
	else if ((innodetype[addr(x, y)] & NOTCH) == NOTCH)
		openthreadhold = charge_notch;
	else
		return ret;

	/* compare carge rate with 3*3 node */
	/* by pass => 1.no compare 2.corner 3.SKIP_MICRO 4.full open fail node */
	for (k = 0; k < 8; k++) {
		tempx = x + sx[k];
		tempy = y + sy[k];

		/*out of range */
		if ((tempx < 0) || (tempx >= core_mp.xch_len) || (tempy < 0) || (tempy >= core_mp.ych_len))
			continue;
		intype = (u32)(innodetype[addr(tempx, tempy)]);
		if ((intype == NO_COMPARE) ||
			((intype & ROUND_CORNER) == ROUND_CORNER) ||
			((intype & SKIP_MICRO) == SKIP_MICRO) ||
			charge_rate[addr(tempx, tempy)] == 0)
			continue;

		if ((charge_rate[addr(tempx, tempy)] - charge_rate[addr(x, y)]) > openthreadhold)
			return openthreadhold;
	}
	return ret;
}

static int full_open_rate_compare(s32 *full_open, s32 *cbk, int x, int y, s32 innodetype, int full_open_rate)
{
	int ret = true;
	u32 uintype;

	uintype = (u32)innodetype;

	if ((uintype == NO_COMPARE) || ((uintype & ROUND_CORNER) == ROUND_CORNER))
		return true;

	if (full_open[addr(x, y)] < (cbk[addr(x, y)] * full_open_rate / 100))
		ret = false;

	return ret;
}

static s32 open_sp_formula(int dac, int raw, int tvch, int tvcl)
{
	s32 ret = 0;
	u16 id = core_mp.chip_id;

	if (id == ILI9881_CHIP) {
		ret = ((dac * 10000 * 161 / 100) - (16384 / 2 - raw) * 20000 * 7 / 16384 * 36 / 10) / (tvch - tvcl) / 2;
	} else {
		ret = ((dac * 10000 * 131 / 100) - (16384 / 2 - raw) * 20000 * 7 / 16384 * 36 / 10) / (tvch - tvcl) / 2;
	}
	return ret;
}

static s32 open_c_formula_compatible(int in_cap1_dac, int in_cap1_raw, int accuracy)
{
	s32 in_cap1_value = 0;
	int in_fout_range = 16384;
	int in_fout_range_half = 8192;
	int in_vadc_range = 36;
	int in_vbk = 39;
	int in_cbk_step = open_spec.cbk_step; /* from mp.ini */
	int in_cint = open_spec.cint; /* from mp.ini */
	int in_vdrv = open_spec.tvch - open_spec.tvcl; /* from mp.ini */
	int in_gain = open_spec.gain; /*  from mp.ini */
	int in_magnification = 10;
	int in_part_dac = 0;
	int in_part_raw = 0;
	u16 id = core_mp.chip_id;
	u8 type = core_mp.chip_type;

	if ((in_cbk_step == 0) || (in_cint == 0)) {
		if (id == ILI9881_CHIP) {
			if ((type == ILI_N) || (type == ILI_O)){
				in_cbk_step = 32;
				in_cint = 70;
			}
		} else if (id == ILI9882_CHIP) {
			if (type == ILI_N) {
				in_cbk_step = 42;
				in_cint = 70;
			} else if (type == ILI_H) {
				in_cbk_step = 42;
				in_cint = 69;
			}
		} else if (id == ILI7807_CHIP){
			if (type == ILI_Q) {
				in_cbk_step = 28;
				in_cint= 70;
			} else if (type == ILI_S) {
				in_cbk_step = 38;
				in_cint= 66;
				in_vbk = 42;
				in_vdrv = in_vdrv / 10;
				in_gain = in_gain / 10;

			} else if (type == ILI_V) {
				in_cbk_step = 42;
				in_cint= 69;
			}
		} else if (id == ILI9883_CHIP) {
			if (type == ILI_A) {
				in_cbk_step = 42;
				in_cint = 70;
			}
		}
	}

	in_part_dac = (in_cap1_dac * in_cbk_step * in_vbk / 2);
	in_part_raw = ((in_cap1_raw - in_fout_range_half) * in_vadc_range *
		in_cint * 10 / in_fout_range);

	if (accuracy)
		in_cap1_value = ((in_part_dac + in_part_raw) * 10) / in_vdrv /
			in_magnification / in_gain;
	else
		in_cap1_value = (in_part_dac + in_part_raw) / in_vdrv /
			in_magnification / in_gain;

	return in_cap1_value;

}

static s32 open_c_formula(int dac, int raw, int tvch, int gain, int accuracy)
{
	s32 ret = 0;
	u16 id = core_mp.chip_id, type = core_mp.chip_type;

	if (!tvch || !gain) {
		TS_LOG_ERR("Divisor is zero\n");
		return -EINVAL;
	}
	if (id == ILI9881_CHIP) {
		ret = (dac * 3206 * 39 / 2 + (raw - 8192) * 9 * 7 * 125 * 5 / 256) / tvch / 1000 / gain;
		return ret;
	}

	if (accuracy) {
		if (type == ILI_Q)
			ret = ((dac * 275 * 39 / 2 * 10) + ((raw - 8192) * 9 * (7 * 100) * 25 / 1024)) / tvch / 100 / gain;
		else if(type == ILI_S)
			ret = ((dac * 377 * 42 / 2 * 10) + ((raw - 8192) * 9 * (66 * 10) * 25 / 1024)) / tvch / 100 / gain;
		else
			ret = 9999;
	} else {
		if (type == ILI_Q)
			ret = ((dac * 275 * 39 / 2) + ((raw - 8192) * 36 * (7 * 100) * 10 * 1 / 16384)) / tvch / 100 / gain;
		else if (type == ILI_S)
			ret = ((dac * 377 * 42 / 2) + ((raw - 8192) * 36 * (66 * 10 ) * 10 * 1 / 16384)) / tvch / 100 / gain;
		else
			ret = 9999;
	}

	return ret;
}

static void allnode_open_cdc_result(int index, int *buf, int *dac, const int *raw)
{
	int i;
	char *desp = titems[index].desp;
	char str[32] = {0};
	int accuracy = 0;

	if (ipio_strcmp(desp, "open test(integration)_sp") == 0) {
		for (i = 0; i < core_mp.frame_len; i++)
			buf[i] = open_sp_formula(dac[i], raw[i], open_spec.tvch, open_spec.tvcl);
	} else if (ipio_strcmp(desp, "open test_c") == 0) {
		if (parser_get_int_data(desp, "accuracy", str, sizeof(str)) > 0)
			accuracy = ili_katoi(str, g_katoi_len);

		for (i = 0; i < core_mp.frame_len; i++) {
			if (ilits->production_test_formula_compatible)
				buf[i] = open_c_formula_compatible(dac[i], raw[i], accuracy);
			else
				buf[i] = open_c_formula(dac[i], raw[i],
					open_spec.tvch - open_spec.tvcl,
					open_spec.gain, accuracy);
		}
	}
}

static int code_to_ohm_compatible(s32* ohm, s32* code, u16 *v_tdf, u16 *h_tdf)
{
	u16 id = core_mp.chip_id;
	u8 type = core_mp.chip_type;
	int in_tvch = short_para.tvch;
	int in_tvcl = short_para.tvcl;
	int in_variation = short_para.variation;
	int in_tdf1 = 0;
	int in_tdf2 = 0;
	int in_cint = short_para.cint;
	int in_rinternal = short_para.rinternal;
	s32 temp = 0;
	int j = 0;

	if (core_mp.is_longv) {
		in_tdf1 = *v_tdf;
		in_tdf2 = *(v_tdf + 1);
	} else {
		in_tdf1 = *h_tdf;
		in_tdf2 = *(h_tdf + 1);
	}

	if (in_variation == 0) {
		in_variation = 100;
	}

	if ((in_cint == 0) ||  (in_rinternal == 0)) {
		if (id == ILI9881_CHIP) {
			if ((type == ILI_N) || (type == ILI_O)){
				in_rinternal = 1915;
				in_cint = 70;
			}
		} else if (id == ILI9882_CHIP) {
			if (type == ILI_N) {
				in_rinternal = 2000;
				in_cint = 70;
			} else if (type == ILI_H) {
				in_rinternal = 2000;
				in_cint = 69;
			}
		} else if (id == ILI7807_CHIP){
			if (type == ILI_Q) {
				in_rinternal = 1500;
				in_cint= 70;
			} else if (type == ILI_S) {
				in_rinternal = 1500;
				in_cint= 66;
				in_tvch = in_tvch/10;
				in_tvcl = in_tvcl/10;
			} else if (type == ILI_V) {
				in_rinternal = 2000;
				in_cint= 69;
			}
		}  else if (id == ILI9883_CHIP) {
			if (type == ILI_A) {
				in_rinternal = 2000;
				in_cint = 70;
			}
		}
	}
	for (j = 0; j < core_mp.frame_len; j++) {
		if (code[j] == 0) {
			TS_LOG_ERR("code is invalid\n");
		} else {
			temp = ((in_tvch - in_tvcl) * in_variation *
				(in_tdf1 - in_tdf2) * (1 << 12) / (9 * code[j] * in_cint)) * 1000;
			temp = (temp - in_rinternal) / 1000;
		}
		ohm[j] = temp;
	}

	/* Unit = M Ohm */
	return temp;
}

static int code_to_ohm(s32 *ohm, s32 *code, u16 *v_tdf, int v_len, u16 *h_tdf, int h_len)
{
	int doutdf1 = 0;
	int doutdf2 = 0;
	int doutvch = 24;
	int doutvcl = 8;
	int doucint = 7;
	int douvariation = 100;
	int dourinternal = 1915;
	int ratio = 100;
	int j = 0;
	u16 id = core_mp.chip_id, type = core_mp.chip_type;
	s32 temp = 0;
	char str[128] = {0};

	if (id == ILI7807_CHIP) {
		if (type == ILI_Q)
			dourinternal = 1500;
		if (type == ILI_S) {
			dourinternal = 1500;
			doucint = 66;
			ratio = 1000;
		}
	}
	core_mp.short_varia = douvariation;

	parser_get_int_data("short test", "tvch", str, sizeof(str));
	if (ili_katoi(str, g_katoi_len) != 0)
		doutvch = ili_katoi(str, g_katoi_len);

	parser_get_int_data("short test", "tvcl", str, sizeof(str));
	if (ili_katoi(str, g_katoi_len) != 0)
		doutvcl = ili_katoi(str, g_katoi_len);

	if (core_mp.is_longv) {
		doutdf1 = *v_tdf;
		doutdf2 = *(v_tdf + 1);
	} else {
		doutdf1 = *h_tdf;
		doutdf2 = *(h_tdf + 1);
	}
	TS_LOG_DEBUG("doutvch = %d, doutvcl = %d, douvariation = %d, doutdf1 = %d, doutdf2 = %d, doucint = %d, ratio = %d,dourinternal = %d\n"
	, doutvch, doutvcl, douvariation, doutdf1, doutdf2, doucint, ratio, dourinternal);
	TS_LOG_DEBUG("temp = ((doutvch - doutvcl) * douvariation * (doutdf1 - doutdf2) * (1 << 12) / (9 * code[j] * doucint)) * ratio;\n");
	TS_LOG_DEBUG("temp = (temp - dourinternal) / 1000;\n");

	for (j = 0; j < core_mp.frame_len; j++) {
		if (code[j] == 0) {
			TS_LOG_ERR("code is invalid\n");
		} else {
			temp = ((doutvch - doutvcl) * douvariation * (doutdf1 - doutdf2) * (1 << 12) / (9 * code[j] * doucint)) * ratio;
			temp = (temp - dourinternal) / 1000;
		}
		ohm[j] = temp;
	}

	/* Unit = M Ohm */
	return temp;

}

static int short_test(int index, int frame_index)
{
	u32 pid = core_mp.chip_pid >> 8;
	int j = 0, ret = 0;
	u16 v_tdf[2] = {0};
	u16 h_tdf[2] = {0};
	char str[32] = {0};

	v_tdf[0] = titems[index].v_tdf_1;
	v_tdf[1] = titems[index].v_tdf_2;
	h_tdf[0] = titems[index].h_tdf_1;
	h_tdf[1] = titems[index].h_tdf_2;

	if (ilits->production_test_formula_compatible) {
		parser_get_int_data("short test", "tvch", str, sizeof(str));
		short_para.tvch = ili_katoi(str, g_katoi_len);

		parser_get_int_data("short test", "tvcl", str, sizeof(str));
		short_para.tvcl = ili_katoi(str, g_katoi_len);

		parser_get_int_data("short test", "variation", str, sizeof(str));
		short_para.variation = ili_katoi(str, g_katoi_len);

		parser_get_int_data("short test", "cint", str, sizeof(str));
		short_para.cint = ili_katoi(str, g_katoi_len);

		parser_get_int_data("short test", "rinternal", str, sizeof(str));
		short_para.rinternal = ili_katoi(str, g_katoi_len);

		if ((pid != ILI9881N_CHIP) && (pid != ILI9881O_CHIP) &&
			(pid != ILI7807Q_CHIP) && (pid != ILI7807S_CHIP)) {
			if ((short_para.cint == 0) || (short_para.rinternal == 0)) {
				TS_LOG_ERR("Failed to get short parameter");
				core_mp.lost_parameter = true;
				return -1;
			}
		}

		TS_LOG_INFO("TVCH = %d, TVCL = %d, Variation = %d, V_TDF1 = %d, V_TDF2 = %d, \
			H_TDF1 = %d, H_TDF2 = %d, Cint = %d, Rinternal = %d\n",
			short_para.tvch, short_para.tvcl, short_para.variation,
			titems[index].v_tdf_1, titems[index].v_tdf_2, titems[index].h_tdf_1,
			titems[index].h_tdf_2, short_para.cint, short_para.rinternal);
	}

	if (core_mp.protocol_ver >= PROTOCOL_VER_540) {
		/* Calculate code to ohm and save to titems[index].buf */
		ili_dump_data(frame_buf, 10, core_mp.frame_len, core_mp.xch_len, "Short Raw 1");
		if (ilits->production_test_formula_compatible)
			code_to_ohm_compatible(&titems[index].buf[frame_index * core_mp.frame_len],
				frame_buf, v_tdf, h_tdf);
		else
			code_to_ohm(&titems[index].buf[frame_index * core_mp.frame_len], frame_buf,
				v_tdf, sizeof(v_tdf)/v_tdf[0], h_tdf, sizeof(h_tdf)/h_tdf[0]);

		ili_dump_data(&titems[index].buf[frame_index * core_mp.frame_len], 10, core_mp.frame_len, core_mp.xch_len, "Short Ohm 1");
	} else {
		for (j = 0; j < core_mp.frame_len; j++)
			titems[index].buf[frame_index * core_mp.frame_len + j] = frame_buf[j];
	}

	return ret;
}

static int allnode_key_cdc_data(int index)
{
	int i, ret = 0, len = 0;
	int indacp = 0, indacn = 0;
	u8 cmd[3] = {0};
	u8 *ori = NULL;

	len = core_mp.key_len * 2;

	TS_LOG_DEBUG("Read key's length = %d\n", len);
	TS_LOG_DEBUG("core_mp.key_len = %d\n", core_mp.key_len);

	if (len <= 0) {
		TS_LOG_ERR("Length is invalid\n");
		ret = -1;
		goto out;
	}

	/* CDC init */
	cmd[0] = P5_X_SET_CDC_INIT;
	cmd[1] = titems[index].cmd;
	cmd[2] = 0;

	// /* Allocate a buffer for the original */
	ori = kcalloc(len, sizeof(u8), GFP_KERNEL);
	if (err_alloc_mem(ori)) {
		TS_LOG_ERR("Failed to allocate ori mem (%ld)\n", PTR_ERR(ori));
		ret = -1;
		goto out;
	}
	ret = ilits->wrapper(cmd, sizeof(cmd), ori, len, ON, ON);
	if (ret < 0) {
		TS_LOG_ERR(" fail to get cdc data\n");
		goto out;
	}
	ili_dump_data(ori, 8, len, 0, "Key CDC original");

	if (key_buf == NULL) {
		key_buf = kcalloc(core_mp.key_len, sizeof(s32), GFP_KERNEL);
		if (err_alloc_mem(key_buf)) {
			TS_LOG_ERR("Failed to allocate FrameBuffer mem (%ld)\n", PTR_ERR(key_buf));
			goto out;
		}
	} else {
		memset(key_buf, 0x0, core_mp.key_len);
	}

	/* Convert original data to the physical one in each node */
	for (i = 0; i < core_mp.frame_len; i++) {
		if (titems[index].cmd == CMD_KEY_DAC) {
			/* DAC - P */
			if (((ori[(2 * i) + 1] & 0x80) >> 7) == 1) {
				/* Negative */
				indacp = 0 - (int)(ori[(2 * i) + 1] & 0x7F);
			} else {
				indacp = ori[(2 * i) + 1] & 0x7F;
			}

			/* DAC - N */
			if (((ori[(1 + (2 * i)) + 1] & 0x80) >> 7) == 1) {
				/* Negative */
				indacn = 0 - (int)(ori[(1 + (2 * i)) + 1] & 0x7F);
			} else {
				indacn = ori[(1 + (2 * i)) + 1] & 0x7F;
			}

			key_buf[i] = (indacp + indacn) / 2;
		}
	}
	ili_dump_data(key_buf, 32, core_mp.frame_len, core_mp.xch_len, "Key CDC combined data");

out:
	ipio_kfree((void **)&ori);
	return ret;
}

static int mp_cdc_get_pv5_4_command(u8 *cmd, int len, int index)
{
	int slen = 0;
	char str[128] = {0};
	char *key = titems[index].desp;

	if (core_mp.td_retry && (ipio_strcmp(key, "peak to peak_td (lcm off)") == 0))
		strncpy(key, "peak to peak_td (lcm off)_2", PEAK_TD_LEN);

	slen = parser_get_int_data("pv5_4 command", key, str, sizeof(str));
	if (slen < 0)
		return -1;

	if (parser_get_u8_array(str, cmd, 16, len) < 0)
		return -1;

	return 0;
}

static int mp_cdc_init_cmd_common(u8 *cmd, int len, int index)
{
	int ret = 0;

	if (core_mp.protocol_ver >= PROTOCOL_VER_540) {
		core_mp.cdc_len = 15;
		return mp_cdc_get_pv5_4_command(cmd, len, index);
	}

	cmd[0] = P5_X_SET_CDC_INIT;
	cmd[1] = titems[index].cmd;
	cmd[2] = 0;

	core_mp.cdc_len = 3;

	if (ipio_strcmp(titems[index].desp, "open test(integration)") == 0)
		cmd[2] = 0x2;
	if (ipio_strcmp(titems[index].desp, "open test(cap)") == 0)
		cmd[2] = 0x3;

	if (titems[index].catalog == PEAK_TO_PEAK_TEST) {
		cmd[2] = ((titems[index].frame_count & 0xff00) >> 8);
		cmd[3] = titems[index].frame_count & 0xff;
		cmd[4] = 0;

		core_mp.cdc_len = 5;

		if (ipio_strcmp(titems[index].desp, "noise peak to peak(cut panel)") == 0)
			cmd[4] = 0x1;

		TS_LOG_DEBUG("P2P CMD: %d,%d,%d,%d,%d\n",
				cmd[0], cmd[1], cmd[2], cmd[3], cmd[4]);
	}

	return ret;
}

static int allnode_open_cdc_data(int mode, int *buf)
{
	int i = 0, ret = 0, len = 0;
	int indacp = 0, indacn = 0;
	u8 cmd[15] = {0};
	u8 *ori = NULL;
	s32 tmp;
	char str[128] = {0};
	char *key[] = {"open dac", "open raw1", "open raw2", "open raw3",
			"open cap1 dac", "open cap1 raw"};

	/* Multipling by 2 is due to the 16 bit in each node */
	len = (core_mp.xch_len * core_mp.ych_len * 2) + 2;

	TS_LOG_DEBUG("Read X/Y Channel length = %d, mode = %d\n", len, mode);

	if (len <= 2) {
		TS_LOG_ERR("Length is invalid\n");
		ret = -EMP_INVAL;
		goto out;
	}

	/* CDC init. Read command from ini file */
	if (parser_get_int_data("pv5_4 command", key[mode], str, sizeof(str)) < 0) {
		TS_LOG_ERR("Failed to parse PV54 command, ret = %d\n", ret);
		ret = -EMP_PARSE;
		goto out;
	}

	parser_get_u8_array(str, cmd, 16, sizeof(cmd));

	ili_dump_data(cmd, 8, sizeof(cmd), 0, "Open SP command");

	/* Allocate a buffer for the original */
	ori = kcalloc(len, sizeof(u8), GFP_KERNEL);
	if (err_alloc_mem(ori)) {
		TS_LOG_ERR("Failed to allocate ori, (%ld)\n", PTR_ERR(ori));
		ret = -EMP_NOMEM;
		goto out;
	}

	/* Get original frame(cdc) data */
	if (ilits->wrapper(cmd, core_mp.cdc_len, ori, len, ON, ON) < 0) {
		TS_LOG_ERR("Failed to get cdc data\n");
		ret = -EMP_GET_CDC;
		goto out;
	}

	ili_dump_data(ori, 8, len, 0, "Open SP CDC original");

	/* Convert original data to the physical one in each node */
	for (i = 0; i < core_mp.frame_len; i++) {
		if ((mode == 0) || (mode == 4)) {
			/* DAC - P */
			if (((ori[(2 * i) + 1] & 0x80) >> 7) == 1) {
				/* Negative */
				indacp = 0 - (int)(ori[(2 * i) + 1] & 0x7F);
			} else {
				indacp = ori[(2 * i) + 1] & 0x7F;
			}

			/* DAC - N */
			if (((ori[(1 + (2 * i)) + 1] & 0x80) >> 7) == 1) {
				/* Negative */
				indacn = 0 - (int)(ori[(1 + (2 * i)) + 1] & 0x7F);
			} else {
				indacn = ori[(1 + (2 * i)) + 1] & 0x7F;
			}

			buf[i] = indacp + indacn;
		} else {
			/* H byte + L byte */
			tmp = (ori[(2 * i) + 1] << 8) + ori[(1 + (2 * i)) + 1];
			if ((tmp & 0x8000) == 0x8000)
				buf[i] = tmp - 65536;
			else
				buf[i] = tmp;

		}
	}
	ili_dump_data(buf, 10, core_mp.frame_len,  core_mp.xch_len, "Open SP CDC combined");
out:

	ipio_kfree((void **)&ori);
	return ret;
}

static int allnode_peak_to_peak_cdc_data(int index)
{
	int i, k, ret = 0, len = 0, rd_frame_num = 1, cmd_len = 0;
	u8 cmd[15] = {0};
	u8 *ori = NULL;
	u32 tmp;

	/* Multipling by 2 is due to the 16 bit in each node */
	len = (core_mp.frame_len * 2) + 2;

	TS_LOG_DEBUG("Read X/Y Channel length = %d\n", len);

	if (len <= 2) {
		TS_LOG_ERR("Length is invalid\n");
		ret = -EMP_INVAL;
		goto out;
	}

	memset(cmd, 0xFF, sizeof(cmd));

	/* Allocate a buffer for the original */
	ori = kcalloc(len, sizeof(u8), GFP_KERNEL);
	if (err_alloc_mem(ori)) {
		TS_LOG_ERR("Failed to allocate ori, (%ld)\n", PTR_ERR(ori));
		ret = -EMP_NOMEM;
		goto out;
	}

	if (titems[index].bch_mrk_multi)
		rd_frame_num = titems[index].bch_mrk_frm_num - 1;

	if (frm_buf == NULL) {
		frm_buf = (s32 **)kzalloc(titems[index].bch_mrk_frm_num * sizeof(s32 *), GFP_KERNEL);
		if (err_alloc_mem(frm_buf)) {
			TS_LOG_ERR("Failed to allocate frm_buf mem (%ld)\n", PTR_ERR(frm_buf));
			ret = -EMP_NOMEM;
			goto out;
		}
		for (i = 0 ; i < titems[index].bch_mrk_frm_num ; i++) {
			frm_buf[i] = (s32 *)kzalloc(core_mp.frame_len * sizeof(s32), GFP_KERNEL);
			if (err_alloc_mem(frm_buf)) {
				TS_LOG_ERR("Failed to allocate frm_buf[%d] mem (%ld)\n", i, PTR_ERR(frm_buf));
				ret = -EMP_NOMEM;
				goto out;
			}
		}
	}

	/* CDC init */
	if (mp_cdc_init_cmd_common(cmd, sizeof(cmd), index) < 0) {
		TS_LOG_ERR("Failed to get cdc command\n");
		ret = -EMP_CMD;
		goto out;
	}

	ili_dump_data(cmd, 8, core_mp.cdc_len, 0, "Mutual CDC command");

	for (k = 0; k < rd_frame_num; k++) {
		if (k == 0)
			cmd_len = core_mp.cdc_len;
		else
			cmd_len = 0;
		memset(ori, 0, len);
		/* Get original frame(cdc) data */
		if (ilits->wrapper(cmd, cmd_len, ori, len, ON, ON) < 0) {
			TS_LOG_ERR("Failed to get cdc data\n");
			ret = -EMP_GET_CDC;
			goto out;
		}

		ili_dump_data(ori, 8, len, 0, "Mutual CDC original");

		/* Convert original data to the physical one in each node */
		for (i = 0; i < core_mp.frame_len; i++) {
			/* H byte + L byte */
			tmp = (ori[(2 * i) + 1] << 8) + ori[(1 + (2 * i)) + 1];

			if ((tmp & 0x8000) == 0x8000)
				frm_buf[k][i] = tmp - 65536;
			else
				frm_buf[k][i] = tmp;

			// multiple case frame3 = frame1 - frame2
			if (titems[index].bch_mrk_multi && rd_frame_num == k + 1)
				frm_buf[k+1][i] = frm_buf[k-1][i] - frm_buf[k][i];
		}
	}

	if (titems[index].bch_mrk_multi) {
		ili_dump_data(frm_buf[0], 32, core_mp.frame_len, core_mp.xch_len, "Mutual CDC combined[0]/frame1");
		ili_dump_data(frm_buf[1], 32, core_mp.frame_len, core_mp.xch_len, "Mutual CDC combined[1]/frame2");
		ili_dump_data(frm_buf[2], 32, core_mp.frame_len, core_mp.xch_len, "Mutual CDC combined[2]/frame3");
	} else {
		ili_dump_data(frm_buf[0], 32, core_mp.frame_len, core_mp.xch_len, "Mutual CDC combined[0]/frame1");
	}

out:
	ipio_kfree((void **)&ori);
	return ret;
}

static int allnode_mutual_cdc_data(int index)
{
	int i, ret = 0, len = 0;
	int indacp = 0, indacn = 0;
	u8 cmd[15] = {0};
	u8 *ori = NULL;
	u32 tmp;

	/* Multipling by 2 is due to the 16 bit in each node */
	len = (core_mp.frame_len * 2) + 2;

	TS_LOG_DEBUG("Read X/Y Channel length = %d\n", len);

	if (len <= 2) {
		TS_LOG_ERR("Length is invalid\n");
		ret = -EMP_INVAL;
		goto out;
	}

	memset(cmd, 0xFF, sizeof(cmd));

	/* CDC init */
	if (mp_cdc_init_cmd_common(cmd, sizeof(cmd), index) < 0) {
		TS_LOG_ERR("Failed to get cdc command\n");
		ret = -EMP_CMD;
		goto out;
	}

	ili_dump_data(cmd, 8, core_mp.cdc_len, 0, "Mutual CDC command");

	/* Allocate a buffer for the original */
	ori = kcalloc(len, sizeof(u8), GFP_KERNEL);
	if (err_alloc_mem(ori)) {
		TS_LOG_ERR("Failed to allocate ori, (%ld)\n", PTR_ERR(ori));
		ret = -EMP_NOMEM;
		goto out;
	}

	/* Get original frame(cdc) data */
	if (ilits->wrapper(cmd, core_mp.cdc_len, ori, len, ON, ON) < 0) {
		TS_LOG_ERR("Failed to get cdc data\n");
		ret = -EMP_GET_CDC;
		goto out;
	}

	ili_dump_data(ori, 8, len, 0, "Mutual CDC original");

	if (frame_buf == NULL) {
		frame_buf = kcalloc(core_mp.frame_len, sizeof(s32), GFP_KERNEL);
		if (err_alloc_mem(frame_buf)) {
			TS_LOG_ERR("Failed to allocate FrameBuffer mem (%ld)\n", PTR_ERR(frame_buf));
			ret = -EMP_NOMEM;
			goto out;
		}
	} else {
		memset(frame_buf, 0x0, core_mp.frame_len);
	}

	/* Convert original data to the physical one in each node */
	for (i = 0; i < core_mp.frame_len; i++) {
		if (ipio_strcmp(titems[index].desp, "calibration data(dac)") == 0) {
			/* DAC - P */
			if (((ori[(2 * i) + 1] & 0x80) >> 7) == 1) {
				/* Negative */
				indacp = 0 - (int)(ori[(2 * i) + 1] & 0x7F);
			} else {
				indacp = ori[(2 * i) + 1] & 0x7F;
			}

			/* DAC - N */
			if (((ori[(1 + (2 * i)) + 1] & 0x80) >> 7) == 1) {
				/* Negative */
				indacn = 0 - (int)(ori[(1 + (2 * i)) + 1] & 0x7F);
			} else {
				indacn = ori[(1 + (2 * i)) + 1] & 0x7F;
			}

			frame_buf[i] = (indacp + indacn) / 2;
		} else {
			/* H byte + L byte */
			tmp = (ori[(2 * i) + 1] << 8) + ori[(1 + (2 * i)) + 1];

			if ((tmp & 0x8000) == 0x8000)
				frame_buf[i] = tmp - 65536;
			else
				frame_buf[i] = tmp;

			if (ipio_strcmp(titems[index].desp, "raw data(no bk)") == 0 ||
				ipio_strcmp(titems[index].desp, "raw data(no bk) (lcm off)") == 0)
				frame_buf[i] -= core_mp.no_bk_shift;
		}
	}

	ili_dump_data(frame_buf, 32, core_mp.frame_len, core_mp.xch_len, "Mutual CDC combined");

out:
	ipio_kfree((void **)&ori);
	return ret;
}

static void compare_maxmin_result(int index, s32 *data)
{
	int x, y;

	for (y = 0; y < core_mp.ych_len; y++) {
		for (x = 0; x < core_mp.xch_len; x++) {
			int shift = y * core_mp.xch_len;

			if (titems[index].catalog == UNTOUCH_P2P) {
				return;
			} else if (titems[index].catalog == TX_RX_DELTA) {
				/* Tx max/min comparison */
				if (core_mp.tx_delta_buf[shift + x] < data[shift + x])
					core_mp.tx_max_buf[shift + x] = data[shift + x];

				if (core_mp.tx_delta_buf[shift + x] > data[shift + x])
					core_mp.tx_min_buf[shift + x] = data[shift + x];

				/* Rx max/min comparison */
				if (core_mp.rx_delta_buf[shift + x] < data[shift + x])
					core_mp.rx_max_buf[shift + x] = data[shift + x];

				if (core_mp.rx_delta_buf[shift + x] > data[shift + x])
					core_mp.rx_min_buf[shift + x] = data[shift + x];
			} else {
				if (titems[index].max_buf[shift + x] < data[shift + x])
					titems[index].max_buf[shift + x] = data[shift + x];

				if (titems[index].min_buf[shift + x] > data[shift + x])
					titems[index].min_buf[shift + x] = data[shift + x];
			}
		}
	}
}

static int create_mp_test_frame_buffer(int index, int frame_count)
{
	int i;

	TS_LOG_DEBUG("Create MP frame buffers (index = %d), count = %d\n",
			index, frame_count);

	if (titems[index].catalog == TX_RX_DELTA) {
		if (core_mp.tx_delta_buf == NULL) {
			core_mp.tx_delta_buf = kcalloc(core_mp.frame_len, sizeof(s32), GFP_KERNEL);
			if (err_alloc_mem(core_mp.tx_delta_buf)) {
				TS_LOG_ERR("Failed to allocate tx_delta_buf mem\n");
				ipio_kfree((void **)&core_mp.tx_delta_buf);
				return -ENOMEM;
			}
		}

		if (core_mp.rx_delta_buf == NULL) {
			core_mp.rx_delta_buf = kcalloc(core_mp.frame_len, sizeof(s32), GFP_KERNEL);
			if (err_alloc_mem(core_mp.rx_delta_buf)) {
				TS_LOG_ERR("Failed to allocate rx_delta_buf mem\n");
				ipio_kfree((void **)&core_mp.rx_delta_buf);
				return -ENOMEM;
			}
		}

		if (core_mp.tx_max_buf == NULL) {
			core_mp.tx_max_buf = kcalloc(core_mp.frame_len, sizeof(s32), GFP_KERNEL);
			if (err_alloc_mem(core_mp.tx_max_buf)) {
				TS_LOG_ERR("Failed to allocate tx_max_buf mem\n");
				ipio_kfree((void **)&core_mp.tx_max_buf);
				return -ENOMEM;
			}
		}

		if (core_mp.tx_min_buf == NULL) {
			core_mp.tx_min_buf = kcalloc(core_mp.frame_len, sizeof(s32), GFP_KERNEL);
			if (err_alloc_mem(core_mp.tx_min_buf)) {
				TS_LOG_ERR("Failed to allocate tx_min_buf mem\n");
				ipio_kfree((void **)&core_mp.tx_min_buf);
				return -ENOMEM;
			}
		}

		if (core_mp.rx_max_buf == NULL) {
			core_mp.rx_max_buf = kcalloc(core_mp.frame_len, sizeof(s32), GFP_KERNEL);
			if (err_alloc_mem(core_mp.rx_max_buf)) {
				TS_LOG_ERR("Failed to allocate rx_max_buf mem\n");
				ipio_kfree((void **)&core_mp.rx_max_buf);
				return -ENOMEM;
			}
		}

		if (core_mp.rx_min_buf == NULL) {
			core_mp.rx_min_buf = kcalloc(core_mp.frame_len, sizeof(s32), GFP_KERNEL);
			if (err_alloc_mem(core_mp.rx_min_buf)) {
				TS_LOG_ERR("Failed to allocate rx_min_buf mem\n");
				ipio_kfree((void **)&core_mp.rx_min_buf);
				return -ENOMEM;
			}
		}
	} else {
		if (titems[index].buf == NULL) {
			titems[index].buf = vmalloc(frame_count * core_mp.frame_len * sizeof(s32));
			if (err_alloc_mem(titems[index].buf)) {
				TS_LOG_ERR("Failed to allocate buf mem\n");
				ipio_kfree((void **)&titems[index].buf);
				return -ENOMEM;
			}
		}

		if (titems[index].result_buf == NULL) {
			titems[index].result_buf = kcalloc(core_mp.frame_len, sizeof(s32), GFP_KERNEL);
			if (err_alloc_mem(titems[index].result_buf)) {
				TS_LOG_ERR("Failed to allocate result_buf mem\n");
				ipio_kfree((void **)&titems[index].result_buf);
				return -ENOMEM;
			}
		}

		if (titems[index].max_buf == NULL) {
			titems[index].max_buf = kcalloc(core_mp.frame_len, sizeof(s32), GFP_KERNEL);
			if (err_alloc_mem(titems[index].max_buf)) {
				TS_LOG_ERR("Failed to allocate max_buf mem\n");
				ipio_kfree((void **)&titems[index].max_buf);
				return -ENOMEM;
			}
		}

		if (titems[index].min_buf == NULL) {
			titems[index].min_buf = kcalloc(core_mp.frame_len, sizeof(s32), GFP_KERNEL);
			if (err_alloc_mem(titems[index].min_buf)) {
				TS_LOG_ERR("Failed to allocate min_buf mem\n");
				ipio_kfree((void **)&titems[index].min_buf);
				return -ENOMEM;
			}
		}

		if (titems[index].spec_option == BENCHMARK) {
			if (titems[index].bch_mrk_max == NULL) {
				titems[index].bch_mrk_max = (s32 **)kzalloc(titems[index].bch_mrk_frm_num * sizeof(s32 *), GFP_KERNEL);
				if (err_alloc_mem(titems[index].bch_mrk_max)) {
					TS_LOG_ERR("Failed to allocate bch_mrk_max mem\n");
					ipio_kfree((void **)&titems[index].bch_mrk_max);
					return -ENOMEM;
				}

				for (i = 0; i < titems[index].bch_mrk_frm_num; i++) {
					titems[index].bch_mrk_max[i] = (s32 *)kzalloc(core_mp.frame_len * sizeof(s32), GFP_KERNEL);
					if (err_alloc_mem(titems[index].bch_mrk_max[i])) {
						TS_LOG_ERR("Failed to allocate bch_mrk_max[%d] mem\n", i);
						ipio_kfree((void **)&titems[index].bch_mrk_max[i]);
						return -ENOMEM;
					}
				}
			}

			if (titems[index].bch_mrk_min == NULL) {
				titems[index].bch_mrk_min = (s32 **)kzalloc(titems[index].bch_mrk_frm_num * sizeof(s32 *), GFP_KERNEL);
				if (err_alloc_mem(titems[index].bch_mrk_min)) {
					TS_LOG_ERR("Failed to allocate bch_mrk_min mem\n");
					ipio_kfree((void **)&titems[index].bch_mrk_min);
					return -ENOMEM;
				}
				for (i = 0; i < titems[index].bch_mrk_frm_num; i++) {
					titems[index].bch_mrk_min[i] = (s32 *)kzalloc(core_mp.frame_len * sizeof(s32), GFP_KERNEL);
					if (err_alloc_mem(titems[index].bch_mrk_min[i])) {
						TS_LOG_ERR("Failed to allocate bch_mrk_min[%d] mem\n", i);
						ipio_kfree((void **)&titems[index].bch_mrk_min[i]);
						return -ENOMEM;
					}
				}
			}

			if (titems[index].bench_mark_max == NULL) {
				titems[index].bench_mark_max = kcalloc(core_mp.frame_len, sizeof(s32), GFP_KERNEL);
				if (err_alloc_mem(titems[index].bench_mark_max)) {
					TS_LOG_ERR("Failed to allocate bench_mark_max mem\n");
					ipio_kfree((void **)&titems[index].bench_mark_max);
					return -ENOMEM;
				}
			}
			if (titems[index].bench_mark_min == NULL) {
				titems[index].bench_mark_min = kcalloc(core_mp.frame_len, sizeof(s32), GFP_KERNEL);
				if (err_alloc_mem(titems[index].bench_mark_min)) {
					TS_LOG_ERR("Failed to allocate bench_mark_min mem\n");
					ipio_kfree((void **)&titems[index].bench_mark_min);
					return -ENOMEM;
				}
			}
		}
	}
	return 0;
}

static int pin_test(int index)
{
	return 0;
}

static int mutual_test(int index)
{
	int i = 0, j = 0, x = 0, y = 0, ret = 0, get_frame_cont = 1;

	TS_LOG_DEBUG("index = %d, desp = %s, Frame Count = %d\n",
		index, titems[index].desp, titems[index].frame_count);

	/*
	 * We assume that users who are calling the test forget to config frame count
	 * as 1, so we just help them to set it up.
	 */
	if (titems[index].frame_count <= 0) {
		TS_LOG_ERR("Frame count is zero, which is at least set as 1\n");
		titems[index].frame_count = 1;
	}

	ret = create_mp_test_frame_buffer(index, titems[index].frame_count);
	if (ret < 0) {
		ret = -EMP_NOMEM;
		goto out;
	}

	/* Init Max/Min buffer */
	for (y = 0; y < core_mp.ych_len; y++) {
		for (x = 0; x < core_mp.xch_len; x++) {
			if (titems[i].catalog == TX_RX_DELTA) {
				core_mp.tx_max_buf[y * core_mp.xch_len + x] = INT_MIN;
				core_mp.rx_max_buf[y * core_mp.xch_len + x] = INT_MIN;
				core_mp.tx_min_buf[y * core_mp.xch_len + x] = INT_MAX;
				core_mp.rx_min_buf[y * core_mp.xch_len + x] = INT_MAX;
			} else {
				titems[index].max_buf[y * core_mp.xch_len + x] = INT_MIN;
				titems[index].min_buf[y * core_mp.xch_len + x] = INT_MAX;
			}
		}
	}

	if (titems[index].catalog != PEAK_TO_PEAK_TEST)
		get_frame_cont = titems[index].frame_count;

	if (titems[index].spec_option == BENCHMARK) {
		parser_ini_benchmark(titems[index].bench_mark_max, titems[index].bench_mark_min,
			titems[index].type_option, titems[index].desp, core_mp.frame_len, BENCHMARK_KEY_NAME);
		dump_benchmark_data(titems[index].bench_mark_max, titems[index].bench_mark_min);
	}

	for (i = 0; i < get_frame_cont; i++) {
		ret = allnode_mutual_cdc_data(index);
		if (ret < 0) {
			TS_LOG_ERR("Failed to initialise CDC data, %d\n", ret);
			goto out;
		}
		switch (titems[index].catalog) {
		case PIXEL:
			run_pixel_test(index);
			break;
		case UNTOUCH_P2P:
			run_untouch_p2p_test(index);
			break;
		case OPEN_TEST:
			run_open_test(index);
			break;
		case TX_RX_DELTA:
			run_tx_rx_delta_test(index);
			break;
		case SHORT_TEST:
			short_test(index, i);
			break;
		default:
			for (j = 0; j < core_mp.frame_len; j++)
				titems[index].buf[i * core_mp.frame_len + j] = frame_buf[j];
			break;
		}
		compare_maxmin_result(index, &titems[index].buf[i * core_mp.frame_len]);
	}

out:
	return ret;
}

static int  peak_to_peak_test(int index)
{
	int i = 0, j = 0, x = 0, y = 0, ret = 0;
	char benchmark_str[128] = {0};

	TS_LOG_DEBUG("index = %d, desp = %s bch_mrk_frm_num = %d\n"
		, index, titems[index].desp, titems[index].bch_mrk_frm_num);

	ret = create_mp_test_frame_buffer(index, titems[index].bch_mrk_frm_num);
	if (ret < 0) {
		ret = -EMP_NOMEM;
		goto out;
	}

	if (titems[index].spec_option == BENCHMARK) {
		for (i = 0; i < titems[index].bch_mrk_frm_num; i++ ) {
			snprintf(benchmark_str, sizeof(benchmark_str), "%s%d", BENCHMARK_KEY_NAME, i + 1);
			parser_ini_benchmark(titems[index].bch_mrk_max[i], titems[index].bch_mrk_min[i],
				titems[index].type_option, titems[index].desp, core_mp.frame_len, benchmark_str);

			dump_benchmark_data(titems[index].bch_mrk_max[i], titems[index].bch_mrk_min[i]);
		}
	}

	/* Init Max/Min buffer */
	for (y = 0; y < core_mp.ych_len; y++) {
		for (x = 0; x < core_mp.xch_len; x++) {
			titems[index].max_buf[y * core_mp.xch_len + x] = INT_MIN;
			titems[index].min_buf[y * core_mp.xch_len + x] = INT_MAX;
		}
	}

	ret = allnode_peak_to_peak_cdc_data(index);
	if (ret < 0) {
		TS_LOG_ERR("Failed to initialise CDC data, %d\n", ret);
		goto out;
	}
	for (i = 0; i < titems[index].bch_mrk_frm_num; i++) {
		for (j = 0; j < core_mp.frame_len; j++)
			titems[index].buf[i * core_mp.frame_len + j] = frm_buf[i][j];

		compare_maxmin_result(index, &titems[index].buf[i * core_mp.frame_len]);
	}
out:
	return ret;
}


static int open_test_sp(int index)
{
	struct mp_test_p540_open *open = NULL;
	int i = 0, x = 0, y = 0, ret = 0, addr = 0;
	int charge_aa = 0, charge_boarder = 0, charge_notch = 0, full_open_rate = 0;
	char str[512] = {0};

	TS_LOG_DEBUG("index = %d, desp = %s, Frame Count = %d\n",
		index, titems[index].desp, titems[index].frame_count);

	/*
	 * We assume that users who are calling the test forget to config frame count
	 * as 1, so we just help them to set it up.
	 */
	if (titems[index].frame_count <= 0) {
		TS_LOG_ERR("Frame count is zero, which is at least set as 1\n");
		titems[index].frame_count = 1;
	}

	open = kzalloc(titems[index].frame_count * sizeof(struct mp_test_p540_open), GFP_KERNEL);
	if (err_alloc_mem(open)) {
		TS_LOG_ERR("Failed to allocate open mem (%ld)\n", PTR_ERR(open));
		ret = -EMP_NOMEM;
		goto out;
	}

	ret = create_mp_test_frame_buffer(index, titems[index].frame_count);
	if (ret < 0) {
		ret = -EMP_NOMEM;
		goto out;
	}

	if (frame1_cbk700 == NULL) {
		frame1_cbk700 = kcalloc(core_mp.frame_len, sizeof(s32), GFP_KERNEL);
		if (err_alloc_mem(frame1_cbk700)) {
			TS_LOG_ERR("Failed to allocate frame1_cbk700 buffer\n");
			ipio_kfree((void **)&open);
			return -EMP_NOMEM;
		}
	} else {
		memset(frame1_cbk700, 0x0, core_mp.frame_len);
	}

	if (frame1_cbk250 == NULL) {
		frame1_cbk250 = kcalloc(core_mp.frame_len, sizeof(s32), GFP_KERNEL);
		if (err_alloc_mem(frame1_cbk250)) {
			TS_LOG_ERR("Failed to allocate frame1_cbk250 buffer\n");
			ipio_kfree((void **)&frame1_cbk700);
			ipio_kfree((void **)&open);
			return -EMP_NOMEM;
		}
	} else {
		memset(frame1_cbk250, 0x0, core_mp.frame_len);
	}

	if (frame1_cbk200 == NULL) {
		frame1_cbk200 = kcalloc(core_mp.frame_len, sizeof(s32), GFP_KERNEL);
		if (err_alloc_mem(frame1_cbk200)) {
			TS_LOG_ERR("Failed to allocate cbk buffer\n");
			ipio_kfree((void **)&frame1_cbk700);
			ipio_kfree((void **)&frame1_cbk250);
			ipio_kfree((void **)&open);
			return -EMP_NOMEM;
		}
	} else {
		memset(frame1_cbk200, 0x0, core_mp.frame_len);
	}

	titems[index].node_type = kcalloc(core_mp.frame_len, sizeof(s32), GFP_KERNEL);
	if (err_alloc_mem(titems[index].node_type)) {
		TS_LOG_ERR("Failed to allocate node_type FRAME buffer\n");
		ipio_kfree((void **)&frame1_cbk700);
		ipio_kfree((void **)&frame1_cbk250);
		ipio_kfree((void **)&frame1_cbk200);
		ipio_kfree((void **)&open);
		return -EMP_NOMEM;
	}

	/* Init Max/Min buffer */
	for (y = 0; y < core_mp.ych_len; y++) {
		for (x = 0; x < core_mp.xch_len; x++) {
			titems[index].max_buf[y * core_mp.xch_len + x] = INT_MIN;
			titems[index].min_buf[y * core_mp.xch_len + x] = INT_MAX;
		}
	}

	if (titems[index].spec_option == BENCHMARK) {
		parser_ini_benchmark(titems[index].bench_mark_max, titems[index].bench_mark_min,
			titems[index].type_option, titems[index].desp, core_mp.frame_len, BENCHMARK_KEY_NAME);
		dump_benchmark_data(titems[index].bench_mark_max, titems[index].bench_mark_min);
	}

	parser_ini_nodetype(titems[index].node_type, NODE_TYPE_KEY_NAME, core_mp.frame_len);
	dump_node_type_buffer(titems[index].node_type, "node type");

	parser_get_int_data(titems[index].desp, "charge_aa", str, sizeof(str));
	charge_aa = ili_katoi(str, g_katoi_len);

	parser_get_int_data(titems[index].desp, "charge_border", str, sizeof(str));
	charge_boarder = ili_katoi(str, g_katoi_len);

	parser_get_int_data(titems[index].desp, "charge_notch", str, sizeof(str));
	charge_notch = ili_katoi(str, g_katoi_len);

	parser_get_int_data(titems[index].desp, "full open", str, sizeof(str));
	full_open_rate = ili_katoi(str, g_katoi_len);

	parser_get_int_data(titems[index].desp, "tvch", str, sizeof(str));
	open_spec.tvch = ili_katoi(str, g_katoi_len);

	parser_get_int_data(titems[index].desp, "tvcl", str, sizeof(str));
	open_spec.tvcl = ili_katoi(str, g_katoi_len);

	TS_LOG_DEBUG("AA = %d, Border = %d, NOTCH = %d, full_open_rate = %d, tvch = %d, tvcl = %d\n",
			charge_aa, charge_boarder, charge_notch,
			full_open_rate, open_spec.tvch, open_spec.tvcl);

	for (i = 0; i < titems[index].frame_count; i++) {
		open[i].tdf_700 = kcalloc(core_mp.frame_len, sizeof(s32), GFP_KERNEL);
		open[i].tdf_250 = kcalloc(core_mp.frame_len, sizeof(s32), GFP_KERNEL);
		open[i].tdf_200 = kcalloc(core_mp.frame_len, sizeof(s32), GFP_KERNEL);
		open[i].cbk_700 = kcalloc(core_mp.frame_len, sizeof(s32), GFP_KERNEL);
		open[i].cbk_250 = kcalloc(core_mp.frame_len, sizeof(s32), GFP_KERNEL);
		open[i].cbk_200 = kcalloc(core_mp.frame_len, sizeof(s32), GFP_KERNEL);
		open[i].charg_rate = kcalloc(core_mp.frame_len, sizeof(s32), GFP_KERNEL);
		open[i].full_open = kcalloc(core_mp.frame_len, sizeof(s32), GFP_KERNEL);
		open[i].dac = kcalloc(core_mp.frame_len, sizeof(s32), GFP_KERNEL);
	}

	for (i = 0; i < titems[index].frame_count; i++) {
		ret = allnode_open_cdc_data(0, open[i].dac);
		if (ret < 0) {
			TS_LOG_ERR("Failed to get Open SP DAC data, %d\n", ret);
			goto out;
		}
		ret = allnode_open_cdc_data(1, open[i].tdf_700);
		if (ret < 0) {
			TS_LOG_ERR("Failed to get Open SP Raw1 data, %d\n", ret);
			goto out;
		}
		ret = allnode_open_cdc_data(2, open[i].tdf_250);
		if (ret < 0) {
			TS_LOG_ERR("Failed to get Open SP Raw2 data, %d\n", ret);
			goto out;
		}
		ret = allnode_open_cdc_data(3, open[i].tdf_200);
		if (ret < 0) {
			TS_LOG_ERR("Failed to get Open SP Raw3 data, %d\n", ret);
			goto out;
		}
		allnode_open_cdc_result(index,
			open[i].cbk_700, open[i].dac, (const int *)open[i].tdf_700);
		allnode_open_cdc_result(index,
			open[i].cbk_250, open[i].dac, (const int *)open[i].tdf_250);
		allnode_open_cdc_result(index,
			open[i].cbk_200, open[i].dac, (const int *)open[i].tdf_200);

		addr = 0;

		/* record fist frame for debug */
		if (i == 0) {
			memcpy(frame1_cbk700, open[i].cbk_700, core_mp.frame_len * sizeof(s32));
			memcpy(frame1_cbk250, open[i].cbk_250, core_mp.frame_len * sizeof(s32));
			memcpy(frame1_cbk200, open[i].cbk_200, core_mp.frame_len * sizeof(s32));
		}

		ili_dump_data(open[i].cbk_700, 10, core_mp.frame_len, core_mp.xch_len, "cbk 700");
		ili_dump_data(open[i].cbk_250, 10, core_mp.frame_len, core_mp.xch_len, "cbk 250");
		ili_dump_data(open[i].cbk_200, 10, core_mp.frame_len, core_mp.xch_len, "cbk 200");

		for (y = 0; y < core_mp.ych_len; y++) {
			for (x = 0; x < core_mp.xch_len; x++) {
				open[i].charg_rate[addr] = open[i].cbk_250[addr] * 100 / open[i].cbk_700[addr];
				open[i].full_open[addr] = open[i].cbk_700[addr] - open[i].cbk_200[addr];
				addr++;
			}
		}

		ili_dump_data(open[i].charg_rate, 10, core_mp.frame_len, core_mp.xch_len, "origin charge rate");
		ili_dump_data(open[i].full_open, 10, core_mp.frame_len, core_mp.xch_len, "origin full open");

		addr = 0;
		for (y = 0; y < core_mp.ych_len; y++) {
			for (x = 0; x < core_mp.xch_len; x++) {
				if (full_open_rate_compare(open[i].full_open, open[i].cbk_700, x, y, titems[index].node_type[addr], full_open_rate) == false) {
					titems[index].buf[(i * core_mp.frame_len) + addr] = 0;
					open[i].charg_rate[addr] = 0;
				}
				addr++;
			}
		}

		ili_dump_data(&titems[index].buf[(i * core_mp.frame_len)], 10, core_mp.frame_len, core_mp.xch_len, "after full_open_rate_compare");

		addr = 0;
		for (y = 0; y < core_mp.ych_len; y++) {
			for (x = 0; x < core_mp.xch_len; x++) {
				titems[index].buf[(i * core_mp.frame_len) + addr] = compare_charge(open[i].charg_rate, x, y, titems[index].node_type, charge_aa, charge_boarder, charge_notch);
				addr++;
			}
		}

		ili_dump_data(&titems[index].buf[(i * core_mp.frame_len)], 10, core_mp.frame_len, core_mp.xch_len, "after compare charge rate");

		compare_maxmin_result(index, &titems[index].buf[(i * core_mp.frame_len)]);
	}

out:
	ipio_kfree((void **)&titems[index].node_type);

	if (open != NULL) {
		for (i = 0; i < titems[index].frame_count; i++) {
			ipio_kfree((void **)&open[i].tdf_700);
			ipio_kfree((void **)&open[i].tdf_250);
			ipio_kfree((void **)&open[i].tdf_200);
			ipio_kfree((void **)&open[i].cbk_700);
			ipio_kfree((void **)&open[i].cbk_250);
			ipio_kfree((void **)&open[i].cbk_200);
			ipio_kfree((void **)&open[i].charg_rate);
			ipio_kfree((void **)&open[i].full_open);
			ipio_kfree((void **)&open[i].dac);
		}
		kfree(open);
		open = NULL;
	}

	return ret;
}

static int open_test_cap(int index)
{
	u32 pid = core_mp.chip_pid >> 8;
	struct mp_test_open_c *open = NULL;
	int i = 0, x = 0, y = 0, ret = 0, addr = 0;
	char str[512] = {0};

	TS_LOG_DEBUG("index = %d, desp = %s, Frame Count = %d\n",
		index, titems[index].desp, titems[index].frame_count);

	if (ilits->production_test_formula_compatible) {
		parser_get_int_data(titems[index].desp, "cbk_step", str, sizeof(str));
		open_spec.cbk_step = ili_katoi(str, g_katoi_len);

		parser_get_int_data(titems[index].desp, "cint", str, sizeof(str));
		open_spec.cint = ili_katoi(str, g_katoi_len);

		/* 9881N, 9881O, 7807Q, 7807S not support cbk_step and cint read from mp.ini */
		if ((pid != ILI9881N_CHIP) && (pid != ILI9881O_CHIP) &&
			(pid != ILI7807Q_CHIP) && (pid != ILI7807S_CHIP)) {
			if ((open_spec.cbk_step == 0) || (open_spec.cint == 0)) {
				TS_LOG_ERR("Failed to get open parameter");
				ret = -EMP_PARA_NULL;
				core_mp.lost_parameter = true;
				goto out;
			}
		}

		TS_LOG_DEBUG("gain = %d, tvch = %d, tvcl = %d, cbk_step = %d, cint = %d\n",
			open_spec.gain, open_spec.tvch, open_spec.tvcl, open_spec.cbk_step,
			open_spec.cint );
	}

	if (titems[index].frame_count <= 0) {
		TS_LOG_ERR("Frame count is zero, which is at least set as 1\n");
		titems[index].frame_count = 1;
	}

	open = kzalloc(titems[index].frame_count * sizeof(struct mp_test_open_c), GFP_KERNEL);
	if (err_alloc_mem(open)) {
		TS_LOG_ERR("Failed to allocate open mem (%ld)\n", PTR_ERR(open));
		ret = -EMP_NOMEM;
		goto out;
	}

	if (create_mp_test_frame_buffer(index, titems[index].frame_count) < 0) {
		ret = -EMP_NOMEM;
		goto out;
	}

	if (cap_dac == NULL) {
		cap_dac = kcalloc(core_mp.frame_len, sizeof(s32), GFP_KERNEL);
		if (err_alloc_mem(cap_dac)) {
			TS_LOG_ERR("Failed to allocate cap_dac buffer\n");
			ipio_kfree((void **)&open);
			return -EMP_NOMEM;
		}
	} else {
		memset(cap_dac, 0x0, core_mp.frame_len);
	}

	if (cap_raw == NULL) {
		cap_raw = kcalloc(core_mp.frame_len, sizeof(s32), GFP_KERNEL);
		if (err_alloc_mem(cap_raw)) {
			TS_LOG_ERR("Failed to allocate cap_raw buffer\n");
			ipio_kfree((void **)&cap_dac);
			ipio_kfree((void **)&open);
			return -EMP_NOMEM;
		}
	} else {
		memset(cap_raw, 0x0, core_mp.frame_len);
	}

	/* Init Max/Min buffer */
	for (y = 0; y < core_mp.ych_len; y++) {
		for (x = 0; x < core_mp.xch_len; x++) {
			titems[index].max_buf[y * core_mp.xch_len + x] = INT_MIN;
			titems[index].min_buf[y * core_mp.xch_len + x] = INT_MAX;
		}
	}

	if (titems[index].spec_option == BENCHMARK) {
		parser_ini_benchmark(titems[index].bench_mark_max, titems[index].bench_mark_min,
				titems[index].type_option, titems[index].desp, core_mp.frame_len, BENCHMARK_KEY_NAME);
		dump_benchmark_data(titems[index].bench_mark_max, titems[index].bench_mark_min);
	}

	parser_get_int_data(titems[index].desp, "gain", str, sizeof(str));
	open_spec.gain = ili_katoi(str, g_katoi_len);

	parser_get_int_data(titems[index].desp, "tvch", str, sizeof(str));
	open_spec.tvch = ili_katoi(str, g_katoi_len);

	parser_get_int_data(titems[index].desp, "tvcl", str, sizeof(str));
	open_spec.tvcl = ili_katoi(str, g_katoi_len);

	TS_LOG_DEBUG("gain = %d, tvch = %d, tvcl = %d\n", open_spec.gain, open_spec.tvch, open_spec.tvcl);

	for (i = 0; i < titems[index].frame_count; i++) {
		open[i].cap_dac = kcalloc(core_mp.frame_len, sizeof(s32), GFP_KERNEL);
		open[i].cap_raw = kcalloc(core_mp.frame_len, sizeof(s32), GFP_KERNEL);
		open[i].dcl_cap = kcalloc(core_mp.frame_len, sizeof(s32), GFP_KERNEL);
	}

	for (i = 0; i < titems[index].frame_count; i++) {
		ret = allnode_open_cdc_data(4, open[i].cap_dac);
		if (ret < 0) {
			TS_LOG_ERR("Failed to get Open CAP DAC data, %d\n", ret);
			goto out;
		}
		ret = allnode_open_cdc_data(5, open[i].cap_raw);
		if (ret < 0) {
			TS_LOG_ERR("Failed to get Open CAP RAW data, %d\n", ret);
			goto out;
		}

		allnode_open_cdc_result(index,
			open[i].dcl_cap, open[i].cap_dac, (const int *)open[i].cap_raw);

		/* record fist frame for debug */
		if (i == 0) {
			memcpy(cap_dac, open[i].cap_dac, core_mp.frame_len * sizeof(s32));
			memcpy(cap_raw, open[i].cap_raw, core_mp.frame_len * sizeof(s32));
		}

		ili_dump_data(open[i].dcl_cap, 10, core_mp.frame_len, core_mp.xch_len, "DCL_Cap");

		addr = 0;
		for (y = 0; y < core_mp.ych_len; y++) {
			for (x = 0; x < core_mp.xch_len; x++) {
				titems[index].buf[(i * core_mp.frame_len) + addr] = open[i].dcl_cap[addr];
				addr++;
			}
		}
		compare_maxmin_result(index, &titems[index].buf[i * core_mp.frame_len]);
	}

out:
	if (open != NULL) {
		for (i = 0; i < titems[index].frame_count; i++) {
			ipio_kfree((void **)&open[i].cap_dac);
			ipio_kfree((void **)&open[i].cap_raw);
			ipio_kfree((void **)&open[i].dcl_cap);
		}
		kfree(open);
		open = NULL;
	}

	return ret;
}

static int key_test(int index)
{
	int i, j = 0, ret = 0;

	TS_LOG_DEBUG("Item = %s, Frame Count = %d\n",
		titems[index].desp, titems[index].frame_count);

	if (titems[index].frame_count == 0) {
		TS_LOG_ERR("Frame count is zero, which at least sets as 1\n");
		ret = -EINVAL;
		goto out;
	}

	ret = create_mp_test_frame_buffer(index, titems[index].frame_count);
	if (ret < 0)
		goto out;

	for (i = 0; i < titems[index].frame_count; i++) {
		ret = allnode_key_cdc_data(index);
		if (ret < 0) {
			TS_LOG_ERR("Failed to initialise CDC data, %d\n", ret);
			goto out;
		}

		for (j = 0; j < core_mp.key_len; j++)
			titems[index].buf[j] = key_buf[j];
	}

	compare_maxmin_result(index, titems[index].buf);

out:
	return ret;
}

static int self_test(int index)
{
	TS_LOG_ERR("TDDI has no self to be tested currently\n");
	return -1;
}

static int st_test(int index)
{
	TS_LOG_ERR("ST Test is not supported by the driver\n");
	return -1;
}

static int mp_get_timing_info(void)
{
	int slen = 0;
	char str[256] = {0};
	u8 info[64] = {0};

	core_mp.is_longv = 0;

	slen = parser_get_int_data("pv5_4 command",
		"timing_info_raw", str, sizeof(str));
	if (slen < 0)
		return -1;

	if (parser_get_u8_array(str, info, 16, slen) < 0)
		return -1;

	core_mp.is_longv = info[6];

	TS_LOG_INFO("DDI Mode = %s\n", (core_mp.is_longv ? "Long V" : "Long H"));

	return 0;
}

static int mp_test_data_sort_average(s32 *oringin_data, int index, s32 *avg_result)
{
	int i, j, k, x, y, len = 5, size;
	s32 u32temp;
	int u32up_frame, u32down_frame;
	s32 *u32sum_raw_data = NULL;
	s32 *u32data_buff = NULL;

	if (titems[index].frame_count <= 1)
		return 0;


	if (err_alloc_mem(oringin_data)) {
		TS_LOG_ERR("Input wrong address\n");
		return -ENOMEM;
	}

	u32data_buff = kcalloc(core_mp.frame_len * titems[index].frame_count, sizeof(s32), GFP_KERNEL);
	u32sum_raw_data = kcalloc(core_mp.frame_len, sizeof(s32), GFP_KERNEL);
	if (err_alloc_mem(u32sum_raw_data) || (err_alloc_mem(u32data_buff))) {
		TS_LOG_ERR("Failed to allocate u32sum_raw_data FRAME buffer\n");
		return -ENOMEM;
	}

	size = core_mp.frame_len * titems[index].frame_count;
	for (i = 0; i < size; i++)
		u32data_buff[i] = oringin_data[i];

	u32up_frame = titems[index].frame_count * titems[index].highest_percentage / 100;
	u32down_frame = titems[index].frame_count * titems[index].lowest_percentage / 100;
	TS_LOG_DEBUG("Up=%d, Down=%d -%s\n", u32up_frame, u32down_frame, titems[index].desp);

	if (debug_en) {
		pr_cont("\n[Show Original frist%d and last%d node data]\n", len, len);
		for (i = 0; i < core_mp.frame_len; i++) {
			for (j = 0 ; j < titems[index].frame_count ; j++) {
				if ((i < len) || (i >= (core_mp.frame_len-len)))
					pr_cont("%d,", u32data_buff[j * core_mp.frame_len + i]);
			}
			if ((i < len) || (i >= (core_mp.frame_len-len)))
				pr_cont("\n");
		}
	}

	for (i = 0; i < core_mp.frame_len; i++) {
		for (j = 0; j < titems[index].frame_count-1; j++) {
			for (k = 0; k < (titems[index].frame_count-1-j); k++) {
				x = i+k*core_mp.frame_len;
				y = i+(k+1)*core_mp.frame_len;
				if (*(u32data_buff+x) > *(u32data_buff+y)) {
					u32temp = *(u32data_buff+x);
					*(u32data_buff+x) = *(u32data_buff+y);
					*(u32data_buff+y) = u32temp;
				}
			}
		}
	}

	if (debug_en) {
		pr_cont("\n[After sorting frist%d and last%d node data]\n", len, len);
		for (i = 0; i < core_mp.frame_len; i++) {
			for (j = u32down_frame; j < titems[index].frame_count - u32up_frame; j++) {
				if ((i < len) || (i >= (core_mp.frame_len - len)))
					pr_cont("%d,", u32data_buff[i + j * core_mp.frame_len]);
			}
			if ((i < len) || (i >= (core_mp.frame_len-len)))
				pr_cont("\n");
		}
	}

	for (i = 0 ; i < core_mp.frame_len ; i++) {
		u32sum_raw_data[i] = 0;
		for (j = u32down_frame; j < titems[index].frame_count - u32up_frame; j++)
			u32sum_raw_data[i] += u32data_buff[i + j * core_mp.frame_len];

		avg_result[i] = u32sum_raw_data[i] / (titems[index].frame_count - u32down_frame - u32up_frame);
	}

	if (debug_en) {
		pr_cont("\n[Average result frist%d and last%d node data]\n", len, len);
		for (i = 0; i < core_mp.frame_len; i++) {
			if ((i < len) || (i >= (core_mp.frame_len-len)))
				pr_cont("%d,", avg_result[i]);
		}
		if ((i < len) || (i >= (core_mp.frame_len-len)))
			pr_cont("\n");
	}

	ipio_kfree((void **)&u32data_buff);
	ipio_kfree((void **)&u32sum_raw_data);
	return 0;
}

static void mp_compare_cdc_result(int index, s32 *tmp, s32 *max_ts, s32 *min_ts, int *result)
{
	int i;

	if (err_alloc_mem(tmp)) {
		TS_LOG_ERR("The data of test item is null (%p)\n", tmp);
		*result = MP_DATA_FAIL;
		return;
	}

	if (titems[index].catalog == SHORT_TEST) {
		for (i = 0; i < core_mp.frame_len; i++) {
			if (tmp[i] < min_ts[i]) {
				*result = MP_DATA_FAIL;
				return;
			}
		}
	} else {
		for (i = 0; i < core_mp.frame_len; i++) {
			if (tmp[i] > max_ts[i] || tmp[i] < min_ts[i]) {
				TS_LOG_DEBUG("Fail No.%d: max=%d, val=%d, min=%d\n", i, max_ts[i], tmp[i], min_ts[i]);
				*result = MP_DATA_FAIL;
				return;
			}
		}
	}
}

static int mp_compare_test_result(int index)
{
	int i, test_result = MP_DATA_PASS;
	s32 *max_threshold = NULL, *min_threshold = NULL;

	if (titems[index].catalog == PIN_TEST)
		return titems[index].item_result;

	max_threshold = kcalloc(core_mp.frame_len, sizeof(s32), GFP_KERNEL);
	if (err_alloc_mem(max_threshold)) {
		TS_LOG_ERR("Failed to allocate threshold FRAME buffer\n");
		test_result = MP_DATA_FAIL;
		goto out;
	}

	min_threshold = kcalloc(core_mp.frame_len, sizeof(s32), GFP_KERNEL);
	if (err_alloc_mem(min_threshold)) {
		TS_LOG_ERR("Failed to allocate threshold FRAME buffer\n");
		test_result = MP_DATA_FAIL;
		goto out;
	}

	/* Show test result as below */
	if (titems[index].catalog == TX_RX_DELTA) {
		if (err_alloc_mem(core_mp.rx_delta_buf) || err_alloc_mem(core_mp.tx_delta_buf)) {
			TS_LOG_ERR("This test item (%s) has no data inside its buffer\n", titems[index].desp);
			test_result = MP_DATA_FAIL;
			goto out;
		}

		for (i = 0; i < core_mp.frame_len; i++) {
			max_threshold[i] = core_mp.tx_delta_max;
			min_threshold[i] = core_mp.tx_delta_min;
		}
		mp_compare_cdc_result(index, core_mp.tx_max_buf, max_threshold, min_threshold, &test_result);
		mp_compare_cdc_result(index, core_mp.tx_min_buf, max_threshold, min_threshold, &test_result);

		for (i = 0; i < core_mp.frame_len; i++) {
			max_threshold[i] = core_mp.rx_delta_max;
			min_threshold[i] = core_mp.rx_delta_min;
		}

		mp_compare_cdc_result(index, core_mp.rx_max_buf, max_threshold, min_threshold, &test_result);
		mp_compare_cdc_result(index, core_mp.rx_min_buf, max_threshold, min_threshold, &test_result);
	} else {
		if (err_alloc_mem(titems[index].buf) || err_alloc_mem(titems[index].max_buf) ||
				err_alloc_mem(titems[index].min_buf) || err_alloc_mem(titems[index].result_buf)) {
			TS_LOG_ERR("This test item (%s) has no data inside its buffer\n", titems[index].desp);
			test_result = MP_DATA_FAIL;
			goto out;
		}

		if (titems[index].spec_option == BENCHMARK) {
			if (titems[index].catalog == PEAK_TO_PEAK_TEST) {
				for (i = 0; i < core_mp.frame_len; i++) {
					max_threshold[i] = titems[index].bch_mrk_max[0][i];
					min_threshold[i] = titems[index].bch_mrk_min[0][i];
				}
			} else {
				for (i = 0; i < core_mp.frame_len; i++) {
					max_threshold[i] = titems[index].bench_mark_max[i];
					min_threshold[i] = titems[index].bench_mark_min[i];
				}
			}
		} else {
			for (i = 0; i < core_mp.frame_len; i++) {
				max_threshold[i] = titems[index].max;
				min_threshold[i] = titems[index].min;
			}
		}

		/* general result */
		if (titems[index].trimmed_mean && titems[index].catalog != PEAK_TO_PEAK_TEST) {
			mp_test_data_sort_average(titems[index].buf, index, titems[index].result_buf);
			mp_compare_cdc_result(index, titems[index].result_buf, max_threshold, min_threshold, &test_result);
		} else {
			if (titems[index].bch_mrk_multi) {
				for (i = 0; i < titems[index].bch_mrk_frm_num; i++)
					mp_compare_cdc_result(index, frm_buf[i], titems[index].bch_mrk_max[i], titems[index].bch_mrk_min[i], &test_result);
			} else {
				mp_compare_cdc_result(index, titems[index].max_buf, max_threshold, min_threshold, &test_result);
				mp_compare_cdc_result(index, titems[index].min_buf, max_threshold, min_threshold, &test_result);
			}
		}
	}

out:
	ipio_kfree((void **)&max_threshold);
	ipio_kfree((void **)&min_threshold);
	if (frm_buf != NULL) {
		for (i = 0; i < titems[index].bch_mrk_frm_num; i++)
			if (frm_buf[i] != NULL) {
				kfree(frm_buf[i]);
				frm_buf[i] = NULL;
			}
		kfree(frm_buf);
		frm_buf = NULL;
	}
	titems[index].item_result = test_result;
	return test_result;
}

static void mp_do_retry(int index, int count)
{
	if (count == 0) {
		TS_LOG_INFO("Finish retry action\n");
		return;
	}

	TS_LOG_INFO("retry = %d, item = %s\n", count, titems[index].desp);

	titems[index].do_test(index);

	if (mp_compare_test_result(index) < 0)
		return mp_do_retry(index, count - 1);
}

static int mp_show_result(struct ts_rawdata_info *raw_info, bool lcm_on)
{
	int ret = MP_DATA_PASS, seq;
	int i, x, y, j, csv_len = 0, pass_item_count = 0, line_count = 0, get_frame_cont = 1;
	s32 *max_threshold = NULL, *min_threshold = NULL;
	char *csv = NULL;
	char *ret_pass_name = NULL, *ret_fail_name = NULL;
	struct file *f = NULL;
	mm_segment_t fs;
	loff_t pos;
	char frame_name[128] = {0};

	csv = vmalloc(CSV_FILE_SIZE);
	if (err_alloc_mem(csv)) {
		TS_LOG_ERR("Failed to allocate CSV mem\n");
		ret = -EMP_NOMEM;
		goto fail_open;
	}

	max_threshold = kcalloc(core_mp.frame_len, sizeof(s32), GFP_KERNEL);
	min_threshold = kcalloc(core_mp.frame_len, sizeof(s32), GFP_KERNEL);
	if (err_alloc_mem(max_threshold) || err_alloc_mem(min_threshold)) {
		TS_LOG_ERR("Failed to allocate threshold FRAME buffer\n");
		ret = -EMP_NOMEM;
		goto fail_open;
	}

	if (parser_get_ini_key_value("pv5_4 command",
		"date", core_mp.ini_date, sizeof(core_mp.ini_date)) < 0)
		memcpy(core_mp.ini_date, "Unknown", strlen("Unknown"));
	if (parser_get_ini_key_value("pv5_4 command",
		"version", core_mp.ini_ver, sizeof(core_mp.ini_ver)) < 0)
		memcpy(core_mp.ini_ver, "Unknown", strlen("Unknown"));

	mp_print_csv_header(csv, &csv_len, &line_count, CSV_FILE_SIZE);

	for (seq = 0; seq < g_run_index.count; seq++) {
		i = g_run_index.index[seq];
		get_frame_cont = 1;

		if (titems[i].item_result == MP_DATA_PASS) {
			TS_LOG_INFO("\n[%s],OK\n\n", titems[i].desp);
			csv_len += snprintf(csv + csv_len, (CSV_FILE_SIZE - csv_len), "\n[%s],OK\n\n", titems[i].desp);
		} else {
			TS_LOG_INFO("\n[%s],NG\n\n", titems[i].desp);
			csv_len += snprintf(csv + csv_len, (CSV_FILE_SIZE - csv_len), "\n[%s],NG\n\n", titems[i].desp);
		}

		if (titems[i].catalog == PIN_TEST) {
			TS_LOG_INFO("Test INT Pin = %d\n", titems[i].test_int_pin);
			csv_len += snprintf(csv + csv_len, (CSV_FILE_SIZE - csv_len), "Test INT Pin = %d\n", titems[i].test_int_pin);
			TS_LOG_INFO("Pulse Test = %d\n", titems[i].int_pulse_test);
			csv_len += snprintf(csv + csv_len, (CSV_FILE_SIZE - csv_len), "Pulse Test = %d\n", titems[i].int_pulse_test);
			TS_LOG_INFO("Delay Time = %d\n", titems[i].delay_time);
			csv_len += snprintf(csv + csv_len, (CSV_FILE_SIZE - csv_len), "Delay Time = %d\n", titems[i].delay_time);
			continue;
		}

		mp_print_csv_cdc_cmd(csv, &csv_len, i, CSV_FILE_SIZE);

		TS_LOG_INFO("Frame count = %d\n", titems[i].frame_count);
		csv_len += snprintf(csv + csv_len, (CSV_FILE_SIZE - csv_len), "Frame count = %d\n", titems[i].frame_count);

		if (titems[i].trimmed_mean && titems[i].catalog != PEAK_TO_PEAK_TEST) {
			TS_LOG_INFO("lowest percentage = %d\n", titems[i].lowest_percentage);
			csv_len += snprintf(csv + csv_len, (CSV_FILE_SIZE - csv_len), "lowest percentage = %d\n", titems[i].lowest_percentage);

			TS_LOG_INFO("highest percentage = %d\n", titems[i].highest_percentage);
			csv_len += snprintf(csv + csv_len, (CSV_FILE_SIZE - csv_len), "highest percentage = %d\n", titems[i].highest_percentage);
		}

		/* Show result of benchmark max and min */
		if (titems[i].spec_option == BENCHMARK) {
			if (titems[i].catalog == PEAK_TO_PEAK_TEST) {
				for (j = 0; j < core_mp.frame_len; j++) {
					max_threshold[j] = titems[i].bch_mrk_max[0][j];
					min_threshold[j] = titems[i].bch_mrk_min[0][j];
				}

				for (j = 0; j < titems[i].bch_mrk_frm_num; j++) {
					char bench_name[128] = {0};
					snprintf(bench_name, (CSV_FILE_SIZE - csv_len), "Max_Bench%d", (j+1));
					mp_compare_cdc_show_result(i, titems[i].bch_mrk_max[j], csv, &csv_len, TYPE_BENCHMARK, max_threshold, min_threshold, bench_name, CSV_FILE_SIZE);
					snprintf(bench_name, (CSV_FILE_SIZE - csv_len), "Min_Bench%d", (j+1));
					mp_compare_cdc_show_result(i, titems[i].bch_mrk_min[j], csv, &csv_len, TYPE_BENCHMARK, max_threshold, min_threshold, bench_name, CSV_FILE_SIZE);
				}
			} else {
				for (j = 0; j < core_mp.frame_len; j++) {
					max_threshold[j] = titems[i].bench_mark_max[j];
					min_threshold[j] = titems[i].bench_mark_min[j];
				}
				mp_compare_cdc_show_result(i, titems[i].bench_mark_max, csv, &csv_len, TYPE_BENCHMARK, max_threshold, min_threshold, "Max_Bench", CSV_FILE_SIZE);
				mp_compare_cdc_show_result(i, titems[i].bench_mark_min, csv, &csv_len, TYPE_BENCHMARK, max_threshold, min_threshold, "Min_Bench", CSV_FILE_SIZE);
			}

		} else {
			for (j = 0; j < core_mp.frame_len; j++) {
				max_threshold[j] = titems[i].max;
				min_threshold[j] = titems[i].min;
			}

			TS_LOG_INFO("Max = %d\n", titems[i].max);
			csv_len += snprintf(csv + csv_len, (CSV_FILE_SIZE - csv_len), "Max = %d\n", titems[i].max);

			TS_LOG_INFO("Min = %d\n", titems[i].min);
			csv_len += snprintf(csv + csv_len, (CSV_FILE_SIZE - csv_len), "Min = %d\n", titems[i].min);
		}

		if (ipio_strcmp(titems[i].desp, "open test(integration)_sp") == 0) {
			mp_compare_cdc_show_result(i, frame1_cbk700, csv, &csv_len, TYPE_NO_JUGE, max_threshold, min_threshold, "frame1 cbk700", CSV_FILE_SIZE);
			mp_compare_cdc_show_result(i, frame1_cbk250, csv, &csv_len, TYPE_NO_JUGE, max_threshold, min_threshold, "frame1 cbk250", CSV_FILE_SIZE);
			mp_compare_cdc_show_result(i, frame1_cbk200, csv, &csv_len, TYPE_NO_JUGE, max_threshold, min_threshold, "frame1 cbk200", CSV_FILE_SIZE);
		}

		if (ipio_strcmp(titems[i].desp, "open test_c") == 0) {
			mp_compare_cdc_show_result(i, cap_dac, csv, &csv_len, TYPE_NO_JUGE, max_threshold, min_threshold, "CAP_DAC", CSV_FILE_SIZE);
			mp_compare_cdc_show_result(i, cap_raw, csv, &csv_len, TYPE_NO_JUGE, max_threshold, min_threshold, "CAP_RAW", CSV_FILE_SIZE);
		}

		if (titems[i].catalog == TX_RX_DELTA) {
			if (err_alloc_mem(core_mp.rx_delta_buf) || err_alloc_mem(core_mp.tx_delta_buf)) {
				TS_LOG_ERR("This test item (%s) has no data inside its buffer\n", titems[i].desp);
				continue;
			}
		} else {
			if (err_alloc_mem(titems[i].buf) || err_alloc_mem(titems[i].max_buf) ||
					err_alloc_mem(titems[i].min_buf)) {
				TS_LOG_ERR("This test item (%s) has no data inside its buffer\n", titems[i].desp);
				continue;
			}
		}

		/* Show test result as below */
		if (titems[i].catalog == KEY_TEST) {
			for (x = 0; x < core_mp.key_len; x++) {
				dump("KEY_%02d ", x);
				csv_len += snprintf(csv + csv_len, (CSV_FILE_SIZE - csv_len), "KEY_%02d,", x);
			}

			dump("\n");
			csv_len += snprintf(csv + csv_len, (CSV_FILE_SIZE - csv_len), "\n");

			for (y = 0; y < core_mp.key_len; y++) {
				dump(" %3d   ", titems[i].buf[y]);
				csv_len += snprintf(csv + csv_len, (CSV_FILE_SIZE - csv_len), " %3d, ", titems[i].buf[y]);
			}

			dump("\n");
			csv_len += snprintf(csv + csv_len, (CSV_FILE_SIZE - csv_len), "\n");
		} else if (titems[i].catalog == TX_RX_DELTA) {
			for (j = 0; j < core_mp.frame_len; j++) {
				max_threshold[j] = core_mp.tx_delta_max;
				min_threshold[j] = core_mp.tx_delta_min;
			}
			mp_compare_cdc_show_result(i, core_mp.tx_max_buf, csv, &csv_len, TYPE_JUGE, max_threshold, min_threshold, "TX Max Hold", CSV_FILE_SIZE);
			mp_compare_cdc_show_result(i, core_mp.tx_min_buf, csv, &csv_len, TYPE_JUGE, max_threshold, min_threshold, "TX Min Hold", CSV_FILE_SIZE);

			for (j = 0; j < core_mp.frame_len; j++) {
				max_threshold[j] = core_mp.rx_delta_max;
				min_threshold[j] = core_mp.rx_delta_min;
			}
			mp_compare_cdc_show_result(i, core_mp.rx_max_buf, csv, &csv_len, TYPE_JUGE, max_threshold, min_threshold, "RX Max Hold", CSV_FILE_SIZE);
			mp_compare_cdc_show_result(i, core_mp.rx_min_buf, csv, &csv_len, TYPE_JUGE, max_threshold, min_threshold, "RX Min Hold", CSV_FILE_SIZE);
		} else {
			/* general result */
			if (titems[i].trimmed_mean && titems[i].catalog != PEAK_TO_PEAK_TEST) {
				mp_compare_cdc_show_result(i, titems[i].result_buf, csv, &csv_len, TYPE_JUGE, max_threshold, min_threshold, "Mean result", CSV_FILE_SIZE);
			} else {
				if (!titems[i].bch_mrk_multi) {
					mp_compare_cdc_show_result(i, titems[i].max_buf, csv, &csv_len, TYPE_JUGE, max_threshold, min_threshold, "Max Hold", CSV_FILE_SIZE);
					mp_compare_cdc_show_result(i, titems[i].min_buf, csv, &csv_len, TYPE_JUGE, max_threshold, min_threshold, "Min Hold", CSV_FILE_SIZE);
				}

			}
			if (titems[i].catalog != PEAK_TO_PEAK_TEST)
				get_frame_cont = titems[i].frame_count;

			if (titems[i].bch_mrk_multi)
				get_frame_cont = titems[i].bch_mrk_frm_num;

			/* result of each frame */
			for (j = 0; j < get_frame_cont; j++) {
				snprintf(frame_name, (CSV_FILE_SIZE - csv_len), "Frame %d", (j+1));
				mp_compare_cdc_show_result(i, &titems[i].buf[(j*core_mp.frame_len)], csv, &csv_len, TYPE_NO_JUGE, max_threshold, min_threshold, frame_name, CSV_FILE_SIZE);
			}
			ilitek_show_rawdata(i, raw_info);
		}
	}

	memset(csv_name, 0, 128 * sizeof(char));

	mp_print_csv_tail(csv, &csv_len, CSV_FILE_SIZE);

	for (i = 0; i < MP_TEST_ITEM; i++) {
		if (titems[i].run) {
			if (titems[i].item_result < 0) {
				pass_item_count = 0;
				break;
			}
			pass_item_count++;
		}
	}

	/* define csv file name */
	ret_pass_name = NORMAL_CSV_PASS_NAME;
	ret_fail_name = NORMAL_CSV_FAIL_NAME;

	if (pass_item_count == 0) {
		core_mp.final_result = MP_DATA_FAIL;
		ret = MP_DATA_FAIL;
		if((core_mp.lost_parameter) && (ilits->production_test_formula_compatible)) {
			ret_fail_name = NULL;
			ret_fail_name = NORMAL_CSV_WARNING_NAME;
			TS_LOG_ERR("WARNING! OPEN or SHORT parameter not found in ini file!!\n");
		}
		if (lcm_on)
			snprintf(csv_name, (CSV_FILE_SIZE - csv_len), "%s/%s_%s.csv", CSV_LCM_ON_PATH, get_date_time_str(), ret_fail_name);
		else
			snprintf(csv_name, (CSV_FILE_SIZE - csv_len), "%s/%s_%s.csv", CSV_LCM_OFF_PATH, get_date_time_str(), ret_fail_name);
	} else {
		core_mp.final_result = MP_DATA_PASS;
		ret = MP_DATA_PASS;

		if (core_mp.lost_benchmark) {
			ret_pass_name = NULL;
			ret_pass_name = NORMAL_CSV_WARNING_NAME;
			TS_LOG_ERR("WARNING! Golden and SPEC in ini file aren't matched!!\n");
		}

		if (lcm_on)
			snprintf(csv_name, (CSV_FILE_SIZE - csv_len), "%s/%s_%s.csv", CSV_LCM_ON_PATH, get_date_time_str(), ret_pass_name);
		else
			snprintf(csv_name, (CSV_FILE_SIZE - csv_len), "%s/%s_%s.csv", CSV_LCM_OFF_PATH, get_date_time_str(), ret_pass_name);
	}

	TS_LOG_INFO("Open CSV : %s\n", csv_name);

	if (f == NULL)
		f = filp_open(csv_name, O_WRONLY | O_CREAT | O_TRUNC, 666);

	if (err_alloc_mem(f)) {
		TS_LOG_ERR("Failed to open CSV file");
		ret = -EMP_NOMEM;
		goto fail_open;
	}

	TS_LOG_INFO("Open CSV succeed, its length = %d\n ", csv_len);

	if (csv_len >= CSV_FILE_SIZE) {
		TS_LOG_ERR("The length saved to CSV is too long !\n");
		ret = -EMP_INVAL;
		goto fail_open;
	}

	fs = get_fs();
	set_fs(KERNEL_DS);
	pos = 0;
	vfs_write(f, csv, csv_len, &pos);
	set_fs(fs);
	filp_close(f, NULL);

	TS_LOG_INFO("Writing Data into CSV succeed\n");

fail_open:
	ipio_vfree((void **)&csv);
	ipio_kfree((void **)&max_threshold);
	ipio_kfree((void **)&min_threshold);
	return ret;
}

static void ilitek_tddi_mp_init_item(void)
{
	int i = 0;

	memset(&core_mp, 0, sizeof(core_mp));
	memset(&open_spec, 0, sizeof(open_spec));
	memset(&short_para, 0, sizeof(short_para));

	core_mp.chip_pid = ilits->chip->pid;
	core_mp.chip_id = ilits->chip->id;
	core_mp.chip_type = ilits->chip->type;
	core_mp.chip_ver = ilits->chip->ver;
	core_mp.fw_ver = ilits->chip->fw_ver;
	core_mp.protocol_ver = ilits->protocol->ver;
	core_mp.core_ver = ilits->chip->core_ver;
	core_mp.cdc_len = ilits->protocol->cdc_len;
	core_mp.no_bk_shift = ilits->chip->no_bk_shift;
	core_mp.xch_len = ilits->xch_num;
	core_mp.ych_len = ilits->ych_num;
	core_mp.frame_len = core_mp.xch_len * core_mp.ych_len;
	core_mp.stx_len = 0;
	core_mp.srx_len = 0;
	core_mp.key_len = 0;
	core_mp.st_len = 0;
	core_mp.tdf = 240;
	core_mp.busy_cdc = INT_CHECK;
	core_mp.retry = ilits->mp_retry;
	core_mp.td_retry = false;
	core_mp.final_result = MP_DATA_FAIL;
	core_mp.lost_benchmark = false;
	core_mp.lost_parameter = false;

	TS_LOG_INFO("============== TP & Panel info ================\n");
	TS_LOG_INFO("Driver version = %s\n", DRIVER_VERSION);
	TS_LOG_INFO("TP Module = %s\n", ilits->md_name);
	TS_LOG_INFO("CHIP = 0x%x\n", core_mp.chip_pid);
	TS_LOG_INFO("Firmware version = %x\n", core_mp.fw_ver);
	TS_LOG_INFO("Protocol version = %x\n", core_mp.protocol_ver);
	TS_LOG_INFO("Core version = %x\n", core_mp.core_ver);
	TS_LOG_INFO("Read CDC Length = %d\n", core_mp.cdc_len);
	TS_LOG_INFO("X length = %d, Y length = %d\n", core_mp.xch_len, core_mp.ych_len);
	TS_LOG_INFO("Frame length = %d\n", core_mp.frame_len);
	TS_LOG_INFO("Check busy method = %s\n", (core_mp.busy_cdc ? "Polling" : "Interrupt"));
	TS_LOG_INFO("===============================================\n");

	for (i = 0; i < MP_TEST_ITEM; i++) {
		titems[i].spec_option = 0;
		titems[i].type_option = 0;
		titems[i].run = false;
		titems[i].max = 0;
		titems[i].max_res = MP_DATA_FAIL;
		titems[i].item_result = MP_DATA_PASS;
		titems[i].min = 0;
		titems[i].min_res = MP_DATA_FAIL;
		titems[i].frame_count = 0;
		titems[i].trimmed_mean = 0;
		titems[i].lowest_percentage = 0;
		titems[i].highest_percentage = 0;
		titems[i].v_tdf_1 = 0;
		titems[i].v_tdf_2 = 0;
		titems[i].h_tdf_1 = 0;
		titems[i].h_tdf_2 = 0;
		titems[i].max_min_mode = 0;
		titems[i].bch_mrk_multi = false;
		titems[i].bch_mrk_frm_num = 1;
		titems[i].goldenmode = 0;
		titems[i].retry_cnt = RETRY_COUNT;
		titems[i].result_buf = NULL;
		titems[i].buf = NULL;
		titems[i].max_buf = NULL;
		titems[i].min_buf = NULL;
		titems[i].bench_mark_max = NULL;
		titems[i].bench_mark_min = NULL;
		titems[i].bch_mrk_max = NULL;
		titems[i].bch_mrk_min = NULL;
		titems[i].node_type = NULL;
		titems[i].delay_time = 0;
		titems[i].test_int_pin = 0;
		titems[i].int_pulse_test = 0;

		if (titems[i].catalog == MUTUAL_TEST) {
			titems[i].do_test = mutual_test;
		} else if (titems[i].catalog == TX_RX_DELTA) {
			titems[i].do_test = mutual_test;
		} else if (titems[i].catalog == UNTOUCH_P2P) {
			titems[i].do_test = mutual_test;
		} else if (titems[i].catalog == PIXEL) {
			titems[i].do_test = mutual_test;
		} else if (titems[i].catalog == OPEN_TEST) {
			if (ipio_strcmp(titems[i].desp, "open test(integration)_sp") == 0)
				titems[i].do_test = open_test_sp;
			else if (ipio_strcmp(titems[i].desp, "open test_c") == 0)
				titems[i].do_test = open_test_cap;
			else
				titems[i].do_test = mutual_test;
		} else if (titems[i].catalog == KEY_TEST) {
			titems[i].do_test = key_test;
		} else if (titems[i].catalog == SELF_TEST) {
			titems[i].do_test = self_test;
		} else if (titems[i].catalog == ST_TEST) {
			titems[i].do_test = st_test;
		} else if (titems[i].catalog == PEAK_TO_PEAK_TEST) {
			titems[i].do_test = peak_to_peak_test;
		} else if (titems[i].catalog == SHORT_TEST) {
			titems[i].do_test = mutual_test;
		} else if (titems[i].catalog == PIN_TEST) {
			titems[i].do_test = pin_test;
		}

		titems[i].result = kmalloc(16, GFP_KERNEL);
		if (!titems[i].result) {
			TS_LOG_ERR("kmalloc failed\n");
			return;
		}
		snprintf(titems[i].result, 16, "%s", "FAIL");
	}
}

static void mp_p2p_td_retry_after_ra_fail(int p2p_td)
{
	int i;

	for (i = 0; i < MP_TEST_ITEM; i++) {
		if (ipio_strcmp(titems[i].desp, "noise peak to peak(with panel) (lcm off)") == 0)
			break;
	}

	if (i >= MP_TEST_ITEM)
		return;

	TS_LOG_DEBUG("i = %d, p2p_noise_ret = %d, p2p_noise_run = %d\n",
		i, titems[i].item_result, titems[i].run);

	if (titems[i].item_result == MP_DATA_PASS && titems[i].run == 1)
		titems[p2p_td].do_test(p2p_td);
}

static void mp_test_run(int index)
{
	int i = index, j;
	char str[512] = {0};


	/* Get parameters from ini */
	parser_get_int_data(titems[i].desp, "spec option", str, sizeof(str));
	titems[i].spec_option = ili_katoi(str, g_katoi_len);
	parser_get_int_data(titems[i].desp, "type option", str, sizeof(str));
	titems[i].type_option = ili_katoi(str, g_katoi_len);
	parser_get_int_data(titems[i].desp, "frame count", str, sizeof(str));
	titems[i].frame_count = ili_katoi(str, g_katoi_len);
	parser_get_int_data(titems[i].desp, "trimmed mean", str, sizeof(str));
	titems[i].trimmed_mean = ili_katoi(str, g_katoi_len);
	parser_get_int_data(titems[i].desp, "lowest percentage", str, sizeof(str));
	titems[i].lowest_percentage = ili_katoi(str, g_katoi_len);
	parser_get_int_data(titems[i].desp, "highest percentage", str, sizeof(str));
	titems[i].highest_percentage = ili_katoi(str, g_katoi_len);
	parser_get_int_data(titems[i].desp, "goldenmode", str, sizeof(str));
	titems[i].goldenmode = ili_katoi(str, g_katoi_len);
	parser_get_int_data(titems[i].desp, "max min mode", str, sizeof(str));
	titems[i].max_min_mode = ili_katoi(str, g_katoi_len);

	parser_get_int_data(titems[i].desp, "retry count", str, sizeof(str));
	titems[i].retry_cnt = ili_katoi(str, g_katoi_len);

	if (titems[i].goldenmode && (titems[i].spec_option != titems[i].goldenmode))
		core_mp.lost_benchmark = true;

	/* Get pin test delay time */
	if (titems[i].catalog == PIN_TEST) {
		parser_get_int_data(titems[i].desp, "test int pin", str, sizeof(str));
		titems[i].test_int_pin = ili_katoi(str, g_katoi_len);
		parser_get_int_data(titems[i].desp, "int pulse test", str, sizeof(str));
		titems[i].int_pulse_test = ili_katoi(str, g_katoi_len);
		parser_get_int_data(titems[i].desp, "delay time", str, sizeof(str));
		titems[i].delay_time = ili_katoi(str, g_katoi_len);
	}

	/* Get TDF value from ini */
	if (titems[i].catalog == SHORT_TEST) {
		parser_get_int_data(titems[i].desp, "v_tdf_1", str, sizeof(str));
		titems[i].v_tdf_1 = parser_get_tdf_value((const char *)str, titems[i].catalog);
		parser_get_int_data(titems[i].desp, "v_tdf_2", str, sizeof(str));
		titems[i].v_tdf_2 = parser_get_tdf_value((const char *)str, titems[i].catalog);
		parser_get_int_data(titems[i].desp, "h_tdf_1", str, sizeof(str));
		titems[i].h_tdf_1 = parser_get_tdf_value((const char *)str, titems[i].catalog);
		parser_get_int_data(titems[i].desp, "h_tdf_2", str, sizeof(str));
		titems[i].h_tdf_2 = parser_get_tdf_value((const char *)str, titems[i].catalog);
	} else {
		parser_get_int_data(titems[i].desp, "v_tdf", str, sizeof(str));
		titems[i].v_tdf_1 = parser_get_tdf_value((const char *)str, titems[i].catalog);
		parser_get_int_data(titems[i].desp, "h_tdf", str, sizeof(str));
		titems[i].h_tdf_1 = parser_get_tdf_value((const char *)str, titems[i].catalog);
	}

	/* Get threshold from ini structure in parser */
	if (ipio_strcmp(titems[i].desp, "tx/rx delta") == 0) {
		parser_get_int_data(titems[i].desp, "tx max", str, sizeof(str));
		core_mp.tx_delta_max = ili_katoi(str, g_katoi_len);
		parser_get_int_data(titems[i].desp, "tx min", str, sizeof(str));
		core_mp.tx_delta_min = ili_katoi(str, g_katoi_len);
		parser_get_int_data(titems[i].desp, "rx max", str, sizeof(str));
		core_mp.rx_delta_max = ili_katoi(str, g_katoi_len);
		parser_get_int_data(titems[i].desp, "rx min", str, sizeof(str));
		core_mp.rx_delta_min = ili_katoi(str, g_katoi_len);
		TS_LOG_DEBUG("%s: Tx Max = %d, Tx Min = %d, Rx Max = %d,  Rx Min = %d\n",
				titems[i].desp, core_mp.tx_delta_max, core_mp.tx_delta_min,
				core_mp.rx_delta_max, core_mp.rx_delta_min);
	} else {
		parser_get_int_data(titems[i].desp, "max", str, sizeof(str));
		titems[i].max = ili_katoi(str, g_katoi_len);
		parser_get_int_data(titems[i].desp, "min", str, sizeof(str));
		titems[i].min = ili_katoi(str, g_katoi_len);
	}


	/* Get pin test delay time */
	if (titems[i].catalog == PEAK_TO_PEAK_TEST) {
		if (titems[i].max_min_mode == 1 && titems[i].spec_option == 1) {
			titems[i].bch_mrk_multi = true;
			for (j = 1; j < 11; j++) {
				char tmp[8] = {0};
				/* format complete string from the name of section "_Benchmark_Data". */
				snprintf(str, sizeof(str), "%s%s%s%d", titems[index].desp, "_", BENCHMARK_KEY_NAME, j);
				if (parser_get_int_data(str, str, tmp, sizeof(tmp)) <= 0)
					break;

				titems[i].bch_mrk_frm_num = j;
			}
			TS_LOG_DEBUG("set bch_mrk_frm_num = %d\n", titems[i].bch_mrk_frm_num);
		}
	}

	TS_LOG_DEBUG("%s: run = %d, max = %d, min = %d, frame_count = %d\n", titems[i].desp,
			titems[i].run, titems[i].max, titems[i].min, titems[i].frame_count);

	TS_LOG_DEBUG("v_tdf_1 = %d, v_tdf_2 = %d, h_tdf_1 = %d, h_tdf_2 = %d", titems[i].v_tdf_1,
			titems[i].v_tdf_2, titems[i].h_tdf_1, titems[i].h_tdf_2);

	TS_LOG_INFO("Run MP Test Item : %s\n", titems[i].desp);
	titems[i].do_test(i);

	mp_compare_test_result(i);

	/* P2P TD retry after RA sample failed. */
	if (ipio_strcmp(titems[i].desp, "peak to peak_td (lcm off)") == 0 &&
		titems[i].item_result == MP_DATA_FAIL) {
		parser_get_int_data(titems[i].desp, "recheck ptop lcm off", str, sizeof(str));
		TS_LOG_INFO("Peak to Peak TD retry = %d\n", ili_katoi(str, g_katoi_len));
		core_mp.td_retry = ili_katoi(str, g_katoi_len);
		if (core_mp.td_retry)
			mp_p2p_td_retry_after_ra_fail(i);
	}

	if (core_mp.retry && titems[i].item_result == MP_DATA_FAIL) {
		TS_LOG_INFO("MP failed, doing retry %d times\n", titems[i].retry_cnt);
		mp_do_retry(i, titems[i].retry_cnt);
	}

}

static void mp_test_free(void)
{
	int i, j;

	TS_LOG_INFO("Free all allocated mem for MP\n");

	core_mp.final_result = MP_DATA_FAIL;
	core_mp.td_retry = false;

	for (i = 0; i < MP_TEST_ITEM; i++) {
		titems[i].run = false;
		titems[i].max_res = MP_DATA_FAIL;
		titems[i].min_res = MP_DATA_FAIL;
		titems[i].item_result = MP_DATA_PASS;

		if (titems[i].catalog == TX_RX_DELTA) {
			ipio_kfree((void **)&core_mp.rx_delta_buf);
			ipio_kfree((void **)&core_mp.tx_delta_buf);
			ipio_kfree((void **)&core_mp.tx_max_buf);
			ipio_kfree((void **)&core_mp.tx_min_buf);
			ipio_kfree((void **)&core_mp.rx_max_buf);
			ipio_kfree((void **)&core_mp.rx_min_buf);
		} else {
			if (titems[i].spec_option == BENCHMARK) {
				ipio_kfree((void **)&titems[i].bench_mark_max);
				ipio_kfree((void **)&titems[i].bench_mark_min);
				if (titems[i].bch_mrk_max != NULL) {
					for (j = 0; j < titems[i].bch_mrk_frm_num; j++)
						if (titems[i].bch_mrk_max[j] != NULL) {
							kfree(titems[i].bch_mrk_max[j]);
							titems[i].bch_mrk_max[j] = NULL;
						}
					kfree(titems[i].bch_mrk_max);
					titems[i].bch_mrk_max = NULL;
				}
				if (titems[i].bch_mrk_min != NULL) {
					for (j = 0; j < titems[i].bch_mrk_frm_num; j++)
						if (titems[i].bch_mrk_min[j] != NULL) {
							kfree(titems[i].bch_mrk_min[j]);
							titems[i].bch_mrk_min[j] = NULL;
						}
					kfree(titems[i].bch_mrk_min);
					titems[i].bch_mrk_min = NULL;
				}
			}
			ipio_kfree((void **)&titems[i].node_type);
			ipio_kfree((void **)&titems[i].result);
			ipio_kfree((void **)&titems[i].result_buf);
			ipio_kfree((void **)&titems[i].max_buf);
			ipio_kfree((void **)&titems[i].min_buf);
			ipio_vfree((void **)&titems[i].buf);
		}
	}

	ipio_kfree((void **)&frame1_cbk700);
	ipio_kfree((void **)&frame1_cbk250);
	ipio_kfree((void **)&frame1_cbk200);
	ipio_kfree((void **)&frame_buf);
	ipio_kfree((void **)&key_buf);
	if (frm_buf != NULL) {
		for (j = 0; j < 3; j++)
			if (frm_buf[j] != NULL) {
				kfree(frm_buf[j]);
				frm_buf[j] = NULL;
			}
		kfree(frm_buf);
		frm_buf = NULL;
	}
}

/* The method to copy results to user depends on what APK needs */
static void ilitek_show_rawdata(int index, struct ts_rawdata_info *raw_info)
{
	int x_ch = 0, y_ch = 0, data_nums = 0;
	int j;

	x_ch = core_mp.xch_len;
	y_ch = core_mp.ych_len;
	data_nums = core_mp.frame_len;
	raw_info->buff[0] = y_ch;
	raw_info->buff[1] = x_ch;
		for (j = 0; j < data_nums; j++)
			raw_info->buff[ilits->offset + j] = (int)(titems[index].buf[j]);
		ilits->offset += data_nums;
}
static void ilitek_result_generate(struct ts_rawdata_info *raw_info, u8 *result_str)
{
	int i, seq,len = 0;

	if (!result_str) {
		TS_LOG_ERR("result buffer is null\n");
		return;
	}
	/* 0- bus test */
	len += snprintf(result_str+len, TS_RAWDATA_RESULT_MAX - len, "0P-");
	for (seq = 0; seq < g_run_index.count; seq++) {
		i = g_run_index.index[seq];
		if (titems[i].item_result == MP_DATA_FAIL) {
			TS_LOG_ERR("[%s] = FAIL\n", titems[i].desp);
			strncpy(ts_test_failed_reason,
				"panel_reason-", ILITEK_TEST_FAILED_REASON_LEN - 1);
			len += snprintf(result_str + len, TS_RAWDATA_RESULT_MAX - len, "%dF-", seq + 1);
		} else {
			TS_LOG_INFO("[%s] = PASS\n", titems[i].desp);
			len += snprintf(result_str + len, TS_RAWDATA_RESULT_MAX - len, "%dP-", seq + 1);
		}
	}
	len += snprintf(result_str + len, TS_RAWDATA_RESULT_MAX - len, "%s%s%x-%s\n",
		ts_test_failed_reason, TDDI_DEV_ID, ilits->chip->pid, ilits->project_id);
	strncpy(raw_info->result,
		result_str, TS_RAWDATA_RESULT_MAX - 1);

}
static void mp_copy_ret_to_apk(u8 *buf)
{
	int i, seq, len = 2;

	if (!buf) {
		TS_LOG_ERR("apk buffer is null\n");
		return;
	}

	len += snprintf(buf + len, PAGE_SIZE - len, "CSV path: %s\n\n", csv_name);
	for (seq = 0; seq < g_run_index.count; seq++) {
		i = g_run_index.index[seq];
		if (titems[i].item_result == MP_DATA_FAIL) {
			TS_LOG_ERR("[%s] = FAIL\n", titems[i].desp);
			len += snprintf(buf + len, PAGE_SIZE - len, "[%s] = FAIL\n", titems[i].desp);
		} else {
			TS_LOG_INFO("[%s] = PASS\n", titems[i].desp);
			len += snprintf(buf + len, PAGE_SIZE - len, "[%s] = PASS\n", titems[i].desp);
		}
	}
	ilits->mp_ret_len = len;
}

static int mp_sort_item(bool lcm_on)
{
	int i, j;
	char str[128] = {0};

	g_run_index.count = 0;
	memset(g_run_index.index, 0x0, MP_TEST_ITEM);

	for (i = 0; i < MAX_SECTION_NUM; i++) {
		for (j = 0; j < MP_TEST_ITEM; j++) {
			if (ipio_strcmp(seq_item[i], titems[j].desp) != 0)
				continue;

			parser_get_int_data(titems[j].desp, "enable", str, sizeof(str));
			titems[j].run = ili_katoi(str, g_katoi_len);
			if (titems[j].run != 1 || titems[j].lcm != lcm_on)
				continue;

			if (g_run_index.count > MP_TEST_ITEM) {
				TS_LOG_ERR("Test item%d is invaild, abort\n", g_run_index.count);
				return -EINVAL;
			}

			g_run_index.index[g_run_index.count] = j;
			g_run_index.count++;
		}
	}
	return 0;
}

int ili_mp_test_main(struct ts_rawdata_info *raw_info, u8 *apk, bool lcm_on)
{
	int i, ret = 0;
	char str[128] = {0}, ver[128] = {0};

	if (ilits->xch_num <= 0 || ilits->ych_num <= 0) {
		TS_LOG_ERR("Invalid frame length %d, %d\n", ilits->xch_num, ilits->ych_num);
		ret = -EMP_INVAL;
		goto out;
	}

	ini_info = (struct ini_file_data *)vmalloc(sizeof(struct ini_file_data) * PARSER_MAX_KEY_NUM);
	if (err_alloc_mem(ini_info)) {
		TS_LOG_ERR("Failed to malloc ini_info\n");
		strncpy(ts_test_failed_reason,
			"tp_initial_failed-", ILITEK_TEST_FAILED_REASON_LEN - 1);
		ilitek_result_init(raw_info, apk);
		ret = -EMP_NOMEM;
		goto out;
	}

	ilitek_tddi_mp_init_item();
	ret = ilitek_test_get_ini_path();
	if (ret < 0) {
		TS_LOG_ERR("cannot get  INI file name\n");
		strncpy(ts_test_failed_reason,
			"software_reason-", ILITEK_TEST_FAILED_REASON_LEN - 1);
		ilitek_result_init(raw_info, apk);
		ret = -EMP_INI;
		goto out;
	}

	ret = ilitek_tddi_mp_ini_parser(ilits->ini_path);
	if (ret < 0) {
		TS_LOG_ERR("Failed to parsing INI file\n");
		strncpy(ts_test_failed_reason,
			"software_reason-", ILITEK_TEST_FAILED_REASON_LEN - 1);
		ilitek_result_init(raw_info, apk);
		ret = -EMP_INI;
		goto out;
	}

	/* Compare both protocol version of ini and firmware */
	parser_get_ini_key_value("pv5_4 command",
		"protocol", str, sizeof(str));
	snprintf(ver, sizeof(ver), "0x%s", str);
	if ((ili_str2hex(ver, ILITEK_MAX_STR_HEX_LEN)) != (core_mp.protocol_ver >> 8)) {
		TS_LOG_ERR("ERROR! MP Protocol version is invaild\n");
		ret = -EMP_PROTOCOL;
		strncpy(ts_test_failed_reason,
			"software_reason-", ILITEK_TEST_FAILED_REASON_LEN - 1);
		ilitek_result_init(raw_info, apk);
		goto out;
	}

	/* Read timing info from ini file */
	if (mp_get_timing_info() < 0) {
		TS_LOG_ERR("Failed to get timing info from ini\n");
		ret = -EMP_TIMING_INFO;
		strncpy(ts_test_failed_reason,
			"software_reason-", ILITEK_TEST_FAILED_REASON_LEN - 1);
		ilitek_result_init(raw_info, apk);

		goto out;
	}

#if MP_INT_LEVEL
	if (ili_ic_int_trigger_ctrl(true) < 0) {
		TS_LOG_ERR("Failed to set INT as Level trigger");
		ret = -EMP_CMD;
		strncpy(ts_test_failed_reason,
			"software_reason-", ILITEK_TEST_FAILED_REASON_LEN - 1);
		ilitek_result_init(raw_info, apk);
		goto out;
	}
#endif

	/* Sort test item by ini file */
	if (mp_sort_item(lcm_on) < 0) {
		TS_LOG_ERR("Failed to sort test item\n");
		ret = -EMP_INI;
		strncpy(ts_test_failed_reason,
			"software_reason-", ILITEK_TEST_FAILED_REASON_LEN - 1);
		ilitek_result_init(raw_info, apk);
		goto out;
	}

	for (i = 0; i < g_run_index.count; i++)
		mp_test_run(g_run_index.index[i]);

	ret = mp_show_result(raw_info, lcm_on);
	if (raw_info) {
		ilitek_result_generate(raw_info, apk);
	} else {
		mp_copy_ret_to_apk(apk);
	}

out:
	mp_test_free();

#if MP_INT_LEVEL
	if (ili_ic_int_trigger_ctrl(false) < 0) {
		TS_LOG_ERR("Failed to set INT back to pluse trigger");
		ret = -EMP_CMD;
	}
#endif

	ipio_vfree((void **)&ini_info);
	return ret;
};
