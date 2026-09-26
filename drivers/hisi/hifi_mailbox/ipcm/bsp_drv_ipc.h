/*
 * bsp_drv_ipc.h
 *
 * ipc driver interfaces
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef __BSP_DRV_IPC_H__
#define __BSP_DRV_IPC_H__

#include <linux/io.h>

#include "mdrv_ipc_enum.h"

#define SIZE_4K 4096
#define BSP_IPC_BASE_ADDR (SOC_IPC_S_BASE_ADDR)
#define IPC_REG_SIZE (SIZE_4K)

#define IRQ_IPC0_S 252
#define IRQ_IPC1_S 253

#define BSP_REG_RD(uwAddr) (0)
#define BSP_REG_WR(uwAddr, uwValue)

#define SOC_IPC_CPU_INT_EN_ADDR(base, i) ((base) + (0x500 + (0x10 * (i))))
#define SOC_IPC_SEM_INT_MASK_ADDR(base, j) ((base) + (0x604 + (0x10 * (j))))
#define SOC_IPC_CPU_INT_CLR_ADDR(base, i) ((base) + (0x40C + (0x10 * (i))))
#define SOC_IPC_CPU_INT_EN_ADDR(base, i) ((base) + (0x500 + (0x10 * (i))))
#define SOC_IPC_CPU_INT_DIS_ADDR(base, i) ((base) + (0x504 + (0x10 * (i))))
#define SOC_IPC_CPU_INT_STAT_ADDR(base, i) ((base) + (0x408 + (0x10 * (i))))
#define SOC_IPC_CPU_RAW_INT_ADDR(base, i) ((base) + (0x400 + (0x10 * (i))))
#define SOC_IPC_CPU_INT_MASK_ADDR(base, i) ((base) + (0x404 + (0x10 * (i))))
#define SOC_IPC_SEM_INT_CLR_ADDR(base, j) ((base) + (0x60C + (0x10 * (j))))
#define SOC_IPC_HS_CTRL_ADDR(base, j, k) \
	((base) + (0x800 + (0x100 * (j)) + (0x8 * (k))))
#define SOC_IPC_SEM_INT_STAT_ADDR(base, j) ((base) + (0x608 + (0x10 * (j))))
#define SOC_IPC_S_BASE_ADDR 0xe0475000

#define UCOM_COMM_UINT32_MAX 0xffffffff
#define BSP_IPC_MAX_INT_NUM 32
#define IPC_MASK 0xFFFFFF0F

#define INT_LEV_IPC_CPU (IRQ_IPC0_S)
#define INT_LEV_IPC_SEM (IRQ_IPC1_S)

#define IVEC_TO_INUM(int_vec) ((int)(int_vec))

#define INT_VEC_IPC_SEM IVEC_TO_INUM(INT_LEV_IPC_SEM)
#define INTSRC_NUM 32

#define INT_VEC_IPC_CPU IVEC_TO_INUM(INT_LEV_IPC_CPU)

typedef int (*irq_handler)(uint32_t);

struct bsp_ipc_entry {
	irq_handler routine;
	uint32_t arg;
};

struct ipc_debug {
	uint32_t recv_int_core;
	uint32_t int_handle_times[INTSRC_NUM];
	uint32_t int_send_times[INTSRC_NUM];
	uint32_t sem_id;
	uint32_t sem_take_times[INTSRC_NUM];
	uint32_t sem_give_times[INTSRC_NUM];
};

int drv_ipc_int_init(void);

void drv_ipc_int_deinit(void);

int ipc_int_connect(enum ipc_int_lev level, irq_handler routine, uint32_t para);

int ipc_int_send(enum ipc_int_core dst_core, enum ipc_int_lev level);

#endif /* __BSP_DRV_IPC_H__ */
