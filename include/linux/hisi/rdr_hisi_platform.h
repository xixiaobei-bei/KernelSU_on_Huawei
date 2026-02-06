#ifndef __RDR_HISI_PLATFORM_H__
#define __RDR_HISI_PLATFORM_H__
#include <linux/hisi/rdr_types.h>
#include <linux/hisi/rdr_pub.h>
#include <mntn_public_interface.h>

/* Confirm this definition is same as in the kernel/drivers/hisi/modem/drv/om/dump/rdr_adp.h */
#define RDR_MODEM_NOC_MOD_ID 0x82000030
#define RDR_MODEM_DMSS_MOD_ID 0x82000031
#define RDR_AUDIO_NOC_MODID 0x84000021

/* PMU register mask */
#define PMU_RESET_REG_MASK PMU_RESET_VALUE_USED

/* record log length */
#define LOG_PATH_LEN    96
#define DEST_LOG_PATH_LEN    (LOG_PATH_LEN + 10)
#define NEXT_LOG_PATH_LEN    (LOG_PATH_LEN + 30)


typedef enum {
	MODID_AP_START            = HISI_BB_MOD_AP_START,
	MODID_AP_S_PANIC          = 0x80000001,
	MODID_AP_S_NOC            = 0x80000002,
	MODID_AP_S_PMU            = 0x80000003,
	MODID_AP_S_DDRC_SEC       = 0x80000004,
	MODID_AP_S_SMPL           = 0x80000005,
	MODID_AP_S_COMBINATIONKEY = 0x80000006,
	MODID_AP_S_SUBPMU         = 0x80000007,
	MODID_AP_S_MAILBOX        = 0x80000008,
	MODID_AP_S_SCHARGER       = 0x80000009,
	MODID_AP_S_F2FS           = 0x8000000a,
	MODID_AP_S_BL31_PANIC     = 0x8000000b,
	MODID_AP_S_RESUME_SLOWY   = 0x8000000c,
	MODID_CHARGER_S_WDT       = 0x8000000d,
	MODID_AP_S_HHEE_PANIC     = 0x8000000e,
	MODID_AP_S_WDT             = 0x8000000f,
	MODID_AP_S_L3CACHE_ECC1    = 0x8000001e,
	MODID_AP_S_L3CACHE_ECC2    = 0x8000001f,
	MODID_AP_S_PANIC_SOFTLOCKUP = 0x80000020,
	MODID_AP_S_PANIC_OTHERCPU_HARDLOCKUP = 0x80000021,
	MODID_AP_S_PANIC_SP805_HARDLOCKUP = 0x80000022,
	MODID_AP_S_PANIC_STORAGE = 0x80000023,
	MODID_AP_S_PANIC_ISP     = 0x80000025,
	MODID_AP_S_PANIC_IVP     = 0x80000026,
	MODID_AP_S_PANIC_GPU     = 0x80000028,
	MODID_AP_S_PANIC_AUDIO_CODEC  = 0x80000029,
	MODID_AP_S_PANIC_MODEM   = 0x80000030,
	MODID_AP_S_PANIC_LB   = 0x80000035,
	MODID_AP_S_PANIC_PLL_UNLOCK   = 0x80000036,

	/* CPU EDAC ECC */
	MODID_AP_S_CPU0_L1_CE    = 0x80000050,
	MODID_AP_S_CPU1_L1_CE    = 0x80000051,
	MODID_AP_S_CPU2_L1_CE    = 0x80000052,
	MODID_AP_S_CPU3_L1_CE    = 0x80000053,
	MODID_AP_S_CPU4_L1_CE    = 0x80000054,
	MODID_AP_S_CPU5_L1_CE    = 0x80000055,
	MODID_AP_S_CPU6_L1_CE    = 0x80000056,
	MODID_AP_S_CPU7_L1_CE    = 0x80000057,
	MODID_AP_S_CPU0_L2_CE    = 0x80000058,
	MODID_AP_S_CPU1_L2_CE    = 0x80000059,
	MODID_AP_S_CPU2_L2_CE    = 0x8000005a,
	MODID_AP_S_CPU3_L2_CE    = 0x8000005b,
	MODID_AP_S_CPU4_L2_CE    = 0x8000005c,
	MODID_AP_S_CPU5_L2_CE    = 0x8000005d,
	MODID_AP_S_CPU6_L2_CE    = 0x8000005e,
	MODID_AP_S_CPU7_L2_CE    = 0x8000005f,
	MODID_AP_S_CPU0_L1_UE    = 0x80000060,
	MODID_AP_S_CPU1_L1_UE    = 0x80000061,
	MODID_AP_S_CPU2_L1_UE    = 0x80000062,
	MODID_AP_S_CPU3_L1_UE    = 0x80000063,
	MODID_AP_S_CPU4_L1_UE    = 0x80000064,
	MODID_AP_S_CPU5_L1_UE    = 0x80000065,
	MODID_AP_S_CPU6_L1_UE    = 0x80000066,
	MODID_AP_S_CPU7_L1_UE    = 0x80000067,
	MODID_AP_S_CPU0_L2_UE    = 0x80000068,
	MODID_AP_S_CPU1_L2_UE    = 0x80000069,
	MODID_AP_S_CPU2_L2_UE    = 0x8000006a,
	MODID_AP_S_CPU3_L2_UE    = 0x8000006b,
	MODID_AP_S_CPU4_L2_UE    = 0x8000006c,
	MODID_AP_S_CPU5_L2_UE    = 0x8000006d,
	MODID_AP_S_CPU6_L2_UE    = 0x8000006e,
	MODID_AP_S_CPU7_L2_UE    = 0x8000006f,
	MODID_AP_S_L3_CE    = 0x80000070,
	MODID_AP_S_L3_UE    = 0x80000071,

	/* DDRC_SEC SUBTYPE REASON */
	MODID_DMSS_START        = 0x80000100,
	MODID_DMSS_END          = 0x80000200,

	/* Exceptions for Huawei Device Co. Ltd. */
	MODID_AP_S_VENDOR_BEGIN = 0x80100000,
	MODID_AP_S_VENDOR_END   = 0x801fffff,

	MODID_AP_END              = HISI_BB_MOD_AP_END
} modid_ap;

typedef enum {
	MODU_NOC,
	MODU_DDR,
	MODU_TMC,
	MODU_MNTN_FAC,
	MODU_SMMU,
	MODU_QIC,
	MODU_GAP, /* 256 byte space as the gap, adding modules need before this */
	MODU_MAX
} dump_mem_module;
#ifdef CONFIG_HISI_MNTN_GT_WATCH_SUPPORT
enum {
	GT_WATCH_POWER_OFF = 0x00,      /* whole machine shutdown */
	GT_WATCH_LOWPOWER_OFF = 0x01,   /* low power shutdown */
	GT_WATCH_NOW_POWER_OFF = 0X02,  /* immediately shutdown */
	GT_WATCH_RESET = 0x03,          /* whole machine reset */
	GT_WATCH_X_POWER_OFF = 0x04,    /* x seconds whole machine shutdown */
	GT_WATCH_AP_RESET = 0x07,       /* AP reset */
	GT_WATCH_MCU_RESET = 0x08,
	GT_WATCH_AP_POWER_OFF = 0x09
};

enum {
	GT_WATCH_DISABLE = 0,
	GT_WATCH_A = 1,      /* ap depends the main processor poweroff */
	GT_WATCH_B = 2,      /* ap is not depends the main processor poweroff */
};

#endif
#ifdef CONFIG_HISI_CORESIGHT_TRACE
#define		ETR_DUMP_NAME		"etr_dump.ad"
#endif

typedef int (*rdr_hisiap_dump_func_ptr) (void *dump_addr, unsigned int size);

#ifdef CONFIG_HISI_BB

extern int g_bbox_fpga_flag;

void save_module_dump_mem(void);
void regs_dump(void);
void hisiap_nmi_notify_lpm3(void);
int register_module_dump_mem_func(rdr_hisiap_dump_func_ptr func,
				const char *module_name, dump_mem_module modu);
int get_module_dump_mem_addr(dump_mem_module modu, unsigned char **dump_addr);
void set_exception_info(unsigned long address);
bool rdr_get_ap_init_done(void);
unsigned long long get_pmu_reset_reg(void);
void set_reboot_reason(unsigned int reboot_reason);
unsigned int get_reboot_reason(void);
void hisi_pmic_powerkey_only_status_get(void);
int rdr_press_key_to_fastboot(struct notifier_block *nb,
		unsigned long event, void *buf);
void rdr_long_press_powerkey(void);
unsigned long long get_pmu_subtype_reg(void);
void set_subtype_exception(unsigned int subtype, bool save_value);
unsigned int get_subtype_exception(void);
char *rdr_get_subtype_name(u32 e_exce_type, u32 subtype);
char *rdr_get_category_name(u32 e_exce_type, u32 subtype);
u32 rdr_get_exec_subtype_value(void);
char *rdr_get_exec_subtype(void);
void flush_logbuff_range(void);
#ifdef CONFIG_HISI_EARLY_PANIC
void rdr_log_buf_notify_bl31(void);
#endif

#ifdef CONFIG_HISI_MNTN_GT_WATCH_SUPPORT
void hisi_pm_get_gt_watch_support_state(void);
unsigned int hisi_pm_gt_watch_type(void);
int hisi_pm_ap_pull_down_state(void);
#endif

#else
static inline void save_module_dump_mem(void) {}
static inline void regs_dump(void) {}
static inline void hisiap_nmi_notify_lpm3(void) {}
static inline void set_exception_info(unsigned long address) {}
static inline void set_reboot_reason(unsigned int reboot_reason) {}
static inline void rdr_long_press_powerkey(void) {}
static inline void set_subtype_exception(unsigned int subtype, bool save_value) {}
static inline void flush_logbuff_range(void) {}

static inline int register_module_dump_mem_func(rdr_hisiap_dump_func_ptr func,
				const char *module_name, dump_mem_module modu)
{
	return -1;
}

static inline int get_module_dump_mem_addr(dump_mem_module modu,
				unsigned char *dump_addr)
{
	return -1;
}

static inline bool rdr_get_ap_init_done(void)
{
	return 0;
}

static inline unsigned long long get_pmu_reset_reg(void)
{
	return 0;
}

static inline unsigned int get_reboot_reason(void)
{
	return 0;
}

static inline void hisi_pmic_powerkey_only_status_get(void)
{
	return;
}

static inline int rdr_press_key_to_fastboot(struct notifier_block *nb,
		unsigned long event, void *buf)
{
	return 0;
}

static inline unsigned long long get_pmu_subtype_reg(void)
{
	return 0;
}

static inline unsigned int get_subtype_exception(void)
{
	return 0;
}

static inline char *rdr_get_subtype_name(u32 e_exce_type, u32 subtype)
{
	return NULL;
}

static inline char *rdr_get_category_name(u32 e_exce_type, u32 subtype)
{
	return NULL;
}

static inline u32 rdr_get_exec_subtype_value(void)
{
	return 0;
}

static inline char *rdr_get_exec_subtype(void)
{
	return NULL;
}
#endif

#ifdef CONFIG_HISI_IRQ_REGISTER
void irq_register_hook(struct pt_regs *reg);
void show_irq_register(void);
#else
static inline void irq_register_hook(struct pt_regs *reg) {}
static inline void show_irq_register(void) {}
#endif

#ifdef CONFIG_HISI_REENTRANT_EXCEPTION
void reentrant_exception(void);
#else
static inline void reentrant_exception(void) {}
#endif

#ifdef CONFIG_HISI_BB_DEBUG
void last_task_stack_dump(void);
#else
static inline void last_task_stack_dump(void) {}
#endif

#endif
