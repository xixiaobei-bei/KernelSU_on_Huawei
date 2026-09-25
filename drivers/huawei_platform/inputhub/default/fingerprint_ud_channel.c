/*
 * fingerprint_ud_channel.c
 *
 * Fingerprint UD Hub Channel driver
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

#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <huawei_platform/inputhub/fingerprinthub.h>

#include "contexthub_route.h"
#include "contexthub_boot.h"
#include "protocol.h"

#define CA_TYPE_DEFAULT                  (-1)
#define FHB_CONFIG_UD_OPTICAL_BRIGHTNESS 14
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

static int fp_ref_cnt;
static bool fingerprint_status[FINGERPRINT_TYPE_END];

static uint8_t g_optical_brightness[5]; /* 5 : max brightness num */
static uint8_t g_optical_brightness_len;

extern int flag_for_sensor_test;

extern bool really_do_enable_disable(int *ref_cnt, bool enable, int bit);
extern int send_app_config_cmd_with_resp(int tag,
	void *app_config, bool use_lock);
extern void inputhub_route_clean_buffer(unsigned short port);

struct fingerprint_cmd_map {
	int fhb_ioctl_app_cmd;
	int ca_type;
	int tag;
	obj_cmd_t cmd;
};

static const struct fingerprint_cmd_map fingerprint_cmd_map_tab[] = {
	{ FHB_IOCTL_FP_START, CA_TYPE_DEFAULT, TAG_FP_UD, CMD_CMN_OPEN_REQ },
	{ FHB_IOCTL_FP_STOP, CA_TYPE_DEFAULT, TAG_FP_UD, CMD_CMN_CLOSE_REQ },
	{ FHB_IOCTL_FP_DISABLE_SET, CA_TYPE_DEFAULT,
		TAG_FP_UD, FHB_IOCTL_FP_DISABLE_SET_CMD },
};

static void update_fingerprint_info(obj_cmd_t cmd, fingerprint_type_t type)
{
	switch (cmd) {
	case CMD_CMN_OPEN_REQ:
		fingerprint_status[type] = true;
		hwlog_err("fingerprint: CMD_CMN_OPEN_REQ in %s, type:%d, %d\n",
			__func__, type, fingerprint_status[type]);
		break;
	case CMD_CMN_CLOSE_REQ:
		fingerprint_status[type] = false;
		hwlog_err("fingerprint: CMD_CMN_CLOSE_REQ in %s, type:%d, %d\n",
			__func__, type, fingerprint_status[type]);
		break;
	default:
		hwlog_err("fingerprint: unknown cmd type in %s, type:%d\n",
			__func__, type);
		break;
	}
}

static void fingerprint_report(void)
{
	char *fingerprint_data = NULL;
	fingerprint_upload_pkt_t fingerprint_upload;

	memset(&fingerprint_upload, 0, sizeof(fingerprint_upload));
	fingerprint_upload.fhd.hd.tag = TAG_FP_UD;
	fingerprint_upload.fhd.hd.cmd = CMD_DATA_REQ;
	fingerprint_upload.data = 0; /* 0: cancel wait sensorhub msg */
	fingerprint_data = (char *)(&fingerprint_upload) +
		sizeof(pkt_common_data_t);
	inputhub_route_write(ROUTE_FHB_UD_PORT, fingerprint_data,
		sizeof(fingerprint_upload.data));
}

static int send_fingerprint_cmd_internal(int tag, obj_cmd_t cmd,
	fingerprint_type_t type, bool use_lock)
{
	interval_param_t interval_param;
	uint8_t app_config[16] = { 0 }; /* 16 : max cmd array length */

	memset(&interval_param, 0, sizeof(interval_param));
	update_fingerprint_info(cmd, type);
	if (cmd == CMD_CMN_OPEN_REQ) {
		if (!really_do_enable_disable(&fp_ref_cnt, true, type))
			return 0;
		inputhub_route_clean_buffer(ROUTE_FHB_UD_PORT);
		app_config[0] = SUB_CMD_FINGERPRINT_OPEN_REQ;
		if (use_lock) {
			inputhub_sensor_enable(tag, true);
			send_app_config_cmd_with_resp(tag, app_config, true);
		} else {
			inputhub_sensor_enable_nolock(tag, true);
			send_app_config_cmd_with_resp(tag, app_config, false);
		}
		hwlog_info("fingerprint:%s:CMD_CMN_OPEN_REQ cmd:%d\n",
			__func__, cmd);
	} else if (cmd == CMD_CMN_CLOSE_REQ) {
		if (!really_do_enable_disable(&fp_ref_cnt, false, type))
			return 0;
		app_config[0] = SUB_CMD_FINGERPRINT_CLOSE_REQ;
		if (use_lock) {
			send_app_config_cmd_with_resp(tag, app_config, true);
			inputhub_sensor_enable(tag, false);
		} else {
			send_app_config_cmd_with_resp(tag, app_config, false);
			inputhub_sensor_enable_nolock(tag, false);
		}
		hwlog_info("fingerprint:%s: CMD_CMN_CLOSE_REQ cmd:%d\n",
			__func__, cmd);
	} else if (cmd == FHB_IOCTL_FP_DISABLE_SET_CMD) {
		fingerprint_report();
		hwlog_info("fingerprint:%s: CMD_FINGERPRINT_DISABLE_SET cmd:%d\n",
			__func__, cmd);
	} else {
		hwlog_err("fingerprint:%s: unknown cmd\n", __func__);
		return -EINVAL;
	}

	return 0;
}

static int send_fingerprint_cmd(unsigned int cmd, unsigned long arg)
{
	uintptr_t arg_tmp = (uintptr_t)arg;
	void __user *argp = (void __user *)arg_tmp;
	int arg_value = 0;
	int i;
	const int len = ARRAY_SIZE(fingerprint_cmd_map_tab);

	if (flag_for_sensor_test)
		return 0;

	hwlog_info("fingerprint:%s enter\n", __func__);

	for (i = 0; i < len; i++) {
		if (cmd == fingerprint_cmd_map_tab[i].fhb_ioctl_app_cmd)
			break;
	}
	if (i == len) {
		hwlog_err("fingerprint:%s unknown cmd %d in parse_ca_cmd\n",
			__func__, cmd);
		return -EFAULT;
	}
	if (copy_from_user(&arg_value, argp, sizeof(arg_value))) {
		hwlog_err("fingerprint:%s copy_from_user failed\n", __func__);
		return -EFAULT;
	}

	if (!(arg_value >= FINGERPRINT_TYPE_START &&
		arg_value < FINGERPRINT_TYPE_END)) {
		hwlog_err("error fingerprint type %d in %s\n", arg_value,
			__func__);
		return -EINVAL;
	}

	return send_fingerprint_cmd_internal(
		fingerprint_cmd_map_tab[i].tag,
		fingerprint_cmd_map_tab[i].cmd, arg_value, true);
}

static int fingerprint_recovery_config_para(const void *data, int len)
{
	int ret;
	fingerprint_req_t fp_pkt;
	write_info_t pkg_ap;

	memset(&fp_pkt, 0, sizeof(fp_pkt));
	memset(&pkg_ap, 0, sizeof(pkg_ap));

	if (len > sizeof(fp_pkt.buf)) {
		hwlog_warn("fingerprint:%s len is out of size, len=%d\n",
			__func__, len);
		return -EINVAL;
	}

	memcpy(&fp_pkt.buf[0], data, len);

	fp_pkt.len = len;
	fp_pkt.sub_cmd = SUB_CMD_FINGERPRINT_CONFIG_SENSOR_DATA_REQ;

	hwlog_info("fingerprint:%s =%d, len=%d\n", __func__,
		fp_pkt.buf[0], len);

	pkg_ap.tag = TAG_FP_UD;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkg_ap.wr_buf = &fp_pkt;
	pkg_ap.wr_len = sizeof(fp_pkt);
	ret = write_customize_cmd(&pkg_ap, NULL, false);
	if (ret)
		hwlog_err("fhb_ud_write fail,ret=%d\n", ret);

	return ret;
}

static void enable_fingerprint_when_recovery_iom3(void)
{
	fingerprint_type_t type;

	fp_ref_cnt = 0;
	fingerprint_recovery_config_para(g_optical_brightness,
		g_optical_brightness_len);
	for (type = FINGERPRINT_TYPE_START;
		type < FINGERPRINT_TYPE_END; ++type) {
		if (fingerprint_status[type]) {
			hwlog_info("fingerprint: finger state %d in %s\n",
				type, __func__);
			send_fingerprint_cmd_internal(TAG_FP_UD,
				CMD_CMN_OPEN_REQ, type, false);
		}
	}
}

void disable_fingerprint_ud_when_sysreboot(void)
{
	fingerprint_type_t type;

	for (type = FINGERPRINT_TYPE_START;
		type < FINGERPRINT_TYPE_END; ++type) {
		if (fingerprint_status[type]) {
			hwlog_info("fingerprint: finger state %d in %s\n",
				type, __func__);
			send_fingerprint_cmd_internal(TAG_FP_UD,
				CMD_CMN_CLOSE_REQ, type, false);
		}
	}
}

/*
 * Description:    read /dev/fingerprinthub_ud
 * Return:         length of read data
 */
static ssize_t fhb_ud_read(struct file *file, char __user *buf,
	size_t count, loff_t *pos)
{
	return inputhub_route_read(ROUTE_FHB_UD_PORT, buf, count);
}

/*
 * Description:    write to /dev/fingerprinthub_ud, do nothing now
 * Return:         length of write data
 */
static ssize_t fhb_ud_write(struct file *file, const char __user *data,
	size_t len, loff_t *ppos)
{
	int ret;
	fingerprint_req_t fp_pkt;
	write_info_t pkg_ap;

	memset(&fp_pkt, 0, sizeof(fp_pkt));
	memset(&pkg_ap, 0, sizeof(pkg_ap));

	if (len > sizeof(fp_pkt.buf)) {
		hwlog_warn("fingerprint:%s len is out of size, len=%lu\n",
			__func__, len);
		return -EINVAL;
	}
	if (copy_from_user(fp_pkt.buf, data, len)) {
		hwlog_warn("fingerprint:%s copy_from_user failed\n", __func__);
		return -EFAULT;
	}

	if (fp_pkt.buf[0] == FHB_CONFIG_UD_OPTICAL_BRIGHTNESS) {
		g_optical_brightness_len = MIN(len,
			sizeof(g_optical_brightness));
		memcpy(g_optical_brightness, fp_pkt.buf,
			g_optical_brightness_len);
	}

	fp_pkt.len = len;
	fp_pkt.sub_cmd = SUB_CMD_FINGERPRINT_CONFIG_SENSOR_DATA_REQ;

	hwlog_info("fingerprint:%s data=%d, len=%lu\n",
		__func__, fp_pkt.buf[0], len);

	pkg_ap.tag = TAG_FP_UD;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkg_ap.wr_buf = &fp_pkt;
	pkg_ap.wr_len = sizeof(fp_pkt);
	ret = write_customize_cmd(&pkg_ap, NULL, true);
	if (ret)
		hwlog_err("%s fail,ret=%d\n", __func__, ret);

	return len;
}

/*
 * Description:    ioctrl function to /dev/fingerprinthub_ud, do open,
 *                 close ca, or set interval and attribute to fingerprinthub_ud
 * Return:         result of ioctrl command, 0 successed, -ENOTTY failed
 */
static long fhb_ud_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	switch (cmd) {
	case FHB_IOCTL_FP_START:
		hwlog_err("fingerprint:%s cmd: FHB_IOCTL_FP_START\n", __func__);
		break;
	case FHB_IOCTL_FP_STOP:
		hwlog_err("fingerprint:%s cmd: FHB_IOCTL_FP_STOP\n", __func__);
		break;
	case FHB_IOCTL_FP_DISABLE_SET:
		hwlog_err("fingerprint:%s set cmd : FHB_IOCTL_FP_DISABLE_SET\n",
			__func__);
		break;
	default:
		hwlog_err("fingerprint:%s unknown cmd : %d\n", __func__, cmd);
		return -ENOTTY;
	}
	return send_fingerprint_cmd(cmd, arg);
}

/*
 * Description:    open to /dev/fingerprinthub_ud
 * Return:         result 0 successed
 */
static int fhb_ud_open(struct inode *inode, struct file *file)
{
	hwlog_info("fingerprint: enter %s\n", __func__);
	return 0;
}

static int fingerprint_recovery_notifier(struct notifier_block *nb,
	unsigned long foo, void *bar)
{
	/* prevent access the emmc now: */
	hwlog_info("%s %lu enter\n", __func__, foo);
	switch (foo) {
	case IOM3_RECOVERY_IDLE:
	case IOM3_RECOVERY_START:
	case IOM3_RECOVERY_MINISYS:
	case IOM3_RECOVERY_3RD_DOING:
	case IOM3_RECOVERY_FAILED:
		break;
	case IOM3_RECOVERY_DOING:
		enable_fingerprint_when_recovery_iom3();
		break;
	default:
		hwlog_err("%s -unknow state %ld\n", __func__, foo);
		break;
	}
	hwlog_info("%s out\n", __func__);
	return 0;
}

static struct notifier_block fingerprint_recovery_notify = {
	.notifier_call = fingerprint_recovery_notifier,
	.priority = -1,
};

/* file_operations to fingerprinthub_ud */
static const struct file_operations fhb_ud_fops = {
	.owner             = THIS_MODULE,
	.read              = fhb_ud_read,
	.write             = fhb_ud_write,
	.unlocked_ioctl    = fhb_ud_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl      = fhb_ud_ioctl,
#endif
	.open              = fhb_ud_open,
};

/* miscdevice to fingerprinthub_ud */
static struct miscdevice fingerprinthub_ud_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "fingerprinthub_ud",
	.fops = &fhb_ud_fops,
};

/*
 * Description:    apply kernel buffer, register fingerprinthub_ud_miscdev
 * Return:         result of function, 0 successed, else false
 */
static int __init fingerprinthub_ud_init(void)
{
	int ret;

	if (is_sensorhub_disabled())
		return -1;

	ret = inputhub_route_open(ROUTE_FHB_UD_PORT);
	if (ret != 0) {
		hwlog_err("fingerprint: %s cannot open inputhub route err=%d\n",
			__func__, ret);
		return ret;
	}

	ret = misc_register(&fingerprinthub_ud_miscdev);
	if (ret != 0) {
		hwlog_err("%s cannot register miscdev err=%d\n", __func__, ret);
		inputhub_route_close(ROUTE_FHB_UD_PORT);
		return ret;
	}
	register_iom3_recovery_notifier(&fingerprint_recovery_notify);
	hwlog_info("%s ok\n", __func__);

	return ret;
}

/* release kernel buffer, deregister fingerprinthub_ud_miscdev */
static void __exit fingerprinthub_ud_exit(void)
{
	inputhub_route_close(ROUTE_FHB_UD_PORT);
	misc_deregister(&fingerprinthub_ud_miscdev);
	hwlog_info("exit %s\n", __func__);
}

late_initcall_sync(fingerprinthub_ud_init);
module_exit(fingerprinthub_ud_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("FPHub driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
