
#ifndef __NV_STRU_PAM_H__
#define __NV_STRU_PAM_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "vos.h"
#include "nv_id_pam.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if (VOS_OS_VER != VOS_WIN32)
#pragma pack(4)
#else
#pragma pack(push, 4)
#endif
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define ESN_NV_DATA_LEN                             (8)
#define MEID_NV_DATA_LEN                            (8)

/*****************************************************************************
  3 枚举定义
*****************************************************************************/

/*****************************************************************************
 枚举名    : AT_PHY_PORT_ENUM
 枚举说明  : 物理端口号枚举值
*****************************************************************************/
enum AT_PHY_PORT_ENUM
{
    AT_UART_PORT = 0,
    AT_PCUI_PORT,
    AT_CTRL_PORT,
    AT_HSUART_PORT,
    AT_PCUI2_PORT,
    AT_PORT_BUTT
};
typedef VOS_UINT32  AT_PHY_PORT_ENUM_UINT32;

enum
{
    CPM_IND_PORT = AT_PORT_BUTT,    /* OM数据上报端口 */
    CPM_CFG_PORT,                   /* OM配置端口 */
    CPM_SD_PORT,
    CPM_WIFI_OM_IND_PORT,           /* WIFI下OM数据上报端口 */
    CPM_WIFI_OM_CFG_PORT,           /* WIFI下OM配置下发端口 */
    CPM_WIFI_AT_PORT,               /* WIFI下AT端口 */
    CPM_HSIC_IND_PORT,
    CPM_HSIC_CFG_PORT,
    CPM_VCOM_IND_PORT,              /* VCOM上OM数据上报接口 */
    CPM_VCOM_CFG_PORT,              /* VCOM上OM配置接口 */
    CPM_FS_PORT,
    CPM_PORT_BUTT
};
typedef VOS_UINT32  CPM_PHY_PORT_ENUM_UINT32;


enum
{
    CPM_CBT_PORT_USB,
    CPM_CBT_PORT_VCOM,
    CPM_CBT_PORT_SOCKET,
    CBP_CBT_PORT_BUTT
};
typedef VOS_UINT32 CPM_CBT_PORT_ENUM_UINT32;

enum
{
    CPM_OM_PORT_TYPE_USB,
    CPM_OM_PORT_TYPE_VCOM,
    CPM_OM_PORT_TYPE_WIFI,
    CPM_OM_PORT_TYPE_SD,
    CPM_OM_PORT_TYPE_FS,
    CPM_OM_PORT_TYPE_HSIC,
    CBP_OM_PORT_TYPE_BUTT
};
typedef VOS_UINT32 CPM_OM_PORT_ENUM_UINT32;


enum ESN_MEID_FLAG_ENUM
{
    ESN_MEID_DISABLE        = 0,
    ESN_ME_ENABLE_FLAG      = 1,
    MEID_ME_ENABLE_FLAG     = 2,
    ESN_MEID_ENABLE_BOTH    = 3,
    ESN_MEID_ENABLE_FLAG_BUTT
};
typedef VOS_UINT8 ESN_MEID_FLAG_ENUM_UINT8;

/*****************************************************************************
  4 全局变量声明
*****************************************************************************/

/*****************************************************************************
  5 消息头定义
*****************************************************************************/

/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/

typedef struct
{
    VOS_UINT8                           ucTempOverCount;     /*在正常模式下温度连续超过告警门限或低功耗门限的极限次数*/
    VOS_UINT8                           ucTempRsumeCount;    /*在低功耗模式或告警模式下温度连续低于告警门的极限次数*/
    VOS_UINT8                           ucTempCloseCount;    /*在降速模式下温度连续超过告警门限的极限次数*/
    VOS_UINT8                           ucTempPowerOffCount; /*超过下电门限的极限次数*/
    VOS_UINT32                          ulWaitTimer;         /*等待定时器时长*/
}TEMP_PARA_CTRL_STRU;


typedef struct
{
    VOS_UINT32                          ulIsEnable;           /*是否需要启动保护机制, 0为关闭，1为开启*/
    VOS_UINT32                          ulLocation;           /*使用哪里的温度作为温保的输入，该参数直接作为入参进入底软的函数,目前的取值范围是0到6 */
    VOS_INT32                           lPowerOffThreshold;   /*进入下电模式的门限*/
    VOS_INT32                           lCloseAdcThreshold;   /*进入低功耗模式的温度门限*/
    VOS_INT32                           lAlarmAdcThreshold;   /*需要进行给出警报的温度门限*/
    VOS_INT32                           lResumeAdcThreshold;  /*恢复到正常模式的温度门限*/
    VOS_INT32                           lSIMPowerOffThreshold;  /*E5形态下SIM过温保护门限*/
    TEMP_PARA_CTRL_STRU                 stTempCtrlPara;
}SPY_TEMP_PROTECT_NV_STRU;


typedef struct
{
    CPM_OM_PORT_ENUM_UINT32             enPortNum;         /* 物理通道类型 */
    CPM_CBT_PORT_ENUM_UINT32            enCbtPortNum;      /* 校准通道类型 */
}OM_CHANNLE_PORT_CFG_STRU;


enum USIMM_NV_CARDAPP_ENUM
{
    USIMM_NV_GUTL_APP          = 0x00000000,
    USIMM_NV_IMS_APP           = 0x00000001,
    USIMM_NV_CDMA_APP          = 0x00000002,
    USIMM_NV_CARDAPP_BUTT      = 0x00000003
};
typedef VOS_UINT32 USIMM_NV_CARDAPP_ENUM_UINT32;


typedef struct
{
    VOS_UINT8                           ucAppNum;
    VOS_UINT8                           aucRsv[3];
    USIMM_NV_CARDAPP_ENUM_UINT32        aenAppList[5];
}USIMM_APP_PRIORITY_CONFIG_STRU;


typedef struct
{
    VOS_UINT16                          usImei0ReplaseImei2;
    VOS_UINT16                          usRsv;
}NV_SC_PERS_CTRL_STRU;


typedef struct
{
    VOS_UINT16                          usFlag;
    VOS_UINT16                          usTimerValue;
}USIMM_CMD_DEBUG_MODE_ST;


typedef struct
{
    ESN_MEID_FLAG_ENUM_UINT8        enEsnMeIDFlag;
    VOS_UINT8                       ucRSv;
    VOS_UINT8                       aucEsn[ESN_NV_DATA_LEN];  /*Len+Data*/
    VOS_UINT8                       aucMeID[MEID_NV_DATA_LEN]; /*Len+Data*/
    VOS_UINT8                       aucRfu[6];
}NV_ESN_MEID_STRU;


typedef VOS_UINT32 SI_PB_STORATE_TYPE;
typedef struct
{
    SI_PB_STORATE_TYPE                  enCurMaterial;
    VOS_UINT8                           aucPasswords[8];
    VOS_UINT16                          usUsed;
    VOS_UINT16                          usTotal;
    VOS_UINT8                           ucAnrMaxNum;        /*Balong支持的ANR的最大值*/
    VOS_UINT8                           ucEmailFlag;        /*Balong支持Email标记*/
    VOS_UINT8                           ucSPBFlag;         /*复合电话本支持标记，为1时表示支持*/
    VOS_UINT8                           ucRsv;
}SI_PB_CTRL_INFO_ST;


typedef struct
{
    VOS_UINT8                           ucProfileLen;
    VOS_UINT8                           aucProfile[47];
}USIMM_USIM_PROFILE_STRU;


typedef struct
{
    VOS_UINT8                           ucProfileLen;
    VOS_UINT8                           aucProfile[31];
}USIMM_SIM_PROFILE_STRU;

typedef struct
{
    VOS_UINT8                           ucFuncEnable;
    VOS_UINT8                           ucTimer;
    VOS_UINT8                           ucTerminalType;
    VOS_UINT8                           ucRsv;
    USIMM_USIM_PROFILE_STRU             stUsimProfile;
    USIMM_SIM_PROFILE_STRU              stSimProfile;
}USIMM_STK_CFG_STRU;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/

#if (VOS_OS_VER != VOS_WIN32)
#pragma pack()
#else
#pragma pack(pop)
#endif



#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif
