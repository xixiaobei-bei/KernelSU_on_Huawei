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
#include <linux/module.h>
#include <product_config.h>
#include <linux/init.h>
#include <nv_stru_drv.h>
#include <nv_stru_msp_interface.h>
#include <adrv.h>
#include <bsp_nvim.h>
#include <bsp_dump.h>
#include <bsp_trace.h>

#define  pmic_print_error(fmt, ...)    (bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_PMU, "[pmu]: <%s> "fmt, __FUNCTION__, ##__VA_ARGS__))
int pmu_dcxo_set(uint16_t dcxo_ctrim, uint16_t dcxo_c2_fix);
int pmu_dcxo_get(uint16_t *dcxo_ctrim, uint16_t *dcxo_c2_fix);

int __init pmic_dcxo_init(void)
{
	u16 c1_value;
	u16 c2_value;
	int iret;
    u16 dcxo_ctrim = 0;
    u16 dcxo_c2_fix = 0;

    iret = bsp_nvm_read(en_NV_Item_DCXO_C_TRIM, (u8*)&dcxo_ctrim, sizeof(dcxo_ctrim));
    iret |= bsp_nvm_read(en_NV_Item_DCXO_C2_FIX, (u8*)&dcxo_c2_fix, sizeof(dcxo_c2_fix));

    if(NV_OK != iret)
    {
        pmic_print_error("pmu dcxo c1c2 read nv error\n");
        return -1;
    }

	pmu_dcxo_get(&c1_value, &c2_value);
	if(c1_value != dcxo_ctrim || c2_value != dcxo_c2_fix)
	{
		pmic_print_error("dcxo nv unequal, [mdm nv]0x%x,0x%x, [ap nv]0x%x,0x%x\n",dcxo_ctrim,dcxo_c2_fix,c1_value,c2_value);
		pmu_dcxo_set(dcxo_ctrim, dcxo_c2_fix);
	}
	else
	{
		pmic_print_error("ap&modem dcxo nv equal, [mdm nv]0x%x,0x%x, [ap nv]0x%x,0x%x\n",dcxo_ctrim,dcxo_c2_fix,c1_value,c2_value);
	}
	return 0;
}


