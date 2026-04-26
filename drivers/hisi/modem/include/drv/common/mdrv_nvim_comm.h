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

/* *
 * @brief   nv模块comm对外头文件
 * @file    mdrv_nvim_comm.h
 * @author  zhaoxiaofeng
 * @version v1.0
 * @date    2019.12.4
 * @note    修改记录(版本号|修订日期|说明)
 * <ul><li>v1.0|2019.12.4|创建文件</li></ul>
 * @since
 */
#ifndef __MDRV_NVIM_COMMON_H__
#define __MDRV_NVIM_COMMON_H__

#include <product_config.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ******************************* NV return value, need to be cleared ******************************** */
#ifndef NV_OK
/* *
 * @brief 函数返回成功
 */
#define NV_OK 0
#endif
/* *
 * @brief 找不到NV
 */
#define NV_ID_NOT_EXIST 1
/* *
 * @brief NV BUFF过长
 */
#define NV_BUFFER_TOO_LONG 2
/* *
 * @brief NV BUFF 不存在
 */
#define NV_BUFFER_NOT_EXIST 4
/* *
 * @brief NV写失败
 */
#define NV_WRITE_SECURE_FAIL 18
/* *
 * @brief NV正在恢复数据
 */
#define NV_RESTORE_RUNNING 0x5A5A55AA
/* *
 * @brief 是鉴权NV
 */
#define NV_AUTH_YES 3
/* *
 * @brief 非鉴权NV
 */
#define NV_AUTH_NO 2

/* ******************************* NV恢复表类型 ******************************** */
/* *
 * @brief NV备份恢复项类型
 */
enum nv_resume_item {
    NV_MANUFACTURE_ITEM = 0,
    NV_USER_ITEM,
    NV_SECURE_ITEM,
    NV_ITEM_BUTT
};
typedef unsigned long NV_RESUME_ITEM_ENUM_UINT32;

/* ****************************************************************************
 * 结构名    : nv_list_info_stru
 * 协议表格  :
 * ASN.1描述 :
 * 结构说明  : 获取NV列表信息
 * **************************************************************************** */
/* *
 * @brief NV列表信息结构体
 */
typedef struct tag_nv_list_info_stru {
    unsigned short usNvId;      /* *<NV ID */
    unsigned char ucNvModemNum; /* *<该NV支持的modem数量 */
    unsigned char ucNvAuthFlag; /* *<该NV是否为鉴权NV，2:不是,3:是 */
} nv_list_info_stru;            /* back */

/* *
 * @brief 读NV数据接口，可指定modem
 *
 * @attention
 * <ul><li>需要指定modemid</li></ul>

 * @param[in]  modemid, NV ID。
 * @param[in]  itemid, NV ID。
 * @param[in]  pdata, 存放NV数据的地址。
 * @param[in]  ulLength, NV数据长度。

 * @retval 0,表示函数执行成功。
 * @retval 0x100f0001, NV内存初始化失败。
 * @retval 0x100f0002, pdata为空或数据长度为0。
 * @retval 0x100f0016, 不存在此项NV。
 * @retval 0x100f0018, NV数据长度错误。
 * @retval 0x100f0039, 不支持的modem ID。
 */
unsigned int mdrv_nv_readex(unsigned int modemid, unsigned int itemid, void *pdata, unsigned int ulLength);

/* *
 * @brief 读NV数据接口，可指定modemID
 *
 * @attention
 * <ul><li>需要指定modemid；读取工厂区数据</li></ul>

 * @param[in]  modemid, NV ID。
 * @param[in]  itemid, NV ID。
 * @param[in]  pdata, 存放NV数据的地址。
 * @param[in]  ulLength, NV数据长度。

 * @retval 0,表示函数执行成功。
 * @retval 0x100f0002, pdata为空或数据长度为0。
 * @retval 0x100f0016, 不存在此项NV。
 * @retval 0x100f0018, NV数据长度错误。
 * @retval 0x100f0039, 不支持的modem ID。
 */
extern unsigned int mdrv_nv_readex_factory(unsigned int modemid, unsigned int itemid, void *pdata,
    unsigned int ulLength);
/* *
 * @brief 读NV数据接口，可指定modemID
 *
 * @todo 无用接口，待删除
 */
extern unsigned int mdrv_nv_readex_img(unsigned int modemid, unsigned int itemid, void *pdata, unsigned int ulLength);

/* *
 * @brief 写NV接口，默认写modem 0
 *
 * @par 描述:
 * 该函数默认写modem 0的NV，如果需要写指定modemid的NV，请调用mdrv_nv_writeex()接口。</li></ul>

 * @attention
 * <ul><li>请注意传入NV数据地址非空，数据长度正确。

 * @param[in]  itemid, NV ID。
 * @param[in]  pdata, NV数据首地址。
 * @param[in]  ulLength, NV数据长度。

 * @retval 0,表示函数执行成功。
 * @retval 0x100f0002, pdata为空或数据长度为0。
 * @retval 0x100f0016, 不存在此项NV。
 * @retval 0x100f0018, NV数据长度错误。
 * @retval 0x100f0039, 不支持的modem ID。

 */
unsigned int mdrv_nv_writeex(unsigned int modemid, unsigned int itemid, void *pdata, unsigned int ulLength);

/* *
 * @brief 读NV部分数据
 *
 * @par 描述:
 * 需要指定要读取的数据在该NV中的偏移地址

 * @attention
 * <ul><li>作用范围为所有的NV，如果传入的modemid超过了这个NV配置的modem个数，则会默认读取modem0的NV数据</li></ul>

 * @param[in]  modemid, modem ID。
 * @param[in]  itemid, NV ID。
 * @param[in]  ulOffset, 偏移地址。
 * @param[out]  pdata, 存放NV数据的地址。
 * @param[in]  ulLength, NV数据长度。

 * @retval 0,表示函数执行成功。
 * @retval 0x100f0001, NV内存初始化失败。
 * @retval 0x100f0002, pdata为空或数据长度为0。
 * @retval 0x100f0016, 不存在此项NV。
 * @retval 0x100f0018, NV数据长度错误。
 * @retval 0x100f0039, 不支持的modem ID。
 */
unsigned int mdrv_nv_read_partex(unsigned int modemid, unsigned int itemid, unsigned int ulOffset, void *pdata,
    unsigned int ulLength);

/* *
 * @brief 写NV部分数据
 *
 * @par 描述:
 * 需要指定要写入的数据在该NV中的偏移地址

 * @attention
 * <ul><li>作用范围为读写属性的NV，如果传入的modemid超过了这个NV配置的modem个数，则会默认写入modem0的NV数据</li></ul>

 * @param[in]  modemid, modem ID。
 * @param[in]  itemid, NV ID。
 * @param[in]  ulOffset, 偏移地址。
 * @param[in]  pdata, 写入NV数据的地址。
 * @param[in]  ulLength, NV数据长度。

 * @retval 0,表示函数执行成功。
 * @retval 0x100f0001, NV内存初始化失败。
 * @retval 0x100f0002, pdata为空或数据长度为0。
 * @retval 0x100f0016, 不存在此项NV。
 * @retval 0x100f0018, NV数据长度错误。
 * @retval 0x100f0039, 不支持的modem ID。
 */
unsigned int mdrv_nv_write_partex(unsigned int modemid, unsigned int itemid, unsigned int ulOffset, void *pdata,
    unsigned int ulLength);


/* *
 * @brief 获取NV长度
 *
 * @param[in]  itemid, NV ID。
 * @param[in]  pulLength, NV数据长度指针。

 * @retval 0,表示函数执行成功。
 * @retval 0x100f0001, NV内存初始化失败。
 * @retval 0x100f0002, pulLength为空。
 * @retval 0x100f0016, 不存在此项NV。
 */
unsigned int mdrv_nv_get_length(unsigned int itemid, unsigned int *pulLength);

/* *
 * @brief 读modem0 NV数据接口
 *
 * @par 描述:
 * 该接口默认读modem 0的NV数据，如需指定modemid，请使用mdrv_nv_readex()

 * @attention
 * <ul><li>读modem0 NV数据</li></ul>

 * @param[in]  itemid, NV ID。
 * @param[out]  pdata, 存放NV数据的地址。
 * @param[in]  ulLength, NV数据长度。

 * @retval 0,表示函数执行成功。
 * @retval 0x100f0001, NV内存初始化失败。
 * @retval 0x100f0002, pdata为空或数据长度为0。
 * @retval 0x100f0016, 不存在此项NV。
 * @retval 0x100f0018, NV数据长度错误。
 * @retval 0x100f0039, 不支持的modem ID。
 */
unsigned int mdrv_nv_read(unsigned int itemid, void *pdata, unsigned int ulLength);

/* *
 * @brief 写NV接口，默认写modem 0
 *
 * @par 描述:
 * 该函数默认写modem 0的NV，如果需要写指定modemid的NV，请调用mdrv_nv_writeex()接口。</li></ul>

 * @attention
 * <ul><li>请注意传入NV数据地址非空，数据长度正确。

 * @param[in]  itemid, NV ID。
 * @param[in]  pdata, NV数据首地址。
 * @param[in]  ulLength, NV数据长度。

 * @retval 0,表示函数执行成功。
 * @retval 0x100f0002, pdata为空或数据长度为0。
 * @retval 0x100f0016, 不存在此项NV。
 * @retval 0x100f0018, NV数据长度错误。
 * @retval 0x100f0039, 不支持的modem ID。

 */
unsigned int mdrv_nv_write(unsigned int itemid, void *pdata, unsigned int ulLength);

/* *
 * @brief 读modem0 NV部分数据
 *
 * @par 描述:
 * 需要指定要读取的数据在该NV中的偏移地址，如果需要指定modem，请使用mdrv_nv_write_partex()接口

 * @attention
 * <ul><li>作用范围为所有的NV</li></ul>

 * @param[in]  itemid, NV ID。
 * @param[in]  ulOffset, 偏移地址。
 * @param[out]  pdata, 存放NV数据的地址。
 * @param[in]  ulLength, NV数据长度。

 * @retval 0,表示函数执行成功。
 * @retval 0x100f0001, NV内存初始化失败。
 * @retval 0x100f0002, pdata为空或数据长度为0。
 * @retval 0x100f0016, 不存在此项NV。
 * @retval 0x100f0018, NV数据长度错误。
 * @retval 0x100f0039, 不支持的modem ID。
 */
unsigned int mdrv_nv_readpart(unsigned int itemid, unsigned int ulOffset, void *pdata, unsigned int ulLength);

/* *
 * @brief 写modem0 NV部分数据，从指定偏移处写
 *
 * @par 描述:
 * 需要指定要写入的数据在该NV中的偏移地址，如果需要指定modem，请使用mdrv_nv_write_partex()接口

 * @attention
 * <ul><li>作用范围为读写属性的NV</li></ul>

 * @param[in]  itemid, NV ID。
 * @param[in]  ulOffset, 偏移地址。
 * @param[in]  pdata, 写入NV数据的地址。
 * @param[in]  ulLength, NV数据长度。

 * @retval 0,表示函数执行成功。
 * @retval 0x100f0001, NV内存初始化失败。
 * @retval 0x100f0002, pdata为空或数据长度为0。
 * @retval 0x100f0016, 不存在此项NV。
 * @retval 0x100f0018, NV数据长度错误。
 * @retval 0x100f0039, 不支持的modem ID。
 */
unsigned int mdrv_nv_writepart(unsigned int itemid, unsigned int ulOffset, void *pdata, unsigned int ulLength);

/* *
 * @brief 维测写NV接口
 *
 * @par 描述:
 * 该接口仅将数据写入内存，不主动更新到flash。

 * @attention
 * <ul><li>维测写NV接口，只写入NV内存中,需要配合mdrv_nv_flush函数才能写入flash。</li></ul>
 * <ul><li>适合对时间要求高的场景，如CBT,HIMS等。</li></ul>

 * @param[in]  modemid, modem ID。
 * @param[in]  itemid, NV ID。
 * @param[in]  ulOffset, 偏移地址。
 * @param[in]  pdata, 写入NV数据的地址。
 * @param[in]  ulLength, NV数据长度。

 * @retval 0,表示函数执行成功。
 * @retval 0x100f0001, NV内存初始化失败。
 * @retval 0x100f0002, pdata为空或数据长度为0。
 * @retval 0x100f0016, 不存在此项NV。
 * @retval 0x100f0018, NV数据长度错误。
 * @retval 0x100f0039, 不支持的modem ID。
 * @retval 0xffffffff, 写入内存时出错。
 */
unsigned int mdrv_nv_om_write(unsigned int modemid, unsigned int itemid, unsigned int ulOffset, void *pdata,
    unsigned int ulLength);

/* *
 * @brief 将NV数据刷入flash

 * @par 描述:
 * 该接口将共享内存中最新的NV数据写入flash上的工作区和备份区

 * @attention
 * <ul><li>写入完成前会锁NV共享内存</li></ul>

 * @retval 0,表示函数执行成功。
 * @retval 0x100f0005, 申请内存失败。
 * @retval 0x100f0017, 发送ICC消息失败。
 * @retval 0x100f0035, 获取信号量超时。
 * @retval 0x100f0036, 释放信号量失败。
 */
unsigned int mdrv_nv_flush(void);

#ifdef __cplusplus
}
#endif
#endif // __MDRV_NVIM_COMMON_H__
