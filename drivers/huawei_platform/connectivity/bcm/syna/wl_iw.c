/*
 * Linux Wireless Extensions support
 *
 * Copyright (C) 1999-2017, Broadcom Corporation
 * 
 *      Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed to you
 * under the terms of the GNU General Public License version 2 (the "GPL"),
 * available at http://www.broadcom.com/licenses/GPLv2.php, with the
 * following added to such license:
 * 
 *      As a special exception, the copyright holders of this software give you
 * permission to link this software with independent modules, and to copy and
 * distribute the resulting executable under terms of your choice, provided that
 * you also meet, for each linked independent module, the terms and conditions of
 * the license of that module.  An independent module is a module which is not
 * derived from this software.  The special exception does not apply to any
 * modifications of the software.
 * 
 *      Notwithstanding the above, under no circumstances may you combine this
 * software in any way with any other Broadcom software provided under a license
 * other than the GPL, without Broadcom's express prior written consent.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: wl_iw.c 616333 2016-02-01 05:30:29Z $
 */

#if defined(USE_IW)
#define LINUX_PORT

#include <typedefs.h>
#include <linuxver.h>
#include <osl.h>

#include <bcmutils.h>
#include <bcmendian.h>
#include <ethernet.h>

#include <linux/if_arp.h>
#include <asm/uaccess.h>
#include <wlioctl.h>
#ifdef WL_IW_NAN
#include <wlioctl_utils.h>
#endif
#include <wl_iw.h>
#include <wl_android.h>
#ifdef WL_ESCAN
#include <wl_escan.h>
#endif
#include <dhd_config.h>
#include <dhd_debug.h>

uint iw_msg_level = WL_ERROR_LEVEL;

#define WL_ERROR_MSG(x, args...) \
	do { \
		if (iw_msg_level & WL_ERROR_LEVEL) { \
			printk(KERN_ERR "[dhd] WEXT-ERROR) %s : " x, __func__, ## args); \
		} \
	} while (0)
#define WL_TRACE_MSG(x, args...) \
	do { \
		if (iw_msg_level & WL_TRACE_LEVEL) { \
			printk(KERN_INFO "[dhd] WEXT-TRACE) %s : " x, __func__, ## args); \
		} \
	} while (0)
#define WL_SCAN_MSG(x, args...) \
	do { \
		if (iw_msg_level & WL_SCAN_LEVEL) { \
			printk(KERN_INFO "[dhd] WEXT-SCAN) %s : " x, __func__, ## args); \
		} \
	} while (0)
#define WL_WSEC_MSG(x, args...) \
	do { \
		if (iw_msg_level & WL_WSEC_LEVEL) { \
			printk(KERN_INFO "[dhd] WEXT-WSEC) %s : " x, __func__, ## args); \
		} \
	} while (0)
#define WL_ERROR(x) WL_ERROR_MSG x
#define WL_TRACE(x) WL_TRACE_MSG x
#define WL_SCAN(x) WL_SCAN_MSG x
#define WL_WSEC(x) WL_WSEC_MSG x
 
#ifdef BCMWAPI_WPI
/* these items should evetually go into wireless.h of the linux system headfile dir */
#ifndef IW_ENCODE_ALG_SM4
#define IW_ENCODE_ALG_SM4 0x20
#endif

#ifndef IW_AUTH_WAPI_ENABLED
#define IW_AUTH_WAPI_ENABLED 0x20
#endif

#ifndef IW_AUTH_WAPI_VERSION_1
#define IW_AUTH_WAPI_VERSION_1	0x00000008
#endif

#ifndef IW_AUTH_CIPHER_SMS4
#define IW_AUTH_CIPHER_SMS4	0x00000020
#endif

#ifndef IW_AUTH_KEY_MGMT_WAPI_PSK
#define IW_AUTH_KEY_MGMT_WAPI_PSK 4
#endif

#ifndef IW_AUTH_KEY_MGMT_WAPI_CERT
#define IW_AUTH_KEY_MGMT_WAPI_CERT 8
#endif
#endif /* BCMWAPI_WPI */

/* Broadcom extensions to WEXT, linux upstream has obsoleted WEXT */
#ifndef IW_AUTH_KEY_MGMT_FT_802_1X
#define IW_AUTH_KEY_MGMT_FT_802_1X 0x04
#endif

#ifndef IW_AUTH_KEY_MGMT_FT_PSK
#define IW_AUTH_KEY_MGMT_FT_PSK 0x08
#endif

#ifndef IW_ENC_CAPA_FW_ROAM_ENABLE
#define IW_ENC_CAPA_FW_ROAM_ENABLE	0x00000020
#endif


/* FC9: wireless.h 2.6.25-14.fc9.i686 is missing these, even though WIRELESS_EXT is set to latest
 * version 22.
 */
#ifndef IW_ENCODE_ALG_PMK
#define IW_ENCODE_ALG_PMK 4
#endif
#ifndef IW_ENC_CAPA_4WAY_HANDSHAKE
#define IW_ENC_CAPA_4WAY_HANDSHAKE 0x00000010
#endif
/* End FC9. */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27))
#include <linux/rtnetlink.h>
#endif

extern bool wl_iw_conn_status_str(uint32 event_type, uint32 status,
	uint32 reason, char* stringBuf, uint buflen);

uint wl_msg_level = WL_ERROR_VAL;

#define MAX_WLIW_IOCTL_LEN WLC_IOCTL_MEDLEN

/* IOCTL swapping mode for Big Endian host with Little Endian dongle.  Default to off */
#define htod32(i) (i)
#define htod16(i) (i)
#define dtoh32(i) (i)
#define dtoh16(i) (i)
#define htodchanspec(i) (i)
#define dtohchanspec(i) (i)

extern struct iw_statistics *dhd_get_wireless_stats(struct net_device *dev);

#if WIRELESS_EXT < 19
#define IW_IOCTL_IDX(cmd)	((cmd) - SIOCIWFIRST)
#define IW_EVENT_IDX(cmd)	((cmd) - IWEVFIRST)
#endif /* WIRELESS_EXT < 19 */


#ifndef WL_ESCAN
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0))
#define DAEMONIZE(a)	do { \
	} while (0)
#elif ((LINUX_VERSION_CODE < KERNEL_VERSION(3, 8, 0)) && \
	(LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)))
#define DAEMONIZE(a) daemonize(a); \
	allow_signal(SIGKILL); \
	allow_signal(SIGTERM);
#else /* Linux 2.4 (w/o preemption patch) */
#define RAISE_RX_SOFTIRQ() \
	cpu_raise_softirq(smp_processor_id(), NET_RX_SOFTIRQ)
#define DAEMONIZE(a) daemonize(); \
	do { if (a) \
		strncpy(current->comm, a, MIN(sizeof(current->comm), (strlen(a) + 1))); \
	} while (0);
#endif /* LINUX_VERSION_CODE  */

#define ISCAN_STATE_IDLE   0
#define ISCAN_STATE_SCANING 1

/* the buf lengh can be WLC_IOCTL_MAXLEN (8K) to reduce iteration */
#define WLC_IW_ISCAN_MAXLEN   2048
typedef struct iscan_buf {
	struct iscan_buf * next;
	char   iscan_buf[WLC_IW_ISCAN_MAXLEN];
} iscan_buf_t;

typedef struct iscan_info {
	struct net_device *dev;
	timer_list_compat_t timer;
	uint32 timer_ms;
	uint32 timer_on;
	int    iscan_state;
	iscan_buf_t * list_hdr;
	iscan_buf_t * list_cur;

	/* Thread to work on iscan */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 7, 0))
	struct task_struct *kthread;
#endif
	long sysioc_pid;
	struct semaphore sysioc_sem;
	struct completion sysioc_exited;
	char ioctlbuf[WLC_IOCTL_SMLEN];
} iscan_info_t;
static void wl_iw_timerfunc(ulong data);
static int wl_iw_iscan(iscan_info_t *iscan, wlc_ssid_t *ssid, uint16 action);
#endif /* !WL_ESCAN */

struct pmk_list {
	pmkid_list_t pmkids;
	pmkid_t foo[MAXPMKID - 1];
};

typedef struct wl_wext_info {
	struct net_device *dev;
	dhd_pub_t *dhd;
	struct delayed_work pm_enable_work;
	struct mutex pm_sync;
	struct wl_conn_info conn_info;
	struct pmk_list pmk_list;
#ifndef WL_ESCAN
	struct iscan_info iscan;
#endif
} wl_wext_info_t;

/* priv_link becomes netdev->priv and is the link between netdev and wlif struct */
typedef struct priv_link {
	wl_iw_t *wliw;
} priv_link_t;

/* dev to priv_link */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24))
#define WL_DEV_LINK(dev)       (priv_link_t*)(dev->priv)
#else
#define WL_DEV_LINK(dev)       (priv_link_t*)netdev_priv(dev)
#endif

/* dev to wl_iw_t */
#define IW_DEV_IF(dev)          ((wl_iw_t*)(WL_DEV_LINK(dev))->wliw)

static int
dev_wlc_ioctl(
	struct net_device *dev,
	int cmd,
	void *arg,
	int len
)
{
	struct ifreq ifr;
	wl_ioctl_t ioc;
	mm_segment_t fs;
	int ret;

	memset(&ioc, 0, sizeof(ioc));
#ifdef CONFIG_COMPAT
	ioc.cmd = cmd | WLC_SPEC_FLAG;
#else
	ioc.cmd = cmd;
#endif
	ioc.buf = arg;
	ioc.len = len;

	strncpy(ifr.ifr_name, dev->name, sizeof(ifr.ifr_name));
	ifr.ifr_name[sizeof(ifr.ifr_name) - 1] = '\0';
	ifr.ifr_data = (caddr_t) &ioc;

	fs = get_fs();
	set_fs(get_ds());
#if defined(WL_USE_NETDEV_OPS)
	ret = dev->netdev_ops->ndo_do_ioctl(dev, &ifr, SIOCDEVPRIVATE);
#else
	ret = dev->do_ioctl(dev, &ifr, SIOCDEVPRIVATE);
#endif
	set_fs(fs);

	return ret;
}

/*
set named driver variable to int value and return error indication
calling example: dev_wlc_intvar_set(dev, "arate", rate)
*/

static int
dev_wlc_intvar_set(
	struct net_device *dev,
	char *name,
	int val)
{
	char buf[WLC_IOCTL_SMLEN];
	uint len;

	val = htod32(val);
	len = bcm_mkiovar(name, (char *)(&val), sizeof(val), buf, sizeof(buf));
	ASSERT(len);

	return (dev_wlc_ioctl(dev, WLC_SET_VAR, buf, len));
}

#ifndef WL_ESCAN
static int
dev_iw_iovar_setbuf(
	struct net_device *dev,
	char *iovar,
	void *param,
	int paramlen,
	void *bufptr,
	int buflen)
{
	int iolen;

	iolen = bcm_mkiovar(iovar, param, paramlen, bufptr, buflen);
	ASSERT(iolen);
	BCM_REFERENCE(iolen);

	return (dev_wlc_ioctl(dev, WLC_SET_VAR, bufptr, iolen));
}

static int
dev_iw_iovar_getbuf(
	struct net_device *dev,
	char *iovar,
	void *param,
	int paramlen,
	void *bufptr,
	int buflen)
{
	int iolen;

	iolen = bcm_mkiovar(iovar, param, paramlen, bufptr, buflen);
	ASSERT(iolen);
	BCM_REFERENCE(iolen);

	return (dev_wlc_ioctl(dev, WLC_GET_VAR, bufptr, buflen));
}
#endif

/*
get named driver variable to int value and return error indication
calling example: dev_wlc_bufvar_get(dev, "arate", &rate)
*/

static int
dev_wlc_bufvar_get(
	struct net_device *dev,
	char *name,
	char *buf, int buflen)
{
	char *ioctlbuf;
	int error;

	uint len;

	ioctlbuf = kmalloc(MAX_WLIW_IOCTL_LEN, GFP_KERNEL);
	if (!ioctlbuf)
		return -ENOMEM;
	len = bcm_mkiovar(name, NULL, 0, ioctlbuf, MAX_WLIW_IOCTL_LEN);
	ASSERT(len);
	BCM_REFERENCE(len);
	error = dev_wlc_ioctl(dev, WLC_GET_VAR, (void *)ioctlbuf, MAX_WLIW_IOCTL_LEN);
	if (!error)
		bcopy(ioctlbuf, buf, buflen);

	kfree(ioctlbuf);
	return (error);
}

/*
get named driver variable to int value and return error indication
calling example: dev_wlc_intvar_get(dev, "arate", &rate)
*/

static int
dev_wlc_intvar_get(
	struct net_device *dev,
	char *name,
	int *retval)
{
	union {
		char buf[WLC_IOCTL_SMLEN];
		int val;
	} var;
	int error;

	uint len;
	uint data_null;

	len = bcm_mkiovar(name, (char *)(&data_null), 0, (char *)(&var), sizeof(var.buf));
	ASSERT(len);
	error = dev_wlc_ioctl(dev, WLC_GET_VAR, (void *)&var, len);

	*retval = dtoh32(var.val);

	return (error);
}

/* Maintain backward compatibility */
#if WIRELESS_EXT < 13
struct iw_request_info
{
	__u16		cmd;		/* Wireless Extension command */
	__u16		flags;		/* More to come ;-) */
};

typedef int (*iw_handler)(struct net_device *dev, struct iw_request_info *info,
	void *wrqu, char *extra);
#endif /* WIRELESS_EXT < 13 */

#if WIRELESS_EXT > 12
static int
wl_iw_set_leddc(
	struct net_device *dev,
	struct iw_request_info *info,
	union iwreq_data *wrqu,
	char *extra
)
{
	int dc = *(int *)extra;
	int error;

	error = dev_wlc_intvar_set(dev, "leddc", dc);
	return error;
}

static int
wl_iw_set_vlanmode(
	struct net_device *dev,
	struct iw_request_info *info,
	union iwreq_data *wrqu,
	char *extra
)
{
	int mode = *(int *)extra;
	int error;

	mode = htod32(mode);
	error = dev_wlc_intvar_set(dev, "vlan_mode", mode);
	return error;
}

static int
wl_iw_set_pm(
	struct net_device *dev,
	struct iw_request_info *info,
	union iwreq_data *wrqu,
	char *extra
)
{
	int pm = *(int *)extra;
	int error;

	pm = htod32(pm);
	error = dev_wlc_ioctl(dev, WLC_SET_PM, &pm, sizeof(pm));
	return error;
}
#endif /* WIRELESS_EXT > 12 */

static void
wl_iw_update_connect_status(struct net_device *dev, enum wl_ext_status status)
{
#ifndef WL_CFG80211
	struct dhd_pub *dhd = dhd_get_pub(dev);
	int cur_eapol_status = 0;
	int wpa_auth = 0;
	int error = -EINVAL;
	wl_wext_info_t *wext_info = NULL;

	if (!dhd || !dhd->conf)
		return;
	wext_info = dhd->wext_info;
	cur_eapol_status = dhd->conf->eapol_status;

	if (status == WL_EXT_STATUS_CONNECTING) {
#ifdef WL_EXT_IAPSTA
		wl_ext_add_remove_pm_enable_work(dev, TRUE);
#endif /* WL_EXT_IAPSTA */
		if ((error = dev_wlc_intvar_get(dev, "wpa_auth", &wpa_auth))) {
			WL_ERROR(("wpa_auth get error %d\n", error));
			return;
		}
		if (wpa_auth & (WPA_AUTH_PSK|WPA2_AUTH_PSK))
			dhd->conf->eapol_status = EAPOL_STATUS_4WAY_START;
		else
			dhd->conf->eapol_status = EAPOL_STATUS_NONE;
	} else if (status == WL_EXT_STATUS_ADD_KEY) {
		dhd->conf->eapol_status = EAPOL_STATUS_4WAY_DONE;
		wake_up_interruptible(&dhd->conf->event_complete);
	} else if (status == WL_EXT_STATUS_DISCONNECTING) {
#ifdef WL_EXT_IAPSTA
		wl_ext_add_remove_pm_enable_work(dev, FALSE);
#endif /* WL_EXT_IAPSTA */
		if (cur_eapol_status >= EAPOL_STATUS_4WAY_START &&
				cur_eapol_status < EAPOL_STATUS_4WAY_DONE) {
			WL_ERROR(("WPA failed at %d\n", cur_eapol_status));
			dhd->conf->eapol_status = EAPOL_STATUS_NONE;
		} else if (cur_eapol_status >= EAPOL_STATUS_WSC_START &&
				cur_eapol_status < EAPOL_STATUS_WSC_DONE) {
			WL_ERROR(("WPS failed at %d\n", cur_eapol_status));
			dhd->conf->eapol_status = EAPOL_STATUS_NONE;
		}
	} else if (status == WL_EXT_STATUS_DISCONNECTED) {
		if (cur_eapol_status >= EAPOL_STATUS_4WAY_START &&
				cur_eapol_status < EAPOL_STATUS_4WAY_DONE) {
			WL_ERROR(("WPA failed at %d\n", cur_eapol_status));
			dhd->conf->eapol_status = EAPOL_STATUS_NONE;
			wake_up_interruptible(&dhd->conf->event_complete);
		} else if (cur_eapol_status >= EAPOL_STATUS_WSC_START &&
				cur_eapol_status < EAPOL_STATUS_WSC_DONE) {
			WL_ERROR(("WPS failed at %d\n", cur_eapol_status));
			dhd->conf->eapol_status = EAPOL_STATUS_NONE;
		}
	}
#endif
	return;
}

int
wl_iw_send_priv_event(
	struct net_device *dev,
	char *flag
)
{
	union iwreq_data wrqu;
	char extra[IW_CUSTOM_MAX + 1];
	int cmd;

	cmd = IWEVCUSTOM;
	memset(&wrqu, 0, sizeof(wrqu));
	if (strlen(flag) > sizeof(extra))
		return -1;

	strncpy(extra, flag, sizeof(extra));
	extra[sizeof(extra) - 1] = '\0';
	wrqu.data.length = strlen(extra);
	wireless_send_event(dev, cmd, &wrqu, extra);
	WL_TRACE(("Send IWEVCUSTOM Event as %s\n", extra));

	return 0;
}

static int
wl_iw_config_commit(
	struct net_device *dev,
	struct iw_request_info *info,
	void *zwrq,
	char *extra
)
{
	wlc_ssid_t ssid;
	int error;
	struct sockaddr bssid;

	WL_TRACE(("%s: SIOCSIWCOMMIT\n", dev->name));

	if ((error = dev_wlc_ioctl(dev, WLC_GET_SSID, &ssid, sizeof(ssid))))
		return error;

	ssid.SSID_len = dtoh32(ssid.SSID_len);

	if (!ssid.SSID_len)
		return 0;

	bzero(&bssid, sizeof(struct sockaddr));
	if ((error = dev_wlc_ioctl(dev, WLC_REASSOC, &bssid, ETHER_ADDR_LEN))) {
		WL_ERROR(("WLC_REASSOC failed (%d)\n", error));
		return error;
	}

	return 0;
}

static int
wl_iw_get_name(
	struct net_device *dev,
	struct iw_request_info *info,
	union iwreq_data *cwrq,
	char *extra
)
{
	int phytype, err;
	uint band[3];
	char cap[5];

	WL_TRACE(("%s: SIOCGIWNAME\n", dev->name));

	cap[0] = 0;
	if ((err = dev_wlc_ioctl(dev, WLC_GET_PHYTYPE, &phytype, sizeof(phytype))) < 0)
		goto done;
	if ((err = dev_wlc_ioctl(dev, WLC_GET_BANDLIST, band, sizeof(band))) < 0)
		goto done;

	band[0] = dtoh32(band[0]);
	switch (phytype) {
		case WLC_PHY_TYPE_A:
			strncpy(cap, "a", sizeof(cap));
			break;
		case WLC_PHY_TYPE_B:
			strncpy(cap, "b", sizeof(cap));
			break;
		case WLC_PHY_TYPE_G:
			if (band[0] >= 2)
				strncpy(cap, "abg", sizeof(cap));
			else
				strncpy(cap, "bg", sizeof(cap));
			break;
		case WLC_PHY_TYPE_N:
			if (band[0] >= 2)
				strncpy(cap, "abgn", sizeof(cap));
			else
				strncpy(cap, "bgn", sizeof(cap));
			break;
	}
done:
	(void)snprintf(cwrq->name, IFNAMSIZ, "IEEE 802.11%s", cap);

	return 0;
}

#define DHD_CHECK(dhd, dev) \
 	if (!dhd) { \
		WL_ERROR (("[dhd-%s] %s: dhd is NULL\n", dev->name, __FUNCTION__)); \
		return -ENODEV; \
	} \

static const iw_handler wl_iw_handler[] =
{
	(iw_handler) wl_iw_config_commit,	/* SIOCSIWCOMMIT */
	(iw_handler) wl_iw_get_name,		/* SIOCGIWNAME */
	(iw_handler) NULL,			/* SIOCSIWNWID */
	(iw_handler) NULL,			/* SIOCGIWNWID */
	(iw_handler) NULL,		/* SIOCSIWFREQ */
	(iw_handler) NULL,		/* SIOCGIWFREQ */
	(iw_handler) NULL,		/* SIOCSIWMODE */
	(iw_handler) NULL,		/* SIOCGIWMODE */
	(iw_handler) NULL,			/* SIOCSIWSENS */
	(iw_handler) NULL,			/* SIOCGIWSENS */
	(iw_handler) NULL,			/* SIOCSIWRANGE */
	(iw_handler) NULL,		/* SIOCGIWRANGE */
	(iw_handler) NULL,			/* SIOCSIWPRIV */
	(iw_handler) NULL,			/* SIOCGIWPRIV */
	(iw_handler) NULL,			/* SIOCSIWSTATS */
	(iw_handler) NULL,			/* SIOCGIWSTATS */
	(iw_handler) NULL,		/* SIOCSIWSPY */
	(iw_handler) NULL,		/* SIOCGIWSPY */
	(iw_handler) NULL,			/* -- hole -- */
	(iw_handler) NULL,			/* -- hole -- */
	(iw_handler) NULL,		/* SIOCSIWAP */
	(iw_handler) NULL,		/* SIOCGIWAP */
	(iw_handler) NULL,		/* SIOCSIWMLME */
	(iw_handler) NULL,			/* SIOCGIWAPLIST */
#if WIRELESS_EXT > 13
	(iw_handler) NULL,	/* SIOCSIWSCAN */
	(iw_handler) NULL,	/* SIOCGIWSCAN */
#else	/* WIRELESS_EXT > 13 */
	(iw_handler) NULL,			/* SIOCSIWSCAN */
	(iw_handler) NULL,			/* SIOCGIWSCAN */
#endif	/* WIRELESS_EXT > 13 */
	(iw_handler) NULL,		/* SIOCSIWESSID */
	(iw_handler) NULL,		/* SIOCGIWESSID */
	(iw_handler) NULL,		/* SIOCSIWNICKN */
	(iw_handler) NULL,		/* SIOCGIWNICKN */
	(iw_handler) NULL,			/* -- hole -- */
	(iw_handler) NULL,			/* -- hole -- */
	(iw_handler) NULL,		/* SIOCSIWRATE */
	(iw_handler) NULL,		/* SIOCGIWRATE */
	(iw_handler) NULL,		/* SIOCSIWRTS */
	(iw_handler) NULL,		/* SIOCGIWRTS */
	(iw_handler) NULL,		/* SIOCSIWFRAG */
	(iw_handler) NULL,		/* SIOCGIWFRAG */
	(iw_handler) NULL,		/* SIOCSIWTXPOW */
	(iw_handler) NULL,		/* SIOCGIWTXPOW */
#if WIRELESS_EXT > 10
	(iw_handler) NULL,		/* SIOCSIWRETRY */
	(iw_handler) NULL,		/* SIOCGIWRETRY */
#endif /* WIRELESS_EXT > 10 */
	(iw_handler) NULL,		/* SIOCSIWENCODE */
	(iw_handler) NULL,		/* SIOCGIWENCODE */
	(iw_handler) NULL,		/* SIOCSIWPOWER */
	(iw_handler) NULL,		/* SIOCGIWPOWER */
#if WIRELESS_EXT > 17
	(iw_handler) NULL,			/* -- hole -- */
	(iw_handler) NULL,			/* -- hole -- */
	(iw_handler) NULL,		/* SIOCSIWGENIE */
	(iw_handler) NULL,		/* SIOCGIWGENIE */
	(iw_handler) NULL,		/* SIOCSIWAUTH */
	(iw_handler) NULL,		/* SIOCGIWAUTH */
	(iw_handler) NULL,	/* SIOCSIWENCODEEXT */
	(iw_handler) NULL,	/* SIOCGIWENCODEEXT */
	(iw_handler) NULL,		/* SIOCSIWPMKSA */
#endif /* WIRELESS_EXT > 17 */
};

#if WIRELESS_EXT > 12
enum {
	WL_IW_SET_LEDDC = SIOCIWFIRSTPRIV,
	WL_IW_SET_VLANMODE,
	WL_IW_SET_PM,
	WL_IW_SET_LAST
};

static iw_handler wl_iw_priv_handler[] = {
	wl_iw_set_leddc,
	wl_iw_set_vlanmode,
	wl_iw_set_pm,
	NULL
};

static struct iw_priv_args wl_iw_priv_args[] = {
	{
		WL_IW_SET_LEDDC,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0,
		"set_leddc"
	},
	{
		WL_IW_SET_VLANMODE,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0,
		"set_vlanmode"
	},
	{
		WL_IW_SET_PM,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0,
		"set_pm"
	},
	{ 0, 0, 0, { 0 } }
};

const struct iw_handler_def wl_iw_handler_def =
{
	.num_standard = ARRAYSIZE(wl_iw_handler),
	.num_private = ARRAY_SIZE(wl_iw_priv_handler),
	.num_private_args = ARRAY_SIZE(wl_iw_priv_args),
	.standard = (const iw_handler *) wl_iw_handler,
	.private = wl_iw_priv_handler,
	.private_args = wl_iw_priv_args,
#if WIRELESS_EXT >= 19
	get_wireless_stats: dhd_get_wireless_stats,
#endif /* WIRELESS_EXT >= 19 */
	};
#endif /* WIRELESS_EXT > 12 */

int
wl_iw_ioctl(
	struct net_device *dev,
	struct ifreq *rq,
	int cmd
)
{
	struct iwreq *wrq = (struct iwreq *) rq;
	struct iw_request_info info;
	iw_handler handler;
	char *extra = NULL;
	size_t token_size = 1;
	int max_tokens = 0, ret = 0;
#ifndef WL_ESCAN
	struct dhd_pub *dhd = dhd_get_pub(dev);
	wl_wext_info_t *wext_info = NULL;
	iscan_info_t *iscan;

	DHD_CHECK(dhd, dev);
	wext_info = dhd->wext_info;
	iscan = &wext_info->iscan;
#endif

	if (cmd < SIOCIWFIRST ||
		IW_IOCTL_IDX(cmd) >= ARRAYSIZE(wl_iw_handler) ||
		!(handler = wl_iw_handler[IW_IOCTL_IDX(cmd)]))
		return -EOPNOTSUPP;

	switch (cmd) {

	case SIOCSIWESSID:
	case SIOCGIWESSID:
	case SIOCSIWNICKN:
	case SIOCGIWNICKN:
		max_tokens = IW_ESSID_MAX_SIZE + 1;
		break;

	case SIOCSIWENCODE:
	case SIOCGIWENCODE:
#if WIRELESS_EXT > 17
	case SIOCSIWENCODEEXT:
	case SIOCGIWENCODEEXT:
#endif
		max_tokens = IW_ENCODING_TOKEN_MAX;
		break;

	case SIOCGIWRANGE:
		max_tokens = sizeof(struct iw_range);
		break;

	case SIOCGIWAPLIST:
		token_size = sizeof(struct sockaddr) + sizeof(struct iw_quality);
		max_tokens = IW_MAX_AP;
		break;

#if WIRELESS_EXT > 13
	case SIOCGIWSCAN:
#ifndef WL_ESCAN
	if (iscan)
		max_tokens = wrq->u.data.length;
	else
#endif
		max_tokens = IW_SCAN_MAX_DATA;
		break;
#endif /* WIRELESS_EXT > 13 */

	case SIOCSIWSPY:
		token_size = sizeof(struct sockaddr);
		max_tokens = IW_MAX_SPY;
		break;

	case SIOCGIWSPY:
		token_size = sizeof(struct sockaddr) + sizeof(struct iw_quality);
		max_tokens = IW_MAX_SPY;
		break;
	default:
		break;
	}

	if (max_tokens && wrq->u.data.pointer) {
		if (wrq->u.data.length > max_tokens)
			return -E2BIG;

		if (!(extra = kmalloc(max_tokens * token_size, GFP_KERNEL)))
			return -ENOMEM;

		if (copy_from_user(extra, wrq->u.data.pointer, wrq->u.data.length * token_size)) {
			kfree(extra);
			return -EFAULT;
		}
	}

	info.cmd = cmd;
	info.flags = 0;

	ret = handler(dev, &info, &wrq->u, extra);

	if (extra) {
		if (copy_to_user(wrq->u.data.pointer, extra, wrq->u.data.length * token_size)) {
			kfree(extra);
			return -EFAULT;
		}

		kfree(extra);
	}

	return ret;
}

/* Convert a connection status event into a connection status string.
 * Returns TRUE if a matching connection status string was found.
 */
bool
wl_iw_conn_status_str(uint32 event_type, uint32 status, uint32 reason,
	char* stringBuf, uint buflen)
{
	typedef struct conn_fail_event_map_t {
		uint32 inEvent;			/* input: event type to match */
		uint32 inStatus;		/* input: event status code to match */
		uint32 inReason;		/* input: event reason code to match */
		const char* outName;	/* output: failure type */
		const char* outCause;	/* output: failure cause */
	} conn_fail_event_map_t;

	/* Map of WLC_E events to connection failure strings */
#	define WL_IW_DONT_CARE	9999
	const conn_fail_event_map_t event_map [] = {
		/* inEvent           inStatus                inReason         */
		/* outName outCause                                           */
		{WLC_E_SET_SSID,     WLC_E_STATUS_SUCCESS,   WL_IW_DONT_CARE,
		"Conn", "Success"},
		{WLC_E_SET_SSID,     WLC_E_STATUS_NO_NETWORKS, WL_IW_DONT_CARE,
		"Conn", "NoNetworks"},
		{WLC_E_SET_SSID,     WLC_E_STATUS_FAIL,      WL_IW_DONT_CARE,
		"Conn", "ConfigMismatch"},
		{WLC_E_PRUNE,        WL_IW_DONT_CARE,        WLC_E_PRUNE_ENCR_MISMATCH,
		"Conn", "EncrypMismatch"},
		{WLC_E_PRUNE,        WL_IW_DONT_CARE,        WLC_E_RSN_MISMATCH,
		"Conn", "RsnMismatch"},
		{WLC_E_AUTH,         WLC_E_STATUS_TIMEOUT,   WL_IW_DONT_CARE,
		"Conn", "AuthTimeout"},
		{WLC_E_AUTH,         WLC_E_STATUS_FAIL,      WL_IW_DONT_CARE,
		"Conn", "AuthFail"},
		{WLC_E_AUTH,         WLC_E_STATUS_NO_ACK,    WL_IW_DONT_CARE,
		"Conn", "AuthNoAck"},
		{WLC_E_REASSOC,      WLC_E_STATUS_FAIL,      WL_IW_DONT_CARE,
		"Conn", "ReassocFail"},
		{WLC_E_REASSOC,      WLC_E_STATUS_TIMEOUT,   WL_IW_DONT_CARE,
		"Conn", "ReassocTimeout"},
		{WLC_E_REASSOC,      WLC_E_STATUS_ABORT,     WL_IW_DONT_CARE,
		"Conn", "ReassocAbort"},
		{WLC_E_PSK_SUP,      WLC_SUP_KEYED,          WL_IW_DONT_CARE,
		"Sup", "ConnSuccess"},
		{WLC_E_PSK_SUP,      WL_IW_DONT_CARE,        WL_IW_DONT_CARE,
		"Sup", "WpaHandshakeFail"},
		{WLC_E_DEAUTH_IND,   WL_IW_DONT_CARE,        WL_IW_DONT_CARE,
		"Conn", "Deauth"},
		{WLC_E_DISASSOC_IND, WL_IW_DONT_CARE,        WL_IW_DONT_CARE,
		"Conn", "DisassocInd"},
		{WLC_E_DISASSOC,     WL_IW_DONT_CARE,        WL_IW_DONT_CARE,
		"Conn", "Disassoc"}
	};

	const char* name = "";
	const char* cause = NULL;
	int i;

	/* Search the event map table for a matching event */
	for (i = 0;  i < sizeof(event_map)/sizeof(event_map[0]);  i++) {
		const conn_fail_event_map_t* row = &event_map[i];
		if (row->inEvent == event_type &&
		    (row->inStatus == status || row->inStatus == WL_IW_DONT_CARE) &&
		    (row->inReason == reason || row->inReason == WL_IW_DONT_CARE)) {
			name = row->outName;
			cause = row->outCause;
			break;
		}
	}

	/* If found, generate a connection failure string and return TRUE */
	if (cause) {
		memset(stringBuf, 0, buflen);
		(void)snprintf(stringBuf, buflen, "%s %s %02d %02d", name, cause, status, reason);
		WL_TRACE(("Connection status: %s\n", stringBuf));
		return TRUE;
	} else {
		return FALSE;
	}
}

#if (WIRELESS_EXT > 14)
/* Check if we have received an event that indicates connection failure
 * If so, generate a connection failure report string.
 * The caller supplies a buffer to hold the generated string.
 */
static bool
wl_iw_check_conn_fail(wl_event_msg_t *e, char* stringBuf, uint buflen)
{
	uint32 event = ntoh32(e->event_type);
	uint32 status =  ntoh32(e->status);
	uint32 reason =  ntoh32(e->reason);

	if (wl_iw_conn_status_str(event, status, reason, stringBuf, buflen)) {
		return TRUE;
	} else
	{
		return FALSE;
	}
}
#endif /* WIRELESS_EXT > 14 */

#ifndef IW_CUSTOM_MAX
#define IW_CUSTOM_MAX 256 /* size of extra buffer used for translation of events */
#endif /* IW_CUSTOM_MAX */

void
wl_iw_event(struct net_device *dev, struct wl_wext_info *wext_info,
	wl_event_msg_t *e, void* data)
{
#if WIRELESS_EXT > 13
	union iwreq_data wrqu;
	char extra[IW_CUSTOM_MAX + 1];
	int cmd = 0;
	uint32 event_type = ntoh32(e->event_type);
	uint16 flags =  ntoh16(e->flags);
	uint32 datalen = ntoh32(e->datalen);
	uint32 status =  ntoh32(e->status);
	uint32 reason =  ntoh32(e->reason);
#ifndef WL_ESCAN
	iscan_info_t *iscan = &wext_info->iscan;
#endif

	memset(&wrqu, 0, sizeof(wrqu));
	memset(extra, 0, sizeof(extra));

	memcpy(wrqu.addr.sa_data, &e->addr, ETHER_ADDR_LEN);
	wrqu.addr.sa_family = ARPHRD_ETHER;

	switch (event_type) {
	case WLC_E_TXFAIL:
		cmd = IWEVTXDROP;
		break;
#if WIRELESS_EXT > 14
	case WLC_E_JOIN:
	case WLC_E_ASSOC_IND:
	case WLC_E_REASSOC_IND:
		cmd = IWEVREGISTERED;
		break;
	case WLC_E_DEAUTH:
	case WLC_E_DISASSOC:
		wl_iw_update_connect_status(dev, WL_EXT_STATUS_DISCONNECTED);
		WL_MSG_RLMT(dev->name, &e->addr, ETHER_ADDR_LEN,
			"disconnected with "MACSTR", event %d, reason %d\n",
			MAC2STR((u8 *)wrqu.addr.sa_data), event_type, reason);
		break;
	case WLC_E_DEAUTH_IND:
	case WLC_E_DISASSOC_IND:
		cmd = SIOCGIWAP;
		WL_MSG(dev->name, "disconnected with "MACSTR", event %d, reason %d\n",
			MAC2STR((u8 *)wrqu.addr.sa_data), event_type, reason);
		bzero(wrqu.addr.sa_data, ETHER_ADDR_LEN);
		bzero(&extra, ETHER_ADDR_LEN);
		wl_iw_update_connect_status(dev, WL_EXT_STATUS_DISCONNECTED);
		break;

	case WLC_E_LINK:
		cmd = SIOCGIWAP;
		if (!(flags & WLC_EVENT_MSG_LINK)) {
			WL_MSG(dev->name, "Link Down with "MACSTR", reason=%d\n",
				MAC2STR((u8 *)wrqu.addr.sa_data), reason);
			bzero(wrqu.addr.sa_data, ETHER_ADDR_LEN);
			bzero(&extra, ETHER_ADDR_LEN);
			wl_iw_update_connect_status(dev, WL_EXT_STATUS_DISCONNECTED);
		} else {
			WL_MSG(dev->name, "Link UP with "MACSTR"\n",
				MAC2STR((u8 *)wrqu.addr.sa_data));
		}
		break;
	case WLC_E_ACTION_FRAME:
		cmd = IWEVCUSTOM;
		if (datalen + 1 <= sizeof(extra)) {
			wrqu.data.length = datalen + 1;
			extra[0] = WLC_E_ACTION_FRAME;
			memcpy(&extra[1], data, datalen);
			WL_TRACE(("WLC_E_ACTION_FRAME len %d \n", wrqu.data.length));
		}
		break;

	case WLC_E_ACTION_FRAME_COMPLETE:
		cmd = IWEVCUSTOM;
		if (sizeof(status) + 1 <= sizeof(extra)) {
			wrqu.data.length = sizeof(status) + 1;
			extra[0] = WLC_E_ACTION_FRAME_COMPLETE;
			memcpy(&extra[1], &status, sizeof(status));
			WL_TRACE(("wl_iw_event status %d  \n", status));
		}
		break;
#endif /* WIRELESS_EXT > 14 */
#if WIRELESS_EXT > 17
	case WLC_E_MIC_ERROR: {
		struct	iw_michaelmicfailure  *micerrevt = (struct  iw_michaelmicfailure  *)&extra;
		cmd = IWEVMICHAELMICFAILURE;
		wrqu.data.length = sizeof(struct iw_michaelmicfailure);
		if (flags & WLC_EVENT_MSG_GROUP)
			micerrevt->flags |= IW_MICFAILURE_GROUP;
		else
			micerrevt->flags |= IW_MICFAILURE_PAIRWISE;
		memcpy(micerrevt->src_addr.sa_data, &e->addr, ETHER_ADDR_LEN);
		micerrevt->src_addr.sa_family = ARPHRD_ETHER;

		break;
	}

	case WLC_E_ASSOC_REQ_IE:
		cmd = IWEVASSOCREQIE;
		wrqu.data.length = datalen;
		if (datalen < sizeof(extra))
			memcpy(extra, data, datalen);
		break;

	case WLC_E_ASSOC_RESP_IE:
		cmd = IWEVASSOCRESPIE;
		wrqu.data.length = datalen;
		if (datalen < sizeof(extra))
			memcpy(extra, data, datalen);
		break;

	case WLC_E_PMKID_CACHE: {
		struct iw_pmkid_cand *iwpmkidcand = (struct iw_pmkid_cand *)&extra;
		pmkid_cand_list_t *pmkcandlist;
		pmkid_cand_t	*pmkidcand;
		int count;

		if (data == NULL)
			break;

		cmd = IWEVPMKIDCAND;
		pmkcandlist = data;
		count = ntoh32_ua((uint8 *)&pmkcandlist->npmkid_cand);
		wrqu.data.length = sizeof(struct iw_pmkid_cand);
		pmkidcand = pmkcandlist->pmkid_cand;
		while (count) {
			bzero(iwpmkidcand, sizeof(struct iw_pmkid_cand));
			if (pmkidcand->preauth)
				iwpmkidcand->flags |= IW_PMKID_CAND_PREAUTH;
			bcopy(&pmkidcand->BSSID, &iwpmkidcand->bssid.sa_data,
			      ETHER_ADDR_LEN);
			wireless_send_event(dev, cmd, &wrqu, extra);
			pmkidcand++;
			count--;
		}
		break;
	}
#endif /* WIRELESS_EXT > 17 */

#ifndef WL_ESCAN
	case WLC_E_SCAN_COMPLETE:
#if WIRELESS_EXT > 14
		cmd = SIOCGIWSCAN;
#endif
		WL_TRACE(("event WLC_E_SCAN_COMPLETE\n"));
		// terence 20150224: fix "wlan0: (WE) : Wireless Event too big (65306)"
		memset(&wrqu, 0, sizeof(wrqu));
		if ((iscan) && (iscan->sysioc_pid >= 0) &&
			(iscan->iscan_state != ISCAN_STATE_IDLE))
			up(&iscan->sysioc_sem);
		break;
#endif

	default:
		/* Cannot translate event */
		break;
	}

	if (cmd) {
#ifndef WL_ESCAN
		if (cmd == SIOCGIWSCAN) {
			if ((!iscan) || (iscan->sysioc_pid < 0)) {
				wireless_send_event(dev, cmd, &wrqu, NULL);
			}
		} else
#endif
			wireless_send_event(dev, cmd, &wrqu, extra);
	}

#if WIRELESS_EXT > 14
	/* Look for WLC events that indicate a connection failure.
	 * If found, generate an IWEVCUSTOM event.
	 */
	memset(extra, 0, sizeof(extra));
	if (wl_iw_check_conn_fail(e, extra, sizeof(extra))) {
		cmd = IWEVCUSTOM;
		wrqu.data.length = strlen(extra);
		wireless_send_event(dev, cmd, &wrqu, extra);
	}
#endif /* WIRELESS_EXT > 14 */

#endif /* WIRELESS_EXT > 13 */
}

#ifdef WL_IW_NAN
static int wl_iw_get_wireless_stats_cbfn(void *ctx, uint8 *data, uint16 type, uint16 len)
{
	struct iw_statistics *wstats = ctx;
	int res = BCME_OK;

	switch (type) {
		case WL_CNT_XTLV_WLC: {
			wl_cnt_wlc_t *cnt = (wl_cnt_wlc_t *)data;
			if (len > sizeof(wl_cnt_wlc_t)) {
				printf("counter structure length invalid! %d > %d\n",
					len, (int)sizeof(wl_cnt_wlc_t));
			}
			wstats->discard.nwid = 0;
			wstats->discard.code = dtoh32(cnt->rxundec);
			wstats->discard.fragment = dtoh32(cnt->rxfragerr);
			wstats->discard.retries = dtoh32(cnt->txfail);
			wstats->discard.misc = dtoh32(cnt->rxrunt) + dtoh32(cnt->rxgiant);
			wstats->miss.beacon = 0;
			WL_TRACE(("wl_iw_get_wireless_stats counters txframe=%d txbyte=%d\n",
				dtoh32(cnt->txframe), dtoh32(cnt->txbyte)));
			WL_TRACE(("wl_iw_get_wireless_stats counters rxundec=%d\n",
				dtoh32(cnt->rxundec)));
			WL_TRACE(("wl_iw_get_wireless_stats counters txfail=%d\n",
				dtoh32(cnt->txfail)));
			WL_TRACE(("wl_iw_get_wireless_stats counters rxfragerr=%d\n",
				dtoh32(cnt->rxfragerr)));
			WL_TRACE(("wl_iw_get_wireless_stats counters rxrunt=%d\n",
				dtoh32(cnt->rxrunt)));
			WL_TRACE(("wl_iw_get_wireless_stats counters rxgiant=%d\n",
				dtoh32(cnt->rxgiant)));
			break;
		}
		case WL_CNT_XTLV_CNTV_LE10_UCODE:
		case WL_CNT_XTLV_LT40_UCODE_V1:
		case WL_CNT_XTLV_GE40_UCODE_V1:
		{
			/* Offsets of rxfrmtoolong and rxbadplcp are the same in
			 * wl_cnt_v_le10_mcst_t, wl_cnt_lt40mcst_v1_t, and wl_cnt_ge40mcst_v1_t.
			 * So we can just cast to wl_cnt_v_le10_mcst_t here.
			 */
			wl_cnt_v_le10_mcst_t *cnt = (wl_cnt_v_le10_mcst_t *)data;
			if (len != WL_CNT_MCST_STRUCT_SZ) {
				printf("counter structure length mismatch! %d != %d\n",
					len, WL_CNT_MCST_STRUCT_SZ);
			}
			WL_TRACE(("wl_iw_get_wireless_stats counters rxfrmtoolong=%d\n",
				dtoh32(cnt->rxfrmtoolong)));
			WL_TRACE(("wl_iw_get_wireless_stats counters rxbadplcp=%d\n",
				dtoh32(cnt->rxbadplcp)));
			BCM_REFERENCE(cnt);
			break;
		}
		default:
			WL_ERROR(("%d: Unsupported type %d\n", __LINE__, type));
			break;
	}
	return res;
}
#endif

int wl_iw_get_wireless_stats(struct net_device *dev, struct iw_statistics *wstats)
{
	int res = 0;
	int phy_noise;
	int rssi;
	scb_val_t scb_val;
#if WIRELESS_EXT > 11
	char *cntbuf = NULL;
	wl_cnt_info_t *cntinfo;
	uint16 ver;
	uint32 corerev = 0;
#endif /* WIRELESS_EXT > 11 */

	phy_noise = 0;
	if ((res = dev_wlc_ioctl(dev, WLC_GET_PHY_NOISE, &phy_noise, sizeof(phy_noise)))) {
		WL_TRACE(("WLC_GET_PHY_NOISE error=%d\n", res));
		goto done;
	}

	phy_noise = dtoh32(phy_noise);
	WL_TRACE(("wl_iw_get_wireless_stats phy noise=%d\n *****", phy_noise));

	memset(&scb_val, 0, sizeof(scb_val));
	if ((res = dev_wlc_ioctl(dev, WLC_GET_RSSI, &scb_val, sizeof(scb_val_t)))) {
		WL_TRACE(("WLC_GET_RSSI error=%d\n", res));
		goto done;
	}

	rssi = dtoh32(scb_val.val);
	rssi = MIN(rssi, RSSI_MAXVAL);
	WL_TRACE(("wl_iw_get_wireless_stats rssi=%d ****** \n", rssi));
	if (rssi <= WL_IW_RSSI_NO_SIGNAL)
		wstats->qual.qual = 0;
	else if (rssi <= WL_IW_RSSI_VERY_LOW)
		wstats->qual.qual = 1;
	else if (rssi <= WL_IW_RSSI_LOW)
		wstats->qual.qual = 2;
	else if (rssi <= WL_IW_RSSI_GOOD)
		wstats->qual.qual = 3;
	else if (rssi <= WL_IW_RSSI_VERY_GOOD)
		wstats->qual.qual = 4;
	else
		wstats->qual.qual = 5;

	/* Wraps to 0 if RSSI is 0 */
	wstats->qual.level = 0x100 + rssi;
	wstats->qual.noise = 0x100 + phy_noise;
#if WIRELESS_EXT > 18
	wstats->qual.updated |= (IW_QUAL_ALL_UPDATED | IW_QUAL_DBM);
#else
	wstats->qual.updated |= 7;
#endif /* WIRELESS_EXT > 18 */

#if WIRELESS_EXT > 11
	WL_TRACE(("wl_iw_get_wireless_stats counters\n *****"));

	cntbuf = kmalloc(MAX_WLIW_IOCTL_LEN, GFP_KERNEL);
	if (!cntbuf) {
		res = BCME_NOMEM;
		goto done;
	}

	memset(cntbuf, 0, MAX_WLIW_IOCTL_LEN);
	res = dev_wlc_bufvar_get(dev, "counters", cntbuf, MAX_WLIW_IOCTL_LEN);
	if (res)
	{
		WL_ERROR(("wl_iw_get_wireless_stats counters failed error=%d ****** \n", res));
		goto done;
	}

	cntinfo = (wl_cnt_info_t *)cntbuf;
	cntinfo->version = dtoh16(cntinfo->version);
	cntinfo->datalen = dtoh16(cntinfo->datalen);
	ver = cntinfo->version;
#ifdef WL_IW_NAN
	CHK_CNTBUF_DATALEN(cntbuf, MAX_WLIW_IOCTL_LEN);
#endif
	if (ver > WL_CNT_T_VERSION) {
		WL_TRACE(("\tIncorrect version of counters struct: expected %d; got %d\n",
			WL_CNT_T_VERSION, ver));
		res = BCME_VERSION;
		goto done;
	}

	if (ver == WL_CNT_VERSION_11) {
		wlc_rev_info_t revinfo;
		memset(&revinfo, 0, sizeof(revinfo));
		res = dev_wlc_ioctl(dev, WLC_GET_REVINFO, &revinfo, sizeof(revinfo));
		if (res) {
			WL_ERROR(("WLC_GET_REVINFO failed %d\n", res));
			goto done;
		}
		corerev = dtoh32(revinfo.corerev);
	}

#ifdef WL_IW_NAN
	res = wl_cntbuf_to_xtlv_format(NULL, cntinfo, MAX_WLIW_IOCTL_LEN, corerev);
	if (res) {
		WL_ERROR(("wl_cntbuf_to_xtlv_format failed %d\n", res));
		goto done;
	}

	if ((res = bcm_unpack_xtlv_buf(wstats, cntinfo->data, cntinfo->datalen,
		BCM_XTLV_OPTION_ALIGN32, wl_iw_get_wireless_stats_cbfn))) {
		goto done;
	}
#endif
#endif /* WIRELESS_EXT > 11 */

done:
#if WIRELESS_EXT > 11
	if (cntbuf) {
		kfree(cntbuf);
	}
#endif /* WIRELESS_EXT > 11 */
	return res;
}

#ifndef WL_ESCAN
static void
wl_iw_timerfunc(ulong data)
{
	iscan_info_t *iscan = (iscan_info_t *)data;
	iscan->timer_on = 0;
	if (iscan->iscan_state != ISCAN_STATE_IDLE) {
		WL_TRACE(("timer trigger\n"));
		up(&iscan->sysioc_sem);
	}
}

static int
wl_iw_iscan_prep(wl_scan_params_t *params, wlc_ssid_t *ssid)
{
	int err = 0;

	memcpy(&params->bssid, &ether_bcast, ETHER_ADDR_LEN);
	params->bss_type = DOT11_BSSTYPE_ANY;
	params->scan_type = 0;
	params->nprobes = -1;
	params->active_time = -1;
	params->passive_time = -1;
	params->home_time = -1;
	params->channel_num = 0;

	params->nprobes = htod32(params->nprobes);
	params->active_time = htod32(params->active_time);
	params->passive_time = htod32(params->passive_time);
	params->home_time = htod32(params->home_time);
	if (ssid && ssid->SSID_len)
		memcpy(&params->ssid, ssid, sizeof(wlc_ssid_t));

	return err;
}

static int
wl_iw_iscan(iscan_info_t *iscan, wlc_ssid_t *ssid, uint16 action)
{
	int params_size = (WL_SCAN_PARAMS_FIXED_SIZE + OFFSETOF(wl_iscan_params_t, params));
	wl_iscan_params_t *params;
	int err = 0;

	if (ssid && ssid->SSID_len) {
		params_size += sizeof(wlc_ssid_t);
	}
	params = (wl_iscan_params_t*)kmalloc(params_size, GFP_KERNEL);
	if (params == NULL) {
		return -ENOMEM;
	}
	memset(params, 0, params_size);
	ASSERT(params_size < WLC_IOCTL_SMLEN);

	err = wl_iw_iscan_prep(&params->params, ssid);

	if (!err) {
		params->version = htod32(ISCAN_REQ_VERSION);
		params->action = htod16(action);
		params->scan_duration = htod16(0);

		/* params_size += OFFSETOF(wl_iscan_params_t, params); */
		(void) dev_iw_iovar_setbuf(iscan->dev, "iscan", params, params_size,
			iscan->ioctlbuf, WLC_IOCTL_SMLEN);
	}

	kfree(params);
	return err;
}

static uint32
wl_iw_iscan_get(iscan_info_t *iscan)
{
	iscan_buf_t * buf;
	iscan_buf_t * ptr;
	wl_iscan_results_t * list_buf;
	wl_iscan_results_t list;
	wl_scan_results_t *results;
	uint32 status;

	/* buffers are allocated on demand */
	if (iscan->list_cur) {
		buf = iscan->list_cur;
		iscan->list_cur = buf->next;
	}
	else {
		buf = kmalloc(sizeof(iscan_buf_t), GFP_KERNEL);
		if (!buf)
			return WL_SCAN_RESULTS_ABORTED;
		buf->next = NULL;
		if (!iscan->list_hdr)
			iscan->list_hdr = buf;
		else {
			ptr = iscan->list_hdr;
			while (ptr->next) {
				ptr = ptr->next;
			}
			ptr->next = buf;
		}
	}
	memset(buf->iscan_buf, 0, WLC_IW_ISCAN_MAXLEN);
	list_buf = (wl_iscan_results_t*)buf->iscan_buf;
	results = &list_buf->results;
	results->buflen = WL_ISCAN_RESULTS_FIXED_SIZE;
	results->version = 0;
	results->count = 0;

	memset(&list, 0, sizeof(list));
	list.results.buflen = htod32(WLC_IW_ISCAN_MAXLEN);
	(void) dev_iw_iovar_getbuf(
		iscan->dev,
		"iscanresults",
		&list,
		WL_ISCAN_RESULTS_FIXED_SIZE,
		buf->iscan_buf,
		WLC_IW_ISCAN_MAXLEN);
	results->buflen = dtoh32(results->buflen);
	results->version = dtoh32(results->version);
	results->count = dtoh32(results->count);
	WL_TRACE(("results->count = %d\n", results->count));

	WL_TRACE(("results->buflen = %d\n", results->buflen));
	status = dtoh32(list_buf->status);
	return status;
}

static void wl_iw_send_scan_complete(iscan_info_t *iscan)
{
	union iwreq_data wrqu;

	memset(&wrqu, 0, sizeof(wrqu));

	/* wext expects to get no data for SIOCGIWSCAN Event  */
	wireless_send_event(iscan->dev, SIOCGIWSCAN, &wrqu, NULL);
}

static int
_iscan_sysioc_thread(void *data)
{
	uint32 status;
	iscan_info_t *iscan = (iscan_info_t *)data;

	WL_MSG("wlan", "thread Enter\n");
	DAEMONIZE("iscan_sysioc");

	status = WL_SCAN_RESULTS_PARTIAL;
	while (down_interruptible(&iscan->sysioc_sem) == 0) {
		if (iscan->timer_on) {
			del_timer(&iscan->timer);
			iscan->timer_on = 0;
		}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27))
		rtnl_lock();
#endif
		status = wl_iw_iscan_get(iscan);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27))
		rtnl_unlock();
#endif

		switch (status) {
			case WL_SCAN_RESULTS_PARTIAL:
				WL_TRACE(("iscanresults incomplete\n"));
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27))
				rtnl_lock();
#endif
				/* make sure our buffer size is enough before going next round */
				wl_iw_iscan(iscan, NULL, WL_SCAN_ACTION_CONTINUE);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27))
				rtnl_unlock();
#endif
				/* Reschedule the timer */
#ifdef WL_HAS_EXT_SUPP
				iscan->timer.expires = jiffies + msecs_to_jiffies(iscan->timer_ms);
#endif
				add_timer(&iscan->timer);
				iscan->timer_on = 1;
				break;
			case WL_SCAN_RESULTS_SUCCESS:
				WL_TRACE(("iscanresults complete\n"));
				iscan->iscan_state = ISCAN_STATE_IDLE;
				wl_iw_send_scan_complete(iscan);
				break;
			case WL_SCAN_RESULTS_PENDING:
				WL_TRACE(("iscanresults pending\n"));
				/* Reschedule the timer */
#ifdef WL_HAS_EXT_SUPP
				iscan->timer.expires = jiffies + msecs_to_jiffies(iscan->timer_ms);
#endif
				add_timer(&iscan->timer);
				iscan->timer_on = 1;
				break;
			case WL_SCAN_RESULTS_ABORTED:
				WL_TRACE(("iscanresults aborted\n"));
				iscan->iscan_state = ISCAN_STATE_IDLE;
				wl_iw_send_scan_complete(iscan);
				break;
			default:
				WL_TRACE(("iscanresults returned unknown status %d\n", status));
				break;
		 }
	}
	WL_MSG("wlan", "was terminated\n");
	complete_and_exit(&iscan->sysioc_exited, 0);
}
#endif /* !WL_ESCAN */

void
wl_iw_detach(struct net_device *dev, dhd_pub_t *dhdp)
{
	wl_wext_info_t *wext_info = dhdp->wext_info;
#ifndef WL_ESCAN
	iscan_buf_t  *buf;
	iscan_info_t *iscan;
#endif
	if (!wext_info)
		return;

#ifndef WL_ESCAN
	iscan = &wext_info->iscan;
	if (iscan->sysioc_pid >= 0) {
		KILL_PROC(iscan->sysioc_pid, SIGTERM);
		wait_for_completion(&iscan->sysioc_exited);
	}

	while (iscan->list_hdr) {
		buf = iscan->list_hdr->next;
		kfree(iscan->list_hdr);
		iscan->list_hdr = buf;
	}
#endif
#ifdef WL_HAS_EXT_SUPP
	wl_ext_event_deregister(dev, dhdp, WLC_E_LAST, wl_iw_event);
#endif
	if (wext_info) {
		kfree(wext_info);
		dhdp->wext_info = NULL;
	}
}

int
wl_iw_attach(struct net_device *dev, dhd_pub_t *dhdp)
{
	wl_wext_info_t *wext_info = NULL;
	int ret = 0;
#ifndef WL_ESCAN
	iscan_info_t *iscan = NULL;
#endif

	if (!dev)
		return 0;
	WL_TRACE(("Enter\n"));

	wext_info = (void *)kzalloc(sizeof(struct wl_wext_info), GFP_KERNEL);
	if (!wext_info)
		return -ENOMEM;
	memset(wext_info, 0, sizeof(wl_wext_info_t));
	wext_info->dev = dev;
	wext_info->dhd = dhdp;
	wext_info->conn_info.bssidx = 0;
	dhdp->wext_info = (void *)wext_info;

#ifndef WL_ESCAN
	iscan = &wext_info->iscan;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 7, 0))
	iscan->kthread = NULL;
#endif
	iscan->sysioc_pid = -1;
	/* we only care about main interface so save a global here */
	iscan->dev = dev;
	iscan->iscan_state = ISCAN_STATE_IDLE;

	/* Set up the timer */
	iscan->timer_ms    = 2000;
	init_timer_compat(&iscan->timer, wl_iw_timerfunc, iscan);

	sema_init(&iscan->sysioc_sem, 0);
	init_completion(&iscan->sysioc_exited);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 7, 0))
	iscan->kthread = kthread_run(_iscan_sysioc_thread, iscan, "iscan_sysioc");
	iscan->sysioc_pid = iscan->kthread->pid;
#else
	iscan->sysioc_pid = kernel_thread(_iscan_sysioc_thread, iscan, 0);
#endif
	if (iscan->sysioc_pid < 0) {
		ret = -ENOMEM;
		goto exit;
	}
#endif
#ifdef WL_HAS_EXT_SUPP
	ret = wl_ext_event_register(dev, dhdp, WLC_E_LAST, wl_iw_event, dhdp->wext_info,
		PRIO_EVENT_WEXT);
#endif
	if (ret) {
		WL_ERROR(("wl_ext_event_register err %d\n", ret));
		goto exit;
	}

	return ret;
exit:
	wl_iw_detach(dev, dhdp);
	return ret;
}

void
wl_iw_down(struct net_device *dev, dhd_pub_t *dhdp)
{
	wl_wext_info_t *wext_info = NULL;

	if (dhdp) {
		wext_info = dhdp->wext_info;
 	} else {
		WL_ERROR (("dhd is NULL\n"));
		return;
	}
}

int
wl_iw_up(struct net_device *dev, dhd_pub_t *dhdp)
{
	wl_wext_info_t *wext_info = NULL;
	int ret = 0;

	if (dhdp) {
		wext_info = dhdp->wext_info;
 	} else {
		WL_ERROR (("dhd is NULL\n"));
		return -ENODEV;
	}

	return ret;
}

#endif /* USE_IW */
