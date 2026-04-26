

#include <asm/uaccess.h>
#include <huawei_platform/log/hw_log.h>
#include <securec.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/syscalls.h>
#include <linux/unistd.h>
#include <linux/vmalloc.h>

#define HWLOG_TAG mcskytone
HWLOG_REGIST();
static struct work_struct register_icc_read_skytone_result;
struct hw_ap_cp_skytone_t icc_skytone;

// mcskytone support write/read by hisi modem PCIE ICC API. All code with this feature
// define regist function connect with PCIE
struct hw_ap_cp_skytone_t {
	unsigned int channel_id;
	int (*icc_open)(unsigned int channel_id, void *set_skytone_cb);
	int (*icc_read)(unsigned int channel_id, unsigned char *pdata, int size);
	int (*icc_write)(unsigned int channel_id, unsigned char *pdata, int size);
};

#define MCSKYTONE_DEVICE_NAME "mcskytone"
#define MCSKYTONE_DEVICE_CLASS "mcskytone_class"

#define MCSKYTONE_FILE_HEAD_LEN 12
#define MCSKYTONE_NO_DATA_LEN 0
#define MCSKYTONE_MAX_DATA_LEN (5*1024)
#define MCSKYTONE_WORK_WAIT_TIMER (30*60)

#define MCSKYTONE_DATA_MODE_INVALID (-1)
#define MCSKYTONE_DATA_MODE_TIME 3
#define MCSKYTONE_DATA_UNAVAILABLE 0
#define MCSKYTONE_DATA_AVAILABLE 1

#define BSP_ERR_MCSKYTONE_OK 0
#define BSP_ERR_MCSKYTONE_ERR 1
#define BSP_ERR_MCSKYTONE_DATA_LENGTH_ERR 2
#define BSP_ERR_MCSKYTONE_PARAM_ERR 3
#define BSP_ERR_MCSKYTONE_MALLOC_FAIL 4
#define BSP_ERR_MCSKYTONE_COPY_FROM_USER_ERR 5
#define BSP_ERR_MCSKYTONE_TIMEOUT 6
#define BSP_ERR_MCSKYTONE_OPEN_FILE_ERR 7
#define BSP_ERR_MCSKYTONE_NO_RESULT 8
#define BSP_ERR_MCSKYTONE_ICC_READ_LEN_ERR 9
#define BSP_ERR_MCSKYTONE_SEND_NULL_DATA 10
#define BSP_ERR_MCSKYTONE_NO_DATA_RECV 11
#define BSP_ERR_MCSKYTONE_COPY_TO_USER_ERR 12
#define BSP_ERR_MCSKYTONE_READ_ERR 13
#define BSP_ERR_MCSKYTONE_WRITE_ERR 14

// character type device
static struct cdev g_kernel_dev;
static unsigned int mcskytone_major;
static struct class *mcskytone_class;

// data send to pcie
static struct mcskytone_file_stru *g_buf_send = NULL;

// data receive from pcie
static struct mcskytone_file_stru *g_buf_recv = NULL;

static struct mcskytone_time_stru *g_mcskytone_time = NULL;

static struct mutex mcskytone_mutex;
static DECLARE_WAIT_QUEUE_HEAD(skytone_set_result_waitq);
static int g_is_new_data_available; // indicate new data received from pcie
static int g_time_data_received; // used to get time from teec

struct mcskytone_file_stru {
	int result;
	int mode;
	unsigned int len;  // data length
	char data[MCSKYTONE_MAX_DATA_LEN];
};

struct mcskytone_time_stru {
	int result;
	int mode;
	unsigned int time_type;  // time type receive from tee
};

struct teec_timer_property_stru {
	unsigned int time_type; /* Timer Type */
	unsigned int time_id;   /* Timer ID */
	unsigned int rev1;
	unsigned int rev2;
};

ssize_t mcskytone_dev_read(struct file *file, char __user *buf, size_t len, loff_t *ppos);
ssize_t mcskytone_dev_write(struct file *file, const char __user *buf, size_t len, loff_t *ppos);
static int mcskytone_dev_open(struct inode *inode, struct file *file);

extern int TC_NS_RegisterServiceCallbackFunc(const char *uuid, void *func, const void *private_data);

static const struct file_operations mcskytone_fops = {
	.owner = THIS_MODULE,
	.read = mcskytone_dev_read,
	.write = mcskytone_dev_write,
	.open = mcskytone_dev_open,
};

void tee_timeout_callback(void *timer_data_callback)
{
	struct teec_timer_property_stru *timer_data = NULL;
	hwlog_info("%s enter.\n", __func__);

	if (timer_data_callback == NULL) {
		hwlog_err("tee_timeout_callback: para error!");
		return;
	}

	mutex_lock(&mcskytone_mutex);
	g_mcskytone_time = kmalloc(sizeof(struct teec_timer_property_stru), GFP_ATOMIC);
	if (g_mcskytone_time == NULL) {
		hwlog_err("construct_result_buf, kmalloc failed!\n");
		mutex_unlock(&mcskytone_mutex);
		return;
	}

	timer_data = (struct teec_timer_property_stru *)timer_data_callback;
	g_mcskytone_time->mode = MCSKYTONE_DATA_MODE_TIME;
	g_mcskytone_time->result = BSP_ERR_MCSKYTONE_OK;
	g_mcskytone_time->time_type = timer_data->time_type;
	hwlog_info("%s, get timeout time_type = %d\n", __func__, timer_data->time_type);

	g_is_new_data_available = MCSKYTONE_DATA_AVAILABLE;
	g_time_data_received = MCSKYTONE_DATA_AVAILABLE;
	mutex_unlock(&mcskytone_mutex);
	/* wake up reader */
	wake_up_interruptible(&skytone_set_result_waitq);
}

void regist_tcc_timeout(void)
{
	// this uuid must same as skytone APK, it is used in tee OS to check validity
	const unsigned char uuid[] = {
		0x47, 0x91, 0xe8, 0xab, 0x61, 0xcd, 0x3f, 0xf4, 0x71, 0xc4, 0x1a, 0x31, 0x7e, 0x40, 0x53, 0x12
	};
	hwlog_info("%s\n", __func__);

	if (TC_NS_RegisterServiceCallbackFunc((const char *)uuid, tee_timeout_callback, NULL) != BSP_ERR_MCSKYTONE_OK)
		hwlog_err("<%s> Reg TEE Timeout CB FUN Fail\n", __func__);
	hwlog_info("<%s> Reg TEE Timeout CB FUN Ok\n", __func__);
	return;
}
static int mcskytone_dev_open(struct inode *inode, struct file *filp)
{
	hwlog_info("%s enter\n", __func__);
	regist_tcc_timeout();
	return BSP_ERR_MCSKYTONE_OK;
}

static int write_skytone_key_to_modem(int data_len)
{
	int ret = BSP_ERR_MCSKYTONE_OK;
	int len;
	hwlog_info("%s, dataLen = %d\n", __func__, data_len);

	if (g_buf_send == NULL || icc_skytone.icc_write == NULL) {
		hwlog_err("%s, g_buf_send = NULL or icc_skytone.icc_write = NULL.\n", __func__);
		ret = BSP_ERR_MCSKYTONE_SEND_NULL_DATA;
	} else {
		if (g_buf_send->len > MCSKYTONE_MAX_DATA_LEN) {
			ret = BSP_ERR_MCSKYTONE_DATA_LENGTH_ERR;
			return;
		}
		len = g_buf_send->len + MCSKYTONE_FILE_HEAD_LEN;
		hwlog_info("%s, actually data Len to send = %d\n", __func__, len);

		ret = icc_skytone.icc_write(icc_skytone.channel_id, (unsigned char *)g_buf_send, len);
		if (ret != len)
			ret = BSP_ERR_MCSKYTONE_WRITE_ERR;
		else
			ret = BSP_ERR_MCSKYTONE_OK;

		if (g_buf_send != NULL) {
			kfree(g_buf_send);
			g_buf_send = NULL;
		}
	}

	return ret;
}

static ssize_t mcskytone_copy_user_data(void** dest_buf, const char __user *src_buf, size_t src_buf_size)
{
	ssize_t ret = BSP_ERR_MCSKYTONE_OK;
	hwlog_info("%s\n", __func__);

	if (src_buf_size > MCSKYTONE_MAX_DATA_LEN) {
		hwlog_err("%s param src_buf_size too big\n", __func__);
		return BSP_ERR_MCSKYTONE_DATA_LENGTH_ERR;
	}

	*dest_buf = kmalloc(src_buf_size, GFP_ATOMIC);
	if (*dest_buf == NULL) {
		hwlog_err("%s kmalloc fail\n", __func__);
		return BSP_ERR_MCSKYTONE_MALLOC_FAIL;
	}

	if (memset_s(*dest_buf, src_buf_size, 0, src_buf_size) != EOK) {
		hwlog_err("%s memset_s fail\n", __func__);
		if (*dest_buf != NULL) {
			kfree(*dest_buf);
			*dest_buf = NULL;
		}
		return BSP_ERR_MCSKYTONE_ERR;
	}

	if (copy_from_user(*dest_buf, src_buf, src_buf_size)) {
		ret = BSP_ERR_MCSKYTONE_COPY_FROM_USER_ERR;
		if (*dest_buf != NULL) {
			kfree(*dest_buf);
			*dest_buf = NULL;
		}
		hwlog_err("copy_from_user error\n");
	}

	return ret;
}

static ssize_t mcskytone_dev_copy_buf(const char __user *buf, size_t count)
{
	ssize_t ret = BSP_ERR_MCSKYTONE_OK;

	ret = mcskytone_copy_user_data((void**)&g_buf_send, buf, count);
	if (ret != BSP_ERR_MCSKYTONE_OK) {
		hwlog_err("mcskytone_dev_copy_buf ret = %d\n", ret);
	}
	return ret;
}

static ssize_t mcskytone_check_buf_size(size_t count)
{
	ssize_t ret = BSP_ERR_MCSKYTONE_OK;

	if (count > MCSKYTONE_MAX_DATA_LEN || count <= MCSKYTONE_FILE_HEAD_LEN)
		ret = BSP_ERR_MCSKYTONE_DATA_LENGTH_ERR;
	return ret;
}

// this fuction actrully save data to receive buffer g_buf_recv
void send_result_to_ril(int len)
{
	int ret;
	hwlog_info("send_result_to_ril enter, len = %d\n", len);

	if (g_buf_recv == NULL) {
		hwlog_err("send_result_to_ril, dataBuf is NULL\n");
		return;
	}

	if (icc_skytone.icc_read == NULL) {
		hwlog_err("send_result_to_ril, icc_skytone.icc_read is NULL\n");
		return;
	}

	ret = icc_skytone.icc_read(icc_skytone.channel_id, (unsigned char *)g_buf_recv, len);
	if (ret != len) {
		hwlog_err("%s icc_read size error = %d, read len = %d\n", __func__, len, ret);
		g_buf_recv->len = MCSKYTONE_NO_DATA_LEN;
		g_buf_recv->result = BSP_ERR_MCSKYTONE_ICC_READ_LEN_ERR;
		g_buf_recv->mode = MCSKYTONE_DATA_MODE_INVALID;
	}
	g_is_new_data_available = MCSKYTONE_DATA_AVAILABLE;

	/* wake up reader */
	wake_up_interruptible(&skytone_set_result_waitq);
}

// this function is called when uplayer wirte data to file
ssize_t mcskytone_dev_write(struct file *file, const char __user *buf, size_t len, loff_t *ppos)
{
	ssize_t ret = BSP_ERR_MCSKYTONE_OK;
	hwlog_info("%s enter\n", __func__);

	if (buf == NULL) {
		hwlog_err("mcskytone_dev_write user buf is NULL\n");
		ret == BSP_ERR_MCSKYTONE_PARAM_ERR;
		return ret;
	}

	ret = mcskytone_check_buf_size(len);
	if (ret != BSP_ERR_MCSKYTONE_OK) {
		hwlog_err("mcskytone_dev_write err: len = %d\n", len);
		return ret;
	}

	mutex_lock(&mcskytone_mutex);
	ret = mcskytone_dev_copy_buf(buf, len);
	if (ret != BSP_ERR_MCSKYTONE_OK) {
		mutex_unlock(&mcskytone_mutex);
		return ret;
	}

	ret = write_skytone_key_to_modem(len);
	mutex_unlock(&mcskytone_mutex);
	if (ret != BSP_ERR_MCSKYTONE_OK) {
		hwlog_err("wirte_skytone_key to modem errcode(%d)!\n", (int)ret);
	}

	return ret;
}

int construct_result_buf(int len)
{
	int ret = BSP_ERR_MCSKYTONE_OK;
	hwlog_info("%s enter\n", __func__);

	if (len > MCSKYTONE_MAX_DATA_LEN) {
		hwlog_err("%s param len too big\n", __func__);
		return BSP_ERR_MCSKYTONE_DATA_LENGTH_ERR;
	}
	mutex_lock(&mcskytone_mutex);
	g_buf_recv = kmalloc(len, GFP_ATOMIC);
	if (g_buf_recv == NULL) {
		hwlog_err("construct_result_buf, kmalloc failed!\n");
		ret = BSP_ERR_MCSKYTONE_MALLOC_FAIL;
	}
	mutex_unlock(&mcskytone_mutex);

	return ret;
}

// mcskytone_dev_open
ssize_t mcskytone_dev_read(struct file *file, char __user *buf, size_t len, loff_t *ppos)
{
	int error = BSP_ERR_MCSKYTONE_OK;
	int ret = BSP_ERR_MCSKYTONE_OK;
	int data_len;
	hwlog_info("%s enter\n", __func__);

	if (buf == NULL || mcskytone_check_buf_size(len) != BSP_ERR_MCSKYTONE_OK) {
		hwlog_err("%s param error. len = %d\n", __func__, len);
		return BSP_ERR_MCSKYTONE_PARAM_ERR;
	}
	error = wait_event_interruptible(
		skytone_set_result_waitq, g_is_new_data_available == MCSKYTONE_DATA_AVAILABLE);

	mutex_lock(&mcskytone_mutex);
	if (error) {
		hwlog_err("wait_event_interruptible error = %d\n", error);
		ret = BSP_ERR_MCSKYTONE_READ_ERR;
	} else if (g_time_data_received == MCSKYTONE_DATA_AVAILABLE && g_mcskytone_time != NULL) {
		data_len = sizeof(struct mcskytone_time_stru);
		if ((len < data_len) || (copy_to_user(buf, g_mcskytone_time, data_len))) {
			hwlog_err("%s, copy time data to_user failed!\n", __func__);
			ret =  BSP_ERR_MCSKYTONE_COPY_TO_USER_ERR;
		}

		g_time_data_received = MCSKYTONE_DATA_UNAVAILABLE;
		hwlog_info("%s, g_time_data_received over\n", __func__);
	} else if (g_buf_recv != NULL) {
		data_len = g_buf_recv->len + MCSKYTONE_FILE_HEAD_LEN;
		if ((len < data_len) || (copy_to_user(buf, g_buf_recv, data_len))) {
			hwlog_err("%s, copy_to_user failed!\n", __func__);
			ret =  BSP_ERR_MCSKYTONE_COPY_TO_USER_ERR;
	    }
		hwlog_info("%s g_buf_recv read data over\n", __func__);
	}

	if (g_mcskytone_time != NULL) {
		kfree(g_mcskytone_time);
		g_mcskytone_time = NULL;
	}

	if (g_buf_recv != NULL) {
		kfree(g_buf_recv);
		g_buf_recv = NULL;
	}
	g_is_new_data_available = MCSKYTONE_DATA_UNAVAILABLE;
	mutex_unlock(&mcskytone_mutex);

	return ret;
}

int mcskytone_init(void)
{
	int ret = BSP_ERR_MCSKYTONE_OK;
	dev_t dev = BSP_ERR_MCSKYTONE_OK;
	unsigned int devno;

	/* get dynamic dev number */
	ret = alloc_chrdev_region(&dev, 0, 1, MCSKYTONE_DEVICE_NAME);
	if (ret) {
		hwlog_err("mcskytone_init, mcskytone failed alloc, ret = %d\n", ret);
		return ret;
	}

	mcskytone_major = MAJOR(dev);	  // get major device number
	devno = MKDEV(mcskytone_major, 0); // device number transfer
	(void)memset_s((void*)&g_kernel_dev, sizeof(struct cdev), 0, sizeof(struct cdev));
	cdev_init(&g_kernel_dev, &mcskytone_fops);  // character device init
	g_kernel_dev.owner = THIS_MODULE;
	g_kernel_dev.ops = &mcskytone_fops; // add device operations

	ret = cdev_add(&g_kernel_dev, devno, 1); // add device to system
	if (ret) {
		hwlog_err("mcskytone_init, cdev_add fail!\n");
		return ret;
	}

	mcskytone_class = class_create(THIS_MODULE, MCSKYTONE_DEVICE_CLASS);
	if (IS_ERR(mcskytone_class)) {
		hwlog_err("mcskytone_init, class create failed!\n");
		return ret;
	}
	device_create(mcskytone_class, NULL, devno, NULL, MCSKYTONE_DEVICE_NAME);

	mutex_init(&mcskytone_mutex);
	g_is_new_data_available = MCSKYTONE_DATA_UNAVAILABLE;
	g_time_data_received = MCSKYTONE_DATA_UNAVAILABLE;

	hwlog_info("mcskytone_init, complete!\n");
	return BSP_ERR_MCSKYTONE_OK;
}

// receive PCIE message
static int check_skytone_result(u32 channel_id, u32 len, void *context)
{
	unsigned int ret = BSP_ERR_MCSKYTONE_OK;
	hwlog_info("%s enter\n", __func__);

	if ((len <= MCSKYTONE_NO_DATA_LEN) || icc_skytone.icc_read == NULL) {
		hwlog_err("process modem info para err, len = %d\n", len);
		ret = BSP_ERR_MCSKYTONE_DATA_LENGTH_ERR;

		mutex_lock(&mcskytone_mutex);
		g_buf_recv = kmalloc(MCSKYTONE_FILE_HEAD_LEN, GFP_ATOMIC);
		if (g_buf_recv == NULL) {
			hwlog_err("construct_result_buf, kmalloc failed!\n");
			mutex_unlock(&mcskytone_mutex);
			return BSP_ERR_MCSKYTONE_MALLOC_FAIL;
		}

		g_buf_recv->len = MCSKYTONE_NO_DATA_LEN;
		g_buf_recv->result = BSP_ERR_MCSKYTONE_DATA_LENGTH_ERR;
		g_buf_recv->mode = MCSKYTONE_DATA_MODE_INVALID;
		g_is_new_data_available = MCSKYTONE_DATA_AVAILABLE;
		mutex_unlock(&mcskytone_mutex);

		wake_up_interruptible(&skytone_set_result_waitq);
		return ret;
	}

	if (construct_result_buf(len) == BSP_ERR_MCSKYTONE_OK)
		send_result_to_ril(len);

	return ret;
}

// register call back function check_skytone_result to PCIE, pcie will call
// check_skytone_result when is message need transmit to P-kernel
static void register_icc_read_skytone_result_handler(struct work_struct *wk)
{
	hwlog_info("%s enter\n", __func__);

	if (icc_skytone.icc_open == NULL) {
		hwlog_err("%s null ptr\n", __func__);
		return;
	}

	icc_skytone.icc_open(icc_skytone.channel_id, check_skytone_result);
}

// this function is callback for set skytone result
void connect_pcie_icc_set_skytone_result(struct hw_ap_cp_skytone_t *ap_cp_skytone)
{
	hwlog_info("%s\n", __func__);

	if (ap_cp_skytone == NULL) {
		hwlog_err("%s null ptr\n", __func__);
		return;
	}
	(void)memset_s((void *)&icc_skytone, sizeof(struct hw_ap_cp_skytone_t), 0, sizeof(struct hw_ap_cp_skytone_t));
	(void)memcpy_s((void *)&icc_skytone, sizeof(icc_skytone), ap_cp_skytone, sizeof(struct hw_ap_cp_skytone_t));
	hwlog_info("%s, multichipSkytone icc channel id = %d\n", __func__, icc_skytone.channel_id);

	INIT_WORK(&register_icc_read_skytone_result, register_icc_read_skytone_result_handler);
	schedule_work(&register_icc_read_skytone_result);
}
EXPORT_SYMBOL_GPL(connect_pcie_icc_set_skytone_result);

void mcskytone_exit(void)
{
	cdev_del(&g_kernel_dev);
	class_destroy(mcskytone_class);
	unregister_chrdev_region(MKDEV(mcskytone_major, 0), 1);
}

module_init(mcskytone_init);
module_exit(mcskytone_exit);
MODULE_AUTHOR("Hisilicon Drive Group");
MODULE_DESCRIPTION("mcskytone driver for Hisilicon");
MODULE_LICENSE("GPL");
