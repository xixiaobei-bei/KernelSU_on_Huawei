/*
 * zinitix_dts.h
 *
 * parase dts config for zinitix driver
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

#ifndef ZINITIX_DTS_H
#define ZINITIX_DTS_H

#include <linux/i2c.h>

#define ZINITIX_VENDOR_COMP_NAME_LEN          32
#define ZINITIX_THRESHOLD_IN_CSV_FILE         1

#define ZINITIX_CHIP_NAME                     "ICA1"
#define HUAWEI_TS_KIT                         "huawei,ts_kit"

#define ZINITIX_IRQ_GPIO                      "attn_gpio"
#define ZINITIX_RST_GPIO                      "reset_gpio"
#define ZINITIX_VDDIO_GPIO_CTRL               "vddio_ctrl_gpio"
#define ZINITIX_VCI_GPIO_CTRL                 "vci_ctrl_gpio"
#define ZINITIX_IRQ_CFG                       "irq_config"
#define ZINITIX_ALGO_ID                       "algo_id"
#define ZINITIX_VDD                           "zts-vdd"
#define ZINITIX_VBUS                          "zts-io"
#define ZINITIX_IC_TYPES                      "ic_type"
#define ZINITIX_IS_IN_CELL                    "is_in_cell"
#define ZINITIX_WD_CHECK                      "need_wd_check_status"
#define ZINITIX_CHECK_STATUS_WATCHDOG_TIMEOUT "check_status_watchdog_timeout"
#define ZINITIX_PRAM_PROJECTID_ADDR           "pram_projectid_addr"
#define ZINITIX_PROJECT_ATM                   "project_atm"
#define ZINITIX_X_MAX                         "x_max"
#define ZINITIX_Y_MAX                         "y_max"
#define ZINITIX_X_MAX_MT                      "x_max_mt"
#define ZINITIX_Y_MAX_MT                      "y_max_mt"
#define ZINITIX_VCI_REGULATOR_TYPE            "vci_regulator_type"
#define ZINITIX_VCI_GPIO_TYPE                 "vci_gpio_type"
#define ZINITIX_VDDIO_REGULATOR_TYPE          "vddio_regulator_type"
#define ZINITIX_VDDIO_GPIO_TYPE               "vddio_gpio_type"
#define ZINITIX_COVER_FORCE_GLOVE             "force_glove_in_smart_cover"
#define ZINITIX_TEST_TYPE                     "tp_test_type"
#define ZINITIX_HOLSTER_SUPPORTED             "holster_supported"
#define ZINITIX_HOSTLER_SWITCH_ADDR           "holster_switch_addr"
#define ZINITIX_HOSTLER_SWITCH_BIT            "holster_switch_bit"
#define ZINITIX_GLOVE_SUPPORTED               "glove_supported"
#define ZINITIX_GLOVE_SWITCH_ADDR             "glove_switch_addr"
#define ZINITIX_GLOVE_SWITCH_BIT              "glove_switch_bit"
#define ZINITIX_ROI_SUPPORTED                 "roi_supported"
#define ZINITIX_ROI_SWITCH_ADDR               "roi_switch_addr"
#define ZINITIX_ROI_PKG_NUM_ADDR              "roi_pkg_num_addr"

#define ZINITIX_VCI_LDO_VALUE                 "vci_value"
#define ZINITIX_VDDIO_LDO_VALUE               "vddio_value"
#define ZINITIX_POWER_SELF_CTRL               "power_self_ctrl"
#define ZINITIX_POWER_DOWN_CTRL               "power_down_ctrl"
#define ZINITIX_NEED_SET_VDDIO_VALUE          "need_set_vddio_value"

#define ZINITIX_FW_UPDATE_LOGIC               "fw_update_logic"

#define ZINITIX_PROJECTID_LEN_CTRL_FLAG       "projectid_length_control_flag"

#define ZINITIX_SLAVE_ADDR                    "slave_address"
#define ZINITIX_SLAVE_ADDR_DFT                (0x20)
/* option delay time */
#define ZINITIX_REBOOT_DELAY                  "reboot_delay"
#define ZINITIX_REBOOT_DELAY_DFT              (200)

#define ZINITIX_GESTURE_SUPPORTED             "gesture_supported"
#define ZINITIX_ENABLE_EDGE_TOUCH             "enable_edge_touch"
#define ZINITIX_EDGE_DATA_ADDR                "zinitix_edge_data_addr"
#define ZINITIX_FW_ONLY_DEPEND_ON_LCD         "zts,fw_only_depend_on_lcd"
#define ZINITIX_FW_NEED_DISTINGUISH_LCD       "zts,need_distinguish_lcd"
#define ZINITIX_TOUCH_SWITCH_FLAG             "touch_switch_flag"
#define ZINITIX_TOUCH_SWITCH_GAME_REG         "touch_switch_game_reg"
#define PROJECT_ATM                           1

/* tp test */
#define ZINITIX_LCD_NOISE_DATA_TEST           "lcd_noise_data_test"
#define ZINITIX_POINT_BY_POINT_JUDGE          "threshold,point_by_point_judge"
#define ZINITIX_OPEN_ONCE_THRESHOLD           "only_open_once_captest_threshold"
#define ZINITIX_IN_CSV_FILE                   "threshold,in_csv_file"
#define ZINITIX_RAW_DATA_MIN                  "threshold,raw_data_min"
#define ZINITIX_RAW_DATA_MAX                  "threshold,raw_data_max"
#define ZINITIX_SCAP_RAW_DATA_MIN             "threshold,scap_raw_data_min"
#define ZINITIX_SCAP_RAW_DATA_MAX             "threshold,scap_raw_data_max"
#define ZINITIX_LCD_NOISE_MAX                 "threshold,lcd_noise_max"
#define ZINITIX_RAW_DATA_MIN_ARRAY            "threshold,raw_data_min_array"
#define ZINITIX_RAW_DATA_MAX_ARRAY            "threshold,raw_data_max_array"
#define ZINITIX_TEST_TYPE_DEFAULT             "Normalize_type:judge_last_result"

#define PINCTRL_STATE_ACTIVE                  "pmx_ts_active"
#define PINCTRL_STATE_SUSPEND                 "pmx_ts_suspend"
#define PINCTRL_STATE_RELEASE                 "pmx_ts_release"

int zinitix_get_vendor_name(struct zinitix_platform_data *zinitix_pdata);

int zinitix_parse_dts(struct device_node *np,
		struct zinitix_platform_data *zinitix_pdata);

int zinitix_parse_ic_config_dts(struct device_node *np,
		struct ts_kit_device_data *dev_data);

int zinitix_parse_cap_test_config(struct zinitix_platform_data *pdata,
		struct zinitix_test_params *params);

#endif
