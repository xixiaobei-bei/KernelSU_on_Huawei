#ifndef __DRV_VENC_EFL_H__
#define __DRV_VENC_EFL_H__

#include "hi_type.h"
#include "hi_drv_mem.h"


enum {
	VEDU_H265 = 0,
	VEDU_H264 = 1
};

typedef struct {
	HI_U32    IpFree;       /* for channel control */
	HI_HANDLE CurrHandle;   /* used in ISR */
	HI_U32   *pRegBase;
	HI_VOID  *pChnLock;     /* lock ChnCtx[MAX_CHN] */
	HI_VOID  *pTask_Frame;  /* for both venc & omxvenc */
	HI_VOID  *pTask_Stream; /* juse for omxvenc */
	HI_U32    StopTask;
	HI_U32    TaskRunning;  /* to block Close IP */
	HI_U32    bReEncode;
} VeduEfl_IpCtx_S;

typedef struct {
	HI_U32 IsFPGA;
	HI_U32 VeduIrqNumNorm;
	HI_U32 VeduIrqNumProt;
	HI_U32 VeduIrqNumSafe;
	HI_U32 VencRegBaseAddr;
	HI_U32 VencRegRange;
	HI_U32 normalRate;
	HI_U32 highRate;
	HI_U32 lowRate;
	HI_U64 SmmuPageBaseAddr;
	HI_U32 VencQosMode;
	HI_U32 svsLowerRate;
} VeduEfl_DTS_CONFIG_S;


typedef enum {
	YUV420_SEMIPLANAR     = 0,
	YUV420_PLANAR         = 3,
	YUV422_PLANAR         = 4,
	YUV422_PACKAGE        = 6,
	RGB_32BIT             = 8,
	YUV420_SEMIPLANAR_CMP = 10,
} COLOR_FMT_TYPE;

HI_S32	VENC_DRV_EflOpenVedu(HI_VOID);
HI_S32	VENC_DRV_EflCloseVedu(HI_VOID);
HI_S32	VENC_DRV_EflResumeVedu(HI_VOID);
HI_S32  VENC_DRV_EflSuspendVedu(HI_VOID);
HI_S32  VENC_SetDtsConfig(VeduEfl_DTS_CONFIG_S* info);

/*************************************************************************************/


#endif //__DRV_VENC_EFL_H__

