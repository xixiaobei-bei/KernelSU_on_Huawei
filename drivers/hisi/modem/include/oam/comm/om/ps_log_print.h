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



#ifndef PS_LOG_PRINT_H
#define PS_LOG_PRINT_H

#include "dopra_def.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define PRINT_OFF             (0)
#define PRINT_ON              (1)
#define PRINT_SWITCH          (PRINT_OFF)

#define PRINT2LAYER_SWITCH    (PRINT_ON)



#ifdef MODEM_FUSION_VERSION
#define LPS_LOG_CAT_PARA(a, b, c, d) a b c d
#endif

#if (VOS_OS_VER == VOS_WIN32)
#define LPS_LOG(ModulePID, SubMod, Level, pcString) \
            vos_printf(" %s, %d, %s\r\n ", __FILE__, __LINE__,pcString)

#define LPS_LOG1(ModulePID, SubMod, Level, pcString, lPara1) \
            vos_printf (" %s, %d, %s, %d, \r\n ",  __FILE__, __LINE__, pcString, lPara1)

#define LPS_LOG2(ModulePID, SubMod, Level, pcString, lPara1, lPara2) \
            vos_printf (" %s, %d, %s, %d, %d \r\n ",  __FILE__, __LINE__, pcString, lPara1, lPara2)

#define LPS_LOG3(ModulePID, SubMod, Level, pcString, lPara1, lPara2, lPara3) \
            vos_printf (" %s, %d, %s, %d, %d, %d\r\n ",  __FILE__, __LINE__, pcString, lPara1, lPara2, lPara3)

#define LPS_LOG4(ModulePID, SubMod, Level, pcString, lPara1, lPara2, lPara3, lPara4) \
            vos_printf (" %s, %d, %s, %d, %d, %d, %d\r\n ",  __FILE__, __LINE__,pcString, lPara1, lPara2, lPara3, lPara4)

#else
#if (!defined(_lint) && (PRINT_SWITCH == PRINT_OFF))
#define LPS_LOG(ModulePID, SubMod, Level, pcString)

#define LPS_LOG1(ModulePID, SubMod, Level, pcString, lPara1)

#define LPS_LOG2(ModulePID, SubMod, Level, pcString, lPara1, lPara2)

#define LPS_LOG3(ModulePID, SubMod, Level, pcString, lPara1, lPara2, lPara3 )

#define LPS_LOG4(ModulePID, SubMod, Level, pcString, lPara1, lPara2, lPara3, lPara4)

#else
#ifdef LOG_SWITCH_OM_FLG
#define LPS_LOG(ModulePID, SubMod, Level, pcString) \
            {/*lint -e778*/OM_Log( __FILE__, __LINE__,(VOS_UINT8)((0x00ff&(ModulePID))), SubMod, LogLevelMap(Level), pcString ); /*lint +e778*/}

#define LPS_LOG1(ModulePID, SubMod, Level, pcString, lPara1) \
            {/*lint -e778*/OM_Log1( __FILE__, __LINE__, (VOS_UINT8)((0x00ff&(ModulePID))), SubMod, LogLevelMap(Level), pcString, (VOS_INT32)(lPara1)); /*lint +e778*/}

#define LPS_LOG2(ModulePID, SubMod, Level, pcString, lPara1, lPara2) \
            {/*lint -e778*/OM_Log2( __FILE__, __LINE__, (VOS_UINT8)((0x00ff&(ModulePID))), SubMod, LogLevelMap(Level), pcString, (VOS_INT32)(lPara1), (VOS_INT32)(lPara2)); /*lint +e778*/}

#define LPS_LOG3(ModulePID, SubMod, Level, pcString, lPara1, lPara2, lPara3 ) \
            {/*lint -e778*/OM_Log3( __FILE__, __LINE__, (VOS_UINT8)((0x00ff&(ModulePID))), SubMod, LogLevelMap(Level), pcString, (VOS_INT32)(lPara1), (VOS_INT32)(lPara2), (VOS_INT32)(lPara3)); /*lint +e778*/}

#define LPS_LOG4(ModulePID, SubMod, Level, pcString, lPara1, lPara2, lPara3, lPara4) \
            {/*lint -e778*/OM_Log4( __FILE__, __LINE__, (VOS_UINT8)((0x00ff&(ModulePID))), SubMod, LogLevelMap(Level), pcString, (VOS_INT32)(lPara1),((VOS_INT32)lPara2), (VOS_INT32)(lPara3), (VOS_INT32)(lPara4)); /*lint +e778*/}
#else
#ifndef MODEM_FUSION_VERSION

#define LPS_LOG(ModulePID, SubMod, Level, pcString) \
            LPS_OM_LOG( __FILE__, __LINE__, ModulePID, Level, pcString )

#define LPS_LOG1(ModulePID, SubMod, Level, pcString, lPara1) \
            LPS_OM_LOG1( __FILE__, __LINE__, ModulePID, Level, pcString, (VOS_INT32)(lPara1))

#define LPS_LOG2(ModulePID, SubMod, Level, pcString, lPara1, lPara2) \
            LPS_OM_LOG2( __FILE__, __LINE__, ModulePID, Level, pcString, (VOS_INT32)(lPara1), (VOS_INT32)(lPara2))

#define LPS_LOG3(ModulePID, SubMod, Level, pcString, lPara1, lPara2, lPara3 ) \
            LPS_OM_LOG3( __FILE__, __LINE__, ModulePID, Level, pcString, (VOS_INT32)(lPara1), (VOS_INT32)(lPara2), (VOS_INT32)(lPara3))

#define LPS_LOG4(ModulePID, SubMod, Level, pcString, lPara1, lPara2, lPara3, lPara4) \
            LPS_OM_LOG4( __FILE__, __LINE__, ModulePID, Level, pcString, (VOS_INT32)(lPara1),((VOS_INT32)lPara2), (VOS_INT32)(lPara3), (VOS_INT32)(lPara4))
#else

#define LPS_LOG(ModulePID, SubMod, Level, pcString) \
            LPS_OM_LOG( __FILE__, __LINE__, ModulePID, Level, (LPS_LOG_CAT_PARA(__FILE__, " ", pcString, " ")))

#define LPS_LOG1(ModulePID, SubMod, Level, pcString, lPara1) \
            LPS_OM_LOG1( __FILE__, __LINE__, ModulePID, Level, (LPS_LOG_CAT_PARA(__FILE__, " ", pcString, " , %d")), (VOS_INT32)(lPara1))

#define LPS_LOG2(ModulePID, SubMod, Level, pcString, lPara1, lPara2) \
            LPS_OM_LOG2( __FILE__, __LINE__, ModulePID, Level, (LPS_LOG_CAT_PARA(__FILE__, " ", pcString, " , %d, %d")), (VOS_INT32)(lPara1), (VOS_INT32)(lPara2))

#define LPS_LOG3(ModulePID, SubMod, Level, pcString, lPara1, lPara2, lPara3 ) \
            LPS_OM_LOG3( __FILE__, __LINE__, ModulePID, Level, (LPS_LOG_CAT_PARA(__FILE__, " ", pcString, " , %d, %d, %d")), (VOS_INT32)(lPara1), (VOS_INT32)(lPara2), (VOS_INT32)(lPara3))

#define LPS_LOG4(ModulePID, SubMod, Level, pcString, lPara1, lPara2, lPara3, lPara4) \
            LPS_OM_LOG4( __FILE__, __LINE__, ModulePID, Level, (LPS_LOG_CAT_PARA(__FILE__, " ", pcString, " , %d, %d, %d, %d")), (VOS_INT32)(lPara1),((VOS_INT32)lPara2), (VOS_INT32)(lPara3), (VOS_INT32)(lPara4))


#endif
#endif
#endif
#endif

#if (VOS_OS_VER == VOS_WIN32)
#ifndef L2_LIBFUZZ_TEST
#define LPS_PRINT_LOG(ModulePID, pcString) \
            vos_printf(" %s, %d, %s\r\n ", __FILE__, __LINE__,pcString)

#define LPS_PRINT_LOG1(ModulePID, pcString, lPara1) \
            vos_printf (" %s, %d, %s, %d, \r\n ",  __FILE__, __LINE__, pcString, lPara1)

#define LPS_PRINT_LOG2(ModulePID, pcString, lPara1, lPara2) \
            vos_printf (" %s, %d, %s, %d, %d \r\n ",  __FILE__, __LINE__, pcString, lPara1, lPara2)

#define LPS_PRINT_LOG3(ModulePID, pcString, lPara1, lPara2, lPara3 ) \
            vos_printf (" %s, %d, %s, %d, %d, %d\r\n ",  __FILE__, __LINE__, pcString, lPara1, lPara2, lPara3)

#define LPS_PRINT_LOG4(ModulePID, pcString, lPara1, lPara2, lPara3, lPara4) \
            vos_printf (" %s, %d, %s, %d, %d, %d, %d\r\n ",  __FILE__, __LINE__,pcString, lPara1, lPara2, lPara3, lPara4)
#else
#define LPS_PRINT_LOG(ModulePID, pcString)
#define LPS_PRINT_LOG1(ModulePID, pcString, lPara1)
#define LPS_PRINT_LOG2(ModulePID, pcString, lPara1, lPara2)
#define LPS_PRINT_LOG3(ModulePID, pcString, lPara1, lPara2, lPara3 )
#define LPS_PRINT_LOG4(ModulePID, pcString, lPara1, lPara2, lPara3, lPara4)
#endif

#else

#ifndef MODEM_FUSION_VERSION

#define LPS_PRINT_LOG(ModulePID, pcString) \
            LPS_OM_LOG( __FILE__, __LINE__, ModulePID, PS_PRINT_INFO, pcString )

#define LPS_PRINT_LOG1(ModulePID, pcString, lPara1) \
            LPS_OM_LOG1( __FILE__, __LINE__, ModulePID, PS_PRINT_INFO, pcString, (VOS_INT32)(lPara1))

#define LPS_PRINT_LOG2(ModulePID, pcString, lPara1, lPara2) \
            LPS_OM_LOG2( __FILE__, __LINE__, ModulePID, PS_PRINT_INFO, pcString, (VOS_INT32)(lPara1), (VOS_INT32)(lPara2))

#define LPS_PRINT_LOG3(ModulePID, pcString, lPara1, lPara2, lPara3 ) \
            LPS_OM_LOG3( __FILE__, __LINE__, ModulePID, PS_PRINT_INFO, pcString, (VOS_INT32)(lPara1), (VOS_INT32)(lPara2), (VOS_INT32)(lPara3))

#define LPS_PRINT_LOG4(ModulePID, pcString, lPara1, lPara2, lPara3, lPara4) \
            LPS_OM_LOG4( __FILE__, __LINE__, ModulePID, PS_PRINT_INFO, pcString, (VOS_INT32)(lPara1),((VOS_INT32)lPara2), (VOS_INT32)(lPara3), (VOS_INT32)(lPara4))
#else
#define LPS_PRINT_LOG(ModulePID, pcString) \
            LPS_OM_LOG( __FILE__, __LINE__, ModulePID, PS_PRINT_INFO, (LPS_LOG_CAT_PARA(__FILE__, " ", pcString, " ")))

#define LPS_PRINT_LOG1(ModulePID, pcString, lPara1) \
            LPS_OM_LOG1( __FILE__, __LINE__, ModulePID, PS_PRINT_INFO, (LPS_LOG_CAT_PARA(__FILE__, " ", pcString, " , %d")), (VOS_INT32)(lPara1))

#define LPS_PRINT_LOG2(ModulePID, pcString, lPara1, lPara2) \
            LPS_OM_LOG2( __FILE__, __LINE__, ModulePID, PS_PRINT_INFO, (LPS_LOG_CAT_PARA(__FILE__, " ", pcString, " , %d, %d")), (VOS_INT32)(lPara1), (VOS_INT32)(lPara2))

#define LPS_PRINT_LOG3(ModulePID, pcString, lPara1, lPara2, lPara3 ) \
            LPS_OM_LOG3( __FILE__, __LINE__, ModulePID, PS_PRINT_INFO, (LPS_LOG_CAT_PARA(__FILE__, " ", pcString, " , %d, %d, %d")), (VOS_INT32)(lPara1), (VOS_INT32)(lPara2), (VOS_INT32)(lPara3))

#define LPS_PRINT_LOG4(ModulePID, pcString, lPara1, lPara2, lPara3, lPara4) \
            LPS_OM_LOG4( __FILE__, __LINE__, ModulePID, PS_PRINT_INFO, (LPS_LOG_CAT_PARA(__FILE__, " ", pcString, " , %d, %d, %d, %d")), (VOS_INT32)(lPara1),((VOS_INT32)lPara2), (VOS_INT32)(lPara3), (VOS_INT32)(lPara4))

#endif
#endif

/* 打印级别定义 */
typedef enum
{
    LOG_LEVEL_OFF = 0,   /* 关闭打印     */
    LOG_LEVEL_ERROR,     /* Error级别    */
    LOG_LEVEL_WARNING,   /* Warning级别  */
    LOG_LEVEL_NORMAL,    /* Normal级别   */
    LOG_LEVEL_INFO,      /* Info级别     */
    LOG_LEVEL_BUTT
}LOG_LEVEL_EN;

/*L2添加，与之前版本日志级别兼容*/
#define PS_PRINT_OFF        LOG_LEVEL_OFF
#define PS_PRINT_ERROR      LOG_LEVEL_ERROR
#define PS_PRINT_WARNING    LOG_LEVEL_WARNING
#define PS_PRINT_NORMAL     LOG_LEVEL_NORMAL
#define PS_PRINT_INFO       LOG_LEVEL_INFO

/* 部件定义 */
typedef enum
{
    LOG_PARTS_PS = 0,   /* 协议栈       */
    LOG_PARTS_DRV,      /* 底软         */
    LOG_PARTS_MEDIA,    /* 媒体         */
    LOG_PARTS_APP,      /* 应用         */
    LOG_PARTS_DSP,      /* 物理层       */
    LOG_PARTS_BUTT
} LOG_PARTS_EN;

/* 模块ID号 */
typedef unsigned int LOG_MODULE_ID_EN;

typedef enum
{
    LOG_SUBMOD_ID_BUTT = 0
}LOG_SUBMOD_ID_EN;




#ifdef OSA_DEBUG
#define OSA_ASSERT(exp) \
    if(!(exp))\
    {\
        vos_assert(__FILE__, __LINE__);\
        return; }
#define OSA_ASSERT_RTN(exp, ret) \
    if(!(exp))\
    {\
        vos_assert(__FILE__, __LINE__);\
        return ret; }
#else
#define OSA_ASSERT( exp ) ( (void)0 )
#define OSA_ASSERT_RTN(exp, ret) ( (void)0 )
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __PSLOGDEF_H__ */


