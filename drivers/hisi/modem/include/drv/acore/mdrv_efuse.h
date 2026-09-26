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

/**
 * @brief   efuse模块在acore上的对外头文件
 * @file    mdrv_efuse.h
 * @author  panjunzhou
 * @version v1.0
 * @date    2019.11.29
 * @note    修改记录(版本号|修订日期|说明)
 * <ul><li>v1.0|2019.11.29|创建文件</li></ul>
 * @since   非融合版本
 */
#ifndef __MDRV_ACORE_EFUSE_H__
#define __MDRV_ACORE_EFUSE_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 读取chip_id。
 *
 * @par 描述:
 * 读取chip_id。
 *
 * @param[out]  buf 存放数据的buffer
 * @param[in]  len group数量（4字节为一个group）
 *
 * @retval =0，表示读取成功。
 * @retval !=0，表示读取失败。
 *
 * @attention
 * <ul><li>非手机平台该函数为桩。</li></ul>
 *
 * @par 依赖:
 * <ul><li>mdrv_efuse.h：该接口声明所在的头文件。</li></ul>
 */
int mdrv_efuse_get_chipid(unsigned char *buf, int length);

/**
 * @brief 读取die_id。
 *
 * @par 描述:
 * 读取die_id。
 *
 * @param[out]  buf 存放数据的buffer
 * @param[in]  len group数量（4字节为一个group）
 *
 * @retval =0，表示读取成功。
 * @retval !=0，表示读取失败。
 *
 * @par 依赖:
 * <ul><li>mdrv_efuse.h：该接口声明所在的头文件。</li></ul>
 */
int mdrv_efuse_get_dieid(unsigned char *buf, int length);

#define MDRV_EFUSE_IOCTL_CMD_GET_SOCID (0)
#define MDRV_EFUSE_IOCTL_CMD_SET_KCE (1)
#define MDRV_EFUSE_IOCTL_CMD_GET_KCE (2)
#define MDRV_EFUSE_IOCTL_CMD_SET_SECURESTATE (3)
#define MDRV_EFUSE_IOCTL_CMD_GET_SECURESTATE (4)
#define MDRV_EFUSE_IOCTL_CMD_SET_SECUREDEBUG (5)
#define MDRV_EFUSE_IOCTL_CMD_GET_SECUREDEBUG (6)
#define MDRV_EFUSE_IOCTL_CMD_SET_DBGEN (7)
#define MDRV_EFUSE_IOCTL_CMD_GET_DBGEN (8)
#define MDRV_EFUSE_IOCTL_CMD_SET_NIDEN (9)
#define MDRV_EFUSE_IOCTL_CMD_GET_NIDEN (10)
#define MDRV_EFUSE_IOCTL_CMD_SET_SPIDEN (11)
#define MDRV_EFUSE_IOCTL_CMD_GET_SPIDEN (12)
#define MDRV_EFUSE_IOCTL_CMD_SET_SPNIDEN (13)
#define MDRV_EFUSE_IOCTL_CMD_GET_SPNIDEN (14)
#define MDRV_EFUSE_IOCTL_CMD_SET_HIFIDBGEN (15)
#define MDRV_EFUSE_IOCTL_CMD_GET_HIFIDBGEN (16)
#define MDRV_EFUSE_IOCTL_CMD_SET_BBE16DBGEN (17)
#define MDRV_EFUSE_IOCTL_CMD_GET_BBE16DBGEN (18)
#define MDRV_EFUSE_IOCTL_CMD_SET_CSDEVICEEN (19)
#define MDRV_EFUSE_IOCTL_CMD_GET_CSDEVICEEN (20)
#define MDRV_EFUSE_IOCTL_CMD_SET_JTAGEN (21)
#define MDRV_EFUSE_IOCTL_CMD_GET_JTAGEN (22)
#define MDRV_EFUSE_IOCTL_CMD_SET_SECDBGRESET (23)
#define MDRV_EFUSE_IOCTL_CMD_GET_SECDBGRESET (24)
#define MDRV_EFUSE_IOCTL_CMD_SET_CSRESET (25)
#define MDRV_EFUSE_IOCTL_CMD_GET_CSRESET (26)
#define MDRV_EFUSE_IOCTL_CMD_SET_DFTSEL (27)
#define MDRV_EFUSE_IOCTL_CMD_GET_DFTSEL (28)
#define MDRV_EFUSE_IOCTL_CMD_SET_AUTHKEY (29)
#define MDRV_EFUSE_IOCTL_CMD_GET_AUTHKEY (30)
#define MDRV_EFUSE_IOCTL_CMD_SET_AUTHKEYRD (31)
#define MDRV_EFUSE_IOCTL_CMD_GET_AUTHKEYRD (32)
#define MDRV_EFUSE_IOCTL_CMD_SET_NSIVERIFY (33)
#define MDRV_EFUSE_IOCTL_CMD_GET_NSIVERIFY (34)
#define MDRV_EFUSE_IOCTL_CMD_SET_UARTDBGEN (35)
#define MDRV_EFUSE_IOCTL_CMD_GET_UARTDBGEN (36)
#define MDRV_EFUSE_IOCTL_CMD_SET_NXBBE32DBGEN (37)
#define MDRV_EFUSE_IOCTL_CMD_GET_NXBBE32DBGEN (38)
#define MDRV_EFUSE_IOCTL_CMD_SET_PDEDBGEN (39)
#define MDRV_EFUSE_IOCTL_CMD_GET_PDEDBGEN (40)
#define MDRV_EFUSE_IOCTL_CMD_SET_CCPUNIDEN (41)
#define MDRV_EFUSE_IOCTL_CMD_GET_CCPUNIDEN (42)
#define MDRV_EFUSE_IOCTL_CMD_SET_ACPUNIDEN (43)
#define MDRV_EFUSE_IOCTL_CMD_GET_ACPUNIDEN (44)


/* ************************************************
MDRV_EFUSE_IOCTL_CMD_GET_SOCID
arg     NA
buf     Buffer地址
len     Buffer长度，不小于8
ret     <0 -- ERROR
        0  -- OK
desciption      ^SOCID? 读取芯片的SOCID(256 bits)

MDRV_EFUSE_IOCTL_CMD_SET_KCE
arg     NA
buf     Buffer地址
len     Buffer长度，不小于4
ret     <0 -- ERROR
        0  -- OK
        1  -- REPEAT
desciption      ^KCE=<value> 烧入 128bit(765之前平台)/384bit (5000及以后)的KEY,用于image的加解密

MDRV_EFUSE_IOCTL_CMD_GET_KCE
arg     NA
buf     Buffer地址
len     Buffer长度，不小于4
ret     <0 -- ERROR
        0  -- OK
desciption      KCE不可读取，该命令用不到

MDRV_EFUSE_IOCTL_CMD_SET_SECURESTATE
arg     0  -- 未设置芯片安全状态
        1  -- 设置芯片为安全状态
        2  -- 设置芯片为非安全状态
buf     NA
len     NA
ret     <0 -- ERROR
        0  -- OK
        1  -- REPEAT
desciption      ^SECURESTATE=<value> 控制芯片的安全状态

MDRV_EFUSE_IOCTL_CMD_GET_SECURESTATE
arg     NA
buf     NA
len     NA
ret     <0 -- ERROR
        0  -- 未设置芯片安全状态
        1  -- 设置芯片为安全状态
        2  -- 设置芯片为非安全状态
desciption      ^SECURESTATE? 获取芯片的安全状态

MDRV_EFUSE_IOCTL_CMD_SET_SECUREDEBUG
arg     0  -- Secure privilege debug
        1  -- 通过密码保护的方式进行鉴权，鉴权通过之后才能使能debug功能
        2  -- 控制信号由eFuse中的烧写值确定
        3  -- DCU决定
buf     NA
len     NA
ret     <0 -- ERROR
        0  -- OK
        1  -- REPEAT
desciption      ^SECUREDEBUG=<value> 设置终端芯片的DEBUG 模式

MDRV_EFUSE_IOCTL_CMD_GET_SECUREDEBUG
arg     NA
buf     NA
len     NA
ret     <0 -- ERROR
        0  -- Secure privilege debug
        1  -- 通过密码保护的方式进行鉴权，鉴权通过之后才能使能debug功能
        2  -- 控制信号由eFuse中的烧写值确定
        3  -- DCU决定
desciption      ^SECUREDEBUG? 获取芯片的DEBUG 模式

MDRV_EFUSE_IOCTL_CMD_SET_DBGEN
arg     0  -- 打开
        1  -- 关闭
buf     NA
len     NA
ret     <0 -- ERROR
        0  -- OK
        1  -- REPEAT
desciption      ^DEBUGEN=<value> 控制是否开启非安全世界侵入式调试功能(只在AT^SECUREDEBUG为0b10时有效)

MDRV_EFUSE_IOCTL_CMD_GET_DBGEN
arg     NA
buf     NA
len     NA
ret     <0 -- ERROR
        0  -- 打开
        1  -- 关闭
desciption      ^DEBUGEN? 获取是否开启非安全世界侵入式调试功能(只在AT^SECUREDEBUG为0b10时有效)

MDRV_EFUSE_IOCTL_CMD_SET_NIDEN
arg     0  -- 打开
        1  -- 关闭
buf     NA
len     NA
ret     <0 -- ERROR
        0  -- OK
        1  -- REPEAT
desciption      ^NIDEN=<value> 控制是否开启非安全世界非侵入式调试功能(只在AT^SECUREDEBUG为0b10时有效)

MDRV_EFUSE_IOCTL_CMD_GET_NIDEN
arg     NA
buf     NA
len     NA
ret     <0 -- ERROR
        0  -- 打开
        1  -- 关闭
desciption      ^NIDEN? 获取是否开启非安全世界非侵入式调试功能(只在AT^SECUREDEBUG为0b10时有效)

MDRV_EFUSE_IOCTL_CMD_SET_SPIDEN
arg     0  -- 打开
        1  -- 关闭
buf     NA
len     NA
ret     <0 -- ERROR
        0  -- OK
        1  -- REPEAT
desciption      ^SPIDEN=<value> 控制是否开启安全世界侵入式调试功能(只在AT^SECUREDEBUG为0b10时有效)

MDRV_EFUSE_IOCTL_CMD_GET_SPIDEN
arg     NA
buf     NA
len     NA
ret     <0 -- ERROR
        0  -- 打开
        1  -- 关闭
desciption      ^SPIDEN? 获取是否开启安全世界侵入式调试功能(只在AT^SECUREDEBUG为0b10时有效)

MDRV_EFUSE_IOCTL_CMD_SET_SPNIDEN
arg     0  -- 打开
        1  -- 关闭
buf     NA
len     NA
ret     <0 -- ERROR
        0  -- OK
        1  -- REPEAT
desciption      ^SPNIDEN=<value> 控制是否开启安全世界非侵入式调试功能(只在AT^SECUREDEBUG为0b10时有效)

MDRV_EFUSE_IOCTL_CMD_GET_SPNIDEN
arg     NA
buf     NA
len     NA
ret     <0 -- ERROR
        0  -- 打开
        1  -- 关闭
desciption      ^SPNIDEN? 获取是否开启安全世界非侵入式调试功能(只在AT^SECUREDEBUG为0b10时有效)

MDRV_EFUSE_IOCTL_CMD_SET_HIFIDBGEN
arg     0  -- 打开
        1  -- 关闭
buf     NA
len     NA
ret     <0 -- ERROR
        0  -- OK
        1  -- REPEAT
desciption      ^HIFIDBGEN=<value> 控制是否开启HiFi调试功能(只在AT^SECUREDEBUG为0b10时有效)

MDRV_EFUSE_IOCTL_CMD_GET_HIFIDBGEN
arg     NA
buf     NA
len     NA
ret     <0 -- ERROR
        0  -- 打开
        1  -- 关闭
desciption      ^HIFIDBGEN? 获取是否开启HiFi调试功能(只在AT^SECUREDEBUG为0b10时有效)

MDRV_EFUSE_IOCTL_CMD_SET_BBE16DBGEN
arg     0  -- 打开
        1  -- 关闭
buf     NA
len     NA
ret     <0 -- ERROR
        0  -- OK
        1  -- REPEAT
desciption      ^BBE16DBGEN=<value> 控制是否开启BBE16调试功能(只在AT^SECUREDEBUG为0b10时有效)

MDRV_EFUSE_IOCTL_CMD_GET_BBE16DBGEN
arg     NA
buf     NA
len     NA
ret     <0 -- ERROR
        0  -- 打开
        1  -- 关闭
desciption      ^BBE16DBGEN? 获取是否开启BBE16调试功能(只在AT^SECUREDEBUG为0b10时有效)

MDRV_EFUSE_IOCTL_CMD_SET_CSDEVICEEN
arg     0  -- 打开
        1  -- 关闭
buf     NA
len     NA
ret     <0 -- ERROR
        0  -- OK
        1  -- REPEAT
desciption      ^CSDEVICEEN=<value> 控制是否开启Coresight调试功能(只在AT^SECUREDEBUG为0b10时有效)

MDRV_EFUSE_IOCTL_CMD_GET_CSDEVICEEN
arg     NA
buf     NA
len     NA
ret     <0 -- ERROR
        0  -- 打开
        1  -- 关闭
desciption      ^CSDEVICEEN? 获取是否开启Coresight调试功能(只在AT^SECUREDEBUG为0b10时有效)

MDRV_EFUSE_IOCTL_CMD_SET_JTAGEN
arg     0  -- 打开
        1  -- 关闭
buf     NA
len     NA
ret     <0 -- ERROR
        0  -- OK
        1  -- REPEAT
desciption      ^JTAGEN=<value> 控制是否开启JTAG调试功能

MDRV_EFUSE_IOCTL_CMD_GET_JTAGEN
arg     NA
buf     NA
len     NA
ret     <0 -- ERROR
        0  -- 打开
        1  -- 关闭
desciption      ^JTAGEN? 获取是否开启JTAG调试功能

MDRV_EFUSE_IOCTL_CMD_SET_SECDBGRESET
arg     0  -- 默认不复位
        1  -- 复位
buf     NA
len     NA
ret     <0 -- ERROR
        0  -- OK
        1  -- REPEAT
desciption      ^SECDBGRESET=<value> 控制是否在进行安全世界调试时，临时复位SEC Engine

MDRV_EFUSE_IOCTL_CMD_GET_SECDBGRESET
arg     NA
buf     NA
len     NA
ret     <0 -- ERROR
        0  -- 默认不复位
        1  -- 复位
desciption      ^SECDBGRESET? 获取是否在进行安全世界调试时，临时复位SEC Engine

MDRV_EFUSE_IOCTL_CMD_SET_CSRESET
arg     0  -- 默认不复位
        1  -- 复位
buf     NA
len     NA
ret     <0 -- ERROR
        0  -- OK
        1  -- REPEAT
desciption      ^CSRESET=<value> 控制是否在用Coresight调试时，临时复位SEC Engine

MDRV_EFUSE_IOCTL_CMD_GET_CSRESET
arg     NA
buf     NA
len     NA
ret     <0 -- ERROR
        0  -- 默认不复位
        1  -- 复位"
desciption      ^CSRESET? 获取是否在用Coresight调试时，临时复位SEC Engine

MDRV_EFUSE_IOCTL_CMD_SET_DFTSEL
arg     0  -- 打开状态
        1  -- 关闭状态
        2/3 -- 永久关闭状态
buf     NA
len     NA
ret     <0 -- ERROR
        0  -- OK
        1  -- REPEAT
desciption      ^DFTSEL=<value> 控制DFTSCAN 的等级

MDRV_EFUSE_IOCTL_CMD_GET_DFTSEL
arg     NA
buf     NA
len     NA
ret     <0 -- ERROR
        0  -- 打开状态
        1  -- 关闭状态
        2/3 -- 永久关闭状态
desciption      ^DFTSEL? 获取DFTSCAN 的等级

MDRV_EFUSE_IOCTL_CMD_SET_AUTHKEY
arg     NA
buf     Buffer地址
len     Buffer长度，不小于2
ret     <0 -- ERROR
        0  -- OK
        1  -- REPEAT
desciption      ^SETKEY=<value> 烧入 64 bits 的 DFT 和密码鉴权的认证密码

MDRV_EFUSE_IOCTL_CMD_GET_AUTHKEY
arg     NA
buf     Buffer地址
len     Buffer长度，不小于2
ret     <0 -- ERROR
        0  -- OK
desciption      ^GETKEYINFO=<value> 读取芯片64 bits 的 DFT 和密码鉴权的认证密码

MDRV_EFUSE_IOCTL_CMD_SET_AUTHKEYRD
arg     0  -- 软件可读
        1  -- 软件不可读
buf     NA
len     NA
ret     <0 -- ERROR
        0  -- OK
        1  -- REPEAT
desciption      ^AUTHKEYRD=<value> 控制AUTHKEY 是否可读

MDRV_EFUSE_IOCTL_CMD_GET_AUTHKEYRD
arg     NA
buf     NA
len     NA
ret     <0 -- ERROR
        0  -- 软件可读
        1  -- 软件不可读
desciption      ^AUTHKEYRD? 获取AUTHKEY 是否可读

MDRV_EFUSE_IOCTL_CMD_SET_NSIVERIFY
arg     0  -- 关闭校验
        1  -- 打开校验
buf     NA
len     NA
ret     <0 -- ERROR
        0  -- OK
        1  -- REPEAT
desciption      ^NSIVERIFY=<value> 控制是否开启非安全产品镜像完整性校验

MDRV_EFUSE_IOCTL_CMD_GET_NSIVERIFY
arg     NA
buf     NA
len     NA
ret     <0 -- ERROR
        0  -- 关闭校验
        1  -- 打开校验
desciption      ^NSIVERIFY? 获取是否开启非安全产品镜像完整性校验

************************************************ */
/**
 * @brief Efuse中相关域的烧写和读取。
 *
 * @par 描述:
 * Efuse中相关域的烧写和读取。
 *
 * @param[in]  cmd MDRV_EFUSE_IOCTL_CMD_ 相关的宏，每个宏与同名的AT命令对应
 * @param[in]  arg 与该命令对应的参数
 * @param[out]  buf 存放数据的buffer
 * @param[in]  len group数量（4字节为一个group）
 *
 * @retval =0，表示执行成功。
 * @retval !=0，表示执行失败。
 *
 * @par 依赖:
 * <ul><li>mdrv_efuse.h：该接口声明所在的头文件。</li></ul>
 */
int mdrv_efuse_ioctl(int cmd, int arg, unsigned char *buf, int len);

#ifdef __cplusplus
}
#endif
#endif
