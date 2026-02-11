

#ifndef __HISI_FLASH_HISEE_OTP_H
#define __HISI_FLASH_HISEE_OTP_H

#ifdef CONFIG_HISI_HISEE
void creat_flash_otp_thread(void);
void reinit_hisee_complete(void);
void release_hisee_complete(void);
bool flash_otp_task_is_started(void);
#else
static inline void creat_flash_otp_thread(void)
{
	return OK;
}

#endif

#endif
