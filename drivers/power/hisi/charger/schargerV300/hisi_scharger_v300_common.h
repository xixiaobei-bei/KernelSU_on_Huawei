/*
 * hisi_scharger_v300_common.h
 *
 * HI6523 charger common inner api header
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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

#ifndef __HI6523_CHARGER_COMMON_H__
#define __HI6523_CHARGER_COMMON_H__
#include <huawei_platform/power/huawei_charger.h>
#include <soc_schargerV300_interface.h>
#include <linux/bitops.h>
#include <linux/i2c.h>
struct hi6523_device_info *get_hi6523_device(void);
int hi6523_write_block(struct hi6523_device_info *di, u8 *value, u8 reg, unsigned int num_bytes);
int hi6523_read_block(struct hi6523_device_info *di, u8 *value, u8 reg, unsigned int num_bytes);
int hi6523_write_byte(u8 reg, u8 value);
int hi6523_read_byte(u8 reg, u8 *value);
int hi6523_write_mask(u8 reg, u8 mask, u8 shift, u8 value);
int hi6523_read_mask(u8 reg, u8 mask, u8 shift, u8 *value);
int hi6523_get_adc_value(u32 chan, u32 *data);
int hi6523_fcp_adapter_reg_read(u8 *val, u8 reg);
int hi6523_fcp_adapter_reg_write(u8 val, u8 reg);
void hi6523_fcp_scp_ops_register(void);
int hi6523_get_vbus_mv(unsigned int *vbus_mv);
int hi6523_sysfs_create_group(struct hi6523_device_info *di);
void hi6523_sysfs_remove_group(struct hi6523_device_info *di);

#define HI6523_I2C_SLAVE_ADDRESS            0x6B
#define CHG_ENABLE                          1
#define CHG_DISABLE                         0

#define CHG_EFUSE_WE7_REG                   0xD1
#define CHG_GAIN_CAL_VALUE_MASK             0xE
#define CHG_GAIN_CAL_REG_MASK               0x10

#define HI6523_REG_TOTAL_NUM                0xEB
#define HI6523_VBUS_STAT_UNKNOWN            0x00

#define CHG_NONE_REG                        0x00
#define CHG_NONE_MSK                        0xFF
#define CHG_NONE_SHIFT                      0x00

#define CHG_VERSION_V300                    0x300
#define CHG_VERSION_V310                    0x310
#define CHG_VERSION_V320                    0x120
#define CHG_VERSION_V200                    0x200

#ifndef MIN
#define MIN(a, b)                           (((a) < (b)) ? (a) : (b))
#endif
#define FCP_ACK_RETRY_CYCLE                 10
#define CHG_VBATT_CHK_CNT                   30
#define CHG_VBATT_SOFT_OVP_CNT              3
#define CHG_VBATT_SOFT_OVP_MAX              4600
#define CHG_VBATT_CV_103(x)                 ((x) * 103 / 100)

#define DPM_DISABLE                         498
#define DPM_ENABLE                          365
#define WEAKSOURCE_CHECK_MAX_CNT            3
#define WEAKSOURCE_CONFIRM_THRES            2
#define FIRST_INSERT_TRUE                   1
#define FIRST_INSERT_FALSE                  0
#define FCP_TRUE                            1
#define FCP_FALSE                           0
#define WEAK_IIN_THRESHOLD                  900
#define WEAK_CHECK_WAIT_100                 100
#define WEAK_TEMP_TRUE                      1
#define WEAK_TEMP_FALSE                     0

#define I2C_ERR_MAX_COUNT                   5
#define RESET_VAL_ZERO                      0

#define HI6523_FAIL                         (-1)
#define HI6523_RESTART_TIME                 4
#define FCP_RETRY_TIME                      5
#define FCP_RESET_RETRY_TIME                3
#define BOOST_5V_CLOSE_FAIL                 (-1)
#define SUCC                                0
#define FAIL                                (-1)

#define HI6523_WATER_CHECKDPDN_NUM          5
#define HI6523_DPDM_CALC_MV(x)              (1U * (x) * 3000U / 4096U)
#define HI6523_DPDM_WATER_THRESH_MIN        700
#define HI6523_DPDM_WATER_THRESH_1460MV     1460
#define HI6523_DPDM_WATER_THRESH_1490MV     1490
#define HI6523_DPDM_WATER_THRESH_1560MV     1560
#define HI6523_DPDM_WATER_THRESH_1590MV     1590
#define HI6523_DPDM_WATER_THRESH_1800MV     1800
#define HI6523_DPDM_WATER_THRESH_2000MV     2000
#define HI6523_DPDM_WATER_THRESH_2500MV     2500
#define HI6523_DPDM_WATER_THRESH_2700MV     2700
#define HI6523_DPDM_WATER_THRESH_MAX        3000

#define WAEKSOURCE_FLAG                     BIT(2)

#define CHG_FCP_SOFT_RST_VAL                (BIT(2))
#define CHG_ILIMIT_100                      100
#define CHG_ILIMIT_130                      130
#define CHG_ILIMIT_470                      470
#define CHG_ILIMIT_699                      699
#define CHG_ILIMIT_900                      900
#define CHG_ILIMIT_1000                     1000
#define CHG_ILIMIT_1080                     1080
#define CHG_ILIMIT_1200                     1200
#define CHG_ILIMIT_1300                     1300
#define CHG_ILIMIT_1400                     1400
#define CHG_ILIMIT_1500                     1500
#define CHG_ILIMIT_1600                     1600
#define CHG_ILIMIT_1700                     1700
#define CHG_ILIMIT_1800                     1800
#define CHG_ILIMIT_1900                     1900
#define CHG_ILIMIT_2000                     2000
#define CHG_ILIMIT_2100                     2100
#define CHG_ILIMIT_2200                     2200
#define CHG_ILIMIT_2280                     2280
#define CHG_ILIMIT_2400                     2400
#define CHG_ILIMIT_2480                     2480
#define CHG_ILIMIT_2600                     2600
#define CHG_ILIMIT_2680                     2680
#define CHG_ILIMIT_2800                     2800
#define CHG_ILIMIT_2880                     2880
#define CHG_ILIMIT_3000                     3000
#define CHG_ILIMIT_3080                     3080
#define CHG_ILIMIT_3200                     3200

#define CHG_FAST_ICHG_STEP_100              100
#define CHG_FAST_ICHG_STEP_200              200
#define CHG_FAST_ICHG_00MA                  0
#define CHG_FAST_ICHG_100MA                 100
#define CHG_FAST_ICHG_200MA                 200
#define CHG_FAST_ICHG_300MA                 300
#define CHG_FAST_ICHG_400MA                 400
#define CHG_FAST_ICHG_500MA                 500
#define CHG_FAST_ICHG_600MA                 600
#define CHG_FAST_ICHG_700MA                 700
#define CHG_FAST_ICHG_800MA                 800
#define CHG_FAST_ICHG_900MA                 900
#define CHG_FAST_ICHG_1000MA                1000
#define CHG_FAST_ICHG_1200MA                1200
#define CHG_FAST_ICHG_1400MA                1400
#define CHG_FAST_ICHG_1600MA                1600
#define CHG_FAST_ICHG_1800MA                1800
#define CHG_FAST_ICHG_2000MA                2000
#define CHG_FAST_ICHG_2200MA                2200
#define CHG_FAST_ICHG_2400MA                2400
#define CHG_FAST_ICHG_2600MA                2600
#define CHG_FAST_ICHG_2800MA                2800
#define CHG_FAST_ICHG_3000MA                3000
#define CHG_FAST_ICHG_3200MA                3200
#define CHG_FAST_ICHG_3400MA                3400
#define CHG_FAST_ICHG_3600MA                3600
#define CHG_FAST_ICHG_3800MA                3800
#define CHG_FAST_ICHG_4000MA                4000
#define CHG_FAST_ICHG_4200MA                4200
#define CHG_FAST_ICHG_4500MA                4500

#define CHG_FAST_VCHG_STEP_50               50
#define CHG_FAST_VCHG_MIN                   3800
#define CHG_FAST_VCHG_3800                  3800
#define CHG_FAST_VCHG_3850                  3850
#define CHG_FAST_VCHG_3900                  3900
#define CHG_FAST_VCHG_3950                  3950
#define CHG_FAST_VCHG_4000                  4000
#define CHG_FAST_VCHG_4050                  4050
#define CHG_FAST_VCHG_4100                  4100
#define CHG_FAST_VCHG_4150                  4150
#define CHG_FAST_VCHG_4200                  4200
#define CHG_FAST_VCHG_4250                  4250
#define CHG_FAST_VCHG_4300                  4300
#define CHG_FAST_VCHG_4350                  4350
#define CHG_FAST_VCHG_4400                  4400
#define CHG_FAST_VCHG_4450                  4450
#define CHG_FAST_VCHG_4500                  4500
#define CHG_FAST_VCHG_MAX                   4500
#endif
