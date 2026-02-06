/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * Description: Sensor Hub Channel driver
 * Author: qindiwen
 * Create: 2012-06-01
 */

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/uaccess.h>

#include <huawei_platform/inputhub/sensorhub.h>
#include <sensor_detect.h>

#include "als_detect.h"
#include "contexthub_boot.h"
#include "contexthub_route.h"
#include "sensor_config.h"
#include "sensor_info.h"

int first_start_flag;
int gyro_first_start_flag;
int handpress_first_start_flag;
extern struct mutex mutex_vibrator;

static bool sensor_switch_flags[TAG_SENSOR_END] = {false};
uint8_t tag_to_hal_sensor_type[TAG_SENSOR_END];
static uint8_t hal_sensor_type_to_tag[SENSORHUB_TYPE_END];
static DEFINE_MUTEX(mutex_status);

struct sensors_cmd_map {
	int hal_sensor_type;
	int tag;
};

static const struct sensors_cmd_map sensors_cmd_map_tab[] = {
	{ SENSORHUB_TYPE_ACCELEROMETER, TAG_ACCEL },
	{ SENSORHUB_TYPE_LIGHT, TAG_ALS },
	{ SENSORHUB_TYPE_PROXIMITY, TAG_PS },
	{ SENSORHUB_TYPE_GYROSCOPE, TAG_GYRO },
	{ SENSORHUB_TYPE_GRAVITY, TAG_GRAVITY },
	{ SENSORHUB_TYPE_MAGNETIC, TAG_MAG },
	{ SENSORHUB_TYPE_LINEARACCELERATE, TAG_LINEAR_ACCEL },
	{ SENSORHUB_TYPE_ORIENTATION, TAG_ORIENTATION },
	{ SENSORHUB_TYPE_ROTATEVECTOR, TAG_ROTATION_VECTORS },
	{ SENSORHUB_TYPE_PRESSURE, TAG_PRESSURE },
	{ SENSORHUB_TYPE_HALL, TAG_HALL },
	{ SENSORHUB_TYPE_MAGNETIC_FIELD_UNCALIBRATED, TAG_MAG_UNCALIBRATED },
	{ SENSORHUB_TYPE_GAME_ROTATION_VECTOR, TAG_GAME_RV },
	{ SENSORHUB_TYPE_GYROSCOPE_UNCALIBRATED, TAG_GYRO_UNCALIBRATED },
	{ SENSORHUB_TYPE_SIGNIFICANT_MOTION, TAG_SIGNIFICANT_MOTION },
	{ SENSORHUB_TYPE_STEP_DETECTOR, TAG_STEP_DETECTOR },
	{ SENSORHUB_TYPE_STEP_COUNTER, TAG_STEP_COUNTER },
	{ SENSORHUB_TYPE_GEOMAGNETIC_ROTATION_VECTOR, TAG_GEOMAGNETIC_RV },
	{ SENSORHUB_TYPE_HANDPRESS, TAG_HANDPRESS },
	{ SENSORHUB_TYPE_CAP_PROX, TAG_CAP_PROX },
	{ SENSORHUB_TYPE_PHONECALL, TAG_PHONECALL },
	{ SENSORHUB_TYPE_MAGN_BRACKET, TAG_MAGN_BRACKET },
	{ SENSORHUB_TYPE_HINGE, TAG_HINGE },
	{ SENSORHUB_TYPE_META_DATA, TAG_FLUSH_META },
	{ SENSORHUB_TYPE_RPC, TAG_RPC },
	{ SENSORHUB_TYPE_AGT, TAG_AGT },
	{ SENSORHUB_TYPE_COLOR, TAG_COLOR },
	{ SENSORHUB_TYPE_ACCELEROMETER_UNCALIBRATED, TAG_ACCEL_UNCALIBRATED },
};

static void init_hash_tables(void)
{
	int i;

	for (i = 0; i < sizeof(sensors_cmd_map_tab) / sizeof(sensors_cmd_map_tab[0]); ++i) {
		tag_to_hal_sensor_type[sensors_cmd_map_tab[i].tag] =
			sensors_cmd_map_tab[i].hal_sensor_type;
		hal_sensor_type_to_tag[sensors_cmd_map_tab[i].hal_sensor_type] =
			sensors_cmd_map_tab[i].tag;
	}
}

static bool switch_sensor(int tag, bool enable)
{
	bool result = false;

	mutex_lock(&mutex_status);
	if ((enable && !sensor_switch_flags[tag]) || (!enable)) {
		sensor_switch_flags[tag] = enable;
		result = true;
	}
	hwlog_info("%s ret is %d\n", __func__, result);
	mutex_unlock(&mutex_status);
	return result;
}

static bool ap_sensor_flush(int tag)
{
	struct sensor_data event;
	t_ap_sensor_ops_record *record = get_all_ap_sensor_operations();
	bool work_on_ap = record[tag].work_on_ap;

	if (work_on_ap) {
		if (record[tag].ops.enable) {
			record[tag].ops.enable(1);
			msleep(60); /* sleep 60 ms */
		}
		event.type = tag_to_hal_sensor_type[TAG_FLUSH_META];
		event.length = 4; /* 4 : sensor data length */
		event.value[0] = tag_to_hal_sensor_type[tag];
		return inputhub_route_write_batch(ROUTE_SHB_PORT,
			(char *)&event, event.length +
			offset_of_end_mem(struct sensor_data, length), 0);
	}

	return work_on_ap;
}

static int send_sensor_batch_flush_cmd(unsigned int cmd,
	struct ioctl_para *para, int tag)
{
	struct write_info winfo;
	uint32_t subcmd;
	interval_param_t batch_param;

	memset(&winfo, 0, sizeof(winfo));
	if (cmd == SHB_IOCTL_APP_SENSOR_BATCH) {
		batch_param.mode = AUTO_MODE;
		batch_param.period = para->period_ms;

		if (para->period_ms != 0)
			batch_param.batch_count =
				(para->timeout_ms > para->period_ms) ?
				(para->timeout_ms / para->period_ms) : 1;
		else
			batch_param.batch_count = 1;

		hwlog_info("%s batch period=%d, count=%d\n", __func__,
			para->period_ms, batch_param.batch_count);
		return inputhub_sensor_setdelay(tag, &batch_param);
	} else if (cmd == SHB_IOCTL_APP_SENSOR_FLUSH) {
		hwlog_info("flush in %s\n", __func__);
		ap_sensor_flush(tag);
		winfo.cmd = CMD_CMN_CONFIG_REQ;
		winfo.tag = tag;
		subcmd = SUB_CMD_FLUSH_REQ;
		winfo.wr_buf = &subcmd;
		winfo.wr_len = SUBCMD_LEN;

		hwlog_info("flush sensor %s tag:%d\n",
			obj_tag_str[tag] ? obj_tag_str[tag] : "TAG_UNKNOWN",
			tag);
		write_customize_cmd(&winfo, NULL, true);
	}
	return 0;
}

static int send_sensor_cmd(unsigned int cmd, unsigned long arg)
{
	uint8_t app_config[16] = {0};
	void __user *argp = (void __user *)(uintptr_t)arg;
	int tag;
	int ret;
	struct ioctl_para para;
	interval_param_t delay_param;

	if (get_flag_for_sensor_test())
		return 0;

	if (copy_from_user(&para, argp, sizeof(para)))
		return -EFAULT;

	delay_param.period = para.delay_ms;
	delay_param.batch_count = 1;
	delay_param.mode = AUTO_MODE;
	delay_param.reserved[0] = TYPE_STANDARD; /* for step counter only */

	if (!(para.shbtype >= SENSORHUB_TYPE_BEGIN &&
		para.shbtype < SENSORHUB_TYPE_END)) {
		hwlog_err("error shbtype %d in %s\n", para.shbtype, __func__);
		return -EINVAL;
	}

	tag = hal_sensor_type_to_tag[para.shbtype];
	switch (cmd) {
	case SHB_IOCTL_APP_ENABLE_SENSOR:
		break;
	case SHB_IOCTL_APP_DISABLE_SENSOR:
		switch_sensor(tag, false);
		if (tag == TAG_STEP_COUNTER)
			return inputhub_sensor_enable_stepcounter(false,
				TYPE_STANDARD);
		else
			return inputhub_sensor_enable(tag, false);
		/* fall-through */
	case SHB_IOCTL_APP_DELAY_SENSOR:
		if (switch_sensor(tag, true)) {
			if (tag == TAG_STEP_COUNTER)
				ret = inputhub_sensor_enable_stepcounter(false,
					TYPE_STANDARD);
			else
				ret = inputhub_sensor_enable(tag, true);
			if (ret) {
				hwlog_err("enable sensor tag:%d err\n", tag);
				return -1;
			}
		}
		if (tag == TAG_STEP_COUNTER) {
			inputhub_sensor_setdelay(tag, &delay_param);
			app_config[0] = 1;
			app_config[1] = TYPE_STANDARD;
			return send_app_config_cmd(TAG_STEP_COUNTER,
				app_config, true);
		} else {
			return inputhub_sensor_setdelay(tag, &delay_param);
		}
		/* fall-through */
	case SHB_IOCTL_APP_SENSOR_BATCH:
		if (switch_sensor(tag, true) &&
			inputhub_sensor_enable(tag, true)) {
			hwlog_err("failed to enable sensor tag %d\n", tag);
			return -1;
		}
		/* fall-through */
	case SHB_IOCTL_APP_SENSOR_FLUSH:
		if (tag == TAG_STEP_COUNTER) {
			ret = send_sensor_batch_flush_cmd(cmd, &para, tag);
			if (ret == 0) {
				app_config[0] = 1;
				app_config[1] = TYPE_STANDARD;
				return send_app_config_cmd(TAG_STEP_COUNTER,
					app_config, true);
			} else {
				hwlog_err("%s failed %d\n", __func__, cmd);
				return ret;
			}
		} else {
			return send_sensor_batch_flush_cmd(cmd, &para, tag);
		}
		/* fall-through */
	default:
		hwlog_err("unknown shb_hal_cmd %d in %s\n", cmd, __func__);
		return -EINVAL;
	}

	return 0;
}

static ssize_t shb_read(struct file *file, char __user *buf, size_t count,
	loff_t *pos)
{
	return inputhub_route_read(ROUTE_SHB_PORT, buf, count);
}

static ssize_t shb_write(struct file *file, const char __user *data,
	size_t len, loff_t *ppos)
{
	hwlog_info("%s need to do...\n", __func__);
	return len;
}
static void get_acc_calibrate_data(void)
{
	int ret;

	if (first_start_flag == 0) {
		ret = send_gsensor_calibrate_data_to_mcu();
		if (ret)
			hwlog_err("%s read from nv err,ret=%d", __func__, ret);
		else
			hwlog_info("read nv success\n");
	}
}

static void get_vibrator_calibrate_data(void)
{
	int ret;
	static int vib_first_flag;

	if (strlen(get_sensor_chip_info_address(VIBRATOR)) == 0)
		return;

	if (vib_first_flag == 0) {
		ret = send_vibrator_calibrate_data_to_mcu();
		if (ret)
			hwlog_err("%s read from nv err,ret=%d", __func__, ret);
		else
			hwlog_info("%s read nv success\n", __func__);

		vib_first_flag = 1;
	}
}

static void get_mag_calibrate_data(void)
{
	int ret;
	static int mag_first_start_flag;

	if (mag_first_start_flag == 0) {
		mag_current_notify();
		ret = send_mag_calibrate_data_to_mcu();
		if (ret)
			hwlog_err("%s read mag from nv,send iom3 err,ret=%d\n",
				__func__, ret);
		else
			hwlog_info("%s read mag from nv,send to iom3 succ\n",
				__func__);
		mag_first_start_flag = 1;
	}
}

static void get_cap_prox_calibrate_data(void)
{
	int ret;
	static int cap_prox_first_start_flag;

	if (cap_prox_first_start_flag == 0) {
		ret = send_cap_prox_calibrate_data_to_mcu();
		if (ret)
			hwlog_err("%s read cap_prox,send iom3 err,ret=%d\n",
				__func__, ret);
		else
			hwlog_info("%s read cap_prox from nv,send iom3 succ\n",
				__func__);
		cap_prox_first_start_flag = 1;
	}
}

static void get_airpress_calibrate_data(void)
{
	int ret;
	static int airpress_first_start_flag;

	if (strlen(get_sensor_chip_info_address(AIRPRESS)) == 0)
		return;
	if (airpress_first_start_flag == 0) {
		ret = send_airpress_calibrate_data_to_mcu();
		if (ret) {
			hwlog_err("%s read airpress,send iom3 err,ret=%d\n",
				__func__, ret);
		} else {
			hwlog_info("%s read airpress from nv,send iom3 succ\n",
				__func__);
			airpress_first_start_flag = 1;
		}
	}
}

static void get_psensor_calibrate_data(void)
{
	struct ps_device_info *dev_info = NULL;
	int ret;

	dev_info = ps_get_device_info(TAG_PS);
	if (!dev_info)
		return;
	if (dev_info->ps_first_start_flag == 0 &&
		(dev_info->chip_type != PS_CHIP_NONE ||
		ps_external_ir_calibrate_flag == 1)) {
		ret = send_ps_calibrate_data_to_mcu();
		if (ret)
			hwlog_err("%s from nv fail, ret=%d", __func__, ret);
		else
			hwlog_info("read nv success\n");
	}

	if (dev_info->ps_first_start_flag == 0 && get_sensor_tof_flag() == 1) {
		ret = send_tof_calibrate_data_to_mcu();
		if (ret)
			hwlog_err("get_tof_calibrate_data from nv err,ret=%d",
				ret);
		else
			hwlog_info("tof read nv success\n");
	}
}

static void get_als_calibrate_data(void)
{
	int ret;
	uint32_t t;
	struct als_device_info *dev_info = NULL;

	dev_info = als_get_device_info(TAG_ALS);
	if (!dev_info) {
		hwlog_err("%s: dev_info is NULL\n", __func__);
		return;
	}

	t = dev_info->chip_type;
	if ((dev_info->als_first_start_flag == 0) && ((t == ALS_CHIP_ROHM_RGB) ||
		(t == ALS_CHIP_AVAGO_RGB) || (t == ALS_CHIP_AMS_TMD3725_RGB) ||
		(t == ALS_CHIP_LITEON_LTR582) || (dev_info->is_cali_supported == 1) ||
		(t == ALS_CHIP_APDS9999_RGB) || (t == ALS_CHIP_AMS_TMD3702_RGB) ||
		(t == ALS_CHIP_APDS9253_RGB) || (t == ALS_CHIP_VISHAY_VCNL36658))) {
		ret = send_als_calibrate_data_to_mcu();
		if (ret)
			hwlog_err("%s read from nv err,ret=%d", __func__, ret);
		else
			hwlog_info("read nv success\n");

		ret = get_tpcolor_from_nv();
		if (ret)
			hwlog_err("get_tpcolor_from_nv fail, ret=%d\n", ret);
		else
			hwlog_info("get_tpcolor_from_nv success\n");
	}
}

static void get_gyro_calibrate_data(void)
{
	int ret;

	if (strlen(get_sensor_chip_info_address(GYRO)) == 0)
		return;

	if (gyro_first_start_flag == 0) {
		ret = send_gyro_calibrate_data_to_mcu();
		if (ret)
			hwlog_err("%s read from nv err,ret=%d", __func__, ret);
		else
			hwlog_info("gyro_sensor read nv success\n");
		ret = send_gyro_temperature_offset_to_mcu();
		if (ret)
			hwlog_err("get gyro temper cali data err,ret=%d", ret);
		else
			hwlog_info("gyro read temper offset from nv succ\n");
	}
}

static void get_handpress_calibrate_data(void)
{
	int ret;

	if (strlen(get_sensor_chip_info_address(HANDPRESS)) == 0)
		return;
	if (handpress_first_start_flag == 0) {
		ret = send_handpress_calibrate_data_to_mcu();
		if (ret)
			hwlog_err("handpress read from nv fail, ret=%d", ret);
		else
			hwlog_info("handpress read nv success\n");
	}
}

static void send_sensor_calib_data(void)
{
	get_acc_calibrate_data();
	get_mag_calibrate_data();
	get_cap_prox_calibrate_data();
	get_airpress_calibrate_data();
	get_psensor_calibrate_data();
	get_als_calibrate_data();
	get_gyro_calibrate_data();
	get_handpress_calibrate_data();
	get_vibrator_calibrate_data();
}

static void send_sensor_add_data(void)
{
	int ret;
	static int sar_first_start_flag;

	if (sar_first_start_flag == 0) {
		ret = send_sar_add_data_to_mcu();
		if (ret)
			hwlog_err("%s send_sar_add_data_to_mcu err, ret=%d\n",
				__func__, ret);
		else
			hwlog_info("%s send_sar_add_data succ\n", __func__);
		sar_first_start_flag = 1;
	}
}

static long shb_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)(uintptr_t)arg;
	int sensor_mcu_mode;

	switch (cmd) {
	case SHB_IOCTL_APP_ENABLE_SENSOR:
	case SHB_IOCTL_APP_DISABLE_SENSOR:
	case SHB_IOCTL_APP_DELAY_SENSOR:
		break;
	case SHB_IOCTL_APP_GET_SENSOR_MCU_MODE:
		sensor_mcu_mode = get_sensor_mcu_mode();
		hwlog_info("isSensorMcuMode %d\n", sensor_mcu_mode);
		if (copy_to_user(argp, &sensor_mcu_mode, sizeof(sensor_mcu_mode)))
			return -EFAULT;
		return 0;
	case SHB_IOCTL_APP_SENSOR_BATCH:
		hwlog_info("shb ioctl : flush SHB_IOCTL_APP_SENSOR_BATCH\n");
		break;
	case SHB_IOCTL_APP_SENSOR_FLUSH:
		hwlog_info("shb ioctl : flush SHB_IOCTL_APP_SENSOR_FLUSH\n");
		break;
	default:
		hwlog_err("shb ioctl unknown cmd : %d\n", cmd);
		return -ENOTTY;
	}
	send_sensor_calib_data();
	send_sensor_add_data(); /* addinitial data */
	return send_sensor_cmd(cmd, arg);
}

static int shb_open(struct inode *inode, struct file *file)
{
	hwlog_info("%s ok\n", __func__);
	return 0;
}

static int shb_release(struct inode *inode, struct file *file)
{
	hwlog_info("%s ok\n", __func__);
	return 0;
}

static void enable_sensors_when_recovery_iom3(void)
{
	int tag;
	interval_param_t interval_param;

	for (tag = TAG_SENSOR_BEGIN; tag < TAG_SENSOR_END; ++tag) {
		if (sensor_status.status[tag]) {
			interval_param.period = sensor_status.delay[tag]; /* default delay_ms */
			interval_param.batch_count =
				sensor_status.batch_cnt[tag];
			interval_param.mode = AUTO_MODE;
			interval_param.reserved[0] = TYPE_STANDARD;
			inputhub_sensor_enable_nolock(tag, true);
			inputhub_sensor_setdelay_nolock(tag, &interval_param);
		}
	}
}

static int sensor_recovery_notifier(struct notifier_block *nb,
	unsigned long foo, void *bar)
{
	hwlog_info("%s %lu +\n", __func__, foo);
	switch (foo) {
	case IOM3_RECOVERY_IDLE:
	case IOM3_RECOVERY_START:
	case IOM3_RECOVERY_MINISYS:
	case IOM3_RECOVERY_3RD_DOING:
	case IOM3_RECOVERY_FAILED:
		break;
	case IOM3_RECOVERY_DOING:
		reset_calibrate_data();
		reset_add_data();
		enable_sensors_when_recovery_iom3();
		break;
	default:
		hwlog_err("%s -unknow state %ld\n", __func__, foo);
		break;
	}
	hwlog_info("%s -\n", __func__);
	return 0;
}

static struct notifier_block sensor_recovery_notify = {
	.notifier_call = sensor_recovery_notifier,
	.priority = -1,
};

static const struct file_operations shb_fops = {
	.owner          = THIS_MODULE,
	.llseek         = no_llseek,
	.read           = shb_read,
	.write          = shb_write,
	.unlocked_ioctl = shb_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl   = shb_ioctl,
#endif
	.open           = shb_open,
	.release        = shb_release,
};

static struct miscdevice senorhub_miscdev = {
	.minor =    MISC_DYNAMIC_MINOR,
	.name =     "sensorhub",
	.fops =     &shb_fops,
};

static int __init sensorhub_init(void)
{
	int ret;

	if (is_sensorhub_disabled())
		return -1;

	init_hash_tables();
	ret = inputhub_route_open(ROUTE_SHB_PORT);
	if (ret != 0) {
		hwlog_err("cannot open inputhub route err=%d\n", ret);
		return ret;
	}

	ret = misc_register(&senorhub_miscdev);
	if (ret != 0) {
		inputhub_route_close(ROUTE_SHB_PORT);
		hwlog_err("cannot register miscdev err=%d\n", ret);
		return ret;
	}
	register_iom3_recovery_notifier(&sensor_recovery_notify);
	return 0;
}

static void __exit sensorhub_exit(void)
{
	inputhub_route_close(ROUTE_SHB_PORT);
	misc_deregister(&senorhub_miscdev);
	hwlog_info("exit %s\n", __func__);
}

late_initcall_sync(sensorhub_init);
module_exit(sensorhub_exit);

MODULE_AUTHOR("SensorHub <smartphone@huawei.com>");
MODULE_DESCRIPTION("SensorHub driver");
MODULE_LICENSE("GPL");
