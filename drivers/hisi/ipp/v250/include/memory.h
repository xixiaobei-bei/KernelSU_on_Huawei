#ifndef _MEMORY_CS_H_INCLUDED
#define _MEMORY_CS_H_INCLUDED

#include "ipp.h"
#include "adapter_common.h"
#include "cfg_table_mcf.h"
#include "cfg_table_cpe_top.h"
#include "config_table_cvdr.h"
#include "slam_common.h"
#include "cfg_table_slam.h"
#include "cfg_table_reorder.h"
#include "cfg_table_compare.h"
#include "cfg_table_vbk.h"

typedef enum _cpe_mem_id {
	MEM_ID_CMDLST_BUF_MM       = 0,
	MEM_ID_CMDLST_ENTRY_MM     = 1,
	MEM_ID_CMDLST_PARA_MM      = 2,
	MEM_ID_MCF_CFG_TAB         = 3,
	MEM_ID_CPETOP_CFG_TAB_MM   = 4,
	MEM_ID_CVDR_CFG_TAB_MM     = 5,
	MEM_ID_SLAM_CFG_TAB        = 6,
	MEM_ID_CMDLST_BUF_SLAM     = 7,
	MEM_ID_CMDLST_ENTRY_SLAM   = 8,
	MEM_ID_CMDLST_PARA_SLAM    = 9,
	MEM_ID_CPETOP_CFG_TAB_SLAM = 10,
	MEM_ID_CVDR_CFG_TAB_SLAM   = 11,
	MEM_ID_SLAM_STAT           = 12,
	MEM_ID_VBK_CFG_TAB         = 13,
	MEM_ID_REORDER_CFG_TAB        = 14,
	MEM_ID_CMDLST_BUF_REORDER     = 15,
	MEM_ID_CMDLST_ENTRY_REORDER   = 16,
	MEM_ID_CMDLST_PARA_REORDER    = 17,
	MEM_ID_CPETOP_CFG_TAB_REORDER = 18,
	MEM_ID_CVDR_CFG_TAB_REORDER   = 19,
	MEM_ID_COMPARE_CFG_TAB        = 20,
	MEM_ID_CMDLST_BUF_COMPARE     = 21,
	MEM_ID_CMDLST_ENTRY_COMPARE   = 22,
	MEM_ID_CMDLST_PARA_COMPARE    = 23,
	MEM_ID_CPETOP_CFG_TAB_COMPARE = 24,
	MEM_ID_CVDR_CFG_TAB_COMPARE   = 25,
	MEM_ID_READ_COMPARE_INDEX     = 26,
	MEM_ID_COMPARE_MATCHED_KPT    = 27,
	MEM_ID_MAX
} cpe_mem_id;

typedef struct _cpe_va_da {
	unsigned long long va;
	unsigned int       da;
} cpe_va_da;

extern int cpe_mem_init(unsigned long long va, unsigned int da, unsigned int size);
extern int cpe_mem_get(cpe_mem_id mem_id, unsigned long long *va, unsigned int *da);
extern void cpe_mem_free(cpe_mem_id mem_id);

extern int cpe_init_memory(unsigned int ipp_buff_size);
extern void *get_cpe_addr_va(void);
extern unsigned int get_cpe_addr_da(void);

#endif /*_MEMORY_CS_H_INCLUDED*/



