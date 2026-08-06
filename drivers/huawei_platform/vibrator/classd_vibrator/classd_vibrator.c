/*
 * classd_vibrator.c
 *
 * classd_vibrator driver
 *
 * Copyright (c) 2018-2020 Huawei Technologies Co., Ltd.
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

#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/switch.h>
#include <linux/hrtimer.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/of_device.h>
#include <linux/io.h>
#include <linux/leds.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/hisi/flat_vibrator.h>
#include <linux/mtd/hisi_nve_interface.h>
#include <linux/regulator/consumer.h>
#include "dsp_vibrator.h"
#include <securec.h>
#include "huawei_platform/log/hw_log.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG hw_classd_vibrator
HWLOG_REGIST();

#define CLASSD_VIBRATOR "classd_vibrator"

#define ON                                 1
#define OFF                                0
#define SEC_MS_RATIO                       1000
#define MS_NSEC_RATIO                      1000000
#define DEFAULT_STOP_DELAY_MS              20
#define DEFAULT_MIN_VIB_TIMEOUT            10

#define HAPTICS_NODE_NAME                   "haptics"
#define MAX_BUF_LENGTH                      16
#define MIN_CALIBRATOIN_NUM                 1000
#define MAX_CALIBRATOIN_NUM                 1010
#define VIB_PLAY_COUNT                      20
#define SWITCH_STATE_PLAY                   2
#define SWITCH_STATE_STOP                   0
#define VIB_CALIDATA_NV_NUM                 337
#define NV_READ_TAG                         1
#define NV_WRITE_TAG                        0

#define CLASSD_ONOFF_ADDRESS                0x053
#define CLASSD_CTRL0_ADDRESS                0x580
#define CLASSD_ON_VALUE                     (1 << 0)
#define CLASSD_CTRL0_VALUE                  (1 << 7)
#define CLASSD_OFF                          0

#define VIB_NV_NAME                         "VIBCAL"
#define HAPTIC_WORK_MAX                     1000

struct classd_vibrator_data {
	struct device *dev;
	struct led_classdev cdev;

	struct work_struct enable_work;
	struct mutex lock;
	struct mutex ldo_lock;
	struct hrtimer stop_timer;

	uint32_t vib_type;
	uint32_t classd_reg_address;
	uint32_t start_vib_value;
	uint32_t reverse_vib_value;
	uint32_t stop_vib_value;
	uint32_t stop_delay_ms;

	uint8_t support_quick_stop;
	uint8_t ldo_status;

	int state;
	struct regulator *ldo;
	int vout_voltage;
	uint8_t vibrator_working;
	uint32_t vibrator_mode;
};

struct haptics_data {
	struct mutex lock;
	dev_t version;
	struct class *class;
	struct device *device;
	struct cdev cdev;
	struct work_struct work_play_eff;
	struct workqueue_struct *vibrator_queue;
	struct classd_vibrator_data *priv;
	unsigned int effect_type;
	wait_queue_head_t wq;
	bool haptics_working;
} *g_haptics_data;

#ifdef CONFIG_HISI_VIBRATOR
extern volatile int vibrator_shake;
#else
volatile int vibrator_shake;
#endif
static uint32_t min_vib_time_ms;
static uint32_t stop_delay_time_ms;
static unsigned char *g_trans_buffer = NULL;	// wave_buffer

static void classd_vibrator_set_cali_data(void);

unsigned char *get_trans_buffer(void)
{
	return g_trans_buffer;
}

void init_trans_buffer(void)
{
	if (g_trans_buffer == NULL) {
		g_trans_buffer = ioremap_wc((phys_addr_t)HISI_AP_AUDIO_VIBRATOR_ADDR,
			(size_t)HISI_AP_AUDIO_VIBRATOR_DATA_SIZE);
		hwlog_info("%s: end:buffer:%p", __func__, g_trans_buffer);
	}
}

void final_trans_buffer(void)
{
	if (g_trans_buffer != NULL) {
		iounmap(g_trans_buffer);
		g_trans_buffer = NULL;
	}
}

static void hisi_pmic_modify(uint32_t address, uint8_t mask, uint8_t value)
{
	uint8_t temp;

	temp = (uint8_t)hisi_pmic_reg_read(address);

	temp &= (~mask);
	temp |= value;

	hisi_pmic_reg_write(address, temp);
}

static void classd_open(void)
{
	hisi_pmic_modify(CLASSD_ONOFF_ADDRESS, CLASSD_ON_VALUE, CLASSD_ON_VALUE);
	hisi_pmic_modify(CLASSD_CTRL0_ADDRESS, CLASSD_CTRL0_VALUE, CLASSD_CTRL0_VALUE);
}

static void classd_close(void)
{
	hisi_pmic_modify(CLASSD_CTRL0_ADDRESS, CLASSD_CTRL0_VALUE, CLASSD_OFF);
	hisi_pmic_modify(CLASSD_ONOFF_ADDRESS, CLASSD_ON_VALUE, CLASSD_OFF);
}

static void set_ldo_power(struct classd_vibrator_data *priv, uint8_t state)
{
	int ret;

	if (priv->ldo == NULL) {
		hwlog_debug("%s: ldo null ,not suppot", __func__);
		return;
	}

	mutex_lock(&priv->ldo_lock);
	if (state == priv->ldo_status) {
		mutex_unlock(&priv->ldo_lock);
		return;
	}

	if (state == ON)
		ret = regulator_enable(priv->ldo);
	else
		ret = regulator_disable(priv->ldo);

	if (ret < 0)
		hwlog_err("%s, fail!! state = %u", __func__, state);
	else
		priv->ldo_status = state;

	mutex_unlock(&priv->ldo_lock);
}

static void vibrator_on(struct classd_vibrator_data *priv)
{
	if (hrtimer_active(&priv->stop_timer))
		hrtimer_cancel(&priv->stop_timer);

	hwlog_info("%s: enter!!", __func__);

	set_ldo_power(priv, ON);
	if (priv->vib_type == VIB_CLASS_D_TYPE) {
		hisi_pmic_reg_write(priv->classd_reg_address, priv->start_vib_value);
	} else if (priv->vib_type == VIB_DSP_PCM_TYPE) {
		classd_open();
		classd_vibrator_set_cali_data();
		start_play_vibrator_data(VIB_WAV_235HZ, VIB_FB_PLAY_CN);
	}

	vibrator_shake = ON;
	priv->vibrator_working = ON;
}

static void vibrator_power_off(struct classd_vibrator_data *priv)
{
	if (priv->vib_type == VIB_DSP_PCM_TYPE) {
		classd_close();
		if(is_play_vibrator())
			stop_play_vibrator_data();
	} else if (priv->vib_type == VIB_CLASS_D_TYPE) {
		hisi_pmic_reg_write(priv->classd_reg_address, priv->stop_vib_value);
	}
	set_ldo_power(priv, OFF);

	vibrator_shake = OFF;
}

static void vibrator_off(struct classd_vibrator_data *priv)
{
	if (priv->vibrator_working == OFF) {
		hwlog_info("%s: already off!!", __func__);
		return;
	}

	if (priv->support_quick_stop == 0 || priv->vib_type == VIB_DSP_PCM_TYPE) {
		priv->vibrator_working = OFF;
		vibrator_power_off(priv);
		hwlog_info("%s: stop right now!!", __func__);
		return;
	}

	hwlog_info("%s: enter!!", __func__);
	if (priv->vib_type == VIB_CLASS_D_TYPE)
		hisi_pmic_reg_write(priv->classd_reg_address, priv->reverse_vib_value);

	hrtimer_start(&priv->stop_timer,
		ktime_set(stop_delay_time_ms / SEC_MS_RATIO,
			(stop_delay_time_ms % SEC_MS_RATIO) * MS_NSEC_RATIO),
		HRTIMER_MODE_REL);
}


static enum hrtimer_restart classd_vibrator_stop_timer_func
	(struct hrtimer *timer)
{
	struct classd_vibrator_data *priv = NULL;

	priv = container_of(timer, struct classd_vibrator_data, stop_timer);
	if (priv == NULL) {
		hwlog_err("%s: invalid data", __func__);
		return HRTIMER_NORESTART;
	}

	priv->vibrator_working = OFF;
	vibrator_power_off(priv);
	hwlog_info("%s: succ!!\n", __func__);
	return HRTIMER_NORESTART;
}


static void vibrator_enable_work(struct work_struct *work)
{
	struct classd_vibrator_data *priv = NULL;
	int state;

	priv = container_of(work, struct classd_vibrator_data, enable_work);
	if (priv == NULL) {
		hwlog_err("%s: invalid data", __func__);
		return;
	}

	state = priv->state;
	hwlog_info("%s: enter!! state = %d", __func__, state);

	mutex_lock(&priv->lock);
	if (state)
		vibrator_on(priv);
	else
		vibrator_off(priv);

	mutex_unlock(&priv->lock);
}

static void classd_vibrator_enable(struct led_classdev *cdev,
	enum led_brightness state)
{
	struct classd_vibrator_data *priv = NULL;

	priv = container_of(cdev, struct classd_vibrator_data, cdev);
	if (priv == NULL) {
		hwlog_err("%s: invalid data", __func__);
		return;
	}

	hwlog_info("%s: enter!! state = %d\n", __func__, (int)state);
	priv->state = (int) state;
	schedule_work(&priv->enable_work);
}

static int classd_vibrator_register_led_classdev(
	struct classd_vibrator_data *priv)
{
	struct led_classdev *cdev = &priv->cdev;

	cdev->name = "vibrator";
	cdev->flags = LED_CORE_SUSPENDRESUME;
	cdev->brightness_set = classd_vibrator_enable;
	cdev->default_trigger = "transient";

	return led_classdev_register(NULL, cdev);
}

static int classd_vibrator_parse_dts_param(struct classd_vibrator_data *priv)
{
	struct device_node *dn = NULL;
	int ret = 0;

	dn = priv->dev->of_node;

	ret += of_property_read_u32(dn, "vib_type",
		&priv->vib_type);
	ret += of_property_read_u32(dn, "reg_address",
		&priv->classd_reg_address);
	ret += of_property_read_u32(dn, "start_vib_value",
		&priv->start_vib_value);
	ret += of_property_read_u32(dn, "stop_vib_value",
		&priv->stop_vib_value);
	if (ret < 0) {
		hwlog_err("%s read reg fail, ret = %d", __func__, ret);
		return -1;
	}
	hwlog_info("%s type is %#x, reg is %#x, val is %#x, %#x", __func__,
		priv->vib_type,
		priv->classd_reg_address,
		priv->start_vib_value,
		priv->stop_vib_value);

	ret = of_property_read_u32(dn, "vibrator_mode",
		&priv->vibrator_mode);
	if (ret != 0) {
		priv->vibrator_mode = MODE_LDO;
		hwlog_info("%s read vibrator mode fail, may not use, ret = %d", __func__, ret);
	}

	ret = of_property_read_u32(dn, "reverse_vib_value",
		&priv->reverse_vib_value);
	if (ret < 0) {
		hwlog_info("%s, read reverse_vib_value fail, not support",
			__func__);
		priv->support_quick_stop = 0; // not support quick stop
	} else {
		priv->support_quick_stop = 1; // support quick stop
	}
	hwlog_info("%s reverse_vib_value is 0x%x, support_quick_stop is %u",
		__func__, priv->reverse_vib_value, priv->support_quick_stop);

	ret = of_property_read_u32(dn, "stop_delay_ms", &stop_delay_time_ms);
	if (ret < 0) {
		hwlog_info("%s, read stop_delay_ms fail, use default",
			__func__);
		stop_delay_time_ms = DEFAULT_STOP_DELAY_MS;
	}
	ret = of_property_read_u32(dn, "min_vib_timeout", &min_vib_time_ms);
	if (ret < 0) {
		hwlog_info("%s, read min_vib_timeout fail, use default",
			__func__);
		min_vib_time_ms = DEFAULT_MIN_VIB_TIMEOUT;
	}
	hwlog_info("%s stop_delay_ms = %u, min_vib_timeout is %u", __func__,
		stop_delay_time_ms, min_vib_time_ms);

	return 0;
}

static int classd_vibrator_get_ldo_regulator(struct platform_device *pdev,
	struct classd_vibrator_data *priv)
{
	struct device_node *dn = NULL;
	int ret;

	dn = priv->dev->of_node;
	if (dn == NULL) {
		hwlog_err("%s: null node", __func__);
		return -1;
	}

	if (priv->vibrator_mode != MODE_LDO) {
		hwlog_info("%s: vibrator not use ldo", __func__);
		return 0;
	}

	priv->ldo = devm_regulator_get(&pdev->dev, "vibrator-vdd");
	if (IS_ERR_OR_NULL(priv->ldo)) {
		hwlog_info("%s: not find ldo node", __func__);
		return -1;
	}

	ret = of_property_read_u32(dn, "vout_voltage", (u32 *)&(priv->vout_voltage));
	if (ret != 0) {
		hwlog_err("%s: read voltage fail ,use default", __func__);
		priv->vout_voltage = 3000000; // 3V
	}

	ret = regulator_set_voltage(priv->ldo,
		priv->vout_voltage, priv->vout_voltage);
	if (ret != 0) {
		hwlog_err("%s: set voltage fail", __func__);
		return -1;
	}

	hwlog_info("%s: succ! vout_voltage = %d", __func__, priv->vout_voltage);
	return 0;
}

static void stop_haptics(void)
{
	g_haptics_data->haptics_working = false;
	wake_up_interruptible(&g_haptics_data->wq);
}

static void play_effect(struct work_struct *work)
{
	struct haptics_data *data = NULL;
	int ret;

	if (work == NULL) {
		hwlog_err("%s: NULL work data\n", __func__);
		return;
	}

	data = container_of(work, struct haptics_data, work_play_eff);
	if (data == NULL) {
		hwlog_err("%s: NULL point\n", __func__);
		return;
	}

	if (data->priv->vib_type != VIB_DSP_PCM_TYPE) {
		hwlog_err("%s: Not support classd vibrator", __func__);
		return;
	}

	data->haptics_working = true;
	vibrator_shake = ON;
	data->priv->vibrator_working = ON;
	set_ldo_power(data->priv, ON);
	classd_open();
	classd_vibrator_set_cali_data();
	start_play_vibrator_data(data->effect_type, VIB_PLAY_COUNT);
	ret = wait_event_interruptible_timeout(data->wq,
		(data->haptics_working == false),
		msecs_to_jiffies(HAPTIC_WORK_MAX));
	if (ret == 0)
		hwlog_info("%s:effect play wait timeout", __func__);
	stop_play_vibrator_data();
	classd_close();
	set_ldo_power(data->priv, OFF);
	vibrator_shake = OFF;
	data->priv->vibrator_working = OFF;
}

static int haptics_open(struct inode *i_node, struct file *filp)
{
	if (g_haptics_data == NULL || filp == NULL) {
		hwlog_err("%s: Invalid data", __func__);
		return -ENODEV;
	}

	filp->private_data = (void *)g_haptics_data;
	return 0;
}

static ssize_t haptics_write(struct file *filp, const char *buff,
			size_t len, loff_t *off)
{
	unsigned int type = 0;
	char write_buf[MAX_BUF_LENGTH] = {0};
	bool ret = false;
	struct haptics_data *data = NULL;

	if (len > MAX_BUF_LENGTH || buff == NULL || filp == NULL) {
		hwlog_err("%s: Invalid data, len : %d", __func__, len);
		return 0;
	}

	data = (struct haptics_data *)filp->private_data;
	if (data == NULL) {
		hwlog_err("%s: NULL data", __func__);
		return 0;
	}

	if (copy_from_user(write_buf, buff, len)) {
		hwlog_err("%s: copy data fail", __func__);
		return 0;
	}

	if (kstrtoint(write_buf, 10, &type)) {
		hwlog_err("%s: transform data fail ", __func__);
		return 0;
	}
	hwlog_info("%s: enter, type is %d", __func__, type);

	if (type > MAX_CALIBRATOIN_NUM || type == 0) {
		hwlog_err("%s: effect type is invalid", __func__);
		return 0;
	}

	if (type >= MIN_CALIBRATOIN_NUM && type <= MAX_CALIBRATOIN_NUM)
		type = (type - MIN_CALIBRATOIN_NUM) + VIB_CALIBRATION_0;

	mutex_lock(&data->lock);
	data->effect_type = type;
	mutex_unlock(&data->lock);
	ret = queue_work(g_haptics_data->vibrator_queue,
		&g_haptics_data->work_play_eff);
	if (!ret)
		hwlog_info("%s: schedule work fail", __func__);
	return len;
}

static const struct file_operations haptic_fops = {
	.open = haptics_open,
	.write = haptics_write,
};

static void vibrator_remove_workqueue(void)
{
	if (g_haptics_data->vibrator_queue) {
		cancel_work_sync(&g_haptics_data->work_play_eff);
		flush_workqueue(g_haptics_data->vibrator_queue);
		destroy_workqueue(g_haptics_data->vibrator_queue);
	}
}

static int vibrator_create_workqueue(void)
{
	g_haptics_data->vibrator_queue =
		create_singlethread_workqueue("vibrator_queue");
	if (!g_haptics_data->vibrator_queue) {
		hwlog_err("%s : vibrator queue create failed\n", __func__);
		return -ENOMEM;
	}
	INIT_WORK(&g_haptics_data->work_play_eff, play_effect);
	return 0;
}

static int haptics_init(struct classd_vibrator_data *priv)
{
	struct haptics_data *node_data = g_haptics_data;
	int ret;

	if (priv == NULL || node_data == NULL) {
		hwlog_err("%s: NULL data", __func__);
		return -ENOMEM;
	}
	node_data->priv = priv;

	ret = alloc_chrdev_region(&node_data->version, 0, 1, HAPTICS_NODE_NAME);
	if (ret < 0) {
		hwlog_err("%s: alloc chrdev region fail", __func__);
		return ret;
	}

	node_data->class = class_create(THIS_MODULE, HAPTICS_NODE_NAME);
	if (node_data->class == NULL) {
		hwlog_err("%s: class create fail", __func__);
		goto unregister_cdev_region;
	}

	node_data->device = device_create(node_data->class, NULL,
		node_data->version, NULL, HAPTICS_NODE_NAME);
	if (node_data->device == NULL) {
		hwlog_err("%s: device create fail", __func__);
		goto destory_class;
	}

	cdev_init(&node_data->cdev, &haptic_fops);
	node_data->cdev.owner = THIS_MODULE;
	ret = cdev_add(&node_data->cdev, node_data->version, 1);
	if (ret != 0) {
		hwlog_err("%s: cdev add fail", __func__);
		goto destory_device;
	}

	ret = vibrator_create_workqueue();
	if (ret != 0) {
		hwlog_err("%s: work queue creat fail", __func__);
		goto work_queue_err;
	}

	mutex_init(&node_data->lock);
	init_waitqueue_head(&node_data->wq);
	dsp_vibrator_register_stop_handler(&stop_haptics);
	return 0;
work_queue_err:
	cdev_del(&node_data->cdev);
destory_device:
	device_destroy(node_data->class, node_data->version);
destory_class:
	class_destroy(node_data->class);
unregister_cdev_region:
	unregister_chrdev_region(node_data->version, 1);

	return ret;
}

#ifdef CONFIG_CLASSD_DEBUG
static ssize_t classd_vibrator_min_timeout_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	uint64_t value = 0;

	if (buf == NULL) {
		hwlog_info("%s: NULL point", __func__);
		return (ssize_t)count;
	}

	if (kstrtoull(buf, 10, &value)) {
		hwlog_info("%s read value error", __func__);
		return (ssize_t)count;
	}

	min_vib_time_ms = (uint32_t)value;
	hwlog_info("%s change min_vib_time_ms is %u", __func__, min_vib_time_ms);
	return (ssize_t)count;
}

static ssize_t classd_vibrator_stop_delay_time_show(
	struct device *dev, struct device_attribute *attr, char *buf)
{
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
		"%d\n", (int)stop_delay_time_ms);
}

static ssize_t classd_vibrator_stop_delay_time_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	uint64_t value = 0;

	if (buf == NULL) {
		hwlog_info("%s: NULL point\n", __func__);
		return (ssize_t)count;
	}

	if (kstrtoull(buf, 10, &value)) {
		hwlog_info("%s read value error\n", __func__);
		return (ssize_t)count;
	}

	stop_delay_time_ms = (uint32_t)value;
	hwlog_info("%s change stop delay time is %u", __func__, stop_delay_time_ms);
	return (ssize_t)count;
}
#endif

static ssize_t classd_vibrator_min_timeout_show(
	struct device *dev, struct device_attribute *attr, char *buf)
{
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
		"%d\n", (int)min_vib_time_ms);
}

static int classd_virbator_write_data_to_nv(uint32_t cali_data)
{
	int ret;
	struct hisi_nve_info_user user_info;

	ret = memset_s(&user_info, sizeof(user_info), 0, sizeof(user_info));
	if (ret != 0){
		hwlog_err("%s: memset fail\n", __func__);
		return -1;
	}

	user_info.nv_operation = NV_WRITE_TAG;
	user_info.nv_number = VIB_CALIDATA_NV_NUM;
	user_info.valid_size = sizeof(cali_data);

	ret = strncpy_s(user_info.nv_name, sizeof(user_info.nv_name),
		VIB_NV_NAME, strlen(VIB_NV_NAME));
	if (ret != 0) {
		hwlog_err("%s: strncpy_s fail\n", __func__);
		return -1;
	}

	hwlog_info("%s cali data = %u", __func__, cali_data);
	ret = memcpy_s(user_info.nv_data, sizeof(user_info.nv_data),
		&cali_data, sizeof(cali_data));
	if (ret != 0) {
		hwlog_err("%s: memcpy_s fail\n", __func__);
		return -1;
	}

	ret = hisi_nve_direct_access(&user_info);
	if (ret != 0) {
		hwlog_err("%s: hisi_nve_direct_access fail\n", __func__);
		return -1;
	}

	hwlog_info("%s: succ\n", __func__);
	return 0;
}

static int classd_vibrator_read_data_from_nv(uint32_t *nv_data)
{
	int ret;
	struct hisi_nve_info_user user_info;

	ret = memset_s(&user_info, sizeof(user_info), 0, sizeof(user_info));
	if (ret != 0){
		hwlog_err("%s: memset fail\n", __func__);
		return -1;
	}

	user_info.nv_operation = NV_READ_TAG;
	user_info.nv_number = VIB_CALIDATA_NV_NUM;
	user_info.valid_size = sizeof(*nv_data);

	ret = strncpy_s(user_info.nv_name, sizeof(user_info.nv_name),
		VIB_NV_NAME, strlen(VIB_NV_NAME));
	if (ret != 0) {
		hwlog_err("%s: strncpy_s fail\n", __func__);
		return -1;
	}

	ret = hisi_nve_direct_access(&user_info);
	if (ret != 0) {
		hwlog_err("%s: hisi_nve_direct_access fail\n", __func__);
		return -1;
	}

	ret = memcpy_s(nv_data, sizeof(*nv_data), user_info.nv_data, sizeof(*nv_data));
	if (ret != 0) {
		hwlog_err("%s: memcpy_s fail\n", __func__);
		return -1;
	}

	hwlog_info("%s: succ\n", __func__);
	return 0;

}

static void classd_vibrator_set_cali_data(void)
{
	static bool set_finish;
	int ret;
	uint32_t nv_data;

	if (set_finish)
		return;

	ret = classd_vibrator_read_data_from_nv(&nv_data);
	if (ret < 0) {
		hwlog_err("%s: read nv fail\n", __func__);
		return;
	}

	dsp_vibrator_set_freq(nv_data);
	set_finish = true;
	return;
}

static ssize_t classd_vibrator_calib_show(
	struct device *dev, struct device_attribute *attr, char *buf)
{
	int ret;
	int nv_data = 0;

	ret = classd_vibrator_read_data_from_nv(&nv_data);
	if (ret < 0) {
		hwlog_err("%s: read nv fail\n", __func__);
		return -EPERM;
	}

	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
		"%d\n", nv_data);
}

static ssize_t classd_vibrator_calib_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	uint64_t value = 0;
	int ret;
	uint32_t freq;

	if (buf == NULL) {
		hwlog_info("%s: NULL point\n", __func__);
		return -EPERM;
	}

	if (kstrtoull(buf, 10, &value)) {
		hwlog_info("%s read value error\n", __func__);
		return -EPERM;
	}

	freq = (uint32_t)value;
	hwlog_info("%s target_freq is %u", __func__, freq);
	dsp_vibrator_set_freq(freq);

	ret = classd_virbator_write_data_to_nv(freq);
	if (ret < 0) {
		hwlog_err("%s wirte nv fail\n", __func__);
		return -EPERM;
	}

	return (ssize_t)count;
}

static ssize_t classd_support_hal_cali_show(
	struct device *dev, struct device_attribute *attr, char *buf)
{
	int support_hal_cali = 1;

	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
		"%d\n", support_hal_cali);
}


#ifdef CONFIG_CLASSD_DEBUG
static DEVICE_ATTR(vibrator_stop_delay_time,
	0600,
	classd_vibrator_stop_delay_time_show,
	classd_vibrator_stop_delay_time_store);

static DEVICE_ATTR(
	vibrator_min_timeout,
	0664,
	classd_vibrator_min_timeout_show,
	classd_vibrator_min_timeout_store);
#else
static DEVICE_ATTR(
	vibrator_min_timeout,
	0664,
	classd_vibrator_min_timeout_show,
	NULL);
#endif

static DEVICE_ATTR(
	vibrator_calib,
	0664,
	classd_vibrator_calib_show,
	classd_vibrator_calib_store);

static DEVICE_ATTR(
	support_hal_cali,
	0664,
	classd_support_hal_cali_show,
	NULL);


static struct attribute *vb_attributes[] = {
	&dev_attr_vibrator_min_timeout.attr,
#ifdef CONFIG_CLASSD_DEBUG
	&dev_attr_vibrator_stop_delay_time.attr,
#endif
	&dev_attr_vibrator_calib.attr,
	&dev_attr_support_hal_cali.attr,
	NULL
};

static const struct attribute_group vb_attr_group = {
	.attrs = vb_attributes,
};

static const struct of_device_id classd_vibrator_match[] = {
	{ .compatible = "huawei,classd-vibrator", },
	{},
};
MODULE_DEVICE_TABLE(of, classd_vibrator_match);

static int classd_vibrator_probe(struct platform_device *pdev)
{
	struct classd_vibrator_data *priv = NULL;
	int ret;

	if (!of_match_node(classd_vibrator_match, pdev->dev.of_node)) {
		hwlog_err("%s, dev node no match. exiting", __func__);
		return -ENODEV;
	}

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (priv == NULL)
		return -ENOMEM;

	priv->dev = &pdev->dev;

	ret = classd_vibrator_parse_dts_param(priv);
	if (ret < 0) {
		hwlog_err("%s, pares dts fail", __func__);
		return -EINVAL;
	}

	if (priv->vib_type == VIB_DSP_PCM_TYPE) {
		g_haptics_data = devm_kzalloc(&pdev->dev,
			sizeof(*g_haptics_data), GFP_KERNEL);
		if (g_haptics_data == NULL) {
			hwlog_err("%s, failed to alloc haptics data", __func__);
			return -ENOMEM;
		}
		ret = haptics_init(priv);
		if (ret) {
			hwlog_err("%s, haptics_init failed", __func__);
			return -EINVAL;
		}
		change_vibrator_mode(priv->vibrator_mode);
		init_trans_buffer();
		if (g_trans_buffer == NULL) {
			hwlog_err("%s, buff remap fail", __func__);
			return -ENOMEM;
		}
	}

	ret = classd_vibrator_get_ldo_regulator(pdev, priv);
	if (ret < 0) {
		hwlog_info("%s, get ldo fail, not need power", __func__);
		priv->ldo = NULL;
		if (priv->vib_type == VIB_DSP_PCM_TYPE)
			goto get_ldo_err;
	}

	ret = classd_vibrator_register_led_classdev(priv);
	if (ret < 0) {
		hwlog_err("%s failed to register led_classdevpriv\n", __func__);
		goto led_register_err;
	}

	ret = sysfs_create_group(&priv->cdev.dev->kobj, &vb_attr_group);
	if (ret < 0) {
		hwlog_err("%s, failed to creat sysfs", __func__);
		goto sys_creat_err;
	}

	mutex_init(&priv->lock);
	mutex_init(&priv->ldo_lock);
	INIT_WORK(&priv->enable_work, vibrator_enable_work);
	hrtimer_init(&priv->stop_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	priv->stop_timer.function = classd_vibrator_stop_timer_func;
	priv->ldo_status = OFF;
	priv->vibrator_working = OFF;
	platform_set_drvdata(pdev, priv);
	hwlog_info("%s: successful!\n", __func__);
	return 0;

get_ldo_err:
	final_trans_buffer();
sys_creat_err:
	led_classdev_unregister(&priv->cdev);
led_register_err:
	if (priv->ldo != NULL)
		devm_regulator_put(priv->ldo);
	priv->ldo = NULL;
	dev_err(&pdev->dev, "%s: failed!\n", __func__);
	return ret;
}

static int classd_vibrator_remove(struct platform_device *pdev)
{
	struct classd_vibrator_data *priv = platform_get_drvdata(pdev);

	if (priv == NULL) {
		dev_err(&pdev->dev, "%s:pdata is NULL\n", __func__);
		return -ENODEV;
	}

	if (hrtimer_active(&priv->stop_timer))
		hrtimer_cancel(&priv->stop_timer);

	if (priv->vib_type == VIB_DSP_PCM_TYPE)
		vibrator_remove_workqueue();

	if (priv->vib_type == VIB_DSP_PCM_TYPE && is_play_vibrator()) {
		classd_close();
		stop_play_vibrator_data();
	}

	sysfs_remove_group(&priv->cdev.dev->kobj, &vb_attr_group);
	led_classdev_unregister(&priv->cdev);

	mutex_destroy(&priv->lock);
	mutex_destroy(&priv->ldo_lock);
	cancel_work_sync(&priv->enable_work);
	devm_regulator_put(priv->ldo);
	priv->ldo = NULL;

	if (priv->vib_type == VIB_DSP_PCM_TYPE)
		final_trans_buffer();

	platform_set_drvdata(pdev, NULL);
	return 0;
}

static void classd_vibrator_shutdown(struct platform_device *pdev)
{
	struct classd_vibrator_data *priv = platform_get_drvdata(pdev);
	if (priv == NULL) {
		hwlog_err("%s: priv null!\n", __func__);
		return;
	}

	if (hrtimer_active(&priv->stop_timer))
		hrtimer_cancel(&priv->stop_timer);

	priv->vibrator_working = OFF;
	vibrator_power_off(priv);
}

#ifdef CONFIG_PM
static int classd_vibrator_suspend(struct platform_device *pdev,
	pm_message_t state)
{
	struct classd_vibrator_data *priv = platform_get_drvdata(pdev);

	if (priv == NULL) {
		hwlog_err("%s: priv null!", __func__);
		return -ENODEV;
	}

	if (hrtimer_active(&priv->stop_timer))
		hrtimer_cancel(&priv->stop_timer);

	priv->vibrator_working = OFF;
	vibrator_power_off(priv);
	hwlog_info("%s: enter!\n", __func__);
	return 0;
}

static int classd_vibrator_resume(struct platform_device *pdev)
{
	struct classd_vibrator_data *priv = platform_get_drvdata(pdev);

	if (priv == NULL) {
		hwlog_err("%s: priv null!", __func__);
		return -ENODEV;
	}

	hwlog_info("%s: enter!\n", __func__);
	return 0;
}
#endif


static struct platform_driver classd_vibrator_driver = {
	.probe    = classd_vibrator_probe,
	.remove   = classd_vibrator_remove,
	.shutdown = classd_vibrator_shutdown,
#ifdef CONFIG_PM
	.suspend  = classd_vibrator_suspend,
	.resume   = classd_vibrator_resume,
#endif
	.driver   = {
		.name = CLASSD_VIBRATOR,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(classd_vibrator_match),
	},
};

static int __init classd_vibrator_init(void)
{
	return platform_driver_register(&classd_vibrator_driver);
}

static void __exit classd_vibrator_exit(void)
{
	platform_driver_unregister(&classd_vibrator_driver);
}

module_init(classd_vibrator_init);
module_exit(classd_vibrator_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei classd-vibrator driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
