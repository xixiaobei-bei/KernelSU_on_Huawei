
#ifndef __DRV_VENC_OSAL_H__
#define __DRV_VENC_OSAL_H__

#include <linux/rtc.h>
#include "hi_type.h"
#include "hal_venc.h"

typedef struct hi_kernel_event {
	wait_queue_head_t   queue_head;
	HI_S32              flag;
} kernel_event_t;

typedef kernel_event_t  vedu_osal_event_t;
typedef unsigned long vedu_lock_flag_t;
typedef struct timer_list venc_timer_t;

#define TIME_PERIOD(begin, end) (((end) >= (begin)) ? ((end) - (begin)) : (~0LL - (begin) + (end)))
#define  MESCS_TO_JIFFIES(time)                         msecs_to_jiffies(time)
#define  HI_WAIT_EVENT(event, flag)                     wait_event_interruptible((event), (flag))
#define  HI_WAIT_EVENT_TIME_OUT(event, flag, ms_wait_time)  wait_event_interruptible_timeout((event), (flag), (ms_wait_time))

extern HI_U32  g_venc_print_enable;

typedef enum {
	VENC_FATAL = 0,
	VENC_ERR,
	VENC_WARN,
	VENC_INFO,
	VENC_DBG,
	VENC_ALW,
} venc_print_t;

#define HI_FATAL_VENC(fmt, ...) hi_print(VENC_FATAL, (char *)__FILE__, (int)__LINE__, (char *)__func__, fmt, ##__VA_ARGS__)
#define HI_ERR_VENC(fmt, ...)   hi_print(VENC_ERR, (char *)__FILE__, (int)__LINE__, (char *)__func__, fmt, ##__VA_ARGS__)
#define HI_WARN_VENC(fmt, ...)  hi_print(VENC_WARN, (char *)__FILE__, (int)__LINE__, (char *)__func__, fmt, ##__VA_ARGS__)
#define HI_INFO_VENC(fmt, ...)  hi_print(VENC_INFO, (char *)__FILE__, (int)__LINE__, (char *)__func__, fmt, ##__VA_ARGS__)
#define HI_DBG_VENC(fmt, ...)   hi_print(VENC_DBG, (char *)__FILE__, (int)__LINE__, (char *)__func__, fmt, ##__VA_ARGS__)

#define OSAL_WAIT_EVENT_TIMEOUT(event, condtion, timeout_in_ms) \
({ \
	int ret = timeout_in_ms; \
	HI_U64 start_time, cur_time; \
	start_time = osal_get_sys_time_in_ms(); \
	while (!(condtion) && (ret != 0)) { \
		ret = wait_event_interruptible_timeout(((event)->queue_head), (condtion), (msecs_to_jiffies(timeout_in_ms))); /*lint !e665 !e666 !e40 !e713 !e578*/ \
		if (ret < 0) { \
			cur_time = osal_get_sys_time_in_ms(); \
			if (TIME_PERIOD(start_time, cur_time) > (HI_U64)(timeout_in_ms)) { \
				HI_FATAL_VENC("waitevent time out, time : %lld", \
					TIME_PERIOD(start_time, cur_time)); \
				ret = 0; \
				break; \
			} \
		} \
	} \
	if (ret == 0) \
		HI_WARN_VENC("waitevent timeout"); \
	if ((condtion)) { \
		ret = HI_SUCCESS; \
	} else { \
		ret = HI_FAILURE; \
	} \
	ret; \
})

HI_VOID     hi_print(HI_U32 type, const char *file, int line, const char *function, HI_CHAR *msg, ...);
HI_U32      *hi_mmap(HI_U32 addr, HI_U32 range);
HI_VOID      hi_munmap(HI_U32 *mem_addr);
HI_S32       hi_strncmp(const HI_PCHAR str_name, const HI_PCHAR dst_name, HI_S32 size);
HI_VOID      hi_sleep_ms(HI_U32 millisec);
HI_VOID     *hi_mem_valloc(HI_U32 mem_size);
HI_VOID      hi_mem_vfree(HI_VOID *mem_addr);
HI_VOID      hi_venc_init_mutex(HI_VOID *sem);
HI_S32       hi_venc_down_interruptible(HI_VOID *sem);
HI_VOID      hi_venc_up_interruptible(HI_VOID *sem);

HI_S32 venc_drv_osal_irq_init(HI_U32 irq, irqreturn_t (*callback)(HI_S32, HI_VOID *));
HI_VOID  venc_drv_osal_irq_free(HI_U32 irq);
HI_S32   venc_drv_osal_lock_create(spinlock_t **phlock);
HI_VOID  venc_drv_osal_lock_destroy(spinlock_t *hlock);

HI_S32 venc_drv_osal_init_event(vedu_osal_event_t *event, HI_S32 initval);
HI_S32 venc_drv_osal_give_event(vedu_osal_event_t *event);
HI_S32 venc_drv_osal_wait_event(vedu_osal_event_t *event, HI_U32 ms_wait_time);

HI_S32 osal_init_timer(struct timer_list *timer,
		void (*function)(unsigned long),
		unsigned long data);
HI_VOID osal_add_timer(struct timer_list *timer, HI_U32 time_in_ms);
HI_S32 osal_del_timer(struct timer_list *timer, HI_BOOL is_sync);
HI_U64 osal_get_sys_time_in_ms(void);

#endif // __DRV_VENC_OSAL_H__

