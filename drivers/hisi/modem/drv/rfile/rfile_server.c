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
#include "rfile_server.h"
#include "rfile_server_icc.h"
#include "rfile_server_dump.h"
#include "rfile_server_load_mode.h"

#include <asm/current.h>
#include <asm/page.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/list.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include "product_config.h"

#include <osl_thread.h>
#include <bsp_slice.h>
#include <bsp_dump.h>
#include <bsp_print.h>
#include <bsp_slice.h>
#include <bsp_version.h>
#include <bsp_mloader.h>
#include <securec.h>


#define THIS_MODU mod_rfile
#define RFILE_APP_NAME "rfile_user"
#define RFILE_APP_NAME_PHONE "rfile_user_phone"
struct rfile_main_stru_acore g_rfile_acore_main = {
    RFILE_STATE_UNINITED,
};
int g_rfile_while = 1;

struct rfile_main_stru_acore *rfile_get_main_info(void)
{
    return &g_rfile_acore_main;
}

bsp_fs_status_e bsp_fs_ok(void)
{
    if (g_rfile_acore_main.state == RFILE_STATE_UNINITED) {
        return BSP_FS_NOT_OK;
    }
    return BSP_FS_OK;
}

void rfile_set_innner_state(enum rfile_acore_state state)
{
    g_rfile_acore_main.state = state;
    rfile_dump_inner_state_dbg(state);
    return;
}

void rfile_set_event(enum rfile_acore_event event)
{
    osl_sem_up(&g_rfile_acore_main.sem_taskmain);
    rfile_dump_event_dbg(event);
    return;
}

void rfile_icc_send(void *data, u32 len)
{
    s32 ret;
    u32 time_stamp[0x3] = {0};

    time_stamp[0] = bsp_get_slice_value();
    while (time_stamp[0x2] <= (bsp_get_slice_freq() * RFILE_WAIT_TIME_S)) {
        ret = rfile_icc_common_send(data, len);
        if (ret == RFILE_CHANNAL_FULL) {
            /* buffer满，延时后重发 */
            rfile_dump_event_dbg(RFILE_EVENT_ICC_FULL);
            schedule();
            time_stamp[0x1] = bsp_get_slice_value();
            time_stamp[0x2] = get_timer_slice_delta(time_stamp[0], time_stamp[1]);
            continue;
        } else if (len != (u32)ret) {
            bsp_err("<%s> icc_send failed, ret = 0x%x, len = 0x%x.\n", __FUNCTION__, (u32)ret, len);
            return;
        } else {
            return;
        }
    }
    return;
}

void rfile_icc_common_cb(void)
{
    rfile_set_event(RFILE_EVENT_ICC_CB);
    return;
}

void rfile_check_load_mode(struct rfile_main_stru_acore *rfile_info)
{
    struct rfile_comm_resp *resp = (struct rfile_comm_resp *)rfile_info->data_buffer;

    resp->ret = (s32)rfile_get_fs_is_support();
    bsp_err("<%s> ccore check load mode, return %d.\n", __FUNCTION__, resp->ret);
    rfile_icc_send(resp, sizeof(struct rfile_comm_resp));
    return;
}

int rfile_read_icc(struct rfile_main_stru_acore *rfile_info, int *need_reread_icc)
{
    s32 ret;
    struct rfile_comm_cmd *cmd = (struct rfile_comm_cmd *)rfile_info->data_buffer;

    *need_reread_icc = 1;
    ret = rfile_icc_common_recv((void *)cmd, RFILE_LEN_MAX);
    if (ret == 0) {
        *need_reread_icc = 0;
        return -1;
    }
    if (((u32)ret > RFILE_LEN_MAX) || (ret < (s32)sizeof(struct rfile_comm_cmd))) {
        bsp_err("<%s> icc_read failed, len = %d.\n", __FUNCTION__, ret);
        *need_reread_icc = 0;
        return -1;
    }
    rfile_dump_handle_dbg(cmd->op_type, cmd->op_num, cmd->cmd_cnt, RFILE_ACORE_ICC_RECV, 0);
    if (cmd->op_type >= EN_RFILE_OP_BUTT) {
        bsp_err("<%s> op_type %d is bigger than EN_RFILE_OP_BUTT.\n", __FUNCTION__,
            cmd->op_type); /* 当前包异常，继续读icc下一包 */
        return -1;
    }
    if (cmd->datalen > RFILE_WR_LEN_MAX) {
        bsp_err("<%s> cmd->datalen %d err.\n", __FUNCTION__, cmd->datalen); /* 当前包异常，继续读icc下一包 */
        return -1;
    }
    if (cmd->op_type == EN_RFILE_OP_CHECK_LOAD_MODE) {
        rfile_check_load_mode(rfile_info); /* 如果是检查启动模式，直接返回，不参与用户态处理，继续读icc下一包 */
        return -1;
    }
    return 0;
}

s32 rfile_recover_prepare(struct rfile_main_stru_acore *rfile_info, struct rfile_fd_list *elemt)
{
    struct rfile_comm_cmd *cmd = (struct rfile_comm_cmd *)rfile_info->data_buffer;
    u32 datalen;
    s32 ret;

    if (elemt->path == NULL) {
        bsp_err("<%s> path null.\n", __FUNCTION__);
        return -1;
    }
    datalen = strlen((char *)elemt->path) + 1;
    cmd->datalen = datalen;
    ret = memcpy_s((void *)cmd->data, RFILE_RD_LEN_MAX, elemt->path, datalen);
    if (ret != EOK) {
        bsp_err("<%s> memcpy_s err. ret = %d.\n", __FUNCTION__, ret);
        return -1;
    }

    if (elemt->open_type == RFILE_OPEN_FILE) {
        cmd->op_type = EN_RFILE_OP_OPEN_RECOVER;
        cmd->mode = elemt->mode;
        cmd->flags = elemt->flags;
        cmd->offset = elemt->offset;
    } else {
        cmd->op_type = EN_RFILE_OP_OPENDIR;
    }
    return 0;
}

void rfile_recover_update_fd(struct rfile_main_stru_acore *rfile_info, struct rfile_fd_list *elemt)
{
    struct rfile_comm_resp *resp = (struct rfile_comm_resp *)rfile_info->data_buffer;
    if (resp->ret < 0) {
        bsp_err("app recover handle failed\n");
        return;
    }
    elemt->fd_new = resp->ret;
    return;
}

void rfile_handle_app_recover(struct rfile_main_stru_acore *rfile_info)
{
    struct list_head *p = NULL;
    struct list_head *n = NULL;
    struct rfile_fd_list *elemt = NULL;
    s32 ret;
    list_for_each_safe(p, n, &rfile_info->fd_list)
    {
        elemt = list_entry(p, struct rfile_fd_list, stlist);
        elemt->fd_outdate_flag = 1;
        if (rfile_recover_prepare(rfile_info, elemt)) {
            continue;
        }
        osl_sem_up(&rfile_info->sem_read);
        ret = osl_sem_downtimeout(&rfile_info->sem_write, msecs_to_jiffies(RFILE_WAIT_TIME_MS));
        if (ret) {
            bsp_err("app recover handle timeout\n");
            return; /* 操作超时可能因为用户态程序再次被杀，此时不清recover标记，需再次执行恢复 */
        }
        rfile_recover_update_fd(rfile_info, elemt);
    }
    bsp_err("app recover handle done\n");
    g_rfile_acore_main.recover_flag = 0;

    return;
}

void rfile_add_fd_node(struct rfile_main_stru_acore *rfile_info, struct rfile_handling_data *middle_data,
    enum rfile_open_type open_type)
{
    int ret;
    struct rfile_comm_resp *resp = (struct rfile_comm_resp *)rfile_info->data_buffer;
    struct rfile_fd_list *elemt = NULL;

    elemt = kzalloc(sizeof(struct rfile_fd_list), GFP_KERNEL);
    if (elemt == NULL) {
        bsp_err("<%s> kzalloc fd_node failed.\n", __FUNCTION__);
        return;
    }
    elemt->fd = resp->ret;
    elemt->open_type = open_type;
    elemt->offset = 0;
    elemt->fd_outdate_flag = 0;
    if (open_type == RFILE_OPEN_FILE) {
        elemt->mode = middle_data->mode;
        elemt->flags = middle_data->flags;
    }
    ret = strcpy_s(elemt->path, sizeof(elemt->path), middle_data->path);
    if (ret != EOK) {
        bsp_err("<%s> strcpy_s err. ret = %d.\n", __FUNCTION__, ret);
        kfree(elemt);
        return;
    }
    list_add(&elemt->stlist, &rfile_info->fd_list);
    return;
}

void rfile_del_fd_node(struct rfile_main_stru_acore *rfile_info, struct rfile_handling_data *middle_data)
{
    struct list_head *p = NULL;
    struct list_head *n = NULL;
    struct rfile_fd_list *elemt = NULL;

    list_for_each_safe(p, n, &rfile_info->fd_list)
    {
        elemt = list_entry(p, struct rfile_fd_list, stlist);
        if (elemt->fd == middle_data->fd) {
            list_del(&elemt->stlist);
            kfree(elemt);
            elemt = NULL;
            return;
        }
    }
    bsp_err("<%s> find fdlist to end, but not found fd %d\n", __FUNCTION__, middle_data->fd);
    return;
}

void rfile_offset_manage(struct rfile_main_stru_acore *rfile_info, struct rfile_handling_data *middle_data,
    enum rfile_offset_manage_type offset_type)
{
    struct list_head *p = NULL;
    struct list_head *n = NULL;
    struct rfile_fd_list *elemt = NULL;
    struct rfile_comm_resp *resp = (struct rfile_comm_resp *)rfile_info->data_buffer;

    list_for_each_safe(p, n, &rfile_info->fd_list)
    {
        elemt = list_entry(p, struct rfile_fd_list, stlist);
        if (elemt->fd == middle_data->fd) {
            if (offset_type == RFILE_MANAGE_READ_WRITE) {
                elemt->offset += resp->ret;
            } else {
                elemt->offset = resp->ret;
            }
            return;
        }
    }
    bsp_err("<%s> find fd_list to end but not found, fd = %d.\n", __FUNCTION__, middle_data->fd);
    return;
}

void rfile_fdlist_manage(struct rfile_main_stru_acore *rfile_info, struct rfile_handling_data *middle_data)
{
    struct rfile_comm_resp *resp = (struct rfile_comm_resp *)rfile_info->data_buffer;

    if (resp->ret < 0) {
        return;
    }
    switch (resp->op_type) {
        case EN_RFILE_OP_OPEN:
            rfile_add_fd_node(rfile_info, middle_data, RFILE_OPEN_FILE);
            break;
        case EN_RFILE_OP_OPENDIR:
            rfile_add_fd_node(rfile_info, middle_data, RFILE_OPEN_DIR);
            break;
        case EN_RFILE_OP_READ:
        case EN_RFILE_OP_WRITE:
        case EN_RFILE_OP_WRITE_SYNC:
            rfile_offset_manage(rfile_info, middle_data, RFILE_MANAGE_READ_WRITE);
            break;
        case EN_RFILE_OP_SEEK:
            rfile_offset_manage(rfile_info, middle_data, RFILE_MANAGE_SEEK);
            break;
        case EN_RFILE_OP_CLOSE:
        case EN_RFILE_OP_CLOSEDIR:
            rfile_del_fd_node(rfile_info, middle_data);
            break;
        default:
            break;
    }
}
void rfile_del_fd_list(struct rfile_main_stru_acore *rfile_info)
{
    struct list_head *p = NULL;
    struct list_head *n = NULL;
    struct rfile_fd_list *elemt = NULL;
    list_for_each_safe(p, n, &rfile_info->fd_list)
    {
        elemt = list_entry(p, struct rfile_fd_list, stlist);
        list_del(&elemt->stlist);
        kfree(elemt);
        elemt = NULL;
    }
    return;
}

void rfile_ret_print(struct rfile_comm_resp *resp, struct rfile_handling_data *middle_data)
{
    if (resp->ret >= 0) {
        return;
    }
    if (middle_data->path[0] == '\0') {
        bsp_err("op:%d handle failed, ret = %d, op_num = %d\n", resp->op_type, resp->ret, resp->op_num);
        return;
    }
    if (resp->ret == -ENOENT) {
        bsp_err("op:%d, no such file, op_num = %d, path = %s\n", resp->op_type, resp->op_num, middle_data->path);
    } else {
        bsp_err("op:%d handle failed, ret = %d, op_num = %d, path = %s\n", resp->op_type, resp->ret, resp->op_num,
            middle_data->path);
    }
    return;
}

void rfile_store_middle_data(struct rfile_main_stru_acore *rfile_info, struct rfile_handling_data *middle_data)
{
    struct rfile_comm_cmd *cmd = (struct rfile_comm_cmd *)rfile_info->data_buffer;
    int ret;
    u32 datalen = strlen(cmd->data);
    if ((cmd->datalen != 0) && (datalen != 0) && (datalen + 1 < sizeof(middle_data->path)) &&
        (cmd->op_type != EN_RFILE_OP_WRITE && cmd->op_type != EN_RFILE_OP_WRITE_SYNC)) {
        ret = strncat_s(middle_data->path, sizeof(middle_data->path), cmd->data, datalen);
        if (ret) {
            bsp_err("strncat err, ret %d\n", ret);
            middle_data->path[0] = '\0';
        }
    }
    middle_data->mode = cmd->mode;
    middle_data->flags = cmd->flags;
    middle_data->fd = cmd->fd;
    return;
}

void rfile_handle_cmd(struct rfile_main_stru_acore *rfile_info)
{
    struct rfile_comm_resp *resp = (struct rfile_comm_resp *)rfile_info->data_buffer;
    int need_reread_icc = 1;
    int ret;
    struct rfile_handling_data middle_data = { 0 };

    if (rfile_read_icc(rfile_info, &need_reread_icc)) {
        goto exit;
    }
    rfile_store_middle_data(rfile_info, &middle_data);
    osl_sem_up(&rfile_info->sem_read);
    ret = osl_sem_downtimeout(&rfile_info->sem_write, msecs_to_jiffies(RFILE_WAIT_TIME_MS));
    if (ret) {
        bsp_err("op:%d, cmd_cnt = %d, handle timeout\n", resp->op_type, resp->cmd_cnt);
        resp->ret = -1;
    }
    rfile_ret_print(resp, &middle_data);
    rfile_fdlist_manage(rfile_info, &middle_data);
    (void)memset_s(rfile_info->data_buffer, RFILE_LEN_MAX, 0, RFILE_LEN_MAX);

exit:
    if (need_reread_icc == 1) {
        osl_sem_up(&rfile_info->sem_taskmain); /* 处理结束后避免ICC通道中有缓存，再次启动读取 */
    }
    return;
}

void rfile_handle_reset(struct rfile_main_stru_acore *rfile_info)
{
    s32 ret;
    struct rfile_comm_cmd *cmd = (struct rfile_comm_cmd *)rfile_info->data_buffer;

    cmd->op_type = EN_RFILE_OP_RESET;
    rfile_del_fd_list(rfile_info);
    osl_sem_up(&rfile_info->sem_read);
    ret = osl_sem_downtimeout(&rfile_info->sem_resetting, msecs_to_jiffies(RFILE_WAIT_TIME_MS));
    if (ret) {
        bsp_err("wait resetting handle timeout\n");
        g_rfile_acore_main.reset_flag = 0;
    }
    osl_sem_up(&rfile_info->sem_taskmain);
    return;
}

s32 rfile_process_thread(void *data)
{
    struct rfile_main_stru_acore *rfile_info = (struct rfile_main_stru_acore *)data;
    bsp_info("<%s> entry.\n", __FUNCTION__);
    while (g_rfile_while) {
        osl_sem_down(&rfile_info->sem_taskmain);
        if (rfile_info->shutdown_flag) {
            rfile_set_innner_state(RFILE_STATE_UNINITED);
            continue;
        } else if (rfile_info->recover_flag) {
            rfile_set_innner_state(RFILE_STATE_APP_RECOVERING);
            rfile_handle_app_recover(rfile_info);
        } else if (rfile_info->reset_flag) {
            rfile_set_innner_state(RFILE_STATE_RESET);
            rfile_handle_reset(rfile_info);
        } else {
            rfile_set_innner_state(RFILE_STATE_DOING);
            rfile_handle_cmd(rfile_info);
        }
        rfile_set_innner_state(RFILE_STATE_IDLE);
    }
    return 0;
}

int rfile_reset_before(void)
{
    if (g_rfile_acore_main.reset_flag == 1) {
        return 0;
    }
    g_rfile_acore_main.reset_flag = 1;
    rfile_set_event(RFILE_EVENT_RESET_BEFORE);
    if (osl_sem_downtimeout(&g_rfile_acore_main.sem_reset_before, msecs_to_jiffies(RFILE_WAIT_TIME_MS))) {
        bsp_err("<%s> reset handle timeout.\n", __FUNCTION__);
        g_rfile_acore_main.reset_flag = 0;
        return -1;
    }

    return 0;
}

int rfile_resetting(void)
{
    g_rfile_acore_main.reset_flag = 0;
    osl_sem_up(&g_rfile_acore_main.sem_resetting);
    rfile_set_event(RFILE_EVENT_RESETTING);

    return 0;
}

s32 bsp_rfile_reset_cb(drv_reset_cb_moment_e eparam, s32 userdata)
{
    int ret = 0;
    bsp_info("<%s> rfile reset in, param = %d.\n", __FUNCTION__, (int)eparam);
    if (eparam == MDRV_RESET_CB_BEFORE) {
        ret = rfile_reset_before();
    } else if (eparam == MDRV_RESET_RESETTING) {
        ret = rfile_resetting();
    }
    if (ret) {
        bsp_info("<%s> rfile reset fail.\n", __FUNCTION__);
    } else {
        bsp_info("<%s> rfile reset success.\n", __FUNCTION__);
    }
    (void)userdata;
    return ret;
}


int rfile_dev_open(struct inode *inode, struct file *file)
{
    static int first_flag = 1;
    if (strcmp(current->comm, RFILE_APP_NAME) && strncmp(current->comm, RFILE_APP_NAME_PHONE, strlen(current->comm))) {
        bsp_err("<%s> wrong task!!!current = %s\n", __FUNCTION__, current->comm);
        return -ENODEV;
    }
    if (first_flag) {
        first_flag = 0;
        return 0;
    }
    bsp_err("<%s> open again, app is killed, start recover\n", __FUNCTION__);
    g_rfile_acore_main.release_cnt++;
    g_rfile_acore_main.recover_flag = 1;
    rfile_set_event(RFILE_EVENT_APP_KILLED);

    (void)inode;
    (void)file;
    return 0;
}


int rfile_dev_mmap(struct file *file, struct vm_area_struct *vma)
{
    unsigned long size = vma->vm_end - vma->vm_start;
    unsigned long pfn_start = virt_to_phys(g_rfile_acore_main.data_buffer) >> PAGE_SHIFT;
    int ret;

    if (strcmp(current->comm, RFILE_APP_NAME) && strncmp(current->comm, RFILE_APP_NAME_PHONE, strlen(current->comm))) {
        bsp_err("<%s> wrong task!!!current = %s\n", __FUNCTION__, current->comm);
        return -ENODEV;
    }
    if (size != RFILE_USER_MAX_BUFSIZE) {
        bsp_err("<%s> user size(%ld) not match, buffer size(%d)\n", __FUNCTION__, size, RFILE_USER_MAX_BUFSIZE);
        return -1;
    }
    ret = remap_pfn_range(vma, vma->vm_start, pfn_start, size, vma->vm_page_prot);
    if (ret < 0) {
        bsp_err("<%s>: remap_pfn_range failed, ret = %d \n", __func__, ret);
        return ret;
    }

    return 0;
}
long rfile_ioctl_read(void)
{
    int ret;
    struct rfile_comm_cmd *cmd = (struct rfile_comm_cmd *)g_rfile_acore_main.data_buffer;

    ret = down_interruptible(&g_rfile_acore_main.sem_read);
    if (ret) {
        bsp_info("<%s>read wakeup by softirq, not err, ret = %d.\n", __FUNCTION__, ret);
        return -RFILE_IOCTL_INTERRUPT_ERRNO;
    }
    rfile_dump_handle_dbg(cmd->op_type, cmd->op_num, cmd->cmd_cnt, RFILE_ACORE_USER_SEND, 0);
    return 0;
}

long rfile_ioctl_write(void)
{
    struct rfile_comm_resp *resp = (struct rfile_comm_resp *)g_rfile_acore_main.data_buffer;
    if (resp == NULL) {
        bsp_err("<%s> resp buf is null.\n", __FUNCTION__);
        return -1;
    }
    if (g_rfile_acore_main.state == RFILE_STATE_UNINITED || g_rfile_acore_main.state == RFILE_STATE_IDLE) {
        bsp_err("<%s> not in recv user data state.\n", __FUNCTION__);
        return -1;
    }
    rfile_dump_handle_dbg(resp->op_type, resp->op_num, resp->cmd_cnt, RFILE_ACORE_USER_RECV, resp->ret);
    if (g_rfile_acore_main.state == RFILE_STATE_APP_RECOVERING) {
        osl_sem_up(&g_rfile_acore_main.sem_write);
    } else if (g_rfile_acore_main.state == RFILE_STATE_RESET) {
        osl_sem_up(&g_rfile_acore_main.sem_reset_before);
    } else if (g_rfile_acore_main.state == RFILE_STATE_DOING) {
        rfile_icc_send(resp, sizeof(struct rfile_comm_resp) + resp->datalen);
        rfile_dump_handle_dbg(resp->op_type, resp->op_num, resp->cmd_cnt, RFILE_ACORE_ICC_SEND,
            sizeof(struct rfile_comm_resp) + resp->datalen);
        osl_sem_up(&g_rfile_acore_main.sem_write);
    } else {
        bsp_info("%s: rfile cur state(%d), nothing to handle\n", __FUNCTION__, g_rfile_acore_main.state);
    }

    return 0;
}


long rfile_dev_ioctl(struct file *fp, unsigned int cmd, unsigned long arg)
{
    if (strcmp(current->comm, RFILE_APP_NAME) && strncmp(current->comm, RFILE_APP_NAME_PHONE, strlen(current->comm))) {
        bsp_err("<%s> wrong task!!!current = %s\n", __FUNCTION__, current->comm);
        return -ENODEV;
    }
    switch (cmd) {
        case RFILE_IOCTL_READ:
            return rfile_ioctl_read();
        case RFILE_IOCTL_WRITE:
            return rfile_ioctl_write();
        default:
            bsp_err("%s: ioctl cmd(%d) not valid\n", __FUNCTION__, cmd);
            return -1;
    }
    return 0;
}

static struct file_operations g_rfile_dev_fops = {
    .open = rfile_dev_open,
    .owner = THIS_MODULE,
    .mmap = rfile_dev_mmap,
    .unlocked_ioctl = rfile_dev_ioctl,
    .compat_ioctl = rfile_dev_ioctl,
};

void rfile_dev_free(struct rfile_dev_info *dev)
{
    struct cdev *cdev = &dev->cdev;

    cdev_del(cdev);
    unregister_chrdev_region(dev->dev_no, RFILE_DEV_NUM);
    return;
}

struct device *rfile_dev_setup(struct rfile_dev_info *dev)
{
    int ret;
    char dev_name[0xa];
    char dev_class[0x14];
    struct file_operations *fops = NULL;
    struct cdev *cdev = &dev->cdev;

    fops = &g_rfile_dev_fops;
    ret = strcpy_s((char *)dev_name, sizeof(dev_name), RFILE_DEV_NAME);
    if (ret) {
        bsp_err("<%s> strcpy_s dev_name err. ret = %d.\n", __FUNCTION__, ret);
        return NULL;
    }
    ret = strcpy_s((char *)dev_class, sizeof(dev_class), RFILE_DEV_CLASS);
    if (ret) {
        bsp_err("<%s> strcpy_s dev_classerr. ret = %d.\n", __FUNCTION__, ret);
        return NULL;
    }

    ret = alloc_chrdev_region(&dev->dev_no, 0, RFILE_DEV_NUM, dev_name);
    if (ret) {
        bsp_err("<%s> alloc_chrdev_region failed, dev_name = %s, ret = %d.\n", __FUNCTION__, dev_name, ret);
    }
    cdev_init(cdev, fops);
    cdev->owner = THIS_MODULE;
    cdev->ops = fops;
    ret = cdev_add(cdev, dev->dev_no, RFILE_DEV_NUM);
    if (ret) {
        bsp_err("<%s> cdev_add failed, dev_name = %s, ret = %d.\n", __FUNCTION__, dev_name, ret);
        unregister_chrdev_region(dev->dev_no, RFILE_DEV_NUM);
        return NULL;
    }
    dev->rfile_class = class_create(THIS_MODULE, (char *)dev_class);
    if (IS_ERR(dev->rfile_class)) {
        bsp_err("<%s> class_create failed, class_name = %s.\n", __FUNCTION__, dev_class);
        cdev_del(cdev);
        unregister_chrdev_region(dev->dev_no, RFILE_DEV_NUM);
        return NULL;
    }
    return device_create(dev->rfile_class, NULL, dev->dev_no, NULL, (char *)dev_name);
}

void rfile_init_prepare(void)
{
    osl_sem_init(0, &g_rfile_acore_main.sem_taskmain);
    osl_sem_init(0, &g_rfile_acore_main.sem_read);
    osl_sem_init(0, &g_rfile_acore_main.sem_write);
    osl_sem_init(0, &g_rfile_acore_main.sem_reset_before);
    osl_sem_init(0, &g_rfile_acore_main.sem_resetting);

    INIT_LIST_HEAD(&g_rfile_acore_main.fd_list);
    rfile_dump_init();

    return;
}

static int rfile_platform_probe(struct platform_device *dev)
{
    struct sched_param sch_para;
    sch_para.sched_priority = 0xf;

    bsp_err("[init]start.\n");
    rfile_init_prepare();

    g_rfile_acore_main.dev.device = rfile_dev_setup(&g_rfile_acore_main.dev);
    if (g_rfile_acore_main.dev.device == NULL) {
        return BSP_ERROR;
    }
    g_rfile_acore_main.data_buffer = kzalloc(RFILE_USER_MAX_BUFSIZE, GFP_KERNEL);
    if (g_rfile_acore_main.data_buffer == NULL) {
        bsp_err("<%s> kzalloc failed, len 0x%x.\n", __FUNCTION__, RFILE_USER_MAX_BUFSIZE);
        rfile_dev_free(&g_rfile_acore_main.dev);
        return BSP_ERROR;
    }
    if (rfile_icc_common_init()) {
        goto err;
    }
    g_rfile_acore_main.taskid_process = kthread_run(rfile_process_thread, &g_rfile_acore_main, "rfile_process");
    if (IS_ERR(g_rfile_acore_main.taskid_process)) {
        bsp_err("[init]: <%s> kthread_run rfile_process failed.\n", __FUNCTION__);
        goto err;
    }
    if (sched_setscheduler(g_rfile_acore_main.taskid_process, SCHED_FIFO, &sch_para)) {
        bsp_err("[init]: <%s> sched_setscheduler rfile_process failed.\n", __FUNCTION__);
        goto err;
    }
    osl_sem_up(&g_rfile_acore_main.sem_taskmain);
    bsp_err("[init]ok.\n");
    return BSP_OK;
err:
    rfile_dev_free(&g_rfile_acore_main.dev);
    kfree(g_rfile_acore_main.data_buffer);
    g_rfile_acore_main.data_buffer = NULL;
    return BSP_ERROR;
}


#ifdef CONFIG_PM
static s32 rfile_pm_suspend(struct device *dev)
{
    static u32 count = 0;
    if (g_rfile_acore_main.state != RFILE_STATE_IDLE) {
        bsp_err(" <%s> rfile enter suspend failed \n", __FUNCTION__);
        return -1;
    }
    count++;
    bsp_err(" <%s> rfile enter suspend! %d times \n", __FUNCTION__, count);
    return 0;
}

static const struct dev_pm_ops g_rfile_pm_ops = {
    .suspend = rfile_pm_suspend,
};

#define BALONG_RFILE_PM_OPS (&g_rfile_pm_ops)
#else
#define BALONG_RFILE_PM_OPS NULL
#endif

static void rfile_platform_shutdown(struct platform_device *dev)
{
    bsp_info("%s shutdown start \n", __func__);
    g_rfile_acore_main.shutdown_flag = 1;
    rfile_set_event(RFILE_EVENT_SHUTDOWN);
    return;
}

static struct platform_driver g_rfile_drv = {
    .probe      = rfile_platform_probe,
    .shutdown   = rfile_platform_shutdown,
    .driver     = {
        .name     = "modem_rfile",
        .owner    = THIS_MODULE,
        .pm       = BALONG_RFILE_PM_OPS,
        .probe_type = PROBE_FORCE_SYNCHRONOUS,
    },
};

struct platform_device g_rfile_device = {
    .name = "modem_rfile",
    .id = 0,
    .dev = {
        .init_name = "modem_rfile",
    },
};

struct device *rfile_get_device(void)
{
    return &g_rfile_device.dev;
}

int modem_rfile_init(void)
{
    int ret;
    (void)file_acore_init();
    ret = platform_device_register(&g_rfile_device);
    if (ret) {
        bsp_err("[init]platform_device_register g_rfile_device fail !\n");
        return -1;
    }
    ret = platform_driver_register(&g_rfile_drv);
    if (ret) {
        bsp_err("[init]platform_driver_register g_rfile_drv fail !\n");
        platform_device_unregister(&g_rfile_device);
        return -1;
    }
    return 0;
}

#if (FEATURE_DELAY_MODEM_INIT == FEATURE_OFF)
module_init(modem_rfile_init);
#endif
