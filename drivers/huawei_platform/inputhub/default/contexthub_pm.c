/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * Description: contexthub pm source file
 * Author: DIVS_SENSORHUB
 * Create: 2012-05-29
 */

#include "contexthub_pm.h"

#include <linux/fb.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/pm_wakeup.h>
#include <linux/regulator/consumer.h>
#include <linux/rtc.h>
#include <linux/suspend.h>
#include <linux/types.h>

#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif
#include <securec.h>

#include "contexthub_boot.h"
#include "contexthub_debug.h"
#include "contexthub_recovery.h"
#include "contexthub_route.h"
#include "protocol.h"
#include "sensor_config.h"
#include "sensor_detect.h"
#include "sensor_feima.h"

uint32_t need_reset_io_power;
uint32_t need_set_3v_io_power;
uint32_t need_set_3_1v_io_power;
uint32_t need_set_3_2v_io_power;
static unsigned long sensor_jiffies;
uint32_t vdd_set_io_power;
uint32_t use_ldo34_flag;
uint32_t use_ldo12_flag;
static uint32_t no_need_sensor_ldo24;
int g_sensorhub_wdt_irq = -1;
sys_status_t iom3_sr_status = ST_WAKEUP;
static int key_state;
int iom3_power_state = ST_POWERON;
struct completion iom3_resume_mini;
struct completion iom3_resume_all;
static struct ipc_debug ipc_debug_info;

static struct sensor_status sensor_status_backup;
static DEFINE_MUTEX(mutex_pstatus);
struct regulator *sensorhub_ldo34_vddio;
struct regulator *sensorhub_ldo12_vddio;

static char *sys_status_t_str[] = {
	[ST_SCREENON] = "ST_SCREENON",
	[ST_SCREENOFF] = "ST_SCREENOFF",
	[ST_SLEEP] = "ST_SLEEP",
	[ST_WAKEUP] = "ST_WAKEUP",
};

sys_status_t get_iom3_sr_status(void)
{
	return iom3_sr_status;
}

int get_iomcu_power_state(void)
{
	return iom3_power_state;
}
EXPORT_SYMBOL(get_iomcu_power_state);

struct ipc_debug *get_ipc_debug_info(void)
{
	return &ipc_debug_info;
}

unsigned long get_sensor_jiffies(void)
{
	return sensor_jiffies;
}

uint32_t get_no_need_sensor_ldo24(void)
{
	return no_need_sensor_ldo24;
}

int get_key_state(void)
{
	return key_state;
}

static inline void clean_ipc_debug_info(void)
{
	memset(&ipc_debug_info, 0, sizeof(ipc_debug_info));
}

static void print_ipc_debug_info(void)
{
	int i;

	for (i = TAG_BEGIN; i < TAG_END; ++i) {
		if (ipc_debug_info.event_cnt[i])
			hwlog_info("event_cnt[%d]: %d\n", i,
			ipc_debug_info.event_cnt[i]);
	}
	if (ipc_debug_info.pack_error_cnt)
		hwlog_err("pack_err_cnt: %d\n", ipc_debug_info.pack_error_cnt);
}

int tell_ap_status_to_mcu(int ap_st)
{
	struct read_info pkg_mcu;
	struct write_info winfo;

	if ((ap_st >= ST_BEGIN) && (ap_st < ST_END)) {
		pkt_sys_statuschange_req_t pkt;

		winfo.tag = TAG_SYS;
		winfo.cmd = CMD_SYS_STATUSCHANGE_REQ;
		winfo.wr_len = sizeof(pkt) - sizeof(pkt.hd);
		pkt.status = ap_st;
		winfo.wr_buf = &pkt.status;

		if (likely((ap_st >= ST_SCREENON) && (ap_st <= ST_WAKEUP))) {
			hwlog_info("------------>tell mcu ap in status %s\n",
				sys_status_t_str[ap_st]);
			iom3_sr_status = ap_st;
		} else {
			hwlog_info("------------>tell mcu ap in status %d\n",
				ap_st);
		}
		return write_customize_cmd(&winfo,
			(ap_st == ST_SLEEP) ? (&pkg_mcu) : NULL, true);
	} else {
		hwlog_err("error status %d in %s\n", ap_st, __func__);
		return -EINVAL;
	}
}

void update_current_app_status(uint8_t tag, uint8_t cmd)
{
	iomcu_power_status *status = get_global_iomcu_power_status();

	mutex_lock(&mutex_pstatus);
	if ((cmd == CMD_CMN_OPEN_REQ) || (cmd == CMD_CMN_INTERVAL_REQ))
		status->app_status[tag] = 1;
	else if (cmd == CMD_CMN_CLOSE_REQ)
		status->app_status[tag] = 0;
	mutex_unlock(&mutex_pstatus);
}

static void check_current_app(void)
{
	int i;
	int flag = 0;
	iomcu_power_status *status = get_global_iomcu_power_status();

	mutex_lock(&mutex_pstatus);
	for (i = 0; i < TAG_END; i++) {
		if (status->app_status[i])
			flag++;
	}
	if (flag > 0) {
		hwlog_info("total %d app running after ap suspend\n", flag);
		status->power_status = SUB_POWER_ON;
		flag = 0;
	} else {
		hwlog_info("iomcu will power off after ap suspend\n");
		status->power_status = SUB_POWER_OFF;
	}
	mutex_unlock(&mutex_pstatus);
}

static int sensorhub_pm_suspend(struct device *dev)
{
	int ret = 0;

	hwlog_info("%s+\n", __func__);
	if (iom3_sr_status != ST_SLEEP) {
		ret = tell_ap_status_to_mcu(ST_SLEEP);
		iom3_power_state = ST_SLEEP;
		check_current_app();
		clean_ipc_debug_info();
	}
	hwlog_info("%s-\n", __func__);
	return ret;
}

static int sensorhub_pm_resume(struct device *dev)
{
	int ret;
	int *flag = get_resume_skip_flag();

	hwlog_info("%s+\n", __func__);
	print_ipc_debug_info();
	*flag = RESUME_INIT;

	reinit_completion(&iom3_reboot);
	reinit_completion(&iom3_resume_mini);
	reinit_completion(&iom3_resume_all);
	barrier();
	write_timestamp_base_to_sharemem();
	tell_ap_status_to_mcu(ST_WAKEUP);
	/* wait mini ready */
	if (!wait_for_completion_timeout(&iom3_resume_mini,
		msecs_to_jiffies(1000))) {
		hwlog_err("RESUME :wait for MINI timeout\n");
		goto resume_err;
	} else if (*flag != RESUME_SKIP) {
		ret = send_fileid_to_mcu();
		if (ret) {
			hwlog_err("RESUME get sensors cfg data from dts fail,ret=%d, use default config data!\n",
				ret);
			goto resume_err;
		} else {
			hwlog_info("RESUME get sensors cfg data from dts success!\n");
		}
	} else {
		hwlog_err("RESUME skip MINI\n");
	}

	/* wait all ready */
	if (!wait_for_completion_timeout(&iom3_resume_all,
		msecs_to_jiffies(1000))) {
		hwlog_err("RESUME :wait for ALL timeout\n");
		goto resume_err;
	} else if (*flag != RESUME_SKIP) {
		hwlog_info("RESUME mcu all ready!\n");
		ret = sensor_set_cfg_data();
		if (ret < 0) {
			hwlog_err("RESUME sensor_chip_detect ret=%d\n", ret);
			goto resume_err;
		}
		reset_calibrate_data();
	} else {
		hwlog_err("RESUME skip ALL\n");
	}

	if (!wait_for_completion_timeout(&iom3_reboot,
		msecs_to_jiffies(1000))) {
		hwlog_err("resume :wait for response timeout\n");
		goto resume_err;
	}

	if (g_config_on_ddr->wrong_wakeup_msg.flag) {
		g_config_on_ddr->wrong_wakeup_msg.flag = 0;
		hwlog_err("************ sensorhub has wrong wakeup mesg\n");
		hwlog_err("time %llu\n", g_config_on_ddr->wrong_wakeup_msg.time);
		hwlog_err("irqs [%x] [%x]\n",
			g_config_on_ddr->wrong_wakeup_msg.irq0,
			g_config_on_ddr->wrong_wakeup_msg.irq1);
		hwlog_err("recvfromapmsg [%x] [%x] [%x] [%x]\n",
			  g_config_on_ddr->wrong_wakeup_msg.recvfromapmsg[0],
			  g_config_on_ddr->wrong_wakeup_msg.recvfromapmsg[1],
			  g_config_on_ddr->wrong_wakeup_msg.recvfromapmsg[2],
			  g_config_on_ddr->wrong_wakeup_msg.recvfromapmsg[3]);
		hwlog_err("recvfromlpmsg [%x] [%x] [%x] [%x]\n",
			  g_config_on_ddr->wrong_wakeup_msg.recvfromlpmsg[0],
			  g_config_on_ddr->wrong_wakeup_msg.recvfromlpmsg[1],
			  g_config_on_ddr->wrong_wakeup_msg.recvfromlpmsg[2],
			  g_config_on_ddr->wrong_wakeup_msg.recvfromlpmsg[3]);
		hwlog_err("sendtoapmsg [%x] [%x] [%x] [%x]\n",
			  g_config_on_ddr->wrong_wakeup_msg.sendtoapmsg[0],
			  g_config_on_ddr->wrong_wakeup_msg.sendtoapmsg[1],
			  g_config_on_ddr->wrong_wakeup_msg.sendtoapmsg[2],
			  g_config_on_ddr->wrong_wakeup_msg.sendtoapmsg[3]);
		hwlog_err("sendtolpmsg [%x] [%x] [%x] [%x]\n",
			  g_config_on_ddr->wrong_wakeup_msg.sendtolpmsg[0],
			  g_config_on_ddr->wrong_wakeup_msg.sendtolpmsg[1],
			  g_config_on_ddr->wrong_wakeup_msg.sendtolpmsg[2],
			  g_config_on_ddr->wrong_wakeup_msg.sendtolpmsg[3]);
		hwlog_err("ap lpm3 tap tlpm3 %x %x %x %x\n",
			  g_config_on_ddr->wrong_wakeup_msg.recvfromapmsgmode,
			  g_config_on_ddr->wrong_wakeup_msg.recvfromlpmsgmode,
			  g_config_on_ddr->wrong_wakeup_msg.sendtoapmsgmode,
			  g_config_on_ddr->wrong_wakeup_msg.sendtolpmsgmode);
		hwlog_err("************ sensorhub has wrong wakeup mesg end\n");
		if (memset_s(&(g_config_on_ddr->wrong_wakeup_msg), sizeof(wrong_wakeup_msg_t),
			0, sizeof(wrong_wakeup_msg_t)) != EOK)
			hwlog_err("%s, memset_s fail\n", __func__);
	}
	goto done;
resume_err:
	iom3_need_recovery(SENSORHUB_MODID, SH_FAULT_RESUME);
done:
	iom3_power_state = ST_WAKEUP;
	hwlog_info("%s-\n", __func__);
	return 0;
}

const static struct of_device_id sensorhub_io_supply_ids[] = {
	{ .compatible = "huawei,sensorhub_io" },
	{}
};
MODULE_DEVICE_TABLE(of, sensorhub_io_supply_ids);

static void sensorhub_xsensor_poweron(int gpio)
{
	int ret;

	ret = gpio_request(gpio, "sensorhub_1v8");
	if (ret == 0) {
		ret = gpio_direction_output(gpio, 1);
		if (ret != 0)
			hwlog_err("%s gpio_direction_output faild, gpio is %d.\n", __func__, gpio);
		else
			hwlog_info("%s gpio_direction_output succ, gpio is %d.\n", __func__, gpio);
	} else {
		hwlog_err("%s gpio_request faild, gpio is %d.\n", __func__, gpio);
	}
}

static int sensorhub_io_driver_probe(struct platform_device *pdev)
{
	int ret;
	int tmp_gpio;
	uint32_t val = 0;
	struct device_node *power_node = NULL;

	if (!of_match_device(sensorhub_io_supply_ids, &pdev->dev)) {
		hwlog_err("[%s,%d]: sensorhub_io_driver_probe match fail !\n",
			__func__, __LINE__);
		return -ENODEV;
	}
	g_sensorhub_wdt_irq = platform_get_irq(pdev, 0);
	if (g_sensorhub_wdt_irq < 0) {
		pr_err("[%s] platform_get_irq err\n", __func__);
		return -ENXIO;
	}
	power_node = of_find_node_by_name(NULL, "sensorhub_io_power");
	if (!power_node) {
		hwlog_err("%s failed to find dts node sensorhub_io_power\n",
			__func__);
	} else {
		if (of_property_read_u32(power_node, "need-reset", &val)) {
			hwlog_err("%s failed to find property need-reset.\n",
				__func__);
		} else {
			need_reset_io_power = val;
			hwlog_info("%s property need-reset is %d.\n",
				__func__, val);
		}
		val = 0;
		if (of_property_read_u32(power_node, "set-3v", &val)) {
			hwlog_err("%s failed to find property set-3v.\n",
				__func__);
		} else {
			need_set_3v_io_power = val;
			hwlog_info("%s property set-3v is %d.\n",
				__func__, val);
		}
		val = 0;
		if (of_property_read_u32(power_node, "set-3_1v", &val)) {
			hwlog_err("%s failed to find property set-3_1v.\n",
				__func__);
		} else {
			need_set_3_1v_io_power = val;
			hwlog_info("%s property set-3_1v is %d.\n",
				__func__, val);
		}
		if (of_property_read_u32(power_node, "set-3_2v", &val)) {
			hwlog_err("%s failed to find property set-3_2v.\n",
				__func__);
		} else {
			need_set_3_2v_io_power = val;
			hwlog_info("%s property set-3_2v is %d.\n",
				__func__, val);
		}
		if (of_property_read_u32(power_node, "vdd-set", &val)) {
			hwlog_info("%s failed to find property vdd-set.\n",
				__func__);
		} else {
			vdd_set_io_power = val;
			hwlog_info("%s property vdd-set is %d.\n",
				__func__, val);
		}
		tmp_gpio = of_get_named_gpio(power_node, "sensorhub_gpio_1v8", 0);
		if (tmp_gpio < 0)
			hwlog_info("%s sensorhub_gpio_1v8 not found.\n", __func__);
		else
			sensorhub_xsensor_poweron(tmp_gpio);
		tmp_gpio = of_get_named_gpio(power_node, "sensorhub_gpio_ag_1v8", 0);
		if (tmp_gpio < 0)
			hwlog_info("%s sensorhub_gpio_ag_1v8 not found.\n", __func__);
		else
			sensorhub_xsensor_poweron(tmp_gpio);

		val = 0;
		if (of_property_read_u32(power_node, "use_ldo34", &val)) {
			hwlog_info("%s failed to find property use_ldo34.\n",
				__func__);
		} else {
			use_ldo34_flag = val;
			hwlog_info("%s property use_ldo34 is %d.\n",
				__func__, val);
		}

		val = 0;
		if (of_property_read_u32(power_node, "use_ldo12", &val)) {
			hwlog_info("%s failed to find property use_ldo12.\n",
				__func__);
		} else {
			use_ldo12_flag = val;
			hwlog_info("%s property use_ldo12 is %d.\n",
				__func__, val);
		}

		val = 0;
		if (of_property_read_u32(power_node,
			"no_need_sensor_ldo24", &val)) {
			hwlog_info("%s failed find no_need_sensor_ldo24.\n",
				__func__);
		} else {
			no_need_sensor_ldo24 = val;
			hwlog_info("%s property no_need_ldo24 is %d.\n",
				__func__, val);
		}
	}
	if (!no_need_sensor_ldo24) {
		sensorhub_vddio = regulator_get(&pdev->dev, SENSOR_VBUS);
		if (IS_ERR(sensorhub_vddio)) {
			hwlog_err("%s: regulator_get fail!\n", __func__);
			return -EINVAL;
		}

		if (need_set_3v_io_power) {
			ret = regulator_set_voltage(sensorhub_vddio,
				SENSOR_VOLTAGE_3V, SENSOR_VOLTAGE_3V);
			if (ret < 0)
				hwlog_err("failed to set sensorhub_vddio voltage to 3V\n");
		} else if (need_set_3_1v_io_power) {
			ret = regulator_set_voltage(sensorhub_vddio,
				SENSOR_VOLTAGE_3_1V, SENSOR_VOLTAGE_3_1V);
			if (ret < 0)
				hwlog_err("failed to set sensorhub_vddio voltage to 3_1V\n");
		} else if (need_set_3_2v_io_power) {
			ret = regulator_set_voltage(sensorhub_vddio,
				SENSOR_VOLTAGE_3_2V, SENSOR_VOLTAGE_3_2V);
			if (ret < 0)
				hwlog_err("failed to set sensorhub_vddio voltage to 3_2V\n");
		} else {
			if (vdd_set_io_power) {
				ret = regulator_set_voltage(sensorhub_vddio,
					vdd_set_io_power, vdd_set_io_power);
				if (ret < 0)
					hwlog_err("failed to set sensorhub_vddio voltage\n");
			}
		}
		ret = regulator_enable(sensorhub_vddio);
		if (ret < 0)
			hwlog_err("failed to enable regulator sensorhub_vddio\n");
	}

	if (use_ldo34_flag == 1) {
		sensorhub_ldo34_vddio =
			regulator_get(&pdev->dev, SENSOR_VBUS_LDO34);
		if (IS_ERR(sensorhub_ldo34_vddio)) {
			hwlog_err("%s: ldo34 regulator_get fail!\n", __func__);
		} else {
			ret = regulator_set_voltage(sensorhub_ldo34_vddio,
				SENSOR_VOLTAGE_3_3V, SENSOR_VOLTAGE_3_3V);
			if (ret < 0)
				hwlog_err("failed to set ldo34 sensorhub_vddio voltage to 3.3V\n");

			ret = regulator_enable(sensorhub_ldo34_vddio);
			if (ret < 0)
				hwlog_err("failed to enable ldo34 regulator sensorhub_vddio\n");
		}
	}

	if (use_ldo12_flag == 1) {
		sensorhub_ldo12_vddio =
			regulator_get(&pdev->dev, SENSOR_VBUS_LDO12);
		if (IS_ERR(sensorhub_ldo12_vddio)) {
			hwlog_err("%s: ldo12 regulator_get fail!\n", __func__);
		} else {
			ret = regulator_set_voltage(sensorhub_ldo12_vddio,
				SENSOR_VOLTAGE_1V8, SENSOR_VOLTAGE_1V8);
			if (ret < 0)
				hwlog_err("failed to set ldo12 sensorhub_vddio voltage to 1.8V\n");

			ret = regulator_enable(sensorhub_ldo12_vddio);
			if (ret < 0)
				hwlog_err("failed to enable ldo12 regulator sensorhub_vddio\n");
		}
	}

	if (!no_need_sensor_ldo24) {
		if (need_reset_io_power &&
			(get_sensorhub_reboot_reason_flag() ==
			SENSOR_POWER_DO_RESET)) {
			hwlog_info("%s : disable vddio\n", __func__);
			ret = regulator_disable(sensorhub_vddio);
			if (ret < 0)
				hwlog_err("failed to disable regulator sensorhub_vddio\n");

			sensor_jiffies = jiffies;
		}
	}
	hwlog_info("%s: sensorhub_io_driver_probe success!\n",
		__func__);
	return 0;
}

static bool should_be_processed_when_sr(int sensor_tag)
{
	bool ret = true; /* can be closed default */

	switch (sensor_tag) {
	case TAG_PS:
	case TAG_STEP_COUNTER:
	case TAG_SIGNIFICANT_MOTION:
	case TAG_PHONECALL:
	case TAG_CONNECTIVITY:
	case TAG_FP:
	case TAG_FP_UD:
	case TAG_MAGN_BRACKET:
	case TAG_DROP:
	case TAG_EXT_HALL:
		ret = false;
		break;

	default:
		break;
	}

	return ret;
}

void disable_sensors_when_suspend(void)
{
	int tag;

	memset(&sensor_status_backup, 0, sizeof(sensor_status_backup));
	memcpy(&sensor_status_backup,
		&sensor_status, sizeof(sensor_status_backup));
	for (tag = TAG_SENSOR_BEGIN; tag < TAG_SENSOR_END; ++tag) {
		if ((sensor_status_backup.status[tag] ||
			sensor_status_backup.opened[tag]) &&
			!(get_hifi_supported() == 1 &&
			(sensor_status.batch_cnt[tag] > 1))) {
			if (should_be_processed_when_sr(tag))
				inputhub_sensor_enable(tag, false);
		}
	}
}

void enable_sensors_when_resume(void)
{
	int tag = 0;
	interval_param_t delay_param = {
		.period = sensor_status_backup.delay[tag],
		.batch_count = 1,
		.mode = AUTO_MODE,
		.reserved[0] = TYPE_STANDARD /* for step counter only */
	};
	for (tag = TAG_SENSOR_BEGIN;
		tag < TAG_SENSOR_END; ++tag) {
		if ((sensor_status_backup.status[tag] ||
			sensor_status_backup.opened[tag]) &&
			!(get_hifi_supported() == 1 &&
			(sensor_status.batch_cnt[tag] > 1))) {
			if (should_be_processed_when_sr(tag)) {
				if (sensor_status_backup.opened[tag] &&
					(sensor_status.opened[tag] == 0))
					inputhub_sensor_enable(tag, true);
				if (sensor_status_backup.status[tag]) {
					delay_param.period =
						sensor_status.status[tag] ?
						sensor_status.delay[tag] :
						sensor_status_backup.delay[tag];
					delay_param.batch_count =
						sensor_status.status[tag] ?
						sensor_status.batch_cnt[tag] :
						sensor_status_backup.batch_cnt[tag];
					inputhub_sensor_setdelay(tag, &delay_param);
				} else if ((sensor_status_backup.status[tag] == 0) &&
					(tag == TAG_ALS || tag == TAG_ALS1 || tag == TAG_ALS2) &&
					sensor_status_backup.opened[tag]) {
					hwlog_info("ALS set delay when backup_status =0 && backup_opened = 1\n");
					delay_param.period = 0;
					delay_param.batch_count = 1;
					inputhub_sensor_setdelay(tag, &delay_param);
				}
			}
		}
	}
}

static void key_fb_notifier_action(int enable)
{
	int ret;
	interval_param_t param;

	hwlog_info("key_fb_notifier_action fb :%d\n", enable);
	if (strlen(get_sensor_chip_info_address(KEY)) == 0) {
		hwlog_info("no key.\n");
		return;
	}

	if (enable) {
		ret = inputhub_sensor_enable(TAG_KEY, true);
		if (ret) {
			hwlog_err("write open err.\n");
			return;
		}
		memset(&param, 0, sizeof(param));
		param.period = 20;
		ret = inputhub_sensor_setdelay(TAG_KEY, &param);
		if (ret)
			hwlog_err("write interval err.\n");
		else
			key_state = 1;
	} else {
		ret = inputhub_sensor_enable(TAG_KEY, false);
		if (ret)
			hwlog_err("write close err.\n");
		else
			key_state = 0;
	}
}

static int sensorhub_fb_notifier(struct notifier_block *nb,
	unsigned long action, void *data)
{
	if (!data)
		return NOTIFY_OK;
	switch (action) {
	case FB_EVENT_BLANK: /* change finished */
	{
		struct fb_event *event = data;
		int *blank = event->data;

		if (registered_fb[0] !=
			event->info) { /* only main screen on/off info send to hub */
			hwlog_err("%s, not main screen info, just return\n",
				__func__);
			return NOTIFY_OK;
		}
		switch (*blank) {
		case FB_BLANK_UNBLANK: /* screen on */
			tell_ap_status_to_mcu(ST_SCREENON);
			key_fb_notifier_action(1);
			break;

		case FB_BLANK_POWERDOWN: /* screen off */
			tell_ap_status_to_mcu(ST_SCREENOFF);
			sensor_redetect_enter();
			key_fb_notifier_action(0);
			break;

		default:
			hwlog_err("unknown---> lcd unknown in %s\n", __func__);
			break;
		}
		break;
	}
	default:
		break;
	}

	return NOTIFY_OK;
}

static int sensorhub_pm_notify(struct notifier_block *nb,
	unsigned long mode, void *_unused)
{
	switch (mode) {
	case PM_SUSPEND_PREPARE: /* suspend */
		hwlog_info("suspend in %s\n", __func__);
		disable_sensors_when_suspend();
		break;

	case PM_POST_SUSPEND: /* resume */
		hwlog_info("resume in %s\n", __func__);
		enable_sensors_when_resume();
		break;

	case PM_HIBERNATION_PREPARE: /* Going to hibernate */
	case PM_POST_HIBERNATION: /* Hibernation finished */
	case PM_RESTORE_PREPARE: /* Going to restore a saved image */
	case PM_POST_RESTORE: /* Restore failed */
	default:
		break;
	}

	return 0;
}

static struct notifier_block fb_notify = {
	.notifier_call = sensorhub_fb_notifier,
};

void set_pm_notifier(void)
{
	init_completion(&iom3_resume_mini);
	init_completion(&iom3_resume_all);
	pm_notifier(sensorhub_pm_notify, 0);
	fb_register_client(&fb_notify);
}

const static struct dev_pm_ops sensorhub_io_pm_ops = {
	.suspend = sensorhub_pm_suspend,
	.resume = sensorhub_pm_resume,
};

static struct platform_driver sensorhub_io_driver = {
	.probe = sensorhub_io_driver_probe,
	.driver = {
		   .name = "Sensorhub_io_driver",
		   .owner = THIS_MODULE,
		   .of_match_table = of_match_ptr(sensorhub_io_supply_ids),
		   .pm = &sensorhub_io_pm_ops,
	},
};

int sensorhub_io_driver_init(void)
{
	int ret;

	hwlog_info("[%s] ++", __func__);

	ret = platform_driver_register(&sensorhub_io_driver);
	if (ret) {
		hwlog_err("%s: platform_device_register(sensorhub_io_driver) failed, ret:%d.\n",
			__func__, ret);
		return ret;
	}
	hwlog_info("[%s] --", __func__);
	return 0;
}
