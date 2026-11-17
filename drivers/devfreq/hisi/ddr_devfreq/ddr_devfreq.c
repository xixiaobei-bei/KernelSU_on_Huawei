/*
 * ddr_devfreq.c
 *
 * ddr devfreq driver
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
#include <linux/version.h>
#include <linux/init.h>
#include <linux/of.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/devfreq.h>
#include <linux/pm_qos.h>
#include <linux/pm_opp.h>
#include <linux/hisi/hisi_devfreq.h>
#include "governor_pm_qos.h"
#include <linux/clk.h>
#include <linux/io.h>
#ifdef CONFIG_INPUT_PULSE_SUPPORT
#include <linux/input.h>
#include <linux/timer.h>
#include <linux/hardirq.h>
#include <linux/types.h>
#include <linux/workqueue.h>
#include <linux/hisi/pm/pwrctrl_multi_dfs.h>
#endif

#ifdef CONFIG_DDR_HW_VOTE_15BITS
#define VOTE_MAX_VALUE	0x7FFF
#define vote_quotient(vote)	(vote)
#else
#define VOTE_MAX_VALUE	0xFF
#define vote_quotient(vote)	((vote) >> 4)
#define vote_remainder(vote)	((vote) & 0xF)
#endif
#define FREQ_HZ	1000000
#define NUMBER_OF_BOOST_ELEMENT	0x1
#define NUMBER_OF_PM_QOS_ELEMENT	0x2

#ifdef CONFIG_PLATFORM_TELE_MNTN
#include <linux/tele_mntn.h>
#endif

#define MODULE_NAME "DDR_DEVFREQ"
typedef unsigned long (*calc_vote_value_func)(struct devfreq *devfreq,
					      unsigned long freq);

struct ddr_devfreq_device {
	struct devfreq *devfreq;
	struct clk *set;
	struct clk *get;
	calc_vote_value_func calc_vote_value;
	unsigned long freq;
};

#ifdef CONFIG_DDR_DEVFREQ_DFX
#define DDR_FREQ_DFX_INFO_BUFF_MAX	10
#define DEVFREQ_DFX_MAGIC_NUM	41250000000

struct ddr_freq_dfx_info {
	unsigned long slice_time_ns;
	unsigned long th_freq;
	unsigned long lat_freq;
	unsigned long cur_freq;
	unsigned long th_vote;
};

static unsigned int g_dfx_info_index = 0;
static struct ddr_freq_dfx_info g_dfx_info_buff[DDR_FREQ_DFX_INFO_BUFF_MAX];
#endif

#ifdef CONFIG_DDR_FREQ_BACKUP
#define MAX_FREQ_DRIVERS	3
#define DDR_VOTE_REG_ADDR	0xFFF01270
#define DDR_VOTE_REG_LENGTH	0x100
#define DDR_INVAILD_FREQ	1000000
static unsigned long g_ddr_freq_backup;
static spinlock_t g_ddr_freq_lock;
static int g_ddr_driver_nums;
#endif

#ifdef CONFIG_INPUT_PULSE_SUPPORT
struct ddrfreq_inputopen {
	struct input_handle *handle;
	struct work_struct inputopen_work;
};

static struct ddrfreq_inputopen g_inputopen;
static struct workqueue_struct *g_down_wq;
static struct pm_qos_request g_ddrfreq_min_req;
atomic_t g_flag;
struct delayed_work g_ddrfreq_begin, g_ddrfreq_end;
static int g_boost_ddrfreq_switch;
static unsigned int g_boost_ddr_dfs_band;
static unsigned int g_boost_ddr_dfs_last;

static void ddrfreq_begin_work(struct work_struct *work)
{
	pm_qos_add_request(&g_ddrfreq_min_req,
			   DFS_QOS_ID_DDR_MINFREQ,
			   g_boost_ddr_dfs_band);
	schedule_delayed_work(&g_ddrfreq_end, g_boost_ddr_dfs_last);
}

static void ddrfreq_end_work(struct work_struct *work)
{
	pm_qos_remove_request(&g_ddrfreq_min_req);
	atomic_dec(&g_flag);
}

/* Pulsed boost on input event raises CPUs to hispeed_freq. */
static void ddrfreq_input_event(struct input_handle *handle,
				unsigned int type,
				unsigned int code, int value)
{
	if (atomic_read(&g_flag) == 0x0) {
		atomic_inc(&g_flag);
		schedule_work(&g_ddrfreq_begin);
	}
}

static void ddrfreq_input_open(struct work_struct *w)
{
	struct ddrfreq_inputopen *io = container_of(w,
						    struct ddrfreq_inputopen,
						    inputopen_work);
	int error;

	error = input_open_device(io->handle);
	if (error != 0)
		input_unregister_handle(io->handle);
}

static int ddrfreq_input_connect(struct input_handler *handler,
				 struct input_dev *dev,
				 const struct input_device_id *id)
{
	struct input_handle *handle = NULL;
	int error;

	pr_info("%s: connect to %s\n", __func__, dev->name);
	handle = kzalloc(sizeof(struct input_handle), GFP_KERNEL);
	if (handle == NULL)
		return -ENOMEM;

	handle->dev = dev;
	handle->handler = handler;
	handle->name = "ddrfreq";

	error = input_register_handle(handle);
	if (error != 0)
		goto err;

	g_inputopen.handle = handle;
	queue_work(g_down_wq, &g_inputopen.inputopen_work);
	return 0;
err:
	kfree(handle);
	return error;
}

static void ddrfreq_input_disconnect(struct input_handle *handle)
{
	input_close_device(handle);
	input_unregister_handle(handle);
	kfree(handle);
}

static const struct input_device_id g_ddrfreq_ids[] = {
	{
		/* multi-touch touchscreen */
		.flags = INPUT_DEVICE_ID_MATCH_EVBIT |
			 INPUT_DEVICE_ID_MATCH_ABSBIT,
		.evbit = { BIT_MASK(EV_ABS) },
		.absbit = {
			[BIT_WORD(ABS_MT_POSITION_X)] =
				BIT_MASK(ABS_MT_POSITION_X) |
				BIT_MASK(ABS_MT_POSITION_Y)
		},
	}, {
		/* touchpad */
		.flags = INPUT_DEVICE_ID_MATCH_KEYBIT |
			 INPUT_DEVICE_ID_MATCH_ABSBIT,
		.keybit = { [BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH) },
		.absbit = {
			[BIT_WORD(ABS_X)] =
				BIT_MASK(ABS_X) | BIT_MASK(ABS_Y)
		},
	},
};

static struct input_handler g_ddrfreq_input_handler = {
	.event = ddrfreq_input_event,
	.connect = ddrfreq_input_connect,
	.disconnect = ddrfreq_input_disconnect,
	.name = "ddrfreq",
	.id_table = g_ddrfreq_ids,
};

#endif /* CONFIG_INPUT_PULSE_SUPPORT */

#ifdef CONFIG_PLATFORM_TELE_MNTN
static void tele_mntn_ddrfreq_setrate(struct devfreq *devfreq,
				      unsigned int new_freq)
{
	struct pwc_tele_mntn_dfs_ddr_qos_stru_s *qos = NULL;
	struct acore_tele_mntn_dfs_ddr_qosinfo_stru_s *info = NULL;
	struct devfreq_pm_qos_data *data = devfreq->data;
	struct acore_tele_mntn_stru_s *p_acore_tele_mntn = NULL;

	p_acore_tele_mntn = acore_tele_mntn_get();
	if (p_acore_tele_mntn == NULL)
		return;

	qos = &p_acore_tele_mntn->dfs_ddr.qos;
	info = &qos->info;
	info->qos_id = (short)data->pm_qos_class;
	if (current != NULL) {
		info->pid = current->pid;
		if (current->parent != NULL)
			info->ppid = current->parent->pid;
	}

	info->new_freq = new_freq;
	info->min_freq = (unsigned int)devfreq->min_freq;
	info->max_freq = (unsigned int)devfreq->max_freq;
	qos->qos_slice_time = get_slice_time();
	(void)tele_mntn_write_log(TELE_MNTN_QOS_DDR_ACPU,
				  sizeof(struct pwc_tele_mntn_dfs_ddr_qos_stru_s),
				  (void *)qos);
}

#endif

#ifdef CONFIG_DDR_DEVFREQ_DFX
static void record_ddr_freq_dfx_info(unsigned long th_freq, unsigned long lat_freq, unsigned long cur_freq, unsigned long th_vote)
{
	g_dfx_info_buff[g_dfx_info_index].slice_time_ns = hisi_getcurtime();
	g_dfx_info_buff[g_dfx_info_index].th_freq = th_freq;
	g_dfx_info_buff[g_dfx_info_index].lat_freq = lat_freq;
	g_dfx_info_buff[g_dfx_info_index].cur_freq = cur_freq;
	g_dfx_info_buff[g_dfx_info_index].th_vote = th_vote;
	g_dfx_info_index++;
	if (g_dfx_info_index >= DDR_FREQ_DFX_INFO_BUFF_MAX)
		g_dfx_info_index = 0;
}

static void print_ddr_freq_dfx_info(void)
{
	unsigned int i;
	unsigned int num = DDR_FREQ_DFX_INFO_BUFF_MAX - g_dfx_info_index;

	pr_err("num\tslice_time(ns)\tth_freq\tlat_freq\tcur_freq\tth_vote\n");
	for (i = 0; i < DDR_FREQ_DFX_INFO_BUFF_MAX; i++) {
		pr_err("%u\t%llu\t%llu\t%llu\t%llu\t%llu\n", num, g_dfx_info_buff[i].slice_time_ns,
			g_dfx_info_buff[i].th_freq, g_dfx_info_buff[i].lat_freq,
			g_dfx_info_buff[i].cur_freq, g_dfx_info_buff[i].th_vote);
		num++;
		if (num >= DDR_FREQ_DFX_INFO_BUFF_MAX)
			num = 0;
	}
}
#endif

static struct devfreq_pm_qos_data *g_ddata = NULL;

struct devfreq_pm_qos_info {
	char *devfreq_type;
	char *devfreq_name;
	struct devfreq_pm_qos_data freq_pm_qos_data;
};

enum {
	MEMORY_LATENCY = 0,
#ifdef CONFIG_DEVFREQ_DMSS_LATENCY
	DMSS_LATENCY,
#endif
	MEMORY_TPUT,
	MEMORY_TPUT_UP_THRESHOLD,
	DEVFREQ_QOS_INFO_MAX,
};
static struct devfreq_pm_qos_info g_pm_qos_info[DEVFREQ_QOS_INFO_MAX] = {
	{
		.devfreq_type = "memory_latency",
		.devfreq_name = "ddrfreq_latency",
		.freq_pm_qos_data.pm_qos_class = PM_QOS_MEMORY_LATENCY,
		.freq_pm_qos_data.freq = 0,
	},
#ifdef CONFIG_DEVFREQ_DMSS_LATENCY
	{
		.devfreq_type = "dmss_latency",
		.devfreq_name = "dmssfreq_latency",
		.freq_pm_qos_data.pm_qos_class = PM_QOS_DMSS_LATENCY,
		.freq_pm_qos_data.freq = 0,
	},
#endif
	{
		.devfreq_type = "memory_tput",
		.devfreq_name = "ddrfreq",
		.freq_pm_qos_data.pm_qos_class = PM_QOS_MEMORY_THROUGHPUT,
		.freq_pm_qos_data.freq = 0,
	},
	{
		.devfreq_type = "memory_tput_up_threshold",
		.devfreq_name = "ddrfreq_up_threshold",
		.freq_pm_qos_data.pm_qos_class = PM_QOS_MEMORY_THROUGHPUT_UP_THRESHOLD,
		.freq_pm_qos_data.freq = 0,
	},
};
static unsigned long calc_vote_value_hw(struct devfreq *devfreq,
					unsigned long freq)
{
	unsigned long freq_hz = freq / FREQ_HZ;
	unsigned long quotient = vote_quotient(freq_hz);

#ifndef CONFIG_DDR_HW_VOTE_15BITS
	unsigned long remainder = vote_remainder(freq_hz);
	unsigned long x_quotient, x_remainder;
	unsigned int lev;

	for (lev = 0; lev < devfreq->profile->max_state; lev++) {
		x_quotient = vote_quotient(devfreq->profile->freq_table[lev] /
					   FREQ_HZ);
		if (quotient == x_quotient) {
			x_remainder = vote_remainder(devfreq->profile->freq_table[lev] /
						     FREQ_HZ);
			if (remainder > x_remainder)
				quotient++;
			break;
		} else if (quotient < x_quotient) {
			break;
		}
	}
#endif
	if (quotient > VOTE_MAX_VALUE)
		quotient = VOTE_MAX_VALUE;

	return (quotient * FREQ_HZ);
}

static unsigned long calc_vote_value_ipc(struct devfreq *devfreq,
					 unsigned long freq)
{
	return freq;
}

static int ddr_devfreq_target(struct device *dev,
			      unsigned long *freq,
			      u32 flags)
{
	struct platform_device *pdev = container_of(dev,
						    struct platform_device,
						    dev);
	struct ddr_devfreq_device *ddev = platform_get_drvdata(pdev);
	struct devfreq_pm_qos_data *data = NULL;
	struct devfreq *devfreq = NULL;
	unsigned long _freq = *freq;
	unsigned int lev;
#ifdef CONFIG_DDR_DEVFREQ_DFX
	unsigned long th_freq = g_pm_qos_info[MEMORY_TPUT].freq_pm_qos_data.freq;
	unsigned long lat_freq = g_pm_qos_info[MEMORY_LATENCY].freq_pm_qos_data.freq;
#endif

	if (ddev == NULL)
		return -EINVAL;

	devfreq = ddev->devfreq;
	data = devfreq->data;

	if (data == NULL) {
		pr_err("%s %d, invalid pointer\n", __func__, __LINE__);
		return -EINVAL;
	}

#ifdef CONFIG_DDR_DEVFREQ_DFX
	if (data->pm_qos_class == PM_QOS_MEMORY_THROUGHPUT && _freq >= DEVFREQ_DFX_MAGIC_NUM) {
		print_ddr_freq_dfx_info();
		goto out;
	}
#endif

	if (data->pm_qos_class == PM_QOS_MEMORY_LATENCY ||
	    data->pm_qos_class == PM_QOS_MEMORY_THROUGHPUT) {
		unsigned long cur_freq;
		/* get current max freq */
		cur_freq = (g_pm_qos_info[MEMORY_LATENCY].freq_pm_qos_data.freq >=
			    g_pm_qos_info[MEMORY_TPUT].freq_pm_qos_data.freq) ?
			   g_pm_qos_info[MEMORY_LATENCY].freq_pm_qos_data.freq :
			   g_pm_qos_info[MEMORY_TPUT].freq_pm_qos_data.freq;

		/* update freq */
		data->freq = *freq;
		*freq = (g_pm_qos_info[MEMORY_LATENCY].freq_pm_qos_data.freq >
			 g_pm_qos_info[MEMORY_TPUT].freq_pm_qos_data.freq) ?
			g_pm_qos_info[MEMORY_LATENCY].freq_pm_qos_data.freq :
			g_pm_qos_info[MEMORY_TPUT].freq_pm_qos_data.freq;

		if (cur_freq != *freq) {
			(void)clk_set_rate(ddev->set,
					   ddev->calc_vote_value(devfreq, *freq));
			ddev->freq = *freq;
		}
	} else {
		data->freq = *freq;
		if (ddev->freq != *freq) {
			/* update ddr frequency down threshold and dmss frequency */
			(void)clk_set_rate(ddev->set,
					   ddev->calc_vote_value(devfreq, *freq));
			ddev->freq = *freq;
		}
	}

	/* fix: fail to update devfreq freq_talbe state. */
	*freq = clk_get_rate(ddev->get);

#ifdef CONFIG_DDR_DEVFREQ_DFX
	if (data->pm_qos_class == PM_QOS_MEMORY_THROUGHPUT)
		record_ddr_freq_dfx_info(th_freq, lat_freq, *freq, _freq);
#endif

	/* check */
	for (lev = 0; lev < devfreq->profile->max_state; lev++) {
		if (*freq == devfreq->profile->freq_table[lev])
			goto out;
	}

	/* exception */
	pr_err("\n");
	dev_err(dev,
		"odd freq status.\n<Target: %09lu hz>\n<Status: %09lu hz>\n%s",
		_freq, *freq, "--- freq table ---\n");
	for (lev = 0; lev < devfreq->profile->max_state; lev++)
		pr_err("<%d> %09lu hz\n", lev,
		       (unsigned long)devfreq->profile->freq_table[lev]);
	pr_err("------- end ------\n");

out:
#ifdef CONFIG_PLATFORM_TELE_MNTN
	tele_mntn_ddrfreq_setrate(devfreq, (unsigned int)ddev->freq);
#endif
	return 0;
}

/*
 * we can ignore setting current devfreq state,
 * because governor, "pm_qos", could get status through pm qos.
 */
static int ddr_devfreq_get_dev_status(struct device *dev,
				      struct devfreq_dev_status *stat)
{
	return 0;
}

static int ddr_devfreq_get_cur_status(struct device *dev,
				      unsigned long *freq)
{
	struct platform_device *pdev = container_of(dev,
						    struct platform_device,
						    dev);
	struct ddr_devfreq_device *ddev = platform_get_drvdata(pdev);

	if (ddev == NULL)
		return -EINVAL;

	*freq = clk_get_rate(ddev->get);
	return 0;
}

static struct devfreq_dev_profile g_ddr_devfreq_dev_profile = {
	.polling_ms = 0,
	.target = ddr_devfreq_target,
	.get_dev_status = ddr_devfreq_get_dev_status,
	.get_cur_freq = ddr_devfreq_get_cur_status,
	.freq_table = NULL,
	.max_state = 0,
};

#ifdef CONFIG_INPUT_PULSE_SUPPORT
static int devfreq_probe_pulse(void)
{
	int rc;
	static int inited = 0;
	struct device_node *root = NULL;

	if (inited != 0)
		return 0;

	root = of_find_compatible_node(NULL, NULL, "hisilicon,ddrfreq_boost");
	if (root == NULL) {
		pr_err("%s hisilicon,ddrfreq_boost no root node\n", __func__);
		goto no_root_node;
	}
	of_property_read_u32_array(root, "switch-value",
				   &g_boost_ddrfreq_switch,
				   NUMBER_OF_BOOST_ELEMENT);
	pr_err("switch-value: %d", g_boost_ddrfreq_switch);

	if (g_boost_ddrfreq_switch != 0x0) {
		of_property_read_u32_array(root, "ddrfreq_dfs_value",
					   &g_boost_ddr_dfs_band,
					   NUMBER_OF_BOOST_ELEMENT);
		of_property_read_u32_array(root, "ddrfreq_dfs_last",
					   &g_boost_ddr_dfs_last,
					   NUMBER_OF_BOOST_ELEMENT);
		pr_err("g_boost_ddr_dfs_band: %d, g_boost_ddr_dfs_last: %d\n",
		       g_boost_ddr_dfs_band, g_boost_ddr_dfs_last);
		rc = input_register_handler(&g_ddrfreq_input_handler);
		if (rc != 0)
			pr_warn("%s: failed to register input handler\n",
				__func__);
		g_down_wq = alloc_workqueue("ddrfreq_down", 0, 1);
		if (g_down_wq == NULL) {
			of_node_put(root);
			return -ENOMEM;
		}

		INIT_WORK(&g_inputopen.inputopen_work, ddrfreq_input_open);
		g_ddrfreq_min_req.pm_qos_class = 0;
		atomic_set(&g_flag, 0x0);
		INIT_DELAYED_WORK(&g_ddrfreq_begin,
				  (work_func_t)ddrfreq_begin_work);
		INIT_DELAYED_WORK(&g_ddrfreq_end,
				  (work_func_t)ddrfreq_end_work);
	}
	of_node_put(root);
no_root_node:
	inited = 1;
	return 0;
}
#endif

static int ddr_set_devfreq_property(struct device_node *np, struct platform_device *pdev, const char *type)
{
	int ret;
	int i;
	struct devfreq_pm_qos_data *pm_qos_data = NULL;

	for (i = 0; i < DEVFREQ_QOS_INFO_MAX; i++) {
		if (strncmp(g_pm_qos_info[i].devfreq_type, type, strlen(type) + 1) == 0)
			break;
	}

	if (i < DEVFREQ_QOS_INFO_MAX) {
		pm_qos_data = &g_pm_qos_info[i].freq_pm_qos_data;
		ret = of_property_read_u32_array(np,
							"pm_qos_data_reg",
							(u32 *)pm_qos_data,
							NUMBER_OF_PM_QOS_ELEMENT);
		if (ret != 0)
			pr_err("%s: %s %d, no type\n", MODULE_NAME, __func__, __LINE__);

		pr_err("%s: %s %d, per_hz %d  utilization %d\n",
				MODULE_NAME, __func__, __LINE__,
				pm_qos_data->bytes_per_sec_per_hz,
				pm_qos_data->bd_utilization);
		g_ddata = pm_qos_data;
		dev_set_name(&pdev->dev, g_pm_qos_info[i].devfreq_name);
	}else {
		pr_err("%s: %s %d, err type\n", MODULE_NAME, __func__, __LINE__);
		ret = -EINVAL;
	}

	return ret;
}

static void ddr_driver_num_init_for_s4(void)
{
#ifdef CONFIG_DDR_FREQ_BACKUP
	static int flag = 0;

	if (flag == 0) {
		g_ddr_driver_nums = 0;
		spin_lock_init(&g_ddr_freq_lock);
		flag++;
	}
#endif
}

int ddr_devfreq_set_vote_profile(struct device_node *np, struct ddr_devfreq_device *ddev,
	struct platform_device *pdev)
{
	const char *vote_method = NULL;

	if (of_property_read_string(np, "vote_method", &vote_method) != 0)
		ddev->calc_vote_value = calc_vote_value_ipc;
	else if (strncmp("hardware", vote_method, strlen(vote_method) + 1) == 0)
		ddev->calc_vote_value = calc_vote_value_hw;
	else
		ddev->calc_vote_value = calc_vote_value_ipc;

	if (dev_pm_opp_of_add_table(&pdev->dev) != 0) {
		ddev->devfreq = NULL;
	} else {
		g_ddr_devfreq_dev_profile.initial_freq = clk_get_rate(ddev->get);
		ddev->devfreq = devm_devfreq_add_device(&pdev->dev,
							&g_ddr_devfreq_dev_profile,
							"pm_qos",
							g_ddata);
	}
	if (IS_ERR_OR_NULL(ddev->devfreq)) {
		pr_err("%s: %s %d, Failed to init ddr devfreq_table\n",
		       MODULE_NAME, __func__, __LINE__);
		return -ENODEV;
	}

	/*
	 * cache value.
	 * It does not mean actual ddr clk currently,
	 * but a frequency cache of every clk setting in the module.
	 * Because, it is not obligatory that setting value is equal to
	 * the currently actual ddr clk frequency.
	 */
	mutex_lock(&ddev->devfreq->lock);
	ddev->freq = 0;
	if (ddev->devfreq != NULL) {
		ddev->devfreq->max_freq =
			g_ddr_devfreq_dev_profile.freq_table[g_ddr_devfreq_dev_profile.max_state - 1];
		ddev->devfreq->min_freq =
			g_ddr_devfreq_dev_profile.freq_table[g_ddr_devfreq_dev_profile.max_state - 1];
	}
	mutex_unlock(&ddev->devfreq->lock);

	platform_set_drvdata(pdev, ddev);

	return 0;
}

static int ddr_devfreq_probe(struct platform_device *pdev)
{
	struct ddr_devfreq_device *ddev = NULL;
	struct device_node *np = pdev->dev.of_node;
	const char *type = NULL;
	int ret;

#ifdef CONFIG_INPUT_PULSE_SUPPORT
	ret = devfreq_probe_pulse();
	if (ret == -ENOMEM)
		return ret;
#endif

	if (np == NULL) {
		pr_err("%s: %s %d, no device node\n",
				MODULE_NAME, __func__, __LINE__);
		ret = -ENODEV;
		goto out;
	}

	ret = of_property_read_string(np, "pm_qos_class", &type);
	if (ret != 0) {
		pr_err("%s: %s %d, no type\n",
				 MODULE_NAME, __func__, __LINE__);
		ret = -EINVAL;
		goto no_type;
	}

	ret = ddr_set_devfreq_property(np, pdev, type);
	if (ret == -EINVAL)
			goto err_type;

	ddr_driver_num_init_for_s4();

	ddev = kzalloc(sizeof(struct ddr_devfreq_device), GFP_KERNEL);
	if (ddev == NULL) {
		pr_err("%s: %s %d, no mem\n",
				MODULE_NAME, __func__, __LINE__);
		ret = -ENOMEM;
		goto no_men;
	}

	ddev->set = of_clk_get(np, 0);
	if (IS_ERR(ddev->set)) {
		pr_err("%s: %s %d, Failed to get set-clk\n",
				MODULE_NAME, __func__, __LINE__);
		ret = -ENODEV;
		goto no_clk1;
	}

	ddev->get = of_clk_get(np, 1);
	if (IS_ERR(ddev->get)) {
		pr_err("%s: %s %d, Failed to get get-clk\n",
				MODULE_NAME, __func__, __LINE__);
		ret = -ENODEV;
		goto no_clk2;
	}

	ret = ddr_devfreq_set_vote_profile(np, ddev, pdev);
	if (ret == -ENODEV)
		goto no_devfreq;

	pr_info("%s: <%s> ready\n", MODULE_NAME, type);

	return ret;

no_devfreq:
	clk_put(ddev->get);
	hisi_devfreq_free_freq_table(&pdev->dev,
					&g_ddr_devfreq_dev_profile.freq_table);
no_clk2:
	clk_put(ddev->set);
no_clk1:
	kfree(ddev);
no_men:
err_type:
no_type:
out:
	return ret;
}

static int ddr_devfreq_remove(struct platform_device *pdev)
{
	struct ddr_devfreq_device *ddev = NULL;
	struct devfreq_dev_profile *profile = NULL;

	ddev = platform_get_drvdata(pdev);
	if (ddev == NULL)
		return -EINVAL;

	profile = ddev->devfreq->profile;

	hisi_devfreq_free_freq_table(&pdev->dev, &profile->freq_table);
	devfreq_remove_device(ddev->devfreq);

	platform_set_drvdata(pdev, NULL);

	clk_put(ddev->get);
	clk_put(ddev->set);

	kfree(ddev);

	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id g_ddr_devfreq_of_match[] = {
	{ .compatible = "hisilicon,ddr_devfreq", },
	{ .compatible = "hisilicon,ddr_devfreq_latency", },
#ifdef CONFIG_DEVFREQ_DMSS_LATENCY
	{ .compatible = "hisilicon,ddr_devfreq_dmss_latency", },
#endif
	{ .compatible = "hisilicon,ddr_devfreq_up_threshold", },
	{ },
};
MODULE_DEVICE_TABLE(of, g_ddr_devfreq_of_match);
#endif

#ifdef CONFIG_DDR_FREQ_BACKUP
static int ddr_devfreq_freeze(struct device *dev)
{
	struct platform_device *pdev =
		container_of(dev, struct platform_device, dev);
	struct ddr_devfreq_device *ddev = platform_get_drvdata(pdev);
	struct devfreq *devfreq = NULL;

	if (ddev == NULL)
		return -EINVAL;
	devfreq = ddev->devfreq;

	spin_lock(&g_ddr_freq_lock);
	g_ddr_driver_nums++;
	if (g_ddr_driver_nums == MAX_FREQ_DRIVERS) {
		g_ddr_freq_backup = clk_get_rate(ddev->get);
		pr_err("freeze:freq_backup = %lu\n", g_ddr_freq_backup);
	}
	spin_unlock(&g_ddr_freq_lock);

	return 0;
}

static int ddr_devfreq_restore(struct device *dev)
{
	struct platform_device *pdev =
		container_of(dev, struct platform_device, dev);
	struct ddr_devfreq_device *ddev = platform_get_drvdata(pdev);
	struct devfreq *devfreq = NULL;
	void __iomem *ddr_vote_virt_addr = NULL;

	if (ddev == NULL)
		return -EINVAL;
	devfreq = ddev->devfreq;

	spin_lock(&g_ddr_freq_lock);
	g_ddr_driver_nums--;
	if (g_ddr_driver_nums == 0) {
		(void)clk_set_rate(ddev->set, DDR_INVAILD_FREQ);
		(void)clk_set_rate(ddev->set,
				   ddev->calc_vote_value(devfreq,
							 g_ddr_freq_backup));

		ddr_vote_virt_addr =
			ioremap(DDR_VOTE_REG_ADDR, DDR_VOTE_REG_LENGTH);
		if (ddr_vote_virt_addr == NULL) {
			pr_err("ddr_vote_virt_addr ioremap failed!\n");
			spin_unlock(&g_ddr_freq_lock);
			return -ENOMEM;
		}

		pr_err("restore:freq:%lu\n", readl(ddr_vote_virt_addr));
		iounmap(ddr_vote_virt_addr);
		ddr_vote_virt_addr = NULL;
	}
	spin_unlock(&g_ddr_freq_lock);

	return 0;
}

static const struct dev_pm_ops g_ddr_devfreq_dev_pm_ops = {
	.freeze = ddr_devfreq_freeze,
	.restore = ddr_devfreq_restore,
};
#endif

static struct platform_driver g_ddr_devfreq_driver = {
	.probe = ddr_devfreq_probe,
	.remove = ddr_devfreq_remove,
	.driver = {
		.name = "ddr_devfreq",
		.owner = THIS_MODULE,
#ifdef CONFIG_DDR_FREQ_BACKUP
		.pm = &g_ddr_devfreq_dev_pm_ops,
#endif
		.of_match_table = of_match_ptr(g_ddr_devfreq_of_match),
	},
};

module_platform_driver(g_ddr_devfreq_driver);
