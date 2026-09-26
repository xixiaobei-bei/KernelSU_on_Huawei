#ifndef DUBAI_SENSORHUB_STATS_H
#define DUBAI_SENSORHUB_STATS_H

#include <linux/types.h>

#define PHYSICAL_SENSOR_TYPE_MAX			40

enum {
	TINY_CORE_MODEL_FD = 0,
	TINY_CORE_MODEL_SD,
	TINY_CORE_MODEL_FR,
	TINY_CORE_MODEL_AD,
	TINY_CORE_MODEL_FP,
	TINY_CORE_MODEL_HD,
	TINY_CORE_MODEL_WD,
	TINY_CORE_MODEL_WE,
	TINY_CORE_MODEL_WC,
	TINY_CORE_MODEL_MAX,
};

struct swing_data {
	uint64_t active_time;
	uint64_t software_standby_time;
	uint64_t als_time;
	uint64_t fill_light_time;
	uint32_t capture_cnt;
	uint32_t fill_light_cnt;
	uint32_t tiny_core_model_cnt[TINY_CORE_MODEL_MAX];
} __packed;

struct tp_duration {
	uint64_t active_time;
	uint64_t idle_time;
} __packed;

struct sensor_time {
	uint32_t type;
	uint32_t time;
} __packed;

struct sensor_time_transmit {
	int32_t count;
	struct sensor_time data[PHYSICAL_SENSOR_TYPE_MAX];
} __packed;

struct lcd_state_duration {
	uint32_t normal_on;
	uint32_t low_power;
} __packed;

#ifdef CONFIG_INPUTHUB_20
long dubai_ioctl_sensorhub(unsigned int cmd, void __user *argp);
#else /* !CONFIG_INPUTHUB_20 */
static inline long dubai_ioctl_sensorhub(unsigned int cmd, void __user *argp) { return -EOPNOTSUPP; }
#endif /* !CONFIG_INPUTHUB_20 */

#endif // DUBAI_SENSORHUB_STATS_H
