

#ifndef __VDEC_FIRMWARE_H__
#define __VDEC_FIRMWARE_H__

#if defined(VFMW_EXTRA_TYPE_DEFINE)
#include "hi_type.h"
#endif

#define  VFMW_VERSION_NUM       2017032400

#if defined(VFMW_EXTRA_TYPE_DEFINE)
#define UINT64 hi_u64
#define UINT32 hi_u32
#define SINT32 hi_s32
#define UINT8  hi_u8
#define SINT8  hi_s8
#define ULONG  hi_size_t
#define UADDR  hi_u32

#ifndef VOID
#define VOID   hi_void
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
#define  TVP_CHAN_NUM             2
#else
#define  TVP_CHAN_NUM             0
#endif

#ifdef ENV_SOS_KERNEL
#define  MAX_CHAN_NUM            (TVP_CHAN_NUM)
#else
#define  MAX_CHAN_NUM             32
#endif

#define MAX_FRAME_NUM             32

#define  VDEC_OK                  0
#define  VDEC_ERR                (-1)
#define  VF_ERR_SYS              (-20)

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
	// vfmw simply provide frame path.
	// for external decoder, eg. mjpeg
	VFMW_USER,
	VFMW_END_RESERVED
} vid_std_e;

/* memory type */
typedef enum {
	MEM_ION = 0,    // ion default
	MEM_ION_CTG,    // ion contigeous
	MEM_CMA,        // kmalloc
	MEM_CMA_ZERO,    // kzalloc
} mem_type_e;

/* memroy description */
typedef struct {
	UINT8 is_secure;
	mem_type_e mem_type;
	UINT64 phy_addr;
	UINT32 length;
	UINT64 vir_addr;
} mem_desc_s;

typedef struct {
	UINT32 is_fpga;
	UINT32 vdec_irq_num_norm;
	UINT32 vdec_irq_num_prot;
	UINT32 vdec_irq_num_safe;
	UINT32 vdh_reg_base_addr;
	UINT32 vdh_reg_range;
	UINT64 smmu_page_base_addr;
	UINT32 pericrg_reg_base_addr;
	UINT32 vdec_qos_mode;
} vfmw_dts_config_s;

#endif    // __VDEC_FIRMWARE_H__
