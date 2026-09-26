/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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
#ifndef __MDRV_DIAG_H__
#define __MDRV_DIAG_H__

#include <stdarg.h>
#include "mdrv_diag_system.h"
#include "mdrv_socp.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/**
 * @brief Generate DIAG ID
 */
#define MDRV_DIAG_ID(module_id, log_type)   (unsigned int)(module_id | (log_type << 12))

/**
* @brief Generate AIR LOG ID
*/
#define MDRV_DIAG_AIR_MSG_LOG_ID(module_id, is_up_link)  MDRV_DIAG_ID(module_id, is_up_link)

/**
* @brief Generate PRINT LOG Module
*/
#define MDRV_DIAG_GEN_LOG_MODULE(modemid, modetype, level)  \
    (((unsigned int)(modemid & 0xff) << 24)  \
     | ((unsigned int)(modetype & 0xf) << 16)  \
     | ((unsigned int)(level & 0xf) << 12))

/**
* @brief Generate LOG Module
*/
#define MDRV_DIAG_GEN_MODULE(modemid, modetype)  \
    (((unsigned int)(modemid & 0xff) << 24)  \
     | ((unsigned int)(modetype & 0xf) << 16))

/**
* @brief Generate LOG Module ex
*/
#define MDRV_DIAG_GEN_MODULE_EX(modemid, modetype, groupid)  \
    (((unsigned int)(modemid & 0xff) << 24)  \
     | ((unsigned int)(modetype & 0xf) << 16) \
     | ((unsigned int)(groupid  & 0xf)  << 8))

/**
* @brief Generate LOG ID
*/
#define MDRV_DIAG_GEN_LOG_ID(filenum, linenum)   \
    ((((unsigned int)(filenum & 0XFFFF)) << 16)   \
     | ((unsigned int)(linenum & 0XFFFF)))

/**
* @brief msg direction
*/
#define OS_MSG_UL                (0x01)/**< 表示上行消息*/
#define OS_MSG_DL                (0x02)/**< 表示下行消息*/

/**
* @brief AIR msg direction
*/
#define DIAG_SIDE_UE             (0x1)  /**< 表示UE接收的空口消息：NET-->UE*/
#define DIAG_SIDE_NET            (0x2)  /**< 表示NET接收的空口消息：UE-->NET*/

/**
* @brief PRINT LOG level
*/
#define DIAG_LOG_TYPE_ERROR 0x00000001UL
#define DIAG_LOG_TYPE_AUDIT_SUCCESS 0x00000002UL
#define DIAG_LOG_TYPE_AUDIT_FAILURE 0x00000004UL
#define DIAG_LOG_TYPE_INFO 0x00000008UL
#define DIAG_LOG_TYPE_WARNING 0x00000010UL

/**
* @brief port disconnect cmd
*/
#define ID_MSG_DIAG_HSO_DISCONN_IND (0x00010004)

/**
* @brief DIAG发送给PS_PID_MM的HSO回放请求
*/
#define ID_MSG_DIAG_CMD_REPLAY_TO_PS (0x00010100)

/**
* @brief DIAG发送给各个子系统的建连请求
*/
#define ID_MSG_DIAG_CMD_CONNECT_REQ (0x00010200)
#define ID_MSG_DIAG_CMD_CONNECT_CNF (ID_MSG_DIAG_CMD_CONNECT_REQ)

/**
* @brief DIAG发送给各个子系统的断连请求
*/
#define ID_MSG_DIAG_CMD_DISCONNECT_REQ (0x00010202)
#define ID_MSG_DIAG_CMD_DISCONNECT_CNF (ID_MSG_DIAG_CMD_DISCONNECT_REQ)

/**
* @brief MSP Error No
*/
#define  ERR_MSP_SUCCESS                (0)
#define  ERR_MSP_FAILURE                (0xFFFFFFFFU)

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
#define  ERR_MSP_INVALID_LEN            (14)
#define  ERR_MSP_MALLOC_FAILUE          (15)
#define  ERR_MSP_NOT_ENOUGH_MEMORY      (16)
#define  ERR_MSP_INVALID_HEAP_ADDR      (17)
#define  ERR_MSP_SHUTDOWN               (18) 
#define  ERR_MSP_NOT_CONNECT            (19)
#define  ERR_MSP_BUSY                   (20)
#define  ERR_MSP_PARA_NULL              (21)

#define  ERR_MSP_UNAVAILABLE            (50)
#define  ERR_MSP_TIMEOUT                (51)
#define  ERR_MSP_INIT_FAILURE           (52)
#define  ERR_MSP_IO_REGISTER_FAILURE    (53)
#define  ERR_MSP_NO_REGISTER            (54)

#define  ERR_MSP_FLASH_ERASE_ERROR      (100)
#define  ERR_MSP_FLASH_WRITE_ERROR      (101)
#define  ERR_MSP_FLASH_ALIGN_ERROR      (102)
#define  ERR_MSP_FLASH_PROTECT_ERROR    (103)
#define  ERR_MSP_FLASH_BIN_LOAD_NEED_FIRST_BLOCK    (104) 
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

#define  ERR_MSP_SUM_ONLY_NVCOPY_OK      (1200)
#define  ERR_MSP_SUM_BACK_OK             (1201) 
#define  ERR_MSP_SUM_BACK_FAIL           (1202) 
#define  ERR_MSP_SUM_BACK_NO_VERSION     (1203) 
#define  ERR_MSP_SUM_UPDATE_RESET_FAILUE (1204)

#define  ERR_MSP_NVIM_BACKUP_FILE_FAIL    (2001)
#define  ERR_MSP_NVIM_BACKUP_DATA_FAIL    (2002)
#define  ERR_MSP_NVIM_WRITE_FILE_FAIL     (2003)
#define  ERR_MSP_NVIM_UPDATA_DATA_FAIL    (2004)
#define  ERR_MSP_NVIM_UPDATA_FILE_FAIL    (2005)
#define  ERR_MSP_NVIM_SECTION_NOT_SUPPORT_UPDATA    2006
#define  ERR_MSP_NVIM_NOT_SUPPORT_WRITE    2007
#define  ERR_MSP_NVIM_NEED_BACK_ERR             2008

#define MSP_SD_OPEN_FILE_ERROR      (2200)
#define	MSP_SD_WRITE_FILE_ERROR     (2201)
#define	MSP_SD_SEEK_FILE_ERROR      (2202)
#define	MSP_SD_GET_STATE_ERROR      (2203)
#define MSP_SD_CREAT_DIR_ERROR      (2204)

#define ERR_MSP_CDM_BEGIN               (0x3000)
#define ERR_MSP_CDM_ACPU_CODER_SRC_FAIL (ERR_MSP_CDM_BEGIN + 1)
#define ERR_MSP_CDM_END                 (0x3100)

#define ERR_MSP_SCM_BEGIN               (0x5000)
#define ERR_MSP_SCM_START_SOCP_FAIL     (ERR_MSP_SCM_BEGIN + 1)

#define  ERR_MSP_DIAG_ERROR_BEGIN               (0x4000)
#define  ERR_MSP_DIAG_ACORE_ERROR               (ERR_MSP_DIAG_ERROR_BEGIN + 1)  
#define  ERR_MSP_DIAG_CCORE_ERROR               (ERR_MSP_DIAG_ERROR_BEGIN + 2)  
#define  ERR_MSP_DIAG_INVALID_CMD               (ERR_MSP_DIAG_ERROR_BEGIN + 3)  
#define  ERR_MSP_DIAG_ERRPID_CMD                (ERR_MSP_DIAG_ERROR_BEGIN + 4)  
#define  ERR_MSP_DIAG_UNKNOWNPID_CMD            (ERR_MSP_DIAG_ERROR_BEGIN + 5)  
#define  ERR_MSP_DIAG_SAMPLE_START_FAIL         (ERR_MSP_DIAG_ERROR_BEGIN + 6)  
#define  ERR_MSP_DIAG_TL_SEND_MSG_FAIL          (ERR_MSP_DIAG_ERROR_BEGIN + 7)  
#define  ERR_MSP_DIAG_GUC_SEND_MSG_FAIL         (ERR_MSP_DIAG_ERROR_BEGIN + 8)  
#define  ERR_MSP_DIAG_EASYRF_SEND_MSG_FAIL      (ERR_MSP_DIAG_ERROR_BEGIN + 9)  
#define  ERR_MSP_DIAG_ALLOC_MALLOC_FAIL         (ERR_MSP_DIAG_ERROR_BEGIN + 10) 
#define  ERR_MSP_DIAG_SEND_MSG_FAIL             (ERR_MSP_DIAG_ERROR_BEGIN + 11) 
#define  ERR_MSP_DIAG_NV_NUM_ERR                (ERR_MSP_DIAG_ERROR_BEGIN + 12) 
#define  ERR_MSP_DIAG_GET_NVLEN_ERR             (ERR_MSP_DIAG_ERROR_BEGIN + 13) 
#define  ERR_MSP_DIAG_GET_NV_LIST_ERR           (ERR_MSP_DIAG_ERROR_BEGIN + 14) 
#define  ERR_MSP_DIAG_WRITE_NV_ERR              (ERR_MSP_DIAG_ERROR_BEGIN + 15) 
#define  ERR_MSP_DIAG_NOT_AUTH_NV_ERR           (ERR_MSP_DIAG_ERROR_BEGIN + 16) 
#define  ERR_MSP_DIAG_FLUSH_NV_ERR              (ERR_MSP_DIAG_ERROR_BEGIN + 17) 
#define  ERR_MSP_DIAG_CB_NULL_ERR               (ERR_MSP_DIAG_ERROR_BEGIN + 18) 
#define  ERR_MSP_DIAG_WRITE_MAILBOX_ERR         (ERR_MSP_DIAG_ERROR_BEGIN + 19)
#define  ERR_MSP_DIAG_CHANN_CONN_FAIL           (ERR_MSP_DIAG_ERROR_BEGIN + 20)
#define  ERR_MSP_DIAG_CONNECT_TIME_OUT          (ERR_MSP_DIAG_ERROR_BEGIN + 21)
#define  ERR_MSP_DIAG_REPEAT_REGISTER           (ERR_MSP_DIAG_ERROR_BEGIN + 22)
#define  ERR_MSP_INVALID_RESULT                 (ERR_MSP_DIAG_ERROR_BEGIN + 23)
#define  ERR_MSP_REPEAT_CONNECT_CNF             (ERR_MSP_DIAG_ERROR_BEGIN + 24)
#define  ERR_MSP_CONN_SN_ERROR                  (ERR_MSP_DIAG_ERROR_BEGIN + 25)
#define  ERR_MSP_CHNNEL_NUM_ERROR               (ERR_MSP_DIAG_ERROR_BEGIN + 26)
#define  ERR_MSP_INALID_LEN_ERROR               (ERR_MSP_DIAG_ERROR_BEGIN + 27)
#define  ERR_MSP_MEMCPY_S_ERROR                 (ERR_MSP_DIAG_ERROR_BEGIN + 28)
#define  ERR_MSP_MSG_ID_ERROR                   (ERR_MSP_DIAG_ERROR_BEGIN + 29) 
#define  ERR_MSP_GET_VIRT_ADDR_FAIL             (ERR_MSP_DIAG_ERROR_BEGIN + 30)
#define  ERR_MSP_WRITE_DONE_FAIL                (ERR_MSP_DIAG_ERROR_BEGIN + 31)
#define  ERR_MSP_STOP_SOCP_FAIL                 (ERR_MSP_DIAG_ERROR_BEGIN + 33)
#define  ERR_MSP_GET_WRITE_BUFF_FAIL            (ERR_MSP_DIAG_ERROR_BEGIN + 34)
#define  ERR_MSP_CLEAR_BUFF_FAIL                (ERR_MSP_DIAG_ERROR_BEGIN + 35)
#define  ERR_MSP_CONNECT_AUTH_FAIL              (ERR_MSP_DIAG_ERROR_BEGIN + 36)
#define  ERR_MSP_GEN_AUTH_HASH_FAIL             (ERR_MSP_DIAG_ERROR_BEGIN + 37)
#define  ERR_MSP_REPEAT_AUTH_HASH_FAIL          (ERR_MSP_DIAG_ERROR_BEGIN + 38)
#define  ERR_MSP_DEC_RSA_DATA_FAIL              (ERR_MSP_DIAG_ERROR_BEGIN + 39)
#define  ERR_MSP_DEC_RSA_SIZE_FAIL              (ERR_MSP_DIAG_ERROR_BEGIN + 40)
#define  ERR_MSP_PORT_SWITCH_FAIL               (ERR_MSP_DIAG_ERROR_BEGIN + 41)
#define  ERR_MSP_PCDEV_LINK_DOWN                (ERR_MSP_DIAG_ERROR_BEGIN + 42)

/**
* @brief 消息类型，对应msg id中的pri_4b
*/
typedef enum mdrv_diag_message_type {
    DIAG_MSG_TYPE_RSV       = 0x0,
    DIAG_MSG_TYPE_MSP       = 0x1,
    DIAG_MSG_TYPE_PS        = 0x2,
    DIAG_MSG_TYPE_PHY       = 0x3,
    DIAG_MSG_TYPE_BBP       = 0x4,
    DIAG_MSG_TYPE_HSO       = 0x5,
    DIAG_MSG_TYPE_BSP       = 0x9,
    DIAG_MSG_TYPE_EASYRF    = 0xa,
    DIAG_MSG_TYPE_AP_BSP    = 0xb,
    DIAG_MSG_TYPE_AUDIO     = 0xc,
    DIAG_MSG_TYPE_APP       = 0xe,
    DIAG_MSG_TYPE_BUTT
} mdrv_diag_message_type_e;

/**
* @brief 模类型，对应msg id中的mode_4b
*/
typedef enum mdrv_diag_mode_type {
    DIAG_MODE_LTE  = 0x0,
    DIAG_MODE_TDS  = 0x1,
    DIAG_MODE_GSM  = 0x2,
    DIAG_MODE_UMTS = 0x3,
    DIAG_MODE_1X   = 0x4,
    DIAG_MODE_HRPD = 0x5,
    DIAG_MODE_NR   = 0x6,
    DIAG_MODE_LTEV = 0x7,
    DIAG_MODE_COMM = 0xf
} mdrv_diag_mode_type_e;

/**
* @brief modem号，对应DIAG一级头中的mdm_id
*/
typedef enum mdrv_diag_modem_id {
    DIAG_MODEM_0 = 0x0,
    DIAG_MODEM_1 = 0x1,
    DIAG_MODEM_2 = 0x2
} mdrv_diag_modem_id_e;

/**
* @brief 协议栈上报的LOG级别
*/
typedef enum {
    PS_LOG_LEVEL_OFF  = 0,
    PS_LOG_LEVEL_ERROR,
    PS_LOG_LEVEL_WARNING,
    PS_LOG_LEVEL_NORMAL,
    PS_LOG_LEVEL_INFO,
    PS_LOG_LEVEL_BUTT
} mdrv_diag_log_level_e;

/**
* @brief 当前DIAG连接状态
*/
typedef enum {
    DIAG_STATE_DISCONN  = 0,
    DIAG_STATE_CONN = 1,
    DIAG_STATE_POWERON_LOG = 2,
    DIAG_STATE_BUTT
} mdrv_diag_state_e;

/**
* @brief DIAG维测信息结构体
*/
typedef struct {
    unsigned int ulOverFlow50Num;
    unsigned int ulOverFlow80Num;

    unsigned int ulAbandonNum;
    unsigned int ulAbandonLen;

    unsigned int ulPackageLen;
    unsigned int ulPackageNum;
} mdrv_diag_debug_info_s;

/**
* @brief Event LOG 结构
*/
typedef struct {
    unsigned int        ulModule;
    unsigned int        ulPid;
    unsigned int        ulEventId;
    unsigned int        ulLength;
    void               *pData;
} mdrv_diag_event_ind_s;

/**
* @brief Air LOG 结构
*/
typedef struct {
    unsigned int        ulModule;
    unsigned int        ulPid;
    unsigned int        ulMsgId;
    unsigned int        ulDirection;
    unsigned int        ulLength;
    void               *pData;
} mdrv_diag_air_ind_s;

/**
* @brief Trans LOG 结构
*/
typedef struct {
    unsigned int        ulModule;
    unsigned int        ulPid;
    unsigned int        ulMsgId;
    unsigned int        ulReserve;
    unsigned int        ulLength;
    void               *pData;
} mdrv_diag_trans_ind_s;

/**
* @brief DIAG一级头消息结构,B5010不再使用
*/
typedef struct {
    unsigned int        ulSSId;         /**< 数据产生的CPU ID */
    unsigned int        ulMsgType;      /**< 所属组件 */
    unsigned int        ulMode;         /**< 模式 */
    unsigned int        ulSubType;      /**< 子类型，DIAG_MSG_SUB_TYPE_unsigned int */
    unsigned int        ulDirection;    /**< 上报消息的方向 */
    unsigned int        ulModemid;
    unsigned int        ulMsgId;        /**< 低16位有效 */
    unsigned int        lTransId;      /**< TransId */
} mdrv_diag_cnf_info_s;

/**
* @brief diag mntn switch
*/
typedef struct {
    unsigned char mntn_switch;    /**< 1 打开 0关闭 */
    unsigned char mntn_time;      /**< 上报周期*/
} mdrv_diag_mntn_req_stru;


/**
 * @brief DIAG状态变化通知注册函数类型
 * @par 描述:
 * 该接口用于提供给协议栈、PHY、HIFI等，用于注册DIAG连接状态变化通知函数
 * @attention
 * <ul><li>RF PHY调用该接口流程中，需要等到接口返回后再根据返回值进行后续流程操作</li></ul>
 * <ul><li>调用者必须判断接口返回值，如果返回值为非0，需要有明确维测信息，停止采数并且给DRV回复相应错误码</li></ul>
 * @param[in]  mdrv_diag_state_e , DIAG状态枚举值
 * @retval void。
 * @see bbpds_timer_set_info_s
 */
typedef void (*mdrv_diag_conn_state_notify_fun)(mdrv_diag_state_e state);

/**
 * @brief 协议栈透传上报接口
 * @par 描述:
 * 该接口提供协议栈使用，用于上报透传LOG
 * @attention
 * <ul><li>精简开机LOG模式下不允许上报trans log，调用该接口会返回错误码3</li></ul>
 * @param[in]  mdrv_diag_trans_ind_s, 透传LOG消息结构
 * @retval 0：LOG上报成功。
 * @retval 3：工具未连接。
 * @retval 12：socp buffer满。
 * @retval 14：LOG 长度不合法。
 * @retval 203：Trans开关没有打开。
 * @retval 0xffffffff：参数错误。
 * @see mdrv_diag_trans_ind_s
 */
MODULE_EXPORTED unsigned int mdrv_diag_trans_report(mdrv_diag_trans_ind_s *trans_data);

/**
 * @brief 协议栈打印LOG上报接口
 * @par 描述:
 * 该接口提供协议栈使用，用于上报打印LOG
 * @attention
 * <ul><li>调用该接口的任务栈中的内存至少为该接口预留1K Bytes空间</li></ul>
 * <ul><li>调用该接口输出的LOG不要超过1K Bytes，超出部分会自动丢弃</li></ul>
 * <ul><li>该接口不允许二次封装</li></ul>
 * @param[in]  module_id, 上报的module id
 * @param[in]  mid, 消息ID
 * @param[in]  file_name, 打印文件名
 * @param[in]  line_num, 文件行号
 * @param[in]  fmt, 格式化字符地址
 * @param[in]  ..., 可变参数
 * @retval 0：LOG上报成功。
 * @retval 3：工具未连接。
 * @retval 12：socp buffer满。
 * @retval 203：Print开关没有打开。
 * @retval 0xffffffff：参数错误。
 */
MODULE_EXPORTED unsigned int mdrv_diag_log_report(unsigned int module_id, unsigned int pid,
                                                  char *file_name, unsigned int line_num, char* fmt, ...);

/**
 * @brief 协议栈事件上报接口
 * @par 描述:
 * 该接口提供协议栈使用，用于上报事件LOG
 * @attention
 * @param[in]  mdrv_diag_event_ind_s, 事件LOG消息结构
 * @retval 0：LOG上报成功。
 * @retval 3：工具未连接。
 * @retval 12：socp buffer满。
 * @retval 14：LOG 长度不合法。
 * @retval 203：event开关没有打开。
 * @retval 0xffffffff：参数错误。
 * @see mdrv_diag_event_ind_s
 */
MODULE_EXPORTED unsigned int mdrv_diag_event_report(mdrv_diag_event_ind_s *event_data);

/**
 * @brief 空口消息上报控制接口
 * @par 描述:
 * 该接口提供协议栈使用，用于查询当前是否允许空口LOG上报
 * @param[in] void
 * @retval 0：允许上报。
 * @retval 203：AIR开关没有打开。
 */
MODULE_EXPORTED unsigned int mdrv_diag_allow_air_msg_report(void);

/**
 * @brief 协议栈空口上报接口
 * @par 描述:
 * 该接口提供协议栈使用，用于上报空口LOG
 * @attention
 * @param[in]  mdrv_diag_air_ind_s, 空口LOG消息结构
 * @retval 0：LOG上报成功。
 * @retval 3：工具未连接。
 * @retval 12：socp buffer满。
 * @retval 22：参数错误。
 * @retval 203：AIR开关没有打开。
 * @see mdrv_diag_air_ind_s
 */
MODULE_EXPORTED unsigned int mdrv_diag_air_report(mdrv_diag_air_ind_s *air_data);

/**
 * @brief 协议栈空层间上报接口
 * @par 描述:
 * 该接口提供协议栈使用，用于上报层间LOG
 * @attention
 * <ul><li>该接口内部不提供协议栈脱敏回调函数，协议栈调用该接口之前自行脱敏</li></ul>
 * @param[in]  void *trace_data, 消息结构
 * @retval 0：LOG上报成功。
 * @retval 3：工具未连接。
 * @retval 12：socp buffer满。
 * @retval 22：参数错误。
 * @retval 203：层间开关没有打开。
 * @see mdrv_diag_air_ind_s
 */
MODULE_EXPORTED unsigned int mdrv_diag_trace_report(void *trace_data);

/**
 * @brief DIAG连接状态查询接口
 * @par 描述:
 * 该接口提供协议栈和物理层使用，用于获取DIAG当前连接状态
 * @param[in]  void
 * @retval 0：工具未连接。
 * @retval 1：工具已连接。
 * @retval 2：开机LOG状态。
 */
MODULE_EXPORTED unsigned int mdrv_diag_get_conn_state(void);

/**
 * @brief 获取当前PRINT LOG级别
 * @par 描述:
 * 该接口提供协议栈和物理层使用，用于获取PRINT LOG上报级别
 * @param[in]  pid 消息mid
 * @retval 0：LOG关闭。
 * @retval 1：ERROR级别。
 * @retval 2：WARNING级别。
 * @retval 2：NORMAL级别。
 * @retval 2：INFO级别。
 */
MODULE_EXPORTED unsigned int mdrv_diag_get_log_level(unsigned int mid);

/**
 * @brief DIAG连接状态通知注册接口
 * @par 描述:
 * 该接口提供协议栈、物理层等上层组件使用，用于注册DIAG连接状态通知处理函数
 * @attention
 * <ul><li>DIAG_ConnStateNotifyFun函数会在DIAG和工具连接断开过程中调用，其中不能有任何阻塞性操作</li></ul>
 * <ul><li>不允许反复注册,单个组件只允许注册一次</li></ul>
 * @param[in] mdrv_diag_conn_state_notify_fun func, DIAG连接状态通知处理函数
 * @retval 0: 注册成功；
 * @retval 12: 注册资源不足；
 */
MODULE_EXPORTED unsigned int mdrv_diag_conn_state_notify_fun_reg(mdrv_diag_conn_state_notify_fun func);

/**
 * @brief 连接端口切换接口
 * @par 描述:
 * 该接口提供T命令使用，用于切换DIAG物理端口切换
 * @param[in] phy_port, 待切换的物理端口
 * @param[in] effect, 是否永久生效
 * @retval 0: 切换成功；
 * @retval ！=0：切换错误
 */
MODULE_EXPORTED  static inline unsigned int mdrv_diag_log_port_switch(unsigned int phy_port, unsigned int effect) {return 0;}

/**
 * @brief 连接端口切换接口
 * @par 描述:
 * 该接口提供T命令使用，用于切换DIAG物理端口切换
 * @param[in] phy_port, 待切换的物理端口
 * @param[in] effect, 是否永久生效
 * @retval 0: 切换成功；
 * @retval ！=0：切换错误
 */
MODULE_EXPORTED unsigned int mdrv_diag_set_log_port(unsigned int phy_port, unsigned int effect);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

