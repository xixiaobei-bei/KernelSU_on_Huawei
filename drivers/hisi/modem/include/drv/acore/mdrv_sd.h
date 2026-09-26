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
 * @brief   sd模块在acore上的对外头文件
 * @file    mdrv_sd.h
 * @author  wangshunxin
 * @version v1.0
 * @date    2019.11.26
 * @note    修改记录(版本号|修订日期|说明)
 * <ul><li>v1.0|2019.11.26|接口文档自动化</li></ul>
 * @since   仅765版本支持，5000之后版本不支持
 */
#ifndef __MDRV_ACORE_SD_H__
#define __MDRV_ACORE_SD_H__

#ifdef __cplusplus
extern "C" {
#endif

/* *
 * @brief 写标记
 */
#define WRFlAG 1
/* *
 * @brief 读标记
 */
#define RDFlAG 0

/* *
 * @brief sd卡相关操作
 */
enum sd_mmc_oprt_enum {
    SD_MMC_OPRT_FORMAT = 0,      /* *<格式化操作 */
    SD_MMC_OPRT_ERASE_ALL,       /* *<全擦操作 */
    SD_MMC_OPRT_ERASE_SPEC_ADDR, /* *<指定地址擦除操作 */
    SD_MMC_OPRT_WRITE,           /* *<写操作 */
    SD_MMC_OPRT_READ,            /* *<读操作 */
    SD_MMC_OPRT_BUTT
};

/* *
 * @brief 卡状态获取
 *
 * @par 描述:
 * 卡状态获取）
 *
 * @retval >=0，卡状态。
 * @retval <0，表示函数执行失败。
 *
 * @par 依赖:
 * <ul><li>mdrv_sd.h：该接口声明所在的头文件。</li></ul>
 */
unsigned int mdrv_sd_get_opptstatus(void);
/* *
 * @brief at^sd,SD卡操作，写，擦除，格式化操作。
 *
 * @par 描述:
 * at^sd,SD卡操作，写，擦除，格式化操作。
 *
 * @param[in]  type 操作类型
 * <ul>
 * <li>0 格式化SD卡内</li>
 * <li>1 擦除整个SD卡内容</li>
 * <li>2 用于指定地址内容的擦除操作，指定擦除的内容长度为512字节。擦除后的地址中写全1</li>
 * <li>3 写数据到SD卡的指定地址中，需要带第二个和第三个参数</li>
 * <li>4 读数据到SD卡的指定地址中</li>
 * </ul>
 * @param[in]  addr 卡地址，以512BYTE为一个单位
 * @param[in]  data 待写入的数据内容，表示512BYTE的内容，每个字节的内容均相同。
 * <ul>
 * <li>0 字节内容为0x00</li>
 * <li>1 字节内容为0x55</li>
 * <li>2 字节内容为0xAA</li>
 * <li>3 字节内容为0xFF</li>
 * </ul>
 * @param[in]  buf 读取数据的目的地址
 * @param[out]  err 具体的错误值
 * <ul>
 * <li>0 表示SD卡不在位</li>
 * <li>1 表示SD卡初始化失败</li>
 * <li>2 表示type参数非法，对应操作不支持(该错误由AT使用,不需要底软使用)</li>
 * <li>3 表示addr地址非法，超过SD卡本身容量</li>
 * <li>4 其他未知错误</li>
 * </ul>
 *
 * @retval !=0，表示函数执行失败。
 * @retval =0，表示函数执行成功。
 *
 * @par 依赖:
 * <ul><li>mdrv_sd.h：该接口声明所在的头文件。</li></ul>
 */
unsigned int mdrv_sd_at_process(unsigned int type, unsigned int addr, unsigned int data, unsigned char *buf,
    unsigned int *err);


#ifdef __cplusplus
}
#endif
#endif
