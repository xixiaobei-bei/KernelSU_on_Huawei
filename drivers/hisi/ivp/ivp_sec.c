
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/fcntl.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/bitops.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/ion.h>
#include <linux/hisi/hisi_ion.h>
#include <linux/hisi-iommu.h>
#include <linux/hisi/hisi_load_image.h>
#include <linux/kthread.h>
#include <linux/cpumask.h>
#include <linux/sched.h>
#include <linux/jiffies.h>
#include <linux/completion.h>
#include <linux/version.h>
#include <linux/syscalls.h>
#include <teek_client_id.h>
#include "ivp_log.h"
#include "ivp_core.h"
#include "ivp_platform.h"
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
#include <linux/ion-iommu.h>
#endif
#include "ivp_sec.h"
#define SEC_NUM_SHIFT (16)
struct ivp_sec_device ivp_sec_dev;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
struct ivp_sec_ion_s *ivp_secmem_ion = NULL;
#else
unsigned long g_ivp_sec_phymem_addr;
#endif

extern struct mutex ivp_sec_mem_mutex;
extern struct mutex ivp_power_up_off_mutex;
static int ivp_sec_load_and_verify_image(void)
{
    struct load_image_info loadinfo;
    struct ivp_sec_device *dev = (struct ivp_sec_device *)&ivp_sec_dev;
    int ret = 0;
    ivp_info("begin to load sec ivp image\n!");

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
    if (NULL == ivp_secmem_ion) {
        ivp_err("secbuff is empty!");
        return -EINVAL;
    }
    loadinfo.image_addr     = ivp_secmem_ion->sec_phymem_addr;
#else
    loadinfo.image_addr     = g_ivp_sec_phymem_addr;
#endif

    loadinfo.ecoretype      = IVP;
    loadinfo.image_size     = IVP_SEC_BUFF_SIZE;
    loadinfo.partion_name   = "ivp";

    if ((ret = bsp_load_and_verify_image(&loadinfo)) < 0) {
        ivp_err("Failed : bsp_load_and_verify_image.%d\n", ret);
        return ret;
    }

    atomic_set(&dev->ivp_image_success, 1);
    ivp_info("ivp_sec_loadimage success!");
    return 0;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
int ivp_trans_sharefd_to_phyaddr(struct device *pdev, unsigned int* buff)
{
    int ret;
    unsigned int i;
    unsigned int share_fd = 0;
    unsigned int fd_num = 0;
    unsigned int sec_fd_num = 0;
    unsigned int nosec_fd_num = 0;
    struct ion_client *ivp_ipc_fd_client;
    struct ion_handle *ivp_ion_fd_handle;
    ion_phys_addr_t ion_phy_addr = 0x0 ;
    mutex_lock(&ivp_ipc_ion_mutex);
    ivp_ipc_fd_client = hisi_ion_client_create("ivp_ipc_fd_client");
    if (IS_ERR(ivp_ipc_fd_client)) {
        ivp_err("ivp_ipc_fd_client create failed!\n");
        goto err_create_client;
    }
    //the second field is sharefd number according to the algo arg struct
    buff++;
    fd_num = *buff++;
    sec_fd_num = fd_num&0xFFFF;
    nosec_fd_num = (fd_num >> 16)&0xFFFF;
    /*fd_num indicate the followed shared_fd number, it should not exceed the
    buffer size(32), buff size = one cmd + one fdnum + fdnum*shard_fd + ..*/
    if (((sec_fd_num+nosec_fd_num) > MAX_FD_NUM)||(0 == sec_fd_num)) {
        ivp_err("ion buff number maybe wrong, num=%d\n", fd_num);
        goto err_ion_buff_num;
    }
    //trans sec buff phyaddr, phyaddr = phyaddr_begin+offset
    share_fd = *buff++;
    ivp_ion_fd_handle = ion_import_dma_buf_fd(ivp_ipc_fd_client, share_fd);
    if (IS_ERR(ivp_ion_fd_handle)) {
        ivp_err("%d, ion_import_dma_buf failed!\n", __LINE__);
        goto err_import_handle;
    }
    ret = ivp_ion_phys(pdev, ivp_ipc_fd_client, ivp_ion_fd_handle, (dma_addr_t *)&ion_phy_addr);
    if (ret < 0) {
        ivp_err("%d, ion_phys failed, result=%d\n", __LINE__, ret);
        goto err_ion_phys;
    }
    for (i = 0; i < sec_fd_num; i++) {
        *buff++ += ion_phy_addr;
    }
    ion_free(ivp_ipc_fd_client, ivp_ion_fd_handle);

    //trans nosec buff phyaddr
    for (i = 0; i < nosec_fd_num; i++) {
        share_fd = *buff;
        ivp_ion_fd_handle = ion_import_dma_buf_fd(ivp_ipc_fd_client, share_fd);
        if (IS_ERR(ivp_ion_fd_handle)){
            ivp_err("%d, ion_import_dma_buf failed!\n", __LINE__);
            goto err_import_handle;
        }
        ret = ivp_ion_phys(pdev, ivp_ipc_fd_client, ivp_ion_fd_handle, (dma_addr_t *)&ion_phy_addr);
        if (ret < 0) {
            ivp_err("%d, ion_phys failed, result=%d\n", __LINE__, ret);
            goto err_ion_phys;
        }
        *buff++ = ion_phy_addr;
        ion_free(ivp_ipc_fd_client, ivp_ion_fd_handle);
    }
    ion_client_destroy(ivp_ipc_fd_client);
    mutex_unlock(&ivp_ipc_ion_mutex);
    return ret;

err_ion_phys:
    ion_free(ivp_ipc_fd_client, ivp_ion_fd_handle);
err_ion_buff_num:
err_import_handle:
    ion_client_destroy(ivp_ipc_fd_client);
err_create_client:
    mutex_unlock(&ivp_ipc_ion_mutex);

    return -EFAULT;
}
int ivp_ion_phys(struct device *dev, struct ion_client *client, struct ion_handle *handle,dma_addr_t *addr)
{
    int ret = -ENODEV;
    int share_fd = 0;
    struct dma_buf *buf = NULL;
    struct dma_buf_attachment *attach = NULL;
    struct sg_table *sgt = NULL;
    struct scatterlist *sgl;
    struct device *device = NULL;

    ivp_dbg("[%s] +\n", __func__);

    if ((IS_ERR(client))||(IS_ERR(handle))) {
        ivp_err("hivp_ion_phys failed \n");
        return -ENODEV;
    }

    device = dev;

    share_fd = ion_share_dma_buf_fd(client, handle);
    if (share_fd < 0) {
        ivp_err("[%s] Failed : ion_share_dma_buf_fd, share_fd.%d\n", __func__, share_fd);
        return share_fd;
    }

    buf = dma_buf_get(share_fd);
    if (IS_ERR(buf)) {
        ivp_err("[%s] Failed : dma_buf_get, buf.%pK\n", __func__, buf);
        goto err_dma_buf_get;
    }

    attach = dma_buf_attach(buf, device);
    if (IS_ERR(attach)) {
        ivp_err("[%s] Failed : dma_buf_attach, attach.%pK\n", __func__, attach);
        goto err_dma_buf_attach;
    }

    sgt = dma_buf_map_attachment(attach, DMA_BIDIRECTIONAL);
    if (IS_ERR(sgt)) {
        ivp_err("[%s] Failed : dma_buf_map_attachment, sgt.%pK\n", __func__, sgt);
        goto err_dma_buf_map_attachment;
    }

    sgl = sgt->sgl;
    if (sgl == NULL) {
        ivp_err("[%s] Failed : sgl.NULL\n", __func__);
        goto err_sgl;
    }

    // Get physical addresses from scatter list
    *addr = sg_phys(sgl);/*[false alarm]:it's not the bounds of allocated memory */

    ivp_dbg("[%s] -\n", __func__);
    ret = 0;
err_sgl:
    dma_buf_unmap_attachment(attach, sgt, DMA_BIDIRECTIONAL);
err_dma_buf_map_attachment:
    dma_buf_detach(buf, attach);
err_dma_buf_attach:
    dma_buf_put(buf);
err_dma_buf_get:
    sys_close(share_fd);
    return ret;
}

int ivp_alloc_secbuff(struct device *ivp_dev, unsigned int size)
{
    ion_phys_addr_t secmem_ion_phys = 0x0 ;
    int ret = 0;

    mutex_lock(&ivp_sec_mem_mutex);
    ivp_secmem_ion = kzalloc(sizeof(struct ivp_sec_ion_s), GFP_KERNEL);/*lint !e838 */
    if (!ivp_secmem_ion) {
        ivp_err("%s ivp_secmem_ion kzalloc is failed\n", __func__);
        mutex_unlock(&ivp_sec_mem_mutex);
        return -ENODEV;
    }

    ivp_secmem_ion->ivp_ion_client = hisi_ion_client_create("ivp_ion_client");

    if (IS_ERR(ivp_secmem_ion->ivp_ion_client)) {
        ivp_err("ivp_secmem_ion_client create failed!\n");
        kfree(ivp_secmem_ion);
        ivp_secmem_ion= NULL;
        mutex_unlock(&ivp_sec_mem_mutex);
        return -ENODEV;
    }
    /* alloc sec mem */
    ivp_secmem_ion->ivp_ion_handle= ion_alloc(ivp_secmem_ion->ivp_ion_client, size+SIZE_1MB, SIZE_1M_ALIGN, (1 << ION_IRIS_HEAP_ID), ION_FLAG_SECURE_BUFFER);
    if (IS_ERR(ivp_secmem_ion->ivp_ion_handle)) {
        ivp_err("ivp_secmem_ion_alloc failed!\n");
        ion_client_destroy(ivp_secmem_ion->ivp_ion_client);
        kfree(ivp_secmem_ion);
        ivp_secmem_ion= NULL;
        mutex_unlock(&ivp_sec_mem_mutex);
        return -ENODEV;
    }
    ret = ivp_ion_phys(ivp_dev, ivp_secmem_ion->ivp_ion_client, ivp_secmem_ion->ivp_ion_handle,(dma_addr_t *)&secmem_ion_phys);/*lint !e838*/
    if (ret < 0) {
        ivp_err("%s, failed to get phy addr,ret:%d!\n", __func__, ret);
        ion_free(ivp_secmem_ion->ivp_ion_client, ivp_secmem_ion->ivp_ion_handle);
        ion_client_destroy(ivp_secmem_ion->ivp_ion_client);
        kfree(ivp_secmem_ion);
        ivp_secmem_ion= NULL;
        mutex_unlock(&ivp_sec_mem_mutex);
        return -ENODEV;
    }
    if ((secmem_ion_phys & MASK_1MB)!= 0) {
        secmem_ion_phys = ((secmem_ion_phys/SIZE_1MB) + 1)*SIZE_1MB;
    }
    ivp_secmem_ion->sec_phymem_addr = (unsigned long)secmem_ion_phys;

    mutex_unlock(&ivp_sec_mem_mutex);
    ivp_info("alloc sec buff success!\n");
    return 0;
}
#else
int ivp_trans_sharefd_to_phyaddr(struct device *pdev, unsigned int* buff)
{
    int ret;
    unsigned int i;
    unsigned int share_fd = 0;
    unsigned int fd_num = 0;
    unsigned int sec_fd_num = 0;
    unsigned int nosec_fd_num = 0;
    unsigned long ion_phy_addr = 0x0 ;
    mutex_lock(&ivp_ipc_ion_mutex);
    // the second field is sharefd number according to the algo arg struct
    buff++;
    fd_num = *buff++;
    sec_fd_num = fd_num&0xFFFF;
    nosec_fd_num = (fd_num >> SEC_NUM_SHIFT)&0xFFFF;
    /* fd_num indicate the followed shared_fd number, it should not exceed the
    buffer size(32), buff size = one cmd + one fdnum + fdnum*shard_fd + .. */
    if (((sec_fd_num + nosec_fd_num) > MAX_FD_NUM)||(sec_fd_num == 0)) {
        ivp_err("ion buff number maybe wrong, num=%d\n", fd_num);
        mutex_unlock(&ivp_ipc_ion_mutex);
        return -EFAULT;
    }
    // trans sec buff phyaddr, phyaddr = phyaddr_begin+offset
    share_fd = *buff++;
    ret = ivp_get_secbuff(share_fd,&ion_phy_addr);
    if (ret < 0) {
        ivp_err("%d, ion_phys failed, result=%d\n", __LINE__, ret);
        mutex_unlock(&ivp_ipc_ion_mutex);
        return -EFAULT;
    }
    for (i = 0; i < sec_fd_num; i++) {
        *buff++ += ion_phy_addr;
    }

    // trans nosec buff phyaddr
    for (i = 0; i < nosec_fd_num; i++) {
        share_fd = *buff;
        ret = ivp_get_secbuff(share_fd, &ion_phy_addr);
        if (ret < 0) {
            ivp_err("%d, ion_phys failed, result=%d\n", __LINE__, ret);
            mutex_unlock(&ivp_ipc_ion_mutex);
            return -EFAULT;
        }
        *buff++ = ion_phy_addr;
    }
    mutex_unlock(&ivp_ipc_ion_mutex);
    return ret;
}
int ivp_get_secbuff(int sec_buf_fd, unsigned long* sec_buf_phy_addr)
{
    struct dma_buf *buf = NULL;
    struct dma_buf_attachment *attach = NULL;
    struct sg_table *sgt = NULL;
    struct scatterlist *sgl;
    struct device *device = NULL;
    int ret = -ENODEV;

    if (sec_buf_fd < 0) {
        ivp_err("[%s] invalided sec buffer fd!\n", __func__);
        return -EINVAL;
    }

    mutex_lock(&ivp_sec_mem_mutex);

    buf = dma_buf_get(sec_buf_fd);
    if (IS_ERR(buf)) {
        ivp_err("[%s] Failed : dma_buf_get, buf.%pK\n", __func__, buf);
        goto err_dma_buf_get;
    }

    attach = dma_buf_attach(buf, device);
    if (IS_ERR(attach)) {
        ivp_err("[%s] Failed: dma_buf_attach, attach.%pK\n", __func__, attach);
        goto err_dma_buf_attach;
    }

    sgt = dma_buf_map_attachment(attach, DMA_BIDIRECTIONAL);
    if (IS_ERR(sgt)) {
        ivp_err("[%s] Failed: dma_buf_map_attachment, sgt.%pK\n", __func__, sgt);
        goto err_dma_buf_map_attachment;
    }

    sgl = sgt->sgl;
    if (sgl == NULL) {
        ivp_err("[%s] Failed: sgl NULL\n", __func__);
        goto err_sgl;
    }

    // Get physical addresses from scatter list
    *sec_buf_phy_addr= sg_phys(sgl);/* [false alarm]:it's not the bounds of allocated memory */
    ret = 0;

err_sgl:
    dma_buf_unmap_attachment(attach, sgt, DMA_BIDIRECTIONAL);
err_dma_buf_map_attachment:
    dma_buf_detach(buf, attach);
err_dma_buf_attach:
    dma_buf_put(buf);
err_dma_buf_get:
    mutex_unlock(&ivp_sec_mem_mutex);
    return ret;
}
#endif

void ivp_free_secbuff(void)
{
    mutex_lock(&ivp_sec_mem_mutex);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
    if (ivp_secmem_ion)
    {
        if (ivp_secmem_ion->ivp_ion_client) {
            ion_free(ivp_secmem_ion->ivp_ion_client, ivp_secmem_ion->ivp_ion_handle);
            ion_client_destroy(ivp_secmem_ion->ivp_ion_client);
        }
        kfree(ivp_secmem_ion);
        ivp_secmem_ion = NULL;
    }
#endif
    mutex_unlock(&ivp_sec_mem_mutex);

    return;
}

static int ivp_secwork_fn(void *data)
{
    struct ivp_sec_device *dev = (struct ivp_sec_device *)&ivp_sec_dev;
    struct cpumask cpu_mask;
    int cpu_no;

    ivp_info("+\n");

    set_user_nice(current, -10);
    cpumask_clear(&cpu_mask);

    for (cpu_no = 4; cpu_no < 8; cpu_no++) {
        cpumask_set_cpu(cpu_no, &cpu_mask);
    }

    if(sched_setaffinity(current->pid, &cpu_mask) < 0) {
        ivp_err("Couldn't set affinity to cpu\n");
    }

    while (1) {
        if (kthread_should_stop())
            break;

        wait_event(dev->secivp_wait, dev->secivp_wake);

        ivp_info("kthread load \n");
        mutex_lock(&ivp_power_up_off_mutex);
        ivp_sec_load_and_verify_image();
        mutex_unlock(&ivp_power_up_off_mutex);

        dev->secivp_wake = false;
        complete(&dev->load_completion);

    }
    ivp_info("-\n");

    return 0;
}

int ivp_create_secimage_thread(struct ivp_device *ivp_devp)
{
    struct ivp_sec_device *dev = (struct ivp_sec_device *)&ivp_sec_dev;
    init_waitqueue_head(&dev->secivp_wait);

    //create thread
    dev->secivp_kthread = kthread_create(ivp_secwork_fn, NULL, "secivpwork");
    if (IS_ERR(dev->secivp_kthread)) {
        ivp_err("Failed : kthread_create.%ld\n", PTR_ERR(dev->secivp_kthread));
        return -1;
    }

    atomic_set(&dev->ivp_image_success, 0);

    dev->secivp_wake = false;
    wake_up_process(dev->secivp_kthread);

    return 0;
}

int ivp_destroy_secimage_thread(struct ivp_device *ivp_devp)
{
    struct ivp_sec_device *dev = (struct ivp_sec_device *)&ivp_sec_dev;
    if (!dev->secivp_kthread) {
        ivp_err("Failed : secivp_kthread.%pK\n", dev->secivp_kthread);
        return -ENXIO;
    }

    kthread_stop(dev->secivp_kthread);

    return 0;
}

int ivp_sec_load(void)
{
    struct ivp_sec_device *dev = (struct ivp_sec_device *)&ivp_sec_dev;

    if (!dev->secivp_kthread) {
        ivp_err("Failed : secivp_kthread.%pK\n", dev->secivp_kthread);
        return -ENXIO;
    }
    ivp_info("begin to load\n");
    atomic_set(&dev->ivp_image_success, 0);
    init_completion(&dev->load_completion);
    dev->secivp_wake = true;
    wake_up(&dev->secivp_wait);

    if (!wait_for_completion_timeout(&dev->load_completion, msecs_to_jiffies(2000))) {
        ivp_err("Failed : timeout!\n");
        return -ETIME;
    }

    if (1 == atomic_read(&dev->ivp_image_success)) {
        ivp_info("load success\n");
        return 0;
    } else {
        ivp_err("Failed : load fail\n");
        return -ENOSYS;
    }
}

