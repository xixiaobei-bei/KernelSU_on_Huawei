/* drv_ipc.h */

#ifndef __DRV_IPC_H__
#define __DRV_IPC_H__

//#include <linux/hisi/hisi_mailbox.h>
#include <linux/errno.h>
#include <linux/hisi/hisi_rproc.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#define HISI_RPROC_TX_TS 0
#define HISI_RPROC_TX_LPM3 1
#define HISI_RPROC_RX_TS_MBX4 2		/*ACPU0*/
#define HISI_RPROC_RX_LPM3_MBX5 3		/*ACPU0*/

#define HISI_RPROC_RX_TS_MBX6 4		/*ACPU1*/
#define HISI_RPROC_RX_LPM3_MBX7 5		/*ACPU1*/

#define HISI_RPROC_RX_TS_MBX8 6		/*ACPU2*/
#define HISI_RPROC_RX_LPM3_MBX9 7		/*ACPU2*/

#define HISI_RPROC_RX_TS_MBX10 8 	/*ACPU3*/
#define HISI_RPROC_RX_LPM3_MBX11 9		/*ACPU3*/

#define IPCDRV_RPROC_MSG_LENGTH 	(8)
#define IPCDRV_MSG_LENGTH		(28)

struct ipcdrv_msg_header {
	u32 msg_type:1;
	u32 cmd_type:7;
	u32 sync_type:1;
	u32 reserved:1;
	u32 msg_length:14;
	u32 msg_index:8;
};

struct ipcdrv_message {
	struct ipcdrv_msg_header ipc_msg_header;
	u8 ipcdrv_payload[IPCDRV_MSG_LENGTH];
};

struct ipcdrv_msg {
	struct ipcdrv_message ipc_message;
	u8 channel_type;
	void (*eventCallbackfunc)(int result, void *args);
};

enum ipc_msg_type{
	MSGTYPE_DRIVER_SEND = 0,
	MSGTYPE_DRIVER_RECEIVE = 1,
};

enum ipc_msg_sync_type {
	IPCDRV_MSG_SYNC = 0,
	IPCDRV_MSG_ASYNC = 1,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* __DRV_IPC_H__ */
