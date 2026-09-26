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
#include <linux/slab.h>
#include <linux/kernel.h>
#include <teek_client_id.h>
#include <teek_client_api.h>

#include <product_config.h>

#include <bsp_efuse.h>

#define THIS_MODU mod_mloader
#define mloader_print_err(fmt, ...) (bsp_err(fmt, ##__VA_ARGS__))
#define mloader_print_info(fmt, ...) (bsp_info(fmt, ##__VA_ARGS__))


#if (((defined CONFIG_TZDRIVER) && (defined CONFIG_LOAD_SEC_IMAGE)) || \
     ((defined CONFIG_TRUSTZONE_HM) && (defined CONFIG_LOAD_SEC_IMAGE)))

#ifdef CONFIG_TEE_DECOUPLE
#define MODEM_COMM_IMG (0x5)
#endif

static DEFINE_MUTEX(load_proc_lock);

static TEEC_Session g_load_session;
static TEEC_Context g_load_context;

/*
 * Function name:TEEK_init.
 * Discription:Init the TEEC and get the context
 * Parameters:
 *    @ session: the bridge from unsec world to sec world.
 *    @ context: context.
 * return value:
 *    @ TEEC_SUCCESS-->success, others-->failed.
 */
int mloader_teek_init(void)
{
    TEEC_Result result;
    TEEC_UUID svc_uuid = TEE_SERVICE_SECBOOT;
    TEEC_Operation operation = {0};
    const char *package_name = "sec_boot";
    u32 root_id = 0;

    result = TEEK_InitializeContext(NULL, &g_load_context);
    if (result != TEEC_SUCCESS) {
        /* cov_verified_start */
        mloader_print_err("TEEK_InitializeContext failed, result %#x\n", result);
        goto error;
        /* cov_verified_stop */
    }

    operation.started = 1;
    operation.cancel_flag = 0;
    operation.paramTypes = (u32)TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE, TEEC_MEMREF_TEMP_INPUT,
                                                 TEEC_MEMREF_TEMP_INPUT); /*lint !e845*/

    operation.params[2U].tmpref.buffer = (void *)(&root_id);
    operation.params[2U].tmpref.size = sizeof(root_id);
    operation.params[3U].tmpref.buffer = (void *)(package_name);
    operation.params[3U].tmpref.size = strlen(package_name) + 1;
    result = TEEK_OpenSession(&g_load_context, &g_load_session, &svc_uuid, TEEC_LOGIN_IDENTIFY, NULL, &operation, NULL);
    if (result != TEEC_SUCCESS) {
        /* cov_verified_start */
        mloader_print_err("TEEK_OpenSession failed, result %#x\n", result);
        TEEK_FinalizeContext(&g_load_context);
        /* cov_verified_stop */
    }

error:
    return (int)result;
}

int mloader_reset_modem(enum SVC_SECBOOT_IMG_TYPE image)
{
    TEEC_Session *session = &g_load_session;
    TEEC_Result result;
    TEEC_Operation operation;
    u32 origin;

    operation.started = 1;
    operation.cancel_flag = 0;

    operation.paramTypes = (u32)TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE); /*lint !e845*/

#ifdef CONFIG_TEE_DECOUPLE
    image += MODEM_START;
#endif
    operation.params[0].value.a = image;

    result = TEEK_InvokeCommand(session, SECBOOT_CMD_ID_RESET_IMAGE, &operation, &origin);
    if (result != TEEC_SUCCESS) {
        /* cov_verified_start */
        mloader_print_err("invoke failed, result %#x\n", result);
        /* cov_verified_stop */
    }

    return (int)result;
}

/*
 * Function name:trans_vrl_to_os.
 * Discription:transfer vrl data to sec_OS
 * Parameters:
 *    @ session: the bridge from unsec world to sec world.
 *    @ image: the data of the image to transfer.
 *    @ buf: the buf in  kernel to transfer
 *    @ size: the size to transfer.
 * return value:
 *    @ TEEC_SUCCESS-->success, others--> failed.
 */
int mloader_trans_vrl_to_os(enum SVC_SECBOOT_IMG_TYPE image, void *buf, const unsigned int size)
{
    TEEC_Session *session = &g_load_session;
    TEEC_Result result;
    TEEC_Operation operation;
    u32 origin;

    operation.started = 1;
    operation.cancel_flag = 0;
    operation.paramTypes = (u32)TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_MEMREF_TEMP_INPUT, TEEC_NONE,
                                                 TEEC_NONE); /*lint !e845*/

#ifdef CONFIG_TEE_DECOUPLE
    image += MODEM_START;
#endif
    operation.params[0].value.a = image;

    operation.params[1].tmpref.buffer = (void *)buf;
    operation.params[1].tmpref.size = size;

    result = TEEK_InvokeCommand(session, SECBOOT_CMD_ID_COPY_VRL_TYPE, &operation, &origin);
    if (result != TEEC_SUCCESS) {
        /* cov_verified_start */
        mloader_print_err("invoke failed, result %#x\n", result);
        /* cov_verified_stop */
    }

    return (int)result;
}

/*
 * Function name:trans_data_to_os.
 * Discription:transfer image data to sec_OS
 * Parameters:
 *    @ session: the bridge from unsec world to sec world.
 *    @ image: the data of the image to transfer.
 *    @ run_addr: the image entry address.
 *    @ buf: the buf in  kernel to transfer
 *    @ offset: the offset to run_addr.
 *    @ size: the size to transfer.
 * return value:
 *    @ TEEC_SUCCESS-->success, others--> failed.
 */
int mloader_trans_data_to_os(enum SVC_SECBOOT_IMG_TYPE image, u32 reserved, u64 buf, const unsigned int offset,
                             const unsigned int size)
{
    TEEC_Session *session = &g_load_session;
    TEEC_Result result;
    TEEC_Operation operation;
    u32 origin;

    operation.started = 1;
    operation.cancel_flag = 0;
    operation.paramTypes = (u32)TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_VALUE_INPUT, TEEC_VALUE_INPUT,
                                                 TEEC_VALUE_INPUT); /*lint !e845*/

#ifdef CONFIG_TEE_DECOUPLE
    image += MODEM_START;
#endif
    operation.params[0].value.a = image;

    /* the follow 64 bits is reserved for dst buffer, which is obsolete */
    operation.params[0].value.b = 0;
    operation.params[1].value.a = 0;

    operation.params[1].value.b = offset;

    /* the follow 64 bits is reserved for src buffer */
    operation.params[2U].value.a = (u32)buf;
    operation.params[2U].value.b = (u64)buf >> 32U;

    operation.params[3U].value.a = size;
    result = TEEK_InvokeCommand(session, SECBOOT_CMD_ID_COPY_DATA_TYPE, &operation, &origin);
    if (result != TEEC_SUCCESS) {
        /* cov_verified_start */
        mloader_print_err("invoke failed, result %#x\n", result);
        /* cov_verified_stop */
    }

    return (int)result;
}

/*
 * Function name:start_soc_image.
 * Discription:start the image verification, if success, unreset the soc
 * Parameters:
 *    @ session: the bridge from unsec world to sec world.
 *    @ image: the image to verification and unreset
 *    @ run_addr: the image entry address
 * return value:
 *    @ TEEC_SUCCESS-->success, others-->failed.
 */
int mloader_verify_soc_image(enum SVC_SECBOOT_IMG_TYPE image, u32 core_id)
{
    TEEC_Session *session = &g_load_session;
    TEEC_Result result;
    TEEC_Operation operation;
    u32 origin;
    unsigned long paddr;
    int ret;

    paddr = core_id;

    ret = bsp_efuse_write_prepare();
    if (ret) {
        return ret;
    }
    operation.started = 1;
    operation.cancel_flag = 0;
    operation.paramTypes = (u32)TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_VALUE_INPUT, TEEC_NONE,
                                                 TEEC_NONE); /*lint !e845*/

#ifdef CONFIG_TEE_DECOUPLE
    image += MODEM_START;
#endif
    operation.params[0].value.a = image;

    operation.params[0].value.b = 0; /* SECBOOT_LOCKSTATE , not used currently */
    operation.params[1].value.a = (u32)(paddr & 0xFFFFFFFF);
    operation.params[1].value.b = (u32)((u64)paddr >> 32U); /* 手机和MBB 兼容 */
    result = TEEK_InvokeCommand(session, SECBOOT_CMD_ID_VERIFY_DATA_TYPE, &operation, &origin);
    if (result != TEEC_SUCCESS) {
        /* cov_verified_start */
        mloader_print_err("image %d verify failed, result is 0x%x!\n", image, result);
        /* cov_verified_stop */
    }
    mloader_print_err("image %d verify succeed!\n", image);
    bsp_efuse_write_complete();
    return (int)result;
}

int mloader_verify_modem_image(unsigned core_id)
{
    int ret;
    mutex_lock(&load_proc_lock);

    mloader_print_err("start verify modem_comm_image\n");
    ret = mloader_verify_soc_image(MODEM_COMM_IMG, core_id);
    if (ret) {
        mloader_print_err("verify modem_comm_image failed\n");
    }
    mloader_print_err("verify modem_comm_image succeed\n");
    mutex_unlock(&load_proc_lock);

    return ret;
}

#else
int mloader_teek_init(void)
{
    return 0;
}
int mloader_reset_modem(enum SVC_SECBOOT_IMG_TYPE image)
{
    return 0;
}
int mloader_verify_soc_image(enum SVC_SECBOOT_IMG_TYPE image, u32 core_id)
{
    return 0;
}
int mloader_verify_modem_image(unsigned core_id)
{
    return 0;
}
#endif
