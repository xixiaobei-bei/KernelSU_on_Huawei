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


/*
约束条件:

1.MSP接收到工具侧下发命令后一次性将所有通道信息和地址信息发送给TL-PHY和GU-PHY,
没有用到的通道信息用0表示，需要物理层去筛选可用的通道信息；

2.MSP发送的通道信息和地址信息中同时包含TL-PHY和GU-PHY需要的信息，需要工具侧和TL-PHY、
GU-PHY做好信息维护，MSP不做差别发送；

3.MSP发送地址为0表示内存没有分配，不允许使用
*/

#ifndef __DIAG_BBP_DS_H__
#define __DIAG_BBP_DS_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include "product_config.h"

#pragma pack(4)

/*****************************************************************************
  1 Include Headfile
*****************************************************************************/

/*****************************************************************************
  2 macro
*****************************************************************************/
#ifdef DIAG_SYSTEM_5G
#define DRX_SAMPLE_BBP_DATA_CHNNUM          15
#define DDR_SAMPLE_BASE_ADDR_MAX            10
#else
#define DRX_SAMPLE_BBP_DATA_CHNNUM          10
#define DDR_SAMPLE_BASE_ADDR_MAX            5
#endif



/* MSP与TL-PHY的BBP消息 */
#define MSG_ID_DIAG2TLPHY_CHAN_ADDR_INFO            (0x30002404)
#define ID_MSG_DIAG_TL_PHY_BBP_GEN_REQ              (0x30002403)
#define ID_MSG_DIAG_TL_PHY_BBP_GEN_CNF              (ID_MSG_DIAG_TL_PHY_BBP_GEN_REQ)
/* MSP与GU-PHY的BBP消息 */
#define MSG_ID_DIAG2GUPHY_CHAN_ADDR_INFO            (0x3200fe2d)
#define ID_MSG_DIAG_GU_PHY_BBP_GEN_REQ              (0x3200fe1f)
#define ID_MSG_DIAG_GU_PHY_BBP_GEN_CNF              (0x3200ff07) /*根据GUC PHY的需求，REQ与CNF不同，方便定位问题*/
/* MSP与HL1C的BBP消息 */
#define MSG_ID_DIAG2HL1C_CHAN_ADDR_INFO             (0x3f00b002)
#define ID_MSG_DIAG_HL1C_BBP_GEN_REQ                (0x3f00b001)
#define ID_MSG_DIAG_HL1C_BBP_GEN_CNF                (ID_MSG_DIAG_HL1C_BBP_GEN_REQ)
/* MSP与EasyRF的BBP消息 */
#define MSG_ID_DIAG2EASYRF_CHAN_ADDR_INFO           (0xaf00f804)
#define ID_MSG_DIAG_EASYRF_BBP_GEN_REQ              (0xaf38f800)
#define ID_MSG_DIAG_EASYRF_BBP_GEN_CNF              (ID_MSG_DIAG_EASYRF_BBP_GEN_REQ)

#define GUPHY_MSG_HEADER    unsigned int ulSenderCpuId;  \
                            unsigned int ulSenderPid;    \
                            unsigned int ulReceiverCpuId;\
                            unsigned int ulReceiverPid;  \
                            unsigned int ulLength;

#define SOCP_BBP_BUS_CHAN                            16   /* SOCP的bus采数通道，PHY调用修改指针接口时作为入参传入 */

/* EasyRF BBA_DBG提供给SOCP的读指针镜像地址 */
#define RFIC0_RPTR_IMG_ADDR                         (0xe19841b4)
#define RFIC1_RPTR_IMG_ADDR                         (0xe1984204)

/*****************************************************************************
  3 Enum
*****************************************************************************/
/*****************************************************************************
描述 : 获取SOCP\BBP DMA基地址等
        此接口是与工具间的接口，不是实际的BBP通道定义
ID  : DIAG_CMD_DRX_SAMPLE_REG_WR
REQ : DIAG_CMD_DRX_SAMPLE_GET_ADDR_REQ_STRU
CNF : DIAG_CMD_DRX_SAMPLE_GET_ADDR_CNF_STRU
IND : DIAG_CMD_DRX_REG_WR_IND_STRU
*****************************************************************************/
 typedef enum
{
                                            /* function   Austin  Dallas/V7R5/V722  Boston */
    DRX_SAMPLE_BBP_DMA_LOG0_CHNSIZE = 0x00, /* BUS        chn 16      chn 16        chn 16 */
    DRX_SAMPLE_BBP_DMA_LOG1_CHNSIZE ,       /* LOG        chn 17      chn 17          NA   */
    DRX_SAMPLE_BBP_DMA_LOG2_CHNSIZE ,       /* LOG1       chn 18        NA            NA   */
    DRX_SAMPLE_BBP_DMA_LOG3_CHNSIZE ,       /* LOG2       chn 19        NA            NA   */
    DRX_SAMPLE_BBP_DMA_LOG4_CHNSIZE ,       /* LOG3       chn 20        NA            NA   */
    DRX_SAMPLE_BBP_DMA_LOG5_CHNSIZE ,       /* LOG4       chn 21        NA            NA   */
    DRX_SAMPLE_BBP_DMA_LOG6_CHNSIZE ,       /* LOG5       chn 22        NA            NA   */
    DRX_SAMPLE_BBP_DMA_LOG7_CHNSIZE ,       /* LOG6       chn 23        NA            NA   */
    DRX_SAMPLE_BBP_DMA_DATA_CHNSIZE ,       /* DATA       chn 24      chn 24        chn 17 */
    DRX_SAMPLE_BBP_CDMA_DATA_CHNSIZE,
#ifdef DIAG_SYSTEM_5G
    DRX_SAMPLE_BBP_RFIC_0_BUS_CHNSIZE, //RFIC 0 BUS CHAN
    DRX_SAMPLE_BBP_RFIC_1_BUS_CHNSIZE, //RFIC 1 BUS CHAN
    DRX_SAMPLE_BBA_DATA_CHNSIZE,       //ACCESS DATA CHAN
    DRX_SAMPLE_BBP_5G_BUS_CHNSIZE,     //5G BUS CHAN
    DRX_SAMPLE_BBP_5G_DATA_CHNSIZE,    //5G DATA CHAN

#endif
}DIAG_CMD_DRX_SAMPLE_CHNSIZE_E;

/*****************************************************************************
描述 : 获取SOCP\BBP DMA基地址等
ID   : DIAG_CMD_DRX_SAMPLE_REG_WR
REQ : DIAG_CMD_DRX_SAMPLE_GET_ADDR_REQ_STRU
CNF : DIAG_CMD_DRX_SAMPLE_GET_ADDR_CNF_STRU
IND : DIAG_CMD_DRX_REG_WR_IND_STRU
*****************************************************************************/
typedef enum
{
	DRX_SAMPLE_BBP_DMA_BASE_ADDR = 0x00,
	DRX_SAMPLE_BBP_DBG_BASE_ADDR ,
	DRX_SAMPLE_BBP_SRC_BASE_ADDR ,
	DRX_SAMPLE_POW_ONOFF_CLK_BASE_ADDR ,
	DRX_SAMPLE_SOCP_BASE_ADDR,
#ifdef DIAG_SYSTEM_5G  /*5G上PHY需要获取其他的地址信息*/
	DRX_SAMPLE_BBA_DBG_CTRL_BASE_ADDR = 0x05,
	DRX_SAMPLE_BBP_RFIC0_BASE_ADDR,
	DRX_SAMPLE_BBP_RFIC1_BASE_ADDR,
	DRX_SAMPLE_5G_BBP_DMA_BASE_ADDR,
	DRX_SAMPLE_5G_BBP_DBG_BASE_ADDR
#endif
}DIAG_CMD_DRX_SAMPLE_ADDR_TYPE_E;

/*****************************************************************************
   5 STRUCT
*****************************************************************************/
typedef struct
{
	DIAG_CMD_DRX_SAMPLE_CHNSIZE_E eDiagDrxSampleChnSize;
}DIAG_CMD_DRX_SAMPLE_GET_CHNSIZE_REQ_STRU;

typedef struct
{
	DIAG_CMD_DRX_SAMPLE_ADDR_TYPE_E eDiagDrxSampleAddr;
}DIAG_CMD_DRX_SAMPLE_GET_ADDR_REQ_STRU;

typedef struct
{
    DIAG_CMD_DRX_SAMPLE_CHNSIZE_E   ulChanEnum;
    unsigned int                      ulChan;
    unsigned int                      ulAddr;
    unsigned int                      ulSize;
}DIAG_DRX_SAMPLE_CHNSIZE_S;

typedef struct
{
    unsigned int   ulDrxSampleType;
    unsigned int   ulDrxSampleAddr;
    unsigned int   ulResv;
} DIAG_CMD_DRX_SAMPLE_BASE_ADDR_STRU;

typedef struct
{
    unsigned int 		ulChanEnum;
    unsigned long long 	ulAddr;
    unsigned int 		ulSize;
    unsigned int 		ulResv;
}DIAG_PHY_BBPDS_CHN_INFO_STRU;

typedef struct
{
    unsigned int                ulChanNum;
    DIAG_CMD_DRX_SAMPLE_BASE_ADDR_STRU AddrInfo[DDR_SAMPLE_BASE_ADDR_MAX];
#ifndef MS_VC6_PLATFORM
	DIAG_PHY_BBPDS_CHN_INFO_STRU ChanInfo[0];   /*lint !e43 */
#else
    DIAG_PHY_BBPDS_CHN_INFO_STRU ChanInfo[1];   /*lint !e43 */
#endif
}DIAG_PHY_DSINFO_STRU;
/* 通知GUPHY、UPHY接口*/
typedef struct
{
     GUPHY_MSG_HEADER            /*VOS头 */
     unsigned int                 ulMsgId;
     unsigned int                ulMsgLen;
     DIAG_PHY_DSINFO_STRU      stDsInfo;
}DIAG_GUPHY_DSINFO_STRU;

/* 通知TLPHY接口*/
typedef struct
{
     unsigned int              ulMsgId;
     DIAG_PHY_DSINFO_STRU      stDsInfo;
}DIAG_TLPHY_DSINFO_STRU;

/*PHY的采数命令回复结构体*/
typedef struct
{
    unsigned int ulChanType;
    unsigned int ulOpsId;
    unsigned int ulResult;
}DIAG_BBP_PHY_CNF_INFO_STRU;

/*HL1C的采数命令回复结构体*/
typedef struct
{
    unsigned int ulChanType;
    unsigned int ulOpsId;
    unsigned int ulResult;
}DIAG_BBP_HL1C_CNF_INFO_STRU;

/*根据GU-PHY的需求，在公共头文件中添加该结构体，用于GU-PHY对MSP发送的start/stop命令进行回复*/
typedef struct
{
    GUPHY_MSG_HEADER
    unsigned int               ulMsgId;
    unsigned int               ulLen;
    DIAG_BBP_PHY_CNF_INFO_STRU stBbpCnfInfo;
}DIAG_MSP_BBP_MSG_STRU;

enum DIAG_BBP_CMD_CNF_INFO_ENUM
{
    DIAG_PHY_BBP_CNF_SUCCESS,
    DIAG_PHY_BBP_CNF_FAIL,
    DIAG_PHY_BBP_CNF_TIMEOUT,
    DIAG_PHY_BBP_CNF_DATA_BUF_USED,
};
//typedef unsigned int DIAG_BBP_CMD_CNF_INFO_ENUM;


#pragma pack()

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of diag_bbp_ds.h */




