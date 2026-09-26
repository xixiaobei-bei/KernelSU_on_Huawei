

#ifndef _HW_CONNECTIVITY_H
#define _HW_CONNECTIVITY_H

#include <linux/types.h>

#define CHIP_TYPE_HI110X "hisi"
#define CHIP_TYPE_BCM "bcm43xx"
#define NFC_CHIP_TYPE_HI110X "hisi_nfc"
#define NFC_CHIP_TYPE_NXP "nxp_nfc"
#ifdef CONFIG_HWCONNECTIVITY
bool isMyConnectivityChip(const char *name);
bool isMyNfcChip(const char *name);
#else
/* For no OneTrack image, always return ture */
static inline bool isMyConnectivityChip(const char *name)
{
	return true;
}

static inline bool isMyNfcChip(const char *name)
{
	return true;
}
#endif /* end of CONFIG_HWCONNECTIVITY */

#endif
