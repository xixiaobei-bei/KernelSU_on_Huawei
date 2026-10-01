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

#ifndef __MDRV_TIMER_COMMON_H__
#define __MDRV_TIMER_COMMON_H__

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        TIMER_ONCE_COUNT = 0,        /* 单次定时器模式 */
        TIMER_PERIOD_COUNT,          /* 周期定时器模式 */
        TIMER_FREERUN_COUNT,         /* 自由定时器模式 */
        TIMER_COUNT_BUTT
    }DRV_TIMER_MODE_E;

    typedef enum
    {
        TIMER_UNIT_MS = 0,           /* 0表示单位ms模式 */
        TIMER_UNIT_US,               /* 1表示单位us模式 */
        TIMER_UNIT_NONE,             /* 2表示单位1，即直接操作load寄存器模式  */
        TIMER_UNIT_BUTT
    }DRV_TIMER_UNIT_E;

    
    void mdrv_timer_debug_register(unsigned int timer_id, FUNCPTR_1 routinue, int arg);

    
    int mdrv_timer_start(unsigned int usrClkId, FUNCPTR_1 routine, int arg,
            unsigned int timerValue, DRV_TIMER_MODE_E mode,  DRV_TIMER_UNIT_E unitType);

    
    int mdrv_timer_stop(unsigned int usrClkId);

    
    int mdrv_timer_get_rest_time(unsigned int usrClkId, unsigned int unitType, unsigned int * pRestTime);

    /*****************************************************************************
     *  函数  : BSP_BBPGetCurTime
     *  功能  : 获取BBP定时器的值。用于OAM 时戳
     *  输入  : void
     *  输出  :
     *           pulLow32bitValue:    指针参数不能为空，否则会返回失败。
     *           pulHigh32bitValue:   如果为空， 则只返回低32bit的值。
     *  返回  : int
     ******************************************************************************/
    int mdrv_timer_get_accuracy_timestamp(unsigned int *pulHigh32bitValue, unsigned int *pulLow32bitValue);

    /************************************************************************
     *  FUNCTION
     *       mdrv_timer_get_normal_timestamp
     * DESCRIPTION
     *       GetSliceValue
     * INPUTS
     *       NONE
     * OUTPUTS
     *       NONE
             获取时间戳，时间戳返回值为递增值
     *************************************************************************/
    unsigned int mdrv_timer_get_normal_timestamp(void);
	/************************************************************************
     *  FUNCTION
     *       mdrv_timer_get_hrt_timestamp
     * DESCRIPTION
     *       GetHrtSliceValue
     * INPUTS
     *       NONE
     * OUTPUTS
     *       NONE
             获取高精度时间戳，规格最小满足精度0.5us，时间戳返回
             值为递增值,系统深睡会停止计时(lixiaojie\gebaizhang\xiajun讨论添加)
     *************************************************************************/
	unsigned int mdrv_timer_get_hrt_timestamp(void);

#ifdef __cplusplus
}
#endif
#endif
