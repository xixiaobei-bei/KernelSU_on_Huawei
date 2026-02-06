/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2018. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef __VOICEAGENTPUBLIC_H__
#define __VOICEAGENTPUBLIC_H__

/*****************************************************************************
  1 Include Headfile
*****************************************************************************/

#include "vos.h"
#include "PsCommonDef.h"
#include "LPSCommon.h"
#include "mdrv.h"
#include "mdrv_udi.h"
#include "ps_tag.h"
#define THIS_MODU ps_modem_voice


/*****************************************************************************
  1.1 Cplusplus Announce
*****************************************************************************/
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
#pragma pack(*)    设置字节对齐方式
*****************************************************************************/
#if (VOS_OS_VER != VOS_WIN32)
#pragma pack(4)
#else
#pragma pack(push, 4)
#endif

/*****************************************************************************
  2 macro
*****************************************************************************/
#define VOICEAGENT_NULL                       (0)
#define VOICEAGENT_SUCC                       (0)
#define VOICEAGENT_FAIL                       (1)
#define VOICEAGENT_TRUE                       (1)
#define VOICEAGENT_FALSE                      (0)
/* IOCTL CMD 定义 */
#define VOICEAGENT_IOCTL_SET_WRITE_CB      0x7F001000
#define VOICEAGENT_IOCTL_SET_READ_CB       0x7F001001
#define VOICEAGENT_IOCTL_SET_EVT_CB        0x7F001002
#define VOICEAGENT_IOCTL_SET_FREE_CB       0x7F001003

#define VOICEAGENT_IOCTL_WRITE_ASYNC       0x7F001010
#define VOICEAGENT_IOCTL_GET_RD_BUFF       0x7F001011
#define VOICEAGENT_IOCTL_RETURN_BUFF       0x7F001012
#define VOICEAGENT_IOCTL_RELLOC_READ_BUFF  0x7F001013
#define VOICEAGENT_IOCTL_SEND_BUFF_CAN_DMA 0x7F001014

#define VOICEAGENT_IOCTL_IS_IMPORT_DONE    0x7F001020
#define VOICEAGENT_IOCTL_WRITE_DO_COPY     0x7F001021

#define VOICEAGENT_MODEM_IOCTL_SET_MSC_READ_CB 0x7F001030
#define VOICEAGENT_MODEM_IOCTL_MSC_WRITE_CMD   0x7F001031
#define VOICEAGENT_MODEM_IOCTL_SET_REL_IND_CB  0x7F001032

#define VOICEAGENT_INVALID_HANDLE     (-1)

#ifdef PS_ITT_PC_TEST
#define VOICEAGENT_INFO_LOG(String) \
                 PS_PRINTF_INFO(" %s\r\n",String)/*LPS_LOG(UE_MODULE_ESM_ID, VOS_NULL, PS_PRINT_INFO, String)
 */
#define VOICEAGENT_INFO_LOG1(String, Para1) \
                 PS_PRINTF_INFO(" %s %d\r\n",String, (long)Para1)/*LPS_LOG1(UE_MODULE_ESM_ID, VOS_NULL, PS_PRINT_INFO, String, (long)Para1 )
 */
#define VOICEAGENT_INFO_LOG2(String, Para1, Para2) \
                 PS_PRINTF_INFO(" %s %d %d\r\n",String, (long)Para1, (long)Para2)/*LPS_LOG2(UE_MODULE_ESM_ID, VOS_NULL, PS_PRINT_INFO, String, (long)Para1, (long)Para2)
 */
#define VOICEAGENT_WARN_LOG(String) \
                 PS_PRINTF_INFO(" %s\r\n",String)/*LPS_LOG(UE_MODULE_ESM_ID, VOS_NULL, PS_PRINT_WARNING, String)
 */
#define VOICEAGENT_WARN_LOG1(String, Para1) \
                     PS_PRINTF_INFO(" %s %d\r\n",String, (long)Para1)/*LPS_LOG(UE_MODULE_ESM_ID, VOS_NULL, PS_PRINT_WARNING, String, (long)Para1 )
 */
#define VOICEAGENT_WARN_LOG2(String, Para1, Para2) \
                         PS_PRINTF_INFO(" %s %d\r\n",String, (long)Para1, (long)Para2)/*LPS_LOG(UE_MODULE_ESM_ID, VOS_NULL, PS_PRINT_WARNING, String, (long)Para1, (long)Para2)
 */
#define VOICEAGENT_ERR_LOG(String) \
                 PS_PRINTF_INFO(" %s\r\n",String)/*LPS_LOG(UE_MODULE_ESM_ID, VOS_NULL, PS_PRINT_ERROR, String)
 */
#define VOICEAGENT_ERR_LOG1(String, Para1) \
                     PS_PRINTF_INFO(" %s %d\r\n",String, (long)Para1)/*LPS_LOG(UE_MODULE_ESM_ID, VOS_NULL, PS_PRINT_ERROR, String, (long)Para1 )
 */

#else
/*打印转层间打开*/
#if (!defined(_lint) && (PRINT_SWITCH == PRINT_OFF) && (VOS_WIN32 != VOS_OS_VER))
#define VOICEAGENT_INFO_LOG(String)
#define VOICEAGENT_INFO_LOG1(String, Para1)
#define VOICEAGENT_INFO_LOG2(String, Para1, Para2)
#define VOICEAGENT_WARN_LOG(String)
#define VOICEAGENT_WARN_LOG1(String, Para1)
#define VOICEAGENT_WARN_LOG2(String, Para1, Para2)
#define VOICEAGENT_ERR_LOG(String)
#define VOICEAGENT_ERR_LOG1(String, Para1)
#else
#define VOICEAGENT_INFO_LOG(String) \
                 PS_PRINTF_INFO(" %s\r\n",String)
#define VOICEAGENT_INFO_LOG1(String, Para1) \
                 PS_PRINTF_INFO(" %s %d\r\n",String, (long)Para1)
#define VOICEAGENT_INFO_LOG2(String, Para1, Para2) \
                 PS_PRINTF_INFO(" %s %d\r\n",String, (long)Para1, (long)Para2)
#define VOICEAGENT_WARN_LOG(String) \
                 PS_PRINTF_WARNING(" %s\r\n",String)
#define VOICEAGENT_WARN_LOG1(String, Para1) \
                 PS_PRINTF_WARNING(" %s %d\r\n",String, (long)Para1)
#define VOICEAGENT_WARN_LOG2(String, Para1, Para2) \
                 PS_PRINTF_WARNING(" %s %d\r\n",String, (long)Para1, (long)Para2)
#define VOICEAGENT_ERR_LOG(String) \
                 PS_PRINTF_ERR(" %s\r\n",String)
#define VOICEAGENT_ERR_LOG1(String, Para1) \
                 PS_PRINTF_ERR(" %s %d\r\n",String, (long)Para1)

#endif
#endif

/* 内存拷贝宏定义 */

#if (VOS_OS_VER == VOS_WIN32)

#define VOICEAGENT_MEM_CPY_S(pucDestBuffer, ulMaxDest, pucSrcBuffer, ulBufferLen) \
    NAS_LMM_SecuMemCpy((pucDestBuffer), (ulMaxDest), (pucSrcBuffer), (ulBufferLen), (__LINE__), (THIS_FILE_ID))

#define VOICEAGENT_MEM_SET_S(pucBuffer, ulMaxDest, ucData, ulBufferLen) \
    NAS_LMM_SecuMemSet((pucBuffer), (ulMaxDest), (ucData), (ulBufferLen), (__LINE__), (THIS_FILE_ID))

#define VOICEAGENT_MEM_MOVE_S(pucDestBuffer, ulMaxDest, pucSrcBuffer, ulBufferLen) \
    NAS_LMM_SecuMemMove( (pucDestBuffer), (ulMaxDest), (pucSrcBuffer), (ulBufferLen), (__LINE__), (THIS_FILE_ID))

#define VOICEAGENT_STRSTR(pucStr1, pucStr2) \
    VOS_StrStr((VOS_CHAR *)(pucStr1), (VOS_CHAR *)(pucStr2))

#else
#define VOICEAGENT_MEM_CPY_S(pDestBuffer, ulDestLen,  pSrcBuffer, ulCount) VOS_MemCpy_s( pDestBuffer, ulDestLen,  pSrcBuffer, ulCount)

#define VOICEAGENT_MEM_SET_S(pDestBuffer, ulDestLen, ucData, ulCount) VOS_MemSet_s( pDestBuffer, ulDestLen, (VOS_CHAR)(ucData), ulCount )

#define VOICEAGENT_MEM_MOVE_S(pDestBuffer, ulDestLen, pucSrcBuffer, ulCount) VOS_MemMove_s( pDestBuffer, ulDestLen, pucSrcBuffer, ulCount )

#endif


#define VOICEAGENT_MEM_ALLOC(ulPid , ulSize) PS_MEM_ALLOC(ulPid , ulSize)
#define VOICEAGENT_MEM_FREE(ulPid , pAddr)PS_MEM_FREE(ulPid, pAddr )

#define VOICEAGENT_MEM_CMP(pucDestBuffer, pucSrcBuffer, ulBufferLen)\
            PS_MEM_CMP ((pucDestBuffer), (pucSrcBuffer), (ulBufferLen))


/*****************************************************************************
  3 Massage Declare
*****************************************************************************/



/*****************************************************************************
  4 Enum
*****************************************************************************/
typedef VOS_INT32 VOICE_AGENT_HANDLE;
enum VOICE_AGENT_DEVICE_ID_ENUM
{
    /* MUST BE LAST */
    VOICE_AGENT_INVAL_DEV_ID = 0xFFFF
} ;

enum VOICE_AGENT_DEV_ENUM
{
    VOICE_AGENT_NV_DEV = 0,
    VOICE_AGENT_OM_DEV,
    VOICE_AGENT_DEV_BUTT
};

typedef VOS_UINT16 VOICE_AGENT_DEVICE_ID_ENUM_UINT16;
/*****************************************************************************
  5 STRUCT
*****************************************************************************/
typedef struct
{
    UDI_DEVICE_ID_E                         enUdiDevId;
    VOICE_AGENT_HANDLE                      slVoiceagentHdl;                 /* 设备对应的句柄 */
    VOS_UINT32                              *pNvVirAddr;
    VOS_UINT32                              *pNvPhyAddr;
    VOS_UINT32                              ulDataSize;
}VOICE_AGENT_DEV_INFO_STRU;

typedef struct
{
    VOICE_AGENT_DEVICE_ID_ENUM_UINT16       enUdiDevId;                           /* 设备ID */
    VOS_VOID                               *pPrivate;                            /* 模块特有的数据 */
} VOICE_AGENT_OPEN_PARAM_S;



typedef struct
{
    VOS_CHAR                            *pVirAddr;
    VOS_CHAR                            *pPhyAddr;
    VOS_UINT32                          u32Size;
    VOS_VOID                            * pDrvPriv;
}HIFI_WR_ASYNC_INFO_STRU;


typedef struct
{
    VOS_UINT32 u32BuffSize;
}HIFI_READ_BUFF_INFO;

/*****************************************************************************
  6 UNION
*****************************************************************************/


/*****************************************************************************
  7 Extern Global Variable
*****************************************************************************/
#if (FEATURE_ON == FEATURE_EXTRA_MODEM_MODE)
extern VOICE_AGENT_DEV_INFO_STRU                        g_stHifDevInfo[VOICE_AGENT_DEV_BUTT];
extern VOS_VOID VOICE_AGENT_PcieReadOmDataCB( VOS_VOID );
extern VOS_VOID VOICE_AGENT_PcieWriteNvDataCB( VOS_VOID );
extern VOS_INT32 VOICE_AGENT_PcieSetRead(VOS_UINT8 dev_index, HIFI_READ_BUFF_INFO* pReadBuff, VOS_VOID* pReadCB);
extern VOS_INT32 VOICE_AGENT_PcieSetWrite(VOS_UINT8 dev_index, VOS_UINT8* pstNvData, VOS_UINT32 len, VOS_VOID* pWriteCB);
extern VOS_INT32 VOICE_AGENT_PcieOpen(VOS_UINT8 dev_index);
#endif
/*****************************************************************************
  9 OTHERS
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

#endif /* end of VOICEAGENTPublic.h */





