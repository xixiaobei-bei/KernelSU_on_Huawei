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

#include <mdrv_diag_system.h>
#include <osl_types.h>
#include <bsp_dump.h>
#include "soft_decode.h"
#include "hdlc.h"



#define THIS_MODU mod_soft_dec

/*
 * 该全局数组为FCS查找表，用于计算16位FCS
 * rfc1662: the lookup table used to calculate the FCS-16.
 */
u16 const g_diag_hdlc_fcs_tab[OM_HDLC_TABLE_SIZE] = {
    0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
    0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
    0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
    0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
    0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
    0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
    0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
    0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
    0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
    0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
    0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
    0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
    0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
    0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
    0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
    0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
    0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
    0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
    0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
    0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
    0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
    0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
    0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
    0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
    0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
    0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
    0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
    0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
    0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
    0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
    0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
    0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};

/*
 * 函 数 名： diag_hdlc_init
 * 功能描述: 该接口初始化HDLC实体的内部变量, 每个应用在第1次使用解封装功能
 * Om_HdlcDecap前需要调用一次该函数
 * 输入参数: om_hdlc_s *p_hdlc
 * 输出参数: om_hdlc_s *p_hdlc
 */
void diag_hdlc_init(om_hdlc_s *hdlc_ctrl)
{
    hdlc_ctrl->info_len = 0;
    hdlc_ctrl->length = 0;
    hdlc_ctrl->mode = OM_HDLC_MODE_HUNT;
}

/*
 * 函 数 名： OM_HdlcFcs
 * 功能描述: 计算FCS. 见RFC 1662 Appendix C and CCITT X.25 section 2.27.
 */
u16 diag_hdlc_fcs(const u8 *data, u32 data_len)
{
    u16 fcs = OM_HDLC_INIT_FCS;

    while (data_len--) {
        fcs = (fcs >> 8) ^ g_diag_hdlc_fcs_tab[(fcs ^ *data++) & 0xff];
    }

    return fcs;
}

static om_hdlc_result_e diag_hdlc_frame_flag_proc(om_hdlc_s *hdlc_ctrl, u8 tmp_char)
{
    u16 fcs;

    hdlc_ctrl->mode &= ~OM_HDLC_MODE_HUNT; /* 请标记 */

    if (hdlc_ctrl->length > OM_HDLC_FCS_LEN) { /* 该分支表示解析到(信息域+FCS)长度大于2 BYTE的帧 */
        fcs = diag_hdlc_fcs(hdlc_ctrl->decap_buff, hdlc_ctrl->length);
        if (fcs != OM_HDLC_GOOD_FCS) {
            hdlc_ctrl->length = 0;
            return HDLC_FCS_ERROR;
        }

        hdlc_ctrl->info_len = hdlc_ctrl->length - OM_HDLC_FCS_LEN;
        hdlc_ctrl->length = 0;
        return HDLC_SUCC;
    }

    /* 该分支若pstHdlc->ulLength为0，认为是正常情况：连续的OM_HDLC_FRAME_FLAG */
    /* 该分支若pstHdlc->length不为0，认为有HDLC帧，但(信息域+FCS)长度小于3，故丢弃 */
    hdlc_ctrl->length = 0;
    return HDLC_NOT_HDLC_FRAME;
}

static om_hdlc_result_e diag_hdlc_esc_proc(om_hdlc_s *hdlc_ctrl, u8 tmp_char)
{
    if (!(hdlc_ctrl->mode & OM_HDLC_MODE_ESC)) {
        hdlc_ctrl->mode |= OM_HDLC_MODE_ESC; /* 置标记 */
        return HDLC_NOT_HDLC_FRAME;
    } else {                                  /* 异常情况: 连续两个OM_HDLC_ESC */
        hdlc_ctrl->mode &= ~OM_HDLC_MODE_ESC; /* 清标记 */
        hdlc_ctrl->mode |= OM_HDLC_MODE_HUNT; /* 置标记 */
        hdlc_ctrl->length = 0;
        soft_decode_error("EscChar Error:0x%x!\n\r", tmp_char);
        return HDLC_FRAME_DISCARD;
    }
}

static om_hdlc_result_e diag_hdlc_other_proc(om_hdlc_s *hdlc_ctrl, u8 tmp_char)
{
    /* 判断目的BUFFER是否已满 */
    if (hdlc_ctrl->length >= hdlc_ctrl->decap_buff_size) { /* 异常情况: 解封装BUFFER不足 */
        hdlc_ctrl->mode |= OM_HDLC_MODE_HUNT;              /* 置标记 */
        hdlc_ctrl->length = 0;
        soft_decode_error("DstBuf is full:BufLen=0x%x\n\r", (s32)hdlc_ctrl->decap_buff_size);
        return HDLC_BUFF_FULL;
    }

    if (hdlc_ctrl->mode & OM_HDLC_MODE_ESC) {
        hdlc_ctrl->mode &= ~OM_HDLC_MODE_ESC; /* 清标记 */
        if ((tmp_char == (OM_HDLC_FRAME_FLAG ^ OM_HDLC_ESC_MASK)) || (tmp_char == (OM_HDLC_ESC ^ OM_HDLC_ESC_MASK))) {
            tmp_char ^= OM_HDLC_ESC_MASK;
        } else {                                  /* 异常情况: OM_HDLC_ESC后面的字符不正确 */
            hdlc_ctrl->mode |= OM_HDLC_MODE_HUNT; /* 置标记 */
            hdlc_ctrl->length = 0;
            soft_decode_error("EscChar Error:0x%x\n\r", tmp_char);
            return HDLC_FRAME_DISCARD;
        }
    }

    hdlc_ctrl->decap_buff[hdlc_ctrl->length] = tmp_char;
    hdlc_ctrl->length++;
    return HDLC_NOT_HDLC_FRAME;
}

om_hdlc_result_e diag_hdlc_decap_process(om_hdlc_s *hdlc_ctrl, u8 tmp_char)
{
    switch (tmp_char) {
        case OM_HDLC_FRAME_FLAG:
            return diag_hdlc_frame_flag_proc(hdlc_ctrl, tmp_char);

        case OM_HDLC_ESC:
            return diag_hdlc_esc_proc(hdlc_ctrl, tmp_char);

        default:
            return diag_hdlc_other_proc(hdlc_ctrl, tmp_char);
    }
}

om_hdlc_result_e diag_hdlc_decap(om_hdlc_s *hdlc_ctrl, u8 tmp_char)
{
    if (hdlc_ctrl == NULL) {
        soft_decode_error("Pointer Para is NULL\n\r");
        return HDLC_PARA_ERROR;
    }

    if (hdlc_ctrl->decap_buff == NULL) {
        soft_decode_error("DstBuff is NULL\n\r");
        return HDLC_PARA_ERROR;
    }

    /* 未找到帧头时，丢弃非OM_HDLC_FRAME_FLAG字符 */
    if ((hdlc_ctrl->mode & OM_HDLC_MODE_HUNT) && (tmp_char != OM_HDLC_FRAME_FLAG)) {
        hdlc_ctrl->length = 0;
        return HDLC_NOT_HDLC_FRAME;
    }

    return diag_hdlc_decap_process(hdlc_ctrl, tmp_char);
}

