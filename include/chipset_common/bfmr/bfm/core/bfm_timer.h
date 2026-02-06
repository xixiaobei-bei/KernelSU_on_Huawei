/*
 * copyright: Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
 *
 * file: bfm_timer.h
 *
 * define the basic external enum/macros/interface for BFM (Boot Fail Monitor)
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef BFM_TIMER_H
#define BFM_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#define BFMR_ACTION_NAME_LEN 64

enum bfm_action_timer {
	ACT_TIMER_START = 0xbfac0000,
	ACT_TIMER_STOP = 0xbfac0001,
	ACT_TIMER_PAUSE = 0xbfac0002,
	ACT_TIMER_RESUME = 0xbfac0003,
};

struct action_ioctl_data {
	enum bfm_action_timer op;
	char action_name[BFMR_ACTION_NAME_LEN];
	u32 action_timer_timeout_value;
};

/*
 * @param: state [out], the state of the boot timer.
 *
 * @return: 0 - success, -1 - failed.
 *
 * @note:
 *	1. this function only need be initialized in kernel.
 *	2. if *state == 0, the boot timer is disabled, if *state == 1,
 *	   the boot timer is enbaled.
 */
int bfm_get_boot_timer_state(int *state);

/*
 * @return: 0 - success, -1 - failed.
 */
int bfm_suspend_boot_timer(void);

/*
 * @return: 0 - success, -1 - failed.
 */
int bfmr_resume_boot_timer(void);

/*
 * @param: timeout_value [int], timeout value to be set, unit:msec.
 *
 * @return: 0 - success, -1 - failed.
 *
 * @note:
 *	1. this function only need be initialized in kernel.
 */
int bfm_set_boot_timer_timeout_value(unsigned int timeout_value);

/*
 * @param: ptimeout_value [out], timeout value of the timer, unit:msec.
 *
 * @return: 0 - success, -1 - failed.
 *
 * @note:
 *	1. this function only need be initialized in kernel.
 */
int bfm_get_boot_timer_timeout_value(unsigned int *ptimeout_value);

/*
 * @brief:
 *	1. creat a new action timer
 *	2. stop the short boot timer
 *
 * @return: 0 - success, -1 - failed.
 */
int bfm_action_timer_start(char *act_name, unsigned int timeout_value);

/*
 * @brief:
 *	1. stop & destroy the action timer
 *	2. resume the short boot timer if the action timer list is empty
 *
 * @return: 0 - success, -1 - failed.
 */
int bfm_action_timer_stop(char *act_name);

/*
 * @brief:
 *	1. pause the action timer
 *
 * @return: 0 - success, -1 - failed.
 */
int bfm_action_timer_pause(char *act_name);

/*
 * @brief:
 *	1. resume the action timer
 *
 * @return: 0 - success, -1 - failed.
 */
int bfm_action_timer_resume(char *act_name);

/*
 * @return: 0 - success, -1 - failed.
 *
 * @note: this function only need be initialized in kernel.
 */
int bfm_stop_boot_timer(void);

/*
 * @return: 0 - success, -1 - failed.
 *
 * @note: this function only need be initialized in kernel.
 */
int bfm_init_boot_timer(void);

#ifdef __cplusplus
}
#endif

#endif

