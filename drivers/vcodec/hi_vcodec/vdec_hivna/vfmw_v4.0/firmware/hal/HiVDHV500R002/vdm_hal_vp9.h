
#ifndef __VDM_HAL_VP9_H__
#define __VDM_HAL_VP9_H__
#include "memory.h"
#include "vfmw_intf.h"

#ifdef MSG_POOL_ADDR_CHECK
SINT32 VP9HAL_StartDec(OMXVDH_REG_CFG_S *pVdhRegCfg, MEM_BUFFER_S* pVdhMemMap);
#else
SINT32 VP9HAL_StartDec(OMXVDH_REG_CFG_S *pVdhRegCfg);
#endif
#endif //__VDM_HAL_AVS_H__
