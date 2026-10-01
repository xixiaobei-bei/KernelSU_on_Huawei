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




#include <product_config.h>
#include <mdrv.h>
#include <mdrv_diag_system.h>
#include "diag_msgbbp.h"
#include "diag_msgphy.h"
#include "diag_msgmsp.h"
#include "diag_debug.h"
#include "msp_diag_comm.h"
#include "soc_socp_adapter.h"
#include "diag_acore_common.h"
#include "bsp_version.h"

#include <linux/mtd/hisi_nve_interface.h>
#include "adrv.h"

#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/of_reserved_mem.h>

#define    THIS_FILE_ID        MSP_FILE_ID_DIAG_MSGBBP_C

DIAG_TRANS_HEADER_STRU g_stBbpTransHead = {{VOS_NULL, VOS_NULL}, 0};


DIAG_BBP_DS_ADDR_INFO_STRU g_stBbpDsAddrInfo={DIAG_BBP_DS_ENABLE,BBP_SOCP_SIZE,BBP_SOCP_ADDR};
DIAG_BBP_DS_ADDR_INFO_STRU g_stBbpXdataDsAddrInfo={DIAG_BBP_XDATA_DS_DISABLE ,0,0};
VOS_UINT32 diag_BbpDrxDdrEnable(void);

static int  bbpds_probe(struct platform_device *pdev)
{
    s32 ret;
    struct device *pdevice = &(pdev->dev);

    ret=of_reserved_mem_device_init(pdevice);
    if(ret<0)
    {
         printk("modem_mem_device_init fail!\n" );
    }
    else
    {
        printk("modem_mem_device_init success!\n");
    }

    return 0;

}
static const struct of_device_id bbpds_dev_of_match[] = {
        {.compatible = "hisilion,modem_xmode_region"},
        {},
};
/*lint -save -e785 -e64*/
static struct platform_driver bbpds_driver = {
        .driver = {
                   .name = "modem_xmode_region",
                   .owner = THIS_MODULE,
                   .of_match_table = bbpds_dev_of_match,
        },
        .probe = bbpds_probe,

};
/*lint -restore +e785 +e64*/

DIAG_BBP_PROC_FUN_STRU g_DiagBbpFunc[] = {
    {diag_DrxSampleGetChnSizeProc        ,DIAG_CMD_BBP_SAMPLE_CHNSIZE_REQ       ,0},
};


/*****************************************************************************
 Function Name      : diag_DsSendChanInfoToLrm
 Description        : SOCP通道信息发送给LRM
 Input              : DIAG_FRAME_INFO_STRU *pData
 Output             : None
 Return             : VOS_VOID

*****************************************************************************/
VOS_VOID diag_DsSendChanInfoToLrm(DIAG_FRAME_INFO_STRU *pData)
{
    DIAG_BBP_DS_ADDR_INFO_STRU  *psDrxDsInfo = NULL;
    DIAG_BBP_MSG_A_TRANS_C_STRU *pstInfo;
    DIAG_CMD_DRX_SAMPLE_GET_CHNSIZE_REQ_STRU *psChanEnumInfo;
    VOS_UINT32 ulAddrType = 0;
    VOS_UINT32 ulLen;
    VOS_UINT32 ulRet;

    /*lint -save -e826*/
    psChanEnumInfo = (DIAG_CMD_DRX_SAMPLE_GET_CHNSIZE_REQ_STRU*)((VOS_UINT8*)(pData->aucData) + sizeof(MSP_DIAG_DATA_REQ_STRU));

    /*AP在发送给LRM命令时，需要把数采地址空间信息一起发送过去*/
    ulLen = (VOS_UINT32)sizeof(DIAG_BBP_MSG_A_TRANS_C_STRU)-VOS_MSG_HEAD_LENGTH + pData->ulMsgLen +(VOS_UINT32)sizeof(DIAG_BBP_DS_ADDR_INFO_STRU);
    pstInfo = (DIAG_BBP_MSG_A_TRANS_C_STRU*)VOS_AllocMsg(MSP_PID_DIAG_APP_AGENT, ulLen);
    if(VOS_NULL == pstInfo)
    {
        diag_error("pstInfo VOS_AllocMsg fail\n");
        return;
    }

    ulLen = sizeof(DIAG_FRAME_INFO_STRU)+pData->ulMsgLen;
    (VOS_VOID)VOS_MemCpy_s(&pstInfo->stInfo, ulLen, pData, ulLen);

    psDrxDsInfo = (DIAG_BBP_DS_ADDR_INFO_STRU*)((VOS_UINT8*)pstInfo+sizeof(DIAG_BBP_MSG_A_TRANS_C_STRU)+pData->ulMsgLen);
    ulAddrType  = psChanEnumInfo->eDiagDrxSampleChnSize;
    switch(ulAddrType)
    {

        case DRX_SAMPLE_BBP_DMA_DATA_CHNSIZE:
            psDrxDsInfo->ulAddr     = g_stBbpDsAddrInfo.ulAddr;
            psDrxDsInfo->ulSize     = g_stBbpDsAddrInfo.ulSize;
            psDrxDsInfo->ulenable   = g_stBbpDsAddrInfo.ulenable;
        break;

        case DRX_SAMPLE_BBP_CDMA_DATA_CHNSIZE:
            psDrxDsInfo->ulAddr     = g_stBbpXdataDsAddrInfo.ulAddr;
            psDrxDsInfo->ulSize     = g_stBbpXdataDsAddrInfo.ulSize;
            psDrxDsInfo->ulenable   = g_stBbpXdataDsAddrInfo.ulenable;
            atfd_hisi_service_access_register_smc( ACCESS_REGISTER_FN_MAIN_ID, (VOS_UINT64)g_stBbpXdataDsAddrInfo.ulAddr,
                (VOS_UINT64)g_stBbpXdataDsAddrInfo.ulSize, ACCESS_REGISTER_FN_SUB_ID_DDR_MODEM_SEC);
        break;

        default:
            break;
    }

    pstInfo->ulSenderPid = MSP_PID_DIAG_APP_AGENT;
    pstInfo->ulReceiverPid  = MSP_PID_DIAG_AGENT;
    pstInfo->ulMsgId        = DIAG_MSG_BBP_A_TRANS_C_REQ;
    ulRet = VOS_SendMsg(MSP_PID_DIAG_APP_AGENT, pstInfo);
    if(ulRet)
    {
        diag_error("VOS_SendMsg to LRM fail, ulAddrType=0x%x, ulRet=0x%x\n", ulAddrType, ulRet);
        return;
    }

    return;
}


/*****************************************************************************
 Function Name      : diag_DrxSampleGetChnSizeProc
 Description        : 获取SOCP通道信息
 Input              : VOS_UINT8* pstReq
 Output             : None
 Return             : VOS_VOID

*****************************************************************************/
VOS_UINT32 diag_DrxSampleGetChnSizeProc(DIAG_FRAME_INFO_STRU *pData)
{
    mdrv_diag_PTR(EN_DIAG_PTR_BBP_GET_CHAN_SIZE_PROC, 1, pData->ulCmdId, 0);

    if(pData->ulMsgLen < sizeof(MSP_DIAG_DATA_REQ_STRU) + sizeof(DIAG_CMD_DRX_SAMPLE_GET_CHNSIZE_REQ_STRU))
    {
        diag_error("ulMsgLen is too short:0x%x\n", pData->ulMsgLen);
        return ERR_MSP_INVALID_PARAMETER;
    }

    diag_DsSendChanInfoToLrm(pData);


    return ERR_MSP_SUCCESS;
}

/*****************************************************************************
 Function Name   : diag_BbpMsgProc
 Description     : MSP bbp部分消息处理函数
 Input           : None
 Output          : None
 Return          : None
 History         :

*****************************************************************************/
VOS_UINT32 diag_BbpMsgProc(DIAG_FRAME_INFO_STRU *pData)
{
    VOS_UINT32 ulRet = ERR_MSP_INVALID_PARAMETER ;
    DIAG_CMD_DRX_SAMPLE_COMM_CNF_STRU stCnfDrxSample;
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = {0};
    VOS_UINT32 i ;
    VOS_UINT32 ulLen;
    DIAG_BBP_MSG_A_TRANS_C_STRU *pstInfo;

    mdrv_diag_PTR(EN_DIAG_PTR_BBP_MSG_PROC, 1, pData->ulCmdId, 0);

    if(DIAG_MSG_TYPE_BBP != pData->stID.pri4b)
    {
        diag_error("Rcv Error MsgId 0x%x\n",pData->ulCmdId);
        return ulRet;
    }

    for(i = 0; i< sizeof(g_DiagBbpFunc)/sizeof(g_DiagBbpFunc[0]);i++)
    {
        if(g_DiagBbpFunc[i].ulCmdId == pData->ulCmdId)
        {
            g_DiagBbpFunc[i].ulReserve ++;
            ulRet = g_DiagBbpFunc[i].pFunc(pData);
            return ulRet;
        }
    }

    /* 2/3/4G 中PAM Trigger命令直接由A核转发 */
    if((DIAG_CMD_BBP_SAMPLE_PAM_TRIGGER_CFG == pData->ulCmdId) || (DIAG_CMD_BBP_SAMPLE_PAM_TRIGGER_START == pData->ulCmdId))
    {
        return diag_TransReqProcEntry(pData, &g_stBbpTransHead);
    }

    if((DIAG_MODE_1X == pData->stID.mode4b) || (DIAG_MODE_HRPD == pData->stID.mode4b))
    {
        ulRet = ERR_MSP_INVALID_PARAMETER;
        goto DIAG_ERROR;
    }

    ulLen = (VOS_UINT32)sizeof(DIAG_BBP_MSG_A_TRANS_C_STRU)-VOS_MSG_HEAD_LENGTH + pData->ulMsgLen;
    pstInfo = (DIAG_BBP_MSG_A_TRANS_C_STRU*)VOS_AllocMsg(MSP_PID_DIAG_APP_AGENT, ulLen);
    if(VOS_NULL == pstInfo)
    {
        ulRet = ERR_MSP_MALLOC_FAILUE;
        goto DIAG_ERROR;
    }
    pstInfo->ulSenderPid   = MSP_PID_DIAG_APP_AGENT;
    ulLen = sizeof(DIAG_FRAME_INFO_STRU)+pData->ulMsgLen;

    /*5G BBP采数命令通过NRM发送给HL1C*/
    if(DIAG_MODE_NR == pData->stID.mode4b)
    {
        pstInfo->ulReceiverPid = MSP_PID_DIAG_NRM_AGENT;
        pstInfo->ulMsgId       = DIAG_MSG_BBP_A_TRANS_NRM_REQ;
    }
    else/*其他模式(包括EasyRF)采数命令通过LRM发送*/
    {
        pstInfo->ulReceiverPid = MSP_PID_DIAG_AGENT;
        pstInfo->ulMsgId       = DIAG_MSG_BBP_A_TRANS_C_REQ;
    }

    (VOS_VOID)VOS_MemCpy_s(&pstInfo->stInfo, ulLen, pData, ulLen);
    ulRet = VOS_SendMsg(MSP_PID_DIAG_APP_AGENT, pstInfo);
    if(ulRet)
    {
        goto DIAG_ERROR;
    }

    return ulRet;

DIAG_ERROR:

    stCnfDrxSample.ulRet = ulRet;
    DIAG_MSG_COMMON_PROC(stDiagInfo,stCnfDrxSample,pData);
    stDiagInfo.ulMsgType = DIAG_MSG_TYPE_BBP;

    ulRet = DIAG_MsgReport(&stDiagInfo,&stCnfDrxSample, (VOS_UINT32)sizeof(DIAG_CMD_DRX_SAMPLE_ABLE_CHN_CNF_STRU));

    return ulRet;
}

#define NVME_WRITE                 0
#define NVME_READ                  1
#define NVME_MDMLOG_NUM          285
#define NVME_MDMLOG_SIZE           4
#define NVE_MODULE_NAME_STR   "nvme"

int nvme_mdmlog_write(unsigned int index, const unsigned char *data, unsigned int len)
{
	int ret;
	struct hisi_nve_info_user pinfo={0};

	pinfo.nv_operation = NVME_WRITE;
	pinfo.nv_number = NVME_MDMLOG_NUM;
	pinfo.valid_size = NVME_MDMLOG_SIZE;

	(void)VOS_MemCpy_s(pinfo.nv_data + index, NVME_MDMLOG_SIZE-index, data, len);
    /* coverity[var_decl] */
	ret = hisi_nve_direct_access(&pinfo);
	if(0 != ret)
    {
		diag_error("Write nve MDMLOG failed !\n");
		return VOS_ERROR;
	}
    diag_crit("Write nve MDMLOG success!\n");
    return VOS_OK;
}

static int nvme_mdmlog_read(unsigned int index, unsigned char *buf, unsigned long len)
{
	int ret;
	struct hisi_nve_info_user pinfo={0};

	pinfo.nv_operation = NVME_READ;
	pinfo.nv_number = NVME_MDMLOG_NUM;
	pinfo.valid_size = NVME_MDMLOG_SIZE;

	/* coverity[var_decl] */
	ret = hisi_nve_direct_access(&pinfo);
	if (0 != ret)
    {
		diag_error("Read nve MDMLOG failed !\n");
		return VOS_ERROR;
	}
    diag_crit("Read nve MDMLOG success !\n");
	(void)VOS_MemCpy_s(buf, NVME_MDMLOG_SIZE, pinfo.nv_data + index, (unsigned int)len);

	return VOS_OK;
}

int set_mdmlog_nvme(void)
{
    int ret;
    char mdmlog_buf[NVME_MDMLOG_SIZE];

    VOS_MemSet_s(mdmlog_buf, sizeof(mdmlog_buf), 0, sizeof(mdmlog_buf));
    ret = nvme_mdmlog_read(0, mdmlog_buf, NVME_MDMLOG_SIZE);
    if(ret)
    {
        diag_error("get MDMLOG from_nvme failed.\n");
        return VOS_ERROR;
    }

    if('1' != mdmlog_buf[0])
    {
        mdmlog_buf[0] = '1';
        ret = nvme_mdmlog_write(0, mdmlog_buf, NVME_MDMLOG_SIZE);
        if(ret)
        {
           diag_error("set_mdmlog_to_nvme failed.\n");
           return VOS_ERROR;
        }
    }
    if('1' != mdmlog_buf[2])
    {
        mdmlog_buf[2] = '1';
        ret = nvme_mdmlog_write(0, mdmlog_buf, NVME_MDMLOG_SIZE);
        if(ret)
        {
            diag_error("set_mdmlog_to_nvme failed\n");
            return VOS_ERROR;
        }
    }

    return VOS_OK;
}

/*lint -save -e423 */
/*****************************************************************************
 Function Name   : diag_BbpMsgInit
 Description     : MSP bbp部分初始化
 Input           : None
 Output          : None
 Return          : None
 History         :

*****************************************************************************/
VOS_VOID diag_BbpMsgInit(VOS_VOID)
{
    VOS_UINT32 ulRet;
    const BSP_VERSION_INFO_S *VerInfo;

    /* 创建节点保护信号量, Diag Trans Bbp */
    ulRet = VOS_SmBCreate("DTB", 1, VOS_SEMA4_FIFO,&g_stBbpTransHead.TransSem);
    if(VOS_OK != ulRet)
    {
        diag_error("[init]VOS_SmBCreate failed.\n");
    }

    /* 初始化请求链表 */
    blist_head_init(&g_stBbpTransHead.TransHead);
    (VOS_VOID)diag_BbpDrxDdrEnable();
    /*注册message消息回调*/
    DIAG_MsgProcReg(DIAG_MSG_TYPE_BBP,diag_BbpMsgProc);
    diag_crit("diag modem:modem_reserver define !");

    printk(KERN_ERR"diag modem:modem_reserver define!\n");

    ulRet= platform_driver_register(&bbpds_driver);
    if(VOS_OK != ulRet)
    {
        diag_error("diag_BbpMsgInit bbpds_driver failed.\n");
    }

    VerInfo = bsp_get_version_info();
    if(NULL == VerInfo)
    {
        diag_error("get_version_info fail\n");
        return;
    }

    if(HW_VER_HIONE_UDP_MAGIC == VerInfo->udp_flag) // 通过udp_mask判断硬件形态为udp
    {
        ulRet = set_mdmlog_nvme();
        if(VOS_OK == ulRet)
        {
            diag_crit("diag_BbpMsgInit: set_mdmlog_nvme success.\n");
        }
    }

    return;
}

/*****************************************************************************
* 函 数 名  : socp_logbuffer_sizeparse
*
* 功能描述  : 在代码编译阶段将CMD LINE中的BUFFER大小参数解析出来
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : 无
*****************************************************************************/
VOS_UINT32 diag_BbpDrxDdrEnable(void)
{
    u32   flag;

    flag = mdrv_socp_bbpmemenable();
    if(flag)
    {
        g_stBbpDsAddrInfo.ulenable  = DIAG_BBP_DS_ENABLE;
        g_stBbpDsAddrInfo.ulAddr    = BBP_SOCP_ADDR;
        g_stBbpDsAddrInfo.ulSize    = BBP_SOCP_SIZE;



        diag_crit("Bbp Ds Addr Enable\n");
    }
    else
    {
        g_stBbpDsAddrInfo.ulenable  = 0;
        g_stBbpDsAddrInfo.ulAddr    = 0;
        g_stBbpDsAddrInfo.ulSize    = 0;


        diag_crit("Bbp Ds Addr Disable\n");
    }
    return 0;
}


/*****************************************************************************
* 函 数 名  : modem_cdma_bbpds_reserve_area
*
* 功能描述  : 在代码编译阶段将动态预留内存申请出来
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : 无
*****************************************************************************/
static int modem_cdma_bbpds_reserve_area(struct reserved_mem *rmem)
{
    const char* heapname;
    char *status ;
    int namesize = 0;

    //printk(KERN_ERR"diag modem: modem_cdma_bbpds_reserve_area!\n" );
    diag_crit("diag modem: modem_cdma_bbpds_reserve_area!\n");

    status = (char *)of_get_flat_dt_prop(rmem->fdt_node, "status", NULL);
    if (status && (strncmp(status, "ok", strlen("ok")) != 0))
    {
         //printk(KERN_ERR"status is not ok [%s]!\n",status);
         diag_crit("status is not ok\n");
         return 0;
    }


    heapname = of_get_flat_dt_prop(rmem->fdt_node, "region-name", &namesize);
    if (!heapname || (namesize <= 0)) {
        /*
        printk(KERN_ERR"%s %s %d, no 'region-name' property namesize=%d\n",
        __FILE__, __FUNCTION__,__LINE__, namesize);
        */
        diag_error("no 'region-name' property namesize=%d\n", namesize);

        return 0;
    }

    g_stBbpXdataDsAddrInfo.ulenable         = DIAG_BBP_XDATA_DS_ENABLE;
    g_stBbpXdataDsAddrInfo.ulSize           = (VOS_UINT32)rmem->size;
    g_stBbpXdataDsAddrInfo.ulAddr           = rmem->base;
    /*
    printk(KERN_ERR"[%s]:diag modem: kernel reserved buffer is useful, base 0x%llx, size is 0x%llx\n",
                 __FUNCTION__, rmem->base, rmem->size );
    */
    diag_crit("diag modem: kernel reserved buffer is useful, base 0x%llx, size is 0x%llx\n",rmem->base, rmem->size);
    return 0;
}

/*lint -e611 -esym(611,*)*/
RESERVEDMEM_OF_DECLARE(modem_cdma_bbpds_region, "modem_xmode_region", modem_cdma_bbpds_reserve_area);
/*lint -e611 +esym(611,*)*/




