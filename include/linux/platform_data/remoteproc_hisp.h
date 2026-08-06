/*
 * Remote Processor - Histar ISP remoteproc platform data.
 * include/linux/platform_data/remoteproc_isp.h
 *
 * Copyright (c) 2013-2014 ISP Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _PLAT_REMOTEPROC_HISP_H
#define _PLAT_REMOTEPROC_HISP_H

#include <linux/firmware.h>
#include <linux/remoteproc.h>
#include <linux/iommu.h>
#include <linux/hisi-iommu.h>
#include <linux/gpio.h>
#include <asm/page.h>
#include <linux/version.h>
#include <linux/platform_data/hisp.h>
#ifdef CONFIG_HISI_LB
#define IOMMU_MASK (~(IOMMU_READ | IOMMU_WRITE | IOMMU_CACHE | IOMMU_NOEXEC \
		| IOMMU_MMIO | IOMMU_DEVICE | IOMMU_EXEC | IOMMU_PORT_MASK))
#else
#define IOMMU_MASK (~(IOMMU_READ | IOMMU_WRITE | IOMMU_CACHE | IOMMU_NOEXEC \
		| IOMMU_MMIO | IOMMU_DEVICE | IOMMU_EXEC))
#endif
#define ISP_FW_MEM_SHARED_SIZE   (0x1000)
#define ISP_MAX_NUM_MAGIC        (0xFFFFFFFF)
#define ISP_MAX_IOVA_MAGIC       (0xE0000000)
#define ISP_MAX_BYTE_BIT_NUM     (31)

struct rproc_ops;
struct platform_device;
struct rproc;
struct scatterlist;
struct virtio_device;
struct virtqueue;
struct dentry;

/**
 * struct fw_rsc_trace - print version information
 * @magic: magic word
 * @module: module
 * @version: version info
 * @built_time: built time
 * @reserved: reserved (must be zero)
 */
struct fw_rsc_version {
	unsigned int magic;
	char module[8];
	char version[8];
	char build_time[32];
	char reserved[4];
} __packed;

/**
 * struct fw_rsc_carveout - physically non-contiguous memory request
 * @da: device address
 * @pa: physical address
 * @len: length (in bytes)
 * @flags: iommu protection flags
 * @reserved: reserved (must be zero)
 * @name: human-readable name of the requested memory region
 */
struct fw_rsc_dynamic_memory {
	u32 da;
	u32 pa;
	u32 len;
	u32 flags;
	u32 reserved;
	u8 name[32];
} __packed;

/**
 * struct fw_rsc_carveout - physically reserved memory request
 * @da: device address
 * @pa: physical address
 * @len: length (in bytes)
 * @flags: iommu protection flags
 * @reserved: reserved (must be zero)
 * @name: human-readable name of the requested memory region
 */
struct fw_rsc_reserved_memory {
	u32 da;
	u32 pa;
	u32 len;
	u32 flags;
	u32 reserved;
	u8 name[32];
} __packed;

/**
 * struct fw_rsc_trace - cda buffer declaration
 * @da: device address
 * @len: length (in bytes)
 * @reserved: reserved (must be zero)
 * @name: human-readable name of the trace buffer
 */
struct fw_rsc_cda {
	u32 da;
	u32 len;
	u32 reserved;
	u8 name[32];
} __packed;

/**
 * struct rproc_cache_entry - memory cache entry
 * @va:	virtual address
 * @len: length, in bytes
 * @node: list node
 */
struct rproc_cache_entry {
	void *va;
	u32 len;
	struct list_head node;
};

/**
 * struct rproc_page - page memory
 * @va:	virtual address of pages
 * @num: number of pages
 * @node: list node
 */
struct rproc_page {
	struct sg_table *table;
	struct list_head node;
};

struct rproc_page_info {
	struct page *page;
	unsigned int order;
	struct list_head node;
};
/*
 * struct omap_rproc_pdata - omap remoteproc's platform data
 * @name: the remoteproc's name
 * @oh_name: omap hwmod device
 * @oh_name_opt: optional, secondary omap hwmod device
 * @firmware: name of firmware file to load
 * @mbox_name: name of omap mailbox device to use with this rproc
 * @ops: start/stop rproc handlers
 * @device_enable: omap-specific handler for enabling a device
 * @device_shutdown: omap-specific handler for shutting down a device
 * @set_bootaddr: omap-specific handler for setting the rproc boot address
 */

struct isp_rproc_data {
	const char *name;
	const char *firmware;
	const char *mbox_name;
	const struct rproc_ops *ops;
	unsigned int ipc_addr;
	int (*device_enable)(struct platform_device *pdev);
	int (*device_shutdown)(struct platform_device *pdev);
	void (*set_bootaddr)(u32);
};

enum isp_msg_rdr_e {
	RPMSG_RDR_LOW                   = 0,
	/* RECV RPMSG INFO */
	RPMSG_CAMERA_SEND_MSG           = 1,
	RPMSG_SEND_MSG_TO_MAILBOX       = 2,
	RPMSG_RECV_MAILBOX_FROM_ISPCPU  = 3,
	RPMSG_ISP_THREAD_RECVED         = 4,
	RPMSG_RECV_SINGLE_MSG           = 5,
	RPMSG_SINGLE_MSG_TO_CAMERA      = 6,
	RPMSG_CAMERA_MSG_RECVED         = 7,
	RPMSG_RDR_MAX,
};
enum isp_msg_err_e {
	RPMSG_SEND_ERR                  = 0,
	RPMSG_SEND_MAILBOX_LOST,
	RPMSG_RECV_MAILBOX_LOST,
	RPMSG_THREAD_RECV_LOST,
	RPMSG_RECV_SINGLE_LOST,
	RPMSG_SINGLE_MSG_TO_CAMERA_LOST,
	RPMSG_CAMERA_MSG_RECVED_LOST,
	RPMSG_RECV_THRED_TIMEOUT,
	RPMSG_CAMERA_GET_TIMEOUT,
	RPMSG_ERR_MAX,
};

enum hisp_phy_id_e {
	HISP_CDPHY_A = 0,
	HISP_CDPHY_B,
	HISP_CDPHY_C,
	HISP_CDPHY_D,
	HISP_CDPHY_E,
	HISP_CDPHY_MAX,
};

enum hisp_phy_mode_e {
	HISP_PHY_MODE_DPHY = 0,
	HISP_PHY_MODE_CPHY,
	HISP_PHY_MODE_MAX,
};

enum hisp_phy_freq_mode_e {
	HISP_PHY_AUTO_FREQ = 0,
	HISP_PHY_MANUAL_FREQ,
	HISP_PHY_FREQ_MODE_MAX,
};

enum hisp_phy_work_mode_e {
	HISP_PHY_SINGLE_MODE = 0,
	HISP_PHY_DUAL_MODE_SENSORA,//dphy use DL1&3,cphy use DL2
	HISP_PHY_DUAL_MODE_SENSORB,//dphy use DL0&2,cphy use DL0&1
	HISP_PHY_WORK_MODE_MAX,
};

struct hisp_phy_info_t {
	unsigned int is_master_sensor;
	enum hisp_phy_id_e phy_id;
	enum hisp_phy_mode_e phy_mode;
	enum hisp_phy_freq_mode_e phy_freq_mode;
	unsigned int phy_freq;
	enum hisp_phy_work_mode_e phy_work_mode;
};

int rpmsg_sensor_ioctl(unsigned int cmd, int index, char *name);

/**
 * enum rpmsg_client_choice- choose which rpmsg client driver for debug
 *
 * @ISP_DEBUG_RPMSG_CLIENT: use isp debug rpmsg client
 * @INVALID_CLIENT: use hisp(new camera arch) rpmsg client
 */
enum rpmsg_client_choice {
	ISP_DEBUG_RPMSG_CLIENT  = 0x1111,
	INVALID_CLIENT          = 0xFFFF,
};
/*
 * isp mem alloc struct
 */
struct hisp_ion_s {
	struct ion_handle *ion_handle;
	struct ion_client *ion_client;
	u64 paddr;
	void *virt_addr;
};

struct hisp_mdc_device {
	int isp_mdc_flag;
	int isp_mdc_count;
	dma_addr_t mdc_dma_addr;
	unsigned long iova;
	size_t size;
};
/*
 * isp nesc cpucfg dump
 */
#define CORE_DUMP_EXC_CUR       (1)
#define CORE_DUMP_ION           (1 << 1)
#define CORE_DUMP_RTOS_FINISH   (1 << 2)
#define CORE_DUMP_ALL_FINISH    (1 << 3)
#define ISP_CPU_POWER_DOWN      (1 << 7)
#define CORE_DUMP_EXCEPTION     (1 << 5)
#define CORE_RPMSG_DEBUG_NAME   (1 << 31)

#define MAX_RESULT_LENGTH       (8)
#define DUMP_ISP_BOOT_SIZE      (64)
#define ISPCPU_STONE_OFFSET     (0xF00)
struct isp_nsec_cpu_dump_s {
	unsigned int reg_addr;
	unsigned int actual_value;
	unsigned int expected_value;
	unsigned int care_bits;
	unsigned int compare_result;
	char result[MAX_RESULT_LENGTH];
};

struct isp_ispcpu_stone_s {
	unsigned int type; /**< EXCPTION TYPE                      */
	unsigned int uw_cpsr; /**< Current program status register (CPSR)     */
	unsigned int uw_r0; /**< Register R0                        */
	unsigned int uw_r1; /**< Register R1                        */
	unsigned int uw_r2; /**< Register R2                        */
	unsigned int uw_r3; /**< Register R3                        */
	unsigned int uw_r4; /**< Register R4                        */
	unsigned int uw_r5; /**< Register R5                        */
	unsigned int uw_r6; /**< Register R6                        */
	unsigned int uw_r7; /**< Register R7                        */
	unsigned int uw_r8; /**< Register R8                        */
	unsigned int uw_r9; /**< Register R9                        */
	unsigned int uw_r10; /**< Register R10                      */
	unsigned int uw_r11; /**< Register R11                      */
	unsigned int uw_r12; /**< Register R12                      */
	unsigned int uw_sp; /**< Stack pointer                      */
	unsigned int uw_lr; /**< Program returning address          */
	unsigned int uw_pc; /**< PC pointer of the exceptional function    */
	unsigned int dfsr; /**< DataAbort STATE      -- DFSR       */
	unsigned int dfar; /**< DataAbort ADDR       -- DFAR       */
	unsigned int ifsr; /**< PrefetchAbort STATE  -- IFSR       */
	unsigned int ifar; /**< PrefetchAbort ADDR   -- IFAR       */
	unsigned int smmu_far_low;  /**< SMMU500 ADDR -- FAR       */
	unsigned int smmu_far_high; /**< SMMU500 ADDR -- FAR       */
	unsigned int smmu_fsr; /**< SMMU500 STATE    -- FSR        */
	unsigned int smmu_fsynr0; /**< SMMU500 STATE -- FSYNR      */
};

extern const struct rproc_fw_ops rproc_elf_fw_ops;
extern const struct rproc_fw_ops rproc_bin_fw_ops;
extern struct rproc_shared_para *isp_share_para;
extern int last_boot_state;

/* HISP cfg api */
extern int get_rproc_enable_status(void);
extern int is_ispcpu_powerup(void);
int is_use_loadbin(void);

extern void __iomem *get_regaddr_by_pa(unsigned int type);
extern int get_ispcpu_idle_stat(unsigned int isppd_adb_flag);
extern int get_ispcpu_cfg_info(void);
extern int bypass_power_updn(void);
extern struct device *get_isp_device(void);

extern u32 get_share_exc_flag(void);
extern u64 isp_getcurtime(void);
extern size_t print_time(u64 ts, char *buf);

/* HISP - api */
extern int hw_is_fpga_board(void);/* form camera hal*/
extern int hisp_rproc_case_set(enum hisp_rproc_case_attr);
extern enum hisp_rproc_case_attr hisp_rproc_case_get(void);

extern int hisp_rproc_enable(void);
extern int hisp_rproc_disable(void);

/* HISP NEW SEC BOOT api*/
int hisp_secmem_ca_map(unsigned int pool_num, int sharefd, unsigned int size);
int hisp_secmem_ca_unmap(unsigned int pool_num);
int hisp_secboot_memsize_get_from_type(unsigned int type, unsigned int *size);
int hisp_secboot_info_set(unsigned int type, int sharefd);
int hisp_secboot_info_release(unsigned int type);
int hisp_secboot_prepare(void);
int hisp_secboot_unprepare(void);
int hisp_sec_ta_enable(void);
int hisp_sec_ta_disable(void);
unsigned int hisp_pool_mem_addr(unsigned int pool_num);

/* MDC info */
int hispmdc_map_fw(unsigned long iova, phys_addr_t paddr,
		size_t size, int prot);
int hispmdc_unmap_fw(unsigned long iova, size_t size);
void hisp_mdc_dev_deinit(void);
void hisp_mdc_dev_init(void);
extern u64 get_mdc_addr_pa(void);
extern void set_shared_mdc_pa_addr(u64 mdc_pa_addr);

/* hisp phy config api */
int hisp_phy_csi_connect(struct hisp_phy_info_t *phy_info,
		unsigned int csi_index);
int hisp_phy_csi_disconnect(void);
int hisp_dsm_register(void);
void hisp_dsm_unregister(void);

/* HISP - rpmsg info */
extern int nonsec_isp_device_enable(struct rproc *rproc);
extern int nonsec_isp_device_disable(void);
extern int secisp_device_enable(void);
extern int secisp_device_disable(void);
extern int hisp_sec_pwron_prepare(void);

int hisp_nsec_probe(struct platform_device *pdev);
int hisp_nsec_remove(struct platform_device *pdev);
int isp_atfisp_probe(struct platform_device *pdev);
int isp_atfisp_remove(struct platform_device *pdev);
void hisp_rproc_init(struct rproc *rproc);
int rpmsg_vdev_map_resource(struct rproc *rproc);
void hisp_rproc_resource_cleanup(struct rproc *rproc);
int isp_rproc_select_def(void);
int isp_rproc_select_idle(void);
int fiq2ispcpu(void);
void rproc_set_sync_flag(bool flag);

#ifdef CONFIG_HISP_REMOTEPROC_DMAALLOC_DEBUG
void *get_vring_dma_addr(u64 *dma_handle, size_t size, unsigned int index);
int get_vring_dma_addr_probe(struct platform_device *pdev);
int get_vring_dma_addr_remove(struct platform_device *pdev);
bool rpmsg_mem_alloc_beforehand(struct virtio_device *vdev);
bool rproc_mem_alloc_beforehand(struct rproc *rproc);
#endif
extern struct rproc_shared_para *rproc_get_share_para(void);
extern int hisp_request_sec_rsctable(void);
extern void *get_rsctable(int *tablesz);
extern void free_secmem_rsctable(void);
void hisp_free_rsctable(void);
/* rproc handle */
int rproc_handle_version(struct rproc *rproc, struct fw_rsc_hdr *hdr,
		int offset, int avail);
void rproc_memory_cache_flush(struct rproc *rproc);
int rproc_handle_dynamic_memory(struct rproc *rproc, struct fw_rsc_hdr *hdr,
		int offset, int avail);
int rproc_handle_rdr_memory(struct rproc *rproc, struct fw_rsc_hdr *hdr,
		int offset, int avail);
int rproc_handle_shared_memory(struct rproc *rproc, struct fw_rsc_hdr *hdr,
		int offset, int avail);
void *rproc_da_to_va(struct rproc *rproc, u64 da, int len);

/* rpmsg rdr */
extern int hisp_rpmsg_rdr_init(void);
extern int hisp_rpmsg_rdr_deinit(void);
extern void hisp_sendin(void *data);
extern void hisp_sendx(void);
extern void hisp_recvtask(void);
extern void hisp_recvthread(void);
extern void hisp_recvin(void *data);
extern void hisp_recvx(void *data);
extern void hisp_recvdone(void *data);
extern void hisp_rpmsgrefs_dump(void);
extern void hisp_rpmsgrefs_reset(void);
extern void hisp_dump_rpmsg_with_id(const unsigned int message_id);

/* hisp mem info */
unsigned int a7_mmu_map(struct scatterlist *sgl, unsigned int size,
		unsigned int prot, unsigned int flag);
void a7_mmu_unmap(unsigned int va, unsigned int size);
void *hisp_get_rsctable(int *tablesz);
void *hisp_get_dyna_array(void);
struct isp_a7mapping_s *hisp_get_ap_dyna_mapping(void);
void a7_map_set_pa_list(void *listmem,
		struct scatterlist *sg, unsigned int size);

/* jpeg pw api */
extern int hisp_nsec_jpeg_powerup(void);
extern int hisp_nsec_jpeg_powerdn(void);
extern int hisp_sec_jpeg_powerup(void);
extern int hisp_sec_jpeg_powerdn(void);
extern int hisp_jpeg_powerdn(void);
extern int hisp_jpeg_powerup(void);

/* hisp clk api */
extern int hisp_set_clk_rate(unsigned int type, unsigned int rate);
extern unsigned long hisp_get_clk_rate(unsigned int type);

unsigned int wait_share_excflag_timeout(unsigned int flag, unsigned int time);

/* hisp fstcam info */
void *isp_fstcma_alloc(dma_addr_t *dma_handle, size_t size, gfp_t flag);
void isp_fstcma_free(void *va, dma_addr_t dma_handle, size_t size);

/* hisp bin load info */
int hisp_bsp_read_bin(const char *partion_name, unsigned int offset,
	unsigned int length, char *buffer);
int hisp_bin_load_segments(struct rproc *rproc);
void *hisp_loadbin_rsctable_init(void);
int wakeup_ispbin_kthread(void);
int isp_loadbin_debug_elf(void);
int hisp_loadbin_init(struct platform_device *pdev);
void hisp_loadbin_deinit(void);

/* hisp share mem info */
extern void set_a7sharedmem_va(void *addr);
extern void set_a7sharedmem_pa(u64 addr);
extern void isp_ispsec_share_para_set(void);
extern u64 get_a7sharedmem_addr(void);
extern void *getsec_a7sharedmem_addr(void);
extern void *get_a7sharedmem_va(void);
extern void set_a7sharedmem_addr(unsigned int addr);

/* hisp page info */
extern unsigned long long get_nonsec_pgd(void);
extern int hisp_rproc_pgd_set(struct rproc *rproc);

/* hisp remap info */
extern u64 get_a7remap_addr(void);
extern void set_a7mem_pa(u64 addr);
extern int set_isp_remap_addr(u64 remap_addr);

/* hisp rdr info */
#ifdef CONFIG_HISP_RDR
int rdr_isp_init(void);
void rdr_isp_exit(void);
extern u64 get_isprdr_addr(void);
extern void __iomem *get_isprdr_va(void);
void isploglevel_update(void);
int sync_isplogcat(void);
int start_isplogcat(void);
void stop_isplogcat(void);
void clear_isplog_info(void);
extern void ap_send_fiq2ispcpu(void);
unsigned int secisp_ta_map(int sharefd, unsigned int type,
		unsigned int size, unsigned int prot);
int secisp_ta_unmap(int sharefd, unsigned int type,
		unsigned int size, unsigned int da);

#else
static inline int rdr_isp_init(void) { return 0; }
static inline void rdr_isp_exit(void) { return; }
static inline u64 get_isprdr_addr(void) { return 0; }
static inline void isploglevel_update(void) { return; }
static inline int sync_isplogcat(void) { return -1; }
static inline int start_isplogcat(void) { return -1; }
static inline void stop_isplogcat(void) { return; }
static inline void clear_isplog_info(void) { return; }
static inline void ap_send_fiq2ispcpu(void) { return; }
static inline unsigned int secisp_ta_map(int sharefd, unsigned int type,
				unsigned int size, unsigned int prot)
{
	return 0;
}
static inline int secisp_ta_unmap(int sharefd, unsigned int type,
			unsigned int size, unsigned int da)
{
	return -1;
}

#endif

#ifdef DEBUG_HISP
/* hisp debug api */
ssize_t isprdr_store(struct device *pdev,
		struct device_attribute *attr, const char *buf, size_t count);
ssize_t isprdr_show(struct device *pdev,
		struct device_attribute *attr, char *buf);
ssize_t ispclk_show(struct device *pdev,
		struct device_attribute *attr, char *buf);
ssize_t ispclk_store(struct device *pdev,
		struct device_attribute *attr, const char *buf, size_t count);
ssize_t isppower_show(struct device *pdev,
		struct device_attribute *attr, char *buf);
ssize_t isppower_store(struct device *pdev,
		struct device_attribute *attr, const char *buf, size_t count);
ssize_t ispmsg_show(struct device *pdev,
		struct device_attribute *attr, char *buf);
ssize_t ispmsg_store(struct device *pdev,
		struct device_attribute *attr, const char *buf, size_t count);
ssize_t atfisp_show(struct device *pdev,
		struct device_attribute *attr, char *buf);
ssize_t atfisp_store(struct device *pdev,
		struct device_attribute *attr, const char *buf, size_t count);
ssize_t regs_show(struct device *pdev,
		struct device_attribute *attr, char *buf);
ssize_t dump_show(struct device *pdev,
		struct device_attribute *attr, char *buf);
ssize_t atfisp_test_show(struct device *pdev,
		struct device_attribute *attr, char *buf);
ssize_t atfisp_test_store(struct device *pdev,
		struct device_attribute *attr, const char *buf, size_t count);
ssize_t security_show(struct device *dev,
		struct device_attribute *attr, char *buf);
ssize_t security_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count);
ssize_t ispbinupdate_show(struct device *pdev,
		struct device_attribute *attr, char *buf);

int set_power_updn(int bypass);
struct isp_nsec_cpu_dump_s *get_debug_ispcpu_param(void);

#endif

#endif /* _PLAT_REMOTEPROC_HISP_H */

