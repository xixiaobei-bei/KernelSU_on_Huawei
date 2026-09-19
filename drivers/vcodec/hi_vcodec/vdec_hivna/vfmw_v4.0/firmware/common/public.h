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

#define HW_ADDR_RSHIFT(addr)  ((addr) >> (4))

#define HW_ADDR_LSHIFT(addr)  ((addr) << (4))

#define ALIGN_UP(val, align)  (((val) + ((align)-1)) & ~((align)-1))

#define ALIGN_DOWN_HAL(val, align)      ((val) & (~(align - 1)))

/*The address is 36bit, take the high 4bits*/
#define TAKE_HIGH_BITS(val)         (((val) >> (32)) & (0xF))

/*To patch the low 32bit address and high 4bit address*/
#define PATCH_ADDRESS(high, low)    ((((UINT64)(high) & 0xf0000000) << 4) | (low))

#define  MSG_SLOT_ALIGN_BYTE    (16)

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
