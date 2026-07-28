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
#include <linux/hisi/hisi-iommu.h>
#include <linux/hisi/ion-iommu.h>
#include <linux/hisi/hisi_load_image.h>
#include <linux/kthread.h>
#include <linux/cpumask.h>
#include <linux/sched.h>
#include <linux/jiffies.h>
#include <linux/completion.h>
#include <teek_client_id.h>
#include "ivp_log.h"
#include "ivp_core.h"
#include "ivp_platform.h"
#include "ivp_sec.h"

struct ivp_sec_device ivp_sec_dev;
struct ivp_sec_ion_s *ivp_secmem_ion = NULL;
extern struct mutex ivp_sec_mem_mutex;
extern struct mutex ivp_power_up_off_mutex;

static int ivp_sec_load_and_verify_image(void)
{
    struct load_image_info loadinfo;
    struct ivp_sec_device *dev = (struct ivp_sec_device *)&ivp_sec_dev;
    int ret = 0;
    ivp_info("begin to load sec ivp image\n!");
    if (NULL == ivp_secmem_ion){
        ivp_err("secbuff is empty!");
        return -EINVAL;
    }

    loadinfo.ecoretype      = IVP;
    loadinfo.image_addr     = ivp_secmem_ion->sec_phymem_addr;
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

int ivp_alloc_secbuff(unsigned int size)
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
    if (IS_ERR(ivp_secmem_ion->ivp_ion_handle)){
        ivp_err("ivp_secmem_ion_alloc failed!\n");
        ion_client_destroy(ivp_secmem_ion->ivp_ion_client);
        kfree(ivp_secmem_ion);
        ivp_secmem_ion= NULL;
        mutex_unlock(&ivp_sec_mem_mutex);
        return -ENODEV;
    }
    ret = ivp_ion_phys(ivp_secmem_ion->ivp_ion_client, ivp_secmem_ion->ivp_ion_handle,(dma_addr_t *)&secmem_ion_phys);/*lint !e838*/
    if (ret < 0)
    {
        ivp_err("%s, failed to get phy addr,ret:%d!\n", __func__, ret);
        ion_free(ivp_secmem_ion->ivp_ion_client, ivp_secmem_ion->ivp_ion_handle);
        ion_client_destroy(ivp_secmem_ion->ivp_ion_client);
        kfree(ivp_secmem_ion);
        ivp_secmem_ion= NULL;
        mutex_unlock(&ivp_sec_mem_mutex);
        return -ENODEV;
    }
    if ((secmem_ion_phys & MASK_1MB)!= 0)
    {
        secmem_ion_phys = ((secmem_ion_phys/SIZE_1MB) + 1)*SIZE_1MB;
    }
    ivp_secmem_ion->sec_phymem_addr = (unsigned long)secmem_ion_phys;

    mutex_unlock(&ivp_sec_mem_mutex);
    ivp_info("alloc sec buff success!\n");
    return 0;
}

void ivp_free_secbuff(void)
{
    mutex_lock(&ivp_sec_mem_mutex);
    if (ivp_secmem_ion)
    {
        if (ivp_secmem_ion->ivp_ion_client)
        {
            ion_free(ivp_secmem_ion->ivp_ion_client, ivp_secmem_ion->ivp_ion_handle);
            ion_client_destroy(ivp_secmem_ion->ivp_ion_client);
        }
        kfree(ivp_secmem_ion);
        ivp_secmem_ion = NULL;
    }
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

    for (cpu_no = 4; cpu_no < 8; cpu_no++)
    {
        cpumask_set_cpu(cpu_no, &cpu_mask);
    }

    if(sched_setaffinity(current->pid, &cpu_mask) < 0)
    {
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
    if (!dev->secivp_kthread)
    {
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

    if (1 == atomic_read(&dev->ivp_image_success)){
        ivp_info("load success\n");
        return 0;
    }
    else
    {
        ivp_err("Failed : load fail\n");
        return -ENOSYS;
    }
}

