#ifndef _MDRV_PCDEV_H_
#define _MDRV_PCDEV_H_

#ifdef _cplusplus
extern "C"
{
#endif


/* IOCTL CMD ∂®“Â */
#define PCDEV_IOCTL_SET_WRITE_CB      0x7F001000
#define PCDEV_IOCTL_SET_READ_CB       0x7F001001
#define PCDEV_IOCTL_SET_EVT_CB        0x7F001002
#define PCDEV_IOCTL_SET_FREE_CB       0x7F001003
#define PCDEV_IOCTL_SET_WRITE_INFO_CB 0x7F001004
#define PCDEV_IOCTL_SEND_EVT          0x7F001005

#define PCDEV_IOCTL_WRITE_ASYNC       0x7F001010
#define PCDEV_IOCTL_GET_RD_BUFF       0x7F001011
#define PCDEV_IOCTL_RETURN_BUFF       0x7F001012
#define PCDEV_IOCTL_RELLOC_READ_BUFF  0x7F001013
#define PCDEV_IOCTL_SEND_BUFF_CAN_DMA 0x7F001014

#define PCDEV_IOCTL_IS_IMPORT_DONE    0x7F001020
#define PCDEV_IOCTL_WRITE_DO_COPY     0x7F001021

#define PCDEV_MODEM_IOCTL_SET_MSC_READ_CB 0x7F001030
#define PCDEV_MODEM_IOCTL_MSC_WRITE_CMD   0x7F001031
#define PCDEV_MODEM_IOCTL_SET_REL_IND_CB  0x7F001032

typedef struct tagPCDEV_WR_ASYNC_INFO
{
    char *pVirAddr;
    char *pPhyAddr;
    unsigned int u32Size;
    void* pDrvPriv;
}PCDEV_WR_ASYNC_INFO;


typedef struct tagPCDEV_WRITE_INFO
{
    char *pVirAddr;
    char *pPhyAddr;
    unsigned int u32Size;
    unsigned int submit_time;
    unsigned int complete_time;
    unsigned int done_time;
}PCDEV_WRITE_INFO;

typedef struct tagPCDEV_READ_BUFF_INFO
{
    unsigned int u32BuffSize;
}PCDEV_READ_BUFF_INFO;


typedef enum tagPCDEV_EVT_E
{
    PCDEV_EVT_DEV_SUSPEND = 0,
    PCDEV_EVT_DEV_READY = 1,
    PCDEV_EVT_DEV_BOTTOM
}PCDEV_EVT_E;

typedef void (*PCDEV_WRITE_DONE_CB_T)(char *pVirAddr, char *pPhyAddr, int size);
typedef void (*PCDEV_WRITE_INFO_DONE_CB_T)(PCDEV_WRITE_INFO *write_info);
typedef void (*PCDEV_READ_DONE_CB_T)(void);
typedef void (*PCDEV_EVENT_CB_T)(PCDEV_EVT_E evt);
typedef void (*PCDEV_ENUM_DONE_CB_T)(void);
typedef void (*PCDEV_DISABLE_CB_T)(void);

unsigned int mdrv_pcdev_reg_enumdonecb(PCDEV_ENUM_DONE_CB_T pFunc);
unsigned int mdrv_pcdev_reg_disablecb(PCDEV_ENUM_DONE_CB_T pFunc);

#ifdef _cplusplus
}
#endif
#endif

