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

#ifndef _MDRV_OM_COMMON_H_
#define _MDRV_OM_COMMON_H_

#ifdef _cplusplus
extern "C"
{
#endif

#define NR_MODID_TAG      (0xB)
#define MODID_TAG         (32)
#define MODID_SYS         (28)
#define MODID_MODULE      (24)
#define MODID_CODE        (16)
/************************************************
31-28bit	 27-24bit	  23-16bit	  15-0bit
NRTAG        异常子系统	  异常模块	  错误码，各模块自由定义
**************************************************/

typedef enum nr_sys
{
    NR_SYSTEM_AP        = 0,
    NR_SYSTEM_CCPU      = 1,
    NR_SYSTEM_L2HAC     = 2,
    NR_SYSTEM_HL1C      = 3,
    NR_SYSTEM_LL1C      = 4,
}NR_SYS;



typedef enum nr_module
{
    MODULE_BSP     = 0,
    MODULE_MSP     ,
    MODULE_OSA     ,
    MODULE_OAM     ,
    MODULE_NAS     ,
    MODULE_AS      ,
    MODULE_PS      ,
    MODULE_PHY     ,
}NR_MODULE;


#define OM_MODID_HEAD(__subsys,__module)   ((unsigned int)(NR_MODID_TAG << (MODID_SYS))|(unsigned int)((__subsys)<< MODID_MODULE)| (unsigned int)((__module) << MODID_CODE))


#define OM_GET_MODULE(error_code)          ((unsigned int)(((error_code)&((unsigned int)(0xff << MODID_CODE))>> MODID_CODE)))



enum OM_FIELD_ID_ENUM
{
    OM_AP_FIELD_BEGIN   = (0x01200000),
    OM_AP_OSA           = (OM_AP_FIELD_BEGIN),
    OM_AP_DIAG          = (OM_AP_FIELD_BEGIN + 1),
    OM_AP_FIELD_END,
#ifndef __OS_NRCCPU__
    OM_CP_FIELD_BEGIN   = (0x02200000),
    OM_CP_OSA           = (OM_CP_FIELD_BEGIN),
    OM_CP_MSP_SLEEP     = (OM_CP_FIELD_BEGIN + 1),
    OM_CP_TLPS          = (OM_CP_FIELD_BEGIN + 2),
    OM_CP_FTM_MNTN      = (OM_CP_FIELD_BEGIN + 3),
    OM_CP_GUAS          = (OM_CP_FIELD_BEGIN + 4),
    OM_CP_DIAG          = (OM_CP_FIELD_BEGIN + 5),
    OM_CP_GUNAS         = (OM_CP_FIELD_BEGIN + 6),
    OM_CP_CPROC         = (OM_CP_FIELD_BEGIN + 7),
    OM_CP_CAS           = (OM_CP_FIELD_BEGIN + 8),
    OM_CP_CNAS          = (OM_CP_FIELD_BEGIN + 9),
    OM_CP_MSCC          = (OM_CP_FIELD_BEGIN + 10),
    OM_CP_CTTF          = (OM_CP_FIELD_BEGIN + 11),
    OM_CP_USIMM0        = (OM_CP_FIELD_BEGIN + 12),
    OM_CP_USIMM1        = (OM_CP_FIELD_BEGIN + 13),
    OM_CP_TLPS1         = (OM_CP_FIELD_BEGIN + 14),
    OM_CP_GUPHY         = (OM_CP_FIELD_BEGIN + 15),
    OM_CP_RFDSP         = (OM_CP_FIELD_BEGIN + 16),
    OM_CP_GUCSLEEP      = (OM_CP_FIELD_BEGIN + 17),
    OM_CP_TAF           = (OM_CP_FIELD_BEGIN + 18),
    OM_CP_FIELD_END,
#else
    OM_CP_FIELD_BEGIN   = (0x07200000),
    OM_CP_OSA           = (OM_CP_FIELD_BEGIN),
    OM_CP_MSP_SLEEP     = (OM_CP_FIELD_BEGIN + 1),
    OM_CP_TLPS          = (OM_CP_FIELD_BEGIN + 2),
    OM_CP_FTM_MNTN      = (OM_CP_FIELD_BEGIN + 3),
    OM_CP_GUAS          = (OM_CP_FIELD_BEGIN + 4),
    OM_CP_DIAG          = (OM_CP_FIELD_BEGIN + 5),
    OM_CP_GUNAS         = (OM_CP_FIELD_BEGIN + 6),
    OM_CP_CPROC         = (OM_CP_FIELD_BEGIN + 7),
    OM_CP_CAS           = (OM_CP_FIELD_BEGIN + 8),
    OM_CP_CNAS          = (OM_CP_FIELD_BEGIN + 9),
    OM_CP_MSCC          = (OM_CP_FIELD_BEGIN + 10),
    OM_CP_CTTF          = (OM_CP_FIELD_BEGIN + 11),
    OM_CP_USIMM0        = (OM_CP_FIELD_BEGIN + 12),
    OM_CP_USIMM1        = (OM_CP_FIELD_BEGIN + 13),
    OM_CP_TLPS1         = (OM_CP_FIELD_BEGIN + 14),
    OM_CP_GUPHY         = (OM_CP_FIELD_BEGIN + 15),
    OM_CP_RFDSP         = (OM_CP_FIELD_BEGIN + 16),
    OM_CP_GUCSLEEP      = (OM_CP_FIELD_BEGIN + 17),
    OM_CP_NRNAS         = (OM_CP_FIELD_BEGIN + 18),

#endif
    OM_HAC_FIELD_BEGIN   = (0x08200000),
    OM_HAC_OSA           = (OM_HAC_FIELD_BEGIN),
    OM_HAC_MSP_SLEEP     = (OM_HAC_FIELD_BEGIN + 1),
    OM_HAC_TLPS          = (OM_HAC_FIELD_BEGIN + 2),
    OM_HAC_FTM_MNTN      = (OM_HAC_FIELD_BEGIN + 3),
    OM_HAC_GUAS          = (OM_HAC_FIELD_BEGIN + 4),
    OM_HAC_DIAG          = (OM_HAC_FIELD_BEGIN + 5),
    OM_HAC_GUNAS         = (OM_HAC_FIELD_BEGIN + 6),
    OM_HAC_CPROC         = (OM_HAC_FIELD_BEGIN + 7),
    OM_HAC_CAS           = (OM_HAC_FIELD_BEGIN + 8),
    OM_HAC_CNAS          = (OM_HAC_FIELD_BEGIN + 9),
    OM_HAC_MSCC          = (OM_HAC_FIELD_BEGIN + 10),
    OM_HAC_CTTF          = (OM_HAC_FIELD_BEGIN + 11),
    OM_HAC_USIMM0        = (OM_HAC_FIELD_BEGIN + 12),
    OM_HAC_USIMM1        = (OM_HAC_FIELD_BEGIN + 13),
    OM_HAC_TLPS1         = (OM_HAC_FIELD_BEGIN + 14),
    OM_HAC_GUPHY         = (OM_HAC_FIELD_BEGIN + 15),
    OM_HAC_RFDSP         = (OM_HAC_FIELD_BEGIN + 16),
    OM_HAC_GUCSLEEP      = (OM_HAC_FIELD_BEGIN + 17),
    OM_HAC_FIELD_END,


};



enum
{
    DUMP_OUT_VOICE = 0,
    DUMP_IN_VOICE = 1,
    DUMP_VOICE_BUTT,
};
typedef int dump_handle;
typedef void (*dump_hook)(void);
typedef void (*dump_notify)(void*);


/*****************************************************************************
 * 函 数 名  : mdrv_om_system_error
 * 功能描述  : 软件异常记录接口，完成异常文件记录之后主动复位单板。
 * 输入参数  : modId:   复位原因
 *             arg1:    用户自定义参数1
 *             arg2:    用户自定义参数2
 *             arg3:    用户自定义参数3，
 *                       buf指针，dump程序会自动拷贝到dump内存，
 *                       在dump文件中的field以用户数据显示。
 *             arg3Length:  arg所指buf的长度最大长度max_len(v7为4k，v8为1k)
 *                       超过max_len只拷贝起始max_len数据
 * 输出参数  : 无。
 * 返 回 值  : 无。
 *****************************************************************************/
void mdrv_om_system_error(int modId, int arg1, int arg2, char * arg3, int arg3Length);

/*****************************************************************************
 * 函 数 名  : mdrv_om_register_field
 * 功能描述  : 记录异常复位信息的地址申请函数，该接口申请出来的内存中存入的数据，
 *             在system_error的时候会保存到dump文件对应的field区域中。
 * 输入参数  : field_id :   申请buffer对应的field id
 *             field_name:  field名称，用于在解析工具中显示。
 *             virt_addr:   自带地址注册的虚拟地址，如果为空，表示从dump分配内存。
 *             phy_addr:    自带地址注册的物理地址
 *             length:      申请buffer的长度
 *             version:     申请buffer的版本号
 *
 * 输出参数  : NULL:  申请buf失败
 *
 * 返 回 值  : 无。
 *****************************************************************************/
unsigned char* mdrv_om_register_field(unsigned int field_id, char * field_name, void * virt_addr,
                            void * phy_addr, unsigned int length, unsigned short version);

/*****************************************************************************
 * 函 数 名  : mdrv_om_get_field_addr
 * 功能描述  : 获取field_id对应的dump内存的虚拟地址，可跨核使用
 *
 * 输入参数  : field_id: 通过mdrv_om_register_field接口向dump申请内存的ID
 *
 * 输出参数  : 无
 *
 * 返 回 值  : >0   成功，返回field虚拟地址
 *             0    失败
 *
 *****************************************************************************/
unsigned char * mdrv_om_get_field_addr(unsigned int field_id);

/*****************************************************************************
 * 函 数 名  : mdrv_om_get_field_phy_addr
 * 功能描述  : 获取field_id对应的dump内存的物理地址，可跨核使用
 *
 * 输入参数  : field_id: 通过mdrv_om_register_field接口向dump申请内存的ID
 *
 * 输出参数  : 无
 *
 * 返 回 值  : >0   成功，返回field物理地址
 *             0    失败
 *
 *****************************************************************************/
unsigned char * mdrv_om_get_field_phy_addr(unsigned int field_id);

/*****************************************************************************
 * 函 数 名  : mdrv_om_register_callback
 * 功能描述  : DUMP处理钩子注册函数，dump模块提供注册接口，其他模块注册处理函数，
 * system_error的时候会依次调用执行注册进来的处理函数。
 *
 * 输入参数  : name :   异常注册的模块名称
 *             pFunc:   系统异常时的回调函数
 *
 * 输出参数  : 无
 *
 * 返 回 值     : >0  成功，返回注册结点句柄
 *        -1, DUMP_HANDLE_INVALID  失败
 *
 *****************************************************************************/
dump_handle mdrv_om_register_callback(char * name, dump_hook pFunc);

/*****************************************************************************
  函 数 名  : mdrv_om_unregister_callback
  功能描述  : 删除DUMP钩子函数。

  输入参数  : handle :   mdrv_om_register_callback注册获取到的句柄

  输出参数  : 无

  返 回 值     : 0  成功
  -1   失败

 *****************************************************************************/
 int mdrv_om_unregister_callback(dump_handle handle);


#ifdef _cplusplus
}
#endif
#endif


