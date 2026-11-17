

#ifndef BOOTFAIL_TIMER_H
#define BOOTFAIL_TIMER_H

/* ---- includes ---- */
#include <linux/types.h>

/* ----c++ support ---- */
#ifdef __cplusplus
extern "C" {
#endif

/* ---- export macroes ---- */
#define ACTION_NAME_LEN 64

/* ---- export prototypes ---- */
enum action_timer_ops {
	ACT_TIMER_START = 0xbfac0000,
	ACT_TIMER_STOP = 0xbfac0001,
	ACT_TIMER_PAUSE = 0xbfac0002,
	ACT_TIMER_RESUME = 0xbfac0003,
};

struct action_ioctl_data {
	enum action_timer_ops op;
	char action_name[ACTION_NAME_LEN];
	u32 action_timer_timeout_value;
};

/* ---- export function prototypes ---- */
int get_boot_timer_state(int *state);
int suspend_boot_timer(void);
int resume_boot_timer(void);
int action_timer_start(char *act_name, unsigned int timeout_value);
int action_timer_stop(char *act_name);
int action_timer_pause(char *act_name);
int action_timer_resume(char *act_name);
int set_boot_timer_timeout_value(unsigned int timeout_value);
int get_boot_timer_timeout_value(unsigned int *ptimeout_value);
int stop_boot_timer(void);
int start_boot_timer(void);
unsigned int get_elapsed_time(void);

/* ---- c++ support ---- */
#ifdef __cplusplus
}
#endif
#endif
