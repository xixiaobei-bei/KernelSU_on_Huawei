#ifndef __PUBLIC_H__
#define __PUBLIC_H__

#include "basedef.h"
#include "vfmw.h"

/* 0X0 : ALWYS, 0X1: ALWYS and FATAL, 0X3: ALWYS and FATAL and ERROR */
#define  DEFAULT_PRINT_ENABLE   (0x3)

typedef enum {
	DEV_SCREEN = 1,
	DEV_SYSLOG,
	DEV_FILE,
	DEV_MEM
} PRINT_DEVICE_TYPE;

typedef enum {
	PRN_FATAL = 0,
	PRN_ERROR,
	PRN_CTRL,
	PRN_VDMREG,

	PRN_DNMSG,
	PRN_RPMSG,
	PRN_UPMSG,
	PRN_STREAM,

	PRN_STR_HEAD,
	PRN_STR_TAIL,
	PRN_STR_BODY,
	PRN_IMAGE,

	PRN_QUEUE,
	PRN_REF,
	PRN_DPB,
	PRN_POC,

	PRN_MARK_MMCO,
	PRN_SEQ,
	PRN_PIC,
	PRN_SLICE,

	PRN_SEI,
	PRN_SE,
	PRN_DBG,
	PRN_BLOCK,

	PRN_SCD_REGMSG,
	PRN_SCD_STREAM,
	PRN_SCD_INFO,
	PRN_CRC,

	PRN_POST,
	PRN_PTS,
	PRN_DEC_MODE,
	PRN_FS,

	PRN_ALWS = 32
} PRINT_MSG_TYPE;

#define dprint_vfmw_nothing(type, fmt, arg...)    ({do{}while(0);0;})

#define dprint_sos_kernel(type, fmt, arg...)                          \
do{                                                                   \
    if ((PRN_ALWS == type) || (0 != (DEFAULT_PRINT_ENABLE & (1LL << type)))) \
    {    \
         printk(KERN_ALERT "VDEC S: "fmt, ##arg);                                  \
    }    \
}while(0)

#define dprint_linux_kernel(type, fmt, arg...)                        \
do{                                                                   \
    if ((PRN_ALWS == type) || (0 != (DEFAULT_PRINT_ENABLE & (1LL << type))))  \
    {    \
            printk(KERN_ALERT "VDEC : "fmt, ##arg);      \
    }    \
}while(0)

#ifdef HI_ADVCA_FUNCTION_RELEASE
#define dprint(type, fmt, arg...)  dprint_vfmw_nothing(type, fmt, ##arg)
#else

#ifdef ENV_SOS_KERNEL
#define dprint(type, fmt, arg...)  dprint_sos_kernel(type, fmt, ##arg)
#else

#ifdef ENV_ARMLINUX_KERNEL
#define dprint(type, fmt, arg...)  dprint_linux_kernel(type, fmt, ##arg)
#else

#define dprint(type, fmt, arg...)  dprint_vfmw_nothing(type, fmt, ##arg)
#endif

#endif

#endif

#endif
