/*
 * rt9759.c
 *
 * rt9759 driver
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

#include "rt9759.h"
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <chipset_common/hwpower/common_module/power_algorithm.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <chipset_common/hwpower/common_module/power_i2c.h>
#include <chipset_common/hwpower/common_module/power_log.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_thermalzone.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_device_id.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_ic_interface.h>

#define HWLOG_TAG rt9759
HWLOG_REGIST();

static int rt9759_write_byte(struct rt9759_device_info *di, u8 reg, u8 value)
{
	if (!di || (di->chip_already_init == 0)) {
		hwlog_err("chip not init\n");
		return -EIO;
	}

	return power_i2c_u8_write_byte(di->client, reg, value);
}

static int rt9759_read_byte(struct rt9759_device_info *di, u8 reg, u8 *value)
{
	if (!di || (di->chip_already_init == 0)) {
		hwlog_err("chip not init\n");
		return -EIO;
	}

	return power_i2c_u8_read_byte(di->client, reg, value);
}

static int rt9759_read_word(struct rt9759_device_info *di, u8 reg, s16 *value)
{
	u16 data = 0;

	if (!di || (di->chip_already_init == 0)) {
		hwlog_err("chip not init\n");
		return -EIO;
	}

	if (power_i2c_u8_read_word(di->client, reg, &data, true))
		return -1;

	*value = (s16)data;
	return 0;
}

static int rt9759_write_mask(struct rt9759_device_info *di,
	u8 reg, u8 mask, u8 shift, u8 value)
{
	int ret;
	u8 val = 0;

	ret = rt9759_read_byte(di, reg, &val);
	if (ret < 0)
		return ret;

	val &= ~mask;
	val |= ((value << shift) & mask);

	return rt9759_write_byte(di, reg, val);
}

static void rt9759_dump_register(struct rt9759_device_info *di)
{
	u8 i;
	int ret;
	u8 val = 0;

	for (i = 0; i < RT9759_DEGLITCH_REG; ++i) {
		ret = rt9759_read_byte(di, i, &val);
		if (ret)
			hwlog_err("dump_register read fail\n");

		hwlog_info("reg [%x]=0x%x\n", i, val);
	}
}

static int rt9759_reg_reset(struct rt9759_device_info *di)
{
	int ret;
	u8 reg = 0;

	ret = rt9759_write_mask(di, RT9759_CONTROL_REG,
		RT9759_REG_RST_MASK, RT9759_REG_RST_SHIFT,
		RT9759_REG_RST_ENABLE);
	if (ret)
		return -1;

	ret = rt9759_read_byte(di, RT9759_CONTROL_REG, &reg);
	if (ret)
		return -1;

	hwlog_info("reg_reset [%x]=0x%x\n", RT9759_CONTROL_REG, reg);
	return 0;
}

static int rt9759_fault_clear(struct rt9759_device_info *di)
{
	int ret;
	u8 reg = 0;

	ret = rt9759_read_byte(di, RT9759_FLT_FLAG_REG, &reg);
	if (ret)
		return -1;

	hwlog_info("fault_flag [%x]=0x%x\n", RT9759_FLT_FLAG_REG, reg);
	return 0;
}

static int rt9759_charge_enable(int enable, void *dev_data)
{
	int ret;
	u8 reg = 0;
	u8 value = enable ? 0x1 : 0x0;
	struct rt9759_device_info *di = dev_data;

	if (!di)
		return -1;

	ret = rt9759_write_mask(di, RT9759_CHRG_CTL_REG,
		RT9759_CHARGE_EN_MASK, RT9759_CHARGE_EN_SHIFT,
		value);
	if (ret)
		return -1;

	ret = rt9759_read_byte(di, RT9759_CHRG_CTL_REG, &reg);
	if (ret)
		return -1;

	hwlog_info("charge_enable [%x]=0x%x\n", RT9759_CHRG_CTL_REG, reg);
	return 0;
}

static int rt9759_enable_adc(int enable, void *dev_data)
{
	int ret;
	u8 reg = 0;
	u8 value = enable ? 0x1 : 0x0;
	struct rt9759_device_info *di = dev_data;

	if (!di)
		return -1;

	ret = rt9759_write_mask(di, RT9759_ADC_CTRL_REG,
		RT9759_ADC_CTRL_EN_MASK, RT9759_ADC_CTRL_EN_SHIFT,
		value);
	if (ret)
		return -1;

	ret = rt9759_read_byte(di, RT9759_ADC_CTRL_REG, &reg);
	if (ret)
		return -1;

	hwlog_info("adc_enable [%x]=0x%x\n", RT9759_ADC_CTRL_REG, reg);
	return 0;
}

static bool rt9759_is_adc_disabled(struct rt9759_device_info *di)
{
	u8 reg = 0;
	int ret;

	ret = rt9759_read_byte(di, RT9759_ADC_CTRL_REG, &reg);
	if (ret || !(reg & RT9759_ADC_CTRL_EN_MASK))
		return true;

	return false;
}

#ifdef ENABLE_RT9759_DISCHARGE
static int rt9759_discharge(int enable, void *dev_data)
{
	int ret;
	u8 reg = 0;
	u8 value = enable ? 0x1 : 0x0;
	struct rt9759_device_info *di = dev_data;

	if (!di)
		return -1;

	ret = rt9759_write_mask(di, RT9759_BUS_OVP_REG,
		RT9759_VBUS_PD_EN_MASK, RT9759_VBUS_PD_EN_SHIFT, value);
	if (ret)
		return -1;

	ret = rt9759_read_byte(di, RT9759_BUS_OVP_REG, &reg);
	if (ret)
		return -1;

	hwlog_info("discharge [%x]=0x%x\n", RT9759_BUS_OVP_REG, reg);
	return 0;
}
#else
static int rt9759_discharge(int enable, void *dev_data)
{
	return 0;
}
#endif /* ENABLE_RT9759_DISCHARGE */

static int rt9759_is_device_close(void *dev_data)
{
	u8 reg = 0;
	int ret;
	struct rt9759_device_info *di = dev_data;

	if (!di)
		return -1;

	ret = rt9759_read_byte(di, RT9759_CHRG_CTL_REG, &reg);
	if (ret)
		return 1;

	if (reg & RT9759_CHARGE_EN_MASK)
		return 0;

	return 1;
}

static int rt9759_get_device_id(void *dev_data)
{
	u8 part_info = 0;
	int ret;
	struct rt9759_device_info *di = dev_data;

	if (!di)
		return -1;

	if (di->get_id_time == RT9759_USED)
		return di->device_id;

	di->get_id_time = RT9759_USED;
	ret = rt9759_read_byte(di, RT9759_PART_INFO_REG, &part_info);
	if (ret) {
		di->get_id_time = RT9759_NOT_USED;
		hwlog_err("get_device_id read fail\n");
		return -1;
	}

	hwlog_info("get_device_id [%x]=0x%x\n",
		RT9759_PART_INFO_REG, part_info);

	part_info = part_info & RT9759_DEVICE_ID_MASK;
	switch (part_info) {
	case RT9759_DEVICE_ID_RT9759:
		di->device_id = SWITCHCAP_RT9759;
		break;
	default:
		di->device_id = -1;
		hwlog_err("device id not match\n");
		break;
	}

	return di->device_id;
}

static int rt9759_get_vbat_mv(void *dev_data)
{
	s16 data = 0;
	int ret;
	int vbat;
	int vbat2;
	struct rt9759_device_info *di = dev_data;

	if (!di)
		return -1;

	if (rt9759_is_adc_disabled(di))
		return 0;

	ret = rt9759_read_word(di, RT9759_VBAT_ADC1_REG, &data);
	if (ret)
		return -1;

	hwlog_info("VBAT_ADC=0x%x\n", data);

	vbat = (int)(data);

	ret = rt9759_read_word(di, RT9759_VBAT_ADC1_REG, &data);
	if (ret)
		return -1;

	hwlog_info("VBAT_ADC=0x%x\n", data);

	vbat2 = (int)(data);

	return vbat < vbat2 ? vbat : vbat2;
}

static int rt9759_get_ibat_ma(int *ibat, void *dev_data)
{
	int ret;
	s16 data = 0;
	struct rt9759_device_info *di = dev_data;

	if (!ibat || !di)
		return -1;

	if (rt9759_is_adc_disabled(di)) {
		*ibat = 0;
		return 0;
	}

	ret = rt9759_read_word(di, RT9759_IBAT_ADC1_REG, &data);
	if (ret)
		return -1;

	hwlog_info("IBAT_ADC=0x%x\n", data);

	*ibat = ((int)data) * di->sense_r_config;
	*ibat /= di->sense_r_actual;

	return 0;
}

static int rt9759_get_ibus_ma(int *ibus, void *dev_data)
{
	s16 data = 0;
	int ret;
	struct rt9759_device_info *di = dev_data;

	if (!ibus || !di)
		return -1;

	if (rt9759_is_adc_disabled(di)) {
		*ibus = 0;
		return 0;
	}

	ret = rt9759_read_word(di, RT9759_IBUS_ADC1_REG, &data);
	if (ret)
		return -1;

	hwlog_info("IBUS_ADC=0x%x\n", data);

	*ibus = (int)(data);

	return 0;
}

static int rt9759_get_vbus_mv(int *vbus, void *dev_data)
{
	int ret;
	s16 data = 0;
	struct rt9759_device_info *di = dev_data;

	if (!vbus || !di)
		return -1;

	if (rt9759_is_adc_disabled(di)) {
		*vbus = 0;
		return 0;
	}

	ret = rt9759_read_word(di, RT9759_VBUS_ADC1_REG, &data);
	if (ret)
		return -1;

	hwlog_info("VBUS_ADC=0x%x\n", data);

	*vbus = (int)(data);

	return 0;
}

static int rt9759_is_tsbat_disabled(void *dev_data)
{
	u8 reg = 0;
	int ret;
	struct rt9759_device_info *di = dev_data;

	if (!di)
		return -1;

	ret = rt9759_read_byte(di, RT9759_CHRG_CTL_REG, &reg);
	if (ret)
		return -1;

	hwlog_info("is_tsbat_disabled [%x]=0x%x\n", RT9759_CHRG_CTL_REG, reg);

	if (reg & RT9759_TSBAT_DIS_MASK)
		return 0;

	return -1;
}

static int rt9759_get_device_temp(int *temp, void *dev_data)
{
	s16 data = 0;
	s16 temperature;
	int ret;
	struct rt9759_device_info *di = dev_data;

	if (!temp || !di)
		return -1;

	if (rt9759_is_adc_disabled(di)) {
		*temp = 0;
		return 0;
	}

	ret = rt9759_read_word(di, RT9759_TDIE_ADC1_REG, &data);
	if (ret)
		return -1;

	hwlog_info("TDIE_ADC=0x%x\n", data);

	temperature = data & ((RT9759_TDIE_ADC1_MASK <<
		RT9759_LENTH_OF_BYTE) | RT9759_LOW_BYTE_INIT);
	*temp = (int)(temperature / RT9759_TDIE_SCALE);

	return 0;
}

static int rt9759_get_vusb_mv(int *vusb, void *dev_data)
{
	int ret;
	s16 data = 0;
	struct rt9759_device_info *di = dev_data;

	if (!vusb || !di)
		return -1;

	if (rt9759_is_adc_disabled(di)) {
		*vusb = 0;
		return 0;
	}

	ret = rt9759_read_word(di, RT9759_VAC_ADC1_REG, &data);
	if (ret)
		return -1;

	hwlog_info("VAC_ADC=0x%x\n", data);

	*vusb = (int)(data);

	return 0;
}

static int rt9759_get_vout_mv(int *vout, void *dev_data)
{
	int ret;
	s16 data = 0;
	struct rt9759_device_info *di = dev_data;

	if (!vout || !di)
		return -1;

	if (rt9759_is_adc_disabled(di)) {
		*vout = 0;
		return 0;
	}

	ret = rt9759_read_word(di, RT9759_VOUT_ADC1_REG, &data);
	if (ret)
		return -1;

	hwlog_info("VOUT_ADC=0x%x\n", data);

	*vout = (int)(data);

	return 0;
}

static int rt9759_get_register_head(char *buffer, int size, void *dev_data)
{
	struct rt9759_device_info *di = dev_data;

	if (!buffer || !di)
		return -1;

	if (di->ic_role == CHARGE_IC_TYPE_MAIN)
		snprintf(buffer, size,
			"   Ibus   Vbus   Ibat   Vusb   Vout   Vbat   Temp");
	else
		snprintf(buffer, size,
			"   Ibus1  Vbus1  Ibat1  Vusb1  Vout1  Vbat1  Temp1");

	return 0;
}

static int rt9759_value_dump(char *buffer, int size, void *dev_data)
{
	int ibus = 0;
	int vbus = 0;
	int ibat = 0;
	int vusb = 0;
	int vout = 0;
	int temp = 0;
	struct rt9759_device_info *di = dev_data;

	if (!buffer || !di)
		return -1;

	(void)rt9759_get_ibus_ma(&ibus, dev_data);
	(void)rt9759_get_vbus_mv(&vbus, dev_data);
	(void)rt9759_get_ibat_ma(&ibat, dev_data);
	(void)rt9759_get_vusb_mv(&vusb, dev_data);
	(void)rt9759_get_vout_mv(&vout, dev_data);
	(void)rt9759_get_device_temp(&temp, dev_data);

	if (di->ic_role == CHARGE_IC_TYPE_MAIN)
		snprintf(buffer, size,
			"     %-7d%-7d%-7d%-7d%-7d%-7d%-7d",
			ibus, vbus, ibat, vusb, vout,
			rt9759_get_vbat_mv(dev_data), temp);
	else
		snprintf(buffer, size,
			"%-7d%-7d%-7d%-7d%-7d%-7d%-7d  ",
			ibus, vbus, ibat, vusb, vout,
			rt9759_get_vbat_mv(dev_data), temp);

	return 0;
}

/*
 * rt9759 probability reset after vbus-gnd short circuit,
 * then we need disable watchdog to stop reset again
 */
static void rt9759_disable_watchdog(struct rt9759_device_info *di)
{
	u8 control_reg = 0;

	if (rt9759_read_byte(di, RT9759_CONTROL_REG, &control_reg))
		return;

	if (control_reg & RT9759_WATCHDOG_DIS_MASK)
		return;

	hwlog_info("watchdog is enable, need disable\n");
	/* 1:disable watchdog */
	if (rt9759_write_mask(di, RT9759_CONTROL_REG, RT9759_WATCHDOG_DIS_MASK,
		RT9759_WATCHDOG_DIS_SHIFT, 1))
		hwlog_err("disable watchdog failed\n");
}

static int rt9759_config_watchdog_ms(int time, void *dev_data)
{
	u8 val;
	u8 reg;
	int ret;
	struct rt9759_device_info *di = dev_data;

	if (!di)
		return -1;

	if (time >= RT9759_WTD_CONFIG_TIMING_30000MS)
		val = RT9759_WTD_SET_30000MS;
	else if (time >= RT9759_WTD_CONFIG_TIMING_5000MS)
		val = RT9759_WTD_SET_30000MS;
	else if (time >= RT9759_WTD_CONFIG_TIMING_1000MS)
		val = RT9759_WTD_SET_30000MS;
	else
		val = RT9759_WTD_SET_30000MS;

	ret = rt9759_write_mask(di, RT9759_CONTROL_REG,
		RT9759_WATCHDOG_CONFIG_MASK, RT9759_WATCHDOG_CONFIG_SHIFT,
		val);
	if (ret)
		return -1;

	ret = rt9759_read_byte(di, RT9759_CONTROL_REG, &reg);
	if (ret)
		return -1;

	hwlog_info("config_watchdog_ms [%x]=0x%x\n", RT9759_CONTROL_REG, reg);

	return 0;
}

static int rt9759_config_vbat_ovp_threshold_mv(struct rt9759_device_info *di,
	int ovp_threshold)
{
	u8 value;
	int ret;

	if (ovp_threshold < RT9759_BAT_OVP_BASE_3500MV)
		ovp_threshold = RT9759_BAT_OVP_BASE_3500MV;

	if (ovp_threshold > RT9759_BAT_OVP_MAX_5075MV)
		ovp_threshold = RT9759_BAT_OVP_MAX_5075MV;

	value = (u8)((ovp_threshold - RT9759_BAT_OVP_BASE_3500MV) /
		RT9759_BAT_OVP_STEP);
	ret = rt9759_write_mask(di, RT9759_BAT_OVP_REG,
		RT9759_BAT_OVP_MASK, RT9759_BAT_OVP_SHIFT,
		value);
	if (ret)
		return -1;

	hwlog_info("config_vbat_ovp_threshold_mv [%x]=0x%x\n",
		RT9759_BAT_OVP_REG, value);

	return 0;
}

static int rt9759_config_ibat_ocp_threshold_ma(struct rt9759_device_info *di,
	int ocp_threshold)
{
	u8 value;
	int ret;

	if (ocp_threshold < RT9759_BAT_OCP_BASE_2000MA)
		ocp_threshold = RT9759_BAT_OCP_BASE_2000MA;

	if (ocp_threshold > RT9759_BAT_OCP_MAX_10000MA)
		ocp_threshold = RT9759_BAT_OCP_MAX_10000MA;

	value = (u8)((ocp_threshold - RT9759_BAT_OCP_BASE_2000MA) /
		RT9759_BAT_OCP_STEP);
	ret = rt9759_write_mask(di, RT9759_BAT_OCP_REG,
		RT9759_BAT_OCP_MASK, RT9759_BAT_OCP_SHIFT,
		value);
	if (ret)
		return -1;

	hwlog_info("config_ibat_ocp_threshold_ma [%x]=0x%x\n",
		RT9759_BAT_OCP_REG, value);

	return 0;
}

static int rt9759_config_ac_ovp_threshold_mv(struct rt9759_device_info *di,
	int ovp_threshold)
{
	u8 value;
	int ret;

	if (ovp_threshold < RT9759_AC_OVP_BASE_11000MV)
		ovp_threshold = RT9759_AC_OVP_BASE_11000MV;

	if (ovp_threshold > RT9759_AC_OVP_MAX_18000MV)
		ovp_threshold = RT9759_AC_OVP_MAX_18000MV;

	value = (u8)((ovp_threshold - RT9759_AC_OVP_BASE_11000MV) /
		RT9759_AC_OVP_STEP);
	ret = rt9759_write_mask(di, RT9759_AC_OVP_REG,
		RT9759_AC_OVP_MASK, RT9759_AC_OVP_SHIFT,
		value);
	if (ret)
		return -1;

	hwlog_info("config_ac_ovp_threshold_mv [%x]=0x%x\n",
		RT9759_AC_OVP_REG, value);

	return 0;
}

static int rt9759_config_vbus_ovp_threshold_mv(struct rt9759_device_info *di,
	int ovp_threshold)
{
	u8 value;
	int ret;

	if (ovp_threshold < RT9759_BUS_OVP_BASE_6000MV)
		ovp_threshold = RT9759_BUS_OVP_BASE_6000MV;

	if (ovp_threshold > RT9759_BUS_OVP_MAX_12350MV)
		ovp_threshold = RT9759_BUS_OVP_MAX_12350MV;

	value = (u8)((ovp_threshold - RT9759_BUS_OVP_BASE_6000MV) /
		RT9759_BUS_OVP_STEP);
	ret = rt9759_write_mask(di, RT9759_BUS_OVP_REG,
		RT9759_BUS_OVP_MASK, RT9759_BUS_OVP_SHIFT,
		value);
	if (ret)
		return -1;

	hwlog_info("config_vbus_ovp_threshold_mv [%x]=0x%x\n",
		RT9759_BUS_OVP_REG, value);

	return 0;
}

static int rt9759_config_ibus_ocp_threshold_ma(struct rt9759_device_info *di,
	int ocp_threshold)
{
	u8 value;
	int ret;

	if (ocp_threshold < RT9759_BUS_OCP_BASE_1000MA)
		ocp_threshold = RT9759_BUS_OCP_BASE_1000MA;

	if (ocp_threshold > RT9759_BUS_OCP_MAX_4750MA)
		ocp_threshold = RT9759_BUS_OCP_MAX_4750MA;

	value = (u8)((ocp_threshold - RT9759_BUS_OCP_BASE_1000MA) /
		RT9759_BUS_OCP_STEP);
	ret = rt9759_write_mask(di, RT9759_BUS_OCP_UCP_REG,
		RT9759_BUS_OCP_MASK, RT9759_BUS_OCP_SHIFT,
		value);
	if (ret)
		return -1;

	hwlog_info("config_ibus_ocp_threshold_ma [%x]=0x%x\n",
		RT9759_BUS_OCP_UCP_REG, value);

	return 0;
}

static int rt9759_config_switching_frequency(struct rt9759_device_info *di,
	int data)
{
	int freq;
	int freq_shift;
	int ret;

	switch (data) {
	case RT9759_SW_FREQ_450KHZ:
		freq = RT9759_FSW_SET_SW_FREQ_500KHZ;
		freq_shift = RT9759_SW_FREQ_SHIFT_M_P10;
		break;
	case RT9759_SW_FREQ_500KHZ:
		freq = RT9759_FSW_SET_SW_FREQ_500KHZ;
		freq_shift = RT9759_SW_FREQ_SHIFT_NORMAL;
		break;
	case RT9759_SW_FREQ_550KHZ:
		freq = RT9759_FSW_SET_SW_FREQ_500KHZ;
		freq_shift = RT9759_SW_FREQ_SHIFT_P_P10;
		break;
	case RT9759_SW_FREQ_675KHZ:
		freq = RT9759_FSW_SET_SW_FREQ_750KHZ;
		freq_shift = RT9759_SW_FREQ_SHIFT_M_P10;
		break;
	case RT9759_SW_FREQ_750KHZ:
		freq = RT9759_FSW_SET_SW_FREQ_750KHZ;
		freq_shift = RT9759_SW_FREQ_SHIFT_NORMAL;
		break;
	case RT9759_SW_FREQ_825KHZ:
		freq = RT9759_FSW_SET_SW_FREQ_750KHZ;
		freq_shift = RT9759_SW_FREQ_SHIFT_P_P10;
		break;
	default:
		freq = RT9759_FSW_SET_SW_FREQ_500KHZ;
		freq_shift = RT9759_SW_FREQ_SHIFT_P_P10;
		break;
	}

	ret = rt9759_write_mask(di, RT9759_CONTROL_REG,
		RT9759_FSW_SET_MASK, RT9759_FSW_SET_SHIFT,
		freq);
	if (ret)
		return -1;

	ret = rt9759_write_mask(di, RT9759_CHRG_CTL_REG,
		RT9759_FREQ_SHIFT_MASK, RT9759_FREQ_SHIFT_SHIFT,
		freq_shift);
	if (ret)
		return -1;

	if (di->special_freq_shift)
		freq_shift = RT9759_SW_FREQ_SHIFT_MP_P10;

	hwlog_info("config_switching_frequency [%x]=0x%x, [%x]=0x%x\n",
		RT9759_CONTROL_REG, freq, RT9759_CHRG_CTL_REG, freq_shift);

	return 0;
}

static int rt9759_config_ibat_sns_res(struct rt9759_device_info *di, int data)
{
	int res_config;

	if (data == SENSE_R_2_MOHM)
		res_config = RT9759_IBAT_RSEN_2MOHM;
	else
		res_config = RT9759_IBAT_RSEN_5MOHM;

	return rt9759_write_mask(di, RT9759_REG_CTRL,
		RT9759_IBAT_RSEN_MASK, RT9759_IBAT_RSEN_SHIFT, res_config);
}

static int rt9759_config_ibus_low_deglitch(struct rt9759_device_info *di, int data)
{
	int ibus_config;

	if (data == IBUS_DEGLITCH_5MS)
		ibus_config = RT9759_IBUS_LOW_DEGLITCH_5MS;
	else
		ibus_config = RT9759_IBUS_LOW_DEGLITCH_10US;

	return rt9759_write_mask(di, RT9759_STAT_FLAG_REG,
		RT9759_IBUS_LOW_DEGLITCH_MASK, RT9759_IBUS_LOW_DEGLITCH_SHIFT,
		ibus_config);
}

static int rt9759_config_ucp_rise_threshold(struct rt9759_device_info *di, int data)
{
	int ucp_config;

	if (data == UCP_RISE_500MA)
		ucp_config = RT9759_UCP_RISE_500MA;
	else
		ucp_config = RT9759_UCP_RISE_300MA;

	return rt9759_write_mask(di, RT9759_REG_CTRL,
		RT9759_UCP_RISE_MASK, RT9759_UCP_RISE_SHIFT,
		ucp_config);
}

static void rt9759_close_vbat_protection(struct rt9759_device_info *di)
{
	rt9759_write_mask(di, RT9759_BAT_OVP_REG,
		RT9759_BAT_OVP_DIS_MASK, RT9759_BAT_OVP_DIS_SHIFT, 0);
	rt9759_write_mask(di, RT9759_FLT_MASK_REG,
		RT9759_BAT_OVP_FLT_MASK_MASK, RT9759_BAT_OVP_FLT_MASK_SHIFT, 1);
}

static int rt9759_reg_init(struct rt9759_device_info *di)
{
	int ret;

	ret = rt9759_write_byte(di, RT9759_CONTROL_REG,
		RT9759_CONTROL_REG_INIT);
	ret += rt9759_write_byte(di, RT9759_CHRG_CTL_REG,
		di->chrg_ctl_reg_init);
	ret += rt9759_write_byte(di, RT9759_INT_MASK_REG,
		RT9759_INT_MASK_REG_INIT);
	ret += rt9759_write_byte(di, RT9759_FLT_MASK_REG,
		RT9759_FLT_MASK_REG_INIT);
	ret += rt9759_write_byte(di, RT9759_ADC_CTRL_REG,
		RT9759_ADC_CTRL_REG_INIT);
	ret += rt9759_write_byte(di, RT9759_ADC_FN_DIS_REG,
		di->adc_fn_reg_init);
	ret += rt9759_write_mask(di, RT9759_BAT_OVP_ALM_REG,
		RT9759_BAT_OVP_ALM_DIS_MASK, RT9759_BAT_OVP_ALM_DIS_SHIFT,
		RT9759_ALM_DISABLE);
	ret += rt9759_write_mask(di, RT9759_BAT_OCP_ALM_REG,
		RT9759_BAT_OCP_ALM_DIS_MASK, RT9759_BAT_OCP_ALM_DIS_SHIFT,
		RT9759_ALM_DISABLE);
	ret += rt9759_write_mask(di, RT9759_BAT_UCP_ALM_REG,
		RT9759_BAT_UCP_ALM_DIS_MASK, RT9759_BAT_UCP_ALM_DIS_SHIFT,
		RT9759_ALM_DISABLE);
	ret += rt9759_write_mask(di, RT9759_BUS_OVP_REG,
		RT9759_VBUS_PD_EN_MASK, RT9759_VBUS_PD_EN_SHIFT,
		RT9759_VBUS_PD_EN_DISABLE);
	ret += rt9759_write_mask(di, RT9759_BUS_OVP_ALM_REG,
		RT9759_BUS_OVP_ALM_DIS_MASK, RT9759_BUS_OVP_ALM_DIS_SHIFT,
		RT9759_ALM_DISABLE);
	ret += rt9759_write_mask(di, RT9759_BUS_OCP_ALM_REG,
		RT9759_BUS_OCP_ALM_DIS_MASK, RT9759_BUS_OCP_ALM_DIS_SHIFT,
		RT9759_ALM_DISABLE);
	ret += rt9759_config_vbat_ovp_threshold_mv(di,
		RT9759_VBAT_OVP_THRESHOLD_INIT);
	ret += rt9759_config_ibat_ocp_threshold_ma(di,
		RT9759_IBAT_OCP_THRESHOLD_INIT);
	ret += rt9759_config_ac_ovp_threshold_mv(di,
		RT9759_AC_OVP_THRESHOLD_INIT);
	ret += rt9759_config_vbus_ovp_threshold_mv(di,
		RT9759_VBUS_OVP_THRESHOLD_INIT);
	ret += rt9759_config_ibus_ocp_threshold_ma(di,
		RT9759_IBUS_OCP_THRESHOLD_INIT);
	ret += rt9759_config_switching_frequency(di, di->switching_frequency);
	ret += rt9759_config_ibat_sns_res(di, di->sense_r_config);
	ret += rt9759_write_mask(di, RT9759_STAT_FLAG_REG,
		RT9759_VBUS_LOW_DEGLITCH_MASK, RT9759_VBUS_LOW_DEGLITCH_SHIFT,
		RT9759_VBUS_LOW_DEGLITCH_10MS);
	ret += rt9759_config_ibus_low_deglitch(di, di->ibus_low_deglitch);
	ret += rt9759_config_ucp_rise_threshold(di, di->ucp_rise_threshold);

	if (di->close_vbat_protection)
		rt9759_close_vbat_protection(di);

	return ret;
}

static int rt9759_charge_init(void *dev_data)
{
	struct rt9759_device_info *di = dev_data;

	if (!di)
		return -1;

	if (rt9759_reg_init(di))
		return -1;

	di->device_id = rt9759_get_device_id(dev_data);
	if (di->device_id == -1)
		return -1;

	hwlog_info("device id is %d\n", di->device_id);

	di->init_finish_flag = RT9759_INIT_FINISH;
	return 0;
}

static int rt9759_charge_exit(void *dev_data)
{
	int ret;
	struct rt9759_device_info *di = dev_data;

	if (!di)
		return -1;

	ret = rt9759_charge_enable(RT9759_SWITCHCAP_DISABLE, dev_data);

	di->init_finish_flag = RT9759_NOT_INIT;
	di->int_notify_enable_flag = RT9759_DISABLE_INT_NOTIFY;

	power_usleep(DT_USLEEP_10MS);

	return ret;
}

static int rt9759_batinfo_exit(void *dev_data)
{
	return 0;
}

static int rt9759_batinfo_init(void *dev_data)
{
	return 0;
}

static int rt9759_get_raw_data(int adc_channel, long *data, void *dev_data)
{
	int adc_value;
	struct rt9759_device_info *di = dev_data;
	struct adc_comp_data comp_data = { 0 };

	if (!di || !data)
		return -1;

	adc_value = power_platform_get_adc_sample(adc_channel);
	if (adc_value < 0)
		return -1;

	comp_data.adc_accuracy = di->adc_accuracy;
	comp_data.adc_v_ref = di->adc_v_ref;
	comp_data.v_pullup = di->v_pullup;
	comp_data.r_pullup = di->r_pullup;
	comp_data.r_comp = di->r_comp;

	*data = (long)power_get_adc_compensation_value(adc_value, &comp_data);
	if (*data < 0)
		return -1;

	return 0;
}

static struct dc_ic_ops rt9759_sysinfo_ops = {
	.dev_name = "rt9759",
	.ic_init = rt9759_charge_init,
	.ic_exit = rt9759_charge_exit,
	.ic_enable = rt9759_charge_enable,
	.ic_adc_enable = rt9759_enable_adc,
	.ic_discharge = rt9759_discharge,
	.is_ic_close = rt9759_is_device_close,
	.get_ic_id = rt9759_get_device_id,
	.config_ic_watchdog = rt9759_config_watchdog_ms,
	.get_ic_status = rt9759_is_tsbat_disabled,
};

static struct dc_batinfo_ops rt9759_batinfo_ops = {
	.init = rt9759_batinfo_init,
	.exit = rt9759_batinfo_exit,
	.get_bat_btb_voltage = rt9759_get_vbat_mv,
	.get_bat_package_voltage = rt9759_get_vbat_mv,
	.get_vbus_voltage = rt9759_get_vbus_mv,
	.get_bat_current = rt9759_get_ibat_ma,
	.get_ic_ibus = rt9759_get_ibus_ma,
	.get_ic_temp = rt9759_get_device_temp,
	.get_ic_vusb = rt9759_get_vusb_mv,
	.get_ic_vout = rt9759_get_vout_mv,
};

static struct power_tz_ops rt9759_temp_sensing_ops = {
	.get_raw_data = rt9759_get_raw_data,
};

static struct power_log_ops rt9759_log_ops = {
	.dev_name = "rt9759",
	.dump_log_head = rt9759_get_register_head,
	.dump_log_content = rt9759_value_dump,
};

static struct dc_ic_ops rt9759_aux_sysinfo_ops = {
	.dev_name = "rt9759_aux",
	.ic_init = rt9759_charge_init,
	.ic_exit = rt9759_charge_exit,
	.ic_enable = rt9759_charge_enable,
	.ic_adc_enable = rt9759_enable_adc,
	.ic_discharge = rt9759_discharge,
	.is_ic_close = rt9759_is_device_close,
	.get_ic_id = rt9759_get_device_id,
	.config_ic_watchdog = rt9759_config_watchdog_ms,
	.get_ic_status = rt9759_is_tsbat_disabled,
};

static struct dc_batinfo_ops rt9759_aux_batinfo_ops = {
	.init = rt9759_batinfo_init,
	.exit = rt9759_batinfo_exit,
	.get_bat_btb_voltage = rt9759_get_vbat_mv,
	.get_bat_package_voltage = rt9759_get_vbat_mv,
	.get_vbus_voltage = rt9759_get_vbus_mv,
	.get_bat_current = rt9759_get_ibat_ma,
	.get_ic_ibus = rt9759_get_ibus_ma,
	.get_ic_temp = rt9759_get_device_temp,
	.get_ic_vusb = rt9759_get_vusb_mv,
	.get_ic_vout = rt9759_get_vout_mv,
};

static struct power_tz_ops rt9759_aux_temp_sensing_ops = {
	.get_raw_data = rt9759_get_raw_data,
};

static struct power_log_ops rt9759_aux_log_ops = {
	.dev_name = "rt9759_aux",
	.dump_log_head = rt9759_get_register_head,
	.dump_log_content = rt9759_value_dump,
};

static void rt9759_init_ops_dev_data(struct rt9759_device_info *di)
{
	if (di->ic_role == CHARGE_IC_TYPE_MAIN) {
		rt9759_sysinfo_ops.dev_data = (void *)di;
		rt9759_batinfo_ops.dev_data = (void *)di;
		rt9759_temp_sensing_ops.dev_data = (void *)di;
		rt9759_log_ops.dev_data = (void *)di;
	} else {
		rt9759_aux_sysinfo_ops.dev_data = (void *)di;
		rt9759_aux_batinfo_ops.dev_data = (void *)di;
		rt9759_aux_temp_sensing_ops.dev_data = (void *)di;
		rt9759_aux_log_ops.dev_data = (void *)di;
	}
}

static int rt9759_ops_register(struct rt9759_device_info *di)
{
	int ret;

	rt9759_init_ops_dev_data(di);

	if (di->ic_role == CHARGE_IC_TYPE_MAIN) {
		ret = dc_ic_ops_register(SC_MODE, di->ic_role,
			&rt9759_sysinfo_ops);
		ret |= dc_batinfo_ops_register(SC_MODE, di->ic_role,
			&rt9759_batinfo_ops);
	} else {
		ret = dc_ic_ops_register(SC_MODE, di->ic_role,
			&rt9759_aux_sysinfo_ops);
		ret |= dc_batinfo_ops_register(SC_MODE, di->ic_role,
			&rt9759_aux_batinfo_ops);
	}
	if (ret) {
		hwlog_err("sysinfo ops register fail\n");
		return ret;
	}

	if (di->ic_role == CHARGE_IC_TYPE_MAIN) {
		ret = power_tz_ops_register(&rt9759_temp_sensing_ops,
			"bq25970");
		ret |= power_log_ops_register(&rt9759_log_ops);
	} else {
		ret = power_tz_ops_register(&rt9759_aux_temp_sensing_ops,
			"bq25970_aux");
		ret |= power_log_ops_register(&rt9759_aux_log_ops);
	}
	if (ret)
		hwlog_err("thermalzone or power log ops register fail\n");

	return 0;
}

static void rt9759_fault_event_notify(unsigned long event, void *data)
{
	power_event_anc_notify(POWER_ANT_SC_FAULT, event, data);
}

static void rt9759_fault_handle(struct rt9759_device_info *di,
	struct nty_data *data)
{
	int val = 0;
	u8 fault_flag = data->event1;
	u8 ac_protection = data->event2;
	u8 converter_state = data->event3;

	if (ac_protection & RT9759_AC_OVP_FLAG_MASK) {
		hwlog_info("AC OVP happened\n");
		rt9759_fault_event_notify(POWER_NE_DC_FAULT_AC_OVP, data);
	} else if (fault_flag & RT9759_BAT_OVP_FLT_FLAG_MASK) {
		val = rt9759_get_vbat_mv(di);
		hwlog_info("BAT OVP happened, vbat=%d mv\n", val);
		if (val >= RT9759_VBAT_OVP_THRESHOLD_INIT)
			rt9759_fault_event_notify(POWER_NE_DC_FAULT_VBAT_OVP, data);
	} else if (fault_flag & RT9759_BAT_OCP_FLT_FLAG_MASK) {
		rt9759_get_ibat_ma(&val, di);
		hwlog_info("BAT OCP happened, ibat=%d ma\n", val);
		if (val >= RT9759_IBAT_OCP_THRESHOLD_INIT)
			rt9759_fault_event_notify(POWER_NE_DC_FAULT_IBAT_OCP, data);
	} else if (fault_flag & RT9759_BUS_OVP_FLT_FLAG_MASK) {
		rt9759_get_vbus_mv(&val, di);
		hwlog_info("BUS OVP happened, vbus=%d mv\n", val);
		if (val >= RT9759_VBUS_OVP_THRESHOLD_INIT)
			rt9759_fault_event_notify(POWER_NE_DC_FAULT_VBUS_OVP, data);
	} else if (fault_flag & RT9759_BUS_OCP_FLT_FLAG_MASK) {
		rt9759_get_ibus_ma(&val, di);
		hwlog_info("BUS OCP happened, ibus=%d ma\n", val);
		if (val >= RT9759_IBUS_OCP_THRESHOLD_INIT)
			rt9759_fault_event_notify(POWER_NE_DC_FAULT_IBUS_OCP, data);
	} else if (fault_flag & RT9759_TSBAT_FLT_FLAG_MASK) {
		hwlog_info("BAT TEMP OTP happened\n");
		rt9759_fault_event_notify(POWER_NE_DC_FAULT_TSBAT_OTP, data);
	} else if (fault_flag & RT9759_TSBUS_FLT_FLAG_MASK) {
		hwlog_info("BUS TEMP OTP happened\n");
		rt9759_fault_event_notify(POWER_NE_DC_FAULT_TSBUS_OTP, data);
	} else if (fault_flag & RT9759_TDIE_ALM_FLAG_MASK) {
		hwlog_info("DIE TEMP OTP happened\n");
	}

	if (converter_state & RT9759_CONV_OCP_FLAG_MASK) {
		hwlog_info("CONV OCP happened\n");
		rt9759_fault_event_notify(POWER_NE_DC_FAULT_CONV_OCP, data);
	}
}

static void rt9759_interrupt_work(struct work_struct *work)
{
	struct rt9759_device_info *di = NULL;
	struct nty_data *data = NULL;
	u8 converter_state = 0;
	u8 fault_flag = 0;
	u8 ac_protection = 0;
	u8 ibus_ucp = 0;

	if (!work)
		return;

	di = container_of(work, struct rt9759_device_info, irq_work);
	if (!di || !di->client) {
		hwlog_err("di is null\n");
		return;
	}

	data = &(di->nty_data);

	(void)rt9759_read_byte(di, RT9759_AC_OVP_REG, &ac_protection);
	(void)rt9759_read_byte(di, RT9759_BUS_OCP_UCP_REG, &ibus_ucp);
	(void)rt9759_read_byte(di, RT9759_FLT_FLAG_REG, &fault_flag);
	(void)rt9759_read_byte(di, RT9759_CONVERTER_STATE_REG,
		&converter_state);

	data->event1 = fault_flag;
	data->event2 = ac_protection;
	data->event3 = converter_state;
	data->addr = di->client->addr;

	if (di->int_notify_enable_flag == RT9759_ENABLE_INT_NOTIFY) {
		rt9759_fault_handle(di, data);
		rt9759_dump_register(di);
	}

	rt9759_disable_watchdog(di);
	hwlog_info("ac_ovp_reg [%x]=0x%x\n", RT9759_AC_OVP_REG, ac_protection);
	hwlog_info("bus_ocp_ucp_reg [%x]=0x%x\n", RT9759_BUS_OCP_UCP_REG,
		ibus_ucp);
	hwlog_info("flt_flag_reg [%x]=0x%x\n", RT9759_FLT_FLAG_REG, fault_flag);
	hwlog_info("converter_state_reg [%x]=0x%x\n",
		RT9759_CONVERTER_STATE_REG, converter_state);

	/* clear irq */
	enable_irq(di->irq_int);
}

static irqreturn_t rt9759_interrupt(int irq, void *_di)
{
	struct rt9759_device_info *di = _di;

	if (!di) {
		hwlog_err("di is null\n");
		return IRQ_HANDLED;
	}

	if (di->chip_already_init == 0)
		hwlog_err("chip not init\n");

	if (di->init_finish_flag == RT9759_INIT_FINISH)
		di->int_notify_enable_flag = RT9759_ENABLE_INT_NOTIFY;

	hwlog_info("int happened\n");
	disable_irq_nosync(di->irq_int);
	schedule_work(&di->irq_work);
	return IRQ_HANDLED;
}

static int rt9759_irq_init(struct rt9759_device_info *di,
	struct device_node *np)
{
	int ret;

	ret = power_gpio_config_interrupt(np,
		"gpio_int", "rt9759_gpio_int", &di->gpio_int, &di->irq_int);
	if (ret)
		return ret;

	ret = request_irq(di->irq_int, rt9759_interrupt,
		IRQF_TRIGGER_FALLING, "rt9759_int_irq", di);
	if (ret) {
		hwlog_err("gpio irq request fail\n");
		di->irq_int = -1;
		gpio_free(di->gpio_int);
		return ret;
	}

	enable_irq_wake(di->irq_int);
	INIT_WORK(&di->irq_work, rt9759_interrupt_work);
	return 0;
}

static int rt9759_reg_reset_and_init(struct rt9759_device_info *di)
{
	int ret;

	ret = rt9759_reg_reset(di);
	if (ret) {
		hwlog_err("reg reset fail\n");
		return ret;
	}

	ret = rt9759_fault_clear(di);
	if (ret) {
		hwlog_err("fault clear fail\n");
		return ret;
	}

	ret = rt9759_reg_init(di);
	if (ret) {
		hwlog_err("reg init fail\n");
		return ret;
	}

	return 0;
}

static void rt9759_parse_dts(struct device_node *np,
	struct rt9759_device_info *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tsbus_high_r_kohm", &di->tsbus_high_r_kohm,
		RT9759_RESISTORS_100KOHM);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tsbus_low_r_kohm", &di->tsbus_low_r_kohm,
		RT9759_RESISTORS_100KOHM);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"switching_frequency", &di->switching_frequency,
		RT9759_SW_FREQ_500KHZ);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"special_freq_shift", &di->special_freq_shift, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ic_role", &di->ic_role, CHARGE_IC_TYPE_MAIN);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"sense_r_config", &di->sense_r_config, SENSE_R_2_MOHM);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"sense_r_actual", &di->sense_r_actual, SENSE_R_2_MOHM);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"chrg_ctl_reg_init", &di->chrg_ctl_reg_init,
		RT9759_CHRG_CTL_REG_INIT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"adc_fn_reg_init", &di->adc_fn_reg_init,
		RT9759_ADC_FN_DIS_REG_INIT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"adc_accuracy", &di->adc_accuracy, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"adc_v_ref", &di->adc_v_ref, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"v_pullup", &di->v_pullup, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"r_pullup", &di->r_pullup, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"r_comp", &di->r_comp, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ibus_low_deglitch", &di->ibus_low_deglitch, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ucp_rise_threshold", &di->ucp_rise_threshold, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"close_vbat_protection", &di->close_vbat_protection, 0);
}

static int rt9759_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct rt9759_device_info *di = NULL;
	struct device_node *np = NULL;

	if (!client || !client->dev.of_node || !id)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &client->dev;
	np = di->dev->of_node;
	di->client = client;
	di->chip_already_init = 1;

	ret = rt9759_get_device_id(di);
	if (ret < 0)
		goto rt9759_fail_0;

	rt9759_parse_dts(np, di);

	ret = rt9759_irq_init(di, np);
	if (ret)
		goto rt9759_fail_0;

	ret = rt9759_reg_reset_and_init(di);
	if (ret)
		goto rt9759_fail_1;

	rt9759_ops_register(di);
	i2c_set_clientdata(client, di);

	return 0;

rt9759_fail_1:
	free_irq(di->irq_int, di);
	gpio_free(di->gpio_int);
rt9759_fail_0:
	di->chip_already_init = 0;
	devm_kfree(&client->dev, di);

	return ret;
}

static int rt9759_remove(struct i2c_client *client)
{
	struct rt9759_device_info *di = i2c_get_clientdata(client);

	if (!di)
		return -ENODEV;

	if (di->irq_int)
		free_irq(di->irq_int, di);

	if (di->gpio_int)
		gpio_free(di->gpio_int);

	return 0;
}

static void rt9759_shutdown(struct i2c_client *client)
{
	struct rt9759_device_info *di = i2c_get_clientdata(client);

	if (!di)
		return;

	rt9759_reg_reset(di);
}

#ifdef CONFIG_PM
static int rt9759_i2c_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct rt9759_device_info *di = NULL;

	if (!client)
		return 0;

	di = i2c_get_clientdata(client);
	if (di)
		rt9759_enable_adc(0, (void *)di);

	return 0;
}

static int rt9759_i2c_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct rt9759_device_info *di = NULL;

	if (!client)
		return 0;

	di = i2c_get_clientdata(client);
	if (di)
		rt9759_enable_adc(1, (void *)di);

	return 0;
}

static const struct dev_pm_ops rt9759_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(rt9759_i2c_suspend, rt9759_i2c_resume)
};
#define RT9759_PM_OPS (&rt9759_pm_ops)
#else
#define RT9759_PM_OPS (NULL)
#endif /* CONFIG_PM */

MODULE_DEVICE_TABLE(i2c, rt9759);
static const struct of_device_id rt9759_of_match[] = {
	{
		.compatible = "bq25970",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id rt9759_i2c_id[] = {
	{ "bq25970", 0 },
	{}
};

static struct i2c_driver rt9759_driver = {
	.probe = rt9759_probe,
	.remove = rt9759_remove,
	.shutdown = rt9759_shutdown,
	.id_table = rt9759_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "rt9759",
		.of_match_table = of_match_ptr(rt9759_of_match),
		.pm = RT9759_PM_OPS,
	},
};

static int __init rt9759_init(void)
{
	return i2c_add_driver(&rt9759_driver);
}

static void __exit rt9759_exit(void)
{
	i2c_del_driver(&rt9759_driver);
}

module_init(rt9759_init);
module_exit(rt9759_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("rt9759 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
