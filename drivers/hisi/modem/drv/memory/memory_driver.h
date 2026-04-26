/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 */

#ifndef MEMORY_DRIVER_H
#define MEMORY_DRIVER_H

#include <mdrv_memory.h>
#include <bsp_print.h>

#ifdef __cplusplus
extern "C" {
#endif

#define mem_fatal(fmt, ...)      (bsp_fatal(fmt, ##__VA_ARGS__))
#define mem_err(fmt, ...)      (bsp_err(fmt, ##__VA_ARGS__))
#define mem_debug(fmt, ...)      (bsp_debug(fmt, ##__VA_ARGS__))


#define MEM_OK    0
#define MEM_ERROR (-1)

#define MEM_MGR_NAME_SIZE 32
typedef struct {
    const char name[MEM_MGR_NAME_SIZE];
    unsigned int size;
    unsigned int offset;
    unsigned int magic;
} mem_mgr_node_info_s;

typedef struct {
    mem_mgr_node_info_s *dts_info;
    phy_addr_t base_addr;
    void *virt_base_addr;
    unsigned int size;
    unsigned int dts_info_size;
} mem_mgr_info_s;

int memory_init(void);

#ifdef __cplusplus
}
#endif

#endif
