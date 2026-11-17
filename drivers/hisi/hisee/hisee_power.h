/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: hisee_power.h
 * Create: 2019-02-17
 */
#ifndef HISEE_POWER_H
#define HISEE_POWER_H
#include <linux/atomic.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/pm.h>
#include <linux/timer.h>
#include <linux/types.h>

#define HISEE_POWERCTRL_TIMEOUT_ERROR        (-4000)
#define HISEE_POWERCTRL_NOTIFY_ERROR         (-4001)
#define HISEE_POWERCTRL_RETRY_FAILURE_ERROR  (-4002)
#define HISEE_POWERCTRL_FLOW_ERROR           (-4003)

#define HISEE_IPC_TXBUF_SIZE  3
#define IPC_TX_CMD_INDEX   0
#define IPC_TX_MSG_0_INDEX 1
#define IPC_TX_MSG_1_INDEX 2
#define HISEE_IPC_ACKBUF_SIZE 2
#define IPC_ACK_MSG_0_INDEX 0
#define IPC_ACK_MSG_1_INDEX 1

/* Lpm3 communication id */
#define HISEE_LPM3_CMD        IPC_CMD(OBJ_AP, OBJ_LPM3, CMD_NOTIFY, 0)
#define HISEE_LPM3_ACK_MAGIC  0xaaccbbdd

#define HISEE_PWROFF_LOCK     1

/* timeout of thread exit when waiting semaphore, 30s */
#define HISEE_THREAD_WAIT_TIMEOUT (msecs_to_jiffies(30000))

#define TIMEOUT_MAX_LEN           32

#define HISEE_POWER_STATE_SHOW_VALUE 1
#define HISEE_FAILURE_SHOW_VALUE     (-1)

enum hisee_timeout_vote_id_type {
	NFC_SERVICE = 0,
#ifdef CONFIG_HISEE_SUPPORT_INSE_ENCRYPT
	INSE_ENCRYPT = 1, /* ID for pin code , 3D face and fingerprint. */
#endif
	MAX_TIMEOUT_ID,
};

/* the para to lpm3 throug atf */
enum hisee_power_operation {
	HISEE_POWER_OFF = 0x01000100,
	HISEE_POWER_ON_BOOTING = 0x01000101,
	HISEE_POWER_ON_UPGRADE = 0x01000102,
	HISEE_POWER_ON_UPGRADE_SM = 0x01000103,
	HISEE_POWER_ON_BOOTING_MISC = 0x01000104,
	HISEE_POWER_MAX_OP,
};

/* the powerctrl command */
enum hisee_power_cmd {
	HISEE_POWER_CMD_ON = 0x01000200,
	HISEE_POWER_CMD_OFF = 0x01000201,
};

/* the power vote record method */
enum hisee_power_vote_record {
	HISEE_POWER_VOTE_RECORD_CNT = 0x01000300,
	HISEE_POWER_VOTE_RECORD_PRO = 0x01000301,
};

/* the power status */
enum hisee_power_status {
	HISEE_POWER_STATUS_ON = 0x01000400,
	HISEE_POWER_STATUS_OFF = 0x01000401,
};

union hisee_power_vote_status {
	unsigned long value;
	struct {
		unsigned int huawei_wallet:8;
		unsigned int u_shield:8;
		unsigned int chip_test_and_upgrade:8;
		unsigned int unknown_id:8;
		unsigned int time_out:8;
#ifdef CONFIG_HISEE_SUPPORT_INSE_ENCRYPT
		unsigned int inse_encrypt:8;
		unsigned int reserved:16;
#else
		unsigned int reserved:24;
#endif
	} status;
};

enum hisee_state {
	HISEE_STATE_POWER_DOWN = 0,
	HISEE_STATE_POWER_UP   = 1,
	HISEE_STATE_MISC_READY = 2,
	HISEE_STATE_COS_READY  = 3,
	HISEE_STATE_POWER_DOWN_DOING = 4,
	HISEE_STATE_POWER_UP_DOING   = 5,
#ifdef CONFIG_HISEE_SUPPORT_DCS
	HISEE_STATE_DCS_UPGRADE_DONE = 7,
#endif
	HISEE_STATE_MAX,
};

struct timer_entry_list {
	struct list_head list;
	struct timer_list timer;
	atomic_t handled;
};

void set_pre_enable_clk(bool value);
unsigned long get_power_vote_status(void);
unsigned int get_used_cos_id(void);
unsigned int get_runtime_cos_id(void);

int hisee_suspend(struct platform_device *pdev, struct pm_message state);
void hisee_power_ctrl_init(void);
ssize_t hisee_check_ready_show(struct device *dev,
			       struct device_attribute *attr, char *buf);
int wait_hisee_ready(enum hisee_state ready_state, unsigned int timeout_ms);
int _check_cos_ready_show(char *result_buff, size_t len);

/* buf is the process id */
int hisee_get_cosid_processid(const void *buf,
			      unsigned int *cos_id, unsigned int *process_id);
int hisee_poweron_booting_func(const void *buf, int para);
int hisee_poweron_upgrade_func(const void *buf, int para);
int hisee_poweroff_func(const void *buf, int para);
int hisee_poweron_timeout_func(const void *buf, int para);
enum hisee_power_status hisee_get_power_status(void);

#ifdef CONFIG_SMX_PROCESS
int smx_process(enum hisee_power_operation op_type,
		unsigned int op_cosid, int power_cmd);
#endif
#ifdef CONFIG_HISEE_NFC_IRQ_SWITCH
int hisee_nfc_irq_switch_func(const void *buf, int para);
#endif
#endif
