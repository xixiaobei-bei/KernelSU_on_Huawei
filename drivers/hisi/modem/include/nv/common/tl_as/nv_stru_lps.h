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

#ifndef __NV_STRU_LPS_H__
#define __NV_STRU_LPS_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 Include Headfile
*****************************************************************************/
#include  "vos.h"
#include  "PsTypeDef.h"
#include  "nv_id_tlas.h"
#include  "nv_stru_rrc.h"
#include  "msp_nvim.h"
#if (VOS_OS_VER != VOS_WIN32) && (!defined NV_DEFINE)
#include  "msp.h"
#endif
#pragma pack(4)

/*****************************************************************************
  2 macro
*****************************************************************************/
#define MAX_GLOBAL_PRANT_NUM   5


#define LPS_NV_JP_DCOM_CON_TO_IDLE_BIT        ( 0x08000000 )

#define LPS_NV_JP_DCOM_REL_OFFSET_BIT        ( 0x10000000 )
#define LPS_NV_JP_DCOM_REL_OFFSET_DEBUG_BIT  ( 0x20000000 )

#define LPS_NV_JP_DCOM_FAST_DORM_BIT  ( 0x40000000 )

/*****************************************************************************
  3 Massage Declare
*****************************************************************************/


/*****************************************************************************
  4 Enum
*****************************************************************************/

/*****************************************************************************
  5 STRU
*****************************************************************************/

/*****************************************************************************
 结构名    : LPS_NV_LONG_BIT_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 协议栈的和协议功能相关的开关的结构体
*****************************************************************************/
typedef struct
{
    VOS_UINT32  bitFlag01                   :1;
    VOS_UINT32  bitFlag02                   :1;
    VOS_UINT32  bitFlag03                   :1;
    VOS_UINT32  bitFlag04                   :1;
    VOS_UINT32  bitFlag05                   :1;
    VOS_UINT32  bitFlag06                   :1;
    VOS_UINT32  bitFlag07                   :1;
    VOS_UINT32  bitFlag08                   :1;
    VOS_UINT32  bitFlag09                   :1;
    VOS_UINT32  bitFlag10                   :1;
    VOS_UINT32  bitFlag11                   :1;
    VOS_UINT32  bitFlag12                   :1;
    VOS_UINT32  bitFlag13                   :1;
    VOS_UINT32  bitFlag14                   :1;
    VOS_UINT32  bitFlag15                   :1;
    VOS_UINT32  bitFlag16                   :1;
    VOS_UINT32  bitFlag17                   :1;
    VOS_UINT32  bitFlag18                   :1;
    VOS_UINT32  bitFlag19                   :1;
    VOS_UINT32  bitFlag20                   :1;
    VOS_UINT32  bitFlag21                   :1;
    VOS_UINT32  bitFlag22                   :1;
    VOS_UINT32  bitFlag23                   :1;
    VOS_UINT32  bitFlag24                   :1;
    VOS_UINT32  bitFlag25                   :1;
    VOS_UINT32  bitFlag26                   :1;
    VOS_UINT32  bitFlag27                   :1;
    VOS_UINT32  bitFlag28                   :1;
    VOS_UINT32  bitFlag29                   :1;
    VOS_UINT32  bitFlag30                   :1;
    VOS_UINT32  bitFlag31                   :1;
    VOS_UINT32  bitFlag32                   :1;
}LPS_NV_LONG_BIT_STRU;


/*****************************************************************************
 结构名    : LPS_NV_FLOW_CONTROL
 协议表格  :
 ASN.1描述 :
 结构说明  : 协议栈流控结构体
*****************************************************************************/
typedef struct
{
    /* 流控总开关 */
    VOS_UINT32                          ulFlowCtrlFlag;
    /*ulFcInitFlag流控初始化选择 0-由RRC动态选择，1-代表由NV项进行控制*/
    VOS_UINT32                          ulFcInitFlag;
    VOS_UINT32                          ulFcPdcpTarget;
    VOS_UINT32                          ulTargetIPMax;
    VOS_UINT32                          ulSendSduMax;
    VOS_UINT32                          ulDlSduMax;
    VOS_UINT32                          ulULIpMax;
    VOS_UINT32                          ulMeasGap;
    VOS_UINT32                          ulSduGap;
}LPS_NV_FLOW_CONTROL;


/*****************************************************************************
 结构名    : LPS_NV_GLOBAL_PRINT
 协议表格  :
 ASN.1描述 :
 结构说明  : 协议栈打印全局变量结构体
*****************************************************************************/
typedef struct
{
    /* 打印总开关 0:关闭打印全局变量，1: 打印一个，2:打印两个，依次 3 4 5 */
    VOS_UINT32                          ulGlobalPrintFlag;
    VOS_UINT32                          aulPara[MAX_GLOBAL_PRANT_NUM];
}LPS_NV_GLOBAL_PRINT_STRU;


/*****************************************************************************
 结构名    : LPS_SWITCH_PARA_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 协议栈的和协议功能相关的开关的结构体
*****************************************************************************/
typedef struct
{
    /* 配置TA Timer超时SRS和PUCCH释放开关 0:关闭，1:打开，默认关闭 */
    VOS_UINT32                          ulCloseTaTimer;

    /* 配置SR触发方开关 0:关闭  1:打开，默认关闭 */
    VOS_UINT32                          ulSrTrigFlag;

    /* 配置SR触发随机接入开关 0:关闭，1:打开，默认关闭 */
    VOS_UINT32                          ulCloseSrRandFlag;

    /*同失步开关，0为关闭，1为打开。默认为关闭，设置为打开*/
    /*VOS_UINT32                          ulSyncCtrlFlag;*/

    /*连接态测量开关，0为关闭，1为打开。默认为关闭，设置为打开*/
    VOS_UINT32                          ulConnMeasFlg;

    /*设置测量IND消息频率，参数表示上报间隔: 取值方法要大于等于1*/
    VOS_UINT32                          ulMeasReportMaxNum;

    /*0:关闭抓包功能 1:打开抓包功能 默认是打开*/
    VOS_UINT32                          ulIfForwardToPc;

    /*打开安全加密开关，仅仅需要时设置该标志*/
    VOS_UINT32                          ulSmcControl;

    /*规避eNB的TDD的接入问题*/
    /*VOS_UINT32                          ulMsg4OnlyPadding;*/

    /*判断是否BAR小区开关*/
    VOS_UINT32                          ulCloseBarCell;

    /*DRX功能开关 0:关闭  1:打开*/
    VOS_UINT32                          ulDrxControlFlag;

   /*外场测试桩代码开关 0:关闭  1:打开.默认为关闭*/
    VOS_UINT32                          ulFieldTestSwitch;

    /* PCO功能开关，0为关闭，1为打开。默认为打开 */
    VOS_UINT32                          ulPcoFlag;

    /*流控结构*/
    LPS_NV_FLOW_CONTROL                  stPsNvFlowCtl;

    /************************stPsGcfFlag01各个BIT含义***************************
     bitFlag01:表示GCF测试场景TAU消息是否加密，平台缺省值为1。
               0: 加密；1:不加密
     bitFlag02:表示GCF测试场景msg4解析开关，平台缺省值为1。
               0: 表示按协议实现; 1:表示按规避方案实现
     bitFlag03:表示GCF测试场景是否支持24.301 Table D.1.1中建链原因设置，平台缺省值为1。
               0: 支持; 1:不支持
     bitFlag04:表示GCF测试场景指示安全功能是否支持测试模式，测试模式支持空完整性算法，平台缺省值为1。
               0: 支持; 1:不支持
    ****************************************************************************/
    LPS_NV_LONG_BIT_STRU                 stPsGcfFlag01;

    /************************stPsGcfFlag02各个BIT含义***************************
     stPsGcfFlag01各个BIT含义:
     bitFlag01:
     bitFlag02:
    ****************************************************************************/
    LPS_NV_LONG_BIT_STRU                 stPsGcfFlag02;

    /************************stPsFunFlag01各个BIT含义***************************
     bitFlag01: Band64是否支持，缺省值:0; 0:不支持;1:支持
     bitFlag02: 删除SDF时是否携带Qos信息,缺省值:0; 0:携带;1:不携带
     bitFlag03: 是否进行连接态收到本ue的业务寻呼后进行释放,缺省值:0; 0:释放;1:不释放
     bitFlag04: 校验安全功能开关,缺省值:0; 0:打开;1:关闭
     bitFlag05:
     bitFlag06:
     bitFlag07: 拨号功能合入之前，装备测试时上行DRB数据在PDCP入口丢掉,0为关闭，1为打开。默认为关闭
     bitFlag08: Print时延优化开关，缺省值:0; 0:关;1:开;但是RRM测试时时自动生效的
     bitFlag09: 层间消息时延优化开关，缺省值:0; 0:关;1:开;
     bitFlag10: MBMS的支持开关，缺省值:0; 0:关;1:开;
     bitFlag11: SBM定制需求开关，特定band支持特定的带宽.缺省值:0; 0:关;1:开;
     bitFlag12: 检查整个小区的带宽是都在协议频段内，缺省值:1; 0:关;1:开;
     bitFlag13: 测试部需求,模拟切换失败用例，缺省值:1; 0:关;1:开;
     bitFlag14: DCM定制需求 GU到L的切换NAS不判断被禁TA;缺省值:0; 0:关;1:开;
     bitFlag15: 是否支持ETWS功能;缺省值:1; 0:关;1:开;
     bitFlag16: 是否支持CMAS功能;缺省值:1; 0:关;1:开;
     bitFlag17:
     bitFlag18: L重建时是否搜索UTRAN;缺省值:1; 0:关;1:开;
     bitFlag19: L重建时是否搜索GSM;缺省值:1; 0:关;1:开;
     bitFlag20: 部分接收到系统信息时,如果小区RSRP门限小于设定值，用于判断否需要bar小区，;缺省值:0; 0:关;1:开;
     bitFlag21: NCC从左向右还是从右向左，目前按照协议实现，缺省值:0; 0表示按照协议，1表示不按照协议
     bitFlag22: 是否支持背景搜索,缺省值:0;1:关，0:开；
     bitFlag23: 重建时需要根据GAP是否有配置重新配置一下物理层，缺省值:1; 1:开，0:关
     bitFlag24:
     bitFlag25:
     bitFlag26: IDLE下重选前不判断NCC，缺省值:0; 0表示需要判断；1表示不判断。
     bitFlag27: V7R1日本DCOM需求，缺省值:0; 0:关;1:开;
     bitFlag28: V7R1日本DCOM需求，缺省值:0; 0:关;1:开;
     bitFlag29: V7R1日本DCOM需求，缺省值:0; 0:关;1:开;
     bitFlag30: V7R1日本DCOM需求，缺省值:0; 0:关;1:开;
     bitFlag31: V7R1日本DCOM需求，缺省值:0; 0:关;1:开;
     bitFlag32: RRM测试时，缺省关闭打印是否生效，缺省值:0; 1:不生效；0:生效。
    ****************************************************************************/
    LPS_NV_LONG_BIT_STRU                 stPsFunFlag01;

    /************************stPsFunFlag02各个BIT含义***************************
     bitFlag01: CA特性开关:缺省值:0; 0:开;1:关:;
     bitFlag02:
     bitFlag03:
     bitFlag04: CMAS在驻留之前还是驻留之后上报，缺省值:0;0: 驻留之后上报; 1:驻留前后都可以上报
     bitFlag05:
     bitFlag06:
     bitFlag07: 背景搜，只搜L单模.不搜GU模开关.缺省值:0; 0:关;1:开;
     bitFlag08: ETWS发生时机; 1:立即发送，0:收齐后再发送
     bitFlag09: TL L2关键事件上报-缺省值:0;0:关;1:开
     bitFlag10: LTE L2 数传信息上报-缺省值:0;0:关;1:开
     bitFlag11: LIST搜网先验频点是否搜索2次。0:关;1:开
     bitFlag12: 网络指定模式重定向结束后是否继续发起其他模式的重定向流程; 0:发起网络指定模式的重定向之后继续发起其他模式的重定向;1:只发起网络指定模式的重定向
     bitFlag13: 是否支持安全上下文激活时处理明文的NAS拒绝消息。缺省值: 0; 0: 不支持 1: 支持
     bitFlag14: LTE CA小区状态信息上报-缺省值:0;0:关;1:开
     bitFlag15: 为解决网络BUG导致的CSFB流程FR回到L时概率收不到paging的问题，设置的接入层发起的异系统变换到L下发起TAU类型总是type2类型开关。缺省值0.0: 关闭 1:打开
     bitFlag16: LTE模内切换NAS忽略被禁TA,缺省值:0; 0:关; 1:开
     bitFlag17: ESR过程中发生切换,未发起TAU流程,重发ESR消息控制开关,缺省值:0; 0:不重传 1:重传
     bitFlag18: csfb场景收不到网侧release场景优化开关，缺省值:0;0:关;1:开
     bitFlag19: LRRC检测异常主动上报CHR功能开关,缺省值:1; 0:关闭;1:打开
     bitFlag20: 单双卡切换UE能力动态获取开关,0:关;1:开;chicago平台默认开,其他平台默认关闭
     bitFlag21: UE不支持CA时,如果仪表强配CA,UE是否回复RECFG_CMP开关;缺省值:0;  0:不回复CMP; 1:回复CMP
     bitFlag22: 台湾CMAS临时方案开关; 缺省值:0;  0:关闭; 1:打开;
     bitFlag23:
     bitFlag24:
     bitFlag25:
     bitFlag26:
     bitFlag27:
     bitFlag28:
     bitFlag29:
     bitFlag30:
     bitFlag31:
     bitFlag32:
    ****************************************************************************/
    LPS_NV_LONG_BIT_STRU                 stPsFunFlag02;

    
    LPS_NV_LONG_BIT_STRU                 stPsFunFlag03;

   /************************stPsFunFlag04各个BIT含义***************************
     bitFlag01: 检测LTE网侧寻呼情况 若打开,IDLE态不处理本UE Paging。默认为0.
                0:功能关闭  1:功能打开
     bitFlag02:
     bitFlag03:
     bitFlag04:
     bitFlag05:
     bitFlag06: 控制L2 MAC随机接入时对于高速小区是否启动小区半径外接入优化，1表示启动,默认关闭，由产品线定制
     bitFlag07: 控制L2 MAC随机接入时对于Preamble差2是否启动小区半径外接入优化,1表示启动，默认关闭，由产品线定制
     bitFlag08:
     bitFlag09:
     bitFlag10:
     bitFlag11:
     bitFlag12:
     bitFlag13:
     bitFlag14:
     bitFlag15:
     bitFlag16:
     bitFlag17:
     bitFlag18:
     bitFlag19:
     bitFlag20:
     bitFlag21:
     bitFlag22:
     bitFlag23:
     bitFlag24:
     bitFlag25:
     bitFlag26:
     bitFlag27:
     bitFlag28:
     bitFlag29:
     bitFlag30:
     bitFlag31:
     bitFlag32:
    ****************************************************************************/
    LPS_NV_LONG_BIT_STRU                 stPsFunFlag04;

    LPS_NV_GLOBAL_PRINT_STRU             stPsNvGlobalPrint;
}LPS_SWITCH_PARA_STRU;


/*****************************************************************************
 结构名    : RRC_UE_EUTRA_CAP_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : UE EUTRA能力信息
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          bitFeatureGroupIndsPresent    : 1;
    VOS_UINT32                          bitUeCapV920IesPresent        : 1;
    VOS_UINT32                          bitSpare                      : 30;
    VOS_UINT8                           aucReserved[2];
    /* 用来设置UE接入层版本，1：支持，0：不支持，从低到高位分别用来指示Rel8,Rel9,.... */
    VOS_UINT8                           ucAsRelBitmap;
    RRC_UE_CATEGORY_ENUM_UINT8          enUeCatg;
    RRC_UE_CAP_PDCP_PARA_STRU           stPdcpPara;
    RRC_UE_CAP_PHY_PARA_STRU            stPhyPara;
    RRC_UE_CAP_RF_PARA_STRU             stRfPara;
    RRC_UE_CAP_MEAS_PARA_STRU           stMeasPara;
    RRC_UE_CAP_FEATURE_GROUP_IND_STRU   stFeatrueGroupInds;
    RRC_UE_CAP_INTER_RAT_PARA_STRU      stInterRatPara;
    RRC_UE_EUTRA_CAP_V920_IES_STRU      stUeCapV920Ies;
} LRRC_NV_UE_EUTRA_CAP_STRU;


/*****************************************************************************
 结构名    : 开机log功能是否打开

 结构说明  : cPowerOnlogC和cPowerOnlogA都为1，此功能才生效
*****************************************************************************/
typedef struct
{
    /* C核的开机log是否打开*/
    VOS_INT8                            cPowerOnlogC;

    /* AP的开机log是否打开,默认为0。在A核linux内核初始化时根据SOCP的50M是否存在修改此NV的值:存在，此值会被修改为1；否则，保持0不变 */
    VOS_INT8                            cPowerOnlogA;
    VOS_INT8                            cSpare1;
    VOS_INT8                            cSpare2;
}NV_POWER_ON_LOG_SWITCH_STRU;


/*****************************************************************************
  6 UNION
*****************************************************************************/


/*****************************************************************************
  7 Extern Global Variable
*****************************************************************************/

/*****************************************************************************
  8 Fuction Extern
*****************************************************************************/


/*****************************************************************************
  9 OTHERS
*****************************************************************************/










#if (VOS_OS_VER == VOS_WIN32)
#pragma pack()
#else
#pragma pack(0)
#endif




#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of PsNvInterface.h */

