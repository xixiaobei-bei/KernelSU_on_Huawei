/*
 * rdr_lpm3.h
 *
 * header for rdr lpm3
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __RDR_LPM3_H__
#define __RDR_LPM3_H__
#include <linux/tele_mntn.h>
/* lpm3 log cleartext -- head info */
#define PC_LO_PWR_DOWN	0x5CC55CC5
#define PC_HI_PWR_DOWN	0xC55CC55C
#define PC_LO_LOCK_TIMEOUT	0x5CC5DEDE
#define PC_HI_LOCK_TIMEOUT	0xC55CDEDE
#define CPU_CORE_NUM	8
#define PC_INFO_STR_MAX_LENGTH	90
#define M3_IPC_MSG_LEN	2
#define RDR_BUF_HEAD_SIZE	2
#define NVIC_TYPE_SIZE	8
#define SCB_TYPE_SIZE	12

/* lpm3 nvic define */
#define NVIC_TYPE_OFFSET	0x0100 /* E000E100 */
#define SCB_TYPE_OFFSET	0x0D00 /* E000ED00 */
#define CORE_DEBUG_TYPE_OFFSET	0x0DF0 /* E000EDF0 */
#define LPMCU_DDR_MEM_PART_PATH_LEN	48UL

struct nvic_type {
	/* Offset: 0x000 (R/W) Interrupt Set Enable Register */
	unsigned int iser[8];
	unsigned int reserved0[24];
	/* Offset: 0x080 (R/W) Interrupt Clear Enable Register */
	unsigned int icer[8];
	unsigned int rserved1[24];
	/* Offset: 0x100 (R/W) Interrupt Set Pending Register */
	unsigned int ispr[8];
	unsigned int reserved2[24];
	/* Offset: 0x180 (R/W) Interrupt Clear Pending Register */
	unsigned int icpr[8];
	unsigned int reserved3[24];
	/* Offset: 0x200 (R/W) Interrupt Active bit Register */
	unsigned int iabr[8];
	unsigned int reserved4[56];
	/* Offset: 0x300 (R/W) Interrupt Priority Register (8Bit wide) */
	unsigned int ip[240];
	unsigned int reserved5[644];
	/* Offset: 0xE00 (W) Software Trigger Interrupt Register */
	unsigned int stir;
};

struct scb_type {
	/* Offset: 0x000 (R) cpuid Base Register */
	unsigned int cpuid;
	/* Offset: 0x004 (R/W) Interrupt Control and State Register */
	unsigned int icsr;
	/* Offset: 0x008 (R/W) Vector Table Offset Register */
	unsigned int vtor;
	/*
	 * Offset: 0x00C (R/W) Application
	 * Interrupt and Reset Control Register
	 */
	unsigned int aircr;
	/* Offset: 0x010 (R/W) System Control Register */
	unsigned int scr;
	/* Offset: 0x014 (R/W) Configuration Control Register */
	unsigned int ccr;
	/*
	 * Offset: 0x018 (R/W) System Handlers
	 * Priority Registers (4-7, 8-11, 12-15)
	 */
	unsigned char shp[12];
	/* Offset: 0x024 (R/W) System Handler Control and State Register */
	unsigned int shcsr;
	/* Offset: 0x028 (R/W) Configurable Fault Status Register */
	unsigned int cfsr;
	/* Offset: 0x02C (R/W) HardFault Status Register */
	unsigned int hfsr;
	/* Offset: 0x030 (R/W) Debug Fault Status Register */
	unsigned int dfsr;
	/* Offset: 0x034 (R/W) MemManage Fault Address Register */
	unsigned int mmfar;
	/* Offset: 0x038 (R/W) BusFault Address Register */
	unsigned int bfar;
	/* Offset: 0x03C (R/W) Auxiliary Fault Status Register */
	unsigned int afsr;
	/* Offset: 0x040 (R) Processor Feature Register */
	unsigned int pfr[2];
	/* Offset: 0x048 (R) Debug Feature Register */
	unsigned int dfr;
	/* Offset: 0x04C (R) Auxiliary Feature Register */
	unsigned int adr;
	/* Offset: 0x050 (R) Memory Model Feature Register */
	unsigned int mmfr[4];
	/* Offset: 0x060 (R) Instruction Set Attributes Register */
	unsigned int isar[5];
	unsigned int reserved0[5];
	/* Offset: 0x088 (R/W) Coprocessor Access Control Register */
	unsigned int cpacr;
};

struct coredebug_type {
	/* Offset: 0x000 (R/W) Debug Halting Control and Status Register */
	unsigned int dhcsr;
	/* Offset: 0x004 (W) Debug Core Register Selector Register */
	unsigned int dcrsr;
	/* Offset: 0x008 (R/W) Debug Core Register Data Register */
	unsigned int dcrdr;
	/*
	 * Offset: 0x00C (R/W) Debug
	 * Exception and Monitor Control Register
	 */
	unsigned int demcr;
};

struct rdr_buf_head {
	unsigned int acore_pc[CPU_CORE_NUM * RDR_BUF_HEAD_SIZE];
#ifdef CONFIG_HISI_MNTN_SP
	unsigned int acore_ls0_sp[CPU_CORE_NUM * RDR_BUF_HEAD_SIZE];
	unsigned int acore_ls1_sp[CPU_CORE_NUM * RDR_BUF_HEAD_SIZE];
#endif
};

char *lpmcu_rdr_ddr_addr_get(void);
unsigned int rdr_lpm3_buf_len_get(void);
int rdr_lpm3_stat_dump(void);

#endif
