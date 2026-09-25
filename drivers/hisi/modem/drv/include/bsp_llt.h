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

#ifndef __BSP_LLT_H
#define __BSP_LLT_H

#ifdef DRV_BUILD_LLT
int is_in_llt(void);
void enable_llt(void);
void disable_llt(void);
#else
static inline int is_in_llt(void)
{
    return 0; 
}
static inline void enable_llt(void)
{
}
static inline void disable_llt(void)
{
}
#endif /* end of DRV_BUILD_LLT */
#define FUC_NAME_LEN  (64)
enum control_flag_seq {
    CONTROL_FLAG_CCORE = 0,
    CONTROL_FLAG_ASYNC ,
    CONTROL_FLAG_PARAMS,
    CONTROL_FLAG_LLTMODE,
};

/*lshell para len*/
#define   LSHELL_PARA_LEN            5
#define LLT_SHELL_OPID 0x123456

#pragma pack(push,4)
struct lltshell_arg {
    unsigned int control_flag;
    char func_name[FUC_NAME_LEN];
    long long para[LSHELL_PARA_LEN];
};
#pragma pack(pop)

#ifdef CONFIG_CCORE_BALONG_PM
extern void debug_pm_wake_unlock(void);
extern void debug_pm_wake_lock(void);
#else
static inline void debug_pm_wake_unlock(void){;}
static inline void debug_pm_wake_lock(void){;}
#endif


#define mdrv_ccore_sleep_forbit() debug_pm_wake_lock()
#define mdrv_ccore_sleep_permit() debug_pm_wake_unlock()

#endif
