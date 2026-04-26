/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Description: Contexthub share memory driver
 * Author: Huawei
 * Create: 2019-10-01
 */
#ifndef __IOMCU_IPC_H__
#define __IOMCU_IPC_H__

#include <linux/types.h>
#include <linux/hisi/contexthub/protocol_as.h>
#include <hisi_rproc.h>


#define IOM3_ST_NORMAL 0
#define IOM3_ST_RECOVERY 1
#define IOM3_ST_REPEAT 2
#define MAX_SEND_LEN 32
#define IPC_BUF_USR_SIZE   (MAX_SEND_LEN - sizeof(struct pkt_header))

enum iomcu_trans_type {
	IOMCU_TRANS_IPC_TYPE = 0,
	IOMCU_TRANS_SHMEM_TYPE = 1,
	IOMCU_TRANS_IPC64_TYPE = 2,
	IOMCU_TRANS_END_TYPE,
};

struct write_info {
	int tag;
	int cmd;
	const void *wr_buf; /* maybe NULL */
	int wr_len; /* maybe zero */
	int app_tag;
};

struct read_info {
	int errno;
	int data_length;
	char data[MAX_PKT_LENGTH];
};


extern int inputhub_recv_msg_app_handler(const struct pkt_header *head, bool is_notifier);
void inputhub_pm_callback(struct pkt_header *head);
extern int iomcu_route_recv_mcu_data(const char *buf, unsigned int length);
extern int write_customize_cmd(const struct write_info *wr, struct read_info *rd, bool is_lock);
extern int register_mcu_event_notifier(int tag, int cmd, int (*notify)(const struct pkt_header *head));
extern int unregister_mcu_event_notifier(int tag, int cmd, int (*notify)(const struct pkt_header *head));
extern void inputhub_update_info(const void *buf, int ret, bool is_in_recovery);
extern void init_locks(void);
extern void init_mcu_notifier_list(void);
extern int g_iom3_state;
extern int iom3_power_state;
int iomcu_ipc_init(void);
void iomcu_ipc_exit(void);
int iomcu_write_ipc_msg(struct pkt_header *pkt, unsigned int length, struct read_info *rd, uint8_t waiter_cmd);
#endif
