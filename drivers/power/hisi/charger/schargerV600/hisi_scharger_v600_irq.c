/*
 * hisi_scharger_v600_irq.c
 *
 * HI6526 charger irq handler inner api
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#include "hisi_scharger_v600.h"

static char *g_buck_irq_str[] = {

	/*
	 * IRQ_FLAG_0 bit7 - bit0
	 * bit[7]:irq_chg_chgstate
	 * 1: charging process is terminated
	 * 0: charging process is not terminated
	 * bit[6]:irq_vbus_ovp BUCK vbus over voltage protection when charging
	 * bit[5]:irq_vbus_uvp BUCK vbus under voltage protection when charging
	 * bit[4]:irq_vbat_ovp in BUCK mode, charger IC provides
	 *        over voltage protection for battery
	 * bit[3]:irq_otg_ovp, boost status reminder, OTG ovp protection
	 * bit[2]:irq_otg_uvp,OTG uvp protection
	 * bit[1]:irq_buck_ocp buck over current protection
	 * bit[0]:irq_otg_scp OTG scp interrupt protection
	 */
	"irq_otg_scp ",
	"irq_buck_ocp ",
	"irq_otg_uvp ",
	"irq_otg_ovp ",
	"irq_vbat_ovp ",
	"irq_vbus_uvp ",
	"irq_vbus_ovp ",
	"irq_chg_chgstate ",

	/*
	 * IRQ_FLAG_3 bit0 - bit7
	 * bit[7]:irq_otg_ocp, OTG ocp protection
	 * bit[6]:irq_buck_scp, BUCK short circuit protection
	 * bit[5]:irq_trickle_chg_fault, trickle charge fault status record
	 * bit[4]:irq_pre_chg_fault, pre charge fault status record
	 * bit[3]:irq_fast_chg_fault, fast charge fault status record
	 * bit[2]:irq_otmp_jreg, 1: when temperature is over the
	 *        preset temperature value
	 * bit[1]:irq_chg_rechg_state, recharge status reminder
	 * bit[0]:irq_sleep_mod, sleep interrupt signal
	 */
	"irq_sleep_mod ",
	"irq_chg_rechg_state ",
	"irq_otmp_jreg ",
	"irq_fast_chg_fault ",
	"irq_pre_chg_fault ",
	"irq_trickle_chg_fault ",
	"irq_buck_scp ",
	"irq_otg_ocp ",

	/*
	 * IRQ_FLAG_4 bit7 - bit5
	 * bit[7]:irq_reversbst, reversbst report interrupt
	 * bit[6]:irq_vusb_buck_ovp, ovp interrupt signal in buck mode
	 * bit[5]:irq_vusb_otg_ovp, ovp interrupt signal in otg mode
	 */
	"none ",
	"none ",
	"none ",
	"none ",
	"none ",
	"irq_vusb_otg_ovp ",
	"irq_vusb_buck_ovp ",
	"irq_reversbst ",
};

static char *g_lvc_sc_irq_str[] = {
	/* IRQ_FLAG_2 bit0 - bit4 */
	"irq_ibus_dc_ucp ",
	"irq_ibus_dc_ocp ",
	"irq_vbus_lvc_uv ",
	"irq_vbus_lvc_ov ",
	"irq_vusb_lvc_ovp ",
	"none ",
	"none ",
	"none ",

	/* IRQ_FLAG_3 bit0 - bit7 */
	"irq_ibat_clamp ",
	"irq_ibus_clamp ",
	"irq_vout_ov_clamp ",
	"irq_bat_ov_clamp ",
	"irq_vusb_scauto_ovp ",
	"irq_vbat_dc_ovp ",
	"irq_vdrop_lvc_ov ",
	"irq_ibus_dc_rcp ",

	/* IRQ_FLAG_4 bit0 - bit7 */
	"irq_ilim_bus_sc_ocp ",
	"irq_ilim_sc_ocp ",
	"irq_ibat_dcucp_alm ",
	"irq_ibat_dc_ocp ",
	"irq_vbat_sc_uvp ",
	"irq_vin2vout_sc ",
	"irq_vbus_sc_uv ",
	"irq_vbus_sc_ov ",
};

static char *g_others_irq_str[] = {
	/* IRQ_FLAG_5 bit4 - bit0
	 * bit[4]:irq_acr_scp, ACR scp protection, this interrupt is invalid
	 * due to the lack of external discharge circuit, the internal cuicuit
	 * is connected to ground
	 * bit[3]:soh_ovh, SOH OVP detect higher threshold warning interrupt
	 * bit[2]:soh_ovl, SOH OVP detect lower threshold warning interrupt
	 * bit[1]:acr_flag, ACR sampling completed interrupt
	 * bit[0]:irq_wdt, watchdog interrupt
	 */
	"irq_wdt ",
	"acr_flag ",
	"soh_ovl ",
	"soh_ovh ",
	"irq_acr_scp ",
	"none ",
	"none ",
	"none ",

	/*
	 * IRQ_FLAG_6 bit7 - bit0
	 * bit[7]:irq_regn_ocp, ldo5 over current signal
	 * bit[6]:irq_chg_batfet_ocp, batfet 12A over current protect state reg
	 * bit[5]:irq_otmp_140, over temperature protection
	 * bit[4]:irq_wl_otg_usbok, wireless OTG mode, USB stick into interrput
	 * bit[3]:irq_vusb_ovp_alm, vusb warning report
	 * bit[2]:irq_vusb_uv, vusb uvlo interrupt
	 * bit[1]:irq_tdie_ot_alm, over temperature warning
	 * bit[0]:irq_vbat_lv, VBAT LV
	 */
	"irq_vbat_lv ",
	"irq_tdie_ot_alm ",
	"irq_vusb_uv ",
	"irq_vusb_ovp_alm ",
	"irq_wl_otg_usbok ",
	"irq_otmp_140 ",
	"irq_chg_batfet_ocp ",
	"irq_regn_ocp ",

	/*
	 * IRQ_FLAG_7 bit7 - bit0
	 * bit[7]:irq_reserved[7]
	 * bit[6]:irq_reserved[6]
	 * bit[5]:irq_vdrop_min, interrupt flag
	 * bit[4]:irq_vdrop_ovp, interrupt flag
	 * bit[3]:irq_reserved[3], irq_vout_dc_ovp vout over vol protection,
	 *        enabled in dc mode
	 * bit[2]:irq_reserved[2], reserved intterrput register
	 * bit[1]:irq_reserved[1], irq_vdrop_usb_ovp ovp tube,
	 *        drop protect interrupt, enabled in dc mode
	 * bit[0]:irq_reserved[0], irq_cfly_scp,fly capacitor
	 *        short circuit interrupt, enabled in dc mode
	 */
	"irq_cfly_scp ",
	"irq_vdrop_usb_ovp ",
	"none ",
	"irq_vout_dc_ovp ",
	"irq_vdrop_ovp ",
	"irq_vdrop_min ",
	"none ",
	"none ",
};

/* inti irq status and configuration */
static void hi6526_mask_all_irq(void)
{
	/* mask all irq output  */
	(void)hi6526_write_mask(CHG_IRQ_MASK_ALL_ADDR,
				CHG_IRQ_MASK_ALL_MSK,
				CHG_IRQ_MASK_ALL_SHIFT,
				CHG_IRQ_MASK_ALL_MSK);
}

static void hi6526_mask_pd_irq(void)
{
	/* mask pd irq output  */
	(void)hi6526_write_mask(CHG_IRQ_MASK_ALL_ADDR, CHG_IRQ_MASK_PD_MSK,
				CHG_IRQ_MASK_ALL_SHIFT, CHG_IRQ_MASK_PD_MSK);
	/* mask irq_cc_ov irq_cc_ovp and irq_cc_uv output */
	(void)hi6526_write_mask(CHG_IRQ_MASK_5,
				CHG_IRQ_MASK_5_CC_MSK,
				CHG_IRQ_MASK_5_CC_SHIFT,
				CHG_IRQ_MASK_5_CC_MSK);
}

/* inti irq status and configuration */
void hi6526_unmask_all_irq(void)
{
	/* buck irq unmask */
	(void)hi6526_write(CHG_IRQ_MASK_0, CHG_IRQ_MASK_0_VAL);
	(void)hi6526_write(CHG_IRQ_MASK_1, CHG_IRQ_MASK_1_VAL);
	(void)hi6526_write(CHG_IRQ_MASK_2, CHG_IRQ_MASK_2_VAL);
	(void)hi6526_write(CHG_IRQ_MASK_3, CHG_IRQ_MASK_3_VAL);
	(void)hi6526_write(CHG_IRQ_MASK_4, CHG_IRQ_MASK_4_VAL);

	/* except irq_cc_ov irq_cc_ovp and irq_cc_uv */
	(void)hi6526_write_mask(CHG_IRQ_MASK_5, CHG_IRQ_MASK_5_IRQ_MSK,
				CHG_IRQ_MASK_5_IRQ_SHIFT, CHG_IRQ_MASK_5_VAL);
	(void)hi6526_write(CHG_IRQ_MASK_6, CHG_IRQ_MASK_6_VAL);
	(void)hi6526_write(CHG_IRQ_MASK_7, CHG_IRQ_MASK_7_VAL);

	/* FCP MASK */
	(void)hi6526_write(CHG_FCP_IRQ5_MASK_ADDR, 0x03);
	(void)hi6526_write(0x9b, 0xff);
	(void)hi6526_write(0x9c, 0xff);

	/* mask all irq output  */
	(void)hi6526_write_mask(CHG_IRQ_MASK_ALL_ADDR,
				CHG_IRQ_MASK_ALL_MSK,
				CHG_IRQ_MASK_ALL_SHIFT,
				CHG_IRQ_UNMASK_DEFAULT);
}

static void buck_irq_str_fmt_cpy(int index, int str_len,
				 char *buf_addr, int *offset, const char *buf)
{
	int ret;

	if (strlen(g_buck_irq_str[index]) <
		(str_len - strlen(buf_addr))) {
		ret = snprintf_s(buf_addr + (*offset), str_len -
				 strlen(buf), strlen(g_buck_irq_str[index]),
				 "%s", g_buck_irq_str[index]);
		if (ret < 0) {
			scharger_err("%s, ret %d, index %d, error!\n",
				     __func__, ret, index);
				return;
		}
		*offset += ret;
	}
}

static void hi6526_buck_irq2str(u32 buck_irq_state)
{
	int index;
	int offset;
	const int str_len = BUF_MAX_512 - 1;
	char buf[BUF_MAX_512] = {0};
	char *buf_addr = buf;

	buck_irq_state &= BUCK_IRQ_MASK;
	offset =
		snprintf_s(buf_addr, str_len, STR_LEN_50,
			   "hi6526_irq_work:buck_irq_state:0x%x:",
			   buck_irq_state);
	if (offset < 0) {
		scharger_err("%s, offset %d, error\n", __func__, offset);
		return;
	}

	for (index = IRQ_BIT_NUM; index >= 0; index--)
		if (buck_irq_state & (1UL << (unsigned int)index))
			buck_irq_str_fmt_cpy(index, str_len, buf_addr,
					     &offset, buf);
	scharger_err("%s\n", buf);
}

static void lvc_irq_str_fmt_cpy(int index, int str_len,
				char *buf_addr, int *offset, const char *buf)
{
	int ret;

	if (strlen(g_lvc_sc_irq_str[index]) <
		(str_len - strlen(buf_addr))) {
		ret = snprintf_s(buf_addr + (*offset), str_len - strlen(buf),
				 strlen(g_lvc_sc_irq_str[index]),
				 "%s", g_lvc_sc_irq_str[index]);
		if (ret < 0) {
			scharger_err("%s, ret %d, index %d, error!\n",
				     __func__, ret, index);
			return;
		}
		*offset += ret;
	}
}

static void hi6526_lvc_sc_irq2str(u32 lvc_sc_irq_state)
{
	int index;
	int offset;
	const int str_len = BUF_MAX_512 - 1;
	char buf[BUF_MAX_512] = {0};
	char *buf_addr = buf;

	lvc_sc_irq_state &= LVC_SC_IRQ_MASK;
	offset = snprintf_s(buf_addr, str_len, STR_LEN_50,
			    "hi6526_irq_work:lvc_sc_irq_state:0x%x:",
			    lvc_sc_irq_state);
	if (offset < 0) {
		scharger_err("%s, offset %d, error\n", __func__, offset);
		return;
	}
	for (index = IRQ_BIT_NUM; index >= 0; index--)
		if (lvc_sc_irq_state & (1UL << (unsigned int)index))
			lvc_irq_str_fmt_cpy(index, str_len, buf_addr,
					    &offset, buf);

	scharger_err("%s\n", buf);
}

static void other_irq_str_fmt_cpy(int index, int str_len,
				  char *buf_addr, int *offset, const char *buf)
{
	int ret;

	if (strlen(g_others_irq_str[index]) <
		(str_len - strlen(buf_addr))) {
		ret = snprintf_s(buf_addr + (*offset), str_len - strlen(buf),
				 strlen(g_others_irq_str[index]),
				 "%s", g_others_irq_str[index]);
		if (ret < 0) {
			scharger_err("%s, ret %d, index %d, error!\n",
				     __func__, ret, index);
			return;
		}
		*offset += ret;
	}
}

static void hi6526_others_irq2str(u32 others_irq_state)
{
	int index;
	int offset;
	const int str_len = BUF_MAX_512 - 1;
	char buf[BUF_MAX_512] = {0};
	char *buf_addr = buf;

	others_irq_state &= OTHERS_IRQ_MASK;
	offset = snprintf_s(buf_addr, str_len, STR_LEN_50,
			    "hi6526_irq_work:others_irq_state:0x%x:",
			    others_irq_state);
	if (offset < 0) {
		scharger_err("%s, offset %d, error\n", __func__, offset);
		return;
	}
	for (index = IRQ_BIT_NUM; index >= 0; index--)
		if (others_irq_state & (1UL << (unsigned int)index))
			other_irq_str_fmt_cpy(index, str_len, buf_addr,
					      &offset, buf);

	scharger_err("%s\n", buf);
}

static void hi6526_direct_dmd_report(u32 fault_type, struct nty_data *data)
{
#ifdef CONFIG_DIRECT_CHARGER
	struct hi6526_device_info *di = get_hi6526_device();
	struct atomic_notifier_head *direct_charge_fault_notifier_list = NULL;

	if ((di == NULL) || !(di->chg_mode == SC || di->chg_mode == LVC))
		return;

	if (di->chg_mode == SC)
		sc_get_fault_notifier(&direct_charge_fault_notifier_list);
	else
		lvc_get_fault_notifier(&direct_charge_fault_notifier_list);

	atomic_notifier_call_chain(direct_charge_fault_notifier_list,
				   fault_type, data);
#endif
}

void hi6526_direct_charge_fault_handle(u32 lvc_sc_irq_state)
{
	int ret;
	struct hi6526_device_info *di = get_hi6526_device();
	u32 fault_type = DC_FAULT_NON;
	struct nty_data *data = NULL;

	hi6526_lvc_sc_irq2str(lvc_sc_irq_state);

	if ((di == NULL) || !(di->chg_mode == SC || di->chg_mode == LVC))
		return;

	data = &(di->dc_nty_data);
	di->abnormal_happened = 1;

	if (lvc_sc_irq_state & (LVC_REGULATOR_IRQ_MASK | IRQ_IBUS_DC_UCP_MASK))
		return;

	data->addr = di->client->addr;
	data->event1 = (lvc_sc_irq_state & 0xff0000) >> 16; /* 16:2 bytes len */
	data->event2 = (lvc_sc_irq_state & 0x00ff00) >> 8; /* 8:1 byte len */
	data->event3 = (lvc_sc_irq_state & 0xff);

	if (lvc_sc_irq_state & FAULT_VBUS_OVP)
		fault_type = DC_FAULT_VBUS_OVP;

	if (lvc_sc_irq_state & FAULT_IBUS_OCP)
		fault_type = DC_FAULT_IBUS_OCP;

	if (lvc_sc_irq_state & FAULT_REVERSE_OCP)
		fault_type = DC_FAULT_REVERSE_OCP;

	if (lvc_sc_irq_state & FAULT_VDROP_OVP)
		fault_type = DC_FAULT_VDROP_OVP;

	if (lvc_sc_irq_state & FAULT_VBAT_OVP)
		fault_type = DC_FAULT_VBAT_OVP;

	if (lvc_sc_irq_state & FAULT_IBAT_OCP)
		fault_type = DC_FAULT_IBAT_OCP;

	hi6526_direct_dmd_report(fault_type, data);
	uvdm_charge_fault_event_notify(fault_type, data);
	ret = hi6526_lvc_enable(0);
	if (ret != 0)
		scharger_err("hi6526_lvc_enable err\n");
	ret = hi6526_sc_enable(0);
	if (ret != 0)
		scharger_err("hi6526_sc_enable err\n");
}

static void hi6526_chip_overtemp_handle(void)
{
	struct hi6526_device_info *di = get_hi6526_device();
	struct nty_data *data = NULL;

	if ((di == NULL) || !(di->chg_mode == SC || di->chg_mode == LVC))
		return;

	data = &(di->dc_nty_data);
	hi6526_direct_dmd_report(DC_FAULT_TDIE_OTP, data);
}

static void hi6526_buck_vbat_ovp_handle(void)
{
	int i, ret;
	u8 vbat_ovp_cnt = 0;
	u8 irq_st0 = 0;

	scharger_err("%s\n", __func__);
	for (i = 0; i < 5; i++) { /* status number is 5 */
		ret = hi6526_read(CHG_IRQ_STATUS0, &irq_st0);
		if (ret || ((irq_st0 & CHG_VBAT_OVP) == CHG_VBAT_OVP)) {
			vbat_ovp_cnt++;
			mdelay(2); /* 2: need 2mS for next read */
		} else {
			vbat_ovp_cnt = 0;
			break;
		}
	}
	if (vbat_ovp_cnt >= 5) { /* 5: total cnt of ovp */
		scharger_err("%s:CHARGE_FAULT_VBAT_OVP\n", __func__);
		hi6526_set_input_current_limit(1);
		atomic_notifier_call_chain(&fault_notifier_list,
					   CHARGE_FAULT_VBAT_OVP, NULL);
	}
}

static void hi6526_buck_fault_handle(u32 buck_irq_state)
{
	struct hi6526_device_info *di = get_hi6526_device();

	hi6526_buck_irq2str(buck_irq_state);

	if ((di == NULL) || di->chg_mode == SC || di->chg_mode == LVC)
		return;

	if (buck_irq_state & FAULT_BUCK_VBAT_OVP)
		hi6526_buck_vbat_ovp_handle();

	if (buck_irq_state & FAULT_OTG_OCP) {
		scharger_err("%s:CHARGE_FAULT_BOOST_OCP\n", __func__);
		atomic_notifier_call_chain(&fault_notifier_list,
					   CHARGE_FAULT_BOOST_OCP, NULL);
	}
	if (buck_irq_state & FAULT_REVERSBST) {
		scharger_err("%s:irq_reversbst, cnt:%d\n",
			     __func__, di->reverbst_cnt);
		di->reverbst_cnt++;
		if (di->reverbst_cnt < REVERBST_RETRY) {
			hi6526_set_anti_reverbst_reset();
		} else {
			set_boot_weaksource_flag();
			scharger_err("%s:CHARGE_FAULT_WEAKSOURCE\n", __func__);
			atomic_notifier_call_chain(&fault_notifier_list,
						   CHARGE_FAULT_WEAKSOURCE,
						   NULL);
		}
	}
	if (buck_irq_state & FAULT_CHG_DONE) {
		atomic_notifier_call_chain(&fault_notifier_list,
			CHARGE_FAULT_CHARGE_DONE, NULL);
		scharger_err("%s : CHARGE_FAULT_CHARGE_DONE\n", __func__);
	}
	if (buck_irq_state & (FAULT_CHG_DONE | FAULT_CHG_FAULT))
		hi6526_set_input_current_limit(1);

	if (buck_irq_state & FAULT_RECHG)
		hi6526_set_input_current_limit(0);
}

static void hi6526_force_clear_all_irq(void)
{
	int i, ret;
	u8 value = 0;
	const int irq_reg_num = 8;

	for (i = 0; i < irq_reg_num; i++) {
		ret = hi6526_read(CHG_BUCK_IRQ_ADDR + i, &value);
		if (ret)
			scharger_inf("%s i2c read error i %d\n", __func__, i);

		if (value) {
			scharger_inf("%s:irq flag%d:0x%x\n", __func__, i, value);
			ret = hi6526_write(CHG_BUCK_IRQ_ADDR + i, value);
			if (ret)
				scharger_inf("%s i2c write error i %d\n",
					     __func__, i);
		}
	}
}

/* handler for chargerIC fault irq in charging process */
void hi6526_irq_work(struct work_struct *work)
{
	u8 irq_state = 0;
	u32 buck_irq_state = 0;
	u32 lvc_sc_irq_state = 0;
	u16 fcp_irq_state1 = 0;
	u32 fcp_irq_state2 = 0;
	u8 fcp_irq_state3 = 0;
	u32 others_irq_state = 0;
	int retry_time = 0;
	int ret;
	struct hi6526_device_info *di =
		container_of(work, struct hi6526_device_info, irq_work);
	int gpio_val;

	if (di == NULL || work == NULL) {
		scharger_inf("%s:input ptr work or hi6526_device_info di is NULL\n");
		return;
	}

again:
	ret = hi6526_read(CHG_IRQ_ADDR, &irq_state);
	scharger_inf("%s %d:irq_state:0x%x\n", __func__, retry_time, irq_state);

	if (irq_state & CHG_BUCK_IRQ) {
		/* 3: totally 3 bytes to be read */
		(void)hi6526_read_block(CHG_BUCK_IRQ_ADDR,
					(u8 *)&buck_irq_state, 3);
		scharger_inf(
			"%s:CHG_BUCK_IRQ,irq_state:0x%x,buck_irq_state:0x%6.6x\n",
			__func__, irq_state, buck_irq_state);
		buck_irq_state &= BUCK_IRQ_MASK;
		hi6526_buck_fault_handle(buck_irq_state);
		(void)hi6526_write_block(CHG_BUCK_IRQ_ADDR,
					 (u8 *)&buck_irq_state, 3);
	}
	if (irq_state & CHG_LVC_SC_IRQ) {
		(void)hi6526_read_block(CHG_LVC_SC_IRQ_ADDR,
					(u8 *)&lvc_sc_irq_state, 3);
		scharger_err(
			"%s:CHG_LVC_SC_IRQ,irq_state:0x%x,lvc_sc_irq_state:0x%6.6x\n",
			__func__, irq_state, lvc_sc_irq_state);
		lvc_sc_irq_state &= LVC_SC_IRQ_MASK;
		hi6526_direct_charge_fault_handle(lvc_sc_irq_state);
		(void)hi6526_write_block(CHG_LVC_SC_IRQ_ADDR,
					 (u8 *)&lvc_sc_irq_state, 3);
	}
	if (irq_state & CHG_OTHERS_IRQ) {
		(void)hi6526_read_block(CHG_OTHERS_IRQ_ADDR,
					(u8 *)&others_irq_state, 3);
		scharger_inf(
			"%s:CHG_OTHERS_IRQ,irq_state:0x%x,others_irq_state:0x%6.6x\n",
			__func__, irq_state, others_irq_state);
		hi6526_others_irq2str(others_irq_state);

		if (others_irq_state & OTHERS_OTP_IRQ_MASK)
			hi6526_chip_overtemp_handle();

		if (others_irq_state & OTHERS_VBAT_LV_IRQ_MASK)
			hi6526_vbat_lv_handle();

		others_irq_state &= OTHERS_IRQ_MASK;
		(void)hi6526_write_block(CHG_OTHERS_IRQ_ADDR,
					 (u8 *)&others_irq_state, 3);
	}
	if (irq_state & CHG_FCP_IRQ) {
		/* 2: totally 2 bytes to be read */
		ret = ret || hi6526_read_block(CHG_FCP_IRQ_ADDR1,
					       (u8 *)&fcp_irq_state1, 2);
		ret = ret || hi6526_read_block(CHG_FCP_IRQ_ADDR2,
					       (u8 *)&fcp_irq_state2, 2);
		ret = ret || hi6526_read(CHG_FCP_IRQ_ADDR3, &fcp_irq_state3);

		scharger_inf("%s:CHG_FCP_IRQ, irq_state:0x%x, fcp_irq_state1:0x%x, fcp_irq_state2:0x%x, fcp_irq_state3:0x%x\n",
			     __func__, irq_state, fcp_irq_state1,
			     fcp_irq_state2, fcp_irq_state3);

		fcp_irq_state1 &= OTHERS_IRQ_MASK1;
		fcp_irq_state2 &= OTHERS_IRQ_MASK2;
		fcp_irq_state3 &= OTHERS_IRQ_MASK3;
		ret = ret || hi6526_write_block(CHG_FCP_IRQ_ADDR1,
						(u8 *)&fcp_irq_state1, 2);
		ret = ret || hi6526_write_block(CHG_FCP_IRQ_ADDR2,
						(u8 *)&fcp_irq_state2, 2);
		ret = ret || hi6526_write(CHG_FCP_IRQ_ADDR3, fcp_irq_state3);
	}
	if (ret)
		scharger_inf("%s:i2c return error\n", __func__);
	gpio_val = gpio_get_value(di->gpio_int);
	if (gpio_val == 0) {
		if ((irq_state & CHG_IRQ_MASK_ALL_MSK) == 0)
			msleep(WAIT_PD_IRQ_TIME);
		if (retry_time > IRQ_RETRY_TIME) {
			scharger_err("%s:gpio is low,retry time %d MAX\n",
				     __func__, retry_time);
			/* clear all irq */
			hi6526_force_clear_all_irq();
		} else {
			retry_time++;
			goto again;
		}
	}
}

/* callback function for chargerIC fault irq in charging process */
static irqreturn_t hi6526_interrupt(int irq, void *_di)
{
	struct hi6526_device_info *di = _di;

	(void)queue_work(system_power_efficient_wq, &di->irq_work);
	return IRQ_HANDLED;
}

static void hi6526_irq_clear(void)
{
	int i, ret;
	u8 irq_state = 0;
	u8 val = 0;

	ret = hi6526_read(CHG_IRQ_ADDR, &irq_state);
	for (i = 0; i < 8; i++) { /* 8 continuous regs to be read */
		ret = ret || hi6526_read(CHG_BUCK_IRQ_ADDR + i, &val);
		if (val) {
			scharger_err("[%s]:irq_state=0x%x, irq[%d]=0x%x\n",
				     __func__, irq_state, i, val);
			ret = ret || hi6526_write(CHG_BUCK_IRQ_ADDR + i, val);
		}
	}
	if (ret)
		scharger_err("%s:i2c error\n", __func__);
}

int hi6526_irq_init(struct hi6526_device_info *di, struct device_node *np)
{
	int ret;

	if (di == NULL || np == NULL)
		return -ENOMEM;

	hi6526_mask_all_irq();
	hi6526_mask_pd_irq();
	hi6526_irq_clear();

	di->gpio_int = of_get_named_gpio(np, "gpio_int", 0);
	if (!gpio_is_valid(di->gpio_int)) {
		scharger_err("%s, gpio_int is not valid\n", __func__);
		ret = -EINVAL;
		goto hi6526_gpio_validate_req_fail;
	}

	ret = gpio_request(di->gpio_int, "charger_int");
	if (ret) {
		scharger_err("%s, could not request gpio_int\n", __func__);
		goto hi6526_gpio_validate_req_fail;
	}
	ret = gpio_direction_input(di->gpio_int);
	if (ret < 0) {
		scharger_err("%s, Could not set gpio direction\n", __func__);
		goto hi6526_gpio_direct_map_fail;
	}
	di->irq_int = gpio_to_irq(di->gpio_int);
	if (di->irq_int < 0) {
		scharger_err("%s, could not map gpio_int to irq\n", __func__);
		goto hi6526_gpio_direct_map_fail;
	}
	ret = request_irq(di->irq_int, hi6526_interrupt,
			  IRQF_SHARED | IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND,
			  "charger_int_irq", di);
	if (ret) {
		scharger_err("%s, could not request irq_int\n", __func__);
		di->irq_int = -1;
		goto hi6526_request_irq_fail;
	}

	return ret;
hi6526_request_irq_fail:
	free_irq(di->irq_int, di);
hi6526_gpio_direct_map_fail:
	gpio_free(di->gpio_int);
hi6526_gpio_validate_req_fail:
	return ret;
}
