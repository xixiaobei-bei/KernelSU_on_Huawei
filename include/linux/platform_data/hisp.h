/*
 * hisp.h
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _LOAD_HISP_H_
#define _LOAD_HISP_H_

#define CONFIG_HISP_REMOTEPROC_DMAALLOC_DEBUG
#ifdef CONFIG_HISP_REMOTEPROC_DMAALLOC_DEBUG
#define HISP_VRING_NUM      3
#define HISP_VRING_SIZE     (0x3000)
#define HISP_VQUEUE_SIZE    (0x40000)
#endif

enum hisp_clk_type {
	ISPCPU_CLK   = 0,
	ISPFUNC_CLK  = 1,
	ISPI2C_CLK   = 2,
	VIVOBUS_CLK  = 3,
	ISPFUNC2_CLK = 3,
	ISPFUNC3_CLK = 4,
	ISPFUNC4_CLK = 5,
	ISPFUNC5_CLK = 6,
	ISP_SYS_CLK  = 7,
	ISPI3C_CLK   = 8,
	ISP_CLK_MAX
};

enum isp_pwstat_e {
	PWSTAT_MEDIA1	= 0,
	PWSTAT_ISPSYS	= 1,
	PWSTAT_MAX
};

enum a7mappint_type {
	A7BOOT = 0,
	A7TEXT,
	A7DATA,
	A7PGD,
	A7PMD,
	A7PTE,
	A7RDR,
	A7SHARED,
	A7VQ,
	A7VRING0,
	A7VRING1,
	A7HEAP,
	A7DYNAMIC,
	A7MDC,
	MAXA7MAPPING
};

enum hisp_clk_down_type {
	HISP_CLK_TURBO      = 0,
	HISP_CLK_NORMINAL   = 1,
	HISP_CLK_LOWSVS     = 2,
	HISP_CLK_HIGHSVS    = 3,
	HISP_CLK_SVS        = 4,
	HISP_CLK_DISDVFS    = 5,
	HISP_CLKDOWN_MAX
};

enum isp_tsmem_offset_info {
	ISP_TSMEM_OFFSET = 0,
	ISP_CPU_PARM_OFFSET,
	ISP_SHARE_MEM_OFFSET,
	ISP_DYN_MEM_OFFSET,
	ISP_TSMEM_MAX,
};

enum hisp_sec_boot_mem_type {
	HISP_SEC_TEXT       = 0,
	HISP_SEC_DATA       = 1,
	HISP_SEC_BOOT_MAX_TYPE
};

enum hisp_sec_mem_pool_type {
	HISP_DYNAMIC_POOL        = 0,
	HISP_SEC_POOL            = 1,
	HISP_ISPSEC_POOL         = 2,
	HISP_SEC_POOL_MAX_TYPE
};

enum hisp_sec_rsv_mem_type {
	HISP_SEC_VR0       = 0,
	HISP_SEC_VR1       = 1,
	HISP_SEC_VQ        = 2,
	HISP_SEC_SHARE     = 3,
	HISP_SEC_RDR       = 4,
	HISP_SEC_RSV_MAX_TYPE
};

enum isp_soc_reg_type {
	CRGPERI     = 0,
	ISPCORE     = 1,
	PMCTRL      = 2,
	PCTRL       = 3,
	SCTRL       = 4,
	VIVOBUS     = 5,
	CSSYS       = 6,
	SUBCTRL     = 7,
	WDT         = 8,
	MEDIA1      = 9,
	ACTRL       = 10,
	NOCDPM      = 11,
	HISP_MAX_REGTYPE
};

enum hisp_rproc_case_attr {
	SEC_CASE = 0,
	NONSEC_CASE,
	INVAL_CASE,
};

enum secisp_mem_type {
	SECISP_TEXT = 0,
	SECISP_DATA,
	SECISP_SEC_POOL,
	SECISP_ISPSEC_POOL,
	SECISP_DYNAMIC_POOL,
	SECISP_RDR,
	SECISP_SHRD,
	SECISP_VQ,
	SECISP_VR0,
	SECISP_VR1,
	SECISP_MAX_TYPE
};

#ifdef DEBUG_HISP
#define DUMP_ISPCPU_PC_TIMES    3

struct hisp_clk_dump_s {
	bool enable;
	unsigned int ispcpu_stat;
	unsigned int ispcpu_pc[DUMP_ISPCPU_PC_TIMES];
	unsigned long freq[ISP_CLK_MAX];
	unsigned int freqmask;
};
#endif

struct hisp_pwr {
	struct device *device;
	struct regulator *clockdep_supply;
	struct regulator *ispcore_supply;
	struct regulator *ispcpu_supply;
	struct regulator *isptcu_supply;
	unsigned int use_smmuv3_flag;
	unsigned int use_buck13_flag;
	unsigned int ispcpu_supply_flag;
	unsigned int clock_num;
	struct clk *ispclk[ISP_CLK_MAX];
	unsigned int ispclk_value[ISP_CLK_MAX];
#ifdef CONFIG_HUAWEI_CAMERA_USE_HISP120
	unsigned int ispclk_value_low[ISP_CLK_MAX];
#endif
	unsigned int clkdis_dvfs[ISP_CLK_MAX];
	unsigned int clkdis_need_div[ISP_CLK_MAX];
	unsigned int clkdn[HISP_CLKDOWN_MAX][ISP_CLK_MAX];
	const char *clk_name[ISP_CLK_MAX];
	unsigned int dvfsmask;
	unsigned int usedvfs;
	unsigned int refs_isp_power;
	unsigned int refs_isp_nsec_init;
	unsigned int refs_isp_sec_init;
	unsigned int refs_ispcpu_init;
	unsigned int refs_isp_nsec_dst;
	unsigned int refs_isp_sec_dst;
	struct mutex pwrlock;
	unsigned int pwstat_num;
	unsigned int pwstat_type[PWSTAT_MAX];
	unsigned int pwstat_offset[PWSTAT_MAX];
	unsigned int pwstat_bit[PWSTAT_MAX];
	unsigned int pwstat_wanted[PWSTAT_MAX];
#ifdef DEBUG_HISP
	struct hisp_clk_dump_s hisp_clk;
#endif
};

struct hisp_nsec {
	struct device *device;
	struct platform_device *isp_pdev;
	void *isp_dma_va;
	dma_addr_t isp_dma;
	u64 pgd_base;
	struct mutex pwrlock;
};

struct secisp_mem_info {
	unsigned int type;
	unsigned int da;
	unsigned int size;
	unsigned int prot;
	unsigned int sec_flag;/* SEC or NESC*/
	int sharefd;
	u64 pa;
};

struct isp_a7mapping_s {
	unsigned int a7va;
	unsigned int size;
	unsigned int prot;
	unsigned int offset;
	unsigned int reserve;
	unsigned long a7pa;
	void *apva;
};

struct isp_atfshrdmem_s {
	u64 sec_pgt;
	unsigned int sec_mem_size;
	struct isp_a7mapping_s a7mapping[MAXA7MAPPING];
};

struct hisp_sec {
	struct device *device;
	unsigned int boardid;
	void *atfshrd_vaddr;
	u64 atfshrd_paddr;
	void *rsctable_vaddr;
	void *rsctable_vaddr_const;
	u64 rsctable_paddr;
	unsigned int rsctable_offset;
	unsigned int rsctable_size;
	struct isp_atfshrdmem_s *shrdmem;
	struct mutex pwrlock;
	struct mutex ca_mem_mutex;
	struct mutex ta_status_mutex;
	struct gen_pool *isp_iova_pool;
	unsigned long isp_iova_start;
	unsigned long isp_iova_size;
	void *ap_dyna_array;
	struct isp_a7mapping_s *ap_dyna;
	int secmem_count;
	int isp_wdt_flag;
	int isp_cpu_reset;
	int is_heap_flag;
	int ispmem_reserved;
	unsigned int trusted_mem_size;
	unsigned int mapping_items;
	unsigned int tsmem_offset[ISP_TSMEM_MAX];
	unsigned int share_mem_size;
	u64 sec_boot_phymem_addr;
	u64 sec_fw_phymem_addr;
	struct rproc_shared_para *sec_isp_share_para;
	int use_ca_ta;
	struct secisp_mem_info boot_mem[HISP_SEC_BOOT_MAX_TYPE];
	struct secisp_mem_info rsv_mem[HISP_SEC_RSV_MAX_TYPE];
	struct secisp_mem_info pool_mem[HISP_SEC_POOL_MAX_TYPE];
	struct dma_buf *boot_dmabuf[HISP_SEC_BOOT_MAX_TYPE];
	struct dma_buf *pool_dmabuf[HISP_SEC_POOL_MAX_TYPE];
	unsigned int sec_ta_enable;
	unsigned int boot_mem_ready_flag;
	unsigned int sec_poweron_status;
};

struct hisp_rproc {
	struct isp_mbox *mbox;
	struct notifier_block nb;
	struct rproc *rproc;
};

struct rproc_camera {
	struct clk *aclk;
	struct clk *aclk_dss;
	struct clk *pclk_dss;
	/* pinctrl */
	struct pinctrl *isp_pinctrl;
	struct pinctrl_state *pinctrl_def;
	struct pinctrl_state *pinctrl_idle;
};

/*
 * struct rproc_shared_para - shared parameters for debug
 * @rdr_enable: the rdr function status
 */
struct isp_plat_cfg {
	unsigned int platform_id;
	unsigned int isp_local_timer;
	unsigned int perf_power;
};

struct rproc_boot_device {
	unsigned int boardid;
	struct platform_device *isp_pdev;
	u64 remap_addr;
	u64 nsec_remap_addr;
	u64 nsec_share_addr;
	void *share_va;
	unsigned int isppd_adb_flag;
	struct hisp_rproc *isp_rproc;
	struct isp_plat_cfg tmp_plat_cfg;
	struct resource *r[HISP_MAX_REGTYPE];
	void __iomem *reg[HISP_MAX_REGTYPE];
	unsigned int reg_num;
	struct rproc_camera isp_data;
	unsigned int a7_ap_mbox;
	unsigned int ap_a7_mbox;
	enum hisp_rproc_case_attr case_type;
	int ispcpu_status;
	struct wakeup_source ispcpu_wakelock;
	struct mutex ispcpu_mutex;
	struct wakeup_source jpeg_wakelock;
	struct mutex jpeg_mutex;
	struct mutex sharedbuf_mutex;
	struct mutex rpmsg_ready_mutex;
	struct mutex hisp_power_mutex;
	struct task_struct *pwronisp;
	struct completion rpmsg_boot_comp;
	wait_queue_head_t pwronisp_wait;
	bool pwronisp_wake;
	unsigned int pwronisp_nice;
	unsigned int pwronisp_thread_status;
	spinlock_t rpmsg_ready_spin_mutex;
	atomic_t rproc_enable_status;
	unsigned int rpmsg_ready_state;
	unsigned int hisp_power_state;
	int bypass_pwr_updn;
	int probe_finished;
	unsigned char isp_efuse_flag;
	void *rsctable_vaddr;
	unsigned int use_logb_flag;
};


/* secisp CA API */
extern int teek_secisp_close(void);
extern int teek_secisp_open(void);
extern int teek_secisp_disreset(void);
extern int teek_secisp_reset(void);
extern int teek_secisp_mem_map(struct secisp_mem_info *buffer);
extern int teek_secisp_mem_unmap(struct secisp_mem_info *buffer);
extern int teek_secisp_dynamic_mem_map(
	struct secisp_mem_info *buffer, struct device *dev);
extern int teek_secisp_dynamic_mem_unmap(
	struct secisp_mem_info *buffer, struct device *dev);
extern int teek_secisp_boot_mem_config(struct hisp_sec *dev);
extern int teek_secisp_boot_mem_deconfig(struct hisp_sec *dev);
extern void teek_secisp_ca_probe(void);
extern void teek_secisp_ca_remove(void);

/* isp power API */
extern int hisp_wait_rpmsg_completion(void);
extern int check_dvfs_valid(void);
extern int get_ispsys_power_state(unsigned int type);
extern unsigned int hisp_get_smmuc3_flag(void);
extern int hisp_subsys_powerup(void);
extern int hisp_subsys_powerdn(void);
extern int hisp_pwr_core_nsec_init(void);
extern int hisp_pwr_core_nsec_exit(void);
extern int hisp_pwr_core_sec_init(u64 phy_pgd_base);
extern int hisp_pwr_core_sec_exit(void);
extern int hisp_pwr_cpu_sec_init(void);
extern int hisp_pwr_cpu_sec_exit(void);
extern int hisp_pwr_cpu_nsec_dst(u64 remap_addr, unsigned int canary);
extern int hisp_pwr_cpu_nsec_rst(void);
extern int hisp_pwr_cpu_sec_dst(int ispmem_reserved, unsigned long image_addr);
extern int hisp_pwr_cpu_sec_rst(void);
extern void hisp_pwr_sec_dst_vote(void);
extern void hisp_pwr_sec_rst_vote(void);
extern unsigned long hisp_pwr_getclkrate(unsigned int type);
extern int hisp_pwr_nsec_setclkrate(unsigned int type, unsigned int rate);
extern int hisp_pwr_sec_setclkrate(unsigned int type, unsigned int rate);
extern int hisp_pwr_probe(struct platform_device *pdev);
extern int hisp_pwr_remove(struct platform_device *pdev);

/* isp dtsi API */
extern int hisp_pwr_getdts(struct platform_device *pdev,
		struct hisp_pwr *dev);
extern int hisp_nsec_getdts(struct platform_device *pdev,
		struct hisp_nsec *dev);
extern int hisp_sec_getdts(struct platform_device *pdev,
		struct hisp_sec *dev);
extern int hisp_secmem_getdts(struct hisp_sec *dev);
extern int hisp_rproc_regaddr_getdts(struct platform_device *pdev,
		struct rproc_boot_device *rproc_dev);
extern void hisp_rproc_regaddr_putdts(struct rproc_boot_device *rproc_dev);
extern struct isp_rproc_data *isp_rproc_data_getdts(struct device *pdev,
		struct rproc_boot_device *dev);
extern void isp_rproc_data_putdts(struct device *pdev);

/* hisp clk debug api */
extern unsigned int get_debug_isp_clk_enable(void);
extern int set_debug_isp_clk_enable(int state);
extern int set_debug_isp_clk_freq(unsigned int type, unsigned long value);
extern unsigned long get_debug_isp_clk_freq(unsigned int type);

/* hisp dump debug api */
extern void isp_secisp_dump(void);
extern int hisp_mntn_dumpregs(void);
extern void dump_hisp_boot(struct rproc *rproc, unsigned int size);
extern void hisp_boot_stat_dump(void);

/* hisp QOS info */
int ispcpu_qos_cfg(void);
int hisp_qos_dtget(struct device_node *np);
void hisp_qos_free(void);

/* hisp status api */
extern int use_sec_isp(void);
extern int use_nonsec_isp(void);

/* hisp CA - TA api */
extern int sec_process_use_ca_ta(void);
extern int hisp_mem_type_pa_init(unsigned int etype, unsigned long paddr);
extern u32 hisp_get_hisp_vring_da(unsigned int etype);

#endif
