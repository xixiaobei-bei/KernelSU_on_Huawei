
#ifndef __DRV_VENC_EFL_H__
#define __DRV_VENC_EFL_H__

#include <linux/rtc.h>

#include "hi_type.h"
#include "hi_drv_mem.h"
#include "drv_venc_ioctl.h"
#include "drv_venc_osal.h"
#include "venc_regulator.h"

#define MAX_OPEN_COUNT 3
#define ENCODE_TIME_MS 100
#define INTERRUPT_TIMEOUT_MS 300
#define ENCODE_DONE_TIMEOUT_MS 500
#define WAIT_CORE_IDLE_TIMEOUT_MS 700
#define FPGA_WAIT_EVENT_TIME_MS 1000000

#define MAX_CHANNEL_NUM 16
#define MAX_RING_BUFFER_SIZE (MAX_CHANNEL_NUM * MAX_SUPPORT_CORE_NUM)

enum {
	VEDU_H265 = 0,
	VEDU_H264 = 1
};

typedef enum {
	VENC_POWER_OFF,
	VENC_IDLE,
	VENC_BUSY,
	VENC_TIME_OUT,
} venc_ip_status_t;

typedef enum {
	YUV420_SEMIPLANAR     = 0,
	YUV420_PLANAR         = 3,
	YUV422_PLANAR         = 4,
	YUV422_PACKAGE        = 6,
	RGB_32BIT             = 8,
	YUV420_SEMIPLANAR_CMP = 10,
} color_format_t;

typedef struct {
	mem_buffer_t internal_buffer;
	mem_buffer_t image_buffer;
	mem_buffer_t stream_buffer[MAX_SLICE_NUM];
	mem_buffer_t stream_head_buffer;
} venc_buffer_info_t;

/* create in open fd, write in interrupt process, read in postthread, destory in close fd */
struct venc_fifo_buffer {
	spinlock_t *lock;
	DECLARE_KFIFO_PTR(fifo_buffer, struct encode_done_info);
};

/* VENC IP Context */
struct venc_context {
	venc_ip_status_t status;
	struct venc_fifo_buffer *buffer;
	struct channel_info  channel;
	HI_U32  *reg_base;
	HI_U32  irq_num_normal;
	HI_U32  irq_num_protect;
	HI_U32  irq_num_safe;
	HI_BOOL first_cfg_flag;
	HI_BOOL is_block;
	venc_timer_t timer;
	HI_SIZE_T tick;
};

extern struct venc_context g_venc_context[MAX_SUPPORT_CORE_NUM];
vedu_osal_event_t *venc_drv_get_encode_done_event_handle(void);
HI_S32 venc_drv_alloc_encode_done_info_buffer(struct file  *file);
HI_S32 venc_drv_free_encode_done_info_buffer(const struct file  *file);
irqreturn_t venc_drv_encode_done(HI_S32 irq, HI_VOID *dev_id);
HI_S32 venc_drv_encode(struct encode_info *encode_info, struct venc_fifo_buffer *buffer);
HI_S32 venc_drv_get_encode_done_info(struct venc_fifo_buffer *buffer, struct encode_done_info *encode_done_info);
HI_S32 venc_drv_init_lock(void);
HI_VOID venc_drv_destroy_lock(void);
HI_S32 venc_drv_open_vedu(void);
HI_S32 venc_drv_close_vedu(void);
HI_S32 venc_drv_suspend_vedu(void);
HI_S32 venc_drv_resume_vedu(void);
HI_S32 venc_drv_resume(struct platform_device *pdev);
HI_S32 venc_drv_suspend(struct platform_device *pdev, pm_message_t state);
HI_S32 venc_check_coreid(HI_S32 core_id);

#endif //__DRV_VENC_EFL_H__

