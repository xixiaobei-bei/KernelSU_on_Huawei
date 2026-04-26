/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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

#ifndef _MDRV_OM_H_
#define _MDRV_OM_H_

#ifdef _cplusplus
extern "C" {
#endif
/**
 *  @brief   可维可测模块对外头文件
 *  @file    mdrv_om.h
 *  @author  xxx
 *  @version v1.0
 *  @date    2019.11.23
 *  @note    修改记录(版本号|修订日期|说明)
 * <ul><li>v1.0|2019.05.30|创建文件</li></ul>
 *  @since
 */
/**
 * @brief 各个模块使用Field ID 枚举类型
 */
enum om_field_id_e {
#ifndef CONFIG_HISI_BALONG_EXTRA_MODEM
    OM_AP_FIELD_BEGIN = (0x01200000),        /**< Modem AP 上层Field id起始值 */
    OM_AP_OSA = (OM_AP_FIELD_BEGIN),         /**< 拼片版本 Phone侧 Modem AP 上层Field id起始值 */
    OM_AP_DIAG = (OM_AP_FIELD_BEGIN + 1),    /**< 拼片版本 Phone侧 Modem AP 上层Field id起始值 */
    OM_AP_OSA_ICC = (OM_AP_FIELD_BEGIN + 0x2), /**< 拼片版本 Phone侧 Modem AP 上层Field id起始值 */
    OM_AP_AT = (OM_AP_FIELD_BEGIN + 0x3),      /**< 拼片版本 Phone侧 Modem AP 上层Field id起始值 */
    OM_AP_DMS = (OM_AP_FIELD_BEGIN + 0x4),     /**< Modem AP DMS 使用的Field id */

    OM_AP_FIELD_END,
#else
    OM_AP_FIELD_BEGIN = (0x0c200000),        /**< Modem AP 上层Field id起始值 */
    OM_AP_OSA = (OM_AP_FIELD_BEGIN),         /**< Modem AP OSA 使用的Field id */
    OM_AP_DIAG = (OM_AP_FIELD_BEGIN + 1),    /**< Modem AP DIAG 使用的Field id */
    OM_AP_OSA_ICC = (OM_AP_FIELD_BEGIN + 0x2), /**< Modem AP OSA_ICC 使用的Field id */
    OM_AP_DMS = (OM_AP_FIELD_BEGIN + 0x3),     /**< Modem AP DMS 使用的Field id */
    OM_AP_FIELD_END,
#endif

#ifndef __OS_NRCCPU__
    OM_CP_FIELD_BEGIN = (0x02200000),          /**< Modem CP 上层Field id起始值 */
    OM_CP_OSA = (OM_CP_FIELD_BEGIN),           /**< Modem CP OSA 使用的Field id */
    OM_CP_MSP_SLEEP = (OM_CP_FIELD_BEGIN + 0x1), /**< Modem CP MSP_SLEEP 使用的Field id */
    OM_CP_TLPS = (OM_CP_FIELD_BEGIN + 0x2),      /**< Modem CP TLPS 使用的Field id */
    OM_CP_FTM_MNTN = (OM_CP_FIELD_BEGIN + 0x3),  /**< Modem CP FTM 使用的Field id */
    OM_CP_GUAS = (OM_CP_FIELD_BEGIN + 0x4),      /**< Modem CP GUAS 使用的Field id */
    OM_CP_DIAG = (OM_CP_FIELD_BEGIN + 0x5),      /**< Modem CP DIAG 使用的Field id */
    OM_CP_GUNAS = (OM_CP_FIELD_BEGIN + 0x6),     /**< Modem CP GUNAS 使用的Field id */
    OM_CP_CPROC = (OM_CP_FIELD_BEGIN + 0x7),     /**< Modem CP CPROC 使用的Field id */
    OM_CP_CAS = (OM_CP_FIELD_BEGIN + 0x8),       /**< Modem CP CAS 使用的Field id */
    OM_CP_CNAS = (OM_CP_FIELD_BEGIN + 0x9),      /**< Modem CP CNAS 使用的Field id */
    OM_CP_MSCC = (OM_CP_FIELD_BEGIN + 0xa),     /**< Modem CP MSCC 使用的Field id */
    OM_CP_CTTF = (OM_CP_FIELD_BEGIN + 0xb),     /**< Modem CP CTTF 使用的Field id */
    OM_CP_USIMM0 = (OM_CP_FIELD_BEGIN + 0xc),   /**< Modem CP USIM0 使用的Field id */
    OM_CP_USIMM1 = (OM_CP_FIELD_BEGIN + 0xd),   /**< Modem CP USIM1 使用的Field id */
    OM_CP_TLPS1 = (OM_CP_FIELD_BEGIN + 0xe),    /**< Modem CP TLPS1 使用的Field id */
    OM_CP_GUPHY = (OM_CP_FIELD_BEGIN + 0xf),    /**< Modem CP GUPHY 使用的Field id */
    OM_CP_RFDSP = (OM_CP_FIELD_BEGIN + 0x10),    /**< Modem CP RFDSP 使用的Field id */
    OM_CP_GUCSLEEP = (OM_CP_FIELD_BEGIN + 0x11), /**< Modem CP GUCSLEEP 使用的Field id */
    OM_CP_TAF = (OM_CP_FIELD_BEGIN + 0x12),      /**< Modem CP TAF 使用的Field id */
    OM_CP_BASTET = (OM_CP_FIELD_BEGIN + 0x13),   /**< Modem CP BASTET 使用的Field id */
    OM_CP_IMS = (OM_CP_FIELD_BEGIN + 0x14),      /**< Modem CP IMS 使用的Field id */
    OM_CP_FIELD_END,
#else
    OM_CP_FIELD_BEGIN = (0x07200000),          /**< Modem NRCCPU 上层Field id起始值 */
    OM_CP_OSA = (OM_CP_FIELD_BEGIN),           /**< Modem NRCCPU OSA 使用的Field id */
    OM_CP_MSP_SLEEP = (OM_CP_FIELD_BEGIN + 0x1), /**< Modem NRCCPU MSP_SLEEP 使用的Field id */
    OM_CP_TLPS = (OM_CP_FIELD_BEGIN + 0x2),      /**< Modem NRCCPU TLPS 使用的Field id */
    OM_CP_FTM_MNTN = (OM_CP_FIELD_BEGIN + 0x3),  /**< Modem NRCCPU FTM 使用的Field id */
    OM_CP_GUAS = (OM_CP_FIELD_BEGIN + 0x4),      /**< Modem NRCCPU GUAS 使用的Field id */
    OM_CP_DIAG = (OM_CP_FIELD_BEGIN + 0x5),      /**< Modem NRCCPU DIAG 使用的Field id */
    OM_CP_GUNAS = (OM_CP_FIELD_BEGIN + 0x6),     /**< Modem NRCCPU GUNAS  使用的Field id */
    OM_CP_CPROC = (OM_CP_FIELD_BEGIN + 0x7),     /**< Modem NRCCPU CPROC  使用的Field id */
    OM_CP_CAS = (OM_CP_FIELD_BEGIN + 0x8),       /**< Modem NRCCPU CAS 使用的Field id */
    OM_CP_CNAS = (OM_CP_FIELD_BEGIN + 0x9),      /**< Modem NRCCPU CNAS  使用的Field id */
    OM_CP_MSCC = (OM_CP_FIELD_BEGIN + 0xa),     /**< Modem NRCCPU MSCC 使用的Field id */
    OM_CP_CTTF = (OM_CP_FIELD_BEGIN + 0xb),     /**< Modem NRCCPU CTTF 使用的Field id */
    OM_CP_USIMM0 = (OM_CP_FIELD_BEGIN + 0xc),   /**< Modem NRCCPU USIMM0  使用的Field id */
    OM_CP_USIMM1 = (OM_CP_FIELD_BEGIN + 0xd),   /**< Modem NRCCPU USIMM1 使用的Field id */
    OM_CP_TLPS1 = (OM_CP_FIELD_BEGIN + 0xe),    /**< Modem NRCCPU TLPS1 使用的Field id */
    OM_CP_GUPHY = (OM_CP_FIELD_BEGIN + 0xf),    /**< Modem NRCCPU GUPHY 使用的Field id */
    OM_CP_RFDSP = (OM_CP_FIELD_BEGIN + 0x10),    /**< Modem NRCCPU RFDSP 使用的Field id */
    OM_CP_GUCSLEEP = (OM_CP_FIELD_BEGIN + 0x11), /**< Modem NRCCPU GUCSLEEP 使用的Field id */
    OM_CP_NRNAS = (OM_CP_FIELD_BEGIN + 0x12),    /**< Modem NRCCPU NRNAS 使用的Field id */
    OM_CP_NRRC = (OM_CP_FIELD_BEGIN + 0x13),     /**< Modem NRCCPU NRRC 使用的Field id */
    OM_CP_LL1CSTL = (OM_CP_FIELD_BEGIN + 0x14),  /**< Modem NRCCPU LL1CSTL  使用的Field id */
    OM_CP_IMS = (OM_CP_FIELD_BEGIN + 0x15),      /**< Modem NRCCPU IMS 使用的Field id */
    OM_CP_FIELD_END,
#endif
    OM_HAC_FIELD_BEGIN = (0x08200000), /**< Modem L2HAC 上层Field id起始值 */
    OM_HAC_FIELD_END,
};
#define LOG_DESC_LEN 4
/**
 * @brief Dump日志Head信息结构体
 */
typedef struct {
    unsigned char description[LOG_DESC_LEN]; /**<  dump文件信息描述，填写使用组件对应的ASCII码  */
    unsigned int dump_size;       /**<  dump文件大小，包含头，单位byte  */
} dump_log_head_s;

/**
 * @brief Modem通话状态
 */
enum {
    DUMP_OUT_VOICE = 0, /**< Modem未在通话中 */
    DUMP_IN_VOICE = 1,  /**< Modem在通话中 */
    DUMP_VOICE_BUTT,
};
/**
 * @brief 系统异常回调函数地址
 */
typedef int dump_handle;
/**
 * @brief 回调函数原型
 */
typedef void (*dump_hook)(void);

/**
 * @brief 触发Modem子系统复位
 * @par 描述:
 * modemap子系统进入复位，并生成复位日志
 *
 * @attention
 * <ul><li> mod_id需要根据各个组件划分的范围填写，不可随意传入</li></ul>
 * @param[in]  mod_id , 异常错误码
 * @param[in]  arg1, 异常参数1
 * @param[in]  arg2, 异常参数2
 * @param[in]  arg3, 异常参数3的虚拟地址
 * @param[in]  arg3_length, 异常参数3长度
 *
 * @par 依赖:
 * <ul><li>mdrv_om.h：该接口声明所在的头文件。</li></ul>
 */
void mdrv_om_system_error(int modId, int arg1, int arg2, char *arg3_virt_addr, int arg3_length);

/**
 * @brief 申请维测内存
 * @par 描述:
 * 申请维测内存，在维测内存中记录的日志可以随mimidump保存
 *
 * @attention
 * <ul><li> field_id需要严格按照给各个模块划分的传入</li></ul>
 * @param[in]  field_id , 维测空间的field
 * @param[in]  field_name, 维测空间的名称
 * @param[in]  length,  申请维测内存的长度
 * @param[in]  version, 维测内存的版本号
 *
 * @retval NULL,申请地址失败
 * @retval != NULL,申请空间成功，返回为该filedid分配的虚拟地址
 * @see enum om_filed_id_enum
 */
unsigned char *mdrv_om_register_field(unsigned int field_id, char *field_name, unsigned int length,
                                      unsigned short version);

/**
 * @brief 查询指定fieldid对应的维测空间的虚拟地址
 * @par 描述:
 * 根据field id查询对应的field id的维测空间的虚拟地址
 *
 * @attention
 * <ul><li> field_id需要严格按照给各个模块划分的传入</li></ul>
 * @param[in]  field_id , 维测空间的field
 *
 * @retval NULL,查询地址失败
 * @retval != NULL,对应fieldid的虚拟地址
 * @par 依赖:
 * <ul><li>mdrv_om_register_field</li></ul>
 * @see enum om_filed_id_enum
 */
unsigned char *mdrv_om_get_field_addr(unsigned int field_id);

/**
 * @brief 注册异常回调函数
 * @par 描述:
 * 异常后，会回调注册的回调函数，各个模块可以在回调中完成相应的动作
 *
 * @param[in]  name , 异常回调名称
 * @param[in]  p_func , 回调函数的地址
 * @retval -1, 注册失败
 * @retval 非0,注册回调成功
 */
dump_handle mdrv_om_register_callback(char *name, dump_hook p_func);

#ifdef _cplusplus
}
#endif
#endif
