/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2020. All rights reserved.
 * Description: provides interfaces for writing hisec otp
 * Create: 2017/09/15
 */

#ifndef FLASH_HISEE_OTP_H
#define FLASH_HISEE_OTP_H

#if defined(CONFIG_HISEE) || defined(CONFIG_MSPC)
void creat_flash_otp_thread(void);
#else
static inline void creat_flash_otp_thread(void)
{
}
#endif

#ifdef CONFIG_HISEE
void reinit_hisee_complete(void);
void release_hisee_complete(void);
bool flash_otp_task_is_started(void);
s32 efuse_check_secdebug_disable(bool *b_disabled);
void register_flash_hisee_otp_fn(int (*fn_ptr)(void));
#endif

#endif
