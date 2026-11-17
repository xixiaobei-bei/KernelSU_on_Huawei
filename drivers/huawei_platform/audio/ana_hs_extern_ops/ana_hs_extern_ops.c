/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: ana hs extern ops function
 * Author: lijinkui
 * Create: 2019-10-22
 */

#include "huawei_platform/audio/ana_hs_extern_ops.h"
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/workqueue.h>
#include <linux/pm_wakeup.h>
#include <securec.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/usb/hw_pd_dev.h>
#include <huawei_platform/power/huawei_charger.h>
#include <ana_hs_kit/ana_hs_core/ana_hs_core.h>

#define HWLOG_TAG ana_hs_extern_ops
HWLOG_REGIST();

/* water detect function, only for fsa4480 chip */
#define IN_FUNCTION hwlog_info("%s comein\n", __func__)
#define OUT_FUNCTION hwlog_info("%s comeout\n", __func__)

struct ana_hs_extern_ops_data {
	/* 4480 water detect data */
	struct wakeup_source wake_lock;
	struct delayed_work notify_water_dw;
	struct delayed_work res_detect_dw;
	struct delayed_work stop_ovp_dw;
	struct delayed_work get_irq_type_dw;
	struct delayed_work detect_onboot_dw;
	int default_pin;
	bool water_intruded;
	bool typec_detach;
	bool report_when_detach;
	bool notified;
	unsigned int res_value_old[PIN_NUM];
};

static struct ana_hs_extern_ops_data *extern_pdata;

static int ana_hs_fsa4480_is_water_intruded(void *dev_data)
{
	if (!extern_pdata) {
		hwlog_err("%s extern_pdata is null\n", __func__);
		return false;
	}

	return extern_pdata->water_intruded;
}

static struct water_detect_ops ana_hs_extern_water_detect_ops = {
	.type_name = "audio_dp_dn",
	.is_water_intruded = ana_hs_fsa4480_is_water_intruded,
};

static void ana_hs_fsa4480_clear_res_detect(void)
{
	unsigned int reg_value = 0x00;

	ana_hs_bus_read(FSA4480_REG_ENABLE, &reg_value);
	ana_hs_bus_write(FSA4480_REG_ENABLE, reg_value & RES_DISABLE);
}

static void ana_hs_fsa4480_clear_res_interrupt(void)
{
	unsigned int reg_value = 0x00;

	ana_hs_bus_read(FSA4480_REG_RES_INT, &reg_value);
	ana_hs_bus_read(FSA4480_REG_RES_INT_MASK, &reg_value);
	ana_hs_bus_write(FSA4480_REG_RES_INT_MASK,
		reg_value | RES_INT_MASK_OPEN);
}

static void ana_hs_fsa4480_cancel_res_detect_work(void)
{
	cancel_delayed_work_sync(&extern_pdata->res_detect_dw);
	cancel_delayed_work_sync(&extern_pdata->get_irq_type_dw);
	hwlog_info("%s res detect work cancled\n", __func__);
}

static void ana_hs_fsa4480_cancel_notify_work(void)
{
	extern_pdata->water_intruded = false;
	cancel_delayed_work_sync(&extern_pdata->notify_water_dw);
	hwlog_info("%s water notify work cancled\n", __func__);
}

void ana_hs_fsa4480_stop_res_detect(bool new_state)
{
	if (!extern_pdata) {
		hwlog_err("%s extern_pdata is null\n", __func__);
		return;
	}

	IN_FUNCTION;

	extern_pdata->typec_detach = new_state;
	ana_hs_fsa4480_clear_res_detect();
	ana_hs_fsa4480_clear_res_interrupt();
	ana_hs_fsa4480_cancel_res_detect_work();
	if (extern_pdata->water_intruded && !extern_pdata->notified)
		ana_hs_fsa4480_cancel_notify_work();
	else
		extern_pdata->notified = false;

	OUT_FUNCTION;
}

static void ana_hs_fsa4480_set_interrupt_pin(void)
{
	unsigned int reg_value = 0x00;

	IN_FUNCTION;

	ana_hs_bus_write(FSA4480_REG_PIN_SET, extern_pdata->default_pin);
	ana_hs_bus_write(FSA4480_REG_DET_INTERVAL, LOOP_1S_DETECT);
	ana_hs_bus_read(FSA4480_REG_RES_INT, &reg_value);
	ana_hs_bus_read(FSA4480_REG_RES_INT_MASK, &reg_value);
	ana_hs_bus_write(FSA4480_REG_RES_INT_MASK,
		reg_value & RES_INT_MASK_CLEAR);
	ana_hs_bus_read(FSA4480_REG_ENABLE, &reg_value);
	ana_hs_bus_write(FSA4480_REG_ENABLE,
		reg_value | RES_ENABLE | RES_PREC_10K);

	OUT_FUNCTION;
}

void ana_hs_fsa4480_start_res_detect(bool new_state)
{
	if (!extern_pdata) {
		hwlog_err("%s extern_pdata is null\n", __func__);
		return;
	}

	IN_FUNCTION;

	extern_pdata->typec_detach = new_state;
	extern_pdata->report_when_detach = true;
	/* get 4 pins res handly, ignore first interrupt signal */
	if (extern_pdata->water_intruded)
		queue_delayed_work(system_long_wq, &extern_pdata->res_detect_dw,
			msecs_to_jiffies(DETECT_DELAY_WATER));
	else
		queue_delayed_work(system_long_wq, &extern_pdata->res_detect_dw,
			msecs_to_jiffies(DETECT_DELAY_DRY));

	OUT_FUNCTION;
}

#ifdef CONFIG_HUAWEI_DSM
static void ovp_dmd_report(void)
{
	int ret;
	char msg_buf[FSA4480_DSM_BUF_SIZE_256] = { 0 };

	ret = snprintf_s(msg_buf, FSA4480_DSM_BUF_SIZE_256,
		FSA4480_DSM_BUF_SIZE_256 - 1,
		"%s\n",
		"D+/D-/SBU ovp happened");
	if (ret < 0)
		hwlog_err("%s Fill fsa4480 ovp dmd msg fail\n", __func__);

	power_dsm_report_dmd(POWER_DSM_BATTERY, ERROR_NO_TYPEC_4480_OVP,
		(void *)msg_buf);
}
#endif

void ana_hs_fsa4480_start_ovp_detect(bool new_state)
{
	unsigned int opv_status = 0;
	unsigned int reg_value;

	if (!extern_pdata) {
		hwlog_err("%s extern_pdata is null\n", __func__);
		return;
	}

	IN_FUNCTION;
	extern_pdata->typec_detach = new_state;
	ana_hs_bus_read(FSA4480_REG_OVP_STATUS, &opv_status);
	ana_hs_bus_read(FSA4480_REG_OVP_INT, &reg_value);
	if (opv_status & OVP_STATUS_VERIFY) {
#ifdef CONFIG_HUAWEI_DSM
		hwlog_info("ovp happened, report ovp dmd\n");
		ovp_dmd_report();
#endif
	} else {
		hwlog_info("no ovp, enable ovp int\n");
		ana_hs_bus_write(FSA4480_REG_OVP_INT_MASK, OVP_INT_MASK_CLEAR);
		ana_hs_bus_read(FSA4480_REG_OVP_INT, &reg_value);
	}
	OUT_FUNCTION;
}

void ana_hs_fsa4480_stop_ovp_detect(bool new_state)
{
	if (!extern_pdata) {
		hwlog_err("%s extern_pdata is null\n", __func__);
		return;
	}

	hwlog_info("%s wait %d ms to stop it", __func__, STOP_OVP_DELAY);
	extern_pdata->typec_detach = new_state;
	queue_delayed_work(system_long_wq, &extern_pdata->stop_ovp_dw,
		msecs_to_jiffies(STOP_OVP_DELAY));
}

static void ana_hs_fsa4480_stop_ovp_detect_work(struct work_struct *work)
{
	unsigned int reg_value;

	hwlog_info("%s stopped\n", __func__);
	ana_hs_bus_read(FSA4480_REG_OVP_INT, &reg_value);
	ana_hs_bus_write(FSA4480_REG_OVP_INT_MASK, OVP_INT_MASK_OPEN);
}

static void ana_hs_fsa4480_detect_onboot_work(struct work_struct *work)
{
	int event = PD_DPM_USB_TYPEC_NONE;

	IN_FUNCTION;

	pd_dpm_get_typec_state(&event);
	if (event == PD_DPM_USB_TYPEC_DETACHED) {
		hwlog_info("typec device detach, start res detect onboot\n");
		extern_pdata->typec_detach = ANA_HS_TYPEC_DEVICE_DETACHED;
		queue_delayed_work(system_long_wq, &extern_pdata->res_detect_dw,
			msecs_to_jiffies(0));
	} else {
		hwlog_info("typec device attach, start ovp detect onboot\n");
		extern_pdata->typec_detach = ANA_HS_TYPEC_DEVICE_ATTACHED;
		ana_hs_fsa4480_start_ovp_detect(extern_pdata->typec_detach);
	}

	OUT_FUNCTION;
}

static void ana_ha_fsa4480_update_pin_state(int i,
	unsigned int res_value, bool *pins_intruded_state)
{
	const static char *detected_pin[PIN_NUM] = {
		"DP", "DN", "SBU1", "SBU2" };

	if (res_value <= RES_INT_THRESHOLD) {
		pins_intruded_state[i] = true;
		hwlog_info("%s %s intruded flag is :%x\n", __func__,
			detected_pin[i], pins_intruded_state[i]);
	} else {
		pins_intruded_state[i] = false; /* pin is not watered */
	}
}

static void notify_water_work(struct work_struct *work)
{
	int event = WD_STBY_MOIST; /* watered */
	(void)work;

	hwlog_info("%s watered, report event\n", __func__);
	/* notify watered event */
	extern_pdata->notified = true;
	power_event_bnc_notify(POWER_BNT_WD, POWER_NE_WD_REPORT_UEVENT, &event);
	power_event_bnc_notify(POWER_BNT_WD, POWER_NE_WD_REPORT_DMD, "fsa4480");
}

static void ana_hs_fsa4480_update_watered_state(const bool pins_intruded_state[])
{
	int j;
	int event;
	int pin_index = 0;
	/* res_detect_pins is D+ D- SBU1 SBU2 */
	const static unsigned int res_detect_pins[PIN_NUM] = {
		0x01, 0x02, 0x03, 0x04 };

	if (!extern_pdata->water_intruded) {
		if (pins_intruded_state[DP] &&
			pins_intruded_state[DN] &&
			(pins_intruded_state[SBU1] ||
			pins_intruded_state[SBU2])) {
			extern_pdata->water_intruded = true;
			queue_delayed_work(system_long_wq, &extern_pdata->notify_water_dw,
				msecs_to_jiffies(NOTIFY_WATER_DELAY));
		} else {
			for (j = 0; j < PIN_NUM; j++) {
				if (!pins_intruded_state[j]) {
					extern_pdata->default_pin =
						res_detect_pins[j];
					hwlog_info("%s no watered, next detect 0x%x\n",
						__func__,
						extern_pdata->default_pin);
					break;
				}
			}
		}
	} else {
		if ((pins_intruded_state[DP] + pins_intruded_state[DN] +
			pins_intruded_state[SBU1] + pins_intruded_state[SBU2]) <= WATER_PIN_NUM) {
			while (pins_intruded_state[pin_index])
				pin_index++;
			extern_pdata->default_pin = res_detect_pins[pin_index];
			extern_pdata->water_intruded = false;
			hwlog_info("%s water dry, next detect 0x%x\n",
				__func__, extern_pdata->default_pin);
			/* notify dry event */
			event = WD_STBY_DRY; /* water dry */
			power_event_bnc_notify(POWER_BNT_WD, POWER_NE_WD_REPORT_UEVENT,
				&event);
		} else {
			hwlog_info("%s still watered\n", __func__);
			if (extern_pdata->report_when_detach) {
				hwlog_info("report when detach under watered\n");
				event = WD_STBY_MOIST; /* watered */
				power_event_bnc_notify(POWER_BNT_WD,
					POWER_NE_WD_REPORT_UEVENT, &event);
			}
		}
	}
}

static void ana_hs_fsa4480_select_detect_mode(void)
{
	if (extern_pdata->water_intruded) {
		hwlog_info("%s set %dms to recall\n", __func__,
			CHECK_TYPEC_DELAY);
		queue_delayed_work(
			system_long_wq,
			&extern_pdata->res_detect_dw,
			msecs_to_jiffies(CHECK_TYPEC_DELAY));
	} else {
		hwlog_info("%s restart res detect\n", __func__);
		ana_hs_fsa4480_set_interrupt_pin();
	}
}

static void ana_hs_fsa4480_res_detect_work(struct work_struct *work)
{
	int i;
	int ret;
	unsigned int res_value;
	unsigned int reg_value = 0x00;

	const static unsigned int res_detect_pins[PIN_NUM] = {
		0x01, 0x02, 0x03, 0x04 };
	const static char *detected_pin[PIN_NUM] = {
		"DP", "DN", "SBU1", "SBU2" };
	static bool pins_intruded_state[PIN_NUM] = {0};

	IN_FUNCTION;

	__pm_wakeup_event(&extern_pdata->wake_lock, WAKE_LOCK_TIMEOUT);
	ana_hs_bus_write(FSA4480_REG_DET_INTERVAL, SIGNAL_DETECT);

	for (i = 0; i < PIN_NUM; i++) {
		if (!extern_pdata->typec_detach) {
			hwlog_info("%s typec attach, exit detect\n", __func__);
			__pm_relax(&extern_pdata->wake_lock);
			return;
		} else {
			res_value = extern_pdata->res_value_old[i];
			ana_hs_bus_write(FSA4480_REG_PIN_SET,
				res_detect_pins[i]);
			ana_hs_bus_read(FSA4480_REG_ENABLE, &reg_value);
			ana_hs_bus_write(FSA4480_REG_ENABLE,
				reg_value | RES_ENABLE | RES_PREC_10K);
			mdelay(RES_DETECT_DELAY);
			ret = ana_hs_bus_read(FSA4480_REG_RES_VAL, &res_value);
			if (ret == 0)
				extern_pdata->res_value_old[i] = res_value;
			ana_hs_bus_read(FSA4480_REG_RES_INT, &reg_value);
			hwlog_info("%s %s res value %d * 10K\n",
				__func__, detected_pin[i], res_value);
			ana_ha_fsa4480_update_pin_state(i,
				res_value, pins_intruded_state);
		}
	}
	ana_hs_fsa4480_update_watered_state(pins_intruded_state);
	extern_pdata->report_when_detach = false;
	ana_hs_fsa4480_select_detect_mode();
	__pm_relax(&extern_pdata->wake_lock);
	OUT_FUNCTION;
}

static void get_ovp_irq_type(unsigned int value)
{
	int irq_type = ANA_FSA4480_OVP_IRQ_MAX;
	unsigned int mask = 0x00;

	__pm_wakeup_event(&extern_pdata->wake_lock, WAKE_LOCK_TIMEOUT);
	ana_hs_bus_read(FSA4480_REG_OVP_INT_MASK, &mask);
	ana_hs_bus_write(FSA4480_REG_OVP_INT_MASK, OVP_INT_MASK_OPEN);

	if ((value & OVP_STATUS_VERIFY) && !(mask & OVP_IRQ_MASK_BIT))
		irq_type = ANA_FSA4480_OVP_IRQ_EXIST;
	hwlog_info("%s 0x02 is 0x%x, 0x01 is 0x%x, irq type is %d\n",
		__func__, value, mask, irq_type);

	switch (irq_type) {
	case ANA_FSA4480_OVP_IRQ_EXIST:
		hwlog_info("%s ANA_FSA4480_OVP_IRQ_EXIST\n", __func__);
#ifdef CONFIG_HUAWEI_DSM
		ovp_dmd_report();
#endif
		break;
	default:
		hwlog_info("%s ANA_FSA4480_OVP_IRQ_MAX\n", __func__);
		break;
	}
	__pm_relax(&extern_pdata->wake_lock);
}

static void get_res_irq_type(unsigned int value)
{
	int irq_type = ANA_FSA4480_IQR_MAX;
	unsigned int mask = 0x00;

	__pm_wakeup_event(&extern_pdata->wake_lock, WAKE_LOCK_TIMEOUT);
	ana_hs_bus_read(FSA4480_REG_RES_INT_MASK, &mask);
	ana_hs_fsa4480_clear_res_interrupt();

	if ((value & ANA_FSA4480_AUDIO_JACK_DETECT_AND_CONFIG_IRQ_BIT) &&
		!(mask & ANA_FSA4480_AUDIO_JACK_DETECT_AND_CONFIG_IRQ_MASK_BIT))
		irq_type = ANA_FSA4480_AUDIO_JACK_DETECT_AND_CONFIG;
	else if ((value & ANA_FSA4480_RES_BELOW_THRESHOLD_IRQ_BIT) &&
		!(mask & ANA_FSA4480_RES_BELOW_THRESHOLD_IRQ_MASK_BIT))
		irq_type = ANA_FSA4480_RES_BELOW_THRESHOLD;
	else if ((value & ANA_FSA4480_RES_DETECT_ACTION_IRQ_BIT)  &&
		!(mask & ANA_FSA4480_RES_DETECT_ACTION_IRQ_MASK_BIT))
		irq_type = ANA_FSA4480_RES_DETECT_ACTION;

	hwlog_info("%s 0x18 is 0x%x, 0x19 is 0x%x, irq type is %d\n",
		__func__, value, mask, irq_type);

	switch (irq_type) {
	case ANA_FSA4480_AUDIO_JACK_DETECT_AND_CONFIG:
		hwlog_info("%s AUDIO_JACK_DETECT_AND_CONFIG\n", __func__);
		__pm_relax(&extern_pdata->wake_lock);
		break;
	case ANA_FSA4480_RES_BELOW_THRESHOLD:
		hwlog_info("%s RES_BELOW_THRESHOLD\n", __func__);
		queue_delayed_work(system_long_wq,
			&extern_pdata->res_detect_dw, 0);
		break;
	case ANA_FSA4480_RES_DETECT_ACTION:
		hwlog_info("%s RES_DETECT_ACTION\n", __func__);
		__pm_relax(&extern_pdata->wake_lock);
		break;
	default:
		hwlog_info("%s IQR_MAX\n", __func__);
		ana_hs_fsa4480_set_interrupt_pin();
		__pm_relax(&extern_pdata->wake_lock);
		break;
	}
}

static void get_irq_type_work(struct work_struct *work)
{
	unsigned int ovp_int = 0;
	unsigned int ovp_mask = OVP_INT_MASK_OPEN;
	unsigned int res_int = 0;
	unsigned int res_mask = RES_INT_MASK_OPEN;

	IN_FUNCTION;

	/* res irq or ovp irq detect */
	ana_hs_bus_read(FSA4480_REG_OVP_INT, &ovp_int);
	ana_hs_bus_read(FSA4480_REG_RES_INT, &res_int);
	ana_hs_bus_read(FSA4480_REG_OVP_INT_MASK, &ovp_mask);
	ana_hs_bus_read(FSA4480_REG_RES_INT_MASK, &res_mask);

	if ((ovp_int & OVP_STATUS_VERIFY) && (ovp_mask == OVP_INT_MASK_CLEAR))
		get_ovp_irq_type(ovp_int);
	if (res_int && (res_mask == RES_INT_MASK_CLEAR))
		get_res_irq_type(res_int);

	OUT_FUNCTION;
}

irqreturn_t ana_hs_fsa4480_irq_handler(int irq, void *data)
{
	IN_FUNCTION;

	__pm_wakeup_event(&extern_pdata->wake_lock, WAKE_LOCK_TIMEOUT);

	queue_delayed_work(system_long_wq,
		&extern_pdata->get_irq_type_dw, WAIT_REG_TIMEOUT);

	OUT_FUNCTION;
	return IRQ_HANDLED;
}

static void ana_hs_fsa4480_reg_init(void)
{
	unsigned int reg_value = 0x00;

	ana_hs_bus_write(FSA4480_REG_OVP_INT_MASK, OVP_INT_MASK_OPEN);
	ana_hs_bus_read(FSA4480_REG_OVP_INT, &reg_value);
	ana_hs_bus_read(FSA4480_REG_RES_INT, &reg_value);
	ana_hs_bus_read(FSA4480_REG_RES_INT_MASK, &reg_value);
	ana_hs_bus_write(FSA4480_REG_RES_INT_MASK,
		reg_value | RES_INT_MASK_OPEN);
	ana_hs_bus_read(FSA4480_REG_ENABLE, &reg_value);
	ana_hs_bus_write(FSA4480_REG_ENABLE, reg_value & RES_DISABLE);
	ana_hs_bus_write(FSA4480_REG_RES_DET_THRD, RES_INT_THRESHOLD);
}

void ana_hs_fsa4480_init(void)
{
	int i;
	extern_pdata = kzalloc(sizeof(*extern_pdata), GFP_KERNEL);
	if (!extern_pdata)
		return;

	IN_FUNCTION;

	extern_pdata->default_pin = 0x01; /* 0x01 means D+ */
	extern_pdata->water_intruded = false;  /* default is false */
	extern_pdata->report_when_detach = false;
	extern_pdata->notified = false;
	extern_pdata->typec_detach = ANA_HS_TYPEC_DEVICE_DETACHED;
	for (i = 0; i < PIN_NUM; i++)
		extern_pdata->res_value_old[i] = 0xff; /* high res value */

	wakeup_source_init(&extern_pdata->wake_lock, "ana_hs_extern_ops");
	INIT_DELAYED_WORK(&extern_pdata->notify_water_dw, notify_water_work);
	INIT_DELAYED_WORK(&extern_pdata->res_detect_dw,
		ana_hs_fsa4480_res_detect_work);
	INIT_DELAYED_WORK(&extern_pdata->stop_ovp_dw,
		ana_hs_fsa4480_stop_ovp_detect_work);
	INIT_DELAYED_WORK(&extern_pdata->get_irq_type_dw,
		get_irq_type_work);
	INIT_DELAYED_WORK(&extern_pdata->detect_onboot_dw,
		ana_hs_fsa4480_detect_onboot_work);
	ana_hs_fsa4480_reg_init();
	water_detect_ops_register(&ana_hs_extern_water_detect_ops);

	queue_delayed_work(system_long_wq, &extern_pdata->detect_onboot_dw,
		msecs_to_jiffies(DETECT_ONBOOT_DELAY));

	OUT_FUNCTION;
}

void ana_hs_fsa4480_remove(void)
{
	ana_hs_fsa4480_clear_res_detect();
	ana_hs_fsa4480_clear_res_interrupt();
	ana_hs_fsa4480_cancel_res_detect_work();
	kfree(extern_pdata);
	extern_pdata = NULL;

	hwlog_info("%s: exit\n", __func__);
}

 /* ana_hs_extern_ops function for public use */
static struct irq_id_func_pair id_func_map[] = {
	{ ID_FSA4480_IRQ, ana_hs_fsa4480_irq_handler },
};

irq_handler_t ana_hs_irq_select_func(int irq_id)
{
	int i;

	for (i = 0; i < ID_IRQ_END; i++) {
		if (id_func_map[i].irq_id == irq_id)
			return id_func_map[i].func;
	}
	return (irq_handler_t)NULL;
}

static struct init_id_func_pair init_func_map[] = {
	{ ID_FSA4480_INIT, ana_hs_fsa4480_init },
};

ana_hs_init_func ana_hs_init_select_func(int init_id)
{
	int i;

	for (i = 0; i < ID_IRQ_END; i++) {
		if (init_func_map[i].init_id == init_id)
			return init_func_map[i].func;
	}
	return (ana_hs_init_func)NULL;
}

static struct remove_id_func_pair remove_func_map[] = {
	{ ID_FSA4480_REMOVE, ana_hs_fsa4480_remove },
};

ana_hs_remove_func ana_hs_remove_select_func(int remove_id)
{
	int i;

	for (i = 0; i < ID_REMOVE_END; i++) {
		if (remove_func_map[i].remove_id == remove_id)
			return remove_func_map[i].func;
	}
	return (ana_hs_remove_func)NULL;
}

static int ana_hs_extern_ops_probe(struct platform_device *pdev)
{
	return 0;
}

static int ana_hs_extern_ops_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id ana_hs_match_table[] = {
	{
		.compatible = "huawei,ana_hs_extern_ops",
		.data = NULL,
	},
	{
	},
};

static struct platform_driver ana_hs_extern_ops_driver = {
	.probe = ana_hs_extern_ops_probe,
	.remove = ana_hs_extern_ops_remove,

	.driver = {
		.name = "huawei,ana_hs_extern_ops",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(ana_hs_match_table),
	},
};

static int __init ana_hs_extern_ops_init(void)
{
	return platform_driver_register(&ana_hs_extern_ops_driver);
}

static void __exit ana_hs_extern_ops_exit(void)
{
	platform_driver_unregister(&ana_hs_extern_ops_driver);
}

subsys_initcall(ana_hs_extern_ops_init);
module_exit(ana_hs_extern_ops_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("ana hs extern ops driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
