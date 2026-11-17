/*
 * direct_charge_ic.h
 *
 * direct charge ic module
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#ifndef _DIRECT_CHARGE_IC_H_
#define _DIRECT_CHARGE_IC_H_

#define CHARGE_PATH_MAX_NUM        2
#define CHARGE_IC_MAIN             BIT(0)
#define CHARGE_IC_AUX              BIT(1)
#define CHARGE_MULTI_IC            (CHARGE_IC_MAIN | CHARGE_IC_AUX)

enum dc_ic_type {
	CHARGE_IC_TYPE_MAIN = 0,
	CHARGE_IC_TYPE_AUX,
	CHARGE_IC_TYPE_MAX,
};

#define IC_MODE_MAX_NUM            2
enum dc_working_mode {
	UNDEFINED_MODE = 0x0,
	LVC_MODE = 0x1,
	SC_MODE = 0x2,
	SC_4_1_MODE = 0x4,
	TOTAL_MODE,
};

#define CHARGE_IC_MAX_NUM          2
enum dc_ic_index {
	IC_ONE = 0,
	IC_TWO,
	IC_TOTAL,
};

enum dc_ic_info {
	DC_IC_INFO_IC_INDEX,
	DC_IC_INFO_PATH_INDEX,
	DC_IC_INFO_MAX_IBAT,
	DC_IC_INFO_IBAT_POINT,
	DC_IC_INFO_VBAT_POINT,
	DC_IC_INFO_TOTAL,
};

#define PARA_NAME_LEN_MAX         32
enum dc_ic_mode_info {
	DC_IC_MODE_INDEX = 0,
	DC_IC_MODE_IC_PARA_NAME,
	DC_IC_MODE_TOTAL,
};

struct dc_ic_para {
	unsigned int ic_index;
	unsigned int path_index;
	unsigned int max_ibat;
	unsigned int ibat_sample_point;
	unsigned int vbat_sample_point;
};

struct dc_ic_mode_para {
	char ic_mode[PARA_NAME_LEN_MAX];
	char ic_para_index[PARA_NAME_LEN_MAX];
	struct dc_ic_para ic_para[CHARGE_IC_MAX_NUM];
	int ic_para_size;
};

struct dc_ic_dev {
	struct device *dev;
	struct dc_ic_mode_para mode_para[IC_MODE_MAX_NUM];
	int mode_num;
	int use_coul_vbat;
	bool para_flag;
};

#define SENSE_R_1_MOHM          10 /* 1 mohm */
#define SENSE_R_2_MOHM          20 /* 2 mohm */
#define SENSE_R_2P5_MOHM        25 /* 2.5 mohm */
#define SENSE_R_5_MOHM          50 /* 5 mohm */
#define SENSE_R_10_MOHM         100 /* 10 mohm */

#define IBUS_DEGLITCH_5MS       5
#define UCP_RISE_500MA          500

struct dc_ic_ops {
	void *dev_data;
	const char *dev_name;
	int (*ic_init)(void *);
	int (*ic_exit)(void *);
	int (*ic_enable)(int, void *);
	int (*ic_adc_enable)(int, void *);
	int (*ic_discharge)(int, void *);
	int (*is_ic_close)(void *);
	int (*ic_enable_prepare)(void *);
	int (*config_ic_watchdog)(int, void *);
	int (*kick_ic_watchdog)(void *);
	int (*get_ic_id)(void *);
	int (*get_ic_status)(void *);
	int (*set_ic_buck_enable)(int, void *);
	int (*ic_reg_reset_and_init)(void *);
	int (*set_ic_freq)(int, void *);
	int (*get_ic_freq)(void *);
};

struct dc_batinfo_ops {
	void *dev_data;
	int (*init)(void *);
	int (*exit)(void *);
	int (*get_bat_btb_voltage)(void *);
	int (*get_bat_package_voltage)(void *);
	int (*get_vbus_voltage)(int *, void *);
	int (*get_bat_current)(int *, void *);
	int (*get_ic_ibus)(int *, void *);
	int (*get_ic_temp)(int *, void *);
	int (*get_ic_vout)(int *, void *);
	int (*get_ic_vusb)(int *, void *);
};

#ifdef CONFIG_DIRECT_CHARGER
int dc_ic_ops_register(int mode, unsigned int index, struct dc_ic_ops *ops);
int dc_batinfo_ops_register(int mode, unsigned int index, struct dc_batinfo_ops *ops);
struct dc_ic_ops *dc_ic_get_ic_ops(int mode, unsigned int index);
struct dc_batinfo_ops *dc_ic_get_battinfo_ops(int mode, unsigned int index);
int dc_ic_get_ic_index(int mode, unsigned int path, unsigned int *index, int len);
int dc_ic_get_ic_index_for_ibat(int mode, unsigned int path, unsigned int *index, int len);
bool dc_ic_get_vbat_from_coul(int *vbat);
#else
static inline int dc_ic_ops_register(int mode, unsigned int index, struct dc_ic_ops *ops)
{
	return -1;
}

static inline int dc_ic_batinfo_ops_register(int mode, unsigned int index, struct dc_batinfo_ops *ops)
{
	return -1;
}

static inline struct dc_ic_ops *dc_ic_get_ic_ops(int mode, unsigned int index)
{
	return NULL;
}

static inline struct dc_batinfo_ops *dc_ic_get_battinfo_ops(int mode, unsigned int index)
{
	return NULL;
}

static inline int dc_ic_get_ic_index(int mode, unsigned int path, unsigned int *index, int len)
{
	return -1;
}

static inline int dc_ic_get_ic_index_for_ibat(int mode, unsigned int path, unsigned int *index, int len)
{
	return -1;
}

static inline bool dc_ic_get_vbat_from_coul(int *vbat)
{
	return false;
}
#endif /* CONFIG_DIRECT_CHARGER */

#endif /* _DIRECT_CHARGE_IC_H_ */
