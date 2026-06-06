#ifndef _PROF_DRV_DEV_H_
#define _PROF_DRV_DEV_H_

#include <linux/wait.h>

#ifndef STATIC
#define STATIC
#endif

#define CHAR_DRIVER_NAME      "prof_drv"
#define DRV_MODE_VERSION      "DAVINCI 1.01"
#define PROF_SQ_SLOT_LEN	      128
#define PROF_CQ_SLOT_LEN	      32

#define TS2DRV_TIMEOUT ((HZ)/100)                 /* 0.01 second */

/* peripheral type */
enum prof_channel_ids {
	CHANNEL_HBM = 1,
	CHANNEL_BUS = 2,
	CHANNEL_PCIE = 3,
	CHANNEL_NIC = 4,
	CHANNEL_DMA = 5,
	CHANNEL_DVPP = 6,
	CHANNEL_TSCPU = 10,
	CHANNEL_AICPU0 = 11,	 /*aicpu: 11---42*/
	CHANNEL_AICORE = 43,
	CHANNEL_TSFW = 44,
	CHANNEL_IDS_MAX
};

/* profiling command */
enum prof_cmd_type {
	PROF_PLATFORM = 201,
	PROF_DEVNUM,
	PROF_DEVIDS,
	PROF_DEVINFO,
	PROF_DEV_START,
	PROF_TS_START,
	PROF_STOP,
	PROF_READ,
	PROF_POLL,
	PROF_CMD_MAX
};

enum prof_channel_type {
	PROF_TS_TYPE,
	PROF_PERIPHERAL_TYPE,
	PROF_CHANNEL_TYPE_MAX
};

/* real time type */
#define PROF_NON_REAL 0
#define PROF_REAL     1

/*channel's status need third state:unused, used, stopping*/
#define PROF_CHANNEL_UNUSED	0
#define PROF_CHANNEL_STOPPING 1
#define PROF_CHANNEL_WAIT		50	/*almost wait 5 second */

#define PROF_ON_DEVICE 0
#define PROF_ON_HOST   1

/* period time / ms */
#define PROF_PERIOD_MIN	1       /* 1ms */
#define PROF_PERIOD_MAX 10000   /* 10s */

#define PROF_FILE_NAME_MAX 128
#define DATA_BUF_RESERVED  0x80
#define PROF_DEVICE_NUM	   64
#define PROF_CHANNEL_MAX   64
#define CACHE_LINE_LEN	   (128)
#define PROF_BUFFER_LEN	  	(10 * 1024)
#define PROF_TS_BUFFER_LEN 	(1024 * 1024)

#define PROF_SAMPLE_PROC_NAME "prof_sample"

#define PROF_OK	     (0)
#define PROF_ERROR   (-1)
#define PROF_TIMEOUT (-2)

#define SAMPLE_MASK	   0x01
#define SAMPLE_ONLY_DATA   0x0
#define SAMPLE_WITH_HEADER 0x1

#define DEV_UNUSED 0
#define DEV_USED   1

#define TIME_UNIT      (HZ)
#define PROF_POLL_DEPTH	512

enum channel_poll_type {
	POLL_INVALID,
	POLL_VALID
};

enum prof_ts_cmd_type {
	TS_START = 1,
	TS_STOP = 2,
	TS_FULL = 3,
	TS_SYNC_READ_PTR =4,
	TS_SYNC_WRITE_PTR = 5
};

enum prof_channel_state {
	CHANNEL_DISABLE,
	CHANNEL_ENABLE
};

enum prof_tscpu_stop_type {
	PROF_DEV_STOP,
	NPU_POWERDOWN_STOP
};

enum prof_channel_index_enum {
	PROF_CHANNEL_AICPU0_IDX = 0,
	PROF_CHANNEL_AICORE_IDX = 1,
	PROF_CHANNEL_TSFW_IDX = 2,
	PROF_CHANNEL_NUM // for AICPU0 log and AICORE log
};


#define MODULE_PROF	"drv_prof"
#define prof_err(fmt ...)   drv_err(MODULE_PROF, fmt)
#define prof_warn(fmt ...)  drv_warn(MODULE_PROF, fmt)
#define prof_info(fmt ...)  drv_info(MODULE_PROF, fmt)
#define prof_debug(fmt ...) drv_debug(MODULE_PROF, fmt)

typedef struct prof_ioctl_para {
	unsigned int device_id;
	unsigned int channel_id;
	unsigned int channel_type;          /* for ts and other device */
	unsigned int buf_len;               /* buffer size */
	unsigned int sample_period;         /* 采样周期 */
	unsigned int real_time;             /* real mode */
	unsigned int ts_data_size;          /* ts configure data's size */
	int	     	ret_val;
	int 		timeout;
	int 		poll_number;

	char prof_file[PROF_FILE_NAME_MAX]; /* file path */
	char ts_data[PROF_SQ_SLOT_LEN];      /* ts data's pointer */

	void *out_buf;                      /* save return info */
} prof_ioctl_para_t;

/*size: 128 byte*/
typedef struct prof_data_head {
	volatile unsigned int read_ptr;
	volatile unsigned int buf_len;
	volatile unsigned int rev1[14];
	volatile unsigned int write_ptr;
	volatile unsigned int rev2[15];
} prof_data_head_t;

typedef struct prof_device_info {
	unsigned int		cmd_verify;
	unsigned int		device_id;
	unsigned int		device_state;
	unsigned int		sq_0_index;
	unsigned int		cq_0_index;
	unsigned int		cq_1_index;
	unsigned int		cq_2_index;
	unsigned long long	sq_0_addr;
	unsigned long long	cq_0_addr;
	unsigned long long	cq_1_addr;
	unsigned long long	cq_2_addr;
	unsigned int		poll_head;
	unsigned int		poll_tail;
	struct mutex		ts_file_mutex;	/*tscpu,aicore is different, but aicpu may be the same filepath*/
	struct mutex		ext_file_mutex;	/*if the device is different , this element can be deleted.*/
	struct mutex		cmd_mutex;		/*for single thread*/
	struct mutex	 	cq1_mutex;		/*for poll box*/
	wait_queue_head_t cq1_wq;
} prof_device_info_t;

typedef struct prof_channel_info {
	unsigned int cmd_verify;
	unsigned int device_id;
	unsigned int channel_id;
	unsigned int channel_state;
	unsigned int real_time;
	unsigned int buf_len;
	unsigned int sample_period;         /* 采样周期 ; unit: ms*/
	int          ret_val;
	int 		poll_flag;
	int		used;			/*the channel is used or not */

	unsigned long	   phy_addr;
	unsigned char	  *vir_addr;
	struct semaphore   sync_wait_sema;
	struct semaphore   sema_channel_stopping;
	struct task_struct *sample_thread;
	char	           prof_file[PROF_FILE_NAME_MAX];
	int (*prof_sample)(void *buf, int len, int flag);
	struct mutex	 cmd_mutex;		/*for multi thread; the cmds are create/delete/read/set */
} prof_channel_info_t;

/*size: 32 byte*/
typedef struct prof_sq_scheduler {
	unsigned int  cmd_verify;
	unsigned int  channel_id;
	unsigned int  channel_cmd;
	unsigned int  buf_len;
	unsigned long phy_addr;
	unsigned int  data_size;        /* ts data size; */
	unsigned char ts_cpu_data[4];   /* ts data , only use data pointer */
} prof_sq_scheduler_t;

#define PROF_SQ_TS_LEN (PROF_SQ_SLOT_LEN - sizeof(struct prof_sq_scheduler))

/*need ts to fill*/
struct prof_cq_scheduler {
	unsigned int cmd_verify;
	unsigned int channel_id;
	unsigned int channel_cmd;               /*type: start, stop, full*/
	int	     ret_val;                   /* the result; */
} prof_cq_scheduler_t;

typedef struct prof_poll_info {
	unsigned int device_id;
	unsigned int channel_id;
} prof_poll_info_t;

typedef struct char_device {
	struct class *dev_class;
	struct cdev   cdev;
	dev_t	      devno;
	struct device *device;
} char_device_t;

typedef struct prof_char_dev_global_info {
	char_device_t char_dev;
	struct prof_poll_info *poll_box;
	struct prof_channel_info prof_channel[PROF_CHANNEL_NUM];
	struct prof_device_info   prof_device;
	unsigned int op_cb_idx;
	unsigned int rl_cb_idx;
} prof_char_dev_global_info_t;

extern int dvpp_profile_func(void *buffer_addr, int buffer_len, int flag);

#endif /* _PROF_DRV_DEV_H_ */
