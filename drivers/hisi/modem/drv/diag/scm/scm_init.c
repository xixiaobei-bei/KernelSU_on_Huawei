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




/*****************************************************************************
  1 头文件包含
**************************************************************************** */
#include <linux/module.h>
#include "OmCommonPpm.h"
#include "scm_ind_src.h"
#include "scm_ind_dst.h"
#include "scm_cnf_src.h"
#include "scm_cnf_dst.h"
#include "scm_debug.h"
#include "diag_system_debug.h"

u32 scm_src_buff_init(void)
{
    u32 ulRet;

    ulRet = scm_init_ind_src_buff();
    if(ulRet)
    {
        diag_error("init ind src buff fail(0x%x)\n", ulRet);
        return ulRet;
    }
    ulRet = scm_init_cnf_src_buff();
    if(ulRet)
    {
        diag_error("init cnf src buff fail(0x%x)\n", ulRet);
        return ulRet;
    }

    return BSP_OK;

}

u32 scm_src_chan_init(void)
{
    u32 ulRet;

    ulRet = scm_ind_src_chan_init();
    if(ulRet)
    {
        diag_error("init ind src chann fail(0x%x)\n", ulRet);
        return ulRet;
    }

    ulRet = scm_cnf_src_chan_init();
    if(ulRet)
    {
        diag_error("init cnf src chan fail(0x%x)\n", ulRet);
        return ulRet;
    }

    return BSP_OK;
}
void scm_dst_func_reg(void)
{
    scm_reg_cnf_coder_dst_send_fuc();
}

u32 scm_dst_mem_init(void)
{
    u32 ulRet = 0;

    ulRet = scm_ind_dst_buff_init();
    if(ulRet)
    {
        diag_error("init ind dst buff fail(0x%x)\n", ulRet);
        return ulRet;
    }

    ulRet = scm_cnf_dst_buff_init();
    if(ulRet)
    {
        diag_error("init cnf dst buff fail(0x%x)\n", ulRet);
        return ulRet;
    }

    return BSP_OK;
}
u32 scm_dst_chan_init(void)
{
    u32 ulRet = 0;

    ulRet = scm_ind_dst_channel_init();
    if(ulRet)
    {
        diag_error("init ind dst chan fail(0x%x)\n", ulRet);
        return ulRet;
    }
    ulRet = scm_cnf_dst_channel_init();
    if(ulRet)
    {
        diag_error("init cnf dst chan fail(0x%x)\n", ulRet);
        return ulRet;
    }

    return BSP_OK;

}


int scm_init(void)
{
    u32 ulRet = 0;

    ulRet = scm_dst_mem_init();
    if(ulRet)
    {
        diag_error("init dst mem fail(0x%x)\n", ulRet);
        return BSP_ERROR;
    }
    ulRet = scm_dst_chan_init();
    if(ulRet)
    {
        diag_error("init dst chan fail(0x%x)\n", ulRet);
        return BSP_ERROR;
    }

    scm_dst_func_reg();

    ulRet = scm_src_buff_init();
    if(ulRet)
    {
        diag_error("init src mem fail(0x%x)\n", ulRet);
        return BSP_ERROR;
    }
    ulRet = scm_src_chan_init();
    if(ulRet)
    {
        diag_error("init src chan fail(0x%x)\n", ulRet);
        return BSP_ERROR;
    }

    diag_crit("[init]scm_init ok\n");

    return BSP_OK;
}
module_init(scm_init);

