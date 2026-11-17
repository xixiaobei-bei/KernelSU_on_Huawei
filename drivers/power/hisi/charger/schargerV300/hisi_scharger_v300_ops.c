/*
 * hisi_scharger_v300_ops.c
 *
 * Description: HI6523 charger operations inner api
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

#include "hisi_scharger_v300_common.h"
#include <linux/power/hisi/charger/hisi_charger_fcp.h>
#include <linux/power/hisi/charger/hisi_charger_scp.h>
#include <hisi_scharger_v300.h>

#ifdef CONFIG_DIRECT_CHARGER
static u32 scp_error_flag;
static int hi6523_is_support_scp(void);
static int g_direct_charge_mode;

static void scp_set_direct_charge_mode(int mode)
{
	g_direct_charge_mode = mode;
	scharger_inf("%s:g_direct_charge_mode=%d\n", __func__,
		     g_direct_charge_mode);
}

static int scp_get_direct_charge_mode(void)
{
	scharger_inf("%s:g_direct_charge_mode=%d\n", __func__,
		     g_direct_charge_mode);
	return g_direct_charge_mode;
}
#endif

#ifdef CONFIG_SWITCH_FSA9685
#include <huawei_platform/usb/switch/switch_fsa9685.h>
#endif

static unsigned int switch_id_flag; /* switch id status no need changed */

static int hi6523_fcp_get_adapter_output_current(void)
{
	return 0;
}

/* Output: fcp output vol(5V/9V/12V)*10 */
static int hi6523_fcp_get_adapter_output_vol(u8 *vol)
{
	u8 num = 0;
	u8 output_vol = 0;
	int ret;

	if (vol == NULL)
		return -1;
	/* get adapter vol list number,exclude 5V */
	ret = hi6523_fcp_adapter_reg_read(&num,
					  CHG_FCP_SLAVE_DISCRETE_CAPABILITIES);
	/* currently,fcp only support three out vol config(5v/9v/12v) */
	if (ret || num > 2) { /* 2: two modes */
		scharger_err("%s: vout list support err, reg[0x21] = %u\n",
			     __func__, num);
		return -1;
	}

	/* get max out vol value */
	ret = hi6523_fcp_adapter_reg_read(
		&output_vol, CHG_FCP_SLAVE_REG_DISCRETE_OUT_V(num));
	if (ret) {
		scharger_err("%s:get max out vol failed,ouputvol=%u,num=%u\n",
			     __func__, output_vol, num);
		return -1;
	}
	*vol = output_vol;
	scharger_inf("%s: get adapter max out vol = %u,num= %u\n",
		     __func__, output_vol, num);
	return 0;
}

/* check adapter voltage is around expected voltage */
static int hi6523_fcp_adapter_vol_check(int adapter_vol_mv)
{
	int i, ret;
	int adc_vol = 0;

	if ((adapter_vol_mv < FCP_ADAPTER_MIN_VOL) ||
		(adapter_vol_mv > FCP_ADAPTER_MAX_VOL)) {
		scharger_err("%s: check vol out of range, input vol = %dmV\n",
			     __func__, adapter_vol_mv);
		return -1;
	}

	for (i = 0; i < FCP_ADAPTER_VOL_CHECK_TIMEOUT; i++) {
		ret = hi6523_get_vbus_mv((unsigned int *)&adc_vol);
		if (ret)
			continue;

		if ((adc_vol > (adapter_vol_mv - FCP_ADAPTER_VOL_CHECK_ERROR)) &&
			(adc_vol < (adapter_vol_mv + FCP_ADAPTER_VOL_CHECK_ERROR))) {
			break;
		}
		msleep(FCP_ADAPTER_VOL_CHECK_POLLTIME);
	}

	if (i == FCP_ADAPTER_VOL_CHECK_TIMEOUT) {
		scharger_err("%s: check vol timeout, input vol = %dmV\n",
			     __func__, adapter_vol_mv);
		return -1;
	}
	scharger_inf("%s: check vol success, input vol = %dmV, spent %dms\n",
		     __func__, adapter_vol_mv,
		     i * FCP_ADAPTER_VOL_CHECK_POLLTIME);
	return 0;
}

static int hi6523_fcp_set_adapter_output_vol(int *output_vol)
{
	u8 val = 0;
	u8 vol = 0;
	int ret, ret0, ret1;

	if (output_vol == NULL)
		return -1;
	/* read ID OUTI, for identify huawei adapter */
	ret = hi6523_fcp_adapter_reg_read(&val, CHG_FCP_SLAVE_ID_OUT0);
	if (ret != 0) {
		scharger_err("%s: adapter ID OUTI read failed, ret is %d\n",
			     __func__, ret);
		return -1;
	}
	scharger_inf("%s: id out reg[0x4] = %u\n", __func__, val);

	/* get adapter max output vol value */
	ret = hi6523_fcp_get_adapter_output_vol(&vol);
	if (ret) {
		scharger_err("%s: fcp get adapter output vol err\n", __func__);
		return -1;
	}

	if (vol > CHG_FCP_OUTPUT_VOL_9V * CHG_FCP_VOL_STEP) {
		vol = CHG_FCP_OUTPUT_VOL_9V * CHG_FCP_VOL_STEP;
		scharger_inf("fcp limit adapter vol to 9V, while adapter support 12V\n");
	}
	*output_vol = vol / CHG_FCP_VOL_STEP;

	/* retry if write fail */
	ret0 = hi6523_fcp_adapter_reg_write(vol, CHG_FCP_SLAVE_VOUT_CONFIG);
	ret1 = hi6523_fcp_adapter_reg_read(&val, CHG_FCP_SLAVE_VOUT_CONFIG);
	scharger_inf("%s: vout config reg[0x2c] = %u\n", __func__, val);
	if (ret0 || ret1 || (val != vol)) {
		scharger_err("%s:out vol config err, reg[0x2c] = %u,vol :%u\n",
			     __func__, val, vol);
		return -1;
	}

	ret = hi6523_fcp_adapter_reg_write(CHG_FCP_SLAVE_SET_VOUT,
					   CHG_FCP_SLAVE_OUTPUT_CONTROL);
	if (ret) {
		scharger_err("%s:enable adapter output vol err\n", __func__);
		return -1;
	}
	/*
	 * code value transfer to actual value
	 * 1000: V to mV
	 */
	ret = hi6523_fcp_adapter_vol_check(vol / CHG_FCP_VOL_STEP * 1000); /* 1000 v to vm */
	if (ret) {
		scharger_err("%s:adc check adapter output vol err\n", __func__);
		return -1;
	}

	scharger_inf("fcp adapter output vol set ok\n");
	return 0;
}


static int hi6523_fcp_switch_to_soc(void)
{
	scharger_inf("%s\n", __func__);
#ifdef CONFIG_SWITCH_FSA9685
	usbswitch_common_manual_sw(FSA9685_USB1_ID_TO_IDBYPASS);
#endif
	return 0;
}

static int hi6523_fcp_switch_to_master(void)
{
	scharger_inf("%s\n", __func__);
#ifdef CONFIG_SWITCH_FSA9685
	usbswitch_common_manual_sw(FSA9685_USB2_ID_TO_IDBYPASS);
#endif
	return 0;
}

/*
 * Return: 0: success
 *        -1: other fail
 *         1: fcp adapter but detect fail
 */
static int hi6523_fcp_adapter_detect(void *dev_data)
{
	u8 reg_val1 = 0;
	u8 reg_val2 = 0;
	int i, ret, ret0, ret1, ret2, ret3;
	struct hi6523_device_info *di = get_hi6523_device();

	if (di == NULL) {
		scharger_err("%s hi6523_device_info is NULL!\n", __func__);
		return -ENOMEM;
	}
	mutex_lock(&di->fcp_detect_lock);
	ret = hi6523_read_byte(CHG_FCP_STATUS_REG, &reg_val2);
	if (ret) {
		scharger_err("%s:read det attach err,ret:%d\n", __func__, ret);
		mutex_unlock(&di->fcp_detect_lock);
		return -1;
	}

	if ((reg_val2 & (CHG_FCP_DVC_MSK | CHG_FCP_ATTATCH_MSK)) ==
		CHG_FCP_SLAVE_GOOD) {
		mutex_unlock(&di->fcp_detect_lock);
		scharger_inf("fcp adapter detect ok\n");
		return CHG_FCP_ADAPTER_DETECT_SUCC;
	}
	ret0 = hi6523_write_mask(CHG_FCP_DET_CTRL_REG, CHG_FCP_DET_EN_MSK,
				 CHG_FCP_DET_EN_SHIFT, TRUE);
	ret1 = hi6523_apple_adapter_detect(APPLE_DETECT_DISABLE);
	ret2 = hi6523_fcp_switch_to_master();
	ret3 = hi6523_write_mask(CHG_FCP_DET_CTRL_REG, CHG_FCP_CMP_EN_MSK,
				 CHG_FCP_CMP_EN_SHIFT, TRUE);
	ret = (ret0 || ret1 || ret2 || ret3);
	if (ret) {
		scharger_err("%s:FCP enable detect fail,ret:%d\n", __func__, ret);
		(void)hi6523_write_mask(CHG_FCP_DET_CTRL_REG,
					CHG_FCP_CMP_EN_MSK,
					CHG_FCP_CMP_EN_SHIFT, FALSE);
		(void)hi6523_fcp_switch_to_soc();
		(void)hi6523_write_mask(CHG_FCP_DET_CTRL_REG,
					CHG_FCP_DET_EN_MSK,
					CHG_FCP_DET_EN_SHIFT, FALSE);
		mutex_unlock(&di->fcp_detect_lock);
		return -1;
	}
	/* wait for fcp_set */
	for (i = 0; i < CHG_FCP_DETECT_MAX_COUT; i++) {
		ret = hi6523_read_byte(CHG_FCP_SET_STATUS_REG, &reg_val1);
		if (ret) {
			scharger_err("%s:read det attach err,ret:%d\n",
				     __func__, ret);
			continue;
		}
		if (reg_val1 & CHG_FCP_SET_STATUS_MSK)
			break;

		msleep(CHG_FCP_POLL_TIME);
	}
	if (i == CHG_FCP_DETECT_MAX_COUT) {
		(void)hi6523_write_mask(CHG_FCP_DET_CTRL_REG,
					CHG_FCP_CMP_EN_MSK,
					CHG_FCP_CMP_EN_SHIFT, FALSE);
		hi6523_fcp_switch_to_soc();
		(void)hi6523_write_mask(CHG_FCP_DET_CTRL_REG,
					CHG_FCP_DET_EN_MSK,
					CHG_FCP_DET_EN_SHIFT, FALSE);
		mutex_unlock(&di->fcp_detect_lock);
		return CHG_FCP_ADAPTER_DETECT_OTHER;
	}

	/* enable fcp_en */
	(void)hi6523_write_mask(CHG_FCP_CTRL_REG, CHG_FCP_EN_MSK,
				CHG_FCP_EN_SHIFT, TRUE);

	/* detect hisi fcp charger, wait for ping succ */
	for (i = 0; i < CHG_FCP_DETECT_MAX_COUT; i++) {
		ret = hi6523_read_byte(CHG_FCP_STATUS_REG, &reg_val2);
		if (ret) {
			scharger_err("%s:read det attach err,ret:%d\n",
				     __func__, ret);
			continue;
		}

		if (((reg_val2 & (CHG_FCP_DVC_MSK | CHG_FCP_ATTATCH_MSK)) ==
			CHG_FCP_SLAVE_GOOD))
			break;

		msleep(CHG_FCP_POLL_TIME);
	}

	if (i == CHG_FCP_DETECT_MAX_COUT) {
		(void)hi6523_write_mask(CHG_FCP_CTRL_REG, CHG_FCP_EN_MSK,
					CHG_FCP_EN_SHIFT, FALSE);
		(void)hi6523_write_mask(CHG_FCP_DET_CTRL_REG,
					CHG_FCP_CMP_EN_MSK,
					CHG_FCP_CMP_EN_SHIFT, FALSE);
		(void)hi6523_fcp_switch_to_soc();
		(void)hi6523_write_mask(CHG_FCP_DET_CTRL_REG,
					CHG_FCP_DET_EN_MSK,
					CHG_FCP_DET_EN_SHIFT, FALSE);
		scharger_err("fcp adapter detect failed,reg[0x%x]=0x%x\n",
			     CHG_FCP_STATUS_REG, reg_val2);
		mutex_unlock(&di->fcp_detect_lock);
		return CHG_FCP_ADAPTER_DETECT_FAIL; /* not fcp adapter */
	}
	scharger_inf("fcp adapter detect ok\n");
	mutex_unlock(&di->fcp_detect_lock);
	return CHG_FCP_ADAPTER_DETECT_SUCC;
}

static int fcp_adapter_detect(void)
{
	int ret;
#ifdef CONFIG_DIRECT_CHARGER
	u8 val;
#endif
	ret = hi6523_fcp_adapter_detect(get_hi6523_device());
	if (ret == CHG_FCP_ADAPTER_DETECT_OTHER) {
		scharger_inf("fcp adapter other detect\n");
		return FCP_ADAPTER_DETECT_OTHER;
	}
	if (ret == CHG_FCP_ADAPTER_DETECT_FAIL) {
		scharger_inf("fcp adapter detect fail\n");
		return FCP_ADAPTER_DETECT_FAIL;
	}
#ifdef CONFIG_DIRECT_CHARGER
	if (hi6523_is_support_scp())
		return FCP_ADAPTER_DETECT_SUCC;

	ret = hi6523_fcp_adapter_reg_read(&val, SCP_ADP_TYPE);
	if (ret) {
		scharger_err("%s:read SCP_ADP_TYPE err, ret=%d\n", __func__, ret);
		return FCP_ADAPTER_DETECT_SUCC;
	}
	return FCP_ADAPTER_DETECT_OTHER;
#else
	return FCP_ADAPTER_DETECT_SUCC;
#endif
}

static int hi6523_is_support_fcp(void)
{
	struct hi6523_device_info *di = get_hi6523_device();

	if (di == NULL) {
		scharger_err("%s hi6523_device_info is NULL!\n", __func__);
		return -ENOMEM;
	}
	if (di->param_dts.fcp_support != 0) {
		scharger_inf("support fcp charge\n");
		return 0;
	} else {
		return 1;
	}
}

static int hi6523_fcp_master_reset(void *dev_data)
{
	int ret0, ret1;

	ret0 = hi6523_write_byte(CHG_FCP_SOFT_RST_REG, CHG_FCP_SOFT_RST_VAL);
	/* clear fcp_en and fcp_master_rst */
	ret1 = hi6523_write_byte(CHG_FCP_CTRL_REG, 0);
	if (ret0 || ret1) {
		scharger_err("%s i2c err\n", __func__);
		return -1;
	}
	return 0;
}

static int hi6523_fcp_adapter_reset(void *dev_data)
{
	u8 val = 0;
	u8 output_vol = 0;
	int i;
	int ret, ret0, ret1;

	ret0 = hi6523_set_vbus_vset(VBUS_VSET_5V);
	ret1 = hi6523_fcp_adapter_reg_read((u8 *)&output_vol,
					   CHG_FCP_SLAVE_REG_DISCRETE_OUT_V(0));
	ret = (ret0 || ret1);
	if (ret) {
		scharger_err("%s get output_vol error\n", __func__);
		return ret;
	}

	/* retry if reset fail */
	for (i = 0; i < FCP_RESET_RETRY_TIME; i++) {
		ret0 = hi6523_fcp_adapter_reg_write(output_vol,
						    CHG_FCP_SLAVE_VOUT_CONFIG);
		ret1 = hi6523_fcp_adapter_reg_read(&val,
						   CHG_FCP_SLAVE_VOUT_CONFIG);
		scharger_inf("%s:vout config reg[0x2c] = %u\n", __func__, val);
		ret = (ret0 || ret1);
		if (ret || (val != output_vol)) {
			scharger_err("%s:set vout config err, reg[0x2c] = %u\n",
				     __func__, val);
			continue;
		}

		ret = hi6523_fcp_adapter_reg_write(
			CHG_FCP_SLAVE_SET_VOUT, CHG_FCP_SLAVE_OUTPUT_CONTROL);
		if (ret) {
			scharger_err("%s:enable adapter output voltage err\n",
				     __func__);
			continue;
		}
		scharger_inf("%s:enable adapter output voltage succ, i=%d\n",
			     __func__, i);
		break;
	}

	ret = hi6523_write_byte(CHG_FCP_CTRL_REG,
				CHG_FCP_EN_MSK | CHG_FCP_MSTR_RST_MSK);
	if (ret) {
		scharger_err("%s: send rst cmd failed\n", __func__);
		return ret;
	}
	/* after reset, v300 need 17ms */
	msleep(25); /* 25: need 25mS for vol check after set */
	ret = hi6523_fcp_adapter_vol_check(FCP_ADAPTER_RST_VOL);
	if (ret) {
		/* clear fcp_en and fcp_master_rst */
		ret0 = hi6523_write_byte(CHG_FCP_CTRL_REG, 0); /* 0: disable */
		if (ret0)
			scharger_err("%s:fcp ctrl reg write err\n", __func__);
		scharger_err("%s:adc check adapter output vol err\n", __func__);
		return ret;
	}
	/* clear fcp_en and fcp_master_rst */
	ret0 = hi6523_write_byte(CHG_FCP_CTRL_REG, 0); /* 0: disable */
	ret1 = hi6523_config_opt_param(VBUS_VSET_5V);
	scharger_inf("%s: fcp adapter output voltage reset succ\n", __func__);

	ret = (ret0 || ret1);
	return ret;
}

/* fcp config chip after stop charging */
static int hi6523_fcp_stop_charge_config(void *dev_data)
{
	scharger_inf("hi6523_fcp_master_reset");
	hi6523_fcp_master_reset(dev_data);
	hi6523_apple_adapter_detect(APPLE_DETECT_ENABLE);
	if (!switch_id_flag) /* when charge stop it not need change */
		hi6523_fcp_switch_to_soc();

	return 0;
}

static int is_fcp_charger_type(void *dev_data)
{
	u8 reg_val = 0;
	int ret;

	if (hi6523_is_support_fcp()) {
		scharger_err("%s:NOT SUPPORT FCP!\n", __func__);
		return FALSE;
	}
	ret = hi6523_read_byte(FCP_ADAPTER_CNTL_REG, &reg_val);
	if (ret) {
		scharger_err("%s reg read fail!\n", __func__);
		return FALSE;
	}
	if ((reg_val & HI6523_ACCP_CHARGER_DET) == HI6523_ACCP_CHARGER_DET)
		return TRUE;
	return FALSE;
}

static int fcp_read_adapter_status(void)
{
	u8 val = 0;
	int ret;

	ret = hi6523_fcp_adapter_reg_read(&val, FCP_ADAPTER_STATUS);
	if (ret != 0) {
		scharger_err("%s:read failed,ret=%d\n", __func__, ret);
		return 0;
	}
	scharger_inf("val is %u\n", val);

	if ((val & FCP_ADAPTER_OVLT) == FCP_ADAPTER_OVLT)
		return FCP_ADAPTER_OVLT;

	if ((val & FCP_ADAPTER_OCURRENT) == FCP_ADAPTER_OCURRENT)
		return FCP_ADAPTER_OCURRENT;

	if ((val & FCP_ADAPTER_OTEMP) == FCP_ADAPTER_OTEMP)
		return FCP_ADAPTER_OTEMP;
	return 0;
}

static int fcp_read_switch_status(void *dev_data)
{
	return 0;
}

static void hi6523_reg_dump(char *ptr)
{
}

#ifdef CONFIG_DIRECT_CHARGER
static int hi6523_is_support_scp(void)
{
	struct hi6523_device_info *di = get_hi6523_device();

	if (!di || !di->param_dts.scp_support)
		return HI6523_FAIL;

	return 0;
}

static int scp_adapter_reg_read(u8 *val, u8 reg)
{
	int ret;

	if (scp_error_flag) {
		scharger_err("%s:scp timeout happened,do not read reg=%u\n",
			     __func__, reg);
		return HI6523_FAIL;
	}
	hi6523_apple_adapter_detect(APPLE_DETECT_DISABLE);
	ret = hi6523_fcp_adapter_reg_read(val, reg);
	if (ret) {
		scharger_err("%s:error reg=%u\n", __func__, reg);
		scp_error_flag = SCP_IS_ERR;
		return HI6523_FAIL;
	}
	return 0;
}

static int scp_adapter_reg_write(u8 val, u8 reg)
{
	int ret;

	if (scp_error_flag) {
		scharger_err("%s:scp timeout happened,do not write reg=%u\n",
			     __func__, reg);
		return HI6523_FAIL;
	}
	hi6523_apple_adapter_detect(APPLE_DETECT_DISABLE);
	ret = hi6523_fcp_adapter_reg_write(val, reg);
	if (ret) {
		scharger_err("%s:error reg=%u\n", __func__, reg);
		scp_error_flag = SCP_IS_ERR;
		return HI6523_FAIL;
	}
	return 0;
}

static int hi6523_scp_adaptor_detect(void)
{
	int ret;
	u8 val = 0;
	struct hi6523_device_info *di = get_hi6523_device();

	scp_error_flag = SCP_NO_ERR;

	if (di == NULL) {
		scharger_err("%s hi6523_device_info is NULL!\n", __func__);
		return -ENOMEM;
	}

	di->adaptor_support = 0;

	ret = hi6523_fcp_adapter_detect(di);
	if (ret == CHG_FCP_ADAPTER_DETECT_OTHER) {
		scharger_inf("scp adapter other detect\n");
		return SCP_ADAPTOR_DETECT_OTHER;
	}
	if (ret == CHG_FCP_ADAPTER_DETECT_FAIL) {
		scharger_inf("scp adapter detect fail\n");
		return SCP_ADAPTOR_DETECT_FAIL;
	}

	ret = scp_adapter_reg_read(&val, SCP_ADP_TYPE0);
	if (ret || !(val & SCP_ADP_TYPE0_B_SC_MASK)) {
		if (ret)
			scharger_err("%s:read SCP_ADP_TYPE_0 fail,ret=%d\n",
				     __func__, ret);

		/* in case the LVC charger does not have 0x7e reg */
		scp_error_flag = SCP_NO_ERR;
		ret = scp_adapter_reg_read(&val, SCP_ADP_TYPE);
		if (ret) {
			scharger_err("%s:read SCP_ADP_TYPE fail,ret=%d\n",
				     __func__, ret);
			return SCP_ADAPTOR_DETECT_OTHER;
		}
		scharger_inf("%s:read SCP_ADP_TYPE val=%u\n", __func__, val);
		if ((val & SCP_ADP_TYPE_B_MASK) == SCP_ADP_TYPE_B) {
			scharger_inf("scp type b adapter detect\n");
			ret = scp_adapter_reg_read(&val, SCP_B_ADP_TYPE);
			if (ret) {
				scharger_err("%s:read SCP_B_ADP_TYPE fail, ret=%d\n",
					     __func__, ret);
				/* not scp adapter */
				return SCP_ADAPTOR_DETECT_OTHER;
			}
			scharger_inf("%s:read SCP_B_ADP_TYPE val=%u\n",
				     __func__, val);
			if (val == SCP_B_DIRECT_ADP) {
				scharger_inf("scp type b direct chg adapter detected\n");

				di->adaptor_support |= LVC_MODE;

				return SCP_ADAPTOR_DETECT_SUCC;
			}
		}
		return SCP_ADAPTOR_DETECT_OTHER;
	}
	scharger_inf("%s:read SCP_ADP_TYPE_0 val=%u\n", __func__, val);
	if (val) {
		if (val & SCP_ADP_TYPE0_B_SC_MASK) {
			scharger_inf("scp type b SC adapter detect\n");
			di->adaptor_support |= SC_MODE;
		}

		if (!(val & SCP_ADP_TYPE0_B_LVC_MASK)) {
			scharger_inf("scp type b lvc adapter detect\n");
			di->adaptor_support |= LVC_MODE;
		}

		scharger_inf("%s:adaptor_support=%d\n", __func__,
			     di->adaptor_support);
		ret = scp_adapter_reg_read(&val, SCP_B_ADP_TYPE);
		if (ret) {
			scharger_err("%s:read SCP_B_ADP_TYPE fail,ret=%d\n",
				     __func__, ret);
			return SCP_ADAPTOR_DETECT_OTHER; /* not scp adapter */
		}
		scharger_inf("%s:read SCP_B_ADP_TYPE val=%u\n", __func__, val);
		if (val == SCP_B_DIRECT_ADP) {
			scharger_inf("scp type b direct chg adapter detect\n");
			return SCP_ADAPTOR_DETECT_SUCC;
		}
	}

	return SCP_ADAPTOR_DETECT_OTHER;
}

static int hi6523_scp_get_adaptor_type(void)
{
	struct hi6523_device_info *di = get_hi6523_device();

	if (di == NULL) {
		scharger_err("%s hi6523_device_info is NULL!\n", __func__);
		return -ENOMEM;
	}

	scharger_inf("%s:adaptor_support=0x%x\n", __func__, di->adaptor_support);
	return (int)di->adaptor_support;
}

static int hi6523_scp_output_mode_enable(int enable)
{
	u8 val;
	int ret;

	ret = scp_adapter_reg_read(&val, SCP_CTRL_BYTE0);
	if (ret) {
		scharger_err("%s:read failed,ret=%d\n", __func__, ret);
		return HI6523_FAIL;
	}
	scharger_inf("[%s]val before is %u\n", __func__, val);
	val &= ~(SCP_OUTPUT_MODE_MASK);
	val |= enable ? SCP_OUTPUT_MODE_ENABLE : SCP_OUTPUT_MODE_DISABLE;
	scharger_inf("[%s]val after is %u\n", __func__, val);
	ret = scp_adapter_reg_write(val, SCP_CTRL_BYTE0);
	if (ret < 0) {
		scharger_err("%s:failed\n", __func__);
		return HI6523_FAIL;
	}
	return 0;
}

static int hi6523_scp_adaptor_output_enable(int enable)
{
	u8 val;
	int ret;

	ret = hi6523_scp_output_mode_enable(OUTPUT_MODE_ENABLE);
	if (ret) {
		scharger_err("%s:scp output mode enable failed,ret=%d\n",
			     __func__, ret);
		return HI6523_FAIL;
	}

	ret = scp_adapter_reg_read(&val, SCP_CTRL_BYTE0);
	if (ret) {
		scharger_err("%s:read failed,ret=%d\n", __func__, ret);
		return HI6523_FAIL;
	}
	scharger_inf("[%s]val before is %u\n", __func__, val);
	val &= ~(SCP_OUTPUT_MASK);
	val |= enable ? SCP_OUTPUT_ENABLE : SCP_OUTPUT_DISABLE;
	scharger_inf("[%s]val after is %u\n", __func__, val);
	ret = scp_adapter_reg_write(val, SCP_CTRL_BYTE0);
	if (ret < 0) {
		scharger_err("%s:failed\n", __func__);
		return HI6523_FAIL;
	}
	return 0;
}

static int hi6523_adaptor_reset(int enable)
{
	u8 val;
	int ret;

	ret = scp_adapter_reg_read(&val, SCP_CTRL_BYTE0);
	if (ret) {
		scharger_err("%s:read failed,ret=%d\n", __func__, ret);
		return HI6523_FAIL;
	}
	scharger_inf("[%s]val before is %u\n", __func__, val);
	val &= ~(SCP_ADAPTOR_RESET_MASK);
	val |= enable ? SCP_ADAPTOR_RESET_ENABLE : SCP_ADAPTOR_RESET_DISABLE;
	scharger_inf("[%s]val after is %u\n", __func__, val);
	ret = scp_adapter_reg_write(val, SCP_CTRL_BYTE0);
	if (ret < 0) {
		scharger_err("%s:failed\n", __func__);
		return HI6523_FAIL;
	}
	return 0;
}

static int hi6523_scp_config_iset_boundary(int iboundary_ma)
{
	u8 val;
	int ret;
	unsigned int iboundary = (unsigned int)iboundary_ma;

	/* high byte store in low address */
	val = (iboundary >> ONE_BYTE_LEN) & ONE_BYTE_MASK;
	ret = scp_adapter_reg_write(val, SCP_ISET_BOUNDARY_L);
	if (ret)
		return ret;
	/* low byte store in high address */
	val = iboundary & ONE_BYTE_MASK;
	ret = scp_adapter_reg_write(val, SCP_ISET_BOUNDARY_H);
	if (ret < 0)
		scharger_err("%s:failed\n", __func__);

	return ret;
}

static int hi6523_scp_config_vset_boundary(int vboundary_mv)
{
	u8 val;
	int ret;
	unsigned int vboundary = (unsigned int)vboundary_mv;

	/* high byte store in low address */
	val = (vboundary >> ONE_BYTE_LEN) & ONE_BYTE_MASK;
	ret = scp_adapter_reg_write(val, SCP_VSET_BOUNDARY_L);
	if (ret)
		return ret;
	/* low byte store in high address */
	val = vboundary & ONE_BYTE_MASK;
	ret = scp_adapter_reg_write(val, SCP_VSET_BOUNDARY_H);
	if (ret < 0)
		scharger_err("%s:failed\n", __func__);

	return ret;
}

static int hi6523_scp_set_adaptor_voltage(int vol)
{
	int val, ret, ret1;
	int dc_mode;
	u8 reg;

	dc_mode = scp_get_direct_charge_mode();
	if (dc_mode == LVC_MODE) {
		val = vol - VSSET_OFFSET;
		val = val / VSSET_STEP;
		ret = scp_adapter_reg_write((u8)val, SCP_VSSET);
		if (ret < 0) {
			scharger_err("%s:failed\n", __func__);
			return HI6523_FAIL;
		}
	} else if (dc_mode == SC_MODE) {
		/* high byte store in low address */
		reg = (((unsigned int)vol) >> ONE_BYTE_LEN) & ONE_BYTE_MASK;
		ret = scp_adapter_reg_write(reg, SCP_VSET_L);
		/* low byte store in high address */
		reg = ((unsigned int)vol) & ONE_BYTE_MASK;
		ret1 = scp_adapter_reg_write(reg, SCP_VSET_H);
		if (ret < 0 || ret1 < 0) {
			scharger_err("%s:failed\n", __func__);
			return HI6523_FAIL;
		}
	}

	return 0;
}

static int hi6523_scp_set_watchdog_timer(int second)
{
	u8 val;
	int ret;

	ret = scp_adapter_reg_read(&val, SCP_CTRL_BYTE1);
	if (ret) {
		scharger_err("%s:read failed,ret=%d\n", __func__, ret);
		return HI6523_FAIL;
	}
	scharger_inf("[%s]val before is %u\n", __func__, val);
	val &= ~(SCP_WATCHDOG_MASK);
	/* 1 bit means 0.5 second */
	val |= ((unsigned int)(second * ONE_BIT_EQUAL_TWO_SECONDS)) &
		SCP_WATCHDOG_MASK;
	scharger_inf("[%s]val after is %u\n", __func__, val);
	ret = scp_adapter_reg_write(val, SCP_CTRL_BYTE1);
	if (ret < 0) {
		scharger_err("%s:failed\n", __func__);
		return HI6523_FAIL;
	}
	return 0;
}

static int hi6523_scp_init(struct scp_init_data *sid)
{
	/* open 5v boost */
	int ret;
	u8 val;

	scp_error_flag = SCP_NO_ERR;
	ret = hi6523_scp_output_mode_enable(sid->scp_mode_enable);
	if (ret)
		return ret;
	ret = hi6523_scp_config_vset_boundary(sid->vset_boundary);
	if (ret)
		return ret;
	ret = hi6523_scp_config_iset_boundary(sid->iset_boundary);
	if (ret)
		return ret;
	ret = hi6523_scp_set_adaptor_voltage(sid->init_adaptor_voltage);
	if (ret)
		return ret;
	ret = hi6523_scp_set_watchdog_timer(sid->watchdog_timer);
	if (ret)
		return ret;
	ret = scp_adapter_reg_read(&val, SCP_CTRL_BYTE0);
	if (ret)
		return ret;
	scharger_inf("%s:CTRL_BYTE0=0x%x\n", __func__, val);
	ret = scp_adapter_reg_read(&val, SCP_CTRL_BYTE1);
	if (ret)
		return ret;
	scharger_inf("%s:CTRL_BYTE1=0x%x\n", __func__, val);
	ret = scp_adapter_reg_read(&val, SCP_STATUS_BYTE0);
	if (ret)
		return ret;
	scharger_inf("%s:STATUS_BYTE0=0x%x\n", __func__, val);
	ret = scp_adapter_reg_read(&val, SCP_STATUS_BYTE1);
	if (ret)
		return ret;
	scharger_inf("%s:STATUS_BYTE1=0x%x\n", __func__, val);
	ret = scp_adapter_reg_read(&val, SCP_VSET_BOUNDARY_H);
	if (ret)
		return ret;
	scharger_inf("%s:VSET_BOUNDARY_H=0x%x\n", __func__, val);
	ret = scp_adapter_reg_read(&val, SCP_VSET_BOUNDARY_L);
	if (ret)
		return ret;
	scharger_inf("%s:VSET_BOUNDARY_L=0x%x\n", __func__, val);
	ret = scp_adapter_reg_read(&val, SCP_ISET_BOUNDARY_H);
	if (ret)
		return ret;
	scharger_inf("%s:ISET_BOUNDARY_H=0x%x\n", __func__, val);
	ret = scp_adapter_reg_read(&val, SCP_ISET_BOUNDARY_L);
	if (ret)
		return ret;
	scharger_inf("%s:ISET_BOUNDARY_L=0x%x\n", __func__, val);
	return ret;
}

static int hi6523_scp_chip_reset(void *dev_data)
{
	int ret;

	ret = hi6523_fcp_master_reset(dev_data);
	if (ret) {
		scharger_inf("%s:hi6523_fcp_master_reset fail!\n", __func__);
		return HI6523_FAIL;
	}
	return 0;
}

static int hi6523_scp_exit(struct direct_charge_device *di)
{
	int ret0;
	int ret1 = 0;

	ret0 = hi6523_scp_output_mode_enable(OUTPUT_MODE_DISABLE);
	switch (di->adaptor_vendor_id) {
	case IWATT_ADAPTER:
		ret1 = hi6523_adaptor_reset(ADAPTOR_RESET);
		break;
	default:
		scharger_inf("%s:not iWatt\n", __func__);
		break;
	}
	msleep(WAIT_FOR_ADAPTOR_RESET);
	scharger_err("%s\n", __func__);
	scp_error_flag = SCP_NO_ERR;

	if (ret0 || ret1)
		return -1;
	return 0;
}

static int hi6523_scp_get_adaptor_voltage(void)
{
	u8 reg_val = 0;
	u8 reg_low = 0;
	u8 reg_high = 0;
	int val, ret, ret1, dc_mode;

	dc_mode = scp_get_direct_charge_mode();
	if (dc_mode == LVC_MODE) {
		ret = scp_adapter_reg_read(&reg_val, SCP_SREAD_VOUT);
		if (ret) {
			scharger_err("%s:read failed,ret=%d\n", __func__, ret);
			return HI6523_FAIL;
		}
		val = reg_val * SCP_SREAD_VOUT_STEP + SCP_SREAD_VOUT_OFFSET;
		scharger_inf("[%s]val is %d\n", __func__, val);
		return val;
	} else if (dc_mode == SC_MODE) {
		ret = scp_adapter_reg_read(&reg_low, SCP_READ_VOLT_L);
		ret1 = scp_adapter_reg_read(&reg_high, SCP_READ_VOLT_H);
		if (ret || ret1) {
			scharger_err("%s:read failed,ret=%d\n", __func__, ret);
			return HI6523_FAIL;
		}
		val = reg_low << ONE_BYTE_LEN;
		val += reg_high;

		scharger_inf("[%s]val is %d\n", __func__, val);
		return val;
	}
	return HI6523_FAIL;
}

static int hi6523_scp_set_adaptor_current(int cur)
{
	u8 val;
	int ret;

	val = cur / ISSET_STEP;
	ret = scp_adapter_reg_write(val, SCP_ISSET);
	if (ret < 0) {
		scharger_err("%s:failed\n", __func__);
		return HI6523_FAIL;
	}
	return 0;
}

static int hi6523_scp_get_adaptor_current(void)
{
	u8 reg_val;
	int val;
	int ret;

	ret = scp_adapter_reg_read(&reg_val, SCP_SREAD_IOUT);
	if (ret) {
		scharger_err("%s:read failed,ret=%d\n", __func__, ret);
		return HI6523_FAIL;
	}
	val = reg_val * SCP_SREAD_IOUT_STEP;
	scharger_inf("[%s]val is %d\n", __func__, val);
	return val;
}

static int hi6523_scp_get_adaptor_current_set(void)
{
	u8 reg_val;
	int val;
	int ret;

	ret = scp_adapter_reg_read(&reg_val, SCP_ISSET);
	if (ret) {
		scharger_err("%s:read failed,ret=%d\n", __func__, ret);
		return HI6523_FAIL;
	}
	val = reg_val * ISSET_STEP;
	scharger_inf("[%s]val is %d\n", __func__, val);
	return val;
}

static int hi6523_scp_get_adaptor_max_current(void)
{
	u8 reg_val;
	int ret, a, b, rs;

	ret = scp_adapter_reg_read(&reg_val, SCP_MAX_IOUT);
	if (ret) {
		scharger_err("%s:read MAX_IOUT failed,ret=%d\n", __func__, ret);
		return HI6523_FAIL;
	}
	scharger_inf("[%s]max_iout reg is %u\n", __func__, reg_val);
	a = (SCP_MAX_IOUT_A_MASK & reg_val) >> SCP_MAX_IOUT_A_SHIFT;
	b = SCP_MAX_IOUT_B_MASK & reg_val;
	switch (a) {
	case MAX_IOUT_EXP_0:
		a = TEN_EXP_0;
		break;
	case MAX_IOUT_EXP_1:
		a = TEN_EXP_1;
		break;
	case MAX_IOUT_EXP_2:
		a = TEN_EXP_2;
		break;
	case MAX_IOUT_EXP_3:
		a = TEN_EXP_3;
		break;
	default:
		return HI6523_FAIL;
	}
	rs = b * a;
	scharger_inf("[%s]MAX IOUT initial is %d\n", __func__, rs);
	ret = scp_adapter_reg_read(&reg_val, SCP_SSTS);
	if (ret) {
		scharger_err("%s:read SSTS failed,ret=%d\n", __func__, ret);
		return HI6523_FAIL;
	}
	scharger_inf("[%s]ssts reg is %u\n", __func__, reg_val);
	b = (SCP_SSTS_B_MASK & reg_val) >> SCP_SSTS_B_SHIFT;
	a = SCP_SSTS_A_MASK & reg_val;
	if (b == DROP_POWER_FLAG)
		rs = rs * a / DROP_POWER_FACTOR;

	scharger_inf("[%s]MAX IOUT final is %d\n", __func__, rs);
	return rs;
}

static int hi6523_scp_get_adaptor_temp(int *temp)
{
	u8 val = 0;
	int ret;

	ret = scp_adapter_reg_read(&val, SCP_INSIDE_TMP);
	if (ret) {
		scharger_err("%s:read failed,ret=%d\n", __func__, ret);
		return HI6523_FAIL;
	}
	scharger_inf("[%s]val is %u\n", __func__, val);
	*temp = val;

	return 0;
}

static int hi6523_scp_cable_detect(void)
{
	u8 val;
	int ret;

	ret = scp_adapter_reg_read(&val, SCP_STATUS_BYTE0);
	if (ret) {
		scharger_err("%s:read failed,ret=%d\n", __func__, ret);
		return SCP_CABLE_DETECT_ERROR;
	}
	scharger_inf("[%s]val is %u\n", __func__, val);
	if (val & SCP_CABLE_STS_MASK)
		return SCP_CABLE_DETECT_SUCC;

	return SCP_CABLE_DETECT_FAIL;
}

static int hi6523_scp_adaptor_reset(void *dev_data)
{
	return hi6523_fcp_adapter_reset(dev_data);
}

static int hi6523_scp_stop_charge_config(void)
{
	return 0;
}

static int hi6523_is_scp_charger_type(void)
{
	return is_fcp_charger_type(get_hi6523_device());
}

static int hi6523_scp_get_adaptor_status(void)
{
	return 0;
}

static int hi6523_scp_get_adaptor_info(void *info)
{
	int ret;
	struct adaptor_info *p = info;

	ret = scp_adapter_reg_read((u8 *)(&(p->b_adp_type)), SCP_B_ADP_TYPE);
	if (ret)
		return ret;
	ret = scp_adapter_reg_read((u8 *)(&(p->vendor_id_h)), SCP_VENDOR_ID_H);
	if (ret)
		return ret;
	ret = scp_adapter_reg_read((u8 *)(&(p->vendor_id_l)), SCP_VENDOR_ID_L);
	if (ret)
		return ret;
	ret = scp_adapter_reg_read((u8 *)(&(p->module_id_h)), SCP_MODULE_ID_H);
	if (ret)
		return ret;
	ret = scp_adapter_reg_read((u8 *)(&(p->module_id_l)), SCP_MODULE_ID_L);
	if (ret)
		return ret;
	ret = scp_adapter_reg_read((u8 *)(&(p->serrial_no_h)), SCP_SERRIAL_NO_H);
	if (ret)
		return ret;
	ret = scp_adapter_reg_read((u8 *)(&(p->serrial_no_l)), SCP_SERRIAL_NO_L);
	if (ret)
		return ret;
	ret = scp_adapter_reg_read((u8 *)(&(p->pchip_id)), SCP_PCHIP_ID);
	if (ret)
		return ret;
	ret = scp_adapter_reg_read((u8 *)(&(p->hwver)), SCP_HWVER);
	if (ret)
		return ret;
	ret = scp_adapter_reg_read((u8 *)(&(p->fwver_h)), SCP_FWVER_H);
	if (ret)
		return ret;
	ret = scp_adapter_reg_read((u8 *)(&(p->fwver_l)), SCP_FWVER_L);

	return ret;
}

static int hi6523_get_adapter_vendor_id(void)
{
	u8 val = 0;
	int ret;

	ret = scp_adapter_reg_read(&val, SCP_PCHIP_ID);
	if (ret) {
		scharger_err("%s:read failed,ret=%d\n", __func__, ret);
		return HI6523_FAIL;
	}
	scharger_inf("[%s]val is 0x%x\n", __func__, val);
	switch (val) {
	case VENDOR_ID_RICHTEK:
		scharger_inf("[%s]adapter is richtek\n", __func__);
		return RICHTEK_ADAPTER;
	case VENDOR_ID_IWATT:
		scharger_inf("[%s]adapter is iwatt\n", __func__);
		return IWATT_ADAPTER;
	default:
		scharger_inf("[%s]adaptor vendor id isn't found!\n", __func__);
		return val;
	}
}

static int hi6523_get_usb_port_leakage_current_info(void)
{
	u8 val = 0;
	int ret;

	ret = scp_adapter_reg_read(&val, SCP_STATUS_BYTE0);
	if (ret) {
		scharger_err("%s:read failed,ret=%d\n", __func__, ret);
		return HI6523_FAIL;
	}
	scharger_inf("[%s]val is 0x%x\n", __func__, val);
	val &= SCP_PORT_LEAKAGE_INFO;
	val = val >> SCP_PORT_LEAKAGE_SHIFT;
	scharger_inf("[%s]val is 0x%x\n", __func__, val);
	return val;
}

static int hi6523_scp_get_chip_status(void)
{
	return 0;
}

struct smart_charge_ops scp_hi6523_ops = {
	.is_support_scp = hi6523_is_support_scp,
	.scp_init = hi6523_scp_init,
	.scp_exit = hi6523_scp_exit,
	.scp_adaptor_detect = hi6523_scp_adaptor_detect,
	.scp_set_adaptor_voltage = hi6523_scp_set_adaptor_voltage,
	.scp_get_adaptor_voltage = hi6523_scp_get_adaptor_voltage,
	.scp_set_adaptor_current = hi6523_scp_set_adaptor_current,
	.scp_get_adaptor_current = hi6523_scp_get_adaptor_current,
	.scp_get_adaptor_current_set = hi6523_scp_get_adaptor_current_set,
	.scp_get_adaptor_max_current = hi6523_scp_get_adaptor_max_current,
	.scp_adaptor_output_enable = hi6523_scp_adaptor_output_enable,
	.scp_stop_charge_config = hi6523_scp_stop_charge_config,
	.is_scp_charger_type = hi6523_is_scp_charger_type,
	.scp_get_adaptor_status = hi6523_scp_get_adaptor_status,
	.scp_get_adaptor_info = hi6523_scp_get_adaptor_info,
	.scp_get_chip_status = hi6523_scp_get_chip_status,
	.scp_cable_detect = hi6523_scp_cable_detect,
	.scp_get_adaptor_temp = hi6523_scp_get_adaptor_temp,
	.scp_get_adapter_vendor_id = hi6523_get_adapter_vendor_id,
	.scp_get_usb_port_leakage_current_info =
	hi6523_get_usb_port_leakage_current_info,
	.scp_set_direct_charge_mode = scp_set_direct_charge_mode,
	.scp_get_adaptor_type = hi6523_scp_get_adaptor_type,
};
#endif

struct fcp_adapter_device_ops fcp_hi6523_ops = {
	.get_adapter_output_current = hi6523_fcp_get_adapter_output_current,
	.set_adapter_output_vol = hi6523_fcp_set_adapter_output_vol,
	.detect_adapter = fcp_adapter_detect,
	.is_support_fcp = hi6523_is_support_fcp,
	.switch_chip_reset = hi6523_fcp_master_reset,
	.fcp_adapter_reset = hi6523_fcp_adapter_reset,
	.stop_charge_config = hi6523_fcp_stop_charge_config,
	.is_fcp_charger_type = is_fcp_charger_type,
	.fcp_read_adapter_status = fcp_read_adapter_status,
	.fcp_read_switch_status = fcp_read_switch_status,
	.reg_dump = hi6523_reg_dump,
};

static int hi6523_fcp_reg_read_block(int reg, int *val, int num,
	void *dev_data)
{
	int ret;
	int i;
	u8 data = 0;

	if (val == NULL) {
		scharger_err("val is null\n");
		return -1;
	}

	for (i = 0; i < num; i++) {
		ret = hi6523_fcp_adapter_reg_read(&data, reg + i);
		if (ret) {
			scharger_err("fcp read failed(reg=0x%x)\n", reg + i);
			return -1;
		}

		val[i] = data;
	}

	return 0;
}

static int hi6523_fcp_reg_write_block(int reg, const int *val, int num,
	void *dev_data)
{
	int ret;
	int i;

	if (val == NULL) {
		scharger_err("val is null\n");
		return -1;
	}

	for (i = 0; i < num; i++) {
		ret = hi6523_fcp_adapter_reg_write(val[i], reg + i);
		if (ret) {
			scharger_err("fcp write failed(reg=0x%x)\n", reg + i);
			return -1;
		}
	}

	return 0;
}

static struct fcp_protocol_ops hi6523_fcp_protocol_ops = {
	.chip_name = "scharger_v300",
	.reg_read = hi6523_fcp_reg_read_block,
	.reg_write = hi6523_fcp_reg_write_block,
	.detect_adapter = hi6523_fcp_adapter_detect,
	.soft_reset_master = hi6523_fcp_master_reset,
	.soft_reset_slave = hi6523_fcp_adapter_reset,
	.get_master_status = fcp_read_switch_status,
	.stop_charging_config = hi6523_fcp_stop_charge_config,
	.is_accp_charger_type = is_fcp_charger_type,
};

#ifdef CONFIG_DIRECT_CHARGER
static int hi6523_scp_reg_read_block(int reg, int *val, int num,
	void *dev_data)
{
	int ret;
	int i;
	u8 data = 0;

	if (val == NULL) {
		scharger_err("val is null\n");
		return -1;
	}

	scp_error_flag = SCP_NO_ERR;

	for (i = 0; i < num; i++) {
		ret = scp_adapter_reg_read(&data, reg + i);
		if (ret) {
			scharger_err("scp read failed(reg=0x%x)\n", reg + i);
			return -1;
		}

		val[i] = data;
	}
	return 0;
}

static int hi6523_scp_reg_write_block(int reg, const int *val, int num,
	void *dev_data)
{
	int ret;
	int i;

	if (val == NULL) {
		scharger_err("val is null\n");
		return -1;
	}

	scp_error_flag = SCP_NO_ERR;

	for (i = 0; i < num; i++) {
		ret = scp_adapter_reg_write(val[i], reg + i);
		if (ret) {
			scharger_err("scp write failed(reg=0x%x)\n", reg + i);
			return -1;
		}
	}
	return 0;
}

static struct scp_protocol_ops hi6523_scp_protocol_ops = {
	.chip_name = "scharger_v300",
	.reg_read = hi6523_scp_reg_read_block,
	.reg_write = hi6523_scp_reg_write_block,
	.detect_adapter = hi6523_fcp_adapter_detect,
	.soft_reset_master = hi6523_scp_chip_reset,
	.soft_reset_slave = hi6523_scp_adaptor_reset,
};
#endif /* CONFIG_DIRECT_CHARGER */

void hi6523_fcp_scp_ops_register(void)
{
	/* if chip support fcp, register fcp adapter ops */
	if (hi6523_is_support_fcp() == 0)
		(void)fcp_protocol_ops_register(&hi6523_fcp_protocol_ops);

#ifdef CONFIG_DIRECT_CHARGER
	/* if chip support scp, register scp adapter ops */
	if (hi6523_is_support_scp() == 0)
		scp_protocol_ops_register(&hi6523_scp_protocol_ops);
#endif /* CONFIG_DIRECT_CHARGER */
}
