#ifndef __ISP_DDR_MAP_H__
#define __ISP_DDR_MAP_H__ 
#include <global_ddr_map.h>
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#define ISP_PCTRL_PERI_STAT_ADDR (0x000000BC)
#define ISP_PCTRL_PERI_FLAG ((1 << 21) | (1 << 20))
#define MEM_ISPFW_SIZE (0x00A00000)
#define MEM_PMD_ADDR_OFFSET (0x00002000)
#define MEM_PTE_ADDR_OFFSET (MEM_ISPFW_SIZE + 0x10000)
#define MEM_SMMU_ERRRD_ADDR_OFFSET (0x0000F000)
#define MEM_SMMU_ERRWR_ADDR_OFFSET (0x0000F800)
#define MEM_RSCTABLE_ADDR_OFFSET (0x00014000)
#define MEM_RSCTABLE_SIZE (0x00004000)
#define MEM_ISPDTS_SIZE (0x02000000)
#define MEM_MDC_DA (0xC1400000)
#define MEM_MDC_SIZE (0x00100000)
#define SHAREDMEM_BASE (0xc2000000)
#define VQ_BASE (0xc2020000)
#define DEVICE_BASE (0xE0000000)
#define TEXT_BASE (0xC0300000)
#define DATA_BASE (0xC0800000)
#define ITCM_BASE (0xFFFF0000)
#define DTCM_BASE (0xE85D0000)
#define ISP_CORE_CFG_BASE_ADDR (0xE8400000)
#define ISP_PMC_BASE_ADDR (0xFFF31000)
#define MAX_REGION_NUM (24)
#define STATIC_REGION_NSEC_NUM (14)
#define STATIC_REGION_SEC_NUM (15)
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
