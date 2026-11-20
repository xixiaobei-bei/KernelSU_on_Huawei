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

//#define HX_FAC_LOG_PRINT	//debug dmesg rawdata test switch
#define HIMAX_PROC_FACTORY_TEST_FILE	"ts/himax_threshold.csv" //"touchscreen/tp_capacitance_data"

#define HIMAX_MPAP_UPDATE_FW  "Himax_mpfirmware.bin"
char *himax_mpap_update_fw;

#define ABS(x)	(((x) < 0) ? -(x) : (x))

#define RESULT_LEN  100

#define IIR_DIAG_COMMAND   1
#define DC_DIAG_COMMAND     2
#define BANK_DIAG_COMMAND   3
#define BASEC_GOLDENBC_DIAG_COMMAND  5
#define CLOSE_DIAG_COMMAND  0

#define IIR_CMD   1
#define DC_CMD        2
#define BANK_CMD   3
#define GOLDEN_BASEC_CMD  5
#define BASEC_CMD  7
#define RTX_DELTA_CMD  8
#define RTX_DELTA_CMD1 11
#define OPEN_CMD 9
#define SHORT_CMD 10
#define MICRO_OPEN_CMD 12
#define HX_RAW_DUMP_FILE "/sdcard/hx_fac_dump.txt"

//use for parse  dts start
#define CAP_RADATA_LIMIT "himax,cap_rawdata_limit"
#define TRX_DELTA_LIMIT "himax,trx_delta_limit"
#define NOISE_LIMIT "himax,noise_limit"
#define OPEN_SHORT_LIMIT "himax,open_short_limit"
#define DOWN_LIMIT 0
#define UP_LIMIT 1
#define HX_RAW_DATA_SIZE   (PAGE_SIZE * 60)

#define CSV_LIMIT_MAX_LEN 2
#define CSV_OPEN_SHORT_LIMIT_MAX_LEN 6

#define CSV_CAP_RADATA_LIMIT "cap_rawdata_limit"
#define CSV_TRX_DELTA_LIMIT "trx_delta_limit"
#define CSV_NOISE_LIMIT_UP "noise_limi_up"
#define CSV_NOISE_LIMIT_DOWN "noise_limi_down"

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

static int32_t cap_rawdata_limit[2] = {0};
static int32_t trx_delta_limit[2] = {0};
static int32_t noise_limit[2] = {0};
static int32_t open_short_limit[8] = {0};

static int32_t *p2p_cap_rawdata_limit_up;
static int32_t *p2p_cap_rawdata_limit_dw;
static int32_t *p2p_tx_delta_limit_up;
static int32_t *p2p_rx_delta_limit_up;
static int32_t *p2p_open_limit_up;
static int32_t *p2p_open_limit_dw;
static int32_t *p2p_micro_open_limit_up;
static int32_t *p2p_micro_open_limit_dw;
static int32_t *p2p_short_limit_up;
static int32_t *p2p_short_limit_dw;
static int32_t *p2p_noise_limit_up;
static int32_t *p2p_noise_limit_down;

//use for parse  dts end
static int32_t rawdata_limit_row;
static int32_t rawdata_limit_col;
static int32_t rawdata_limit_os_cnt;
static int32_t *mutual_iir;
static int32_t *self_iir;
static int32_t *tx_delta;
static int32_t *rx_delta;
static int32_t *mutual_bank;
static int32_t *self_bank;
static int32_t *mutual_open;
static int32_t *self_open;
static int32_t *mutual_mopen;
static int32_t *self_mopen;
static int32_t *mutual_short;
static int32_t *self_short;

static uint16_t NOISEMAX;
static uint16_t g_recal_thx;
#define	BS_OPENSHORT		0

//Himax MP Password
#define	PWD_OPEN_START		0x77
#define	PWD_OPEN_END		0x88
#define	PWD_SHORT_START		0x11
#define	PWD_SHORT_END		0x33
#define	PWD_RAWDATA_START	0x00
#define	PWD_RAWDATA_END		0x99
#define	PWD_NOISE_START		0x00
#define	PWD_NOISE_END		0x99
#define	PWD_SORTING_START	0xAA
#define	PWD_SORTING_END		0xCC

//Himax DataType
#define	DATA_OPEN	0x0B
#define	DATA_MICRO_OPEN	0x0C
#define	DATA_SHORT	0x0A
#define	DATA_BACK_NORMAL	0x00
#define DATA_NOISE 0x0F

//Himax Data Ready Password
#define	Data_PWD0	0xA5
#define	Data_PWD1	0x5A

typedef enum {
	HIMAX_INSPECTION_OPEN,
	HIMAX_INSPECTION_MICRO_OPEN,
	HIMAX_INSPECTION_SHORT,
	HX_WT_NOISE,
	HIMAX_INSPECTION_SORTING,
	HIMAX_INSPECTION_BACK_NORMAL,
} THP_INSPECTION_ENUM;

/* Error code of AFE Inspection */
typedef enum {
	HX_INSPECT_OK      = 0,               /* OK */
	HX_INSPECT_ESPI,        /* SPI communication error */
	HX_INSPECT_EOPEN,        /* Sensor open error */
	HX_INSPECT_EMOPEN,        /* Sensor micro open error */
	HX_INSPECT_ESHORT,        /* Sensor short error */
	HX_INSPECT_ERC,        /* Sensor RC error */
	HX_INSPECT_EOTHER,     /* All other errors */
} HX_INSPECT_ERR_ENUM;

static int current_index;
static char hx_result_fail_str[4] = "0F-";
static char hx_result_pass_str[4] = "0P-";
static int hx_result_status[8] = {0};
static char buf_test_result[RESULT_LEN] = { 0 };	/*store mmi test result*/
atomic_t hmx_zf_mmi_test_status = ATOMIC_INIT(0);
static struct ts_rawdata_info *info_test;
extern char himax_zf_project_id[];
extern struct himax_ts_data *g_himax_zf_ts_data;
extern char himax_firmware_name[64];

enum hx_test_item {
	test0 = 0,
	test1,
	test2,
	test3,
	test4,
	test5,
	test6,
	test7,

};
enum hx_limit_index {
	bank_self_up = 0,
	bank_self_down,
	bank_mutual_up,
	bank_mutual_down,
	dc_self_up,
	dc_self_down,
	dc_mutual_up,
	dc_mutual_down,
	basec_self,
	basec_mutual,
	iir_self,
	iir_mutual,
	delta_up,
	delta_down,
	open_up,
	open_down,
	micro_open_up,
	micro_open_down,
	short_up,
	short_down,

};

struct get_csv_data {
	uint64_t size;
	int32_t csv_data[];
};

static void himax_free_rawmem(void);
static uint32_t mpTestFunc(uint8_t checktype, uint32_t datalen);

static void hxfree(int32_t *ptr)
{
	if (ptr) {
		kfree(ptr);
		ptr = NULL;
	}
}

static int hx_alloc_Rawmem(void)
{
	uint32_t self_num = 0;
	uint32_t mutual_num = 0;
	uint32_t tx_delta_num = 0;
	uint32_t rx_delta_num = 0;
	uint32_t rx = hx_zf_getXChannel();
	uint32_t tx = hx_zf_getYChannel();

	mutual_num	= rx * tx;
	self_num	= rx + tx;
	tx_delta_num = rx * (tx - 1);
	rx_delta_num = (rx - 1) * tx;
	if (!mutual_bank) {
		mutual_bank = kzalloc(mutual_num * sizeof(uint32_t), GFP_KERNEL);
		if (mutual_bank == NULL) {
			TS_LOG_ERR("%s:mutual_bank is NULL\n", __func__);
			goto exit_mutual_bank;
		}
	}
	if (!self_bank) {
		self_bank = kzalloc(self_num * sizeof(uint32_t), GFP_KERNEL);
		if (self_bank == NULL) {
			TS_LOG_ERR("%s:self_bank is NULL\n", __func__);
			goto exit_self_bank;
		}
	}
	if (!tx_delta) {
		tx_delta = kzalloc(tx_delta_num * sizeof(uint32_t), GFP_KERNEL);
		if (tx_delta == NULL) {
			TS_LOG_ERR("%s:tx_delta is NULL\n", __func__);
			goto exit_tx_delta;
		}
	}
	if (!rx_delta) {
		rx_delta = kzalloc(rx_delta_num * sizeof(uint32_t), GFP_KERNEL);
		if (rx_delta == NULL) {
			TS_LOG_ERR("%s:rx_delta is NULL\n", __func__);
			goto exit_rx_delta;
		}
	}
	if (!mutual_iir) {
		mutual_iir = kzalloc(mutual_num * sizeof(uint32_t), GFP_KERNEL);
		if (mutual_iir == NULL) {
			TS_LOG_ERR("%s:mutual_iir is NULL\n", __func__);
			goto exit_mutual_iir;
		}
	}
	if (!self_iir) {
		self_iir = kzalloc(self_num * sizeof(uint32_t), GFP_KERNEL);
		if (self_iir == NULL) {
			TS_LOG_ERR("%s:self_iir is NULL\n", __func__);
			goto exit_self_iir;
		}
	}
	if (IC_ZF_TYPE == HX_83108A_SERIES_PWON) {
		if (!mutual_open) {
			mutual_open = kzalloc(mutual_num * sizeof(uint32_t), GFP_KERNEL);
			if (mutual_open == NULL) {
				TS_LOG_ERR("%s:mutual_open is NULL\n", __func__);
				goto exit_mutual_open;
			}
		}
		if (!self_open) {
			self_open = kzalloc(self_num * sizeof(uint32_t), GFP_KERNEL);
			if (self_open == NULL) {
				TS_LOG_ERR("%s:self_open is NULL\n", __func__);
				goto exit_self_open;
			}
		}
		if (!mutual_mopen) {
			mutual_mopen = kzalloc(mutual_num * sizeof(uint32_t), GFP_KERNEL);
			if (mutual_mopen == NULL) {
				TS_LOG_ERR("%s:mutual_mopen is NULL\n", __func__);
				goto exit_mutual_mopen;
			}
		}
		if (!self_mopen) {
			self_mopen = kzalloc(self_num * sizeof(uint32_t), GFP_KERNEL);
			if (self_mopen == NULL) {
				TS_LOG_ERR("%s:self_mopen is NULL\n", __func__);
				goto exit_self_mopen;
			}
		}
		if (!mutual_short) {
			mutual_short = kzalloc(mutual_num * sizeof(uint32_t), GFP_KERNEL);
			if (mutual_short == NULL) {
				TS_LOG_ERR("%s:mutual_short is NULL\n", __func__);
				goto exit_mutual_short;
			}
		}
		if (!self_short) {
			self_short = kzalloc(self_num * sizeof(uint32_t), GFP_KERNEL);
			if (self_short == NULL) {
				TS_LOG_ERR("%s:self_short is NULL\n", __func__);
				goto exit_self_short;
			}
		}
	}
	return NO_ERR;
	if (HX_83108A_SERIES_PWON == IC_ZF_TYPE) {
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
	uint32_t self_num;
	uint32_t mutual_num;
	uint32_t tx_delta_num;
	uint32_t rx_delta_num;
	uint32_t rx = hx_zf_getXChannel();
	uint32_t tx = hx_zf_getYChannel();

	mutual_num = rx * tx;
	self_num = rx + tx;
	tx_delta_num = rx * (tx - 1);
	rx_delta_num = (rx - 1) * tx;

	memset(mutual_bank, 0x00, mutual_num * sizeof(uint32_t));
	memset(self_bank, 0x00, self_num * sizeof(uint32_t));
	memset(tx_delta, 0x00, tx_delta_num * sizeof(uint32_t));
	memset(rx_delta, 0x00, rx_delta_num * sizeof(uint32_t));
	memset(mutual_iir, 0x00, mutual_num * sizeof(uint32_t));
	memset(self_iir, 0x00, self_num * sizeof(uint32_t));

	if (IC_ZF_TYPE == HX_83108A_SERIES_PWON) {
		memset(mutual_open, 0x00, mutual_num * sizeof(uint32_t));
		memset(self_open, 0x00, self_num * sizeof(uint32_t));
		memset(mutual_mopen, 0x00, mutual_num * sizeof(uint32_t));
		memset(self_mopen, 0x00, self_num * sizeof(uint32_t));
		memset(mutual_short, 0x00, mutual_num * sizeof(uint32_t));
		memset(self_short, 0x00, self_num * sizeof(uint32_t));
	}
}

static void hx_print_rawdata(int mode)
{
	int index1 = 0;
	uint16_t self_num = 0;
	uint16_t mutual_num = 0;
	uint16_t x_channel = hx_zf_getXChannel();
	uint16_t y_channel = hx_zf_getYChannel();
	char buf[MAX_CAP_DATA_SIZE] = {0};

	//check if devided by zero
	if ((x_channel == 0) || (x_channel == 1)) {
		TS_LOG_ERR("%s devided by zero\n", __func__);
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
				MAX_CAP_DATA_SIZE);
			if ((index1 % (x_channel)) == (x_channel - DATA_1))
				strncat((char *)info_test->tx_delta_buf,
					"\n", DATA_1);
		}
		for (index1 = 0; index1 < self_num; index1++) {
			snprintf(buf, sizeof(buf), "%5d,",
				rx_delta[index1]);
			strncat((char *)info_test->rx_delta_buf,
				buf, MAX_CAP_DATA_SIZE);
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
		TS_LOG_INFO("%s: %s\n", __func__, "OPEN_CMD");
		mutual_num = x_channel * y_channel;
		TS_LOG_INFO("%s: x_ch = %d, y_ch = %d, mu_num = %d\n",
			__func__, x_channel, y_channel, mutual_num);
		for (index1 = 0; index1 < mutual_num; index1++)
			info_test->buff[current_index++] =
				mutual_open[index1];
		break;
	case MICRO_OPEN_CMD:
		TS_LOG_INFO("%s: %s\n", __func__, "OPEN_CMD");
		mutual_num = x_channel * y_channel;
		TS_LOG_INFO("%s: x_ch = %d, y_ch = %d, mu_num = %d\n",
			__func__, x_channel, y_channel, mutual_num);
		for (index1 = 0; index1 < mutual_num; index1++)
			info_test->buff[current_index++] = mutual_mopen[index1];
		break;
	case SHORT_CMD:
		TS_LOG_INFO("%s: %s\n", __func__, "SHORT_CMD");
		mutual_num = x_channel * y_channel;
		for (index1 = 0; index1 < mutual_num; index1++)
			info_test->buff[current_index++] = mutual_short[index1];
		break;
	default:
		break;
	}
}

static int hx_interface_on_test(int step)
{
	int cnt = 0;
	int retval = NO_ERR;
	uint8_t tmp_data[5] = {0};
	uint8_t tmp_data2[2] = {0};

	TS_LOG_INFO("%s: start \n", __func__);

	if (himax_zf_bus_read(DUMMY_REGISTER, tmp_data, FOUR_BYTE_CMD, sizeof(tmp_data), DEFAULT_RETRY_CNT) < 0) {
		TS_LOG_ERR("[SW_ISSUE] %s: spi access fail!\n", __func__);
		goto err_spi;
	}

	do {
		//===========================================
		// Enable continuous burst mode : 0x13 ==> 0x31
		//===========================================
		tmp_data[0] = DATA_EN_BURST_MODE;
		if (hx_zf_bus_write(ADDR_EN_BURST_MODE, tmp_data, ONE_BYTE_CMD, sizeof(tmp_data), DEFAULT_RETRY_CNT) < 0) {
			TS_LOG_ERR("[SW_ISSUE] %s: spi access fail!\n", __func__);
			goto err_spi;
		}
		//===========================================
		// AHB address auto +4 : 0x0D ==> 0x11
		// Do not AHB address auto +4 : 0x0D ==> 0x10
		//===========================================
		tmp_data2[0] = DATA_AHB;
		if (hx_zf_bus_write(ADDR_AHB, tmp_data2, ONE_BYTE_CMD, sizeof(tmp_data2), DEFAULT_RETRY_CNT) < 0) {
			TS_LOG_ERR("[SW_ISSUE] %s: spi access fail!\n", __func__);
			goto err_spi;
		}

		// Check cmd
		himax_zf_bus_read(ADDR_EN_BURST_MODE, tmp_data, ONE_BYTE_CMD, sizeof(tmp_data), DEFAULT_RETRY_CNT);
		himax_zf_bus_read(ADDR_AHB, tmp_data2, ONE_BYTE_CMD, sizeof(tmp_data2), DEFAULT_RETRY_CNT);

		if (tmp_data[0] == DATA_EN_BURST_MODE && tmp_data2[0] == DATA_AHB) {
			break;
		}
		msleep(HX_SLEEP_1MS);
	} while (++cnt < 10);

	if (cnt > 0)
		TS_LOG_DEBUG("%s:Polling burst mode: %d times", __func__, cnt);

	hx_result_pass_str[0] = '0';
	TS_LOG_DEBUG("%s: SPI Test --> PASS\n", __func__);
	strncat(buf_test_result, hx_result_pass_str, strlen(hx_result_pass_str) + 1);
	return retval;

err_spi:
	TS_LOG_ERR("[SW_ISSUE] %s: Exit because there is SPI fail.\n", __func__);
	himax_free_rawmem();

	hx_result_fail_str[0] = '0';
	TS_LOG_DEBUG("[SW_ISSUE] %s: SPI Test --> fail\n", __func__);
	strncat(buf_test_result, hx_result_fail_str, strlen(hx_result_fail_str)+1);
	return SPI_WORK_ERR;
}

static int hx_raw_data_test(int step) //for Rawdara
{
	int i = 0;
	int j = 0;
	int index1 = 0;
	int new_data = 0;
	int result = NO_ERR;
	int rx = hx_zf_getXChannel();
	int tx = hx_zf_getYChannel();
	unsigned int index = 0;

	static uint8_t *info_data_hx83108a = NULL;
	if (!info_data_hx83108a) {
		info_data_hx83108a = kcalloc(MUTUL_NUM_HX83108A * DATA_2,
			sizeof(uint8_t), GFP_KERNEL);
		if (!info_data_hx83108a) {
			TS_LOG_ERR("fail to alloc info data mem\n");
			return HX_ERR;
		}
	}

	//check if devided by zero
	if (rx == 0) {
		TS_LOG_ERR("%s devided by zero\n", __func__);
		return HX_ERR;
	}

	TS_LOG_DEBUG("Get Raw Data Start:\n");

	hx_zf_diag_register_set(0x0A);/*===DC===*/
	hx_zf_burst_enable(1);
	hx_zf_get_dsram_data(info_data_hx83108a);
	index = 0;
	for (i = 0; i < tx; i++) {
		for (j = 0; j < rx; j++) {
			new_data = ((
				info_data_hx83108a[index +
				DATA_1] << DATA_8) |
				info_data_hx83108a[index]);
			mutual_bank[i*rx+j] = new_data;
			index += 2;
		}
	}

	hx_zf_return_event_stack();
	hx_zf_diag_register_set(0x00);

	TS_LOG_DEBUG("Get Raw Data End:\n");
	for (index1 = 0; index1 < rx * tx; index1++) {
		if (g_himax_zf_ts_data->p2p_test_sel) {
			cap_rawdata_limit[UP_LIMIT] = p2p_cap_rawdata_limit_up[index1];
			cap_rawdata_limit[DOWN_LIMIT] = p2p_cap_rawdata_limit_dw[index1];
		}
		if (mutual_bank[index1] < cap_rawdata_limit[DOWN_LIMIT] || mutual_bank[index1] > cap_rawdata_limit[UP_LIMIT]) {
			TS_LOG_INFO("[PANEL_ISSUE] POS_RX:%3d, POS_TX:%3d, CAP_RAWDATA: %6d\n", (index1 % rx), (index1 / rx), mutual_bank[index1]);
			TS_LOG_INFO("[PANEL_ISSUE] UP_LIMIT:%6d, DOWN_LIMIT: %6d\n", cap_rawdata_limit[UP_LIMIT], cap_rawdata_limit[DOWN_LIMIT]);
			result = HX_ERR;
		}
	}

	TS_LOG_DEBUG("Raw Data test End\n");

	if (result == 0 && hx_result_status[test0] == 0) {
		hx_result_pass_str[0] = '0'+step;
		strncat(buf_test_result, hx_result_pass_str, strlen(hx_result_pass_str)+1);
		TS_LOG_INFO("%s: End --> PASS\n", __func__);
		result = NO_ERR;//use for test
	} else {
		hx_result_fail_str[0] = '0'+step;
		strncat(buf_test_result, hx_result_fail_str, strlen(hx_result_fail_str)+1);
		TS_LOG_INFO("[PANEL_ISSUE] %s: End --> FAIL\n", __func__);
	}
	memset(info_data_hx83108a, 0x00, MUTUL_NUM_HX83108A * DATA_2 * sizeof(uint8_t));

	return result;
}

static int hx_self_delta_test(int step)
{
	int m = 0;
	int index1 = 0;
	int index2 = 0;
	int result = NO_ERR;
	int tx = hx_zf_getYChannel();
	int rx = hx_zf_getXChannel();
	uint16_t tx_delta_num = 0;
	uint16_t rx_delta_num = 0;

	tx_delta_num = rx*(tx-1);
	rx_delta_num = (rx-1)*tx;

	//check if devided by zero
	if ((rx == 0) || (rx == 1)) {
		TS_LOG_ERR("%s devided by zero\n", __func__);
		return HX_ERR;
	}

	/*TX Delta*/
	TS_LOG_INFO("TX Delta Start:\n");
	for (index1 = 0; index1 < tx_delta_num; index1++) {
		m = index1 + rx;
		tx_delta[index1] = ABS(mutual_bank[m] - mutual_bank[index1]);
		if (g_himax_zf_ts_data->p2p_test_sel)
			trx_delta_limit[UP_LIMIT] = p2p_tx_delta_limit_up[index1];
		if (tx_delta[index1] > trx_delta_limit[UP_LIMIT]) {
			TS_LOG_INFO("[PANEL_ISSUE] POS_RX:%3d, POS_TX:%3d, TX_DELTA: %6d\n", (index1 % rx), (index1 / rx), tx_delta[index1]);
			TS_LOG_INFO("[PANEL_ISSUE] UP_LIMIT:%6d\n", trx_delta_limit[UP_LIMIT]);
			result = HX_ERR;
		}

	}
	TS_LOG_INFO("TX Delta End\n");

	/*RX Delta*/
	TS_LOG_INFO("TX RX Delta Start:\n");
	/*lint -save -e* */
	for (index1 = 1; index2 < rx_delta_num; index1++) {
		if (index1%(rx) == 0)
			continue;
		rx_delta[index2] = ABS(mutual_bank[index1] - mutual_bank[index1-1]);
		if (g_himax_zf_ts_data->p2p_test_sel)
			trx_delta_limit[UP_LIMIT] = p2p_rx_delta_limit_up[index2];
		if (rx_delta[index2] > trx_delta_limit[UP_LIMIT]) {
			TS_LOG_INFO("[PANEL_ISSUE] POS_RX:%3d, POS_TX:%3d, RX_DELTA: %6d\n", (index1 % (rx - 1)), (index1 / (rx - 1)), rx_delta[index2]);
			TS_LOG_INFO("[PANEL_ISSUE] UP_LIMIT:%6d\n", trx_delta_limit[UP_LIMIT]);
			result = HX_ERR;
		}

		index2++;
	}
	/*lint -restore*/
	TS_LOG_INFO("TX RX Delta End\n");

	if (result == 0 && hx_result_status[test0] == 0) {
		hx_result_pass_str[0] = '0'+step;
		strncat(buf_test_result, hx_result_pass_str, strlen(hx_result_pass_str)+1);
		TS_LOG_INFO("%s: End --> PASS\n", __func__);
		result = NO_ERR;//use for test
	} else {
		hx_result_fail_str[0] = '0'+step;
		strncat(buf_test_result, hx_result_fail_str, strlen(hx_result_fail_str)+1);
		TS_LOG_INFO("[PANEL_ISSUE] %s: End --> FAIL\n", __func__);
	}

	return result;
}

static int hx_iir_test(int step) //for Noise Delta
{

	int retval = NO_ERR;
	TS_LOG_INFO("Step=%d,[NOISE_TEST]\n", step);
	retval += mpTestFunc(HX_WT_NOISE, (HX_ZF_TX_NUM*HX_ZF_RX_NUM) + HX_ZF_TX_NUM + HX_ZF_RX_NUM);
	if (retval == 0 && hx_result_status[test0] == 0) {
		hx_result_pass_str[0] = '0' + step;
		strncat(buf_test_result, hx_result_pass_str, strlen(hx_result_pass_str)+1);
		TS_LOG_INFO("%s: End --> PASS\n", __func__);
	} else {
		hx_result_fail_str[0] = '0' + step;
		strncat(buf_test_result, hx_result_fail_str, strlen(hx_result_fail_str)+1);
		TS_LOG_INFO("%s: End --> FAIL\n", __func__);
	}
	return retval;
}

/* inspection api start */
static int hx_switch_mode_inspection(int mode)
{
    uint8_t tmp_addr[4] = {0};
	uint8_t tmp_data[4] = {0};
	TS_LOG_INFO("%s: Entering\n", __func__);

	//Stop Handshaking
	hx_addr_reg_assign(ADDR_STP_HNDSHKG, DATA_INIT, tmp_addr, tmp_data);
	hx_zf_flash_write_burst_lenth(tmp_addr, tmp_data, FOUR_BYTE_CMD);

	/*Swtich Mode*/
	hx_reg_assign(ADDR_SORTING_MODE_SWITCH, tmp_addr);
	switch (mode) {
	case HIMAX_INSPECTION_SORTING:
		tmp_data[3] = 0x00; tmp_data[2] = 0x00; tmp_data[1] = PWD_SORTING_START; tmp_data[0] = PWD_SORTING_START;
		break;
	case HIMAX_INSPECTION_OPEN:
		tmp_data[3] = 0x00; tmp_data[2] = 0x00; tmp_data[1] = PWD_OPEN_START; tmp_data[0] = PWD_OPEN_START;
		break;
	case HIMAX_INSPECTION_MICRO_OPEN:
		tmp_data[3] = 0x00; tmp_data[2] = 0x00; tmp_data[1] = PWD_OPEN_START; tmp_data[0] = PWD_OPEN_START;
		break;
	case HIMAX_INSPECTION_SHORT:
		tmp_data[3] = 0x00; tmp_data[2] = 0x00; tmp_data[1] = PWD_SHORT_START; tmp_data[0] = PWD_SHORT_START;
		break;
	case HX_WT_NOISE:
		tmp_data[3] = 0x00; tmp_data[2] = 0x00;
		tmp_data[1] = PWD_NOISE_START;
		tmp_data[0] = PWD_NOISE_START;
		break;
	}
	hx_zf_flash_write_burst_lenth(tmp_addr, tmp_data, FOUR_BYTE_CMD);
	TS_LOG_INFO("%s: End of setting!\n", __func__);

	return NO_ERR;
}

static uint32_t hx_get_rawdata_func(int raw[], uint32_t datalen,  uint8_t checktype)
{

	static uint8_t *tmp_rawdata = NULL;
	int i = 0;
	int j = 0;
	int index = 0;
	int min_data = 99999;
	int max_data = -99999;
	if (!tmp_rawdata) {
		tmp_rawdata = kzalloc(sizeof(uint8_t)*(datalen*2), GFP_KERNEL);
		if (tmp_rawdata == NULL) {
			TS_LOG_INFO("%s: Memory allocate fail!\n", __func__);
			return HX_ERR;
		}
	}
	 hx_zf_get_dsram_data(tmp_rawdata);
	/* Copy Data*/
	for (i = 0; i < datalen; i++) {

		if (checktype == HX_WT_NOISE) {
			raw[i] = ((int8_t)tmp_rawdata[(i * 2) + 1] << 8) +
				tmp_rawdata[(i * 2)];
		} else {
			raw[i] = tmp_rawdata[(i * 2) + 1] << 8 |
								tmp_rawdata[(i * 2)];
		}

		if (i < (datalen - HX_ZF_RX_NUM - HX_ZF_TX_NUM)) {
			if (i == 0)
				min_data = max_data = raw[0];
			else if (raw[i] > max_data)
				max_data = raw[i];
			else if (raw[i] < min_data)
				min_data = raw[i];
		}
	}
	TS_LOG_INFO("Max = %5d, Min = %5d\n", max_data, min_data);

DIRECT_END:
	memset(tmp_rawdata, 0x00, sizeof(uint8_t) * (datalen * 2));

	return HX_INSPECT_OK;
}

static void himax_switch_data_type(uint8_t checktype)
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
	case HX_WT_NOISE:
		datatype = DATA_NOISE;
		break;
	default:
		TS_LOG_ERR("Wrong type=%d\n", checktype);
		break;
	}
	hx_zf_diag_register_set(datatype);
}

static void himax_zf_set_N_frame(uint16_t Nframe, uint8_t checktype)
{
	uint8_t tmp_addr[4] = {0};
	uint8_t tmp_data[4] = {0};

	//IIR MAX
	hx_reg_assign(ADDR_NFRAME_SEL, tmp_addr);
	tmp_data[3] = 0x00; tmp_data[2] = 0x00;
	tmp_data[1] = (uint8_t)((Nframe & 0xFF00) >> 8);
	tmp_data[0] = (uint8_t)(Nframe & 0x00FF);
	hx_zf_flash_write_burst_lenth(tmp_addr, tmp_data, FOUR_BYTE_CMD);

	//skip frame
	hx_reg_assign(ADDR_TXRX_INFO, tmp_addr);
	hx_zf_register_read(tmp_addr, FOUR_BYTE_CMD, tmp_data);

	tmp_data[0] = BS_OPENSHORT;

	hx_zf_flash_write_burst_lenth(tmp_addr, tmp_data, FOUR_BYTE_CMD);
}

static uint32_t hx_check_mode(uint8_t checktype)
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
	case HX_WT_NOISE:
		wait_pwd[0] = PWD_NOISE_END;
		wait_pwd[1] = PWD_NOISE_END;
		break;
	default:
		TS_LOG_ERR("Wrong type=%d\n", checktype);
		break;
	}

	hx_reg_assign(ADDR_SORTING_MODE_SWITCH, tmp_addr);
	hx_zf_register_read(tmp_addr, FOUR_BYTE_CMD, tmp_data);
	TS_LOG_INFO("%s: hx_wait_sorting_mode, tmp_data[0]=%x,tmp_data[1]=%x\n", __func__, tmp_data[0], tmp_data[1]);

	if (wait_pwd[0] == tmp_data[0] && wait_pwd[1] == tmp_data[1]) {
		TS_LOG_INFO("Change to mode\n");
		return !MODE_ND_CHNG;
	} else
		return MODE_ND_CHNG;
}

static uint32_t hx_wait_sorting_mode(uint8_t checktype)
{
	uint8_t tmp_addr[4] = {0};
	uint8_t tmp_data[4] = {0};
	uint8_t wait_data[4] = {0};
	uint8_t wait_pwd[2] = {0};
	int count = 0;
	int wait_flag = 0;

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
	case HX_WT_NOISE:
		wait_pwd[0] = PWD_NOISE_END;
		wait_pwd[1] = PWD_NOISE_END;
		break;
	default:
		TS_LOG_ERR("Wrong type=%d\n", checktype);
		break;
	}

	do {
		hx_reg_assign(ADDR_SORTING_MODE_SWITCH, tmp_addr);
		hx_zf_register_read(tmp_addr, FOUR_BYTE_CMD, tmp_data);
		TS_LOG_INFO("%s: hx_wait_sorting_mode, tmp_data[0]=%x,tmp_data[1]=%x\n", __func__, tmp_data[0], tmp_data[1]);
		if (wait_pwd[0] == tmp_data[0] && wait_pwd[1] == tmp_data[1]) {
			return NO_ERR;
		}

		wait_data[0] = tmp_data[0];
		wait_data[1] = tmp_data[1];

		hx_reg_assign(ADDR_READ_MODE_CHK, tmp_addr);
		hx_zf_register_read(tmp_addr, FOUR_BYTE_CMD, tmp_data);
		TS_LOG_INFO("%s: 0x900000A8, tmp_data[0]=%x, tmp_data[1]=%x, tmp_data[2]=%x, tmp_data[3]=%x\n", __func__, tmp_data[0], tmp_data[1], tmp_data[2], tmp_data[3]);
		if (tmp_data[0] == wait_data[0] && tmp_data[1] == wait_data[1])
			wait_flag++;

		hx_reg_assign(ADDR_HW_STST_CHK, tmp_addr);
		hx_zf_register_read(tmp_addr, FOUR_BYTE_CMD, tmp_data);
		TS_LOG_INFO("%s: 0x900000E4, tmp_data[0]=%x, tmp_data[1]=%x, tmp_data[2]=%x, tmp_data[3]=%x\n", __func__, tmp_data[0], tmp_data[1], tmp_data[2], tmp_data[3]);
		if (tmp_data[0] == wait_data[0] && tmp_data[1] == wait_data[1])
			wait_flag++;

		hx_reg_assign(ADDR_READ_REG_FW_STATUS, tmp_addr);
		hx_zf_register_read(tmp_addr, FOUR_BYTE_CMD, tmp_data);
		TS_LOG_INFO("%s: 0x900000E8, tmp_data[0]=%x, tmp_data[1]=%x, tmp_data[2]=%x, tmp_data[3]=%x\n", __func__, tmp_data[0], tmp_data[1], tmp_data[2], tmp_data[3]);
		if (tmp_data[0] == wait_data[0] && tmp_data[1] == wait_data[1])
			wait_flag++;

		hx_reg_assign(ADDR_READ_FW_BUG_MSG_ADDR, tmp_addr);
		hx_zf_register_read(tmp_addr, FOUR_BYTE_CMD, tmp_data);
		TS_LOG_INFO("%s: 0x10007F40, tmp_data[0]=%x, tmp_data[1]=%x, tmp_data[2]=%x, tmp_data[3]=%x\n", __func__, tmp_data[0], tmp_data[1], tmp_data[2], tmp_data[3]);
		TS_LOG_INFO("Now retry %d times!\n", count++);
		if (tmp_data[0] == wait_data[0] && tmp_data[1] == wait_data[1])
			wait_flag++;

		if (wait_flag == MODE_CHANGE_NOREADY)
			return wait_flag;

		msleep(HX_SLEEP_50MS);
	} while (count < 200);

	return FW_NOT_READY;
}

static int hx_get_max_dc(void)
{
	uint8_t tmp_data[4] = {0};
	uint8_t tmp_addr[4];
	int dc_max = 0;

	hx_reg_assign(ADDR_MAC_DC, tmp_addr);

	hx_zf_register_read(tmp_addr, 4, tmp_data);
	TS_LOG_INFO("%s: tmp_data[0-3] = %02x%02x%02x%02x\n", __func__,
		tmp_data[0], tmp_data[1], tmp_data[2], tmp_data[3]);

	dc_max = tmp_data[3]<<24 | tmp_data[2]<<16 |
			tmp_data[1]<<8 | tmp_data[0];
	TS_LOG_INFO("%s: dc max = %d\n", __func__, dc_max);
	return dc_max;
}

static void hx_get_noise_base(void)/*Normal Threshold*/
{
	uint8_t tmp_addr[4];
	uint8_t tmp_data[4];
	uint8_t tmp_addr2[4];
	uint8_t tmp_data2[4];

	hx_reg_assign(addr_normal_noise_thx, tmp_addr);
	hx_reg_assign(addr_noise_scale, tmp_addr2);

	hx_zf_register_read(tmp_addr2, 4, tmp_data2);
	tmp_data2[1] = tmp_data2[1]>>4;
	if (tmp_data2[1] == 0)
		tmp_data2[1] = 1;

	/*normal : 0x1000708F, LPWUG:0x10007093*/
	hx_zf_register_read(tmp_addr, 4, tmp_data);
	NOISEMAX = tmp_data[3] * tmp_data2[1];

	hx_reg_assign(addr_recal_thx, tmp_addr);
	hx_zf_register_read(tmp_addr, 4, tmp_data);
	g_recal_thx = tmp_data[2] * tmp_data2[1];/*0x10007092*/
	TS_LOG_INFO("%s: NOISEMAX = %d, g_recal_thx = %d\n", __func__,
		NOISEMAX, g_recal_thx);
}

static uint16_t hx_get_palm_num(void)/*Palm Number*/
{
	uint8_t tmp_addr[4];
	uint8_t tmp_data[4];
	uint16_t palm_num;

	hx_reg_assign(addr_palm_num, tmp_addr);
	hx_zf_register_read(tmp_addr, 4, tmp_data);
	palm_num = tmp_data[3];/*0x100070AB*/
	TS_LOG_INFO("%s: palm_num = %d ", __func__, palm_num);

	return palm_num;
}

static int hx_get_noise_weight_test(void)
{
	uint8_t tmp_addr[4];
	uint8_t tmp_data[4];
	uint16_t weight = 0;
	uint16_t value = 0;

	hx_reg_assign(addr_weight_sup, tmp_addr);

	/*0x100072C8 weighting value*/
	hx_zf_register_read(tmp_addr, 4, tmp_data);
	if (tmp_data[3] != 0x72 || tmp_data[2] != 0xC8)
		return FW_NOT_READY;

	value = (tmp_data[1] << 8) | tmp_data[0];
	TS_LOG_INFO("%s: value = %d, %d, %d ", __func__, value, tmp_data[2], tmp_data[3]);

	hx_reg_assign(addr_normal_weight_a, tmp_addr);

	/*Normal:0x1000709C, LPWUG:0x100070A0 weighting threshold*/
	hx_zf_register_read(tmp_addr, 4, tmp_data);
	weight = tmp_data[0];

	hx_reg_assign(addr_weight_b, tmp_addr);
	hx_zf_register_read(tmp_addr, 4, tmp_data);
	tmp_data[1] = tmp_data[1]&0x0F;
	if (tmp_data[1] == 0)
		tmp_data[1] = 1;
	weight = tmp_data[1] * weight;/*0x10007095 weighting threshold*/
	TS_LOG_INFO("%s: weight = %d ", __func__, weight);

	if (value > weight)
		return -1;
	else
		return 0;
}

static uint32_t mpTestFunc(uint8_t checktype, uint32_t datalen)
{
	uint32_t i = 0;
	uint32_t ret = 0;
	uint16_t noise_count = 0;
	uint16_t palm_num = 0;
	static int *raw = NULL;
	int count = 0;

	if (!raw) {
		raw = kzalloc(datalen * sizeof(uint32_t), GFP_KERNEL);
		if (raw == NULL) {
			TS_LOG_ERR("%s:raw is NULL\n", __func__);
			return ALLC_MEM_ERR;
		}
	}
	if (hx_check_mode(checktype)) {
		do {
			TS_LOG_INFO("Need Change Mode");

			hx_reset_device();

			hx_zf_sense_off();

			hx_switch_mode_inspection(checktype);

			himax_zf_set_N_frame(60, checktype);

			hx_zf_sense_on(SENSE_ON_1);

			ret = hx_wait_sorting_mode(checktype);
			if (ret == MODE_CHANGE_NOREADY) {
				TS_LOG_INFO("%s: wait next hx_wait_sorting_mode\n", __func__);
			} else if (ret == FW_NOT_READY) {
				TS_LOG_ERR("%s: hx_wait_sorting_mode FAIL\n", __func__);
				goto ERR_EXIT;
			} else {
				TS_LOG_ERR("%s: hx_wait_sorting_mode OK\n", __func__);
				break;
			}

			if (count > 2) {
				TS_LOG_ERR("%s: hx_wait_sorting_mode FAIL\n", __func__);
				goto ERR_EXIT;
			}
			count++;
		} while (ret == MODE_CHANGE_NOREADY);
	}
	ret = 0;

	himax_switch_data_type(checktype);
	hx_get_max_dc();
	ret = hx_get_rawdata_func(raw, datalen, checktype);
	if (ret) {
		TS_LOG_ERR("%s: hx_get_rawdata_func FAIL\n", __func__);
		goto ERR_EXIT;
	}

	/* back to normal */
	himax_switch_data_type(HIMAX_INSPECTION_BACK_NORMAL);

	//Check Data
	switch (checktype) {
	case HIMAX_INSPECTION_OPEN:
		memcpy(mutual_open, raw, sizeof(uint32_t) * (HX_ZF_TX_NUM * HX_ZF_RX_NUM));
		for (i = 0; i < (HX_ZF_TX_NUM*HX_ZF_RX_NUM); i++) {
			if (g_himax_zf_ts_data->p2p_test_sel) {
				open_short_limit[UP_LIMIT] = p2p_open_limit_up[i];
				open_short_limit[DOWN_LIMIT] = p2p_open_limit_dw[i];
			}
			if (raw[i] > open_short_limit[UP_LIMIT] || raw[i] < open_short_limit[DOWN_LIMIT]) {
				TS_LOG_ERR("%s: open test FAIL\n", __func__);
				TS_LOG_INFO("[PANEL_ISSUE] POS_RX:%3d, POS_TX:%3d, CAP_RAWDATA: %6d\n", (i % HX_ZF_RX_NUM), (i / HX_ZF_RX_NUM), raw[i]);
				TS_LOG_INFO("[PANEL_ISSUE] UP_LIMIT:%6d, DOWN_LIMIT: %6d\n", open_short_limit[UP_LIMIT], open_short_limit[DOWN_LIMIT]);
				ret = HX_INSPECT_EOPEN;
				goto ERR_EXIT;
			}
		}
		TS_LOG_INFO("%s: open test PASS\n", __func__);
		break;

	case HIMAX_INSPECTION_MICRO_OPEN:
		memcpy(mutual_mopen, raw, sizeof(uint32_t) * (HX_ZF_TX_NUM * HX_ZF_RX_NUM));
		for (i = 0; i < (HX_ZF_TX_NUM*HX_ZF_RX_NUM); i++) {
			if (g_himax_zf_ts_data->p2p_test_sel) {
				open_short_limit[UP_LIMIT + 2] = p2p_micro_open_limit_up[i];
				open_short_limit[DOWN_LIMIT + 2] = p2p_micro_open_limit_dw[i];
			}
			if (raw[i] > open_short_limit[UP_LIMIT + 2] || raw[i] < open_short_limit[DOWN_LIMIT+2]) {
				TS_LOG_INFO("[PANEL_ISSUE] POS_RX:%3d, POS_TX:%3d, CAP_RAWDATA: %6d\n", (i % HX_ZF_RX_NUM), (i / HX_ZF_RX_NUM), raw[i]);
				TS_LOG_INFO("[PANEL_ISSUE] UP_LIMIT:%6d, DOWN_LIMIT: %6d\n", open_short_limit[UP_LIMIT+2], open_short_limit[DOWN_LIMIT+2]);
				TS_LOG_ERR("%s: micro open test FAIL\n", __func__);
				ret = HX_INSPECT_EMOPEN;
				goto ERR_EXIT;
			}
		}
		TS_LOG_INFO("%s: micro open test PASS\n", __func__);
		break;

	case HIMAX_INSPECTION_SHORT:
		memcpy(mutual_short, raw, sizeof(uint32_t) * (HX_ZF_TX_NUM * HX_ZF_RX_NUM));
		for (i = 0; i < (HX_ZF_TX_NUM*HX_ZF_RX_NUM); i++) {
			if (g_himax_zf_ts_data->p2p_test_sel) {
				open_short_limit[UP_LIMIT + 4] = p2p_short_limit_up[i];
				open_short_limit[DOWN_LIMIT + 4] = p2p_short_limit_dw[i];
			}
			if (raw[i] > open_short_limit[UP_LIMIT+4] || raw[i] < open_short_limit[DOWN_LIMIT+4]) {
				TS_LOG_INFO("[PANEL_ISSUE] POS_RX:%3d, POS_TX:%3d, CAP_RAWDATA: %6d\n", (i % HX_ZF_RX_NUM), (i / HX_ZF_RX_NUM), raw[i]);
				TS_LOG_INFO("[PANEL_ISSUE] UP_LIMIT:%6d, DOWN_LIMIT: %6d\n", open_short_limit[UP_LIMIT+4], open_short_limit[DOWN_LIMIT+4]);
				TS_LOG_ERR("%s: short test FAIL\n", __func__);
				ret = HX_INSPECT_ESHORT;
				goto ERR_EXIT;

			}
		}
		TS_LOG_INFO("%s: short test PASS\n", __func__);
		break;

	case HX_WT_NOISE:
		noise_count = 0;
		memcpy(mutual_iir, raw, sizeof(uint32_t) * (HX_ZF_TX_NUM * HX_ZF_RX_NUM));
		hx_get_noise_base();
		palm_num = hx_get_palm_num();

		for (i = 0; i < (HX_ZF_TX_NUM * HX_ZF_RX_NUM);i++) {
			if (raw[i] > NOISEMAX)
				noise_count++;
		}
		TS_LOG_INFO("noise_count=%d\n", noise_count);
		if (noise_count > palm_num) {
			TS_LOG_ERR("%s: noise test FAIL\n", __func__);
			break;
		}
		if (hx_get_noise_weight_test() < 0) {
			TS_LOG_ERR("%s: get noise weight test FAIL\n", __func__);
			break;
		}
		memcpy(mutual_short, raw, sizeof(uint32_t) * (HX_ZF_TX_NUM * HX_ZF_RX_NUM));
		for (i = 0; i < (HX_ZF_TX_NUM*HX_ZF_RX_NUM); i++) {
			if (g_himax_zf_ts_data->p2p_test_sel) {
				noise_limit[UP_LIMIT] = p2p_noise_limit_up[i];
				noise_limit[DOWN_LIMIT] = p2p_noise_limit_down[i];
			}
			if (raw[i] > noise_limit[UP_LIMIT] * NOISEMAX / 100  || raw[i] < noise_limit[DOWN_LIMIT] * g_recal_thx / 100) {
				TS_LOG_INFO("[PANEL_ISSUE] POS_RX:%3d, POS_TX:%3d, CAP_RAWDATA: %6d\n", (i % HX_ZF_RX_NUM), (i / HX_ZF_RX_NUM), raw[i]);
				TS_LOG_INFO("[PANEL_ISSUE] UP_LIMIT:%6d, DOWN_LIMIT: %6d\n", noise_limit[UP_LIMIT], noise_limit[DOWN_LIMIT]);
				TS_LOG_ERR("%s: Noise test FAIL\n", __func__);
				ret = HX_INSPECT_ESHORT;
				goto ERR_EXIT;
			}
		}
		TS_LOG_INFO("%s: Noise test PASS\n", __func__);
		break;
	default:
		TS_LOG_ERR("Wrong type=%d\n", checktype);
		break;
	}
	ret = NO_ERR;
ERR_EXIT:
	memset(raw, 0x00, datalen * sizeof(uint32_t));
	return ret;
}

/* inspection api end */

static int hx_open_test(int step)
{
	int retval = NO_ERR;
	TS_LOG_INFO("Step=%d,[MP_OPEN_TEST]\n", step);
	retval += mpTestFunc(HIMAX_INSPECTION_OPEN, (HX_ZF_TX_NUM*HX_ZF_RX_NUM) + HX_ZF_TX_NUM + HX_ZF_RX_NUM);
	if (retval == 0 && hx_result_status[test0] == 0) {
		hx_result_pass_str[0] = '0'+step;
		strncat(buf_test_result, hx_result_pass_str, strlen(hx_result_pass_str)+1);
		TS_LOG_INFO("%s: End --> PASS\n", __func__);
	} else {
		hx_result_fail_str[0] = '0'+step;
		strncat(buf_test_result, hx_result_fail_str, strlen(hx_result_fail_str)+1);
		TS_LOG_INFO("%s: End --> FAIL\n", __func__);
	}

	return retval;
}

static int hx_short_test(int step)
{
	int retval = NO_ERR;
	TS_LOG_INFO("Step=%d,[MP_SHORT_TEST]\n", step);
	retval += mpTestFunc(HIMAX_INSPECTION_SHORT, (HX_ZF_TX_NUM*HX_ZF_RX_NUM) + HX_ZF_TX_NUM + HX_ZF_RX_NUM);

	if (retval == 0 && hx_result_status[test0] == 0) {
		hx_result_pass_str[0] = '0' + step;
		strncat(buf_test_result, hx_result_pass_str, strlen(hx_result_pass_str)+1);
		TS_LOG_INFO("%s: End --> PASS\n", __func__);
	} else {
		hx_result_fail_str[0] = '0'+step;
		strncat(buf_test_result, hx_result_fail_str, strlen(hx_result_fail_str)+1);
		TS_LOG_INFO("%s: End --> FAIL\n", __func__);
	}

	return retval;
}


static int hx_get_threshold_from_csvfile(int columns, int rows, char *target_name, struct get_csv_data *data)
{
	char file_path[100] = {0};
	char file_name[64] = {0};
	int ret = 0;
	int result = 0;

	TS_LOG_INFO("%s called\n", __func__);

	if (!data || !target_name || columns*rows > data->size) {
		TS_LOG_ERR("parse csvfile failed: data or target_name is NULL\n");
		return HX_ERROR;
	}

	snprintf(file_name, sizeof(file_name), "%s_%s_%s_%s_raw.csv",
			g_himax_zf_ts_data->tskit_himax_data->ts_platform_data->product_name,
			g_himax_zf_ts_data->tskit_himax_data->ts_platform_data->chip_data->chip_name,
			himax_zf_project_id,
			g_himax_zf_ts_data->tskit_himax_data->ts_platform_data->chip_data->module_name);

	snprintf(file_path, sizeof(file_path), "/odm/etc/firmware/ts/%s", file_name);
	TS_LOG_INFO("threshold file name:%s, rows_size=%d, columns_size=%d, target_name = %s\n",
		file_path, rows, columns, target_name);

	result =  ts_kit_parse_csvfile(file_path, target_name, data->csv_data, rows, columns);
	if (HX_OK == result) {
		ret = HX_OK;
		TS_LOG_INFO("Get threshold successed form csvfile\n");
	} else {
		TS_LOG_INFO("csv file parse fail:%s\n", file_path);
		ret = HX_ERROR;
	}
	return ret;
}

static int hx_parse_threshold_csvfile(void)
{
	int retval = 0;
	struct get_csv_data *rawdata_limit = NULL;

	rawdata_limit = kzalloc(8*sizeof(int32_t) + sizeof(struct get_csv_data), GFP_KERNEL);
	if (!rawdata_limit) {
		TS_LOG_ERR("%s:alloc storage fail", __func__);
		return HX_ERR;
	}
	rawdata_limit->size = CSV_LIMIT_MAX_LEN;

	if  (HX_ERROR == hx_get_threshold_from_csvfile(2, 1, CSV_CAP_RADATA_LIMIT, rawdata_limit))  {
		TS_LOG_ERR("%s:get threshold from csvfile failed\n", __func__);
		retval = -1;
		goto exit;
	} else {
		memcpy(cap_rawdata_limit, rawdata_limit->csv_data, 2*sizeof(u32));
	}
	TS_LOG_INFO("%s :cap_rawdata_limit[%d] = %d, cap_rawdata_limit[%d] = %d \n", __func__, DOWN_LIMIT, cap_rawdata_limit[0], UP_LIMIT, cap_rawdata_limit[1]);

	if  (HX_ERROR == hx_get_threshold_from_csvfile(2, 1, CSV_TRX_DELTA_LIMIT, rawdata_limit))  {
		TS_LOG_ERR("%s:get threshold from csvfile failed\n", __func__);
		retval = -1;
		goto exit;
	} else {
		memcpy(trx_delta_limit, rawdata_limit->csv_data, 2*sizeof(u32));
	}
	TS_LOG_INFO("%s :trx_delta_limit[%d] = %d, trx_delta_limit[%d] = %d \n", __func__, DOWN_LIMIT, trx_delta_limit[0], UP_LIMIT, trx_delta_limit[1]);

	if (HX_ERROR == hx_get_threshold_from_csvfile(2, 1, CSV_NOISE_LIMIT_UP, rawdata_limit))  {
		TS_LOG_ERR("%s:get threshold from csvfile failed\n", __func__);
		retval = -1;
		goto exit;
	} else {
		memcpy(noise_limit, rawdata_limit->csv_data, 2*sizeof(u32));
	}
	TS_LOG_INFO("%s :noise_limit[%d] = %d, noise_limit[%d] = %d \n", __func__, DOWN_LIMIT, noise_limit[0], UP_LIMIT, noise_limit[1]);

	rawdata_limit->size = CSV_OPEN_SHORT_LIMIT_MAX_LEN;
	if (HX_ERROR == hx_get_threshold_from_csvfile(6, 1, CSV_OPEN_SHORT_LIMIT, rawdata_limit))  {
		TS_LOG_ERR("%s:get threshold from csvfile failed\n", __func__);
		retval = -1;
		goto exit;
	} else {
		memcpy(open_short_limit, rawdata_limit->csv_data, 6*sizeof(u32));
	}

	TS_LOG_DEBUG("%s :open_short_limit_aa_down[%d] = %d, open_short_limit_aa_up[%d] = %d\n", __func__, DOWN_LIMIT,
		open_short_limit[DOWN_LIMIT], UP_LIMIT, open_short_limit[UP_LIMIT]);
	TS_LOG_DEBUG("%s :open_short_limit_key_down[%d] = %d, open_short_limit_key_up[%d] = %d\n", __func__, DOWN_LIMIT+2,
		open_short_limit[DOWN_LIMIT+2], UP_LIMIT+2, open_short_limit[UP_LIMIT+2]);
	TS_LOG_DEBUG("%s :open_short_limit_avg_down[%d] = %d, open_short_limit_avg_up[%d] = %d\n", __func__, DOWN_LIMIT+4,
		open_short_limit[DOWN_LIMIT+4], UP_LIMIT+4, open_short_limit[UP_LIMIT+4]);

	exit:
	if (rawdata_limit)
		kfree(rawdata_limit);

	TS_LOG_INFO("%s:--\n", __func__);

	return retval;
}

static int hx_parse_threshold_csvfile_p2p(void)
{
	int retval = 0;
	static struct get_csv_data *rawdata_limit = NULL;
	if (!rawdata_limit) {
		rawdata_limit = kzalloc(rawdata_limit_col * rawdata_limit_row * sizeof(int32_t) + sizeof(struct get_csv_data), GFP_KERNEL);
		if (rawdata_limit == NULL) {
			TS_LOG_ERR("%s:rawdata_limit alloc memory failed\n", __func__);
			return HX_ERR;
		}
	}
	rawdata_limit->size = rawdata_limit_col * rawdata_limit_row;
	/*	1	*/
	if (HX_ERROR == hx_get_threshold_from_csvfile(rawdata_limit_col, rawdata_limit_row, CSV_CAP_RADATA_LIMIT_UP, rawdata_limit)) {
		TS_LOG_ERR("%s:get threshold from csvfile failed\n", __func__);
		retval = -1;
		goto exit;
	} else {
		memcpy(p2p_cap_rawdata_limit_up, rawdata_limit->csv_data, rawdata_limit_col*rawdata_limit_row * sizeof(int32_t));
	}
	/*	2	*/
	if (HX_ERROR == hx_get_threshold_from_csvfile(rawdata_limit_col, rawdata_limit_row, CSV_CAP_RADATA_LIMIT_DW, rawdata_limit)) {
		TS_LOG_ERR("%s:get threshold from csvfile failed\n", __func__);
		retval = -1;
		goto exit;
	} else {
		memcpy(p2p_cap_rawdata_limit_dw, rawdata_limit->csv_data, rawdata_limit_col * rawdata_limit_row * sizeof(int32_t));
	}
	/*	3	*/
	rawdata_limit->size = rawdata_limit_col * (rawdata_limit_row - 1);
	if (HX_ERROR == hx_get_threshold_from_csvfile(rawdata_limit_col, (rawdata_limit_row - 1), CSV_TX_DELTA_LIMIT_UP, rawdata_limit))  {
		TS_LOG_ERR("%s:get threshold from csvfile failed\n", __func__);
		retval = -1;
		goto exit;
	} else {
		memcpy(p2p_tx_delta_limit_up, rawdata_limit->csv_data, rawdata_limit_col*(rawdata_limit_row - 1) * sizeof(int32_t));
	}
	/*	4	*/
	rawdata_limit->size = (rawdata_limit_col - 1) * rawdata_limit_row;
	if (HX_ERROR == hx_get_threshold_from_csvfile((rawdata_limit_col - 1), rawdata_limit_row, CSV_RX_DELTA_LIMIT_UP, rawdata_limit))  {
		TS_LOG_ERR("%s:get threshold from csvfile failed\n", __func__);
		retval = -1;
		goto exit;
	} else {
		memcpy(p2p_rx_delta_limit_up, rawdata_limit->csv_data, (rawdata_limit_col - 1)*rawdata_limit_row * sizeof(int32_t));

	}
	/*	5	*/
	rawdata_limit->size = rawdata_limit_col * rawdata_limit_row;
	if (HX_ERROR == hx_get_threshold_from_csvfile(rawdata_limit_col, rawdata_limit_row, CSV_OPEN_LIMIT_UP, rawdata_limit))  {
		TS_LOG_ERR("%s:get threshold from csvfile failed\n", __func__);
		retval = -1;
		goto exit;
	} else {
		memcpy(p2p_open_limit_up, rawdata_limit->csv_data, rawdata_limit_col*rawdata_limit_row * sizeof(int32_t));
	}
	/*	6	*/
	if (HX_ERROR == hx_get_threshold_from_csvfile(rawdata_limit_col, rawdata_limit_row, CSV_OPEN_LIMIT_DW, rawdata_limit))  {
		TS_LOG_ERR("%s:get threshold from csvfile failed\n", __func__);
		retval = -1;
		goto exit;
	} else {
		memcpy(p2p_open_limit_dw, rawdata_limit->csv_data, rawdata_limit_col*rawdata_limit_row * sizeof(int32_t));
	}
	/*	7	*/
	if (HX_ERROR == hx_get_threshold_from_csvfile(rawdata_limit_col, rawdata_limit_row, CSV_MICRO_OPEN_LIMIT_UP, rawdata_limit))  {
		TS_LOG_ERR("%s:get threshold from csvfile failed\n", __func__);
		retval = -1;
		goto exit;
	} else {
		memcpy(p2p_micro_open_limit_up, rawdata_limit->csv_data, rawdata_limit_col*rawdata_limit_row * sizeof(int32_t));
	}
	/*	8	*/
	if (HX_ERROR == hx_get_threshold_from_csvfile(rawdata_limit_col, rawdata_limit_row, CSV_MICRO_OPEN_LIMIT_DW, rawdata_limit))  {
		TS_LOG_ERR("%s:get threshold from csvfile failed\n", __func__);
		retval = -1;
		goto exit;
	} else {
		memcpy(p2p_micro_open_limit_dw, rawdata_limit->csv_data, rawdata_limit_col*rawdata_limit_row * sizeof(int32_t));
	}
	/*	9	*/
	if (HX_ERROR == hx_get_threshold_from_csvfile(rawdata_limit_col, rawdata_limit_row, CSV_SHORT_LIMIT_UP, rawdata_limit))  {
		TS_LOG_ERR("%s:get threshold from csvfile failed\n", __func__);
		retval = -1;
		goto exit;
	} else {
			memcpy(p2p_short_limit_up, rawdata_limit->csv_data, rawdata_limit_col*rawdata_limit_row * sizeof(int32_t));
	}
	/*	10	*/
	if (HX_ERROR == hx_get_threshold_from_csvfile(rawdata_limit_col, rawdata_limit_row, CSV_SHORT_LIMIT_DW, rawdata_limit))  {
		TS_LOG_ERR("%s:get threshold from csvfile failed\n", __func__);
		retval = -1;
		goto exit;
	} else {
			memcpy(p2p_short_limit_dw, rawdata_limit->csv_data, rawdata_limit_col*rawdata_limit_row * sizeof(int32_t));
	}
	/*	11	*/
	if (HX_ERROR == hx_get_threshold_from_csvfile(rawdata_limit_col, rawdata_limit_row, CSV_NOISE_LIMIT_UP, rawdata_limit))  {
		TS_LOG_ERR("%s:get threshold from csvfile failed\n", __func__);
		retval = -1;
		goto exit;
	} else {
		memcpy(p2p_noise_limit_up, rawdata_limit->csv_data, rawdata_limit_col*rawdata_limit_row * sizeof(int32_t));
	}

	if (HX_ERROR == hx_get_threshold_from_csvfile(rawdata_limit_col, rawdata_limit_row, CSV_NOISE_LIMIT_DOWN, rawdata_limit))  {
		TS_LOG_ERR("%s:get threshold from csvfile failed\n", __func__);
		retval = -1;
		goto exit;
	} else {
		memcpy(p2p_noise_limit_down, rawdata_limit->csv_data, rawdata_limit_col*rawdata_limit_row * sizeof(int32_t));
	}

	exit:
	if (rawdata_limit)
		memset(rawdata_limit, 0x00, rawdata_limit_col * rawdata_limit_row * sizeof(int32_t) + sizeof(struct get_csv_data));

	TS_LOG_INFO("%s:--\n", __func__);

	return retval;
}

static int hx_parse_threshold_dts(void)
{
	int retval = 0;
	struct device_node *device;

	struct himax_ts_data *ts_cap = g_himax_zf_ts_data;
	device = ts_cap->ts_dev->dev.of_node;

	retval = of_property_read_u32_array(device, CAP_RADATA_LIMIT, &cap_rawdata_limit[0], 2);
	if (retval < 0) {
		TS_LOG_ERR("Failed to read limits from dt:retval = %d \n", retval);
		return retval;
	}
	TS_LOG_INFO("%s :cap_rawdata_limit[%d] = %d, cap_rawdata_limit[%d] = %d \n", __func__, DOWN_LIMIT, cap_rawdata_limit[0], UP_LIMIT, cap_rawdata_limit[1]);

	retval = of_property_read_u32_array(device, TRX_DELTA_LIMIT, &trx_delta_limit[0], 2);
	if (retval < 0) {
		TS_LOG_ERR("Failed to read limits from dt:retval =%d\n", retval);
		return retval;
	}
	TS_LOG_INFO("%s :trx_delta_limit[%d] = %d, trx_delta_limit[%d] = %d \n", __func__, DOWN_LIMIT, trx_delta_limit[0], UP_LIMIT, trx_delta_limit[1]);

	retval = of_property_read_u32_array(device, NOISE_LIMIT, &noise_limit[0], 2);
	if (retval < 0) {
		TS_LOG_ERR("Failed to read limits from dt:retval = %d \n", retval);
		return retval;
	}
	TS_LOG_INFO("%s :noise_limit[%d] = %d, noise_limit[%d] = %d \n", __func__, DOWN_LIMIT, noise_limit[0], UP_LIMIT, noise_limit[1]);

	retval = of_property_read_u32_array(device, OPEN_SHORT_LIMIT, &open_short_limit[0], 6);
	if (retval < 0) {
		TS_LOG_ERR("Failed to read limits from dt:retval = %d \n", retval);
		return retval;
	}
	TS_LOG_INFO("%s :open_short_limit_aa_down[%d] = %d, open_short_limit_aa_up[%d] = %d\n", __func__, DOWN_LIMIT,
		open_short_limit[DOWN_LIMIT], UP_LIMIT, open_short_limit[UP_LIMIT]);
	TS_LOG_INFO("%s :open_short_limit_key_down[%d] = %d, open_short_limit_key_up[%d] = %d\n", __func__, DOWN_LIMIT+2,
		open_short_limit[DOWN_LIMIT+2], UP_LIMIT+2, open_short_limit[UP_LIMIT+2]);
	TS_LOG_INFO("%s :open_short_limit_avg_down[%d] = %d, open_short_limit_avg_up[%d] = %d\n", __func__, DOWN_LIMIT+4,
		open_short_limit[DOWN_LIMIT+4], UP_LIMIT+4, open_short_limit[UP_LIMIT+4]);
	return retval;
}

static void himax_p2p_test_deinit(void)
{
	memset(p2p_cap_rawdata_limit_up, 0x00, rawdata_limit_col * rawdata_limit_row * sizeof(int32_t));
	memset(p2p_cap_rawdata_limit_dw, 0x00, rawdata_limit_col * rawdata_limit_row * sizeof(int32_t));
	memset(p2p_open_limit_up, 0x00, rawdata_limit_col * rawdata_limit_row * sizeof(int32_t));
	memset(p2p_open_limit_dw, 0x00, rawdata_limit_col * rawdata_limit_row * sizeof(int32_t));
	memset(p2p_micro_open_limit_up, 0x00, rawdata_limit_col * rawdata_limit_row * sizeof(int32_t));
	memset(p2p_micro_open_limit_dw, 0x00, rawdata_limit_col * rawdata_limit_row * sizeof(int32_t));
	memset(p2p_short_limit_up, 0x00, rawdata_limit_col * rawdata_limit_row * sizeof(int32_t));
	memset(p2p_short_limit_dw, 0x00, rawdata_limit_col * rawdata_limit_row * sizeof(int32_t));
	memset(p2p_tx_delta_limit_up, 0x00, rawdata_limit_col * rawdata_limit_row * sizeof(int32_t));
	memset(p2p_rx_delta_limit_up, 0x00, rawdata_limit_col * rawdata_limit_row * sizeof(int32_t));
	memset(p2p_noise_limit_up, 0x00, rawdata_limit_col * rawdata_limit_row * sizeof(int32_t));
	memset(p2p_noise_limit_down, 0x00, rawdata_limit_col * rawdata_limit_row * sizeof(int32_t));
}

static int hx_p2p_test_init(void)
{
	int p2p_test_sel = g_himax_zf_ts_data->p2p_test_sel;
	TS_LOG_INFO("%s:\n", __func__);
	if (p2p_test_sel) {
		rawdata_limit_col = HX_ZF_RX_NUM;
		rawdata_limit_row = HX_ZF_TX_NUM;

		if (!p2p_cap_rawdata_limit_up) {
			p2p_cap_rawdata_limit_up = kzalloc(rawdata_limit_col * rawdata_limit_row * sizeof(u32), GFP_KERNEL);
			if (!p2p_cap_rawdata_limit_up) {
				TS_LOG_ERR("%s:kzalloc fail!\n", __func__);
				goto p2p1;
			}
		}
		if (!p2p_cap_rawdata_limit_dw) {
			p2p_cap_rawdata_limit_dw = kzalloc(rawdata_limit_col * rawdata_limit_row * sizeof(u32), GFP_KERNEL);
			if (!p2p_cap_rawdata_limit_dw) {
				TS_LOG_ERR("%s:kzalloc fail!\n", __func__);
				goto p2p2;
			}
		}

		if (!p2p_tx_delta_limit_up) {
			p2p_tx_delta_limit_up = kzalloc(rawdata_limit_col * (rawdata_limit_row - 1) * sizeof(u32), GFP_KERNEL);
			if (!p2p_tx_delta_limit_up) {
				TS_LOG_ERR("%s:kzalloc fail!\n", __func__);
				goto p2p3;
			}
		}

		if (!p2p_rx_delta_limit_up) {
			p2p_rx_delta_limit_up = kzalloc((rawdata_limit_col - 1) * rawdata_limit_row * sizeof(u32), GFP_KERNEL);
			if (!p2p_rx_delta_limit_up) {
				TS_LOG_ERR("%s:kzalloc fail!\n", __func__);
				goto p2p4;
			}
		}

		if (!p2p_open_limit_up) {
			p2p_open_limit_up = kzalloc(rawdata_limit_col * rawdata_limit_row * sizeof(u32), GFP_KERNEL);
			if (!p2p_open_limit_up) {
				TS_LOG_ERR("%s:kzalloc fail!\n", __func__);
				goto p2p5;
			}
		}

		if (!p2p_open_limit_dw) {
			p2p_open_limit_dw = kzalloc(rawdata_limit_col * rawdata_limit_row * sizeof(u32), GFP_KERNEL);
			if (!p2p_open_limit_dw) {
				TS_LOG_ERR("%s:kzalloc fail!\n", __func__);
				goto p2p6;
			}
		}

		if (!p2p_micro_open_limit_up) {
			p2p_micro_open_limit_up = kzalloc(rawdata_limit_col * rawdata_limit_row * sizeof(u32), GFP_KERNEL);
			if (!p2p_micro_open_limit_up) {
				TS_LOG_ERR("%s:kzalloc fail!\n", __func__);
				goto p2p7;
			}
		}

		if (!p2p_micro_open_limit_dw) {
			p2p_micro_open_limit_dw = kzalloc(rawdata_limit_col * rawdata_limit_row * sizeof(u32), GFP_KERNEL);
			if (!p2p_micro_open_limit_dw) {
				TS_LOG_ERR("%s:kzalloc fail!\n", __func__);
				goto p2p8;
			}
		}

		if (!p2p_short_limit_up) {
			p2p_short_limit_up = kzalloc(rawdata_limit_col * rawdata_limit_row * sizeof(u32), GFP_KERNEL);
			if (!p2p_short_limit_up) {
				TS_LOG_ERR("%s:kzalloc fail!\n", __func__);
				goto p2p9;
			}
		}

		if (!p2p_short_limit_dw) {
			p2p_short_limit_dw = kzalloc(rawdata_limit_col * rawdata_limit_row * sizeof(u32), GFP_KERNEL);
			if (!p2p_short_limit_dw) {
				TS_LOG_ERR("%s:kzalloc fail!\n", __func__);
				goto p2p10;
			}
		}

		if (!p2p_noise_limit_up) {
			p2p_noise_limit_up = kzalloc(rawdata_limit_col * rawdata_limit_row * sizeof(u32), GFP_KERNEL);
			if (!p2p_noise_limit_up) {
				TS_LOG_ERR("%s:kzalloc fail!\n", __func__);
				goto p2p11;
			}
		}

		if (!p2p_noise_limit_down) {
			p2p_noise_limit_down= kzalloc(rawdata_limit_col * rawdata_limit_row * sizeof(u32), GFP_KERNEL);
			if (!p2p_noise_limit_down) {
				TS_LOG_ERR("%s:kzalloc fail!\n", __func__);
				goto p2p12;
			}
		}

	} else {
		rawdata_limit_col = 2;
		rawdata_limit_row = 1;
		rawdata_limit_os_cnt = 6;
	}

	return 0;

p2p12:
	hxfree(p2p_noise_limit_up);
	p2p_noise_limit_up = NULL;
p2p11:
	hxfree(p2p_short_limit_dw);
	p2p_short_limit_dw = NULL;
p2p10:
	hxfree(p2p_short_limit_up);
	p2p_short_limit_up = NULL;
p2p9:
	hxfree(p2p_micro_open_limit_dw);
	p2p_micro_open_limit_dw = NULL;
p2p8:
	hxfree(p2p_micro_open_limit_up);
	p2p_micro_open_limit_up = NULL;
p2p7:
	hxfree(p2p_open_limit_dw);
	p2p_open_limit_dw = NULL;
p2p6:
	hxfree(p2p_open_limit_up);
	p2p_open_limit_up = NULL;
p2p5:
	hxfree(p2p_rx_delta_limit_up);
	p2p_rx_delta_limit_up = NULL;
p2p4:
	hxfree(p2p_tx_delta_limit_up);
	p2p_tx_delta_limit_up = NULL;
p2p3:
	hxfree(p2p_cap_rawdata_limit_dw);
	p2p_cap_rawdata_limit_dw = NULL;
p2p2:
	hxfree(p2p_cap_rawdata_limit_up);
	p2p_cap_rawdata_limit_up = NULL;
p2p1:

	return NO_ERR;
}

#define RETRY_TIME 3
static int hx_zf_fw_mpap_update(void)
{

	int result;
	char fileName[128];
	int fw_type = 0;
	char firmware_name[64] = {0};
	int count = 0;

	const struct firmware *fw = NULL;

	himax_mpap_update_fw = HIMAX_MPAP_UPDATE_FW;
	snprintf(firmware_name, sizeof(firmware_name), "ts/%s", himax_mpap_update_fw);
	TS_LOG_INFO("%s: firmware_name = %s\n", __func__, firmware_name);

	do {
		if (count != 0)
			hx_reset_device();

		/* manual upgrade will not use embedded firmware */
		result = request_firmware(&fw, firmware_name, &g_himax_zf_ts_data->tskit_himax_data->ts_platform_data->ts_dev->dev);
		if (result < 0) {
			TS_LOG_ERR("request FW %s failed\n", firmware_name);
			return result;
		}

		firmware_update(fw);
		release_firmware(fw);
		himax_zf_read_FW_info();
	} while ((g_himax_zf_ts_data->vendor_panel_ver != 0x0B) && (count++ < RETRY_TIME));

	if (count == RETRY_TIME)
		TS_LOG_ERR("%s: The correct PANEL_VER number cannot be read\n", __func__);

	TS_LOG_INFO("%s: end!\n", __func__);
	return result;

}

int hx_zf_factory_start(struct himax_ts_data *ts, struct ts_rawdata_info *info_top)
{
	int retval = NO_ERR;
	uint32_t self_num = 0;
	uint32_t mutual_num = 0;
	uint16_t rx = 0, tx = 0;
	unsigned long timer_start = 0, timer_end = 0;

	timer_start = jiffies;
	rx = hx_zf_getXChannel();
	tx = hx_zf_getYChannel();
	mutual_num	= rx * tx;
	self_num	= rx + tx;

	retval = hx_p2p_test_init();
	if (!!retval)
		goto err_alloc_p2p_test;

	TS_LOG_INFO("%s: Enter\n", __func__);
	hx_zf_fw_mpap_update();

	info_test = info_top;

	/* init */

	if (ts->test_capacitance_via_csvfile == true) {
		if (g_himax_zf_ts_data->p2p_test_sel)
			retval = hx_parse_threshold_csvfile_p2p();
		else
			retval = hx_parse_threshold_csvfile();
		if (retval < 0) {
			TS_LOG_ERR("[SW_ISSUE] %s: Parse csvfile limit Fail.\n", __func__);
		}
	} else {
		retval = hx_parse_threshold_dts();
		if (retval < 0) {
			TS_LOG_ERR("[SW_ISSUE] %s: Parse dts limit Fail.\n", __func__);
		}
	}

	if (atomic_read(&hmx_zf_mmi_test_status)) {
		TS_LOG_ERR("[SW_ISSUE] %s factory test already has been called.\n", __func__);
		retval = FACTORY_RUNNING;
		goto err_atomic_read;
	}

	atomic_set(&hmx_zf_mmi_test_status, 1);
	memset(buf_test_result, 0, RESULT_LEN);

	TS_LOG_DEBUG("himax_self_test enter \n");

	__pm_stay_awake(&g_himax_zf_ts_data->tskit_himax_data->ts_platform_data->ts_wake_lock);

	retval = hx_alloc_Rawmem();
	if (retval != 0) {
		TS_LOG_ERR("[SW_ISSUE] %s factory test alloc_Rawmem failed.\n", __func__);
		goto err_alloc_rawmem;
	}

	/* step0: himax self test*/
	hx_result_status[test0] = hx_interface_on_test(test0);

	/* step1: cap rawdata */
	hx_result_status[test1] = hx_raw_data_test(test1); //for Rawdata

	/* step2: TX RX Delta */
	hx_result_status[test2] = hx_self_delta_test(test2); //for TRX delta

	if (IC_ZF_TYPE == HX_83108A_SERIES_PWON) {
		/* step3: Noise Delta */
		hx_result_status[test3] = hx_iir_test(test3);
		hx_result_status[test4] = hx_open_test(test4);
		hx_result_status[test5] = hx_short_test(test5);
	}
	//=============Show test result===================
	strncat(buf_test_result, STR_IC_VENDOR, strlen(STR_IC_VENDOR)+1);
	strncat(buf_test_result, "-", strlen("-")+1);
	strncat(buf_test_result, ts->tskit_himax_data->ts_platform_data->chip_data->chip_name, strlen(ts->tskit_himax_data->ts_platform_data->chip_data->chip_name)+1);
	strncat(buf_test_result, "-", strlen("-")+1);
	strncat(buf_test_result, ts->tskit_himax_data->ts_platform_data->product_name, strlen(ts->tskit_himax_data->ts_platform_data->product_name)+1);
	strncat(buf_test_result, ";", strlen(";")+1);

	strncat(info_test->result, buf_test_result, strlen(buf_test_result)+1);

	info_test->buff[0] = rx;
	info_test->buff[1] = tx;

	/*print basec and dc*/
	current_index = 2;
	if (IC_ZF_TYPE == HX_83108A_SERIES_PWON) {
		hx_print_rawdata(BANK_CMD);
		hx_print_rawdata(IIR_CMD);
		hx_print_rawdata(OPEN_CMD);
		hx_print_rawdata(MICRO_OPEN_CMD);
		hx_print_rawdata(SHORT_CMD);
		hx_print_rawdata(RTX_DELTA_CMD1);
	}

	info_test->used_size = current_index;

	himax_free_rawmem();

	hx_zf_fw_resume_update(himax_firmware_name);
	hx_zf_hw_reset(HX_LOADCONFIG_EN, HX_INT_DISABLE);

err_alloc_rawmem:
    __pm_relax(&g_himax_zf_ts_data->tskit_himax_data->ts_platform_data->ts_wake_lock);
    atomic_set(&hmx_zf_mmi_test_status, 0);

err_atomic_read:
err_alloc_p2p_test:
	himax_p2p_test_deinit();
	timer_end = jiffies;
	TS_LOG_INFO("%s: self test time:%d\n", __func__, jiffies_to_msecs(timer_end-timer_start));
	return retval;
}
