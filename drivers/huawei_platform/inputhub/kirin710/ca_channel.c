/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2020. All rights reserved.
 * Description: CA Hub Channel driver
 * Author: luchangxue
 * Create: 2016-3-10
 */

#include <linux/err.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/uaccess.h>

#include <huawei_platform/inputhub/cahub.h>
#include <huawei_platform/inputhub/sensorhub.h>

#include "contexthub_boot.h"
#include "contexthub_route.h"
#include "protocol.h"
#include "sensor_info.h"

#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif

extern bool really_do_enable_disable(int *ref_cnt, bool enable, int bit);
extern int send_app_config_cmd(int tag, void *app_config, bool use_lock);
static bool ca_status[CA_TYPE_END];
static int ca_ref_cnt;

struct cas_cmd_map {
	int chb_ioctl_app_cmd;
	int ca_type;
	int tag;
	enum obj_cmd cmd;
	enum obj_sub_cmd subcmd;
};

static const struct cas_cmd_map cas_cmd_map_tab[] = {
	{ CHB_IOCTL_CA_START, -1, TAG_CA, CMD_CMN_OPEN_REQ, SUB_CMD_NULL_REQ },
	{ CHB_IOCTL_CA_STOP, -1, TAG_CA, CMD_CMN_CLOSE_REQ, SUB_CMD_NULL_REQ },
	{ CHB_IOCTL_CA_ATTR_START, -1, TAG_CA, CMD_CMN_CONFIG_REQ, SUB_CMD_CA_ATTR_ENABLE_REQ },
	{ CHB_IOCTL_CA_ATTR_STOP, -1, TAG_CA, CMD_CMN_CONFIG_REQ, SUB_CMD_CA_ATTR_DISABLE_REQ },
	{ CHB_IOCTL_CA_INTERVAL_SET, -1, TAG_CA, CMD_CMN_INTERVAL_REQ, SUB_CMD_NULL_REQ },
};

static void update_ca_info(enum obj_cmd cmd, ca_type_t type)
{
	switch (cmd) {
	case CMD_CMN_OPEN_REQ:
		ca_status[type] = true;
		break;

	case CMD_CMN_CLOSE_REQ:
		ca_status[type] = false;
		break;

	default:
		hwlog_err("unknown cmd type in %s\n", __func__);
		break;
	}
}

static int send_ca_cmd_internal(int tag, enum obj_cmd cmd, enum obj_sub_cmd subcmd,
	ca_type_t type, bool use_lock)
{
	uint8_t app_config[16] = {0}; /* 16:config num */
	interval_param_t interval_param;

	if (!(type >= CA_TYPE_START && type < CA_TYPE_END))
		return -EINVAL;

	app_config[0] = type;
	app_config[1] = cmd;

	memset(&interval_param, 0, sizeof(interval_param));
	update_ca_info(cmd, type);

	if (cmd == CMD_CMN_OPEN_REQ) {
		if (really_do_enable_disable(&ca_ref_cnt, true, type)) {
			if (use_lock) {
				inputhub_sensor_enable(tag, true);
				inputhub_sensor_setdelay(tag, &interval_param);
			} else {
				inputhub_sensor_enable_nolock(tag, true);
				inputhub_sensor_setdelay_nolock(tag,
					&interval_param);
			}
		}
		send_app_config_cmd(TAG_CA, app_config, use_lock);
	} else if (cmd == CMD_CMN_CLOSE_REQ) {
		send_app_config_cmd(TAG_CA, app_config, use_lock);

		if (really_do_enable_disable(&ca_ref_cnt, false, type)) {
			if (use_lock)
				inputhub_sensor_enable(tag, false);
			else
				inputhub_sensor_enable_nolock(tag, false);
		}
	} else {
		hwlog_err("%s unknown cmd\n", __func__);
		return -EINVAL;
	}
	return 0;
}

static int send_ca_cmd(unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)(uintptr_t)arg;
	int argvalue = 0;
	int i;

	if (get_flag_for_sensor_test())
		return 0;

	hwlog_info("%s enter\n", __func__);
	for (i = 0; i < sizeof(cas_cmd_map_tab) / sizeof(cas_cmd_map_tab[0]); ++i) {
		if (cmd == cas_cmd_map_tab[i].chb_ioctl_app_cmd)
			break;
	}

	if (i == sizeof(cas_cmd_map_tab) / sizeof(cas_cmd_map_tab[0])) {
		hwlog_err("%s unknown cmd %d in parse_ca_cmd\n", __func__, cmd);
		return -EFAULT;
	}

	if (copy_from_user(&argvalue, argp, sizeof(argvalue))) {
		hwlog_err("%s copy_from_user failed\n", __func__);
		return -EFAULT;
	}

	hwlog_info("%s leave before send_ca_cmd_internal\n", __func__);
	return send_ca_cmd_internal(cas_cmd_map_tab[i].tag, cas_cmd_map_tab[i].cmd, cas_cmd_map_tab[i].subcmd, argvalue, true);
}

static void enable_cas_when_recovery_iom3(void)
{
	ca_type_t type;

	ca_ref_cnt = 0; /* to send open motion cmd when open first type */
	for (type = CA_TYPE_START; type < CA_TYPE_END; ++type) {
		if (ca_status[type]) {
			hwlog_info("ca state %d in %s\n", type, __func__);
			send_ca_cmd_internal(TAG_CA, CMD_CMN_OPEN_REQ,
				SUB_CMD_NULL_REQ, type, false);
		}
	}
}

void disable_cas_when_sysreboot(void)
{
	ca_type_t type;

	for (type = CA_TYPE_START; type < CA_TYPE_END; ++type) {
		if (ca_status[type]) {
			hwlog_info("ca state %d in %s\n", type, __func__);
			send_ca_cmd_internal(TAG_CA, CMD_CMN_CLOSE_REQ,
				SUB_CMD_NULL_REQ, type, false);
		}
	}
}

static ssize_t chb_read(struct file *file, char __user *buf, size_t count,
	loff_t *pos)
{
	hwlog_info("enter %s\n", __func__);
	return inputhub_route_read(ROUTE_CA_PORT, buf, count);
}

static ssize_t chb_write(struct file *file, const char __user *data,
	size_t len, loff_t *ppos)
{
	return len;
}

static long chb_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	hwlog_info("%s cmd: %d\n", __func__, cmd);

	switch (cmd) {
	case CHB_IOCTL_CA_START:
	case CHB_IOCTL_CA_STOP:
	case CHB_IOCTL_CA_ATTR_START:
	case CHB_IOCTL_CA_ATTR_STOP:
	case CHB_IOCTL_CA_INTERVAL_SET:
		break;
	default:
		hwlog_err("%s unknown cmd : %d\n", __func__, cmd);
		return -ENOTTY;
	}

	return send_ca_cmd(cmd, arg);
}

static int chb_open(struct inode *inode, struct file *file)
{
	hwlog_info("enter %s\n", __func__);
	return 0;
}

static int chb_release(struct inode *inode, struct file *file)
{
	hwlog_info("enter %s\n", __func__);
	return 0;
}

static int ca_recovery_notifier(struct notifier_block *nb, unsigned long foo,
	void *bar)
{
	/* prevent access the emmc now */
	hwlog_info("%s %lu +\n", __func__, foo);
	switch (foo) {
	case IOM3_RECOVERY_IDLE:
	case IOM3_RECOVERY_START:
	case IOM3_RECOVERY_MINISYS:
	case IOM3_RECOVERY_3RD_DOING:
	case IOM3_RECOVERY_FAILED:
		break;
	case IOM3_RECOVERY_DOING:
		enable_cas_when_recovery_iom3();
		break;
	default:
		hwlog_err("%s -unknow state %ld\n", __func__, foo);
		break;
	}
	hwlog_info("%s -\n", __func__);
	return 0;
}

static struct notifier_block ca_recovery_notify = {
	.notifier_call = ca_recovery_notifier,
	.priority = -1,
};

static const struct file_operations chb_fops = {
	.owner             = THIS_MODULE,
	.llseek            = no_llseek,
	.read              = chb_read,
	.write             = chb_write,
	.unlocked_ioctl    = chb_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl      = chb_ioctl,
#endif
	.open              = chb_open,
	.release           = chb_release,
};

static struct miscdevice cahub_miscdev = {
	.minor =    MISC_DYNAMIC_MINOR,
	.name =     "cahub",
	.fops =     &chb_fops,
};

static int __init cahub_init(void)
{
	int ret;

	if (is_sensorhub_disabled())
		return -1;

	ret = inputhub_route_open(ROUTE_CA_PORT);
	if (ret != 0) {
		hwlog_err("%s inputhub route open err:%d\n", __func__, ret);
		return ret;
	}

	ret = misc_register(&cahub_miscdev);
	if (ret != 0) {
		hwlog_err("%s cannot register miscdev err=%d\n", __func__, ret);
		inputhub_route_close(ROUTE_CA_PORT);
		return ret;
	}
	register_iom3_recovery_notifier(&ca_recovery_notify);
	return ret;
}

static void __exit cahub_exit(void)
{
	hwlog_info("enter %s\n", __func__);
	inputhub_route_close(ROUTE_CA_PORT);
	misc_deregister(&cahub_miscdev);
	hwlog_info("exit %s\n", __func__);
}

late_initcall_sync(cahub_init);
module_exit(cahub_exit);

MODULE_AUTHOR("CAHub <changxue.lu@huawei.com>");
MODULE_DESCRIPTION("CAHub driver");
MODULE_LICENSE("GPL");
