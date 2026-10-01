
#ifndef __DRV_VENC_IOCTL_H__
#define __DRV_VENC_IOCTL_H__

#include "Vedu_RegAll.h"
#include "hi_type.h"

#define MAX_STREAMBUF_NUM  (16)

typedef enum
{
	VENC_SET_CFGREG = 100,
	VENC_SET_CFGREGSIMPLE
}CMD_TYPE;

typedef enum {
#ifdef HIVCODECV500
	VENC_CLK_RATE_LOWER = 0,
#endif
	VENC_CLK_RATE_LOW,
	VENC_CLK_RATE_NORMAL,
	VENC_CLK_RATE_HIGH,
} VENC_CLK_TYPE;

typedef struct {
	HI_S32 share_fd;
	HI_U32 iova;
	HI_U64 vir_addr;
	HI_U32 iova_size;
} VencBufferRecord;


typedef struct
{
	HI_S32   InteralShareFd;
	HI_S32   ImageShareFd;
	HI_S32   StreamShareFd[MAX_STREAMBUF_NUM];
	HI_S32   StreamHeadShareFd;
}VENC_MEM_INFO_S;

typedef struct
{
	HI_U32 recLumaSize;
	HI_U32 recChromaSize;
	HI_U32 recLumaHeadSize;
	HI_U32 recChromaHeadSize;
	HI_U32 qpgldSize;
	HI_U32 nbiSize;
	HI_U32 pmeSize;
	HI_U32 pmeInfoSize;
	HI_U32 veduSrcYLength;
	HI_U32 veduSrcCLength;
	HI_U32 veduSrcVLength;
	HI_U32 veduSrcYHLength;
	HI_U32 veduSrcCHLength;
	HI_U32 masterStreamStart;
	HI_U32 masterStreamEnd;
}VENC_BUFFER_ALLOC_INFO;

typedef struct
{
	CMD_TYPE cmd;

	HI_BOOL bResetReg;
	HI_BOOL bClkCfg;
	HI_BOOL bFirstNal2Send;
	unsigned int   bSecureFlag;
	U_FUNC_VCPI_RAWINT    hw_done_type;
	S_HEVC_AVC_REGS_TYPE_CFG all_reg;
	VENC_CLK_TYPE clk_type;
	VENC_MEM_INFO_S mem_info;
	VENC_BUFFER_ALLOC_INFO venc_inter_buffer;
}VENC_REG_INFO_S;

#define CMD_VENC_START_ENCODE          _IOWR(IOC_TYPE_VENC, 0x32, VENC_REG_INFO_S)
#define CMD_VENC_IOMMU_MAP             _IOWR(IOC_TYPE_VENC, 0x33, VencBufferRecord)
#define CMD_VENC_IOMMU_UNMAP           _IOWR(IOC_TYPE_VENC, 0x34, VencBufferRecord)

#endif //__HI_DRV_VENC_H__

