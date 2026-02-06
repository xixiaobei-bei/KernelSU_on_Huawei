

#ifndef __BFGX_IOCTRL_H__
#define __BFGX_IOCTRL_H__

// BT
#define BT_IOCTL_FAKE_CLOSE_CMD     100
#define BT_IOCTL_HCISETPROTO        101
#define BT_IOCTL_HCIUNSETPROTO      102
#define BT_IOCTL_OPEN               103
#define BT_IOCTL_RELEASE            104

#define BT_FAKE_CLOSE               1
#define BT_REAL_CLOSE               0

// GNSS
#define GNSS_SET_READ_TIME     1

/* gnss update para */
#define PLAT_GNSS_MAGIC             'w'
#define PLAT_GNSS_DCXO_SET_PARA_CMD _IOW(PLAT_GNSS_MAGIC, 1, int)
#define PLAT_GNSS_ABB_CLK_PARA_CMD  _IOW(PLAT_GNSS_MAGIC, 2, int)
#define PLAT_GNSS_SLEEP_VOTE_CMD    _IOW(PLAT_GNSS_MAGIC, 4, int)

/* gnss abb clk control cmd */
#define GNSS_ABB_CLK_ENABLE  1
#define GNSS_ABB_CLK_DISABLE 0

/* gnss sleep vote */
#define AGREE_SLEEP     1
#define NOT_AGREE_SLEEP 0

/* fm */
#define FM_SET_READ_TIME     1

#endif /* __BFGX_IOCTRL_H__ */
