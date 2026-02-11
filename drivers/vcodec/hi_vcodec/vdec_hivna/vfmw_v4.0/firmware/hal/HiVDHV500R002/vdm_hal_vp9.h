

#ifndef __VDM_HAL_VP9_H__
#define __VDM_HAL_VP9_H__
#include "memory.h"
#include "vfmw_intf.h"

#ifdef MSG_POOL_ADDR_CHECK
SINT32 vp9hal_startdec(omxvdh_reg_cfg_s *p_vdh_reg_cfg, mem_buffer_s *p_vdh_mem_map);
#else
SINT32 vp9hal_startdec(omxvdh_reg_cfg_s *p_vdh_reg_cfg);
#endif
#endif //__VDM_HAL_AVS_H__
