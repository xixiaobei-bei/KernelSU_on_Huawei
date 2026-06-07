/*
 * hisi_charger_scp.h
 *
 * scp define for hisi charger
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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

#ifndef _HISI_CHARGER_SCP_H_
#define _HISI_CHARGER_SCP_H_

#include <huawei_platform/power/direct_charger.h>

#ifndef BIT
#define BIT(x) (1 << x)
#endif

#define SCP_ADP_TYPE                 0x80
#define SCP_ADP_TYPE_B_MASK          (BIT(5) | BIT(4))
#define SCP_ADP_TYPE_B               0x10

#define SCP_ADP_TYPE0                0x7E
#define SCP_ADP_TYPE0_B_MASK         BIT(4)
#define SCP_ADP_TYPE0_B_SC_MASK      BIT(3)
#define SCP_ADP_TYPE0_B_LVC_MASK     BIT(2)

#define SCP_B_ADP_TYPE               0x81
#define SCP_B_DIRECT_ADP             0x10

#define SCP_VENDOR_ID_H              0x82
#define SCP_VENDOR_ID_L              0x83
#define SCP_MODULE_ID_H              0x84
#define SCP_MODULE_ID_L              0x85
#define SCP_SERRIAL_NO_H             0x86
#define SCP_SERRIAL_NO_L             0x87
#define SCP_PCHIP_ID                 0x88
#define SCP_HWVER                    0x89
#define SCP_FWVER_H                  0x8a
#define SCP_FWVER_L                  0x8b
#define SCP_SPID                     0x8c
#define SCP_NEED_OPEN_OUTPUT2        0x31
#define SCP_MAX_POWER                0x90
#define SCP_CNT_POWER                0x91
#define SCP_MIN_VOUT                 0x92
#define SCP_MAX_VOUT                 0x93
#define SCP_MIN_IOUT                 0x94
#define SCP_MAX_IOUT                 0x95
#define SCP_MAX_IOUT_A_MASK          (BIT(7) | BIT(6))
#define SCP_MAX_IOUT_A_SHIFT         6
#define SCP_MAX_IOUT_B_MASK          (BIT(5) | BIT(4) | BIT(3) | BIT(2) | \
	BIT(1) | BIT(0))
#define SCP_VSTEP                    0x96
#define SCP_ISTEP                    0x97
#define SCP_MAX_VERR                 0x98
#define SCP_MAX_IVRR                 0x99
#define SCP_MAX_STTIME               0x9a
#define SCP_MAX_RSPTIME              0x9b
#define SCP_SCTRL                    0x9e
#define SCP_OUTPUT2_ENABLE_MASK      BIT(7)
#define SCP_OUTPUT2_ENABLE           BIT(7)
#define SCP_CTRL_BYTE0               0xa0
#define SCP_OUTPUT_MODE_MASK         BIT(6)
#define SCP_OUTPUT_MODE_ENABLE       BIT(6)
#define SCP_OUTPUT_MODE_DISABLE      0
#define SCP_OUTPUT_MASK              BIT(7)
#define SCP_OUTPUT_ENABLE            BIT(7)
#define SCP_OUTPUT_DISABLE           0
#define SCP_ADAPTOR_RESET_MASK       BIT(5)
#define SCP_ADAPTOR_RESET_ENABLE     BIT(5)
#define SCP_ADAPTOR_RESET_DISABLE    0
#define SCP_CTRL_BYTE1               0xa1
#define SCP_DP_DELITCH_MASK          (BIT(3) | BIT(4))
#define SCP_DP_DELITCH_5_MS          (BIT(3) | BIT(4))
#define SCP_WATCHDOG_BITS_PER_SECOND 2
#define SCP_WATCHDOG_MASK            (BIT(2) | BIT(1) | BIT(0))
#define SCP_STATUS_BYTE0             0xa2
#define SCP_CABLE_STS_MASK           BIT(3)
#define SCP_PORT_LEAKAGE_INFO        BIT(4)
#define SCP_PORT_LEAKAGE_SHIFT       4
#define SCP_STATUS_BYTE1             0xa3
#define SCP_SSTS                     0xa5
#define SCP_SSTS_A_MASK              (BIT(3) | BIT(2) | BIT(1))
#define SCP_SSTS_B_MASK              BIT(0)
#define SCP_SSTS_B_SHIFT             0
#define SCP_INSIDE_TMP               0xa6
#define SCP_PORT_TMP                 0xa7
#define SCP_READ_VOLT_L              0xa8
#define SCP_READ_VOLT_H              0xa9
#define SCP_READ_IOLT_L              0xaa
#define SCP_READ_IOLT_H              0xab
#define SCP_SREAD_VOUT               0xc8
#define SCP_SREAD_VOUT_OFFSET        3000
#define SCP_SREAD_VOUT_STEP          10
#define SCP_SREAD_IOUT               0xc9
#define SCP_SREAD_IOUT_STEP          50
#define SCP_DAC_VSET_L               0xac
#define SCP_DAC_VSET_H               0xad
#define SCP_DAC_ISET_L               0xae
#define SCP_DAC_ISET_H               0xaf
#define SCP_VSET_BOUNDARY_L          0xb0
#define SCP_VSET_BOUNDARY_H          0xb1
#define SCP_ISET_BOUNDARY_L          0xb2
#define SCP_ISET_BOUNDARY_H          0xb3
#define SCP_MAX_VSET_OFFSET          0xb4
#define SCP_MAX_ISET_OFFSET          0xb5
#define SCP_VSET_L                   0xb8
#define SCP_VSET_H                   0xb9
#define SCP_ISET_L                   0xba
#define SCP_ISET_H                   0xbb
#define SCP_VSSET                    0xca
#define VSSET_OFFSET                 3000
#define VSSET_STEP                   10
#define SCP_ISSET                    0xcb
#define ISSET_STEP                   50
#define SCP_VSET_OFFSET_L            0xbc
#define SCP_VSET_OFFSET_H            0xbd
#define SCP_ISET_OFFSET_L            0xbe
#define SCP_ISET_OFFSET_H            0xbf
#define SCP_STEP_VSET_OFFSET         0xcc
#define SCP_STEP_ISET_OFFSET         0xcd

#define SCP_CABLE_DETECT_FAIL        1
#define SCP_CABLE_DETECT_SUCC        0
#define SCP_CABLE_DETECT_ERROR       (-1)

#define MAX_IOUT_EXP_0               0
#define MAX_IOUT_EXP_1               1
#define MAX_IOUT_EXP_2               2
#define MAX_IOUT_EXP_3               3
#define TEN_EXP_0                    1
#define TEN_EXP_1                    10
#define TEN_EXP_2                    100
#define TEN_EXP_3                    1000
#define DROP_POWER_FLAG              1
#define DROP_POWER_FACTOR            8
#define SCP_NO_ERR                   0
#define SCP_IS_ERR                   1
#define ONE_BYTE_LEN                 8
#define ONE_BYTE_MASK                0xff
#define ONE_BIT_EQUAL_TWO_SECONDS    2
#define WAIT_FOR_ADAPTOR_RESET       50
#define OUTPUT_MODE_ENABLE           1
#define OUTPUT_MODE_DISABLE          0

#define VENDOR_ID_RICHTEK            0x01
#define VENDOR_ID_WELTREND           0x02
#define VENDOR_ID_IWATT              0x03
#define VENDOR_ID_0X32               0x32

#define ADAPTOR_RESET                1

#define SCP_ADAPTOR_DETECT_FAIL      1
#define SCP_ADAPTOR_DETECT_SUCC      0
#define SCP_ADAPTOR_DETECT_OTHER     (-1)

enum adapter_vendor {
	RICHTEK_ADAPTER,
	IWATT_ADAPTER,
	WELTREND_ADAPTER,
	ID0X32_ADAPTER,
};

struct scp_init_data {
	int scp_mode_enable;
	int vset_boundary;
	int iset_boundary;
	int init_adaptor_voltage;
	int watchdog_timer;
};

struct adaptor_info {
	int b_adp_type;
	int vendor_id_h;
	int vendor_id_l;
	int module_id_h;
	int module_id_l;
	int serrial_no_h;
	int serrial_no_l;
	int pchip_id;
	int hwver;
	int fwver_h;
	int fwver_l;
};

struct smart_charge_ops {
	int (*is_support_scp)(void);
	int (*scp_init)(struct scp_init_data *);
	int (*scp_exit)(struct direct_charge_device *);
	int (*scp_adaptor_detect)(void);
	int (*scp_set_adaptor_voltage)(int);
	int (*scp_get_adaptor_voltage)(void);
	int (*scp_set_adaptor_current)(int);
	int (*scp_get_adaptor_current)(void);
	int (*scp_get_adaptor_current_set)(void);
	int (*scp_get_adaptor_max_current)(void);
	int (*scp_adaptor_output_enable)(int);
	int (*scp_stop_charge_config)(void);
	int (*is_scp_charger_type)(void);
	int (*scp_get_adaptor_status)(void);
	int (*scp_get_chip_status)(void);
	int (*scp_get_adaptor_info)(void *);
	int (*scp_cable_detect)(void);
	int (*scp_get_adaptor_temp)(int *);
	int (*scp_get_adapter_vendor_id)(void);
	int (*scp_get_usb_port_leakage_current_info)(void);
	void (*scp_set_direct_charge_mode)(int);
	int (*scp_get_adaptor_type)(void);
};

#endif /* _HISI_CHARGER_SCP_H_ */
