

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
#include <media/huawei/hisp210_cfg.h>
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
#include <linux/pm_wakeup.h>
#include <linux/hisi-iommu.h>
#include <linux/platform_data/remoteproc-hisi.h>
#include <linux/iommu.h>
#include <linux/mutex.h>
#include <hicam_buf.h>
#include <securec.h>

//lint -save -e740
#define HISP_MSG_LOG_MOD 100

#define TIMEOUT_IS_FPGA_BOARD 60000
#define TIMEOUT_IS_NOT_FPGA_BOARD 15000

DEFINE_MUTEX(hisi_rpmsg_service_mutex);

static struct wakeup_source hisp_power_wakelock;
static struct mutex hisp_wake_lock_mutex;
static struct mutex hisp_power_lock_mutex;
extern void hisi_isp_boot_stat_dump(void);
extern int hisp_secmem_size_get(unsigned int *);
extern int memset_s(void *dest, size_t destMax, int c, size_t count);

typedef enum _timestamp_state_t {
    TIMESTAMP_UNINTIAL = 0,
    TIMESTAMP_INTIAL,
} timestamp_state_t;

static timestamp_state_t s_timestamp_state;
static struct timeval s_timeval;
static u32 s_system_couter_rate;
static u64 s_system_counter;

enum hisp210_rpmsg_state {
    RPMSG_UNCONNECTED,
    RPMSG_CONNECTED,
    RPMSG_FAIL,
};
enum Hisp210_Dvfs_Frequency {
    ISPFUNC_ULTLOW_FREQUENCY  = 181000000,
    ISPFUNC_LOW_FREQUENCY     = 271000000,
    ISPFUNC_NORMAL_FREQUENCY  = 325000000,
    ISPFUNC_TURBO_FREQUENCY   = 480000000,
    ISPFUNC2_ULTLOW_FREQUENCY = 163000000,
    ISPFUNC2_LOW_FREQUENCY    = 203000000,
    ISPFUNC2_NORMAL_FREQUENCY = ISPFUNC_NORMAL_FREQUENCY,
    ISPFUNC2_TURBO_FREQUENCY  = 541000000,
    ISPFUNC3_ULTLOW_FREQUENCY = ISPFUNC2_ULTLOW_FREQUENCY,
    ISPFUNC3_LOW_FREQUENCY    = ISPFUNC2_LOW_FREQUENCY,
    ISPFUNC3_NORMAL_FREQUENCY = ISPFUNC_NORMAL_FREQUENCY,
    ISPFUNC3_TURBO_FREQUENCY  = ISPFUNC_TURBO_FREQUENCY,
    ISPCPU_NORMAL_FREQUENCY   = 1200000000,
};

enum isp_clk_level {
    ISP_CLK_LEVEL_TURBO,
    ISP_CLK_LEVEL_NORMAL,
    ISP_CLK_LEVEL_LOWPOWER,
    ISP_CLK_LEVEL_ULTRALOW,
    ISP_CLK_LEVEL_MAX,
};

/*
 * These are used for distinguish the rpmsg_msg status
 * The process in hisp210_rpmsg_ept_cb are different
 * for the first receive and later.
 */
enum {
	HISP_SERV_FIRST_RECV,
	HISP_SERV_NOT_FIRST_RECV,
};

/* @brief the instance for rpmsg service
 *
 * When Histar ISP is probed, this sturcture will be initialized,
 * the object is used to send/recv rpmsg and store the rpmsg data
 *
 * @end
 */
struct rpmsg_hisp210_service {
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

// #define SUPPORT_IRIS_SMP
enum hisp210_mem_pool_attr {
	MEM_POOL_ATTR_READ_WRITE_CACHE = 0,
	MEM_POOL_ATTR_READ_WRITE_SECURITY,
	MEM_POOL_ATTR_READ_WRITE_ISP_SECURITY,
	MEM_POOL_ATTR_READ_WRITE_CACHE_OFF_LINE,
	MEM_POOL_ATTR_MAX,
};

struct hisp210_mem_pool {
    void *ap_va;
    unsigned int prot;
    unsigned int ion_iova;
    unsigned int r8_iova;
    size_t size;
    size_t align_size;
    int active;
    unsigned int security_isp_mode;
    struct sg_table *sgt;
} ;

/* @brief the instance to talk to hisp driver
 *
 * When Histar ISP is probed, this sturcture will be initialized,
 * the object is used to notify hisp driver when needed.
 *
 * @end
 */
typedef struct _tag_hisp210 {
	hisp_intf_t intf;
	hisp_notify_intf_t *notify;
	char const *name;
	atomic_t opened;
	struct platform_device*  pdev; //by used to get dts node
	hisp_dt_data_t dt;
    struct hisp210_mem_pool mem_pool[MEM_POOL_ATTR_MAX];
} hisp210_t;

struct rpmsg_service_info {
	struct rpmsg_hisp210_service *hisi_isp_serv;
	struct completion isp_comp;
	int isp_minor;
};

extern void a7_mmu_unmap(unsigned int va, unsigned int size);

/* Store the only rpmsg_hisp210_service pointer to local static rpmsg_local */
static struct rpmsg_service_info rpmsg_local;
static bool remote_processor_up = false;
static int g_hisp_ref = 0;

#define I2HI(i) container_of(i, hisp210_t, intf)

static void hisp210_notify_rpmsg_cb(void);
char const *hisp210_get_name(hisp_intf_t *i);
static int hisp210_config(hisp_intf_t *i, void *cfg);

static int hisp210_power_on(hisp_intf_t *i);
static int hisp210_power_off(hisp_intf_t *i);

static int hisp210_open(hisp_intf_t *i);
static int hisp210_close(hisp_intf_t *i);
static int hisp210_send_rpmsg(hisp_intf_t *i, hisp_msg_t *m, size_t len);
static int hisp210_recv_rpmsg(hisp_intf_t *i,
				  hisp_msg_t *user_addr, size_t len);

void hisp210_init_timestamp(void);
void hisp210_destroy_timestamp(void);
void hisp210_set_timestamp(unsigned int *timestamp_h, unsigned int *timestamp_l);
void hisp210_handle_msg(hisp_msg_t *msg);

void hisp210_init_timestamp(void)
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
		sizeof(s_timeval)/sizeof(u32)); //FIXME
}

void hisp210_destroy_timestamp(void)
{
	s_timestamp_state		= TIMESTAMP_UNINTIAL;
	s_system_counter 		= 0;
	s_system_couter_rate	= 0;
	memset(&s_timeval, 0x00, sizeof(s_timeval));
}

/* Function declaration */
/**********************************************
 * |-----pow-on------->||<----  fw-SOF ---->|
 *  timeval(got)       ----------------->fw_timeval=?
 *  system_counter(got)----------------->fw_sys_counter(got)
 *
 *  fw_timeval = timeval + (fw_sys_counter - system_counter)
 *
 * With a base position(<timeval, system_counter>, we get it at same time),
 * we can calculate fw_timeval with fw syscounter
 * and deliver it to hal. Hal then gets second and microsecond
 *********************************************/
void hisp210_set_timestamp(unsigned int *timestamp_h, unsigned int *timestamp_l)
{
/* #define NANOSECOND_PER_SECOND 	(1000000000) */

#define MICROSECOND_PER_SECOND 	(1000000)
	u64 fw_micro_second = 0;
	u64 fw_sys_counter = 0;
	u64 micro_second = 0;

	if (s_timestamp_state == TIMESTAMP_UNINTIAL) {
		cam_err("%s wouldn't enter this branch.\n", __func__);
		hisp210_init_timestamp();
	}

	if (timestamp_h == NULL || timestamp_l == NULL) {
		cam_err("%s timestamp_h or timestamp_l is null.\n", __func__);
		return;
	}

	cam_debug("%s ack_high:0x%x ack_low:0x%x", __func__,
		*timestamp_h, *timestamp_l);

	if (*timestamp_h == 0 && *timestamp_l == 0) {
		return;
	}

    fw_sys_counter = ((u64)(*timestamp_h) << 32) | // 32 for Bit operations
		(u64)(*timestamp_l); //lint !e838
	micro_second = (fw_sys_counter - s_system_counter) * MICROSECOND_PER_SECOND / s_system_couter_rate;//lint !e838

	// chang nano second to micro second
	fw_micro_second =
		(micro_second / MICROSECOND_PER_SECOND + s_timeval.tv_sec) * MICROSECOND_PER_SECOND +
		((micro_second % MICROSECOND_PER_SECOND) + s_timeval.tv_usec);//lint !e737

	*timestamp_h = (u32)(fw_micro_second >> 32 & 0xFFFFFFFF); // 32 for Bit operations
	*timestamp_l = (u32)(fw_micro_second & 0xFFFFFFFF);

	cam_debug("%s h:0x%x l:0x%x", __func__, *timestamp_h, *timestamp_l);
}

void hisp210_handle_msg(hisp_msg_t *msg)
{
	if (msg == NULL)
		return;
	switch (msg->api_name) {
		case REQUEST_RESPONSE:
			hisp210_set_timestamp(&(msg->u.ack_request.timestampH), &(msg->u.ack_request.timestampL));
			break;
		case MSG_EVENT_SENT:
			hisp210_set_timestamp(&(msg->u.event_sent.timestampH), &(msg->u.event_sent.timestampL));
			break;

		default:
			break;
	}
}

static hisp_vtbl_t s_vtbl_hisp210 = {
	.get_name = hisp210_get_name,
	.config = hisp210_config,
	.power_on = hisp210_power_on,
	.power_off = hisp210_power_off,
	.send_rpmsg = hisp210_send_rpmsg,
	.recv_rpmsg = hisp210_recv_rpmsg,
	.open = hisp210_open,
	.close = hisp210_close,
};

static hisp210_t s_hisp210 = {
	.intf = {.vtbl = &s_vtbl_hisp210,},
	.name = "hisp210",
};

static void hisp210_notify_rpmsg_cb(void)
{
	hisp_event_t isp_ev;
	isp_ev.kind = HISP_RPMSG_CB;
	hisp_notify_intf_rpmsg_cb(s_hisp210.notify, &isp_ev);
}


/* Function declaration */
/**********************************************
 * Save the rpmsg from isp to locally skb queue.
 * Only called by hisp210_rpmsg_ept_cb when api_name
 * is NOT POWER_REQ, will notify user space through HISP
 *********************************************/
static void hisp210_save_rpmsg_data(const void *data, int len)
{
	struct rpmsg_hisp210_service *hisi_serv = rpmsg_local.hisi_isp_serv;
	struct sk_buff *skb = NULL;
	char *skbdata = NULL;

	if (hisi_serv == NULL) {
		cam_err("%s: hisi_serv is NULL", __func__);
		return;
	}
	hisp_assert(data != NULL);
	hisp_assert(len > 0);

	skb = alloc_skb(len, GFP_KERNEL);
	if (!skb) {
		cam_err("%s() %d failed: alloc_skb len is %u!\n", __func__,
			__LINE__, len);
		return;
	}

	skbdata = skb_put(skb, len); /*lint !e64 */
	memcpy(skbdata, data, len);

	/* add skb to skb queue */
	mutex_lock(&hisi_serv->recv_lock);
	skb_queue_tail(&hisi_serv->queue, skb);
	mutex_unlock(&hisi_serv->recv_lock);

	wake_up_interruptible(&hisi_serv->readq);
	hisp210_notify_rpmsg_cb();
}

static int hisp210_rpmsg_ept_cb(struct rpmsg_device *rpdev,
			 void *data, int len, void *priv, u32 src)
{
	struct rpmsg_hisp210_service *hisi_serv = rpmsg_local.hisi_isp_serv;
	hisp_msg_t *msg = NULL;
	struct rpmsg_hdr *rpmsg_msg = NULL;

	if (hisi_serv == NULL) {
		cam_err("func %s: hisi_serv is NULL", __func__);
		return -EINVAL;
	}
	if (data == NULL) {
		cam_err("func %s: data is NULL", __func__);
		return -EINVAL;
	}

	hisp_assert(len > 0);

	if (hisi_serv->state != RPMSG_CONNECTED) {
		hisp_assert(hisi_serv->state == RPMSG_UNCONNECTED);
		rpmsg_msg = container_of(data, struct rpmsg_hdr, data);
		cam_info("msg src.%u, msg dst.%u\n", rpmsg_msg->src,
			  rpmsg_msg->dst);

		/* add instance dst and modify the instance state */
		hisi_serv->dst = rpmsg_msg->src;
		hisi_serv->state = RPMSG_CONNECTED;
	}

	msg = (hisp_msg_t *)(data);
	hisp_recvx(data);
	/* save the data and wait for hisp210_recv_rpmsg to get the data */
	hisp210_save_rpmsg_data(data, len);
	return 0;
}

char const *hisp210_get_name(hisp_intf_t *i)
{
	hisp210_t *hi = NULL;
	hisp_assert(i != NULL);
	hi = I2HI(i);
	if (hi == NULL) {
		cam_err("func %s: hi is NULL", __func__);
		return NULL;
	}
	return hi->name;
}

static int hisp210_unmap_a7isp_addr(void *cfg)
{
#ifdef CONFIG_HISI_CAMERA_ISP_SECURE
    struct hisp_cfg_data *pcfg = NULL;
    if (cfg == NULL) {
		cam_err("func %s: cfg is NULL", __func__);
		return -1;
	}

	pcfg = (struct hisp_cfg_data *)cfg;

    cam_info("func %s: a7 %x, size %x", __func__, pcfg->param.moduleAddr, pcfg->param.size);
    a7_mmu_unmap(pcfg->param.moduleAddr, pcfg->param.size);
    return 0;
#else
    return -ENODEV;
#endif
}

static int hisp210_get_a7isp_addr(void *cfg)
{
	int ret = -ENODEV;
#ifdef CONFIG_HISI_CAMERA_ISP_SECURE
    struct hisp_cfg_data *pcfg = NULL;
    struct scatterlist *sg = NULL;
    struct sg_table *table = NULL;
	struct dma_buf *buf = NULL;
	struct dma_buf_attachment *attach = NULL;

	if (cfg == NULL) {
		cam_err("func %s: cfg is NULL", __func__);
		return -1;
	}
	pcfg = (struct hisp_cfg_data *)cfg;

	mutex_lock(&hisi_rpmsg_service_mutex);

	ret = hisp_get_sg_table(pcfg->param.sharedFd,&(s_hisp210.pdev->dev),&buf,&attach,&table);
	if (ret < 0) {
		cam_err("func %s: get_sg_table failed", __func__);
		goto err_ion_client;
	}


    sg = table->sgl;

    pcfg->param.moduleAddr = a7_mmu_map(sg, pcfg->param.size,
                        pcfg->param.prot, pcfg->param.type);

    cam_info("func %s: a8 %x", __func__, pcfg->param.moduleAddr);
	ret = 0;
	hisp_free_dma_buf(&buf,&attach,&table);
err_ion_client:
    mutex_unlock(&hisi_rpmsg_service_mutex);
#endif
    return ret;
}

static int buffer_is_invalid(int share_fd, unsigned int req_addr, unsigned int req_size)
{
    int ret = 0;
    struct iommu_format fmt = { 0 };

    ret = hicam_buf_map_iommu(share_fd, &fmt);
    if (ret < 0) {
        cam_err("%s: fail to map iommu.", __func__);
        return ret;
    }

    if (req_addr != fmt.iova || req_size > fmt.size) {
        cam_err("%s: req_iova:%#x,  req_size:%u", __func__, req_addr, req_size);
        cam_err("%s:real_iova:%#llx, real_size:%llu.", __func__, fmt.iova, fmt.size);
        ret = -ERANGE;
    }
    hicam_buf_unmap_iommu(share_fd, &fmt);

    return ret;
}

static int find_suitable_mem_pool(struct hisp_cfg_data *pcfg)
{
    int ipool;
    if (pcfg->param.type == MAP_TYPE_RAW2YUV) {
        ipool =  MEM_POOL_ATTR_READ_WRITE_CACHE_OFF_LINE;
    } else if (pcfg->param.type == MAP_TYPE_STATIC_ISP_SEC) {
        ipool =  MEM_POOL_ATTR_READ_WRITE_ISP_SECURITY;
    } else {
        for (ipool = 0; ipool < MEM_POOL_ATTR_MAX; ipool++) {
            if (s_hisp210.mem_pool[ipool].prot == pcfg->param.prot) {
                break;
            }
        }
        if (ipool >= MEM_POOL_ATTR_MAX) {
            cam_err("func %s: no pool hit for prot:%d", __func__, pcfg->param.prot);
            return -EINVAL;
        }
    }
    return ipool;
}

static int hisp210_init_r8isp_memory_pool(void *cfg)
{
    int ipool;
    uint32_t r8va;
    struct hisp_cfg_data *pcfg;
    struct sg_table *sgt;
    enum mapType enMapType;

    if (cfg == NULL) {
        cam_err("func %s: cfg is NULL", __func__);
        return -1;
    }

    pcfg = (struct hisp_cfg_data *)cfg;
    cam_info("%s: pool cfg vaddr=0x%pK, iova=0x%x, size=0x%x", __func__,
            pcfg->param.vaddr,
            pcfg->param.iova,
            pcfg->param.size);
    cam_info("%s: type=%d, prot=0x%x sec=0x%x", __func__,
            pcfg->param.type,
            pcfg->param.prot,
            pcfg->param.security_isp_mode);

    if ((!pcfg->param.security_isp_mode) &&
            buffer_is_invalid(pcfg->param.sharedFd, pcfg->param.iova, pcfg->param.size)) {
        cam_err("check buffer fail!");
        return -EINVAL;
    }

    // find suitable mem pool
    ipool = find_suitable_mem_pool(pcfg);
    if (ipool < 0) {
        return ipool;
    }

    if (ipool == MEM_POOL_ATTR_READ_WRITE_CACHE_OFF_LINE) {
        enMapType = MAP_TYPE_RAW2YUV;
    } else {
        enMapType = pcfg->param.security_isp_mode ? MAP_TYPE_STATIC_SEC : MAP_TYPE_DYNAMIC;
    }

    /* take care of putting sgtable. */
    sgt = hicam_buf_get_sgtable(pcfg->param.sharedFd);
    if (IS_ERR_OR_NULL(sgt)) {
        cam_err("%s: fail to get sgtable.", __func__);
        return -ENOENT;
    }

    mutex_lock(&hisi_rpmsg_service_mutex);
    r8va = hisp_mem_map_setup(sgt->sgl, pcfg->param.iova, pcfg->param.size,
            pcfg->param.prot, (unsigned int)ipool, enMapType,
            (unsigned int)(pcfg->param.pool_align_size));

    if (!r8va) {
        cam_err("%s: hisp_mem_map_setup failed", __func__);
        mutex_unlock(&hisi_rpmsg_service_mutex);
        hicam_buf_put_sgtable(sgt);
        return -ENOMEM;
    }

    /* hold sg_table things, release at deinit. */
    s_hisp210.mem_pool[ipool].sgt = sgt;
    s_hisp210.mem_pool[ipool].r8_iova = r8va;
    s_hisp210.mem_pool[ipool].ap_va = pcfg->param.vaddr;
    s_hisp210.mem_pool[ipool].ion_iova = pcfg->param.iova;
    s_hisp210.mem_pool[ipool].size = pcfg->param.size;
    s_hisp210.mem_pool[ipool].align_size = pcfg->param.pool_align_size;
    s_hisp210.mem_pool[ipool].security_isp_mode = pcfg->param.security_isp_mode;

    // ion iova isn't equal r8 iova, security or unsecurity, align etc
    // return r8 iova to daemon, and send to r8 later
    pcfg->param.iova = s_hisp210.mem_pool[ipool].r8_iova;
    s_hisp210.mem_pool[ipool].active = 1;

    cam_info("func %s: r8_iova_pool_base=0x%x", __func__, s_hisp210.mem_pool[ipool].r8_iova);
    mutex_unlock(&hisi_rpmsg_service_mutex);
    return 0;
}

static int hisp210_deinit_r8isp_memory_pool(void *cfg)
{
    int ipool = 0;
    int rc;

    if (cfg == NULL) {
        cam_err("func %s: cfg is NULL", __func__);
        return -EINVAL;
    }

    ipool = find_suitable_mem_pool(cfg);
    if (ipool < 0) {
        return ipool;
    }

    mutex_lock(&hisi_rpmsg_service_mutex);
    if (s_hisp210.mem_pool[ipool].active) {
        s_hisp210.mem_pool[ipool].active = 0;
        hisp_mem_pool_destroy((unsigned int)ipool);
        /* release sg_table things. */
        hicam_buf_put_sgtable(s_hisp210.mem_pool[ipool].sgt);
    }
    rc = memset_s(&(s_hisp210.mem_pool[ipool]), sizeof(struct hisp210_mem_pool),
            0, sizeof(struct hisp210_mem_pool));
    if (rc != 0) {
        cam_warn("%s: fail for memset_s mem_pool.", __func__);
    }
    mutex_unlock(&hisi_rpmsg_service_mutex);

    return 0;
}

// handle daemon carsh
// miss ispmanager poweroff
// miss memory pool deinit
static int hisp210_deinit_r8isp_memory_pool_force(void)
{
    int ipool = 0;
    int rc;

    cam_warn("func %s", __func__);

    mutex_lock(&hisi_rpmsg_service_mutex);
    for (ipool = 0; ipool < MEM_POOL_ATTR_MAX; ipool++) {
        if (s_hisp210.mem_pool[ipool].active) {
            cam_warn("%s: force deiniting pool:%d", __func__, ipool);
            s_hisp210.mem_pool[ipool].active = 0;
            hisp_mem_pool_destroy((unsigned int)ipool);
            hicam_buf_put_sgtable(s_hisp210.mem_pool[ipool].sgt);
        }
        rc = memset_s(&(s_hisp210.mem_pool[ipool]), sizeof(struct hisp210_mem_pool),
                0, sizeof(struct hisp210_mem_pool));
        if (rc != 0) {
            cam_warn("%s: fail for memset_s mem_pool.", __func__);
        }
    }

    mutex_unlock(&hisi_rpmsg_service_mutex);

    return 0;
}

static int hisp210_alloc_r8isp_addr(void *cfg)
{
    int ipool = 0;
    unsigned int r8_iova = 0;
    size_t  offset = 0;
    struct hisp_cfg_data *pcfg = NULL;
    int rc = 0;
    bool bSecureMode = false;

    if (cfg == NULL) {
        cam_err("func %s: cfg is NULL", __func__);
        return -1;
    }

    mutex_lock(&hisi_rpmsg_service_mutex);

    pcfg = (struct hisp_cfg_data *)cfg;

    //handle static memory, just return r8 reserved iova address == map only
    if (pcfg->param.type == MAP_TYPE_STATIC) {
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

    for (ipool = 0; ipool < MEM_POOL_ATTR_MAX; ipool++) {
        if (s_hisp210.mem_pool[ipool].security_isp_mode) {
            bSecureMode = true;
            break;
        }
    }

    // hanlde dynamic memory alloc
    ipool = find_suitable_mem_pool(pcfg);
    if (ipool < 0) {
        rc = -EINVAL;
        goto alloc_err;
    }

    r8_iova = (unsigned int)hisp_mem_pool_alloc_iova(pcfg->param.size, (unsigned int)ipool);
    if (!r8_iova) {
        cam_err("func %s: hisp_mem_pool_alloc_iova error", __func__);
        rc = -ENOMEM;
        goto alloc_err;
    }

    // offset calculator
    // security mode, pool base is r8_iova, is security address, not align
    // normal mode, pool base is ion_iova, is normal address,  align by isp.
    if (pcfg->param.type == MAP_TYPE_RAW2YUV) {
        offset = r8_iova - s_hisp210.mem_pool[ipool].r8_iova;
    } else {
		offset = bSecureMode ? (r8_iova - s_hisp210.mem_pool[ipool].r8_iova) : (r8_iova - s_hisp210.mem_pool[ipool].ion_iova);
    }

    if (offset > s_hisp210.mem_pool[ipool].size) {
        cam_err("func %s: r8_iova invalid", __func__);
        rc = -EFAULT;
        goto alloc_err;
    }

    pcfg->param.vaddr = (void *)(((unsigned char *)s_hisp210.mem_pool[ipool].ap_va) + offset);
    pcfg->param.iova = r8_iova;
    pcfg->param.offset_in_pool = offset;

    mutex_unlock(&hisi_rpmsg_service_mutex);
    return 0;

alloc_err:
    mutex_unlock(&hisi_rpmsg_service_mutex);
    return rc;
}

static int hisp210_free_r8isp_addr(void *cfg)
{
    int rc = 0;
    int ipool = 0;
    struct hisp_cfg_data *pcfg = NULL;

    if (cfg == NULL) {
        cam_err("func %s: cfg is NULL", __func__);
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
			cam_err("func %s: hisp_mem_pool_free_carveout error", __func__);
		}
		mutex_unlock(&hisi_rpmsg_service_mutex);
		return 0;
	}

    // hanlde dynamic memory alloc
    ipool = find_suitable_mem_pool(pcfg);
    if (ipool < 0) {
        rc = -EFAULT;
        goto free_err;
    }

    rc = (int)hisp_mem_pool_free_iova((unsigned int)ipool, pcfg->param.iova, pcfg->param.size);
    if (rc) {
	cam_err("func %s: hisp_mem_pool_free_iova error", __func__);
        rc = -EFAULT;
        goto free_err;
    }

    mutex_unlock(&hisi_rpmsg_service_mutex);
    return 0;

free_err:
    mutex_unlock(&hisi_rpmsg_service_mutex);
    return rc;
}

static int hisp210_mem_pool_pre_init(void)
{
    int ipool = 0;
    int prot = 0;
    int rc;

    for (ipool = 0; ipool < MEM_POOL_ATTR_MAX; ipool++) {
        rc = memset_s(&(s_hisp210.mem_pool[ipool]), sizeof(struct hisp210_mem_pool),
                0, sizeof(struct hisp210_mem_pool));
        if (rc != 0) {
            cam_warn("%s: fail for memset_s mem_pool.", __func__);
        }

        switch (ipool) {
            case MEM_POOL_ATTR_READ_WRITE_CACHE:
            case MEM_POOL_ATTR_READ_WRITE_CACHE_OFF_LINE:
                prot = IOMMU_READ | IOMMU_WRITE|IOMMU_CACHE;
                break;

            case MEM_POOL_ATTR_READ_WRITE_SECURITY:
			case MEM_POOL_ATTR_READ_WRITE_ISP_SECURITY:
				prot = IOMMU_READ | IOMMU_WRITE|IOMMU_CACHE|IOMMU_SEC;
                break;

            default:
                prot = -1;
                break;
        }

        cam_info("%s  ipool %d prot 0x%x", __func__, ipool, prot);

        if (prot < 0) {
            cam_err("%s unkown ipool %d prot 0x%x", __func__, ipool, prot);
            return -EINVAL;
        }

        s_hisp210.mem_pool[ipool].prot = (unsigned int)prot;
    }

    return 0;
}

static int hisp210_mem_pool_later_deinit(void)
{
    int ipool = 0;
    int rc;
    cam_debug("%s", __func__);

    for (ipool = 0; ipool < MEM_POOL_ATTR_MAX; ipool++) {
        if (ipool == MEM_POOL_ATTR_READ_WRITE_CACHE_OFF_LINE) {
            continue;
        }
        if (s_hisp210.mem_pool[ipool].active) {
            cam_warn("%s: force deiniting pool:%d", __func__, ipool);
            s_hisp210.mem_pool[ipool].active = 0;
            hisp_mem_pool_destroy((unsigned int)ipool);
            hicam_buf_put_sgtable(s_hisp210.mem_pool[ipool].sgt);
        }
        rc = memset_s(&s_hisp210.mem_pool[ipool], sizeof(struct hisp210_mem_pool),
                0, sizeof(struct hisp210_mem_pool));
        if (rc != 0) {
            cam_warn("%s: fail for memset_s mem_pool.", __func__);
        }
    }

    return 0;
}

static int hisp210_set_clk_rate(int clk_level)
{
    int rc = 0;
    int rc0 = 0;
    int rc1 = 0;
    int rc2 = 0;

    switch (clk_level) {
    case ISP_CLK_LEVEL_TURBO:
        rc0 = hisp_set_clk_rate(ISPFUNC_CLK,  ISPFUNC_TURBO_FREQUENCY);
        rc1 = hisp_set_clk_rate(ISPFUNC2_CLK, ISPFUNC2_TURBO_FREQUENCY);
        rc2 = hisp_set_clk_rate(ISPFUNC3_CLK, ISPFUNC3_TURBO_FREQUENCY);
        break;
    case ISP_CLK_LEVEL_NORMAL:
        rc0 = hisp_set_clk_rate(ISPFUNC_CLK,  ISPFUNC_NORMAL_FREQUENCY);
        rc1 = hisp_set_clk_rate(ISPFUNC2_CLK, ISPFUNC2_NORMAL_FREQUENCY);
        rc2 = hisp_set_clk_rate(ISPFUNC3_CLK, ISPFUNC3_NORMAL_FREQUENCY);
        break;
    case ISP_CLK_LEVEL_LOWPOWER:
        rc0 = hisp_set_clk_rate(ISPFUNC_CLK,  ISPFUNC_LOW_FREQUENCY);
        rc1 = hisp_set_clk_rate(ISPFUNC2_CLK, ISPFUNC2_LOW_FREQUENCY);
        rc2 = hisp_set_clk_rate(ISPFUNC3_CLK, ISPFUNC3_LOW_FREQUENCY);
        break;
    case ISP_CLK_LEVEL_ULTRALOW:
        rc0 = hisp_set_clk_rate(ISPFUNC_CLK,  ISPFUNC_ULTLOW_FREQUENCY);
        rc1 = hisp_set_clk_rate(ISPFUNC2_CLK, ISPFUNC2_ULTLOW_FREQUENCY);
        rc2 = hisp_set_clk_rate(ISPFUNC3_CLK, ISPFUNC3_ULTLOW_FREQUENCY);
        break;
	default:
        break;
    }

    if (rc0 < 0 || rc1 < 0 || rc2 < 0) {
        cam_err("%s: set clk fail, rc:%d, %d, %d", __func__, rc0, rc1, rc2);
        rc = -EFAULT;
    }
    return rc;
}

static int hisp210_config(hisp_intf_t *i, void *cfg)
{
    int rc = 0;
    hisp210_t *hi = NULL;
    struct hisp_cfg_data *pcfg = NULL;

    hisp_assert(i != NULL);
    if (cfg == NULL) {
        cam_err("func %s: cfg is NULL", __func__);
        return -1;
    }
    pcfg = (struct hisp_cfg_data *)cfg;
    hi = I2HI(i);
    hisp_assert(hi != NULL);

    switch (pcfg->cfgtype) {
    case HISP_CONFIG_POWER_ON:
        mutex_lock(&hisp_power_lock_mutex);
        if (!remote_processor_up) {
            if (pcfg->isSecure == 0) {
                hisi_isp_rproc_case_set(NONSEC_CASE);
            } else if (pcfg->isSecure == 1) {
                hisi_isp_rproc_case_set(SEC_CASE);
            } else {
                cam_info("%s invalid mode\n", __func__);
            }
            cam_info("%s power on the hisp210.\n", __func__);
            rc = hisp210_power_on(i);
        } else {
        cam_warn("%s hisp210 is still on power-on state, power off it.\n",
            __func__);

            rc = hisp210_power_off(i);
            if (rc != 0) {
                mutex_unlock(&hisp_power_lock_mutex);
                break;
            }

            hisp210_deinit_r8isp_memory_pool_force();
            cam_warn("%s begin to power on the hisp210.\n", __func__);
            rc = hisp210_power_on(i);
         }
        mutex_unlock(&hisp_power_lock_mutex);
        break;
    case HISP_CONFIG_POWER_OFF:
        mutex_lock(&hisp_power_lock_mutex);
        if (remote_processor_up) {
            cam_notice("%s power off the hisp210.\n", __func__);
            rc = hisp210_power_off(i);
        }
        mutex_unlock(&hisp_power_lock_mutex);
        break;
    case HISP_CONFIG_GET_MAP_ADDR:
        rc = hisp210_get_a7isp_addr(cfg);
        break;
    case HISP_CONFIG_UNMAP_ADDR:
        cam_notice("%s unmap a7 address from isp atf\n", __func__);
        rc = hisp210_unmap_a7isp_addr(cfg);
        break;

    case HISP_CONFIG_INIT_MEMORY_POOL:
        rc = hisp210_init_r8isp_memory_pool(cfg);
        break;

    case HISP_CONFIG_DEINIT_MEMORY_POOL:
        rc = hisp210_deinit_r8isp_memory_pool(cfg);
        break;

    case HISP_CONFIG_ALLOC_MEM:
        rc = hisp210_alloc_r8isp_addr(cfg);
        break;

    case HISP_CONFIG_FREE_MEM:
        rc = hisp210_free_r8isp_addr(cfg);
        break;

    // Func->>FE Func2->>SRT Func3->>CRAW/CBE
    case HISP_CONFIG_ISP_TURBO:
        cam_info("%s HISP_CONFIG_ISP_TURBO", __func__);
        rc = hisp210_set_clk_rate(ISP_CLK_LEVEL_TURBO);
        break;
    case HISP_CONFIG_ISP_NORMAL:
        cam_info("%s HISP_CONFIG_ISP_NORMAL", __func__);
        rc = hisp210_set_clk_rate(ISP_CLK_LEVEL_NORMAL);
        break;
    case HISP_CONFIG_ISP_LOWPOWER:
        cam_info("%s HISP_CONFIG_ISP_LOWPOWER", __func__);
        rc = hisp210_set_clk_rate(ISP_CLK_LEVEL_LOWPOWER);
        break;
    case HISP_CONFIG_ISP_ULTRALOW:
        cam_info("%s HISP_CONFIG_ISP_ULTRALOW", __func__);
        rc = hisp210_set_clk_rate(ISP_CLK_LEVEL_ULTRALOW);
        break;
    case HISP_CONFIG_R8_TURBO:
    case HISP_CONFIG_R8_NORMAL:
    case HISP_CONFIG_R8_LOWPOWER:
    case HISP_CONFIG_R8_ULTRALOW:
        cam_info("%s HISP_CONFIG_R8_CLK", __func__);
        rc = hisp_set_clk_rate(ISPCPU_CLK,  ISPCPU_NORMAL_FREQUENCY);
        break;
    case HISP_CONFIG_PROC_TIMEOUT:
        hisp_dump_rpmsg_with_id(pcfg->cfgdata[0]);
        break;
    case HISP_CONFIG_GET_SEC_ISPFW_SIZE:
        rc = hisp_secmem_size_get(&pcfg->buf_size);
        break;
    case HISP_CONFIG_SET_SEC_ISPFW_BUFFER:
        rc = hisp_set_sec_fw_buffer(cfg);
        break;
    case HISP_CONFIG_RELEASE_SEC_ISPFW_BUFFER:
        rc = hisp_release_sec_fw_buffer();
        break;
    default:
        cam_err("%s: unsupported cmd:%#x", __func__, pcfg->cfgtype);
        break;
    }
    if (rc < 0) {
        cam_err("%s: cmd:%#x fail, rc:%u", __func__, pcfg->cfgtype, rc);
    }
    return rc;
}

static int hisp210_open(hisp_intf_t *i)
{
	cam_info("%s hisp210 device open! \n", __func__);

	mutex_lock(&hisp_power_lock_mutex);
	g_hisp_ref++;
	mutex_unlock(&hisp_power_lock_mutex);
	return 0;
}

static int hisp210_close(hisp_intf_t *i)
{
	int rc = 0;
	cam_info("%s hisp210 device close! \n", __func__);
	mutex_lock(&hisp_power_lock_mutex);

	if (g_hisp_ref) {
		g_hisp_ref--;
	}

	if ((g_hisp_ref == 0) && remote_processor_up) {
		cam_warn("%s hisp210 is still on power-on state, power off it.\n",
				__func__);

		rc = hisp210_power_off(i);
		if (rc != 0) {
			cam_err("failed to hisp210 power off! \n");
		}

		hisp210_deinit_r8isp_memory_pool_force();
	}
	mutex_unlock(&hisp_power_lock_mutex);
	return rc;
}
static int hisp210_power_on(hisp_intf_t *i)
{
	int rc = 0;
	bool rproc_enabled = false;
	bool hi_opened = false;
	hisp210_t *hi = NULL;
	unsigned long current_jiffies = jiffies;
	uint32_t timeout = hw_is_fpga_board() ? TIMEOUT_IS_FPGA_BOARD : TIMEOUT_IS_NOT_FPGA_BOARD;

	struct rpmsg_hisp210_service *hisi_serv = NULL;
	struct rpmsg_channel_info chinfo = {
		.src = RPMSG_ADDR_ANY,
	};
	if (i == NULL) {
		return -1;
	}
	hi = I2HI(i);

	cam_info("%s enter ....\n", __func__);

	mutex_lock(&hisp_wake_lock_mutex);
	if (!hisp_power_wakelock.active) {
		__pm_stay_awake(&hisp_power_wakelock);
		cam_info("%s hisp power on enter, wake lock\n", __func__);
	}
	mutex_unlock(&hisp_wake_lock_mutex); /*lint !e456 */

	mutex_lock(&hisi_rpmsg_service_mutex);
	if (!atomic_read((&hi->opened))) {
		if (!hw_is_fpga_board()) {
			if (!IS_ERR(hi->dt.pinctrl_default)) {
				rc = pinctrl_select_state(hi->dt.pinctrl, hi->dt.pinctrl_default);
				if (rc != 0) {
					goto failed_ret;
				}
			}
		}

        hisp_rpmsgrefs_reset();
		rc = hisi_isp_rproc_enable();
		if (rc != 0) {
			goto failed_ret;
		}
		rproc_enabled = true;

		rc  = wait_for_completion_timeout(&channel_sync, msecs_to_jiffies(timeout));
		if (rc == 0) {
			rc = -ETIME;
			hisi_isp_boot_stat_dump();
			goto failed_ret;
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
		goto failed_ret;
	}

	/* assign a new, unique, local address and associate instance with it */
#pragma GCC visibility push(default)
	hisi_serv->ept =
		rpmsg_create_ept(hisi_serv->rpdev, hisp210_rpmsg_ept_cb, hisi_serv,
				 chinfo);
#pragma GCC visibility pop

	if (!hisi_serv->ept) {
		rc = -ENOMEM;
		goto failed_ret;
	}
	cam_info("%s() %d hisi_serv->rpdev:src.%d, dst.%d\n",
			__func__, __LINE__,
			hisi_serv->rpdev->src, hisi_serv->rpdev->dst);
	hisi_serv->state = RPMSG_CONNECTED;

	/* set the instance recv_count */
	hisi_serv->recv_count = HISP_SERV_FIRST_RECV;

	hisp210_init_timestamp();


    if (hisp210_mem_pool_pre_init()) {
        cam_err("failed to pre init mem pool \n");
        rc = -ENOMEM;
        goto failed_ret;
    }

	mutex_unlock(&hisi_rpmsg_service_mutex);
	cam_info("%s exit ,power on time:%d....\n", __func__,
			jiffies_to_msecs(jiffies - current_jiffies) );
	return rc; /*lint !e454 */

failed_ret:
	if (hi_opened) {
		atomic_dec(&hi->opened);
	}

	if (rproc_enabled) {
		hisi_isp_rproc_disable();
               rproc_set_sync_flag(true);
	}

    hisp210_mem_pool_later_deinit();
	remote_processor_up = false;

	mutex_unlock(&hisi_rpmsg_service_mutex);

	mutex_lock(&hisp_wake_lock_mutex);
	if (hisp_power_wakelock.active) {
		__pm_relax(&hisp_power_wakelock);
		cam_err("%s hisp power on failed, wake unlock\n", __func__);
	}
	mutex_unlock(&hisp_wake_lock_mutex); /*lint !e456 */
	return rc; /*lint !e454 */
} /*lint !e454 */

static int hisp210_power_off(hisp_intf_t *i)
{
	int rc = 0;
	hisp210_t *hi = NULL;
	unsigned long current_jiffies = jiffies;
	struct rpmsg_hisp210_service *hisi_serv = NULL;
	if (i == NULL) {
		return -1;
	}
	hi = I2HI(i);

	cam_info("%s enter ....\n", __func__);

	/*check the remote processor boot flow */
	if (remote_processor_up == false) {
		rc = -EPERM;
		goto ret;
	}

	hisi_serv = rpmsg_local.hisi_isp_serv;
	if (!hisi_serv) {
		rc = -ENODEV;
		goto ret;
	}

	if (hisi_serv->state == RPMSG_FAIL) {
		rc = -EFAULT;
		goto ret;
	}

	mutex_lock(&hisi_rpmsg_service_mutex);

	if (!hisi_serv->ept) {
		rc = -ENODEV;
		goto unlock_ret;
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
				if (rc != 0) {
					// Empty.
				}
			}
		}

		remote_processor_up = false;
		atomic_dec(&hi->opened);
	} else {
		cam_notice("%s isp hasn't been opened.\n", __func__);
	}

	hisp210_destroy_timestamp();
unlock_ret:
    hisp210_mem_pool_later_deinit();

	mutex_unlock(&hisi_rpmsg_service_mutex);
ret:
	cam_info("%s exit ,power 0ff time:%d....\n", __func__,
			jiffies_to_msecs(jiffies - current_jiffies) );

	mutex_lock(&hisp_wake_lock_mutex);
	if (hisp_power_wakelock.active) {
		__pm_relax(&hisp_power_wakelock);
		cam_info("%s hisp power off exit, wake unlock\n", __func__);
	}
	mutex_unlock(&hisp_wake_lock_mutex); /*lint !e456 */
	return rc;
}

static void hisp210_rpmsg_remove(struct rpmsg_device *rpdev)
{
	struct rpmsg_hisp210_service *hisi_serv = dev_get_drvdata(&rpdev->dev);

	cam_info("%s enter ....\n", __func__);

	if (hisi_serv == NULL) {
		cam_err("%s: hisi_serv == NULL!", __func__);
		return;
	}

	mutex_destroy(&hisi_serv->send_lock);
	mutex_destroy(&hisi_serv->recv_lock);

	kfree(hisi_serv);
	rpmsg_local.hisi_isp_serv = NULL;
	cam_notice("rpmsg hisi driver is removed\n");
}

static int
hisp210_rpmsg_driver_cb(struct rpmsg_device *rpdev,
			void *data, int len, void *priv, u32 src)
{
	cam_info("%s enter ....\n", __func__);
	cam_warn("%s() %d uhm, unexpected message!\n", __func__,
		__LINE__);

	print_hex_dump(KERN_DEBUG, __func__, DUMP_PREFIX_NONE, 16, 1, // 16 for lenth
			   data, len, true);
	return 0;
}

static int
hisp210_send_rpmsg(hisp_intf_t *i, hisp_msg_t *from_user, size_t len)
{
	int rc = 0;
	hisp210_t *hi = NULL;
	struct rpmsg_hisp210_service *hisi_serv = NULL;
	hisp_msg_t *msg = from_user;
	hisp_assert(i != NULL);
	hisp_assert(from_user != NULL);
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
		goto ret;
	}

	if (!hisi_serv->ept) {
		cam_err("%s() %d failed:hisi_serv->ept does not exist!\n", __func__,
				__LINE__);
		rc = -ENODEV;
		goto ret;
	}

	mutex_lock(&hisi_serv->send_lock);
	/*if the msg is the first msg, let's treat it special */
	if (RPMSG_CONNECTED != hisi_serv->state) {
		if (!hisi_serv->rpdev) {
			cam_err("%s() %d failed:hisi_serv->rpdev does not exist!\n", __func__,
				__LINE__);
			rc = -ENODEV;
			goto unlock_ret;
		}
		hisp_sendin(msg);
		rc = rpmsg_send_offchannel(hisi_serv->ept,
					   hisi_serv->ept->addr,
					   hisi_serv->rpdev->dst, (void *)msg,
					   len);

		if (rc) {
			cam_err("%s() %d failed: first rpmsg_send_offchannel ret is %d!\n", __func__,
				__LINE__, rc);
		}
		goto unlock_ret;
	}
	hisp_sendin(msg);
	rc = rpmsg_send_offchannel(hisi_serv->ept, hisi_serv->ept->addr,
				   hisi_serv->dst, (void *)msg, len);

	if (rc) {
		cam_err("%s() %d failed: rpmsg_send_offchannel ret is %d!\n", __func__,
			__LINE__, rc);
		goto unlock_ret;
	}

unlock_ret:
	mutex_unlock(&hisi_serv->send_lock);
ret:
	return rc;
}

static int
hisp210_recv_rpmsg(hisp_intf_t *i, hisp_msg_t *user_addr, size_t len)
{
	int rc = len;
	hisp210_t *hi = NULL;
	struct rpmsg_hisp210_service *hisi_serv = NULL;
	struct sk_buff *skb = NULL;
	hisp_msg_t *msg = NULL;
	hisp_assert(i != NULL);
	if (user_addr == NULL) {
		cam_err("func %s: user_addr is NULL", __func__);
		return -1;
	}
	hi = I2HI(i);

	cam_debug("%s enter. \n", __func__);

	hisi_serv = rpmsg_local.hisi_isp_serv;
	if (!hisi_serv) {
		cam_err("%s() %d failed: hisi_serv does not exist!\n", __func__,
			__LINE__);
		rc = -ENODEV;
		goto ret;
	}

	if (hisi_serv->recv_count == HISP_SERV_FIRST_RECV) {
		hisi_serv->recv_count = HISP_SERV_NOT_FIRST_RECV;
	}

	if (mutex_lock_interruptible(&hisi_serv->recv_lock)) {
		cam_err("%s() %d failed: mutex_lock_interruptible!\n", __func__,
			__LINE__);
		rc = -ERESTARTSYS;
		goto ret;
	}

	if (hisi_serv->state != RPMSG_CONNECTED) {
		cam_err("%s() %d hisi_serv->state != RPMSG_CONNECTED!\n",
			__func__, __LINE__);
		rc = -ENOTCONN;
		goto unlock_ret;
	}

	/* nothing to read ? */
	/* check if skb_queue is NULL ? */
	if (skb_queue_empty(&hisi_serv->queue)) {
		mutex_unlock(&hisi_serv->recv_lock); /*lint !e455 */
		cam_err("%s() %d skb_queue is empty!\n", __func__, __LINE__);

		/* otherwise block, and wait for data */
		if (wait_event_interruptible_timeout(hisi_serv->readq,
						 (!skb_queue_empty
						  (&hisi_serv->queue)
						  || hisi_serv->state ==
						  RPMSG_FAIL),
						  msecs_to_jiffies(HISP_WAIT_TIMEOUT))) { //lint !e666 !e665
			cam_err("%s() %d hisi_serv->state = %d!\n", __func__,
					__LINE__, hisi_serv->state);
			rc = -ERESTARTSYS;
			goto ret;
		}

		if (mutex_lock_interruptible(&hisi_serv->recv_lock)) {
			cam_err("%s() %d failed: mutex_lock_interruptible!\n",
					__func__, __LINE__);
			rc = -ERESTARTSYS;
			goto ret;
		}
	}

	if (hisi_serv->state == RPMSG_FAIL) {
		cam_err("%s() %d state = RPMSG_FAIL!\n", __func__, __LINE__);
		rc = -ENXIO;
		goto unlock_ret;
	}

	skb = skb_dequeue(&hisi_serv->queue);
	if (!skb) {
		cam_err("%s() %d skb is NULL!\n", __func__, __LINE__);
		rc = -EIO;
		goto unlock_ret;
	}

	rc = min((unsigned int)len, skb->len);
	msg = (hisp_msg_t *) (skb->data);
	hisp_recvdone((void *)msg);
	if (msg->api_name == ISP_CPU_POWER_OFF_RESPONSE) {
		hisp_rpmsgrefs_dump();
	}
	cam_debug("%s: api_name(0x%x)\n", __func__, msg->api_name);

	if (msg->message_id % HISP_MSG_LOG_MOD == 0) {
		cam_info("%s: api_name:%#x, message_id:%#x\n", __FUNCTION__,
				msg->api_name, msg->message_id);
	}

	hisp210_handle_msg(msg);
	if (!memcpy(user_addr, msg, rc)) {
		rc = -EFAULT;
		cam_err("Fail: %s()%d ret = %d\n", __func__, __LINE__, rc);
	}
	kfree_skb(skb);

unlock_ret:
	mutex_unlock(&hisi_serv->recv_lock); /*lint !e455 */
ret:
	return rc;
}
static int32_t hisp210_rpmsg_probe(struct rpmsg_device *rpdev)
{
	int32_t ret = 0;
	struct rpmsg_hisp210_service *hisi_serv = NULL;
	cam_info("%s enter\n", __func__);

	if (rpmsg_local.hisi_isp_serv != NULL) {
		cam_notice("%s hisi_serv is already up!\n", __func__);
		goto server_up;
	}

	hisi_serv = kzalloc(sizeof(*hisi_serv), GFP_KERNEL);
	if (!hisi_serv) {
		cam_err("%s() %d kzalloc failed!\n", __func__, __LINE__);
		ret = -ENOMEM;
		goto error_ret;
	}
	mutex_init(&hisi_serv->send_lock);
	mutex_init(&hisi_serv->recv_lock);
	skb_queue_head_init(&hisi_serv->queue);
	init_waitqueue_head(&hisi_serv->readq);
	hisi_serv->ept = NULL;
	hisi_serv->comp = &rpmsg_local.isp_comp;

	rpmsg_local.hisi_isp_serv = hisi_serv;
server_up:
	if (hisi_serv == NULL) {
		cam_err("func %s: hisi_serv is NULL", __func__);
		return -1;
	}
	hisi_serv->rpdev = rpdev;
	hisi_serv->state = RPMSG_UNCONNECTED;
	dev_set_drvdata(&rpdev->dev, hisi_serv);

	complete(hisi_serv->comp);

	cam_info("new HISI connection srv channel: %u -> %u!\n",
						rpdev->src, rpdev->dst);
error_ret:
	return ret;
}

static struct rpmsg_device_id rpmsg_hisp210_id_table[] = {
	{.name = "rpmsg-hisi"},
	{},
};

MODULE_DEVICE_TABLE(platform, rpmsg_hisp210_id_table);

static const struct of_device_id s_hisp210_dt_match[] = {
	{
	 .compatible = "huawei,hisi_isp210",
	 .data = &s_hisp210.intf,
	 },
	{
	 },
};

MODULE_DEVICE_TABLE(of, s_hisp210_dt_match);
#pragma GCC visibility push(default)
static struct rpmsg_driver rpmsg_hisp210_driver = {
	.drv.name   = KBUILD_MODNAME, //lint !e64 !e485
	.drv.owner  = THIS_MODULE, //lint !e64 !e485
	.id_table = rpmsg_hisp210_id_table,
	.probe = hisp210_rpmsg_probe,
	.callback = hisp210_rpmsg_driver_cb,
	.remove = hisp210_rpmsg_remove,
};
#pragma GCC visibility pop

static int32_t
hisp210_platform_probe(
	struct platform_device* pdev)
{
	int32_t ret = 0;

	cam_info("%s: enter", __func__);
	wakeup_source_init(&hisp_power_wakelock, "hisp_power_wakelock");
	mutex_init(&hisp_wake_lock_mutex);
	mutex_init(&hisp_power_lock_mutex);
	ret = hisp_get_dt_data(pdev, &s_hisp210.dt);
	if (ret < 0) {
		cam_err("%s: get dt failed.", __func__);
		goto error;
	}

	init_completion(&rpmsg_local.isp_comp);
	ret = hisp_register(pdev, &s_hisp210.intf, &s_hisp210.notify);
	if (ret == 0) {
		atomic_set(&s_hisp210.opened, 0); /*lint !e1058 */
	} else {
		cam_err("%s() %d hisp_register failed with ret %d!\n", __func__, __LINE__, ret);
		goto error;
	}

	rpmsg_local.hisi_isp_serv = NULL;

	ret = register_rpmsg_driver(&rpmsg_hisp210_driver);
	if (ret != 0) {
		cam_err("%s() %d register_rpmsg_driver failed with ret %d!\n", __func__, __LINE__, ret);
		goto error;
	}

    s_hisp210.pdev = pdev;
	return 0;

error:
	wakeup_source_trash(&hisp_power_wakelock);
	mutex_destroy(&hisp_wake_lock_mutex);
	mutex_destroy(&hisp_power_lock_mutex);
	cam_notice("%s exit with ret = %d\n", __func__, ret);
	return ret;
}

static struct platform_driver
s_hisp210_driver =
{
	.probe = hisp210_platform_probe,
	.driver =
	{
		.name = "huawei,hisi_isp210",
		.owner = THIS_MODULE,
		.of_match_table = s_hisp210_dt_match,
	},
};

static int __init
hisp210_init_module(void)
{
	cam_info("%s enter\n", __func__);
	return platform_driver_register(&s_hisp210_driver);
}

static void __exit
hisp210_exit_module(void)
{
	cam_info("%s enter\n", __func__);
	hisp_unregister(s_hisp210.pdev);
	platform_driver_unregister(&s_hisp210_driver);
	wakeup_source_trash(&hisp_power_wakelock);
	mutex_destroy(&hisp_wake_lock_mutex);
}

module_init(hisp210_init_module);
module_exit(hisp210_exit_module);
MODULE_DESCRIPTION("hisp210 driver");
MODULE_LICENSE("GPL v2");
//lint -restore
