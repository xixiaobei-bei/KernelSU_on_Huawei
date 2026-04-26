#ifndef _LINUX_ECHUB_BATTERY_H
#define _LINUX_ECHUB_BATTERY_H

enum charge_status_event {
	VCHRG_POWER_NONE_EVENT = 0,
	VCHRG_NOT_CHARGING_EVENT,
	VCHRG_START_CHARGING_EVENT,
	VCHRG_START_AC_CHARGING_EVENT,
	VCHRG_START_USB_CHARGING_EVENT,
	VCHRG_CHARGE_DONE_EVENT,
	VCHRG_STOP_CHARGING_EVENT,
	VCHRG_POWER_SUPPLY_OVERVOLTAGE,
	VCHRG_POWER_SUPPLY_WEAKSOURCE,
	VCHRG_STATE_WDT_TIMEOUT,
	VCHRG_STATE_WATER_INTRUSED,
	BATTERY_LOW_WARNING,
	BATTERY_LOW_SHUTDOWN,
	BATTERY_MOVE,
	SWITCH_INTB_WORK_EVENT,
	VCHRG_THERMAL_POWER_OFF,
	WIRELESS_TX_STATUS_CHANGED,
	WIRELESS_COVER_DETECTED,
	VCHRG_CURRENT_FULL_EVENT,
};

/* State-of-Charge-Threshold 1 bit */
#define BQ_FLAG_SOC1			(1<<2)
/* State-of-Charge-Threshold Final bit */
#define BQ_FLAG_SOCF			(1<<1)
#define BQ_FLAG_LOCK			(BQ_FLAG_SOC1 | BQ_FLAG_SOCF)

#define RECHG_PROTECT_THRESHOLD    (150)
#define WORK_INTERVAL_PARA_LEVEL    (5)
#define CAPACITY_MIN   0
#define CAPACITY_MAX 100
#define CAPACITY_FULL 100
#define WORK_INTERVAL_MIN             1
#define WORK_INTERVAL_MAX            60000 /*60s*/
#define WORK_INTERVAL_NOARMAL 10000 /*10s*/
#define WORK_INTERVAL_REACH_FULL 30000 /*30s*/

#define CAP_LOCK_PARA_LEVEL    (2)

#define CAPATICY_DEC_PARA_LEVEL       (2)

#define BASE_DECIMAL (10)
#define CAPACITY_DEC_SAMPLE_NUMS       (12)
#define CAPATICY_DEC_TIMER_INTERVAL  (800) /* time out setting, 0.8 seconds */
#define CAPATICY_DEC_PARA_LEVEL       (2)

enum work_interval_para_info {
	WORK_INTERVAL_CAP_MIN = 0,
	WORK_INTERVAL_CAP_MAX,
	WORK_INTERVAL_VALUE,
	WORK_INTERVAL_PARA_TOTAL,
};

struct work_interval_para {
	int cap_min;
	int cap_max;
	int work_interval;
};

struct capacity_dec_para {
	u32 cap_dec_base_decimal;
	u32 cap_dec_sample_nums;
	u32 cap_dec_timer_interval;
};
struct cap_lock_para {
	int cap;
	int level_vol;
};

enum capacity_dec_para_info {
	CAP_DEC_BASE_DECIMAL = 0,
	CAP_DEC_SAMPLE_NUMS,
	CAP_DEC_TIMER_INTERVAL,
	CAP_DEC_PARA_TOTAL,
};

#endif
