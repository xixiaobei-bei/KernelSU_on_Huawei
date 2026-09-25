/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Description: Contexthub share memory driver
 * Author: Huawei
 * Create: 2019-10-01
 */
#ifndef __IOMCU_IPC_H__
#define __IOMCU_IPC_H__

#include <linux/types.h>
#include <protocol.h>
#include <hisi_rproc.h>


#define IOM3_ST_NORMAL 0
#define IOM3_ST_RECOVERY 1
#define IOM3_ST_REPEAT 2
#define MAX_SEND_LEN 32


enum iomcu_trans_type {
	IOMCU_TRANS_IPC_TYPE = 0,
	IOMCU_TRANS_SHMEM_TYPE = 1,
	IOMCU_TRANS_IPC64_TYPE = 2,
	IOMCU_TRANS_END_TYPE,
};

typedef struct write_info {
	int tag;
	int cmd;
	const void *wr_buf; /* maybe NULL */
	int wr_len; /* maybe zero */
	int app_tag;
} write_info_t;

typedef struct read_info {
	int errno;
	int data_length;
	char data[MAX_PKT_LENGTH];
} read_info_t;


extern int inputhub_recv_msg_app_handler(const pkt_header_t *head, bool is_notifier);
void inputhub_pm_callback(pkt_header_t *head);
extern int iomcu_route_recv_mcu_data(const char *buf, unsigned int length);
extern int write_customize_cmd(const struct write_info *wr,
		struct read_info *rd, bool is_lock);
extern int register_mcu_event_notifier(int tag, int cmd,
		int (*notify)(const pkt_header_t *head));
extern int unregister_mcu_event_notifier(int tag,
		int cmd, int (*notify)(const pkt_header_t *head));
extern void inputhub_update_info(const void *buf,int ret,bool is_in_recovery);
extern void init_locks(void);
extern void init_mcu_notifier_list(void);
extern void iomcu_ipc_init(void);
extern int inputhub_mcu_write_cmd_adapter(const void *buf, unsigned int length,
	struct read_info *rd);
extern int inputhub_mcu_write_cmd_nolock(const void *buf, unsigned int length);
int iomcu_write_ipc_msg(pkt_header_t *pkt, unsigned int length,struct read_info *rd);
#endif
