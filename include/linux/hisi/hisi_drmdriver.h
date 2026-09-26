/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: drmdriver for kirin platform
 * Author: security-ap
 * Create: 2014/5/16
 */

#ifndef __HISI_DRMDRIVER_H__
#define __HISI_DRMDRIVER_H__

#include <linux/init.h>
#include <linux/mutex.h>

/* list sub-function id for access register service */
#define ACCESS_REGISTER_FN_MAIN_ID                          0xc500aa01UL
#define ACCESS_REGISTER_FN_SUB_ID_DDR_INTLV                 0x55bbcce0UL
#define ACCESS_REGISTER_FN_SUB_ID_DDR_FLUX_W                0x55bbcce1UL
#define ACCESS_REGISTER_FN_SUB_ID_DDR_FLUX_R                0x55bbcce2UL
#define ACCESS_REGISTER_FN_SUB_ID_DDR_DRAM_R                0x55bbcce3UL
#define ACCESS_REGISTER_FN_SUB_ID_DDR_STDID_W               0x55bbcce4UL
#define ACCESS_REGISTER_FN_SUB_ID_DDR_DSS                   0x55bbcce5UL
#define ACCESS_REGISTER_FN_SUB_ID_DDR_PASR                  0x55bbcce6UL
#define ACCESS_REGISTER_FN_SUB_ID_MASTER_SECURITY_CONFIG    0x55bbcce7UL
#define ACCESS_REGISTER_FN_SUB_ID_DDR_DRM_SET               0x55bbcce8UL
#define ACCESS_REGISTER_FN_SUB_ID_DDR_MODEM_SEC             0x55bbcce9UL
#define ACCESS_REGISTER_FN_SUB_ID_DDR_DRM_CLR               0x55bbcceaUL
#define ACCESS_REGISTER_FN_SUB_ID_DDR_ISP_SEC_OPEN          0x55bbccebUL
#define ACCESS_REGISTER_FN_SUB_ID_DDR_ISP_SEC_CLOSE         0x55bbccecUL
#define ACCESS_REGISTER_FN_SUB_ID_DDR_KERNEL_CODE_PROTECT   0x55bbccedUL
#define ACCESS_REGISTER_FN_SUB_ID_DDR_HIFI_SEC_OPEN         0x55bbcceeUL
#define ACCESS_REGISTER_FN_SUB_ID_DDR_HIFI_SEC_CLOSE        0x55bbccefUL
#define ACCESS_REGISTER_FN_SUB_ID_SECS_POWER_CTRL           0x55bbccf0UL
#define ACCESS_REGISTER_FN_SUB_ID_DDR_PERF_CTRL             0x55bbccf3UL

#define DRMDRIVER_MODULE_INIT_SUCCESS_FLG                   0x12345678

#define MAX_DSS_CHN_IDX                                     12

enum _master_id_type_ {
	MASTER_VDEC_ID = 0,
	MASTER_VENC_ID,
	MASTER_DSS_ID,
	MASTER_G3D_ID,
	MASTER_ID_MAX
};

enum master_dss_op_type {
	DSS_SMMU_INIT = 0,
	DSS_CH_MMU_SEC_CONFIG,
	DSS_CH_MMU_SEC_DECONFIG,
	DSS_CH_SEC_CONFIG,
	DSS_CH_SEC_DECONFIG,
	DSS_CH_DEFAULT_SEC_CONFIG,
	MASTER_OP_SECURITY_MAX,
};

/*
 * Below typedef are being used in DDR model for now,
 * please fix it in DDR group CSEC.
 */
enum compose_mode {
	ONLINE_COMPOSE_MODE,
	OFFLINE_COMPOSE_MODE,
	OVL1_ONLINE_COMPOSE_MODE,
	OVL3_OFFLINE_COMPOSE_MODE,
	MAX_COMPOSE_MODE,
};

typedef enum _dynamic_ddr_sec_type {
	DDR_DRM_SEC_SET = 0,
	DDR_TUI_SEC_SET = 1,
	DDR_IRIS_SEC_SET = 2,
	DYNAMIC_DDR_SET_MAX = 8,
} dynamic_ddr_sec_type;

struct tag_atfd_data {
	phys_addr_t  buf_phy_addr;
	size_t buf_size;
	u8 *buf_virt_addr;
	struct mutex atfd_mutex;  /* The mutex of ATFD */
	u32 module_init_success_flg;
};

/*
 * Below typedef are being used in DDR model for now,
 * please fix it in DDR group CSEC.
 * Please also explain why you use unsigned long type.
 */
typedef struct {
	unsigned long start_addr; /* start address of the region */
	unsigned long sub_rgn_size;
	unsigned long bit_map;    /* 48bit */
	unsigned long sec_port;
} drm_sec_cfg;

#ifdef CONFIG_DRMDRIVER
noinline s32 atfd_hisi_service_access_register_smc(u64 main_fun_id,
						   u64 buff_addr_phy,
						   u64 data_len,
						   u64 sub_fun_id);
void configure_master_security(u32 is_security, s32 master_id);
void configure_dss_register_security(u32 addr, u32 val, u8 bw, u8 bs);
s32 configure_dss_service_security(u32 master_op_type, u32 channel, u32 mode);
s32 hisi_sec_ddr_set(drm_sec_cfg *sec_cfg, dynamic_ddr_sec_type type);
s32 hisi_sec_ddr_clr(dynamic_ddr_sec_type type);
#else /* !CONFIG_DRMDRIVER */
static inline s32 atfd_hisi_service_access_register_smc(u64 main_fun_id,
							u64 buff_addr_phy,
							u64 data_len,
							u64 sub_fun_id)
{
	return 0;
}

static inline void configure_master_security(u32 is_security, s32 master_id)
{
}

static inline void configure_dss_register_security(
	u32 addr, u32 val, u8 bw, u8 bs)
{
}

static inline s32 configure_dss_service_security(u32 master_op_type,
						 u32 channel, u32 mode)
{
	return 0;
}

static inline s32 hisi_sec_ddr_clr(dynamic_ddr_sec_type type)
{
	return 0;
}

static inline s32 hisi_sec_ddr_set(drm_sec_cfg *sec_cfg,
				   dynamic_ddr_sec_type type)
{
	return 0;
}
#endif

#endif /* HISI_DRMDRIVER_H_ */
