/******************************************************************************

                  版权所有 (C), 2001-2016, 华为技术有限公司

 ******************************************************************************
  文 件 名   : errlog_cfg.c
  版 本 号   : 初稿
  作    者   : d00212987
  生成日期   : 2016年2月19日
  最近修改   :
  功能描述   : Errlog关联列表模块
  函数列表   :
  修改历史   :
  1.日    期   : 2016年2月19日
    作    者   : d00212987
    修改内容   : Errlo关联列表新增

******************************************************************************/
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "errlog_cfg.h"


/*lint -e767 修改人:d00212987;原因:Log打印 */
//#define    THIS_FILE_ID        PS_FILE_ID_ERRLOG_CFG_C
/*lint +e767 修改人:d00212987;*/

/*****************************************************************************
  2 全局变量声明
*****************************************************************************/

/* Warning：代码自动生成区域，不能手工改动 begin */
/* TODO: g_aulModemErrRept hash_value:001e13bf53cc4cff835a0c12e5a166fc */
/* TODO: python search flag satrt */
int g_aulModem0ErrRept[][3]=
{
    /* GU ErrLog 上报相关 Item:13 */
    {FAULT_ID_GU_ERR_LOG_REPT, I0_WUEPS_PID_MMC, 0},
    {FAULT_ID_GU_ERR_LOG_REPT, I0_UEPS_PID_MTA, 0},
    {FAULT_ID_GU_ERR_LOG_REPT, I0_WUEPS_PID_USIM, 0},
    {FAULT_ID_GU_ERR_LOG_REPT, I0_UEPS_PID_GAS, 0},
    {FAULT_ID_GU_ERR_LOG_REPT, I0_UEPS_PID_SN, 0},
    {FAULT_ID_GU_ERR_LOG_REPT, WUEPS_PID_WRR, 0},
    {FAULT_ID_GU_ERR_LOG_REPT, I0_DSP_PID_APM, 0},
    {FAULT_ID_GU_ERR_LOG_REPT, PS_PID_ERRC, 0},
    {FAULT_ID_GU_ERR_LOG_REPT, TPS_PID_RRC, 0},
    {FAULT_ID_GU_ERR_LOG_REPT, PS_PID_MM, 0},
    {FAULT_ID_GU_ERR_LOG_REPT, TPS_PID_MAC, 0},
    {FAULT_ID_GU_ERR_LOG_REPT, PS_PID_MAC_UL, 0},
    {FAULT_ID_GU_ERR_LOG_REPT, TLPHY_PID_RTTAGENT, 0},



};

int g_aulModem1ErrRept[][3]=
{
    /* GU ErrLog 上报相关 Item:6 */
    {FAULT_ID_GU_ERR_LOG_REPT, I1_WUEPS_PID_MMC, 0},
    {FAULT_ID_GU_ERR_LOG_REPT, I1_UEPS_PID_MTA, 0},
    {FAULT_ID_GU_ERR_LOG_REPT, I1_WUEPS_PID_USIM, 0},
    {FAULT_ID_GU_ERR_LOG_REPT, I1_UEPS_PID_GAS, 0},
    {FAULT_ID_GU_ERR_LOG_REPT, I1_UEPS_PID_SN, 0},
    {FAULT_ID_GU_ERR_LOG_REPT, I1_DSP_PID_APM, 0},



};

int g_aulModem2ErrRept[][3]=
{
    /* GU ErrLog 上报相关 Item:6 */
    {FAULT_ID_GU_ERR_LOG_REPT, I2_WUEPS_PID_MMC, 0},
    {FAULT_ID_GU_ERR_LOG_REPT, I2_UEPS_PID_MTA, 0},
    {FAULT_ID_GU_ERR_LOG_REPT, I2_WUEPS_PID_USIM, 0},
    {FAULT_ID_GU_ERR_LOG_REPT, I2_UEPS_PID_GAS, 0},
    {FAULT_ID_GU_ERR_LOG_REPT, I2_UEPS_PID_SN, 0},
    {FAULT_ID_GU_ERR_LOG_REPT, I2_DSP_PID_APM, 0},



};
/* TODO: python search flag end */
/* Warning：代码自动生成区域，不能手工改动 end */

/*****************************************************************************
  3 函数申明
*****************************************************************************/

/*****************************************************************************
 函 数 名  : ErrLog_GetErrReptAddrAndSize
 功能描述  : 获取Errlog全局变量地址
 输入参数  : ulModemId 主modem/副modem的id

 输出参数  : pulErrLogAddr
             pulsize

 返 回 值  : 0：success  其他：fail
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年02月19日
    作    者   : d00212987
    修改内容   :  ERR LOG FAULT ID关联项目新增
*****************************************************************************/
VOS_INT32 ErrLog_GetErrReptAddrAndSize(
    VOS_UINT32                           ulModemId,
    VOS_UINT_PTR                        *pulErrLogAddr,
    VOS_UINT32                          *pulsize
)
{
    if (MODEM_ID_0 == ulModemId)
    {
        *pulErrLogAddr = (VOS_UINT_PTR)g_aulModem0ErrRept;
        *pulsize = sizeof(g_aulModem0ErrRept);
    }
    else if (MODEM_ID_1 == ulModemId)
    {
        *pulErrLogAddr = (VOS_UINT_PTR)g_aulModem1ErrRept;
        *pulsize = sizeof(g_aulModem1ErrRept);

    }
    else if (MODEM_ID_2 == ulModemId)
    {
        *pulErrLogAddr = (VOS_UINT_PTR)g_aulModem2ErrRept;
        *pulsize = sizeof(g_aulModem2ErrRept);
    }
    else
    {
        return VOS_ERR;
    }

    return VOS_OK;


}

/*****************************************************************************
 函 数 名  : ErrLog_GetPidAndAlarmId
 功能描述  : ulFaultId关联的pid/alarm_id查询接口
 输入参数  : ulModemId 主modem/副modem的id
             ulFaultId fault id

 输出参数  : paustAlarmArray fault id相关联的pid alarm_id数组，内存空间由diag_om提供，最大长度2048，支持一个faultid关联256个alarm_id；
             pulAlarmNum     fault id相关联的pid alarm_id数组成员个数，最大256；

 返 回 值  : 0：success  其他：fail
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年02月19日
    作    者   : d00212987
    修改内容   : ERR LOG FAULT ID关联项目新增
*****************************************************************************/
VOS_INT32 ErrLog_GetPidAndAlarmId(
    VOS_UINT32                          ulModemId,
    VOS_UINT32                          ulFaultId,
    ERR_LOG_ALARM_STRU                 *paustAlarmArray,
    VOS_UINT32                         *pulAlarmNum
)
{
    VOS_UINT_PTR                        ulErrLogAddr = VOS_NULL;
    VOS_UINT32                          ulsize;
    VOS_UINT32                          ulCount = 0;
    VOS_UINT32                          i;
    VOS_UINT32                          (*pulErrLogArry)[3];

    if (VOS_NULL_PTR == paustAlarmArray)
    {
        return VOS_ERR;
    }

    if (VOS_OK != ErrLog_GetErrReptAddrAndSize(ulModemId, &ulErrLogAddr, &ulsize))
    {
        return VOS_ERR;
    }

    pulErrLogArry = (VOS_UINT32(*)[])ulErrLogAddr;

    for (i=0; i < (ulsize / (3 * sizeof(VOS_UINT32))); i++)
    {
        if (ulFaultId == pulErrLogArry[i][0])
        {
            if (FAULT_ID_CONTAIN_ALARM_ID_MAX_NUM <= ulCount)
            {
                return VOS_ERR;
            }

            paustAlarmArray[ulCount].ulPid      = pulErrLogArry[i][1];
            paustAlarmArray[ulCount].ulAlarmID  = pulErrLogArry[i][2];
            ulCount++;
        }
    }

    *pulAlarmNum = ulCount;

    return VOS_OK;
}

/*****************************************************************************
 函 数 名  : ErrLog_IsContainPID
 功能描述  : 检查PID是否已经存在
 输入参数  : PID数组已及大小

 输出参数  : 是否存在

 返 回 值  : VOS_TRUE：存在  其他：VOS_FALSE
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年02月19日
    作    者   : d00212987
    修改内容   : ERR LOG FAULT ID关联项目新增
*****************************************************************************/
VOS_BOOL ErrLog_IsContainPID(
    VOS_UINT32                         *PaulPidArray,
    VOS_UINT32                          ulPidArrayLen,
    VOS_UINT32                          ulDstPid
)
{
    VOS_UINT32                          i;

    for (i=0; i < ulPidArrayLen; i++)
    {
        if (ulDstPid == PaulPidArray[i])
        {
            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}

/*****************************************************************************
 函 数 名  : ErrLog_GetErrlogPid
 功能描述  : 提供errlog相关的pid列表，用于errlog开关控制给各pid发送消息
 输入参数  : ulModemId 主modem/副modem的id

 输出参数  : PaulPidArray   errlog相关联的pid数组，内存空间由diag_om提供，最大长度1024，支持256个pid；
             pulPidNum      errlog相关联的pid数组成员个数，最大256；

 返 回 值  : 0：success  其他：fail
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年02月19日
    作    者   : d00212987
    修改内容   :  ERR LOG FAULT ID关联项目新增
*****************************************************************************/
VOS_INT32 ErrLog_GetErrlogPid(
    VOS_UINT32                          ulModemId,
    VOS_UINT32                         *PaulPidArray,
    VOS_UINT32                         *pulPidNum
)
{
    VOS_UINT_PTR                        ulErrLogAddr = VOS_NULL;
    VOS_UINT32                          ulsize;
    VOS_UINT32                          ulCount = 0;
    VOS_UINT32                          i;
    VOS_UINT32                          (*pulErrLogArry)[3];

    if (VOS_NULL_PTR == PaulPidArray)
    {
        return VOS_ERR;
    }

    if (VOS_OK != ErrLog_GetErrReptAddrAndSize(ulModemId, &ulErrLogAddr, &ulsize))
    {
        return VOS_ERR;
    }

    pulErrLogArry = (VOS_UINT32(*)[])ulErrLogAddr;

    for (i=0; i < (ulsize / (3 * sizeof(VOS_UINT32))); i++)
    {
        if (0 == i)
        {
            PaulPidArray[0] = pulErrLogArry[i][1];
            ulCount++;
            continue;
        }

        if (VOS_TRUE != ErrLog_IsContainPID(PaulPidArray, ulCount, pulErrLogArry[i][1]))
        {
            if (ERR_LOG_PID_MAX_NUM <= ulCount)
            {
                return VOS_ERR;
            }

            PaulPidArray[ulCount] = pulErrLogArry[i][1];
            ulCount++;
        }
    }

    *pulPidNum = ulCount;

    return VOS_OK;
}



