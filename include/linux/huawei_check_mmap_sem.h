/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: provide function to find out mmap_sem's owner.
 * Author: Gong Chen <gongchen4@huawei.com>
 * Create: 2020-11-11
 */
#ifndef _HUAWEI_MMAP_SEM_CHECK_H_
#define _HUAWEI_MMAP_SEM_CHECK_H_

#include <linux/rwsem.h>
#include <linux/types.h>

void check_mmap_sem(pid_t pid);

#ifdef CONFIG_DETECT_HUAWEI_MMAP_SEM_DBG
void mmap_sem_debug(const struct rw_semaphore *sem);
#endif

#endif
