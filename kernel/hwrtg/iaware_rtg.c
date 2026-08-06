/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
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
 * rtg ioctl entry
 */

#include "include/iaware_rtg.h"

#include <linux/atomic.h>
#include <linux/cred.h>
#include <linux/sched/topology.h>
#ifdef CONFIG_HW_RTG_SCHED
#include <linux/perf_ctrl.h>
#include <linux/sched/frame.h>
#endif
#ifdef CONFIG_HW_MTK_RTG_SCHED
#include <mtkrtg/frame.h>
#endif

#include "include/frame_timer.h"
#include "include/proc_state.h"
#include "include/set_rtg.h"
#include "include/trans_rtg.h"

#define SYSTEM_SERVER_UID 1000
#define MIN_APP_UID 10000
#define MAX_BOOST_DURATION_MS 5000

atomic_t g_rtg_enable = ATOMIC_INIT(0);
#ifdef CONFIG_HW_MTK_RTG_SCHED
atomic_t g_enable_type = ATOMIC_INIT(ALL_ENABLE); // default: all enable
#endif

atomic_t g_fps_state = ATOMIC_INIT(INIT_STATE);
static struct rme_fps_data g_fps_data[MAX_BUF_SIZE] = {
	{
		.deadline_margin = 4,
		.input_margin = 18,
		.animation_margin = 19,
		.traversal_margin = 20,
		.input_period = 3,
		.animation_period = 4,
		.traversal_period = 4,
		.default_util = 600,
		.frame_rate = 60,
		.min_prev_util = FRAME_DEFAULT_MIN_PREV_UTIL,
		.max_prev_util = FRAME_DEFAULT_MAX_PREV_UTIL,
	},
	{
		.deadline_margin = 0,
		.input_margin = 0,
		.animation_margin = 0,
		.traversal_margin = 0,
		.input_period = 0,
		.animation_period = 0,
		.traversal_period = 0,
		.default_util = 0,
		.frame_rate = 0,
	}// conservastive backup
};

static int set_enable_config(char *config_str)
{
	char *p = NULL;
	char *tmp = NULL;
	int value;
	int config[RTG_CONFIG_NUM];
	int i;

	for (i = 0; i < RTG_CONFIG_NUM; i++)
		config[i] = INVALID_VALUE;
	/* eg: key1:value1;key2:value2;key3:value3 */
	for (p = strsep(&config_str, ";"); p != NULL;
		p = strsep(&config_str, ";")) {
		tmp = strsep(&p, ":");
		if ((tmp == NULL) || (p == NULL))
			continue;
		if (kstrtoint((const char *)p, DECIMAL, &value))
			return -INVALID_ARG;

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
#ifdef CONFIG_HW_MTK_RTG_SCHED
		} else if (!strcmp(tmp, "enable_type")) {
			atomic_set(&g_enable_type, value);
#endif
		} else {
			pr_err("[AWARE_RTG] parse enable config failed!\n");
			return -INVALID_ARG;
		}
	}
	for (i = 0; i < RTG_CONFIG_NUM; i++)
		pr_info("[AWARE_RTG] config[%d] = %d\n", i, config[i]);

	set_trans_config(config[RTG_TRANS_DEPTH], config[RTG_MAX_THREADS]);
	return init_proc_state(config, RTG_CONFIG_NUM);
}

/*lint -save -e446 -e666 -e732 -e734 -e507 -e545*/
static void enable(const struct rtg_enable_data *data)
{
	char temp[MAX_DATA_LEN];

	if (atomic_read(&g_rtg_enable) == 1) {
		pr_info("[AWARE_RTG] already enabled!\n");
		return;
	}
	if ((data->len <= 0) || (data->len >= MAX_DATA_LEN)) {
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

	init_frame_timer();
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
	deinit_frame_timer();
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

static long ctrl_set_config(int abi, unsigned long arg)
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
	if ((rs.len <= 0) || (rs.len >= MAX_DATA_LEN)) {
		pr_err("[AWARE_RTG] CMD_ID_SET_CONFIG data len invalid\n");
		return -INVALID_ARG;
	}

	switch (abi) {
	case IOCTL_ABI_ARM32:
		copy_from_user(&temp,
			(void __user *)compat_ptr((compat_uptr_t)rs.data), rs.len);
		break;
	case IOCTL_ABI_AARCH64:
		copy_from_user(&temp, (void __user *)rs.data, rs.len);
		break;
	default:
		pr_err("[AWARE_RTG] CMD_ID_SET_CONFIG abi format error");
		return -INVALID_ARG;
	}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
	temp[rs.len] = '\0';
	rs.data = &temp;
	return parse_config(&rs);
#pragma GCC diagnostic pop
}

static long ctrl_set_rtg_thread(int abi, unsigned long arg)
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
	if ((rs.len <= 0) || (rs.len >= MAX_DATA_LEN)) {
		pr_err("[AWARE_RTG] CMD_ID_SET_RTG_THREAD data len invalid\n");
		return -INVALID_ARG;
	}

	switch (abi) {
	case IOCTL_ABI_ARM32:
		copy_from_user(&temp,
			(void __user *)compat_ptr((compat_uptr_t)rs.data), rs.len);
		break;
	case IOCTL_ABI_AARCH64:
		copy_from_user(&temp, (void __user *)rs.data, rs.len);
		break;
	default:
		pr_err("[AWARE_RTG] CMD_ID_SET_RTG_THREAD abi format error");
		return -INVALID_ARG;
	}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
	temp[rs.len] = '\0';
	rs.data = &temp;
	if (strstr(temp, "aux"))
		return parse_aux_thread(&rs);
	else if (strstr(temp, "key"))
		return parse_aux_comm_config(&rs);
	else if (strstr(temp, "boost"))
		return parse_boost_thread(&rs);
	else
		return parse_frame_thread(&rs);
#pragma GCC diagnostic pop
}

static long ctrl_set_rtg_cfs_thread(int abi, unsigned long arg)
{
	void __user *uarg = (void __user *)arg;
	struct rtg_str_data rs;
	char temp[MAX_DATA_LEN];

	if (uarg == NULL)
		return -INVALID_ARG;

	if (copy_from_user(&rs, uarg, sizeof(rs))) {
		pr_err("[AWARE_RTG] CMD_ID_SET_RTG_CFS_THREAD  copy data failed\n");
		return -INVALID_ARG;
	}
	if ((rs.len <= 0) || (rs.len >= MAX_DATA_LEN)) {
		pr_err("[AWARE_RTG] CMD_ID_SET_RTG_CFS_THREAD data len invalid\n");
		return -INVALID_ARG;
	}

	switch (abi) {
	case IOCTL_ABI_ARM32:
		copy_from_user(&temp,
			(void __user *)compat_ptr((compat_uptr_t)rs.data), rs.len);
		break;
	case IOCTL_ABI_AARCH64:
		copy_from_user(&temp, (void __user *)rs.data, rs.len);
		break;
	default:
		pr_err("[AWARE_RTG] CMD_ID_SET_RTG_CFS_THREAD abi format error");
		return -INVALID_ARG;
	}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
	temp[rs.len] = '\0';
	rs.data = &temp;
	return parse_frame_cfs_thread(&rs);
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

static int ctrl_rme_state(int freq_type)
{
	/* if rme set frame_freq_type, translated margin to freq_type */
	switch (freq_type) {
	case FRAME_INPUT: {
		freq_type = g_fps_data[PING].input_margin;
		break;
	}
	case FRAME_ANIMATION: {
		freq_type = g_fps_data[PING].animation_margin;
		break;
	}
	case FRAME_TRAVERSAL: {
		freq_type = g_fps_data[PING].traversal_margin;
		break;
	}
	default:
		break;
	}

	return freq_type;
}

static long ctrl_frame_state(unsigned long arg, bool is_enter)
{
	void __user *uarg = (void __user *)arg;
	struct proc_state_data state_data;
	int freq_type;

	if (uarg == NULL)
		return -INVALID_ARG;

	if (copy_from_user(&state_data, uarg, sizeof(state_data))) {
		pr_err("[AWARE_RTG] CMD_ID_FRAME_FREQ copy data failed\n");
		return -INVALID_ARG;
	}
	freq_type = ctrl_rme_state(state_data.frame_freq_type);

	return update_frame_state(&(state_data.head), freq_type, is_enter);
}

static long ctrl_stop_frame_freq(unsigned long arg)
{
	void __user *uarg = (void __user *)arg;
	struct rtg_data_head rd;

	if (uarg == NULL)
		return -INVALID_ARG;

	if (copy_from_user(&rd, uarg, sizeof(rd))) {
		pr_err("[AWARE_RTG] CMD_ID_END_FREQ copy data failed\n");
		return -INVALID_ARG;
	}

	return stop_frame_freq(&rd);
}

static long ctrl_set_rtg_attr(unsigned long arg)
{
	void __user *uarg = (void __user *)arg;
	struct rtg_str_data rs;
	char temp[MAX_DATA_LEN];

	if (uarg == NULL)
		return -INVALID_ARG;

	if (current_uid().val != SYSTEM_SERVER_UID)
		return -NOT_SYSTEM_UID;

	if (copy_from_user(&rs, uarg, sizeof(rs))) {
		pr_err("[AWARE_RTG] CMD_ID_SET_RTG_ATTR copy data failed\n");
		return -INVALID_ARG;
	}
	if ((rs.len <= 0) || (rs.len >= MAX_DATA_LEN)) {
		pr_err("[AWARE_RTG] CMD_ID_SET_RTG_ATTR data len invalid\n");
		return -INVALID_ARG;
	}

	if (copy_from_user(&temp, (void __user *)rs.data, rs.len)) {
		pr_err("[AWARE_RTG] CMD_ID_SET_RTG_ATTR copy rs.data failed\n");
		return -INVALID_ARG;
	}
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
	temp[rs.len] = '\0';
	rs.data = &temp;
	return parse_rtg_attr(&rs);
#pragma GCC diagnostic pop
}

static long ctrl_rtg_boost(unsigned long arg)
{
	void __user *uarg = (void __user *)arg;
	struct rtg_boost_data boost_data;
	int duration;
	int min_util;

	if (uarg == NULL)
		return -INVALID_ARG;

	if (copy_from_user(&boost_data, uarg, sizeof(boost_data))) {
		pr_err("[AWARE_RTG] CMD_ID_END_FREQ copy data failed\n");
		return -INVALID_ARG;
	}

	duration = boost_data.duration;
	min_util = boost_data.min_util;
	if ((duration <= 0) || (duration > MAX_BOOST_DURATION_MS) ||
		(min_util <= 0) || (min_util > DEFAULT_MAX_UTIL))
		return -ERR_RTG_BOOST_ARG;

	start_rtg_boost();
	start_boost_timer(duration, min_util);
	return 0;
}

static long ctrl_set_min_util(unsigned long arg)
{
	void __user *uarg = (void __user *)arg;
	struct proc_state_data state_data;

	if (uarg == NULL)
		return -INVALID_ARG;

	if (copy_from_user(&state_data, uarg, sizeof(state_data))) {
		pr_err("[AWARE_RTG] CMD_ID_SET_MIN_UTIL copy data failed\n");
		return -INVALID_ARG;
	}
	return set_min_util(&(state_data.head), state_data.frame_freq_type);
}

static long ctrl_set_margin(unsigned long arg)
{
	void __user *uarg = (void __user *)arg;
	struct proc_state_data state_data;
	int margin;

	if (uarg == NULL)
		return -INVALID_ARG;

	if (copy_from_user(&state_data, uarg, sizeof(state_data))) {
		pr_err("[AWARE_RTG] CMD_ID_SET_MARGIN copy data failed\n");
		return -INVALID_ARG;
	}
	margin = ctrl_rme_state(state_data.frame_freq_type);
	return set_margin(&(state_data.head), margin);
}

static long ctrl_set_min_util_and_margin(unsigned long arg)
{
	void __user *uarg = (void __user *)arg;
	struct min_util_margin_data state_data;
	int util = 0;

	if (uarg == NULL)
		return -INVALID_ARG;

	if (copy_from_user(&state_data, uarg, sizeof(state_data))) {
		pr_err("[RME][RMEKERNEL] CMD_ID_SET_MIN_UTIL_AND_MARGIN copy data failed\n");
		return -INVALID_ARG;
	}

	if (state_data.min_util == 1) /* 1 means use util */
		util = g_fps_data[PING].default_util;
	return set_min_util_and_margin(&(state_data.head),
		util, (state_data.margin + g_fps_data[PING].deadline_margin));
}

static long ctrl_set_rme_margin(unsigned long arg)
{
	void __user *uarg = (void __user *)arg;
	struct rme_fps_data fps_data;
	const int min_margin = -16;

	if (current_uid().val != SYSTEM_SERVER_UID) {
		pr_err("[RME][RMEKERNEL] Invalid uid\n");
		return -INVALID_ARG;
	}
	if (uarg == NULL)
		return -INVALID_ARG;
	if (copy_from_user(&fps_data, uarg, sizeof(fps_data))) {
		pr_err("[RME][RMEKERNEL] CMD_ID_SET_RME_MARGIN copy data failed\n");
		return -INVALID_ARG;
	}

	if ((fps_data.deadline_margin < min_margin) ||
		(fps_data.input_margin < min_margin) ||
		(fps_data.animation_margin < min_margin) ||
		(fps_data.traversal_margin < min_margin) ||
		(fps_data.input_period < 0) || /* peroid above 0 */
		(fps_data.animation_period < 0) || /* peroid above 0 */
		(fps_data.traversal_period < 0) || /* peroid above 0 */
		(fps_data.default_util >= 1024)) { /* util below 1024 */
		pr_err("[RME][RMEKERNEL] CMD_ID_SET_RME_MARGIN Para Invalid\n");
		return -INVALID_ARG;
	}

	if (atomic_read(&g_fps_state) != INIT_STATE)
		return SUCC;
	atomic_set(&g_fps_state, WRITING);
	if (atomic_read(&g_fps_state) != WRITING)
		return SUCC;

	if (g_fps_data[PONG].frame_rate == fps_data.frame_rate) {
		atomic_set(&g_fps_state, INIT_STATE);
		return SUCC;
	}
	g_fps_data[PONG] = fps_data;
	atomic_set(&g_fps_state, INIT_STATE);
	return SUCC;
}

static long ctrl_get_rme_margin(unsigned long arg)
{
	void __user *uarg = (void __user *)arg;
	struct rme_fps_data margin_data;

	if (uarg == NULL)
		return -INVALID_ARG;
	if (copy_from_user(&margin_data, uarg, sizeof(margin_data))) {
		pr_err("[RME][RMEKERNEL] CMD_ID_GET_RME_MARGIN copy data failed\n");
		return -INVALID_ARG;
	}

	if (atomic_read(&g_fps_state) != INIT_STATE)
		return SUCC;
	atomic_set(&g_fps_state, READING);
	if (atomic_read(&g_fps_state) != READING)
		return SUCC;

	if (g_fps_data[PONG].frame_rate != 0)
		g_fps_data[PING] = g_fps_data[PONG];

	if (margin_data.frame_rate != g_fps_data[PING].frame_rate)
		margin_data = g_fps_data[PING];
	else /* notify do not need to update para */
		margin_data.frame_rate = 0;

	atomic_set(&g_fps_state, INIT_STATE);
	if (copy_to_user(uarg, &margin_data, sizeof(margin_data))) {
		pr_err("[RME][RMEKERNEL] CMD_ID_GET_RME_MARGIN send data failed\n");
		return -INVALID_ARG;
	}
	return SUCC;
}

#ifdef CONFIG_HW_RTG_SCHED
static unsigned long get_dev_cap(void)
{
	unsigned long cap = 0;

	cap |= BIT(CAP_AI_SCHED_COMM_CMD);
#ifdef CONFIG_HISI_RTG
	cap |= BIT(CAP_RTG_CMD);
#endif
#ifdef CONFIG_RENDER_RT
	cap |= BIT(CAP_RENDER_RT_CMD);
#endif

	return cap;
}

static int ctrl_get_dev_cap(void __user *uarg)
{
	unsigned long cap = get_dev_cap();

	if (uarg == NULL)
		return -EINVAL;

	if (copy_to_user(uarg, &cap, sizeof(unsigned long))) {
		pr_err("%s: copy_to_user fail\n", __func__);
		return -EFAULT;
	}

	return 0;
}
#endif

static long rtg_config_ioctl(int abi, unsigned int cmd, unsigned long arg)
{
	long ret;
#ifdef CONFIG_HW_RTG_SCHED
	void __user *uarg = (void __user *)arg;
#endif

	switch (cmd) {
	case CMD_ID_GET_QOS_CLASS: {
		ret = ctrl_get_qos(arg);
		break;
	}
	case CMD_ID_ENABLE_RTG_BOOST: {
		ret = ctrl_rtg_boost(arg);
		break;
	}
	case CMD_ID_SET_CONFIG: {
		ret = ctrl_set_config(abi, arg);
		break;
	}
	case CMD_ID_SET_MIN_UTIL: {
		ret = ctrl_set_min_util(arg);
		break;
	}
	case CMD_ID_SET_MARGIN: {
		ret = ctrl_set_margin(arg);
		break;
	}
	case CMD_ID_SET_MIM_UTIL_AND_MARGIN: {
		ret = ctrl_set_min_util_and_margin(arg);
		break;
	}
	case CMD_ID_SET_RME_MARGIN: {
		ret = ctrl_set_rme_margin(arg);
		break;
	}
	case CMD_ID_SET_ENABLE: {
		ret = ctrl_set_enable(arg);
		break;
	}
	case CMD_ID_GET_RME_MARGIN: {
		ret = ctrl_get_rme_margin(arg);
		break;
	}
#ifdef CONFIG_HW_RTG_SCHED
	case RTG_CMD_RT_CTRL_INIT_RENDER_PID: {
		ret = init_render_rthread(uarg);
		break;
	}
	case RTG_CMD_RT_CTRL_GET_RENDER_RT: {
		ret = get_render_rthread(uarg);
		break;
	}
	case RTG_CMD_RT_CTRL_STOP_RENDER_RT: {
		ret = stop_render_rthread(uarg);
		break;
	}
	case RTG_CMD_RT_CTRL_GET_RENDER_HT: {
		ret = get_render_hrthread(uarg);
		break;
	}
	case RTG_CMD_RT_CTRL_DESTROY_RENDER_RT: {
		ret = destroy_render_rthread(uarg);
		break;
	}
	case RTG_CMD_RT_CTRL_GET_DEV_CAP: {
		ret = ctrl_get_dev_cap(uarg);
		break;
	}
#endif
	default:
		ret = -INVALID_CMD;
		break;
	}
	return ret;
}

static long do_proc_rtg_ioctl(int abi, struct file *file, unsigned int cmd, unsigned long arg)
{
	long ret;

	if (_IOC_TYPE(cmd) != RTG_SCHED_IPC_MAGIC)
		return -INVALID_MAGIC;

	if ((cmd != CMD_ID_SET_ENABLE) && !atomic_read(&g_rtg_enable))
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
		ret = ctrl_set_rtg_thread(abi, arg);
		break;
	}
	case CMD_ID_END_FREQ: {
		ret = ctrl_stop_frame_freq(arg);
		break;
	}
	case CMD_ID_SET_RTG_CFS_THREAD: {
		ret = ctrl_set_rtg_cfs_thread(abi, arg);
		break;
	}
	case CMD_ID_SET_RTG_ATTR: {
		ret = ctrl_set_rtg_attr(arg);
		break;
	}
	default:
		ret = rtg_config_ioctl(abi, cmd, arg);
		break;
	}
	return ret;
}


bool is_rtg_enable(void)
{
	return atomic_read(&g_rtg_enable) == 1;
}

int proc_rtg_open(struct inode *inode, struct file *filp)
{
	if ((current_uid().val != SYSTEM_SERVER_UID) &&
		(current_uid().val < MIN_APP_UID))
		return -OPEN_ERR_UID;

	return SUCC;
}

#ifdef CONFIG_COMPAT
long proc_rtg_compat_ioctl(struct file *file,
	unsigned int cmd, unsigned long arg)
{
	/*lint -e712*/
	return do_proc_rtg_ioctl(IOCTL_ABI_ARM32, file, cmd,
		(unsigned long)(compat_ptr((compat_uptr_t)arg)));
	/*lint +e712*/
}
#endif

long proc_rtg_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	return do_proc_rtg_ioctl(IOCTL_ABI_AARCH64, file, cmd, arg);
}

#ifdef CONFIG_HW_MTK_RTG_SCHED
int get_enable_type(void)
{
	return atomic_read(&g_enable_type);
}
#endif
/*lint -restore*/
