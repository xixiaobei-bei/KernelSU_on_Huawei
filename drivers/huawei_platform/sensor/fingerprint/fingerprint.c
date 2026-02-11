/*
 * fingerprint.c
 *
 * fingerprint driver
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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
#include "fingerprint.h"
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/hisi/hw_cmdline_parse.h>
#include <media/huawei/hw_extern_pmic.h>
#include <linux/slab.h>
#include <linux/atomic.h>
#include <linux/hisi/hisi_cpufreq_lowtemp.h>
#include <linux/accurate_delay.h>
#if defined(CONFIG_FB)
#include <linux/notifier.h>
#include <linux/fb.h>
#endif
#include <huawei_platform/fingerprint_interface/fingerprint_interface.h>
#include <huawei_platform/sensor/hw_comm_pmic.h>
#include "hisi_fb.h"
#if defined(CONFIG_TEE_TUI)
#include "tui.h"
#endif

#if defined(CONFIG_HUAWEI_DSM)
#include <dsm/dsm_pub.h>

#define OPTICAL   1
#define COMMON    0
#define DSM_FP_BUFFER_SIZE 1024

struct dsm_dev dsm_fingerprint = {
	.name = "dsm_fingerprint",
	.device_name = "fingerprint",
	.ic_name = "NNN",
	.module_name = "NNN",
	.fops = NULL,
	.buff_size = DSM_FP_BUFFER_SIZE,
};

struct dsm_client *fingerprint_dclient;
static DEFINE_MUTEX(ic_name_lock);

static struct fp_sensor_info g_fp_sensor_info[] = {
	{ 0x021b, "FPC1021B" },
	{ 0x021f, "FPC1021F" },
	{ 0x0111, "FPC10221" },
	{ 0x0121, "FPC10222" },
	{ 0x1401, "FPC11401" },
	{ 0x140a, "FPC1140A" },
	{ 0x140b, "FPC1140B" },
	{ 0x140c, "FPC1140C" },
	{ 0x0321, "FPC1268X" },
	{ 0x0311, "FPC1265X" },
	{ 0x0711, "FPC1075S" },
	{ 0x0721, "FPC1075T" },
	{ 0x0611, "FPC1023SMA" },
	{ 0x0612, "FPC1023SMB" },
	{ 0x0621, "FPC1023TSC" },
	{ 0x0341, "FPC1266Z120" },
	{ 0x0074, "SYNA109A0" },
	{ 0x3266, "GOODIX3266" },
	{ 0x3288, "GOODIX3288" },
	{ 0x8206, "GOODIX8206" },
	{ 0x5266, "GOODIX5266" },
	{ 0x5296, "GOODIX5296" },
	{ 0x5288, "GOODIX5288" },
	{ 0x1204, "GOODIX1204" },
	{ 0x3658, "GOODIX3658" },
	{ 0x6185, "SILEAD6185" },
	{ 0x6165, "SILEAD6165" },
	{ 0x6175, "SILEAD6275" },
	{ 0x1500, "QFP1500" },
};
#endif
#define HWLOG_TAG fingerprint
HWLOG_REGIST();

/*
 * tui_flg will be set to 1 when tui init(which mean tui is displaying)
 * set to 0 when tui exit
 * This will let fp hal do not read from rst gpio,just return 1
 */
static int tui_flg;
/*lint -save -e* */
static DEFINE_MUTEX(tui_flg_lock);
/*lint -restore*/
extern unsigned int runmode_is_factory(void);

/*
 * fp_ready_flg will be set to 1 when fp hal has init fp sensor succeed
 * This will tell tui_fp_notify when to send msg to fp hal
 */
static int fp_ready_flg;

/*
 * When tui is displaying, the fingerprint sensor will response as a fiq
 * in this situation fp_fiq_flg will be set to 1
 * When hal check gpio, if fp_fiq_flg is 1 then just return irq=1,
 * else return irq=0
 */
static int fp_fiq_flg;

/* extern ldo power supply for fingerprint */
struct regulator *fp_ex_regulator;
#define FINGERPRINT_EXTERN_LDO_NUM "fingreprint_ldo"
#define FINGERPRINT_EXTERN_LDO_NAME "EXTERN_LDO"
#define SUB_LDO "SUB_PMIC_LDO"
#define HBM_WAIT_TIMEOUT (550 * HZ / 1000)
extern unsigned int get_pd_charge_flag(void);
extern int tp_gpio_num;

/*
 * sysf node to check the interrupt status of the sensor, the interrupt
 * handler should perform sysf_notify to allow userland to poll the node.
 */
static ssize_t irq_get(struct device *device,
	struct device_attribute *attribute, char *buffer)
{
	int irq;
	struct fp_data *fp = dev_get_drvdata(device);

	if (fp == NULL)
		return -EINVAL;

	if (fp->use_tp_irq == USE_TP_IRQ) {
		pr_err("%s: USE_TP_IRQ\n", __func__);
		return -EINVAL;
	}
	hwlog_info("tui_flg is %d, fp_ready_flg is %d, fp_fiq_flg is %d\n",
		tui_flg, fp_ready_flg, fp_fiq_flg);

	mutex_lock(&tui_flg_lock);
	if (tui_flg == false) {
		irq = gpio_get_value(fp->irq_gpio);
	} else {
		if (!fp_fiq_flg) {
			irq = IRQ_GPIO_LOW;
		} else {
			irq = IRQ_GPIO_HIGH;
			fp_fiq_flg = false;
		}
	}
	mutex_unlock(&tui_flg_lock);
	return scnprintf(buffer, PAGE_SIZE, "%i\n", irq);
}

/*
 * writing to the irq node will just drop a printk message
 * and return success, used for latency measurement.
 */
static ssize_t irq_ack(struct device *device,
	struct device_attribute *attribute, const char *buffer, size_t count)
{
	return count;
}

static ssize_t result_show(struct device *device,
	struct device_attribute *attribute, char *buffer)
{
	struct fp_data *fp = dev_get_drvdata(device);

	if (fp == NULL)
		return -EINVAL;

	return scnprintf(buffer, PAGE_SIZE, "%lu", fp->finger_num);
}

static ssize_t result_store(struct device *device,
	struct device_attribute *attribute, const char *buffer, size_t count)
{
	unsigned long val = 0;
	struct fp_data *fp = dev_get_drvdata(device);

	if (fp == NULL)
		return -EINVAL;
	if (kstrtoul(buffer, DECIMAL_BASE, &val))
		return -EINVAL;
	fp->finger_num = val;
	sysfs_notify(&fp->pf_dev->dev.kobj, NULL, "result");
	return count;
}

static ssize_t read_image_flag_show(struct device *device,
	struct device_attribute *attribute, char *buffer)
{
	struct fp_data *fp = dev_get_drvdata(device);

	if (fp == NULL)
		return -EINVAL;

	return scnprintf(buffer, PAGE_SIZE, "%d",
		fp->read_image_flag ? true : false);
}

static ssize_t read_image_flag_store(struct device *device,
	struct device_attribute *attribute, const char *buffer, size_t count)
{
	unsigned long val = 0;
	struct fp_data *fp = dev_get_drvdata(device);

	if (fp == NULL)
		return -EINVAL;
	if (kstrtoul(buffer, DECIMAL_BASE, &val))
		return -EINVAL;
	fp->read_image_flag = (bool)val;
	return count;
}

static ssize_t irq_sensorhub_enabled_show(struct device *device,
	struct device_attribute *attribute, char *buffer)
{
	struct fp_data *fp = dev_get_drvdata(device);

	if (fp == NULL)
		return -EINVAL;

	return scnprintf(buffer, PAGE_SIZE, "%d",
		fp->irq_sensorhub_enabled ? IRQ_ENABLE : IRQ_DISABLE);
}

static ssize_t irq_sensorhub_enabled_store(struct device *device,
	struct device_attribute *attribute, const char *buffer, size_t count)
{
	unsigned long val = 0;
	struct fp_data *fp = dev_get_drvdata(device);

	if (fp == NULL)
		return -EINVAL;
	if (kstrtoul(buffer, DECIMAL_BASE, &val))
		return -EINVAL;
	fp->irq_sensorhub_enabled = (bool)val;
	hwlog_info("%s irq_sensorhub_enabled = %d\n", __func__,
		(int)fp->irq_sensorhub_enabled);
	return count;
}

static ssize_t test_power_en_show(struct device *device,
	struct device_attribute *attribute, char *buffer)
{
	struct fp_data *fp = dev_get_drvdata(device);
	int status;

	if (fp == NULL)
		return -EINVAL;
	if (fp->power_en_gpio == (-EINVAL))
		return scnprintf(buffer, PAGE_SIZE, "%d", (-EIO));

	status = gpio_get_value_cansleep(fp->power_en_gpio);
	return scnprintf(buffer, PAGE_SIZE, "%d", status);
}

static ssize_t test_power_en_store(struct device *device,
	struct device_attribute *attribute, const char *buffer, size_t count)
{
	struct fp_data *fp = dev_get_drvdata(device);
	int ret;
	unsigned long power_en_value = FP_POWER_DISABLE;

	if ((fp == NULL) || (fp->power_en_gpio == (-EINVAL)))
		return -EIO;

	if (!runmode_is_factory()) /* if normal version just go out */
		return -EIO;

	if (kstrtoul(buffer, DECIMAL_BASE, &power_en_value))
		return -EINVAL;
	hwlog_err("%s power_en_value = %lu\n", __func__, power_en_value);
	if ((power_en_value != FP_POWER_DISABLE) &&
		(power_en_value != FP_POWER_ENABLE))
		return -EINVAL;

	if (power_en_value == FP_POWER_DISABLE)
		/* wait for TA actions finished then power off */
		msleep(50); /* 50ms */

	ret = gpio_direction_output(fp->power_en_gpio, power_en_value);
	if (ret) {
		hwlog_err("%s gpio_direction_output failed\n", __func__);
		return ret;
	}

	return count;
}

static ssize_t nav_show(struct device *device,
	struct device_attribute *attribute, char *buffer)
{
	struct fp_data *fp = dev_get_drvdata(device);

	if (fp == NULL)
		return -EINVAL;

	return scnprintf(buffer, PAGE_SIZE, "%d", fp->nav_stat);
}

static ssize_t nav_store(struct device *device,
	struct device_attribute *attribute, const char *buffer, size_t count)
{
	unsigned long val = 0;
	struct fp_data *fp = dev_get_drvdata(device);

	if (fp == NULL)
		return -EINVAL;
	if (kstrtoul(buffer, DECIMAL_BASE, &val))
		return -EINVAL;
	fp->nav_stat = val;

	return count;
}

static ssize_t module_id_show(struct device *device,
	struct device_attribute *attribute, char *buffer)
{
	struct fp_data *fp = dev_get_drvdata(device);

	if (fp == NULL)
		return -EINVAL;
	return scnprintf(buffer, MAX_MODULE_ID_LEN, "%s", fp->module_id);
}

static ssize_t module_id_store(struct device *device,
	struct device_attribute *attribute, const char *buffer, size_t count)
{
	struct fp_data *fp = dev_get_drvdata(device);

	if (fp == NULL)
		return -EINVAL;
	strncpy(fp->module_id, buffer, MAX_MODULE_ID_LEN - 1);
	fp->module_id[MAX_MODULE_ID_LEN - 1] = '\0';
	return count;
}

static ssize_t module_id_ud_show(struct device *device,
	struct device_attribute *attribute, char *buffer)
{
	struct fp_data *fp = dev_get_drvdata(device);

	if (buffer == NULL || fp == NULL)
		return -EINVAL;
	return scnprintf(buffer, MAX_MODULE_ID_UD_LEN, "%s", fp->module_id_ud);
}

static ssize_t module_id_ud_store(struct device *device,
	struct device_attribute *attribute, const char *buffer, size_t count)
{
	struct fp_data *fp = dev_get_drvdata(device);

	if (fp == NULL)
		return -EINVAL;
	strncpy(fp->module_id_ud, buffer, MAX_MODULE_ID_UD_LEN - 1);
	fp->module_id_ud[MAX_MODULE_ID_UD_LEN - 1] = '\0';
	return count;
}

static ssize_t low_temperature_show(struct device *device,
	struct device_attribute *attribute, char *buffer)
{
	bool temperature_status = false;

	if (buffer == NULL)
		return -EINVAL;
	temperature_status = is_low_temprature();
	return scnprintf(buffer, PAGE_SIZE, "%d", temperature_status);
}

static ssize_t low_temperature_store(struct device *device,
	struct device_attribute *attribute, const char *buffer, size_t count)
{
	return count;
}

static DEVICE_ATTR(irq, 0600, irq_get, irq_ack);
static DEVICE_ATTR(result, 0600, result_show, result_store);
static DEVICE_ATTR(read_image_flag, 0600, read_image_flag_show,
	read_image_flag_store);
static DEVICE_ATTR(test_power_en, 0600, test_power_en_show,
	test_power_en_store);
static DEVICE_ATTR(nav, 0660, nav_show, nav_store);
static DEVICE_ATTR(module_id, 0660, module_id_show, module_id_store);
static DEVICE_ATTR(irq_enabled, 0600, irq_sensorhub_enabled_show,
	irq_sensorhub_enabled_store);
static DEVICE_ATTR(module_id_ud, 0660, module_id_ud_show, module_id_ud_store);
static DEVICE_ATTR(low_temperature, 0660, low_temperature_show,
	low_temperature_store);

static int fingerprint_get_module_name(struct fp_data *fp,
	char *module, unsigned int len)
{
	int ret;
	unsigned int index;
	const char *module_info = NULL;
	struct device_node *np = NULL;
	static const char * const module_str[] = {
		"fingerprint,moduleid_low",
		"fingerprint,moduleid_high",
		"fingerprint,moduleid_float"
	};

	if (fp->dev == NULL || fp->dev->of_node == NULL) {
		hwlog_err("%s failed,the pointer is null\n", __func__);
		return -EINVAL;
	}

	np = fp->dev->of_node;
	index = fp->module_vendor_info;

	switch (index) {
	case MODULEID_LOW:
	case MODULEID_HIGT:
	case MODULEID_FLOATING:
		ret = of_property_read_string(np, module_str[index],
			&module_info);
		if (ret) {
			hwlog_err("%s failed to get %s from device tree\n",
				__func__, module_str[index]);
			break;
		}
		strncpy(module, module_info, len - 1);
		break;
	default:
		ret = FP_RETURN_SUCCESS;
		strncpy(module, "NN", FP_DEFAULT_INFO_LEN);
		break;
	}
	*(module + len - 1) = '\0';
	return ret;
}

static char *fingerprint_get_sensor_name(unsigned int sensor_id)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g_fp_sensor_info); i++) {
		if (sensor_id == g_fp_sensor_info[i].sensor_id) {
			hwlog_info("sensor_name:%s\n",
			g_fp_sensor_info[i].sensor_name);
			return g_fp_sensor_info[i].sensor_name;
		}
	}
	hwlog_err("sensor_name not found, sensor_id = %d\n", sensor_id);
	return NULL;
}

#if defined(CONFIG_HUAWEI_DSM)
static void fingerprint_update_vendor_info(struct fp_data *fp)
{
	int ret;
	char *ic_name = NULL;
	char module_name[FP_MAX_MODULE_INFO_LEN] = {0};

	if (fp == NULL) {
		hwlog_err("the pointer is null");
		return;
	}
	mutex_lock(&ic_name_lock);
	ret = fingerprint_get_module_name(fp, module_name,
		FP_MAX_MODULE_INFO_LEN);
	if (ret == FP_RETURN_SUCCESS)
		dsm_fingerprint.module_name = module_name;

	ic_name = fingerprint_get_sensor_name(fp->sensor_id);
	if (ic_name) {
		dsm_fingerprint.ic_name = ic_name;
		if (dsm_update_client_vendor_info(&dsm_fingerprint))
			hwlog_err("dsm update client_vendor_info is failed\n");
	} else {
		ic_name = kzalloc(FP_MAX_SENSOR_ID_LEN, GFP_KERNEL);
		if (ic_name == NULL) {
			hwlog_err("malloc failed\n");
			mutex_unlock(&ic_name_lock);
			return;
		}
		snprintf(ic_name, FP_MAX_SENSOR_ID_LEN, "%x", fp->sensor_id);
		dsm_fingerprint.ic_name = ic_name;
			if (dsm_update_client_vendor_info(&dsm_fingerprint))
				hwlog_err("dsm update client_vendor_info is failed\n");
		kfree(ic_name);
		dsm_fingerprint.ic_name = NULL;
	}
	mutex_unlock(&ic_name_lock);
}
#endif

static ssize_t chip_info_show(struct device *device,
	unsigned int sensor_type, char *chip_info)
{
	char *ic_name = NULL;
	unsigned int sensor_id;
	int ret;
	char module_name[FP_MAX_MODULE_INFO_LEN] = {0};
	struct fp_data *fp = NULL;

	if (device == NULL || chip_info == NULL) {
		hwlog_err("%s failed,the pointer is null\n", __func__);
		return -EINVAL;
	}
	fp = dev_get_drvdata(device);
	if (fp == NULL) {
		hwlog_err("%s failed,the parameters is null\n", __func__);
		return -EINVAL;
	}

	if (sensor_type == COMMON) {
		sensor_id = fp->sensor_id;
	} else if (sensor_type == OPTICAL) {
		sensor_id = fp->sensor_id_ud;
	} else {
		hwlog_err("%s: sensor_type = %d, not found\n", __func__,
			sensor_type);
		return -EFAULT;
	}

	ret = fingerprint_get_module_name(fp, module_name,
		FP_MAX_MODULE_INFO_LEN);
	if (ret != FP_RETURN_SUCCESS)
		strncpy(module_name, "NN", FP_DEFAULT_INFO_LEN);

	ic_name = fingerprint_get_sensor_name(sensor_id);
	if (ic_name != NULL) {
		ret = snprintf(chip_info, FP_MAX_SENSOR_ID_LEN, "%s--%s\n",
			ic_name, module_name);
		hwlog_info("chip_info:%s, %s\n", chip_info, __func__);
	} else {
		hwlog_info("hw_debug didn't find fingerprint:%x, %s\n",
			sensor_id, __func__);
		ret = snprintf(chip_info, FP_MAX_SENSOR_ID_LEN, "%x--%s\n",
			sensor_id, module_name);
	}

	if (ret >= FP_MAX_SENSOR_ID_LEN) {
		hwlog_err("%s: chip_info truncated to '%s'", __func__,
			chip_info);
		return -EFAULT;
	}

	return ret;
}

static ssize_t fingerprint_chip_info_show(struct device *device,
	struct device_attribute *attribute, char *buf)
{
	hwlog_info("fingerprint_chip_info_show\n");
	return chip_info_show(device, COMMON, buf);
}

static ssize_t ud_fingerprint_chip_info_show(struct device *device,
	struct device_attribute *attribute, char *buf)
{
	hwlog_info("ud_fingerprint_chip_info_show\n");
	return chip_info_show(device, OPTICAL, buf);
}

static DEVICE_ATTR(fingerprint_chip_info, 0440,
	fingerprint_chip_info_show, NULL);
static DEVICE_ATTR(ud_fingerprint_chip_info, 0440,
	ud_fingerprint_chip_info_show, NULL);

static struct attribute *attributes[] = {
	&dev_attr_irq.attr,
	&dev_attr_fingerprint_chip_info.attr,
	&dev_attr_result.attr,
	&dev_attr_read_image_flag.attr,
	&dev_attr_test_power_en.attr,
	&dev_attr_nav.attr,
	&dev_attr_module_id.attr,
	&dev_attr_irq_enabled.attr,
	&dev_attr_module_id_ud.attr,
	&dev_attr_low_temperature.attr,
	&dev_attr_ud_fingerprint_chip_info.attr,
	NULL
};

static const struct attribute_group attribute_group = {
	.attrs = attributes,
};

static struct fp_data *g_fingerprint;

int tui_fp_notify(void)
{
	struct fp_data *fp = g_fingerprint;

	if (!fp) {
		hwlog_err("%s failed g_fingerprint is NULL\n", __func__);
		return -EINVAL;
	}
	if (!fp_ready_flg)
		return FP_RETURN_SUCCESS;

	hwlog_info("%s: fp is ready\n", __func__);
	fp_fiq_flg = true;

	/* MEMORY_BARRIER */
	smp_rmb();

	if (fp->wakeup_enabled)
		__pm_wakeup_event(&fp->ttw_wl, FPC_TTW_HOLD_TIME);

	sysfs_notify(&fp->pf_dev->dev.kobj, NULL, dev_attr_irq.attr.name);
	return FP_RETURN_SUCCESS;
}
EXPORT_SYMBOL(tui_fp_notify);

void ud_fp_on_hbm_completed(void)
{
	struct fp_data *fp = g_fingerprint;

	if (fp == NULL)
		return;

	fp->hbm_status = HBM_ON;
	wake_up(&fp->hbm_queue);
}

void fp_set_lcd_charge_time(int time)
{
	struct fp_data *fp = g_fingerprint;

	if (fp == NULL)
		return;

	fp->fingerprint_bigdata.lcd_charge_time = time;
}

void fp_set_lcd_light_on_time(int time)
{
	struct fp_data *fp = g_fingerprint;

	if (fp == NULL)
		return;

	fp->fingerprint_bigdata.lcd_on_time = time;
}

void fp_set_cpu_wake_up_time(int time)
{
	struct fp_data *fp = g_fingerprint;

	if (fp == NULL)
		return;

	fp->fingerprint_bigdata.cpu_wakeup_time = time;
}

#define PEN_ANTI_TIME_MS 500
#define NAV_ANTI_TIME_MS 500
static unsigned long last_pen_stamp;
static unsigned long last_nav_stamp;
static atomic_t g_pen_in_range_status = ATOMIC_INIT(TS_PEN_OUT_RANGE);
static struct notifier_block pen_nb;
/* Define in vendor/huawei/chipset_common/devkit/tpkit/huawei_ts_kit_core.c */
/* for ts status notify */
#if defined(CONFIG_HUAWEI_TS_KIT) || defined(CONFIG_HUAWEI_TS_KIT_3_0)
extern int ts_event_notifier_register(struct notifier_block *nb);
extern int ts_event_notifier_unregister(struct notifier_block *nb);
#else
static int ts_event_notifier_register(struct notifier_block *nb)
{
	return FP_RETURN_SUCCESS;
}
static int ts_event_notifier_unregister(struct notifier_block *nb)
{
	return FP_RETURN_SUCCESS;
}
#endif

static void update_last_nav_event_time_stamp(int key)
{
	switch (key) {
	case EVENT_LEFT:
	case EVENT_RIGHT:
	case EVENT_UP:
	case EVENT_DOWN:
	case EVENT_HOLD:
	case EVENT_CLICK:
		last_nav_stamp = jiffies;
		break;
	default:
		break;
	}
}

/* true filter current event */
static bool fingerprint_check_pen_anti_time(unsigned long lst_pen,
	unsigned long lst_nav)
{
	unsigned long pen_anti_jiffies = lst_pen +
		msecs_to_jiffies(PEN_ANTI_TIME_MS);
	unsigned long nav_anti_jiffies = lst_nav +
		msecs_to_jiffies(NAV_ANTI_TIME_MS);

	if (time_before(jiffies, nav_anti_jiffies))
		hwlog_info("%s nav time not anti\n", __func__);

	/* in PEN_ANTI_TIME_MS time */
	if (time_before(jiffies, pen_anti_jiffies)) {
		hwlog_info("%s pen time antied\n", __func__);
		return true;
	}
	return false;
}

static int stylus_pen_notify_fp(struct notifier_block *nb,
	unsigned long event, void *data)
{
	int in_range_status = (int)event;
	int lst_in_range_status = atomic_read(&g_pen_in_range_status);
	struct fp_data *fp = g_fingerprint;

	if (!nb) {
		hwlog_err("%s failed nb is NULL\n", __func__);
		return -EINVAL;
	}
	if (!fp) {
		hwlog_err("%s failed g_fingerprint is NULL\n", __func__);
		return -EINVAL;
	}
	if (fp->pen_anti_enable == false)
		return FP_RETURN_SUCCESS;

	if ((in_range_status < TS_PEN_OUT_RANGE) ||
		(in_range_status > TS_PEN_IN_RANGE))
		return FP_RETURN_SUCCESS;

	/* state not change, do nothing */
	if (lst_in_range_status == in_range_status)
		return FP_RETURN_SUCCESS;

	hwlog_info("%s lst_in_range_status = %d, in_range_status = %d\n",
		__func__, lst_in_range_status, in_range_status);
	/* record pen status changed time jiffies, for anti touch use */
	last_pen_stamp = jiffies;
	atomic_set(&g_pen_in_range_status, in_range_status);
	return FP_RETURN_SUCCESS;
}

static irqreturn_t fingerprint_irq_handler(int irq, void *handle)
{
	struct fp_data *fp = handle;
	int irq_gpio;

	/* MEMORY_BARRIER */
	smp_rmb();

	if (fp->use_tp_irq == USE_TP_IRQ) {
		pr_err("%s: USE_TP_IRQ\n", __func__);
		return IRQ_HANDLED;
	}
	if (fp->wakeup_enabled)
		__pm_wakeup_event(&fp->ttw_wl, FPC_TTW_HOLD_TIME);

	if (fp->irq_custom_scheme == FP_IRQ_SCHEME_ONE) {
		irq_gpio = gpio_get_value(fp->irq_gpio);
		if (irq_gpio == IRQ_GPIO_LOW)
			return IRQ_HANDLED;
	}

	sysfs_notify(&fp->pf_dev->dev.kobj, NULL,
		dev_attr_irq.attr.name);
	return IRQ_HANDLED;
}

void fingerprint_get_navigation_adjustvalue(struct device *dev,
	struct fp_data *fp_data)
{
	struct device_node *np = NULL;
	int adjust1 = NAVIGATION_ADJUST_NOREVERSE;
	int adjust2 = NAVIGATION_ADJUST_NOTURN;

	if (!dev || !dev->of_node) {
		hwlog_err("%s failed dev or dev node is NULL\n", __func__);
		return;
	}

	if (!fp_data) {
		hwlog_err("%s failed fp_data is NULL\n", __func__);
		return;
	}

	np = dev->of_node;

	(void)of_property_read_u32(np, "fingerprint,navigation_adjust1",
		(unsigned int *)(&adjust1));

	if (adjust1 != NAVIGATION_ADJUST_NOREVERSE &&
		adjust1 != NAVIGATION_ADJUST_REVERSE) {
		adjust1 = NAVIGATION_ADJUST_NOREVERSE;
		hwlog_err("%s navigation_adjust1 set err only support 0 and 1\n",
			__func__);
	}

	(void)of_property_read_u32(np, "fingerprint,navigation_adjust2",
		(unsigned int *)(&adjust2));

	if (adjust2 != NAVIGATION_ADJUST_NOTURN &&
		adjust2 != NAVIGATION_ADJUST_TURN90 &&
		adjust2 != NAVIGATION_ADJUST_TURN180 &&
		adjust2 != NAVIGATION_ADJUST_TURN270) {
		adjust2 = NAVIGATION_ADJUST_NOTURN;
		hwlog_err("%s navigation_adjust2 set err only support 0 90 180 and 270\n",
			__func__);
	}

	fp_data->navigation_adjust1 = adjust1;
	fp_data->navigation_adjust2 = adjust2;

	hwlog_info("%s get navigation_adjust1 = %d, navigation_adjust2 = %d\n",
		__func__, fp_data->navigation_adjust1,
		fp_data->navigation_adjust2);
}

static int fingerprint_get_power_supply_config(struct device *dev,
	struct fp_data *fp_data)
{
	int result;
	struct device_node *np = NULL;
	struct fp_data *priv = NULL;

	if ((dev == NULL) || (dev->of_node == NULL) || (fp_data == NULL)) {
		hwlog_err("%s failed NULL pointer error\n", __func__);
		return -EINVAL;
	}

	priv = dev_get_drvdata(dev);
	np = dev->of_node;
	if (strncmp(priv->extern_ldo_name, SUB_LDO, strlen(SUB_LDO)) == 0) {
		fp_data->power_en_gpio = -EINVAL;
		hwlog_info("%s extern_ldo_name using sub_pmic_ldo\n", __func__);
		result = of_property_read_u32(np, "fingerprint,sub_pmic_num",
			(unsigned int *)(&fp_data->sub_pmic_num));
		if (result) {
			fp_data->sub_pmic_num = -EINVAL;
			hwlog_info("%s failed to get sub_pmic_num\n", __func__);
		}
		result = of_property_read_u32(np, "fingerprint,sub_ldo_num",
			(unsigned int *)(&fp_data->sub_ldo_num));
		if (result) {
			fp_data->sub_ldo_num = -EINVAL;
			hwlog_info("%s failed to get sub_ldo_num\n", __func__);
		}
	} else {
		hwlog_info("%s not using sub_pmic_ldo\n", __func__);
		result = of_property_read_u32(np, "fingerprint,extern_ldo_num",
			(unsigned int *)(&fp_data->extern_ldo_num));
		if (result) {
			fp_data->extern_ldo_num = -EINVAL;
			hwlog_info("%s failed to get extern_ldo_num\n",
				__func__);
		}
		result = of_property_read_u32(np, "fingerprint,extern_vol",
			(unsigned int *)(&fp_data->extern_vol));
		if (result) {
			fp_data->extern_vol = -EINVAL;
			hwlog_info("%s failed to get extern_vol\n", __func__);
		}
	}
	return result;
}

static int fingerprint_get_gpio_rst_cs(struct device_node *np,
	struct fp_data *fp_data)
{
	int ret;

	ret = of_property_read_u32(np, "fingerprint,reset_gpio",
		(unsigned int *)(&fp_data->rst_gpio));
	if (ret) {
		fp_data->rst_gpio = -EINVAL;
		hwlog_err("%s failed to get reset gpio from device tree\n",
			__func__);
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "fingerprint,reset_gpio_ud",
		(unsigned int *)(&fp_data->rst1_gpio));
	if (ret) {
		fp_data->rst1_gpio = -EINVAL;
		hwlog_err("%s failed to get rst1_gpio from device tree\n",
			__func__);
	}

	ret = of_property_read_u32(np, "fingerprint,cs_gpio",
		(unsigned int *)(&fp_data->cs0_gpio));
	if (ret) {
		fp_data->cs0_gpio = -EINVAL;
		hwlog_err("%s failed to get cs0_gpio from device tree\n",
			__func__);
	}

	ret = of_property_read_u32(np, "fingerprint,cs_gpio_ud",
		(unsigned int *)(&fp_data->cs1_gpio));
	if (ret) {
		fp_data->cs1_gpio = -EINVAL;
		hwlog_err("%s failed to get cs gpio ud from device tree\n",
			__func__);
	}

	return FP_RETURN_SUCCESS;
}

static int fingerprint_get_gpio_irq_power(struct device_node *np,
	struct fp_data *fp_data)
{
	int ret;

	fp_data->use_tp_irq = USE_SELF_IRQ;
	ret = of_property_read_u32(np,
		"fingerprint,use_tp_irq", &(fp_data->use_tp_irq));
	if (ret) {
		fp_data->use_tp_irq = USE_SELF_IRQ;
		hwlog_info("%s failed to get use_tp_irq\n", __func__);
	}

	ret = of_property_read_u32(np, "fingerprint,irq_gpio",
		(unsigned int *)(&fp_data->irq_gpio));
	if (ret) {
		if (fp_data->use_tp_irq == USE_SELF_IRQ) {
			fp_data->irq_gpio = -EINVAL;
			hwlog_err("%s failed to get irq gpio from device tree\n",
				__func__);
			return -EINVAL;
		}
	}

	if (of_property_read_u32(np, "fingerprint,power_en_gpio",
		(unsigned int *)(&fp_data->power_en_gpio))) {
		fp_data->power_en_gpio = -EINVAL;
		hwlog_info("%s failed to get power_en_gpio\n", __func__);
	}

	return FP_RETURN_SUCCESS;
}

static void fingerprint_get_gpio_ldo_product(struct device *dev,
	struct device_node *np, struct fp_data *fp_data)
{
	int ret;
	const char *product = NULL;
	const char *ldo_info = NULL;
	int name_len;

	ret = of_property_read_string(np,
		"fingerprint,extern_ldo_name", &ldo_info);
	if (ret) {
		strncpy(fp_data->extern_ldo_name, "EINVAL", sizeof("EINVAL"));
		hwlog_info("%s failed to get extern_ldo_name\n", __func__);
	} else {
		name_len = sizeof(fp_data->extern_ldo_name);
		strncpy(fp_data->extern_ldo_name, ldo_info, name_len - 1);
		fp_data->extern_ldo_name[name_len - 1] = '\0';
		if (fingerprint_get_power_supply_config(dev, fp_data))
			hwlog_err("%s get_power_supply_config fail\n",
				__func__);
	}

	ret = of_property_read_string(np, "fingerprint,product", &product);
	if (ret) {
		strncpy(fp_data->product_name, "EINVAL", sizeof("EINVAL"));
		hwlog_err("%s failed to get product_name from device tree, just go on\n",
			__func__);
	} else {
		name_len = sizeof(fp_data->product_name);
		strncpy(fp_data->product_name, product, name_len - 1);
		fp_data->product_name[name_len - 1] = '\0';
	}

	fp_data->module_id_gpio = of_get_named_gpio(np,
		"fingerprint,moduleid_gpio", 0);

	if (fp_data->module_id_gpio < 0) {
		ret = of_property_read_u32(np, "fingerprint,moduleid_gpio",
			(unsigned int *)(&fp_data->module_id_gpio));
		if (ret) {
			/* some products didn't support module ID gpio */
			fp_data->module_id_gpio = -EINVAL;
			hwlog_info("%s failed to get moduleid_gpio from device tree\n",
				__func__);
		}
	}

	hwlog_info("%s moduleid_gpio=%d\n", __func__, fp_data->module_id_gpio);
}

int fingerprint_get_dts_data(struct device *dev, struct fp_data *fp_data)
{
	struct device_node *np = NULL;
	int ret;

	hwlog_info("%s enter\n", __func__);
	if (!dev || !dev->of_node) {
		hwlog_err("%s failed dev or dev node is NULL\n", __func__);
		return -EINVAL;
	}

	if (!fp_data) {
		hwlog_err("%s failed fp_data is NULL\n", __func__);
		return -EINVAL;
	}

	np = dev->of_node;

	ret = fingerprint_get_gpio_rst_cs(np, fp_data);
	if (ret == (-EINVAL))
		goto exit;

	ret = fingerprint_get_gpio_irq_power(np, fp_data);
	if (ret == (-EINVAL))
		goto exit;

	fingerprint_get_gpio_ldo_product(dev, np, fp_data);

	ret = of_property_read_u32(np, "fingerprint,pen_anti_enable",
		&(fp_data->pen_anti_enable));
	if (ret) {
		fp_data->pen_anti_enable = false; /* set default not support */
		ret = FP_RETURN_SUCCESS;
		hwlog_info("%s failed to get pen_anti_enable from device tree, just go on\n",
			__func__);
	}

	ret = of_property_read_u32(np, "fingerprint,cts_home",
		(unsigned int *)&(fp_data->cts_home));
	if (ret) {
		/* set default not support for three key home */
		fp_data->cts_home = false;
		ret = FP_RETURN_SUCCESS;
		hwlog_info("%s failed to get cts_home from device tree, just go on\n",
			__func__);
	}

	ret = of_property_read_u32(np, "fingerprint,irq_custom_scheme",
		&(fp_data->irq_custom_scheme));
	if (ret) {
		/* set 0 as common config, no special deal */
		fp_data->irq_custom_scheme = 0;
		hwlog_info("%s failed to get irq_custom_scheme from device tree, just go on\n",
			__func__);
	}
#if defined(CONFIG_FB)
	ret = of_property_read_u32(np, "fingerprint,notify_enable",
		&(fp_data->notify_enable));
	if (ret) {
		fp_data->notify_enable = FP_UNBLANK_NOTIFY_DISABLE;
		hwlog_info("%s failed to get notify_enable from device tree, go on\n",
			__func__);
	}
#endif
	return FP_RETURN_SUCCESS;

exit:
	return -EINVAL;
}

static int fingerprint_key_remap_reverse(int key)
{
	switch (key) {
	case EVENT_LEFT:
		key = EVENT_RIGHT;
		break;
	case EVENT_RIGHT:
		key = EVENT_LEFT;
		break;
	default:
		break;
	}

	return key;
}

static int fingerprint_key_remap_turn90(int key)
{
	switch (key) {
	case EVENT_LEFT:
		key = EVENT_UP;
		break;
	case EVENT_RIGHT:
		key = EVENT_DOWN;
		break;
	case EVENT_UP:
		key = EVENT_RIGHT;
		break;
	case EVENT_DOWN:
		key = EVENT_LEFT;
		break;
	default:
		break;
	}

	return key;
}

static int fingerprint_key_remap_turn180(int key)
{
	switch (key) {
	case EVENT_LEFT:
		key = EVENT_RIGHT;
		break;
	case EVENT_RIGHT:
		key = EVENT_LEFT;
		break;
	case EVENT_UP:
		key = EVENT_DOWN;
		break;
	case EVENT_DOWN:
		key = EVENT_UP;
		break;
	default:
		break;
	}

	return key;
}

static int fingerprint_key_remap_turn270(int key)
{
	switch (key) {
	case EVENT_LEFT:
		key = EVENT_DOWN;
		break;
	case EVENT_RIGHT:
		key = EVENT_UP;
		break;
	case EVENT_UP:
		key = EVENT_LEFT;
		break;
	case EVENT_DOWN:
		key = EVENT_RIGHT;
		break;
	default:
		break;
	}

	return key;
}

static int fingerprint_key_remap(struct fp_data *fp, int key)
{
	if (key != EVENT_RIGHT && key != EVENT_LEFT && key != EVENT_UP &&
		key != EVENT_DOWN)
		return key;

	if (fp->navigation_adjust1 == NAVIGATION_ADJUST_REVERSE)
		key = fingerprint_key_remap_reverse(key);

	switch (fp->navigation_adjust2) {
	case NAVIGATION_ADJUST_TURN90:
		key = fingerprint_key_remap_turn90(key);
		break;
	case NAVIGATION_ADJUST_TURN180:
		key = fingerprint_key_remap_turn180(key);
		break;
	case NAVIGATION_ADJUST_TURN270:
		key = fingerprint_key_remap_turn270(key);
		break;
	default:
		break;
	}

	return key;
}

static void fingerprint_input_report(struct fp_data *fp, int event)
{
	int lst_in_range_status = (int)TS_PEN_OUT_RANGE;

	if (fp->pen_anti_enable) {
		lst_in_range_status = atomic_read(&g_pen_in_range_status);
		if (lst_in_range_status == TS_PEN_IN_RANGE) {
			update_last_nav_event_time_stamp(event);
			hwlog_info("%s event=%d, filter by pen, pen=%d\n",
				__func__, event, lst_in_range_status);
			return;
		}
		/* current status is pen out of range */
		if (fingerprint_check_pen_anti_time(last_pen_stamp,
			last_nav_stamp)) {
			update_last_nav_event_time_stamp(event);
			hwlog_info("%s event=%d, filter by pen, pen=%d\n",
				__func__, event, lst_in_range_status);
			return;
		}
		update_last_nav_event_time_stamp(event);
	}

	event = fingerprint_key_remap(fp, event);
	hwlog_info("%s event=%d\n", __func__, event);
	input_report_key(fp->input_dev, event, true);
	input_sync(fp->input_dev);
	input_report_key(fp->input_dev, event, false);
	input_sync(fp->input_dev);
}

static int fingerprint_open(struct inode *inode, struct file *file)
{
	struct fp_data *fp = NULL;

	hwlog_info("%s\n", __func__);
	fp = container_of(inode->i_cdev, struct fp_data, cdev);
	file->private_data = fp;
	return FP_RETURN_SUCCESS;
}

static ssize_t fingerprint_write(struct file *file, const char *buff,
	size_t count, loff_t *ppos)
{
	hwlog_info("%s\n", __func__);
	return count;
}

static ssize_t fingerprint_read(struct file *file, char *buff,
	size_t count, loff_t *ppos)
{
	hwlog_info("%s\n", __func__);
	return FP_RETURN_SUCCESS;
}

static int fingerprint_get_irq_status(struct fp_data *fp)
{
	int status;

	status = gpio_get_value_cansleep(fp->irq_gpio);
	return status;
}

static int fingerprint_ipc_wakelock_status(unsigned int value)
{
	struct fp_data *fp = g_fingerprint;

	if (!fp) {
		hwlog_err("%s:error:g_fingerprint is NULL\n", __func__);
		return -EINVAL;
	}

	if (value)
		pm_stay_awake(fp->dev);
	else
		pm_relax(fp->dev);

	return FP_RETURN_SUCCESS;
}

static void fingerprint_gpio_direction_output(int fp_gpio, int value)
{
	int ret;

	if (fp_gpio != -EINVAL) {
		ret = gpio_direction_output(fp_gpio, value);
		if (ret)
			hwlog_err("%s gpio_direction_output %d failed\n",
				__func__, fp_gpio);
	} else {
		hwlog_err("%s fp_gpio %d is not expected value\n",
			__func__, fp_gpio);
	}
}

/*
 * some device the ldo can not be closed,
 * so there is a loadswitch(gpio) to control the power of fingerprint,
 * and now close the loadswitch
 */
static void fingerprint_poweroff_close_loadswtich(struct fp_data *fp)
{
	hwlog_info("%s enter\n", __func__);
	if ((fp == NULL) || (fp->power_en_gpio == (-EINVAL))) {
		hwlog_err("%s fail\n", __func__);
		return;
	}
	fingerprint_gpio_direction_output(fp->power_en_gpio, GPIO_LOW_LEVEL);
}

static void fingerprint_sensor_abnormal_poweroff(struct fp_data *fp)
{
	int ret;
	unsigned int poweroff_scheme = 0;

	ret = of_property_read_u32(fp->dev->of_node,
		"fingerprint,poweroff_scheme", &poweroff_scheme);
	if (ret) {
		hwlog_info("%s failed to get poweroff_scheme from device tree, just go on\n",
			__func__);
		return;
	}
	/*
	 * when the sensor is abnormal,
	 * the loadswtich maybe cause leakage of current,
	 * so should close the loadswtich
	 */
	if (poweroff_scheme == FP_POWEROFF_SCHEME_TWO)
		fingerprint_poweroff_close_loadswtich(fp);
}

static void fp_ioc_enable_irq(struct fp_data *fp)
{
	hwlog_info("fingerprint_ioctl FP_IOC_CMD_ENABLE_IRQ\n");
	if (fp->use_tp_irq == USE_TP_IRQ) {
		pr_err("%s: USE_TP_IRQ\n", __func__);
		return;
	}
	fp_ready_flg = true;
	if (fp->ttw_wl.active)
		__pm_relax(&fp->ttw_wl);
	if (fp->irq_enabled == true) {
		hwlog_info("%s fp irq already enable", __func__);
		return;
	}
	enable_irq(gpio_to_irq(fp->irq_gpio));
	fp->irq_enabled = true;
}

static void fp_ioc_disable_irq(struct fp_data *fp)
{
	hwlog_info("fingerprint_ioctl FP_IOC_CMD_DISABLE_IRQ\n");
	if (fp->use_tp_irq == USE_TP_IRQ) {
		pr_err("%s: USE_TP_IRQ\n", __func__);
		return;
	}
	fp_ready_flg = false;
	if (fp->irq_enabled == false) {
		hwlog_info("%s fp irq already disable", __func__);
		return;
	}
	disable_irq(gpio_to_irq(fp->irq_gpio));
	fp->irq_enabled = false;
}

static int fp_ioc_send_uevent(struct fp_data *fp, const void __user *argp)
{
	int event = FP_KEY_MIN;

	if (copy_from_user(&event, argp, sizeof(event))) {
		hwlog_err("%s copy_from_user failed\n", __func__);
		return -EFAULT;
	}
	if (event < FP_KEY_MIN || event > FP_KEY_MAX) {
		hwlog_err("%s nav event is %d\n", __func__, event);
		return -EFAULT;
	}

	fingerprint_input_report(fp, event);
	hwlog_info("fingerprint_ioctl FP_IOC_CMD_SEND_UEVENT\n");
	return FP_RETURN_SUCCESS;
}

static int fp_ioc_get_irq_status(struct fp_data *fp, void __user *argp)
{
	int status;
	int ret;

	status = fingerprint_get_irq_status(fp);
	if (status != GPIO_LOW_LEVEL && status != GPIO_HIGH_LEVEL) {
		hwlog_err("%s fingerprint irq is %d\n", __func__, status);
		return -EFAULT;
	}

	ret = copy_to_user(argp, &status, sizeof(status));
	if (ret) {
		hwlog_err("%s copy_to_user failed, ret=%d\n", __func__,
			ret);
		return -EFAULT;
	}

	hwlog_info("fingerprint_ioctl FP_IOC_CMD_GET_IRQ_STATUS status=%d\n",
		status);
	return FP_RETURN_SUCCESS;
}

static int fp_ioc_set_wakelock_status(struct fp_data *fp,
	const void __user *argp)
{
	int value = false;

	if (copy_from_user(&value, argp, sizeof(value))) {
		hwlog_err("%s copy_from_user failed\n", __func__);
		return -EFAULT;
	}

	if (value == true)
		fp->wakeup_enabled = true;
	else
		fp->wakeup_enabled = false;

	hwlog_info("fingerprint_ioctl FP_IOC_CMD_SET_WAKELOCK_STATUS value = %d\n",
		value);
	return FP_RETURN_SUCCESS;
}

static int fp_ioc_send_sensorid(struct fp_data *fp, const void __user *argp)
{
	unsigned int sensor_id = 0;

	if (copy_from_user(&sensor_id, argp, sizeof(sensor_id))) {
		hwlog_err("%s copy_from_user failed\n", __func__);
		return -EFAULT;
	}

	fp->sensor_id = sensor_id;
#if defined(CONFIG_HUAWEI_DSM)
	fingerprint_update_vendor_info(fp);
#endif
	hwlog_info("fingerprint_ioctl FP_IOC_CMD_SEND_SENSORID =%x\n",
		sensor_id);
	return FP_RETURN_SUCCESS;
}

static int fp_ioc_send_sensorid_ud(struct fp_data *fp, const void __user *argp)
{
	unsigned int sensor_id = 0;

	if (copy_from_user(&sensor_id, argp, sizeof(sensor_id))) {
		hwlog_err("%s copy_from_user failed\n", __func__);
		return -EFAULT;
	}

	fp->sensor_id_ud = sensor_id;
	hwlog_info("fingerprint_ioctl FP_IOC_CMD_SEND_SENSORID_UD =%x\n",
		fp->sensor_id_ud);
	return FP_RETURN_SUCCESS;
}

static int fp_ioc_set_ipc_wakelocks(const void __user *argp)
{
	int ret;
	unsigned int set_ipc_wakelock = 0;

	if (copy_from_user(&set_ipc_wakelock, argp,
		sizeof(set_ipc_wakelock))) {
		hwlog_err("%s copy_from_user failed\n", __func__);
		return -EFAULT;
	}
	ret = fingerprint_ipc_wakelock_status(set_ipc_wakelock);
	if (ret < 0)
		hwlog_err("fingerprint set ipc_wakelock failed\n");

	hwlog_info("fingerprint_ioctl FP_IOC_CMD_SET_IPC_WAKELOCKS =%d\n",
		set_ipc_wakelock);
	return ret;
}

static int fp_ioc_check_hbm_status(struct fp_data *fp)
{
	int ret = FP_RETURN_SUCCESS;

	if (wait_event_timeout(fp->hbm_queue,
		fp->hbm_status == HBM_ON, HBM_WAIT_TIMEOUT) <= 0) {
		hwlog_err("fingerprint wait hbm timeout\n");
		ret = -EFAULT;
	}
	fp->hbm_status = HBM_NONE;
	return ret;
}

static int fp_ioc_get_bigdata(struct fp_data *fp, void __user *argp)
{
	int ret;

	ret = copy_to_user(argp, &fp->fingerprint_bigdata,
		sizeof(fp->fingerprint_bigdata));
	if (ret) {
		hwlog_err("%s copy_to_user failed, ret=%d\n",
			__func__, ret);
		return -EFAULT;
	}

	return FP_RETURN_SUCCESS;
}

/* active te single when using fingeprint , fix fingeprint blink question .*/
static int fp_ioc_update_te()
{
	de_open_update_te();
	return FP_RETURN_SUCCESS;
}

static long fingerprint_ioctl(struct file *file, unsigned int cmd,
	unsigned long arg)
{
	int ret = FP_RETURN_SUCCESS;
	struct fp_data *fp = NULL;
	uintptr_t arg_tmp = (uintptr_t)arg;
	void __user *argp = (void __user *)arg_tmp;

	fp = (struct fp_data *)file->private_data;
	if (fp == NULL) {
		hwlog_err("%s fingerprint is NULL\n", __func__);
		return -EFAULT;
	}

	if (_IOC_TYPE(cmd) != FP_IOC_MAGIC)
		return -ENOTTY;

	switch (cmd) {
	case FP_IOC_CMD_ENABLE_IRQ:
		fp_ioc_enable_irq(fp);
		break;

	case FP_IOC_CMD_DISABLE_IRQ:
		fp_ioc_disable_irq(fp);
		break;

	case FP_IOC_CMD_SEND_UEVENT:
		ret = fp_ioc_send_uevent(fp, argp);
		break;

	case FP_IOC_CMD_GET_IRQ_STATUS:
		ret = fp_ioc_get_irq_status(fp, argp);
		break;

	case FP_IOC_CMD_SET_WAKELOCK_STATUS:
		ret = fp_ioc_set_wakelock_status(fp, argp);
		break;

	case FP_IOC_CMD_SEND_SENSORID:
		ret = fp_ioc_send_sensorid(fp, argp);
		break;

	case FP_IOC_CMD_SEND_SENSORID_UD:
		ret = fp_ioc_send_sensorid_ud(fp, argp);
		break;

	case FP_IOC_CMD_SET_IPC_WAKELOCKS:
		ret = fp_ioc_set_ipc_wakelocks(argp);
		break;

	case FP_IOC_CMD_CHECK_HBM_STATUS:
		ret = fp_ioc_check_hbm_status(fp);
		break;

	case FP_IOC_CMD_RESET_HBM_STATUS:
		fp->hbm_status = HBM_NONE;
		break;

	case FP_IOC_CMD_SET_POWEROFF:
		hwlog_info("%s FP_IOC_CMD_SET_POWEROFF\n", __func__);
		fingerprint_sensor_abnormal_poweroff(fp);
		break;

	case FP_IOC_CMD_GET_BIGDATA:
		ret = fp_ioc_get_bigdata(fp, argp);
		break;

	case FP_IOC_CMD_NOTIFY_DISPLAY_FP_DOWN_UD:
		ret = fp_ioc_update_te();
		break;
	default:
		hwlog_err("%s error = -EFAULT\n", __func__);
		ret = -EFAULT;
		break;
	}

	return ret;
}

static int fingerprint_release(struct inode *inode, struct file *file)
{
	hwlog_info("%s\n", __func__);
	return FP_RETURN_SUCCESS;
}

static const struct file_operations fingerprint_fops = {
	.owner          = THIS_MODULE,
	.open           = fingerprint_open,
	.write          = fingerprint_write,
	.read           = fingerprint_read,
	.release        = fingerprint_release,
	.unlocked_ioctl = fingerprint_ioctl,
};

static int fingerprint_reset_gpio_init(struct fp_data *fp)
{
	int ret = FP_RETURN_SUCCESS;

#if defined(CONFIG_HISI_PARTITION_KIRIN970)
#elif defined(CONFIG_HISI_PARTITION_LIBRA)
#elif defined(CONFIG_HISI_PARTITION_CANCER)
#elif defined(CONFIG_HISI_PARTITION_TAURUS)
#elif defined(CONFIG_HISI_PARTITION_CAPRICORN)
#else
	ret = gpio_request(fp->rst_gpio, "fingerprint_reset_gpio");
	if (ret) {
		hwlog_err("%s gpio_request (reset_gpio) failed\n", __func__);
		return -EINVAL;
	}

	fp->pctrl = devm_pinctrl_get(&fp->pf_dev->dev);
	if (IS_ERR(fp->pctrl)) {
		hwlog_err("%s devm_pinctrl_get failed\n", __func__);
		return -EINVAL;
	}

	fp->pins_default = pinctrl_lookup_state(fp->pctrl, "default");
	if (IS_ERR(fp->pins_default)) {
		hwlog_err("%s pinctrl_lookup_state failed\n", __func__);
		ret = -EINVAL;
		goto err_pinctrl_put;
	}

	ret = pinctrl_select_state(fp->pctrl, fp->pins_default);
	if (ret < 0) {
		hwlog_err("%s pinctrl_select_state failed, ret=%d\n",
			__func__, ret);
		ret = -EINVAL;
		goto err_pinctrl_put;
	}

	ret = gpio_direction_output(fp->rst_gpio, GPIO_LOW_LEVEL);
	if (ret) {
		hwlog_err("%s gpio_direction_output failed\n", __func__);
		return ret;
	}

	return ret;

err_pinctrl_put:
	devm_pinctrl_put(fp->pctrl);
#endif
	return ret;
}

static int fingerprint_power_en_gpio_init(struct fp_data *fp)
{
	int ret;

	ret = gpio_request(fp->power_en_gpio, "fingerprint_power_en_gpio");
	if (ret) {
		hwlog_err("%s gpio_request power_en_gpio failed, ret = %d\n",
			__func__, ret);
		return -EINVAL;
	}

	ret = gpio_direction_output(fp->power_en_gpio, GPIO_HIGH_LEVEL);
	if (ret)
		hwlog_err("%s gpio_direction_output power_en_gpio failed\n",
			__func__);
	return ret;
}

static void fingerprint_poweroff_close_ldo(struct fp_data *fp)
{
	int ret;
	int max_cnt = 100; /* max times that try to close the power */

	if (IS_ERR(fp_ex_regulator)) {
		hwlog_err("%s:No extern ldo found for fingerprint\n", __func__);
		return;
	}
	/*
	 * the power may be shared with other modules,
	 * so now close the power maybe more than one times
	 */
	do {
		hwlog_info("%s regulator flag:%d\n",
			__func__, regulator_is_enabled(fp_ex_regulator));
		if (regulator_is_enabled(fp_ex_regulator)) {
			ret = regulator_disable(fp_ex_regulator);
			if (ret != FP_RETURN_SUCCESS)
				hwlog_err("%s:regulator_disable fail, ret:%d\n",
					__func__, ret);

			/* break the process when the ldo regulator is close */
			if (regulator_is_enabled(fp_ex_regulator) == false) {
				hwlog_info("regulator is close and break\n");
				break;
			}
		}
		max_cnt--;
	} while (max_cnt > 0);
}

static void fingerprint_poweroff_pd_charge(struct fp_data *fp)
{
	int ret;
	unsigned int poweroff_scheme = 0;

	hwlog_info("%s enter\n", __func__);
	ret = of_property_read_u32(fp->dev->of_node,
		"fingerprint,poweroff_scheme", &poweroff_scheme);
	if (ret) {
		hwlog_info("%s fail to get poweroff_scheme from device tree, just go on\n",
			__func__);
		return;
	}
	hwlog_info("fingerprint deal power down charge config:%d charger:%d\n",
		poweroff_scheme, get_pd_charge_flag());
	/* when the state is power down charge the charge_flag is true */
	if (get_pd_charge_flag() == true) {
		switch (poweroff_scheme) {
		case FP_POWEROFF_SCHEME_ONE:
			fingerprint_poweroff_close_ldo(fp);
			break;
		case FP_POWEROFF_SCHEME_TWO:
			fingerprint_poweroff_close_loadswtich(fp);
			break;
		default:
			hwlog_err("%s poweroff_scheme config error %d\n",
				__func__, poweroff_scheme);
			break;
		}
	}
}

static int fingerprint_sub_pmic_power_set(struct fp_data *fp,
	unsigned int enable)
{
	struct fp_data *priv_data = NULL;
	struct hw_comm_pmic_cfg_t fp_pmic_ldo_set;

	if (fp == NULL) {
		pr_err("%s null pointer error\n", __func__);
		return -EINVAL;
	}
	hwlog_info("%s enable %d\n", __func__, enable);
	priv_data = fp;

	if ((priv_data->sub_pmic_num == (-EINVAL)) ||
		(priv_data->sub_ldo_num == (-EINVAL))) {
		pr_err("%s sub_pmic error\n", __func__);
		return -EINVAL;
	}

	fp_pmic_ldo_set.pmic_num = priv_data->sub_pmic_num;
	fp_pmic_ldo_set.pmic_power_type = priv_data->sub_ldo_num;
	fp_pmic_ldo_set.pmic_power_voltage = FP_POWER_LDO_VOLTAGE;
	fp_pmic_ldo_set.pmic_power_state = enable;

	return hw_pmic_power_cfg(FP_PMIC_REQ, &fp_pmic_ldo_set);
}

static int fingerprint_extern_ldo_proc(struct fp_data *fp)
{
	int ret;

	if (fp->extern_vol == -EINVAL) {
		hwlog_err("%s something wrong with dts config extern ldo\n",
			__func__);
		return -EINVAL;
	}

	fp_ex_regulator = devm_regulator_get(fp->dev,
		FINGERPRINT_EXTERN_LDO_NUM);
	if (IS_ERR(fp_ex_regulator)) {
		hwlog_err("%s:No extern ldo found for fingerprint\n",
			__func__);
		return -EINVAL;
	}

	ret = regulator_set_voltage(fp_ex_regulator, fp->extern_vol,
		fp->extern_vol);
	if (ret != FP_RETURN_SUCCESS)
		hwlog_err("%s:regulator_set_voltage fail,ret = %d\n",
			__func__, ret);

	ret = regulator_set_mode(fp_ex_regulator, REGULATOR_MODE_NORMAL);
	if (ret != FP_RETURN_SUCCESS)
		hwlog_err("%s:regulator_set_mode fail,ret = %d\n",
			__func__, ret);

	ret = regulator_enable(fp_ex_regulator);
	if (ret != FP_RETURN_SUCCESS)
		hwlog_err("%s:regulator_enable,ret = %d\n", __func__, ret);

	fingerprint_poweroff_pd_charge(fp);
	return FP_RETURN_SUCCESS;
}

static int fingerprint_extern_power_en(struct fp_data *fp)
{
	if (strncmp(fp->extern_ldo_name, "CAM_LDO", strlen("CAM_LDO")) == 0) {
		if ((fp->extern_ldo_num == -EINVAL) ||
			(fp->extern_vol == -EINVAL)) {
			hwlog_err("%s something wrong with dts config extern ldo\n",
				__func__);
			return -EINVAL;
		}
#if defined(CONFIG_USE_CAMERA3_ARCH)
		hw_extern_pmic_config(fp->extern_ldo_num, fp->extern_vol,
			FP_POWER_ENABLE);
#endif
	} else if (strncmp(fp->extern_ldo_name, FINGERPRINT_EXTERN_LDO_NAME,
		strlen(FINGERPRINT_EXTERN_LDO_NAME)) == 0) {
		if (fingerprint_extern_ldo_proc(fp))
			return -EINVAL;
	} else if (strncmp(fp->extern_ldo_name, SUB_LDO,
		strlen(SUB_LDO)) == 0) {
		hwlog_info("%s sub_ldo enable\n", __func__);
		if (fingerprint_sub_pmic_power_set(fp, FP_POWER_ENABLE))
			hwlog_info("%s pmic_power_set failed\n", __func__);
	} else {
		hwlog_err("%s other ldo\n", __func__);
	}

	return FP_RETURN_SUCCESS;
}

/*
 * This Function will be called when tui init or exit
 * It will set tui_flg, when it is set to 1 this mean tui is displaying
 * Fingerprint driver cannot read rst gpio because gpio is in secure status
 */
#if defined(CONFIG_TEE_TUI_HI3650)
static int tui_fp_init(void *data, int secure)
{
	struct fp_data *fp = g_fingerprint;

	if (!fp) {
		pr_err("%s:error:g_fingerprint is NULL\n", __func__);
		return -EINVAL;
	}
	if (fp->use_tp_irq == USE_TP_IRQ) {
		pr_err("%s: USE_TP_IRQ\n", __func__);
		return -EINVAL;
	}
	/*
	 * if fp rst gpio is in the same group with tui tp gpio
	 * then register the tui_fp_init
	 */
	if (fp->irq_gpio / GPIO_NUM_PER_GROUP ==
		tp_gpio_num / GPIO_NUM_PER_GROUP) {
		if (secure) {
			mutex_lock(&tui_flg_lock);
			tui_flg = true;
			mutex_unlock(&tui_flg_lock);
			disable_irq(gpio_to_irq(fp->irq_gpio));
			fp->irq_enabled = false;
		} else {
			mutex_lock(&tui_flg_lock);
			tui_flg = false;
			mutex_unlock(&tui_flg_lock);
			enable_irq(gpio_to_irq(fp->irq_gpio));
			fp->irq_enabled = true;
		}
		hwlog_info("%s: tui_flg is %d\n", __func__, tui_flg);
	}
	return FP_RETURN_SUCCESS;
}
#endif

static int finerprint_get_module_gpio_info(struct fp_data *fp)
{
	int ret;

	fp->pctrl = devm_pinctrl_get(&fp->pf_dev->dev);
	if (IS_ERR(fp->pctrl)) {
		hwlog_err("%s devm_pinctrl_get failed\n", __func__);
		ret = -EINVAL;
		return ret;
	}

	fp->pins_default = pinctrl_lookup_state(fp->pctrl, "default");
	if (IS_ERR(fp->pins_default)) {
		hwlog_err("%s pinctrl_lookup_state pins_default failed\n",
			__func__);
		ret = -EINVAL;
		goto error_pinctrl_put;
	}

	fp->pins_idle = pinctrl_lookup_state(fp->pctrl, "idle");
	if (IS_ERR(fp->pins_idle)) {
		hwlog_err("%s pinctrl_lookup_state pins_idle failed\n",
			__func__);
		ret = -EINVAL;
		goto error_pinctrl_put;
	}

	ret = gpio_direction_input(fp->module_id_gpio);
	if (ret < 0) {
		hwlog_err("%s set input mode error, ret=%d\n",
			__func__, ret);
		ret = -EINVAL;
		return ret;
	}

	ret = pinctrl_select_state(fp->pctrl, fp->pins_default);
	if (ret < 0) {
		hwlog_err("%s set iomux normal error, ret=%d\n",
			__func__, ret);
		ret = -EINVAL;
		goto error_pinctrl_put;
	}

	return ret;

error_pinctrl_put:
	devm_pinctrl_put(fp->pctrl);
	return ret;
}

static int finerprint_get_module_vendor_info(struct fp_data *fp)
{
	int ret;
	int pd_value;
	int pu_value;

	/*lint -save -e* */
	mdelay(10); /* delay 10 ms */
	/*lint -restore*/
	pu_value = gpio_get_value_cansleep(fp->module_id_gpio);
	hwlog_info("%s PU module id gpio = %d\n", __func__, pu_value);

	ret = gpio_direction_input(fp->module_id_gpio);
	if (ret < 0) {
		hwlog_err("%s set input mode error, ret=%d\n",
			__func__, ret);
		ret = -EINVAL;
		return ret;
	}

	ret = pinctrl_select_state(fp->pctrl, fp->pins_idle);
	if (ret < 0) {
		hwlog_err("%s set iomux normal error, ret=%d\n",
			__func__, ret);
		ret = -EINVAL;
		return ret;
	}
	/*lint -save -e* */
	mdelay(10);  /* delay 10 ms */
	/*lint -restore*/
	pd_value = gpio_get_value_cansleep(fp->module_id_gpio);
	hwlog_info("%s PD module id gpio=%d\n", __func__, pd_value);

	if (pu_value == pd_value) {
		if (pu_value == GPIO_HIGH_LEVEL) {
			fp->module_vendor_info = MODULEID_HIGT;
			hwlog_info("%s moduleID pin is HIGT\n", __func__);
		}

		if (pd_value == GPIO_LOW_LEVEL) {
			fp->module_vendor_info = MODULEID_LOW;
			hwlog_info("%s moduleID pin is LOW\n", __func__);
		}
	} else {
		fp->module_vendor_info = MODULEID_FLOATING;
		hwlog_info("%s moduleID pin is FLOATING\n", __func__);
	}

	return ret;
}

static int finerprint_get_module_info(struct fp_data *fp)
{
	int ret;

	ret = gpio_request(fp->module_id_gpio, "fingerprint_moduleID_gpio");
	if (ret) {
		hwlog_err("%s gpio_request failed\n", __func__);
		return ret;
	}

	ret = finerprint_get_module_gpio_info(fp);
	if (ret == (-EINVAL))
		return ret;

	ret = finerprint_get_module_vendor_info(fp);

	return ret;
}

static void fingerprint_custom_timing_scheme_one(struct fp_data *fp)
{
	hwlog_info("%s enter\n", __func__);
	accurate_delay_100us(10); /* delay 1ms */
	fingerprint_gpio_direction_output(fp->rst_gpio, GPIO_HIGH_LEVEL);
	fingerprint_gpio_direction_output(fp->cs0_gpio, GPIO_HIGH_LEVEL);
}

static void fingerprint_custom_timing_scheme_two(struct fp_data *fp)
{
	hwlog_info("%s enter\n", __func__);
	msleep(10); /* delay 10ms */
	fingerprint_gpio_direction_output(fp->rst_gpio, GPIO_HIGH_LEVEL);
	fingerprint_gpio_direction_output(fp->cs0_gpio, GPIO_HIGH_LEVEL);
}
/*
 * some device need spacial timing scheme3:
 * first power on the sensor that open loadswtich(gpio), then delay 10ms
 * power on cs, and then delay 600us, power on rst
 */
static void fingerprint_custom_timing_scheme_three(struct fp_data *fp)
{
	hwlog_info("%s enter\n", __func__);
	msleep(10); /* delay 10ms */
	fingerprint_gpio_direction_output(fp->cs0_gpio, GPIO_HIGH_LEVEL);
	accurate_delay_100us(6); /* delay 600us */
	fingerprint_gpio_direction_output(fp->rst_gpio, GPIO_HIGH_LEVEL);
}

static void fingerprint_custom_timing_scheme_four(struct fp_data *fp)
{
	hwlog_info("%s enter\n", __func__);
	fingerprint_gpio_direction_output(fp->rst1_gpio, GPIO_HIGH_LEVEL);
	fingerprint_gpio_direction_output(fp->cs1_gpio, GPIO_HIGH_LEVEL);
	fingerprint_gpio_direction_output(fp->rst_gpio, GPIO_HIGH_LEVEL);
	fingerprint_gpio_direction_output(fp->cs0_gpio, GPIO_HIGH_LEVEL);
}

static void fingerprint_custom_timing(struct fp_data *fp)
{
	int ret;
	unsigned int timing_scheme = 0;

	ret = of_property_read_u32(fp->dev->of_node,
		"fingerprint,custom_timing_scheme", &timing_scheme);
	if (ret) {
		hwlog_info("%s failed to get custom_timing_scheme from device tree, just go on\n",
			__func__);
		return;
	}
	switch (timing_scheme) {
	case FP_CUSTOM_TIMING_SCHEME_ONE:
		fingerprint_custom_timing_scheme_one(fp);
		break;
	case FP_CUSTOM_TIMING_SCHEME_TWO:
		fingerprint_custom_timing_scheme_two(fp);
		break;
	case FP_CUSTOM_TIMING_SCHEME_THREE:
		fingerprint_custom_timing_scheme_three(fp);
		break;
	case FP_CUSTOM_TIMING_SCHEME_FOUR:
		fingerprint_custom_timing_scheme_four(fp);
		break;
	default:
		hwlog_err("%s timing_scheme config error %d\n",
			__func__, timing_scheme);
		break;
	}
}

static void check_power_en_gpio_exsit(struct fp_data *fp)
{
	int ret;

	if (fp->power_en_gpio != (-EINVAL)) {
		ret = fingerprint_power_en_gpio_init(fp);
		if (ret)
			hwlog_err("something wrong in power_en_gpio_init ret=%d\n",
				ret);
	}

	if (strncmp(fp->extern_ldo_name, "EINVAL", strlen("EINVAL")) != 0) {
		ret = fingerprint_extern_power_en(fp);
		if (ret)
			hwlog_err("something wrong in extern_power_en init ret=%d\n",
				ret);
	}
}

static int fingerprint_device_create(struct fp_data *fp, struct device *dev)
{
	int ret;

	fp->class = class_create(THIS_MODULE, FP_CLASS_NAME);
	/*
	 * 0: first of the requested range of minor numbers
	 * 1: the number of minor numbers required
	 */
	ret = alloc_chrdev_region(&fp->devno, 0, 1, FP_DEV_NAME);
	if (ret) {
		hwlog_err("%s alloc_chrdev_region failed ret=%d\n", __func__,
			ret);
		return ret;
	}

	fp->device = device_create(fp->class, NULL, fp->devno, NULL, "%s",
		FP_DEV_NAME);
	cdev_init(&fp->cdev, &fingerprint_fops);
	fp->cdev.owner = THIS_MODULE;
	/* 1: consecutive minor numbers corresponding to this device */
	ret = cdev_add(&fp->cdev, fp->devno, 1);
	if (ret) {
		hwlog_err("%s cdev_add failed ret=%d\n", __func__, ret);
		return -ENOMEM;
	}

	fp->input_dev = devm_input_allocate_device(dev);
	if (!fp->input_dev) {
		hwlog_err("%s failed to allocate input device\n", __func__);
		return -ENOMEM;
	}

	fp->input_dev->name = "fingerprint";

	return FP_RETURN_SUCCESS;
}

static void fingerprint_device_set(struct fp_data *fp)
{
	/* Also register the key for wake up */
	input_set_capability(fp->input_dev, EV_KEY, EVENT_UP);
	input_set_capability(fp->input_dev, EV_KEY, EVENT_DOWN);
	input_set_capability(fp->input_dev, EV_KEY, EVENT_LEFT);
	input_set_capability(fp->input_dev, EV_KEY, EVENT_RIGHT);
	input_set_capability(fp->input_dev, EV_KEY, EVENT_HOLD);
	input_set_capability(fp->input_dev, EV_KEY, EVENT_CLICK);
	input_set_capability(fp->input_dev, EV_KEY, EVENT_HOLD);
	input_set_capability(fp->input_dev, EV_KEY, EVENT_DCLICK);
	input_set_capability(fp->input_dev, EV_KEY, EVENT_FINGER_UP);
	input_set_capability(fp->input_dev, EV_KEY, EVENT_FINGER_DOWN);
	input_set_capability(fp->input_dev, EV_KEY, EVENT_FINGER_IDENTIFY);
	input_set_capability(fp->input_dev, EV_KEY, EVENT_IDENTIFY_END);
	input_set_capability(fp->input_dev, EV_KEY, EVENT_FINGER_ENROLL);
	if (fp->cts_home > 0) {
		hwlog_info("%s fingerprint->cts_home=%d\n", __func__,
			fp->cts_home);
		input_set_capability(fp->input_dev, EV_KEY, EVENT_CTS_HOME);
		set_bit(EVENT_CTS_HOME, fp->input_dev->evbit);
	}
	set_bit(EV_KEY, fp->input_dev->evbit);
	set_bit(EVENT_UP, fp->input_dev->evbit);
	set_bit(EVENT_DOWN, fp->input_dev->evbit);
	set_bit(EVENT_LEFT, fp->input_dev->evbit);
	set_bit(EVENT_RIGHT, fp->input_dev->evbit);
	set_bit(EVENT_CLICK, fp->input_dev->evbit);
	set_bit(EVENT_HOLD, fp->input_dev->evbit);
	set_bit(EVENT_DCLICK, fp->input_dev->evbit);
	set_bit(EVENT_FINGER_UP, fp->input_dev->evbit);
	set_bit(EVENT_FINGER_DOWN, fp->input_dev->evbit);
	set_bit(EVENT_FINGER_IDENTIFY, fp->input_dev->evbit);
	set_bit(EVENT_IDENTIFY_END, fp->input_dev->evbit);
	set_bit(EVENT_FINGER_ENROLL, fp->input_dev->evbit);
}

static int fingerprint_device_register(struct fp_data *fp)
{
	int ret;

	ret = input_register_device(fp->input_dev);
	if (ret) {
		hwlog_err("%s input_register_device failed ret=%d\n",
			__func__, ret);
		return ret;
	}

	fp->wakeup_enabled = false;

	ret = sysfs_create_group(&fp->pf_dev->dev.kobj, &attribute_group);
	if (ret) {
		hwlog_err("%s sysfs_create_group failed ret=%d\n",
			__func__, ret);
		return ret;
	}

	return FP_RETURN_SUCCESS;
}

static int fingerprint_device_init(struct fp_data *fp, struct device *dev)
{
	int irqf;
	int ret = FP_RETURN_SUCCESS;
	int pen_ret = FP_RETURN_SUCCESS;

	irqf = IRQF_TRIGGER_RISING | IRQF_ONESHOT | IRQF_NO_SUSPEND;

	if (fp->irq_custom_scheme == FP_IRQ_SCHEME_ONE)
		irqf = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING |
			IRQF_ONESHOT | IRQF_NO_SUSPEND;

	device_init_wakeup(dev, true);
	wakeup_source_init(&fp->ttw_wl, "fpc_ttw_wl");
	init_waitqueue_head(&fp->hbm_queue);
	mutex_init(&fp->lock);

	if (fp->use_tp_irq == USE_TP_IRQ) {
		hwlog_info("%s don't request irq\n", __func__);
		goto use_tp_irq_tag;
	}

	ret = devm_request_threaded_irq(dev, gpio_to_irq(fp->irq_gpio), NULL,
		fingerprint_irq_handler, irqf, dev_name(dev), fp);
	if (ret) {
		hwlog_err("%s failed could not request irq %d\n", __func__,
			gpio_to_irq(fp->irq_gpio));
		return ret;
	}

	/* Request that the interrupt should be wakeable */
	enable_irq_wake(gpio_to_irq(fp->irq_gpio));
	fp->wakeup_enabled = true;
	fp->irq_enabled = true;

use_tp_irq_tag:
	fp->nav_stat = 0;
	fp->sensor_id = 0;
	fp->hbm_status = HBM_NONE;
	fp->fingerprint_bigdata.lcd_charge_time = 60; /* unit ms */
	fp->fingerprint_bigdata.lcd_on_time = 50; /* unit ms */
	fp->fingerprint_bigdata.cpu_wakeup_time = 80; /* unit ms */
	if (fp->pen_anti_enable) {
		pen_nb.notifier_call = stylus_pen_notify_fp;
		pen_ret = ts_event_notifier_register(&pen_nb);
		hwlog_info("%s pen_ret = %d\n", __func__, pen_ret);
	}

	g_fingerprint = fp;
	hwlog_info("fingerprint_probe OK\n");

	return ret;
}

#if defined(CONFIG_FB)
static int fb_notifier_callback(struct notifier_block *self,
	unsigned long event, void *data)
{
	int *blank = NULL;
	struct fb_event *event_data = data;
	struct fp_data *fingerprint = NULL;

	if (event_data == NULL || self == NULL) {
		hwlog_err("%s: event_data or self is null, return\n", __func__);
		return 0;
	}

	fingerprint = container_of(self, struct fp_data, fb_fp_notify);
	if (fingerprint == NULL) {
		hwlog_err("%s: fingerprintf is null, return\n", __func__);
		return 0;
	}

	smp_rmb();
	if (event_data->data && (event == FB_EVENT_BLANK) &&
		(event_data->info == registered_fb[0])) {
		blank = event_data->data;
		if (*blank == FB_BLANK_UNBLANK && fingerprint->irq_enabled == true) {
			hwlog_info("%s fingerprint fb unblank\n", __func__);
			sysfs_notify(&fingerprint->pf_dev->dev.kobj, NULL, "irq");
		}
	}

	return 0;
}
#endif

static int fingerprint_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	int ret;
	struct device_node *np = dev->of_node;
	struct fp_data *fp = devm_kzalloc(dev, sizeof(*fp), GFP_KERNEL);

#if defined(CONFIG_HUAWEI_DSM)
	if (!fingerprint_dclient)
		fingerprint_dclient = dsm_register_client(&dsm_fingerprint);
#endif
	if (!fp) {
		hwlog_err("%s failed to allocate memory for struct fp_data\n",
			__func__);
		ret = -ENOMEM;
		goto exit;
	}

	hwlog_info("fingerprint driver v4.0 for Android N\n");

	fp->dev = dev;
	dev_set_drvdata(dev, fp);
	fp->pf_dev = pdev;

	if (!np) {
		hwlog_err("%s dev->of_node not found\n", __func__);
		ret = -EINVAL;
		goto exit;
	}
	fp->pen_anti_enable = false;
#if defined(CONFIG_FB)
	fp->notify_enable = FP_UNBLANK_NOTIFY_DISABLE;
#endif
	ret = fingerprint_get_dts_data(&pdev->dev, fp);
	if (ret) {
		hwlog_err("something wrong in fingerprint_get_dts_data ret=%d\n",
			ret);
		ret = -EINVAL;
		goto exit;
	}

	fingerprint_get_navigation_adjustvalue(&pdev->dev, fp);

	ret = fingerprint_reset_gpio_init(fp);
	if (ret) {
		hwlog_err("something wrong in reset gpio init ret=%d\n", ret);
		ret = -EINVAL;
		goto exit;
	}

	check_power_en_gpio_exsit(fp);

	fingerprint_custom_timing(fp);

#if defined(CONFIG_TEE_TUI_HI3650)
	/*
	 * when tui init, the fingerprint should know
	 * then the fingerprint driver will not get gpio value
	 * Because the gpio is set to secure status
	 */
	register_tui_driver(tui_fp_init, "fp", NULL);
#endif

	if (fp->module_id_gpio >= 0) {
		ret = finerprint_get_module_info(fp);
		if (ret < 0) {
			hwlog_err("unknow vendor info ret=%d\n", ret);
			goto exit;
		}
	}

	ret = fingerprint_device_create(fp, dev);
	if (ret != FP_RETURN_SUCCESS)
		goto exit;

	fingerprint_device_set(fp);

	ret = fingerprint_device_register(fp);
	if (ret != FP_RETURN_SUCCESS)
		goto exit;

	ret = fingerprint_device_init(fp, dev);

#if defined(CONFIG_FB)
	if (fp->notify_enable == FP_UNBLANK_NOTIFY_ENABLE) {
		fp->fb_fp_notify.next = NULL;
		fp->fb_fp_notify.priority = -1;
		fp->fb_fp_notify.notifier_call = fb_notifier_callback;
		ret = fb_register_client(&fp->fb_fp_notify);
		if (ret)
			hwlog_err("fp register fb_notifier fail: %d,enable value:%d\n",
				ret, fp->notify_enable);
	}
#endif

exit:
#if defined(CONFIG_HUAWEI_DSM)
	if (ret && !dsm_client_ocuppy(fingerprint_dclient)) {
		dsm_client_record(fingerprint_dclient,
			"fingerprint_probe failed ret=%d\n", ret);
		dsm_client_notify(fingerprint_dclient,
			DSM_FINGERPRINT_PROBE_FAIL_ERROR_NO);
	}
#endif
	return ret;
}

static int fingerprint_remove(struct platform_device *pdev)
{
	int ret;
	struct  fp_data *fp = dev_get_drvdata(&pdev->dev);

	if (fp == NULL)
		return -EINVAL;
	ret = strncmp(fp->extern_ldo_name, FINGERPRINT_EXTERN_LDO_NAME,
		strlen(FINGERPRINT_EXTERN_LDO_NAME));
	if (ret == 0) {
		ret = regulator_disable(fp_ex_regulator);
		if (ret < 0)
			pr_err("%s:regulator_disable fail, ret = %d\n",
				__func__, ret);
	}
	if (strncmp(fp->extern_ldo_name,
		SUB_LDO, strlen(SUB_LDO)) == 0) {
		hwlog_info("%s extern_ldo_name using sub_ldo\n", __func__);
		if (fingerprint_sub_pmic_power_set(fp, FP_POWER_DISABLE))
			hwlog_info("%s pmic_power_set failed\n", __func__);
	}
	if (fp->pen_anti_enable)
		ts_event_notifier_unregister(&pen_nb);

#if defined(CONFIG_FB)
	if (fp->notify_enable == FP_UNBLANK_NOTIFY_ENABLE) {
		if (fb_unregister_client(&fp->fb_fp_notify))
			hwlog_info("%s fb_unregister_client error\n", __func__);
	}
#endif
	sysfs_remove_group(&fp->pf_dev->dev.kobj, &attribute_group);
	cdev_del(&fp->cdev);
	/* 1: device numbers to unregister */
	unregister_chrdev_region(fp->devno, 1);
	input_free_device(fp->input_dev);
	mutex_destroy(&fp->lock);
	wakeup_source_trash(&fp->ttw_wl);
	hwlog_info("%s\n", __func__);
	return FP_RETURN_SUCCESS;
}

static int fingerprint_suspend(struct device *dev)
{
#if defined(CONFIG_HUAWEI_DSM)
	struct  fp_data *fp = dev_get_drvdata(dev);

	if (fp == NULL)
		return -EINVAL;
	if (!runmode_is_factory() && (fp->sensor_id != 0) &&
		(fp->navigation_adjust1 == NAVIGATION_ADJUST_REVERSE)) {
		if (!(fp->irq_enabled || fp->irq_sensorhub_enabled))
			hwlog_err("irq not enable before suspend, adjust1=%d irq_enabled=%d irq_sensorhub_enabled=%d",
				fp->navigation_adjust1, (int)fp->irq_enabled,
				(int)fp->irq_sensorhub_enabled);
	}
#endif
	atomic_set(&g_pen_in_range_status, TS_PEN_OUT_RANGE);
	hwlog_info("%s\n", __func__);
	return FP_RETURN_SUCCESS;
}

static int fingerprint_resume(struct device *dev)
{
	hwlog_info("%s\n", __func__);
	return FP_RETURN_SUCCESS;
}

static const struct dev_pm_ops fingerprint_pm = {
	.suspend = fingerprint_suspend,
	.resume = fingerprint_resume
};

static const struct of_device_id fingerprint_of_match[] = {
	{ .compatible = "fpc,fingerprint", },
	{}
};

MODULE_DEVICE_TABLE(of, fingerprint_of_match);

static struct platform_driver fingerprint_driver = {
	.driver = {
	.name = "fingerprint",
	.owner = THIS_MODULE,
	.of_match_table = fingerprint_of_match,
	.pm = &fingerprint_pm
	},
	.probe = fingerprint_probe,
	.remove = fingerprint_remove
};

static int __init fingerprint_init(void)
{
	int ret = platform_driver_register(&fingerprint_driver);

	if (!ret)
		hwlog_info("%s OK\n", __func__);
	else
		hwlog_err("%s %d\n", __func__, ret);

	return ret;
}

static void __exit fingerprint_exit(void)
{
	hwlog_info("%s\n", __func__);
	platform_driver_unregister(&fingerprint_driver);
#if defined(CONFIG_TEE_TUI)
	unregister_tui_driver("fp");
#endif
}

#ifdef CONFIG_LLT_TEST
struct LLT_fingprint_ops llt_fingerprint = {
	.irq_get = irq_get,
	.fingerprint_irq_handler = fingerprint_irq_handler,
	.fingerprint_open = fingerprint_open,
	.fingerprint_get_irq_status = fingerprint_get_irq_status,
	.fingerprint_ioctl = fingerprint_ioctl,
	.fingerprint_reset_gpio_init = fingerprint_reset_gpio_init,
	.finerprint_get_module_info = finerprint_get_module_info,
	.fingerprint_probe = fingerprint_probe,
	.fingerprint_remove = fingerprint_remove,
};
#endif
/*lint -save -e* */
module_init(fingerprint_init);
module_exit(fingerprint_exit);
/*lint -restore*/

MODULE_LICENSE("GPL v2");
