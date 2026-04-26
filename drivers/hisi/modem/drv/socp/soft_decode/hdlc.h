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


#ifndef __HDLC_H__
#define __HDLC_H__

#include <osl_types.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push)
#pragma pack(4)
/*
 * 2 宏定义
 */
#define OM_HDLC_FRAME_FLAG 0x7e /* HDLC帧标志位，固定为0x7e，区分不同的HDLC帧 */
#define OM_HDLC_ESC 0x7d        /* 转义字符，紧接其后的字符需要转义 */
#define OM_HDLC_ESC_MASK 0x20   /* 转义字符掩字 */

#define OM_HDLC_INIT_FCS 0xffff /* 计算FCS的初始FCS值 */
#define OM_HDLC_GOOD_FCS 0xf0b8 /* 计算结果为该FCS值时，说明FCS校验正确 */

#define OM_HDLC_MODE_HUNT 0x1 /* 解封装模式中bit0为1，表明未找到帧标志，没有开始解封装 */
#define OM_HDLC_MODE_ESC 0x2 /* 解封装模式中bit1为1，表明上个字符为转义字符，当前字符为真正数据的掩字 */

#define OM_HDLC_FCS_LEN 2 /* HDLC帧中FCS未转义时的长度(单位: Byte) */

#define OM_HDLC_TABLE_SIZE 256
/*
 * 3 枚举定义
 */
typedef enum tag_om_hdlc_result_e {
    HDLC_SUCC = 0,           /* 0    HDLC处理成功           */
    HDLC_NOT_HDLC_FRAME = 1, /* 1    没有完整的帧             */
    HDLC_FCS_ERROR,          /* 2    发生了校验错误        */
    HDLC_FRAME_DISCARD,      /* 3    发生异常，丢弃当前HDLC帧 */
    HDLC_BUFF_FULL,          /* 4    目的缓存满           */
    HDLC_PARA_ERROR,         /* 5    参数检查异常           */
    HDLC_BUTT
} om_hdlc_result_e;


typedef struct {
    u32 mode;            /* HDLC解析使用的内部参数，HDLC负责维护 */
    u32 length;          /* HDLC解析时内部维护的中间变量 */
    u32 decap_buff_size; /* HDLC解析内容存放BUFFER的长度，调用者负责指定 */
    u8 *decap_buff;      /* HDLC解析内容存放地址，由调用者负责指定空间 */
    u32 info_len;        /* HDLC解析成功的数据长度，HDLC负责维护 */
} om_hdlc_s;

extern void diag_hdlc_init(om_hdlc_s *hdlc_ctrl);

extern om_hdlc_result_e diag_hdlc_decap(om_hdlc_s *hdlc_ctrl, u8 tmp_char);

om_hdlc_result_e diag_hdlc_decap_process(om_hdlc_s *hdlc_ctrl, u8 tmp_char);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* OmHdlcInterface.h */
