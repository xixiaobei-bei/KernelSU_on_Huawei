/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: platform config file for hisp310
 * Author: sunshaodan
 * Create: 2018-08-20
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 */

#include <linux/compiler.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/rpmsg.h>
#include <linux/skbuff.h>
#include <linux/types.h>
#include <linux/videodev2.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-subdev.h>
#include <media/videobuf2-core.h>
#include <linux/pm_qos.h>
#include <clocksource/arm_arch_timer.h>
#include <asm/arch_timer.h>
#include <linux/time.h>
#include <linux/jiffies.h>
#include <linux/pm_wakeup.h>
#include <linux/hisi-iommu.h>
#include <linux/platform_data/remoteproc_hisp.h>
#include <linux/platform_data/hisp_mempool.h>
#include <linux/iommu.h>
#include <linux/mutex.h>
#include <media/huawei/camera.h>
#include <media/huawei/hisp250_cfg.h>
#include <hicam_buf.h>
#include "cam_log.h"
#include "hisp_intf.h"
#include "platform/sensor_commom.h"
#include <securec.h>

#define HISP_MSG_LOG_MOD 100

#define ISP_TURBO_ISPFUNC_CLK_RATE  640000000
#define ISP_TURBO_ISPFUNC2_CLK_RATE 640000000
#define ISP_TURBO_ISPFUNC3_CLK_RATE 541000000
#define ISP_TURBO_ISPFUNC4_CLK_RATE 541000000

#define ISP_NORMAL_ISPFUNC_CLK_RATE  480000000
#define ISP_NORMAL_ISPFUNC2_CLK_RATE 480000000
#define ISP_NORMAL_ISPFUNC3_CLK_RATE 400000000
#define ISP_NORMAL_ISPFUNC4_CLK_RATE 400000000

#define ISP_LOWPOWER_ISPFUNC_CLK_RATE  325000000
#define ISP_LOWPOWER_ISPFUNC2_CLK_RATE 325000000
#define ISP_LOWPOWER_ISPFUNC3_CLK_RATE 300000000
#define ISP_LOWPOWER_ISPFUNC4_CLK_RATE 300000000

#define ISP_ULTRALOW_ISPFUNC_CLK_RATE  203000000
#define ISP_ULTRALOW_ISPFUNC2_CLK_RATE 203000000
#define ISP_ULTRALOW_ISPFUNC3_CLK_RATE 166000000
#define ISP_ULTRALOW_ISPFUNC4_CLK_RATE 181000000

#define ISP_MIDDLE_ISPFUNC3_CLK_RATE   172000000
#define ISP_MIDDLE_ISPFUNC4_CLK_RATE   172000000

#define R8_TURBO_ISPCPU_CLK_RATE    1200000000
#define R8_NORMAL_ISPCPU_CLK_RATE   1200000000
#define R8_LOWPOWER_ISPCPU_CLK_RATE 1200000000
#define R8_ULTRALOW_ISPCPU_CLK_RATE 1200000000

#define TIMEOUT_IS_FPGA_BOARD 60000
#define TIMEOUT_IS_NOT_FPGA_BOARD 15000

DEFINE_MUTEX(hisi_rpmsg_service_mutex);

#ifdef CONFIG_HISI_DEBUG_FS
static struct pm_qos_request qos_request_ddr_down_record;
static int current_ddr_bandwidth = 0;
#endif
static struct wakeup_source hisp_power_wakelock;
static struct mutex hisp_wake_lock_mutex;
static struct mutex hisp_power_lock_mutex;
static struct mutex hisp_mem_lock_mutex;

extern void hisp_boot_stat_dump(void);
extern int hisp_secmem_size_get(unsigned int*);

static void hisp310_deinit_isp_mem(void);
typedef enum _timestamp_state_t {
	TIMESTAMP_UNINTIAL = 0,
	TIMESTAMP_INTIAL,
} timestamp_state_t;

static timestamp_state_t s_timestamp_state;
static struct timeval s_timeval;
static u32 s_system_couter_rate;
static u64 s_system_counter;

enum hisp310_rpmsg_state {
	RPMSG_UNCONNECTED,
	RPMSG_CONNECTED,
	RPMSG_FAIL,
};

enum isp_clk_level {
	ISP_CLK_LEVEL_TURBO,
	ISP_CLK_LEVEL_NORMAL,
	ISP_CLK_LEVEL_LOWPOWER,
	ISP_CLK_LEVEL_ULTRALOW,
	ISP_CLK_LEVEL_MAX,
};
static int clk_level_pre = ISP_CLK_LEVEL_NORMAL;

/*
 * These are used for distinguish the rpmsg_msg status
 * The process in hisp310_rpmsg_ept_cb are different
 * for the first receive and later.
 */
enum {
	HISP_SERV_FIRST_RECV,
	HISP_SERV_NOT_FIRST_RECV,
};

/*
 * @brief the instance for rpmsg service
 * When Histar ISP is probed, this sturcture will be initialized,
 * the object is used to send/recv rpmsg and store the rpmsg data
 * @end
 */
struct rpmsg_hisp310_service {
	struct rpmsg_device *rpdev;
	struct mutex send_lock;
	struct mutex recv_lock;
	struct completion *comp;
	struct sk_buff_head queue;
	wait_queue_head_t readq;
	struct rpmsg_endpoint *ept;
	u32 dst;
	int state;
	char recv_count;
};

enum hisp310_mem_pool_attr {
	MEM_POOL_ATTR_READ_WRITE_CACHE = 0,
	MEM_POOL_ATTR_READ_WRITE_SECURITY,
	MEM_POOL_ATTR_READ_WRITE_ISP_SECURITY,
	MEM_POOL_ATTR_READ_WRITE_CACHE_OFF_LINE,
	MEM_POOL_ATTR_MAX,
};

struct hisp310_mem_pool {
	void *ap_va;
	unsigned int prot;
	unsigned int ion_iova;
	unsigned int r8_iova;
	size_t size;
	size_t align_size;
	int active;
	unsigned int security_isp_mode;

	struct sg_table *sgt;
	unsigned int shared_fd;
	unsigned int is_ap_cached;
};

struct isp_mem {
	int active;
	struct dma_buf *dmabuf;
};

/*
 * @brief the instance to talk to hisp driver
 * When Histar ISP is probed, this sturcture will be initialized,
 * the object is used to notify hisp driver when needed.
 * @end
 */
typedef struct _tag_hisp310 {
	hisp_intf_t intf;
	hisp_notify_intf_t *notify;
	char const *name;
	atomic_t opened;
	struct platform_device *pdev; /* by used to get dts node */
	hisp_dt_data_t dt;
	struct hisp310_mem_pool mem_pool[MEM_POOL_ATTR_MAX];
	struct isp_mem mem;
} hisp310_t;

struct rpmsg_service_info {
	struct rpmsg_hisp310_service *hisi_isp_serv;
	struct completion isp_comp;
	int isp_minor;
};

extern void a7_mmu_unmap(unsigned int va, unsigned int size);

/* Store the only rpmsg_hisp310_service pointer to local static rpmsg_local */
static struct rpmsg_service_info rpmsg_local;
static bool remote_processor_up = false;
static int g_hisp_ref = 0;

#define i_2_hi(i) container_of(i, hisp310_t, intf)

static void hisp310_notify_rpmsg_cb(void);
char const *hisp310_get_name(hisp_intf_t *i);
static int hisp310_config(hisp_intf_t *i, void *cfg);

static int hisp310_power_on(hisp_intf_t *i);
static int hisp310_power_off(hisp_intf_t *i);

static int hisp310_open(hisp_intf_t *i);
static int hisp310_close(hisp_intf_t *i);
static int hisp310_send_rpmsg(hisp_intf_t *i, hisp_msg_t *m, size_t len);
static int hisp310_recv_rpmsg(hisp_intf_t *i,
	hisp_msg_t *user_addr, size_t len);

static int hisp310_set_sec_fw_buffer(struct hisp_cfg_data *cfg);
static int hisp310_release_sec_fw_buffer(void);

#ifdef CONFIG_HISI_DEBUG_FS
static void hisp310_set_ddrfreq(int ddr_bandwidth);
static void hisp310_release_ddrfreq(void);
static void hisp310_update_ddrfreq(unsigned int ddr_bandwidth);
#endif

void hisp310_init_timestamp(void);
void hisp310_destroy_timestamp(void);
void hisp310_set_timestamp(unsigned int *timestampH, unsigned int *timestampL);
void hisp310_handle_msg(hisp_msg_t *msg);

void hisp310_init_timestamp(void)
{
	s_timestamp_state = TIMESTAMP_INTIAL;
	s_system_counter = arch_counter_get_cntvct();
	s_system_couter_rate = arch_timer_get_rate();
	do_gettimeofday(&s_timeval);

	cam_debug("%s state=%u system_counter=%llu rate=%u"
		" time_second=%ld time_usecond=%ld size=%lu",
		__FUNCTION__,
		(unsigned int)s_timestamp_state,
		s_system_counter,
		s_system_couter_rate,
		s_timeval.tv_sec,
		s_timeval.tv_usec,
		sizeof(s_timeval) / sizeof(u32));
}

void hisp310_destroy_timestamp(void)
{
	s_timestamp_state = TIMESTAMP_UNINTIAL;
	s_system_counter = 0;
	s_system_couter_rate = 0;
	memset_s(&s_timeval, sizeof(s_timeval), 0x00, sizeof(s_timeval));
}

/* Function declaration */
/*
 * |-----pow-on------->||<----  fw-SOF ---->|
 * timeval(got) ----------------->fw_timeval=?
 * system_counter(got)----------------->fw_sys_counter(got)
 *
 * fw_timeval = timeval + (fw_sys_counter - system_counter)
 *
 * With a base position(<timeval, system_counter>, we get it at same time),
 * we can calculate fw_timeval with fw syscounter
 * and deliver it to hal. Hal then gets second and microsecond
 */
void hisp310_set_timestamp(unsigned int *timestamp_high, unsigned int *timestamp_low)
{
#define MICROSECOND_PER_SECOND 1000000
	u64 fw_micro_second = 0;
	u64 fw_sys_counter = 0;
	u64 micro_second = 0;

	if (TIMESTAMP_UNINTIAL == s_timestamp_state) {
		cam_err("%s wouldn't enter this branch", __FUNCTION__);
		hisp310_init_timestamp();
	}

	if (timestamp_high == NULL || timestamp_low == NULL) {
		cam_err("%s timestamp_high or timestamp_low is null", __FUNCTION__);
		return;
	}

	cam_debug("%s ack_high:0x%x ack_low:0x%x", __FUNCTION__,
		*timestamp_high, *timestamp_low);

	if (*timestamp_high == 0 && *timestamp_low == 0)
		return;

	fw_sys_counter =
		((u64)(*timestamp_high) << 32) | /* 32 for Bit operations */
		(u64)(*timestamp_low);
	micro_second = (fw_sys_counter - s_system_counter) *
		MICROSECOND_PER_SECOND / s_system_couter_rate;

	/* chang nano second to micro second */
	fw_micro_second =
		(micro_second / MICROSECOND_PER_SECOND +
		s_timeval.tv_sec) * MICROSECOND_PER_SECOND +
		((micro_second % MICROSECOND_PER_SECOND) + s_timeval.tv_usec);
	/* 32 for Bit operations */
	*timestamp_high = (u32)(fw_micro_second >> 32 & 0xFFFFFFFF);
	*timestamp_low = (u32)(fw_micro_second & 0xFFFFFFFF);

	cam_debug("%s h:0x%x l:0x%x", __FUNCTION__, *timestamp_high, *timestamp_low);
}

void hisp310_handle_msg(hisp_msg_t *msg)
{
	if (msg == NULL)
		return;
	switch (msg->api_name) {
	case BATCH_REQUEST_RESPONSE:
		msg->u.ack_batch_request.system_couter_rate =
			s_system_couter_rate;
		cam_info("%s batch h:0x%x l:0x%x, rate %d",
			__FUNCTION__,
			msg->u.ack_batch_request.timestampH,
			msg->u.ack_batch_request.timestampL,
			msg->u.ack_batch_request.system_couter_rate);
		hisp310_set_timestamp(&(msg->u.ack_batch_request.timestampH),
			&(msg->u.ack_batch_request.timestampL));
		break;
	case REQUEST_RESPONSE:
		hisp310_set_timestamp(&(msg->u.ack_request.timestampH),
			&(msg->u.ack_request.timestampL));
		break;
	case MSG_EVENT_SENT:
		hisp310_set_timestamp(&(msg->u.event_sent.timestampH),
			&(msg->u.event_sent.timestampL));
		break;
	default:
		break;
	}
}

static hisp_vtbl_t s_vtbl_hisp310 = {
	.get_name = hisp310_get_name,
	.config = hisp310_config,
	.power_on = hisp310_power_on,
	.power_off = hisp310_power_off,
	.send_rpmsg = hisp310_send_rpmsg,
	.recv_rpmsg = hisp310_recv_rpmsg,
	.open = hisp310_open,
	.close = hisp310_close,
};

static hisp310_t s_hisp310 = {
	.intf = {.vtbl = &s_vtbl_hisp310,},
	.name = "hisp310",
};

static void hisp310_notify_rpmsg_cb(void)
{
	hisp_event_t isp_ev;
	isp_ev.kind = HISP_RPMSG_CB;
	hisp_notify_intf_rpmsg_cb(s_hisp310.notify, &isp_ev);
}


/* Function declaration */
/**********************************************
 * ave the rpmsg from isp to locally skb queue.
 * Only called by hisp310_rpmsg_ept_cb when api_name
 * is NOT POWER_REQ, will notify user space through HISP
 *********************************************/
static void hisp310_save_rpmsg_data(const void *data, int len)
{
	struct rpmsg_hisp310_service *hisi_serv = rpmsg_local.hisi_isp_serv;
	struct sk_buff *skb = NULL;
	char *skbdata = NULL;

	if (hisi_serv == NULL) {
		cam_err("%s: hisi_serv is NULL", __FUNCTION__);
		return;
	}
	hisp_assert(data != NULL);
	if (data == NULL)
		return;
	hisp_assert(len > 0);

	skb = alloc_skb(len, GFP_KERNEL);
	if (skb == NULL) {
		cam_err("%s() %d failed: alloc_skb len is %u", __FUNCTION__,
			__LINE__, len);
		return;
	}

	skbdata = skb_put(skb, len);
	if(memcpy_s(skbdata, len, data, len) != EOK)
		cam_err("%s: memcpy_s fail", __FUNCTION__);

	/* add skb to skb queue */
	mutex_lock(&hisi_serv->recv_lock);
	skb_queue_tail(&hisi_serv->queue, skb);
	mutex_unlock(&hisi_serv->recv_lock);

	wake_up_interruptible(&hisi_serv->readq);
	hisp310_notify_rpmsg_cb();
}

/* Function declaration */
/**********************************************
 * Power up CSI/DPHY/sensor according to isp req
 * Only called by hisp310_rpmsg_ept_cb when api_name
 * is POWER_REQ, and will send a POWER_RSP to isp
 * after power request done.
 *********************************************/
static int hisp310_rpmsg_ept_cb(struct rpmsg_device *rpdev,
	void *data, int len, void *priv, u32 src)
{
	struct rpmsg_hisp310_service *hisi_serv = rpmsg_local.hisi_isp_serv;
	hisp_msg_t *msg = NULL;
	struct rpmsg_hdr *rpmsg_msg = NULL;

	hisp_recvin((void*)data);
	if (hisi_serv == NULL) {
		cam_err("func %s: hisi_serv is NULL", __FUNCTION__);
		return -EINVAL;
	}
	if (data == NULL) {
		cam_err("func %s: data is NULL", __FUNCTION__);
		return -EINVAL;
	}

	hisp_assert(len > 0);

	if (hisi_serv->state != RPMSG_CONNECTED) {
		hisp_assert(hisi_serv->state == RPMSG_UNCONNECTED);
		rpmsg_msg = container_of(data, struct rpmsg_hdr, data);
		cam_info("msg src.%u, msg dst.%u", rpmsg_msg->src,
			rpmsg_msg->dst);

		/* add instance dst and modify the instance state */
		hisi_serv->dst = rpmsg_msg->src;
		hisi_serv->state = RPMSG_CONNECTED;
	}

	msg = (hisp_msg_t*)(data);
	/* save the data and wait for hisp310_recv_rpmsg to get the data */
	hisp_recvx(data);
	hisp310_save_rpmsg_data(data, len);
	return 0;
}

char const *hisp310_get_name(hisp_intf_t *i)
{
	hisp310_t *hi = NULL;
	hisp_assert(i != NULL);
	hi = i_2_hi(i);
	if (hi == NULL) {
		cam_err("func %s: hi is NULL", __FUNCTION__);
		return NULL;
	}
	return hi->name;
}

static int buffer_is_invalid(int share_fd,
	unsigned int req_addr, unsigned int req_size)
{
	int ret = 0;
	struct iommu_format fmt = {0};

	ret = hicam_buf_map_iommu(share_fd, &fmt);
	if (ret < 0) {
		cam_err("%s: fail to map iommu", __FUNCTION__);
		return ret;
	}

	if (req_addr != fmt.iova || req_size > fmt.size) {
		cam_err("%s: req_iova:%#x, req_size:%u",
			__FUNCTION__, req_addr, req_size);
		cam_err("%s:real_iova:%#llx, real_size:%llu",
			__FUNCTION__, fmt.iova, fmt.size);
		ret = -ERANGE;
	}
	hicam_buf_unmap_iommu(share_fd, &fmt);

	return ret;
}

static int find_suitable_mem_pool(struct hisp_cfg_data *pcfg)
{
	int ipool;
	if (pcfg->param.type == MAP_TYPE_RAW2YUV) {
		ipool = MEM_POOL_ATTR_READ_WRITE_CACHE_OFF_LINE;
	} else if (pcfg->param.type == MAP_TYPE_STATIC_ISP_SEC) {
		ipool = MEM_POOL_ATTR_READ_WRITE_ISP_SECURITY;
	} else {
		for (ipool = 0; ipool < MEM_POOL_ATTR_MAX; ipool++) {
			if (s_hisp310.mem_pool[ipool].prot ==
				pcfg->param.prot) {
				break;
			}
		}
		if (ipool >= MEM_POOL_ATTR_MAX) {
			cam_err("func %s: no pool hit for prot:%d",
				__FUNCTION__, pcfg->param.prot);
			return -EINVAL;
		}
	}
	return ipool;
}

static int hisp310_init_r8isp_memory_pool(void *cfg)
{
	int ipool;
	uint32_t r8va;
	struct hisp_cfg_data *pcfg = NULL;
	struct sg_table *sgt = NULL;
	enum maptype enm_map_type;

	if (cfg == NULL) {
		cam_err("%s: cfg is NULL", __FUNCTION__);
		return -EINVAL;
	}

	pcfg = (struct hisp_cfg_data*)cfg;
	cam_info("%s: pool cfg vaddr=0x%pK, iova=0x%x, size=0x%x", __FUNCTION__,
		pcfg->param.vaddr, pcfg->param.iova, pcfg->param.size);
	cam_info("%s: type=%d, prot=0x%x align=0x%zd sec=0x%x", __FUNCTION__,
		pcfg->param.type, pcfg->param.prot,
		pcfg->param.pool_align_size, pcfg->param.security_isp_mode);

	if (!pcfg->param.security_isp_mode &&
		buffer_is_invalid(pcfg->param.sharedFd,
			pcfg->param.iova, pcfg->param.size)) {
		cam_err("check buffer fail");
		return -EINVAL;
	}

	ipool = find_suitable_mem_pool(pcfg);
	if (ipool < 0)
		return ipool;

	if (ipool == MEM_POOL_ATTR_READ_WRITE_CACHE_OFF_LINE)
		enm_map_type = MAP_TYPE_RAW2YUV;
	else
		enm_map_type = pcfg->param.security_isp_mode ?
			MAP_TYPE_STATIC_SEC : MAP_TYPE_DYNAMIC;
	/* take care of putting sgtable. */
	sgt = hicam_buf_get_sgtable(pcfg->param.sharedFd);
	if (IS_ERR(sgt)) {
		cam_err("%s: fail to get sgtable", __FUNCTION__);
		return -ENOENT;
	}

	mutex_lock(&hisi_rpmsg_service_mutex);
	r8va = hisp_mem_map_setup(sgt->sgl, pcfg->param.iova, pcfg->param.size,
		pcfg->param.prot, (unsigned int)ipool, enm_map_type,
		(unsigned int)(pcfg->param.pool_align_size));
	if (!r8va) {
		cam_err("%s: hisp_mem_map_setup failed", __FUNCTION__);
		mutex_unlock(&hisi_rpmsg_service_mutex);
		hicam_buf_put_sgtable(sgt);
		return -ENOMEM;
	}
	/* hold sg_table things, release at deinit. */
	s_hisp310.mem_pool[ipool].sgt = sgt;
	s_hisp310.mem_pool[ipool].r8_iova = r8va;
	s_hisp310.mem_pool[ipool].ap_va = pcfg->param.vaddr;
	s_hisp310.mem_pool[ipool].ion_iova = pcfg->param.iova;
	s_hisp310.mem_pool[ipool].size = pcfg->param.size;
	s_hisp310.mem_pool[ipool].align_size = pcfg->param.pool_align_size;
	s_hisp310.mem_pool[ipool].security_isp_mode =
		pcfg->param.security_isp_mode;
	s_hisp310.mem_pool[ipool].shared_fd = pcfg->param.sharedFd;
	s_hisp310.mem_pool[ipool].is_ap_cached = pcfg->param.isApCached;

	/*
	 * ion iova isn't equal r8 iova, security or unsecurity, align etc
	 * return r8 iova to daemon, and send to r8 later
	 */
	pcfg->param.iova = r8va;
	s_hisp310.mem_pool[ipool].active = 1;

	cam_info("%s: r8_iova_pool_base=0x%x",
		__FUNCTION__, s_hisp310.mem_pool[ipool].r8_iova);
	mutex_unlock(&hisi_rpmsg_service_mutex);
	return 0;
}

static int hisp310_deinit_r8isp_memory_pool(void *cfg)
{
	int ipool;
	if (cfg == NULL) {
		cam_err("func %s: cfg is NULL", __FUNCTION__);
		return -EINVAL;
	}

	ipool = find_suitable_mem_pool(cfg);
	if (ipool < 0)
		return ipool;

	mutex_lock(&hisi_rpmsg_service_mutex);
	if (s_hisp310.mem_pool[ipool].active) {
		s_hisp310.mem_pool[ipool].active = 0;
		hisp_mem_pool_destroy((unsigned int)ipool);
		/* release sg_table things. */
		hicam_buf_put_sgtable(s_hisp310.mem_pool[ipool].sgt);
	}

	memset_s(&(s_hisp310.mem_pool[ipool]),
		sizeof(struct hisp310_mem_pool),
		0,
		sizeof(struct hisp310_mem_pool));
	mutex_unlock(&hisi_rpmsg_service_mutex);

	return 0;
}

/*
 * handle daemon carsh
 * miss ispmanager poweroff
 * miss memory pool deinit
 */
static int hisp310_deinit_r8isp_memory_pool_force(void)
{
	int ipool = 0;
	cam_warn("func %s", __FUNCTION__);

	mutex_lock(&hisi_rpmsg_service_mutex);
	for (ipool = 0; ipool < MEM_POOL_ATTR_MAX; ipool++) {
		if (s_hisp310.mem_pool[ipool].active) {
			cam_warn("%s: force deiniting pool:%d",
				__FUNCTION__, ipool);
			s_hisp310.mem_pool[ipool].active = 0;
			hisp_mem_pool_destroy((unsigned int)ipool);
			hicam_buf_put_sgtable(s_hisp310.mem_pool[ipool].sgt);
		}
		memset_s(&(s_hisp310.mem_pool[ipool]),
			sizeof(struct hisp310_mem_pool),
			0,
			sizeof(struct hisp310_mem_pool));
	}

	mutex_unlock(&hisi_rpmsg_service_mutex);
	return 0;
}

static int hisp310_alloc_r8isp_addr(void *cfg)
{
	int ipool = 0;
	unsigned int r8_iova = 0;
	size_t offset = 0;
	struct hisp_cfg_data *pcfg = NULL;
	int rc = 0;
	bool secure_mode = false;

	if (cfg == NULL) {
		cam_err("func %s: cfg is NULL", __FUNCTION__);
		return -1;
	}

	pcfg = (struct hisp_cfg_data*)cfg;
	/*
	 * handle static memory,
	 * just return r8 reserved iova address == map only
	 */
	if (pcfg->param.type == MAP_TYPE_STATIC) {
#ifndef CONFIG_HISI_CAMERA_ISP_SECURE
		return -ENODEV;
#else
		cam_debug("func %s static", __FUNCTION__);
		mutex_lock(&hisi_rpmsg_service_mutex);
		pcfg->param.iova = a7_mmu_map(NULL, pcfg->param.size,
			pcfg->param.prot, pcfg->param.type);
		mutex_unlock(&hisi_rpmsg_service_mutex);
		return 0;
#endif
	}

	/* handle dynamic carveout alloc */
	if (pcfg->param.type == MAP_TYPE_DYNAMIC_CARVEOUT) {
		cam_debug("func %s dynamic carveout", __FUNCTION__);
		mutex_lock(&hisi_rpmsg_service_mutex);
		pcfg->param.iova =
			hisp_mem_pool_alloc_carveout(pcfg->param.size,
			pcfg->param.type);
		mutex_unlock(&hisi_rpmsg_service_mutex);
		return 0;
	}

	for (ipool = 0; ipool < MEM_POOL_ATTR_MAX; ipool++) {
		if (s_hisp310.mem_pool[ipool].security_isp_mode) {
			secure_mode = true;
			break;
		}
	}

	ipool = find_suitable_mem_pool(pcfg);
	if (ipool < 0)
		return ipool;

	mutex_lock(&hisi_rpmsg_service_mutex);
	r8_iova = (unsigned int)hisp_mem_pool_alloc_iova(pcfg->param.size,
		(unsigned int)ipool);
	if (!r8_iova) {
		cam_err("func %s: hisp_mem_pool_alloc_iova error", __FUNCTION__);
		rc = -ENOMEM;
		goto alloc_err;
	}

	/*
	 * offset calculator
	 * security mode, pool base is r8_iova, is security address, not align
	 * normal mode, pool base is ion_iova, is normal address, align by isp.
	 */
	if (pcfg->param.type == MAP_TYPE_RAW2YUV) {
		offset = r8_iova - s_hisp310.mem_pool[ipool].r8_iova;
	} else {
		if (secure_mode)
			offset = r8_iova - s_hisp310.mem_pool[ipool].r8_iova;
		else
			offset = r8_iova - s_hisp310.mem_pool[ipool].ion_iova;
	}

	if (offset > s_hisp310.mem_pool[ipool].size) {
		cam_err("func %s: r8_iova invalid", __FUNCTION__);
		goto check_err;
	}

	pcfg->param.vaddr =
		(void*)(((unsigned char*)s_hisp310.mem_pool[ipool].ap_va) +
		offset);
	pcfg->param.iova = r8_iova;
	pcfg->param.offset_in_pool = offset;
	pcfg->param.sharedFd = s_hisp310.mem_pool[ipool].shared_fd;
	pcfg->param.isApCached = s_hisp310.mem_pool[ipool].is_ap_cached;
	mutex_unlock(&hisi_rpmsg_service_mutex);
	return rc;

check_err:
	rc = (int)hisp_mem_pool_free_iova((unsigned int)ipool,
		r8_iova, pcfg->param.size);
	if (rc)
		cam_err("func %s: hisp_mem_pool_free_iova error", __FUNCTION__);
alloc_err:
	mutex_unlock(&hisi_rpmsg_service_mutex);
	return rc;
}

static int hisp310_free_r8isp_addr(void *cfg)
{
	int rc = 0;
	int ipool = 0;
	struct hisp_cfg_data *pcfg = NULL;

	if (cfg == NULL) {
		cam_err("func %s: cfg is NULL", __FUNCTION__);
		return -EINVAL;
	}

	pcfg = (struct hisp_cfg_data*)cfg;

	/* handle static memory, unmap only */
	if (pcfg->param.type == MAP_TYPE_STATIC) {
#ifndef CONFIG_HISI_CAMERA_ISP_SECURE
		return -ENODEV;
#else
		cam_debug("func %s static", __FUNCTION__);
		mutex_lock(&hisi_rpmsg_service_mutex);
		a7_mmu_unmap(pcfg->param.iova, pcfg->param.size);
		mutex_unlock(&hisi_rpmsg_service_mutex);
		return 0;
#endif
	}

	/* handle dynamic carveout free */
	if (pcfg->param.type == MAP_TYPE_DYNAMIC_CARVEOUT) {
		cam_debug("func %s dynamic carveout", __FUNCTION__);
		mutex_lock(&hisi_rpmsg_service_mutex);
		rc = hisp_mem_pool_free_carveout(pcfg->param.iova,
			pcfg->param.size);
		if (rc)
			cam_err("func %s: hisp_mem_pool_free_carveout error",
				__FUNCTION__);
		mutex_unlock(&hisi_rpmsg_service_mutex);
		return rc;
	}

	ipool = find_suitable_mem_pool(pcfg);
	if (ipool < 0)
		return ipool;

	mutex_lock(&hisi_rpmsg_service_mutex);
	rc = (int)hisp_mem_pool_free_iova((unsigned int)ipool,
		pcfg->param.iova, pcfg->param.size);
	if (rc)
		cam_err("func %s: hisp_mem_pool_free_iova error", __FUNCTION__);
	mutex_unlock(&hisi_rpmsg_service_mutex);
	return rc;
}

static int hisp310_mem_pool_pre_init(void)
{
	int ipool = 0;
	int prot = 0;

	for (ipool = 0; ipool < MEM_POOL_ATTR_MAX; ipool++) {
		memset_s(&(s_hisp310.mem_pool[ipool]),
			sizeof(struct hisp310_mem_pool),
			0,
			sizeof(struct hisp310_mem_pool));

		switch (ipool) {
		case MEM_POOL_ATTR_READ_WRITE_CACHE:
		case MEM_POOL_ATTR_READ_WRITE_CACHE_OFF_LINE:
			prot = IOMMU_READ | IOMMU_WRITE | IOMMU_CACHE;
			break;

		case MEM_POOL_ATTR_READ_WRITE_SECURITY:
		case MEM_POOL_ATTR_READ_WRITE_ISP_SECURITY:
			prot = IOMMU_READ | IOMMU_WRITE | IOMMU_CACHE |
				IOMMU_SEC;
			break;

		default:
			prot = -1;
			break;
		}

		cam_debug("%s  ipool %d prot 0x%x", __FUNCTION__, ipool, prot);

		if (prot < 0) {
			cam_err("%s unkown ipool %d prot 0x%x",
				__FUNCTION__, ipool, prot);
			return -EINVAL;
		}
		s_hisp310.mem_pool[ipool].prot = (unsigned int)prot;
	}
	return 0;
}

static int hisp310_mem_pool_later_deinit(void)
{
	int ipool = 0;
	cam_debug("%s", __FUNCTION__);

	for (ipool = 0; ipool < MEM_POOL_ATTR_MAX; ipool++) {
		if (ipool == MEM_POOL_ATTR_READ_WRITE_CACHE_OFF_LINE)
			continue;
		if (s_hisp310.mem_pool[ipool].active) {
			cam_warn("%s: force deiniting pool:%d",
				__FUNCTION__, ipool);
			s_hisp310.mem_pool[ipool].active = 0;
			hisp_mem_pool_destroy((unsigned int)ipool);
			hicam_buf_put_sgtable(s_hisp310.mem_pool[ipool].sgt);
		}
		memset_s(&s_hisp310.mem_pool[ipool],
			sizeof(struct hisp310_mem_pool),
			0,
			sizeof(struct hisp310_mem_pool));
	}
	return 0;
}

static int hisp310_set_clk_rate(int clk_level)
{
	int rc = 0;
	int rc_func = 0;
	int rc_func2 = 0;
	int rc_func3 = 0;
	int rc_func4 = 0;
	int rc_mid_3 = 0;
	int rc_mid_4 = 0;

	switch (clk_level) {
	case ISP_CLK_LEVEL_TURBO:
		rc_func = hisp_set_clk_rate(ISPFUNC_CLK,
			ISP_TURBO_ISPFUNC_CLK_RATE);
		rc_func2 = hisp_set_clk_rate(ISPFUNC2_CLK,
			ISP_TURBO_ISPFUNC2_CLK_RATE);
		rc_func3 = hisp_set_clk_rate(ISPFUNC3_CLK,
			ISP_TURBO_ISPFUNC3_CLK_RATE);
		rc_func4 = hisp_set_clk_rate(ISPFUNC4_CLK,
			ISP_TURBO_ISPFUNC4_CLK_RATE);
		break;
	case ISP_CLK_LEVEL_NORMAL:
		rc_func = hisp_set_clk_rate(ISPFUNC_CLK,
			ISP_NORMAL_ISPFUNC_CLK_RATE);
		rc_func2 = hisp_set_clk_rate(ISPFUNC2_CLK,
			ISP_NORMAL_ISPFUNC2_CLK_RATE);
		rc_func3 = hisp_set_clk_rate(ISPFUNC3_CLK,
			ISP_NORMAL_ISPFUNC3_CLK_RATE);
		rc_func4 = hisp_set_clk_rate(ISPFUNC4_CLK,
			ISP_NORMAL_ISPFUNC4_CLK_RATE);
		break;
	case ISP_CLK_LEVEL_LOWPOWER:
		rc_func = hisp_set_clk_rate(ISPFUNC_CLK,
			ISP_LOWPOWER_ISPFUNC_CLK_RATE);
		rc_func2 = hisp_set_clk_rate(ISPFUNC2_CLK,
			ISP_LOWPOWER_ISPFUNC2_CLK_RATE);
		rc_func3 = hisp_set_clk_rate(ISPFUNC3_CLK,
			ISP_LOWPOWER_ISPFUNC3_CLK_RATE);
		rc_func4 = hisp_set_clk_rate(ISPFUNC4_CLK,
			ISP_LOWPOWER_ISPFUNC4_CLK_RATE);
		break;
	case ISP_CLK_LEVEL_ULTRALOW:
		rc_func = hisp_set_clk_rate(ISPFUNC_CLK,
			ISP_ULTRALOW_ISPFUNC_CLK_RATE);
		rc_func2 = hisp_set_clk_rate(ISPFUNC2_CLK,
			ISP_ULTRALOW_ISPFUNC2_CLK_RATE);
		/* from NORMAL/LOWER to ULTRALOW,
		 * ISP_FUNC3/FUNC4 will pass 541/406Mhz(abnormal Freq),
		 * the correct path is
		 * MORMAL/LOWER -->MIDDLE(172MHz) -->ULTRALOW
		 */
		if ((clk_level_pre == ISP_CLK_LEVEL_NORMAL) ||
			(clk_level_pre == ISP_CLK_LEVEL_LOWPOWER)) {
			rc_mid_3 = hisp_set_clk_rate(ISPFUNC3_CLK,
				ISP_MIDDLE_ISPFUNC3_CLK_RATE);
			rc_mid_4 = hisp_set_clk_rate(ISPFUNC4_CLK,
				ISP_MIDDLE_ISPFUNC4_CLK_RATE);
		}
		rc_func3 = hisp_set_clk_rate(ISPFUNC3_CLK,
			ISP_ULTRALOW_ISPFUNC3_CLK_RATE);
		rc_func4 = hisp_set_clk_rate(ISPFUNC4_CLK,
			ISP_ULTRALOW_ISPFUNC4_CLK_RATE);
		break;
	default:
		cam_err("case clk_level[%d] err", clk_level);
		break;
	}
	clk_level_pre = clk_level;

	if (rc_func < 0 || rc_func2 < 0 || rc_func3 < 0 ||
		rc_func4 < 0 || rc_mid_3 < 0 || rc_mid_4 < 0) {
		cam_err("%s: set clk fail, rc:%d, %d, %d, %d, %d ,%d",
			__FUNCTION__, rc_func, rc_func2,
			rc_func3, rc_func4, rc_mid_3, rc_mid_4);
		rc = -EFAULT;
	}
	return rc;
}


static int hisp310_phy_csi_connect(void *pdata)
{
	int rc = 0;
	struct msg_req_connect_camera_t *req_connect = NULL;

	if (pdata == NULL) {
		cam_err("%s: data is NULL", __FUNCTION__);
		return -1;
	}
	req_connect = (struct msg_req_connect_camera_t*)(pdata);

	rc = hisp_phy_csi_connect(
		(struct hisp_phy_info_t*)&(req_connect->phy_info),
		req_connect->csi_index);
	if (rc != 0)
		cam_err("%s: phy csi connect fail:%d", __FUNCTION__, rc);
	return rc;
}

static int hisp310_config_power_on(hisp_intf_t *i, struct hisp_cfg_data *pcfg)
{
	int rc = 0;

	mutex_lock(&hisp_power_lock_mutex);
	if (!remote_processor_up) {
		if (pcfg->isSecure == 0)
			hisp_rproc_case_set(NONSEC_CASE);
		else if (pcfg->isSecure == 1)
			hisp_rproc_case_set(SEC_CASE);
		else
			cam_info("%s invalid mode", __FUNCTION__);
		cam_notice("%s power on the hisp310", __FUNCTION__);
		rc = hisp310_power_on(i);
	} else {
		cam_warn("%s hisp310 is still on power-on state, power off it",
			__FUNCTION__);

		rc = hisp310_power_off(i);
		if (rc != 0) {
			mutex_unlock(&hisp_power_lock_mutex);
			return rc;
		}

		hisp310_deinit_r8isp_memory_pool_force();

		cam_warn("%s begin to power on the hisp310", __FUNCTION__);
		rc = hisp310_power_on(i);
	}
	mutex_unlock(&hisp_power_lock_mutex);
	return rc;
}

static int hisp310_config_power_off(hisp_intf_t *i)
{
	int rc = 0;
	mutex_lock(&hisp_power_lock_mutex);
	if (remote_processor_up) {
		cam_notice("%s power off the hisp310", __FUNCTION__);
		rc = hisp310_power_off(i);
	}
	mutex_unlock(&hisp_power_lock_mutex);

	return rc;
}

static int hisp310_config(hisp_intf_t *i,
	void *cfg) /* todo modify switch case */
{
	int rc = 0;
	hisp310_t *hi = NULL;
	struct hisp_cfg_data *pcfg = NULL;

	hisp_assert(i != NULL);
	if (cfg == NULL) {
		cam_err("func %s: cfg is NULL", __FUNCTION__);
		return -1;
	}
	pcfg = (struct hisp_cfg_data*)cfg;
	hi = i_2_hi(i);
	hisp_assert(hi != NULL);

	switch (pcfg->cfgtype) {
	case HISP_CONFIG_POWER_ON:
		rc = hisp310_config_power_on(i, pcfg);
		break;
	case HISP_CONFIG_POWER_OFF:
		rc = hisp310_config_power_off(i);
		break;

	case HISP_CONFIG_INIT_MEMORY_POOL:
		rc = hisp310_init_r8isp_memory_pool(cfg);
		break;

	case HISP_CONFIG_DEINIT_MEMORY_POOL:
		rc = hisp310_deinit_r8isp_memory_pool(cfg);
		break;

	case HISP_CONFIG_ALLOC_MEM:
		rc = hisp310_alloc_r8isp_addr(cfg);
		break;

	case HISP_CONFIG_FREE_MEM:
		rc = hisp310_free_r8isp_addr(cfg);
		break;
	/* Func->FE, Func2->SRT, Func3->CRAW/CBE, func4->VRAW/VBE */
	case HISP_CONFIG_ISP_TURBO:
		cam_debug("%s HISP_CONFIG_ISP_TURBO", __FUNCTION__);
		rc = hisp310_set_clk_rate(ISP_CLK_LEVEL_TURBO);
		break;
	case HISP_CONFIG_ISP_NORMAL:
		cam_debug("%s HISP_CONFIG_ISP_NORMAL", __FUNCTION__);
		rc = hisp310_set_clk_rate(ISP_CLK_LEVEL_NORMAL);
		break;
	case HISP_CONFIG_ISP_LOWPOWER:
		cam_debug("%s HISP_CONFIG_ISP_LOWPOWER", __FUNCTION__);
		rc = hisp310_set_clk_rate(ISP_CLK_LEVEL_LOWPOWER);
		break;
	case HISP_CONFIG_ISP_ULTRALOW:
		cam_debug("%s HISP_CONFIG_ISP_ULTRALOW", __FUNCTION__);
		rc = hisp310_set_clk_rate(ISP_CLK_LEVEL_ULTRALOW);
		break;
	case HISP_CONFIG_R8_TURBO:
		cam_debug("%s HISP_CONFIG_R8_TURBO", __FUNCTION__);
		rc = hisp_set_clk_rate(ISPCPU_CLK, R8_TURBO_ISPCPU_CLK_RATE);
		break;
	case HISP_CONFIG_R8_NORMAL:
		cam_debug("%s HISP_CONFIG_R8_NORMAL", __FUNCTION__);
		rc = hisp_set_clk_rate(ISPCPU_CLK, R8_NORMAL_ISPCPU_CLK_RATE);
		break;
	case HISP_CONFIG_R8_LOWPOWER:
		cam_debug("%s HISP_CONFIG_R8_LOWPOWER", __FUNCTION__);
		rc = hisp_set_clk_rate(ISPCPU_CLK, R8_LOWPOWER_ISPCPU_CLK_RATE);
		break;
	case HISP_CONFIG_R8_ULTRALOW:
		cam_debug("%s HISP_CONFIG_R8_ULTRALOW", __FUNCTION__);
		rc = hisp_set_clk_rate(ISPCPU_CLK, R8_ULTRALOW_ISPCPU_CLK_RATE);
		break;
	case HISP_CONFIG_PROC_TIMEOUT:
		cam_info("%s message_id.0x%x", __FUNCTION__, pcfg->cfgdata[0]);
		hisp_dump_rpmsg_with_id(pcfg->cfgdata[0]);
		break;
	case HISP_CONFIG_GET_SEC_ISPFW_SIZE:
		rc = hisp_secmem_size_get(&pcfg->buf_size);
		break;
	case HISP_CONFIG_SET_SEC_ISPFW_BUFFER:
		rc = hisp310_set_sec_fw_buffer(cfg);
		break;
	case HISP_CONFIG_RELEASE_SEC_ISPFW_BUFFER:
		rc = hisp310_release_sec_fw_buffer();
		break;
	case HISP_CONFIG_SET_MDC_BUFFER:
		rc = hisp_set_mdc_buffer(cfg);
		break;
	case HISP_CONFIG_RELEASE_MDC_BUFFER:
		rc = hisp_release_mdc_buffer();
		break;
	case HISP_CONFIG_PHY_CSI_CONNECT:
		rc = hisp310_phy_csi_connect((void*)(pcfg->cfgdata));
		break;
	default:
		cam_err("%s: unsupported cmd:%#x", __FUNCTION__, pcfg->cfgtype);
		break;
	}

	if (rc < 0)
		cam_err("%s: cmd:%#x fail, rc:%u",
			__FUNCTION__, pcfg->cfgtype, rc);
	return rc;
}

static int hisp310_open(hisp_intf_t *i)
{
	cam_info("%s hisp310 device open", __FUNCTION__);

	mutex_lock(&hisp_power_lock_mutex);
	g_hisp_ref++;
	mutex_unlock(&hisp_power_lock_mutex);
	return 0;
}

static int hisp310_close(hisp_intf_t *i)
{
	int rc = 0;
	cam_info("%s hisp310 device close", __FUNCTION__);
	mutex_lock(&hisp_power_lock_mutex);

	if (g_hisp_ref)
		g_hisp_ref--;

	if ((g_hisp_ref == 0) && remote_processor_up) {
		cam_warn("%s hisp310 is still on power-on state, power off it",
			__FUNCTION__);

		rc = hisp310_power_off(i);
		if (rc != 0)
			cam_err("failed to hisp310 power off");
		hisp310_deinit_r8isp_memory_pool_force();
	}

	if (g_hisp_ref == 0)
		hisp310_deinit_isp_mem();

	mutex_unlock(&hisp_power_lock_mutex);
	return rc;
}

void deal_power_on_fail_situation(bool hi_opened,
	bool rproc_enabled, hisp310_t *hi)
{
	if (hi_opened)
		atomic_dec(&hi->opened);

	if (rproc_enabled) {
		hisp_rproc_disable();
		rproc_set_sync_flag(true);
	}

	hisp310_mem_pool_later_deinit();
	remote_processor_up = false;

	mutex_unlock(&hisi_rpmsg_service_mutex);

	mutex_lock(&hisp_wake_lock_mutex);
	if (hisp_power_wakelock.active) {
		__pm_relax(&hisp_power_wakelock);
		cam_err("%s hisp power on failed, wake unlock", __FUNCTION__);
	}
	mutex_unlock(&hisp_wake_lock_mutex);
}

static int hisp310_power_on(hisp_intf_t *i)
{
	int rc = 0;
	bool rproc_enabled = false;
	bool hi_opened = false;
	hisp310_t *hi = NULL;
	unsigned long current_jiffies = jiffies;
	uint32_t timeout = hw_is_fpga_board() ?
		TIMEOUT_IS_FPGA_BOARD : TIMEOUT_IS_NOT_FPGA_BOARD;

	struct rpmsg_hisp310_service *hisi_serv = NULL;
	struct rpmsg_channel_info chinfo = {
		.src = RPMSG_ADDR_ANY,
	};
	if (i == NULL)
		return -1;
	hi = i_2_hi(i);

	cam_info("%s enter ... ", __FUNCTION__);

	mutex_lock(&hisp_wake_lock_mutex);
	if (!hisp_power_wakelock.active) {
		__pm_stay_awake(&hisp_power_wakelock);
		cam_info("%s hisp power on enter, wake lock", __FUNCTION__);
	}
	mutex_unlock(&hisp_wake_lock_mutex);

	mutex_lock(&hisi_rpmsg_service_mutex);
	if (!atomic_read((&hi->opened))) {
		if (!hw_is_fpga_board()) {
			if (!IS_ERR(hi->dt.pinctrl_default)) {
				rc = pinctrl_select_state(hi->dt.pinctrl,
					hi->dt.pinctrl_default);
				if (rc != 0) {
					deal_power_on_fail_situation(hi_opened,
						rproc_enabled, hi);
					return rc;
				}
			}
		}

		hisp_rpmsgrefs_reset();
		rc = hisp_rproc_enable();
		if (rc != 0) {
			deal_power_on_fail_situation(hi_opened,
				rproc_enabled, hi);
			return rc;
		}
		rproc_enabled = true;

		rc = wait_for_completion_timeout(&rpmsg_local.isp_comp,
			msecs_to_jiffies(timeout));
		if (rc == 0) {
			rc = -ETIME;
			hisp_boot_stat_dump();
			deal_power_on_fail_situation(hi_opened,
				rproc_enabled, hi);
			return rc;
		} else {
			cam_info("%s() %d after wait completion, rc = %d",
				__FUNCTION__, __LINE__, rc);
			rc = 0;
		}

		atomic_inc(&hi->opened);
		hi_opened = true;
	} else {
		cam_notice("%s isp has been opened", __FUNCTION__);
	}
	remote_processor_up = true;
	hisi_serv = rpmsg_local.hisi_isp_serv;
	if (hisi_serv == NULL) {
		rc = -ENODEV;
		deal_power_on_fail_situation(hi_opened, rproc_enabled, hi);
		return rc;
	}

	/* assign a new, unique, local address and associate instance with it */
#pragma GCC visibility push(default)
	hisi_serv->ept =
		rpmsg_create_ept(hisi_serv->rpdev,
			hisp310_rpmsg_ept_cb, hisi_serv, chinfo);
#pragma GCC visibility pop
	if (hisi_serv->ept == NULL) {
		hisi_serv->state = RPMSG_FAIL;
		rc = -ENOMEM;
		deal_power_on_fail_situation(hi_opened, rproc_enabled, hi);
		return rc;
	}
	cam_info("%s() %d hisi_serv->rpdev:src.%d, dst.%d",
		__FUNCTION__, __LINE__,
		hisi_serv->rpdev->src, hisi_serv->rpdev->dst);
	hisi_serv->state = RPMSG_CONNECTED;

	/* set the instance recv_count */
	hisi_serv->recv_count = HISP_SERV_FIRST_RECV;

	hisp310_init_timestamp();

	if (hisp310_mem_pool_pre_init()) {
		cam_err("failed to pre init mem pool ");
		rc = -ENOMEM;
		deal_power_on_fail_situation(hi_opened, rproc_enabled, hi);
		return rc;
	}

	mutex_unlock(&hisi_rpmsg_service_mutex);
	cam_info("%s exit ,power on time:%d... ", __FUNCTION__,
		jiffies_to_msecs(jiffies - current_jiffies));
	return rc;
}

static int hisp310_power_off(hisp_intf_t *i)
{
	int rc = 0;
	hisp310_t *hi = NULL;
	unsigned long current_jiffies = jiffies;
	struct rpmsg_hisp310_service *hisi_serv = NULL;
	if (i == NULL)
		return -1;
	hi = i_2_hi(i);

	cam_info("%s enter ... ", __FUNCTION__);

	/* check the remote processor boot flow */
	if (remote_processor_up == false) {
		rc = -EPERM;
		goto RET;
	}

	hisi_serv = rpmsg_local.hisi_isp_serv;
	if (hisi_serv == NULL) {
		rc = -ENODEV;
		goto RET;
	}

	if (hisi_serv->state == RPMSG_FAIL) {
		rc = -EFAULT;
		goto RET;
	}

	mutex_lock(&hisi_rpmsg_service_mutex);

	if (hisi_serv->ept == NULL) {
		rc = -ENODEV;
		goto UNLOCK_RET;
	}
	rpmsg_destroy_ept(hisi_serv->ept);
	hisi_serv->ept = NULL;

	hisi_serv->state = RPMSG_UNCONNECTED;
	hisi_serv->recv_count = HISP_SERV_FIRST_RECV;

	if (atomic_read((&hi->opened))) {
		hisp_phy_csi_disconnect();
		hisp_rproc_disable();
		if (!hw_is_fpga_board()) {
			if (!IS_ERR(hi->dt.pinctrl_idle)) {
				rc = pinctrl_select_state(hi->dt.pinctrl,
					hi->dt.pinctrl_idle);
				if (rc != 0) {
					// Empty.
				}
			}
		}

		remote_processor_up = false;
		atomic_dec(&hi->opened);
	} else {
		cam_notice("%s isp hasn't been opened", __FUNCTION__);
	}

	hisp310_destroy_timestamp();
UNLOCK_RET:
	hisp310_mem_pool_later_deinit();

	mutex_unlock(&hisi_rpmsg_service_mutex);
RET:
	cam_info("%s exit ,power 0ff time:%d... ", __FUNCTION__,
		jiffies_to_msecs(jiffies - current_jiffies));

	mutex_lock(&hisp_wake_lock_mutex);
	if (hisp_power_wakelock.active) {
		__pm_relax(&hisp_power_wakelock);
		cam_info("%s hisp power off exit, wake unlock", __FUNCTION__);
	}
	mutex_unlock(&hisp_wake_lock_mutex);
	return rc;
}

static void hisp310_rpmsg_remove(struct rpmsg_device *rpdev)
{
	struct rpmsg_hisp310_service *hisi_serv = dev_get_drvdata(&rpdev->dev);

	cam_info("%s enter ... ", __FUNCTION__);

	if (hisi_serv == NULL) {
		cam_err("%s: hisi_serv == NULL", __FUNCTION__);
		return;
	}

	mutex_destroy(&hisi_serv->send_lock);
	mutex_destroy(&hisi_serv->recv_lock);

	kfree(hisi_serv);
	rpmsg_local.hisi_isp_serv = NULL;
	cam_notice("rpmsg hisi driver is removed");
}

static int hisp310_rpmsg_driver_cb(struct rpmsg_device *rpdev,
	void *data, int len, void *priv, u32 src)
{
	cam_info("%s enter ... ", __FUNCTION__);
	cam_warn("%s() %d uhm, unexpected message", __FUNCTION__,
		__LINE__);

	print_hex_dump(KERN_DEBUG, __FUNCTION__,
		DUMP_PREFIX_NONE, 16, 1, /* 16 for lenth */
		data, len, true);
	return 0;
}

static int hisp310_send_rpmsg(hisp_intf_t *i, hisp_msg_t *from_user, size_t len)
{
	int rc = 0;
	hisp310_t *hi = NULL;
	struct rpmsg_hisp310_service *hisi_serv = NULL;
	hisp_msg_t *msg = from_user;
	hisp_assert(i != NULL);
	hisp_assert(from_user != NULL);
	hi = i_2_hi(i);
	cam_debug("%s enter. api_name(0x%x)", __FUNCTION__, msg->api_name);

	if (msg->message_id % HISP_MSG_LOG_MOD == 0)
		cam_info("%s: api_name:%#x, message_id:%#x", __FUNCTION__,
			msg->api_name, msg->message_id);

	hisi_serv = rpmsg_local.hisi_isp_serv;
	if (hisi_serv == NULL) {
		cam_err("%s() %d failed: hisi_serv does not exist",
			__FUNCTION__, __LINE__);
		rc = -ENODEV;
		goto RET;
	}

	if (hisi_serv->ept == NULL) {
		cam_err("%s() %d failed:hisi_serv->ept does not exist",
			__FUNCTION__, __LINE__);
		rc = -ENODEV;
		goto RET;
	}

	mutex_lock(&hisi_serv->send_lock);
	/* if the msg is the first msg, let's treat it special */
	if (hisi_serv->state != RPMSG_CONNECTED) {
		if (hisi_serv->rpdev == NULL) {
			cam_err("%s() %d failed:hisi_serv->rpdev does not exist",
				__FUNCTION__, __LINE__);
			rc = -ENODEV;
			goto UNLOCK_RET;
		}
		hisp_sendin(msg);
		rc = rpmsg_send_offchannel(hisi_serv->ept,
			hisi_serv->ept->addr,
			hisi_serv->rpdev->dst, (void*)msg,
			len);
		if (rc)
			cam_err("%s() %d failed: first rpmsg_send_offchannel ret is %d",
				__FUNCTION__, __LINE__, rc);
		goto UNLOCK_RET;
	}
	hisp_sendin(msg);

	rc = rpmsg_send_offchannel(hisi_serv->ept, hisi_serv->ept->addr,
		hisi_serv->dst, (void*)msg, len);
	if (rc) {
		cam_err("%s() %d failed: rpmsg_send_offchannel ret is %d",
			__FUNCTION__, __LINE__, rc);
		goto UNLOCK_RET;
	}
UNLOCK_RET:
	mutex_unlock(&hisi_serv->send_lock);
RET:
	return rc;
}

static int hisp310_recv_rpmsg(hisp_intf_t *i, hisp_msg_t *user_addr, size_t len)
{
	int rc = len;
	hisp310_t *hi = NULL;
	struct rpmsg_hisp310_service *hisi_serv = NULL;
	struct sk_buff *skb = NULL;
	hisp_msg_t *msg = NULL;
	hisp_assert(i != NULL);
	if (user_addr == NULL) {
		cam_err("func %s: user_addr is NULL", __FUNCTION__);
		return -1;
	}
	hi = i_2_hi(i);

	cam_debug("%s enter. ", __FUNCTION__);

	hisi_serv = rpmsg_local.hisi_isp_serv;
	if (hisi_serv == NULL) {
		cam_err("%s() %d failed: hisi_serv does not exist",
			__FUNCTION__, __LINE__);
		rc = -ENODEV;
		goto RET;
	}

	if (hisi_serv->recv_count == HISP_SERV_FIRST_RECV)
		hisi_serv->recv_count = HISP_SERV_NOT_FIRST_RECV;

	if (mutex_lock_interruptible(&hisi_serv->recv_lock)) {
		cam_err("%s() %d failed: mutex_lock_interruptible",
			__FUNCTION__, __LINE__);
		rc = -ERESTARTSYS;
		goto RET;
	}

	if (hisi_serv->state != RPMSG_CONNECTED) {
		cam_err("%s() %d hisi_serv->state != RPMSG_CONNECTED",
			__FUNCTION__, __LINE__);
		rc = -ENOTCONN;
		goto UNLOCK_RET;
	}

	/* nothing to read ? */
	/* check if skb_queue is NULL ? */
	if (skb_queue_empty(&hisi_serv->queue)) {
		mutex_unlock(&hisi_serv->recv_lock);
		cam_err("%s() %d skb_queue is empty", __FUNCTION__, __LINE__);
		/* otherwise block, and wait for data */
		if (wait_event_interruptible_timeout(hisi_serv->readq,
			(!skb_queue_empty(&hisi_serv->queue) ||
			hisi_serv->state == RPMSG_FAIL),
			msecs_to_jiffies(HISP_WAIT_TIMEOUT))) {
			cam_err("%s() %d hisi_serv->state = %d", __FUNCTION__,
				__LINE__, hisi_serv->state);
			rc = -ERESTARTSYS;
			goto RET;
		}

		if (mutex_lock_interruptible(&hisi_serv->recv_lock)) {
			cam_err("%s() %d failed: mutex_lock_interruptible",
				__FUNCTION__, __LINE__);
			rc = -ERESTARTSYS;
			goto RET;
		}
	}

	if (hisi_serv->state == RPMSG_FAIL) {
		cam_err("%s() %d state = RPMSG_FAIL", __FUNCTION__, __LINE__);
		rc = -ENXIO;
		goto UNLOCK_RET;
	}

	skb = skb_dequeue(&hisi_serv->queue);
	if (skb == NULL) {
		cam_err("%s() %d skb is NULL", __FUNCTION__, __LINE__);
		rc = -EIO;
		goto UNLOCK_RET;
	}

	rc = min((unsigned int)len, skb->len);
	msg = (hisp_msg_t*)(skb->data);
	hisp_recvdone((void*)msg);
	if (msg->api_name == ISP_CPU_POWER_OFF_RESPONSE)
		hisp_rpmsgrefs_dump();
	cam_debug("%s: api_name(0x%x)", __FUNCTION__, msg->api_name);

	if (msg->message_id % HISP_MSG_LOG_MOD == 0)
		cam_info("%s: api_name:%#x, message_id:%#x", __FUNCTION__,
			msg->api_name, msg->message_id);

	hisp310_handle_msg(msg);
	if (memcpy_s(user_addr, sizeof(hisp_msg_t), msg, rc) != EOK) {
		rc = -EFAULT;
		cam_err("Fail: %s()%d ret = %d", __FUNCTION__, __LINE__, rc);
	}
	kfree_skb(skb);

UNLOCK_RET:
	mutex_unlock(&hisi_serv->recv_lock);
RET:
	return rc;
}

int hisp310_set_sec_fw_buffer(struct hisp_cfg_data *cfg)
{
	int rc;
	mutex_lock(&hisp_mem_lock_mutex);
	rc = hisp_set_sec_fw_buffer(cfg);
	if(rc < 0)
		cam_err("%s: fail, rc:%d", __FUNCTION__, rc);
	if (s_hisp310.mem.active) {
		s_hisp310.mem.active = 0;
		dma_buf_put(s_hisp310.mem.dmabuf);
	}

	s_hisp310.mem.dmabuf = dma_buf_get(cfg->share_fd);
	if (IS_ERR_OR_NULL(s_hisp310.mem.dmabuf)) {
		cam_err("Fail: dma buffer error");
		mutex_unlock(&hisp_mem_lock_mutex);
		return -EFAULT;
	}
	s_hisp310.mem.active = 1;
	mutex_unlock(&hisp_mem_lock_mutex);
	return rc;
}

int hisp310_release_sec_fw_buffer(void)
{
	int rc;
	mutex_lock(&hisp_mem_lock_mutex);
	rc = hisp_release_sec_fw_buffer();
	if (rc < 0)
		cam_err("%s: fail, rc:%d", __FUNCTION__, rc);

	if (s_hisp310.mem.active) {
		s_hisp310.mem.active = 0;
		dma_buf_put(s_hisp310.mem.dmabuf);
	}
	memset_s(&(s_hisp310.mem), sizeof(struct isp_mem),
		0, sizeof(struct isp_mem));
	mutex_unlock(&hisp_mem_lock_mutex);
	return rc;
}

static void hisp310_deinit_isp_mem(void)
{
	cam_info("func %s", __FUNCTION__);
	mutex_lock(&hisp_mem_lock_mutex);
	if (s_hisp310.mem.active) {
		cam_err("sec isp ex,put dmabuf");
		s_hisp310.mem.active = 0;
		dma_buf_put(s_hisp310.mem.dmabuf);
	}

	memset_s(&(s_hisp310.mem),
		sizeof(struct isp_mem), 0, sizeof(struct isp_mem));
	mutex_unlock(&hisp_mem_lock_mutex);

	return;
}

#ifdef CONFIG_HISI_DEBUG_FS
static void hisp310_set_ddrfreq(int ddr_bandwidth)
{
	cam_info("%s enter,ddr_bandwidth:%d", __FUNCTION__,ddr_bandwidth);
}

static void hisp310_release_ddrfreq(void)
{
	cam_info("%s enter", __FUNCTION__);
	if (current_ddr_bandwidth == 0)
		return;
	pm_qos_remove_request(&qos_request_ddr_down_record);
	current_ddr_bandwidth = 0;
}

static void hisp310_update_ddrfreq(unsigned int ddr_bandwidth)
{
	cam_info("%s enter,ddr_bandwidth:%u", __FUNCTION__,ddr_bandwidth);
	if (!atomic_read(&s_hisp310.opened)) {
		cam_info("%s ,cam is not opened,so u can not set ddr bandwidth",
			__FUNCTION__);
		return;
	}

	if (current_ddr_bandwidth == 0) {
		hisp310_set_ddrfreq(ddr_bandwidth);
	} else if (current_ddr_bandwidth > 0) {
		pm_qos_update_request(&qos_request_ddr_down_record,
			ddr_bandwidth);
		current_ddr_bandwidth = ddr_bandwidth;
	} else {
		cam_err("%s,current_ddr_bandwidth is invalid", __FUNCTION__);
	}
}

static ssize_t hisp_ddr_freq_ctrl_show(struct device *dev,
	struct device_attribute *attr,char *buf)
{
	cam_info("enter %s,current_ddr_bandwidth:%d",
		__FUNCTION__, current_ddr_bandwidth);

	return scnprintf(buf, PAGE_SIZE, "%d", current_ddr_bandwidth);
}

static ssize_t hisp_ddr_freq_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ddr_bandwidth = 0;
	if (buf == NULL) {
		cam_err("%s,input buffer is invalid", __FUNCTION__);
		return -EINVAL;
	}

	ddr_bandwidth =
		simple_strtol(buf, NULL, 10); /* 10 for simple_strtol base */
	cam_info("%s enter,ddr_bandwidth:%d", __FUNCTION__, ddr_bandwidth);

	if (ddr_bandwidth < 0) {
		cam_err("%s,ddr_bandwidth is invalid", __FUNCTION__);
		return -EINVAL;
	} else if (ddr_bandwidth == 0) {
		hisp310_release_ddrfreq();
	} else if (ddr_bandwidth > 0) {
		hisp310_update_ddrfreq(ddr_bandwidth);
	}

	return count;
}
#endif

static int32_t hisp310_rpmsg_probe(struct rpmsg_device *rpdev)
{
	int32_t ret = 0;
	struct rpmsg_hisp310_service *hisi_serv = NULL;
	cam_info("%s enter", __FUNCTION__);

	if (rpmsg_local.hisi_isp_serv != NULL) {
		cam_notice("%s hisi_serv is already up", __FUNCTION__);
		goto SERVER_UP;
	}

	hisi_serv = kzalloc(sizeof(*hisi_serv), GFP_KERNEL);
	if (hisi_serv == NULL) {
		cam_err("%s() %d kzalloc failed", __FUNCTION__, __LINE__);
		ret = -ENOMEM;
		goto ERROR_RET;
	}
	mutex_init(&hisi_serv->send_lock);
	mutex_init(&hisi_serv->recv_lock);
	skb_queue_head_init(&hisi_serv->queue);
	init_waitqueue_head(&hisi_serv->readq);
	hisi_serv->ept = NULL;
	hisi_serv->comp = &rpmsg_local.isp_comp;

	rpmsg_local.hisi_isp_serv = hisi_serv;
SERVER_UP:
	if (hisi_serv == NULL) {
		cam_err("func %s: hisi_serv is NULL", __FUNCTION__);
		return -1;
	}
	hisi_serv->rpdev = rpdev;
	hisi_serv->state = RPMSG_UNCONNECTED;
	dev_set_drvdata(&rpdev->dev, hisi_serv);

	complete(hisi_serv->comp);

	cam_info("new HISI connection srv channel: %u -> %u",
		rpdev->src, rpdev->dst);
ERROR_RET:
	return ret;
}

static struct rpmsg_device_id rpmsg_hisp310_id_table[] = {
	{.name = "rpmsg-hisi"},
	{},
};

MODULE_DEVICE_TABLE(platform, rpmsg_hisp310_id_table);

static const struct of_device_id s_hisp310_dt_match[] = {
	{
	 .compatible = "huawei,hisi_isp310",
	 .data = &s_hisp310.intf,
	},
	{},
};

MODULE_DEVICE_TABLE(of, s_hisp310_dt_match);
#pragma GCC visibility push(default)
static struct rpmsg_driver rpmsg_hisp310_driver = {
	.drv.name = KBUILD_MODNAME,
	.drv.owner = THIS_MODULE,
	.id_table = rpmsg_hisp310_id_table,
	.probe = hisp310_rpmsg_probe,
	.callback = hisp310_rpmsg_driver_cb,
	.remove = hisp310_rpmsg_remove,
};
#pragma GCC visibility pop

#ifdef CONFIG_HISI_DEBUG_FS
static struct device_attribute hisp_ddr_freq_ctrl_attr =
	__ATTR(ddr_freq_ctrl, 0660, /* 0660 for ATTR mode */
		hisp_ddr_freq_ctrl_show, hisp_ddr_freq_store);
#endif /* CONFIG_HISI_DEBUG_FS */

static int32_t hisp310_platform_probe(struct platform_device *pdev)
{
	int32_t ret = 0;
	cam_info("%s: enter", __FUNCTION__);
	wakeup_source_init(&hisp_power_wakelock, "hisp_power_wakelock");
	mutex_init(&hisp_wake_lock_mutex);
	mutex_init(&hisp_power_lock_mutex);
	mutex_init(&hisp_mem_lock_mutex);
	ret = hisp_get_dt_data(pdev, &s_hisp310.dt);
	if (ret < 0) {
		cam_err("%s: get dt failed", __FUNCTION__);
		goto error;
	}

	init_completion(&rpmsg_local.isp_comp);
	ret = hisp_register(pdev, &s_hisp310.intf, &s_hisp310.notify);
	if (ret == 0) {
		atomic_set(&s_hisp310.opened, 0);
	} else {
		cam_err("%s() %d hisp_register failed with ret %d",
			__FUNCTION__, __LINE__, ret);
		goto error;
	}

	rpmsg_local.hisi_isp_serv = NULL;

	ret = register_rpmsg_driver(&rpmsg_hisp310_driver);
	if (ret != 0) {
		cam_err("%s() %d register_rpmsg_driver failed with ret %d",
			__FUNCTION__, __LINE__, ret);
		goto error;
	}
	s_hisp310.pdev = pdev;

	memset_s(&(s_hisp310.mem),
		sizeof(struct isp_mem),
		0,
		sizeof(struct isp_mem));
#ifdef CONFIG_HISI_DEBUG_FS
	ret = device_create_file(&pdev->dev, &hisp_ddr_freq_ctrl_attr);
	if (ret < 0) {
		cam_err("%s failed to creat hisp ddr freq ctrl attribute",
			__FUNCTION__);
		unregister_rpmsg_driver(&rpmsg_hisp310_driver);
		hisp_unregister(s_hisp310.pdev);
		goto error;
	}
#endif
	return 0;

error:
	wakeup_source_trash(&hisp_power_wakelock);
	mutex_destroy(&hisp_wake_lock_mutex);
	mutex_destroy(&hisp_power_lock_mutex);
	mutex_destroy(&hisp_mem_lock_mutex);
	cam_notice("%s exit with ret = %d", __FUNCTION__, ret);
	return ret;
}

static struct platform_driver s_hisp310_driver = {
	.probe = hisp310_platform_probe,
	.driver = {
		.name = "huawei,hisi_isp310",
		.owner = THIS_MODULE,
		.of_match_table = s_hisp310_dt_match,
	},
};

static int __init hisp310_init_module(void)
{
	cam_notice("%s enter", __FUNCTION__);
	return platform_driver_register(&s_hisp310_driver);
}

static void __exit hisp310_exit_module(void)
{
	cam_notice("%s enter", __FUNCTION__);
	hisp_unregister(s_hisp310.pdev);
	platform_driver_unregister(&s_hisp310_driver);
	wakeup_source_trash(&hisp_power_wakelock);
	mutex_destroy(&hisp_wake_lock_mutex);
}

module_init(hisp310_init_module);
module_exit(hisp310_exit_module);
MODULE_DESCRIPTION("hisp310 driver");
MODULE_LICENSE("GPL v2");
