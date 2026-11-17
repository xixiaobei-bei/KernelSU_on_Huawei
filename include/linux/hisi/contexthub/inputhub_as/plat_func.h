/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019. All rights reserved.
 * Description: plat_func.h.
 * Create: 2019/11/05
 */

#ifndef __PLAT_FUNC_H
#define __PLAT_FUNC_H

#include <linux/hisi/contexthub/iomcu_log.h>
#include <linux/hisi/contexthub/iomcu_boot.h>

struct log_buff_t {
	u32 mutex;
	u16 index;
	u16 pingpang;
	u32 buff;
	u32 ddr_log_buff_cnt;
	u32 ddr_log_buff_index;
	u32 ddr_log_buff_last_update_index;
};


struct wrong_wakeup_msg_t {
	u32 flag;
	u32 reserved1;
	u64 time;
	u32 irq0;
	u32 irq1;
	u32 recvfromapmsg[4];
	u32 recvfromlpmsg[4];
	u32 sendtoapmsg[4];
	u32 sendtolpmsg[4];
	u32 recvfromapmsgmode;
	u32 recvfromlpmsgmode;
	u32 sendtoapmsgmode;
	u32 sendtolpmsgmode;
};

struct config_on_ddr {
	struct dump_config dump_config;
	struct log_buff_t logbuff_cb_backup;
	struct wrong_wakeup_msg_t wrong_wakeup_msg;
	u32 log_level;
	float gyro_offset[3];
	u8 modem_open_app_state;
};

struct config_on_ddr *get_config_on_ddr(void);

static inline bool is_space_ch(char ch)
{
	return (ch == ' ') || (ch == '\t');
}

static inline bool end_of_string(char ch)
{
	return (ch == '\0') || (ch == '\r') || (ch == '\n');
}

/*
 * Function    : get_sensor_mcu_mode
 * Description : return 1 contexthub boot succ, otherwise fail
 * Input       : none
 * Output      : none
 * Return      : 0 OK, other error
 */
int get_sensor_mcu_mode(void);

/*
 * Function    : get_tag_str
 * Description : get string of tag
 * Input       : [tag]
 * Output      : none
 * Return      : null is error, other ok
 */
char *get_tag_str(int tag);

/*
 * Function    : get_str_begin
 * Description : get string of begin, skip spaces and '\t'
 * Input       : [cmd_buf] command string
 * Output      : none
 * Return      : null is no valid string, other return valid string
 *               start position
 */
const char *get_str_begin(const char *cmd_buf);

/*
 * Function    : get_str_end
 * Description : get string of end, skip spaces and '\t'
 * Input       : [cmd_buf] command string
 * Output      : none
 * Return      : null is no valid string or input is null, other return
 *               valid string last position
 */
const char *get_str_end(const char *cmd_buf);

/*
 * Function    : get_arg
 * Description : get args of string, support digit arg or hex-digit arg
 * Input       : [str] args string
 *             : [arg] save int value which get from str
 * Output      : none
 * Return      : input args is not digit or hex-digit, return false;
 *               Otherwise return true.
 */
bool get_arg(const char *str, int *arg);

#endif /* __PLAT_FUNC_H */
