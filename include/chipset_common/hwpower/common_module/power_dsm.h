/*
 * power_dsm.h
 *
 * dsm for power module
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#ifndef _POWER_DSM_H_
#define _POWER_DSM_H_

#include <dsm/dsm_pub.h>

/* define dsm buffer size */
#define POWER_DSM_BUF_SIZE_0016       16
#define POWER_DSM_BUF_SIZE_0128       128
#define POWER_DSM_BUF_SIZE_0256       256
#define POWER_DSM_BUF_SIZE_0512       512
#define POWER_DSM_BUF_SIZE_1024       1024
#define POWER_DSM_BUF_SIZE_2048       2048
#define POWER_DSM_RESERVED_SIZE       16

enum power_dsm_type {
	POWER_DSM_TYPE_BEGIN = 0,
	POWER_DSM_CPU_BUCK = POWER_DSM_TYPE_BEGIN,
	POWER_DSM_USB,
	POWER_DSM_BATTERY_DETECT,
	POWER_DSM_BATTERY,
	POWER_DSM_CHARGE_MONITOR,
	POWER_DSM_SUPERSWITCH,
	POWER_DSM_SMPL,
	POWER_DSM_PD_RICHTEK,
	POWER_DSM_PD,
	POWER_DSM_USCP,
	POWER_DSM_PMU_OCP,
	POWER_DSM_PMU_IRQ,
	POWER_DSM_VIBRATOR_IRQ,
	POWER_DSM_DIRECT_CHARGE_SC,
	POWER_DSM_FCP_CHARGE,
	POWER_DSM_MTK_SWITCH_CHARGE2,
	POWER_DSM_LIGHTSTRAP,
	POWER_DSM_TYPE_END,
};

struct power_dsm_client {
	unsigned int type;
	const char *name;
	struct dsm_client *client;
	struct dsm_dev *dev;
};

struct power_dsm_dump {
	unsigned int type;
	int error_no;
	bool dump_enable;
	bool dump_always;
	const char *pre_buf;
	bool (*support)(void);
	void (*dump)(char *, unsigned int);
	bool (*check_error)(char *, unsigned int);
};

#ifdef CONFIG_HUAWEI_DSM
struct dsm_client *power_dsm_get_dclient(unsigned int type);
int power_dsm_report_dmd(unsigned int type, int err_no, const char *buf);

#ifdef CONFIG_HUAWEI_DATA_ACQUISITION
int power_dsm_report_fmd(unsigned int type, int err_no,
	const void *msg);
#else
static inline int power_dsm_report_fmd(unsigned int type,
	int err_no, const void *msg)
{
	return 0;
}
#endif /* CONFIG_HUAWEI_DATA_ACQUISITION */

#define power_dsm_report_format_dmd(type, err_no, fmt, args...) do { \
	if (power_dsm_get_dclient(type)) { \
		if (!dsm_client_ocuppy(power_dsm_get_dclient(type))) { \
			dsm_client_record(power_dsm_get_dclient(type), fmt, ##args); \
			dsm_client_notify(power_dsm_get_dclient(type), err_no); \
			pr_info("report type:%d, err_no:%d\n", type, err_no); \
		} else { \
			pr_err("power dsm client is busy\n"); \
		} \
	} \
} while (0)

void power_dsm_reset_dump_enable(struct power_dsm_dump *data, unsigned int size);
void power_dsm_dump_data(struct power_dsm_dump *data, unsigned int size);
void power_dsm_dump_data_with_error_no(struct power_dsm_dump *data,
	unsigned int size, int err_no);
#else
static inline struct dsm_client *power_dsm_get_dclient(unsigned int type)
{
	return NULL;
}

static inline int power_dsm_report_dmd(unsigned int type,
	int err_no, const char *buf)
{
	return 0;
}

static inline int power_dsm_report_fmd(unsigned int type,
	int err_no, const void *msg)
{
	return 0;
}

#define power_dsm_report_format_dmd(type, err_no, fmt, args...)

static inline void power_dsm_reset_dump_enable(
	struct power_dsm_dump *data, unsigned int size)
{
}

static inline void power_dsm_dump_data(
	struct power_dsm_dump *data, unsigned int size)
{
}

static inline void power_dsm_dump_data_with_error_no(
	struct power_dsm_dump *data, unsigned int size, int err_no)
{
}
#endif /* CONFIG_HUAWEI_DSM */

#endif /* _POWER_DSM_H_ */
