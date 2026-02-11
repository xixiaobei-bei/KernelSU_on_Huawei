/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Description: Contexthub IPC send api.
 * Author: Huawei
 * Create: 2017-06-08
 */

#ifndef __HISI_INPUTHUB_API_H__
#define __HISI_INPUTHUB_API_H__
#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/err.h>
#include "protocol.h"
#ifdef CONFIG_INPUTHUB_20
#include "contexthub_route.h"
#include "contexthub_boot.h"
#include "contexthub_recovery.h"
#else
#include "inputhub_route.h"
#include "inputhub_bridge.h"

/*
 * 发送IPC消息给contexthub并等待回复
 */
extern int inputhub_mcu_write_cmd_adapter(const void *buf, unsigned int length, read_info_t *rd);
/*
 * 发送IPC消息给contexthub nolock
 */
extern int inputhub_mcu_write_cmd_nolock(const void *buf, unsigned int length);
/*
 * 注册 contexthub 复位后的恢复通知
 */
extern int register_iom3_recovery_notifier(struct notifier_block *nb);
#endif

#define CONTEXTHUB_HEADER_SIZE (sizeof(pkt_header_t) + sizeof(unsigned int))

/*
 * 注册 contexthub 消息通知
 */
extern int register_mcu_event_notifier(int tag, int cmd, int (*notify)(const pkt_header_t *head));
/*
 * 注销 contexthub 消息通知
 */
extern int unregister_mcu_event_notifier(int tag, int cmd, int (*notify) (const pkt_header_t *head));
/*
 * 获取contexthub模式
 */
extern int getSensorMcuMode(void);
/*
 * 发送IPC消息给contexthub
 */
int send_cmd_from_kernel(unsigned char cmd_tag, unsigned char cmd_type, unsigned int subtype, const char *buf, size_t count);
/*
 * 发送IPC消息给contexthub并等待回复
 */
int send_cmd_from_kernel_response(unsigned char cmd_tag, unsigned char cmd_type, unsigned int subtype, const char *buf, size_t count, struct read_info *rd);
/*
 * 发送IPC消息给contexthub nolock
 */
int send_cmd_from_kernel_nolock(unsigned char cmd_tag, unsigned char cmd_type, unsigned int subtype, const char *buf, size_t count);
#endif
