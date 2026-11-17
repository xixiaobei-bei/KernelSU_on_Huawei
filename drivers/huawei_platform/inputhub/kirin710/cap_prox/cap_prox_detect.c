/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: cap prox detect source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "cap_prox_detect.h"

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <securec.h>

#include "contexthub_route.h"

#define RET_FAIL             (-1)

static char *sar_calibrate_order = NULL;

struct sar_platform_data sar_pdata = {
	.cfg = DEF_SENSOR_COM_SETTING,
	.poll_interval = 200, /* 200 ms */
	.calibrate_type = 5, /* 5: calibrate type */
	.gpio_int = 0,
};

struct adux_sar_add_data_t adux_sar_add_data;

char *cap_prox_get_calibrate_order_string(void)
{
	return sar_calibrate_order;
}

static void read_abov_sar_data_from_dts(struct device_node *dn)
{
	uint16_t abov_phone_type = 0;
	uint16_t abov_project_id_type = 0;
	uint16_t *calibrate_thred = NULL;

	read_chip_info(dn, CAP_PROX);
	if (of_property_read_u16(dn, "phone_type", &abov_phone_type)) {
		sar_pdata.sar_datas.abov_data.phone_type = 0;
		hwlog_err("%s:read phone_type fail\n", __func__);
	} else {
		sar_pdata.sar_datas.abov_data.phone_type = abov_phone_type;
		hwlog_info("%s:read phone_type:0x%x\n", __func__,
			sar_pdata.sar_datas.abov_data.phone_type);
	}

	if (of_property_read_u16(dn, "abov_project_id", &abov_project_id_type)) {
		sar_pdata.sar_datas.abov_data.abov_project_id = 0;
		hwlog_err("%s:read abov_project_id fail\n", __func__);
	} else {
		sar_pdata.sar_datas.abov_data.abov_project_id = abov_project_id_type;
		hwlog_info("%s:read abov_project_id:0x%x\n", __func__,
			sar_pdata.sar_datas.abov_data.abov_project_id);
	}

	calibrate_thred = sar_pdata.sar_datas.abov_data.calibrate_thred;
	if (of_property_read_u16_array(dn, "calibrate_thred", calibrate_thred,
		CAP_CALIBRATE_THRESHOLE_LEN)) {
		hwlog_err("%s:read calibrate_thred fail\n", __func__);
		*calibrate_thred = 0;
		*(calibrate_thred + 1) = 0;
		*(calibrate_thred + 2) = 0; /* 2: calibrate_thred[2] */
		*(calibrate_thred + 3) = 0; /* 3: calibrate_thred[3] */
	}
	hwlog_info("calibrate_thred:%u %u %u %u\n", *calibrate_thred,
		*(calibrate_thred + 1),
		*(calibrate_thred + 2), /* 2: calibrate_thred[2] */
		*(calibrate_thred + 3)); /* 3: calibrate_thred[3] */
}

static void read_cap_prox_config1_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	int temp;

	temp = of_get_named_gpio(dn, "gpio_int", 0);
	if (temp < 0)
		hwlog_err("%s:read gpio_int1 fail\n", __func__);
	else
		sar_pdata.gpio_int = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "gpio_int_sh", &temp))
		hwlog_err("%s:read gpio_int_sh fail\n", __func__);
	else
		sar_pdata.gpio_int_sh = (GPIO_NUM_TYPE)temp;
	if (of_property_read_u32(dn, "reg", &temp))
		hwlog_err("%s:read cap_prox reg fail\n", __func__);
	else
		sar_pdata.cfg.i2c_address = (uint8_t)temp;
	if (of_property_read_u32(dn, "poll_interval", &temp))
		hwlog_err("%s:read poll_interval fail\n", __func__);
	else
		sar_pdata.poll_interval = (uint16_t)temp;
	hwlog_info("sar.poll_interval: %d\n", sar_pdata.poll_interval);

	if (of_property_read_u32(dn, "file_id", &temp))
		hwlog_err("%s:read sar file_id fail\n", __func__);
	else
		read_dyn_file_list((uint16_t)temp);

	if (of_property_read_u32(dn, "sensor_list_info_id", &temp))
		hwlog_err("%s:read sar sensor_list_info_id fail\n", __func__);
	else
		add_sensor_list_info_id((uint16_t)temp);
}

static void read_cap_prox_config2_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	int temp = 0;

	if (of_property_read_u32(dn, "calibrate_type", &temp))
		hwlog_err("%s:read sar calibrate_type fail\n", __func__);
	else
		sar_pdata.calibrate_type = temp;

	if (of_property_read_string(dn, "calibrate_order",
		(const char **)&sar_calibrate_order))
		hwlog_err("read calibrate order err\n");

	hwlog_info("calibrate order:%s\n", sar_calibrate_order);
}

static void read_cap_prox_cypress_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	uint16_t threshold_to_ap = 0;
	uint16_t *threshold_to_modem = NULL;

	if (of_property_read_u16(dn, "to_ap_threshold", &threshold_to_ap))
		sar_pdata.sar_datas.cypress_data.threshold_to_ap = 0xC8;
	else
		sar_pdata.sar_datas.cypress_data.threshold_to_ap = threshold_to_ap;

	threshold_to_modem = sar_pdata.sar_datas.cypress_data.threshold_to_modem;
	if (of_property_read_u16_array(dn, "to_modem_threshold",
		threshold_to_modem, CAP_MODEM_THRESHOLE_LEN)) {
		*threshold_to_modem = 0xC8;
		*(threshold_to_modem + 1) = 0;
	}
	hwlog_info("ap:%u, modem:%u %u %u\n", threshold_to_ap,
		*threshold_to_modem, *(threshold_to_modem + 1),
		*(threshold_to_modem + (CAP_MODEM_THRESHOLE_LEN - 1)));
}

static void read_cap_prox_adi1_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	int temp = 0;
	int i;

	if (memset_s(&adux_sar_add_data, sizeof(adux_sar_add_data), 0x00,
		sizeof(adux_sar_add_data)) != EOK)
		return;

	if (of_property_read_u16_array(dn, "to_ap_threshold",
		adux_sar_add_data.threshold_to_ap_stg, STG_SUPPORTED_NUM)) {
		adux_sar_add_data.threshold_to_ap_stg[0] = DEFAULT_THRESHOLD;
		adux_sar_add_data.threshold_to_ap_stg[1] = DEFAULT_THRESHOLD;
		adux_sar_add_data.threshold_to_ap_stg[2] = DEFAULT_THRESHOLD;
		hwlog_info("read threshold_to_ap fail\n");
	}

	if (of_property_read_u16_array(dn, "to_modem_threshold",
		adux_sar_add_data.threshold_to_modem_stg,
		TO_MODEM_SUPPORTED_LEVEL_NUM * STG_SUPPORTED_NUM)) {
		adux_sar_add_data.threshold_to_modem_stg[0] = DEFAULT_THRESHOLD;
		adux_sar_add_data.threshold_to_modem_stg[1 * TO_MODEM_SUPPORTED_LEVEL_NUM] =
			DEFAULT_THRESHOLD;
		/* index 2 */
		adux_sar_add_data.threshold_to_modem_stg[2 * TO_MODEM_SUPPORTED_LEVEL_NUM] =
			DEFAULT_THRESHOLD;
		hwlog_info("read threshold_to_modem fail\n");
	}

	if (of_property_read_u32(dn, "update_offset", &temp))
		hwlog_err("%s:read sar updata_offset fail\n", __func__);
	else
		adux_sar_add_data.updata_offset = temp;

	if (of_property_read_u32(dn, "cdc_calue_threshold", &temp))
		hwlog_err("%s:read sar cdc_calue_threshold fail\n", __func__);
	else
		adux_sar_add_data.cdc_calue_threshold = temp;

	hwlog_info("ap0=0x%x, ap1=0x%x, ap2=0x%x\n",
		adux_sar_add_data.threshold_to_ap_stg[0],
		adux_sar_add_data.threshold_to_ap_stg[1],
		adux_sar_add_data.threshold_to_ap_stg[2]);

	for (i = 0; i < TO_MODEM_SUPPORTED_LEVEL_NUM * STG_SUPPORTED_NUM; i++)
		hwlog_info("modem%d = 0x%x\n", i, adux_sar_add_data.threshold_to_modem_stg[i]);
}

static void read_cap_prox_adi2_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	uint16_t *calibrate_thred = NULL;
	uint32_t *init_reg_val = NULL;
	uint32_t init_reg_val_default[17] = { /* count 17 */
		0x00010005, 0x00020529, 0x000300cc, 0x00040001,
		0x00050F55,
		0x00069905, 0x000700e8, 0x00080200, 0x00090000,
		0x000a000C, 0x00798000,
		0x000b9905, 0x000c00e8, 0x000d0200, 0x000e0000,
		0x000f000C, 0x007a8000
	};

	calibrate_thred = adux_sar_add_data.calibrate_thred;
	if (of_property_read_u16_array(dn, "calibrate_thred",
		calibrate_thred, CAP_CALIBRATE_THRESHOLE_LEN)) {
		hwlog_err("%s:read calibrate_thred fail\n", __func__);
		*calibrate_thred = 0;
		*(calibrate_thred + 1) = 0;
		*(calibrate_thred + 2) = 0; /* 2: calibrate_thred[2] */
		*(calibrate_thred + 3) = 0; /* 3: calibrate_thred[3] */
	}
	hwlog_info("calibrate_thred:%u %u %u %u\n",
		*calibrate_thred, *(calibrate_thred + 1),
		*(calibrate_thred + 2), /* 2: calibrate_thred[2] */
		*(calibrate_thred + 3)); /* 3: calibrate_thred[3] */

	init_reg_val = sar_pdata.sar_datas.adux_data.init_reg_val;
	if (of_property_read_u32_array(dn, "init_reg_val", init_reg_val,
		ADUX_REGS_NEED_INITIATED_NUM)) {
		hwlog_err("%s:read init_reg_val fail\n", __func__);
		if (memcpy_s(init_reg_val,
			sizeof(sar_pdata.sar_datas.adux_data.init_reg_val),
			init_reg_val_default,
			sizeof(init_reg_val_default)) != EOK)
			return;
	}
	hwlog_info("init_reg_val[0]:%8x init_reg_val[%d]%8x\n",
		*init_reg_val, ADUX_REGS_NEED_INITIATED_NUM - 1,
		*(init_reg_val + ADUX_REGS_NEED_INITIATED_NUM - 1));
}

static void read_cap_prox_semtech1_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	uint16_t threshold_to_ap = 0;
	uint16_t *threshold_to_modem = NULL;
	uint32_t *init_reg_val = NULL;
	uint32_t init_reg_val_default[17] = { /* count 17 */
		0x00010005, 0x00020529, 0x000300cc, 0x00040001,
		0x00050F55,
		0x00069905, 0x000700e8, 0x00080200, 0x00090000,
		0x000a000C, 0x00798000,
		0x000b9905, 0x000c00e8, 0x000d0200, 0x000e0000,
		0x000f000C, 0x007a8000
	};

	if (of_property_read_u16(dn, "to_ap_threshold", &threshold_to_ap))
		sar_pdata.sar_datas.semteck_data.threshold_to_ap = 0xC8;
	else
		sar_pdata.sar_datas.semteck_data.threshold_to_ap = threshold_to_ap;

	threshold_to_modem = sar_pdata.sar_datas.semteck_data.threshold_to_modem;
	if (of_property_read_u16_array(dn, "to_modem_threshold",
		threshold_to_modem, CAP_MODEM_THRESHOLE_LEN)) {
		*threshold_to_modem = 0xC8;
		*(threshold_to_modem + 1) = 0;
		hwlog_info("read threshold_to_modem fail\n");
	}
	hwlog_info("read threshold_to_modem %u %u %u\n",
		*threshold_to_modem, *(threshold_to_modem + 1),
		*(threshold_to_modem + (CAP_MODEM_THRESHOLE_LEN - 1)));

	init_reg_val = sar_pdata.sar_datas.semteck_data.init_reg_val;
	if (of_property_read_u32_array(dn, "init_reg_val", init_reg_val,
		SEMTECH_REGS_NEED_INITIATED_NUM)) {
		hwlog_err("%s:read init_reg_val fail\n", __func__);
		if (memcpy_s(init_reg_val,
			sizeof(sar_pdata.sar_datas.semteck_data.init_reg_val),
			init_reg_val_default,
			sizeof(init_reg_val_default)) != EOK)
			return;
	}
	hwlog_info("init_reg_val[0]:%8x init_reg_val[%d]%8x\n",
		*init_reg_val, SEMTECH_REGS_NEED_INITIATED_NUM - 1,
		*(init_reg_val + SEMTECH_REGS_NEED_INITIATED_NUM - 1));
}

static void read_cap_prox_semtech2_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	uint16_t temp16 = 0;
	uint32_t ph = 0;
	uint16_t *calibrate_thred = NULL;

	if (of_property_read_u16(dn, "phone_type", &temp16)) {
		sar_pdata.sar_datas.semteck_data.phone_type = 0;
		hwlog_err("%s:read phone_type fail\n", __func__);
	} else {
		sar_pdata.sar_datas.semteck_data.phone_type = temp16;
		hwlog_info("%s:read phone_type:0x%x\n",
			__func__, sar_pdata.sar_datas.semteck_data.phone_type);
	}

	if (of_property_read_u32(dn, "ph", &ph)) {
		sar_pdata.sar_datas.semteck_data.ph = 0x2f;
		hwlog_err("%s:read ph fail\n", __func__);
	} else {
		sar_pdata.sar_datas.semteck_data.ph = (uint8_t)ph;
		hwlog_info("%s:read ph:0x%x\n",
			__func__, sar_pdata.sar_datas.semteck_data.ph);
	}

	calibrate_thred = sar_pdata.sar_datas.semteck_data.calibrate_thred;
	if (of_property_read_u16_array(dn, "calibrate_thred",
		calibrate_thred, CAP_CALIBRATE_THRESHOLE_LEN)) {
		hwlog_err("%s:read calibrate_thred fail\n", __func__);
		*calibrate_thred = 0;
		*(calibrate_thred + 1) = 0;
		*(calibrate_thred + 2) = 0; /* 2: calibrate_thred[2] */
		*(calibrate_thred + 3) = 0; /* 3: calibrate_thred[3] */
	}

	hwlog_info("calibrate_thred:%u %u %u %u\n",
		*calibrate_thred, *(calibrate_thred + 1),
		*(calibrate_thred + 2), /* 2: calibrate_thred[2] */
		*(calibrate_thred + 3)); /* 3: calibrate_thred[3] */
}

static void read_cap_prox_aw96101_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	uint16_t *threshold_to_modem = NULL;
	uint32_t *init_reg_val = NULL;
	uint32_t init_reg_val_default[INIT_REG_VALUE_COUNT] = {
		0x00010005, 0x00020529, 0x000300cc, 0x00040001,
		0x00050F55,
		0x00069905, 0x000700e8, 0x00080200, 0x00090000,
		0x000a000C, 0x00798000,
		0x000b9905, 0x000c00e8, 0x000d0200, 0x000e0000,
		0x000f000C, 0x007a8000
	};

	threshold_to_modem = sar_pdata.sar_datas.aw9610_data.threshold_to_modem;
	if (of_property_read_u16_array(dn, "to_modem_threshold",
		threshold_to_modem, CAP_MODEM_THRESHOLE_LEN)) {
		*threshold_to_modem = 0xC8;
		*(threshold_to_modem + 1) = 0;
		hwlog_info("read threshold_to_modem fail\n");
	}
	hwlog_info("read threshold_to_modem %u %u %u\n",
		*threshold_to_modem, *(threshold_to_modem + 1),
		*(threshold_to_modem + (CAP_MODEM_THRESHOLE_LEN - 1)));

	init_reg_val = sar_pdata.sar_datas.aw9610_data.init_reg_val;
	if (of_property_read_u32_array(dn, "init_reg_val", init_reg_val,
		AWINIC_REGS_NEED_INITIATED_NUM)) {
		hwlog_err("%s:read init_reg_val fail\n", __func__);
		if (memcpy_s(init_reg_val,
			sizeof(sar_pdata.sar_datas.aw9610_data.init_reg_val),
			init_reg_val_default,
			sizeof(init_reg_val_default)) != EOK)
			return;
	}
	hwlog_info("init_reg_val[0]:%8x init_reg_val[%d]%8x\n",
		*init_reg_val, AWINIC_REGS_NEED_INITIATED_NUM - 1,
		*(init_reg_val + AWINIC_REGS_NEED_INITIATED_NUM - 1));
}

static void read_cap_prox_aw96102_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	uint16_t *calibrate_thred = NULL;

	calibrate_thred = sar_pdata.sar_datas.aw9610_data.calibrate_thred;
	if (of_property_read_u16_array(dn, "calibrate_thred",
		calibrate_thred, CAP_CALIBRATE_THRESHOLE_LEN)) {
		hwlog_err("%s:read calibrate_thred fail\n", __func__);
		*calibrate_thred = 0;
		*(calibrate_thred + CALIBRATE_THRED_1) = 0;
		*(calibrate_thred + CALIBRATE_THRED_2) = 0;
		*(calibrate_thred + CALIBRATE_THRED_3) = 0;
	}

	hwlog_info("calibrate_thred:%u %u %u %u\n",
		*calibrate_thred, *(calibrate_thred + CALIBRATE_THRED_1),
		*(calibrate_thred + CALIBRATE_THRED_2),
		*(calibrate_thred + CALIBRATE_THRED_3));
}


void read_capprox_data_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	char *chip_info = get_sensor_chip_info_address(CAP_PROX);

	read_chip_info(dn, CAP_PROX);
	read_cap_prox_config1_from_dts(dn, sm);
	read_cap_prox_config2_from_dts(dn, sm);

	if (!strncmp(chip_info, "huawei,cypress_sar_psoc4000",
		strlen("huawei,cypress_sar_psoc4000"))) {
		read_cap_prox_cypress_from_dts(dn, sm);
	} else if (!strncmp(chip_info, "huawei,adi-adux1050",
		strlen("huawei,adi-adux1050"))) {
		read_cap_prox_adi1_from_dts(dn, sm);
		read_cap_prox_adi2_from_dts(dn, sm);
	} else if (!strncmp(chip_info, "huawei,semtech-sx9323",
		strlen("huawei,semtech-sx9323"))) {
		read_cap_prox_semtech1_from_dts(dn, sm);
		read_cap_prox_semtech2_from_dts(dn, sm);
	} else if (!strncmp(chip_info, "huawei,abov-a96t3x6",
		strlen("huawei,abov-a96t3x6"))) {
		read_abov_sar_data_from_dts(dn);
	} else if (!strncmp(chip_info, "huawei,awi-aw9610x",
		strlen("huawei,awi-aw9610x"))) {
		read_cap_prox_aw96101_from_dts(dn, sm);
		read_cap_prox_aw96102_from_dts(dn, sm);
	}

	read_sensorlist_info(dn, CAP_PROX);
}

static int is_cap_prox_shared_with_sar(struct device_node *dn)
{
	int sar_shared_flag = 0;
	int ret;
	int i2c_address = 0;
	int i2c_bus_num = 0;

	ret = of_property_read_u32(dn, "shared_flag", &sar_shared_flag);
	if (!ret && sar_shared_flag) {
		hwlog_info("sar chip shared with key\n");
		if (of_property_read_u32(dn, "bus_number", &i2c_bus_num) ||
			of_property_read_u32(dn, "reg", &i2c_address))
			hwlog_err("read sar sensor bus or reg err\n");

		sar_pdata.cfg.bus_num = (uint8_t)i2c_bus_num;
		sar_pdata.cfg.i2c_address = (uint8_t)i2c_address;
		return 0;
	}
	return -1;
}

int cap_prox_sensor_detect(struct device_node *dn,
	struct sensor_detect_manager *sm, int index)
{
	int ret;
	struct sensor_detect_manager *p = NULL;
	struct sensor_combo_cfg cfg = { 0 };

	if (!dn || !sm)
		return RET_FAIL;

	ret = is_cap_prox_shared_with_sar(dn);
	if (!ret)
		return ret;

	ret = _device_detect(dn, index, &cfg);
	if (!ret) {
		p = sm + index;
		if (memcpy_s((void *)p->spara, sizeof(struct sensor_combo_cfg),
			(void *)&cfg, sizeof(struct sensor_combo_cfg)) != EOK)
			ret = RET_FAIL;
	}
	return ret;
}

void cap_prox_detect_init(struct sensor_detect_manager *sm, uint32_t len)
{
	struct sensor_detect_manager *p = NULL;

	if (len <= CAP_PROX) {
		hwlog_err("%s:len=%d\n", __func__, len);
		return;
	}

	p = sm + CAP_PROX;
	p->spara = (const void *)&sar_pdata;
	p->cfg_data_length = sizeof(sar_pdata);
}

