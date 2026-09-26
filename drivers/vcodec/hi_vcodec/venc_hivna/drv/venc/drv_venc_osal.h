#ifndef  __DRV_VENC_OSAL_H__
#define  __DRV_VENC_OSAL_H__

#include "hi_type.h"
#include "hal_venc.h"
#include <linux/rtc.h>


typedef struct hiKERN_EVENT_S
{
	wait_queue_head_t   queue_head;
	HI_S32              flag;
} KERN_EVENT_S;

typedef KERN_EVENT_S  VEDU_OSAL_EVENT;
typedef unsigned long VEDU_LOCK_FLAG;

#define  MESCS_TO_JIFFIES(time)                         msecs_to_jiffies(time)
#define  HiWaitEvent( pEvent, flag)                     wait_event_interruptible((pEvent), (flag))
#define  HiWaitEventTimeOut( pEvent, flag, msWaitTime)  wait_event_interruptible_timeout((pEvent), (flag), (msWaitTime))

extern HI_U32  g_VencPrintEnable;

typedef enum {
	VENC_FATAL = 0,
	VENC_ERR,
	VENC_WARN,
	VENC_INFO,
	VENC_DBG,
	VENC_ALW,
}VENC_PRINT_TYPE;

#define HI_FATAL_VENC(fmt,...) HI_PRINT(VENC_FATAL,(char *)__FILE__, (int)__LINE__, (char *)__FUNCTION__, fmt, ##__VA_ARGS__)
#define HI_ERR_VENC(fmt,...)   HI_PRINT(VENC_ERR,(char *)__FILE__, (int)__LINE__, (char *)__FUNCTION__, fmt, ##__VA_ARGS__)
#define HI_WARN_VENC(fmt,...)  HI_PRINT(VENC_WARN,(char *)__FILE__, (int)__LINE__, (char *)__FUNCTION__, fmt,##__VA_ARGS__)
#define HI_INFO_VENC(fmt,...)  HI_PRINT(VENC_INFO,(char *)__FILE__, (int)__LINE__, (char *)__FUNCTION__, fmt,##__VA_ARGS__)
#define HI_DBG_VENC(fmt,...)   HI_PRINT(VENC_DBG,(char *)__FILE__, (int)__LINE__, (char *)__FUNCTION__, fmt,##__VA_ARGS__)

HI_VOID     HI_PRINT(HI_U32 type, char *file, int line, char *function, HI_CHAR *msg, ... );
HI_U32*      HiMmap(HI_U32 Addr ,HI_U32 Range);
HI_VOID      HiMunmap(HI_U32 * pMemAddr);
HI_S32       HiStrNCmp(const HI_PCHAR pStrName,const HI_PCHAR pDstName,HI_S32 nSize);
HI_VOID      HiSleepMs(HI_U32 a_MilliSec);
HI_VOID     *HiMemVAlloc(HI_U32 nMemSize);
HI_VOID      HiMemVFree(HI_VOID * pMemAddr);
HI_S32       HiMemSet(HI_VOID * a_pHiDstMem, HI_S32 a_Value, size_t a_Size);
HI_S32       HiMemCpy(HI_VOID * a_pHiDstMem, HI_VOID * a_pHiSrcMem, size_t a_Size);
HI_VOID      HiVENC_INIT_MUTEX(HI_VOID* pSem);
HI_S32       HiVENC_DOWN_INTERRUPTIBLE(HI_VOID* pSem);
HI_VOID      HiVENC_UP_INTERRUPTIBLE(HI_VOID* pSem);

//#endif //endif XY_ADD
HI_S32   VENC_DRV_OsalIrqInit(HI_U32 Irq, HI_VOID(*ptrCallBack)(HI_VOID));
HI_VOID  VENC_DRV_OsalIrqFree(HI_U32 Irq);
HI_S32   VENC_DRV_OsalLockCreate (HI_VOID** phLock);
HI_VOID  VENC_DRV_OsalLockDestroy(HI_VOID* hLock);

HI_U32 GetTimeInUs(HI_VOID);
HI_S32 VENC_DRV_OsalInitEvent( VEDU_OSAL_EVENT *pEvent, HI_S32 InitVal );
HI_S32 VENC_DRV_OsalGiveEvent( VEDU_OSAL_EVENT *pEvent );
HI_S32 VENC_DRV_OsalWaitEvent( VEDU_OSAL_EVENT *pEvent, HI_U32 msWaitTime );


#endif //__DRV_VENC_OSAL_H__

