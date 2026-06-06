#ifndef __MDRV_PCV_H__
#define __MDRV_PCV_H__

#include <linux/skbuff.h>

/**
 * mdrv_rx_cb - callback protype receives skbuff from PC5
 * @skb: skbuff from PC5
 * if failed return non-zero
 */
typedef int (*mdrv_rx_cb)(struct sk_buff* skb);

/**
 * mdrv_pcv_cb_register - callback register
 * @cb: ul rx callback 
 * if failed return non-zero
 */
int mdrv_pcv_cb_register(mdrv_rx_cb cb);

/**
 * mdrv_pcv_xmit - transmit a PC5 skbuff to PC5 interface layer
 * @skb: skbuff send to PC5
 * if failed return non-zero
 */
int mdrv_pcv_xmit(struct sk_buff* skb);

#endif
