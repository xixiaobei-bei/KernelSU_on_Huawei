

#ifndef __OMXVDEC_H__
#define __OMXVDEC_H__

#include "platform.h"
#include "public.h"
#include "drv_omxvdec.h"

#define OMXVDEC_VERSION         2017032300
#define MAX_OPEN_COUNT          32
#define IORE_MAP_PARA           4

typedef struct {
	hi_u32 open_count;
	atomic_t nor_chan_num;
	atomic_t sec_chan_num;
	mem_buffer_s com_msg_pool;
	struct mutex omxvdec_mutex;
	struct mutex vdec_mutex_scd;
	struct mutex vdec_mutex_vdh;
	struct mutex vdec_mutex_sec_scd;
	struct mutex vdec_mutex_sec_vdh;
	hi_bool device_locked;
	struct cdev cdev;
	struct device *device;
} omx_vdec_entry;

typedef struct {
	hi_s32 share_fd;
	UADDR  iova;
	hi_u64 vir_addr;
	hi_u32 iova_size;
} vdec_buffer_record;

omx_vdec_entry omx_vdec_get_entry(hi_void);

#endif
