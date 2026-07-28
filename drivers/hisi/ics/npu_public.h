#include <linux/errno.h>
#include <linux/kernel.h>

#ifndef _NPU_PUBLIC_H
#define _NPU_PUBLIC_H

//#define DEBUG_PRINT

#define NPU_ERR(string, arg...)    printk(KERN_ERR "[%s][%d]:NPU_ERROR:" string "\n", __func__, __LINE__, ##arg)
#define NPU_PRINT(string, arg...)    printk(KERN_ERR "[%s][%d]:" string "\n", __func__, __LINE__, ##arg)

#ifdef DEBUG_PRINT
#define NPU_INFO(string, arg...)  printk(KERN_ERR "[%s][%d]:" string "\n", __func__, __LINE__, ##arg)
#define NPU_DEBUG(string, arg...)  printk(KERN_ERR "[%s][%d]: " string  "\n", __func__, __LINE__, ##arg)
#else
#define NPU_INFO(string, arg...)  printk(KERN_INFO "[%s][%d]:" string "\n", __func__, __LINE__, ##arg)
#define NPU_DEBUG(string, arg...)
#endif

typedef enum {
	NPU_CORE_0,
	NPU_CORE_1,
	MAX_SUPPORT_CORE_NUM,
} NPU_CORE_ID_E;

typedef enum {
	TASK_PROCESS_OK    = 0,
	TASK_INTERRUPT_ERR = 1,
	TASK_SEC_CANCEL    = 2,
} NPU_TASK_PROCESS_E;

#define UNUSED_PARAMETER(x) ((void)(x))
#define CHECK_COREID_INVALID_RETURN(coreID) \
do{\
	if ((coreID >= MAX_SUPPORT_CORE_NUM) || (NPU_VERSION_V200 != hard_version && NPU_CORE_1 == coreID)) {\
		printk(KERN_ERR"[%s]: IPU_ERROR:invalid core ID=%d\n", __func__, coreID);\
		return -EINVAL;\
	}\
} while(0);

#define CHECK_COREID_INVALID_RETURN_VOID(coreID) \
do{\
	if ((coreID >= MAX_SUPPORT_CORE_NUM) || (NPU_VERSION_V200 != hard_version && NPU_CORE_1 == coreID)) {\
		printk(KERN_ERR"[%s]: IPU_ERROR:invalid core ID=%d\n", __func__, coreID);\
		return;\
	}\
} while(0);

extern int hard_version;

#define PLATFORM_VERSION       (0x00100001)
#define NPU_VERSION_V200_SSWQ  (0x00010000)
#define NPU_VERSION_V200       (0x00010001)
#define NPU_VERSION_V150       (0x00000000)

#endif

