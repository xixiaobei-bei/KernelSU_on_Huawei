/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: cap prox detect header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __CAP_PROX_DETECT_H__
#define __CAP_PROX_DETECT_H__

#include "sensor_sysfs.h"

#define CAP_PROX_DEV_COUNT_MAX 2

#define CAP_MODEM_THRESHOLE_LEN 8
#define CAP_CALIBRATE_THRESHOLE_LEN 4
#define CAP_CHIPID_DATA_LENGTH 2
#define SAR_USE_PH_NUM 2

struct semteck_sar_data {
	uint16_t threshold_to_ap;
	uint16_t phone_type;
	uint16_t threshold_to_modem[CAP_MODEM_THRESHOLE_LEN]; /* default array length */
	uint32_t init_reg_val[17];
	uint8_t ph;
	uint16_t calibrate_thred[4];
};

struct abov_sar_data {
	uint16_t phone_type;
	uint16_t abov_project_id;
	uint16_t threshold_to_modem[CAP_MODEM_THRESHOLE_LEN];
	uint8_t ph;
	uint16_t calibrate_thred[CAP_CALIBRATE_THRESHOLE_LEN];
};

struct awi_sar_data {
	uint16_t threshold_to_modem;
	uint16_t calibrate_thred[CAP_CALIBRATE_THRESHOLE_LEN];
	uint16_t phone_type;
};
union sar_data {
	struct semteck_sar_data semteck_data;
	struct abov_sar_data abov_data;
	struct awi_sar_data awi_data;
	/* add the others here */
};

/*
 * calibrate_type: config by bit(0~7): 0-free 1-near 2-far other-reserve
 * sar_datas: data for diffrent devices
 */
struct sar_platform_data {
	struct sensor_combo_cfg cfg;
	GPIO_NUM_TYPE gpio_int;
	GPIO_NUM_TYPE gpio_int_sh;
	uint16_t poll_interval;
	uint16_t calibrate_type;
	union sar_data sar_datas;
};

struct sar_sensor_detect {
	char *chip_name;
	struct sensor_combo_cfg cfg;
	uint8_t detect_flag;
	uint16_t chip_id;
	uint32_t chip_id_value[CAP_CHIPID_DATA_LENGTH];
};

struct sar_calibrate_data {
	uint16_t offset[SAR_USE_PH_NUM];
	uint16_t diff[SAR_USE_PH_NUM];
};

enum {
	CAP_PROX_PHASE0,
	CAP_PROX_PHASE1,
};

struct cap_prox_device_info {
	int32_t obj_tag;
	uint32_t detect_list_id;
	uint8_t cap_prox_dev_index;
	uint8_t cap_prox_first_start_flag;
	struct sar_calibrate_data sar_calibrate_datas;
};

struct sar_platform_data *cap_prox_get_platform_data(int32_t tag);
struct cap_prox_device_info *cap_prox_get_device_info(int32_t tag);
char *cap_prox_get_sensors_id_string(void);
void cap_prox_get_sensors_id_from_dts(struct device_node *dn);
char *cap_prox_get_calibrate_order_string(void);
void read_abov_sar_data_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm);
void read_capprox_data_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm);
int cap_prox_sensor_detect(struct device_node *dn,
	struct sensor_detect_manager *sm, int index);
void read_cap_prox1_info(struct device_node *dn);
void read_cap_prox_info(struct device_node *dn);
void cap_prox_detect_init(struct sensor_detect_manager *sm, uint32_t len);
unsigned int get_sar_detect_result(void);
unsigned int get_sar_aux_detect_result(void);

#endif
