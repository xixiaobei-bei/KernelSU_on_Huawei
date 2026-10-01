#ifndef __VDM_HAL_MPEG2_H__
#define __VDM_HAL_MPEG2_H__

#include "basedef.h"
#include "memory.h"
#include "vfmw_intf.h"

#ifdef MSG_POOL_ADDR_CHECK
typedef struct
{
	USIGN pmv_top_addr:32;
} MP2DNMSG_D48;

typedef struct
{
	USIGN first_slc_dnmsg_addr:                32;
} MP2DNMSG_D63;

SINT32 MP2HAL_StartDec(OMXVDH_REG_CFG_S *pVdhRegCfg, MEM_BUFFER_S *pVdhMemMap);
#else
SINT32 MP2HAL_StartDec(OMXVDH_REG_CFG_S *pVdhRegCfg);
#endif
#endif
