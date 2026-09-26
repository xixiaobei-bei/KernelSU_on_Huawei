/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: IPC AP IOMCU BASE
 * Author: Huawei
 * Create: 2020-08-26
 */
#ifndef __IPC_AP_IOMCU_BASE_H__
#define __IPC_AP_IOMCU_BASE_H__

#include <linux/hisi/contexthub/protocol_as.h>

#define STARTUP_IOM3_CMD 0x00070001
#define RELOAD_IOM3_CMD 0x0007030D
#define IPC_SHM_MAGIC 0x1a2b3c4d
#define IPC_SHM_BUSY 0x67
#define IPC_SHM_FREE 0xab
#define MID_PKT_LEN (128 - sizeof(struct pkt_header))

struct ipc_shm_ctrl_hdr {
	int module_id;
	unsigned int buf_size;
	unsigned int offset;
	int msg_type;
	int checksum;
	unsigned int priv;
};

struct shmem_ipc_ctrl_package {
	struct pkt_header hd;
	struct ipc_shm_ctrl_hdr sh_hdr;
};

struct ipcshm_data_hdr {
	unsigned int magic_word;
	unsigned char data_free;
	unsigned char reserved[3]; /* reserved */
	struct pkt_header pkt;
};

#endif
