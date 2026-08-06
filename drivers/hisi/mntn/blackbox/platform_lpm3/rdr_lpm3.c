/*
 * rdr_lpm3.c
 *
 * mntn module for lmp3
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
 *
 */

#include <linux/rdr_lpm3.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/semaphore.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/hisi/rdr_pub.h>
#include <linux/hisi/hisi_rproc.h>
#include <linux/hisi/ipc_msg.h>
#include <linux/hisi/util.h>
#include <linux/hisi/rdr_hisi_platform.h>

#include "../../mntn_filesys.h"
#include "../rdr_print.h"
#include "../rdr_inner.h"
#include "../rdr_field.h"

#include <soc_acpu_baseaddr_interface.h>
#include <soc_sctrl_interface.h>
#include <global_ddr_map.h>
#include <m3_rdr_ddr_map.h>
#include <lpmcu_runtime_save_user.h>
#include <lpmcu_core_mntn.h>
#include <securec.h>

typedef int (*cleartext_func_t)(const char *dir_path,
				struct file *fp,
				unsigned long long log_addr,
				unsigned int log_len);
enum rdr_lpm3_system_error_type {
	M3_WDT_TIMEOUT = HISI_BB_MOD_LPM_START,
	AP_WDT_TIMEOUT,
	SYSTEM_ERROR_TYPE_MAX = HISI_BB_MOD_LPM_END,
};

struct log_lpm3_cleartext {
	char *log_name;
	unsigned int log_addr_offset;
	unsigned int log_len;
	cleartext_func_t cleartext_func;
};

#define LPMCU_RESET_OFF_MODID_PANIC	0x1
#define LPMCU_RESET_OFF_MODID_WDT	0x2

#define IRQ_TRACE_LINESIZE		16
#define TASK_TRACE_LINESIZE		8
#define IRQ_TRACE_OFFSET		EXC_SPECIAL_SAVE_SIZE
#define TASK_TRACE_OFFSET		(IRQ_TRACE_OFFSET + INT_TRACK_SIZE)

#define PSCI_MSG_TYPE_M3_CTXSAVE \
	IPC_CMD(OBJ_AP, OBJ_LPM3, CMD_INQUIRY, 0)
#define PSCI_MSG_TYPE_M3_RDRBUF \
	IPC_CMD(OBJ_AP, OBJ_LPM3, CMD_INQUIRY, 2)
#define PSCI_MSG_TYPE_M3_PANIC_OFF \
	IPC_CMD(OBJ_AP, OBJ_LPM3, CMD_INQUIRY, 3)
#define PSCI_MSG_TYPE_M3_STAT_DUMP \
	IPC_CMD(OBJ_AP, OBJ_LPM3, CMD_INQUIRY, 4)
#define LPM3_RDR_SAVE_DONE \
	IPC_CMD(OBJ_LPM3, OBJ_AP, CMD_NOTIFY, TYPE_RESET)
#define LPM3_RDR_FILE_NAME	"LPM3.txt"

static u64 g_current_core_id = RDR_LPM3;
static struct rdr_register_module_result g_current_info;

static u64 g_rdr_lpm3_buf_addr;
static u32 g_rdr_lpm3_buf_len;

char *g_lpmcu_rdr_ddr_addr;
static struct semaphore g_rdr_lpm3_sem;
static volatile pfn_cb_dump_done g_pfn_cb_dumpdone;

static volatile u32 g_modid;

static char *g_sctrl_base;
static char g_lpmcu_ddr_memory_path[LOG_PATH_LEN + LPMCU_DDR_MEM_PART_PATH_LEN];
static struct notifier_block g_rdr_ipc_block;
static struct task_struct *g_rdr_lpm3_thread;
char *lpmcu_rdr_ddr_addr_get(void)
{
	return g_lpmcu_rdr_ddr_addr;
}

u32 rdr_lpm3_buf_len_get(void)
{
	return g_rdr_lpm3_buf_len;
}

static int system_reg_parse(const char *dir_path, struct file *fp,
			    u64 log_addr, u32 log_len);
static int head_info_parse(const char *dir_path, struct file *fp,
			   u64 log_addr, u32 log_len);
static int lpm3_exc_special_parse(const char *dir_path, struct file *fp,
				  u64 log_addr, u32 log_len);
static int lpm3_core_reg_parse(const char *dir_path, struct file *fp,
			       u64 log_addr, u32 log_len);
static int lpm3_nvic_reg_parse(const char *dir_path, struct file *fp,
			       u64 log_addr, u32 log_len);
static int lpm3_log_parse(const char *dir_path, struct file *fp,
			  u64 log_addr, u32 log_len);
static int lpmcu_runtime_parse(const char *dir_path, struct file *fp,
			       u64 log_addr, u32 log_len);
static int irq_trace_parse(const char *dir_path, struct file *fp,
			  u64 log_addr, u32 log_len);
static int task_trace_parse(const char *dir_path, struct file *fp,
			  u64 log_addr, u32 log_len);

static struct log_lpm3_cleartext g_lpm3_cleartext[] = {
	{
		"HEAD_INFO",
		M3_RDR_SYS_CONTEXT_HEAD_OFFSET,
		M3_RDR_SYS_CONTEXT_HEAD_SIZE,
		head_info_parse
	},
	{
		"LPM3_CORE_REGS",
		M3_RDR_SYS_CONTEXT_M3_COREREG_OFFSET,
		M3_RDR_SYS_CONTEXT_M3_COREREG_SIZE,
		lpm3_core_reg_parse
	},
	{
		"LPM3_EXC_SPECIAL",
		M3_RDR_SYS_CONTEXT_EXC_SPECIAL_OFFSET,
		M3_RDR_SYS_CONTEXT_EXC_SPECIAL_SIZE,
		lpm3_exc_special_parse
	},
	{
		"LPM3_NVIC_REGS",
		M3_RDR_SYS_CONTEXT_M3_NVICREG_OFFSET,
		M3_RDR_SYS_CONTEXT_M3_NVICREG_SIZE,
		lpm3_nvic_reg_parse
	},
	{
		"CRG_PERI_REGS",
		M3_RDR_CRG_PERI_OFFSET,
		M3_RDR_CRG_PERI_SIZE,
		system_reg_parse
	},
	{
		"SCTRL_REGS",
		M3_RDR_SCTRL_OFFSET,
		M3_RDR_SCTRL_SIZE,
		system_reg_parse
	},
	{
		"PMCTRL_REGS",
		M3_RDR_PMCTRL_OFFSET,
		M3_RDR_PMCTRL_SIZE,
		system_reg_parse
	},
	{
		"PCTRL_REGS",
		M3_RDR_PCTRL_OFFSET,
		M3_RDR_PCTRL_SIZE,
		system_reg_parse
	},
	{
		"RUNTIME_VAR_INFO",
		M3_RDR_SYS_CONTEXT_RUNTIME_VAR_OFFSET,
		M3_RDR_SYS_CONTEXT_RUNTIME_VAR_SIZE,
		lpmcu_runtime_parse
	},
	{
		"IRQ_TRACE",
		M3_RDR_SYS_CONTEXT_EXPOSE_STORAGE_OFFSET + IRQ_TRACE_OFFSET,
		INT_TRACK_SIZE,
		irq_trace_parse
	},
	{
		"TASK_TRACE",
		M3_RDR_SYS_CONTEXT_EXPOSE_STORAGE_OFFSET + TASK_TRACE_OFFSET,
		TASK_TRACK_SIZE,
		task_trace_parse
	},
	{
		"LPM3_LOG",
		M3_RDR_SYS_CONTEXT_M3_LOG_OFFSET,
		M3_RDR_SYS_CONTEXT_M3_LOG_SIZE,
		lpm3_log_parse
	},
};

/* ap address -> m3 address */
static u64 address_map(u64 orig_addr)
{
	u64 mapped_addr = orig_addr;
	return mapped_addr;
}

static void fn_dump(u32 modid, u32 etype, u64 coreid, char *pathname,
		    pfn_cb_dump_done pfn_cb)
{
	s32 ret;
	u32 msg[M3_IPC_MSG_LEN] = {PSCI_MSG_TYPE_M3_CTXSAVE, 0};

	BB_PRINT_PN("modid:0x%x,etype:0x%x,coreid:0x%llx,%s,pfn_cb:%pK\n",
		    modid, etype, coreid, pathname, pfn_cb);
	msg[1] = modid;
	g_pfn_cb_dumpdone = pfn_cb;
	g_modid = modid;
	ret = strncpy_s(g_lpmcu_ddr_memory_path, LOG_PATH_LEN, pathname,
			LOG_PATH_LEN - 1UL);
	if (ret != EOK)
		BB_PRINT_ERR("%s:strncpy_s failed:%d", __func__, ret);
	ret = RPROC_ASYNC_SEND(HISI_ACPU_LPM3_MBX_5, (mbox_msg_t *)msg,
			       M3_IPC_MSG_LEN);
	if (ret != 0)
		BB_PRINT_ERR("%s:RPROC_ASYNC_SEND failed! return (0x%x)\n",
			     __func__, ret);
	BB_PRINT_PN("%s end\n", __func__);
}

static void fn_reset(u32 modid, u32 etype, u64 coreid)
{
	(void)modid;
	(void)etype;
	(void)coreid;
}

static int rdr_lpm3_msg_handler(struct notifier_block *nb,
				unsigned long len, void *msg)
{
	u32 *_msg = msg;

	(void)nb;
	(void)len;
	BB_PRINT_PN("%s, [lpm3] -> [ap]: 0x%x\n", __func__, _msg[0]);
	if (_msg[0] == LPM3_RDR_SAVE_DONE) {
		up(&g_rdr_lpm3_sem);
		BB_PRINT_PN("%s lpm3 tell me that its sys context has saved\n",
			    __func__);
	}
	return 0;
}

/* work for rdr lpm3 */
static int rdr_lpm3_thread_body(void *arg)
{
	s32 ret;
	char *lpmcu_ddr_base = NULL;
	u32 base_addr;

	(void)arg;
	base_addr = HISI_RESERVED_LPMX_CORE_PHYMEM_BASE_UNIQUE;
	lpmcu_ddr_base = (char *)ioremap((phys_addr_t)base_addr, //lint !e446
					 HISI_RESERVED_LPMX_CORE_PHYMEM_SIZE);
	if (lpmcu_ddr_base == NULL)
		return -EINVAL;
	while (1) {
		if (down_interruptible(&g_rdr_lpm3_sem) != 0) {
			iounmap(lpmcu_ddr_base);
			return -EINVAL;
		}
		BB_PRINT_PN(" %s %d g_pfn_cb_dumpdone:%pK\n", __func__,
			    __LINE__, g_pfn_cb_dumpdone);
		if (g_pfn_cb_dumpdone != NULL) {
			ret = strncat_s(g_lpmcu_ddr_memory_path,
					LPMCU_DDR_MEM_PART_PATH_LEN,
					"/lpmcu_log/lpmcu_ddr_memory.bin",
					sizeof("/lpmcu_log/lpmcu_ddr_memory.bin"));
			if (ret != EOK)
				BB_PRINT_ERR("%s:strncpy_s failed:%d",
					     __func__, ret);
			mntn_filesys_write_log(g_lpmcu_ddr_memory_path,
					       lpmcu_ddr_base,
					       HISI_RESERVED_LPMX_CORE_PHYMEM_SIZE, 0);
			g_pfn_cb_dumpdone(g_modid, g_current_core_id);
			g_pfn_cb_dumpdone = NULL;
			BB_PRINT_PN("modid:0x%x,coreid:0x%llx\n", g_modid,
				    g_current_core_id);
		}
	}
}

static int rdr_lpm3_reset_off(int mod, int sw)
{
	s32 ret;
	unsigned int msg[M3_IPC_MSG_LEN] = {0};
	u32 value;

	if (mod == LPMCU_RESET_OFF_MODID_PANIC) {
		msg[0] = PSCI_MSG_TYPE_M3_PANIC_OFF;
		msg[1] = (unsigned int)sw;

		ret = RPROC_ASYNC_SEND(HISI_ACPU_LPM3_MBX_5,
				       (mbox_msg_t *)msg, M3_IPC_MSG_LEN);
		if (ret != 0) {
			BB_PRINT_ERR("RPROC_ASYNC_SEND failed! return 0x%x, &msg:(%pK)\n",
				     ret, msg);
			return ret;
		}
		BB_PRINT_PN("%s: (ap)->(lpm3) ipc send (0x%x 0x%x)!\n",
			    __func__, msg[0], msg[1]);
	} else if (mod == LPMCU_RESET_OFF_MODID_WDT) {
		if (g_sctrl_base != NULL) {
			value = (unsigned int)readl(SOC_SCTRL_SCBAKDATA10_ADDR(g_sctrl_base));
			if (sw != 0) {
				/* sctrl value parse */
				value &= ~(1U << 2);
			} else {
				/* sctrl value parse */
				value |= (1U << 2);
			}
			writel(value,
			       (void *)(SOC_SCTRL_SCBAKDATA10_ADDR(g_sctrl_base)));
		}
	}
	return 0;
}

int rdr_lpm3_stat_dump(void)
{
	s32 ret;
	unsigned int msg = PSCI_MSG_TYPE_M3_STAT_DUMP;

	ret = RPROC_ASYNC_SEND(HISI_ACPU_LPM3_MBX_5, (mbox_msg_t *)&msg, 1);
	if (ret != 0) {
		BB_PRINT_ERR("RPROC_ASYNC_SEND failed! return 0x%x, msg:(0x%x)\n",
			     ret, msg);
		return ret;
	}
	msleep(20);
	return 0;
}

static int system_reg_parse(const char *dir_path, struct file *fp,
			    u64 log_addr, u32 log_len)
{
	u32 i;
	bool error = false;

	(void)dir_path;
	if (IS_ERR_OR_NULL(fp)) {
		BB_PRINT_ERR("%s() error:fp 0x%pK.\n", __func__, fp);
		return -EINVAL;
	}

	rdr_cleartext_print(fp, &error, "offset      val\n");
	/* register address interval is 4 */
	for (i = 0; i < log_len; i = i + 4)
		rdr_cleartext_print(fp, &error, "0x%03x       0x%08x\n", i,
				    *((u32 volatile *)(uintptr_t)(log_addr + i)));
	if (error)
		return -EINVAL;

	return 0;
}

static int irq_trace_parse(const char *dir_path, struct file *fp,
			   u64 log_addr, u32 log_len)
{
	u32 i;
	bool error = false;
	u64 print_addr;

	(void)dir_path;
	if (IS_ERR_OR_NULL(fp)) {
		BB_PRINT_ERR("%s() error:fp 0x%pK.\n", __func__, fp);
		return -EINVAL;
	}
	/* print irq_num & time */
	rdr_cleartext_print(fp, &error, "num  irq_in        time  irq_out        time\n");
	for (i = 0; i < log_len / IRQ_TRACE_LINESIZE; i++) {
		print_addr = log_addr + i * IRQ_TRACE_LINESIZE;
		rdr_cleartext_print(fp, &error, "%02u:     %03u",
				    i + 1, *((u8 volatile *)(uintptr_t)print_addr));
		rdr_cleartext_print(fp, &error, "  0x%08X",
				    *((u32 volatile *)(uintptr_t)(print_addr + 4)));
		rdr_cleartext_print(fp, &error, "      %03u",
				    *((u8 volatile *)(uintptr_t)(print_addr + 8)));
		rdr_cleartext_print(fp, &error, "  0x%08X\n",
				    *((u32 volatile *)(uintptr_t)(print_addr + 12)));
	}
	if (error)
		return -EINVAL;

	return 0;
}

static int task_trace_parse(const char *dir_path, struct file *fp,
			   u64 log_addr, u32 log_len)
{
	u32 i;
	bool error = false;
	u64 print_addr;

	(void)dir_path;
	if (IS_ERR_OR_NULL(fp)) {
		BB_PRINT_ERR("%s() error:fp 0x%pK.\n", __func__, fp);
		return -EINVAL;
	}
	/* print task_id & time */
	rdr_cleartext_print(fp, &error, "num  task_in  task_out       time\n");
	for (i = 0; i < log_len / TASK_TRACE_LINESIZE; i++) {
		print_addr = log_addr + i * TASK_TRACE_LINESIZE;
		rdr_cleartext_print(fp, &error, "%02u:       %02u       %02u",
				    i + 1,
				    *((u8 volatile *)(uintptr_t)print_addr),
				    *((u8 volatile *)(uintptr_t)(print_addr + 2)));
		rdr_cleartext_print(fp, &error, "  0x%08X\n",
				    *((u32 volatile *)(uintptr_t)(print_addr + 4)));
	}
	if (error)
		return -EINVAL;

	return 0;
}

static int head_info_parse(const char *dir_path, struct file *fp,
			   u64 log_addr, u32 log_len)
{
	u32 cpu_idx;
	s32 ret;
	bool error = false;
	u32 data_low, data_high;
	s8 pc_info[PC_INFO_STR_MAX_LENGTH];
	struct rdr_buf_head *head = NULL;

	(void)dir_path;
	(void)log_len;
	if (IS_ERR_OR_NULL(fp)) {
		BB_PRINT_ERR("%s() error:fp 0x%pK.\n", __func__, fp);
		return -EINVAL;
	}

	head = (struct rdr_buf_head *)(uintptr_t)log_addr;
	for (cpu_idx = 0; cpu_idx < CPU_CORE_NUM; cpu_idx++) {
		pc_info[PC_INFO_STR_MAX_LENGTH - 1] = '\0';
		data_low = head->acore_pc[RDR_BUF_HEAD_SIZE * cpu_idx];
		BB_PRINT_PN("lpm3 cleartest head_info low: 0x%x.\n", data_low);
		data_high = head->acore_pc[RDR_BUF_HEAD_SIZE * cpu_idx + 1];
		BB_PRINT_PN("lpm3 cleartest head_info high: 0x%x.\n", data_high);
		if (data_low == PC_LO_PWR_DOWN && data_high == PC_HI_PWR_DOWN) {
			ret = snprintf_s(pc_info, PC_INFO_STR_MAX_LENGTH,
					 PC_INFO_STR_MAX_LENGTH - 1,
					 "cpu%u: %s\n", cpu_idx, "PWR_DOWN");
		} else if (data_low == PC_LO_LOCK_TIMEOUT &&
			   data_high == PC_HI_LOCK_TIMEOUT) {
			ret = snprintf_s(pc_info,
					 PC_INFO_STR_MAX_LENGTH,
					 PC_INFO_STR_MAX_LENGTH - 1,
					 "cpu%u: %s\n",
					 cpu_idx, "LOCK_TIMEOUT");
		} else {
			ret = snprintf_s(pc_info, PC_INFO_STR_MAX_LENGTH,
					 PC_INFO_STR_MAX_LENGTH - 1,
					 "cpu%u PC: 0x%08x%08x\n",
					 cpu_idx,
					 data_high, data_low);
		}

		if (ret < 0) {
			BB_PRINT_ERR("[%s], snprintf_s ret %d!\n",
				     __func__, ret);
			return -EINVAL;
		}
		rdr_cleartext_print(fp, &error, pc_info);
	}

	if (error)
		return -EINVAL;

	return 0;
}

static int lpm3_exc_special_parse(const char *dir_path, struct file *fp,
				  u64 log_addr, u32 log_len)
{
	u32 reason_idx;
	bool error = false;
	struct exc_spec_data *p_exc = NULL;

	(void)dir_path;
	(void)log_len;
	if (IS_ERR_OR_NULL(fp)) {
		BB_PRINT_ERR("%s() error:fp 0x%pK.\n", __func__, fp);
		return -EINVAL;
	}
	p_exc = (struct exc_spec_data *)(uintptr_t)log_addr;
	rdr_cleartext_print(fp, &error, "\n");
	for (reason_idx = 0; reason_idx < RDR_BACKUP_IDEX_MAX; reason_idx++)
		rdr_cleartext_print(fp, &error, "reset_reason[%u]   0x%x\n",
				    reason_idx,
				    p_exc->reset_reason[reason_idx]);

	rdr_cleartext_print(fp, &error, "slice      0x%x\n", p_exc->slice);
	rdr_cleartext_print(fp, &error, "rtc        0x%x\n", p_exc->rtc);
	rdr_cleartext_print(fp, &error, "R13        0x%x\n", p_exc->r13);
	rdr_cleartext_print(fp, &error, "LR1        0x%x\n", p_exc->lr1);
	rdr_cleartext_print(fp, &error, "PC         0x%x\n", p_exc->pc);
	rdr_cleartext_print(fp, &error, "XPSR       0x%x\n", p_exc->xpsr);
	rdr_cleartext_print(fp, &error, "CFSR       0x%x\n", p_exc->cfsr);
	rdr_cleartext_print(fp, &error, "HFSR       0x%x\n", p_exc->hfsr);
	rdr_cleartext_print(fp, &error, "BFAR       0x%x\n", p_exc->bfar);
	rdr_cleartext_print(fp, &error, "exc_trace  0x%x\n", p_exc->exc_trace);
	rdr_cleartext_print(fp, &error, "ddr_exc    0x%x\n", p_exc->ddr_exc);
	rdr_cleartext_print(fp, &error, "task_id    0x%x\n", p_exc->task_id);

	if (error)
		return -EINVAL;

	return 0;
}

static void lpm3_core_reg_print(struct file *fp,
				struct rdr_lpmcu_cregs *p,
				u32 reason_idx)
{
	bool error = false;
	rdr_cleartext_print(fp, &error, "****REGION %u****\n", reason_idx);
	rdr_cleartext_print(fp, &error, "R0:        0x%x\n", p->r0);
	rdr_cleartext_print(fp, &error, "R1:        0x%x\n", p->r1);
	rdr_cleartext_print(fp, &error, "R2:        0x%x\n", p->r2);
	rdr_cleartext_print(fp, &error, "R3:        0x%x\n", p->r3);
	rdr_cleartext_print(fp, &error, "R4:        0x%x\n", p->r4);
	rdr_cleartext_print(fp, &error, "R5:        0x%x\n", p->r5);
	rdr_cleartext_print(fp, &error, "R6:        0x%x\n", p->r6);
	rdr_cleartext_print(fp, &error, "R7:        0x%x\n", p->r7);
	rdr_cleartext_print(fp, &error, "R8:        0x%x\n", p->r8);
	rdr_cleartext_print(fp, &error, "R9:        0x%x\n", p->r9);
	rdr_cleartext_print(fp, &error, "R10:       0x%x\n", p->r10);
	rdr_cleartext_print(fp, &error, "R11:       0x%x\n", p->r11);
	rdr_cleartext_print(fp, &error, "R12:       0x%x\n", p->r12);
	rdr_cleartext_print(fp, &error, "R13:       0x%x\n", p->r13);
	rdr_cleartext_print(fp, &error, "MSP:       0x%x\n", p->sp);
	rdr_cleartext_print(fp, &error, "PSP:       0x%x\n", p->psp);
	rdr_cleartext_print(fp, &error, "LR0:       0x%x\n", p->lr0);
	rdr_cleartext_print(fp, &error, "LR1:       0x%x\n", p->lr1);
	rdr_cleartext_print(fp, &error, "PC:        0x%x\n", p->pc);
	rdr_cleartext_print(fp, &error, "XPSR:      0x%x\n", p->xpsr);
	rdr_cleartext_print(fp, &error, "PRIMASK:   0x%x\n", p->primask);
	rdr_cleartext_print(fp, &error, "BASEPRI:   0x%x\n", p->basepri);
	rdr_cleartext_print(fp, &error, "FAULTMASK: 0x%x\n", p->faultmask);
	rdr_cleartext_print(fp, &error, "CONTROL:   0x%x\n", p->control);
	rdr_cleartext_print(fp, &error, "\n");
}

static int lpm3_core_reg_parse(const char *dir_path, struct file *fp,
			       u64 log_addr, u32 log_len)
{
	u32 idx;
	bool error = false;
	struct rdr_lpmcu_cregs *p_reg = NULL;
	u64 core_addr;

	(void)dir_path;
	(void)log_len;
	if (IS_ERR_OR_NULL(fp)) {
		BB_PRINT_ERR("%s() error:fp 0x%pK.\n", __func__, fp);
		return -EINVAL;
	}
	rdr_cleartext_print(fp, &error, "\n");
	for (idx = 0; idx < RDR_BACKUP_IDEX_MAX; idx++) {
		core_addr =
			log_addr + idx * sizeof(struct rdr_lpmcu_cregs);
		p_reg = (struct rdr_lpmcu_cregs *)(uintptr_t)core_addr;
		lpm3_core_reg_print(fp, p_reg, idx);
	}
	if (error)
		return -EINVAL;
	return 0;
}

static int lpm3_nvic_reg_parse(const char *dir_path, struct file *fp,
			       u64 log_addr, u32 log_len)
{
	u32 i;
	bool error = false;
	struct nvic_type *p_lpm3_nvic_reg = NULL;
	struct scb_type *p_lpm3_scb_reg = NULL;
	struct coredebug_type *p_lpm3_core_debug_reg = NULL;

	(void)dir_path;
	(void)log_len;
	if (IS_ERR_OR_NULL(fp)) {
		BB_PRINT_ERR("%s() error:fp 0x%pK.\n", __func__, fp);
		return -EINVAL;
	}
	p_lpm3_nvic_reg =
		(struct nvic_type *)(uintptr_t)(log_addr + NVIC_TYPE_OFFSET);
	p_lpm3_scb_reg =
		(struct scb_type *)(uintptr_t)(log_addr + SCB_TYPE_OFFSET);
	p_lpm3_core_debug_reg =
		(struct coredebug_type *)(uintptr_t)(log_addr + CORE_DEBUG_TYPE_OFFSET);
	rdr_cleartext_print(fp, &error, "\n");
	for (i = 0; i < NVIC_TYPE_SIZE; i++)
		rdr_cleartext_print(fp, &error, "setena%u    0x%x\n", i,
				    p_lpm3_nvic_reg->iser[i]);
	for (i = 0; i < NVIC_TYPE_SIZE; i++)
		rdr_cleartext_print(fp, &error, "clrena%u    0x%x\n", i,
				    p_lpm3_nvic_reg->icer[i]);
	for (i = 0; i < NVIC_TYPE_SIZE; i++)
		rdr_cleartext_print(fp, &error, "setpend%u   0x%x\n", i,
				    p_lpm3_nvic_reg->ispr[i]);
	for (i = 0; i < NVIC_TYPE_SIZE; i++)
		rdr_cleartext_print(fp, &error, "clrpend%u   0x%x\n", i,
				    p_lpm3_nvic_reg->icpr[i]);
	for (i = 0; i < NVIC_TYPE_SIZE; i++)
		rdr_cleartext_print(fp, &error, "active%u    0x%x\n", i,
				    p_lpm3_nvic_reg->iabr[i]);
	rdr_cleartext_print(fp, &error, "cpuid      0x%x\n",
			    p_lpm3_scb_reg->cpuid);
	rdr_cleartext_print(fp, &error, "icsr       0x%x\n",
			    p_lpm3_scb_reg->icsr);
	rdr_cleartext_print(fp, &error, "vtor       0x%x\n",
			    p_lpm3_scb_reg->vtor);
	rdr_cleartext_print(fp, &error, "aircr      0x%x\n",
			    p_lpm3_scb_reg->aircr);
	rdr_cleartext_print(fp, &error, "scr        0x%x\n",
			    p_lpm3_scb_reg->scr);
	rdr_cleartext_print(fp, &error, "ccr        0x%x\n",
			    p_lpm3_scb_reg->ccr);
	for (i = 0; i < SCB_TYPE_SIZE; i++)
		/* pri start index is 4 */
		rdr_cleartext_print(fp, &error, "PRI_%02d     0x%x\n",
				    (i + 4),
				    p_lpm3_scb_reg->shp[i]);
	rdr_cleartext_print(fp, &error, "shcsr      0x%x\n",
			    p_lpm3_scb_reg->shcsr);
	rdr_cleartext_print(fp, &error, "cfsr       0x%x\n",
			    p_lpm3_scb_reg->cfsr);
	rdr_cleartext_print(fp, &error, "hfsr       0x%x\n",
			    p_lpm3_scb_reg->hfsr);
	rdr_cleartext_print(fp, &error, "dfsr       0x%x\n",
			    p_lpm3_scb_reg->dfsr);
	rdr_cleartext_print(fp, &error, "mmfar      0x%x\n",
			    p_lpm3_scb_reg->mmfar);
	rdr_cleartext_print(fp, &error, "bfar       0x%x\n",
			    p_lpm3_scb_reg->bfar);
	rdr_cleartext_print(fp, &error, "afsr       0x%x\n",
			    p_lpm3_scb_reg->afsr);
	rdr_cleartext_print(fp, &error, "dhcsr      0x%x\n",
			    p_lpm3_core_debug_reg->dhcsr);
	rdr_cleartext_print(fp, &error, "dcrsr      0x%x\n",
			    p_lpm3_core_debug_reg->dcrsr);
	rdr_cleartext_print(fp, &error, "dcrdr      0x%x\n",
			    p_lpm3_core_debug_reg->dcrdr);
	rdr_cleartext_print(fp, &error, "demcr      0x%x\n",
			    p_lpm3_core_debug_reg->demcr);
	rdr_cleartext_print(fp, &error, "\n");
	if (error)
		return -EINVAL;
	return 0;
}

static int lpm3_log_parse(const char *dir_path, struct file *fp,
			  u64 log_addr, u32 log_len)
{
	char *log_buff = NULL;
	u32 log_size = log_len + 1;
	s32 ret = 0;

	(void)dir_path;
	if (IS_ERR_OR_NULL(fp)) {
		BB_PRINT_ERR("%s() error:fp 0x%pK.\n", __func__, fp);
		return -EINVAL;
	}

	log_buff = (char *)kzalloc(log_size, GFP_KERNEL);
	if (log_buff == NULL) {
		BB_PRINT_ERR("kmalloc fail for lpm3_log\n");
		return -EINVAL;
	}

	if (memcpy_s((void *)log_buff, log_size,
		     (void *)(uintptr_t)log_addr, log_len) != EOK) {
		BB_PRINT_ERR("memcpy fail for lpm3_log\n");
		ret = -EINVAL;
		goto out;
	}

	ret = vfs_write(fp, log_buff, (size_t)log_len, &fp->f_pos);
	if (ret != log_len) {
		BB_PRINT_ERR("%s():write file exception with ret %d.\n",
			     __func__, ret);
		ret = -EINVAL;
		goto out;
	}
out:
	kfree(log_buff);

	return ret;
}

static int lpmcu_runtime_parse(const char *dir_path, struct file *fp,
			       u64 log_addr, u32 log_len)
{
	u32 i;
	u32 offset;
	u32 runtime_offset;
	bool error = false;
	u64 log_offset;
	u64 size;

	(void)dir_path;
	if (IS_ERR_OR_NULL(fp)) {
		BB_PRINT_ERR("%s() error:fp 0x%pK.\n", __func__, fp);
		return -EINVAL;
	}

	rdr_cleartext_print(fp, &error, "offset      val\n");
	for (i = 0; i < ARRAY_SIZE(g_rdr_runtime_save_info); i++) {
		runtime_offset = g_rdr_runtime_save_info[i].offset;
		size = g_rdr_runtime_save_info[i].size;
		/* register 4 byte  */
		for (offset = 0;
		     offset < size && (offset + runtime_offset) < log_len;
		     offset = offset + 4) {
			log_offset = log_addr + runtime_offset + offset;
			rdr_cleartext_print(fp, &error, "0x%03x       0x%08x\n",
					    (runtime_offset + offset),
					    *((u32 volatile *)(uintptr_t)log_offset)); //lint !e539
		}
	}

	if (error)
		return -EINVAL;

	return 0;
}

static int rdr_lpm3_cleartext_print(const char *dir_path,
				    u64 log_addr, u32 log_len)
{
	bool error = false;
	u32 idx;
	u64 log_addr_parse;
	u32 log_len_parse;
	s32 ret = 0;
	struct file *fp = NULL;

	fp = bbox_cleartext_get_filep(dir_path, LPM3_RDR_FILE_NAME);
	if (IS_ERR_OR_NULL(fp)) {
		BB_PRINT_ERR("%s() error:fp 0x%pK.\n", __func__, fp);
		return -EINVAL;
	}
	if (IS_ERR_OR_NULL(dir_path) ||
	    IS_ERR_OR_NULL((void *)(uintptr_t)log_addr)) {
		BB_PRINT_ERR("%s() error:dir_path 0x%pK log_addr 0x%llx.\n",
			     __func__, dir_path, log_addr);
		ret = -EINVAL;
		goto out;
	}
	if (log_len != M3_RDR_SYS_CONTEXT_SIZE) {
		BB_PRINT_ERR("log_len error:0x%x.\n", log_len);
		ret = -EINVAL;
		goto out;
	}

	for (idx = 0;
	     idx < ARRAY_SIZE(g_lpm3_cleartext);
	     idx++) {
		log_addr_parse = log_addr +
				 g_lpm3_cleartext[idx].log_addr_offset;
		log_len_parse = g_lpm3_cleartext[idx].log_len;
		rdr_cleartext_print(fp, &error,
				    "=================%s START================\n\n",
				    g_lpm3_cleartext[idx].log_name);
		ret = g_lpm3_cleartext[idx].cleartext_func(dir_path, fp,
							   log_addr_parse,
							   log_len_parse);
		rdr_cleartext_print(fp, &error,
				    "=================%s END================\n\n",
				    g_lpm3_cleartext[idx].log_name);
	}
out:
	bbox_cleartext_end_filep(fp, dir_path, LPM3_RDR_FILE_NAME);
	return ret;
}

static int einfo_set(struct rdr_exception_info_s *einfo)
{
	s32 ret;

	if (einfo == NULL) {
		return -EINVAL;
	}
	einfo->e_modid = (unsigned int)M3_WDT_TIMEOUT;
	einfo->e_modid_end = (unsigned int)M3_WDT_TIMEOUT;
	einfo->e_process_priority = RDR_ERR;
	einfo->e_reboot_priority = RDR_REBOOT_WAIT;
	einfo->e_notify_core_mask = RDR_AP | RDR_LPM3;
	einfo->e_reset_core_mask = RDR_LPM3;
	einfo->e_reentrant = (unsigned int)RDR_REENTRANT_DISALLOW;
	einfo->e_exce_type = LPM3_S_EXCEPTION;
	einfo->e_from_core = RDR_LPM3;
	ret = memcpy_s(einfo->e_from_module,
		       sizeof(einfo->e_from_module),
		       "RDR M3 WDT",
		       sizeof("RDR M3 WDT"));
	if (ret != EOK) {
		BB_PRINT_ERR("%s:%d, memcpy_s err:%d\n",
			     __func__, __LINE__, ret);
		return ret;
	}
	ret = memcpy_s(einfo->e_desc, sizeof(einfo->e_desc), "RDR M3 WDT",
		       sizeof("RDR M3 WDT"));
	if (ret != EOK) {
		BB_PRINT_ERR("%s:%d, memcpy_s err:%d\n",
			     __func__, __LINE__, ret);
		return ret;
	}
	return 0;
}

static void rdr_addr_send_lpm3(void)
{
	s32 ret;
	/* rdr addr ipc msg size is 4 */
	static u32 msg[4] = {0};

	BB_PRINT_PN("%s: log_addr = 0x%llx, log_len = %u\n",
		    __func__, g_current_info.log_addr,
		    g_current_info.log_len);
	g_lpmcu_rdr_ddr_addr = (char *)hisi_bbox_map((phys_addr_t)g_current_info.log_addr,
						     g_current_info.log_len);
	if (g_lpmcu_rdr_ddr_addr != NULL) {
		BB_PRINT_PN("rdr_ddr_addr success\n");
		ret = memset_s(g_lpmcu_rdr_ddr_addr,
			       (size_t)g_rdr_lpm3_buf_len, 0,
			       (size_t)g_rdr_lpm3_buf_len);
		if (ret != EOK) {
			BB_PRINT_ERR("%s:%d, memset_s err:%d\n",
				     __func__, __LINE__, ret);
			return;
		}
	}

	msg[0] = PSCI_MSG_TYPE_M3_RDRBUF;
	msg[1] = (u32)g_rdr_lpm3_buf_addr;
	/* lpm3 rdr buf addr set */
	msg[2] = g_rdr_lpm3_buf_addr >> 32;
	msg[3] = g_rdr_lpm3_buf_len;
	/* send rdr info to lpm3 */
	ret = RPROC_ASYNC_SEND(HISI_ACPU_LPM3_MBX_5,
			       (mbox_msg_t *)msg, 4);
	if (ret != 0)
		BB_PRINT_ERR("RPROC_ASYNC_SEND failed! return 0x%x, &msg:(%pK)\n",
			     ret, msg);
	/* ipc info print */
	BB_PRINT_PN("%s: (ap)->(lpm3) ipc send (0x%x 0x%x 0x%x 0x%x)!\n",
		    __func__, msg[0], msg[1], msg[2], msg[3]);
}

static int __init rdr_lpm3_init(void)
{
	struct rdr_module_ops_pub s_module_ops;
	struct rdr_exception_info_s einfo;
	s32 ret;

	BB_PRINT_PN("enter %s\n", __func__);
	/* repmap size 0x1000UL */
	g_sctrl_base =
		(char *)ioremap((phys_addr_t)SOC_ACPU_SCTRL_BASE_ADDR, 0x1000UL); //lint !e446
	if (g_sctrl_base == NULL)
		return -EINVAL;
	BB_PRINT_PN("g_sctrl_base: %pK\n", g_sctrl_base);
	g_rdr_ipc_block.next = NULL;
	g_rdr_ipc_block.notifier_call = rdr_lpm3_msg_handler;
	ret = RPROC_MONITOR_REGISTER(HISI_LPM3_ACPU_MBX_2, &g_rdr_ipc_block);
	if (ret != 0) {
		iounmap(g_sctrl_base);
		g_sctrl_base = NULL;
		BB_PRINT_ERR("%s:RPROC_MONITOR_REGISTER failed", __func__);
		return ret;
	}
	sema_init(&g_rdr_lpm3_sem, 0);
	g_rdr_lpm3_thread =
		kthread_run(rdr_lpm3_thread_body, NULL, "rdr_lpm3_thread");
	if (g_rdr_lpm3_thread == NULL) {
		BB_PRINT_ERR("%s: create thread rdr_main_thread failed.\n",
			     __func__);
		iounmap(g_sctrl_base);
		g_sctrl_base = NULL;
		return -EINVAL;
	}
	s_module_ops.ops_dump = fn_dump;
	s_module_ops.ops_reset = fn_reset;
	ret = rdr_register_module_ops(g_current_core_id, &s_module_ops,
				      &g_current_info);
	if (ret != 0) {
		iounmap(g_sctrl_base);
		g_sctrl_base = NULL;
		kthread_stop(g_rdr_lpm3_thread);
		BB_PRINT_ERR("rdr_register_module_ops failed! return 0x%x\n",
			     ret);
		return ret;
	}
	ret = rdr_register_cleartext_ops(g_current_core_id,
					 rdr_lpm3_cleartext_print);
	if (ret < 0) {
		iounmap(g_sctrl_base);
		g_sctrl_base = NULL;
		kthread_stop(g_rdr_lpm3_thread);
		BB_PRINT_ERR("rdr_register_cleartext_ops failed! return 0x%x\n",
			     ret);
		return ret;
	}
	BB_PRINT_PN("rdr_register_cleartext_success\n");
	ret = memset_s(&einfo, sizeof(einfo), 0,
		       sizeof(struct rdr_exception_info_s));
	if (ret != EOK) {
		iounmap(g_sctrl_base);
		g_sctrl_base = NULL;
		kthread_stop(g_rdr_lpm3_thread);
		BB_PRINT_ERR("%s:%d, memset_s err:%d\n",
			     __func__, __LINE__, ret);
		return ret;
	}
	ret = einfo_set(&einfo);
	if (ret != 0) {
		iounmap(g_sctrl_base);
		g_sctrl_base = NULL;
		kthread_stop(g_rdr_lpm3_thread);
		return ret;
	}
	(void)rdr_register_exception(&einfo);
	g_rdr_lpm3_buf_addr = address_map(g_current_info.log_addr);
	g_rdr_lpm3_buf_len = g_current_info.log_len;
	rdr_addr_send_lpm3();
	(void)rdr_lpm3_reset_off(LPMCU_RESET_OFF_MODID_PANIC,
				 check_himntn(HIMNTN_LPM3_PANIC_INTO_LOOP));
	return ret;
}

static void __exit rdr_lpm3_exit(void)
{
	if (g_sctrl_base != NULL) {
		iounmap(g_sctrl_base);
		g_sctrl_base = NULL;
	}
	if (g_rdr_lpm3_thread != NULL)
		kthread_stop(g_rdr_lpm3_thread);
}

subsys_initcall(rdr_lpm3_init);
module_exit(rdr_lpm3_exit);
