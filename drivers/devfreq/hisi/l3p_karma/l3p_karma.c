/*
 *
 * Copyright (C) 2018 Hisilicon
 * License terms: GNU General Public License (GPL) version 2
 *
 */


#include <linux/devfreq.h>
#include <linux/ktime.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pm_opp.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/version.h>
#include <asm/compiler.h>
#include <trace/events/power.h>
#include <linux/cpu_pm.h>
#include <linux/cpu.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/perf_event.h>
#include <l3p_karma.h>

#include "governor.h"

#include "hisi_l3share.h"

#define CREATE_TRACE_POINTS
#include <trace/events/l3p_karma.h>

#define set_bits(mask, addr)  do {writel((((u32)readl(addr))|(mask)), (addr));} while (0)
#define clr_bits(mask, addr)  do {writel((((u32)readl(addr))&(~((u32)(mask)))), (addr));} while (0)


#define L3P_KARMA_PLATFORM_DEVICE_NAME			"l3p_karma"
#define L3P_KARMA_GOVERNOR_NAME					"l3p_karma_gov"
#define L3P_KARMA_DEFAULT_POLLING_MS			20

//#define L3P_KARMA_CPU_HWMON					1

#define THRESHOLD_MIN		0
#define THRESHOLD_MAX		1000

#define DSU_BW_MIN		0
#define DSU_BW_MAX		1000

#define WIDTH_TO_MASK(width)    ((1UL << (width)) - 1)

/* CPU PMU EVENT ID */
#define L3D_REFILL_EV		0x02A
#define L3D_EV			0x02B
#define INST_EV		0x08
#define CYC_EV		0x11

/* DSU PMU EVENT ID */
#define DSU_L3D_CACHE_EV			0x02B
#define DSU_L3D_REFILL_EV		0x02A
#define DSU_HZD_ADDR_EV			0x0D0
#define DSU_CYCLE_EV				0x011

/* profile mode */
enum profile_mode {
	DISABLE_MODE = 0,
	SLEEP_MODE,
	ENABLE_TEMP_MODE,
	ENABLE_MODE,
	MAX_MODE
};

enum threshold_idx {
	DSU_BW_HI,  		/* 0 */
	DSU_BW_MD, 	/* 1 */
	DSU_BW_LW, 	/* 2 */
	DSU_HIT_HI,		 /* 3 */
	DSU_HIT_MD, 	/* 4 */
	DSU_HIT_LW, 	/* 5 */
	REP_EFF_HI, 		/* 6 */
	REP_EFF_MD,		 /* 7 */
	REP_EFF_LW, 	/* 8 */
	DCP_EFF_HI, 		/* 9 */
	DCP_EFF_MD, 	/* 10 */
	DCP_EFF_LW, 	/* 11 */
	TOT_EFF_HI, 		/* 12 */
	TOT_EFF_MD, 	/* 13 */
	TOT_EFF_LW, 	/* 14 */
	ACP_HIT_HI, 		/*15 */
	ACP_HIT_MD, 	/* 16 */
	ACP_HIT_LW, 	/* 17 */
	HZD_HI, 			/* 18 */
	HZD_MD, 		/* 19 */
	HZD_LW, 		/* 20 */
	MAX_THRESHOLD, /* 21 */
};

const char *threshold_name_set[MAX_THRESHOLD] = {
	"dsu-bw-hi",
	"dsu-bw-md",
	"dsu-bw-lw",
	"dsu-hit-hi",
	"dsu-hit-md",
	"dsu-hit-lw",
	"rep-eff-hi",
	"rep-eff-md",
	"rep-eff-lw",
	"dcp-eff-hi",
	"dcp-eff-md",
	"dcp-eff-lw",
	"tot-eff-hi",
	"tot-eff-md",
	"tot-eff-lw",
	"acp-hit-hi",
	"acp-hit-md",
	"acp-hit-lw",
	"hzd-hi",
	"hzd-md",
	"hzd-lw"
};

/* CPU PMU EVENT IDX */
enum cpu_ev_idx {
	L3D_REFILL_IDX,
	L3D_IDX,
	INST_IDX,
	CYC_IDX,
	CPU_NUM_EVENTS
};

/* KARMA PMU EVENT IDX */
enum karma_ev_index {
	ACP_HIT_IDX,
	ACP_MISS_IDX,
	REP_GOOD_IDX,
	DCP_GOOD_IDX,
	REP_GEN_IDX,
	DCP_GEN_IDX,
	KARMA_NUM_EVENTS
};

/* DSU PMU EVENT IDX */
enum dsu_ev_index {
	DSU_L3D_CACHE_IDX,
	DSU_L3D_REFILL_IDX,
	DSU_HZD_ADDR_IDX,
	DSU_CYCLE_IDX,
	DSU_NUM_EVENTS
};

/* monitor_enable vote souces */
enum vote_src_mon {
	VOTE_NODE,
	VOTE_SHARE,
	VOTE_MAX
};

struct event_data {
	struct perf_event *pevent;
	unsigned long prev_count;
};

struct cpu_evt_count {
	unsigned long l3d_refill_cnt;
	unsigned long l3d_cnt;
	unsigned long inst_cnt;
	unsigned long cyc_cnt;
};

struct karma_evt_count {
	unsigned long acp_hit_cnt;
	unsigned long acp_miss_cnt;
	unsigned long rep_gd_cnt;
	unsigned long dcp_gd_cnt;
	unsigned long rep_gen_cnt;
	unsigned long dcp_gen_cnt;
};

struct dsu_evt_count {
	unsigned long l3d_cnt;
	unsigned long l3d_refill_cnt;
	unsigned long hzd_addr_cnt;
	unsigned long cycle_cnt;
};

struct cpu_evt_data {
	struct event_data events[CPU_NUM_EVENTS];
	ktime_t prev_ts;
};

struct karma_evt_data {
	struct event_data events[KARMA_NUM_EVENTS];
};

struct dsu_evt_data {
	struct event_data events[DSU_NUM_EVENTS];
};

struct cpu_grp_info {
	cpumask_t cpus;
	cpumask_t inited_cpus;
	unsigned int event_ids[CPU_NUM_EVENTS];
	struct cpu_evt_data *evtdata;
	struct notifier_block perf_cpu_notif;
	struct list_head mon_list;
};

struct cpu_hwmon{
	int (*start_hwmon)(struct cpu_hwmon *hw);
	void (*stop_hwmon)(struct cpu_hwmon *hw);
	unsigned long (*get_cnt)(struct cpu_hwmon *hw);
	struct cpu_grp_info *cpu_grp;
	cpumask_t cpus;
	unsigned int num_cores;
	struct cpu_evt_count *evtcnts;
	unsigned long total_refill;
	unsigned long total_access;
	unsigned long total_inst;
	unsigned long total_cyc;
};

struct karma_hwmon {
	int (*start_hwmon)(struct karma_hwmon *hw);
	void (*stop_hwmon)(struct karma_hwmon *hw);
	unsigned long (*get_cnt)(struct karma_hwmon *hw);
	unsigned int event_ids[KARMA_NUM_EVENTS];
	struct karma_evt_count count;
	struct karma_evt_data hw_data;
};

struct dsu_hwmon {
	int (*start_hwmon)(struct dsu_hwmon *hw);
	void (*stop_hwmon)(struct dsu_hwmon *hw);
	unsigned long (*get_cnt)(struct dsu_hwmon *hw);
	unsigned int event_ids[DSU_NUM_EVENTS];
	struct dsu_evt_count count;
	struct dsu_evt_data hw_data;
};

#define to_evtdata(cpu_grp, cpu) \
	(&cpu_grp->evtdata[cpu - cpumask_first(&cpu_grp->cpus)])
#define to_evtcnts(hw, cpu) \
	(&hw->evtcnts[cpu - cpumask_first(&hw->cpus)])

static LIST_HEAD(perf_mon_list);
static DEFINE_MUTEX(list_lock);

struct set_reg {
	u32 offset;
	u32 shift;
	u32 width;
	u32 value;
};

struct init_reg {
	u32 offset;
	u32 value;
};

struct profile_data {
	struct set_reg *opp_entry;
	u8 opp_count;
};

struct time_type {
	unsigned long usec_delta;
	unsigned int last_update;
} time;

struct alg_type {
	unsigned int dsu_hit_rate;
	unsigned int dsu_bw;;
	unsigned int rep_eff;
	unsigned int dcp_eff;
	unsigned int tot_eff;
	unsigned int hzd_rate;
	unsigned int acp_hit_rate;
} alg_info;

struct threshold_data {
	int count;
	u32 *data;
};

struct l3p_karma {
	struct devfreq *devfreq;
	struct platform_device *pdev;
	struct devfreq_dev_profile *devfreq_profile;
	void __iomem *base;
	u32 polling_ms;
	unsigned int monitor_enable;
	bool mon_started;
	u8 vote_mon_dis;
	struct mutex mon_mutex_lock;
	u32 control_data[KARMA_SYS_CTRL_NUM-1];
	bool fcm_idle;
	spinlock_t data_spinlock;

	struct profile_data *profile_table;
	unsigned long *freq_table;
	int table_len;
	unsigned long initial_freq;
	unsigned long cur_freq;

	unsigned long freq_min;
	unsigned long freq_max;

	struct time_type time;

	struct threshold_data **threshold_table;
	int threshold_len;

	struct workqueue_struct *update_wq;
	struct delayed_work work;
	u32 work_ms;
	int quick_sleep_enable;

	struct cpu_hwmon *cpu_hw;
	struct karma_hwmon *karma_hw;
	struct dsu_hwmon *dsu_hw;

	struct notifier_block idle_nb;
	struct notifier_block acp_notify;
	struct attribute_group *attr_grp;
};


static const struct of_device_id l3p_karma_devfreq_id[] = {
	{.compatible = "hisi,l3p_karma"},
	{}
};



static int l3p_start_monitor(struct devfreq *df)
{
	struct l3p_karma *l3p = dev_get_drvdata(df->dev.parent);
#ifdef L3P_KARMA_CPU_HWMON
	struct cpu_hwmon *cpu_hw = NULL;
#endif
	struct karma_hwmon *karma_hw = NULL;
	struct dsu_hwmon *dsu_hw = NULL;
	struct device *dev = NULL;
	int ret = 0;

	if (!l3p->monitor_enable || l3p->mon_started) {
		return 0;
	}

#ifdef L3P_KARMA_CPU_HWMON
	cpu_hw = l3p->cpu_hw;
#endif
	karma_hw = l3p->karma_hw;
	dsu_hw = l3p->dsu_hw;
	dev = df->dev.parent;

#ifdef L3P_KARMA_CPU_HWMON
	ret = cpu_hw->start_hwmon(cpu_hw);
	if (ret) {
		dev_err(dev, "Unable to start CPU HW monitor! (%d)\n", ret);
		goto cpu_err;
	}
#endif

	ret = karma_hw->start_hwmon(karma_hw);
	if (ret) {
		dev_err(dev, "Unable to start KARMA HW monitor! (%d)\n", ret);
		goto cpu_err;
	}

	ret = dsu_hw->start_hwmon(dsu_hw);
	if (ret) {
		dev_err(dev, "Unable to start DSU HW monitor! (%d)\n", ret);
		goto karma_err;
	}

	devfreq_monitor_start(df);
	l3p->mon_started = true;

	return 0;

karma_err:
	karma_hw->stop_hwmon(karma_hw);
cpu_err:
#ifdef L3P_KARMA_CPU_HWMON
	cpu_hw->stop_hwmon(cpu_hw);
#endif
	return ret;
}

static void l3p_stop_monitor(struct devfreq *df)
{
	struct l3p_karma *l3p = dev_get_drvdata(df->dev.parent);
#ifdef L3P_KARMA_CPU_HWMON
	struct cpu_hwmon *cpu_hw = NULL;
#endif
	struct karma_hwmon *karma_hw = NULL;
	struct dsu_hwmon *dsu_hw = NULL;

	if (!l3p->monitor_enable || !l3p->mon_started) {
		return ;
	}

#ifdef L3P_KARMA_CPU_HWMON
	cpu_hw = l3p->cpu_hw;
#endif
	karma_hw = l3p->karma_hw;
	dsu_hw = l3p->dsu_hw;

	l3p->mon_started = false;

	devfreq_monitor_stop(df);
	//cancel_delayed_work_sync(&l3p->work);
#ifdef L3P_KARMA_CPU_HWMON
	cpu_hw->stop_hwmon(cpu_hw);
#endif
	karma_hw->stop_hwmon(karma_hw);
	dsu_hw->stop_hwmon(dsu_hw);
}

static int l3p_karma_init(struct platform_device *pdev)
{
	struct l3p_karma *l3p = platform_get_drvdata(pdev);
	struct device *dev = &pdev->dev;
	struct init_reg *init_table = NULL;
	struct device_node *node = pdev->dev.of_node;
	struct device_node *parent = NULL;
	struct device_node *init= NULL;
	int init_count = 0;
	int i = 0, offset = 0, value = 0;
	int ret = 0;

	of_node_get(node);

	parent = of_get_child_by_name(node, "initialize");
	if (!parent) {
		dev_err(dev, "Failed to find initialize node\n");
		ret = -EINVAL;
		goto err_out;
	}

	init_count = of_get_child_count(parent);
	if (!init_count) {
		dev_err(dev, "no any initialize config\n");
		ret = -EINVAL;
		goto err_out;
	}
	dev_info(dev, "init_count = %d\n", init_count);

	init_table = devm_kzalloc(dev, sizeof(struct init_reg) * init_count, GFP_KERNEL);
	if (IS_ERR_OR_NULL(init_table)) {
		dev_err(dev, "Failed to allocate for karma init_table\n");
		ret =  -ENOMEM;
		goto err_out;
	}

	i =0;
	for_each_child_of_node(parent, init) {
		dev_info(dev, "init name:%s\n", init->name);
		ret = of_property_read_u32(init, "offset", &(init_table[i].offset));
		if (ret) {
			dev_err(dev, "parse %s offset failed%d!\n", init->name, ret);
			goto err_free;
		}
		dev_info(dev, "offset = 0x%x\n", init_table[i].offset);

		ret = of_property_read_u32(init, "value", (&(init_table[i].value)));
		if (ret) {
			dev_err(dev, "parse %s value failed%d!\n", init->name, ret);
			goto err_free;
		}
		dev_info(dev, "value = 0x%x\n", init_table[i].value);
		i++;
	}

	for(i = 0; i < init_count; i++) {
		offset = init_table[i].offset;
		value = init_table[i].value;
#ifdef DVM_SYNC_CLEAR_FIX
		/*
		 * DVM sync clear disable always
		 */
		if(KARMA_REP_CONTROL_OFFSET == offset &&
			0 == ((value & BIT(DVM_SYNC_CLEAR)) >> DVM_SYNC_CLEAR)) {
			value |= BIT(DVM_SYNC_CLEAR);
			WARN_ON(1);
		}
#endif
		writel(value, l3p->base + offset);
	}

err_free:
	devm_kfree(dev, init_table);
	init_table = NULL;
err_out:
	of_node_put(node);
	return ret;
}

static void l3p_karma_enable(struct l3p_karma *l3p)
{
	/* enable  karma*/
	set_bits(BIT(0), (l3p->base + KARMA_MAIN_ENABLE_OFFSET));
}

static void l3p_karma_disable(struct l3p_karma *l3p)
{
	u32 value = 0;

	/* disable  karma*/
	clr_bits(BIT(0), (l3p->base + KARMA_MAIN_ENABLE_OFFSET));

	/*
	* if cur mode is sleep mode, enable ACP outstanding firstly,
	* otherwise can not wait idle status
	* bit15:19  ACP outstanding
	*/
	if(SLEEP_MODE == l3p->cur_freq) {
		value = readl(l3p->base + KARMA_GLOBAL_CONTROL_OFFSET);
		value |= 0xF8000;
		writel(value, l3p->base + KARMA_GLOBAL_CONTROL_OFFSET);
	}

	/* check ACP outstanding is 0 or not, at last */
	value = readl(l3p->base + KARMA_GLOBAL_CONTROL_OFFSET);
	if(0 == (value & 0xF8000)) {
		BUG_ON(1);
	}

	value = readl(l3p->base + KARMA_IDLE_STATUS_OFFSET);
	while(0 == (value & BIT(0))){
		value = readl(l3p->base + KARMA_IDLE_STATUS_OFFSET);
	}
}

static void l3p_save_reg_data(struct l3p_karma *l3p)
{
	int i = 0;
	u32 offset = 0, value = 0;

	/* not include idle status register */
	for(i = 0; i < (KARMA_SYS_CTRL_NUM - 1); i++){
		offset = i * 4;
		value = readl(l3p->base + offset);
#ifdef DVM_SYNC_CLEAR_FIX
		/*
		 * DVM sync clear disable always
		 */
		if(KARMA_REP_CONTROL_OFFSET == offset &&
			0 == ((value & BIT(DVM_SYNC_CLEAR)) >> DVM_SYNC_CLEAR)) {
			value |= BIT(DVM_SYNC_CLEAR);
			BUG_ON(1);
		}
#endif
		l3p->control_data[i] = value;
	}
}

static void l3p_restore_reg_data(struct l3p_karma *l3p)
{
	int i = 0;
	u32 enable;
	u32 offset = 0, value = 0;

	/*
	  * if karma is enabled, disable it firstly
	  * eg: fcm not power off/on in fact
	  * suspend abort
	  */
	enable = readl(l3p->base + KARMA_MAIN_ENABLE_OFFSET);
	if(1 == (enable & BIT(0))) {
		l3p_karma_disable(l3p);
	}

	/* firstly restore exclude enable register, at last do it */
	for(i = 1; i < (KARMA_SYS_CTRL_NUM - 1); i++){
		offset = i * 4;
		value = l3p->control_data[i];
#ifdef DVM_SYNC_CLEAR_FIX
		/*
		 * DVM sync clear disable always
		 */
		if(KARMA_REP_CONTROL_OFFSET == offset &&
			0 == ((value & BIT(DVM_SYNC_CLEAR)) >> DVM_SYNC_CLEAR)) {
			value |= BIT(DVM_SYNC_CLEAR);
			BUG_ON(1);
		}
#endif
		writel(value, (l3p->base + offset));
	}
	writel(l3p->control_data[0], (l3p->base + KARMA_MAIN_ENABLE_OFFSET));
}


static ssize_t show_monitor_enable(struct device *dev, struct device_attribute *attr,
			char *buf)
{
	ssize_t ret = 0;
	struct devfreq *df = to_devfreq(dev);
	struct l3p_karma *l3p = dev_get_drvdata(df->dev.parent);

	mutex_lock(&l3p->mon_mutex_lock);
	ret = scnprintf(buf, PAGE_SIZE, "monitor_enable=%u\n", l3p->monitor_enable);
	mutex_unlock(&l3p->mon_mutex_lock);

	return ret;
}

static ssize_t store_monitor_enable(struct device *dev,
			struct device_attribute *attr, const char *buf,
			size_t count)
{
	struct devfreq *df = to_devfreq(dev);
	struct l3p_karma *l3p = dev_get_drvdata(df->dev.parent);
	unsigned int val;
	int ret = 0;

	ret = kstrtouint(buf, 10, &val);
	if (ret)
		return ret;

	if (val != 0)
		val = 1;

	mutex_lock(&l3p->mon_mutex_lock);
	if(1 == val) {
		l3p->vote_mon_dis &= (~BIT(VOTE_NODE));
	} else if(0 == val) {
		l3p->vote_mon_dis |= BIT(VOTE_NODE);
	}
	if (0 == l3p->monitor_enable && 1 == val && 0 == l3p->vote_mon_dis) {
		l3p->monitor_enable = 1;
		ret = l3p_start_monitor(df);
		if (ret) {
			/* handle error */
			l3p->monitor_enable = 0;
			//l3p_karma_disable(l3p);
			ret = -EINVAL;
		} else {
			/* enable karma */
			spin_lock(&l3p->data_spinlock);
			l3p_restore_reg_data(l3p);
			l3p_karma_enable(l3p);
			spin_unlock(&l3p->data_spinlock);
		}
	} else if (1 == l3p->monitor_enable && 0 == val) {
		/* NOTICE
		* this thread can not sleep before save register data,
		* if get mon_mutex_lock already
		*/
		spin_lock(&l3p->data_spinlock);
		l3p_karma_disable(l3p);
		l3p_save_reg_data(l3p);
		spin_unlock(&l3p->data_spinlock);
		l3p_stop_monitor(df);
		l3p->monitor_enable = 0;
	}
	mutex_unlock(&l3p->mon_mutex_lock);

	return  ret ? ret : count;
}
static DEVICE_ATTR(monitor_enable, 0660, show_monitor_enable, store_monitor_enable);


#ifdef CONFIG_HISI_L3P_KARMA_DEBUG
static ssize_t show_reg_cfg(struct device *dev, struct device_attribute *attr,
			char *buf)
{
	ssize_t ret = 0;
	struct devfreq *df = to_devfreq(dev);
	struct l3p_karma *l3p = dev_get_drvdata(df->dev.parent);
	ssize_t count = 0;
	int i = 0;

	spin_lock(&l3p->data_spinlock);
	for(i = 0; i< KARMA_SYS_CTRL_NUM; i++){
		ret = snprintf(buf + count, (PAGE_SIZE - count), /* unsafe_function_ignore: snprintf */
			 "offset:0x%x \t value:0x%x\n", (i * 4), readl(l3p->base + i * 4));
		if (ret >= (PAGE_SIZE - count) || ret < 0) { /*lint !e574 */
			goto err;
		}
		count += ret;
	}
err:
	spin_unlock(&l3p->data_spinlock);
	return count;
}
static DEVICE_ATTR(reg_cfg, 0660, show_reg_cfg, NULL);

static ssize_t show_threshold(struct device *dev, struct device_attribute *attr,
			char *buf)
{
	ssize_t ret = 0;
	struct devfreq *devfreq = to_devfreq(dev);
	struct l3p_karma *l3p = dev_get_drvdata(devfreq->dev.parent);
	struct threshold_data *threshold = NULL;
	ssize_t count = 0;
	int state = 0, idx = 0, id = 0;

	mutex_lock(&l3p->devfreq->lock);
	/* traverse from sleep state, because disable state has no threshold */
	for(state = 1; state< MAX_MODE; state++){
		threshold = l3p->threshold_table[state];
		for(idx = 0; idx < MAX_THRESHOLD; idx++) {
			if(0 == threshold[idx].count) {
				continue;
			}
			for(id = 0; id < threshold[idx].count; id++) {
				ret = snprintf(buf + count, (PAGE_SIZE - count), /* unsafe_function_ignore: snprintf */
					 "state:%u\t %s\t idx:%u\t id:%u\t value:%u\n",
					 state, threshold_name_set[idx], idx, id, threshold[idx].data[id]);
				if (ret >= (PAGE_SIZE - count) || ret < 0) { /*lint !e574 */
					goto err;
				}
				count += ret;
			}
		}

	}
err:
	mutex_unlock(&l3p->devfreq->lock);
	return count;
}

static int cmd_parse(char *para_cmd, char *argv[], int max_args)
{
    int para_id = 0;

	while (*para_cmd != '\0')
	{
		if (para_id >= max_args)
			break;
		while (*para_cmd == ' ')
			para_cmd++;

		if ('\0' == *para_cmd)
			break;

		argv[para_id] = para_cmd;
		para_id++;

		while ((*para_cmd != ' ') && (*para_cmd != '\0'))
			para_cmd++;

		if ('\0' == *para_cmd)
			break;

		*para_cmd = '\0';
		para_cmd++;

	}

	return para_id;
}

#define LOCAL_BUF_MAX       (128)
static ssize_t store_threshold(struct device *dev,
			struct device_attribute *attr, const char *buf,
			size_t count)
{
	struct devfreq *devfreq = to_devfreq(dev);
	struct l3p_karma *l3p = dev_get_drvdata(devfreq->dev.parent);
	struct threshold_data *threshold = NULL;
	char local_buf[LOCAL_BUF_MAX] = {0};
	char *argv[4] = {0};
	unsigned int state = 0, idx = 0, value = 0;
	int id = 0, argc = 0, ret = 0;

	if (count >= sizeof(local_buf)) {
		return -ENOMEM;
	}

	memset(local_buf, 0, sizeof(local_buf)); /* unsafe_function_ignore: memset */

	strncpy(local_buf, buf, min_t(size_t, sizeof(local_buf) - 1, count)); /* unsafe_function_ignore: strncpy */

	argc = cmd_parse(local_buf, argv, sizeof(argv) / sizeof(argv[0]));
	if (4 != argc) {
		dev_err(dev, "error, only surport four para\n");
		return -EINVAL;
	}

	ret = sscanf(argv[0], "%u", &state); /* unsafe_function_ignore: sscanf */
	if (ret != 1) {
		dev_err(dev, "parse state error %s\n", argv[0]);
		return -EINVAL;
	}

	ret = sscanf(argv[1], "%u", &idx);/* unsafe_function_ignore: sscanf */
	if (ret != 1) {
		dev_err(dev, "parse idx error %s\n", argv[1]);
		return -EINVAL;
	}

	ret = sscanf(argv[2], "%d", &id);/* unsafe_function_ignore: sscanf */
	if (ret != 1) {
		dev_err(dev, "parse id error %s\n", argv[2]);
		return -EINVAL;
	}

	ret = sscanf(argv[3], "%u", &value);/* unsafe_function_ignore: sscanf */
	if (ret != 1) {
		dev_err(dev, "parse value error %s\n", argv[3]);
		return -EINVAL;
	}

	dev_err(dev, "state=%u, idx=%u, id=%d, value=%u\n", state, idx, id, value);

	if(0 == state || state >= MAX_MODE) {
		dev_err(dev, "state is not valid, state = %u\n", state);
		return -EINVAL;
	}

	threshold = l3p->threshold_table[state];
	if(idx >= MAX_THRESHOLD || 0 == threshold[idx].count) {
		dev_err(dev, "idx is not valid, idx = %u\n", idx);
		return -EINVAL;
	}

	if(id >= threshold[idx].count) {
		dev_err(dev, "id is not valid, id = %d\n", id);
		return -EINVAL;
	}

	if(value > THRESHOLD_MAX) {
		dev_err(dev, "value is not valid, value = %u\n", value);
		return -EINVAL;
	}

	mutex_lock(&l3p->devfreq->lock);
	threshold[idx].data[id] = value;
	ret = update_devfreq(devfreq);
	if(0 == ret) {
		ret = count;
	}
	mutex_unlock(&l3p->devfreq->lock);

	return  count;
}
static DEVICE_ATTR(threshold, 0660, show_threshold, store_threshold);

/* quick_sleep_enable */
static ssize_t show_quick_sleep_enable(struct device *dev, struct device_attribute *attr,
			char *buf)
{
	ssize_t ret = 0;
	struct devfreq *df = to_devfreq(dev);
	struct l3p_karma *l3p = dev_get_drvdata(df->dev.parent);

	mutex_lock(&l3p->devfreq->lock);
	ret = scnprintf(buf, PAGE_SIZE, "quick_sleep_enable=%d\n", l3p->quick_sleep_enable);
	mutex_unlock(&l3p->devfreq->lock);

	return ret;
}

static ssize_t store_quick_sleep_enable(struct device *dev,
			struct device_attribute *attr, const char *buf,
			size_t count)
{
	struct devfreq *devfreq = to_devfreq(dev);
	struct l3p_karma *l3p = dev_get_drvdata(devfreq->dev.parent);
	int value = 0;
	int ret = 0;

	ret = sscanf(buf, "%d", &value);	/* unsafe_function_ignore: sscanf */
	if (ret != 1 || value < 0) {
		dev_err(dev, "input data is not valid, value = %d\n", value);
		return -EINVAL;
	}

	mutex_lock(&l3p->devfreq->lock);
	l3p->quick_sleep_enable = value;
	ret = update_devfreq(devfreq);
	if(0 == ret) {
		ret = count;
	}
	mutex_unlock(&l3p->devfreq->lock);

	return ret;
}
static DEVICE_ATTR(quick_sleep_enable, 0660, show_quick_sleep_enable, store_quick_sleep_enable);

/* work_ms */
static ssize_t show_work_ms(struct device *dev, struct device_attribute *attr,
			char *buf)
{
	ssize_t ret = 0;
	struct devfreq *df = to_devfreq(dev);
	struct l3p_karma *l3p = dev_get_drvdata(df->dev.parent);

	mutex_lock(&l3p->devfreq->lock);
	ret = scnprintf(buf, PAGE_SIZE, "work_ms=%u\n", l3p->work_ms);
	mutex_unlock(&l3p->devfreq->lock);

	return ret;
}

#define WORK_MS_MIN		2
#define WOKR_MS_MAX		20
static ssize_t store_work_ms(struct device *dev,
			struct device_attribute *attr, const char *buf,
			size_t count)
{
	struct devfreq *devfreq = to_devfreq(dev);
	struct l3p_karma *l3p = dev_get_drvdata(devfreq->dev.parent);
	unsigned int value = 0;
	int ret = 0;

	ret = sscanf(buf, "%u", &value);	/* unsafe_function_ignore: sscanf */
	if (ret != 1 || value > WOKR_MS_MAX || value < WORK_MS_MIN) {
		dev_err(dev, "input data is not valid, value = %u\n", value);
		return -EINVAL;
	}

	mutex_lock(&l3p->devfreq->lock);
	l3p->work_ms = value;
	ret = update_devfreq(devfreq);
	if(0 == ret) {
		ret = count;
	}
	mutex_unlock(&l3p->devfreq->lock);

	return ret;
}
static DEVICE_ATTR(work_ms, 0660, show_work_ms, store_work_ms);
#endif


static struct attribute *dev_attr[] = {
	&dev_attr_monitor_enable.attr,
#ifdef CONFIG_HISI_L3P_KARMA_DEBUG
	&dev_attr_quick_sleep_enable.attr,
	&dev_attr_work_ms.attr,
	&dev_attr_reg_cfg.attr,
	&dev_attr_threshold.attr,
#endif
	NULL,
};

static struct attribute_group dev_attr_group = {
	.name = "karma",
	.attrs = dev_attr,
};

static inline unsigned long read_event(struct event_data *event)
{
	unsigned long ev_count;
	u64 total, enabled, running;

	if (!event->pevent){
		pr_err("l3p_karma: %s pevent is NULL\n", __func__);
		return 0;
	}

	total = perf_event_read_value(event->pevent, &enabled, &running);
	//trace_printk("config=0x%x, total= %llu\n", event->pevent->attr.config, total);
	ev_count = total - event->prev_count;
	event->prev_count = total;
	return ev_count;
}

#ifdef L3P_KARMA_CPU_HWMON
static void cpu_read_perf_counters(int cpu, struct cpu_hwmon *hw)
{
	struct cpu_grp_info *cpu_grp = hw->cpu_grp;
	struct cpu_evt_data *evtdata = to_evtdata(cpu_grp, cpu);
	struct cpu_evt_count *evtcnts = to_evtcnts(hw, cpu);

	evtcnts->l3d_refill_cnt = read_event(&evtdata->events[L3D_REFILL_IDX]);
	evtcnts->l3d_cnt = read_event(&evtdata->events[L3D_IDX]);
	evtcnts->inst_cnt = read_event(&evtdata->events[INST_IDX]);
	evtcnts->cyc_cnt = read_event(&evtdata->events[CYC_IDX]);
}

static unsigned long cpu_get_cnt(struct cpu_hwmon *hw)
{
	int cpu;
	struct cpu_grp_info *cpu_grp = hw->cpu_grp;
	struct cpu_evt_count *evtcnts =  NULL;

	hw->total_refill= 0;
	hw->total_access = 0;
	hw->total_inst= 0;
	hw->total_cyc = 0;
	for_each_cpu(cpu, &cpu_grp->inited_cpus){
		cpu_read_perf_counters(cpu, hw);
		evtcnts =  to_evtcnts(hw, cpu);
		hw->total_refill += evtcnts->l3d_refill_cnt;
		hw->total_access += evtcnts->l3d_cnt;
		hw->total_inst += evtcnts->inst_cnt;
		hw->total_cyc += evtcnts->cyc_cnt;
	}
	return 0;
}
#endif

static unsigned long karma_get_cnt(struct karma_hwmon *hw)
{
	struct karma_evt_data *hw_data = &(hw->hw_data);

	hw->count.acp_hit_cnt =
			read_event(&hw_data->events[ACP_HIT_IDX]);
	hw->count.acp_miss_cnt =
			read_event(&hw_data->events[ACP_MISS_IDX]);
	hw->count.rep_gd_cnt =
			read_event(&hw_data->events[REP_GOOD_IDX]);
	hw->count.dcp_gd_cnt =
			read_event(&hw_data->events[DCP_GOOD_IDX]);
	hw->count.rep_gen_cnt =
			read_event(&hw_data->events[REP_GEN_IDX]);
	hw->count.dcp_gen_cnt =
			read_event(&hw_data->events[DCP_GEN_IDX]);

	return 0;
}

static unsigned long dsu_get_cnt(struct dsu_hwmon *hw)
{
	struct dsu_evt_data *hw_data = &(hw->hw_data);

	hw->count.l3d_cnt =
			read_event(&hw_data->events[DSU_L3D_CACHE_IDX]);
	hw->count.l3d_refill_cnt =
			read_event(&hw_data->events[DSU_L3D_REFILL_IDX]);
	hw->count.hzd_addr_cnt=
			read_event(&hw_data->events[DSU_HZD_ADDR_IDX]);
	hw->count.cycle_cnt=
			read_event(&hw_data->events[DSU_CYCLE_IDX]);

	return 0;
}

#ifdef L3P_KARMA_CPU_HWMON
static void cpu_delete_events(struct cpu_evt_data *evtdata)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(evtdata->events); i++) {
		evtdata->events[i].prev_count = 0;
		if(evtdata->events[i].pevent){
			perf_event_disable(evtdata->events[i].pevent);
			perf_event_release_kernel(evtdata->events[i].pevent);
			evtdata->events[i].pevent = NULL;
		}
	}
}
#endif

static void karma_delete_events(struct karma_evt_data *hw_data)
{
	int i;

	for (i = 0; i < KARMA_NUM_EVENTS; i++) {
		hw_data->events[i].prev_count = 0;
		if(hw_data->events[i].pevent){
			perf_event_release_kernel(hw_data->events[i].pevent);
			hw_data->events[i].pevent = NULL;
		}
	}
}

static void dsu_delete_events(struct dsu_evt_data *hw_data)
{
	int i;

	for (i = 0; i < DSU_NUM_EVENTS; i++) {
		hw_data->events[i].prev_count = 0;
		if(hw_data->events[i].pevent){
			perf_event_release_kernel(hw_data->events[i].pevent);
			hw_data->events[i].pevent = NULL;
		}
	}
}

#ifdef L3P_KARMA_CPU_HWMON
static void cpu_stop_hwmon(struct cpu_hwmon *hw)
{
	int cpu;
	struct cpu_grp_info *cpu_grp = hw->cpu_grp;
	struct cpu_evt_count *evtcnts;

	get_online_cpus();
	for_each_cpu(cpu, &cpu_grp->inited_cpus) {
		cpu_delete_events(to_evtdata(cpu_grp, cpu));
		evtcnts = to_evtcnts(hw, cpu);
		evtcnts->l3d_refill_cnt = 0;
		evtcnts->l3d_cnt = 0;
		evtcnts->inst_cnt = 0;
		evtcnts->cyc_cnt = 0;
		hw->total_refill = 0;
		hw->total_access = 0;
		hw->total_inst = 0;
		hw->total_cyc = 0;
	}
	mutex_lock(&list_lock);
	if (!cpumask_equal(&cpu_grp->cpus, &cpu_grp->inited_cpus)) {
		list_del(&cpu_grp->mon_list);
	}
	mutex_unlock(&list_lock);
	cpumask_clear(&cpu_grp->inited_cpus);
	put_online_cpus();

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
	unregister_cpu_notifier(&cpu_grp->perf_cpu_notif);
#else
	cpuhp_remove_state_nocalls(CPUHP_AP_HISI_L3P_KARMA_NOTIFY_PREPARE);
#endif
}
#endif

static void karma_stop_hwmon(struct karma_hwmon *hw)
{
	struct karma_evt_data *hw_data = &(hw->hw_data);

	karma_delete_events(hw_data);

	hw->count.acp_hit_cnt = 0;
	hw->count.acp_miss_cnt = 0;
	hw->count.rep_gd_cnt = 0;
	hw->count.dcp_gd_cnt = 0;
	hw->count.rep_gen_cnt = 0;
	hw->count.dcp_gen_cnt = 0;
}

static void dsu_stop_hwmon(struct dsu_hwmon *hw)
{
	struct dsu_evt_data *hw_data = &(hw->hw_data);

	dsu_delete_events(hw_data);
	hw->count.l3d_cnt = 0;
	hw->count.l3d_refill_cnt = 0;
	hw->count.hzd_addr_cnt = 0;
	hw->count.cycle_cnt = 0;
}

#ifdef L3P_KARMA_CPU_HWMON
static struct perf_event_attr *cpu_alloc_attr(void)
{
	struct perf_event_attr *attr;

	attr = kzalloc(sizeof(struct perf_event_attr), GFP_KERNEL);
	if (!attr)
		return attr;

	attr->type = PERF_TYPE_RAW;
	attr->size = sizeof(struct perf_event_attr);
	attr->pinned = 1;
	attr->exclude_idle = 1;

	return attr;
}
#endif

static struct perf_event_attr *karma_alloc_attr(void)
{
	struct perf_event_attr *attr;

	attr = kzalloc(sizeof(struct perf_event_attr), GFP_KERNEL);
	if (!attr)
		return attr;

	attr->type = PERF_TYPE_KARMA;
	attr->size = sizeof(struct perf_event_attr);
	attr->pinned = 1;

	return attr;
}

static struct perf_event_attr *dsu_alloc_attr(void)
{
	struct perf_event_attr *attr;

	attr = kzalloc(sizeof(struct perf_event_attr), GFP_KERNEL);
	if (!attr)
		return attr;

	attr->type = PERF_TYPE_DSU;
	attr->size = sizeof(struct perf_event_attr);
	attr->pinned = 1;

	return attr;
}

#ifdef L3P_KARMA_CPU_HWMON
static int cpu_set_events(struct cpu_grp_info *cpu_grp, int cpu)
{
	struct perf_event *pevent;
	struct perf_event_attr *attr;
	int err;
	unsigned int i = 0, j = 0;
	struct cpu_evt_data *evtdata = to_evtdata(cpu_grp, cpu);

	/* Allocate an attribute for event initialization */
	attr = cpu_alloc_attr();
	if (!attr) {
		pr_info("cpu alloc attr failed\n");
		return -ENOMEM;
	}

	for (i = 0; i < ARRAY_SIZE(evtdata->events); i++) {
		attr->config = cpu_grp->event_ids[i];
		pevent = perf_event_create_kernel_counter(attr, cpu, NULL,
							  NULL, NULL);
		if (IS_ERR(pevent))
			goto err_out;
		evtdata->events[i].pevent = pevent;
		perf_event_enable(pevent);
	}

	kfree(attr);
	attr = NULL;
	return 0;

err_out:
	pr_err("l3p_karma: fail to create %d events\n", i);
	for (j = 0; j < i; j++) {
		perf_event_disable(evtdata->events[j].pevent);
		perf_event_release_kernel(evtdata->events[j].pevent);
		evtdata->events[j].pevent = NULL;
	}
	err = PTR_ERR(pevent);
	kfree(attr);
	attr = NULL;
	return err;
}
#endif

static int karma_set_events(struct karma_hwmon *hw, int cpu)
{
	struct perf_event *pevent= NULL;
	struct perf_event_attr *attr = NULL;
	struct karma_evt_data *hw_data = &hw->hw_data;
	unsigned int i = 0, j = 0;
	int err;

	/* Allocate an attribute for event initialization */
	attr = karma_alloc_attr();
	if (!attr){
		pr_info("karma alloc attr failed\n");
		return -ENOMEM;
	}

	for (i = 0; i < ARRAY_SIZE(hw_data->events); i++) {
		attr->config = hw->event_ids[i];
		pevent = perf_event_create_kernel_counter(attr, cpu, NULL,
							  NULL, NULL);
		if (IS_ERR(pevent))
			goto err_out;
		hw_data->events[i].pevent = pevent;
		perf_event_enable(pevent);
	}

	kfree(attr);
	attr = NULL;
	return 0;

err_out:
	pr_err("[%s] fail to create %d events\n", __func__, i);
	for (j = 0; j < i; j++) {
		perf_event_disable(hw_data->events[j].pevent);
		perf_event_release_kernel(hw_data->events[j].pevent);
		hw_data->events[j].pevent = NULL;
	}
	err = PTR_ERR(pevent);
	kfree(attr);
	attr = NULL;
	return err;
}

static int dsu_set_events(struct dsu_hwmon *hw, int cpu)
{
	struct perf_event *pevent= NULL;
	struct perf_event_attr *attr = NULL;
	struct dsu_evt_data *hw_data = &hw->hw_data;
	unsigned int i = 0, j = 0;
	int err;

	/* Allocate an attribute for event initialization */
	attr = dsu_alloc_attr();
	if (!attr) {
		pr_info("dsu alloc attr failed\n");
		return -ENOMEM;
	}

	for (i = 0; i < ARRAY_SIZE(hw_data->events); i++) {
		attr->config = hw->event_ids[i];
		pevent = perf_event_create_kernel_counter(attr, cpu, NULL,
							  NULL, NULL);
		if (IS_ERR(pevent))
			goto err_out;
		hw_data->events[i].pevent = pevent;
		perf_event_enable(pevent);
	}

	kfree(attr);
	attr = NULL;
	return 0;

err_out:
	pr_err("[%s] fail to create %d events\n", __func__, i);
	for (j = 0; j < i; j++) {
		perf_event_disable(hw_data->events[j].pevent);
		perf_event_release_kernel(hw_data->events[j].pevent);
		hw_data->events[j].pevent = NULL;
	}
	err = PTR_ERR(pevent);
	kfree(attr);
	attr = NULL;
	return err;
}

#ifdef L3P_KARMA_CPU_HWMON
static int cpu_start_hwmon(struct cpu_hwmon *hw)
{
	int cpu, ret = 0;
	struct cpu_grp_info *cpu_grp = hw->cpu_grp;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
	register_cpu_notifier(&cpu_grp->perf_cpu_notif);
#else
	cpuhp_setup_state_nocalls(CPUHP_AP_HISI_L3P_KARMA_NOTIFY_PREPARE, "l3p_karma:online",
				  l3p_karma_cpu_online, NULL);
#endif

	get_online_cpus();
	for_each_cpu(cpu, &cpu_grp->cpus) {
		ret = cpu_set_events(cpu_grp, cpu);
		if (ret) {
			if (!cpu_online(cpu)) {
				ret = 0;
			} else {
				pr_warn("Perf event init failed on CPU%d\n", cpu);
				break;
			}
		} else {
			cpumask_set_cpu(cpu, &cpu_grp->inited_cpus);
		}
	}
	mutex_lock(&list_lock);
	if (!cpumask_equal(&cpu_grp->cpus, &cpu_grp->inited_cpus)){
		list_add_tail(&cpu_grp->mon_list, &perf_mon_list);
	}
	mutex_unlock(&list_lock);
	put_online_cpus();

	return ret;
}
#endif

static int karma_start_hwmon(struct karma_hwmon *hw)
{
	int ret = 0;
	int cpu = 0;

	ret = karma_set_events(hw, cpu);
	if (ret) {
		pr_err("[%s]perf event init failed on CPU%d\n", __func__, cpu);
		WARN_ON_ONCE(1);
		return ret;
	}

	return ret;
}

static int dsu_start_hwmon(struct dsu_hwmon *hw)
{
	int ret = 0;
	/* cpu must be 0*/
	int cpu = 0;

	ret = dsu_set_events(hw, cpu);
	if (ret) {
		pr_err("[%s]perf event init failed on CPU%d\n", __func__, cpu);
		WARN_ON_ONCE(1);
		return ret;
	}

	return ret;
}

#ifdef L3P_KARMA_CPU_HWMON
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
static int l3p_karma_cpu_callback(struct notifier_block *nb,
		unsigned long action, void *hcpu)
{
	unsigned long cpu = (unsigned long)hcpu;
	struct cpu_grp_info *cpu_grp, *tmp;

	if (action != CPU_ONLINE)
		return NOTIFY_OK;

	mutex_lock(&list_lock);
	list_for_each_entry_safe(cpu_grp, tmp, &perf_mon_list, mon_list) {
		if (!cpumask_test_cpu(cpu, &cpu_grp->cpus) ||
			cpumask_test_cpu(cpu, &cpu_grp->inited_cpus))
			continue;

		if (cpu_set_events(cpu_grp, cpu))
			pr_warn("Failed to create perf ev for CPU%lu\n", cpu);
		else
			cpumask_set_cpu(cpu, &cpu_grp->inited_cpus);

		if (cpumask_equal(&cpu_grp->cpus, &cpu_grp->inited_cpus)){
			list_del(&cpu_grp->mon_list);
			}
	}
	mutex_unlock(&list_lock);

	return NOTIFY_OK;
}
#else
static int __ref l3p_karma_cpu_online(unsigned int cpu)
{
	struct cpu_grp_info *cpu_grp, *tmp;

	mutex_lock(&list_lock);
	list_for_each_entry_safe(cpu_grp, tmp, &perf_mon_list, mon_list) {
		if (!cpumask_test_cpu(cpu, &cpu_grp->cpus) ||
			cpumask_test_cpu(cpu, &cpu_grp->inited_cpus))
			continue;

		if (cpu_set_events(cpu_grp, cpu))
			pr_warn("Failed to create perf ev for CPU%lu\n", cpu);
		else
			cpumask_set_cpu(cpu, &cpu_grp->inited_cpus);

		if (cpumask_equal(&cpu_grp->cpus, &cpu_grp->inited_cpus)){
			list_del(&cpu_grp->mon_list);
			}
	}
	mutex_unlock(&list_lock);
	return 0;
}
#endif

static int cpu_hwmon_setup(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct l3p_karma *l3p = platform_get_drvdata(pdev);
	struct cpu_hwmon *hw = NULL;
	struct cpu_grp_info *cpu_grp = NULL;
	int ret = 0;

	hw = devm_kzalloc(dev, sizeof(*hw), GFP_KERNEL);
	if (IS_ERR_OR_NULL(hw))
		return -ENOMEM;
	l3p->cpu_hw = hw;

	cpu_grp = devm_kzalloc(dev, sizeof(*cpu_grp), GFP_KERNEL);
	if (IS_ERR_OR_NULL(cpu_grp))
		return -ENOMEM;
	hw->cpu_grp = cpu_grp;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
	cpu_grp->perf_cpu_notif.notifier_call = l3p_karma_cpu_callback;
#endif

	cpumask_copy(&cpu_grp->cpus, cpu_online_mask);
	cpumask_copy(&hw->cpus, &cpu_grp->cpus);

	hw->num_cores = cpumask_weight(&cpu_grp->cpus);
	hw->evtcnts = devm_kzalloc(dev, hw->num_cores *
				sizeof(*(hw->evtcnts)), GFP_KERNEL);
	if (IS_ERR_OR_NULL(hw->evtcnts))
		return -ENOMEM;

	cpu_grp->evtdata = devm_kzalloc(dev, hw->num_cores *
		sizeof(*(cpu_grp->evtdata)), GFP_KERNEL);
	if (IS_ERR_OR_NULL(cpu_grp->evtdata))
		return -ENOMEM;

	cpu_grp->event_ids[L3D_REFILL_IDX] = L3D_REFILL_EV;
	cpu_grp->event_ids[L3D_IDX] = L3D_EV;
	cpu_grp->event_ids[INST_IDX] = INST_EV;
	cpu_grp->event_ids[CYC_IDX] = CYC_EV;

	hw->start_hwmon = &cpu_start_hwmon;
	hw->stop_hwmon = &cpu_stop_hwmon;
	hw->get_cnt = &cpu_get_cnt;

	return ret;
}
#endif

static int karma_hwmon_setup(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct l3p_karma *l3p = platform_get_drvdata(pdev);
	struct karma_hwmon *hw = NULL;
	int ret = 0;

	hw = devm_kzalloc(dev, sizeof(*hw), GFP_KERNEL);
	if (IS_ERR_OR_NULL(hw)){
		return -ENOMEM;
	}

	l3p->karma_hw = hw;

	hw->start_hwmon = &karma_start_hwmon;
	hw->stop_hwmon = &karma_stop_hwmon;
	hw->get_cnt = &karma_get_cnt;

	hw->event_ids[ACP_HIT_IDX] = ACP_HIT_EV;
	hw->event_ids[ACP_MISS_IDX] = ACP_MISS_EV;
	hw->event_ids[REP_GOOD_IDX] = REP_GOOD_EV;
	hw->event_ids[DCP_GOOD_IDX] = DCP_GOOD_EV;
	hw->event_ids[REP_GEN_IDX] = REP_GEN_EV;
	hw->event_ids[DCP_GEN_IDX] = DCP_GEN_EV;

	return ret;
}


static int dsu_hwmon_setup(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct l3p_karma *l3p = platform_get_drvdata(pdev);
	struct dsu_hwmon *hw = NULL;
	int ret = 0;

	hw = devm_kzalloc(dev, sizeof(*hw), GFP_KERNEL);
	if (IS_ERR_OR_NULL(hw)){
		return -ENOMEM;
	}

	l3p->dsu_hw = hw;

	hw->start_hwmon = &dsu_start_hwmon;
	hw->stop_hwmon = &dsu_stop_hwmon;
	hw->get_cnt = &dsu_get_cnt;

	hw->event_ids[DSU_L3D_CACHE_IDX] = DSU_L3D_CACHE_EV;
	hw->event_ids[DSU_L3D_REFILL_IDX] = DSU_L3D_REFILL_EV;
	hw->event_ids[DSU_HZD_ADDR_IDX] = DSU_HZD_ADDR_EV;
	hw->event_ids[DSU_CYCLE_IDX] = DSU_CYCLE_EV;

	return ret;
}

static int l3p_karma_target_pre(struct l3p_karma *l3p, unsigned long *freq)
{
	int ret = 0;

	if (!mutex_trylock(&l3p->mon_mutex_lock)) {
		return -EPERM;
	}

	/*
	* if devfreq monitor was stoped, by calling update_devfreq through min_freq/max_freq,
	* it will enable karma
	*/
	if (!l3p->mon_started) {
		mutex_unlock(&l3p->mon_mutex_lock); /*lint !e455*/
		return -EPERM;
	}
	mutex_unlock(&l3p->mon_mutex_lock); /*lint !e455*/

	if(*freq > l3p->freq_max || *freq < l3p->freq_min) {
		pr_err("l3p_karma: freq %lu is invalid\n", *freq);
		return -EINVAL;
	}

	if (*freq == l3p->cur_freq) {
		return -EPERM;
	}

	return ret;
}

static int l3p_karma_target(struct device *dev,
				    unsigned long *freq, u32 flags)
{
	struct l3p_karma *l3p = dev_get_drvdata(dev);
	struct set_reg *opp_entry = NULL;
	u8 opp_count = 0;
	u32 offset = 0, shift = 0, width = 0;
	u32 value = 0, data = 0;
	int ret = 0;
	int i = 0;

	ret = l3p_karma_target_pre(l3p, freq);
	if(ret) {
		return 0;
	}

	spin_lock(&l3p->data_spinlock);
	if(true == l3p->fcm_idle) {
		dev_err(dev, "do not update cur_freq before fcm_idle changed to false\n");
		goto err;
	}

	l3p_karma_disable(l3p);
	if(*freq == l3p->freq_min) {
		dev_err(dev, "freq %lu is not expected\n", *freq);
		WARN_ON(1);
		goto out;
	}

	opp_entry = l3p->profile_table[*freq].opp_entry;
	opp_count = l3p->profile_table[*freq].opp_count;

	for(i = 0; i < opp_count; i++) {
		offset = opp_entry[i].offset;
		shift = opp_entry[i].shift;
		width = opp_entry[i].width;
		value = opp_entry[i].value;
		if(value > (WIDTH_TO_MASK(width))) {
			dev_err(dev, "value 0x%x is not valid\n", value);
			ret = -EINVAL;
			goto err;
		} else if(offset > KARMA_RW_OFFSET_MAX || 0 != (offset % 4)) {
			dev_err(dev, "offset 0x%x is not valid\n", offset);
			ret = -EINVAL;
			goto err;
		}

		data = readl(l3p->base + offset);
		data &= (~(WIDTH_TO_MASK(width) << shift));
		data |= value << shift;
#ifdef DVM_SYNC_CLEAR_FIX
		/*
		 * DVM sync clear disable always
		 */
		if(KARMA_REP_CONTROL_OFFSET == offset &&
			0 == ((value & BIT(DVM_SYNC_CLEAR)) >> DVM_SYNC_CLEAR)) {
			value |= BIT(DVM_SYNC_CLEAR);
			WARN_ON(1);
		}
#endif
		writel(data, l3p->base + offset);
	}
	/* at last enable karma if it is enabled just now */
	l3p_karma_enable(l3p);

	trace_l3p_karma_target(*freq);

	if(l3p->quick_sleep_enable && ENABLE_TEMP_MODE == *freq) {
		queue_delayed_work(l3p->update_wq, &l3p->work,
					msecs_to_jiffies(l3p->work_ms));
	}

out:
	l3p->cur_freq = *freq;
err:
	spin_unlock(&l3p->data_spinlock);
	return ret;
}

static int l3p_df_get_dev_status(struct device *dev,
					    struct devfreq_dev_status *stat)
{
	return 0;
}

/* check whether need to enter enable temp from sleep */
static unsigned long check_from_slp_to_en_temp(
    struct l3p_karma *l3p, struct alg_type *alg)
{
	struct threshold_data *threshold = l3p->threshold_table[SLEEP_MODE];
	unsigned int dsu_hit_rate =alg->dsu_hit_rate;
	unsigned int dsu_bw = alg->dsu_bw;
	unsigned int rep_eff = alg->rep_eff;
	unsigned int dcp_eff = alg->dcp_eff;
	unsigned int tot_eff = alg->tot_eff;
	unsigned int acp_hit_rate = alg->acp_hit_rate;
	unsigned int dsuhit = 0, bw = 0, rep = 0, dcp = 0, tot = 0;
	unsigned int rep_dcp = 0, bw_hit_1 =0;
	unsigned int bw_hit_2 = 0, dsuhit_bw = 0;
	unsigned int acphit = 0, acphit_dsuhit = 0, rep_bw = 0;
	unsigned int keep_sleep = 0;

	dsuhit = dsu_hit_rate > threshold[DSU_HIT_HI].data[0];

	bw = dsu_bw > threshold[DSU_BW_HI].data[0];

	rep = rep_eff < threshold[REP_EFF_HI].data[0];

	dcp = dcp_eff < threshold[DCP_EFF_HI].data[0];

	tot = tot_eff < threshold[TOT_EFF_LW].data[0];

	rep_dcp = rep_eff < threshold[REP_EFF_LW].data[0]
				&& dcp_eff < threshold[DCP_EFF_LW].data[0];

	dsuhit_bw = dsu_hit_rate > threshold[DSU_HIT_LW].data[0]
				|| dsu_bw > threshold[DSU_BW_MD].data[0];

	bw_hit_1 = dsu_bw > threshold[DSU_BW_MD].data[0]
				&& dsu_hit_rate > threshold[DSU_HIT_MD].data[0];

	bw_hit_2 = dsu_bw > threshold[DSU_BW_LW].data[0]
				&& dsu_hit_rate > threshold[DSU_HIT_MD].data[1];

	acphit = acp_hit_rate > threshold[ACP_HIT_HI].data[0];

	acphit_dsuhit = acp_hit_rate > threshold[ACP_HIT_LW].data[0]
				&& dsu_hit_rate > threshold[DSU_HIT_MD].data[2];

	rep_bw = rep_eff > threshold[REP_EFF_HI].data[1]
				&& dsu_bw > threshold[DSU_BW_MD].data[0];

	keep_sleep = bw || (rep && dcp &&
				((rep_dcp && dsuhit_bw) || tot || dsuhit  || bw_hit_1 ||
				bw_hit_2 ||acphit || acphit_dsuhit || rep_bw));

	trace_check_from_slp_to_en_temp(keep_sleep);

	return !keep_sleep;
}


static unsigned long decide_from_en_temp(
    struct l3p_karma *l3p, struct alg_type *alg)
{
	struct threshold_data *threshold = l3p->threshold_table[ENABLE_TEMP_MODE];
	unsigned int dsu_bw = alg->dsu_bw;
	unsigned int rep_eff = alg->rep_eff;
	unsigned int dsu_hit_rate = alg->dsu_hit_rate;
	unsigned int hzd_rate = alg->hzd_rate;
	unsigned int acp_hit_rate = alg->acp_hit_rate;
	unsigned int acphit =0, acphit_dsuhit = 0, hzd_acphit = 0;
	unsigned int rep_bw = 0, bw_dsuhit_acphit = 0;
	unsigned int quick_sleep = 1;

	acphit = acp_hit_rate > threshold[ACP_HIT_HI].data[0];

	acphit_dsuhit = acp_hit_rate > threshold[ACP_HIT_LW].data[0]
				&& dsu_hit_rate > threshold[DSU_HIT_MD].data[0];

	hzd_acphit = hzd_rate > threshold[HZD_MD].data[0]
				&& acp_hit_rate > threshold[ACP_HIT_MD].data[0];

	rep_bw = rep_eff >  threshold[REP_EFF_HI].data[0]
				&& dsu_bw > threshold[DSU_BW_MD].data[0];

	bw_dsuhit_acphit = dsu_bw > threshold[DSU_BW_MD].data[0]
				&& dsu_hit_rate > threshold[DSU_HIT_MD].data[1]
				&& acp_hit_rate > threshold[ACP_HIT_MD].data[1];

	quick_sleep = acphit || acphit_dsuhit || hzd_acphit ||
				rep_bw || bw_dsuhit_acphit;

	trace_decide_from_en_temp(quick_sleep);

	return quick_sleep;
}

static unsigned long check_from_en_to_slp(
    struct l3p_karma *l3p, struct alg_type *alg)
{
	struct threshold_data *threshold = l3p->threshold_table[ENABLE_MODE];
	unsigned int dsu_hit_rate = alg->dsu_hit_rate;
	unsigned int dsu_bw = alg->dsu_bw;
	unsigned int rep_eff = alg->rep_eff;
	unsigned int dcp_eff = alg->dcp_eff;
	unsigned int tot_eff = alg->tot_eff;
	unsigned int hzd_rate = alg->hzd_rate;
	unsigned int acp_hit_rate = alg->acp_hit_rate;
	unsigned int bw = 0, rep = 0, dcp = 0, hzd = 0;
	unsigned int acphit = 0, tot = 0, rep_bw = 0;
	unsigned int hzd_tot = 0, rep_dcp = 0, acphit_hzd = 0;
	unsigned int hzd_acphit = 0, bw_dsuhit_acphit = 0;
	unsigned int en_to_slp = 0;

	bw = dsu_bw > threshold[DSU_BW_HI].data[0];

	rep = rep_eff < threshold[REP_EFF_HI].data[0];

	dcp = dcp_eff < threshold[DCP_EFF_HI].data[0];

	hzd = hzd_rate < threshold[HZD_LW].data[0];

	acphit = acp_hit_rate > threshold[ACP_HIT_HI].data[0];

	tot = tot_eff < threshold[TOT_EFF_LW].data[0];

	rep_dcp = rep_eff < threshold[REP_EFF_LW].data[0]
				&& dcp_eff < threshold[DCP_EFF_LW].data[0];

	acphit_hzd = acp_hit_rate < threshold[ACP_HIT_LW].data[0]
				&& hzd_rate > threshold[HZD_MD].data[0]
				&& dsu_bw < threshold[DSU_BW_MD].data[0];

	hzd_tot = hzd_rate > threshold[HZD_HI].data[0]
				&& tot_eff < threshold[TOT_EFF_HI].data[0];

	hzd_acphit = hzd_rate > threshold[HZD_MD].data[1]
				&& acp_hit_rate > threshold[ACP_HIT_MD].data[0];

	rep_bw = rep_eff > threshold[REP_EFF_HI].data[1]
				&& dsu_bw > threshold[DSU_BW_MD].data[0];

	bw_dsuhit_acphit = dsu_bw > threshold[DSU_BW_MD].data[0]
				&& dsu_hit_rate > threshold[DSU_HIT_MD].data[0]
				&& acp_hit_rate > threshold[ACP_HIT_MD].data[1];

	en_to_slp = bw || (rep && dcp &&
			(hzd ||acphit ||tot ||
			((rep_dcp) && (!acphit_hzd)) ||
			hzd_tot || hzd_acphit || rep_bw ||
			bw_dsuhit_acphit));

	trace_check_from_en_to_slp(en_to_slp);

	return en_to_slp;
}

#define MAX_DSU_BW	0xFFFFFFFF
static void l3p_df_algo_factors_calc(struct l3p_karma *l3p, struct alg_type *alg)
{
	struct karma_hwmon *karma_hw = l3p->karma_hw;
	struct karma_evt_count *kam_cnt = &karma_hw->count;
	struct dsu_hwmon *dsu_hw = l3p->dsu_hw;
	struct dsu_evt_count *dsu_cnt = &dsu_hw->count;

	if (dsu_cnt->l3d_cnt) {
		alg->dsu_hit_rate = 100 - (dsu_cnt->l3d_refill_cnt * 100 /dsu_cnt->l3d_cnt);
	}

	if (dsu_cnt->l3d_cnt < (kam_cnt->acp_hit_cnt + kam_cnt->acp_miss_cnt)) {
		alg->dsu_bw = MAX_DSU_BW;
	} else if (dsu_cnt->cycle_cnt) {
		alg->dsu_bw = (dsu_cnt->l3d_cnt - kam_cnt->acp_hit_cnt - kam_cnt->acp_miss_cnt)  * 1000
					/dsu_cnt->cycle_cnt;
	}

	if(kam_cnt->rep_gen_cnt) {
		alg->rep_eff = kam_cnt->rep_gd_cnt * 100 /kam_cnt->rep_gen_cnt;
	}

	if(kam_cnt->dcp_gen_cnt) {
		alg->dcp_eff = kam_cnt->dcp_gd_cnt * 100
					/kam_cnt->dcp_gen_cnt;
	}

	if(kam_cnt->rep_gen_cnt + kam_cnt ->dcp_gen_cnt) {
		alg->tot_eff = (kam_cnt->rep_gd_cnt + kam_cnt->dcp_gd_cnt) * 100
					/ (kam_cnt->rep_gen_cnt + kam_cnt->dcp_gen_cnt);
	}

	if(dsu_cnt->l3d_cnt) {
		alg->hzd_rate = dsu_cnt->hzd_addr_cnt * 100
					/dsu_cnt->l3d_cnt;
	}

	if(kam_cnt->acp_hit_cnt + kam_cnt->acp_miss_cnt) {
		alg->acp_hit_rate = kam_cnt->acp_hit_cnt * 100
					/ (kam_cnt->acp_hit_cnt + kam_cnt->acp_miss_cnt);
	}

	trace_l3p_karma_algo_info(alg->dsu_hit_rate, alg->dsu_bw, alg->rep_eff,
						alg->dcp_eff, alg->tot_eff, alg->hzd_rate, alg->acp_hit_rate);

}

static unsigned long l3p_df_calc_next_freq(struct l3p_karma *l3p)
{
	struct alg_type alg;
	unsigned long ret = 0;
	unsigned long freq  = l3p->cur_freq;

	memset(&alg, 0, sizeof(alg)); /* unsafe_function_ignore: memset */

	l3p_df_algo_factors_calc(l3p, &alg);

	switch (freq) {
		case SLEEP_MODE:
			/*
			* return 0, keep sleep;
			* return 1, sleep -> enable_temp
			*/
			ret = check_from_slp_to_en_temp(l3p, &alg);
			if(ret) {
				freq = ENABLE_TEMP_MODE;
			}
			break;
		case ENABLE_TEMP_MODE:
			/*
			* return 0, enanle_temp ->enable;
			* return 1, enable_temp -> sleep
			*/
			ret = decide_from_en_temp(l3p, &alg);
			if(ret) {
				freq = SLEEP_MODE;
			} else {
				freq = ENABLE_MODE;
			}
			break;
		case ENABLE_MODE:
			/*
			* return 0, keep enable;
			* return 1, enable -> sleep
			*/
			ret = check_from_en_to_slp(l3p, &alg);
			if(ret) {
				freq = SLEEP_MODE;
			}
			break;
		case DISABLE_MODE:
			freq = SLEEP_MODE;
			break;
		default:
			pr_err("l3p_karma: profile mode is not valid, freq = %lu\n", freq);
			break;
	}

	trace_l3p_karma_freq_trans(l3p->cur_freq, freq);
	return freq;
}


static int l3p_gov_get_target(struct devfreq *df,
						  unsigned long *freq)
{
	struct l3p_karma *l3p = dev_get_drvdata(df->dev.parent);
#ifdef L3P_KARMA_CPU_HWMON
	struct cpu_hwmon *cpu_hw = l3p->cpu_hw;
	struct cpu_evt_count *evtcnts = cpu_hw->evtcnts;
#endif
	struct karma_hwmon *karma_hw = l3p->karma_hw;
	//struct karma_evt_count *kam_cnt = &karma_hw->count;
	struct dsu_hwmon *dsu_hw = l3p->dsu_hw;
	//struct dsu_evt_count *dsu_cnt = &dsu_hw->count;
	unsigned int const usec = ktime_to_us(ktime_get());
	unsigned int delta = 0;
	int err = 0;

	err = devfreq_update_stats(df);
	if (err)
		return err;

	delta = usec - l3p->time.last_update;
	l3p->time.last_update = usec;
	l3p->time.usec_delta = delta;

	if (!mutex_trylock(&l3p->mon_mutex_lock)) {
		return 0;
	}

	if (!l3p->mon_started) {
		mutex_unlock(&l3p->mon_mutex_lock); /*lint !e455*/
		*freq = SLEEP_MODE;
		return 0;
	}

#ifdef L3P_KARMA_CPU_HWMON
	cpu_get_cnt(cpu_hw);
#endif
	karma_get_cnt(karma_hw);
	dsu_get_cnt(dsu_hw);
	mutex_unlock(&l3p->mon_mutex_lock); /*lint !e455*/

#ifdef L3P_KARMA_CPU_HWMON
	trace_printk("inst_cnt=%lu, cyc_cnt=%lu\n", evtcnts->inst_cnt, evtcnts->cyc_cnt);
#endif

/*
	trace_printk("l3d_cnt=%lu, l3d_refill_cnt=%lu, hzd_addr_cnt=%lu, cycle_cnt=%lu, delta=%u\n",
			dsu_cnt->l3d_cnt, dsu_cnt->l3d_refill_cnt, dsu_cnt->hzd_addr_cnt, dsu_cnt->cycle_cnt, delta);

	trace_printk("rep_gd_cnt = %lu, rep_gen_cnt = %lu\n", kam_cnt->rep_gd_cnt,  kam_cnt->rep_gen_cnt);
	trace_printk("dcp_gd_cnt = %lu, dcp_gen_cnt = %lu\n", kam_cnt->dcp_gd_cnt,  kam_cnt->dcp_gen_cnt);
	trace_printk("acp_hit_cnt = %lu, acp_miss_cnt = %lu\n", kam_cnt->acp_hit_cnt, kam_cnt->acp_miss_cnt);
*/

	*freq = l3p_df_calc_next_freq(l3p);

	if (*freq < l3p->freq_min) {
		*freq = l3p->freq_min;
	} else if (*freq > l3p->freq_max) {
		*freq = l3p->freq_max;
	}

	return 0;
}

static int l3p_gov_start(struct devfreq *df)
{
	struct l3p_karma *l3p = dev_get_drvdata(df->dev.parent);
	int ret = 0;

	mutex_lock(&l3p->mon_mutex_lock);
	l3p->monitor_enable = 1;
	ret = l3p_start_monitor(df);
	if (ret) {
		l3p->monitor_enable = 0;
		goto err_start;
	}

err_start:
	mutex_unlock(&l3p->mon_mutex_lock);
	return ret;
}

static void l3p_gov_stop(struct devfreq *df)
{
	struct l3p_karma *l3p = df->data;

	mutex_lock(&l3p->mon_mutex_lock);
	l3p_stop_monitor(df);
	l3p->monitor_enable = 0;
	mutex_unlock(&l3p->mon_mutex_lock);
}

#define MIN_MS	10U
#define MAX_MS	1000U
static int l3p_gov_ev_handler(struct devfreq *devfreq,
					    unsigned int event, void *data)
{
	int ret = 0;
	unsigned int sample_ms;

	switch (event) {
	case DEVFREQ_GOV_START:
		ret = l3p_gov_start(devfreq);
		if(ret)
			return ret;
		dev_dbg(devfreq->dev.parent,
			"Started L3 prefetch governor\n");
		break;

	case DEVFREQ_GOV_STOP:
		l3p_gov_stop(devfreq);
		dev_dbg(devfreq->dev.parent,
			"Stoped L3 prefetch governor\n");
		break;

	case DEVFREQ_GOV_SUSPEND:
		devfreq_monitor_suspend(devfreq);
		break;

	case DEVFREQ_GOV_RESUME:
		devfreq_monitor_resume(devfreq);
		break;

	case DEVFREQ_GOV_INTERVAL:
		sample_ms = *(unsigned int *)data;
		sample_ms = max(MIN_MS, sample_ms);
		sample_ms = min(MAX_MS, sample_ms);

		mutex_lock(&devfreq->lock);
		devfreq->profile->polling_ms = sample_ms;
		dev_dbg(devfreq->dev.parent,
			"interval polling_ms=%u\n", devfreq->profile->polling_ms);
		mutex_unlock(&devfreq->lock);
		break;
	}

	return ret;
}

static struct devfreq_governor l3p_karma_governor = {
	.name		 = L3P_KARMA_GOVERNOR_NAME,
	.immutable = 1,
	.get_target_freq = l3p_gov_get_target,
	.event_handler	 = l3p_gov_ev_handler,
};

static void l3p_karma_handle_update(struct work_struct *work)
{
	struct l3p_karma *l3p = container_of(work, struct l3p_karma,
					     work.work);

	mutex_lock(&l3p->devfreq->lock);
	(void)update_devfreq(l3p->devfreq);
	mutex_unlock(&l3p->devfreq->lock);
}

static int l3p_init_update_time(struct device *dev)
{
	struct l3p_karma *l3p = dev_get_drvdata(dev);

	/* Clean the time statistics and start from scrach */
	l3p->time.usec_delta = 0;
	l3p->time.last_update = ktime_to_us(ktime_get());

	return 0;

}

static int l3p_setup_devfreq_profile(struct platform_device *pdev)
{
	struct l3p_karma *l3p = platform_get_drvdata(pdev);
	struct devfreq_dev_profile *df_profile;

	l3p->devfreq_profile = devm_kzalloc(&pdev->dev,
			sizeof(struct devfreq_dev_profile), GFP_KERNEL);
	if (IS_ERR_OR_NULL(l3p->devfreq_profile)) {
		dev_err(&pdev->dev, "no memory.\n");
		return PTR_ERR(l3p->devfreq_profile);
	}

	df_profile = l3p->devfreq_profile;

	df_profile->target = l3p_karma_target;
	df_profile->get_dev_status = l3p_df_get_dev_status;
	df_profile->freq_table = l3p->freq_table;
	df_profile->max_state = l3p->table_len;
	df_profile->polling_ms = l3p->polling_ms;
	//df_profile->polling_ms = 1000;
	df_profile->initial_freq = l3p->initial_freq;

	return 0;
}

static int l3p_parse_dt_profile(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct l3p_karma *l3p = platform_get_drvdata(pdev);
	struct device_node *node = pdev->dev.of_node;
	struct device_node *parent = NULL;
	struct device_node *profile = NULL;
	struct device_node *opp = NULL;
	struct profile_data *profile_table = NULL;
	struct set_reg *opp_entry = NULL;
	int table_len = 0;
	int opp_count = 0;
	u32 mask = 0;
	int ret = 0;

	of_node_get(node);

	parent = of_get_child_by_name(node, "profile");
	if (!parent) {
		dev_err(dev, "Failed to find profile node\n");
		ret = -EINVAL;
		goto err_out;
	}

	table_len = of_get_child_count(parent);
	if (!table_len) {
		dev_err(dev, "no any profile child node\n");
		ret = -EINVAL;
		goto err_out;
	}
	dev_info(dev, "table_len = %d\n", table_len);

	profile_table = devm_kzalloc(dev, sizeof(struct profile_data) * table_len, GFP_KERNEL);
	if (IS_ERR_OR_NULL(profile_table)) {
		dev_err(dev, "Failed to allocate for karma profile_table\n");
		ret = -ENOMEM;
		goto err_out;
	}
	l3p->profile_table = profile_table;
	l3p->table_len = table_len;

	for_each_child_of_node(parent, profile) {
		opp_count = of_get_child_count(profile);
		if (!opp_count) {
			dev_err(dev, "profile name:%s has no any opp child\n", profile->name);
			ret = -EINVAL;
			goto err_out;
		}
		dev_info(dev, "profile name:%s opp_count = %d\n", profile->name, opp_count);
		opp_entry = devm_kzalloc(dev, sizeof(struct set_reg) * opp_count, GFP_KERNEL);
		if (IS_ERR_OR_NULL(opp_entry)) {
			dev_err(dev, "Failed to allocate for karma opp_entry\n");
			ret =  -ENOMEM;
			goto err_out;
		}
		profile_table->opp_entry = opp_entry;
		profile_table->opp_count = opp_count;

		for_each_child_of_node(profile, opp) {
			dev_info(dev, "opp name:%s\n", opp->name);
			ret = of_property_read_u32(opp, "offset", &(opp_entry->offset));
			if (ret) {
				dev_err(dev, "parse %s offset failed%d!\n", opp->name, ret);
				goto err_out;
			}
			dev_info(dev, "offset = 0x%x\n", opp_entry->offset);

			ret = of_property_read_u32(opp, "mask",  &mask);
			if (ret) {
				dev_err(dev, "parse %s mask failed%d!\n", opp->name, ret);
				goto err_out;
			}
			opp_entry->shift = ffs(mask) - 1;
			opp_entry->width = fls(mask) - ffs(mask) + 1;
			dev_info(dev, "shift = 0x%x, width = 0x%x\n", opp_entry->shift, opp_entry->width);

			ret = of_property_read_u32(opp, "value", (&(opp_entry->value)));
			if (ret) {
				dev_err(dev, "parse %s value failed%d!\n", opp->name, ret);
				goto err_out;
			}
			dev_info(dev, "value = 0x%x\n", opp_entry->value);

			opp_entry++;
		}
		profile_table++;
	}

err_out:
	of_node_put(node);
	return ret;
}

static int l3p_parse_dt_threshold(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct l3p_karma *l3p = platform_get_drvdata(pdev);
	struct device_node *node = pdev->dev.of_node;
	struct device_node *parent = NULL;
	struct device_node *state = NULL;
	struct threshold_data **threshold_table = NULL;
	int threshold_len = 0;
	int idx = 0, s_idx = 0, t_idx = 0;
	int i = 0, len = 0, ret = 0;
	u32 *data = NULL;

	of_node_get(node);
	parent = of_get_child_by_name(node, "threshold");
	if (!parent) {
		dev_err(dev, "Failed to find threshold node\n");
		ret= -EINVAL;
		goto err_out;
	}

	threshold_len = of_get_child_count(parent);
	if (!threshold_len) {
		dev_err(dev, "no any threshold child node\n");
		ret= -EINVAL;
		goto err_out;
	}
	dev_info(dev, "threshold_len = %d\n", threshold_len);

	threshold_table = devm_kzalloc(dev, threshold_len * sizeof(struct threshold_data *), GFP_KERNEL);
	if (IS_ERR_OR_NULL(threshold_table)) {
		dev_err(dev, "Failed to allocate for karma threshold_table\n");
		ret = -ENOMEM;
		goto err_out;
	}
	l3p->threshold_table = threshold_table;
	l3p->threshold_len = threshold_len;

	s_idx = 0;
	for_each_child_of_node(parent, state) {
		dev_err(dev, "state name = %s\n", state->name);
		/* if it is threshold of disable state, no need to alloc memory */
		if(0 == strcmp(state->name, "disable")) {
			dev_err(dev, "no threshold in disable state\n");
			s_idx++;
			continue;
		}
		threshold_table[s_idx] = devm_kzalloc(dev, MAX_THRESHOLD * sizeof(struct threshold_data), GFP_KERNEL);
		if (IS_ERR_OR_NULL(threshold_table[s_idx])) {
			dev_err(dev, "fail to allocate for threshold_table of per-state\n");
			ret = -ENOMEM;
			goto err_out;
		}
		idx = 0, t_idx = 0;
		while(idx < MAX_THRESHOLD) {
			if (of_find_property(state, threshold_name_set[idx], &len)) {
				len /= sizeof(*data); /*lint !e573*/
				data = devm_kzalloc(dev, len * sizeof(*data), GFP_KERNEL);
				if (IS_ERR_OR_NULL(data)) {
					dev_err(dev, "fail to allocate for data\n");
					ret = -ENOMEM;
					goto err_out;
				}
				threshold_table[s_idx][idx].data = data;

				ret = of_property_read_u32_array(state, threshold_name_set[idx],
								 data, len);
				if (ret) {
					dev_err(dev, "parse %s failed in %s\n", threshold_name_set[idx], state->name);
					goto err_out;
				}

				for (i = 0; i < len; i++) {
					dev_err(dev, "threshold name:%s, data[%d]=%d\n", threshold_name_set[idx],  i, data[i]);
					threshold_table[s_idx][idx].count = len;
					threshold_table[s_idx][idx].data[i] = data[i];
				}
			}
			idx++;
		}
		s_idx++;
	}

err_out:
	of_node_put(node);
	return ret;
}


static int l3p_parse_dt_base(struct platform_device *pdev)
{
	struct l3p_karma *l3p = platform_get_drvdata(pdev);
	struct device_node *node = pdev->dev.of_node;
	int ret = 0;

	of_node_get(node);

	ret = of_property_read_u32(node, "polling", &l3p->polling_ms);
	if (ret)
		l3p->polling_ms = L3P_KARMA_DEFAULT_POLLING_MS;
	dev_info(&pdev->dev, "polling_ms = %d\n", l3p->polling_ms);

	of_node_put(node);

	return 0;
}

#ifdef CONFIG_HISI_L3P_KARMA_DEBUG
 void l3p_karma_show_registers(struct l3p_karma *l3p)
{
	int i = 0;
	u32 value = 0;

	for(i = 0; i < KARMA_SYS_CTRL_NUM; i++) {
		value = readl(l3p->base + i * 4);
		pr_err("l3p_karma: offset = 0x%x, value = 0x%x\n", i * 4, value);
	}
}
#endif

static int l3p_karma_setup(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct l3p_karma *l3p = platform_get_drvdata(pdev);
	int i = 0;
	int ret = 0;

	l3p->base = devm_ioremap(dev, KARMA_CTRL_BASEADDR, KARMA_CTRL_SIZE);
	if(IS_ERR_OR_NULL(l3p->base)){
		dev_err(dev, "l3p karma ctrl baseaddr ioremap failed.\n");
		return -ENOMEM;
	}
	dev_info(dev, "l3p->base = %pK\n", l3p->base);

	l3p->attr_grp = &dev_attr_group;
	l3p->monitor_enable = 0;
	l3p->vote_mon_dis = 0;
	l3p->mon_started = false;
	mutex_init(&l3p->mon_mutex_lock);
	spin_lock_init(&l3p->data_spinlock);

	l3p->update_wq = create_freezable_workqueue("hisi_l3p_karma_wq");
	if (IS_ERR_OR_NULL(l3p->update_wq)) {
		dev_err(&pdev->dev, "cannot create workqueue.\n");
		ret = PTR_ERR(l3p->update_wq);
		goto out;
	}
	INIT_DEFERRABLE_WORK(&l3p->work, l3p_karma_handle_update);

	l3p->work_ms = 4; /* 4000us */
	l3p->quick_sleep_enable = 1;

	l3p->freq_table = devm_kcalloc(dev, l3p->table_len,
				       sizeof(*l3p->freq_table),
				       GFP_KERNEL);
	if (IS_ERR_OR_NULL(l3p->freq_table)) {
		dev_err(dev, "failed to alloc mem for freq_table.\n");
		goto er_wq;
	}

	for (i = 0; i < l3p->table_len; i++){
		l3p->freq_table[i] = i;
		dev_info(dev, "i = %d, freq_table = %lu\n", i, l3p->freq_table[i]);
	}

	l3p->freq_min = l3p->freq_table[0];
	l3p->freq_max = l3p->freq_table[l3p->table_len - 1];

	l3p->initial_freq = l3p->freq_max;
	l3p->cur_freq = l3p->initial_freq;

	dev_info(dev, "cur_freq = %lu\n",  l3p->cur_freq);

	ret = l3p_setup_devfreq_profile(pdev);
	if (ret) {
		dev_err(dev, "device setup devfreq profile failed.\n");
		goto er_wq;
	}

	l3p->devfreq = devm_devfreq_add_device(dev,
					       l3p->devfreq_profile,
					       L3P_KARMA_GOVERNOR_NAME, l3p);

	if (IS_ERR_OR_NULL(l3p->devfreq)) {
		dev_err(dev, "registering to devfreq failed.\n");
		ret = PTR_ERR(l3p->devfreq);
		goto er_wq;
	}

	l3p_init_update_time(dev);

	mutex_lock(&l3p->devfreq->lock);
	l3p->devfreq->min_freq = l3p->freq_min;
	l3p->devfreq->max_freq = l3p->freq_max;
	dev_info(dev, "min_freq = %lu, max_freq = %lu\n", l3p->devfreq->min_freq, l3p->devfreq->max_freq);
	mutex_unlock(&l3p->devfreq->lock);

	spin_lock(&l3p->data_spinlock);
	ret = l3p_karma_init(pdev);
	if(ret) {
		dev_err(dev, "karma device init failed\n");
		spin_unlock(&l3p->data_spinlock);
		goto err_devfreq;
	}
	l3p_karma_enable(l3p);
#ifdef CONFIG_HISI_L3P_KARMA_DEBUG
	l3p_karma_show_registers(l3p);
#endif
	spin_unlock(&l3p->data_spinlock);

	return 0;

err_devfreq:
	devm_devfreq_remove_device(dev, l3p->devfreq);
er_wq:
	/* Wait for pending work */
	flush_workqueue(l3p->update_wq);
	/* Destroy workqueue */
	destroy_workqueue(l3p->update_wq);
out:
	devm_iounmap(dev, l3p->base);
	return ret;
}

#ifdef CONFIG_HISI_MULTIDRV_CPUIDLE
extern bool hisi_fcm_cluster_pwrdn(void);
#else
static inline bool hisi_fcm_cluster_pwrdn(void) {return 0;}
#endif

static int l3p_fcm_idle_notif(struct notifier_block *nb, unsigned long action,
                                                        void *data)
{
	bool fcm_pwrdn = 0;
	struct l3p_karma *l3p = container_of(nb, struct l3p_karma, idle_nb);

	/*
	* device hasn't been initilzed yet,
	* mon_started is necessary, otherwise although monitor_enable is true,
	* but thread is sleep in l3p_start_monitor, so fcm can enter idle.
	* However, karma is not enabled, reset value is saved.
	*/
	if (!l3p->monitor_enable || !l3p->mon_started) {
		return NOTIFY_OK;
	}

	/* In fact, karma maybe not power off */
	switch (action) {
	case CPU_PM_ENTER:
			spin_lock(&l3p->data_spinlock);
			fcm_pwrdn = hisi_fcm_cluster_pwrdn();
			if (fcm_pwrdn) {
				l3p_save_reg_data(l3p);
				l3p->fcm_idle = true;
			}
			spin_unlock(&l3p->data_spinlock);
			break;

	case CPU_PM_ENTER_FAILED:
	case CPU_PM_EXIT:
			spin_lock(&l3p->data_spinlock);
			if(true == l3p->fcm_idle){
				l3p->fcm_idle = false;
				l3p_restore_reg_data(l3p);
			}
			spin_unlock(&l3p->data_spinlock);
			break;
	}

	return NOTIFY_OK;
}

static int l3p_karma_suspend(struct device *dev)
{
	struct l3p_karma *l3p = dev_get_drvdata(dev);
	struct devfreq *devfreq = l3p->devfreq;
	int ret = 0;

	mutex_lock(&l3p->mon_mutex_lock);
	/*save all karma contrl register*/
	spin_lock(&l3p->data_spinlock);
	if(l3p->monitor_enable) {
		l3p_save_reg_data(l3p);
	}
	spin_unlock(&l3p->data_spinlock);

	l3p_stop_monitor(devfreq);
	mutex_unlock(&l3p->mon_mutex_lock);

	return ret;
}

static int l3p_karma_resume(struct device *dev)
{

	struct l3p_karma *l3p = dev_get_drvdata(dev);
	struct devfreq *devfreq = l3p->devfreq;
	int ret = 0;

	l3p_init_update_time(dev);

	mutex_lock(&l3p->mon_mutex_lock);
	/*restore all karma contrl register*/
	spin_lock(&l3p->data_spinlock);
	if(l3p->monitor_enable) {
		l3p_restore_reg_data(l3p);
	}
	spin_unlock(&l3p->data_spinlock);

	ret = l3p_start_monitor(devfreq);
	if (ret) {
		l3p->monitor_enable = 0;
	}
	mutex_unlock(&l3p->mon_mutex_lock);

	return ret;
}

static SIMPLE_DEV_PM_OPS(l3p_karma_pm, l3p_karma_suspend, l3p_karma_resume);

static int l3p_karma_acp_callback(struct notifier_block *nb,
		unsigned long mode, void *data)
{
	struct l3p_karma *l3p = container_of(nb, struct l3p_karma,
					     acp_notify);
	struct devfreq *df = l3p->devfreq;
	int ret = 0;

	switch (mode) {
		case L3SHARE_ACP_ENABLE:
			mutex_lock(&l3p->mon_mutex_lock);
			if (1 == l3p->monitor_enable) {
				/* NOTICE
				* this thread can not sleep before save register data,
				* if get mon_mutex_lock already
				*/
				spin_lock(&l3p->data_spinlock);
				l3p_karma_disable(l3p);
				l3p_save_reg_data(l3p);
				spin_unlock(&l3p->data_spinlock);
				l3p_stop_monitor(df);
				l3p->monitor_enable = 0;
				trace_l3p_karma_acp_callback("stop monitor and disable karma", mode);
			}
			l3p->vote_mon_dis |= BIT(VOTE_SHARE);
			mutex_unlock(&l3p->mon_mutex_lock);
			break;
		case L3SHARE_ACP_DISABLE:
			mutex_lock(&l3p->mon_mutex_lock);
			l3p->vote_mon_dis &= (~BIT(VOTE_SHARE));
			if (0 == l3p->monitor_enable && 0 == l3p->vote_mon_dis) {
				trace_l3p_karma_acp_callback("start monitor and enable karma", mode);
				l3p->monitor_enable = 1;
				ret = l3p_start_monitor(df);
				if (ret) {
					/* handle error */
					l3p->monitor_enable = 0;
					//l3p_karma_disable(l3p);
					ret = -EINVAL;
				} else {
					/* enable karma */
					spin_lock(&l3p->data_spinlock);
					l3p_restore_reg_data(l3p);
					l3p_karma_enable(l3p);
					spin_unlock(&l3p->data_spinlock);
				}
			}
			mutex_unlock(&l3p->mon_mutex_lock);
			break;
		default:
			//pr_err("%s error case %lu\n", __func__, mode);
			break;
	}

	return NOTIFY_OK;
}

/*lint -e429*/
static int l3p_karma_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct l3p_karma *l3p;
	int ret = 0;

	dev_err(dev, "register l3p karma enter\n");

	l3p = devm_kzalloc(dev, sizeof(*l3p), GFP_KERNEL);
	if (!l3p)
		return -ENOMEM;

	platform_set_drvdata(pdev, l3p);
	l3p->pdev = pdev;

	ret = l3p_parse_dt_base(pdev);
	if (ret){
		dev_err(dev, "parse dt base failed\n");
		goto err;
	}

	ret = l3p_parse_dt_profile(pdev);
	if (ret){
		dev_err(dev, "parse dt profile failed\n");
		goto err;
	}

	ret = l3p_parse_dt_threshold(pdev);
	if (ret){
		dev_err(dev, "parse dt threshold failed\n");
		goto err;
	}

#ifdef L3P_KARMA_CPU_HWMON
	ret = cpu_hwmon_setup(pdev);
	if (ret){
		dev_err(dev, "cpu hwmon setup failed\n");
		goto err;
	}
#endif

	ret = karma_hwmon_setup(pdev);
	if (ret){
		dev_err(dev, "karma hwmon setup failed\n");
		goto err;
	}

	ret = dsu_hwmon_setup(pdev);
	if (ret){
		dev_err(dev, "dsu hwmon setup failed\n");
		goto err;
	}

	l3p->idle_nb.notifier_call = l3p_fcm_idle_notif;
	ret = cpu_pm_register_notifier(&l3p->idle_nb);
	if (ret){
		dev_err(dev, "cpu pm register failed\n");
		goto err;
	}

	ret = l3p_karma_setup(pdev);
	if (ret){
		dev_err(dev, "setup failed\n");
		goto err_cpu_pm;
	}

	ret = sysfs_create_group(&l3p->devfreq->dev.kobj, l3p->attr_grp);
	if (ret) {
		dev_err(dev, "sysfs create failed\n");
		goto err_setup;
	}

	l3p->acp_notify.notifier_call = l3p_karma_acp_callback;
	ret = register_l3share_acp_notifier(&l3p->acp_notify);
	if (ret) {
		dev_err(&pdev->dev, "register l3p acp notifier failed!\n");
		goto err_sysfs;
	}

	dev_err(dev, "register l3p karma exit\n");
	return 0;

err_sysfs:
	sysfs_remove_group(&l3p->devfreq->dev.kobj, l3p->attr_grp);
err_setup:
	devm_devfreq_remove_device(dev, l3p->devfreq);
	/* Wait for pending work */
	flush_workqueue(l3p->update_wq);
	/* Destroy workqueue */
	destroy_workqueue(l3p->update_wq);
	devm_iounmap(dev, l3p->base);
err_cpu_pm:
	cpu_pm_unregister_notifier(&l3p->idle_nb);
err:
	dev_err(dev, "failed to register driver, err %d.\n", ret);
	return ret;
}
/*lint +e429*/

static int l3p_karma_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct l3p_karma *l3p = platform_get_drvdata(pdev);
	int ret = 0;

	unregister_l3share_acp_notifier(&l3p->acp_notify);
	/* Wait for pending work */
	flush_workqueue(l3p->update_wq);
	/* Destroy workqueue */
	destroy_workqueue(l3p->update_wq);

	devm_devfreq_remove_device(dev, l3p->devfreq);
	cpu_pm_unregister_notifier(&l3p->idle_nb);

	return ret;
}

MODULE_DEVICE_TABLE(of, l3p_karma_devfreq_id);

static struct platform_driver l3p_karma_driver = {
	.probe	= l3p_karma_probe,
	.remove = l3p_karma_remove,
	.driver = {
		.name = L3P_KARMA_PLATFORM_DEVICE_NAME,
		.of_match_table = l3p_karma_devfreq_id,
		.pm = &l3p_karma_pm,
		.owner = THIS_MODULE,
	},
};

static int __init l3p_karma_devfreq_init(void)
{
	int ret = 0;

	ret = devfreq_add_governor(&l3p_karma_governor);
	if (ret) {
		pr_err("%s: failed to add governor: %d.\n", __func__, ret);
		return ret;
	}

	ret = platform_driver_register(&l3p_karma_driver);
	if (ret){
		ret = devfreq_remove_governor(&l3p_karma_governor);
		if(ret){
			pr_err("%s: failed to remove governor: %d.\n", __func__, ret);
		}
	}

	return ret;
}

static void __exit l3p_karma_devfreq_exit(void)
{
	int ret;

	ret = devfreq_remove_governor(&l3p_karma_governor);
	if (ret)
		pr_err("%s: failed to remove governor: %d.\n", __func__, ret);

	platform_driver_unregister(&l3p_karma_driver);
}

late_initcall(l3p_karma_devfreq_init)

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("L3CACHE PREFETCHER devfreq driver");
MODULE_VERSION("1.0");
