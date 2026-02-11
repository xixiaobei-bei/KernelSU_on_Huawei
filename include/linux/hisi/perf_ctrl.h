#ifndef __PERF_CTRL_H__
#define __PERF_CTRL_H__

#include <linux/hisi/hisi_gpu_hook.h>

#define PERF_CTRL_MAGIC 'x'

enum {
	GET_SCHED_STAT = 1,
	SET_TASK_UTIL,
	GET_IPA_STAT,
	GET_DDR_FLUX,
	GET_RELATED_TID,
	GET_DEV_FREQ,
	GET_THERMAL_CDEV_POWER,
	SET_FRAME_RATE,
	SET_FRAME_MARGIN,
	SET_FRAME_STATUS,
	SET_TASK_RTG,
	SET_RTG_CPUS,
	SET_RTG_FREQ,
	SET_RTG_FREQ_UPDATE_INTERVAL,
	SET_RTG_UTIL_INVALID_INTERVAL,
	GET_GPU_FENCE,
	/* render related threads */
	INIT_RENDER_PID,
	GET_RENDER_RT,
	STOP_RENDER_RT,
	GET_RENDER_HT,
	DESTROY_RENDER_RT,
	SET_RTG_LOAD_MODE,
	SET_RTG_ED_PARAMS,
	GET_DEV_CAP,

	PERF_CTRL_MAX_NR,
};

/* use cap to track the difference of the ioctl cmd */
enum {
	CAP_AI_SCHED_COMM_CMD = 0,
	CAP_RTG_CMD,
	CAP_RENDER_RT_CMD,
};

#define PERF_CTRL_GET_SCHED_STAT _IOR(PERF_CTRL_MAGIC, GET_SCHED_STAT, struct sched_stat)
#define PERF_CTRL_GET_IPA_STAT _IOR(PERF_CTRL_MAGIC, GET_IPA_STAT, struct ipa_stat)
#define PERF_CTRL_GET_DDR_FLUX _IOR(PERF_CTRL_MAGIC, GET_DDR_FLUX, struct ddr_flux)
#define PERF_CTRL_GET_RELATED_TID _IOWR(PERF_CTRL_MAGIC, GET_RELATED_TID, struct related_tid_info)
#define PERF_CTRL_DRG_GET_DEV_FREQ _IOWR(PERF_CTRL_MAGIC, GET_DEV_FREQ, struct drg_dev_freq)
#define PERF_CTRL_GET_THERMAL_CDEV_POWER _IOWR(PERF_CTRL_MAGIC, GET_THERMAL_CDEV_POWER, struct thermal_cdev_power)
#define PERF_CTRL_GET_GPU_FENCE _IOWR(PERF_CTRL_MAGIC, GET_GPU_FENCE, struct kbase_fence_info)

#define PERF_CTRL_SET_FRAME_RATE _IOWR(PERF_CTRL_MAGIC, SET_FRAME_RATE, int*)
#define PERF_CTRL_SET_FRAME_MARGIN _IOWR(PERF_CTRL_MAGIC, SET_FRAME_MARGIN, int*)
#define PERF_CTRL_SET_FRAME_STATUS _IOWR(PERF_CTRL_MAGIC, SET_FRAME_STATUS, int*)
#define PERF_CTRL_SET_TASK_RTG _IOWR(PERF_CTRL_MAGIC, SET_TASK_RTG, struct rtg_group_task)
#define PERF_CTRL_SET_RTG_CPUS _IOWR(PERF_CTRL_MAGIC, SET_RTG_CPUS, struct rtg_cpus)
#define PERF_CTRL_SET_RTG_FREQ _IOWR(PERF_CTRL_MAGIC, SET_RTG_FREQ, struct rtg_freq)
#define PERF_CTRL_SET_RTG_FREQ_UPDATE_INTERVAL _IOWR(PERF_CTRL_MAGIC, SET_RTG_FREQ_UPDATE_INTERVAL, struct rtg_interval)
#define PERF_CTRL_SET_RTG_UTIL_INVALID_INTERVAL _IOWR(PERF_CTRL_MAGIC, SET_RTG_UTIL_INVALID_INTERVAL, struct rtg_interval)
#define PERF_CTRL_SET_RTG_LOAD_MODE _IOW(PERF_CTRL_MAGIC, SET_RTG_LOAD_MODE, struct rtg_load_mode)
#define PERF_CTRL_SET_RTG_ED_PARAMS _IOW(PERF_CTRL_MAGIC, SET_RTG_ED_PARAMS, struct rtg_ed_params)

#define PERF_CTRL_INIT_RENDER_PID _IOW(PERF_CTRL_MAGIC, INIT_RENDER_PID, struct render_init_paras)
#define PERF_CTRL_GET_RENDER_RT _IOWR(PERF_CTRL_MAGIC, GET_RENDER_RT, struct render_rt)
#define PERF_CTRL_STOP_RENDER_RT _IOW(PERF_CTRL_MAGIC, STOP_RENDER_RT, struct render_stop)
#define PERF_CTRL_GET_RENDER_HT _IOWR(PERF_CTRL_MAGIC, GET_RENDER_HT, struct render_ht)
#define PERF_CTRL_DESTROY_RENDER_RT _IOW(PERF_CTRL_MAGIC, DESTROY_RENDER_RT, pid_t)
#define PERF_CTRL_GET_DEV_CAP _IOR(PERF_CTRL_MAGIC, GET_DEV_CAP, unsigned long)

#define DEV_TYPE_CPU_CLUSTER (1 << 0) << 8

enum drg_dev_type {
	DRG_NONE_DEV = 0,
	/* CPU TYPE if new cpu add after(eg. small strong ...) */
	DRG_CPU_CLUSTER0 = (1 << 0) << 8,
	DRG_CPU_CLUSTER1,
	DRG_CPU_CLUSTER2,
	/* CACHE TYPE if new cache add after(eg. l4 l5 l6 ...)*/
	DRG_L3_CACHE = (1 << 1) << 8,
	/* If new type add after here */
};

struct drg_dev_freq {
	/* enum drg_dev_type */
	unsigned int type;
	/* described as Hz  */
	unsigned long max_freq;
	unsigned long min_freq;
};

static inline bool devtype_name_compare(enum drg_dev_type type,const char *dev_name)
{
	if (type == DRG_L3_CACHE && strcasecmp(dev_name, "l3c_devfreq") == 0)
		return true;

	return false;
}

enum thermal_cdev_type {
	CDEV_GPU = 0,
	CDEV_CPU_CLUSTER0,
	CDEV_CPU_CLUSTER1,
	CDEV_CPU_CLUSTER2,
	THERMAL_CDEV_MAX,
};

enum thermal_zone_type {
	SOC_THERMAL_ZONE = 0,
	BOARD_THERMAL_ZONE,
	THERMAL_ZONE_MAX,
};

struct thermal_cdev_power {
	int thermal_zone_type;
	unsigned int cdev_power[THERMAL_CDEV_MAX];
};

#define IPA_SENSOR_SYSTEM_H "system_h"
#define IPA_SENSOR_NUM 3
typedef enum {
        DDR_PERFDATA_PERFCTRL = 0,
        DDR_PERFDATA_KARMA,
        DDR_PERFDATA_CLIENT_MAX,
} DDR_PERFDATA_CLIENT;

struct ddr_flux {
	unsigned long long rd_flux;
	unsigned long long wr_flux;
};
extern int ipa_get_sensor_value(u32 sensor, int *val);
extern int ipa_get_periph_value(u32 sensor, int *val);
extern int ipa_get_periph_id(char *name);
extern int get_ddrc_flux_all_ch(struct ddr_flux* ddr_flux_str, DDR_PERFDATA_CLIENT client);

struct sched_stat {
	pid_t pid;
	unsigned long long sum_exec_runtime;
	unsigned long long run_delay;
	unsigned long pcount;
};

struct ipa_stat {
	unsigned int cluster0;
	unsigned int cluster1;
	unsigned int cluster2;
	unsigned int gpu;
	int soc_temp;
	int board_temp;
};

#define MAX_TID_COUNT 512

struct related_tid_info {
	/* pid of query task */
	pid_t pid;
	/* Obtain tid count */
	int rel_count;
	/* Obtain tid array */
	pid_t rel_tid[MAX_TID_COUNT];
};

struct rtg_group_task {
	pid_t pid;
	unsigned int grp_id;
	bool pmu_sample_enabled;
};

struct rtg_cpus {
	unsigned int grp_id;
	int cluster_id;
};

struct rtg_freq {
	unsigned int grp_id;
	unsigned int freq;
};

struct rtg_interval {
	unsigned int grp_id;
	unsigned int interval;
};

/* rtg:load_mode */
struct rtg_load_mode {
	unsigned int grp_id;
	bool freq_enabled;
	bool util_enabled;
};

/* rtg: ed_params */
struct rtg_ed_params {
	unsigned int grp_id;
	bool enabled;
	unsigned int running_ns;
	unsigned int waiting_ns;
	unsigned int nt_running_ns;
};

/*
 * pid == 0 indicates invalid entry.
 * util should be in range of 0 to 1024
 */
struct thread_util {
	pid_t pid;
	unsigned long util;
};

#define MAX_THREAD_NUM	(21)
struct render_rt {
	pid_t	render_pid;
	int 	num;
	struct	thread_util utils[MAX_THREAD_NUM];
};

struct render_ht {
	pid_t	render_pid;
	struct	thread_util utils[MAX_THREAD_NUM];
};

struct render_init_paras {
	pid_t	render_pid;
	bool	force_init;
};

struct render_stop {
	pid_t	render_pid;
	int	stopped;
};
#endif /*__PERF_CTRL_H__*/
