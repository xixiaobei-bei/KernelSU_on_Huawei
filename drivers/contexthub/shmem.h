/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Description: Contexthub share memory driver
 * Author: Huawei
 * Create: 2016-04-01
 */
#ifndef __HISI_CONTEXTHUB_SHMEM_H__
#define __HISI_CONTEXTHUB_SHMEM_H__
#include "protocol.h"

#include <iomcu_ddr_map.h>

#define HISI_RESERVED_CH_BLOCK_SHMEM_PHYMEM_BASE       DDR_SHMEM_LARGE_BLK_BASE_AP
#define HISI_RESERVED_CH_BLOCK_SHMEM_PHYMEM_SIZE       DDR_SHMEM_LARGE_BLK_SIZE

/*
 * sharemem消息发送接口，将数据复制到DDR中，并发送IPC通知contexthub处理
 */
extern int shmem_send(obj_tag_t module_id, const void *usr_buf, unsigned int usr_buf_size);

/*
 * Contexthub sharemem驱动初始化
 */
extern int contexthub_shmem_init(void);

/*
 * 将sharemem包中的数据从DDR中取出，如果是大数据包消息同时启动workqueue回复IPC确认
 */
extern const pkt_header_t *shmempack(const char *buf, unsigned int length);

/*
 * 获得sharemem数据发送的size上限
 */
extern unsigned int shmem_get_capacity(void);

/*
 * sharemem消息发送后，收到contexthub的回复确认后up信号量
 */
extern int shmem_send_resp(const pkt_header_t *head);

#endif
