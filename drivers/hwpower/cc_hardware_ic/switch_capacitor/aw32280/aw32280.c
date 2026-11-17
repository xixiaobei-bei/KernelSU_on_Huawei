/*
 * aw32280.c
 *
 * aw32280 driver
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#include "aw32280.h"
#include <chipset_common/hwpower/common_module/power_i2c.h>
#include <chipset_common/hwpower/common_module/power_log.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <huawei_platform/power/direct_charger/direct_charger.h>

#define HWLOG_TAG aw32280
HWLOG_REGIST();

static int aw32280_write_byte(struct aw32280_device_info *di, u16 reg, u8 value)
{
	if (!di || (di->chip_already_init == 0)) {
		hwlog_err("chip not init\n");
		return -ENODEV;
	}

	return power_i2c_u16_write_byte(di->client, reg, value);
}

static int aw32280_read_byte(struct aw32280_device_info *di, u16 reg, u8 *value)
{
	if (!di || (di->chip_already_init == 0)) {
		hwlog_err("chip not init\n");
		return -ENODEV;
	}

	return power_i2c_u16_read_byte(di->client, reg, value);
}

static int aw32280_read_word(struct aw32280_device_info *di, u16 reg, u16 *value)
{
	if (!di || (di->chip_already_init == 0)) {
		hwlog_err("chip not init\n");
		return -ENODEV;
	}

	return power_i2c_u16_read_word(di->client, reg, value, false);
}

static int aw32280_write_mask(struct aw32280_device_info *di, u16 reg,
	u8 mask, u8 shift, u8 value)
{
	int ret;
	u8 val = 0;

	ret = aw32280_read_byte(di, reg, &val);
	if (ret < 0)
		return ret;

	val &= ~mask;
	val |= ((value << shift) & mask);

	return aw32280_write_byte(di, reg, val);
}

u16 aw32280_dump_map[] = {
	AW32280_SC_PRO_TOP_CFG_4_REG,
	AW32280_SC_PRO_TOP_CFG_5_REG,
	AW32280_SC_PRO_TOP_CFG_8_REG,
	AW32280_SC_VOUT_OVP_REG,
};

static void aw32280_dump_register(struct aw32280_device_info *di)
{
	int i;
	u8 val;

	for (i = 0; i <= ARRAY_SIZE(aw32280_dump_map); i++) {
		val = 0;
		if (!aw32280_read_byte(di, i, &val))
			hwlog_info("reg[0x%x]=0x%x", i, val);
	}
}

static int aw32280_reg_reset(struct aw32280_device_info *di)
{
	return 0;
}

u16 aw32280_fault_map[] = {
	AW32280_IRQ_FLAG_REG,
	AW32280_IRQ_FLAG_2_REG,
	AW32280_IRQ_FLAG_3_REG,
	AW32280_IRQ_FLAG_4_REG,
	AW83220_IRQ_FLAG_5_REG,
};

static void aw32280_fault_clear(struct aw32280_device_info *di)
{
	int i;
	u8 val;

	for (i = 0; i <= ARRAY_SIZE(aw32280_fault_map); i++) {
		val = 0;
		if (!aw32280_read_byte(di, i, &val))
			hwlog_info("fault_reg[%x]=0x%x\n", i, val);
	}
}

static int aw32280_adc_enable(int enable, void *dev_data)
{
	int ret;
	u8 reg = 0;
	u8 value = enable ? 0x1 : 0x0;
	struct aw32280_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = aw32280_write_mask(di, AW32280_HKADC_EN_REG,
		AW32280_SC_HKADC_EN_MASK, AW32280_SC_HKADC_EN_SHIFT, value);
	ret += aw32280_read_byte(di, AW32280_HKADC_EN_REG, &reg);
	if (ret)
		return ret;

	hwlog_info("adc_enable [%x]=0x%x\n", AW32280_HKADC_EN_REG, reg);

	return 0;
}

static bool aw32280_is_adc_disabled(struct aw32280_device_info *di)
{
	u8 reg = 0;
	int ret;

	ret = aw32280_read_byte(di, AW32280_HKADC_EN_REG, &reg);
	if (ret || !(reg & AW32280_SC_HKADC_EN_MASK))
		return true;

	return false;
}

static int aw32280_discharge(int enable, void *dev_data)
{
	return 0;
}

static int aw32280_is_device_close(void *dev_data)
{
	u8 val = 0;
	struct aw32280_device_info *di = dev_data;

	if (!di)
		return 1;

	if (aw32280_read_byte(di, AW32280_SC_SC_EN_REG, &val))
		return 1;

	if (val & AW32280_SC_SC_EN)
		return 0;

	return 1;
}

static int aw32280_get_device_id(void *dev_data)
{
	int ret;
	u8 chip_id[AW32280_CHIP_ID_REG_NUM] = {0};
	struct aw32280_device_info *di = dev_data;

	if (!di)
		return AW32280_DEVICE_ID_GET_FAIL;

	if (di->get_id_time == AW32280_USED)
		return di->device_id;

	di->get_id_time = AW32280_USED;

	ret = power_i2c_read_block(di->client, AW32280_CHIP_ID_0_REG,
		AW32280_CHIP_ID_REG_NUM, chip_id, AW32280_CHIP_ID_REG_NUM);
	if (ret)
		return -EIO;

	if ((chip_id[0] != AW32280_CHIP_ID_0) ||
		(chip_id[1] != AW32280_CHIP_ID_1) ||
		(chip_id[2] != AW32280_CHIP_ID_2) ||
		(chip_id[3] != AW32280_CHIP_ID_3) ||
		(chip_id[4] != AW32280_CHIP_ID_4) ||
		(chip_id[5] != AW32280_CHIP_ID_5)) {
		di->get_id_time = AW32280_NOT_USED;
		return AW32280_DEVICE_ID_GET_FAIL;
	}

	di->device_id = SWITCHCAP_AW32280;
	hwlog_info("device_id: 0x%x\n", di->device_id);

	return di->device_id;
}

static int aw32280_get_vbat_mv(void *dev_data)
{
	u16 data = 0;
	int ret;
	struct aw32280_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	if (aw32280_is_adc_disabled(di))
		return 0;

	ret = aw32280_read_word(di, AW32280_VBAT1_ADC_L_REG, &data);
	if (ret)
		return ret;

	hwlog_info("VBAT_ADC=0x%x\n", data);

	return (int)data;
}

static int aw32280_get_ibat_ma(int *ibat, void *dev_data)
{
	int ret;
	u16 data = 0;
	struct aw32280_device_info *di = dev_data;

	if (!ibat || !di)
		return -ENODEV;

	if (aw32280_is_adc_disabled(di)) {
		*ibat = 0;
		return 0;
	}

	ret = aw32280_read_word(di, AW32280_IBAT1_ADC_L_REG, &data);
	if (ret)
		return ret;

	hwlog_info("IBAT_ADC=0x%x\n", data);
	*ibat = (int)data * di->sense_r_config;
	*ibat /= di->sense_r_actual;

	return 0;
}

static int aw32280_get_ibus_ma(int *ibus, void *dev_data)
{
	u16 data = 0;
	int ret;
	struct aw32280_device_info *di = dev_data;

	if (!di || !ibus)
		return -ENODEV;

	if (aw32280_is_adc_disabled(di)) {
		*ibus = 0;
		return 0;
	}

	ret = aw32280_read_word(di, AW32280_IBUS_ADC_L_REG, &data);
	if (ret)
		return ret;

	hwlog_info("IBUS_ADC=0x%x\n", data);
	*ibus = (int)data;

	return 0;
}

static int aw32280_get_vbus_mv(int *vbus, void *dev_data)
{
	int ret;
	u16 data = 0;
	struct aw32280_device_info *di = dev_data;

	if (!di || !vbus)
		return -ENODEV;

	if (aw32280_is_adc_disabled(di)) {
		*vbus = 0;
		return 0;
	}

	ret = aw32280_read_word(di, AW32280_VBUS_ADC_L_REG, &data);
	if (ret)
		return ret;

	hwlog_info("VBUS_ADC=0x%x\n", data);
	*vbus = (int)data;

	return 0;
}

static int aw32280_is_tsbat_disabled(void *dev_data)
{
	return 0;
}

static int aw32280_get_device_temp(int *temp, void *dev_data)
{
	s16 data = 0;
	int ret;
	struct aw32280_device_info *di = dev_data;

	if (!temp || !di)
		return -ENODEV;

	if (aw32280_is_adc_disabled(di)) {
		*temp = 0;
		return 0;
	}

	ret = aw32280_read_word(di, AW32280_TDIE_ADC_L_REG, &data);
	if (ret)
		return ret;

	hwlog_info("TDIE_ADC=0x%x\n", data);
	*temp = (int)data;

	return 0;
}

static int aw32280_get_vusb_mv(int *vusb, void *dev_data)
{
	int ret;
	u16 data = 0;
	struct aw32280_device_info *di = dev_data;

	if (!vusb || !di)
		return -ENODEV;

	if (aw32280_is_adc_disabled(di)) {
		*vusb = 0;
		return 0;
	}

	ret = aw32280_read_word(di, AW32280_VUSB_ADC_L_REG, &data);
	if (ret)
		return ret;

	hwlog_info("VUSB_ADC=0x%x\n", data);
	*vusb = (int)data;

	return 0;
}

static int aw32280_get_vout_mv(int *vout, void *dev_data)
{
	int ret;
	u16 data = 0;
	struct aw32280_device_info *di = dev_data;

	if (!vout || !di)
		return -ENODEV;

	if (aw32280_is_adc_disabled(di)) {
		*vout = 0;
		return 0;
	}

	ret = aw32280_read_word(di, AW32280_VOUT1_ADC_L_REG, &data);
	if (ret)
		return ret;

	hwlog_info("VOUT_ADC=0x%x\n", data);
	*vout = (int)data;

	return 0;
}

static int aw32280_get_register_head(char *buffer, int size, void *dev_data)
{
	struct aw32280_device_info *di = dev_data;

	if (!buffer || !di)
		return -ENODEV;

	if (di->ic_role == CHARGE_IC_TYPE_MAIN)
		snprintf(buffer, size,
			"   mode   Ibus   Vbus   Ibat   Vusb   Vout   Vbat   Temp");
	else
		snprintf(buffer, size,
			"   mode1  Ibus1  Vbus1  Ibat1  Vusb1  Vout1  Vbat1  Temp1");

	return 0;
}

static int aw32280_value_dump(char *buffer, int size, void *dev_data)
{
	int vbat;
	int ibus = 0;
	int vbus = 0;
	int ibat = 0;
	int vusb = 0;
	int vout = 0;
	int temp = 0;
	u8 val = 0;
	char buff[AW32280_BUF_LEN] = { 0 };
	struct aw32280_device_info *di = dev_data;

	if (!buffer || !di)
		return -ENODEV;

	vbat = aw32280_get_vbat_mv(dev_data);
	(void)aw32280_get_ibus_ma(&ibus, dev_data);
	(void)aw32280_get_vbus_mv(&vbus, dev_data);
	(void)aw32280_get_ibat_ma(&ibat, dev_data);
	(void)aw32280_get_vusb_mv(&vusb, dev_data);
	(void)aw32280_get_vout_mv(&vout, dev_data);
	(void)aw32280_get_device_temp(&temp, dev_data);
	(void)aw32280_read_byte(di, AW32280_SC_SC_MODE_REG, &val);

	if (aw32280_is_device_close(dev_data))
		snprintf(buff, sizeof(buff), "%s", "OFF    ");
	else if (((val & AW32280_SC_SC_MODE_MASK) >> AW32280_SC_SC_MODE_SHIFT) ==
		AW32280_CHG_FBYPASS_MODE)
		snprintf(buff, sizeof(buff), "%s", "LVC    ");
	else if (((val & AW32280_SC_SC_MODE_MASK) >> AW32280_SC_SC_MODE_SHIFT) ==
		AW32280_CHG_F21SC_MODE)
		snprintf(buff, sizeof(buff), "%s", "SC     ");
	else if (((val & AW32280_SC_SC_MODE_MASK) >> AW32280_SC_SC_MODE_SHIFT) ==
		AW32280_CHG_F41SC_MODE)
		snprintf(buff, sizeof(buff), "%s", "41SC   ");

	strncat(buffer, buff, strlen(buff));
	snprintf(buff, sizeof(buff), "%-7d%-7d%-7d%-7d%-7d%-7d%-7d",
		ibus, vbus, ibat, vusb, vout, vbat, temp);
	strncat(buffer, buff, strlen(buff));

	return 0;
}

static int aw32280_config_watchdog_ms(int time, void *dev_data)
{
	u8 val;
	u8 reg;
	int ret;
	struct aw32280_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	if (time > AW32280_WTD_CONFIG_TIMING_2000MS)
		val = AW32280_WDT_CONFIG_5S;
	else if (time > AW32280_WTD_CONFIG_TIMING_1000MS)
		val = AW32280_WDT_CONFIG_2S;
	else if (time > AW32280_WTD_CONFIG_TIMING_500MS)
		val = AW32280_WDT_CONFIG_1S;
	else
		val = AW32280_WDT_CONFIG_500MS;

	ret = aw32280_write_mask(di, AW32280_WDT_CONFIG_REG,
		AW32280_WDT_CONFIG_MASK, AW32280_WDT_CONFIG_SHIFT, val);
	ret += aw32280_read_byte(di, AW32280_WDT_CONFIG_REG, &reg);
	if (ret)
		return ret;

	hwlog_info("config_watchdog_ms [%x]=0x%x\n", AW32280_WDT_CONFIG_REG, reg);

	return 0;
}

static int aw32280_config_vbat_ovp_threshold(struct aw32280_device_info *di,
	int ovp_th)
{
	u8 vbat1;
	u8 vbat2 = 0;
	int ret;

	if (ovp_th < AW32280_FWD_VBAT_OVP_MIN)
		ovp_th = AW32280_FWD_VBAT_OVP_MIN;

	if (ovp_th > AW32280_FWD_VBAT_OVP_MAX)
		ovp_th = AW32280_FWD_VBAT_OVP_MAX;

	vbat1 = (u8)((ovp_th - AW32280_FWD_VBAT_OVP_MIN) /
		AW32280_FWD_VBAT_OVP_STEP);
	ret = aw32280_write_mask(di, AW32280_FWD_VBAT1_OVP_SEL_REG,
		AW32280_FWD_VBAT_OVP_MASK, AW32280_FWD_VBAT_OVP_SHIFT, vbat1);
	ret += aw32280_write_mask(di, AW32280_FWD_VBAT2_OVP_SEL_REG,
		AW32280_FWD_VBAT_OVP_MASK, AW32280_FWD_VBAT_OVP_SHIFT, vbat1);
	ret += aw32280_read_byte(di, AW32280_FWD_VBAT1_OVP_SEL_REG, &vbat1);
	ret += aw32280_read_byte(di, AW32280_FWD_VBAT2_OVP_SEL_REG, &vbat2);
	if (ret)
		return ret;

	hwlog_info("config_vbat1_ovp_threshold_mv [%x]=0x%x\n",
		AW32280_FWD_VBAT1_OVP_SEL_REG, vbat1);
	hwlog_info("config_vbat2_ovp_threshold_mv [%x]=0x%x\n",
		AW32280_FWD_VBAT2_OVP_SEL_REG, vbat2);

	return 0;
}

static int aw32280_config_ibat_ocp_threshold(struct aw32280_device_info *di,
	int ocp_th)
{
	u8 ibat1;
	u8 ibat2 = 0;
	int ret;

	if (ocp_th < AW32280_FWD_IBAT_OVP_MIN)
		ocp_th = AW32280_FWD_IBAT_OVP_MIN;

	if (ocp_th > AW32280_FWD_IBAT_OVP_MAX)
		ocp_th = AW32280_FWD_IBAT_OVP_MAX;

	ibat1 = (u8)((ocp_th - AW32280_FWD_IBAT_OVP_MIN) /
		AW32280_FWD_IBAT_OVP_STEP);
	ret = aw32280_write_mask(di, AW32280_SC_PRO_TOP_CFG_4_REG,
		AW32280_FWD_IBAT1_OVP_MASK, AW32280_FWD_IBAT1_OVP_SHIFT, ibat1);
	ret += aw32280_write_mask(di, AW32280_SC_PRO_TOP_CFG_5_REG,
		AW32280_FWD_IBAT2_OVP_MASK, AW32280_FWD_IBAT2_OVP_SHIFT, ibat1);
	ret += aw32280_read_byte(di, AW32280_SC_PRO_TOP_CFG_4_REG, &ibat1);
	ret += aw32280_read_byte(di, AW32280_SC_PRO_TOP_CFG_5_REG, &ibat2);
	if (ret)
		return ret;

	hwlog_info("config_ibat1_ovp_threshold [%x]=0x%x\n",
		AW32280_SC_PRO_TOP_CFG_4_REG, ibat1);
	hwlog_info("config_ibat2_ovp_threshold [%x]=0x%x\n",
		AW32280_SC_PRO_TOP_CFG_5_REG, ibat2);

	return 0;
}

static int aw32280_config_ibat_sns_res(struct aw32280_device_info *di)
{
	u8 res_config;
	int ret;

	if (di->sense_r_config == SENSE_R_2_MOHM)
		res_config = AW32280_IBAT_SNS_RES_2MOHM;
	else
		res_config = AW32280_IBAT_SNS_RES_1MOHM;

	ret = aw32280_write_byte(di, AW32280_IBAT1_RES_PLACE_REG,
		AW32280_IBAT_RES_LOWSIDE);
	ret += aw32280_write_byte(di, AW32280_IBAT2_RES_PLACE_REG,
		AW32280_IBAT_RES_LOWSIDE);
	ret += aw32280_write_mask(di, AW32280_SC_PRO_TOP_CFG_6_REG,
		AW32280_IBAT_RES_SEL_MASK, AW32280_IBAT_RES_SEL_SHIFT, res_config);

	return ret;
}

static int aw32280_opt_regs_init(struct aw32280_device_info *di, u8 mode)
{
	int ret;
	u8 usb_ovp_val;
	u8 psw_ovp_val;

	if (mode == AW32280_CHG_FBYPASS_MODE) {
		usb_ovp_val = AW32280_USB_OVP_FBPSC_VAL;
		psw_ovp_val = AW32280_PSW_OVP_FBPSC_VAL;
	} else if (mode == AW32280_CHG_F21SC_MODE) {
		usb_ovp_val = AW32280_USB_OVP_F21SC_VAL;
		psw_ovp_val = AW32280_PSW_OVP_F21SC_VAL;
	} else if (mode == AW32280_CHG_F41SC_MODE) {
		usb_ovp_val = AW32280_USB_OVP_F41SC_VAL;
		psw_ovp_val = AW32280_PSW_OVP_F41SC_VAL;
	}

	ret = aw32280_write_byte(di, AW32280_SC_DET_TOP_CFG_2_REG,
		AW32280_DET_TOP_CFG_2_REG_INIT);
	ret += aw32280_write_mask(di, AW32280_USB_OVP_CFG_2_REG,
		AW32280_USB_OVP_MASK, AW32280_USB_OVP_SHIFT, usb_ovp_val);
	ret += aw32280_write_mask(di, AW32280_PSW_OVP_CFG_2_REG,
		AW32280_PSW_OVP_MASK, AW32280_PSW_OVP_SHIFT, psw_ovp_val);
	ret += aw32280_write_byte(di, AW32280_SC_PRO_TOP_CFG_7_REG,
		AW32280_PRO_TOP_CFG_7_REG_INIT);
	ret += aw32280_write_byte(di, AW32280_SC_PRO_TOP_CFG_8_REG,
		AW32280_PRO_TOP_CFG_8_REG_INIT);
	ret += aw32280_config_vbat_ovp_threshold(di, AW80322_FWD_VBAT_OVP_INIT);
	ret += aw32280_write_byte(di, AW32280_SC_VOUT_OVP_REG,
		AW32280_SC_VOUT_OVP_REG_INIT);
	ret += aw32280_write_byte(di, AW32280_SC_TOP_CFG_8_REG,
		AW32280_SC_TOP_CFG_8_REG_INIT);
	ret += aw32280_write_byte(di, AW32280_SC_TOP_CFG_7_REG,
		AW32280_SC_TOP_CFG_7_REG_INIT);
	ret += aw32280_write_byte(di, AW22803_SC_PRO_TOP_CFG_2_REG,
		AW22803_SC_PRO_TOP_CFG_2_REG_INIT);
	ret += aw32280_write_byte(di, AW32280_SC_PRO_TOP_CFG_0_REG,
		AW32280_SC_PRO_TOP_CFG_0_REG_INIT);
	ret += aw32280_config_ibat_ocp_threshold(di, AW32280_FWD_IBAT_OVP_INIT);
	ret += aw32280_write_byte(di, AW32280_SC_PRO_TOP_CFG_3_REG,
		AW32280_SC_PRO_TOP_CFG_3_REG_INIT);
	ret += aw32280_write_byte(di, AW32280_SC_DET_TOP_CFG_0_REG,
		AW32280_SC_DET_TOP_CFG_0_REG_INIT);
	ret += aw32280_write_byte(di, AW32280_USB_OVP_CFG_0_REG,
		AW32280_USB_OVP_CFG_0_REG_INIT);
	ret += aw32280_write_byte(di, AW32280_SC_PRO_TOP_CFG_12_REG,
		AW32280_SC_PRO_TOP_CFG_12_REG_INIT);
	ret += aw32280_write_mask(di, AW32280_SC_TOP_CFG_6_REG,
		AW32280_SC_PARALLEL_SEL_MASK, AW32280_SC_PARALLEL_SEL_SHIFT,
		di->parallel_mode);
	ret += aw32280_write_mask(di, AW32280_SC_TOP_CFG_4_REG,
		AW32280_SC_HOST_SEL_MASK, AW32280_SC_HOST_SEL_SHIFT, di->slave_mode);

	return ret;
}

static int aw32280_charge_mode_enable(int enable, void *dev_data, u8 mode)
{
	int ret;
	u8 mode_reg = 0;
	u8 chg_en_reg = 0;
	u8 chg_en = enable ? AW32280_SC_SC_EN : AW32280_SC_SC_DISABLE;
	struct aw32280_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = aw32280_write_mask(di, AW32280_SC_SC_EN_REG, AW32280_SC_SC_EN_MASK,
		AW32280_SC_SC_EN_SHIFT, AW32280_SC_SC_DISABLE);
	ret += aw32280_write_mask(di, AW32280_SC_SC_MODE_REG,
		AW32280_SC_SC_MODE_MASK, AW32280_SC_SC_MODE_SHIFT, mode);
	ret += aw32280_opt_regs_init(di, mode);
	ret += aw32280_write_mask(di, AW32280_SC_SC_EN_REG, AW32280_SC_SC_EN_MASK,
		AW32280_SC_SC_EN_SHIFT, chg_en);
	ret += aw32280_read_byte(di, AW32280_SC_SC_MODE_REG, &mode_reg);
	ret += aw32280_read_byte(di, AW32280_SC_SC_EN_REG, &chg_en_reg);
	if (ret)
		return ret;

	di->charge_mode = mode;
	hwlog_info("ic_role=%d, charge_enable [%x]=0x%x,[%x]=0x%x\n",
		di->ic_role, AW32280_SC_SC_MODE_REG, mode_reg,
		AW32280_SC_SC_EN_REG, chg_en_reg);

	return 0;
}

static int aw32280_lvc_charge_enable(int enable, void *dev_data)
{
	return aw32280_charge_mode_enable(enable, dev_data, AW32280_CHG_FBYPASS_MODE);
}

static int aw32280_sc_charge_enable(int enable, void *dev_data)
{
	return aw32280_charge_mode_enable(enable, dev_data, AW32280_CHG_F21SC_MODE);
}

static int aw32280_sc_41_charge_enable(int enable, void *dev_data)
{
	return aw32280_charge_mode_enable(enable, dev_data, AW32280_CHG_F41SC_MODE);
}

static int aw32280_reg_init(struct aw32280_device_info *di)
{
	int ret;

	ret = aw32280_config_watchdog_ms(AW32280_WTD_CONFIG_TIMING_5000MS, di);
	ret += aw32280_opt_regs_init(di, AW32280_CHG_F41SC_MODE);
	ret += aw32280_config_ibat_sns_res(di);
	ret += aw32280_write_byte(di, AW32280_HKADC_CTRL1_REG,
		AW32280_HKADC_CTRL1_INIT);
	ret += aw32280_write_mask(di, AW32280_HKADC_EN_REG,
		AW32280_SC_HKADC_EN_MASK, AW32280_SC_HKADC_EN_SHIFT, 1);
	ret += aw32280_write_byte(di, AW32280_HKADC_SEQ_CH_H_REG,
		AW32280_HKADC_SEQ_CH_H_INIT);
	ret += aw32280_write_byte(di, AW32280_HKADC_SEQ_CH_L_REG,
		AW32280_HKADC_SEQ_CH_L_INIT);
	ret += aw32280_write_mask(di, AW32280_HKADC_START_REG,
		AW32280_HKADC_START_MASK, AW32280_HKADC_START_SHIFT, 1);
	if (ret)
		hwlog_err("reg_init fail\n");

	return ret;
}

static int aw32280_reg_reset_and_init(void *dev_data)
{
	int ret;
	struct aw32280_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = aw32280_reg_reset(di);
	aw32280_fault_clear(di);
	ret += aw32280_reg_init(di);

	return ret;
}

static int aw32280_charge_init(void *dev_data)
{
	struct aw32280_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	if (aw32280_reg_init(di))
		return -EINVAL;

	di->device_id = aw32280_get_device_id(dev_data);
	if (di->device_id == AW32280_DEVICE_ID_GET_FAIL)
		return -EINVAL;

	hwlog_info("device id is %d\n", di->device_id);

	di->init_finish_flag = AW32280_INIT_FINISH;

	return 0;
}

static int aw32280_charge_exit(void *dev_data)
{
	int ret;
	struct aw32280_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = aw32280_charge_mode_enable(false, dev_data, AW32280_CHG_F41SC_MODE);
	di->init_finish_flag = AW32280_NOT_INIT;
	di->int_notify_enable_flag = AW32280_DISABLE_INT_NOTIFY;
	power_usleep(DT_USLEEP_10MS);

	return ret;
}

static int aw32280_batinfo_exit(void *dev_data)
{
	return 0;
}

static int aw32280_batinfo_init(void *dev_data)
{
	struct aw32280_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	return aw32280_charge_init(di);
}

static struct dc_ic_ops aw32280_lvc_ops = {
	.dev_name = "aw32280",
	.ic_init = aw32280_charge_init,
	.ic_exit = aw32280_charge_exit,
	.ic_enable = aw32280_lvc_charge_enable,
	.ic_adc_enable = aw32280_adc_enable,
	.ic_discharge = aw32280_discharge,
	.is_ic_close = aw32280_is_device_close,
	.get_ic_id = aw32280_get_device_id,
	.config_ic_watchdog = aw32280_config_watchdog_ms,
	.get_ic_status = aw32280_is_tsbat_disabled,
	.ic_reg_reset_and_init = aw32280_reg_reset_and_init,
};

static struct dc_ic_ops aw32280_sc_ops = {
	.dev_name = "aw32280",
	.ic_init = aw32280_charge_init,
	.ic_exit = aw32280_charge_exit,
	.ic_enable = aw32280_sc_charge_enable,
	.ic_adc_enable = aw32280_adc_enable,
	.ic_discharge = aw32280_discharge,
	.is_ic_close = aw32280_is_device_close,
	.get_ic_id = aw32280_get_device_id,
	.config_ic_watchdog = aw32280_config_watchdog_ms,
	.get_ic_status = aw32280_is_tsbat_disabled,
	.ic_reg_reset_and_init = aw32280_reg_reset_and_init,
};

static struct dc_ic_ops aw32280_sc_41_ops = {
	.dev_name = "aw32280",
	.ic_init = aw32280_charge_init,
	.ic_exit = aw32280_charge_exit,
	.ic_enable = aw32280_sc_41_charge_enable,
	.ic_adc_enable = aw32280_adc_enable,
	.ic_discharge = aw32280_discharge,
	.is_ic_close = aw32280_is_device_close,
	.get_ic_id = aw32280_get_device_id,
	.config_ic_watchdog = aw32280_config_watchdog_ms,
	.get_ic_status = aw32280_is_tsbat_disabled,
	.ic_reg_reset_and_init = aw32280_reg_reset_and_init,
};

static struct dc_batinfo_ops aw32280_batinfo_ops = {
	.init = aw32280_batinfo_init,
	.exit = aw32280_batinfo_exit,
	.get_bat_btb_voltage = aw32280_get_vbat_mv,
	.get_bat_package_voltage = aw32280_get_vbat_mv,
	.get_vbus_voltage = aw32280_get_vbus_mv,
	.get_bat_current = aw32280_get_ibat_ma,
	.get_ic_ibus = aw32280_get_ibus_ma,
	.get_ic_temp = aw32280_get_device_temp,
	.get_ic_vout = aw32280_get_vout_mv,
	.get_ic_vusb = aw32280_get_vusb_mv,
};

static struct power_log_ops aw32280_log_ops = {
	.dev_name = "aw32280",
	.dump_log_head = aw32280_get_register_head,
	.dump_log_content = aw32280_value_dump,
};

static struct dc_ic_ops aw32280_aux_lvc_ops = {
	.dev_name = "aw32280_aux",
	.ic_init = aw32280_charge_init,
	.ic_exit = aw32280_charge_exit,
	.ic_enable = aw32280_lvc_charge_enable,
	.ic_adc_enable = aw32280_adc_enable,
	.ic_discharge = aw32280_discharge,
	.is_ic_close = aw32280_is_device_close,
	.get_ic_id = aw32280_get_device_id,
	.config_ic_watchdog = aw32280_config_watchdog_ms,
	.get_ic_status = aw32280_is_tsbat_disabled,
	.ic_reg_reset_and_init = aw32280_reg_reset_and_init,
};

static struct dc_ic_ops aw32280_aux_sc_ops = {
	.dev_name = "aw32280_aux",
	.ic_init = aw32280_charge_init,
	.ic_exit = aw32280_charge_exit,
	.ic_enable = aw32280_sc_charge_enable,
	.ic_adc_enable = aw32280_adc_enable,
	.ic_discharge = aw32280_discharge,
	.is_ic_close = aw32280_is_device_close,
	.get_ic_id = aw32280_get_device_id,
	.config_ic_watchdog = aw32280_config_watchdog_ms,
	.get_ic_status = aw32280_is_tsbat_disabled,
	.ic_reg_reset_and_init = aw32280_reg_reset_and_init,
};

static struct dc_ic_ops aw32280_aux_sc_41_ops = {
	.dev_name = "aw32280_aux",
	.ic_init = aw32280_charge_init,
	.ic_exit = aw32280_charge_exit,
	.ic_enable = aw32280_sc_41_charge_enable,
	.ic_adc_enable = aw32280_adc_enable,
	.ic_discharge = aw32280_discharge,
	.is_ic_close = aw32280_is_device_close,
	.get_ic_id = aw32280_get_device_id,
	.config_ic_watchdog = aw32280_config_watchdog_ms,
	.get_ic_status = aw32280_is_tsbat_disabled,
	.ic_reg_reset_and_init = aw32280_reg_reset_and_init,
};

static struct dc_batinfo_ops aw32280_aux_batinfo_ops = {
	.init = aw32280_batinfo_init,
	.exit = aw32280_batinfo_exit,
	.get_bat_btb_voltage = aw32280_get_vbat_mv,
	.get_bat_package_voltage = aw32280_get_vbat_mv,
	.get_vbus_voltage = aw32280_get_vbus_mv,
	.get_bat_current = aw32280_get_ibat_ma,
	.get_ic_ibus = aw32280_get_ibus_ma,
	.get_ic_temp = aw32280_get_device_temp,
	.get_ic_vout = aw32280_get_vout_mv,
	.get_ic_vusb = aw32280_get_vusb_mv,
};

static struct power_log_ops aw32280_aux_log_ops = {
	.dev_name = "aw32280_aux",
	.dump_log_head = aw32280_get_register_head,
	.dump_log_content = aw32280_value_dump,
};

static void aw32280_init_ops_dev_data(struct aw32280_device_info *di)
{
	if (di->ic_role == CHARGE_IC_TYPE_MAIN) {
		aw32280_lvc_ops.dev_data = (void *)di;
		aw32280_sc_ops.dev_data = (void *)di;
		aw32280_sc_41_ops.dev_data = (void *)di;
		aw32280_batinfo_ops.dev_data = (void *)di;
		aw32280_log_ops.dev_data = (void *)di;
	} else {
		aw32280_aux_lvc_ops.dev_data = (void *)di;
		aw32280_aux_sc_ops.dev_data = (void *)di;
		aw32280_aux_sc_41_ops.dev_data = (void *)di;
		aw32280_aux_batinfo_ops.dev_data = (void *)di;
		aw32280_aux_log_ops.dev_data = (void *)di;
	}
}

static int aw32280_ops_register(struct aw32280_device_info *di)
{
	int ret;

	aw32280_init_ops_dev_data(di);

	if (di->ic_role == CHARGE_IC_TYPE_MAIN) {
		ret = dc_ic_ops_register(LVC_MODE, di->ic_role,
			&aw32280_lvc_ops);
		ret += dc_ic_ops_register(SC_MODE, di->ic_role,
			&aw32280_sc_ops);
		ret += dc_ic_ops_register(SC_4_1_MODE, di->ic_role,
			&aw32280_sc_41_ops);
		ret += dc_batinfo_ops_register(SC_MODE, di->ic_role,
			&aw32280_batinfo_ops);
		ret += dc_batinfo_ops_register(LVC_MODE, di->ic_role,
			&aw32280_batinfo_ops);
		ret += dc_batinfo_ops_register(SC_4_1_MODE, di->ic_role,
			&aw32280_batinfo_ops);
		ret += power_log_ops_register(&aw32280_log_ops);
	} else {
		ret = dc_ic_ops_register(LVC_MODE, di->ic_role,
			&aw32280_aux_lvc_ops);
		ret += dc_ic_ops_register(SC_MODE, di->ic_role,
			&aw32280_aux_sc_ops);
		ret += dc_ic_ops_register(SC_4_1_MODE, di->ic_role,
			&aw32280_aux_sc_41_ops);
		ret += dc_batinfo_ops_register(SC_MODE, di->ic_role,
			&aw32280_aux_batinfo_ops);
		ret += dc_batinfo_ops_register(LVC_MODE, di->ic_role,
			&aw32280_aux_batinfo_ops);
		ret += dc_batinfo_ops_register(SC_4_1_MODE, di->ic_role,
			&aw32280_aux_batinfo_ops);
		ret += power_log_ops_register(&aw32280_aux_log_ops);
	}
	if (ret) {
		hwlog_err("ops register fail\n");
		return ret;
	}

	return 0;
}

static bool aw32280_vbus_value_check(struct aw32280_device_info *di, int val)
{
	bool ret = false;

	switch (di->charge_mode) {
	case AW32280_CHG_FBYPASS_MODE:
		if (val >= AW32280_VBUS_OVP_FBPSC_MIN)
			ret = true;
		break;
	case AW32280_CHG_F21SC_MODE:
		if (val >= AW32280_VBUS_OVP_F21SC_MIN)
			ret = true;
		break;
	case AW32280_CHG_F41SC_MODE:
		if (val >= AW32280_VBUS_OVP_F41SC_MIN)
			ret = true;
		break;
	default:
		hwlog_err("mode check fail, mode=%u\n", di->charge_mode);
		break;
	}

	return ret;
}

static bool aw32280_ibus_value_check(struct aw32280_device_info *di, int val)
{
	bool ret = false;

	switch (di->charge_mode) {
	case AW32280_CHG_FBYPASS_MODE:
	case AW32280_CHG_F21SC_MODE:
		if (val >= AW32280_IBUS_OVP_F21SC_MIN)
			ret = true;
		break;
	case AW32280_CHG_F41SC_MODE:
		if (val >= AW32280_IBUS_OVP_F41SC_MIN)
			ret = true;
		break;
	default:
		hwlog_err("mode check fail, mode=%u\n", di->charge_mode);
		break;
	}

	return ret;
}

static void aw32280_fault_event_notify(unsigned long event, void *data)
{
	power_event_anc_notify(POWER_ANT_SC_FAULT, event, data);
}

static void aw32280_fault_handle(struct aw32280_device_info *di,
	struct nty_data *data)
{
	int val = 0;
	u8 flag = data->event1;
	u8 flag1 = data->event2;
	u8 flag2 = data->event3;

	if (flag2 & AW32280_VBAT_OVP_FLAG_MASK) {
		val = aw32280_get_vbat_mv(di);
		hwlog_info("BAT OVP happened, vbat=%d\n", val);
		if (val >= AW80322_FWD_VBAT_OVP_INIT)
			aw32280_fault_event_notify(POWER_NE_DC_FAULT_VBAT_OVP, data);
	} else if (flag1 & AW32280_IBAT_OVP_FLAG_MASK) {
		aw32280_get_ibat_ma(&val, di);
		hwlog_info("BAT OCP happened,ibat=%d\n", val);
		if (val >= AW32280_FWD_IBAT_OVP_INIT)
			aw32280_fault_event_notify(POWER_NE_DC_FAULT_IBAT_OCP, data);
	} else if (flag & AW32280_VBUS_OVP_FLAG_MASK) {
		aw32280_get_vbus_mv(&val, di);
		hwlog_info("BUS OVP happened,vbus=%d\n", val);
		if (aw32280_vbus_value_check(di, val))
			aw32280_fault_event_notify(POWER_NE_DC_FAULT_VBUS_OVP, data);
	} else if (flag2 & AW32280_IBUS_OVP_FLAG_MASK) {
		aw32280_get_ibus_ma(&val, di);
		hwlog_info("BUS OCP happened,ibus=%d\n", val);
		if (aw32280_ibus_value_check(di, val))
			aw32280_fault_event_notify(POWER_NE_DC_FAULT_IBUS_OCP, data);
	} else if (flag & AW32280_VOUT_OVP_FLAG_MASK) {
		hwlog_info("VOUT OVP happened\n");
	}
}

static void aw32280_interrupt_work(struct work_struct *work)
{
	u8 irq_flag2 = 0;
	u8 irq_flag3 = 0;
	u8 irq_flag4 = 0;
	u8 en_reg = 0;
	u8 mode_reg = 0;
	struct aw32280_device_info *di = NULL;
	struct nty_data *data = NULL;

	if (!work)
		return;

	di = container_of(work, struct aw32280_device_info, irq_work);
	if (!di || !di->client) {
		hwlog_err("di is null\n");
		return;
	}

	(void)aw32280_read_byte(di, AW32280_IRQ_FLAG_2_REG, &irq_flag2);
	(void)aw32280_read_byte(di, AW32280_IRQ_FLAG_3_REG, &irq_flag3);
	(void)aw32280_read_byte(di, AW32280_IRQ_FLAG_4_REG, &irq_flag4);
	(void)aw32280_read_byte(di, AW32280_SC_SC_EN_REG, &en_reg);
	(void)aw32280_read_byte(di, AW32280_SC_SC_MODE_REG, &mode_reg);

	data = &(di->nty_data);
	data->event1 = irq_flag2;
	data->event2 = irq_flag3;
	data->event3 = irq_flag4;
	data->addr = di->client->addr;

	if (di->int_notify_enable_flag) {
		aw32280_fault_handle(di, data);
		aw32280_dump_register(di);
	}

	hwlog_info("FLAG2 [0x%x]=0x%x, FLAG3 [0x%x]=0x%x, FLAG4 [0x%x]=0x%x\n",
		AW32280_IRQ_FLAG_2_REG, irq_flag2, AW32280_IRQ_FLAG_3_REG, irq_flag3,
		AW32280_IRQ_FLAG_4_REG, irq_flag4);
	hwlog_info("FLAG3 [0x%x]=0x%x, FLAG4 [0x%x]=0x%x\n",
		AW32280_SC_SC_EN_REG, en_reg, AW32280_SC_SC_MODE_REG, mode_reg);

	enable_irq(di->irq_int);
}

static irqreturn_t aw32280_interrupt(int irq, void *_di)
{
	struct aw32280_device_info *di = _di;

	if (!di)
		return IRQ_HANDLED;

	if (di->chip_already_init == 0)
		hwlog_err("chip not init\n");

	if (di->init_finish_flag == AW32280_INIT_FINISH)
		di->int_notify_enable_flag = AW32280_ENABLE_INT_NOTIFY;

	hwlog_info("int happened\n");
	disable_irq_nosync(di->irq_int);
	schedule_work(&di->irq_work);

	return IRQ_HANDLED;
}

static int aw32280_irq_init(struct aw32280_device_info *di,
	struct device_node *np)
{
	int ret;

	ret = power_gpio_config_interrupt(np,
		"gpio_int", "aw32280_gpio_int", &di->gpio_int, &di->irq_int);
	if (ret)
		return ret;

	ret = request_irq(di->irq_int, aw32280_interrupt,
		IRQF_TRIGGER_FALLING, "aw32280_int_irq", di);
	if (ret) {
		hwlog_err("gpio irq request fail\n");
		di->irq_int = -1;
		gpio_free(di->gpio_int);
		return ret;
	}

	enable_irq_wake(di->irq_int);
	INIT_WORK(&di->irq_work, aw32280_interrupt_work);

	return 0;
}

static int aw32280_gpio_init(struct aw32280_device_info *di,
	struct device_node *np)
{
	if (power_gpio_config_input(np,
		"gpio_enable", "aw32280_gpio_enable", &di->gpio_enable))
		return -1;

	if (power_gpio_config_input(np,
		"gpio_reset", "aw32280_gpio_reset", &di->gpio_reset))
		return -1;

	return 0;
}

static void aw32280_parse_dts(struct device_node *np,
	struct aw32280_device_info *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ic_role", &di->ic_role, CHARGE_IC_TYPE_MAIN);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"slave_mode", &di->slave_mode, AW32280_HOST_MODE);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"parallel_mode", &di->parallel_mode, AW32280_STANDALONE_MODE);
}

static int aw32280_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct aw32280_device_info *di = NULL;
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

	ret = aw32280_get_device_id(di);
	if (ret == AW32280_DEVICE_ID_GET_FAIL)
		goto aw32280_fail_0;

	aw32280_parse_dts(np, di);

	ret = aw32280_gpio_init(di, np);
	if (ret)
		goto aw32280_fail_0;

	ret = aw32280_irq_init(di, np);
	if (ret)
		goto aw32280_fail_0;

	ret = aw32280_reg_reset_and_init(di);
	if (ret)
		goto aw32280_fail_1;

	aw32280_ops_register(di);
	i2c_set_clientdata(client, di);

	return 0;

aw32280_fail_1:
	free_irq(di->irq_int, di);
	gpio_free(di->gpio_int);
aw32280_fail_0:
	di->chip_already_init = 0;
	devm_kfree(&client->dev, di);

	return ret;
}

static int aw32280_remove(struct i2c_client *client)
{
	struct aw32280_device_info *di = i2c_get_clientdata(client);

	if (!di)
		return -ENODEV;

	if (di->irq_int)
		free_irq(di->irq_int, di);

	if (di->gpio_int)
		gpio_free(di->gpio_int);

	return 0;
}

static void aw32280_shutdown(struct i2c_client *client)
{
	struct aw32280_device_info *di = i2c_get_clientdata(client);

	if (!di)
		return;

	aw32280_reg_reset(di);
}

#ifdef CONFIG_PM
static int aw32280_i2c_suspend(struct device *dev)
{
	return 0;
}

static int aw32280_i2c_resume(struct device *dev)
{
	return 0;
}

static const struct dev_pm_ops aw32280_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(aw32280_i2c_suspend, aw32280_i2c_resume)
};
#define AW32280_PM_OPS (&aw32280_pm_ops)
#else
#define AW32280_PM_OPS (NULL)
#endif /* CONFIG_PM */

MODULE_DEVICE_TABLE(i2c, aw32280);
static const struct of_device_id aw32280_of_match[] = {
	{
		.compatible = "aw32280",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id aw32280_i2c_id[] = {
	{ "aw32280", 0 },
	{}
};

static struct i2c_driver aw32280_driver = {
	.probe = aw32280_probe,
	.remove = aw32280_remove,
	.shutdown = aw32280_shutdown,
	.id_table = aw32280_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "aw32280",
		.of_match_table = of_match_ptr(aw32280_of_match),
		.pm = AW32280_PM_OPS,
	},
};

static int __init aw32280_init(void)
{
	return i2c_add_driver(&aw32280_driver);
}

static void __exit aw32280_exit(void)
{
	i2c_del_driver(&aw32280_driver);
}

module_init(aw32280_init);
module_exit(aw32280_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("aw32280 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
