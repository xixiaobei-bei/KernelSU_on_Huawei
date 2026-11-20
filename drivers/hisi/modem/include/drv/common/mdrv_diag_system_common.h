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

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#ifndef __MDRV_DIAG_SYSTEM_COMMON_H__
#define __MDRV_DIAG_SYSTEM_COMMON_H__


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#include <product_config.h>
#include <mdrv_socp_common.h>

typedef struct
{
    unsigned int                  ulHisiMagic;   /*"HISI"*/
    unsigned int                  ulDataLen;      /*数据长度*/
    unsigned char                   pucData[0];
}SCM_CODER_SRC_PACKET_HEADER_STRU;

#define SCM_HISI_HEADER_LENGTH              (sizeof(SCM_CODER_SRC_PACKET_HEADER_STRU))

typedef struct
{
    void        *pHeader;           /* 之前获取的编码源buffer的首地址(虚拟地址) */
    void        *pSrc;              /* 将要拷贝的数据缓冲区地址 */
    unsigned int      uloffset;           /* 将要拷贝到的偏移地址 */
    unsigned int      ulLen;              /* 将要拷贝的数据长度 */
}SCM_CODER_SRC_MEMCPY_STRU;


typedef enum
{
    EN_DIAG_PTR_BEGIN = 0,
    EN_DIAG_PTR_PPM_READDATA    = 1,
    EN_DIAG_PTR_CPM_COMRCV      = 2,
    EN_DIAG_PTR_SCM_SOFTDECODE  = 3,
    EN_DIAG_PTR_SCM_SELFTASK    = 4,
    EN_DIAG_PTR_SCM_RCVDATA     = 5,
    EN_DIAG_PTR_SCM_RCVDATA_SUCCESS = 6,
    EN_DIAG_PTR_SCM_DISPATCH    = 7,
    EN_DIAG_PTR_MSP_SERVICE_1   = 8,
    EN_DIAG_PTR_MSP_SERVICE_2   = 9,
    EN_DIAG_PTR_SERVICE_IN      = 0xA,
    EN_DIAG_PTR_SERVICE_1       = 0xB,
    EN_DIAG_PTR_MESSAGE_IN      = 0xC,
    EN_DIAG_PTR_MESSAGE_PROC    = 0xD,
    EN_DIAG_PTR_MSGMSP_IN       = 0xE,
    EN_DIAG_PTR_MSGMSP_TRANS    = 0xF,
    EN_DIAG_PTR_MSGMSP_PS_IN    = 0x10,
    EN_DIAG_PTR_MSGMSP_PS_OUT   = 0x11,
    EN_DIAG_PTR_MSGMSP_CONN_IN  = 0x12,
    EN_DIAG_PTR_MSGMSP_DISCONN_IN=0x13,
    EN_DIAG_PTR_MESSAGE_REPORT  = 0x14,
    EN_DIAG_PTR_SERVICE_PACKETDATA=0x15,
    EN_DIAG_PTR_SEND_CODER_SRC  = 0x16,
    EN_DIAG_PTR_SCM_CODER_SRC   = 0x17,
    EN_DIAG_PTR_SCM_CODER_DST_CB= 0x18,
    EN_DIAG_PTR_SCM_SENDTOUDI   = 0x19,
    EN_DIAG_PTR_CPM_COMSEND     = 0x1A,
    EN_DIAG_PTR_PPM_PORTSEND    = 0x1B,
    EN_DIAG_PTR_BSP_MSG_PROC    = 0x1C,
    EN_DIAG_PTR_TRANS_BBP_PROC  = 0x1D,
    EN_DIAG_PTR_BBP_MSG_PROC    = 0x1E,
    EN_DIAG_PTR_BBP_MSG_PROC_CNF        = 0x1F,
    EN_DIAG_PTR_BBP_GET_CHAN_SIZE_PROC  = 0x20,
    EN_DIAG_PTR_BBP_GET_CHAN_SIZE_CNF   = 0x21,
    EN_DIAG_PTR_BBP_SAMPLE_GEN_PROC     = 0x22,
    EN_DIAG_PTR_BBP_SAMPLE_GEN_CNF      = 0x23,
    EN_DIAG_PTR_BBP_GET_ADDR_PROC       = 0x24,
    EN_DIAG_PTR_BBP_GET_ADDR_PROC_CNF   = 0x25,
    EN_DIAG_PTR_BBP_GET_VERSION_PROC    = 0x26,
    EN_DIAG_PTR_BBP_GET_VERSION_CNF     = 0x27,
    EN_DIAG_PTR_BBP_ABLE_CHAN_PROC      = 0x28,
    EN_DIAG_PTR_BBP_ABLE_CHAN_CNF       = 0x29,
    EN_DIAG_PTR_APP_TRANS_PHY_PROC      = 0x2A,
    EN_DIAG_PTR_DSP_MSG_PROC            = 0x2B,
    EN_DIAG_PTR_DSP_MSG_CNF             = 0x2C,
    EN_DIAG_PTR_APP_TRANS_HIFI_PROC     = 0x2D,
    EN_DIAG_PTR_HIFI_MSG_PROC           = 0x2E,
    EN_DIAG_PTR_HIFI_MSG_CNF            = 0x2F,
    EN_DIAG_PTR_SOCP_VOTE_CNF_MSGPROC   = 0x30,
    EN_DIAG_PTR_APP_TRANS_MSG_PROC      = 0x31,
    EN_DIAG_PTR_FAIL_CMD_CNF            = 0x32,
    EN_DIAG_PTR_PRINT_CFG_PROC          = 0x33,
    EN_DIAG_PTR_PRINT_CFG_SUCESS_CNF    = 0x34,
    EN_DIAG_PTR_PRINT_CFG_FAIL_CNF      = 0x35,
    EN_DIAG_PTR_LAYER_CFG_PROC          = 0x36,
    EN_DIAG_PTR_LAYER_CFG_SUCESS_CNF    = 0x37,
    EN_DIAG_PTR_LAYER_CFG_FAIL_CNF      = 0x38,
    EN_DIAG_PTR_AIR_CFG_PROC            = 0x39,
    EN_DIAG_PTR_AIR_CFG_SUCESS_CNF      = 0x3A,
    EN_DIAG_PTR_AIR_CFG_FAIL_CNF        = 0x3b,
    EN_DIAG_PTR_EVENT_CFG_PROC          = 0x3C,
    EN_DIAG_PTR_EVENT_CFG_SUCESS_CNF    = 0x3D,
    EN_DIAG_PTR_EVENT_CFG_FAIL_CNF      = 0x3E,
    EN_DIAG_PTR_MSG_CFG_PROC            = 0x3F,
    EN_DIAG_PTR_MSG_CFG_SUCESS_CNF      = 0x40,
    EN_DIAG_PTR_MSG_CFG_FAIL_CNF        = 0x41,
    EN_DIAG_PTR_GTR_CFG_PROC            = 0x42,
    EN_DIAG_PTR_GTR_CFG_SUCESS_CNF      = 0x43,
    EN_DIAG_PTR_GTR_CFG_FAIL_CNF        = 0x44,
    EN_DIAG_PTR_GUGTR_CFG_PROC          = 0x45,
    EN_DIAG_PTR_GET_TIME_STAMP      = 0x46,
    EN_DIAG_PTR_GET_TIME_STAMP_CNF  = 0x47,
    EN_DIAG_PTR_GET_MODEM_NUM       = 0x48,
    EN_DIAG_PTR_GET_MODEM_NUM_CNF   = 0x49,
    EN_DIAG_PTR_GET_PID_TABLE       = 0x4A,
    EN_DIAG_PTR_GET_PID_TABLE_CNF   = 0x4B,
    EN_DIAG_PTR_TRANS_CNF_PROC      = 0x4C,
    EN_DIAG_PTR_FS_QUERY_PROC       = 0x4D,
    EN_DIAG_PTR_FS_SCAN_PROC        = 0x4E,
    EN_DIAG_PTR_FS_MAKE_DIR_PROC    = 0x4F,
    EN_DIAG_PTR_FS_OPEN_PROC        = 0x50,
    EN_DIAG_PTR_FS_IMPORT_PROC      = 0x51,
    EN_DIAG_PTR_FS_EXOPORT_PROC     = 0x52,
    EN_DIAG_PTR_FS_DELETE_PROC      = 0x53,
    EN_DIAG_PTR_FS_SPACE_PROC       = 0x54,
    EN_DIAG_PTR_NV_RD_PROC          = 0x55,
    EN_DIAG_PTR_NV_RD_SUCESS_CNF    = 0x56,
    EN_DIAG_PTR_NV_RD_FAIL_CNF      = 0x57,
    EN_DIAG_PTR_GET_NV_LIST_PROC    = 0x58,
    EN_DIAG_PTR_GET_NV_LIST_SUCESS_CNF = 0x59,
    EN_DIAG_PTR_GET_NV_LIST_FAIL_CNF   = 0x5A,
    EN_DIAG_PTR_GET_NV_RESUME_LIST_PROC         = 0x5B,
    EN_DIAG_PTR_GET_NV_RESUME_LIST_SUCESS_CNF  = 0x5C,
    EN_DIAG_PTR_GET_NV_RESUME_LIST_FAIL_CNF    = 0x5D,
    EN_DIAG_PTR_NV_WR_PROC          = 0x5E,
    EN_DIAG_PTR_NV_WR_SUCESS_CNF    = 0x5F,
    EN_DIAG_PTR_NV_WR_FAIL_CNF      = 0x60,
    EN_DIAG_PTR_AUTH_NV_CFG         = 0x61,
    EN_DIAG_PTR_NV_AUTH_PROC        = 0x62,
    EN_DIAG_PTR_NV_AUTH_FAIL_CNF    = 0x63,


    /* 失败流程的点 */
    EN_DIAG_PTR_ERR_BEGIN = 0x100,
    EN_DIAG_PTR_PPM_ERR1,
    EN_DIAG_PTR_PPM_ERR2,
    EN_DIAG_PTR_CPM_ERR1,
    EN_DIAG_PTR_CPM_ERR2,
    EN_DIAG_PTR_CPM_ERR3,
    EN_DIAG_PTR_SCM_ERR1,
    EN_DIAG_PTR_SCM_ERR2,
    EN_DIAG_PTR_SCM_ERR3,
    EN_DIAG_PTR_MSP_SERVICE_ERR1,
    EN_DIAG_PTR_MSP_SERVICE_ERR2,
    EN_DIAG_PTR_MSP_SERVICE_ERR3,
    EN_DIAG_PTR_DIAG_SERVICE_ERR1,
    EN_DIAG_PTR_DIAG_SERVICE_ERR2,
    EN_DIAG_PTR_DIAG_SERVICE_ERR3,
    EN_DIAG_PTR_DIAG_BUFFER_ERR1,
    EN_DIAG_PTR_DIAG_BUFFER_ERR2,
    EN_DIAG_PTR_MAX
} DIAG_PTR_ID_ENUM;

/* throughput 吞吐率信息*******************************************/

typedef enum
{
    EN_DIAG_THRPUT_DATA_CHN_ENC = 0,    /* 数据通道编码输入端 */
    EN_DIAG_THRPUT_DATA_CHN_PHY,        /* 数据通道写入物理通道 */

    EN_DIAG_THRPUT_DATA_CHN_CB,         /* 数据物理通道回调 */
    EN_DIAG_THRPUT_MAX
} DIAG_THRPUT_ID_ENUM;

/*diag mem**********************************************************************/
#ifdef ENABLE_DIAG_FIX_ADDR

#define DIAG_CODER_SRC_ACORE_CNF_PADDR      (DDR_SOCP_ADDR)
#define DIAG_CODER_SRC_ACORE_CNF_LENGTH     (1024*64)

#define DIAG_CODER_SRC_ACORE_IND_PADDR      (DIAG_CODER_SRC_ACORE_CNF_PADDR+DIAG_CODER_SRC_ACORE_CNF_LENGTH)
#define DIAG_CODER_SRC_ACORE_IND_LENGTH     (1024*1024)

#define DIAG_CODER_SRC_CCORE_CNF_PADDR      (DIAG_CODER_SRC_ACORE_IND_PADDR+DIAG_CODER_SRC_ACORE_IND_LENGTH)
#define DIAG_CODER_SRC_CCORE_CNF_LENGTH     (1024*64)

#define DIAG_CODER_SRC_CCORE_IND_PADDR      (DIAG_CODER_SRC_CCORE_CNF_PADDR+DIAG_CODER_SRC_CCORE_CNF_LENGTH)
#define DIAG_CODER_SRC_CCORE_IND_LENGTH     (1536*1024)


#define DIAG_CORER_SRC_DSP_PADDR            (DIAG_CODER_SRC_CCORE_IND_PADDR+DIAG_CODER_SRC_CCORE_IND_LENGTH)
#define DIAG_CORER_SRC_DSP_LENGTH           (1024*1024)

#define BBP_LOG0_MEM_ADDR                   (DIAG_CORER_SRC_DSP_PADDR+DIAG_CORER_SRC_DSP_LENGTH)
#define BBP_LOG0_MEM_SIZE                   (64*1024)

#define BBP_LOG1_MEM_ADDR                   (BBP_LOG0_MEM_ADDR + BBP_LOG0_MEM_SIZE)
#define BBP_LOG1_MEM_SIZE                   (8*1024)

#define SOCP_CODER_SRC_GUDSP_ADDR            (BBP_LOG1_MEM_ADDR+BBP_LOG1_MEM_SIZE)
#define SOCP_CODER_SRC_GUDSP_LENGTH          (128*1024)

#define SOCP_CODER_SRC_XDSP_ADDR             (SOCP_CODER_SRC_GUDSP_ADDR+SOCP_CODER_SRC_GUDSP_LENGTH)
#define SOCP_CODER_SRC_XDSP_LENGTH           (512*1024)

#define SOCP_CODER_SRC_HIFI_ADDR             (SOCP_CODER_SRC_XDSP_ADDR+SOCP_CODER_SRC_XDSP_LENGTH)
#define SOCP_CODER_SRC_HIFI_LENGTH           (32*1024)

#define SOCP_CODER_SRC_CCORE_HDS_ADDR        (SOCP_CODER_SRC_HIFI_ADDR+SOCP_CODER_SRC_HIFI_LENGTH)
#define SOCP_CODER_SRC_CCORE_HDS_LENGTH      (8*1024)

#define SOCP_CODER_SRC_ACORE_HDS_ADDR        (SOCP_CODER_SRC_CCORE_HDS_ADDR+SOCP_CODER_SRC_CCORE_HDS_LENGTH)
#define SOCP_CODER_SRC_ACORE_HDS_LENGTH      (8*1024)


#define BBP_SOCP_ADDR                       (DDR_SOCP_ADDR+8*1024*1024)
#define BBP_SOCP_SIZE                       (DDR_SOCP_SIZE-8*1024*1024)

#else

#ifdef DIAG_BBP_WRPTR_OFFSET
#define BBP_SOCP_PTR_ADDR               (DDR_SOCP_ADDR)
#define BBP_SOCP_PTR_SIZE               (128)
#define BBP_SOCP_BUS_ADDR               (BBP_SOCP_PTR_ADDR + BBP_SOCP_PTR_SIZE)
#define BBP_SOCP_BUS_SIZE               (16*1024*1024 - BBP_SOCP_PTR_SIZE)
#define BBP_SOCP_ADDR                   (BBP_SOCP_BUS_ADDR + BBP_SOCP_BUS_SIZE)
#define BBP_SOCP_SIZE                   (DDR_SOCP_SIZE - BBP_SOCP_BUS_SIZE - BBP_SOCP_PTR_SIZE)
#else
#define BBP_SOCP_ADDR                   (DDR_SOCP_ADDR)
#define BBP_SOCP_SIZE                   (DDR_SOCP_SIZE)
#endif

#define BBP_BUS_MEM_SIZE                   (64*1024)
#define BBP_LOG1_MEM_SIZE                   (8*1024)

#ifdef DIAG_SYSTEM_5G
#define BBP_5G_BUS_MEM_SIZE                (64*1024)
#define BBP_RFIC0_BUS_MEM_SIZE             (64*1024)
#define BBP_RFIC1_BUS_MEM_SIZE             (64*1024)

/*暂时为了解决编译问题，待XDSP的相关宏定义在主线关掉后删除该ADDR和SIZE*/
#define SOCP_CODER_SRC_XDSP_ADDR           (BBP_SOCP_ADDR)
#define SOCP_CODER_SRC_XDSP_LENGTH         (512*1024)
#endif

#endif

/********************* BBP DS内存配置 **************************
* BBP LOG 0:  64K
* BBP LOG 1:   8K
* BBP DS   :  随机
* TOTAL SIZE: BBPDS + 120K
****************************************************************/


#define BBP_DS_MEM_ADDR                         	(BBP_SOCP_ADDR)
#define BBP_DS_MEM_SIZE                             (BBP_SOCP_SIZE)


/*error no begin**********************************************************************/
#define  ERR_MSP_SUCCESS                (0)
#define  ERR_MSP_WAIT_ASYNC             (1)
#define  ERR_MSP_INVALID_ID             (2)
#define  ERR_MSP_NO_INITILIZATION       (3)
#define  ERR_MSP_NOT_FOUND              (4)
#define  ERR_MSP_FULL                   (7)
#define  ERR_MSP_INSUFFICIENT_BUFFER    (8)
#define  ERR_MSP_CONSUMED               (9)
#define  ERR_MSP_CONTINUE               (10)
#define  ERR_MSP_TOO_SMALL_BUFFER       (11)
#define  ERR_MSP_NOT_FREEE_SPACE        (12)
#define  ERR_MSP_NO_MORE_DATA           (13)
#define  ERR_MSP_MALLOC_FAILUE          (15)
#define  ERR_MSP_NOT_ENOUGH_MEMORY      (16)
#define  ERR_MSP_INVALID_HEAP_ADDR      (17)
#define  ERR_MSP_SHUTDOWN               (18) /* 未能关机*/
#define  ERR_MSP_NOT_CONNECT            (19) /* 未正确连接UE*/
#define  ERR_MSP_BUSY                   (20) /* 系统忙或者使用冲突*/


#define  ERR_MSP_UNAVAILABLE            (50)
#define  ERR_MSP_TIMEOUT                (51)
#define  ERR_MSP_INIT_FAILURE           (52)
#define  ERR_MSP_IO_REGISTER_FAILURE    (53)
#define  ERR_MSP_NO_REGISTER            (54)

#define  ERR_MSP_FLASH_ERASE_ERROR      (100)
#define  ERR_MSP_FLASH_WRITE_ERROR      (101)
#define  ERR_MSP_FLASH_ALIGN_ERROR      (102)             /* sector aligned addresses*/
#define  ERR_MSP_FLASH_PROTECT_ERROR    (103)
#define  ERR_MSP_FLASH_BIN_LOAD_NEED_FIRST_BLOCK    (104) /* while bin file load,req need first block*/
#define  ERR_MSP_DIAG_CMD_SIZE_INVALID              (106)


#define  ERR_MSP_CFG_RAM_SUCCESS         (201)
#define  ERR_MSP_CFG_PART_SUCCESS        (202)
#define  ERR_MSP_CFG_LOG_NOT_ALLOW       (203)
#define  ERR_MSP_DIAG_CYC_CMD_NOT_START  (204)
#define  ERR_MSP_DIAG_MSG_CFG_NOT_SET    (205)
#define  ERR_MSP_QUEUE_FULL              (300)
#define  ERR_MSP_QUEUE_NOT_EXIST         (301)
#define  ERR_MSP_INVALID_TASK_ID         (302)
#define  ERR_MSP_INVALID_TASK_QUEUE      (303)

#define  ERR_MSP_VCOM_WRITE_ERROR        (401)

/* AT错误码，可以和字符串对应*/
/* 该错误码和MSP内部错误码统一定义*/
#define  ERR_MSP_NOT_EXIST               (500) /* 字符串："未实现"   “Undone”*/
#define  ERR_MSP_UNKNOWN                 (501) /* 字符串："未知错误" “unknown error”*/
#define  ERR_MSP_INVALID_PARAMETER       (502) /* 字符串："输入参数非法，包括不在合法范围等错误" "invalid parameters"*/
#define  ERR_MSP_NV_NOT_SUPPORT_ID       (520) /* 字符串："NV不存在""NVIM Not Exist"*/
#define  ERR_MSP_NV_ERROR_READ           (521) /* 字符串："NV读取失败" "Read NVIM Failure"*/
#define  ERR_MSP_NV_NOT_SUPPORT_LENTH    (522) /* 字符串："NV写失败，长度过长""Write Error for Length Overflow"*/
#define  ERR_MSP_NV_BAD_BLOCK            (523) /* 字符串："NV写失败，Flash坏块" "Write Error for Flash Bad Block"*/
#define  ERR_MSP_NV_ERROR_WRITE          (524) /* 字符串："NV写失败，其它错误" "Write Error for Unknown Reason"*/
#define  ERR_MSP_VCTCXO_OVER_HIGH        (525) /* 字符串："设置VCTCXO电压过高" "Higher Voltage"	*/
#define  ERR_MSP_UE_MODE_ERR             (526) /* 字符串："单板模式错误(比如信令模式)"	"UE Mode Error"	*/
#define  ERR_MSP_NOT_SET_CHAN            (527) /* 字符串："没有设置相关信道""Not Set Appointed Channel"	*/
#define  ERR_MSP_OPEN_RFTX_ERROR         (528) /* 字符串："打开发射机失败""Open TX Transmitter Failure"	*/
#define  ERR_MSP_OPEN_RFRX_ERROR         (529) /* 字符串："打开接收机失败""Open RX Transmitter Failure"	*/
#define  ERR_MSP_OPEN_CHAN_ERROR         (530) /* 字符串："通道打开失败""Open Channel Failure"	*/
#define  ERR_MSP_CLOSE_CHAN_ERROR        (531) /* 字符串："通道关闭失败""Close Channel Failure"	*/
#define  ERR_MSP_NOT_SUPPORT             (532) /* 字符串："不支持（如打开通道2的发送通道，DD800项目采用1T2R模式）"	"Not Support"*/
#define  ERR_MSP_INVALID_OP              (533) /* 字符串："非法操作（比如读取特殊NV）""Invalid Operation"	*/
#define  ERR_MSP_ENTER_BAND_ERROR        (534) /* 字符串："输入频段信息无法对应" "Band No Match"	*/
#define  ERR_MSP_SET_CHAN_INFOR_ERROR    (535) /* 字符串："设置信道信息失败" "Set Channel Information Failure"	*/
#define  ERR_MSP_FRE_CHANI_NOTMATCH      (536) /* 字符串："输入频段和信道信息无法组合""Band And Channel Not Combinated"	*/
#define  ERR_MSP_SET_TX_POWER_FAILUE     (537) /* 字符串："设置TX的发射功率失败""Set TX Transmitter Power Error"	*/
#define  ERR_MSP_SET_PA_LEVEL_FAILUE     (538) /* 字符串："设置PA级失败""Set PA Level Failure"	*/
#define  ERR_MSP_NOT_SET_CURRENT_CHAN    (539) /* 字符串："未设置当前信道"	"Not Set Current Channel"*/
#define  ERR_MSP_NOT_ABLETO_RD_APC       (540) /* 字符串："无法读取当前设置的APC值"	"APC Value Can't Be Read"*/
#define  ERR_MSP_SET_APC_ERROR           (541) /* 字符串："设置APC操作失败"	"Write APC Failure"*/
#define  ERR_MSP_RD_APC_ERROR            (542) /* 字符串："读取APC操作失败"	"Read APC Failure"*/
#define  ERR_MSP_SET_LINA_ERROR          (543) /* 字符串："设置LNA失败"	"Set AAGC Failure"*/
#define  ERR_MSP_NOT_OPEN_RXRX           (544) /* 字符串："没有打开接收机"	"RX Transmitter Not Open"*/
#define  ERR_MSP_NOT_OPEN_RXTX           (545) /* 字符串："没有打开发射机"	"TX Transmitter Not Open"*/
#define  ERR_MSP_NO_SIGNAL               (546) /* 字符串："无信号输入或检测不到信号" "No Signal"	*/
#define  ERR_MSP_PHYNUM_LENGTH_ERROR     (547) /* 字符串："物理号长度错误"	"PHY Number Length Error"*/
#define  ERR_MSP_PHYNUM_INVALID          (548) /* 字符串："物理号不合法" "Invalid PHY Number"	*/
#define  ERR_MSP_PHYNUM_TYPE_ERROR       (549) /* 字符串："物理号类型错误""Invalid PHY Type"*/
#define  ERR_MSP_PLATINFOR_NOTABLE_TORD  (550) /* 字符串："无法读取平台信息""Platform Information Can't Be Read"	*/
#define  ERR_MSP_WRT_NOT_UNLOCK          (551) /* 字符串："表示写数据时，数据保护未解锁, 无法操作保护数据"	"Not unlock, write error"*/
#define  ERR_MSP_CALLNUM_OVER20          (552) /* 字符串："输入的号段多于20段"	"Card Number Morethan 20"*/
#define  ERR_MSP_MNC_ENTER_OVERFLOW      (553) /* 字符串："<mnc-digital-num>输入超出范围" "<mnc-digital-num> Enter Error"	*/
#define  ERR_MSP_RD_VER_FAILUE           (554) /* 字符串："版本信息读取失败""Read VERSION Failure"	*//* [false alarm]:alarm */
#define  ERR_MSP_OPERTION_ERROR          (555) /* 字符串："操作失败""Operation Failure"*//* [false alarm]:alarm */
#define  ERR_MSP_PWD_ERR                 (556) /* 字符串: "输入密码错误""Password Error"*//* [false alarm]:alarm */
#define  ERR_MSP_TIME_OUT                (557) /* 字符串: "超时错误" "Overtime"*//* [false alarm]:alarm */
#define  ERR_MSP_NO_MORE_MEMORY	         (558) /* 内存不足 "No Memory"*/
#define  ERR_MSP_SIMM_LOCK               (559) /* 启用了SIMLOCK功能"Sim Lock Active"*/
#define  ERR_MSP_CLOSE_RFTX_ERROR        (560) /* 字符串："关闭发射机失败""Close TX Transmitter Failure"	*/
#define  ERR_MSP_CLOSE_RFRX_ERROR        (561) /* 字符串："关闭接收机失败""Close RX Transmitter Failure"*/
#define  ERR_MSP_NV_DATA_INVALID         (562) /* 字符串："读取的NV项内容非法""NV DATA INVALID"*/
#define  ERR_MSP_AT_CHANNEL_CLOSE        (563) /* DIAG/MUX的AT通道关闭*/
#define  ERR_MSP_AT_CHANNEL_BUSY         (564) /* DIAG/MUX的AT通道BUSY*/
#define  ERR_MSP_OS_MSG_BODY_NULL        (565) /* 接收到的消息体为NULL(本应不为NULL)*/
#define  ERR_MSP_AT_ACK_URC_INVALID      (566) /* AT ACK or URC invalid, such as size is zero*/
#define  ERR_MSP_PS_START_FAILURE        (567) /* START PS FAILURE*/
#define  ERR_MSP_PS_STOP_FAILURE         (568) /* STOP PS FAILURE*/
#define  ERR_MSP_PS_SET_APN_FAILURE      (569) /* SET APN FAILURE*/
#define  ERR_MSP_PS_SET_AUTH_FAILURE     (570) /* SET AUTH FAILURE*/
#define  ERR_MSP_PS_SET_BEARER_TYPE_FAILURE      (571) /* SET BEARER TYPE*/


#define  ERR_MSP_BNADSW_ERROR            (572) /* 字符串: "BAND切换错误"*/
#define	 ERR_MSP_TXWAVE_ERROR	         (573) /* 字符串: "发射波形类型错误"*/
#define  ERR_MSP_AT_FW_TABLEID_EXIST     (574) /* client id 已存在*/
#define  ERR_MSP_START_TIMER_FAIL           (575)

/* add simm error number (周罗俊/67193/2009-08-14)*/
/* 600 ~ 699 should be used by simm*/
#define  ERR_MSP_SIMM_NOT_INIT             (601)          /*SIMM未初始化*/
#define  ERR_MSP_SIMM_WRONG_PARA           (602)          /*因为参数错误，函数执行失败*/
#define  ERR_MSP_SIMM_NOAPP                (603)          /*因为没有可用应用，函数执行失败*/
#define  ERR_MSP_SIMM_NO_MEM               (604)          /*因为初始化没有完成，函数执行失败*/
#define  ERR_MSP_SIMM_ALLOCMEM_FAILED      (605)          /*因为内存分配失败，函数执行失败*/
#define  ERR_MSP_SIMM_SENDMSG_FAILED       (606)          /*因为消息发送失败，函数执行失败*/
#define  ERR_MSP_SIMM_TIMEOUT              (607)          /*超时,函数执行失败*/
#define  ERR_MSP_SIMM_NO_CARD              (608)          /*卡不在位失败*/
#define  ERR_MSP_SIMM_SEM                  (609)          /*任务队列信号量错误*/

#define  ERR_MSP_SIMM_QUEUE_FULL           (621)          /*队列已满*/
#define  ERR_MSP_SIMM_QUEUE_EMPTY          (622)          /*队列为空*/
#define  ERR_MSP_SIMM_CMD_EXCEPTION        (623)          /*命令处理失败*/
#define  ERR_MSP_SIMM_CMD_TIMEOUT          (624)          /*命令处理超时*/

#define  ERR_MSP_SIMM_DF_FCP_FAILED        (630)          /*DF文件解析失败*/
#define  ERR_MSP_SIMM_PIDO_FAILED          (631)          /*PIDO解析失败*/
#define  ERR_MSP_SIMM_FCP_FAILED           (632)          /*EF文件解析失败*/
#define  ERR_MSP_SIMM_EFDIR_FAILED         (632)          /*EF文件解析失败*/

#define  ERR_MSP_SIMM_NEED_PIN             (640)          /*需要PIN码*/
#define  ERR_MSP_SIMM_NEED_PIN2            (641)          /*需要PIN2码*/
#define  ERR_MSP_SIMM_NEED_PUK             (642)          /*需要PUK码*/
#define  ERR_MSP_SIMM_NEED_PUK2            (643)          /*需要PUK2码*/

#define  ERR_MSP_SIMM_AUTH_MAC_FAILURE     (645)          /*鉴权MAC错误*/
#define  ERR_MSP_SIMM_AUTH_SYNC_FAILURE    (646)          /*鉴权SYNC错误*/
#define  ERR_MSP_SIMM_AUTH_OTHER_FAILURE   (647)          /*其他鉴权错误*/
#define  ERR_MSP_SIMM_OTHER_FAILURE        (648)          /*其他鉴权错误*/


#define  ERR_MSP_QUEUE_NOT_OPEN          (1000)
#define  ERR_MSP_QUEUE_UNAVAILABLE       (1001)
#define  ERR_MSP_QUEUE_SHUTDOWN          (1002)

#define  ERR_MSP_NV_ITEM_IS_EMPTY        (1100)
#define  ERR_MSP_NV_FLASH_FULL           (1103)

/* 对应BSP错误码*/
#define  ERR_MSP_NV_GOOD_BLOCK           (1105)
#define  ERR_MSP_NV_ERROR_INIT           (1106)
#define  ERR_MSP_NV_ERROR_ARGS           (1107)
#define  ERR_MSP_NV_ERROR_MALLOC         (1108)
#define  ERR_MSP_NV_ERROR_ERASE          (1111)

/* 升级错误码*/
#define  ERR_MSP_SUM_ONLY_NVCOPY_OK      (1200)
#define  ERR_MSP_SUM_BACK_OK             (1201) /* 回退成功 */
#define  ERR_MSP_SUM_BACK_FAIL           (1202) /* 回退失败 */
#define  ERR_MSP_SUM_BACK_NO_VERSION     (1203) /* 没有版本可回退 */
#define  ERR_MSP_SUM_UPDATE_RESET_FAILUE (1204)

#define  ERR_MSP_NVIM_BACKUP_FILE_FAIL    (2001)
#define  ERR_MSP_NVIM_BACKUP_DATA_FAIL    (2002)
#define  ERR_MSP_NVIM_WRITE_FILE_FAIL     (2003)
#define  ERR_MSP_NVIM_UPDATA_DATA_FAIL    (2004)
#define  ERR_MSP_NVIM_UPDATA_FILE_FAIL    (2005)
#define  ERR_MSP_NVIM_SECTION_NOT_SUPPORT_UPDATA    2006
#define  ERR_MSP_NVIM_NOT_SUPPORT_WRITE    2007
#define  ERR_MSP_NVIM_NEED_BACK_ERR             2008

#define LNVM_OPEN_FILE_ERROR      (2010)
#define	LNVM_READ_FILE_ERROR      (2011)
#define	LNVM_FTELL_FILE_ERROR     (2012)
#define	LNVM_WRITE_FILE_ERROR     (2013)
#define	LNVM_SEEK_FILE_ERROR      (2014)
#define	LNVM_REMOVE_FILE_ERROR    (2015)
#define	LNVM_CLOSE_FILE_ERROR     (2016)

#define MSP_SD_OPEN_FILE_ERROR      (2200)
#define	MSP_SD_WRITE_FILE_ERROR     (2201)
#define	MSP_SD_SEEK_FILE_ERROR      (2202)
#define	MSP_SD_GET_STATE_ERROR      (2203)
#define MSP_SD_CREAT_DIR_ERROR      (2204)


/*CDM ERR NO. START*/

#define ERR_MSP_CDM_BEGIN               (0x3000)

#define ERR_MSP_CDM_ACPU_CODER_SRC_FAIL (ERR_MSP_CDM_BEGIN + 1)

#define ERR_MSP_CDM_END                 (0x3100)
/*CDM ERR NO. END*/

/*scm ERR NO. START*/
#define ERR_MSP_SCM_BEGIN               (0x5000)
#define ERR_MSP_SCM_START_SOCP_FAIL     (ERR_MSP_CDM_BEGIN + 1)

/*scm ERR NO. END*/

/* DIAG给工具软件回复的错误码 0x4000-0x4fff */

#define  ERR_MSP_DIAG_ERROR_BEGIN               (0x4000)
#define  ERR_MSP_DIAG_ACORE_ERROR               (ERR_MSP_DIAG_ERROR_BEGIN + 1)  /* A核处理失败 */
#define  ERR_MSP_DIAG_CCORE_ERROR               (ERR_MSP_DIAG_ERROR_BEGIN + 2)  /* C核处理失败 */
#define  ERR_MSP_DIAG_INVALID_CMD               (ERR_MSP_DIAG_ERROR_BEGIN + 3)  /* 不能识别的诊断命令 */
#define  ERR_MSP_DIAG_ERRPID_CMD                (ERR_MSP_DIAG_ERROR_BEGIN + 4)  /* 透传命令的接收PID不支持 */
#define  ERR_MSP_DIAG_UNKNOWNPID_CMD            (ERR_MSP_DIAG_ERROR_BEGIN + 5)  /* 透传命令的接收PID未知 */
#define  ERR_MSP_DIAG_SAMPLE_START_FAIL         (ERR_MSP_DIAG_ERROR_BEGIN + 6)  /* 启动采数命令失败错误码 */
#define  ERR_MSP_DIAG_TL_SEND_MSG_FAIL          (ERR_MSP_DIAG_ERROR_BEGIN + 7)  /* 启动采数命令TL发送消息失败 */
#define  ERR_MSP_DIAG_GUC_SEND_MSG_FAIL         (ERR_MSP_DIAG_ERROR_BEGIN + 8)  /* 启动采数命令GUC发送消息失败 */
#define  ERR_MSP_DIAG_EASYRF_SEND_MSG_FAIL      (ERR_MSP_DIAG_ERROR_BEGIN + 9)  /* 启动采数命令EasyRF发送消息失败 */
#define  ERR_MSP_DIAG_ALLOC_MALLOC_FAIL         (ERR_MSP_DIAG_ERROR_BEGIN + 10)  /* 内存分配失败发送消息失败 */
#define  ERR_MSP_DIAG_SEND_MSG_FAIL             (ERR_MSP_DIAG_ERROR_BEGIN + 11)  /* 发送消息失败 */
#define  ERR_MSP_DIAG_NV_NUM_ERR                (ERR_MSP_DIAG_ERROR_BEGIN + 12)  /* nv id数量错误 */
#define  ERR_MSP_DIAG_GET_NVLEN_ERR             (ERR_MSP_DIAG_ERROR_BEGIN + 13)  /* nv id数量错误 */
#define  ERR_MSP_DIAG_GET_NV_LIST_ERR           (ERR_MSP_DIAG_ERROR_BEGIN + 14)  /* 获取nv id list错误 */
#define  ERR_MSP_DIAG_WRITE_NV_ERR              (ERR_MSP_DIAG_ERROR_BEGIN + 15)  /* 写NV错误 */
#define  ERR_MSP_DIAG_NOT_AUTH_NV_ERR           (ERR_MSP_DIAG_ERROR_BEGIN + 16)  /* 该Nv不可以被改写 */
#define  ERR_MSP_DIAG_FLUSH_NV_ERR              (ERR_MSP_DIAG_ERROR_BEGIN + 17)  /* fush nv错误 */
#define  ERR_MSP_DIAG_CB_NULL_ERR               (ERR_MSP_DIAG_ERROR_BEGIN + 18)  /* 回调函数为空错误 */
#define  ERR_MSP_DIAG_WRITE_MAILBOX_ERR         (ERR_MSP_DIAG_ERROR_BEGIN + 19)  /* 回调函数为空错误 */
#define  ERR_MSP_DIAG_CHANN_CONN_FAIL           (ERR_MSP_DIAG_ERROR_BEGIN + 20)
#define  ERR_MSP_DIAG_CONNECT_TIME_OUT          (ERR_MSP_DIAG_ERROR_BEGIN + 21)
#define  ERR_MSP_DIAG_REPEAT_REGISTER           (ERR_MSP_DIAG_ERROR_BEGIN + 22)
#define  ERR_MSP_INVALID_RESULT                 (ERR_MSP_DIAG_ERROR_BEGIN + 23)
#define  ERR_MSP_REPEAT_CONNECT_CNF             (ERR_MSP_DIAG_ERROR_BEGIN + 24)
#define  ERR_MSP_CONN_SN_ERROR                  (ERR_MSP_DIAG_ERROR_BEGIN + 25)
#define  ERR_MSP_CHNNEL_NUM_ERROR               (ERR_MSP_DIAG_ERROR_BEGIN + 26)
#define  ERR_MSP_INALID_LEN_ERROR               (ERR_MSP_DIAG_ERROR_BEGIN + 27) /* 工具下发的数据长度非法 */
#define  ERR_MSP_MEMCPY_S_ERROR                 (ERR_MSP_DIAG_ERROR_BEGIN + 28) /* memcpy出错 */



#define  ERR_MSP_FAILURE                (0XFFFFFFFFU)
#define  ERR_MSP_SYSTEM                 (0XFFFFFFFFU)
/*error no end**********************************************************************/


void mdrv_scm_spinlock_intlock_ind_src_buff(void);
void mdrv_scm_spinunlock_intunlock_ind_src_buff(void);
unsigned int mdrv_scm_get_ind_src_buff(unsigned int ulDataLen, SCM_CODER_SRC_PACKET_HEADER_STRU** pstCoderHeader, SOCP_BUFFER_RW_STRU *pstSocpBuf);
void mdrv_scm_ind_src_buff_mempy(SCM_CODER_SRC_MEMCPY_STRU *pInfo, SOCP_BUFFER_RW_STRU *pstSocpBuf);
void mdrv_scm_spinlock_intlock_cnf_src_buff(void);
void mdrv_scm_spinunlock_intunlock_cnf_src_buff(void);
unsigned int mdrv_scm_get_cnf_src_buff(unsigned int ulDataLen, SCM_CODER_SRC_PACKET_HEADER_STRU** pstCoderHeader, SOCP_BUFFER_RW_STRU *pstSocpBuf);
void mdrv_scm_cnf_src_buff_mempy(SCM_CODER_SRC_MEMCPY_STRU *pInfo, SOCP_BUFFER_RW_STRU *pstSocpBuf);
unsigned int mdrv_scm_send_cnf_src_data(unsigned char *pucSendDataAddr, unsigned int ulSendLen);
void mdrv_diag_PTR(DIAG_PTR_ID_ENUM enType, unsigned int paraMark, unsigned int para0, unsigned int para1);
unsigned int mdrv_GetThrputInfo(DIAG_THRPUT_ID_ENUM type);
unsigned int mdrv_scm_send_ind_src_data(unsigned char *pucSendDataAddr, unsigned int ulSendLen);




#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif

