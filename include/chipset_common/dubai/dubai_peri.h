#ifndef DUBAI_PERI_H
#define DUBAI_PERI_H

#include <linux/types.h>

#define PERI_IP_NAME_LEN	16
#define PERI_VOLT_CNT_MAX	6
#define PERI_IP_CNT_MAX		10

struct dubai_peri_volt_time {
	int32_t volt;
	int64_t time;
} __packed;

struct dubai_peri_ip_volt_time {
	int32_t volt;
	int64_t active_time;
	int64_t idle_time;
	int64_t off_time;
} __packed;

struct dubai_peri_ip_stats {
	char ip[PERI_IP_NAME_LEN];
	struct dubai_peri_ip_volt_time volt_time[PERI_VOLT_CNT_MAX];
} __packed;

typedef int32_t (*dubai_peri_get_volt_cnt_fn_t)(void);
typedef int32_t (*dubai_peri_get_ip_cnt_fn_t)(void);
typedef int32_t (*dubai_peri_get_volt_time_fn_t)(int32_t volt_cnt, struct dubai_peri_volt_time *volt_time);
typedef int32_t (*dubai_peri_get_ip_stats_fn_t)(int32_t ip_cnt, struct dubai_peri_ip_stats *ip_stats);

struct dubai_peri_stats_ops {
	dubai_peri_get_volt_cnt_fn_t get_volt_cnt;
	dubai_peri_get_ip_cnt_fn_t get_ip_cnt;
	dubai_peri_get_volt_time_fn_t get_volt_time;
	dubai_peri_get_ip_stats_fn_t get_ip_stats;
};

#endif // DUBAI_PERI_H
