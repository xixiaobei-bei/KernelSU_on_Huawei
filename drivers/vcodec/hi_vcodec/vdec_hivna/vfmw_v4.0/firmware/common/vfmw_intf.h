

#ifndef __VFMW_INTF_H__
#define __VFMW_INTF_H__
#include "memory.h"
#include "vdm_drv.h"
#include "scd_drv.h"
#include "platform.h"

#define VCTRL_OK                0
#define VCTRL_ERR              (-1)
#define MSG_POOL_ADDR_CHECK
#define VFMW_OSAL_SLEEP_TIME    10
#define VFMW_OSAL_SLEEP_COUNT   30


typedef struct hi_drv_mem_s {
	mem_record_s st_vdh_reg;
} drv_mem_s;

typedef struct {
	mem_buffer_s scd[SCD_SHAREFD_MAX];
	mem_buffer_s vdh[VDH_SHAREFD_MAX];
	clk_rate_e   clk_rate;
	struct file  *file;
} vdec_mem_info;

#ifdef MSG_POOL_ADDR_CHECK
SINT32 check_frm_buf_addr(UADDR  src_pmv_addr, mem_buffer_s *p_vdh_mem_map);
SINT32 check_pmv_buf_addr(UADDR  src_pmv_addr, mem_buffer_s *p_vdh_mem_map);
SINT32 vctrl_vdh_unmap_message_pool(mem_buffer_s *p_mem_map);
#endif

SINT32 vctrl_open_drivers(void);

SINT32 vctrl_open_vfmw(void);

SINT32 vctrl_close_vfmw(void);

SINT32 vctrl_vdm_hal_process(
	omxvdh_reg_cfg_s *p_vdm_reg_cfg, vdmhal_backup_s *p_vdm_reg_statei,
	mem_buffer_s *p_vdh_mem_map, mem_buffer_s *p_com_msg_map);

SINT32 vctrl_scd_hal_process(
	omx_scd_reg_cfg_s *p_scd_reg_cfg, scd_state_reg_s *p_scd_state_reg,
	mem_buffer_s *p_scd_mem_map);

SINT32 vctrl_vdm_hal_is_run(void);

VOID vctrl_suspend(void);

VOID vctrl_resume(void);

hi_bool vctrl_scen_ident(hi_u32 cmd);

#endif
