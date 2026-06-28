


#ifndef __DRV_OMXVDEC_H__
#define __DRV_OMXVDEC_H__

#include <linux/ioctl.h>
#include "hi_type.h"
#include "scd_drv.h"

#define OMXVDEC_NAME                    "hi_vdec"
#define PATH_LEN                        (256)

/* ========================================================
 * internal struct enum definition
 * ========================================================*/


/* ========================================================
 * IOCTL for interaction with omx components
 * ========================================================*/
/* For Compat USER 32 -> KER 64, all pointer menbers set in last in structure.
    NOTICE: MUST NOT ALTER the member sequence of this structure */
typedef struct hi_OMXVDEC_IOCTL_MSG {
	HI_S32 chan_num;
	HI_S32 in_size;
	HI_S32 out_size;
	HI_VOID *in;
	HI_VOID *out;
} OMXVDEC_IOCTL_MSG;

//Modified for 64-bit platform
typedef struct hi_COMPAT_IOCTL_MSG {
	HI_S32 chan_num;
	HI_S32 in_size;
	HI_S32 out_size;
	compat_ulong_t in;
	compat_ulong_t out;
} COMPAT_IOCTL_MSG;

typedef SINT32(*VDEC_PROC_CMD) (OMXVDEC_IOCTL_MSG *pVdecMsg);

#define VDEC_IOCTL_MAGIC 'v'

#define VDEC_IOCTL_SET_CLK_RATE      \
	_IO(VDEC_IOCTL_MAGIC, 20)

#define VDEC_IOCTL_GET_VDM_HWSTATE      \
	_IO(VDEC_IOCTL_MAGIC, 21)

#define VDEC_IOCTL_SCD_PROC      \
	_IO(VDEC_IOCTL_MAGIC, 22)

#define VDEC_IOCTL_VDM_PROC      \
	_IO(VDEC_IOCTL_MAGIC, 23)

#define VDEC_IOCTL_LOCK_HW     \
	_IO(VDEC_IOCTL_MAGIC, 24)

#define VDEC_IOCTL_UNLOCK_HW     \
	_IO(VDEC_IOCTL_MAGIC, 25)
#endif
