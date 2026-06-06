#ifndef _HISI_USB_CHARGER_H_
#define _HISI_USB_CHARGER_H_

#include <linux/hisi/usb/hisi_usb.h>
#include "dwc3-hisi.h"

/*
 * hisi dwc3 bc registers
 */
#define BC_CTRL0		0x30		/* BC¿ØÖÆÆ÷¼Ä´æÆ÷0 */
#define BC_CTRL1		0x34		/* BC¿ØÖÆÆ÷¼Ä´æÆ÷1 */
#define BC_CTRL2		0x38		/* BC¿ØÖÆÆ÷¼Ä´æÆ÷2 */
#define BC_STS0			0x3C		/* BC×´Ì¬¼Ä´æÆ÷0 */

/* BC_CTRL0 */
# define BC_CTRL0_BC_IDPULLUP		(1 << 10)
# define BC_CTRL0_BC_SUSPEND_N		(1 << 9)
# define BC_CTRL0_BC_DMPULLDOWN		(1 << 8)
# define BC_CTRL0_BC_DPPULLDOWN		(1 << 7)
# define BC_CTRL0_BC_TXVALIDH		(1 << 6)
# define BC_CTRL0_BC_TXVALID		(1 << 5)
# define BC_CTRL0_BC_TERMSELECT		(1 << 4)
# define BC_CTRL0_BC_XCVRSELECT(x)	(((x) << 2) & (3 << 2))
# define BC_CTRL0_BC_OPMODE(x)		((x) & 3)

/* BC_CTRL1 */
# define BC_CTRL1_BC_MODE	1

/* BC_CTRL2 */
# define BC_CTRL2_BC_PHY_VDATDETENB	(1 << 4)
# define BC_CTRL2_BC_PHY_VDATARCENB	(1 << 3)
# define BC_CTRL2_BC_PHY_CHRGSEL		(1 << 2)
# define BC_CTRL2_BC_PHY_DCDENB		(1 << 1)
# define BC_CTRL2_BC_PHY_ACAENB		(1 << 0)

/* BC_STS0 */
# define BC_STS0_BC_LINESTATE(x)	(((x) << 9) & (3 << 9))
# define BC_STS0_BC_PHY_CHGDET		(1 << 8)
# define BC_STS0_BC_PHY_FSVMINUS	(1 << 7)
# define BC_STS0_BC_PHY_FSVPLUS		(1 << 6)
# define BC_STS0_BC_RID_GND		(1 << 5)
# define BC_STS0_BC_RID_FLOAT		(1 << 4)
# define BC_STS0_BC_RID_C		(1 << 3)
# define BC_STS0_BC_RID_B		(1 << 2)
# define BC_STS0_BC_RID_A		(1 << 1)
# define BC_STS0_BC_SESSVLD		(1 << 0)

/*
 * hisi usb bc
 */

#define BC_AGAIN_DELAY_TIME_1 200
#define BC_AGAIN_DELAY_TIME_2 8000
#define BC_AGAIN_ONCE	1
#define BC_AGAIN_TWICE	2
void notify_charger_type(struct hisi_dwc3_device *hisi_dwc3);

/* bc interface */
void hisi_bc_disable_vdp_src(struct hisi_dwc3_device *hisi_dwc3);
void hisi_bc_enable_vdp_src(struct hisi_dwc3_device *hisi_dwc3);
void hisi_bc_dplus_pulldown(struct hisi_dwc3_device *hisi_dwc);
void hisi_bc_dplus_pullup(struct hisi_dwc3_device *hisi_dwc);
enum hisi_charger_type detect_charger_type(struct hisi_dwc3_device *hisi_dwc3);
void schedule_bc_again(struct hisi_dwc3_device *hisi_dwc);
void cancel_bc_again(struct hisi_dwc3_device *hisi_dwc, int sync);
bool enumerate_allowed(struct hisi_dwc3_device *hisi_dwc);
bool sleep_allowed(struct hisi_dwc3_device *hisi_dwc);
bool bc_again_allowed(struct hisi_dwc3_device *hisi_dwc);
int hisi_usb_bc_init(struct hisi_dwc3_device *hisi_dwc);
void hisi_usb_bc_exit(struct hisi_dwc3_device *hisi_dwc);
#endif /* _HISI_USB_CHARGER_H_ */
