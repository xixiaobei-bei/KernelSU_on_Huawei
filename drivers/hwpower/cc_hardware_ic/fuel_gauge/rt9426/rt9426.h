/*
 * rt9426.h
 *
 * driver for rt9426 battery fuel gauge
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

#ifndef _RT9426_H_
#define _RT9426_H_

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/power_supply.h>
#include <linux/delay.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <chipset_common/hwpower/common_module/power_algorithm.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_log.h>
#include <chipset_common/hwpower/coul/coul_interface.h>
#include <chipset_common/hwpower/battery/battery_model_public.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/coul/coul_calibration.h>
#include <chipset_common/hwpower/common_module/power_dts.h>

#define RT9426_DRIVER_VER               0x0007

#define RT9426_UNSEAL_KEY               0x12345678

#define RT9426_REG_CNTL                 0x00
#define RT9426_REG_RSVD_FLAG            0x02
#define RT9426_REG_CURR                 0x04
#define RT9426_REG_TEMP                 0x06
#define RT9426_REG_VBAT                 0x08
#define RT9426_REG_FLAG1                0x0A
#define RT9426_REG_FLAG2                0x0C
#define RT9426_REG_RM                   0x10
#define RT9426_REG_FCC                  0x12
#define RT9426_REG_AI                   0x14
#define RT9426_REG_TTE                  0x16
#define RT9426_REG_DUMMY                0x1E
#define RT9426_REG_VER                  0x20
#define RT9426_REG_VGCOMP12             0x24
#define RT9426_REG_VGCOMP34             0x26
#define RT9426_REG_INTT                 0x28
#define RT9426_REG_CYC                  0x2A
#define RT9426_REG_SOC                  0x2C
#define RT9426_REG_SOH                  0x2E
#define RT9426_REG_FLAG3                0x30
#define RT9426_REG_IRQ                  0x36
#define RT9426_REG_DSNCAP               0x3C
#define RT9426_REG_ADV                  0x3A
#define RT9426_REG_EXTREGCNTL           0x3E
#define RT9426_REG_SWINDOW1             0x40
#define RT9426_REG_SWINDOW2             0x42
#define RT9426_REG_SWINDOW3             0x44
#define RT9426_REG_SWINDOW4             0x46
#define RT9426_REG_SWINDOW5             0x48
#define RT9426_REG_SWINDOW6             0x4A
#define RT9426_REG_SWINDOW7             0x4C
#define RT9426_REG_SWINDOW8             0x4E
#define RT9426_REG_SWINDOW9             0x50
#define RT9426_REG_SWINDOW10            0x52
#define RT9426_REG_SWINDOW11            0x54
#define RT9426_REG_SWINDOW12            0x56
#define RT9426_REG_SWINDOW13            0x58
#define RT9426_REG_SWINDOW14            0x5A
#define RT9426_REG_SWINDOW15            0x5C
#define RT9426_REG_SWINDOW16            0x5E
#define RT9426_REG_OCV                  0x62
#define RT9426_REG_AV                   0x64
#define RT9426_REG_AT                   0x66
#define RT9426_REG_UN_FLT_SOC           0x70
#define RT9426_EXIT_SLP_CMD             0x7400
#define RT9426_ENTR_SLP_CMD             0x74AA

#ifdef CONFIG_DEBUG_FS
enum {
	RT9426FG_SOC_OFFSET_SIZE,
	RT9426FG_SOC_OFFSET_DATA,
	RT9426FG_PARAM_LOCK,
	RT9426FG_OFFSET_IP_ORDER,
	RT9426FG_FIND_OFFSET_TEST,
	RT9426FG_PARAM_CHECK,
	RT9426FG_DENTRY_NR,
};
#endif /* CONFIG_DEBUG_FS */

#define RT9426_BATPRES_MASK             0x0040
#define RT9426_RI_MASK                  0x0100
#define RT9426_BATEXIST_FLAG_MASK       0x8000
#define RT9426_USR_TBL_USED_MASK        0x0800
#define RT9426_CSCOMP1_OCV_MASK         0x0300
#define RT9426_UNSEAL_MASK              0x0003
#define RT9426_UNSEAL_STATUS            0x0001

#define RT9426_SMOOTH_POLL              20
#define RT9426_NORMAL_POLL              30
#define RT9426_SOCALRT_MASK             0x20
#define RT9426_SOCL_SHFT                0
#define RT9426_SOCL_MASK                0x1F
#define RT9426_SOCL_MAX                 32
#define RT9426_SOCL_MIN                 1

#define RT9426_RDY_MASK                 0x0080

#define RT9426_UNSEAL_PASS              0
#define RT9426_UNSEAL_FAIL              1
#define RT9426_PAGE_0                   0
#define RT9426_PAGE_1                   1
#define RT9426_PAGE_2                   2
#define RT9426_PAGE_3                   3
#define RT9426_PAGE_4                   4
#define RT9426_PAGE_5                   5
#define RT9426_PAGE_6                   6
#define RT9426_PAGE_7                   7
#define RT9426_PAGE_8                   8
#define RT9426_PAGE_9                   9

#define RT9426_SENSE_R_2P5_MOHM         25 /* 2.5 mohm */
#define RT9426_SENSE_R_5_MOHM           50 /* 5 mohm */
#define RT9426_SENSE_R_10_MOHM          100 /* 10 mohm */
#define RT9426_SENSE_R_20_MOHM          200 /* 20 mohm */

#define RT9426_RS_SEL_MASK              0xFF3F
#define RT9426_RS_SEL_REG_00            0x0000 /* 2.5 mohm */
#define RT9426_RS_SEL_REG_01            0x0040 /* 5 mohm */
#define RT9426_RS_SEL_REG_10            0x0080 /* 10 mohm */
#define RT9426_RS_SEL_REG_11            0x00C0 /* 20 mohm */

#define RT9426_BAT_TEMP_VAL             250
#define RT9426_BAT_VOLT_VAL             3800
#define RT9426_BAT_CURR_VAL             1000
#define RT9426_DESIGN_CAP_VAL           2000
#define RT9426_DESIGN_FCC_VAL           2000

#define RT9426_CHG_CURR_VAL             500

#define RT9426_READ_PAGE_CMD            0x6500
#define RT9426_WRITE_PAGE_CMD           0x6550
#define RT9426_EXT_READ_CMD_PAGE_5      0x6505
#define RT9426_EXT_WRITE_CMD_PAGE_5     0x6555

#define RT9426_OP_CONFIG_SIZE           5
#define RT9426_OP_CONFIG_0_DEFAULT_VAL  0x9480
#define RT9426_OP_CONFIG_1_DEFAULT_VAL  0x3241
#define RT9426_OP_CONFIG_2_DEFAULT_VAL  0x3EFF
#define RT9426_OP_CONFIG_3_DEFAULT_VAL  0x2000
#define RT9426_OP_CONFIG_4_DEFAULT_VAL  0x2A7F

#define RT9426_OCV_DATA_TOTAL_SIZE      80
#define RT9426_OCV_ROW_SIZE             10
#define RT9426_OCV_COL_SIZE             8
#define RT9426_BLOCK_PAGE_SIZE          8
#define RT9426_MAX_PARAMS               12
#define RT9426_WRITE_BUF_LEN            128
#define RT9426_SOC_OFFSET_SIZE          2
#define RT9426_SOC_OFFSET_DATA_SIZE     3
#define RT9426FG_OFFSET_IP_ORDER_SIZE   2
#define RT9426FG_FIND_OFFSET_TEST_SIZE  2
#define RT9426FG_PARAM_LOCK_SIZE        1
#define RT9426_DTSI_VER_SIZE            2
#define RT9426_DT_OFFSET_PARA_SIZE      3
#define RT9426_DT_EXTREG_PARA_SIZE      3
#define RT9426_OFFSET_INTERPLO_SIZE     2
#define RT9426_VOLT_SOURCE_NONE         0
#define RT9426_VOLT_SOURCE_VBAT         1
#define RT9426_VOLT_SOURCE_OCV          2
#define RT9426_VOLT_SOURCE_AV           3
#define RT9426_TEMP_SOURCE_NONE         0
#define RT9426_TEMP_SOURCE_TEMP         1
#define RT9426_TEMP_SOURCE_INIT         2
#define RT9426_TEMP_SOURCE_AT           3
#define RT9426_TEMP_ABR_LOW             (-400)
#define RT9426_TEMP_ABR_HIGH            800
#define RT9426_CURR_SOURCE_NONE         0
#define RT9426_CURR_SOURCE_CURR         1
#define RT9426_CURR_SOURCE_AI           2
#define RT9426_OTC_TTH_DEFAULT_VAL      0x0064
#define RT9426_OTD_ITH_DEFAULT_VAL      0x0064
#define RT9426_OTC_ITH_DEFAULT_VAL      0x0B5F
#define RT9426_OTD_DCHG_ITH_DEFAULT_VAL 0x0B5F
#define RT9426_UV_OV_DEFAULT_VAL        0x00FF
#define RT9426_CURR_DB_DEFAULT_VAL      0x0005
#define RT9426_FC_VTH_DEFAULT_VAL       0x0078
#define RT9426_FC_ITH_DEFAULT_VAL       0x000D
#define RT9426_FC_STH_DEFAULT_VAL       0x0046
#define RT9426_FD_VTH_DEFAULT_VAL       0x0091

#define RT9426_CALI_ENTR_CMD            0x0081
#define RT9426_CALI_EXIT_CMD            0x0080
#define RT9426_CURR_CONVERT_CMD         0x0009
#define RT9426_VOLT_CONVERT_CMD         0x008C
#define RT9426_CALI_MODE_MASK           0x1000
#define RT9426_SYS_TICK_ON_CMD          0xBBA1
#define RT9426_SYS_TICK_OFF_CMD         0xBBA0

#define RT9426_CALI_MODE_PASS           0
#define RT9426_CALI_MODE_FAIL           1

#define RT9426_SHDN_MASK                0x4000
#define RT9426_SHDN_ENTR_CMD            0x64AA
#define RT9426_SHDN_EXIT_CMD            0x6400

#define TA_IS_CONNECTED                 1
#define TA_IS_DISCONNECTED              0
#define RT9426_FD_TBL_IDX               4
#define RT9426_FD_DATA_IDX              10
#define RT9426_FD_BASE                  2500

#define RT9426_HIGH_BYTE_MASK           0xFF00
#define RT9426_LOW_BYTE_MASK            0x00FF
#define RT9426_BYTE_BITS                8

#define RT9426_VTERM_INCREASE           50
#define RT9426_MAX_VTERM_SIZE           5
#define RT9426_REG_OCV_WRITE            0xCB00
#define RT9426_REG_OCV_COL_ADDRESS      0x0040
#define RT9426_OCV_TABLE_FIRST          0x0013

struct data_point {
	union {
		int x;
		int voltage;
		int soc;
	};
	union {
		int y;
		int temperature;
	};
	union {
		int z;
		int curr;
	};
	union {
		int w;
		int offset;
	};
};

struct extreg_data_point {
	union {
		int extreg_page;
	};
	union {
		int extreg_addr;
	};
	union {
		int extreg_data;
	};
};

struct extreg_update_table {
	struct extreg_data_point *extreg_update_data;
};

struct submask_condition {
	int x, y, z;
	int order_x, order_y, order_z;
	int xnr, ynr, znr;
	const struct data_point *mesh_src;
};

struct soc_offset_table {
	int soc_voltnr;
	int tempnr;
	struct data_point *soc_offset_data;
};

enum comp_offset_typs {
	FG_COMP = 0,
	SOC_OFFSET,
	EXTREG_UPDATE,
};

enum { /* temperature source table */
	RT9426_TEMP_FROM_AP,
	RT9426_TEMP_FROM_IC,
};

struct fg_ocv_table {
	int data[RT9426_OCV_COL_SIZE];
};

#define NTC_PARA_LEVEL 10
#define COMPENSATION_THRESHOLD    200 /* 20c */

enum ntc_temp_compensation_para_info {
	NTC_PARA_ICHG = 0,
	NTC_PARA_VALUE,
	NTC_PARA_TOTAL,
};

#endif /* _RT9426_H_ */
