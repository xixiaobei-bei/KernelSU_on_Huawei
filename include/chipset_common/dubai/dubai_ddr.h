#ifndef DUBAI_DDR_H
#define DUBAI_DDR_H

#include <linux/types.h>

#define DDR_IP_NAME_LEN		16
#define DDR_FREQ_CNT_MAX	16
#define DDR_IP_CNT_MAX		10

struct dubai_ddr_freq_time {
	int64_t freq;
	int64_t time;
} __packed;

struct dubai_ddr_time_in_state {
	int64_t sr_time;
	int64_t pd_time;
	struct dubai_ddr_freq_time freq_time[DDR_FREQ_CNT_MAX];
} __packed;

struct dubai_ddr_freq_data {
	int32_t freq;
	uint64_t data;
} __packed;

struct dubai_ddr_ip_stats {
	char ip[DDR_IP_NAME_LEN];
	struct dubai_ddr_freq_data freq_data[DDR_FREQ_CNT_MAX];
} __packed;

typedef int32_t (*dubai_ddr_get_freq_cnt_fn_t)(void);
typedef int32_t (*dubai_ddr_get_ip_cnt_fn_t)(void);
typedef int32_t (*dubai_ddr_get_time_in_state_fn_t)(struct dubai_ddr_time_in_state *time_in_state);
typedef int32_t (*dubai_ddr_get_ip_stats_fn_t)(int32_t ip_cnt, struct dubai_ddr_ip_stats *ip_stats);

struct dubai_ddr_stats_ops {
	dubai_ddr_get_freq_cnt_fn_t get_freq_cnt;
	dubai_ddr_get_ip_cnt_fn_t get_ip_cnt;
	dubai_ddr_get_time_in_state_fn_t get_time_in_state;
	dubai_ddr_get_ip_stats_fn_t get_ip_stats;
};

#endif // DUBAI_DDR_H
