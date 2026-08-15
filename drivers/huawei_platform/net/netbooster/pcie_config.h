

#ifndef pcie_config__h
#define pcie_config__h

/* callback func type */
typedef void (*power_config_fp)(uint32_t power_level, uint32_t timeout);

typedef struct {
	uint32_t power_level;  /* 0:low power, 1:hight perf */
	uint32_t timeout;  /* ms */
} power_level_info;

void config_pcie_power_level(int power_level, int timeout);
void kirin_pm_mode_notify_register(power_config_fp handler);
void kirin_pm_mode_notify_unregister(void);
power_level_info get_power_level_info(void);

#endif
