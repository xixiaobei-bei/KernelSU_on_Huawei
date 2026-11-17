/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: hisi_eis_core_freq.h
 *
 * eis core freq driver header
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
#ifndef _BATT_EIS_CORE_H_
#define _BATT_EIS_CORE_H_

#include <linux/cdev.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/pm_wakeup.h>
#include <linux/poll.h>
#include <stdbool.h>
#include <uapi/linux/ioctl.h>
#include <linux/power/hisi/hisi_eis.h>
#include "pmic/hisi_pmic_eis.h"

#ifndef STATIC
#define STATIC static
#endif

struct collect_para {
	unsigned int (*ptr_v)[V_BAT_FIFO_DEPTH];
	unsigned int (*ptr_i)[I_BAT_FIFO_DEPTH];
	int key_prds;
	long long timeout_ms;
	struct completion *chg_resume_com;
};

enum eis_drv_ops_type {
	FREQ_DRV_OPS    = 0,
	EIS_OPS         = 3
};

enum eis_mode {
	EIS_FREQ = 0,
};

/* eis print interface */
#define hisi_eis_err(fmt, args...) \
	pr_err("[eis_err][%s-line%d]:" fmt, __func__, __LINE__, ##args)
#define hisi_eis_inf(fmt, args...) \
	pr_info("[eis_inf][%s-line%d]:" fmt, __func__, __LINE__, ##args)
#define hisi_eis_debug(fmt, args...) \
	pr_debug("[eis_debug][%s-line%d]:" fmt, __func__, __LINE__, ##args)

#define check_eis_freq_ops(_di, _name) do { \
	if ((_di)->eis_freq_dev.freq_ops->_name == NULL) \
		return _ERROR; \
	} while (0)

#define check_eis_ops(_di, _name) do { \
	if ((_di)->eis_ops->_name == NULL) \
		return _ERROR; \
	} while (0)

#define EIS_MAX_INT             ((~0U) / 2)
#define EIS_MAX_UINT            (~0U)
#define EIS_MAX_U32             ((~(u32)0) / 2)
#define EIS_MAX_U8              ((~(u8)0) / 2)
#define EIS_FREQ_ENTER_CUR      20 /* mA */
#define EIS_FREQ_TEMP_LOW       10 /* Celsius degree */
#define EIS_FREQ_TEMP_HIGH      45 /* Celsius degree */
#define EIS_FREQ_SOC            99 /* percent */

#define CHGER_DISABLE           0
#define CHGER_RESM_TYPE_LVC     5
#define CHGER_RESM_TYPE_SC      6
#define CHGER_RESM_TYPE_BUCK    1
#define CHGER_RESM_TYPE_ALL     12
#define CHGER_RESM_SYS_TYPE     0
#define CHGER_RESM_ENABLE       1

#define F_SAMP_NULL_PRDS        4
#define FREQ_SAMP_PRDS_10       10
#define FREQ_SAMP_PRDS_5        5
#define CHGER_DIS_PRDS_GAP      1
#define EIS_IRQ_START           0
#define EIS_IRQ_END             1
#define EIS_FIFO_IRQ_4          2
#define EIS_FIFO_IRQ_8          3
#define EIS_FIFO_IRQ_12         4
#define EIS_FIFO_IRQ_16         5
#define FREQ_RESAMPLING_MS      200
#define FREQ_EIS_AVGED_LEN      5
#define EIS_IS_WORKING          0
#define EIS_NOT_WORKING         1
#define EIS_T_FLAG_ON           1
#define EIS_T_FLAG_OFF          0
#define EIS_W_FLAG_ON           1
#define EIS_W_FLAG_OFF          0
#define EIS_TIME_ALG_NUM        5
#define CAL_ERR                 (-1)
#define CUR_OVERFLOW_FLAG       1
#define EARLY_STAGE_PERIODS     125
#define TIMEOUT_UNIT_MS         4
#define SIZE_OF_RAW_EIS         (sizeof(long long) * FREQ_SAMP_PRDS_10)
#define TIMEOUT_PARA_NUM        3
#define T_N_M_PARA_LEN          3
#define WAIT_EIS_END            600000
#define SIZEOF_VBAT_SAMPLE \
	(sizeof(unsigned int) * FREQ_SAMP_PRDS_10 * V_BAT_FIFO_DEPTH)
#define SIZEOF_IBAT_SAMPLE \
	(sizeof(unsigned int) * FREQ_SAMP_PRDS_10 * I_BAT_FIFO_DEPTH)
#define S_TO_MS                 1000
#define MS_TO_US                1000
#define T_W_COLLECT_FLAG        0
#define F_COLLECT_FLAG          4

struct eis_freq_sample_info {
	int v_gain;
	int i_gain;
	unsigned int sample_freq;
	unsigned int sample_prds;
	unsigned int v_bat[FREQ_SAMP_PRDS_10][V_BAT_FIFO_DEPTH];
	unsigned int i_bat[FREQ_SAMP_PRDS_10][I_BAT_FIFO_DEPTH];
	long long raw_eis_r[FREQ_SAMP_PRDS_10];
	long long raw_eis_i[FREQ_SAMP_PRDS_10];
	unsigned int valid_sample_cnt;
	bool valid_prd_idx[FREQ_SAMP_PRDS_10];
};

struct eis_freq_device_ops {
	int (*locate_in_ary)(const unsigned int *array, int len,
		unsigned int val);
	int (*get_v_bat_gain)(void);
	int (*get_v_digit_data)(const int *raw_vol_data, int len_raw,
		int *pure_vol_data, int len_pure);
	int (*chk_adjust_freq_i_gain)(int *i_gain,
		int periods, int (*raw_sample_cur_data)[I_BAT_FIFO_DEPTH]);
	int (*chk_adjust_freq_v_gain)(int *v_gain, int periods,
		int (*raw_sample_vol_data)[V_BAT_FIFO_DEPTH]);
	void (*set_v_gain)(unsigned int v_gain);
	void (*set_i_gain)(unsigned int i_gain);
	void (*set_cur_thres)(int curr_offset);
	void (*eis_sampling_init)(void);
};

struct eis_freq_device {
	struct notifier_block eis_freq_notify;
	struct eis_freq_info freq_info[LEN_T_FREQ];
	struct eis_freq_sample_info freq_sample_info[LEN_T_FREQ];
	struct eis_freq_device_ops *freq_ops;
	struct delayed_work freq_cal_work;
	struct mutex eis_freq_mutex;
	struct device *dev;
	struct device_node *np;
	int freq_support;
	int freq_work_flag;
};

struct eis_device_ops {
	unsigned int (*get_fifo_period_now)(void);
	unsigned int (*get_fifo_order_now)(void);
	unsigned int (*get_v_fifo_data)(unsigned int fifo_order);
	unsigned int (*get_i_fifo_data)(unsigned int fifo_order);
	int (*read_and_clear_irq)(unsigned int irq_num,
		enum eis_irq_ops irq_ops);
	void (*irq_seq_clear)(unsigned int start, unsigned int end);
	int (*irq_seq_read)(int start, int end);
	int (*eis_detect_flag)(void);
	void (*set_t_n_m)(unsigned int t, unsigned int n, unsigned int m);
	void (*get_t_n_m)(unsigned int *t, unsigned int *n, unsigned int *m);
	void (*eis_enable)(bool enable);
	int (*cnt_overflow_num)(const int *sample_vol_data, int len);
	int (*cur_offset)(void);
};

struct hisi_eis_device {
	struct eis_freq_device eis_freq_dev;
	struct device *dev;
	struct device_node *np;
	struct eis_device_ops *eis_ops;
	int charger_dis_prds_diff;
	struct mutex screen_update_lock;
	struct notifier_block screen_status_nb;
	struct completion freq_sampling_com;
	struct delayed_work charger_disable_work;
	spinlock_t sample_lock;
	struct mutex eis_mutex;
	struct wakeup_source eis_wake_lock;
};

int eis_core_drv_ops_register(void *ops, enum eis_drv_ops_type ops_type);
struct hisi_eis_device *get_g_eis_di(void);
#endif
