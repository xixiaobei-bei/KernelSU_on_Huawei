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

#define RET_FAIL              (-1)

#define CAP_PROX_DEVICE_ID_0 0
#define CAP_PROX_DEVICE_ID_1 1
#define CAP_SENSOR_DEFAULT_ID "65541"

static char *cap_sensor_id = CAP_SENSOR_DEFAULT_ID;
static char *sar_calibrate_order = NULL;

static struct sar_platform_data sar_pdata[CAP_PROX_DEV_COUNT_MAX] = {
	{
	.cfg = DEF_SENSOR_COM_SETTING,
	.poll_interval = 200, /* 200 ms */
	.calibrate_type = 5, /* 5: calibrate type */
	.gpio_int = 0,
	},
};
static struct cap_prox_device_info g_cap_prox_dev_info[CAP_PROX_DEV_COUNT_MAX];

struct sar_sensor_detect cap_prox_detect_list[] = {
	{
		.chip_name = "huawei,semtech-sx9323",
		.cfg = DEF_SENSOR_COM_SETTING,
		.detect_flag = 0,
		.chip_id = 0,
	},
	{
		.chip_name = "huawei,abov-a96t3x6",
		.cfg = DEF_SENSOR_COM_SETTING,
		.detect_flag = 0,
		.chip_id = 0,
	},
	{
		.chip_name = "huawei,awi-aw9610x",
		.cfg = DEF_SENSOR_COM_SETTING,
		.detect_flag = 0,
		.chip_id = 0,
	},
};

struct sar_sensor_detect cap_prox1_detect_list[] = {
	{
		.chip_name = "huawei,semtech-sx9323",
		.cfg = DEF_SENSOR_COM_SETTING,
		.detect_flag = 0,
		.chip_id = 0,
	},
	{
		.chip_name = "huawei,abov-a96t3x6",
		.cfg = DEF_SENSOR_COM_SETTING,
		.detect_flag = 0,
		.chip_id = 0,
	},
	{
		.chip_name = "huawei,semtech-sx9335",
		.cfg = DEF_SENSOR_COM_SETTING,
		.detect_flag = 0,
		.chip_id = 0,
	},
};

struct sar_platform_data *cap_prox_get_platform_data(int32_t tag)
{
	if (tag == TAG_CAP_PROX)
		return (&(sar_pdata[CAP_PROX_DEVICE_ID_0]));
	else if (tag == TAG_CAP_PROX1)
		return (&(sar_pdata[CAP_PROX_DEVICE_ID_1]));

	hwlog_info("%s error, please check tag %d\n", __func__, tag);
	return NULL;
}

struct cap_prox_device_info *cap_prox_get_device_info(int32_t tag)
{
	if (tag == TAG_CAP_PROX)
		return (&(g_cap_prox_dev_info[CAP_PROX_DEVICE_ID_0]));
	else if (tag == TAG_CAP_PROX1)
		return (&(g_cap_prox_dev_info[CAP_PROX_DEVICE_ID_1]));

	hwlog_info("%s error, please check tag %d\n", __func__, tag);
	return NULL;
}

char *cap_prox_get_sensors_id_string(void)
{
	return cap_sensor_id;
}

void cap_prox_get_sensors_id_from_dts(struct device_node *dn)
{
	if (dn == NULL)
		return;
	if (of_property_read_string(dn,
		"cap_sensor_id", (const char **)&cap_sensor_id)) {
		cap_sensor_id = CAP_SENSOR_DEFAULT_ID;
		hwlog_err("cap_sensor_id err\n");
	}
	hwlog_info("cap_sensor_id:%s\n", cap_sensor_id);
}

char *cap_prox_get_calibrate_order_string(void)
{
	return sar_calibrate_order;
}

void read_abov1_sar_data_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	struct sar_platform_data *pf_data =
		(struct sar_platform_data *)sm->spara;
	uint16_t abov_phone_type = 0;
	uint16_t abov_project_id = 0;
	uint32_t ph = 0;
	uint16_t *threshold_to_modem = NULL;

	read_chip_info(dn, sm->sensor_id);
	threshold_to_modem = pf_data->sar_datas.abov_data.threshold_to_modem;
	if (of_property_read_u16_array(dn, "to_modem_threshold",
		threshold_to_modem, CAP_MODEM_THRESHOLE_LEN)) {
		*threshold_to_modem = 0xC8; /* 0xC8: default modem_threshold */
		*(threshold_to_modem + 1) = 0;
		hwlog_info("read threshold_to_modem fail\n");
	}
	hwlog_info("read threshold_to_modem %u %u %u\n",
		*threshold_to_modem, *(threshold_to_modem + 1),
		*(threshold_to_modem + (CAP_MODEM_THRESHOLE_LEN - 1)));

	if (of_property_read_u16(dn, "phone_type", &abov_phone_type)) {
		pf_data->sar_datas.abov_data.phone_type = 0;
		hwlog_err("%s:read phone_type fail\n", __func__);
	} else {
		pf_data->sar_datas.abov_data.phone_type = abov_phone_type;
		hwlog_info("%s:read phone_type:0x%x\n",
			__func__, pf_data->sar_datas.abov_data.phone_type);
	}

	if (of_property_read_u16(dn, "abov_project_id", &abov_project_id)) {
		pf_data->sar_datas.abov_data.abov_project_id = 0;
		hwlog_err("%s:read project id fail\n", __func__);
	} else {
		pf_data->sar_datas.abov_data.abov_project_id = abov_project_id;
		hwlog_info("%s:read project id:0x%x\n", __func__,
			pf_data->sar_datas.abov_data.abov_project_id);
	}

	if (of_property_read_u32(dn, "ph", &ph)) {
		pf_data->sar_datas.abov_data.ph = 0; /* 0: enable ph0 */
		hwlog_err("%s:read ph fail\n", __func__);
	} else {
		pf_data->sar_datas.abov_data.ph = (uint8_t)ph;
		hwlog_info("%s:read ph:0x%x\n",
			__func__, pf_data->sar_datas.abov_data.ph);
	}
}

void read_abov2_sar_data_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	struct sar_platform_data *pf_data =
		(struct sar_platform_data *)sm->spara;
	uint16_t *calibrate_thred = NULL;

	calibrate_thred = pf_data->sar_datas.abov_data.calibrate_thred;
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

void read_awi_sar_data_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	struct sar_platform_data *pf_data =
		(struct sar_platform_data *)sm->spara;
	uint16_t *calibrate_thred = NULL;
	uint16_t temp = 0;
	uint16_t phone_type = 0;

	calibrate_thred = pf_data->sar_datas.awi_data.calibrate_thred;
	if (of_property_read_u16_array(dn, "calibrate_thred",
		calibrate_thred, CAP_CALIBRATE_THRESHOLE_LEN))
		hwlog_err("%s:read calibrate_thred fail\n", __func__);

	if (of_property_read_u16(dn, "to_modem_threshold",
		&temp))
		hwlog_info("read threshold_to_modem fail\n");
	pf_data->sar_datas.awi_data.threshold_to_modem = temp;

	if (of_property_read_u16(dn, "phone_type", &phone_type)) {
		pf_data->sar_datas.awi_data.phone_type = 0;
		hwlog_err("%s:read %s phone_type fail\n",
			__func__, sm->sensor_name_str);
	} else {
		pf_data->sar_datas.awi_data.phone_type = phone_type;
		hwlog_info("%s:read %s phone_type:0x%x\n",
			__func__, sm->sensor_name_str,
			pf_data->sar_datas.awi_data.phone_type);
	}
}


static void read_cap_prox_config1_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	struct sar_platform_data *pf_data =
		(struct sar_platform_data *)sm->spara;
	int temp;
	int aux = 0;

	temp = of_get_named_gpio(dn, "gpio_int", 0);
	if (temp < 0)
		hwlog_err("%s:read %s gpio_int fail\n", __func__,
			sm->sensor_name_str);
	else
		pf_data->gpio_int = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "gpio_int_sh", &temp))
		hwlog_err("%s:read %s gpio_int_sh fail\n", __func__,
			sm->sensor_name_str);
	else
		pf_data->gpio_int_sh = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "reg", &temp))
		hwlog_err("%s:read cap_prox reg fail\n", __func__);
	else
		pf_data->cfg.i2c_address = (uint8_t)temp;
	if (of_property_read_u32(dn, "poll_interval", &temp))
		hwlog_err("%s:read %s poll_interval fail\n",
			__func__, sm->sensor_name_str);
	else
		pf_data->poll_interval = (uint16_t)temp;

	hwlog_info("sar.poll_interval: %d\n", pf_data->poll_interval);

	if (of_property_read_u32(dn, "aux_prop", &aux))
		hwlog_err("%s:read %s aux_prop fail\n", __func__,
			sm->sensor_name_str);
	else
		hwlog_info("%s:read %s aux_prop %d\n", __func__,
			sm->sensor_name_str, aux);

	if (of_property_read_u32(dn, "file_id", &temp))
		hwlog_err("%s:read %s file_id fail\n", __func__,
			sm->sensor_name_str);
	else if (aux)
		read_aux_file_list(temp, sm->tag);
	else
		read_dyn_file_list((uint16_t)temp);

	if (of_property_read_u32(dn, "sensor_list_info_id", &temp))
		hwlog_err("%s:read %s sar sensor_list_info_id fail\n", __func__,
			sm->sensor_name_str);
	else
		add_sensor_list_info_id((uint16_t)temp);
}

static void read_cap_prox_config2_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	struct sar_platform_data *pf_data =
		(struct sar_platform_data *)sm->spara;
	int temp = 0;

	if (of_property_read_u32(dn, "calibrate_type", &temp))
		hwlog_err("%s:read %s sar calibrate_type fail\n", __func__,
			sm->sensor_name_str);
	else
		pf_data->calibrate_type = (uint16_t)temp;

	if (of_property_read_string(dn, "calibrate_order",
		(const char **)&sar_calibrate_order))
		hwlog_err("read calibrate order err\n");

	hwlog_info("calibrate order:%s\n", sar_calibrate_order);
}

static void read_cap_prox_semtech1_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	struct sar_platform_data *pf_data =
		(struct sar_platform_data *)sm->spara;
	uint16_t temp16 = 0;
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

	if (of_property_read_u16(dn, "to_ap_threshold", &temp16))
		pf_data->sar_datas.semteck_data.threshold_to_ap = 0xC8;
	else
		pf_data->sar_datas.semteck_data.threshold_to_ap = temp16;

	threshold_to_modem =
		pf_data->sar_datas.semteck_data.threshold_to_modem;
	if (of_property_read_u16_array(dn, "to_modem_threshold",
		threshold_to_modem, 8)) { /* 8 x u16 */
		*threshold_to_modem =  0xC8;
		*(threshold_to_modem+1) = 0;
		hwlog_info("read threshold_to_modem fail\n");
	}
	hwlog_info("read threshold_to_modem %u %u %u\n",
		*threshold_to_modem, *(threshold_to_modem + 1),
		*(threshold_to_modem + (CAP_MODEM_THRESHOLE_LEN - 1)));

	init_reg_val = pf_data->sar_datas.semteck_data.init_reg_val;
	if (of_property_read_u32_array(dn, "init_reg_val",
		init_reg_val, SEMTECH_REGS_NEED_INITIATED_NUM)) {
		hwlog_err("%s:read %s init_reg_val fail\n", __func__,
			sm->sensor_name_str);
		if (memcpy_s(init_reg_val,
			sizeof(init_reg_val_default),
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
	struct sar_platform_data *pf_data =
		(struct sar_platform_data *)sm->spara;
	uint16_t temp16 = 0;
	uint32_t ph = 0;
	uint16_t *calibrate_thred = NULL;

	if (of_property_read_u16(dn, "phone_type", &temp16)) {
		pf_data->sar_datas.semteck_data.phone_type = 0;
		hwlog_err("%s:read %s phone_type fail\n",
			__func__, sm->sensor_name_str);
	} else {
		pf_data->sar_datas.semteck_data.phone_type = temp16;
		hwlog_info("%s:read %s phone_type:0x%x\n",
			__func__, sm->sensor_name_str,
			pf_data->sar_datas.semteck_data.phone_type);
	}

	if (of_property_read_u32(dn, "ph", &ph)) {
		pf_data->sar_datas.semteck_data.ph = 0x2f;
		hwlog_err("%s:read %s ph fail\n",
			__func__, sm->sensor_name_str);
	} else {
		pf_data->sar_datas.semteck_data.ph = (uint8_t)ph;
		hwlog_info("%s:read %s ph:0x%x\n",
			__func__, sm->sensor_name_str,
			pf_data->sar_datas.semteck_data.ph);
	}

	calibrate_thred = pf_data->sar_datas.semteck_data.calibrate_thred;
	if (of_property_read_u16_array(dn, "calibrate_thred",
		calibrate_thred, 4)) { /* 4 x u16 */
		hwlog_err("%s:read %s calibrate_thred fail\n",
			__func__, sm->sensor_name_str);
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

void read_capprox_data_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	read_chip_info(dn, sm->sensor_id);
	read_cap_prox_config1_from_dts(dn, sm);
	read_cap_prox_config2_from_dts(dn, sm);

	if (!strncmp(get_sensor_chip_info_address(sm->sensor_id),
		"huawei,semtech-sx9323",
		strlen("huawei,semtech-sx9323"))) {
		read_cap_prox_semtech1_from_dts(dn, sm);
		read_cap_prox_semtech2_from_dts(dn, sm);
	} else if (!strncmp(get_sensor_chip_info_address(CAP_PROX),
		"huawei,abov-a96t3x6",
		strlen("huawei,abov-a96t3x6"))) {
		read_abov1_sar_data_from_dts(dn, sm);
		read_abov2_sar_data_from_dts(dn, sm);
	} else if (!strncmp(get_sensor_chip_info_address(CAP_PROX),
		"huawei,awi-aw9610x",
		strlen("huawei,awi-aw9610x"))) {
		read_awi_sar_data_from_dts(dn, sm);
	}

	read_sensorlist_info(dn, sm->sensor_id);
}

static int is_cap_prox_shared_with_sar(struct device_node *dn)
{
	struct sar_platform_data *pf_data = &(sar_pdata[CAP_PROX_DEVICE_ID_0]);
	int sar_shared_flag = 0;
	int ret;
	int i2c_address = 0;
	int i2c_bus_num = 0;

	ret = of_property_read_u32(dn, "shared_flag", &sar_shared_flag);
	if (!ret && sar_shared_flag) {
		hwlog_info("sar chip shared with key\n");
		if (of_property_read_u32(dn, "bus_number", &i2c_bus_num) ||
		    of_property_read_u32(dn, "reg", &i2c_address))
			hwlog_err("read sar sensor bus/reg err\n");

		pf_data->cfg.bus_num = (uint8_t)i2c_bus_num;
		pf_data->cfg.i2c_address = (uint8_t)i2c_address;
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

void read_cap_prox1_info(struct device_node *dn)
{
	int register_add = 0;
	int i2c_address = 0;
	int i2c_bus_num = 0;
	u32 wia[CAP_CHIPID_DATA_LENGTH] = { 0 };
	char *chip_info = NULL;
	int i;

	if (of_property_read_u32(dn, "bus_number", &i2c_bus_num) ||
		of_property_read_u32(dn, "reg", &i2c_address) ||
		of_property_read_u32(dn, "chip_id_register", &register_add))
		hwlog_err("sar sensor :read i2c bus_number or bus address orchip_id_register from dts fail\n");

	if (of_property_read_string(dn, "compatible",
		(const char **)&chip_info))
		hwlog_err("%s:read name_id:CAP_PROX1 info fail\n", __func__);

	if (of_property_read_u32_array(dn, "chip_id_value",
		wia, CAP_CHIPID_DATA_LENGTH))
		hwlog_err("sar1 sensor:read chip_id_value from dts fail\n");

	for (i = 0; i < ARRAY_SIZE(cap_prox1_detect_list); i++) {
		if (!strncmp(chip_info, cap_prox1_detect_list[i].chip_name,
		strlen(cap_prox1_detect_list[i].chip_name))) {
			hwlog_debug("%s:cap_prox1 sensor from dts is %s\n",
				__func__, cap_prox1_detect_list[i].chip_name);
			cap_prox1_detect_list[i].detect_flag = 1;
			cap_prox1_detect_list[i].cfg.bus_num = (uint8_t)i2c_bus_num;
			cap_prox1_detect_list[i].cfg.i2c_address = (uint8_t)i2c_address;
			cap_prox1_detect_list[i].chip_id = (uint16_t)register_add;
			/* chipid value data len is 2 */
			cap_prox1_detect_list[i].chip_id_value[0] = (uint32_t)wia[0];
			cap_prox1_detect_list[i].chip_id_value[1] = (uint32_t)wia[1];
		}
	}
}

void read_cap_prox_info(struct device_node *dn)
{
	int register_add = 0;
	int i2c_address = 0;
	int i2c_bus_num = 0;
	u32 wia[2] = { 0 };
	char *chip_info = NULL;
	int i;
	struct property *prop = NULL;

	if (of_property_read_u32(dn, "bus_number", &i2c_bus_num) ||
		of_property_read_u32(dn, "reg", &i2c_address) ||
		of_property_read_u32(dn, "chip_id_register", &register_add))
		hwlog_err("sar sensor :read i2c bus_number %d or bus address %x or chip_id_register %x from dts fail\n",
			i2c_bus_num, i2c_address, register_add);

	prop = of_find_property(dn, "chip_id_value", NULL);
	if (!prop) {
		hwlog_err("%s not find property chip_id_value\n", __func__);
		return;
	}
	if (!prop->value)
		return;

	if (of_property_read_u32_array(dn, "chip_id_value", wia, prop->length / sizeof(u32)))
		hwlog_err("sar sensor:read chip_id_value id0= 0x%x id1=0x%x from dts fail\n",
			wia[0], wia[1]);

	if (of_property_read_string(dn, "compatible", (const char **)&chip_info))
		hwlog_err("%s:read name_id:CAP_PROX info fail\n", __func__);

	for (i = 0; i < ARRAY_SIZE(cap_prox_detect_list); i++) {
		if (!strncmp(chip_info, cap_prox_detect_list[i].chip_name,
		strlen(cap_prox_detect_list[i].chip_name))) {
			hwlog_info("%s:cap_prox sensor from dts is %s\n",
				__func__, cap_prox_detect_list[i].chip_name);
			cap_prox_detect_list[i].detect_flag = 1;
			cap_prox_detect_list[i].cfg.bus_num = (uint8_t)i2c_bus_num;
			cap_prox_detect_list[i].cfg.i2c_address = (uint8_t)i2c_address;
			cap_prox_detect_list[i].chip_id = (uint16_t)register_add;
			cap_prox_detect_list[i].chip_id_value[0] = (uint32_t)wia[0];
			cap_prox_detect_list[i].chip_id_value[1] = (uint32_t)wia[1];
		}
	}
}

static unsigned int sar_sensor_i2c_detect(struct sar_sensor_detect sar_detect)
{
	unsigned int detect_result;
	int ret;
	uint8_t bus_num;
	uint32_t i2c_address;
	uint16_t reg_add;
	uint8_t buf_temp[sizeof(uint32_t)] = { 0 };
	uint32_t register_add_len;
	uint32_t rx_len;
	uint32_t chip_id_value;

	/* ##(bus_num)##(i2c_addr)##(reg_addr)##(len) */
	bus_num = sar_detect.cfg.bus_num;
	i2c_address = sar_detect.cfg.i2c_address;
	reg_add = sar_detect.chip_id;
	buf_temp[0] = 0;

	if (reg_add & 0xFF00) {
		register_add_len = sizeof(uint16_t); /* reg addr is 16bit */
		rx_len = sizeof(uint32_t);
	} else {
		register_add_len = 1; /* reg addr is 8bit */
		rx_len = 1;
	}

	/*
	 * static int mcu_i2c_rw(uint8_t bus_num, uint8_t i2c_add,
	 * uint8_t register_add,uint8_t *rw, int len, uint8_t *buf)
	 */
	ret = mcu_i2c_rw(TAG_I2C, bus_num, i2c_address, (uint8_t *)&reg_add,
		register_add_len, &buf_temp[0], rx_len);
	if (ret < 0) {
		hwlog_err("In %s! i2c read reg fail\n", __func__);
		return 0;
	}

	if (memcpy_s(&chip_id_value, sizeof(chip_id_value),
		buf_temp, sizeof(buf_temp)) != EOK)
		return 0;

	if ((chip_id_value == sar_detect.chip_id_value[0]) ||
		(chip_id_value == sar_detect.chip_id_value[1])) {
		detect_result = 1;
		hwlog_info("sar_sensor_detect_succ 0x%8x\n", chip_id_value);
	} else {
		detect_result = 0;
		hwlog_info("sar_sensor_detect_fail 0x%8x\n", chip_id_value);
	}
	return detect_result;
}

unsigned int get_sar_detect_result(void)
{
	int i;
	unsigned int detect_result;
	unsigned int sar_detect_result = 0;

	for (i = 0; i < ARRAY_SIZE(cap_prox_detect_list); i++) {
		if (cap_prox_detect_list[i].detect_flag == 1) {
			detect_result = sar_sensor_i2c_detect(cap_prox_detect_list[i]);
			if (detect_result == 1)
				hwlog_info("%s %s detected\n",
					__func__, cap_prox_detect_list[i].chip_name);
			sar_detect_result |= detect_result;
		}
	}

	return sar_detect_result;
}

unsigned int get_sar_aux_detect_result(void)
{
	int i;
	unsigned int sar_aux_detect_result = 0;
	unsigned int detect_result;

	for (i = 0; i < ARRAY_SIZE(cap_prox1_detect_list); i++) {
		if (cap_prox1_detect_list[i].detect_flag == 1) {
			detect_result = sar_sensor_i2c_detect(cap_prox1_detect_list[i]);
			if (detect_result == 1)
				hwlog_info("%s %s detected\n",
					__func__, cap_prox1_detect_list[i].chip_name);
			sar_aux_detect_result |= detect_result;
		}
	}

	return sar_aux_detect_result;
}

void cap_prox_detect_init(struct sensor_detect_manager *sm, uint32_t len)
{
	struct sensor_detect_manager *p = NULL;
	int32_t i;

	if (len <= CAP_PROX1) {
		hwlog_err("%s:len=%d\n", __func__, len);
		return;
	}
	g_cap_prox_dev_info[CAP_PROX_DEVICE_ID_0].obj_tag = TAG_CAP_PROX;
	g_cap_prox_dev_info[CAP_PROX_DEVICE_ID_1].obj_tag = TAG_CAP_PROX1;
	g_cap_prox_dev_info[CAP_PROX_DEVICE_ID_0].detect_list_id = CAP_PROX;
	g_cap_prox_dev_info[CAP_PROX_DEVICE_ID_1].detect_list_id = CAP_PROX1;
	for (i = 0; i < CAP_PROX_DEV_COUNT_MAX; i++) {
		g_cap_prox_dev_info[i].cap_prox_dev_index = i;
		p = sm + g_cap_prox_dev_info[i].detect_list_id;
		p->spara = (const void *)&(sar_pdata[i]);
		p->cfg_data_length = sizeof(sar_pdata[i]);
		if (i == 0)
			continue;
		if (memcpy_s(&(sar_pdata[i]), sizeof(sar_pdata[i]),
			&(sar_pdata[0]), sizeof(sar_pdata[0])) != EOK)
			hwlog_info("%s: memcpy_s error i=%d\n", __func__, i);
	}
}

