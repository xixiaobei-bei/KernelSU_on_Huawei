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

#ifdef CONFIG_APP_INFO
#include <misc/app_info.h>
#endif

#if defined (CONFIG_HUAWEI_DSM)
#include <dsm/dsm_pub.h>
#endif

#define HIMAX_ROI
#if defined(CONFIG_TOUCHSCREEN_HIMAX_ZF_DEBUG)
#ifdef HX_TP_SYS_DIAG
	uint8_t hx_zf_diag_coor[HX_RECEIVE_BUF_MAX_SIZE] = {0};
#endif
#endif

#ifdef HX_ESD_WORKAROUND
static uint8_t ESD_RESET_ACTIVATE = 1;
static int g_zero_event_count;
#endif

#define RETRY_TIMES 200
#define HIMAX_VENDER_NAME "himax64zf"
#define VENDER_NAME_LENGTH 7
#define INFO_SECTION_NUM 2
#define INFO_START_ADDR 0x20000
#define INFO_PAGE_LENGTH 0x1000
#define CRC_FW_ADDR 0x15400

static uint8_t *huawei_project_id;
static uint32_t huawei_project_id_len;
static uint8_t *huawei_cg_color;
static uint32_t huawei_cg_color_len;

char himax_zf_project_id[HX_PROJECT_ID_LEN+1] = {"999999999"};

static uint8_t hw_reset_activate;
static uint8_t EN_NoiseFilter;
static uint8_t Last_EN_NoiseFilter;
static int hx_real_point_num;

static uint32_t hx_id_addr[ID_ADDR_LEN] = {0};
static uint32_t hx_flash_addr[FLASH_ADDR_LEN] = {0};

static int gest_pt_cnt;
static int gest_pt_x[GEST_PT_MAX_NUM] = {0};
static int gest_pt_y[GEST_PT_MAX_NUM] = {0};
static int gest_start_x;
static int gest_start_y;
static int gest_end_x;
static int gest_end_y;
static int gest_most_left_x;
static int gest_most_left_y;
static int gest_most_right_x;
static int gest_most_right_y;
static int gest_most_top_x;
static int gest_most_top_y;
static int gest_most_bottom_x;
static int gest_most_bottom_y;
static int16_t *self_data;
static int16_t *mutual_data;
static int HX_TOUCH_INFO_POINT_CNT;

static int touch_driver_read_panel_info(void);
static int himax_read_project_id(void);
struct himax_ts_data *g_himax_zf_ts_data;
static struct mutex wrong_touch_lock;
static int himax_palm_switch(struct ts_palm_info *info);
static int hx_charger_switch(struct ts_charger_info *info);
static int hx_horizon_switch(struct ts_horizon_info *info);
static int touch_driver_get_projectid_color(struct himax_ts_data *ts);

extern bool zf_dsram_flag;
extern char himax_firmware_name[64];
int hx_zf_input_config(struct input_dev *input_dev);

int hx_addr_reg_assign(int reg_addr, int reg_data, uint8_t *addr_buf, uint8_t *data_buf)
{
	if ((!addr_buf) || (!data_buf))
		return HX_ERR;
	addr_buf[3] = (uint8_t)((reg_addr >> 24) & 0x000000FF);
	addr_buf[2] = (uint8_t)((reg_addr >> 16) & 0x000000FF);
	addr_buf[1] = (uint8_t)((reg_addr >> 8) & 0x000000FF);
	addr_buf[0] = (uint8_t)(reg_addr & 0x000000FF);

	data_buf[3] = (uint8_t)((reg_data >> 24) & 0x000000FF);
	data_buf[2] = (uint8_t)((reg_data >> 16) & 0x000000FF);
	data_buf[1] = (uint8_t)((reg_data >> 8) & 0x000000FF);
	data_buf[0] = (uint8_t)(reg_data & 0x000000FF);

	TS_LOG_INFO("%s:addr_buf[3~1] = %2X  %2X  %2X  %2X\n", __func__,
		addr_buf[3], addr_buf[2], addr_buf[1], addr_buf[0]);
	TS_LOG_INFO("%s:data_buf[3~1] = %2X  %2X  %2X  %2X\n", __func__,
		data_buf[3], data_buf[2], data_buf[1], data_buf[0]);
	return NO_ERR;
}

int hx_reg_assign(int reg_data, uint8_t *data_buf)
{
	if (data_buf == NULL)
		return HX_ERR;

	data_buf[3] = (uint8_t)((reg_data >> 24) & 0x000000FF);
	data_buf[2] = (uint8_t)((reg_data >> 16) & 0x000000FF);
	data_buf[1] = (uint8_t)((reg_data >> 8) & 0x000000FF);
	data_buf[0] = (uint8_t)(reg_data & 0x000000FF);

	TS_LOG_DEBUG("%s:data_buf[3~1] = %2X  %2X  %2X  %2X\n",\
		__func__, data_buf[3], data_buf[2], data_buf[1], data_buf[0]);

	return NO_ERR;
}
#ifdef ROI
#define HEADER_LEN 4
#define ROW_LEN 7
#define COL_LEN 14
#define OFFSET 8
static int hx_knuckle(void)
{
	unsigned char roi_data[ROI_DATA_READ_LENGTH] = {0};
	int i;
	int j;
	TS_LOG_INFO("%s: Entering!\n", __func__);

	roi_data[0] = g_himax_zf_ts_data->hx_rawdata_buf_roi[5];
	roi_data[1] = g_himax_zf_ts_data->hx_rawdata_buf_roi[4];
	roi_data[2] = g_himax_zf_ts_data->hx_rawdata_buf_roi[7];
	roi_data[3] = g_himax_zf_ts_data->hx_rawdata_buf_roi[6];

	for (i = 0; i < ROW_LEN; i++) {
		for (j = 0; j < COL_LEN; j++) {
			if (j % 2 == 0)
				roi_data[COL_LEN * i + j + HEADER_LEN] =
					g_himax_zf_ts_data->hx_rawdata_buf_roi[ROW_LEN * j + 2 * i + OFFSET];
			else
				roi_data[COL_LEN * i + j + HEADER_LEN] =
					g_himax_zf_ts_data->hx_rawdata_buf_roi[ROW_LEN * (j - 1) + 2 * i + OFFSET + 1];
		}
	}
	memcpy(g_himax_zf_ts_data->roi_data, roi_data, ROI_DATA_READ_LENGTH);
	return NO_ERR;
}

static unsigned char *hx_roi_rawdata(void)
{
	TS_LOG_INFO("%s:return roi data\n", __func__);
	return g_himax_zf_ts_data->roi_data;
}

static int himax_roi_switch(struct ts_roi_info *info)
{
	int i = 0;

	if (IS_ERR_OR_NULL(info)) {
		TS_LOG_ERR("info invaild\n");
		return -EINVAL;
	}
	switch (info->op_action) {
	case TS_ACTION_READ:
		if (HX_ROI_EN_PSD == hx_zf_getDiagCommand())
			info->roi_switch = HX_ROI_ENABLE;
		else
			info->roi_switch = HX_ROI_DISABLE;
		break;
	case TS_ACTION_WRITE:
		if (!!info->roi_switch) {
			g_zf_diag_command = HX_ROI_EN_PSD;
			g_himax_zf_ts_data->roiflag = HX_ROI_ENABLE;
		} else {
			g_zf_diag_command = HX_ROI_DISABLE;
			g_himax_zf_ts_data->roiflag = HX_ROI_DISABLE;
		}
		hx_zf_diag_register_set((uint8_t)g_zf_diag_command);

		if (!info->roi_switch) {
			for (i = 0; i < ROI_DATA_READ_LENGTH; i++)
				g_himax_zf_ts_data->roi_data[i] = 0;
		}
		break;
	default:
		TS_LOG_ERR("op action invalid : %d\n", info->op_action);
		return -EINVAL;
	}
	return 0;
}
#endif

static int hmx_wakeup_gesture_enable_switch(struct ts_wakeup_gesture_enable_info *info)
{
	return NO_ERR;
}

static int hx_get_glove_switch(u8 *glove_switch)
{
	int ret = 0;
	u8 glove_value = 0;
	uint8_t tmp_addr[4] = {0};
	uint8_t tmp_data[4] = {0};

	hx_reg_assign(ADDR_GLOVE_EN, tmp_addr);
	hx_zf_register_read(tmp_addr, FOUR_BYTE_CMD, tmp_data);

	if (tmp_data[3] == 0xA5 && tmp_data[2] == 0x5A &&
		tmp_data[1] == 0xA5 && tmp_data[0] == 0x5A)
		glove_value = GLOVE_EN;

	*glove_switch = glove_value;

	TS_LOG_INFO("%s:glove value=%d\n", __func__, *glove_switch);
	return ret;
}

static int hx_set_glove_switch(u8 glove_switch)
{
	uint8_t tmp_addr[4] = {0};
	uint8_t tmp_data[4] = {0};
	uint8_t back_data[4] = {0};
	uint8_t retry_cnt = 0;

	TS_LOG_INFO("%s glove_switch :%d .\n", __func__, glove_switch);
	do {//Enable:0x10007F10 = 0xA55AA55A
		if (glove_switch) {
			hx_addr_reg_assign(ADDR_GLOVE_EN, DATA_GLOVE_EN, tmp_addr, tmp_data);
			hx_zf_write_burst(tmp_addr, tmp_data);
		} else {
			hx_addr_reg_assign(ADDR_GLOVE_EN, (int)glove_switch, tmp_addr, tmp_data);
			hx_zf_write_burst(tmp_addr, tmp_data);
		}
		back_data[3] = tmp_data[3];
		back_data[2] = tmp_data[2];
		back_data[1] = tmp_data[1];
		back_data[0] = tmp_data[0];
		hx_zf_register_read(tmp_addr, FOUR_BYTE_CMD, tmp_data);
		retry_cnt++;
	} while ((tmp_data[3] != back_data[3] || tmp_data[2] != back_data[2] || tmp_data[1] != back_data[1]  || tmp_data[0] != back_data[0]) && retry_cnt < MAX_RETRY_CNT);
	TS_LOG_INFO("%s end.\n", __func__);
	return NO_ERR;
}

static int hx_glove_switch(struct ts_glove_info *info)
{
	int ret = 0;

	if (!info) {
		TS_LOG_ERR("%s:info is null\n", __func__);
		ret = -ENOMEM;
		return ret;
	}

	switch (info->op_action) {
	case TS_ACTION_READ:
		ret = hx_get_glove_switch(&info->glove_switch);
		if (ret) {
			TS_LOG_ERR("%s:get glove switch fail,ret=%d\n",
				__func__, ret);
			return ret;
		} else {
			TS_LOG_INFO("%s:glove switch=%d\n",
				__func__, info->glove_switch);
		}
		break;
	case TS_ACTION_WRITE:
		TS_LOG_INFO("%s:glove switch=%d\n",
			__func__, info->glove_switch);
		ret = hx_set_glove_switch(!!info->glove_switch);
		if (ret) {
			TS_LOG_ERR("%s:set glove switch fail, ret=%d\n",
				__func__, ret);
			return ret;
		}
		break;
	default:
		TS_LOG_ERR("%s:invalid op action:%d\n",
			__func__, info->op_action);
		return -EINVAL;
	}
	return 0;
}

static int hx_get_rawdata(struct ts_rawdata_info *info, struct ts_cmd_node *out_cmd)
{
	int retval = NO_ERR;

	if ((info == NULL) || (out_cmd == NULL))
		return HX_ERROR;
	TS_LOG_INFO("%s: Entering\n", __func__);
	info->hybrid_buff[DATA_0] = HX_ZF_RX_NUM;
	info->hybrid_buff[DATA_1] = HX_ZF_TX_NUM;
	himax_zf_int_enable(g_himax_zf_ts_data->tskit_himax_data->ts_platform_data->irq_id, IRQ_DISABLE);
	retval = hx_zf_factory_start(g_himax_zf_ts_data, info);
	TS_LOG_INFO("%s: End\n", __func__);
	himax_zf_int_enable(g_himax_zf_ts_data->tskit_himax_data->ts_platform_data->irq_id, IRQ_ENABLE);

	return retval;
}

static int hx_get_capacitance_test_type(struct ts_test_type_info *info)
{
	struct ts_kit_device_data *chip_data = NULL;

	chip_data = g_himax_zf_ts_data->tskit_himax_data->ts_platform_data->chip_data;

	TS_LOG_INFO("%s enter\n", __func__);
	if (!info) {
		TS_LOG_ERR("%s\n", __func__);
		return INFO_FAIL;
	}
	memcpy(info->tp_test_type, chip_data->tp_test_type, TS_CAP_TEST_TYPE_LEN);
	TS_LOG_INFO("%s:test_type=%s\n", __func__, info->tp_test_type);
	return NO_ERR;
}

static int hx_irq_top_half(struct ts_cmd_node *cmd)
{
	 cmd->command = TS_INT_PROCESS;
	 TS_LOG_DEBUG("%s\n", __func__);
	 return NO_ERR;
}

int hx_zf_input_config(struct input_dev *input_dev)
{
	TS_LOG_INFO("%s: called\n", __func__);
	if (input_dev == NULL)
		return HX_ERROR;

	g_himax_zf_ts_data->input_dev = input_dev;

	set_bit(EV_SYN, input_dev->evbit);
	set_bit(EV_ABS, input_dev->evbit);
	set_bit(EV_KEY, input_dev->evbit);
	set_bit(BTN_TOUCH, input_dev->keybit);
	set_bit(TS_DOUBLE_CLICK, input_dev->keybit);
	set_bit(INPUT_PROP_DIRECT, input_dev->propbit);
	if (g_himax_zf_ts_data->esd_palm_iron_support)
		set_bit(TS_KEY_IRON, input_dev->keybit);

	TS_LOG_INFO("input_set_abs_params: min_x %d, max_x %d, min_y %d, max_y %d\n",
		g_himax_zf_ts_data->pdata->abs_x_min, g_himax_zf_ts_data->pdata->abs_x_max,
		g_himax_zf_ts_data->pdata->abs_y_min, g_himax_zf_ts_data->pdata->abs_y_max);
	input_set_abs_params(input_dev, ABS_MT_POSITION_X, g_himax_zf_ts_data->pdata->abs_x_min,
		g_himax_zf_ts_data->pdata->abs_x_max, g_himax_zf_ts_data->pdata->abs_x_fuzz, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y, g_himax_zf_ts_data->pdata->abs_y_min,
		g_himax_zf_ts_data->pdata->abs_y_max, g_himax_zf_ts_data->pdata->abs_y_fuzz, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, g_himax_zf_ts_data->pdata->abs_pressure_min,
		g_himax_zf_ts_data->pdata->abs_pressure_max, g_himax_zf_ts_data->pdata->abs_pressure_fuzz, 0);
	input_set_abs_params(input_dev, ABS_MT_TRACKING_ID, 0, g_himax_zf_ts_data->nFinger_support, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_PRESSURE, g_himax_zf_ts_data->pdata->abs_pressure_min,
		g_himax_zf_ts_data->pdata->abs_pressure_max, g_himax_zf_ts_data->pdata->abs_pressure_fuzz, 0);
	input_set_abs_params(input_dev, ABS_MT_WIDTH_MAJOR, g_himax_zf_ts_data->pdata->abs_width_min,
		g_himax_zf_ts_data->pdata->abs_width_max, g_himax_zf_ts_data->pdata->abs_pressure_fuzz, 0);
	return NO_ERR;
}

int hx_reset_device(void)
{
	int retval = NO_ERR;

#if defined(HUAWEI_CHARGER_FB)
	struct ts_kit_device_data *ts;
	struct ts_feature_info *info;

	ts = g_himax_zf_ts_data->tskit_himax_data;
	info = &ts->ts_platform_data->feature_info;
#endif
	hw_reset_activate = 1;
	TS_LOG_INFO("%s: Now pin reset the Touch chip.\n", __func__);

	himax_zf_rst_gpio_set(g_himax_zf_ts_data->rst_gpio, 0);
	usleep_range(RST_LOW_PERIOD_S, RST_LOW_PERIOD_E);
	himax_zf_rst_gpio_set(g_himax_zf_ts_data->rst_gpio, 1);
	usleep_range(RST_HIGH_PERIOD_S, RST_HIGH_PERIOD_E);

	return retval;
}

static void calcDataSize(uint8_t finger_num)
{
	struct himax_ts_data *ts_data = g_himax_zf_ts_data;

	ts_data->coord_data_size = HX_COORD_BYTE_NUM * finger_num;// 1 coord 4 bytes.
	ts_data->area_data_size = ((finger_num / HX_COORD_BYTE_NUM) + (finger_num % HX_COORD_BYTE_NUM ? 1 : 0)) * HX_COORD_BYTE_NUM;// 1 area 4 finger ?
	ts_data->raw_data_frame_size = HX_RECEIVE_BUF_MAX_SIZE - ts_data->coord_data_size - ts_data->area_data_size - 4 - 4 - 1;
	//check if devided by zero
	if (ts_data->raw_data_frame_size == 0) {
		TS_LOG_ERR("%s divided by zero\n", __func__);
		return;
	}
	ts_data->raw_data_nframes  = ((uint32_t)ts_data->x_channel * ts_data->y_channel + ts_data->x_channel + ts_data->y_channel) / ts_data->raw_data_frame_size +
						(((uint32_t)ts_data->x_channel * ts_data->y_channel + ts_data->x_channel + ts_data->y_channel) % ts_data->raw_data_frame_size) ? 1 : 0;
	TS_LOG_INFO("%s: coord_data_size: %d, area_data_size:%d, raw_data_frame_size:%d, raw_data_nframes:%d", __func__,
				ts_data->coord_data_size, ts_data->area_data_size, ts_data->raw_data_frame_size, ts_data->raw_data_nframes);
}

static void calculate_point_number(void)
{
	HX_TOUCH_INFO_POINT_CNT = HX_ZF_MAX_PT * HX_COORD_BYTE_NUM;

	if ((HX_ZF_MAX_PT % 4) == 0)
		HX_TOUCH_INFO_POINT_CNT += (HX_ZF_MAX_PT / HX_COORD_BYTE_NUM) * HX_COORD_BYTE_NUM;
	else
		HX_TOUCH_INFO_POINT_CNT += ((HX_ZF_MAX_PT / HX_COORD_BYTE_NUM) + 1) * HX_COORD_BYTE_NUM;
}

static void hx_touch_information(void)
{
	struct ts_kit_device_data *chip_data = NULL;
	struct himax_platform_data *pdata = NULL;

	chip_data = g_himax_zf_ts_data->tskit_himax_data->ts_platform_data->chip_data;
	pdata = g_himax_zf_ts_data->pdata;

	if (IC_ZF_TYPE == HX_83108A_SERIES_PWON) {
		if (chip_data->rx_num)
			HX_ZF_RX_NUM = chip_data->rx_num;
		else
			HX_ZF_RX_NUM = HX83108A_RX_NUM;

		if (chip_data->tx_num)
			HX_ZF_TX_NUM = chip_data->tx_num;
		else
			HX_ZF_TX_NUM = HX83108A_TX_NUM;
		HX_ZF_BT_NUM = HX83108A_BT_NUM;
		if (pdata->screenWidth)
			HX_ZF_X_RES = pdata->screenWidth;
		else
			HX_ZF_X_RES = HX83108A_X_RES;
		if (pdata->screenHeight)
			HX_ZF_Y_RES = pdata->screenHeight;
		else
			HX_ZF_Y_RES = HX83108A_Y_RES;
		if (pdata->max_pt)
			HX_ZF_MAX_PT = pdata->max_pt;
		else
			HX_ZF_MAX_PT = HX83108A_MAX_PT;
		HX_ZF_XY_REVERSE = false;
	} else {
		HX_ZF_RX_NUM = 0;
		HX_ZF_TX_NUM = 0;
		HX_ZF_BT_NUM = 0;
		HX_ZF_X_RES = 0;
		HX_ZF_Y_RES = 0;
		HX_ZF_MAX_PT = 0;
		HX_ZF_XY_REVERSE = false;
	}

	hx_zf_setXChannel(HX_ZF_RX_NUM); // X channel
	hx_zf_setYChannel(HX_ZF_TX_NUM); // Y channel
	TS_LOG_INFO("%s:HX_RX_NUM =%d,HX_TX_NUM =%d,HX_MAX_PT=%d\n", __func__, HX_ZF_RX_NUM, HX_ZF_TX_NUM, HX_ZF_MAX_PT);
	TS_LOG_INFO("%s:HX_XY_REVERSE =%d,HX_Y_RES =%d,HX_X_RES=%d\n", __func__, HX_ZF_XY_REVERSE, HX_ZF_Y_RES, HX_ZF_X_RES);
}

void hx_zf_get_information(void)
{
	TS_LOG_INFO("%s:enter\n", __func__);
	hx_touch_information();
	TS_LOG_INFO("%s:exit\n", __func__);
}

/*int_off:  false: before reset, need disable irq; true: before reset, don't need disable irq.*/
/*loadconfig:  after reset, load config or not.*/
int hx_zf_hw_reset(bool loadconfig, bool int_off)
{
	int retval = 0;
#if defined(HUAWEI_CHARGER_FB)
	struct ts_kit_device_data *ts;
	struct ts_feature_info *info;

	ts = g_himax_zf_ts_data->tskit_himax_data;
	info = &ts->ts_platform_data->feature_info;
#endif
	hw_reset_activate = 1;
	TS_LOG_INFO("%s: Now reset the Touch chip.\n", __func__);

	himax_zf_rst_gpio_set(g_himax_zf_ts_data->rst_gpio, 0);
	usleep_range(RST_LOW_PERIOD_S, RST_LOW_PERIOD_E);
	himax_zf_rst_gpio_set(g_himax_zf_ts_data->rst_gpio, 1);
	usleep_range(RST_HIGH_PERIOD_S, RST_HIGH_PERIOD_E);
	hx_reload_to_active();

	if (loadconfig)
		hx_zf_get_information();
	if (retval < 0)
		return retval;

	return NO_ERR;
}

static bool hx_ic_package_check(void)
{
	int i = 0;
	int retry = 3;
	uint8_t ret_data = 0x00;
	uint8_t tmp_addr[4] = {0};
	uint8_t tmp_data[4] = {0};

	for (i = 0; i < retry; i++) {
		// Product ID
		// Touch
		tmp_addr[3] = (uint8_t)hx_id_addr[3];
		tmp_addr[2] = (uint8_t)hx_id_addr[2];
		tmp_addr[1] = (uint8_t)hx_id_addr[1];
		tmp_addr[0] = (uint8_t)hx_id_addr[0];

		TS_LOG_INFO("%s:tmp_addr = %X,%X,%X,%X\n", __func__,
			tmp_addr[DATA_0], tmp_addr[DATA_1], tmp_addr[DATA_2], tmp_addr[DATA_3]);
		hx_zf_register_read(tmp_addr, FOUR_BYTE_CMD, tmp_data);

		IC_ZF_CHECKSUM = HX_TP_BIN_CHECKSUM_CRC;

		/*Himax: Set FW and CFG Flash Address*/
		/*parse from dts file default setting*/
		ZF_FW_VER_MAJ_FLASH_ADDR = hx_flash_addr[0];
		ZF_FW_VER_MAJ_FLASH_LENG = hx_flash_addr[1];
		ZF_FW_VER_MIN_FLASH_ADDR = hx_flash_addr[2];
		ZF_FW_VER_MIN_FLASH_LENG = hx_flash_addr[3];
		ZF_CFG_VER_MAJ_FLASH_ADDR = hx_flash_addr[4];
		ZF_CFG_VER_MAJ_FLASH_LENG = hx_flash_addr[5];
		ZF_CFG_VER_MIN_FLASH_ADDR = hx_flash_addr[6];
		ZF_CFG_VER_MIN_FLASH_LENG = hx_flash_addr[7];
		ZF_CID_VER_MAJ_FLASH_ADDR = hx_flash_addr[8];
		ZF_CID_VER_MAJ_FLASH_LENG = hx_flash_addr[9];
		ZF_CID_VER_MIN_FLASH_ADDR = hx_flash_addr[10];
		ZF_CID_VER_MIN_FLASH_LENG = hx_flash_addr[11];

		TS_LOG_INFO("%s:tmp_data = %X,%X,%X,%X\n", __func__,
			tmp_data[DATA_0], tmp_data[DATA_1], tmp_data[DATA_2], tmp_data[DATA_3]);

		if ((tmp_data[DATA_3] == HX_83108A_ID_PART_1) &&
			(tmp_data[DATA_2] == HX_83108A_ID_PART_2) &&
			(tmp_data[DATA_1] == HX_83108A_ID_PART_3)) {
			IC_ZF_TYPE = HX_83108A_SERIES_PWON;
			ret_data = IC_PACK_CHECK_SUCC;
			TS_LOG_INFO("Hx IC package 83108A series\n");
		} else {
			ret_data = IC_PACK_CHECK_FAIL;
			TS_LOG_ERR("%s:ID match fail!\n", __func__);
		}

		if (ret_data == IC_PACK_CHECK_SUCC)
			break;
	}

	if (ret_data != IC_PACK_CHECK_SUCC)
		return IC_PACK_CHECK_FAIL;

	// 4. After read finish, set initial register from driver
	hx_addr_reg_assign(ADDR_SORTING_MODE_SWITCH, DATA_INIT, &tmp_addr[0], &tmp_data[0]);
	himax_zf_register_write(tmp_addr, FOUR_BYTE_CMD, tmp_data);
	hx_addr_reg_assign(ADDR_DIAG_REG_SET, DATA_INIT, &tmp_addr[0], &tmp_data[0]);
	himax_zf_register_write(tmp_addr, FOUR_BYTE_CMD, tmp_data);
	hx_addr_reg_assign(ADDR_NFRAME_SEL, DATA_INIT, &tmp_addr[0], &tmp_data[0]);
	himax_zf_register_write(tmp_addr, FOUR_BYTE_CMD, tmp_data);
	hx_addr_reg_assign(ADDR_SWITCH_FLASH_RLD, DATA_INIT, &tmp_addr[0], &tmp_data[0]);
	himax_zf_register_write(tmp_addr, FOUR_BYTE_CMD, tmp_data);
	TS_LOG_ERR("%s:ret_data = %u\n", __func__, ret_data);
	return ret_data;
}

void himax_zf_read_FW_info(void)
{
	uint8_t cmd[4] = {0};
	uint8_t data[64] = {0};

	//=====================================
	// Read FW version : 0x1000_7004  but 05,06 are the real addr for FW Version
	//=====================================

	hx_reg_assign(ADDR_READ_FW_VER, cmd);
	hx_zf_register_read(cmd, FOUR_BYTE_CMD, data);

	g_himax_zf_ts_data->vendor_panel_ver = data[0];
	g_himax_zf_ts_data->vendor_fw_ver = data[1] << 8 | data[2];

	TS_LOG_INFO("PANEL_VER : 0x%2.2X\n", g_himax_zf_ts_data->vendor_panel_ver);
	TS_LOG_INFO("FW_VER : 0x%2.2X\n", g_himax_zf_ts_data->vendor_fw_ver);

	hx_reg_assign(ADDR_READ_CONFIG_VER, cmd);
	hx_zf_register_read(cmd, FOUR_BYTE_CMD, data);

	g_himax_zf_ts_data->vendor_config_ver = data[2] << 8 | data[3];

	TS_LOG_INFO("CFG_VER : 0x%2.2X\n", g_himax_zf_ts_data->vendor_config_ver);

	hx_reg_assign(ADDR_READ_CID_VER, cmd);
	hx_zf_register_read(cmd, FOUR_BYTE_CMD, data);

	g_himax_zf_ts_data->vendor_cid_maj_ver = data[2];
	g_himax_zf_ts_data->vendor_cid_min_ver = data[3];

	TS_LOG_INFO("CID_VER : 0x%2.2X\n", (g_himax_zf_ts_data->vendor_cid_maj_ver << 8 | g_himax_zf_ts_data->vendor_cid_min_ver));
}

#ifdef HX_ESD_WORKAROUND
static int himax_mcu_ic_test_recovery(void)
{
	uint8_t tmp_addr[4] = {0};
	uint8_t tmp_data[4] = {0};
	uint8_t tmp_read[4] = {0};
	int retry = 0;

	// unlock
	hx_reg_assign(0x9000009C, tmp_addr);
	hx_reg_assign(0x000000DD, tmp_data);
	himax_zf_register_write(tmp_addr, 4, tmp_data);
	usleep_range(20000, 20001);

	hx_reg_assign(0x90000280, tmp_addr);
	hx_reg_assign(0x000000A5, tmp_data);
	himax_zf_register_write(tmp_addr, 4, tmp_data);
	usleep_range(20000, 20001);

	hx_reg_assign(0x300B9000, tmp_addr);
	hx_reg_assign(0x8A108300, tmp_data);
	himax_zf_register_write(tmp_addr, 4, tmp_data);
	usleep_range(20000, 20001);

	hx_reg_assign(0x300EB000, tmp_addr);
	hx_reg_assign(0xCC665500, tmp_data);
	himax_zf_register_write(tmp_addr, 4, tmp_data);
	usleep_range(20000, 20001);

	hx_reg_assign(0x300EB004, tmp_addr);
	hx_reg_assign(0x5AB395C6, tmp_data);
	himax_zf_register_write(tmp_addr, 4, tmp_data);
	usleep_range(20000, 20001);

	do {
		// read 0A
		hx_reg_assign(0x3000A001, tmp_addr);
		hx_zf_register_read(tmp_addr, 4, tmp_read);
		TS_LOG_INFO("retry read 0A register = %d, R%02X%02X%02X%02XH = 0x%02X%02X%02X%02X\n",
				retry, tmp_addr[3], tmp_addr[2], tmp_addr[1], tmp_addr[0],
				tmp_read[3], tmp_read[2], tmp_read[1], tmp_read[0]);

	} while ((retry++ < 5) && (tmp_read[0] != 0x9D));

	if (retry == 0x06) {
		TS_LOG_INFO("retry = 5, return\n");
		if ((tmp_read[0] == 0xFF) || (tmp_read[0] == 0x00)) {
			hx_reset_device(); // bus fail
			hw_reset_activate = 0;
			ESD_RESET_ACTIVATE = 0;
		}
		return HX_ERROR;
	}
	retry = 0;
	do {
		hx_reg_assign(0x300B8800, tmp_addr);
		hx_reg_assign(0x000001B8, tmp_data);
		himax_zf_register_write(tmp_addr, 4, tmp_data);
		usleep_range(10000, 10100);

		// read 0A
		hx_reg_assign(0x3000A001, tmp_addr);
		hx_zf_register_read(tmp_addr, 4, tmp_read);
		TS_LOG_INFO("retry = %d, R%02X%02X%02X%02XH = 0x%02X%02X%02X%02X\n",
				retry, tmp_addr[3], tmp_addr[2], tmp_addr[1], tmp_addr[0],
				tmp_read[3], tmp_read[2], tmp_read[1], tmp_read[0]);

		if (tmp_addr[0] != 0x9D)
			return HX_OK;
	} while (retry++ < 5);
	return HX_ERROR;
}

static void ESD_HW_REST(void)
{
	int ret;

	ESD_RESET_ACTIVATE = 1;
	g_zero_event_count = 0;
#if defined (CONFIG_HUAWEI_DSM)
	ts_dmd_report(DSM_TP_ESD_ERROR_NO, "HX chip esd reset\n");
#endif
	ret = himax_mcu_ic_test_recovery();
	if (!ret)
		TS_LOG_INFO("ESD_HX TP: ESD - Reset\n");
}
#endif

static void hx_get_rawdata_from_event(int RawDataLen, int hx_touch_info_size, int mul_num, int sel_num, int index, uint8_t *buff)
{
	int i = 0;
	int temp1 = 0;
	int temp2 = 0;
	uint8_t *buf = NULL;

	if (buff == NULL)
		return;

	buf = kzalloc(sizeof(uint8_t) * (HX_RECEIVE_BUF_MAX_SIZE - hx_touch_info_size), GFP_KERNEL);
	if (buf == NULL)
		return;

	memcpy(buf, &buff[hx_touch_info_size], (HX_RECEIVE_BUF_MAX_SIZE - hx_touch_info_size));

	for (i = 0; i < RawDataLen; i++) {
		temp1 = index + i;
		if (temp1 < mul_num)//mutual
			mutual_data[index + i] = buf[i * 2 + 4 + 1] * 256 + buf[i * 2 + 4];

		else {//self
			temp1 = i + index;
			temp2 = mul_num + sel_num;
			if (temp1 >= temp2)
				break;

			self_data[i + index - mul_num] = (buf[i * 2 + 4 + 1] << 8) + (buf[i * 2 + 4]);//4: RawData Header
		}
	}

	kfree(buf);
	buf = NULL;
}

static bool header_format_check(uint8_t *buf, int buf_len)
{
	if (buf_len != HX_RECEIVE_ROI_BUF_MAX_SIZE) {
		TS_LOG_ERR("%s: buf_len = %d, invalid buf len!", __func__, buf_len);
		return false;
	}
	return (buf[HX_TOUCH_SIZE] == 0xA3) && (buf[HX_TOUCH_SIZE + 1] == 0x3A) &&
		(buf[HX_RECEIVE_ROI_BUF_MAX_SIZE - 2] != 0) && (buf[HX_RECEIVE_ROI_BUF_MAX_SIZE - 1] != 0);
}

static int hx_start_get_roi_data_from_event(int hx_touch_info_size, uint8_t *buf, int buf_len)
{
	int i;
	int check_sum_cal;
	int retval = NO_ERR;

	if (!header_format_check(buf, buf_len)) {
		TS_LOG_ERR("confirm the firmware format, header format is wrong!\n");
		retval = HX_ERROR;
		return retval;
	}

	for (i = 0, check_sum_cal = 0; i < (HX_RECEIVE_ROI_BUF_MAX_SIZE - HX_TOUCH_SIZE); i = i + 2)
		check_sum_cal += (buf[i + 56 + 1] * 256 + buf[i + 56]);

	if (check_sum_cal % 0x10000 != 0) {
		TS_LOG_ERR("fail,  check_sum_cal: %d\n", check_sum_cal);
		retval = HX_ERROR;
		return retval;
	}
#ifdef ROI
	if (hx_zf_getDiagCommand() == HX_ROI_EN_PSD)
		hx_knuckle();
#endif
	return retval;
}

static int hx_start_get_rawdata_from_event(int hx_touch_info_size, int RawDataLen, uint8_t *buf)
{
	int i = 0;
	int index = 0;
	int check_sum_cal = 0;
	int mul_num = 0;
	int self_num = 0;
	int retval = NO_ERR;

	if ((buf == NULL) || (hx_touch_info_size > HX_RECEIVE_BUF_MAX_SIZE - 3))
		return HX_ERROR;

	for (i = 0, check_sum_cal = 0; i < (HX_RECEIVE_BUF_MAX_SIZE - hx_touch_info_size); i = i+2)
		check_sum_cal += (buf[i + hx_touch_info_size + 1]*256 + buf[i + hx_touch_info_size]);

	if  (check_sum_cal % 0x10000 != 0) {
		TS_LOG_ERR("fail,  check_sum_cal: %d\n", check_sum_cal);
		retval = HX_ERROR;
		return retval;
	}

	mutual_data = hx_zf_getMutualBuffer();
	self_data = hx_zf_getSelfBuffer();
	mul_num = hx_zf_getXChannel() * hx_zf_getYChannel();
	self_num = hx_zf_getXChannel() + hx_zf_getYChannel();
	//header format check
	if ((buf[hx_touch_info_size] == 0x3A) &&
		(buf[hx_touch_info_size + 1] == 0xA3) &&
		(buf[hx_touch_info_size + 2] > 0)) {
		RawDataLen = RawDataLen / 2;
		index = (buf[hx_touch_info_size+2] - 1) * RawDataLen;
		hx_get_rawdata_from_event(RawDataLen, hx_touch_info_size,  mul_num, self_num, index, buf);
	} else {
		TS_LOG_INFO("[HX TP MSG]%s: header format is wrong!\n", __func__);
		retval = HX_ERROR;
		return retval;
	}

	return retval;
}
#ifdef HX_ESD_WORKAROUND
static int hx_check_report_data_for_esd(int hx_touch_info_size, uint8_t *buf)
{
	int i = 0;
	int retval = 0;

	if (buf == NULL) {
		TS_LOG_ERR("%s buf pointer NULL!\n", __func__);
		return HX_ERROR;
	}
	for (i = 0; i < hx_touch_info_size; i++) {
		if (buf[i] == ESD_EVENT_ALL_ZERO) { // case 2 ESD recovery flow-Disable
			retval = ESD_ALL_ZERO_BAK_VALUE; // if hand shanking fail,firmware error
		} else if (buf[i] == ESD_EVENT_ALL_ED) { /* case 1 ESD recovery flow */
			retval = ESD_ALL_ED_BAK_VALUE; // ESD event,ESD reset
		} else if (buf[i] == ESD_EVENT_ALL_EB) { /* case 1 ESD recovery flow */
			retval = ESD_ALL_EB_BAK_VALUE; // ESD event,ESD reset
		} else if (buf[i] == ESD_EVENT_ALL_EC) { /* case 1 ESD recovery flow */
			retval = ESD_ALL_EC_BAK_VALUE; // ESD event,ESD reset
		} else {
			retval = 0;
			g_zero_event_count = 0;
			break;
		}
	}
	return retval;
}
#endif

static void himax_debug_level_print(int debug_mode, int status, int hx_touch_info_size, struct himax_touching_data hx_touching, uint8_t *buf)
{
	uint32_t m = (uint32_t)hx_touch_info_size;

	if (buf == NULL)
		return;

	switch (debug_mode)	{
	case 0:
		for (hx_touching.loop_i = 0; hx_touching.loop_i < m; hx_touching.loop_i++) {
			TS_LOG_INFO("0x%2.2X ", buf[hx_touching.loop_i]);
			if (hx_touching.loop_i % 8 == 7)
				printk("\n");
		}
	break;
	case 1:
		TS_LOG_INFO("Finger %d=> W:%d, Z:%d, F:%d, N:%d\n",
		hx_touching.loop_i + 1, hx_touching.w, hx_touching.w, hx_touching.loop_i + 1, EN_NoiseFilter);
	break;
	case 2:
	break;
	case 3:
		if (status == 0) {//reporting down
			if ((hx_touching.old_finger >> hx_touching.loop_i) == 0) {
				if (g_himax_zf_ts_data->useScreenRes) {
						TS_LOG_INFO("Screen:F:%02d Down, W:%d, N:%d\n",
						hx_touching.loop_i+1,  hx_touching.w, EN_NoiseFilter);
				} else {
						TS_LOG_INFO("Raw:F:%02d Down, W:%d, N:%d\n",
						hx_touching.loop_i+1, hx_touching.w, EN_NoiseFilter);
				}
			}
		} else if (status == 1) {//reporting up
			if ((hx_touching.old_finger >> hx_touching.loop_i) == 1) {
				if (g_himax_zf_ts_data->useScreenRes) {
					TS_LOG_INFO("Screen:F:%02d Up, X:%d, Y:%d, N:%d\n",
					hx_touching.loop_i+1, (g_himax_zf_ts_data->pre_finger_data[hx_touching.loop_i][0] * g_himax_zf_ts_data->widthFactor) >> SHIFTBITS,
					(g_himax_zf_ts_data->pre_finger_data[hx_touching.loop_i][1] * g_himax_zf_ts_data->heightFactor) >> SHIFTBITS, Last_EN_NoiseFilter);
				} else {
					TS_LOG_INFO("Raw:F:%02d Up, X:%d, Y:%d, N:%d\n",
					hx_touching.loop_i + 1, g_himax_zf_ts_data->pre_finger_data[hx_touching.loop_i][0],
					g_himax_zf_ts_data->pre_finger_data[hx_touching.loop_i][1], Last_EN_NoiseFilter);
				}
			}
		} else if (status == 2) {//all leave event
			for (hx_touching.loop_i = 0; hx_touching.loop_i < g_himax_zf_ts_data->nFinger_support && (g_himax_zf_ts_data->debug_log_level & BIT(3)) > 0; hx_touching.loop_i++) {
				if (((g_himax_zf_ts_data->pre_finger_mask >> hx_touching.loop_i) & 1) == 1) {
					if (g_himax_zf_ts_data->useScreenRes) {
							TS_LOG_INFO("status:%X, Screen:F:%02d Up, X:%d, Y:%d, N:%d\n", 0,
							hx_touching.loop_i + 1, (g_himax_zf_ts_data->pre_finger_data[hx_touching.loop_i][0] * g_himax_zf_ts_data->widthFactor) >> SHIFTBITS,
							(g_himax_zf_ts_data->pre_finger_data[hx_touching.loop_i][1] * g_himax_zf_ts_data->heightFactor) >> SHIFTBITS, Last_EN_NoiseFilter);
					} else {
						TS_LOG_INFO("status:%X, Raw:F:%02d Up, X:%d, Y:%d, N:%d\n", 0,
							hx_touching.loop_i + 1, g_himax_zf_ts_data->pre_finger_data[hx_touching.loop_i][0],
							g_himax_zf_ts_data->pre_finger_data[hx_touching.loop_i][1], Last_EN_NoiseFilter);
					}
				}
			}
			g_himax_zf_ts_data->pre_finger_mask = 0;
		}
	break;
	default:
	break;
	}
}

static int himax_checksum_cal(int hx_touch_info_size, struct himax_touching_data hx_touching, uint8_t *buf)
{
	int checksum = 0;
	uint32_t m = (uint32_t)hx_touch_info_size;

	if (buf == NULL)
		return checksum;

	for (hx_touching.loop_i = 0; hx_touching.loop_i < m; hx_touching.loop_i++)
		checksum += buf[hx_touching.loop_i];

	return checksum;
}

static void hx_parse_coords(int hx_touch_info_size, int hx_point_num, struct ts_fingers *info,
	struct himax_touching_data hx_touching, uint8_t *buf)
{
	int m = 0;
	int m1 = 0;
	int m2 = 0;
	int base = 0;
	uint8_t coordInfoSize = g_himax_zf_ts_data->coord_data_size + g_himax_zf_ts_data->area_data_size + 4;

	TS_LOG_DEBUG("%s enter\n", __func__);

	if (buf == NULL || info == NULL)
		return ;

	if (hx_point_num != 0) {
		hx_touching.old_finger = g_himax_zf_ts_data->pre_finger_mask;
		g_himax_zf_ts_data->pre_finger_mask = 0;
		hx_touching.finger_num =  buf[coordInfoSize - 4] & 0x0F;

		for (hx_touching.loop_i = 0; hx_touching.loop_i < g_himax_zf_ts_data->nFinger_support; hx_touching.loop_i++) {
			base = hx_touching.loop_i * 4;//every finger coordinate need 4 bytes.
			m = base + 1;
			m1 = base + 2;
			m2 = base + 3;
			hx_touching.x = ((buf[base]) << 8) | (buf[m]);
			hx_touching.y = ((buf[m1]) << 8) | (buf[m2]);
			hx_touching.w = 10;

			if (hx_touching.x >= 0 && hx_touching.x <= g_himax_zf_ts_data->pdata->abs_x_max && hx_touching.y >= 0 && hx_touching.y <= g_himax_zf_ts_data->pdata->abs_y_max) {
				if ((g_himax_zf_ts_data->debug_log_level & BIT(3)) > 0)//debug 3: print finger coordinate information
					himax_debug_level_print(3, 0, hx_touch_info_size, hx_touching, buf); //status = report down

				info->fingers[hx_touching.loop_i].status = TS_FINGER_PRESS;
				info->fingers[hx_touching.loop_i].x = hx_touching.x;
				info->fingers[hx_touching.loop_i].y = hx_touching.y;
				info->fingers[hx_touching.loop_i].major = 255;
				info->fingers[hx_touching.loop_i].minor = 255;
				info->fingers[hx_touching.loop_i].pressure = hx_touching.w;

				if (!g_himax_zf_ts_data->first_pressed)
					g_himax_zf_ts_data->first_pressed = 1;//first report

				g_himax_zf_ts_data->pre_finger_data[hx_touching.loop_i][0] = hx_touching.x;
				g_himax_zf_ts_data->pre_finger_data[hx_touching.loop_i][1] = hx_touching.y;

				if (g_himax_zf_ts_data->debug_log_level & BIT(1))
					himax_debug_level_print(1, 0, hx_touch_info_size, hx_touching, buf);  //status useless

				g_himax_zf_ts_data->pre_finger_mask = g_himax_zf_ts_data->pre_finger_mask + (1 << hx_touching.loop_i);
			} else {
				if (hx_touching.loop_i == 0 && g_himax_zf_ts_data->first_pressed == 1) {
					g_himax_zf_ts_data->first_pressed = 2;
					TS_LOG_DEBUG("E1@%d, %d\n",
					g_himax_zf_ts_data->pre_finger_data[0][0], g_himax_zf_ts_data->pre_finger_data[0][1]);
				}
				if ((g_himax_zf_ts_data->debug_log_level & BIT(3)) > 0)
					himax_debug_level_print(3, 1, hx_touch_info_size, hx_touching, buf); //status= report up
			}
		}
	} else {
		for (hx_touching.loop_i = 0; hx_touching.loop_i < g_himax_zf_ts_data->nFinger_support; hx_touching.loop_i++) {
				if (((g_himax_zf_ts_data->pre_finger_mask >> hx_touching.loop_i) & 1) == 1) {
					info->fingers[hx_touching.loop_i].status = TS_FINGER_RELEASE;
					info->fingers[hx_touching.loop_i].x = 0;
					info->fingers[hx_touching.loop_i].y = 0;
					info->fingers[hx_touching.loop_i].major = 0;
					info->fingers[hx_touching.loop_i].minor = 0;
					info->fingers[hx_touching.loop_i].pressure = 0;
				}
			}
		if (g_himax_zf_ts_data->pre_finger_mask > 0) {
			himax_debug_level_print(3, 3, hx_touch_info_size, hx_touching, buf);  //all leave event
		}

		if (g_himax_zf_ts_data->first_pressed == 1) {
			g_himax_zf_ts_data->first_pressed = 2;
			TS_LOG_DEBUG("E1@%d, %d\n", g_himax_zf_ts_data->pre_finger_data[0][0], g_himax_zf_ts_data->pre_finger_data[0][1]);
		}

		if (g_himax_zf_ts_data->debug_log_level & BIT(1))
			TS_LOG_INFO("All Finger leave\n");
	}
}

static void hx_set_smwp_enable(uint8_t smwp_enable)
{
	uint8_t tmp_addr[4] = {0};
	uint8_t tmp_data[4] = {0};
	uint8_t back_data[4] = {0};
	uint8_t retry_cnt = 0;
	TS_LOG_INFO("%s enter\n", __func__);
	do { // Enable:0x10007F10 = 0xA55AA55A
		if (smwp_enable) {
			hx_addr_reg_assign(ADDR_SMWP_EN, DATA_SMWP_EN, tmp_addr, tmp_data);
			hx_zf_write_burst(tmp_addr, tmp_data);
			TS_LOG_INFO("smwp_enable\n");
		} else {
			hx_addr_reg_assign(ADDR_SMWP_EN, (int)smwp_enable, tmp_addr, tmp_data);
			hx_zf_write_burst(tmp_addr, tmp_data);
			TS_LOG_INFO("smwp_disable\n");
		}
		back_data[3] = tmp_data[3];
		back_data[2] = tmp_data[2];
		back_data[1] = tmp_data[1];
		back_data[0] = tmp_data[0];
		hx_zf_register_read(tmp_addr, FOUR_BYTE_CMD, tmp_data);
		retry_cnt++;
	} while ((tmp_data[3] != back_data[3] || tmp_data[2] != back_data[2] || tmp_data[1] != back_data[1] || tmp_data[0] != back_data[0]) && retry_cnt < MAX_RETRY_CNT);
}

static void gest_pt_log_coordinate(int rx, int tx)
{
	gest_pt_x[gest_pt_cnt] = rx * HX_ZF_X_RES / 255;
	gest_pt_y[gest_pt_cnt] = tx * HX_ZF_Y_RES / 255;
}

static int easy_wakeup_gesture_report_coordinate(unsigned int reprot_gesture_point_num, struct ts_fingers *info, uint8_t *buf)
{
	int i = 0;
	int retval = 0;
	int tmp_max_x = 0x00;
	int tmp_min_x = 0xFFFF;
	int tmp_max_y = 0x00;
	int tmp_min_y = 0xFFFF;
	int gest_len = 0;
	int max_high_index = 0;
	int max_low_index = 0;
	int max_left_index = 0;
	int max_right_index = 0;

	if (reprot_gesture_point_num != 0) {
		/*
		 *The most points num is 6,point from 1(lower address) to 6(higher address) means:
		 *1.beginning 2.end 3.top 4.leftmost 5.bottom 6.rightmost
		 */
		if (buf[GEST_PTLG_ID_LEN] == GEST_PTLG_HDR_ID1 && buf[GEST_PTLG_ID_LEN + 1] == GEST_PTLG_HDR_ID2) {
			gest_len = buf[GEST_PTLG_ID_LEN + 2];
			i = 0;
			gest_pt_cnt = 0;
			while (i < (gest_len + 1) / 2) {
				gest_pt_log_coordinate(buf[GEST_PTLG_ID_LEN + 4 + i * 2], buf[GEST_PTLG_ID_LEN + 4 + i * 2 + 1]);
				i++;
				TS_LOG_DEBUG("gest_pt_x[%d]=%d gest_pt_y[%d]=%d\n", gest_pt_cnt, gest_pt_x[gest_pt_cnt], gest_pt_cnt, gest_pt_y[gest_pt_cnt]);
				gest_pt_cnt += 1;
			}
			if (gest_pt_cnt) {
				for (i = 0; i < gest_pt_cnt; i++) {
					if (tmp_max_x < gest_pt_x[i]) {
						tmp_max_x = gest_pt_x[i];
						max_right_index = i;
					}
					if (tmp_min_x > gest_pt_x[i]) {
						tmp_min_x = gest_pt_x[i];
						max_left_index = i;
					}
					if (tmp_max_y < gest_pt_y[i]) {
						tmp_max_y = gest_pt_y[i];
						max_high_index = i;
					}
					if (tmp_min_y > gest_pt_y[i]) {
						tmp_min_y = gest_pt_y[i];
						max_low_index = i;
					}
				}
		//start
				gest_start_x = gest_pt_x[0];
				gest_start_y = gest_pt_y[0];
				//end
				gest_end_x = gest_pt_x[gest_pt_cnt-1];
				gest_end_y = gest_pt_y[gest_pt_cnt-1];
				//most_left
				gest_most_left_x = gest_pt_x[max_left_index];
				gest_most_left_y = gest_pt_y[max_left_index];
				//most_right
				gest_most_right_x = gest_pt_x[max_right_index];
				gest_most_right_y = gest_pt_y[max_right_index];
				//top
				gest_most_top_x = gest_pt_x[max_high_index];
				gest_most_top_y = gest_pt_x[max_high_index];
				//bottom
				gest_most_bottom_x = gest_pt_x[max_low_index];
				gest_most_bottom_y = gest_pt_x[max_low_index];
			}
		}
		TS_LOG_INFO("%s: gest_len = %d\n", __func__, gest_len);

		if (reprot_gesture_point_num == 2) {
			TS_LOG_INFO("%s: Gesture Dobule Click\n", __func__);
			/*1.beginning 2.end */
			g_himax_zf_ts_data->tskit_himax_data->easy_wakeup_info.easywake_position[0] = gest_start_x << 16 | gest_start_y;
			g_himax_zf_ts_data->tskit_himax_data->easy_wakeup_info.easywake_position[1] = gest_end_x << 16 | gest_end_y;
			return retval;
		} else {
			/*1.begin */
			g_himax_zf_ts_data->tskit_himax_data->easy_wakeup_info.easywake_position[0] = gest_start_x << 16 | gest_start_y;
			TS_LOG_INFO("begin = 0x%08x,  begin_x= %d , begin_y= %d \n",
				g_himax_zf_ts_data->tskit_himax_data->easy_wakeup_info.easywake_position[0], gest_start_x, gest_start_y);
			/*2.end */
			g_himax_zf_ts_data->tskit_himax_data->easy_wakeup_info.easywake_position[1] = gest_end_x << 16 | gest_end_y;
			TS_LOG_INFO("top = 0x%08x,  end_x= %d , end_y= %d \n",
				g_himax_zf_ts_data->tskit_himax_data->easy_wakeup_info.easywake_position[1], gest_end_x, gest_end_y);
			/*3.top */
			g_himax_zf_ts_data->tskit_himax_data->easy_wakeup_info.easywake_position[2] = gest_most_top_x << 16 | gest_most_top_y;
			TS_LOG_INFO("top = 0x%08x,  top_x= %d , top_y= %d \n",
				g_himax_zf_ts_data->tskit_himax_data->easy_wakeup_info.easywake_position[2], gest_most_top_x, gest_most_top_y);
			/*4.leftmost */
			g_himax_zf_ts_data->tskit_himax_data->easy_wakeup_info.easywake_position[3] = gest_most_left_x << 16 | gest_most_left_y;
			TS_LOG_INFO("leftmost = 0x%08x,  left_x= %d , left_y= %d \n",
				g_himax_zf_ts_data->tskit_himax_data->easy_wakeup_info.easywake_position[3], gest_most_left_x, gest_most_left_y);
			/*5.bottom */
			g_himax_zf_ts_data->tskit_himax_data->easy_wakeup_info.easywake_position[4] = gest_most_bottom_x << 16 | gest_most_bottom_y;
			TS_LOG_INFO("bottom = 0x%08x,  bottom_x= %d , bottom_y= %d \n",
				g_himax_zf_ts_data->tskit_himax_data->easy_wakeup_info.easywake_position[4], gest_most_bottom_x, gest_most_bottom_x);
			/*6.rightmost */
			g_himax_zf_ts_data->tskit_himax_data->easy_wakeup_info.easywake_position[5] = gest_most_right_x << 16 | gest_most_right_y;
			TS_LOG_INFO("rightmost = 0x%08x,  right_x= %d , right_y= %d \n",
				g_himax_zf_ts_data->tskit_himax_data->easy_wakeup_info.easywake_position[5], gest_most_right_x, gest_most_right_y);
		}
	}
	return retval;
}

static int hx_check_key_gesture_report(struct ts_fingers *info, struct ts_easy_wakeup_info *gesture_report_info, unsigned char get_gesture_wakeup_data, uint8_t *buf)
{
	int retval = 0;
	unsigned int reprot_gesture_key_value = 0;
	unsigned int reprot_gesture_point_num = 0;

	TS_LOG_DEBUG("get_gesture_wakeup_data is %d\n",
		    get_gesture_wakeup_data);

	switch (get_gesture_wakeup_data) {
	case DOUBLE_CLICK_WAKEUP:
		if (IS_APP_ENABLE_GESTURE(GESTURE_DOUBLE_CLICK) &
			gesture_report_info->easy_wakeup_gesture) {
			TS_LOG_DEBUG("@@@DOUBLE_CLICK_WAKEUP detected!@@@\n");
			reprot_gesture_key_value = TS_DOUBLE_CLICK;
			reprot_gesture_point_num = LINEAR_LOCUS_NUM;
		}
	break;
	case SPECIFIC_LETTER_C:
		if (IS_APP_ENABLE_GESTURE(GESTURE_LETTER_C) &
			gesture_report_info->easy_wakeup_gesture) {
			TS_LOG_DEBUG
			    ("@@@SPECIFIC_LETTER_c detected!@@@\n");
			reprot_gesture_key_value = TS_LETTER_C;
			reprot_gesture_point_num = LETTER_LOCUS_NUM;
		}
	break;
	case SPECIFIC_LETTER_E:
		if (IS_APP_ENABLE_GESTURE(GESTURE_LETTER_E) &
			gesture_report_info->easy_wakeup_gesture) {
			TS_LOG_DEBUG
			    ("@@@SPECIFIC_LETTER_e detected!@@@\n");
			reprot_gesture_key_value = TS_LETTER_E;
			reprot_gesture_point_num = LETTER_LOCUS_NUM;
		}
	break;
	case SPECIFIC_LETTER_M:
		if (IS_APP_ENABLE_GESTURE(GESTURE_LETTER_M) &
			gesture_report_info->easy_wakeup_gesture) {
			TS_LOG_DEBUG
			    ("@@@SPECIFIC_LETTER_m detected!@@@\n");
			reprot_gesture_key_value = TS_LETTER_M;
			reprot_gesture_point_num = LETTER_LOCUS_NUM;
		}
	break;
	case SPECIFIC_LETTER_W:
		if (IS_APP_ENABLE_GESTURE(GESTURE_LETTER_W) &
			gesture_report_info->easy_wakeup_gesture) {
			TS_LOG_DEBUG
			    ("@@@SPECIFIC_LETTER_w detected!@@@\n");
			reprot_gesture_key_value = TS_LETTER_W;
			reprot_gesture_point_num = LETTER_LOCUS_NUM;
		}
	break;
	default:
		TS_LOG_INFO("@@@unknow gesture detected!\n");

	return 1;
	}

	if (0 != reprot_gesture_key_value) {
		mutex_lock(&wrong_touch_lock);
		if (true == gesture_report_info->off_motion_on) {
			retval = easy_wakeup_gesture_report_coordinate(reprot_gesture_point_num, info, buf);
			if (retval < 0) {
				mutex_unlock(&wrong_touch_lock);
				TS_LOG_ERR("%s: report line_coordinate error!retval = %d\n",
					__func__, retval);
				return retval;
			}
			info->gesture_wakeup_value = reprot_gesture_key_value;
			TS_LOG_DEBUG("%s: info->gesture_wakeup_value = %d\n",
				__func__, info->gesture_wakeup_value);
		}
		mutex_unlock(&wrong_touch_lock);
	}
	return NO_ERR;
}

static int hx_parse_wake_event(uint8_t *buf, struct ts_fingers *info)
{
	int i = 0;
	int retval = 0;
	int check_FC = 0;
	int gesture_flag = 0;
	unsigned char check_sum_cal = 0;

	struct ts_easy_wakeup_info *gesture_report_info = &g_himax_zf_ts_data->tskit_himax_data->easy_wakeup_info;

	TS_LOG_INFO("Hx gesture buf[0] = 0x%x buf[1] = 0x%x buf[2] = 0x%x buf[3] = 0x%x\n",
		buf[0], buf[1], buf[2], buf[3]);
	for (i = 0; i < 4; i++) {
		if (check_FC == 0) {
			if ((buf[0] != 0x00) && ((buf[0] <= 0x0F) || (buf[0] == 0x80))) {
				check_FC = 1;
				gesture_flag = buf[i];
			} else {
				check_FC = 0;
				TS_LOG_INFO("ID START at %x , value = %x skip the event\n", i, buf[i]);
				break;
			}
		} else {
			if (buf[i] != gesture_flag) {
				check_FC = 0;
				TS_LOG_INFO("ID NOT the same %x != %x So STOP parse event\n", buf[i], gesture_flag);
				break;
			}
		}

		TS_LOG_INFO("0x%2.2X ", buf[i]);
		if (i % 8 == 7)
			TS_LOG_INFO("\n");
	}
	TS_LOG_INFO("Himax gesture_flag= %x\n", gesture_flag);
	TS_LOG_INFO("Himax check_FC is %d\n", check_FC);

	if (check_FC == 0)
		return 1;
	if (buf[4] != 0xCC ||
			buf[4+1] != 0x44)
		return 1;
	for (i = 0; i < (4 + 4); i++)
		check_sum_cal += buf[i];

	if ((check_sum_cal != 0x00)) {
		TS_LOG_INFO("%s:check_sum_cal: 0x%02X\n", __func__, check_sum_cal);
		return 1;
	}
	retval = hx_check_key_gesture_report(info, gesture_report_info, gesture_flag, buf);

	return retval;
}

static int hx_read_event_stack(uint8_t *buf, uint8_t length, uint8_t buf_len)
{
	uint8_t cmd[4] = {0};

	if (length > buf_len) {
		TS_LOG_ERR("%s: invalid length, length = %d, buf = %d",
			__func__, length, buf_len);
		return SPI_FAIL;
	}

	cmd[0] = DATA_BURST_READ_OFF;
	if (hx_zf_bus_write(ADDR_BURST_READ, cmd, ONE_BYTE_CMD, sizeof(cmd), DEFAULT_RETRY_CNT) < 0) {
		TS_LOG_ERR("%s: spi access fail!\n", __func__);
		return SPI_FAIL;
	}

	himax_zf_bus_read(ADDR_READ_EVENT_STACK, buf, length, HX_RECEIVE_ROI_BUF_MAX_SIZE, DEFAULT_RETRY_CNT);

	cmd[0] = DATA_BURST_READ_ON;
	if (hx_zf_bus_write(ADDR_BURST_READ, cmd, ONE_BYTE_CMD, sizeof(cmd), DEFAULT_RETRY_CNT) < 0) {
		TS_LOG_ERR("%s: spi access fail!\n", __func__);
		return SPI_FAIL;
	}
	return NO_ERR;
}

static int himax_esd_palm_iron_report(unsigned int *key, uint16_t value)
{
	TS_LOG_DEBUG("esd_palm_iron_report: called value = %d\n", value);
	if (value) {
		*key = TS_KEY_IRON;
		return NO_ERR;
	}
	return HX_ERROR;
}

static int hx_irq_bottom_half(struct ts_cmd_node *in_cmd, struct ts_cmd_node *out_cmd)
{
	int m = 0;
	int retval = 0;
	int RawDataLen = 0;
	int raw_cnt_max = 0;
	int raw_cnt_rmd = 0;
	int hx_touch_info_size = 0;
	static int iCount;
	unsigned char check_sum_cal = 0;
	struct algo_param *algo_p = NULL;
	struct ts_fingers *info = NULL;
	struct himax_touching_data hx_touching;
	uint8_t tmp_addr[DATA_4] = {0};
	uint8_t tmp_data[DATA_4] = {0};
	uint16_t value;
	unsigned int *ts_palm_key = NULL;

#ifdef HX_TP_SYS_DIAG
	uint8_t diag_cmd = 0;
#endif
	uint8_t hx_state_info_pos = 0;
	uint8_t buf[HX_RECEIVE_ROI_BUF_MAX_SIZE] = {0};

	TS_LOG_DEBUG("%s:enter\n", __func__);

	if (in_cmd == NULL || out_cmd == NULL)
		return HX_ERROR;

	algo_p = &out_cmd->cmd_param.pub_params.algo_param;
	info = &algo_p->info;
	hx_touching.x = 0;
	hx_touching.y = 0;
	hx_touching.w = 0;
	hx_touching.finger_num = 0;
	hx_touching.old_finger = 0;
	hx_touching.loop_i = 0;

	raw_cnt_max = HX_ZF_MAX_PT / 4;
	raw_cnt_rmd = HX_ZF_MAX_PT % 4;

	if (g_himax_zf_ts_data->esd_palm_iron_support) {
		ts_palm_key = &out_cmd->cmd_param.pub_params.ts_key;
		TS_LOG_DEBUG("irq_bottom_half: esd_palm_iron_support=%d",
			g_himax_zf_ts_data->esd_palm_iron_support);
		hx_reg_assign(ADDR_ESD_STATUS_HX83102E, tmp_addr);
		hx_zf_register_read(tmp_addr, FOUR_BYTE_CMD, tmp_data);
		TS_LOG_DEBUG("irq_bottom_half: tmp_data[0]=%d, tmp_data[1]=%d\n",
			tmp_data[DATA_0], tmp_data[DATA_1]);
		value = ((tmp_data[DATA_1] << LEFT_MOV_8BIT) |
			tmp_data[DATA_0]);
		retval = himax_esd_palm_iron_report(ts_palm_key, value);
		if (!retval) {
			TS_LOG_INFO("HXTP esd trigger, report key value\n");
			out_cmd->command = TS_PALM_KEY;
			goto err_no_reset_out;
		}
	}

	if (raw_cnt_rmd != 0x00) {//more than 4 fingers
		RawDataLen = HX_RECEIVE_BUF_MAX_SIZE - ((HX_ZF_MAX_PT + raw_cnt_max + 3) * 4) - 1;
		hx_touch_info_size = (HX_ZF_MAX_PT + raw_cnt_max + 2) * 4;
	} else {//less than 4 fingers
		RawDataLen = HX_RECEIVE_BUF_MAX_SIZE - ((HX_ZF_MAX_PT + raw_cnt_max + 2) * 4) - 1;
		hx_touch_info_size = (HX_ZF_MAX_PT + raw_cnt_max + 1) * 4;
	}

	if (hx_touch_info_size > HX_RECEIVE_BUF_MAX_SIZE) {
		TS_LOG_ERR("%s:hx_touch_info_size larger than HX_RECEIVE_BUF_MAX_SIZE\n", __func__);
		goto err_no_reset_out;
	}

	hx_state_info_pos = hx_touch_info_size - 3;

	TS_LOG_DEBUG("%s: hx_touch_info_size = %d\n", __func__, hx_touch_info_size);
	if (atomic_read(&g_himax_zf_ts_data->suspend_mode) && g_himax_zf_ts_data->tskit_himax_data->ts_platform_data->chip_data->easy_wakeup_info.sleep_mode) {
		/*increase wake_lock time to avoid system suspend.*/
		__pm_wakeup_event(&g_himax_zf_ts_data->tskit_himax_data->ts_platform_data->ts_wake_lock, jiffies_to_msecs(TS_WAKE_LOCK_TIMEOUT));
		msleep(HX_SLEEP_200MS);
		retval = hx_read_event_stack(buf, HX_RECEIVE_BUF_MAX_SIZE, sizeof(buf));
		if (retval < 0)
			TS_LOG_ERR("%s: can't read data from chip!\n", __func__);
		retval = hx_parse_wake_event(buf, info);
		if (!retval)
			out_cmd->command = TS_INPUT_ALGO;

		return  retval;
	}
	TS_LOG_DEBUG("%s: hx_parse_wake_event end\n", __func__);

#ifdef HX_TP_SYS_DIAG
	diag_cmd = hx_zf_getDiagCommand();
#ifdef HX_ESD_WORKAROUND
	if ((diag_cmd) || (ESD_RESET_ACTIVATE) || (hw_reset_activate)) {
#else
	if ((diag_cmd) || (hw_reset_activate)) {
#endif
		retval = hx_read_event_stack(buf, HX_RECEIVE_ROI_BUF_MAX_SIZE, sizeof(buf)); // diag cmd not 0, need to read 164.
		if (header_format_check(buf, sizeof(buf)))
			memcpy(g_himax_zf_ts_data->hx_rawdata_buf_roi, &buf[HX_TOUCH_SIZE], HX_RECEIVE_ROI_BUF_MAX_SIZE - HX_TOUCH_SIZE);
	} else {
		retval = hx_read_event_stack(buf, hx_touch_info_size, sizeof(buf));
	}

	if (buf[hx_state_info_pos] != 0xFF && buf[hx_state_info_pos + 1] != 0xFF)
		memcpy(g_himax_zf_ts_data->hx_state_info, &buf[hx_state_info_pos], 2);
	else
		memset(g_himax_zf_ts_data->hx_state_info, 0x00, sizeof(g_himax_zf_ts_data->hx_state_info));

	TS_LOG_DEBUG("%s: hx_read_event_stack: retval = %d\n", __func__, retval);//use for debug
	if (retval < 0) {
#else
	if (hx_read_event_stack(buf, hx_touch_info_size, sizeof(buf)) < 0) {
#endif
		TS_LOG_ERR("%s: can't read data from chip!\n", __func__);
		iCount++;
		TS_LOG_ERR("%s: error count is %d !\n", __func__, iCount);
		if (iCount >= RETRY_TIMES) {
			iCount = 0;
			goto err_workqueue_out;
		}
		goto err_no_reset_out;
	} else {
		out_cmd->command = TS_INPUT_ALGO;

		if (g_himax_zf_ts_data->debug_log_level & BIT(0)) {
			TS_LOG_INFO("%s: raw data:\n", __func__);
			himax_debug_level_print(0, 0, hx_touch_info_size, hx_touching, buf); // status uselss
		}
#ifdef HX_ESD_WORKAROUND
		check_sum_cal = hx_check_report_data_for_esd(hx_touch_info_size, buf);
#ifdef HX_TP_SYS_DIAG
		diag_cmd = hx_zf_getDiagCommand();
		if (check_sum_cal != 0 && ESD_RESET_ACTIVATE == 0 && hw_reset_activate == 0
			&& (diag_cmd == 0 || diag_cmd == 5)) { // ESD Check
#else
		if (check_sum_cal != 0 && ESD_RESET_ACTIVATE == 0 && hw_reset_activate == 0) { // ESD Check
#endif
			if (check_sum_cal == ESD_ALL_ZERO_BAK_VALUE) {
				g_zero_event_count++;
				TS_LOG_INFO("ALL Zero event is %d times.\n", g_zero_event_count);

				if (g_zero_event_count > 2) {
					g_zero_event_count = 0;
					TS_LOG_INFO("EXCEPTION event checked - ALL Zero.\n");
					himax_zf_read_fw_status();
					ESD_HW_REST();
					goto err_no_reset_out;
				}

			} else if (check_sum_cal == ESD_ALL_ED_BAK_VALUE) {
				TS_LOG_INFO("[HX TP MSG]: ESD event checked - ALL 0xED.\n");
			} else if (check_sum_cal == ESD_ALL_EB_BAK_VALUE) {
				TS_LOG_INFO("[HX TP MSG]: ESD event checked - ALL 0xEB.\n");
			} else if (check_sum_cal == ESD_ALL_EC_BAK_VALUE) {
				TS_LOG_INFO("[HX TP MSG]: ESD event checked - ALL 0xEC.\n");
			}
			if ((check_sum_cal == ESD_ALL_ED_BAK_VALUE) ||
				(check_sum_cal == ESD_ALL_EB_BAK_VALUE) ||
				(check_sum_cal == ESD_ALL_EC_BAK_VALUE)) {
				himax_zf_read_fw_status();
				ESD_HW_REST();
				goto err_no_reset_out;
			}
		} else if (ESD_RESET_ACTIVATE) {
			ESD_RESET_ACTIVATE = 0; /* drop 1st interrupts after chip reset */
			TS_LOG_INFO("[HX TP MSG]:%s: Back from reset, ready to serve.\n", __func__);
			return retval;
		} else if (hw_reset_activate) {
#else
		if (hw_reset_activate) {
#endif
			hw_reset_activate = 0; /* drop 1st interrupts after chip reset */
			TS_LOG_INFO("[HX TP MSG]:%s: hw_reset_activate = 1 Skip an interrupt.\n", __func__);
			return retval;
		}

		check_sum_cal = himax_checksum_cal(hx_touch_info_size, hx_touching, buf);

		if (check_sum_cal != 0x00) {
			TS_LOG_INFO("[HX TP MSG] checksum fail : check_sum_cal: 0x%02X\n", check_sum_cal);
			iCount++;
			TS_LOG_ERR("%s: error count is %d !\n", __func__, iCount);
			if (iCount >= RETRY_TIMES) {
				iCount = 0;
				goto err_workqueue_out;
			}
			goto err_no_reset_out;
		}
	}

	/*touch monitor raw data fetch*/
#ifdef HX_TP_SYS_DIAG
	diag_cmd = hx_zf_getDiagCommand();
	if (diag_cmd >= 1 && diag_cmd <= 6)	{
		if (!g_himax_zf_ts_data->roiflag) {
			if (hx_start_get_rawdata_from_event(hx_touch_info_size, RawDataLen, buf) == HX_ERROR)
				goto bypass_checksum_failed_packet;
		} else {
			if (hx_start_get_roi_data_from_event(hx_touch_info_size, buf, sizeof(buf)) == HX_ERROR)
				goto bypass_checksum_failed_packet;
		}
	} else if (diag_cmd == 7) {
		memcpy(&(hx_zf_diag_coor[0]), &buf[0], HX_RECEIVE_BUF_MAX_SIZE);
	}
#endif
bypass_checksum_failed_packet:
	m = HX_TOUCH_INFO_POINT_CNT + 2;
	EN_NoiseFilter = ((buf[m]) >> 3);//HX_TOUCH_INFO_POINT_CNT: 52 ;
	EN_NoiseFilter = EN_NoiseFilter & 0x01;

	if (buf[HX_TOUCH_INFO_POINT_CNT] == 0xff)
		hx_real_point_num = 0;
	else
		hx_real_point_num = buf[HX_TOUCH_INFO_POINT_CNT] & 0x0f;//only use low 4 bits.

	/*Touch Point information*/
	hx_parse_coords(hx_touch_info_size, hx_real_point_num, info, hx_touching, buf);
	TS_LOG_DEBUG("%s: hx_parse_coords end\n", __func__);
	Last_EN_NoiseFilter = EN_NoiseFilter;

	iCount = 0;
	return retval;

err_workqueue_out:
	TS_LOG_ERR("%s: Now reset the Touch chip.\n", __func__);

	hx_zf_hw_reset(HX_LOADCONFIG_EN, HX_INT_DISABLE);

err_no_reset_out:
	return NO_RESET_OUT;

}
static int hx_parse_specific_dts(struct himax_ts_data *ts, struct himax_platform_data *pdata)
{
	int retval = 0;
	int read_val = 0;
	int length = 0;
	int i = 0;
	struct property *prop = NULL;
	struct device_node *dt = NULL;
	struct ts_kit_device_data *chip_data = NULL;
	struct ts_kit_platform_data *ts_kit_pdata = NULL;

	if (ts == NULL || pdata == NULL)
		return HX_ERR;

	chip_data = g_himax_zf_ts_data->tskit_himax_data->ts_platform_data->chip_data;
	ts_kit_pdata = g_himax_zf_ts_data->tskit_himax_data->ts_platform_data;

	/*parse IC feature*/
	dt = of_find_compatible_node(NULL, NULL, chip_data->chip_name);

	if (ts->power_support) {
		retval = of_property_read_u32(dt,
			"himax,power_type_sel", &read_val);
		if (retval) {
			TS_LOG_INFO("Not define power_type_sel\n");
			read_val = 0; // default 0: no power type sel
		}
		TS_LOG_INFO("DT:power control type: %d\n", read_val);

		if (read_val != 0) {
			TS_LOG_INFO("DT:regulator ctl pow is not supported\n");
			ts->power_support = 0; // default 0:
		}
	}
	if (ts->rst_support) {
		pdata->reset_gpio = ts_kit_pdata->reset_gpio;
		TS_LOG_INFO("get HXTP reset_gpio = %X\n", pdata->reset_gpio);
	}


	retval = of_property_read_u32(dt, "himax,irq_config", &chip_data->irq_config);
	if (retval) {
		TS_LOG_ERR("Not define irq_config in Dts\n");
		return retval;
	}
	TS_LOG_INFO("get HXTP irq_config = %d\n", chip_data->irq_config);


	prop = of_find_property(dt, "himax,id-addr", NULL);
	if (prop) {
		length = prop->length / ((int) sizeof(uint32_t));
		TS_LOG_DEBUG("%s:id-addr length = %d", __func__, length);
	}

	if (of_property_read_u32_array(dt, "himax,id-addr", hx_id_addr, length) == NO_ERR) {
		TS_LOG_INFO("DT-%s:id-addr = %2X, %2X, %2X, %2X\n", __func__, hx_id_addr[0],
				hx_id_addr[1], hx_id_addr[2], hx_id_addr[3]);
	} else {
		TS_LOG_ERR("Not define HXTP,id-addr\n");
		return HX_ERR;
	}

	prop = of_find_property(dt, "himax,flash-addr", NULL);
	if (prop) {
		length = prop->length / ((int) sizeof(uint32_t));
		TS_LOG_DEBUG("%s:flash-addr length = %d", __func__, length);
	}

	if (of_property_read_u32_array(dt, "himax,flash-addr", hx_flash_addr, length) == NO_ERR) {
		for (i = 0; i < FLASH_ADDR_LEN; i += 7)
			TS_LOG_INFO("DT-%s:flash-addr = %d, %d, %d, %d, %d, %d,%d\n", __func__,
				hx_flash_addr[i], hx_flash_addr[i + 1], hx_flash_addr[i + 2], hx_flash_addr[i + 3],
				hx_flash_addr[i + 4], hx_flash_addr[i + 5], hx_flash_addr[i + 6]);
	} else {
		TS_LOG_ERR("Not define flash-addr\n");
		return HX_ERR;
	}

	retval = of_property_read_u32(dt, "himax,p2p-test-en", &read_val);
	if (retval) {
		ts->p2p_test_sel = 0;
		TS_LOG_INFO("get device p2p_test_sel not exit,use default value.\n");
	} else {
		ts->p2p_test_sel = read_val;
		TS_LOG_INFO("get device p2p_test_sel:%d\n", read_val);
	}

	return NO_ERR;
}

static int hx_parse_project_dts(struct himax_ts_data *ts, struct himax_platform_data *pdata)
{
	int retval = 0;
	int read_val = 0;
	int coords_size = 0;
	const char *tptesttype = NULL;
	uint32_t coords[HX_COORDS_MAX_SIZE] = {0};
	const char *modulename = NULL;
	struct property *prop = NULL;
	struct device_node *dt = NULL;
	struct ts_kit_device_data *chip_data = NULL;

	if (ts == NULL || pdata == NULL)
		return HX_ERR;

	chip_data = g_himax_zf_ts_data->tskit_himax_data->ts_platform_data->chip_data;

	/*parse module feature*/
	dt = of_find_compatible_node(NULL, NULL, himax_zf_project_id);

	retval = of_property_read_string(dt, "module", &modulename);

	if (retval) {
		TS_LOG_INFO("Not define module in Dts!\n");
	} else
		strncpy(chip_data->module_name, modulename, strlen(modulename)+1);

	TS_LOG_INFO("module_name: %s\n", chip_data->module_name);

	retval = of_property_read_string(dt, "tp_test_type", &tptesttype);
	if (retval) {
		TS_LOG_INFO("Not define device tp_test_type in Dts, use default\n");
		strncpy(chip_data->tp_test_type, "Normalize_type:judge_last_result", TS_CAP_TEST_TYPE_LEN);
	} else {
		snprintf(chip_data->tp_test_type, TS_CAP_TEST_TYPE_LEN, "%s", tptesttype);
	}
	TS_LOG_INFO("get tp test type = %s\n", chip_data->tp_test_type);

	retval = of_property_read_u32(dt, "himax,rawdata_timeout", &chip_data->rawdata_get_timeout);
	if (retval) {
		chip_data->rawdata_get_timeout = RAWDATA_GET_TIME_DEFAULT;
		TS_LOG_INFO("Not define chip rawdata limit time in Dts, use default\n");
	}
	TS_LOG_INFO("get chip rawdata limit time = %d\n", chip_data->rawdata_get_timeout);

	retval = of_property_read_u32(dt, "himax,tx_num", &chip_data->tx_num);
	if (retval) {
		chip_data->tx_num = HX83108A_TX_NUM;
		TS_LOG_INFO("Not define chip tx amount in Dts, use %d\n",
			chip_data->tx_num);
	}
	TS_LOG_INFO("get chip tx_num = %d\n", chip_data->tx_num);

	retval = of_property_read_u32(dt, "himax,rx_num", &chip_data->rx_num);
	if (retval) {
		chip_data->rx_num = HX83108A_RX_NUM;
		TS_LOG_INFO("Not define rx_num in Dts, use %d\n",
			chip_data->rx_num);
	}
	TS_LOG_INFO("get chip rx_num = %d\n", chip_data->rx_num);

	retval = of_property_read_u32(dt, "himax,max_pt", &pdata->max_pt);
	if (retval) {
		pdata->max_pt = HX83108A_MAX_PT;
		TS_LOG_INFO("Not define chip max_pt amount in Dts, use %d\n",
			pdata->max_pt);
	}
	TS_LOG_INFO("get chip max_pt = %d\n", pdata->max_pt);

	retval = of_property_read_u32(dt, "himax,trx_delta_test_support", &chip_data->trx_delta_test_support);
	if (retval) {
		TS_LOG_INFO("get device trx_delta_test_support null, use default\n");
		chip_data->trx_delta_test_support = 0;
		retval = 0;
	} else {
		TS_LOG_INFO("get device trx_delta_test_support = %d\n", chip_data->trx_delta_test_support);
	}

	retval = of_property_read_u32(dt, TEST_CAPACITANCE_VIA_CSVFILE, &read_val);
	if (retval) {
	ts->test_capacitance_via_csvfile = false;
	TS_LOG_INFO("get device test_capacitance_via_csvfile not exit,use default value.\n");
	} else {
	ts->test_capacitance_via_csvfile = read_val;
	TS_LOG_INFO("get device test_capacitance_via_csvfile:%d\n", read_val);
	}

	retval = of_property_read_u32(dt, CSVFILE_USE_SYSTEM_TYPE, &read_val);
	if (retval) {
		ts->csvfile_use_system = false;
		TS_LOG_INFO("get device csvfile_use_system not exit,use default value.\n");
	} else {
		ts->csvfile_use_system = read_val;
		TS_LOG_INFO("get device csvfile_use_system:%d\n", read_val);
	}
	return NO_ERR;
}

static int hx_parse_dts(struct device_node *device, struct ts_kit_device_data *chip_data)
{
	u32 read_val = 0;
	int retval = NO_ERR;
	int coords_size = 0;
	uint32_t coords[HX_COORDS_MAX_SIZE] = {0};
	const char *chipname = NULL;
	struct property *prop = NULL;
	struct himax_ts_data *ts;
#if defined(HUAWEI_CHARGER_FB)
	struct ts_charger_info *charger_info;
#endif

	ts = g_himax_zf_ts_data;

	TS_LOG_INFO("%s: parameter init begin\n", __func__);
	if ((device == NULL) || (chip_data == NULL))
		return HX_ERR;

	retval = of_property_read_u32(device, "ic_type", &chip_data->ic_type);
	if (retval) {
		chip_data->ic_type = ONCELL;
		TS_LOG_ERR("Not define device ic_type in Dts\n");
	}
	TS_LOG_INFO("get chip_data->ic_type = %d.\n", chip_data->ic_type);

	retval = of_property_read_string(device, "chip_name", &chipname);
	if (retval) {
		TS_LOG_INFO("Not define module in Dts!\n");
	} else {
		strncpy(chip_data->chip_name, chipname, CHIP_NAME_LEN);
		TS_LOG_INFO("get hx_chipname successfully\n");
	}

	retval = of_property_read_u32(device, "hx_ic_rawdata_proc_printf", &read_val);
	if (retval) {
		read_val = false;
		TS_LOG_INFO("No chip is_ic_rawdata_proc_printf, use def\n");
	}
	chip_data->is_ic_rawdata_proc_printf = !!read_val;
	TS_LOG_INFO("get chip is_ic_rawdata_proc_printf = %d\n",
		chip_data->is_ic_rawdata_proc_printf);

	retval = of_property_read_u32(device, "himax,power_support", &read_val);
	if (retval) {
		read_val = NOT_SUPPORT;
		TS_LOG_INFO("NOT support to parse the power dts!\n");
	} else
	ts->power_support = (u8)read_val;
	TS_LOG_INFO("HXTP,power_support = %d\n", ts->power_support);

	retval = of_property_read_u32(device, "himax,rst_support", &read_val);
	if (retval) {
		read_val = NOT_SUPPORT;
		TS_LOG_INFO("NOT support to parse the rst dts!\n");
	}
	ts->rst_support = (u8)read_val;
	TS_LOG_INFO("HXTP,rst_support = %d\n", ts->rst_support);

	prop = of_find_property(device, "himax,panel-coords", NULL);
	if (prop) {
		coords_size = prop->length / ((int) sizeof(uint32_t));
		if (coords_size != HX_COORDS_MAX_SIZE)
			TS_LOG_DEBUG("%s:Invalid panel coords size %d", __func__, coords_size);
	}

	if (of_property_read_u32_array(device, "himax,panel-coords", coords, coords_size) == NO_ERR) {
		ts->pdata->abs_x_min = coords[0], ts->pdata->abs_x_max = coords[1];
		ts->pdata->abs_y_min = coords[2], ts->pdata->abs_y_max = coords[3];
		TS_LOG_INFO("DT-%s:panel-coords = %d, %d, %d, %d\n", __func__, ts->pdata->abs_x_min,
				ts->pdata->abs_x_max, ts->pdata->abs_y_min, ts->pdata->abs_y_max);
	} else {
		ts->pdata->abs_x_max = ABS_X_MAX_DEFAULT;
		ts->pdata->abs_y_max = ABS_Y_MAX_DEFAULT;
	}

	prop = of_find_property(device, "himax,display-coords", NULL);
	if (prop) {
		coords_size = prop->length / ((int) sizeof(uint32_t));
		if (coords_size != HX_COORDS_MAX_SIZE)
			TS_LOG_DEBUG("%s:Invalid display coords size %d", __func__, coords_size);
	}

	retval = of_property_read_u32_array(device, "himax,display-coords", coords, coords_size);
	if (retval) {
		TS_LOG_DEBUG("%s:Fail to read display-coords %d\n", __func__, retval);
		return retval;
	}
	ts->pdata->screenWidth  = coords[1];
	ts->pdata->screenHeight = coords[3];

	TS_LOG_INFO("DT-%s:display-coords = (%d, %d)", __func__, ts->pdata->screenWidth,
		ts->pdata->screenHeight);

	retval = of_property_read_u32(device, "himax,max_pt", &ts->pdata->max_pt);
	if (retval) {
		ts->pdata->max_pt = HX83108A_MAX_PT;
		TS_LOG_INFO("Not define chip max_pt amount in Dts, use %d\n",
			ts->pdata->max_pt);
	}
	TS_LOG_INFO("get chip max_pt = %d\n", ts->pdata->max_pt);

	retval = of_property_read_u32(device, "himax,glove_supported", &read_val);
	if (retval) {
		read_val = NOT_SUPPORT;
		TS_LOG_INFO("NOT support to parse the glove dts!\n");
	}
	chip_data->ts_platform_data->feature_info.glove_info.glove_supported = (u8)read_val;
	TS_LOG_INFO("HXTP,glove_support = %d\n",
		chip_data->ts_platform_data->feature_info.glove_info.glove_supported);

	retval = of_property_read_u32(device, "himax,roi_supported", &read_val);
	if (retval) {
		read_val = NOT_SUPPORT;
		TS_LOG_INFO("NOT support to parse the roi dts!\n");
	}
	chip_data->ts_platform_data->feature_info.roi_info.roi_supported = (u8)read_val;
	TS_LOG_INFO("HXTP,roi_supported = %d\n",
		chip_data->ts_platform_data->feature_info.roi_info.roi_supported);

	retval = of_property_read_u32(device, "horizon_change_supported", &read_val);
	if (retval) {
		read_val = NOT_SUPPORT;
		TS_LOG_INFO("NOT support to parse the roi dts!\n");
	}
	chip_data->ts_platform_data->feature_info.horizon_info.horizon_supported = (u8)read_val;
	TS_LOG_INFO("HXTP,horizon_supported = %d\n",
		chip_data->ts_platform_data->feature_info.horizon_info.horizon_supported);

	retval = of_property_read_u32(device, "himax,gesture_supported", &read_val);
	if (retval) {
		read_val = NOT_SUPPORT;
		TS_LOG_INFO("NOT support to parse the gesture dts!\n");
	}
	chip_data->ts_platform_data->feature_info.wakeup_gesture_enable_info.switch_value = (u8)read_val;
	TS_LOG_INFO("HXTP,gesture_supported = %d\n",
		chip_data->ts_platform_data->feature_info.wakeup_gesture_enable_info.switch_value);
#if defined(HUAWEI_CHARGER_FB)
	charger_info =
		&(chip_data->ts_platform_data->feature_info.charger_info);
	retval = of_property_read_u32(device, "charger_supported", &read_val);
	if (retval) {
		TS_LOG_ERR("%s: Not define charger_support in Dts\n", __func__);
		read_val = 0;
	}
	charger_info->charger_supported = (uint8_t)read_val;
	TS_LOG_INFO("%s: HXTP charger_support = %d\n",
		__func__, charger_info->charger_supported);
#endif
	retval = of_property_read_u32(device, "esd_palm_iron_support",
		&g_himax_zf_ts_data->esd_palm_iron_support);
	if (retval) {
		g_himax_zf_ts_data->esd_palm_iron_support = 0;
		TS_LOG_INFO("parse_dts: get esd_palm_iron_support failed\n");
	}
	/* get tp color flag */
	retval = of_property_read_u32(device,
		"support_get_tp_color", &read_val);
	if (retval) {
		TS_LOG_INFO("%s, get support_get_tp_color failed\n",
			__func__);
		read_val = 0; // default 0: no need know tp color
	}
	ts->support_get_tp_color = (uint8_t)read_val;
	TS_LOG_INFO("%s, support_get_tp_color = %d\n",
		__func__, ts->support_get_tp_color);
	/* get project id flag */
	retval = of_property_read_u32(device,
		"support_read_projectid", &read_val);
	if (retval) {
		TS_LOG_INFO("%s, get support_read_projectid failed,\n ",
			__func__);
		read_val = 0; // default 0: no need know tp color
	}
	ts->support_read_projectid = (uint8_t)read_val;
	TS_LOG_INFO("%s, support_read_projectid = %d\n",
		__func__, ts->support_read_projectid);

	/* get hx_opt_hw_crc */
	retval = of_property_read_u32(device, "support_hx_opt_hw_crc", &read_val);
	if (retval) {
		TS_LOG_INFO("%s, get support_hx_opt_hw_crc failed,\n ",
			__func__);
		read_val = 0; // default 0: no support opt_hw_crc
	}
	ts->support_hx_opt_hw_crc = (uint8_t)read_val;
	TS_LOG_INFO("%s, support_hx_opt_hw_crc = %d\n",
		__func__, ts->support_hx_opt_hw_crc);

	return NO_ERR;
}

struct zf_opt_crc g_zf_opt_crc;
static int hx_chip_detect(struct ts_kit_platform_data *platform_data)
{
	int err = NO_ERR;
	uint8_t tmp_spi_mode = SPI_MODE_0;
	struct himax_ts_data *ts = NULL;
	struct himax_platform_data *pdata = NULL;
	struct device_node *device = NULL;

	TS_LOG_INFO("%s:called\n", __func__);

	if (!platform_data) {
		TS_LOG_ERR("ts_kit_platform_data *platform_data is NULL\n");
		err = -EINVAL;
		goto out;
	}

	g_himax_zf_ts_data->ts_dev = platform_data->ts_dev;
	g_himax_zf_ts_data->ts_dev->dev.of_node = g_himax_zf_ts_data->tskit_himax_data->cnode;
	g_himax_zf_ts_data->tskit_himax_data->ts_platform_data = platform_data;
	g_himax_zf_ts_data->tskit_himax_data->is_in_cell = false;
	g_himax_zf_ts_data->tskit_himax_data->is_i2c_one_byte = 0;
	g_himax_zf_ts_data->tskit_himax_data->is_new_oem_structure = 0;
	g_himax_zf_ts_data->dev = &platform_data->ts_dev->dev;
	g_himax_zf_ts_data->spi = platform_data->spi;
	g_himax_zf_ts_data->firmware_updating = false;

	ts = g_himax_zf_ts_data;
	device = ts->ts_dev->dev.of_node;

	pdata = kzalloc(sizeof(*pdata), GFP_KERNEL);
	if (pdata == NULL) {
		err = -ENOMEM;
		goto err_alloc_platform_data_fail;
	}
	ts->pdata = pdata;
	err = hx_parse_dts(device, platform_data->chip_data);
	if (err)
		TS_LOG_ERR("hx_parse_dts err:%d\n", err);

	if (ts->ts_dev->dev.of_node) { /*DeviceTree Init Platform_data*/
		err = hx_parse_specific_dts(ts, pdata);
		if (err < 0) {
			TS_LOG_INFO(" %s hx_parse_specific_dts failed\n", __func__);
			goto err_parse_pdata_failed;
		}
	}
	pdata->gpio_reset = ts->tskit_himax_data->ts_platform_data->reset_gpio;
	TS_LOG_INFO("pdata->gpio_reset:%d\n", pdata->gpio_reset);
	pdata->gpio_irq = ts->tskit_himax_data->ts_platform_data->irq_gpio;
	TS_LOG_INFO("pdata->gpio_irq:%d\n", pdata->gpio_irq);
	ts->rst_gpio = pdata->gpio_reset;
	//config rst gpio in kernel and keep high
	err = gpio_direction_output(ts->rst_gpio, 1);
	if (err) {
		TS_LOG_ERR("%s:gpio direction output to 1 fail, err=%d\n",
			__func__, err);
		return err;
	}
	err = hx_reset_device();
	if (err < 0) {
		TS_LOG_ERR("hx_reset_device error\n");
		goto  err_ic_package_failed;
	}
	/* backup and change to SPI mode 0 for novatek SIF */
	tmp_spi_mode = g_himax_zf_ts_data->spi->mode;
	g_himax_zf_ts_data->spi->mode = SPI_MODE_3;
	err = spi_setup(g_himax_zf_ts_data->spi);
	if (err)
		TS_LOG_ERR("%s:setup spi fail\n", __func__);

	if (hx_ic_package_check() == IC_PACK_CHECK_FAIL) {
		TS_LOG_ERR("Hx chip does NOT EXIST");
		err = -ENOMEM;
		goto err_ic_package_failed;
	}

	if ((ts->support_hx_opt_hw_crc) && (IC_ZF_TYPE == HX_83108A_SERIES_PWON))
		g_zf_opt_crc.fw_addr = CRC_FW_ADDR;

	err = hostprocessing_get_project_id(himax_zf_project_id, HX_PROJECT_ID_LEN);
	if (err) {
		TS_LOG_ERR("%s, fail get project_id\n", __func__);
		goto err_parse_project_dts_failed;
	}
	memcpy(ts->project_id, himax_zf_project_id, HX_PROJECT_ID_LEN);
	TS_LOG_INFO("%s, projectid:%s\n", __func__, ts->project_id);

	/* 1: enable download firmware in recovery mode */
	platform_data->chip_data->download_fw_inrecovery = 1;

	if (ts->ts_dev->dev.of_node) { /* DeviceTree Init Platform_data */
		err = hx_parse_project_dts(ts, pdata);
		if (err < 0) {
			TS_LOG_INFO(" %s hx_parse_project_dts failed\n", __func__);
			goto err_parse_project_dts_failed;
		}
	}
	return NO_ERR;

err_parse_project_dts_failed:

err_ic_package_failed:
	g_himax_zf_ts_data->spi->mode = tmp_spi_mode;
	if (spi_setup(g_himax_zf_ts_data->spi))
		TS_LOG_ERR("%s:setup spi setup fail\n", __func__);

err_parse_pdata_failed:
	kfree(pdata);
	pdata = NULL;
err_alloc_platform_data_fail:
out:
	TS_LOG_ERR("detect HXTP error\n");
	return err;
}

static int hx_init_chip(void)
{
	int err = NO_ERR;
	struct himax_ts_data *ts = NULL;

	TS_LOG_INFO("%s:called\n", __func__);

	ts = g_himax_zf_ts_data;

	hx_zf_get_information();

	calculate_point_number();

	mutex_init(&wrong_touch_lock);

#ifdef HX_TP_SYS_DIAG
	hx_zf_setXChannel(HX_ZF_RX_NUM); // X channel
	hx_zf_setYChannel(HX_ZF_TX_NUM); // Y channel
	hx_zf_setMutualBuffer();
	if (hx_zf_getMutualBuffer() == NULL) {
		TS_LOG_ERR("%s: mutual buffer allocate fail failed\n", __func__);
		goto err_getMutualBuffer_failed;
	}
	hx_zf_setMutualNewBuffer();
	if (hx_zf_getMutualNewBuffer() == NULL) {
		TS_LOG_ERR("%s: New mutual buffer allocate fail failed\n", __func__);
		goto err_getMutualNewBuffer_failed;
	}
	hx_zf_setMutualOldBuffer();
	if (hx_zf_getMutualOldBuffer() == NULL) {
		TS_LOG_ERR("%s: Old mutual buffer allocate fail failed\n", __func__);
		goto err_getMutualOldBuffer_failed;
	}
	hx_zf_setSelfBuffer();
	if (hx_zf_getSelfBuffer() == NULL) {
		TS_LOG_ERR("%s: Old mutual buffer allocate fail failed\n", __func__);
		goto err_geSelf_failed;
	}
#endif

	ts->x_channel = HX_ZF_RX_NUM;
	ts->y_channel = HX_ZF_TX_NUM;
	ts->nFinger_support = HX_ZF_MAX_PT;
	/* calculate the data size */
	calcDataSize(ts->nFinger_support);
	TS_LOG_INFO("%s: calcDataSize complete\n", __func__);
	ts->pdata->abs_pressure_min = 0;
	ts->pdata->abs_pressure_max = 200;
	ts->pdata->abs_width_min	= 0;
	ts->pdata->abs_width_max	= 200;
	ts->suspended	= false;

	atomic_set(&ts->suspend_mode, 0);

#ifdef HX_ESD_WORKAROUND
	ESD_RESET_ACTIVATE = 0;
#endif
	hw_reset_activate = 0;

#if defined(CONFIG_TOUCHSCREEN_HIMAX_ZF_DEBUG)
		hx_zf_touch_sysfs_init();
#endif
	TS_LOG_INFO("%s:sucess\n", __func__);
	return NO_ERR;

#ifdef HX_TP_SYS_DIAG
	hx_zf_freeSelfBuffer();
err_geSelf_failed:
	hx_zf_freeMutualOldBuffer();
err_getMutualOldBuffer_failed:
	hx_zf_freeMutualNewBuffer();
err_getMutualNewBuffer_failed:
	hx_zf_freeMutualBuffer();
err_getMutualBuffer_failed:
#endif
	return err;
}

static int hx_mcu_ulpm_in(void)
{
	uint8_t tmp_data[4];
	int rtimes = 0;
	int ret = 0;

	TS_LOG_INFO("%s:entering\n", __func__);

	/* 34 -> 11 */
	do {
		if (rtimes > 10) {
			TS_LOG_INFO("%s:1/7 retry over 10 times!\n", __func__);
			return false;
		}

		tmp_data[0] = fw_data_ulpm_11;
		if (hx_zf_bus_write(fw_addr_ulpm_34, tmp_data,
			ONE_BYTE_CMD, sizeof(tmp_data), DEFAULT_RETRY_CNT) < 0) {
			TS_LOG_ERR("%s: bus access fail!\n", __func__);
			return false;
		}

		if (himax_zf_bus_read(fw_addr_ulpm_34, tmp_data,
			ONE_BYTE_CMD, sizeof(tmp_data), DEFAULT_RETRY_CNT) < 0) {
			TS_LOG_ERR("%s: bus access fail!\n", __func__);
			return;
		}

		TS_LOG_INFO("%s:retry times %d,addr=0x34,correct 0x11=current 0x%2.2X\n",
			__func__, rtimes, tmp_data[0]);
		rtimes++;
	} while (tmp_data[0] != fw_data_ulpm_11);

	rtimes = 0;
	/* 34 -> 11 */
	do {
		if (rtimes > 10) {
			TS_LOG_INFO("%s:1/7 retry over 10 times!\n", __func__);
			return false;
		}

		tmp_data[0] = fw_data_ulpm_11;
		if (hx_zf_bus_write(fw_addr_ulpm_34, tmp_data,
			ONE_BYTE_CMD, sizeof(tmp_data), DEFAULT_RETRY_CNT) < 0) {
			TS_LOG_ERR("%s: bus access fail!\n", __func__);
			return false;
		}

		if (himax_zf_bus_read(fw_addr_ulpm_34, tmp_data,
			ONE_BYTE_CMD, sizeof(tmp_data), DEFAULT_RETRY_CNT) < 0) {
			TS_LOG_ERR("%s: bus access fail!\n", __func__);
			return;
		}

		TS_LOG_INFO("%s:retry times %d,addr=0x34,correct 0x11=current 0x%2.2X\n",
				__func__, rtimes, tmp_data[0]);
		rtimes++;
	} while (tmp_data[0] != fw_data_ulpm_11);

	/* 33 -> 33 */
	rtimes = 0;
	do {
		if (rtimes > 10) {
			TS_LOG_INFO("%s:3/7 retry over 10 times!\n", __func__);
			return false;
		}

		tmp_data[0] = fw_data_ulpm_33;
		if (hx_zf_bus_write(fw_addr_ulpm_33, tmp_data,
			ONE_BYTE_CMD, sizeof(tmp_data), DEFAULT_RETRY_CNT) < 0) {
			TS_LOG_ERR("%s: bus access fail!\n", __func__);
			return false;
		}

		if (himax_zf_bus_read(fw_addr_ulpm_33, tmp_data,
			ONE_BYTE_CMD, sizeof(tmp_data), DEFAULT_RETRY_CNT) < 0) {
			TS_LOG_ERR("%s: bus access fail!\n", __func__);
			return;
		}

		TS_LOG_INFO("%s:retry times %d,addr=0x33,correct 0x33=current 0x%2.2X\n",
			__func__, rtimes, tmp_data[0]);
		rtimes++;
	} while (tmp_data[0] != fw_data_ulpm_33);

	/* 34 -> 22 */
	rtimes = 0;
	do {
		if (rtimes > 10) {
			TS_LOG_INFO("%s:4/7 retry over 10 times!\n", __func__);
			return false;
		}
		tmp_data[0] = fw_data_ulpm_22;
		if (hx_zf_bus_write(fw_addr_ulpm_34, tmp_data,
			ONE_BYTE_CMD, sizeof(tmp_data), DEFAULT_RETRY_CNT) < 0) {
			TS_LOG_ERR("%s: bus access fail!\n", __func__);
			return false;
		}

		if (himax_zf_bus_read(fw_addr_ulpm_34, tmp_data,
			ONE_BYTE_CMD, sizeof(tmp_data), DEFAULT_RETRY_CNT) < 0) {
			TS_LOG_ERR("%s: bus access fail!\n", __func__);
			return;
		}

		TS_LOG_INFO("%s:retry times %d,addr=0x34,correct 0x22=current 0x%2.2X\n",
			__func__, rtimes, tmp_data[0]);
		rtimes++;
	} while (tmp_data[0] != fw_data_ulpm_22);

	/* 33 -> AA */
	rtimes = 0;
	do {
		if (rtimes > 10) {
			TS_LOG_INFO("%s:5/7 retry over 10 times!\n", __func__);
			return false;
		}
		tmp_data[0] = fw_data_ulpm_aa;
		if (hx_zf_bus_write(fw_addr_ulpm_33, tmp_data,
			ONE_BYTE_CMD, sizeof(tmp_data), DEFAULT_RETRY_CNT) < 0) {
			TS_LOG_ERR("%s: bus access fail!\n", __func__);
			return false;
		}

		if (himax_zf_bus_read(fw_addr_ulpm_33, tmp_data,
			ONE_BYTE_CMD, sizeof(tmp_data), DEFAULT_RETRY_CNT) < 0) {
			TS_LOG_ERR("%s: bus access fail!\n", __func__);
			return;
		}

		TS_LOG_INFO("%s:retry times %d,addr=0x33, correct 0xAA=current 0x%2.2X\n",
			__func__, rtimes, tmp_data[0]);
		rtimes++;
	} while (tmp_data[0] != fw_data_ulpm_aa);

	/* 33 -> 33 */
	rtimes = 0;
	do {
		if (rtimes > 10) {
			TS_LOG_INFO("%s:6/7 retry over 10 times!\n", __func__);
			return false;
		}

		tmp_data[0] = fw_data_ulpm_33;
		if (hx_zf_bus_write(fw_addr_ulpm_33, tmp_data,
			ONE_BYTE_CMD, sizeof(tmp_data), DEFAULT_RETRY_CNT) < 0) {
			TS_LOG_ERR("%s: bus access fail!\n", __func__);
			return false;
		}

		if (himax_zf_bus_read(fw_addr_ulpm_33, tmp_data,
			ONE_BYTE_CMD, sizeof(tmp_data), DEFAULT_RETRY_CNT) < 0) {
			TS_LOG_ERR("%s: bus access fail!\n", __func__);
			return;
		}

		TS_LOG_INFO("%s:retry times %d,addr=0x33,correct 0x33=current 0x%2.2X\n",
			__func__, rtimes, tmp_data[0]);
		rtimes++;
	} while (tmp_data[0] != fw_data_ulpm_33);

	/* 33 -> AA */
	rtimes = 0;
	do {
		if (rtimes > 10) {
			TS_LOG_INFO("%s:7/7 retry over 10 times!\n", __func__);
			return false;
		}

		tmp_data[0] = fw_data_ulpm_aa;
		if (hx_zf_bus_write(fw_addr_ulpm_33, tmp_data,
			ONE_BYTE_CMD, sizeof(tmp_data), DEFAULT_RETRY_CNT) < 0) {
			TS_LOG_ERR("%s: bus access fail!\n", __func__);
			return false;
		}

		if (himax_zf_bus_read(fw_addr_ulpm_33, tmp_data,
			ONE_BYTE_CMD, sizeof(tmp_data), DEFAULT_RETRY_CNT) < 0) {
			TS_LOG_ERR("%s: bus access fail!\n", __func__);
			return;
		}

		TS_LOG_INFO("%s:retry times %d,addr=0x33,correct 0xAA=current 0x%2.2X\n",
			__func__, rtimes, tmp_data[0]);
		rtimes++;
	} while (tmp_data[0] != fw_data_ulpm_aa);

	TS_LOG_INFO("%s:END\n", __func__);
	return true;
}

static int hx_mcu_black_gest_ctrl(bool enable)
{
	struct himax_ts_data *ts = NULL;
	ts = g_himax_zf_ts_data;

	TS_LOG_INFO("%s:enable=%d, ts->is_suspended=%d\n", __func__,
		enable, ts->suspended);

	if (ts->suspended) {
		if (enable) {
			TS_LOG_INFO("%s: Already suspended. Skipped.\n", __func__);
			hx_zf_hw_reset(HX_INT_DISABLE, HX_INT_DISABLE);
		} else {
			hx_mcu_ulpm_in();
		}
		return SUSPEND_IN;
	} else {
		hx_zf_sense_on(SENSE_ON_0);
	}
}

static int hx_enter_sleep_mode(void)
{
	TS_LOG_INFO("%s: enter\n", __func__);
	/*ULTRA LOW POWER*/
	hx_mcu_black_gest_ctrl(1);
	TS_LOG_INFO("%s: exit\n", __func__);
	return NO_ERR;
}

static int hx_exit_sleep_mode(void)
{
	TS_LOG_INFO("%s: enter\n", __func__);
	TS_LOG_INFO("%s: exit\n", __func__);
	return NO_ERR;
}

static int hx_core_suspend(void)
{
	int retval = 0;
	struct himax_ts_data *ts = NULL;

	TS_LOG_INFO("%s: Enter suspended\n", __func__);

	ts = g_himax_zf_ts_data;

	if (ts->firmware_updating) {
		TS_LOG_INFO("%s: tp fw is updating, return\n", __func__);
		return NO_ERR;
	}
	if (atomic_read(&hmx_zf_mmi_test_status)) {
		TS_LOG_INFO("%s: tp fw is hmx_zf_mmi_test_status, return\n", __func__);
		return NO_ERR;
	}
	if (ts->suspended) {
		TS_LOG_INFO("%s: Already suspended. Skipped.\n", __func__);
		return SUSPEND_IN;
	} else {
		ts->suspended = true;
		TS_LOG_INFO("%s: enter\n", __func__);
	}

	ts->first_pressed = 0;
	atomic_set(&ts->suspend_mode, 1);
	ts->pre_finger_mask = 0;
	if (true == ts->tskit_himax_data->ts_platform_data->feature_info.wakeup_gesture_enable_info.switch_value) {
		if (ts->tskit_himax_data->ts_platform_data->chip_data->easy_wakeup_info.sleep_mode) {

			mutex_lock(&wrong_touch_lock);
			g_himax_zf_ts_data->tskit_himax_data->easy_wakeup_info.off_motion_on = true;
			mutex_unlock(&wrong_touch_lock);
			TS_LOG_INFO("ENABLE gesture mode\n");
			return NO_ERR;
		} else {
			retval = hx_enter_sleep_mode();
			if (retval < 0) {
			   TS_LOG_ERR("[HXTP] %s: hx_enter_sleep_mode fail!\n", __func__);
			   return retval;
			}
		}
	}
	retval = gpio_direction_output(ts->rst_gpio, SET_OFF);
	if (retval) {
		TS_LOG_ERR("%s:set off rst_gpio fail, retval=%d\n",
			__func__, retval);
		return retval;
	}
	gpio_direction_output(ts->tskit_himax_data->ts_platform_data->cs_gpio, 0);
	TS_LOG_INFO("%s: exit\n", __func__);

	return NO_ERR;
}

static int hx_core_resume(void)
{
	struct himax_ts_data *ts;
	int retval = 0;

	TS_LOG_INFO("%s: enter\n", __func__);

	ts = g_himax_zf_ts_data;

	if (ts->firmware_updating) {
		TS_LOG_INFO("%s: tp fw is updating, return\n", __func__);
		return NO_ERR;
	}
	if (atomic_read(&hmx_zf_mmi_test_status)) {
		TS_LOG_INFO("%s: tp fw is hmx_zf_mmi_test_status, return\n", __func__);
		return NO_ERR;
	}

	if (ts->suspended) {
		TS_LOG_INFO("%s: will be resume\n", __func__);
	} else {
		TS_LOG_INFO("%s: Already resumed. Skipped.\n", __func__);
		return RESUME_IN;
	}

	if (ts->tskit_himax_data->ts_platform_data->chip_data->easy_wakeup_info.sleep_mode)	{
		mutex_lock(&wrong_touch_lock);
		g_himax_zf_ts_data->tskit_himax_data->easy_wakeup_info.off_motion_on = false;
		mutex_unlock(&wrong_touch_lock);
		msleep(HX_SLEEP_5MS);
	}

	retval = hx_exit_sleep_mode();
	if (retval < 0) {
		TS_LOG_ERR("[HXTP] %s: hx_exit_sleep_mode fail!\n", __func__);
		return retval;
	}
	retval = gpio_direction_output(ts->rst_gpio, SET_ON);
	if (retval) {
		TS_LOG_ERR("%s:set on rst_gpio fail, retval=%d\n",
			__func__, retval);
		return retval;
	}

	TS_LOG_INFO("%s: power on.\n", __func__);
	atomic_set(&ts->suspend_mode, 0);

	ts->suspended = false;

	TS_LOG_INFO("%s: exit\n", __func__);

	return NO_ERR;
}

static int hx_after_resume(void *feature_info)
{
	struct ts_feature_info *info = NULL;
	int retval;

	TS_LOG_INFO("%s: enter\n", __func__);

#ifdef HX_ESD_WORKAROUND
	g_zero_event_count = 0;
	hw_reset_activate = 0;
	ESD_RESET_ACTIVATE = 0;
#endif

	TS_LOG_INFO("%s: DRIVER_VER = %s\n", __func__, HIMAX_DRIVER_VER);

	if (!feature_info) {
		TS_LOG_ERR("%s: ts_feature_info is NULL\n", __func__);
		return -EINVAL;
	}
	info = (struct ts_feature_info *)feature_info;

	if (info->glove_info.glove_supported) {
		retval = hx_set_glove_switch(info->glove_info.glove_switch);
		if (retval < 0) {
			TS_LOG_ERR("Failed to set glove switch(%d), err: %d\n",
				   info->glove_info.glove_switch, retval);
			return retval;
		}
	}
	if (true == info->wakeup_gesture_enable_info.switch_value)
		hx_set_smwp_enable(SMWP_ON);
	else
		hx_set_smwp_enable(SMWP_OFF);

	hx_zf_fw_resume_update(himax_firmware_name);

#if defined(HUAWEI_CHARGER_FB)
	TS_LOG_INFO("%s: charger_supported = %d\n", __func__,
		info->charger_info.charger_supported);
	if (info->charger_info.charger_supported) {
		TS_LOG_INFO("%s: set charger switch:%d\n", __func__,
			info->charger_info.charger_switch);
		retval = hx_charger_switch(&(info->charger_info));
		if (retval < 0) {
			TS_LOG_ERR("%s:Failed set charger switch:%d, err:%d\n",
				__func__, info->charger_info.charger_switch, retval);
			return retval;
		}
	}
#endif
	TS_LOG_INFO("%s: horizon_supported = %d\n",
		__func__, info->horizon_info.horizon_supported);
	retval = hx_horizon_switch(&(info->horizon_info));
	if (retval < 0)
		TS_LOG_ERR("%s:Failed set horizon switch:%d, err:%d\n",
			__func__, info->horizon_info.horizon_switch, retval);
	TS_LOG_INFO("%s: exit\n", __func__);

	return NO_ERR;
}

static void hx_power_off_gpio_set(void)
{
	TS_LOG_INFO("%s:enter\n", __func__);

	if (g_himax_zf_ts_data->pdata->gpio_reset >= 0)
		gpio_free(g_himax_zf_ts_data->pdata->gpio_reset);

	if (g_himax_zf_ts_data->pdata->gpio_3v3_en >= 0)
		gpio_free(g_himax_zf_ts_data->pdata->gpio_3v3_en);

	if (g_himax_zf_ts_data->pdata->gpio_1v8_en >= 0)
		gpio_free(g_himax_zf_ts_data->pdata->gpio_1v8_en);

	if (gpio_is_valid(g_himax_zf_ts_data->pdata->gpio_irq))
		gpio_free(g_himax_zf_ts_data->pdata->gpio_irq);

	TS_LOG_INFO("%s:exit\n", __func__);
}

static int hx_power_off(void)
{
	int err = 0;

	TS_LOG_INFO("%s:enter\n", __func__);

	if (g_himax_zf_ts_data->pdata->gpio_3v3_en >= 0) {
		err = gpio_direction_output(g_himax_zf_ts_data->pdata->gpio_3v3_en, 0);
		if (err) {
			TS_LOG_ERR("unable to set direction for gpio [%d]\n",
				g_himax_zf_ts_data->pdata->gpio_3v3_en);
			return err;
		}
	}
	if (g_himax_zf_ts_data->pdata->gpio_1v8_en >= 0) {
		err = gpio_direction_output(g_himax_zf_ts_data->pdata->gpio_1v8_en, 0);
		if (err) {
			TS_LOG_ERR("unable to set direction for gpio [%d]\n",
				g_himax_zf_ts_data->pdata->gpio_1v8_en);
			return err;
		}
	}
	hx_power_off_gpio_set();
	return err;
}

static void hx_shutdown(void)
{
	TS_LOG_INFO("%s call power off\n", __func__);
	hx_power_off();
}

static int himax_algo_cp(struct ts_kit_device_data *dev_data, struct ts_fingers *in_info, struct ts_fingers *out_info)
{
	int index = 0;
	int id = 0;

	TS_LOG_INFO("hx_algo_cp Enter");
	if ((dev_data == NULL) || (in_info == NULL) || (out_info == NULL))
		return HX_ERROR;

	for (index = 0, id = 0; index < TS_MAX_FINGER; index++, id++) {
		if (in_info->cur_finger_number == 0) {
			out_info->fingers[0].status = TS_FINGER_RELEASE;
			if (id >= 1)
				out_info->fingers[id].status = 0;
		} else {
			if (in_info->fingers[index].x != 0 || in_info->fingers[index].y != 0) {
				if (in_info->fingers[index].event == HIMAX_EV_TOUCHDOWN
				    || in_info->fingers[index].event == HIMAX_EV_MOVE
				    || in_info->fingers[index].event == HIMAX_EV_NO_EVENT) {
					out_info->fingers[id].x = in_info->fingers[index].x;
					out_info->fingers[id].y = in_info->fingers[index].y;
					out_info->fingers[id].pressure = in_info->fingers[index].pressure;
					out_info->fingers[id].status = TS_FINGER_PRESS;
				} else if (in_info->fingers[index].event == HIMAX_EV_LIFTOFF)
					out_info->fingers[id].status = TS_FINGER_RELEASE;
				else
					TS_LOG_INFO("hx_algo_cp  Nothing to do.");
			} else
				out_info->fingers[id].status = 0;
		}
	}
	return NO_ERR;
}

static struct ts_algo_func himax_algo_f1 = {
	.algo_name = "himax_algo_cp",
	.chip_algo_func = himax_algo_cp,
};

static int hx_register_algo(struct ts_kit_device_data *chip_data)
{
	int retval = -EIO;

	TS_LOG_INFO("%s: himax_reg_algo called\n", __func__);
	if (NULL == chip_data)
		return retval;

	retval = register_ts_algo_func(chip_data, &himax_algo_f1);
	return retval;
}

static int hx_chip_check_status(void)
{
	TS_LOG_DEBUG("%s +\n", __func__);
	TS_LOG_DEBUG("%s -\n", __func__);
	return 0;
}

static int hx_chip_get_info(struct ts_chip_info_param *info)
{
	int retval = NO_ERR;
	struct ts_kit_device_data *chip_data = g_himax_zf_ts_data->tskit_himax_data;

	TS_LOG_INFO("%s Enter\n", __func__);
	if (info == NULL)
		return HX_ERROR;

	if (chip_data->ts_platform_data->hide_plain_id) {
		snprintf(info->ic_vendor, HX_PROJECT_ID_LEN + 1, "%s",
			himax_zf_project_id);
	} else {
		snprintf(info->ic_vendor, VENDER_NAME_LENGTH, "himax");
	}
	snprintf(info->mod_vendor, HX_PROJECT_ID_LEN + 1, "%s",
		himax_zf_project_id);
	snprintf(info->fw_vendor, CHIP_INFO_LENGTH*2,
		"%x.%x.%x.%x.%x", g_himax_zf_ts_data->vendor_panel_ver,
		g_himax_zf_ts_data->vendor_fw_ver,
		g_himax_zf_ts_data->vendor_config_ver,
		g_himax_zf_ts_data->vendor_cid_maj_ver,
		g_himax_zf_ts_data->vendor_cid_min_ver);

	if (chip_data->ts_platform_data->hide_plain_id)
		snprintf(info->ic_vendor, CHIP_INFO_LENGTH * 2,
			"%s-%s", info->mod_vendor, info->fw_vendor);

	return retval;
}

bool hx_zf_sense_off(void)
{
	uint8_t cnt = 0;
	uint8_t tmp_addr[4] = {0};
	uint8_t tmp_data[4] = {0};
	bool check_en = false;

	do {
		if (cnt == 0
		|| (tmp_data[0] != 0xA5
		&& tmp_data[0] != 0x00
		&& tmp_data[0] != 0x87)) {
			TS_LOG_INFO("Ready to set fw_addr_ctrl_fw<=fw_data_fw_stop!\n");
			hx_reg_assign(HX_FW_ADDR_CTRL_FW, tmp_addr);
			hx_reg_assign(HX_FW_DATA_FW_STOP, tmp_data);
			hx_zf_write_burst(tmp_addr, tmp_data);
			usleep_range(1000, 1100);
		}

		hx_reg_assign(ADDR_READ_MODE_CHK, tmp_addr);
		hx_zf_register_read(tmp_addr, FOUR_BYTE_CMD, tmp_data);
		if (tmp_data[0] != HX_FW_WAKING) {
			TS_LOG_INFO("No need to set 87!\n");
			break;
		}
		hx_reg_assign(HX_FW_ADDR_CTRL_FW, tmp_addr);
		hx_reg_assign(HX_FW_DATA_FW_STOP, tmp_data);
		hx_zf_register_read(tmp_addr, FOUR_BYTE_CMD, tmp_data);
		TS_LOG_INFO("%s: cnt = %d, data[0] = 0x%02X!\n", __func__,
			cnt, tmp_data[0]);

	} while (tmp_data[0] != 0x87 && (++cnt < 10) && check_en == true);

	cnt = 0;

	do {
		//===========================================
		//  0x31 ==> 0x27
		//===========================================
		tmp_data[0] = DATA_SENSE_SWITCH_1_OFF;
		if (hx_zf_bus_write(ADDR_SENSE_SWITCH_1, tmp_data,
			ONE_BYTE_CMD, sizeof(tmp_data), DEFAULT_RETRY_CNT) < 0) {
			TS_LOG_ERR("%s: bus access fail!\n", __func__);
			return false;
		}
		//===========================================
		//  0x32 ==> 0x95
		//===========================================
		tmp_data[0] = DATA_SENSE_SWITCH_2_OFF;
		if (hx_zf_bus_write(ADDR_SENSE_SWITCH_2, tmp_data,
			ONE_BYTE_CMD, sizeof(tmp_data), DEFAULT_RETRY_CNT) < 0) {
			TS_LOG_ERR("%s: bus access fail!\n", __func__);
			return false;
		}
		// ======================
		// Check enter_save_mode
		// ======================
		hx_reg_assign(ADDR_READ_MODE_CHK, tmp_addr);
		hx_zf_register_read(tmp_addr, FOUR_BYTE_CMD, tmp_data);
		TS_LOG_INFO("%s: Check enter_save_mode data[0]=%X\n", __func__, tmp_data[0]);

		if (tmp_data[0] == ENTER_SAVE_MODE) {
			//=====================================
			// Reset TCON
			//=====================================
			hx_addr_reg_assign(ADDR_RESET_TCON, DATA_INIT, tmp_addr, tmp_data);
			hx_zf_write_burst(tmp_addr, tmp_data);
			usleep_range(1000, 1100);
			tmp_data[0] = tmp_data[0] | 0x01;
			hx_zf_write_burst(tmp_addr, tmp_data);
			//=====================================
			// Reset ADC
			//=====================================
			hx_addr_reg_assign(ADDR_RESET_ADC, DATA_INIT, tmp_addr, tmp_data);
			hx_zf_write_burst(tmp_addr, tmp_data);
			usleep_range(1000, 1100);
			tmp_data[0] = tmp_data[0] | 0x01;
			hx_zf_write_burst(tmp_addr, tmp_data);
			return true;
		} else {
			hx_reset_device();
		}
	} while (cnt++ < 5);

	return false;
}

void hx_reload_to_active(void)
{
	uint8_t addr[FOUR_BYTE_CMD] = {0};
	uint8_t data[FOUR_BYTE_CMD] = {0};
	uint8_t retry_cnt = 0;

	TS_LOG_INFO("%s Enter \n", __func__);

	struct himax_ts_data *ts = NULL;

	ts = g_himax_zf_ts_data;

	if ((ts->support_hx_opt_hw_crc) && (g_zf_opt_crc.fw_addr != 0)) {
		TS_LOG_INFO("%s: Start opt crc\n", __func__);
		addr[3] = g_zf_opt_crc.start_addr[3];
		addr[2] = g_zf_opt_crc.start_addr[2];
		addr[1] = g_zf_opt_crc.start_addr[1];
		addr[0] = g_zf_opt_crc.start_addr[0];

		do {
			data[3] = g_zf_opt_crc.start_data[3];
			data[2] = g_zf_opt_crc.start_data[2];
			data[1] = g_zf_opt_crc.start_data[1];
			data[0] = g_zf_opt_crc.start_data[0];
			himax_zf_register_write(addr, FOUR_BYTE_CMD, data);
			usleep_range(1000, 1100);
			hx_zf_register_read(addr, FOUR_BYTE_CMD, data);
			TS_LOG_INFO("%s: 0x%02X%02X%02X%02X, retry_cnt=%d\n", __func__,
				data[3], data[2], data[1], data[0], retry_cnt);
			retry_cnt++;
		} while ((data[1] != g_zf_opt_crc.start_data[1]
			|| data[0] != g_zf_opt_crc.start_data[0])
			&& retry_cnt < DEFAULT_RETRY_CNT);

		retry_cnt = 0;
		addr[3] = g_zf_opt_crc.end_addr[3];
		addr[2] = g_zf_opt_crc.end_addr[2];
		addr[1] = g_zf_opt_crc.end_addr[1];
		addr[0] = g_zf_opt_crc.end_addr[0];

		do {
			data[3] = g_zf_opt_crc.end_data[3];
			data[2] = g_zf_opt_crc.end_data[2];
			data[1] = g_zf_opt_crc.end_data[1];
			data[0] = g_zf_opt_crc.end_data[0];

			himax_zf_register_write(addr, FOUR_BYTE_CMD, data);
			usleep_range(1000, 1100);
			hx_zf_register_read(addr, FOUR_BYTE_CMD, data);
			TS_LOG_INFO("%s: 0x%02X%02X%02X%02X, retry_cnt=%d\n", __func__,
				data[3], data[2], data[1], data[0], retry_cnt);
			retry_cnt++;
		} while ((data[1] != g_zf_opt_crc.end_data[1]
			|| data[0] != g_zf_opt_crc.end_data[0])
			&& retry_cnt < DEFAULT_RETRY_CNT);

	}

	TS_LOG_INFO("%s: Clear CRC\n", __func__);
	hx_addr_reg_assign(HX_ADDR_CRC_S1, HX_DATA_CRC_S1,
		addr, data);
	himax_zf_register_write(addr, FOUR_BYTE_CMD, data);
	hx_addr_reg_assign(HX_ADDR_CRC_S2, HX_DATA_CRC_S2,
		addr, data);
	himax_zf_register_write(addr, FOUR_BYTE_CMD, data);
	hx_addr_reg_assign(HX_ADDR_CRC_S3, HX_DATA_CRC_S3,
		addr, data);
	himax_zf_register_write(addr, FOUR_BYTE_CMD, data);

	do {
		hx_addr_reg_assign(HX_ADDR_CRC_S4, HX_DATA_CRC_S4,
		addr, data);
		himax_zf_register_write(addr, FOUR_BYTE_CMD, data);
		usleep_range(1000, 1100);
		hx_zf_register_read(addr, FOUR_BYTE_CMD, data);
		TS_LOG_INFO("%s: data[1]=%d, data[0]=%d, retry_cnt=%d\n", __func__,
				data[1], data[0], retry_cnt);
		retry_cnt++;
	} while (data[0] != 0x00 && retry_cnt < 5);

	retry_cnt = 0;
	TS_LOG_INFO("%s: Start reload to active\n", __func__);

	do {
		hx_addr_reg_assign(HX_ADDR_RELOAD_ACTIVE, HX_DATA_RELOAD_ACTIVE,
		addr, data);
		himax_zf_register_write(addr, FOUR_BYTE_CMD, data);
		usleep_range(1000, 1100);
		hx_zf_register_read(addr, FOUR_BYTE_CMD, data);
		TS_LOG_INFO("%s: data[1]=%d, data[0]=%d, retry_cnt=%d\n", __func__,
				data[1], data[0], retry_cnt);
		retry_cnt++;
	} while ((data[1] != 0x01
		|| data[0] != 0xEC)
		&& retry_cnt < 5);
}

static void hx_system_reset(void)
{
	uint8_t tmp_addr[FOUR_BYTE_CMD];
	uint8_t tmp_data[FOUR_BYTE_CMD];
	int retry = 0;

	hx_zf_interface_on();
	hx_addr_reg_assign(HX_FW_ADDR_CTRL_FW, DATA_INIT, tmp_addr, tmp_data);
	himax_zf_register_write(tmp_addr, FOUR_BYTE_CMD, tmp_data);
	usleep_range(1000, 1100);
	do {
		/* reset code*/
		/**
		 * password[7:0] set Enter safe mode : 0x31 ==> 0x27
		 */
		tmp_data[0] = DATA_SENSE_SWITCH_1_OFF;
		if (hx_zf_bus_write(ADDR_SENSE_SWITCH_1, tmp_data,
			ONE_BYTE_CMD, sizeof(tmp_data), DEFAULT_RETRY_CNT) < 0)
			TS_LOG_ERR("%s: bus access fail!\n", __func__);

		/**
		 * password[15:8] set Enter safe mode :0x32 ==> 0x95
		 */
		 tmp_data[0] = DATA_SENSE_SWITCH_2_OFF;
		if (hx_zf_bus_write(ADDR_SENSE_SWITCH_2, tmp_data,
			ONE_BYTE_CMD, sizeof(tmp_data), DEFAULT_RETRY_CNT) < 0)
			TS_LOG_ERR("%s: bus access fail!\n", __func__);

		/**
		 * password[7:0] set Enter safe mode : 0x31 ==> 0x00
		 */
		tmp_data[0] = DATA_SENSE_SWITCH_ON;
		if (hx_zf_bus_write(ADDR_SENSE_SWITCH_1, tmp_data,
			ONE_BYTE_CMD, sizeof(tmp_data), DEFAULT_RETRY_CNT) < 0)
			TS_LOG_ERR("%s: bus access fail!\n", __func__);

		usleep_range(1000, 1100);
		hx_reg_assign(ADDR_HW_STST_CHK, tmp_addr);
		hx_zf_register_read(tmp_addr, FOUR_BYTE_CMD, tmp_data);
		TS_LOG_INFO("%s:Read status from IC=%X,%X\n", __func__,
			tmp_data[0], tmp_data[1]);

	} while ((tmp_data[1] != 0x02 || tmp_data[0] != 0x00) && retry++ < 5);
}

void hx_zf_sense_on(uint8_t FlashMode)
{
	uint8_t tmp_addr[4] = {0};
	uint8_t tmp_data[4] = {0};
	int retry = 0;

	TS_LOG_INFO("Enter %s\n", __func__);

	hx_zf_interface_on();
	hx_addr_reg_assign(HX_FW_ADDR_CTRL_FW, DATA_INIT,
		tmp_addr, tmp_data);
	himax_zf_register_write(tmp_addr, FOUR_BYTE_CMD, tmp_data);
	usleep_range(10000, 10010);

	if (!FlashMode) {
		hx_reset_device();
	} else {
		do {
			hx_addr_reg_assign(HX_FW_ADDR_FLAG_RST_EVENT, DATA_INIT, tmp_addr, tmp_data);
			hx_zf_register_read(tmp_addr, FOUR_BYTE_CMD, tmp_data);
			TS_LOG_INFO("%s:Read status from IC = %X,%X\n",
				__func__, tmp_data[0], tmp_data[1]);

		} while ((tmp_data[1] != 0x01 || tmp_data[0] != 0x00) && retry++ < 5);

		if (retry >= 5) {
			TS_LOG_ERR("%s: Fail:\n", __func__);
			hx_reset_device();
		} else {
			TS_LOG_INFO("%s:OK and Read status from IC = %X,%X\n",
				__func__, tmp_data[0], tmp_data[1]);
			/* reset code*/
			tmp_data[0] = DATA_SENSE_SWITCH_ON;
			if (hx_zf_bus_write(ADDR_SENSE_SWITCH_1, tmp_data,
				ONE_BYTE_CMD, sizeof(tmp_data), DEFAULT_RETRY_CNT) < 0) {
				TS_LOG_ERR("%s: bus access fail!\n", __func__);
			}
			if (hx_zf_bus_write(ADDR_SENSE_SWITCH_2, tmp_data,
				ONE_BYTE_CMD, sizeof(tmp_data), DEFAULT_RETRY_CNT) < 0) {
				TS_LOG_ERR("%s: bus access fail!\n", __func__);
			}
		}
	}
	hx_reload_to_active();
}

static int touch_driver_projectid_read(uint8_t *buffer)
{
	uint32_t len;
	uint8_t type;
	int j;

	if (!buffer) {
		TS_LOG_ERR("%s:buffer null!\n", __func__);
		return -EINVAL;
	}

	len = (buffer[DATA_3] << LEFT_MOV_24BIT) |
		(buffer[DATA_2] << LEFT_MOV_16BIT) |
		(buffer[DATA_1] << LEFT_MOV_8BIT) |
		buffer[DATA_0];
	type = buffer[DATA_7];

	/* project id */
	if (type == DATA_0)
		huawei_project_id = kzalloc((len + DATA_1) *
			sizeof(uint8_t), GFP_KERNEL);
	else if (type == DATA_1)
		huawei_project_id = kzalloc(len *
			sizeof(uint8_t), GFP_KERNEL);
	else
		TS_LOG_ERR("%s: project id UNKNOWN type %d\n",
			__func__, type);

	if (huawei_project_id != NULL) {
		memcpy(huawei_project_id, buffer + DATA_8, len);
		huawei_project_id_len = len;
		TS_LOG_INFO("%s: project id: ", __func__);
		if (type == DATA_0) {
			huawei_project_id[len] = '\0';
			TS_LOG_INFO("%s", huawei_project_id);
		} else {
			for (j = 0; j < len; j++)
				TS_LOG_INFO("0x%02X",
					*(huawei_project_id + j));
		}
		TS_LOG_INFO("\n");
	}
	return NO_ERR;
}

static int touch_driver_tpcolor_read(uint8_t *buffer)
{
	uint32_t len;
	uint8_t type;
	int j;

	if (!buffer) {
		TS_LOG_ERR("%s:buffer null!\n", __func__);
		return -EINVAL;
	}
	len = (buffer[DATA_3] << LEFT_MOV_24BIT) |
		(buffer[DATA_2] << LEFT_MOV_16BIT) |
		(buffer[DATA_1] << LEFT_MOV_8BIT) |
		buffer[DATA_0];
	type = buffer[DATA_7];
	/* CG color */
	if (type == DATA_0)
		huawei_cg_color = kzalloc((len + DATA_1) *
			 sizeof(uint8_t), GFP_KERNEL);
	else if (type == DATA_1)
		huawei_cg_color = kzalloc(len *
			sizeof(uint8_t), GFP_KERNEL);
	else
		TS_LOG_ERR("%s: CG color UNKNOWN type %d\n",
			__func__, type);

	if (huawei_cg_color != NULL) {
		memcpy(huawei_cg_color, buffer + DATA_8, len);
		huawei_cg_color_len = len;
		TS_LOG_INFO("%s: cg_color: ", __func__);
		if (type == DATA_0) {
			huawei_cg_color[len] = '\0';
			TS_LOG_INFO("%s", huawei_cg_color);
		} else {
			for (j = 0; j < len; j++)
				TS_LOG_INFO("0x%02X",
					*(huawei_cg_color + j));
		}
		TS_LOG_INFO("\n");
	}
	return NO_ERR;
}

static int touch_driver_read_panel_info(void)
{
	uint8_t *buffer = NULL;
	uint8_t tmp_addr[DATA_4] = {0};
	uint8_t tmp_buf[NOR_READ_LENGTH] = {0};
	uint32_t temp_addr;
	int i;
	uint32_t saddr;
	uint8_t title[DATA_3] = {0};

	buffer = kzalloc(INFO_PAGE_LENGTH * sizeof(uint8_t),
		GFP_KERNEL);
	if (!buffer) {
		TS_LOG_ERR("%s: Memory allocate FAIL!\n", __func__);
		return -ENOMEM;
	}
	hx_zf_sense_off();
	hx_zf_burst_enable(DATA_0);
	for (i = 0; i < INFO_SECTION_NUM; i++) {
		saddr = INFO_START_ADDR + i * INFO_PAGE_LENGTH;
		tmp_addr[DATA_0] = saddr % HEX_ONE_HUNDRED;
		tmp_addr[DATA_1] = (saddr >> RIGHT_MOV_8BIT) % HEX_ONE_HUNDRED;
		tmp_addr[DATA_2] = (saddr >> RIGHT_MOV_16BIT) % HEX_ONE_HUNDRED;
		tmp_addr[DATA_3] = saddr / HEX_ONE_MILLION;
		if (hx_zf_hw_check_crc(tmp_addr,
			INFO_PAGE_LENGTH) != 0) {
			TS_LOG_ERR("%s: panel info section %d CRC FAIL\n",
				__func__, i);
			return -EINVAL;
		}
		for (temp_addr = saddr; temp_addr < saddr +
			INFO_PAGE_LENGTH; temp_addr += NOR_READ_LENGTH) {
			tmp_addr[DATA_0] = temp_addr % HEX_ONE_HUNDRED;
			tmp_addr[DATA_1] = (temp_addr >> RIGHT_MOV_8BIT) %
				HEX_ONE_HUNDRED;
			tmp_addr[DATA_2] = (temp_addr >> RIGHT_MOV_16BIT) %
				HEX_ONE_HUNDRED;
			tmp_addr[DATA_3] = temp_addr / HEX_ONE_MILLION;
			hx_zf_register_read(tmp_addr,
				NOR_READ_LENGTH, tmp_buf);
			memcpy(&buffer[temp_addr - saddr],
				tmp_buf, NOR_READ_LENGTH);
		}
		title[DATA_0] = buffer[DATA_4];
		title[DATA_1] = buffer[DATA_5];
		title[DATA_2] = buffer[DATA_6];
		if (title[DATA_0] == DATA_0 &&
			title[DATA_1] == DATA_1 && title[DATA_2] == DATA_0) {
			touch_driver_projectid_read(buffer);
		} else if (title[DATA_0] == DATA_0 &&
			title[DATA_1] == DATA_0 && title[DATA_2] == DATA_1) {
			touch_driver_tpcolor_read(buffer);
		} else {
			TS_LOG_ERR("%s: UNKNOWN title %X%X%X\n",
				__func__, title[DATA_0],
				title[DATA_1], title[DATA_2]);
		}
	}
	if (buffer != NULL)
		kfree(buffer);
	hx_zf_sense_on(SENSE_ON_1);
	return NO_ERR;
}

static int touch_driver_get_projectid_color(struct himax_ts_data *ts)
{
	memcpy(ts->color_id, huawei_cg_color,
		sizeof(ts->color_id));
	if (ts->support_get_tp_color) {
		cypress_ts_kit_color[DATA_0] = ts->color_id[DATA_0];
		TS_LOG_INFO("support_get_tp_color, tp color:0x%x\n",
			cypress_ts_kit_color[DATA_0]); /* 1th tpcolor */
	}
	if (ts->support_read_projectid) {
		memcpy(ts->project_id, huawei_project_id,
			HIMAX_ACTUAL_PROJECTID_LEN);
		ts->project_id[HIMAX_ACTUAL_PROJECTID_LEN] = '\0';
		memcpy(himax_zf_project_id, huawei_project_id,
			HIMAX_ACTUAL_PROJECTID_LEN);
		himax_zf_project_id[HIMAX_ACTUAL_PROJECTID_LEN] = '\0';
		TS_LOG_INFO("support_read_projectid, projectid:%s\n",
			ts->project_id);
	}
	return NO_ERR;
}

void hx_zf_interface_on(void)
{
	int cnt = 0;
	uint8_t tmp_data[5] = {0};
	uint8_t tmp_buf[2] = {0};

	if (himax_zf_bus_read(DUMMY_REGISTER, tmp_data,
		FOUR_BYTE_CMD, sizeof(tmp_data), DEFAULT_RETRY_CNT) < 0) {
		TS_LOG_ERR("%s: bus access fail!\n", __func__);
		return;
	}
	do {
		//===========================================
		// Enable continuous burst mode : 0x13 ==> 0x31
		//===========================================
		tmp_data[0] = DATA_EN_BURST_MODE;
		if (hx_zf_bus_write(ADDR_EN_BURST_MODE, tmp_data,
			ONE_BYTE_CMD, sizeof(tmp_data), DEFAULT_RETRY_CNT) < 0) {
			TS_LOG_ERR("%s: bus access fail!\n", __func__);
			return;
		}
		//===========================================
		// AHB address auto +4		: 0x0D ==> 0x11
		// Do not AHB address auto +4 : 0x0D ==> 0x10
		//===========================================
		tmp_data[0] = DATA_AHB;
		if (hx_zf_bus_write(ADDR_AHB, tmp_data,
			ONE_BYTE_CMD, sizeof(tmp_data), DEFAULT_RETRY_CNT) < 0) {
			TS_LOG_ERR("%s: bus access fail!\n", __func__);
			return;
		}
		// Check cmd
		himax_zf_bus_read(ADDR_EN_BURST_MODE, tmp_data, ONE_BYTE_CMD, sizeof(tmp_data), DEFAULT_RETRY_CNT);
		himax_zf_bus_read(ADDR_AHB, tmp_buf, ONE_BYTE_CMD, sizeof(tmp_buf), DEFAULT_RETRY_CNT);

		if (tmp_data[0] == DATA_EN_BURST_MODE && tmp_buf[0] == DATA_AHB)
			break;
		msleep(HX_SLEEP_1MS);
	} while (++cnt < 10);

	if (cnt > 0)
		TS_LOG_INFO("%s:Polling burst mode: %d times", __func__, cnt);
}

void crc_data_prepare(const struct firmware *fw)
{
	struct himax_ts_data *ts = NULL;

	TS_LOG_INFO("%s enter", __func__);
	ts = g_himax_zf_ts_data;
	if ((ts->support_hx_opt_hw_crc) && (g_zf_opt_crc.fw_addr != 0)) {
		g_zf_opt_crc.start_addr[3] = fw->data[g_zf_opt_crc.fw_addr + 3];
		g_zf_opt_crc.start_addr[2] = fw->data[g_zf_opt_crc.fw_addr + 2];
		g_zf_opt_crc.start_addr[1] = fw->data[g_zf_opt_crc.fw_addr + 1];
		g_zf_opt_crc.start_addr[0] = fw->data[g_zf_opt_crc.fw_addr];

		g_zf_opt_crc.start_data[3] = fw->data[g_zf_opt_crc.fw_addr + 7];
		g_zf_opt_crc.start_data[2] = fw->data[g_zf_opt_crc.fw_addr + 6];
		g_zf_opt_crc.start_data[1] = fw->data[g_zf_opt_crc.fw_addr + 5];
		g_zf_opt_crc.start_data[0] = fw->data[g_zf_opt_crc.fw_addr + 4];

		g_zf_opt_crc.end_addr[3] = fw->data[g_zf_opt_crc.fw_addr + 11];
		g_zf_opt_crc.end_addr[2] = fw->data[g_zf_opt_crc.fw_addr + 10];
		g_zf_opt_crc.end_addr[1] = fw->data[g_zf_opt_crc.fw_addr + 9];
		g_zf_opt_crc.end_addr[0] = fw->data[g_zf_opt_crc.fw_addr + 8];

		g_zf_opt_crc.end_data[3] = fw->data[g_zf_opt_crc.fw_addr + 15];
		g_zf_opt_crc.end_data[2] = fw->data[g_zf_opt_crc.fw_addr + 14];
		g_zf_opt_crc.end_data[1] = fw->data[g_zf_opt_crc.fw_addr + 13];
		g_zf_opt_crc.end_data[0] = fw->data[g_zf_opt_crc.fw_addr + 12];

		TS_LOG_INFO("%s: opt_crc start: R%02X%02X%02X%02XH <= 0x%02X%02X%02X%02X\n", __func__,
			g_zf_opt_crc.start_addr[3], g_zf_opt_crc.start_addr[2], g_zf_opt_crc.start_addr[1], g_zf_opt_crc.start_addr[0],
			g_zf_opt_crc.start_data[3], g_zf_opt_crc.start_data[2], g_zf_opt_crc.start_data[1], g_zf_opt_crc.start_data[0]);

		TS_LOG_INFO("%s: opt_crc end: R%02X%02X%02X%02XH <= 0x%02X%02X%02X%02X\n", __func__,
			g_zf_opt_crc.end_addr[3], g_zf_opt_crc.end_addr[2], g_zf_opt_crc.end_addr[1], g_zf_opt_crc.end_addr[0],
			g_zf_opt_crc.end_data[3], g_zf_opt_crc.end_data[2], g_zf_opt_crc.end_data[1], g_zf_opt_crc.end_data[0]);
	}
}

uint32_t hx_zf_hw_check_crc(uint8_t *start_addr,
		int reload_length)
{
	uint32_t result;
	uint8_t tmp_addr[DATA_4] = {0};
	uint8_t tmp_data[DATA_4] = {0};
	uint32_t mod;
	int cnt;
	unsigned int length = reload_length / DATA_4;

	hx_reg_assign(CRC_ADDR, tmp_addr);
	himax_zf_register_write(tmp_addr, FOUR_BYTE_CMD, start_addr);
	tmp_data[DATA_3] = TMP_DATA0;
	tmp_data[DATA_2] = TMP_DATA1;
	tmp_data[DATA_1] = length >> RIGHT_MOV_8BIT;
	tmp_data[DATA_0] = length;
	hx_reg_assign(ADDR_CRC_DATAMAXLEN_SET, tmp_addr);
	himax_zf_register_write(tmp_addr, FOUR_BYTE_CMD, tmp_data);
	cnt = DATA_0;
	hx_reg_assign(ADDR_CRC_STATUS_SET, tmp_addr);
	do {
		mod = ADDR_CRC_STATUS_SET % DATA_4;
		hx_zf_register_read(tmp_addr, FOUR_BYTE_CMD, tmp_data);
		TS_LOG_INFO("%s:tmp_data[3]=%X, tmp_data[2]=%X\n",
			__func__, tmp_data[DATA_3], tmp_data[DATA_2]);
		TS_LOG_INFO("%s:tmp_data[1]=%X, tmp_data[0]=%X\n",
			__func__, tmp_data[DATA_1], tmp_data[DATA_0]);
		if ((tmp_data[DATA_0 + mod] & HEX_NUM1) != HEX_NUM1) {
			hx_reg_assign(ADDR_CRC_RESULT, tmp_addr);
			hx_zf_register_read(tmp_addr,
				FOUR_BYTE_CMD, tmp_data);
			TS_LOG_INFO("%s: tmp_data[3]=%X, tmp_data[2]=%X\n",
				 __func__, tmp_data[DATA_3], tmp_data[DATA_2]);
			TS_LOG_INFO("%s: tmp_data[1]=%X, tmp_data[0]=%X\n",
				__func__, tmp_data[DATA_1], tmp_data[DATA_0]);
			result = ((tmp_data[DATA_3] << LEFT_MOV_24BIT) +
				(tmp_data[DATA_2] << LEFT_MOV_16BIT) +
				(tmp_data[DATA_1] << LEFT_MOV_8BIT) +
				tmp_data[DATA_0]);
			break;
		}
		TS_LOG_INFO("wait for HW ready!\n");
	} while (cnt++ < CNT);

	if (cnt < CNT)
		return result;
	else
		return FWU_FW_CRC_ERROR;
}

void hx_zf_get_dsram_data(uint8_t *info_data)
{
	int i = 0;
	int address = 0;
	int fw_run_flag = -1;
	int m_key_num = 0;
	int total_size_temp = 0;
	int total_read_times = 0;
	unsigned char tmp_addr[4] = {0};
	unsigned char tmp_data[4] = {0};
	uint8_t max_spi_size = 128;
	uint8_t x_num = HX_ZF_RX_NUM;
	uint8_t y_num = HX_ZF_TX_NUM;
	int mutual_data_size = x_num * y_num * 2;
	int total_size =  (x_num * y_num + x_num + y_num) * 2 + 4;
	uint16_t check_sum_cal = 0;
	uint8_t *temp_info_data = NULL;

	temp_info_data = kcalloc(total_size,
		sizeof(uint8_t), GFP_KERNEL);
	if (!temp_info_data) {
		TS_LOG_ERR("%s: Memory allocate FAIL!\n",
			__func__);
		return;
	}

	/*1. Read number of MKey R100070E8H to determin data size*/
	hx_reg_assign(ADDR_MKEY, tmp_addr);
	hx_zf_register_read(tmp_addr, FOUR_BYTE_CMD, tmp_data);
	m_key_num = tmp_data[0] & 0x03;
	total_size += m_key_num * 2;

	/* 2. Start DSRAM Rawdata and Wait Data Ready */
	hx_addr_reg_assign(ADDR_DSRAM_START, DATA_DSRAM_START,
		tmp_addr, tmp_data);
	fw_run_flag = himax_zf_write_read_reg(tmp_addr, tmp_data, 0xA5, 0x5A);

	if (fw_run_flag < 0) {
		TS_LOG_INFO("%s Data NOT ready => bypass\n", __func__);
		return;
	}

	/* 3. Read RawData */
	total_size_temp = total_size;

	hx_reg_assign(ADDR_DSRAM_START, tmp_addr);

	if (total_size % max_spi_size == 0)
		total_read_times = total_size / max_spi_size;
	else
		total_read_times = total_size / max_spi_size + 1;

	for (i = 0; i < (total_read_times); i++) {
		if (total_size_temp >= max_spi_size) {
			hx_zf_register_read(tmp_addr, max_spi_size,
				&temp_info_data[i * max_spi_size]);

			total_size_temp = total_size_temp - max_spi_size;
		} else {
			TS_LOG_DEBUG("last total_size_temp=%d\n", total_size_temp);
			hx_zf_register_read(tmp_addr,
				total_size_temp % max_spi_size,
				&temp_info_data[i * max_spi_size]);
		}

		address = ((i+1)*max_spi_size);
		tmp_addr[1] = (uint8_t)((address>>8)&0x00FF);
		tmp_addr[0] = (uint8_t)((address)&0x00FF);
	}

	/* 4. FW stop outputing */
	TS_LOG_DEBUG("zf_dsram_flag=%d\n", zf_dsram_flag);
	if (zf_dsram_flag == false)	{
		TS_LOG_DEBUG("Return to Event Stack!\n");
		hx_addr_reg_assign(ADDR_DSRAM_START, DATA_INIT,
			tmp_addr, tmp_data);
		hx_zf_write_burst(tmp_addr, tmp_data);
	} else {
		TS_LOG_DEBUG("Continue to SRAM!\n");
		hx_addr_reg_assign(ADDR_DSRAM_START, DATA_RETURN_SRAM_EVENT,
			tmp_addr, tmp_data);
		hx_zf_write_burst(tmp_addr, tmp_data);
	}
	/* 5. Data Checksum Check */
	for (i = 2; i < total_size; i = i+2) {/* 2:PASSWORD NOT included */
		check_sum_cal += (temp_info_data[i + DATA_1] *
			SHIFT_NUM + temp_info_data[i]);
	}
	if (check_sum_cal % 0x10000 != 0) {
		TS_LOG_INFO("%s check_sum_cal fail=%2X\n", __func__, check_sum_cal);
		if (g_himax_zf_ts_data->debug_log_level & BIT(DATA_5)) {
			TS_LOG_DEBUG("%s skip checksum fail\n",
				__func__);
			memcpy(info_data, &temp_info_data[DATA_4],
				mutual_data_size * sizeof(uint8_t));
		}
		return;
	} else {
		memcpy(info_data, &temp_info_data[DATA_4],
			mutual_data_size * sizeof(uint8_t));

		TS_LOG_DEBUG("%s checksum PASS\n", __func__);
	}
	kfree(temp_info_data);
}

void hx_zf_diag_register_set(uint8_t diag_command)
{
	uint8_t tmp_addr[4] = {0};
	uint8_t tmp_data[4] = {0};

	TS_LOG_INFO("diag_command = %d\n", diag_command);

	hx_zf_interface_on();

	hx_addr_reg_assign(ADDR_DIAG_REG_SET,
		(int)diag_command, tmp_addr, tmp_data);

	hx_zf_write_burst(tmp_addr, tmp_data);

	hx_zf_register_read(tmp_addr, FOUR_BYTE_CMD, tmp_data);
	TS_LOG_INFO("%s: tmp_data[3]=0x%02X,tmp_data[2]=0x%02X,tmp_data[1]=0x%02X,tmp_data[0]=0x%02X!\n",
		__func__, tmp_data[3], tmp_data[2], tmp_data[1], tmp_data[0]);

}

void hx_zf_reload_disable(void)
{
	uint8_t tmp_addr[4] = {0};
	uint8_t tmp_data[4] = {0};

	TS_LOG_INFO("%s:entering\n", __func__);
	/*Diable Flash Reload*/
	hx_addr_reg_assign(ADDR_SWITCH_FLASH_RLD_STS,
		DATA_DISABLE_ZF_FLASH_RLD, tmp_addr, tmp_data);
	himax_zf_register_write(tmp_addr, FOUR_BYTE_CMD, tmp_data);
	TS_LOG_INFO("%s:end!\n", __func__);
}

void hx_zf_idle_mode(int disable)
{
	uint8_t tmp_addr[4] = {0};
	uint8_t tmp_data[4] = {0};

	TS_LOG_INFO("%s:entering\n", __func__);
	if (disable) {
		hx_reg_assign(ADDR_IDLE_MODE, tmp_addr);
		hx_zf_register_read(tmp_addr, FOUR_BYTE_CMD, tmp_data);

		tmp_data[0] = tmp_data[0] & 0xF7;

		hx_zf_write_burst(tmp_addr, tmp_data);

		hx_zf_register_read(tmp_addr, FOUR_BYTE_CMD, tmp_data);
		TS_LOG_INFO("%s:After turn ON/OFF IDLE Mode [0] = 0x%02X,[1] = 0x%02X,[2] = 0x%02X,[3] = 0x%02X\n",
			__func__, tmp_data[0], tmp_data[1], tmp_data[2], tmp_data[3]);
	} else
		TS_LOG_INFO("%s: enable!\n", __func__);
}

int hx_zf_switch_mode(int mode)
{
	uint8_t tmp_addr[4] = {0};
	uint8_t tmp_data[4] = {0};
	uint8_t mode_wirte_cmd = 0;
	uint8_t mode_read_cmd = 0;
	int result = -1;
	int retry = 200;

	TS_LOG_INFO("%s: Entering\n", __func__);

	hx_zf_sense_off();

	/* clean up FW status */
	hx_addr_reg_assign(ADDR_DSRAM_START, DATA_INIT,
		tmp_addr, tmp_data);
	hx_zf_write_burst(tmp_addr, tmp_data);

	if (mode == 0) {/*normal mode*/
		hx_addr_reg_assign(ADDR_SORTING_MODE_SWITCH, DATA_NOR_MODE,
			tmp_addr, tmp_data);
		mode_wirte_cmd = tmp_data[1];
		mode_read_cmd = tmp_data[0];
	} else {/*sorting mode*/
		hx_addr_reg_assign(ADDR_SORTING_MODE_SWITCH, DATA_SORT_MODE,
			tmp_addr, tmp_data);
		mode_wirte_cmd = tmp_data[1];
		mode_read_cmd = tmp_data[0];
	}

	hx_zf_write_burst(tmp_addr, tmp_data);
	hx_zf_idle_mode(ON);

	// To stable the sorting
	if (mode) {
		hx_addr_reg_assign(ADDR_SORTING_MODE_SWITCH,
			DATA_SORT_MODE_NFRAME, tmp_addr, tmp_data);
		hx_zf_write_burst(tmp_addr, tmp_data);
	} else {
		hx_addr_reg_assign(ADDR_NFRAME_SEL, DATA_SET_IIR_FRM,
			tmp_addr, tmp_data);/*0x0A normal mode 10 frame*/
		/* N Frame Sorting*/
		hx_zf_write_burst(tmp_addr, tmp_data);
		hx_zf_idle_mode(ON);
	}

	hx_zf_sense_on(SENSE_ON_1);
	TS_LOG_INFO("mode_wirte_cmd(0)=0x%2.2X,mode_wirte_cmd(1)=0x%2.2X\n",
		tmp_data[0], tmp_data[1]);
	while (retry != 0) {
		TS_LOG_INFO("[%d]Read 100007FC!\n", retry);

		hx_reg_assign(ADDR_SORTING_MODE_SWITCH, tmp_addr);
		hx_zf_register_read(tmp_addr, FOUR_BYTE_CMD, tmp_data);
		msleep(HX_SLEEP_100MS);
		TS_LOG_INFO("mode_read_cmd(0)=0x%2.2X,mode_read_cmd(1)=0x%2.2X\n",
			tmp_data[0], tmp_data[1]);
		if (tmp_data[0] == mode_read_cmd &&
			tmp_data[1] == mode_read_cmd) {
			TS_LOG_INFO("Read OK!\n");
			result = NO_ERR;
			break;
		}

		hx_reg_assign(ADDR_READ_MODE_CHK, tmp_addr);
		hx_zf_register_read(tmp_addr, FOUR_BYTE_CMD, tmp_data);
		if (tmp_data[0] == 0x00 && tmp_data[1] == 0x00 &&
			tmp_data[2] == 0x00 && tmp_data[3] == 0x00)	{
			TS_LOG_ERR("%s,: FW Stop!\n", __func__);
			break;
		}
		retry--;
	}

	if (result == NO_ERR) {
		if (mode == 0)
			return NORMAL_MODE;
		else
			return SORTING_MODE;
	} else
		return HX_ERR;    //change mode fail
}

void hx_zf_return_event_stack(void)
{
	int retry = 20;
	uint8_t tmp_addr[4] = {0};
	uint8_t tmp_data[4] = {0};

	TS_LOG_INFO("%s:entering\n", __func__);
	do {
		TS_LOG_INFO("%s,now %d times\n!", __func__, retry);

		hx_addr_reg_assign(ADDR_DSRAM_START, DATA_INIT,
			tmp_addr, tmp_data);
		hx_zf_write_burst(tmp_addr, tmp_data);

		hx_zf_register_read(tmp_addr, FOUR_BYTE_CMD, tmp_data);
		retry--;
		msleep(HX_SLEEP_10MS);

	} while ((tmp_data[1] != 0x00 && tmp_data[0] != 0x00) && retry > 0);

	TS_LOG_INFO("%s: End of setting!\n", __func__);
}

static int touch_driver_rawdata_proc_printf(
	struct seq_file *m, struct ts_rawdata_info *info,
	int range_size, int row_size)
{
	int index;
	int index1;
	int rx_num = info->hybrid_buff[DATA_0];
	int tx_num = info->hybrid_buff[DATA_1];

	if ((range_size == DATA_0) || (row_size == DATA_0)) {
		TS_LOG_ERR("%s: range_size OR row_size is 0\n", __func__);
		return -EINVAL;
	}
	if (rx_num <= DATA_0 || rx_num > RAWDATA_NUM_OF_TRX_MAX ||
		tx_num <= DATA_0 || tx_num > RAWDATA_NUM_OF_TRX_MAX) {
		TS_LOG_ERR("%s: rx_num or tx_num is wrong value\n", __func__);
		return -EINVAL;
	}
	TS_LOG_INFO("range_size = %d, row_size = %d, info->used_size = %d\n", range_size, row_size, (int)(info->used_size));
	for (index = 0; row_size * index + DATA_2 < info->used_size; index++) {
		if (index == DATA_0) {
			seq_puts(m, "rawdata begin\n");
		} else if (index == range_size) {
			seq_puts(m, "rawdata end\n");
			seq_puts(m, "noise begin\n");
		} else if (index == range_size * DATA_2) {
			seq_puts(m, "noise end\n");
			seq_puts(m, "open begin\n");
		} else if (index == range_size * DATA_3) {
			seq_puts(m, "open end\n");
			seq_puts(m, "micro open begin\n");
		} else if (index == range_size * DATA_4) {
			seq_puts(m, "micro open end\n");
			seq_puts(m, "short begin\n");
		}
		for (index1 = 0; index1 < row_size; index1++)
			seq_printf(m, "%4d,",
				info->buff[DATA_2 + row_size * index + index1]);
		seq_puts(m, "\n ");
	}
	if (g_ts_kit_platform_data.chip_data->trx_delta_test_support) {
		seq_puts(m, "tx_delta_buf\n");
		for (index = 0; index < range_size - DATA_1; index++) {
			for (index1 = 0; index1 < row_size; index1++)
				seq_printf(m, "%4d,",
					info->tx_delta_buf[index *
					row_size + index1]);
			seq_puts(m, "\n");
		}
		seq_puts(m, "rx_delta_buf\n");
		for (index = 0; index < range_size; index++) {
			for (index1 = 0; index1 < row_size - DATA_1; index1++)
				seq_printf(m, "%4d,",
					info->rx_delta_buf[index *
					(row_size - DATA_1) + index1]);
			seq_puts(m, "\n");
		}
			seq_printf(m, "%llX\n", (uint64_t)info->tx_delta_buf);
			seq_printf(m, "%llX\n", (uint64_t)info->rx_delta_buf);
	}
	return NO_ERR;
}

#if defined(HUAWEI_CHARGER_FB)
static int hx_charger_switch(struct ts_charger_info *info)
{
	uint8_t tmp_addr[DATA_4] = {0};
	uint8_t tmp_data[DATA_4] = {0};

	TS_LOG_INFO("%s: called\n", __func__);
	if (info == NULL) {
		TS_LOG_ERR("%s:pointer info is NULL\n", __func__);
		return -ENOMEM;
	}
	if (info->charger_switch == USB_PIUG_OUT) { /* usb plug out */
		TS_LOG_INFO("%s: usb plug out DETECTED\n", __func__);
		hx_addr_reg_assign(ADDR_AC_SWITCH_HIMAX,
			DATA_SET_AC_OFF, tmp_addr, tmp_data);
		himax_zf_register_write(tmp_addr, FOUR_BYTE_CMD, tmp_data);
	} else if (info->charger_switch == USB_PIUG_IN) { /* usb plug in */
		TS_LOG_INFO("%s: usb plug in DETECTED\n", __func__);
		hx_addr_reg_assign(ADDR_AC_SWITCH_HIMAX,
			DATA_SET_AC_ON, tmp_addr, tmp_data);
		himax_zf_register_write(tmp_addr, FOUR_BYTE_CMD, tmp_data);
	} else {
		TS_LOG_INFO("%s: unknown USB status, info->charger_switch = %d\n",
			__func__, info->charger_switch);
	}

	return NO_ERR;
}
#endif

#define HORIZON_ENABLE 1
#define HORIZON_DISABLE 0
static int hx_horizon_switch(struct ts_horizon_info *info)
{
	uint8_t tmp_addr[DATA_4] = {0};
	uint8_t tmp_data[DATA_4] = {0};
	int retval = NO_ERR;

	TS_LOG_INFO("%s: called\n", __func__);
	if (!info) {
		TS_LOG_ERR("%s: pointer info is NULL\n", __func__);
		return -EFAULT;
	}

	if (!info->horizon_supported) {
		TS_LOG_INFO("%s: horizon change is not supported, horizon_supported = %d",
			__func__, info->horizon_supported);
		return retval;
	}

	TS_LOG_INFO("%s: write_horizon_switch=%d\n", __func__, info->horizon_switch);
	if (!info->horizon_switch) {
		TS_LOG_INFO("%s: Vertical screen status 0xA55AA55A\n", __func__);
		hx_addr_reg_assign(ADDR_HORIZON_SWITCH_HIMAX, VERTICAL_SCREEN_STATUS,
			tmp_addr, tmp_data);
		himax_zf_register_write(tmp_addr, FOUR_BYTE_CMD, tmp_data);
	} else if (info->horizon_switch == HORIZON_ENABLE) {
		TS_LOG_INFO("%s: Horizontal screen status 0xA33AA33A\n", __func__);
		hx_addr_reg_assign(ADDR_HORIZON_SWITCH_HIMAX, HORIZONTAL_SCREEN_STATUS,
			tmp_addr, tmp_data);
		himax_zf_register_write(tmp_addr, FOUR_BYTE_CMD, tmp_data);
	} else {
		TS_LOG_INFO("%s: afferent screen status error!\n");
	}
	return retval;
}

struct ts_device_ops ts_kit_himax_zf_ops = {
	.chip_parse_config = hx_parse_dts,
	.chip_detect = hx_chip_detect,
	.chip_init = hx_init_chip,
	.chip_register_algo = hx_register_algo,
	.chip_input_config = hx_zf_input_config,
	.chip_irq_top_half = hx_irq_top_half,
	.chip_irq_bottom_half = hx_irq_bottom_half,
	.chip_suspend = hx_core_suspend,
	.chip_resume = hx_core_resume,
	.chip_after_resume = hx_after_resume,
	.chip_reset = hx_reset_device,
	.chip_fw_update_boot = hx_zf_fw_update_boot,
	.chip_fw_update_sd = hx_zf_fw_update_sd,
	.chip_get_info = hx_chip_get_info,
	.chip_get_rawdata = hx_get_rawdata,
	.chip_get_capacitance_test_type = hx_get_capacitance_test_type,
	.chip_shutdown = hx_shutdown,/*NOT tested*/
	.chip_wakeup_gesture_enable_switch = hmx_wakeup_gesture_enable_switch,
	.chip_palm_switch = himax_palm_switch,
	.chip_check_status = hx_chip_check_status,
	.chip_glove_switch = hx_glove_switch,
	.chip_horizon_switch = hx_horizon_switch,
#ifdef ROI
	.chip_roi_rawdata = hx_roi_rawdata,
	.chip_roi_switch = himax_roi_switch,
#endif
#if defined(HUAWEI_CHARGER_FB)
	.chip_charger_switch = hx_charger_switch,
#endif
	.chip_special_rawdata_proc_printf = touch_driver_rawdata_proc_printf,
};
static int himax_palm_switch(struct ts_palm_info *info)
{
	return NO_ERR;
}

static int __init hx_module_init(void)
{
	bool found = false;
	struct device_node *child = NULL;
	struct device_node *root = NULL;
	int err = NO_ERR;

	TS_LOG_INFO("[HXTP] hx_module_init called here\n");
	g_himax_zf_ts_data = kzalloc(sizeof(struct himax_ts_data), GFP_KERNEL);
	if (!g_himax_zf_ts_data) {
		TS_LOG_ERR("Failed to alloc mem for struct g_himax_zf_ts_data\n");
		err = -ENOMEM;
		goto himax_ts_data_alloc_fail;
	}
	g_himax_zf_ts_data->tskit_himax_data = kzalloc(sizeof(struct ts_kit_device_data), GFP_KERNEL);
	if (!g_himax_zf_ts_data->tskit_himax_data) {
		TS_LOG_ERR("Failed to alloc mem for struct tskit_himax_data\n");
		err =  -ENOMEM;
		goto tskit_himax_data_alloc_fail;
	}

	g_himax_zf_ts_data->hx_rawdata_buf_roi = kzalloc(sizeof(uint8_t) * (HX_RECEIVE_ROI_BUF_MAX_SIZE - HX_TOUCH_SIZE), GFP_KERNEL);
	if (g_himax_zf_ts_data->hx_rawdata_buf_roi == NULL)
		goto mem_alloc_fail_rawdata_buf;

	root = of_find_compatible_node(NULL, NULL, "huawei,ts_kit");
	if (!root) {
		TS_LOG_ERR("[HXTP]huawei_ts, find_compatible_node huawei,ts_kit error\n");
		err = -EINVAL;
		goto out;
	}
	/*find the chip node*/
	for_each_child_of_node(root, child)	{
		if (of_device_is_compatible(child, HIMAX_VENDER_NAME)) {
			found = true;
			break;
		}
	}
	if (!found) {
		TS_LOG_ERR(" not found chip HXTP child node  !\n");
		err = -EINVAL;
		goto out;
	}
	g_himax_zf_ts_data->tskit_himax_data->cnode = child;
	g_himax_zf_ts_data->tskit_himax_data->ops = &ts_kit_himax_zf_ops;
	TS_LOG_INFO("found child node!\n");
	err = huawei_ts_chip_register(g_himax_zf_ts_data->tskit_himax_data);
	if (err) {
		TS_LOG_ERR(" HXTP chip register fail !\n");
		goto out;
	}
	TS_LOG_INFO("HXTP chip_register sucess! teturn value=%d\n", err);
	return err;
out:
	kfree(g_himax_zf_ts_data->hx_rawdata_buf_roi);
	g_himax_zf_ts_data->hx_rawdata_buf_roi = NULL;

mem_alloc_fail_rawdata_buf:
	kfree(g_himax_zf_ts_data->tskit_himax_data);
	g_himax_zf_ts_data->tskit_himax_data = NULL;

tskit_himax_data_alloc_fail:
	kfree(g_himax_zf_ts_data);
	g_himax_zf_ts_data = NULL;

himax_ts_data_alloc_fail:

	return err;
}

static void __exit hx_module_exit(void)
{
	TS_LOG_INFO("hx_module_exit called here\n");
	kfree(g_himax_zf_ts_data->pdata);
	g_himax_zf_ts_data->pdata = NULL;
}

/*lint -save -e* */
late_initcall(hx_module_init);
module_exit(hx_module_exit);
/*lint -restore*/
MODULE_AUTHOR("Huawei Device Company");
MODULE_DESCRIPTION("Huawei TouchScreen Driver");
MODULE_LICENSE("GPL");
