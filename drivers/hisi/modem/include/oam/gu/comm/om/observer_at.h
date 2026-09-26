

#ifndef __OBSERVER_AT_H__
#define __OBSERVER_AT_H__

#include "vos.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

/*
 * 功能说明: SPY与AT消息接口枚举
 */
enum TEMPPRT_AT_MsgId {
    /* SPY -> AT */
    ID_TEMPPRT_AT_EVENT_IND = 0,    /* _H2ASN_MsgChoice TEMP_PROTECT_EventAtInd */

    ID_TEMPPRT_STATUS_AT_EVENT_IND, /* _H2ASN_MsgChoice TEMP_PROTECT_EventAtInd */

    ID_TEMPPRT_AT_MSG_ID_BUTT

};
typedef VOS_UINT32 TEMPPRT_AT_MsgIdUint32;
/*
 * 功能说明: SPY给AT上报的消息结构
 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                  msgId;
    VOS_UINT32                  tempProtectEvent;
    VOS_UINT32                  tempProtectParam;
} TEMP_PROTECT_EventAtInd;

typedef struct {
    VOS_UINT32 isEnable;           /* 是否需要启动保护机制, 0为关闭，1为开启 */
    VOS_INT32  closeAdcThreshold;  /* 进入低功耗模式的温度门限 */
    VOS_INT32  alarmAdcThreshold;  /* 需要进行给出警报的温度门限 */
    VOS_INT32  resumeAdcThreshold; /* 恢复到正常模式的温度门限 */
} SPY_TempThresholdPara;

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
