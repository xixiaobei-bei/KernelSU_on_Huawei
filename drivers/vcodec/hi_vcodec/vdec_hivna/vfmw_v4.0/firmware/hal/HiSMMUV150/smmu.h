

#ifndef __HIVDEC_SMMU_H__
#define __HIVDEC_SMMU_H__
// for VDM_REG_PHY_ADDR, SCD_REG_PHY_ADDR, BPD_REG_PHY_ADDR
#include "sysconfig.h"
#include "vfmw.h"

#define SMMU_OK     0
#define SMMU_ERR   (-1)

#define SECURE_ON   1
#define SECURE_OFF  0
#define SMMU_ON     1
#define SMMU_OFF    0

#define VDH_MASTER_BW 1
#define VDH_MASTER_BS 20
#define COMMON_REG_ADDR_OFFSET 4
#define COMMON_REG_VAR 4
#define COMMON_REG_BW 4
#define COMMON_REG_BS 4

typedef enum {
	MFDE = 0,
	BPD,
	SCD,
} smmu_master_type;

SINT32 smmu_init(void);
VOID smmu_deinit(void);
VOID smmu_set_master_reg(
	smmu_master_type master_type, UINT8 secure_en,
	UINT8 mmu_en);
#ifdef PLATFORM_HIVCODECV200
VOID smmu_set_mem_ctl_reg(void);
#endif
VOID smmu_init_global_reg(void);
VOID smmu_int_serv_proc(void);

#endif
