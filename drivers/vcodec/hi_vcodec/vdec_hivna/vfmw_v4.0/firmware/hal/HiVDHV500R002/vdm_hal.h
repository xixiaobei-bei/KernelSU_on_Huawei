#ifndef _VDM_HAL_HEADER_
#define _VDM_HAL_HEADER_

#include "vdm_drv.h"

#define   VDMHAL_OK                    (0)
#define   VDMHAL_ERR                   (-1)

#define   MAX_SLICE_SLOT_NUM           (200)

#define   MAX_IMG_WIDTH_IN_MB          (512)
#define   MAX_IMG_HALF_HEIGHT_IN_MB    (256)
#define   MAX_IMG_HEIGHT_IN_MB         (MAX_IMG_HALF_HEIGHT_IN_MB * 2)
#define   MAX_MB_NUM_IN_PIC            (MAX_IMG_WIDTH_IN_MB * MAX_IMG_HEIGHT_IN_MB)

#define   MAX_SLOT_WIDTH               (4096)
#ifndef   HIVCODEC_PLATFORM_ECONOMIC
#define   MAX_STRIDE                   ((1024 * MAX_SLOT_WIDTH / 64 + ((1024) - 1)) & (~ ((1024) - 1)))
#else
#define   ALIGN_LEN                    128
#define   MAX_STRIDE                   (((MAX_SLOT_WIDTH + ALIGN_LEN - 1) / ALIGN_LEN + 1) * ALIGN_LEN * 16)
#endif
/************************************************************************/
/*  Register read/write interface                                       */
/************************************************************************/
/* mfde register read/write */
#define RD_VREG( reg, dat, VdhId )               \
do {                    \
	if (VdhId < MAX_VDH_NUM)                \
		dat = readl(((volatile SINT32*)((SINT8*)g_HwMem[VdhId].pVdmRegVirAddr + reg))); \
	else                 \
		dprint(PRN_ALWS,"%s: RD_VREG but Vdh : %d is more than MAX_VDH_NUM : %d\n", __func__, VdhId, MAX_VDH_NUM); \
} while(0)

#define WR_VREG( reg, dat, VdhId )               \
do {                     \
	if (VdhId < MAX_VDH_NUM)                \
		writel((dat), ((volatile SINT32*)((SINT8*)g_HwMem[VdhId].pVdmRegVirAddr + reg))); \
	else                 \
		dprint(PRN_ALWS,"%s: WR_VREG but Vdh : %d is more than MAX_VDH_NUM : %d\n", __func__, VdhId, MAX_VDH_NUM); \
} while(0)

/* message pool read/write */
#define RD_MSGWORD( vir_addr, dat )          \
do {                 \
	dat = *((volatile SINT32*)(vir_addr));                      \
} while(0)

#define WR_MSGWORD( vir_addr, dat )          \
do {                 \
	*((volatile SINT32*)((SINT8*)(vir_addr))) = dat;                      \
} while(0)

/* condition check */
#define VDMHAL_ASSERT_RET( cond, else_print )     \
do {               \
	if (!(cond)) {           \
		dprint(PRN_FATAL,"%s %d: %s\n", __func__, __LINE__,  else_print ); \
		return VDMHAL_ERR;          \
	}               \
} while(0)

#endif
