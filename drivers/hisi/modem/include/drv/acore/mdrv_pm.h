/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 */

#ifndef MDRV_PM_H
#define MDRV_PM_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum pm_wake_src {
    PM_WAKE_SRC_UART0 = 0,
    PM_WAKE_SRC_SOCP,
    PM_WAKE_SRC_DRX_TIMER,
    PM_WAKE_SRC_BUT
} pm_wake_src_e;

typedef enum {
    DFS_QOS_ID_ACPU_MINFREQ_E = 0,
    DFS_QOS_ID_ACPU_MAXFREQ_E,
    DFS_QOS_ID_CCPU_MINFREQ_E,
    DFS_QOS_ID_CCPU_MAXFREQ_E,
    DFS_QOS_ID_DDR_MINFREQ_E,
    DFS_QOS_ID_DDR_MAXFREQ_E,
    DFS_QOS_ID_GPU_MINFREQ_E,
    DFS_QOS_ID_GPU_MAXFREQ_E,
    DFS_QOS_ID_BUS_MINFREQ_E,
    DFS_QOS_ID_BUS_MAXFREQ_E
} dfs_qos_id_e;

#ifdef __cplusplus
}
#endif
#endif
