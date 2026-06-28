#ifndef __OMXVDEC_H__
#define __OMXVDEC_H__

#include "platform.h"
#include "public.h"
#include "drv_omxvdec.h"

#define OMXVDEC_VERSION         (2017032300)
#define MAX_OPEN_COUNT          (32)

typedef struct {
	HI_U32 open_count;
	atomic_t nor_chan_num;
	atomic_t sec_chan_num;
	MEM_BUFFER_S com_msg_pool;
	struct mutex omxvdec_mutex;
	struct mutex vdec_mutex_scd;
	struct mutex vdec_mutex_vdh;
	struct mutex vdec_mutex_sec_scd;
	struct mutex vdec_mutex_sec_vdh;
	HI_BOOL device_locked;
	struct cdev cdev;
	struct device *device;
} OMXVDEC_ENTRY;

#endif
