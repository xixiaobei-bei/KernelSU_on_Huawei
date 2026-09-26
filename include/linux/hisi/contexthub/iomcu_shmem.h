/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Description: Contexthub share memory driver
 * Author: Huawei
 * Create: 2019-10-01
 */
#ifndef __HISI_IOMCU_SHMEM_H__
#define __HISI_IOMCU_SHMEM_H__

#include <linux/types.h>
#include <linux/hisi/contexthub/protocol_as.h>
#include <linux/hisi/contexthub/iomcu_ipc.h>
#include <iomcu_ddr_map.h>

int contexthub_shmem_init(void);
void iomcu_shm_exit(void);
unsigned int shmem_get_capacity(void);
int ipcshm_send(struct write_info *wr, struct pkt_header *pkt, struct read_info *rd);
void *ipcshm_get_data(const char *buf);
#endif
