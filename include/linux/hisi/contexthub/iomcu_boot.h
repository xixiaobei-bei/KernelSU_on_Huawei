/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Description: Contexthub share memory driver
 * Author: Huawei
 * Create: 2016-04-01
 */
#ifndef __IOMCU_BOOT_H__
#define __IOMCU_BOOT_H__
#include <linux/types.h>
#include  <iomcu_ddr_map.h>

#define STARTUP_IOM3_CMD 0x00070001
#define RELOAD_IOM3_CMD 0x0007030D

#define IOMCU_CONFIG_SIZE (DDR_CONFIG_SIZE * 3 / 8)
#define IOMCU_CONFIG_START DDR_CONFIG_ADDR_AP

#define IOMCU_SMPLAT_SCFG_START (IOMCU_CONFIG_START + DDR_CONFIG_SIZE * 3 / 8)
#define IOMCU_SMPLAT_SCFG_SIZE (DDR_CONFIG_SIZE * 5 / 8)

#define IOMCU_SMPLAT_DDR_MAGIC 0xabcd1243


typedef enum DUMP_LOC {
	DL_NONE = 0,
	DL_TCM,
	DL_EXT,
	DL_BOTTOM = DL_EXT,
} dump_loc_t;
enum DUMP_REGION {
	DE_TCM_CODE,
	DE_DDR_CODE,
	DE_DDR_DATA,
	DE_BOTTOM = 16,
};

typedef struct {
	u8 loc;
	u8 reserved[3];
} dump_region_config_t;

typedef struct {
	u64 dump_addr;
	u32 dump_size;
	u32 reserved1;
	u64 ext_dump_addr;
	u32 ext_dump_size;
	u8 enable;
	u8 finish;
	u8 reason;
	u8 reserved2;
	dump_region_config_t elements[DE_BOTTOM];
} dump_config_t;

typedef struct {
	u64 syscnt;
	u64 kernel_ns;
} timestamp_kernel_t;

typedef struct {
	u64 syscnt;
	u64 kernel_ns;
	u32 timer32k_cyc;
	u32 reserved;
} timestamp_iomcu_base_t;

typedef struct {
	u64 log_addr;
	u32 log_size;
	u32 rsvd;
} bbox_config_t;

struct plat_cfg_str{
	int magic;
	dump_config_t dump_config;
	timestamp_kernel_t timestamp_base;
	timestamp_iomcu_base_t timestamp_base_iomcu;
	bbox_config_t bbox_conifg;
};

extern uint32_t need_reset_io_power;
extern uint32_t need_set_3v_io_power;
extern uint32_t need_set_3_2v_io_power;
extern atomic_t iom3_rec_state;
void write_timestamp_base_to_cfgddr(void);
void inputhub_init_before_boot(void);
void inputhub_init_after_boot(void);
extern struct plat_cfg_str *g_smplat_scfg;

#endif /* __LINUX_INPUTHUB_CMU_H__ */
