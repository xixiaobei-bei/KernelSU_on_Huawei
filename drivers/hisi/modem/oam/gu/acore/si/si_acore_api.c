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

/*
 * 版权所有（c）华为技术有限公司 2012-2019
 * 功能描述: 该文件定义了SI Acore使用的公共API
 * 生成日期: 2018年03月24日
 */
#include "vos.h"
#include "product_config.h"
#include "nv_id_pam.h"
#include "si_acore_api.h"

#include "mdrv_nvim.h"


#define THIS_FILE_ID PS_FILE_ID_SI_ACORE_API_C

/*
 * 功能描述: 根据modem id查询slot id
 * 1. 日    期: 2018年03月13日
 *    修改内容: Create
 */
SI_PIH_CardSlotUint32 SI_GetSlotIdByModemId(ModemIdUint16 modemId)
{
    SI_PIH_CardSlotUint32 slotId;
    USIMM_SciCfg sciCfg = { 0 };

    if (mdrv_nv_readex(MODEM_ID_0, NV_ITEM_SCI_DSDA_CFG, &sciCfg, sizeof(sciCfg)) != NV_OK) {
        return SI_PIH_CARD_SLOT_BUTT;
    }

    switch (modemId) {
        case MODEM_ID_0:
            slotId = sciCfg.bitModem0Slot;
            break;
#if (1 < MULTI_MODEM_NUMBER)
        case MODEM_ID_1:
            slotId = sciCfg.bitModem1Slot;
            break;
#if (3 == MULTI_MODEM_NUMBER)
        case MODEM_ID_2:
            slotId = sciCfg.bitModem2Slot;
            break;
#endif
#endif
        default:
            slotId = SI_PIH_CARD_SLOT_BUTT;
            break;
    }

    return slotId;
}

