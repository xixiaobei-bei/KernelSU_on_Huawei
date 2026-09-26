/*
 * sc8545.c
 *
 * sc8545 direct charge driver
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

#include "sc8545.h"
#include "sc8545_i2c.h"
#include "sc8545_protocol.h"
#include <chipset_common/hwpower/common_module/power_log.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <huawei_platform/power/direct_charger/direct_charger.h>

#define HWLOG_TAG sc8545
HWLOG_REGIST();

static void sc8545_dump_register(void *dev_data)
{
	int i;
	u8 flag = 0;
	struct sc8545_device_info *di = dev_data;

	for (i = SC8545_VBATREG_REG; i <= SC8545_PMID2OUT_OVP_UVP_REG; i++) {
		sc8545_read_byte(di, i, &flag);
		hwlog_info("reg[0x%x]=0x%x\n", i, flag);
	}
}

static int sc8545_discharge(int enable, void *dev_data)
{
	int ret;
	u8 reg = 0;
	u8 value = enable ? 0x1 : 0x0;
	struct sc8545_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = sc8545_write_mask(di, SC8545_CTRL2_REG,
		SC8545_VBUS_PD_EN_MASK, SC8545_VBUS_PD_EN_SHIFT, value);
	ret += sc8545_read_byte(di, SC8545_CTRL2_REG, &reg);
	if (ret)
		return -EIO;

	if (enable) {
		/* delay 50ms to disable vbus_pd manually */
		(void)power_msleep(DT_MSLEEP_50MS, 0, NULL);
		(void)sc8545_write_mask(di, SC8545_CTRL2_REG,
			SC8545_VBUS_PD_EN_MASK, SC8545_VBUS_PD_EN_SHIFT, 0);
	}

	hwlog_info("discharge [%x]=0x%x\n", SC8545_CTRL2_REG, reg);
	return 0;
}

static int sc8545_is_device_close(void *dev_data)
{
	u8 reg = 0;
	int ret;
	struct sc8545_device_info *di = dev_data;

	if (!di)
		return 1;

	ret = sc8545_read_byte(di, SC8545_CONVERTER_STATE_REG, &reg);
	if (ret)
		return 1;

	if (reg & SC8545_CP_SWITCHING_STAT_MASK)
		return 0;

	return 1;
}

static int sc8545_get_device_id(void *dev_data)
{
	u8 dev_id = 0;
	int ret;
	static bool first_rd;
	struct sc8545_device_info *di = dev_data;

	if (!di)
		return SC8545_DEVICE_ID_GET_FAIL;

	if (first_rd)
		return di->device_id;

	first_rd = true;
	ret = sc8545_read_byte(di, SC8545_DEVICE_ID_REG, &dev_id);
	if (ret) {
		first_rd = false;
		hwlog_err("get_device_id read fail\n");
		return SC8545_DEVICE_ID_GET_FAIL;
	}
	hwlog_info("get_device_id [%x]=0x%x\n", SC8545_DEVICE_ID_REG, dev_id);

	if (dev_id == SC8545_DEVICE_ID_VALUE)
		di->device_id = SWITCHCAP_SC8545;
	else
		di->device_id = SC8545_DEVICE_ID_GET_FAIL;

	hwlog_info("device_id: 0x%x\n", di->device_id);

	return di->device_id;
}

static int sc8545_get_vbat_mv(void *dev_data)
{
	s16 data = 0;
	int ret;
	int vbat;
	struct sc8545_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = sc8545_read_word(di, SC8545_VBAT_ADC1_REG, &data);
	if (ret)
		return -EIO;

	/* VBAT ADC LSB: 1.25mV */
	vbat = (int)data * 125 / 100;

	hwlog_info("VBAT_ADC=0x%x, vbat=%d\n", data, vbat);
	return vbat;
}

static int sc8545_get_ibat_ma(int *ibat, void *dev_data)
{
	int ret;
	s16 data = 0;
	struct sc8545_device_info *di = dev_data;

	if (!di || !ibat)
		return -EINVAL;

	ret = sc8545_read_word(di, SC8545_IBAT_ADC1_REG, &data);
	if (ret)
		return -EIO;

	/* IBAT ADC LSB: 3.125mA */
	*ibat = ((int)data * 3125 / 1000) * di->sense_r_config;
	*ibat /= di->sense_r_actual;

	hwlog_info("IBAT_ADC=0x%x, ibat=%d\n", data, *ibat);
	return 0;
}

static int sc8545_get_ibus_ma(int *ibus, void *dev_data)
{
	s16 data = 0;
	int ret;
	struct sc8545_device_info *di = dev_data;

	if (!di || !ibus)
		return -EINVAL;

	ret = sc8545_read_word(di, SC8545_IBUS_ADC1_REG, &data);
	if (ret)
		return -EIO;

	/* IBUS ADC LSB: 1.875mA */
	*ibus = (int)data * 1875 / 1000;

	hwlog_info("IBUS_ADC=0x%x, ibus=%d\n", data, *ibus);
	return 0;
}

int sc8545_get_vbus_mv(int *vbus, void *dev_data)
{
	int ret;
	s16 data = 0;
	struct sc8545_device_info *di = dev_data;

	if (!di || !vbus)
		return -EINVAL;

	ret = sc8545_read_word(di, SC8545_VBUS_ADC1_REG, &data);
	if (ret)
		return -EIO;

	/* VBUS ADC LSB: 3.75mV */
	*vbus = (int)data * 375 / 100;

	hwlog_info("VBUS_ADC=0x%x, vbus=%d\n", data, *vbus);
	return 0;
}

static int sc8545_get_vusb_mv(int *vusb, void *dev_data)
{
	int ret;
	s16 data = 0;
	struct sc8545_device_info *di = dev_data;

	ret = sc8545_read_word(di, SC8545_VAC_ADC1_REG, &data);
	if (ret)
		return -EIO;

	/* VUSB ADC LSB: 5mV */
	*vusb = (int)data * 5;

	hwlog_info("VUSB_ADC=0x%x, vusb=%d\n", data, *vusb);
	return 0;
}

static int sc8545_get_device_temp(int *temp, void *dev_data)
{
	s16 data = 0;
	int ret;
	struct sc8545_device_info *di = dev_data;

	if (!di || !temp)
		return -EINVAL;

	ret = sc8545_read_word(di, SC8545_TDIE_ADC1_REG, &data);
	if (ret)
		return -EIO;

	/* TDIE_ADC LSB: 0.5C */
	*temp = (int)data * 5 / 10;

	hwlog_info("TDIE_ADC=0x%x, temp=%d\n", data, *temp);
	return 0;
}

static int sc8545_config_watchdog_ms(int time, void *dev_data)
{
	int ret;
	u8 val;
	u8 reg = 0;
	struct sc8545_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	if (time >= SC8545_WTD_CONFIG_TIMING_30000MS)
		val = SC8545_WTD_SET_30000MS;
	else if (time >= SC8545_WTD_CONFIG_TIMING_5000MS)
		val = SC8545_WTD_SET_5000MS;
	else if (time >= SC8545_WTD_CONFIG_TIMING_1000MS)
		val = SC8545_WTD_SET_1000MS;
	else if (time >= SC8545_WTD_CONFIG_TIMING_500MS)
		val = SC8545_WTD_SET_500MS;
	else if (time >= SC8545_WTD_CONFIG_TIMING_200MS)
		val = SC8545_WTD_SET_200MS;
	else
		val = SC8545_WTD_CONFIG_TIMING_DIS;

	ret = sc8545_write_mask(di, SC8545_CTRL3_REG, SC8545_WTD_TIMEOUT_MASK,
		SC8545_WTD_TIMEOUT_SHIFT, val);
	ret += sc8545_read_byte(di, SC8545_CTRL3_REG, &reg);
	if (ret)
		return -EIO;

	hwlog_info("config_watchdog_ms [%x]=0x%x\n", SC8545_CTRL3_REG, reg);
	return 0;
}

static int sc8545_config_vbat_ovp_th_mv(void *dev_data, int ovp_th)
{
	u8 value;
	int ret;
	struct sc8545_device_info *di = dev_data;

	if (ovp_th < SC8545_VBAT_OVP_BASE)
		ovp_th = SC8545_VBAT_OVP_BASE;

	if (ovp_th > SC8545_VBAT_OVP_MAX)
		ovp_th = SC8545_VBAT_OVP_MAX;

	value = (u8)((ovp_th - SC8545_VBAT_OVP_BASE) / SC8545_VBAT_OVP_STEP);
	ret = sc8545_write_mask(di, SC8545_VBAT_OVP_REG, SC8545_VBAT_OVP_MASK,
		SC8545_VBAT_OVP_SHIFT, value);
	ret += sc8545_read_byte(di, SC8545_VBAT_OVP_REG, &value);
	if (ret)
		return -EIO;

	hwlog_info("config_vbat_ovp_threshold_mv [%x]=0x%x\n",
		SC8545_VBAT_OVP_REG, value);
	return 0;
}

static int sc8545_config_vbat_regulation(void *dev_data, int vbat_regulation)
{
	u8 value;
	int ret;
	struct sc8545_device_info *di = dev_data;

	if (vbat_regulation < SC8545_VBATREG_BELOW_OVP_BASE)
		vbat_regulation = SC8545_VBATREG_BELOW_OVP_BASE;

	if (vbat_regulation > SC8545_VBATREG_BELOW_OVP_MAX)
		vbat_regulation = SC8545_VBATREG_BELOW_OVP_MAX;

	value = (u8)((vbat_regulation - SC8545_VBATREG_BELOW_OVP_BASE) /
		SC8545_VBATREG_BELOW_OVP_STEP);
	ret = sc8545_write_mask(di, SC8545_VBATREG_REG,
		SC8545_VBATREG_BELOW_OVP_MASK,
		SC8545_VBATREG_BELOW_OVP_SHIFT, value);
	ret += sc8545_write_mask(di, SC8545_VBATREG_REG, SC8545_VBATREG_EN_MASK,
		SC8545_VBATREG_EN_SHIFT, 1);
	ret += sc8545_read_byte(di, SC8545_VBATREG_REG, &value);
	if (ret)
		return -EIO;

	hwlog_info("config_vbat_reg_threshold_mv [%x]=0x%x\n",
		SC8545_IBATREG_REG, value);
	return 0;
}

static int sc8545_config_ibat_ocp_th_ma(void *dev_data, int ocp_th)
{
	u8 value;
	int ret;
	struct sc8545_device_info *di = dev_data;

	if (ocp_th < SC8545_IBAT_OCP_BASE)
		ocp_th = SC8545_IBAT_OCP_BASE;

	if (ocp_th > SC8545_IBAT_OCP_MAX)
		ocp_th = SC8545_IBAT_OCP_MAX;

	value = (u8)((ocp_th - SC8545_IBAT_OCP_BASE) / SC8545_IBAT_OCP_STEP);
	ret = sc8545_write_mask(di, SC8545_IBAT_OCP_REG, SC8545_IBAT_OCP_MASK,
		SC8545_IBAT_OCP_SHIFT, value);
	ret += sc8545_read_byte(di, SC8545_IBAT_OCP_REG, &value);
	if (ret)
		return -EIO;

	hwlog_info("config_ibat_ocp_threshold_ma [%x]=0x%x\n",
		SC8545_IBAT_OCP_REG, value);
	return 0;
}

static int sc8545_config_ibat_regulation(void *dev_data, int ibat_regulation)
{
	u8 value;
	int ret;
	struct sc8545_device_info *di = dev_data;

	if (ibat_regulation < SC8545_IBATREG_BELOW_OCP_BASE)
		ibat_regulation = SC8545_IBATREG_BELOW_OCP_BASE;

	if (ibat_regulation > SC8545_IBATREG_BELOW_OCP_MAX)
		ibat_regulation = SC8545_IBATREG_BELOW_OCP_MAX;

	value = (u8)((ibat_regulation - SC8545_IBATREG_BELOW_OCP_BASE) /
		SC8545_IBATREG_BELOW_OCP_STEP);
	ret = sc8545_write_mask(di, SC8545_IBATREG_REG,
		SC8545_IBATREG_BELOW_OCP_MASK,
		SC8545_IBATREG_BELOW_OCP_SHIFT, value);
	ret += sc8545_write_mask(di, SC8545_IBATREG_REG, SC8545_IBATREG_EN_MASK,
		SC8545_IBATREG_EN_SHIFT, 1);
	ret += sc8545_read_byte(di, SC8545_IBATREG_REG, &value);
	if (ret)
		return -EIO;

	hwlog_info("config_ibat_reg_regulation_mv [%x]=0x%x\n",
		SC8545_IBATREG_REG, value);
	return 0;
}

int sc8545_config_vac_ovp_th_mv(void *dev_data, int ovp_th)
{
	u8 value;
	int ret;
	struct sc8545_device_info *di = dev_data;

	if (ovp_th < SC8545_VAC_OVP_BASE)
		ovp_th = SC8545_VAC_OVP_DEFAULT;

	if (ovp_th > SC8545_VAC_OVP_MAX)
		ovp_th = SC8545_VAC_OVP_MAX;

	value = (u8)((ovp_th - SC8545_VAC_OVP_BASE) / SC8545_VAC_OVP_STEP);
	ret = sc8545_write_mask(di, SC8545_VAC_OVP_REG, SC8545_VAC_OVP_MASK,
		SC8545_VAC_OVP_SHIFT, value);
	ret += sc8545_read_byte(di, SC8545_VAC_OVP_REG, &value);
	if (ret)
		return -EIO;

	hwlog_info("config_ac_ovp_threshold_mv [%x]=0x%x\n",
		SC8545_VAC_OVP_REG, value);
	return 0;
}

int sc8545_config_vbus_ovp_th_mv(void *dev_data, int ovp_th)
{
	u8 value;
	int ret;
	struct sc8545_device_info *di = dev_data;

	if (ovp_th < SC8545_VBUS_OVP_BASE)
		ovp_th = SC8545_VBUS_OVP_BASE;

	if (ovp_th > SC8545_VBUS_OVP_MAX)
		ovp_th = SC8545_VBUS_OVP_MAX;

	value = (u8)((ovp_th - SC8545_VBUS_OVP_BASE) / SC8545_VBUS_OVP_STEP);
	ret = sc8545_write_mask(di, SC8545_VBUS_OVP_REG, SC8545_VBUS_OVP_MASK,
		SC8545_VBUS_OVP_SHIFT, value);
	ret += sc8545_read_byte(di, SC8545_VBUS_OVP_REG, &value);
	if (ret)
		return -EIO;

	hwlog_info("config_vbus_ovp_threshole_mv [%x]=0x%x\n",
		SC8545_VBUS_OVP_REG, value);
	return 0;
}

static int sc8545_config_ibus_ocp_th_ma(void *dev_data, int ocp_th)
{
	u8 value;
	int ret;
	struct sc8545_device_info *di = dev_data;

	if (ocp_th < SC8545_IBUS_OCP_BASE)
		ocp_th = SC8545_IBUS_OCP_BASE;

	if (ocp_th > SC8545_IBUS_OCP_MAX)
		ocp_th = SC8545_IBUS_OCP_MAX;

	value = (u8)((ocp_th - SC8545_IBUS_OCP_BASE) / SC8545_IBUS_OCP_STEP);
	ret = sc8545_write_mask(di, SC8545_IBUS_OCP_UCP_REG,
		SC8545_IBUS_OCP_MASK, SC8545_IBUS_OCP_SHIFT, value);
	ret += sc8545_read_byte(di, SC8545_IBUS_OCP_UCP_REG, &value);
	if (ret)
		return -EIO;

	hwlog_info("config_ibus_ocp_threshold_ma [%x]=0x%x\n",
		SC8545_IBUS_OCP_UCP_REG, value);
	return 0;
}

static int sc8545_config_switching_frequency(int data, void *dev_data)
{
	int freq;
	int freq_shift;
	int ret;
	struct sc8545_device_info *di = dev_data;

	switch (data) {
	case SC8545_SW_FREQ_450KHZ:
		freq = SC8545_FSW_SET_SW_FREQ_500KHZ;
		freq_shift = SC8545_SW_FREQ_SHIFT_M_P10;
		break;
	case SC8545_SW_FREQ_500KHZ:
		freq = SC8545_FSW_SET_SW_FREQ_500KHZ;
		freq_shift = SC8545_SW_FREQ_SHIFT_NORMAL;
		break;
	case SC8545_SW_FREQ_550KHZ:
		freq = SC8545_FSW_SET_SW_FREQ_500KHZ;
		freq_shift = SC8545_SW_FREQ_SHIFT_P_P10;
		break;
	case SC8545_SW_FREQ_675KHZ:
		freq = SC8545_FSW_SET_SW_FREQ_750KHZ;
		freq_shift = SC8545_SW_FREQ_SHIFT_M_P10;
		break;
	case SC8545_SW_FREQ_750KHZ:
		freq = SC8545_FSW_SET_SW_FREQ_750KHZ;
		freq_shift = SC8545_SW_FREQ_SHIFT_NORMAL;
		break;
	case SC8545_SW_FREQ_825KHZ:
		freq = SC8545_FSW_SET_SW_FREQ_750KHZ;
		freq_shift = SC8545_SW_FREQ_SHIFT_P_P10;
		break;
	default:
		freq = SC8545_FSW_SET_SW_FREQ_500KHZ;
		freq_shift = SC8545_SW_FREQ_SHIFT_NORMAL;
		break;
	}

	ret = sc8545_write_mask(di, SC8545_CTRL1_REG, SC8545_SW_FREQ_MASK,
		SC8545_SW_FREQ_SHIFT, freq);
	ret += sc8545_write_mask(di, SC8545_CTRL1_REG, SC8545_FREQ_SHIFT_MASK,
		SC8545_FREQ_SHIFT_SHIFT, freq_shift);
	if (ret)
		return -EIO;

	hwlog_info("config_switching_frequency [%x]=0x%x\n",
		SC8545_CTRL1_REG, freq);
	hwlog_info("config_adjustable_switching_frequency [%x]=0x%x\n",
		SC8545_CTRL1_REG, freq_shift);

	return 0;
}

static int sc8545_congfig_ibat_sns_res(void *dev_data)
{
	int ret;
	u8 value;
	struct sc8545_device_info *di = dev_data;

	if (di->sense_r_config == SENSE_R_5_MOHM)
		value = SC8545_IBAT_SNS_RES_5MOHM;
	else
		value = SC8545_IBAT_SNS_RES_2MOHM;

	ret = sc8545_write_mask(di, SC8545_CTRL2_REG,
		SC8545_SET_IBAT_SNS_RES_MASK,
		SC8545_SET_IBAT_SNS_RES_SHIFT, value);
	if (ret)
		return -EIO;

	hwlog_info("congfig_ibat_sns_res=%d\n", di->sense_r_config);
	return 0;
}

static int sc8545_threshold_reg_init(void *dev_data, u8 mode)
{
	int ret;
	int ocp_th;
	struct sc8545_device_info *di = dev_data;

	if (mode == SC8545_CHG_MODE_BYPASS)
		ocp_th = SC8545_LVC_IBUS_OCP_TH_INIT;
	else if (mode == SC8545_CHG_MODE_CHGPUMP)
		ocp_th = SC8545_SC_IBUS_OCP_TH_INIT;
	else
		ocp_th = SC8545_IBUS_OCP_TH_INIT;

	ret = sc8545_config_vac_ovp_th_mv(di, SC8545_VAC_OVP_TH_INIT);
	ret += sc8545_config_vbus_ovp_th_mv(di, SC8545_VBUS_OVP_TH_INIT);
	ret += sc8545_config_ibus_ocp_th_ma(di, ocp_th);
	ret += sc8545_config_vbat_ovp_th_mv(di, SC8545_VBAT_OVP_TH_INIT);
	ret += sc8545_config_ibat_ocp_th_ma(di, SC8545_IBAT_OCP_TH_INIT);
	ret += sc8545_config_ibat_regulation(di, SC8545_IBAT_REGULATION_TH_INIT);
	ret += sc8545_config_vbat_regulation(di, SC8545_VBAT_REGULATION_TH_INIT);
	if (ret)
		hwlog_err("protect threshold init fail\n");

	return ret;
}

static int sc8545_lvc_charge_enable(int enable, void *dev_data)
{
	int ret;
	u8 ctrl1_reg = 0;
	u8 ctrl3_reg = 0;
	u8 mode = SC8545_CHG_MODE_BYPASS;
	u8 chg_en = enable ? SC8545_CHG_CTRL_ENABLE : SC8545_CHG_CTRL_DISABLE;
	struct sc8545_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = sc8545_threshold_reg_init(di, mode);
	ret += sc8545_write_mask(di, SC8545_CTRL1_REG, SC8545_CHG_CTRL_MASK,
		SC8545_CHG_CTRL_SHIFT, SC8545_CHG_CTRL_DISABLE);
	ret += sc8545_write_mask(di, SC8545_CTRL3_REG, SC8545_CHG_MODE_MASK,
		SC8545_CHG_MODE_SHIFT, mode);
	ret += sc8545_write_mask(di, SC8545_CTRL1_REG, SC8545_CHG_CTRL_MASK,
		SC8545_CHG_CTRL_SHIFT, chg_en);
	ret += sc8545_read_byte(di, SC8545_CTRL1_REG, &ctrl1_reg);
	ret += sc8545_read_byte(di, SC8545_CTRL3_REG, &ctrl3_reg);
	if (ret)
		return -EIO;

	hwlog_info("ic_role=%d, charge_enable [%x]=0x%x,[%x]=0x%x\n",
		di->ic_role, SC8545_CTRL1_REG, ctrl1_reg,
		SC8545_CTRL3_REG, ctrl3_reg);
	return 0;
}

static int sc8545_sc_charge_enable(int enable, void *dev_data)
{
	int ret;
	u8 ctrl1_reg = 0;
	u8 mode = SC8545_CHG_MODE_CHGPUMP;
	u8 chg_en = enable ? SC8545_CHG_CTRL_ENABLE : SC8545_CHG_CTRL_DISABLE;
	struct sc8545_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = sc8545_threshold_reg_init(di, mode);
	ret += sc8545_write_mask(di, SC8545_CTRL1_REG, SC8545_CHG_CTRL_MASK,
		SC8545_CHG_CTRL_SHIFT, SC8545_CHG_CTRL_DISABLE);
	ret += sc8545_write_mask(di, SC8545_CTRL3_REG, SC8545_CHG_MODE_MASK,
		SC8545_CHG_MODE_SHIFT, mode);
	ret += sc8545_write_mask(di, SC8545_CTRL1_REG, SC8545_CHG_CTRL_MASK,
		SC8545_CHG_CTRL_SHIFT, chg_en);
	ret += sc8545_read_byte(di, SC8545_CTRL1_REG, &ctrl1_reg);
	if (ret)
		return -EIO;

	hwlog_info("ic_role=%d, charge_enable [%x]=0x%x\n",
		di->ic_role, SC8545_CTRL1_REG, ctrl1_reg);
	return 0;
}

static int sc8545_reg_reset(void *dev_data)
{
	int ret;
	u8 ctrl1_reg = 0;
	struct sc8545_device_info *di = dev_data;

	ret = sc8545_write_mask(di, SC8545_CTRL1_REG,
		SC8545_REG_RST_MASK, SC8545_REG_RST_SHIFT,
		SC8545_REG_RST_ENABLE);
	if (ret)
		return -EIO;

	power_usleep(DT_USLEEP_1MS);
	ret = sc8545_read_byte(di, SC8545_CTRL1_REG, &ctrl1_reg);
	ret += sc8545_config_vac_ovp_th_mv(di, SC8545_VAC_OVP_TH_INIT);
	if (ret)
		return -EIO;

	hwlog_info("reg_reset [%x]=0x%x\n", SC8545_CTRL1_REG, ctrl1_reg);
	return 0;
}

static int sc8545_chip_init(void *dev_data)
{
	return 0;
}

static int sc8545_reg_init(void *dev_data)
{
	int ret;
	u8 mode = SC8545_CHG_MODE_BUCK;
	struct sc8545_device_info *di = dev_data;

	ret = sc8545_config_watchdog_ms(SC8545_WTD_CONFIG_TIMING_5000MS, di);
	ret += sc8545_threshold_reg_init(di, mode);
	ret += sc8545_config_switching_frequency(di->switching_frequency, di);
	ret += sc8545_congfig_ibat_sns_res(di);
	ret += sc8545_write_byte(di, SC8545_INT_MASK_REG,
		SC8545_INT_MASK_REG_INIT);
	ret += sc8545_write_mask(di, SC8545_ADCCTRL_REG,
		SC8545_ADC_EN_MASK, SC8545_ADC_EN_SHIFT, 1);
	ret += sc8545_write_mask(di, SC8545_ADCCTRL_REG,
		SC8545_ADC_RATE_MASK, SC8545_ADC_RATE_SHIFT, 0);
	ret += sc8545_write_mask(di, SC8545_DPDM_CTRL2_REG,
		SC8545_DP_BUFF_EN_MASK, SC8545_DP_BUFF_EN_SHIFT, 1);
	ret += sc8545_write_byte(di, SC8545_SCP_FLAG_MASK_REG,
		SC8545_SCP_FLAG_MASK_REG_INIT);
	if (ret)
		hwlog_err("reg_init fail\n");

	return ret;
}

static int sc8545_charge_init(void *dev_data)
{
	struct sc8545_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	di->device_id = sc8545_get_device_id((void *)di);
	if (di->device_id == SC8545_DEVICE_ID_GET_FAIL)
		return -EINVAL;

	if (sc8545_reg_init(di))
		return -EINVAL;

	di->init_finish_flag = true;
	return 0;
}

static int sc8545_reg_reset_and_init(void *dev_data)
{
	int ret;
	struct sc8545_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = sc8545_reg_reset(di);
	ret += sc8545_reg_init(di);

	return ret;
}

static int sc8545_charge_exit(void *dev_data)
{
	int ret;
	struct sc8545_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = sc8545_sc_charge_enable(SC8545_SWITCHCAP_DISABLE, di);
	di->fcp_support = false;
	di->init_finish_flag = false;
	di->int_notify_enable_flag = false;

	return ret;
}

static int sc8545_batinfo_exit(void *dev_data)
{
	return 0;
}

static int sc8545_batinfo_init(void *dev_data)
{
	struct sc8545_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	return sc8545_chip_init(di);
}

/* print the register head in charging process */
static int sc8545_register_head(char *buffer, int size, void *dev_data)
{
	struct sc8545_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	if (di->ic_role == CHARGE_IC_TYPE_MAIN)
		snprintf(buffer, size,
			"   mode   Ibus   Vbus   Vusb   Ibat   Vbat   Temp");
	else
		snprintf(buffer, size,
			"   mode1  Ibus1  Vbus1  Vusb1  Ibat1  Vbat1  Temp1");

	return 0;
}

/* print the register value in charging process */
static int sc8545_dump_reg(char *buffer, int size, void *dev_data)
{
	int vbus = 0;
	int ibat = 0;
	int temp = 0;
	int ibus = 0;
	int vusb = 0;
	char buff[SC8545_BUF_LEN] = { 0 };
	u8 reg = 0;
	struct sc8545_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	(void)sc8545_get_vbus_mv(&vbus, di);
	(void)sc8545_get_ibat_ma(&ibat, di);
	(void)sc8545_get_ibus_ma(&ibus, di);
	(void)sc8545_get_device_temp(&temp, di);
	(void)sc8545_get_vusb_mv(&vusb, di);
	(void)sc8545_read_byte(di, SC8545_CTRL3_REG, &reg);

	if (sc8545_is_device_close(di))
		snprintf(buff, sizeof(buff), "%s", "OFF    ");
	else if (((reg & SC8545_CHG_MODE_MASK) >> SC8545_CHG_MODE_SHIFT) ==
		SC8545_CHG_MODE_BYPASS)
		snprintf(buff, sizeof(buff), "%s", "LVC    ");
	else if (((reg & SC8545_CHG_MODE_MASK) >> SC8545_CHG_MODE_SHIFT) ==
		SC8545_CHG_MODE_CHGPUMP)
		snprintf(buff, sizeof(buff), "%s", "SC     ");

	strncat(buffer, buff, strlen(buff));
	snprintf(buff, sizeof(buff), "%-7d%-7d%-7d%-7d%-7d%-7d",
		ibus, vbus, vusb, ibat, sc8545_get_vbat_mv(di), temp);
	strncat(buffer, buff, strlen(buff));

	return 0;
}

static struct dc_ic_ops g_sc8545_lvc_ops = {
	.dev_name = "sc8545",
	.ic_init = sc8545_charge_init,
	.ic_exit = sc8545_charge_exit,
	.ic_enable = sc8545_lvc_charge_enable,
	.ic_discharge = sc8545_discharge,
	.is_ic_close = sc8545_is_device_close,
	.get_ic_id = sc8545_get_device_id,
	.config_ic_watchdog = sc8545_config_watchdog_ms,
	.ic_reg_reset_and_init = sc8545_reg_reset_and_init,
};

static struct dc_ic_ops g_sc8545_sc_ops = {
	.dev_name = "sc8545",
	.ic_init = sc8545_charge_init,
	.ic_exit = sc8545_charge_exit,
	.ic_enable = sc8545_sc_charge_enable,
	.ic_discharge = sc8545_discharge,
	.is_ic_close = sc8545_is_device_close,
	.get_ic_id = sc8545_get_device_id,
	.config_ic_watchdog = sc8545_config_watchdog_ms,
	.ic_reg_reset_and_init = sc8545_reg_reset_and_init,
};

static struct dc_batinfo_ops g_sc8545_batinfo_ops = {
	.init = sc8545_batinfo_init,
	.exit = sc8545_batinfo_exit,
	.get_bat_btb_voltage = sc8545_get_vbat_mv,
	.get_bat_package_voltage = sc8545_get_vbat_mv,
	.get_vbus_voltage = sc8545_get_vbus_mv,
	.get_bat_current = sc8545_get_ibat_ma,
	.get_ic_ibus = sc8545_get_ibus_ma,
	.get_ic_temp = sc8545_get_device_temp,
};

static struct power_log_ops g_sc8545_log_ops = {
	.dev_name = "sc8545",
	.dump_log_head = sc8545_register_head,
	.dump_log_content = sc8545_dump_reg,
};

static struct dc_ic_ops g_sc8545_aux_lvc_ops = {
	.dev_name = "sc8545_aux",
	.ic_init = sc8545_charge_init,
	.ic_exit = sc8545_charge_exit,
	.ic_enable = sc8545_lvc_charge_enable,
	.ic_discharge = sc8545_discharge,
	.is_ic_close = sc8545_is_device_close,
	.get_ic_id = sc8545_get_device_id,
	.config_ic_watchdog = sc8545_config_watchdog_ms,
	.ic_reg_reset_and_init = sc8545_reg_reset_and_init,
};

static struct dc_ic_ops g_sc8545_aux_sc_ops = {
	.dev_name = "sc8545_aux",
	.ic_init = sc8545_charge_init,
	.ic_exit = sc8545_charge_exit,
	.ic_enable = sc8545_sc_charge_enable,
	.ic_discharge = sc8545_discharge,
	.is_ic_close = sc8545_is_device_close,
	.get_ic_id = sc8545_get_device_id,
	.config_ic_watchdog = sc8545_config_watchdog_ms,
	.ic_reg_reset_and_init = sc8545_reg_reset_and_init,
};

static struct dc_batinfo_ops g_sc8545_aux_batinfo_ops = {
	.init = sc8545_batinfo_init,
	.exit = sc8545_batinfo_exit,
	.get_bat_btb_voltage = sc8545_get_vbat_mv,
	.get_bat_package_voltage = sc8545_get_vbat_mv,
	.get_vbus_voltage = sc8545_get_vbus_mv,
	.get_bat_current = sc8545_get_ibat_ma,
	.get_ic_ibus = sc8545_get_ibus_ma,
	.get_ic_temp = sc8545_get_device_temp,
};

static struct power_log_ops g_sc8545_aux_log_ops = {
	.dev_name = "sc8545_aux",
	.dump_log_head = sc8545_register_head,
	.dump_log_content = sc8545_dump_reg,
};

static void sc8545_init_ops_dev_data(struct sc8545_device_info *di)
{
	if (di->ic_role == CHARGE_IC_TYPE_MAIN) {
		g_sc8545_lvc_ops.dev_data = (void *)di;
		g_sc8545_sc_ops.dev_data = (void *)di;
		g_sc8545_batinfo_ops.dev_data = (void *)di;
		g_sc8545_log_ops.dev_data = (void *)di;
	} else {
		g_sc8545_aux_lvc_ops.dev_data = (void *)di;
		g_sc8545_aux_sc_ops.dev_data = (void *)di;
		g_sc8545_aux_batinfo_ops.dev_data = (void *)di;
		g_sc8545_aux_log_ops.dev_data = (void *)di;
	}
}

static int sc8545_ops_register(struct sc8545_device_info *di)
{
	int ret;

	sc8545_init_ops_dev_data(di);

	if (di->ic_role == CHARGE_IC_TYPE_MAIN) {
		ret = dc_ic_ops_register(LVC_MODE, di->ic_role,
			&g_sc8545_lvc_ops);
		ret += dc_ic_ops_register(SC_MODE, di->ic_role,
			&g_sc8545_sc_ops);
		ret += dc_batinfo_ops_register(SC_MODE, di->ic_role,
			&g_sc8545_batinfo_ops);
		ret += dc_batinfo_ops_register(LVC_MODE, di->ic_role,
			&g_sc8545_batinfo_ops);
		ret += power_log_ops_register(&g_sc8545_log_ops);
		ret += sc8545_protocol_ops_register(di);
	} else {
		ret = dc_ic_ops_register(LVC_MODE, di->ic_role,
			&g_sc8545_aux_lvc_ops);
		ret += dc_ic_ops_register(SC_MODE, di->ic_role,
			&g_sc8545_aux_sc_ops);
		ret += dc_batinfo_ops_register(SC_MODE, di->ic_role,
			&g_sc8545_aux_batinfo_ops);
		ret += dc_batinfo_ops_register(LVC_MODE, di->ic_role,
			&g_sc8545_aux_batinfo_ops);
		ret += power_log_ops_register(&g_sc8545_aux_log_ops);
		ret += sc8545_protocol_ops_register(di);
	}
	if (ret)
		hwlog_err("ops_register fail\n");

	return ret;
}

static void sc8545_fault_event_notify(unsigned long event, void *data)
{
	power_event_anc_notify(POWER_ANT_SC_FAULT, event, data);
}

static void sc8545_interrupt_handle(struct sc8545_device_info *di,
	struct nty_data *data)
{
	int val = 0;
	u8 flag = data->event1;
	u8 flag1 = data->event2;
	u8 scp_flag = data->event3;

	if (flag1 & SC8545_VAC_OVP_FLAG_MASK) {
		hwlog_info("AC OVP happened\n");
		sc8545_fault_event_notify(POWER_NE_DC_FAULT_AC_OVP, data);
	} else if (flag & SC8545_VBAT_OVP_FLAG_MASK) {
		val = sc8545_get_vbat_mv(di);
		hwlog_info("BAT OVP happened, vbat=%d\n", val);
		if (val >= SC8545_VBAT_OVP_TH_INIT)
			sc8545_fault_event_notify(POWER_NE_DC_FAULT_VBAT_OVP, data);
	} else if (flag & SC8545_IBAT_OCP_FLAG_MASK) {
		sc8545_get_ibat_ma(&val, di);
		hwlog_info("BAT OCP happened,ibat=%d\n", val);
		if (val >= SC8545_IBAT_OCP_TH_INIT)
			sc8545_fault_event_notify(POWER_NE_DC_FAULT_IBAT_OCP, data);
	} else if (flag & SC8545_VBUS_OVP_FLAG_MASK) {
		sc8545_get_vbus_mv(&val, di);
		hwlog_info("BUS OVP happened,vbus=%d\n", val);
		if (val >= SC8545_VBUS_OVP_TH_INIT)
			sc8545_fault_event_notify(POWER_NE_DC_FAULT_VBUS_OVP, data);
	} else if (flag & SC8545_IBUS_OCP_FLAG_MASK) {
		sc8545_get_ibus_ma(&val, di);
		hwlog_info("BUS OCP happened,ibus=%d\n", val);
		if (val >= SC8545_IBUS_OCP_TH_INIT)
			sc8545_fault_event_notify(POWER_NE_DC_FAULT_IBUS_OCP, data);
	} else if (flag & SC8545_VOUT_OVP_FLAG_MASK) {
		hwlog_info("VOUT OVP happened\n");
	}

	if (scp_flag & SC8545_TRANS_DONE_FLAG_MASK)
		di->scp_trans_done = true;
}

static void sc8545_interrupt_work(struct work_struct *work)
{
	u8 flag[6] = { 0 };
	struct sc8545_device_info *di = NULL;
	struct nty_data *data = NULL;

	if (!work)
		return;

	di = container_of(work, struct sc8545_device_info, irq_work);
	if (!di || !di->client) {
		hwlog_err("di is null\n");
		return;
	}

	(void)sc8545_read_byte(di, SC8545_INT_FLAG_REG, &flag[0]);
	(void)sc8545_read_byte(di, SC8545_VAC_OVP_REG, &flag[1]);
	(void)sc8545_read_byte(di, SC8545_VDROP_OVP_REG, &flag[2]);
	(void)sc8545_read_byte(di, SC8545_CONVERTER_STATE_REG, &flag[3]);
	(void)sc8545_read_byte(di, SC8545_CTRL3_REG, &flag[4]);
	(void)sc8545_read_byte(di, SC8545_SCP_FLAG_MASK_REG, &flag[5]);

	data = &(di->notify_data);
	data->event1 = flag[0];
	data->event2 = flag[1];
	data->event3 = flag[5];
	data->addr = di->client->addr;

	if (di->int_notify_enable_flag) {
		sc8545_interrupt_handle(di, data);
		sc8545_dump_register(di);
	}

	hwlog_info("FLAG0 [0x%x]=0x%x, FLAG1 [0x%x]=0x%x, FLAG2 [0x%x]=0x%x\n",
		SC8545_INT_FLAG_REG, flag[0], SC8545_VAC_OVP_REG, flag[1],
		SC8545_VDROP_OVP_REG, flag[2]);
	hwlog_info("FLAG3 [0x%x]=0x%x, FLAG4 [0x%x]=0x%x, FLAG5 [0x%x]=0x%x\n",
		SC8545_CONVERTER_STATE_REG, flag[3], SC8545_CTRL3_REG, flag[4],
		SC8545_SCP_FLAG_MASK_REG, flag[5]);

	enable_irq(di->irq_int);
}

static irqreturn_t sc8545_interrupt(int irq, void *_di)
{
	struct sc8545_device_info *di = _di;

	if (!di)
		return IRQ_HANDLED;

	if (di->init_finish_flag)
		di->int_notify_enable_flag = true;

	hwlog_info("int happened\n");

	disable_irq_nosync(di->irq_int);
	schedule_work(&di->irq_work);

	return IRQ_HANDLED;
}

static int sc8545_irq_init(struct sc8545_device_info *di,
	struct device_node *np)
{
	int ret;

	ret = power_gpio_config_interrupt(np,
		"intr_gpio", "sc8545_gpio_int", &di->gpio_int, &di->irq_int);
	if (ret)
		return ret;

	ret = request_irq(di->irq_int, sc8545_interrupt,
		IRQF_TRIGGER_FALLING, "sc8545_int_irq", di);
	if (ret) {
		hwlog_err("gpio irq request fail\n");
		di->irq_int = -1;
		gpio_free(di->gpio_int);
		return ret;
	}

	enable_irq_wake(di->irq_int);
	INIT_WORK(&di->irq_work, sc8545_interrupt_work);
	return 0;
}

static void sc8545_parse_dts(struct device_node *np,
	struct sc8545_device_info *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"switching_frequency", &di->switching_frequency,
		SC8545_SW_FREQ_550KHZ);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "scp_support",
		(u32 *)&(di->dts_scp_support), 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "fcp_support",
		(u32 *)&(di->dts_fcp_support), 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "ic_role",
		(u32 *)&(di->ic_role), CHARGE_IC_TYPE_MAIN);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"sense_r_config", &di->sense_r_config, SENSE_R_5_MOHM);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"sense_r_actual", &di->sense_r_actual, SENSE_R_5_MOHM);
}

static void sc8545_init_lock_mutex(struct sc8545_device_info *di)
{
	mutex_init(&di->scp_detect_lock);
	mutex_init(&di->accp_adapter_reg_lock);
}

static void sc8545_destroy_lock_mutex(struct sc8545_device_info *di)
{
	mutex_destroy(&di->scp_detect_lock);
	mutex_destroy(&di->accp_adapter_reg_lock);
}

static int sc8545_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct sc8545_device_info *di = NULL;
	struct device_node *np = NULL;

	if (!client || !client->dev.of_node || !id)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &client->dev;
	np = di->dev->of_node;
	di->client = client;
	i2c_set_clientdata(client, di);
	di->chip_already_init = 1;

	ret = sc8545_get_device_id(di);
	if (ret == SC8545_DEVICE_ID_GET_FAIL)
		goto sc8545_fail_0;

	sc8545_parse_dts(np, di);
	sc8545_init_lock_mutex(di);

	ret = sc8545_irq_init(di, np);
	if (ret)
		goto sc8545_fail_1;

	ret = sc8545_ops_register(di);
	if (ret)
		goto sc8545_fail_2;

	ret = sc8545_reg_reset(di);
	if (ret)
		goto sc8545_fail_2;

	ret = sc8545_reg_init(di);
	if (ret)
		goto sc8545_fail_2;

	return 0;

sc8545_fail_2:
	free_irq(di->irq_int, di);
	gpio_free(di->gpio_int);
sc8545_fail_1:
	sc8545_destroy_lock_mutex(di);
sc8545_fail_0:
	di->chip_already_init = 0;
	devm_kfree(&client->dev, di);

	return ret;
}

static int sc8545_remove(struct i2c_client *client)
{
	struct sc8545_device_info *di = i2c_get_clientdata(client);

	if (!di)
		return -ENODEV;

	sc8545_reg_reset(di);

	if (di->irq_int)
		free_irq(di->irq_int, di);
	if (di->gpio_int)
		gpio_free(di->gpio_int);
	sc8545_destroy_lock_mutex(di);

	return 0;
}

static void sc8545_shutdown(struct i2c_client *client)
{
	struct sc8545_device_info *di = i2c_get_clientdata(client);

	if (!di)
		return;

	sc8545_reg_reset(di);
}

MODULE_DEVICE_TABLE(i2c, sc8545);
static const struct of_device_id sc8545_of_match[] = {
	{
		.compatible = "sm5450",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id sc8545_i2c_id[] = {
	{ "sm5450", 0 }, {}
};

static struct i2c_driver sc8545_driver = {
	.probe = sc8545_probe,
	.remove = sc8545_remove,
	.shutdown = sc8545_shutdown,
	.id_table = sc8545_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "sc8545",
		.of_match_table = of_match_ptr(sc8545_of_match),
	},
};

static int __init sc8545_init(void)
{
	return i2c_add_driver(&sc8545_driver);
}

static void __exit sc8545_exit(void)
{
	i2c_del_driver(&sc8545_driver);
}

module_init(sc8545_init);
module_exit(sc8545_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("sc8545 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
