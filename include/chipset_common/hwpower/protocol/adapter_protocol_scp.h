/*
 * adapter_protocol_scp.h
 *
 * scp protocol driver
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

#ifndef _ADAPTER_PROTOCOL_SCP_H_
#define _ADAPTER_PROTOCOL_SCP_H_

#include <linux/bitops.h>

#define SCP_PROTOCOL_BYTE_BITS               8
#define SCP_PROTOCOL_BYTE_MASK               0xff
#define SCP_PROTOCOL_WORD_BITS               16
#define SCP_PROTOCOL_WORD_MASK               0xffff

/* adapter type information register */
#define SCP_PROTOCOL_ADP_TYPE0               0x7e
#define SCP_PROTOCOL_ADP_TYPE0_AB_MASK       (BIT(7) | BIT(5) | BIT(4))
#define SCP_PROTOCOL_UNDETACH_CABLE          BIT(6)
#define SCP_PROTOCOL_ADP_TYPE0_B_MASK        BIT(4)
#define SCP_PROTOCOL_ADP_TYPE0_B_SC_MASK     BIT(3)
#define SCP_PROTOCOL_ADP_TYPE0_B_LVC_MASK    BIT(2)

#define SCP_PROTOCOL_ADP_TYPE1               0x80
#define SCP_PROTOCOL_ADP_TYPE1_B_MASK        BIT(4)

#define SCP_PROTOCOL_COMPILEDVER_HBYTE       0x7c
#define SCP_PROTOCOL_COMPILEDVER_LBYTE       0x7d

/* adapter information register */
#define SCP_PROTOCOL_B_ADP_TYPE              0x81
#define SCP_PROTOCOL_B_ADP_TYPE_B_MASK       BIT(4)

#define SCP_PROTOCOL_VENDOR_ID_HBYTE         0x82
#define SCP_PROTOCOL_VENDOR_ID_LBYTE         0x83

/* vendor id list */
#define SCP_PROTOCOL_VENDOR_ID_FAIRCHILD     0x00A1
#define SCP_PROTOCOL_VENDOR_ID_DIALOG        0x00A2
#define SCP_PROTOCOL_VENDOR_ID_DIODES        0x00A3
#define SCP_PROTOCOL_VENDOR_ID_UPI           0x00A4
#define SCP_PROTOCOL_VENDOR_ID_ON_BRIGHT     0x00A5
#define SCP_PROTOCOL_VENDOR_ID_ON_O2         0x00A6
#define SCP_PROTOCOL_VENDOR_ID_NXP           0x00A7
#define SCP_PROTOCOL_VENDOR_ID_JCKJ          0x00A8
#define SCP_PROTOCOL_VENDOR_ID_GENESYSLOGIC  0x00A9
#define SCP_PROTOCOL_VENDOR_ID_BYD           0x00AA
#define SCP_PROTOCOL_VENDOR_ID_IDT           0x00AB
#define SCP_PROTOCOL_VENDOR_ID_TYKJ          0x00AC
#define SCP_PROTOCOL_VENDOR_ID_SWW           0x00AD
#define SCP_PROTOCOL_VENDOR_ID_MIASTEK       0x00AE
#define SCP_PROTOCOL_VENDOR_ID_RFK           0x00AF
#define SCP_PROTOCOL_VENDOR_ID_RICHTEK       0x00B1
#define SCP_PROTOCOL_VENDOR_ID_ONSEMI        0x00B3
#define SCP_PROTOCOL_VENDOR_ID_HUAWEI        0x00B4
#define SCP_PROTOCOL_VENDOR_ID_NJKB          0x00B5
#define SCP_PROTOCOL_VENDOR_ID_SLJ           0x00B6
#define SCP_PROTOCOL_VENDOR_ID_DLGZ          0x00B7
#define SCP_PROTOCOL_VENDOR_ID_ZHZR          0x00B8
#define SCP_PROTOCOL_VENDOR_ID_TWXW          0x00B9
#define SCP_PROTOCOL_VENDOR_ID_SLW           0x00BA
#define SCP_PROTOCOL_VENDOR_ID_HXWDZ         0x00BB
#define SCP_PROTOCOL_VENDOR_ID_MSDS          0x00BC
#define SCP_PROTOCOL_VENDOR_ID_YJX           0x00BD
#define SCP_PROTOCOL_VENDOR_ID_MPS           0x00BE
#define SCP_PROTOCOL_VENDOR_ID_SYBDT         0x00BF
#define SCP_PROTOCOL_VENDOR_ID_XWKJ          0x00C0
#define SCP_PROTOCOL_VENDOR_ID_IWATT         0x80A2
#define SCP_PROTOCOL_VENDOR_ID_WELTREND      0x80B2

#define SCP_PROTOCOL_MODULE_ID_HBYTE         0x84
#define SCP_PROTOCOL_MODULE_ID_LBYTE         0x85

#define SCP_PROTOCOL_SERIAL_NO_HBYTE         0x86 /* years: start 2015 */
#define SCP_PROTOCOL_SERIAL_NO_LBYTE         0x87 /* weeks: start 0 */
#define SCP_PROTOCOL_START_YEARS             2015
#define SCP_PROTOCOL_START_WEEKS             0

#define SCP_PROTOCOL_CHIP_ID                 0x88
#define SCP_PROTOCOL_CHIP_ID_RICHTEK         0x01
#define SCP_PROTOCOL_CHIP_ID_WELTREND        0x02
#define SCP_PROTOCOL_CHIP_ID_IWATT           0x03
#define SCP_PROTOCOL_CHIP_ID_0X32            0x32

#define SCP_PROTOCOL_HWVER                   0x89

#define SCP_PROTOCOL_FWVER_HBYTE             0x8a /* xx */
#define SCP_PROTOCOL_FWVER_LBYTE             0x8b /* yy.zz */
#define SCP_PROTOCOL_FWVER_XX_MASK           (BIT(7) | BIT(6) | BIT(5) | \
	BIT(4) | BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define SCP_PROTOCOL_FWVER_XX_SHIFT          0
#define SCP_PROTOCOL_FWVER_YY_MASK           (BIT(7) | BIT(6) | BIT(5) | BIT(4))
#define SCP_PROTOCOL_FWVER_YY_SHIFT          4
#define SCP_PROTOCOL_FWVER_ZZ_MASK           (BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define SCP_PROTOCOL_FWVER_ZZ_SHIFT          0

#define SCP_PROTOCOL_RESERVED_0X8C           0x8c

#define SCP_PROTOCOL_ADP_B_TYPE1                   0x8d
#define SCP_PROTOCOL_ADP_B_TYPE1_25W_IWATT         0x00 /* iwatt 5v5a, lvc */
#define SCP_PROTOCOL_ADP_B_TYPE1_25W_RICH1         0x01 /* richtek1 5v5a, lvc */
#define SCP_PROTOCOL_ADP_B_TYPE1_25W_RICH2         0x03 /* richtek2 5v5a, lvc */
#define SCP_PROTOCOL_ADP_B_TYPE1_40W               0x02 /* 10v4a 11v3a, lvc+sc+hv */
#define SCP_PROTOCOL_ADP_B_TYPE1_40W_1             0x07 /* 10v4a, lvc+sc+hv */
#define SCP_PROTOCOL_ADP_B_TYPE1_20W               0x03 /* 10v2a 10v1.8a, sc+hv */
#define SCP_PROTOCOL_ADP_B_TYPE1_65W_MAX           0x04 /* 20v3.25a, lvc+sc+hv */
#define SCP_PROTOCOL_ADP_B_TYPE1_65W               0x05 /* 20v3.25a, sc+hv */
#define SCP_PROTOCOL_ADP_B_TYPE1_65W_1             0x08 /* 20v3.25a, sc+hv */
#define SCP_PROTOCOL_ADP_B_TYPE1_40W_BANK          0x06 /* 10v4a, lvc+sc+hv */
#define SCP_PROTOCOL_ADP_B_TYPE1_40W_CAR           0x09 /* 10v4a, lvc+sc+hv */
#define SCP_PROTOCOL_ADP_B_TYPE1_22P5W             0x0a /* 10v2.25a, sc+hv */
#define SCP_PROTOCOL_ADP_B_TYPE1_22P5W_BANK        0x0b /* wifi pro 5v4.5a 10v2a */
#define SCP_PROTOCOL_ADP_B_TYPE1_22P5W_CAR         0x0c /* 10v2.25a, sc+hv */
#define SCP_PROTOCOL_ADP_B_TYPE1_66W               0x0d /* 11v6a, lvc+sc+hv */
#define SCP_PROTOCOL_ADP_B_TYPE1_66W_CAR           0x0e /* 11v6a */
/* Qiantang River double port adapter typeC+typeA */
#define SCP_PROTOCOL_ADP_B_TYPE1_QTR_C_60W         0x0f /* 10v4a, 20v3a, lvc+sc+hv */
#define SCP_PROTOCOL_ADP_B_TYPE1_QTR_C_40W         0x10 /* 10v4a, lvc+sc+hv */
#define SCP_PROTOCOL_ADP_B_TYPE1_QTR_A_40W         0x11 /* 10v4a, lvc+sc+hv */
#define SCP_PROTOCOL_ADP_B_TYPE1_QTR_A_22P5W       0x12 /* 10v2.25a, lvc+sc+hv */
#define SCP_PROTOCOL_ADP_B_TYPE1_66W_BANK          0x13 /* 11v6a */
/* Huangpu River 2.0 three port adapter typeC1+typeA1+typeA2 */
#define SCP_PROTOCOL_ADP_B_TYPE1_HPR_C_66W         0x14 /* 10v6.6a */
#define SCP_PROTOCOL_ADP_B_TYPE1_HPR_C_40W         0x15 /* 10v4a 20v2a */
#define SCP_PROTOCOL_ADP_B_TYPE1_HPR_A_66W         0x16 /* 10v6.6A */
#define SCP_PROTOCOL_ADP_B_TYPE1_HPR_A_40W         0x17 /* 10v4a 20v2a */
#define SCP_PROTOCOL_ADP_B_TYPE1_HPR_A_22P5W       0x18 /* 10v2.25a */
#define SCP_PROTOCOL_ADP_B_TYPE1_NR_40W            0x19 /* 10v4a */
#define SCP_PROTOCOL_ADP_B_TYPE1_NR_40W_1          0x1a /* 10v4a */
#define SCP_PROTOCOL_ADP_B_TYPE1_NR_40W_2          0x1b /* 10v4a */
#define SCP_PROTOCOL_ADP_B_TYPE1_HHR_90W           0x1c /* 20v4.5a */
#define SCP_PROTOCOL_ADP_B_TYPE1_MJR_66W           0x1d /* 11v6a */
#define SCP_PROTOCOL_ADP_B_TYPE1_66W_CAR_1         0x1e /* 11v6a */
#define SCP_PROTOCOL_ADP_B_TYPE1_HPR_C_66W_1       0x1F
#define SCP_PROTOCOL_ADP_B_TYPE1_HPR_C_40W_1       0x20
#define SCP_PROTOCOL_ADP_B_TYPE1_HPR_A_66W_1       0x21
#define SCP_PROTOCOL_ADP_B_TYPE1_HPR_A_40W_1       0x22
#define SCP_PROTOCOL_ADP_B_TYPE1_HPR_A_22P5W_1     0x23
#define SCP_PROTOCOL_ADP_B_TYPE1_65W_2             0x24 /* 20v3.25a */
#define SCP_PROTOCOL_ADP_B_TYPE1_NR_40W_3          0x25 /* 10v4a */
#define SCP_PROTOCOL_ADP_B_TYPE1_JLR_135W          0x26
#define SCP_PROTOCOL_ADP_B_TYPE1_22P5W_CAR_1       0x27 /* 10v2.25a */
#define SCP_PROTOCOL_ADP_B_TYPE1_40W_2             0x28 /* 10v4a */
#define SCP_PROTOCOL_ADP_B_TYPE1_MJR_66W_1         0x29 /* 11v6a */
#define SCP_PROTOCOL_ADP_B_TYPE1_YLR_100W_CAR      0x2a /* 20v5a */
#define SCP_PROTOCOL_ADP_B_TYPE1_22P5W_BANK_1      0x2b /* 10v2.25a */
#define SCP_PROTOCOL_ADP_B_TYPE1_XR_65W_PC         0x2c /* 20v3.25a */
#define SCP_PROTOCOL_ADP_B_TYPE1_FCR_65W           0x2d /* 20v3.25a */
#define SCP_PROTOCOL_ADP_B_TYPE1_HHR_90W_1         0x2e /* 20v4.5a */
/* Power Strip three port adapter typeC1+typeC2+typeA */
#define SCP_PROTOCOL_ADP_B_TYPE1_PS_C_65W          0x2f /* 20v3.25a */
#define SCP_PROTOCOL_ADP_B_TYPE1_PS_C_40W          0x30 /* 10v4a */
#define SCP_PROTOCOL_ADP_B_TYPE1_PS_C_40W_1        0x31 /* 10v4a */
#define SCP_PROTOCOL_ADP_B_TYPE1_PS_C_22P5W        0x32 /* 10v2.25a */
#define SCP_PROTOCOL_ADP_B_TYPE1_PS_A_40W          0x33 /* 10v4a */
#define SCP_PROTOCOL_ADP_B_TYPE1_PS_A_22P5W        0x34 /* 10v2.25a */

#define SCP_PROTOCOL_FACTORY_ID              0x8e

#define SCP_PROTOCOL_POWER_CURVE_NUM         0x8f
#define SCP_PROTOCOL_POWER_CURVE_BASE0       0xd0
#define SCP_PROTOCOL_POWER_CURVE_BASE1       0xe0
#define SCP_PROTOCOL_POWER_CURVE_SIZE        32
#define SCP_PROTOCOL_POWER_CURVE_VOLT_STEP   500 /* mV */
#define SCP_PROTOCOL_POWER_CURVE_CUR_STEP    100 /* mA */

/* adapter index specification information register */
#define SCP_PROTOCOL_MAX_POWER               0x90
#define SCP_PROTOCOL_CNT_POWER               0x91

#define SCP_PROTOCOL_MIN_VOUT                0x92
#define SCP_PROTOCOL_MAX_VOUT                0x93
#define SCP_PROTOCOL_VOUT_A_MASK             (BIT(7) | BIT(6))
#define SCP_PROTOCOL_VOUT_A_SHIFT            6
#define SCP_PROTOCOL_VOUT_B_MASK             (BIT(5) | BIT(4) | BIT(3) | \
	BIT(2) | BIT(1) | BIT(0))
#define SCP_PROTOCOL_VOUT_B_SHIFT            0

#define SCP_PROTOCOL_VOUT_A_0                0
#define SCP_PROTOCOL_VOUT_A_1                1
#define SCP_PROTOCOL_VOUT_A_2                2
#define SCP_PROTOCOL_VOUT_A_3                3

#define SCP_PROTOCOL_MIN_IOUT                0x94
#define SCP_PROTOCOL_MAX_IOUT                0x95
#define SCP_PROTOCOL_IOUT_A_MASK             (BIT(7) | BIT(6))
#define SCP_PROTOCOL_IOUT_A_SHIFT            6
#define SCP_PROTOCOL_IOUT_B_MASK             (BIT(5) | BIT(4) | BIT(3) | \
	BIT(2) | BIT(1) | BIT(0))
#define SCP_PROTOCOL_IOUT_B_SHIFT            0

#define SCP_PROTOCOL_IOUT_A_0                0
#define SCP_PROTOCOL_IOUT_A_1                1
#define SCP_PROTOCOL_IOUT_A_2                2
#define SCP_PROTOCOL_IOUT_A_3                3

#define SCP_PROTOCOL_VSTEP                   0x96
#define SCP_PROTOCOL_ISTEP                   0x97

#define SCP_PROTOCOL_MAX_VERR                0x98
#define SCP_PROTOCOL_MAX_IEER                0x99

#define SCP_PROTOCOL_MAX_STTIME              0x9a
#define SCP_PROTOCOL_MAX_RSPTIME             0x9b

#define SCP_PROTOCOL_MAX_IOUT_EXTEND         0x9c
#define SCP_PROTOCOL_MAX_IOUT_EXTEND_TH      6000 /* mA */
#define SCP_PROTOCOL_RESERVED_0X9D           0x9d
#define SCP_PROTOCOL_RESERVED_0X9E           0x9e
#define SCP_PROTOCOL_RESERVED_0X9F           0x9f

/* adapter control information register */
#define SCP_PROTOCOL_CTRL_BYTE0              0xa0
#define SCP_PROTOCOL_OUTPUT_EN_MASK          BIT(7)
#define SCP_PROTOCOL_OUTPUT_EN_SHIFT         7
#define SCP_PROTOCOL_OUTPUT_MODE_MASK        BIT(6)
#define SCP_PROTOCOL_OUTPUT_MODE_SHIFT       6
#define SCP_PROTOCOL_RESET_MASK              BIT(5)
#define SCP_PROTOCOL_RESET_SHIFT             5

#define SCP_PROTOCOL_OUTPUT_ENABLE           1
#define SCP_PROTOCOL_OUTPUT_DISABLE          0

#define SCP_PROTOCOL_OUTPUT_MODE_ENABLE      1
#define SCP_PROTOCOL_OUTPUT_MODE_DISABLE     0

#define SCP_PROTOCOL_RESET_ENABLE            1
#define SCP_PROTOCOL_RESET_DISABLE           0

#define SCP_PROTOCOL_CTRL_BYTE1              0xa1
#define SCP_PROTOCOL_DP_DELITCH_MASK         (BIT(4) | BIT(3))
#define SCP_PROTOCOL_DP_DELITCH_SHIFT        3
#define SCP_PROTOCOL_WATCHDOG_MASK           (BIT(2) | BIT(1) | BIT(0))
#define SCP_PROTOCOL_WATCHDOG_SHIFT          0

#define SCP_PROTOCOL_DP_DELITCH_1MS          0x0
#define SCP_PROTOCOL_DP_DELITCH_2MS          0x1
#define SCP_PROTOCOL_DP_DELITCH_3MS          0x2
#define SCP_PROTOCOL_DP_DELITCH_5MS          0x3

#define SCP_PROTOCOL_WATCHDOG_BITS_UNIT      2 /* 1 bit means 0.5 second */

/* adapter status information register */
#define SCP_PROTOCOL_STATUS_BYTE0            0xa2
#define SCP_PROTOCOL_LEAKAGE_FLAG_MASK       BIT(4)
#define SCP_PROTOCOL_LEAKAGE_FLAG_SHIFT      4
#define SCP_PROTOCOL_CCCV_STS_MASK           BIT(4)
#define SCP_PROTOCOL_CCCV_STS_SHIFT          4

#define SCP_PROTOCOL_PORT_LEAKAGE            1
#define SCP_PROTOCOL_PORT_NOT_LEAKAGE        0

#define SCP_PROTOCOL_CC_STATUS               1
#define SCP_PROTOCOL_CV_STATUS               0

#define SCP_PROTOCOL_STATUS_BYTE1            0xa3
#define SCP_PROTOCOL_STATUS_BYTE2            0xa4

#define SCP_PROTOCOL_SSTS                    0xa5
#define SCP_PROTOCOL_SSTS_DPARTO_MASK        (BIT(3) | BIT(2) | BIT(1))
#define SCP_PROTOCOL_SSTS_DPARTO_SHIFT       1
#define SCP_PROTOCOL_SSTS_DROP_MASK          BIT(0)
#define SCP_PROTOCOL_SSTS_DROP_SHIFT         0

#define SCP_PROTOCOL_DROP_ENABLE             1
#define SCP_PROTOCOL_DROP_FACTOR             8

#define SCP_PROTOCOL_INSIDE_TMP              0xa6
#define SCP_PROTOCOL_INSIDE_TMP_UNIT         1 /* step: 1centigrade */

#define SCP_PROTOCOL_PORT_TMP                0xa7
#define SCP_PROTOCOL_PORT_TMP_UNIT           1 /* step: 1centigrade */

#define SCP_PROTOCOL_READ_VOUT_HBYTE         0xa8
#define SCP_PROTOCOL_READ_VOUT_LBYTE         0xa9
#define SCP_PROTOCOL_READ_VOUT_STEP          1 /* step: 1mv */

#define SCP_PROTOCOL_READ_IOUT_HBYTE         0xaa
#define SCP_PROTOCOL_READ_IOUT_LBYTE         0xab
#define SCP_PROTOCOL_READ_IOUT_STEP          1 /* step: 1ma */

#define SCP_PROTOCOL_DAC_VSET_HBYTE          0xac
#define SCP_PROTOCOL_DAC_VSET_LBYTE          0xad

#define SCP_PROTOCOL_DAC_ISET_HBYTE          0xae
#define SCP_PROTOCOL_DAC_ISET_LBYTE          0xaf

#define SCP_PROTOCOL_SREAD_VOUT              0xc8
#define SCP_PROTOCOL_SREAD_VOUT_OFFSET       3000
#define SCP_PROTOCOL_SREAD_VOUT_STEP         10 /* step: 10mv */

#define SCP_PROTOCOL_SREAD_IOUT              0xc9
#define SCP_PROTOCOL_SREAD_IOUT_STEP         50 /* step: 50ma */

/* control adapter information response register */
#define SCP_PROTOCOL_VSET_BOUNDARY_HBYTE     0xb0
#define SCP_PROTOCOL_VSET_BOUNDARY_LBYTE     0xb1
#define SCP_PROTOCOL_VSET_BOUNDARY_STEP      1 /* step: 1mv */

#define SCP_PROTOCOL_ISET_BOUNDARY_HBYTE     0xb2
#define SCP_PROTOCOL_ISET_BOUNDARY_LBYTE     0xb3
#define SCP_PROTOCOL_ISET_BOUNDARYSTEP       1 /* step: 1ma */

#define SCP_PROTOCOL_MAX_VSET_OFFSET         0xb4
#define SCP_PROTOCOL_MAX_ISET_OFFSET         0xb5

#define SCP_PROTOCOL_RESERVED_0XB6           0xb6
#define SCP_PROTOCOL_RESERVED_0XB7           0xb7

#define SCP_PROTOCOL_VSET_HBYTE              0xb8
#define SCP_PROTOCOL_VSET_LBYTE              0xb9
#define SCP_PROTOCOL_VSET_STEP               1 /* step: 1mv */

#define SCP_PROTOCOL_ISET_HBYTE              0xba
#define SCP_PROTOCOL_ISET_LBYTE              0xbb
#define SCP_PROTOCOL_ISET_STEP               1 /* step: 1ma */

#define SCP_PROTOCOL_VSET_OFFSET_HBYTE       0xbc
#define SCP_PROTOCOL_VSET_OFFSET_LBYTE       0xbd

#define SCP_PROTOCOL_ISET_OFFSET_HBYTE       0xbe
#define SCP_PROTOCOL_ISET_OFFSET_LBYTE       0xbf

#define SCP_PROTOCOL_RESERVED_0XC0           0xc0
#define SCP_PROTOCOL_RESERVED_0XC1           0xc1
#define SCP_PROTOCOL_RESERVED_0XC2           0xc2
#define SCP_PROTOCOL_RESERVED_0XC3           0xc3
#define SCP_PROTOCOL_RESERVED_0XC4           0xc4
#define SCP_PROTOCOL_RESERVED_0XC5           0xc5

#define SCP_PROTOCOL_VROFFSET                0xc6
#define SCP_PROTOCOL_VSOFFSET                0xc7

#define SCP_PROTOCOL_VSSET                   0xca
#define SCP_PROTOCOL_VSSET_OFFSET            3000
#define SCP_PROTOCOL_VSSET_STEP              10 /* step: 10mv */
#define SCP_PROTOCOL_VSSET_MAX_VOLT          5500 /* max: 5500mv */

#define SCP_PROTOCOL_ISSET                   0xcb
#define SCP_PROTOCOL_ISSET_STEP              50 /* step: 50ma */

#define SCP_PROTOCOL_STEP_VSET_OFFSET        0xcc
#define SCP_PROTOCOL_STEP_ISET_OFFSET        0xcd

/* adapter encrypt register */
#define SCP_PROTOCOL_KEY_INDEX               0xce
#define SCP_PROTOCOL_KEY_INDEX_BASE          0x00
#define SCP_PROTOCOL_KEY_INDEX_PUBLIC        (SCP_PROTOCOL_KEY_INDEX_BASE + 0x01)
#define SCP_PROTOCOL_KEY_INDEX_1             (SCP_PROTOCOL_KEY_INDEX_BASE + 0x02)
#define SCP_PROTOCOL_KEY_INDEX_2             (SCP_PROTOCOL_KEY_INDEX_BASE + 0x03)
#define SCP_PROTOCOL_KEY_INDEX_3             (SCP_PROTOCOL_KEY_INDEX_BASE + 0x04)
#define SCP_PROTOCOL_KEY_INDEX_4             (SCP_PROTOCOL_KEY_INDEX_BASE + 0x05)
#define SCP_PROTOCOL_KEY_INDEX_5             (SCP_PROTOCOL_KEY_INDEX_BASE + 0x06)
#define SCP_PROTOCOL_KEY_INDEX_6             (SCP_PROTOCOL_KEY_INDEX_BASE + 0x07)
#define SCP_PROTOCOL_KEY_INDEX_7             (SCP_PROTOCOL_KEY_INDEX_BASE + 0x08)
#define SCP_PROTOCOL_KEY_INDEX_8             (SCP_PROTOCOL_KEY_INDEX_BASE + 0x09)
#define SCP_PROTOCOL_KEY_INDEX_9             (SCP_PROTOCOL_KEY_INDEX_BASE + 0x0a)
#define SCP_PROTOCOL_KEY_INDEX_10            (SCP_PROTOCOL_KEY_INDEX_BASE + 0x0b)
#define SCP_PROTOCOL_KEY_INDEX_RELEASE       (SCP_PROTOCOL_KEY_INDEX_BASE + 0xff)

#define SCP_PROTOCOL_ENCRYPT_INFO            0xcf
#define SCP_PROTOCOL_ENCRYPT_ENABLE_MASK     BIT(7)
#define SCP_PROTOCOL_ENCRYPT_ENABLE_SHIFT    7
#define SCP_PROTOCOL_ENCRYPT_COMPLETED_MASK  BIT(6)
#define SCP_PROTOCOL_ENCRYPT_COMPLETED_SHIFT 6

#define SCP_PROTOCOL_ENCRYPT_DISABLE         0
#define SCP_PROTOCOL_ENCRYPT_ENABLE          1

#define SCP_PROTOCOL_ENCRYPT_RANDOM_WR_BASE  0xa0
#define SCP_PROTOCOL_ENCRYPT_RANDOM_WR_SIZE  8

#define SCP_PROTOCOL_ENCRYPT_RANDOM_RD_BASE  0xa8
#define SCP_PROTOCOL_ENCRYPT_RANDOM_RD_SIZE  8

#define SCP_PROTOCOL_ENCRYPT_HASH_RD_BASE    0xb0
#define SCP_PROTOCOL_ENCRYPT_HASH_RD_SIZE    16

#define SCP_PROTOCOL_USBPD_INFO              0xcf
#define SCP_PROTOCOL_USBPD_ENABLE_MASK       BIT(0)
#define SCP_PROTOCOL_USBPD_ENABLE_SHIFT      0

#define SCP_PROTOCOL_USBPD_DISABLE           1
#define SCP_PROTOCOL_USBPD_ENABLE            0

enum scp_protocol_error_code {
	SCP_DETECT_OTHER = -1,
	SCP_DETECT_SUCC = 0,
	SCP_DETECT_FAIL = 1,
};

enum scp_protocol_reset_time {
	RESET_TIME_10MS = 10,
	RESET_TIME_20MS = 20,
	RESET_TIME_30MS = 30,
	RESET_TIME_40MS = 40,
	RESET_TIME_50MS = 50,
	RESET_TIME_51MS = 51,
};

struct scp_protocol_device_info {
	int support_mode; /* adapter support mode */
	int vid_h; /* vendor id */
	int vid_l;
	int mid_h; /* module id */
	int mid_l;
	int serial_h; /* serial no */
	int serial_l;
	int chip_id; /* chip id */
	int hwver; /* hardware version */
	int fwver_h; /* firmware version */
	int fwver_l;
	int chip_vid; /* chip vendor id */
	int min_volt; /* minimum voltage */
	int max_volt; /* maximum voltage */
	int min_cur; /* minimum current */
	int max_cur; /* maximum current */
	int adp_type;
	int vid_rd_flag;
	int mid_rd_flag;
	int serial_rd_flag;
	int chip_id_rd_flag;
	int hwver_rd_flag;
	int fwver_rd_flag;
	int min_volt_rd_flag;
	int max_volt_rd_flag;
	int min_cur_rd_flag;
	int max_cur_rd_flag;
	int adp_type_rd_flag;
	int rw_error_flag;
	int reg80_rw_error_flag;
	int reg7e_rw_error_flag;
	int detect_finish_flag;
	unsigned int is_undetach_cable;
};

struct scp_protocol_ops {
	const char *chip_name;
	void *dev_data;
	int (*reg_read)(int, int *, int, void *);
	int (*reg_write)(int, const int *, int, void *);
	int (*reg_multi_read)(u8, u8 *, u8, void *);
	int (*detect_adapter)(void *);
	int (*soft_reset_master)(void *);
	int (*soft_reset_slave)(void *);
	int (*pre_init)(void *); /* process non protocol flow */
	int (*post_init)(void *); /* process non protocol flow */
	int (*pre_exit)(void *); /* process non protocol flow */
	int (*post_exit)(void *); /* process non protocol flow */
};

struct scp_protocol_dev {
	struct device *dev;
	unsigned char encrypt_random_host[BYTE_EIGHT];
	unsigned char encrypt_random_slave[BYTE_EIGHT];
	unsigned char encrypt_hash_slave[BYTE_SIXTEEN];
	struct scp_protocol_device_info info;
	int dev_id;
	struct scp_protocol_ops *p_ops;
};

#ifdef CONFIG_ADAPTER_PROTOCOL_SCP
int scp_protocol_ops_register(struct scp_protocol_ops *ops);
int scp_protocol_get_reg80_rw_error_flag(void);
int scp_protocol_get_protocol_register_state(void);
#else
static inline int scp_protocol_ops_register(struct scp_protocol_ops *ops)
{
	return -1;
}

static inline int scp_protocol_get_reg80_rw_error_flag(void)
{
	return 0;
}

static inline int scp_protocol_get_protocol_register_state(void)
{
	return -1;
}
#endif /* CONFIG_ADAPTER_PROTOCOL_SCP */

#endif /* _ADAPTER_PROTOCOL_SCP_H_ */
