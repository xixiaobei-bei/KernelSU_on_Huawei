#ifndef _DRV_LOG_H_
#define _DRV_LOG_H_

#define drv_printk(level, module, fmt, ...) \
	printk(level " [%s] [%s %d] " fmt, module, \
		__func__, __LINE__, ## __VA_ARGS__)

#define drv_err(module, fmt ...)   drv_printk(KERN_ERR, module, fmt)
#define drv_warn(module, fmt ...)  drv_printk(KERN_WARNING, module, fmt)
#define drv_info(module, fmt ...)
#define drv_debug(module, fmt ...)


#define module_devdrv   "NPU_DRV"
#define devdrv_drv_err(fmt, ...)	drv_printk(KERN_ERR, module_devdrv"] [E",  fmt, ## __VA_ARGS__)
#define devdrv_drv_warn(fmt, ...)	drv_printk(KERN_WARNING, module_devdrv"] [W",  fmt, ## __VA_ARGS__)
#define devdrv_drv_info(fmt, ...)	//drv_printk(KERN_INFO, module_devdrv"] [I", fmt, ## __VA_ARGS__)
#define devdrv_drv_debug(fmt, ...)	//drv_printk(KERN_DEBUG, module_devdrv"] [D", fmt, ## __VA_ARGS__)


#define NPU_DRV_BOOT_TIME_TAG(fmt, ...)     //devdrv_drv_warn(fmt, ## __VA_ARGS__)


#endif/* _DRV_LOG_H_ */
