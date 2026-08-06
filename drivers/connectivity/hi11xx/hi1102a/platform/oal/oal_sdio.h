

#ifndef __OAL_SDIO_H__
#define __OAL_SDIO_H__

#ifdef CONFIG_HI110X_SDIO_STD_CARD_SUPPORT
#define HISDIO_VENDOR_ID_HI1102  0x0296 /* VENDOR 1102 */
#define HISDIO_VENDOR_ID_HI1103  0x12D1 /* VENDOR 1103 */
#define HISDIO_VENDOR_ID_HI1102A 0x12D1 /* VENDOR 1102A */

#define HISDIO_PRODUCT_ID_HISI       0x5347 /* Product 110x */
#define HISDIO_PRODUCT_ID_1102A_HISI 0x5348 /* Product 1102a */

#else
#define HISDIO_VENDOR_ID_HI1102  0x22D1 /* VENDOR 1102 */
#define HISDIO_VENDOR_ID_HI1103  0x22D1 /* VENDOR 1103 */
#define HISDIO_VENDOR_ID_HI1102A 0x22D1 /* VENDOR 1103 */

#define HISDIO_PRODUCT_ID_HISI 0x1101 /* Product 110x */
#endif

#define SDIO_MAX_CONTINUS_RX_COUNT 64 /* sdio max continus rx cnt */

#define HISDIO_REG_FUNC1_FIFO              0x00 /* Read Write FIFO */
#define HISDIO_REG_FUNC1_INT_STATUS        0x08 /* interrupt mask and clear reg */
#define HISDIO_REG_FUNC1_INT_ENABLE        0x09 /* interrupt */
#define HISDIO_REG_FUNC1_XFER_COUNT        0x0c /* notify number of bytes to be read */
#define HISDIO_REG_FUNC1_WRITE_MSG         0x24 /* write msg to device */
#define HISDIO_REG_FUNC1_MSG_FROM_DEV      0x28 /* notify Host that device has got the msg */
#define HISDIO_REG_FUNC1_MSG_HIGH_FROM_DEV 0x2b /* notify Host that device has got the msg, Host receive the msg ack */

/* sdio extend function, add 64B register for hcc */
#define HISDIO_FUNC1_EXTEND_REG_BASE 0x3c
#define HISDIO_FUNC1_EXTEND_REG_LEN  64

#define HISDIO_FUNC1_INT_DREADY (1 << 0) /* data ready interrupt */
#define HISDIO_FUNC1_INT_RERROR (1 << 1) /* data read error interrupt */
#define HISDIO_FUNC1_INT_MFARM  (1 << 2) /* ARM Msg interrupt */
#define HISDIO_FUNC1_INT_ACK    (1 << 3) /* ACK interrupt */

#define HISDIO_FUNC1_INT_MASK (HISDIO_FUNC1_INT_DREADY | HISDIO_FUNC1_INT_RERROR | HISDIO_FUNC1_INT_MFARM)

/* Device to Host Msg */

#endif /* end of oal_sdio.h */

