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

#include <linux/slab.h>
#include <linux/err.h>
#include "../../adrv/adrv.h"
#include "mdrv.h"
#include <bsp_icc.h>
#include "udi_balong.h"



#include <securec.h>
#include "bsp_print.h"
#include "adp_udi.h"


#define THIS_MODU mod_udi


static int udi_adp_acm_init(void);

/* 各模块特性值定义 */
#define UDI_USB_ACM_CAPA (UDI_CAPA_BLOCK_READ | UDI_CAPA_BLOCK_WRITE | UDI_CAPA_READ_CB | UDI_CAPA_WRITE_CB)
#define UDI_USB_NCM_NDIS_CAPA (UDI_CAPA_READ_CB | UDI_CAPA_BUFFER_LIST)
#define UDI_USB_NCM_CTRL_CAPA (UDI_CAPA_READ_CB | UDI_CAPA_CTRL_OPT)
#define UDI_UART_CAPA (UDI_CAPA_BLOCK_READ | UDI_CAPA_BLOCK_WRITE)

#define UDI_MAX_NET_NUM 4
unsigned int g_udi_data_chn[UDI_MAX_NET_NUM] = {UDI_NCM_NDIS_ID, UDI_NCM_NDIS1_ID,
    UDI_NCM_NDIS2_ID, UDI_NCM_NDIS3_ID};
unsigned int g_udi_ctrl_chn[UDI_MAX_NET_NUM] = {UDI_NCM_CTRL_ID, UDI_NCM_CTRL1_ID,
    UDI_NCM_CTRL2_ID, UDI_NCM_CTRL3_ID};

#ifndef CONFIG_USB_SUPPORT
void *bsp_acm_open(u32 dev_id)
{
    return NULL;
}

s32 bsp_acm_close(void *handle)
{
    return 0;
}

s32 bsp_acm_write(void *handle, void *buf, u32 size)
{
    return 0;
}

s32 bsp_acm_read(void *handle, void *buf, u32 size)
{
    return 0;
}

s32 bsp_acm_ioctl(void *handle, u32 cmd, void *para)
{
    return 0;
}

void acm_adp_dump(void)
{
}
#endif

/* 各模块初始化函数定义 */
void *g_udi_init_func_table[(unsigned int)UDI_DEV_MAX + 1] = {
    /* ACM Init */
    udi_adp_acm_init,

    NULL,

    /* ICC Init */
    NULL,

    /* UART Init */
    NULL,

    /* HSUART Init */
    NULL,

    NULL,

    /* Must Be END */
    NULL
};

udi_adp_init_cb *get_udi_init_func_table(void)
{
    return (udi_adp_init_cb *)g_udi_init_func_table;
}

/* ** eric * */
static int udi_acm_adp_open(struct udi_open_param *param, int handle)
{
    unsigned int type;
    void *filep = NULL;

    UDI_UNUSED_PARAM(handle);

    type = UDI_GET_DEV_TYPE((unsigned int)param->devid);
    filep = bsp_acm_open(type);
    if (filep == NULL) {
        return (-1);
    }

    (void)bsp_udi_set_private(param->devid, filep);
    return 0;
}

/*lint -e429 -esym(429,*)*/
static int udi_adp_acm_init(void)
{
    struct udi_drv_interface_table *drv_interface;
    unsigned int cnt;
    int ret;

    /* 构造回调函数指针列表 */
    drv_interface = kmalloc(sizeof(struct udi_drv_interface_table), GFP_KERNEL);
    if (drv_interface == NULL) {
        bsp_err("BSP_MODU_UDI NO Mem, line:%d\n", __LINE__);
        return (-1);
    }
    ret = memset_s((void *)drv_interface, sizeof(struct udi_drv_interface_table), 0,
                   sizeof(struct udi_drv_interface_table));
    if (ret) {
        bsp_err("BSP_MODU_UDI mem set err:%d, line:%d\n", ret, __LINE__);
        goto ADP_ACM_INIT_ERR;
    }

    /* 只设置需要实现的几个接口 */
    drv_interface->open_cb = udi_acm_adp_open;
    drv_interface->close_cb = (udi_close_cb)bsp_acm_close;
    drv_interface->write_cb = (udi_write_cb)bsp_acm_write;
    drv_interface->read_cb = (udi_read_cb)bsp_acm_read;
    drv_interface->ioctl_cb = (udi_ioctl_cb)bsp_acm_ioctl;

    /*  ACM 都使用同一套驱动函数指针 */
    for (cnt = UDI_USB_ACM_CTRL; cnt < UDI_USB_ACM_MAX; cnt++) {
        (void)bsp_udi_set_capability((enum udi_device_id)UDI_BUILD_DEV_ID(UDI_DEV_USB_ACM, cnt), UDI_USB_ACM_CAPA);
        (void)bsp_udi_set_intf_table((enum udi_device_id)UDI_BUILD_DEV_ID(UDI_DEV_USB_ACM, cnt), drv_interface);
    }

    return 0;

ADP_ACM_INIT_ERR:
    if (drv_interface != NULL) {
        kfree(drv_interface);
    }
    return (-1);
}



