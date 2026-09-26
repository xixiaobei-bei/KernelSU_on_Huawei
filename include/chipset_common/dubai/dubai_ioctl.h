#ifndef DUBAI_IOCTL_H
#define DUBAI_IOCTL_H

#include <linux/ioctl.h>

#define DUBAI_DIR_IOC(dir, type, nr, param)		_IO##dir(DUBAI_IOC_##type, nr, param)
#define DUBAI_IOC(type, nr)						_IO(DUBAI_IOC_##type, nr)
#define DUBAI_DISPLAY_DIR_IOC(dir, nr, param)	DUBAI_DIR_IOC(dir, DISPLAY, nr, param)
#define DUBAI_CPU_DIR_IOC(dir, nr, param)		DUBAI_DIR_IOC(dir, CPU, nr, param)
#define DUBAI_GPU_DIR_IOC(dir, nr, param)		DUBAI_DIR_IOC(dir, GPU, nr, param)
#define DUBAI_SENSORHUB_DIR_IOC(dir, nr, param)	DUBAI_DIR_IOC(dir, SENSORHUB, nr, param)
#define DUBAI_BATTERY_DIR_IOC(dir, nr, param)	DUBAI_DIR_IOC(dir, BATTERY, nr, param)
#define DUBAI_UTILS_DIR_IOC(dir, nr, param)		DUBAI_DIR_IOC(dir, UTILS, nr, param)
#define DUBAI_SR_DIR_IOC(dir, nr, param)		DUBAI_DIR_IOC(dir, SR, nr, param)
#define DUBAI_MISC_DIR_IOC(dir, nr, param)		DUBAI_DIR_IOC(dir, MISC, nr, param)
#define DUBAI_DDR_DIR_IOC(dir, nr, param)		DUBAI_DIR_IOC(dir, DDR, nr, param)
#define DUBAI_PERI_DIR_IOC(dir, nr, param)		DUBAI_DIR_IOC(dir, PERI, nr, param)

enum dubai_ioctl_type_t {
	DUBAI_IOC_DISPLAY = 0,
	DUBAI_IOC_CPU = 1,
	DUBAI_IOC_GPU = 2,
	DUBAI_IOC_SENSORHUB = 3,
	DUBAI_IOC_BATTERY = 4,
	DUBAI_IOC_UTILS = 5,
	DUBAI_IOC_SR = 6,
	DUBAI_IOC_MISC = 7,
	DUBAI_IOC_DDR = 8,
	DUBAI_IOC_PERI = 9,
};

#endif // DUBAI_IOCTL_H
