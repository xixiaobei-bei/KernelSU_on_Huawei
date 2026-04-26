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
#ifndef __MDRV_VCOM_H__
#define __MDRV_VCOM_H__

#define com_id(n, i)     COM_ID_##n##_##i
#define com_name(n, i)  n#i
#define com_map(n, i)    com_name(#n, i), com_id(n, i)
#define MEXMSGLEN   4096
#define SYNCCOM     0
#define ASYNCOM     1
#define THROUGH     2
#define BINDPID     3
#define HOTPLUG     4

/**************************枚举定义*************************/
/**
* @brief vcom通道分类
*/
enum com_id_e {
    COM_ID_APPVCOM_0,
    COM_ID_APPVCOM_1,
    COM_ID_APPVCOM_2,
    COM_ID_APPVCOM_3,
    COM_ID_APPVCOM_4,
    COM_ID_APPVCOM_5,
    COM_ID_APPVCOM_6,
    COM_ID_APPVCOM_7,
    COM_ID_APPVCOM_8,
    COM_ID_APPVCOM_9,
    COM_ID_APPVCOM_10,
    COM_ID_APPVCOM_11,
    COM_ID_APPVCOM_12,
    COM_ID_APPVCOM_13,
    COM_ID_APPVCOM_14,
    COM_ID_APPVCOM_15,
    COM_ID_APPVCOM_16,
    COM_ID_APPVCOM_17,
    COM_ID_APPVCOM_18,
    COM_ID_APPVCOM_19,
    COM_ID_APPVCOM_20,
    COM_ID_APPVCOM_21,
    COM_ID_APPVCOM_22,
    COM_ID_APPVCOM_23,
    COM_ID_APPVCOM_24,
    COM_ID_APPVCOM_25,
    COM_ID_APPVCOM_26,
    COM_ID_APPVCOM_27,
    COM_ID_APPVCOM_28,
    COM_ID_APPVCOM_29,
    COM_ID_APPVCOM_30,
    COM_ID_APPVCOM_31,
    COM_ID_APPVCOM_32,
    COM_ID_APPVCOM_33,
    COM_ID_APPVCOM_34,
    COM_ID_APPVCOM_35,
    COM_ID_APPVCOM_36,
    COM_ID_APPVCOM_37,
    COM_ID_APPVCOM_38,
    COM_ID_APPVCOM_39,
    COM_ID_APPVCOM_40,
    COM_ID_APPVCOM_41,
    COM_ID_APPVCOM_42,
    COM_ID_APPVCOM_43,
    COM_ID_APPVCOM_44,
    COM_ID_APPVCOM_45,
    COM_ID_APPVCOM_46,
    COM_ID_APPVCOM_47,
    COM_ID_APPVCOM_48,
    COM_ID_APPVCOM_49,
    COM_ID_APPVCOM_50,
    COM_ID_APPVCOM_51,
    COM_ID_APPVCOM_52,
    COM_ID_APPVCOM_53,  // COM_ID_APPVCOM_ERRLOG
    COM_ID_APPVCOM_54,  // COM_ID_APPVCOM_TLLOG,
    COM_ID_APPVCOM_55,  // COM_ID_APPVCOM_LOG,
    COM_ID_APPVCOM_56,  // COM_ID_APPVCOM_LOG1
    COM_ID_APPVCOM_57,
    COM_ID_APPVCOM_58,
    COM_ID_APPVCOM_59,
    COM_ID_APPVCOM_60,
    COM_ID_APPVCOM_61,
    COM_ID_APPVCOM_62,
    COM_ID_APPVCOM_63,
    COM_ID_NMCTRLVCOM,
    COM_ID_BASTET,
    COM_ID_BASTET_IO,
    COM_ID_SIMHOTPLUG_1,
    COM_ID_MODEMSTATUS,
    COM_ID_MODEMSTATUS_W,
    COM_ID_GETSLICE,
    COM_ID_GETSLICE_W,
    COM_ID_PORTCFG_R,
    COM_ID_PORTCFG_W,
    COM_ID_ACT,
    BIND_PID,
    COM_ID_RNIC,
    COM_ID_BOTTOM,
};


enum com_id_ex_e {
    COM_EX_ID_POWER_STATE = COM_ID_BOTTOM,
};

#endif
