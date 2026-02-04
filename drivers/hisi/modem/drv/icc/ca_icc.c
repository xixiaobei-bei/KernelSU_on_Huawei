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
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <osl_common.h>
#include <osl_thread.h>
#include <osl_sem.h>
#include <drv_comm.h>
#include "icc_core.h"

#include <teek_client_api.h>
#include <teek_client_id.h>

typedef struct {
    u32 timeLow;
    u32 timeMid;
    u32 timeHiAndVersion;
    u32 clockSeqAndNode[8];
}DTS_UUID;

struct ca_icc_ctrl
{
    DTS_UUID   dts_uuid;
    const char *svc_name;
    u32  session_cmd_id;
    u32  is_dynamic_ta;
};

OSL_TASK_ID  ca_icc_task_id;

/******************************************************************************
Function:       TEEK_ca_icc_init
Description:    TEEK初始化
Input:          session
                context
                svc_uuid
                svc_name
Output:         none
Return:         0: OK  其他: ERROR码
******************************************************************************/
static int TEEK_ca_icc_init(TEEC_Session *session, TEEC_Context *context, TEEC_UUID *svc_uuid, const char *svc_name)
{
    TEEC_Result result;
    TEEC_Operation operation = {0};
    u32 root_id = 0;

    result = TEEK_InitializeContext(NULL, context);

    if(result != TEEC_SUCCESS) {
        icc_print_error("TEEK_InitializeContext failed,result = 0x%x!\n",result);
        return BSP_ERROR;
    }

    operation.started = 1;
    operation.cancel_flag = 0;
    operation.paramTypes = TEEC_PARAM_TYPES(
            TEEC_NONE,
            TEEC_NONE,
            TEEC_MEMREF_TEMP_INPUT,
            TEEC_MEMREF_TEMP_INPUT);
    operation.params[2].tmpref.buffer = (void *)(&root_id);
    operation.params[2].tmpref.size = sizeof(root_id);
    operation.params[3].tmpref.buffer = (void *)svc_name;
    operation.params[3].tmpref.size = strlen(svc_name) + 1;
    result = TEEK_OpenSession(
                context,
                session,
                svc_uuid,
                TEEC_LOGIN_IDENTIFY,
                NULL,
                &operation,
                NULL);

    if (result != TEEC_SUCCESS)
    {
        icc_print_error("TEEK_OpenSession failed,result = 0x%x!\n",result);
        TEEK_FinalizeContext(context);
        return BSP_ERROR;
    }

    return BSP_OK;
}

/******************************************************************************
Function:        TEEK_ca_icc_session
Description:     通过session传入命令字或值
Input:
        cmd_id      - 传送给安全OS的cmd_id
Output:          none
Return:          0: OK  其他: ERROR码
******************************************************************************/
static int TEEK_ca_icc_session(TEEC_Session *session, u32 cmd_id)
{
    TEEC_Result result;
    TEEC_Operation operation;
    unsigned int   origin;

    operation.started = 1;
    operation.cancel_flag = 0;
    operation.paramTypes = TEEC_PARAM_TYPES(
                           TEEC_NONE,
                           TEEC_NONE,
                           TEEC_NONE,
                           TEEC_NONE);

    result = TEEK_InvokeCommand(
                           session,
                           cmd_id,
                           &operation,
                           &origin);
    if (result != TEEC_SUCCESS) {
        icc_print_error("invoke failed!result = 0x%x!\n",result);
        result = BSP_ERROR;
    }

    return BSP_OK;
}

int ca_icc_task_func(void* data)
{
    TEEC_Session session;
    TEEC_Context context;
    struct ca_icc_ctrl *ca_icc_control = (struct ca_icc_ctrl *)data;
    TEEC_UUID svc_uuid;
    int i,ret = BSP_ERROR;

    svc_uuid.timeLow = ca_icc_control->dts_uuid.timeLow;
    svc_uuid.timeMid = (u16)ca_icc_control->dts_uuid.timeMid;
    svc_uuid.timeHiAndVersion = (u16)ca_icc_control->dts_uuid.timeHiAndVersion;
    for (i = 0; i < 8; i++)
	    svc_uuid.clockSeqAndNode[i] = (u8)ca_icc_control->dts_uuid.clockSeqAndNode[i];

    /* coverity[no_escape] */
    for( ; ; )
    {
        ret = TEEK_ca_icc_init(&session, &context, &svc_uuid, ca_icc_control->svc_name);
        if(BSP_ERROR == ret)
        {
            if(1 == ca_icc_control->is_dynamic_ta)
            {
                osl_task_delay(500);
                continue;
            }
            icc_print_error("TEEK_InitializeContext failed!\n");
            return ret;
        }

        ret = TEEK_ca_icc_session(&session, ca_icc_control->session_cmd_id);
        if(BSP_ERROR == ret)
        {
            icc_print_error("TEEK_cmd_session fail!\n");
        }

        TEEK_CloseSession(&session);
        TEEK_FinalizeContext(&context);
    }

    return ret;
}

int __init ca_icc_init(void)
{
    struct device_node *node = NULL;
    struct device_node *child_node = NULL;
    u32  task_priority;

    int ret = 0;

    node = of_find_compatible_node(NULL, NULL, "hisilicon,ca_icc");
    if (!node)
    {
        return -1;
    }

    for_each_available_child_of_node(node, child_node)
    {
        char task_name[32] = "";
        char cmd_id_str[16] = "";
        struct ca_icc_ctrl *dt_ca_icc;

        /* coverity[var_assign] *//* coverity[alloc_fn] */
        dt_ca_icc = (struct ca_icc_ctrl*)osl_malloc(sizeof(struct ca_icc_ctrl));
        if (!dt_ca_icc)
        {
            icc_print_error("dt_ca_icc malloc fail!\n");
            continue;
        }

        /* coverity[noescape] */
        ret |= of_property_read_u32_array(child_node, "svc_uuid", (unsigned int*)(&dt_ca_icc->dts_uuid), (unsigned int)(sizeof(DTS_UUID) / sizeof(unsigned int)));
        ret |= of_property_read_string(child_node, "svc_name", &dt_ca_icc->svc_name);
        ret |= of_property_read_u32(child_node, "session_cmd_id", &dt_ca_icc->session_cmd_id);
        ret |= of_property_read_u32(child_node, "task_priority", &task_priority);
        ret |= of_property_read_u32(child_node, "is_dynamic_ta", &dt_ca_icc->is_dynamic_ta);
        if (ret)
        {
            icc_print_error("ca_icc of read fail\n");
            osl_free((void *)dt_ca_icc);
            continue;
        }

        strncat(task_name, dt_ca_icc->svc_name, strlen(dt_ca_icc->svc_name));
        sprintf(cmd_id_str, "%d", dt_ca_icc->session_cmd_id);
        strncat(task_name, cmd_id_str, strlen(cmd_id_str));
        icc_print_error("task_name:%s\n",task_name);
        ret = osl_task_init(task_name, task_priority, 1024,(OSL_TASK_FUNC)ca_icc_task_func,(void *)dt_ca_icc, &ca_icc_task_id);
        if(ERROR == ret)
        {
            icc_print_error("ca icc task init fail!\n");
            osl_free((void *)dt_ca_icc);
            continue;
        }
        /* coverity[leaked_storage] */
    }

    icc_print_error("ok\n");

    return ret;
}

module_init(ca_icc_init);
