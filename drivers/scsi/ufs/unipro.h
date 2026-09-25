/*
 * drivers/scsi/ufs/unipro.h
 *
 * Copyright (C) 2013 Samsung Electronics Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef _UNIPRO_H_
#define _UNIPRO_H_

#include "mphy-unipro.h"
#include "mphy-unipro-vs.h"

/* M-TX Configuration Attributes */
#define TX_REFCLKFREQ				0x00EB
#define TX_CFGCLKFREQVAL			0x00EC
#define	CFGEXTRATTR				0x00F0
#define DITHERCTRL2				0x00F1
#define TX_FSM_DISABLED				0x0
#define TX_FSM_HIBERN8				0x1
#define TX_FSM_SLEEP				0x2
#define TX_FSM_STALL				0x3
#define TX_FSM_LS_BURST				0x4
#define TX_FSM_HS_BURST				0x5
#define TX_FSM_LINE_CFG				0x6
#define TX_FSM_LINE_RESET			0x7

/* M-RX Configuration Attributes */
#define RX_ENTER_HIBERN8			0x00A7
#define RX_REFCLKFREQ				0x00EB
#define	RX_CFGCLKFREQVAL			0x00EC
#define CFGWIDEINLN				0x00F0
#define CFGRXCDR8				0x00BA
#define ENARXDIRECTCFG4				0x00F2
#define CFGRXOVR8				0x00BD
#define RXDIRECTCTRL2				0x00C7
#define ENARXDIRECTCFG3				0x00F3
#define RXCALCTRL				0x00B4
#define ENARXDIRECTCFG2				0x00F4
#define CFGRXOVR4				0x00E9
#define RXSQCTRL				0x00B5
#define CFGRXOVR6				0x00BF

#define is_mphy_tx_attr(attr)			((attr) < RX_MODE)
#define RX_MIN_ACTIVATETIME_UNIT_US		100
#define HIBERN8TIME_UNIT_US			100

/* Common Block Attributes */
#define TX_GLOBALHIBERNATE			UNIPRO_CB_OFFSET(0x002B)
#define REFCLKMODE				UNIPRO_CB_OFFSET(0x00BF)
#define DIRECTCTRL19				UNIPRO_CB_OFFSET(0x00CD)
#define DIRECTCTRL10				UNIPRO_CB_OFFSET(0x00E6)
#define CDIRECTCTRL6				UNIPRO_CB_OFFSET(0x00EA)
#define RTOBSERVESELECT				UNIPRO_CB_OFFSET(0x00F0)
#define CBDIVFACTOR				UNIPRO_CB_OFFSET(0x00F1)
#define CBDCOCTRL5				UNIPRO_CB_OFFSET(0x00F3)
#define CBPRGPLL2				UNIPRO_CB_OFFSET(0x00F8)
#define CBPRGTUNING				UNIPRO_CB_OFFSET(0x00FB)

#define UNIPRO_CB_OFFSET(x)			(0x8000 | (x))

/* PHY Adpater attributes */
#define PA_PHY_TYPE		0x1500
#define PA_TXFORCECLOCK		0x1562
#define PA_TXPWRMODE		0x1563
#define PA_LEGACYDPHYESCDL	0x1570
#define PA_MAXTXSPEEDFAST	0x1521
#define PA_MAXTXSPEEDSLOW	0x1522
#define PA_MAXRXSPEEDFAST	0x1541
#define PA_MAXRXSPEEDSLOW	0x1542
#define PA_TXLINKSTARTUPHS	0x1544
#define PA_TXSPEEDFAST		0x1565
#define PA_TXSPEEDSLOW		0x1566
#define PA_RXLSTERMCAP		0x15A6
#define PA_PWRMODEUSER_DATA0	0x15B0

/* UFS Device UniPro version */
#define UNIPRO_VER_1_6_1	4
#define UNIPRO_VER_1_8		5

#define PA_TACTIVATE_TIME_UNIT_US	10
#define PA_HIBERN8_TIME_UNIT_US		100

/* Other attributes */
#define VS_MPHYCFGUPDT		0xD085
#define VS_DEBUGOMC		0xD09E
#define VS_POWERSTATE		0xD083

#define PA_GRANULARITY_MIN_VAL	1
#define PA_GRANULARITY_MAX_VAL	6

/* PHY Adapter Protocol Constants */
#define PA_MAXDATALANES	4

/* PA power modes */
enum {
	FAST_MODE	= 1,
	SLOW_MODE	= 2,
	FASTAUTO_MODE	= 4,
	SLOWAUTO_MODE	= 5,
	UNCHANGED	= 7,
};

/* PA TX/RX Frequency Series */
enum {
	PA_HS_MODE_A	= 1,
	PA_HS_MODE_B	= 2,
};

enum ufs_pwm_gear_tag {
	UFS_PWM_DONT_CHANGE,    /* Don't change Gear */
	UFS_PWM_G1,             /* PWM Gear 1 (default for reset) */
	UFS_PWM_G2,             /* PWM Gear 2 */
	UFS_PWM_G3,             /* PWM Gear 3 */
	UFS_PWM_G4,             /* PWM Gear 4 */
	UFS_PWM_G5,             /* PWM Gear 5 */
	UFS_PWM_G6,             /* PWM Gear 6 */
	UFS_PWM_G7,             /* PWM Gear 7 */
};

enum ufs_hs_gear_tag {
	UFS_HS_DONT_CHANGE,     /* Don't change Gear */
	UFS_HS_G1,              /* HS Gear 1 (default for reset) */
	UFS_HS_G2,              /* HS Gear 2 */
	UFS_HS_G3,              /* HS Gear 3 */
	UFS_HS_G4,              /* HS Gear 4 */
};

enum ufs_unipro_ver {
	UFS_UNIPRO_VER_RESERVED = 0,
	UFS_UNIPRO_VER_1_40 = 1, /* UniPro version 1.40 */
	UFS_UNIPRO_VER_1_41 = 2, /* UniPro version 1.41 */
	UFS_UNIPRO_VER_1_6 = 3,  /* UniPro version 1.6 */
	UFS_UNIPRO_VER_MAX = 4,  /* UniPro unsupported version */
	/* UniPro version field mask in PA_LOCALVERINFO */
	UFS_UNIPRO_VER_MASK = 0xF,
};

enum {
	PA_HS_APT_REFRESH,      /* REFRESH */
	PA_HS_APT_INITIAL,      /* INITIAL */
	PA_HS_APT_RESERVED,     /* Reserved */
	PA_HS_APT_NO,           /* No ADAPT */
};

#define CPORT_APPLICATION	1
#define CPORT_UNDER_TEST	2

#ifdef FALSE
#undef FALSE
#endif

#ifdef TRUE
#undef TRUE
#endif

/* CPort setting */
#define E2EFC_ON	(1 << 0)
#define E2EFC_OFF	(0 << 0)
#define CSD_N_ON	(0 << 1)
#define CSD_N_OFF	(1 << 1)
#define CSV_N_ON	(0 << 2)
#define CSV_N_OFF	(1 << 2)
#define CPORT_DEF_FLAGS	(CSV_N_OFF | CSD_N_OFF | E2EFC_OFF)

/* CPort connection state */
enum {
	CPORT_IDLE = 0,
	CPORT_CONNECTED,
};

/* Boolean attribute values */
enum {
	FALSE = 0,
	TRUE,
};

#endif /* _UNIPRO_H_ */
