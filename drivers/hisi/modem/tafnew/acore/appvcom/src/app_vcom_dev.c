/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
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

#include "PsTypeDef.h"
#include "vos.h"
#include "ps_common_def.h"
#include "app_vcom_dev.h"
#include "taf_type_def.h"
#include "at_type_def.h"
#include "at_mntn.h"
#include "securec.h"
#if ((FEATURE_IOT_ATNLPROXY == FEATURE_ON) || (FEATURE_IOT_RAW_DATA == FEATURE_ON))
#include "at_cmd_proc.h"
#endif

/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_APP_VCOM_DEV_C

/* VCOM CTX,用于保存VCOM的全局变量 */
APP_VCOM_DevCtx g_vcomCtx[APP_VCOM_MAX_NUM];

APP_VCOM_DebugInfo g_appVcomDebugInfo;

/* 虚拟串口文件操作接口 */
static const struct file_operations g_operations_Fops = {
    .owner   = THIS_MODULE,
    .read    = APP_VCOM_Read,
    .poll    = APP_VCOM_Poll,
    .write   = APP_VCOM_Write,
    .open    = APP_VCOM_Open,
    .release = APP_VCOM_Release,
    .unlocked_ioctl = APP_VCOM_Ioctl,
};

/*
 * APPVCOM的规格和应用
 * APPVCOM ID   缓存大小  用途           是否AT的CLIENT    ModemId   USER版本是否使用(参见g_debugAppPortIndex)
 * APPVCOM        4K      RIL(主)               是         MODEM0          Y
 * APPVCOM1       4K      RIL(呼叫)             是         MODEM0          Y
 * APPVCOM2       4K      工程菜单              是         MODEM0          Y
 * APPVCOM3       8K      生产装备(AT SERVER)   是         MODEM0          Y
 * APPVCOM4       4K      audio RIL             是         MODEM0          Y
 * APPVCOM5       4K      RIL(主)               是         MODEM1          Y
 * APPVCOM6       4K      RIL(呼叫)             是         MODEM1          Y
 * APPVCOM7       8K      生产装备(AT SERVER)   是         MODEM1          Y
 * APPVCOM8       4K      工程菜单/HIDP         是         MODEM1          Y
 * APPVCOM9      20K      AGPS                  是         MODEM0          Y
 * APPVCOM10      4K      RIL                   是         MODEM0          Y
 * APPVCOM11      4K      ISDB                  是         MODEM0          UnCertain
 * APPVCOM12     20K      AGPS                  是         MODEM1          Y
 * APPVCOM13      4K      RIL(查询)             是         MODEM0          Y
 * APPVCOM14      4K      RIL(查询)             是         MODEM1          Y
 * APPVCOM15      4K      RIL                   是         MODEM1          Y
 * APPVCOM16      4K      HIDP                  是         MODEM0          Y
 * APPVCOM17      4K      AGPS-AP               是         MODEM0          Y
 * APPVCOM18      4K      NFC                   是         MODEM0          Y
 * APPVCOM19      4K      海外运营商写simlock   是         MODEM0          Y
 * APPVCOM20      4K      RIL(主)               是         MODEM2          Y
 * APPVCOM21      4K      RIL(呼叫)             是         MODEM2          Y
 * APPVCOM22      4K      工程菜单              是         MODEM2          Y
 * APPVCOM23      8K      生产装备(AT SERVER)   是         MODEM2          Y
 * APPVCOM24      4K      AGPS                  是         MODEM2          Y
 * APPVCOM25      4K      RIL                   是         MODEM2          Y
 * APPVCOM26      4K      RIL(查询)             是         MODEM2          Y
 * APPVCOM27      4K      RIL                   是         MODEM0          Y
 * APPVCOM28      4K      RIL                   是         MODEM0          Y
 * APPVCOM29      4K      RIL                   是         MODEM1          Y
 * APPVCOM30      4K      RIL                   是         MODEM1          Y
 * APPVCOM31      4K      RIL                   是         MODEM2          Y
 * APPVCOM32      4K      RIL                   是         MODEM2          Y
 * APPVCOM33      4K      生产装备(AT SERVER)   是         MODEM1          Y
 * APPVCOM34      4K      NFC                   是         MODEM1          Y
 * APPVCOM35      4K      预留                  是         MODEM0          N
 * APPVCOM36      4K      预留                  是         MODEM0          N
 * APPVCOM37      4K      预留                  是         MODEM0          N
 * APPVCOM38      4K      预留                  是         MODEM0          N
 * APPVCOM39      4K      预留                  是         MODEM0          N
 * APPVCOM40      4K      预留                  是         MODEM0          N
 * APPVCOM41      4K      预留                  是         MODEM0          N
 * APPVCOM42      4K      预留                  是         MODEM0          N
 * APPVCOM43      4K      预留                  是         MODEM0          N
 * APPVCOM44      4K      预留                  是         MODEM0          N
 * APPVCOM45      4K      预留                  是         MODEM0          N
 * APPVCOM46      4K      预留                  是         MODEM0          N
 * APPVCOM47      4K      预留                  是         MODEM0          N
 * APPVCOM48      4K      预留                  是         MODEM0          N
 * APPVCOM49      4K      预留                  是         MODEM0          N
 * APPVCOM50      4K      预留                  是         MODEM0          N
 * APPVCOM51      4K      预留                  是         MODEM0          N
 * APPVCOM52      4K      预留                  是         MODEM0          N
 * APPVCOM53      128K    errlog                是                         Y
 * APPVCOM54      4K      T/L装备               否                         UnCertain
 * APPVCOM55      2M      log 3.5               否                         Y
 * APPVCOM56      2M      log 3.5               否                         UnCertain
 * APPVCOM57      4K      预留                  是         MODEM0          N
 * APPVCOM58      4K      预留                  是         MODEM0          N
 * APPVCOM59      4K      预留                  是         MODEM0          N
 * APPVCOM60      4K      预留                  是         MODEM0          N
 * APPVCOM61      4K      预留                  是         MODEM0          N
 * APPVCOM62      4K      预留                  是         MODEM0          N
 * APPVCOM63      4K      预留                  是         MODEM0          N
 */
#if (FEATURE_ON == FEATURE_VCOM_EXT)
static const APP_VCOM_DevConfig g_appVcomCogfigTab[] = {
    { APP_VCOM_DEV_NAME_0, APP_VCOM_SEM_NAME_0, 0x1000, 0 },   /* APPVCOM */
    { APP_VCOM_DEV_NAME_1, APP_VCOM_SEM_NAME_1, 0x1000, 0 },   /* APPVCOM1 */
    { APP_VCOM_DEV_NAME_2, APP_VCOM_SEM_NAME_2, 0x1000, 0 },   /* APPVCOM2 */
    { APP_VCOM_DEV_NAME_3, APP_VCOM_SEM_NAME_3, 0x2000, 0 },   /* APPVCOM3 */
    { APP_VCOM_DEV_NAME_4, APP_VCOM_SEM_NAME_4, 0x1000, 0 },   /* APPVCOM4 */
    { APP_VCOM_DEV_NAME_5, APP_VCOM_SEM_NAME_5, 0x1000, 0 },   /* APPVCOM5 */
    { APP_VCOM_DEV_NAME_6, APP_VCOM_SEM_NAME_6, 0x1000, 0 },   /* APPVCOM6 */
    { APP_VCOM_DEV_NAME_7, APP_VCOM_SEM_NAME_7, 0x2000, 0 },   /* APPVCOM7 */
    { APP_VCOM_DEV_NAME_8, APP_VCOM_SEM_NAME_8, 0x1000, 0 },   /* APPVCOM8 */
    { APP_VCOM_DEV_NAME_9, APP_VCOM_SEM_NAME_9, 0x5000, 0 },   /* APPVCOM9 */
    { APP_VCOM_DEV_NAME_10, APP_VCOM_SEM_NAME_10, 0x1000, 0 }, /* APPVCOM10 */
    { APP_VCOM_DEV_NAME_11, APP_VCOM_SEM_NAME_11, 0x1000, 0 }, /* APPVCOM11 */
    { APP_VCOM_DEV_NAME_12, APP_VCOM_SEM_NAME_12, 0x5000, 0 }, /* APPVCOM12 */
    { APP_VCOM_DEV_NAME_13, APP_VCOM_SEM_NAME_13, 0x1000, 0 }, /* APPVCOM13 */
    { APP_VCOM_DEV_NAME_14, APP_VCOM_SEM_NAME_14, 0x1000, 0 }, /* APPVCOM14 */
    { APP_VCOM_DEV_NAME_15, APP_VCOM_SEM_NAME_15, 0x1000, 0 }, /* APPVCOM15 */
    { APP_VCOM_DEV_NAME_16, APP_VCOM_SEM_NAME_16, 0x1000, 0 }, /* APPVCOM16 */
    { APP_VCOM_DEV_NAME_17, APP_VCOM_SEM_NAME_17, 0x1000, 0 }, /* APPVCOM17 */
    { APP_VCOM_DEV_NAME_18, APP_VCOM_SEM_NAME_18, 0x1000, 0 }, /* APPVCOM18 */
    { APP_VCOM_DEV_NAME_19, APP_VCOM_SEM_NAME_19, 0x1000, 0 }, /* APPVCOM19 */
    { APP_VCOM_DEV_NAME_20, APP_VCOM_SEM_NAME_20, 0x1000, 0 }, /* APPVCOM20 */
    { APP_VCOM_DEV_NAME_21, APP_VCOM_SEM_NAME_21, 0x1000, 0 }, /* APPVCOM21 */
    { APP_VCOM_DEV_NAME_22, APP_VCOM_SEM_NAME_22, 0x1000, 0 }, /* APPVCOM22 */
    { APP_VCOM_DEV_NAME_23, APP_VCOM_SEM_NAME_23, 0x2000, 0 }, /* APPVCOM23 */
    { APP_VCOM_DEV_NAME_24, APP_VCOM_SEM_NAME_24, 0x1000, 0 }, /* APPVCOM24 */
    { APP_VCOM_DEV_NAME_25, APP_VCOM_SEM_NAME_25, 0x1000, 0 }, /* APPVCOM25 */
    { APP_VCOM_DEV_NAME_26, APP_VCOM_SEM_NAME_26, 0x1000, 0 }, /* APPVCOM26 */

    { APP_VCOM_DEV_NAME_27, APP_VCOM_SEM_NAME_27, 0x1000, 0 }, /* APPVCOM27 */
    { APP_VCOM_DEV_NAME_28, APP_VCOM_SEM_NAME_28, 0x1000, 0 }, /* APPVCOM28 */
    { APP_VCOM_DEV_NAME_29, APP_VCOM_SEM_NAME_29, 0x1000, 0 }, /* APPVCOM29 */
    { APP_VCOM_DEV_NAME_30, APP_VCOM_SEM_NAME_30, 0x1000, 0 }, /* APPVCOM30 */
    { APP_VCOM_DEV_NAME_31, APP_VCOM_SEM_NAME_31, 0x1000, 0 }, /* APPVCOM31 */

    { APP_VCOM_DEV_NAME_32, APP_VCOM_SEM_NAME_32, 0x1000, 0 }, /* APPVCOM32 */
    { APP_VCOM_DEV_NAME_33, APP_VCOM_SEM_NAME_33, 0x1000, 0 }, /* APPVCOM33 */
    { APP_VCOM_DEV_NAME_34, APP_VCOM_SEM_NAME_34, 0x1000, 0 }, /* APPVCOM34 */
    { APP_VCOM_DEV_NAME_35, APP_VCOM_SEM_NAME_35, 0x1000, 0 }, /* APPVCOM35 */
    { APP_VCOM_DEV_NAME_36, APP_VCOM_SEM_NAME_36, 0x1000, 0 }, /* APPVCOM36 */
    { APP_VCOM_DEV_NAME_37, APP_VCOM_SEM_NAME_37, 0x1000, 0 }, /* APPVCOM37 */
    { APP_VCOM_DEV_NAME_38, APP_VCOM_SEM_NAME_38, 0x1000, 0 }, /* APPVCOM38 */
    { APP_VCOM_DEV_NAME_39, APP_VCOM_SEM_NAME_39, 0x1000, 0 }, /* APPVCOM39 */
    { APP_VCOM_DEV_NAME_40, APP_VCOM_SEM_NAME_40, 0x1000, 0 }, /* APPVCOM40 */
    { APP_VCOM_DEV_NAME_41, APP_VCOM_SEM_NAME_41, 0x1000, 0 }, /* APPVCOM41 */
    { APP_VCOM_DEV_NAME_42, APP_VCOM_SEM_NAME_42, 0x1000, 0 }, /* APPVCOM42 */
    { APP_VCOM_DEV_NAME_43, APP_VCOM_SEM_NAME_43, 0x1000, 0 }, /* APPVCOM43 */
    { APP_VCOM_DEV_NAME_44, APP_VCOM_SEM_NAME_44, 0x1000, 0 }, /* APPVCOM44 */
    { APP_VCOM_DEV_NAME_45, APP_VCOM_SEM_NAME_45, 0x1000, 0 }, /* APPVCOM45 */
    { APP_VCOM_DEV_NAME_46, APP_VCOM_SEM_NAME_46, 0x1000, 0 }, /* APPVCOM46 */
    { APP_VCOM_DEV_NAME_47, APP_VCOM_SEM_NAME_47, 0x1000, 0 }, /* APPVCOM47 */
    { APP_VCOM_DEV_NAME_48, APP_VCOM_SEM_NAME_48, 0x1000, 0 }, /* APPVCOM48 */
    { APP_VCOM_DEV_NAME_49, APP_VCOM_SEM_NAME_49, 0x1000, 0 }, /* APPVCOM49 */
    { APP_VCOM_DEV_NAME_50, APP_VCOM_SEM_NAME_50, 0x1000, 0 }, /* APPVCOM50 */
    { APP_VCOM_DEV_NAME_51, APP_VCOM_SEM_NAME_51, 0x1000, 0 }, /* APPVCOM51 */
    { APP_VCOM_DEV_NAME_52, APP_VCOM_SEM_NAME_52, 0x1000, 0 }, /* APPVCOM52 */

    { APP_VCOM_DEV_NAME_53, APP_VCOM_SEM_NAME_53, 0x20000, 0 },  /* APPVCOM53 */
    { APP_VCOM_DEV_NAME_54, APP_VCOM_SEM_NAME_54, 0x1000, 0 },   /* APPVCOM54 */
    { APP_VCOM_DEV_NAME_55, APP_VCOM_SEM_NAME_55, 0x200000, 0 }, /* APPVCOM55 */
    { APP_VCOM_DEV_NAME_56, APP_VCOM_SEM_NAME_56, 0x200000, 0 }, /* APPVCOM56 */
    { APP_VCOM_DEV_NAME_57, APP_VCOM_SEM_NAME_57, 0x1000, 0 },   /* APPVCOM57 */

    { APP_VCOM_DEV_NAME_58, APP_VCOM_SEM_NAME_58, 0x1000, 0 }, /* APPVCOM58 */
    { APP_VCOM_DEV_NAME_59, APP_VCOM_SEM_NAME_59, 0x1000, 0 }, /* APPVCOM59 */
    { APP_VCOM_DEV_NAME_60, APP_VCOM_SEM_NAME_60, 0x1000, 0 }, /* APPVCOM60 */
    { APP_VCOM_DEV_NAME_61, APP_VCOM_SEM_NAME_61, 0x1000, 0 }, /* APPVCOM61 */
    { APP_VCOM_DEV_NAME_62, APP_VCOM_SEM_NAME_62, 0x1000, 0 }, /* APPVCOM62 */
#if (FEATURE_IOT_RAW_DATA == FEATURE_OFF)
    { APP_VCOM_DEV_NAME_63, APP_VCOM_SEM_NAME_63, 0x1000, 0 }  /* APPVCOM63 */
#else
    { APP_VCOM_DEV_NAME_IOT, APP_VCOM_SEM_NAME_IOT, 0x1000, 0 }  /* RAW DATA IOT */
#endif
};
#else
static const APP_VCOM_DevConfig g_appVcomCogfigTab[] = {
    { APP_VCOM_DEV_NAME_0, APP_VCOM_SEM_NAME_0, 0x1000, 0 }, /* APPVCOM */
    { APP_VCOM_DEV_NAME_1, APP_VCOM_SEM_NAME_1, 0x1000, 0 }, /* APPVCOM1 */
#if (FEATURE_IOT_RAW_DATA == FEATURE_ON)
    { APP_VCOM_DEV_NAME_IOT, APP_VCOM_SEM_NAME_IOT, 0x1000, 0 } /* RAW DATA IOT */
#endif
};
#endif

APP_VCOM_DebugCfg g_appVcomDebugCfg;

#if (FEATURE_ON == FEATURE_VCOM_EXT)
#if (FEATURE_OFF == FEATURE_DEBUG_APP_PORT)
/*
 * user版本不使用端口列表，当前只关闭保留端口，
 * 非保留端口大部分当前已经明确使用，当前不确认的端口有:
 * APPVCOM11 APPVCOM54 APPVCOM55,由于这几个端口当前不是很明确是否使用，所以user版本默认不关闭
 */
static const APP_VCOM_DevIndexUint8 g_debugAppPortIndex[] = {
    APP_VCOM_DEV_INDEX_35, APP_VCOM_DEV_INDEX_36, APP_VCOM_DEV_INDEX_37, APP_VCOM_DEV_INDEX_38, APP_VCOM_DEV_INDEX_39,
    APP_VCOM_DEV_INDEX_40, APP_VCOM_DEV_INDEX_41, APP_VCOM_DEV_INDEX_42, APP_VCOM_DEV_INDEX_43, APP_VCOM_DEV_INDEX_44,
    APP_VCOM_DEV_INDEX_45, APP_VCOM_DEV_INDEX_46, APP_VCOM_DEV_INDEX_47, APP_VCOM_DEV_INDEX_48, APP_VCOM_DEV_INDEX_49,
    APP_VCOM_DEV_INDEX_50, APP_VCOM_DEV_INDEX_51, APP_VCOM_DEV_INDEX_52, APP_VCOM_DEV_INDEX_57, APP_VCOM_DEV_INDEX_58,
    APP_VCOM_DEV_INDEX_59, APP_VCOM_DEV_INDEX_60, APP_VCOM_DEV_INDEX_61, APP_VCOM_DEV_INDEX_62, APP_VCOM_DEV_INDEX_63
};

VOS_UINT8 APP_VCOM_IsDebugAppPort(APP_VCOM_DevIndexUint8 devIndex)
{
    VOS_UINT32 i = 0;

    for (i = 0; i < APP_VCOM_ARRAY_SIZE(g_debugAppPortIndex); i++) {
        if (devIndex == g_debugAppPortIndex[i]) {
            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}
#endif
#endif

/*
 * 3 函数、变量声明
 */
#if (FEATURE_IOT_RAW_DATA == FEATURE_ON)
VOS_INT IOT_VCOM_Ioctl(struct file *f, unsigned int cmd, unsigned long arg);
VOS_INT IOT_VCOM_Ioctl_CmdModeProc(APP_VCOM_DevIndexUint8 devId, VOS_UINT16 physicalPortId);
VOS_INT IOT_VCOM_Ioctl_SetDataModeProc(APP_VCOM_DevIndexUint8 devId, VOS_UINT16 physicalPortId);
VOS_INT IOT_VCOM_Ioctl_ClearRxData(APP_VCOM_DevIndexUint8 devId);
VOS_INT IOT_VCOM_ProcModeChange(APP_VCOM_IoctlUint8 ioCtrlCmd, DMS_PortIdUint16 modePortId);

VOS_UINT32 IOT_VCOM_Send(VOS_UINT8 *data, VOS_UINT32 uslength)
{
    APP_VCOM_TRACE_INFO(APP_VCOM_DEV_INDEX_IOT, "IOT_VCOM_Send, uslength%d. ", uslength);
    return APP_VCOM_Send(APP_VCOM_DEV_INDEX_IOT, data, uslength);
}

VOS_VOID IOT_VCOM_WaterMarkProc(VOS_VOID)
{
    APP_VCOM_DevEntity           *vcomDev = VOS_NULL_PTR;
    APP_VCOM_TRACE_INFO(APP_VCOM_DEV_INDEX_IOT, "IOT_VCOM_WaterMarkProc.");
    vcomDev = APP_VCOM_GetAppVcomDevEntity(APP_VCOM_DEV_INDEX_IOT);
    wake_up_interruptible(&vcomDev->iotWriteWait);
}

VOS_INT32 IOT_VCOM_Write(VOS_UINT8 devId, VOS_UINT8 *data, VOS_UINT32 len)
{
    APP_VCOM_DevCtx              *pstVcomCtx = VOS_NULL_PTR;
    APP_VCOM_DevEntity           *vcomDev = VOS_NULL_PTR;

    /* 获得设备实体指针 */
    vcomDev = APP_VCOM_GetAppVcomDevEntity(devId);
    pstVcomCtx = APP_VCOM_GetVcomCtxAddr(devId);

#if (FEATURE_AT_HSUART == FEATURE_ON)
    APP_VCOM_TRACE_INFO(devId, "IOT_VCOM_Write 111.");
    if (pstVcomCtx->dstPortId == DMS_PORT_HSUART) {
        if (wait_event_interruptible(vcomDev->iotWriteWait, !DMS_PORT_GetUartWaterMarkFlg())) {
            return -ERESTARTSYS;
        }
    }
    APP_VCOM_TRACE_INFO(devId, "IOT_VCOM_Write 222.");
#endif

    return (VOS_INT32)DMS_PORT_SendData(pstVcomCtx->dstPortId, data, len);
}

VOS_INT IOT_VCOM_Ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
    APP_VCOM_DevCtx                    *vcomCtx = VOS_NULL_PTR;
    APP_VCOM_DevEntity                 *vcomDev = VOS_NULL_PTR;
    struct cdev                        *cdev = VOS_NULL_PTR;
    LU_IOC_CONTROL                     *control = VOS_NULL_PTR;
    LU_IOC_CONTROL                      iocmode;
    VOS_UINT                            devMajor;
    VOS_INT                             result = -1;
    VOS_UINT16                          phyPortId;
    APP_VCOM_DevIndexUint8              devId;

    vcomDev = f->private_data;
    cdev    = &(vcomDev->appVcomDev);
    devMajor = MAJOR(cdev->dev);
    devId = APP_VCOM_GetIndexFromMajorDevId(devMajor);
    vcomCtx = APP_VCOM_GetVcomCtxAddr(devId);
    control = (LU_IOC_CONTROL *)(uintptr_t)arg;

    if ((devId != APP_VCOM_DEV_INDEX_IOT) || (control == VOS_NULL_PTR)) {
        return result;
    }

    if (copy_from_user(&iocmode, control, sizeof(LU_IOC_CONTROL))) {
        APP_VCOM_TRACE_ERR(devId, "IOT_VCOM_Ioctl, copy_from_user error!");
        return result;
    }

    phyPortId = AT_GetDmsPortIdByClientId((AT_ClientIdUint16)iocmode.mode_port_id);
    APP_VCOM_TRACE_INFO(devId, "IOT_VCOM_Ioctl, phyPortId: %d. mode:%d", phyPortId, iocmode.mode_port_id);
    switch(cmd) {
        case LU_IOC_CMD:
            result = IOT_VCOM_Ioctl_CmdModeProc(devId, phyPortId);
            break;

        case LU_IOC_RAWDATA:
            result = IOT_VCOM_Ioctl_SetDataModeProc(devId, phyPortId);
            break;

        case LU_IOC_CLEARDATA:
            result = IOT_VCOM_Ioctl_ClearRxData(devId);
            break;

        default:
            break;
    }

    return result;
}

VOS_VOID IOT_VCOM_RxFlowCtrl(APP_VCOM_DevEntity *vcomDev, DMS_PortIdUint16 portId)
{
    APP_VCOM_TRACE_INFO(APP_VCOM_DEV_INDEX_IOT, "IOT_VCOM_RxFlowCtrl, cur:%u H:%u,L:%u ON_OFF:%d.",
                                                vcomDev->currentLen,
                                                vcomDev->rxWmHighThresh,
                                                vcomDev->rxWmLowThresh,
                                                vcomDev->isFlowctrlOn);

    if ((vcomDev->currentLen >= vcomDev->rxWmHighThresh) && (vcomDev->isFlowctrlOn == VOS_FALSE)) {
        vcomDev->isFlowctrlOn = VOS_TRUE;
        DMS_PORT_StartFlowCtrl(portId, VOS_TRUE);
        g_appVcomDebugInfo.startFlowCtrl++;
    } else if ((vcomDev->currentLen <= vcomDev->rxWmLowThresh) && (vcomDev->isFlowctrlOn == VOS_TRUE)) {
        vcomDev->isFlowctrlOn = VOS_FALSE;
        DMS_PORT_StopFlowCtrl(portId, 0);
        g_appVcomDebugInfo.stopFlowCtrl++;
    }
}

VOS_VOID IOT_VCOM_ForceDisableFlow(APP_VCOM_DevIndexUint8 devIndex)
{
    APP_VCOM_DevEntity           *vcomDev = VOS_NULL_PTR;
    APP_VCOM_DevCtx              *vcomCtx = VOS_NULL_PTR;

    vcomDev = APP_VCOM_GetAppVcomDevEntity(devIndex);
    vcomCtx = APP_VCOM_GetVcomCtxAddr(devIndex);

    if (vcomDev->isFlowctrlOn == VOS_TRUE) {
        vcomDev->isFlowctrlOn = VOS_FALSE;
        g_appVcomDebugInfo.stopFlowCtrl++;
        DMS_PORT_StopFlowCtrl(vcomCtx->dstPortId, 0);
        msleep(WAIT_DISCARD_TIME_MS);// 防止将透传未传输完成的数据当做AT命令处理
        APP_VCOM_ERR_LOG(devIndex, "Forcedisableflow.");
    }
}

VOS_INT IOT_VCOM_Ioctl_CmdModeProc(APP_VCOM_DevIndexUint8 devId, VOS_UINT16 physicalPortId)
{
    APP_VCOM_DevCtx                    *vcomCtx = VOS_NULL_PTR;
    APP_VCOM_DevEntity                 *vcomDev = VOS_NULL_PTR;
    VOS_INT                             result = -1;

    vcomCtx = APP_VCOM_GetVcomCtxAddr(devId);
    vcomDev = APP_VCOM_GetAppVcomDevEntity(devId);
    APP_VCOM_TRACE_INFO(devId, "IOT_VCOM_Ioctl_CmdModeProc, ioctrlProcessing: %d.", vcomCtx->ioctrlProcessing);
    if (!vcomCtx->ioctrlProcessing) {
        vcomCtx->ioctrlProcessing = VOS_TRUE;
        vcomDev->discard = VOS_TRUE; // 丢弃数据，避免再次进入反压状态
        IOT_VCOM_ForceDisableFlow(devId);
        result = IOT_VCOM_ProcModeChange(IOCTL_SWITCH_CMD_MODE, physicalPortId);
        APP_VCOM_RegDataCallback(devId, VOS_NULL_PTR);//断开下行发送通路
        vcomCtx->ioctrlProcessing = VOS_FALSE;
        vcomCtx->ioctrlResult     = result;
        vcomCtx->dstPortId = DMS_PORT_BUTT;
    }

    return vcomCtx->ioctrlResult;
}

VOS_INT IOT_VCOM_Ioctl_SetDataModeProc(APP_VCOM_DevIndexUint8 devId, VOS_UINT16 physicalPortId)
{
    APP_VCOM_DevCtx                    *vcomCtx = VOS_NULL_PTR;
    APP_VCOM_DevEntity                 *vcomDev = VOS_NULL_PTR;
    VOS_INT                             result = -1;

    vcomCtx = APP_VCOM_GetVcomCtxAddr(devId);
    vcomDev = APP_VCOM_GetAppVcomDevEntity(devId);
    APP_VCOM_TRACE_INFO(devId, "IOT_VCOM_Ioctl_SetDataModeProc, ioctrlProcessing: %d. ", vcomCtx->ioctrlProcessing);
    if (!vcomCtx->ioctrlProcessing) {
        vcomCtx->ioctrlProcessing = VOS_TRUE;
        vcomDev->discard = VOS_FALSE; //恢复数据接收
        result = IOT_VCOM_ProcModeChange(IOCTL_SWITCH_RAW_DATA_MODE, physicalPortId);
        vcomCtx->dstPortId = physicalPortId;
        APP_VCOM_RegDataCallback(devId, IOT_VCOM_Write); //注册APPVCOM下行数据处理函数
        vcomCtx->ioctrlProcessing = VOS_FALSE;
        vcomCtx->ioctrlResult     = result;
    }

    return vcomCtx->ioctrlResult;
}

VOS_INT IOT_VCOM_Ioctl_ClearRxData(APP_VCOM_DevIndexUint8 devId)
{
    APP_VCOM_DevEntity *vcomDev = VOS_NULL_PTR;

    vcomDev = APP_VCOM_GetAppVcomDevEntity(devId);

    down(&vcomDev->msgSendSem);
    vcomDev->currentLen = 0;
    up(&vcomDev->msgSendSem);
    APP_VCOM_TRACE_INFO(devId, "IOT_VCOM_Ioctl_ClearRxData, currentLen: %d.", vcomDev->currentLen);
    return VOS_OK;
}

VOS_INT IOT_VCOM_ProcModeChange(APP_VCOM_IoctlUint8 ioCtrlCmd, DMS_PortIdUint16 modePortId)
{
    APP_VCOM_TRACE_INFO(APP_VCOM_DEV_INDEX_IOT, "IOT_VCOM_ProcModeChange, ioCtrlCmd:%d modePortId: %d.", ioCtrlCmd, modePortId);

    if ((modePortId >= DMS_PORT_BUTT)) {
        return VOS_ERROR;
    }

    switch (ioCtrlCmd) {
        case IOCTL_SWITCH_CMD_MODE:
            DMS_PORT_ResumeCmdMode(modePortId);
            break;

        case IOCTL_SWITCH_RAW_DATA_MODE:
            /* 注册物理端口上行数据处理函数 */
            DMS_PORT_SwitchRawDataMode(modePortId, IOT_VCOM_Send);
            break;

        default:
            break;
    }
    return VOS_OK;
}

VOS_VOID IOT_VCOM_EntityInit(VOS_UINT32 indexNum, APP_VCOM_DevEntity *vcomDevp)
{
    init_waitqueue_head(&vcomDevp->iotWriteWait);
    vcomDevp->isFlowctrlOn = VOS_FALSE;
    vcomDevp->rxWmLowThresh = g_appVcomCogfigTab[indexNum].appVcomMemSize / 4;
    vcomDevp->rxWmHighThresh = g_appVcomCogfigTab[indexNum].appVcomMemSize * 3 / 4;
    vcomDevp->discard = VOS_FALSE;
}
#endif

APP_VCOM_DevCtx* APP_VCOM_GetVcomCtxAddr(VOS_UINT8 indexNum)
{
    return &(g_vcomCtx[indexNum]);
}

APP_VCOM_DevEntity* APP_VCOM_GetAppVcomDevEntity(VOS_UINT8 indexNum)
{
    return (g_vcomCtx[indexNum].appVcomDevEntity);
}

VOS_UINT32 APP_VCOM_RegDataCallback(VOS_UINT8 devIndex, SEND_UL_AT_FUNC func)
{
    APP_VCOM_DevCtx *vcomCtx = VOS_NULL_PTR;

    /* 索引号错误 */
    if (devIndex >= APP_VCOM_DEV_INDEX_BUTT) {
        return VOS_ERR;
    }

    vcomCtx = APP_VCOM_GetVcomCtxAddr(devIndex);

    /* 函数指针赋给全局变量 */
    vcomCtx->sendUlAtFunc = func;

    return VOS_OK;
}

VOS_UINT8 APP_VCOM_GetIndexFromMajorDevId(VOS_UINT majorDevId)
{
    VOS_UINT32       loop;
    APP_VCOM_DevCtx *vcomCtx = VOS_NULL_PTR;

    for (loop = 0; loop < APP_VCOM_MAX_NUM; loop++) {
        vcomCtx = APP_VCOM_GetVcomCtxAddr(loop);

        if (vcomCtx->appVcomMajorId == majorDevId) {
            break;
        }
    }

    return loop;
}

VOS_VOID APP_VCOM_InitSpecCtx(VOS_UINT8 devIndex)
{
    APP_VCOM_DevCtx *vcomCtx = VOS_NULL_PTR;
    errno_t          memResult;

    if (devIndex >= APP_VCOM_MAX_NUM) {
        return;
    }

    vcomCtx = APP_VCOM_GetVcomCtxAddr(devIndex);

    (VOS_VOID)memset_s(vcomCtx->sendSemName, sizeof(vcomCtx->sendSemName), 0x00, sizeof(vcomCtx->sendSemName));
    (VOS_VOID)memset_s(vcomCtx->appVcomName, sizeof(vcomCtx->appVcomName), 0x00, sizeof(vcomCtx->appVcomName));

    if (VOS_StrLen(g_appVcomCogfigTab[devIndex].appVcomName) > 0) {
        memResult = memcpy_s(vcomCtx->appVcomName, sizeof(vcomCtx->appVcomName),
                             g_appVcomCogfigTab[devIndex].appVcomName,
                             VOS_StrLen(g_appVcomCogfigTab[devIndex].appVcomName));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(vcomCtx->appVcomName),
                            VOS_StrLen(g_appVcomCogfigTab[devIndex].appVcomName));
    }

    if (VOS_StrLen(g_appVcomCogfigTab[devIndex].sendSemName) > 0) {
        memResult = memcpy_s(vcomCtx->sendSemName, sizeof(vcomCtx->sendSemName),
                             g_appVcomCogfigTab[devIndex].sendSemName,
                             VOS_StrLen(g_appVcomCogfigTab[devIndex].sendSemName));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(vcomCtx->sendSemName),
                            VOS_StrLen(g_appVcomCogfigTab[devIndex].sendSemName));
    }

    vcomCtx->appVcomMajorId = APP_VCOM_MAJOR_DEV_ID + devIndex;
#if (FEATURE_IOT_RAW_DATA == FEATURE_ON)
    vcomCtx->ioctrlProcessing = VOS_FALSE;
    vcomCtx->ioctrlResult     = VOS_OK;
    vcomCtx->dstPortId        = DMS_PORT_BUTT;
#endif
}

VOS_VOID APP_VCOM_Setup(APP_VCOM_DevEntity *dev, VOS_UINT8 indexNum)
{
    static struct class *comClass; /*lint !e565*/
    APP_VCOM_DevCtx     *vcomCtx;
    VOS_INT              err;
    dev_t                devno;

    vcomCtx = APP_VCOM_GetVcomCtxAddr(indexNum);

    devno = MKDEV(vcomCtx->appVcomMajorId, indexNum);

    cdev_init(&dev->appVcomDev, &g_operations_Fops);

    err = cdev_add(&dev->appVcomDev, devno, 1);
    if (err) {
        APP_VCOM_TRACE_ERR(indexNum, "APP_VCOM_Setup cdev_add error! ");
        return;
    }

    comClass = class_create(THIS_MODULE, vcomCtx->appVcomName);

    device_create(comClass, NULL, MKDEV(vcomCtx->appVcomMajorId, indexNum), "%s", vcomCtx->appVcomName);
}

VOS_UINT32 APP_VCOM_RegDev(VOS_UINT32 indexNum, APP_VCOM_DevEntity *vcomDevp)
{
    APP_VCOM_DevCtx    *vcomCtx  = VOS_NULL_PTR;
    VOS_INT             result1;
    VOS_INT             result2;
    dev_t               devno;

    /* 初始化全局变量 */
    APP_VCOM_InitSpecCtx(indexNum);

    /* 获取全局变量指针 */
    vcomCtx = APP_VCOM_GetVcomCtxAddr(indexNum);

    /* 将设备号转换成dev_t 类型 */
    devno = MKDEV(vcomCtx->appVcomMajorId, indexNum);

    result1 = register_chrdev_region(devno, 1, vcomCtx->appVcomName);

    /* 注册失败则动态申请设备号 */
    if (result1 < 0) {
        result2 = alloc_chrdev_region(&devno, 0, 1, vcomCtx->appVcomName);

        if (result2 < 0) {
            return VOS_ERR;
        }

        vcomCtx->appVcomMajorId = MAJOR(devno);
    }

    /* 动态申请设备结构体内存 */
    vcomCtx->appVcomDevEntity = kmalloc(sizeof(APP_VCOM_DevEntity), GFP_KERNEL);

    if (vcomCtx->appVcomDevEntity == VOS_NULL_PTR) {
        /* 去注册该设备，返回错误 */
        unregister_chrdev_region(devno, 1);
        APP_VCOM_TRACE_ERR(indexNum, "APP_VCOM_Init malloc device Entity fail. ");
        return VOS_ERR;
    }

    /* 获取设备实体指针 */
    vcomDevp = vcomCtx->appVcomDevEntity;

    (VOS_VOID)memset_s(vcomDevp, sizeof(APP_VCOM_DevEntity), 0x00, sizeof(APP_VCOM_DevEntity));

    if (APPVCOM_STATIC_MALLOC_MEMORY(indexNum)) {
        vcomDevp->appVcomMem = kmalloc(g_appVcomCogfigTab[indexNum].appVcomMemSize, GFP_KERNEL);

        if (vcomDevp->appVcomMem == VOS_NULL_PTR) {
            /* 去注册该设备，返回错误 */
            unregister_chrdev_region(devno, 1);
            APP_VCOM_TRACE_ERR(indexNum, "APP_VCOM_Init malloc device buff fail. ");
            kfree(vcomCtx->appVcomDevEntity);
            vcomCtx->appVcomDevEntity = VOS_NULL_PTR;
            return VOS_ERR;
        }
    }

    init_waitqueue_head(&vcomDevp->readWait);
    (VOS_VOID)memset_s(vcomDevp->wakeLockName, sizeof(vcomDevp->wakeLockName), 0x00, sizeof(vcomDevp->wakeLockName));
    scnprintf(vcomDevp->wakeLockName, APP_VCOM_RD_WAKE_LOCK_NAME_LEN, "appvcom%d_rd_wake", indexNum);
    vcomDevp->wakeLockName[APP_VCOM_RD_WAKE_LOCK_NAME_LEN - 1] = '\0';
    wakeup_source_init(&vcomDevp->rdWakeLock, vcomDevp->wakeLockName);
#if (FEATURE_IOT_RAW_DATA == FEATURE_ON)
    IOT_VCOM_EntityInit(indexNum, vcomDevp);
#endif
    mutex_init(&vcomDevp->mutex);

    APP_VCOM_Setup(vcomDevp, indexNum);

    /* 创建信号量 */
    sema_init(&vcomDevp->msgSendSem, 1);
    sema_init(&vcomDevp->wrtSem, 1);
    return VOS_OK;
}

VOS_INT APP_VCOM_Init(VOS_VOID)
{
    APP_VCOM_DevEntity *vcomDevp = VOS_NULL_PTR;
    VOS_UINT32          indexNum;

    APP_VCOM_PR_LOGI("entry,%u", VOS_GetSlice());

    /* 初始化可维可测全局变量 */
    (VOS_VOID)memset_s(&g_appVcomDebugInfo, sizeof(g_appVcomDebugInfo), 0x00, sizeof(g_appVcomDebugInfo));

    (VOS_VOID)memset_s(&g_appVcomDebugCfg, sizeof(g_appVcomDebugCfg), 0x00, sizeof(g_appVcomDebugCfg));

    /* 初始化虚拟设备 */
    for (indexNum = 0; indexNum < APP_VCOM_MAX_NUM; indexNum++) {
#if (FEATURE_ON == FEATURE_VCOM_EXT)
#if (FEATURE_OFF == FEATURE_DEBUG_APP_PORT)
        if (APP_VCOM_IsDebugAppPort(indexNum) == VOS_TRUE) {
            APP_VCOM_TRACE_NORM(indexNum, "APP_VCOM_Init, DEBUG_APP_PORT not open, port is eng port, dont init. ");
            continue;
        }
#endif
#endif

        if (APP_VCOM_RegDev(indexNum, vcomDevp) != VOS_OK) {
            return VOS_ERROR;
        }
    }

    APP_VCOM_PR_LOGI("eixt,%u", VOS_GetSlice());

    return VOS_OK;
}

VOS_UINT32 APP_VCOM_CheckReleasePara(struct inode *inode, struct file *filp)
{
    if ((inode == VOS_NULL_PTR) || (filp == VOS_NULL_PTR)) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 APP_VCOM_CheckOpenPara(struct inode *inode, struct file *filp)
{
    if (inode == VOS_NULL_PTR || filp == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 APP_VCOM_CheckReadPara(struct file *filp, char __user *buf, loff_t *ppos)
{
    if ((filp == VOS_NULL_PTR) || (buf == VOS_NULL_PTR) || (ppos == VOS_NULL_PTR)) {
        return VOS_ERR;
    }

    return VOS_OK;
}

LOCAL VOS_UINT32 APP_VCOM_CopyToUser(char __user *buf, APP_VCOM_DevCtx *vcomCtx, APP_VCOM_DevEntity *vcomDev,
    size_t *count, VOS_UINT8 indexNum)
{
    errno_t             memResult;

    /* 获取信号量 */
    down(&vcomCtx->appVcomDevEntity->msgSendSem);
#if (FEATURE_IOT_RAW_DATA == FEATURE_ON)
    g_appVcomDebugInfo.totalRdLen += *count;
#endif
    if (APPVCOM_DYNAMIC_MALLOC_MEMORY(indexNum) && (vcomDev->appVcomMem == VOS_NULL_PTR)) {
        return VOS_ERR;
    }

    *count = TAF_MIN(*count, vcomDev->currentLen);

    if (copy_to_user(buf, vcomDev->appVcomMem, (VOS_ULONG)(*count))) {
        APP_VCOM_TRACE_ERR(indexNum, "APP_VCOM_CopyToUser, copy_to_user fail. ");
#if (FEATURE_IOT_RAW_DATA == FEATURE_ON)
        g_appVcomDebugInfo.rdCopyErr++;
#endif
        return VOS_ERR;
    }

    if ((vcomDev->currentLen - *count) > 0) {
        /* FIFO数据前移 */
        /*lint -save -e661*/
        memResult = memmove_s(vcomDev->appVcomMem, g_appVcomCogfigTab[indexNum].appVcomMemSize,
                              vcomDev->appVcomMem + *count, vcomDev->currentLen - *count);
        TAF_MEM_CHK_RTN_VAL(memResult, g_appVcomCogfigTab[indexNum].appVcomMemSize, vcomDev->currentLen - *count);
        /*lint -restore +e661*/
        APP_VCOM_TRACE_INFO(indexNum, "APP_VCOM_CopyToUser, FIFO move. ");
    }

    /* 有效数据长度减小 */
    vcomDev->currentLen -= *count;

    APP_VCOM_TRACE_INFO(indexNum, "APP_VCOM_CopyToUser, read %u bytes, current_len:%u.", *count,
                        vcomDev->currentLen); /*lint !e559 */

    if (vcomDev->currentLen == 0) {
        APP_VCOM_TRACE_NORM(indexNum, "APP_VCOM_CopyToUser: read all data. ");
        /*lint -save -e455 */
        __pm_relax(&vcomDev->rdWakeLock);
        /*lint -restore */
    }
#if (FEATURE_IOT_RAW_DATA == FEATURE_ON)
    if (indexNum == APP_VCOM_DEV_INDEX_IOT) {
        IOT_VCOM_RxFlowCtrl(vcomDev, vcomCtx->dstPortId);
        g_appVcomDebugInfo.totalRdoutLen += *count;
    }
#endif
    /* 释放信号量 */
    up(&vcomCtx->appVcomDevEntity->msgSendSem);

    return VOS_OK;
}

#if (FEATURE_ON == FEATURE_VCOM_EXT)
VOS_VOID APP_VCOM_FreeDynamicMem(VOS_UINT8 indexNum, APP_VCOM_DevCtx *vcomCtx)
{
    APP_VCOM_DevEntity *vcomDevp = VOS_NULL_PTR;

    if (APPVCOM_DYNAMIC_MALLOC_MEMORY(indexNum)) {
        vcomDevp = vcomCtx->appVcomDevEntity;

        down(&vcomDevp->msgSendSem);

        if (vcomDevp->appVcomMem != VOS_NULL_PTR) {
            kfree(vcomDevp->appVcomMem);
            vcomDevp->appVcomMem = VOS_NULL_PTR;
            APP_VCOM_TRACE_INFO(indexNum, "APP_VCOM_Release free memory is ok. ");
        }

        up(&vcomDevp->msgSendSem);
    }
}

VOS_UINT32 APP_VCOM_MallocDynamicMem(VOS_UINT8 indexNum, APP_VCOM_DevCtx *vcomCtx)
{
    APP_VCOM_DevEntity *vcomDevp = VOS_NULL_PTR;
    VOS_UINT32          memSize;

    if (APPVCOM_DYNAMIC_MALLOC_MEMORY(indexNum)) {
        /* 获取设备实体指针 */
        vcomDevp = vcomCtx->appVcomDevEntity;

        down(&vcomDevp->msgSendSem);

        if (vcomDevp->appVcomMem == VOS_NULL_PTR) {
            memSize              = TAF_MIN(g_appVcomCogfigTab[indexNum].appVcomMemSize, KMALLOC_MAX_SIZE);
            vcomDevp->appVcomMem = kmalloc(memSize, GFP_KERNEL);

            if (vcomDevp->appVcomMem == VOS_NULL_PTR) {
                APP_VCOM_TRACE_ERR(indexNum, "APP_VCOM_Open alloc memory is err. ");
                up(&vcomDevp->msgSendSem);
                return VOS_ERR;
            }
            APP_VCOM_TRACE_INFO(indexNum, "APP_VCOM_Open alloc memory is ok. ");
        }

        up(&vcomDevp->msgSendSem);
    }
    return VOS_OK;
}
#endif

int APP_VCOM_Release(struct inode *inode, struct file *filp)
{
    VOS_UINT         devMajor;
    VOS_UINT8        indexNum;
    APP_VCOM_DevCtx *vcomCtx = VOS_NULL_PTR;

    if (APP_VCOM_CheckReleasePara(inode, filp) != VOS_TRUE) {
        return VOS_ERROR;
    }

    /* 获取主设备号 */
    devMajor = imajor(inode);

    /* 根据主设备号得到设备在全局变量中的索引值 */
    indexNum = APP_VCOM_GetIndexFromMajorDevId(devMajor);

    if (indexNum >= APP_VCOM_MAX_NUM) {
        APP_VCOM_ERR_LOG(indexNum, "APP_VCOM_Release ucIndex is error. ");
        return VOS_ERROR;
    }

    /* 获取VCOM全局变量 */
    vcomCtx = APP_VCOM_GetVcomCtxAddr(indexNum);

    if (vcomCtx->appVcomDevEntity == VOS_NULL_PTR) {
        APP_VCOM_TRACE_ERR(indexNum, "APP_VCOM_Release VcomDevEntity is NULL. ");
        return VOS_ERROR;
    }

#if (FEATURE_ON == FEATURE_VCOM_EXT)
    APP_VCOM_FreeDynamicMem(indexNum, vcomCtx);
#endif

    /* 将设备结构体指针赋值给文件私有数据指针 */
    filp->private_data = vcomCtx->appVcomDevEntity;

    APP_VCOM_TRACE_INFO(indexNum, "APP_VCOM_Release enter. ");

    if (vcomCtx->eventFunc != VOS_NULL_PTR) {
        (VOS_VOID)(vcomCtx->eventFunc(APP_VCOM_EVT_RELEASE));
    }

    vcomCtx->appVcomDevEntity->isDeviceOpen = VOS_FALSE;
    /*lint -save -e455 */
    __pm_relax(&vcomCtx->appVcomDevEntity->rdWakeLock);
    /*lint -restore */

    return VOS_OK;
}

int APP_VCOM_Open(struct inode *inode, struct file *filp)
{
    VOS_UINT         devMajor;
    VOS_UINT8        indexNum;
    APP_VCOM_DevCtx *vcomCtx = VOS_NULL_PTR;

    if (APP_VCOM_CheckOpenPara(inode, filp) != VOS_TRUE) {
        return VOS_ERROR;
    }

    /* 获取主设备号 */
    devMajor = imajor(inode);

    /* 根据主设备号得到设备在全局变量中的索引值 */
    indexNum = APP_VCOM_GetIndexFromMajorDevId(devMajor);

    if (indexNum >= APP_VCOM_MAX_NUM) {
        APP_VCOM_ERR_LOG(indexNum, "APP_VCOM_Open ucIndex is error. ");
        return VOS_ERROR;
    }

    /* 获取VCOM全局变量 */
    vcomCtx = APP_VCOM_GetVcomCtxAddr(indexNum);

    if (vcomCtx->appVcomDevEntity == VOS_NULL_PTR) {
        APP_VCOM_TRACE_ERR(indexNum, "APP_VCOM_Open VcomDevEntity is NULL. ");
        return VOS_ERR;
    }

#if (FEATURE_ON == FEATURE_VCOM_EXT)
    if (APP_VCOM_MallocDynamicMem(indexNum, vcomCtx) != VOS_OK) {
        return VOS_ERROR;
    }
#endif
    /* 将设备结构体指针赋值给文件私有数据指针 */
    filp->private_data = vcomCtx->appVcomDevEntity;

    APP_VCOM_TRACE_INFO(indexNum, "APP_VCOM_Open enter. ");

    if (vcomCtx->eventFunc != NULL) {
        (VOS_VOID)(vcomCtx->eventFunc(APP_VCOM_EVT_OPEN));
    }

    vcomCtx->appVcomDevEntity->isDeviceOpen = VOS_TRUE;

    return VOS_OK;
}

ssize_t APP_VCOM_Read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
    VOS_UINT8           indexNum;
    VOS_UINT            devMajor;
    struct cdev        *cdev    = VOS_NULL_PTR;
    APP_VCOM_DevEntity *vcomDev = VOS_NULL_PTR;
    APP_VCOM_DevCtx    *vcomCtx = VOS_NULL_PTR;

    if (APP_VCOM_CheckReadPara(filp, buf, ppos) != VOS_OK) {
        return APP_VCOM_ERROR;
    }

    /* 获得设备结构体指针 */
    vcomDev = filp->private_data;

    /* 获得设备主设备号 */
    cdev     = &(vcomDev->appVcomDev);
    devMajor = MAJOR(cdev->dev);

    /* 获得设备在全局变量中的索引值 */
    indexNum = APP_VCOM_GetIndexFromMajorDevId(devMajor);
    APP_VCOM_TRACE_INFO(indexNum, "APP_VCOM_Read, enter. ");
    if (indexNum >= APP_VCOM_MAX_NUM) {
        return APP_VCOM_ERROR;
    }

    vcomCtx = APP_VCOM_GetVcomCtxAddr(indexNum);

    APP_VCOM_TRACE_INFO(indexNum, "APP_VCOM_Read, read count:%u, current_len:%u. ", count,
                        vcomDev->currentLen); /*lint !e559 */
    APP_VCOM_TRACE_INFO(indexNum, "APP_VCOM_Read, f_flags:%d. ", filp->f_flags);
    APP_VCOM_TRACE_INFO(indexNum, "APP_VCOM_Read, wait_event 111, flag:%d. ", vcomDev->readWakeUpFlg);

    if (filp->f_flags & O_NONBLOCK) {
        return APP_VCOM_ERROR;
    }

    /*lint -e730 ;cause:two thread will write global variables */
    if (wait_event_interruptible(vcomDev->readWait, (vcomDev->currentLen != 0))) {
        return -ERESTARTSYS;
    }
    /*lint +e730 ;cause:two thread will write global variables */

    if (vcomDev->currentLen == 0) {
        g_appVcomDebugInfo.readLenErr[indexNum]++;
    }

    APP_VCOM_TRACE_INFO(indexNum, "APP_VCOM_Read, wait_event 222,flag:%d. ", vcomDev->readWakeUpFlg);

    if (APP_VCOM_CopyToUser(buf, vcomCtx, vcomDev, &count, indexNum) != VOS_OK) {
        up(&vcomCtx->appVcomDevEntity->msgSendSem);
        return APP_VCOM_ERROR;
    }

    return (ssize_t)count;
}

ssize_t APP_VCOM_Write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
    VOS_INT             iRst;
    VOS_UINT            devMajor;
    VOS_UINT8           indexNum;
    struct cdev        *cdev    = VOS_NULL_PTR;
    VOS_UINT8          *dataBuf = VOS_NULL_PTR;
    APP_VCOM_DevEntity *vcomDev = VOS_NULL_PTR;
    APP_VCOM_DevCtx    *vcomCtx = VOS_NULL_PTR;

    /* 获得设备结构体指针 */
    vcomDev = filp->private_data;

    /* 获得设备主设备号 */
    cdev     = &(vcomDev->appVcomDev);
    devMajor = MAJOR(cdev->dev);

    /* 获得设备在全局变量中的索引值 */
    indexNum = APP_VCOM_GetIndexFromMajorDevId(devMajor);

    if (indexNum >= APP_VCOM_MAX_NUM) {
        APP_VCOM_ERR_LOG(indexNum, "APP_VCOM_Write, ucIndex fail. ");
        return APP_VCOM_ERROR;
    }

    if (buf == VOS_NULL_PTR) {
        APP_VCOM_TRACE_ERR(indexNum, "APP_VCOM_Write, buf is null ");
        return APP_VCOM_ERROR;
    }

    if ((count > APP_VCOM_MAX_DATA_LENGTH) || (count == 0)) {
        APP_VCOM_TRACE_ERR(indexNum, "APP_VCOM_Write, count is %d, it is error", count);
        return APP_VCOM_ERROR;
    }

    /* 获得全局变量地址 */
    vcomCtx = APP_VCOM_GetVcomCtxAddr(indexNum);

    /* 申请内存 */
    dataBuf = kmalloc(count, GFP_KERNEL);
    if (dataBuf == VOS_NULL_PTR) {
        APP_VCOM_TRACE_ERR(indexNum, "APP_VCOM_Write, kmalloc fail. ");

        return APP_VCOM_ERROR;
    }

    /* buffer清零 */
    (VOS_VOID)memset_s(dataBuf, count, 0x00, (VOS_SIZE_T)count);

    if (copy_from_user(dataBuf, buf, (VOS_ULONG)count)) {
        APP_VCOM_TRACE_ERR(indexNum, "APP_VCOM_Write, copy_from_user fail. ");

        kfree(dataBuf);
        dataBuf = VOS_NULL_PTR;
        return APP_VCOM_ERROR;
    }

#if (FEATURE_ON == FEATURE_VCOM_EXT)
    if (APP_VCOM_ProcAgpsPortCache_InWrite(dataBuf, count, indexNum) == VOS_TRUE) {
        return (ssize_t)count; /*lint !e429*/
    }
#endif

    /* 调用回调函数处理buf中的AT码流 */
    if (vcomCtx->sendUlAtFunc == VOS_NULL_PTR) {
        APP_VCOM_TRACE_ERR(indexNum, "APP_VCOM_Write, pSendUlAtFunc is null. ");
        kfree(dataBuf);
        dataBuf = VOS_NULL_PTR;
        return APP_VCOM_ERROR;
    }

    mutex_lock(&vcomDev->mutex);

    iRst = vcomCtx->sendUlAtFunc(indexNum, dataBuf, (VOS_UINT32)count);
    if (iRst != VOS_OK) {
        g_appVcomDebugInfo.atCallBackErr[indexNum]++;
        APP_VCOM_TRACE_ERR(indexNum, "APP_VCOM_Write, AT_RcvCmdData fail. ");
        mutex_unlock(&vcomDev->mutex);
        kfree(dataBuf);
        dataBuf = VOS_NULL_PTR;

        return APP_VCOM_ERROR;
    }

    mutex_unlock(&vcomDev->mutex);

    APP_VCOM_TRACE_INFO(indexNum, "APP_VCOM_Write, write %d bytes, AT_RcvCmdData Success.", count);

    /* 释放内存 */
    kfree(dataBuf);
    dataBuf = VOS_NULL_PTR;

    return (ssize_t)count;
}

unsigned int APP_VCOM_Poll(struct file *fp, struct poll_table_struct *wait)
{
    APP_VCOM_DevEntity *vcomDev = VOS_NULL_PTR;
    unsigned int        mask    = 0;

    struct cdev *cdev;
    VOS_UINT     devMajor;
    VOS_UINT8    indexNum;

    vcomDev = fp->private_data;

    cdev     = &(vcomDev->appVcomDev);
    devMajor = MAJOR(cdev->dev);
    indexNum = APP_VCOM_GetIndexFromMajorDevId(devMajor);

    poll_wait(fp, &vcomDev->readWait, wait);

    if (vcomDev->currentLen != 0) {
        mask |= POLLIN | POLLRDNORM;
    }

    APP_VCOM_TRACE_INFO(indexNum, "APP_VCOM_Poll, mask = %d. ", mask);

    return mask;
}
/*
 * 功能描述: 检查内存的有效性
 */
LOCAL VOS_UINT32 APP_VCOM_CheckMemAvailable(APP_VCOM_DevIndexUint8 devIndex, APP_VCOM_DevEntity *vcomDev)
{
    if (APPVCOM_DYNAMIC_MALLOC_MEMORY(devIndex) && (vcomDev->appVcomMem == VOS_NULL_PTR)) {
        return VOS_FALSE;
    }

    /* 队列满则直接返回 */
    /*lint -e661*/
    if (g_appVcomCogfigTab[devIndex].appVcomMemSize == vcomDev->currentLen) {
    /*lint +e661*/
        if (vcomDev->isDeviceOpen == VOS_TRUE) {
            APP_VCOM_ERR_LOG(devIndex, "APP_VCOM_Send: VCOM MEM FULL. ");
        }

        /*lint -e661*/
        g_appVcomDebugInfo.memFullErr[devIndex]++;
        /*lint +e661*/
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

/*
 * 功能描述: APP_VCOM_Send预处理
 */
VOS_UINT32 APP_VCOM_SendPreProc(APP_VCOM_DevIndexUint8 devIndex, VOS_UINT32 length, VOS_UINT32 *appFlag)
{
    APP_VCOM_DevCtx *vcomCtx = VOS_NULL_PTR;
    APP_VCOM_DevEntity *vcomDev = VOS_NULL_PTR;

    *appFlag = VOS_FALSE;
    if (devIndex >= APP_VCOM_MAX_NUM) {
        g_appVcomDebugInfo.devIndexErr++;
        APP_VCOM_ERR_LOG(devIndex, "APP_VCOM_SendPreProc, enDevIndex is error. ");
        return VOS_ERR;
    }

    vcomCtx = APP_VCOM_GetVcomCtxAddr(devIndex);
    if (vcomCtx == VOS_NULL_PTR) {
        APP_VCOM_TRACE_ERR(devIndex, "APP_VCOM_SendPreProc, pstVcomCtx is null. ");
        return VOS_ERR;
    }

#if ((FEATURE_VCOM_EXT == FEATURE_ON) && (FEATURE_DEBUG_APP_PORT == FEATURE_OFF))
    if (APP_VCOM_IsDebugAppPort(devIndex) == VOS_TRUE) {
        APP_VCOM_TRACE_NORM(devIndex, "APP_VCOM_SendPreProc, DEBUG_APP_PORT not open, port is eng port, not send. ");
        *appFlag = VOS_TRUE;
        return VOS_ERR;
    }
#endif

    vcomDev = APP_VCOM_GetAppVcomDevEntity(devIndex);
    if (vcomDev == VOS_NULL_PTR) {
        g_appVcomDebugInfo.vcomDevErr[devIndex]++;
        APP_VCOM_TRACE_ERR(devIndex, "APP_VCOM_Send, pstVcomDev is null. ");
        return VOS_ERR;
    }

#if (FEATURE_VCOM_EXT == FEATURE_ON)
    APP_VCOM_ProcAgpsPortCache_InSend(devIndex);
#endif

#if (FEATURE_IOT_RAW_DATA == FEATURE_ON)
    APP_VCOM_TRACE_INFO(devIndex, "APP_VCOM_Send, vcomDev->discard:%d", vcomDev->discard);
    if (vcomDev->discard == VOS_TRUE) {
        APP_VCOM_ERR_LOG(devIndex, "APP_VCOM_Send: discard");
        g_appVcomDebugInfo.lostLen += length;
        return VOS_ERR;
    }
#endif

    return VOS_OK;
}

VOS_UINT32 APP_VCOM_Send(APP_VCOM_DevIndexUint8 devIndex, VOS_UINT8 *data, VOS_UINT32 length)
{
    APP_VCOM_DevEntity *vcomDev = VOS_NULL_PTR;
    APP_VCOM_DevCtx *vcomCtx = VOS_NULL_PTR;
    VOS_UINT32 appFlag = VOS_FALSE;
    errno_t         memResult;

    if (APP_VCOM_SendPreProc(devIndex, length, &appFlag) != VOS_OK) {
        return (appFlag == VOS_FALSE) ? VOS_ERR : VOS_OK;
    }

    vcomCtx = APP_VCOM_GetVcomCtxAddr(devIndex);
    vcomDev = APP_VCOM_GetAppVcomDevEntity(devIndex);

    APP_VCOM_TRACE_INFO(devIndex, "APP_VCOM_Send, uslength:%d, current_len:%d. ", length, vcomDev->currentLen);

    /* 获取信号量 */
    down(&vcomDev->msgSendSem);

    if (APP_VCOM_CheckMemAvailable(devIndex, vcomDev) != VOS_TRUE) {
#if (FEATURE_IOT_RAW_DATA == FEATURE_ON)
        if (devIndex == APP_VCOM_DEV_INDEX_IOT) {
            IOT_VCOM_RxFlowCtrl(vcomDev, vcomCtx->dstPortId);
        }
#endif
        up(&vcomDev->msgSendSem);
        return VOS_ERR;
    }

    /* 发送数据大于剩余Buffer大小 */
    /*lint -e661*/
    if (length > (g_appVcomCogfigTab[devIndex].appVcomMemSize - vcomDev->currentLen)) {
        /*lint +e661*/
        APP_VCOM_TRACE_NORM(devIndex, "APP_VCOM_Send: data more than Buffer. ");

        if (vcomDev->isDeviceOpen == VOS_TRUE) {
            APP_VCOM_ERR_LOG(devIndex, "APP_VCOM_Send: VCOM MEM FULL. ");
        }

        /*lint -e661*/
        length = g_appVcomCogfigTab[devIndex].appVcomMemSize - (VOS_UINT32)vcomDev->currentLen;
        /*lint +e661*/
    }

    /* 复制到BUFFER */
    if (length > 0) {
        /*lint -save -e661*/
        memResult = memcpy_s(vcomDev->appVcomMem + vcomDev->currentLen,
                             g_appVcomCogfigTab[devIndex].appVcomMemSize - (VOS_UINT32)vcomDev->currentLen,
                             data,
                             length);
        TAF_MEM_CHK_RTN_VAL(memResult, g_appVcomCogfigTab[devIndex].appVcomMemSize - (VOS_UINT32)vcomDev->currentLen,
                            length);
        /*lint -restore +e661*/
    }
    vcomDev->currentLen += length;
#if (FEATURE_IOT_RAW_DATA == FEATURE_ON)
    if (devIndex == APP_VCOM_DEV_INDEX_IOT) {
        IOT_VCOM_RxFlowCtrl(vcomDev, vcomCtx->dstPortId);
    }
#endif
    APP_VCOM_TRACE_INFO(devIndex, "APP_VCOM_Send, written %d byte(s), new len: %d. ", length, vcomDev->currentLen);

    APP_VCOM_TRACE_INFO(devIndex, "APP_VCOM_Send, IsDeviceOpen: %d. ", vcomDev->isDeviceOpen);
    if (vcomDev->isDeviceOpen == VOS_TRUE) {
        __pm_wakeup_event(&vcomDev->rdWakeLock, APP_VCOM_READ_WAKE_LOCK_LEN);
    }

    /* 释放信号量 */
    up(&vcomDev->msgSendSem);
    wake_up_interruptible(&vcomDev->readWait);

    if (vcomDev->currentLen == 0) {
        /*lint -e661*/
        g_appVcomDebugInfo.sendLenErr[devIndex]++;
        /*lint +e661*/
    }

    APP_VCOM_TRACE_INFO(devIndex, "APP_VCOM_Send, wakeup. ");
    return VOS_OK;
}

long APP_VCOM_Ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
#if (FEATURE_IOT_RAW_DATA == FEATURE_ON)
    return (long)IOT_VCOM_Ioctl(f, cmd, arg);
#endif
    return -EOPNOTSUPP;
}

#if (FEATURE_ON == FEATURE_VCOM_EXT)

VOS_UINT32 APP_VCOM_ProcAgpsPortCache_InWrite(VOS_UINT8 *dataBuf, size_t count, VOS_UINT8 indexNum)
{
    APP_VCOM_DevCtx *vcomCtx;

    vcomCtx = APP_VCOM_GetVcomCtxAddr(indexNum);

    if ((indexNum == APP_VCOM_DEV_INDEX_9) || (indexNum == APP_VCOM_DEV_INDEX_12)) {
        down(&vcomCtx->appVcomDevEntity->wrtSem);

        if (vcomCtx->appVcomDevEntity->wrtBuffer != VOS_NULL_PTR) {
            APP_VCOM_TRACE_NORM(indexNum, "APP_VCOM_Write: free buff. ");

            kfree(vcomCtx->appVcomDevEntity->wrtBuffer);
            vcomCtx->appVcomDevEntity->wrtBuffer    = VOS_NULL_PTR;
            vcomCtx->appVcomDevEntity->wrtBufferLen = 0;
        }

        if (vcomCtx->sendUlAtFunc == VOS_NULL_PTR) {
            APP_VCOM_TRACE_NORM(indexNum, "APP_VCOM_Write: save buff. ");

            vcomCtx->appVcomDevEntity->wrtBuffer    = dataBuf;
            vcomCtx->appVcomDevEntity->wrtBufferLen = count;
            up(&vcomCtx->appVcomDevEntity->wrtSem);

            return VOS_TRUE;
        }

        up(&vcomCtx->appVcomDevEntity->wrtSem);
    }

    return VOS_FALSE;
} /*lint !e429*/

VOS_VOID APP_VCOM_ProcAgpsPortCache_InSend(APP_VCOM_DevIndexUint8 devIndex)
{
    APP_VCOM_DevEntity *vcomDev;
    APP_VCOM_DevCtx    *vcomCtx;

    vcomCtx = APP_VCOM_GetVcomCtxAddr(devIndex);

    /* 获得设备实体指针 */
    vcomDev = APP_VCOM_GetAppVcomDevEntity(devIndex);

    if ((devIndex == APP_VCOM_DEV_INDEX_9) || (devIndex == APP_VCOM_DEV_INDEX_12)) {
        down(&vcomDev->wrtSem);

        if (vcomDev->wrtBuffer != VOS_NULL_PTR) {
            if (vcomCtx->sendUlAtFunc != VOS_NULL_PTR) {
                APP_VCOM_TRACE_NORM(devIndex, "APP_VCOM_Send: handle buff. ");

                (VOS_VOID)vcomCtx->sendUlAtFunc(devIndex, vcomDev->wrtBuffer, vcomDev->wrtBufferLen);

                kfree(vcomDev->wrtBuffer);
                vcomDev->wrtBuffer    = VOS_NULL_PTR;
                vcomDev->wrtBufferLen = 0;
            }
        }

        up(&vcomDev->wrtSem);
    }
}
#endif

VOS_VOID APP_VCOM_ShowDebugInfo(VOS_VOID)
{
    int i;
    PS_PRINTF_INFO("App Vcom Debug Info:\n");
    PS_PRINTF_INFO("Index Err: %d\n", g_appVcomDebugInfo.devIndexErr);

#if (FEATURE_IOT_RAW_DATA == FEATURE_ON)
    PS_PRINTF_INFO("AppVcom Start flow control Num:         %d\r\n", g_appVcomDebugInfo.startFlowCtrl);
    PS_PRINTF_INFO("AppVcom Stop flow control Num:  %d\r\n", g_appVcomDebugInfo.stopFlowCtrl);
    PS_PRINTF_INFO("AppVcom ulMaxLen:  %d\r\n", g_appVcomDebugInfo.maxLen);
    PS_PRINTF_INFO("AppVcom ulLostLen:  %d\r\n", g_appVcomDebugInfo.lostLen);
    PS_PRINTF_INFO("AppVcom ulTotalRcvLen:  %d\r\n", g_appVcomDebugInfo.totalRcvLen);
    PS_PRINTF_INFO("AppVcom ulTotalSendLen:  %d\r\n", g_appVcomDebugInfo.totalSendLen);
    PS_PRINTF_INFO("AppVcom ulTotalRdLen:  %d\r\n", g_appVcomDebugInfo.totalRdLen);
    PS_PRINTF_INFO("AppVcom ulTotalRdoutLen:  %d\r\n", g_appVcomDebugInfo.totalRdoutLen);
    PS_PRINTF_INFO("AppVcom ulRdCopyErr:  %d\r\n", g_appVcomDebugInfo.rdCopyErr);
    PS_PRINTF_INFO("AppVcom ulFlowCtrlRpt:  %d\r\n", g_appVcomDebugInfo.flowCtrlRpt);
#endif

    for (i = 0; i < APP_VCOM_MAX_NUM; i++) {
        PS_PRINTF_INFO("AppVcom[%d] Callback Function Return Err Num: %d\n", i, g_appVcomDebugInfo.atCallBackErr[i]);
        PS_PRINTF_INFO("AppVcom[%d] Mem Full Num:                     %d\n", i, g_appVcomDebugInfo.memFullErr[i]);
        PS_PRINTF_INFO("AppVcom[%d] Read Data Length = 0 Num:         %d\n", i, g_appVcomDebugInfo.readLenErr[i]);
        PS_PRINTF_INFO("AppVcom[%d] Send Data Length = 0 Num:         %d\n", i, g_appVcomDebugInfo.sendLenErr[i]);
        PS_PRINTF_INFO("AppVcom[%d] Get App Vcom Dev Entity Err Num:  %d\n", i, g_appVcomDebugInfo.vcomDevErr[i]);
    }
}

#if (VOS_WIN32 == VOS_OS_VER)

VOS_VOID APP_VCOM_FreeMem(VOS_VOID)
{
    APP_VCOM_DevCtx    *vcomCtx = VOS_NULL_PTR;
    VOS_UINT32          indexNum;
    APP_VCOM_DevEntity *vcomDevp = VOS_NULL_PTR;

    for (indexNum = 0; indexNum < APP_VCOM_MAX_NUM; indexNum++) {
#if (FEATURE_ON == FEATURE_VCOM_EXT)
#if (FEATURE_OFF == FEATURE_DEBUG_APP_PORT)
        if (APP_VCOM_IsDebugAppPort(indexNum) == VOS_TRUE) {
            continue;
        }
#endif
#endif

        vcomCtx = APP_VCOM_GetVcomCtxAddr(indexNum);

        vcomDevp = vcomCtx->appVcomDevEntity;

        if (APPVCOM_STATIC_MALLOC_MEMORY(indexNum)) {
            if (vcomDevp->appVcomMem != VOS_NULL_PTR) {
                free(vcomDevp->appVcomMem);
                vcomDevp->appVcomMem = VOS_NULL_PTR;
            }
        }

        if (vcomCtx->appVcomDevEntity != VOS_NULL_PTR) {
            free(vcomCtx->appVcomDevEntity);
            vcomCtx->appVcomDevEntity = VOS_NULL_PTR;
        }
    }
}
#endif

VOS_VOID APP_VCOM_SendDebugNvCfg(VOS_UINT32 portIdMask1, VOS_UINT32 portIdMask2, VOS_UINT32 debugLevel)
{
    g_appVcomDebugCfg.portIdMask1 = portIdMask1;
    g_appVcomDebugCfg.portIdMask2 = portIdMask2;
    g_appVcomDebugCfg.debugLevel  = debugLevel;
}

VOS_VOID APP_VCOM_MNTN_LogPrintf(VOS_UINT32 lvl, VOS_CHAR *pcFmt, ...)
{
    VOS_CHAR   acBuf[APP_VCOM_TRACE_BUF_LEN] = {0};
    VOS_UINT32 printLength                   = 0;

    /* 格式化输出BUFFER */
    /*lint -e713 -e507 -e530*/
    APP_VCOM_LOG_FORMAT(printLength, acBuf, APP_VCOM_TRACE_BUF_LEN, pcFmt);
    /*lint +e713 +e507 +e530*/

    if (lvl == APP_VCOM_TRACE_LEVEL_ERR) {
        APP_VCOM_PR_LOGE("%s", acBuf);
    } else if ((lvl == APP_VCOM_TRACE_LEVEL_INFO) || (lvl == APP_VCOM_TRACE_LEVEL_NORM)) {
        APP_VCOM_PR_LOGI("%s", acBuf);
    } else {
    }
}

#if (VOS_LINUX == VOS_OS_VER)
#if (FEATURE_ON == FEATURE_DELAY_MODEM_INIT)
#ifndef CONFIG_HISI_BALONG_MODEM_MODULE
module_init(APP_VCOM_Init);
#endif
#endif
#endif

