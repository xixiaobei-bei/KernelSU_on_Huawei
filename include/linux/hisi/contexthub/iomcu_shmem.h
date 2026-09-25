/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Description: Contexthub share memory driver
 * Author: Huawei
 * Create: 2019-10-01
 */
#ifndef __HISI_IOMCU_SHMEM_H__
#define __HISI_IOMCU_SHMEM_H__

#include "protocol.h"
#include <linux/hisi/contexthub/iomcu_ipc.h>
#include <iomcu_ddr_map.h>
#include <linux/types.h>

extern int contexthub_shmem_init(void);
extern unsigned int shmem_get_capacity(void);
int ipcshm_send(struct write_info *wr, pkt_header_t *pkt,struct read_info *rd);
void *ipcshm_get_data(const char *buf);
#endif
