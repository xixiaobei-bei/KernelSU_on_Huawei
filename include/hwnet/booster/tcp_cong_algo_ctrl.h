

#ifndef _TCP_CONG_ALGO_CTRL_H
#define _TCP_CONG_ALGO_CTRL_H

#include <linux/spinlock.h>

#include "netlink_handle.h"

#define MAX_CONFIG_APK_NUM 20
#define INVALID_VAL (-1)
#define DATA_SERVICE_ON 1
#define DATA_SERVICE_OFF 2

struct config_info {
	int uid;
	int network_type;
	int uid_state;
	int restore_threshold;
	int package_loss_rate;
	int total_throughput;
	int average_rate;
	int retransmit_times;
	int cong_algo;
};

/* config information sent by booster */
struct apk_config {
	struct config_info config[MAX_CONFIG_APK_NUM];

	int nums; // app count
	spinlock_t lock; // this context lock
};

/* Dynamic update current uid state */
struct curr_uid_info {
	int cur_uid; // current uid
	int cur_uid_state; // current uid state background or foreground
	int cur_ds_state; // current data service on or off
};

/* The supported tcp congestion algorithms list */
enum tcp_cong_algos {
	CUBIC = 0,
	BBR,
	MIBO,
};

msg_process *hw_tcp_cong_algo_ctrl_init(notify_event *notify);
#endif
