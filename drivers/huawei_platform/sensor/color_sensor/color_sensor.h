/*
 * color_sensor.h
 *
 * code for color sensor
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
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

#ifndef __COLOR_SENSOR_H__
#define __COLOR_SENSOR_H__

#include <linux/types.h>
#include <linux/pm.h>
#include <linux/timer.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>

#define MAX_TARGET_CALI_LEN            9
#define MAX_REPORT_LEN                 9
#define MAX_RAW_DATA_LEN               (MAX_REPORT_LEN - 1)
#define MAX_NAME_STR_LEN               50
#define MAX_ALGO_TYPE_STR_LEN          16
#define TIME_OUT_DEFAULT               1500
#define ONE_SHOW_LEN                   10
#define MAX_FLICK_DATA_LEN             1024
#define NV_READ_TAG                    1
#define NV_WRITE_TAG                   0
#define UINT8                          uint8_t
#define INT8                           int8_t
#define UINT16                         uint16_t
#define INT16                          int16_t
#define UINT32                         uint32_t
#define INT32                          int32_t
// shift left 8 for comb a word
#define COMB_WORD(a, b)                (((a) << 8) | (b))

enum color_sensor_cal_states {
	CAL_STATE_GAIN_1,
	CAL_STATE_GAIN_2,
	CAL_STATE_GAIN_3,
	CAL_STATE_GAIN_4,
	CAL_STATE_GAIN_5,
	CAL_STATE_GAIN_LAST
};

struct color_sensor_cali_nv_t {
	uint32_t nv_x;
	uint32_t nv_y;
	uint32_t nv_z;
	uint32_t nv_ir;
	uint32_t cal_x_ratio[CAL_STATE_GAIN_LAST];
	uint32_t cal_y_ratio[CAL_STATE_GAIN_LAST];
	uint32_t cal_z_ratio[CAL_STATE_GAIN_LAST];
	uint32_t cal_ir_ratio[CAL_STATE_GAIN_LAST];
};

struct at_color_sensor_cali_nv_t {
	uint32_t nv_c;
	uint32_t nv_r;
	uint32_t nv_g;
	uint32_t nv_b;
	uint32_t nv_w;
	uint32_t reserved[3];
	uint16_t cal_c_ratio[CAL_STATE_GAIN_LAST];
	uint16_t cal_r_ratio[CAL_STATE_GAIN_LAST];
	uint16_t cal_g_ratio[CAL_STATE_GAIN_LAST];
	uint16_t cal_b_ratio[CAL_STATE_GAIN_LAST];
	uint16_t cal_w_ratio[CAL_STATE_GAIN_LAST];
	uint16_t reserved_1[7];
};

struct color_sensor_input_t {
	uint32_t tar_x;
	uint32_t tar_y;
	uint32_t tar_z;
	uint32_t tar_ir;
	uint32_t enable;
};

struct at_color_sensor_input_t {
	uint32_t enable;
	uint32_t reserverd[MAX_RAW_DATA_LEN];
};

struct color_sensor_output_t {
	uint32_t result;
	uint32_t report_x[CAL_STATE_GAIN_LAST];
	uint32_t report_y[CAL_STATE_GAIN_LAST];
	uint32_t report_z[CAL_STATE_GAIN_LAST];
	uint32_t report_ir[CAL_STATE_GAIN_LAST];
};

struct at_color_sensor_output_t {
	uint32_t result;
	uint32_t gain_arr;
	uint32_t color_arr;
	uint32_t cali_gain[CAL_STATE_GAIN_LAST];
	uint32_t cali_rst[MAX_RAW_DATA_LEN][CAL_STATE_GAIN_LAST];
};

struct color_sensor_cali_t {
	enum color_sensor_cal_states cal_state;
	uint32_t cal_x_raw;
	uint32_t cal_y_raw;
	uint32_t cal_z_raw;
	uint32_t cal_ir_raw;
	uint32_t cal_x_tar;
	uint32_t cal_y_tar;
	uint32_t cal_z_tar;
	uint32_t cal_ir_tar;
	uint32_t cal_x_rst[CAL_STATE_GAIN_LAST];
	uint32_t cal_y_rst[CAL_STATE_GAIN_LAST];
	uint32_t cal_z_rst[CAL_STATE_GAIN_LAST];
	uint32_t cal_ir_rst[CAL_STATE_GAIN_LAST];
	uint32_t cal_raw_count;
	uint32_t cal_c_raw;
	uint32_t cal_r_raw;
	uint32_t cal_g_raw;
	uint32_t cal_b_raw;
	uint32_t cal_w_raw;
	uint32_t cal_c_tar;
	uint32_t cal_r_tar;
	uint32_t cal_g_tar;
	uint32_t cal_b_tar;
	uint32_t cal_w_tar;
	uint32_t cal_c_rst[CAL_STATE_GAIN_LAST];
	uint32_t cal_r_rst[CAL_STATE_GAIN_LAST];
	uint32_t cal_g_rst[CAL_STATE_GAIN_LAST];
	uint32_t cal_b_rst[CAL_STATE_GAIN_LAST];
	uint32_t cal_w_rst[CAL_STATE_GAIN_LAST];
};

struct cali_gain_thr_t {
	uint32_t again;
	uint32_t low_thr;
	uint32_t high_thr;
};

// define a common struct for rgb cali para
struct rgb_cali_tar_t {
	uint32_t rawdata;
	uint32_t high_th;
	uint32_t low_th;
	uint32_t again;
	uint32_t atime_ms;
	uint32_t target_val;
};

enum report_type {
	// invalid report type
	AWB_SENSOR_RAW_SEQ_TYPE_INVALID = 0,

	// 16 raw data SEQ: r,g,b,ir,0,...,0
	AWB_SENSOR_RAW_SEQ_TYPE_R_G_B_IR = 1,

	// 16 raw data SEQ: c,r,g,b,w,0,...,0
	AWB_SENSOR_RAW_SEQ_TYPE_C_R_G_B_W = 2,

	AWB_SENSOR_RAW_SEQ_TYPE_COMMON = 9,
	AWB_SENSOR_RAW_SEQ_TYPE_MAX,
};

struct color_priv_data_t {
	unsigned int rgb_support;
	unsigned int rgb_absent;
};

struct color_chip {
	struct mutex lock;
	struct i2c_client *client;
	struct driver_i2c_platform_data *pdata;
	int in_suspend;
	int wake_irq;
	int irq_pending;
	bool unpowered;
	bool in_cal_mode;
	struct color_sensor_cali_t cali_ctx;
	u8 device_index;
	void *device_ctx;
	struct timer_list work_timer;
	struct timer_list fd_timer;
	struct work_struct als_work;
	struct work_struct fd_work;
	struct device *dev;
	void (*color_show_calibrate_state)(struct color_chip *,
		struct color_sensor_output_t *);
	void (*color_store_calibrate_state)(struct color_chip *,
		struct color_sensor_input_t *);
	void (*at_color_store_calibrate_state)(struct color_chip *,
		struct at_color_sensor_input_t *);
	void (*at_color_show_calibrate_state)(struct color_chip *,
		struct at_color_sensor_output_t *);
	void (*color_enable_show_state)(struct color_chip *, int *);
	void (*color_enable_store_state)(struct color_chip *, int);
	void (*flicker_enable_store_state)(struct color_chip *, int);
	void (*get_flicker_data)(struct color_chip *, char *);
	INT32 (*color_sensor_get_gain)(const void *);
	INT32 (*color_sensor_set_gain)(void *, int);
	void (*flicker_enable_show_state)(struct color_chip *, int *);
	int (*color_report_type)(void);
	char *(*color_chip_name)(void);
	char *(*color_algo_type)(void);
};

// define common func here for rgb sensor
void color_notify_absent(void);
void color_notify_support(void);
int ap_color_report(int value[], int length);
int color_register(struct color_chip *chip);
int color_sensor_get_byte(const struct i2c_client *i2c, uint8_t reg,
	uint8_t *data);
int color_sensor_set_byte(const struct i2c_client *i2c, uint8_t reg,
	uint8_t data);
int color_sensor_read_fifo(struct i2c_client *client, uint8_t reg,
	void *buf, size_t len);
int read_color_calibrate_data_from_nv(int nv_number, int nv_size,
	const char *nv_name, char *temp);
int write_color_calibrate_data_to_nv(int nv_number, int nv_size,
	const char *nv_name, const char *temp);

#endif /* __COLOR_SENSOR_H__ */
