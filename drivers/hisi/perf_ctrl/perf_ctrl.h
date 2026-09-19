#ifndef __PERF_CTRL_H__
#define __PERF_CTRL_H__

#include <linux/hisi/hisi_perf_ctrl.h>

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
	PERF_CTRL_MAX_NR = SET_RTG_UTIL_INVALID_INTERVAL,
};

#define PERF_CTRL_GET_SCHED_STAT _IOR(PERF_CTRL_MAGIC, GET_SCHED_STAT, struct sched_stat)
#define PERF_CTRL_GET_IPA_STAT _IOR(PERF_CTRL_MAGIC, GET_IPA_STAT, struct ipa_stat)
#define PERF_CTRL_GET_DDR_FLUX _IOR(PERF_CTRL_MAGIC, GET_DDR_FLUX, struct ddr_flux)
#define PERF_CTRL_GET_RELATED_TID _IOWR(PERF_CTRL_MAGIC, GET_RELATED_TID, struct related_tid_info)
#define PERF_CTRL_DRG_GET_DEV_FREQ _IOWR(PERF_CTRL_MAGIC, GET_DEV_FREQ, struct drg_dev_freq)
#define PERF_CTRL_GET_THERMAL_CDEV_POWER _IOWR(PERF_CTRL_MAGIC, GET_THERMAL_CDEV_POWER, struct thermal_cdev_power)

#define PERF_CTRL_SET_FRAME_RATE _IOWR(PERF_CTRL_MAGIC, SET_FRAME_RATE, int*)
#define PERF_CTRL_SET_FRAME_MARGIN _IOWR(PERF_CTRL_MAGIC, SET_FRAME_MARGIN, int*)
#define PERF_CTRL_SET_FRAME_STATUS _IOWR(PERF_CTRL_MAGIC, SET_FRAME_STATUS, int*)
#define PERF_CTRL_SET_TASK_RTG _IOWR(PERF_CTRL_MAGIC, SET_TASK_RTG, struct rtg_group_task)
#define PERF_CTRL_SET_RTG_CPUS _IOWR(PERF_CTRL_MAGIC, SET_RTG_CPUS, struct rtg_cpus)
#define PERF_CTRL_SET_RTG_FREQ _IOWR(PERF_CTRL_MAGIC, SET_RTG_FREQ, struct rtg_freq)
#define PERF_CTRL_SET_RTG_FREQ_UPDATE_INTERVAL _IOWR(PERF_CTRL_MAGIC, SET_RTG_FREQ_UPDATE_INTERVAL, struct rtg_interval)
#define PERF_CTRL_SET_RTG_UTIL_INVALID_INTERVAL _IOWR(PERF_CTRL_MAGIC, SET_RTG_UTIL_INVALID_INTERVAL, struct rtg_interval)


#define IPA_SENSOR_SYSTEM_H "system_h"
#define IPA_SENSOR_NUM 3

extern int ipa_get_sensor_value(u32 sensor, int *val);
extern int ipa_get_periph_value(u32 sensor, int *val);
extern int ipa_get_periph_id(char *name);

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

struct ddr_flux {
	unsigned long long rd_flux;
	unsigned long long wr_flux;
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

#endif /*__PERF_CTRL_H__*/
