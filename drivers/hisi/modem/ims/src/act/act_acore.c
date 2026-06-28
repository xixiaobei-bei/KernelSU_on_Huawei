#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/list.h>

//BSP
//#include "bsp_icc.h"
#include "mdrv_icc.h"

#define BUFFERSIZE 256
#define RCSSRVBUFSIZE 3796
#define DEVICE_MAJOR 250 /*设置一个主设备号*/
#define DEVICE_MINOR 0   /*次设备号*/

//#define VT_AP_CP_CHID  ((ICC_CHN_GUOM0 << 16) || GUOM0_FUNC_ID_VT_AC)

#define VT_AP_CP_CHID  MDRV_ICC_VT_VOIP
#define ACT_DEV_NUM	2
#define ACT_DEV_VOLTE_INSTANCE 0
#define ACT_DEV_RCS_INSTANCE 1


/*----------------------------------------------*
 * Global Variable                                    *
 *----------------------------------------------*/
//osl_sem_id g_ActSemId;               //tunnel signal


//static bool hasData = false;

struct act_cdev_data {
    struct list_head msg_list;
    u32 len;
    u8  data[];
};

struct act_instance {
    bool hasData;
    u32 len;
    u32 iChannelID;
    u8 *ucData;
    wait_queue_head_t inq;
    spinlock_t list_lock;//新增一个成员，定义一个spin锁,访问链表时需要加锁
    struct list_head msg_list;
    struct act_cdev *act_cdev;
};

struct act_cdev {
    struct cdev cdev; /*cdev结构体，与字符设备对应*/
    struct act_instance stInstance[2];
};

struct act_cdev *act_cdevp     = NULL; /*设备结构体指针*/
static struct class *act_class = NULL;
static int act_major = 0;

/*lint -e429*/
s32 act_rcsSrvMsgProc(u32 channel_id , u32 len, void* context)
{
    struct act_cdev_data *data = NULL;
    unsigned long flags    = 0;
    int ret                = 0;

    printk(KERN_INFO "Enter act_rcsSrvMsgProc len(%u).\n", len);

    if (len > RCSSRVBUFSIZE || 0 == len)
    {
       printk(KERN_ERR "the length(%d) received exceed the MAX RCSSRVBUFSIZE.\n", len);
       return -1;
    }

    //分配链表内存kmalloc，用于存储数据
    data = (struct act_cdev_data*)kmalloc(sizeof(struct act_cdev_data) + len, GFP_KERNEL);
    if (!data)
    {
        ret = -ENOMEM;
        printk(KERN_ERR "act_rcsSrvMsgProc: kmalloc data failed.\n");
        return ret;
    }

    ret = mdrv_icc_read(channel_id, data->data, len);
    if(ret == len)
    {
        data->len = len;
    }
    else
    {
        printk(KERN_ERR "act_rcsSrvMsgProc: mdrv_icc_read error ret: 0x%x.\n",ret);
        kfree(data);
        return -1;
    }
    
    
    //获取信号量 spinlock_irq_save(lock)
    spin_lock_irqsave(&(act_cdevp->stInstance[1].list_lock), flags);

    //挂接到链表，spinlick_irq_restore释放锁
    list_add_tail(&(data->msg_list), &(act_cdevp->stInstance[1].msg_list)); //应该是list_add_tail

    //释放信号量
    spin_unlock_irqrestore(&(act_cdevp->stInstance[1].list_lock), flags);
    act_cdevp->stInstance[1].hasData = true;
    wake_up(&(act_cdevp->stInstance[1].inq)); //wake up the read process
    
    printk(KERN_INFO "End act_rcsSrvMsgProc result(%d).\n", ret);

    return ret;

}
/*lint +e429*/

/*lint -e429*/
s32 act_msgProc(u32 channel_id , u32 len, void* context)
{
    struct act_cdev_data *data = NULL;
    unsigned long flags    = 0;
    int ret                = 0;

    printk(KERN_INFO "Enter act_msgProc len(%d).\n", len);

    if (len > BUFFERSIZE)
    {
       printk(KERN_ERR "the length(%d) received exceed the MAX BUFFERSIZE.\n", len);
       return -1;
    }

    //分配链表内存kmalloc，用于存储数据
    data = (struct act_cdev_data*)kmalloc(sizeof(struct act_cdev_data) + len, GFP_KERNEL);
    if (!data)
    {
        ret = -ENOMEM;
        printk(KERN_ERR "act_msgProc: kmalloc data failed.\n");
        return ret;
    }

    //调用mdrv_icc_read读取数据到分配的内存中
    ret = mdrv_icc_read(channel_id, data->data, len);
    data->len = len;

    //获取信号量 spinlock_irq_save(lock)
    spin_lock_irqsave(&(act_cdevp->stInstance[0].list_lock), flags);

    //挂接到链表，spinlick_irq_restore释放锁
    list_add_tail(&(data->msg_list), &(act_cdevp->stInstance[0].msg_list)); //应该是list_add_tail

    //释放信号量
    spin_unlock_irqrestore(&(act_cdevp->stInstance[0].list_lock), flags);

    act_cdevp->stInstance[0].hasData = true;
    wake_up(&(act_cdevp->stInstance[0].inq)); //wake up the read process

    
    printk(KERN_INFO "End act_msgProc result(%d).\n", ret);

    return ret;
}
/*lint +e429*/

static int act_open(struct inode *node, struct file *filp)
{
    struct act_cdev *act_cdev = NULL;
    struct act_instance *pstInstance = NULL;

    act_cdev = container_of(node->i_cdev, struct act_cdev, cdev);
    pstInstance = &(act_cdev->stInstance[MINOR(node->i_rdev)]);
    filp->private_data = pstInstance;
    if(0 == MINOR(node->i_rdev))
    {
        pstInstance->iChannelID = MDRV_ICC_VT_VOIP;
    }

    if(1 == MINOR(node->i_rdev))
    {
        pstInstance->iChannelID = MDRV_ICC_RCS_SERV;
    }
    printk(KERN_ERR "info act_open: minor (%d) \n", MINOR(node->i_rdev));

    return 0;
}

static int act_release(struct inode *node, struct file *filp)
{
    //struct act_cdev_data *data = NULL;
    //struct act_cdev_data *temp_data = NULL;
    //struct act_instance *act_instance;
    //struct act_cdev *act_cdev;
    //unsigned long flags = NULL;
    int ret = 0;

    printk(KERN_INFO "Enter act_release.\n");

    return ret;
}

/*lint -e571*/
static size_t act_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos)
{
    int ret = 0;
    unsigned long flags    = NULL;
    struct act_cdev_data *data = NULL;
    struct act_instance *pstInstance = NULL;

    printk(KERN_INFO "Enter act_read to bufSize(%u).\n", (unsigned int)size);

    pstInstance = (struct act_instance*)filp->private_data;

    while(!pstInstance->hasData)
    {
        if (filp->f_flags & O_NONBLOCK)
        {
            printk(KERN_ERR "act_read: NONBLOCK\n");
            //osl_sem_up(&g_ActSemId);
            return (size_t)(-EAGAIN);
        }

        wait_event_interruptible(pstInstance->inq, pstInstance->hasData);
    }

    printk(KERN_INFO "act_read start read Data(%d).\n", pstInstance->len);

    //获取信号量 spinlock_irq_save(lock)
    spin_lock_irqsave(&(pstInstance->list_lock), flags);

    //读取数据链表
    if (! list_empty(&(pstInstance->msg_list)))
    {
        data = list_first_entry(&(pstInstance->msg_list), struct act_cdev_data, msg_list);

        /*read data to user space*/
        if (copy_to_user(buf, (void*)(data->data), data->len))
        {
            ret = -EFAULT;
            printk(KERN_ERR "act_read error\n");
        }
        else
        {
           ret = (int)(data->len);
           printk(KERN_INFO "act_read has read %d bytes.\n", ret);
        }
        list_del(data);
        kfree(data);
    }
    else
    {
        printk(KERN_ERR "act_read msg_list is empty! \n");
    }

    //判断链表是否为空 list_empty(act_cdevp->data)；如果是空，false；如果非空，true;
    if (list_empty(&(pstInstance->msg_list)))
    {
        pstInstance->hasData = false; //has no data to read
    }
    else
    {
        pstInstance->hasData = true; //has data to read
        printk(KERN_ERR "act_read msg_list is not empty! \n");
    }
    //释放信号量
    spin_unlock_irqrestore(&(pstInstance->list_lock), flags);

    //osl_sem_up(&g_ActSemId);
    return (size_t)ret;
}
/*lint +e571*/

static size_t act_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos)
{
    size_t len = size;
    struct act_instance *pstInstance =  NULL;

    if (NULL == filp)
    {
        return -EFAULT;
    }
    
    pstInstance = (struct act_instance*)(filp->private_data);
    if(NULL == pstInstance || NULL == pstInstance->ucData)
    {
        printk(KERN_ERR "act_write: instance OR buf is null\n");
        return -EFAULT;
    }

    if( VT_AP_CP_CHID == pstInstance->iChannelID)
    {
        if (BUFFERSIZE < size)
        {
            printk(KERN_ERR "act_write input size long than buffer, channel:%u, size:%u.", pstInstance->iChannelID, (unsigned int)size);
            return -E2BIG;
        }
        memset(pstInstance->ucData, 0, BUFFERSIZE);
    }

    if(MDRV_ICC_RCS_SERV == pstInstance->iChannelID)
    {
        if (RCSSRVBUFSIZE < size)
        {
            printk(KERN_ERR "act_write input size long than buffer, channel:%u, size:%u.", pstInstance->iChannelID, (unsigned int)size);
            return -E2BIG;
        }
        memset(pstInstance->ucData, 0, RCSSRVBUFSIZE);
    }

    if (copy_from_user(pstInstance->ucData, buf, size))
    {
        printk(KERN_ERR "act_write failed.");
        return -EFAULT;
    }

    len = mdrv_icc_write(pstInstance->iChannelID, pstInstance->ucData, (int)size);

    printk(KERN_INFO "act_write: send data (%u)bytes, iccreturn (%u), channelId(%u)\n", (unsigned int)size, (unsigned int)len, pstInstance->iChannelID);
    return len;
}

static unsigned int act_poll(struct file* filp, poll_table *wait)
{
    struct act_instance *act_instance;
    unsigned int mask = 0;

    printk(KERN_INFO "Enter act_poll.\n");

	act_instance = filp->private_data;

    /*put the queue into poll_table*/
    poll_wait(filp, &(act_instance->inq), wait);

    if (act_instance->hasData)
    {
       mask |= POLLIN | POLLRDNORM;
       printk(KERN_INFO "notify read  process");
    }

    return mask;
}


static const struct file_operations act_cdev_fops =
{
    .owner   = THIS_MODULE,
    .read    = act_read,
    .write   = act_write,
    .open    = act_open,
    .release = act_release,
    .poll    = act_poll,
};

static void act_cdev_setup( struct act_cdev *dev,  int index)
{
    int err = 0;
    dev_t devno = MKDEV(act_major, index);

    printk(KERN_INFO "Enter act_cdev_setup.");

    //mutex_init(&(dev->lock));

    //Init the device
    cdev_init( &dev->cdev, &act_cdev_fops );
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &act_cdev_fops;

    //add the device into devices list
    err = cdev_add( &dev->cdev, devno, ACT_DEV_NUM);
    if (err)
    {
        printk(KERN_NOTICE "Error %d adding ACT Device Failed", err);
        return;
    }
}

static void act_instance_init(struct act_instance *pstInstance)
{
    pstInstance->hasData = false;
    init_waitqueue_head(&(pstInstance->inq));
    INIT_LIST_HEAD(&(pstInstance->msg_list));
    spin_lock_init(&(pstInstance->list_lock));
}

/* Init the ACT device*/
static int __init act_cdev_init(void)
{
    int ret = 0;
    dev_t devno = MKDEV(DEVICE_MAJOR, 0);
    struct act_instance *pstInstance;
    ICC_CHAN_ATTR_S chanAttr;
    ICC_CHAN_ATTR_S rcsChanAttr;

    int i = 0;

    printk(KERN_INFO "Enter act_cdev_init.");

    if (act_major)
    {
        ret = register_chrdev_region(devno, 2, "act");
    }
    else
    {
        ret = alloc_chrdev_region(&devno, 0, 2, "act");
        act_major = MAJOR(devno);
    }

    if (ret < 0)
    {
        printk(KERN_ERR "Error(%d) Init ACT device failed.", ret);
        return ret;
    }

    //alloc the reource
    act_cdevp = (struct act_cdev*)kmalloc(sizeof(struct act_cdev), GFP_KERNEL);
    if (!act_cdevp)
    {
        ret = -ENOMEM;
        printk(KERN_ERR "act_cdev_init: kmalloc failed.\n");
        unregister_chrdev_region(devno, 2);
        return ret;
    }

    memset(act_cdevp, 0, sizeof(struct act_cdev));

    act_cdev_setup(act_cdevp, 0);

    //create the device node
    act_class = class_create(THIS_MODULE, "act");
    if (IS_ERR(act_class))
    {
       printk(KERN_ERR "Act_Init: failed to create class.\n");
       ret = -EFAULT;
    }
    else
    {
        /*MINOR 0 is volte device, MINOR 1 is rcs device*/
       device_create(act_class, NULL, MKDEV(act_major, 0), NULL, "act");
       device_create(act_class, NULL, MKDEV(act_major, 1), NULL, "actr");
    }

    for (i = 0; i < ACT_DEV_NUM; i++) {
    	pstInstance = &(act_cdevp->stInstance[i]);
    	pstInstance->act_cdev = act_cdevp;
       /*MINOR 0 is volte device, */
       if(0 == i)
       {
            pstInstance->ucData = kmalloc(BUFFERSIZE, GFP_KERNEL);
       }
       /*MINIOR 1 is RCS device*/
       if(1 == i)
       {
            pstInstance->ucData = kmalloc(RCSSRVBUFSIZE, GFP_KERNEL);
       }
    	act_instance_init(pstInstance);
    }
	memset(&chanAttr, 0, sizeof(chanAttr));

    //osl_sem_init(1, &g_ActSemId);
    chanAttr.read_cb = (icc_read_cb)act_msgProc;
    //ret = bsp_icc_event_register(VT_AP_CP_CHID, act_msgProc, NULL, NULL, NULL);
    ret = mdrv_icc_open (MDRV_ICC_VT_VOIP, &chanAttr);

    memset(&rcsChanAttr, 0, sizeof(rcsChanAttr));
    rcsChanAttr.read_cb = (icc_read_cb)act_rcsSrvMsgProc;
    ret = mdrv_icc_open(MDRV_ICC_RCS_SERV, &rcsChanAttr);

    if (0 != ret)
    {
        printk(KERN_ERR "act_open: failed to init actunel with the code(0x%x)\n", ret);
    }
    return ret;
}

static void __exit act_cdev_exit(void)
{
    dev_t devno = MKDEV(act_major, 0);

    printk(KERN_INFO "Enter act_cdev_exit.");

    //bsp_icc_event_unregister(VT_AP_CP_CHID);

    //release device file
    cdev_del(&(act_cdevp->cdev));

    device_destroy(act_class, devno);
    class_destroy(act_class);

    kfree(act_cdevp);
    unregister_chrdev_region(devno, 1);
}

MODULE_AUTHOR("HUAWEI");
MODULE_LICENSE("Dual BSD/GPL");

module_init(act_cdev_init);
module_exit(act_cdev_exit);
