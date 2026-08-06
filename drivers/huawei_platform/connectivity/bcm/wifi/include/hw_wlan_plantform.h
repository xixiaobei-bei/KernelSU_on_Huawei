
#ifndef _HW_WLANP_LANTFORM_H
#define _HW_WLANP_LANTFORM_H
#define WLAN_PLAT_NODFS_FLAG	0x01
#ifdef CONFIG_BCMDHD
#define WLAN_PLAT_AP_FLAG   0x02
#endif
struct wifi_platform_data {
	int (*set_power)(int val);
	int (*set_reset)(int val);
	int (*set_carddetect)(int val);
	void *(*mem_prealloc)(int section, unsigned long size);
	int (*get_mac_addr)(unsigned char *buf);
	int (*get_wake_irq)(void);
	void *(*get_country_code)(char *ccode, u32 flags);
#ifdef CONFIG_BCMDHD
	int (*get_nvram_path)(char *val, int len);
#ifdef HW_WIFI_DRIVER_NORMALIZE
	int (*get_fw_path)(char *val, int len);
	int (*get_chip_type)(char *val, int len);
#endif /* HW_WIFI_DRIVER_NORMALIZE */
#ifdef HW_CUSTOM_BCN_TIMEOUT
	int (*get_bcn_timeout)(void);
#endif /* HW_CUSTOM_BCN_TIMEOUT */
#endif
}__no_randomize_layout;
#endif
