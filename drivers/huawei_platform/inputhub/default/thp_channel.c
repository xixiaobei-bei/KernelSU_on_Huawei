/*
 * drivers/inputhub/thp_channel.c
 *
 * THP Channel driver
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
#include <huawei_platform/inputhub/thphub.h>
#include "contexthub_route.h"
#include "contexthub_boot.h"
#include "protocol.h"

#ifdef CONFIG_CONTEXTHUB_SHMEM
#include "shmem.h"
#endif

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG thp_channel
HWLOG_REGIST();

extern int flag_for_sensor_test;
extern int g_iom3_state;
extern int iom3_power_state;

static thp_status_data g_thp_status;
static uint8_t g_thp_open_flag;

struct thp_cmd_map {
	unsigned int thp_ioctl_app_cmd;
	int tag;
	obj_sub_cmd_t cmd;
};

static const struct thp_cmd_map thp_cmd_map_tab[] = {
	{ THP_IOCTL_THP_DEBUG_CMD, TAG_THP, SUB_CMD_THP_DEBUG_REQ },
	{ THP_IOCTL_THP_CONFIG_CMD, TAG_THP, SUB_CMD_THP_CONFIG_REQ },
	{ THP_IOCTL_THP_OPEN_CMD, TAG_THP, SUB_CMD_THP_OPEN_REQ },
	{ THP_IOCTL_THP_CLOSE_CMD, TAG_THP, SUB_CMD_THP_CLOSE_REQ },
	{ THP_IOCTL_THP_STATUS_CMD, TAG_THP, SUB_CMD_THP_STATUS_REQ },
};

static int send_thp_switch_cmd(int tag, obj_sub_cmd_t subcmd, bool use_lock)
{
	write_info_t pkg_ap;
	pkt_parameter_req_t cpkt;
	pkt_header_t *hd = (pkt_header_t *)&cpkt;
	obj_cmd_t cmd;
	int ret;

	memset(&pkg_ap, 0, sizeof(pkg_ap));
	cmd = (subcmd == SUB_CMD_THP_OPEN_REQ) ?
			CMD_CMN_OPEN_REQ : CMD_CMN_CLOSE_REQ;

	pkg_ap.cmd = cmd;
	pkg_ap.tag = tag;
	cpkt.subcmd = subcmd;
	pkg_ap.wr_buf = &hd[1];
	pkg_ap.wr_len = SUBCMD_LEN;
	hwlog_info("thp: %s cmd %d, tag %d sub cmd: %u\n", __func__,
		pkg_ap.cmd, pkg_ap.tag, cpkt.subcmd);
	if ((g_iom3_state == IOM3_ST_RECOVERY) ||
		(iom3_power_state == ST_SLEEP))
		ret = write_customize_cmd(&pkg_ap, NULL, false);
	else
		ret = write_customize_cmd(&pkg_ap, NULL, true);
	if (ret < 0) {
		hwlog_err("thp: %s write cmd err\n", __func__);
		return -1;
	}

	return 0;
}

int send_thp_open_cmd(void)
{
	g_thp_open_flag = 1;
	return send_thp_switch_cmd(TAG_THP, SUB_CMD_THP_OPEN_REQ, true);
}

static int send_thp_config_cmd(unsigned int subcmd, int inpara_len,
	const char *inpara, struct read_info *pkg_mcu, bool use_lock)
{
	write_info_t pkg_ap;
	pkt_parameter_req_t cpkt;
	pkt_header_t *hd = (pkt_header_t *)&cpkt;
	int ret;

	if (!(inpara) || (inpara_len <= 0)) {
		hwlog_err("%s para failed\n", __func__);
		return -1;
	}

	memset(&pkg_ap, 0, sizeof(pkg_ap));

	if (THP_IS_INVALID_BUFF(inpara, inpara_len,
		(MAX_PKT_LENGTH - SUBCMD_LEN - (int)sizeof(pkt_header_t)))) {
		hwlog_err("%s: subcmd: %u, inpara len: %d, maxlen: 128\n",
			__func__, subcmd, inpara_len);
		return -EINVAL;
	}

	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkg_ap.tag = TAG_THP;
	cpkt.subcmd = subcmd;
	pkg_ap.wr_buf = &hd[1];
	pkg_ap.wr_len = SUBCMD_LEN + inpara_len;
	if (inpara_len > 0)
		memcpy(cpkt.para, inpara, (unsigned int)inpara_len);

	hwlog_info("%s: cmd %d, tag %d, subcmd %u, para0 %u, para1 %u\n",
		__func__, pkg_ap.cmd, pkg_ap.tag, subcmd,
		(unsigned int)(cpkt.para[0]), (unsigned int)(cpkt.para[1]));
	if ((g_iom3_state == IOM3_ST_RECOVERY) ||
		(iom3_power_state == ST_SLEEP))
		ret = write_customize_cmd(&pkg_ap, NULL, false);
	else
		ret = write_customize_cmd(&pkg_ap, pkg_mcu, use_lock);
	if (ret < 0) {
		hwlog_err("%s: err. write cmd\n", __func__);
		return -1;
	}

	return 0;
}

static int send_thp_status_cmd(int inpara_len, const char *inpara)
{
	int ret;

	if (!(inpara) || (inpara_len <= 0)) {
		hwlog_err("%s para failed\n", __func__);
		return -1;
	}
	if (inpara[0] >= ST_CMD_TYPE_MAX) {
		hwlog_err("%s: cmd is invalid\n", __func__);
		return -1;
	}
	ret = send_thp_config_cmd(SUB_CMD_THP_STATUS_REQ, inpara_len,
			inpara, NULL, true);
	return ret;
}

static int send_thp_screen_status_cmd(unsigned char power_on,
	unsigned int gesture_support)
{
	int ret;
	char cmd_para[THP_STATUS_CMD_PARA_LEN] = {0};
	int cmd_len;

	g_thp_status.screen_state = power_on ? TP_SWITCH_ON : TP_SWITCH_OFF;
	if (g_thp_status.screen_state == TP_SWITCH_OFF)
		g_thp_status.support_mode = gesture_support;

	/* 0:cmd type offset, 1:power status offset, 2:gesture offset */
	cmd_para[0] = ST_CMD_TYPE_SET_SCREEN_STATUS;
	cmd_para[1] = g_thp_status.screen_state;
	/* 2~5: put 4 bytes gesture_support into cmd_para from index 2 */
	cmd_para[2] = (char)(gesture_support & 0x000000ff);
	cmd_para[3] = (char)((gesture_support >> 8) & 0x000000ff);
	cmd_para[4] = (char)((gesture_support >> 16) & 0x000000ff);
	cmd_para[5] = (char)((gesture_support >> 24) & 0x000000ff);
	cmd_len = THP_STATUS_CMD_PARA_LEN;
	hwlog_info("%s: called\n", __func__);
	ret = send_thp_status_cmd(cmd_len, cmd_para);
	return ret;
}

static int send_thp_tui_status_cmd(unsigned char status)
{
	int ret;
	char cmd_para[THP_STATUS_CMD_PARA_LEN] = {0};
	int cmd_len;

	g_thp_status.tui_mode = status;
	/* 0:cmd type offset, 1:tui mode offset */
	cmd_para[0] = ST_CMD_TYPE_SET_TUI_STATUS;
	cmd_para[1] = g_thp_status.tui_mode;
	cmd_len = THP_STATUS_CMD_PARA_LEN;
	hwlog_info("%s: called, status = %u\n", __func__, status);
	ret = send_thp_status_cmd(cmd_len, cmd_para);
	return ret;
}

static int send_thp_audio_status_cmd(unsigned char audio_on)
{
	int ret;
	char cmd_para[THP_STATUS_CMD_PARA_LEN] = {0};
	int cmd_len;

	if (audio_on == TP_SWITCH_ON) {
		g_thp_status.support_mode |= (1 << THP_RING_ON);
		g_thp_status.support_mode &= ~(1 << THP_PHONE_STATUS);
	} else if (audio_on == TP_ON_THE_PHONE) {
		g_thp_status.support_mode |= (1 << THP_PHONE_STATUS);
		g_thp_status.support_mode &= ~(1 << THP_RING_ON);
	} else {
		g_thp_status.support_mode &= ~(1 << THP_RING_ON);
		g_thp_status.support_mode &= ~(1 << THP_PHONE_STATUS);
	}

	/* 0:cmd type offset, 1:audio status offset */
	cmd_para[0] = ST_CMD_TYPE_SET_AUDIO_STATUS;
	cmd_para[1] = audio_on;
	cmd_len = THP_STATUS_CMD_PARA_LEN;
	hwlog_info("%s: called, status = %u\n", __func__, audio_on);
	ret = send_thp_status_cmd(cmd_len, cmd_para);
	return ret;
}

int send_thp_driver_status_cmd(unsigned char status,
	unsigned int para, unsigned char cmd)
{
	int ret;

	switch (cmd) {
	case ST_CMD_TYPE_SET_SCREEN_STATUS:
		ret = send_thp_screen_status_cmd(status, para);
		break;
	case ST_CMD_TYPE_SET_TUI_STATUS:
		ret = send_thp_tui_status_cmd(status);
		break;
	case ST_CMD_TYPE_SET_AUDIO_STATUS:
		ret = send_thp_audio_status_cmd(status);
		break;
	default:
		hwlog_err("thp: %s unknown cmd : %u\n", __func__, cmd);
		return -1;
	}
	return ret;
}

int send_thp_ap_event(int inpara_len, const char *inpara, uint8_t cmd)
{
	int ret;
	char cmd_para[THP_STATUS_CMD_PARA_LEN] = {0};
	int cmd_len;

	if (!(inpara) || (inpara_len <= 0)) {
		hwlog_err("%s para failed\n", __func__);
		return -1;
	}
	cmd_para[0] = (char)cmd;
	hwlog_info("thp: %s, cmd = %u, inpara_len = %d\n", __func__, cmd,
		inpara_len);
	if (inpara_len > (THP_STATUS_CMD_PARA_LEN - 1)) {
		hwlog_err("thp: %s, inpara_len is too large. size = %d\n",
			__func__, inpara_len);
		return -1;
	}
	memcpy((void *)&cmd_para[1], (void *)inpara, (unsigned int)inpara_len);
	cmd_len = inpara_len + 1;
	ret = send_thp_status_cmd(cmd_len, cmd_para);

	return ret;
}

static int send_thp_algo_sync_info_normal(int inpara_len, const char *inpara)
{
	char cmd_para[THP_ALGO_SYNC_BUFF_LEN] = {0};
	int cmd_len;

	cmd_para[0] = ST_CMD_TYPE_ALGO_SYNC_EVENT;
	memcpy((void *)&cmd_para[1], (void *)inpara, (unsigned int)inpara_len);
	cmd_len = inpara_len + 1;
	return send_thp_config_cmd(SUB_CMD_THP_STATUS_REQ, cmd_len,
		cmd_para, NULL, true);
}

int send_thp_algo_sync_event(int inpara_len, const char *inpara)
{
	int ret = 0;

	if (!inpara || (inpara_len <= 0)) {
		hwlog_err("%s para failed\n", __func__);
		return -1;
	}
	if (inpara_len <= MAX_PKT_LENGTH - SUBCMD_LEN -
			THP_CMD_TYPE_LEN - (int)sizeof(pkt_header_t)) {
		ret = send_thp_algo_sync_info_normal(inpara_len, inpara);
		if (ret) {
			hwlog_err("%s error\n", __func__);
			return ret;
		}
	} else {
		/* only for screen on/off, support more than 128 byte */
		ret = shmem_send(TAG_THP, inpara, (unsigned int)inpara_len);
		if (ret) {
			hwlog_err("%s shmem send sync info error\n", __func__);
			return ret;
		}
	}
	hwlog_info("%s send ok\n", __func__);
	return ret;
}

int send_thp_auxiliary_data(unsigned int inpara_len, const char *inpara)
{
	int ret;

	if (!inpara || (inpara_len == 0)) {
		hwlog_err("%s para failed\n", __func__);
		return -EINVAL;
	}
	ret = shmem_send(TAG_FP_UD, inpara, inpara_len);
	if (ret) {
		hwlog_err("%s shmem send auxiliary data error\n", __func__);
		return ret;
	}
	hwlog_info("%s send ok, datalen:%u\n", __func__, inpara_len);
	return ret;
}

static int execute_thp_config_cmd(int tag, obj_sub_cmd_t subcmd,
	unsigned long arg, bool use_lock)
{
	uintptr_t arg_tmp = (uintptr_t)arg;
	void __user *argp = (void __user *)arg_tmp;
	int ret;
	struct thp_ioctl_para para;
	struct read_info resp_para;
	char tmp_para[MAX_PKT_LENGTH];
	int datalen;

	memset((void *)tmp_para, 0, sizeof(tmp_para));
	memset((void *)&resp_para, 0, sizeof(resp_para));

	if (copy_from_user(&para, argp, sizeof(para))) {
		hwlog_err("thp: %s copy_from_user failed\n", __func__);
		return -EFAULT;
	}

	ret = send_thp_config_cmd(subcmd, sizeof(para.cmd_para),
			para.cmd_para, &resp_para, use_lock);

	if ((resp_para.data_length != 0) && (para.outbuf_len > 0) &&
		(para.outbuf)) {
		datalen = (para.outbuf_len < resp_para.data_length) ?
				para.outbuf_len : resp_para.data_length;
		memcpy(tmp_para, resp_para.data, (unsigned int)datalen);
		if (copy_to_user(para.outbuf, tmp_para,
			(unsigned int)datalen)) {
			hwlog_err("thp: %s copy_to_user failed\n", __func__);
			return -EFAULT;
		}
	}

	return ret;
}

static int send_thp_cmd(unsigned int cmd, unsigned long arg)
{
	int i;
	int ret;
	int len = sizeof(thp_cmd_map_tab) / sizeof(thp_cmd_map_tab[0]);

	if (flag_for_sensor_test) {
		hwlog_info("thp: %s flag_for_sensor_test is enabled, cmd = %u\n",
			__func__, cmd);
		return 0;
	}
	for (i = 0 ; i < len; i++) {
		if (thp_cmd_map_tab[i].thp_ioctl_app_cmd == cmd)
			break;
	}
	if (i == len) {
		hwlog_err("thp: %s unknown cmd %u\n", __func__, cmd);
		return -EFAULT;
	}

	switch (thp_cmd_map_tab[i].cmd) {
	case SUB_CMD_THP_DEBUG_REQ:
		ret = execute_thp_config_cmd(thp_cmd_map_tab[i].tag,
			thp_cmd_map_tab[i].cmd, arg, true);
		break;
	case SUB_CMD_THP_CONFIG_REQ:
	case SUB_CMD_THP_STATUS_REQ:
		ret = execute_thp_config_cmd(thp_cmd_map_tab[i].tag,
			thp_cmd_map_tab[i].cmd, arg, true);
		break;
	case SUB_CMD_THP_OPEN_REQ:
	case SUB_CMD_THP_CLOSE_REQ:
		ret = send_thp_switch_cmd(thp_cmd_map_tab[i].tag,
			thp_cmd_map_tab[i].cmd, true);
		break;
	default:
		hwlog_err("thp: %s unknown cmd : %u\n", __func__, cmd);
		return -ENOTTY;
	}

	return ret;
}

static void enable_thp_when_recovery_iom3(void)
{
	int ret;
	char cmd_para[THP_STATUS_CMD_PARA_LEN] = {0};

	/* 0:cmd type offset, 1:screen status offset, 2:tui mode offset */
	cmd_para[0] = ST_CMD_TYPE_SET_RECOVER_STATUS;
	cmd_para[1] = g_thp_status.screen_state;
	cmd_para[2] = g_thp_status.tui_mode;
	/* 3~6: put 4 bytes gesture_support into cmd_para from index 3 */
	memcpy((void *)&cmd_para[3], (void *)&g_thp_status.support_mode,
		sizeof(g_thp_status.support_mode));
	hwlog_info("%s: screen_state = %u, tui_mode = %u\n", __func__,
		g_thp_status.screen_state, g_thp_status.tui_mode);
	ret = send_thp_status_cmd(THP_STATUS_CMD_PARA_LEN, cmd_para);
	if (ret)
		hwlog_err("%s: send status failed\n", __func__);
}

void disable_thp_when_sysreboot(void)
{
	hwlog_info("thp: thp state: %s\n", __func__);
}

/* read /dev/thphub, do nothing now */
static ssize_t thp_read(struct file *file, char __user *buf, size_t count,
	loff_t *pos)
{
	hwlog_info("thp: enter %s\n", __func__);
	return 0;
}

/* write to /dev/thphub, do nothing now */
static ssize_t thp_write(struct file *file, const char __user *data,
	size_t len, loff_t *ppos)
{
	hwlog_info("thp: enter %s\n", __func__);
	return 0;
}

/*
 * Description: ioctl function to /dev/thphub, do open, do config/debug
 *              cmd to thphub
 * Return:      result of ioctrl command, 0 successed, -ENOTTY failed
 */
static long thp_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	switch (cmd) {
	case THP_IOCTL_THP_DEBUG_CMD:
		hwlog_info("thp: %s  cmd: THP_IOCTL_THP_DEBUG_CMD\n", __func__);
		break;
	case THP_IOCTL_THP_CONFIG_CMD:
		hwlog_info("thp: %s cmd: THP_IOCTL_THP_CONFIG_CMD\n", __func__);
		break;
	case THP_IOCTL_THP_OPEN_CMD:
		hwlog_info("thp: %s  cmd: THP_IOCTL_THP_OPEN_CMD\n", __func__);
		break;
	case THP_IOCTL_THP_CLOSE_CMD:
		hwlog_info("thp: %s cmd: THP_IOCTL_THP_CLOSE_CMD\n", __func__);
		break;
	case THP_IOCTL_THP_STATUS_CMD:
		hwlog_info("thp: %s cmd: THP_IOCTL_THP_STATUS_CMD\n", __func__);
		break;
	default:
		hwlog_err("thp: %s unknown cmd : %u\n", __func__, cmd);
		return -ENOTTY;
	}

	return send_thp_cmd(cmd, arg);
}

/* open to /dev/thphub, do nothing now */
static int thp_open(struct inode *inode, struct file *file)
{
	hwlog_info("thp: enter %s\n", __func__);
	return 0;
}

/* release to /dev/thphub, do nothing now */
static int thp_release(struct inode *inode, struct file *file)
{
	hwlog_info("thp: %s ok\n", __func__);
	return 0;
}

static int thp_recovery_notifier(struct notifier_block *nb,
			unsigned long foo, void *bar)
{
	/* prevent access the emmc now: */
	hwlog_info("%s: %lu\n", __func__, foo);
	switch (foo) {
	case IOM3_RECOVERY_IDLE:
		if (g_thp_open_flag)
			enable_thp_when_recovery_iom3();
		break;
	case IOM3_RECOVERY_START:
	case IOM3_RECOVERY_MINISYS:
	case IOM3_RECOVERY_3RD_DOING:
	case IOM3_RECOVERY_FAILED:
		break;
	case IOM3_RECOVERY_DOING:
		break;
	default:
		hwlog_err("%s: unknown state %lu\n", __func__, foo);
		break;
	}
	hwlog_info("%s -\n", __func__);
	return 0;
}

static struct notifier_block thp_recovery_notify = {
	.notifier_call = thp_recovery_notifier,
	.priority = -1,
};

/* file_operations to thphub */
static const struct file_operations thp_fops = {
	.owner = THIS_MODULE,
	.read = thp_read,
	.write = thp_write,
	.unlocked_ioctl = thp_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = thp_ioctl,
#endif
	.open = thp_open,
	.release = thp_release,
};

/* miscdevice to thphub */
static struct miscdevice thphub_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "thphub",
	.fops = &thp_fops,
};

/*
 * Description: apply kernel buffer, register thphub_miscdev
 * Return:     result of function, 0 successed, else false
 */
static int __init thphub_init(void)
{
	int ret;

	if (is_sensorhub_disabled())
		return -1;

	ret = misc_register(&thphub_miscdev);
	if (ret) {
		hwlog_err("%s cannot register miscdev err=%d\n", __func__, ret);
		inputhub_route_close(ROUTE_FHB_PORT);
		return ret;
	}
	register_iom3_recovery_notifier(&thp_recovery_notify);
	g_thp_status.screen_state = TP_SWITCH_ON;
	g_thp_status.tui_mode = TP_SWITCH_OFF;
	hwlog_info("%s ok\n", __func__);
	return ret;
}

/* release kernel buffer, deregister thphub_miscdev */
static void __exit thphub_exit(void)
{
	misc_deregister(&thphub_miscdev);
	hwlog_info("exit %s\n", __func__);
}

late_initcall_sync(thphub_init);
module_exit(thphub_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("THPHub driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
