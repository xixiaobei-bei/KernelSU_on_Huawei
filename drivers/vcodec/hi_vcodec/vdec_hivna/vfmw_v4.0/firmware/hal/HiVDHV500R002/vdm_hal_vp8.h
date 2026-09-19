#ifndef __VDM_HAL_VP8_HERAER__
#define __VDM_HAL_VP8_HERAER__
#include "memory.h"
#include "vfmw_intf.h"

#ifdef MSG_POOL_ADDR_CHECK
typedef struct
{
	USIGN sed_top_addr:                         32;
} VP8_PICDNMSG_D36;

typedef struct
{
	USIGN pmv_top_addr:                         32;
} VP8_PICDNMSG_D37;

typedef struct
{
	USIGN rcn_top_addr:                         32;
} VP8_PICDNMSG_D38;

typedef struct
{
	USIGN tab_addr:                             32;
} VP8_PICDNMSG_D39;
typedef struct
{
	USIGN dblk_top_addr:                        32;
} VP8_PICDNMSG_D40;

SINT32 VP8HAL_StartDec(OMXVDH_REG_CFG_S *pVdhRegCfg, MEM_BUFFER_S* pVdhMemMap);
#else
SINT32 VP8HAL_StartDec(OMXVDH_REG_CFG_S *pVdhRegCfg);
#endif
#endif

