/*
 * Huawei Touchscreen Driver
 *
 * Copyright (c) 2012-2021 Huawei Technologies Co., Ltd.
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/spi/spi.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/notifier.h>
#include <linux/fb.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/of_gpio.h>
#include <linux/of.h>
#ifdef CONFIG_HUAWEI_THP_MTK
#include <linux/of_irq.h>
#endif
#include <linux/regulator/consumer.h>
#include <linux/hwspinlock.h>
#include <linux/kthread.h>
#include "hwspinlock_internal.h"
#if ((defined CONFIG_HUAWEI_THP_QCOM) && (!defined CONFIG_LCD_KIT_DRIVER))
#include "dsi_panel.h"
#endif

#ifndef CONFIG_LCD_KIT_DRIVER
#include "../../lcdkit/lcdkit1.0/core/common/lcdkit_tp.h"
#endif
#if (!defined CONFIG_HUAWEI_THP_QCOM) && (!defined CONFIG_HUAWEI_THP_MTK)
#include "../../lcdkit/lcdkit1.0/include/lcdkit_ext.h"
#endif

#include "huawei_thp.h"
#include "huawei_thp_mt_wrapper.h"
#include "huawei_thp_attr.h"
#ifdef CONFIG_HUAWEI_SHB_THP
#include "huawei_thp_log.h"
#endif

#if defined(CONFIG_LCD_KIT_DRIVER)
#include <lcd_kit_core.h>
#endif

#ifdef CONFIG_INPUTHUB_20
#include "contexthub_recovery.h"
#endif

#ifdef CONFIG_HUAWEI_PS_SENSOR
#include "ps_sensor.h"
#endif

#ifdef CONFIG_HUAWEI_SENSORS_2_0
#include "sensor_scp.h"
#endif

#if (!defined CONFIG_HUAWEI_THP_QCOM) && (!defined CONFIG_HUAWEI_THP_MTK)
#include <huawei_platform/sensor/hw_comm_pmic.h>
#endif

#ifdef CONFIG_BCI_BATTERY
#include <linux/power/hisi/bci_battery.h>
#endif

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#if (defined(CONFIG_HUAWEI_THP_MTK) || defined(CONFIG_HUAWEI_THP_QCOM))
#include <hwmanufac/dev_detect/dev_detect.h>
#else
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif
#endif

#if defined(CONFIG_HUAWEI_DSM)
#include <dsm/dsm_pub.h>
#endif

#if defined(CONFIG_TEE_TUI)
#include "tui.h"
#endif
#if ((defined CONFIG_HUAWEI_THP_MTK) || (defined CONFIG_HUAWEI_THP_QCOM))
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#endif
#if defined(CONFIG_HUAWEI_TS_KIT_3_0)
#include "../3_0/trace-events-touch.h"
#else
#define trace_touch(x...)
#endif

#ifdef CONFIG_HUAWEI_THP_QCOM
int g_tskit_pt_station_flag;
#endif

struct thp_core_data *g_thp_core;
static u8 *spi_sync_tx_buf;
static u8 *spi_sync_rx_buf;
static u8 is_fw_update;
static bool locked_by_daemon;
static int spi_sync_count_in_suspend;
static int get_frame_count_in_suspend;

static int thp_unregister_ic_num;
static struct thp_cmd_node ping_cmd_buff;
static struct thp_cmd_node pang_cmd_buff;

static u8 *g_queue_data_buf;
static unsigned int g_thp_queue_node_len = THP_QUEUE_NODE_BUFF_MAX_LEN;

static bool g_thp_prox_enable;
static bool onetime_poweroff_done;
#if (!defined CONFIG_HUAWEI_THP_QCOM) && (!defined CONFIG_HUAWEI_THP_MTK)
static struct hw_comm_pmic_cfg_t tp_pmic_ldo_set;
#endif

#if defined(CONFIG_TEE_TUI)
struct thp_tui_data thp_tui_info;
EXPORT_SYMBOL(thp_tui_info);

struct multi_tp_tui_data tp_tui_data[MAX_SUPPORT_TP_TUI_NUM];
#endif
#if defined(CONFIG_HUAWEI_DSM)
#define THP_CHIP_DMD_REPORT_SIZE 1024
#define THP_DSM_BUFF_SIZE 4096

static struct dsm_dev dsm_thp = {
	.name = "dsm_tphostprocessing",
	.device_name = "TPHOSTPROCESSING",
	.ic_name = "syn",
	.module_name = "NNN",
	.fops = NULL,
	.buff_size = THP_DSM_BUFF_SIZE,
};

struct dsm_client *dsm_thp_dclient;

#define PRINTF_ERROR_CODE "printf error\n"
void thp_dmd_report(int dmd_num, const char *psz_format, ...)
{
	va_list args;
	char *input_buf = NULL;
	int ret;
	int buf_length;
	struct thp_core_data *cd = thp_get_core_data();

	if ((!cd) || (!dsm_thp_dclient)) {
		thp_log_err("cd or dsm_thp_dclient is NULL\n");
		return;
	}
	if (!psz_format) {
		thp_log_err("psz_format is NULL\n");
		return;
	}

	input_buf = kzalloc(THP_CHIP_DMD_REPORT_SIZE, GFP_KERNEL);
	if (!input_buf) {
		thp_log_err("%s: kzalloc failed!\n", __func__);
		return;
	}

	va_start(args, psz_format);
	ret = vsnprintf(input_buf, THP_CHIP_DMD_REPORT_SIZE, psz_format, args);
	if (ret < 0) {
		thp_log_err("vsnprintf failed, ret: %d\n", ret);
		strcpy(input_buf, PRINTF_ERROR_CODE);
	}
	va_end(args);
	buf_length = strlen(input_buf);
	ret = snprintf((input_buf + buf_length),
		(THP_CHIP_DMD_REPORT_SIZE - buf_length),
		"irq_gpio:%d\tvalue:%d\nreset_gpio:%d\tvalue:%d\nTHP_status:%u\nprojectid:%s\n",
		cd->gpios.irq_gpio,
		gpio_get_value(cd->gpios.irq_gpio),
		cd->gpios.rst_gpio,
		gpio_get_value(cd->gpios.rst_gpio),
		thp_get_status_all(),
		cd->project_id);
	if (ret < 0)
		thp_log_err("snprintf failed, ret: %d\n", ret);

	if (!dsm_client_ocuppy(dsm_thp_dclient)) {
		dsm_client_record(dsm_thp_dclient, input_buf);
		dsm_client_notify(dsm_thp_dclient, dmd_num);
		thp_log_info("%s %s\n", __func__, input_buf);
	}

	kfree(input_buf);
}
#endif

#define PINCTRL_INIT_ENABLE "pinctrl_init_enable"

#define THP_DEVICE_NAME	"huawei_thp"
#define THP_MISC_DEVICE_NAME "thp"

#ifdef CONFIG_HUAWEI_THP_MTK
struct ud_fp_ops *g_ud_fp_operations;
static void send_event_to_fingerprint_ud(struct thp_udfp_data udfp_data);
void fp_ops_register(struct ud_fp_ops *ops)
{
	if (g_ud_fp_operations == NULL) {
		g_ud_fp_operations = ops;
		thp_log_info("%s:g_ud_fp_ops register success\n", __func__);
	} else {
		thp_log_info("%s:g_ud_fp_ops has been registered\n", __func__);
	}
}

void fp_ops_unregister(struct ud_fp_ops *ops)
{
	if (g_ud_fp_operations != NULL) {
		g_ud_fp_operations = NULL;
		thp_log_info("%s:g_ud_fp_ops_unregister success\n", __func__);
	} else {
		thp_log_info("%s:g_ud_fp_ops has been unregister\n", __func__);
	}
}

struct ud_fp_ops *fp_get_ops(void)
{
	return g_ud_fp_operations;
}
#endif

int thp_send_esd_event(unsigned int status)
{
	struct thp_core_data *cd = thp_get_core_data();

	thp_log_info("%s: enter\n", __func__);

	if (cd == NULL) {
		thp_log_err("%s: cd is null\n", __func__);
		return -EINVAL;
	}
	if (cd->lcd_esd_event_upload == 0) {
		thp_log_info("%s: not support esd\n", __func__);
		return -EINVAL;
	}
	if (thp_mt_wrapper_esd_event(status)) {
		thp_log_err("%s: gpio detect fail\n", __func__);
		return -EINVAL;
	}

	return 0;
}

#if defined(CONFIG_LCD_KIT_DRIVER)
int thp_power_control_notify(enum lcd_kit_ts_pm_type pm_type, int timeout);
static int thp_get_status_by_type(int type, int *status);
int thp_multi_power_control_notify(enum lcd_kit_ts_pm_type pm_type, int timeout,
	int panel_index);
static bool thp_get_afe_download_status(struct timeval *record_tv);
struct ts_kit_ops thp_ops = {
	.ts_power_notify = thp_power_control_notify,
	.get_tp_proxmity = thp_get_prox_switch_status,
	.get_tp_status_by_type = thp_get_status_by_type,
	.ts_multi_power_notify = thp_multi_power_control_notify,
	.get_afe_status = thp_get_afe_download_status,
	.send_esd_event = thp_send_esd_event,
};

static bool thp_get_afe_download_status(struct timeval *record_tv)
{
	struct thp_core_data *cd = thp_get_core_data();

	if ((record_tv == NULL) || (cd == NULL)) {
		thp_log_err("%s: record_tv or cd is null\n", __func__);
		return false;
	}

	if (cd->lcd_need_get_afe_status == 0) {
		thp_log_err("%s: lcd no need get afe status\n", __func__);
		return false;
	}

	if (cd->afe_download_status) {
		record_tv->tv_sec = cd->afe_status_record_tv.tv_sec;
		record_tv->tv_usec = cd->afe_status_record_tv.tv_usec;
	}
	return cd->afe_download_status;
}

static int thp_get_status_by_type(int type, int *status)
{
	struct thp_core_data *cd = thp_get_core_data();

	if (status == NULL) {
		thp_log_err("status is null\n");
		return -EINVAL;
	}

	/*
	 * To avoid easy_wakeup_info.sleep_mode being changed
	 * during suspend, we assign cd->sleep_mode to
	 * easy_wakeup_info.sleep_mode in suspend.
	 * For TDDI, tp suspend must before lcd power off to
	 * make sure of easy_wakeup_info.sleep_mode be assigned.
	 */
	if ((type == TS_GESTURE_FUNCTION) &&
		(cd->easy_wakeup_info.sleep_mode == TS_GESTURE_MODE) &&
		cd->support_gesture_mode &&
		cd->lcd_gesture_mode_support) { /* TDDI need this */
		*status = GESTURE_MODE_OPEN;
		return NO_ERR;
	}

	return -EINVAL;
}
#endif

#ifdef CONFIG_HUAWEI_SHB_THP
int thp_send_sync_info_to_ap(const char *head)
{
	struct thp_sync_info *rx = NULL;

	if (head == NULL) {
		thp_log_err("thp_sync_info, data is NULL\n");
		return -EINVAL;
	}
	rx = (struct thp_sync_info *)head;
	/* here we do something sending data to afe hal */
	thp_log_info("thp_sync_info, len:%d\n", rx->size);

	return 0;
}

int thp_send_event_to_drv(const char *data)
{
	char sub_event;
	struct thp_core_data *cd = thp_get_core_data();

	if (!data) {
		thp_log_err("%s: data is NULL\n", __func__);
		return -EINVAL;
	}
	if (!atomic_read(&cd->register_flag)) {
		thp_log_err("%s: thp have not be registered\n", __func__);
		return -ENODEV;
	}
	sub_event = data[0];
	switch (sub_event) {
	case THP_SUB_EVENT_SINGLE_CLICK:
		__pm_wakeup_event(&cd->thp_wake_lock, jiffies_to_msecs(HZ));
		thp_inputkey_report(TS_SINGLE_CLICK);
		break;
	case THP_SUB_EVENT_DOUBLE_CLICK:
		__pm_wakeup_event(&cd->thp_wake_lock, jiffies_to_msecs(HZ));
		thp_inputkey_report(TS_DOUBLE_CLICK);
		break;
	case THP_SUB_EVENT_STYLUS_SINGLE_CLICK_AND_PRESS:
		__pm_wakeup_event(&cd->thp_wake_lock, jiffies_to_msecs(HZ));
		thp_input_pen_report(TS_STYLUS_WAKEUP_TO_MEMO);
		break;
	case THP_SUB_EVENT_STYLUS_SINGLE_CLICK:
		__pm_wakeup_event(&cd->thp_wake_lock, jiffies_to_msecs(HZ));
		thp_input_pen_report(TS_STYLUS_WAKEUP_SCREEN_ON);
		break;
	default:
		thp_log_err("%s: sub_event is invalid, event = %d\n",
			__func__, sub_event);
		return -EINVAL;
	}
	thp_log_info("%s sub_event = %d\n", __func__, sub_event);
	return 0;
}
#endif

#if defined(CONFIG_TEE_TUI)
static void thp_tui_secos_exit(void);
#endif
int thp_spi_sync(struct spi_device *spi, struct spi_message *message)
{
	int ret;

	trace_touch(TOUCH_TRACE_SPI, TOUCH_TRACE_FUNC_IN, "thp");
	ret = spi_sync(spi, message);
	trace_touch(TOUCH_TRACE_SPI, TOUCH_TRACE_FUNC_OUT, "thp");

	return ret;
}

static int thp_is_valid_project_id(const char *id)
{
	int i;

	if ((id == NULL) || (*id == '\0'))
		return false;
	for (i = 0; i < THP_PROJECT_ID_LEN; i++) {
		if (!isascii(*id) || !isalnum(*id))
			return false;
		id++;
	}
	return true;
}

unsigned int thp_is_factory(void)
{
	unsigned int is_factory;

#if (defined(CONFIG_HUAWEI_THP_MTK) || defined(CONFIG_HUAWEI_THP_QCOM))
	is_factory = get_runmode_type();
#else
	is_factory = runmode_is_factory();
#endif
	return (!!is_factory);
}

int thp_project_id_provider(char *project_id,
	unsigned int len)
{
	struct thp_core_data *cd = thp_get_core_data();

	if (len < THP_PROJECT_ID_LEN) {
		thp_log_err("%s len is too small\n", __func__);
		return -EINVAL;
	}
	if ((project_id != NULL) && (cd != NULL)) {
		if (thp_is_valid_project_id(cd->project_id)) {
			strncpy(project_id, cd->project_id,
				THP_PROJECT_ID_LEN);
		} else {
			thp_log_debug("%s: invalid project id", __func__);
			return -EINVAL;
		}
	}
	return 0;
}
EXPORT_SYMBOL(thp_project_id_provider);

const int get_thp_unregister_ic_num(void)
{
	return thp_unregister_ic_num;
}
EXPORT_SYMBOL(get_thp_unregister_ic_num);

struct thp_core_data *thp_get_core_data(void)
{
	return g_thp_core;
}
EXPORT_SYMBOL(thp_get_core_data);

static void thp_wake_up_frame_waitq(struct thp_core_data *cd)
{
	cd->frame_waitq_flag = WAITQ_WAKEUP;
	wake_up_interruptible(&(cd->frame_waitq));
}
static int thp_pinctrl_get_init(struct thp_device *tdev);
static int thp_pinctrl_select_normal(struct thp_device *tdev);
static int thp_pinctrl_select_lowpower(struct thp_device *tdev);

#define is_invaild_power_id(x) ((x) >= THP_POWER_ID_MAX)

static char *thp_power_name[THP_POWER_ID_MAX] = {
	"thp-iovdd",
	"thp-vcc",
};

static const char *thp_power_id2name(enum thp_power_id id)
{
	return !is_invaild_power_id(id) ? thp_power_name[id] : 0;
}

int thp_power_supply_get(enum thp_power_id power_id)
{
	struct thp_core_data *cd = thp_get_core_data();
	struct thp_power_supply *power = NULL;
	int ret;

	if (is_invaild_power_id(power_id)) {
		thp_log_err("%s: invalid power id %d", __func__, power_id);
		return -EINVAL;
	}

	power = &cd->thp_powers[power_id];
	if (power->type == THP_POWER_UNUSED)
		return 0;
	if (power->use_count) {
		power->use_count++;
		return 0;
	}
	switch (power->type) {
	case THP_POWER_LDO:
		power->regulator = regulator_get(&cd->sdev->dev,
			thp_power_id2name(power_id));
		if (IS_ERR_OR_NULL(power->regulator)) {
			thp_log_err("%s:fail to get %s\n", __func__,
				thp_power_id2name(power_id));
			return -ENODEV;
		}

		ret = regulator_set_voltage(power->regulator, power->ldo_value,
				power->ldo_value);
		if (ret) {
			regulator_put(power->regulator);
			thp_log_err("%s:fail to set %s valude %d\n", __func__,
					thp_power_id2name(power_id),
					power->ldo_value);
			return ret;
		}
		break;
	case THP_POWER_GPIO:
		ret = gpio_request(power->gpio, thp_power_id2name(power_id));
		if (ret) {
			thp_log_err("%s:request gpio %d for %s failed\n",
				__func__, power->gpio,
				thp_power_id2name(power_id));
			return ret;
		}
		break;

#if (!defined CONFIG_HUAWEI_THP_QCOM) && (!defined CONFIG_HUAWEI_THP_MTK)
	case THP_POWER_PMIC:
		thp_log_info("%s call %d,%d,%d\n", __func__,
			power->pmic_power.pmic_num, power->pmic_power.ldo_num,
			power->pmic_power.value);
		tp_pmic_ldo_set.pmic_num = power->pmic_power.pmic_num;
		tp_pmic_ldo_set.pmic_power_type = power->pmic_power.ldo_num;
		tp_pmic_ldo_set.pmic_power_voltage = power->pmic_power.value;
		break;
#endif

	default:
		thp_log_err("%s: invalid power type %d\n",
			__func__, power->type);
		break;
	}
	power->use_count++;
	return 0;
}

int thp_power_supply_put(enum thp_power_id power_id)
{
	struct thp_core_data *cd = thp_get_core_data();
	struct thp_power_supply *power = NULL;

	if (is_invaild_power_id(power_id)) {
		thp_log_err("%s: invalid power id %d", __func__, power_id);
		return -EINVAL;
	}

	power = &cd->thp_powers[power_id];
	if (power->type == THP_POWER_UNUSED)
		return 0;
	if ((--power->use_count) > 0)
		return 0;

	switch (power->type) {
	case THP_POWER_LDO:
		if (IS_ERR_OR_NULL(power->regulator)) {
			thp_log_err("%s:fail to get %s\n", __func__,
				thp_power_id2name(power_id));
			return -ENODEV;
		}
		regulator_put(power->regulator);
		break;
	case THP_POWER_GPIO:
		gpio_direction_output(power->gpio, 0);
		gpio_free(power->gpio);
		break;
	case THP_POWER_PMIC:
		thp_log_err("%s: power supply %d\n", __func__, power->type);
		break;

	default:
		thp_log_err("%s: invalid power type %d\n",
			__func__, power->type);
		break;
	}
	return 0;
}

int thp_power_supply_ctrl(enum thp_power_id power_id,
	int status, unsigned int delay_ms)
{
	struct thp_core_data *cd = thp_get_core_data();
	struct thp_power_supply *power = NULL;
	int rc = 0;

	if (is_invaild_power_id(power_id)) {
		thp_log_err("%s: invalid power id %d", __func__, power_id);
		return -EINVAL;
	}

	power = &cd->thp_powers[power_id];
	if (power->type == THP_POWER_UNUSED)
		goto exit;

	thp_log_info("%s:power %s %s\n", __func__,
		thp_power_id2name(power_id), status ? "on" : "off");

	if (!power->use_count) {
		thp_log_err("%s:regulator %s not gotten yet\n", __func__,
				thp_power_id2name(power_id));
		return -ENODEV;
	}
	switch (power->type) {
	case THP_POWER_LDO:
		if (IS_ERR_OR_NULL(power->regulator)) {
			thp_log_err("%s:fail to get %s\n", __func__,
				thp_power_id2name(power_id));
			return -ENODEV;
		}
		rc = status ? regulator_enable(power->regulator) :
			regulator_disable(power->regulator);
		break;
	case THP_POWER_GPIO:
		gpio_direction_output(power->gpio, status ? 1 : 0);
		break;

#if (!defined CONFIG_HUAWEI_THP_QCOM) && (!defined CONFIG_HUAWEI_THP_MTK)
	case THP_POWER_PMIC:
		tp_pmic_ldo_set.pmic_power_state = (status ? 1 : 0);
		rc = hw_pmic_power_cfg(TP_PMIC_REQ, &tp_pmic_ldo_set);
		if (rc)
			thp_log_err("%s:pmic %s failed, %d\n", __func__,
				thp_power_id2name(power_id), rc);
		break;
#endif

	default:
		thp_log_err("%s: invalid power type %d\n",
			__func__, power->type);
		break;
	}
exit:
	if (delay_ms)
		mdelay(delay_ms);
	return rc;
}

#define POWER_CONFIG_NAME_MAX 20
static void thp_paser_pmic_power(struct device_node *thp_node,
	struct thp_core_data *cd, int power_id)
{
	const char *power_name = NULL;
	char config_name[POWER_CONFIG_NAME_MAX] = {0};
	struct thp_power_supply *power = NULL;
	int rc;

	power_name = thp_power_id2name(power_id);
	power = &cd->thp_powers[power_id];
	snprintf(config_name, (POWER_CONFIG_NAME_MAX - 1),
		"%s-value", power_name);
	rc = of_property_read_u32(thp_node, config_name,
		&power->pmic_power.value);
	if (rc)
		thp_log_err("%s:failed to get %s\n",
			__func__, config_name);
	snprintf(config_name, (POWER_CONFIG_NAME_MAX - 1),
		"%s-ldo-num", power_name);
	rc = of_property_read_u32(thp_node, config_name,
		&power->pmic_power.ldo_num);
	if (rc)
		thp_log_err("%s:failed to get %s\n",
			__func__, config_name);
	snprintf(config_name, (POWER_CONFIG_NAME_MAX - 1),
		"%s-pmic-num", power_name);
	rc = of_property_read_u32(thp_node, config_name,
		&power->pmic_power.pmic_num);
	if (rc)
		thp_log_err("%s:failed to get %s\n",
			__func__, config_name);
	thp_log_info("%s: to get %d, %d,%d\n", __func__,
		power->pmic_power.ldo_num,
		power->pmic_power.pmic_num,
		power->pmic_power.value);
}

static int thp_parse_one_power(struct device_node *thp_node,
	struct thp_core_data *cd, int power_id)
{
	const char *power_name = NULL;
	char config_name[POWER_CONFIG_NAME_MAX] = {0};
	struct thp_power_supply *power = NULL;
	int rc;

	power_name = thp_power_id2name(power_id);
	power = &cd->thp_powers[power_id];

	rc = snprintf(config_name, POWER_CONFIG_NAME_MAX - 1,
		"%s-type", power_name);

	thp_log_info("%s:parse power: %s\n", __func__, config_name);

	rc = of_property_read_u32(thp_node, config_name, &power->type);
	if (rc || power->type == THP_POWER_UNUSED) {
		thp_log_info("%s: power %s type not config or 0, unused\n",
			__func__, config_name);
		return 0;
	}

	switch (power->type) {
	case THP_POWER_GPIO:
		snprintf(config_name, POWER_CONFIG_NAME_MAX - 1,
			"%s-gpio", power_name);
		power->gpio = of_get_named_gpio(thp_node, config_name, 0);
		if (!gpio_is_valid(power->gpio)) {
			thp_log_err("%s:failed to get %s\n",
				__func__, config_name);
			return -ENODEV;
		}
		break;
	case THP_POWER_LDO:
		snprintf(config_name, POWER_CONFIG_NAME_MAX - 1,
			"%s-value", power_name);
		rc = of_property_read_u32(thp_node, config_name,
				&power->ldo_value);
		if (rc) {
			thp_log_err("%s:failed to get %s\n",
				__func__, config_name);
			return rc;
		}
		break;
	case THP_POWER_PMIC:
		thp_paser_pmic_power(thp_node, cd, power_id);
		break;
	default:
		thp_log_err("%s: invaild power type %d", __func__, power->type);
		break;
	}

	return 0;
}

static int thp_parse_power_config(struct device_node *thp_node,
	struct thp_core_data *cd)
{
	int rc;
	int i;

	for (i = 0; i < THP_POWER_ID_MAX; i++) {
		rc = thp_parse_one_power(thp_node, cd, i);
		if (rc)
			return rc;
	}

	return 0;
}

int is_valid_project_id(const char *id)
{
	while (*id != '\0') {
		if ((*id & BIT_MASK(PROJECTID_BIT_MASK_NUM)) || (!isalnum(*id)))
			return false;
		id++;
	}

	return true;
}


#define GET_HWLOCK_FAIL 0
int thp_bus_lock(void)
{
	int ret;
	unsigned long time;
	unsigned long timeout;
	struct thp_core_data *cd = thp_get_core_data();
	struct hwspinlock *hwlock = cd->hwspin_lock;

	mutex_lock(&cd->spi_mutex);
	if (!cd->use_hwlock)
		return 0;

	timeout = jiffies + msecs_to_jiffies(THP_GET_HARDWARE_TIMEOUT);

	do {
		ret = hwlock->bank->ops->trylock(hwlock);
		if (ret == GET_HWLOCK_FAIL) {
			time = jiffies;
			if (time_after(time, timeout)) {
				thp_log_err("%s:get hardware_mutex timeout\n",
					__func__);
				mutex_unlock(&cd->spi_mutex);
				return -ETIME;
			}
		}
	} while (ret == GET_HWLOCK_FAIL);

	return 0;
}

void thp_bus_unlock(void)
{
	struct thp_core_data *cd = thp_get_core_data();
	struct hwspinlock *hwlock = cd->hwspin_lock;

	mutex_unlock(&cd->spi_mutex);
	if (cd->use_hwlock)
		hwlock->bank->ops->unlock(hwlock);
}

#define QUEUE_IS_FULL 1
#define QUEUE_IS_NOT_FULL 0
static struct thp_queue *thp_queue_init(struct thp_core_data *cd)
{
	int i;
	struct thp_queue *queue = NULL;

	thp_log_info("%s start\n", __func__);
	if (cd->thp_queue_buf_len)
		g_thp_queue_node_len = cd->thp_queue_buf_len;
	if (g_thp_queue_node_len > THP_MAX_FRAME_SIZE) {
		thp_log_err("%s thp queue node len is invalid\n", __func__);
		return NULL;
	}
	thp_log_info("%s:thp queue node len is %u\n",
		__func__, g_thp_queue_node_len);
	queue = kzalloc(sizeof(*queue), GFP_KERNEL);
	if (queue == NULL) {
		thp_log_err("%s queue malloc failed\n", __func__);
		return NULL;
	}
	g_queue_data_buf = vzalloc(g_thp_queue_node_len * THP_LIST_MAX_FRAMES);
	if (!g_queue_data_buf) {
		thp_log_err("%s g_queue_data_buf vzalloc failed\n", __func__);
		kfree(queue);
		queue = NULL;
		return NULL;
	}

	for (i = 0; i < THP_LIST_MAX_FRAMES; i++)
		queue->frame_data[i].buf = g_queue_data_buf +
			(i * g_thp_queue_node_len);

	queue->front = 0;
	queue->tail = 0;
	queue->size = THP_LIST_MAX_FRAMES;
	queue->flag = QUEUE_IS_NOT_FULL;
	return queue;
}

static bool thp_queue_is_empty(struct thp_queue *queue)
{
	if (queue == NULL) {
		thp_log_err("%s queue is NULL\n", __func__);
		return false;
	}
	if ((queue->flag == QUEUE_IS_NOT_FULL) &&
		(queue->front == queue->tail))
		return true;
	else
		return false;
}

static bool thp_queue_is_full(struct thp_queue *queue)
{
	if (queue == NULL) {
		thp_log_err("%s queue is NULL\n", __func__);
		return false;
	}
	if (queue->flag == QUEUE_IS_FULL)
		return true;
	else
		return false;
}

static bool thp_queue_dequeue(struct thp_queue *queue)
{
	if (queue == NULL) {
		thp_log_err("%s queue is NULL\n", __func__);
		return false;
	}
	thp_log_debug("%s in\n", __func__);

	if (thp_queue_is_empty(queue)) {
		thp_log_err("%s queue is empty\n", __func__);
		return false;
	}
	memset(queue->frame_data[queue->front].buf, 0, g_thp_queue_node_len);
	memset(&queue->frame_data[queue->front].tv, 0,
		sizeof(queue->frame_data[queue->front].tv));
	queue->front = ((queue->front + 1) < queue->size) ?
		(queue->front + 1) : 0;
	queue->flag = QUEUE_IS_NOT_FULL;
	return true;
}

static bool thp_queue_enqueue(struct thp_queue *queue,
	u8 *read_buf, unsigned int len)
{
	if ((read_buf == NULL) || (queue == NULL)) {
		thp_log_err("%s read_buf is NULL\n", __func__);
		return false;
	}
	thp_log_debug("%s in\n", __func__);

	if (thp_queue_is_full(queue)) {
		if (!thp_queue_dequeue(queue)) {
			thp_log_err("%s dequeue failed\n", __func__);
			return false;
		}
	}
	if (len > g_thp_queue_node_len) {
		thp_log_debug("%s len is too big\n", __func__);
		len = g_thp_queue_node_len;
	}
	memcpy(queue->frame_data[queue->tail].buf, read_buf, len);
	do_gettimeofday(&queue->frame_data[queue->tail].tv);
	queue->tail = ((queue->tail + 1) < queue->size) ? (queue->tail + 1) : 0;
	if (queue->tail == queue->front)
		queue->flag = QUEUE_IS_FULL;
	return true;
}

static struct thp_queue_node *thp_queue_get_head(
	struct thp_queue *queue)
{
	if (queue == NULL) {
		thp_log_err("%s queue is NULL\n", __func__);
		return NULL;
	}
	if (thp_queue_is_empty(queue)) {
		thp_log_err("%s queue is empty\n", __func__);
		return NULL;
	}
	return &queue->frame_data[queue->front];
}

static void thp_queue_clear_all(struct thp_queue *queue)
{
	int i;

	if (queue == NULL) {
		thp_log_err("%s queue is NULL\n", __func__);
		return;
	}

	for (i = 0; i < THP_LIST_MAX_FRAMES; i++) {
		memset(queue->frame_data[i].buf, 0, g_thp_queue_node_len);
		memset(&queue->frame_data[i].tv, 0,
			sizeof(queue->frame_data[i].tv));
	}

	queue->front = 0;
	queue->tail = 0;
	queue->size = THP_LIST_MAX_FRAMES;
	queue->flag = QUEUE_IS_NOT_FULL;
}

static void thp_queue_free(struct thp_queue *queue)
{
	if (queue == NULL) {
		thp_log_err("%s queue is NULL\n", __func__);
		return;
	}
	vfree(g_queue_data_buf);
	g_queue_data_buf = NULL;
	kfree(queue);
	queue = NULL;
}

static int thp_setup_spi(struct thp_core_data *cd);
int thp_set_spi_max_speed(unsigned int speed)
{
	struct thp_core_data *cd = thp_get_core_data();
	int rc;

	cd->sdev->max_speed_hz = speed;

	thp_log_info("%s:set max_speed_hz %d\n", __func__, speed);
	rc = thp_bus_lock();
	if (rc) {
		thp_log_err("%s: get lock failed\n", __func__);
		return rc;
	}
	if (thp_setup_spi(cd)) {
		thp_log_err("%s: set spi speed fail\n", __func__);
		rc = -EIO;
	}
	thp_bus_unlock();
	return rc;
}

static int thp_wait_frame_waitq(struct thp_core_data *cd)
{
	int t;

	cd->frame_waitq_flag = WAITQ_WAIT;

	/* if not use timeout */
	if (!cd->timeout) {
		t = wait_event_interruptible(cd->frame_waitq,
				(cd->frame_waitq_flag == WAITQ_WAKEUP));
		return 0;
	}

	/* if use timeout */
	t = wait_event_interruptible_timeout(cd->frame_waitq,
			(cd->frame_waitq_flag == WAITQ_WAKEUP),
			msecs_to_jiffies(cd->timeout));
	if (!is_tmo(t))
		return 0;

#if defined(CONFIG_HUAWEI_DSM)
	thp_log_err("%s: wait frame timed out, dmd code:%d\n",
			__func__, DSM_TPHOSTPROCESSING_DEV_STATUS_ERROR_NO);
#ifdef THP_TIMEOUT_DMD
	thp_dmd_report(DSM_TPHOSTPROCESSING_DEV_STATUS_ERROR_NO,
		"%s, wait frame timed out\n", __func__);
#endif
#endif

	return -ETIMEDOUT;
}

int thp_set_status(int type, int status)
{
	struct thp_core_data *cd = thp_get_core_data();

	mutex_lock(&cd->status_mutex);
	status ? __set_bit(type, (unsigned long *)&cd->status) :
		__clear_bit(type, (unsigned long *)&cd->status);
	mutex_unlock(&cd->status_mutex);

	thp_mt_wrapper_wakeup_poll();

	thp_log_info("%s:type=%d value=%d\n", __func__, type, status);
	return 0;
}

int thp_get_status(int type)
{
	struct thp_core_data *cd = thp_get_core_data();

	return test_bit(type, (unsigned long *)&cd->status);
}

u32 thp_get_status_all(void)
{
	struct thp_core_data *cd = thp_get_core_data();

	return cd->status;
}

static void thp_clear_frame_buffer(struct thp_core_data *cd)
{
	struct thp_frame *temp = NULL;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;

	if (cd->use_thp_queue) {
		thp_queue_clear_all(cd->thp_queue);
	} else {
		if (list_empty(&cd->frame_list.list))
			return;
		list_for_each_safe(pos, n, &cd->frame_list.list) {
			temp = list_entry(pos, struct thp_frame, list);
			list_del(pos);
			kfree(temp);
			temp = NULL;
		}
	}
	cd->frame_count = 0;
}

static int thp_spi_transfer(struct thp_core_data *cd,
	char *tx_buf, char *rx_buf, unsigned int len, unsigned int lock_status)
{
	int rc;
	struct spi_message msg;
	struct spi_device *sdev = cd->sdev;
	struct spi_transfer xfer = {
		.tx_buf = tx_buf,
		.rx_buf = rx_buf,
		.len = len,
		.delay_usecs = cd->thp_dev->timing_config.spi_transfer_delay_us,
	};

	if (cd->suspended && (!cd->need_work_in_suspend)) {
		thp_log_err("%s - suspended\n", __func__);
		return 0;
	}

	spi_message_init(&msg);
	spi_message_add_tail(&xfer, &msg);
	if (lock_status == NEED_LOCK) {
		rc = thp_bus_lock();
		if (rc < 0) {
			thp_log_err("%s:get lock failed:%d\n", __func__, rc);
			return rc;
		}
	}
	if (!cd->support_no_cs_pinctrl_for_mtk)
		thp_spi_cs_set(GPIO_HIGH);
	rc = thp_spi_sync(sdev, &msg);
	if (lock_status == NEED_LOCK)
		thp_bus_unlock();

	return rc;
}

/*
 * If irq is disabled/enabled, can not disable/enable again
 * disable - status 0; enable - status not 0
 */
void thp_set_irq_status(struct thp_core_data *cd, int status)
{
	if (!cd) {
		thp_log_err("%s: cd is nullptr\n", __func__);
		return;
	}
	mutex_lock(&cd->irq_mutex);
	if (cd->irq_enabled != (!!status)) {
		status ? enable_irq(cd->irq) : disable_irq(cd->irq);
		cd->irq_enabled = !!status;
		thp_log_info("%s: %s irq\n", __func__,
			status ? "enable" : "disable");
	}
	mutex_unlock(&cd->irq_mutex);
};

/*
 * This function is called for recording the system time when tp
 * receive the suspend cmd from lcd driver for proximity feature.
 */
static void thp_prox_suspend_record_time(struct thp_core_data *cd)
{
	do_gettimeofday(&cd->tp_suspend_record_tv);
	thp_log_info(
		"[Proximity_feature] TP early suspend at %ld secs %ld microseconds\n",
		cd->tp_suspend_record_tv.tv_sec,
		cd->tp_suspend_record_tv.tv_usec);
}

static bool need_work_in_suspend_switch(struct thp_core_data *cd)
{
	bool result = false;
	unsigned int double_tap_status;
	unsigned int ap_gesture_status;

	if (!cd->use_ap_gesture)
		return (cd->easy_wakeup_info.sleep_mode == TS_GESTURE_MODE &&
		cd->support_gesture_mode && !cd->support_shb_thp);

	double_tap_status = (cd->easy_wakeup_info.sleep_mode ==
		TS_GESTURE_MODE) && cd->support_gesture_mode;
	ap_gesture_status = thp_get_status(THP_STATUS_UDFP) ||
		cd->aod_touch_status;
	result = (double_tap_status || ap_gesture_status) &&
		!cd->support_shb_thp;
	return result;
}

static int thp_suspend(struct thp_core_data *cd)
{
	if (cd->suspended) {
		thp_log_info("%s: already suspended, return\n", __func__);
		return 0;
	}
	cd->afe_download_status = false;
	cd->invalid_irq_num = 0; /* clear invalid irq count */
	mutex_lock(&cd->suspend_flag_mutex);
	cd->suspended = true;
	mutex_unlock(&cd->suspend_flag_mutex);
	if (cd->proximity_support == PROX_SUPPORT) {
		if (cd->need_work_in_suspend) {
			thp_log_info(
				"[Proximity_feature] %s: Enter prximity mode, no need suspend!\n",
				__func__);
			return 0;
		}
	}
	if (need_work_in_suspend_switch(cd)) {
#ifdef CONFIG_HUAWEI_THP_MTK
		if (cd->need_enable_irq_wake)
			if (enable_irq_wake(cd->irq))
				thp_log_err("%s:irq wake fail\n", __func__);
#endif
		if (cd->disable_irq_gesture_suspend)
			thp_set_irq_status(cd, THP_IRQ_DISABLE);
		cd->thp_dev->ops->suspend(cd->thp_dev);
		thp_set_irq_status(cd, THP_IRQ_ENABLE);
	} else {
#ifndef CONFIG_HUAWEI_THP_MTK
		if (cd->support_pinctrl == 1)
			thp_pinctrl_select_lowpower(cd->thp_dev);
#endif
		if (cd->open_count)
			thp_set_irq_status(cd, THP_IRQ_DISABLE);
		cd->thp_dev->ops->suspend(cd->thp_dev);
	}
	cd->work_status = SUSPEND_DONE;
	return 0;
}

static int thp_resume(struct thp_core_data *cd)
{
	if (!cd->suspended) {
		thp_log_info("%s: already resumed, return\n", __func__);
		return 0;
	}

	cd->work_status = BEFORE_RESUME;
	thp_log_info("%s: %s\n", __func__, cd->project_id);
	thp_log_info("%s: spi_sync_count_in_suspend = %d, get_get_frame_count_in_suspend = %d\n",
		__func__, spi_sync_count_in_suspend, get_frame_count_in_suspend);
	spi_sync_count_in_suspend = 0;
	get_frame_count_in_suspend = 0;
	cd->thp_dev->ops->resume(cd->thp_dev);

	/* clear rawdata frame buffer list */
	mutex_lock(&cd->mutex_frame);
	thp_clear_frame_buffer(cd);
	mutex_unlock(&cd->mutex_frame);
#ifndef CONFIG_HUAWEI_THP_MTK
	if (cd->support_pinctrl == 1)
		thp_pinctrl_select_normal(cd->thp_dev);
#endif

	if (cd->proximity_support == PROX_SUPPORT) {
		g_thp_prox_enable = cd->prox_cache_enable;
		onetime_poweroff_done = false;
		cd->early_suspended = false;
		thp_log_info(
			"[Proximity_feature] %s: update g_thp_prox_enable to %d!\n",
			__func__, g_thp_prox_enable);
	}
	cd->suspended = false;
	cd->work_status = RESUME_DONE;
	return 0;
}


static void thp_after_resume_work_fn(struct work_struct *work)
{
	struct thp_core_data *cd = thp_get_core_data();

	if (cd->delay_work_for_pm) {
		if (cd->thp_dev->ops->after_resume)
			cd->thp_dev->ops->after_resume(cd->thp_dev);
		thp_set_status(THP_STATUS_POWER, THP_RESUME);
	}
	cd->suspend_resume_waitq_flag = WAITQ_WAKEUP;
	wake_up_interruptible(&(cd->suspend_resume_waitq));
	thp_log_info("%s: after resume end\n", __func__);
}

DECLARE_WORK(thp_after_resume_work, thp_after_resume_work_fn);

/*
 * Add some delay before screenoff to avoid quick resume-suspend TP-firmware
 * does not download success but thp have notified afe screenoff.
 * Use 'wait_event_interruptible_timeout' wait THP_AFE_STATUS_SCREEN_ON
 * notify and set enough timeout make sure of TP-firmware download success.
 * This solution will cause suspend be more slowly.
 */
static void thp_delay_before_screenoff(struct thp_core_data *cd)
{
	int rc;
	u32 suspend_delay_ms;

	if (!cd || !(cd->thp_dev)) {
		thp_log_err("%s: cd is null\n", __func__);
		return;
	}

	suspend_delay_ms = cd->thp_dev->timing_config.early_suspend_delay_ms;
	if ((atomic_read(&(cd->afe_screen_on_waitq_flag)) == WAITQ_WAKEUP) ||
		!suspend_delay_ms) {
		thp_log_info("%s, do not need wait\n", __func__);
		return;
	}

	thp_log_info("%s:wait afe screen on complete\n", __func__);
	rc = wait_event_interruptible_timeout(cd->afe_screen_on_waitq,
		(atomic_read(&(cd->afe_screen_on_waitq_flag)) == WAITQ_WAKEUP),
		msecs_to_jiffies(suspend_delay_ms));
	if (rc)
		return;
	/* if timeout and condition not true, rc is 0 need report DMD */
	atomic_set(&(cd->afe_screen_on_waitq_flag), WAITQ_WAKEUP);
#if defined(CONFIG_HUAWEI_DSM)
	thp_dmd_report(DSM_TPHOSTPROCESSING_DEV_GESTURE_EXP2,
		"%s, screen on %u ms, but fw not ready\n",
		__func__, suspend_delay_ms);
#endif
	thp_log_info("%s, screen on %u ms, but fw not ready\n",
		__func__, suspend_delay_ms);
}

#if defined(CONFIG_LCD_KIT_DRIVER)
#define SUSPEND_WAIT_TIMEOUT 2000
static void thp_early_suspend(struct thp_core_data *cd)
{
	int rc;

	thp_log_info("%s: early suspend,%d\n", __func__,
		cd->suspend_resume_waitq_flag);
	/*
	 * to avoid easy_wakeup_info.sleep_mode being changed during suspend,
	 * assign cd->sleep_mode to easy_wakeup_info.sleep_mode once
	 */
	cd->easy_wakeup_info.sleep_mode = cd->sleep_mode;
	if (cd->multi_panel_index != SINGLE_TOUCH_PANEL)
		cd->easy_wakeup_info.aod_mode = cd->aod_touch_status;
	/*
	 * TDDI need make sure of firmware download complete,
	 * then lcd send 2810 to screen off,
	 * otherwise gesture mode will enter failed.
	 */
	if (cd->wait_afe_screen_on_support)
		thp_delay_before_screenoff(cd);
	if (cd->delay_work_for_pm) {
		if (cd->suspend_resume_waitq_flag != WAITQ_WAKEUP) {
			thp_log_info("%s:wait resume complete\n", __func__);
			rc =  wait_event_interruptible_timeout(
				cd->suspend_resume_waitq,
				(cd->suspend_resume_waitq_flag == WAITQ_WAKEUP),
				SUSPEND_WAIT_TIMEOUT);
			if (!rc)
				thp_log_err("%s:wait resume complete timeout\n",
					__func__);
		}
		thp_set_status(THP_STATUS_POWER, THP_SUSPEND);
		cd->suspend_resume_waitq_flag = WAITQ_WAIT;
	} else {
		if (cd->proximity_support != PROX_SUPPORT) {
			thp_log_info("%s: early suspend\n", __func__);
			if (cd->multi_panel_index == SINGLE_TOUCH_PANEL)
				thp_set_status(THP_STATUS_POWER, THP_SUSPEND);
			/*
			 * if multi screen and in quickly screen switch,
			 * not notify daemon
			 */
			else if (!cd->quickly_screen_on_off)
				thp_set_status(THP_STATUS_POWER, THP_SUSPEND);
			else
				cd->quickly_screen_on_off = false;
		} else {
			thp_prox_suspend_record_time(cd);
			thp_log_info(
				"%s: early suspend! g_thp_prox_enable = %d\n",
				__func__, g_thp_prox_enable);
			cd->need_work_in_suspend = g_thp_prox_enable;
			cd->prox_cache_enable = g_thp_prox_enable;
			cd->early_suspended = true;
			if (cd->need_work_in_suspend)
				thp_set_status(THP_STATUS_AFE_PROXIMITY,
					THP_PROX_ENTER);
			else
				thp_set_status(THP_STATUS_POWER, THP_SUSPEND);
		}
	}
}

static void thp_after_resume(struct thp_core_data *cd)
{
	if (cd->delay_work_for_pm) {
		thp_log_info("%s: after resume called\n", __func__);
		schedule_work(&thp_after_resume_work);
	} else {
		if (cd->proximity_support != PROX_SUPPORT) {
			thp_log_info("%s: after resume\n", __func__);
			if (cd->multi_panel_index == SINGLE_TOUCH_PANEL)
				thp_set_status(THP_STATUS_POWER, THP_RESUME);
			/*
			 * if multi screen and in quickly screen switch,
			 * not notify daemon
			 */
			else if (!cd->quickly_screen_on_off)
				thp_set_status(THP_STATUS_POWER, THP_RESUME);
		} else {
			thp_log_info(
				"%s: after resume! need_work_in_suspend = %d\n",
				__func__, cd->need_work_in_suspend);
			if (cd->need_work_in_suspend)
				thp_set_status(THP_STATUS_AFE_PROXIMITY,
					THP_PROX_EXIT);
			else
				thp_set_status(THP_STATUS_POWER, THP_RESUME);
		}
	}
	if (cd->wait_afe_screen_on_support &&
		(atomic_read(&(cd->afe_screen_on_waitq_flag)) != WAITQ_WAIT))
		atomic_set(&(cd->afe_screen_on_waitq_flag), WAITQ_WAIT);
}
#endif

#ifndef CONFIG_LCD_KIT_DRIVER
static int thp_lcdkit_notifier_callback(struct notifier_block *self,
	unsigned long event, void *data)
{
	struct thp_core_data *cd = thp_get_core_data();
	unsigned long pm_type = event;

	thp_log_debug("%s: called by lcdkit, pm_type=%lu\n", __func__, pm_type);

	switch (pm_type) {
	case LCDKIT_TS_EARLY_SUSPEND:
		/*
		 * to avoid easy_wakeup_info.sleep_mode changed during suspend,
		 * assign cd->sleep_mode to easy_wakeup_info.sleep_mode once
		 */
		cd->easy_wakeup_info.sleep_mode = cd->sleep_mode;
		if (cd->proximity_support != PROX_SUPPORT) {
			thp_log_info("%s: early suspend\n", __func__);
			thp_set_status(THP_STATUS_POWER, THP_SUSPEND);
		} else {
			thp_prox_suspend_record_time(cd);
			thp_log_info("%s:early suspend!g_thp_prox_enable=%d\n",
				__func__, g_thp_prox_enable);
			cd->need_work_in_suspend = g_thp_prox_enable;
			cd->prox_cache_enable = g_thp_prox_enable;
			cd->early_suspended = true;
			if (cd->need_work_in_suspend)
				thp_set_status(THP_STATUS_AFE_PROXIMITY,
					THP_PROX_ENTER);
			else
				thp_set_status(THP_STATUS_POWER, THP_SUSPEND);
		}
		break;

	case LCDKIT_TS_SUSPEND_DEVICE:
		thp_log_info("%s: suspend\n", __func__);
		thp_clean_fingers();
		break;

	case LCDKIT_TS_BEFORE_SUSPEND:
		thp_log_info("%s: before suspend\n", __func__);
		thp_suspend(cd);
		break;

	case LCDKIT_TS_RESUME_DEVICE:
		thp_log_info("%s: resume\n", __func__);
		thp_resume(cd);
		break;

	case LCDKIT_TS_AFTER_RESUME:
		if (cd->proximity_support != PROX_SUPPORT) {
			thp_log_info("%s: after resume\n", __func__);
			thp_set_status(THP_STATUS_POWER, THP_RESUME);
		} else {
			thp_log_info(
				"%s:after resume!need_work_in_suspend=%d\n",
				__func__, cd->need_work_in_suspend);
			if (cd->need_work_in_suspend)
				thp_set_status(THP_STATUS_AFE_PROXIMITY,
				THP_PROX_EXIT);
			else
				thp_set_status(THP_STATUS_POWER, THP_RESUME);
		}
		break;
	default:
		break;
	}

	return 0;
}
#endif

#ifdef CONFIG_HUAWEI_SHB_THP
#define POWER_OFF 0
#define POWER_ON 1
static int send_thp_cmd_to_shb(unsigned char power_status)
{
	struct thp_core_data *cd = thp_get_core_data();
	int ret = -EINVAL;

	if (cd == NULL) {
		thp_log_err("%s: have null ptr\n", __func__);
		return ret;
	}
	if (cd->support_shb_thp == 0) {
		thp_log_info("%s: not support shb\n", __func__);
		return ret;
	}
	switch (power_status) {
	case POWER_OFF:
		if (!cd->poweroff_function_status) {
			ret = send_thp_close_cmd();
			thp_log_info("%s: close shb thp, ret is %d\n",
				__func__, ret);
		} else {
			ret = send_thp_driver_status_cmd(power_status,
				cd->poweroff_function_status,
				ST_CMD_TYPE_SET_SCREEN_STATUS);
			thp_log_info(
				"%s: call poweroff_function_status = 0x%x\n",
				__func__, cd->poweroff_function_status);
		}
		break;
	case POWER_ON:
		if (!cd->poweroff_function_status) {
			ret = send_thp_open_cmd();
			thp_log_info("%s: open shb thp, ret is %d\n",
				__func__, ret);
		}
		ret = send_thp_driver_status_cmd(power_status,
			POWER_KEY_ON_CTRL, ST_CMD_TYPE_SET_SCREEN_STATUS);
		thp_log_info("%s: power on and send status\n", __func__);
		break;
	default:
		thp_log_err("%s: invaild power_status: %u\n",
			__func__, power_status);
	}
	if (ret)
		thp_log_err("%s: ret = %d\n", __func__, ret);
	return ret;
}
#endif

#if defined(CONFIG_LCD_KIT_DRIVER)
int thp_proc_screen_switch_cmd(enum lcd_kit_ts_pm_type pm_type, int timeout,
	int panel_index)
{
	struct thp_core_data *cd = thp_get_core_data();
	int ret;

	if (!cd) {
		thp_log_err("%s: tp is not registered\n", __func__);
		return -ENODEV;
	}
	thp_log_info("%s: called, index:%d, pm_type:%u\n", __func__,
		panel_index, pm_type);

	switch (panel_index) {
	case MAIN_TOUCH_PANEL:
		ret = thp_power_control_notify(pm_type, timeout);
		break;
	case SUB_TOUCH_PANEL:
#ifdef CONFIG_HUAWEI_TS_KIT_3_0
		if (cd->multi_panel_index != SINGLE_TOUCH_PANEL) {
			if (pm_type == TS_SUSPEND_DEVICE)
				cd->ts_suspended = true;
			if (pm_type == TS_AFTER_RESUME)
				cd->ts_suspended = false;
		}
		ret  = ts_kit_multi_power_control_notify(pm_type, timeout,
			panel_index);
#endif
		break;
	default:
		thp_log_err("%s: invalid index: %d\n", __func__, panel_index);
		return -EINVAL;
	}
	if (ret)
		thp_log_err("%s:control notify error\n", __func__);
	return ret;
}

#define PM_TYPE_LIST_LEN 5
int multi_screen_status_switch(u32 current_status)
{
	struct thp_core_data *cd = thp_get_core_data();
	int i;
	enum lcd_kit_ts_pm_type pm_type_list[PM_TYPE_LIST_LEN] = {
		TS_RESUME_DEVICE,
		TS_AFTER_RESUME,
		TS_EARLY_SUSPEND,
		TS_BEFORE_SUSPEND,
		TS_SUSPEND_DEVICE
	};

	if (!cd) {
		thp_log_err("%s: tp is not registered\n", __func__);
		return -ENODEV;
	}
	if ((cd->sleep_mode == TS_POWER_OFF_MODE) &&
		(cd->aod_touch_status == 0)) {
#ifdef CONFIG_HUAWEI_TS_KIT_3_0
		ts_kit_screen_status_notify(current_status);
#endif
		if ((current_status == SCREEN_OFF_FOLD) ||
			(current_status == SCREEN_FOLDED))
			cd->multi_screen_fold = true;
		else if (current_status == SCREEN_UNFOLD)
			cd->multi_screen_fold = false;
		else
			thp_log_err("%s: invalid status: %u\n",
				__func__, current_status);
		thp_log_info("%s: power off mode, no need switch power status\n",
			__func__);
		return 0;
	}
	if (current_status == SCREEN_OFF_FOLD) {
#ifdef CONFIG_HUAWEI_TS_KIT_3_0
		/* before fold, nofity sub panel */
		ts_kit_screen_status_notify(current_status);
#endif
		cd->quickly_screen_on_off = true;
		thp_proc_screen_switch_cmd(TS_SCREEN_SWITCH,
			NO_SYNC_TIMEOUT, MAIN_TOUCH_PANEL);
		for (i = 0; i < PM_TYPE_LIST_LEN; i++) {
			if (pm_type_list[i] == TS_EARLY_SUSPEND)
				msleep(200); /* 200ms wait ic ready */

			thp_proc_screen_switch_cmd(pm_type_list[i],
				SHORT_SYNC_TIMEOUT, SUB_TOUCH_PANEL);
			thp_proc_screen_switch_cmd(pm_type_list[i],
				NO_SYNC_TIMEOUT, MAIN_TOUCH_PANEL);
		}
		cd->multi_screen_fold = true;
		thp_log_info("%s: switch to SCREEN_OFF_FOLD\n", __func__);
	} else if (current_status == SCREEN_FOLDED) {
#ifdef CONFIG_HUAWEI_TS_KIT_3_0
		/* before fold, nofity sub panel */
		ts_kit_screen_status_notify(current_status);
#endif
		if (cd->ts_suspended && cd->suspended) {
			cd->quickly_screen_on_off = true;
			thp_proc_screen_switch_cmd(TS_SCREEN_SWITCH,
				NO_SYNC_TIMEOUT, MAIN_TOUCH_PANEL);
			for (i = 0; i < PM_TYPE_LIST_LEN; i++) {
				if (pm_type_list[i] == TS_EARLY_SUSPEND)
					msleep(200); /* 200ms wait ic ready */

				thp_proc_screen_switch_cmd(pm_type_list[i],
					SHORT_SYNC_TIMEOUT, SUB_TOUCH_PANEL);
				thp_proc_screen_switch_cmd(pm_type_list[i],
					NO_SYNC_TIMEOUT, MAIN_TOUCH_PANEL);
			}
		}

		if (cd->ts_suspended && !cd->suspended)
			thp_proc_screen_switch_cmd(TS_SCREEN_SWITCH,
				NO_SYNC_TIMEOUT, MAIN_TOUCH_PANEL);
		/* finish fold, notify main panel and it will power off */
		cd->multi_screen_fold = true;
		thp_log_info("%s: switch to SCREEN_FOLDED\n", __func__);
	} else if (current_status == SCREEN_UNFOLD) {
		/* before unfold, nofity main panel */
		cd->multi_screen_fold = false;
		thp_proc_screen_switch_cmd(TS_SCREEN_SWITCH,
			SHORT_SYNC_TIMEOUT, SUB_TOUCH_PANEL);
		if (cd->ts_suspended && cd->suspended) {
			for (i = 0; i < PM_TYPE_LIST_LEN; i++) {
				if (pm_type_list[i] == TS_EARLY_SUSPEND)
					msleep(200); /* 200ms wait ic ready */
				thp_proc_screen_switch_cmd(pm_type_list[i],
					SHORT_SYNC_TIMEOUT, SUB_TOUCH_PANEL);
			}
		}
#ifdef CONFIG_HUAWEI_TS_KIT_3_0
		/* finish unfold , notify sub panel and it will power off */
		ts_kit_screen_status_notify(current_status);
#endif
		thp_log_info("%s: switch to SCREEN_UNFOLD\n", __func__);
	}
	return 0;
}

static void thp_easy_wake_mode_change(struct thp_core_data *cd)
{
	if (cd->sleep_mode != TS_POWER_OFF_MODE) {
		cd->sleep_mode_temp = cd->sleep_mode;
		cd->sleep_mode = TS_POWER_OFF_MODE;
	}
	if (cd->aod_touch_status != TS_TOUCH_AOD_CLOSE) {
		cd->aod_mode_temp = cd->aod_touch_status;
		cd->aod_touch_status = TS_TOUCH_AOD_CLOSE;
	}
	thp_log_info("MAIN_TOUCH_PANEL enter TS_SCREEN_SWITCH mode\n");
}

static int pm_type_switch(struct thp_core_data *cd,
	enum lcd_kit_ts_pm_type pm_type)
{
#ifdef CONFIG_HUAWEI_SHB_THP
	int ret;
	char fp_event_to_udfp[UDFP_EVENT_DATA_LEN] = {0};
#endif
#ifdef CONFIG_HUAWEI_THP_MTK
	struct thp_udfp_data udfp_data;
#endif
#if defined(CONFIG_TEE_TUI)
	if (cd->send_tui_exit_in_suspend &&
		tui_enable &&
		(pm_type == TS_EARLY_SUSPEND)) {
		thp_log_info("In TUI mode, need exit TUI before suspend\n");
		thp_tui_secos_exit();
	}
#endif
	/* just for multi-screen switch call, not put in switch-case */
	if (cd->multi_panel_index != SINGLE_TOUCH_PANEL &&
		pm_type == TS_SCREEN_SWITCH) {
		cd->screen_switch_flag = true;
		return 0;
	}
	switch (pm_type) {
	case TS_EARLY_SUSPEND:
		if ((cd->multi_panel_index != SINGLE_TOUCH_PANEL) &&
			(cd->screen_switch_flag || cd->multi_screen_fold)) {
			thp_easy_wake_mode_change(cd);
			cd->screen_switch_flag = false;
		}
		thp_early_suspend(cd);
		break;

	case TS_SUSPEND_DEVICE:
		if (cd->multi_panel_index != SINGLE_TOUCH_PANEL) {
			if (cd->sleep_mode_temp != TS_POWER_OFF_MODE) {
				cd->sleep_mode = cd->sleep_mode_temp;
				cd->sleep_mode_temp = TS_POWER_OFF_MODE;
				thp_log_info("MAIN_PANEL sleep_mode revert\n");
			}
			if (cd->aod_mode_temp != TS_TOUCH_AOD_CLOSE) {
				cd->aod_touch_status = cd->aod_mode_temp;
				cd->aod_mode_temp = TS_TOUCH_AOD_CLOSE;
				thp_log_info("MAIN_PANEL aod_mode revert\n");
			}
		}
		thp_log_info("%s: suspend\n", __func__);
		thp_clean_fingers();
		break;

	case TS_BEFORE_SUSPEND:
		thp_log_info("%s: before suspend\n", __func__);
		if (cd->support_daemon_init_protect &&
			atomic_read(&(cd->fw_update_protect))) {
			thp_log_info("%s: suspend when fw update, return directly\n",
				__func__);
			thp_set_status(THP_STATUS_POWER, THP_RESUME);
			atomic_set(&(cd->resend_suspend_after_fw_update), 1);
			return 0;
		}
		thp_suspend(cd);
#ifdef CONFIG_HUAWEI_SHB_THP
		if (cd->support_shb_thp_app_switch) {
			ret = send_thp_cmd_to_shb(POWER_OFF);
			if (ret)
				thp_log_err("%s: send_thp_cmd_to_shb fail\n",
					__func__);
			break;
		}
		if (cd->support_shb_thp) {
			thp_log_info(
				"%s call poweroff_function_status = 0x%x\n",
				__func__, cd->poweroff_function_status);
			/* 0: power off */
			ret = send_thp_driver_status_cmd(0,
				cd->poweroff_function_status,
				ST_CMD_TYPE_SET_SCREEN_STATUS);
			if (ret)
				thp_log_err("%s: send_thp_status_off fail",
					__func__);
		}
#endif
		break;

	case TS_RESUME_DEVICE:
		thp_log_info("%s: resume\n", __func__);
		if (cd->support_daemon_init_protect &&
			atomic_read(&(cd->fw_update_protect)) &&
			atomic_read(&(cd->resend_suspend_after_fw_update))) {
			atomic_set(&(cd->resend_suspend_after_fw_update), 0);
			thp_log_info("%s: resume when fw update, return directly\n",
				__func__);
			return 0;
		}
		thp_resume(cd);
#ifdef CONFIG_HUAWEI_SHB_THP
		if (cd->support_shb_thp_app_switch) {
			ret = send_thp_cmd_to_shb(POWER_ON);
			if (ret)
				thp_log_err("%s: send_thp_cmd_to_shb fail\n",
					__func__);
			break;
		}
		if (cd->support_shb_thp) {
			/* 1: power on */
			ret = send_thp_driver_status_cmd(1, POWER_KEY_ON_CTRL,
				ST_CMD_TYPE_SET_SCREEN_STATUS);
			if (ret)
				thp_log_err("%s: send_thp_status_on fail",
					__func__);
		}
		if (cd->tsa_event_to_udfp) {
		/* fp_event_to_udfp[UDFP_EVENT_DATA_LEN - 1] = 1 is finger up */
			fp_event_to_udfp[UDFP_EVENT_DATA_LEN - 1] = 1;
			ret = send_tp_ap_event(UDFP_EVENT_DATA_LEN,
				fp_event_to_udfp,
				ST_CMD_TYPE_FINGERPRINT_EVENT);
			if (ret < 0)
				thp_log_err("%s:tsa_event notify fp err %d\n",
					__func__, ret);
		}
#endif
#ifdef CONFIG_HUAWEI_THP_MTK
		if (cd->use_ap_gesture) {
			memset(&udfp_data, 0, sizeof(udfp_data));
			/* udfp_event = 1 is finger up */
			udfp_data.tpud_data.udfp_event = 1;
			thp_log_err("%s:resume send fingerup to fp\n", __func__);
			send_event_to_fingerprint_ud(udfp_data);
		}
#endif
		break;

	case TS_AFTER_RESUME:
		thp_after_resume(cd);
		break;
	case TS_2ND_POWER_OFF:
		if (cd->thp_dev && cd->thp_dev->ops &&
			cd->thp_dev->ops->second_poweroff)
			cd->thp_dev->ops->second_poweroff();
		break;
	default:
		break;
	}

	return 0;
}

int thp_power_control_notify(enum lcd_kit_ts_pm_type pm_type, int timeout)
{
	struct thp_core_data *cd = thp_get_core_data();

	if (thp_is_factory() && cd->always_poweron_in_screenoff) {
		thp_log_info("%s screenoff cap testing, NO poweroff\n",
			__func__);
		return 0;
	}
	thp_log_debug("%s: called by lcdkit, pm_type=%d\n", __func__, pm_type);
#ifdef CONFIG_HUAWEI_THP_MTK
	/* If the AP side AOD display function is enabled, including
	 * click-display, all-day display, and fingerprint, the
	 * AOD controls the TP power status.
	 */
	if (cd->use_aod_power_ctrl_notify && cd->aod_notify_tp_power_status) {
		thp_log_info("%s: use AOD notify, return\n", __func__);
		return 0;
	}
#endif
	return pm_type_switch(cd, pm_type);
}

#define PM_STATUS_BIT1 2
#define PM_STATUS_BIT0 1
static void thp_set_multi_pm_status(enum lcd_kit_ts_pm_type pm_type,
	int panel_index)
{
	struct thp_core_data *cd = thp_get_core_data();

	if (panel_index == MAIN_TOUCH_PANEL) {
		if (pm_type == TS_EARLY_SUSPEND)
			cd->current_pm_status &= ~PM_STATUS_BIT1;
		else if (pm_type == TS_RESUME_DEVICE)
			cd->current_pm_status |= PM_STATUS_BIT1;
	} else if (panel_index == SUB_TOUCH_PANEL) {
		if (pm_type == TS_EARLY_SUSPEND)
			cd->current_pm_status &= ~PM_STATUS_BIT0;
		else if (pm_type == TS_RESUME_DEVICE)
			cd->current_pm_status |= PM_STATUS_BIT0;
	}
}

int thp_multi_power_control_notify(enum lcd_kit_ts_pm_type pm_type, int timeout,
	int panel_index)
{
	struct thp_core_data *cd = thp_get_core_data();
	int ret;
	struct thp_cmd_node cmd;
#ifdef CONFIG_HUAWEI_TS_KIT_3_0
	static int boot_screen_off_panel;
#endif

	if (!cd) {
		thp_log_err("%s: tp is not registered\n", __func__);
		return -ENODEV;
	}
	thp_log_info("%s: called, index:%d, pm_type:%u\n", __func__,
		panel_index, pm_type);
#ifdef CONFIG_HUAWEI_TS_KIT_3_0
	/*
	 * when boot firstly, lcd will notify tp to trun off the unused panel
	 * and we should change the unused panel to power_off state
	 */
	if (boot_screen_off_panel == 0) {
		if (panel_index == MAIN_TOUCH_PANEL)
			cd->multi_screen_fold = true;
		else
			ts_kit_screen_status_notify(SCREEN_UNFOLD);
		boot_screen_off_panel = BOOT_SCREEN_OFF_DONE;
	}
#endif
	if ((pm_type == TS_AFTER_RESUME) ||
		(pm_type == TS_BEFORE_SUSPEND) ||
		(pm_type == TS_SUSPEND_DEVICE)) {
		thp_log_info("%s: ignore %d", __func__, pm_type);
		return 0;
	}
	memset(&cmd, 0, sizeof(cmd));
	if (panel_index == MAIN_TOUCH_PANEL) {
		if (pm_type == TS_RESUME_DEVICE)
			cmd.command = THP_MUTIL_RESUME_THREAD;
		else
			cmd.command = THP_MUTIL_SUSPEND_THREAD;
	} else if (panel_index == SUB_TOUCH_PANEL) {
		if (pm_type == TS_RESUME_DEVICE)
			cmd.command = TSKIT_MUTIL_RESUME_THREAD;
		else
			cmd.command = TSKIT_MUTIL_SUSPEND_THREAD;
	} else {
		thp_log_err("%s: invalid index: %d\n", __func__, panel_index);
		return -EINVAL;
	}
	cmd.cmd_param.pub_params.dev = cd->thp_dev;
	ret = thp_put_one_cmd(&cmd, 0);
	if (ret)
		thp_log_err("%s: put cmd error :%d\n", __func__, ret);
	return ret;
}

static void thp_multi_resume(void)
{
	int err;

	err = thp_power_control_notify(TS_RESUME_DEVICE, NO_SYNC_TIMEOUT);
	if (err)
		thp_log_err("%s: TS_RESUME_DEVICE fail\n", __func__);
	err = thp_power_control_notify(TS_AFTER_RESUME, NO_SYNC_TIMEOUT);
	if (err)
		thp_log_err("%s: TS_AFTER_RESUME fail\n", __func__);
	thp_set_multi_pm_status(TS_RESUME_DEVICE, MAIN_TOUCH_PANEL);
}

static void thp_multi_suspend(void)
{
	int err;

	thp_set_multi_pm_status(TS_EARLY_SUSPEND, MAIN_TOUCH_PANEL);
	err = thp_power_control_notify(TS_EARLY_SUSPEND, NO_SYNC_TIMEOUT);
	if (err)
		thp_log_err("%s: TS_EARLY_SUSPEND fail\n", __func__);
	err = thp_power_control_notify(TS_BEFORE_SUSPEND, NO_SYNC_TIMEOUT);
	if (err)
		thp_log_err("%s: TS_BEFORE_SUSPEND fail\n", __func__);
	err = thp_power_control_notify(TS_SUSPEND_DEVICE, NO_SYNC_TIMEOUT);
	if (err)
		thp_log_err("%s: TS_SUSPEND_DEVICE fail\n", __func__);
}

static void tskit_multi_resume(void)
{
	struct thp_core_data *cd = thp_get_core_data();
#ifdef CONFIG_HUAWEI_TS_KIT_3_0
	int err;
#endif

	cd->ts_suspended = false;
#ifdef CONFIG_HUAWEI_TS_KIT_3_0
	err = ts_kit_multi_power_control_notify(TS_RESUME_DEVICE,
		SHORT_SYNC_TIMEOUT, SUB_TOUCH_PANEL);
	if (err)
		thp_log_err("%s: TS_RESUME_DEVICE fail\n", __func__);
	err = ts_kit_multi_power_control_notify(TS_AFTER_RESUME,
		SHORT_SYNC_TIMEOUT, SUB_TOUCH_PANEL);
	if (err)
		thp_log_err("%s: TS_AFTER_RESUME fail\n", __func__);
#endif
	thp_set_multi_pm_status(TS_RESUME_DEVICE, SUB_TOUCH_PANEL);
}

static void tskit_multi_suspend(void)
{
	struct thp_core_data *cd = thp_get_core_data();
#ifdef CONFIG_HUAWEI_TS_KIT_3_0
	int err;
#endif

	cd->ts_suspended = true;
	thp_set_multi_pm_status(TS_EARLY_SUSPEND, SUB_TOUCH_PANEL);
#ifdef CONFIG_HUAWEI_TS_KIT_3_0
	err = ts_kit_multi_power_control_notify(TS_EARLY_SUSPEND,
		SHORT_SYNC_TIMEOUT, SUB_TOUCH_PANEL);
	if (err)
		thp_log_err("%s: TS_EARLY_SUSPEND fail\n", __func__);
	err = ts_kit_multi_power_control_notify(TS_BEFORE_SUSPEND,
		SHORT_SYNC_TIMEOUT, SUB_TOUCH_PANEL);
	if (err)
		thp_log_err("%s: TS_BEFORE_SUSPEND fail\n", __func__);
	err = ts_kit_multi_power_control_notify(TS_SUSPEND_DEVICE,
		SHORT_SYNC_TIMEOUT, SUB_TOUCH_PANEL);
	if (err)
		thp_log_err("%s: TS_SUSPEND_DEVICE fail\n", __func__);
#endif
}
#endif

#ifdef CONFIG_HUAWEI_THP_MTK
static void thp_suspend_thread(void)
{
	struct thp_core_data *cd = thp_get_core_data();

	if (!cd) {
		thp_log_err("%s: null pointer\n", __func__);
		return;
	}
	pm_type_switch(cd, TS_EARLY_SUSPEND);
	msleep(cd->suspend_delayms_early_to_before);
	pm_type_switch(cd, TS_BEFORE_SUSPEND);
	msleep(cd->suspend_delayms_before_to_later);
	pm_type_switch(cd, TS_SUSPEND_DEVICE);
}

static void thp_resume_thread(void)
{
	struct thp_core_data *cd = thp_get_core_data();

	if (!cd) {
		thp_log_err("%s: null pointer\n", __func__);
		return;
	}
	pm_type_switch(cd, TS_RESUME_DEVICE);
	msleep(cd->resume_delayms_early_to_later);
	pm_type_switch(cd, TS_AFTER_RESUME);
}

void aod_status_notifier(struct thp_core_data *cd, unsigned int status)
{
	static unsigned int last_status = INVALID_VALUE;
	struct thp_cmd_node cmd;

	thp_log_info("%s: status is %u\n", __func__, status);
	if (status == last_status) {
		thp_log_info("%s: repeat event, dont handle!!\n", __func__);
		return;
	}
	memset(&cmd, 0, sizeof(cmd));
	switch (status) {
	case AOD_NOTIFY_TP_SUSPEND:
		cmd.command = TS_SUSPEND;
		break;
	case AOD_NOTIFY_TP_RESUME:
		cmd.command = TS_RESUME;
		break;
	default:
		cmd.command = TS_INVAILD_CMD;
		break;
	}
	last_status = status;
	if (thp_put_one_cmd(&cmd, NO_SYNC_TIMEOUT))
		thp_log_err("%s: put cmd error\n", __func__);
}
#endif

static int thp_open(struct inode *inode, struct file *filp)
{
	int ret;
	struct thp_core_data *cd = thp_get_core_data();

	thp_log_info("%s: called\n", __func__);

	mutex_lock(&cd->thp_mutex);
	if (cd->open_count) {
		thp_log_err("%s: dev have be opened\n", __func__);
		mutex_unlock(&cd->thp_mutex);
		return -EBUSY;
	}

	cd->open_count++;
	mutex_unlock(&cd->thp_mutex);
	cd->reset_flag = 0; /* current isn't in reset status */
	cd->get_frame_block_flag = THP_GET_FRAME_BLOCK;

	cd->frame_size = THP_MAX_FRAME_SIZE;
#ifdef THP_NOVA_ONLY
	cd->frame_size = NT_MAX_FRAME_SIZE;
#endif
	cd->timeout = THP_DEFATULT_TIMEOUT_MS;

	/*
	 * Daemon default is 0
	 * setting to 1 will trigger daemon to init or restore the status
	 */
	__set_bit(THP_STATUS_WINDOW_UPDATE, (unsigned long *)&cd->status);
	__set_bit(THP_STATUS_TOUCH_SCENE, (unsigned long *)&cd->status);

	thp_log_info("%s: cd->status = 0x%x\n", __func__, cd->status);
	mutex_lock(&cd->mutex_frame);
	thp_clear_frame_buffer(cd);
	mutex_unlock(&cd->mutex_frame);
	/* restore spi config */
	ret = thp_set_spi_max_speed(cd->spi_config.max_speed_hz);
	if (ret)
		thp_log_err("%s: thp_set_spi_max_speed error\n", __func__);

	return 0;
}

static int thp_release(struct inode *inode, struct file *filp)
{
	struct thp_core_data *cd = thp_get_core_data();

	thp_log_info("%s: called\n", __func__);

	mutex_lock(&cd->thp_mutex);
	cd->open_count--;
	if (cd->open_count < 0) {
		thp_log_err("%s: abnormal release\n", __func__);
		cd->open_count = 0;
	}
	mutex_unlock(&cd->thp_mutex);

	thp_wake_up_frame_waitq(cd);
	thp_set_irq_status(cd, THP_IRQ_DISABLE);

	if (locked_by_daemon) {
		thp_log_info("%s: need unlock here\n", __func__);
		thp_bus_unlock();
		locked_by_daemon = false;
	}

	return 0;
}

static int thp_spi_sync_alloc_mem(void)
{
	if (spi_sync_rx_buf || spi_sync_tx_buf) {
		thp_log_debug("%s: has requested memory\n", __func__);
		return 0;
	}

	spi_sync_rx_buf = kzalloc(THP_SYNC_DATA_MAX, GFP_KERNEL);
	if (!spi_sync_rx_buf) {
		thp_log_err("%s:spi_sync_rx_buf request memory fail\n",
			__func__);
		goto exit;
	}
	spi_sync_tx_buf = kzalloc(THP_SYNC_DATA_MAX, GFP_KERNEL);
	if (!spi_sync_tx_buf) {
		thp_log_err("%s:spi_sync_tx_buf request memory fail\n",
			__func__);
		kfree(spi_sync_rx_buf);
		spi_sync_rx_buf = NULL;
		goto exit;
	}
	return 0;

exit:
	return -ENOMEM;
}

static unsigned int thp_get_spi_msg_lens(
	struct thp_ioctl_spi_xfer_data *spi_data, unsigned int xfer_num)
{
	int length = 0;
	int i;

	if (!spi_data || !xfer_num || (xfer_num > MAX_SPI_XFER_DATA_NUM)) {
		thp_log_err("%s:invalid input\n", __func__);
		return 0;
	}
	for (i = 0; i < xfer_num ; i++) {
		if ((spi_data[i].len == 0) ||
			(spi_data[i].len > THP_MAX_FRAME_SIZE)) {
			thp_log_err("%s:spi_data[i].len invalid\n", __func__);
			return 0;
		}
		length += spi_data[i].len;
		thp_log_debug("%s: spi_data[i].len = %d\n",
			__func__, spi_data[i].len);
	}
	return length;
}

static long thp_ioctl_multiple_spi_xfer_sync(
	const void __user *data, unsigned int lock_status)
{
	struct thp_ioctl_spi_msg_package ioctl_spi_msg;
	struct thp_core_data *cd = thp_get_core_data();
	struct thp_ioctl_spi_xfer_data *ioctl_spi_xfer = NULL;
	struct spi_message msg;
	struct spi_transfer *xfer = NULL;
	int rc;
	int i;
	unsigned int current_speed = 0;
	u8 *tx_buf = NULL;
	u8 *rx_buf = NULL;
	unsigned int msg_len;

	memset(&ioctl_spi_msg, 0, sizeof(ioctl_spi_msg));
	if (cd->suspended || (!data)) {
		thp_log_info("%s:suspended or invalid data,return\n", __func__);
		return -EIO;
	}
#if defined(CONFIG_TEE_TUI)
	if (tui_enable) {
		thp_log_info("%s:TUI status,return!\n", __func__);
		return -EIO;
	}
#endif

	if (copy_from_user(&ioctl_spi_msg, data, sizeof(ioctl_spi_msg))) {
		thp_log_err("%s:Failed to copy spi data\n", __func__);
		return -EFAULT;
	}
	if ((ioctl_spi_msg.xfer_num > MAX_SPI_XFER_DATA_NUM) ||
		!ioctl_spi_msg.xfer_num) {
		thp_log_err("xfer_num:%d\n", ioctl_spi_msg.xfer_num);
		return -EINVAL;
	}

	ioctl_spi_xfer = kcalloc(ioctl_spi_msg.xfer_num,
			sizeof(*ioctl_spi_xfer), GFP_KERNEL);
	if (!ioctl_spi_xfer) {
		thp_log_err("%s:failed alloc memory for spi_xfer_data\n",
			__func__);
		return -EFAULT;
	}
	if (ioctl_spi_msg.xfer_data) {
		if (copy_from_user(ioctl_spi_xfer, ioctl_spi_msg.xfer_data,
			sizeof(*ioctl_spi_xfer) * ioctl_spi_msg.xfer_num)) {
			thp_log_err("%s:failed copy xfer_data\n", __func__);
			rc = -EINVAL;
			goto exit;
		}
	}
	msg_len = thp_get_spi_msg_lens(ioctl_spi_xfer, ioctl_spi_msg.xfer_num);
	if (msg_len > THP_MAX_FRAME_SIZE || !msg_len) {
		thp_log_err("%s:invalid msg len :%d\n", __func__, msg_len);
		rc = -EINVAL;
		goto exit;
	}

	xfer = kcalloc(ioctl_spi_msg.xfer_num, sizeof(*xfer), GFP_KERNEL);
	rx_buf = kzalloc(msg_len, GFP_KERNEL);
	tx_buf = kzalloc(msg_len, GFP_KERNEL);
	if (!rx_buf || !tx_buf || !xfer) {
		thp_log_err("%s:failed alloc buffer\n", __func__);
		rc = -EINVAL;
		goto exit;
	}

	spi_message_init(&msg);
	for (i = 0, msg_len = 0; i < ioctl_spi_msg.xfer_num; i++) {
		if (ioctl_spi_xfer[i].tx) {
			rc = copy_from_user(tx_buf + msg_len,
					ioctl_spi_xfer[i].tx,
					ioctl_spi_xfer[i].len);
			if (rc) {
				thp_log_err("%s:failed copy tx_buf:%d\n",
					__func__, rc);
				goto exit;
			}
		}
		xfer[i].tx_buf = tx_buf + msg_len;
		xfer[i].rx_buf = rx_buf + msg_len;
		xfer[i].len = ioctl_spi_xfer[i].len;
		xfer[i].cs_change = !!ioctl_spi_xfer[i].cs_change;
		xfer[i].delay_usecs = ioctl_spi_xfer[i].delay_usecs;

		thp_log_debug(
			"%s:%d, cs_change=%d,len =%d,delay_usecs=%d\n",
			__func__, i, ioctl_spi_xfer[i].cs_change,
			ioctl_spi_xfer[i].len, ioctl_spi_xfer[i].delay_usecs);
		spi_message_add_tail(&xfer[i], &msg);
		msg_len += ioctl_spi_xfer[i].len;
	}
	if (lock_status == NEED_LOCK) {
		rc = thp_bus_lock();
		if (rc) {
			thp_log_info("%s:failed get lock:%d", __func__, rc);
			goto exit;
		}
	}

	if (ioctl_spi_msg.speed_hz != 0) {
		thp_log_debug("%s change to 3.5k-> speed =%d\n",
			__func__, ioctl_spi_msg.speed_hz);
		current_speed = cd->sdev->max_speed_hz;
		cd->sdev->max_speed_hz = ioctl_spi_msg.speed_hz;
		rc = thp_setup_spi(cd);
		if (rc) {
			thp_log_err("%s:set max speed failed rc:%d",
				__func__, rc);
			if (lock_status == NEED_LOCK)
				thp_bus_unlock();
			goto exit;
		}
	}

	rc = thp_spi_sync(cd->sdev, &msg);
	if (rc) {
		thp_log_err("%s:failed sync msg:%d", __func__, rc);
		if (lock_status == NEED_LOCK)
			thp_bus_unlock();
		goto exit;
	}
	if (ioctl_spi_msg.speed_hz != 0) {
		thp_log_debug("%s current_speed-> %d\n",
			__func__, current_speed);
		cd->sdev->max_speed_hz = current_speed;
		rc = thp_setup_spi(cd);
		if (rc) {
			thp_log_err("%s:set max speed failed rc:%d",
				__func__, rc);
			if (lock_status == NEED_LOCK)
				thp_bus_unlock();
			goto exit;
		}
	}
	if (lock_status == NEED_LOCK)
		thp_bus_unlock();

	for (i = 0, msg_len = 0; i < ioctl_spi_msg.xfer_num; i++) {
		if (ioctl_spi_xfer[i].rx) {
			rc = copy_to_user(ioctl_spi_xfer[i].rx,
					rx_buf + msg_len,
					ioctl_spi_xfer[i].len);
			if (rc) {
				thp_log_err("%s:copy to rx buff fail:%d",
					__func__, rc);
				goto exit;
			}
		}
		msg_len += ioctl_spi_xfer[i].len;
	}
exit:
	kfree(ioctl_spi_xfer);
	kfree(xfer);
	kfree(rx_buf);
	kfree(tx_buf);
	return rc;
}

#define MAX_IOCTL_COUNT_IN_SUSPEND 10
static long thp_ioctl_spi_sync(const void __user *data,
	unsigned int lock_status)
{
	struct thp_core_data *cd = thp_get_core_data();
	struct thp_ioctl_spi_sync_data sync_data;
	struct thp_ioctl_spi_sync_data_compat sync_data_compat;
	int rc = 0;
	u8 *tx_buf = NULL;
	u8 *rx_buf = NULL;

	thp_log_debug("%s: called\n", __func__);
	memset(&sync_data, 0, sizeof(sync_data));
	memset(&sync_data_compat, 0, sizeof(sync_data_compat));

	if (cd->suspended && (!cd->need_work_in_suspend)) {
		if (spi_sync_count_in_suspend < MAX_IOCTL_COUNT_IN_SUSPEND)
			thp_log_info("%s suspended return!\n", __func__);
		spi_sync_count_in_suspend++;
		return 0;
	}

	if (!data) {
		thp_log_err("%s: input parameter null\n", __func__);
		return -EINVAL;
	}

#if defined(CONFIG_TEE_TUI)
	if (tui_enable)
		return 0;
#endif

	if (cd->compat_flag == true) {
		if (copy_from_user(&sync_data_compat, data,
			sizeof(sync_data_compat))) {
			thp_log_err("Failed to copy_from_user\n");
			return -EFAULT;
		}
		sync_data.rx = compat_ptr(sync_data_compat.rx);
		sync_data.tx = compat_ptr(sync_data_compat.tx);
		sync_data.size = sync_data_compat.size;
	} else {
		if (copy_from_user(&sync_data, data,
			sizeof(sync_data))) {
			thp_log_err("Failed to copy_from_user\n");
			return -EFAULT;
		}
	}

	if (sync_data.size > THP_SYNC_DATA_MAX) {
		thp_log_err("sync_data.size out of range\n");
		return -EINVAL;
	}

	if (cd->need_huge_memory_in_spi) {
		rc = thp_spi_sync_alloc_mem();
		if (!rc) {
			rx_buf = spi_sync_rx_buf;
			tx_buf = spi_sync_tx_buf;
		} else {
			thp_log_err("%s:buf request memory fail\n", __func__);
			goto exit;
		}
	} else {
		rx_buf = kzalloc(sync_data.size, GFP_KERNEL);
		tx_buf = kzalloc(sync_data.size, GFP_KERNEL);
		if (!rx_buf || !tx_buf) {
			thp_log_err(
				"%s:buf request memory fail,sync_data.size = %d\n",
				__func__, sync_data.size);
			goto exit;
		}
	}
	rc = copy_from_user(tx_buf, sync_data.tx, sync_data.size);
	if (rc) {
		thp_log_err("%s:copy in buff fail\n", __func__);
		goto exit;
	}
	if (lock_status == NEED_LOCK) {
		if (cd->thp_dev->ops->spi_transfer)
			rc = cd->thp_dev->ops->spi_transfer(tx_buf,
				rx_buf, sync_data.size);
		else
			rc = thp_spi_transfer(cd, tx_buf, rx_buf,
				sync_data.size, lock_status);
	} else {
		rc = thp_spi_transfer(cd, tx_buf, rx_buf,
			sync_data.size, lock_status);
	}
	if (rc) {
		thp_log_err("%s: transfer error, ret = %d\n", __func__, rc);
		goto exit;
	}

	if (sync_data.rx) {
		rc = copy_to_user(sync_data.rx, rx_buf, sync_data.size);
		if (rc) {
			thp_log_err("%s:copy out buff fail\n", __func__);
			goto exit;
		}
	}

exit:
	if (!cd->need_huge_memory_in_spi) {
		kfree(rx_buf);
		rx_buf = NULL;

		kfree(tx_buf);
		tx_buf = NULL;
	}
	return rc;
}

static long thp_ioctl_finish_notify(unsigned long arg)
{
	struct thp_core_data *cd = thp_get_core_data();
	unsigned long event_type = arg;
	int rc;
	struct thp_device_ops *ops = cd->thp_dev->ops;

	thp_log_info("%s: called\n", __func__);
	switch (event_type) {
	case THP_AFE_NOTIFY_FW_UPDATE:
		rc = ops->afe_notify ?
			ops->afe_notify(cd->thp_dev, event_type) : 0;
		break;
	default:
		thp_log_err("%s: illegal event type\n", __func__);
		rc = -EINVAL;
	}
	return rc;
}

static long thp_ioctl_get_frame_count(unsigned long arg)
{
	struct thp_core_data *cd = thp_get_core_data();
	u32 __user *frame_cnt = (u32 *)arg;

	if (cd->frame_count)
		thp_log_info("%s:frame_cnt=%d\n", __func__, cd->frame_count);

	if (frame_cnt == NULL) {
		thp_log_err("%s: input parameter null\n", __func__);
		return -EINVAL;
	}

	if (copy_to_user(frame_cnt, &cd->frame_count, sizeof(u32))) {
		thp_log_err("%s:copy frame_cnt failed\n", __func__);
		return -EFAULT;
	}

	return 0;
}

static long thp_ioctl_clear_frame_buffer(void)
{
	struct thp_core_data *cd = thp_get_core_data();

	if (cd->frame_count == 0)
		return 0;

	thp_log_info("%s called\n", __func__);
	mutex_lock(&cd->mutex_frame);
	thp_clear_frame_buffer(cd);
	mutex_unlock(&cd->mutex_frame);
	return 0;
}

static long thp_get_frame_from_thp_queue(
	struct thp_ioctl_get_frame_data *data)
{
	struct thp_core_data *cd = thp_get_core_data();
	struct thp_queue_node *temp_frame = NULL;
	long rc = 0;

	if (data->size > g_thp_queue_node_len) {
		thp_log_debug("%s data size too big, set to %u\n",
			__func__, g_thp_queue_node_len);
		data->size = g_thp_queue_node_len;
	}
	cd->frame_size = data->size;
	mutex_lock(&cd->suspend_flag_mutex);
	if (cd->suspended && (!cd->need_work_in_suspend)) {
		thp_log_info("%s: drv suspended\n", __func__);
		mutex_unlock(&cd->suspend_flag_mutex);
		return -EINVAL;
	}
	thp_set_irq_status(cd, THP_IRQ_ENABLE);
	mutex_unlock(&cd->suspend_flag_mutex);
	if (thp_queue_is_empty(cd->thp_queue) &&
		cd->get_frame_block_flag) {
		if (thp_wait_frame_waitq(cd))
			rc = -ETIMEDOUT;
	}
	mutex_lock(&cd->mutex_frame);
	if (thp_queue_is_empty(cd->thp_queue) == false) {
		temp_frame = thp_queue_get_head(cd->thp_queue);
		if (temp_frame == NULL) {
			thp_log_err("Failed to temp_frame is NULL\n");
			rc = -EFAULT;
			goto out;
		}
		if (data->buf == NULL) {
			thp_log_err("Failed to data buf is NULL\n");
			rc = -EFAULT;
			goto out;
		}
		if (copy_to_user(data->buf, temp_frame->buf,
			cd->frame_size)) {
			thp_log_err("Failed to copy_to_user\n");
			rc = -EFAULT;
			goto out;
		}
		if (data->tv == NULL) {
			thp_log_err("Failed to data tv is NULL\n");
			rc = -EFAULT;
			goto out;
		}
		if (copy_to_user(data->tv, &(temp_frame->tv),
			sizeof(temp_frame->tv))) {
			thp_log_err("Failed to copy_to_user tv\n");
			rc = -EFAULT;
			goto out;
		}
		if (!thp_queue_dequeue(cd->thp_queue))
			thp_log_err("%s queue remove failed\n", __func__);
		temp_frame = NULL;
		cd->frame_count--;
		rc = 0;
	} else {
		thp_log_err("%s:no frame\n", __func__);
		/*
		 * When wait timeout, try to get data.
		 * If timeout and no data, return -ETIMEDOUT
		 */
		if (rc != -ETIMEDOUT)
			rc = -ENODATA;
	}
out:
	mutex_unlock(&cd->mutex_frame);
	trace_touch(TOUCH_TRACE_GET_FRAME, TOUCH_TRACE_FUNC_OUT,
		rc ? "no frame" : "with frame");
	return rc;
}

static long thp_ioctl_get_frame(unsigned long arg, unsigned int f_flag)
{
	struct thp_ioctl_get_frame_data data;
	struct thp_ioctl_get_frame_data_compat data_compat;
	struct thp_core_data *cd = thp_get_core_data();
	void __user *argp = (void __user *)arg;
	long rc = 0;

	memset(&data, 0, sizeof(data));
	memset(&data_compat, 0, sizeof(data_compat));
	trace_touch(TOUCH_TRACE_GET_FRAME, TOUCH_TRACE_FUNC_IN, "thp");

	if (!arg) {
		thp_log_err("%s: input parameter null\n", __func__);
		return -EINVAL;
	}

	if (cd->suspended && (!cd->need_work_in_suspend)) {
		if (get_frame_count_in_suspend < MAX_IOCTL_COUNT_IN_SUSPEND)
			thp_log_info("%s: drv suspended\n", __func__);
		get_frame_count_in_suspend++;
		return -ETIMEDOUT;
	}

	if (cd->compat_flag == true) {
		if (copy_from_user(&data_compat, argp,
			sizeof(data_compat))) {
			thp_log_err("Failed to copy_from_user\n");
			return -EFAULT;
		}
		data.buf = compat_ptr(data_compat.buf);
		data.tv = compat_ptr(data_compat.tv);
		data.size = data_compat.size;
	} else {
		if (copy_from_user(&data, argp,
			sizeof(data))) {
			thp_log_err("Failed to copy_from_user\n");
			return -EFAULT;
		}
	}

	if (data.buf == 0 || data.size == 0 ||
		data.size > THP_MAX_FRAME_SIZE || data.tv == 0) {
		thp_log_err("%s:input buf invalid\n", __func__);
		return -EINVAL;
	}
	if (cd->use_thp_queue)
		return thp_get_frame_from_thp_queue(&data);

	cd->frame_size = data.size;
	mutex_lock(&cd->suspend_flag_mutex);
	if (cd->suspended && (!cd->need_work_in_suspend)) {
		thp_log_info("%s: drv suspended\n", __func__);
		mutex_unlock(&cd->suspend_flag_mutex);
		return -EINVAL;
	}
	thp_set_irq_status(cd, THP_IRQ_ENABLE);
	mutex_unlock(&cd->suspend_flag_mutex);
	if (list_empty(&cd->frame_list.list) && cd->get_frame_block_flag) {
		if (thp_wait_frame_waitq(cd))
			rc = -ETIMEDOUT;
	}

	mutex_lock(&cd->mutex_frame);
	if (!list_empty(&cd->frame_list.list)) {
		struct thp_frame *temp;

		temp = list_first_entry(&cd->frame_list.list,
				struct thp_frame, list);
		if (data.buf == NULL) {
			thp_log_err("Failed to copy_to_user()\n");
			rc = -EFAULT;
			goto out;
		}
		if (copy_to_user(data.buf, temp->frame, cd->frame_size)) {
			thp_log_err("Failed to copy_to_user()\n");
			rc = -EFAULT;
			goto out;
		}

		if (data.tv == NULL) {
			thp_log_err("Failed to copy_to_user()\n");
			rc = -EFAULT;
			goto out;
		}
		if (copy_to_user(data.tv, &(temp->tv),
					sizeof(struct timeval))) {
			thp_log_err("Failed to copy_to_user()\n");
			rc = -EFAULT;
			goto out;
		}

		list_del(&temp->list);
		kfree(temp);
		cd->frame_count--;
		rc = 0;
	} else {
		thp_log_err("%s:no frame\n", __func__);
		/*
		 * When wait timeout, try to get data.
		 * If timeout and no data, return -ETIMEDOUT
		 */
		if (rc != -ETIMEDOUT)
			rc = -ENODATA;
	}
out:
	mutex_unlock(&cd->mutex_frame);
	trace_touch(TOUCH_TRACE_GET_FRAME, TOUCH_TRACE_FUNC_OUT,
		rc ? "no frame" : "with frame");
	return rc;
}

static long thp_ioctl_reset(unsigned long reset)
{
	struct thp_core_data *cd = thp_get_core_data();

	thp_log_info("%s:set reset status %lu\n", __func__, reset);

#ifndef CONFIG_HUAWEI_THP_MTK
	gpio_set_value(cd->gpios.rst_gpio, !!reset);
#else
	if (cd->support_pinctrl == 0) {
		thp_log_info("%s: not support pinctrl\n", __func__);
		return 0;
	}

	if (!!reset)
		pinctrl_select_state(cd->pctrl, cd->mtk_pinctrl.reset_high);
	else
		pinctrl_select_state(cd->pctrl, cd->mtk_pinctrl.reset_low);
#endif

	if ((!!reset) && (cd->send_bt_status_to_fw) &&
		(!cd->enter_stylus3_mmi_test) &&
		(cd->thp_dev->ops->bt_handler)) {
		if (cd->thp_dev->ops->bt_handler(cd->thp_dev, true))
			thp_log_err("ioctl reset send bt status fail\n");
	}
	cd->frame_waitq_flag = WAITQ_WAIT;
	cd->reset_flag = !reset;

	return 0;
}

static long thp_ioctl_hw_lock_status(unsigned long arg)
{
	thp_log_info("%s: set hw lock status %lu\n", __func__, arg);
	if (arg) {
		if (thp_bus_lock()) {
			thp_log_err("%s: get lock failed\n", __func__);
			return -ETIME;
		}
		locked_by_daemon = true;
	} else {
		thp_bus_unlock();
		locked_by_daemon = false;
	}
	return 0;
}

static long thp_ioctl_set_timeout(unsigned long arg)
{
	struct thp_core_data *ts = thp_get_core_data();
	unsigned int timeout_ms = min_t(unsigned int, arg, THP_WAIT_MAX_TIME);

	thp_log_info("set wait time %d ms.(current %dms)\n",
			timeout_ms, ts->timeout);

	if (timeout_ms != ts->timeout) {
		ts->timeout = timeout_ms;
		thp_wake_up_frame_waitq(ts);
	}

	return 0;
}

static long thp_ioctl_set_block(unsigned long arg)
{
	struct thp_core_data *ts = thp_get_core_data();
	unsigned int block_flag = arg;

	if (block_flag)
		ts->get_frame_block_flag = THP_GET_FRAME_BLOCK;
	else
		ts->get_frame_block_flag = THP_GET_FRAME_NONBLOCK;

	thp_log_info("%s:set block %d\n", __func__, block_flag);

	thp_wake_up_frame_waitq(ts);
	return 0;
}


static long thp_ioctl_set_irq(unsigned long arg)
{
	struct thp_core_data *ts = thp_get_core_data();
	unsigned int irq_en = (unsigned int)arg;

	mutex_lock(&ts->suspend_flag_mutex);
	if (ts->suspended && (!ts->need_work_in_suspend)) {
		thp_log_info("%s: drv suspended\n", __func__);
		mutex_unlock(&ts->suspend_flag_mutex);
		return -EINVAL;
	}
	thp_set_irq_status(ts, irq_en);
	mutex_unlock(&ts->suspend_flag_mutex);
	return 0;
}

static long thp_ioctl_get_irq_gpio_value(unsigned long arg)
{
	struct thp_core_data *cd = thp_get_core_data();
	u32 __user *out_value = (u32 *)arg;
	u32 value;

	value = gpio_get_value(cd->gpios.irq_gpio);
	if (copy_to_user(out_value, &value, sizeof(u32))) {
		thp_log_err("%s:copy value fail\n", __func__);
		return -EFAULT;
	}
	return 0;
}

static long thp_ioctl_set_spi_speed(unsigned long arg)
{
	struct thp_core_data *cd = thp_get_core_data();
	unsigned long max_speed_hz = arg;
	int rc;

	if ((max_speed_hz == cd->sdev->max_speed_hz) || cd->spi_speed_mode)
		return 0;
	rc = thp_set_spi_max_speed(max_speed_hz);
	if (rc)
		thp_log_err("%s: failed to set spi speed\n", __func__);
	return rc;
}

static long thp_ioctl_spi_sync_ssl_bl(const void __user *data)
{
	struct thp_ioctl_spi_sync_data sync_data;
	struct thp_core_data *cd = NULL;
	int rc = 0;
	u8 *tx_buf = NULL;
	u8 *rx_buf = NULL;

	memset(&sync_data, 0, sizeof(sync_data));
	thp_log_debug("%s: called\n", __func__);

	if (!data) {
		thp_log_err("%s: data null\n", __func__);
		return -EINVAL;
	}
	cd = thp_get_core_data();
	if (cd == NULL) {
		thp_log_err("%s: thp get core data err\n", __func__);
		return -EINVAL;
	}
	if (cd->suspended)
		return 0;

#if defined(CONFIG_TEE_TUI)
	if (tui_enable)
		return 0;
#endif

	if (copy_from_user(&sync_data, data,
				sizeof(struct thp_ioctl_spi_sync_data))) {
		thp_log_err("Failed to copy_from_user()\n");
		return -EFAULT;
	}

	if (sync_data.size > THP_SYNC_DATA_MAX || 0 == sync_data.size) {
		thp_log_err("sync_data.size out of range\n");
		return -EINVAL;
	}

	rx_buf = kzalloc(sync_data.size, GFP_KERNEL);
	tx_buf = kzalloc(sync_data.size, GFP_KERNEL);
	if (!rx_buf || !tx_buf) {
		thp_log_err("%s:buf request memory fail,sync_data.size = %d\n",
			__func__, sync_data.size);
		goto exit;
	}

	if (sync_data.tx) {
		rc = copy_from_user(tx_buf, sync_data.tx, sync_data.size);
		if (rc) {
			thp_log_err("%s:copy in buff fail\n", __func__);
			goto exit;
		}
	}

	if (cd) {
		if (cd->thp_dev->ops->spi_transfer_one_byte_bootloader)
			rc = cd->thp_dev->ops->spi_transfer_one_byte_bootloader(
				cd->thp_dev, tx_buf, rx_buf, sync_data.size);
		else
			rc = -EINVAL;
		if (rc) {
			thp_log_err("%s: transfer error, ret = %d\n",
				__func__, rc);
			goto exit;
		}
	}

	if (sync_data.rx) {
		rc = copy_to_user(sync_data.rx, rx_buf, sync_data.size);
		if (rc) {
			thp_log_err("%s:copy out buff fail\n", __func__);
			goto exit;
		}
	}

exit:

	kfree(rx_buf);
	kfree(tx_buf);

	return rc;
}

static void thp_wakeup_screenon_waitq(struct thp_core_data *cd)
{
	if (!cd) {
		thp_log_err("%s: cd is null\n", __func__);
		return;
	}

	if (atomic_read(&(cd->afe_screen_on_waitq_flag)) != WAITQ_WAKEUP) {
		atomic_set(&(cd->afe_screen_on_waitq_flag), WAITQ_WAKEUP);
		wake_up_interruptible(&(cd->afe_screen_on_waitq));
	} else {
		thp_log_info("afe set status screen on have done\n");
	}
}

static long thp_ioctl_set_afe_status(const void __user *data)
{
	int rc;
	struct thp_ioctl_set_afe_status set_afe_status;
	struct thp_core_data *cd = NULL;
	struct thp_device *tdev = NULL;

	thp_log_info("%s: called\n", __func__);
	if (!data) {
		thp_log_err("%s: data null\n", __func__);
		return -EINVAL;
	}
	cd = thp_get_core_data();
	tdev = cd->thp_dev;
	if (copy_from_user(&set_afe_status, data,
				sizeof(struct thp_ioctl_set_afe_status))) {
		thp_log_err("Failed to copy_from_user()\n");
		return -EFAULT;
	}
	thp_log_info("%s ->%d,%d,%d\n", __func__, set_afe_status.type,
		set_afe_status.status, set_afe_status.parameter);

	switch (set_afe_status.type) {
	case THP_AFE_STATUS_FW_UPDATE:
		if (cd->thp_dev->ops->set_fw_update_mode != NULL)
			rc = cd->thp_dev->ops->set_fw_update_mode(cd->thp_dev,
				set_afe_status);
		else
			rc = -EINVAL;
		if (!rc) {
			if (set_afe_status.parameter == NORMAL_WORK_MODE) {
				is_fw_update = 0;
			} else if (set_afe_status.parameter ==
						FW_UPDATE_MODE) {
				is_fw_update = 1;
			} else {
				thp_log_err("%s->error mode\n",
					__func__);
				rc = -EINVAL;
			}
			thp_log_info("%s call is_fw_updating=%d\n",
				__func__, is_fw_update);
		}
		break;
	case THP_AFE_STATUS_SCREEN_ON:
		if (cd->wait_afe_screen_on_support)
			thp_wakeup_screenon_waitq(cd);
		if (cd->lcd_need_get_afe_status &&
			(set_afe_status.status == 1)) {
			do_gettimeofday(&cd->afe_status_record_tv);
			cd->afe_download_status = true;
		}

		if ((cd->send_bt_status_to_fw) &&
			(cd->thp_dev->ops->bt_handler)) {
			if (cd->thp_dev->ops->bt_handler(cd->thp_dev, false))
				thp_log_err("send bt status to fw fail\n");
		}
		break;
	default:
		thp_log_err("%s: illegal type\n", __func__);
		rc = -EINVAL;
		break;
	}
	return rc;
}

static long thp_ioctl_get_work_status(unsigned long arg)
{
	struct thp_core_data *cd = thp_get_core_data();
	u32 __user *work_status = (u32 *)(uintptr_t)arg;
	u32 status;

	if (cd == NULL) {
		thp_log_err("%s: thp cord data null\n", __func__);
		return -EINVAL;
	}
	if (work_status == NULL) {
		thp_log_err("%s: input parameter null\n", __func__);
		return -EINVAL;
	}
	if (cd->suspended == true)
		status = THP_WORK_STATUS_SUSPENDED;
	else
		status = THP_WORK_STATUS_RESUMED;

	if (copy_to_user(work_status, &status, sizeof(u32))) {
		thp_log_err("%s:get wort_status failed\n", __func__);
		return -EFAULT;
	}

	return 0;
}

static long thp_ioctl(struct file *filp, unsigned int cmd,
	unsigned long arg)
{
	long ret;

	switch (cmd) {
	case THP_IOCTL_CMD_GET_FRAME_COMPAT:
	case THP_IOCTL_CMD_GET_FRAME:
		ret = thp_ioctl_get_frame(arg, filp->f_flags);
		break;

	case THP_IOCTL_CMD_RESET:
		ret = thp_ioctl_reset(arg);
		break;

	case THP_IOCTL_CMD_SET_TIMEOUT:
		ret = thp_ioctl_set_timeout(arg);
		break;

	case THP_IOCTL_CMD_SPI_SYNC_COMPAT:
	case THP_IOCTL_CMD_SPI_SYNC:
		ret = thp_ioctl_spi_sync((void __user *)arg, NEED_LOCK);
		break;

	case THP_IOCTL_CMD_FINISH_NOTIFY:
		ret = thp_ioctl_finish_notify(arg);
		break;
	case THP_IOCTL_CMD_SET_BLOCK:
		ret = thp_ioctl_set_block(arg);
		break;

	case THP_IOCTL_CMD_SET_IRQ:
		ret = thp_ioctl_set_irq(arg);
		break;

	case THP_IOCTL_CMD_GET_FRAME_COUNT:
		ret = thp_ioctl_get_frame_count(arg);
		break;
	case THP_IOCTL_CMD_CLEAR_FRAME_BUFFER:
		ret = thp_ioctl_clear_frame_buffer();
		break;

	case THP_IOCTL_CMD_GET_IRQ_GPIO_VALUE:
		ret = thp_ioctl_get_irq_gpio_value(arg);
		break;

	case THP_IOCTL_CMD_SET_SPI_SPEED:
		ret = thp_ioctl_set_spi_speed(arg);
		break;
	case THP_IOCTL_CMD_SPI_SYNC_SSL_BL:
		ret = thp_ioctl_spi_sync_ssl_bl((void __user *) arg);
		break;
	case THP_IOCTL_CMD_SET_AFE_STATUS:
		ret = thp_ioctl_set_afe_status((void __user *)arg);
		break;
	case THP_IOCTL_CMD_MUILTIPLE_SPI_XFRE_SYNC:
		ret = thp_ioctl_multiple_spi_xfer_sync((void __user *)arg,
			NEED_LOCK);
		break;
	case THP_IOCTL_CMD_HW_LOCK:
		ret = thp_ioctl_hw_lock_status(arg);
		break;
	case THP_IOCTL_CMD_SPI_SYNC_NO_LOCK:
		ret = thp_ioctl_spi_sync((void __user *)arg, DONOT_NEED_LOCK);
		break;
	case THP_IOCTL_CMD_MUILTIPLE_SPI_XFRE_SYNC_NO_LOCK:
		ret = thp_ioctl_multiple_spi_xfer_sync((void __user *)arg,
			DONOT_NEED_LOCK);
		break;
	case THP_IOCTL_CMD_GET_WORK_STATUS:
		ret = thp_ioctl_get_work_status(arg);
		break;
	default:
		thp_log_err("%s: cmd unknown, cmd = 0x%x\n", __func__, cmd);
		ret = 0;
	}

	return ret;
}

static long thp_compat_ioctl(struct file *filp, unsigned int cmd,
	unsigned long arg)
{
	long ret;
	struct thp_core_data *cd = thp_get_core_data();

	cd->compat_flag = true;
	ret = thp_ioctl(filp, cmd, arg);
	if (ret)
		thp_log_err("%s: ioctl err %ld\n", __func__, ret);

	return ret;
}

static const struct file_operations g_thp_fops = {
	.owner = THIS_MODULE,
	.open = thp_open,
	.release = thp_release,
	.unlocked_ioctl = thp_ioctl,
	.compat_ioctl = thp_compat_ioctl,
};

static struct miscdevice g_thp_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = THP_MISC_DEVICE_NAME,
	.fops = &g_thp_fops,
};

#ifdef CONFIG_HUAWEI_THP_MTK
static void send_event_to_fingerprint_ud(struct thp_udfp_data udfp_data)
{
	struct ud_fp_ops *fp_ops = NULL;

	fp_ops = fp_get_ops();
	if (!fp_ops || !fp_ops->fp_irq_notify) {
		thp_log_err("%s: point is NULL!!\n", __func__);
		return;
	}
	if (fp_ops->fp_irq_notify(&udfp_data.tpud_data))
		thp_log_err("%s: tpud event notify fp failed\n", __func__);
}

static void tp_event_dispatch(struct thp_udfp_data udfp_data)
{
	thp_log_info("%s: fp_event = %d, aod_event = %u, keyevent = %u\n",
		__func__, udfp_data.tpud_data.udfp_event, udfp_data.aod_event,
		udfp_data.key_event);
	if (udfp_data.tpud_data.udfp_event != TP_FP_EVENT_MAX) {
		send_event_to_fingerprint_ud(udfp_data);
		thp_udfp_event_to_aod(udfp_data);
	}
	if (udfp_data.aod_event == AOD_VALID_EVENT)
		thp_aod_click_report(udfp_data);
	if (udfp_data.key_event == TS_DOUBLE_CLICK)
		thp_inputkey_report(udfp_data.key_event);
}

static void gesture_event_process(struct thp_core_data *cd)
{
	int rc;
	struct thp_udfp_data udfp_data;

	memset(&udfp_data, 0, sizeof(udfp_data));
	if (!cd->thp_dev->ops->get_event_info) {
		thp_log_err("%s: ops is NULL\n", __func__);
		return;
	}
	rc = cd->thp_dev->ops->get_event_info(cd->thp_dev, &udfp_data);
	if (rc) {
		thp_log_err("%s: get event info fail, ret = %d\n",
			__func__, rc);
		return;
	}
	tp_event_dispatch(udfp_data);
}
#endif

static void thp_copy_frame_to_thp_queue(struct thp_core_data *cd)
{
	static int pre_frame_count = -1;

	mutex_lock(&(cd->mutex_frame));
	/* check for max limit */
	if (cd->frame_count >= THP_LIST_MAX_FRAMES) {
		if (cd->frame_count != pre_frame_count)
			thp_log_err("frame_lite buf full start,frame_count:%d\n",
				cd->frame_count);

		if (!thp_queue_dequeue(cd->thp_queue))
			thp_log_err("%s queue remove failed\n", __func__);
		pre_frame_count = cd->frame_count;
		cd->frame_count--;
	} else if (pre_frame_count >= THP_LIST_MAX_FRAMES) {
		thp_log_err(
			"%s:frame buf full exception restored,frame_count:%d\n",
			__func__, cd->frame_count);
		pre_frame_count = cd->frame_count;
	}

	if (!thp_queue_enqueue(cd->thp_queue, cd->frame_read_buf,
		cd->frame_size))
		thp_log_err("%s queue insert failed\n", __func__);
	cd->frame_count++;
	mutex_unlock(&(cd->mutex_frame));
}

static void thp_copy_frame(struct thp_core_data *cd)
{
	struct thp_frame *temp = NULL;
	static int pre_frame_count = -1;
	unsigned long len;

	mutex_lock(&(cd->mutex_frame));

	/* check for max limit */
	if (cd->frame_count >= THP_LIST_MAX_FRAMES) {
		if (cd->frame_count != pre_frame_count)
			thp_log_err("frame buf full start,frame_count:%d\n",
				cd->frame_count);

		temp = list_first_entry(&cd->frame_list.list,
				struct thp_frame, list);
		list_del(&temp->list);
		kfree(temp);
		pre_frame_count = cd->frame_count;
		cd->frame_count--;
	} else if (pre_frame_count >= THP_LIST_MAX_FRAMES) {
		thp_log_err(
			"%s:frame buf full exception restored,frame_count:%d\n",
			__func__, cd->frame_count);
		pre_frame_count = cd->frame_count;
	}

	temp = kzalloc(sizeof(struct thp_frame), GFP_KERNEL);
	if (!temp) {
		thp_log_err("%s:memory out\n", __func__);
		mutex_unlock(&(cd->mutex_frame));
		return;
	}

	if (cd->frame_size > sizeof(temp->frame)) {
		thp_log_err("%s: frame size is too large: %u\n",
			__func__, cd->frame_size);
		len = sizeof(temp->frame);
	} else {
		len = cd->frame_size;
	}
	memcpy(temp->frame, cd->frame_read_buf, len);
	do_gettimeofday(&(temp->tv));
	list_add_tail(&(temp->list), &(cd->frame_list.list));
	cd->frame_count++;
	mutex_unlock(&(cd->mutex_frame));
}

/*
 * disable the interrupt if the interrupt is enabled,
 * which is only used in irq handler
 */
static void thp_disable_irq_in_irq_process(void)
{
	struct thp_core_data *cd = thp_get_core_data();
	int ret;

	/*
	 * Use mutex_trylock to avoid the irq process requesting lock failure,
	 * thus solving the problem that other process calls disable_irq
	 * process is blocked.
	 */
	ret = mutex_trylock(&cd->irq_mutex);
	if (ret) {
		if (cd->irq_enabled) {
			disable_irq_nosync(cd->irq);
			cd->irq_enabled = 0;
			thp_log_info("%s:disable irq to protect irq storm\n",
				__func__);
		}
		mutex_unlock(&cd->irq_mutex);
		return;
	}
	thp_log_info("%s:failed to try lock, only need ignore it\n", __func__);
}

static void protect_for_irq_storm(void)
{
	struct thp_core_data *cd = thp_get_core_data();
	struct timeval end_time;
	static struct timeval irq_storm_start_time;
	long delta_time;

	/*
	 * We should not try to disable irq when we
	 * need to handle the irq in screen off state
	 */
	if (need_work_in_suspend_switch(cd)) {
		thp_log_err("%s:ignore the irq\n", __func__);
		return;
	}
	if (cd->invalid_irq_num == 0) {
		do_gettimeofday(&irq_storm_start_time);
		return;
	}
	if (cd->invalid_irq_num == MAX_INVALID_IRQ_NUM) {
		memset(&end_time, 0, sizeof(end_time));
		cd->invalid_irq_num = 0;
		do_gettimeofday(&end_time);
		/* multiply 1000000 to transfor second to us */
		delta_time = ((end_time.tv_sec - irq_storm_start_time.tv_sec) *
			1000000) + end_time.tv_usec -
			irq_storm_start_time.tv_usec;
		/* divide 1000 to transfor sec to us to ms */
		delta_time /= 1000;
		thp_log_info("%s:delta_time = %ld ms\n", __func__, delta_time);
		if (delta_time <= (MAX_INVALID_IRQ_NUM / THP_IRQ_STORM_FREQ))
			thp_disable_irq_in_irq_process();
	}
}

static void thp_irq_thread_suspend_process(struct thp_core_data *cd)
{
	int rc;
	unsigned int gesture_wakeup_value = 0;
	if (need_work_in_suspend_switch(cd) &&
		(cd->work_status != RESUME_DONE)) {
		__pm_wakeup_event(&cd->thp_wake_lock, jiffies_to_msecs(HZ));
		disable_irq_nosync(cd->irq);
		thp_log_info("%s:ts gesture mode irq\n", __func__);
#ifdef CONFIG_HUAWEI_THP_MTK
		if (cd->use_ap_gesture) {
			/* This function includes tpud, AOD click, doubletap */
			gesture_event_process(cd);
			goto exit;
		}
#endif
		if (cd->thp_dev->ops->chip_gesture_report) {
			rc = cd->thp_dev->ops->chip_gesture_report(cd->thp_dev,
				&gesture_wakeup_value);
			if (rc) {
				thp_log_err(
					"%s:gesture report failed this irq,rc = %d\n",
					__func__, rc);
				goto exit;
			}
		} else {
			thp_log_err("%s:gesture not support\n", __func__);
			goto exit;
		}
		thp_inputkey_report(gesture_wakeup_value);
		goto exit;
	}

	if (cd->invalid_irq_num < MAX_PRINT_LOG_INVALID_IRQ_NUM)
		thp_log_err("%s: ignore this irq\n", __func__);
	protect_for_irq_storm();
	cd->invalid_irq_num++;
	return;
exit:
	enable_irq(cd->irq);
	trace_touch(TOUCH_TRACE_IRQ_BOTTOM, TOUCH_TRACE_FUNC_OUT, "thp");
}

static irqreturn_t thp_irq_thread(int irq, void *dev_id)
{
	struct thp_core_data *cd = dev_id;
	u8 *read_buf = (u8 *)cd->frame_read_buf;
	int rc;

	trace_touch(TOUCH_TRACE_IRQ_BOTTOM, TOUCH_TRACE_FUNC_IN, "thp");
	if (cd->reset_flag) {
		thp_log_err("%s:reset state, ignore this irq\n", __func__);
		return IRQ_HANDLED;
	}

#if defined(CONFIG_TEE_TUI)
	if (tui_enable) {
		thp_log_err("%s:tui_mode, disable irq\n", __func__);
		thp_disable_irq_in_irq_process();
		return IRQ_HANDLED;
	}
#endif

	if (cd->suspended && (!cd->need_work_in_suspend)) {
		thp_irq_thread_suspend_process(cd);
		return IRQ_HANDLED;
	}

	disable_irq_nosync(cd->irq);

	/* get frame */
	rc = cd->thp_dev->ops->get_frame(cd->thp_dev, read_buf, cd->frame_size);
	if (rc) {
		thp_log_err("%s: failed to read frame %d\n", __func__, rc);
		goto exit;
	}

	trace_touch(TOUCH_TRACE_DATA2ALGO, TOUCH_TRACE_FUNC_IN, "thp");
	if (cd->use_thp_queue)
		thp_copy_frame_to_thp_queue(cd);
	else
		thp_copy_frame(cd);
	thp_wake_up_frame_waitq(cd);
	trace_touch(TOUCH_TRACE_DATA2ALGO, TOUCH_TRACE_FUNC_OUT, "thp");

exit:
	enable_irq(cd->irq);
	trace_touch(TOUCH_TRACE_IRQ_BOTTOM, TOUCH_TRACE_FUNC_OUT, "thp");
	return IRQ_HANDLED;
}

void thp_spi_cs_set(u32 control)
{
	int rc = 0;
	struct thp_core_data *cd = thp_get_core_data();
	struct thp_timing_config *timing = NULL;

	if (!cd || !cd->thp_dev) {
		thp_log_err("%s:no driver data", __func__);
		return;
	}

	timing = &cd->thp_dev->timing_config;
#ifndef CONFIG_HUAWEI_THP_MTK
	if (control == SSP_CHIP_SELECT) {
		rc = gpio_direction_output(cd->gpios.cs_gpio, control);
		ndelay(timing->spi_sync_cs_low_delay_ns);
		if (timing->spi_sync_cs_low_delay_us && (!is_fw_update))
			udelay(timing->spi_sync_cs_low_delay_us);
	} else {
		rc = gpio_direction_output(cd->gpios.cs_gpio, control);
		ndelay(timing->spi_sync_cs_hi_delay_ns);
	}
#else
	if (cd->support_pinctrl == 0) {
		thp_log_info("%s: not support pinctrl\n", __func__);
		return;
	}

	if (control == SSP_CHIP_SELECT) {
		pinctrl_select_state(cd->pctrl, cd->mtk_pinctrl.cs_low);
		ndelay(cd->thp_dev->timing_config.spi_sync_cs_low_delay_ns);
	} else {
		pinctrl_select_state(cd->pctrl, cd->mtk_pinctrl.cs_high);
		ndelay(cd->thp_dev->timing_config.spi_sync_cs_hi_delay_ns);
	}
#endif

	if (rc < 0)
		thp_log_err("%s:fail to set gpio cs", __func__);
}
EXPORT_SYMBOL(thp_spi_cs_set);
#ifdef CONFIG_BCI_BATTERY
static int thp_charger_detect_notifier_callback(struct notifier_block *self,
					unsigned long event, void *data)
{
	int charger_stat_before = thp_get_status(THP_STATUS_CHARGER);
	int charger_state_new = charger_stat_before;

	thp_log_debug("%s called, charger type: %lu\n", __func__, event);

	switch (event) {
	case VCHRG_START_USB_CHARGING_EVENT:
	case VCHRG_START_AC_CHARGING_EVENT:
	case VCHRG_START_CHARGING_EVENT:
		charger_state_new = 1;
		break;
	case VCHRG_STOP_CHARGING_EVENT:
		charger_state_new = 0;
		break;
	default:
		break;
	}

	if (charger_stat_before != charger_state_new) {
		thp_log_info("%s, set new status: %d\n",
			__func__, charger_state_new);
		thp_set_status(THP_STATUS_CHARGER, charger_state_new);
	}

	return 0;
}
#endif

#define THP_PROJECTID_LEN 9
#define THP_PROJECTID_PRODUCT_NAME_LEN 4
#define THP_PROJECTID_IC_NAME_LEN 2
#define THP_PROJECTID_VENDOR_NAME_LEN 3

struct thp_vendor {
	char *vendor_id;
	char *vendor_name;
};

struct thp_ic_name {
	char *ic_id;
	char *ic_name;
};

static struct thp_vendor thp_vendor_table[] = {
	{ "000", "ofilm" },
	{ "030", "mutto" },
	{ "080", "jdi" },
	{ "090", "samsung" },
	{ "100", "lg" },
	{ "101", "lg" },
	{ "102", "lg" },
	{ "110", "tianma" },
	{ "111", "tianma" },
	{ "112", "tianma" },
	{ "113", "tianma" },
	{ "120", "cmi" },
	{ "130", "boe" },
	{ "140", "ctc" },
	{ "160", "sharp" },
	{ "170", "auo" },
	{ "250", "txd" },
	{ "270", "tcl" },
};

static struct thp_ic_name thp_ic_table[] = {
	{ "32", "rohm" },
	{ "47", "rohm" },
	{ "49", "novatech" },
	{ "59", "himax" },
	{ "60", "himax" },
	{ "61", "himax" },
	{ "62", "synaptics" },
	{ "65", "novatech" },
	{ "66", "himax" },
	{ "68", "focaltech" },
	{ "69", "synaptics" },
	{ "71", "novatech" },
	{ "77", "novatech" },
	{ "78", "goodix" },
	{ "79", "ilitek" },
	{ "86", "synaptics" },
	{ "88", "novatech" },
	{ "91", "synaptics" },
	{ "96", "synaptics" },
	{ "9A", "focaltech" },
	{ "9B", "synaptics" },
	{ "9C", "novatech" },
	{ "9D", "focaltech" },
	{ "9I", "novatech" },
	{ "9J", "novatech" },
	{ "9L", "himax" },
	{ "9M", "novatech" },
	{ "9N", "focaltech" },
	{ "9Y", "novatech" },
	{ "A5", "synaptics" },
	{ "A7", "novatech" },
	{ "A8", "synaptics" },
	{ "A9", "focaltech" },
	{ "A6", "ilitek" },
};

static int thp_projectid_to_vender_name(const char *project_id,
	char **vendor_name, int project_id_len)
{
	char temp_buf[THP_PROJECTID_LEN + 1] = {'0'};
	int i;

	if (strlen(project_id) > project_id_len)
		thp_log_err("%s:project_id has a wrong length\n", __func__);
	strncpy(temp_buf, project_id + THP_PROJECTID_PRODUCT_NAME_LEN +
		THP_PROJECTID_IC_NAME_LEN, THP_PROJECTID_VENDOR_NAME_LEN);

	for (i = 0; i < ARRAY_SIZE(thp_vendor_table); i++) {
		if (!strncmp(thp_vendor_table[i].vendor_id, temp_buf,
			strlen(thp_vendor_table[i].vendor_id))) {
			*vendor_name = thp_vendor_table[i].vendor_name;
			return 0;
		}
	}

	return -ENODATA;
}

static int thp_projectid_to_ic_name(const char *project_id,
	char **ic, int project_id_len)
{
	char temp_buf[THP_PROJECTID_LEN + 1] = { '0' };
	int i;

	if (strlen(project_id) > project_id_len)
		thp_log_err("%s:project_id has a wrong length\n", __func__);
	strncpy(temp_buf, project_id + THP_PROJECTID_PRODUCT_NAME_LEN,
			THP_PROJECTID_IC_NAME_LEN);

	for (i = 0; i < ARRAY_SIZE(thp_ic_table); i++) {
		if (!strncmp(thp_ic_table[i].ic_id, temp_buf,
			strlen(thp_ic_table[i].ic_id))) {
			*ic = thp_ic_table[i].ic_name;
			return 0;
		}
	}

	return -ENODATA;
}

static int thp_init_chip_info(struct thp_core_data *cd)
{
	int rc = 0;
#if defined(CONFIG_LCD_KIT_DRIVER)
	struct lcd_kit_ops *tp_ops = lcd_kit_get_ops();
#endif

	if (cd->is_udp) {
#if (!defined CONFIG_HUAWEI_THP_QCOM) && (!defined CONFIG_HUAWEI_THP_MTK)
		rc = hostprocessing_get_project_id_for_udp(cd->project_id,
			THP_PROJECT_ID_LEN + 1);
#endif
	} else {
#ifndef CONFIG_LCD_KIT_DRIVER
		rc = hostprocessing_get_project_id(cd->project_id,
			THP_PROJECT_ID_LEN + 1);
#else
		if (tp_ops && tp_ops->get_project_id) {
			rc = tp_ops->get_project_id(cd->project_id);
		} else {
			rc = -EINVAL;
			thp_log_err("%s:get lcd_kit_get_ops fail\n", __func__);
		}
#endif
	}
	if (rc)
		thp_log_err("%s:get project id form LCD fail\n", __func__);
	else
		thp_log_info("%s:project id :%s\n", __func__, cd->project_id);

	cd->project_id[THP_PROJECT_ID_LEN] = '\0';

	rc = thp_projectid_to_vender_name(cd->project_id,
			(char **)&cd->vendor_name, THP_PROJECT_ID_LEN + 1);
	if (rc)
		thp_log_info("%s:vendor name parse fail\n", __func__);

	rc = thp_projectid_to_ic_name(cd->project_id,
			(char **)&cd->ic_name, THP_PROJECT_ID_LEN + 1);
	if (rc)
		thp_log_info("%s:ic name parse fail\n", __func__);
	return rc;
}

#ifdef CONFIG_HUAWEI_THP_MTK
static int thp_mtk_pinctrl_get_init(struct thp_device *tdev)
{
	int ret;
	struct thp_core_data *cd = tdev->thp_core;

	cd->mtk_pinctrl.reset_high =
		pinctrl_lookup_state(cd->pctrl, PINCTRL_STATE_RESET_HIGH);
	if (IS_ERR_OR_NULL(cd->mtk_pinctrl.reset_high)) {
		thp_log_err("Can not lookup %s pinstate\n",
			PINCTRL_STATE_RESET_HIGH);
		ret = -EINVAL;
		return ret;
	}

	cd->mtk_pinctrl.reset_low =
		pinctrl_lookup_state(cd->pctrl, PINCTRL_STATE_RESET_LOW);
	if (IS_ERR_OR_NULL(cd->mtk_pinctrl.reset_low)) {
		thp_log_err("Can not lookup %s pinstate\n",
			PINCTRL_STATE_RESET_LOW);
		ret = -EINVAL;
		return ret;
	}

	cd->mtk_pinctrl.cs_high =
		pinctrl_lookup_state(cd->pctrl, PINCTRL_STATE_CS_HIGH);
	if (IS_ERR_OR_NULL(cd->mtk_pinctrl.cs_high)) {
		thp_log_err("Can not lookup %s pinstate\n",
			PINCTRL_STATE_CS_HIGH);
		ret = -EINVAL;
		return ret;
	}

	cd->mtk_pinctrl.cs_low =
		pinctrl_lookup_state(cd->pctrl, PINCTRL_STATE_CS_LOW);
	if (IS_ERR_OR_NULL(cd->mtk_pinctrl.cs_low)) {
		thp_log_err("Can not lookup %s pinstate\n",
			PINCTRL_STATE_CS_LOW);
		ret = -EINVAL;
		return ret;
	}

	cd->mtk_pinctrl.as_int =
		pinctrl_lookup_state(cd->pctrl, PINCTRL_STATE_AS_INT);
	if (IS_ERR_OR_NULL(cd->mtk_pinctrl.as_int)) {
		thp_log_err("Can not lookup %s pinstate\n",
			PINCTRL_STATE_AS_INT);
		ret = -EINVAL;
		return ret;
	}

	ret = pinctrl_select_state(cd->pctrl, cd->mtk_pinctrl.as_int);
	if (ret < 0)
		thp_log_err("set gpio as int failed\n");

	if (cd->change_spi_driving_force) {
		cd->mtk_pinctrl.spi_status = pinctrl_lookup_state(cd->pctrl,
			PINCTRL_STATE_SPI_STATUS);
		if (IS_ERR_OR_NULL(cd->mtk_pinctrl.spi_status)) {
			thp_log_err("Can not lookup %s pinstate\n",
				PINCTRL_STATE_SPI_STATUS);
			ret = -EINVAL;
			return ret;
		}
		ret = pinctrl_select_state(cd->pctrl,
			cd->mtk_pinctrl.spi_status);
		if (ret < 0)
			thp_log_err("change spi driving force failed\n");
	}
	return ret;
}
#endif

static int thp_setup_irq(struct thp_core_data *cd)
{
	int rc;
	int irq;
	unsigned long irq_flag_type;
	u32 current_trigger_mode;

	if (!cd) {
		thp_log_err("%s: thp_core_data is null\n", __func__);
		return -EINVAL;
	}

#ifndef CONFIG_HUAWEI_THP_MTK
	irq = gpio_to_irq(cd->gpios.irq_gpio);
#else
	irq = irq_of_parse_and_map(cd->thp_node, 0);
#endif

	/*
	 * IRQF_TRIGGER_RISING 0x00000001
	 * IRQF_TRIGGER_FALLING 0x00000002
	 * IRQF_TRIGGER_HIGH 0x00000004
	 * IRQF_TRIGGER_LOW 0x00000008
	 * IRQF_NO_SUSPEND 0x00004000
	 */
	current_trigger_mode = cd->irq_flag;
	thp_log_info("%s:current_trigger_mode->0x%x\n",
		__func__, current_trigger_mode);

	if (cd->support_gesture_mode || cd->use_ap_gesture)
		irq_flag_type = IRQF_ONESHOT |
			IRQF_NO_SUSPEND |
			current_trigger_mode;
	else
		irq_flag_type = IRQF_ONESHOT | current_trigger_mode;
	rc = request_threaded_irq(irq, NULL,
				thp_irq_thread, irq_flag_type,
				"thp", cd);
	if (rc) {
		thp_log_err("%s: request irq fail\n", __func__);
		return rc;
	}
	mutex_lock(&cd->irq_mutex);
	disable_irq(irq);
	cd->irq_enabled = false;
	mutex_unlock(&cd->irq_mutex);
	thp_log_info("%s: disable irq\n", __func__);
	cd->irq = irq;

	return 0;
}

static int thp_setup_gpio(struct thp_core_data *cd)
{
	int rc;

	thp_log_info("%s: called\n", __func__);

	rc = gpio_request(cd->gpios.rst_gpio, "thp_reset");
	if (rc) {
		thp_log_err("%s:gpio_request %d failed\n", __func__,
				cd->gpios.rst_gpio);
		return rc;
	}

	rc = gpio_request(cd->gpios.cs_gpio, "thp_cs");
	if (rc) {
		thp_log_err("%s:gpio_request %d failed\n", __func__,
				cd->gpios.cs_gpio);
		gpio_free(cd->gpios.rst_gpio);
		return rc;
	}
	gpio_direction_output(cd->gpios.cs_gpio, GPIO_HIGH);
	thp_log_info("%s:set cs gpio %d deault hi\n", __func__,
				cd->gpios.cs_gpio);

	rc = gpio_request(cd->gpios.irq_gpio, "thp_int");
	if (rc) {
		thp_log_err("%s: irq gpio %d request failed\n", __func__,
				cd->gpios.irq_gpio);
		gpio_free(cd->gpios.rst_gpio);
		gpio_free(cd->gpios.cs_gpio);
		return rc;
	}
	rc = gpio_direction_input(cd->gpios.irq_gpio);
	if (rc)
		thp_log_info("%s:gpio_direction_input failed\n", __func__);

	return 0;
}

static void thp_free_gpio(struct thp_core_data *ts)
{
	gpio_free(ts->gpios.irq_gpio);
	gpio_free(ts->gpios.cs_gpio);
	gpio_free(ts->gpios.rst_gpio);
}

static int thp_setup_spi(struct thp_core_data *cd)
{
	int rc;

#ifdef CONFIG_HUAWEI_THP_MTK
	cd->mtk_spi_config.cs_setuptime =
		cd->thp_dev->timing_config.spi_sync_cs_low_delay_ns;
#endif
	rc = spi_setup(cd->sdev);
	if (rc) {
		thp_log_err("%s: spi setup fail\n", __func__);
		return rc;
	}

	return 0;
}
int thp_set_spi_com_mode(struct thp_core_data *cd, u8 spi_com_mode)
{
	int rc;

	if (!cd) {
		thp_log_err("%s: tdev null\n", __func__);
		return -EINVAL;
	}

	if (spi_com_mode != SPI_DMA_MODE && spi_com_mode != SPI_POLLING_MODE) {
		thp_log_err("%s ->error mode\n", __func__);
		return -EINVAL;
	}
	cd->spi_config.pl022_spi_config.com_mode = spi_com_mode;
	cd->sdev->controller_data = &cd->spi_config.pl022_spi_config;
	rc = thp_setup_spi(cd);
	thp_log_info("%s rc->%d\n", __func__, rc);
	return rc;
}
#if defined(CONFIG_TEE_TUI)
static u32 thp_get_multi_pm_status(void)
{
	struct thp_core_data *cd = thp_get_core_data();

	thp_log_info("%s current_pm_status:%u\n", __func__,
		cd->current_pm_status);
	return cd->current_pm_status;
}

__attribute__((weak)) int i2c_init_secos(struct i2c_adapter *adap)
{
	return 0;
}

__attribute__((weak)) int i2c_exit_secos(struct i2c_adapter *adap)
{
	return 0;
}

#ifdef CONFIG_HUAWEI_THP_MTK
int spi_exit_secos(unsigned int spi_bus_id)
{
	struct thp_core_data *cd = thp_get_core_data();
	int rc;

	rc = thp_setup_spi(cd);
	thp_log_info("%s rc = %d\n", __func__, rc);

	return 0;
}

int spi_init_secos(unsigned int spi_bus_id)
{
	thp_log_info("%s enter, %u\n", __func__, spi_bus_id);
	return 0;
}
#endif

void thp_tui_secos_init(void)
{
	struct thp_core_data *cd = thp_get_core_data();
	int t;
#ifdef CONFIG_HUAWEI_SHB_THP
	int ret;
#endif

	if (!cd) {
		thp_log_err("%s: core not inited\n", __func__);
		return;
	}

	/* NOTICE: should not change this path unless ack daemon */
	thp_set_status(THP_STATUS_TUI, 1);
	cd->thp_ta_waitq_flag = WAITQ_WAIT;

	thp_log_info("%s: busid=%d. disable irq=%d\n",
		__func__, cd->spi_config.bus_id, cd->irq);
	t = wait_event_interruptible_timeout(cd->thp_ta_waitq,
		(cd->thp_ta_waitq_flag == WAITQ_WAKEUP), HZ);
	thp_log_info("%s: wake up finish\n", __func__);

#ifdef CONFIG_HUAWEI_SHB_THP
	if (cd->support_shb_thp) {
		if (cd->thp_dev && cd->thp_dev->ops &&
			cd->thp_dev->ops->switch_int_sh) {
			ret = cd->thp_dev->ops->switch_int_sh(cd->thp_dev);
			if (ret)
				thp_log_err("%s: switch to sh fail", __func__);
		}
		thp_set_irq_status(cd, THP_IRQ_DISABLE);
		ret = send_thp_driver_status_cmd(TP_SWITCH_ON, 0,
			ST_CMD_TYPE_SET_TUI_STATUS);
		if (ret)
			thp_log_err("%s: send thp tui on fail", __func__);
	} else {
#endif
		thp_set_irq_status(cd, THP_IRQ_DISABLE);
		if (cd->no_need_secos_bus_init == 0)
			spi_init_secos(cd->spi_config.bus_id);
#ifdef CONFIG_HUAWEI_SHB_THP
	}
#endif
	tui_enable = 1;
	thp_log_info("%s set tui_enable:%d\n", __func__, tui_enable);
	if (cd->multi_panel_index != SINGLE_TOUCH_PANEL)
		tp_tui_data[MAIN_TOUCH_PANEL].panel_in_tui = true;
}

void thp_tui_secos_exit(void)
{
	struct thp_core_data *cd = thp_get_core_data();
#ifdef CONFIG_HUAWEI_SHB_THP
	int ret;
#endif

	if (!cd) {
		thp_log_err("%s: core not inited\n", __func__);
		return;
	}
	if (cd->send_tui_exit_in_suspend && (!tui_enable)) {
		thp_log_err("%s TUI has exit\n", __func__);
		return;
	}
	tui_enable = 0;
	thp_log_info("%s: busid=%u tui_enable=%u\n", __func__,
		cd->spi_config.bus_id, tui_enable);
  	if (cd->multi_panel_index != SINGLE_TOUCH_PANEL)
		tp_tui_data[MAIN_TOUCH_PANEL].panel_in_tui = false;
#ifdef CONFIG_HUAWEI_SHB_THP
	if (cd->support_shb_thp) {
		if (cd->thp_dev && cd->thp_dev->ops &&
			cd->thp_dev->ops->switch_int_ap) {
			ret = cd->thp_dev->ops->switch_int_ap(cd->thp_dev);
			if (ret)
				thp_log_err("%s: switch to ap fail", __func__);
			}
		ret = send_thp_driver_status_cmd(TP_SWITCH_OFF, 0,
			ST_CMD_TYPE_SET_TUI_STATUS);
		if (ret)
			thp_log_err("%s: send thp tui off fail", __func__);
	} else {
#endif
		if (cd->no_need_secos_bus_init == 0)
			spi_exit_secos(cd->spi_config.bus_id);
#ifdef CONFIG_HUAWEI_SHB_THP
	}
#endif
	mutex_lock(&cd->suspend_flag_mutex);
	if (cd->suspended && (!cd->need_work_in_suspend)) {
		thp_log_info("%s: drv suspended\n", __func__);
		mutex_unlock(&cd->suspend_flag_mutex);
		return;
	}
	thp_set_irq_status(cd, THP_IRQ_ENABLE);
	mutex_unlock(&cd->suspend_flag_mutex);
	thp_set_status(THP_STATUS_TUI, 0);
	thp_log_info("%s end\n", __func__);
}

static int thp_tui_switch(void *data, int secure)
{
	thp_log_info("%s:tui secure is %d\n", __func__, secure);

	if (secure)
		thp_tui_secos_init();
	else
		thp_tui_secos_exit();
	return 0;
}

static int multi_tp_tui_switch(void *data, int secure)
{
	int ret = 0;
	int panel_id;

	if (secure) {
		/* according to panel status to enter tui */
		switch (thp_get_multi_pm_status()) {
		case M_ON_S_OFF:
			panel_id = MAIN_TOUCH_PANEL;
			break;
		case M_OFF_S_ON:
			panel_id = SUB_TOUCH_PANEL;
			break;
		default:
			goto err;
		}
	} else {
		/* exit the panel which in tui mode */
		if (tp_tui_data[SUB_TOUCH_PANEL].panel_in_tui)
			panel_id = SUB_TOUCH_PANEL;
		else if (tp_tui_data[MAIN_TOUCH_PANEL].panel_in_tui)
			panel_id = MAIN_TOUCH_PANEL;
		else
			goto err;
	}

	memcpy(&thp_tui_info, &tp_tui_data[panel_id], sizeof(thp_tui_info));
	ret = tp_tui_data[panel_id].tui_drv_switch(data, secure);
	thp_log_info("%s:device_name: %s  ret:%d\n", __func__,
		thp_tui_info.project_id, ret);
	return ret;
err:
	thp_log_err("%s invalid mode\n", __func__);
	return ret;
}

#define IC_THP_SHB_TUI_COMMON_ID "shb_tui"
#define IC_THP_SHB_TUI_COMMON_ID_LEN 8

static void thp_tui_init(struct thp_core_data *cd)
{
	int rc;

	if (!cd) {
		thp_log_err("%s: core not inited\n", __func__);
		return;
	}
	cd->current_pm_status = M_ON_S_ON;
	tui_enable = 0;
	if (cd->support_shb_thp && cd->support_shb_thp_tui_common_id) {
#ifdef CONFIG_HUAWEI_SHB_THP
		rc = snprintf(thp_tui_info.project_id,
			IC_THP_SHB_TUI_COMMON_ID_LEN, IC_THP_SHB_TUI_COMMON_ID);
		if (rc < 0)
			thp_log_err("snprintf failed, rc: %d\n", rc);
#endif
		thp_log_info("%s thp_tui_info.project_id:%s\n",
			__func__, thp_tui_info.project_id);
	} else {
		strncpy(thp_tui_info.project_id, cd->project_id,
			THP_PROJECT_ID_LEN);
		thp_tui_info.project_id[THP_PROJECT_ID_LEN] = '\0';
		thp_log_info("%s thp_tui_info.project_id:%s\n",
			__func__, thp_tui_info.project_id);
	}
	thp_tui_info.frame_data_addr = cd->frame_data_addr;
	thp_log_info("%s thp_tui_info.get_frame_addr %d\n",
		__func__, thp_tui_info.frame_data_addr);
	memcpy(&tp_tui_data[MAIN_TOUCH_PANEL].tp_tui_normalized_data.thp_tui_data,
		&thp_tui_info, sizeof(thp_tui_info));
	if (cd->multi_panel_index == SINGLE_TOUCH_PANEL) {
		rc = register_tui_driver(thp_tui_switch, "tp", &thp_tui_info);
		if (rc != 0) {
			thp_log_err("%s reg thp_tui_switch fail: %d\n",
				__func__, rc);
			return;
		}
	} else {
		if (cd->multi_panel_index == MAIN_TOUCH_PANEL) {
			thp_log_err("%s register_tui_driver thp_tui_switch\n",
				__func__);
			tp_tui_data[MAIN_TOUCH_PANEL].tui_drv_switch =
				thp_tui_switch;
			thp_log_info("%s thp_tui_info.project_id:%s\n",
				__func__, thp_tui_info.project_id);
			rc = register_tui_driver(multi_tp_tui_switch,
				"tp", &thp_tui_info);
			if (rc != 0) {
				thp_log_err("%s reg thp_tui_switch fail: %d\n",
					__func__, rc);
				return;
			}
		}
	}
}
#endif

static int thp_pinctrl_get_init(struct thp_device *tdev)
{
	int ret = 0;

	tdev->thp_core->pctrl = devm_pinctrl_get(&tdev->sdev->dev);
	if (IS_ERR(tdev->thp_core->pctrl)) {
		thp_log_err("failed to devm pinctrl get\n");
		ret = -EINVAL;
		return ret;
	}

	tdev->thp_core->pins_default =
		pinctrl_lookup_state(tdev->thp_core->pctrl, "default");
	if (IS_ERR(tdev->thp_core->pins_default)) {
		thp_log_err("failed to pinctrl lookup state default\n");
		ret = -EINVAL;
		goto err_pinctrl_put;
	}

	tdev->thp_core->pins_idle =
			pinctrl_lookup_state(tdev->thp_core->pctrl, "idle");
	if (IS_ERR(tdev->thp_core->pins_idle)) {
		thp_log_err("failed to pinctrl lookup state idle\n");
		ret = -EINVAL;
		goto err_pinctrl_put;
	}

#ifdef CONFIG_HUAWEI_THP_MTK
	ret = thp_mtk_pinctrl_get_init(tdev);
	if (ret < 0) {
		thp_log_err("%s: mtk pinctrl init failed\n", __func__);
		goto err_pinctrl_put;
	}
#endif

	return 0;

err_pinctrl_put:
	devm_pinctrl_put(tdev->thp_core->pctrl);
	return ret;
}

static int thp_pinctrl_select_normal(struct thp_device *tdev)
{
	int retval;

	retval =
		pinctrl_select_state(tdev->thp_core->pctrl,
		tdev->thp_core->pins_default);
	if (retval < 0)
		thp_log_err("set iomux normal error, %d\n", retval);
	return retval;
}

static int thp_pinctrl_select_lowpower(struct thp_device *tdev)
{
	int retval;

	retval = pinctrl_select_state(tdev->thp_core->pctrl,
		tdev->thp_core->pins_idle);
	if (retval < 0)
		thp_log_err("set iomux lowpower error, %d\n", retval);
	return retval;
}

const char *thp_get_vendor_name(void)
{
	struct thp_core_data *cd = thp_get_core_data();

	if (cd && cd->support_multi_panel_attach)
		return (cd->thp_dev && cd->thp_dev->multi_vendor_name) ?
			cd->thp_dev->multi_vendor_name : "null";

	return (cd && cd->thp_dev) ? cd->thp_dev->ic_name : 0;
}
EXPORT_SYMBOL(thp_get_vendor_name);

static int thp_project_in_tp(struct thp_core_data *cd)
{
	int ret = 0;
	unsigned int value = 0;

	if (cd->project_in_tp && cd->thp_dev->ops->get_project_id)
		ret = cd->thp_dev->ops->get_project_id(
						cd->thp_dev,
						cd->project_id,
						THP_PROJECT_ID_LEN);

	if (ret) {
		strncpy(
			cd->project_id,
			cd->project_id_dummy,
			THP_PROJECT_ID_LEN);
		thp_log_info("%s:get projectfail ,use dummy id:%s\n",
			__func__, cd->project_id);
	}

	/* projectid_from_panel_ver: open 1, close 0 */
	if (cd->projectid_from_panel_ver == 1) {
#ifndef CONFIG_LCD_KIT_DRIVER
		if (lcdkit_get_panel_version(&value))
			return ret;
#endif
		thp_log_info("%s SVX value from LCD is %u\n", __func__, value);
		value = value >> 4; /* right offset 4 bits */
		if (value == SV1_PANEL) {
			strncpy(cd->project_id, PROJECTID_SV1,
				THP_PROJECT_ID_LEN);
			thp_log_info("%s:project id SV1 is :%s\n", __func__,
				cd->project_id);
		} else if (value == SV2_PANEL) {
			strncpy(cd->project_id, PROJECTID_SV2,
				THP_PROJECT_ID_LEN);
			thp_log_info("%s:project id SV2 is :%s\n",
				__func__, cd->project_id);
		}
	}

	thp_log_info("%s:project id:%s\n", __func__, cd->project_id);
	return ret;
}

static int thp_edit_product_in_project_id(char *project_id,
	unsigned int project_id_length, const char *product)
{
	size_t len;

	if (product == NULL) {
		thp_log_err("%s:product is NULL\n", __func__);
		return -EINVAL;
	}
	len = strlen(product);

	thp_log_info("%s:product len is %zu\n", __func__, len);
	if ((len > project_id_length) || (len == 0))
		return -EINVAL;
	memcpy(project_id, product, len);

	return 0;
}

static int thp_project_id_mapping(struct thp_core_data *cd)
{
	size_t len;

	if ((cd->target_project_id == NULL) ||
		(cd->map_project_id == NULL)) {
		thp_log_err("%s: target_map_project_id is NULL\n", __func__);
		return -EINVAL;
	}
	len = strlen(cd->target_project_id);
	if ((len > THP_PROJECT_ID_LEN) || (len == 0))
		return -EINVAL;
	if (memcmp(cd->target_project_id, cd->project_id, len) == 0)
		memcpy(cd->project_id, cd->map_project_id, len);
	return 0;
}

static int thp_project_init(struct thp_core_data *cd)
{
	int ret = thp_project_in_tp(cd);

	if (cd->edit_product_in_project_id &&
		strncmp(cd->project_id, cd->project_id_exception,
		THP_PROJECT_ID_LEN))
		ret = thp_edit_product_in_project_id(cd->project_id,
			THP_PROJECT_ID_LEN, cd->product);
	if (cd->support_project_id_mapping)
		ret = thp_project_id_mapping(cd);
	thp_log_info("%s:edit_product:%d project id:%s ret:%d\n", __func__,
		cd->edit_product_in_project_id, cd->project_id, ret);
	if (ret != 0)
		return -EINVAL;

	return 0;
}

#if ((defined CONFIG_HUAWEI_THP_MTK) || (defined CONFIG_HUAWEI_THP_QCOM))
__attribute__((weak)) int power_event_bnc_register(unsigned int type,
	struct notifier_block *nb)
{
	return 0;
}

__attribute__((weak)) int power_event_bnc_unregister(unsigned int type,
	struct notifier_block *nb)
{
	return 0;
}

static int thp_ctrl_event_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	int charger_state_before = thp_get_status(THP_STATUS_CHARGER);
	int charger_state_new = charger_state_before;

	thp_log_debug("%s char event %d\n", __func__, event);
	switch (event) {
	case POWER_NE_USB_CONNECT:
	case POWER_NE_WIRELESS_CONNECT:
		charger_state_new = CHARGER_CONNECT;
		break;
	case POWER_NE_USB_DISCONNECT:
	case POWER_NE_WIRELESS_DISCONNECT:
		charger_state_new = CHARGER_DISCONNECT;
		break;
	default:
		break;
	}
	if (charger_state_before != charger_state_new) {
		thp_log_info("%s, set new status: %d\n",
			__func__, charger_state_new);
		thp_set_status(THP_STATUS_CHARGER, charger_state_new);
	}
	return 0;
}

static void thp_nt_charger_notifier_register(struct thp_core_data *cd)
{
	int ret;

	if (!cd->supported_charger) {
		thp_log_info("%s No supported charger\n", __func__);
		return;
	}
	cd->charger_detect_notify.notifier_call = thp_ctrl_event_call;
	ret = power_event_bnc_register(POWER_BNT_CONNECT,
		&cd->charger_detect_notify);
	if (ret) {
		thp_log_err("%s power_event_nc_register failed\n", __func__);
		cd->charger_detect_notify.notifier_call = NULL;
	}
}
#endif

static int thp_core_init(struct thp_core_data *cd)
{
	int rc;

	/* step 1 : init mutex */
	mutex_init(&cd->mutex_frame);
	mutex_init(&cd->irq_mutex);
	mutex_init(&cd->thp_mutex);
	mutex_init(&cd->status_mutex);
	mutex_init(&cd->suspend_flag_mutex);
#ifdef CONFIG_HUAWEI_THP_MTK
	mutex_init(&cd->aod_power_ctrl_mutex);
#endif
	wakeup_source_init(&cd->thp_wake_lock, "thp_wake_lock");
	if (cd->support_gesture_mode)
		mutex_init(&cd->thp_wrong_touch_lock);
	dev_set_drvdata(&cd->sdev->dev, cd);
	cd->ic_name = cd->thp_dev->ic_name;
	cd->prox_cache_enable = false;
	cd->need_work_in_suspend = false;
	g_thp_prox_enable = false;
	onetime_poweroff_done = false;
	cd->work_status = RESUME_DONE;

#if defined(CONFIG_HUAWEI_DSM)
	if (cd->ic_name)
		dsm_thp.ic_name = cd->ic_name;
	if (strlen(cd->project_id))
		dsm_thp.module_name = cd->project_id;
	dsm_thp_dclient = dsm_register_client(&dsm_thp);
#endif
	rc = thp_project_init(cd);
	if (rc)
		thp_log_err("%s: failed to get project id form tp ic\n",
			__func__);

	rc = misc_register(&g_thp_misc_device);
	if (rc)	{
		thp_log_err("%s: failed to register misc device\n", __func__);
		goto err_register_misc;
	}

	rc = thp_mt_wrapper_init();
	if (rc) {
		thp_log_err("%s: failed to init input_mt_wrapper\n", __func__);
		goto err_init_wrapper;
	}

	rc = thp_init_sysfs(cd);
	if (rc) {
		thp_log_err("%s: failed to create sysfs\n", __func__);
		goto err_init_sysfs;
	}

	rc = thp_setup_irq(cd);
	if (rc) {
		thp_log_err("%s: failed to set up irq\n", __func__);
		goto err_register_misc;
	}
#ifndef CONFIG_LCD_KIT_DRIVER
	cd->lcd_notify.notifier_call = thp_lcdkit_notifier_callback;
	rc = lcdkit_register_notifier(&cd->lcd_notify);
	if (rc) {
		thp_log_err("%s: failed to register fb_notifier: %d\n",
			__func__, rc);
		goto err_register_fb_notify;
	}
#endif

#if defined(CONFIG_LCD_KIT_DRIVER)

	rc = ts_kit_ops_register(&thp_ops);
	if (rc)
		thp_log_info("%s:ts_kit_ops_register fail\n", __func__);
#endif

#ifdef CONFIG_BCI_BATTERY
	cd->charger_detect_notify.notifier_call =
			thp_charger_detect_notifier_callback;
	rc = bci_register_notifier(&cd->charger_detect_notify, 1);
	if (rc < 0) {
		thp_log_err("%s:charger notifier register failed\n", __func__);
		cd->charger_detect_notify.notifier_call = NULL;
	} else {
		thp_log_info("%s:charger notifier register succ\n", __func__);
	}
#endif
#if ((defined CONFIG_HUAWEI_THP_MTK) || (defined CONFIG_HUAWEI_THP_QCOM))
	thp_nt_charger_notifier_register(cd);
#endif
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	set_hw_dev_flag(DEV_I2C_TOUCH_PANEL);
#endif

#if defined(CONFIG_TEE_TUI)
	thp_tui_init(cd);
#endif

#ifdef CONFIG_HUAWEI_SHB_THP
	if (cd->support_shb_thp_log) {
		if (thp_log_init())
			thp_log_err("%s: failed to init thp log thread\n",
				__func__);
	} else {
		thp_log_info("%s: sensorhub thp log is disabled\n", __func__);
	}
#endif
	atomic_set(&cd->register_flag, 1);
	thp_set_status(THP_STATUS_POWER, 1);
	return 0;

err_init_sysfs:
	thp_mt_wrapper_exit();
err_init_wrapper:
	misc_deregister(&g_thp_misc_device);
err_register_misc:

#ifndef CONFIG_LCD_KIT_DRIVER
	lcdkit_unregister_notifier(&cd->lcd_notify);
err_register_fb_notify:
#endif

#if defined(CONFIG_LCD_KIT_DRIVER)
	rc = ts_kit_ops_unregister(&thp_ops);
	if (rc)
		thp_log_info("%s:ts_kit_ops_register fail\n", __func__);
#endif
	mutex_destroy(&cd->mutex_frame);
	mutex_destroy(&cd->irq_mutex);
	mutex_destroy(&cd->thp_mutex);
#ifdef CONFIG_HUAWEI_THP_MTK
	mutex_destroy(&cd->aod_power_ctrl_mutex);
#endif
	wakeup_source_trash(&cd->thp_wake_lock);
	if (cd->support_gesture_mode)
		mutex_destroy(&cd->thp_wrong_touch_lock);
	return rc;
}

static int thp_parse_test_config(struct device_node *test_node,
	struct thp_test_config *config)
{
	int rc;
	unsigned int value = 0;

	if (!test_node || !config) {
		thp_log_info("%s: input dev null\n", __func__);
		return -ENODEV;
	}

	rc = of_property_read_u32(test_node,
			"pt_station_test", &value);
	if (!rc) {
		config->pt_station_test = value;
		thp_log_info("%s:pt_test_flag %d\n",
			__func__, value);
	}

	return 0;
}

static struct device_node *thp_get_dev_node(struct thp_core_data *cd,
	struct thp_device *dev)
{
	struct device_node *dev_node = of_get_child_by_name(cd->thp_node,
						dev->ic_name);

	if (!dev_node && dev->dev_node_name)
		return of_get_child_by_name(cd->thp_node, dev->dev_node_name);

	return dev_node;
}

static void thp_chip_detect(struct thp_cmd_node *in_cmd)
{
	int ret;
	struct thp_device *dev = NULL;

	if (in_cmd == NULL) {
		thp_log_err("%s:input is NULL\n", __func__);
		return;
	}
	dev = in_cmd->cmd_param.pub_params.dev;
	ret = thp_register_dev(dev);
	if (ret)
		thp_log_err("%s,register failed\n", __func__);
}

void thp_send_detect_cmd(struct thp_device *dev, int timeout)
{
	int error;
	struct thp_cmd_node cmd;

	thp_log_info("%s: called\n", __func__);
	if (dev == NULL) {
		thp_log_info("%s: input is invalid\n", __func__);
		return;
	}
	thp_unregister_ic_num++;
	thp_log_info("%s:thp_unregister_ic_num:%d",
		__func__, thp_unregister_ic_num);
	memset(&cmd, 0, sizeof(cmd));
	cmd.command = TS_CHIP_DETECT;
	cmd.cmd_param.pub_params.dev = dev;
	error = thp_put_one_cmd(&cmd, timeout);
	if (error)
		thp_log_err("%s: put cmd error :%d\n", __func__, error);
}

void thp_time_delay(unsigned int time)
{
	struct thp_core_data *cd = thp_get_core_data();

	if (time == 0)
		return;
	if ((time < TIME_DELAY_MS_MAX) && cd->use_mdelay)
		mdelay(time);
	else
		thp_do_time_delay(time);
}

int thp_register_dev(struct thp_device *dev)
{
	int rc = -EINVAL;
	struct thp_core_data *cd = thp_get_core_data();

	if ((dev == NULL) || (cd == NULL)) {
		thp_log_err("%s: input null\n", __func__);
		goto register_err;
	}
	thp_log_info("%s: called\n", __func__);
	/* check device configed ot not */
	if (!thp_get_dev_node(cd, dev)) {
		thp_log_info("%s:%s not config in dts\n",
				__func__, dev->ic_name);
		goto register_err;
	}

	if (atomic_read(&cd->register_flag)) {
		thp_log_err("%s: thp have registerd\n", __func__);
		goto register_err;
	}

	if (!cd->project_in_tp && cd->ic_name && dev->ic_name &&
			strcmp(cd->ic_name, dev->ic_name)) {
		thp_log_err("%s:driver support ic mismatch connected device\n",
					__func__);
		goto register_err;
	}

	dev->thp_core = cd;
	dev->gpios = &cd->gpios;
	dev->sdev = cd->sdev;
	cd->thp_dev = dev;
	is_fw_update = 0;

	rc = thp_parse_timing_config(cd->thp_node, &dev->timing_config);
	if (rc) {
		thp_log_err("%s: timing config parse fail\n", __func__);
		goto register_err;
	}

	rc = thp_parse_test_config(cd->thp_node, &dev->test_config);
	if (rc) {
		thp_log_err("%s: special scene config parse fail\n", __func__);
		goto register_err;
	}

	rc = dev->ops->init(dev);
	if (rc) {
		thp_log_err("%s: dev init fail\n", __func__);
		goto dev_init_err;
	}

#ifdef CONFIG_HUAWEI_THP_MTK
	if (cd->support_pinctrl == 1) {
		rc = thp_pinctrl_get_init(dev);
		if (rc) {
			thp_log_err("%s:pinctrl get init fail\n", __func__);
			goto dev_init_err;
		}
		pinctrl_select_state(dev->thp_core->pctrl,
			dev->thp_core->mtk_pinctrl.cs_high);
	}
#else
	rc = thp_setup_gpio(cd);
	if (rc) {
		thp_log_err("%s: spi dev init fail\n", __func__);
		goto dev_init_err;
	}
#endif

	rc = thp_setup_spi(cd);
	if (rc) {
		thp_log_err("%s: spi dev init fail\n", __func__);
		goto err;
	}

	rc = dev->ops->detect(dev);
	if (rc) {
		thp_log_err("%s: chip detect fail\n", __func__);
		goto err;
	}
#ifndef CONFIG_HUAWEI_THP_MTK
	if (cd->support_pinctrl == 1) {
		rc = thp_pinctrl_get_init(dev);
		if (rc) {
			thp_log_err("%s:pinctrl get init fail\n", __func__);
			goto err;
		}
		if (cd->pinctrl_init_enable) {
			rc = thp_pinctrl_select_normal(cd->thp_dev);
			if (rc) {
				thp_log_err(
					"%s:thp_pinctrl_select_normal fail\n",
					__func__);
				goto err;
			}
			thp_log_info("%s: thp_pinctrl_select_normal sucessed\n",
				__func__);
		}
	}
#endif
	rc = thp_core_init(cd);
	if (rc) {
		thp_log_err("%s: core init\n", __func__);
		goto err;
	}
	if (cd->fast_booting_solution) {
		thp_unregister_ic_num--;
		thp_log_info("%s:thp_unregister_ic_num :%d",
			__func__, thp_unregister_ic_num);
	}
	return 0;
err:
#ifndef CONFIG_HUAWEI_THP_MTK
	thp_free_gpio(cd);
#endif
dev_init_err:
	cd->thp_dev = 0;
register_err:
	if (cd && cd->fast_booting_solution) {
		thp_unregister_ic_num--;
		thp_log_info("%s:thp_unregister_ic_num :%d",
			__func__, thp_unregister_ic_num);
	}
	return rc;
}
EXPORT_SYMBOL(thp_register_dev);

#define THP_SUPPORT_PINCTRL "support_pinctrl"
int thp_parse_pinctrl_config(struct device_node *spi_cfg_node,
	struct thp_core_data *cd)
{
	int rc;
	unsigned int value = 0;

	if ((spi_cfg_node == NULL) || (cd == NULL)) {
		thp_log_info("%s: input null\n", __func__);
		return -ENODEV;
	}
	rc = of_property_read_u32(spi_cfg_node, THP_SUPPORT_PINCTRL, &value);
	if (rc == 0)
		cd->support_pinctrl = value;
	else
		cd->support_pinctrl = 0;
	thp_log_info("%s:support_pinctrl %d\n", __func__, value);

	rc = of_property_read_u32(spi_cfg_node, PINCTRL_INIT_ENABLE, &value);
	if (rc == 0)
		cd->pinctrl_init_enable = value;
	else
		cd->pinctrl_init_enable = 0;
	thp_log_info("%s:pinctrl_init_enable %u\n", __func__, value);

	return 0;
}

int thp_parse_spi_config(struct device_node *spi_cfg_node,
	struct thp_core_data *cd)
{
	int rc;
	unsigned int value;
	struct thp_spi_config *spi_config = NULL;
	struct pl022_config_chip *pl022_spi_config = NULL;

	if (!spi_cfg_node || !cd) {
		thp_log_info("%s: input null\n", __func__);
		return -ENODEV;
	}

	spi_config = &cd->spi_config;
	pl022_spi_config = &cd->spi_config.pl022_spi_config;

	value = 0;
	rc = of_property_read_u32(spi_cfg_node, "spi-max-frequency", &value);
	if (!rc) {
		spi_config->max_speed_hz = value;
		thp_log_info("%s:spi-max-frequency configed %d\n",
				__func__, value);
	}

	value = 0;
	rc = of_property_read_u32(spi_cfg_node, "spi-bus-id", &value);
	if (!rc) {
		spi_config->bus_id = (u8)value;
		thp_log_info("%s:spi-bus-id configed %d\n", __func__, value);
	}

	value = 0;
	rc = of_property_read_u32(spi_cfg_node, "spi-mode", &value);
	if (!rc) {
		spi_config->mode = value;
		thp_log_info("%s:spi-mode configed %d\n", __func__, value);
	}

	value = 0;
	rc = of_property_read_u32(spi_cfg_node, "bits-per-word", &value);
	if (!rc) {
		spi_config->bits_per_word = value;
		thp_log_info("%s:bits-per-word configed %d\n", __func__, value);
	}

	value = 0;
	rc = of_property_read_u32(spi_cfg_node, "pl022,interface", &value);
	if (!rc) {
		pl022_spi_config->iface = value;
		thp_log_info("%s: pl022,interface parsed\n", __func__);
	}
	value = 0;
	rc = of_property_read_u32(spi_cfg_node, "pl022,com-mode", &value);
	if (!rc) {
		pl022_spi_config->com_mode = value;
		thp_log_info("%s:com_mode parsed\n", __func__);
	}
	value = 0;
	rc = of_property_read_u32(spi_cfg_node, "pl022,rx-level-trig", &value);
	if (!rc) {
		pl022_spi_config->rx_lev_trig = value;
		thp_log_info("%s:rx-level-trig parsed\n", __func__);
	}

	value = 0;
	rc = of_property_read_u32(spi_cfg_node, "pl022,tx-level-trig", &value);
	if (!rc) {
		pl022_spi_config->tx_lev_trig = value;
		thp_log_info("%s:tx-level-trig parsed\n", __func__);
	}

	value = 0;
	rc = of_property_read_u32(spi_cfg_node, "pl022,ctrl-len", &value);
	if (!rc) {
		pl022_spi_config->ctrl_len = value;
		thp_log_info("%s:ctrl-len parsed\n", __func__);
	}

	value = 0;
	rc = of_property_read_u32(spi_cfg_node, "pl022,wait-state", &value);
	if (!rc) {
		pl022_spi_config->wait_state = value;
		thp_log_info("%s:wait-state parsed\n", __func__);
	}

	value = 0;
	rc = of_property_read_u32(spi_cfg_node, "pl022,duplex", &value);
	if (!rc) {
		pl022_spi_config->duplex = value;
		thp_log_info("%s:duplex parsed\n", __func__);
	}

	cd->spi_config.pl022_spi_config.cs_control = thp_spi_cs_set;
	cd->spi_config.pl022_spi_config.hierarchy = SSP_MASTER;

	if (!cd->spi_config.max_speed_hz)
		cd->spi_config.max_speed_hz = THP_SPI_SPEED_DEFAULT;
	if (!cd->spi_config.mode)
		cd->spi_config.mode = SPI_MODE_0;
	if (!cd->spi_config.bits_per_word)
	/* spi_config.bits_per_word default value is 8 */
		cd->spi_config.bits_per_word = 8;

#ifdef CONFIG_HUAWEI_THP_MTK
	/* tx ordering, 1-msb first send; 0-lsb first end */
	cd->mtk_spi_config.rx_mlsb = 1;
	/* rx ordering, 1-msb first send; 0-lsb first end */
	cd->mtk_spi_config.tx_mlsb = 1;
	/* control cs polarity, 0-active low; 1-active high */
	cd->mtk_spi_config.cs_pol = 0;
	/* control sample edge, 0-positive edge; 1-negative edge */
	cd->mtk_spi_config.sample_sel = 0;
	/* cs setup time, 0-default time */
	cd->mtk_spi_config.cs_setuptime = 0;
#endif
	cd->sdev->mode = spi_config->mode;
	cd->sdev->max_speed_hz = spi_config->max_speed_hz;
	cd->sdev->bits_per_word = spi_config->bits_per_word;
#ifndef CONFIG_HUAWEI_THP_MTK
	cd->sdev->controller_data = &spi_config->pl022_spi_config;
#else
	cd->sdev->controller_data = (void *)&(cd->mtk_spi_config);
#endif

	return 0;
}
EXPORT_SYMBOL(thp_parse_spi_config);

int thp_parse_timing_config(struct device_node *timing_cfg_node,
	struct thp_timing_config *timing)
{
	int rc;
	unsigned int value;

	if (!timing_cfg_node || !timing) {
		thp_log_info("%s: input null\n", __func__);
		return -ENODEV;
	}

	rc = of_property_read_u32(timing_cfg_node,
					"boot_reset_hi_delay_ms", &value);
	if (!rc) {
		timing->boot_reset_hi_delay_ms = value;
		thp_log_info("%s:boot_reset_hi_delay_ms configed %d\n",
				__func__, value);
	}

	rc = of_property_read_u32(timing_cfg_node,
					"boot_reset_low_delay_ms", &value);
	if (!rc) {
		timing->boot_reset_low_delay_ms = value;
		thp_log_info("%s:boot_reset_low_delay_ms configed %d\n",
				__func__, value);
	}

	rc = of_property_read_u32(timing_cfg_node,
					"boot_reset_after_delay_ms", &value);
	if (!rc) {
		timing->boot_reset_after_delay_ms = value;
		thp_log_info("%s:boot_reset_after_delay_ms configed %d\n",
				__func__, value);
	}

	rc = of_property_read_u32(timing_cfg_node,
					"resume_reset_after_delay_ms", &value);
	if (!rc) {
		timing->resume_reset_after_delay_ms = value;
		thp_log_info("%s:resume_reset_after_delay_ms configed %d\n",
				__func__, value);
	}

	rc = of_property_read_u32(timing_cfg_node,
					"suspend_reset_after_delay_ms", &value);
	if (!rc) {
		timing->suspend_reset_after_delay_ms = value;
		thp_log_info("%s:suspend_reset_after_delay configed_ms %d\n",
				__func__, value);
	}

	rc = of_property_read_u32(timing_cfg_node,
					"spi_sync_cs_hi_delay_ns", &value);
	if (!rc) {
		timing->spi_sync_cs_hi_delay_ns = value;
		thp_log_info("%s:spi_sync_cs_hi_delay_ns configed_ms %d\n",
				__func__, value);
	}

	rc = of_property_read_u32(timing_cfg_node,
					"spi_sync_cs_low_delay_ns", &value);
	if (!rc) {
		timing->spi_sync_cs_low_delay_ns = value;
		thp_log_info("%s:spi_sync_cs_low_delay_ns configed_ms %d\n",
				__func__, value);
	}
	rc = of_property_read_u32(timing_cfg_node,
					"spi_sync_cs_low_delay_us", &value);
	if (!rc) {
		timing->spi_sync_cs_low_delay_us = value;
		thp_log_info("%s:spi_sync_cs_low_delay_us = %d\n",
				__func__, value);
	} else {
		timing->spi_sync_cs_low_delay_us = 0;
	}

	rc = of_property_read_u32(timing_cfg_node,
		"boot_vcc_on_after_delay_ms", &value);
	if (!rc) {
		timing->boot_vcc_on_after_delay_ms = value;
		thp_log_info("%s:boot_vcc_on_after_delay_ms configed_ms %d\n",
			__func__, value);
	}
	rc = of_property_read_u32(timing_cfg_node,
		"boot_vddio_on_after_delay_ms", &value);
	if (!rc) {
		timing->boot_vddio_on_after_delay_ms = value;
		thp_log_info("%s:boot_vddio_on_after_delay_ms configed_ms %d\n",
			__func__, value);
	}
	rc = of_property_read_u32(timing_cfg_node,
		"spi_transfer_delay_us", &value);
	if (!rc) {
		timing->spi_transfer_delay_us = value;
		thp_log_info("%s:spi_transfer_delay_us = %d\n",
			__func__, value);
	} else {
		timing->spi_transfer_delay_us = 0;
	}
	if (!of_property_read_u32(timing_cfg_node,
		"early_suspend_delay_ms", &value)) {
		timing->early_suspend_delay_ms = value;
		thp_log_info("%s:early_suspend_delay_ms configed_ms %u\n",
			__func__, value);
	}
	return 0;
}
EXPORT_SYMBOL(thp_parse_timing_config);

int thp_parse_trigger_config(struct device_node *thp_node,
	struct thp_core_data *cd)
{
	int rc;
	unsigned int value = 0;

	thp_log_debug("%s:Enter!\n", __func__);
	rc = of_property_read_u32(thp_node, "irq_flag", &value);
	if (!rc) {
		cd->irq_flag = value;
		thp_log_info("%s:cd->irq_flag %d\n", __func__, value);
	} else {
		cd->irq_flag = IRQF_TRIGGER_FALLING;
		thp_log_info("%s:cd->irq_flag defaule => %d\n",
			__func__, cd->irq_flag);
	}
	return 0;
}
EXPORT_SYMBOL(thp_parse_trigger_config);

static void thp_parse_pen_feature_config(
	struct device_node *thp_node, struct thp_core_data *cd)
{
	int rc;
	unsigned int value = 0;

	rc = of_property_read_u32(thp_node, "pen_supported", &value);
	if (!rc) {
		cd->pen_supported = value;
		thp_log_info("%s:pen_supported %u\n", __func__, value);
	}

	rc = of_property_read_u32(thp_node, "pen_mt_enable_flag",
		&value);
	if (!rc) {
		cd->pen_mt_enable_flag = value;
		thp_log_info("%s:pen_mt_enable_flag configed %u\n",
			__func__, value);
	}

	rc = of_property_read_u32(thp_node, "pen_change_protocol", &value);
	if (!rc) {
		cd->pen_change_protocol = value;
		thp_log_info("%s:pen_change_protocol %u\n", __func__, value);
	}

	rc = of_property_read_u32(thp_node, "send_bt_status_to_fw", &value);
	if (!rc) {
		cd->send_bt_status_to_fw = value;
		thp_log_info("%s: send_bt_status_to_fw %u\n", __func__,
			value);
	}

	rc = of_property_read_u32(thp_node,
		"support_stylus3_plam_suppression", &value);
	if (!rc) {
		cd->support_stylus3_plam_suppression = value;
		thp_log_info("%s: support_stylus3_plam_suppression %u\n",
			__func__, value);
	}
}

static void thp_parse_multi_panel_config(
	struct device_node *thp_node, struct thp_core_data *cd)
{
	unsigned int value = 0;

	cd->multi_panel_index = SINGLE_TOUCH_PANEL;
	if (!of_property_read_u32(thp_node, "multi_panel_index", &value))
		cd->multi_panel_index = value;
	thp_log_info("%s: multi_panel_index %u\n", __func__,
		cd->multi_panel_index);

	cd->support_multi_panel_attach = 0;
	if (!of_property_read_u32(thp_node, "support_multi_panel_attach",
		&value))
		cd->support_multi_panel_attach = value;
	thp_log_info("%s: support_multi_panel_attach %u\n",
		__func__, cd->support_multi_panel_attach);
}

static void  thp_parse_extra_feature_config(
	struct device_node *thp_node, struct thp_core_data *cd)
{
	int rc;
	unsigned int value = 0;

	rc = of_property_read_u32(thp_node, "get_spi_hw_info_enable", &value);
	if (!rc) {
		cd->get_spi_hw_info_enable = value;
		thp_log_info("%s: get_spi_hw_info_enable %u\n",
			__func__, value);
	}

	rc = of_property_read_u32(thp_node, "support_factory_mode_extra_cmd",
		&value);
	if (!rc) {
		cd->support_factory_mode_extra_cmd = value;
		thp_log_info("%s: support_factory_mode_extra_cmd %u\n",
			__func__, value);
	}
	value = 0;
	rc = of_property_read_u32(thp_node, "support_no_cs_pinctrl_for_mtk",
		&value);
	if (!rc) {
		cd->support_no_cs_pinctrl_for_mtk = value;
		thp_log_info("%s:support_no_cs_pinctrl_for_mtk %u\n",
			__func__, value);
	}
	cd->support_control_cs_off = 0;
	rc = of_property_read_u32(thp_node, "support_control_cs_off", &value);
	if (!rc) {
		cd->support_control_cs_off = value;
		thp_log_info("%s:support_control_cs_off %u\n", __func__,
			value);
	}

	if (of_find_property(thp_node, "kirin-udp", NULL))
		cd->is_udp = true;
	else
		cd->is_udp = false;

	rc = of_property_read_u32(thp_node,
		"support_shb_thp_tui_common_id", &value);
	if (!rc) {
		cd->support_shb_thp_tui_common_id = value;
		thp_log_info("%s: support_shb_thp_tui_common_id %u\n",
			__func__, value);
	}

	rc = of_property_read_u32(thp_node, "support_diff_resolution",
		&value);
	if (!rc) {
		cd->support_diff_resolution = value;
		thp_log_info("%s: support_diff_resolution %u\n",
			__func__, value);
	}

	rc = of_property_read_u32(thp_node, "need_notify_to_roi_algo",
		&value);
	if (!rc) {
		cd->need_notify_to_roi_algo = value;
		thp_log_info("%s: need_notify_to_roi_algo %u\n",
			__func__, value);
	}

	rc = of_property_read_u32(thp_node, "support_daemon_init_protect",
		&value);
	if (!rc) {
		cd->support_daemon_init_protect = value;
		thp_log_info("%s: support_daemon_init_protect %u\n",
			__func__, value);
	}
	cd->support_reuse_ic_type = 0;
	rc = of_property_read_u32(thp_node, "support_reuse_ic_type",
		&value);
	if (!rc) {
		cd->support_reuse_ic_type = value;
		thp_log_info("%s: support_reuse_ic_type %u\n",
			__func__, value);
	}
	rc = of_property_read_u32(thp_node, "use_thp_queue",
		&value);
	if (!rc) {
		cd->use_thp_queue = value;
		thp_log_info("%s: use_thp_queue %u\n",
			__func__, value);
	}
	rc = of_property_read_u32(thp_node, "thp_queue_buf_len",
		&value);
	if (!rc) {
		cd->thp_queue_buf_len = value;
		thp_log_info("%s: thp_queue_buf_len %u\n",
			__func__, value);
	}
	rc = of_property_read_u32(thp_node, "supported_charger", &value);
	if (!rc) {
		cd->supported_charger = value;
		thp_log_info("%s:supported_charger %u\n", __func__, value);
	}
	rc = of_property_read_u32(thp_node, "support_phone_sleep_mode",
		&value);
	if (!rc) {
		cd->support_phone_sleep_mode = value;
		thp_log_info("%s: support_phone_sleep_mode %u\n",
			__func__, value);
	}

	thp_parse_multi_panel_config(thp_node, cd);
}

static void thp_parse_project_id_map_config(struct device_node *thp_node,
	struct thp_core_data *cd)
{
	unsigned int value;

	cd->support_project_id_mapping = 0;
	if (!of_property_read_u32(thp_node, "support_project_id_mapping",
		&value)) {
		cd->support_project_id_mapping = value;
		thp_log_info("%s: support_project_id_mapping %u\n", __func__,
			value);
	}
	if (!(cd->support_project_id_mapping))
		return;
	cd->target_project_id = NULL;
	cd->map_project_id = NULL;
	if (!of_property_read_string(thp_node, "target_project_id",
		(const char **)&cd->target_project_id))
		thp_log_info("%s: target_project_id %s\n", __func__,
			cd->target_project_id);
	if (!of_property_read_string(thp_node, "map_project_id",
		(const char **)&cd->map_project_id))
		thp_log_info("%s: map_project_id %s\n", __func__,
			cd->map_project_id);
}

int thp_parse_feature_config(struct device_node *thp_node,
	struct thp_core_data *cd)
{
	int rc;
	unsigned int value = 0;

	thp_log_debug("%s:Enter!\n", __func__);
	rc = of_property_read_u32(thp_node, "need_huge_memory_in_spi", &value);
	if (!rc) {
		cd->need_huge_memory_in_spi = value;
		thp_log_info("%s:need_huge_memory_in_spi configed %u\n",
				__func__, value);
	}
	rc = of_property_read_u32(thp_node, "self_control_power", &value);
	if (!rc) {
		cd->self_control_power = value;
		thp_log_info("%s:self_control_power configed %u\n",
			__func__, value);
	}
	rc = of_property_read_u32(thp_node, "project_in_tp", &value);
	if (!rc) {
		cd->project_in_tp = value;
		thp_log_info("%s:project_in_tp: %u\n", __func__, value);
	}

	rc = of_property_read_u32(thp_node, "lcd_esd_event_upload", &value);
	if (!rc) {
		cd->lcd_esd_event_upload = value;
		thp_log_info("%s:lcd_esd_event_upload: %u\n", __func__, value);
	}

	cd->project_id_dummy = "dummy";
	rc = of_property_read_string(thp_node, "project_id_dummy",
				(const char **)&cd->project_id_dummy);
	if (!rc)
		thp_log_info("%s:project_id_dummy configed %s\n",
				__func__, cd->project_id_dummy);

	rc = of_property_read_u32(thp_node, "supported_func_indicater",
				&value);
	if (!rc) {
		cd->supported_func_indicater = value;
		thp_log_info("%s:supported_func_indicater configed %u\n",
				__func__, value);
	}

	rc = of_property_read_u32(thp_node, "use_hwlock", &value);
	if (!rc) {
		cd->use_hwlock = value;
		thp_log_info("%s:use_hwlock configed %u\n", __func__, value);
	}

	rc = of_property_read_u32(thp_node, "support_shb_thp", &value);
	if (!rc) {
		cd->support_shb_thp = value;
		thp_log_info("%s:support_shb_thp configed %u\n",
			__func__, value);
	}

	rc = of_property_read_u32(thp_node, "support_shb_thp_log", &value);
	if (!rc) {
		cd->support_shb_thp_log = value;
		thp_log_info("%s:support_shb_thp_log configed %u\n",
			__func__, value);
	}

	rc = of_property_read_u32(thp_node, "support_shb_thp_app_switch",
		&value);
	if (!rc) {
		cd->support_shb_thp_app_switch = value;
		thp_log_info("%s:support_shb_thp_app_switch configed %u\n",
			__func__, value);
	}

	rc = of_property_read_u32(thp_node, "delay_work_for_pm", &value);
	if (!rc) {
		cd->delay_work_for_pm = value;
		thp_log_info("%s:delay_work_for_pm configed %u\n",
				__func__, value);
	}
#ifdef CONFIG_HUAWEI_THP_MTK
	rc = of_property_read_u32(thp_node, "use_ap_gesture", &value);
	if (!rc) {
		cd->use_ap_gesture = value;
		thp_log_info("%s:use_ap_gesture configed %u\n",
			__func__, value);
	}
	rc = of_property_read_u32(thp_node, "use_aod_power_ctrl_notify",
		&value);
	if (!rc) {
		cd->use_aod_power_ctrl_notify = value;
		thp_log_info("%s:use_aod_power_ctrl_notify config %u\n",
			__func__, value);
	}
	if (cd->use_aod_power_ctrl_notify) {
		if (!of_property_read_u32(thp_node,
			"suspend_delayms_early_to_before", &value)) {
			cd->suspend_delayms_early_to_before = value;
			thp_log_info("%s:suspend_delayms_early_to_before %u\n",
				__func__, value);
		}
		if (!of_property_read_u32(thp_node,
			"suspend_delayms_before_to_later", &value)) {
			cd->suspend_delayms_before_to_later = value;
			thp_log_info("%s:suspend_delayms_before_to_later %u\n",
				__func__, value);
		}
		if (!of_property_read_u32(thp_node,
			"resume_delayms_early_to_later", &value)) {
			cd->resume_delayms_early_to_later = value;
			thp_log_info("%s:resume_delayms_early_to_later %u\n",
				__func__, value);
		}
	}
#endif
	rc = of_property_read_u32(thp_node, "support_gesture_mode", &value);
	if (!rc) {
		cd->support_gesture_mode = value;
		thp_log_info("%s:support_gesture_mode configed %u\n",
			__func__, value);
		rc = of_property_read_u32(thp_node, "gesture_from_sensrohub",
			&value);
		if (!rc) {
			cd->gesture_from_sensorhub = value;
			thp_log_info("%s:gesture from sensorhub configed %u\n",
				__func__, value);
		}
	}

	rc = of_property_read_u32(thp_node, "aod_display_support", &value);
	if (!rc) {
		cd->aod_display_support = value;
		thp_log_info("%s:aod_display_support configed %u\n",
			__func__, value);
	}

	rc = of_property_read_u32(thp_node, "tsa_event_to_udfp", &value);
	if (!rc) {
		cd->tsa_event_to_udfp = value;
		thp_log_info("%s:tsa_event_to_udfp configed %u\n",
			__func__, value);
	}

	if (!of_property_read_u32(thp_node, "lcd_gesture_mode_support",
		&value)) {
		cd->lcd_gesture_mode_support = value;
		thp_log_info("%s:lcd_gesture_mode_support configed %u\n",
			__func__, value);
	}
	/*
	 * TDDI(TP powered by LCD) download fw in afe screen on,
	 * need wait interruptible to make sure of screen on done.
	 */
	if (!of_property_read_u32(thp_node, "wait_afe_screen_on_support",
		&value)) {
		cd->wait_afe_screen_on_support = value;
		thp_log_info("%s:wait_afe_screen_on_support configed %u\n",
			__func__, value);
	}

#if defined(CONFIG_TEE_TUI)
	if (!of_property_read_u32(thp_node, "send_tui_exit_in_suspend",
		&value)) {
		cd->send_tui_exit_in_suspend = value;
		thp_log_info("%s:send_tui_exit_in_suspend configed %u\n",
			__func__, value);
	}
	if (!of_property_read_u32(thp_node, "no_need_secos_bus_init",
		&value)) {
		cd->no_need_secos_bus_init = value;
		thp_log_info("%s:no_need_secos_bus_init configed %u\n",
			__func__, value);
	}
#endif

	rc = of_property_read_u32(thp_node, "support_ring_feature", &value);
	if (!rc) {
		cd->support_ring_feature = value;
		/* open or close ring switch by user, this is initial value */
		cd->ring_setting_switch = value;
		thp_log_info("%s:support_ring_feature configed %u\n",
			__func__, value);
	}

	rc = of_property_read_u32(thp_node, "support_ring_setting_switch",
		&value);
	if (!rc) {
		cd->support_ring_setting_switch = value;
		thp_log_info("%s:support_ring_setting_switch configed %u\n",
			__func__, value);
	}

	rc = of_property_read_u32(thp_node, "support_fingerprint_switch",
		&value);
	if (!rc) {
		cd->support_fingerprint_switch = value;
		thp_log_info("%s:support_fingerprint_switch configed %u\n",
			__func__, value);
	}

	rc = of_property_read_u32(thp_node, "support_extra_key_event_input",
		&value);
	if (!rc) {
		cd->support_extra_key_event_input = value;
		thp_log_info("%s:support_extra_key_event_input configed %u\n",
			__func__, value);
	}
	rc = of_property_read_u32(thp_node, "hide_product_info_en", &value);
	if (!rc) {
		cd->hide_product_info_en = value;
		thp_log_info("%s:hide_product_info_en %u\n", __func__, value);
	}
	rc = of_property_read_u32(thp_node, "support_oem_info", &value);
	if (!rc) {
		cd->support_oem_info = value;
		thp_log_info("%s:support_oem_info %u\n", __func__, value);
	}

	rc = of_property_read_u32(thp_node, "projectid_from_panel_ver", &value);
	if (!rc) {
		cd->projectid_from_panel_ver = value;
		thp_log_info("%s:projectid_from_panel_ver %u\n",
			__func__, value);
	}

	cd->support_vendor_ic_type = 0;
	rc = of_property_read_u32(thp_node, "support_vendor_ic_type", &value);
	if (!rc) {
		cd->support_vendor_ic_type = value;
		thp_log_info("%s:support_vendor_ic_type %u\n", __func__, value);
	}

	rc = of_property_read_u32(thp_node, "suspend_no_reset", &value);
	if (!rc) {
		cd->suspend_no_reset = value;
		thp_log_info("%s:suspend_no_reset %u\n", __func__, value);
	}

	cd->edit_product_in_project_id = 0;
	if (!of_property_read_u32(thp_node, "edit_product_in_project_id",
		&value)) {
		cd->edit_product_in_project_id = value;
		thp_log_info("%s: edit_product_in_project_id %u\n", __func__,
			value);
	}
	if (cd->edit_product_in_project_id) {
		cd->product = NULL;
		if (!of_property_read_string(thp_node, "product",
			(const char **)&cd->product))
			thp_log_info("%s:product configed %s\n", __func__,
				cd->product);
		cd->project_id_exception = "dummy";
		if (!of_property_read_string(thp_node, "project_id_exception",
			(const char **)&cd->project_id_exception))
			thp_log_info("%s:project_id_exception configed %s\n",
				__func__, cd->project_id_exception);
	}
	cd->need_resume_reset = 0;
	rc = of_property_read_u32(thp_node, "need_resume_reset", &value);
	if (!rc) {
		cd->need_resume_reset = value;
		thp_log_info("%s:need_resume_reset %u\n", __func__, value);
	}
	cd->disable_irq_gesture_suspend = 0;
	if (!of_property_read_u32(thp_node, "disable_irq_gesture_suspend",
		&value)) {
		cd->disable_irq_gesture_suspend = value;
		thp_log_info("%s: disable_irq_gesture_suspend %u\n", __func__,
			value);
	}
	cd->support_dual_chip_arch = 0;
	if (!of_property_read_u32(thp_node, "support_dual_chip_arch",
		&value)) {
		cd->support_dual_chip_arch = value;
		thp_log_info("%s: support_dual_chip_arch %u\n", __func__,
			value);
	}

	thp_parse_project_id_map_config(thp_node, cd);

#ifdef CONFIG_HUAWEI_THP_MTK
	value = 0;
	cd->change_spi_driving_force = 0;
	rc = of_property_read_u32(thp_node, "change_spi_driving_force", &value);
	if (!rc) {
		cd->change_spi_driving_force = value;
		thp_log_info("%s:change_spi_driving_force %u\n", __func__,
			value);
	}
	cd->need_enable_irq_wake = 0;
	rc = of_property_read_u32(thp_node, "need_enable_irq_wake", &value);
	if (!rc) {
		cd->need_enable_irq_wake = value;
		thp_log_info("%s:need_enable_irq_wake %u\n", __func__, value);
	}
#endif
	cd->gesture_retry_times = 20; /* defult gesture retry times:20 */
	rc = of_property_read_u32(thp_node, "gesture_retry_times", &value);
	if (!rc) {
		cd->gesture_retry_times = value;
		thp_log_info("%s:gesture_retry_times %u\n", __func__, value);
	}
	cd->lcd_need_get_afe_status = 0;
	if (!of_property_read_u32(thp_node, "lcd_need_get_afe_status",
		&value)) {
		cd->lcd_need_get_afe_status = value;
		thp_log_info("%s: lcd_need_get_afe_status %u\n", __func__,
			value);
	}

	rc = of_property_read_u32(thp_node,
		"support_report_interval_adjustment", &value);
	if (!rc) {
		cd->support_interval_adjustment = value;
		rc = of_property_read_u32(thp_node, "time_interval", &value);
		if (!rc) {
			cd->time_interval = value;
			thp_log_info("%s:support_adjustment %u\n",
				__func__, cd->time_interval);
		} else {
			cd->support_interval_adjustment = 0;
		}
		rc = of_property_read_u32(thp_node,
			"time_min_interval", &value);
		if (!rc) {
			cd->time_min_interval = value;
			thp_log_info("%s:support_adjustment %u\n",
				__func__, cd->time_min_interval);
		} else {
			cd->support_interval_adjustment = 0;
		}
	}
	rc = of_property_read_u32(thp_node, "support_screen_switch", &value);
	if (!rc) {
		cd->support_screen_switch = value;
		thp_log_info("%s: support_screen_switch %u\n", __func__, value);
	}

	rc = of_property_read_u32(thp_node,
		"iovdd_power_on_delay_ms", &value);
	if (!rc) {
		cd->iovdd_power_on_delay_ms = value;
		thp_log_info("%s:iovdd_power_on_delay_ms %u\n",
			__func__, cd->iovdd_power_on_delay_ms);
	} else {
		cd->iovdd_power_on_delay_ms = 1; /* default delay 1 ms */
	}

	cd->change_vendor_name = 0;
	rc = of_property_read_u32(thp_node, "change_vendor_name",
		&value);
	if (!rc) {
		cd->change_vendor_name = value;
		thp_log_info("%s: change_vendor_name %u\n",
			__func__, value);
	}

	return 0;
}
EXPORT_SYMBOL(thp_parse_feature_config);

int is_pt_test_mode(struct thp_device *tdev)
{
	int thp_pt_station_flag = 0;

#if defined(CONFIG_LCD_KIT_DRIVER)
	int ret;
	struct lcd_kit_ops *lcd_ops = lcd_kit_get_ops();

	if ((lcd_ops) && (lcd_ops->get_status_by_type)) {
		ret = lcd_ops->get_status_by_type(PT_STATION_TYPE,
			&thp_pt_station_flag);
		if (ret < 0) {
			thp_log_info("%s: get thp_pt_station_flag fail\n",
				__func__);
			return ret;
		}
	}
#else
	thp_pt_station_flag = g_tskit_pt_station_flag &&
			tdev->test_config.pt_station_test;
#endif

	thp_log_info("%s thp_pt_station_flag = %d\n", __func__,
		thp_pt_station_flag);

	return thp_pt_station_flag;
}
EXPORT_SYMBOL(is_pt_test_mode);

int is_tp_detected(void)
{
	int ret = TP_DETECT_SUCC;
	struct thp_core_data *cd = thp_get_core_data();

	if (!cd) {
		thp_log_err("%s: thp_core_data is not inited\n", __func__);
		return TP_DETECT_FAIL;
	}
	if (!atomic_read(&cd->register_flag))
		ret = TP_DETECT_FAIL;

	thp_log_info(
		"[Proximity_feature] %s : Check if tp is in place, ret = %d\n",
		__func__, ret);
	return ret;
}

/*
 * Here to count the period of time which is from suspend to a new
 * disable status, if the period is less than 1000ms then call lcdkit
 * power off, otherwise bypass the additional power off.
 */
static bool thp_prox_timeout_check(struct thp_core_data *cd)
{
	long delta_time;
	struct timeval tv;

	memset(&tv, 0, sizeof(tv));
	do_gettimeofday(&tv);
	thp_log_info(
		"[Proximity_feature] check time at %ld seconds %ld microseconds\n",
		tv.tv_sec, tv.tv_usec);
	/* multiply 1000000 to transfor second to us */
	delta_time = (tv.tv_sec - cd->tp_suspend_record_tv.tv_sec) * 1000000 +
		tv.tv_usec - cd->tp_suspend_record_tv.tv_usec;
	/* divide 1000 to transfor sec to us to ms */
	delta_time /= 1000;
	thp_log_info("[Proximity_feature] delta_time = %ld ms\n", delta_time);
	if (delta_time >= AFTER_SUSPEND_TIME)
		return true;
	else
		return false;
}

/*
 * After lcd driver complete the additional power down,calling this function
 * do something for matching current power status. Such as updating the
 * proximity switch status, sending the screen_off cmd to tp daemon, pulling
 * down the gpios and so on.
 */
static void thp_prox_add_suspend(struct thp_core_data *cd, bool enable)
{
	thp_log_info("[Proximity_feature] %s call enter\n", __func__);
	/* update the control status based on proximity switch */
	cd->need_work_in_suspend = enable;
	/* notify daemon to do screen off cmd */
	thp_set_status(THP_STATUS_POWER, THP_SUSPEND);
	/* notify daemon to do proximity off cmd */
	thp_set_status(THP_STATUS_AFE_PROXIMITY, THP_PROX_EXIT);
	/* pull down the gpio pin */
#ifndef CONFIG_HUAWEI_THP_MTK
	gpio_set_value(cd->thp_dev->gpios->rst_gpio, 0);
	gpio_set_value(cd->thp_dev->gpios->cs_gpio, 0);
#else
	if (cd->reset_status_in_suspend_mode)
		thp_log_info("[Proximity_feature] %s second power down\n",
			__func__);
	else
		pinctrl_select_state(cd->pctrl, cd->mtk_pinctrl.reset_low);
	pinctrl_select_state(cd->pctrl, cd->mtk_pinctrl.cs_low);
#endif
	/* disable the irq */
	if (cd->open_count)
		thp_set_irq_status(cd, THP_IRQ_DISABLE);
	cd->work_status = SUSPEND_DONE;
	/* clean the fingers */
	thp_clean_fingers();
	thp_log_info("[Proximity_feature] %s call exit\n", __func__);
}

/*
 * In this function, differentiating lcdkit1.0 and lcdkit 3.0's interfaces,
 * and increasing some judgements, only meet these conditions then
 * the additional power off will be called.
 */
static void thp_prox_add_poweroff(struct thp_core_data *cd, bool enable)
{
#ifdef CONFIG_LCD_KIT_DRIVER
	struct lcd_kit_ops *tp_ops = lcd_kit_get_ops();
#endif

	if ((enable == false) && (onetime_poweroff_done == false)) {
		onetime_poweroff_done = true;
		if (thp_prox_timeout_check(cd)) {
			thp_log_info(
				"[Proximity_feature] timeout, bypass poweroff\n");
			return;
		}
#ifdef CONFIG_LCDKIT_DRIVER
		if (!lcdkit_proximity_poweroff())
			thp_prox_add_suspend(cd, enable);
#endif

#ifdef CONFIG_LCD_KIT_DRIVER
		if (tp_ops && tp_ops->proximity_power_off) {
			if (!tp_ops->proximity_power_off())
				thp_prox_add_suspend(cd, enable);
		} else {
			thp_log_err("[Proximity_feature] point is null\n");
		}
#endif
	}
}

/*
 * This function receive the proximity switch status and save it for
 *  controlling power operation or cmds transferring to daemon
 * (proximity_on or scrren_off).
 */
int thp_set_prox_switch_status(bool enable)
{
#if ((defined CONFIG_INPUTHUB_20) || (defined CONFIG_HUAWEI_PS_SENSOR) || \
	(defined CONFIG_HUAWEI_SENSORS_2_0))
	int ret;
	int report_value[PROX_VALUE_LEN] = {0};
#endif
	struct thp_core_data *cd = thp_get_core_data();

	if (!cd) {
		thp_log_err("%s: thp_core_data is not inited\n", __func__);
		return 0;
	}
	if (!atomic_read(&cd->register_flag))
		return 0;

	if (cd->proximity_support == PROX_SUPPORT) {
#if ((defined CONFIG_INPUTHUB_20) || (defined CONFIG_HUAWEI_PS_SENSOR) || \
	(defined CONFIG_HUAWEI_SENSORS_2_0))
		report_value[0] = AWAY_EVENT_VALUE;
		ret = thp_prox_event_report(report_value, PROX_EVENT_LEN);
		if (ret < 0)
			thp_log_info("%s: report event fail\n", __func__);
		thp_log_info(
			"[Proximity_feature] %s: default report [far] event!\n",
			__func__);
#endif
		thp_set_status(THP_STATUS_TOUCH_APPROACH, enable);
		if (cd->early_suspended == false) {
			g_thp_prox_enable = enable;
			thp_log_info(
				"[Proximity_feature] %s: 1.Update g_thp_prox_enable to %d in screen on!\n",
				__func__, g_thp_prox_enable);
		} else {
			cd->prox_cache_enable = enable;
			thp_log_info(
				"[Proximity_feature] %s: 2.Update prox_cache_enable to %d in screen off!\n",
				__func__, cd->prox_cache_enable);
			/*
			 * When disable proximity after suspend,
			 * call power off once.
			 */
			thp_prox_add_poweroff(cd, enable);
		}
		return 0;
	}
	thp_log_info(
		"[Proximity_feature] %s : Not support proximity feature!\n",
		__func__);
	return 0;
}

/*
 * This function is supplied for lcd driver to get the current proximity status
 * when lcdkit go to power off, and use this status to contorl power.
 */
bool thp_get_prox_switch_status(void)
{
	struct thp_core_data *cd = thp_get_core_data();

	if (!cd) {
		thp_log_err("%s: thp_core_data is not inited\n", __func__);
		return 0;
	}
	if (cd->proximity_support == PROX_SUPPORT) {
		thp_log_info(
			"[Proximity_feature] %s: need_work_in_suspend = %d!\n",
			__func__, cd->need_work_in_suspend);
		return cd->need_work_in_suspend;
	}
	thp_log_info("[Proximity_feature] %s :Not support proximity feature!\n",
		__func__);
	return 0;
}

static int thp_parse_config(struct thp_core_data *cd,
	struct device_node *thp_node)
{
	int rc;
	unsigned int value;

	if (!thp_node) {
		thp_log_err("%s:thp not config in dts, exit\n", __func__);
		return -ENODEV;
	}

	rc = thp_parse_spi_config(thp_node, cd);
	if (rc) {
		thp_log_err("%s: spi config parse fail\n", __func__);
		return rc;
	}

	rc = thp_parse_power_config(thp_node, cd);
	if (rc) {
		thp_log_err("%s: power config parse fail\n", __func__);
		return rc;
	}

	rc = thp_parse_pinctrl_config(thp_node, cd);
	if (rc) {
		thp_log_err("%s:pinctrl parse fail\n", __func__);
		return rc;
	}

	cd->irq_flag = IRQF_TRIGGER_FALLING;
	rc = of_property_read_u32(thp_node, "irq_flag", &value);
	if (!rc) {
		cd->irq_flag = value;
		thp_log_info("%s:irq_flag parsed\n", __func__);
	}
	cd->fast_booting_solution = 0;
	rc = of_property_read_u32(thp_node, "fast_booting_solution", &value);
	if (!rc) {
		cd->fast_booting_solution = value;
		thp_log_info("%s:fast_booting_solution parsed:%d\n",
			__func__, cd->fast_booting_solution);
	}
	cd->use_mdelay = 0;
	if (!of_property_read_u32(thp_node, "use_mdelay", &value)) {
		cd->use_mdelay = value;
		thp_log_info("%s:use_mdelay parsed:%u\n",
			__func__, cd->use_mdelay);
	}
	cd->proximity_support = PROX_NOT_SUPPORT;
	rc = of_property_read_u32(thp_node, "proximity_support", &value);
	if (!rc) {
		cd->proximity_support = value;
		thp_log_info("%s:parsed success, proximity_support = %u\n",
			__func__, cd->proximity_support);
	} else {
		thp_log_info("%s:parsed failed, proximity_support = %u\n",
			__func__, cd->proximity_support);
	}

	cd->platform_type = THP_PLATFORM_HISI;
	rc = of_property_read_u32(thp_node, "platform_type", &value);
	if (!rc) {
		cd->platform_type = value;
		thp_log_info("%s:parsed success, platform_type = %u\n",
			__func__, cd->platform_type);
	} else {
		thp_log_info("%s:parsed failed, platform_type = %u\n",
			__func__, cd->platform_type);
	}

#ifndef CONFIG_HUAWEI_THP_MTK
	value = of_get_named_gpio(thp_node, "irq_gpio", 0);
	thp_log_info("irq gpio_ = %d\n", value);
	if (!gpio_is_valid(value)) {
		thp_log_err("%s: get irq_gpio failed\n", __func__);
		return rc;
	}
	cd->gpios.irq_gpio = value;

	value = of_get_named_gpio(thp_node, "rst_gpio", 0);
	thp_log_info("rst_gpio = %d\n", value);
	if (!gpio_is_valid(value)) {
		thp_log_err("%s: get rst_gpio failed\n", __func__);
		return rc;
	}
	cd->gpios.rst_gpio = value;

	value = of_get_named_gpio(thp_node, "cs_gpio", 0);
	thp_log_info("cs_gpio = %d\n", value);
	if (!gpio_is_valid(value)) {
		thp_log_err("%s: get cs_gpio failed\n", __func__);
		return rc;
	}
	cd->gpios.cs_gpio = value;
#endif
	thp_parse_feature_config(thp_node, cd);
	thp_parse_extra_feature_config(thp_node, cd);
	thp_parse_pen_feature_config(thp_node, cd);
	cd->thp_node = thp_node;
	return 0;
}

static int thp_cmd_sync_allocate(struct thp_cmd_node *cmd, int timeout)
{
	struct thp_cmd_sync *sync = NULL;

	if (timeout == 0) {
		cmd->sync = NULL;
		return 0;
	}
	sync = kzalloc(sizeof(*sync), GFP_KERNEL);
	if (sync == NULL) {
		thp_log_err("failed to kzalloc completion\n");
		return -ENOMEM;
	}
	init_completion(&sync->done);
	atomic_set(&sync->timeout_flag, TS_NOT_TIMEOUT);
	cmd->sync = sync;
	return 0;
}

int thp_put_one_cmd(struct thp_cmd_node *cmd, int timeout)
{
	int error = -EIO;
	unsigned long flags;
	struct thp_cmd_queue *q = NULL;
	struct thp_core_data *cd = thp_get_core_data();

	if ((cmd == NULL) || (cd == NULL)) {
		thp_log_err("%s:null pointer\n", __func__);
		goto out;
	}
	if ((!atomic_read(&cd->register_flag)) &&
		(cmd->command != TS_CHIP_DETECT)) {
		thp_log_err("%s: not initialize\n", __func__);
		goto out;
	}
	if (thp_cmd_sync_allocate(cmd, timeout)) {
		thp_log_debug("%s:allocate success\n", __func__);
		/*
		 * When the command execution timeout the memory occupied
		 * by sync will be released  in the command execution module,
		 * else the memory will be released after waiting for the
		 * command return normally.
		 */
		goto out;
	}
	q = &cd->queue;
	spin_lock_irqsave(&q->spin_lock, flags);
	smp_wmb(); /* Make sure queue has assigned correctly */
	if (q->cmd_count == q->queue_size) {
		spin_unlock_irqrestore(&q->spin_lock, flags);
		thp_log_err("%s:queue is full\n", __func__);
		WARN_ON(1);
		error = -EIO;
		goto free_sync;
	}
	memcpy(&q->ring_buff[q->wr_index], cmd, sizeof(*cmd));
	q->cmd_count++;
	q->wr_index++;
	q->wr_index %= q->queue_size;
	smp_mb(); /* Make sure queue is refreshed correctly */
	spin_unlock_irqrestore(&q->spin_lock, flags);
	thp_log_debug("%s:%d in ring buff\n", __func__, cmd->command);
	error = NO_ERR;
	wake_up_process(cd->thp_task); /* wakeup thp process */
	if (timeout && (cmd->sync != NULL) &&
		!(wait_for_completion_timeout(&cmd->sync->done,
			abs(timeout) * HZ))) {
		atomic_set(&cmd->sync->timeout_flag, TS_TIMEOUT);
		thp_log_err("%s:wait cmd respone timeout\n", __func__);
		error = -EBUSY;
		goto out;
	}
	smp_wmb(); /* Make sure code has been completed before function ends */
free_sync:
	kfree(cmd->sync);
	cmd->sync = NULL;
out:
	return error;
}

static int get_one_cmd(struct thp_cmd_node *cmd)
{
	unsigned long flags;
	int error = -EIO;
	struct thp_cmd_queue *q = NULL;
	struct thp_core_data *cd = thp_get_core_data();

	if (unlikely(!cmd)) {
		thp_log_err("%s:find null pointer\n", __func__);
		goto out;
	}

	q = &cd->queue;
	spin_lock_irqsave(&q->spin_lock, flags);
	smp_wmb(); /* Make sure queue has assigned correctly */
	if (!q->cmd_count) {
		thp_log_debug("%s: queue is empty\n", __func__);
		spin_unlock_irqrestore(&q->spin_lock, flags);
		goto out;
	}
	memcpy(cmd, &q->ring_buff[q->rd_index], sizeof(*cmd));
	q->cmd_count--;
	q->rd_index++;
	q->rd_index %= q->queue_size;
	smp_mb(); /* Make sure queue is refreshed correctly */
	spin_unlock_irqrestore(&q->spin_lock, flags);
	thp_log_debug("%s:%d from ring buff\n", __func__,
		cmd->command);
	error = NO_ERR;
out:
	return error;
}

static bool thp_task_continue(void)
{
	bool task_continue = true;
	unsigned long flags;
	struct thp_core_data *cd = thp_get_core_data();

	thp_log_debug("%s:prepare enter idle\n", __func__);
	while (task_continue) {
		if (unlikely(kthread_should_stop())) {
			task_continue = false;
			goto out;
		}
		spin_lock_irqsave(&cd->queue.spin_lock, flags);
		/*
		 * Make sure the memory and assignment are completed
		 * before updating the current process.
		 */
		smp_wmb();
		if (cd->queue.cmd_count) {
			set_current_state(TASK_RUNNING);
			thp_log_debug("%s:TASK_RUNNING\n", __func__);
			goto out_unlock;
		} else {
			set_current_state(TASK_INTERRUPTIBLE);
			thp_log_debug("%s:TASK_INTERRUPTIBLE\n", __func__);
			spin_unlock_irqrestore(&cd->queue.spin_lock, flags);
			schedule();
		}
	}

out_unlock:
	spin_unlock_irqrestore(&cd->queue.spin_lock, flags);
out:
	return task_continue;
}

static int thp_proc_command(struct thp_cmd_node *cmd)
{
	int error = NO_ERR;
	struct thp_cmd_sync *sync = NULL;
	struct thp_cmd_node *proc_cmd = cmd;
	struct thp_cmd_node *out_cmd = &pang_cmd_buff;

	if (!cmd) {
		thp_log_err("%s:invalid cmd\n", __func__);
		goto out;
	}
	sync = cmd->sync;
	/* discard timeout cmd */
	if (sync && (atomic_read(&sync->timeout_flag) == TS_TIMEOUT)) {
		kfree(sync);
		goto out;
	}
	while (true) {
		out_cmd->command = TS_INVAILD_CMD;
		switch (proc_cmd->command) {
		case TS_CHIP_DETECT:
			thp_chip_detect(proc_cmd);
			break;
#ifdef CONFIG_HUAWEI_THP_MTK
		case TS_SUSPEND:
			thp_suspend_thread();
			break;
		case TS_RESUME:
			thp_resume_thread();
			break;
#endif
#if defined CONFIG_LCD_KIT_DRIVER
		case THP_MUTIL_RESUME_THREAD:
			thp_multi_resume();
			break;
		case THP_MUTIL_SUSPEND_THREAD:
			thp_multi_suspend();
			break;
		case TSKIT_MUTIL_RESUME_THREAD:
			tskit_multi_resume();
			break;
		case TSKIT_MUTIL_SUSPEND_THREAD:
			tskit_multi_suspend();
			break;
		case TS_SCREEN_FOLD:
			error = multi_screen_status_switch(SCREEN_FOLDED);
			break;
		case TS_SCREEN_OFF_FOLD:
			error = multi_screen_status_switch(SCREEN_OFF_FOLD);
			break;
		case TS_SCREEN_UNFOLD:
			error = multi_screen_status_switch(SCREEN_UNFOLD);
			break;
#endif
		default:
			break;
		}

		thp_log_debug("%s:command :%d process result:%d\n",
			__func__, proc_cmd->command, error);
		if (out_cmd->command != TS_INVAILD_CMD) {
			thp_log_debug("%s:related command :%d  need process\n",
				__func__, out_cmd->command);
			/* ping - pang */
			swap(proc_cmd, out_cmd);
		} else {
			break;
		}
	}
	/* notify wait threads by completion */
	if (sync) {
		smp_mb(); /* Make sure current timeout_flag is up to date */
		thp_log_debug("%s:wakeup threads in waitqueue\n", __func__);
		if (atomic_read(&sync->timeout_flag) == TS_TIMEOUT)
			kfree(sync);
		else
			complete(&sync->done);
	}

out:
	return error;
}

static int thp_thread(void *p)
{
	static const struct sched_param param = {
		/* The priority of thread scheduling is 99 */
		.sched_priority = 99,
	};
	/*
	 * Make sure buff is properly refreshed
	 * before the process is executed.
	 */
	smp_wmb();
	thp_log_info("%s: in\n", __func__);
	memset(&ping_cmd_buff, 0, sizeof(ping_cmd_buff));
	memset(&pang_cmd_buff, 0, sizeof(pang_cmd_buff));
	/*
	 * Make sure buff is properly refreshed
	 * before the process is executed.
	 */
	smp_mb();
	sched_setscheduler(current, SCHED_RR, &param);
	while (thp_task_continue()) {
		/* get one command */
		while (!get_one_cmd(&ping_cmd_buff)) {
			thp_proc_command(&ping_cmd_buff);
			memset(&ping_cmd_buff, 0, sizeof(ping_cmd_buff));
			memset(&pang_cmd_buff, 0, sizeof(pang_cmd_buff));
		}
	}
	thp_log_err("%s: stop\n", __func__);
	return NO_ERR;
}

static int thp_thread_init(struct thp_core_data *cd)
{
	cd->thp_task = kthread_create(thp_thread, cd, "ts_thread:%d", 0);
	if (IS_ERR(cd->thp_task)) {
		kfree(cd->frame_read_buf);
		kfree(cd);
		g_thp_core = NULL;
		thp_log_err("%s: creat thread failed\n", __func__);
		return -ENODEV;
	}
	cd->queue.rd_index = 0;
	cd->queue.wr_index = 0;
	cd->queue.cmd_count = 0;
	cd->queue.queue_size = THP_CMD_QUEUE_SIZE;
	spin_lock_init(&cd->queue.spin_lock);
	smp_mb(); /* Make sure queue is initialized before wake up the task */
	wake_up_process(cd->thp_task);
	return 0;
}

static int thp_probe(struct spi_device *sdev)
{
	struct thp_core_data *thp_core = NULL;
	u8 *frame_read_buf = NULL;
	int rc;

	thp_log_info("%s: in\n", __func__);
#if (defined(CONFIG_LCD_KIT_DRIVER) && defined(CONFIG_HUAWEI_THP_QCOM))
	if (!lcd_kit_get_ops()) {
		thp_log_info("%s: lcdkit not init, need wait\n", __func__);
		return -EPROBE_DEFER;
	}
#endif
	thp_core = kzalloc(sizeof(struct thp_core_data), GFP_KERNEL);
	if (!thp_core) {
		thp_log_err("%s: thp_core out of memory\n", __func__);
		return -ENOMEM;
	}

	frame_read_buf = kzalloc(THP_MAX_FRAME_SIZE, GFP_KERNEL);
	if (!frame_read_buf) {
		thp_log_err("%s: frame_read_buf out of memory\n", __func__);
		kfree(thp_core);
		return -ENOMEM;
	}

	thp_core->frame_read_buf = frame_read_buf;
	thp_core->sdev = sdev;
	rc = thp_parse_config(thp_core, sdev->dev.of_node);
	if (rc) {
		thp_log_err("%s: parse dts fail\n", __func__);
		kfree(thp_core->frame_read_buf);
		kfree(thp_core);
		return -ENODEV;
	}
	if (thp_core->use_thp_queue) {
		thp_core->thp_queue = thp_queue_init(thp_core);
		if (thp_core->thp_queue == NULL) {
			thp_log_err("%s: kzalloc frame fail\n", __func__);
			kfree(thp_core->frame_read_buf);
			thp_core->frame_read_buf = NULL;
			kfree(thp_core);
			thp_core = NULL;
			return -ENOMEM;
		}
	}

	rc = thp_init_chip_info(thp_core);
	if (rc)
		thp_log_err("%s: chip info init fail\n", __func__);

	mutex_init(&thp_core->spi_mutex);
	thp_log_info("%s:use_hwlock = %d\n", __func__, thp_core->use_hwlock);
	if (thp_core->use_hwlock) {
		thp_core->hwspin_lock = hwspin_lock_request_specific(
			TP_HWSPIN_LOCK_CODE);
		if (!thp_core->hwspin_lock)
			thp_log_err("%s: hwspin_lock request failed\n",
				__func__);
	}

	atomic_set(&thp_core->register_flag, 0);
	INIT_LIST_HEAD(&thp_core->frame_list.list);
	init_waitqueue_head(&(thp_core->frame_waitq));
	init_waitqueue_head(&(thp_core->thp_ta_waitq));
	init_waitqueue_head(&(thp_core->thp_event_waitq));
	init_waitqueue_head(&(thp_core->suspend_resume_waitq));
	thp_core->event_flag = false;
	thp_core->suspend_resume_waitq_flag = WAITQ_WAKEUP;
	if (thp_core->wait_afe_screen_on_support) {
		init_waitqueue_head(&(thp_core->afe_screen_on_waitq));
		atomic_set(&(thp_core->afe_screen_on_waitq_flag), WAITQ_WAKEUP);
		thp_log_info("%s, init afe_screen_on_waitq done\n", __func__);
	}
	spi_set_drvdata(sdev, thp_core);

	g_thp_core = thp_core;
	if (thp_core->fast_booting_solution) {
		thp_unregister_ic_num = 0;
		rc = thp_thread_init(thp_core);
		return rc;
	}
	thp_log_info("%s:out\n", __func__);
	return 0;
}

static int thp_remove(struct spi_device *sdev)
{
	struct thp_core_data *cd = spi_get_drvdata(sdev);
#if defined(CONFIG_LCD_KIT_DRIVER)
	int rc;
#endif
	thp_log_info("%s: in\n", __func__);

	if (cd->use_thp_queue) {
		mutex_lock(&cd->mutex_frame);
		thp_queue_free(cd->thp_queue);
		mutex_unlock(&cd->mutex_frame);
	}

	if (atomic_read(&cd->register_flag)) {
		thp_sysfs_release(cd);

#if ((defined THP_CHARGER_FB) && (!defined CONFIG_HUAWEI_THP_QCOM) && \
	(!defined CONFIG_HUAWEI_THP_MTK))
		if (cd->charger_detect_notify.notifier_call)
			chip_charger_type_notifier_unregister(
					&cd->charger_detect_notify);
#endif
#if ((defined THP_CHARGER_FB) && ((defined CONFIG_HUAWEI_THP_MTK) || \
	(defined CONFIG_HUAWEI_THP_QCOM)))
	if (cd->charger_detect_notify.notifier_call && cd->supported_charger) {
		rc = power_event_bnc_unregister(POWER_BNT_CONNECT,
			&cd->charger_detect_notify);
		if (rc)
			thp_log_err("%s:power_event_nc_unregister fail\n",
				__func__);
	}
#endif
#ifndef CONFIG_LCD_KIT_DRIVER
		lcdkit_unregister_notifier(&cd->lcd_notify);
#endif

#if defined(CONFIG_LCD_KIT_DRIVER)
	rc = ts_kit_ops_unregister(&thp_ops);
	if (rc)
		thp_log_info("%s:ts_kit_ops_register fail\n", __func__);
#endif

		misc_deregister(&g_thp_misc_device);
		mutex_destroy(&cd->mutex_frame);
		thp_mt_wrapper_exit();
	}

	kfree(cd->frame_read_buf);
	kfree(spi_sync_rx_buf);
	spi_sync_rx_buf = NULL;

	kfree(spi_sync_tx_buf);
	spi_sync_tx_buf = NULL;

	kfree(cd);
	cd = NULL;

#if defined(CONFIG_TEE_TUI)
	unregister_tui_driver("tp");
#endif
	return 0;
}

static const struct of_device_id g_thp_psoc_match_table[] = {
	{ .compatible = "huawei,thp", },
	{ },
};

static const struct spi_device_id g_thp_device_id[] = {
	{ THP_DEVICE_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(spi, g_thp_device_id);

static struct spi_driver g_thp_spi_driver = {
	.probe = thp_probe,
	.remove = thp_remove,
	.id_table = g_thp_device_id,
	.driver = {
		.name = THP_DEVICE_NAME,
		.owner = THIS_MODULE,
		.bus = &spi_bus_type,
		.of_match_table = g_thp_psoc_match_table,
	},
};

module_spi_driver(g_thp_spi_driver);

MODULE_AUTHOR("Huawei Device Company");
MODULE_DESCRIPTION("Huawei THP Driver");
MODULE_LICENSE("GPL");

