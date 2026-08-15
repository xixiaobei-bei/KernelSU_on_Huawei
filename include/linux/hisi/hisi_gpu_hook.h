/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Description: This file describe GPU interface/callback
 * Author:
 * Create: 2019-2-24
 */
#ifndef HISI_GPU_HOOK_H
#define HISI_GPU_HOOK_H

#include <linux/types.h>

/* GPU AI frequency schedule interface */
struct kbase_fence_info {
        pid_t game_pid;
        u64 signaled_seqno;
};

#ifdef CONFIG_HISI_GPU_AI_FENCE_INFO
int mali_kbase_report_fence_info(struct kbase_fence_info *fence);
#else
static inline int mali_kbase_report_fence_info(struct kbase_fence_info *fence) { return -1; };
#endif

#endif /* HISI_GPU_HOOK_H */
