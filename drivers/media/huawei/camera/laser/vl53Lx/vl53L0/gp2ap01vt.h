#ifndef __GP2AP00VT00F_H__
#define __GP2AP00VT00F_H__

#include "hw_laser_i2c.h"
/* operation mode */
#define SINGLE_MODE				 0
#define CONTINUOUS_MODE			 1

/* get mode */
#define INTERRUPT_MODE			0
#define POLLING_MODE			1

/* error status */
#define VALID_DATA				0
#define SIGNAL_ERROR			2
#define CLOSE_RANGE_ERROR		3
#define WAF_ERROR				4
#define HARDWARE_ERROR	       5
//#define SOFT_WAF_ERROR			5

struct gp2ap01vt00f_data
{
	struct i2c_client	   	*client;
        void *client_object; /* cci or i2c client */ //i add this parameter
	struct mutex			work_mutex;
	struct mutex			update_lock;
	struct input_dev	   	*gp2ap01vt00f_input_dev;
	struct kobject 			*range_kobj;
	struct delayed_work		gp2ap01vt00f_int_work;

	int						gp2ap01vt00f_gpio;
       int  					gp2ap01vt00f_gpio2;
	enum of_gpio_flags		irq_gpio_flags;
	int						gp2ap01vt00f_irq;
	int						irq_flags;
       const char                          *dev_name;
	int						enabled;
	int						gp2ap01vt00f_opened;

    struct timer_list 		timer;
	uint32_t 				flushCount;
	
	/* delay time in miniseconds*/
	uint8_t delay_ms;
	
    /* operation mode */
	u8 						operation_mode;		/* continuous/single mode */
	
	/* get mode */
	u8 						get_mode;			/* interrupt/polling */
	
    /* range parameters */
    u32                     range_result;
    u32                     range_result2;
	u32						ret_sig;
	u32						ret_sig2;
    u32						ret_amb;
    u32						ref_sig;
    u8						reg01h;
    u8						reg10h;
    u32						reg_error_status;
	u16						c1;
	u16						c2;
    u32                     range_raw;
    u32                     range_mm;
	u8						error_status;	
	
	int						offset_calib_data;
	int						xtalk_calib_data;

    int						range_result_ave;
    int						ret_sig_ave;
    int						ret_amb_ave;

	/* number of measurement */
	u8						num_measure;
	
	/* Polling thread */
	struct task_struct 		*poll_thread;
	/* Wait Queue on which the poll thread blocks */
	wait_queue_head_t 		poll_thread_wq;

    //add for data
    u32  timingBudget;
    u16 signalRateLimit;
    u16 sigmaLimit;
    u16 ritValue;
    u32 ritEnable;
    u8  init_flag; 
    unsigned int fracount;
};

/* platform data */
struct gp2ap01vt00f_platform_data
{
	int		gpio ;
} ;

typedef struct {
	uint16_t RangeMilliMeter;	/*!< range distance in millimeter. */

	uint32_t SignalRtnCounts;
		/*!< Return signal counts\n 24-bit value,
		 *	which is effectively a measure of target reflectance.*/
	uint32_t AmbientRtnCounts;
		/*!< Return ambient counts\n 24-bit value,
		 *	which is effectively a measure of the ambient light.*/

	uint8_t RangeStatus;
		/*!< Range Status for the current measurement. This is device
		 *	dependent. Value = 0 means value is valid.
		 *	See \ref RangeStatusPage */
} GP2AP01VT_RangingMeasurementData_t;

int gp2ap01vt00f_data_init(void** pdata, struct i2c_data *i2c_object);
int gp2ap01vt00f_laser_remove(struct i2c_client *client);
long gp2ap01vt00f_laser_ioctl(void *hw_data, unsigned int cmd, void  *p);
#endif /* GP2AP00VT00F_H */

