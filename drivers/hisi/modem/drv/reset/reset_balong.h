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

#ifndef __RESET_BALONG_H__
#define __RESET_BALONG_H__

#ifdef __cplusplus /* __cplusplus */
extern "C"
{
#endif /* __cplusplus */

#include <linux/wakelock.h>
#include <linux/printk.h>
#include <linux/suspend.h>
#include <linux/workqueue.h>
#include <linux/completion.h>
#include <bsp_print.h>

#include <osl_sem.h>
#include <osl_spinlock.h>
#include <acore_nv_stru_drv.h>
#include <gunas_errno.h>

#include <bsp_reset.h>
#include <bsp_print.h>

/************************** 宏和枚举定义 **************************/
#define RESET_WAIT_RESP_TIMEOUT          (5000)   /*ms, time of wating for reply from hifi/mcu*/
#define RESET_WAIT_MODEM_IN_IDLE_TIMEOUT (310)   /*ms, time of wating for reply from modem in idle*/
#define RESET_WAIT_M3_RESETING_REPLY_TIMEOUT (200)   /*ms, time of wating for reply from modem in idle*/
#define RESET_WAIT_CCORE_WAKEUP_REPLY_TIMEOUT (100)   /*ms, time of wating for modem wakeup*/
#define RESET_WAIT_CCPU_STARTUP_TIMEOUT  (20000)
#define DRV_RESET_MODULE_NAME_LEN        (9)
#define THIS_MODU mod_reset
#define reset_print_err(fmt, ...)        (bsp_err("[%s] "fmt, __FUNCTION__, ##__VA_ARGS__))
#define reset_print_shorterr(fmt, ...) (bsp_err(fmt, ##__VA_ARGS__))
#define RESET_RETRY_TIMES (3)
#define RESET_IS_SUCC                    (1)
#define CP_RESET_DUMP_SIZE          (1024)
#define CP_RESET_DUMP_INVOKE_END    (750)
#define CP_RESET_DUMP_INVOKE_OFFSET (256)
#define CP_RESET_DUMP_MASTER_OFFSET (768)
#define RESET_DUMP_MAGIC            (0x54534552)   /* REST */

/*
 * cpreset 主流程关键步骤打点
 */
enum RESET_DUMP_MOD_ID
{
	RESET_DUMP_START               = 0,
	RESET_DUMP_PREPARE             = 1,
	RESET_DUMP_POWER_OFF           = 2,
	RESET_DUMP_CB_BEFORE_START     = 3,
	RESET_DUMP_CB_BEFORE_END       = 4,
	RESET_DUMP_SECCALL_OFF_START   = 5,
	RESET_DUMP_SECCALL_OFF_END     = 6,
	RESET_DUMP_SECCALL_ERROR       = 7,
	RESET_DUMP_HIFI_START          = 8,
	RESET_DUMP_HIFI_END            = 9,
	RESET_DUMP_MIDLE_TIMEOUT       = 10,
	RESET_DUMP_MIDLE_SUCCESS       = 11,
	RESET_DUMP_M3_BEFORE_FAIL      = 12,
	RESET_DUMP_M3_BEFORE_ACK_FAIL  = 13,
	RESET_DUMP_M3_BEFORE_SUCCEED   = 14,
	RESET_DUMP_SECCALL_RESET_START = 15,
	RESET_DUMP_SECCALL_RESET_END   = 16,
	RESET_DUMP_LOAD_IMAGE_START    = 17,
	RESET_DUMP_LOAD_IMAGE_END      = 18,
	RESET_DUMP_M3_ING_FAIL         = 19,
	RESET_DUMP_M3_ING_ACK_FAIL     = 20,
	RESET_DUMP_M3_ING_SUCCESS      = 21,
	RESET_DUMP_SECCALL_ON_START    = 22,
	RESET_DUMP_SECCALL_ON_END      = 23,
	RESET_DUMP_CP_STARTUP          = 24,
	RESET_DUMP_CP_STARTUP_TIMEOUT  = 25,
	RESET_DUMP_CB_AFTER_START      = 26,
	RESET_DUMP_CB_AFTER_END        = 27,
	RESET_DUMP_M3_AFTER_FAIL       = 28,
	RESET_DUMP_M3_AFTER_ACK_FAIL   = 29,
	RESET_DUMP_M3_AFTER_SUCCESS    = 30,
	RESET_DUMP_END                 = 31,

	RESET_DUMP_MOD_ID_MAX
};


enum RESET_TYPE
{
    RESET_TYPE_CB_INVOKE_BEFORE,
    RESET_TYPE_SEND_MSG2_M3_FAIL_BEFORE,
    RESET_TYPE_RECV_WRONG_MSG_FROM_M3_BEFORE,
    RESET_TYPE_SEND_MSG2_HIFI_FAIL_BEFORE,
    RESET_TYPE_WAIT_HIFI_REPLY_FAIL_BEFORE,
    RESET_TYPE_LOAD_MODEM_IMG_FAIL,
    RESET_TYPE_SEND_MSG2_M3_FAIL_RESTING,
    RESET_TYPE_RECV_WRONG_MSG_FROM_M3_RESTING,
    RESET_TYPE_M3_REPLY_FAIL_RESTING,
    RESET_TYPE_SEND_MSG2_M3_FAIL_AFTER,
    RESET_TYPE_RECV_WRONG_MSG_FROM_M3_AFTER,
    RESET_TYPE_CB_INVOKE_RESTING,
    RESET_TYPE_WAIT_CCORE_RELAY_TIMEOUT,
    RESET_TYPE_CB_INVOKE_AFTER,
    RESET_TYPE_ICC_READ_FAIL,
    RESET_TYPE_RECV_M3_MSG_FAIL,
    RESET_TYPE_RECV_M3_MSG_BUS_ERR,
    RESET_TYPE_RECV_CCORE_MSG_FAIL,
    RESET_TYPE_RECV_HIFI_MSG_FAIL,

    /*add new above,please!!*/
    RESET_TYPE_FOR_TEST,        /*用于测试*/
    RESET_TYPE_MAX
} ;

typedef enum _modem_reset
{
    MODEM_RESET_DRV_ERR               = DRV_ERRNO_MODEM_RST_FAIL,   /* DRV异常 */
    MODEM_RESET_NAS_CB_ERR            = NAS_REBOOT_MOD_ID_RESET,    /* NAS回调失败 */
    MODEM_RESET_LOAD_SEC_IMAGE_ERR    = TEEOS_ERRNO_LOAD_SEC_IMAGE, /* 加载安全镜像失败 */
    MODEM_RESET_HIFI_CB_ERR           = HIFI_ERRNO_MODEM_RESET,     /* HIFI回调失败 */
    MODEM_RESET_M3_ERR                = LPM3_ERRNO_MODEM_RESET,     /* M3异常 */
}modem_reset_e;

/* 结构体定义 */

/*Record information of callback functions*/
struct reset_cb_info
{
    char name[DRV_RESET_MODULE_NAME_LEN + 1];
    u32 priolevel;
    pdrv_reset_cbfun cbfun;
    s32 userdata;
};

struct reset_cb_list
{
	struct reset_cb_info cb_info;
	struct reset_cb_list *next;
};

struct modem_reset_ctrl
{
	u32 boot_mode;            /* 表示ccore启动状态: 正常启动/单独复位后启动 */
	u32 multicore_msg_switch; /* 核间消息开关: 1, 表示打开; 0表示关闭 */
	struct task_struct* task;
	u32 modem_action;
	spinlock_t action_lock;
	osl_sem_id action_sem;
	osl_sem_id task_sem;
	osl_sem_id wait_mcore_reply_sem;
	osl_sem_id wait_mcore_reply_reseting_sem;
	osl_sem_id wait_hifi_reply_sem;
	osl_sem_id wait_ccore_reset_ok_sem;
	osl_sem_id wait_modem_master_in_idle_sem;
	struct reset_cb_list *list_head;
	struct workqueue_struct *reset_wq;
	struct work_struct  work_reset;
	struct work_struct  work_power_off;
	struct work_struct  work_power_on;
	struct wake_lock wake_lock;
	struct notifier_block pm_notify;
	struct completion suspend_completion;
	IPC_INT_LEV_E ipc_send_irq_wakeup_ccore;
	IPC_INT_LEV_E ipc_recv_irq_idle;
	IPC_INT_LEV_E ipc_recv_irq_reboot;
	s32 in_suspend_state;
	u32 list_use_cnt;
	u32 reset_cnt;
	void *crg_base;
	u32 exec_time;
	DRV_CCORE_RESET_STRU nv_config;
	u32 state;
};

/*
 * struct reset_dump_header - The reset dump file header entry table structure
 * @magic	: indicating module's name defined by enum RESET_DUMP_MAGIC
 * @len		: length of module's length among DUMP/LOG area
 */
struct reset_dump_header
{
	u32  magic;
	u32  len;
	u32  version;
	DRV_CCORE_RESET_STRU nv_config;
};

struct reset_dump_cfg
{
	void *base_addr;
	char *invoke_addr;
	char *master_addr;
	struct reset_dump_header *file_head;
	u32  *entry_tbl;
};

struct modem_reset_debug
{
	u32 print_sw;
	u32 main_stage;
	u32 dump_state;
	struct reset_dump_cfg dump_info;
};

extern struct modem_reset_debug g_reset_debug;

#define reset_has_debug_print
#ifdef reset_has_debug_print

#define  reset_print_debug(fmt, ...) \
do {                               \
    if (g_reset_debug.print_sw)    \
        reset_print_err(fmt, ##__VA_ARGS__);\
} while (0)

#else
#define  reset_print_debug(fmt, ...)
#endif

#define reset_stamp(macro_addr)   readl((volatile const void *)macro_addr)

/* 打点时间戳 */
static inline void cp_reset_timestamp_dump(enum RESET_DUMP_MOD_ID reset_dumpid)
{
    if (g_reset_debug.dump_state == (u32)RESET_DUMP_MAGIC)
        g_reset_debug.dump_info.entry_tbl[reset_dumpid] = bsp_get_slice_value();
}


/* 函数声明 */
struct modem_reset_ctrl *bsp_reset_control_get(void);
s32 send_sync_msg_to_mcore(u32 reset_info, u32 *ack_val);
s32 reset_pm_notify(struct notifier_block *notify_block,unsigned long mode, void *unused);

#ifdef __cplusplus /* __cplusplus */
}
#endif /* __cplusplus */

#endif    /*  __RESET_BALONG_H__ */
