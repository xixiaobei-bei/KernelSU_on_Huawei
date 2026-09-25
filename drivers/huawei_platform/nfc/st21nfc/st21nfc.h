/*
 * Copyright (C) 2013 ST Microelectronics S.A.
 * Copyright (C) 2010 Stollmann E+V GmbH
 * Copyright (C) 2010 Trusted Logic S.A.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#define ST21NFC_MAGIC	0xEA

#define ST21NFC_NAME "st21nfc"
/*
 * ST21NFC power control via ioctl
 * ST21NFC_GET_WAKEUP :  poll gpio-level for Wakeup pin
 * PN544_SET_PWR(1): power on
 * PN544_SET_PWR(>1): power on with firmware download enabled
 */
#define ST21NFC_GET_WAKEUP	      _IOR(ST21NFC_MAGIC, 0x01, unsigned int)
#define ST21NFC_PULSE_RESET		_IOR(ST21NFC_MAGIC, 0x02, unsigned int)
#define ST21NFC_SET_POLARITY_RISING   _IOR(ST21NFC_MAGIC, 0x03, unsigned int)
#define ST21NFC_SET_POLARITY_FALLING  _IOR(ST21NFC_MAGIC, 0x04, unsigned int)
#define ST21NFC_SET_POLARITY_HIGH     _IOR(ST21NFC_MAGIC, 0x05, unsigned int)
#define ST21NFC_SET_POLARITY_LOW      _IOR(ST21NFC_MAGIC, 0x06, unsigned int)
#define ST21NFC_GET_POLARITY	      _IOR(ST21NFC_MAGIC, 0x07, unsigned int)
#define ST21NFC_RECOVERY              _IOR(ST21NFC_MAGIC, 0x08, unsigned int)

#define DEFAULT_NFC_CLK_RATE	(192 * 100000L)
#define NFC_CLK_PIN     "clk_out0"

#define CLR_BIT     0
#define SET_BIT     1

#define DLOAD_EXTENTED_GPIO_MASK (0x01)
#define IRQ_EXTENTED_GPIO_MASK (0x02)
#define VEN_EXTENTED_GPIO_MASK (0x04)

#define MAX_DETECT_SE_SIZE	32
#define MAX_CONFIG_NAME_SIZE		64

#define ENABLE_START	0
#define ENABLE_END		1

#define WAKE_LOCK_TIMEOUT_DISABLE		  0
#define WAKE_LOCK_TIMEOUT_ENALBE		  1
#define MAX_WAKE_LOCK_TIMEOUT_SIZE	16

struct st21nfc_platform_data {
	unsigned int irq_gpio;
	unsigned int ena_gpio;
	unsigned int reset_gpio;
	unsigned int polarity_mode;
	struct clk *nfc_clk;
};

typedef struct pmu_reg_control {
    int addr;  /* reg address */
    int pos;   /* bit position */
} t_pmu_reg_control;

enum NFC_ON_TYPE {
	NFC_ON_BY_GPIO = 0,
	NFC_ON_BY_HISI_PMIC,        // chicago platform, Hi6421V530
	NFC_ON_BY_HI6421V600_PMIC,  // boston platform, Hi6421V600
	NFC_ON_BY_REGULATOR_BULK,
	NFC_ON_BY_HI6555V110_PMIC,  // V8R5 platform, Hi555V110
};

enum NFC_SWP_SWITCH_PMU_PLATFROM_TYPE {
	NFC_SWP_WITHOUT_SW = 0,
	NFC_SWP_SW_HI6421V500 = 1,
	NFC_SWP_SW_HI6421V600 = 2,
	NFC_SWP_SW_HI6555V110 = 3,
};

/*
 * NFCC can wired different eSEs by SWP/DWP
 * default NFCC don't wired eSE.
 * NFC_WITHOUT_ESE: NFCC don't wired eSE
 * NFC_ESE_P61: use NXP p61 as eSE
 * NFC_ESE_HISEE: use hisi se as eSE
 */
enum NFC_ESE_TYPE {
	NFC_WITHOUT_ESE = 0,
	NFC_ESE_P61,
	NFC_ESE_HISEE,
};

#define MAX_ATTRIBUTE_BUFFER_SIZE 128

#define NFC_CLK_SRC_CPU		  0
#define NFC_CLK_SRC_PMU		  1
#define NFC_CLK_SRC_PMU_HI6555 2
#define NFC_CLK_SRC_PMU_HI6421V600 3
#define NFC_CLK_SRC_XTAL        4

#define CARD_UNKNOWN	0
#define CARD1_SELECT	 1
#define CARD2_SELECT  2

#define NFC_SVDD_SW_ON	1
#define NFC_SVDD_SW_OFF	0

#define UICC_SUPPORT_CARD_EMULATION (1<<0)
#define eSE_SUPPORT_CARD_EMULATION (1<<1)
#define MAX_NFC_FW_VERSION_SIZE	32
#define NFC_DMD_NUMBER_MIN  923002000
#define NFC_DMD_NUMBER_MAX  923002016

#define TEL_HUAWEI_NV_NFCCAL_NUMBER   372
#define TEL_HUAWEI_NV_NFCCAL_NAME     "NFCCAL"
#define TEL_HUAWEI_NV_NFCCAL_LEGTH    104



