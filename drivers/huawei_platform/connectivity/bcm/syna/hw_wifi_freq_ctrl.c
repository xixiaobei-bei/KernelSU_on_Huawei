/*********************************************************************
Copyright  (C),  2001-2012,  Huawei  Tech.  Co.,  Ltd.

**********************************************************************/

#include <linux/hisi/hisi_cpufreq_req.h>
#include "hw_wifi_freq_ctrl.h"

struct cpufreq_req cpu_freq_req[8];
#define CPU_MAX_NUM      8
#define L_CPU_MAX_FREQ          1697000
#define H_CPU_MAX_FREQ          1997000

void hw_wifi_freq_ctrl_init(void)
{
	int cpu_id = 0;
	for(cpu_id;cpu_id < CPU_MAX_NUM;cpu_id++){
		hisi_cpufreq_init_req(&cpu_freq_req[cpu_id], cpu_id);
	}

	return;
}

void hw_wifi_freq_ctrl_destroy(void)
{
	int cpu_id = 0;
	for(cpu_id;cpu_id < CPU_MAX_NUM;cpu_id++){
		hisi_cpufreq_exit_req(&cpu_freq_req[cpu_id]);
	}

	return;
}

int hw_wifi_speed_calc_process(void)
{
	int cpu_id = 0;
	for(cpu_id;cpu_id < CPU_MAX_NUM;cpu_id++){
	if(cpu_id < 4){
		hisi_cpufreq_update_req(&cpu_freq_req[cpu_id],L_CPU_MAX_FREQ);
	}
	else{
		hisi_cpufreq_update_req(&cpu_freq_req[cpu_id],H_CPU_MAX_FREQ);
	}
	}

	return 0;
}

int hw_wifi_revert_process(void)
{
	int cpu_id = 0;
	for(cpu_id;cpu_id < CPU_MAX_NUM;cpu_id++){
		hisi_cpufreq_update_req(&cpu_freq_req[cpu_id], 0);
	}
	return 0;
}

/*
 * value 1: max cpu freq; 0: revert cpu freq
 */
int hw_wifi_set_cpu_performance(int value)
{
	if (value) {
		hw_wifi_speed_calc_process();
	} else {
		hw_wifi_revert_process();
	}
	return 0;
}
