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

/*
 * #include <vxWorks.h>
 * #include <logLib.h>
 * #include <usrLib.h>
 * #include <string.h>
 * #include <stdio.h>
 * #include "BSP_UDI.h"
 * #include "BSP_UDI_DRV.h"
 */
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/semaphore.h>
#include <securec.h>

#include "udi_balong.h"
#include "bsp_print.h"
#include "adp_udi.h"


#define THIS_MODU mod_udi
#define UDI_MAX_MAIN_DEV_NUM UDI_DEV_MAX
#define UDI_MAX_DEV_TYPE_NUM 32
#define UDI_MAX_OPEN_NODE_NUM 64
#define UDI_OPEN_NODE_HEADER 0x5A0000

/*
 * 类型定义
 */
/* 设备打开节点 */
struct udi_open_node {
    int open;
    void *private;
    struct udi_drv_interface_table *drv_table;
};

/* 设备实例属性 */
struct udi_dev_instance {
    unsigned int capability; /* 设备特性 */
    struct udi_drv_interface_table *drv_interface; /* 接口回调列表 */
    void *private; /* 每个驱动私有全局 */
};

/*
 * 全局变量
 */
struct udi_open_node g_open_node_table[UDI_MAX_OPEN_NODE_NUM];
unsigned int g_open_node_pos = 0;
struct udi_dev_instance g_device_table[UDI_MAX_MAIN_DEV_NUM][UDI_MAX_DEV_TYPE_NUM];
struct semaphore g_udi_mtx_open;

/*
 * 宏实现
 */
#define UDI_IDX_TO_HANDLE(idx) ((int)(UDI_OPEN_NODE_HEADER | (idx)))
#define UDI_HANDLE_TO_IDX(hdl) ((unsigned int)((hdl)&0xFFFF))

#define UDI_IS_INVALID_TABLE(dev_table) (0 == (uintptr_t)(dev_table) || (uintptr_t)(-1) == (uintptr_t)(dev_table))

#define UDI_PARSE_DEV_ID(dev_id, main_id, dev_type) do { \
    main_id = UDI_GET_MAIN_DEV_ID((dev_id)); \
    dev_type = UDI_GET_DEV_TYPE((dev_id)); \
    BSP_ASSERT((main_id) < UDI_MAX_MAIN_DEV_NUM); \
    BSP_ASSERT((dev_type) < UDI_MAX_DEV_TYPE_NUM); \
} while (0)
/*
 * 内部函数
 */
static int udi_check_and_get_index(int handle)
{
    int index;

    index = (unsigned int)handle & 0xFFFF;
    if ((((unsigned int)handle & 0xFF0000) != UDI_OPEN_NODE_HEADER) || index >= UDI_MAX_OPEN_NODE_NUM) {
        return -1;
    }

    if (UDI_IS_INVALID_TABLE(g_open_node_table[index].drv_table)) {
        return (-1);
    }

    return index;
}

static int udi_get_out_open_node(void)
{
    unsigned int cnt;
    int handle = UDI_INVALID_HANDLE;

    down(&g_udi_mtx_open);
    /* 优先从当前位置找 */
    for (cnt = g_open_node_pos; cnt < UDI_MAX_OPEN_NODE_NUM; cnt++) {
        if (g_open_node_table[cnt].open == 0) {
            handle = UDI_IDX_TO_HANDLE(cnt);
            break;
        }
    }

    /* 否则, 再从头找 */
    if (handle == UDI_INVALID_HANDLE) {
        for (cnt = 0; cnt < g_open_node_pos; cnt++) {
            if (g_open_node_table[cnt].open == 0) {
                handle = UDI_IDX_TO_HANDLE(cnt);
                break;
            }
        }
    }

    /* 找到一个可用的handle */
    if (handle != UDI_INVALID_HANDLE) {
        g_open_node_pos = (cnt + 1) % UDI_MAX_OPEN_NODE_NUM;
        g_open_node_table[cnt].open = 1;
    }
    up(&g_udi_mtx_open);

    return handle;
}

static int udi_ret_open_node(unsigned int idx)
{
    BSP_ASSERT(idx < UDI_MAX_OPEN_NODE_NUM);
    /*lint 661*/
    if (idx >= UDI_MAX_OPEN_NODE_NUM) {
        bsp_err("udi_ret_open_node para error: idx=%u\n", idx);
        return (-1);
    }
    down(&g_udi_mtx_open);
    g_open_node_table[idx].open = 0;
    g_open_node_table[idx].drv_table = NULL;
    up(&g_udi_mtx_open);

    return 0;
}

/*
 * 功能描述: 设置驱动内部私有数据
 */
int bsp_udi_set_private(enum udi_device_id dev_id, void *private)
{
    unsigned int main_id;
    unsigned int dev_type;
    /*lint  e661 e662*/
    UDI_PARSE_DEV_ID((unsigned int)dev_id, main_id, dev_type);
    if ((main_id >= UDI_MAX_MAIN_DEV_NUM) || (dev_type >= UDI_MAX_DEV_TYPE_NUM)) {
        bsp_err("bsp_udi_set_private para error: main_id=%u dev_type=%u\n", main_id, dev_type);
        return (-1);
    }
    g_device_table[main_id][dev_type].private = private;

    return 0;
}

/*
 * 功能描述: 设置设备特性值
 */
int bsp_udi_set_capability(enum udi_device_id dev_id, unsigned int capability)
{
    unsigned int main_id;
    unsigned int dev_type;

    /*lint e662 e661*/
    UDI_PARSE_DEV_ID((unsigned int)dev_id, main_id, dev_type);
    if ((main_id >= UDI_MAX_MAIN_DEV_NUM) || (dev_type >= UDI_MAX_DEV_TYPE_NUM)) {
        bsp_err("bsp_udi_set_capability para error: main_id=%u dev_type=%u\n", main_id, dev_type);
        return (-1);
    }
    g_device_table[main_id][dev_type].capability = capability;

    return 0;
}

/*
 * 功能描述: 设置设备回调函数列表(由适配层调用)
 */
int bsp_udi_set_intf_table(enum udi_device_id dev_id, struct udi_drv_interface_table *drv_intf)
{
    unsigned int main_id;
    unsigned int dev_type;
    /*lint e661 e662*/
    UDI_PARSE_DEV_ID((unsigned int)dev_id, main_id, dev_type);
    if ((main_id >= UDI_MAX_MAIN_DEV_NUM) || (dev_type >= UDI_MAX_DEV_TYPE_NUM)) {
        bsp_err("bsp_udi_set_intf_table para error: main_id=%u dev_type=%u\n", main_id, dev_type);
        return (-1);
    }
    g_device_table[main_id][dev_type].drv_interface = drv_intf;

    return 0;
}

/*
 * 功能描述: UDI 模块初始化
 */
int bsp_udi_init(void)
{
    udi_adp_init_cb init_cb;
    unsigned int cnt;
    int ret;
    udi_adp_init_cb *g_udi_init_func_table = get_udi_init_func_table();

    ret = memset_s(g_device_table, sizeof(g_device_table), 0,
                   UDI_MAX_MAIN_DEV_NUM * UDI_MAX_DEV_TYPE_NUM * sizeof(struct udi_dev_instance));
    if (ret) {
        bsp_err("BSP_MODU_UDI mem set err:%d, line:%d\n", ret, __LINE__);
        return ret;
    }
    ret = memset_s(g_open_node_table, sizeof(g_open_node_table), 0,
        UDI_MAX_OPEN_NODE_NUM * sizeof(struct udi_open_node));
    if (ret) {
        bsp_err("BSP_MODU_UDI mem set err:%d, line:%d\n", ret, __LINE__);
        return ret;
    }
    sema_init(&g_udi_mtx_open, 1);

    /* 调用初始化函数 */
    for (cnt = 0; cnt < (unsigned int)UDI_DEV_MAX; cnt++) {
        init_cb = g_udi_init_func_table[cnt];
        if (init_cb) {
            if (init_cb() != 0) {
                bsp_err("BSP_MODU_UDI usr init_cb fail, line:%d\n", __LINE__);
                return (-1);
            }
        }
    }
    return 0;
}

/*
 * 功能描述: 根据设备ID获取当前设备支持的特性
 */
int udi_get_capability(enum udi_device_id dev_id)
{
    unsigned int main_id;
    unsigned int dev_type;

    UDI_PARSE_DEV_ID((unsigned int)dev_id, main_id, dev_type);
    /* lint e661 e662 */
    if ((main_id >= UDI_MAX_MAIN_DEV_NUM) || (dev_type >= UDI_MAX_DEV_TYPE_NUM)) {
        bsp_err("udi_get_capability para error: main_id=%u dev_type=%u\n", main_id, dev_type);
        return (-1);
    }
    return (int)g_device_table[main_id][dev_type].capability;
}

/*
 * 功能描述: 打开设备(数据通道)
 */
int mdrv_udi_open(struct udi_open_param *param)
{
    unsigned int main_id = 0;
    unsigned int dev_type = 0;
    unsigned int idx = 0;
    int handle;
    struct udi_drv_interface_table *drv_interface = NULL;

    if (NULL == param) {
        goto UDI_OPEN_ERR;
    }
    /*lint e662 e661*/
    UDI_PARSE_DEV_ID((unsigned int)param->devid, main_id, dev_type);
    if ((main_id >= UDI_MAX_MAIN_DEV_NUM) || (dev_type >= UDI_MAX_DEV_TYPE_NUM)) {
        goto UDI_OPEN_ERR;
    }
    /* 查找一个可用的节点 */
    handle = udi_get_out_open_node();
    if (UDI_INVALID_HANDLE == handle) {
        goto UDI_OPEN_ERR;
    }
    idx = UDI_HANDLE_TO_IDX((unsigned int)handle);

    /* 调用用户回调函数 */
    drv_interface = g_device_table[main_id][dev_type].drv_interface;
    if (NULL == drv_interface || NULL == drv_interface->open_cb) {
        goto UDI_OPEN_ERR_RET_NODE;
    }
    if (drv_interface->open_cb(param, handle)) {
        goto UDI_OPEN_ERR_RET_NODE;
    }

    /* 保存驱动私有数据 */
    g_open_node_table[idx].drv_table = g_device_table[main_id][dev_type].drv_interface;
    g_open_node_table[idx].private = g_device_table[main_id][dev_type].private;

    return handle;

UDI_OPEN_ERR_RET_NODE:
    (void)udi_ret_open_node(idx);
UDI_OPEN_ERR:
    return UDI_INVALID_HANDLE;
}
EXPORT_SYMBOL(mdrv_udi_open);

/*
 * 功能描述: 关闭设备(数据通道)
 */
int mdrv_udi_close(int handle)
{
    int index;
    int ret = -1;
    struct udi_drv_interface_table *drv_table = NULL;
    void *priv = NULL;

    index = udi_check_and_get_index(handle);
    if (index < 0) {
        return index;
    }

    drv_table = g_open_node_table[index].drv_table;
    if (!(drv_table->close_cb)) {
        return ret;
    }

    priv = g_open_node_table[index].private;
    ret = drv_table->close_cb(priv);
    (void)udi_ret_open_node((unsigned int)index);

    return ret;
}
EXPORT_SYMBOL(mdrv_udi_close);

/*
 * 功能描述: 数据写
 */
int mdrv_udi_write(int handle, void *mem_obj, unsigned int size)
{
    int index;
    int ret = -1;
    struct udi_drv_interface_table *drv_table = NULL;
    void *priv = NULL;

    index = udi_check_and_get_index(handle);
    if (index < 0) {
        return index;
    }

    drv_table = g_open_node_table[index].drv_table;
    if (!(drv_table->write_cb)) {
        return ret;
    }

    priv = g_open_node_table[index].private;
    ret = drv_table->write_cb(priv, mem_obj, size);

    return ret;
}
EXPORT_SYMBOL(mdrv_udi_write);

/*
 * 功能描述: 数据读
 */
int mdrv_udi_read(int handle, void *mem_obj, unsigned int size)
{
    int index;
    int ret = -1;
    struct udi_drv_interface_table *drv_table = NULL;
    void *priv = NULL;

    index = udi_check_and_get_index(handle);
    if (index < 0) {
        return index;
    }

    drv_table = g_open_node_table[index].drv_table;
    if (!(drv_table->read_cb)) {
        return ret;
    }

    priv = g_open_node_table[index].private;
    ret = drv_table->read_cb(priv, mem_obj, size);

    return ret;
}
EXPORT_SYMBOL(mdrv_udi_read);

/*
 * 功能描述: 数据通道属性配置
 */
int mdrv_udi_ioctl(int handle, unsigned int cmd, void *param)
{
    int index;
    int ret = -1;
    struct udi_drv_interface_table *drv_table = NULL;
    void *priv = NULL;

    index = udi_check_and_get_index(handle);
    if (index < 0) {
        return index;
    }

    drv_table = g_open_node_table[index].drv_table;
    if (!(drv_table->ioctl_cb)) {
        return ret;
    }

    priv = g_open_node_table[index].private;
    ret = drv_table->ioctl_cb(priv, cmd, param);

    return ret;
}

EXPORT_SYMBOL(mdrv_udi_ioctl);
#ifndef CONFIG_HISI_BALONG_MODEM_MODULE
arch_initcall(bsp_udi_init);
#endif
