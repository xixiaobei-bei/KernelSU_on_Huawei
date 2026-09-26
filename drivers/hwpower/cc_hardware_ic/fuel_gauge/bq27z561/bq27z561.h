/*
 * bq27z561.h
 *
 * coul with bq27z561 driver
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

#ifndef _BQ27Z561_H_
#define _BQ27Z561_H_

#include <linux/module.h>
#include <linux/param.h>
#include <linux/jiffies.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/idr.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <asm/unaligned.h>
#include <linux/of.h>
#include <linux/gpio/consumer.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_algorithm.h>
#include <chipset_common/hwpower/common_module/power_log.h>
#include <chipset_common/hwpower/coul/coul_interface.h>
#include <chipset_common/hwpower/battery/battery_model_public.h>
#include <chipset_common/hwpower/coul/coul_calibration.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <chipset_common/hwpower/common_module/power_dts.h>

#define FG_ERR_PARA_NULL                 1
#define FG_ERR_PARA_WRONG                2
#define FG_ERR_MISMATCH                  3
#define FG_ERR_NO_SPACE                  4
#define FG_ERR_I2C_R                     5
#define FG_ERR_I2C_W                     6
#define FG_ERR_I2C_WR                    7
#define INVALID_REG_ADDR                 0xFF
#define FG_ROM_MODE_I2C_ADDR_CFG         0x16
#define FG_ROM_MODE_I2C_ADDR             0x0B
#define FG_FLASH_MODE_I2C_ADDR           0x55
#define FG_FLAGS_FD                      BIT(4)
#define FG_FLAGS_FC                      BIT(5)
#define FG_FLAGS_DSG                     BIT(6)
#define FG_FLAGS_RCA                     BIT(9)
#define FG_IO_CONFIG_VAL                 0x24
#define FG_MAC_WR_MAX                    32
#define FG_MAX_BUFFER_LEN                128
#define FG_MAC_ADDR_LEN                  2
#define FG_MAC_TRAN_BUFF                 ((FG_MAC_WR_MAX) + 8)
#define FG_PARA_INVALID_VER              0xFF
#define FG_FULL_CAPACITY                 100
#define FG_CAPACITY_TH                   7
#define FG_SW_MODE_DELAY_TIME            4000
#define FG_WRITE_MAC_DELAY_TIME          100

#define I2C_MAX_TRAN                     FG_MAC_TRAN_BUFF

#define BYTE_LEN                         1
#define MSG_LEN                          2
#define DWORD_LEN                        4
#define MAC_DATA_LEN                     11
#define I2C_RETRY_CNT                    3
#define ABNORMAL_BATT_TEMPERATURE_LOW    (-400)
#define ABNORMAL_BATT_TEMPERATURE_HIGH   800
#define CMD_MAX_DATA_SIZE                32
#define BQ27Z561_WORD_LEN                2
#define I2C_BLK_SIZE                     32
#define I2C_BLK_SIZE_HALF                16
#define BQ27Z561_DELAY_TIME              5
#define BQ27Z561_DELAY_CALI_TIME         0xE8

#define BQ27Z561_SOC_DELTA_TH_REG        0x6f
#define BQ27Z561_SOC_DELTA_TH_VAL        100
#define BQ27Z561_VOLT_LOW_SET_REG        0x66
#define BQ27Z561_VOLT_LOW_SET_VAL        2600
#define BQ27Z561_VOLT_OVER_VAL           5000
#define BQ27Z561_VOLT_LOW_CLR_REG        0x68
#define BQ27Z561_VOLT_LOW_CLR_OFFSET     100
#define BQ27Z561_VOLT_LOW_CLR_VAL        ((BQ27Z561_VOLT_LOW_SET_VAL) + 100)
#define BQ27Z561_INT_STATUS_REG          0x6E
#define BQ27Z561_CALI_EN_DISABLE_MASK    0x8000
#define FLOAT_EXPONENTIAL_DEFAULT        0x7f
#define FLOAT_EXPONENTIAL_SIGN_BIT       0x80
#define FLOAT_EXPONENTIAL_REDUCE_VALUE   2
#define FLOAT_BIT_NUMS                   64
#define FLOAT_FRACTION_SIZE              23
#define FLOAT_POINT_BIT                  2
#define NTC_PARA_LEVEL                   10

enum ntc_temp_compensation_para_info {
	NTC_PARA_ICHG = 0,
	NTC_PARA_VALUE,
	NTC_PARA_TOTAL,
};

enum bq27z561_reg_idx {
	BQ_FG_REG_CTRL = 0,
	BQ_FG_REG_TEMP, /* Battery Temperature */
	BQ_FG_REG_VOLT, /* Battery Voltage */
	BQ_FG_REG_I, /* Battery Current */
	BQ_FG_REG_AI, /* Average Current */
	BQ_FG_REG_BATT_STATUS, /* BatteryStatus */
	BQ_FG_REG_TTE, /* Time to Empty */
	BQ_FG_REG_TTF, /* Time to Full */
	BQ_FG_REG_FCC, /* Full Charge Capacity */
	BQ_FG_REG_RM, /* Remaining Capacity */
	BQ_FG_REG_CC, /* Cycle Count */
	BQ_FG_REG_SOC, /* Relative State of Charge */
	BQ_FG_REG_SOH, /* State of Health */
	BQ_FG_REG_DC, /* Design Capacity */
	BQ_FG_REG_ALT_MAC, /* AltManufactureAccess */
	BQ_FG_REG_MAC_CHKSUM, /* MACChecksum */
	NUM_REGS,
};

enum bq27z561_mac_cmd {
	FG_MAC_CMD_CTRL_STATUS = 0x0000,
	FG_MAC_CMD_DEV_TYPE = 0x0001,
	FG_MAC_CMD_FW_VER = 0x0002,
	FG_MAC_CMD_HW_VER = 0x0003,
	FG_MAC_CMD_IF_SIG = 0x0004,
	FG_MAC_CMD_DF_SIG = 0x0005,
	FG_MAC_CMD_CHEM_ID = 0x0006,
	FG_MAC_CMD_GAUGING = 0x0021,
	FG_MAC_CMD_CAL_EN = 0x002D,
	FG_MAC_CMD_SEAL = 0x0030,
	FG_MAC_CMD_DEV_RESET = 0x0041,
	FG_MAC_CMD_OP_STATUS = 0x0054,
	FG_MAC_CMD_CHARGING_STATUS = 0x0055,
	FG_MAC_CMD_GAUGING_STATUS = 0x0056,
	FG_MAC_CMD_MANU_STATUS = 0x0057,
	FG_MAC_CMD_MANU_INFO = 0x0070,
	FG_MAC_CMD_IT_STATUS1 = 0x0073,
	FG_MAC_CMD_IT_STATUS2 = 0x0074,
	FG_MAC_CMD_IT_STATUS3 = 0x0075,
	FG_MAC_CMD_SOC_CFG = 0x007A,
	FG_MAC_CMD_CELL_GAIN = 0x4000,
	FG_MAC_CMD_CC_GAIN = 0x4006,
	FG_MAC_CMD_CAP_GAIN = 0x400A,
	FG_MAC_CMD_MANU_INFO_DIR = 0x4041,
	FG_MAC_CMD_RA_TABLE = 0x40C0,
	FG_MAC_CMD_QMAX_CELL = 0x4146,
	FG_MAC_CMD_UPDATE_STATUS = 0x414A,
	FG_MAC_CMD_IO_CONFIG = 0x4484,
	FG_MAC_CMD_RAW_ADC = 0xF081,
};

enum {
	BQ27Z561_CALI_VOL = 0,
	BQ27Z561_CALI_CUR,
};

enum bq_fg_device {
	BQ27Z561,
};

enum {
	CMD_INVALID = 0,
	CMD_R, /* Read */
	CMD_W, /* Write */
	CMD_C, /* Compare */
	CMD_X, /* Delay */
};

/* single precision float */
struct spf {
	u32 fraction : 23; /* Data Bit */
	u32 exponent : 8; /* Exponential Bit */
	u32 sign : 1; /* Sign Bit */
};

/* float element position */
struct fep {
	int dec_point_pos;
	int first_one_pos;
};

static const unsigned char *device2str[] = {
	"bq27z561",
};

static u8 bq27z561_regs[NUM_REGS] = {
	0x00, /* Control */
	0x06, /* Temp */
	0x08, /* Volt */
	0x0C, /* Current */
	0x14, /* Avg current */
	0x0A, /* Flags */
	0x16, /* Time To Empty */
	0x18, /* Time To Full */
	0x12, /* Full Charge Capacity */
	0x10, /* Remaining Capacity */
	0x2A, /* CycleCount */
	0x2C, /* State Of Charge */
	0x2E, /* State Of Health */
	0x3C, /* Design Capacity */
	0x3E, /* AltManufacturerAccess */
	0x60, /* MACChecksum */
};

struct bq27z561_display_data {
	int temp;
	int vbat;
	int ibat;
	int avg_ibat;
	int rm;
	int soc;
	int fcc;
	int qmax;
};

struct fg_batt_profile {
	const unsigned char *bqfs_image;
	u16 array_size;
	u8 version;
};

struct bqfs_cmd_info {
	u8 cmd_type;
	u8 addr;
	u8 reg;
	union {
		u8 bytes[CMD_MAX_DATA_SIZE + 1];
		u16 delay;
	} data;
	u8 data_len;
	u8 line_num;
};

struct bq27z561_device_info {
	struct device *dev;
	struct i2c_client *client;
	struct delayed_work batt_para_check_work;
	struct delayed_work batt_para_monitor_work;
	struct wakeup_source wakelock;
	struct mutex rd_mutex;
	struct mutex mac_mutex;
	atomic_t pm_suspend;
	atomic_t is_update;
	u32 gpio;
	int irq;
	u8 chip;
	int batt_version;
	u8 regs[NUM_REGS];
	u8 *bqfs_image_data;
	int fg_para_version;
	int count;
	bool batt_fc;
	bool batt_fd;
	bool batt_dsg;
	bool batt_rca; /* Remaining Capacity Alarm */
	bool calib_enable;
	int seal_state; /* 0 - Full Access, 1 - Unsealed, 2 - Sealed */
	int batt_tte;
	int batt_soc;
	int last_batt_soc;
	int batt_fcc; /* Full Charge Capacity */
	int batt_rm; /* Remaining Capacity */
	int batt_dc; /* Design Capacity */
	int batt_volt;
	int batt_temp;
	int batt_curr;
	int batt_status;
	int batt_qmax;
	int control_status;
	int op_status;
	int charing_status;
	int gauging_status;
	int manu_status;
	int batt_curr_avg;
	int batt_cyclecnt;
	int batt_mode;
	int cur_gain;
	int vol_gain;
	int ntc_compensation_is;
	struct compensation_para ntc_temp_compensation_para[NTC_PARA_LEVEL];
};

#endif /* _BQ27Z561_H_ */
