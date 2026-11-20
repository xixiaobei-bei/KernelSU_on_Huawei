/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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


#ifndef __OM_COMMON_PPM_H__
#define __OM_COMMON_PPM_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include <mdrv.h>
#include <mdrv_diag_system.h>
#include <osl_types.h>
#include <bsp_dump.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#pragma pack(push)
#pragma pack(4)

extern u32                          g_ulOmAcpuDbgFlag ;

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define USB_MAX_DATA_LEN            (60*1024)   /*USB发送的最大数据长度*/

#define BIT_N(num)          (0x01 << (num))

#define OM_ACPU_RECV_USB        BIT_N(0)
#define OM_ACPU_DISPATCH_MSG    BIT_N(1)
#define OM_ACPU_SEND_SOCP       BIT_N(2)
#define OM_ACPU_SEND_USB        BIT_N(3)
#define OM_ACPU_USB_CB          BIT_N(4)
#define OM_ACPU_SEND_USB_IND    BIT_N(5)
#define OM_ACPU_ERRLOG_SEND     BIT_N(6)
#define OM_ACPU_ERRLOG_RCV      BIT_N(7)
#define OM_ACPU_ERRLOG_PRINT    BIT_N(8)
#define OM_ACPU_RECV_SOCKET     BIT_N(9)
#define OM_ACPU_SEND_SOCKET     BIT_N(10)
#define OM_ACPU_DATA            BIT_N(11)
#define OM_ACPU_READ_DONE       BIT_N(12)

#define ppm_printf  diag_system_printf


#define OM_ACPU_DEBUG_CHANNEL_TRACE(enChanID, pucData, ulDataLen, ulSwitch, ulDataSwitch) \
        if(false != (g_ulOmAcpuDbgFlag&ulSwitch)) \
        { \
            (void)ppm_printf("%s, channal ID: = %d, Data Len: = %d\n", __FUNCTION__, enChanID, ulDataLen); \
            if(false != (g_ulOmAcpuDbgFlag&ulDataSwitch) )\
            {\
                if (NULL != pucData)\
                {\
                    u32 ulOmDbgIndex;\
                    for (ulOmDbgIndex = 0 ; ulOmDbgIndex < ulDataLen; ulOmDbgIndex++) \
                    { \
                        bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_DIAG_SYSTEM, "%02x ", *((u8*)pucData + ulOmDbgIndex));\
                    } \
                }\
            }\
        }


/*物理通道类型枚举*/
enum
{
    CPM_OM_PORT_TYPE_USB    = 0,
    CPM_OM_PORT_TYPE_VCOM   = 1,
    CPM_OM_PORT_TYPE_WIFI   = 2,
    CPM_OM_PORT_TYPE_SD     = 3,
    CPM_OM_PORT_TYPE_FS     = 4,
    CPM_OM_PORT_TYPE_HSIC   = 5,
    CBP_OM_PORT_TYPE_BUTT
};

/*******************************************************************************
  3 枚举定义
*******************************************************************************/
/* UDI设备句柄 */
enum OM_PROT_HANDLE_ENUM
{
    OM_USB_IND_PORT_HANDLE      =   0,
    OM_USB_CFG_PORT_HANDLE      =   1,
    OM_USB_CBT_PORT_HANDLE      =   2,
    OM_HSIC_IND_PORT_HANDLE     =   3,
    OM_HSIC_CFG_PORT_HANDLE     =   4,
    OM_PORT_HANDLE_BUTT             /*OM_PORT_HANDLE_NUM = OM_PORT_HANDLE_BUTT，如果更改此枚举注意更改OM_PORT_HANDLE_NUM*/
};

typedef u32  OM_PROT_HANDLE_ENUM_UINT32;

enum OM_LOGIC_CHANNEL_ENUM
{
     OM_LOGIC_CHANNEL_CBT = 0,
     OM_LOGIC_CHANNEL_CNF,
     OM_LOGIC_CHANNEL_IND,
     OM_LOGIC_CHANNEL_BUTT
};

typedef u32     OM_LOGIC_CHANNEL_ENUM_UINT32;


/*****************************************************************************
  4 结构体定义
*****************************************************************************/

typedef struct
{
    u8                          *pucAsyncCBData;      /* DRV_UDI_IOCTL接口异步返回后传入的数据指针 */
    u32                          ulLen;               /* DRV_UDI_IOCTL接口异步返回后返回的实际处理数据长度 */
    u32                          ulRsv;               /* Reserve */
}OM_PSEUDO_SYNC_STRU;


typedef struct
{
    u32                          ulPortTypeErr;
    u32                          ulSwitchFail;
    u32                          ulSwitchSucc;
    u32                          ulStartSlice;
    u32                          ulEndSlice;
}PPM_PORT_CFG_INFO_STRU;

/*****************************************************************************
 枚举名    : AT_PHY_PORT_ENUM
 枚举说明  : 物理端口号枚举值 从omnvinterface.h拷贝来的
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
typedef u32  AT_PHY_PORT_ENUM_UINT32;

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
};//物理端口枚举
typedef u32  CPM_PHY_PORT_ENUM_UINT32;


/*****************************************************************************
  4 函数声明
*****************************************************************************/
extern u32 PPM_DisconnectTLPort(void);

extern void   PPM_DisconnectAllPort(OM_LOGIC_CHANNEL_ENUM_UINT32 enChannel);

extern u32 PPM_ReadPortData(CPM_PHY_PORT_ENUM_UINT32 enPhyPort, UDI_HANDLE UdiHandle, OM_PROT_HANDLE_ENUM_UINT32 enHandle);


extern void   PPM_PortWriteAsyCB(OM_PROT_HANDLE_ENUM_UINT32 enHandle, u8* pucData, s32 lLen);

extern void   PPM_ReadPortDataInit(CPM_PHY_PORT_ENUM_UINT32        enPhyPort,
                                    OM_PROT_HANDLE_ENUM_UINT32          enHandle,
                                    void                            *pReadCB,
                                    void                            *pWriteCB,
                                    void                            *pStateCB);

extern u32 PPM_UdiRegCallBackFun(UDI_HANDLE enHandle, u32 ulCmdType, void* pFunc);

extern void   PPM_PortCloseProc(OM_PROT_HANDLE_ENUM_UINT32  enHandle, CPM_PHY_PORT_ENUM_UINT32 enPhyPort);

extern u32 PPM_PortSend(OM_PROT_HANDLE_ENUM_UINT32 enHandle, u8 *pucVirAddr, u8 *pucPhyAddr, u32 ulDataLen);

extern void   PPM_PortStatus(OM_PROT_HANDLE_ENUM_UINT32 enHandle, CPM_PHY_PORT_ENUM_UINT32 enPhyPort,ACM_EVT_E enPortState);

extern void   PPM_GetSendDataLen(SOCP_CODER_DST_ENUM_U32 enChanID, u32 ulDataLen, u32 *pulSendDataLen, CPM_PHY_PORT_ENUM_UINT32 *penPhyport);

extern u32 PPM_PortInit(void);

extern int PPM_InitPhyPort(void);

extern u32 PPM_UsbPortInit(void);

extern void   PPM_HsicPortInit(void);

extern void   PPM_VComPortInit(void);
void PPM_RegDisconnectCb(PPM_DisconnectTLPortFuc cb);
u32 PPM_LogPortSwitch(u32  ulPhyPort, bool ulEffect);
void PPM_OmPortInfoShow(OM_PROT_HANDLE_ENUM_UINT32  enHandle);
void PPM_OmPortDebugInfoShow(void);
void PPM_PortSwitchInfoShow(void);

extern OM_PSEUDO_SYNC_STRU * PPM_ComPpmGetSyncInfo(void);
extern OM_ACPU_DEBUG_INFO * PPM_ComPpmGetDebugInfo(void);

/*****************************************************************************
  5 全局变量声明
*****************************************************************************/
extern UDI_HANDLE                              g_astOMPortUDIHandle[OM_PORT_HANDLE_BUTT];

/* USB承载的OM IND端口中，伪造为同步接口使用的数据结构体 */
extern OM_PSEUDO_SYNC_STRU                     g_stUsbIndPseudoSync;

/* USB承载的OM CNF端口中，伪造为同步接口使用的数据结构体 */
extern OM_PSEUDO_SYNC_STRU                     g_stUsbCfgPseudoSync;

/*****************************************************************************
  6 OTHERS定义
*****************************************************************************/




#pragma pack(pop)

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of OmCommonPpm.h*/

