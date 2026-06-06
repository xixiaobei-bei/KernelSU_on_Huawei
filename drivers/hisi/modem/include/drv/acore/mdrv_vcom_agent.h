#ifndef __MDRV_VCOM_AGENT_H__
#define __MDRV_VCOM_AGENT_H__
#include <mdrv_vcom.h>

#ifdef  __cplusplus
#if  __cplusplus
extern "C"{
#endif
#endif

typedef int (*COM_TX_CB_F)( enum com_id_e id, const unsigned char* data, unsigned int size);
typedef int (*COM_RX_CB_F)( enum com_id_e id, unsigned char* data, unsigned int size);

int mdrv_com_rxcb_reg(enum com_id_e id, COM_RX_CB_F func);

int mdrv_com_txcb_reg(enum com_id_e id, COM_TX_CB_F func);

int mdrv_com_write(enum com_id_e id, const unsigned char* data, unsigned int s);

#ifdef  __cplusplus
#if  __cplusplus
}
#endif
#endif

#endif
