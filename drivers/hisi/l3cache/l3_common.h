#ifndef __L3SHARE_H__
#define __L3SHARE_H__

#include <asm/compiler.h>
#include <linux/hisi/hisi_l3share.h>

#define L3_SHARE_SVC                    (0xc500f008)
#define L3_DEBUG_SVC                    (0xc500f001)
#define L3_DEBUG_PMU_SVC                (0xc500f002)



#define SIZE_4K                 (0x1000)
#define NR_TLB_ENTRY            (0x1000)
#define MEM_BLOCK_SIZE          (SIZE_4K * NR_TLB_ENTRY /2)


typedef enum _L3_CACHE_SHARE_CMD {
	CMD_ENABLE_ACP = 0,
	CMD_DISABLE_ACP,
} l3_cache_share_cmd;


typedef enum _L3_CACHE_DEBUG_CMD {
	CMD_SET_CLSPWRDN = 0,
	CMD_SET_PART_CTRL,
	CMD_SET_SCHEME_ID,
	CMD_SET_POWER_CTRL,
	CMD_SET_FUNC_RETEN,
	CMD_GET_MISS_COUNT,
	CMD_GET_HIT_COUNT,
} l3_cache_debug_cmd;


typedef enum _L3_CACHE_DEBUG_PMU_CMD {
	CMD_GET_PMU_L3_COUNT = 0,
	CMD_GET_PMU_ACP_COUNT,
	CMD_GET_PMU_L3_RD_COUNT,
	CMD_GET_PMU_L3_WR_COUNT,
} l3_cache_debug_pmu_cmd;


#endif
