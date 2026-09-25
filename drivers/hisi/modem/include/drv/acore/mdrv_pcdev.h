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
#ifndef _MDRV_PCDEV_H_
#define _MDRV_PCDEV_H_

#ifdef _cplusplus
extern "C"
{
#endif

#include <linux/dma-mapping.h>

#define MAX_DESC_NUM 4

/* IOCTL CMD 定义 */
#define PCDEV_IOCTL_SET_WRITE_CB      0x7F001000
#define PCDEV_IOCTL_SET_READ_CB       0x7F001001
#define PCDEV_IOCTL_SET_EVT_CB        0x7F001002
#define PCDEV_IOCTL_SET_FREE_CB       0x7F001003
#define PCDEV_IOCTL_SET_WRITE_INFO_CB 0x7F001004
#define PCDEV_IOCTL_SEND_EVT          0x7F001005
#define PCDEV_IOCTL_SEND_MSC_READ     0x7F001006
#define PCDEV_IOCTL_SEND_REL_IND      0x7F001007

#define PCDEV_IOCTL_WRITE_ASYNC       0x7F001010
#define PCDEV_IOCTL_GET_RD_BUFF       0x7F001011
#define PCDEV_IOCTL_RETURN_BUFF       0x7F001012
#define PCDEV_IOCTL_RELLOC_READ_BUFF  0x7F001013
#define PCDEV_IOCTL_SEND_BUFF_CAN_DMA 0x7F001014
#define PCDEV_IOCTL_SET_READ_BUFF     0x7F001015

#define PCDEV_IOCTL_IS_IMPORT_DONE    0x7F001020
#define PCDEV_IOCTL_WRITE_DO_COPY     0x7F001021

#define PCDEV_MODEM_IOCTL_SET_MSC_READ_CB 0x7F001030
#define PCDEV_MODEM_IOCTL_MSC_WRITE_CMD   0x7F001031
#define PCDEV_MODEM_IOCTL_SET_REL_IND_CB  0x7F001032
#define PCDEV_MODEM_IOCTL_SET_MSC_WRITE_CB  0x7F001033

#define PCDEV_MODEM_IOCTL_SET_DIAG_STATUS  0x7F001040

typedef struct tagPCDEV_WR_ASYNC_INFO
{
    char *pVirAddr;
    char *pPhyAddr;
    unsigned int u32Size;
    void* pDrvPriv;
}PCDEV_WR_ASYNC_INFO;


typedef struct tagPCDEV_WRITE_INFO
{
    char *pVirAddr;
    char *pPhyAddr;
    unsigned int u32Size;
    unsigned int submit_time;
    unsigned int complete_time;
    unsigned int done_time;
}PCDEV_WRITE_INFO;

typedef struct tagPCDEV_READ_BUFF_INFO
{
    unsigned int u32BuffSize;
}PCDEV_READ_BUFF_INFO;

typedef struct tagPCDEV_READ_BUFF_SET_INFO
{
    dma_addr_t u32BuffBase[MAX_DESC_NUM];
    unsigned long long virBuffBase[MAX_DESC_NUM];
    unsigned int u32BuffSize;
    unsigned int u32BuffNum;
}PCDEV_READ_BUFF_SET_INFO;


typedef enum tagPCDEV_EVT_E
{
    PCDEV_EVT_DEV_SUSPEND = 0,
    PCDEV_EVT_DEV_READY = 1,
    PCDEV_EVT_DEV_BOTTOM
}PCDEV_EVT_E;

typedef enum tagPCDEV_DIAG_MODE_E
{
    PCDEV_DIAG_DISCONNECT = 0,
    PCDEV_DIAG_CONNECT = 1,
    PCDEV_DIAG_BOTTOM
}PCDEV_DIAG_MODE_E;

/* ACM MODEM 管角信号结构体定义*/
typedef struct tagPCDEV_MDM_MSC_STRU
{
    unsigned int OP_Dtr     :    1;          /*DTR CHANGE FLAG*/
    unsigned int OP_Dsr     :    1;          /*DSR CHANGE FLAG*/
    unsigned int OP_Cts     :    1;          /*CTSCHANGE FLAG*/
    unsigned int OP_Rts     :    1;          /*RTS CHANGE FLAG*/
    unsigned int OP_Ri      :    1;          /*RI CHANGE FLAG*/
    unsigned int OP_Dcd     :    1;          /*DCD CHANGE FLAG*/
    unsigned int OP_Fc      :    1;          /*FC CHANGE FLAG*/
    unsigned int OP_Brk     :    1;          /*BRK CHANGE FLAG*/
    unsigned int OP_Spare   :    24;         /*reserve*/
    unsigned char   ucDtr;                   /*DTR  VALUE*/
    unsigned char   ucDsr;                   /*DSR  VALUE*/
    unsigned char   ucCts;                   /*DTS VALUE*/
    unsigned char   ucRts;                   /*RTS  VALUE*/
    unsigned char   ucRi;                    /*RI VALUE*/
    unsigned char   ucDcd;                   /*DCD  VALUE*/
    unsigned char   ucFc;                    /*FC  VALUE*/
    unsigned char   ucBrk;                   /*BRK  VALUE*/
    unsigned char   ucBrkLen;                /*BRKLEN VALUE*/
} PCDEV_MDM_MSC_STRU;

typedef void (*PCDEV_WRITE_DONE_CB_T)(char *pVirAddr, char *pPhyAddr, int size);
typedef void (*PCDEV_WRITE_INFO_DONE_CB_T)(PCDEV_WRITE_INFO *write_info);
typedef void (*PCDEV_READ_DONE_CB_T)(void);
typedef void (*PCDEV_EVENT_CB_T)(PCDEV_EVT_E evt);
typedef void (*PCDEV_ENUM_DONE_CB_T)(void);
typedef void (*PCDEV_DISABLE_CB_T)(void);
typedef void (*PCDEV_MODEM_MSC_READ_CB_T)(PCDEV_MDM_MSC_STRU *pModemMsc);
typedef void (*PCDEV_MODEM_REL_IND_CB_T)(unsigned int bEnable);
typedef void (*PCDEV_MODEM_MSC_WRITE_CB_T)(PCDEV_MDM_MSC_STRU *pModemMsc);

unsigned int mdrv_pcdev_reg_enumdonecb(PCDEV_ENUM_DONE_CB_T pFunc);
unsigned int mdrv_pcdev_reg_disablecb(PCDEV_ENUM_DONE_CB_T pFunc);

#ifdef _cplusplus
}
#endif
#endif

