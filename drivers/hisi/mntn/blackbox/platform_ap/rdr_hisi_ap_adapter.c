/*
 *
 * Based on the RDR framework, adapt to the AP side to implement resource
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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
#include "rdr_hisi_ap_adapter.h"
#include "rdr_hisi_ap_exception_logsave.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/thread_info.h>
#include <linux/hardirq.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_fdt.h>
#include <linux/jiffies.h>
#include <linux/uaccess.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/kthread.h>
#include <linux/notifier.h>
#include <linux/delay.h>
#include <linux/syscalls.h>
#include <linux/preempt.h>
#include <asm/cacheflush.h>
#include <linux/kmsg_dump.h>
#include <linux/slab.h>
#include <linux/kdebug.h>
#include <linux/hisi/mntn_record_sp.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/stacktrace.h>
#include <linux/kallsyms.h>
#include <linux/blkdev.h>
#include <linux/hisi/mntn_dump.h>
#include <linux/hisi/util.h>
#include <linux/hisi/rdr_pub.h>
#include <linux/hisi/hisi_bootup_keypoint.h>
#include <linux/hisi/rdr_hisi_ap_ringbuffer.h>
#include "../rdr_inner.h"
#include "../../mntn_filesys.h"
#include "../../dump.h"
#include <linux/hisi/mntn_dump.h>
#include <linux/hisi/eeye_ftrace_pub.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/hisi/powerkey_event.h>
#include <linux/hisi/dfx_universal_wdt.h>
#include <linux/hisi/lpcpu_mntn_l3cache_ecc.h>
#include <securec.h>
#include <linux/version.h>
#include <linux/hisi/hisi_pstore.h>
#include <linux/hisi/hisi_bbox_diaginfo.h>
#include <linux/watchdog.h>
#include <mntn_subtype_exception.h>
#include "../rdr_print.h"
#include <pr_log.h>
#include <linux/hisi/hisi_hw_diag.h>
#include <linux/dma-direction.h>
#include <linux/hisi/mntn_dump.h>


#define PR_LOG_TAG BLACKBOX_TAG


static struct ap_eh_root *g_rdr_ap_root = NULL;
#ifdef CONFIG_PM
/* Test function declaration */
static struct ap_eh_root g_rdr_ap_root_bak;
#endif
static AP_RECORD_PC *g_bbox_ap_record_pc = NULL;
u64 g_hisiap_addr;
static char g_log_path[LOG_PATH_LEN];
static int g_rdr_ap_init;
static void __iomem *sctrl_map_addr = NULL;
static void __iomem *g_32k_timer_l32bit_addr = NULL;
static void __iomem *g_32k_timer_h32bit_addr = NULL;
static struct rdr_register_module_result g_current_info;
static struct mutex g_dump_mem_mutex;

static unsigned long long g_pmu_reset_reg;
static unsigned long long g_pmu_subtype_reg;
/* -1: Not initialized; 0 Non-fpga board; 1 fpga board */
int g_bbox_fpga_flag = -1;

static unsigned int g_dump_buffer_size_tbl[HK_MAX] = {0};
static unsigned int g_last_task_struct_size;
static unsigned int g_last_task_stack_size;
struct trace_buffer_percpu {
	int (*func)(struct percpu_buffer_info *buffer_info, unsigned char *addr, unsigned int size);
};

struct trace_buffer_single {
	int (*func)(unsigned char **addr, unsigned int size);
};

/* percpu array size must be equal to (HK_PERCPU_TAG - 1) */
struct trace_buffer_percpu g_trace_buffer_percpu[] = {
	{ irq_buffer_init },
	{ task_buffer_init },
	{ cpuidle_buffer_init },
	{ worker_buffer_init },
	{ mem_alloc_buffer_init },
	{ ion_alloc_buffer_init },
	{ time_buffer_init },
	{ net_tx_softirq_init },
	{ net_rx_softirq_init },
	{ block_softirq_init },
	{ sched_softirq_init },
	{ rcu_softirq_init },
};

/* single array size must be equal to (HK_MAX - HK_PERCPU_TAG) */
struct trace_buffer_single g_trace_buffer_single[] = {
	{ cpu_onoff_buffer_init },
	{ syscall_buffer_init },
	{ hung_task_buffer_init },
	{ tasklet_buffer_init },
	{ diaginfo_buffer_init },
	{ timeout_softirq_buffer_init },
};

static char g_trace_buffer_modu[HK_MAX][AMNTN_MODULE_COMP_LEN] = {
	[HK_IRQ] = { "ap_trace_irq_size" },
	[HK_TASK] = { "ap_trace_task_size" },
	[HK_CPUIDLE] = { "ap_trace_cpu_idle_size" },
	[HK_WORKER] = { "ap_trace_worker_size" },
	[HK_MEM_ALLOCATOR] = { "" },
	[HK_ION_ALLOCATOR] = { "" },
	[HK_TIME] = { "ap_trace_time_size" },
	[HK_NET_TX_SOFTIRQ] = { "ap_trace_net_tx_softirq_size" },
	[HK_NET_RX_SOFTIRQ] = { "ap_trace_net_rx_softirq_size" },
	[HK_BLOCK_SOFTIRQ] = { "ap_trace_block_softirq_size" },
	[HK_SCHED_SOFTIRQ] = { "ap_trace_sched_softirq_size" },
	[HK_RCU_SOFTIRQ] = { "ap_trace_rcu_softirq_size" },
	[HK_CPU_ONOFF] = { "ap_trace_cpu_on_off_size" },
	[HK_SYSCALL] = { "ap_trace_syscall_size" },
	[HK_HUNGTASK] = { "ap_trace_hung_task_size" },
	[HK_TASKLET] = { "ap_trace_tasklet_size" },
	[HK_DIAGINFO] = { "ap_trace_diaginfo_size" },
	[HK_TIMEOUT_SOFTIRQ] = { "ap_trace_timeout_softirq_size" },
};

static struct log_buf_dump_info *g_logbuff_dump_info = NULL;

int decode_log_buff(char *dst, size_t size, const char *src);

u64 get_hisiap_addr(void)
{
	return g_hisiap_addr;
}
/*
 * struct rdr_exception_info_s {
 *  struct list_head e_list;
 *  u32 e_modid;
 *  u32 e_modid_end;
 *  u32 e_process_priority;
 *  u32 e_reboot_priority;
 *  u64 e_notify_core_mask;
 *  u64 e_reset_core_mask;
 *  u64 e_from_core;
 *  u32 e_reentrant;
 *  u32 e_exce_type;
 *  u32 e_upload_flag;
 *  u8  e_from_module[MODULE_NAME_LEN];
 *  u8  e_desc[STR_EXCEPTIONDESC_MAXLEN];
 *  u32 e_reserve_u32;
 *  void*   e_reserve_p;
 *  rdr_e_callback e_callback;
 * };
 */
struct rdr_exception_info_s g_einfo[] = {
	{ { 0, 0 }, MODID_AP_S_PANIC, MODID_AP_S_PANIC, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_PANIC, HI_APPANIC_RESERVED, (u32)RDR_UPLOAD_YES, "ap", "ap",
	 0, 0, 0 },
	{ { 0, 0 }, MODID_AP_S_PANIC_GPU, MODID_AP_S_PANIC_GPU, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_PANIC, HI_APPANIC_GPU, (u32)RDR_UPLOAD_YES, "ap", "ap",
	 0, 0, 0 },
	{ { 0, 0 }, MODID_AP_S_L3CACHE_ECC1, MODID_AP_S_L3CACHE_ECC1, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_PANIC, HI_APPANIC_L3CACHE_ECC1, (u32)RDR_UPLOAD_YES, "ap", "ap",
	 0, 0, 0 },
	{ { 0, 0 }, MODID_AP_S_L3CACHE_ECC2, MODID_AP_S_L3CACHE_ECC2, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_PANIC, HI_APPANIC_L3CACHE_ECC2, (u32)RDR_UPLOAD_YES, "ap", "ap",
	 0, 0, 0 },
	{ { 0, 0 }, MODID_AP_S_PANIC_SOFTLOCKUP, MODID_AP_S_PANIC_SOFTLOCKUP, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_PANIC, HI_APPANIC_SOFTLOCKUP, (u32)RDR_UPLOAD_YES, "ap", "ap",
	 0, 0, 0 },
	{ { 0, 0 }, MODID_AP_S_PANIC_OTHERCPU_HARDLOCKUP, MODID_AP_S_PANIC_OTHERCPU_HARDLOCKUP, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_PANIC, HI_APPANIC_OHARDLOCKUP, (u32)RDR_UPLOAD_YES, "ap", "ap",
	 0, 0, 0 },
	{ { 0, 0 }, MODID_AP_S_PANIC_SP805_HARDLOCKUP, MODID_AP_S_PANIC_SP805_HARDLOCKUP, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_PANIC, HI_APPANIC_HARDLOCKUP, (u32)RDR_UPLOAD_YES, "ap", "ap",
	 0, 0, 0 },
	{ { 0, 0 }, MODID_AP_S_PANIC_STORAGE, MODID_AP_S_PANIC_STORAGE, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_PANIC, HI_APPANIC_Storage, (u32)RDR_UPLOAD_YES, "ap", "ap",
	 0, 0, 0 },
	{ { 0, 0 }, MODID_AP_S_NOC, MODID_AP_S_NOC, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_NOC, 0, (u32)RDR_UPLOAD_YES, "ap", "ap",
	 0, 0, 0 },
	{ { 0, 0 }, MODID_AP_S_DDRC_SEC, MODID_AP_S_DDRC_SEC, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, 0, (u32)RDR_UPLOAD_YES, "ap",
	 "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_AP_S_COMBINATIONKEY, MODID_AP_S_COMBINATIONKEY,
	 RDR_ERR, RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_COMBINATIONKEY, 0, (u32)RDR_UPLOAD_YES,
	 "ap", "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_AP_S_MAILBOX, MODID_AP_S_MAILBOX, RDR_WARN,
	 RDR_REBOOT_NO, RDR_AP, 0, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_MAILBOX, 0, (u32)RDR_UPLOAD_YES, "ap",
	 "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_AP_S_PMU, MODID_AP_S_PMU, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_PMU, 0, (u32)RDR_UPLOAD_YES, "ap pmu", "ap pmu",
	 0, 0, 0 },
	{ { 0, 0 }, MODID_AP_S_SUBPMU, MODID_AP_S_SUBPMU, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	(u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_SUBPMU, 0, (u32)RDR_UPLOAD_YES, "ap subpmu", "ap subpmu",
	 0, 0, 0 },
	{ { 0, 0 }, MODID_AP_S_SMPL, MODID_AP_S_SMPL, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_SMPL, 0, (u32)RDR_UPLOAD_YES, "ap smpl", "ap smpl",
	 0, 0, 0 },
	{ { 0, 0 }, MODID_AP_S_SCHARGER, MODID_AP_S_SCHARGER, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_SCHARGER, 0, (u32)RDR_UPLOAD_YES, "ap scharger", "ap scharger",
	 0, 0, 0 },
	{ { 0, 0 }, MODID_AP_S_RESUME_SLOWY, MODID_AP_S_RESUME_SLOWY, RDR_WARN,
	 RDR_REBOOT_NO, RDR_AP, 0, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, AP_S_RESUME_SLOWY, 0, (u32)RDR_UPLOAD_YES,
	 "ap resumeslowy", "ap resumeslowy", 0, 0, 0 },
	{ { 0, 0 }, MODID_AP_S_PANIC_ISP, MODID_AP_S_PANIC_ISP, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_PANIC, HI_APPANIC_ISP, (u32)RDR_UPLOAD_YES, "ap", "ap",
	 0, 0, 0 },
	{ { 0, 0 }, MODID_AP_S_PANIC_IVP, MODID_AP_S_PANIC_IVP, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_PANIC, HI_APPANIC_IVP, (u32)RDR_UPLOAD_YES, "ap", "ap",
	 0, 0, 0 },
	{ { 0, 0 }, MODID_AP_S_PANIC_AUDIO_CODEC, MODID_AP_S_PANIC_AUDIO_CODEC, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AUDIO_CODEC_EXCEPTION, 0, (u32)RDR_UPLOAD_YES,
	 "audio codec error", "audio codec error", 0, 0, 0 },
	{ { 0, 0 }, MODID_AP_S_PANIC_MODEM, MODID_AP_S_PANIC_MODEM, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP | RDR_CP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_PANIC, HI_APPANIC_MODEM, (u32)RDR_UPLOAD_YES,
	 "ap modem drv", "ap modem drv", 0, 0, 0 },
	{ { 0, 0 }, MODID_AP_S_PANIC_LB, MODID_AP_S_PANIC_LB, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_PANIC, HI_APPANIC_LB, (u32)RDR_UPLOAD_YES,
	 "ap lb drv", "ap lb drv", 0, 0, 0 },
	{ { 0, 0 }, MODID_AP_S_PANIC_PLL_UNLOCK, MODID_AP_S_PANIC_PLL_UNLOCK, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_PANIC, HI_APPANIC_PLL_UNLOCK, (u32)RDR_UPLOAD_YES,
	 "ap", "ap", 0, 0, 0 },
};

/*
 * The following table lists the dump memory used by other maintenance
 * and test modules and IP addresses of the AP.
 */
static unsigned int g_dump_modu_mem_size_tbl[MODU_MAX] = {0};
static char g_dump_modu_compatible[MODU_MAX][AMNTN_MODULE_COMP_LEN] = {
	[MODU_NOC] = { "ap_dump_mem_modu_noc_size" },
	[MODU_DDR] = { "ap_dump_mem_modu_ddr_size" },
	[MODU_TMC] = {""},
	[MODU_MNTN_FAC] = {""},
	[MODU_SMMU] = {""},
	[MODU_QIC] = {""},
	[MODU_GAP] = { "ap_dump_mem_modu_gap_size" },
};

static int rdr_hisiap_early_panic_notify(struct notifier_block *nb,
				unsigned long event, void *buf);

static void get_product_version_work_fn(struct work_struct *work);

static struct notifier_block acpu_panic_loop_block = {
	.notifier_call = acpu_panic_loop_notify,
	.priority = INT_MAX,
};

static struct notifier_block rdr_hisiap_early_panic_block = {
	.notifier_call = rdr_hisiap_early_panic_notify,
	.priority = INT_MIN,
};

static struct notifier_block rdr_hisiap_panic_block = {
	.notifier_call = rdr_hisiap_panic_notify,
	.priority = INT_MIN,
};

static struct notifier_block rdr_hisiap_die_block = {
	.notifier_call = rdr_hisiap_die_notify,
	.priority = INT_MIN,
};

static struct notifier_block rdr_hisiap_powerkey_block = {
	.notifier_call = rdr_press_key_to_fastboot,
	.priority = INT_MIN,
};

static DECLARE_DELAYED_WORK(get_product_version_work,
			get_product_version_work_fn);

/*
 * Description:   Read the memory size of each track on the AP side from dts.
 * Return:        0:read success, non 0:fail
 */
static int get_ap_trace_mem_size_from_dts(void)
{
	int ret;
	struct device_node *np = NULL;
	u32 i = 0;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,rdr_ap_adapter");
	if (!np) {
		BB_PRINT_ERR("[%s], cannot find rdr_ap_adapter node in dts!\n", __func__);
		return -ENODEV;
	}

	for (i = 0; i < HK_MAX; i++) {
		/* empty size string, set size zero */
		if (g_trace_buffer_modu[i][0] == '\0') {
			g_dump_buffer_size_tbl[i] = 0;
			continue;
		}
		ret = of_property_read_u32(np, g_trace_buffer_modu[i], &g_dump_buffer_size_tbl[i]);
		if (ret) {
			/* no dts property, set size zero */
			BB_PRINT_ERR("[%s], cannot find %s in dts!\n", __func__, g_trace_buffer_modu[i]);
			g_dump_buffer_size_tbl[i] = 0;
		}
		BB_PRINT_DBG("[%s], %s=0x%x get from dts\n", __func__, g_trace_buffer_modu[i], g_dump_buffer_size_tbl[i]);
	}

	return 0;
}

/*
 * Description:  Read the dump memory size of each module on the AP side from the dts file.
 * Return:       0:read success, non 0:fail
 */
static int get_ap_dump_mem_modu_size_from_dts(void)
{
	int ret = 0;
	struct device_node *np = NULL;
	u32 i = 0;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,rdr_ap_adapter");
	if (!np) {
		BB_PRINT_ERR("[%s], cannot find rdr_ap_adapter node in dts!\n", __func__);
		return -ENODEV;
	}

	for (i = 0; i < MODU_MAX; i++) {
		/* empty string, set size zero */
		if (g_dump_modu_compatible[i][0] == 0) {
			g_dump_modu_mem_size_tbl[i] = 0;
			continue;
		}
		ret = of_property_read_u32(np, g_dump_modu_compatible[i], &g_dump_modu_mem_size_tbl[i]);
		if (ret) {
			BB_PRINT_ERR("[%s], cannot find %s in dts!\n", __func__, g_dump_modu_compatible[i]);
			return ret;
		}
	}

	return ret;
}

void print_debug_info(void)
{
	unsigned int i;
	struct regs_info *regs_info = g_rdr_ap_root->dump_regs_info;

	pr_info("=================struct ap_eh_root================");
	pr_info("[%s], dump_magic [0x%x]\n", __func__, g_rdr_ap_root->dump_magic);
	pr_info("[%s], version [%s]\n", __func__, g_rdr_ap_root->version);
	pr_info("[%s], modid [0x%x]\n", __func__, g_rdr_ap_root->modid);
	pr_info("[%s], e_exce_type [0x%x],\n", __func__, g_rdr_ap_root->e_exce_type);
	pr_info("[%s], e_exce_subtype [0x%x],\n", __func__, g_rdr_ap_root->e_exce_subtype);
	pr_info("[%s], coreid [0x%llx]\n", __func__, g_rdr_ap_root->coreid);
	pr_info("[%s], slice [%llu]\n", __func__, g_rdr_ap_root->slice);
	pr_info("[%s], enter_times [0x%x]\n", __func__, g_rdr_ap_root->enter_times);
	pr_info("[%s], num_reg_regions [0x%x]\n", __func__, g_rdr_ap_root->num_reg_regions);

	for (i = 0; i < g_rdr_ap_root->num_reg_regions; i++)
		pr_info(
			"[%s], reg_name [%s], reg_base [0x%pK], reg_size [0x%x], reg_dump_addr [0x%pK]\n",
			__func__, regs_info[i].reg_name,
			(void *)(uintptr_t)regs_info[i].reg_base,
			regs_info[i].reg_size,
			regs_info[i].reg_dump_addr);
}

static int check_addr_overflow(const unsigned char *addr)
{
	unsigned char *max_addr = NULL;

	if (!addr) {
		BB_PRINT_ERR("[%s], invalid addr!\n", __func__);
		return -1;
	}

	max_addr = g_rdr_ap_root->rdr_ap_area_map_addr +
		g_rdr_ap_root->ap_rdr_info.log_len - PMU_RESET_RECORD_DDR_AREA_SIZE;
	if ((addr < g_rdr_ap_root->rdr_ap_area_map_addr) || (addr >= max_addr))
		return -1;

	return 0;
}

/* The 1K space occupied by the struct ap_eh_root is not included */
static unsigned char *get_rdr_hisiap_dump_start_addr(void)
{
	unsigned char *addr = NULL;
	unsigned int timers = sizeof(*g_rdr_ap_root) / SIZE_1K + 1;

	addr = g_rdr_ap_root->rdr_ap_area_map_addr + ALIGN(sizeof(*g_rdr_ap_root), timers * SIZE_1K);
	pr_info("[%s], aligned by %u, dump_start_addr [0x%pK]\n", __func__, timers, addr);
	if (check_addr_overflow(addr)) {
		BB_PRINT_ERR("[%s], there is no space left for ap to dump!\n", __func__);
		return NULL;
	}
	return addr;
}

static int io_resources_init(void)
{
	int ret;
	unsigned int i;
	struct device_node *np = NULL;
	struct resource res;
	struct regs_info *regs = NULL;
	unsigned char *tmp = NULL;
	u32 data[DTS_32K_TIMER_SIZE];

	regs = g_rdr_ap_root->dump_regs_info;
	memset((void *)regs, 0, REGS_DUMP_MAX_NUM * sizeof(*regs));

	np = of_find_compatible_node(NULL, NULL, "hisilicon,rdr_ap_adapter");
	if (!np) {
		BB_PRINT_ERR("[%s], cannot find rdr_ap_adapter node in dts!\n", __func__);
		return -ENODEV;
	}

	ret = of_property_read_u32(np, "reg-dump-regions", &g_rdr_ap_root->num_reg_regions);
	if (ret) {
		BB_PRINT_ERR("[%s], cannot find reg-dump-regions in dts!\n", __func__);
		goto timer_ioinit;
	}

	if (g_rdr_ap_root->num_reg_regions == 0) {
		BB_PRINT_ERR("[%s], reg-dump-regions in zero, so no reg resource to init\n", __func__);
		goto timer_ioinit;
	}

	for (i = 0; i < g_rdr_ap_root->num_reg_regions; i++) {
		if (of_address_to_resource(np, i, &res)) {
			BB_PRINT_ERR("[%s], of_addr_to_resource [%u] fail!\n", __func__, i);
			goto timer_ioinit;
		}

		strncpy(regs[i].reg_name, res.name, REG_NAME_LEN - 1);
		regs[i].reg_name[REG_NAME_LEN - 1] = '\0';
		regs[i].reg_base = res.start;
		regs[i].reg_size = resource_size(&res);

		if (regs[i].reg_size == 0) {
			BB_PRINT_ERR("[%s], [%s] registers size is 0, skip map!\n", __func__, (regs[i].reg_name));
			goto reg_dump_addr_init;
		}
		regs[i].reg_map_addr = of_iomap(np, i);
		pr_info("[%s], regs_info[%u].reg_name[%s], reg_base[0x%pK], reg_size[0x%x], map_addr[0x%pK]\n",
			__func__, i, regs[i].reg_name, (void *)(uintptr_t)regs[i].reg_base, regs[i].reg_size,
			regs[i].reg_map_addr);
		if (!regs[i].reg_map_addr) {
			BB_PRINT_ERR("[%s], unable to map [%s] registers\n", __func__, (regs[i].reg_name));
			goto timer_ioinit;
		}
		pr_info("[%s], map [%s] registers ok\n", __func__, (regs[i].reg_name));
reg_dump_addr_init:

		if (i == 0) {
			regs[i].reg_dump_addr = get_rdr_hisiap_dump_start_addr();
			if (IS_ERR_OR_NULL(regs[i].reg_dump_addr)) {
				BB_PRINT_ERR("[%s], reg_dump_addr is invalid!\n", __func__);
				goto timer_ioinit;
			}
		} else {
			regs[i].reg_dump_addr = regs[i - 1].reg_dump_addr + regs[i - 1].reg_size;
		}
	}

	tmp = regs[i - 1].reg_dump_addr + regs[i - 1].reg_size;
	if (check_addr_overflow(tmp))
		BB_PRINT_ERR("[%s], there is no space left for ap to dump regs!\n", __func__);

timer_ioinit:

	/* io resource of 32k timer */
	ret = of_property_read_u32_array(np, "ldrx2dbg-abs-timer", &data[SCTRL_ADDR_INDEX], DTS_32K_TIMER_SIZE);
	if (ret) {
		BB_PRINT_ERR("[%s], cannot find ldrx2dbg-abs-timer in dts!\n", __func__);
		return ret;
	}

	sctrl_map_addr = ioremap(data[SCTRL_ADDR_INDEX], data[SCTRL_SIZE_INDEX]);
	if (!sctrl_map_addr) {
		BB_PRINT_ERR("[%s], cannot find ldrx2dbg-abs-timer in dts!\n", __func__);
		return -EFAULT;
	}
	g_32k_timer_l32bit_addr = sctrl_map_addr + data[L32BIT_ADDR_INDEX];
	g_32k_timer_h32bit_addr = sctrl_map_addr + data[H32BIT_ADDR_INDEX];

	return 0;
}

static unsigned int get_total_regdump_region_size(const struct regs_info *regs_info, u32 regs_info_len)
{
	unsigned int i;
	u32 total = 0;

	if (!regs_info) {
		BB_PRINT_ERR("[%s],regs_info is null\n", __func__);
		return 0;
	}

	if (regs_info_len < REGS_DUMP_MAX_NUM)
		BB_PRINT_ERR("[%s],regs_info_len is too small\n", __func__);

	for (i = 0; i < g_rdr_ap_root->num_reg_regions; i++)
		total += regs_info[i].reg_size;

	pr_info("[%s], num_reg_regions [%u], regdump size [0x%x]\n",
		__func__, g_rdr_ap_root->num_reg_regions, total);
	return total;
}

/*
 * The interrupt, task, and cpuidle are divided based on the CPU frequency,
 * nitialization of other track areas that do not differentiate CPUs
 */
int __init hook_buffer_alloc(void)
{
	int i, ret;
	unsigned int percpu_arr_size, single_arr_size;


	percpu_arr_size = ARRAY_SIZE(g_trace_buffer_percpu);
	single_arr_size = ARRAY_SIZE(g_trace_buffer_single);
	if (percpu_arr_size + single_arr_size > HK_MAX)
		return -1;

	pr_info("[%s], irq_buffer_init start!\n", __func__);

	for (i = 0; i < HK_MAX; i++) {
		if (i < HK_PERCPU_TAG) {
			if (g_trace_buffer_percpu[i].func) {
				ret = g_trace_buffer_percpu[i].func(&g_rdr_ap_root->hook_percpu_buffer[i],
					g_rdr_ap_root->hook_buffer_addr[i], g_dump_buffer_size_tbl[i]);
				if (ret) {
					BB_PRINT_ERR("[%s], hook_buffer_addr[%d] init failed!\n", __func__, i);
					return ret;
				}
			}
		} else {
			if (g_trace_buffer_single[i - HK_PERCPU_TAG].func) {
				ret = g_trace_buffer_single[i - HK_PERCPU_TAG].func(&g_rdr_ap_root->hook_buffer_addr[i],
					g_dump_buffer_size_tbl[i]);
				if (ret) {
					BB_PRINT_ERR("[%s], hook_buffer_addr[%d] init failed!\n", __func__, i);
					return ret;
				}
			}
		}

	}

	return 0;
}

static void rdr_hisiap_print_all_dump_addrs(void)
{
	unsigned int i;

	if (IS_ERR_OR_NULL(g_rdr_ap_root)) {
		BB_PRINT_ERR("[%s], g_rdr_ap_root [0x%pK] is invalid\n", __func__, g_rdr_ap_root);
		return;
	}

	for (i = 0; i < g_rdr_ap_root->num_reg_regions; i++)
		pr_info("[%s], reg_name [%s], reg_dump_addr [0x%pK]\n",
			__func__,
			g_rdr_ap_root->dump_regs_info[i].reg_name,
			g_rdr_ap_root->dump_regs_info[i].reg_dump_addr);

	pr_info("[%s], rdr_ap_area_map_addr [0x%pK]\n",
			__func__, g_rdr_ap_root->rdr_ap_area_map_addr);
	for (i = 0; i < HK_MAX; i++)
		pr_info("[%s], %s addr [0x%pK]\n", __func__,
			g_trace_buffer_modu[i], g_rdr_ap_root->hook_buffer_addr[i]);

	for (i = 0; i < NR_CPUS; i++)
		pr_info("[%s], last_task_stack_dump_addr[%u] [0x%pK]\n",
			__func__, i, g_rdr_ap_root->last_task_stack_dump_addr[i]);

	for (i = 0; i < NR_CPUS; i++)
		pr_info("[%s], last_task_struct_dump_addr[%u] [0x%pK]\n",
			__func__, i, g_rdr_ap_root->last_task_struct_dump_addr[i]);

	for (i = 0; i < MODU_MAX; i++) {
		if (g_rdr_ap_root->module_dump_info[i].dump_size != 0)
			pr_info("[%s], module_dump_info[%u].dump_addr [0x%pK]\n",
				__func__, i, g_rdr_ap_root->module_dump_info[i].dump_addr);
	}
}

static void module_dump_mem_init(void)
{
	int i;

	for (i = 0; i < MODU_MAX; i++) {
		if (i == 0) {
			g_rdr_ap_root->module_dump_info[0].dump_addr =
				g_rdr_ap_root->last_task_stack_dump_addr[NR_CPUS - 1] +
				g_last_task_stack_size;
		} else {
			g_rdr_ap_root->module_dump_info[i].dump_addr =
				g_rdr_ap_root->module_dump_info[i - 1].dump_addr +
				g_dump_modu_mem_size_tbl[i - 1];
		}

		if (check_addr_overflow(g_rdr_ap_root->module_dump_info[i].dump_addr +
				g_dump_modu_mem_size_tbl[i])) {
			BB_PRINT_ERR("[%s], there is no enough space for modu [%d] to dump mem!\n", __func__, i);
			break;
		}
		g_rdr_ap_root->module_dump_info[i].dump_size = g_dump_modu_mem_size_tbl[i];

		pr_info("[%s], dump_addr [0x%pK] dump_size [0x%x]!\n",
				__func__,
				g_rdr_ap_root->module_dump_info[i].dump_addr,
				g_rdr_ap_root->module_dump_info[i].dump_size);
	}
	/*
	 * When ap_last_task_switch is disabled,
	 * the start address of the dump memory of the last_task struct and stack is set to 0
	 */
	if (!get_ap_last_task_switch_from_dts()) {
		for (i = 0; i < NR_CPUS; i++) {
			g_rdr_ap_root->last_task_struct_dump_addr[i] = 0;
			g_rdr_ap_root->last_task_stack_dump_addr[i] = 0;
		}
	}
}

static int __init ap_dump_buffer_init(void)
{
	int i;

	/* Track record area */
	g_rdr_ap_root->hook_buffer_addr[0] = get_rdr_hisiap_dump_start_addr() +
		get_total_regdump_region_size(g_rdr_ap_root->dump_regs_info, REGS_DUMP_MAX_NUM);
	for (i = 1; i < HK_MAX; i++)
		g_rdr_ap_root->hook_buffer_addr[i] =
			g_rdr_ap_root->hook_buffer_addr[i - 1] + g_dump_buffer_size_tbl[i - 1];

	/* Task record area */
	if (get_ap_last_task_switch_from_dts()) {
		g_last_task_struct_size = sizeof(struct task_struct);
		g_last_task_stack_size = THREAD_SIZE;
	}

	g_rdr_ap_root->last_task_struct_dump_addr[0] =
		g_rdr_ap_root->hook_buffer_addr[HK_MAX - 1] + g_dump_buffer_size_tbl[HK_MAX - 1];
	for (i = 1; i < NR_CPUS; i++)
		g_rdr_ap_root->last_task_struct_dump_addr[i] =
			g_rdr_ap_root->last_task_struct_dump_addr[i - 1] + g_last_task_struct_size;

	g_rdr_ap_root->last_task_stack_dump_addr[0] = (unsigned char *)(uintptr_t)
		ALIGN(((uintptr_t)g_rdr_ap_root->last_task_struct_dump_addr[NR_CPUS - 1] +
				g_last_task_struct_size), SIZE_1K); /* Aligned by 1K */
	for (i = 1; i < NR_CPUS; i++)
		g_rdr_ap_root->last_task_stack_dump_addr[i] =
			g_rdr_ap_root->last_task_stack_dump_addr[i - 1] +
			g_last_task_stack_size;

	if (check_addr_overflow(g_rdr_ap_root->last_task_stack_dump_addr[NR_CPUS - 1] +
		g_last_task_stack_size)) {
		BB_PRINT_ERR("[%s], there is no enough space for ap to dump!\n", __func__);
		return -ENOSPC;
	}
	pr_info("[%s], module_dump_mem_init start\n", __func__);
	module_dump_mem_init();

	rdr_hisiap_print_all_dump_addrs();
	return hook_buffer_alloc();
}

u64 get_32k_abs_timer_value(void)
{
	u64 timer_count;

	if (!g_32k_timer_l32bit_addr || !g_32k_timer_h32bit_addr)
		return 0;

	timer_count = *(volatile unsigned int *)g_32k_timer_l32bit_addr;
	timer_count |= ((u64)(*(volatile unsigned int *)g_32k_timer_h32bit_addr)) << RDR_REG_BITS;
	return timer_count;
}

void rdr_set_wdt_kick_slice(u64 kickslice)
{
	static u32 kicktimes;
	struct rdr_arctimer_s *rdr_arctimer = rdr_get_arctimer_record();

	rdr_arctimer_register_read(rdr_arctimer);

	if (g_rdr_ap_root != NULL) {
		g_rdr_ap_root->wdt_kick_slice[kicktimes % WDT_KICK_SLICE_TIMES] = kickslice;
		kicktimes++;
	}
}

u64 rdr_get_last_wdt_kick_slice(void)
{
	int i;
	u64 last_kick_slice;

	if (WDT_KICK_SLICE_TIMES <= 0)
		return 0;

	if (!g_rdr_ap_root)
		return 0;

	if (WDT_KICK_SLICE_TIMES == 1)
		return g_rdr_ap_root->wdt_kick_slice[0];

	last_kick_slice = g_rdr_ap_root->wdt_kick_slice[0];
	for (i = 1; i < WDT_KICK_SLICE_TIMES; i++)
		last_kick_slice = max(last_kick_slice, g_rdr_ap_root->wdt_kick_slice[i]);

	return last_kick_slice;
}

void regs_dump(void)
{
	unsigned int i;
	struct regs_info *regs_info = NULL;

	regs_info = g_rdr_ap_root->dump_regs_info;

	/*
	 * NOTE:sctrl in the power-on area, pctrl, pctrl, pericrg in the peripheral area,
	 * Do not check the domain when accessing the A core
	 */
	for (i = 0; i < g_rdr_ap_root->num_reg_regions; i++) {
		if (IS_ERR_OR_NULL(regs_info[i].reg_map_addr) ||
			IS_ERR_OR_NULL(regs_info[i].reg_dump_addr)) {
			regs_info[i].reg_dump_addr = 0;
			BB_PRINT_ERR(
				"[%s], regs_info[%u].reg_map_addr [%pK] reg_dump_addr [%pK] invalid!\n",
				__func__, i, regs_info[i].reg_map_addr,
				regs_info[i].reg_dump_addr);
			continue;
		}
		pr_info(
			"[%s], memcpy [0x%x] size from regs_info[%u].reg_map_addr [%pK] to reg_dump_addr [%pK]\n",
			__func__, regs_info[i].reg_size, i,
			regs_info[i].reg_map_addr,
			regs_info[i].reg_dump_addr);
		rdr_regs_dump(regs_info[i].reg_dump_addr,
			regs_info[i].reg_map_addr,
			regs_info[i].reg_size);
	}
}


static void get_product_version_work_fn(struct work_struct *work)
{
	BB_PRINT_PN("[%s], enter!\n", __func__);
	if (!g_rdr_ap_root) {
		BB_PRINT_ERR("[%s], g_rdr_ap_root is NULL!\n", __func__);
		return;
	}

	get_product_version((char *)g_rdr_ap_root->version, PRODUCT_VERSION_LEN);
	BB_PRINT_PN("[%s], exit!\n", __func__);
}

/*
 * Description:    Memory dump registration interface provided for the AP maintenance
 *                 and test module and IP address
 * Input:          func:Registered dump function, module_name, mod
 * Output:         NA
 * Return:         0:Registration succeeded, <0:fail
 */
int register_module_dump_mem_func(rdr_hisiap_dump_func_ptr func,
				const char *module_name,
				dump_mem_module modu)
{
	int ret = -1;

	if (modu >= MODU_MAX) {
		BB_PRINT_ERR("[%s], modu [%u] is invalid!\n", __func__, modu);
		return -EINVAL;
	}

	if (IS_ERR_OR_NULL(func)) {
		BB_PRINT_ERR("[%s], func [0x%pK] is invalid!\n", __func__, func);
		return -EINVAL;
	}

	if (IS_ERR_OR_NULL(module_name)) {
		BB_PRINT_ERR("[%s], module_name is invalid!\n", __func__);
		return -EINVAL;
	}

	if (!g_rdr_ap_root) {
		BB_PRINT_ERR("[%s], g_rdr_ap_root is null!\n", __func__);
		return -1;
	}

	BB_PRINT_PN("[%s], module_name [%s]\n", __func__, module_name);
	mutex_lock(&g_dump_mem_mutex);
	if (g_rdr_ap_root->module_dump_info[modu].dump_size != 0) {
		g_rdr_ap_root->module_dump_info[modu].dump_funcptr = func;
		strncpy(g_rdr_ap_root->module_dump_info[modu].module_name,
			module_name, AMNTN_MODULE_NAME_LEN - 1);
		g_rdr_ap_root->module_dump_info[modu].module_name[AMNTN_MODULE_NAME_LEN - 1] = '\0';
		ret = 0;
	}
	mutex_unlock(&g_dump_mem_mutex);

	if (ret)
		BB_PRINT_ERR(
			"[%s], func[0x%pK], size[%u], [%s] register failed!\n",
			__func__, func,
			g_rdr_ap_root->module_dump_info[modu].dump_size,
			module_name);
	return ret;
}

/*
 * Description:  Obtains the dump start address of the dump module
 * Input:        modu:Module ID,This is a unified allocation;
 * Output:       dump_addr:Start address of the dump memory allocated to the module MODU
 * Return:       0:The data is successfully obtained ,Smaller than 0:Obtaining failed
 */
int get_module_dump_mem_addr(dump_mem_module modu, unsigned char **dump_addr)
{
	if (!rdr_get_ap_init_done()) {
		BB_PRINT_ERR("[%s]rdr not init\n", __func__);
		return -EPERM;
	}

	if (modu >= MODU_MAX) {
		BB_PRINT_ERR("[%s]modu [%u] is invalid\n", __func__, modu);
		return -EINVAL;
	}

	if (!dump_addr) {
		BB_PRINT_ERR("[%s]dump_addr is invalid\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&g_dump_mem_mutex);
	if (g_rdr_ap_root->module_dump_info[modu].dump_size == 0) {
		BB_PRINT_ERR("[%s]modu[%u] dump_size is zero\n", __func__, modu);
		mutex_unlock(&g_dump_mem_mutex);
		return -EPERM;
	}
	*dump_addr = g_rdr_ap_root->module_dump_info[modu].dump_addr;

	if (!(*dump_addr)) {
		BB_PRINT_ERR("[%s] *dump_addr is invalid\n", __func__);
		mutex_unlock(&g_dump_mem_mutex);
		return -EINVAL;
	}

	mutex_unlock(&g_dump_mem_mutex);

	return 0;
}

/*
 * Description:   Before the abnormal reset, the AP maintenance and test
 *                module and the memory dump registration function provided
 *                by the IP are invoked
 */
void save_module_dump_mem(void)
{
	int i;
	void *addr = NULL;
	unsigned int size = 0;

	BB_PRINT_PN("[%s], enter\n", __func__);


	for (i = 0; i < MODU_MAX; i++) {
		if (g_rdr_ap_root->module_dump_info[i].dump_funcptr != NULL) {
			addr = (void *)g_rdr_ap_root->module_dump_info[i].dump_addr;
			size = g_rdr_ap_root->module_dump_info[i].dump_size;
			if (!g_rdr_ap_root->module_dump_info[i].dump_funcptr(addr, size)) {
				BB_PRINT_ERR("[%s], [%s] dump failed!\n", __func__,
					g_rdr_ap_root->module_dump_info[i].module_name);
				continue;
			}
		}
	}
	BB_PRINT_PN("[%s], exit\n", __func__);
}

#ifdef CONFIG_PM
void rdr_hisiap_root_backup(void)
{
	if (g_rdr_ap_root)
		(void)memcpy_s(&g_rdr_ap_root_bak, sizeof(struct ap_eh_root),
				g_rdr_ap_root, sizeof(struct ap_eh_root));
}

void rdr_hisiap_root_restore(void)
{
	if (g_rdr_ap_root)
		(void)memcpy_s(g_rdr_ap_root, sizeof(struct ap_eh_root),
				&g_rdr_ap_root_bak, sizeof(struct ap_eh_root));
}
#endif

int __init rdr_hisiap_dump_init(const struct rdr_register_module_result *retinfo)
{
	int ret = 0;

	BB_PRINT_PN("[%s], begin\n", __func__);

	if (!retinfo) {
		BB_PRINT_ERR("%s():%d:retinfo is NULL!\n", __func__, __LINE__);
		return -1;
	}

	g_rdr_ap_root = (struct ap_eh_root *)(uintptr_t)g_hisiap_addr;
	strncpy(g_log_path, g_rdr_ap_root->log_path, LOG_PATH_LEN - 1);
	g_log_path[LOG_PATH_LEN - 1] = '\0';

	/*
	 * The pmu subboard does not exist.
	 * Replace the last eight bytes of the AP exception area temporarily
	 */
	memset((void *)(uintptr_t)g_hisiap_addr, 0,
		retinfo->log_len - PMU_RESET_RECORD_DDR_AREA_SIZE);
	g_rdr_ap_root = (struct ap_eh_root *)(uintptr_t) g_hisiap_addr;
	g_rdr_ap_root->ap_rdr_info.log_addr = retinfo->log_addr;
	g_rdr_ap_root->ap_rdr_info.log_len = retinfo->log_len;
	g_rdr_ap_root->ap_rdr_info.nve = retinfo->nve;
	g_rdr_ap_root->rdr_ap_area_map_addr = (void *)(uintptr_t)g_hisiap_addr;
	get_device_platform(g_rdr_ap_root->device_id, PRODUCT_DEVICE_LEN);
	g_rdr_ap_root->bbox_version = BBOX_VERSION;
	g_rdr_ap_root->dump_magic = AP_DUMP_MAGIC;
	g_rdr_ap_root->end_magic = AP_DUMP_END_MAGIC;

	BB_PRINT_PN("[%s], io_resources_init start\n", __func__);
	ret = io_resources_init();
	if (ret) {
		BB_PRINT_ERR("[%s], io_resources_init failed!\n", __func__);
		return ret;
	}

	BB_PRINT_PN("[%s], get_ap_trace_mem_size_from_dts start\n", __func__);
	ret = get_ap_trace_mem_size_from_dts();
	if (ret) {
		BB_PRINT_ERR("[%s], get_ap_trace_mem_size_from_dts failed!\n", __func__);
		return ret;
	}

	BB_PRINT_PN("[%s], get_ap_dump_mem_modu_size_from_dts start\n", __func__);
	ret = get_ap_dump_mem_modu_size_from_dts();
	if (ret) {
		BB_PRINT_ERR("[%s], get_ap_dump_mem_modu_size_from_dts failed!\n", __func__);
		return ret;
	}

	BB_PRINT_PN("[%s], ap_dump_buffer_init start\n", __func__);
	ret = ap_dump_buffer_init();
	if (ret) {
		BB_PRINT_ERR("[%s], ap_dump_buffer_init failed!\n", __func__);
		return ret;
	}

	BB_PRINT_PN("[%s], register_arch_timer_func_ptr start\n", __func__);
	ret = register_arch_timer_func_ptr(get_32k_abs_timer_value);
	if (ret) {
		BB_PRINT_ERR("[%s], register_arch_timer_func_ptr failed!\n", __func__);
		return ret;
	}
	/* Whether the track is generated is consistent with the kernel dump */
	if (check_himntn(HIMNTN_KERNEL_DUMP_ENABLE)) {
		BB_PRINT_PN("[%s], hisi_trace_hook_install start\n", __func__);
		ret = hisi_trace_hook_install();
		if (ret) {
			BB_PRINT_ERR("[%s], hisi_trace_hook_install failed!\n", __func__);
			return ret;
		}
	} else {
		BB_PRINT_PN("[%s], hisi_ap_defopen_hook_install start\n", __func__);
		hisi_ap_defopen_hook_install();
	}

	schedule_delayed_work(&get_product_version_work, 0);

	BB_PRINT_PN("[%s], end\n", __func__);
	return ret;
}


void save_bl31_exc_memory(void)
{
	int iret;
	int len;
	char fullpath_arr[LOG_PATH_LEN] = "";
	char dst_str[LOG_PATH_LEN] = "";

	(void)memset_s(fullpath_arr, LOG_PATH_LEN, 0, LOG_PATH_LEN);

	strncat(fullpath_arr, g_log_path,
		(LOG_PATH_LEN - 1 - strlen(fullpath_arr)));
	strncat(fullpath_arr, "/ap_log/",
		(LOG_PATH_LEN - 1 - strlen(fullpath_arr)));
	BB_PRINT_PN("%s: %s\n", __func__, fullpath_arr);

	iret = mntn_filesys_create_dir(fullpath_arr, DIR_LIMIT);
	if (iret != 0) {
		BB_PRINT_ERR("%s: iret is [%d]\n", __func__, iret);
		return;
	}
	len = strlen(fullpath_arr);
	iret = memcpy_s(dst_str, LOG_PATH_LEN, fullpath_arr, len + 1);
	if (iret != EOK) {
		BB_PRINT_ERR("%s():%d:memcpy_s fail!\n", __func__, __LINE__);
		return;
	}
	strncat(&dst_str[len], "/bl31_mntn_memory.bin", (LOG_PATH_LEN - 1 - len));
	iret = rdr_copy_file_apend(dst_str, SRC_BL31_MNTN_MEMORY);
	if (iret)
		BB_PRINT_ERR("[%s], save bl31_mntn_memory.bin error, ret = %d\n", __func__, iret);
}

int rdr_save_logbuff_memory(const char *dst, const char *src)
{
	long fddst = -1;
	long fdsrc = -1;
	char *src_buf = NULL;
	char *dst_buf = NULL;
	int ret = -1;

	fdsrc = SYS_OPEN(src, O_RDONLY, FILE_LIMIT);
	if (fdsrc < 0) {
		BB_PRINT_ERR("rdr:%s():open %s failed\n", __func__, src);
		goto err_fdsrc;
	}
	fddst = SYS_OPEN(dst, O_CREAT | O_WRONLY, FILE_LIMIT);
	if (fddst < 0) {
		BB_PRINT_ERR("rdr:%s():open %s failed\n", __func__, dst);
		goto err_fddst;
	}

	src_buf = vmalloc(SRC_LOG_BUFF_LEN);
	if (!src_buf) {
		BB_PRINT_ERR("rdr:%s():vmalloc for reading fail\n", __func__);
		goto err_src_buf;
	}
	dst_buf = vmalloc(DST_LOG_BUFF_LEN);
	if (!dst_buf) {
		BB_PRINT_ERR("rdr:%s():vmalloc for writing fail\n", __func__);
		goto err_dst_buf;
	}

	if (SYS_READ(fdsrc, src_buf, SRC_LOG_BUFF_LEN) <= 0)
		goto err_other;

	ret = decode_log_buff(dst_buf, DST_LOG_BUFF_LEN, src_buf);
	if (ret <= 0)
		goto err_other;

	if (SYS_WRITE(fddst, dst_buf, ret) <= 0) {
		BB_PRINT_ERR("rdr:%s():write %s failed\n", __func__, dst);
		goto err_other;
	}

	ret = 0;
err_other:
	vfree(dst_buf);
err_dst_buf:
	vfree(src_buf);
err_src_buf:
	SYS_CLOSE(fddst);
err_fddst:
	SYS_CLOSE(fdsrc);
err_fdsrc:
	return ret;
}

void save_logbuff_memory(void)
{
	int iret;
	size_t len;
	char fullpath_arr[LOG_PATH_LEN] = "";
	char dst_str[LOG_PATH_LEN] = "";

	(void)memset_s(fullpath_arr, LOG_PATH_LEN, 0, LOG_PATH_LEN);

	if (strncat_s(fullpath_arr, LOG_PATH_LEN,
			g_log_path, (LOG_PATH_LEN - 1 - strlen(fullpath_arr))) != EOK) {
		BB_PRINT_ERR("%s():%d:strncat_s fail!\n", __func__, __LINE__);
		return;
	}

	if (strncat_s(fullpath_arr, LOG_PATH_LEN,
			"/ap_log/", (LOG_PATH_LEN - 1 - strlen(fullpath_arr))) != EOK) {
		BB_PRINT_ERR("%s():%d:strncat_s fail!\n", __func__, __LINE__);
		return;
	}

	BB_PRINT_PN("%s: %s\n", __func__, fullpath_arr);

	iret = mntn_filesys_create_dir(fullpath_arr, DIR_LIMIT);
	if (iret != 0) {
		BB_PRINT_ERR("%s: iret is [%d]\n", __func__, iret);
		return;
	}

	len = strlen(fullpath_arr);
	if (memcpy_s((void *)dst_str, LOG_PATH_LEN, fullpath_arr, len + 1) != EOK) {
		BB_PRINT_ERR("[%s], memcpy_s fail!\n", __func__);
		return;
	}

	if (strncat_s(&dst_str[len], LOG_PATH_LEN - len, "/kernel_logbuff.bin",
		strlen("/kernel_logbuff.bin") + 1) != EOK) {
		BB_PRINT_ERR("%s():%d:strncat_s fail!\n", __func__, __LINE__);
		return;
	}

	if (rdr_save_logbuff_memory(dst_str, SRC_LOGBUFFER_MEMORY)) {
		BB_PRINT_ERR("%s:save decode logbuff fail\n", __func__);
		mntn_filesys_rm_file(dst_str);
		iret = rdr_copy_file_apend(dst_str, SRC_LOGBUFFER_MEMORY);
		if (iret)
			BB_PRINT_ERR("[%s], save bin failed\n", __func__);
	}
}

#ifdef CONFIG_HHEE
void save_hhee_exc_memory(void)
{
	int iret;
	unsigned long len;
	char fullpath_arr[LOG_PATH_LEN] = "";
	char dst_str[LOG_PATH_LEN] = "";

	(void)memset_s(fullpath_arr, (unsigned long)LOG_PATH_LEN, 0, (unsigned long)LOG_PATH_LEN);

	strncat(fullpath_arr, g_log_path,
		((LOG_PATH_LEN - 1) - strlen(fullpath_arr)));
	strncat(fullpath_arr, "/ap_log/",
		((LOG_PATH_LEN - 1) - strlen(fullpath_arr)));
	BB_PRINT_PN("%s: %s\n", __func__, fullpath_arr);

	iret = mntn_filesys_create_dir(fullpath_arr, DIR_LIMIT);
	if (iret != 0) {
		BB_PRINT_ERR("%s: iret is [%d]\n", __func__, iret);
		return;
	}
	len = strlen(fullpath_arr);
	iret = memcpy_s(dst_str, LOG_PATH_LEN, fullpath_arr, len + 1);
	if (iret != EOK) {
		BB_PRINT_ERR("%s():%d:memcpy_s fail!\n", __func__, __LINE__);
		return;
	}
	strncat(&dst_str[len], "/hhee_mntn_memory.bin",
		((LOG_PATH_LEN - 1) - len));
	iret = rdr_copy_file_apend(dst_str, SRC_HHEE_MNTN_MEMORY);
	if (iret)
		BB_PRINT_ERR("[%s], save hhee_mntn_memory.bin error, ret = %d\n", __func__, iret);
}
#endif


void save_lpmcu_exc_memory(void)
{
	int iret;
	int len;
	char fullpath_arr[LOG_PATH_LEN] = "";
	char dst_str[LOG_PATH_LEN] = "";

	(void)memset_s(fullpath_arr, LOG_PATH_LEN, 0, LOG_PATH_LEN);

	strncat(fullpath_arr, g_log_path, ((LOG_PATH_LEN - 1) - strlen(fullpath_arr)));
	strncat(fullpath_arr, "/lpmcu_log/", ((LOG_PATH_LEN - 1) - strlen(fullpath_arr)));
	BB_PRINT_PN("%s: %s\n", __func__, fullpath_arr);

	iret = mntn_filesys_create_dir(fullpath_arr, DIR_LIMIT);
	if (iret != 0) {
		BB_PRINT_ERR("%s: iret is [%d]\n", __func__, iret);
		return;
	}
	len = strlen(fullpath_arr);
	iret = memcpy_s(dst_str, LOG_PATH_LEN, fullpath_arr, len + 1);
	if (iret != EOK) {
		BB_PRINT_ERR("%s():%d:memcpy_s fail!\n", __func__, __LINE__);
		return;
	}
	iret = memcpy_s(&dst_str[len], LOG_PATH_LEN - strlen(dst_str), "/lpmcu_ddr_memory.bin",
			strlen("/lpmcu_ddr_memory.bin") + 1);
	if (iret != EOK) {
		BB_PRINT_ERR("%s():%d:memcpy_s fail!\n", __func__, __LINE__);
		return;
	}
	iret = rdr_copy_file_apend(dst_str, SRC_LPMCU_DDR_MEMORY);
	if (iret)
		BB_PRINT_ERR("[%s], save lpmcu_ddr_memory.bin error, ret = %d\n", __func__, iret);
}

void save_kernel_dump(void *arg)
{
	int fd = -1;
	int ret;
	u32 len;
	char date[DATATIME_MAXLEN];
	char dst_str[LOG_PATH_LEN] = "";
	char exce_dir[LOG_PATH_LEN] = "";

	snprintf(exce_dir, LOG_PATH_LEN, "%s%s/", PATH_ROOT, PATH_MEMDUMP);

	mntn_rm_old_log(exce_dir, 1);
	memset(dst_str, 0, LOG_PATH_LEN);
	memset(date, 0, DATATIME_MAXLEN);

	BB_PRINT_PN("exce_dir is [%s]\n", exce_dir);

	/*
	 * If the value of arg is true, it indicates that the thread saves the kerneldump.
	 * In this case, the exception timestamp needs to be obtained from the memory
	 */
	if (arg && (LOG_PATH_LEN - 1 >= strlen(g_log_path))) {
		len = strnlen(g_log_path, LOG_PATH_LEN);
		if (len >= DATATIME_MAXLEN) {
			ret = memcpy_s(date, DATATIME_MAXLEN,
					&g_log_path[len - DATATIME_MAXLEN], DATATIME_MAXLEN - 1);
			if (ret != EOK) {
				BB_PRINT_ERR("%s():%d:memcpy_s fail!\n", __func__, __LINE__);
				return;
			}
		}
	} else {
		/*
		 * If the value of arg is empty, it indicates that the AP is abnormal.
		 * If the value is the log saved after the reset, obtain the current time
		 */
		snprintf(date, DATATIME_MAXLEN, "%s-%08lld", rdr_get_timestamp(), rdr_get_tick());
	}

	fd = SYS_OPEN(exce_dir, O_DIRECTORY, 0);
	/* if dir is not exist,then create new dir */
	if (fd < 0) {
		fd = SYS_MKDIR(exce_dir, DIR_LIMIT);
		if (fd < 0) {
			BB_PRINT_ERR("%s %d\n", __func__, fd);
			goto out;
		}
	} else {
		SYS_CLOSE(fd);
	}
	strncat(exce_dir, "/", ((LOG_PATH_LEN - 1) - strlen(exce_dir)));
	strncat(exce_dir, date, ((LOG_PATH_LEN - 1) - strlen(exce_dir)));
	fd = SYS_MKDIR(exce_dir, DIR_LIMIT);
	if (fd < 0) {
		BB_PRINT_ERR("%s %d\n", __func__, fd);
		goto out;
	}

	len = strlen(exce_dir);
	ret = memcpy_s(dst_str, LOG_PATH_LEN - 1, exce_dir, len);
	if (ret != EOK) {
		BB_PRINT_ERR("%s():%d:memcpy_s fail!\n", __func__, __LINE__);
		return;
	}
	ret = memcpy_s(&dst_str[len], LOG_PATH_LEN - strlen(dst_str), "/kerneldump_", strlen("/kerneldump_") + 1);
	if (ret != EOK) {
		BB_PRINT_ERR("%s():%d:memcpy_s fail!\n", __func__, __LINE__);
		return;
	}

	strncat(dst_str, date, ((LOG_PATH_LEN - 1) - strlen(dst_str)));
	BB_PRINT_PN("%s: %s\n", __func__, dst_str);

	if (check_himntn(HIMNTN_KERNEL_DUMP_ENABLE)) {
		/*
		 * On FPGA it will take half one hour to transfer kerneldump,
		 * it's too slowly and useless to transfer kerneldump.
		 * We just create the kerneldump file name.
		 */
		if (g_bbox_fpga_flag == FPGA) {
			ret = rdr_copy_big_file_apend(dst_str, SRC_KERNELDUMP);
		} else {
			ret = skp_save_kdump_file(dst_str, exce_dir);
		}

		if (ret) {
			BB_PRINT_ERR("[%s], save kerneldump error, ret = %d\n", __func__, ret);
			goto out;
		}
	}

out:
	return;
}

void rdr_hisiap_dump_root_head(u32 modid, u32 etype, u64 coreid)
{
	if (!g_rdr_ap_root) {
		BB_PRINT_ERR("[%s], exit!\n", __func__);
		return;
	}

	g_rdr_ap_root->modid = modid;
	g_rdr_ap_root->e_exce_type = etype;
	g_rdr_ap_root->coreid = coreid;
	g_rdr_ap_root->slice = get_32k_abs_timer_value();
	g_rdr_ap_root->enter_times++;
}

void rdr_hisiap_dump(u32 modid, u32 etype,
				u64 coreid, char *log_path,
				pfn_cb_dump_done pfn_cb)
{
	uintptr_t exception_info = 0;
	unsigned long exception_info_len = 0;
	int ret;

	BB_PRINT_PN("[%s], begin\n", __func__);
	BB_PRINT_PN("modid is 0x%x, etype is 0x%x\n", modid, etype);

	if (!g_rdr_ap_init) {
		BB_PRINT_ERR("rdr_hisi_ap_adapter is not ready\n");
		if (pfn_cb)
			pfn_cb(modid, coreid);
		return;
	}

	if (modid == RDR_MODID_AP_ABNORMAL_REBOOT ||
		modid == BBOX_MODID_LAST_SAVE_NOT_DONE) {
		BB_PRINT_PN("RDR_MODID_AP_ABNORMAL_REBOOT\n");
		if (log_path && check_himntn(HIMNTN_GOBAL_RESETLOG)) {
			strncpy(g_log_path, log_path, LOG_PATH_LEN - 1);
			g_log_path[LOG_PATH_LEN - 1] = '\0';

			save_hisiap_log(log_path, modid);
		}

		if (pfn_cb)
			pfn_cb(modid, coreid);
		return;
	}

	console_loglevel = RDR_CONSOLE_LOGLEVEL_DEFAULT;

	/* If etype is panic, record the pc pointer and transfer it to the fastboot */
	if ((etype == AP_S_PANIC || etype == AP_S_VENDOR_PANIC) && g_bbox_ap_record_pc) {
		get_exception_info(&exception_info, &exception_info_len);
		memset(g_bbox_ap_record_pc->exception_info, 0,
			RECORD_PC_STR_MAX_LENGTH);
		ret = memcpy_s(g_bbox_ap_record_pc->exception_info, RECORD_PC_STR_MAX_LENGTH,
			(char *)exception_info, exception_info_len);
		if (ret != EOK) {
			BB_PRINT_ERR("%s():%d:memcpy_s fail!\n", __func__, __LINE__);
			return;
		}
		BB_PRINT_PN("exception_info is [%s],len is [%lu]\n",
			(char *)exception_info, exception_info_len);
		g_bbox_ap_record_pc->exception_info_len = exception_info_len;
	}

	BB_PRINT_PN("%s modid[%x],etype[%x],coreid[%llx], log_path[%s]\n", __func__,
		modid, etype, coreid, log_path);
	BB_PRINT_PN("[%s], hisi_trace_hook_uninstall start!\n", __func__);
	hisi_trace_hook_uninstall();

	if (!g_rdr_ap_root)
		goto out;

	g_rdr_ap_root->modid = modid;
	g_rdr_ap_root->e_exce_type = etype;
	g_rdr_ap_root->coreid = coreid;
	BB_PRINT_PN("%s log_path_ptr [%pK]\n", __func__, g_rdr_ap_root->log_path);
	if (log_path) {
		strncpy(g_rdr_ap_root->log_path, log_path, LOG_PATH_LEN - 1);
		g_rdr_ap_root->log_path[LOG_PATH_LEN - 1] = '\0';
	}

	g_rdr_ap_root->slice = get_32k_abs_timer_value();

	BB_PRINT_PN("[%s], regs_dump start!\n", __func__);
	regs_dump();

	BB_PRINT_PN("[%s], last_task_stack_dump start!\n", __func__);
	last_task_stack_dump();

	g_rdr_ap_root->enter_times++;

	BB_PRINT_PN("[%s], save_module_dump_mem start!\n", __func__);
	save_module_dump_mem();

	print_debug_info();

	show_irq_register();
out:
	BB_PRINT_PN("[%s], exit!\n", __func__);
	if (pfn_cb)
		pfn_cb(modid, coreid);
}

int get_pmu_reset_base_addr(void)
{
	u64 pmu_reset_reg_addr = (u64)0;
	unsigned int fpga_flag = 0;
	struct device_node *np = NULL;
	int ret;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,hisifb");
	if (!np) {
		BB_PRINT_ERR("NOT FOUND device node 'hisilicon,hisifb'!\n");
		return -ENXIO;
	}
	ret = of_property_read_u32(np, "fpga_flag", &fpga_flag);
	if (ret) {
		BB_PRINT_ERR("failed to get fpga_flag resource\n");
		return -ENXIO;
	}
	if (fpga_flag == FPGA) {
		g_bbox_fpga_flag = FPGA;
		pmu_reset_reg_addr = FPGA_RESET_REG_ADDR;
		g_pmu_reset_reg = (unsigned long long)(uintptr_t)hisi_bbox_map(pmu_reset_reg_addr, REG_MAP_SIZE);
		if (!g_pmu_reset_reg) {
			BB_PRINT_ERR("get pmu reset reg error\n");
			return -1;
		}
		g_pmu_subtype_reg = (unsigned long long)(uintptr_t)
			hisi_bbox_map(FPGA_EXCSUBTYPE_REG_ADDR, REG_MAP_SIZE);
		if (!g_pmu_subtype_reg) {
			BB_PRINT_ERR("get pmu subtype reg error\n");
			return -1;
		}
		BB_PRINT_DBG("pmu reset reg phy is 0x%llx\n", pmu_reset_reg_addr);
	}
	return 0;
}

/*
 * Description : Obtains the address of the pmu register recording the reset cause
 */
unsigned long long get_pmu_reset_reg(void)
{
	return g_pmu_reset_reg;
}

/*
 * Description : Obtains the address of the pmu register recording the subtype of the reset reason
 */
unsigned long long get_pmu_subtype_reg(void)
{
	return g_pmu_subtype_reg;
}

/*
 * Description : Set reboot reason to pmu
 */
void set_reboot_reason(unsigned int reboot_reason)
{
	unsigned int value = 0;
	uintptr_t pmu_reset_reg;

	if (g_bbox_fpga_flag == FPGA) {
		pmu_reset_reg = get_pmu_reset_reg();
		if (pmu_reset_reg)
			value = readl((char *)pmu_reset_reg);
	} else {
		value = pmic_read_reg(PMU_RESET_REG_OFFSET);
	}
	value &= (PMU_RESET_REG_MASK);
	reboot_reason &= (RST_FLAG_MASK);
	value |= reboot_reason;
	if (g_bbox_fpga_flag == FPGA) {
		pmu_reset_reg = get_pmu_reset_reg();
		if (pmu_reset_reg)
			writel(value, (char *)pmu_reset_reg);
	} else {
		pmic_write_reg(PMU_RESET_REG_OFFSET, value);
	}
	BB_PRINT_PN("[%s]:set reboot_reason is 0x%x\n", __func__, value);
}

/*
 * Description : Obtaining the reboot reason
 */
unsigned int get_reboot_reason(void)
{
	unsigned int value = 0;
	uintptr_t pmu_reset_reg;

	if (g_bbox_fpga_flag == FPGA) {
		pmu_reset_reg = get_pmu_reset_reg();
		if (pmu_reset_reg)
			value = readl((char *)pmu_reset_reg);
	} else {
		value = pmic_read_reg(PMU_RESET_REG_OFFSET);
	}
	value &= RST_FLAG_MASK;
	return value;
}

/*
 * Description : register exceptionwith to rdr
 */
static void rdr_hisiap_register_exception(void)
{
	unsigned int i;
	u32 ret;

	for (i = 0; i < sizeof(g_einfo) / sizeof(struct rdr_exception_info_s); i++) {
		BB_PRINT_DBG("register exception:%u", g_einfo[i].e_exce_type);
		g_einfo[i].e_callback = hisiap_callback;
		if (i == 0)
			/*
			 * Register the common callback function of the AP core.
			 * If other cores notify the ap core dump, invoke the callback function.
			 * RDR_COMMON_CALLBACK is a public callback tag,
			 * and ap core is a private callback function that is not marked
			 */
			g_einfo[i].e_callback = (rdr_e_callback)(uintptr_t)(
				(uintptr_t)(g_einfo[i].e_callback) | BBOX_COMMON_CALLBACK);
		ret = rdr_register_exception(&g_einfo[i]);
		if (ret == 0)
			BB_PRINT_ERR("rdr_register_exception fail, ret = [%u]\n", ret);
	}
	BB_PRINT_PN("[%s], end\n", __func__);
}

/*
 * Description : Register the dump and reset functions to the rdr
 */
static int rdr_hisiap_register_core(void)
{
	struct rdr_module_ops_pub s_soc_ops;
	struct rdr_register_module_result retinfo;
	int ret;
	u64 coreid = RDR_AP;

	s_soc_ops.ops_dump = rdr_hisiap_dump;
	s_soc_ops.ops_reset = rdr_hisiap_reset;

	ret = rdr_register_module_ops(coreid, &s_soc_ops, &retinfo);
	if (ret < 0) {
		BB_PRINT_ERR("rdr_register_module_ops fail, ret = [%d]\n", ret);
		return ret;
	}

	ret = rdr_register_cleartext_ops(coreid, rdr_hisiap_cleartext_print);
	if (ret < 0) {
		BB_PRINT_ERR("rdr_register_cleartext_ops fail, ret = [%d]\n", ret);
		return ret;
	}

	g_current_info.log_addr = retinfo.log_addr;
	g_current_info.log_len = retinfo.log_len;
	g_current_info.nve = retinfo.nve;

	return ret;
}

/*
 * Description : Obtains the initialization status
 */
bool rdr_get_ap_init_done(void)
{
	return g_rdr_ap_init == 1;
}

/*
 * Description:    Saving exception information
 * Input:          void *arg Not Used now
 * Return:         -1 : error, 0 : ok
 */
int save_exception_info(void *arg)
{
	int ret;
	char exce_dir[LOG_PATH_LEN];
	char dst_dir_str[DEST_LOG_PATH_LEN];

	BB_PRINT_ERR("[%s], start\n", __func__);

	/* Obtain the path of the abnormal log directory from the global variable */
	(void)memset_s(exce_dir, LOG_PATH_LEN, 0, LOG_PATH_LEN);

	if (LOG_PATH_LEN - 1 < strlen(g_log_path)) {
		BB_PRINT_ERR("g_log_path's len too large\n");
		return -1;
	}
	ret = memcpy_s(exce_dir, LOG_PATH_LEN, g_log_path, strlen(g_log_path) + 1);
	if (ret != EOK) {
		BB_PRINT_ERR("%s():%d:memcpy_s fail!\n", __func__, __LINE__);
		return -1;
	}
	BB_PRINT_ERR("exce_dir is [%s]\n", exce_dir);

	ret = create_exception_dir(exce_dir, dst_dir_str, sizeof(dst_dir_str));
	if (ret < 0) {
		BB_PRINT_ERR("[%s],create_exception_dir failed err\n", __func__);
		return -1;
	}
	BB_PRINT_ERR("dst_dir_str is [%s]\n", dst_dir_str);

	save_fastboot_log(dst_dir_str);
	save_pstore_info(dst_dir_str);

	/*
	 * If the input parameter of the function is null,
	 * it indicates that the function is invoked in rdr_dfx_ap_dump
	 */
	if (!arg)
		return ret;

	/* Create the DONE file in the abnormal directory, indicating that the exception log is saved done */
	bbox_save_done(g_log_path, BBOX_SAVE_STEP_DONE);

	/* The file system sync ensures that the read and write tasks are complete */
	if (!in_atomic() && !irqs_disabled() && !in_irq())
		SYS_SYNC();

	/*
	 * According to the permission requirements, the group of the dfx_logs directory
	 * and its subdirectories is changed to root-system
	 */
	ret = (int)bbox_chown((const char __user *)g_log_path, ROOT_UID, SYSTEM_GID, true);
	if (ret)
		BB_PRINT_ERR("[%s], chown %s uid [%u] gid [%u] failed err [%d]!\n",
			__func__, PATH_ROOT, ROOT_UID, SYSTEM_GID, ret);
	return ret;
}

void flush_logbuff_range(void)
{
	__dma_map_area((void *)hisirdr_ex_log_buf,
		hisirdr_ex_log_buf_len, DMA_TO_DEVICE);
	__dma_map_area((void *)hisirdr_ex_log_first_idx,
		sizeof(u32), DMA_TO_DEVICE);
	__dma_map_area((void *)hisirdr_ex_log_next_idx,
		sizeof(u32), DMA_TO_DEVICE);
}

static void rdr_hisiap_logbuff_head_crc(void)
{
	g_logbuff_dump_info->crc = 0;
	g_logbuff_dump_info->crc = checksum32((u32 *)g_logbuff_dump_info, sizeof(*g_logbuff_dump_info));
}

/*
 * Description : panic reset hook function at the initial stage of startup
 */
static int rdr_hisiap_early_panic_notify(struct notifier_block *nb,
				unsigned long event, void *buf)
{
	BB_PRINT_PN("%s ++\n", __func__);

	if (g_logbuff_dump_info)
		g_logbuff_dump_info->reboot_reason = AP_S_PANIC;

	rdr_hisiap_logbuff_head_crc();

	flush_logbuff_range();

	if (check_himntn(HIMNTN_PANIC_INTO_LOOP))
		do {} while (1);
	hisiap_nmi_notify_lpm3();

	BB_PRINT_PN("%s --\n", __func__);

	return 0;
}
/*
 * Description : Initialization Function
 */
int __init rdr_hisiap_init(void)
{
	struct task_struct *recordTask = NULL;
	u32 reboot_type;
	u32 bootup_keypoint;
	int ret;

	BB_PRINT_PN("%s init start\n", __func__);

	mutex_init(&g_dump_mem_mutex);

	ret = hisiap_nmi_notify_init();
	if (ret)
		return ret;

	rdr_hisiap_register_exception();
	ret = rdr_hisiap_register_core();
	if (ret) {
		BB_PRINT_ERR("%s rdr_hisiap_register_core fail, ret = [%d]\n", __func__, ret);
		return ret;
	}

	ret = register_mntn_dump(MNTN_DUMP_PANIC, sizeof(AP_RECORD_PC), (void **)&g_bbox_ap_record_pc);
	if (ret) {
		BB_PRINT_ERR("%s register g_bbox_ap_record_pc fail\n", __func__);
		return ret;
	}

	if (!g_bbox_ap_record_pc)
		BB_PRINT_ERR("%s g_bbox_ap_record_pc is NULL\n", __func__);

	g_hisiap_addr =
		(uintptr_t)hisi_bbox_map(g_current_info.log_addr +
				PMU_RESET_RECORD_DDR_AREA_SIZE,
				g_current_info.log_len - PMU_RESET_RECORD_DDR_AREA_SIZE);
	if (!g_hisiap_addr) {
		BB_PRINT_ERR("hisi_bbox_map g_hisiap_addr fail\n");
		return -1;
	}
	BB_PRINT_PN("[%s], retinfo: log_addr [0x%llx][0x%llx]",
		__func__, g_current_info.log_addr, g_hisiap_addr);

	ret = rdr_hisiap_dump_init(&g_current_info);
	if (ret) {
		BB_PRINT_ERR("%s rdr_hisiap_dump_init fail, ret = [%d]\n", __func__, ret);
		return -1;
	}
	reboot_type = rdr_get_reboot_type();
	bootup_keypoint = get_last_boot_keypoint();

	if (reboot_type < REBOOT_REASON_LABEL1 &&
		(!(reboot_type == AP_S_PRESS6S && bootup_keypoint != STAGE_BOOTUP_END)) &&
			check_himntn(HIMNTN_GOBAL_RESETLOG)) {
		recordTask = kthread_run(record_reason_task, NULL, "recordTask");
		pr_info("%s: create record_reason_task, return %pK\n", __func__, recordTask);
	}

	atomic_notifier_chain_unregister(&panic_notifier_list, &rdr_hisiap_early_panic_block);

	atomic_notifier_chain_register(&panic_notifier_list, &acpu_panic_loop_block);
	atomic_notifier_chain_register(&panic_notifier_list, &rdr_hisiap_panic_block);

	panic_on_oops = 1;
	register_die_notifier(&rdr_hisiap_die_block);
	powerkey_register_notifier(&rdr_hisiap_powerkey_block);
	get_bbox_curtime_slice();
	bbox_diaginfo_init();
#ifdef CONFIG_HISI_HW_DIAG
	hisi_hw_diag_init();
#endif

	g_rdr_ap_init = 1;

	BB_PRINT_PN("%s init end\n", __func__);

	return 0;
}

static void __exit rdr_hisiap_exit(void)
{
	BB_PRINT_PN("%s\n", __func__);
}

module_init(rdr_hisiap_init);
module_exit(rdr_hisiap_exit);


int rdr_hisiap_early_init(void)
{
	int ret;

	ret = hisiap_nmi_notify_init();
	if (ret)
		return ret;
	ret = register_mntn_dump(MNTN_DUMP_LOGBUF, sizeof(*g_logbuff_dump_info),
		(void **)&g_logbuff_dump_info);
	if (ret < 0) {
		BB_PRINT_ERR("[%s]register_mntn_dump error\n", __func__);
		return ret;
	}

	g_logbuff_dump_info->reboot_reason = 0;
	g_logbuff_dump_info->logbuf_addr = virt_to_phys(hisirdr_ex_log_buf);
	g_logbuff_dump_info->logbuf_size = hisirdr_ex_log_buf_len;
	g_logbuff_dump_info->idx_size = sizeof(u32);
	g_logbuff_dump_info->log_first_idx_addr = virt_to_phys(hisirdr_ex_log_first_idx);
	g_logbuff_dump_info->log_next_idx_addr = virt_to_phys(hisirdr_ex_log_next_idx);
	g_logbuff_dump_info->magic = LOGBUF_DUMP_MAGIC;

	rdr_hisiap_logbuff_head_crc();

	atomic_notifier_chain_register(&panic_notifier_list, &rdr_hisiap_early_panic_block);
	return 0;
}


