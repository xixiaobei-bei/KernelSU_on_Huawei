/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ps detect header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __PS_DETECT_H__
#define __PS_DETECT_H__

#include "sensor_sysfs.h"

#define PS_DEV_COUNT_MAX 1

enum {
	PS_CHIP_NONE = 0,
	PS_CHIP_PA224,
	PS_CHIP_TMD2620,
	PS_CHIP_APDS9110,
	PS_CHIP_TMD3725,
	PS_CHIP_LTR582,
	PS_CHIP_APDS9999,
	PS_CHIP_TMD3702,
	PS_CHIP_VCNL36658,
	PS_CHIP_APDS9253_006,
	PS_CHIP_TCS3701,
	PS_CHIP_LTR2594,
	PS_CHIP_STK3638,
};

enum {
	PS_CALIB_COLLECT_0 = 0,
	PS_CALIB_COLLECT_1 = 1,
	PS_CALIB_COLLECT_2 = 2,
	PS_CALIB_COLLECT_3 = 3,
	PS_CALIB_COLLECT_4 = 4,
	PS_CALIB_COLLECT_5 = 5,
	PS_CALIB_COLLECT_6 = 6,
	PS_CALIB_COLLECT_7 = 7,
	PS_CALIB_COLLECT_TOTAL = 8,
};

struct ps_platform_data {
	struct sensor_combo_cfg cfg;
	int min_proximity_value;
	int pwindows_value;
	int pwave_value;
	int threshold_value;
	int rdata_under_sun;           /* threshold under sun detect */
	int pwindows_screenon_value;
	int pwave_screenon_value;
	int threshold_screenon_value;
	GPIO_NUM_TYPE gpio_int1;
	GPIO_NUM_TYPE gpio_int1_sh;
	uint16_t oily_max_near_pdata;
	uint16_t max_oily_add_pdata;
	uint16_t poll_interval;
	uint16_t init_time;
	uint16_t use_oily_judge;
	uint16_t ps_tp_threshold;
	uint16_t ps_calib_20cm_threshold;
	uint16_t ps_calib_5cm_threshold;
	uint16_t ps_calib_3cm_threshold;
	uint8_t need_reduce_interference;
	uint8_t ps_pulse_count;        /* Pulse number for proximity */
	uint8_t persistent;            /* consecutive Interrupt persistence */
	uint8_t ptime;                 /* Prox integration time */
	uint8_t p_on;                  /* need to close oscillator */
	uint8_t ps_oily_threshold;
	uint8_t wtime;                 /* wait time (ms) */
	uint8_t pulse_len;             /* pulse length (us) */
	uint8_t pgain;                 /* ps gain */
	uint8_t led_current;           /* mA */
	uint8_t prox_avg;              /* open filter or not */
	uint8_t offset_max;
	uint8_t offset_min;
	uint8_t max_near_pdata_loop;
	uint8_t oily_count_size;
	uint8_t digital_offset_max;
	uint8_t is_always_on;
	uint8_t position;
	uint8_t is_check_ps_broke;
	GPIO_NUM_TYPE sound_gpio_irq_soc;
	GPIO_NUM_TYPE sound_gpio_irq_sh;
	uint8_t need_far_near_cali;
	uint16_t pwave_cali_threshold;
	uint16_t pwindow_cali_threshold;
	uint16_t cali_target;
	uint16_t max_oily_threshold;
	uint8_t is_need_lcd_freq;
};

struct ps_device_info {
	int32_t obj_tag;
	uint32_t detect_list_id;
	uint8_t ps_dev_index;
	uint8_t ps_first_start_flag;
	uint8_t ps_cali_supported;
	uint8_t update_ps_cali;
	uint32_t chip_type;
	int32_t ps_support_abs_threshold;
	uint8_t ps_support_cali_after_sale;
	int32_t ps_calib_data_for_data_collect[PS_CALIB_COLLECT_TOTAL];
	enum ret_type ps_calibration_res;
};

struct ps_platform_data *ps_get_platform_data(int32_t tag);
struct ps_device_info *ps_get_device_info(int32_t tag);
ssize_t attr_ultra_ps_feature_show(struct device *dev,
	struct device_attribute *attr, char *buf);
void read_ps_data_from_dts(struct device_node *dn);
int ps_data_from_mcu(const struct pkt_header *head);
int ps_sensor_detect(struct device_node *dn,
	struct sensor_detect_manager *sm, int index);
void ps_detect_init(struct sensor_detect_manager *sm, uint32_t len);
void ps_set_config_to_mcu(int32_t tag);

#endif
