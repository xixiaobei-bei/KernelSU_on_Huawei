 /*
 *Hisilicon K3 SOC camera driver source file
 *
 *Copyright (C) Huawei Technology Co., Ltd.
 *
 *Author:
 *Email:
 *Date:
 *
 *This program is free software; you can redistribute it and/or modify
 *it under the terms of the GNU General Public License as published by
 *the Free Software Foundation; either version 2 of the License, or
 *(at your option) any later version.
 *
 *This program is distributed in the hope that it will be useful,
 *but WITHOUT ANY WARRANTY; without even the implied warranty of
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *GNU General Public License for more details.
 *
 *You should have received a copy of the GNU General Public License
 *along with this program; if not, write to the Free Software
 *Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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
#include <media/huawei/camera.h>
#include <media/huawei/hisp160_cfg.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-subdev.h>
#include <media/videobuf2-core.h>
#include <linux/pm_qos.h>
#include <clocksource/arm_arch_timer.h>
#include <asm/arch_timer.h>
#include <linux/time.h>
#include <linux/jiffies.h>
#include "cam_log.h"
#include "hisp_intf.h"
#include "platform/sensor_commom.h"
#include <linux/wakelock.h>
#include <linux/hisi/hisi_ion.h>
#include <linux/hisi-iommu.h>
#include <linux/platform_data/remoteproc-hisi.h>
#include <linux/iommu.h>
#include <linux/mutex.h>

//lint -save -e740
#define HISP_MSG_LOG_MOD 100

DEFINE_MUTEX(hisi_rpmsg_service_mutex);

static struct pm_qos_request qos_request_ddr_down_record;
static int current_ddr_bandwidth = 0;
static struct wake_lock hisp_power_wakelock;
static struct mutex hisp_wake_lock_mutex;

extern void hisi_isp_boot_stat_dump(void);

typedef enum _timestamp_state_t{
    TIMESTAMP_UNINTIAL = 0,
    TIMESTAMP_INTIAL,
}timestamp_state_t;

static timestamp_state_t s_timestamp_state;
static struct timeval s_timeval;
static u32 s_system_couter_rate;
static u64 s_system_counter;

enum hisp160_rpmsg_state {
	RPMSG_UNCONNECTED,
	RPMSG_CONNECTED,
	RPMSG_FAIL,
};

/*
 *These are used for distinguish the rpmsg_msg status
 *The process in hisp160_rpmsg_ept_cb are different
 *for the first receive and later.
 */
enum {
	HISP_SERV_FIRST_RECV,
	HISP_SERV_NOT_FIRST_RECV,
};

/**@brief the instance for rpmsg service
 *
 *When Histar ISP is probed, this sturcture will be initialized,
 *the object is used to send/recv rpmsg and store the rpmsg data
 *
 *@end
 */
struct rpmsg_hisp160_service {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0))
    struct rpmsg_device *rpdev;
#else
    struct rpmsg_channel *rpdev;
#endif
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

//#define SUPPORT_IRIS_SMP
enum hisp160_mem_pool_attr
{
    MEM_POOL_ATTR_READ_WRITE_CACHE = 0,
    MEM_POOL_ATTR_READ_WRITE_SECURITY,
    MEM_POOL_ATTR_READ_WRITE_CACHE_OFF_LINE,
    MEM_POOL_ATTR_MAX,
};

struct hisp160_mem_pool {
    void *ap_va;
    unsigned int prot;
    unsigned int ion_iova;
    unsigned int r8_iova;
    size_t size;
    size_t align_size;
    int active;
    unsigned int security_isp_mode;
    struct ion_client *ion_client;
} ;

/**@brief the instance to talk to hisp driver
 *
 *When Histar ISP is probed, this sturcture will be initialized,
 *the object is used to notify hisp driver when needed.
 *
 *@end
 */
typedef struct _tag_hisp160 {
	hisp_intf_t intf;
	hisp_notify_intf_t *notify;
	char const *name;
	atomic_t opened;
	struct platform_device*  pdev; //by used to get dts node
	hisp_dt_data_t dt;
    struct iommu_domain *domain;
    struct ion_client *ion_client;
    struct hisp160_mem_pool mem_pool[MEM_POOL_ATTR_MAX];
} hisp160_t;

struct rpmsg_service_info {
	struct rpmsg_hisp160_service *hisi_isp_serv;
	struct completion isp_comp;
	int isp_minor;
};

extern void a7_mmu_unmap(unsigned int va, unsigned int size);

/*Store the only rpmsg_hisp160_service pointer to local static rpmsg_local*/
static struct rpmsg_service_info rpmsg_local;
static bool remote_processor_up = false;

#define I2HI(i) container_of(i, hisp160_t, intf)

static void hisp160_notify_rpmsg_cb(void);
char const *hisp160_get_name(hisp_intf_t *i);
static int hisp160_config(hisp_intf_t *i, void *cfg);

static int hisp160_power_on(hisp_intf_t *i);
static int hisp160_power_off(hisp_intf_t *i);

static int hisp160_send_rpmsg(hisp_intf_t *i, hisp_msg_t *m, size_t len);
static int hisp160_recv_rpmsg(hisp_intf_t *i,
				  hisp_msg_t *user_addr, size_t len);

static void hisp160_set_ddrfreq(int ddr_bandwidth);
static void hisp160_release_ddrfreq(void);
static void hisp160_update_ddrfreq(unsigned int ddr_bandwidth);

void hisp160_init_timestamp(void);
void hisp160_destroy_timestamp(void);
void hisp160_set_timestamp(unsigned int *timestampH, unsigned int *timestampL);
void hisp160_handle_msg(hisp_msg_t *msg);

void hisp160_init_timestamp(void)
{
	s_timestamp_state  		= TIMESTAMP_INTIAL;
	s_system_counter  		= arch_counter_get_cntvct();
	s_system_couter_rate 	= arch_timer_get_rate();
	do_gettimeofday(&s_timeval);

	cam_debug("%s state=%u system_counter=%llu rate=%u"
		" time_second=%ld time_usecond=%ld size=%lu",
		__func__,
		(unsigned int)s_timestamp_state,
		s_system_counter,
		s_system_couter_rate,
		s_timeval.tv_sec,
		s_timeval.tv_usec,
		sizeof(s_timeval)/sizeof(u32));//FIXME
}

void hisp160_destroy_timestamp(void)
{
	s_timestamp_state		= TIMESTAMP_UNINTIAL;
	s_system_counter 		= 0;
	s_system_couter_rate	= 0;
	memset(&s_timeval, 0x00, sizeof(s_timeval));
}

/*Function declaration */
/**********************************************
 * |-----pow-on------->||<----  fw-SOF ---->|
 *  timeval(got)       ----------------->fw_timeval=?
 *  system_counter(got)----------------->fw_sys_counter(got)
 *
 *  fw_timeval = timeval + (fw_sys_counter - system_counter)
 *
 *With a base position(<timeval, system_counter>, we get it at same time),
 *we can calculate fw_timeval with fw syscounter
 *and deliver it to hal. Hal then gets second and microsecond
 *********************************************/
void hisp160_set_timestamp(unsigned int *timestampH, unsigned int *timestampL)
{
/* #define NANOSECOND_PER_SECOND 	(1000000000) */

#define MICROSECOND_PER_SECOND 	(1000000)
	u64 fw_micro_second = 0;
	u64 fw_sys_counter = 0;
	u64 micro_second = 0;

	if (TIMESTAMP_UNINTIAL ==  s_timestamp_state){
		cam_err("%s wouldn't enter this branch.\n", __func__);
		hisp160_init_timestamp();
	}

	if (timestampH == NULL || timestampL == NULL) {
		cam_err("%s timestampH or timestampL is null.\n", __func__);
		return;
	}

	cam_debug("%s ack_high:0x%x ack_low:0x%x", __func__,
		*timestampH, *timestampL);

	if (*timestampH == 0 && *timestampL == 0) {
		return;
	}

    fw_sys_counter = ((u64)(*timestampH)<< 32) | (u64)(*timestampL);//lint !e838
	micro_second = (fw_sys_counter - s_system_counter) * MICROSECOND_PER_SECOND / s_system_couter_rate;//lint !e838

	//chang nano second to micro second
	fw_micro_second =
		(micro_second / MICROSECOND_PER_SECOND + s_timeval.tv_sec) * MICROSECOND_PER_SECOND
		+ ((micro_second % MICROSECOND_PER_SECOND) + s_timeval.tv_usec);//lint !e737

	*timestampH = (u32)(fw_micro_second >>32 & 0xFFFFFFFF);
	*timestampL = (u32)(fw_micro_second & 0xFFFFFFFF);

	cam_debug("%s h:0x%x l:0x%x", __func__, *timestampH, *timestampL);
}

void hisp160_handle_msg(hisp_msg_t *msg)
{
	if (NULL == msg)
		return;
	switch (msg->api_name)
	{
		case REQUEST_RESPONSE:
			hisp160_set_timestamp(&(msg->u.ack_request.timestampH), &(msg->u.ack_request.timestampL));
			break;
		case MSG_EVENT_SENT:
			hisp160_set_timestamp(&(msg->u.event_sent.timestampH), &(msg->u.event_sent.timestampL));
			break;

		default:
			break;
	}
}

static hisp_vtbl_t s_vtbl_hisp160 = {
	.get_name = hisp160_get_name,
	.config = hisp160_config,
	.power_on = hisp160_power_on,
	.power_off = hisp160_power_off,
	.send_rpmsg = hisp160_send_rpmsg,
	.recv_rpmsg = hisp160_recv_rpmsg,
};

static hisp160_t s_hisp160 = {
	.intf = {.vtbl = &s_vtbl_hisp160,},
	.name = "hisp160",
};

static void hisp160_notify_rpmsg_cb(void)
{
	hisp_event_t isp_ev;
	isp_ev.kind = HISP_RPMSG_CB;
	hisp_notify_intf_rpmsg_cb(s_hisp160.notify, &isp_ev);
}


/*Function declaration */
/**********************************************
 *Save the rpmsg from isp to locally skb queue.
 *Only called by hisp160_rpmsg_ept_cb when api_name
 *is NOT POWER_REQ, will notify user space through HISP
 *********************************************/
static void hisp160_save_rpmsg_data(void *data, int len)
{
	struct rpmsg_hisp160_service *hisi_serv = rpmsg_local.hisi_isp_serv;
	struct sk_buff *skb = NULL;
	char *skbdata = NULL;

	if (NULL == hisi_serv){
		cam_err("%s: hisi_serv is NULL",__func__);
		return;
	}
	hisp_assert(NULL != data);
	if (NULL == data){
		return;
	}
	hisp_assert(len > 0);

	skb = alloc_skb(len, GFP_KERNEL);
	if (!skb) {
		cam_err("%s() %d failed: alloc_skb len is %u!\n", __func__,
			__LINE__, len);
		return;
	}

	skbdata = skb_put(skb, len); /*lint !e64 */
	memcpy(skbdata, data, len);

	/*add skb to skb queue */
	mutex_lock(&hisi_serv->recv_lock);
	skb_queue_tail(&hisi_serv->queue, skb);
	mutex_unlock(&hisi_serv->recv_lock);

	wake_up_interruptible(&hisi_serv->readq);
	hisp160_notify_rpmsg_cb();
}

/*Function declaration */
/**********************************************
 *Power up CSI/DPHY/sensor according to isp req
 *Only called by hisp160_rpmsg_ept_cb when api_name
 *is POWER_REQ, and will send a POWER_RSP to isp
 *after power request done.
 *********************************************/

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0))
static void
hisp160_rpmsg_ept_cb(struct rpmsg_device *rpdev,
              void *data, int len, void *priv, u32 src)
#else
static void
hisp160_rpmsg_ept_cb(struct rpmsg_channel *rpdev,
              void *data, int len, void *priv, u32 src)
#endif
{
	struct rpmsg_hisp160_service *hisi_serv = rpmsg_local.hisi_isp_serv;
	hisp_msg_t *msg = NULL;
	struct rpmsg_hdr *rpmsg_msg = NULL;

	if (NULL == hisi_serv){
		cam_err("func %s: hisi_serv is NULL",__func__);
		return;
	}
	if (NULL == data){
		cam_err("func %s: data is NULL",__func__);
		return;
	}

	hisp_assert(len > 0);

	if (RPMSG_CONNECTED != hisi_serv->state) {
		hisp_assert(RPMSG_UNCONNECTED == hisi_serv->state);
		rpmsg_msg = container_of(data, struct rpmsg_hdr, data);
		cam_info("msg src.%u, msg dst.%u\n", rpmsg_msg->src,
			  rpmsg_msg->dst);

		/*add instance dst and modify the instance state */
		hisi_serv->dst = rpmsg_msg->src;
		hisi_serv->state = RPMSG_CONNECTED;
	}

	msg = (hisp_msg_t *) (data);
	/* save the data and wait for hisp160_recv_rpmsg to get the data*/
	hisp_recvx(data);
	hisp160_save_rpmsg_data(data, len);
}

char const *hisp160_get_name(hisp_intf_t *i)
{
	hisp160_t *hi = NULL;
	hisp_assert(NULL != i);
	hi = I2HI(i);
	if (NULL == hi){
		cam_err("func %s: hi is NULL",__func__);
		return NULL;
	}
	return hi->name;
}

static int hisp160_unmap_a7isp_addr(void *cfg)
{
#ifdef CONFIG_HISI_CAMERA_ISP_SECURE
    struct hisp_cfg_data *pcfg = NULL;
    if (NULL == cfg){
		cam_err("func %s: cfg is NULL",__func__);
		return -1;
	}

	pcfg = (struct hisp_cfg_data *)cfg;

    cam_info("func %s: a7 %x, size %x",__func__, pcfg->param.moduleAddr, pcfg->param.size);
    a7_mmu_unmap(pcfg->param.moduleAddr, pcfg->param.size);
    return 0;
#else
    return -ENODEV;
#endif
}

static int hisp160_get_a7isp_addr(void *cfg)
{
    int ret = -ENODEV;
#ifdef CONFIG_HISI_CAMERA_ISP_SECURE
    struct hisp_cfg_data *pcfg = NULL;
    struct scatterlist *sg;
    struct sg_table *table;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0))
    struct dma_buf *buf = NULL;
    struct dma_buf_attachment *attach = NULL;
#else
    struct ion_handle* hdl = NULL;
#endif
    if (NULL == cfg){
        cam_err("func %s: cfg is NULL",__func__);
        return -1;
    }
    pcfg = (struct hisp_cfg_data *)cfg;

    mutex_lock(&hisi_rpmsg_service_mutex);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0))
    ret = hisp_get_sg_table(pcfg->param.sharedFd,&(s_hisp160.pdev->dev),&buf,&attach,&table);
    if(ret < 0){
       cam_err("func %s: get_sg_table failed",__func__);
       goto err_ion_client;
    }
#else
    if (IS_ERR_OR_NULL(s_hisp160.ion_client)) {
        cam_err("func %s: s_hisp160.ion_client is NULL or ERR",__func__);
        goto err_ion_client;
    }

    hdl = ion_import_dma_buf(s_hisp160.ion_client, pcfg->param.sharedFd);
    if (IS_ERR_OR_NULL(hdl)) {
        cam_err("failed to create ion handle!");
        goto err_ion_client;
    }

    cam_info("func %s: import ok",__func__);
    table = ion_sg_table(s_hisp160.ion_client, hdl);
    if (IS_ERR_OR_NULL(table)) {
        cam_err("%s Failed : ion_sg_table.%lu\n", __func__, PTR_ERR(table));
        goto err_ion_sg_table;
    }
    cam_info("func %s: ion_sg_table ok",__func__);
#endif

    sg = table->sgl;

    pcfg->param.moduleAddr = a7_mmu_map(sg, pcfg->param.size,
                        pcfg->param.prot, pcfg->param.type);

    cam_info("func %s: a8 %x",__func__, pcfg->param.moduleAddr);

    ret = 0;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0))
    hisp_free_dma_buf(&buf,&attach,&table);
#else
err_ion_sg_table:
    ion_free(s_hisp160.ion_client, hdl);
#endif
err_ion_client:
    mutex_unlock(&hisi_rpmsg_service_mutex);
#endif
    return ret;
}

static int check_buffer_vaild(struct ion_client * client, struct ion_handle *ionhnd, unsigned int vaild_addr, unsigned int vaild_size)
{
	struct iommu_map_format iommu_format;
	int ret = 0;

	if (IS_ERR(ionhnd) || IS_ERR_OR_NULL(client)) {
		return -1;//lint !e438
	} else {
		memset(&iommu_format, 0, sizeof(struct iommu_map_format));
		if (ion_map_iommu(client, ionhnd, &iommu_format)) {
			return -1;
		}
		if (vaild_addr != iommu_format.iova_start) {
			ret = -1;
		}
		if (vaild_size > iommu_format.iova_size){
			ret = -1;
		}
		ion_unmap_iommu(client, ionhnd);
	}

	return ret;
}

static int hisp160_init_r8isp_memory_pool(void *cfg)
{
    int ret = -ENODEV;
    int ipool;
    struct hisp_cfg_data *pcfg;
    struct scatterlist *sg;
    struct sg_table *table;
    struct ion_handle* hdl = NULL;
    struct ion_client *ion_client;
    enum mapType enMapType;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0))
	struct dma_buf *buf = NULL;
	struct dma_buf_attachment *attach = NULL;
#endif

    if (NULL == cfg){
        cam_err("func %s: cfg is NULL",__func__);
        return -1;
    }

    mutex_lock(&hisi_rpmsg_service_mutex);

    pcfg = (struct hisp_cfg_data *)cfg;

    cam_info("func %s: pool cfg vaddr=0x%pK, iova=0x%x, size=0x%x, type=%d, prot=0x%x align=0x%zd sec=0x%x",__func__, pcfg->param.vaddr,
            pcfg->param.iova,
            pcfg->param.size,
            pcfg->param.type,
            pcfg->param.prot,
            pcfg->param.pool_align_size,
            pcfg->param.security_isp_mode);

    // find suitable mem pool
    if(pcfg->param.type == MAP_TYPE_RAW2YUV){
        ipool =  MEM_POOL_ATTR_READ_WRITE_CACHE_OFF_LINE;
    }else{
        for (ipool = 0; ipool < MEM_POOL_ATTR_MAX; ipool++)
        {
            if (s_hisp160.mem_pool[ipool].prot == pcfg->param.prot) {
                break;
            }
        }

        if (ipool >= MEM_POOL_ATTR_MAX) {
            cam_err("func %s: no pool hit",__func__);
            goto err_ion_client;
        }
    }

    s_hisp160.mem_pool[ipool].ap_va= pcfg->param.vaddr;
    s_hisp160.mem_pool[ipool].ion_iova = pcfg->param.iova;
    s_hisp160.mem_pool[ipool].size  = pcfg->param.size;
    s_hisp160.mem_pool[ipool].align_size  = pcfg->param.pool_align_size;
    s_hisp160.mem_pool[ipool].security_isp_mode  = pcfg->param.security_isp_mode;

    if(ipool == MEM_POOL_ATTR_READ_WRITE_CACHE_OFF_LINE)
    {
        char ion_name[32];

        memset(ion_name, 0, sizeof(ion_name));
        snprintf(ion_name, sizeof(ion_name), "hwcam-hisp160-%d", ipool);
        s_hisp160.mem_pool[ipool].ion_client = hisi_ion_client_create(ion_name);
        if (IS_ERR_OR_NULL(s_hisp160.mem_pool[ipool].ion_client  )) {
            cam_err("failed to create ion client %s\n", ion_name);
            goto err_ion_client;
        }
    }

    ion_client = s_hisp160.mem_pool[ipool].ion_client;
    if (IS_ERR_OR_NULL(ion_client)) {
        cam_err("func %s: s_hisp160.ion_client[%d] is NULL or ERR",__func__,ipool);
        goto err_ion_client;
    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0))
	hdl = ion_import_dma_buf_fd(ion_client, (int)(pcfg->param.sharedFd));
#else
	hdl = ion_import_dma_buf(ion_client, (int)(pcfg->param.sharedFd));
#endif
    if (IS_ERR_OR_NULL(hdl)) {
        cam_err("failed to create ion handle!");
        goto err_ion_client;
    }

    if ((!pcfg->param.security_isp_mode) && check_buffer_vaild(ion_client, hdl, pcfg->param.iova, pcfg->param.size)) {
        cam_err("check buffer fail!");
        goto err_ion_sg_table;
    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0))
	ret = hisp_get_sg_table(pcfg->param.sharedFd,&(s_hisp160.pdev->dev),&buf,&attach,&table);
	if(ret < 0){
		cam_err("func %s: get_sg_table failed",__func__);
		goto err_ion_sg_table;
	}
#else
    table = ion_sg_table(ion_client, hdl);
    if (IS_ERR_OR_NULL(table)) {
        cam_err("%s Failed : ion_sg_table.%lu\n", __func__, PTR_ERR(table));
        goto err_ion_sg_table;
    }
#endif
    sg = table->sgl;

    if(ipool == MEM_POOL_ATTR_READ_WRITE_CACHE_OFF_LINE){
        s_hisp160.mem_pool[ipool].r8_iova  = hisp_mem_map_steup(sg, pcfg->param.iova,
            pcfg->param.size,
            pcfg->param.prot,
            (unsigned int)ipool,
            MAP_TYPE_RAW2YUV,
            (unsigned int)(pcfg->param.pool_align_size));
    }else{
        enMapType = pcfg->param.security_isp_mode ? MAP_TYPE_STATIC_SEC : MAP_TYPE_DYNAMIC;
        s_hisp160.mem_pool[ipool].r8_iova  = hisp_mem_map_steup(sg, pcfg->param.iova,
            pcfg->param.size,
            pcfg->param.prot,
            (unsigned int)ipool,
            enMapType,
            (unsigned int)(pcfg->param.pool_align_size));
    }
    if (!s_hisp160.mem_pool[ipool].r8_iova) {
        cam_err("func %s: hisp_mem_map_steup failed",__func__);
        goto err_ion_sg_table;
    }

    // ion iova isn't equal r8 iova, security or unsecurity, align etc
    // return r8 iova to daemon, and send to r8 later
    pcfg->param.iova = s_hisp160.mem_pool[ipool].r8_iova;
    s_hisp160.mem_pool[ipool].active = 1;

    cam_info("func %s: r8_iova_pool_base=0x%x",__func__, s_hisp160.mem_pool[ipool].r8_iova);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0))
    hisp_free_dma_buf(&buf,&attach,&table);
#endif
    ret = 0;
err_ion_sg_table:
    ion_free(ion_client, hdl);
err_ion_client:
    mutex_unlock(&hisi_rpmsg_service_mutex);
    return ret;
}

static int hisp160_deinit_r8isp_memory_pool(void *cfg)
{
    int ipool;
    struct hisp_cfg_data *pcfg = NULL;

    mutex_lock(&hisi_rpmsg_service_mutex);

    if (NULL == cfg){
        cam_err("func %s: cfg is NULL",__func__);
        goto err_ion_client;
    }
    pcfg = (struct hisp_cfg_data *)cfg;

    // find suitable mem pool
    if(pcfg->param.type == MAP_TYPE_RAW2YUV){
        ipool =  MEM_POOL_ATTR_READ_WRITE_CACHE_OFF_LINE;
    }else{
        for (ipool = 0; ipool < MEM_POOL_ATTR_MAX; ipool++)
        {
            if (s_hisp160.mem_pool[ipool].prot == pcfg->param.prot) {
                break;
            }
        }

        if (ipool >= MEM_POOL_ATTR_MAX) {
            cam_err("func %s: no pool hit",__func__);
            goto err_ion_client;
        }
    }

    hisp_mem_pool_destroy((unsigned int)ipool);

    if(s_hisp160.mem_pool[ipool].ion_client
        && (ipool == MEM_POOL_ATTR_READ_WRITE_CACHE_OFF_LINE))
    {
        cam_debug("%s deinit memory pool ion client %d\n", __func__, ipool);
        ion_client_destroy(s_hisp160.mem_pool[ipool].ion_client);
        s_hisp160.mem_pool[ipool].ion_client = NULL;
    }

    mutex_unlock(&hisi_rpmsg_service_mutex);

    return 0;

err_ion_client:
    mutex_unlock(&hisi_rpmsg_service_mutex);
    return -ENODEV;
}

// handle daemon carsh
// miss ispmanager poweroff
// miss memory pool deinit
static int hisp160_deinit_r8isp_memory_pool_force(void)
{
    int ipool;

    cam_info("func %s", __func__);

    mutex_lock(&hisi_rpmsg_service_mutex);

    for (ipool = 0; ipool < MEM_POOL_ATTR_MAX; ipool++)
    {
        if (s_hisp160.mem_pool[ipool].active) {
            hisp_mem_pool_destroy((unsigned int)ipool);
        }

        if(s_hisp160.mem_pool[ipool].ion_client
            && (ipool == MEM_POOL_ATTR_READ_WRITE_CACHE_OFF_LINE))
        {
            cam_debug("%s deinit memory pool ion client %d\n", __func__, ipool);
            ion_client_destroy(s_hisp160.mem_pool[ipool].ion_client);
            s_hisp160.mem_pool[ipool].ion_client = NULL;
        }
    }

    mutex_unlock(&hisi_rpmsg_service_mutex);

    return 0;
}

static int hisp160_alloc_r8isp_addr(void *cfg)
{
    int ipool;
    unsigned int r8_iova;
    size_t  offset;
    struct hisp_cfg_data *pcfg;
    int rc = 0;
    bool bSecureMode = false;

    if (NULL == cfg){
        cam_err("func %s: cfg is NULL",__func__);
        return -1;
    }

    mutex_lock(&hisi_rpmsg_service_mutex);

    pcfg = (struct hisp_cfg_data *)cfg;

    //handle static memory, just return r8 reserved iova address == map only
    if (pcfg->param.type == MAP_TYPE_STATIC)
    {
#ifndef CONFIG_HISI_CAMERA_ISP_SECURE
        mutex_unlock(&hisi_rpmsg_service_mutex);
        return -ENODEV;
#else
        cam_debug("func %s static", __func__);
        pcfg->param.iova = a7_mmu_map(NULL, pcfg->param.size,
                pcfg->param.prot, pcfg->param.type);
        mutex_unlock(&hisi_rpmsg_service_mutex);
        return 0;
#endif
    }

	// handle dynamic carveout alloc
    if (pcfg->param.type == MAP_TYPE_DYNAMIC_CARVEOUT) {
        cam_debug("func %s dynamic carveout", __func__);
        pcfg->param.iova = hisp_mem_pool_alloc_carveout(pcfg->param.size, pcfg->param.type);
        mutex_unlock(&hisi_rpmsg_service_mutex);
        return 0;
    }

    for (ipool = 0; ipool < MEM_POOL_ATTR_MAX; ipool++)
    {
        if (s_hisp160.mem_pool[ipool].security_isp_mode) {
            bSecureMode = true;
            break;
        }
    }

    // hanlde dynamic memory alloc
    if(pcfg->param.type == MAP_TYPE_RAW2YUV){
        ipool =  MEM_POOL_ATTR_READ_WRITE_CACHE_OFF_LINE;
    }else{
        for (ipool = 0; ipool < MEM_POOL_ATTR_MAX; ipool++)
        {
            if (s_hisp160.mem_pool[ipool].prot == pcfg->param.prot) {
                break;
            }
        }

        if (ipool >= MEM_POOL_ATTR_MAX) {
            cam_err("func %s: no pool hit",__func__);
            rc = -EINVAL;
            goto alloc_err;
        }
    }

    r8_iova = (unsigned int)hisp_mem_pool_alloc_iova(pcfg->param.size, (unsigned int)ipool);
    if (!r8_iova) {
        cam_err("func %s: hisp_mem_pool_alloc_iova error",__func__);
        rc = -ENOMEM;
        goto alloc_err;
    }

    // offset calculator
    // security mode, pool base is r8_iova, is security address, not align
    // normal mode, pool base is ion_iova, is normal address,  align by isp.
    if(pcfg->param.type == MAP_TYPE_RAW2YUV){
        offset = r8_iova - s_hisp160.mem_pool[ipool].r8_iova;
    }else{
        if (bSecureMode) {
            offset = r8_iova - s_hisp160.mem_pool[ipool].r8_iova;
        } else {
            offset = r8_iova - s_hisp160.mem_pool[ipool].ion_iova;
        }

    }

    if (offset > s_hisp160.mem_pool[ipool].size) {
        cam_err("func %s: r8_iova invalid",__func__);
        rc = -EFAULT;
        goto alloc_err;
    }

    pcfg->param.vaddr = (void *)(((unsigned char *)s_hisp160.mem_pool[ipool].ap_va) + offset);
    pcfg->param.iova = r8_iova;
    pcfg->param.offset_in_pool = offset;

    mutex_unlock(&hisi_rpmsg_service_mutex);
    return 0;

alloc_err:
    mutex_unlock(&hisi_rpmsg_service_mutex);
    return rc;
}

static int hisp160_free_r8isp_addr(void *cfg)
{
    int rc = 0;
    int ipool;
    struct hisp_cfg_data *pcfg;

    if (NULL == cfg) {
        cam_err("func %s: cfg is NULL",__func__);
        return -1;
    }

    mutex_lock(&hisi_rpmsg_service_mutex);

    pcfg = (struct hisp_cfg_data *)cfg;

    //handle static memory, unmap only
    if (pcfg->param.type == MAP_TYPE_STATIC) {
#ifndef CONFIG_HISI_CAMERA_ISP_SECURE
        mutex_unlock(&hisi_rpmsg_service_mutex);
        return -ENODEV;
#else
        cam_debug("func %s static", __func__);
        a7_mmu_unmap(pcfg->param.iova, pcfg->param.size);
        mutex_unlock(&hisi_rpmsg_service_mutex);
        return 0;
#endif
    }


    // handle dynamic carveout free
    if (pcfg->param.type == MAP_TYPE_DYNAMIC_CARVEOUT) {
        cam_debug("func %s dynamic carveout", __func__);
        rc = hisp_mem_pool_free_carveout(pcfg->param.iova, pcfg->param.size);
     if (rc) {
             cam_err("func %s: hisp_mem_pool_free_carveout error",__func__);
        }
        mutex_unlock(&hisi_rpmsg_service_mutex);
        return 0;
    }

    // hanlde dynamic memory alloc
    if(pcfg->param.type == MAP_TYPE_RAW2YUV){
        ipool =  MEM_POOL_ATTR_READ_WRITE_CACHE_OFF_LINE;
    }else{
        for (ipool = 0; ipool < MEM_POOL_ATTR_MAX; ipool++)
        {
            if (s_hisp160.mem_pool[ipool].prot == pcfg->param.prot) {
                break;
            }
        }

        if (ipool >= MEM_POOL_ATTR_MAX) {
            cam_err("func %s: no pool hit",__func__);
            rc = -EINVAL;
            goto free_err;
        }
    }

    rc = (int)hisp_mem_pool_free_iova((unsigned int)ipool, pcfg->param.iova, pcfg->param.size);
    if (rc) {
        cam_err("func %s: hisp_mem_pool_free_iova error",__func__);
        rc = -EFAULT;
        goto free_err;
    }

    mutex_unlock(&hisi_rpmsg_service_mutex);
    return 0;

free_err:
    mutex_unlock(&hisi_rpmsg_service_mutex);
    return rc;
}

static int hisp160_mem_pool_pre_init(void)
{
    int ipool;
    int prot;
    char ion_name[32];

    cam_info("func %s", __func__);

    for (ipool = 0; ipool < MEM_POOL_ATTR_MAX; ipool++)
    {
        memset(&(s_hisp160.mem_pool[ipool]), 0, sizeof(struct hisp160_mem_pool));

        switch (ipool)
        {
            case MEM_POOL_ATTR_READ_WRITE_CACHE:
            case MEM_POOL_ATTR_READ_WRITE_CACHE_OFF_LINE:
                prot = IOMMU_READ | IOMMU_WRITE | IOMMU_CACHE;
                break;

            case MEM_POOL_ATTR_READ_WRITE_SECURITY:
                prot = IOMMU_READ | IOMMU_WRITE | IOMMU_CACHE | IOMMU_SEC;
                break;

            default:
                prot = -1;
                break;
        }

        cam_debug("%s  ipool %d prot 0x%x", __func__, ipool, prot);

        if (prot < 0) {
            cam_err("%s unkown ipool %d prot 0x%x", __func__, ipool, prot);
            return -EINVAL;
        }

        s_hisp160.mem_pool[ipool].prot = (unsigned int)prot;

        if(ipool != MEM_POOL_ATTR_READ_WRITE_CACHE_OFF_LINE)
        {
            memset(ion_name, 0, sizeof(ion_name));
            snprintf(ion_name, sizeof(ion_name), "hwcam-hisp160-%d", ipool);
            s_hisp160.mem_pool[ipool].ion_client = hisi_ion_client_create(ion_name);
            if (IS_ERR_OR_NULL(s_hisp160.mem_pool[ipool].ion_client  )) {
                cam_err("failed to create ion client %s\n", ion_name);
                return -ENOMEM;
            }
        }
    }

    return 0;
}


static int hisp160_mem_pool_later_deinit(void)
{
    int ipool;
    cam_info("func %s", __func__);

    for (ipool = 0; ipool < MEM_POOL_ATTR_MAX; ipool++)
    {
        if (s_hisp160.mem_pool[ipool].ion_client
            && (ipool != MEM_POOL_ATTR_READ_WRITE_CACHE_OFF_LINE)) {
            cam_debug("%s deinit memory pool ion client %d\n", __func__, ipool);
            ion_client_destroy(s_hisp160.mem_pool[ipool].ion_client);
            s_hisp160.mem_pool[ipool].ion_client = NULL;
        }

        memset(&(s_hisp160.mem_pool[ipool]), 0, sizeof(struct hisp160_mem_pool));
    }

    return 0;
}
static int hisp160_config(hisp_intf_t *i, void *cfg)
{
	int rc = 0;
	hisp160_t *hi = NULL;
	struct hisp_cfg_data *pcfg = NULL;

	hisp_assert(NULL != i);
	if (NULL == cfg){
		cam_err("func %s: cfg is NULL",__func__);
		return -1;
	}
	pcfg = (struct hisp_cfg_data *)cfg;
	hi = I2HI(i);
	hisp_assert(NULL != hi);

	switch (pcfg->cfgtype) {
	case HISP_CONFIG_POWER_ON:
		if (!remote_processor_up) {
			if(pcfg->isSecure == 0){
				hisi_isp_rproc_case_set(NONSEC_CASE);
			}else if(pcfg->isSecure == 1){
				hisi_isp_rproc_case_set(SEC_CASE);
			}else{
				cam_info("%s invalid mode\n", __func__);
			}
			cam_notice("%s power on the hisp160.\n", __func__);
			rc = hisp160_power_on(i);
		}
		else{
			cam_warn("%s hisp160 is still on power-on state, power off it.\n",
				__func__);

			hisp160_deinit_r8isp_memory_pool_force();

			rc = hisp160_power_off(i);
			if (0 != rc){
				break;
			}

			cam_warn("%s begin to power on the hisp160.\n", __func__);
			rc = hisp160_power_on(i);
		}
		break;
	case HISP_CONFIG_POWER_OFF:
		if (remote_processor_up) {
			cam_notice("%s power off the hisp160.\n", __func__);
			rc = hisp160_power_off(i);
		}
		break;
    case HISP_CONFIG_GET_MAP_ADDR:
        rc = hisp160_get_a7isp_addr(cfg);
        cam_notice("%s get a7 map address 0x%x\n", __func__,pcfg->param.moduleAddr);
        break;
    case HISP_CONFIG_UNMAP_ADDR:
        cam_notice("%s unmap a7 address from isp atf\n", __func__);
        rc = hisp160_unmap_a7isp_addr(cfg);
        break;

    case HISP_CONFIG_INIT_MEMORY_POOL:
        rc = hisp160_init_r8isp_memory_pool(cfg);
        break;

    case HISP_CONFIG_DEINIT_MEMORY_POOL:
        rc = hisp160_deinit_r8isp_memory_pool(cfg);
        break;

    case HISP_CONFIG_ALLOC_MEM:
        rc = hisp160_alloc_r8isp_addr(cfg);
        break;

    case HISP_CONFIG_FREE_MEM:
        rc = hisp160_free_r8isp_addr(cfg);
        break;

    case HISP_CONFIG_ISP_TURBO:
        cam_info("%s HISP_CONFIG_ISP_TURBO", __func__);
        rc = hisp_set_clk_rate(VIVOBUS_CLK, 600000000);
        rc = hisp_set_clk_rate(ISPFUNC_CLK, 554000000);
        break;
    case HISP_CONFIG_ISP_NORMAL:
        cam_info("%s HISP_CONFIG_ISP_NORMAL", __func__);
        rc = hisp_set_clk_rate(VIVOBUS_CLK, 400000000);
        rc = hisp_set_clk_rate(ISPFUNC_CLK, 480000000);
        break;
    case HISP_CONFIG_ISP_LOWPOWER:
        cam_info("%s HISP_CONFIG_ISP_LOWPOWER", __func__);
        rc = hisp_set_clk_rate(VIVOBUS_CLK, 300000000);
        rc = hisp_set_clk_rate(ISPFUNC_CLK, 300000000);
        break;
    case HISP_CONFIG_R8_TURBO:
        cam_info("%s HISP_CONFIG_R8_TURBO", __func__);
        rc = hisp_set_clk_rate(ISPCPU_CLK,  900000000);
        break;
    case HISP_CONFIG_R8_NORMAL:
        cam_info("%s HISP_CONFIG_R8_NORMAL", __func__);
        rc = hisp_set_clk_rate(ISPCPU_CLK,  830000000);
        break;
    case HISP_CONFIG_R8_LOWPOWER:
        cam_info("%s HISP_CONFIG_R8_TURBO", __func__);
        rc = hisp_set_clk_rate(ISPCPU_CLK,  600000000);
        break;
    case HISP_CONFIG_PROC_TIMEOUT:
        cam_info("%s message_id.0x%x",__func__, pcfg->cfgdata[0]);
        hisp_dump_rpmsg_with_id(pcfg->cfgdata[0]);
        break;
    default:
        break;
	}
	return rc;
}

static int hisp160_power_on(hisp_intf_t *i)
{
	int rc = 0;
	bool rproc_enabled = false;
	bool hi_opened = false;
       bool ion_client_created = false;
	hisp160_t *hi = NULL;
	unsigned long current_jiffies = jiffies;
	uint32_t timeout = hw_is_fpga_board() ? 60000 : 15000;

	struct rpmsg_hisp160_service *hisi_serv = NULL;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0))
    struct rpmsg_channel_info chinfo = {
        .src = RPMSG_ADDR_ANY,
    };
#endif
	if (NULL == i){
		return -1;
	}
	hi = I2HI(i);

	cam_info("%s enter ....\n", __func__);

	mutex_lock(&hisp_wake_lock_mutex);
	if (!wake_lock_active(&hisp_power_wakelock)) {
		wake_lock(&hisp_power_wakelock);
		cam_info("%s hisp power on enter, wake lock\n", __func__);
	}
	mutex_unlock(&hisp_wake_lock_mutex); /*lint !e456 */

	mutex_lock(&hisi_rpmsg_service_mutex);
	if (!atomic_read((&hi->opened))) {
		if (!hw_is_fpga_board()) {
			if (!IS_ERR(hi->dt.pinctrl_default)) {
				rc = pinctrl_select_state(hi->dt.pinctrl, hi->dt.pinctrl_default);
				if (0 != rc) {
					goto FAILED_RET;
				}
			}
		}

        hisp_rpmsgrefs_reset();
		rc = hisi_isp_rproc_enable();
		if (rc != 0) {
			goto FAILED_RET;
		}
		rproc_enabled = true;

		rc  = wait_for_completion_timeout(&channel_sync, msecs_to_jiffies(timeout));
		if (0 == rc ) {
			rc = -ETIME;
			hisi_isp_boot_stat_dump();
			goto FAILED_RET;
		}else {
			cam_info("%s() %d after wait completion, rc = %d!\n", __func__, __LINE__, rc);
			rc = 0;
		}

		atomic_inc(&hi->opened);
		hi_opened = true;
	} else {
		cam_notice("%s isp has been opened.\n", __func__);
	}
	remote_processor_up = true;
	hisi_serv = rpmsg_local.hisi_isp_serv;
	if (!hisi_serv) {
		rc = -ENODEV;
		goto FAILED_RET;
	}

	/*assign a new, unique, local address and associate instance with it */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0))
    hisi_serv->ept = rpmsg_create_ept(hisi_serv->rpdev, hisp160_rpmsg_ept_cb, hisi_serv,
              chinfo);
#else
hisi_serv->ept =
rpmsg_create_ept(hisi_serv->rpdev, hisp160_rpmsg_ept_cb, hisi_serv,
              RPMSG_ADDR_ANY);
#endif
	if (!hisi_serv->ept) {
		rc = -ENOMEM;
		goto FAILED_RET;
	}
	cam_info("%s() %d hisi_serv->rpdev:src.%d, dst.%d\n",
			__func__, __LINE__,
			hisi_serv->rpdev->src, hisi_serv->rpdev->dst);
	hisi_serv->state = RPMSG_CONNECTED;

	/*set the instance recv_count */
	hisi_serv->recv_count = HISP_SERV_FIRST_RECV;

	hisp160_init_timestamp();

    s_hisp160.ion_client = hisi_ion_client_create("hwcam-hisp160");
    if (IS_ERR_OR_NULL(s_hisp160.ion_client )) {
        cam_err("failed to create ion client! \n");
        rc = -ENOMEM;
        goto FAILED_RET;
    }

    if (hisp160_mem_pool_pre_init() ) {
        cam_err("failed to pre init mem pool \n");
        rc = -ENOMEM;
        goto FAILED_RET;
    }

    ion_client_created = true;
	mutex_unlock(&hisi_rpmsg_service_mutex);
	cam_info("%s exit ,power on time:%d....\n", __func__,
			jiffies_to_msecs(jiffies - current_jiffies) );
	return rc; /*lint !e454 */

FAILED_RET:
	if(hi_opened){
		atomic_dec(&hi->opened);
	}

	if (rproc_enabled) {
		hisi_isp_rproc_disable();
               rproc_set_sync_flag(true);
	}

    if (ion_client_created) {
        ion_client_destroy(s_hisp160.ion_client);
        s_hisp160.ion_client = NULL;
        hisp160_mem_pool_later_deinit();
    }
	remote_processor_up = false;

	mutex_unlock(&hisi_rpmsg_service_mutex);

	mutex_lock(&hisp_wake_lock_mutex);
	if (wake_lock_active(&hisp_power_wakelock)) {
		wake_unlock(&hisp_power_wakelock);
		cam_err("%s hisp power on failed, wake unlock\n", __func__);
	}
	mutex_unlock(&hisp_wake_lock_mutex); /*lint !e456 */
	return rc; /*lint !e454 */
} /*lint !e454 */

static int hisp160_power_off(hisp_intf_t *i)
{
	int rc = 0;
	hisp160_t *hi = NULL;
	unsigned long current_jiffies = jiffies;
	struct rpmsg_hisp160_service *hisi_serv = NULL;
	if (NULL == i){
		return -1;
	}
	hi = I2HI(i);

	cam_info("%s enter ....\n", __func__);

	/*check the remote processor boot flow */
	if (false == remote_processor_up) {
		rc = -EPERM;
		goto RET;
	}

	hisi_serv = rpmsg_local.hisi_isp_serv;
	if (!hisi_serv) {
		rc = -ENODEV;
		goto RET;
	}

	if (RPMSG_FAIL == hisi_serv->state) {
		rc = -EFAULT;
		goto RET;
	}

	mutex_lock(&hisi_rpmsg_service_mutex);

	if (!hisi_serv->ept) {
		rc = -ENODEV;
		goto UNLOCK_RET;
	}
	rpmsg_destroy_ept(hisi_serv->ept);
	hisi_serv->ept = NULL;

	hisi_serv->state = RPMSG_UNCONNECTED;
	hisi_serv->recv_count = HISP_SERV_FIRST_RECV;

	if (atomic_read((&hi->opened))) {

		hisi_isp_rproc_disable();


		if (!hw_is_fpga_board()) {
			if (!IS_ERR(hi->dt.pinctrl_idle)) {
				rc = pinctrl_select_state(hi->dt.pinctrl, hi->dt.pinctrl_idle);
				if (0 != rc) {
            // Empty.
				}
			}
		}

		remote_processor_up = false;
		atomic_dec(&hi->opened);
	} else {
		cam_notice("%s isp hasn't been opened.\n", __func__);
	}

	hisp160_destroy_timestamp();
UNLOCK_RET:
    if (s_hisp160.ion_client) {
        ion_client_destroy(s_hisp160.ion_client);
        s_hisp160.ion_client = NULL;
    }

    hisp160_mem_pool_later_deinit();

	mutex_unlock(&hisi_rpmsg_service_mutex);
RET:
	cam_info("%s exit ,power 0ff time:%d....\n", __func__,
			jiffies_to_msecs(jiffies - current_jiffies) );

	mutex_lock(&hisp_wake_lock_mutex);
	if (wake_lock_active(&hisp_power_wakelock)) {
		wake_unlock(&hisp_power_wakelock);
		cam_info("%s hisp power off exit, wake unlock\n", __func__);
	}
	mutex_unlock(&hisp_wake_lock_mutex); /*lint !e456 */
	return rc;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0))
static void hisp160_rpmsg_remove(struct rpmsg_device *rpdev)
#else
static void hisp160_rpmsg_remove(struct rpmsg_channel *rpdev)
#endif
{
	struct rpmsg_hisp160_service *hisi_serv = dev_get_drvdata(&rpdev->dev);

	cam_info("%s enter ....\n", __func__);

	if(hisi_serv == NULL){
		cam_err("%s: hisi_serv == NULL!", __func__);
		return;
	}

	mutex_destroy(&hisi_serv->send_lock);
	mutex_destroy(&hisi_serv->recv_lock);

	kfree(hisi_serv);
	rpmsg_local.hisi_isp_serv = NULL;
	cam_notice("rpmsg hisi driver is removed\n");
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0))
static void
hisp160_rpmsg_driver_cb(struct rpmsg_device *rpdev,
            void *data, int len, void *priv, u32 src)
#else
static void
hisp160_rpmsg_driver_cb(struct rpmsg_channel *rpdev,
            void *data, int len, void *priv, u32 src)
#endif
{
	cam_info("%s enter ....\n", __func__);
	cam_warn("%s() %d uhm, unexpected message!\n", __func__,
		__LINE__);

	print_hex_dump(KERN_DEBUG, __func__, DUMP_PREFIX_NONE, 16, 1,
			   data, len, true);
}

static int
hisp160_send_rpmsg(hisp_intf_t *i, hisp_msg_t *from_user, size_t len)
{
	int rc = 0;
	hisp160_t *hi = NULL;
	struct rpmsg_hisp160_service *hisi_serv = NULL;
	hisp_msg_t *msg = from_user;
	hisp_assert(NULL != i);
	hisp_assert(NULL != from_user);
	hi = I2HI(i);

	cam_debug("%s enter. api_name(0x%x)\n", __func__, msg->api_name);

	if (msg->message_id % HISP_MSG_LOG_MOD == 0) {
		cam_info("%s: api_name:%#x, message_id:%#x\n", __FUNCTION__,
				msg->api_name, msg->message_id);
	}

	hisi_serv = rpmsg_local.hisi_isp_serv;
	if (!hisi_serv) {
		cam_err("%s() %d failed: hisi_serv does not exist!\n", __func__,
				__LINE__);
		rc = -ENODEV;
		goto RET;
	}

	if (!hisi_serv->ept) {
		cam_err("%s() %d failed:hisi_serv->ept does not exist!\n", __func__,
			__LINE__);
		rc = -ENODEV;
		goto RET;
	}

	mutex_lock(&hisi_serv->send_lock);
	/*if the msg is the first msg, let's treat it special */
	if (RPMSG_CONNECTED != hisi_serv->state) {
		if (!hisi_serv->rpdev) {
			cam_err("%s() %d failed:hisi_serv->rpdev does not exist!\n", __func__,
				__LINE__);
			rc = -ENODEV;
			goto UNLOCK_RET;
		}
		hisp_sendin(msg);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0))
		rc = rpmsg_send_offchannel(hisi_serv->ept,
					   hisi_serv->ept->addr,
					   hisi_serv->rpdev->dst, (void *)msg,
					   len);
#else
        rc = rpmsg_send_offchannel(hisi_serv->rpdev,
					   hisi_serv->ept->addr,
					   hisi_serv->rpdev->dst, (void *)msg,
					   len);
#endif

		if (rc) {
			cam_err("%s() %d failed: first rpmsg_send_offchannel ret is %d!\n", __func__,
				__LINE__, rc);
		}
		goto UNLOCK_RET;
	}
	hisp_sendin(msg);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0))
	rc = rpmsg_send_offchannel(hisi_serv->ept, hisi_serv->ept->addr,
				   hisi_serv->dst, (void *)msg, len);
#else
    rc = rpmsg_send_offchannel(hisi_serv->rpdev, hisi_serv->ept->addr,
				   hisi_serv->dst, (void *)msg, len);
#endif

	if (rc) {
		cam_err("%s() %d failed: rpmsg_send_offchannel ret is %d!\n", __func__,
			__LINE__, rc);
		goto UNLOCK_RET;
	}
UNLOCK_RET:
	mutex_unlock(&hisi_serv->send_lock);
RET:
	return rc;
}

static int
hisp160_recv_rpmsg(hisp_intf_t *i, hisp_msg_t *user_addr, size_t len)
{
	int rc = len;
	hisp160_t *hi = NULL;
	struct rpmsg_hisp160_service *hisi_serv = NULL;
	struct sk_buff *skb = NULL;
	hisp_msg_t *msg = NULL;
	hisp_assert(NULL != i);
	if (NULL == user_addr){
		cam_err("func %s: user_addr is NULL",__func__);
		return -1;
	}
	hi = I2HI(i);

	cam_debug("%s enter. \n", __func__);

	hisi_serv = rpmsg_local.hisi_isp_serv;
	if (!hisi_serv) {
		cam_err("%s() %d failed: hisi_serv does not exist!\n", __func__,
			__LINE__);
		rc = -ENODEV;
		goto RET;
	}

	if (HISP_SERV_FIRST_RECV == hisi_serv->recv_count) {
		hisi_serv->recv_count = HISP_SERV_NOT_FIRST_RECV;
	}

	if (mutex_lock_interruptible(&hisi_serv->recv_lock)) {
		cam_err("%s() %d failed: mutex_lock_interruptible!\n", __func__,
			__LINE__);
		rc = -ERESTARTSYS;
		goto RET;
	}

	if (RPMSG_CONNECTED != hisi_serv->state) {
		cam_err("%s() %d hisi_serv->state != RPMSG_CONNECTED!\n",
			__func__, __LINE__);
		rc = -ENOTCONN;
		goto UNLOCK_RET;
	}

	/*nothing to read ? */
	/*check if skb_queue is NULL ? */
	if (skb_queue_empty(&hisi_serv->queue)) {
		mutex_unlock(&hisi_serv->recv_lock); /*lint !e455 */
		cam_err("%s() %d skb_queue is empty!\n", __func__, __LINE__);

		/*otherwise block, and wait for data */
		if (wait_event_interruptible_timeout(hisi_serv->readq,
						 (!skb_queue_empty
						  (&hisi_serv->queue)
						  || hisi_serv->state ==
						  RPMSG_FAIL),
						  HISP_WAIT_TIMEOUT)) { //lint !e666 !e665
			cam_err("%s() %d hisi_serv->state = %d!\n", __func__,
			__LINE__, hisi_serv->state);
			rc = -ERESTARTSYS;
			goto RET;
		}

		if (mutex_lock_interruptible(&hisi_serv->recv_lock)) {
			cam_err("%s() %d failed: mutex_lock_interruptible!\n",
				__func__, __LINE__);
			rc = -ERESTARTSYS;
			goto RET;
		}
	}

	if (RPMSG_FAIL == hisi_serv->state) {
		cam_err("%s() %d state = RPMSG_FAIL!\n", __func__, __LINE__);
		rc = -ENXIO;
		goto UNLOCK_RET;
	}

	skb = skb_dequeue(&hisi_serv->queue);
	if (!skb) {
		cam_err("%s() %d skb is NULL!\n", __func__, __LINE__);
		rc = -EIO;
		goto UNLOCK_RET;
	}

	rc = min((unsigned int)len, skb->len);
	msg = (hisp_msg_t *) (skb->data);
	hisp_recvdone((void *)msg);
	if(msg->api_name == ISP_CPU_POWER_OFF_RESPONSE)
	{
		hisp_rpmsgrefs_dump();
	}
	cam_debug("%s: api_name(0x%x)\n", __func__, msg->api_name);

	if (msg->message_id % HISP_MSG_LOG_MOD == 0) {
		cam_info("%s: api_name:%#x, message_id:%#x\n", __FUNCTION__,
				msg->api_name, msg->message_id);
	}

	hisp160_handle_msg(msg);
	if (!memcpy(user_addr, msg, rc)) {
		rc = -EFAULT;
		cam_err("Fail: %s()%d ret = %d\n", __func__, __LINE__, rc);
	}
	kfree_skb(skb);

UNLOCK_RET:
	mutex_unlock(&hisi_serv->recv_lock); /*lint !e455 */
RET:
	return rc;
}

static void hisp160_set_ddrfreq(int ddr_bandwidth)
{
	cam_info("%s enter,ddr_bandwidth:%d\n",__func__,ddr_bandwidth);
}

static void hisp160_release_ddrfreq(void)
{
	cam_info("%s enter\n",__func__);
	if(current_ddr_bandwidth == 0)
		return;
	pm_qos_remove_request(&qos_request_ddr_down_record);
	current_ddr_bandwidth = 0;
}

static void hisp160_update_ddrfreq(unsigned int ddr_bandwidth)
{
	cam_info("%s enter,ddr_bandwidth:%u\n",__func__,ddr_bandwidth);
	if(!atomic_read(&s_hisp160.opened)){
		cam_info("%s ,cam is not opened,so u can not set ddr bandwidth\n",__func__);
		return;
	}

	if(current_ddr_bandwidth == 0){
		hisp160_set_ddrfreq(ddr_bandwidth);
	}else if(current_ddr_bandwidth > 0){
		pm_qos_update_request(&qos_request_ddr_down_record, ddr_bandwidth);
		current_ddr_bandwidth = ddr_bandwidth;
	}else{
		cam_err("%s,current_ddr_bandwidth is invalid\n",__func__);
	}
}

static ssize_t hisp_ddr_freq_ctrl_show(struct device *dev,
	struct device_attribute *attr,char *buf)
{
	cam_info("enter %s,current_ddr_bandwidth:%d\n", __func__, current_ddr_bandwidth);

	return snprintf(buf, PAGE_SIZE, "%d\n", current_ddr_bandwidth);
}

static ssize_t hisp_ddr_freq_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ddr_bandwidth = 0;
	if (buf == NULL) {
		cam_err("%s,input buffer is invalid\n",__func__);
		return -EINVAL;
	}

	ddr_bandwidth = simple_strtol(buf, NULL, 10);
	cam_info("%s enter,ddr_bandwidth:%d\n", __func__, ddr_bandwidth);

	if (ddr_bandwidth < 0){
		cam_err("%s,ddr_bandwidth is invalid\n",__func__);
		return -EINVAL;
	}else if (ddr_bandwidth == 0) {
		hisp160_release_ddrfreq();
	}else if (ddr_bandwidth > 0) {
		hisp160_update_ddrfreq(ddr_bandwidth);
	}

	return count;
}


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0))
static int32_t hisp160_rpmsg_probe(struct rpmsg_device *rpdev)
#else
static int32_t hisp160_rpmsg_probe(struct rpmsg_channel *rpdev)
#endif
{
	int32_t ret = 0;
	struct rpmsg_hisp160_service *hisi_serv = NULL;
	cam_info("%s enter\n", __func__);

	if (NULL != rpmsg_local.hisi_isp_serv) {
		cam_notice("%s hisi_serv is already up!\n", __func__);
		goto SERVER_UP;
	}

	hisi_serv = kzalloc(sizeof(*hisi_serv), GFP_KERNEL);
	if (!hisi_serv) {
		cam_err("%s() %d kzalloc failed!\n", __func__, __LINE__);
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
	if (NULL == hisi_serv){
		cam_err("func %s: hisi_serv is NULL",__func__);
		return -1;
	}
	hisi_serv->rpdev = rpdev;
	hisi_serv->state = RPMSG_UNCONNECTED;
	dev_set_drvdata(&rpdev->dev, hisi_serv);

	complete(hisi_serv->comp);

	cam_info("new HISI connection srv channel: %u -> %u!\n",
						rpdev->src, rpdev->dst);
ERROR_RET:
	return ret;
}

static struct rpmsg_device_id rpmsg_hisp160_id_table[] = {
	{.name = "rpmsg-hisi"},
	{},
};

MODULE_DEVICE_TABLE(platform, rpmsg_hisp160_id_table);

static const struct of_device_id s_hisp160_dt_match[] = {
	{
	 .compatible = "huawei,hisi_isp160",
	 .data = &s_hisp160.intf,
	 },
	{
	 },
};

MODULE_DEVICE_TABLE(of, s_hisp160_dt_match);

static struct rpmsg_driver rpmsg_hisp160_driver = {
	.drv.name   = KBUILD_MODNAME, //lint !e64 !e485
	.drv.owner  = THIS_MODULE, //lint !e64 !e485
	.id_table = rpmsg_hisp160_id_table,
	.probe = hisp160_rpmsg_probe,
	.callback = hisp160_rpmsg_driver_cb,
	.remove = hisp160_rpmsg_remove,
};

#ifdef CONFIG_HISI_DEBUG_FS
static struct device_attribute hisp_ddr_freq_ctrl_attr =
    __ATTR(ddr_freq_ctrl, 0660, hisp_ddr_freq_ctrl_show, hisp_ddr_freq_store);
#endif /* CONFIG_HISI_DEBUG_FS */

static int32_t
hisp160_platform_probe(
	struct platform_device* pdev)
{
	int32_t ret = 0;

	cam_info("%s: enter", __func__);
	wake_lock_init(&hisp_power_wakelock, WAKE_LOCK_SUSPEND, "hisp_power_wakelock");
	mutex_init(&hisp_wake_lock_mutex);
	ret = hisp_get_dt_data(pdev, &s_hisp160.dt);
	if (ret < 0) {
		cam_err("%s: get dt failed.", __func__);
		goto error;
	}

	init_completion(&rpmsg_local.isp_comp);
	ret = hisp_register(pdev, &s_hisp160.intf, &s_hisp160.notify);
	if (0 == ret) {
		atomic_set(&s_hisp160.opened, 0); /*lint !e1058 */
	} else {
		cam_err("%s() %d hisp_register failed with ret %d!\n", __func__, __LINE__, ret);
		goto error;
	}

	rpmsg_local.hisi_isp_serv = NULL;

	ret = register_rpmsg_driver(&rpmsg_hisp160_driver);
	if (0 != ret){
		cam_err("%s() %d register_rpmsg_driver failed with ret %d!\n", __func__, __LINE__, ret);
		goto error;
	}

    s_hisp160.pdev = pdev;
    s_hisp160.ion_client = NULL;

#ifdef CONFIG_HISI_DEBUG_FS
	ret = device_create_file(&pdev->dev, &hisp_ddr_freq_ctrl_attr);
	if (ret < 0) {
		cam_err("%s failed to creat hisp ddr freq ctrl attribute.", __func__);
		unregister_rpmsg_driver(&rpmsg_hisp160_driver);
		hisp_unregister(s_hisp160.pdev);
		goto error;
	}
#endif
	return 0;

error:
	wake_lock_destroy(&hisp_power_wakelock);
	mutex_destroy(&hisp_wake_lock_mutex);
	cam_notice("%s exit with ret = %d\n", __func__, ret);
	return ret;
}

static struct platform_driver
s_hisp160_driver =
{
	.probe = hisp160_platform_probe,
	.driver =
	{
		.name = "huawei,hisi_isp160",
		.owner = THIS_MODULE,
		.of_match_table = s_hisp160_dt_match,
	},
};

static int __init
hisp160_init_module(void)
{
	cam_notice("%s enter\n", __func__);
	return platform_driver_register(&s_hisp160_driver);
}

static void __exit
hisp160_exit_module(void)
{
	cam_notice("%s enter\n", __func__);
	hisp_unregister(s_hisp160.pdev);
	platform_driver_unregister(&s_hisp160_driver);
	wake_lock_destroy(&hisp_power_wakelock);
	mutex_destroy(&hisp_wake_lock_mutex);
}

module_init(hisp160_init_module);
module_exit(hisp160_exit_module);
MODULE_DESCRIPTION("hisp160 driver");
MODULE_LICENSE("GPL v2");
//lint -restore
