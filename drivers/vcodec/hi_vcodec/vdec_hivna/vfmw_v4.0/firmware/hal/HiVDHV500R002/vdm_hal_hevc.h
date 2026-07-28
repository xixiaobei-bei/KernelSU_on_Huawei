#ifndef __VDM_HAL__HEVC_H__
#define __VDM_HAL__HEVC_H__

#include "basedef.h"
#include "mem_manage.h"
#include "memory.h"
#include "vfmw_intf.h"

#ifdef MSG_POOL_ADDR_CHECK
SINT32 HEVCHAL_StartDec(OMXVDH_REG_CFG_S *pVdhRegCfg, MEM_BUFFER_S *pVdhMemMap);
#else
SINT32 HEVCHAL_StartDec(OMXVDH_REG_CFG_S *pVdhRegCfg);
#endif
#endif
