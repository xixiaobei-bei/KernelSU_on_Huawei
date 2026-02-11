#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/semaphore.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/hisi/hisi_mailbox.h>
#include <linux/hisi/hisi_mailbox_dev.h>
#include <linux/hisi/rdr_pub.h>
#include <linux/hisi/hisi_rproc.h>
#include <linux/hisi/ipc_msg.h>
#include <linux/hisi/util.h>

#include "../../mntn_filesys.h"
#include <linux/hisi/rdr_hisi_platform.h>
#include "../rdr_print.h"
#include "../rdr_inner.h"
#include "../rdr_field.h"
#include "rdr_hisi_lpm3.h"

#include <soc_acpu_baseaddr_interface.h>
#include <soc_syscounter_interface.h>
#include <soc_sctrl_interface.h>
#include <global_ddr_map.h>
#include <m3_rdr_ddr_map.h>
#include <lpmcu_runtime_save_user.h>
#include <mntn_subtype_exception.h>
#include <securec.h>

#define M3_WDT_TIMEOUT_IRQ_NUM		206
#define AP_WDT_TIMEOUT_IRQ_NUM		77

enum RDR_LPM3_SYSTEM_ERROR_TYPE {
	M3_WDT_TIMEOUT = HISI_BB_MOD_LPM_START,
	AP_WDT_TIMEOUT,
	SYSTEM_ERROR_TYPE_MAX = HISI_BB_MOD_LPM_END,
};

#define LPMCU_RESET_OFF_MODID_PANIC	0x1
#define LPMCU_RESET_OFF_MODID_WDT	0x2

#define PSCI_MSG_TYPE_M3_CTXSAVE	IPC_CMD(OBJ_AP, OBJ_LPM3, CMD_INQUIRY, 0)
#define PSCI_MSG_TYPE_M3_WDTTIMEOUT	IPC_CMD(OBJ_AP, OBJ_LPM3, CMD_INQUIRY, 1)
#define PSCI_MSG_TYPE_M3_RDRBUF		IPC_CMD(OBJ_AP, OBJ_LPM3, CMD_INQUIRY, 2)
#define PSCI_MSG_TYPE_M3_PANIC_OFF	IPC_CMD(OBJ_AP, OBJ_LPM3, CMD_INQUIRY, 3)
#define PSCI_MSG_TYPE_M3_STAT_DUMP	IPC_CMD(OBJ_AP, OBJ_LPM3, CMD_INQUIRY, 4)
#define LPM3_RDR_SAVE_DONE		IPC_CMD(OBJ_LPM3, OBJ_AP, CMD_NOTIFY, TYPE_RESET)
/*lint +esym(750,*)*/

static u64 current_core_id = RDR_LPM3;
static struct rdr_register_module_result current_info;

u64 rdr_lpm3_buf_addr;
u32 rdr_lpm3_buf_len;

char *g_lpmcu_rdr_ddr_addr;
static struct semaphore rdr_lpm3_sem;
static volatile pfn_cb_dump_done pfn_cb_dumpdone;

static volatile u32 g_modid;

/*static char* counter_base;*/
static char *sctrl_base;

/*lpm3 log cleartext define*/
typedef int (*cleartext_func_t)(char *dir_path, struct file *fp, u64 log_addr, u32 log_len);

typedef struct LOG_LPM3_CLEARTEXT {
		s8 *log_name;
		u32 log_addr_offset;
		u32 log_len;
		cleartext_func_t cleartext_func;
} log_lpm3_cleartext_t;

static int system_reg_prase(char *dir_path, struct file *fp, u64 log_addr, u32 log_len);
static int head_info_prase(char *dir_path, struct file *fp, u64 log_addr, u32 log_len);
static int lpm3_exc_special_prase(char *dir_path, struct file *fp, u64 log_addr, u32 log_len);
static int lpm3_core_reg_prase(char *dir_path, struct file *fp, u64 log_addr, u32 log_len);
static int lpm3_nvic_reg_prase(char *dir_path, struct file *fp, u64 log_addr, u32 log_len);
static int lpm3_log_prase(char *dir_path, struct file *fp, u64 log_addr, u32 log_len);
static int lpmcu_runtime_prase(char *dir_path, struct file *fp, u64 log_addr, u32 log_len);

log_lpm3_cleartext_t g_lpm3_cleartext[] = {
		{"HEAD_INFO", M3_RDR_SYS_CONTEXT_HEAD_OFFSET, M3_RDR_SYS_CONTEXT_HEAD_SIZE, head_info_prase},
		{"LPM3_CORE_REGS", M3_RDR_SYS_CONTEXT_M3_COREREG_OFFSET, M3_RDR_SYS_CONTEXT_M3_COREREG_SIZE, lpm3_core_reg_prase},
		{"LPM3_EXC_SPECIAL", M3_RDR_SYS_CONTEXT_EXC_SPECIAL_OFFSET, M3_RDR_SYS_CONTEXT_EXC_SPECIAL_SIZE, lpm3_exc_special_prase},
		{"LPM3_NVIC_REGS", M3_RDR_SYS_CONTEXT_M3_NVICREG_OFFSET, M3_RDR_SYS_CONTEXT_M3_NVICREG_SIZE, lpm3_nvic_reg_prase},
		{"CRG_PERI_REGS", M3_RDR_CRG_PERI_OFFSET, M3_RDR_CRG_PERI_SIZE, system_reg_prase},
		{"SCTRL_REGS", M3_RDR_SCTRL_OFFSET, M3_RDR_SCTRL_SIZE, system_reg_prase},
		{"PMCTRL_REGS", M3_RDR_PMCTRL_OFFSET, M3_RDR_PMCTRL_SIZE, system_reg_prase},
		{"PCTRL_REGS", M3_RDR_PCTRL_OFFSET, M3_RDR_PCTRL_SIZE, system_reg_prase},
		{"RUNTIME_VAR_INFO", M3_RDR_SYS_CONTEXT_RUNTIME_VAR_OFFSET, M3_RDR_SYS_CONTEXT_RUNTIME_VAR_SIZE, lpmcu_runtime_prase},
		{"LPM3_LOG", M3_RDR_SYS_CONTEXT_M3_LOG_OFFSET, M3_RDR_SYS_CONTEXT_M3_LOG_SIZE, lpm3_log_prase},
};

/*lpm3 log cleartext -- head info*/
#define PC_LO_PWR_DOWN				(0x5CC55CC5)
#define PC_HI_PWR_DOWN				(0xC55CC55C)
#define PC_LO_LOCK_TIMEOUT			(0x5CC5DEDE)
#define PC_HI_LOCK_TIMEOUT			(0xC55CDEDE)
#define CPU_CORE_NUM				(8)
#define PC_INFO_STR_MAX_LENGTH		(90)

/*lpm3 nvic define*/
#define NVIC_TYPE_OFFSET			(0x0100)	/*E000E100*/
typedef struct
{
	u32 ISER[8];                 /*!< Offset: 0x000 (R/W)  Interrupt Set Enable Register           */
	u32 RESERVED0[24];
	u32 ICER[8];                 /*!< Offset: 0x080 (R/W)  Interrupt Clear Enable Register         */
	u32 RSERVED1[24];
	u32 ISPR[8];                 /*!< Offset: 0x100 (R/W)  Interrupt Set Pending Register          */
	u32 RESERVED2[24];
	u32 ICPR[8];                 /*!< Offset: 0x180 (R/W)  Interrupt Clear Pending Register        */
	u32 RESERVED3[24];
	u32 IABR[8];                 /*!< Offset: 0x200 (R/W)  Interrupt Active bit Register           */
	u32 RESERVED4[56];
	u32 IP[240];                 /*!< Offset: 0x300 (R/W)  Interrupt Priority Register (8Bit wide) */
	u32 RESERVED5[644];
	u32 STIR;                    /*!< Offset: 0xE00 ( /W)  Software Trigger Interrupt Register     */
} NVIC_Type;

#define SCB_TYPE_OFFSET			(0x0D00)	/*E000ED00*/
typedef struct
{
	u32 CPUID;                   /*!< Offset: 0x000 (R/ )  CPUID Base Register                                   */
	u32 ICSR;                    /*!< Offset: 0x004 (R/W)  Interrupt Control and State Register                  */
	u32 VTOR;                    /*!< Offset: 0x008 (R/W)  Vector Table Offset Register                          */
	u32 AIRCR;                   /*!< Offset: 0x00C (R/W)  Application Interrupt and Reset Control Register      */
	u32 SCR;                     /*!< Offset: 0x010 (R/W)  System Control Register                               */
	u32 CCR;                     /*!< Offset: 0x014 (R/W)  Configuration Control Register                        */
	u8  SHP[12];                 /*!< Offset: 0x018 (R/W)  System Handlers Priority Registers (4-7, 8-11, 12-15) */
	u32 SHCSR;                   /*!< Offset: 0x024 (R/W)  System Handler Control and State Register             */
	u32 CFSR;                    /*!< Offset: 0x028 (R/W)  Configurable Fault Status Register                    */
	u32 HFSR;                    /*!< Offset: 0x02C (R/W)  HardFault Status Register                             */
	u32 DFSR;                    /*!< Offset: 0x030 (R/W)  Debug Fault Status Register                           */
	u32 MMFAR;                   /*!< Offset: 0x034 (R/W)  MemManage Fault Address Register                      */
	u32 BFAR;                    /*!< Offset: 0x038 (R/W)  BusFault Address Register                             */
	u32 AFSR;                    /*!< Offset: 0x03C (R/W)  Auxiliary Fault Status Register                       */
	u32 PFR[2];                  /*!< Offset: 0x040 (R/ )  Processor Feature Register                            */
	u32 DFR;                     /*!< Offset: 0x048 (R/ )  Debug Feature Register                                */
	u32 ADR;                     /*!< Offset: 0x04C (R/ )  Auxiliary Feature Register                            */
	u32 MMFR[4];                 /*!< Offset: 0x050 (R/ )  Memory Model Feature Register                         */
	u32 ISAR[5];                 /*!< Offset: 0x060 (R/ )  Instruction Set Attributes Register                   */
	u32 RESERVED0[5];
	u32 CPACR;                   /*!< Offset: 0x088 (R/W)  Coprocessor Access Control Register                   */
} SCB_Type;

#define CORE_DEBUG_TYPE_OFFSET	(0x0DF0)	/*E000EDF0*/
typedef struct
{
	u32 DHCSR;                   /*!< Offset: 0x000 (R/W)  Debug Halting Control and Status Register    */
	u32 DCRSR;                   /*!< Offset: 0x004 ( /W)  Debug Core Register Selector Register        */
	u32 DCRDR;                   /*!< Offset: 0x008 (R/W)  Debug Core Register Data Register            */
	u32 DEMCR;                   /*!< Offset: 0x00C (R/W)  Debug Exception and Monitor Control Register */
} CoreDebug_Type;


/* ap address -> m3 address */
u64 address_map(u64 orig_addr)
{
	u64 mapped_addr = orig_addr;
	return mapped_addr;
}

void m3_nmi_send(void)
{
	/* SCLPMCUCTRL nmi_in */
	BB_PRINT_PN("%s start\n", __func__);
	/* rdr will send the NMI to lpm3 */
	if (sctrl_base) {
		writel(0x1 << 2, (void *)(SOC_SCTRL_SCLPMCUCTRL_ADDR(sctrl_base)));/* the addr need ioremap */
	} else {
		BB_PRINT_ERR("%s err\n", __func__);
	}
	BB_PRINT_PN("%s end\n", __func__);
}
#define LPMCU_DDR_MEM_PART_PATH_LEN 48UL
char g_lpmcu_ddr_memory_path[LOG_PATH_LEN + LPMCU_DDR_MEM_PART_PATH_LEN];
static void fn_dump(u32 modid, u32 etype, u64 coreid,
				char *pathname, pfn_cb_dump_done pfn_cb)
{
	s32 ret;
	u32 msg[2] = {PSCI_MSG_TYPE_M3_CTXSAVE, 0};

	BB_PRINT_PN("modid:0x%x,etype:0x%x,coreid:0x%llx,%s,pfn_cb:%pK\n", modid, etype, coreid, pathname, pfn_cb);
	msg[1] = modid;
	pfn_cb_dumpdone = pfn_cb;
	g_modid = modid;
	ret = strncpy_s(g_lpmcu_ddr_memory_path, LOG_PATH_LEN, pathname, LOG_PATH_LEN-1UL);
	if (ret != EOK)
		BB_PRINT_ERR("%s:strncpy_s failed:%d", __func__, ret);

	ret = RPROC_ASYNC_SEND(HISI_RPROC_LPM3_MBX17, (mbox_msg_t *)&msg, 2);
	if (ret != 0)
		BB_PRINT_ERR("%s:RPROC_ASYNC_SEND failed! return (0x%x)\n", __func__, ret);

	BB_PRINT_PN("%s end\n", __func__);
	return;
}
static void fn_reset(u32 modid, u32 etype, u64 coreid)
{
	return;
}/*lint !e715*/


static int rdr_lpm3_msg_handler(struct notifier_block *nb,
										unsigned long len, void *msg)
{
	u32 *_msg = msg;
	BB_PRINT_PN("%s, [lpm3] -> [ap]: 0x%x\n", __func__, _msg[0]);
	if (_msg[0] == LPM3_RDR_SAVE_DONE) {	/*lint !e845 *//* lpm3 -> ap: "my sys context save is done" */
		up(&rdr_lpm3_sem);
		BB_PRINT_PN("%s lpm3 tell me that its sys context has saved\n", __func__);
	}
	return 0;
}/*lint !e715 */

/* work for rdr lpm3 */
static int rdr_lpm3_thread_body(void *arg)
{
	s32 ret;
	char *lpmcu_ddr_base = NULL;

	while (1) {
		if (down_interruptible(&rdr_lpm3_sem))
			return -1;

		BB_PRINT_PN(" %s %d pfn_cb_dumpdone:%pK\n", __func__, __LINE__, pfn_cb_dumpdone);
		if (pfn_cb_dumpdone != NULL) {
			ret = strncat_s(g_lpmcu_ddr_memory_path, LPMCU_DDR_MEM_PART_PATH_LEN,
			                "/lpmcu_log/lpmcu_ddr_memory.bin", sizeof("/lpmcu_log/lpmcu_ddr_memory.bin"));
			if (ret != EOK)
				BB_PRINT_ERR("%s:strncpy_s failed:%d", __func__, ret);

			lpmcu_ddr_base = (char *)ioremap((phys_addr_t)HISI_RESERVED_LPMX_CORE_PHYMEM_BASE_UNIQUE, HISI_RESERVED_LPMX_CORE_PHYMEM_SIZE);/*lint !e747*/
			if (lpmcu_ddr_base) {
				mntn_filesys_write_log(g_lpmcu_ddr_memory_path, lpmcu_ddr_base, HISI_RESERVED_LPMX_CORE_PHYMEM_SIZE, 0);
				iounmap(lpmcu_ddr_base);
				lpmcu_ddr_base = NULL;
			}
			pfn_cb_dumpdone(g_modid, current_core_id);
			pfn_cb_dumpdone = NULL;
			BB_PRINT_PN("modid:0x%x,coreid:0x%llx\n", g_modid, current_core_id);
		}
	}
}/*lint !e715*/

int rdr_lpm3_reset_off(int mod, int sw)
{
	s32 ret = -1;
	unsigned int msg[2] = {0};
	u32 tmp;

	if (LPMCU_RESET_OFF_MODID_PANIC == mod) {
		msg[0] = PSCI_MSG_TYPE_M3_PANIC_OFF;
		msg[1] = (unsigned int)sw;

		ret = RPROC_ASYNC_SEND(HISI_RPROC_LPM3_MBX17, (mbox_msg_t *)msg, 2);
		if (ret != 0) {
			BB_PRINT_ERR("RPROC_ASYNC_SEND failed! return 0x%x, &msg:(%pK)\n", ret, msg);
			return ret;
		}
		BB_PRINT_PN("%s: (ap)->(lpm3) ipc send (0x%x 0x%x)!\n", __func__, msg[0], msg[1]);
	} else if (LPMCU_RESET_OFF_MODID_WDT == mod) {
		if (sctrl_base) {
			tmp = (unsigned int)readl(SOC_SCTRL_SCBAKDATA10_ADDR(sctrl_base));
			if (sw) {
				tmp &= ~((unsigned int)0x1 << 2);
			} else {
				tmp |= ((unsigned int)0x1 << 2);
			}
			writel(tmp, (void *)(SOC_SCTRL_SCBAKDATA10_ADDR(sctrl_base)));
		}
	}

	return 0; /*lint !e438*/

}/*lint !e550*/

int rdr_lpm3_stat_dump(void)
{
	s32 ret;
	unsigned int msg = PSCI_MSG_TYPE_M3_STAT_DUMP;

	ret = RPROC_ASYNC_SEND(HISI_RPROC_LPM3_MBX17, (mbox_msg_t *)&msg, 1);

	if (ret != 0) {
		BB_PRINT_ERR("RPROC_ASYNC_SEND failed! return 0x%x, msg:(0x%x)\n", ret, msg);
		return ret;
	}

	msleep(1);
	return 0;
}

static int system_reg_prase(char *dir_path, struct file *fp, u64 log_addr, u32 log_len)
{
	u32 i;
	bool error = false;

	if (IS_ERR_OR_NULL(fp)) {
		BB_PRINT_ERR("%s() error:fp 0x%pK.\n", __func__, fp);
		return -1;
	}

	rdr_cleartext_print(fp, &error, "offset      val\n");
	for(i = 0; i < log_len; i = i + 4) {
		rdr_cleartext_print(fp, &error, "0x%03x       0x%08x\n", i, *((u32 volatile *)(uintptr_t)(log_addr + i)));
	}

	if (true == error)
		return -1;

	return 0;
}

struct rdr_buf_head {
	u32 acore_pc[CPU_CORE_NUM * 2];
#ifdef CONFIG_HISI_MNTN_SP
	u32 acore_ls0_sp[CPU_CORE_NUM * 2];
	u32 acore_ls1_sp[CPU_CORE_NUM * 2];
#endif
};

static int head_info_prase(char *dir_path, struct file *fp, u64 log_addr, u32 log_len)
{
	u8 cpu_idx;
	s32 ret;
	bool error = false;
	u32 data_low, data_high;
	s8 pc_info[PC_INFO_STR_MAX_LENGTH];
	struct rdr_buf_head *head = NULL;

	if (IS_ERR_OR_NULL(fp)) {
		BB_PRINT_ERR("%s() error:fp 0x%pK.\n", __func__, fp);
		return -1;
	}


	head = (struct rdr_buf_head *)(uintptr_t)log_addr;
	for (cpu_idx = 0; cpu_idx < CPU_CORE_NUM; cpu_idx++) {
		pc_info[PC_INFO_STR_MAX_LENGTH - 1] = '\0';

		data_low = head->acore_pc[2 * cpu_idx];
		BB_PRINT_PN("lpm3 cleartest head_info low: 0x%x.\n", data_low);
		data_high = head->acore_pc[2 * cpu_idx + 1];
		BB_PRINT_PN("lpm3 cleartest head_info high: 0x%x.\n", data_high);

		if (data_low == PC_LO_PWR_DOWN && data_high == PC_HI_PWR_DOWN)
			ret = snprintf_s(pc_info, PC_INFO_STR_MAX_LENGTH, PC_INFO_STR_MAX_LENGTH - 1,
					"cpu%d: %s\n", cpu_idx, "PWR_DOWN");
		else if (data_low == PC_LO_LOCK_TIMEOUT && data_high == PC_HI_LOCK_TIMEOUT)
			ret = snprintf_s(pc_info, PC_INFO_STR_MAX_LENGTH, PC_INFO_STR_MAX_LENGTH - 1,
					"cpu%d: %s\n", cpu_idx, "LOCK_TIMEOUT");
		else {
#ifdef CONFIG_HISI_MNTN_SP
			ret = snprintf_s(pc_info, PC_INFO_STR_MAX_LENGTH, PC_INFO_STR_MAX_LENGTH - 1,
					"cpu%d PC: 0x%08x%08x LS0_SP: 0x%08x%08x LS1_SP: 0x%08x%08x\n",
					cpu_idx, data_high, data_low,
					head->acore_ls0_sp[2 * cpu_idx + 1], head->acore_ls0_sp[2 * cpu_idx],
					head->acore_ls1_sp[2 * cpu_idx + 1], head->acore_ls1_sp[2 * cpu_idx]
			);
#else
			ret = snprintf_s(pc_info, PC_INFO_STR_MAX_LENGTH, PC_INFO_STR_MAX_LENGTH - 1,
					"cpu%d PC: 0x%08x%08x\n", cpu_idx, data_high, data_low);
#endif
		}

		if (ret < 0) {
			BB_PRINT_ERR("[%s], snprintf_s ret %d!\n", __func__, ret);
			return -1;
		}

		rdr_cleartext_print(fp, &error, pc_info);
	}


	if (true == error)
		return -1;

	return 0;
}

static int lpm3_exc_special_prase(char *dir_path, struct file *fp, u64 log_addr, u32 log_len)
{
	u8 reason_idx;
	bool error = false;
	EXC_SPECIAL_BACKUP_DATA_STRU *p_lpm3_exc_special = NULL;

	if (IS_ERR_OR_NULL(fp)) {
		BB_PRINT_ERR("%s() error:fp 0x%pK.\n", __func__, fp);
		return -1;
	}

	p_lpm3_exc_special = (EXC_SPECIAL_BACKUP_DATA_STRU *)(uintptr_t)(log_addr);

	rdr_cleartext_print(fp, &error, "\n");
	for(reason_idx = 0; reason_idx < RDR_REG_BACKUP_IDEX_MAX; reason_idx++)
		rdr_cleartext_print(fp, &error, "reset_reason[%d]   0x%x\n", reason_idx, p_lpm3_exc_special->reset_reason[reason_idx]);

	rdr_cleartext_print(fp, &error, "slice             0x%x\n", p_lpm3_exc_special->slice);
	rdr_cleartext_print(fp, &error, "rtc               0x%x\n", p_lpm3_exc_special->rtc);
	rdr_cleartext_print(fp, &error, "R13               0x%x\n", p_lpm3_exc_special->REG_Reg13);
	rdr_cleartext_print(fp, &error, "LR1               0x%x\n", p_lpm3_exc_special->REG_LR1);
	rdr_cleartext_print(fp, &error, "PC                0x%x\n", p_lpm3_exc_special->REG_PC);
	rdr_cleartext_print(fp, &error, "XPSR              0x%x\n", p_lpm3_exc_special->REG_XPSR);
	rdr_cleartext_print(fp, &error, "NVIC_CFSR         0x%x\n", p_lpm3_exc_special->NVIC_CFSR);
	rdr_cleartext_print(fp, &error, "NVIC_HFSR         0x%x\n", p_lpm3_exc_special->NVIC_HFSR);
	rdr_cleartext_print(fp, &error, "NVIC_BFAR         0x%x\n", p_lpm3_exc_special->NVIC_BFAR);
	rdr_cleartext_print(fp, &error, "exc_trace         0x%x\n", p_lpm3_exc_special->exc_trace);
	rdr_cleartext_print(fp, &error, "ddr_exc           0x%x\n", p_lpm3_exc_special->ddr_exc);
	rdr_cleartext_print(fp, &error, "task_id           0x%x\n", p_lpm3_exc_special->task_id);


	if (true == error)
		return -1;

	return 0;
}

static int lpm3_core_reg_prase(char *dir_path, struct file *fp, u64 log_addr, u32 log_len)
{
	u8 reason_idx;
	bool error = false;
	RDR_REG_BACKUP_DATA_STRU *p_lpm3_core_reg = NULL;

	if (IS_ERR_OR_NULL(fp)) {
		BB_PRINT_ERR("%s() error:fp 0x%pK.\n", __func__, fp);
		return -1;
	}

	rdr_cleartext_print(fp, &error, "\n");

	for(reason_idx = 0; reason_idx < RDR_REG_BACKUP_IDEX_MAX; reason_idx++) {
		p_lpm3_core_reg = (RDR_REG_BACKUP_DATA_STRU *)(uintptr_t)(log_addr + reason_idx * sizeof(RDR_REG_BACKUP_DATA_STRU));

		rdr_cleartext_print(fp, &error, "****REGION %d****\n", reason_idx);
		rdr_cleartext_print(fp, &error, "R0:        0x%x\n", p_lpm3_core_reg->Reg0);
		rdr_cleartext_print(fp, &error, "R1:        0x%x\n", p_lpm3_core_reg->Reg1);
		rdr_cleartext_print(fp, &error, "R2:        0x%x\n", p_lpm3_core_reg->Reg2);
		rdr_cleartext_print(fp, &error, "R3:        0x%x\n", p_lpm3_core_reg->Reg3);
		rdr_cleartext_print(fp, &error, "R4:        0x%x\n", p_lpm3_core_reg->Reg4);
		rdr_cleartext_print(fp, &error, "R5:        0x%x\n", p_lpm3_core_reg->Reg5);
		rdr_cleartext_print(fp, &error, "R6:        0x%x\n", p_lpm3_core_reg->Reg6);
		rdr_cleartext_print(fp, &error, "R7:        0x%x\n", p_lpm3_core_reg->Reg7);
		rdr_cleartext_print(fp, &error, "R8:        0x%x\n", p_lpm3_core_reg->Reg8);
		rdr_cleartext_print(fp, &error, "R9:        0x%x\n", p_lpm3_core_reg->Reg9);
		rdr_cleartext_print(fp, &error, "R10:       0x%x\n", p_lpm3_core_reg->Reg10);
		rdr_cleartext_print(fp, &error, "R11:       0x%x\n", p_lpm3_core_reg->Reg11);
		rdr_cleartext_print(fp, &error, "R12:       0x%x\n", p_lpm3_core_reg->Reg12);
		rdr_cleartext_print(fp, &error, "R13:       0x%x\n", p_lpm3_core_reg->Reg13);
		rdr_cleartext_print(fp, &error, "MSP:       0x%x\n", p_lpm3_core_reg->MSP);
		rdr_cleartext_print(fp, &error, "PSP:       0x%x\n", p_lpm3_core_reg->PSP);
		rdr_cleartext_print(fp, &error, "LR0_CTRL:  0x%x\n", p_lpm3_core_reg->LR0_CONTROL);
		rdr_cleartext_print(fp, &error, "LR1:       0x%x\n", p_lpm3_core_reg->LR1);
		rdr_cleartext_print(fp, &error, "PC:        0x%x\n", p_lpm3_core_reg->PC);
		rdr_cleartext_print(fp, &error, "XPSR:      0x%x\n", p_lpm3_core_reg->XPSR);
		rdr_cleartext_print(fp, &error, "PRIMASK:   0x%x\n", p_lpm3_core_reg->PRIMASK);
		rdr_cleartext_print(fp, &error, "BASEPRI:   0x%x\n", p_lpm3_core_reg->BASEPRI);
		rdr_cleartext_print(fp, &error, "FAULTMASK: 0x%x\n", p_lpm3_core_reg->FAULTMASK);
		rdr_cleartext_print(fp, &error, "CONTROL:   0x%x\n", p_lpm3_core_reg->CONTROL);
		rdr_cleartext_print(fp, &error, "\n");
	}


	if (true == error)
		return -1;

	return 0;
}

static int lpm3_nvic_reg_prase(char *dir_path, struct file *fp, u64 log_addr, u32 log_len)
{
	u8 i;
	bool error = false;
	NVIC_Type *p_lpm3_nvic_reg = NULL;
	SCB_Type *p_lpm3_scb_reg = NULL;
	CoreDebug_Type *p_lpm3_core_debug_reg = NULL;


	if (IS_ERR_OR_NULL(fp)) {
		BB_PRINT_ERR("%s() error:fp 0x%pK.\n", __func__, fp);
		return -1;
	}

	p_lpm3_nvic_reg = (NVIC_Type *)(uintptr_t)(log_addr + NVIC_TYPE_OFFSET);
	p_lpm3_scb_reg = (SCB_Type *)(uintptr_t)(log_addr + SCB_TYPE_OFFSET);
	p_lpm3_core_debug_reg = (CoreDebug_Type *)(uintptr_t)(log_addr + CORE_DEBUG_TYPE_OFFSET);

	rdr_cleartext_print(fp, &error, "\n");

	for(i = 0; i < 8; i++)
		rdr_cleartext_print(fp, &error, "SETENA%d    0x%x\n", i, p_lpm3_nvic_reg->ISER[i]);
	for(i = 0; i < 8; i++)
		rdr_cleartext_print(fp, &error, "CLRENA%d    0x%x\n", i, p_lpm3_nvic_reg->ICER[i]);
	for(i = 0; i < 8; i++)
		rdr_cleartext_print(fp, &error, "SETPEND%d   0x%x\n", i, p_lpm3_nvic_reg->ISPR[i]);
	for(i = 0; i < 8; i++)
		rdr_cleartext_print(fp, &error, "CLRPEND%d   0x%x\n", i, p_lpm3_nvic_reg->ICPR[i]);
	for(i = 0; i < 8; i++)
		rdr_cleartext_print(fp, &error, "ACTIVE%d    0x%x\n", i, p_lpm3_nvic_reg->IABR[i]);

	rdr_cleartext_print(fp, &error, "CPUID      0x%x\n",  p_lpm3_scb_reg->CPUID);
	rdr_cleartext_print(fp, &error, "ICSR       0x%x\n",  p_lpm3_scb_reg->ICSR);
	rdr_cleartext_print(fp, &error, "VTOR       0x%x\n",  p_lpm3_scb_reg->VTOR);
	rdr_cleartext_print(fp, &error, "AIRCR      0x%x\n",  p_lpm3_scb_reg->AIRCR);
	rdr_cleartext_print(fp, &error, "SCR        0x%x\n",  p_lpm3_scb_reg->SCR);
	rdr_cleartext_print(fp, &error, "CCR        0x%x\n",  p_lpm3_scb_reg->CCR);
	for(i = 0; i < 12; i++)
		rdr_cleartext_print(fp, &error, "PRI_%02d     0x%x\n", (i + 4), p_lpm3_scb_reg->SHP[i]);
	rdr_cleartext_print(fp, &error, "SHCSR      0x%x\n",  p_lpm3_scb_reg->SHCSR);
	rdr_cleartext_print(fp, &error, "CFSR       0x%x\n",  p_lpm3_scb_reg->CFSR);
	rdr_cleartext_print(fp, &error, "HFSR       0x%x\n",  p_lpm3_scb_reg->HFSR);
	rdr_cleartext_print(fp, &error, "DFSR       0x%x\n",  p_lpm3_scb_reg->DFSR);
	rdr_cleartext_print(fp, &error, "MMFAR      0x%x\n",  p_lpm3_scb_reg->MMFAR);
	rdr_cleartext_print(fp, &error, "BFAR       0x%x\n",  p_lpm3_scb_reg->BFAR);
	rdr_cleartext_print(fp, &error, "AFSR       0x%x\n",  p_lpm3_scb_reg->AFSR);

	rdr_cleartext_print(fp, &error, "DHCSR      0x%x\n",  p_lpm3_core_debug_reg->DHCSR);
	rdr_cleartext_print(fp, &error, "DCRSR      0x%x\n",  p_lpm3_core_debug_reg->DCRSR);
	rdr_cleartext_print(fp, &error, "DCRDR      0x%x\n",  p_lpm3_core_debug_reg->DCRDR);
	rdr_cleartext_print(fp, &error, "DEMCR      0x%x\n",  p_lpm3_core_debug_reg->DEMCR);

	rdr_cleartext_print(fp, &error, "\n");

	if (true == error)
		return -1;

	return 0;
}

static int lpm3_log_prase(char *dir_path, struct file *fp, u64 log_addr, u32 log_len)
{
	s8 *log_buff = NULL;
	u32 log_size = log_len + 1;
	s32 ret = 0;

	if (IS_ERR_OR_NULL(fp)) {
		BB_PRINT_ERR("%s() error:fp 0x%pK.\n", __func__, fp);
		return -1;
	}

	log_buff = (s8 *)kzalloc(log_size, GFP_KERNEL);
	if (NULL == log_buff) {
		BB_PRINT_ERR("kmalloc fail for lpm3_log\n");
		return -1;
	}

	if (memcpy_s((void *)log_buff, log_len, (void *)(uintptr_t)(log_addr), log_len)) {
		BB_PRINT_ERR("memcpy fail for lpm3_log\n");
		ret = -1;
		goto out;
	}

	ret = vfs_write(fp, log_buff, (size_t)log_len, &(fp->f_pos));/*lint !e613 */
	if (ret != log_len) {
		BB_PRINT_ERR("%s():write file exception with ret %d.\n", __func__, ret);
		ret = -1;
		goto out;
	}
out:
	kfree(log_buff);

	return ret;
}

static int lpmcu_runtime_prase(char *dir_path, struct file *fp, u64 log_addr, u32 log_len)
{
	u32 i;
	u32 offset;
	u32 runtime_offset;
	bool error = false;

	if (IS_ERR_OR_NULL(fp)) {
		BB_PRINT_ERR("%s() error:fp 0x%pK.\n", __func__, fp);
		return -1;
	}

	rdr_cleartext_print(fp, &error, "offset      val\n");
	for(i = 0; i < sizeof(g_rdr_runtime_save_info) / sizeof(g_rdr_runtime_save_info[0]); i++) {
		runtime_offset = g_rdr_runtime_save_info[i].offset;
		for(offset = 0; offset < g_rdr_runtime_save_info[i].size && (offset + runtime_offset) < log_len; offset = offset + 4) {
			rdr_cleartext_print(fp, &error, "0x%03x       0x%08x\n",
					    (runtime_offset + offset),
					    *((u32 volatile *)(uintptr_t)(log_addr + runtime_offset + offset)));
		}
	}

	if (true == error)
		return -1;

	return 0;
}

static int rdr_hisi_lpm3_cleartext_print(char *dir_path, u64 log_addr, u32 log_len)
{
	bool error = false;
	u8 idx;
	u64 log_addr_prase;
	u32 log_len_prase;
	s32 ret = 0;
	struct file *fp = NULL;

	fp = bbox_cleartext_get_filep(dir_path, "LPM3.txt");
	if (IS_ERR_OR_NULL(fp)) {
		BB_PRINT_ERR("%s() error:fp 0x%pK.\n", __func__, fp);
		return -1;
	}

	if (IS_ERR_OR_NULL(dir_path) || IS_ERR_OR_NULL((void *)(uintptr_t)log_addr)) {
		BB_PRINT_ERR("%s() error:dir_path 0x%pK log_addr 0x%pK.\n", __func__, dir_path, (void *)(uintptr_t)log_addr);
		return -1;
	}

	if (M3_RDR_SYS_CONTEXT_SIZE != log_len) {
		BB_PRINT_ERR("log_len error:0x%x.\n", log_len);
		return -1;
	}

	for(idx = 0; idx < sizeof(g_lpm3_cleartext) / sizeof(log_lpm3_cleartext_t); idx++) {
		log_addr_prase = log_addr + g_lpm3_cleartext[idx].log_addr_offset;
		log_len_prase = g_lpm3_cleartext[idx].log_len;
		rdr_cleartext_print(fp, &error, "=================%s START================\n\n", g_lpm3_cleartext[idx].log_name);
		ret = g_lpm3_cleartext[idx].cleartext_func(dir_path, fp, log_addr_prase, log_len_prase);
		rdr_cleartext_print(fp, &error, "=================%s END================\n\n", g_lpm3_cleartext[idx].log_name);
	}
	bbox_cleartext_end_filep(fp, dir_path, "LPM3.txt");

	return ret;
}

static struct notifier_block rdr_ipc_block;
int __init rdr_lpm3_init(void)
{
	struct rdr_module_ops_pub s_module_ops;
	struct rdr_exception_info_s einfo;
	s32 ret = -1;
	static u32 msg[4] = {0};
	BB_PRINT_PN("enter %s\n", __func__);

	sctrl_base = (char *)ioremap((phys_addr_t)SOC_ACPU_SCTRL_BASE_ADDR, 0x1000UL);

	BB_PRINT_PN("sctrl_base: %pK\n", sctrl_base);

	rdr_ipc_block.next = NULL;
	rdr_ipc_block.notifier_call = rdr_lpm3_msg_handler;
	ret = RPROC_MONITOR_REGISTER(HISI_RPROC_RDR_MBX1, &rdr_ipc_block);/*lint !e838*/
	if (ret != 0) {
		BB_PRINT_ERR("%s:RPROC_MONITOR_REGISTER failed", __func__);
		return ret;
	}

	sema_init(&rdr_lpm3_sem, 0);
	if (!kthread_run(rdr_lpm3_thread_body, NULL, "rdr_lpm3_thread")) {
		BB_PRINT_ERR("%s: create thread rdr_main_thread faild.\n", __func__);
		return -1;
	}

	s_module_ops.ops_dump = fn_dump;
	s_module_ops.ops_reset = fn_reset;
	ret = rdr_register_module_ops(current_core_id, &s_module_ops, &current_info);
	if (ret != 0) {
		BB_PRINT_ERR("rdr_register_module_ops failed! return 0x%x\n", ret);
		return ret;
	}

	ret = rdr_register_cleartext_ops(current_core_id, rdr_hisi_lpm3_cleartext_print);
	if(ret < 0) {
		BB_PRINT_ERR("rdr_register_cleartext_ops failed! return 0x%x\n", ret);
		return ret;
	}
	BB_PRINT_PN("rdr_register_cleartext_success\n");

	ret = memset_s(&einfo, sizeof(einfo), 0, sizeof(struct rdr_exception_info_s));
	if (ret != EOK) {
		BB_PRINT_ERR("%s:%d, memset_s err:%d\n", __func__, __LINE__, ret);
		return ret;
	}

	einfo.e_modid = (unsigned int)M3_WDT_TIMEOUT;
	einfo.e_modid_end = (unsigned int)M3_WDT_TIMEOUT;
	einfo.e_process_priority = RDR_ERR;
	einfo.e_reboot_priority = RDR_REBOOT_WAIT;
	einfo.e_notify_core_mask = RDR_AP | RDR_LPM3; /*lint !e655*/
	einfo.e_reset_core_mask = RDR_LPM3;
	einfo.e_reentrant = (unsigned int)RDR_REENTRANT_DISALLOW;
	einfo.e_exce_type = LPM3_S_EXCEPTION;
	einfo.e_from_core = RDR_LPM3;
	ret = memcpy_s(einfo.e_from_module, sizeof(einfo.e_from_module), "RDR M3 WDT", sizeof("RDR M3 WDT"));
	if (ret != EOK) {
		BB_PRINT_ERR("%s:%d, memcpy_s err:%d\n", __func__, __LINE__, ret);
		return ret;
	}

	ret = memcpy_s(einfo.e_desc, sizeof(einfo.e_desc), "RDR M3 WDT", sizeof("RDR M3 WDT"));
	if (ret != EOK) {
		BB_PRINT_ERR("%s:%d, memcpy_s err:%d\n", __func__, __LINE__, ret);
		return ret;
	}

	(void)rdr_register_exception(&einfo);

	rdr_lpm3_buf_addr = address_map(current_info.log_addr);
	rdr_lpm3_buf_len = current_info.log_len;

	BB_PRINT_PN("%s: log_addr = 0x%llx, log_len = %u\n", __func__, current_info.log_addr, current_info.log_len);
	g_lpmcu_rdr_ddr_addr = (char *)hisi_bbox_map((phys_addr_t)current_info.log_addr, (unsigned long)current_info.log_len);

	if (g_lpmcu_rdr_ddr_addr){
		BB_PRINT_PN("rdr_ddr_addr success\n");
		(void)memset_s(g_lpmcu_rdr_ddr_addr, (size_t)rdr_lpm3_buf_len, 0 , (size_t)rdr_lpm3_buf_len);
	}

	msg[0] = PSCI_MSG_TYPE_M3_RDRBUF;
	msg[1] = (unsigned int)rdr_lpm3_buf_addr;
	msg[2] = rdr_lpm3_buf_addr >> 32;
	msg[3] = rdr_lpm3_buf_len;

	ret = RPROC_ASYNC_SEND(HISI_RPROC_LPM3_MBX17, (mbox_msg_t *)msg, 4);/*lint !e838*/

	if (ret != 0) {
		BB_PRINT_ERR("RPROC_ASYNC_SEND failed! return 0x%x, &msg:(%pK)\n", ret, msg);
	}
	BB_PRINT_PN("%s: (ap)->(lpm3) ipc send (0x%x 0x%x 0x%x 0x%x)!\n", __func__, msg[0], msg[1], msg[2], msg[3]);

	(void)rdr_lpm3_reset_off(LPMCU_RESET_OFF_MODID_PANIC, check_himntn(HIMNTN_LPM3_PANIC_INTO_LOOP));

	return ret;
}

static void __exit rdr_lpm3_exit(void)
{
	return;
}

/*lint -e528 -esym(528,__initcall_rdr_lpm3_init4,__exitcall_rdr_lpm3_exit)*/
subsys_initcall(rdr_lpm3_init);
module_exit(rdr_lpm3_exit);

