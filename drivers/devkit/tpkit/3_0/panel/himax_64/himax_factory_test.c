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

// #define HX_FAC_LOG_PRINT // debug dmesg rawdata test switch
#define HIMAX_PROC_FACTORY_TEST_FILE "ts/himax_threshold.csv" // "touchscreen/tp_capacitance_data"
#define hx_abs(x) (((x) < 0) ? -(x) : (x))
#define RESULT_LEN 100
#define IIR_DIAG_COMMAND 1
#define DC_DIAG_COMMAND 2
#define BANK_DIAG_COMMAND 3
#define BASEC_GOLDENBC_DIAG_COMMAND 5
#define CLOSE_DIAG_COMMAND 0
#define IIR_CMD 1
#define DC_CMD 2
#define BANK_CMD 3
#define GOLDEN_BASEC_CMD 5
#define BASEC_CMD 7
#define RTX_DELTA_CMD 8
#define RTX_DELTA_CMD1 11
#define OPEN_CMD 9
#define SHORT_CMD 10
#define HX_RAW_DUMP_FILE "/sdcard/hx_fac_dump.txt"
// use for parse  dts start
#define CAP_RADATA_LIMIT "himax,cap_rawdata_limit"
#define TRX_DELTA_LIMIT "himax,trx_delta_limit"
#define NOISE_LIMIT "himax,noise_limit"
#define OPEN_SHORT_LIMIT "himax,open_short_limit"
#define DOWN_LIMIT 0
#define UP_LIMIT 1
#define HX_RAW_DATA_SIZE (PAGE_SIZE * 60)
#define CSV_LIMIT_MAX_LEN 2
#define CSV_OPEN_SHORT_LIMIT_MAX_LEN 6
#define CSV_CAP_RADATA_LIMIT "cap_rawdata_limit"
#define CSV_TRX_DELTA_LIMIT "trx_delta_limit"
#define CSV_NOISE_LIMIT "noise_limit"
#define CSV_OPEN_SHORT_LIMIT "open_short_limit"
#define CSV_CAP_RADATA_LIMIT_UP "cap_rawdata_limit_up"
#define CSV_CAP_RADATA_LIMIT_DW "cap_rawdata_limit_dw"
#define CSV_TX_DELTA_LIMIT_UP "tx_delta_limit_up"
#define CSV_TX_DELTA_LIMIT_DW "tx_delta_limit_dw"
#define CSV_RX_DELTA_LIMIT_UP "rx_delta_limit_up"
#define CSV_RX_DELTA_LIMIT_DW "rx_delta_limit_dw"
#define CSV_OPEN_LIMIT_UP "open_limit_up"
#define CSV_OPEN_LIMIT_DW "open_limit_dw"
#define CSV_MICRO_OPEN_LIMIT_UP "micro_open_limit_up"
#define CSV_MICRO_OPEN_LIMIT_DW "micro_open_limit_dw"
#define CSV_SHORT_LIMIT_UP "short_limit_up"
#define CSV_SHORT_LIMIT_DW "short_limit_dw"

u32 cap_rawdata_limit[2] = {0};
u32 trx_delta_limit[2] = {0};
u32 noise_limit[2] = {0};
u32 open_short_limit[6] = {0};

u32 *p2p_cap_rawdata_limit_up = NULL;
u32 *p2p_cap_rawdata_limit_dw = NULL;

u32 *p2p_tx_delta_limit_up = NULL;
u32 *p2p_rx_delta_limit_up = NULL;

u32 *p2p_open_limit_up = NULL;
u32 *p2p_open_limit_dw = NULL;

u32 *p2p_micro_open_limit_up = NULL;
u32 *p2p_micro_open_limit_dw = NULL;

u32 *p2p_short_limit_up = NULL;
u32 *p2p_short_limit_dw = NULL;

u32 *p2p_noise_limit = NULL;

// use for parse  dts end
static int16_t rawdata_limit_row = 0;
static int16_t rawdata_limit_col = 0;
static int16_t rawdata_limit_os_cnt = 0;

static int16_t *mutual_iir = NULL;
static int16_t *self_iir = NULL;
static int16_t *tx_delta = NULL;
static int16_t *rx_delta = NULL;
static uint16_t *mutual_bank = NULL;
static uint16_t *self_bank = NULL;
// used for HX83112 start
static uint32_t *mutual_open = NULL;
static uint32_t *self_open = NULL;

static uint32_t *mutual_mopen = NULL;
static uint32_t *self_mopen = NULL;

static uint32_t *mutual_short = NULL;
static uint32_t *self_short = NULL;

#define	BS_OPENSHORT 0

// Himax MP Password
#define PWD_OPEN_START 0x77
#define PWD_OPEN_END 0x88
#define PWD_SHORT_START 0x11
#define PWD_SHORT_END 0x33
#define PWD_RAWDATA_START 0x00
#define PWD_RAWDATA_END 0x99
#define PWD_NOISE_START 0x00
#define PWD_NOISE_END 0x99
#define PWD_SORTING_START 0xAA
#define PWD_SORTING_END 0xCC

// Himax DataType
#define DATA_OPEN 0x0B
#define DATA_MICRO_OPEN 0x0C
#define DATA_SHORT 0x0A
#define DATA_BACK_NORMAL 0x00

// Himax Data Ready Password
#define DATA_PWD0 0xA5
#define DATA_PWD1 0x5A

typedef enum {
	HIMAX_INSPECTION_OPEN,
	HIMAX_INSPECTION_MICRO_OPEN,
	HIMAX_INSPECTION_SHORT,
	HIMAX_INSPECTION_SORTING,
	HIMAX_INSPECTION_BACK_NORMAL,
} thp_inspection_enum;

/* Error code of AFE Inspection */
typedef enum {
	HX_INSPECT_OK = 0, /* OK */
	HX_INSPECT_ESPI, /* SPI communication error */
	HX_INSPECT_EOPEN, /* Sensor open error */
	HX_INSPECT_EMOPEN, /* Sensor micro open error */
	HX_INSPECT_ESHORT, /* Sensor short error */
	HX_INSPECT_ERC, /* Sensor RC error */
	HX_INSPECT_EOTHER, /* All other errors */
} hx_inspect_err_enum;

// used for HX83112 end

static int current_index = 0;
static char hx_result_fail_str[4] = "0F-";
static char hx_result_pass_str[4] = "0P-";
static int hx_result_status[8] = {0};
static int16_t *fac_iir_dump_buffer = NULL;
static uint16_t *fac_dc_dump_buffer = NULL;
static char buf_test_result[RESULT_LEN] = {0}; /* store mmi test result */
atomic_t hmx_nc_mmi_test_status = ATOMIC_INIT(0);
struct ts_rawdata_info *info_test = NULL;
extern char himax_nc_project_id[];
extern struct himax_ts_data *g_himax_nc_ts_data;

enum hx_test_item {
	TEST0 = 0,
	TEST1,
	TEST2,
	TEST3,
	TEST4,
	TEST5,
	TEST6,
	TEST7,
};

enum hx_limit_index {
	BANK_SELF_UP = 0,
	BANK_SELF_DOWN,
	BANK_MUTUAL_UP,
	BANK_MUTUAL_DOWN,
	DC_SELF_UP,
	DC_SELF_DOWN,
	DC_MUTUAL_UP,
	DC_MUTUAL_DOWN,
	BASEC_SELF,
	BASEC_MUTUAL,
	IIR_SELF,
	IIR_MUTUAL,
	DELTA_UP,
	DELTA_DOWN,
	OPEN_UP,
	OPEN_DOWN,
	MICRO_OPEN_UP,
	MICRO_OPEN_DOWN,
	SHORT_UP,
	SHORT_DOWN,
};

struct get_csv_data {
	uint64_t size;
	int32_t csv_data[];
};

static void himax_free_rawmem(void);
extern bool himax_nc_sense_off(void);
extern void himax_nc_sense_on(uint8_t flashmode);
extern void himax_nc_flash_write_burst_length(uint8_t *reg_byte,
	uint8_t *write_data, int length);
extern void himax_nc_reload_disable(int on);

int himax_nc_chip_self_test(void)
{
	int i;
	int pf_value = 0x00;
	uint8_t tmp_addr[4] = {0};
	uint8_t tmp_data[128] = {0};
	uint8_t self_test_info[24] = {0};
	uint8_t test_result_id;

	himax_nc_interface_on();
	himax_nc_sense_off();
	himax_burst_enable(ON);
	himax_nc_reload_disable(ON);

	if (ic_nc_type == HX_83112A_SERIES_PWON) {
		// 0x10007F18 -> 0x00006AA6
		himax_rw_reg_reformat_com(ADDR_HAND_SHAKING_HX83112,
			DATA_HAND_SHAKING, tmp_addr, tmp_data);
	} else if (ic_nc_type == HX_83102B_SERIES_PWON) {
		// 0x100007F8 -> 0x00006AA6
		himax_rw_reg_reformat_com(ADDR_HAND_SHAKING_HX83102,
			DATA_HAND_SHAKING, tmp_addr, tmp_data);
	} else if (ic_nc_type == HX_83102E_SERIES_PWON) {
		himax_rw_reg_reformat_com(ADDR_HAND_SHAKING_HX83102E,
			DATA_HAND_SHAKING, tmp_addr, tmp_data);
	}
	himax_flash_write_burst(tmp_addr, tmp_data,
		sizeof(tmp_addr), sizeof(tmp_data));
	himax_rw_reg_reformat(ADDR_SET_CRITERIA, tmp_addr);
	tmp_data[0] = open_short_limit[UP_LIMIT];
	tmp_data[1] = open_short_limit[DOWN_LIMIT];
	tmp_data[2] = open_short_limit[UP_LIMIT + 2];
	tmp_data[3] = open_short_limit[DOWN_LIMIT + 2];
	tmp_data[4] = open_short_limit[UP_LIMIT + 4];
	tmp_data[5] = open_short_limit[DOWN_LIMIT + 4];
	tmp_data[6] = RESERVED_VALUE; // reserved
	tmp_data[7] = RESERVED_VALUE; // reserved
	himax_nc_flash_write_burst_length(tmp_addr, tmp_data, 2 * FOUR_BYTE_CMD);
	// 0x10007294 -> 0x0000190 SET IIR_MAX FRAMES
	himax_rw_reg_reformat_com(ADDR_NFRAME_SEL, DATA_SET_IIR_FRM,
		tmp_addr, tmp_data);
	himax_flash_write_burst(tmp_addr, tmp_data,
		sizeof(tmp_addr), sizeof(tmp_data));
	// Disable IDLE Mode
	himax_nc_idle_mode(ON);
	himax_rw_reg_reformat_com(ADDR_SWITCH_FLASH_RLD,
		DATA_DISABLE_FLASH_RLD, tmp_addr, tmp_data);
	himax_flash_write_burst(tmp_addr, tmp_data,
		sizeof(tmp_addr), sizeof(tmp_data));
	// start selftest leave safe mode
	himax_nc_sense_on(ON);
	// Hand shaking -> 0x100007F8 waiting 0xA66A
	for (i = 0; i < 10; i++) {
		if (ic_nc_type == HX_83112A_SERIES_PWON) {
			// 0x10007F18 -> 0x00006AA6
			himax_rw_reg_reformat(ADDR_HAND_SHAKING_HX83112, tmp_addr);
		} else if (ic_nc_type == HX_83102B_SERIES_PWON) {
			// 0x100007F8 -> 0x00006AA6
			himax_rw_reg_reformat(ADDR_HAND_SHAKING_HX83102, tmp_addr);
		} else if (ic_nc_type == HX_83102E_SERIES_PWON) {
			himax_rw_reg_reformat(ADDR_HAND_SHAKING_HX83102E,
				tmp_addr);
		}
		himax_nc_register_read(tmp_addr, FOUR_BYTE_CMD, tmp_data);
		TS_LOG_INFO("chip_self_test: tmp_data[0]=0x%02X,tmp_data[1]=0x%02X\n",
			tmp_data[0], tmp_data[1]);
		TS_LOG_INFO("chip_self_test: tmp_data[2]=0x%02X,tmp_data[3]=0x%02X,cnt=%d\n",
			tmp_data[2], tmp_data[3], i);
		msleep(HX_SLEEP_10MS);
		if (tmp_data[1] == 0xA6 && tmp_data[0] == 0x6A) {
			TS_LOG_INFO("chip_self_test: Data ready goto moving data\n");
			break;
		}
	}

	himax_nc_sense_off();
	msleep(HX_SLEEP_20MS);
	// =====================================
	// Read test result ID : 0x10007f24 ==> bit[2][1][0] = [key][AA][avg] => 0xF = PASS
	// =====================================
	himax_rw_reg_reformat(ADDR_OS_TEST_RESULT, tmp_addr);
	himax_nc_register_read(tmp_addr, 6 * FOUR_BYTE_CMD, self_test_info);
	test_result_id = self_test_info[0];
	TS_LOG_INFO("chip_self_test: check test result, test_result_id=%x, test_result=%x\n",
		test_result_id, self_test_info[0]);
	if (test_result_id == 0xAA) {
		TS_LOG_INFO("[Hx]: self-test pass\n");
		pf_value = 0x0;
	} else {
		TS_LOG_ERR("[PANEL_ISSUE]: self-test fail\n");
		pf_value = 0x1;
	}

	// Enable IDLE Mode
	himax_nc_idle_mode(OFF);
	himax_burst_enable(ON);
	himax_nc_reload_disable(OFF);
	himax_nc_sense_on(ON);
	msleep(HX_SLEEP_120MS);

	return pf_value;
}

void himax_fac_dc_dump(uint16_t mutual_num, uint16_t *mutual)
{
	uint16_t raw_dump_addr;

	raw_dump_addr = mutual_num;
	TS_LOG_INFO("fac_dc_dump: raw_dump_addr=%d\n", raw_dump_addr);
	memcpy(fac_dc_dump_buffer + raw_dump_addr, mutual, mutual_num);
}

void himax_fac_iir_dump(uint16_t mutual_num, int16_t *mutual)
{
	int16_t raw_dump_addr;

	raw_dump_addr = mutual_num;
	TS_LOG_INFO("fac_iir_dump: raw_dump_addr=%d\n", raw_dump_addr);
	memcpy(fac_iir_dump_buffer + raw_dump_addr, mutual, mutual_num);
}

static int himax_alloc_rawmem(void)
{
	uint16_t self_num;
	uint16_t mutual_num;
	uint16_t tx_delta_num;
	uint16_t rx_delta_num;
	uint16_t rx = hx_nc_get_x_channel();
	uint16_t tx = hx_nc_get_y_channel();

	mutual_num = rx * tx;
	self_num = rx + tx;
	tx_delta_num = rx * (tx - 1);
	rx_delta_num = (rx - 1) * tx;

	mutual_bank = kzalloc(mutual_num * sizeof(uint16_t), GFP_KERNEL);
	if (mutual_bank == NULL) {
		TS_LOG_ERR("alloc_rawmem: mutual_bank is NULL\n");
		goto exit_mutual_bank;
	}

	self_bank = kzalloc(self_num * sizeof(uint16_t), GFP_KERNEL);
	if (self_bank == NULL) {
		TS_LOG_ERR("alloc_rawmem: self_bank is NULL\n");
		goto exit_self_bank;
	}

	tx_delta = kzalloc(tx_delta_num * sizeof(int16_t), GFP_KERNEL);
	if (tx_delta == NULL) {
		TS_LOG_ERR("alloc_rawmem: tx_delta is NULL\n");
		goto exit_tx_delta;
	}

	rx_delta = kzalloc(rx_delta_num * sizeof(int16_t), GFP_KERNEL);
	if (rx_delta == NULL) {
		TS_LOG_ERR("alloc_rawmem: rx_delta is NULL\n");
		goto exit_rx_delta;
	}

	mutual_iir = kzalloc(mutual_num * sizeof(int16_t), GFP_KERNEL);
	if (mutual_iir == NULL) {
		TS_LOG_ERR("alloc_rawmem: mutual_iir is NULL\n");
		goto exit_mutual_iir;
	}
	self_iir = kzalloc(self_num * sizeof(int16_t), GFP_KERNEL);
	if (self_iir == NULL) {
		TS_LOG_ERR("alloc_rawmem: self_iir is NULL\n");
		goto exit_self_iir;
	}
	if ((ic_nc_type == HX_83112A_SERIES_PWON) ||
		(ic_nc_type == HX_83102E_SERIES_PWON)) {
		mutual_open = kzalloc(mutual_num * sizeof(uint32_t), GFP_KERNEL);
		if (mutual_open == NULL) {
			TS_LOG_ERR("alloc_rawmem: mutual_open is NULL\n");
			goto exit_mutual_open;
		}

		self_open = kzalloc(self_num * sizeof(uint32_t), GFP_KERNEL);
		if (self_open == NULL) {
			TS_LOG_ERR("alloc_rawmem: self_open is NULL\n");
			goto exit_self_open;
		}

		mutual_mopen = kzalloc(mutual_num * sizeof(uint32_t), GFP_KERNEL);
		if (mutual_mopen == NULL) {
			TS_LOG_ERR("alloc_rawmem: mutual_mopen is NULL\n");
			goto exit_mutual_mopen;
		}
		self_mopen = kzalloc(self_num * sizeof(uint32_t), GFP_KERNEL);
		if (self_mopen == NULL) {
			TS_LOG_ERR("alloc_rawmem: self_mopen is NULL\n");
			goto exit_self_mopen;
		}

		mutual_short = kzalloc(mutual_num * sizeof(uint32_t), GFP_KERNEL);
		if (mutual_short == NULL) {
			TS_LOG_ERR("alloc_rawmem: mutual_short is NULL\n");
			goto exit_mutual_short;
		}
		self_short = kzalloc(self_num * sizeof(uint32_t), GFP_KERNEL);
		if (self_short == NULL) {
			TS_LOG_ERR("alloc_rawmem: self_short is NULL\n");
			goto exit_self_short;
		}
	}

	memset(mutual_bank, 0xFF, mutual_num * sizeof(uint16_t));
	memset(self_bank, 0xFF, self_num * sizeof(uint16_t));
	memset(tx_delta, 0xFF, tx_delta_num * sizeof(int16_t));
	memset(rx_delta, 0xFF, rx_delta_num * sizeof(int16_t));
	memset(mutual_iir, 0xFF, mutual_num * sizeof(int16_t));
	memset(self_iir, 0xFF, self_num * sizeof(int16_t));

	if ((ic_nc_type == HX_83112A_SERIES_PWON) ||
		(ic_nc_type == HX_83102E_SERIES_PWON)) {
		memset(mutual_open, 0xFF, mutual_num * sizeof(uint32_t));
		memset(self_open, 0xFF, self_num * sizeof(uint32_t));
		memset(mutual_mopen, 0xFF, mutual_num * sizeof(uint32_t));
		memset(self_mopen, 0xFF, self_num * sizeof(uint32_t));
		memset(mutual_short, 0xFF, mutual_num * sizeof(uint32_t));
		memset(self_short, 0xFF, self_num * sizeof(uint32_t));
	}
	return NO_ERR;
	if (ic_nc_type == HX_83112A_SERIES_PWON) {
exit_self_short:
	kfree(mutual_short);
	mutual_short = NULL;
exit_mutual_short:
	kfree(self_mopen);
	self_mopen = NULL;
exit_self_mopen:
	kfree(mutual_mopen);
	mutual_mopen = NULL;
exit_mutual_mopen:
	kfree(self_open);
	self_open = NULL;
exit_self_open:
	kfree(mutual_open);
	mutual_open = NULL;
exit_mutual_open:
	kfree(self_iir);
	self_iir = NULL;
	}
exit_self_iir:
	kfree(mutual_iir);
	mutual_iir = NULL;
exit_mutual_iir:
	kfree(rx_delta);
	rx_delta = NULL;
exit_rx_delta:
	kfree(tx_delta);
	tx_delta = NULL;
exit_tx_delta:
	kfree(self_bank);
	self_bank = NULL;
exit_self_bank:
	kfree(mutual_bank);
	mutual_bank = NULL;
exit_mutual_bank:

	return ALLOC_FAIL;
}

static void himax_free_rawmem(void)
{
	kfree(mutual_bank);
	kfree(self_bank);
	kfree(tx_delta);
	kfree(rx_delta);
	kfree(mutual_iir);
	kfree(self_iir);

	if ((ic_nc_type == HX_83112A_SERIES_PWON) ||
		(ic_nc_type == HX_83102E_SERIES_PWON)) {
		kfree(mutual_open);
		kfree(self_open);
		kfree(mutual_mopen);
		kfree(self_mopen);
		kfree(mutual_short);
		kfree(self_short);

		mutual_open = NULL;
		self_open = NULL;
		mutual_mopen = NULL;
		self_mopen = NULL;
		mutual_short = NULL;
		self_short = NULL;
	}

	mutual_bank = NULL;
	self_bank = NULL;
	tx_delta = NULL;
	rx_delta = NULL;
	mutual_iir = NULL;
	self_iir = NULL;
}

static void himax_print_rawdata(int mode)
{
	int index1;
	uint16_t self_num;
	uint16_t mutual_num;
	uint16_t x_channel = hx_nc_get_x_channel();
	uint16_t y_channel = hx_nc_get_y_channel();
	char buf[MAX_CAP_DATA_SIZE] = {0};

	// check if devided by zero
	if ((x_channel == 0) || (x_channel == 1)) {
		TS_LOG_ERR("print_rawdata: devided by zero\n");
		return;
	}

	switch (mode) {
	case BANK_CMD:
		mutual_num = x_channel * y_channel;
		for (index1 = 0; index1 < mutual_num; index1++)
			info_test->buff[current_index++] =
				(int)mutual_bank[index1];
		break;
	case IIR_CMD:
		mutual_num = x_channel * y_channel;
		for (index1 = 0; index1 < mutual_num; index1++)
			info_test->buff[current_index++] =
				mutual_iir[index1];
		break;
	case RTX_DELTA_CMD:
		mutual_num = x_channel * (y_channel - DATA_1);
		self_num = (x_channel - DATA_1) * y_channel;
		for (index1 = 0; index1 < mutual_num; index1++) {
			snprintf(buf, sizeof(buf), "%5d,",
				tx_delta[index1]);
			strncat((char *)info_test->tx_delta_buf, buf,
				sizeof(buf));
			if ((index1 % (x_channel)) == (x_channel - DATA_1))
				strncat((char *)info_test->tx_delta_buf,
					"\n", DATA_1);
		}
		for (index1 = 0; index1 < self_num; index1++) {
			snprintf(buf, sizeof(buf), "%5d,",
				rx_delta[index1]);
			strncat((char *)info_test->rx_delta_buf,
				buf, sizeof(buf));
			if ((index1 % (x_channel - DATA_1)) ==
				(x_channel - DATA_2))
				strncat((char *)info_test->rx_delta_buf,
					"\n", DATA_1);
		}
		break;
	case RTX_DELTA_CMD1:
		mutual_num = x_channel * (y_channel - DATA_1);
		self_num = (x_channel - DATA_1) * y_channel;
		for (index1 = 0; index1 < self_num; index1++)
			info_test->rx_delta_buf[index1] =
				rx_delta[index1];

		for (index1 = 0; index1 < mutual_num; index1++)
			info_test->tx_delta_buf[index1] =
				tx_delta[index1];
		break;
	case OPEN_CMD:
		TS_LOG_INFO("print_rawdata: %s\n", "OPEN_CMD");
		mutual_num = x_channel * y_channel;
		TS_LOG_INFO("print_rawdata: x_ch = %d, y_ch = %d, mu_num = %d\n",
			x_channel, y_channel, mutual_num);
		for (index1 = 0; index1 < mutual_num; index1++)
			info_test->buff[current_index++] =
				mutual_open[index1];
		break;
	case SHORT_CMD:
		TS_LOG_INFO("print_rawdata: %s\n", "SHORT_CMD");
		mutual_num = x_channel * y_channel;
		for (index1 = 0; index1 < mutual_num; index1++)
			info_test->buff[current_index++] =
				mutual_short[index1];
		break;
	default:
		break;
	}
}

static int himax_interface_on_test(int step)
{
	int cnt = 0;
	int retval = NO_ERR;
	uint8_t tmp_data[5] = {0};
	uint8_t tmp_data2[2] = {0};

	TS_LOG_INFO("interface_on_test: start \n");
	himax_nc_int_enable(g_himax_nc_ts_data->tskit_himax_data->ts_platform_data->irq_id,
		IRQ_DISABLE);
	// Read a dummy register to wake up I2C.
	if (i2c_himax_nc_read(DUMMY_REGISTER, tmp_data,
		FOUR_BYTE_CMD, sizeof(tmp_data), DEFAULT_RETRY_CNT) < 0) { // to knock I2C
		TS_LOG_ERR("[SW_ISSUE] interface_on_test: i2c access fail\n");
		goto err_i2c;
	}

	do {
		// ===========================================
		// Enable continuous burst mode : 0x13 ==> 0x31
		// ===========================================
		tmp_data[0] = DATA_EN_BURST_MODE;
		if (i2c_himax_nc_write(ADDR_EN_BURST_MODE, tmp_data,
			ONE_BYTE_CMD, sizeof(tmp_data), DEFAULT_RETRY_CNT) < 0) {
			TS_LOG_ERR("[SW_ISSUE] interface_on_test: i2c access fail\n");
			goto err_i2c;
		}
		// ===========================================
		// AHB address auto +4		: 0x0D ==> 0x11
		// Do not AHB address auto +4 : 0x0D ==> 0x10
		// ===========================================
		tmp_data2[0] = DATA_AHB;
		if (i2c_himax_nc_write(ADDR_AHB, tmp_data2, ONE_BYTE_CMD,
			sizeof(tmp_data2), DEFAULT_RETRY_CNT) < 0) {
			TS_LOG_ERR("[SW_ISSUE]interface_on_test: i2c access fail\n");
			goto err_i2c;
		}

		// Check cmd
		i2c_himax_nc_read(ADDR_EN_BURST_MODE, tmp_data, ONE_BYTE_CMD,
			sizeof(tmp_data), DEFAULT_RETRY_CNT);
		i2c_himax_nc_read(ADDR_AHB, tmp_data2, ONE_BYTE_CMD,
			sizeof(tmp_data2), DEFAULT_RETRY_CNT);
		if (tmp_data[0] == DATA_EN_BURST_MODE && tmp_data2[0] == DATA_AHB) {
			break;
		}
		msleep(HX_SLEEP_1MS);
	} while (++cnt < 10);

	if (cnt > 0)
		TS_LOG_DEBUG("interface_on_test: Polling burst mode: %d times\n", cnt);

	hx_result_pass_str[0] = '0';
	TS_LOG_DEBUG("interface_on_test: I2C Test --> PASS\n");
	strncat(buf_test_result, hx_result_pass_str, strlen(hx_result_pass_str) + 1);
	himax_nc_int_enable(g_himax_nc_ts_data->tskit_himax_data->ts_platform_data->irq_id,
		IRQ_ENABLE);
	return retval;

err_i2c:
	TS_LOG_ERR("[SW_ISSUE] interface_on_test: Exit because there is I2C fail\n");
	himax_nc_int_enable(g_himax_nc_ts_data->tskit_himax_data->ts_platform_data->irq_id,
		IRQ_ENABLE);
	himax_free_rawmem();

	hx_result_fail_str[0] = '0';
	TS_LOG_DEBUG("[SW_ISSUE] interface_on_test: I2C Test --> fail\n");
	strncat(buf_test_result, hx_result_fail_str, strlen(hx_result_fail_str) + 1);
	return I2C_WORK_ERR;
}

int himax_raw_data_test(int step) // for Rawdara
{
	int i;
	int j;
	int new_data = 0;
	int result = NO_ERR;
	int rx = hx_nc_get_x_channel();
	int tx = hx_nc_get_y_channel();
	unsigned int index;

	uint8_t info_data_hx102b[MUTUL_NUM_HX83102 * 2] = {0};
	uint8_t info_data_hx112a[MUTUL_NUM_HX83112 * 2] = {0};
	uint8_t *info_data_hx83102e = NULL;

	if (ic_nc_type == HX_83102E_SERIES_PWON) {
		info_data_hx83102e = kcalloc(MUTUL_NUM_HX83102E * DATA_2,
			sizeof(uint8_t), GFP_KERNEL);
		if (!info_data_hx83102e) {
			TS_LOG_ERR("raw_data_test: fail to alloc info data mem\n");
			return HX_ERR;
		}
	}
	// check if devided by zero
	if (rx == 0) {
		TS_LOG_ERR("raw_data_test: devided by zero\n");
		return HX_ERR;
	}

	TS_LOG_DEBUG("raw_data_test: Get Raw Data Start\n");
	himax_nc_int_enable(g_himax_nc_ts_data->tskit_himax_data->ts_platform_data->irq_id,
		IRQ_DISABLE);

	if (ic_nc_type == HX_83102E_SERIES_PWON) {
		himax_nc_switch_mode(DATA_0);
	} else {
		himax_nc_sense_off();
		mdelay(HX_SLEEP_10MS);
		himax_nc_sense_on(ON);
	}

	himax_nc_diag_register_set(0x0A); /*===DC===*/
	himax_burst_enable(1);
	if (ic_nc_type == HX_83102B_SERIES_PWON)
		himax_nc_get_dsram_data(info_data_hx102b,
			sizeof(info_data_hx102b));
	else if (ic_nc_type == HX_83112A_SERIES_PWON)
		himax_nc_get_dsram_data(info_data_hx112a,
			sizeof(info_data_hx112a));
	else if (ic_nc_type == HX_83102E_SERIES_PWON)
		himax_nc_get_dsram_data(info_data_hx83102e,
			MUTUL_NUM_HX83102E * DATA_2);
	index = 0;
	for (i = 0; i < tx; i++) {
		for (j = 0; j < rx; j++) {
			if (ic_nc_type == HX_83102B_SERIES_PWON)
				new_data = (short)((info_data_hx102b[index + 1] << 8) |
					info_data_hx102b[index]);
			else if (ic_nc_type == HX_83112A_SERIES_PWON)
				new_data = (short)((info_data_hx112a[index + 1] << 8) |
					info_data_hx112a[index]);
			else if (ic_nc_type == HX_83102E_SERIES_PWON)
				new_data = (short)((info_data_hx83102e[index + DATA_1]
					<< DATA_8) | info_data_hx83102e[index]);
			mutual_bank[i * rx + j] = new_data;
			index += 2;
		}
	}

	himax_nc_int_enable(g_himax_nc_ts_data->tskit_himax_data->ts_platform_data->irq_id,
		IRQ_ENABLE);
	himax_nc_return_event_stack();
	himax_nc_diag_register_set(0x00);

	TS_LOG_DEBUG("Get Raw Data End:\n");
	for (i = 0; i < rx * tx; i++) {
		if (g_himax_nc_ts_data->p2p_test_sel) {
			cap_rawdata_limit[UP_LIMIT] = p2p_cap_rawdata_limit_up[i];
			cap_rawdata_limit[DOWN_LIMIT] = p2p_cap_rawdata_limit_dw[i];
		}
		if (mutual_bank[i] < cap_rawdata_limit[DOWN_LIMIT] ||
			mutual_bank[i] > cap_rawdata_limit[UP_LIMIT]) {
			TS_LOG_INFO("[PANEL_ISSUE] POS_RX:%3d, POS_TX:%3d, CAP_RAWDATA: %6d\n",
				(i % rx), (i / rx), mutual_bank[i]);
			TS_LOG_INFO("[PANEL_ISSUE] UP_LIMIT:%6d, DOWN_LIMIT: %6d\n",
				cap_rawdata_limit[UP_LIMIT], cap_rawdata_limit[DOWN_LIMIT]);
			result = HX_ERR;
		}
	}
	TS_LOG_DEBUG("Raw Data test End\n");

	if (result == 0 && hx_result_status[TEST0] == 0) {
		hx_result_pass_str[0] = '0' + step;
		strncat(buf_test_result, hx_result_pass_str,
			strlen(hx_result_pass_str) + 1);
		TS_LOG_INFO("raw_data_test: End --> PASS\n");
		result = NO_ERR; // use for test
	} else {
		hx_result_fail_str[0] = '0' + step;
		strncat(buf_test_result, hx_result_fail_str,
			strlen(hx_result_fail_str) + 1);
		TS_LOG_INFO("[PANEL_ISSUE] raw_data_test: End --> FAIL\n");
	}
	if (ic_nc_type == HX_83102E_SERIES_PWON) {
		kfree(info_data_hx83102e);
		info_data_hx83102e = NULL;
	}

	return result;
}

static int himax_self_delta_test(int step)
{
	int m;
	int index1;
	int index2 = 0;
	int result = NO_ERR;
	int tx = hx_nc_get_y_channel();
	int rx = hx_nc_get_x_channel();
	uint16_t tx_delta_num;
	uint16_t rx_delta_num;

	// check if divided by zero
	if ((rx == 0) || (rx == 1)) {
		TS_LOG_ERR("print_rawdata: divided by zero\n");
		return HX_ERR;
	}

	tx_delta_num = rx * (tx - 1);
	rx_delta_num = (rx - 1) * tx;
	/* TX Delta */
	TS_LOG_INFO("TX Delta Start:\n");
	for (index1 = 0; index1 < tx_delta_num; index1++) {
		m = index1 + rx;
		tx_delta[index1] = hx_abs(mutual_bank[m] - mutual_bank[index1]);
		if (g_himax_nc_ts_data->p2p_test_sel)
			trx_delta_limit[UP_LIMIT] = p2p_tx_delta_limit_up[index1];
		if (tx_delta[index1] > trx_delta_limit[UP_LIMIT]) {
			TS_LOG_INFO("[PANEL_ISSUE] POS_RX:%3d, POS_TX:%3d, TX_DELTA: %6d\n",
				(index1 % rx), (index1 / rx), tx_delta[index1]);
			TS_LOG_INFO("[PANEL_ISSUE] UP_LIMIT:%6d\n",
				trx_delta_limit[UP_LIMIT]);
			result = HX_ERR;
		}
	}
	TS_LOG_INFO("TX Delta End\n");

	/* RX Delta */
	TS_LOG_INFO("TX RX Delta Start:\n");
	/*lint -save -e* */
	for (index1 = 1; index2 < rx_delta_num; index1++) {
		if (index1 % (rx) == 0)
			continue;
		rx_delta[index2] = hx_abs(mutual_bank[index1] -
			mutual_bank[index1 - 1]);
		if (g_himax_nc_ts_data->p2p_test_sel)
			trx_delta_limit[UP_LIMIT] = p2p_rx_delta_limit_up[index2];
		if (rx_delta[index2] > trx_delta_limit[UP_LIMIT]) {
			TS_LOG_INFO("[PANEL_ISSUE] POS_RX:%3d, POS_TX:%3d, RX_DELTA: %6d\n",
				(index1 % (rx - 1)), (index1 / (rx - 1)), rx_delta[index2]);
			TS_LOG_INFO("[PANEL_ISSUE] UP_LIMIT:%6d\n", trx_delta_limit[UP_LIMIT]);
			result = HX_ERR;
		}
		index2++;
	}
	/* lint -restore */
	TS_LOG_INFO("TX RX Delta End\n");
	if (result == 0 && hx_result_status[TEST0] == 0) {
		hx_result_pass_str[0] = '0' + step;
		strncat(buf_test_result, hx_result_pass_str,
			strlen(hx_result_pass_str) + 1);
		TS_LOG_INFO("self_delta_test: End --> PASS\n");
		result = NO_ERR; // use for test
	} else {
		hx_result_fail_str[0] = '0' + step;
		strncat(buf_test_result, hx_result_fail_str,
			strlen(hx_result_fail_str) + 1);
		TS_LOG_INFO("[PANEL_ISSUE] self_delta_test: End --> FAIL\n");
	}

	return result;
}

static int himax_iir_test(int step) // for Noise Delta
{
	int i;
	int j;
	int new_data = 0;
	int result = NO_ERR;
	int tx = hx_nc_get_y_channel();
	int rx = hx_nc_get_x_channel();
	unsigned int index;

	uint8_t info_data_hx102b[MUTUL_NUM_HX83102 * 2] = {0};
	uint8_t info_data_hx112a[MUTUL_NUM_HX83112 * 2] = {0};
	uint8_t *info_data_hx83102e = NULL;

	if (ic_nc_type == HX_83102E_SERIES_PWON) {
		info_data_hx83102e = kcalloc(MUTUL_NUM_HX83102E * DATA_2,
			sizeof(uint8_t), GFP_KERNEL);
		if (!info_data_hx83102e) {
			TS_LOG_ERR("fail to alloc info data mem\n");
			return HX_ERR;
		}
	}
	// check if devided by zero
	if (rx == 0) {
		TS_LOG_ERR("iir_testt: devided by zero\n");
		return HX_ERR;
	}

	TS_LOG_INFO("iir_test: Entering\n");
	himax_nc_int_enable(g_himax_nc_ts_data->tskit_himax_data->ts_platform_data->irq_id,
		IRQ_DISABLE);
	himax_nc_switch_mode(0);
	himax_nc_diag_register_set(0x0F); /* ===IIR=== */
	himax_burst_enable(1);
	if (ic_nc_type == HX_83102B_SERIES_PWON)
		himax_nc_get_dsram_data(info_data_hx102b,
			sizeof(info_data_hx102b));
	else if (ic_nc_type == HX_83112A_SERIES_PWON)
		himax_nc_get_dsram_data(info_data_hx112a,
			sizeof(info_data_hx112a));
	else if (ic_nc_type == HX_83102E_SERIES_PWON)
		himax_nc_get_dsram_data(info_data_hx83102e,
			MUTUL_NUM_HX83102E * DATA_2);

	for (i = 0, index = 0; i < tx; i++) {
		for (j = 0; j < rx; j++) {
			if (ic_nc_type == HX_83102B_SERIES_PWON)
				new_data = (short)((info_data_hx102b[index + 1]
					<< 8) | info_data_hx102b[index]);
			else if(ic_nc_type == HX_83112A_SERIES_PWON)
				new_data = (short)((info_data_hx112a[index + 1]
					<< 8) | info_data_hx112a[index]);
			else if (ic_nc_type == HX_83102E_SERIES_PWON)
				new_data = (short)((info_data_hx83102e[index + DATA_1]
				<< DATA_8) | info_data_hx83102e[index]);
			mutual_iir[i * rx + j] = new_data;
			index += 2;
		}
	}
	himax_nc_int_enable(g_himax_nc_ts_data->tskit_himax_data->ts_platform_data->irq_id,
		IRQ_ENABLE);
	himax_nc_return_event_stack();
	himax_nc_diag_register_set(0x00);

	TS_LOG_INFO("IIR Start:\n");
	for (i = 0; i < rx * tx; i++) {
		if (g_himax_nc_ts_data->p2p_test_sel)
			noise_limit[UP_LIMIT] = p2p_noise_limit[i];
		if (mutual_iir[i] > noise_limit[UP_LIMIT]) {
			TS_LOG_INFO("[PANEL_ISSUE] POS_RX:%3d, POS_TX:%3d, NOISE : %6d\n",
				(i % rx), (i / rx), mutual_iir[i]);
			TS_LOG_INFO("[PANEL_ISSUE] UP_LIMIT:%6d\n", noise_limit[UP_LIMIT]);
			result = -1;
		}

	}
	TS_LOG_INFO("IIR  End\n");

	if (result == 0 && hx_result_status[TEST0] == 0) {
		hx_result_pass_str[0] = '0' + step;
		strncat(buf_test_result, hx_result_pass_str,
			strlen(hx_result_pass_str) + 1);
		TS_LOG_INFO("iir_test: End --> PASS\n");
	} else {
		hx_result_fail_str[0] = '0' + step;
		strncat(buf_test_result, hx_result_fail_str,
			strlen(hx_result_fail_str) + 1);
		TS_LOG_INFO("[PANEL_ISSUE] iir_test: End --> FAIL\n");
	}
	if (ic_nc_type == HX_83102E_SERIES_PWON) {
		kfree(info_data_hx83102e);
		info_data_hx83102e = NULL;
	}

	return result;

}

/* use for HX83112 start */
/* inspection api start */
int himax_switch_mode_inspection(int mode)
{
	uint8_t tmp_addr[4] = {0};
	uint8_t tmp_data[4] = {0};

	TS_LOG_INFO("switch_mode_inspection: Entering\n");
	// Stop Handshaking
	himax_rw_reg_reformat_com(ADDR_STP_HNDSHKG, DATA_INIT,
		tmp_addr, tmp_data);
	himax_nc_flash_write_burst_length(tmp_addr, tmp_data, FOUR_BYTE_CMD);
	/* Swtich Mode */
	himax_rw_reg_reformat(ADDR_MODE_SWITCH_HX83112, tmp_addr);
	switch (mode) {
	case HIMAX_INSPECTION_SORTING:
		tmp_data[3] = 0x00;
		tmp_data[2] = 0x00;
		tmp_data[1] = PWD_SORTING_START;
		tmp_data[0] = PWD_SORTING_START;
		break;
	case HIMAX_INSPECTION_OPEN:
		tmp_data[3] = 0x00;
		tmp_data[2] = 0x00;
		tmp_data[1] = PWD_OPEN_START;
		tmp_data[0] = PWD_OPEN_START;
		break;
	case HIMAX_INSPECTION_MICRO_OPEN:
		tmp_data[3] = 0x00;
		tmp_data[2] = 0x00;
		tmp_data[1] = PWD_OPEN_START;
		tmp_data[0] = PWD_OPEN_START;
		break;
	case HIMAX_INSPECTION_SHORT:
		tmp_data[3] = 0x00;
		tmp_data[2] = 0x00;
		tmp_data[1] = PWD_SHORT_START;
		tmp_data[0] = PWD_SHORT_START;
		break;
	}
	himax_nc_flash_write_burst_length(tmp_addr, tmp_data, FOUR_BYTE_CMD);
	TS_LOG_INFO("switch_mode_inspection: End of setting\n");

	return NO_ERR;
}

uint32_t himax_get_rawdata_func(uint32_t *raw, uint32_t datalen)
{
	uint32_t address;
	int total_read_times;
	int total_size_temp;
	int nml_retry_cnt = 200;
	int max_retry_cnt = 300;
	int total_size = datalen * 2 + 4;

	uint8_t retry = 0;
	uint8_t tmp_addr[4] = {0};
	uint8_t tmp_data[4] = {0};
	uint8_t *tmp_rawdata = NULL;
	uint8_t max_i2c_size = NOR_READ_LENGTH;
	uint16_t checksum_cal;

	uint32_t i;
	uint32_t j;
	uint32_t index = 0;
	uint32_t min_data = 0xFFFFFFFF;
	uint32_t max_data = 0x00000000;

	tmp_rawdata = kzalloc(sizeof(uint8_t) * (datalen * 2), GFP_KERNEL);
	if (tmp_rawdata == NULL) {
		TS_LOG_INFO("get_rawdata_func: Memory allocate fail!\n");
		return HX_ERR;
	}

	/* 1 Set Data Ready PWD */
	while (retry < max_retry_cnt) {
		himax_rw_reg_reformat(ADDR_STP_HNDSHKG, tmp_addr);
		tmp_data[3] = 0x00;
		tmp_data[2] = 0x00;
		tmp_data[1] = DATA_PWD1;
		tmp_data[0] = DATA_PWD0;
		himax_nc_flash_write_burst_length(tmp_addr, tmp_data, FOUR_BYTE_CMD);
		himax_nc_register_read(tmp_addr, FOUR_BYTE_CMD, tmp_data);
		if ((tmp_data[0] == DATA_PWD0 && tmp_data[1] == DATA_PWD1) ||
			(tmp_data[0] == DATA_PWD1 && tmp_data[1] == DATA_PWD0)) {
			break;
		}
		retry++;
		msleep(HX_SLEEP_1MS);
	}

	if (retry >= nml_retry_cnt) {
		kfree(tmp_rawdata);
		return FW_NOT_READY;
	} else {
		retry = 0;
	}

	while (retry < nml_retry_cnt) {
		if (tmp_data[0] == DATA_PWD1 && tmp_data[1] == DATA_PWD0) {
			break;
		}
		retry++;
		msleep(HX_SLEEP_1MS);
		himax_nc_register_read(tmp_addr, FOUR_BYTE_CMD, tmp_data);
	}

	if (retry >= nml_retry_cnt) {
		kfree(tmp_rawdata);
		return FW_NOT_READY;
	} else {
		retry = 0;
	}

	/* 2 Read Data from SRAM */
	while (retry < 10) {
		checksum_cal = 0;
		total_size_temp = total_size;
		himax_rw_reg_reformat(ADDR_STP_HNDSHKG, tmp_addr);
		if (total_size % max_i2c_size == 0) {
			total_read_times = total_size / max_i2c_size;
		} else {
			total_read_times = total_size / max_i2c_size + 1;
		}

		for (i = 0; i < (total_read_times); i++) {
			if (total_size_temp >= max_i2c_size) {
				himax_nc_register_read(tmp_addr, max_i2c_size,
					&tmp_rawdata[i * max_i2c_size]);
				total_size_temp = total_size_temp - max_i2c_size;
			} else {
				TS_LOG_DEBUG("last total_size_temp=%d\n", total_size_temp);
				himax_nc_register_read(tmp_addr, total_size_temp % max_i2c_size,
					&tmp_rawdata[i * max_i2c_size]);
			}

			address = ((i + 1) * max_i2c_size);
			tmp_addr[1] = (uint8_t)((address >> 8) & 0x00FF);
			tmp_addr[0] = (uint8_t)((address) & 0x00FF);
		}

		/* 3 Check Checksum */
		for (i = 2; i < datalen * 2 + 4; i = i + 2)
			checksum_cal += tmp_rawdata[i + 1] *
				MAX_READ_LENGTH + tmp_rawdata[i];

		if (checksum_cal == NO_ERR) {
			break;
		}
		retry++;
	}

	if (checksum_cal != 0) {
		TS_LOG_ERR("get_rawdata_func: Get rawdata checksum fail\n");
		kfree(tmp_rawdata);
		return HX_ERR;
	}

	/* 4 Copy Data */
	for (i = 0; i < hx_nc_tx_num * hx_nc_rx_num; i++)
		raw[i] = tmp_rawdata[(i * 2) + 1 + 4] * 256 +
			tmp_rawdata[(i * 2) + 4];

	kfree(tmp_rawdata);

	for (i = 0; i < hx_nc_tx_num; i++) {
		for (j = 0; j < hx_nc_rx_num; j++) {
			if (raw[index] > max_data)
				max_data = raw[index];

			if (raw[index] < min_data)
				min_data = raw[index];
			index++;
		}
	}
	TS_LOG_INFO("Max = %5d, Min = %5d \n", max_data, min_data);

	return HX_INSPECT_OK;
}

void himax_switch_data_type(uint8_t checktype)
{
	uint8_t datatype = 0;

	switch (checktype) {
	case HIMAX_INSPECTION_OPEN:
		datatype = DATA_OPEN;
		break;
	case HIMAX_INSPECTION_MICRO_OPEN:
		datatype = DATA_MICRO_OPEN;
		break;
	case HIMAX_INSPECTION_SHORT:
		datatype = DATA_SHORT;
		break;
	case HIMAX_INSPECTION_BACK_NORMAL:
		datatype = DATA_BACK_NORMAL;
		break;
	default:
		TS_LOG_ERR("Wrong type=%d\n", checktype);
		break;
	}
	himax_nc_diag_register_set(datatype);
}

void himax_set_n_frame(uint16_t nframe, uint8_t checktype)
{
	uint8_t tmp_addr[4] = {0};
	uint8_t tmp_data[4] = {0};

	// IIR MAX
	himax_rw_reg_reformat(ADDR_NFRAME_SEL, tmp_addr);
	tmp_data[3] = 0x00;
	tmp_data[2] = 0x00;
	tmp_data[1] = (uint8_t)((nframe & 0xFF00) >> 8);
	tmp_data[0] = (uint8_t)(nframe & 0x00FF);
	himax_nc_flash_write_burst_length(tmp_addr, tmp_data, FOUR_BYTE_CMD);

	// skip frame
	himax_rw_reg_reformat(ADDR_TXRX_INFO, tmp_addr);
	himax_nc_register_read(tmp_addr, FOUR_BYTE_CMD, tmp_data);
	tmp_data[0] = BS_OPENSHORT;
	himax_nc_flash_write_burst_length(tmp_addr, tmp_data, FOUR_BYTE_CMD);
}

uint32_t himax_check_mode(uint8_t checktype)
{
	uint8_t tmp_addr[4] = {0};
	uint8_t tmp_data[4] = {0};
	uint8_t wait_pwd[2] = {0};

	switch (checktype) {
	case HIMAX_INSPECTION_OPEN:
		wait_pwd[0] = PWD_OPEN_END;
		wait_pwd[1] = PWD_OPEN_END;
		break;
	case HIMAX_INSPECTION_MICRO_OPEN:
		wait_pwd[0] = PWD_OPEN_END;
		wait_pwd[1] = PWD_OPEN_END;
		break;
	case HIMAX_INSPECTION_SHORT:
		wait_pwd[0] = PWD_SHORT_END;
		wait_pwd[1] = PWD_SHORT_END;
		break;
	default:
		TS_LOG_ERR("Wrong type=%d\n", checktype);
		break;
	}

	himax_rw_reg_reformat(ADDR_MODE_SWITCH_HX83112, tmp_addr);
	himax_nc_register_read(tmp_addr, FOUR_BYTE_CMD, tmp_data);
	TS_LOG_INFO("check_mode: himax_wait_sorting_mode, tmp_data[0]=%x,tmp_data[1]=%x\n",
		tmp_data[0], tmp_data[1]);

	if (wait_pwd[0] == tmp_data[0] && wait_pwd[1] == tmp_data[1]) {
		TS_LOG_INFO("Change to mode=%d\n",
			himax_switch_mode_inspection(checktype));
		return !MODE_ND_CHNG;
	} else {
		return MODE_ND_CHNG;
	}
}
uint32_t himax_wait_sorting_mode(uint8_t checktype)
{
	uint8_t tmp_addr[4] = {0};
	uint8_t tmp_data[4] = {0};
	uint8_t wait_pwd[2] = {0};
	int count = 0;

	switch (checktype) {
	case HIMAX_INSPECTION_OPEN:
		wait_pwd[0] = PWD_OPEN_END;
		wait_pwd[1] = PWD_OPEN_END;
		break;
	case HIMAX_INSPECTION_MICRO_OPEN:
		wait_pwd[0] = PWD_OPEN_END;
		wait_pwd[1] = PWD_OPEN_END;
		break;
	case HIMAX_INSPECTION_SHORT:
		wait_pwd[0] = PWD_SHORT_END;
		wait_pwd[1] = PWD_SHORT_END;
		break;
	default:
		TS_LOG_ERR("Wrong type=%d\n", checktype);
		break;
	}

	do {
		himax_rw_reg_reformat(ADDR_MODE_SWITCH_HX83112, tmp_addr);
		himax_nc_register_read(tmp_addr, FOUR_BYTE_CMD, tmp_data);
		TS_LOG_INFO("wait_sorting_mode: himax_wait_sorting_mode, tmp_data[0]=%x,tmp_data[1]=%x\n",
			tmp_data[0], tmp_data[1]);
		if (wait_pwd[0] == tmp_data[0] && wait_pwd[1] == tmp_data[1]) {
			return NO_ERR;
		}

		himax_rw_reg_reformat(ADDR_READ_MODE_CHK, tmp_addr);
		himax_nc_register_read(tmp_addr, FOUR_BYTE_CMD, tmp_data);
		TS_LOG_INFO("wait_sorting_mode: 0x900000A8, tmp_data[0]=%x,tmp_data[1]=%x,tmp_data[2]=%x,tmp_data[3]=%x \n",
			tmp_data[0], tmp_data[1], tmp_data[2], tmp_data[3]);

		himax_rw_reg_reformat(ADDR_HW_STST_CHK, tmp_addr);
		himax_nc_register_read(tmp_addr, FOUR_BYTE_CMD, tmp_data);
		TS_LOG_INFO("wait_sorting_mode: 0x900000E4, tmp_data[0]=%x,tmp_data[1]=%x,tmp_data[2]=%x,tmp_data[3]=%x \n",
			tmp_data[0], tmp_data[1], tmp_data[2], tmp_data[3]);

		himax_rw_reg_reformat(ADDR_READ_REG_FW_STATUS, tmp_addr);
		himax_nc_register_read(tmp_addr, FOUR_BYTE_CMD, tmp_data);
		TS_LOG_INFO("wait_sorting_mode: 0x900000F8, tmp_data[0]=%x,tmp_data[1]=%x,tmp_data[2]=%x,tmp_data[3]=%x \n",
			tmp_data[0], tmp_data[1], tmp_data[2], tmp_data[3]);
		TS_LOG_INFO("Now retry %d times!\n", count++);
		msleep(HX_SLEEP_50MS);
	} while (count < 200);

	return FW_NOT_READY;
}
uint32_t mp_test_func(uint8_t checktype, uint32_t datalen)
{
	uint32_t i;
	uint32_t ret;
	uint32_t *raw = NULL;

	raw = kzalloc(datalen * sizeof(uint32_t), GFP_KERNEL);
	if (raw == NULL) {
		TS_LOG_ERR("%s:raw is NULL\n", __func__);
		return ALLC_MEM_ERR;
	}
	if (himax_check_mode(checktype)) {
		TS_LOG_INFO("Need Change Mode, target=%d",
			himax_switch_mode_inspection(checktype));
		himax_nc_sense_off();
		himax_nc_reload_disable(ON);
		himax_switch_mode_inspection(checktype);
		himax_set_n_frame(2, checktype);
		himax_nc_sense_on(0x01);
		ret = himax_wait_sorting_mode(checktype);
		if (ret) {
			TS_LOG_ERR("%s: himax_wait_sorting_mode FAIL\n", __func__);
			goto ERR_EXIT;
		}
	}
	himax_switch_data_type(checktype);
	ret = himax_get_rawdata_func(raw, datalen);
	if (ret) {
		TS_LOG_ERR("%s: himax_get_rawdata_func FAIL\n", __func__);
		goto ERR_EXIT;
	}

	/* back to normal */
	himax_switch_data_type(HIMAX_INSPECTION_BACK_NORMAL);
	// Check Data
	switch (checktype) {
	case HIMAX_INSPECTION_OPEN:
		memcpy(mutual_open, raw,
			sizeof(uint32_t) * (hx_nc_tx_num * hx_nc_rx_num));
		for (i = 0; i < (hx_nc_tx_num * hx_nc_rx_num); i++) {
			if (g_himax_nc_ts_data->p2p_test_sel) {
				open_short_limit[UP_LIMIT] = p2p_open_limit_up[i];
				open_short_limit[DOWN_LIMIT] = p2p_open_limit_dw[i];
			}
			if (raw[i] > open_short_limit[UP_LIMIT] || raw[i] <
				open_short_limit[DOWN_LIMIT]) {
				TS_LOG_ERR("%s: open test FAIL\n", __func__);
				TS_LOG_INFO("[PANEL_ISSUE] POS_RX:%3d, POS_TX:%3d, CAP_RAWDATA: %6d\n",
					(i % hx_nc_rx_num), (i / hx_nc_rx_num), raw[i]);
				TS_LOG_INFO("[PANEL_ISSUE] UP_LIMIT:%6d, DOWN_LIMIT: %6d\n",
					open_short_limit[UP_LIMIT], open_short_limit[DOWN_LIMIT]);
				ret = HX_INSPECT_EOPEN;
				goto ERR_EXIT;
			}
		}
		TS_LOG_INFO("%s: open test PASS\n", __func__);
		break;
	case HIMAX_INSPECTION_MICRO_OPEN:
		memcpy(mutual_mopen, raw,
			sizeof(uint32_t) * (hx_nc_tx_num * hx_nc_rx_num));
		for (i = 0; i < (hx_nc_tx_num * hx_nc_rx_num); i++) {
			if (g_himax_nc_ts_data->p2p_test_sel) {
				open_short_limit[UP_LIMIT + 2] = p2p_micro_open_limit_up[i];
				open_short_limit[DOWN_LIMIT + 2] = p2p_micro_open_limit_dw[i];
			}
			if (raw[i] > open_short_limit[UP_LIMIT + 2] ||
				raw[i] < open_short_limit[DOWN_LIMIT + 2]) {
				TS_LOG_INFO("[PANEL_ISSUE] POS_RX:%3d, POS_TX:%3d, CAP_RAWDATA: %6d\n",
					(i % hx_nc_rx_num), (i / hx_nc_rx_num), raw[i]);
				TS_LOG_INFO("[PANEL_ISSUE] UP_LIMIT:%6d, DOWN_LIMIT: %6d\n",
					open_short_limit[UP_LIMIT + 2], open_short_limit[DOWN_LIMIT + 2]);
				TS_LOG_ERR("%s: micro open test FAIL\n", __func__);
				ret = HX_INSPECT_EMOPEN;
				goto ERR_EXIT;
			}
		}
		TS_LOG_INFO("%s: micro open test PASS\n", __func__);
		break;
	case HIMAX_INSPECTION_SHORT:
		memcpy(mutual_short, raw,
			sizeof(uint32_t) * (hx_nc_tx_num * hx_nc_rx_num));
		for (i = 0; i < (hx_nc_tx_num * hx_nc_rx_num); i++) {
			if (g_himax_nc_ts_data->p2p_test_sel) {
				open_short_limit[UP_LIMIT + 4] = p2p_short_limit_up[i];
				open_short_limit[DOWN_LIMIT + 4] = p2p_short_limit_dw[i];
			}
			if (raw[i] > open_short_limit[UP_LIMIT + 4] || raw[i] <
				open_short_limit[DOWN_LIMIT + 4]) {
				TS_LOG_INFO("[PANEL_ISSUE] POS_RX:%3d, POS_TX:%3d, CAP_RAWDATA: %6d\n",
					(i % hx_nc_rx_num), (i / hx_nc_rx_num), raw[i]);
				TS_LOG_INFO("[PANEL_ISSUE] UP_LIMIT:%6d, DOWN_LIMIT: %6d\n",
					open_short_limit[UP_LIMIT + 4], open_short_limit[DOWN_LIMIT + 4]);
				TS_LOG_ERR("%s: short test FAIL\n", __func__);
				ret = HX_INSPECT_ESHORT;
				goto ERR_EXIT;

			}
		}
		TS_LOG_INFO("%s: short test PASS\n", __func__);
		break;
	default:
		TS_LOG_ERR("Wrong type=%d\n", checktype);
		break;
	}
	ret = NO_ERR;
ERR_EXIT:
	kfree(raw);
	raw = NULL;
	return ret;
}

/* inspection api end */

int himax_open_test(int step)
{
	int retval = NO_ERR;
	TS_LOG_INFO("Step=%d,[MP_OPEN_TEST]\n", step);
	retval += mp_test_func(HIMAX_INSPECTION_OPEN, (hx_nc_tx_num * hx_nc_rx_num) +
		hx_nc_tx_num + hx_nc_rx_num);

	if (retval == 0 && hx_result_status[TEST0] == 0) {
		hx_result_pass_str[0] = '0' + step;
		strncat(buf_test_result, hx_result_pass_str,
			strlen(hx_result_pass_str) + 1);
		TS_LOG_INFO("open_test: End --> PASS\n");
	} else {
		hx_result_fail_str[0] = '0' + step;
		strncat(buf_test_result, hx_result_fail_str,
			strlen(hx_result_fail_str) + 1);
		TS_LOG_INFO("open_test: End --> FAIL\n");
	}

	return retval;
}

int himax_micro_open_test(int step)
{
	int retval = NO_ERR;
	TS_LOG_INFO("Step=%d,[MP_MICRO_OPEN_TEST]\n", step);
	retval += mp_test_func(HIMAX_INSPECTION_MICRO_OPEN,
		(hx_nc_tx_num * hx_nc_rx_num) + hx_nc_tx_num + hx_nc_rx_num);

	if (retval == 0 && hx_result_status[TEST0] == 0) {
		hx_result_pass_str[0] = '0' + step;
		strncat(buf_test_result, hx_result_pass_str,
			strlen(hx_result_pass_str) + 1);
		TS_LOG_INFO("micro_open_test: End --> PASS\n");
	} else {
		hx_result_fail_str[0] = '0' + step;
		strncat(buf_test_result, hx_result_fail_str,
			strlen(hx_result_fail_str) + 1);
		TS_LOG_INFO("micro_open_test: End --> FAIL\n");
	}

	return retval;
}

int himax_short_test(int step)
{
	int retval = NO_ERR;
	TS_LOG_INFO("Step=%d,[MP_SHORT_TEST]\n", step);
	retval += mp_test_func(HIMAX_INSPECTION_SHORT, (hx_nc_tx_num * hx_nc_rx_num)
		+ hx_nc_tx_num + hx_nc_rx_num);

	if (retval == 0 && hx_result_status[TEST0] == 0) {
		hx_result_pass_str[0] = '0' + step;
		strncat(buf_test_result, hx_result_pass_str,
			strlen(hx_result_pass_str) + 1);
		TS_LOG_INFO("short_test: End --> PASS\n");
	} else {
		hx_result_fail_str[0] = '0' + step;
		strncat(buf_test_result, hx_result_fail_str,
			strlen(hx_result_fail_str) + 1);
		TS_LOG_INFO("short_test: End --> FAIL\n");
	}

	return retval;
}
/* use for HX83112 end */

static int himax_basec_test(int step) // for open/short
{
	int retval;

	himax_nc_int_enable(g_himax_nc_ts_data->tskit_himax_data->ts_platform_data->irq_id,
		IRQ_DISABLE); // disable irq
	retval = himax_nc_chip_self_test();
	himax_nc_int_enable(g_himax_nc_ts_data->tskit_himax_data->ts_platform_data->irq_id,
		IRQ_ENABLE); // enable irq

	if ((retval == 0) && (hx_result_status[TEST0] == 0)) {
		hx_result_pass_str[0] = '0' + step;
		strncat(buf_test_result, hx_result_pass_str,
			strlen(hx_result_pass_str) + 1);
		TS_LOG_INFO("basec_test: End --> PASS\n");
	} else {
		hx_result_fail_str[0] = '0' + step;
		strncat(buf_test_result, hx_result_fail_str,
			strlen(hx_result_fail_str) + 1);
		TS_LOG_INFO("basec_test: End --> FAIL\n");
	}

	return retval;
}

static int himax_get_threshold_from_csvfile(int columns, int rows,
	char* target_name, struct get_csv_data *data)
{
	char file_path[100] = {0};
	char file_name[64] = {0};
	int ret;
	int result;

	TS_LOG_INFO("get_threshold_from_csvfile: called\n");

	if (!data || !target_name || columns * rows > data->size) {
		TS_LOG_ERR("parse csvfile failed: data or target_name is NULL\n");
		return HX_ERROR;
	}

	snprintf(file_name, sizeof(file_name), "%s_%s_%s_%s_raw.csv",
			g_himax_nc_ts_data->tskit_himax_data->ts_platform_data->product_name,
			g_himax_nc_ts_data->tskit_himax_data->ts_platform_data->chip_data->chip_name,
			himax_nc_project_id,
			g_himax_nc_ts_data->tskit_himax_data->ts_platform_data->chip_data->module_name);

	snprintf(file_path, sizeof(file_path),
		"/odm/etc/firmware/ts/%s", file_name);
	TS_LOG_INFO("threshold file name:%s, rows_size=%d, columns_size=%d, target_name = %s\n",
		file_path, rows, columns, target_name);
	result =  ts_kit_parse_csvfile(file_path, target_name,
		data->csv_data, rows, columns);
	if (result == HX_OK) {
		ret = HX_OK;
		TS_LOG_INFO("Get threshold successed form csvfile\n");
	} else {
		TS_LOG_INFO("csv file parse fail:%s\n", file_path);
		ret = HX_ERROR;
	}
	return ret;
}

int himax_parse_threshold_csvfile(void)
{
	int retval = 0;
	struct get_csv_data *rawdata_limit = NULL;

	rawdata_limit = kzalloc(8 * sizeof(int32_t) +
		sizeof(struct get_csv_data), GFP_KERNEL);
	if (rawdata_limit == NULL) {
		TS_LOG_ERR("rawdata_limit alloc memory failed\n");
		return HX_ERR;
	}

	rawdata_limit->size = CSV_LIMIT_MAX_LEN;

	if (HX_ERROR == himax_get_threshold_from_csvfile(2, 1,
		CSV_CAP_RADATA_LIMIT, rawdata_limit)) {
		TS_LOG_ERR("parse_threshold_csvfile:get threshold from csvfile failed\n");
		retval = -1;
		goto exit;
	} else {
		memcpy(cap_rawdata_limit, rawdata_limit->csv_data, 2 * sizeof(u32));
	}
	TS_LOG_INFO("parse_threshold_csvfile:cap_rawdata_limit[%d] = %d, cap_rawdata_limit[%d] = %d \n",
		DOWN_LIMIT, cap_rawdata_limit[0], UP_LIMIT, cap_rawdata_limit[1]);

	if (HX_ERROR == himax_get_threshold_from_csvfile(2, 1,
		CSV_TRX_DELTA_LIMIT, rawdata_limit)) {
		TS_LOG_ERR("parse_threshold_csvfile:get threshold from csvfile failed\n");
		retval = -1;
		goto exit;
	} else {
		memcpy(trx_delta_limit, rawdata_limit->csv_data, 2 * sizeof(u32));
	}
	TS_LOG_INFO("parse_threshold_csvfile: trx_delta_limit[%d] = %d, trx_delta_limit[%d] = %d\n",
		DOWN_LIMIT, trx_delta_limit[0], UP_LIMIT, trx_delta_limit[1]);

	if (HX_ERROR == himax_get_threshold_from_csvfile(2, 1,
		CSV_NOISE_LIMIT, rawdata_limit)) {
		TS_LOG_ERR("parse_threshold_csvfile: get threshold from csvfile failed\n");
		retval = -1;
		goto exit;
	} else {
		memcpy(noise_limit, rawdata_limit->csv_data, 2 * sizeof(u32));
	}
	TS_LOG_INFO("parse_threshold_csvfile: noise_limit[%d] = %d, noise_limit[%d] = %d \n",
		DOWN_LIMIT, noise_limit[0], UP_LIMIT, noise_limit[1]);

	rawdata_limit->size = CSV_OPEN_SHORT_LIMIT_MAX_LEN;
	if (HX_ERROR == himax_get_threshold_from_csvfile(6, 1,
		CSV_OPEN_SHORT_LIMIT, rawdata_limit)) {
		TS_LOG_ERR("parse_threshold_csvfile: get threshold from csvfile failed\n");
		retval = -1;
		goto exit;
	} else {
		memcpy(open_short_limit, rawdata_limit->csv_data, 6 * sizeof(u32));
	}

	TS_LOG_DEBUG("parse_threshold_csvfile: open_short_limit_aa_down[%d] = %d, open_short_limit_aa_up[%d] = %d\n",
		DOWN_LIMIT, open_short_limit[DOWN_LIMIT], UP_LIMIT, open_short_limit[UP_LIMIT]);
	TS_LOG_DEBUG("parse_threshold_csvfile: open_short_limit_key_down[%d] = %d, open_short_limit_key_up[%d] = %d\n",
		DOWN_LIMIT + 2, open_short_limit[DOWN_LIMIT + 2], UP_LIMIT + 2, open_short_limit[UP_LIMIT + 2]);
	TS_LOG_DEBUG("parse_threshold_csvfile: open_short_limit_avg_down[%d] = %d, open_short_limit_avg_up[%d] = %d\n",
		DOWN_LIMIT + 4, open_short_limit[DOWN_LIMIT + 4], UP_LIMIT + 4, open_short_limit[UP_LIMIT + 4]);

exit:
	kfree(rawdata_limit);
	rawdata_limit = NULL;
	TS_LOG_INFO("parse_threshold_csvfile: --\n");

	return retval;
}

int himax_parse_threshold_csvfile_p2p(void)
{
	int retval = 0;
	struct get_csv_data *rawdata_limit = NULL;
	rawdata_limit = kzalloc(rawdata_limit_col * rawdata_limit_row *
		sizeof(int32_t) + sizeof(struct get_csv_data), GFP_KERNEL);
	if (rawdata_limit == NULL) {
		TS_LOG_ERR("parse_csvfile_p2p: rawdata_limit alloc memory failed\n");
		return HX_ERR;
	}
	rawdata_limit->size = rawdata_limit_col * rawdata_limit_row;
	/*	1	*/
	if (HX_ERROR == himax_get_threshold_from_csvfile(rawdata_limit_col,
		rawdata_limit_row, CSV_CAP_RADATA_LIMIT_UP, rawdata_limit)) {
		TS_LOG_ERR("parse_csvfile_p2p:get threshold from csvfile failed\n");
		retval = -1;
		goto exit;
	} else {
		memcpy(p2p_cap_rawdata_limit_up, rawdata_limit->csv_data,
			rawdata_limit_col * rawdata_limit_row * sizeof(int32_t));
	}
	/*	2	*/
	if (HX_ERROR == himax_get_threshold_from_csvfile(rawdata_limit_col,
		rawdata_limit_row, CSV_CAP_RADATA_LIMIT_DW, rawdata_limit)) {
		TS_LOG_ERR("parse_csvfile_p2p:get threshold from csvfile failed\n");
		retval = -1;
		goto exit;
	} else {
		memcpy(p2p_cap_rawdata_limit_dw, rawdata_limit->csv_data,
			rawdata_limit_col * rawdata_limit_row * sizeof(int32_t));
	}
	/*	3	*/
	rawdata_limit->size = rawdata_limit_col * (rawdata_limit_row - 1);
	if (HX_ERROR == himax_get_threshold_from_csvfile(rawdata_limit_col,
		(rawdata_limit_row - 1), CSV_TX_DELTA_LIMIT_UP, rawdata_limit)) {
		TS_LOG_ERR("parse_csvfile_p2p:get threshold from csvfile failed\n");
		retval = -1;
		goto exit;
	} else {
		memcpy(p2p_tx_delta_limit_up, rawdata_limit->csv_data,
			rawdata_limit_col * (rawdata_limit_row - 1) * sizeof(int32_t));
	}
	/*	4	*/
	rawdata_limit->size = (rawdata_limit_col - 1) * rawdata_limit_row;
	if (HX_ERROR == himax_get_threshold_from_csvfile((rawdata_limit_col - 1),
		rawdata_limit_row, CSV_RX_DELTA_LIMIT_UP, rawdata_limit)) {
		TS_LOG_ERR("parse_csvfile_p2p:get threshold from csvfile failed\n");
		retval = -1;
		goto exit;
	} else {
		memcpy(p2p_rx_delta_limit_up, rawdata_limit->csv_data,
			(rawdata_limit_col - 1) * rawdata_limit_row * sizeof(int32_t));
	}
	/*	5	*/
	rawdata_limit->size = rawdata_limit_col * rawdata_limit_row;
	if (HX_ERROR == himax_get_threshold_from_csvfile(rawdata_limit_col,
		rawdata_limit_row, CSV_OPEN_LIMIT_UP, rawdata_limit)) {
		TS_LOG_ERR("parse_csvfile_p2p:get threshold from csvfile failed\n");
		retval = -1;
		goto exit;
	} else {
		memcpy(p2p_open_limit_up, rawdata_limit->csv_data,
			rawdata_limit_col * rawdata_limit_row * sizeof(int32_t));
	}
	/*	6	*/
	if (HX_ERROR == himax_get_threshold_from_csvfile(rawdata_limit_col,
		rawdata_limit_row, CSV_OPEN_LIMIT_DW, rawdata_limit)) {
		TS_LOG_ERR("parse_csvfile_p2p:get threshold from csvfile failed\n");
		retval = -1;
		goto exit;
	} else {
		memcpy(p2p_open_limit_dw, rawdata_limit->csv_data,
			rawdata_limit_col * rawdata_limit_row * sizeof(int32_t));
	}
	/*	7	*/
	if (HX_ERROR == himax_get_threshold_from_csvfile(rawdata_limit_col,
		rawdata_limit_row, CSV_MICRO_OPEN_LIMIT_UP, rawdata_limit)) {
		TS_LOG_ERR("parse_csvfile_p2p:get threshold from csvfile failed\n");
		retval = -1;
		goto exit;
	} else {
		memcpy(p2p_micro_open_limit_up, rawdata_limit->csv_data,
			rawdata_limit_col * rawdata_limit_row * sizeof(int32_t));
	}
	/*	8	*/
	if (HX_ERROR == himax_get_threshold_from_csvfile(rawdata_limit_col,
		rawdata_limit_row, CSV_MICRO_OPEN_LIMIT_DW, rawdata_limit)) {
		TS_LOG_ERR("parse_csvfile_p2p:get threshold from csvfile failed\n");
		retval = -1;
		goto exit;
	} else {
		memcpy (p2p_micro_open_limit_dw, rawdata_limit->csv_data,
			rawdata_limit_col * rawdata_limit_row * sizeof(int32_t));
	}
	/*	9	*/
	if (HX_ERROR == himax_get_threshold_from_csvfile(rawdata_limit_col,
		rawdata_limit_row, CSV_SHORT_LIMIT_UP, rawdata_limit)) {
		TS_LOG_ERR("parse_csvfile_p2p:get threshold from csvfile failed\n");
		retval = -1;
		goto exit;
	} else {
			memcpy(p2p_short_limit_up, rawdata_limit->csv_data,
				rawdata_limit_col * rawdata_limit_row * sizeof(int32_t));
	}
	/*	10	*/
	if (HX_ERROR == himax_get_threshold_from_csvfile(rawdata_limit_col,
		rawdata_limit_row, CSV_SHORT_LIMIT_DW, rawdata_limit)) {
		TS_LOG_ERR("parse_csvfile_p2p:get threshold from csvfile failed\n");
		retval = -1;
		goto exit;
	} else {
			memcpy (p2p_short_limit_dw, rawdata_limit->csv_data,
				rawdata_limit_col * rawdata_limit_row * sizeof(int32_t));
	}
	/*	11	*/
	if (HX_ERROR == himax_get_threshold_from_csvfile(rawdata_limit_col,
		rawdata_limit_row, CSV_NOISE_LIMIT, rawdata_limit)) {
		TS_LOG_ERR("parse_csvfile_p2p:get threshold from csvfile failed\n");
		retval = -1;
		goto exit;
	} else {
		memcpy(p2p_noise_limit, rawdata_limit->csv_data,
			rawdata_limit_col * rawdata_limit_row * sizeof(int32_t));
	}

exit:
	if (rawdata_limit)
		kfree(rawdata_limit);
	TS_LOG_INFO("parse_csvfile_p2p: --\n");

	return retval;
}

int himax_parse_threshold_dts(void)
{
	int retval;
	struct device_node *device;

	struct himax_ts_data *ts_cap = g_himax_nc_ts_data;
	device = ts_cap->ts_dev->dev.of_node;

	retval = of_property_read_u32_array(device, CAP_RADATA_LIMIT,
		&cap_rawdata_limit[0], 2);
	if (retval < 0) {
		TS_LOG_ERR("Failed to read limits from dt:retval = %d \n", retval);
		return retval;
	}
	TS_LOG_INFO("parse_threshold_dts:cap_rawdata_limit[%d] = %d, cap_rawdata_limit[%d] = %d \n",
		DOWN_LIMIT, cap_rawdata_limit[0], UP_LIMIT, cap_rawdata_limit[1]);

	retval = of_property_read_u32_array(device, TRX_DELTA_LIMIT,
		&trx_delta_limit[0], 2);
	if (retval < 0) {
		TS_LOG_ERR("Failed to read limits from dt:retval =%d\n", retval);
		return retval;
	}
	TS_LOG_INFO("parse_threshold_dts:trx_delta_limit[%d] = %d, trx_delta_limit[%d] = %d \n",
		DOWN_LIMIT, trx_delta_limit[0], UP_LIMIT, trx_delta_limit[1]);

	retval = of_property_read_u32_array(device, NOISE_LIMIT, &noise_limit[0], 2);
	if (retval < 0) {
		TS_LOG_ERR("Failed to read limits from dt:retval = %d \n", retval);
		return retval;
	}
	TS_LOG_INFO("parse_threshold_dts:noise_limit[%d] = %d, noise_limit[%d] = %d \n",
		DOWN_LIMIT, noise_limit[0], UP_LIMIT, noise_limit[1]);

	retval = of_property_read_u32_array(device, OPEN_SHORT_LIMIT, &open_short_limit[0], 6);
	if (retval < 0) {
		TS_LOG_ERR("Failed to read limits from dt:retval = %d\n", retval);
		return retval;
	}
	TS_LOG_INFO("parse_threshold_dts:open_short_limit_aa_down[%d] = %d, open_short_limit_aa_up[%d] = %d\n",
		DOWN_LIMIT, open_short_limit[DOWN_LIMIT], UP_LIMIT, open_short_limit[UP_LIMIT]);
	TS_LOG_INFO("parse_threshold_dts:open_short_limit_key_down[%d] = %d, open_short_limit_key_up[%d] = %d\n",
		DOWN_LIMIT + 2, open_short_limit[DOWN_LIMIT + 2], UP_LIMIT + 2, open_short_limit[UP_LIMIT + 2]);
	TS_LOG_INFO("parse_threshold_dts:open_short_limit_avg_down[%d] = %d, open_short_limit_avg_up[%d] = %d\n",
		DOWN_LIMIT + 4, open_short_limit[DOWN_LIMIT + 4], UP_LIMIT + 4, open_short_limit[UP_LIMIT + 4]);
	return retval;
}

static void himax_free(u32 *limit)
{
	TS_LOG_INFO("free: enter\n");

	if (limit)
		kfree(limit);
	limit = NULL;

}

static void himax_p2p_test_deinit(void)
{
	himax_free(p2p_cap_rawdata_limit_up);
	himax_free(p2p_cap_rawdata_limit_dw);
	himax_free(p2p_open_limit_up);
	himax_free(p2p_open_limit_dw);
	himax_free(p2p_micro_open_limit_up);
	himax_free(p2p_micro_open_limit_dw);
	himax_free(p2p_short_limit_up);
	himax_free(p2p_short_limit_dw);
	himax_free(p2p_tx_delta_limit_up);
	himax_free(p2p_rx_delta_limit_up);
	himax_free(p2p_noise_limit);

}

static int himax_p2p_test_init(void)
{
	int p2p_test_sel = g_himax_nc_ts_data->p2p_test_sel;

	if (p2p_test_sel) {
		rawdata_limit_col = hx_nc_rx_num;
		rawdata_limit_row = hx_nc_tx_num;
		p2p_cap_rawdata_limit_up = kzalloc(rawdata_limit_col *
			rawdata_limit_row * sizeof(u32), GFP_KERNEL);
		p2p_cap_rawdata_limit_dw = kzalloc(rawdata_limit_col *
			rawdata_limit_row * sizeof(u32), GFP_KERNEL);
		p2p_tx_delta_limit_up = kzalloc(rawdata_limit_col *
			(rawdata_limit_row - 1) * sizeof(u32), GFP_KERNEL);
		p2p_rx_delta_limit_up = kzalloc((rawdata_limit_col - 1) *
			rawdata_limit_row * sizeof(u32), GFP_KERNEL);
		p2p_open_limit_up = kzalloc(rawdata_limit_col *
			rawdata_limit_row * sizeof(u32), GFP_KERNEL);
		p2p_open_limit_dw = kzalloc(rawdata_limit_col *
			rawdata_limit_row * sizeof(u32), GFP_KERNEL);
		p2p_micro_open_limit_up = kzalloc(rawdata_limit_col *
			rawdata_limit_row * sizeof(u32), GFP_KERNEL);
		p2p_micro_open_limit_dw = kzalloc(rawdata_limit_col *
			rawdata_limit_row * sizeof(u32), GFP_KERNEL);
		p2p_short_limit_up = kzalloc(rawdata_limit_col *
			rawdata_limit_row * sizeof(u32), GFP_KERNEL);
		p2p_short_limit_dw = kzalloc(rawdata_limit_col *
			rawdata_limit_row * sizeof(u32), GFP_KERNEL);
		p2p_noise_limit = kzalloc(rawdata_limit_col *
			rawdata_limit_row * sizeof(u32), GFP_KERNEL);

		if (!p2p_cap_rawdata_limit_up ||
			!p2p_cap_rawdata_limit_dw ||
			!p2p_tx_delta_limit_up ||
			!p2p_rx_delta_limit_up ||
			!p2p_open_limit_up ||
			!p2p_open_limit_dw ||
			!p2p_micro_open_limit_up ||
			!p2p_micro_open_limit_dw ||
			!p2p_short_limit_up ||
			!p2p_short_limit_dw ||
			!p2p_noise_limit) {
			TS_LOG_ERR("p2p_test_init: alloc mem fail! p2p_test_sel = %d \n",
				p2p_test_sel);
			return HX_ERROR;
		}
	} else {
		rawdata_limit_col = 2;
		rawdata_limit_row = 1;
		rawdata_limit_os_cnt = 6;
	}

	return NO_ERR;
}

int himax_nc_factory_start(struct himax_ts_data *ts,
	struct ts_rawdata_info *info_top)
{
	int retval;
	uint16_t self_num;
	uint16_t mutual_num;
	unsigned long timer_start;
	unsigned long timer_end;
	uint16_t rx;
	uint16_t tx;

	timer_start = jiffies;
	rx = hx_nc_get_x_channel();
	tx = hx_nc_get_y_channel();
	// struct file *fn;
	mutual_num = rx * tx;
	self_num = rx + tx;
	retval = himax_p2p_test_init();
	if (!!retval)
		goto err_alloc_p2p_test;

	fac_dc_dump_buffer = kzalloc(mutual_num * 2 *
		sizeof(uint16_t), GFP_KERNEL);
	if (!fac_dc_dump_buffer) {
		TS_LOG_ERR("[SW_ISSUE] device, fac_dc_dump_buffer is NULL\n");
		goto err_alloc_p2p_test;
	}
	fac_iir_dump_buffer = kzalloc(mutual_num * 2 * sizeof(int16_t),
		GFP_KERNEL);
	if (!fac_iir_dump_buffer) {
		TS_LOG_ERR("[SW_ISSUE] device, fac_iir_dump_buffer is NULL\n");
		retval = HX_ERROR;
		goto err_alloc_fac_iir;
	}

	TS_LOG_DEBUG("factory_start:Entering\n");
	info_test = info_top;
	/* init */
	if (ts->test_capacitance_via_csvfile == true) {
			if (g_himax_nc_ts_data->p2p_test_sel)
				retval = himax_parse_threshold_csvfile_p2p();
			else
				retval = himax_parse_threshold_csvfile();
			if (retval < 0)
				TS_LOG_ERR("[SW_ISSUE] factory_start: Parse csvfile limit Fail\n");
		} else {
			retval = himax_parse_threshold_dts();
			if (retval < 0)
				TS_LOG_ERR("[SW_ISSUE] factory_start: Parse dts limit Fail\n");
		}

	if (atomic_read(&hmx_nc_mmi_test_status)) {
		TS_LOG_ERR("[SW_ISSUE] factory_start: factory test already has been called\n");
		retval = FACTORY_RUNNING;
		goto err_atomic_read;
	}

	atomic_set(&hmx_nc_mmi_test_status, 1);
	memset(buf_test_result, 0, RESULT_LEN);
	TS_LOG_DEBUG("himax_self_test enter\n");
	__pm_stay_awake(&g_himax_nc_ts_data->tskit_himax_data->ts_platform_data->ts_wake_lock);

	retval = himax_alloc_rawmem();
	if (retval != 0) {
		TS_LOG_ERR("[SW_ISSUE] factory_start factory test alloc_Rawmem failed\n");
		goto err_alloc_rawmem;
	}

	/* step0: himax self test */
	hx_result_status[TEST0] = himax_interface_on_test(TEST0); // 0 :i2c test pass  -1: i2c test fail
	/* step1: cap rawdata */
	hx_result_status[TEST1] = himax_raw_data_test(TEST1); // for Rawdata
	himax_fac_dc_dump(mutual_num, mutual_bank);// self_bank);use for test
	/* step2: TX RX Delta */
	hx_result_status[TEST2] = himax_self_delta_test(TEST2); // for TRX delta
	/* step3: Noise Delta */
	hx_result_status[TEST3] = himax_iir_test(TEST3); // for Noise Delta
	himax_fac_iir_dump(mutual_num, mutual_iir);// self_iir); use for test
	/* step4: Open/Short */
	if (ic_nc_type == HX_83102B_SERIES_PWON) {
		hx_result_status[TEST4] = himax_basec_test(TEST4); // for short/open
	} else if (ic_nc_type == HX_83102E_SERIES_PWON) {
		hx_result_status[TEST4] = himax_open_test(TEST4);
		hx_result_status[TEST5] = himax_short_test(TEST5);
	} else if (ic_nc_type == HX_83112A_SERIES_PWON) {
		hx_result_status[TEST4] = himax_open_test(TEST4);
		hx_result_status[TEST5] = himax_short_test(TEST5);
	}

	// =============Show test result===================
	strncat(buf_test_result, STR_IC_VENDOR, strlen(STR_IC_VENDOR) + 1);
	strncat(buf_test_result, "-", strlen("-") + 1);
	strncat(buf_test_result,
		ts->tskit_himax_data->ts_platform_data->chip_data->chip_name,
		strlen(ts->tskit_himax_data->ts_platform_data->chip_data->chip_name)
		+ 1);
	strncat(buf_test_result, "-", strlen("-") + 1);
	strncat(buf_test_result,
		ts->tskit_himax_data->ts_platform_data->product_name,
		strlen(ts->tskit_himax_data->ts_platform_data->product_name) + 1);
	strncat(buf_test_result, ";", strlen(";") + 1);

	strncat(info_test->result, buf_test_result, strlen(buf_test_result) + 1);
	info_test->buff[0] = rx;
	info_test->buff[1] = tx;

	/* print basec and dc */
	current_index = 2;
	if (ic_nc_type == HX_83102E_SERIES_PWON) {
		himax_print_rawdata(BANK_CMD);
		himax_print_rawdata(IIR_CMD);
		himax_print_rawdata(OPEN_CMD);
		himax_print_rawdata(SHORT_CMD);
		himax_print_rawdata(RTX_DELTA_CMD1);
	} else {
		himax_print_rawdata(BANK_CMD);
		himax_print_rawdata(IIR_CMD);
		himax_print_rawdata(RTX_DELTA_CMD);
	}

	info_test->used_size = current_index;
	himax_free_rawmem();
	if (ic_nc_type == HX_83102E_SERIES_PWON)
		himax_nc_switch_mode(DATA_0);
	himax_nc_hw_reset(HX_LOADCONFIG_EN, HX_INT_DISABLE);

err_alloc_rawmem:
	__pm_relax(&g_himax_nc_ts_data->tskit_himax_data->ts_platform_data->ts_wake_lock);
	atomic_set(&hmx_nc_mmi_test_status, 0);

err_atomic_read:
	kfree(fac_iir_dump_buffer);
	fac_iir_dump_buffer = NULL;

err_alloc_fac_iir:
	kfree(fac_dc_dump_buffer);
	fac_dc_dump_buffer = NULL;
err_alloc_p2p_test:
	himax_p2p_test_deinit();
	timer_end = jiffies;
	TS_LOG_INFO("factory_start: self test time:%d\n",
		jiffies_to_msecs(timer_end - timer_start));
	return retval;
}
