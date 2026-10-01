#ifndef __VDEC_FIRMWARE_H__
#define __VDEC_FIRMWARE_H__

#if defined(VFMW_EXTRA_TYPE_DEFINE)
#include "hi_type.h"
#endif

#define  VFMW_VERSION_NUM       (2017032400)

#if defined(VFMW_EXTRA_TYPE_DEFINE)
#define UINT64 HI_U64
#define UINT32 HI_U32
#define SINT32 HI_S32
#define UINT8  HI_U8
#define SINT8  HI_S8
#define ULONG  HI_SIZE_T
#define UADDR  HI_U32

#ifndef VOID
#define VOID   HI_VOID
#endif

typedef unsigned   USIGN;
#else
typedef  unsigned long long UINT64;
typedef  unsigned int       UINT32;
typedef  signed int         SINT32;
typedef  signed char        SINT8;
typedef  unsigned char      UINT8;
typedef  void               VOID;
typedef  unsigned int       UADDR;
typedef  unsigned           USIGN;
typedef  unsigned long      ULONG;
#endif

#ifndef NULL
#define NULL               0L
#endif

#ifdef HI_TVP_SUPPORT
#define  TVP_CHAN_NUM            (2)
#else
#define  TVP_CHAN_NUM            (0)
#endif

#ifdef ENV_SOS_KERNEL
#define  MAX_CHAN_NUM            (TVP_CHAN_NUM)
#else
#define  MAX_CHAN_NUM            (32)
#endif

#define MAX_FRAME_NUM           (32)

#define  VDEC_OK                (0)
#define  VDEC_ERR               (-1)
#define  VF_ERR_SYS             (-20)

typedef enum {
	VFMW_START_RESERVED = 0,
	VFMW_H264           = 0,
	VFMW_VC1,
	VFMW_MPEG4,
	VFMW_MPEG2,
	VFMW_H263,
	VFMW_DIVX3,
	VFMW_AVS,
	VFMW_JPEG,
	VFMW_REAL8 = 8,
	VFMW_REAL9 = 9,
	VFMW_VP6   = 10,
	VFMW_VP6F,
	VFMW_VP6A,
	VFMW_VP8,
	VFMW_VP9,
	VFMW_SORENSON,
	VFMW_MVC,
	VFMW_HEVC,
	VFMW_RAW,
	VFMW_USER,    /*## vfmw simply provide frame path. for external decoder, eg. mjpeg ## */
	VFMW_END_RESERVED
} VID_STD_E;

/*memory type*/
typedef enum {
	MEM_ION = 0,    // ion default
	MEM_ION_CTG,    // ion contigeous
	MEM_CMA,        // kmalloc
	MEM_CMA_ZERO,    // kzalloc
} MEM_TYPE_E;

/* memroy description */
typedef struct {
	UINT8 IsSecure;
	MEM_TYPE_E MemType;
	UINT64 PhyAddr;
	UINT32 Length;
	UINT64 VirAddr;
} MEM_DESC_S;

typedef struct {
	UINT32 IsFPGA;
	UINT32 VdecIrqNumNorm;
	UINT32 VdecIrqNumProt;
	UINT32 VdecIrqNumSafe;
	UINT32 VdhRegBaseAddr;
	UINT32 VdhRegRange;
	UINT64 SmmuPageBaseAddr;
	UINT32 PERICRG_RegBaseAddr;
	UINT32 VdecQosMode;
} VFMW_DTS_CONFIG_S;

#endif    // __VDEC_FIRMWARE_H__
