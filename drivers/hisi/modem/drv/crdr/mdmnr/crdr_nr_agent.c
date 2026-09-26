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
#include <product_config.h>
#include "osl_types.h"
#include "bsp_slice.h"
#include "bsp_ipc.h"
#include "bsp_dump.h"
#include "bsp_slice.h"
#include "bsp_wdt.h"
#include "crdr_config.h"
#include "crdr_exc_handle.h"
#include "crdr_nr_agent.h"

#undef THIS_MODU
#define THIS_MODU mod_dump

void crdr_nrccpu_wdt_handle(u32 mod_id, u32 arg1, u32 arg2, char *data, u32 length)
{
    dump_exception_info_s exception_info_s = {
        0,
    };
    crdr_ok("[0x%x]modem nrccpu wdt or pdlock enter system error! \n", bsp_get_slice_value());
    crdr_ok("mod_id=0x%x arg1=0x%x arg2=0x%x len=0x%x\n", mod_id, arg1, arg2, length);
    exception_info_s.core = DUMP_CPU_NRCCPU;
    exception_info_s.mod_id = mod_id;
#ifndef BSP_CONFIG_PHONE_TYPE
    exception_info_s.rdr_mod_id = RDR_MODEM_AP_MOD_ID;
#else
    exception_info_s.rdr_mod_id = RDR_MODEM_NR_MOD_ID;
#endif
    exception_info_s.arg1 = 0;
    exception_info_s.arg2 = 0;

    crdr_register_exception(&exception_info_s);
}

void crdr_nrccpu_wdt_hook(unsigned int mod_id)
{
    if (0 == crdr_get_exc_state()) {
        crdr_set_exc_state(1);
    } else {
        crdr_ok("excepiton is handling,exit\n");
    }

    crdr_nrccpu_wdt_handle(mod_id, DUMP_REASON_WDT, 0, 0, 0);
}

void crdr_nr_agent_handle(u32 param)
{
    dump_exception_info_s exception_info_s = { 0, DUMP_REASON_NORMAL };

    crdr_ok("NR enter system error!,timestamp:0x%x\n", bsp_get_slice_value());

    if (0 == crdr_get_exc_state()) {
        crdr_set_exc_state(1);
    } else {
        crdr_ok("excepiton is handling,exit\n");
    }

    crdr_ok("stop nrccpu wdt finish\n");
    exception_info_s.core = DUMP_CPU_NRCCPU;
    exception_info_s.mod_id = DRV_ERRNO_DUMP_ARM_EXC;
#ifndef BSP_CONFIG_PHONE_TYPE
    exception_info_s.rdr_mod_id = RDR_MODEM_AP_MOD_ID;
#else
    exception_info_s.rdr_mod_id = RDR_MODEM_NR_MOD_ID;
#endif

    exception_info_s.arg1 = 0;
    exception_info_s.arg2 = 0;

    crdr_register_exception(&exception_info_s);

    return;
}

__init s32 crdr_nr_agent_init(void)
{
    s32 ret;
    ret = bsp_ipc_int_connect(IPC_ACPU_SRC_NRCCPU_DUMP, (voidfuncptr)crdr_nr_agent_handle, 0);
    if (unlikely(ret != BSP_OK)) {
        crdr_error("fail to connect ipc int\n");
        return BSP_ERROR;
    }
    ret = bsp_ipc_int_enable(IPC_ACPU_SRC_NRCCPU_DUMP);
    if (unlikely(ret != BSP_OK)) {
        crdr_error("fail to enbale ipc int\n");
        return BSP_ERROR;
    }
    ret = bsp_wdt_register_hook(WDT_NRCCPU_ID, crdr_nrccpu_wdt_hook);
    if (ret == BSP_ERROR) {
        crdr_error("fail to bsp_wdt_register_hook\n");
    }

    crdr_ok("nrdrr init ok");
    return BSP_OK;
}
