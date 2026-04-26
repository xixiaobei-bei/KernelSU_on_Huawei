/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 */

#ifndef MEMORY_LAYOUT_H
#define MEMORY_LAYOUT_H

#include <mdrv_memory_layout.h>
#include <bsp_print.h>
#include <osl_types.h>

#define NAME_MAX_LEN 16

#undef THIS_MODU
#define THIS_MODU mod_memory
#define MEM_INFO(fmt, ...) bsp_info(fmt, ##__VA_ARGS__)
#define MEM_DEBUG(fmt, ...) bsp_debug(fmt, ##__VA_ARGS__)
#define MEM_ERR(fmt, ...) bsp_err(fmt, ##__VA_ARGS__)

typedef struct {
    u64 addr; /* 内存首地址 */
    u32 size; /* 内存大小 */
    char name[NAME_MAX_LEN]; /* 内存名称 */
}mem_node_info; /*lint !e959*/

typedef struct {
    unsigned int size; /* 内存信息总大小 */
    mem_node_info *memory_info; /* 内存详细信息 */
}mem_layout;

int bsp_memory_layout_init(void);

#endif
