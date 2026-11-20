/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2020. All rights reserved.
 * Description: polar parameter for coul module
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
#ifndef _HISI_POLAR_TABLE_H_
#define _HISI_POLAR_TABLE_H_

#include <stdbool.h>
#include <linux/mutex.h>
#include <linux/notifier.h>
#include <linux/platform_device.h>
#include <linux/hisi/rdr_hisi_ap_ringbuffer.h>

#define UVOLT_PER_MVOLT               1000
#define UA_PER_MA                     1000
#define UOHM_PER_MOHM                 1000
#define COUL_FIFO_SAMPLE_INTERVAL     110
#define COUL_POLAR_SAMPLE_TIME        (1200 * 1000UL)
#define COUL_FIFO_SAMPLE_TIME         (35 * 1000UL)
#define POLAR_TIME_ARRAY_NUM          16
#define POLAR_CURR_ARRAY_NUM          8
#define POLAR_CURR_ARRAY_VECTOR_NUM   4
#define POLAR_TIME_ARRAY_0S           0
#define POLAR_TIME_ARRAY_5S           3
#define POLAR_TIME_ARRAY_25S          6
#define POLAR_TIME_0S                 500
#define POLAR_TIME_5S                 5000
#define POLAR_TIME_25S                25000
#define POLAR_VECTOR_SIZE             17
#define POLAR_VECTOR_5S               4
#define POLAR_OCV_PC_TEMP_ROWS        41
#define POLAR_OCV_PC_TEMP_COLS        9
#define POLAR_RES_PC_CURR_ROWS        20
#define POLAR_MIN_VOL_ESTIMATE_UV     10000
#define POLAR_OCV_CURRENT_LIMIT_UA    200000
#define POLAR_ERR_COE_MUL             1000
#define POLAR_CURR_PREDICT_MSECS      5000
#define POLAR_RES_MHOM_MUL            10
#define POLAR_RATIO_PERCENTAGE        100
#define POLAR_ERR_A_DEFAULT           1000
#define POLAR_ERR_A_MIN               900
#define POLAR_ERR_A_MAX               3000
#define POLAR_ERR_B_MIN               (-50000)
#define POLAR_ERR_B_MAX               50000
#define VPERT_PAST_LOW_B              (-10000)
#define VPERT_PAST_HIGH_B             10000
#define VPERT_NOW_LOW_B               (-5000)
#define VPERT_NOW_HIGH_B              5000
#define VPERT_NOW_LOW_A               (-20000)
#define VPERT_NOW_HIGH_A              20000
#define POLAR_A_COE_MUL               100
#define POLAR_INT_COE                 100
#define POLAR_VALID_A_NUM             3
#define POLAR_ARRAY_NUM               2
#define POLAR_VOL_INVALID             0x5a5a5a5a
#define POLAR_BUFFER_SIZE             (1024 * 1024)
#define FIFO_BUFFER_SIZE              (20 * 1024)
#define two_avg(a, b)                 (((a) + (b)) / 2)
/* ma = ua/1000 = uas/s/1000 = uah*3600/s/1000 = uah*18/(s*5) */
#define cc_uas2ma(cc, time)           (((cc) * 18) / ((time) * 5))
#define POLAR_SOC_STEP                25
#define OCV_STEP_START                (-4)
#define OCV_STEP_END                  4
#define POLAR_SOC_OCV_UPDATE          100
#define POLAR_CURR_OCV_UPDATE         200
#define POLAR_TEMP_OCV_UPDATE         10
#define COUL_DEFAULT_SAMPLE_INTERVAL  250
#define VBAT_LEARN_GAP_MV             100
#define VBAT_LEARN_COE_HIGH           7
#define VBAT_LEARN_COE_LOW            4
#define VBAT_LEARN_AVGCURR_HIGH       (-1000)
#define VBAT_LEARN_AVGCURR_LOW        (-500)
#define VBAT_LEARN_NOLIMIT_MV         150
#define POLAR_LEARN_TEMP_COLS         48
#define POLAR_LEARN_TEMP_RANGE        4
#define TENTH                         10
#define TEMP_25_DEGREE                25
#define SELF_LEARN_GAP_LOW            4
#define SELF_LEARN_GAP_NORMAL         1800
#define BATTCELL_NAME_SIZE_MAX        25
#define MAX_PREDICT_CYCLE             3
#define DEFAULT_ISHORT_TIME_INTERVAL  60
#define DEFAULT_DISCHG_INTERVAL       10
#define DEFAULT_ISHORT_CURRENT_MA     50

#ifndef TRUE
#define TRUE                          1
#endif
#ifndef FALSE
#define FALSE                         0
#endif

struct info_to_update_polar_info {
	int temp;
	int soc;
	int sr_sleep_time;
	int sleep_cc;
	unsigned int suspend_time;
	int suspend_cc;
	int r_pcb;
	int charging_state;
	int charging_stop_soc;
	int batt_soc_real;
	int qmax;
};

struct polar_ishort_info {
	int enable;
	int reported;
	int report_urgent;
	int time_interval;
	int dischg_interval;
	int report_curr_ma;
	int has_chg_done;
	int curr_ua;
	int ocv0;
	s64 cc0;
	int time0;
	int last_time;
	int last_chg_cycle;
	int last_curr_ua;
	s64 cc_all;
};

struct polar_device_ops {
	int (*convert_regval2uah)(u64 reg_val);
	unsigned int (*get_coul_time)(void);
	int (*calculate_cc_uah)(void);
	void (*get_eco_sample_flag)(u8 *get_val);
	void (*set_eco_sample_flag)(u8 set_val);
	void (*clr_eco_data)(u8 set_val);
};

struct bat_ocv_info  {
	unsigned int eco_vbat_reg;
	unsigned int eco_ibat_reg;
	int last_sample_time;
	int now_sample_time;
	s64 eco_cc_reg;
};

struct polar_calc_info {
	long vol;
	int curr_5s;
	int curr_peak;
	int ocv;
	int ocv_old;
	int ori_vol;
	int ori_cur;
	int err_a;
	int sr_polar_vol1;
	int sr_polar_vol0;
	int sr_polar_err_a;
	int polar_ocv;
	int polar_ocv_time;
	int last_calc_temp;
};

struct ploarized_info {
	unsigned long sample_time;
	unsigned long duration;
	int current_ma;
	int temperature;
	int soc_now;
	struct list_head list;
};

struct hisi_polar_device {
	/* 20min polar info buffer */
	struct hisiap_ringbuffer_s *polar_buffer;
	/* 35s fifo info buffer */
	struct hisiap_ringbuffer_s *fifo_buffer;
	struct device *dev;
	struct device_node *np;
	struct hrtimer coul_sample_timer;
	spinlock_t coul_fifo_lock;
	struct ploarized_info polar_head;
	struct ploarized_info coul_fifo_head;
	struct mutex polar_vol_lock;
	long polar_vol_array[POLAR_ARRAY_NUM];
	int polar_vol_index;
	int last_max_avg_curr;
	int last_max_peak_curr;
	int polar_res_future;
	int last_avgcurr_0s;
	int last_avgcurr_5s;
	int last_avgcurr_25s;
	u32 fifo_interval;
	u32 fifo_depth;
	u32 r_pcb;
	u32 v_cutoff;
	char batt_cell_name[BATTCELL_NAME_SIZE_MAX];
	struct polar_ishort_info ishort_info;
	struct polar_calc_info polar;
	struct bat_ocv_info eco_info;
	struct notifier_block bat_lpm3_ipc_block;
	struct polar_device_ops *polar_dev_ops;
};

struct polar_curr_info {
	s64 current_avg;
	long duration;
	long cnt;
	int soc_avg;
};

struct polar_ocv_tbl {
	int rows;
	int cols;
	int *percent;
	int *temp;
	int ocv[POLAR_OCV_PC_TEMP_ROWS][POLAR_OCV_PC_TEMP_COLS];
};

struct polar_res_tbl {
	int rows;
	int cols;
	int z_lens;
	int *x_array;
	int *y_array;
	int *z_array;
	int value[POLAR_OCV_PC_TEMP_COLS][POLAR_RES_PC_CURR_ROWS][
		POLAR_CURR_ARRAY_NUM];
};

struct polar_x_y_z_tbl {
	int rows;
	int cols;
	int z_lens;
	int *x_array;
	int *y_array;
	int *z_array;
	int value[POLAR_OCV_PC_TEMP_COLS][POLAR_RES_PC_CURR_ROWS][
		POLAR_CURR_ARRAY_VECTOR_NUM][POLAR_VECTOR_SIZE];
};

struct polar_learn {
	short a_trained;
	short polar_vol_mv;
};

struct polar_learn_tbl {
	int rows;
	int cols;
	int *x_array;
	int *y_array;
	struct polar_learn value[POLAR_RES_PC_CURR_ROWS][POLAR_LEARN_TEMP_COLS];
};
#endif
