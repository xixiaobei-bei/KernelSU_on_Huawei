/*
 * iaware_rtg.c
 *
 * rtg ioctl entry
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#include "include/iaware_rtg.h"
#include "include/proc_state.h"
#include "include/trans_rtg.h"
#include <linux/atomic.h>

#define SYSTEM_SERVER_UID 1000
#define MIN_APP_UID 10000

atomic_t g_rtg_enable = ATOMIC_INIT(0);

static int set_enable_config(char *config_str)
{
	char *p;
	char *tmp;
	int value;
	bool minus;
	int config[RTG_CONFIG_NUM];
	int i;

	for (i = 0; i < RTG_CONFIG_NUM; i++)
		config[i] = INVALID_VALUE;

	// eg: key1:value1;key2:value2;key3:value3
	for (p = strsep(&config_str, ";"); p != NULL;
		p = strsep(&config_str, ";")) {
		tmp = strsep(&p, ":");
		if (tmp == NULL || p == NULL)
			continue;
		minus = false;
		if (*p == '-') {
			minus = true;
			if (strlen(p) <= 1)
				continue;
			p++;
		}
		if (kstrtouint((const char *)p, DECIMAL, &value))
			return -INVALID_ARG;
		if (minus)
			value = 0 - value;
		if (!strcmp(tmp, "load_freq_switch")) {
			config[RTG_LOAD_FREQ] = value;
		} else if (!strcmp(tmp, "sched_cycle")) {
			config[RTG_FREQ_CYCLE] = value;
		} else if (!strcmp(tmp, "transfer_level")) {
			config[RTG_TRANS_DEPTH] = value;
		} else if (!strcmp(tmp, "max_threads")) {
			config[RTG_MAX_THREADS] = value;
		} else if (!strcmp(tmp, "frame_max_util")) {
			config[RTG_FRAME_MAX_UTIL] = value;
		} else if (!strcmp(tmp, "act_max_util")) {
			config[RTG_ACT_MAX_UTIL] = value;
		} else if (!strcmp(tmp, "invalid_interval")) {
			config[RTG_INVALID_INTERVAL] = value;
		} else {
			pr_err("[AWARE_RTG] parse enable config failed!\n");
			return -INVALID_ARG;
		}
	}
	for (i = 0; i < RTG_CONFIG_NUM; i++)
		pr_info("[AWARE_RTG] config[%d] = %d\n", i, config[i]);

	set_trans_config(config[RTG_TRANS_DEPTH], config[RTG_MAX_THREADS]);
	return init_proc_state(config);
}

/*lint -save -e446 -e666 -e732 -e734*/
static void enable(const struct rtg_enable_data *data)
{
	char temp[MAX_DATA_LEN];

	if (atomic_read(&g_rtg_enable) == 1) {
		pr_info("[AWARE_RTG] already enabled!\n");
		return;
	}
	if (data->len <= 0 || data->len >= MAX_DATA_LEN) {
		pr_err("[AWARE_RTG] %s data len invalid\n", __func__);
		return;
	}
	if (copy_from_user(&temp, (void __user *)data->data, data->len)) {
		pr_err("[AWARE_RTG] %s copy user data failed\n", __func__);
		return;
	}
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
	temp[data->len] = '\0';

	if (set_enable_config(&temp) != SUCC) {
		pr_err("[AWARE_RTG] %s failed!\n", __func__);
		return;
	}
#pragma GCC diagnostic pop
	atomic_set(&g_rtg_enable, 1);
	pr_info("[AWARE_RTG] enabled!\n");
}

static void disable(void)
{
	if (atomic_read(&g_rtg_enable) == 0) {
		pr_info("[AWARE_RTG] already disabled!\n");
		return;
	}
	pr_info("[AWARE_RTG] disabled!\n");
	atomic_set(&g_rtg_enable, 0);
	deinit_proc_state();
}

static long ctrl_set_enable(unsigned long arg)
{
	void __user *uarg = (void __user *)arg;
	struct rtg_enable_data rs_enable;

	if (uarg == NULL)
		return -INVALID_ARG;

	if (current_uid().val != SYSTEM_SERVER_UID)
		return -NOT_SYSTEM_UID;

	if (copy_from_user(&rs_enable, uarg, sizeof(rs_enable))) {
		pr_err("[AWARE_RTG] CMD_ID_SET_ENABLE copy data failed\n");
		return -INVALID_ARG;
	}
	if (rs_enable.enable == 1)
		enable(&rs_enable);
	else
		disable();

	return SUCC;
}

static long ctrl_set_config(unsigned long arg)
{
	void __user *uarg = (void __user *)arg;
	struct rtg_str_data rs;
	char temp[MAX_DATA_LEN];

	if (uarg == NULL)
		return -INVALID_ARG;

	if (current_uid().val != SYSTEM_SERVER_UID)
		return -NOT_SYSTEM_UID;

	if (copy_from_user(&rs, uarg, sizeof(rs))) {
		pr_err("[AWARE_RTG] CMD_ID_SET_CONFIG copy data failed\n");
		return -INVALID_ARG;
	}
	if (rs.len <= 0 || rs.len >= MAX_DATA_LEN) {
		pr_err("[AWARE_RTG] CMD_ID_SET_CONFIG data len invalid\n");
		return -INVALID_ARG;
	}

	if (copy_from_user(&temp, (void __user *)rs.data, rs.len)) {
		pr_err("[AWARE_RTG] CMD_ID_SET_CONFIG failed copy data failed\n");
		return -INVALID_ARG;
	}
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
	temp[rs.len] = '\0';
	rs.data = &temp;
	return parse_config(&rs);
#pragma GCC diagnostic pop
}

static long ctrl_set_rtg_thread(unsigned long arg)
{
	void __user *uarg = (void __user *)arg;
	struct rtg_str_data rs;
	char temp[MAX_DATA_LEN];

	if (uarg == NULL)
		return -INVALID_ARG;

	if (current_uid().val != SYSTEM_SERVER_UID)
		return -NOT_SYSTEM_UID;

	if (copy_from_user(&rs, uarg, sizeof(rs))) {
		pr_err("[AWARE_RTG] CMD_ID_SET_RTG_THREAD  copy data failed\n");
		return -INVALID_ARG;
	}
	if (rs.len <= 0 || rs.len >= MAX_DATA_LEN) {
		pr_err("[AWARE_RTG] CMD_ID_SET_RTG_THREAD data len invalid\n");
		return -INVALID_ARG;
	}
	if (copy_from_user(&temp, (void __user *)rs.data, rs.len)) {
		pr_err("[AWARE_RTG] CMD_ID_SET_RTG_THREAD send data failed\n");
		return -INVALID_ARG;
	}
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
	temp[rs.len] = '\0';
	rs.data = &temp;
	return parse_rtg_thread(&rs);
#pragma GCC diagnostic pop
}

static long ctrl_get_qos(unsigned long arg)
{
	void __user *uarg = (void __user *)arg;
	struct rtg_qos_data qos_data;

	if (uarg == NULL)
		return -INVALID_ARG;

	if (copy_from_user(&qos_data, uarg, sizeof(qos_data))) {
		pr_err("[AWARE_RTG] CMD_ID_GET_QOS_CLASS copy data failed\n");
		return -INVALID_ARG;
	}
	qos_data.is_rtg = is_cur_frame();
	if (copy_to_user(uarg, &qos_data, sizeof(qos_data))) {
		pr_err("[AWARE_RTG] CMD_ID_GET_QOS_CLASS send data failed\n");
		return -INVALID_ARG;
	}
	return SUCC;
}

static long ctrl_activity_state(unsigned long arg, bool is_enter)
{
	void __user *uarg = (void __user *)arg;
	struct proc_state_data state_data;

	if (uarg == NULL)
		return -INVALID_ARG;

	if (copy_from_user(&state_data, uarg, sizeof(state_data))) {
		pr_err("[AWARE_RTG] CMD_ID_ACTIVITY_FREQ copy data failed\n");
		return -INVALID_ARG;
	}
	return update_act_state(&(state_data.head), is_enter);
}

static long ctrl_frame_state(unsigned long arg, bool is_enter)
{
	void __user *uarg = (void __user *)arg;
	struct proc_state_data state_data;

	if (uarg == NULL)
		return -INVALID_ARG;

	if (copy_from_user(&state_data, uarg, sizeof(state_data))) {
		pr_err("[AWARE_RTG] CMD_ID_FRAME_FREQ copy data failed\n");
		return -INVALID_ARG;
	}
	return update_frame_state(&(state_data.head),
				state_data.frame_freq_type, is_enter);
}

int proc_rtg_open(struct inode *inode, struct file *filp)
{
	if (current_uid().val != SYSTEM_SERVER_UID &&
		current_uid().val < MIN_APP_UID)
		return -OPEN_ERR_UID;
	if (current_uid().val >= MIN_APP_UID && current->pid != current->tgid)
		return -OPEN_ERR_TID;
	return SUCC;
}

#ifdef CONFIG_COMPAT
long proc_rtg_compat_ioctl(struct file *file,
	unsigned int cmd, unsigned long arg)
{
	/*lint -e712*/
	return proc_rtg_ioctl(file, cmd, (unsigned long)(compat_ptr(arg)));
	/*lint +e712*/
}
#endif

long proc_rtg_ioctl(struct file *file,  unsigned int cmd, unsigned long arg)
{
	long ret = SUCC;

	if (_IOC_TYPE(cmd) != RTG_SCHED_IPC_MAGIC)
		return -INVALID_MAGIC;

	if (cmd != CMD_ID_SET_ENABLE && !atomic_read(&g_rtg_enable))
		return -RTG_DISABLED;

	if (_IOC_NR(cmd) >= CMD_ID_MAX)
		return -INVALID_CMD;

	switch (cmd) {
	case CMD_ID_BEGIN_FRAME_FREQ: {
		ret = ctrl_frame_state(arg, true);
		break;
	}
	case CMD_ID_END_FRAME_FREQ: {
		ret = ctrl_frame_state(arg, false);
		break;
	}
	case CMD_ID_BEGIN_ACTIVITY_FREQ: {
		ret = ctrl_activity_state(arg, true);
		break;
	}
	case CMD_ID_END_ACTIVITY_FREQ: {
		ret = ctrl_activity_state(arg, false);
		break;
	}
	case CMD_ID_SET_RTG_THREAD: {
		ret = ctrl_set_rtg_thread(arg);
		break;
	}
	case CMD_ID_GET_QOS_CLASS: {
		ret = ctrl_get_qos(arg);
		break;
	}
	case CMD_ID_SET_ENABLE: {
		ret = ctrl_set_enable(arg);
		break;
	}
	case CMD_ID_SET_CONFIG: {
		ret = ctrl_set_config(arg);
		break;
	}
	default:
		ret = -INVALID_ARG;
		break;
	}
	return ret;
}
/*lint -restore*/
